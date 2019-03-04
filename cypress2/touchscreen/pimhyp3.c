/*
 *  Driver for Pimoroni Hyperpixel 3.5" Touchscreen LCD
 *  uses Cypress Cy8c20466 Touchscreen Capsense Psoc
 *
 *  Adapted from Pimoroni's Python driver.
 *  Adds commands to program the LCD, from Pimoroni's hyperpixel-init.c
 *
 *  Copyright (c) 2019 ProCount
 */

//#define DEBUG

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2 of the License.
 */

/* DTparameters:
 *        x-invert     = <&cy8c20466>, "touchscreen-inverted-x:0";
 *        y-invert     = <&cy8c20466>, "touchscreen-inverted-y:0";
 *        xy-swap      = <&cy8c20466>, "touchscreen-swapped-x-y:0";
 *        x-size       = <&cy8c20466>, "touchscreen-size-x:0";
 *        y-size       = <&cy8c20466>, "touchscreen-size-y:0";
 *        x2y          = <&cy8c20466>, "touchscreen-x2y:0";         //Determines order of inversion vs swapping (?)
 *        refresh-rate = <&cy8c20466>, "touchscreen-refresh-rate:0";
 *        rotate_0     = rotate   0 deg cw
 *        rotate_1     = rotate  90 deg cw
 *        rotate_2     = rotate 180 deg cw
 *        rotate_3     = rotate 270 deg cw
 */

#include <linux/kernel.h>
#include <linux/dmi.h>
#include <linux/firmware.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <asm/unaligned.h>
#include <linux/jiffies.h>
#include <linux/delay.h>

/////////////////////////////// DEFINITIONS ////////////////////////////////

#define uDELAY 100 // clock pulse time in microseconds
#define mWAIT  120  // wait time in milliseconds

#define PIMHYP3_GPIO_INT_NAME       "irq"
#define PIMHYP3_GPIO_MOSI_NAME      "mosi"
#define PIMHYP3_GPIO_CS_NAME        "cs"

#define PIMHYP3_INT_TRIGGER		    0

#define PIMHYP3_MAX_WIDTH           800
#define PIMHYP3_MAX_HEIGHT          480

/* Addresses within i2c map */
#define PIMHYP3_FIXED1              0x30
#define PIMHYP3_FIXED2              0x70

#define PIMHYP3_NUM_TOUCHES         0x6D
#define PIMHYP3_INT_SETTINGS        0x6E
#define PIMHYP3_COORDS              0x40

const u8 fixed1[16] = {0xe0, 0x00, 0x01, 0x04, 0x04, 0x3c, 0x53, 0x78, 0x78, 0x00, 0x01, 0x00, 0x00, 0x0c, 0x14, 0x00}; 
const u8 fixed2[16] = {0xa0, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x11, 0x23, 0x50, 0x13, 0x01, 0x00, 0x00};

struct pimhyp3_ts_data {
    struct i2c_client  *client;
    struct input_dev   *input_dev;
    struct gpio_desc   *gpiod_int;
    struct gpio_desc   *gpiod_mosi;
    struct gpio_desc   *gpiod_cs;
    struct task_struct *thread;
    u16 id;
    u16 version;
    //instance data
    int last_x1;
    int last_y1;
    int last_x2;
    int last_y2;
    int last_numTouches;
    //Configuration parameters
    bool use_poll;
    bool X2Y;
    u32 abs_x_max;
    u32 abs_y_max;
    u8   requestedRefreshRate;
    bool swapped_x_y;
    bool inverted_x;
    bool inverted_y;
    unsigned int max_touch_num;
    unsigned long irq_flags;
    unsigned int int_trigger_type;
};

static const unsigned long pimhyp3_irq_flags[] = {
	IRQ_TYPE_EDGE_RISING,
	IRQ_TYPE_EDGE_FALLING,
	IRQ_TYPE_LEVEL_LOW,
	IRQ_TYPE_LEVEL_HIGH,
};

/////////////////////////////// LOW LEVEL //////////////////////////////////

/**
 * pimhyp3_i2c_read - read data from a register of the i2c slave device.
 *
 * @client: i2c Client info.
 * @reg: the register to read from.
 * @buf: raw read data buffer.
 * @len: length of the buffer to read
 * @returns 0, or -EIO if error
 */
static int pimhyp3_i2c_read(struct i2c_client *client,
               u16 reg, u8 *buf, int len)
{
    int bytes_read;

    bytes_read = i2c_smbus_read_i2c_block_data(client, reg, len, buf);
    return (bytes_read != len ? -EIO : 0);
}

