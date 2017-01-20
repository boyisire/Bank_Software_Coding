#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

/*函数原型*/
int swLoadcomcfg( char *aPortName );
int swConnectser(char *aIp,int iPort);
int swTcprcv(int iSockfd,char *aBuffer,unsigned int *piLength);
int swTcpconnect( int iPort );
int swReadf(int iSockfd,char *aBuffer,unsigned int *iMsglen);
int swTcpsnd(int iSockfd,char *aBuffer,unsigned int iLen);
int swReadn(int iSockfd, char *aBuffer,unsigned int iLength);
int swConnsend(short iflag,int *iSockfd, char *aMsgbuf,unsigned int iMsglen);
int swCheck_comm();

/****************************************************************/
/* 函数编号    ：swLoadcomcfg                                   */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2001/5/                                        */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/

int swLoadcomcfg( char *aPortName )
{
  FILE  *fp;
  int  ilRc;
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  
  swVdebug(4,"S0010: [函数调用] swLoadcomcfg(%s)",aPortName);
  memset( (char *)&sgComcfg, 0x00, sizeof(sgComcfg));

  if((fp=_swFopen("config/SWCOMM.CFG","r"))==NULL)
  {
    swVdebug(0,"S0020: 打开通讯配置文件出错!");
    return(-1);
  }

  /* 预读通讯端口配置文件到数组中 */
  while (1)
  {
    memset(palFldvalue,0x0,sizeof(palFldvalue));
    swVdebug(2,"S0030: iFLDNUMBER = %d\n",iFLDNUMBER);
/*    printf("iFLDNUMBER = %d\n",iFLDNUMBER);*/
    ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
    if (ilRc < 0) break;

    _swTrim( palFldvalue[0] );
    
    if ( strcmp( palFldvalue[0], aPortName ) == 0 )
    {
      swReadcomcfg(palFldvalue);
      strcpy( sgTcpcfg.aPartner_addr, palFldvalue[14] );
      sgTcpcfg.iPartner_port = atoi(palFldvalue[15]);

      swVdebug(3,"S0040: 端口通讯TCP参数:");
      swVdebug(3,"S0050: sgTcpcfg.aPartner_addr= [%s]",sgTcpcfg.aPartner_addr);
      swVdebug(3,"S0060: sgTcpcfg.iPartner_port= [%d]",sgTcpcfg.iPartner_port);
       
      fclose(fp);
      swVdebug(4,"S0070: [函数返回] swDoitass()返回码=0");
      return(0);
    } /* end if */   
  } /* end while */

  swVdebug(0,"S0080: 读取通讯配置文件出错!");
  fclose(fp);
  return(-1);
}
  
/****************************************************************/
/* 函数编号    ：swConnectser                                   */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2001/5/15                                      */
/* 最后修改日期：2001/9/15                                      */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   -1                                      */
/****************************************************************/

