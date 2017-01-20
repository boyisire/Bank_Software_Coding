/****************************************************************/
/* 模块编号    ：BMQFILECLS                                     */
/* 模块名称    ：服务器文件传输侦听模块                         */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2002/12/20                                     */
/* 最后修改日期：2002/12/20                                     */
/* 模块用途    ：新晨Q Client/Server方式下服务器端文件传输侦听  */
/****************************************************************/

/****************************************************************/
/* 修改记录:                                                    */
/*  封版V2.0,程序共691行                                        */
/****************************************************************/

static	int	_bmqClsprocess(  );
extern  int compress(void *src, unsigned long src_len, void *dst);
extern  int	decompress(void *src, unsigned long src_len, void *dst);
extern  int _bmqcompress(void *src, unsigned src_len, void *dst ,char *press);
extern  int _bmqdecompress(void *src, unsigned src_len,	void *dst,char *press);

/*库函数定义*/
#include "bmq.h"

static struct	sockaddr_in sgSvr_addr;
static struct	sockaddr_in sgCli_addr;
static int	igSockfd;
static int	iSend_pack_size;

int main(int argc,char **argv)
{
  int    ilRc,ilLen;
  int    ilSockfd;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
  int  ilOpt;

  /*显示版本信息*/
  _bmqShowversion(argc,argv);

  /*设置信号*/
  _bmqSignalinit();

  /* 创建守护进程 */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0000:创建守护进程失败，退出!");
    exit(FAIL);
  }
 
  /*读入参数配置文件DEBUGFLAG,FILECLS PORT*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  { 
    _bmqDebug("S0010:载入DEBUG参数失败，请查看参数文件%s/etc/bmq.ini",
      getenv("BMQ_PATH"));
    exit(FAIL);
  }
  ilRc = _bmqConfig_load("FILECLS",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0020:载入FILECLS参数失败，请查看参数文件%s/etc/bmq.ini",
      getenv("BMQ_PATH"));
    exit(FAIL);
  }
  
  iSend_pack_size = igFileTranBlockSize;
  if(iSend_pack_size < 128 || iSend_pack_size > iMBMAXPACKSIZE)
  {
    _bmqDebug("S000:发送数据包大小必须在128~%d之间!",iMBMAXPACKSIZE);
    exit(0);
  }
  
  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030:连接共享内存区出错:%d",ilRc);
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
    _bmqDebug("S0040:创建SOCKET错误! errno:%d,%s",errno,strerror(errno));
    exit(FAIL);
  }
 
  ilOpt = 1;
  ilLen = sizeof(ilOpt);
  setsockopt(ilSockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&ilOpt,ilLen);

  /*绑定SOCKET端口*/
  if (bind(ilSockfd,(struct sockaddr*)&sgSvr_addr,sizeof(struct sockaddr_in))
    == -1)
  {
    _bmqDebug("S0050:%d,%s",igBmqclsport,errno,strerror(errno));
    close( ilSockfd );
    exit(FAIL);
  }

  if (listen(ilSockfd, SOMAXCONN) == -1)
  {
    _bmqDebug("S0060:侦听连接出错,errno:%d,%s",errno,strerror(errno)); 
    close (ilSockfd);
    exit(FAIL);
  }

  /*保存进程pid*/
  psgMbshm->lBmqfilecls_pid = getpid();

  _bmqDebug("S0070: ***Client/Server Filetrans 守护进程 listen***");

  for( ; ; )
  {
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(ilSockfd,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      _bmqDebug("S0080:准备连接出错,errno:%d,%s",errno,strerror(errno)); 
      return (-1010); 
    }
 
    switch ( fork() )
    {
      case -1:
        close( ilSockfd );
        close( igSockfd );  
        exit( -1 );
      case 0:	
        close( ilSockfd );
        if ( _bmqClsprocess() != 0 )
        {
          _bmqDebug("S0090:子进程处理出错!");  
          bmqClose();
          close ( igSockfd );
        }
  	exit(0);
      default:
        close( igSockfd );
        break;
    }
  }
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
 short           *length;
