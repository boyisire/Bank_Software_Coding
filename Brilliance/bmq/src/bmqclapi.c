/****************************************************************/
/* 模块编号    ：BMQCLAPI                                       */
/* 模块名称    ：客户端API库                                    */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2000/10/26                                     */
/* 最后修改日期：2002/12/20                                     */
/* 模块用途    ：新晨Q客户端API函数                             */
/* 本模块中包含如下函数及功能说明：                             */
/*                        (1)  int _bmqConnectser()             */
/*                        (2)  int _bmqDebug()                  */
/*                        (3)  int  bmqOpen()                   */
/*                        (4)  int  bmqPut()                    */
/*                        (5)  int  bmqGet()                    */
/*                        (6)  int  bmqGetw()                   */
/*                        (7)  int  bmqClearmb()                */
/*                        (8)  int  bmqClose()                  */
/*                        (9)  int  bmqGetmbinfo()              */
/*                        (10) int  bmqSendfile()               */
/*                        (11) int  bmqPutfilter()              */
/*                        (12) int  bmqGetwfilter()             */
/*                        (13) int  bmqGetserialno()            */
/*                        (14) int  bmqRecvfile()               */
/*                        (15) int  bmqPutfile()                */
/*                        (16) int  bmqGetfileX()               */
/*                        (17) int  bmqPutFile()                */
/*                        (18) int  bmqGetFile()                */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共922行                              */
/* 2002.10.19 徐军,将bmqPut,bmqGet,bmqGetw之前做网络字节序转换  */
/*            bmqGet,bmqGetw之后马上做主机字节序转换            */
/* 2002.12.20 徐军,增加文件传输功能API                          */
/****************************************************************/

/*库函数定义*/
#include "bmq.h"

struct	monmsg	sgMonMsg;

/*函数定义*/
extern int _bmqGetfield(char *fname,char *key,int key_n,char *field);
/*extern int _bmqRecvconf(int sockfd,char *buffer,short *length); delete by wh*/
extern int _bmqRecvconf(int sockfd,char *buffer,TYPE_SWITCH *length); /*add by wh */
extern int compress(void *src, unsigned long src_len, void *dst,char *);
extern int decompress(void *src, unsigned long src_len, void *dst,char *);
extern int _bmqcompress(void *src, unsigned src_len, void *dst ,char *press);
extern int _bmqdecompress(void *src, unsigned src_len,	void *dst,char *press);
extern int _bmqFile_comm(char aParm[][iFLDVALUELEN]);
extern int _bmqFile_msg(char aParm[][iFLDVALUELEN]);
extern int _comTcpSend();
extern int _comTcpReceive();
extern int _bmqConnectfileser();
extern int bmqRecvfileX();
extern int bmqRecvfileByName();

static int		igSockfd;
static short	igTimeout;
static short	igProctype;

/**************************************************************
 ** 函数名: _bmqConnectser
 ** 功能:   连接服务器
 ** 作者:
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:alIp--服务器IP地址  ilPort--通讯端口
 ** 返回值: 0--成功
***************************************************************/
static int _bmqConnectser(alIp,ilPort)
char *alIp;
int  ilPort;
{
  int sockfd;
  struct sockaddr_in slServ_addr;

  /* 创建套接字 */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    _bmqDebug("S0010 %s : Creat Socket Error! :%d,%s",__FILE__,errno,strerror(errno));
    return (-1050);
  }

  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( alIp );
  slServ_addr.sin_port = htons( ilPort );

  /*连接服务器通讯端口*/
  if (connect(sockfd, (struct sockaddr *)&slServ_addr, sizeof(slServ_addr)) < 0)
  {
    _bmqDebug("S0020 %s: Connect Server Error! :%d,%s",__FILE__,errno,strerror(errno));
    return (-1060);
  }
  return sockfd;
}

/**************************************************************
 ** 函数名:bmqOpen
 ** 功能: 打开指定邮箱
 ** 作者: szy
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义    :iMbid--邮箱号
 ** 返回值: 成功：SUCCESS 错误：错误码
***************************************************************/
int bmqOpen(short iMbid)
{
  int ilRc;
  int ilMbport;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  struct linger ilLinger;
  struct sockaddr_in slCli_addr;
  char alVal[16],alResult[100],alMbhost[100],alMsgbuf[iMBMAXPACKSIZE+100],alInitfile[100];

  memset(alVal,0x00,sizeof(alVal));

  /*从参数文件中取出服务器 IP地址和通讯端口号*/
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"MBHOST",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0030 从文件%s中取 MBHOST 字段失败!",alInitfile);
    exit(-1);
  }
  strcpy(alMbhost,alResult);

  ilRc = _bmqGetfield(alInitfile,"MBPORT",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0040 从文件%s中取 MBPORT 字段失败!",alInitfile);
    exit(-1);
  }
  ilMbport = atoi(alResult);
  
  /* 从参数文件中取出超时时间 处理方式发送到SERVER */
  ilRc = _bmqGetfield(alInitfile,"MBTIMEOUT",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0042 从文件%s中取 MBTIMEOUT 字段失败!",alInitfile);
    exit(-1);
  }
  igTimeout = atoi(alResult);
  ilRc = _bmqGetfield(alInitfile,"MBPROCTYPE",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0030 从文件%s中取 MBPROCTYPE 字段失败!",alInitfile);
    exit(-1);
  }
  igProctype = atoi(alResult);

  /* 连接到服务器 */
  igSockfd = _bmqConnectser( alMbhost, ilMbport );
  if ( igSockfd < 0 )
  {
    _bmqDebug("S0050 --%d--连接到服务器出错!",ilRc);
    return (igSockfd);
  }

  /* 获取当前SOCKET名 */
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( igSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0060 unable to read socket address errno:%d,%s",errno,strerror(errno));
    return (-1070);
  }

  /* 设置SOCKET选项 */
  ilLinger.l_onoff = 1 ;
  ilLinger.l_linger = 1 ;
  ilRc = setsockopt( igSockfd, SOL_SOCKET, SO_LINGER, &ilLinger,\
                     sizeof(struct linger));
  if ( ilRc < 0 )
  {
    _bmqDebug("S0070 Unable to set socket option errno:%d,%s",errno,strerror(errno));
    return (-1080); 
  }
   
  /* 组织报文 */
  ilMsglen = 24;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 1;
  sprintf(alVal,"%05d",iMbid);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%05d",igTimeout);
  memcpy(alMsgbuf+14,alVal,5);
  sprintf(alVal,"%05d",igProctype);
  memcpy(alMsgbuf+19,alVal,5);

  /* 发送报文到服务器 */ 
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0080 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }
 
  /* 接收报文 */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0090 %s : Receive from server error!!--%d",__FILE__,ilRc);
    return (ilRc);
  }

  memcpy(alVal,alMsgbuf+1,5);
  alVal[5]=0; /* add by nh 200200905 */
  ilRc = atoi(alVal);
  igMb_sys_current_mailbox = iMbid;
  return(ilRc);
}

/**************************************************************
 ** 函数名:bmqPut
 ** 功能:  存放报文
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iGrpid-目标组   iMbid-目标邮箱   iPrior-优先级
             iType-报文Type  iClass-报文Class aMsgbuf-报文Buf
             iMsglen-报文长度
 ** 返回值: SUCCESS
***************************************************************/
/*int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,
  char *aMsgbuf,short iMsglen) delete by wh*/
