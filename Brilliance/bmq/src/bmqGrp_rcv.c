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
/*static int _bmqGrpRecv( int sockfd,char *buffer, short *length);delete by wh*/
static int _bmqGrpRecv( int sockfd,char *buffer, TYPE_SWITCH *length);/*add by wh*/
extern int _comTcpSend();

int main(int argc,char **argv)
{
  int     i,ilRc,ilLen,ilSockfd;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
  int    ilOpt;

  /*显示新晨Q版本信息*/
  _bmqShowversion(argc,argv);

  /*设置信号*/
  _bmqSignalinit();
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

  if(agCommode[0] == 'S')
  {
    _bmqDebug("S0042 本组设置的连接方式是短连接!");
    exit(FAIL);
  }

  memset(&sgSvr_addr,0x00,sizeof(struct sockaddr_in));
  memset(&sgCli_addr,0x00,sizeof(struct sockaddr_in));
  
  sgSvr_addr.sin_family = AF_INET;
  sgSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sgSvr_addr.sin_port = htons(igBmqgrpport);

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
    if(i == 1023) i = 0;	

    memset((char *)&sgCli_addr, 0x00, sizeof(struct sockaddr_in));
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(ilSockfd,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      _bmqDebug("S0090 %s:准备连接出错,errno:%d,%s",__FILE__,errno,strerror(errno)); 
      return (-1010); 
    }
     
    switch ( (lgSndpid[i] = fork()) )
    {
      case -1:
        close( igSockfd );  
        exit(FAIL);
      case 0:	
        close( ilSockfd );
        if ( (_bmqGrpprocess()) != 0 )
          _bmqDebug("S0100 子进程处理出错!!\n");  
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
static int _bmqGrpprocess(  )
{
  struct hostent *hp;
  struct linger ilLinger;

  char   alCliname[80];
  char   alMacbuf[MAC_LEN];
  char   alMsgbuf_rcv[iMBMAXPACKSIZE];
  char   alMsgbuf[iMBMAXPACKSIZE+100],alVal[16];
  int    ilRc,ilLen,ilLen1;
  short  ilPrior,ilMbid;
  
  char   alFilter[iMASKLEN];
  char   alHostName[100];
  
  short  ilGrpid;//ilMsglen;
  TYPE_SWITCH ilMsglen;/*add by wh*/
  struct grprecvmsg  slMsgrcv_grp;

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
  
  /* 连接身份认证 */
  memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
  ilMsglen = 8;
  ilRc = _bmqGrpRecv(igSockfd, alMsgbuf, &ilMsglen) ;
  if ( ilRc != 0 || ilMsglen <= 0)
  {
    _bmqDebug("S0120 _bmqGrpRecv error! Retcode=%d Msglen=[%d]",ilRc,ilMsglen);
    return(FAIL);
  }
  memcpy(alVal,alMsgbuf,8);
  ilLen = ntohs(atoi(alVal));

  ilMsglen = ilLen - 8;
  ilRc = _bmqGrpRecv(igSockfd, alMsgbuf+8, &ilMsglen) ;
  if ( ilRc != 0 || ilMsglen <= 0)
  {
    _bmqDebug("S0130 _bmqGrpRecv error! Retcode=%d",ilRc);
    return(FAIL);
  }
  
  ilLen1 = 5;
  sprintf(alHostName,"%s",inet_ntoa( sgCli_addr.sin_addr ));
  _bmqLokiEnc(alHostName);
  if(memcmp(alHostName,alMsgbuf+13,ilMsglen-5))
  {
    _bmqDebug("S0140 IP地址串不匹配!");
    _bmqDebughex(alHostName,strlen(alHostName));
    _bmqDebughex(alMsgbuf+8,ilMsglen);
    _comTcpSend(igSockfd,"UNMAC",&ilLen1);
    return(FAIL);
  }

  memset(alVal,0x00,sizeof(alVal));
  memcpy(alVal,alMsgbuf+8,5);
  ilRc = _bmqConfig_load("GROUP",atoi(alVal));
  if (ilRc < 0)
  {
    _bmqDebug("S0150 组[%d]在本服务器没有定义!",atoi(alVal));
    _comTcpSend(igSockfd,"UNDEF",&ilLen1);
    return(FAIL);
  }
  _bmqVdebug(2,"S0160 Grpid=[%d],IP=[%s],Port=[%d]", igGroupid,agBmqhostip, igBmqgrpport);
  if(!memcmp(agBmqhostip,inet_ntoa(sgCli_addr.sin_addr),strlen(agBmqhostip)))
  {
    _comTcpSend(igSockfd,"AAAAA",&ilLen1);
  }
  else
  {
    _comTcpSend(igSockfd,"IPERR",&ilLen1);
    _bmqDebug("S0162 IP地址不正确,rcv=[%s],def=[%s]",inet_ntoa(sgCli_addr.sin_addr),agBmqhostip);
    return(FAIL);
  } 
  
  if(igDebug >= 1)
    _bmqDebug("S0170 组[%d-%s]的身份认证得到确认!",igGroupid,agBmqhostip);

  for(;;)
  {
    /*从客户端接收报文*/
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
    ilLen = ntohs(atoi(alVal));

    /*接收指定长度的报文*/
    ilMsglen = ilLen - 8;
    ilRc = _bmqGrpRecv(igSockfd, alMsgbuf+8, &ilMsglen) ;
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0130 从客户端{%s}收报文出错!-Retcode=%d", alCliname, ilRc);
      return(FAIL);
    }

    if(igDebug >= 1)
    {
      _bmqDebug("S0140 从客户端{%s}接收报文完毕!!--ilMsglen=%d order=%d",
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
    slMsgrcv_grp.iDes_group = ntohs(slMsgrcv_grp.iDes_group);
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
    memset(alMsgbuf_rcv,0x00,sizeof(alMsgbuf_rcv));
    ilMsglen = ilLen-sizeof(struct grprecvmsg)-8-MAC_LEN;
    memcpy(alMsgbuf_rcv,alMsgbuf+8+sizeof(struct grprecvmsg),ilMsglen);
    _bmqMac(alMsgbuf_rcv,ilMsglen,alMacbuf);
    if(memcmp(alMacbuf,alMsgbuf+ilLen-MAC_LEN,MAC_LEN)) { /*MAC ERROR!*/
      _bmqDebug("S0091 Recv Msg MAC ERROR!");
      ilLen1 = 5;
      _comTcpSend(igSockfd,"MACER",&ilLen1);  
      continue;
    }
    else { /* receive success */
      ilLen1 = 5;
      _comTcpSend(igSockfd,"AAAAA",&ilLen1);  
    }
    ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter,alMsgbuf_rcv,ilMsglen);
    if(ilRc) 
    {
      _bmqDebug("S0101 报文[%d]存入[%d][%d]错误!",(ilLen-sizeof(struct grprecvmsg)-8),ilGrpid,ilMbid);
      continue;
    }
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
  short   *length;delete by wh*/
static int _bmqGrpRecv( sockfd, buffer, length)
  int     sockfd;
  char *buffer;
  TYPE_SWITCH *length;
{
  /*short len=0,ilRcvlen=0; delete by wh*/
  TYPE_SWITCH len=0,ilRcvlen=0;/*add by wh*/

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

