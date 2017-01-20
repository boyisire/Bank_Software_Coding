/****************************************************************/
/* 模块编号    ：BMQGRP_SND                                     */
/* 模块名称    ：跨组级联发送模块                               */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2001/08/08                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：新晨Q 跨组级联发送守护进程                     */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共325行                              */
/****************************************************************/
#include "bmqtype.h"    /*add by wh*/

/*函数定义*/
int _bmqGrpsndproc(short iGrpid,char *aHostip,short iPort,long lTrytime);
static int _bmqGrpcontser(char *alIp,short ilPort); 
/*static int _bmqGrpSend(int sockfd, char *buffer, short *length);delete by wh*/
static int _bmqGrpSend(int sockfd, char *buffer, TYPE_SWITCH *length); /*add by wh*/
extern int _comTcpSend();
extern int _comTcpReceive();

/*库函数定义*/
#include "bmq.h"

static  long	lgSndpid[1024];
static  void  _bmqQuit();

int main(int argc,char **argv)
{
  int    ilRc,i;
  int    ilStatus;

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

  /*保存进程pid*/
  psgMbshm->lBmqgrpsnd_pid = getpid();
 
  _bmqDebug("S0040 ***跨组级联 发送守护进程 msgrcv***");
  
  for(i=0;i<1024;i++) lgSndpid[i] = 0;
  i = 0;
  while(1)
  {
    /*取出目标组的IP地址和端口号*/ 
    ilRc = _bmqConfig_load("GROUP",-1);
    if (ilRc < 0)
    {
      _bmqDebug("S0100 -1182:取出目标组的IP地址和端口号错误!");
      continue;
    }
    /*目标组使用短连接 */
    if( (agCommode[0] == 'S') && ( ilRc != FINISH) ) continue;
    if (ilRc == FINISH) break;
    if( igDebug >= 2 )
    {
       _bmqDebug("S0110 取出目标组[%d]的IP地址和端口号:[%s][%d]",
         igGroupid,agBmqhostip, igBmqgrpport);
    }
    if ( (lgSndpid[i]=fork()) == 0)
    {
      _bmqGrpsndproc(igGroupid,agBmqhostip, igBmqgrpport,lgGroup_try_time);
      exit(0);
    }
    i ++;
  }
  while( wait(&ilStatus) != -1)
  {
    _bmqDebug("S0111 跨组发送进程结束!!");
    exit(0);
  }
  exit(0);
}

