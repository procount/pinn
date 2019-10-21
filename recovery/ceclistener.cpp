/* Berryboot -- CEC listener thread
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


#include "ceclistener.h"
#include "json.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QMouseEvent>
#include <QWidget>


#ifdef RASPBERRY_CEC_SUPPORT
extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
#include <interface/vchiq_arm/vchiq_if.h>
#include <interface/vmcs_host/vc_tvservice.h>
}
#endif

void inject_key(int key);

static keymap_str cec_map[]={
    {"CEC_User_Control_Number1",     CEC_User_Control_Number1},
    {"CEC_User_Control_Number2",     CEC_User_Control_Number2},
    {"CEC_User_Control_Number3",     CEC_User_Control_Number3},
    {"CEC_User_Control_Number4",     CEC_User_Control_Number4},
    {"CEC_User_Control_Number5",     CEC_User_Control_Number5},
    {"CEC_User_Control_Number6",     CEC_User_Control_Number6},
    {"CEC_User_Control_Number7",     CEC_User_Control_Number7},
    {"CEC_User_Control_Number8",     CEC_User_Control_Number8},
    {"CEC_User_Control_Number9",     CEC_User_Control_Number9},
    {"CEC_User_Control_Number0",     CEC_User_Control_Number0},
    {"CEC_User_Control_Play",        CEC_User_Control_Play},
    {"CEC_User_Control_Stop",        CEC_User_Control_Stop},
    {"CEC_User_Control_Pause",       CEC_User_Control_Pause},
    {"CEC_User_Control_Record",      CEC_User_Control_Record},
    {"CEC_User_Control_Rewind",      CEC_User_Control_Rewind},
    {"CEC_User_Control_FastForward", CEC_User_Control_FastForward},
    {"CEC_User_Control_EPG",         CEC_User_Control_EPG},
    {"CEC_User_Control_Exit",        CEC_User_Control_Exit},
    {"CEC_User_Control_F1Blue",      CEC_User_Control_F1Blue},
    {"CEC_User_Control_F2Red",       CEC_User_Control_F2Red},
    {"CEC_User_Control_F3Green",     CEC_User_Control_F3Green},
    {"CEC_User_Control_F4Yellow",    CEC_User_Control_F4Yellow},
    {"CEC_User_Control_ChannelUp",   CEC_User_Control_ChannelUp},
    {"CEC_User_Control_ChannelDown", CEC_User_Control_ChannelDown},
    {"CEC_User_Control_Left",        CEC_User_Control_Left},
    {"CEC_User_Control_Right",       CEC_User_Control_Right},
    {"CEC_User_Control_Up",          CEC_User_Control_Up},
    {"CEC_User_Control_Down",        CEC_User_Control_Down},
    {"CEC_User_Control_Select",      CEC_User_Control_Select},
    {"CEC_User_Control_PreviousChannel",CEC_User_Control_PreviousChannel},
    {NULL,                             0}
};



CecListener::CecListener(QObject *parent) :
    Kinput(parent)
{
}

CecListener::~CecListener()
{
    _waitcond.wakeAll();
    if (!wait(2000))
    {
        qDebug() << "CEC thread did not stop gracefully";
        terminate();
    }
}

/* Not sure if any of the vchi functions are blocking, but running them in a seperate thread just in case */
void CecListener::run()
{
#ifdef RASPBERRY_CEC_SUPPORT
    VCHI_INSTANCE_T vchiq;
    VCHI_CONNECTION_T *conn;
    QMutex mutex;
    mutex.lock();

    qDebug() << "CecListener thread started";
    if (vchi_initialise(&vchiq))
    {
        qDebug() << "Error during vchi_initialise()";
        return;
    }
    if (vchi_connect(NULL, 0, vchiq))
    {
        qDebug() << "Error connecting to vchi";
        return;
    }
    if (vc_vchi_tv_init(vchiq, &conn, 1))
    {
        qDebug() << "Error during vc_vchi_tv_init()";
        return;
    }
    TV_GET_STATE_RESP_T tvstate;
    vc_tv_get_state(&tvstate);

    if (!(tvstate.state & (VC_HDMI_STANDBY | VC_HDMI_HDMI)))
    {
        qDebug() << "Not in HDMI mode";
        return;
    }

    vc_vchi_cec_init(vchiq, &conn, 1);
    vc_cec_set_osd_name("ARCADE");
    vc_cec_register_callback(_cec_callback, this);

    qDebug() << "CecListener done initializing";
    /* Wait until we are signaled to quit */
    _waitcond.wait(&mutex);

    vc_vchi_cec_stop();
#endif
}

int CecListener::map_button(QVariant cec)
{
    int result=0;
    if (cec.type() == QVariant::String)
    {
        result = map_string(cec_map, cec.toString());
    }
    else
        result=cec.toInt();
    return (result);
}

/*static*/ void CecListener::_cec_callback(void *userptr, uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{
    static_cast<CecListener *>(userptr)->cec_callback(reason, param1, param2, param3, param4);
}

void CecListener::cec_callback(uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{
    Q_UNUSED(param2);
    Q_UNUSED(param3);
    Q_UNUSED(param4);
#ifdef RASPBERRY_CEC_SUPPORT
    int cec_buttoncode = CEC_CB_OPERAND1(param1);
    if (CEC_CB_REASON(reason) == VC_CEC_BUTTON_PRESSED)
    {
        keyPressed=1;
        emit keyPress(cec_buttoncode,1);
        qDebug() << "CEC key: " << cec_buttoncode << " " << decode_key(cec_map, cec_buttoncode);
    }
    else if (CEC_CB_REASON(reason) == VC_CEC_BUTTON_RELEASE)
    {
        emit keyPress(cec_buttoncode,0);
        qDebug() << "CEC key RELEASED: " << cec_buttoncode << " " << decode_key(cec_map, cec_buttoncode);
    }
    else if (CEC_CB_REASON(reason) == VC_CEC_REMOTE_PRESSED)
    {
        qDebug() << "Vendor key: " << cec_buttoncode;
    }

#else
    qDebug() << "CEC:" << reason << param1;
#endif
}


void CecListener::process_cec(int cec_code, int value)
{
    int found=0;
    int key=-1;
    QString menu;
    QString wnd;

    qDebug() << "Proces_CEC:  code: "<<cec_code<<" Value: "<<value<<" wnd: "<<_wnd<<" Menu: "<<_menu;
    int done=0;
    wnd=_wnd;
    do
    {
        qDebug()<<"Searching for "<<wnd;
        if (_map.contains(wnd))
        {
            mapmenu_t m = _map.value(wnd);

            menu = _menu;
            do {
                qDebug()<<"Searching for "<<menu;
                if (m.contains(menu))
                {
                    mapkeys_t k = m.value(menu);
                    if (k.contains(cec_code))
                    {
                        key = k.value(cec_code);
                        qDebug() << "found cec " << cec_code << " ("<< decode_key(cec_map, cec_code) <<  ") in "<<wnd<<" : "<<menu<<" as "<<key<< " ("<<decode_key(key_map,key)<<")";
                        found = 1;
                        done = 1;
                    }
                }
                if (!found)
                {
                    done=0;
                    if (menu !="any")
                        menu="any";
                    else
                        done=1;
                }
            } while (!done);
        }
        if (!found)
        {
            done=0;
            if (wnd != "any")
                wnd="any";
            else
                done=1;
        }
    } while (!done);

    if (key == -1)
        return;

    inject_key(key,value);
}

