/****************************************************************/
/* 模块编号    ：BMQCLS                                         */
/* 模块名称    ：服务器侦听模块                                 */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：史正烨                                         */
/* 建立日期    ：2000/10/26                                     */
/* 最后修改日期：2001/08/08                                     */
/* 模块用途    ：新晨Q Client/Server方式下服务器端侦听          */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共529行                              */
/* 2006.11.23 徐军,增加对WINCLI的收发文件支持,程序共945行       */
/****************************************************************/

static int _bmqClsprocess(  );
static void  _bmqTimeout();
/*库函数定义*/
#include "bmq.h"

static struct	sockaddr_in sgSvr_addr;
static struct	sockaddr_in sgCli_addr;
static int	igSockfd;
static short	igFlag;
static short	igTimeout;
static short	igProctype;
int    ilSrvBakFlag;
int    ilPid;

static int _bmqDenyCli(char *aCliAddr);
extern int _bmqGetfield();
/*extern int bmqPutFile();*/
extern int bmqGetfileX();
extern int bmqSendFile();
extern int bmqRecvFile();
char template[ ]="template-XXXXXX";
int main(int argc,char **argv)
{
  int    ilRc,ilLen;
  int    ilSockfd;
  char   alInitfile[101];
  char   alResult[10];
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
  int  ilOpt;
  short ilMaxlinknum;

  /*显示版本信息*/
  _bmqShowversion(argc,argv);

  /*设置信号*/
  _bmqSignalinit();

  /* 创建守护进程 */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010 bmqcls:创建守护进程失败，退出!");
    exit(FAIL);
  }
 
  /*读入参数配置文件DEBUGFLAG,C/S PORT*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  { 
    _bmqDebug("S0020 %s :载入DEBUG参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }
  ilRc = _bmqConfig_load("CLS",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0030 %s :载入C/S参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }
  
  /* add by xujun 20061117 next 10 lines
     文件收走后是否在服务器保留备份标志 */
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"WINSRVBAKFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 MBPROCTYPE 字段失败!",alInitfile);
    alResult[0] = '0';
  }
  ilSrvBakFlag = atoi(alResult);
  
  /*从配置文件中对读取c/s最大连接数*/
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"MAXLINKNUM",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 MAXLINKNUM 字段失败!",alInitfile);
    alResult[0] = '9';
  }
  ilMaxlinknum = atoi(alResult);

  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0040 %s:连接共享内存区出错:%d",__FILE__,ilRc);
    exit(FAIL);
  }

  memset(&sgSvr_addr,0x00,sizeof(struct sockaddr_in));
  memset(&sgCli_addr,0x00,sizeof(struct sockaddr_in));
  
  sgSvr_addr.sin_family = AF_INET;
  sgSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sgSvr_addr.sin_port = htons(igBmqclsport);

  /*创建SOCKET*/
  ilSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (ilSockfd == -1)
  {
    _bmqDebug("S0050 %s:创建SOCKET错误! errno:%d,%s",__FILE__,errno,strerror(errno));
    exit(FAIL);
  }
 
  ilOpt = 1;
  ilLen = sizeof(ilOpt);
  setsockopt(ilSockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&ilOpt,ilLen);

  /*绑定SOCKET端口*/
  if (bind(ilSockfd,(struct sockaddr*)&sgSvr_addr,sizeof(struct sockaddr_in))
    == -1)
  {
    _bmqDebug("S0060 绑定SOCKET端口[%d]出错,errno:%d,%s",igBmqclsport,errno,strerror(errno));
    close( ilSockfd );
    exit(FAIL);
  }

  if (listen(ilSockfd, SOMAXCONN) == -1)
  {
    _bmqDebug("S0070 %s:侦听连接出错,errno:%d,%s",__FILE__,errno,strerror(errno)); 
    close (ilSockfd);
    exit(FAIL);
  }

  /*保存进程pid*/
  psgMbshm->lBmqcls_pid = getpid();

  _bmqDebug("S0080 ***Client/Server 守护进程 listen***");

  for( ; ; )
  {
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(ilSockfd,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      _bmqDebug("S0090 %s:准备连接出错,errno:%d,%s",__FILE__,errno,strerror(errno)); 
      return (-1010); 
    }
    
    /* 判断客户端是否被定义为允许连接 */
    if( _bmqDenyCli(inet_ntoa(sgCli_addr.sin_addr)) )
    {
      _bmqDebug("S0092: 客户端[%s]未被定义允许连接!",inet_ntoa(sgCli_addr.sin_addr));
      close ( igSockfd );
      continue;
    }

    /*最大socket连接数判断*/
    if (psgMbshm->iCLSConnectNum > ilMaxlinknum)
    {
      _bmqDebug("S0091: socket连接数超过最大连接数");
      close ( igSockfd );
      continue;
    }        
    else
    {
      switch ( fork() )
      {
        case -1:
          close( ilSockfd );
          close( igSockfd );  
          exit( -1 );
        case 0:	
          close( ilSockfd );
          /*修改socket连接计数器psgMbshm->iCLSConnectNum. add by qh 20070110*/
          ilPid = getpid();
          if(_bmqLock(LK_ALL_2) != 0)
          {
            _bmqDebug("S0092: 申请信号灯失败!");
            exit(0);
          }
          psgMbshm->iCLSConnectNum++;
          _bmqUnlock(LK_ALL_2);

          if ( _bmqClsprocess() != 0 )
          {
            _bmqDebug("S0100 %s:子进程处理出错!",__FILE__);  
            bmqClose();
            close ( igSockfd );
          }
          _bmqShmConnect();
      
          /*修改socket连接计数器psgMbshm->iCLSConnectNum. add by qh 20070110*/
          if(_bmqLock(LK_ALL_2) != 0)
          {
            _bmqDebug("S0101: 申请信号灯失败!");
            exit(0);
          }         
          psgMbshm->iCLSConnectNum--;
          _bmqUnlock(LK_ALL_2);
  	  exit(0);
        default:
          close( igSockfd );
          break;
      }
    }
  }
}

/**************************************************************
 ** 函数名: _bmqParseAddr
 ** 功能:   解析IP地址串
 ** 作者:
 ** 建立日期: 2007/01/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0--成功，-1--失败
***************************************************************/
int _bmqParseAddr(char *addr,int *d1,int *d2,int *d3,int *d4)
{
  int   i,len;
  char  alTmp[16];
  
  len = strlen(addr);
  if( (len > 15) || (len < 7) )
  {
    _bmqDebug("S0102: IP地址(%s)长度(%d)非法!",addr,len);
    return(FAIL);
  }
  
  memset(alTmp,0x20,sizeof(alTmp));
  
  for(i=0;i<len;i++)
  {
    if( (isdigit(addr[i]) == 0 ) && addr[i] != '.' )
    {
      _bmqDebug("S0103: IP地址含非法字符[%c]!",addr[i]);
      return(FAIL);
    }
    if(addr[i] != '.')
      alTmp[i] = addr[i];
  }
  
  sscanf(alTmp,"%d %d %d %d",d1,d2,d3,d4);
  if( *d1>255  || *d2>255  || *d3>255  || *d4>255 )
  {
    _bmqDebug("S0104: IP地址越界!",addr[i]);
    return(FAIL);
  }
  
  return(SUCCESS); 
}

