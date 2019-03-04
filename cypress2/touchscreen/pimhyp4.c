/*
 *  Driver for Pimoroni Hyperpixel 4.0" Touchscreen LCD
 *  Includes driver for Goodix Touchscreens
 *  Modified for writing X2Y, width and height for screens without rst/int pins
 *
 *  Adds commands to program the LCD, from Pimoroni' hyperpixel4-init.c
 *
 *  Copyright (c) 2014 Red Hat Inc.
 *  Copyright (c) 2015 K. Merker <merker@debian.org>
 *  Copyright (c) 2019 ProCount
 *
 *  This code is based on gt9xx.c authored by andrew@goodix.com:
 *  and goodix.c
 *  2010 - 2012 Goodix Technology.
 */

//#define DEBUG

/*
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; version 2 of the License.
 */

#include <linux/kernel.h>
#include <linux/dmi.h>
#include <linux/firmware.h>
#include <linux/gpio/consumer.h>
#include <linux/i2c.h>
#include <linux/input.h>
#include <linux/input/mt.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/slab.h>
#include <linux/acpi.h>
#include <linux/of.h>
#include <asm/unaligned.h>

struct pimhyp4_ts_data {
	struct i2c_client *client;
	struct input_dev *input_dev;
	int abs_x_max;
	int abs_y_max;
	bool swapped_x_y;
	bool inverted_x;
	bool inverted_y;
	unsigned int max_touch_num;
	unsigned int int_trigger_type;
	int cfg_len;
	struct gpio_desc *gpiod_int;
	struct gpio_desc *gpiod_rst;
	struct gpio_desc *gpiod_mosi;
	struct gpio_desc *gpiod_cs;
	u16 id;
	u16 version;
	const char *cfg_name;
	struct completion firmware_loading_complete;
	unsigned long irq_flags;

	bool X2Y;
	bool requestedX2Y;
	u16  requestedXSize;
	u16  requestedYSize;
	u8	 requestedRefreshRate;
};

#define uDELAY 100 // clock pulse time in microseconds
#define mWAIT  120  // wait time in milliseconds


#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

#define PIMHYP4_GPIO_INT_NAME		"irq"
#define PIMHYP4_GPIO_RST_NAME		"reset"
#define PIMHYP4_GPIO_MOSI_NAME       "mosi"
#define PIMHYP4_GPIO_CS_NAME         "cs"

#define PIMHYP4_MAX_HEIGHT		4096
#define PIMHYP4_MAX_WIDTH		4096
#define PIMHYP4_INT_TRIGGER		1
#define PIMHYP4_CONTACT_SIZE		8
#define PIMHYP4_MAX_CONTACTS		10

#define PIMHYP4_CONFIG_MAX_LENGTH	240
#define PIMHYP4_CONFIG_911_LENGTH	186
#define PIMHYP4_CONFIG_967_LENGTH	228

/* Register defines */
#define PIMHYP4_REG_COMMAND		0x8040
#define PIMHYP4_CMD_SCREEN_OFF		0x05

#define PIMHYP4_READ_COOR_ADDR		0x814E
#define PIMHYP4_REG_CONFIG_DATA		0x8047
#define PIMHYP4_REG_ID			0x8140

#define PIMHYP4_BUFFER_STATUS_READY	BIT(7)
#define PIMHYP4_BUFFER_STATUS_TIMEOUT	20

#define RESOLUTION_LOC		1
#define RESOLUTION_LOC_X	1
#define RESOLUTION_LOC_Y	3
#define MAX_CONTACTS_LOC	5
#define TRIGGER_LOC		6

static const unsigned long pimhyp4_irq_flags[] = {
	IRQ_TYPE_EDGE_RISING,
	IRQ_TYPE_EDGE_FALLING,
	IRQ_TYPE_LEVEL_LOW,
	IRQ_TYPE_LEVEL_HIGH,
};

static int checkFirmware(struct pimhyp4_ts_data *ts, u8 *config);


/**
 * pimhyp4_i2c_read - read data from a register of the i2c slave device.
 *
 * @client: i2c device.
 * @reg: the register to read from.
 * @buf: raw write data buffer.
 * @len: length of the buffer to write
 */
static int pimhyp4_i2c_read(struct i2c_client *client,
			   u16 reg, u8 *buf, int len)
{
	struct i2c_msg msgs[2];
	u16 wbuf = cpu_to_be16(reg);
	int ret;

	msgs[0].flags = 0;
	msgs[0].addr  = client->addr;
	msgs[0].len   = 2;
	msgs[0].buf   = (u8 *)&wbuf;

	msgs[1].flags = I2C_M_RD;
	msgs[1].addr  = client->addr;
	msgs[1].len   = len;
	msgs[1].buf   = buf;

	ret = i2c_transfer(client->adapter, msgs, 2);
	return ret < 0 ? ret : (ret != ARRAY_SIZE(msgs) ? -EIO : 0);
}

/**
 * pimhyp4_i2c_write - write data to a register of the i2c slave device.
 *
 * @client: i2c device.
 * @reg: the register to write to.
 * @buf: raw data buffer to write.
 * @len: length of the buffer to write
 */
