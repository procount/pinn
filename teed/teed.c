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
 const char seed_ca[]={0x0a, 0x06, 0xea, 0xce};
 const char seed_cs[]={0x0e, 0x61, 0x27, 0x8c};

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
		setbuf(*fp, NULL);	/* tee must not buffer output. */
		fp++;
		np++;
	};

#if CIPHER
    FILE * fserver;
    FILE * fclient;
    
    unhidepaths();

    unlink(sockclient);
    unlink(sockserver);


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
        }
        else
        {
            keysize=0;
        }
    }

    /* Do our copying */
#else
    keysize=1;
    key[0]=0x55;
#endif
    unlink(sockclient);
    unlink(sockserver);
#endif

    progress=0;
    while(1)
    {
    	fread(&ch, 1, 1, stdin);
        if (feof(stdin))
            break;
#if CIPHER
        ch = decrypt(ch);
#endif
		fp = files;
		do
            fwrite(&ch, 1, 1, *fp);
		while (*++fp);

    }
    return 0;
}

