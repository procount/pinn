# CEC_KEYS

PINN v2.8.1 onward supports programmable CEC Keys
PINN includes its own CEC mapping file, but a user-supplied one can override it for different TV models, CEC implementations or personal preference.

## CEC Key Mapping file

The user-supplied CEC key mapping file must be called *cec_keys.json* and be stored in PINN's recovery partition (/dev/mmcblk0p1 if on an SD card, where you will find 
the BUILD-DATA and recovery.rfs files - the only partition visible on the SD card if you insert it in a Windows PC)

The default mapping looks like this:
```
{
    "mainwindow": {
        "Main Menu": {
            "Key_I":        "CEC_User_Control_Number1",
            "Key_W":        "CEC_User_Control_Number2",
            "Key_H":        "CEC_User_Control_Number3",
            "Key_N":        "CEC_User_Control_Number4",
            "Key_Escape":   "CEC_User_Control_Number5"
        },
        "Archival": {
            "Key_D":        "CEC_User_Control_Number1",
            "Key_C":        "CEC_User_Control_Number2",
            "Key_N":        "CEC_User_Control_Number3",
            "Key_F":        "CEC_User_Control_Number4"
        },
        "Maintenance": {
            "Key_E":        "CEC_User_Control_Number1",
            "Key_P":        "CEC_User_Control_Number2",
            "Key_F":        "CEC_User_Control_Number3",
            "Key_R":        "CEC_User_Control_Number4",
            "Key_N":        "CEC_User_Control_Number5"
        },
        "any": {
            "Key_Space":    "CEC_User_Control_Play",
            "Key_Escape":   "CEC_User_Control_Exit",
            "Key_M":        "CEC_User_Control_F2Red",
            "Key_Enter":    "CEC_User_Control_Play",
            "Key_PageDown": "CEC_User_Control_Number9",
            "Key_Down":     "CEC_User_Control_ChannelDown",
            "Key_Up":       "CEC_User_Control_ChannelUp" 
        }
    },

    "bootSelection": {
        "any": {
            "Key_Enter":    "CEC_User_Control_Play",
            "Key_Escape":   "CEC_User_Control_Exit",
            "Key_Up":       "CEC_User_Control_ChannelUp",
            "Key_Down":     "CEC_User_Control_ChannelDown"
        }
    },

    "any": {
        "any": {
            "mouse_left":   "CEC_User_Control_Left",
            "mouse_right":  "CEC_User_Control_Right",
            "mouse_up":     "CEC_User_Control_Up",
            "mouse_down":   "CEC_User_Control_Down",
            "mouse_lclick": "CEC_User_Control_Select"
        }
    }
}
```
This file uses the json file format. Be careful that you get the syntax correct, otherwise the file will not load.
All given strings must match exactly in spelling and case to avoid any errors.

This file structure consists of three levels:
[list]
[*] Window name (e.g. "mainwindow")
[*] Menu name   (e.g. "Archival")
[*] Key mapping (e.g. "Key_Enter": "CEC\_User\_Control\_Left")
[/list]


## Window Name
Only the given "mainwindow", "bootSelection" or "any" window names can be specified.

## Menu Name
Only the "mainwindow" has menus which could be "Main Menu", "Archival", "Maintenance" or "any". Windows which don't have a menu should use the menu name of "any"

## Key mapping
The key mapping lines consist of 2 parts: a keypress code name, and a cec code name.
The key code name appears first so that the list of all useable keys can still be listed for each menu, even if they are not mapped.
Following this is the CEC code name of the button that will be used to send that keycode to the user interface.
The CEC code can be a string name in quotes, or it could be a decimal integer. A CEC code of -1 can be used to ignore the mapping to a keycode.

## Key map operation

When a button is pressed on the remote, the CEC code is sent to PINN.
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

The "mouse_*" entries are "virtual" keys that will move the mouse around and click the left mouse button.

## CEC codes

The following CEC codes are recognised:

    "CEC_User_Control_Number1"
    "CEC_User_Control_Number2"
    "CEC_User_Control_Number3"
    "CEC_User_Control_Number4"
    "CEC_User_Control_Number5"
    "CEC_User_Control_Number6"
    "CEC_User_Control_Number7"
    "CEC_User_Control_Number8"
    "CEC_User_Control_Number9"
    "CEC_User_Control_Number0"
    "CEC_User_Control_Play"
    "CEC_User_Control_Stop"
    "CEC_User_Control_Pause"
    "CEC_User_Control_Record"
    "CEC_User_Control_Rewind"
    "CEC_User_Control_FastForward"
    "CEC_User_Control_EPG"
    "CEC_User_Control_Exit"
    "CEC_User_Control_F1Blue"
    "CEC_User_Control_F2Red"
    "CEC_User_Control_F3Green"
    "CEC_User_Control_F4Yellow"
    "CEC_User_Control_ChannelUp"
    "CEC_User_Control_ChannelDown"
    "CEC_User_Control_Left"
    "CEC_User_Control_Right"
    "CEC_User_Control_Up"
    "CEC_User_Control_Down"
    "CEC_User_Control_Select"
    "CEC_User_Control_PreviousChannel"

Not all remote button presses are passed through to PINN. Some that are used to control the TV will be suppressed.
To identify which keys are supported by your remote, enter the recovery shell by pressing "ctrl-alt-F2" and login with the username/password of root/raspberry.
At the command prompt type `tail -f /tmp/debug`. You can now press each button in turn on your remote and note down which CEC codes are produced. 
Some TVs/remotes may support additional CEC codes and will be marked as "unknown". These can still be used but will need to be entered as decimal numbers in the key mapping table.



