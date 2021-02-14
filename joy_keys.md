# JOY_KEYS

PINN v3.3.2 onwards supports programmable USB Joysticks/Gamepads
PINN includes its own Joystick mapping file, but a user-supplied one can override it for different joysticks, gamepads or personal preference.

After v3.5.4, the joystick file has been enhanced. See below for more details.

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

# New Joystick file format

After V3.5.4, the joystick configuration files have changed, although the old style will still work to a certain extent.

PINN will now search for a joystick configuration file based on the name of the joystick and the number of channels it supports. The name of the file is formatted as "JoystickName_xA_yB.json" where x and y are the number of the Analog and Digital channels respectively. You will find this name in the debug log. If the configuration file cannot be found, PINN will use the default joy_keys.json file instead with the default key mappings.

The calibration codes are no longer used and this section can be omitted.

## Inputs section

A new section to define the inputs has been added.

Here is an example of how this inputs section will look:


    "inputs": {
        "A_btn":                [0,1],
        "B_btn":                [1,1],
        "X_btn":                [2,1],
        "Y_btn":                [3,1],
        "L1_btn":               [4,1],
        "R1_btn":               [5,1],
        
        "L2_axis":              [2, 1, -30000],
        "R2_axis":              [5, 1, -30000],
        
        "DLeft_axis":           [6,-1],
        "DRight_axis":          [6, 1],
        
        "DUp_axis":             [7,-1],
        "DDown_axis":           [7, 1],
        
        "Select_btn":           [6,1],
        "Start_btn":            [7,1],
        "Home_btn":             [8,1],
        
        "LAnalog_btn":          [9,1],
        "RAnalog_btn":          [10,1],
        
        "LAnalogLeft_axis":     [ 0, -1, 6000],
        "LAnalogRight_axis":    [ 0, 1, 6000],
        "LAnalogUp_axis":       [ 1, -1, 6000],
        "LAnalogDown_axis":     [ 1, 1, 6000],
        
        "RAnalogLeft_axis":     [ 3, -1, 6000],
        "RAnalogRight_axis":    [ 3, 1, 6000],
        "RAnalogUp_axis":       [ 4, -1, 6000],
        "RAnalogDown_axis":     [ 4, 1, 6000]
    },

The format of this file is as follows:

        "InputName_type":    [ id, direction {, deadzone} ],

### Input Name & type

The inputName name can be whatever you like, but it is best to stick to the defined joy-code names defined above.
The inputName must end with either '_btn' or '_axis' to indicate its type to determine its behaviour.

### ID

The id is the number of the input channel as given by the joystick.

### Direction

The direction of the button type (_btn) must always be 1 as the button has a default value of 0 which changes to 1 when pressed.
Analog channels (_axis) are centred at 0 and send positive values when moved in one direction and negative values when moved in the opposite direction. These are mapped to 2 different buttons with the direction=1 representing the positive axis and direction=-1 representing the negative direction.

### Deadzone

The optional deadzone value can serve 2 different purposes for analog channels. 

Some joysticks give a definite 0 value when centred, whilst others jitter around. A positive deadzone value can be used to stop the jitter, since any value between +/- deadzone is forced to zero.

Some joystick analog channels are called triggers, which default to -32767 and increase in value through 0 to +32767 when pressed. These can be indicated by a negative deadzone value. When the joystick is below the deadzone value, it is reported as a 0, and a 1 when above the deadzone value.

## Multiple buttons

It is now possible to map multiple joystick channels to the same key by adding an array of joystick button names. For example:

        "mouse_left":   ["LAnalogLeft_axis","RAnalogLeft_axis"],

Here, both the Left analog and Right analog joysticks are mapped onto the mouse_left function.
This is a convenience function only and may not work fully for all button or axis functionality.

## Multiple Joysticks

PINN can support 2 joysticks and use both of them simultaneously. However, some joysticks appear as 2 devices, so if one of those is detected first, it will take up both slots. In that case, try changing the order they are plugged into the Pi after it has booted. 


