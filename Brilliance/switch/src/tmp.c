/*******************************************************************
ģ����: swSyncdata
ģ������: ���ݿ����õ��ļ�����ͬ��������������
����    : ����Ÿ
��������: 2007-1-29
ģ����;: ͬ������TCP�������򣬵��пͻ������ӹ���ʱ�������������ת������
          ��Ӧ���е�����ת��Ϊ�����ļ�,
          ת�������ļ���ͨ������$SWITCH_DIR/bin/swLoadfile��ʵ�֣�
          �����µ����ӹ����Ժ󣬶�ȡ�ͻ��˵�����,Ȼ����������
          �Ժ�ѱ�ʾ����ı��ķ��͵��ͻ��ˣ��ض�����.
˵��    : ������Ҫһ������������ָ����TCP �����Ķ˿�
          
********************************************************************/

/********************************************************************
���ĸ�ʽ˵��
����:
ǰ�ĸ��ֽڣ�      ���ĳ���   ���ĳ���ָ���Ǻ���ı��ĵ��ַ�������
�������ĸ��ֽ�    ��������   0000-����ȫ����,0001-���²��ֱ�
�����������÷ֺŷָ����ı��� ����1;����2;...   (ֻ�и��²��ֱ��ʱ�����Ҫ�����б�)


�ظ�:
ǰ�ĸ��ֽڣ�      ���ĳ���   ���ĳ���ָ���Ǻ���ı��ĵ��ַ�������
�������ĸ��ֽ�    �ظ���     0000-����ȫ���ɹ�,0001-�в��ֱ�ʧ��,0002-ȫ��ʧ��
�����������÷ֺŷָ����ı��� ����1;����2;...   (ֻ�и��²��ֱ�ʧ�ܵ�ʱ�����Ҫ�����б�)
*******************************************************************/

#include "swapi.h"
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* ����������  */
#define SYNC_REQUEST_ALL_TABLE  "0000"       /* �������еı� */
#define SYNC_REQUEST_TABLE      "0001"       /* ָ���ı�   */
/* Ӧ���� */
#define SYNC_RESP_ALLOK         "0000"       /* ���еı���ɹ� */
#define SYNC_RESP_NOTALL        "0001"       /* �в��ֱ�û�гɹ� */
#define SYNC_RESP_ALLFAIL       "0002"       /* ���еı���ʧ�� */

int igRun=1;
char agImage[1024];
void sigproc(int iSig);

int _swCliProc(int fd);

int main(int argc, char* argv[])
{
  int ilSock;
  int ilCliSock;
  int ilRet;
  int ilPort;
  int ilLen;
  pid_t ilPid;
  struct sockaddr_in slAddr,slCliAddr;
  struct pollfd slListen[1];

  memset(agImage,0,sizeof(agImage));
  sprintf(agImage,"%s/bin/swLoadfile",getenv("SWITCH_DIR"));
  
  sprintf(agDebugfile,"swSyncdata.debug");
 
	/* main ������Ҫһ��������ָ��TCP �����Ķ˿� */ 
  if( argc==1)
  {
    swVdebug(1,"S0010:������������û��ָ�������˿ں�");
    return -1;
  }
  
  ilPort=atoi(argv[1]);
  
  if(signal(SIGTERM,sigproc)==SIG_ERR)
  {
    swVdebug(1,"S0020:�趨��Ϣ����������,:%s",strerror(errno));
    return -1;
  }
  

  ilSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  
  if(ilSock<0)
  {
    swVdebug(1,"S0030:����socket() ����,%s\n",strerror(errno));
    return -1;
  }
  
  slAddr.sin_port=htons((short)ilPort);
  slAddr.sin_family=AF_INET;
  slAddr.sin_addr.s_addr=htonl(INADDR_ANY);
  
  ilRet=bind(ilSock,(struct sockaddr*) &slAddr,sizeof(struct sockaddr_in));
  if(ilRet)
  {
    swVdebug(1,"S0040:��socket ���˿� %d ���󣬴�����Ϣ:%s",ilPort,strerror(errno));
    return -1;
  }
  
  ilRet=listen(ilSock,5);
  if(ilRet==-1)
  {
    swVdebug(1,"listen �������ô���,%s",strerror(errno));
    return -1;
  }
  
  slListen[0].fd=ilSock;
  slListen[0].events=POLLIN;
  
  while(1)
  {
    if(poll(slListen,1,1000)>0)
    {
      ilCliSock=accept(ilSock,(struct sockaddr*)&slCliAddr,&ilLen);
      if(ilCliSock==-1) /* some error happend */
      {
        swVdebug(1,"S0050:%s",strerror(errno));
      }
      else
      {
        
        _swCliProc(ilCliSock);
        /*
        ilPid=fork();
        if(ilPid==0)
        {
          close(ilSock);
          
          exit(0);
        }
        else if(ilPid>0)
        {
          close(ilCliSock);
        }
        else
        {
          close(ilCliSock);
          swVdebug(1,"fork() ���ô���");
        }
        //*/
        
      }
    }
    
    if(igRun==0)
    {
      close(ilSock);
      break;
    }
  }
  
  return 0;
}

