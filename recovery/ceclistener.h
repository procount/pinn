#ifndef CECLISTENER_H
#define CECLISTENER_H

/* Berryboot -- CEC handling thread
 *
 * Copyright (c) 2012, Floris Bos
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * Modified by @procount (c) 2019
 *
 */

#include "input.h"

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <stdint.h>
#include <QVariantMap>

/*                                                        SAMSUNG
   CEC_User_Control_Select                      = 0x00,   centre of cursor keys
   CEC_User_Control_Up                          = 0x01,   up
   CEC_User_Control_Down                        = 0x02,   down
   CEC_User_Control_Left                        = 0x03,   left
   CEC_User_Control_Right                       = 0x04,   right
   CEC_User_Control_RightUp                     = 0x05,
   CEC_User_Control_RightDown                   = 0x06,
   CEC_User_Control_LeftUp                      = 0x07,
   CEC_User_Control_LeftDown                    = 0x08,
   CEC_User_Control_RootMenu                    = 0x09,
   CEC_User_Control_SetupMenu                   = 0x0A,
   CEC_User_Control_ContentsMenu                = 0x0B,
   CEC_User_Control_FavoriteMenu                = 0x0C,
   CEC_User_Control_Exit                        = 0x0D,  Exit
   CEC_User_Control_Number0                     = 0x20,  0
   CEC_User_Control_Number1                     = 0x21,  1
   CEC_User_Control_Number2                     = 0x22,  2
   CEC_User_Control_Number3                     = 0x23,  3
   CEC_User_Control_Number4                     = 0x24,  4
   CEC_User_Control_Number5                     = 0x25,  5
   CEC_User_Control_Number6                     = 0x26,  6
   CEC_User_Control_Number7                     = 0x27,  7
   CEC_User_Control_Number8                     = 0x28,  8
   CEC_User_Control_Number9                     = 0x29,  9
   CEC_User_Control_Dot                         = 0x2A,
   CEC_User_Control_Enter                       = 0x2B,
   CEC_User_Control_Clear                       = 0x2C,
   CEC_User_Control_ChannelUp                   = 0x30,  chan up
   CEC_User_Control_ChannelDown                 = 0x31,  chan down
   CEC_User_Control_PreviousChannel             = 0x32,  Prev ch
   CEC_User_Control_SoundSelect                 = 0x33,
   CEC_User_Control_InputSelect                 = 0x34,
   CEC_User_Control_DisplayInformation          = 0x35,
   CEC_User_Control_Help                        = 0x36,
   CEC_User_Control_PageUp                      = 0x37,
   CEC_User_Control_PageDown                    = 0x38,
   CEC_User_Control_Power                       = 0x40,
   CEC_User_Control_VolumeUp                    = 0x41,
   CEC_User_Control_VolumeDown                  = 0x42,
   CEC_User_Control_Mute                        = 0x43,
   CEC_User_Control_Play                        = 0x44,  play
   CEC_User_Control_Stop                        = 0x45,  stop
   CEC_User_Control_Pause                       = 0x46,  pause
   CEC_User_Control_Record                      = 0x47,  record
   CEC_User_Control_Rewind                      = 0x48,  rewind
   CEC_User_Control_FastForward                 = 0x49,  fast forward
   CEC_User_Control_Eject                       = 0x4A,
   CEC_User_Control_Forward                     = 0x4B,
   CEC_User_Control_Backward                    = 0x4C,
   CEC_User_Control_Angle                       = 0x50,
   CEC_User_Control_Subpicture                  = 0x51,
   CEC_User_Control_VideoOnDemand               = 0x52,
   CEC_User_Control_EPG                         = 0x53,  Guide
   CEC_User_Control_TimerProgramming            = 0x54,
   CEC_User_Control_InitialConfig               = 0x55,
   CEC_User_Control_PlayFunction                = 0x60,
   CEC_User_Control_PausePlayFunction           = 0x61,
   CEC_User_Control_RecordFunction              = 0x62,
   CEC_User_Control_PauseRecordFunction         = 0x63,
   CEC_User_Control_StopFunction                = 0x64,
   CEC_User_Control_MuteFunction                = 0x65,
   CEC_User_Control_RestoreVolumeFunction       = 0x66,
   CEC_User_Control_TuneFunction                = 0x67,
   CEC_User_Control_SelectDiskFunction          = 0x68,
   CEC_User_Control_SelectAVInputFunction       = 0x69,
   CEC_User_Control_SelectAudioInputFunction    = 0x6A,
   CEC_User_Control_F1Blue                      = 0x71,  blue
   CEC_User_Control_F2Red                       = 0x72,  Red
   CEC_User_Control_F3Green                     = 0x73,  green
   CEC_User_Control_F4Yellow                    = 0x74,  yellow
   CEC_User_Control_F5                          = 0x75
 */

/* Samsung keys
1 33
2 34
3 35
4 36
5 37
6 38
7 39
8 40
9 41
0 32
Pre-ch 50
ch up 48
ch dn 49
guide 83
exit 13
red 114
green 115
yellow 116
blue 113
rewind 72
pause 70
ffwd 73
rec 71
play 68
stop 69
*/



class CecListener : public Kinput
{
    Q_OBJECT
public:
    explicit CecListener(QObject *parent = 0);
    virtual ~CecListener();
    void loadMap(QString filename) { Kinput::loadMap(filename, "cec_keys.json"); }

    void process_cec(int cec_code, int value);

signals:
    void keyPress(int key,int value);
public slots:

protected:
    virtual void run();
    static void _cec_callback(void *userptr, uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4);
    void cec_callback(uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4);
    int map_button(QVariant cec);

    QWaitCondition _waitcond;

};

#endif // CECLISTENER_H
