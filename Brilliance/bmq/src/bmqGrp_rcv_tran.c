/****************************************************************/
/* ģ����    ��BMQGRP_RCV                                     */
/* ģ������    �����鼶������ģ��                               */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���³�Q ���鼶�������ػ�����                     */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2001.08.17 ���V2.0,����307��                              */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"

static struct sockaddr_in sgSvr_addr;
static struct sockaddr_in sgCli_addr;
static int igSockfd;
static  long    lgSndpid[1024];

static  void  _bmqQuit();
static int _bmqGrpprocess(  );
/*static int _bmqGrpRecv( int sockfd,char *buffer, short *length); delete by wh*/
static int _bmqGrpRecv( int sockfd,char *buffer, TYPE_SWITCH *length); /*add by wh*/

int main(int argc,char **argv)
{
  int     i,ilRc,ilLen,ilSockfd;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
  int    ilOpt;
  char aIp[32];
  int iPort;
  
  /*��ʾ�³�Q�汾��Ϣ*/
  _bmqShowversion(argc,argv);

  /*�����ź�*/
  signal(SIGQUIT,SIG_IGN);
  /*** zcb add 2005-08-08 ***/
  signal(SIGTTOU,SIG_IGN);
  signal(SIGTTIN,SIG_IGN);
  
  signal(SIGHUP,SIG_IGN);
  signal(SIGINT,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGSTOP,SIG_IGN);
  signal(SIGUSR1,SIG_IGN);
  signal(SIGUSR2,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
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

  /*ȡ�������IP��ַ�Ͷ˿ں�*/
  ilRc = _bmqConfig_load("GROUP",psgMbshm->iMbgrpid);
  if (ilRc < 0)
  {  
    _bmqDebug("S0040 %s :����GROUP[%d]����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,psgMbshm->iMbgrpid,getenv("BMQ_PATH"));
    exit(FAIL);
  }
  memset(&sgSvr_addr,0x00,sizeof(struct sockaddr_in));
  memset(&sgCli_addr,0x00,sizeof(struct sockaddr_in));
  
  sgSvr_addr.sin_family = AF_INET;
  /*add by pc 20060302 �޸�Ϊ����ָ��IP��ַ�Ͷ˿ں�*/
  if (argc==1) {
  sgSvr_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  sgSvr_addr.sin_port = htons(igBmqgrpport);  	
  }	
  else if (argc==2) {
	strcpy(aIp,argv[1]);
	sgSvr_addr.sin_addr.s_addr = inet_addr(aIp);
	sgSvr_addr.sin_port = htons(igBmqgrpport);    	
  }
  else if (argc==3) {
	strcpy(aIp,argv[1]);
    iPort=atoi(argv[2]);	
	sgSvr_addr.sin_addr.s_addr = inet_addr(aIp);
    sgSvr_addr.sin_port = htons(iPort);
    igBmqgrpport=iPort;
  }		


  /*����SOCKET*/
  ilSockfd = socket (AF_INET, SOCK_STREAM, 0);
  if (ilSockfd == -1)
  {
    _bmqDebug("S0050 ����SOCKET����!errno: %d,%s",errno,strerror(errno));
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

  /*��SOCKET*/
  if (bind(ilSockfd,(struct sockaddr*)&sgSvr_addr,sizeof(struct sockaddr_in))
    == -1)
  {
    _bmqDebug("S0060 ��SOCKET���˿�[%d]����!! errno: %d,%s",igBmqgrpport,errno,strerror(errno));
    close( ilSockfd );
    exit(FAIL);
  }

  if (listen(ilSockfd, SOMAXCONN) == -1)
  {
    _bmqDebug("S0070 �������Ӵ���!!errno: %d,%s",errno,strerror(errno)); 
    close (ilSockfd);
    exit(FAIL);
  }

  /*�������pid*/
  psgMbshm->lBmqgrprcv_pid = getpid();

  for(i=0;i<1024;i++) lgSndpid[i] = 0;
  i = 0;
  _bmqDebug("S0080 ***���鼶�� �����ػ����� listen[%d]***",igBmqgrpport);

  for( ; ; )
  {
    /*** zcb add 2005-03-17 ***/
    if(i == 1023) 
    {
      i = 0;	
    }
    
    /*** zcb add 2005-08-08 ***/
    memset((char *)&sgCli_addr, 0x00, sizeof(struct sockaddr_in));
    
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(ilSockfd,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      _bmqDebug("S0090 %s:׼�����ӳ���,errno:%d,%s",__FILE__,errno,strerror(errno));
      
      /*** zcb add 2005-08-08 ***/
      close(igSockfd);
      continue;
      
      /*** zcb delete 2005-08-08      
      return (-1010);
      ***/
    }
 
    switch((lgSndpid[i] = fork()))
    {
      case -1:
        /*** zcb delete 2005-08-08
        close( ilSockfd );
        ***/
        close( igSockfd );  
        exit(FAIL);
      case 0:	
        close( ilSockfd );
        if ( (_bmqGrpprocess()) != 0 )
          _bmqDebug("S0100 �ӽ��̴������!!\n");  
        /*20060323 add by pc Translate �ӳ�30���ٹ�����*/
        sleep(30);  
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
 ** ������: _bmqGrpprocess
 ** ����:   ����ͻ�������
 ** ����:   ���
 ** ��������: 2000/10/26
 ** ����޸�����:2001/08/08
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqGrpprocess()
{
  struct hostent *hp;
  struct linger ilLinger;

  char   alCliname[80];
  char   alMsgbuf[iMBMAXPACKSIZE+100],alVal[16];
  int    ilRc,ilLen;
  short  ilPrior,ilMbid;
  
  char alFilter[iMASKLEN];
  
  short  ilGrpid;/*ilMsglen;*/
  TYPE_SWITCH ilMsglen;  /*add by wh*/
  struct grprecvmsg  slMsgrcv_grp;
	struct 
	{
		char type[2];          /*���ݰ������ͣ�Ŀǰ���ã���չ	*/
		/*unsigned short msglen;   delete by wh���ݰ��ĳ���	*/
		TYPE_SWITCH msglen;  /*���ݰ��ĳ���	*/
		char destaddr[16];      /*Ŀ�ĵص�X.25��ַ	*/
		char souraddr[16];      /*ԭ���ص�X.25��ַ	*/
	}msg_head;
	char aTmp[64];
	int ilLenTmp=0;
  memset(alVal,0x00,sizeof(alVal));
  memset(alFilter,0x00,sizeof(alFilter));
  /*��öԷ�������*/
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
  /**delete by nh 20050121 for kgxt
  for(;;)
  {
  */
    /*�ӿͻ��˽��ձ���*/
    /*���� msg_head ���ֽڣ��ٻ�ִʱ����󷵻�*/
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen=sizeof(msg_head);
    ilRc = _bmqGrpRecv(igSockfd, alMsgbuf, &ilMsglen); 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0120 �ӿͻ���{%s}��Translate����ͷ����!-Retcode=%d Msglen=[%d]", alCliname, ilRc,ilMsglen);
      return(FAIL);
    }
    else {
    	memcpy((char *)&msg_head,alMsgbuf,sizeof(msg_head));
    	/*ʵ�� Դ��Ŀ�� ��λ��*/
    	memset(aTmp,0x00,sizeof(aTmp));
    	memcpy(aTmp,msg_head.destaddr,sizeof(msg_head.destaddr));
    	memcpy(msg_head.destaddr,msg_head.souraddr,sizeof(msg_head.destaddr));
    	memcpy(msg_head.souraddr,aTmp,sizeof(msg_head.souraddr));
    	/*���㳤��*/
    	msg_head.msglen=sizeof(msg_head)+2;	/*����ƴHZ�����ַ�*/
    	ilLenTmp=msg_head.msglen;
    	msg_head.msglen=htons(msg_head.msglen);
    	memset(aTmp,0x00,sizeof(aTmp));
    	memcpy(aTmp,(char *)&msg_head,sizeof(msg_head));
    	memcpy(aTmp+sizeof(msg_head),"HZ",2);
    }	
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;
	
    /*����8�ֽڱ��ĳ���*/
    ilRc = _bmqGrpRecv(igSockfd, alMsgbuf, &ilMsglen) ; 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0120 �ӿͻ���{%s}�ձ��ĳ���!-Retcode=%d Msglen=[%d]", alCliname, ilRc,ilMsglen);
      return(FAIL);
    }
    memcpy(alVal,alMsgbuf,8);
    /************* delete by xuchengyong 2002/10/18   *********
    ilLen = atoi(alVal);
    ************* delete by xuchengyong 2002/10/18   *********/
    /************* add by xuchengyong 2002/10/18   *********/
    ilLen = ntohs(atoi(alVal));
    /************* add by xuchengyong 2002/10/18   *********/

    /*����ָ�����ȵı���*/
    ilMsglen = ilLen - 8;
    ilRc = _bmqGrpRecv(igSockfd, alMsgbuf+8, &ilMsglen);
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0130 �ӿͻ���{%s}�ձ��ĳ���!-Retcode=%d", alCliname, ilRc);
      return(FAIL);
    }

    if(igDebug >= 1)
    {
      _bmqDebug("S0140 �ӿͻ���{%s}����ָ�����ȱ������!!--ilMsglen=[%d] order=[%d]",
        alCliname, ilMsglen + 8 ,alMsgbuf[8]);
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

/*add by pc 20060221 ���ӶԼٻ�ִ�Ĵ���*/
	/*���鱨�� ���2λΪ ���ͷ�ʽ����(1λ) 1 ����ר�� 2����Over 3����Translate +ͨѶ��·���(1λ)*/
	if (slMsgrcv_grp.iPrior==3) {
		/*�����ʹ������Translate ���յı��ģ����ٻ�ִ*/

		ilRc = send(igSockfd,aTmp,ilLenTmp,0);
		if (ilRc<=0) {
			_bmqDebug("S0150 �ٻ�ִӦ��ʧ�� send ilRc(len)[%d] errno[%d]",ilRc,errno);
		}
		else {
			_bmqDebug("S0000 �ٻ�ִӦ��ɹ�");
			if( igDebug >= 2 ) {
			_bmqDebug("���������ٻ�ִ���ݿ�ʼ...ilRc=[%d] errno[%d]",ilRc,errno);
			_bmqDebughex(aTmp,ilLenTmp);
			_bmqDebug("���������ٻ�ִ���ݽ���...");
			}							
		}

	}
/*add by pc 20060221 ���ӶԼٻ�ִ�Ĵ���*/


    slMsgrcv_grp.iDes_group = ntohs(slMsgrcv_grp.iDes_group);
/*add by pc 20060221 ��Ŀ����Ž����޸�*/
	if (slMsgrcv_grp.iDes_group==9999){
		slMsgrcv_grp.iDes_group=0;
	}    
/*add by pc 20060221 ��Ŀ����Ž����޸�*/	
    slMsgrcv_grp.iDes_mailbox = ntohs(slMsgrcv_grp.iDes_mailbox);
    /************* add by xuchengyong 2002/10/18   *********/
    ilGrpid = slMsgrcv_grp.iDes_group;
    ilMbid  = slMsgrcv_grp.iDes_mailbox;

    ilPrior = slMsgrcv_grp.iPrior;
    memcpy(alFilter,slMsgrcv_grp.aFilter,iMASKLEN);
  
    /*���Ĵ���*/
    igMb_sys_current_group   = slMsgrcv_grp.iOrg_group;
    igMb_sys_current_mailbox = slMsgrcv_grp.iOrg_mailbox;
    igMbopenflag = 1;  
    lgCurrent_pid = getpid();
    ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter,
      alMsgbuf+8+sizeof(struct grprecvmsg),ilLen-sizeof(struct grprecvmsg)-8 );
    if(ilRc) 
    {
      _bmqDebug("S0101 ����[%d]����[%d][%d]����!",(ilLen-sizeof(struct grprecvmsg)-8),ilGrpid,ilMbid);
      return(FAIL);;
    }
  /**delete by nh 20050121 for kgxt
  }
  */
  
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqGrpRecv
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
/*static int _bmqGrpRecv( sockfd, buffer, length)
  int     sockfd;
  char *buffer;
  short   *length;
  delete by wh*/
static int _bmqGrpRecv( sockfd, buffer, length)
  int     sockfd;
  char *buffer;
  TYPE_SWITCH *length; /*add by wh*/
{
  /*short len=0,ilRcvlen=0; delete by wh*/
  TYPE_SWITCH len=0,ilRcvlen=0; /*add by wh*/

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

