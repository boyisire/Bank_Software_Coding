/****************************************************************/
/* 模块编号    ：BMQGRP_RCV                                     */
/* 模块名称    ：跨组级联接收模块                               */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2001/08/08                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：新晨Q 跨组级联接收守护进程                     */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共307行                              */
/****************************************************************/

/*库函数定义*/
#include "bmq.h"
#include <pthread.h>
#define MAX_THREADS 200
#define DEFINE_THREADS 2
/*#define MSGMAXLEN 4096*/  /*最大报文长度*/
#define MSGMAXLEN 7168

int   q_reqid;
char  aReqid[10];
int   thread_nums ;
int   rcv_thread_nums;
int   snd_thread_nums;
static struct sockaddr_in sgSvr_addr;
static struct sockaddr_in sgCli_addr;
static int igSockfd;
static  long    lgSndpid[1024];

pthread_t send_file_thread;
pthread_t recv_file_thread;
pthread_t check_timeout_thread;
pthread_t server_thread[MAX_THREADS],client_thread[MAX_THREADS];
pthread_mutex_t  connsock_mutex,server_lock ;

struct Msgbuf{
long mtype;
long msglen;
char data[MSGMAXLEN+100];
}sMsgbuf;

/* 函数定义 */
void *check_timeout(void *);
void *server_proc(void *);
static  void  _bmqQuit();
/*static int _bmqGrpRecv( int sockfd,char *buffer, short *length); delete by wh*/
static int _bmqGrpRecv( int sockfd,char *buffer, TYPE_SWITCH *length);
static int GetProfileString();