int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,
  char *aMsgbuf,TYPE_SWITCH iMsglen) /*add by wh*/
{
  char alFilter[iMASKLEN];
  int ilRc;

  lType = htonl(lType);
  lClass = htonl(lClass);

  memset(alFilter,0x00,sizeof(alFilter));
  memcpy(alFilter,(char *)&lType,sizeof(long));
  memcpy(alFilter+sizeof(long),(char *)&lClass,sizeof(long));
  ilRc = bmqPutfilter(iGrpid,iMbid,iPrior,alFilter,aMsgbuf,iMsglen);
  return(ilRc);
}

/**************************************************************
 ** 函数名      :bmqPutfilter
 ** 功能        :  存放报文
 ** 作者        : 史正烨
 ** 建立日期    :2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义:iGrpid-目标组   iMbid-目标邮箱   iPrior-优先级
             aFilter-过滤条件  aMsgbuf-报文Buf  iMsglen-报文长度
 ** 返回值: SUCCESS
***************************************************************/
/*int bmqPutfilter(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen) delete by wh*/
int bmqPutfilter(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen) /*add by wh*/
{
  char  alVal[iMASKLEN + 1];
  char  alMac[MAC_LEN];
  char  alMsgbuf[iMBMAXPACKSIZE+100];
  int   ilRc;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
 
  /*判断报文长度是否超过最大值add by nh 2002625*/
  if(iMsglen > iMBMAXPACKSIZE)
  {
    _bmqDebug("S0750 报文长度[%d]超过最大值－ [%d]",iMsglen,iMBMAXPACKSIZE);
    return(1012);
  }
  
  /*组织报文*/ 
  _bmqMac(aMsgbuf,iMsglen,alMac);
  memset(alVal,0x00,sizeof(alVal));
  /* modify by xujun 20061129 */
  ilMsglen = (24+iMASKLEN) + iMsglen + MAC_LEN;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 5;
  sprintf(alVal,"%05d",iGrpid);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%05d",iMbid);
  memcpy(alMsgbuf+14,alVal,5);
  sprintf(alVal,"%05d",iPrior);
  memcpy(alMsgbuf+19,alVal,5);
  memcpy(alMsgbuf+24,aFilter,iMASKLEN);
  memcpy(alMsgbuf+24+iMASKLEN,aMsgbuf,iMsglen);
  /* add by xujun 20061129 */
  memcpy(alMsgbuf+24+iMASKLEN+iMsglen,alMac,MAC_LEN);
  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0100 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }

  /* 接收报文 */
  ilMsglen = 6;
  ilRc = _bmqRecvconf( igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0110 Receive from server error!!--ERRNO = %d!!",ilRc);
    return (ilRc);
  }

  memcpy(alVal,alMsgbuf+1,5);
  alVal[5]=0; /* add by nh 200200905 */
  ilRc = atoi(alVal);
  return(ilRc);
}

/**************************************************************
 ** 函数名:bmqGet
 ** 功能:  提取报文(非阻塞)
 ** 作者:  史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:piGrpid-源发组   piMbid-源发邮箱    piPrior-优先级
             piType-报文Type  piClass-报文Class  aMsgbuf-报文Buf
             piMsglen-报文长度
 ** 返回值: SUCCESS,错误码
***************************************************************/
/*int bmqGet(short *piGrpid,short *piMbid,short *piPrior,long *plType,
  long *plClass,char *aMsgbuf,short *piMsglen) delete by wh*/
int bmqGet(short *piGrpid,short *piMbid,short *piPrior,long *plType,
  long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen) /*add by wh*/
{
  char alFilter[iMASKLEN];
  char alMask[iMASKLEN];
  int ilRc;
    
  *plType = htonl(*plType);
  *plClass = htonl(*plClass);

  memset(alFilter,0x00,sizeof(alFilter));
  memset(alMask,0x00,sizeof(alMask));
  memcpy(alFilter,(char *)plType,sizeof(long));
  memcpy(alFilter+sizeof(long),(char *)plClass,sizeof(long));
  if ((*plType) != 0) memset(alMask,0xff,sizeof(long));
  if ((*plClass) != 0) memset(alMask+sizeof(long),0xff,sizeof(long));
  ilRc = bmqGetfilter(piGrpid,piMbid,piPrior,alFilter,alMask,aMsgbuf,piMsglen);
  memcpy((char *)plType,alFilter,sizeof(long));
  memcpy((char *)plClass,alFilter+sizeof(long),sizeof(long));
  *plType = ntohl(*plType);
  *plClass = ntohl(*plClass);
  
  return(ilRc);
}

/**************************************************************
 ** 函数名      :bmqGetfilter
 ** 功能        :提取报文(非阻塞)
 ** 作者        :史正烨
 ** 建立日期    :2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :piGrpid-源发组   piMbid-源发邮箱    piPrior-优先级
                 aFilter-过滤条件  aMask-过滤掩码  aMsgbuf-报文Buf
                 piMsglen-报文长度
 ** 返回值: SUCCESS,错误码
***************************************************************/
/*int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,short *piMsglen) delete by wh*/
int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen) /*add by wh*/
{
  char  alVal[iMASKLEN+1];
  char  alMac[MAC_LEN];
  char  alMsgbuf[iMBMAXPACKSIZE+100];
  int   ilRc;
  /*short ilMsglen,ilLen,ilMsglen1,ilLen1; delete by wh*/
  TYPE_SWITCH ilMsglen,ilLen,ilMsglen1,ilLen1; /*add by wh*/
  char  alTmp[5];
  /*short ilTmp; delete by wh 20150107*/
  TYPE_SWITCH ilTmp;
 
  /* 组织报文 */ 
  memset(alVal,0x00,sizeof(alVal));
  ilMsglen = 24+iMASKLEN+iMASKLEN;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 3;
  sprintf(alVal,"%05d",*piGrpid);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%05d",*piMbid);
  memcpy(alMsgbuf+14,alVal,5);
  sprintf(alVal,"%05d",*piPrior);
  memcpy(alMsgbuf+19,alVal,5);
  memcpy(alMsgbuf+24,aFilter,iMASKLEN);
  memcpy(alMsgbuf+24+iMASKLEN,aMask,iMASKLEN);
  
  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0120 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }

  /* Receive from socket */
  memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
  ilMsglen = 8;

  ilRc = _comTcpReceive(igSockfd, alMsgbuf, &ilMsglen) ; 
  if ( ilRc != 0 || ilMsglen <= 0)
  {
    _bmqDebug("S0130 收报文出错!-Retcode=%d",ilRc);
    close( igSockfd );
    return (ilRc);
  }
  memcpy(alVal,alMsgbuf,8);
  ilLen = atoi(alVal);

  ilLen1 = ilMsglen1 =0;
  while( ilMsglen1 < ilLen )
  {
    ilMsglen = ilLen - 8 - ilLen1;
    ilRc = _comTcpReceive(igSockfd, alMsgbuf+8+ilLen1, &ilMsglen) ;
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0140 收报文出错!-Retcode=%d",ilRc);
      close( igSockfd );
      return (ilRc);
    }
    ilLen1 += ilMsglen;
    ilMsglen1 = ilLen1 + 8;
  }
  
   /* 报文解包 */
  memset(alVal,0x00,sizeof(alVal)); 
  memcpy(alVal,alMsgbuf+9,5);
  ilRc = atoi(alVal);
  if (ilRc) return(ilRc);
  memcpy(alVal,alMsgbuf+14,5);
  *piGrpid = atoi(alVal);
  memcpy(alVal,alMsgbuf+19,5);
  *piMbid = atoi(alVal);
  memcpy(alVal,alMsgbuf+24,5);
  *piPrior = atoi(alVal);
  memcpy(aFilter,alMsgbuf+29,iMASKLEN);
  memcpy(aMask,alMsgbuf+29+iMASKLEN,iMASKLEN);
  memcpy(aMsgbuf,alMsgbuf+29+iMASKLEN+iMASKLEN,ilLen-(21+iMASKLEN+iMASKLEN+MAC_LEN));
  *piMsglen = ilLen - (29+iMASKLEN+iMASKLEN+MAC_LEN);
  _bmqMac(aMsgbuf,*piMsglen,alMac);

  if( (memcmp(alMsgbuf+ilLen-MAC_LEN,alMac,MAC_LEN)) &&
      (memcmp(alMsgbuf+ilLen-MAC_LEN,"00000000",MAC_LEN)) )
    memcpy(alTmp,"MACE",4);
  else
    memcpy(alTmp,"AAAA",4);

  /* 发送确认信息 */
  ilMsglen = 4;
  ilRc = _comTcpSend(igSockfd,alTmp, &ilMsglen);
  if( ilRc )
  {
    _bmqDebug("S0170: 发送确认信息出错!");
    return (ilRc);	
  }
  ilTmp = 1;
  ilRc = _bmqRecvconf( igSockfd,alTmp,&ilTmp);
  if ( ilRc != 0 )
  {
    _bmqDebug("S0180: Receive from server error!!--ERRNO = %d!!",ilRc);
    return (ilRc);
  }
  if(alTmp[3] == 'A')
    return(SUCCESS); 
  else
    return(1015);
}

