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
#include <pthread.h>
#define MAX_THREADS 200
#define DEFINE_THREADS 2
/*#define MSGMAXLEN 4096*/  /*����ĳ���*/
#define MSGMAXLEN 7168

int   q_reqid;
char  aReqid[10];
int   thread_nums ;
int   rcv_thread_nums;
int   snd_thread_nums;
static struct sockaddr_in sgSvr_addr;
static struct sockaddr_in sgCli_addr;
static int igSockfd;
static  long    lgSndpid[1024];

pthread_t send_file_thread;
pthread_t recv_file_thread;
pthread_t check_timeout_thread;
pthread_t server_thread[MAX_THREADS],client_thread[MAX_THREADS];
pthread_mutex_t  connsock_mutex,server_lock ;

struct Msgbuf{
long mtype;
long msglen;
char data[MSGMAXLEN+100];
}sMsgbuf;

/* �������� */
void *check_timeout(void *);
void *server_proc(void *);
static  void  _bmqQuit();
/*static int _bmqGrpRecv( int sockfd,char *buffer, short *length); delete by wh*/
static int _bmqGrpRecv( int sockfd,char *buffer, TYPE_SWITCH *length);
static int GetProfileString();

int main(int argc,char **argv)
{
  int     j,ilRc,ilLen;
  int    ilOpt;
  char aIp[32];
  int iPort;
  char alcfname[100];
  pthread_attr_t attr;
  
  
  thread_nums  = 0;

  /* default thread numbers */
  if (thread_nums  == 0)
     thread_nums = DEFINE_THREADS;
  else if (thread_nums > MAX_THREADS)
     thread_nums = MAX_THREADS;

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
  /*
  signal(SIGTERM,_bmqQuit);
  */
  
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

  sprintf(alcfname,"%s%s",getenv("PT_DIR"),"/etc/bmqgrp.ini");
  if (GetProfileString(alcfname,"BMQGRP","REQID",aReqid) < 0)
  {
     _bmqDebug("������ͨѶ��Ϣ���м�ֵ����![%d]",errno);
     exit(-1);
  }

  _bmqDebug("������м���   aReqid = [%s]", aReqid);

  /*������Ϣ����*/
  q_reqid = msgget(atoi(aReqid),IPC_CREAT|0660);
  if (q_reqid < 0){
     _bmqDebug("Resp msgget() error:[%d], ͨѶ����[%s]!\n",errno,aReqid);
     exit(-1);
  }

  _bmqDebug("����Ϣ���гɹ�! reqid=[%d]",q_reqid);
  	

  /*�������pid*/
  /*
  psgMbshm->lBmqgrprcv_pid = getpid();
  
  for(i=0;i<1024;i++) lgSndpid[i] = 0;
  i = 0;
  _bmqDebug("S0080 ***���鼶�� �����ػ����� listen[%d]***",igBmqgrpport);
  */
  
  
  ilRc = pthread_attr_init(&attr);
  if ( ilRc==-1 ){
     _bmqDebug("pthread_attr_init error![%d]\n",errno);
     exit(-1);
  }
  ilRc = pthread_attr_setstacksize(&attr,56+16*10);
  if ( ilRc == -1 ){
     _bmqDebug("pthread_attr_setstacksize error![%d]",errno);
     exit(-1);
  }


  pthread_setconcurrency(2*thread_nums +2);
  ilRc = pthread_mutex_init(&connsock_mutex,NULL);
  if ( ilRc==-1 ){
     _bmqDebug("���ù�����ʧ��![%d]",errno);
     exit(-1);
  }
    
  ilRc = pthread_mutex_init(&server_lock,NULL);
  if ( ilRc==-1 ){
     _bmqDebug("���÷�������ʧ��![%d]",errno);
     exit(-1);
  }


  /*Create the thread to check connect socket timeout*/
  /*
  rc = pthread_create(&check_timeout_thread,&attr,check_timeout,NULL);
  if (rc){
     _bmqDebug("Check_timeout_thread created error,rc=%d",rc);
     exit(-1);
  }
  swVdebug(4,"thread_nums=%d",2*thread_nums+1 );
  

  (void)	InitStruct();
  */

  /*Create server thread to accept connect*/
  for (j=0;j<thread_nums ;j++){
     ilRc = pthread_create(&server_thread[j],&attr,server_proc,NULL);
     _bmqDebug( "SERVER THREAD[%7d] CREATED",server_thread[j]);
     if (ilRc ){
        _bmqDebug("Create server process thread Error:%d",ilRc);
        exit(0);
     }
  }

  /*pthread_join(check_timeout_thread,NULL);	*/
  for (j=0;j<thread_nums ;j++)
     pthread_join(server_thread[j],NULL);

  while(1) pause();  

}


