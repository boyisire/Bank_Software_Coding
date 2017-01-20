/****************************************************************/
/* ģ����    ��BMQGRP_RCVS                                    */
/* ģ������    �����鼶������ģ��(������SVR)                    */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2007/01/28                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���³�Q ��������Ӽ��������ػ�����,���̴߳���,
                 ���ձ��ĺ�ͨ����Ϣ���з��͸����Ĵ洢ģ��       */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*                                                              */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"
#include <sys/param.h>
#include <pthread.h>

static struct sockaddr_in sgSvr_addr;
static struct sockaddr_in sgCli_addr;
static int igSockfd;

/*extern int _bmqGrpRecv( int sockfd,char *buffer, short *length); delete by wh*/
extern int _bmqGrpRecv( int sockfd,char *buffer, TYPE_SWITCH *length); /*add by wh*/
extern int _comTcpSend();

pthread_t server_thread[DEFINE_THREADS];
pthread_mutex_t  connsock_mutex,server_lock;
void *server_proc(void *);

struct Msgbuf{
  long   mtype;
  struct grprecvmsg rcvmsg;
  long   msglen;
  char   data[7168];
}sgMsgbuf;

int main(int argc,char **argv)
{
  int     i,ilRc,ilLen;
  int     ilOpt;
  pthread_attr_t attr;

  /*��ʾ�³�Q�汾��Ϣ*/
  _bmqShowversion(argc,argv);

  /*�����ź�*/
  _bmqSignalinit();
  
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

  /*ȡ�������IP��ַ�Ͷ˿ں�*/
  ilRc = _bmqConfig_load("GROUP",psgMbshm->iMbgrpid);
  if (ilRc < 0)
  {  
    _bmqDebug("S0040 %s :����GROUP[%d]����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,psgMbshm->iMbgrpid,getenv("BMQ_PATH"));
    exit(FAIL);
  }
  
  if(agCommode[0] != 'S')
  {
    _bmqDebug("S0042 �������õ����ӷ�ʽ���Ƕ�����!");
    exit(FAIL);
  }
  
  memset(&sgSvr_addr,0x00,sizeof(struct sockaddr_in));
  memset(&sgCli_addr,0x00,sizeof(struct sockaddr_in));
  
  sgSvr_addr.sin_family = AF_INET;
  sgSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sgSvr_addr.sin_port = htons(igBmqgrpport);

  /*����SOCKET*/
  igSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (igSockfd == -1)
  {
    _bmqDebug("S0050 ����SOCKET����!errno: %d,%s",errno,strerror(errno));
    exit(FAIL);
  }

  ilOpt = 1;
  ilLen = sizeof(ilOpt);
  ilRc = setsockopt(igSockfd,SOL_SOCKET,SO_REUSEADDR,(char *)&ilOpt,ilLen);
  if(ilRc < 0)
  {
    _bmqDebug("S0055 Set Socketopt Error! errno:%d,%s",errno,strerror(errno));
    close( igSockfd );
    exit(FAIL);
  }

  /*��SOCKET*/
  if (bind(igSockfd,(struct sockaddr*)&sgSvr_addr,sizeof(struct sockaddr_in))
    == -1)
  {
    _bmqDebug("S0060 ��SOCKET���˿�[%d]����!! errno: %d,%s",igBmqgrpport,errno,strerror(errno));
    close( igSockfd );
    exit(FAIL);
  }

  if (listen(igSockfd, SOMAXCONN) == -1)
  {
    _bmqDebug("S0070 �������Ӵ���!!errno: %d,%s",errno,strerror(errno)); 
    close (igSockfd);
    exit(FAIL);
  }

  /*�������pid*/
  psgMbshm->lBmqgrprcvs_pid = getpid();
  
  /*�������鱨�Ĵ洢��Ϣ����*/
  psgMbshm->lMsgid_grp_rcv = msgget(IPC_PRIVATE,IPC_CREAT|0600);
  if (psgMbshm->lMsgid_grp_rcv < 0)
  {
    _bmqDebug("S0075 �������鱨�Ĵ洢��Ϣ���д��� msgget errno:%d:%s",errno,strerror(errno));
    close (igSockfd);
    exit(FAIL);
  }

  _bmqVdebug(1,"get msg id=[%ld]",psgMbshm->lMsgid_grp_rcv);
  
  /*��ʼ���߳�����*/
  ilRc = pthread_attr_init(&attr);
  if ( ilRc==-1 ){
     _bmqDebug("S0080 pthread_attr_init error![%d]\n",errno);
     close (igSockfd);
     exit(-1);
  }
  ilRc = pthread_attr_setstacksize(&attr,56+16*10);
  if ( ilRc == -1 ){
     _bmqDebug("S0090 pthread_attr_setstacksize error![%d]",errno);
     close (igSockfd);
     exit(-1);
  }

  pthread_setconcurrency(2*DEFINE_THREADS +2);
  ilRc = pthread_mutex_init(&connsock_mutex,NULL);
  if ( ilRc==-1 ){
     _bmqDebug("S0100 ���ù�����ʧ��![%d]",errno);
     close (igSockfd);
     exit(-1);
  }
    
  ilRc = pthread_mutex_init(&server_lock,NULL);
  if ( ilRc==-1 ){
     _bmqDebug("S0110 ���÷�������ʧ��![%d]",errno);
     close (igSockfd);
     exit(-1);
  }

  /*Create server thread to accept connect*/
  for (i=0;i<DEFINE_THREADS ;i++){
     ilRc = pthread_create(&server_thread[i],&attr,server_proc,NULL);
     _bmqDebug( "S0120 SERVER THREAD[%7d] CREATED",server_thread[i]);
     if (ilRc )
     	{
        _bmqDebug("S0130 Create server process thread Error:%d",ilRc);
        close (igSockfd);
        exit(0);
     }
  }

  for (i=0;i<DEFINE_THREADS ;i++)
     pthread_join(server_thread[i],NULL);

  while(1) pause();
  
}

/**************************************************************
 ** ������      :   server_proc()
 ** ��  ��      :   SERVER�̴߳�����
 ** ��  ��      :
 ** ��������    :   2007/01/28
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :   
 ** ����ֵ      :   ��
***************************************************************/
void * server_proc(void *args)
{
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
  struct linger ilLinger;
  struct sockaddr_in cli_addr;

  int    ilSockfd;
  char   alMacbuf[MAC_LEN];
  char   alMsgbuf_rcv[iMBMAXPACKSIZE];
  char   alMsgbuf[iMBMAXPACKSIZE+100],alVal[16];
  int    ilRc,ilLen,ilLen1;
  char   alHostName[MAXHOSTNAMELEN]; 
  /*short  ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  struct grprecvmsg  slMsgrcv_grp;
  
  pthread_detach(pthread_self());
  
  for( ; ; ) 
  {
    memset((char *)&cli_addr, 0x00, sizeof(struct sockaddr_in));
    
    ilAddrlen = sizeof(struct sockaddr_in);
    ilSockfd  = accept(igSockfd,(struct sockaddr*)&cli_addr, &ilAddrlen);
    if ( ilSockfd == -1)
    {
      _bmqDebug("S0090 %s:׼�����ӳ���,errno:%d,%s",__FILE__,errno,strerror(errno));
      close(ilSockfd);
      continue;
    }	 
    
    memset(alVal,0x00,sizeof(alVal));
    
    ilLinger.l_onoff  =1;
    ilLinger.l_linger =1;
    ilRc=setsockopt(ilSockfd,SOL_SOCKET,SO_LINGER,&ilLinger,
      sizeof(struct linger));
    if ( ilRc == -1 )
    {
      _bmqDebug("S0110 {%s}Unable to set socket option errno:%d,%s",alHostName,errno,strerror(errno));
      close ( ilSockfd );
      continue;
    }    
    
    /* ���������֤ */
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;
    ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf, &ilMsglen) ;
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0120 _bmqGrpRecv error! Retcode=%d Msglen=[%d]",ilRc,ilMsglen);
      close(ilSockfd);
      return NULL;
    }
    memcpy(alVal,alMsgbuf,8);
    ilLen = ntohs(atoi(alVal));
  
    ilMsglen = ilLen - 8;
    ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf+8, &ilMsglen) ;
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0130 _bmqGrpRecv error! Retcode=%d",ilRc);
      close(ilSockfd);
      return NULL;
    }
    
    ilLen1 = 5;
    memcpy(alHostName,alMsgbuf+13,ilMsglen-5);
    _bmqLokiDec(alHostName);
    
    memset(alVal,0x00,sizeof(alVal));
    memcpy(alVal,alMsgbuf+8,5);
    ilRc = _bmqConfig_load("GROUP",atoi(alVal));
    if (ilRc < 0)
    {
      _bmqDebug("S0150 ��[%d]�ڱ�������û�ж���!",atoi(alVal));
      _comTcpSend(ilSockfd,"UNDEF",&ilLen1);
      return NULL;
    }
    _bmqVdebug(2,"S0160 Grpid=[%d],IP=[%s],Port=[%d]", igGroupid,agBmqhostip, igBmqgrpport);
    if(!memcmp(agBmqhostip,alHostName,strlen(agBmqhostip)))
    {
      _comTcpSend(ilSockfd,"AAAAA",&ilLen1);
    }
    else
    {
      _comTcpSend(ilSockfd,"IPERR",&ilLen1);
      _bmqDebug("S0162 IP��ַ����ȷ,rcv=[%s],def=[%s]",alHostName,agBmqhostip);
      return NULL;
    } 
    
    if(igDebug >= 1)
      _bmqDebug("S0170 ��[%d-%s]�������֤�õ�ȷ��!",igGroupid,agBmqhostip);


    /*�ӿͻ��˽��ձ���*/
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;

    /*����8�ֽڱ��ĳ���*/
    ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf, &ilMsglen) ; 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0120 �ӿͻ����ձ��ĳ���!-Retcode=%d Msglen=[%d]", ilRc,ilMsglen);
      close ( ilSockfd );
      continue;
    }

    memcpy(alVal,alMsgbuf,8);
    ilLen = ntohs(atoi(alVal));

    /*����ָ�����ȵı���*/
    ilMsglen = ilLen - 8;
    ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf+8, &ilMsglen) ;
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0130 �ӿͻ���{%s}�ձ��ĳ���!-Retcode=%d", alHostName, ilRc);
      close ( ilSockfd );
      continue;
    }

    if(igDebug >= 1)
    {
      _bmqDebug("S0140 �ӿͻ���{%s}���ձ������!!--ilMsglen=%d order=%d",
        alHostName, ilMsglen + 8 ,alMsgbuf[8]);
    }

    memset(alVal,0x00,sizeof(alVal));

    /*���*/
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

    memset(alMsgbuf_rcv,0x00,sizeof(alMsgbuf_rcv));
    ilMsglen = ilLen-sizeof(struct grprecvmsg)-8-MAC_LEN;
    memcpy(alMsgbuf_rcv,alMsgbuf+8+sizeof(struct grprecvmsg),ilMsglen);
    _bmqMac(alMsgbuf_rcv,ilMsglen,alMacbuf);
    if(memcmp(alMacbuf,alMsgbuf+ilLen-MAC_LEN,MAC_LEN)) { /*MAC ERROR!*/
      _bmqDebug("S0091 Recv Msg MAC ERROR!");
      ilLen1 = 5;
      _comTcpSend(ilSockfd,"MACER",&ilLen1);  
      close ( ilSockfd );
      continue;
    }
    if(igDebug >= 3)
    {
      _bmqDebug("S0100 �յ����鱨������:");
      _bmqDebughex(alMsgbuf_rcv,ilLen);
    }

    pthread_mutex_lock(&connsock_mutex); 
    
    /*���ͱ��ĵ����鱨�Ĵ洢��Ϣ����*/
    memset(&sgMsgbuf,0x00,sizeof(sgMsgbuf));
    memcpy(&sgMsgbuf.rcvmsg,&slMsgrcv_grp,sizeof(struct grprecvmsg));
    sgMsgbuf.mtype  = 99;
    sgMsgbuf.msglen = ilMsglen;
    memcpy(sgMsgbuf.data,alMsgbuf_rcv,ilMsglen);
    ilRc = msgsnd(psgMbshm->lMsgid_grp_rcv,&sgMsgbuf,sizeof(struct Msgbuf),0);
    if (ilRc < 0) {
      pthread_mutex_unlock(&connsock_mutex);
      _bmqDebug("S0110 д���ĵ����鱨�Ĵ洢��Ϣ���г���errno=[%d],qid=[%ld]",errno,psgMbshm->lMsgid_grp_rcv);
      ilLen1 = 5;
      _comTcpSend(ilSockfd,"SNDER",&ilLen1);     
      continue;
    }
    
    pthread_mutex_unlock(&connsock_mutex);
    _bmqVdebug(2,"S0120 д���ĵ����鱨�Ĵ洢��Ϣ����[%ld]�ɹ�",psgMbshm->lMsgid_grp_rcv);
    ilLen1 = 5;
    _comTcpSend(ilSockfd,"AAAAA",&ilLen1);     
    close ( ilSockfd );
  }
}
