#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

/*����ԭ��*/
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
/* �������    ��swLoadcomcfg                                   */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2001/5/                                        */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/

int swLoadcomcfg( char *aPortName )
{
  FILE  *fp;
  int  ilRc;
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  
  swVdebug(4,"S0010: [��������] swLoadcomcfg(%s)",aPortName);
  memset( (char *)&sgComcfg, 0x00, sizeof(sgComcfg));

  if((fp=_swFopen("config/SWCOMM.CFG","r"))==NULL)
  {
    swVdebug(0,"S0020: ��ͨѶ�����ļ�����!");
    return(-1);
  }

  /* Ԥ��ͨѶ�˿������ļ��������� */
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

      swVdebug(3,"S0040: �˿�ͨѶTCP����:");
      swVdebug(3,"S0050: sgTcpcfg.aPartner_addr= [%s]",sgTcpcfg.aPartner_addr);
      swVdebug(3,"S0060: sgTcpcfg.iPartner_port= [%d]",sgTcpcfg.iPartner_port);
       
      fclose(fp);
      swVdebug(4,"S0070: [��������] swDoitass()������=0");
      return(0);
    } /* end if */   
  } /* end while */

  swVdebug(0,"S0080: ��ȡͨѶ�����ļ�����!");
  fclose(fp);
  return(-1);
}
  
/****************************************************************/
/* �������    ��swConnectser                                   */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2001/5/15                                      */
/* ����޸����ڣ�2001/9/15                                      */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   -1                                      */
/****************************************************************/

int swConnectser(char *aIp,int iPort)
{
  struct sockaddr_in slServ_addr;

  int ilSockfd;
  int ilRc;

  swVdebug(4,"S0090: [��������] swConnetser()");
  /* create endpoint */
  if ((ilSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    swVdebug(0,"S0100: [����/ϵͳ����] socket()���� F[%s]:L[%d]:Connect Server Error",__FILE__,__LINE__);
    swVdebug(0,"S0110: [����/ϵͳ����] socket()���� Connect Server Error errno=%d[%s]",errno,strerror(errno));
    return -1;
  }
  swVdebug(2,"S0120: [%s][%d]",aIp,iPort);
 
  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( aIp );
  slServ_addr.sin_port = htons( iPort );

  ilRc = connect(ilSockfd,(struct sockaddr *)&slServ_addr,sizeof(slServ_addr));
  if (ilRc < 0)
  {
    swVdebug(0,"S0130: [����/ϵͳ����] connect()����,F[%s]:L[%d]:Connect Server Error",__FILE__,__LINE__);
    swVdebug(0,"S0140: [����/ϵͳ����] connect()����,Connect Server Error Code[%d]--%s",errno,
      strerror(errno));
    close(ilSockfd);
    return -1;
  }
  swVdebug(4,"S0150: [��������] swConnectser()������=%d",ilSockfd);
  return ilSockfd;
}
 
/****************************************************************/
/* �������    ��swTcprcv                                       */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/8                                       */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/
int swTcprcv(int iSockfd,char *aBuffer,unsigned int *piLength)
{
  int    ilRc = -1;
  unsigned int ilRcvlen=0;
  unsigned int ilLength=0;
  char   alLen[20];

  swVdebug(4,"S0160: [��������] swTcprcv()");
  if (sgComcfg.aExp_len[0])
  {
    /* ���ݳ��ȱ��ʽ��ȡ���� */
    ilRc = swReadn(iSockfd,aBuffer,sgComcfg.iPre_msglen);
    if (ilRc)
    {
      swVdebug(0,"S0170: [����/��������] swReadn()��������,������=%d",ilRc);
      return(-1);
    }
    agMsgbody = aBuffer;
    igMsgbodylen = sgComcfg.iPre_msglen;
    ilRc = _swExpress(sgComcfg.aExp_len,alLen,&ilLength);
    if (ilRc)
    {
      swVdebug(0,"S0180: [����/��������] _swExpress()��������,������=%d",ilRc);
      return(-1);
    }
    alLen[ilLength] = '\0';
    ilRcvlen = atoi(alLen);
    ilRc=swReadn(iSockfd,aBuffer+sgComcfg.iPre_msglen,ilRcvlen);
    if ( ilRc )
    {
      swVdebug(0,"S0190: [����/��������] swReadn()����,������=%d",ilRc);
      return(-1);
    }
    *piLength = sgComcfg.iPre_msglen + ilRcvlen;
    swVdebug(4,"S0200: [��������] swTcprcv()������=0");
    return(0);
  }

  if (sgComcfg.aEnd_string[0])
  {
    /* ���ݽ����ַ���ȡ���� */
    ilRc = swReadf(iSockfd,aBuffer,piLength);
    if (ilRc)
    {
      swVdebug(0,"S0210: [����/��������] swReadf()��������,������=%d",ilRc);
      return(-1);
    }
    swVdebug(4,"S0220: [��������] swTcprcv()������=0");
    return(0);
  }

  /* ������һ���Զ�ȡ���� */
  ilRc = recv(iSockfd,aBuffer,iMSGMAXLEN,0);
  if (ilRc < 0)
  {
    swVdebug(0,"S0230: [����/ϵͳ����] recv()����,errno=%d[%s]",errno,strerror(errno));
    return(-1);
  }
  *piLength = ilRc;
  swVdebug(2,"S0240: recv()�ɹ�");
  swVdebug(4,"S0250: [��������] swTcprcv()������=0");
  return(0);
}

/****************************************************************/
/* �������    ��swReadf                                        */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/

int swReadf(int iSockfd,char *aBuffer,unsigned int *iMsglen)
{
  int  ilRc,ilLen = 0;
  unsigned int ilStrlen;
  char alString[11];
  unsigned int ilMsglen;
   
  swVdebug(4,"S0260: [��������] swReadf()����");
  ilRc = swHextoasc(sgComcfg.aEnd_string,alString,&ilStrlen);
  if (ilRc)
  {
    swVdebug(0,"S0270: [����/��������] swHextoasc()����,���Ľ����������,������=%d",ilRc);
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
      swVdebug(0,"S0280: [����/ϵͳ����] read()��������,errno=%d[%s]",errno,strerror(errno));
      return(-1);
    }
    if (ilLen==0)
    {
      swVdebug(0,"S0290: [����/����] read()�����ձ���");
      return(-1);
    }
    ilMsglen=ilMsglen+ilLen;
    if (ilMsglen>=ilStrlen)
    {
      ilRc = memcmp(aBuffer+ilMsglen-ilStrlen,alString,ilStrlen);
      if (ilRc == 0)
      {
        *iMsglen = ilMsglen;
        swVdebug(4,"S0300: [��������] swReadf()������=0");
        return(0);
      }
    }
  }
}