/**************************************************************
 ** 函数名:bmqGetw
 ** 功能:  提取报文(阻塞)
 ** 作者:  史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:piGrpid-源发组   piMbid-源发邮箱   piPrior-优先级
             piType-报文Type  piClass-报文Class aMsgbuf-报文Buf
             piMsglen-报文长度   iTimeout-阻塞读间隔时间
 ** 返回值: SUCCESS,错误码
***************************************************************/
/*int bmqGetw(short *piGrpid,short *piMbid,short *piPrior,long *plType,
  long *plClass,char *aMsgbuf,short *piMsglen,short iTimeout) delete by wh*/
int bmqGetw(short *piGrpid,short *piMbid,short *piPrior,long *plType,
  long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout) /*add by wh*/
{
  char alFilter[iMASKLEN];
  char alMask[iMASKLEN];
  int ilRc;
    
  *plType = htonl(*plType);
  *plClass = htonl(*plClass);

  memset(alFilter,0x00,sizeof(alFilter));
  memset(alMask,0x00,sizeof(alMask));

  memcpy(alFilter,(char *)plType,sizeof(long));  
  memcpy(alFilter+sizeof(long),(char *)plClass,sizeof(long));
  if ((*plType) != 0) memset(alMask,0xff,sizeof(long));
  if ((*plClass) != 0) memset(alMask+sizeof(long),0xff,sizeof(long));

  ilRc = bmqGetwfilter(piGrpid,piMbid,piPrior,alFilter,alMask,aMsgbuf,piMsglen,iTimeout);

  memcpy((char *)plType,alFilter,sizeof(long));
  memcpy((char *)plClass,alFilter+sizeof(long),sizeof(long));
  *plType = ntohl(*plType);
  *plClass = ntohl(*plClass);
  
  return(ilRc);
}

/**************************************************************
 ** 函数名      :bmqGetwfilter
 ** 功能        :提取报文(阻塞)
 ** 作者        :史正烨
 ** 建立日期    :2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :piGrpid-源发组   piMbid-源发邮箱   piPrior-优先级
                 aFilter-过滤条件  aMask-过滤掩码   aMsgbuf-报文Buf
                 piMsglen-报文长度   iTimeout-阻塞读间隔时间
 ** 返回值: SUCCESS,错误码
***************************************************************/
/*int bmqGetwfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,short *piMsglen,short iTimeout) delete by wh*/
int bmqGetwfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout) /*add by wh*/
{
  char  alVal[iMASKLEN+1];
  char  alMac[MAC_LEN];
  char  alMsgbuf[iMBMAXPACKSIZE+100];
  int   ilRc;
  /*short ilMsglen,ilLen,ilMsglen1,ilLen1; delete by wh*/
  TYPE_SWITCH ilMsglen,ilLen,ilMsglen1,ilLen1; /*add by wh*/
  char	alTmp[5];
  /*short ilTmp; delete by wh 20150107*/
  TYPE_SWITCH ilTmp; /*add by wh 20150107*/
  
  /* 组织报文 */ 
  memset(alVal,0x00,sizeof(alVal));
  ilMsglen = 29+iMASKLEN+iMASKLEN;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 4;
  sprintf(alVal,"%05d",*piGrpid);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%05d",*piMbid);
  memcpy(alMsgbuf+14,alVal,5);
  sprintf(alVal,"%05d",*piPrior);
  memcpy(alMsgbuf+19,alVal,5);
  memcpy(alMsgbuf+24,aFilter,iMASKLEN);
  memcpy(alMsgbuf+24+iMASKLEN,aMask,iMASKLEN);
  sprintf(alVal,"%05d",iTimeout);
  memcpy(alMsgbuf+24+iMASKLEN+iMASKLEN,alVal,10);
  
  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0150 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }

  /* Receive from socket */
  memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
  ilMsglen = 8;

  ilRc = _comTcpReceive(igSockfd, alMsgbuf, &ilMsglen) ; 
  if ( ilRc != 0 || ilMsglen <= 0)
  {
    _bmqDebug("S0160 收报文出错!-Retcode=%d",ilRc);
    close( igSockfd );
    return (ilRc);
  }

  memcpy(alVal,alMsgbuf,8);
  alVal[8] = '\0';
  ilLen = atoi(alVal);

  ilLen1 = ilMsglen1 =0;
  while( ilMsglen1 < ilLen )
  {
    ilMsglen = ilLen - 8 - ilLen1;
    ilRc = _comTcpReceive(igSockfd, alMsgbuf+8+ilLen1, &ilMsglen) ;
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0170 收报文出错!-Retcode=%d",ilRc);
      close( igSockfd );
      return (ilRc);
    }
    ilLen1 += ilMsglen;
    ilMsglen1 = ilLen1 + 8;
  }

  /* 报文解包 */
  memset(alVal,0x00,sizeof(alVal));
  memcpy(alVal,alMsgbuf+9,5);
  ilRc = atoi(alVal);
  if (ilRc) return(ilRc);
  memcpy(alVal,alMsgbuf+14,5);
  *piGrpid = atoi(alVal);
  memcpy(alVal,alMsgbuf+19,5);
  *piMbid = atoi(alVal);
  memcpy(alVal,alMsgbuf+24,5);
  *piPrior = atoi(alVal);
  memcpy(aFilter,alMsgbuf+29,iMASKLEN);
  memcpy(aMask,alMsgbuf+29+iMASKLEN,iMASKLEN);
  memcpy(aMsgbuf,alMsgbuf+29+iMASKLEN+iMASKLEN,ilLen-(21+iMASKLEN+iMASKLEN));
  *piMsglen = ilLen - (29+iMASKLEN+iMASKLEN+MAC_LEN);
  _bmqMac(aMsgbuf,*piMsglen,alMac);
  
  if( (memcmp(alMsgbuf+ilLen-MAC_LEN,alMac,MAC_LEN)) &&
      (memcmp(alMsgbuf+ilLen-MAC_LEN,"00000000",MAC_LEN)) )
    memcpy(alTmp,"MACE",4);
  else
    memcpy(alTmp,"AAAA",4);

  /* 发送确认信息 */
  ilMsglen = 4;
  ilRc = _comTcpSend(igSockfd,alTmp, &ilMsglen);
  if( ilRc )
  {
    _bmqDebug("S0220: 发送确认信息出错!");
    return (ilRc);
  }
  ilTmp = 1;
  ilRc = _bmqRecvconf( igSockfd,alTmp,&ilTmp);
  if ( ilRc != 0 )
  {
    _bmqDebug("S0230: Receive from server error!!--ERRNO = %d!!",ilRc);
    return (ilRc);
  }

  if(alTmp[3] == 'A')
    return(SUCCESS);
  else
   return(1015);
}

