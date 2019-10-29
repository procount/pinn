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


#define CIPHER 1
#define BLOCK_IO 1
#define KEYSIZE 20

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
 const char seed_ca[KEYSIZE]= {0x46,0x3a,0x90,0xd4,0xaf,0x4a,0xaa,0x37,0x92,0x02,0xeb,0x80,0xfb,0xa0,0x9f,0x44,0xec,0xc8,0x7f,0x82};
 const char seed_cs[KEYSIZE]= {0x94,0x80,0xde,0x94,0xf7,0x35,0xc7,0x53,0x88,0x6a,0x40,0xd4,0xae,0x21,0x21,0x66,0xb3,0x1b,0xff,0xd3};

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
#endif

int main(int argc, char **argv)
{
    int ch;
    FILE **files;
    FILE **fp;
    char **names;
    char **np;
    char retval;
    struct timeval tv[4];
    struct stat fstatus;

    gettimeofday(&tv[0], NULL);

    if (4*sizeof(struct timeval) < KEYSIZE)
    {
        printf("tv size is not big enough\n");
        exit(EXIT_FAILURE);
    }
    /* gnu tee ignores SIGPIPE in case one of the output files is a pipe
     * that doesn't consume all its input.  Good idea... */
    signal(SIGPIPE, SIG_IGN);

    /* Allocate an array of FILE *'s, with one extra for a sentinel. */
    fp = files = malloc(sizeof(FILE *) * (argc + 2));
    if (!fp)
        error("Cannot allocate memory");
    memset(fp,0,sizeof(FILE *) * (argc + 2));
    np = names = argv - 1;

    gettimeofday(&tv[2], NULL);

    files[0] = stdout;
    setbuf(files[0], NULL);	/* tee must not buffer output. */
    fp++;
    np++;
    argv++;
    while (*argv)
    {
        *fp = fopen(*argv, "w");
        if (*fp == NULL)
        {
            error("Cannot open output file");
        }
        *np = *argv++;
        //setbuf(*fp, NULL);	/* tee must not buffer output. */
        fp++;
        np++;
    };
    gettimeofday(&tv[1], NULL);

#if CIPHER
    FILE * fserver;
    FILE * fclient;

    unhidepaths();

    unlink(sockclient);
    unlink(sockserver);


#if CIPHER==1
    gettimeofday(&tv[3], NULL);

    outsize=KEYSIZE;
    memcpy(out,&tv[0],outsize);

    keysize=KEYSIZE;
    memcpy(key, seed_ca,keysize);
    decryptblock(out,outsize);

    fserver = fopen(sockserver,"wb");
    if (fserver)
    {
        fwrite(&outsize, 1, sizeof(outsize), fserver);
        fwrite(out,1, outsize, fserver);
        fclose(fserver);

        int elapsed=0;
        while ( (elapsed <30) && (access( sockclient, F_OK ) == -1) )
        {
            usleep(100000);
            elapsed++;
        }
        insize=0;

        fclient = fopen(sockclient,"rb");
        if (fclient)
        {
            do
            {
                stat(sockclient, &fstatus);
            } while (fstatus.st_size <4);
            fread(&insize, 1, sizeof(insize), fclient);

            do
            {
                stat(sockclient, &fstatus);
            } while (fstatus.st_size <4+insize);
            fread(in, 1, insize, fclient);

            fclose (fclient);

            keysize=KEYSIZE;
            memcpy(key, seed_ca, keysize);
            decryptblock(in,insize);
            memcpy(key, seed_cs, keysize);
            decryptblock(in,insize);
            memcpy(key, &tv, KEYSIZE);
            decryptblock(in,insize);
            memcpy(key, in, insize);
            keysize=insize;
        }
        else
        {
            keysize=0;
        }
    }

    /* Do our copying */
#else   //CIPHER==2
    keysize=1;
    key[0]=0x55;
#endif
    unlink(sockclient);
    unlink(sockserver);

    progress=0;
#endif

    char buf[1024];
    size_t i;
    size_t c;

    while ((c = fread(buf, 1, sizeof(buf), stdin)) > 0)
    {
        fp = files;
#if CIPHER
        if (keysize)
        {
            for (i=0; i<c; i++)
            {
                buf[i] ^= key[progress];
                progress = (progress+1)%keysize;
            }
        }
#endif
        do {
            fwrite(buf, 1, c, *fp);
        } while (*++fp);

        if (feof(stdin))
    	    break;
    }
    if (c < 0) {            /* Make sure read errors are signaled. */
        return(EXIT_FAILURE);
    }

    return 0;
}

