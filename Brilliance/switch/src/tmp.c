/*******************************************************************
模块编号: swSyncdata
模块名称: 数据库配置到文件配置同步程序侦听程序
作者    : 王海鸥
建立日期: 2007-1-29
模块用途: 同步配置TCP侦听程序，当有客户端连接过来时，根据请求调用转化程序将
          相应表中的配置转化为配置文件,
          转换配置文件是通过调用$SWITCH_DIR/bin/swLoadfile来实现，
          当有新的连接过来以后，读取客户端的请求,然后处理，处理完
          以后把表示结果的报文发送到客户端，关断连接.
说明    : 程序需要一个启动参数，指定了TCP 侦听的端口
          
********************************************************************/

/********************************************************************
报文格式说明
请求:
前四个字节，      报文长度   报文长度指的是后面的报文的字符串长度
接下来四个字节    请求类型   0000-更新全部表,0001-更新部分表
接下来的是用分号分隔开的表名 表名1;表名2;...   (只有更新部分表的时候才需要表名列表)


回复:
前四个字节，      报文长度   报文长度指的是后面的报文的字符串长度
接下来四个字节    回复码     0000-更新全部成功,0001-有部分表失败,0002-全部失败
接下来的是用分号分隔开的表名 表名1;表名2;...   (只有更新部分表失败的时候才需要表名列表)
*******************************************************************/

#include "swapi.h"
#include <errno.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>

/* 定义请求定义  */
#define SYNC_REQUEST_ALL_TABLE  "0000"       /* 加载所有的表 */
#define SYNC_REQUEST_TABLE      "0001"       /* 指定的表   */
/* 应答定义 */
#define SYNC_RESP_ALLOK         "0000"       /* 所有的表处理成功 */
#define SYNC_RESP_NOTALL        "0001"       /* 有部分表没有成功 */
#define SYNC_RESP_ALLFAIL       "0002"       /* 所有的表处理都失败 */

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
 
	/* main 函数需要一个参数，指定TCP 侦听的端口 */ 
  if( argc==1)
  {
    swVdebug(1,"S0010:启动参数错误，没有指定侦听端口号");
    return -1;
  }
  
  ilPort=atoi(argv[1]);
  
  if(signal(SIGTERM,sigproc)==SIG_ERR)
  {
    swVdebug(1,"S0020:设定消息处理函数错误,:%s",strerror(errno));
    return -1;
  }
  

  ilSock=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
  
  if(ilSock<0)
  {
    swVdebug(1,"S0030:调用socket() 错误,%s\n",strerror(errno));
    return -1;
  }
  
  slAddr.sin_port=htons((short)ilPort);
  slAddr.sin_family=AF_INET;
  slAddr.sin_addr.s_addr=htonl(INADDR_ANY);
  
  ilRet=bind(ilSock,(struct sockaddr*) &slAddr,sizeof(struct sockaddr_in));
  if(ilRet)
  {
    swVdebug(1,"S0040:绑定socket 到端口 %d 错误，错误消息:%s",ilPort,strerror(errno));
    return -1;
  }
  
  ilRet=listen(ilSock,5);
  if(ilRet==-1)
  {
    swVdebug(1,"listen 函数调用错误,%s",strerror(errno));
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
          swVdebug(1,"fork() 调用错误");
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

/* 信号SIGTERM处理函数 */
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
  
  if(strcmp(alTmp,"0000")==0) /* 如果更新所有的表 */
  {
    ilPid=fork();
    if(ilPid==0)
    {
      /* 子进程用execl 函数调用转换工具 */
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
      swVdebug(1,"调用加载工具时fork() 失败.");
    }
    if(ilResult==0)
      sprintf(alSend,"00000000");
    else
      sprintf(alSend,"00000002");
  }
  else if(strcmp(alTmp,"0001")==0)  /* 更新部分表 */
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
      swVdebug(1,"write()失败.");
      break;
      
    }
    ilSendLen+=ilRet;
  }
  
  close(fd);
  return 0;
}