static int pimhyp4_i2c_write(struct i2c_client *client, u16 reg, const u8 *buf,
			    unsigned len)
{
	u8 *addr_buf;
	struct i2c_msg msg;
	int ret;

	addr_buf = kmalloc(len + 2, GFP_KERNEL);
	if (!addr_buf)
		return -ENOMEM;

	addr_buf[0] = reg >> 8;
	addr_buf[1] = reg & 0xFF;
	memcpy(&addr_buf[2], buf, len);

	msg.flags = 0;
	msg.addr = client->addr;
	msg.buf = addr_buf;
	msg.len = len + 2;

	ret = i2c_transfer(client->adapter, &msg, 1);
	kfree(addr_buf);
	return ret < 0 ? ret : (ret != 1 ? -EIO : 0);
}

static int pimhyp4_i2c_write_u8(struct i2c_client *client, u16 reg, u8 value)
{
	return pimhyp4_i2c_write(client, reg, &value, sizeof(value));
}


static int pimhyp4_get_cfg_len(u16 id)
{
	switch (id) {
	case 911:
	case 9271:
	case 9110:
	case 927:
	case 928:
		return PIMHYP4_CONFIG_911_LENGTH;

	case 912:
	case 967:
		return PIMHYP4_CONFIG_967_LENGTH;

	default:
		return PIMHYP4_CONFIG_MAX_LENGTH;
	}
}

static int pimhyp4_ts_read_input_report(struct pimhyp4_ts_data *ts, u8 *data)
{
	unsigned long max_timeout;
	int touch_num;
	int error;

	/*
	 * The 'buffer status' bit, which indicates that the data is valid, is
	 * not set as soon as the interrupt is raised, but slightly after.
	 * This takes around 10 ms to happen, so we poll for 20 ms.
	 */
	max_timeout = jiffies + msecs_to_jiffies(PIMHYP4_BUFFER_STATUS_TIMEOUT);
	do {
		error = pimhyp4_i2c_read(ts->client, PIMHYP4_READ_COOR_ADDR,
					data, PIMHYP4_CONTACT_SIZE + 1);
		if (error) {
			dev_err(&ts->client->dev, "I2C transfer error: %d\n",
					error);
			return error;
		}

		if (data[0] & PIMHYP4_BUFFER_STATUS_READY) {
			touch_num = data[0] & 0x0f;
			if (touch_num > ts->max_touch_num)
				return -EPROTO;

			if (touch_num > 1) {
				data += 1 + PIMHYP4_CONTACT_SIZE;
				error = pimhyp4_i2c_read(ts->client,
						PIMHYP4_READ_COOR_ADDR +
							1 + PIMHYP4_CONTACT_SIZE,
						data,
						PIMHYP4_CONTACT_SIZE *
							(touch_num - 1));
				if (error)
					return error;
			}

			return touch_num;
		}

		usleep_range(1000, 2000); /* Poll every 1 - 2 ms */
	} while (time_before(jiffies, max_timeout));

	/*
	 * The Goodix panel will send spurious interrupts after a
	 * 'finger up' event, which will always cause a timeout.
	 */
	return 0;
}

static void pimhyp4_ts_report_touch(struct pimhyp4_ts_data *ts, u8 *coor_data)
{
	int id = coor_data[0] & 0x0F;
	int input_x = get_unaligned_le16(&coor_data[1]);
	int input_y = get_unaligned_le16(&coor_data[3]);
	int input_w = get_unaligned_le16(&coor_data[5]);

	if(ts->X2Y)
	{
		/* Inversions have to happen before axis swapping */
		if (ts->inverted_x)
			input_x = ts->abs_x_max - input_x;
		if (ts->inverted_y)
			input_y = ts->abs_y_max - input_y;
		if (ts->swapped_x_y)
			swap(input_x, input_y);
	}
	else
	{
		/* Inversions have to happen after axis swapping */
		if (ts->swapped_x_y)
			swap(input_x, input_y);
		if (ts->inverted_x)
			input_x = ts->abs_x_max - input_x;
		if (ts->inverted_y)
			input_y = ts->abs_y_max - input_y;
	}

	input_mt_slot(ts->input_dev, id);
	input_mt_report_slot_state(ts->input_dev, MT_TOOL_FINGER, true);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_X, input_x);
	input_report_abs(ts->input_dev, ABS_MT_POSITION_Y, input_y);
	input_report_abs(ts->input_dev, ABS_MT_TOUCH_MAJOR, input_w);
	input_report_abs(ts->input_dev, ABS_MT_WIDTH_MAJOR, input_w);


}

/**
 * pimhyp4_process_events - Process incoming events
 *
 * @ts: our pimhyp4_ts_data pointer
 *
 * Called when the IRQ is triggered. Read the current device state, and push
 * the input events to the user space.
 */
static void pimhyp4_process_events(struct pimhyp4_ts_data *ts)
{
	u8  point_data[1 + PIMHYP4_CONTACT_SIZE * PIMHYP4_MAX_CONTACTS];
	int touch_num;
	int i;

	touch_num = pimhyp4_ts_read_input_report(ts, point_data);
	if (touch_num < 0)
		return;

	/*
	 * Bit 4 of the first byte reports the status of the capacitive
	 * Windows/Home button.
	 */
	input_report_key(ts->input_dev, KEY_LEFTMETA, point_data[0] & BIT(4));

	for (i = 0; i < touch_num; i++)
		pimhyp4_ts_report_touch(ts,
				&point_data[1 + PIMHYP4_CONTACT_SIZE * i]);

	input_mt_sync_frame(ts->input_dev);
	input_sync(ts->input_dev);
}