/**************************************************************
 ** 函数名: _bmqGrpsndproc
 ** 功能:   发送处理进程
 ** 作者:
 ** 建立日期: 2001/09/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0--成功，-1031--失败
***************************************************************/
int _bmqGrpsndproc(short iGrpid,char *aHostip,short iPort,long lTrytime)
{
  int	 ilRc,ilLentmp;
  int    ilSockfd;                 /*套接字*/
  /*short  ilMsglen,ilMsglen_grp;    delete by wh 报文长度*/
  TYPE_SWITCH ilMsglen,ilMsglen_grp;    /*报文长度 add by wh*/
  short  ilGrpid,ilMbid,ilPrior;
  char   alFilter[iMASKLEN];
  char   alVal[11],alTmp[10];
  char   alMacbuf[MAC_LEN];
  char   alMsgbuf[iMBMAXPACKSIZE+100];       /*报文存储数组*/
  char   alMsgbuf_grp[iMBMAXPACKSIZE+100];   /*报文发送数组*/
  struct grprecvmsg slRcvmsg_grp;  /*触发消息结构*/
  struct linger ilLinger;
  struct sockaddr_in slCli_addr;
  fd_set	infds,outfds;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif

  /*连接到服务器*/
GRPSND:
  while(1)
  {
    ilSockfd = _bmqGrpcontser(aHostip, iPort);
    if ( ilSockfd < 0 )
    {
      if( igDebug >= 1 )
        _bmqDebug("S0120 连接到服务器出错!! IP=[%s] Port=[%d],[%ld]秒后重试",aHostip, iPort,lTrytime);
      sleep(lTrytime);
      continue;
    }
    else
    {
      _bmqDebug("S0121 连接到服务器IP=[%s] Port=[%d]成功",aHostip, iPort);
      break;
    }
  }

  /*取得SOCKET信息*/
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( ilSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0130 unable to read socket address");
    close(ilSockfd);
    return(FAIL);
  }

  /*设置SOCKET选项*/
  ilLinger.l_onoff = 1 ;
  ilLinger.l_linger = 1 ;
  ilRc = setsockopt( ilSockfd, SOL_SOCKET, SO_LINGER, &ilLinger,
    sizeof(struct linger));
  if ( ilRc < 0 )
  {
    _bmqDebug("S0140 Unable to set socket option errno: %d,%s",errno,strerror(errno));
    close(ilSockfd);
    return(FAIL);
  }
  
  /* 发送身份确认信息,以组号和IP地址(密码串)组成源串 */
  memset(alMsgbuf,0x00,sizeof(alMsgbuf));
  sprintf(alMsgbuf,"%s",inet_ntoa(slCli_addr.sin_addr));
  _bmqLokiEnc(alMsgbuf);
  memset(alMsgbuf_grp,0x00,sizeof(alMsgbuf_grp));
  ilMsglen_grp = 13 + strlen(alMsgbuf);
  sprintf(alVal,"%08d",htons(ilMsglen_grp));
  memcpy(alMsgbuf_grp,alVal,8);
  sprintf(alVal,"%05d",psgMbshm->iMbgrpid);
  memcpy(alMsgbuf_grp+8,alVal,5);
  memcpy(alMsgbuf_grp+13,alMsgbuf,strlen(alMsgbuf));
  ilRc = _comTcpSend(ilSockfd,alMsgbuf_grp,&ilMsglen_grp);
  if( ilRc )
  {
    _bmqDebug("S0141 发送身份确认信息失败!");
    close(ilSockfd);
    return(FAIL);
  }
  memset(alMsgbuf,0x00,sizeof(alMsgbuf));
  ilMsglen_grp = 5;
  ilRc = _comTcpReceive(ilSockfd,alMsgbuf,&ilMsglen_grp);
  if( ilRc )
  {
    _bmqDebug("S0142 接收身份确认应答信息失败!");
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"UNDEF",5))
  {
    _bmqDebug("S0143 服务器IP=[%s]端未定义本组信息!!!",aHostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"UNMAC",5))
  {
    _bmqDebug("S0144 服务器IP=[%s]未通过身份确认信息!!!",aHostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"IPERR",5))
  {
    _bmqDebug("S0145 服务器IP=[%s]未通过身份确认信息!!!",aHostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(memcmp(alMsgbuf,"AAAAA",5))
  {
    _bmqDebug("S0146 其他错误");
    close(ilSockfd);
    return(FAIL);
  }
 
  while(1)
  {
    /*阻塞读取跨组传送触发消息*/
    ilRc = msgrcv(psgMbshm->lMsgid_grp,&slRcvmsg_grp,
      sizeof(struct grprecvmsg),iGrpid,0);
    if (ilRc < 0)
    {
      if (errno == EINTR)
      {
        _bmqDebug("S0050 msgrcv error! errno: %d,%s! loop",errno,strerror(errno));
        continue;
      }
      _bmqDebug("S0060 msgrcv error! errno: %d,%s! exit",errno,strerror(errno));
      close(ilSockfd);
      exit(FAIL);
    }
    

    if(igDebug >= 0)
    {
      _bmqDebug("S0070 从组间消息队列(收)中读出消息:Pack_head=[%ld],Des_grp=[%d] Des_mailbox=[%d]",
       slRcvmsg_grp.lPack_head,
       slRcvmsg_grp.iDes_group,
       slRcvmsg_grp.iDes_mailbox);
    }

    /* 判断Server端是否已经close */
    FD_ZERO(&infds);
    FD_ZERO(&outfds);
    FD_SET(ilSockfd,&infds);
    FD_SET(ilSockfd,&outfds);
    if(select(ilSockfd+1,&infds,&outfds,NULL,0)==-1)
    {
      _bmqDebug("S0071 select error!");
      ilRc = msgsnd(psgMbshm->lMsgid_grp,&slRcvmsg_grp,
                      sizeof(struct grprecvmsg),0);
      if(ilRc < 0)
      {
        _bmqDebug("S0071 跨组发送消息出错 msgsnd errno:%d:%s",
                    errno,strerror(errno));
      }
      goto GRPSND;
    }
    if(FD_ISSET(ilSockfd,&infds))
    {
      ilRc = recv(ilSockfd,alTmp,10,0);
      if(ilRc <= 0)
      {
        _bmqDebug("S0072 从SOCKET接收信号错误!");
        ilRc = msgsnd(psgMbshm->lMsgid_grp,&slRcvmsg_grp, 
                        sizeof(struct grprecvmsg),0);
        if(ilRc < 0)
        {
          _bmqDebug("S0074 跨组发送消息出错 msgsnd errno:%d:%s", 
                       errno,strerror(errno));
        }
        goto GRPSND;
      }
    }
    if(FD_ISSET(ilSockfd,&outfds))
    {
    /*提取报文修改邮箱信息*/
    ilRc = _bmqLock(LK_ALL);
    if( ilRc ) 
    {
      _bmqDebug("S0075 申请信号灯出错!");
      close(ilSockfd);
      return(ilRc);
    }
    ilMsglen = sizeof(alMsgbuf);
    ilRc = _bmqPack_get(alMsgbuf,&ilMsglen,slRcvmsg_grp.lPack_head);
    if(ilRc ==FAIL)
    {
      _bmqDebug("S0080 提取报文失败!");
      _bmqUnlock(LK_ALL);
      continue;
    }
    if( igDebug >= 0 )
    {
      _bmqDebug("S0090 提取报文[%d]发往[%d]组[%d]邮箱",
        slRcvmsg_grp.lPack_head,
        slRcvmsg_grp.iDes_group,
        slRcvmsg_grp.iDes_mailbox);
    }
    _bmqUnlock(LK_ALL); 

    /************* add by xuchengyong 2002/10/18   *********/
    slRcvmsg_grp.mtype = htonl(slRcvmsg_grp.mtype);
    slRcvmsg_grp.lPack_head = htonl(slRcvmsg_grp.lPack_head);
    slRcvmsg_grp.iOrg_group = htons(slRcvmsg_grp.iOrg_group);
    slRcvmsg_grp.iOrg_mailbox = htons(slRcvmsg_grp.iOrg_mailbox);
    slRcvmsg_grp.iPrior = htons(slRcvmsg_grp.iPrior);
    slRcvmsg_grp.iDes_group = htons(slRcvmsg_grp.iDes_group);
    slRcvmsg_grp.iDes_mailbox = htons(slRcvmsg_grp.iDes_mailbox);
    /************* add by xuchengyong 2002/10/18   *********/

    /*组织报文*/
    memset(alMsgbuf_grp,0x00,sizeof(alMsgbuf_grp));
    ilMsglen_grp = ilMsglen + sizeof(struct grprecvmsg) + 8 + MAC_LEN;
    /************* add by xuchengyong 2002/10/18   *********/
    sprintf(alVal,"%08d",htons(ilMsglen_grp));
    /************* add by xuchengyong 2002/10/18   *********/
    memcpy(alMsgbuf_grp,alVal,8);
    memcpy(alMsgbuf_grp+8, (char *)&slRcvmsg_grp, sizeof(struct grprecvmsg));
    memcpy(alMsgbuf_grp+8+sizeof(struct grprecvmsg), alMsgbuf, ilMsglen);
    _bmqMac(alMsgbuf,ilMsglen,alMacbuf);
    memcpy(alMsgbuf_grp+ilMsglen_grp-MAC_LEN,alMacbuf,MAC_LEN);

    ilLentmp = ilMsglen_grp;
  
    /*发送报文*/
    ilRc = _bmqGrpSend(ilSockfd,alMsgbuf_grp,&ilMsglen_grp);
    if ( ilRc != 0 )
    {
      _bmqDebug("S0150 %s: Send to server ERROR!!",__FILE__);
      /* add by xujun 2001.11.29 start */
      /*报文存入*/

      /************* add by xuchengyong 2002/10/18   *********/
      slRcvmsg_grp.mtype = ntohl(slRcvmsg_grp.mtype);
      slRcvmsg_grp.lPack_head = ntohl(slRcvmsg_grp.lPack_head);
      slRcvmsg_grp.iOrg_group = ntohs(slRcvmsg_grp.iOrg_group);
      slRcvmsg_grp.iOrg_mailbox = ntohs(slRcvmsg_grp.iOrg_mailbox);
      slRcvmsg_grp.iPrior = ntohs(slRcvmsg_grp.iPrior);
      slRcvmsg_grp.iDes_group = ntohs(slRcvmsg_grp.iDes_group);
      slRcvmsg_grp.iDes_mailbox = ntohs(slRcvmsg_grp.iDes_mailbox);
      /************* add by xuchengyong 2002/10/18   *********/

      ilGrpid = slRcvmsg_grp.iDes_group;
      ilMbid  = slRcvmsg_grp.iDes_mailbox;
      ilPrior = slRcvmsg_grp.iPrior;
      memcpy(alFilter,slRcvmsg_grp.aFilter,iMASKLEN);
      igMb_sys_current_group   = slRcvmsg_grp.iOrg_group;
      igMb_sys_current_mailbox = slRcvmsg_grp.iOrg_mailbox;
      igMbopenflag = 1;
      lgCurrent_pid = getpid();
      ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter, alMsgbuf,ilMsglen );
      if(ilRc)
      {
        _bmqDebug("S0151 报文[%d]存入[%d][%d]错误!",ilMsglen,ilGrpid,ilMbid);
      }
      close(ilSockfd);
      /* add by xujun 2001.11.29 end */
      goto GRPSND;
    }

    if( igDebug >= 2 )
    {
      _bmqDebug("S0160 Send to server completed! Msglen=[%d] Sendlen=[%d]",
        ilLentmp,ilMsglen_grp);
    }
    }
  }
}

