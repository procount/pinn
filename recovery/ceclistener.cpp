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
#include <QDebug>
#include <QWSServer>

#ifdef RASPBERRY_CEC_SUPPORT
extern "C" {
#include <interface/vmcs_host/vc_cecservice.h>
#include <interface/vchiq_arm/vchiq_if.h>
#include <interface/vmcs_host/vc_tvservice.h>
}
#endif

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
    vc_cec_set_osd_name("PINN");
    vc_cec_register_callback(_cec_callback, this);

    qDebug() << "CecListener done initializing";
    /* Wait until we are signaled to quit */
    _waitcond.wait(&mutex);

    vc_vchi_cec_stop();
#endif
}

/*static*/ void CecListener::_cec_callback(void *userptr, uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{
    static_cast<CecListener *>(userptr)->cec_callback(reason, param1, param2, param3, param4);
}

void CecListener::cec_callback(uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4)
{
#ifdef RASPBERRY_CEC_SUPPORT
    if (CEC_CB_REASON(reason) == VC_CEC_BUTTON_PRESSED)
    {
        int cec_buttoncode = CEC_CB_OPERAND1(param1);
        keyPressed=1;
        emit keyPress(cec_buttoncode);
        qDebug() << "CEC key: " << cec_buttoncode;
    }
    else if (CEC_CB_REASON(reason) == VC_CEC_REMOTE_PRESSED)
    {
        int cec_buttoncode = CEC_CB_OPERAND1(param1);
        qDebug() << "Vendor key: " << cec_buttoncode;
    }
    else
    {
        qDebug() << "CEC reason: " << CEC_CB_REASON(reason) <<", " <<param1 <<", " << param2 <<", " << param3 <<", " <<param4;
    }

#else
    qDebug() << "CEC:" << reason << param1;
#endif
}