/**
 * pimhyp4_ts_irq_handler - The IRQ handler
 *
 * @irq: interrupt number.
 * @dev_id: private data pointer.
 */
static irqreturn_t pimhyp4_ts_irq_handler(int irq, void *dev_id)
{
	struct pimhyp4_ts_data *ts = dev_id;


	pimhyp4_process_events(ts);

	if (pimhyp4_i2c_write_u8(ts->client, PIMHYP4_READ_COOR_ADDR, 0) < 0)
		dev_err(&ts->client->dev, "I2C write end_cmd error\n");

	return IRQ_HANDLED;
}

static void pimhyp4_free_irq(struct pimhyp4_ts_data *ts)
{
	devm_free_irq(&ts->client->dev, ts->client->irq, ts);
}

static int pimhyp4_request_irq(struct pimhyp4_ts_data *ts)
{
	return devm_request_threaded_irq(&ts->client->dev, ts->client->irq,
					 NULL, pimhyp4_ts_irq_handler,
					 ts->irq_flags, ts->client->name, ts);
}

/**
 * pimhyp4_check_cfg - Checks if config fw is valid
 *
 * @ts: pimhyp4_ts_data pointer
 * @cfg: firmware config data
 */
static int pimhyp4_check_cfg(struct pimhyp4_ts_data *ts,
			    const struct firmware *cfg)
{
	int i, raw_cfg_len;
	u8 check_sum = 0;

	if (cfg->size > PIMHYP4_CONFIG_MAX_LENGTH) {
		dev_err(&ts->client->dev,
			"The length of the config fw is not correct");
		return -EINVAL;
	}

	raw_cfg_len = cfg->size - 2;
	for (i = 0; i < raw_cfg_len; i++)
		check_sum += cfg->data[i];
	check_sum = (~check_sum) + 1;
	if (check_sum != cfg->data[raw_cfg_len]) {
		dev_err(&ts->client->dev,
			"The checksum of the config fw is not correct");
		return -EINVAL;
	}

	if (cfg->data[raw_cfg_len + 1] != 1) {
		dev_err(&ts->client->dev,
			"Config fw must have Config_Fresh register set");
		return -EINVAL;
	}

	return 0;
}

/**
 * pimhyp4_send_cfg - Write fw config to device
 *
 * @ts: pimhyp4_ts_data pointer
 * @cfg: config firmware to write to device
 */
static int pimhyp4_send_cfg(struct pimhyp4_ts_data *ts,
			   const struct firmware *cfg)
{
	int error;

	error = pimhyp4_check_cfg(ts, cfg);
	if (error)
		return error;

	error = pimhyp4_i2c_write(ts->client, PIMHYP4_REG_CONFIG_DATA, cfg->data,
				 cfg->size);
	if (error) {
		dev_err(&ts->client->dev, "Failed to write config data: %d",
			error);
		return error;
	}
	dev_dbg(&ts->client->dev, "Config sent successfully.");

	/* Let the firmware reconfigure itself, so sleep for 10ms */
	usleep_range(10000, 11000);

	return 0;
}

static int pimhyp4_int_sync(struct pimhyp4_ts_data *ts)
{
	int error;

	error = gpiod_direction_output(ts->gpiod_int, 0);
	if (error)
		return error;

	msleep(50);				/* T5: 50ms */

	error = gpiod_direction_input(ts->gpiod_int);
	if (error)
		return error;

	return 0;
}

/////////////////////////////// INITIALISATION /////////////////////////////


