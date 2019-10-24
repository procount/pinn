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

#define CIPHER 1

#if CIPHER
 #define MAXMSG  32
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
#endif

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

#if CIPHER
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
    ch ^= key[progress];
    if (keysize)
        progress = (progress+1)%keysize;

    return(ch);
}
#endif

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

int main()
{
    int ch;
#if CIPHER
    FILE * fserver;
    FILE * fclient;

    unhidepaths();

    unlink(sockclient);
    unlink(sockserver);

#if CIPHER==1
    strcpy(out, "Hello");
    outsize=strlen(out)+1;
    fserver = fopen(sockserver,"wb");
    if (fserver)
    {
        fwrite(&outsize, 1, sizeof(outsize), fserver);
        fwrite(out,1, outsize, fserver);
        fclose(fserver);

        sleep(2);
        int elapsed=0;
        while ( (elapsed <3) && (access( sockclient, F_OK ) == -1) )
        {
            sleep(1);
            elapsed++;
        }
        sleep(1);

        fclient = fopen(sockclient,"rb");
        if (fclient)
        {
            fread(&insize, 1, sizeof(insize), fclient);
            fread(in, 1, insize, fclient);
            fclose (fclient);
        }
    }

    /* Do our copying */
    process_key();
#else
    keysize=1;
    key[0]=0x55;
    progress=0;
#endif
    unlink(sockclient);
    unlink(sockserver);
#endif

    while(1)
    {
    	fread(&ch, 1, 1, stdin);
        if (feof(stdin))
            break;
#if CIPHER
        ch = decrypt(ch);
#endif
        fwrite(&ch, 1, 1, stdout);
    }
    return 0;
}

