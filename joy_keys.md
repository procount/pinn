# JOY_KEYS

PINN v3.3.2 onwards supports programmable USB Joysticks/Gamepads
PINN includes its own Joystick mapping file, but a user-supplied one can override it for different joysticks, gamepads or personal preference.

## JOY Key Mapping file

The user-supplied JOY key mapping file must be called *joy_keys.json* and be stored in PINN's recovery partition (/dev/mmcblk0p1 if on an SD card, where you will find 
the BUILD-DATA and recovery.rfs files - the only partition visible on the SD card if you insert it in a Windows PC)

The default mapping looks like this:
```
{
    "calibration": [
        {
            "threshold": 1,
            "step": 1
        },
        {
            "threshold": 10000,
            "step": 10
        },
        {
            "threshold": 20000,
            "step": 100
        }
    ],

    "VKeyboard": {
        "any": {
            "Key_Left":     "Y",
            "Key_Right":    "RB",
            "Key_Up":       "LB",
            "Key_Down":     "B",
            "Key_Space":    "Back"
        }
    },

    "any": {
        "any": {
            "mouse_left":   "LAnalogLeft",
            "mouse_right":  "LAnalogRight",
            "mouse_up":     "LAnalogUp",
            "mouse_down":   "LAnalogDown",

            "joy_left":     "RAnalogLeft",
            "joy_right":    "RAnalogRight",
            "joy_up":       "RAnalogUp",
            "joy_down":     "RAnalogDown",

            "Key_Left":     "Y",
            "Key_Right":    "RB",
            "Key_Up":       "LB",
            "Key_Down":     "B",
            "Key_space":    "Start",

            "mouse_lclick": "A",
            "Key_Escape":   "X",
            "Key_Enter":    "Start"

        }
    }

}
```
This file uses the json file format. Be careful that you get the syntax correct, otherwise the file will not load.
All given strings must match exactly in spelling and case to avoid any errors.

This file structure consists of three levels:
 * Window name (e.g. "mainwindow")
 * Menu name   (e.g. "Archival")
 * Key mapping (e.g. "Key_Enter": "CEC_User_Control_Left")

## Window Name
Only the given "mainwindow", "bootSelection", "VKeyboard" or "any" window names can be specified.

## Menu Name
Only the "mainwindow" has menus which could be "Main Menu", "Archival", "Maintenance" or "any". Windows which don't have a menu should use the menu name of "any"

## Key mapping
The key mapping lines consist of 2 parts: a keypress code name, and a joystick code name.
The key code name appears first so that the list of all useable keys can still be listed for each menu, even if they are not mapped.
Following this is the JOY code name of the button that will be used to send that keycode to the user interface.
The JOY code can be a string name in quotes, or it could be a decimal integer. A JOY code of -1 can be used to ignore the mapping to a keycode.

## Key map operation

When a button is pressed on the Joystick/remote, the JOY code is sent to PINN.
PINN will select the key mapping based on the currently active window and menu from the mapping file.
If no key mapping is found, it will look in the "any" menu of the active window.
If still not found, it will look in the current menu of the "any" window.
If still not found, it will look in the "any" menu of the "any" window section.

"The "any" sections can be used to define common mappings that apply to all menus of a given window, or to all windows, without having to repeat them.

## Key Codes

The following key codes are recognised:

    "Key_Escape"
    "Key_Space"
    "Key_Enter"
    "Key_PageUp"
    "Key_PageDown"
    "Key_Up"
    "Key_Down"
    "Key_Left"
    "Key_Right"
    "Key_A"
    "Key_B"
    "Key_C"
    "Key_D"
    "Key_E"
    "Key_F"
    "Key_G"
    "Key_H"
    "Key_I"
    "Key_J"
    "Key_K"
    "Key_L"
    "Key_M"
    "Key_N"
    "Key_O"
    "Key_P"
    "Key_Q"
    "Key_R"
    "Key_S"
    "Key_T"
    "Key_U"
    "Key_V"
    "Key_W"
    "Key_X"
    "Key_Y"
    "Key_Z"
    "mouse_left"
    "mouse_right"
    "mouse_up"
    "mouse_down"
    "mouse_lclick"
    "joy_left"
    "joy_right"
    "joy_up"
    "joy_down"
    "joy_lclick"

The "mouse_*" entries are "virtual" keys that will move the mouse around and click the left mouse button.
The "joy_*" entries are "virtual" keys that are meant for analog joysticks (in combination with the "calibration" section), but are not fully implemented or tested yet.

## JOY codes

The following JOY codes are recognised:

    "LAnalogLeft"
    "LAnalogRight"
    "LAnalogUp"
    "LAnalogDown"
    "LT"
    "RAnalogLeft"
    "RAnalogRight"
    "RAnalogUp"
    "RAnalogDown"
    "RT"
    "D-padLeft"
    "D-padRight"
    "D-padUp"
    "D-padDown"
    "A"
    "B"
    "X"
    "Y"
    "LB"
    "RB"
    "Back"
    "Start"
    "Mode"
    "LAnalog_click"
    "RAnalog_click"

To identify which buttons are supported by your joystick/gamepad, enter the recovery shell by pressing "ctrl-alt-F2" and login with the username/password of root/raspberry.
At the command prompt type `tail -f /tmp/debug`. You can now press each button in turn on your joystick/gamepad and note down which JOY codes are produced. 

