#!/usr/bin/python

# Somewhat minimal Adafruit Joy Bonnet handler.  Runs in background,
# translates inputs from buttons and ADC joystick to virtual USB keyboard
# events.
# Prerequisites:
# sudo apt-get install python-pip python-smbus python-dev
# sudo pip install evdev
# Be sure to enable I2C via raspi-config.  Also, udev rules will need to
# be set up per retrogame directions.
# Credit to Pimoroni for Picade HAT scripts as starting point.

import time
import signal
import os
import sys
from datetime import datetime

try:
    from evdev import uinput, UInput, ecodes as e
except ImportError:
    exit("This library requires the evdev module\nInstall with: sudo pip install evdev")

try:
    import RPi.GPIO as gpio
except ImportError:
    exit("This library requires the RPi.GPIO module\nInstall with: sudo pip install RPi.GPIO")

try:
     from smbus import SMBus
except ImportError:
    exit("This library requires the smbus module\nInstall with: sudo pip install smbus")

DEBUG = False
BOUNCE_TIME = 0.01 # Debounce time in seconds

BUTTON_A = 12
BUTTON_B = 6
BUTTON_X = 16
BUTTON_Y = 13
SELECT   = 20
START    = 26
PLAYER1  = 23
PLAYER2  = 22
BUTTONS = [BUTTON_A, BUTTON_B, BUTTON_X, BUTTON_Y, SELECT, START, PLAYER1, PLAYER2]

ANALOG_THRESH_NEG = -600
ANALOG_THRESH_POS = 600
analog_states = [False, False, False, False]  # up down left right

KEYS= { # EDIT KEYCODES IN THIS TABLE TO YOUR PREFERENCES:
	# See /usr/include/linux/input.h for keycode names
	# Keyboard        Bonnet        EmulationStation
	BUTTON_A: e.KEY_LEFTCTRL, # 'A' button
	BUTTON_B: e.KEY_LEFTALT,  # 'B' button
	BUTTON_X: e.KEY_Z,        # 'X' button
	BUTTON_Y: e.KEY_X,        # 'Y' button
	SELECT:   e.KEY_SPACE,    # 'Select' button
	START:    e.KEY_ENTER,    # 'Start' button
	PLAYER1:  e.KEY_1,        # '#1' button         
	PLAYER2:  e.KEY_2,        # '#2' button
	1000:     e.KEY_UP,       # Analog up
	1001:     e.KEY_DOWN,     # Analog down
	1002:     e.KEY_LEFT,     # Analog left
	1003:     e.KEY_RIGHT,    # Analog right
}

###################################### ADS1015 microdriver #################################
# Register and other configuration values:
ADS1x15_DEFAULT_ADDRESS        = 0x48
ADS1x15_POINTER_CONVERSION     = 0x00
ADS1x15_POINTER_CONFIG         = 0x01

ADS1015_REG_CONFIG_CQUE_NONE    = 0x0003 # Disable the comparator and put ALERT/RDY in high state (default)
ADS1015_REG_CONFIG_CLAT_NONLAT  = 0x0000 # Non-latching comparator (default)
ADS1015_REG_CONFIG_CPOL_ACTVLOW = 0x0000 # ALERT/RDY pin is low when active (default)
ADS1015_REG_CONFIG_CMODE_TRAD   = 0x0000 # Traditional comparator with hysteresis (default)
ADS1015_REG_CONFIG_DR_1600SPS   = 0x0080 # 1600 samples per second (default)
ADS1015_REG_CONFIG_MODE_SINGLE  = 0x0100 # Power-down single-shot mode (default)
ADS1015_REG_CONFIG_GAIN_ONE     = 0x0200 # gain of 1

ADS1015_REG_CONFIG_MUX_SINGLE_0 = 0x4000 # channel 0
ADS1015_REG_CONFIG_MUX_SINGLE_1 = 0x5000 # channel 1
ADS1015_REG_CONFIG_MUX_SINGLE_2 = 0x6000 # channel 2
ADS1015_REG_CONFIG_MUX_SINGLE_3 = 0x7000 # channel 3

ADS1015_REG_CONFIG_OS_SINGLE    = 0x8000 # start a single conversion

ADS1015_REG_CONFIG_CHANNELS = (ADS1015_REG_CONFIG_MUX_SINGLE_0, ADS1015_REG_CONFIG_MUX_SINGLE_1,
			       ADS1015_REG_CONFIG_MUX_SINGLE_2, ADS1015_REG_CONFIG_MUX_SINGLE_3)

