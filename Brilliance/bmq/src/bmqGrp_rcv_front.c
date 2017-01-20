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

/*
static struct sockaddr_in sgSvr_addr;
static struct sockaddr_in sgCli_addr;

static int igSockfd;
static  long    lgSndpid[1024];
*/

/*#define MSGMAXLEN 4096*/  /*����ĳ���*/
#define MSGMAXLEN 7168

int   q_reqid;
char  aReqid[10];

struct Msgbuf{
long mtype;
long msglen;
char data[MSGMAXLEN+100];
}sMsgbuf;


/*
static  void  _bmqQuit();
static int _bmqGrpprocess(  );
static int _bmqGrpRecv( int sockfd,char *buffer, short *length);
*/
static int  GetProfileString();

int main(int argc,char **argv)
{
  int ilRc,ilLen;

  char alcfname[100];
  char   alMsgbuf[MSGMAXLEN+100];
  short  ilPrior,ilMbid;
  short  ilGrpid;
  char alFilter[iMASKLEN];
  struct grprecvmsg  slMsgrcv_grp;
  char aMemo[20];  
  char agrprecvmsgbuf[49];
  
  int ilGroupid;
  short igGrpidtmp;
  
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
  /*signal(SIGTERM,_bmqQuit);	*/
  
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
   
   while( msgrcv(q_reqid,(char *)&sMsgbuf,\
     sizeof(sMsgbuf),0,IPC_NOWAIT) >=0  );

   _bmqDebug("����Ϣ���гɹ�! aReqid[%s],reqid=[%d]",aReqid,q_reqid);

	ilGroupid=atoi(getenv("BMQ_GROUP_ID"));
	_bmqDebug("��ǰ���Ϊ[%d]",ilGroupid);
	
	igGrpidtmp=(unsigned   short)ilGroupid;
	_bmqDebug("��ǰ���Ϊ[%d]",igGrpidtmp);
  for( ; ; )
  {
    memset(&sMsgbuf,0x00,sizeof(sMsgbuf));
    memset(&slMsgrcv_grp,0x00,sizeof(slMsgrcv_grp));
    
    sMsgbuf.mtype=10;
    
    /* ������ͨ������ */
    ilRc = msgrcv (q_reqid, (char *)&sMsgbuf,sizeof(sMsgbuf),0,0);
    if (ilRc < 0 ) {
       _bmqDebug("��ͨѶ�������[%d] ,errno:%d,q_reqid[%d]", ilRc,errno,q_reqid );
       /*exit(0);*/
       continue;
    }
	if( igDebug >= 2) _bmqDebughex(sMsgbuf.data,sMsgbuf.msglen);
    
    memset(alMsgbuf,0x00,sizeof(alMsgbuf));
    memcpy(alMsgbuf,sMsgbuf.data,sizeof(sMsgbuf.data));
    ilLen=sMsgbuf.msglen;
    
    
	if( igDebug >= 2) _bmqDebug("S0120 �ӿͻ����ձ���!sMsgbuf.msglen=[%d],ilLen[%d],sMsgbuf.data[%s]",sMsgbuf.msglen,ilLen,sMsgbuf.data);    
    /*���*/
    int test=1;
    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));
    memcpy(agrprecvmsgbuf, alMsgbuf+8, 8);
	  slMsgrcv_grp.mtype=atol(agrprecvmsgbuf);
	   _bmqDebug("mtype[%ld]", slMsgrcv_grp.mtype); 

    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));	   
	  memcpy(agrprecvmsgbuf, alMsgbuf+16, 8);
	  slMsgrcv_grp.lPack_head =atol(agrprecvmsgbuf);
	   _bmqDebug("lPack_head[%ld]", slMsgrcv_grp.lPack_head); 
	   
    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));	   
	  memcpy(agrprecvmsgbuf, alMsgbuf+24, 8);
	  slMsgrcv_grp.iOrg_group =atoi(agrprecvmsgbuf);
	   _bmqDebug("iOrg_group[%d]", slMsgrcv_grp.iOrg_group); 
	   
    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));	      
	  memcpy(agrprecvmsgbuf, alMsgbuf+32, 8);
	  slMsgrcv_grp.iOrg_mailbox =atoi(agrprecvmsgbuf);
	   _bmqDebug("iOrg_mailbox[%d]", slMsgrcv_grp.iOrg_mailbox); 
	   
    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));	      
	  memcpy(agrprecvmsgbuf, alMsgbuf+40, 8);
	  slMsgrcv_grp.iPrior =atoi(agrprecvmsgbuf);
	   _bmqDebug("iPrior[%d]", slMsgrcv_grp.iPrior); 
	   
    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));	      
	  memcpy(slMsgrcv_grp.aFilter, alMsgbuf+48, 20);
	   _bmqDebug("aFilter[%s]", slMsgrcv_grp.aFilter); 
	   
    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));	      
	  memcpy(agrprecvmsgbuf, alMsgbuf+68, 8);
	  slMsgrcv_grp.iDes_group =atoi(agrprecvmsgbuf);
	   _bmqDebug("iDes_group[%d]", slMsgrcv_grp.iDes_group); 

    memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));	      
	  memcpy(agrprecvmsgbuf, alMsgbuf+76, 8);
	  slMsgrcv_grp.iDes_mailbox =atoi(agrprecvmsgbuf);
	   _bmqDebug("iDes_mailbox[%d]", slMsgrcv_grp.iDes_mailbox); 

    /*memcpy(&slMsgrcv_grp+8, alMsgbuf, sizeof(struct grprecvmsg)-8);
  
   ilRc=sizeof(struct grprecvmsg)-8;
   _bmqDebug("ztq :sizeof(struct grprecvmsg)[%d]sizeof(long)[%d]sizeof(short)[%d]",sizeof(struct grprecvmsg),sizeof(long),sizeof(short) );
   
   _bmqDebughex(&slMsgrcv_grp+8,ilRc);*/


    /************* add by xuchengyong 2002/10/18   *********/
    /*slMsgrcv_grp.mtype = ntohl(slMsgrcv_grp.mtype);    
    
    slMsgrcv_grp.lPack_head = ntohl(slMsgrcv_grp.lPack_head);   
    
    slMsgrcv_grp.iOrg_group = ntohs(slMsgrcv_grp.iOrg_group);    
    
    slMsgrcv_grp.iOrg_mailbox = ntohs(slMsgrcv_grp.iOrg_mailbox);
    
    slMsgrcv_grp.iPrior = ntohs(slMsgrcv_grp.iPrior);
    slMsgrcv_grp.iDes_group = ntohs(slMsgrcv_grp.iDes_group);
    slMsgrcv_grp.iDes_mailbox = ntohs(slMsgrcv_grp.iDes_mailbox);*/ 
    /************* add by xuchengyong 2002/10/18   *********/
