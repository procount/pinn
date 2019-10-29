/*
 * Copyright (c) 2019, @procount
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
 */

#include "custom.h"
#define KEYSIZE 20
custom::custom()
{
}

QString custom::read(const char * key)
{
    //Get the map of windows (& calibration)
    QVariantMap map = Json::loadFromFile(":/custom.json").toMap();
    return (map.value(key).toString());
}
const char seed_sa[KEYSIZE]= {0xd2,0xba,0x4e,0x40,0x58,0x7f,0x6d,0x64,0x1a,0x68,0xab,0x54,0x55,0x81,0xbe,0x22,0x5f,0xd3,0x80,0x51};
const char seed_cak[KEYSIZE]={0x0a,0x4d,0x23,0x23,0x95,0xbc,0x2a,0x36,0xce,0x27,0x18,0x91,0x52,0x09,0xdb,0x28,0x04,0xd5,0x0e,0x9c};

int custom::readhex(const char * key, char * out, size_t * len)
{
    QString raw = read(key);
    if (raw=="seed_sa")
    {
        memcpy(out,seed_sa, KEYSIZE);
        *len = KEYSIZE;
    }
    else if (raw=="seed_cak")
    {
        memcpy(out,seed_sa, KEYSIZE);
        *len = KEYSIZE;
    }
    else
        return hexdecode(raw.toAscii().data(), out, len);
    return(0);
}