/**************************************************************
 ** 函数名: _bmqGrpcontser
 ** 功能:   连接服务器
 ** 作者:
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:alIp--服务器IP地址  ilPort--通讯端口
 ** 返回值: 0--成功
***************************************************************/
static int _bmqGrpcontser(char *alIp,short ilPort)
{
  int    sockfd;
  struct sockaddr_in slServ_addr;

  /* 创建套接字 */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    _bmqDebug("S0170 %s : Creat Socket Error! :errno: %d,%s",__FILE__,errno,strerror(errno));
    return (-1050);
  }

  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( alIp );
  slServ_addr.sin_port = htons( ilPort );

  /*连接通讯端口*/
  if(connect(sockfd,(struct sockaddr *)&slServ_addr,sizeof(slServ_addr)) < 0)
  {
    _bmqDebug("S0180 %s: Connect Server[%s][%d] Error! errno: %d,%s",__FILE__,alIp,ilPort,errno,strerror(errno));
    close(sockfd);
    return (-1060);
  }

  return sockfd;
}

/**************************************************************
 ** 函数名: _comGrpSend
 ** 功能:   发送数据
 ** 作者:
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:sockfd--套接字描述符 buffer--发送信息的地址
             length--发送信息的长度
 ** 返回值: 0--成功，-1031--失败
***************************************************************/
/*static int _bmqGrpSend(sockfd, buffer, length)
 int             sockfd;
 char   *buffer;
 short           *length; delete by wh*/
static int _bmqGrpSend(sockfd, buffer, length)
 int             sockfd;
 char   *buffer;
 TYPE_SWITCH           *length;
{
  int     ret;
  char    buf[6];
  /*short   len=-1;
  short   ilSendlen=0; delete by wh*/
  TYPE_SWITCH len=-1;   /*add by wh*/
  TYPE_SWITCH ilSendlen=0; /*add by wh*/

  while( ilSendlen != *length )
  {
    if ( ( len = send(sockfd,buffer+ilSendlen,*length-ilSendlen,0)) > 0 )
    {
      ilSendlen += len;
    }
    else
    {
      _bmqDebug("S0111: _bmqGrpSend error! errno:%d,%s",errno,strerror(errno));       return(-1031);
    }
  }
  *length = ilSendlen;

  /* receive ack */
  len = 5;
  ret = _comTcpReceive(sockfd,buf,&len);
  if(ret) return(FAIL);
  if(memcmp(buf,"AAAAA",5))
  {
    _bmqDebug("S0112 Grp_rcv MAC ERROR!");
    return(FAIL);
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
  exit(0);
}
