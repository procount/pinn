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
 char key[MAXMSG]={0};
 char sockserver[12];
 char sockclient[12];

#include "busyboc.h"

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
}

void customread(const char * key, char * output, int maxlen)
{
    FILE *fp;
    int status;
    char cmd[128];
    strcpy(cmd, "custom ");
    strcat(cmd, key);
    *output='\0';

    fp = popen(cmd, "r");
    if (fp)
    {
        if ( fgets(output, maxlen, fp) == NULL)
            *output='\0';
        pclose(fp);
    }
}

void customreadhex(const char * key, char * out, size_t * len)
{
    size_t i;
    char raw[200];

    customread(key,raw,200);
    hexdecode(raw, out, len);
    for (i=0; i<*len; i++)
        out[i]^=0x55;
    out[i]='\0';
}

void unhidepaths()
{
    size_t len;

    customreadhex(BdhuPmBf, sockserver, &len);
    customreadhex(QynRlYS, sockclient, &len);
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

    if (*sockclient)
        unlink(sockclient);
    if (*sockserver)
        unlink(sockserver);

#if CIPHER==1

    //Check if "custom zxcv" returns vcxz
    // if not, skip this and behave like 'tee'

    gettimeofday(&tv[3], NULL);

    outsize=KEYSIZE;
    memcpy(out,&tv[0],outsize);

    keysize=KEYSIZE;
    memcpy(key, seed_ca,keysize);
    decryptblock(out,outsize);

    keysize=0;
    if ((*sockserver) && (*sockclient))
    {
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
        }
    }

    /* Do our copying */
#else   //CIPHER==2
    keysize=1;
    key[0]=0x55;
#endif
    if (*sockclient)
        unlink(sockclient);
    if (*sockserver)
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
        for (i=0; i<c; i++)
        {
            if (keysize)
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

