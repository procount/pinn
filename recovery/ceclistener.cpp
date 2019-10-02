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
 */


#include "ceclistener.h"
#include "json.h"

#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QMouseEvent>
#include <QWidget>

#include <QWSServer>

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


keymap_str key_map[]={
    {"Key_Escape",      Qt::Key_Escape},
    {"Key_Space",       Qt::Key_Space},
    {"Key_Enter",       Qt::Key_Enter},
    {"Key_PageUp",      Qt::Key_PageUp},
    {"Key_PageDown",    Qt::Key_PageDown},
    {"Key_Up",          Qt::Key_Up},
    {"Key_Down",        Qt::Key_Down},
    {"Key_Left",        Qt::Key_Left},
    {"Key_Right",       Qt::Key_Right},
    {"Key_A",           Qt::Key_A},
    {"Key_B",           Qt::Key_B},
    {"Key_C",           Qt::Key_C},
    {"Key_D",           Qt::Key_D},
    {"Key_E",           Qt::Key_E},
    {"Key_F",           Qt::Key_F},
    {"Key_G",           Qt::Key_G},
    {"Key_H",           Qt::Key_H},
    {"Key_I",           Qt::Key_I},
    {"Key_J",           Qt::Key_J},
    {"Key_K",           Qt::Key_K},
    {"Key_L",           Qt::Key_L},
    {"Key_M",           Qt::Key_M},
    {"Key_N",           Qt::Key_N},
    {"Key_O",           Qt::Key_O},
    {"Key_P",           Qt::Key_P},
    {"Key_Q",           Qt::Key_Q},
    {"Key_R",           Qt::Key_R},
    {"Key_S",           Qt::Key_S},
    {"Key_T",           Qt::Key_T},
    {"Key_U",           Qt::Key_U},
    {"Key_V",           Qt::Key_V},
    {"Key_W",           Qt::Key_W},
    {"Key_X",           Qt::Key_X},
    {"Key_Y",           Qt::Key_Y},
    {"Key_Z",           Qt::Key_Z},
    {"mouse_left",      mouse_left},
    {"mouse_right",     mouse_right},
    {"mouse_up",        mouse_up},
    {"mouse_down",      mouse_down},
    {"mouse_lclick",    mouse_lclick},
    {"joy_left",        joy_left},
    {"joy_right",       joy_right},
    {"joy_up",          joy_up},
    {"joy_down",        joy_down},
    {"joy_lclick",      mouse_lclick},
    {NULL,                0}
};


int CecListener::keyPressed = 0;

CecListener::CecListener(QObject *parent) :
    QThread(parent)
{
    keyPressed=0;
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

int CecListener::map_string(struct keymap_str *map, QString str)
{
    while (map->string)
    {
        if (map->string == str)
            return(map->code);
        map++;
    }
    return(0);
}

int CecListener::map_cec(QVariant cec)
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

int CecListener::map_key(QString key)
{
    return ( map_string(key_map, key) );
}

void CecListener::loadCECmap(QString filename)
{
    QString fname = filename;
    if (!QFile::exists(fname))
        fname=":/cec_keys.json"; //Use default mapping

    if (QFile::exists(fname))
    {
        qDebug() << "Loading CEC mappings from " << fname;
        _CECmap.clear();

        QVariantMap mapWnd = Json::loadFromFile(fname).toMap();

        for(QVariantMap::const_iterator iWnd = mapWnd.begin(); iWnd != mapWnd.end(); ++iWnd)
        {   //For each window, get the map of menus
            QVariantMap mapMenu = iWnd.value().toMap();

            mapmenu_t m; //My own map of menus
            for(QVariantMap::const_iterator iMenu = mapMenu.begin(); iMenu != mapMenu.end(); ++iMenu)
            {   //For each menu, get the map of keys
                QVariantMap mapKeys = iMenu.value().toMap();

                mapkeys_t k; //My own map of keys
                for(QVariantMap::const_iterator iKey = mapKeys.begin(); iKey != mapKeys.end(); ++iKey)
                {   //For each key
                    int cec_code = map_cec(iKey.value());   //Convert the CEC code (string or int)
                    int key_code = map_key(iKey.key());     //Convert the KEY code to press (string)
                    if (cec_code !=-1)                      //Use CEC code of -1 to ignore that key
                        k[cec_code] = key_code;             //Map the CEC code to the key to be pressed
                }
                //Add key mapping to my menu
                m[iMenu.key()] = k;
            }
            //Add my menu to my window map
            _CECmap[iWnd.key()] = m;
        }
    }
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
        emit keyPress(cec_buttoncode);
        qDebug() << "CEC key: " << cec_buttoncode << " " << decode_key(cec_map, cec_buttoncode);
    }
    else if (CEC_CB_REASON(reason) == VC_CEC_BUTTON_RELEASE)
    {
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

const char * decode_key(struct keymap_str *map, int code)
{
    while (map->string)
    {
        if (map->code == code)
            return(map->string);
        map++;
    }
    return("Unknown");
}

void CecListener::process_cec(int cec_code)
{
    int found=0;
    int key=-1;
    QString menu;
    QString wnd;

    int done=0;
    wnd=_wnd;
    do
    {
        if (_CECmap.contains(wnd))
        {
            mapmenu_t m = _CECmap.value(wnd);

            menu = _menu;
            do {
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

    inject_key(key);
}

void inject_key(int key)
{
    Qt::KeyboardModifiers modifiers = Qt::NoModifier;
    QPoint p = QCursor::pos();

    switch (key)
    {
    /* MOUSE SIMULATION */
    case mouse_left:
        p.rx()-=10;
        QCursor::setPos(p);
        break;
    case mouse_right:
        p.rx()+=10;
        QCursor::setPos(p);
        break;
    case mouse_up:
        p.ry()-=10;
        QCursor::setPos(p);
        break;
    case mouse_down:
        p.ry()+=10;
        QCursor::setPos(p);
        break;
    case mouse_lclick:
        { //Click!
            QWidget* widget = dynamic_cast<QWidget*>(QApplication::widgetAt(QCursor::pos()));
            if (widget)
            {
                QPoint pos = QCursor::pos();
                QMouseEvent *event = new QMouseEvent(QEvent::MouseButtonPress,widget->mapFromGlobal(pos), Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
                QCoreApplication::sendEvent(widget,event);
                QMouseEvent *event1 = new QMouseEvent(QEvent::MouseButtonRelease,widget->mapFromGlobal(pos), Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
                QCoreApplication::sendEvent(widget,event1);
                qApp->processEvents();
            }
        }
        break;
    default:
        // key press
        QWSServer::sendKeyEvent(0, key, modifiers, true, false);
        // key release
        QWSServer::sendKeyEvent(0, key, modifiers, false, false);
        break;
    }
}