int swConnectser(char *aIp,int iPort)
{
  struct sockaddr_in slServ_addr;

  int ilSockfd;
  int ilRc;

  swVdebug(4,"S0090: [函数调用] swConnetser()");
  /* create endpoint */
  if ((ilSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    swVdebug(0,"S0100: [错误/系统调用] socket()函数 F[%s]:L[%d]:Connect Server Error",__FILE__,__LINE__);
    swVdebug(0,"S0110: [错误/系统调用] socket()函数 Connect Server Error errno=%d[%s]",errno,strerror(errno));
    return -1;
  }
  swVdebug(2,"S0120: [%s][%d]",aIp,iPort);
 
  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( aIp );
  slServ_addr.sin_port = htons( iPort );

  ilRc = connect(ilSockfd,(struct sockaddr *)&slServ_addr,sizeof(slServ_addr));
  if (ilRc < 0)
  {
    swVdebug(0,"S0130: [错误/系统调用] connect()函数,F[%s]:L[%d]:Connect Server Error",__FILE__,__LINE__);
    swVdebug(0,"S0140: [错误/系统调用] connect()函数,Connect Server Error Code[%d]--%s",errno,
      strerror(errno));
    close(ilSockfd);
    return -1;
  }
  swVdebug(4,"S0150: [函数返回] swConnectser()返回码=%d",ilSockfd);
  return ilSockfd;
}
 
/****************************************************************/
/* 函数编号    ：swTcprcv                                       */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/8                                       */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/
int swTcprcv(int iSockfd,char *aBuffer,unsigned int *piLength)
{
  int    ilRc = -1;
  unsigned int ilRcvlen=0;
  unsigned int ilLength=0;
  char   alLen[20];

  swVdebug(4,"S0160: [函数调用] swTcprcv()");
  if (sgComcfg.aExp_len[0])
  {
    /* 根据长度表达式读取报文 */
    ilRc = swReadn(iSockfd,aBuffer,sgComcfg.iPre_msglen);
    if (ilRc)
    {
      swVdebug(0,"S0170: [错误/函数调用] swReadn()函数出错,返回码=%d",ilRc);
      return(-1);
    }
    agMsgbody = aBuffer;
    igMsgbodylen = sgComcfg.iPre_msglen;
    ilRc = _swExpress(sgComcfg.aExp_len,alLen,&ilLength);
    if (ilRc)
    {
      swVdebug(0,"S0180: [错误/函数调用] _swExpress()函数出错,返回码=%d",ilRc);
      return(-1);
    }
    alLen[ilLength] = '\0';
    ilRcvlen = atoi(alLen);
    ilRc=swReadn(iSockfd,aBuffer+sgComcfg.iPre_msglen,ilRcvlen);
    if ( ilRc )
    {
      swVdebug(0,"S0190: [错误/函数调用] swReadn()出错,返回码=%d",ilRc);
      return(-1);
    }
    *piLength = sgComcfg.iPre_msglen + ilRcvlen;
    swVdebug(4,"S0200: [函数返回] swTcprcv()返回码=0");
    return(0);
  }

  if (sgComcfg.aEnd_string[0])
  {
    /* 根据结束字符读取报文 */
    ilRc = swReadf(iSockfd,aBuffer,piLength);
    if (ilRc)
    {
      swVdebug(0,"S0210: [错误/函数调用] swReadf()函数出错,返回码=%d",ilRc);
      return(-1);
    }
    swVdebug(4,"S0220: [函数返回] swTcprcv()返回码=0");
    return(0);
  }

  /* 无条件一次性读取报文 */
  ilRc = recv(iSockfd,aBuffer,iMSGMAXLEN,0);
  if (ilRc < 0)
  {
    swVdebug(0,"S0230: [错误/系统调用] recv()出错,errno=%d[%s]",errno,strerror(errno));
    return(-1);
  }
  *piLength = ilRc;
  swVdebug(2,"S0240: recv()成功");
  swVdebug(4,"S0250: [函数返回] swTcprcv()返回码=0");
  return(0);
}

/****************************************************************/
/* 函数编号    ：swReadf                                        */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/

int swReadf(int iSockfd,char *aBuffer,unsigned int *iMsglen)
{
  int  ilRc,ilLen = 0;
  unsigned int ilStrlen;
  char alString[11];
  unsigned int ilMsglen;
   
  swVdebug(4,"S0260: [函数调用] swReadf()函数");
  ilRc = swHextoasc(sgComcfg.aEnd_string,alString,&ilStrlen);
  if (ilRc)
  {
    swVdebug(0,"S0270: [错误/函数调用] swHextoasc()函数,报文结束符定义错,返回码=%d",ilRc);
    return(-1);
  }
  ilMsglen = 0;
  for (;;)
  {
    ilLen = read(iSockfd,aBuffer+ilMsglen,1);
    if (ilLen<0) 
    {
      if (errno==EINTR)
      {
        continue;
      }
      swVdebug(0,"S0280: [错误/系统调用] read()函数出错,errno=%d[%s]",errno,strerror(errno));
      return(-1);
    }
    if (ilLen==0)
    {
      swVdebug(0,"S0290: [错误/其它] read()读到空报文");
      return(-1);
    }
    ilMsglen=ilMsglen+ilLen;
    if (ilMsglen>=ilStrlen)
    {
      ilRc = memcmp(aBuffer+ilMsglen-ilStrlen,alString,ilStrlen);
      if (ilRc == 0)
      {
        *iMsglen = ilMsglen;
        swVdebug(4,"S0300: [函数返回] swReadf()返回码=0");
        return(0);
      }
    }
  }
}

/****************************************************************/
/* 函数编号    ：swReadn                                        */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/

int swReadn(int iSockfd, char *aBuffer,unsigned int iLength)
{
  int  ilLen = 0;
  int  ilTotalcnt=0;

  swVdebug(4,"S0310: [函数调用] swReadn()");
  for(;;)
  {
    ilLen = read(iSockfd, aBuffer+ilTotalcnt,iLength-ilTotalcnt);
    if (ilLen<0) 
    {
      if (errno==EINTR)
      {
        continue;
      }
      swVdebug(0,"S0320: [错误/系统调用] read()函数出错,errno=%d[%s]",errno,strerror(errno));
      return(-1);
    }
    if (ilLen==0)
    {
      swVdebug(0,"S0330: [错误/其它] read()读到空报文");
      return(-1);
    }
    ilTotalcnt=ilTotalcnt+ilLen;
    if (ilTotalcnt >= iLength)
    {
      swVdebug(4,"S0340: [函数返回] swReadn()返回码=0");
      return(0);
    }
  }
}

/****************************************************************/
/* 函数编号    ：swTcpsnd                                     */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/

int swTcpsnd(int iSockfd,char *aBuffer,unsigned int iLen)
{
  int  ilSendlen = 0;
  int  ilCount= 0;
  
  swVdebug(4,"S0350: [函数调用] swTcpsend()");
  for (;;)
  {
    /*aBuffer = aBuffer + ilCount;*/
    ilSendlen = write(iSockfd,aBuffer+ilCount,iLen-ilCount);
    swVdebug(2,"S0360: ilSendlen=%d",ilSendlen);
    if (ilSendlen < 0)
    {
      if (errno==EINTR)
      {
        ilSendlen = 0;
        continue;
      }
      else
      {
        swVdebug(0,"S0370: [错误/系统调用] write()函数出错,errno=%d[%s]",errno,strerror(errno));
        return(-1);
      }
    }
    if (ilSendlen == 0)
    {
      swVdebug(0,"S0380: [错误/其它] write()函数发送字节数为0");
      return(-1);
    }
    swVdebug(2,"S0390: write() SUCCESS");
    ilCount=ilCount+ilSendlen;
    if (ilCount >= iLen) 
    {
      swVdebug(4,"S0400: [函数返回] swTcpsend()返回码=0");
      return(0);
    }
  }
}

/****************************************************************/
/* 函数编号    ：swTcpconnect                                   */
/* 函数名称    ：                                               */
/* 作    者    : 顾晓忠                                         */
/* 建立日期    ：2001/8/16                                      */
/* 最后修改日期：                                               */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   -1                                      */
/****************************************************************/

int swTcpconnect( int iPort )
{
  struct sockaddr_in slServ_addr;
  int ilSockid;
  int ilRc;

  swVdebug(4,"S0410: [函数调用] swTcpconnect(%d)",iPort);
  memset((char *)&slServ_addr,0x00,sizeof(struct sockaddr_in));

  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  slServ_addr.sin_port = htons(iPort);
  ilSockid=socket(AF_INET,SOCK_STREAM,0);
  if (ilSockid==-1)
  {
    swVdebug(0,"S0420: [错误/系统调用] socket()函数,创建SOCKET错误,errno=%d[%s]",errno,strerror(errno));
    return(-1);
  }
  ilRc=bind(ilSockid,(struct sockaddr*)&slServ_addr,sizeof(struct sockaddr_in));
  if (ilRc == -1)
  {
    swVdebug(0,"S0430: [错误/系统调用] bind()函数,绑定SOCKET错误,errno=%d[%s]",errno,strerror(errno));
    close(ilSockid);
    return(-1);
  }

  if (listen(ilSockid,SOMAXCONN)==-1)
  {
    swVdebug(0,"S0440: [错误/系统调用] listen()函数,侦听连接错误,errno=%d[%s]",errno,strerror(errno));
    close(ilSockid);
    return(-1);
  }
  swVdebug(4,"S0450: [函数返回] swTcpconnect()返回码=%d",ilSockid);
  return(ilSockid);
}

/****************************************************************/
/* 函数编号    ：swCheck_comm                                   */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  :                                                */
/*                                                              */
/****************************************************************/

int swCheck_comm()
{
  int ilRc,ilConnect_flag=0;
  long ilSockid;
  short ilTrytimes;

  swVdebug(4,"S0460: [函数调用] swCheck_comm()");
  /* next modify by nh 2002.6.6
  ilRc = swPortset( sgComcfg.iMb_comm_id, 1, 0); */
  ilRc = swPortset( sgComcfg.iMb_fore_id, 1, 0);
  if ( ilRc )
  {
    swVdebug(0,"S0470: [错误/函数调用] swPortset()函数,置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
    return( -1 );
  }
  ilTrytimes = sgComcfg.iTrytimes;

  for(;;)
  {
    sleep( sgComcfg.iTryfreq);
    ilSockid = swConnectser( sgTcpcfg.aPartner_addr, sgTcpcfg.iPartner_port );
    if (ilSockid > 0) 
    {
      ilConnect_flag = 1;
      break;
    }
    if (ilTrytimes == -1) continue;
    ilTrytimes --;
    if (ilTrytimes <= 0) break;
  }
  if ( ilConnect_flag == 0 )
  {
    swVdebug(0,"S0480: [错误/函数调用] swConnectser()函数,连接到服务器出错!!");
    return( -1 );
  }
  /* 置端口通讯层为UP */
  /* next modify by nh 2002.6.6
  ilRc = swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
  ilRc = swPortset( sgComcfg.iMb_fore_id, 1, 1 );
  if ( ilRc )
  {
    swVdebug(0,"S0490: [错误/函数调用] swPortset()函数,置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
    return( -1 );
  }
  swVdebug(4,"S0500: [函数返回] swCheck_comm()返回码=%d",ilSockid);
  return( ilSockid );
}

/****************************************************************/
/* 函数编号    ：swswConnsend                                   */
/* 函数名称    ：                                               */
/* 作    者    ：                                               */
/* 建立日期    ：2000/9/15                                      */
/* 最后修改日期：                                               */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   -1                                      */
/****************************************************************/

int swConnsend(short iflag,int *iSockfd, char *aMsgbuf,unsigned int iMsglen)
{
  int    ilRc;

  #ifdef OS_SCO
    int  ilAddrlen;
  #else
    socklen_t  ilAddrlen;
  #endif

  struct linger slLinger;
  struct sockaddr_in slCli_addr;
  struct msghead *pslMsghead;
 
  swVdebug(4,"S0510: [函数调用] swConnsend(%d,%d,%d)",iflag,*iSockfd,iMsglen);
  pslMsghead = (struct msghead *)aMsgbuf;

  /*连接到服务器*/
  if ( iflag == 0 )         /*后续包的第一个包*/
  {
    *iSockfd = swConnectser( sgTcpcfg.aPartner_addr, sgTcpcfg.iPartner_port );
    if (*iSockfd < 0) 
    {
      *iSockfd=swCheck_comm();
      if ( *iSockfd < 0 )
      {
        swVdebug(1,"S0520: [错误/函数调用] swChek_comm() errno=%d[%s]",errno,strerror(errno));
        return(-1);
      }
    }
    swVdebug(2,"S0530: 连接到服务器成功");  
    ilAddrlen = sizeof(struct sockaddr_in);
    memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
 
    ilRc = getsockname(*iSockfd,(struct sockaddr*)&slCli_addr, \
      &ilAddrlen);
    if ( ilRc == -1 )
    {  
      swVdebug(1,"S0540: [错误/系统调用] getsockname()函数,read socket errno=%d[%s]",errno,strerror(errno));
      return(-1);
    }
    swVdebug(2,"S0550: getsockname SUCCESS");

    slLinger.l_onoff = 1 ;
    slLinger.l_linger = 1 ;
  
    ilRc = setsockopt(*iSockfd,SOL_SOCKET,SO_LINGER,&slLinger, 
    sizeof(struct linger));
    if ( ilRc ) 
    {
      swVdebug(0,"S0560: [错误/系统调用] setsockopt()函数,Connected errno=%d[%s]",errno, strerror(errno));
      return(-1); 
    }

    if ( cgDebug >= 1 )
    {
      swVdebug(2,"S0570: 连接到服务器{%s}的端口[%d]成功", \
        sgTcpcfg.aPartner_addr,sgTcpcfg.iPartner_port);
    }
  }
           
  /* 报文发送到端口 */
  swVdebug(2,"S0580: 发送报文到服务器...");
     
  /*判断有无报文头*/
  if(sgComcfg.iMsghead == 0)
  {
    
    /* 不带报文头往外发 */
    ilRc = swTcpsnd(*iSockfd,aMsgbuf + sizeof(struct msghead),
      iMsglen - sizeof(struct msghead));
  }
  else
  {
    ilRc = swTcpsnd(*iSockfd,aMsgbuf,iMsglen);
  }
  if ( ilRc ) 
  {
    swVdebug(1,"S0590: [错误/函数调用] swTcpsend()函数,发送报文到服务器出错,errno=%d[%s]",errno,strerror(errno));
    return(-1);
  }

  swVdebug(2,"S0600: 发送长度为[%d]报文到服务器成功!",iMsglen);
  swVdebug(4,"S0610: [函数返回] swconnsend()返回码=0");
  return(0);
}
