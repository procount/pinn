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
 
 char seed_c[MAXMSG];
 char seed_s[MAXMSG];
 char seed_a[MAXMSG];
 char seed_k[MAXMSG];

 size_t seed_c_size=0;
 size_t seed_s_size=0;
 size_t seed_a_size=0;
 size_t seed_k_size=0;

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

void printinput(FILE * fp, char * block, int len)
{
    int i;
    char * s=block;
    progress=0;
    for (i=0; i< len; i++)
    {
        fprintf(fp,"%02x", (unsigned int)( *s++ & 0xff));
    }
    fprintf(fp,"\n");

}


void printblock(FILE * fp, char * block, int len)
{
    int i;
    char * s=block;
    progress=0;

    fprintf (fp," {");
    s=block;
    for (i=0; i< len; i++)
    {
        fprintf(fp,"0x%02x", (unsigned int)( *s++ & 0xff));
        if (i<len-1) fprintf(fp,",");
    }
    fprintf(fp,"};\n");
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

int main(int argc, char**argv) 
{
    char buffer[MAXMSG];
    FILE * fp;
    File * finput;

    printf("Enter seed_c (plain hex): ");
    scanf("%s",buffer);
    hexdecode(buffer, seed_c, &seed_c_size);

    printf("Enter seed_s (plain hex): ");
    scanf("%s",buffer);
    hexdecode(buffer, seed_s, &seed_s_size);

    printf("Enter seed_a (plain hex): ");
    scanf("%s",buffer);
    hexdecode(buffer, seed_a, &seed_a_size);

    printf("Enter seed_k (plain hex): ");
    scanf("%s",buffer);
    hexdecode(buffer, seed_k, &seed_k_size);

    fp = fopen("busyboc.h", "wb");

    memcpy(in, seed_c, seed_c_size);
    insize=seed_c_size;
    memcpy(key, seed_a, seed_a_size);
    keysize=seed_a_size;
    decryptblock(in,insize);
    fprintf(fp,"const char seed_ca[KEYSIZE]= ");
    printblock(fp, in, insize);

    memcpy(in, seed_c, seed_c_size);
    insize=seed_c_size;
    memcpy(key, seed_s, seed_s_size);
    keysize=seed_s_size;
    decryptblock(in,insize);
    fprintf(fp,"const char seed_cs[KEYSIZE]= ");
    printblock(fp, in, insize);

    fclose (fp);

    fp = fopen("daemon.h", "wb");
    finput = fopen("input", "wb");

    memcpy(in, seed_s, seed_s_size);
    insize=seed_s_size;
    memcpy(key, seed_a, seed_a_size);
    keysize=seed_a_size;
    decryptblock(in,insize);
    fprintf(fp,"const char seed_sa[KEYSIZE]= ");

    printblock(fp, in, insize);
    printinput(finput, in, insize);

    memcpy(in, seed_k, seed_k_size);
    insize=seed_k_size;
    memcpy(key, seed_c, seed_c_size);
    keysize=seed_c_size;
    decryptblock(in,insize);
    memcpy(key, seed_a, seed_a_size);
    keysize=seed_a_size;
    decryptblock(in,insize);
    fprintf(fp, "const char seed_cak[KEYSIZE]= ");
 
    printblock(fp, in, insize);
    printinput(finput, in, insize);

    fclose (input);
    fclose (fp);
    exit(EXIT_SUCCESS);
}
