/****************************************************************/
/* ģ����    ��BMQFILECLS                                     */
/* ģ������    ���������ļ���������ģ��                         */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2002/12/20                                     */
/* ����޸����ڣ�2002/12/20                                     */
/* ģ����;    ���³�Q Client/Server��ʽ�·��������ļ���������  */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼:                                                    */
/*  ���V2.0,����691��                                        */
/****************************************************************/

static	int	_bmqClsprocess(  );
extern  int compress(void *src, unsigned long src_len, void *dst);
extern  int	decompress(void *src, unsigned long src_len, void *dst);
extern  int _bmqcompress(void *src, unsigned src_len, void *dst ,char *press);
extern  int _bmqdecompress(void *src, unsigned src_len,	void *dst,char *press);

/*�⺯������*/
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

  /*��ʾ�汾��Ϣ*/
  _bmqShowversion(argc,argv);

  /*�����ź�*/
  _bmqSignalinit();

  /* �����ػ����� */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0000:�����ػ�����ʧ�ܣ��˳�!");
    exit(FAIL);
  }
 
  /*������������ļ�DEBUGFLAG,FILECLS PORT*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  { 
    _bmqDebug("S0010:����DEBUG����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      getenv("BMQ_PATH"));
    exit(FAIL);
  }
  ilRc = _bmqConfig_load("FILECLS",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0020:����FILECLS����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      getenv("BMQ_PATH"));
    exit(FAIL);
  }
  
  iSend_pack_size = igFileTranBlockSize;
  if(iSend_pack_size < 128 || iSend_pack_size > iMBMAXPACKSIZE)
  {
    _bmqDebug("S000:�������ݰ���С������128~%d֮��!",iMBMAXPACKSIZE);
    exit(0);
  }
  
  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030:���ӹ����ڴ�������:%d",ilRc);
    exit(FAIL);
  }

  memset(&sgSvr_addr,0x00,sizeof(struct sockaddr_in));
  memset(&sgCli_addr,0x00,sizeof(struct sockaddr_in));
  
  sgSvr_addr.sin_family = AF_INET;
  sgSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sgSvr_addr.sin_port = htons(igBmqclsport);

  /*����SOCKET*/
  ilSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (ilSockfd == -1)
  {
    _bmqDebug("S0040:����SOCKET����! errno:%d,%s",errno,strerror(errno));
    exit(FAIL);
  }
 
  ilOpt = 1;
  ilLen = sizeof(ilOpt);
  setsockopt(ilSockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&ilOpt,ilLen);

  /*��SOCKET�˿�*/
  if (bind(ilSockfd,(struct sockaddr*)&sgSvr_addr,sizeof(struct sockaddr_in))
    == -1)
  {
    _bmqDebug("S0050:%d,%s",igBmqclsport,errno,strerror(errno));
    close( ilSockfd );
    exit(FAIL);
  }

  if (listen(ilSockfd, SOMAXCONN) == -1)
  {
    _bmqDebug("S0060:�������ӳ���,errno:%d,%s",errno,strerror(errno)); 
    close (ilSockfd);
    exit(FAIL);
  }

  /*�������pid*/
  psgMbshm->lBmqfilecls_pid = getpid();

  _bmqDebug("S0070: ***Client/Server Filetrans �ػ����� listen***");

  for( ; ; )
  {
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(ilSockfd,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      _bmqDebug("S0080:׼�����ӳ���,errno:%d,%s",errno,strerror(errno)); 
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
          _bmqDebug("S0090:�ӽ��̴������!");  
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
 ** ������: _comTcpSend
 ** ����:   ��������
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--������Ϣ�ĵ�ַ
             length--������Ϣ�ĳ���
 ** ����ֵ: 0--�ɹ���-1031--ʧ��
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
 ** ������: _comTcpReceive
 ** ����:   ��������
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--�洢��Ϣ�ĵ�ַ
             length--�����������ߴ�
 ** ����ֵ: 0--�ɹ���-1041--ʧ��
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
 ** ������: _bmqClsprocess
 ** ����:   ����ͻ�������
 ** ����:   ���
 ** ��������: 2000/10/26
 ** ����޸�����:2001/08/08
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
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

  /*��ÿͻ���������*/
  hp = gethostbyaddr( (char *) &sgCli_addr.sin_addr, sizeof(struct in_addr),
    sgCli_addr.sin_family);
  if( hp == NULL)
    strcpy( alCliname, inet_ntoa( sgCli_addr.sin_addr ) );
  else
    strcpy( alCliname, hp->h_name );

  ilLinger.l_onoff  =1;
  ilLinger.l_linger =1;
  /*���� SOCKET ���� */
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
    /*�ӿͻ��˽��ձ���*/
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;
   
    /*����8�ֽڱ��ĳ���*/
    ilRc = _comTcpReceive(igSockfd, alMsgbuf, &ilMsglen) ; 

    /* next 7 rows add by nh 2002/7/4 */
    if(ilRc == 1040)
    {
      if( igDebug >= 1)
        _bmqDebug("S0180: ��{%s}TCPͨѶ��·�ѶϿ�!",alCliname);
      close ( igSockfd );
      return(SUCCESS);
    }
    
    else if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0190: �ӿͻ���{%s}�ձ��ĳ���!-Retcode=%d", alCliname, ilRc);
      close( igSockfd );
      return (ilRc);
    }

    memcpy(alVal,alMsgbuf,8);
    alVal[8] = '\0';
    ilLen = atoi(alVal);
    ilLen1 = ilMsglen1 =0;

     
    /*����ָ�����ȵı���*/
    while( ilMsglen1 < ilLen )
    {
      ilMsglen = ilLen - 8 - ilLen1;
      ilRc = _comTcpReceive(igSockfd, alMsgbuf+8+ilLen1, &ilMsglen) ;
      if ( ilRc != 0 || ilMsglen <= 0)
      {
        _bmqDebug("S0200: �ӿͻ���{%s}�ձ��ĳ���!-Retcode=%d", alCliname, ilRc);
        close( igSockfd );
        return (ilRc);
      }
      ilLen1 += ilMsglen;
      ilMsglen1 = ilLen1 + 8;
    }    

    alMsgbuf[ilLen] = 0;
   
    if(igDebug >= 1)
    {
      _bmqDebug("S0210: �ӿͻ���{%s}���ձ������!!--ilMsglen=%d order=%d", 
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

        if(llOffset == 0) /*�µĴ���*/
        {
          _bmqfilechr(alMsgbuf+29,ilLen-29,alFile,'/');
          sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",alFile);
          bmqSendfile(0,alFilename,slTransparm,&llSerialno);
          if ( (fp = fopen(alFilename,"w+")) == NULL )
          {
            _bmqDebug("S0220: �����ļ�[%s]ʧ��,errno=%d",alFilename,errno);
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
        else if(llOffset > 0) /*�ϵ�����*/
        {
          _bmqfilechr(alMsgbuf+29,ilLen-29,alFile,'/');
          sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",alFile);
          /*��������������
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
        strcpy(slMonMsg.aStatusDesc,"�յ��ļ���������");
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
          strcpy(slMonMsg.aStatusDesc,"�����ļ��ɹ�");
        }
        else
        {
          llOffset = ftell(fp); 
          memcpy(alMsgbuf,"ER",2);
          sprintf(alVal,"%16ld",llOffset);
          memcpy(alMsgbuf+2,alVal,16);
          ilMsglen = 18;
          slMonMsg.iSendFlag  = 4;
          slMonMsg.lFileSize  = -1; /* �ļ�����δ֪ */
          slMonMsg.lSendSize  = llOffset;
          strcpy(slMonMsg.aStatusDesc,"�����ļ�ʧ��");
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
           �ļ����ߺ��Ƿ��ڷ������������ݱ�־ */
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
          _bmqDebug("S0230: ȡ��������¼�ļ�����,ilRc=%d,Fileno=%ld,Filename=%s",
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
          _bmqDebug("S0240: �ļ�[%s]��ʧ��:%d,%s",alFileName,errno,strerror(errno));
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
            _bmqDebug("S0250: fseek�ļ�[%s][%ld]ʧ��:%d,%s",alFileName,llOffset,errno,strerror(errno));
            ilMsglen = 9;
            sprintf(alMsgbuf,"%08d",ilMsglen);
            alMsgbuf[8] = 'R';
            break;
          }
        }
        i = 0;                  
        i++;
        /* ȡ���ļ��ĵ�һ���ַ�������ѹ����� */
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
          /* ����ѹ��������� */   
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
          /* ȡ����һ���ļ��ַ� */
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
           �ļ����ߺ�,�������������� */
        if(ilSrvBakFlag==0) /*������*/
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
            /*���յ������ļ����ܲ���ȷ*/
            ilErrFlag = 1;
            ilMsglen = 0;
            break;
          }
          if(strlen(alCryptFlag)!=0)
          {
            _bmqCrypt_all(alMsgbuf+17,ilLen-17-MAC_LEN,FILEKEY,alCryptFlag);
          }
          /* ��ѹ���������� */
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
        _bmqDebug("S0280: �յ�δ֪���͡�%d������",alMsgbuf[8]);
        break;
    }
   
    if(!ilMsglen) continue; 
    ilRc = _comTcpSend ( igSockfd, alMsgbuf, &ilMsglen);
    if ( ilRc != 0 )
    {
      _bmqDebug("S0290: �ͻؿͻ��˳���!!--Retcode = %d",ilRc );
      close ( igSockfd );
      return (ilRc);
    }    

    alMsgbuf[ilMsglen] = 0;


    if( igDebug >= 1 )
    {
      _bmqDebug("S0300: �ѳɹ��ͻؿͻ���{%s}!--ilMsglen=%d\n",
        alCliname,ilMsglen);
    }    
  }
}
