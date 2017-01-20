/****************************************************************/
/* ģ����    ��BMQGRP_SND                                     */
/* ģ������    �����鼶������ģ��                               */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���³�Q ���鼶�������ػ�����                     */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2001.08.17 ���V2.0,����325��                              */
/****************************************************************/
#include "bmqtype.h"    /*add by wh*/

/*��������*/
int _bmqGrpsndproc(short iGrpid,char *aHostip,short iPort,long lTrytime);
static int _bmqGrpcontser(char *alIp,short ilPort); 
/*static int _bmqGrpSend(int sockfd, char *buffer, short *length);delete by wh*/
static int _bmqGrpSend(int sockfd, char *buffer, TYPE_SWITCH *length); /*add by wh*/
extern int _comTcpSend();
extern int _comTcpReceive();

/*�⺯������*/
#include "bmq.h"

static  long	lgSndpid[1024];
static  void  _bmqQuit();

int main(int argc,char **argv)
{
  int    ilRc,i;
  int    ilStatus;

  /*�����ź�*/
  _bmqSignalinit();
  signal(SIGTERM,_bmqQuit);

  /* �����ػ����� */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010 %s:�����ػ�����ʧ��!",__FILE__);
    exit(FAIL);
  }

  /*������������ļ�DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0020 %s :����DEBUG����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }

  /*���ӹ����ڴ�*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030 %s:���ӹ����ڴ�������:%d",__FILE__,ilRc);
    exit(FAIL);
  }

  /*�������pid*/
  psgMbshm->lBmqgrpsnd_pid = getpid();
 
  _bmqDebug("S0040 ***���鼶�� �����ػ����� msgrcv***");
  
  for(i=0;i<1024;i++) lgSndpid[i] = 0;
  i = 0;
  while(1)
  {
    /*ȡ��Ŀ�����IP��ַ�Ͷ˿ں�*/ 
    ilRc = _bmqConfig_load("GROUP",-1);
    if (ilRc < 0)
    {
      _bmqDebug("S0100 -1182:ȡ��Ŀ�����IP��ַ�Ͷ˿ںŴ���!");
      continue;
    }
    /*Ŀ����ʹ�ö����� */
    if( (agCommode[0] == 'S') && ( ilRc != FINISH) ) continue;
    if (ilRc == FINISH) break;
    if( igDebug >= 2 )
    {
       _bmqDebug("S0110 ȡ��Ŀ����[%d]��IP��ַ�Ͷ˿ں�:[%s][%d]",
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
    _bmqDebug("S0111 ���鷢�ͽ��̽���!!");
    exit(0);
  }
  exit(0);
}

/**************************************************************
 ** ������: _bmqGrpsndproc
 ** ����:   ���ʹ������
 ** ����:
 ** ��������: 2001/09/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: 0--�ɹ���-1031--ʧ��
***************************************************************/
int _bmqGrpsndproc(short iGrpid,char *aHostip,short iPort,long lTrytime)
{
  int	 ilRc,ilLentmp;
  int    ilSockfd;                 /*�׽���*/
  /*short  ilMsglen,ilMsglen_grp;    delete by wh ���ĳ���*/
  TYPE_SWITCH ilMsglen,ilMsglen_grp;    /*���ĳ��� add by wh*/
  short  ilGrpid,ilMbid,ilPrior;
  char   alFilter[iMASKLEN];
  char   alVal[11],alTmp[10];
  char   alMacbuf[MAC_LEN];
  char   alMsgbuf[iMBMAXPACKSIZE+100];       /*���Ĵ洢����*/
  char   alMsgbuf_grp[iMBMAXPACKSIZE+100];   /*���ķ�������*/
  struct grprecvmsg slRcvmsg_grp;  /*������Ϣ�ṹ*/
  struct linger ilLinger;
  struct sockaddr_in slCli_addr;
  fd_set	infds,outfds;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif

  /*���ӵ�������*/
GRPSND:
  while(1)
  {
    ilSockfd = _bmqGrpcontser(aHostip, iPort);
    if ( ilSockfd < 0 )
    {
      if( igDebug >= 1 )
        _bmqDebug("S0120 ���ӵ�����������!! IP=[%s] Port=[%d],[%ld]�������",aHostip, iPort,lTrytime);
      sleep(lTrytime);
      continue;
    }
    else
    {
      _bmqDebug("S0121 ���ӵ�������IP=[%s] Port=[%d]�ɹ�",aHostip, iPort);
      break;
    }
  }

  /*ȡ��SOCKET��Ϣ*/
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( ilSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0130 unable to read socket address");
    close(ilSockfd);
    return(FAIL);
  }

  /*����SOCKETѡ��*/
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
  
  /* �������ȷ����Ϣ,����ź�IP��ַ(���봮)���Դ�� */
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
    _bmqDebug("S0141 �������ȷ����Ϣʧ��!");
    close(ilSockfd);
    return(FAIL);
  }
  memset(alMsgbuf,0x00,sizeof(alMsgbuf));
  ilMsglen_grp = 5;
  ilRc = _comTcpReceive(ilSockfd,alMsgbuf,&ilMsglen_grp);
  if( ilRc )
  {
    _bmqDebug("S0142 �������ȷ��Ӧ����Ϣʧ��!");
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"UNDEF",5))
  {
    _bmqDebug("S0143 ������IP=[%s]��δ���屾����Ϣ!!!",aHostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"UNMAC",5))
  {
    _bmqDebug("S0144 ������IP=[%s]δͨ�����ȷ����Ϣ!!!",aHostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"IPERR",5))
  {
    _bmqDebug("S0145 ������IP=[%s]δͨ�����ȷ����Ϣ!!!",aHostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(memcmp(alMsgbuf,"AAAAA",5))
  {
    _bmqDebug("S0146 ��������");
    close(ilSockfd);
    return(FAIL);
  }
 
  while(1)
  {
    /*������ȡ���鴫�ʹ�����Ϣ*/
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
      _bmqDebug("S0070 �������Ϣ����(��)�ж�����Ϣ:Pack_head=[%ld],Des_grp=[%d] Des_mailbox=[%d]",
       slRcvmsg_grp.lPack_head,
       slRcvmsg_grp.iDes_group,
       slRcvmsg_grp.iDes_mailbox);
    }

    /* �ж�Server���Ƿ��Ѿ�close */
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
        _bmqDebug("S0071 ���鷢����Ϣ���� msgsnd errno:%d:%s",
                    errno,strerror(errno));
      }
      goto GRPSND;
    }
    if(FD_ISSET(ilSockfd,&infds))
    {
      ilRc = recv(ilSockfd,alTmp,10,0);
      if(ilRc <= 0)
      {
        _bmqDebug("S0072 ��SOCKET�����źŴ���!");
        ilRc = msgsnd(psgMbshm->lMsgid_grp,&slRcvmsg_grp, 
                        sizeof(struct grprecvmsg),0);
        if(ilRc < 0)
        {
          _bmqDebug("S0074 ���鷢����Ϣ���� msgsnd errno:%d:%s", 
                       errno,strerror(errno));
        }
        goto GRPSND;
      }
    }
    if(FD_ISSET(ilSockfd,&outfds))
    {
    /*��ȡ�����޸�������Ϣ*/
    ilRc = _bmqLock(LK_ALL);
    if( ilRc ) 
    {
      _bmqDebug("S0075 �����źŵƳ���!");
      close(ilSockfd);
      return(ilRc);
    }
    ilMsglen = sizeof(alMsgbuf);
    ilRc = _bmqPack_get(alMsgbuf,&ilMsglen,slRcvmsg_grp.lPack_head);
    if(ilRc ==FAIL)
    {
      _bmqDebug("S0080 ��ȡ����ʧ��!");
      _bmqUnlock(LK_ALL);
      continue;
    }
    if( igDebug >= 0 )
    {
      _bmqDebug("S0090 ��ȡ����[%d]����[%d]��[%d]����",
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

    /*��֯����*/
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
  
    /*���ͱ���*/
    ilRc = _bmqGrpSend(ilSockfd,alMsgbuf_grp,&ilMsglen_grp);
    if ( ilRc != 0 )
    {
      _bmqDebug("S0150 %s: Send to server ERROR!!",__FILE__);
      /* add by xujun 2001.11.29 start */
      /*���Ĵ���*/

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
        _bmqDebug("S0151 ����[%d]����[%d][%d]����!",ilMsglen,ilGrpid,ilMbid);
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
 ** ������: _bmqGrpcontser
 ** ����:   ���ӷ�����
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:alIp--������IP��ַ  ilPort--ͨѶ�˿�
 ** ����ֵ: 0--�ɹ�
***************************************************************/
static int _bmqGrpcontser(char *alIp,short ilPort)
{
  int    sockfd;
  struct sockaddr_in slServ_addr;

  /* �����׽��� */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    _bmqDebug("S0170 %s : Creat Socket Error! :errno: %d,%s",__FILE__,errno,strerror(errno));
    return (-1050);
  }

  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( alIp );
  slServ_addr.sin_port = htons( ilPort );

  /*����ͨѶ�˿�*/
  if(connect(sockfd,(struct sockaddr *)&slServ_addr,sizeof(slServ_addr)) < 0)
  {
    _bmqDebug("S0180 %s: Connect Server[%s][%d] Error! errno: %d,%s",__FILE__,alIp,ilPort,errno,strerror(errno));
    close(sockfd);
    return (-1060);
  }

  return sockfd;
}

/**************************************************************
 ** ������: _comGrpSend
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