static int32_t commands[] = {
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x101, 0x008, 0x110,
    0x021, 0x109, 0x030, 0x102, 0x031, 0x100, 0x040, 0x110,
    0x041, 0x155, 0x042, 0x102, 0x043, 0x109, 0x044, 0x107,
    0x050, 0x178, 0x051, 0x178, 0x052, 0x100, 0x053, 0x16d,
    0x060, 0x107, 0x061, 0x100, 0x062, 0x108, 0x063, 0x100,
    0x0a0, 0x100, 0x0a1, 0x107, 0x0a2, 0x10c, 0x0a3, 0x10b,
    0x0a4, 0x103, 0x0a5, 0x107, 0x0a6, 0x106, 0x0a7, 0x104,
    0x0a8, 0x108, 0x0a9, 0x10c, 0x0aa, 0x113, 0x0ab, 0x106,
    0x0ac, 0x10d, 0x0ad, 0x119, 0x0ae, 0x110, 0x0af, 0x100,
    0x0c0, 0x100, 0x0c1, 0x107, 0x0c2, 0x10c, 0x0c3, 0x10b,
    0x0c4, 0x103, 0x0c5, 0x107, 0x0c6, 0x107, 0x0c7, 0x104,
    0x0c8, 0x108, 0x0c9, 0x10c, 0x0ca, 0x113, 0x0cb, 0x106,
    0x0cc, 0x10d, 0x0cd, 0x118, 0x0ce, 0x110, 0x0cf, 0x100,
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x106, 0x000, 0x120,
    0x001, 0x10a, 0x002, 0x100, 0x003, 0x100, 0x004, 0x101,
    0x005, 0x101, 0x006, 0x198, 0x007, 0x106, 0x008, 0x101,
    0x009, 0x180, 0x00a, 0x100, 0x00b, 0x100, 0x00c, 0x101,
    0x00d, 0x101, 0x00e, 0x100, 0x00f, 0x100, 0x010, 0x1f0,
    0x011, 0x1f4, 0x012, 0x101, 0x013, 0x100, 0x014, 0x100,
    0x015, 0x1c0, 0x016, 0x108, 0x017, 0x100, 0x018, 0x100,
    0x019, 0x100, 0x01a, 0x100, 0x01b, 0x100, 0x01c, 0x100,
    0x01d, 0x100, 0x020, 0x101, 0x021, 0x123, 0x022, 0x145,
    0x023, 0x167, 0x024, 0x101, 0x025, 0x123, 0x026, 0x145,
    0x027, 0x167, 0x030, 0x111, 0x031, 0x111, 0x032, 0x100,
    0x033, 0x1ee, 0x034, 0x1ff, 0x035, 0x1bb, 0x036, 0x1aa,
    0x037, 0x1dd, 0x038, 0x1cc, 0x039, 0x166, 0x03a, 0x177,
    0x03b, 0x122, 0x03c, 0x122, 0x03d, 0x122, 0x03e, 0x122,
    0x03f, 0x122, 0x040, 0x122, 0x052, 0x110, 0x053, 0x110,
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x107, 0x018, 0x11d,
    0x017, 0x122, 0x002, 0x177, 0x026, 0x1b2, 0x0e1, 0x179,
    0x0ff, 0x1ff, 0x198, 0x106, 0x104, 0x100, 0x03a, 0x160,
    0x035, 0x100, 0x011, 0x100,    -1, 0x029, 0x100,    -1
};


static void send_bits(struct pimhyp4_ts_data *ts, uint16_t data, uint16_t count)
{
    int x;
    int mask = 1 << (count-1);
    for(x = 0; x < count; x++){
        gpiod_set_value(ts->gpiod_mosi,(data & mask) > 0);
        //bcm2835_gpio_write(MOSI, (data & mask) > 0);
        data <<= 1;

        gpiod_set_value(ts->gpiod_int, 0);
        //bcm2835_gpio_write(CLK, LOW);

        udelay(uDELAY);
        //bcm2835_delayMicroseconds(DELAY);

        gpiod_set_value(ts->gpiod_int, 1);
        //bcm2835_gpio_write(CLK, HIGH);

        udelay(uDELAY);
        //bcm2835_delayMicroseconds(DELAY);
    }
    gpiod_set_value(ts->gpiod_mosi,0);
    //bcm2835_gpio_write(MOSI, LOW);
}

static void write(struct pimhyp4_ts_data *ts, uint16_t command)
{
    gpiod_set_value(ts->gpiod_cs,0);
    //bcm2835_gpio_write(CS, LOW);

    send_bits(ts, command, 9);

    gpiod_set_value(ts->gpiod_cs,1);
    //bcm2835_gpio_write(CS, HIGH);
}

static void lcd_init(struct pimhyp4_ts_data *ts)
{
#if 1
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
    //bcm2835_gpio_set_pud(CLK, BCM2835_GPIO_PUD_OFF);
#endif
}

////////////////////////////////////////////////////

/**
 * pimhyp4_reset - Reset device during power on
 *
 * @ts: pimhyp4_ts_data pointer
 */
static int pimhyp4_reset(struct pimhyp4_ts_data *ts)
{
	int error;

	/* begin select I2C slave addr */
	error = gpiod_direction_output(ts->gpiod_rst, 0);
	if (error)
		return error;

	msleep(20);				/* T2: > 10ms */

	/* HIGH: 0x28/0x29, LOW: 0xBA/0xBB */
	error = gpiod_direction_output(ts->gpiod_int, ts->client->addr == 0x14);
	if (error)
		return error;

	usleep_range(100, 2000);		/* T3: > 100us */

	error = gpiod_direction_output(ts->gpiod_rst, 1);
	if (error)
		return error;

	usleep_range(6000, 10000);		/* T4: > 5ms */

	/* end select I2C slave addr */
	error = gpiod_direction_input(ts->gpiod_rst);
	if (error)
		return error;

	error = pimhyp4_int_sync(ts);
	if (error)
		return error;

	return 0;
}

/**
 * pimhyp4_get_gpio_config - Get GPIO config from ACPI/DT
 *
 * @ts: pimhyp4_ts_data pointer
 */
