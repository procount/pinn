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
#define DAEMON 1

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

int main(void) 
{
        struct stat fstatus;

#if DAEMON
        /* Our process ID and Session ID */
        pid_t pid, sid;

        /* Fork off the parent process */
        pid = fork();
        if (pid < 0) 
        {
                exit(EXIT_FAILURE);
        }
        /* If we got a good PID, then
           we can exit the parent process. */
        if (pid > 0) 
        {
                exit(EXIT_SUCCESS);
        }

        /* Change the file mode mask */
        umask(0);

        /* Open any logs here */

        /* Create a new SID for the child process */
        sid = setsid();
        if (sid < 0) 
        {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }

        /* Change the current working directory */
        if ((chdir("/")) < 0) 
        {
                /* Log the failure */
                exit(EXIT_FAILURE);
        }

        /* Close out the standard file descriptors */
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
#endif
        /* Daemon-specific initialization goes here */

        FILE * fserver;
        FILE * fclient;

        unhidepaths();

        unlink(sockclient);
        unlink(sockserver);
        

        /* The Big Loop */
        while (1) 
        {
            /* Do some task here ... */
            sleep(1); /* wait 1 second */

            if (access( sockserver, F_OK) ==-1)
                continue;

            fserver = fopen(sockserver,"rb");
            if (fserver)
            {
                do
                {
                    stat(sockserver, &fstatus);
                } while (fstatus.st_size <4);
                fread(&insize, 1, sizeof(insize), fserver);
                fprintf(stderr,"D got %u\n", insize);

                do
                {
                    stat(sockserver, &fstatus);
                } while (fstatus.st_size <4+insize);
                fread(in, 1, insize, fserver);
                fclose (fserver);
                unlink(sockserver);

                /* Process the request */
                process();

                /* Output the response */
                fclient = fopen(sockclient,"wb");
                if (fclient)
                {
                    fwrite(&outsize, 1, sizeof(outsize), fclient);
                    fwrite(out,1, outsize, fclient);
                    fclose(fclient);
                }
            }
       }
       exit(EXIT_SUCCESS);
}
