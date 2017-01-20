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

static struct sockaddr_in sgSvr_addr;
static struct sockaddr_in sgCli_addr;
static int igSockfd;
static  long    lgSndpid[1024];

static  void  _bmqQuit();
static int _bmqGrpprocess(  );
/*static int _bmqGrpRecv( int sockfd,char *buffer, short *length); delete by wh*/
static int _bmqGrpRecv( int sockfd,char *buffer, TYPE_SWITCH *length); /*add by wh*/

int main(int argc,char **argv)
{
  int     i,ilRc,ilLen,ilSockfd;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
  int    ilOpt;
  char aIp[32];
  int iPort;
  
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
  signal(SIGTERM,_bmqQuit);
  
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
  ilSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (ilSockfd == -1)
  {
    _bmqDebug("S0050 创建SOCKET错误!errno: %d,%s",errno,strerror(errno));
    exit(FAIL);
  }

  ilOpt = 1;
  ilLen = sizeof(ilOpt);
  ilRc = setsockopt(ilSockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&ilOpt,ilLen);
  if(ilRc < 0)
  {
    _bmqDebug("S0055 Set Socketopt Error! errno:%d,%s",errno,strerror(errno));
    close( ilSockfd );
    exit(FAIL);
  }

  /*绑定SOCKET*/
  if (bind(ilSockfd,(struct sockaddr*)&sgSvr_addr,sizeof(struct sockaddr_in))
    == -1)
  {
    _bmqDebug("S0060 绑定SOCKET至端口[%d]错误!! errno: %d,%s",igBmqgrpport,errno,strerror(errno));
    close( ilSockfd );
    exit(FAIL);
  }

  if (listen(ilSockfd, SOMAXCONN) == -1)
  {
    _bmqDebug("S0070 侦听连接错误!!errno: %d,%s",errno,strerror(errno)); 
    close (ilSockfd);
    exit(FAIL);
  }

  /*保存进程pid*/
  psgMbshm->lBmqgrprcv_pid = getpid();

  for(i=0;i<1024;i++) lgSndpid[i] = 0;
  i = 0;
  _bmqDebug("S0080 ***跨组级联 接收守护进程 listen[%d]***",igBmqgrpport);

  for( ; ; )
  {
    /*** zcb add 2005-03-17 ***/
    if(i == 1023) 
    {
      i = 0;	
    }
    
    /*** zcb add 2005-08-08 ***/
    memset((char *)&sgCli_addr, 0x00, sizeof(struct sockaddr_in));
    
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(ilSockfd,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      _bmqDebug("S0090 %s:准备连接出错,errno:%d,%s",__FILE__,errno,strerror(errno));
      
      /*** zcb add 2005-08-08 ***/
      close(igSockfd);
      continue;
      
      /*** zcb delete 2005-08-08      
      return (-1010);
      ***/
    }
 
    switch((lgSndpid[i] = fork()))
    {
      case -1:
        /*** zcb delete 2005-08-08
        close( ilSockfd );
        ***/
        close( igSockfd );  
        exit(FAIL);
      case 0:	
        close( ilSockfd );
        if ( (_bmqGrpprocess()) != 0 )
          _bmqDebug("S0100 子进程处理出错!!\n");  
        /*20060323 add by pc Translate 延迟30秒再关连接*/
        sleep(30);  
        close ( igSockfd );
  	exit(SUCCESS);
      default:
        close( igSockfd );
        break;
    }
    i ++;
  }
}