/**
 * pimhyp3_i2c_write - write data to a register of the i2c slave device.
 *
 * @client: i2c device.
 * @reg: the register to write to.
 * @buf: raw data buffer to write.
 * @len: length of the buffer to write
 */
static int pimhyp3_i2c_write(struct i2c_client *client, u16 reg, const u8 *buf,
                unsigned len)
{
    int bytes_written;

    bytes_written = i2c_smbus_write_i2c_block_data(client, reg, len, buf);
    return (bytes_written != len ? -EIO : 0);
}

/////////////////////////////// PROCESSING /////////////////////////////////

static void pimhyp3_ts_process_coords(struct pimhyp3_ts_data *ts, u16 x1, u16 y1,u16 x2, u16 y2, u8 numTouches) 
{
    int output;

    /* Adjust coordinates to screen rotation */
    if(!ts->X2Y)
    {
        /* Inversions have to happen after axis swapping */
        if (ts->swapped_x_y)
            swap(x1, y1);
    }
    if (ts->inverted_x)
        x1 = ts->abs_x_max - x1;
    if (ts->inverted_y)
        y1 = ts->abs_y_max - y1;

    if (ts->X2Y)
    {
        /* Inversions have to happen before axis swapping */
        if (ts->swapped_x_y)
            swap(x1, y1);
    }

    output=0;
    if (numTouches)
    {
        if (!ts->last_numTouches)
        {
            /* new 1 press */

            output |= 1;
            input_mt_slot(ts->input_dev, 0);
            input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, 0);

            input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x1);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y1);
            input_report_key(ts->input_dev, BTN_TOUCH, 1);
            input_report_abs(ts->input_dev, ABS_X, x1);
            input_report_abs(ts->input_dev, ABS_Y, y1);
            dev_dbg( &ts->client->dev, "Press 1: (%d,%d)",x1,y1);
        }
        else
        {
            /* Contact moved? */
            if ((x1 != ts->last_x1) || (y1 != ts->last_y1))
            {
                output |= 1;
                input_mt_slot(ts->input_dev, 0);

                input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, 0);
            	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);
                if (x1 != ts->last_x1)
                    input_report_abs(ts->input_dev, ABS_X, x1);
                if (y1 != ts->last_y1)
                    input_report_abs(ts->input_dev, ABS_Y, y1);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x1);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y1);
                dev_dbg( &ts->client->dev, "Move 1: (%d,%d)",x1,y1);
            }
        }
    }
    else if (ts->last_numTouches) //!numTouches
    {
        /* new 1 release */
        output |= 1;
        input_mt_slot(ts->input_dev, 0);

        input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, -1);
       	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
        input_report_key(ts->input_dev, BTN_TOUCH, 0);
        dev_dbg( &ts->client->dev, "Release 1");
    }

    if (numTouches==2)
    {
        if (ts->last_numTouches<2)
        {
            /* new 2 press */
            output |= 2;
            input_mt_slot(ts->input_dev, 1);

            input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, 1);
        	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x2);
            input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y2);
            input_report_key(ts->input_dev, BTN_TOUCH, 1);
            input_report_abs(ts->input_dev, ABS_X, x2);
            input_report_abs(ts->input_dev, ABS_Y, y2);
            dev_dbg( &ts->client->dev, "Press 2: (%d,%d)",x2,y2);
        }
        else
        {
            /* Contact moved? */
            if ((x2 != ts->last_x2) || (y2 != ts->last_y2))
            {
                output |= 2;
                input_mt_slot(ts->input_dev, 1);

                input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, 1);
            	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);
                if (x2 != ts->last_x2)
                    input_report_abs(ts->input_dev, ABS_X, x2);
                if (y2 != ts->last_y2)
                    input_report_abs(ts->input_dev, ABS_Y, y2);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_X, x2);
                input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, y2);
                dev_dbg( &ts->client->dev, "Move 2: (%d,%d)",x2,y2);
            }
        }
    }
    else if (ts->last_numTouches==2) // (numTouches<2))
    {
        /* new 2 release */
        output |= 2;
        input_mt_slot(ts->input_dev, 1);

        input_report_abs(ts->input_dev, ABS_MT_TRACKING_ID, -1);
       	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, false);
        input_report_key(ts->input_dev, BTN_TOUCH, 0);
        dev_dbg( &ts->client->dev, "Release 2");
    }

    if (output)
        input_sync(ts->input_dev);

    ts->last_x1 = x1;
    ts->last_y1 = y1;
    ts->last_x2 = x2;
    ts->last_y2 = y2;
    ts->last_numTouches = numTouches;
}

