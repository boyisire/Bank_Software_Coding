#include "switch.h"

#include "swNdbstruct.h"
#define LOCKFILE "lockfile"

int TcpSrv(short iPort,int *pSock);
int TcpCli(char *aAddr,short iPort,int *pSock);
int TcpServe(int sock,int *piCliSock,char *aCliaddr);
int swTcpPut(int sock,char *aMsgpack,unsigned int iMsglen);
int TcpGet(int sock,char *aMsgpack,unsigned int *piMsglen,short iTimeout);
int swTcpGet(int sock,char *aMsgpack,unsigned int *piMsglen,short iTimeout);
int swGetcfgval(char *aFldname,char *aFldval);
void swClsprocess(int sockfd);

void swQuit(int sock);
int swMovesign(char *aBuf);
extern char agPath[101];

int main(int argc,char *argv[])
{
  int    sockfd,ilRc,sock;
  char   alAddr[20],alVal[101];
  short  ilPort=0;
  char   alCmd[200];  /* add by zjj  */
  char	 alMsgbuf[1024];
  unsigned int ilLen;
  int    clTmp;  /* add by nh 20020924 */

  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swSyscom Version 4.3.0",argv[1]);

  /* LICENSE 
  if (_swLicense("swSyscom")) exit(FAIL);
  */

  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
  signal(SIGTERM,swQuit);

  clTmp=0;
  agPath[0]=0;
  while((clTmp=getopt(argc,argv,"p:d:")) != -1)
  {
    switch(clTmp)
    {
      case 'p': /* 定义端口号 */
        ilPort=atoi(optarg);
        break;
      case 'd':  /* 定义配置文件路径 */
        strcpy(agPath,optarg);
        break;
      default:
        break;
    }
  }
  /* 设置调试程序名称 */
  strcpy(agDebugfile,"swSyscom.debug");

  /* 取得 DEBUG 标志 */
  if((cgDebug=_swDebugflag("swSyscom"))==FAIL) exit(FAIL);
  /* next add by nh 20020924 */
  swVdebug(2,"S0010: port=[%d],path=[%s]\n",ilPort,agPath);
  if(agPath[0] == 0)
  {
    strcpy(agPath,getenv("SWITCH_CFGDIR"));
    swVdebug(2,"S0020: getenv path=[%s]",agPath);
    swMovesign(agPath);
    swVdebug(2,"S0030: 处理后的路径为=[%s]",agPath);
  }
  if(ilPort == 0)
  {
    ilRc = swGetcfgval("SYSCOM_PORT",alVal); 
    if(ilRc)
    {
      swVdebug(0,"S0040:从[SWCONFIG.CFG]取syscom端口出错!ilRc = [%d]",ilRc);
      swQuit(sock);
    }
    ilPort = atoi(alVal);
  }
  swVdebug(2,"S0050: 取端口成功! PORT = [%d]",ilPort);
  /* end add by nh 20020924 */
  ilRc = TcpSrv(ilPort,&sock);
  if(ilRc)
  {
    swVdebug(0,"S0060: 创建Socket出错! ilRc = [%d]",ilRc);
    swQuit(sock);
  }
  swVdebug(2,"S0070: 创建 Socket 成功!");
  
  /* modify by nh 20020924
  sprintf(alCmd,"rm -f %s/config/%s",getenv("SWITCH_CFGDIR"),LOCKFILE);*/
  sprintf(alCmd,"rm -f %s/config/%s",agPath,LOCKFILE);
  system(alCmd);
  for(;;)
  {
    ilRc = TcpServe(sock,&sockfd,alAddr);
    
    if(ilRc)
    {
      swVdebug(1,"S0080: 连接(accept)错误!! -- code = ",ilRc); 
      continue;
    }
    swVdebug(2,"S0090: 连接(accept)成功!"); 
    switch(fork())
    {
      case -1:
        close(sockfd);
        break; 
      case 0:
        close(sock); 
        swVdebug(2,"S0100: 子进程[%d]开始运行!",getpid());
        swClsprocess(sockfd); 
        swVdebug(2,"S0110: 子进程[%d]退出!",getpid());
        ilRc = TcpGet(sockfd,alMsgbuf,&ilLen,0);       
        close(sockfd);
        exit(0);
      default:
        close(sockfd);
        break;
    }
  }
}
int swMovesign(char *aBuf)
{
  int i=0;

  while(aBuf[i] != '\0')
  {
    if(aBuf[i] == ':')
      aBuf[i]=0;
    i++;
  }
  return(0);
}

