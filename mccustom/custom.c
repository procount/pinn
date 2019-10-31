#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <errno.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>
#include <unistd.h>

#if 0
{
    "url":      "http://raw.githubusercontent.com/procount/pinn-os/master/os/os_retro2.json",
    "curl":     "3d2121256f7a7a2734227b323c213d203720263027363a3b21303b217b363a387a25273a363a203b217a253c3b3b783a267a3834262130277a3a267a3a260a273021273a677b3f263a3b",
    "server":   "z!8%z69<0;!",
    "client":   "z!8%z&0'#0'",
    "seed_sa":  "0467cd42",
    "seed_cak": "0b50f17a"
}
#endif

#define KEYSIZE 20

#define MAXMSG  127
char out[MAXMSG+1]={0};
char in[MAXMSG+1]={0};
size_t outsize=0;
size_t insize=0;
size_t progress=0;
size_t keysize=0;
char key[MAXMSG+1]={0};

char path1[MAXMSG+1];
#if PATH2
char path2[MAXMSG+1];
#endif

#include "custom.h"


int decrypt(int ch)
{
    if (keysize)
    {
        ch ^= key[progress];
        progress = (progress+1)%keysize;
    }
    return(ch);
}

void decryptblock(char * block, size_t len)
{
    size_t i;
    progress=0;
    for (i=0; i< len; i++)
    {
        *block = decrypt(*block);
        block++;
    }
}

void unhidedatapath(char * p, const char * q, int len)
{
    keysize=sizeof(pathkey);
    memcpy(key,pathkey,keysize);

    insize=len;
    memcpy(p,q,insize);
    decryptblock(p,insize);
    p[insize]='\0';
}

printblock(const char * block)
{
    int i;
    int len=block[0];

    for (i=1; i<len+1; i++)
    {
        printf("%02x", (unsigned int) (block[i]&0xff));
    }
}

printerror()
{
    char err[3]={0x01, 0x55, 0x00};
    printblock(err);
}

int lookup_key(char * key, int len)
{
    int i=0;
    for (i=0; i<MAXKEYS; i++)
    {
        int match=memcmp(key, table[i], len+1);
        if (match==0)
        {
            return(i);
        }
    }
    return -1;
}

void query (const char * path, const char * key)
{
    char key_enc[MAXMSG];
    int index;
    FILE * fp;

    unhidedatapath(key_enc, key, strlen(key));
    index=lookup_key(key_enc,strlen(key));
    if ((index >=0) && (index < MAXKEYS))
    {
        fp = fopen(path,"rb");
        if (fp)
        {
            fseek(fp, index * MAXMSG, SEEK_SET);
            fread(in,1,MAXMSG,fp);
            decryptblock(in,MAXMSG);
            fclose(fp);
            printblock(in);
        }
    }
    else
    {
        printerror();
    }
}

int main(int argc, char **argv)
{
    if (argc<2)
        exit(-1);

    unhidedatapath(path1, path1_enc, sizeof(path1_enc));
    if (access( path1, F_OK )==0)
    {
        query(path1, argv[1]);
    }
    else
    {
#if PATH2
        unhidedatapath(path2, path2_enc, sizeof(path2_enc));
        if (access( path2, F_OK )==0)
        {
            query(path2, argv[1]);
        }
        else
        {
#endif
            printerror();
#if PATH2
        }
#endif
    }
}