/**
 * pimhyp3_process_events - Process incoming events
 *
 * @ts: our pimhyp3_ts_data pointer
 *
 * Called from poll or when the IRQ is triggered. Read the current device state, and push
 * the input events to the user space.
 */
static void pimhyp3_process_events(struct pimhyp3_ts_data *ts)
{
    //int pin = desc_to_gpio(ts->gpiod_int);

    //int value;

    if (gpiod_get_value(ts->gpiod_int) || ts->last_numTouches)
    {   //Only process the TS if something has happened or a contact in progress

        u16 x1,y1,x2,y2;
        u8 rawdata[8];
        u8 numTouches;

        pimhyp3_i2c_read(ts->client, PIMHYP3_NUM_TOUCHES, &numTouches, 1);

        pimhyp3_i2c_read(ts->client, PIMHYP3_COORDS, rawdata, 8);

        x1 = rawdata[0] | (rawdata[4] << 8);
        y1 = rawdata[1] | (rawdata[5] << 8);
        x2 = rawdata[2] | (rawdata[6] << 8);
        y2 = rawdata[3] | (rawdata[7] << 8);

        pimhyp3_ts_process_coords(ts, x1,y1,x2,y2,numTouches);
    }
}


/* Thread to poll for touchscreen events
 *
 */
static int pimhyp3_thread(void *arg)
{

    while (!kthread_should_stop())
    {
        struct pimhyp3_ts_data *ts = (struct pimhyp3_ts_data *) arg;

        /* 60fps polling */
        msleep_interruptible(ts->requestedRefreshRate); //17

        pimhyp3_process_events(ts);
    }

    return 0;
}

/**
 * goodix_ts_irq_handler - The IRQ handler
 *
 * @irq: interrupt number.
 * @dev_id: private data pointer.
 */
static irqreturn_t pimhyp3_ts_irq_handler(int irq, void *dev_id)
{
	struct pimhyp3_ts_data *ts = dev_id;

	pimhyp3_process_events(ts);

//	if (goodix_i2c_write_u8(ts->client, GOODIX_READ_COOR_ADDR, 0) < 0)
//		dev_err(&ts->client->dev, "I2C write end_cmd error\n");

	return IRQ_HANDLED;
}

static void pimhyp3_free_irq(struct pimhyp3_ts_data *ts)
{
	devm_free_irq(&ts->client->dev, ts->client->irq, ts);
}

static int pimhyp3_request_irq(struct pimhyp3_ts_data *ts)
{
	return devm_request_threaded_irq(&ts->client->dev, ts->client->irq,
					 NULL, pimhyp3_ts_irq_handler,
					 ts->irq_flags, ts->client->name, ts);
}

/////////////////////////////// DETECTION //////////////////////////////////

/**
 * pimhyp3_check_fixed - I2C test function to chcek the fixed data is correct.
 *
 * @client: the i2c client
 * @reg:    the starting register
 * @fixed:  a 16 byte array of hte expected data
 * @returns 0 if ok. -1 data mismatch, -2 can't read data
 */
static int pimhyp3_check_fixed(struct i2c_client *client, int reg, const u8* fixed)
{
    u8 input_buffer[16];
    int error=0;
    int i;


    error = pimhyp3_i2c_read(client, reg, input_buffer, 16);
    if (error)
    {
        dev_err(&client->dev, "i2c test failed: %d\n",error);
        return(-2);
    }
    for (i=0; i<16; i++)
    {
        if (input_buffer[i] != fixed[i])
        {
            dev_err(&client->dev, "i2c check failed at: %02x = [%02x]\n",reg+i,input_buffer[i] );
            error = -1;
        }
    }
    return(error);
}

/**
 * pimhyp3_i2c_test - I2C test function to check if the device answers.
 *
 * @client: the i2c client
 * @returns: 0 if ok, -1 or -2 if error
 */
static int pimhyp3_i2c_test(struct i2c_client *client)
{
    /* Code to detect TS is fitted -ok */
    int retry = 0;
    int error1, error2;

    while (retry++ <2)
    {
        error1 = pimhyp3_check_fixed(client, PIMHYP3_FIXED1, fixed1);
        error2 = pimhyp3_check_fixed(client, PIMHYP3_FIXED2, fixed2);

        if (!error1 && !error2)
            return(0);
        msleep(20);
    }

    if (error2)
        error1 = error2;

    return error1;
}