/**************************************************************
 ** 函数名:bmqClose
 ** 功能: 关闭当前邮箱
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义    :
 ** 返回值: 成功：SUCCESS 错误：FAIL
***************************************************************/
int bmqClose()
{
  char alVal[16];
  char alMsgbuf[iMBMAXPACKSIZE+100];
  int ilRc;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  
  memset(alVal,0x00,sizeof(alVal));

  /*组织报文*/
  ilMsglen = 9;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 2;
 
  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0180 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }

  /* Receive from socket */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0190 Receive from server error!!--ERRNO = %d!!",ilRc);
    return (ilRc);
  }

  memcpy(alVal,alMsgbuf+1,5);
  alVal[5]=0; /* add by nh 200200905 */ 
  ilRc = atoi(alVal);
  
  /* add by nh */
  close(igSockfd);
  return(ilRc); 
}

/**************************************************************
 ** 函数名:  bmqClearmb
 ** 功  能:  清理邮箱
 ** 作  者:  徐军
 ** 建立日期:    2001.08.08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: iMbid--指定清理邮箱号 0表示清理所有邮箱
 ** 返回值:  SUCCESS , FAIL
***************************************************************/
int bmqClearmb(short iMbid)
{
  int ilRc;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];

  memset(alVal,0x00,sizeof(alVal));

  /*组织报文*/
  ilMsglen = 14;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 6;
  sprintf(alVal,"%05d",iMbid);
  memcpy(alMsgbuf+9,alVal,5);

  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0200 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }
  
  /* Receive from socket */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0210 Receive from server error!!--ERRNO = %d!!",ilRc);
    return (ilRc);
  }

  memcpy(alVal,alMsgbuf+1,5);
  alVal[5]=0; /* add by nh 200200905 */
  ilRc = atoi(alVal);
  return(ilRc);
}

/**************************************************************
 ** 函数名:  bmqGetmbinfo
 ** 功  能:  获取邮箱信息
 ** 作  者:  徐军
 ** 建立日期:    2001.08.08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: iMbid--指定清理邮箱号
 ** 返回值:  SUCCESS , FAIL
***************************************************************/
int bmqGetmbinfo(short iMbid,struct mbinfo *psMbinfo)
{
  int ilRc;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];

  memset(alVal,0x00,sizeof(alVal));

  /*组织报文*/
  ilMsglen = 14;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 7;
  sprintf(alVal,"%05d",iMbid);
  memcpy(alMsgbuf+9,alVal,5);

  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0200 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }

  /* Receive from socket */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0210 Receive from server error!!--ERRNO = %d!!",ilRc);
    return (ilRc);
  }

  memcpy(alVal,alMsgbuf+1,5);
  memcpy(psMbinfo,alMsgbuf+6,sizeof(struct mbinfo));
  alVal[5]=0; /* add by nh 200200905 */
  ilRc = atoi(alVal);
  return(ilRc);
}