int main(int argc,char **argv)
{
  int     j,ilRc,ilLen;
  int    ilOpt;
  char aIp[32];
  int iPort;
  char alcfname[100];
  pthread_attr_t attr;
  
  
  thread_nums  = 0;

  /* default thread numbers */
  if (thread_nums  == 0)
     thread_nums = DEFINE_THREADS;
  else if (thread_nums > MAX_THREADS)
     thread_nums = MAX_THREADS;

  /*显示新晨Q版本信息*/
  _bmqShowversion(argc,argv);

  /*设置信号*/
  signal(SIGQUIT,SIG_IGN);
  /*** zcb add 2005-08-08 ***/
  signal(SIGTTOU,SIG_IGN);
  signal(SIGTTIN,SIG_IGN);
  
  signal(SIGHUP,SIG_IGN);
  signal(SIGINT,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGSTOP,SIG_IGN);
  signal(SIGUSR1,SIG_IGN);
  signal(SIGUSR2,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
  /*
  signal(SIGTERM,_bmqQuit);
  */
  
  /* 创建守护进程 */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010 %s:创建守护进程失败!",__FILE__);
    exit(FAIL);
  }
 
  /*读入参数配置文件DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0020 %s :载入DEBUG参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }
   
  /*连接共享内存*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030 %s:连接共享内存区出错:%d",__FILE__,ilRc);
    exit(FAIL);
  }

  /*取出本组的IP地址和端口号*/
  ilRc = _bmqConfig_load("GROUP",psgMbshm->iMbgrpid);
  if (ilRc < 0)
  {  
    _bmqDebug("S0040 %s :载入GROUP[%d]参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,psgMbshm->iMbgrpid,getenv("BMQ_PATH"));
    exit(FAIL);
  }
  memset(&sgSvr_addr,0x00,sizeof(struct sockaddr_in));
  memset(&sgCli_addr,0x00,sizeof(struct sockaddr_in));
  
  sgSvr_addr.sin_family = AF_INET;
  /*add by pc 20060302 修改为可以指定IP地址和端口号*/
  if (argc==1) {
	sgSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	sgSvr_addr.sin_port = htons(igBmqgrpport);  	
  }	
  else if (argc==2) {
	strcpy(aIp,argv[1]);
	sgSvr_addr.sin_addr.s_addr = inet_addr(aIp);
	sgSvr_addr.sin_port = htons(igBmqgrpport);    	
  }
  else if (argc==3) {
	strcpy(aIp,argv[1]);
    iPort=atoi(argv[2]);
	sgSvr_addr.sin_addr.s_addr = inet_addr(aIp);
    sgSvr_addr.sin_port = htons(iPort);
    igBmqgrpport=iPort;
  }
  
  /*创建SOCKET*/
  igSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (igSockfd == -1)
  {
    _bmqDebug("S0050 创建SOCKET错误!errno: %d,%s",errno,strerror(errno));
    exit(FAIL);
  }

  ilOpt = 1;
  ilLen = sizeof(ilOpt);
  ilRc = setsockopt(igSockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&ilOpt,ilLen);
  if(ilRc < 0)
  {
    _bmqDebug("S0055 Set Socketopt Error! errno:%d,%s",errno,strerror(errno));
    close( igSockfd );
    exit(FAIL);
  }

  /*绑定SOCKET*/
  if (bind(igSockfd,(struct sockaddr*)&sgSvr_addr,sizeof(struct sockaddr_in))
    == -1)
  {
    _bmqDebug("S0060 绑定SOCKET至端口[%d]错误!! errno: %d,%s",igBmqgrpport,errno,strerror(errno));
    close( igSockfd );
    exit(FAIL);
  }

  if (listen(igSockfd, SOMAXCONN) == -1)
  {
    _bmqDebug("S0070 侦听连接错误!!errno: %d,%s",errno,strerror(errno)); 
    close (igSockfd);
    exit(FAIL);
  }

  sprintf(alcfname,"%s%s",getenv("PT_DIR"),"/etc/bmqgrp.ini");
  if (GetProfileString(alcfname,"BMQGRP","REQID",aReqid) < 0)
  {
     _bmqDebug("读夸组通讯消息队列键值出错![%d]",errno);
     exit(-1);
  }

  _bmqDebug("请求队列键：   aReqid = [%s]", aReqid);

  /*请求消息队列*/
  q_reqid = msgget(atoi(aReqid),IPC_CREAT|0660);
  if (q_reqid < 0){
     _bmqDebug("Resp msgget() error:[%d], 通讯邮箱[%s]!\n",errno,aReqid);
     exit(-1);
  }

  _bmqDebug("打开消息队列成功! reqid=[%d]",q_reqid);
  	

  /*保存进程pid*/
  /*
  psgMbshm->lBmqgrprcv_pid = getpid();
  
  for(i=0;i<1024;i++) lgSndpid[i] = 0;
  i = 0;
  _bmqDebug("S0080 ***跨组级联 接收守护进程 listen[%d]***",igBmqgrpport);
  */
  
  
  ilRc = pthread_attr_init(&attr);
  if ( ilRc==-1 ){
     _bmqDebug("pthread_attr_init error![%d]\n",errno);
     exit(-1);
  }
  ilRc = pthread_attr_setstacksize(&attr,56+16*10);
  if ( ilRc == -1 ){
     _bmqDebug("pthread_attr_setstacksize error![%d]",errno);
     exit(-1);
  }


  pthread_setconcurrency(2*thread_nums +2);
  ilRc = pthread_mutex_init(&connsock_mutex,NULL);
  if ( ilRc==-1 ){
     _bmqDebug("设置共享锁失败![%d]",errno);
     exit(-1);
  }
    
  ilRc = pthread_mutex_init(&server_lock,NULL);
  if ( ilRc==-1 ){
     _bmqDebug("设置服务区锁失败![%d]",errno);
     exit(-1);
  }


  /*Create the thread to check connect socket timeout*/
  /*
  rc = pthread_create(&check_timeout_thread,&attr,check_timeout,NULL);
  if (rc){
     _bmqDebug("Check_timeout_thread created error,rc=%d",rc);
     exit(-1);
  }
  swVdebug(4,"thread_nums=%d",2*thread_nums+1 );
  

  (void)	InitStruct();
  */

  /*Create server thread to accept connect*/
  for (j=0;j<thread_nums ;j++){
     ilRc = pthread_create(&server_thread[j],&attr,server_proc,NULL);
     _bmqDebug( "SERVER THREAD[%7d] CREATED",server_thread[j]);
     if (ilRc ){
        _bmqDebug("Create server process thread Error:%d",ilRc);
        exit(0);
     }
  }

  /*pthread_join(check_timeout_thread,NULL);	*/
  for (j=0;j<thread_nums ;j++)
     pthread_join(server_thread[j],NULL);

  while(1) pause();  

}


/**************************************************************
 ** 函数名      :   serverproc()
 ** 功  能      :   SERVER线程处理函数
 ** 作  者      :
 ** 建立日期    :   2001/09/03
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :   Conn:       输入：内部结构
 ** 返回值      :   无
***************************************************************/

void * server_proc(void *args)
{
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
   char   alMsgbuf[MSGMAXLEN+100],alVal[16];
   struct sockaddr_in cli_addr;
   int    ilRc;
   long   ilSockfd;
   /*short  ilMsglen; delete by wh*/
   TYPE_SWITCH ilMsglen; /*add by wh*/
   long	  ilLen;
   struct linger ilLinger;
   char   alCliaddr[21];                 /* 客户主机地址 */


   pthread_t my_pthread_id;

   my_pthread_id = pthread_self();
   pthread_detach(pthread_self());

   for( ; ; ) {
     memset((char *)&cli_addr, 0x00, sizeof(struct sockaddr_in));
    
     ilAddrlen = sizeof(struct sockaddr_in);
     ilSockfd  = accept(igSockfd,(struct sockaddr*)&cli_addr, &ilAddrlen);
     if ( ilSockfd == -1)
     {
       _bmqDebug("S0090 %s:准备连接出错,errno:%d,%s",__FILE__,errno,strerror(errno));
       
       /*** zcb add 2005-08-08 ***/
       close(ilSockfd);
       continue;
       
       /*** zcb delete 2005-08-08      
       return (-1010);
       ***/
     }	
     
     /*获得对方主机名*/
     memset(alCliaddr, 0x00, sizeof(alCliaddr));
     sprintf(alCliaddr, "%s", inet_ntoa(cli_addr.sin_addr));
     
     _bmqDebug("S0100 收到[%s]的信息",alCliaddr);
     /*
     hp = gethostbyaddr( (char *) &sgCli_addr.sin_addr, sizeof(struct in_addr),
        sgCli_addr.sin_family);
     if( hp == NULL)
       strcpy( alCliname, inet_ntoa( sgCli_addr.sin_addr ) );
     else
       strcpy( alCliname, hp->h_name );
       */
     
     ilLinger.l_onoff  =1;
     ilLinger.l_linger =0;
     ilRc=setsockopt(ilSockfd,SOL_SOCKET,SO_LINGER,&ilLinger,sizeof(struct linger));
     if ( ilRc == -1 )
     {
       _bmqDebug("S0110 {%s}Unable to set socket option errno:%d,%s",alCliaddr,errno,strerror(errno));
       close ( ilSockfd );
       exit(-1);
     }
     
     /*从客户端接收报文*/
     memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
     ilMsglen = 8;
     
     /*先收8字节报文长度*/
     ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf, &ilMsglen) ; 
     if ( ilRc != 0 || ilMsglen <= 0)
     {
       close ( ilSockfd );     	
       _bmqDebug("S0120 从客户端{%s}收报文出错!-Retcode=%d Msglen=[%d]", alCliaddr, ilRc,ilMsglen);
       continue;
     }

     memset(alVal,0x00,sizeof(alVal));
     memcpy(alVal,alMsgbuf,8);
     /************* delete by xuchengyong 2002/10/18   *********/
     ilLen = atoi(alVal);
     /************* delete by xuchengyong 2002/10/18   *********/
     /************* add by xuchengyong 2002/10/18   *********
     ilLen = ntohs(atoi(alVal));
     ************* add by xuchengyong 2002/10/18   *********/
		if( igDebug >= 2) _bmqDebug("S0120 从客户端收报文!len=[%d]",ilLen);
     
     /*接收指定长度的报文*/
     ilMsglen = ilLen - 8;
     
     ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf+8, &ilMsglen);

     if ( ilRc != 0 || ilMsglen <= 0)
     {
       close ( ilSockfd );
       _bmqDebug("S0130 从客户端{%s}收报文出错!-Retcode=%d", alCliaddr, ilRc);
       continue;
     }
     
      _bmqDebug("S02 ztq从客户端收报文[%s]",alMsgbuf+8);
      _bmqDebughex(alMsgbuf+8,ilRc);
     
     close( ilSockfd );
 
    
	 pthread_mutex_lock(&connsock_mutex); 
	   	     
     memset(&sMsgbuf,0x00,sizeof(sMsgbuf));
     sMsgbuf.msglen=ilLen;
     memcpy(sMsgbuf.data,alMsgbuf,ilLen);
     sMsgbuf.mtype=10;
     
	 if( igDebug >= 2) _bmqDebug("S0120 从客户端收报文!sMsgbuf.msglen=[%d]",sMsgbuf.msglen);
	 if( igDebug >= 2) _bmqDebug("S0120 从客户端收报文!alMsgbuf=[%s]",alMsgbuf);
	 if( igDebug >= 2) _bmqDebug("S0120 发送方式!alMsgbuf=[%s]",alMsgbuf+sMsgbuf.msglen-2);
	 if( igDebug >= 2) _bmqDebug("S0120 从客户端收报文!ilMsglen=[%d]",ilMsglen);     
    
     ilRc=sizeof(struct Msgbuf);
     _bmqDebug("S0 ztq msgsnd len[%d]",ilRc);        
     _bmqDebughex(&sMsgbuf,ilRc);
     /*发送报文到前置消息队列*/
     ilRc = msgsnd(q_reqid,&sMsgbuf,sizeof(struct Msgbuf),0);
     if (ilRc < 0) {
        pthread_mutex_unlock(&connsock_mutex);
        _bmqDebug("写报文到前置Q[%d]出错[%d]ilRc=[%d] aReqid=[%s]", q_reqid,errno,ilRc,aReqid);
        continue;
     }
     pthread_mutex_unlock(&connsock_mutex);
     
	if( igDebug >= 2) _bmqDebughex(sMsgbuf.data,sMsgbuf.msglen);
	
     if( igDebug >= 2) _bmqDebug("发送报文到前置Q[%d]成功", q_reqid);
   }
}