delete by wh*/ 
static int _comTcpSend(sockfd, buffer, length)
 int             sockfd;
 unsigned char   *buffer;
 TYPE_SWITCH *length; /*add by wh*/
{
  /*short   len=-1;
  short   ilSendlen=0; delete by wh*/
  TYPE_SWITCH len=-1; /*add by wh*/
  TYPE_SWITCH ilSendlen=0; /*add by wh*/
  struct  timeval  timev;
 
  timev.tv_sec = 60;
  timev.tv_usec = 0;
  setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&timev,sizeof(timev)); 

  while( ilSendlen != *length )
  {
    if ( ( len = send(sockfd,buffer+ilSendlen,*length-ilSendlen,0)) > 0 )
    {
      ilSendlen += len;
    }
    else
    {
      _bmqDebug("S0110: send error! errno:%d,%s",errno,strerror(errno));
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
  int   ilRn = -1040;
  /*short ilRcvlen=0; delete by wh*/
  TYPE_SWITCH ilRcvlen=0; /*add by wh*/
  struct  timeval  timev;
  int   ilLen;

  timev.tv_sec = 60;
  timev.tv_usec = 0;
  setsockopt(sockfd,SOL_SOCKET,SO_RCVTIMEO,&timev,sizeof(timev));
  
  memset (buffer, 0x00, *length);
  ilLen = *length;
  for(;;)
  {
     if ( ( ilRcvlen=recv(sockfd,buffer,ilLen,MSG_EOR)) > 0 )
     {
       if (ilRcvlen != ilLen)
       {
       	 ilLen = ilLen - ilRcvlen;
       	 buffer = buffer + ilRcvlen;
       	 continue; 
       }
       else
         return(0);
     }
     else if(ilRcvlen == 0)
     {
       ilRn = 1040;
       if(igDebug >= 1)
         _bmqDebug("S0130: _comTcpReceive() error=[%d][%s]",errno,strerror(errno));
     }
     else
       _bmqDebug("S0140: _comTcpReceive error! errno:%d,%s",errno,strerror(errno));
     break;
  }   
  return ( ilRn );
}

/**************************************************************
 ** 函数名: _bmqClsprocess
 ** 功能:   处理客户端请求
 ** 作者:   徐军
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

  char   alCliname[80];
  char   alMsgbuf[iMBMAXPACKSIZE+100];
  char	 alBuf[iMBMAXPACKSIZE+100];
  char	 alSendbuf[iMBMAXPACKSIZE+100];
  char	 alVal[iMASKLEN+1];
  char	 alMac[MAC_LEN];
  int    ilRc,ilLen,ilLen1;
  int    ilSrvBakFlag;
  int    ilErrFlag;
  /*short  ilBuflen,ilBuflen1,ilMsglen,ilMsglen1,ilTrytimes,ilSendlen = 0; delete by wh*/
  TYPE_SWITCH ilBuflen,ilBuflen1,ilMsglen,ilMsglen1,ilTrytimes,ilSendlen = 0; /*add by wh*/
  char   alFile[101],alFilename[101],alFileName[101];
  long	 i,llFileserial,llSerialno,llFileid;
  struct transparm slTransparm;
  FILE   *fp;
  long   llOffset;
  char alCompressFlag[10];
  char alCryptFlag[10];
  struct monmsg slMonMsg;

  memset(alVal,0x00,sizeof(alVal));
  memset(&slMonMsg,0x00,sizeof(struct monmsg));

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
    _bmqDebug("S0170:%d,%s",alCliname,errno,strerror(errno));
    close ( igSockfd );
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
        _bmqDebug("S0180: 与{%s}TCP通讯链路已断开!",alCliname);
      close ( igSockfd );
      return(SUCCESS);
    }
    
    else if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0190: 从客户端{%s}收报文出错!-Retcode=%d", alCliname, ilRc);
      close( igSockfd );
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
        _bmqDebug("S0200: 从客户端{%s}收报文出错!-Retcode=%d", alCliname, ilRc);
        close( igSockfd );
        return (ilRc);
      }
      ilLen1 += ilMsglen;
      ilMsglen1 = ilLen1 + 8;
    }    

    alMsgbuf[ilLen] = 0;
   
    if(igDebug >= 1)
    {
      _bmqDebug("S0210: 从客户端{%s}接收报文完毕!!--ilMsglen=%d order=%d", 
        alCliname, ilMsglen + 8 ,alMsgbuf[8]); 
    }
    memset(alVal,0x00,sizeof(alVal));
    switch (alMsgbuf[8])
    {
      case 8: /* Senfile Begin */ 
        memcpy(alVal,alMsgbuf+9,5);
        alVal[5]       = '\0';
        strcpy(alCompressFlag,alVal);
        memcpy(alVal,alMsgbuf+14,5);
        alVal[5]    = '\0';
        strcpy(alCryptFlag,alVal);
        memcpy(alVal,alMsgbuf+19,10);
        alVal[10]   = '\0';
        llOffset    = atol(alVal);

        if(llOffset == 0) /*新的传送*/
        {
          _bmqfilechr(alMsgbuf+29,ilLen-29,alFile,'/');
          sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",alFile);
          bmqSendfile(0,alFilename,slTransparm,&llSerialno);
          if ( (fp = fopen(alFilename,"w+")) == NULL )
          {
            _bmqDebug("S0220: 创建文件[%s]失败,errno=%d",alFilename,errno);
            memcpy(alMsgbuf,"ER",2);
            ilMsglen = 2;
            break;
          }
          else
          {
            memcpy(alMsgbuf,"OK",2);
            sprintf(alVal,"%015ld",llSerialno);
          
            memcpy(alMsgbuf+2,alVal,15);
            llOffset = 0L;
            sprintf(alVal,"%015ld",llOffset);
            memcpy(alMsgbuf+17,alVal,15);
            ilMsglen = 32;
          }
        }
        else if(llOffset > 0) /*断点续传*/
        {
          _bmqfilechr(alMsgbuf+29,ilLen-29,alFile,'/');
          sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",alFile);
          /*重新生成索引号
          bmqSendfile(0,alFilename,slTransparm,&llSerialno);
          */
          if ( (fp = fopen(alFilename,"r+")) == NULL )
          {
            memcpy(alMsgbuf,"ER",2);
            ilMsglen = 2;
            break;
          }
          else
          {
            memcpy(alMsgbuf,"OK",2);
            sprintf(alVal,"%015ld",llSerialno);
            memcpy(alMsgbuf+2,alVal,15);
            fseek(fp, llOffset, SEEK_SET);
            sprintf(alVal,"%015ld",llOffset);
            memcpy(alMsgbuf+17,alVal,15);
            ilMsglen = 32;
          }
        }

        llFileserial = 0;
        ilErrFlag    = 0;
        strcpy(slMonMsg.aStatusDesc,"收到文件发送请求");
        strcpy(slMonMsg.aFileName,alFilename);
        slMonMsg.iSendFlag  = 1;
        slMonMsg.lSendSize  = llOffset;
        slMonMsg.fSendSpeed = 0;
        ftime(&slMonMsg.sBeginTime);
        /*_bmqMonMsgSend(MONMSGCODE,slMonMsg);*/
        break;

      case 9: /* Sendfile end */
        if(ilErrFlag == 0)
        {
          memcpy(alMsgbuf,"OK",2);
          ilMsglen = 2;
          llOffset = ftell(fp); 
          slMonMsg.iSendFlag  = 3;
          slMonMsg.lFileSize  = llOffset;
          slMonMsg.lSendSize  = llOffset;
          strcpy(slMonMsg.aStatusDesc,"接收文件成功");
        }
        else
        {
          llOffset = ftell(fp); 
          memcpy(alMsgbuf,"ER",2);
          sprintf(alVal,"%16ld",llOffset);
          memcpy(alMsgbuf+2,alVal,16);
          ilMsglen = 18;
          slMonMsg.iSendFlag  = 4;
          slMonMsg.lFileSize  = -1; /* 文件长度未知 */
          slMonMsg.lSendSize  = llOffset;
          strcpy(slMonMsg.aStatusDesc,"接收文件失败");
        }
        fclose(fp);        
        ftime(&slMonMsg.sEndTime);
        /*_bmqMonMsgSend(MONMSGCODE,slMonMsg);*/
        break;
      case 10:
        _bmqfilechr(alMsgbuf+9,ilLen-9,alFile,'/');
        sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",alFile);
        bmqSendfile(0,alFilename,slTransparm,&llSerialno);
        memcpy(alMsgbuf,"OK",2);
        sprintf(alVal,"%015ld",llSerialno);
        memcpy(alMsgbuf+2,alVal,15);
        ilMsglen = 17;
        break;

      case 11:
        memcpy(alVal,alMsgbuf+9,8);
        alVal[8]   = '\0';
        llSerialno = atol(alVal);
        /* add by xujun 20061117 next 3 lines
           文件收走后是否在服务器保留备份标志 */
        memcpy(alVal,alMsgbuf+17,5);
        alVal[5]   = '\0';
        ilSrvBakFlag = atoi(alVal);
        memcpy(alVal,alMsgbuf+22,5);
        alVal[5]       = '\0';
        strcpy(alCompressFlag,alVal);
        memcpy(alVal,alMsgbuf+27,5);
        alVal[5]    = '\0';
        strcpy(alCryptFlag,alVal);
        ilRc = _bmqGetindexfile(llSerialno,alFilename);
        if( ilRc )
        {
          _bmqDebug("S0230: 取出索引记录文件错误,ilRc=%d,Fileno=%ld,Filename=%s",
                     ilRc,llSerialno,alFilename);                       
          memcpy(alMsgbuf,"ERROR",5); 
          break;               
        }
        _bmqfilechr(alFilename,strlen(alFilename),alFile,'/');
        sprintf(alFileName,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",alFile);
        
        if ((fp = fopen(alFileName, "r")) != NULL)
        {
          memcpy(alMsgbuf,"BEGIN",5);
          fclose(fp);
        }
        else
          memcpy(alMsgbuf,"OPERR",5);
        strcpy(alMsgbuf+5,alFile);
        ilMsglen = 5 + strlen(alFile);
        break;   
      case 12:
        if ((fp = fopen(alFileName, "r")) == NULL)
        {
          _bmqDebug("S0240: 文件[%s]打开失败:%d,%s",alFileName,errno,strerror(errno));
          ilMsglen = 9;
          sprintf(alMsgbuf,"%08d",ilMsglen);
          alMsgbuf[8] = 'R';
          break;
        } 
        memcpy(alVal,alMsgbuf+9,15);
        alVal[15]   = '\0';
        llOffset = atol(alVal);
        if (llOffset > 0) {
          ilRc = fseek(fp,llOffset,SEEK_SET);
          if (ilRc)
          {
            _bmqDebug("S0250: fseek文件[%s][%ld]失败:%d,%s",alFileName,llOffset,errno,strerror(errno));
            ilMsglen = 9;
            sprintf(alMsgbuf,"%08d",ilMsglen);
            alMsgbuf[8] = 'R';
            break;
          }
        }
        i = 0;                  
        i++;
        /* 取出文件的第一段字符，进行压缩组包 */
        ilMsglen = fread(alBuf,sizeof(char),iSend_pack_size,fp);
        if(strlen(alCompressFlag)!=0)
          ilBuflen =_bmqcompress(alBuf,ilMsglen,alSendbuf,alCompressFlag);
        else
        {
          ilBuflen = ilMsglen;
          memcpy(alSendbuf,alBuf,ilMsglen);
        }
        if(ilBuflen==0)
        {
          ilBuflen = ilMsglen;
          memcpy(alSendbuf,alBuf,ilMsglen);
        }
        if(strlen(alCryptFlag)!=0)
        {
          _bmqCrypt_all(alSendbuf,ilBuflen,FILEKEY,alCryptFlag);
        }
        
        sprintf(alMsgbuf,"%08d",(ilBuflen+17+MAC_LEN));
        alMsgbuf[8]	= 'F';
        sprintf(alMsgbuf+9,"%08ld",i);
        memcpy(alMsgbuf+17,alSendbuf,ilBuflen);
        _bmqMac(alSendbuf,ilBuflen,alMac);
        memcpy(alMsgbuf+17+ilBuflen,alMac,MAC_LEN);
        
        while(1)
        { 
          /* 发送压缩后的数据 */   
          ilTrytimes = 3;
          while(1)
          {
            ilTrytimes--;
            ilSendlen = ilBuflen + 17 + MAC_LEN;
            ilRc =  _comTcpSend(igSockfd, alMsgbuf, &ilSendlen); 
            if(!ilRc) 
              break;
            else if(!ilTrytimes)
            {
              fclose(fp);
              return(-1);	
            }
          }
          /* 取出下一段文件字符 */
          if(!feof(fp))
          {
            ilMsglen = fread(alBuf,sizeof(char),iSend_pack_size,fp);
            if(strlen(alCompressFlag)!=0)
              ilBuflen1 =_bmqcompress(alBuf,ilMsglen,alSendbuf,alCompressFlag);
            else
            {
              ilBuflen1 = ilMsglen;
              memcpy(alSendbuf,alBuf,ilMsglen);
            }
            if(ilBuflen1==0)
            {
              ilBuflen1 = ilMsglen;
              memcpy(alSendbuf,alBuf,ilMsglen);
            } 
            if(strlen(alCryptFlag)!=0)
            {
              _bmqCrypt_all(alSendbuf,ilBuflen,FILEKEY,alCryptFlag);
            }

            sprintf(alMsgbuf,"%08d",(ilBuflen1+17+MAC_LEN));
            alMsgbuf[8]	= 'F';
            i++;
            sprintf(alMsgbuf+9,"%08ld",i);
            memcpy(alMsgbuf+17,alSendbuf,ilBuflen1);
            _bmqMac(alSendbuf,ilBuflen1,alMac);
            memcpy(alMsgbuf+17+ilBuflen1,alMac,MAC_LEN);
          }
          else 
            break;
          ilBuflen = ilBuflen1;
        }
        ilMsglen = 9;
        sprintf(alMsgbuf,"%08d",ilMsglen);
        alMsgbuf[8] = 'E';   
        fclose(fp);     
        /* add by xujun 20061117 
           文件收走后,服务器不作备份 */
        if(ilSrvBakFlag==0) /*不保留*/
          unlink(alFileName);
        break;       
      case 13: /* recv file by filename */
        memcpy(alVal,alMsgbuf+9,5);
        alVal[5]       = '\0';
        strcpy(alCompressFlag,alVal);
        memcpy(alVal,alMsgbuf+14,5);
        alVal[5]    = '\0';
        strcpy(alCryptFlag,alVal);
        memcpy(alFileName,alMsgbuf+19,ilLen - 19);
        _bmqfilechr(alFileName,strlen(alFileName),alFile,'/');
        sprintf(alFileName,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",alFile);
        if ((fp = fopen(alFileName, "r")) != NULL)
        {
          memcpy(alMsgbuf,"BEGIN",5); 
          fclose(fp);
        }
        else
          memcpy(alMsgbuf,"OPERR",5); 
        strcpy(alMsgbuf+5,alFile);
        ilMsglen = 5 + strlen(alFile);
        break;
      case 'F': /* file transform */
        memcpy(alVal,alMsgbuf+9,8);
        alVal[8] = '\0';
        llFileid = atol(alVal);
        ilMsglen = ilLen - 17 - MAC_LEN;
        if( !ilErrFlag && (llFileid == llFileserial+1) )
        {
          _bmqMac(alMsgbuf+17,ilMsglen,alMac);
          if( (memcmp(alMsgbuf+ilLen-MAC_LEN,alMac,MAC_LEN)) &&
              (memcmp(alMsgbuf+ilLen-MAC_LEN,"00000000",MAC_LEN)) )
          {
            /*接收的数据文件可能不正确*/
            ilErrFlag = 1;
            ilMsglen = 0;
            break;
          }
          if(strlen(alCryptFlag)!=0)
          {
            _bmqCrypt_all(alMsgbuf+17,ilLen-17-MAC_LEN,FILEKEY,alCryptFlag);
          }
          /* 解压缩报文数据 */
          if(strlen(alCompressFlag)!=0)
            ilBuflen = _bmqdecompress(alMsgbuf+17,ilLen-17-MAC_LEN,alSendbuf,alCompressFlag);
          else
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilLen-17-MAC_LEN);
          }
          if(ilBuflen==0)
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilLen-17-MAC_LEN);
          }
          if(ilBuflen > 0)
            ilMsglen = fwrite(alSendbuf,sizeof(char),ilBuflen,fp);
          llFileserial = llFileid;
        }
        ilMsglen = 0;
        break; 	
      default:
        ilMsglen = 0;
        _bmqDebug("S0280: 收到未知类型【%d】报文",alMsgbuf[8]);
        break;
    }
   
    if(!ilMsglen) continue; 
    ilRc = _comTcpSend ( igSockfd, alMsgbuf, &ilMsglen);
    if ( ilRc != 0 )
    {
      _bmqDebug("S0290: 送回客户端出错!!--Retcode = %d",ilRc );
      close ( igSockfd );
      return (ilRc);
    }    

    alMsgbuf[ilMsglen] = 0;


    if( igDebug >= 1 )
    {
      _bmqDebug("S0300: 已成功送回客户端{%s}!--ilMsglen=%d\n",
        alCliname,ilMsglen);
    }    
  }
}