/**************************************************************
 ** ������      :   serverproc()
 ** ��  ��      :   SERVER�̴߳�����
 ** ��  ��      :
 ** ��������    :   2001/09/03
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :   Conn:       ���룺�ڲ��ṹ
 ** ����ֵ      :   ��
***************************************************************/

void * server_proc(void *args)
{
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif
   char   alMsgbuf[MSGMAXLEN+100],alVal[16];
   struct sockaddr_in cli_addr;
   int    ilRc;
   long   ilSockfd;
   /*short  ilMsglen; delete by wh*/
   TYPE_SWITCH ilMsglen; /*add by wh*/
   long	  ilLen;
   struct linger ilLinger;
   char   alCliaddr[21];                 /* �ͻ�������ַ */


   pthread_t my_pthread_id;

   my_pthread_id = pthread_self();
   pthread_detach(pthread_self());

   for( ; ; ) {
     memset((char *)&cli_addr, 0x00, sizeof(struct sockaddr_in));
    
     ilAddrlen = sizeof(struct sockaddr_in);
     ilSockfd  = accept(igSockfd,(struct sockaddr*)&cli_addr, &ilAddrlen);
     if ( ilSockfd == -1)
     {
       _bmqDebug("S0090 %s:׼�����ӳ���,errno:%d,%s",__FILE__,errno,strerror(errno));
       
       /*** zcb add 2005-08-08 ***/
       close(ilSockfd);
       continue;
       
       /*** zcb delete 2005-08-08      
       return (-1010);
       ***/
     }	
     
     /*��öԷ�������*/
     memset(alCliaddr, 0x00, sizeof(alCliaddr));
     sprintf(alCliaddr, "%s", inet_ntoa(cli_addr.sin_addr));
     
     _bmqDebug("S0100 �յ�[%s]����Ϣ",alCliaddr);
     /*
     hp = gethostbyaddr( (char *) &sgCli_addr.sin_addr, sizeof(struct in_addr),
        sgCli_addr.sin_family);
     if( hp == NULL)
       strcpy( alCliname, inet_ntoa( sgCli_addr.sin_addr ) );
     else
       strcpy( alCliname, hp->h_name );
       */
     
     ilLinger.l_onoff  =1;
     ilLinger.l_linger =0;
     ilRc=setsockopt(ilSockfd,SOL_SOCKET,SO_LINGER,&ilLinger,sizeof(struct linger));
     if ( ilRc == -1 )
     {
       _bmqDebug("S0110 {%s}Unable to set socket option errno:%d,%s",alCliaddr,errno,strerror(errno));
       close ( ilSockfd );
       exit(-1);
     }
     
     /*�ӿͻ��˽��ձ���*/
     memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
     ilMsglen = 8;
     
     /*����8�ֽڱ��ĳ���*/
     ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf, &ilMsglen) ; 
     if ( ilRc != 0 || ilMsglen <= 0)
     {
       close ( ilSockfd );     	
       _bmqDebug("S0120 �ӿͻ���{%s}�ձ��ĳ���!-Retcode=%d Msglen=[%d]", alCliaddr, ilRc,ilMsglen);
       continue;
     }

     memset(alVal,0x00,sizeof(alVal));
     memcpy(alVal,alMsgbuf,8);
     /************* delete by xuchengyong 2002/10/18   *********/
     ilLen = atoi(alVal);
     /************* delete by xuchengyong 2002/10/18   *********/
     /************* add by xuchengyong 2002/10/18   *********
     ilLen = ntohs(atoi(alVal));
     ************* add by xuchengyong 2002/10/18   *********/
		if( igDebug >= 2) _bmqDebug("S0120 �ӿͻ����ձ���!len=[%d]",ilLen);
     
     /*����ָ�����ȵı���*/
     ilMsglen = ilLen - 8;
     
     ilRc = _bmqGrpRecv(ilSockfd, alMsgbuf+8, &ilMsglen);

     if ( ilRc != 0 || ilMsglen <= 0)
     {
       close ( ilSockfd );
       _bmqDebug("S0130 �ӿͻ���{%s}�ձ��ĳ���!-Retcode=%d", alCliaddr, ilRc);
       continue;
     }
     
      _bmqDebug("S02 ztq�ӿͻ����ձ���[%s]",alMsgbuf+8);
      _bmqDebughex(alMsgbuf+8,ilRc);
     
     close( ilSockfd );
 
    
	 pthread_mutex_lock(&connsock_mutex); 
	   	     
     memset(&sMsgbuf,0x00,sizeof(sMsgbuf));
     sMsgbuf.msglen=ilLen;
     memcpy(sMsgbuf.data,alMsgbuf,ilLen);
     sMsgbuf.mtype=10;
     
	 if( igDebug >= 2) _bmqDebug("S0120 �ӿͻ����ձ���!sMsgbuf.msglen=[%d]",sMsgbuf.msglen);
	 if( igDebug >= 2) _bmqDebug("S0120 �ӿͻ����ձ���!alMsgbuf=[%s]",alMsgbuf);
	 if( igDebug >= 2) _bmqDebug("S0120 ���ͷ�ʽ!alMsgbuf=[%s]",alMsgbuf+sMsgbuf.msglen-2);
	 if( igDebug >= 2) _bmqDebug("S0120 �ӿͻ����ձ���!ilMsglen=[%d]",ilMsglen);     
    
     ilRc=sizeof(struct Msgbuf);
     _bmqDebug("S0 ztq msgsnd len[%d]",ilRc);        
     _bmqDebughex(&sMsgbuf,ilRc);
     /*���ͱ��ĵ�ǰ����Ϣ����*/
     ilRc = msgsnd(q_reqid,&sMsgbuf,sizeof(struct Msgbuf),0);
     if (ilRc < 0) {
        pthread_mutex_unlock(&connsock_mutex);
        _bmqDebug("д���ĵ�ǰ��Q[%d]����[%d]ilRc=[%d] aReqid=[%s]", q_reqid,errno,ilRc,aReqid);
        continue;
     }
     pthread_mutex_unlock(&connsock_mutex);
     
	if( igDebug >= 2) _bmqDebughex(sMsgbuf.data,sMsgbuf.msglen);
	
     if( igDebug >= 2) _bmqDebug("���ͱ��ĵ�ǰ��Q[%d]�ɹ�", q_reqid);
   }
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
  delete by wh */
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
    _bmqDebug("S03 ztq: _bmqGrpRecv len:%d",len);
    len=0;
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