/**************************************************************
 ** 函数名:  bmqSendfile
 ** 功能:    发送文件到Server
 ** 作者:    徐军 	
 ** 建立日期: 2002/12/20
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0--成功
***************************************************************/
int bmqSendfile(short iGrpid,char *aFilename,struct transparm sTransparm,
                long *lSerialno)
{
  int	ilRc,ilSockfd;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char	alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];
  char	alParm[10][iFLDVALUELEN];
  long  llOffset;
  FILE  *fq;
  char	alFile[100];
  char	alSendFileName[120];
  char  alTmpFileName[100];
  char  alOffset[20];
  char  alResult[10];
  char  alInitfile[101];
  char	alCommand[120];
  char ilCompressFlag[10];
  char   alCryptFlag[10];
    
  memset(alCommand,0x00,sizeof(alCommand));
  _bmqfilechr(aFilename,strlen(aFilename),alTmpFileName,'/');
  sprintf(alCommand,"ln %s %s/saf/%s",aFilename,getenv("BMQ_FILE_DIR"),alTmpFileName);
  system(alCommand);

  /* 连接到服务器,为了防止CLI和SRV之间存在不同用户的读取权限问题,
     CLI发送时,无论是否跨组,都通过FILESRV模块拆包传送 */
  ilSockfd = _bmqConnectfileser(iGrpid);
  if ( ilSockfd < 0 )
  {
    _bmqDebug("S0300: --%d--连接到服务器出错!",ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"连接服务器失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }

  /* add by xujun 2006.11.15 支持断点续传 begin */
  memset(alOffset,0x00,sizeof(alOffset));
  memset(alTmpFileName,0x00,sizeof(alTmpFileName));
  sprintf(alTmpFileName,"%s/temp/%s.tpf",getenv("BMQ_FILE_DIR"),aFilename);
  llOffset = 0;
  if (( fq = fopen(alTmpFileName,"r")) != NULL)
  {
    /*文件存在,上次传送未完成,取出上次传送点偏移量*/
    ilRc = fread(alOffset,sizeof(char),sizeof(alOffset),fq);
    llOffset = atol(alOffset);
    fclose(fq);
  }
  /* add by xujun 2006.11.15 支持断点续传 end */

  /* 读取参数,是否需要加密和压缩 */
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"COMPRESSFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 COMPRESSFLAG 字段失败!",alInitfile);
    alResult[0] = '0'; /*缺省为不压缩*/
  }
  strcpy(ilCompressFlag,alResult);
  ilRc = _bmqGetfield(alInitfile,"CRYPTFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 CRYPTFLAG 字段失败!",alInitfile);
    alResult[0] = '0'; /*缺省为不加密*/
  }
  strcpy(alCryptFlag,alResult);
  /* 组织命令报文 */
  ilMsglen = 29 + strlen(aFilename);
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 8;
  sprintf(alVal,"%5s",ilCompressFlag);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%5s",alCryptFlag);
  memcpy(alMsgbuf+14,alVal,5);
  sprintf(alVal,"%10ld",llOffset);
  memcpy(alMsgbuf+19,alVal,10);
  memcpy(alMsgbuf+29,aFilename,strlen(aFilename));
  
  /* 发送开始发送文件命令报文到服务器 */
  if(llOffset == 0)
  {
    strcpy(sgMonMsg.aStatusDesc,"开始发送文件");
    sgMonMsg.iSendFlag  = 1;
  }
  else
  {
  	strcpy(sgMonMsg.aStatusDesc,"继续发送文件");
    sgMonMsg.iSendFlag  = 2;
  }
  sgMonMsg.lSendSize  = llOffset;
  sgMonMsg.fSendSpeed = 0;

  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0310: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"开始发送请求失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }

  /* 接收响应报文 */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0320: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"接收开始发送请求响应报文失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }

  if(memcmp(alMsgbuf,"OK",2))
  {
    _bmqDebug("S0330: 服务器拒绝接收文件！");
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"服务器拒绝接收文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }
 
  if(!llOffset)
  {
    memcpy(alVal,alMsgbuf+2,15);
    alVal[15] = 0x00;
    *lSerialno = atol(alVal);
    sgMonMsg.lSerialno = *lSerialno;
  }
  memcpy(alVal,alMsgbuf+17,15);
  alVal[25] = 0x00;
  llOffset   = atol(alVal);
  ftime(&sgMonMsg.sBeginTime);
  _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
  
  _bmqfilechr(aFilename,strlen(aFilename),alTmpFileName,'/');
  sprintf(alSendFileName,"%s/saf/%s",getenv("BMQ_FILE_DIR"),alTmpFileName);
  strcpy(alParm[0],alSendFileName);
  sprintf(alParm[1],"%d",ilSockfd);
  sprintf(alParm[2],"%ld",llOffset);
  sprintf(alParm[3],"%5s",ilCompressFlag);
  sprintf(alParm[4],"%5s",alCryptFlag);

  /* 调用函数指针完成文件预处理和发送 */
  ilRc = sTransparm.bmqFile_msg(alParm);
  if(ilRc)
  {
    _bmqDebug("S0340: 文件信息交互处理函数错误，退出发送进程!");
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"文件预处理失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }
  ilRc = sTransparm.bmqFile_comm(alParm);
  if(ilRc)
  {
    _bmqDebug("S0350: 文件发送发生错误,退出发送进程!请查明错误原因后重新发送.");
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"文件发送失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }
  
  /* 组织命令报文 */
  ilMsglen = 9;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 9;			/* 命令标志 9=Sendfile end */  

  /* 发送结束传送标志给服务器 */
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0360: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"发送结束传送标志失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }
  
  ilMsglen = 2;
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S3430: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"发送结束响应接收失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }
  if(memcmp(alMsgbuf,"OK",2))
  {
    _bmqDebug("S3440: 接收方未能正确接收文件[%s],需要重发!",aFilename);
    ilMsglen = 16;
    ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
    if ( ilRc != 0 ) 
    {
      _bmqDebug("S3450: Receive from server error!!--%d",ilRc);
      close(ilSockfd);
      sgMonMsg.iSendFlag  = 4;
      strcpy(sgMonMsg.aStatusDesc,"发送结束响应接收失败");
      _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
      return (FAIL);
    }
    alMsgbuf[ilMsglen] = 0x0;
    if (( fq = fopen(alTmpFileName,"w+")) != NULL)
    {
      /*写入传送点偏移量*/
      ilRc = fwrite(alMsgbuf,sizeof(char),ilMsglen,fq);
      fclose(fq);
    }
    
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    sgMonMsg.lSendSize  = atol(alMsgbuf);
    strcpy(sgMonMsg.aStatusDesc,"接收方未能正确接收文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }

  close(ilSockfd);
  sgMonMsg.iSendFlag  = 3;
  sgMonMsg.lSendSize  = sgMonMsg.lFileSize;
  strcpy(sgMonMsg.aStatusDesc,"文件发送完成");
  ftime(&sgMonMsg.sEndTime);
  sgMonMsg.fSendSpeed = sgMonMsg.lFileSize / 
                         ((sgMonMsg.sEndTime.time-sgMonMsg.sBeginTime.time)*1000 + 
                          (sgMonMsg.sEndTime.millitm -sgMonMsg.sBeginTime.millitm));
  _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  _bmqfilechr(aFilename,strlen(aFilename),alFile,'/');
  sprintf(alTmpFileName,"%s/saf/%s",getenv("BMQ_FILE_DIR"),alFile);
  unlink(alTmpFileName);
  return(SUCCESS);
}  

/**************************************************************/
int bmqGetserialno(char *aFilename,long *lSerialno,long *llOffset)
{
  int	ilRc,ilSockfd;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char	alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];

  /* 连接到服务器 */
  ilSockfd = _bmqConnectfileser(0);
  if ( ilSockfd < 0 )
  {
    _bmqDebug("S0390: --%d--连接到服务器出错!",ilSockfd);
    return (ilSockfd);
  }
   
  ilMsglen = 9 + strlen(aFilename);
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 10;
  memcpy(alMsgbuf+9,aFilename,strlen(aFilename));


  /* 发送开始发送文件命令报文到服务器取文件serialno */ 
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0400: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    return (ilRc);
  }

  /* 接收响应报文 */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0410: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    return (ilRc);
  }

  if(memcmp(alMsgbuf,"OK",2))
  {
    _bmqDebug("S0420: 服务器拒绝接收文件！");
    close(ilSockfd);
    return(-1);
  }
 
  memcpy(alVal,alMsgbuf+2,15);
  alVal[15] = '\0';
  *lSerialno = atol(alVal);
  
  close(ilSockfd);
  return(SUCCESS);
}  
/**************************************************************
 ** 函数名:  bmqRecvfile
 ** 功能:    发送文件到Server
 ** 作者:    徐军 	
 ** 建立日期: 2002/12/23
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0--成功
***************************************************************/
int bmqRecvfile(long lSerialno,FILE **fp)
{
  int		ilRc,ilSockfd;
  /*short 	ilBuflen,ilMsglen,ilLen,ilLen1,ilMsglen1; delete by wh*/
  TYPE_SWITCH ilBuflen,ilMsglen,ilLen,ilLen1,ilMsglen1; /*add by wh*/
  char		alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];
  char		alSendbuf[iMBMAXPACKSIZE+100];
  char		alMac[MAC_LEN];
  char		alResult[10];
  char		alInitfile[101];
  char		alFilename[101];
  FILE		*fq;
  long		llFileserial,llFileid;
  struct	timeval timev;
  long		i = 0;
  int		ilErrFlag;
  int		ilSrvBakFlag;
  char ilCompressFlag[10];
  char		alCryptFlag[10];

  memset(alVal,0x00,sizeof(alVal));
  
  *fp = NULL;
 
  /* 连接到服务器 */
  ilSockfd = _bmqConnectfileser(0);
  if ( ilSockfd < 0 )
  {
    _bmqDebug("S0430: --%d--连接到服务器出错!",ilSockfd);    
    return (ilSockfd);
  }

  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"SRVBAKFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 MBPROCTYPE 字段失败!",alInitfile);
    alResult[0] = '0';
  }
  ilSrvBakFlag = atoi(alResult);
  ilRc = _bmqGetfield(alInitfile,"COMPRESSFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 COMPRESSFLAG 字段失败!",alInitfile);
    alResult[0] = '0'; /*缺省为不压缩*/
  }
  strcpy(ilCompressFlag,alResult);
  ilRc = _bmqGetfield(alInitfile,"CRYPTFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 CRYPTFLAG 字段失败!",alInitfile);
    alResult[0] = '0'; /*缺省为不加密*/
  }
  strcpy(alCryptFlag,alResult);

  /* 组织命令报文 */
  ilMsglen = 32;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 11;			/* 命令标志 11=Request Recvfile */
  sprintf(alVal,"%08ld",lSerialno);
  memcpy(alMsgbuf+9,alVal,8);
  sprintf(alVal,"%05d",ilSrvBakFlag);
  memcpy(alMsgbuf+17,alVal,5);
  sprintf(alVal,"%5s",ilCompressFlag);
  memcpy(alMsgbuf+22,alVal,5);
  sprintf(alVal,"%5s",alCryptFlag);
  memcpy(alMsgbuf+27,alVal,5);

  /* 发送请求接收文件命令报文到服务器 */ 
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0440: Send to server ERROR!!--%d",ilRc);
    return (ilRc);
  }
  
  /* 接收响应报文 */
  ilMsglen = sizeof(alMsgbuf);
  memset(alMsgbuf,0x00,sizeof(alMsgbuf));
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0450: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    return (ilRc);
  }

  if(memcmp(alMsgbuf,"BEGIN",5))
  {
    _bmqDebug("S0460: 服务器未能准备号开始发送文件！");
    close(ilSockfd);
    return(-1);
  }
    
  sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/recv/",alMsgbuf+5);
  if ( (fq = fopen(alFilename,"w+")) == NULL ) 
  {
    _bmqDebug("S0470: 不能打开文件【%s】",alFilename);
    close(ilSockfd);
    return(-1);
  }
  
  ilMsglen = 9;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 12;			/* 命令标志 12=Recvfile is ready... */   

  /* 发送接收准备已毕文件命令报文到服务器 */ 
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0480: Send to server ERROR!!--%d",ilRc);
    fclose(fq);
    return (ilRc);
  }
  
  /* 设置SOCKET超时时间 */
  timev.tv_sec = 15;                                        
  timev.tv_usec = 0;                                             
  setsockopt(ilSockfd,SOL_SOCKET,SO_RCVTIMEO,&timev,sizeof(timev));

  llFileserial = 0; 
  while(1)
  {
    i++;
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;
   
    ilRc = _comTcpReceive(ilSockfd, alMsgbuf, &ilMsglen) ; 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0490: 收报文出错!-Retcode=%d",ilRc);
      close( ilSockfd );
      fclose(fq);
      return (ilRc);
    }  	
    
    memcpy(alVal,alMsgbuf,8);
    ilLen = atoi(alVal);

    ilLen1 = ilMsglen1 =0;
    while( ilMsglen1 < ilLen )
    {
      ilMsglen = ilLen - 8 - ilLen1;
      ilRc = _comTcpReceive(ilSockfd, alMsgbuf+8+ilLen1, &ilMsglen) ;
      if ( ilRc != 0 || ilMsglen <= 0)
      {
        _bmqDebug("S0500: 收报文出错!-Retcode=%d",ilRc);
        close( ilSockfd );
        fclose(fq);
        return (ilRc);
      }
      ilLen1 += ilMsglen;
      ilMsglen1 = ilLen1 + 8;
    }
    /* 报文解析 */
    switch(alMsgbuf[8])
    {
      case 'F': /* file transform*/
        memcpy(alVal,alMsgbuf+9,8);
        alVal[8] = '\0';
        llFileid = atol(alVal);
        ilMsglen = ilLen - 17 - MAC_LEN;
        if( !ilErrFlag && (llFileid == llFileserial+1) )
        {
          _bmqMac(alMsgbuf+17,ilMsglen,alMac);
          if( (memcmp(alMac,alMsgbuf+ilLen-MAC_LEN,MAC_LEN)) &&
              (memcmp(alMsgbuf+ilLen-MAC_LEN,"00000000",MAC_LEN)) )
          {
            /*接收的数据文件可能不正确*/
            ilErrFlag = 1;
            break;
          }
          if(strlen(alCryptFlag)!=0)
          {
            _bmqCrypt_all(alMsgbuf+17,ilLen-17-MAC_LEN,FILEKEY,alCryptFlag);
          }
          /* 解压缩报文数据 */
          if(strlen(ilCompressFlag)!=0)
            ilBuflen = _bmqdecompress(alMsgbuf+17,ilLen-17-MAC_LEN,alSendbuf,ilCompressFlag);
          else
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilBuflen);
          }
          if(ilBuflen==0)
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilBuflen);
          }
          if(ilBuflen > 0)
            ilMsglen = fwrite(alSendbuf,sizeof(char),ilBuflen,fq);
          llFileserial = llFileid;
        }
        break;                    
      case 'E': /* file tranform end flag */
        close(ilSockfd);
        fclose(fq);
        if ((*fp = fopen(alFilename, "r")) == NULL)
        {
          _bmqDebug("S0520: 文件[%s]打开失败:%d,%s",alFilename,errno,strerror(errno));
          *fp = NULL;
          return(FAIL); 
        }
        return(SUCCESS);
      case 'R':/*　open file error */
        return(SUCCESS);
      default:
        _bmqDebug("S0530: 不可识别的命令类型[%d]",alMsgbuf[8]);
        ilMsglen = 0;
        break;
    }
  } 
}  

