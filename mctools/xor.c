//XOR.C
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>

#define CIPHER 1

#if CIPHER
 #define MAXMSG  256
 char out[MAXMSG]={0};
 char in[MAXMSG]={0};
 size_t outsize=0;
 size_t insize=0;
 size_t progress=0;
 size_t keysize=0;
 const char SOCKSERVER[]= "z!8%z69<0;!";
 char key[MAXMSG]={0};
 char sockserver[12];
 char sockclient[12];
 const char SOCKCLIENT[]= "z!8%z&0'#0'";
 
 const char seed_sa[]={0x04, 0x67, 0xcd, 0x42};
 const char seed_cak[]={0x0b, 0x50, 0xf1, 0x7a};

#endif

void process_key()
{
    int i,j;
    keysize=insize;

    for (i=0,j=0; i<insize; i++)
    {
        key[i] = out[j++] ^ in[i];
        if (j>outsize)
            j=0;
    }
}

int decrypt(int ch)
{
    if (keysize)
    {
        ch ^= key[progress];
        progress = (progress+1)%keysize;
    }
    return(ch);
}

void decryptblock(char * block, int len)
{
    int i;
    progress=0;
    for (i=0; i< len; i++)
    {
        *block = decrypt(*block);
        block++;
    }
}

void printblock(char * block, int len)
{
    int i;
    int col=0;
    char * s=block;
    progress=0;
    for (i=0; i< len; i++)
    {
        printf("%02x", (unsigned int)( *s++ & 0xff));
        //col = (col+1)%16;
        //if (!col)
        //    printf("\n");
    }

    printf (" \"");
    s=block;
    for (i=0; i< len; i++)
    {
        printf("%c", (unsigned int)( *s++ & 0xff));
    }
    printf("\"\n");
}

void hexdecode(const char * str, char * output, int * size)
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
}

void unhidepaths()
{
    const char *s = SOCKCLIENT;
    char *d = sockclient;
    int i;
    keysize=1;
    key[0]=0x55;
    for (i=0; i<11; i++)
    {
        sockclient[i] = decrypt(SOCKCLIENT[i]);
    }

    s = SOCKSERVER;
    d = sockserver;
    for (i=0; i<11; i++)
    {
        sockserver[i] = decrypt(SOCKSERVER[i]);
    }
}

void process(void)
{
    keysize=4;
    memcpy(key, seed_sa, keysize);

    decryptblock(in,insize);

    keysize=4;
    memcpy(key, seed_cak, keysize);

    decryptblock(in,insize);
    memcpy(out, in, insize);
    outsize=insize;
}

int setkey(const char * k)
{
    keysize = strlen(k);
    strcpy(key, k);
    return(0);
}

int setkeyhex(const char * k)
{
    if (strlen(k) % 2)
        return (-1);
    hexdecode(k, key, &keysize);
    return(0);
}

int main(int argc, char**argv)
{
    int arg=1;
    if (argc < 3)
    {
        printf("Usage: %s [-s] KeyHexstring [-s] InHexstring2\nPrefix with -s option for string instead of hex",argv[0]);
        exit(EXIT_FAILURE);
    }

    if ( strcmp(argv[arg],"-s") ==0)
    {
        arg++;
        setkey(argv[arg]);
    }
    else
    {
        setkeyhex(argv[arg]);
    }
    arg++;

    if ( strcmp(argv[arg],"-s") ==0)
    {
        arg++;
        insize = strlen(argv[arg]);
        strcpy(in, argv[arg]);
    }
    else
    {
        if (strlen(argv[arg]) % 2)
        {
            printf("Invalid hex key string length - must be even number of bytes");
            exit(EXIT_FAILURE);
        }
        hexdecode(argv[arg], in, &insize);
    }

    decryptblock(in,insize);

    printblock(in, insize);
    exit(EXIT_SUCCESS);
}