def ads_read(channel):
  #configdata = bus.read_i2c_block_data(ADS1x15_DEFAULT_ADDRESS, ADS1x15_POINTER_CONFIG, 2) 
  #print("Getting config byte = 0x%02X%02X" % (configdata[0], configdata[1]))

  configword = ADS1015_REG_CONFIG_CQUE_NONE | ADS1015_REG_CONFIG_CLAT_NONLAT | ADS1015_REG_CONFIG_CPOL_ACTVLOW | ADS1015_REG_CONFIG_CMODE_TRAD   |  ADS1015_REG_CONFIG_DR_1600SPS | ADS1015_REG_CONFIG_MODE_SINGLE  | ADS1015_REG_CONFIG_GAIN_ONE | ADS1015_REG_CONFIG_CHANNELS[channel] | ADS1015_REG_CONFIG_OS_SINGLE 
  configdata = [configword >> 8, configword & 0xFF]

  #print("Setting config byte = 0x%02X%02X" % (configdata[0], configdata[1]))
  bus.write_i2c_block_data(ADS1x15_DEFAULT_ADDRESS, ADS1x15_POINTER_CONFIG, configdata)

  configdata = bus.read_i2c_block_data(ADS1x15_DEFAULT_ADDRESS, ADS1x15_POINTER_CONFIG, 2) 
  #print("Getting config byte = 0x%02X%02X" % (configdata[0], configdata[1]))

  while True:
     try:
       configdata = bus.read_i2c_block_data(ADS1x15_DEFAULT_ADDRESS, ADS1x15_POINTER_CONFIG, 2) 
       #print("Getting config byte = 0x%02X%02X" % (configdata[0], configdata[1]))
       if (configdata[0] & 0x80):
         break
     except:
       pass
  # read data out!
  analogdata = bus.read_i2c_block_data(ADS1x15_DEFAULT_ADDRESS, ADS1x15_POINTER_CONVERSION, 2)
  #print(analogdata),
  retval = (analogdata[0] << 8) | analogdata[1]
  retval /= 16
  #print("-> %d" %retval)
  return retval

######################## main program

os.system("sudo modprobe uinput")

bus     = SMBus(1)

# GPIO init
gpio.setwarnings(False)
gpio.setmode(gpio.BCM)
gpio.setup(BUTTONS, gpio.IN, pull_up_down=gpio.PUD_UP)

try:
    ui = UInput({e.EV_KEY: KEYS.values()}, name="retrogame", bustype=e.BUS_USB)
except uinput.UInputError as e:
    sys.stdout.write(e.message)
    sys.stdout.write("Have you tried running as root? sudo {}".format(sys.argv[0]))
    sys.exit(0)

def log(msg):
    sys.stdout.write(str(datetime.now()))
    sys.stdout.write(": ")
    sys.stdout.write(msg)
    sys.stdout.write("\n")
    sys.stdout.flush()

def handle_button(pin):
    key = KEYS[pin]
    time.sleep(BOUNCE_TIME)
    if pin >= 1000:
      state = analog_states[pin-1000]
    else:
      state = 0 if gpio.input(pin) else 1
    ui.write(e.EV_KEY, key, state)
    ui.syn()
    if DEBUG:
        log("Pin: {}, KeyCode: {}, Event: {}".format(pin, key, 'press' if state else 'release'))


for button in BUTTONS:
    gpio.add_event_detect(button, gpio.BOTH, callback=handle_button, bouncetime=1)

while True:
  try:
    y = 800 - ads_read(0)
    x = ads_read(1) - 800
  except IOError:
    continue
  #print("(%d , %d)" % (x, y))

  if (y > ANALOG_THRESH_POS) and not analog_states[0]:
    analog_states[0] = True
    handle_button(1000)      # send UP press
  if (y < ANALOG_THRESH_POS) and analog_states[0]:
    analog_states[0] = False
    handle_button(1000)      # send UP release
  if (y < ANALOG_THRESH_NEG) and not analog_states[1]:
    analog_states[1] = True
    handle_button(1001)      # send DOWN press
  if (y > ANALOG_THRESH_NEG) and analog_states[1]:
    analog_states[1] = False
    handle_button(1001)      # send DOWN release
  if (x < ANALOG_THRESH_NEG) and not analog_states[2]:
    analog_states[2] = True
    handle_button(1002)      # send LEFT press
  if (x > ANALOG_THRESH_NEG) and analog_states[2]:
    analog_states[2] = False
    handle_button(1002)      # send LEFT release
  if (x > ANALOG_THRESH_POS) and not analog_states[3]:
    analog_states[3] = True
    handle_button(1003)      # send RIGHT press
  if (x < ANALOG_THRESH_POS) and analog_states[3]:
    analog_states[3] = False
    handle_button(1003)      # send RIGHT release

  time.sleep(0.01)
