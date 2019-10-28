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
 const char seed_ca_hex[]="0a06eace";
 const char seed_cs_hex[]="0e61278c";

 char seed_ca[MAXMSG];
 char seed_cs[MAXMSG];
 size_t seed_ca_size=0;
 size_t seed_cs_size=0;

 const char test_t[]={0x0c, 0x50, 0x1f, 0x02};

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
#endif

int main(int argc, char **argv)
{
    int ch;
	FILE **files;
	FILE **fp;
	char **names;
	char **np;
	char retval;
    struct timeval tv;
    struct stat fstatus;

	/* gnu tee ignores SIGPIPE in case one of the output files is a pipe
	 * that doesn't consume all its input.  Good idea... */
	signal(SIGPIPE, SIG_IGN);

	/* Allocate an array of FILE *'s, with one extra for a sentinel. */
	fp = files = malloc(sizeof(FILE *) * (argc + 2));
    if (!fp)
        error("Cannot allocate memory");
    memset(fp,0,sizeof(FILE *) * (argc + 2));
	np = names = argv - 1;

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

#if CIPHER
    FILE * fserver;
    FILE * fclient;

    unhidepaths();

    unlink(sockclient);
    unlink(sockserver);

    hexdecode(seed_ca_hex,seed_ca,&seed_ca_size);
    hexdecode(seed_cs_hex,seed_cs,&seed_cs_size);

#if CIPHER==1
    gettimeofday(&tv, NULL);

    outsize=4;

    memcpy(out,&tv,outsize);

    keysize=4;
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

            keysize=4;
            memcpy(key, seed_ca, keysize);
            decryptblock(in,insize);
            memcpy(key, seed_cs, keysize);
            decryptblock(in,insize);
            memcpy(key, &tv, 4);
            decryptblock(in,insize);
            memcpy(key, in, insize);
            keysize=insize;
        }
        else
        {
            keysize=0;
        }
    }

    /* Do our copying (cipher==2) */
#else
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
		for (i=0; i<c; i++)
		{
			buf[i] ^= key[progress];
            progress = (progress+1)%keysize;
		}

#endif
        do {
            fwrite(buf, 1, c, *fp);
        } while (*++fp);

        if (feof(stdin))
            break;
    }
    if (c < 0) {            /* Make sure read errors are signaled. */
        retval = EXIT_FAILURE;
    }

    return 0;
}