/**************************************************************
 ** 函数名:  bmqPutfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/23
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
***************************************************************/
/*int bmqPutfile(short iGrpid,short iMbid,short iPrior,long lType,
  char *aMsgbuf,short iMsglen,char *aFilename,struct transparm sTransparm,long lSerialno) delete by wh*/
int bmqPutfile(short iGrpid,short iMbid,short iPrior,long lType,
  char *aMsgbuf,TYPE_SWITCH iMsglen,char *aFilename,struct transparm sTransparm,long lSerialno) /*add by wh*/
{
  int	ilRc;
  if(aFilename != NULL)
  {
    ilRc = bmqSendfile(0,aFilename,sTransparm,&lSerialno);
    if(ilRc)
    {
      _bmqDebug("S0540: 发送文件到Server失败!");
      return(FAIL);
    }
  }   
  
  ilRc = bmqPut(iGrpid,iMbid,iPrior,lType,lSerialno,aMsgbuf,iMsglen);
  if(ilRc)
  {
    _bmqDebug("S0550: [函数调用失败]bmqPut error!");
    return(FAIL);	
  }
  
  return(SUCCESS);	
}

/**************************************************************
 ** 函数名:  bmqGetfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/23
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
***************************************************************/
/*int bmqGetfile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,
  char *aMsgbuf,short *piMsglen,short iTimeout,FILE **fp) delete by wh*/
int bmqGetfile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,
  char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout,FILE **fp)
{
  int	ilRc;
  
  *fp = NULL;
  ilRc = bmqGetw(piGrpid,piMbid,piPrior,plType,plClass,aMsgbuf,piMsglen,iTimeout);
  if(ilRc)
  {
    _bmqDebug("S0560: [函数调用失败]bmqGetw error! ilRc=%d",ilRc);
    *fp = NULL;
    return(FAIL);
  }
    
  if(*plClass)
  {
    ilRc = bmqRecvfile(*plClass,fp);	
    if(ilRc)
    {
      _bmqDebug("S0570: [函数调用失败]bmqRecvfile() error");
      return(FAIL);
    }    	
  }  	
     
  return(SUCCESS);	
}

/*int bmqGetfileX(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,
  char *aMsgbuf,short *piMsglen,short iTimeout,char *aFileName)
  delete by wh*/
int bmqGetfileX(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,
  char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout,char *aFileName) /*add by wh*/
{
  int	ilRc;
  ilRc = bmqGetw(piGrpid,piMbid,piPrior,plType,plClass,aMsgbuf,piMsglen,iTimeout);
  if(ilRc)
  {
    _bmqDebug("S0580: [函数调用失败]bmqGetw error! ilRc=%d",ilRc);
    return(FAIL);
  }
    
  if(*plClass)
  {
    ilRc = bmqRecvfileX(*plClass,aFileName);	
    if(ilRc)
    {
      _bmqDebug("S0590: [函数调用失败]bmqRecvfileX() error");
      aFileName = NULL;
      return(FAIL);
    }
  }  	
     
  return(SUCCESS);	
}

/* add by xujun 2006.11.20 for shanghai test file trans begin */
/*int bmqPutFile(short iGrpid,short iMbid,short iPrior,long lType,char *aMsgbuf,short iMsglen,char *aFilename) delete by wh*/
int bmqPutFile(short iGrpid,short iMbid,short iPrior,long lType,char *aMsgbuf,TYPE_SWITCH iMsglen,char *aFilename) /*add by wh*/
{
  int	ilRc;
  struct bmqMessage sendmulfile;  
  memset(&sendmulfile,0x00,sizeof(struct bmqMessage));
  sendmulfile.iGrpid=iGrpid;
  sendmulfile.iMbid=iMbid;
  sendmulfile.iPrior=iPrior;
  sendmulfile.lType=lType;
  strcpy(sendmulfile.aTopic,aFilename);
  strcpy(sendmulfile.aFileName[0],aFilename);
  sendmulfile.iFilenum=1;
  ilRc =bmqMulPutFile(sendmulfile);
  if(ilRc)
  {
    printf("[FAIL!]\n");  
    _bmqDebug("S0610: [函数调用失败]bmqPutFilen error!");
    return(FAIL);	
  }  
  return(SUCCESS);	
}