/**************************************************************
 ** 函数名: _bmqGrpprocess
 ** 功能:   处理客户端请求
 ** 作者:   徐军
 ** 建立日期: 2000/10/26
 ** 最后修改日期:2001/08/08
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqGrpprocess()
{
  struct hostent *hp;
  struct linger ilLinger;

  char   alCliname[80];
  char   alMsgbuf[iMBMAXPACKSIZE+100],alVal[16];
  int    ilRc,ilLen;
  short  ilPrior,ilMbid;
  
  char alFilter[iMASKLEN];
  
  short  ilGrpid;/*ilMsglen;*/
  TYPE_SWITCH ilMsglen;  /*add by wh*/
  struct grprecvmsg  slMsgrcv_grp;
	struct 
	{
		char type[2];          /*数据包的类型，目前不用，扩展	*/
		/*unsigned short msglen;   delete by wh数据包的长度	*/
		TYPE_SWITCH msglen;  /*数据包的长度	*/
		char destaddr[16];      /*目的地的X.25地址	*/
		char souraddr[16];      /*原发地的X.25地址	*/
	}msg_head;
	char aTmp[64];
	int ilLenTmp=0;
  memset(alVal,0x00,sizeof(alVal));
  memset(alFilter,0x00,sizeof(alFilter));
  /*获得对方主机名*/
  hp = gethostbyaddr( (char *) &sgCli_addr.sin_addr, sizeof(struct in_addr),
     sgCli_addr.sin_family);
  if( hp == NULL)
    strcpy( alCliname, inet_ntoa( sgCli_addr.sin_addr ) );
  else
    strcpy( alCliname, hp->h_name );

  ilLinger.l_onoff  =1;
  ilLinger.l_linger =1;
  ilRc=setsockopt(igSockfd,SOL_SOCKET,SO_LINGER,&ilLinger,
    sizeof(struct linger));
  if ( ilRc == -1 )
  {
    _bmqDebug("S0110 {%s}Unable to set socket option errno:%d,%s",alCliname,errno,strerror(errno));
    close ( igSockfd );
    return (-1081);
  }
  /**delete by nh 20050121 for kgxt
  for(;;)
  {
  */
    /*从客户端接收报文*/
    /*先收 msg_head 个字节，假回执时处理后返回*/
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen=sizeof(msg_head);
    ilRc = _bmqGrpRecv(igSockfd, alMsgbuf, &ilMsglen); 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0120 从客户端{%s}收Translate报文头出错!-Retcode=%d Msglen=[%d]", alCliname, ilRc,ilMsglen);
      return(FAIL);
    }
    else {
    	memcpy((char *)&msg_head,alMsgbuf,sizeof(msg_head));
    	/*实现 源和目的 换位置*/
    	memset(aTmp,0x00,sizeof(aTmp));
    	memcpy(aTmp,msg_head.destaddr,sizeof(msg_head.destaddr));
    	memcpy(msg_head.destaddr,msg_head.souraddr,sizeof(msg_head.destaddr));
    	memcpy(msg_head.souraddr,aTmp,sizeof(msg_head.souraddr));
    	/*计算长度*/
    	msg_head.msglen=sizeof(msg_head)+2;	/*后面拼HZ两个字符*/
    	ilLenTmp=msg_head.msglen;
    	msg_head.msglen=htons(msg_head.msglen);
    	memset(aTmp,0x00,sizeof(aTmp));
    	memcpy(aTmp,(char *)&msg_head,sizeof(msg_head));
    	memcpy(aTmp+sizeof(msg_head),"HZ",2);
    }	
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;
	
    /*先收8字节报文长度*/
    ilRc = _bmqGrpRecv(igSockfd, alMsgbuf, &ilMsglen) ; 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0120 从客户端{%s}收报文出错!-Retcode=%d Msglen=[%d]", alCliname, ilRc,ilMsglen);
      return(FAIL);
    }
    memcpy(alVal,alMsgbuf,8);
    /************* delete by xuchengyong 2002/10/18   *********
    ilLen = atoi(alVal);
    ************* delete by xuchengyong 2002/10/18   *********/
    /************* add by xuchengyong 2002/10/18   *********/
    ilLen = ntohs(atoi(alVal));
    /************* add by xuchengyong 2002/10/18   *********/

    /*接收指定长度的报文*/
    ilMsglen = ilLen - 8;
    ilRc = _bmqGrpRecv(igSockfd, alMsgbuf+8, &ilMsglen);
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0130 从客户端{%s}收报文出错!-Retcode=%d", alCliname, ilRc);
      return(FAIL);
    }

    if(igDebug >= 1)
    {
      _bmqDebug("S0140 从客户端{%s}接收指定长度报文完毕!!--ilMsglen=[%d] order=[%d]",
        alCliname, ilMsglen + 8 ,alMsgbuf[8]);
    }


    memset(alVal,0x00,sizeof(alVal));

    /*解包*/
    memcpy(&slMsgrcv_grp, alMsgbuf+8, sizeof(struct grprecvmsg));
    
    /************* add by xuchengyong 2002/10/18   *********/
    slMsgrcv_grp.mtype = ntohl(slMsgrcv_grp.mtype);    
    
    slMsgrcv_grp.lPack_head = ntohl(slMsgrcv_grp.lPack_head);    
    
    slMsgrcv_grp.iOrg_group = ntohs(slMsgrcv_grp.iOrg_group);    
    
    slMsgrcv_grp.iOrg_mailbox = ntohs(slMsgrcv_grp.iOrg_mailbox);
    
    slMsgrcv_grp.iPrior = ntohs(slMsgrcv_grp.iPrior);