/////////////////////////////// TERMINATION ////////////////////////////////


static int pimhyp3_ts_remove(struct i2c_client *client)
{
    struct pimhyp3_ts_data *ts = i2c_get_clientdata(client);

    if (ts->gpiod_int)
        devm_gpiod_put(&client->dev, ts->gpiod_int);

    if (ts->use_poll)
    	kthread_stop(ts->thread);
    else
        pimhyp3_free_irq(ts);

    return 0;
}

/////////////////////////////// INITIALISATION /////////////////////////////


static int32_t commands[] = {
    -1,     0x0011, -1,     0x0001, -1,     0x00c1, 0x01a8, 0x01b1,
    0x0145, 0x0104, 0x00c5, 0x0180, 0x016c, 0x00c6, 0x01bd, 0x0184,
    0x00c7, 0x01bd, 0x0184, 0x00bd, 0x0102, 0x0011, -1,     0x0100,
    0x0100, 0x0182, 0x0026, 0x0108, 0x00e0, 0x0100, 0x0104, 0x0108,
    0x010b, 0x010c, 0x010d, 0x010e, 0x0100, 0x0104, 0x0108, 0x0113,
    0x0114, 0x012f, 0x0129, 0x0124, 0x00e1, 0x0100, 0x0104, 0x0108,
    0x010b, 0x010c, 0x0111, 0x010d, 0x010e, 0x0100, 0x0104, 0x0108,
    0x0113, 0x0114, 0x012f, 0x0129, 0x0124, 0x0026, 0x0108, 0x00fd,
    0x0100, 0x0108, 0x0029
};


static void send_bits(struct pimhyp3_ts_data *ts, uint16_t data, uint16_t count)
{
    int x;
    int mask = 1 << (count-1);
    for(x = 0; x < count; x++){
        gpiod_set_value(ts->gpiod_mosi,(data & mask) > 0);
        data <<= 1;
        gpiod_set_value(ts->gpiod_int, 0);
        udelay(uDELAY);
        gpiod_set_value(ts->gpiod_int, 1);
        udelay(uDELAY);
    }
    gpiod_set_value(ts->gpiod_mosi,0);
}

static void write(struct pimhyp3_ts_data *ts, uint16_t command)
{
    gpiod_set_value(ts->gpiod_cs,0);
    send_bits(ts, command, 9);
    gpiod_set_value(ts->gpiod_cs,1);
}

static void lcd_init(struct pimhyp3_ts_data *ts)
{
    int count;
    int x;
    //setup_pins

    gpiod_direction_output(ts->gpiod_cs,1);
    gpiod_direction_output(ts->gpiod_int,1);    //CLK = output
    gpiod_direction_output(ts->gpiod_mosi,0);

    //setup_lcd
    count = sizeof(commands) / sizeof(int32_t);
    for(x = 0; x < count; x++){
        int32_t command = commands[x];
        if(command == -1){
            mdelay(mWAIT);
            continue;
        }
        write(ts,(uint16_t)command);
    }

    //cleanup_pins
    // Return the touch interrupt pin to a usable state
    gpiod_direction_input(ts->gpiod_int);
}


/**
 * pimhyp3_get_gpio_config - Get GPIO config from ACPI/DT
 *
 * @ts: pimhyp3_ts_data pointer
 */