int bmqRecvFile(short iGrpid,char *aFileName)
{
  int	ilRc;
  
  ilRc = bmqRecvfileByName(iGrpid,aFileName,'C');	
  if(ilRc)
  {
    _bmqDebug("S0620: [函数调用失败]bmqRecvfileByName() 失败");
    return(ilRc);
  }  	
     
  return(SUCCESS);	
}

int bmqSendFile(short iGrpid,char *aFilename)
{
  int	ilRc;
  long	llSerialno = 0;
  struct transparm slTransparm;
  struct stat f_stat; 
  
  memset(&sgMonMsg,0x00,sizeof(struct monmsg));
  if( stat( aFilename, &f_stat ) == -1 )
  {
  	_bmqDebug("S0628: 无法打开指定文件[%s]!",aFilename);
  	sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aFileName,aFilename);
    strcpy(sgMonMsg.aStatusDesc,"无法打开指定文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }
  
  slTransparm.bmqFile_comm	= _bmqFile_comm;
  slTransparm.bmqFile_msg	= _bmqFile_msg;
  
  if(aFilename != NULL)
  {	
  	sgMonMsg.iOrgGrpid  = 0;
    sgMonMsg.iOrgMbid   = igMb_sys_current_mailbox;
    sgMonMsg.iDesGrpid  = iGrpid;
    sgMonMsg.lFileSize  = f_stat.st_size;
    sgMonMsg.lSendPid   = getpid(); 
    strcpy(sgMonMsg.aFileName,aFilename);
    ilRc = bmqSendfile(iGrpid,aFilename,slTransparm,&llSerialno);
    if(ilRc)
    {
      _bmqDebug("S0630: 发送文件失败!");
      return(FAIL);
    }
  }
  
  return(SUCCESS);
}

/*int bmqGetFile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,short *piMsglen,short iTimeout,char *aFileName,char *alFilepath)
 delete by wh*/
int bmqGetFile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout,char *aFileName,char *alFilepath) /*add by wh*/
{
  int	ilRc;
  struct bmqMessage revmulfile;
  memset(&revmulfile,0x00,sizeof(struct bmqMessage));
  revmulfile.iGrpid=*piGrpid;
  revmulfile.iMbid=*piMbid;
  revmulfile.iPrior=*piPrior;
  revmulfile.lType=*plType;
  revmulfile.lClass=0;
  strcpy(revmulfile.aTopic,"Receive...");
  revmulfile.iFilenum=1;
  ilRc   =bmqMulGetFile(&revmulfile,iTimeout,alFilepath);
  if(ilRc)
  {     
    _bmqDebug("S0610: [函数调用失败]bmqPutFilen error!");
    return(FAIL);	
  } 
  *piGrpid=revmulfile.iGrpid;
  *piMbid=revmulfile.iMbid;
  *piPrior=revmulfile.iPrior;
  *plType=revmulfile.lType;
  *plClass=revmulfile.lClass;
   memcpy(aFileName,revmulfile.aFileName[0],80);
   return(SUCCESS);	
}

int bmqTestPutFile()
{
  char alVal[iMASKLEN + 1];
  char alMsgbuf[iMBMAXPACKSIZE+100];
  int  ilRc;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  FILE  *fp;

  /*组织报文*/
  memset(alVal,0x00,sizeof(alVal));
  ilMsglen =  135;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 8;
  sprintf(alVal,"%05d",2);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%05d",88);
  memcpy(alMsgbuf+14,alVal,5);
  sprintf(alVal,"%05d",0);
  memcpy(alMsgbuf+19,alVal,5);
  sprintf(alVal,"%10ld",0L);
  memcpy(alMsgbuf+24,alVal,10);
  memcpy(alMsgbuf+34,"testfile",8);
  fp = fopen("testfile","a+");
  sprintf(alVal,"%16ld",ftell(fp));
  memcpy(alMsgbuf+114,alVal,16);
  memcpy(alMsgbuf+130,"xujun",5);

  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
  if(ilRc)
  {
    _bmqDebug("S0660: Send buf err!");
    return(-1);
  }

  fseek(fp,0,SEEK_SET);
  /* 发送文件到服务器 */
  while(!feof(fp))
  {
    memset(alMsgbuf,0x00,sizeof(alMsgbuf));
    ilMsglen = fread(alMsgbuf,sizeof(char),3000,fp);
    ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
    if(ilRc)
    {
      _bmqDebug("S0670: Send file err!");
      return(-1);
    }
  } 
  
  /* 接收报文 */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( igSockfd,alMsgbuf,&ilMsglen);
  memcpy(alVal,alMsgbuf+1,5);
  alVal[5]=0;
  ilRc = atoi(alVal);
  return(ilRc);
}
/* add by xujun 2006.11.20 for shanghai test file trans end */

/* add by hxz 2007.01.19 for mulfiles send begin */
int bmqMulPutFile(struct bmqMessage sendmulfile)
{  
  char alVal[iMASKLEN + 80];
  char alMsgbuf[iMBMAXPACKSIZE+100];
  int  ilRc,i;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char	alCommand[120];
  char  alTmpFileName[100];
  FILE  *fp;
  struct stat f_stat;
  /*
  strcpy(alVal,sendmulfile.aTopic);
  _bmqTrim(alVal);
  if(strstr(alVal," "))
  { _bmqDebug("S0450: In aTopic whitespace appear!【%s】",sendmulfile.aTopic);
    return(-1);
  }  
  */
  /*组织报文*/
  memset(alVal,0x00,sizeof(alVal));        /*报文的前八个字节定义为报文的长度  */     
  alMsgbuf[8] = 'C';                       /*报文的第九个字节定义为请求的名令类型:  */
  sprintf(alVal,"%05d",sendmulfile.iGrpid);            /*报文的第10～14个字节为组号            */ 
  memcpy(alMsgbuf+9,alVal,5);                   
  sprintf(alVal,"%05d",sendmulfile.iMbid);             /*报文的第15～19个字节为邮箱号           */
  memcpy(alMsgbuf+14,alVal,5);                 
  sprintf(alVal,"%05d",sendmulfile.iPrior);            
  memcpy(alMsgbuf+19,alVal,5);             /*第20～24字节：  报文优先级             */
  sprintf(alVal,"%10ld",0L);                                                      
  memcpy(alMsgbuf+24,alVal,10);            /*第25～34字节：  报文过滤条件           */
  sprintf(alVal,"%40s",sendmulfile.aTopic);                                                      
  memcpy(alMsgbuf+34,alVal,40);            /*消息主题*/
  sprintf(alVal,"%5d",sendmulfile.iFilenum);      
  memcpy(alMsgbuf+74,alVal,5);             /*附件个数*/
  ilMsglen=79;  
  for(i=0;i<sendmulfile.iFilenum;i++)
  {
    if( stat( sendmulfile.aFileName[i], &f_stat ) == -1 )
    {
      _bmqDebug("S0470: 不能打开文件【%s】",sendmulfile.aFileName[i]);
      return(-1);
    }
    sprintf(alVal,"%s",sendmulfile.aFileName[i]);
    memset(alCommand,0x00,sizeof(alCommand));
    _bmqfilechr(alVal,strlen(alVal),alTmpFileName,'/');
    sprintf(alVal,"%80s",alTmpFileName);
    memcpy(alMsgbuf+79+96*i,alVal,80);
    ilMsglen=ilMsglen+80;
    sprintf(alVal,"%16d",f_stat.st_size);
    memcpy(alMsgbuf+ilMsglen,alVal,16);
    ilMsglen=ilMsglen+16; 
  }
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
  if(ilRc)
  {
    _bmqDebug("S0660: Send buf err!");
    return(-1);
  }
  for(i=0;i<sendmulfile.iFilenum;i++)
  {
    if( stat( sendmulfile.aFileName[i], &f_stat ) == -1 )
    {
      _bmqDebug("S0470: 不能打开文件【%s】",sendmulfile.aFileName[i]);
      return(-1);
    }
    fp=fopen(sendmulfile.aFileName[i],"r");
    fseek(fp,0,SEEK_SET);
    /* 发送文件到服务器 */
    while(!feof(fp))
    {
      memset(alMsgbuf,0x00,sizeof(alMsgbuf));
      ilMsglen = fread(alMsgbuf,sizeof(char),1024,fp);
      ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
      if(ilRc)
      {
        _bmqDebug("S0670: Send file err!");
        return(-1);
      }
    }     
  }
  /* 接收报文 */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( igSockfd,alMsgbuf,&ilMsglen);
  memcpy(alVal,alMsgbuf+1,5);
  alVal[5]=0;
  ilRc = atoi(alVal);
  return(ilRc);
}
/* add by hxz 2007.01.19 for mulfiles send end */

