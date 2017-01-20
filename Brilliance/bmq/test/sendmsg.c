#include <stdio.h>
#include "bmq.h"
struct igMail
{
    int ilSourq;
    int ilDesq;
};
//#define MAXLEN 40000
#define MAXLEN 65536

int readfile(char *buf)
{
    char *name = "tt.txt";
    FILE *fp = fopen(name,"r");
    fseek(fp,0,SEEK_END);
    unsigned int ilen = ftell(fp);
    printf("len is:%d\n",ilen);
    fseek(fp, 0, SEEK_SET);
    ilen = fread(buf, sizeof(char), ilen, fp);
    fclose(fp);
    return 0;
}



void bmqSend_t(struct igMail *mail, long count)
{
    int i, ilSourq,ilDesq, flag;
    ilSourq = mail->ilSourq;
    ilDesq = mail->ilDesq;

    char putBuf[MAXLEN];
    bzero(putBuf,sizeof(putBuf));

    readfile(putBuf);
    //strcpy(putBuf,"这是我的一段测试内容");
    unsigned int  iMsglength = strlen(putBuf);

    int ilRc = bmqOpen(ilSourq);
    if(ilRc)
    {
        fprintf(stderr,"open error!\n");
        exit(0);
    }
    else
        printf("open success!\n");
    flag =0;
    for(i =0; i< count ; ) 
    {
	ilRc = bmqPut(0,ilDesq,12,12,12,putBuf,iMsglength);
	if(ilRc)
    	{

#ifdef SHM_LIMITED
		if( ilRc ==  SHM_USED_UP ) 
		{	
			printf("shm used up\n");
//			fllush(stdout);
			continue;
		}
#endif
        	bmqClose();
        	fprintf(stderr,"put error!\n");
        	exit(0);
    	}
        i++;
/*
    	else
        	printf("send message success!\n");
*/
    }

    ilRc = bmqClose();
}

int main(int argc, char *argv[])
{
    if(argc != 4)
    {
        printf("usage: test source target\n");
        exit(0);
    }
 //   printf("1 11111111\n");

    int ilSourq,ilDesq;
    long snd_count;
    struct igMail mail;
  //  printf("2 22222222222222\n");
//    printf("argv[1] is %s\n",argv[1]);
    ilSourq = atoi(argv[1]);
    printf("ilSourq....... = %d\n",ilSourq);
    ilDesq = atoi(argv[2]);
    snd_count = atoi(argv[3]);
   // printf("before mail\n");
    mail.ilSourq = ilSourq;
    mail.ilDesq = ilDesq;
    bmqSend_t(&mail, snd_count);
    return 0;
}