static int pimhyp3_get_gpio_config(struct pimhyp3_ts_data *ts)
{
    int error;
    struct device *dev;
    struct gpio_desc *gpiod;
    u32 sizeX, sizeY, refreshRate;

    if (!ts->client)
        return -EINVAL;
    dev = &ts->client->dev;

    /* Get the interrupt GPIO pin description */
    gpiod = devm_gpiod_get(dev, PIMHYP3_GPIO_INT_NAME, GPIOD_IN);
    if (IS_ERR(gpiod)) {
        error = PTR_ERR(gpiod);
        if (error != -EPROBE_DEFER)
            dev_dbg(dev, "Failed to get %s GPIO: %d\n",
                PIMHYP3_GPIO_INT_NAME, error);
        return error;
    }

    /* Ensure the correct direction is set because this pin is also used to
     * program the LCD controller
     */
    ts->gpiod_int = gpiod;
    gpiod_direction_input(ts->gpiod_int);

    /* Get the MOSI GPIO pin description */
    gpiod = devm_gpiod_get(dev, PIMHYP3_GPIO_MOSI_NAME, GPIOD_OUT_LOW);
    if (IS_ERR(gpiod)) {
        error = PTR_ERR(gpiod);
        if (error != -EPROBE_DEFER)
            dev_dbg(dev, "Failed to get %s GPIO: %d\n",
                PIMHYP3_GPIO_MOSI_NAME, error);
        return error;
    }

    ts->gpiod_mosi = gpiod;

    /* Get the CS GPIO pin description */
    gpiod = devm_gpiod_get(dev, PIMHYP3_GPIO_CS_NAME, GPIOD_OUT_HIGH);
    if (IS_ERR(gpiod)) {
        error = PTR_ERR(gpiod);
        if (error != -EPROBE_DEFER)
            dev_dbg(dev, "Failed to get %s GPIO: %d\n",
                PIMHYP3_GPIO_CS_NAME, error);
        return error;
    }

    ts->gpiod_cs = gpiod;


    ts->id = 0x1001;
    ts->version=0x0101;

    ts->last_numTouches=0;
    ts->max_touch_num=2;
    ts->abs_x_max = PIMHYP3_MAX_WIDTH;
    ts->abs_y_max = PIMHYP3_MAX_HEIGHT;
    ts->use_poll = false;
    ts->int_trigger_type = PIMHYP3_INT_TRIGGER;

    // Read DT configuration parameters
    ts->X2Y = device_property_read_bool(&ts->client->dev,
                                "touchscreen-x2y");
    dev_dbg(&ts->client->dev, "touchscreen-x2y %u", ts->X2Y);

    ts->use_poll = device_property_read_bool(&ts->client->dev,
                                "touchscreen-poll");
    dev_dbg(&ts->client->dev, "touchscreen-poll %u", ts->use_poll);


    if(device_property_read_u32(&ts->client->dev, "touchscreen-refresh-rate", &refreshRate))
    {
        dev_dbg(&ts->client->dev, "touchscreen-refresh-rate not found");
        ts->requestedRefreshRate = 17;
    }
    else
    {
        dev_dbg(&ts->client->dev, "touchscreen-refresh-rate found %u", refreshRate);
        ts->requestedRefreshRate = (u8)refreshRate;
    }

    if(device_property_read_u32(&ts->client->dev, "touchscreen-size-x", &sizeX))
    {
        dev_dbg(&ts->client->dev, "touchscreen-size-x not found. Using default of %u",ts->abs_x_max);
    }
    else
    {
        ts->abs_x_max = (u16)sizeX;
        dev_dbg(&ts->client->dev, "touchscreen-size-x found %u", ts->abs_x_max);
    }

    if(device_property_read_u32(&ts->client->dev, "touchscreen-size-y", &sizeY))
    {
        dev_dbg(&ts->client->dev, "touchscreen-size-y not found. Using default of %u",ts->abs_y_max);
    }
    else
    {
        ts->abs_y_max = (u16)sizeY;
        dev_dbg(&ts->client->dev, "touchscreen-size-y found %u", ts->abs_y_max);
    }

    dev_dbg(&ts->client->dev, "requested size (%u, %u)", ts->abs_x_max, ts->abs_y_max);

    ts->swapped_x_y = device_property_read_bool(&ts->client->dev, "touchscreen-swapped-x-y");
    dev_dbg(&ts->client->dev, "touchscreen-swapped-x-y %u", ts->swapped_x_y);

    ts->inverted_x = device_property_read_bool(&ts->client->dev,
                           "touchscreen-inverted-x");
    dev_dbg(&ts->client->dev, "touchscreen-inverted-x %u", ts->inverted_x);

    ts->inverted_y = device_property_read_bool(&ts->client->dev, "touchscreen-inverted-y");
    dev_dbg(&ts->client->dev, "touchscreen-inverted-y %u", ts->inverted_y);

    return 0;
}

/**
 * pimhyp3_request_input_dev - Allocate, populate and register the input device
 * @ts: our pimhyp3_ts_data pointer
 * Must be called during probe
 */