/**************************************************************
 ** 函数名: _bmqGrpRecv
 ** 功能:   接收数据
 ** 作者:
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:sockfd--套接字描述符 buffer--存储信息的地址
             length--缓存区的最大尺寸
 ** 返回值: 0--成功，-1041--失败
***************************************************************/
/*static int _bmqGrpRecv( sockfd, buffer, length)
  int     sockfd;
  char *buffer;
  short   *length;
  delete by wh */
static int _bmqGrpRecv( sockfd, buffer, length)
  int     sockfd;
  char *buffer;
  TYPE_SWITCH *length; /*add by wh*/
{
  /*short len=0,ilRcvlen=0; delete by wh*/
  TYPE_SWITCH len=0,ilRcvlen=0; /*add by wh*/

  memset (buffer, 0x00, *length);
  while(ilRcvlen != *length)
  {
    if ( ( len=recv(sockfd,buffer+ilRcvlen,*length-ilRcvlen,0)) > 0 )
    {
      ilRcvlen += len;
    }
    else
    {
      _bmqDebug("S0111: _bmqGrpRecv error! errno:%d,%s",errno,strerror(errno));
      return(-1040);
    }
    _bmqDebug("S03 ztq: _bmqGrpRecv len:%d",len);
    len=0;
  }
  return ( SUCCESS );
}