/*add by pc 20060221 增加对假回执的处理*/
	/*跨组报文 最后2位为 发送方式代号(1位) 1 地面专线 2卫星Over 3卫星Translate +通讯线路序号(1位)*/
	if (slMsgrcv_grp.iPrior==3) {
		/*如果是使用卫星Translate 接收的报文，给假回执*/

		ilRc = send(igSockfd,aTmp,ilLenTmp,0);
		if (ilRc<=0) {
			_bmqDebug("S0150 假回执应答失败 send ilRc(len)[%d] errno[%d]",ilRc,errno);
		}
		else {
			_bmqDebug("S0000 假回执应答成功");
			if( igDebug >= 2 ) {
			_bmqDebug("发送完整假回执内容开始...ilRc=[%d] errno[%d]",ilRc,errno);
			_bmqDebughex(aTmp,ilLenTmp);
			_bmqDebug("发送完整假回执内容结束...");
			}							
		}

	}
/*add by pc 20060221 增加对假回执的处理*/


    slMsgrcv_grp.iDes_group = ntohs(slMsgrcv_grp.iDes_group);
/*add by pc 20060221 对目标组号进行修改*/
	if (slMsgrcv_grp.iDes_group==9999){
		slMsgrcv_grp.iDes_group=0;
	}    
/*add by pc 20060221 对目标组号进行修改*/	
    slMsgrcv_grp.iDes_mailbox = ntohs(slMsgrcv_grp.iDes_mailbox);
    /************* add by xuchengyong 2002/10/18   *********/
    ilGrpid = slMsgrcv_grp.iDes_group;
    ilMbid  = slMsgrcv_grp.iDes_mailbox;

    ilPrior = slMsgrcv_grp.iPrior;
    memcpy(alFilter,slMsgrcv_grp.aFilter,iMASKLEN);
  
    /*报文存入*/
    igMb_sys_current_group   = slMsgrcv_grp.iOrg_group;
    igMb_sys_current_mailbox = slMsgrcv_grp.iOrg_mailbox;
    igMbopenflag = 1;  
    lgCurrent_pid = getpid();
    ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter,
      alMsgbuf+8+sizeof(struct grprecvmsg),ilLen-sizeof(struct grprecvmsg)-8 );
    if(ilRc) 
    {
      _bmqDebug("S0101 报文[%d]存入[%d][%d]错误!",(ilLen-sizeof(struct grprecvmsg)-8),ilGrpid,ilMbid);
      return(FAIL);;
    }
  /**delete by nh 20050121 for kgxt
  }
  */
  
  return(SUCCESS);
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
  delete by wh*/
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