static int pimhyp4_get_gpio_config(struct pimhyp4_ts_data *ts)
{
	int error;
	struct device *dev;
	struct gpio_desc *gpiod;

	if (!ts->client)
		return -EINVAL;
	dev = &ts->client->dev;

	/* Get the interrupt GPIO pin number */
	gpiod = devm_gpiod_get_optional(dev, PIMHYP4_GPIO_INT_NAME, GPIOD_IN);
	if (IS_ERR(gpiod)) {
		error = PTR_ERR(gpiod);
		if (error != -EPROBE_DEFER)
			dev_dbg(dev, "Failed to get %s GPIO: %d\n",
				PIMHYP4_GPIO_INT_NAME, error);
		return error;
	}

	ts->gpiod_int = gpiod;

	/* Get the reset line GPIO pin number */
	gpiod = devm_gpiod_get_optional(dev, PIMHYP4_GPIO_RST_NAME, GPIOD_IN);
	if (IS_ERR(gpiod)) {
		error = PTR_ERR(gpiod);
		if (error != -EPROBE_DEFER)
			dev_dbg(dev, "Failed to get %s GPIO: %d\n",
				PIMHYP4_GPIO_RST_NAME, error);
		return error;
	}

	ts->gpiod_rst = gpiod;

    /* Ensure the correct direction is set because this pin is also used to
     * program the LCD controller
     */
    gpiod_direction_input(ts->gpiod_int);

    /* Get the MOSI GPIO pin description */
    gpiod = devm_gpiod_get(dev, PIMHYP4_GPIO_MOSI_NAME, GPIOD_OUT_LOW);
    if (IS_ERR(gpiod)) {
        error = PTR_ERR(gpiod);
        if (error != -EPROBE_DEFER)
            dev_dbg(dev, "Failed to get %s GPIO: %d\n",
                PIMHYP4_GPIO_MOSI_NAME, error);
        return error;
    }

    ts->gpiod_mosi = gpiod;

    /* Get the CS GPIO pin description */
    gpiod = devm_gpiod_get(dev, PIMHYP4_GPIO_CS_NAME, GPIOD_OUT_HIGH);
    if (IS_ERR(gpiod)) {
        error = PTR_ERR(gpiod);
        if (error != -EPROBE_DEFER)
            dev_dbg(dev, "Failed to get %s GPIO: %d\n",
                PIMHYP4_GPIO_CS_NAME, error);
        return error;
    }

    ts->gpiod_cs = gpiod;

	return 0;
}

/**
 * pimhyp4_read_config - Read the embedded configuration of the panel
 *
 * @ts: our pimhyp4_ts_data pointer
 *
 * Must be called during probe
 */
static void pimhyp4_read_config(struct pimhyp4_ts_data *ts)
{
	u8 config[PIMHYP4_CONFIG_MAX_LENGTH];
	int error;

	error = pimhyp4_i2c_read(ts->client, PIMHYP4_REG_CONFIG_DATA,
				config, ts->cfg_len);
	if (error) {
		dev_warn(&ts->client->dev,
			 "Error reading config (%d), using defaults\n",
			 error);
		ts->abs_x_max = PIMHYP4_MAX_WIDTH;
		ts->abs_y_max = PIMHYP4_MAX_HEIGHT;
		if (ts->swapped_x_y)
			swap(ts->abs_x_max, ts->abs_y_max);
		ts->int_trigger_type = PIMHYP4_INT_TRIGGER;
		ts->max_touch_num = PIMHYP4_MAX_CONTACTS;
		return;
	}

	dev_dbg(&ts->client->dev, "Current size = (%u, %u), X2Y = %u, refresh = %u", ts->requestedXSize, ts->requestedYSize, ts->X2Y, ts->requestedRefreshRate);

	checkFirmware(ts, config);

	ts->abs_x_max = get_unaligned_le16(&config[RESOLUTION_LOC]);
	ts->abs_y_max = get_unaligned_le16(&config[RESOLUTION_LOC + 2]);
	ts->X2Y = CHECK_BIT(config[0x804d - PIMHYP4_REG_CONFIG_DATA], 3);

	dev_dbg(&ts->client->dev, "Current size = (%u, %u), X2Y = %u, refresh = %u", ts->requestedXSize, ts->requestedYSize, ts->X2Y, ts->requestedRefreshRate);

	if (ts->swapped_x_y)
		swap(ts->abs_x_max, ts->abs_y_max);
	ts->int_trigger_type = config[TRIGGER_LOC] & 0x03;
	ts->max_touch_num = config[MAX_CONTACTS_LOC] & 0x0f;
	if (!ts->abs_x_max || !ts->abs_y_max || !ts->max_touch_num) {
		dev_err(&ts->client->dev,
			"Invalid config, using defaults\n");
		ts->abs_x_max = PIMHYP4_MAX_WIDTH;
		ts->abs_y_max = PIMHYP4_MAX_HEIGHT;
		if (ts->swapped_x_y)
			swap(ts->abs_x_max, ts->abs_y_max);
		ts->max_touch_num = PIMHYP4_MAX_CONTACTS;
	}

}

/**
 * pimhyp4_read_version - Read goodix touchscreen version
 *
 * @ts: our pimhyp4_ts_data pointer
 */
static int pimhyp4_read_version(struct pimhyp4_ts_data *ts)
{
	int error;
	u8 buf[6];
	char id_str[5];

	error = pimhyp4_i2c_read(ts->client, PIMHYP4_REG_ID, buf, sizeof(buf));
	if (error) {
		dev_err(&ts->client->dev, "read version failed: %d\n", error);
		return error;
	}

	memcpy(id_str, buf, 4);
	id_str[4] = 0;
	if (kstrtou16(id_str, 10, &ts->id))
		ts->id = 0x1001;

	ts->version = get_unaligned_le16(&buf[4]);

	dev_info(&ts->client->dev, "ID %d, version: %04x\n", ts->id,
		 ts->version);

	return 0;
}

/**
 * pimhyp4_i2c_test - I2C test function to check if the device answers.
 *
 * @client: the i2c client
 */