/* �ź�SIGTERM������ */
void sigproc(int iSig)
{
  signal(SIGTERM,SIG_IGN);
  igRun=0;
}


int _swCliProc(int fd)
{
char alBuf[4096];
char alSend[4096];
char alTmp[1024];
int ilRecLen=0;
int ilRecMsgLen=-1;
int ilRet;
int ilStatus;
int ilResult;
int ilFail;
int ilSendLen;
char* plTables;
char* plTable;
char alLen[5];
pid_t ilPid;
struct pollfd slFd[1];


  slFd[0].fd=fd;
  slFd[0].events=POLLIN ;
  memset(alBuf,0,sizeof(alBuf));
  while(1)
  {
    if(poll(slFd,1,1000)>0)
    {
      if((slFd[0].revents & POLLIN )==POLLIN)
      {
        ilRet=read(fd,alBuf+ilRecLen,sizeof(alBuf)-ilRecLen);
        printf("read %d bytes\n",ilRet);
        if (ilRet<=0)
        {
          swVdebug(1,"S0060:%d",ilRet);
          return 0;
        }
        ilRecLen+=ilRet;
        
      }
      if(ilRecLen>=4)
      {
        memset(alLen,0,sizeof(alLen));
        memcpy(alLen,alBuf,4);
        ilRecMsgLen=atoi(alLen);
      }
    }
    
    if(ilRecMsgLen!=-1)
    {
      if(ilRecMsgLen==(ilRecLen-4))
        break;
    }
  }
  memset(alTmp,0,sizeof(alTmp));
  memcpy(alTmp,alBuf+4,4);
  
  if(strcmp(alTmp,"0000")==0) /* ����������еı� */
  {
    ilPid=fork();
    if(ilPid==0)
    {
      /* �ӽ�����execl ��������ת������ */
      close(fd);
      ilRet=execl(agImage,"Loadfile",NULL);
      if(ilRet==-1)
      {
        exit(1);
      }
        
    }
    else if( ilPid>0)
    {
      wait(&ilStatus);
      
      if(WIFEXITED(ilStatus))
      {
        ilRet=WEXITSTATUS(ilStatus);
        if(ilRet==0)
          ilResult=0;
        else
          ilResult=1;
      }
      else
      {
        ilResult=1;
      }
    }
    else
    {
      ilResult=1;
      swVdebug(1,"���ü��ع���ʱfork() ʧ��.");
    }
    if(ilResult==0)
      sprintf(alSend,"00000000");
    else
      sprintf(alSend,"00000002");
  }
  else if(strcmp(alTmp,"0001")==0)  /* ���²��ֱ� */
  {
    memset(alSend,0,sizeof(alSend));
    memset(alSend,'1',8);
    plTables=alBuf+8;
    plTable=strtok(plTables,";");
    while(plTable!=NULL)
    {
      ilPid=fork();
      if(ilPid==0)
      {
        close(fd);
        ilRet=execl(agImage,"Loadfile",plTable,NULL);
        if(ilRet==-1)
        {
          
          exit(1);
        }
      }
      else if( ilPid>0)
      {
        wait(&ilStatus);
        if(WIFEXITED(ilStatus))
        {
          ilRet=WEXITSTATUS(ilStatus);
          if(ilRet==0)
            ilFail=0;
          else
            ilFail=1;
        }
        else
          ilFail=1;
      }
      else
        ilFail=1;
        
      if(ilFail)
      {
        if(strlen(alSend)==8)
          sprintf(alSend,"%s%s",alSend,plTable);
        else
          sprintf(alSend,"%s;%s",alSend,plTable);
      }
      plTable=strtok(NULL,";");
    }
    
    if(strlen(alSend)==8)
      memcpy(alSend+4,"0000",4);
    else
      memcpy(alSend+4,"0001",4);
    
    
    
  }
  sprintf(alLen,"%04d",strlen(alSend)-4);
  memcpy(alSend,alLen,4);
  ilSendLen=0;
  while(ilSendLen<=strlen(alSend))
  {
    ilRet=write(fd,alSend+ilSendLen,strlen(alSend)-ilSendLen);
    if (ilRet<=0)
    {
      swVdebug(1,"write()ʧ��.");
      break;
      
    }
    ilSendLen+=ilRet;
  }
  
  close(fd);
  return 0;
}
