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
void bmqSend_t(struct igMail *mail)
{
    int ilSourq,ilDesq;
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

    ilRc = bmqPut(0,ilDesq,12,12,12,putBuf,iMsglength);

    if(ilRc)
    {
        bmqClose();
        fprintf(stderr,"put error!\n");
        exit(0);
    }
    else
        printf("send message success!\n");

    ilRc = bmqClose();

}

int main(int argc, char *argv[])
{
    if(argc != 3)
    {
        printf("usage: test source target\n");
        exit(0);
    }

    int ilSourq,ilDesq;
    struct igMail mail;
    ilSourq = atoi(argv[1]);
    printf("ilSourq = %d\n",ilSourq);
    ilDesq = atoi(argv[2]);
    mail.ilSourq = ilSourq;
    mail.ilDesq = ilDesq;

    int pronum = 20;
    int i;
    int ret;
    for(i = 0; i < pronum; i++)
    {
         ret = fork();
         printf("第%d个子进程！\n",i);
        if(ret == -1 || ret == 0)
            break;
    }
    if(-1 == ret)
    {
        printf("error\n");
        exit(-1);
    }
    else if(0 == ret)
    {
        bmqSend_t(&mail);
    }


    return 0;
}
