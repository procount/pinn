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

//#define KEYSIZE 20

#define MAXMSG  127
char out[MAXMSG+1]={0};
char in[MAXMSG+1]={0};
size_t outsize=0;
size_t insize=0;
size_t progress=0;
size_t keysize=0;
char key[MAXMSG+1]={0};

const char pathkey[]={0xaf,0x55,0xc4,0x27,0x52,0x4e,0x21,0x8d,0xc4,0xe9,0xcf,0xf4,0xa4,0xcc,0x0f,0xf2,0xae,0x5f,0x46,0xfc};
const char path1_enc[]={0xaf,0x55,0xc4,0x27,0x52,0x4e,0x21,0x8d,0xc4,0xe9,0xcf,0xf4,0xa4,0xcc,0x0f,0xf2,0xae,0x5f,0x46,0xfc}; //"/lib/libcustom.so.1";
const char path2_enc[]={0xaf,0x55,0xc4,0x27,0x52,0x4e,0x21,0x8d,0xc4,0xe9,0xcf,0xf4,0xa4,0xcc,0x0f,0xf2,0xae,0x5f,0x46,0xfc}; //"/lib/firmware/brcm45111";

char path1[MAXMSG+1];
char path2[MAXMSG+1];

#define MAXKEYS 5

const char * table[MAXKEYS]={
    "curl",
    "server",
    "client",
    "seed_sa",
    "seed_cak"
};

char block[MAXMSG*(MAXKEYS+2)];

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
    for (i=0; i<MAXMSG; i++)
    {
        if (block[i])
            printf("%02x", (unsigned int) (block[i]&0xff));
        else
            break;
    }
    printf("\n");
}

int lookup_key(char * key, int len)
{
    int i=0;
    for (i=0; i<MAXKEYS; i++)
    {
        if (memcmp(key, &table[i], len)==0)
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
    if ((index >0) && (index < MAXKEYS))
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
        printf("Error");
}

void pad(char * buff, int len)
{
    int i;
    for (i=0; i<len; i++)
        buff[i]=32+rand()%96;
    buff[len]='\0';
}

void hexdecode(char * str, char * output, int * size)
{
    *size=0;
    char buff[3];
    buff[2]='\0';
    if ((strlen(str) % 2))
        return;
    while (*str)
    {
        buff[0]=*str++;
        buff[1]=*str++;
        long int num = strtol(buff, NULL, 16);
        *output++ = (char)num;
        (*size)++;
    }
    *output='\0';
}

int main(int argc, char **argv)
{
    char buffer[MAXMSG+1];
    int i;
    int j;
    time_t t;
    FILE * fp;
    int len;

    int blk=0;
    /* Intializes random number generator */
    srand((unsigned) time(&t));

    keysize=sizeof(pathkey);
    memcpy(key,pathkey,keysize);


    //URL is XORed with 0x55, then stored as binary
    printf("Enter URL as path: ");
    pad (&buffer[1],MAXMSG);
    scanf("%s",&buffer[1]);
    len=strlen(&buffer[1]);
    buffer[0]=len;
    for (i=1; i< len+1; i++)
    {
        buffer[i] ^= 0x55;
    }
    decryptblock(buffer,MAXMSG);
    memcpy(&block[MAXMSG*blk++], buffer, MAXMSG);


    //server XORed with 0xff, then stored as binary
    printf("Enter Server as path: ");
    pad (&buffer[1],MAXMSG);
    scanf("%s",&buffer[1]);
    len=strlen(&buffer[1]);
    buffer[0]=len;
    for (i=1; i< len+1; i++)
    {
        buffer[i] ^= 0x55;
    }
    decryptblock(buffer,MAXMSG);
    memcpy(&block[MAXMSG*blk++], buffer, MAXMSG);


    //client XORed with 0xff, then stored as binary
    printf("Enter Client as path: ");
    pad (&buffer[1],MAXMSG);
    scanf("%s",&buffer[1]);
    len=strlen(&buffer[1]);
    buffer[0]=len;
    for (i=1; i< len+1; i++)
    {
        buffer[i] ^= 0x55;
    }
    decryptblock(buffer,MAXMSG);
    memcpy(&block[MAXMSG*blk++], buffer, MAXMSG);


    //seed is stored in hex
    printf("Enter seed_sa as raw hex string: ");
    pad (&buffer[1],MAXMSG);
    scanf("%s",&buffer[1]);
    pad(in,MAXMSG);// strlen(buffer)>>1);
    hexdecode(&buffer[1],&in[1], &insize);
    in[0]=insize;
    for (i=1; i< insize+1; i++)
    {
        in[i] ^= 0x55;
    }
    decryptblock(in,MAXMSG);
    memcpy(&block[MAXMSG*blk++], in, MAXMSG);


    printf("Enter seed_cak as raw hex string: ");
    pad (&buffer[1],MAXMSG);
    scanf("%s",&buffer[1]);
    pad(in,MAXMSG);// strlen(buffer)>>1);
    hexdecode(&buffer[1],&in[1], &insize);
    in[0]=insize;
    for (i=1; i< insize+1; i++)
    {
        in[i] ^= 0x55;
    }
    decryptblock(in,MAXMSG);
    memcpy(&block[MAXMSG*blk++], in, MAXMSG);

    fp = fopen("output","wb");
    if (fp)
    {
        fwrite(&block[0], (MAXMSG)*blk, 1, fp);
        fclose(fp);
    }


    for (i=0; i<MAXKEYS; i++)
    {
        int len = strlen(table[i]);

        unhidedatapath(buffer, table[i], len);

        printf("%s {",table[i]);
        for (j=0; j<len+1; j++)
        {
            printf("0x%02x", (unsigned int) buffer[j]&0xff);
            if (i<len-1)
                printf(",");
        }
        printf("};\n");
    }

    //Path1
    printf("Enter path1: ");
    scanf("%s",buffer);
    decryptblock(buffer,MAXMSG);

    printf("path1 {");
    for (j=0; j<MAXMSG; j++)
    {
        printf("0x%02x", (unsigned int) buffer[j]&0xff);
        if (i<MAXMSG-1)
            printf(",");
    }
    printf("};\n");

#if 0
    //Path2
    printf("Enter path2: ");
    scanf("%s",buffer);
    decryptblock(buffer,MAXMSG);

    printf("path2 {");
    for (j=0; j<MAXMSG; j++)
    {
        printf("0x%02x", (unsigned int) buffer[j] & 0xff);
        if (i<MAXMSG-1)
            printf(",");
    }
    printf("};\n");
#endif
}