static int pimhyp4_i2c_test(struct i2c_client *client)
{
	int retry = 0;
	int error;
	u8 test;

	while (retry++ < 2) {
		error = pimhyp4_i2c_read(client, PIMHYP4_REG_CONFIG_DATA,
					&test, 1);
		if (!error)
			return 0;

		dev_err(&client->dev, "i2c test failed attempt %d: %d\n",
			retry, error);
		msleep(20);
	}

	return error;
}

/**
 * pimhyp4_request_input_dev - Allocate, populate and register the input device
 *
 * @ts: our pimhyp4_ts_data pointer
 *
 * Must be called during probe
 */
static int pimhyp4_request_input_dev(struct pimhyp4_ts_data *ts)
{
	int error;

	ts->input_dev = devm_input_allocate_device(&ts->client->dev);
	if (!ts->input_dev) {
		dev_err(&ts->client->dev, "Failed to allocate input device.");
		return -ENOMEM;
	}

	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_X,
			     0, ts->abs_x_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_POSITION_Y,
			     0, ts->abs_y_max, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_WIDTH_MAJOR, 0, 255, 0, 0);
	input_set_abs_params(ts->input_dev, ABS_MT_TOUCH_MAJOR, 0, 255, 0, 0);

	input_mt_init_slots(ts->input_dev, ts->max_touch_num,
			    INPUT_MT_DIRECT | INPUT_MT_DROP_UNUSED);

	ts->input_dev->name = "Pimoroni Hyperpixel4 Capacitive TouchScreen";
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

/**
 * pimhyp4_configure_dev - Finish device initialization
 *
 * @ts: our pimhyp4_ts_data pointer
 *
 * Must be called from probe to finish initialization of the device.
 * Contains the common initialization code for both devices that
 * declare gpio pins and devices that do not. It is either called
 * directly from probe or from request_firmware_wait callback.
 */
static int pimhyp4_configure_dev(struct pimhyp4_ts_data *ts)
{
	int error;
	u32 sizeX, sizeY, refreshRate;

	// X2Y setting
	ts->requestedX2Y = device_property_read_bool(&ts->client->dev,
								"touchscreen-x2y");

	if(device_property_read_u32(&ts->client->dev, "touchscreen-refresh-rate", &refreshRate))
	{
		dev_dbg(&ts->client->dev, "touchscreen-refresh-rate not found");
		ts->requestedRefreshRate = 5;
	}
	else
	{
		dev_dbg(&ts->client->dev, "touchscreen-refresh-rate found %u", sizeX);
		ts->requestedRefreshRate = (u8)refreshRate;
	}

	if(device_property_read_u32(&ts->client->dev, "touchscreen-size-x", &sizeX))
	{
		dev_dbg(&ts->client->dev, "touchscreen-size-x not found");
		ts->requestedXSize = 0;
	}
	else
	{
		dev_dbg(&ts->client->dev, "touchscreen-size-x found %u", sizeX);
		ts->requestedXSize = (u16)sizeX;
	}

	if(device_property_read_u32(&ts->client->dev, "touchscreen-size-y", &sizeY))
	{
		dev_dbg(&ts->client->dev, "touchscreen-size-y not found");
		ts->requestedYSize = 0;
	}
	else
	{
		dev_dbg(&ts->client->dev, "touchscreen-size-y found %u", sizeY);
		ts->requestedYSize = (u16)sizeY;
	}

	dev_dbg(&ts->client->dev, "requested size (%u, %u)", ts->requestedXSize, ts->requestedYSize);

	ts->swapped_x_y = device_property_read_bool(&ts->client->dev,
						    "touchscreen-swapped-x-y");
	ts->inverted_x = device_property_read_bool(&ts->client->dev,
						   "touchscreen-inverted-x");
	ts->inverted_y = device_property_read_bool(&ts->client->dev,
						   "touchscreen-inverted-y");

	pimhyp4_read_config(ts);

	error = pimhyp4_request_input_dev(ts);
	if (error)
		return error;

	ts->irq_flags = pimhyp4_irq_flags[ts->int_trigger_type] | IRQF_ONESHOT;
	error = pimhyp4_request_irq(ts);
	if (error) {
		dev_err(&ts->client->dev, "request IRQ failed: %d\n", error);
		return error;
	}

	return 0;
}

/**
 * pimhyp4_config_cb - Callback to finish device init
 *
 * @ts: our pimhyp4_ts_data pointer
 *
 * request_firmware_wait callback that finishes
 * initialization of the device.
 */
static void pimhyp4_config_cb(const struct firmware *cfg, void *ctx)
{
	struct pimhyp4_ts_data *ts = ctx;
	int error;

	if (cfg) {
		/* send device configuration to the firmware */
		error = pimhyp4_send_cfg(ts, cfg);
		if (error)
			goto err_release_cfg;
	}

	pimhyp4_configure_dev(ts);

err_release_cfg:
	release_firmware(cfg);
	complete_all(&ts->firmware_loading_complete);
}

