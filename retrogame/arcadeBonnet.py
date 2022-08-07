#!/usr/bin/python

# Somewhat minimal Adafruit Arcade Bonnet handler.  Runs in background,
# translates inputs from MCP23017 port expander to virtual USB keyboard
# events.  Not -quite- as efficient or featuretastic as retrogame, but
# still reasonably lightweight and may be easier and/or more reliable than
# retrogame for some users.  Supports ONE port expander, no regular GPIO
# (non-port-expander) or "Vulcan nerve pinch" features.
# Prerequisites:
# sudo apt-get install python-pip python-smbus python-dev
# sudo pip install evdev
# Be sure to enable I2C via raspi-config.  Also, udev rules will need to
# be set up per retrogame directions.
# Credit to Pimoroni for Picade HAT scripts as starting point.

import os
import time
import RPi.GPIO as gpio
from evdev import uinput, UInput, ecodes as e
from smbus import SMBus

key = [ # EDIT KEYCODES IN THIS TABLE TO YOUR PREFERENCES:
	# See /usr/include/linux/input.h for keycode names
	# Keyboard        Bonnet        EmulationStation
	e.KEY_LEFTCTRL, # 1A            'A' button
	e.KEY_LEFTALT,  # 1B            'B' button
	e.KEY_A,        # 1C            'X' button
	e.KEY_S,        # 1D            'Y' button
	e.KEY_5,        # 1E            'Select' button
	e.KEY_1,        # 1F            'Start' button
	0,              # Bit 6 NOT CONNECTED on Bonnet
	0,              # Bit 7 NOT CONNECTED on Bonnet
	e.KEY_DOWN,     # 4-way down    D-pad down
	e.KEY_UP,       # 4-way up      D-pad up
	e.KEY_RIGHT,    # 4-way right   D-pad right
	e.KEY_LEFT,     # 4-way left    D-pad left
	e.KEY_L,        # Analog right
	e.KEY_H,        # Analog left
	e.KEY_J,        # Analog down
	e.KEY_K         # Analog up
]

addr   = 0x26 # I2C Address of MCP23017
irqPin = 17   # IRQ pin for MCP23017

os.system("sudo modprobe uinput")

ui      = UInput({e.EV_KEY: key}, name="retrogame", bustype=e.BUS_USB)
bus     = SMBus(1)
IODIRA  = 0x00
IOCONA  = 0x0A
INTCAPA = 0x10

# Initial MCP23017 config:
bus.write_byte_data(addr, 0x05  , 0x00) # If bank 1, switch to 0
bus.write_byte_data(addr, IOCONA, 0x44) # Bank 0, INTB=A, seq, OD IRQ

# Read/modify/write remaining MCP23017 config:
cfg = bus.read_i2c_block_data(addr, IODIRA, 14)
cfg[ 0] = 0xFF # Input bits
cfg[ 1] = 0xFF
cfg[ 2] = 0x00 # Polarity
cfg[ 3] = 0x00
cfg[ 4] = 0xFF # Interrupt pins
cfg[ 5] = 0xFF
cfg[12] = 0xFF # Pull-ups
cfg[13] = 0xFF
bus.write_i2c_block_data(addr, IODIRA, cfg)

# Clear interrupt by reading INTCAP and GPIO registers
x        = bus.read_i2c_block_data(addr, INTCAPA, 4)
oldState = x[2] | (x[3] << 8)

# Callback for MCP23017 interrupt request
def mcp_irq(pin):
	global oldState
	x = bus.read_i2c_block_data(addr, INTCAPA, 4)
	newState = x[2] | (x[3] << 8)
	for i in range(16):
		bit = 1 << i
		lvl = newState & bit
		if lvl != (oldState & bit):
			ui.write(e.EV_KEY, key[i], 0 if lvl else 1)
	ui.syn()
	oldState = newState

# GPIO init
gpio.setwarnings(False)
gpio.setmode(gpio.BCM)

# Enable pullup and callback on MCP23017 IRQ pin
gpio.setup(irqPin, gpio.IN, pull_up_down=gpio.PUD_UP)
gpio.add_event_detect(irqPin, gpio.FALLING, callback=mcp_irq)

while True: time.sleep(1)
