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
 */

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <stdint.h>



class CecListener : public QThread
{
    Q_OBJECT
public:
    explicit CecListener(QObject *parent = 0);
    virtual ~CecListener();
    static int hasKeyPressed() {return(keyPressed);}
    static int clearKeyPressed() {keyPressed=0;}

signals:
    void keyPress(int key);
public slots:

protected:
    virtual void run();
    static void _cec_callback(void *userptr, uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4);
    void cec_callback(uint32_t reason, uint32_t param1, uint32_t param2, uint32_t param3, uint32_t param4);

    QWaitCondition _waitcond;
    static int keyPressed;
};

#endif // CECLISTENER_H