static int pimhyp4_ts_probe(struct i2c_client *client,
			   const struct i2c_device_id *id)
{
	struct pimhyp4_ts_data *ts;
	int error;

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
	init_completion(&ts->firmware_loading_complete);

	error = pimhyp4_get_gpio_config(ts);
	if (error)
		return error;

	if (ts->gpiod_int && ts->gpiod_rst) {
		/* reset the controller */
		error = pimhyp4_reset(ts);
		if (error) {
			dev_err(&client->dev, "Controller reset failed.\n");
			return error;
		}
	}

	error = pimhyp4_i2c_test(client);
	if (error) {
		dev_err(&client->dev, "I2C communication failure: %d\n", error);
		return error;
	}

	error = pimhyp4_read_version(ts);
	if (error) {
		dev_err(&client->dev, "Read version failed.\n");
		return error;
	}

	//Device is detected!
	pr_info("Detected: pimhyp4");
	if (device_property_read_bool(&ts->client->dev,"touchscreen-check"))
	{
		//We have notified the device is fitted, but we don't want to use it
		pr_info("Device check only. Exiting pimhyp4");
		return -EBUSY;
	}



	ts->cfg_len = pimhyp4_get_cfg_len(ts->id);

	if (ts->gpiod_int && ts->gpiod_rst) {
		/* update device config */
		ts->cfg_name = devm_kasprintf(&client->dev, GFP_KERNEL,
					      "goodix_%d_cfg.bin", ts->id);
		if (!ts->cfg_name)
			return -ENOMEM;

		error = request_firmware_nowait(THIS_MODULE, true, ts->cfg_name,
						&client->dev, GFP_KERNEL, ts,
						pimhyp4_config_cb);
		if (error) {
			dev_err(&client->dev,
				"Failed to invoke firmware loader: %d\n",
				error);
			return error;
		}

		return 0;
	} else {
		error = pimhyp4_configure_dev(ts);
		if (error)
			return error;
	}

    lcd_init(ts);

	return 0;
}

static int pimhyp4_ts_remove(struct i2c_client *client)
{
	struct pimhyp4_ts_data *ts = i2c_get_clientdata(client);

	if (ts->gpiod_int && ts->gpiod_rst)
		wait_for_completion(&ts->firmware_loading_complete);

	return 0;
}

static int __maybe_unused pimhyp4_suspend(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct pimhyp4_ts_data *ts = i2c_get_clientdata(client);
	int error;

	/* We need gpio pins to suspend/resume */
	if (!ts->gpiod_int || !ts->gpiod_rst) {
		disable_irq(client->irq);
		return 0;
	}

	wait_for_completion(&ts->firmware_loading_complete);

	/* Free IRQ as IRQ pin is used as output in the suspend sequence */
	pimhyp4_free_irq(ts);

	/* Output LOW on the INT pin for 5 ms */
	error = gpiod_direction_output(ts->gpiod_int, 0);
	if (error) {
		pimhyp4_request_irq(ts);
		return error;
	}

	usleep_range(5000, 6000);

	error = pimhyp4_i2c_write_u8(ts->client, PIMHYP4_REG_COMMAND,
				    PIMHYP4_CMD_SCREEN_OFF);
	if (error) {
		dev_err(&ts->client->dev, "Screen off command failed\n");
		gpiod_direction_input(ts->gpiod_int);
		pimhyp4_request_irq(ts);
		return -EAGAIN;
	}

	/*
	 * The datasheet specifies that the interval between sending screen-off
	 * command and wake-up should be longer than 58 ms. To avoid waking up
	 * sooner, delay 58ms here.
	 */
	msleep(58);
	return 0;
}

static int __maybe_unused pimhyp4_resume(struct device *dev)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct pimhyp4_ts_data *ts = i2c_get_clientdata(client);
	int error;

	if (!ts->gpiod_int || !ts->gpiod_rst) {
		enable_irq(client->irq);
		return 0;
	}

	/*
	 * Exit sleep mode by outputting HIGH level to INT pin
	 * for 2ms~5ms.
	 */
	error = gpiod_direction_output(ts->gpiod_int, 1);
	if (error)
		return error;

	usleep_range(2000, 5000);

	error = pimhyp4_int_sync(ts);
	if (error)
		return error;

	error = pimhyp4_request_irq(ts);
	if (error)
		return error;

	return 0;
}


//0x41,0xe0,0x01,0x20,0x03,0x05,0x08,0x00,
//0x01,0x0a,0x28,0x0a,0x50,0x32,0x03,0x05,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x18,
//0x19,0x1e,0x14,0x87,0x26,0x08,0x43,0x41,
//0x0c,0x08,0x00,0x00,0x01,0x02,0x03,0x1d,
//0x00,0x00,0x00,0x00,0x00,0x03,0x64,0x32,
//0x00,0x00,0x00,0x1e,0x50,0x94,0xc5,0x02,
//0x07,0x00,0x00,0x00,0xb2,0x21,0x00,0xc0,
//0x28,0x00,0x7d,0x31,0x00,0x6b,0x3b,0x00,
//0x5c,0x48,0x00,0x5c,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x02,0x04,0x06,0x08,0x0a,0x0c,0x0e,0x10,
//0xff,0xff,0xff,0xff,0xff,0xff,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
//0x04,0x06,0x08,0x0a,0x0c,0x1d,0x1e,0x1f,
//0x20,0x21,0x22,0xff,0xff,0xff,0xff,0xff,
//0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
//0x7c,0x00