/**************************************************************
 ** 函数名: _bmqAllowCli
 ** 功能:   判断客户端是否被定义允许连接
 ** 作者:
 ** 建立日期: 2007/01/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0--成功，-1--失败
***************************************************************/
static int _bmqAllowCli(char *aCliAddr)
{
  int       ilRc;
  int       c1,c2,c3,c4;
  int       d1,d2,d3,d4;
  FILE      *fp;
  char      alFile[81];
  char      alBuf[80],alBuf1[80];
  char      alType[40],alResult[40];
  
  ilRc = _bmqParseAddr(aCliAddr,&c1,&c2,&c3,&c4);
  if(ilRc) return(FAIL);
  
  sprintf(alFile,"%s/etc/bmq.ini",getenv("BMQ_PATH"));
  if ((fp = fopen(alFile, "r")) == NULL) return(FAIL);

  while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
  {
    _bmqTrim(alBuf);
    if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
    if (memcmp(alBuf,"BEGIN",5) != 0 ) continue;
  
    sscanf(alBuf,"%s %s", alResult,alType);
    if(strcmp(alType,"ALLOWCLI"))  continue;
    
    while( fgets(alBuf1, sizeof(alBuf1), fp) != NULL)
    {
      _bmqTrim(alBuf1);
      if (strlen(alBuf1) == 0 || alBuf1[0] == '#') continue;
      if ( memcmp( alBuf1,"END",3) == 0 )
      {
        fclose(fp);
        return(FAIL);
      }
      /*设置了允许名单*/
      sscanf(alBuf1,"%s",alResult);
      _bmqTrim(alResult);
      ilRc = _bmqParseAddr(alResult,&d1,&d2,&d3,&d4);
      if(ilRc) return(FAIL);
      /*设置了全允许0.0.0.0*/
      if((d1+d2+d3+d4)==0)
      {
         fclose(fp);
         return(SUCCESS);            	
      }
      /*设置了部分允许*/
      else
      {
        if( (!d1 || d1==c1) && (!d2 || d2==c2) && (!d3 || d3==c3) && (!d4 || d4==c4) )
        {
          fclose(fp);
          return(SUCCESS);
        }
      }             
    }
  }
  
  fclose(fp);
  return(FAIL);
}