static int pimhyp3_request_input_dev(struct pimhyp3_ts_data *ts)
{
    int error;

    ts->input_dev = devm_input_allocate_device(&ts->client->dev);
    if (!ts->input_dev) {
        dev_err(&ts->client->dev, "Failed to allocate input device.");
        return -ENOMEM;
    }

    /* Set up device parameters */
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X,
                 0, ts->abs_x_max, 0, 0);
    input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y,
                 0, ts->abs_y_max, 0, 0);

    input_mt_init_slots(ts->input_dev, ts->max_touch_num,
                INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);

    ts->input_dev->name = "Cypress Capacitive TouchScreen";
    ts->input_dev->phys = "input/ts";
    ts->input_dev->id.bustype = BUS_I2C;
    ts->input_dev->id.vendor = 0x0416;
    ts->input_dev->id.product = ts->id;
    ts->input_dev->id.version = ts->version;

    error = input_register_device(ts->input_dev);
    if (error) {
        dev_err(&ts->client->dev,
        "Failed to register input device: %d", error);
    return error;
    }

    return 0;
}


static int pimhyp3_ts_probe(struct i2c_client *client,
               const struct i2c_device_id *id)
{
    struct pimhyp3_ts_data *ts;
    int error;
    u8 buf;

    dev_dbg(&client->dev, "I2C Address: 0x%02x\n", client->addr);

    if (!i2c_check_functionality(client->adapter, I2C_FUNC_I2C)) {
        dev_err(&client->dev, "I2C check functionality failed.\n");
        return -ENXIO;
    }

    ts = devm_kzalloc(&client->dev, sizeof(*ts), GFP_KERNEL);
    if (!ts)
        return -ENOMEM;

    ts->client = client;
    i2c_set_clientdata(client, ts);


    error = pimhyp3_i2c_test(client);
    if (error) {
        dev_err(&client->dev, "I2C communication failure: %d\n", error);
        return error;
    }

    //Device is detected!
    pr_info("Detected: pimhyp3");
    if (device_property_read_bool(&ts->client->dev,"touchscreen-check"))
    {
        //We have notified the device is fitted, but we don't want to use it
        pr_info("Device check only. Exiting pimhyp3");
        return -EBUSY;
    }

    pimhyp3_get_gpio_config(ts);

    if (ts->use_poll)
        buf = 0b00001110; //En_Int, Int_POLL, INT_MODE1, INT_MODE2
    else
        buf = 0b00001100; //En_Int, Int_POLL, INT_MODE1, INT_MODE2
                            //For polling,  use 00001110
                            //for interrupt use 00001100
    pimhyp3_i2c_write(client, PIMHYP3_INT_SETTINGS, &buf,1);

    lcd_init(ts);

    pimhyp3_request_input_dev(ts);

    dev_dbg(&client->dev, "Cypress cy8c20466 driver installed\n");

    if (ts->use_poll)
    {
        /* create thread that polls the touch events */
        dev_dbg( &ts->client->dev, "Setting up POLL thread");
        ts->thread = kthread_run(pimhyp3_thread, ts, "cy82466?");
        if (ts->thread == NULL) {
            dev_err(&client->dev, "Failed to create kernel thread");
            error = -ENOMEM;
            return error;
        }
    }
    else
    {
        /* Enable Interrupt to handle touch events */
        dev_dbg( &ts->client->dev, "Setting up IRQ");
	    ts->irq_flags = pimhyp3_irq_flags[ts->int_trigger_type] | IRQF_ONESHOT;
	    error = pimhyp3_request_irq(ts);
	    if (error) {
		    dev_err(&ts->client->dev, "request IRQ failed: %d\n", error);
		    return error;
	    }
    }

    return 0;
}

/////////////////////////////// ACPI ///////////////////////////////////////


/////////////////////////////// DEVICE DRIVER DATA /////////////////////////

static const struct i2c_device_id pimhyp3_ts_id[] = {
    { "cy8c20466:00", 0 },
    { }
};
MODULE_DEVICE_TABLE(i2c, pimhyp3_ts_id);

#ifdef CONFIG_OF
static const struct of_device_id pimhyp3_of_match[] = {
    { .compatible = "pimoroni,cy8c20466" },
    { }
};
MODULE_DEVICE_TABLE(of, pimhyp3_of_match);
#endif

static struct i2c_driver pimhyp3_ts_driver = {
    .probe = pimhyp3_ts_probe,
    .remove = pimhyp3_ts_remove,
    .id_table = pimhyp3_ts_id,
    .driver = {
        .name = "pimhyp3-TS",
        .of_match_table = of_match_ptr(pimhyp3_of_match),
    },
};
module_i2c_driver(pimhyp3_ts_driver);

MODULE_AUTHOR("procount <kevin.procount@googlemail.com>");
MODULE_DESCRIPTION("Pimoroni Hyperpixel 3.5 touchscreen driver");
MODULE_LICENSE("GPL v2");