/*add by pc 20060221 ��Ŀ����Ž����޸�*/
	if (slMsgrcv_grp.iDes_group==9999){
		slMsgrcv_grp.iDes_group=0;
		slMsgrcv_grp.iOrg_group=0;
		slMsgrcv_grp.iOrg_group=igGrpidtmp;
		
	}
/*add by pc 20060221 ��Ŀ����Ž����޸�*/	

    ilGrpid = slMsgrcv_grp.iDes_group;
    ilMbid  = slMsgrcv_grp.iDes_mailbox;

    ilPrior = slMsgrcv_grp.iPrior;
    memcpy(alFilter,slMsgrcv_grp.aFilter,iMASKLEN);
    if( igDebug >= 2) { 
		_bmqDebug("slMsgrcv_grp.mtype[%d] slMsgrcv_grp.lPack_head[%d]",slMsgrcv_grp.mtype,slMsgrcv_grp.lPack_head );  
		_bmqDebug("slMsgrcv_grp.iOrg_group[%d] slMsgrcv_grp.iOrg_mailbox[%d]",slMsgrcv_grp.iOrg_group,slMsgrcv_grp.iOrg_mailbox );  
		_bmqDebug("slMsgrcv_grp.iPrior[%d] slMsgrcv_grp.iDes_group[%d]",slMsgrcv_grp.iPrior,slMsgrcv_grp.iDes_group );  
		_bmqDebug("slMsgrcv_grp.iDes_mailbox[%d] slMsgrcv_grp.aFilter[%s]",slMsgrcv_grp.iDes_mailbox,slMsgrcv_grp.aFilter );  
	}
	
    /*���Ĵ���*/
    igMb_sys_current_group   = slMsgrcv_grp.iOrg_group;
 
    igMb_sys_current_mailbox = slMsgrcv_grp.iOrg_mailbox;
      
    igMbopenflag = 1;  
    lgCurrent_pid = getpid();

    if( igDebug >= 2) { 
    	_bmqDebug("add by pc ilGrpid=[%d] ilPrior=[%d]",ilGrpid,ilPrior);
  	}
  	
    if(test==1){
     ilRc=ilLen-8-76;
    ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter,alMsgbuf+8+76,ilRc );     
    }
     else {
     	ilRc=ilLen-sizeof(struct grprecvmsg)-8;
     	_bmqDebughex(alMsgbuf+8+sizeof(struct grprecvmsg),ilRc);
    ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter, \
      alMsgbuf+8+sizeof(struct grprecvmsg),ilLen-sizeof(struct grprecvmsg)-8 );  	
    }
    
    /*ilRc = bmqPutfilter(ilGrpid,ilMbid,ilPrior,alFilter, \
      alMsgbuf+8+sizeof(struct grprecvmsg),ilLen-sizeof(struct grprecvmsg)-8 );*/

    if(ilRc)
    {
      _bmqDebug("S0101 ���Ĵ���[%d][%d]����!",ilGrpid,ilMbid);
    }
  }
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