/**************************************************************
 ** 函数名: _bmqDenyCli
 ** 功能:   判断客户端是否被拒绝连接
 ** 作者:
 ** 建立日期: 2007/01/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0--成功，-1--失败
***************************************************************/
static int _bmqDenyCli(char *aCliAddr)
{
  int       ilRc;
  int       c1,c2,c3,c4;
  int       d1,d2,d3,d4;
  FILE      *fp;
  char      alFile[81];
  char      alBuf[80],alBuf1[80];
  char      alType[40],alResult[40];
  
  /* 拒绝策略说明:首先判断时侯设置了拒绝名单－－
  　　　　　　　　　　如无，则表示全部允许
  　　　　　　　　　　如果拒绝中设置为全拒绝，则只有在允许名单中才允许通过
  　　　　　　　　　　如果设置了部分拒绝，则只有在拒绝名单中才拒绝，其他全允许通过
  */
  ilRc = _bmqParseAddr(aCliAddr,&c1,&c2,&c3,&c4);
  if(ilRc) return(FAIL);
  
  sprintf(alFile,"%s/etc/bmq.ini",getenv("BMQ_PATH"));
  if ((fp = fopen(alFile, "r")) == NULL) return(FAIL);

  while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
  {
    _bmqTrim(alBuf);
    if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
    if (memcmp(alBuf,"BEGIN",5) != 0 ) continue;
  
    sscanf(alBuf,"%s %s", alResult,alType);
    if(strcmp(alType,"DENYCLI"))  continue;
    
    while( fgets(alBuf1, sizeof(alBuf1), fp) != NULL)
    {
      _bmqTrim(alBuf1);
      if (strlen(alBuf1) == 0 || alBuf1[0] == '#') continue;
      if ( memcmp( alBuf1,"END",3) == 0 )
      {
        fclose(fp);
        return(SUCCESS);
      }
      /*设置了拒绝名单*/
      sscanf(alBuf1,"%s",alResult);
      _bmqTrim(alResult);
      ilRc = _bmqParseAddr(alResult,&d1,&d2,&d3,&d4);
      if(ilRc) return(FAIL);
      /*设置了全拒绝0.0.0.0*/
      if((d1+d2+d3+d4)==0)
      {
         ilRc = _bmqAllowCli(aCliAddr);
         {
           fclose(fp);
           return(ilRc);            	
         }
      }
      /*设置了部分拒绝*/
      else
      {
        if( (!d1 || d1==c1) && (!d2 || d2==c2) && (!d3 || d3==c3) && (!d4 || d4==c4) )
        {
          fclose(fp);
          return(FAIL);
        }            
      }             
    }
  }

  fclose(fp);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _comTcpSend
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
/*static int _comTcpSend(sockfd, buffer, length)
 int             sockfd;
 unsigned char   *buffer;
 short           *length;  delete by wh*/
static int _comTcpSend(sockfd, buffer, length)
 int             sockfd;
 unsigned char   *buffer;
 TYPE_SWITCH *length; /*add by wh*/
{
  /*short   len=-1;
  short   ilSendlen=0; delete by wh*/
  TYPE_SWITCH len=-1; /*add by wh*/
  TYPE_SWITCH ilSendlen=0; /*add by wh*/
  while( ilSendlen != *length )
  {
    if ( ( len = send(sockfd,buffer+ilSendlen,*length-ilSendlen,0)) > 0 )
    {
      ilSendlen += len;
    }
    else
    {
      _bmqDebug("S0111: _comTcpSend error! errno:%d,%s",errno,strerror(errno));
      return(-1031);
    }
  }
  *length = ilSendlen;
  return ( SUCCESS );
}

/**************************************************************
 ** 函数名: _comTcpReceive
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
/*static int _comTcpReceive( sockfd, buffer, length)
  int     sockfd;
  unsigned char *buffer;
  short   *length; 
  delete by wh*/
static int _comTcpReceive( sockfd, buffer, length)
  int     sockfd;
  unsigned char *buffer;
  TYPE_SWITCH *length; /*add by wh*/
{
  int ilRn = -1040;
  /*short ilRcvlen=0; delete by wh*/
  TYPE_SWITCH ilRcvlen=0; /*add by wh*/

  memset (buffer, 0x00, *length);

  if ( ( ilRcvlen=recv(sockfd,buffer,*length,MSG_EOR)) > 0 )
  {
     *length = ilRcvlen;
       ilRn  = 0;
  }
  /* next 5 rows add by nh 2002/7/4 */
  else if(ilRcvlen == 0)
  {
    ilRn = 1040;
    if(igDebug >= 1)
      _bmqDebug("S0085: _comTcpReceive() error=[%d][%s]",errno,strerror(errno));
  }
  else
    _bmqDebug("S0090: _comTcpReceive error! errno:%d,%s",errno,strerror(errno));

  return ( ilRn );
}

/**************************************************************
 ** 函数名: _bmqRecvconf
 ** 功能:   接收确认数据
 ** 作者:   徐军
 ** 建立日期: 2002/07/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:sockfd--套接字描述符 buffer--存储信息的地址
             length--缓存区的最大尺寸
 ** 返回值: 0--成功，-1041--失败
***************************************************************/
/*static int _bmqRecvconf(sockfd,buffer,length)
  int  sockfd;
  char *buffer;
  short   *length;
  delete by wh*/
static int _bmqRecvconf(sockfd,buffer,length)
  int  sockfd;
  char *buffer;
  TYPE_SWITCH *length;  /*add by wh*/
{
  int   ilRc     = -1041;
  /*short ilRcvlen = 0; delete by wh*/
  TYPE_SWITCH ilRcvlen = 0; /*add by wh*/
  int   ilLen; 

  /*设置阻塞时间*/
  if (igTimeout != 0)
  {
    signal(SIGALRM,_bmqTimeout);
    alarm(igTimeout);
  }
  igFlag = 0;


  ilLen = *length;
  for(;;)
  {
    ilRcvlen=recv(sockfd,buffer,ilLen,MSG_EOR);
    if(igFlag) return -1;
    if(ilRcvlen>0)
    {
      if(ilRcvlen != ilLen)
      {
        ilLen -= ilRcvlen;
        buffer = buffer + ilRcvlen;
        continue;
      }
      else
      {
        ilRc = 0;
        break; 
      }
    }
    else
    {
      _bmqDebug("S0130: _bmqRecvconf[%d]错误:%d,%s",ilRcvlen,errno,strerror(errno));
      ilRc = -1041;
      break;
    }
  }

  if(igTimeout != 0)
  {
    alarm(0);
    signal(SIGALRM,SIG_DFL);
  }

  if(ilRc) return ilRc;

  if(memcmp(buffer,"AAAA",4)==0)
    ilRc = 0;
  else
    ilRc = 1014;

  /* send char 'A' for respon */
  send(sockfd,buffer,1,0);
  return ilRc;
}

static void _bmqTimeout()
{
  igFlag = 1;
  alarm(0);
  signal(SIGALRM,SIG_DFL);
}

/**************************************************************
 ** 函数名: _bmqClsprocess
 ** 功能:   处理客户端请求
 ** 作者:   史正烨
 ** 建立日期: 2000/10/26
 ** 最后修改日期:2001/08/08
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqClsprocess(  )
{
  struct hostent *hp;
  struct linger ilLinger;
  struct mbinfo slMbinfo;
  
  char   alCliname[80];
  char   alMsgbuf[iMBMAXPACKSIZE+100],alVal[iMASKLEN+1];
  int    ilRc,ilLen,ilLen1;
  int    ilVirFlag;
  short  ilPrior,ilMbid;
  short  ilReputflag = 0;
  long   llType,llClass,llFileLen;
  long   llMsglen,llLen;
  char   alFileBuf[iMBMAXPACKSIZE];
  char   alMsgbufSaf[iMBMAXPACKSIZE];
  char   alTmpFileName[100];
  FILE   *fp;
  
  short  ilGrpid,ilQid,/*ilMsglen,ilMsglen1,*/ilTimeout; /*delete by wh*/
  TYPE_SWITCH ilMsglen, ilMsglen1;  /*add by wh*/
  char   alFilter[iMASKLEN],alMask[iMASKLEN];

  char   alTmp[5];
  char   alMac[MAC_LEN];
  char   alOrgBuf[iMBMAXPACKSIZE];
  char   alFile[81],alFileName[81];
  short  ilTmpmb,ilTmpgrp,ilTmplen;
  
  char   alMsgTopic[41];
  char   alSendFile[120],alVirName[100];
  short  i,ilFilenum;
  char   alFileList[iMBMAXPACKSIZE];
  char   alCommand[200];
  char   alPath[100];

  memset(alVal,0x00,sizeof(alVal));

  /*获得客户端主机名*/
  hp = gethostbyaddr( (char *) &sgCli_addr.sin_addr, sizeof(struct in_addr),
    sgCli_addr.sin_family);
  if( hp == NULL)
    strcpy( alCliname, inet_ntoa( sgCli_addr.sin_addr ) );
  else
    strcpy( alCliname, hp->h_name );

  ilLinger.l_onoff  =1;
  ilLinger.l_linger =1;
  /*设置 SOCKET 配置 */
  ilRc=setsockopt(igSockfd,SOL_SOCKET,SO_LINGER,&ilLinger,
    sizeof(struct linger));
  if( ilRc == FAIL )
  {
    _bmqDebug("S0110 {%s}Unable to set socket option errno:%d,%s",alCliname,errno,strerror(errno));
    return(-1081);
  }
  for(;;)
  {
    /*从客户端接收报文*/
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;

    /*先收8字节报文长度*/
    ilRc = _comTcpReceive(igSockfd, alMsgbuf, &ilMsglen) ; 
    /* next 7 rows add by nh 2002/7/4 */
    if(ilRc == 1040)
    {
      if( igDebug >= 1)
        _bmqDebug("S0116 与{%s}TCP通讯链路已断开!",alCliname);
      close ( igSockfd );
      return(SUCCESS);
    }
    
    else if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0120 从客户端{%s}收报文出错!-Retcode=%d", alCliname, ilRc);
      return (ilRc);
    }

    memcpy(alVal,alMsgbuf,8);
    alVal[8] = '\0';
    ilLen = atoi(alVal);
    ilLen1 = ilMsglen1 =0;
    /*接收指定长度的报文*/
    while( ilMsglen1 < ilLen )
    {
      ilMsglen = ilLen - 8 - ilLen1;
      ilRc = _comTcpReceive(igSockfd, alMsgbuf+8+ilLen1, &ilMsglen) ;
      if ( ilRc != 0 || ilMsglen <= 0)
      {
        _bmqDebug("S0130 从客户端{%s}收报文出错!-Retcode=%d", alCliname, ilRc);
        return (ilRc);
      }
      ilLen1 += ilMsglen;
      ilMsglen1 = ilLen1 + 8;
    }

    if(igDebug >= 1)
    {
      _bmqDebug("S0140 从客户端{%s}接收报文完毕!!--ilMsglen=%d order=%d", alCliname, ilMsglen + 8 ,alMsgbuf[8]);
      if(igDebug >= 3)
        _bmqDebughex(alMsgbuf,ilMsglen + 8);
    }

    memset(alVal,0x00,sizeof(alVal));
    ilReputflag = 0;
    switch (alMsgbuf[8])
    {
      case 1: /* bmqOpen() */ 
        memcpy(alVal,alMsgbuf+9,5);
        ilQid  = atoi(alVal);
        memcpy(alVal,alMsgbuf+14,5);
        igTimeout  = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);
        igProctype  = atoi(alVal);
        ilRc   = bmqOpen(ilQid);
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        ilMsglen = 6;
        break;

      case 2: /* bmqClose() */
        ilRc = bmqClose();
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        ilMsglen = 6;
        break;

      case 3: /* bmqGet() */
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memcpy(alVal,alMsgbuf+14,5);
        ilMbid = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);
        ilPrior = atoi(alVal);
        memcpy(alFilter,alMsgbuf+24,iMASKLEN);
        memcpy(alMask,alMsgbuf+24+iMASKLEN,iMASKLEN);
        memset(alMask,0xff,iMASKLEN);
        
        ilMsglen = sizeof( alMsgbuf );
        ilRc = bmqGetfilter(&ilGrpid,&ilMbid,&ilPrior,alFilter,alMask,
          alMsgbuf+29+iMASKLEN+iMASKLEN,&ilMsglen);
        ilTmplen = ilMsglen;
        if (ilRc)
          ilMsglen = 29+iMASKLEN+iMASKLEN;
        else
          ilMsglen = ilMsglen + (29+iMASKLEN+iMASKLEN) + MAC_LEN;
        if(igProctype == 1) ilReputflag = 1; 
        _bmqMac(alMsgbuf+29+iMASKLEN+iMASKLEN,ilTmplen,alMac);
        sprintf(alVal,"%08d",ilMsglen);
        memcpy(alMsgbuf,alVal,8);
        alMsgbuf[8] = '0';
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+9,alVal,5);
        sprintf(alVal,"%05d",ilGrpid);
        memcpy(alMsgbuf+14,alVal,5);
        sprintf(alVal,"%05d",ilMbid);
        memcpy(alMsgbuf+19,alVal,5);
        sprintf(alVal,"%05d",ilPrior);
        memcpy(alMsgbuf+24,alVal,5);
        memcpy(alMsgbuf+29,alFilter,iMASKLEN);
        memcpy(alMsgbuf+29+iMASKLEN,alMask,iMASKLEN);
        memcpy(alMsgbuf+29+iMASKLEN+iMASKLEN+ilTmplen,alMac,MAC_LEN);
        break;
        
      case 4: /* bmqGetw() */
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memcpy(alVal,alMsgbuf+14,5);
        ilMbid = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);
        ilPrior = atoi(alVal);
        memcpy(alFilter,alMsgbuf+24,iMASKLEN);
        memcpy(alMask,alMsgbuf+24+iMASKLEN,iMASKLEN);
        memset(alMask,0xff,iMASKLEN);
        memcpy(alVal,alMsgbuf+24+iMASKLEN+iMASKLEN,5);
        ilTimeout = atoi(alVal);
        ilMsglen = sizeof( alMsgbuf );
        
        ilRc = bmqGetwfilter(&ilGrpid,&ilMbid,&ilPrior,alFilter,alMask,
          alMsgbuf+29+iMASKLEN+iMASKLEN,&ilMsglen,ilTimeout);
        ilTmplen = ilMsglen;
        if (ilRc)
          ilMsglen = 29+iMASKLEN+iMASKLEN;
        else
          ilMsglen = ilMsglen + (29+iMASKLEN+iMASKLEN+MAC_LEN);
        if(igProctype == 1) ilReputflag = 1; 
        _bmqMac(alMsgbuf+29+iMASKLEN+iMASKLEN,ilTmplen,alMac);
        sprintf(alVal,"%08d",ilMsglen);
        memcpy(alMsgbuf,alVal,8);
        alMsgbuf[8] = '0';
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+9,alVal,5);
        sprintf(alVal,"%05d",ilGrpid);
        memcpy(alMsgbuf+14,alVal,5);
        sprintf(alVal,"%05d",ilMbid);
        memcpy(alMsgbuf+19,alVal,5);
        sprintf(alVal,"%05d",ilPrior);
        memcpy(alMsgbuf+24,alVal,5);
        memcpy(alMsgbuf+29,alFilter,iMASKLEN);
        memcpy(alMsgbuf+29+iMASKLEN,alMask,iMASKLEN);
        memcpy(alMsgbuf+29+iMASKLEN+iMASKLEN+ilTmplen,alMac,MAC_LEN);
        break;
        
      case 5: /* bmqPut() */
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memcpy(alVal,alMsgbuf+14,5);
        ilMbid = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);
        ilPrior = atoi(alVal);
        memcpy(alFilter,alMsgbuf+24,iMASKLEN);
        memset(alMac,0x00,sizeof(alMac));
        memcpy(alMac,alMsgbuf+ilLen-MAC_LEN,MAC_LEN);
        if(memcmp(alMac,"00000000",MAC_LEN))
        {
          memset(alMac,0x00,sizeof(alMac));
          memset(alOrgBuf,0x00,sizeof(alOrgBuf));
          memcpy(alOrgBuf,alMsgbuf+(24+iMASKLEN),ilLen-(24+iMASKLEN+MAC_LEN));
          _bmqMac(alOrgBuf,ilLen-(24+iMASKLEN+MAC_LEN),alMac);
          if(memcmp(alMac,alMsgbuf+ilLen-MAC_LEN,MAC_LEN))
          {
            /* 报文校验失败，直接返回错误信息 */
            memcpy(alMsgbuf+1,"01015",5);
            ilMsglen = 6;
            break;
          }
        }
        ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter,
          alMsgbuf+(24+iMASKLEN),ilLen-(24+iMASKLEN+MAC_LEN));
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        ilMsglen = 6;
        break;
      case 6:/*bmqClearmb()*/
        memcpy(alVal,alMsgbuf+9,5);
        ilQid = atoi(alVal);
        ilRc = bmqClearmb(ilQid);
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        ilMsglen = 6;
        break;
      case 7:/*bmqGetmbinfo()*/
        memcpy(alVal,alMsgbuf+9,5);
        ilQid = atoi(alVal);
        memset(&slMbinfo,0x00,sizeof(struct mbinfo));
        ilRc = bmqGetmbinfo(ilQid,&slMbinfo);
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        memcpy(alMsgbuf+6,&slMbinfo,sizeof(struct mbinfo));
        ilMsglen = 6 + sizeof(struct mbinfo);
        break;
      /* add by xujun 2006.11.15 for file transform begin */
      case 8: /* bmqPutFile() */
        /*先读取文件传送命令报文*/
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memcpy(alVal,alMsgbuf+14,5);
        ilMbid = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);
        ilPrior = atoi(alVal);
        memcpy(alVal,alMsgbuf+24,10);
        llType = atol(alVal);
        memset(alFileName,0x00,sizeof(alFileName));
        memcpy(alFileName,alMsgbuf+34,80);
        _bmqTrim(alFileName);
        memcpy(alVal,alMsgbuf+114,16);
        llFileLen = atol(alVal);
               
        /*命令报文结束,开始读取指定长度(llFileLen)的文件数据*/
        /*生成临时文件*/
        memset(alFile,0x00,sizeof(alFile));
        /*去除文件路径*/
        _bmqfilechr(alFileName,80,alFile,'\\');
        memset(alTmpFileName,0x00,sizeof(alTmpFileName));
        sprintf(alTmpFileName,"%s/temp/%s",getenv("BMQ_FILE_DIR"),alFile);
        if (( fp = fopen(alTmpFileName,"w+")) == NULL)
        {
          _bmqDebug("S0200: 不能创建临时文件[%s] errno:%d:%s",alTmpFileName,errno,strerror(errno));
          return( ilRc );
        }  
        llMsglen = llLen = 0;
         
        while( llMsglen < llFileLen )
        {
          if( (llFileLen - llMsglen) >= 1024 )
            ilMsglen =  1024;
          else  
            ilMsglen = llFileLen - llMsglen;
          memset(alFileBuf,0x00,sizeof(alFileBuf));
          ilRc = _comTcpReceive(igSockfd, alFileBuf, &ilMsglen) ;
          if ( ilRc != 0 || ilMsglen <= 0)
          {
            _bmqDebug("S0210: 从客户端{%s}收文件(%s)流出错!-Retcode=%d", alCliname, alFileName,ilRc);
            fclose(fp);
            return( ilRc );
          }
          /*文件流写入临时文件*/
          fwrite(alFileBuf,sizeof(char),ilMsglen,fp);
          llMsglen += ilMsglen;
        } 
        fclose(fp);
        /*调用文件发送的API将临时文件发送到目标组*/ 
        ilRc = bmqPutFile(ilGrpid,ilMbid,ilPrior,llType,alMsgbuf+130,ilLen-130,alTmpFileName);
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        ilMsglen = 6;
        /*删除SRV的临时文件*/
        unlink(alTmpFileName);
        break;
      case 9: /* bmqGetFile() */
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memcpy(alVal,alMsgbuf+14,5);
        ilMbid = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);
        ilPrior = atoi(alVal);
        memcpy(alVal,alMsgbuf+24,10);
        llType = atol(alVal);
        memcpy(alVal,alMsgbuf+34,10);
        llClass = atol(alVal);
        memset(alVal,0x00,sizeof(alVal));
        memcpy(alVal,alMsgbuf+44,5);
        ilTimeout = atoi(alVal);
        ilMsglen = sizeof( alMsgbuf );

        ilRc = bmqGetfileX(&ilGrpid,&ilMbid,&ilPrior,&llType,&llClass,alMsgbuf+49+80+16,&ilMsglen,ilTimeout,alFileName);
        ilTmplen = ilMsglen;
        memset(alMsgbufSaf,0x00,sizeof(alMsgbufSaf));
        memcpy(alMsgbufSaf,alMsgbuf+49+80+16,ilMsglen);
        if (ilRc)
          ilMsglen = 49+80+16;
        else
          ilMsglen = ilMsglen + 49 + 80 + 16;
        /*if(igProctype == 1) ilReputflag = 1; */
        ilReputflag = 1; 
        sprintf(alVal,"%08d",ilMsglen);
        memcpy(alMsgbuf,alVal,8);
        alMsgbuf[8] = '9';
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+9,alVal,5);
        sprintf(alVal,"%05d",ilGrpid);
        memcpy(alMsgbuf+14,alVal,5);
        sprintf(alVal,"%05d",ilMbid);
        memcpy(alMsgbuf+19,alVal,5);
        sprintf(alVal,"%05d",ilPrior);
        memcpy(alMsgbuf+24,alVal,5);
        sprintf(alVal,"%010ld",llType);
        memcpy(alMsgbuf+29,alVal,10);
        sprintf(alVal,"%010ld",llClass);
        memcpy(alMsgbuf+39,alVal,10);
        memset(alTmpFileName,0x00,sizeof(alTmpFileName));
        /*去除文件路径*/
        if(ilMsglen > (49+80+16))
        {
          _bmqfilechr(alFileName,80,alTmpFileName,'/');
          memcpy(alMsgbuf+49,alTmpFileName,strlen(alTmpFileName));
          if (( fp = fopen(alFileName,"r")) == NULL)
          {
            _bmqDebug("S0220: 不能打开文件[%s] errno:%d:%s",alFileName,errno,strerror(errno));
            llFileLen = 0;
          }
          else
          {
            fseek(fp, 0, SEEK_END);
            llFileLen = ftell(fp);
          }
          sprintf(alVal,"%016ld",llFileLen);
          memcpy(alMsgbuf+129,alVal,16);
        }
        /* 发送报文到服务器 */
        ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
        if ( ilRc != 0 )
        {
          _bmqDebug("S0230: 送回客户端出错!!--Retcode = %d",ilRc );
          fclose(fp);
          goto REPUT;
        }
        if(ilMsglen == (49+80+16))
        {
           _bmqDebug("S0232:  bmqGetfileX执行失败");
           continue;
        }
        
        /*将文件以流方式发送到SRV*/
        if(llFileLen)
        { 
          fseek(fp,0,SEEK_SET);
          /* 发送文件到服务器 */
          while(!feof(fp))
          {
            memset(alMsgbuf,0x00,sizeof(alMsgbuf));
            ilMsglen = fread(alMsgbuf,sizeof(char),1024,fp);
            ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
            if(ilRc)
            {
              _bmqDebug("S0240: 文件流送回客户端出错!!--Retcode = %d",ilRc );
              fclose(fp);
              goto REPUT;
            }
          }
        }

        ilMsglen = 4;
        ilRc = _bmqRecvconf(igSockfd,alTmp,&ilMsglen);
        if ( ilRc )
        {
          if (ilRc == 1014) 
            _bmqDebug("S0250: 读到的不是合法确认信息,buf=[%s]!",alTmp);
          else if (ilRc == 1016)
            _bmqDebug("S0260: 读确认信息超时!");
          else
            _bmqDebug("S0270: Socket read error[%d]",ilRc);
        REPUT:
          /* 报文回写 */
          if (igProctype == 1)
          {
            ilTmpmb = igMb_sys_current_mailbox;
            ilTmpgrp = igMb_sys_current_group;
            igMb_sys_current_group = ilGrpid;
            igMb_sys_current_mailbox = ilMbid;
      	    ilRc = bmqPut(ilTmpgrp,ilTmpmb,ilPrior,llType,llClass,
                                alMsgbufSaf,ilTmplen);
      	    if(ilRc)
      	    {
              _bmqDebug("S0280: 报文[%d][%d][%d][%c~%c]回写FAIL!",
                          ilTmpgrp,ilTmpmb,ilPrior,alFilter,
                          alMsgbuf[29+iMASKLEN*2],
                          alMsgbuf[28+iMASKLEN*2+ilTmplen]);
      	    }
            else
            {
              if( igDebug >= 2 )
              {
                _bmqDebug("S0290: 报文[%d][%d][%d][%s][%c~%c]回写成功!",
                            ilGrpid,ilMbid,ilPrior,alFilter,
                            alMsgbuf[29+iMASKLEN*2],
                            alMsgbuf[28+iMASKLEN*2+ilTmplen]);
              }
            }
            igMb_sys_current_mailbox = ilTmpmb;
            igMb_sys_current_group   = ilTmpgrp;
          }
          return (ilRc);
        }
        /*收到CLI确认应答,文件接收完成,删除SRV的临时文件*/
        if( ilGrpid == 0 ) ilGrpid = psgMbshm->iMbgrpid;
        if( (psgMbshm->iMbgrpid != ilGrpid) && (ilSrvBakFlag==0) )
          unlink(alFileName);   
        continue;
      case 'A': /* bmqSendFile() */
        /*先读取文件传送命令报文*/
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memset(alFileName,0x00,sizeof(alFileName));
        memcpy(alFileName,alMsgbuf+14,80);
        _bmqTrim(alFileName);
        memcpy(alVal,alMsgbuf+94,16);
        llFileLen = atol(alVal);
        /*命令报文结束,开始读取指定长度(llFileLen)的文件数据*/
        /*生成临时文件*/
        memset(alTmpFileName,0x00,sizeof(alTmpFileName));
        sprintf(alTmpFileName,"%s/temp/%s",getenv("BMQ_FILE_DIR"),alFileName);
        if (( fp = fopen(alTmpFileName,"w+")) == NULL)
        {
          _bmqDebug("S0300: 不能创建临时文件[%s] errno:%d:%s",alTmpFileName,errno,strerror(errno));
          return( ilRc );
        }  
        llMsglen = llLen = 0;
        while( llMsglen < llFileLen )
        {
          if( (llFileLen - llMsglen) >= 1024 )
            ilMsglen =  1024;
          else  
            ilMsglen = llFileLen - llMsglen;
          memset(alFileBuf,0x00,sizeof(alFileBuf));
          ilRc = _comTcpReceive(igSockfd, alFileBuf, &ilMsglen) ;
          if ( ilRc != 0 || ilMsglen <= 0)
          {
            _bmqDebug("S0310: 从客户端{%s}收文件(%s)流出错!-Retcode=%d", alCliname, alFileName,ilRc);
            fclose(fp);
            return( ilRc );
          }
          /*文件流写入临时文件*/
          fwrite(alFileBuf,sizeof(char),ilMsglen,fp);
          llMsglen += ilMsglen;
          /*ilMsglen = 1;
          ilRc = _comTcpReceive( igSockfd,'0',&ilMsglen);
          */
        } 
        fclose(fp);
       
        /*调用文件发送的API将临时文件发送到目标组*/ 
        ilRc = bmqSendFile(ilGrpid,alTmpFileName);
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        ilMsglen = 6;
        /*删除SRV的临时文件*/
        unlink(alTmpFileName);
        break;
      case 'B': /* bmqRecvFile() */
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memset(alFileName,0x00,sizeof(alFileName));
        memcpy(alFileName,alMsgbuf+14,80);
        _bmqTrim(alFileName);
        ilRc = bmqRecvFile(ilGrpid,alFileName);
        ilMsglen = 30;
        sprintf(alVal,"%08d",ilMsglen);
        memcpy(alMsgbuf,alVal,8);
        alMsgbuf[8] = 'R';
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+9,alVal,5);
        if (( fp = fopen(alFileName,"r")) == NULL)
        {
          _bmqDebug("S0320: 不能打开文件[%s] errno:%d:%s",alFileName,errno,strerror(errno));
          llFileLen = 0;
        }
        else
        {
          fseek(fp, 0, SEEK_END);
          llFileLen = ftell(fp);
        }
        sprintf(alVal,"%016ld",llFileLen);
        memcpy(alMsgbuf+14,alVal,16);
        /* 发送报文到服务器 */
        ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
        if ( ilRc != 0 )
        {
          _bmqDebug("S0330: 送回客户端出错!!--Retcode = %d",ilRc );
          fclose(fp);
          continue;
        }
        /*将文件以流方式发送到SRV*/
        if(llFileLen)
        { 
          fseek(fp,0,SEEK_SET);
          /* 发送文件到服务器 */
          while(!feof(fp))
          {
            memset(alMsgbuf,0x00,sizeof(alMsgbuf));
            ilMsglen = fread(alMsgbuf,sizeof(char),1024,fp);
            ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
            if(ilRc)
            {
              _bmqDebug("S0340: 文件流送回客户端出错!!--Retcode = %d",ilRc );
              fclose(fp);
              continue;
            }
          }
        }
        
        ilMsglen = 4;
        ilRc = _bmqRecvconf(igSockfd,alTmp,&ilMsglen);
        if ( ilRc )
        {
           _bmqDebug("S0350: 未收到接收方正确收取文件完成之应答");
           return(ilRc);
        }
       
        /*收到CLI确认应答,文件接收完成,如果跨组接收,删除SRV的临时文件*/
        if( ilGrpid == 0 ) ilGrpid = psgMbshm->iMbgrpid;
        if(psgMbshm->iMbgrpid != ilGrpid) 
          unlink(alFileName);   
        continue;
      /* add by xujun 2006.11.15 for file transform end */
      /* add by xujun 2006.12.28 支持多文件传输 begin */
      case 'C': /* 发送多文件 */
        /*先读取多文件传输命令报文*/
        memcpy(alVal,alMsgbuf+9,5);		/*目标组号*/
        ilGrpid = atoi(alVal);		
        memcpy(alVal,alMsgbuf+14,5);	/*目标邮箱*/
        ilMbid = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);	/*邮件优先级*/
        ilPrior = atoi(alVal);
        memcpy(alVal,alMsgbuf+24,10);	/*邮件过滤条件*/
        llType = atol(alVal);
        memset(alMsgTopic,0x00,sizeof(alMsgTopic));
        memcpy(alMsgTopic,alMsgbuf+34,40);	/*消息主题*/
        _bmqTrim(alMsgTopic);
        
        /*add by hxz2007-2-12 ,deal with alMsgTopic is null! begin*/
        if(strlen(alMsgTopic)==0)
        { 
          mkstemp(template);
          strcpy(alMsgTopic,template);
        }
        /*add by hxz2007-2-12 end*/
        
        memset(alVal,0x00,sizeof(alVal));
        memcpy(alVal,alMsgbuf+74,5);	/*附件个数*/
        ilFilenum = atoi(alVal);
        memset(alFileList,0x00,sizeof(alFileList));
        memcpy(alFileList,alMsgbuf+79,96*ilFilenum);	/*附件列表*/
        sprintf(alPath,"%s/temp",getenv("BMQ_FILE_DIR"));
        sprintf(alSendFile,"%s/file/%s",getenv("BMQ_FILE_DIR"),alMsgTopic);                
        fp = fopen(alSendFile,"w+");
        fclose(fp);
        /*开始收取附件*/
        for(i = 0;i < ilFilenum; i++)
        {         
          memset(alFileName,0x00,sizeof(alFileName));
          memcpy(alFileName,alFileList+96*i,80);
          _bmqTrim(alFileName);
          memset(alVal,0x00,sizeof(alVal));
          memcpy(alVal,alFileList+96*i+80,16);
          llFileLen = atol(alVal);
          /*开始读取指定长度(llFileLen)的文件数据*/
          /*生成临时文件*/
          memset(alFile,0x00,sizeof(alFile));
          /*去除文件路径*/
          _bmqfilechr(alFileName,80,alFile,'\\');
          memset(alTmpFileName,0x00,sizeof(alTmpFileName));
          sprintf(alTmpFileName,"%s/temp/%s",getenv("BMQ_FILE_DIR"),alFile);          
          if (( fp = fopen(alTmpFileName,"w+")) == NULL)
          {
            _bmqDebug("S0300: 不能创建临时文件[%s] errno:%d:%s",alTmpFileName,errno,strerror(errno));
            return( ilRc );
          }  
          llMsglen = llLen = 0;
         
          while( llMsglen < llFileLen )
          {
            if( (llFileLen - llMsglen) >= 1024 )
              ilMsglen =  1024;
            else  
              ilMsglen = llFileLen - llMsglen;
            memset(alFileBuf,0x00,sizeof(alFileBuf));
            ilRc = _comTcpReceive(igSockfd, alFileBuf, &ilMsglen) ;
            if ( ilRc != 0 || ilMsglen <= 0)
            {
              _bmqDebug("S0210: 从客户端{%s}收文件(%s)流出错!-Retcode=%d", alCliname, alFileName,ilRc);
              fclose(fp);
              return( ilRc );
            }
            /*文件流写入临时文件*/
            fwrite(alFileBuf,sizeof(char),ilMsglen,fp);
            llMsglen += ilMsglen;
          } 
          fclose(fp);
          /*生产传送的tar包,以消息主题为包名*/
          _bmqDelspace(alSendFile);
          sprintf(alCommand,"bmqfiletar_r %s %s %s",alPath,alSendFile,alFile);
          system(alCommand);
          unlink(alTmpFileName);
        }
        /*调用文件发送的API将临时文件发送到目标组*/
        if(ilFilenum)
          ilRc = bmqPutFile(ilGrpid,ilMbid,ilPrior,llType,alMsgbuf+34,ilLen-34,alSendFile);
        else
          ilRc = bmqPut(ilGrpid,ilMbid,ilPrior,llType,0,alMsgbuf+34,ilLen-34);
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+1,alVal,5);
        ilMsglen = 6;
        /*删除SRV的临时文件*/
        unlink(alSendFile);
        break;    
      case 'D': /* 接收多文件 */
        /*先读取多文件传输命令报文*/
        memcpy(alVal,alMsgbuf+9,5);
        ilGrpid = atoi(alVal);
        memcpy(alVal,alMsgbuf+14,5);
        ilMbid = atoi(alVal);
        memcpy(alVal,alMsgbuf+19,5);
        ilPrior = atoi(alVal);
        memcpy(alVal,alMsgbuf+24,10);
        llType = atol(alVal);
        memcpy(alVal,alMsgbuf+34,10);
        llClass = atol(alVal);
        memset(alVal,0x00,sizeof(alVal));
        memcpy(alVal,alMsgbuf+44,5);
        ilTimeout = atoi(alVal);
        ilMsglen = sizeof( alMsgbuf );        
        memset(alMsgbuf,0x00,sizeof(alMsgbuf));
        ilRc = bmqGetfileX(&ilGrpid,&ilMbid,&ilPrior,&llType,&llClass,alMsgbuf,&ilMsglen,ilTimeout,alFileName);
        ilReputflag = 1; 
        if (ilRc)
        {
          ilMsglen = 14;        
          sprintf(alVal,"%08d",ilMsglen);
          memcpy(alMsgbuf,alVal,8);
          alMsgbuf[8] = 'D';
          sprintf(alVal,"%05d",ilRc);        
          memcpy(alMsgbuf+9,alVal,5);
          _bmqDebug("S2020: bmqGetfileX()调用失败,ilRc=[%d]",ilRc);
          /*送回客户端 */
          ilRc = _comTcpSend ( igSockfd, alMsgbuf, &ilMsglen);
          if ( ilRc != 0 )
          {
            _bmqDebug("S0150 送回客户端出错!!--Retcode = %d",ilRc );
            return (ilRc);
          }
          continue;
        }
        if(ilFilenum)
        {
          /* add by xujun 2007.01.11
             发送前检测病毒(如果邮箱设置了接收检测) */
          if( psgMbinfo[igMb_sys_current_mailbox - 1].iRecvScanFlag )
          {
          	sprintf(alTmpFileName,"%s/temp/%s",getenv("BMQ_FILE_DIR"),alFileName);
            memset(alVirName,0x00,sizeof(alVirName));
            ilVirFlag = 0;
            ilRc = _bmqVirusScanByRoot(alFileName,sgRoot,alVirName);
            if(ilRc)
            {
              _bmqDebug("S3341: 文件[%s]病毒扫描失败!",alFileName); 
              ilVirFlag = 1;             
            }
            else if(strlen(alVirName))
            {
              _bmqDebug("S3342: 文件[%s]检测出病毒[%s]!",alFileName,alVirName);              
              ilVirFlag = 1;
            }
            if(ilVirFlag)
            {
              ilMsglen = 14;      
              ilRc = 1018;  
              sprintf(alVal,"%08d",ilMsglen);
              memcpy(alMsgbuf,alVal,8);
              alMsgbuf[8] = 'D';
              sprintf(alVal,"%05d",ilRc);        
              memcpy(alMsgbuf+9,alVal,5);
              /*送回客户端 */
              ilRc = _comTcpSend ( igSockfd, alMsgbuf, &ilMsglen);
              if ( ilRc != 0 )
              {
                _bmqDebug("S0150 送回客户端出错!!--Retcode = %d",ilRc );
                return (ilRc);
              }
              continue;	
            } 
          }
        }
        
        ilTmplen = ilMsglen;
        memset(alMsgbufSaf,0x00,sizeof(alMsgbufSaf));
        memcpy(alMsgbufSaf,alMsgbuf,ilMsglen);
        
        memset(alMsgTopic,0x00,sizeof(alMsgTopic));
        memcpy(alMsgTopic,alMsgbuf,40);					/*消息主题*/
        memset(alVal,0x00,sizeof(alVal));
        memcpy(alVal,alMsgbuf+40,5);					/*附件个数*/
        ilFilenum = atoi(alVal);
        memset(alFileList,0x00,sizeof(alFileList));
        memcpy(alFileList,alMsgbuf+45,96*ilFilenum);	/*附件列表*/
      
        ilMsglen = 49 + ilMsglen;
        sprintf(alVal,"%08d",ilMsglen);
        memcpy(alMsgbuf,alVal,8);
        alMsgbuf[8] = 'D';
        sprintf(alVal,"%05d",ilRc);
        memcpy(alMsgbuf+9,alVal,5);
        sprintf(alVal,"%05d",ilGrpid);
        memcpy(alMsgbuf+14,alVal,5);
        sprintf(alVal,"%05d",ilMbid);
        memcpy(alMsgbuf+19,alVal,5);
        sprintf(alVal,"%05d",ilPrior);
        memcpy(alMsgbuf+24,alVal,5);
        sprintf(alVal,"%010ld",llType);
        memcpy(alMsgbuf+29,alVal,10);
        sprintf(alVal,"%010ld",llClass);
        memcpy(alMsgbuf+39,alVal,10);
        memcpy(alMsgbuf+49,alMsgbufSaf,ilTmplen);       
        /* 发送报文到服务器 */
        if(igDebug >= 3)
          _bmqDebughex(alMsgbuf,ilMsglen);
        ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
        if ( ilRc != 0 )
        {
          _bmqDebug("S0330: 送回客户端出错!!--Retcode = %d",ilRc );
          fclose(fp);
          goto REPUT;
        }
        
        if(ilFilenum)
        {      
          sprintf(alPath,"%s/temp",getenv("BMQ_FILE_DIR"));
          sprintf(alCommand,"bmqfiletar_x %s %s",alPath,alFileName);
          system(alCommand);
        }
        
        for(i = 0;i < ilFilenum; i++)
        {
          memset(alSendFile,0x00,sizeof(alSendFile));
          memcpy(alSendFile,alFileList+96*i,80);
          _bmqTrim(alSendFile);
          memset(alVal,0x00,sizeof(alVal));
          memcpy(alVal,alFileList+96*i+80,16);
          llFileLen = atol(alVal);
          
          memset(alFile,0x00,sizeof(alFile));
          _bmqfilechr(alSendFile,80,alFile,'/');
          memset(alTmpFileName,0x00,sizeof(alTmpFileName));
          sprintf(alTmpFileName,"%s/temp/%s",getenv("BMQ_FILE_DIR"),alFile);
          if (( fp = fopen(alTmpFileName,"r")) == NULL)
          {
            _bmqDebug("S0340: 不能打开文件[%s] errno:%d:%s",alTmpFileName,errno,strerror(errno));
            goto REPUT;;
          }
          fseek(fp, 0, SEEK_END);
          if(llFileLen  != ftell(fp))
          {
            _bmqDebug("S0350: 文件实际长度(%ld)与报文中描述的长度(%ld)不符!",ftell(fp),llFileLen);
            fclose(fp);
            goto REPUT;;
          }
                  
          /*将文件以流方式发送到SRV*/
          if(llFileLen)
          {             
            /* 发送文件到服务器 */
            fseek(fp, 0, SEEK_SET);
            while(!feof(fp))
            {
              memset(alMsgbuf,0x00,sizeof(alMsgbuf));
              ilMsglen = fread(alMsgbuf,sizeof(char),1024,fp);
              ilRc = _comTcpSend(igSockfd,alMsgbuf,&ilMsglen);
              if(ilRc)
              {
                _bmqDebug("S0360: 文件流送回客户端出错!!--Retcode = %d",ilRc );
                fclose(fp);
                goto REPUT;
              }
            }
          }
          unlink(alTmpFileName);
        }

        ilMsglen = 4;
        ilRc = _bmqRecvconf(igSockfd,alTmp,&ilMsglen);
        if ( ilRc )
        {
          if (ilRc == 1014) 
            _bmqDebug("S0370: 读到的不是合法确认信息,buf=[%s]!",alTmp);
          else if (ilRc == 1016)
            _bmqDebug("S0380: 读确认信息超时!");
          else
            _bmqDebug("S0390: Socket read error[%d]",ilRc);    
          
          goto REPUT;    
        }
        
        /*收到CLI确认应答,文件接收完成,删除SRV的临时文件*/
        if( ilGrpid == 0 ) ilGrpid = psgMbshm->iMbgrpid;
        if( (psgMbshm->iMbgrpid != ilGrpid) && (ilSrvBakFlag==0) )
          unlink(alFileName);   
        continue;
      /* add by xujun 2006.12.28 支持多文件传输 end */      
      default:
        _bmqDebug("S0360: 非法报文,buf=[%s],command=[%d]",alMsgbuf,alMsgbuf[8]);
        return(-1);
    }

    /*送回客户端 */
    ilRc = _comTcpSend ( igSockfd, alMsgbuf, &ilMsglen);
    if ( ilRc != 0 )
    {
      _bmqDebug("S0150 送回客户端出错!!--Retcode = %d",ilRc );
      return (ilRc);
    }
    /* 接收确认信息 */
    if(ilReputflag)
    {
      ilMsglen = 4;
      ilRc = _bmqRecvconf(igSockfd,alTmp,&ilMsglen);
      if(igDebug >= 1)
      {
        _bmqDebug("S0151 接收确认信息 rtcode = [%d]",ilRc); 
      }
      if ( ilRc )
      {
        if (ilRc == 1014) 
        {
          _bmqDebug("S0152 读到的不是合法确认信息!");
        }
        else if (ilRc == 1016)
        {
          _bmqDebug("S0154 读确认信息超时!");
        }
        else
        {
          _bmqDebug("S0156 Socket read error[%d]",ilRc);
        }
        /* 报文回写 */
        if (igProctype == 1)
        {
          ilTmpmb = igMb_sys_current_mailbox;
          ilTmpgrp = igMb_sys_current_group;
          igMb_sys_current_group = ilGrpid;
          igMb_sys_current_mailbox = ilMbid;
          ilRc = bmqPutfilter(ilTmpgrp,ilTmpmb,ilPrior,alFilter,
                  alMsgbuf+29+iMASKLEN+iMASKLEN,ilTmplen);
      	  if(ilRc)
      	  {
            _bmqDebug("S0158 报文[%d][%d][%d][%c~%c]回写FAIL!",
                        ilTmpgrp,ilTmpmb,ilPrior,alFilter,
                        alMsgbuf[29+iMASKLEN*2],
                        alMsgbuf[28+iMASKLEN*2+ilTmplen]);
      	  }
          else
          {
            if( igDebug >= 2 )
            {
              _bmqDebug("S0159 报文[%d][%d][%d][%s][%c~%c]回写成功!",
                          ilGrpid,ilMbid,ilPrior,alFilter,
                          alMsgbuf[29+iMASKLEN*2],
                          alMsgbuf[28+iMASKLEN*2+ilTmplen]);
            }
          }
          igMb_sys_current_mailbox = ilTmpmb;
          igMb_sys_current_group   = ilTmpgrp;
        }
        return (ilRc);
      }   
    }

    if( igDebug >= 1 )
    {
      _bmqDebug("S0160 已成功送回客户端{%s}!--ilMsglen=%d\n",
        alCliname,ilMsglen);
    }
  }
}


