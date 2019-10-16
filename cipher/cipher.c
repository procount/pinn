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

#define SOCKSERVER "/tmp/server"
#define SOCKCLIENT "/tmp/client"

#define MAXMSG  32
char out[MAXMSG]={0};
char in[MAXMSG]={0};
size_t outsize=0;
size_t insize=0;
size_t progress=0;
size_t keysize=0;
char key[MAXMSG]={0};

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

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
    progress = (progress+1)%keysize;

    return(ch);
}

int main()
{
    int ch;
    FILE * fserver;
    FILE * fclient;

    unlink(SOCKCLIENT);
    unlink(SOCKSERVER);

    strcpy(out, "Hello");
    outsize=strlen(out)+1;
    fserver = fopen(SOCKSERVER,"wb");
    if (fserver)
    {
        fwrite(&outsize, 1, sizeof(outsize), fserver);
        fwrite(out,1, outsize, fserver);
        fclose(fserver);

        sleep(2);
        int elapsed=0;
        while ( (elapsed <3) && (access( SOCKCLIENT, F_OK ) == -1) )
        {
            sleep(1);
            elapsed++;
        }
        sleep(1);

        fclient = fopen(SOCKCLIENT,"rb");
        if (fclient)
        {
            fread(&insize, 1, sizeof(insize), fclient);
            fread(in, 1, insize, fclient);
            fclose (fclient);
        }
        unlink(SOCKCLIENT);
    }

    /* Do our copying */
    process_key();

    progress=0;
    while(1)
    {
    	fread(&ch, 1, 1, stdin);
        if (feof(stdin))
            break;

        ch = decrypt(ch);

        fwrite(&ch, 1, 1, stdout);
    }
    return 0;
}

