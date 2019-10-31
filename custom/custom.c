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

#define MAXKEYS 1

const char k0[]="curl";
const char k1[]=""; //End of table

const char * table[MAXKEYS]={
    k0,k1
};

const char r0[]="http://downloads.raspberrypi.org/os_list_v3.json";
const char r1[]=""; //End of table

const char * responses[MAXKEYS]={
    r0,r1
};


printblock(const char * block)
{
    int i;
    int len=strlen(block);

    for (i=1; i<len+1; i++)
    {
        printf("%02x", (unsigned int) ((block[i]^0x55)&0xff));
    }
}

printerror()
{
    printf("55");
}

int lookup_key(const char * key, int len)
{
    int i=0;
    for (i=0; i<MAXKEYS; i++)
    {
        if (*table[i]=="\0")
            return -1;  //End of table reached
        int match=memcmp(key, table[i], len+1);
        if (match==0)
        {
            return(i);
        }
    }
    return -1;
}

void query (const char * key)
{
    int index;
    FILE * fp;

    index=lookup_key(key,strlen(key));
    if ((index >=0) && (index < MAXKEYS))
    {
        printblock(responses[index]);
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

    query(argv[1]);
}