static void _bmqQuit()
{
  int i;

  for(i=0;i<1024;i++)
  {
    if( lgSndpid[i] )
    {
      kill(lgSndpid[i],9);
    }
  }
  close(igSockfd);
  exit(0);
}

/****************************************************************/
/* 模块编号    ：GetProfileString                               */
/* 模块名称    ：数据提取                                       */
/* 版 本 号    ：                                               */
/* 作    者    ：Li Yongguang                                   */
/* 建立日期    ：2006-02-07                                     */
/* 最后修改日期：2006-02-09	                                */
/* 模块用途    ：从文件读数据                                   */
/****************************************************************/

int GetProfileString(FileName,Section,Index,GetValue)
char *FileName,*Section,*Index,*GetValue;
{

	FILE *fp;
	int iSectionLen,iIndexLen,iFind;
	char buff[512];
	int i;

	iSectionLen=strlen(Section);
	iIndexLen=strlen(Index);

	/* 打开配置文件 */

	if ( ( fp = fopen (FileName,"r") ) == NULL )
		return (-1);

	/* 寻找匹配条件 */

	iFind =-2;
	while( fgets(buff,sizeof(buff) - 1,fp)) {
		if ( *buff == '#' || *buff == ';' )
			continue;

		if ( (*buff=='[') && (*(buff+iSectionLen+1)==']') && \
		    (!memcmp(buff+1,Section,iSectionLen)) ) {
			iFind =0;
			break;
		}
	}
	if (iFind) {
		fclose(fp);
		return ( -2 );
	}

	/* 寻找匹配数据 */

	iFind =-3;

	while( fgets(buff,sizeof(buff),fp)) {

		if (*buff=='[' )
			break;

		if (*buff=='#'||*buff==';')
			continue;

		if ( (*buff==*Index) && ( *(buff+iIndexLen)=='=') && \
			(!memcmp(buff,Index,iIndexLen)) ) {

			for (i = iIndexLen;i<= strlen(buff);i++){
				/*
				if (buff[i] == ';'|| buff[i] == '#'|| buff[i]=='\n'){
				*/
				if (buff[i] == ' ' || buff[i] == '\n'){
					buff[i] = 0x00;
					break;
				}
			}

			strcpy( GetValue,buff+iIndexLen+1);
			iFind = strlen(GetValue) ;
			GetValue[iFind]=0;
			break;
		}
	}
	fclose(fp);
	return (iFind);
}