/****************************************************************/
/* ģ����    ��GetProfileString                               */
/* ģ������    ��������ȡ                                       */
/* �� �� ��    ��                                               */
/* ��    ��    ��Li Yongguang                                   */
/* ��������    ��2006-02-07                                     */
/* ����޸����ڣ�2006-02-09	                                */
/* ģ����;    �����ļ�������                                   */
/****************************************************************/

int GetProfileString(FileName,Section,Index,GetValue)
char *FileName,*Section,*Index,*GetValue;
{

	FILE *fp;
	int iSectionLen,iIndexLen,iFind;
	char buff[512];
	int i;

	iSectionLen=strlen(Section);
	iIndexLen=strlen(Index);

	/* �������ļ� */

	if ( ( fp = fopen (FileName,"r") ) == NULL )
		return (-1);

	/* Ѱ��ƥ������ */

	iFind =-2;
	while( fgets(buff,sizeof(buff) - 1,fp)) {
		if ( *buff == '#' || *buff == ';' )
			continue;

		if ( (*buff=='[') && (*(buff+iSectionLen+1)==']') && \
		    (!memcmp(buff+1,Section,iSectionLen)) ) {
			iFind =0;
			break;
		}
	}
	if (iFind) {
		fclose(fp);
		return ( -2 );
	}

	/* Ѱ��ƥ������ */

	iFind =-3;

	while( fgets(buff,sizeof(buff),fp)) {

		if (*buff=='[' )
			break;

		if (*buff=='#'||*buff==';')
			continue;

		if ( (*buff==*Index) && ( *(buff+iIndexLen)=='=') && \
			(!memcmp(buff,Index,iIndexLen)) ) {

			for (i = iIndexLen;i<= strlen(buff);i++){
				/*
				if (buff[i] == ';'|| buff[i] == '#'|| buff[i]=='\n'){
				*/
				if (buff[i] == ' ' || buff[i] == '\n'){
					buff[i] = 0x00;
					break;
				}
			}

			strcpy( GetValue,buff+iIndexLen+1);
			iFind = strlen(GetValue) ;
			GetValue[iFind]=0;
			break;
		}
	}
	fclose(fp);
	return (iFind);
}