static int checkFirmware(struct pimhyp4_ts_data *ts, u8 *config)
{
	int result = 0;

	bool bUpdate = false;
	bool bNewX2Y = ts->requestedX2Y;
	bool bCurrentX2Y = CHECK_BIT(config[0x804d - PIMHYP4_REG_CONFIG_DATA], 3);

	u8 currentRefreshRate = config[0x8056 - PIMHYP4_REG_CONFIG_DATA];


	dev_info(&ts->client->dev, "Checking firmware");
	if(ts->requestedRefreshRate != currentRefreshRate)
	{
		dev_dbg(&ts->client->dev, "Requested refresh rate = %u, orig = %u", ts->requestedRefreshRate, currentRefreshRate);
		config[0x8056 - PIMHYP4_REG_CONFIG_DATA] =  (config[0x8056 - PIMHYP4_REG_CONFIG_DATA] & 0xF0) +  (ts->requestedRefreshRate & 0x0F);
		bUpdate = true;
	}

	if(ts->requestedXSize)
	{
		u16 xOrig;

		xOrig = get_unaligned_le16(&config[RESOLUTION_LOC_X]);
		dev_dbg(&ts->client->dev, "Requested X size = %u, orig = %u", ts->requestedXSize, xOrig);
		if(ts->requestedXSize != xOrig)
		{
			dev_dbg(&ts->client->dev, "Requested X size needs firmware update");
			put_unaligned_le16(ts->requestedXSize, &config[RESOLUTION_LOC_X]);
			bUpdate = true;
		}
	}

	if(ts->requestedYSize)
	{
		u16 yOrig;

		yOrig = get_unaligned_le16(&config[RESOLUTION_LOC_Y]);
		dev_dbg(&ts->client->dev, "Requested Y size = %u, orig = %u", ts->requestedYSize, yOrig);
		if(ts->requestedYSize != yOrig)
		{
			dev_dbg(&ts->client->dev, "Requested Y size needs firmware update");
			put_unaligned_le16(ts->requestedYSize, &config[RESOLUTION_LOC_Y]);
			bUpdate = true;
		}
	}

	if(bNewX2Y != bCurrentX2Y)
	{
		dev_dbg(&ts->client->dev, "Requested X2Y size needs firmware update");

		if(bNewX2Y)
			config[0x804d - PIMHYP4_REG_CONFIG_DATA] |= 0x08;
		else
			config[0x804d - PIMHYP4_REG_CONFIG_DATA] &= 0xf7;

		bUpdate = true;
	}

	if(bUpdate)
	{
		int error;
		struct firmware fw;
		int check;
		int rawCfgLlen;
		int i;
		u8 checkSum = 0;

		dev_info(&ts->client->dev, "Updating firmware");

		fw.data = config;
		fw.size = PIMHYP4_CONFIG_911_LENGTH;
		config[PIMHYP4_CONFIG_911_LENGTH-1]=1;	// flash fw flag

		// calculate checksum
		rawCfgLlen = PIMHYP4_CONFIG_911_LENGTH- 2;
		for (i = 0; i < rawCfgLlen; i++)
			checkSum += config[i];
		checkSum = (~checkSum) + 1;
		config[rawCfgLlen] = checkSum;

		// check firmware is ok
		check =  pimhyp4_check_cfg(ts,  &fw);

		if(!check)
		{
			error = pimhyp4_i2c_write(ts->client, PIMHYP4_REG_CONFIG_DATA, fw.data, fw.size);
			if (error)
				dev_err(&ts->client->dev, "Failed to write config data: (%d)",	error);
			else
				dev_info(&ts->client->dev, "Updated firmware correctly.");


			/* Let the firmware reconfigure itself, so sleep for 10ms */
			usleep_range(10000, 11000);
		}
		else
		{
			dev_err(&ts->client->dev, "Firmware data is invalid.");
			result = 1;
		}
	}
	else
		dev_dbg(&ts->client->dev, "Firmware already updated with correct values");

	return result;
}


static SIMPLE_DEV_PM_OPS(pimhyp4_pm_ops, pimhyp4_suspend, pimhyp4_resume);

static const struct i2c_device_id pimhyp4_ts_id[] = {
	{ "GDIX1001:00", 0 },
	{ }
};
MODULE_DEVICE_TABLE(i2c, pimhyp4_ts_id);

#ifdef CONFIG_OF
static const struct of_device_id pimhyp4_of_match[] = {
	{ .compatible = "pimoroni,ft6236" },
	{ }
};
MODULE_DEVICE_TABLE(of, pimhyp4_of_match);
#endif

static struct i2c_driver pimhyp4_ts_driver = {
	.probe = pimhyp4_ts_probe,
	.remove = pimhyp4_ts_remove,
	.id_table = pimhyp4_ts_id,
	.driver = {
		.name = "pimhyp4-TS",
		.of_match_table = of_match_ptr(pimhyp4_of_match),
		.pm = &pimhyp4_pm_ops,
	},
};
module_i2c_driver(pimhyp4_ts_driver);

MODULE_AUTHOR("procount <kevin.procount@googlemail.com>");
MODULE_AUTHOR("Benjamin Tissoires <benjamin.tissoires@gmail.com>");
MODULE_AUTHOR("Bastien Nocera <hadess@hadess.net>");
MODULE_DESCRIPTION("pimhyp4 touchscreen driver");
MODULE_LICENSE("GPL v2");