/****************************************************************/
/* �������    ��swReadn                                        */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/

int swReadn(int iSockfd, char *aBuffer,unsigned int iLength)
{
  int  ilLen = 0;
  int  ilTotalcnt=0;

  swVdebug(4,"S0310: [��������] swReadn()");
  for(;;)
  {
    ilLen = read(iSockfd, aBuffer+ilTotalcnt,iLength-ilTotalcnt);
    if (ilLen<0) 
    {
      if (errno==EINTR)
      {
        continue;
      }
      swVdebug(0,"S0320: [����/ϵͳ����] read()��������,errno=%d[%s]",errno,strerror(errno));
      return(-1);
    }
    if (ilLen==0)
    {
      swVdebug(0,"S0330: [����/����] read()�����ձ���");
      return(-1);
    }
    ilTotalcnt=ilTotalcnt+ilLen;
    if (ilTotalcnt >= iLength)
    {
      swVdebug(4,"S0340: [��������] swReadn()������=0");
      return(0);
    }
  }
}

/****************************************************************/
/* �������    ��swTcpsnd                                     */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/

int swTcpsnd(int iSockfd,char *aBuffer,unsigned int iLen)
{
  int  ilSendlen = 0;
  int  ilCount= 0;
  
  swVdebug(4,"S0350: [��������] swTcpsend()");
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
        swVdebug(0,"S0370: [����/ϵͳ����] write()��������,errno=%d[%s]",errno,strerror(errno));
        return(-1);
      }
    }
    if (ilSendlen == 0)
    {
      swVdebug(0,"S0380: [����/����] write()���������ֽ���Ϊ0");
      return(-1);
    }
    swVdebug(2,"S0390: write() SUCCESS");
    ilCount=ilCount+ilSendlen;
    if (ilCount >= iLen) 
    {
      swVdebug(4,"S0400: [��������] swTcpsend()������=0");
      return(0);
    }
  }
}

/****************************************************************/
/* �������    ��swTcpconnect                                   */
/* ��������    ��                                               */
/* ��    ��    : ������                                         */
/* ��������    ��2001/8/16                                      */
/* ����޸����ڣ�                                               */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   -1                                      */
/****************************************************************/

int swTcpconnect( int iPort )
{
  struct sockaddr_in slServ_addr;
  int ilSockid;
  int ilRc;

  swVdebug(4,"S0410: [��������] swTcpconnect(%d)",iPort);
  memset((char *)&slServ_addr,0x00,sizeof(struct sockaddr_in));

  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  slServ_addr.sin_port = htons(iPort);
  ilSockid=socket(AF_INET,SOCK_STREAM,0);
  if (ilSockid==-1)
  {
    swVdebug(0,"S0420: [����/ϵͳ����] socket()����,����SOCKET����,errno=%d[%s]",errno,strerror(errno));
    return(-1);
  }
  ilRc=bind(ilSockid,(struct sockaddr*)&slServ_addr,sizeof(struct sockaddr_in));
  if (ilRc == -1)
  {
    swVdebug(0,"S0430: [����/ϵͳ����] bind()����,��SOCKET����,errno=%d[%s]",errno,strerror(errno));
    close(ilSockid);
    return(-1);
  }

  if (listen(ilSockid,SOMAXCONN)==-1)
  {
    swVdebug(0,"S0440: [����/ϵͳ����] listen()����,�������Ӵ���,errno=%d[%s]",errno,strerror(errno));
    close(ilSockid);
    return(-1);
  }
  swVdebug(4,"S0450: [��������] swTcpconnect()������=%d",ilSockid);
  return(ilSockid);
}