/* add by hxz 2007.01.19 for mulfiles reveice begin */
int bmqMulGetFile(struct bmqMessage *revmulfile,short iTimeout,char *alFilepath)
{
  
  char alVal[iMASKLEN + 800];
  char alMsgbuf[iMBMAXPACKSIZE+100];
  int  ilRc,i;
  /*short ilMsglen,ilLen,ilLen1,ilMsglen1; delete by wh*/
  TYPE_SWITCH ilMsglen,ilLen,ilLen1,ilMsglen1; /*add by wh*/
  char  alTmpFileName[100];
  char altmpfullname[256];
  long ilFilelen;
  FILE  *fp;
  /*组织报文*/
  memset(alVal,0x00,sizeof(alVal));
  alMsgbuf[8] = 'D';     
  sprintf(alVal,"%05d",revmulfile->iGrpid);
  memcpy(alMsgbuf+9,alVal,5);    
  sprintf(alVal,"%05d",revmulfile->iMbid);             
  memcpy(alMsgbuf+14,alVal,5);    
  sprintf(alVal,"%05d",revmulfile->iPrior); 
  memcpy(alMsgbuf+19,alVal,5);                
  sprintf(alVal,"%10ld",revmulfile->lType); 
  memcpy(alMsgbuf+24,alVal,10);                
  sprintf(alVal,"%10ld",revmulfile->lClass); 
  memcpy(alMsgbuf+34,alVal,10);   
  sprintf(alVal,"%05d",iTimeout);
  memcpy(alMsgbuf+44,alVal,5);
  ilMsglen=49;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  /* 发送报文到服务器 */
  ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0150 %s : Send to server ERROR!!--%d",__FILE__,ilRc);
    return (ilRc);
  }
  /* Receive from socket */
  memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
  ilMsglen = 8;
  ilRc = _comTcpReceive(igSockfd, alMsgbuf, &ilMsglen) ; 
  if ( ilRc != 0 || ilMsglen <= 0)
  {
    _bmqDebug("S0160 收报文出错!-Retcode=%d",ilRc);
    close( igSockfd );
    return (ilRc);
  }
  memset ( alVal, 0x00, sizeof(alVal) );
  memcpy(alVal,alMsgbuf,8);
  ilLen = atoi(alVal);
  ilLen1 = ilMsglen1 =0;
  while( ilMsglen1 < ilLen )
  {
    ilMsglen = ilLen - 8 - ilLen1;
    ilRc = _comTcpReceive(igSockfd, alMsgbuf+8+ilLen1, &ilMsglen) ;
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0170 收报文出错!-Retcode=%d",ilRc);
      close( igSockfd );
      return (ilRc);
    }
    ilLen1 += ilMsglen;
    ilMsglen1 = ilLen1 + 8;
  }
  /* 报文解包 */
  memset(alVal,0x00,sizeof(alVal));
  memcpy(alVal,alMsgbuf+9,5);
  ilRc = atoi(alVal);
  if (ilRc) return(ilRc);
  memcpy(alVal,alMsgbuf+14,5);
  revmulfile->iGrpid = atoi(alVal);
  memcpy(alVal,alMsgbuf+19,5);
  revmulfile->iMbid = atoi(alVal);
  memcpy(alVal,alMsgbuf+24,5);
  revmulfile->iPrior = atoi(alVal);
  memcpy(alVal,alMsgbuf+29,10);
  revmulfile->lType = atol(alVal);
  memcpy(alVal,alMsgbuf+39,10);
  revmulfile->lClass = atol(alVal);
  memset(revmulfile->aTopic,0x00,sizeof(revmulfile->aTopic));
  memcpy(revmulfile->aTopic,alMsgbuf+49,40);
  memset(alVal,0x00,sizeof(alVal));
  memcpy(alVal,alMsgbuf+89,5);
  revmulfile->iFilenum=atoi(alVal);
  for(i=0;i<revmulfile->iFilenum;i++)
  {
    memset(revmulfile->aFileName[i],0x00,100);
    memcpy(revmulfile->aFileName[i],alMsgbuf+94+96*i,96);
  }
  for(i=0;i<revmulfile->iFilenum;i++)
  {   
    strcpy(altmpfullname,alFilepath);
    memset(alTmpFileName,0x00,sizeof(alTmpFileName));
    memset(alVal,0x00,sizeof(alVal));
    memcpy(alVal,revmulfile->aFileName[i],80);
    _bmqTrim(alVal);
    strcat(altmpfullname,alVal);
    memcpy(alVal,revmulfile->aFileName[i]+80,16);
    _bmqTrim(alVal);
    ilFilelen=atol(alVal);
    fp=fopen(altmpfullname,"w+");
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 1024;
    while(ilFilelen>0)
    {  
       if(ilFilelen-ilMsglen>1024)
       {
         ilMsglen=1024;
         ilFilelen=ilFilelen-1024;  
       }
       else
       {
         ilMsglen=ilFilelen;
         ilFilelen=0;
       }         
       ilRc = _comTcpReceive(igSockfd, alMsgbuf, &ilMsglen) ; 
       if ( ilRc != 0 || ilMsglen <= 0)
       {
         _bmqDebug("S0130 收报文出错!-Retcode=%d",ilRc);
         close( igSockfd );
         fclose(fp);
         return (ilRc);
       }
       fwrite(alMsgbuf,sizeof(char),ilMsglen,fp);
    }
    fclose(fp);
  }
  ilMsglen = 4;
  strcpy(alTmpFileName,"AAAA");
  ilRc = _comTcpSend(igSockfd,alTmpFileName, &ilMsglen);
  if( ilRc )
  {
    _bmqDebug("S0170: 发送确认信息出错!");
    return (ilRc);	
  }
  return(ilRc);	
}
/* add by hxz 2007.01.19 for mulfiles reveice end */

/* add by hxz 2007.01.19 for delete whitespace begin */
int _bmqDelspace(char *alstr)
{
 int i,n=0;
 char alTmpstr[iMBMAXPACKSIZE];
 memset(alTmpstr,0x00,sizeof(alTmpstr));
 _bmqTrim(alstr);
 for(i=0;i<strlen(alstr);i++) 
 {
  if(!(alstr[i]==' '))
  {
   alTmpstr[n]= alstr[i];
   n++;
  }
 }  
 strcpy(alstr,alTmpstr);
 return(0);
}
/* add by hxz 2007.01.19 for delete whitespace end */