/****************************************************************/
/* �������    ��swCheck_comm                                   */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  :                                                */
/*                                                              */
/****************************************************************/

int swCheck_comm()
{
  int ilRc,ilConnect_flag=0;
  long ilSockid;
  short ilTrytimes;

  swVdebug(4,"S0460: [��������] swCheck_comm()");
  /* next modify by nh 2002.6.6
  ilRc = swPortset( sgComcfg.iMb_comm_id, 1, 0); */
  ilRc = swPortset( sgComcfg.iMb_fore_id, 1, 0);
  if ( ilRc )
  {
    swVdebug(0,"S0470: [����/��������] swPortset()����,�ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
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
    swVdebug(0,"S0480: [����/��������] swConnectser()����,���ӵ�����������!!");
    return( -1 );
  }
  /* �ö˿�ͨѶ��ΪUP */
  /* next modify by nh 2002.6.6
  ilRc = swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
  ilRc = swPortset( sgComcfg.iMb_fore_id, 1, 1 );
  if ( ilRc )
  {
    swVdebug(0,"S0490: [����/��������] swPortset()����,�ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
    return( -1 );
  }
  swVdebug(4,"S0500: [��������] swCheck_comm()������=%d",ilSockid);
  return( ilSockid );
}

/****************************************************************/
/* �������    ��swswConnsend                                   */
/* ��������    ��                                               */
/* ��    ��    ��                                               */
/* ��������    ��2000/9/15                                      */
/* ����޸����ڣ�                                               */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   -1                                      */
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
 
  swVdebug(4,"S0510: [��������] swConnsend(%d,%d,%d)",iflag,*iSockfd,iMsglen);
  pslMsghead = (struct msghead *)aMsgbuf;

  /*���ӵ�������*/
  if ( iflag == 0 )         /*�������ĵ�һ����*/
  {
    *iSockfd = swConnectser( sgTcpcfg.aPartner_addr, sgTcpcfg.iPartner_port );
    if (*iSockfd < 0) 
    {
      *iSockfd=swCheck_comm();
      if ( *iSockfd < 0 )
      {
        swVdebug(1,"S0520: [����/��������] swChek_comm() errno=%d[%s]",errno,strerror(errno));
        return(-1);
      }
    }
    swVdebug(2,"S0530: ���ӵ��������ɹ�");  
    ilAddrlen = sizeof(struct sockaddr_in);
    memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
 
    ilRc = getsockname(*iSockfd,(struct sockaddr*)&slCli_addr, \
      &ilAddrlen);
    if ( ilRc == -1 )
    {  
      swVdebug(1,"S0540: [����/ϵͳ����] getsockname()����,read socket errno=%d[%s]",errno,strerror(errno));
      return(-1);
    }
    swVdebug(2,"S0550: getsockname SUCCESS");

    slLinger.l_onoff = 1 ;
    slLinger.l_linger = 1 ;
  
    ilRc = setsockopt(*iSockfd,SOL_SOCKET,SO_LINGER,&slLinger, 
    sizeof(struct linger));
    if ( ilRc ) 
    {
      swVdebug(0,"S0560: [����/ϵͳ����] setsockopt()����,Connected errno=%d[%s]",errno, strerror(errno));
      return(-1); 
    }

    if ( cgDebug >= 1 )
    {
      swVdebug(2,"S0570: ���ӵ�������{%s}�Ķ˿�[%d]�ɹ�", \
        sgTcpcfg.aPartner_addr,sgTcpcfg.iPartner_port);
    }
  }
           
  /* ���ķ��͵��˿� */
  swVdebug(2,"S0580: ���ͱ��ĵ�������...");
     
  /*�ж����ޱ���ͷ*/
  if(sgComcfg.iMsghead == 0)
  {
    
    /* ��������ͷ���ⷢ */
    ilRc = swTcpsnd(*iSockfd,aMsgbuf + sizeof(struct msghead),
      iMsglen - sizeof(struct msghead));
  }
  else
  {
    ilRc = swTcpsnd(*iSockfd,aMsgbuf,iMsglen);
  }
  if ( ilRc ) 
  {
    swVdebug(1,"S0590: [����/��������] swTcpsend()����,���ͱ��ĵ�����������,errno=%d[%s]",errno,strerror(errno));
    return(-1);
  }

  swVdebug(2,"S0600: ���ͳ���Ϊ[%d]���ĵ��������ɹ�!",iMsglen);
  swVdebug(4,"S0610: [��������] swconnsend()������=0");
  return(0);
}
