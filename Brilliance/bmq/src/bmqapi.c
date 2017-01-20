/****************************************************************/
/* ģ����	��BMQAPI						*/
/* ģ������	���³�Q API��					*/
/* �� �� ��	��V2.0						*/
/* ��    ��	��ʷ����						*/
/* ��������	��2000/10/26					*/
/* ����޸�����	��2002/07/26					*/
/* ģ����;	���³�Q��������API����				*/
/* ��ģ���а������º���������˵����				*/
/*       (1)  int _bmqShowversion()             */
/*       2)  int _bmqStartup()			*/
/*			 (3)  int _bmqShutdown()		*/
/*			 (4)  int bmqOpen()			*/
/*			 (5)  int bmqClose()			*/
/*			 (6)  int bmqClearmb()			*/
/*			 (7)  int bmqGetmbinfo()		*/
/*			 (8)  int bmqPut()    			*/
/*			 (9)  int bmqPutfilter()		*/
/*			(10)  int bmqGet()			*/
/*			(11)  int bmqGetfilter()		*/
/*			(12)  int bmqGetw()			*/
/*			(13)  int bmqGetwfilter()		*/
/*			(14)  int _bmqQueue_put()		*/
/*			(15)  int _bmqQueue_get()		*/
/*			(16)  int _bmqQueue_del()		*/
/*			(17)  int _bmqQueue_add()		*/
/*			(18)  int _bmqQueue_sort()		*/
/*			(19)  int _bmqQueueInsert_Before()	*/
/*			(20)  int _bmqQueuefile_put()		*/
/*			(21)  int _bmqQueuefile_set()		*/
/*			(22)  int _bmqQueuefile_pick()		*/
/*			(23)  int _bmqPack_put()		*/
/*			(24)  int _bmqPack_get()		*/
/*			(25)  int _bmqPackfile_put()		*/
/*			(26)  int _bmqPackfile_set()		*/
/*			(27)  int _bmqPackfile_pick()		*/
/*			(28)  int _bmqGetlog()			*/
/*			(29)  int _bmqPutlog()			*/
/*			(30)  int _bmqConnect_add()		*/
/*			(31)  int _bmqConnect_del()		*/
/*			(32)  int _bmqConnect_update()		*/
/*			(33)  int _bmqConn_select()		*/
/*			(34)  int _bmqConnWait_set()		*/
/*			(35)  int _bmqConn_judge()		*/
/*			(36)  int _bmqConnfile_put()		*/
/*			(37)  int _bmqConnfile_set()		*/
/*			(38)  int _bmqConnfile_pick()		*/
/*			(39)  int _bmqOpenfile()		*/
/*			(40)  int _bmqClosefile()		*/
/*			(41)  int _bmqUnlinkfile()		*/
/*			(42)  int _bmqGetfile()			*/
/*			(43)  int _bmqPutfile()			*/
/*			(44)  int _bmqConfig_load()		*/
/*			(45)  int _bmqGrpmsg_snd()		*/
/*			(46)  int _bmqLock()			*/
/*			(47)  int _bmqUnlock()			*/
/*			(48)  int _bmqQuit()			*/
/*			(49)  int _bmqTrim()			*/
/*			(50)  int _bmqShmConnect()		*/
/*			(51)  void _bmqProshut()		*/
/*			(52)  void _bmqTimeout()		*/
/*			(53)  int _bmqDebug()			*/
/*			(54)  int _bmqDebughex()	        */
/*			(55)  int _bmqDebugmbinfo()		*/
/*			(56)  int _bmqDebugmbqueue()		*/
/*			(57)  int _bmqDebugmbpack()		*/
/*			(58)  int _bmqDebugmbconnect()		*/
/*			(59)  int _bmqPrintmbinfo()		*/
/*			(60)  int _bmqPrintmbqueue()		*/
/*			(61)  int _bmqPrintmbpack()		*/
/*			(62)  int _bmqPrintmbconnect()		*/
/*			(63)  int _bmqGetmsgid()		*/
/*                      (64)  int _bmqStrand()                  */
/*                      (65)  int _bmqGetfrommsg()		*/
/*			(66)  int _bmqSendstat()		*/
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��(Maintenance by XuJun)				*/
/* 2001.11.30 ���V2.0,����5054��				*/
/* 2002.07.26 ���ӷ���ͳ�ƹ���,����5103��			*/
/****************************************************************/

/* �⺯������ */
#include	"bmq.h"

/* �������� */
static	int	igFlag;				/* ��־���� */
static	short	igMbinfo;			/* ������Ŀ */
static	long	lgMbqueuecount;			/* ���������Ŀ */
static	long	lgMbpacksize;			/* ���д洢����С */
static	short	igMbcontcount;			/* �������������Ŀ */
static	short	igMbfilecount;			/* ���ͬʱ�����ļ��� */
static	long	lgLogfilesize;			/* ��־�ļ���С */
static	char	*agShmtmp_d;			/* �����ڴ��ַ */
static	int   igMb_sys_shm_id;		/* ��ǰ�����ڴ�id */

#define DEFAULTCONTIME 10 	/*ȱʡ���ӳ�ʱʱ��*/

/*20060220 add by pc*/
char agIp[32];
int igPort;
char agSendIp[16];
int igTranslateFlag;
char agAddMsg[64];
int igTotalTime;
int igTimeOut;
long lgBegintime;
char cgTranType;
int igConnNum=0;
int igAdmitFlag=0;
//int igLineNum;


/* �������� */
/*int		bmqPutfilter(short, short, short, char *, char *, short);delete by wh*/
int		bmqPutfilter(short, short, short, char *, char *, TYPE_SWITCH);/*add by wh*/
/*int		bmqGetwfilter(short *, short *, short *, char *, char *, char *, short *, short);delete by wh*/
int		bmqGetwfilter(short *, short *, short *, char *, char *, char *, TYPE_SWITCH*, short);/*add by wh*/
void		_bmqProshut();
int		_bmqConnect_add(short, long);
int		_bmqConfig_load(char *, short);
int		_bmqConnect_del(short, struct mbconnect *);
int		_bmqConnWait_set(short, char *, char *);
/*int		_bmqPutfile(FILE *, long, short, char *);delete by wh*/
int		_bmqPutfile(FILE *, long, TYPE_SWITCH, char *);/*add by wh*/
/*int		_bmqPackfile_set(FILE *, short, short, long); delete by wh*/
int		_bmqPackfile_set(FILE *, short, TYPE_SWITCH, long); /*add by wh*/
int		_bmqStrand(char *, char *, short, char *);
int		_bmqGetmsgid(short);
int		_bmqGetfrommsg(short, short, long);
int		_bmqQueuefile_set(FILE *, short, long, long, long);
int		_bmqQueue_del(short, struct mbqueue *, short *, short *, short *, char *, long *);
int		_bmqPutindexfile(long ,char *);
int		_bmqGetindexfile(long ,char *);
int              _bmqDelspace(char *alstr);

static	void	_bmqTimeout();
static	int	_bmqGrpmsg_snd(long, short, char *, short, short);
static	int	_bmqConn_select(short, short, char *, long);
static	int	_bmqQueue_put (short, short, short, char *, long, long *);
/*static	int	_bmqPack_put(char *, short, long *);delete by wh*/
static	int	_bmqPack_put(char *, TYPE_SWITCH, long *);/*add by wh*/
static	int	_bmqQueue_get(short *, short *, short *, char *, char *, long *);
static	int	_bmqQueue_javaget(short *, short *, short *, char *, char *, long *);
static	int	_bmqQueue_add(long, short, short, short, char *, long, struct mbqueue *);
static	int	_bmqQueue_sort(short, long, long, long, struct mbqueue *);
static	int	_bmqQueueInsert_Before(short, long, long);
static	int	_bmqConnect_update(short, struct mbconnect *);
/*static	int	_bmqGetlog(char *, short, long, char *);delete by wh*/
static	int	_bmqGetlog(char *, TYPE_SWITCH, long, char *);/*add by wh*/
static	int	_bmqQuit();
static	int	_bmqConn_judge(long);
static	int	_bmqUnlinkfile();
static	int	_bmqQueuefile_put(short, short, short, char *, long, long *);
static	int	_bmqConnfile_put(short, long);
static	int	_bmqConnfile_set(short, short, short, char *, char *, short, short);
/*static	int	_bmqPackfile_put(char *, short, long *); delete by wh*/
static	int	_bmqPackfile_put(char *, TYPE_SWITCH, long *); /*add by wh*/
static	int	_bmqPackfile_pick(FILE *, struct mbfilepack *, long);
static	int	_bmqSendstat(short);

static  int     _bmqGrpcontser(char *alIp,short ilPort,char *aSendIp,long ilTime);
/*static  int     _bmqGrpSend(int sockfd,char * buffer,short *length);delete by wh*/
static  int     _bmqGrpSend(int sockfd,char * buffer,TYPE_SWITCH *length);/*add by wh*/


extern int bmqRecvfileByName();
extern int bmqRecvfileX();
extern int _bmqConnectfileser();
extern int _comTcpSend();
extern int _bmqRecvconf();
extern int _bmqFile_comm(char aParm[][iFLDVALUELEN]);
extern int _bmqFile_msg(char aParm[][iFLDVALUELEN]);
extern int _bmqGetfield(char *,char *,int ,char *);



/**************************************************************
 ** ������:_bmqShowversion
 ** ����: ��ʾ�汾��
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������    :
 ** ����ֵ: SUCCESS
***************************************************************/
int _bmqShowversion(int iArgc,char **wcArgv)
{
  int i;

  for(i=2;i<=iArgc;i++)
  {
    if (strcmp(wcArgv[i-1],"-v")==0)
    {
      printf("BMQ Version 2.0\n");
      exit(0);
    }
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������:_bmqStartup
 ** ����: �����³�Q
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:_bmqConfig_load,_bmqUnlinkfile
 ** ȫ�ֱ���:
 ** ��������    :
 ** ����ֵ: �ɹ���SUCCESS ���󣺴�����
***************************************************************/
int _bmqStartup()
{
  int   i,ilRc;
  long  llSize;
  short ilArray[iSEMNUM];
  char  *alShmtmp_d; 
  char  alFp_name[80];
  char  alPro_name[20];

  /*������������ļ�*/
  ilRc = _bmqConfig_load("BMQ",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0010 %s :����BMQ����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1184);
  }
  ilRc = _bmqConfig_load("SHMK",0); 
  if (ilRc < 0)
  {
    _bmqDebug("S0020 %s :����SHMK����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1185);
  }
 
  /*���㹲���ڴ��С*/
  llSize=sizeof(struct mbshm)+sizeof(struct mbinfo)*igMbinfo \
    +sizeof(struct mbqueue)*lgMbqueuecount \
    +sizeof(struct mbpack)*lgMbpacksize \
    +sizeof(struct mbconnect)*igMbcontcount \
    +sizeof(struct monmsg)*igMbfilecount \
    +sizeof(struct mbrouter)*iMAXROUTEREC;

   /*���������ڴ�*/
  ilRc = shmget(lgSHMKEY_MB,llSize,IPC_CREAT|IPC_EXCL|0600);
  if (ilRc < 0)
  {
    if(errno == EEXIST) return(1000);
    _bmqDebug("S0030 ���������ڴ����,errno:%d:%s",errno,strerror(errno));
    return(-1150);
  }
  igMb_sys_shm_id = ilRc;


  /*ȡ�ù����ڴ��ַ*/
  alShmtmp_d = shmat(igMb_sys_shm_id,NULL,0);/*NULL--ϵͳ�Ƽ�*/
  if (alShmtmp_d == (char *)-1)
  {
    _bmqDebug("S0040 ȡ�ù����ڴ��ַ����shmat errno:%d:%s",errno,strerror(errno));
    ilRc = shmctl(igMb_sys_shm_id,IPC_RMID,0);
    if(ilRc < 0)
    {
      _bmqDebug("S0050 ɾ�������ڴ���� shmctl(RM) errno:%d:%s",errno,strerror(errno));
      return(-1153);
    }
    return(-1151);
  }

  /*��ʼ�������ڴ�ṹ*/
  memset(alShmtmp_d,0x00,llSize);
  psgMbshm = (struct mbshm *)alShmtmp_d;  
  psgMbshm->iMbinfo          = igMbinfo;
  psgMbshm->lMbqueuecount    = lgMbqueuecount;
  psgMbshm->lMbpacksize      = lgMbpacksize;
  psgMbshm->iMbcontcount     = igMbcontcount;
  psgMbshm->iFilecount       = igMbfilecount;
  psgMbshm->lLogfilesize     = lgLogfilesize;
  psgMbshm->iRouternum       = 0;
 
  /*�������ƫ��*/
  psgMbshm->lMbqueueoffset   =  sizeof(struct mbinfo)*psgMbshm->iMbinfo;
  psgMbshm->lMbpackoffset    =  psgMbshm->lMbqueueoffset
    + sizeof(struct mbqueue)*psgMbshm->lMbqueuecount;
  psgMbshm->lMbconnectoffset =  psgMbshm->lMbpackoffset 
    + sizeof(struct mbpack)*psgMbshm->lMbpacksize;
  psgMbshm->lFileoffset      =  psgMbshm->lMbconnectoffset 
    + sizeof(struct mbconnect)*psgMbshm->iMbcontcount;
  psgMbshm->lMbrouteroffset  =  psgMbshm->lFileoffset 
    + sizeof(struct monmsg)*psgMbshm->iFilecount;

  /*���������ڴ��������*/
  psgMbinfo    = (struct mbinfo *)(alShmtmp_d + sizeof(struct mbshm));
  psgMbqueue   = (struct mbqueue *)(alShmtmp_d + sizeof(struct mbshm) 
    + psgMbshm->lMbqueueoffset);
  psgMbpack    = (struct mbpack *)(alShmtmp_d + sizeof(struct mbshm) 
    + psgMbshm->lMbpackoffset);
  psgMbconnect = (struct mbconnect *)(alShmtmp_d + sizeof(struct mbshm) 
    + psgMbshm->lMbconnectoffset);
  psgMbfilemng = (struct monmsg *)(alShmtmp_d + sizeof(struct mbshm)
    + psgMbshm->lFileoffset);
  psgMbrouter  = (struct mbrouter *)(alShmtmp_d + sizeof(struct mbshm)
    + psgMbshm->lMbrouteroffset);

  for(i=0;i<psgMbshm->iMbinfo;i++)
    psgMbinfo[i].iFlag = 1;

  for(i=0;i<psgMbshm->iFilecount;i++)
    psgMbfilemng[i].iFlag = 0;

  /* ���붨���·��������Ϣ(���Բ����������ļ�����Ϣ���ʲ��Ը���Ŀ���д����ж�) */
  _bmqLoadRouterinfo();
  /*
  while(1)
  {
    ilRc = _bmqConfig_load("CLINF",-1);
    if (ilRc < 0)
    {
      _bmqDebug("S0010 %s :����CLINF����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
        __FILE__,getenv("BMQ_PATH"));
      return(-1000);
    }
    if (ilRc == FINISH) break;
  }
  del by xujun 2006.12.28
  ��ʹ��ͨ��������*/

   /*��������פ��ʱ��ͷ�ֵ����*/
  ilRc = _bmqConfig_load("MAIL",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0060 %s :����MAIL����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    _bmqQuit();
    return(-1183);
  }

  /*�������ͨѶ��Ϣ����*/
  psgMbshm->lMsgid_grp = msgget(IPC_PRIVATE,IPC_CREAT|0600);
  if (psgMbshm->lMsgid_grp < 0)
  {
    _bmqDebug("S0080 �������ͨѶ��Ϣ���д��� msgget errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1160);
  }
  psgMbshm->lMsgid_grp_rcv = 0;
  /*next 9 rows add by nh
    ����C/S��ͨѶ��Ϣ����
  psgMbshm->lMsgid_cls = msgget(IPC_PRIVATE,IPC_CREAT|0600);
  if (psgMbshm->lMsgid_cls < 0)
  {
    _bmqDebug("S0080 ����C/S��ͨѶ��Ϣ���д��� msgget errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1000);
  }
  del by xujun 2006.12.28
  ��ʹ��ͨ��������*/

  /*�����źŵƲ���ʼ��*/
  psgMbshm->lSemid = semget(IPC_PRIVATE,iSEMNUM,IPC_CREAT|0600);
  if ( psgMbshm->lSemid == -1 )
  {
    _bmqDebug("S0090 �����źŵƴ���semget errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1170);
  }

  for (i=1;i<=iSEMNUM;i++) ilArray[i-1]=1; /*iSEMNUM=2 �ź���*/
  ilRc = semctl(psgMbshm->lSemid,iSEMNUM,SETALL,ilArray);
  if (ilRc==-1)
  {
    _bmqDebug("S0100 �źŵƳ�ʼ������semctl SETALL errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1170); 
  }
   /*��ʼ��Ψһ��ϢID*/
  psgMbshm->lgMb_sys_queue_serial_num = 0;
  igMbopenflag = 0;

  /*ȡ��BMQ_GROUP*/
  if ((psgMbshm->iMbgrpid = atoi(getenv("BMQ_GROUP_ID")) ) <= 0)
  {
    _bmqDebug("S0110 ϵͳ��������[BMQ_GROUP_ID]û�����û�����ֵС����!");
    _bmqQuit();
    return(-1190);
  }
  /*ȡ�������IP��ַ�Ͷ˿ں�*/
  ilRc = _bmqConfig_load("GROUP",psgMbshm->iMbgrpid);
  if (ilRc < 0)
  {
    _bmqDebug("S0040 %s :����GROUP[%d]����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,psgMbshm->iMbgrpid,getenv("BMQ_PATH"));
    return(-1190);
  }
  
  /*�����������ļ�*/
  ilRc = _bmqUnlinkfile();
  if(ilRc == FAIL)
  {
    _bmqDebug("S0120 �����������ļ�����,��鿴ϵͳ��������BMQ_PATH����");
    _bmqQuit();
    return(-1191);
  }
  
  /*�����������ػ����̡�����״̬��ؽ��̺Ϳ����ػ�����*/
  /*
  if( fork() == 0 )
  {
    sprintf(alFp_name,"%s/bin/bmqClssnd",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqClssnd");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  if( fork() == 0 )
  {
    sprintf(alFp_name,"%s/bin/bmqClsrcv",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqClsrcv");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  del by xujun 2006.12.28
  ��ʹ��ͨ��������*/
  if( fork() == 0)
  {
    sprintf(alFp_name,"%s/bin/bmqcls",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqcls");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  if( fork() == 0)
  {
    sprintf(alFp_name,"%s/bin/bmqclean",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqclean");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  if( fork() == 0)
  {
    sprintf(alFp_name,"%s/bin/bmqGrp_snd",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqGrp_snd");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  
  if(agCommode[0] == 'L')
  {
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqGrp_rcv",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqGrp_rcv");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      exit(0);
    }  
  }
  else
  {
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqGrp_rcvs",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqGrp_rcvs");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      exit(0);
    }
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqGrp_rcvw",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqGrp_rcvw");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      exit(0);
    }
  }
  /*add by xujun 2006.12.28
    ����4���ļ������ػ�����*/
  if( fork() == 0)
  {
    sprintf(alFp_name,"%s/bin/bmqfilecls",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqfilecls");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  if( fork() == 0)
  {
    sprintf(alFp_name,"%s/bin/bmqfilemng",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqfilemng");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  if( fork() == 0)
  {
    sprintf(alFp_name,"%s/bin/bmqfilesaf",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqfilesaf");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }
  if( fork() == 0)
  {
    sprintf(alFp_name,"%s/bin/bmqfilelog",getenv("BMQ_PATH"));
    sprintf(alPro_name,"%s","bmqfilelog");
    ilRc = execl(alFp_name,alPro_name,(char *)0);
    exit(0);
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������:_bmqShutdown
 ** ����: �ر��³�Q
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 * ������������:_bmqConfig_load;_bmqProshut
 ** ȫ�ֱ���:
 ** ��������    :
 ** ����ֵ: �ɹ���SUCCESS ���󣺴�����
***************************************************************/
int _bmqShutdown()
{
  int  ilRc,i;
  FILE   *fp;
  short  ilConn_head,ilConn_head_tmp;
  struct mbconnect slMbconnect;

  /*���ӹ����ڴ�*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0130 ���ӹ����ڴ�������:%d",ilRc);
    return(ilRc);
  }

  /*�ر��ػ�����*/
  _bmqProshut();
  
  /*�����ڴ��е�����*/
  
  /*ɾ����Ϣ���У��źŵƣ������ڴ�*/
  for( i=0; i<psgMbshm->iMbinfo; i++)
  {
    if(psgMbinfo[i].iConnect == 0) continue;
    ilConn_head = psgMbinfo[i].iConn_head; 
 
    while(ilConn_head)
    { 
      /*��¼���ļ���*/
      if(ilConn_head > psgMbshm->iMbcontcount)
      {
        /*�����ӻ����ļ�*/
        if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
        {
          _bmqDebug("S0120 %s:�����ӻ����ļ�����",__FILE__);
          break;
        }
        /*ȡ��ָ����¼*/
        ilRc = _bmqConnfile_pick(fp,&slMbconnect,ilConn_head);
        ilConn_head_tmp = slMbconnect.iRlink;
        /*�жϽ����Ƿ����*/
        if( slMbconnect.lMsgid_recv )    
        {
          msgctl(slMbconnect.lMsgid_recv,IPC_RMID,0);
          ilConn_head = ilConn_head_tmp;
        }
        _bmqClosefile(fp); 
      }
      else
      {
        ilConn_head_tmp = psgMbconnect[ilConn_head-1].iRlink;
        if( psgMbconnect[ilConn_head-1].lMsgid_recv )
        { 
          msgctl(psgMbconnect[ilConn_head-1].lMsgid_recv,IPC_RMID,0);
        }
        ilConn_head = ilConn_head_tmp;
      }
    }    /*while(ilConn_head)*/
  }     /*for(Mbinfo)*/
  ilRc = _bmqQuit();
  if(ilRc)
  {
    _bmqDebug("S0140 �³�Q Shutdown, _bmqQuit() ��������!");
    return(ilRc);
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������:bmqOpen
 ** ����: ��ָ������
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:_bmqConfig_load,_bmqConn_judge,_bmqConnfile_put
 ** ȫ�ֱ���:
 ** ��������    :iMbid--�����
 ** ����ֵ: �ɹ���SUCCESS ���󣺴�����
***************************************************************/
int bmqOpen(short iMbid)
{
  int    ilRc;
  int    ilNum;
  long   llMsgid;

  /*������������ļ�DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0150 %s :����DEBUG����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1187);
  }

  if(igDebug >=2)
  {
    _bmqDebug("S0160 ****���ú���bmqOpen(%d)****",iMbid);
  }

  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0170 ���ӹ����ڴ�������:%d",ilRc);
    return(ilRc);
  }
  
  /* iMbid �Ƿ񳬳���Χ */
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) ) 
  {
    _bmqDebug("S0180 ����[%d]������Χ 1~%d",iMbid,psgMbshm->iMbinfo);
    return(1004);
  }
  /* next 5 rows add by nh �ж��Ƿ񱾵����� */
  if( psgMbinfo[iMbid - 1].iFlag  == 0 )
  {
    _bmqDebug("S0180 ����[%d]Ϊ�Ǳ�������",iMbid);
    return(1013);
  }

  /* add by xujun 2007.01 load virus database */
  if( psgMbinfo[iMbid - 1].iSendScanFlag || psgMbinfo[iMbid - 1].iRecvScanFlag )
  {
    sgRoot = NULL;
    ilRc = _bmqLoadVirusDbDir(NULL,&sgRoot,&ilNum);
    if(ilRc)
      _bmqDebug("S0182 Load virus database error!");
  }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*�жϸý����Ƿ��Ѿ����������*/
  ilRc = _bmqConn_judge(getpid());
  if(ilRc == FAIL)
  {
    _bmqDebug("S0190 �ý����Ѿ����������[%d]����",iMbid);
    _bmqUnlock(LK_ALL);
    return(1007);
  }

  /*�õ�ǰ����*/
  igMb_sys_current_mailbox = iMbid;
  igMb_sys_current_group   = psgMbshm->iMbgrpid;
  if(igDebug >=3)
  {
    _bmqDebug("S0200 �õ�ǰ����Ϊ[%d]",iMbid);  
  } 

  /*������Ϣ����*/
  llMsgid = msgget(IPC_PRIVATE,IPC_CREAT|0600);
  if (llMsgid < 0)
  {
    _bmqDebug("S0070 ������Ϣ���д��� msgget errno:%d:%s",errno,strerror(errno));
    _bmqUnlock(LK_ALL);
    return(-1160);
  }

  /*������������������һ����¼*/
  ilRc = _bmqConnect_add(iMbid,llMsgid);
  if(ilRc == FAIL)
  {
    _bmqDebug("S0210 ������������������һ����¼ʧ��!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  /*��������򿪱�־*/
  igMbopenflag  = 1;
  lgCurrent_pid = getpid();
  _bmqUnlock(LK_ALL);

  if(igDebug >= 2)
  {
    _bmqDebug("S0220 ****�������ú���bmqOpen(%d)****",iMbid); 
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������:bmqClose
 ** ����: �رյ�ǰ����
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:_bmqConnect_del,_bmqConnfile_pick
 ** ȫ�ֱ���:
 ** ��������    :
 ** ����ֵ: �ɹ���SUCCESS ����FAIL
***************************************************************/
int bmqClose()
{
  int    ilRc,ilSize;
  short  i,j;
  char   alBuf[iFLDVALUELEN];
  struct mbconnect slMbconnect;
  FILE   *fp;

  alarm(0);
  signal(SIGALRM,SIG_IGN);

  if(igDebug >= 2)
  {
    _bmqDebug("S0230 ***���ú���bmqClose()***");    
  }

  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGetw() û�д������ִ�в���!!");
      return(1011);
    }
  
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /* ȡ����ǰ���������*/
  ilSize = sizeof(struct mbconnect) + 1;
  i      = igMb_sys_current_mailbox - 1;
  j      = igMb_sys_current_connect - 1;

  /*��ǰ�����ڹ����ڴ���*/
  if( igMb_sys_current_connect <= psgMbshm->iMbcontcount )
  {
    /*ɾ����ǰ���ӽڵ�*/
    ilRc = _bmqConnect_del(i,&psgMbconnect[j]);
    if(ilRc ==  FAIL) 
    {
      _bmqDebug("S0240 ɾ����ǰ���ӽڵ����_bmqConnect_del()"); 
      _bmqUnlock(LK_ALL);
      return(FAIL); 
    }

    if(igDebug >= 2)
    {
      _bmqDebug("S0250 �ر������,����[%d]����ID:head=%d,tail=%d",i+1,
         psgMbinfo[i].iConn_head,
           psgMbinfo[i].iConn_tail);
    }
  }
  /*��ǰ�����ڻ����ļ���*/
  else
  {
    /*ȡ����¼*/
    if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
    {
      _bmqDebug("S0260 �����ӻ����ļ�����");
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
    _bmqConnfile_pick(fp,&slMbconnect,igMb_sys_current_connect);
 
    /*ɾ����ǰ���ӽڵ�*/
    ilRc = _bmqConnect_del(i,&slMbconnect);
    if(ilRc ==  FAIL)
    {
       _bmqDebug("S0270 ɾ����ǰ���ӽڵ� _bmqConnect_del()����"); 
       _bmqUnlock(LK_ALL);
       _bmqClosefile(fp);
       return(FAIL);
    }
   
    /*���û����ļ���¼*/
    alBuf[0] = '0';  
    fseek(fp,0,SEEK_SET);
    ilRc = _bmqPutfile(fp, (igMb_sys_current_connect-psgMbshm->iMbcontcount-1)*ilSize, ilSize,alBuf);
    if (ilRc)
    {
      _bmqDebug("S0280 �������ӻ����ļ���¼д�ļ�����");
      _bmqClosefile(fp);
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
 
    _bmqClosefile(fp);
  }

  igMbopenflag  = 0;
  lgCurrent_pid = 0;
  _bmqUnlock(LK_ALL);

  /*�Ͽ������ڴ�*/
  ilRc = shmdt ( (void *)agShmtmp_d);
  if ( ilRc ) 
  {
    _bmqDebug("S0070 �Ͽ������ڴ���� msgget errno:%d:%s",errno,strerror(errno));
    return(-1152);
  }
  psgMbshm = (struct mbshm *)NULL;

  if(sgRoot) _bmqFreeVirusRoot(sgRoot);

  if(igDebug >= 2)
  {
    _bmqDebug("S0290 ***�������ú���bmqClose()***");    
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������:  bmqClearmb
 ** ��  ��:  ��������
 ** ��  ��:  ���
 ** ��������:    2001.08.08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: iMbid--ָ����������� 0��ʾ������������
 ** ����ֵ:  SUCCESS , FAIL
***************************************************************/
int bmqClearmb(short iMbid)
{
  int    i,ilRc;
  long   llLink;
  /*short  ilMsglen;delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char   alBuf[iMBMAXPACKSIZE];
  FILE   *fp;
  struct mbqueue slMbqueue;

  if (igDebug >= 2)
  {
    _bmqDebug("S0300 ****���ú���bmqClearmb(%d)****",iMbid);    
  }

  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqClearmb() û�д������ִ�в���!!");
      return(1011);
    }

  if ((iMbid > psgMbshm->iMbinfo) || (iMbid < 0))
  {
    _bmqDebug("S0310 ����Ŵ���,�䷶ΧӦΪ1��%d\n",psgMbshm->iMbinfo);
    return(1004);
  }
  /* next 5 rows add by nh �ж��Ƿ񱾵�����
     next 1 rows modi by xujun iMbid=0�����ж� */
  if( iMbid && ( psgMbinfo[iMbid - 1].iFlag  == 0 ) )
  {
    _bmqDebug("S0180 ����[%d]Ϊ�Ǳ�������",iMbid);
    return(1013);
  }
  
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {
    if ((iMbid > 0) && (iMbid != i)) continue;
    llLink = psgMbinfo[i-1].lQueue_head;
    for(;;)
    {
      if (llLink == 0) break;
      if (llLink > psgMbshm->lMbqueuecount)
      {
        if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
        {
          _bmqDebug("S0320 �򿪶��л����ļ�_bmqOpenfile()����");
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        ilRc = _bmqQueuefile_pick(fp, &slMbqueue, llLink);
        if (ilRc == FAIL) 
        { 
          _bmqDebug("S0330 ���л����ļ���ȡ��¼_bmqQueuefile_pick()ʧ��!");
          _bmqUnlock(LK_ALL);
          _bmqClosefile(fp);
          return(FAIL);
        }
        if(slMbqueue.lPack_head > 0)
        {
          ilRc = _bmqPack_get(alBuf,&ilMsglen,slMbqueue.lPack_head);
          if (ilRc == FAIL) 
          { 
            _bmqDebug("S0340 ȡ���� _bmqPack_get()ʧ��!");
            _bmqUnlock(LK_ALL);
            _bmqClosefile(fp);
            return(FAIL);
          }
        } 
        llLink = slMbqueue.lRlink; 
        _bmqClosefile(fp);
      }
      else
      {
        if (psgMbqueue[llLink-1].lPack_head > 0)
        {
          ilRc = _bmqPack_get(alBuf,&ilMsglen,
            psgMbqueue[llLink-1].lPack_head);
          if (ilRc == FAIL)
          {
            _bmqDebug("S0350 ȡ����_bmqPack_get()ʧ��!");
            _bmqUnlock(LK_ALL);
            return(FAIL);
          }
        }
        llLink = psgMbqueue[llLink-1].lRlink;
      }
    }

    llLink = psgMbinfo[i-1].lQueue_head;
    for(;;)
    {
      if (llLink == 0) break;
      if(llLink > psgMbshm->lMbqueuecount)
      {
        if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
        {
          _bmqDebug("S0360 ���ļ�����");
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        ilRc = _bmqQueuefile_pick(fp, &slMbqueue, llLink);
        if (ilRc == FAIL) 
        {
          _bmqDebug("S0370 ���л����ļ���ȡ��¼_bmqQueuefile_pick()ʧ��!");      
          _bmqClosefile(fp);
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        ilRc = _bmqQueuefile_set(fp, 0, 0, 0, llLink);
        if (ilRc == FAIL) 
        {
          _bmqDebug("S0380 ���л����ļ���¼����_bmqQueuefile_set()ʧ��!");      
          _bmqClosefile(fp);
          _bmqUnlock(LK_ALL);
           return(FAIL);
        }
        llLink = slMbqueue.lRlink;
        _bmqClosefile(fp);
      }
      else 
      { 
        psgMbqueue[llLink-1].cTag = 0;
        llLink  = psgMbqueue[llLink-1].lRlink;
      }
    }
    psgMbinfo[i-1].lQueue_head = 0;
    psgMbinfo[i-1].lQueue_tail = 0;
    psgMbinfo[i-1].lPendnum = 0;
  }

  _bmqUnlock(LK_ALL);

  if (igDebug >= 2)
    {
      _bmqDebug("S0390 ****�������ú���bmqClearmb(%d)****",iMbid);    
    }

  return(SUCCESS);
}

/**************************************************************
 ** ������:  bmqGetmbinfo
 ** ��  ��:  ��ȡָ��������Ϣ
 ** ��  ��:  ���
 ** ��������:    2001.08.08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: iMbid--����� plMbinfo--����ṹָ��
 ** ����ֵ:  SUCCESS , FAIL
***************************************************************/
int bmqGetmbinfo(short iMbid,struct mbinfo *plMbinfo)
{
  if (igDebug >= 2)
  {
    _bmqDebug("S0400 ****���ú���bmqGetmbinfo(%d,struct mbinfo *plMbinfo)****",
      iMbid);    
  }

  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGetmbinfo() û�д������ִ�в���!!");
      return(1011);
    }


  /* iMbid �Ƿ񳬳���Χ */
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) )
  { 
    return(OVERFIELD);
  }  

  /* next 5 rows add by nh �ж��Ƿ񱾵�����*/
  if( psgMbinfo[iMbid - 1].iFlag  == 0 )
  {
    _bmqDebug("S0180 ����[%d]Ϊ�Ǳ�������",iMbid);
    return(1013);
  }

  memcpy(plMbinfo,&psgMbinfo[iMbid-1],sizeof(struct mbinfo));
 
  if (igDebug >= 2)
  {
    _bmqDebug("S0410 ****�������ú���bmqGetmbinfo()****");    
  }

  if( psgMbinfo[iMbid-1].iConnect == 0 ) 
    return(SUCCESS);
  else
    return 1;
}

/**************************************************************
 ** ������:bmqGet
 ** ����:  ��ȡ����(������)
 ** ����:  ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:_bmqQueue_get,_bmqPack_get
 ** ȫ�ֱ���:
 ** ��������:piGrpid-Դ����   piMbid-Դ������    piPrior-���ȼ�
             piType-����Type  piClass-����Class  aMsgbuf-����Buf
             piMsglen-���ĳ���
 ** ����ֵ: SUCCESS,������
***************************************************************/
/*int bmqGet(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,short *piMsglen)delete by wh*/
int bmqGet(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen)/*add by wh*/
{
  char alFilter[iMASKLEN];
  char alMask[iMASKLEN];
  int ilRc;

  *plType = htonl(*plType);
  *plClass = htonl(*plClass);
    
  memset(alFilter,0x00,sizeof(alFilter));
  memset(alMask,0x00,sizeof(alMask));
  
  memcpy(alFilter,(char *)plType,sizeof(long));
  memcpy(alFilter+sizeof(long),(char *)plClass,sizeof(long));
  if ((*plType) != 0) 
    memset(alMask,0xff,sizeof(long));
  if ((*plClass) != 0) 
    memset(alMask+sizeof(long),0xff,sizeof(long));
  ilRc = bmqGetfilter(piGrpid,piMbid,piPrior,alFilter,alMask,aMsgbuf,piMsglen);
  memcpy((char *)plType,alFilter,sizeof(long));
  memcpy((char *)plClass,alFilter+sizeof(long),sizeof(long));
  *plType = ntohl(*plType);
  *plClass = ntohl(*plClass);
  
  return(ilRc);
}

/**************************************************************
 ** ������:bmqGetfilter
 ** ����:  ��ȡ����(������)
 ** ����:  ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:_bmqQueue_get,_bmqPack_get
 ** ȫ�ֱ���:
 ** ��������:piGrpid-Դ����       piMbid-Դ������  piPrior-���ȼ�
             aFilter-���Ĺ������� aMask-��������   aMsgbuf-����Buf
             piMsglen-���ĳ���
 ** ����ֵ: SUCCESS,������
***************************************************************/
/*int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,short *piMsglen)delete by wh*/
int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen)/*add by wh*/
{
  int  ilRc;
  long llPack_head;

  if(igDebug >= 2)
  {
    _bmqDebug("S0420 ***���ú���:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }

  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGet() û�д������ִ�в���!!");
      return(1011);
    }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*����������м�¼,ȡ����������ͷ*/
  ilRc = _bmqQueue_get(piGrpid,piMbid,piPrior,aFilter,aMask,&llPack_head);
  if (ilRc)
  {
    if(ilRc != 1001)  	
      _bmqDebug("S0430 ȡ����������ͷ����!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }
 
  /*�޸�������Ϣ*/
  psgMbinfo[igMb_sys_current_mailbox-1].lPendnum--;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime));

  /*��ȡ����*/
  ilRc = _bmqPack_get(aMsgbuf,piMsglen,llPack_head);
  if( ilRc )
  {
    _bmqDebug("S0440 ��ȡ����ERROR!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  _bmqUnlock(LK_ALL);
  if(igDebug >= 1)
  {
    _bmqDebug("S0450 ����[%d]ȡ����[%d]����[%d]��[%d]����",
      igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
  }


  if(igDebug >= 3)
  {
    _bmqDebug("S0460 ����[%d]�޸ĺ���ϢlPendnum[%d],lRecvpid[%d],lRecvtime[%d]",
      igMb_sys_current_mailbox,
      psgMbinfo[igMb_sys_current_mailbox-1].lPendnum,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime);

  }

  if(igDebug >= 2)
  {
    _bmqDebug("S0470 ***�������ú���:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S0480 ***����bmqGetȡ���ı�������aMsgbufΪ***");
    _bmqDebug("S0490 %s",aMsgbuf); 
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������:bmqGetw
 ** ����:  ��ȡ����(����)
 ** ����:  ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:_bmqQueue_get,_bmqPack_get
 ** ȫ�ֱ���:
 ** ��������:piGrpid-Դ����   piMbid-Դ������   piPrior-���ȼ�
             piType-����Type  piClass-����Class aMsgbuf-����Buf
             piMsglen-���ĳ���   iTimeout-���������ʱ��
 ** ����ֵ: SUCCESS,������
***************************************************************/
/*int bmqGetw(short *piGrpid,short *piMbid,short *piPrior,long *plType,
  long *plClass,char *aMsgbuf,short *piMsglen,short iTimeout) delete by wh*/
int bmqGetw(short *piGrpid,short *piMbid,short *piPrior,long *plType,
  long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout)/*add by wh*/
{
  char alFilter[iMASKLEN];
  char alMask[iMASKLEN];
  int ilRc;
  
  *plType = htonl(*plType);
  *plClass = htonl(*plClass);
 
  memset(alFilter,0x00,sizeof(alFilter));
  memset(alMask,0x00,sizeof(alMask));

  memcpy(alFilter,(char *)plType,sizeof(long));
  memcpy(alFilter+sizeof(long),(char *)plClass,sizeof(long));
  if ((*plType) != 0) 
    memset(alMask,0xff,sizeof(long));
  if ((*plClass) != 0) 
    memset(alMask+sizeof(long),0xff,sizeof(long));

  ilRc = bmqGetwfilter(piGrpid,piMbid,piPrior,alFilter,alMask,aMsgbuf,piMsglen,iTimeout);
  memcpy((char *)plType,alFilter,sizeof(long));
  memcpy((char *)plClass,alFilter+sizeof(long),sizeof(long));
  *plType = ntohl(*plType);
  *plClass = ntohl(*plClass);
  
  return(ilRc);
}
/**************************************************************
 ** ������:bmqGetw
 ** ����:  ��ȡ����(����)
 ** ����:  ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:_bmqQueue_get,_bmqPack_get
 ** ȫ�ֱ���:
 ** ��������:piGrpid-Դ����        piMbid-Դ������  piPrior-���ȼ�
             aFilter-���Ĺ�������  aMask-��������   aMsgbuf-����Buf
             piMsglen-���ĳ���   iTimeout-���������ʱ��
 ** ����ֵ: SUCCESS,������
***************************************************************/
/*int bmqGetwfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,
  char *aMask,char *aMsgbuf,short *piMsglen,short iTimeout)delete by wh*/
int bmqGetwfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,
  char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout)/*add by wh*/
{
  int    ilRc,ilSize;
  short  j;
  long   llPack_head;
  long   i,llMsgid;
  char   alBuf[iFLDVALUELEN];
  FILE   *fp;
  struct recvmsg slRecvmsg;
  struct mbqueue slMbqueue;
  
  if(igDebug >= 2)
  {
    _bmqDebug("S0500 ***���ú���:bmqGetw(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }

  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() ) 
    {
      _bmqDebug("S0510 bmqGetw() û�д������ִ�в���!!"); 
      return(1011);
    }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*�Է�������ʽ��ȡһ��,���������򷵻�*/
  ilRc = _bmqQueue_get(piGrpid,piMbid,piPrior,aFilter,aMask,&llPack_head);
  if (ilRc == 0)
  { 
    psgMbinfo[igMb_sys_current_mailbox-1].lPendnum--;
    psgMbinfo[igMb_sys_current_mailbox-1].lRecvnum++;
    psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid = getpid();
    time(&(psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime));

    ilRc = _bmqPack_get(aMsgbuf,piMsglen,llPack_head);
    if(ilRc)
    {
      _bmqDebug("S0508 ��ȡ���Ĵ���!");
      _bmqUnlock(LK_ALL);
      return(FAIL);
    }

    _bmqUnlock(LK_ALL);

    if(igDebug >= 1)
    {
      _bmqDebug("S0510 ����[%d]���ú���:bmqGetw()�Է�������ʽȡ����[%d]����[%d][%d]����",igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
    }
    if(igDebug >= 2)
    {
      _bmqDebug("S0520 ***�����Է�������ʽ���ú���:bmqGetw(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
        *piGrpid,*piMbid,*piPrior,*piMsglen);
    }
    if(igDebug >= 3)
    {
      _bmqDebug("S0530 *��������ʽ���ú���bmqGetw()ȡ���ı�������aMsgbufΪ*");
      _bmqDebug("S0540 %s",aMsgbuf); 
    }

    return(SUCCESS);
  }


  j = igMb_sys_current_connect - 1;

  /*�������������� cTag==2����������*/
  ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
  if(ilRc == FAIL)
  {
    _bmqDebug("S0550 ������������������ERROR!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  llMsgid = _bmqGetmsgid(igMb_sys_current_connect);
  if(llMsgid == FAIL)
  {
    _bmqDebug("S0550 ȡ��Ϣ����ID����!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  _bmqUnlock(LK_ALL);

Tagmsgrcv:
  if(igDebug >= 2)
    _bmqDebug("S0560 ����Ϣ����(��)��������:mtype=%d",j+1);

  /*������������Ϣʱ��*/
  if (iTimeout != 0)
  {
    signal(SIGALRM,_bmqTimeout);
    alarm(iTimeout);
  }
  igFlag = 0;

  /*��������Ϣ������������ȡ��Ϣ*/
  ilRc = msgrcv(llMsgid,&slRecvmsg,sizeof(struct recvmsg),0,0);
  if (iTimeout != 0)
  {
    alarm(0);
    signal(SIGALRM,SIG_DFL);
  }
  if (igFlag == 1) return(1006);
   
  if (ilRc < 0)
  {
    if (errno == EINTR)
    {
      _bmqDebug("S0570 ��������Ϣ������������ȡ��Ϣ���� msgrcv() errno:%d:%s",errno,strerror(errno)); 
      goto Tagmsgrcv;
    }
    _bmqDebug("S0580 ��������Ϣ������������ȡ��Ϣ���� msgrcv(),errno:%d:%s",errno,strerror(errno)); 
    _bmqUnlock(LK_ALL);  
    return(-1140);
  }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  j = igMb_sys_current_connect-1;

  if(igDebug >= 2)
  {
    _bmqDebug("S0590 ����Ϣ����(��)�ж�����Ϣ:lQueueid=%d,lSerialnum=%ld",
      slRecvmsg.lQueueid,slRecvmsg.lSerialnum);
  }

  i = slRecvmsg.lQueueid - 1;

  for (;;)
  {
    /*�ж���Ϣ�Ϸ���*/
    if (i >= psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S0600 ���ļ�����_bmqOpenfile()");
        _bmqUnlock(LK_ALL);
        return (FAIL);
      }
      _bmqQueuefile_pick(fp,&slMbqueue,slRecvmsg.lQueueid);
      _bmqClosefile(fp);
      if(slMbqueue.lSerialnum == slRecvmsg.lSerialnum)
        break; 
    }
    else
    {
      if (psgMbqueue[i].lSerialnum == slRecvmsg.lSerialnum)
        break;
    }
    if(igDebug >=2)
    {
      _bmqDebug("S0610 ������ϢΪ������Ϣ,��Ϣ����:mb=%d mbqueue.cTag=%d,mbqueue.lSerialnum=%ld,��Ϣ����lSerialnum=%ld",i,
        psgMbqueue[i].cTag,psgMbqueue[i].lSerialnum, slRecvmsg.lSerialnum);
      _bmqDebug("S0620 ����Ϣ����(��)�з�������:mtype=%d",j+1);
    }

    /*������ϢΪ������Ϣ,��Ϣ���ϴ���Ϣ����(��)�з�������*/
    ilRc = msgrcv(llMsgid,&slRecvmsg,sizeof(struct recvmsg),
      0,IPC_NOWAIT);
    if (ilRc < 0) 
    {
      if (errno == ENOMSG)
      {
        if(igDebug >= 2)
          _bmqDebug("S0630 ������[%d]��cTag��Ϊ��������״̬",j+1);

        /*�������������� cTag==2����������*/
        ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
        if(ilRc == FAIL)
        {
          _bmqDebug("S0640 ��������������ERROR!");
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        _bmqUnlock(LK_ALL);

        goto Tagmsgrcv;
      }
      _bmqDebug("S0650 ����Ϣ����(��)�з����������� msgrcv errno:%d:%s",errno,strerror(errno)); 
      _bmqUnlock(LK_ALL);
      return(-1140);
    }
  }

  /*�����Ϸ���Ϣ���������Ӽ�¼*/
  if(igMb_sys_current_connect > psgMbshm->iMbcontcount)
  {
    ilRc = _bmqConnfile_set(1, -1, -1, aFilter, aMask, igMb_sys_current_connect,-1);
    if(ilRc == FAIL)
    {
      _bmqDebug("S0660 _bmqConnfile_set() ���س���");
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
  }
  else
    psgMbconnect[j].cTag = 1;

  /*ɾ�������м�¼*/
  if(slRecvmsg.lQueueid > psgMbshm->lMbqueuecount)
  {
  	
    /* �򿪶��л����ļ� */
    ilSize = sizeof(struct mbqueue) + 1;
    if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
    {
      _bmqDebug("S0670 ���ļ�����!");
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
    _bmqQueuefile_pick(fp,&slMbqueue,slRecvmsg.lQueueid); 
    
    ilRc = _bmqQueue_del(igMb_sys_current_mailbox,&slMbqueue, piGrpid, piMbid, 
                         piPrior, aFilter,&llPack_head);
    if(ilRc == WITHOUTUSE)
    {
      _bmqDebug("S0672 ɾ���Ķ���[%d]û������",slRecvmsg.lQueueid);
      /*�������������� cTag==2����������*/
      ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
      _bmqUnlock(LK_ALL);
      if(ilRc == FAIL)
      {
        _bmqDebug("S0674 ��������������ERROR!");
        return(FAIL);
      }
      goto Tagmsgrcv;
    }

    /* ��д�ļ� */
    alBuf[0] = '0';
    fseek(fp,0,SEEK_SET);
    ilRc = _bmqPutfile(fp, (slRecvmsg.lQueueid-psgMbshm->lMbqueuecount-1)
      *ilSize, ilSize,alBuf);
    if (ilRc)
    {
      _bmqDebug("S0680 д�ļ�����");
      _bmqClosefile(fp);
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
    _bmqClosefile(fp);
  }
  else
  {
    ilRc = _bmqQueue_del(igMb_sys_current_mailbox,&psgMbqueue[i], piGrpid, 
                         piMbid, piPrior, aFilter,&llPack_head);
    if(ilRc == WITHOUTUSE)
    {
      _bmqDebug("S0682 ɾ���Ķ���[%d]û������",i+1);
      /*�������������� cTag==2����������*/
      ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
      _bmqUnlock(LK_ALL);
      if(ilRc == FAIL)
      {
        _bmqDebug("S0684 ��������������ERROR!");
        return(FAIL);
      }
      goto Tagmsgrcv;
    }
  }

  if(igDebug >= 2)
  {
    _bmqDebug("S0690 ���ʼ�����[%d]�ж�����Ϣ:org_group=%d,org_q=%d,pack_head=%ld,prior=%d",
      i+1,*piGrpid,*piMbid,llPack_head,*piPrior);
  }

  psgMbinfo[igMb_sys_current_mailbox-1].lPendnum--;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime));

   /*��ȡ����*/
  ilRc = _bmqPack_get(aMsgbuf,piMsglen,llPack_head);
  if(ilRc)
  {
    _bmqDebug("S0692 ��ȡ���Ĵ���!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  _bmqUnlock(LK_ALL);

  if(igDebug >= 1)
  {
    _bmqDebug("S0700 ����[%d]���ú���:bmqGetw()��������ʽȡ����[%d]����[%d]��[%d]����",igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
  }
  if(igDebug >= 2)
  {
    _bmqDebug("S0710 ***�������ú���:bmqGetw(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S0720 ***����bmqGetw()ȡ���ı�������aMsgbufΪ***");
    _bmqDebug("S0730 %s",aMsgbuf); 
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������:bmqPut
 ** ����:  ��ű���
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iGrpid-Ŀ����   iMbid-Ŀ������   iPrior-���ȼ�
             iType-����Type  iClass-����Class aMsgbuf-����Buf
             iMsglen-���ĳ��� 
 ** ����ֵ: SUCCESS
***************************************************************/
/*int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,
  char *aMsgbuf,short iMsglen)delete by wh*/
int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,
  char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  char alFilter[iMASKLEN];
  int ilRc;
  
  _bmqVdebug(2,"S0735 ***���ú��� bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",iGrpid,iMbid,iPrior,iMsglen); 
  
  lType = htonl(lType);
  lClass = htonl(lClass);
  
  memset(alFilter,0x00,sizeof(alFilter));
  memcpy(alFilter,(char *)&lType,sizeof(long));
  memcpy(alFilter+sizeof(long),(char *)&lClass,sizeof(long));
  ilRc = bmqPutfilter(iGrpid,iMbid,iPrior,alFilter,aMsgbuf,iMsglen);
  return(ilRc);
}
/********************************************************************
 ** ������:bmqPutfilter
 ** ����:  ��ű���
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iGrpid-Ŀ����        iMbid-Ŀ������  iPrior-���ȼ�
             aFilter-���Ĺ������� aMsgbuf-����Buf iMsglen-���ĳ��� 
 ** ����ֵ: SUCCESS
*********************************************************************/
/*int bmqPutfilter(short iGrpid,short iMbid,short iPrior,char *aFilter,
  char *aMsgbuf,short iMsglen)delete by wh*/
int bmqPutfilter(short iGrpid,short iMbid,short iPrior,char *aFilter,
  char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    ilRc;
  long   llPack_head,llQueue_id;

  if( igDebug >= 2)
  {
    _bmqDebug("S0740 ***���ú��� bmqPutfilter(%d,%d,%d,aFilter,aMsgbuf,%d)***",
      iGrpid,iMbid,iPrior,iMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    
  }

  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() ) 
  {
    _bmqDebug("S0510 bmqPut() û�д������ִ�в���!!");
    return(1011);
  }
 
  /*�жϲ����Ϸ���*/
  if( iGrpid < 0) return(1009);
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) ) return(1004);
  if( (iPrior < 0) || (iPrior > 99) ) return(1010);

   /*�ж��Ƿ��Ѿ�����ָ����ֵ*/
  if( psgMbinfo[iMbid-1].lMaxqueue && psgMbinfo[iMbid-1].lPendnum >= 
    psgMbinfo[iMbid-1].lMaxqueue )
  {
    return(1008);
  }
  
  /*�жϱ��ĳ����Ƿ񳬹����ֵ*/
  if(iMsglen > iMBMAXPACKSIZE)
  {
    _bmqDebug("S0750 ���ĳ���[%d]�������ֵ�� [%d]",iMsglen,iMBMAXPACKSIZE);
    return(1012);
  }
  
  if( iGrpid == 0 ) iGrpid = psgMbshm->iMbgrpid;
 
  	
  if(psgMbshm->iMbgrpid != iGrpid)
  {
    ilRc = _bmqConfig_load("GROUP",iGrpid);
    if (ilRc < 0)
    {  
      _bmqDebug("0751 :����GROUP[%d]����ʧ��",iGrpid);
      return(FAIL);
    }
  }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*������߳����ӿ���ʱ�洢����*/
  if( ( psgMbshm->iMbgrpid == iGrpid ) || (agCommode[0] != 'S') )
  {
    ilRc = _bmqPack_put(aMsgbuf,iMsglen,&llPack_head);
    if (ilRc)
    {
      _bmqUnlock(LK_ALL);
      return(ilRc);
    }
    if(igDebug >= 3)
    {
      _bmqDebug("S0760 ����bmqPut()����ı�������aMsgbufΪ:",aMsgbuf);
      _bmqDebughex(aMsgbuf,iMsglen);
    }
  }

  /*������飬�ػ����̷��ʹ�����Ϣ*/
  if( iGrpid == 0 ) iGrpid = psgMbshm->iMbgrpid;
  if(psgMbshm->iMbgrpid != iGrpid)
  {
    psgMbinfo[igMb_sys_current_mailbox-1].lSendnum++;
    psgMbinfo[igMb_sys_current_mailbox-1].lSendpid = getpid();
    time(&(psgMbinfo[igMb_sys_current_mailbox-1].lSendtime));
    if(psgMbshm->iStatflag)
    {
      _bmqSendstat(iMbid);
    }
    _bmqUnlock(LK_ALL);    
    
    /*Ŀ����ʹ�ö����� */
    if( agCommode[0] == 'S' ) 
    {
      ilRc = _bmqGrpasc_snd(iGrpid,iMbid,iPrior,aFilter,aMsgbuf,iMsglen);
      return(ilRc);      
    }
    /*Ŀ����ʹ�ó�����*/
    ilRc = _bmqGrpmsg_snd(llPack_head,iPrior,aFilter,iGrpid,iMbid);
    if(ilRc) return(ilRc);

    if(igDebug >= 1)
    {
      _bmqDebug("S0770 ����[%d]����[%d]����[%d]����[%d]����",
        igMb_sys_current_mailbox,iGrpid,iMbid,llPack_head); 
    }
    if( igDebug >= 2)
    {
      _bmqDebug("S0780 ***�������ú��� bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",
        iGrpid,iMbid,iPrior,iMsglen);
    }

    return(SUCCESS);
  }
  /*�Ǳ������䷢�͵��ͻ���
  next add by nh
  if(psgMbinfo[iMbid-1].iFlag == 0)
  {
    psgMbinfo[igMb_sys_current_mailbox-1].lSendnum++;
    psgMbinfo[igMb_sys_current_mailbox-1].lSendpid = getpid();
    time(&(psgMbinfo[igMb_sys_current_mailbox-1].lSendtime));
    if(psgMbshm->iStatflag)
    {
      _bmqSendstat(iMbid);
    }
    _bmqUnlock(LK_ALL);
    ilRc = _bmqClsmsg_snd(llPack_head,iPrior,aFilter,iGrpid,iMbid);
    if(ilRc) return(ilRc);
    
    if(igDebug >= 1)
    {
      _bmqDebug("S0771 ����[%d]����[%d]����[%d]����[%d]����",
        igMb_sys_current_mailbox,iGrpid,iMbid,llPack_head);
    }
    if( igDebug >= 2)
    {
      _bmqDebug("S0781 ***�������ú��� bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",
        iGrpid,iMbid,iPrior,iMsglen);
    }

    return(SUCCESS);
  }
  del by xujun
  ��ʹ��ͨ��������*/

  /*�洢����*/
  ilRc = _bmqQueue_put( iGrpid, iMbid, iPrior, aFilter,llPack_head,&llQueue_id);
  if (ilRc)
  {
    _bmqDebug("S0790 �洢����_bmqQueue_put(...)����!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  /*�޸�������Ϣ*/
  psgMbinfo[iMbid-1].lPendnum++;

  /*if ( igMb_sys_current_group == psgMbshm->iMbgrpid )*/
  psgMbinfo[igMb_sys_current_mailbox-1].lSendnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lSendpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lSendtime));
  
  if(psgMbshm->iStatflag)
  {
    _bmqSendstat(iMbid);
  }

  /*���������Ľ��̷���֪ͨ��Ϣ*/
  ilRc = _bmqConn_select(iMbid, iPrior, aFilter, llQueue_id);
  _bmqUnlock(LK_ALL);

  if(igDebug >= 1)
  {
    _bmqDebug("S0800 ����[%d]����[%d]����[%d]����[%d]����",
      igMb_sys_current_mailbox,iGrpid,iMbid,llPack_head); 
  }
  if( igDebug >= 2)
  {
    _bmqDebug("S0810 ***�������ú��� bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",
    iGrpid,iMbid,iPrior,iMsglen);
  }
  if(ilRc == FAIL)
  {
    _bmqDebug("S0798 ���������Ľ��̷���֪ͨ��Ϣ����!");
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqGrpasc_snd
 ** ����:   ���Ϳ��鱨��(�����ӷ�ʽ)
 ** ����:   ���
 ** ��������: 2007/01/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*int _bmqGrpasc_snd(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen)delete by wh*/
int _bmqGrpasc_snd(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    z;			/* zcb ��Ҫ��ν���*/
  
  int	 ilRc,ilLentmp;
  int    ilSockfd;                 /*�׽���*/
  /*short  ilMsglen_grp,ilLen;    delete by wh ���ĳ���*/
  TYPE_SWITCH ilMsglen_grp;/*ilLen;*/    /*���ĳ��� add by wh*/
  char   alVal[11];
  char   alMsgbuf[iMBMAXPACKSIZE+100];       /*���Ĵ洢����*/
  char   alMsgbuf_grp[iMBMAXPACKSIZE+100];   /*���ķ�������*/
  
  /*char   alTmpbuf[iMBMAXPACKSIZE+100];	add by pc ��ʱbuf*/
  
  char   agrprecvmsgbuf[49];
  
  struct linger ilLinger;
  struct sockaddr_in slCli_addr;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif

  long llBeginTime;
  /*long llEndTime;
  char aTmp[16]; delete by wh 20150107*/
   struct timeval rwto;
	struct grprecvmsg sGrpmsg;
	
  time(&llBeginTime); 
  /*���ӵ�������*/
  if ((igConnNum>3)||(igConnNum<1)) igConnNum=DEFAULTCONNUM;
  for(z=0;z<igConnNum;z++) 
  {
  	if ((igTimeOut>0)&&(igTimeOut<DEFAULTCONTIME)){
  		
  	}	
  	else
  		igTimeOut=DEFAULTCONTIME;
 		
	ilSockfd = _bmqGrpcontser(agIp,igPort,agSendIp,igTimeOut);
    if ( ilSockfd < 0 )
    {
      _bmqDebug("S0120 _bmqGrpasc_snd()���ӵ�����������!! IP=[%s] Port=[%d] Timeout[%d]",
        agIp,igPort,igTimeOut);
      _bmqDebug("ѭ������,����:%d",z);
      continue;
    }
    else
    {
      break;
    }
  }

  if(z == DEFAULTCONNUM)
  {
    return(FAIL);	
  }
  
  if( igDebug >= 2) _bmqDebug("S0121 _bmqGrpasc_snd()���ӵ�������IP=[%s] Port=[%d]�ɹ�",agIp,igPort);

  /*ȡ��SOCKET��Ϣ*/
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( ilSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0130 _bmqGrpasc_snd()unable to read socket address ilSockfd [%d] errno[%d]",ilSockfd,errno);
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
    _bmqDebug("S0140 _bmqGrpasc_snd()Unable to set socket option errno: %d,%s",errno,strerror(errno));
    close(ilSockfd);
    return(FAIL);
  }
  


  sGrpmsg.mtype        = iGrpid;
//  sGrpmsg.lPack_head   = lPack_head;
  sGrpmsg.lPack_head   = 0;
  sGrpmsg.iOrg_group   = psgMbshm->iMbgrpid;
  sGrpmsg.iOrg_mailbox = igMb_sys_current_mailbox;	/*����Ƿ���21*/
  sGrpmsg.iPrior       = iPrior;
  memcpy(sGrpmsg.aFilter,aFilter,iMASKLEN);
  sGrpmsg.iDes_group   = iGrpid;
  sGrpmsg.iDes_mailbox = iMbid;

  /*****used by test**********************************/
  int test=1;
  memset(agrprecvmsgbuf,0x00,sizeof(agrprecvmsgbuf));
  /*sprintf(agrprecvmsgbuf,"%8ld%8ld%2d%2d%2d%20s%2d%2d",sGrpmsg.mtype,sGrpmsg.lPack_head,sGrpmsg.iOrg_group,sGrpmsg.iOrg_mailbox,sGrpmsg.iPrior,sGrpmsg.aFilter,sGrpmsg.iDes_group,sGrpmsg.iDes_mailbox);*/
  sprintf(agrprecvmsgbuf,"%8ld%8ld%8d%8d%8d%20s%8d%8d",sGrpmsg.mtype,sGrpmsg.lPack_head,sGrpmsg.iOrg_group,sGrpmsg.iOrg_mailbox,sGrpmsg.iPrior,sGrpmsg.aFilter,sGrpmsg.iDes_group,sGrpmsg.iDes_mailbox);
  _bmqDebug("ztq :agrprecvmsgbuf[%s]",agrprecvmsgbuf);
  /*****used by test**********************************/

	  /************* add by xuchengyong 2002/10/18   *********/
	  /*sGrpmsg.mtype = htonl(sGrpmsg.mtype);
	  sGrpmsg.lPack_head = htonl(sGrpmsg.lPack_head);
	  sGrpmsg.iOrg_group = htons(sGrpmsg.iOrg_group);
	  sGrpmsg.iOrg_mailbox = htons(sGrpmsg.iOrg_mailbox);
	  sGrpmsg.iPrior = htons(sGrpmsg.iPrior);
	  sGrpmsg.iDes_group = htons(sGrpmsg.iDes_group);
	  sGrpmsg.iDes_mailbox = htons(sGrpmsg.iDes_mailbox);*/
	  /************* add by xuchengyong 2002/10/18   *********/

    /*��֯����*/
    memset(alMsgbuf_grp,0x00,sizeof(alMsgbuf_grp));
    if(test!=1)
	    ilMsglen_grp = iMsglen + sizeof(struct grprecvmsg) + 8;
	   else
	   	 ilMsglen_grp = iMsglen + sizeof(struct grprecvmsg) + 8+28;

    sprintf(alVal,"%08d",ilMsglen_grp);
    memcpy(alMsgbuf_grp,alVal,8);   

    if(test!=1){
	    memcpy(alMsgbuf_grp+8, (char *)&sGrpmsg, sizeof(struct grprecvmsg));    
	    memcpy(alMsgbuf_grp+8+sizeof(struct grprecvmsg), aMsgbuf, iMsglen);
	  }
	  else{
	  	 memcpy(alMsgbuf_grp+8, agrprecvmsgbuf, 76);
	  	 memcpy(alMsgbuf_grp+8+76, aMsgbuf, iMsglen);
	  	}


    ilLentmp = ilMsglen_grp;

	/*2006-8-21 10:07 ���ӻ�ִ��־*/
/*   	memset(alTmpbuf,0x00,sizeof(alTmpbuf));
	sprintf(alTmpbuf,"%1d",igAdmitFlag);
	memcpy(alTmpbuf+1,alMsgbuf_grp,ilMsglen_grp);
	memset(alMsgbuf_grp,0x00,sizeof(alMsgbuf_grp));
	memcpy(alMsgbuf_grp,alTmpbuf,ilMsglen_grp+1);
	ilMsglen_grp+=1;

    time(&llEndTime);
    igTotalTime=igTotalTime-(llEndTime-llBeginTime);
    if (cgTranType!='6') {
    	memset(aTmp,0x00,sizeof(aTmp));
    	sprintf(aTmp,"%04d",igTotalTime);
    	ilLen=ilMsglen_grp-4-4-6-1-1;
    	memcpy(alMsgbuf_grp+ilLen,aTmp,4);
    }*/
    /*add by pc 20060311 ���б����޸�4λ �ܳ�ʱʱ����Ϣ*/
/*add by pc Ӧ�����÷��ͳ�ʱѡ��*/ 
	if( igDebug >= 2 )
	{
		_bmqDebug("���������");
		_bmqDebughex(alMsgbuf_grp,ilMsglen_grp);
	}	

    /*���ͱ���*/
    ilRc = _bmqGrpSend(ilSockfd,alMsgbuf_grp,&ilMsglen_grp);
    if ( ilRc != 0 )
    {
      _bmqDebug("S0150 %s: _bmqGrpasc_snd()Send to server ERROR!!",__FILE__);
      close(ilSockfd);
      return(FAIL);
    }

    if( igDebug >= 2 )
    {
      _bmqDebug("S0160 _bmqGrpasc_snd()Send to server completed! Msglen=[%d] Sendlen=[%d]",
        ilLentmp,ilMsglen_grp);
    }
	/*2006-8-21 �ȴ��ٻ�ִ*/
	if (igAdmitFlag==1) {
		memset(alMsgbuf,0x00,sizeof(alMsgbuf));
		/*recv��ʱʱ��Ӧ������*/
		rwto.tv_sec = 30;
		rwto.tv_usec= 0;
		
		ilRc = setsockopt(ilSockfd,SOL_SOCKET,SO_RCVTIMEO,&rwto,sizeof(rwto));
		if ( ilRc < 0) {
			close(ilSockfd);
			_bmqDebug("S165 setsockopt SO_RCVTIMEO���� [%d]",errno);
			return(FAIL);			
		}
		ilRc=recv(ilSockfd,alMsgbuf,sizeof(alMsgbuf),0);
		
		if( igDebug >= 2 ) {
			_bmqDebug("�յ������ٻ�ִ���ݿ�ʼ...");
			_bmqDebughex(alMsgbuf,ilRc);
			_bmqDebug("�յ������ٻ�ִ���ݽ���...");
		}
		if (ilRc>0) {
			if (memcmp(alMsgbuf,"HZ",2)!=0) {
				close(ilSockfd);
				_bmqDebug("S170 �յ��ļٻ�ִ���ݴ��� [%s]",alMsgbuf);
				return(FAIL);			
			}
			else {
				_bmqDebug("S0000 �յ��ٻ�ִ�ɹ�");
			}	
		}
		else {
			close(ilSockfd);
			_bmqDebug("S0180 û�յ��ٻ�ִ[%d]",ilRc);
			return(FAIL);
		}
	}
	/*2006-8-21 �ȴ��ٻ�ִ*/
	
    close(ilSockfd);	/*�Ƿ���Բ��أ������´η���*/
    return(SUCCESS);
}

/*int _bmqGrpasc_snd_old(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen)delete by wh*/
int _bmqGrpasc_snd_old(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    ilTrytimes;                         /*�������Ӵ���*/
  int    ilRc;                               /*����ֵ*/
  int    ilSockfd;                           /*�׽���*/
  /*short  ilMsglen_grp;                       delete by wh ���ĳ���*/
  TYPE_SWITCH ilMsglen_grp;                       /*���ĳ��� add by wh*/
  char   alMacbuf[MAC_LEN];                  /*����MAC*/
  char   alVal[11];                          /*��ʱ����*/
  char   alMsgbuf[20];                       /*IP��ַBUF*/
  char   alMsgbuf_grp[iMBMAXPACKSIZE+100];   /*���ķ�������*/
  struct grprecvmsg slGrpmsg;                /*������Ϣ*/
  struct linger ilLinger;                    /*socket����*/
  struct sockaddr_in slCli_addr;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif

  _bmqVdebug(2,"S0118 ���Ϳ��鱨��(�����ӷ�ʽ),grp=[%d],mb=[%d]",iGrpid,iMbid);

  /*���ӵ�Ŀ�������*/
  for(ilTrytimes=0;ilTrytimes<DEFAULTCONNUM;ilTrytimes++) 
  {
	  ilSockfd = _bmqGrpcontserS(agBmqhostip,igBmqgrpport,lgGroup_try_time);
    if ( ilSockfd < 0 )
    {
      _bmqDebug("S0120 _bmqGrpasc_snd()���ӵ�����������!! IP=[%s] Port=[%d] Timeout[%d]",
        agBmqhostip,igBmqgrpport,lgGroup_try_time);
      _bmqDebug("S0121 ѭ������,����:%d",ilTrytimes);
      continue;
    }
    else
      break;
  }
  
  if(ilTrytimes == DEFAULTCONNUM)
  {
    _bmqDebug("S0122 _bmqGrpasc_snd()���ӵ�������ʧ��!! IP=[%s] Port=[%d]",
      agBmqhostip,igBmqgrpport);
    return(FAIL);	
  }
  _bmqVdebug(2,"S0123 _bmqGrpasc_snd()���ӵ�������IP=[%s] Port=[%d]�ɹ�",
    agBmqhostip,igBmqgrpport);

  /*ȡ��SOCKET��Ϣ*/
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( ilSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0130 _bmqGrpasc_snd()unable to read socket address");
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
    _bmqDebug("S0140 _bmqGrpasc_snd()Unable to set socket option errno: %d,%s",errno,strerror(errno));
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
  _bmqDebughex(alMsgbuf_grp,ilMsglen_grp);
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
    _bmqDebug("S0143 ������IP=[%s]��δ���屾����Ϣ!!!",agBmqhostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"UNMAC",5))
  {
    _bmqDebug("S0144 ������IP=[%s]δͨ�����ȷ����Ϣ!!!",agBmqhostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"IPERR",5))
  {
    _bmqDebug("S0145 ������IP=[%s]δͨ�����ȷ����Ϣ!!!",agBmqhostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(memcmp(alMsgbuf,"AAAAA",5))
  {
    _bmqDebug("S0146 ��������");
    close(ilSockfd);
    return(FAIL);
  }
  
  /*��֯����*/
  slGrpmsg.mtype        = iGrpid;
  slGrpmsg.lPack_head   = 0;
  slGrpmsg.iOrg_group   = igMb_sys_current_group;
  slGrpmsg.iOrg_mailbox = igMb_sys_current_mailbox;
  slGrpmsg.iPrior       = iPrior;
  memcpy(slGrpmsg.aFilter,aFilter,iMASKLEN);
  slGrpmsg.iDes_group   = iGrpid;
  slGrpmsg.iDes_mailbox = iMbid;
  
  /*�����ֽ���ת��*/ 
  slGrpmsg.mtype        = htonl(slGrpmsg.mtype);
  slGrpmsg.lPack_head   = htonl(slGrpmsg.lPack_head);
  slGrpmsg.iOrg_group   = htons(slGrpmsg.iOrg_group);
  slGrpmsg.iOrg_mailbox = htons(slGrpmsg.iOrg_mailbox);
  slGrpmsg.iPrior       = htons(slGrpmsg.iPrior);
  slGrpmsg.iDes_group   = htons(slGrpmsg.iDes_group);
  slGrpmsg.iDes_mailbox = htons(slGrpmsg.iDes_mailbox);

  /*��֯����*/
  memset(alMsgbuf_grp,0x00,sizeof(alMsgbuf_grp));
  ilMsglen_grp = iMsglen + sizeof(struct grprecvmsg) + 8 + MAC_LEN;
  sprintf(alVal,"%08d",htons(ilMsglen_grp));
  memcpy(alMsgbuf_grp,alVal,8);
  memcpy(alMsgbuf_grp+8, (char *)&slGrpmsg, sizeof(struct grprecvmsg));
  memcpy(alMsgbuf_grp+8+sizeof(struct grprecvmsg), aMsgbuf, iMsglen);
  _bmqMac(aMsgbuf,iMsglen,alMacbuf);
  memcpy(alMsgbuf_grp+ilMsglen_grp-MAC_LEN,alMacbuf,MAC_LEN);

  /*���ͱ���*/
  if( igDebug >= 3 )
	{
		_bmqDebug("���ͱ�������(%d):",ilMsglen_grp);
		_bmqDebughex(alMsgbuf_grp,ilMsglen_grp);
	}
  ilRc = _bmqGrpSend(ilSockfd,alMsgbuf_grp,&ilMsglen_grp);
  if ( ilRc != 0 )
  {
    _bmqDebug("S0150 : _bmqGrpasc_snd()Send to server ERROR!!");
    close(ilSockfd);
    return(FAIL);
  }

  _bmqVdebug(2,"S0152 ���Ϳ��鱨��(�����ӷ�ʽ)���!");
  /*�ر�����*/
  close(ilSockfd);
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqGrpmsg_snd
 ** ����:   ���Ϳ��鴥������
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:lPack_head--����ͷ�� iPrior--���ȼ� aFilter--���Ĺ�������
             iDes_grp--Ŀ���� iMbid--Ŀ������
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqGrpmsg_snd(long lPack_head,short iPrior,char *aFilter,
  short iDes_grp,short iMbid)
{
  int ilRc;
  struct grprecvmsg  slRecv_grp;

  if(igDebug >= 2)
  {
    _bmqDebug("S0820 *****���ÿ��鷢�ʹ������ĺ���_bmqGrpmsg_snd(...)****");
  };
  if( igDebug >= 3)
  {
    _bmqDebug("S0830 ����[%d]����[%d]����[%d]����[%d]��������Ϣ,lPack_head,Prior=%d",
      psgMbshm->iMbgrpid,igMb_sys_current_mailbox,iDes_grp,iMbid,lPack_head,iPrior);
    _bmqDebughex(aFilter,iMASKLEN);
    
  }

  slRecv_grp.mtype        = iDes_grp;
  slRecv_grp.lPack_head   = lPack_head;
  slRecv_grp.iOrg_group   = psgMbshm->iMbgrpid;
  slRecv_grp.iOrg_mailbox = igMb_sys_current_mailbox;
  slRecv_grp.iPrior       = iPrior;
  memcpy(slRecv_grp.aFilter,aFilter,iMASKLEN);
  slRecv_grp.iDes_group   = iDes_grp;
  slRecv_grp.iDes_mailbox = iMbid;

  ilRc = msgsnd(psgMbshm->lMsgid_grp,&slRecv_grp,sizeof(struct grprecvmsg),0);
  if (ilRc < 0)
  {
     _bmqDebug("S0840 ���鷢����Ϣ���� msgsnd(....) errno:%d:%s",errno,strerror(errno)); 
     return(-1130);
  }
 
  if(igDebug >= 2)
  {
    _bmqDebug("S0850 *****�������ÿ��鷢�ʹ������ĺ���_bmqGrpmsg_snd(...)****");
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqClsmsg_snd
 ** ����:   ����C/S��������
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:lPack_head--����ͷ�� iPrior--���ȼ� aFilter--���Ĺ�������
             iDes_grp--Ŀ���� iMbid--Ŀ������
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************
static int _bmqClsmsg_snd(long lPack_head,short iPrior,char *aFilter,
  short iDes_grp,short iMbid)
{
  int ilRc;
  struct clmbinf  slRecv_cls;

  if(igDebug >= 2)
  {
    _bmqDebug("S0821 *****����C/S���ʹ������ĺ���_bmqClsmsg_snd(...)****");
  };
  if( igDebug >= 3)
  {
    _bmqDebug("S0830 ����[%d]����[%d]����[%d]����[%d]��������Ϣ,lPack_head,Prior=%d",
      psgMbshm->iMbgrpid,igMb_sys_current_mailbox,iDes_grp,iMbid,lPack_head,iPrior);
    _bmqDebughex(aFilter,iMASKLEN);
  }

  slRecv_cls.mtype        = iMbid;
  slRecv_cls.lPack_head   = lPack_head;
  slRecv_cls.iOrg_group   = psgMbshm->iMbgrpid;
  slRecv_cls.iOrg_mailbox = igMb_sys_current_mailbox;
  slRecv_cls.iPrior       = iPrior;
  memcpy(slRecv_cls.aFilter,aFilter,iMASKLEN);
  slRecv_cls.iDes_group   = iDes_grp;
  slRecv_cls.iDes_mailbox = iMbid;

  ilRc = msgsnd(psgMbshm->lMsgid_cls,&slRecv_cls,sizeof(struct clmbinf),0);
  if (ilRc < 0)
  {
     _bmqDebug("S0841 C/S������Ϣ���� msgsnd(....) errno:%d:%s",errno,strerror(errno)); 
     return(-1130);
  }
 
  if(igDebug >= 2)
  {
    _bmqDebug("S0851 *****��������C/S���ʹ������ĺ���_bmqClsmsg_snd(...)****");
  }
  return(SUCCESS);
}
del by xujun 2006.12.28
��ʹ��ͨ��������*/

/**************************************************************
 ** ������: _bmqConn_select
 ** ����:   ѡ������������������Ľ��̷���֪ͨ��Ϣ
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iMbid--Ŀ������ iPrior--���ȼ� aFilter--���Ĺ�������
             lQueue_id--��Ϣ����ID
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqConn_select(short iMbid,short iPrior,char *aFilter,
  long lQueue_id)
{
  int    ilRc,ilSize,i,ilFilter_flag;
  short  ilLink;
  FILE   *fp;
  long   llMsgid;
  char   alBuf[iFLDVALUELEN];
  struct recvmsg slRecvmsg;
  struct mbconnect slMbconnect;
  struct mbqueue   slMbqueue;
  char alResult1[iMASKLEN];
  char alResult2[iMASKLEN];
   
  if(igDebug >= 2)
  {
    _bmqDebug("S0860 ***���ú���:mbconn_select(%d,%d,aFilter,%ld)***",
      iMbid,iPrior,lQueue_id);
    _bmqDebughex(aFilter,iMASKLEN);
  }

  ilSize = sizeof(struct mbconnect) + 1;
  ilLink = psgMbinfo[iMbid-1].iConn_head;
  for(;;)
  {
    if(ilLink == 0) return 0;

    if(ilLink > psgMbshm->iMbcontcount)
    {
      if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
      {
        _bmqDebug("S0880 ���ļ�����");
        return (FAIL);
      }
      _bmqConnfile_pick(fp,&slMbconnect,ilLink);
      if(igDebug >= 2)
      {
        _bmqDebug("S0870 �������[%d]�Ƿ����ʼ���������ƥ��:tag=%d,\
          slMbconnect.aFilter,aFilter,slMbconnect.aMask",\
          ilLink, slMbconnect.cTag);
        _bmqDebughex(slMbconnect.aFilter,iMASKLEN);
        _bmqDebughex(aFilter,iMASKLEN);
        _bmqDebughex(slMbconnect.aMask,iMASKLEN);
      }

      ilRc = _bmqStrand(aFilter, slMbconnect.aMask, iMASKLEN, alResult1);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        _bmqDebughex(aFilter,iMASKLEN);
        _bmqDebughex(slMbconnect.aMask,iMASKLEN);
        _bmqClosefile(fp);
        return(FAIL);
      }
      ilRc = _bmqStrand(slMbconnect.aFilter, slMbconnect.aMask, iMASKLEN, alResult2);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        _bmqDebughex(slMbconnect.aFilter,iMASKLEN);
        _bmqDebughex(slMbconnect.aMask,iMASKLEN);
        _bmqClosefile(fp);
        return(FAIL);
      }
      
      ilFilter_flag = 0;
      for(i=0;i<iMASKLEN;i++)
      {
        if(slMbconnect.aFilter[i] != 0x00)
        {
          ilFilter_flag = 1;
          break;
        }
      }

      if ((slMbconnect.cTag == 2) && ( !ilFilter_flag || memcmp(alResult1, alResult2, iMASKLEN) == 0))
      {
   
        if(igDebug >= 2)
        {
          _bmqDebug("S0890 ����[%d]���ʼ���������ƥ��:tag=%d,\
            slMbconnect.aFilter,slMbconnect.aMask",ilLink,slMbconnect.cTag);
        }

        llMsgid = slMbconnect.lMsgid_recv;
        slMbconnect.cTag   = 1;
        slMbconnect.iPrior = iPrior;
        memcpy(slMbconnect.aFilter,aFilter,iMASKLEN);
                    
        memcpy(alBuf + 1, (char *)&slMbconnect, ilSize - 1);

        fseek(fp,0,SEEK_SET);
        ilRc = _bmqPutfile(fp, (ilLink-psgMbshm->iMbcontcount-1)*ilSize, 
          ilSize,alBuf);
        if (ilRc)
        {
          _bmqDebug("S0900 д�ļ�����");
          _bmqClosefile(fp);
          return (FAIL);
        }

        _bmqClosefile(fp);
        break;
      }
      else
      {
        ilLink = slMbconnect.iRlink;
        _bmqClosefile(fp);
      }
    }   
    else
    {
      if(igDebug >= 2)
      {
        _bmqDebug("S0875 �������[%d]�Ƿ����ʼ���������ƥ��:tag=%d,\
          psgMbconnect[ilLink-1].aFilter,aFilter,psgMbconnect[ilLink-1].aMask",\
          ilLink, psgMbconnect[ilLink-1].cTag);
        _bmqDebughex(psgMbconnect[ilLink-1].aFilter,iMASKLEN);
        _bmqDebughex(aFilter,iMASKLEN);
        _bmqDebughex(psgMbconnect[ilLink-1].aMask,iMASKLEN);
      }

      if(psgMbconnect[ilLink-1].cTag == 2)
      {
        ilRc = _bmqStrand(aFilter,psgMbconnect[ilLink-1].aMask, iMASKLEN, alResult1);
        if (ilRc !=SUCCESS)
        {  
          _bmqDebug("S0000: _bmqStrand error!");
          _bmqDebughex(aFilter,iMASKLEN);
          _bmqDebughex(psgMbconnect[ilLink-1].aMask,iMASKLEN);
          return(FAIL);
        }
        ilRc = _bmqStrand(psgMbconnect[ilLink-1].aFilter, psgMbconnect[ilLink-1].aMask, iMASKLEN, alResult2);
        if (ilRc !=SUCCESS)
        {  
          _bmqDebug("S0000: _bmqStrand error!");
          _bmqDebughex(psgMbconnect[ilLink-1].aFilter,iMASKLEN);
          _bmqDebughex(psgMbconnect[ilLink-1].aMask,iMASKLEN);
          return(FAIL);
        }
        ilFilter_flag = 0;
        for(i=0;i<iMASKLEN;i++)
        {
          if(psgMbconnect[ilLink-1].aFilter[i] != 0x00)
          {
            ilFilter_flag = 1;
            break;
          }
        }

        if ( !ilFilter_flag || (memcmp(alResult1, alResult2, iMASKLEN) == 0))
        {

          if(igDebug >= 2)
          {
            _bmqDebug("S0910 ����[%d]���ʼ���������ƥ��:tag=%d, \
              psgMbconnect[ilLink-1].aFilter,psgMbconnect[ilLink-1].aMask",\
              ilLink,psgMbconnect[ilLink-1].cTag);
          }
          llMsgid = psgMbconnect[ilLink-1].lMsgid_recv;
          psgMbconnect[ilLink-1].cTag   = 1;
          psgMbconnect[ilLink-1].iPrior = iPrior;
          memcpy(psgMbconnect[ilLink-1].aFilter,aFilter,iMASKLEN);
          break;
        }
      }       
      ilLink = psgMbconnect[ilLink-1].iRlink;
    }
  }

  slRecvmsg.mtype    = ilLink;
  slRecvmsg.lQueueid = lQueue_id; 
  if(lQueue_id > psgMbshm->lMbqueuecount)
  {
    if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
    {
      _bmqDebug("S0920 ���ļ�����");
      return (FAIL);
    }
    _bmqQueuefile_pick(fp,&slMbqueue,lQueue_id);
    slRecvmsg.lSerialnum = slMbqueue.lSerialnum;
    _bmqClosefile(fp);
  }  
  else
    slRecvmsg.lSerialnum = psgMbqueue[lQueue_id-1].lSerialnum;

  if(igDebug >= 2)  
  {
    _bmqDebug("S0930 ����Ϣ�����з�����Ϣ,mtype=%d Queue_id=%d serialnum=%d",
      slRecvmsg.mtype,slRecvmsg.lQueueid,slRecvmsg.lSerialnum);
  }
  
  ilRc = msgsnd(llMsgid,&slRecvmsg,sizeof(struct recvmsg),0);
  if (ilRc < 0)
  {
    _bmqDebug("S0940 msgsnd errno:%d:%s",errno,strerror(errno));
    return(-1130);
  }

  if(igDebug >= 2)
  {
    _bmqDebug("S0950 *****�������ú���:mbconn_select(%d,%d,aFilter,%ld)***",
      iMbid,iPrior,lQueue_id);
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueue_put
 ** ����:   �洢һ�����м�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iGrpid--���          iMbid--Ŀ������ iPrior--���ȼ� 
             aFilter--���Ĺ������� lPack_head--��������ͷ
             lQueue_id--��Ϣ����ID
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqQueue_put (short iGrpid,short iMbid,short iPrior,char *aFilter,
  long lPack_head,long *plQueue_id)
{
  int   ilRc,i;

  if(igDebug >= 2)
  {
    _bmqDebug("S0960 ***���ú��� _bmqQueue_put(%d,%d,%d,aFilter,%ld,queue_id)Q_free=%ld maxqueue=%ld***",
       iGrpid,iMbid,iPrior,lPack_head,
       psgMbshm->lMbqueue_free,psgMbshm->lMbqueuecount);
    _bmqDebughex(aFilter,iMASKLEN);
   
  }

  i = psgMbshm->lMbqueue_free;

  for(;;)
  {
    if (i >= psgMbshm->lMbqueuecount) i = 0;
    if (psgMbqueue[i].cTag == 0) break;
    i ++;
    if (i == psgMbshm->lMbqueue_free)
    {
      ilRc = _bmqQueuefile_put(iGrpid,iMbid,iPrior,aFilter,lPack_head,plQueue_id);
      if(ilRc == FAIL)
      {
        _bmqDebug("S0970 ���ļ���д����г���,_bmqQueuefile_put(..).");
        return(1003);
      }

  if( igDebug >= 2)
  {
    _bmqDebug("S0980 ***���д浽�ļ�,�������ú���:_bmqQueue_put(...)***");
  }
      return(SUCCESS);
    }
  }
  
  if(igDebug >=2)
     _bmqDebug("S0990 ʹ�ÿ��ж���[%d]",i+1);
  *plQueue_id = i+1;
  psgMbshm->lMbqueue_free = i + 1;

  /*����һ�����м�¼*/
  ilRc = _bmqQueue_add(i + 1,iGrpid,iMbid,iPrior,aFilter,lPack_head,&psgMbqueue[i]);  
  if(ilRc == FAIL)
    {
      _bmqDebug("S1000 ���ڴ�д����м�¼����,_bmqQueue_add(...).");  
      return(1013);
    }

  if( igDebug >= 2)
  {
    _bmqDebug("S1010 ***���д浽�ڴ�,�������ú���:_bmqQueue_put(...)***");
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqPack_put
 ** ����:   ����һ�����ļ�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:aMsgbuf--���Ĵ洢buf iMsglen--���ĳ��� 
             plPack_head--���Ĵ洢��¼����ͷ
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*static int _bmqPack_put(char *aMsgbuf,short iMsglen,long *plPack_head)delete by wh*/
static int _bmqPack_put(char *aMsgbuf,TYPE_SWITCH iMsglen,long *plPack_head)/*add by wh*/
{
  long  j,k;
  /*short ilMsglen;delete by wh*/
  TYPE_SWITCH ilMsglen;
  int   ilRc;
  long  i,llFile_head;

  if(igDebug >= 2)
    _bmqDebug("S1020 ***���ú���:_bmqPack_put(Msgbuf,%d,Pack_head) Pack_free=[%d]***",iMsglen,psgMbshm->lMbpack_free);

  i = psgMbshm->lMbpack_free;
  ilMsglen = iMsglen;
  *plPack_head = 0;
  k = 0;
  for(;;)
  {
    if (psgMbpack[i].iMsglen)
    {
      i++;
      if (i>=psgMbshm->lMbpacksize)
      {
        i = 0;
        /*continue;*/
      }
      if (i == psgMbshm->lMbpack_free)
      {
        if(igDebug >= 2)
          _bmqDebug("S1030 �����ڴ������,������ļ���");
        ilRc = _bmqPackfile_put(aMsgbuf+k*iMBPERPACKSIZE,ilMsglen,&llFile_head);
                if(ilRc == FAIL) 
        {
          _bmqDebug("S1040 ����д�뻺���ļ�ʧ��");
          return(1002);
        }
        
        if ((*plPack_head) == 0)
        {
          *plPack_head = llFile_head + psgMbshm->lMbpacksize;
        }
        else
        {
          psgMbpack[j].lLink = llFile_head + psgMbshm->lMbpacksize;
        }
        if(igDebug >= 2)
        {
          _bmqDebug("S1050 ***�������ú���:_bmqPack_put(...)***");
        }
        return(SUCCESS);
      }
      continue;
    }

    /*������ڴ��*/
    if ((*plPack_head) == 0) 
    {
      *plPack_head = i+1;
    }
    else
    {
      psgMbpack[j].lLink = i+1;
    }
    if (ilMsglen > iMBPERPACKSIZE)
    {
      memcpy(psgMbpack[i].aPack,aMsgbuf + k * iMBPERPACKSIZE,iMBPERPACKSIZE);
      psgMbpack[i].iMsglen = iMBPERPACKSIZE;

      ilMsglen = ilMsglen - iMBPERPACKSIZE;
    }
    else
    {
      memcpy(psgMbpack[i].aPack,aMsgbuf + k * iMBPERPACKSIZE,ilMsglen);
      psgMbpack[i].iMsglen = ilMsglen;
      psgMbpack[i].lLink = 0;

      if(igDebug >= 2)
      {
        _bmqDebug("S1060 ���汨�ĵ��ڴ�:�ڴ��ID=%d,�ʼ�ͷ=%d,msglen=%d",
          i+1,*plPack_head,psgMbpack[i].iMsglen);
      }
      break;
    }
    if(igDebug >= 2)
    {
      _bmqDebug("S1070 ���汨�ĵ��ڴ�:�ڴ��ID=%d,�ʼ�ͷ=%d,msglen=%d",
        i+1,*plPack_head,psgMbpack[i].iMsglen); 
    }

    k ++;
    j = i;
  }
  psgMbshm->lMbpack_free = i;

  if(igDebug >= 2)
    {
      _bmqDebug("S1080 ***�������ú���:_bmqPack_put(...)***");
    }

  return(SUCCESS);
}
/**************************************************************
 ** ������: _bmqPack_get
 ** ����:   ��ȡһ�����ļ�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:aMsgbuf--���Ĵ洢buf iMsglen--���ĳ���
             lPack_head--���Ĵ洢��¼����ͷ
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*int _bmqPack_get(char *aMsgbuf,short *piMsglen,long lPack_head)delete by wh*/
int _bmqPack_get(char *aMsgbuf,TYPE_SWITCH *piMsglen,long lPack_head)/*add by wh*/
{
  long    i;
  FILE    *fp;
  int     ilRc;
  struct  mbfilepack slMbfilepack;
 
  if(igDebug >= 2) 
    _bmqDebug("S1090 ***���ú���:_bmqPack_get() lPack_head=%d***",lPack_head);

  /* add by xujun 2001.10.16*/
  if (lPack_head <= 0)
  {
    _bmqDebug("S0191 �Ƿ��ı���ͷֵ:%ld",lPack_head);
    return(FAIL);
  }
  /* add end by xujun*/

  if (lPack_head > psgMbshm->lMbpacksize )
  {
    /*ȡ�����Ĵ洢������¼*/
    if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
    {
      _bmqDebug("S1100 ���ļ�����!");
      return (FAIL);
    }
    _bmqPackfile_pick(fp,&slMbfilepack,lPack_head);

    *piMsglen = slMbfilepack.lLoglen;
    ilRc =  _bmqGetlog(slMbfilepack.aLogfile,*piMsglen,
      slMbfilepack.lLogoffset,aMsgbuf);
    if (ilRc) 
    {
      _bmqClosefile(fp);
      return(ilRc);
    }
    /*���±��Ĵ洢������¼*/
    _bmqPackfile_set(fp,0,0,lPack_head);
    _bmqClosefile(fp);

    if(igDebug >= 2)
    {
      _bmqDebug("S1110 ���ļ���ȡ����:file=[%s],offset=[%ld],len=[%d]",
        slMbfilepack.aLogfile,
          slMbfilepack.lLogoffset,*piMsglen);
    }
    return(SUCCESS);
  }
  i = lPack_head;
  *piMsglen = 0;
  for(;;)
  {
    memcpy(aMsgbuf+(*piMsglen),psgMbpack[i-1].aPack,
      psgMbpack[i-1].iMsglen);
    *piMsglen = *piMsglen + psgMbpack[i-1].iMsglen;

    if(igDebug >= 2)
    {
      _bmqDebug("S1120 ���ڴ���ȡ����:ID=[%d],len=[%d]",i,psgMbpack[i-1].iMsglen);
    }
    psgMbpack[i-1].iMsglen = 0;
    i = psgMbpack[i-1].lLink;

    if (i > psgMbshm->lMbpacksize)
    {
      /*ȡ�����Ĵ洢������¼*/
      if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
      {
        _bmqDebug("S1130 ���ļ�����");
        return (FAIL);
      }
      _bmqPackfile_pick(fp,&slMbfilepack,i);

      ilRc =  _bmqGetlog(slMbfilepack.aLogfile,slMbfilepack.lLoglen,
      slMbfilepack.lLogoffset,aMsgbuf+*piMsglen);
      if (ilRc)
      {
        _bmqDebug("S1140 �Ӽ�¼�ļ�����ȡ���ĳ���,_bmqGetlog(..)");
        _bmqClosefile(fp);
        return(ilRc);
      }
      /*���±��Ĵ洢������¼*/
      _bmqPackfile_set(fp,0,0,i);
      _bmqClosefile(fp);

      *piMsglen = *piMsglen + slMbfilepack.lLoglen;

      if(igDebug >= 2)
      {
        _bmqDebug("S1150 ���ļ���ȡ����:file=[%s],offset=[%ld],len=[%d]",
          slMbfilepack.aLogfile,
            slMbfilepack.lLogoffset,
              slMbfilepack.lLoglen);
      }
      if (igDebug >= 2) _bmqDebug("S1160 ***�������ú���:_bmqPack_get()***");
      return(SUCCESS);
    }

    if (i == 0) break;
  }
  if (igDebug >= 2) _bmqDebug("S1170 ***�������ú���:_bmqPack_get()***");
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueue_get
 ** ����:   ��ȡһ�����м�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:piGrpid--���   piMbid--Ŀ������?piPrior--���ȼ� 
             aFilter--���Ĺ�������  aMask--��������   
             plPack_head--��������ͷ
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
*****************************************************************************************************/
static int _bmqQueue_get(short *piGrpid,short *piMbid,short *piPrior,
char *aFilter,char *aMask,long *plPack_head)
{
  int    ilSize,ilRc,i,ilFilter_flag;
  FILE   *fp;
  long   llQueue_head;
  char   alBuf[iFLDVALUELEN];
  struct mbqueue slMbqueue;
  char alResult1[iMASKLEN], alResult2[iMASKLEN];
  
  if( igDebug >=2)
  {
    _bmqDebug("S1180 ***������ȡһ�����м�¼����:_bmqQueue_get(%d,%d,%d,aFilter,aMask)***",
      *piGrpid,*piMbid,*piPrior);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }

  ilFilter_flag = 0;
  for(i=0;i<iMASKLEN;i++)
  {
    if(aFilter[i] != 0x00)
    {
      ilFilter_flag = 1;
      break;
    }
  }

  /* ȡ��������Ϣ����ͷ */
  llQueue_head = psgMbinfo[igMb_sys_current_mailbox - 1].lQueue_head;
  ilSize       = sizeof(struct mbqueue) + 1;
  
  for(;;)
  {
    /* δ�ҵ�ָ����¼ */
    if (llQueue_head == 0)
      return(1001);
     
    /* ��¼�ڻ����ļ��� */
    if (llQueue_head > psgMbshm->lMbqueuecount)
    {
      /* �򿪶��л����ļ� */
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1190 ���ļ�����");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_pick(fp,&slMbqueue,llQueue_head);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1200 ���л����ļ���ȡ��¼ʧ��!");
        _bmqClosefile(fp);
        _bmqUnlock(LK_ALL);
        return(FAIL);
      }

      /* ƥ����Ϣ Filter,Mask */
      
      ilRc = _bmqStrand(aFilter, aMask, iMASKLEN, alResult1);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      ilRc = _bmqStrand(slMbqueue.aFilter, aMask, iMASKLEN, alResult2);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }

      if (!ilFilter_flag || memcmp(alResult1, alResult2, iMASKLEN) == 0)
      {
        ilRc = _bmqQueue_del(igMb_sys_current_mailbox,&slMbqueue,piGrpid,piMbid,
                             piPrior,aFilter,plPack_head);
        if(ilRc == WITHOUTUSE)
        {
          _bmqDebug("S1208 ɾ���Ķ���[%d]û������",llQueue_head);
          return (FAIL);
        }

        /* ��д�ļ� */
        alBuf[0] = '0';
        fseek(fp,0,SEEK_SET);
        ilRc = _bmqPutfile(fp, (llQueue_head-psgMbshm->lMbqueuecount-1)*ilSize, 
          ilSize,alBuf);
        if (ilRc)
        {
          _bmqDebug("S1210 д�ļ�����");
          _bmqClosefile(fp);
          return (FAIL);
        }
        _bmqClosefile(fp);
        break;
      }
      else
      {
        llQueue_head = slMbqueue.lRlink;
        _bmqClosefile(fp);
      }
    }
    /* ��¼�ڹ����ڴ��� */
    else
    {
      /* ƥ����Ϣ Filter,Mask */
      ilRc = _bmqStrand(aFilter, aMask, iMASKLEN, alResult1);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      ilRc = _bmqStrand(psgMbqueue[llQueue_head-1].aFilter,aMask,iMASKLEN,alResult2);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }

      if (!ilFilter_flag || memcmp(alResult1, alResult2, iMASKLEN) == 0)
      {
        ilRc = _bmqQueue_del(igMb_sys_current_mailbox,
                             &psgMbqueue[llQueue_head-1],
                             piGrpid,piMbid,piPrior,aFilter,plPack_head);
        if(ilRc == WITHOUTUSE)
        {
          _bmqDebug("S0672 ɾ���Ķ���[%d]û������",llQueue_head);
          return (FAIL);
        }
        break;
      }
      else
        llQueue_head = psgMbqueue[llQueue_head-1].lRlink;
    } 
  }   

  if( igDebug >=2)
  {
    _bmqDebug("S1220 ***�������ú���:_bmqQueue_get(...)***");
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueue_del
 ** ����:   ������ɾ��һ�����м�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iMbid--��ǰ����? slMbqueue--���нṹ 
             piGrpid--���     iMbid--�����
             piPrior--���ȼ�   aFilter--���Ĺ�������
             lPack_head--��������ͷ  
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqQueue_del(short iMbid,struct mbqueue *slMbqueue,short *piGrpid,
  short *piMbid,short *piPrior,char *aFilter,long *plPack_head)
{
  long  llRlink,llLlink;
  int   ilRc;
  FILE  *fp;
 
  if( igDebug >=2)
  {
    _bmqDebug("S1230 ����(������ɾ��һ�����м�¼)���� _bmqQueue_del(%d,%d,%d,aFilter)",
      *piGrpid,*piMbid,*piPrior);
    _bmqDebughex(aFilter,iMASKLEN);
  }

  if( !slMbqueue->cTag )
  {
    _bmqDebug("S1232 ����û������,���Ŀ����ѱ���������ȡ��");
    return(WITHOUTUSE);
  }

  *piGrpid     = slMbqueue->iOrg_grp;
  *piMbid      = slMbqueue->iOrg_mailbox;
  *piPrior     = slMbqueue->iPrior;
  memcpy(aFilter,slMbqueue->aFilter,iMASKLEN);
  *plPack_head = slMbqueue->lPack_head;

  llRlink = slMbqueue->lRlink;
  llLlink = slMbqueue->lLlink;

  slMbqueue->cTag       = 0;
  slMbqueue->lLlink     = 0;
  slMbqueue->lRlink     = 0;
  slMbqueue->lSerialnum = 0;
  

  /* ǰ���ж��� */
  if (llLlink)
  {
    if(llLlink > psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1240 ���ļ�����");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_set(fp,-1, -1, llRlink, llLlink);
      _bmqClosefile(fp);
      if (ilRc == FAIL) return (FAIL);
    }
    else
    {
      psgMbqueue[llLlink-1].lRlink = llRlink;
    }
  }
  /* ����ͷ */
  else
  {
    psgMbinfo[iMbid - 1].lQueue_head = llRlink;
  }

  /* �����ж��� */
  if (llRlink)
  {
    if(llRlink > psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1250 ���ļ�����");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_set(fp, -1, llLlink, -1,llRlink);
      _bmqClosefile(fp);
      if (ilRc == FAIL) return (FAIL);
    }
    else
    {
      psgMbqueue[llRlink-1].lLlink = llLlink;
    }
  }
  /* ����β */
  else
  {
    psgMbinfo[iMbid - 1].lQueue_tail = llLlink;
  }
  if ( igDebug >= 2 )
    {
      _bmqDebug("S1260 ***�������ú���  _bmqQueue_del(...)***");
    }
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueue_add
 ** ����:   ����������һ�����м�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:lQueue_id--���м�¼ID?iGrpid--��� iMbid--�����
             iPrior--���ȼ� aFilter--���Ĺ�������
             lPack_head--��������ͷ  
             psMbqueue--���нṹָ��
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqQueue_add(long lQueue_id,short iGrpid,short iMbid,short iPrior,
  char *aFilter,long lPack_head,struct mbqueue *psMbqueue)
{
  int    ilRc;
  long   llQueue_head,llQueue_tail;
  
  if(igDebug >= 2)
  {
    _bmqDebug("S1270 ***���ú���_bmqQueue_add(%ld,%d,%d,%d,%ld,aFilter,%ld)****",
      lQueue_id,iGrpid,iMbid,iPrior,lPack_head);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebug("S1280 ����_bmqQueue_add(..)����:����������һ�����м�¼");
  }
  
  psgMbshm->lgMb_sys_queue_serial_num ++;

  /* �ṹ��ֵ */
  psMbqueue->cTag         = 1;
  psMbqueue->lSerialnum   = psgMbshm->lgMb_sys_queue_serial_num;
  psMbqueue->iPrior       = iPrior;
  memcpy(psMbqueue->aFilter,aFilter,iMASKLEN);
  psMbqueue->iOrg_grp     = igMb_sys_current_group;
  psMbqueue->iOrg_mailbox = igMb_sys_current_mailbox;
  psMbqueue->lPack_head   = lPack_head;
  time(&(psMbqueue->lBegintime));

  if(igDebug >= 2)
    _bmqDebug("S1290 �������ݰ���ˮ��[%d]",psgMbshm->lgMb_sys_queue_serial_num);

   llQueue_head = psgMbinfo[iMbid-1].lQueue_head;  
   llQueue_tail = psgMbinfo[iMbid-1].lQueue_tail;  

  /* �������� */
  ilRc = _bmqQueue_sort(iMbid,lQueue_id,llQueue_head,llQueue_tail,psMbqueue);
  if(ilRc == FAIL)
  {
    _bmqDebug("S1300 ��������ERROR!");
    return(FAIL);
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueue_sort
 ** ����:   �����¼�ڶ��������а����ȼ�����
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iMbid--Ŀ������ lQueue_id--��Ϣ����ID 
             psMbqueue--���нṹָ��
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqQueue_sort(short iMbid,long lQueue_id,long lQueue_head,
  long lQueue_tail,struct mbqueue *psMbqueue)
{
  int    ilRc,ilSize;
  FILE   *fp;
  char   alBuf[iFLDVALUELEN];
  short  ilPrior;
  struct mbqueue slMbqueue;

  if( igDebug >= 2)
  {
    _bmqDebug("S1310 ***���ú��� _bmqQueue_sort(%d,%ld,%ld,Mbqueue)***",iMbid,lQueue_id,lQueue_head);
  }
    
  /*�ն���,���ڶ���ͷ*/
  if (lQueue_head == 0)
  {
    psMbqueue->lLlink = 0;
    psMbqueue->lRlink = 0;
    psgMbinfo[iMbid-1].lQueue_head = lQueue_id;
    psgMbinfo[iMbid-1].lQueue_tail = lQueue_id;

    if(igDebug >= 2)
    {
      _bmqDebug("S1320 �ն���,���ڶ���ͷ,����Ķ���ͷ[%d] %d %d",
        psgMbinfo[iMbid-1].lQueue_head,
          psMbqueue->lLlink,
            psMbqueue->lRlink);
    }
    return(SUCCESS);
  }
  
  ilPrior = psMbqueue->iPrior;
  ilSize  = sizeof(struct mbqueue) + 1;
  /*�����ȼ�����*/
  for(;;)
  {
    if( lQueue_tail > psgMbshm->lMbqueuecount )
    {
      /*���ļ�ȡ����¼���бȽ�*/
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1330 ���ļ�����");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_pick(fp,&slMbqueue,lQueue_tail);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1340 ���л����ļ���ȡ��¼ _bmqQueuefile_pick(..)���ش���");
        _bmqClosefile(fp);
        return(FAIL);
      }
      if ( (slMbqueue.iPrior >= ilPrior) || (slMbqueue.lLlink == 0) )
      {
        if( slMbqueue.iPrior >= ilPrior)
        {
          psMbqueue->lRlink = slMbqueue.lRlink;
          psMbqueue->lLlink = lQueue_tail;
          slMbqueue.lRlink  = lQueue_id;
          if(igDebug >= 2)
            _bmqDebug("S1350 [%d]���ڵ�ǰ����[%d]ǰ",lQueue_id,lQueue_head);
          _bmqQueueInsert_Before(iMbid,lQueue_id,psMbqueue->lRlink);
        }
        else
        {
          psMbqueue->lLlink = 0;
          psMbqueue->lRlink = lQueue_tail;
          slMbqueue.lLlink  = lQueue_id;
          psgMbinfo[iMbid-1].lQueue_head = lQueue_id;
        }
        /*д�ļ���¼*/
        alBuf[0] = '1';
        memcpy(alBuf + 1, (char *)&slMbqueue, ilSize - 1);
        fseek(fp,0,SEEK_SET);
        ilRc = _bmqPutfile(fp, (lQueue_tail-psgMbshm->lMbqueuecount-1)*ilSize,
          ilSize, alBuf);
        if (ilRc)
        {
          _bmqDebug("S1360 д�ļ�����!");
          _bmqClosefile(fp);
          return (FAIL);
        }
        _bmqClosefile(fp);
        break;
      }
      lQueue_tail = slMbqueue.lLlink;
      _bmqClosefile(fp);
    }
    else
    {
      if (psgMbqueue[lQueue_tail-1].iPrior >= ilPrior)
      {
        psMbqueue->lLlink  = lQueue_tail;
        psMbqueue->lRlink  = psgMbqueue[lQueue_tail-1].lRlink;
        psgMbqueue[lQueue_tail-1].lRlink = lQueue_id;
        if(igDebug >= 2)
          _bmqDebug("S1370 [%d]���ڵ�ǰ����[%d]ǰ",lQueue_id,lQueue_head);
        _bmqQueueInsert_Before(iMbid,lQueue_id,psMbqueue->lRlink);
        break;
      }
      if (psgMbqueue[lQueue_tail-1].lLlink == 0)
      {
        psMbqueue->lLlink  = 0;
        psMbqueue->lRlink  = lQueue_tail;
        psgMbqueue[lQueue_tail-1].lLlink = lQueue_id;
        psgMbinfo[iMbid-1].lQueue_head = lQueue_id;
        break;
      }
      lQueue_tail = psgMbqueue[lQueue_tail-1].lLlink;
    }
  }

 if( igDebug >= 2)
  {
    _bmqDebug("S1380 ***�������ú��� _bmqQueue_sort(...)***");
  }

  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueueInsert_Before
 ** ����:   ��¼���ڵ�ǰ����ǰ
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iMbid--Ŀ������?lQueue_id--��Ϣ����ID
             lQueue_Llink--���м�¼����
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqQueueInsert_Before(short iMbid,long lQueue_id,long lQueue_Rlink)
{
  int    ilRc;
  FILE   *fp;

  if( igDebug >= 2)
  {
    _bmqDebug("S1390 ****���ú���_bmqQueueInsert_Before(%d,%ld,%ld)****",
      iMbid,lQueue_id,lQueue_Rlink);
  }   
  if(lQueue_Rlink)
  {
    if(lQueue_Rlink > psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1400 ���ļ�����");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_set(fp, -1, lQueue_id,-1, lQueue_Rlink);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1410 _bmqQueuefile_set(..)���ش���!"); 
        _bmqClosefile(fp);
        return(FAIL);
      }
      _bmqClosefile(fp);
    }
    else
      psgMbqueue[lQueue_Rlink - 1].lLlink = lQueue_id;
  }
  else
  {
    if(igDebug >= 2)
      _bmqDebug("S1420 ����Ķ���ͷ[%d]",psgMbinfo[iMbid-1].lQueue_head);
    psgMbinfo[iMbid-1].lQueue_tail = lQueue_id;
  }
  if( igDebug >= 2)
  {
    _bmqDebug("S1430 ****�������ú���_bmqQueueInsert_Before(..)****");
  }
  return(0);
}

/**************************************************************
 ** ������: _bmqQueue_move
 ** ����:   ���ļ����ƶ�һ����¼�������ڴ�
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fp--�����ļ�ָ��   lQueue_id--��Ϣ����ID
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/* delete by szhengye 2001.10.29
static int _bmqQueue_move(FILE *fp,long lQueueid)
{
  int    ilRc;
  long   llQueue_head;
  short  ilPrior,i;
  struct mbqueue slMbqueue;

  if(igDebug >= 2)
    _bmqDebug("S1440 ***���ú��� _bmqQueue_move(%d)***",lQueueid);
 
  for(i=0;i<psgMbshm->iMbinfo;i++)
  {
    llQueue_head = psgMbinfo[i].lQueue_head;
    if(llQueue_head == 0) continue;
    for(;;)
    {
      if(llQueue_head == 0) break;
      if(llQueue_head > psgMbshm->lMbqueuecount)
      {
        ilRc = _bmqQueuefile_pick(fp,&slMbqueue,llQueue_head);
        if (ilRc == FAIL) return (FAIL);
        memcpy(&psgMbqueue[lQueueid - 1],&slMbqueue,sizeof(struct mbqueue));
        ilRc = _bmqQueuefile_set(fp, 0, 0, 0, llQueue_head);
        if (ilRc == FAIL) return (FAIL);
        if(slMbqueue.lLlink == 0) 
        {
          psgMbinfo[i].lQueue_head = lQueueid;
        }
        else
          psgMbqueue[slMbqueue.lLlink - 1].lRlink = lQueueid;
        if(slMbqueue.lRlink > psgMbshm->lMbqueuecount)
        {
          ilRc = _bmqQueuefile_set(fp, -1, lQueueid, -1, slMbqueue.lRlink);
          if (ilRc == FAIL) return (FAIL);
        }
        else 
        {
          if( slMbqueue.lRlink )
            psgMbqueue[slMbqueue.lRlink - 1].lLlink = lQueueid;
        }
        ilPrior = psgMbqueue[lQueueid - 1].iPrior;
      
        if(igDebug >= 2)
           _bmqDebug("S1450 ***�������ú��� _bmqQueue_move(..)***");
        return(FINISH);
      }
      llQueue_head = psgMbqueue[llQueue_head - 1].lRlink;
    }
  }

  if(igDebug >= 2)
    _bmqDebug("S1460 ***�������ú��� _bmqQueue_move(..)***");
  return(SUCCESS);
}
 delete by szhengye 2001.10.29 */

/**************************************************************
 ** ������: _bmqConnect_add
 ** ����:   ����һ�����Ӽ�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:_bmqConnfile_put,_bmqConnect_update
 ** ȫ�ֱ���:
 ** ��������:iMbid--��ǰ�����
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
int _bmqConnect_add(short iMbid,long lMsgid)
{
  int ilRc,i;
  
  if( igDebug >= 2)
  {
    _bmqDebug("S1470 ***���ú��� _bmqConnect_add(%d)*** ",iMbid);
  }
 
  psgMbinfo[iMbid-1].iConnect ++;
    
  i = psgMbshm->iMbconnect_free;

  /*�ҳ�������*/
  for(;;)
  {
    if (i>=psgMbshm->iMbcontcount) i = 0;
    if (psgMbconnect[i].cTag == 0) break;
    i ++;
    /*�����ڴ�����*/
    if (i == psgMbshm->iMbconnect_free)
    {
      /*д�뻺���ļ�*/
      ilRc = _bmqConnfile_put(iMbid,lMsgid);
      if(ilRc == FAIL) 
      {
        _bmqDebug("S1480 д�������������ļ�����!");
        return(1005);
      }
      return(SUCCESS);
    }
  }

  psgMbshm->iMbconnect_free = i + 1;
  igMb_sys_current_connect  = i + 1;

  /*������������¼����*/
  ilRc = _bmqConnect_update(iMbid,&psgMbconnect[i]);
  if(ilRc == FAIL)
  {
    _bmqDebug("S1490 ������������¼���ݳ���!");
    return(FAIL);  
  }
  psgMbconnect[i].lMsgid_recv = lMsgid;

  if( igDebug >= 2)
  {
    _bmqDebug("S1500 ***�������ú��� _bmqConnect_add(%d)*** ",iMbid);
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqConnect_del
 ** ����:   ɾ��һ�����Ӽ�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iMbid--��ǰ����   psMbconnect--���ӽṹָ��
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
int _bmqConnect_del(short iMbid,struct mbconnect *psMbconnect)
{
  int    ilRc;
  short  ilLlink,ilRlink;
  
  if( igDebug >= 2)
    _bmqDebug("S1510 ****���ú���_bmqConnect_del(%d,*psMbconnect)****",iMbid);

  psgMbinfo[iMbid].iConnect --;

  ilLlink = psMbconnect->iLlink;
  ilRlink = psMbconnect->iRlink;

  psMbconnect->cTag   = 0;
  psMbconnect->iLlink = 0;
  psMbconnect->iRlink = 0;

  if (ilLlink)
  { 
    if(ilLlink > psgMbshm->iMbcontcount)
    {
      /*���¼�¼����*/
      ilRc = _bmqConnfile_set(-1,-1,ilRlink,psMbconnect->aFilter,psMbconnect->aMask,ilLlink,-1);
      if(ilRc == FAIL)
      {
        _bmqDebug("S1520 ���¼�¼�������� _bmqConnfile_set() ���س���");
        return (FAIL);
      }
    }
    else
      psgMbconnect[ilLlink-1].iRlink = ilRlink;
  }
  else
  {
    /*��������ͷ*/
    psgMbinfo[iMbid].iConn_head = ilRlink;
  }
  
  if (ilRlink)
  {
    if(ilRlink > psgMbshm->iMbcontcount)
    {
      /*���¼�¼����*/
      ilRc = _bmqConnfile_set(-1,ilLlink,-1,psgMbconnect->aFilter,psgMbconnect->aMask,ilRlink,-1);
      if(ilRc == FAIL)
      {
        _bmqDebug("S1530 ���¼�¼����_bmqConnfile_set() ���س���");
        return (FAIL);
      }
    }
    else
      psgMbconnect[ilRlink-1].iLlink = ilLlink;
  }
  else
  {
    /*��������β*/
    psgMbinfo[iMbid].iConn_tail = ilLlink;
  }
  /* next 7 rows add by nh 2001.11.29*/
  ilRc = _bmqGetfrommsg(iMbid+1,psMbconnect->iPrior,psMbconnect->lMsgid_recv);
  if(ilRc)
  {
    _bmqDebug("S0000 ���ú���_bmqGetfrommsg()����");
    return(FAIL);
  }

  ilRc = msgctl(psMbconnect->lMsgid_recv,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1850 ɾ����Ϣ���� msgctl(RM-recv) ���� errno:%d:%s",errno,strerror(errno));
    return(FAIL);
  }
  psMbconnect->lMsgid_recv = 0;
  
  if( igDebug >= 2)
    _bmqDebug("S1540 ***�������ú���_bmqConnect_del(%d,*psMbconnect)***",iMbid);
  return(SUCCESS);
}

/**************************************************************
 ** ������      :_bmqStrand
 ** ����        : 
 ** ����        : 
 ** ��������    :2001/9/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : SUCCESS
***************************************************************/
int _bmqStrand(char *aBuf1,char *aBuf2,short iLen,char *aResult)
{
  short i;
  for(i = 0;i < iLen; i++)
    *(aResult++) = *(aBuf1++) & *(aBuf2++);
    
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqConnect_update
 ** ����:   �������Ӽ�¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iMbid--��ǰ����   psMbconnect--���ӽṹָ��
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
static int _bmqConnect_update(short iMbid,struct mbconnect *psMbconnect)
{
  int  ilRc;
 
  if( igDebug>=2 )
    _bmqDebug("S1550 ***���ú���_bmqConnect_update(%d, *psMbconnect)***",iMbid);       
  psMbconnect->cTag = 1;
  psMbconnect->lConnpid = getpid();

  /*�����ڶ�β*/
  if (psgMbinfo[iMbid-1].iConn_tail != 0)
  {
    /*β���ڻ����ļ���*/
    if(psgMbinfo[iMbid-1].iConn_tail > psgMbshm->iMbcontcount)
    {
      ilRc = _bmqConnfile_set(-1,-1,igMb_sys_current_connect,psMbconnect->aFilter,
                psMbconnect->aMask,psgMbinfo[iMbid-1].iConn_tail,-1);
      if(ilRc == FAIL)
      {
        _bmqDebug("S1560 _bmqConnfile_set() ���س���");
        return (FAIL);
      }
    }
    else
      psgMbconnect[psgMbinfo[iMbid-1].iConn_tail-1].iRlink =
        igMb_sys_current_connect;
  }
  psMbconnect->iLlink = psgMbinfo[iMbid-1].iConn_tail;
  psMbconnect->iRlink = 0;

  /*����ͷ����β��*/
  if (psgMbinfo[iMbid-1].iConn_head == 0)
    psgMbinfo[iMbid-1].iConn_head = igMb_sys_current_connect; 
    
  psgMbinfo[iMbid-1].iConn_tail = igMb_sys_current_connect; 

  if(igDebug >= 2)
  {
    _bmqDebug("S1570 �������,��������ID:head=%d,tail=%d,Llink=%d,Rlink=%d",
       psgMbinfo[iMbid-1].iConn_head, psgMbinfo[iMbid-1].iConn_tail,
         psMbconnect->iLlink,psMbconnect->iRlink);
  }
  if( igDebug>=2 )
    _bmqDebug("S1580 ***���ú���_bmqConnect_update(%d, *psMbconnect)***",iMbid);
  return(SUCCESS);
}

/**************************************************************
 ** ������:_bmqConnWait_set
 ** ����:  ����������ȡ����
 ** ����:  ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:_bmqConnfile_set
 ** ȫ�ֱ���:
 ** ��������:iConid--���Ӽ�¼ID aFilter--���Ĺ�������
 	     aMask--�������� 
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
int _bmqConnWait_set(short iConid,char *aFilter,char *aMask)
{
  int  ilRc;
  
  if( igDebug>=2 )
  {
    _bmqDebug("S1590 ***���ú��� _bmqConnWait_set(%d,aFilter,aMask)***",
              iConid);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }

  if(iConid > psgMbshm->iMbcontcount)
 {
    /*��ǰ���Ӽ�¼���ļ���*/
    /*�������������� aFilter aMask */
    ilRc = _bmqConnfile_set(2, -1, -1, aFilter,aMask,iConid,0);
    if(ilRc == FAIL)
    {
      _bmqDebug("S1600 _bmqConnfile_set() ���س���");
      return (FAIL);
    }
  }
  /*��ǰ�������ڴ�*/
  else
  {
    psgMbconnect[iConid - 1].cTag = 2;
    memcpy(psgMbconnect[iConid - 1].aFilter,aFilter,iMASKLEN);
    memcpy(psgMbconnect[iConid - 1].aMask,aMask,iMASKLEN);
  }

  if( igDebug>=2 )
  {
    _bmqDebug("S1610 ***�������ú��� _bmqConnWait_set(..)***");
  }
  return(0);
}

/**************************************************************
 ** ������: _bmqGetlog
 ** ����:   �ӱ��Ĵ洢�ļ�����ȡһ�����Ĵ洢��¼
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:logfile--log�ļ��� loglen--log���ĳ���
             logoffset--�ļ�ƫ����  msgpack--���Ĵ洢buf
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*static int _bmqGetlog(char * logfile,short loglen,long logoffset,char *msgpack) delete by wh*/
static int _bmqGetlog(char * logfile,TYPE_SWITCH loglen,long logoffset,char *msgpack)
{
  FILE *fp;
  long llLen;
  char amFp_name[80];

  if(igDebug >= 2)
    _bmqDebug("S1620 ***���ú���_bmqGetlog(logfile,loglen,logoffset,msgpack**");

  memset(amFp_name,0x00,80);
  sprintf(amFp_name,"%s/bmqlog/logfile/%s",getenv("BMQ_PATH"),logfile);
  fp=fopen(amFp_name,"r");
  if (fp==NULL) return(-1023);
  fflush(fp);
  llLen = fseek(fp,logoffset,SEEK_SET);
  if (llLen) return(-1110);
  llLen = fread(msgpack,sizeof(char),loglen,fp);
  fclose(fp);

 if(igDebug >= 2)
    _bmqDebug("S1630 ***���ú��� _bmqGetlog(....)***");

  if (llLen >= 0) return (0);
  else return (-1120);
}

/**************************************************************
 ** ������: _bmqPutlog
 ** ����:   ���Ĵ洢�ļ��д��һ�����Ĵ洢��¼
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:logfile--log�ļ��� loglen--log���ĳ���
             logoffset--�ļ�ƫ����  msgpack--���Ĵ洢buf
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
****************************************************************/
static int _bmqPutlog(char *msgpack,short loglen,long *logoffset,char * logfile)
{
  static short ilFileid = 1;  /*��̬�ļ�����׺��ֵ��ʼֵ*/
  long   llLen;
  FILE   *fp;
  long   ilPid;
  char   amFp_name[80];

  if (igDebug >=2 )
    _bmqDebug("S1640 **���ú��� _bmqPutlog(msgpack,loglen,logoffset,logfile)*");
  ilPid = getpid();
  memset(amFp_name,0x00,80);
  sprintf(amFp_name,"%s/bmqlog/logfile/bmq%ld.%d",
    getenv("BMQ_PATH"),ilPid,ilFileid);

  for(;;)
  {
    fp=fopen(amFp_name,"a+");
    if(fp==NULL) 
    {
      _bmqDebug("S1650 �޷����ļ�[%s]",amFp_name);
      _bmqDebug("S1651 ���ļ�����errno:%d:%s",errno,strerror(errno));
      return(-1023);
    }
    fflush(fp);
    llLen=fseek(fp, 0, SEEK_END);
    if ( llLen ) 
    {
      _bmqDebug("S1660 ����־�ļ�����:fseek():error");
      _bmqDebug("S1661 ��λ��־�ļ�����errno:%d:%s",errno,strerror(errno));
      return(-1110);
    }
    *logoffset = ftell(fp);
    if ((*logoffset) >= psgMbshm->lLogfilesize)
    {
      ilFileid++;
      fclose(fp);
      memset(amFp_name,0x00,80);
      sprintf(amFp_name,"%s/bmqlog/logfile/bmq%ld.%d",
        getenv("BMQ_PATH"),ilPid,ilFileid);
    }
    else
    {
      break;
    }
  }

  memset(logfile,0x00,11);
  sprintf(logfile,"bmq%ld.%d",ilPid,ilFileid);

  llLen=fwrite(msgpack, 1, loglen, fp);
  if ( llLen < 0) 
  {
    _bmqDebug("S1670 д��־�ļ�����errno:%d:%s",errno,strerror(errno));
    return(-1120);
  }

  fclose(fp);

  if(igDebug >= 2)
    _bmqDebug("S1680 **�������� _bmqPutlog(...)**");

  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqDebugmbinfo
 ** ����:   ��¼������Debug��Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqDebugmbinfo()
{
  int i;

  _bmqDebug("S1690 ***������Ϣ(psgMbinfo[])***");
  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {

    if ((psgMbinfo[i-1].lSendnum)
      ||(psgMbinfo[i-1].lRecvnum)
      ||(psgMbinfo[i-1].lPendnum)
      ||(psgMbinfo[i-1].iConnect))
      _bmqDebug("S1700 %d,%ld,%ld,%d,%d,%d",i,
        psgMbinfo[i-1].iConnect,
        psgMbinfo[i-1].lQueue_head,
        psgMbinfo[i-1].lQueue_tail,
        psgMbinfo[i-1].iConn_head,
        psgMbinfo[i-1].iConn_tail);
  }
  return(0);
}

/**************************************************************
 ** ������: _bmqDebugmbqueue
 ** ����:   ��¼������Debug��Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqDebugmbqueue()
{
  int i;

  _bmqDebug("S1710 ***�ʼ�����(psgMbqueue[])***");
  for(i=1;i<=psgMbshm->lMbqueuecount;i++)
  {
    if (psgMbqueue[i-1].cTag)
      _bmqDebug("S1720 %d,%d,%d,%d,%ld,%d,%s,%d,%d",i,
        psgMbqueue[i-1].cTag,
        psgMbqueue[i-1].lLlink,
        psgMbqueue[i-1].lRlink,
        psgMbqueue[i-1].lSerialnum,
        psgMbqueue[i-1].iPrior,
        psgMbqueue[i-1].aFilter,
        psgMbqueue[i-1].iOrg_mailbox,
        psgMbqueue[i-1].lPack_head);
  }
  return(0);
}

/**************************************************************
 ** ������: _bmqDebugmbpack
 ** ����:   ��¼������Debug��Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqDebugmbpack()
{
  int i;

  _bmqDebug("S1730 ***�����ڴ����ݰ�(psgMbpack[])***");
  for(i=1;i<=psgMbshm->lMbpacksize;i++)
  {
    if (psgMbpack[i-1].iMsglen)
      _bmqDebug("S1740 %d,%d,%d",i,
        psgMbpack[i-1].iMsglen,
        psgMbpack[i-1].lLink);
  }
  return(0);
}

/**************************************************************
 ** ������: _bmqDebugmbconnect
 ** ����:   ��¼������Debug��Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqDebugmbconnect()
{
  int i;

  _bmqDebug("S1750 ***�����������ջ(psgMbconnect[])***");
  for(i=1;i<=psgMbshm->iMbcontcount;i++)
  {
    if (psgMbconnect[i-1].cTag)
      _bmqDebug("S1760 %d,%d,%d,%d,%d,%d,%d,%s,%s",i,
        psgMbconnect[i-1].cTag,
        psgMbconnect[i-1].iLlink,
        psgMbconnect[i-1].iRlink,
        psgMbconnect[i-1].iPrior,
        psgMbconnect[i-1].aFilter,
        psgMbconnect[i-1].aMask );
  }
  return(0);
}

/**************************************************************
 ** ������: _bmqPrintmbinfo
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqPrintmbinfo()
{
  int i;

  printf("***������Ϣ(psgMbinfo[])***\n");
  printf("ID     iConnect lQueue_head lQueue_tail iConn_head iConn_tail\n");
  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {

    if ((psgMbinfo[i-1].lSendnum)
      ||(psgMbinfo[i-1].lRecvnum)
      ||(psgMbinfo[i-1].lPendnum)
      ||(psgMbinfo[i-1].iConnect))
      printf("%-6d %-8d %-11ld %-11ld %-10d %-10d\n",i,
        psgMbinfo[i-1].iConnect,
        psgMbinfo[i-1].lQueue_head,
        psgMbinfo[i-1].lQueue_tail,
        psgMbinfo[i-1].iConn_head,
        psgMbinfo[i-1].iConn_tail);
  }
  return(0);
}

/**************************************************************
 ** ������: _bmqPrintmbqueue
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqPrintmbqueue()
{
  int    ilRc;
  long   i;
  FILE   *fp;
  struct mbqueue slMbqueue;

  printf("***�ʼ�����(psgMbqueue[])***\n");
  printf("ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
  for(i=1;i<=psgMbshm->lMbqueuecount;i++)
  {
    if (psgMbqueue[i-1].cTag)
      printf("%-6ld %-2d %-6ld %-6ld %-7ld %-5d %-5d %-4d %-6ld %-20s\n",i,
        psgMbqueue[i-1].cTag,
        psgMbqueue[i-1].lLlink,
        psgMbqueue[i-1].lRlink,
        psgMbqueue[i-1].lSerialnum,
        psgMbqueue[i-1].iPrior,
        psgMbqueue[i-1].iOrg_grp,
        psgMbqueue[i-1].iOrg_mailbox,
        psgMbqueue[i-1].lPack_head,
        psgMbqueue[i-1].aFilter);
  }
  printf("***�ʼ����л����ļ���¼***\n");
  printf("ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
  if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
  {
    printf("���ļ�����\n");
    exit(-1);
  }
  i = psgMbshm->lMbqueuecount;
  for(;;)
  {
    ilRc = _bmqQueuefile_pick(fp,&slMbqueue,++i);
     if(ilRc == FAIL || ilRc == SHMFILEEND) break;
     if(ilRc == WITHOUTUSE) continue;
     printf("%-6ld %-2d %-6ld %-6ld %-7ld %-5d %-5d %-4d %-6ld %-20s\n",i,
        slMbqueue.cTag,
        slMbqueue.lLlink,
        slMbqueue.lRlink,
        slMbqueue.lSerialnum,
        slMbqueue.iPrior,
        slMbqueue.iOrg_grp,
        slMbqueue.iOrg_mailbox,
        slMbqueue.lPack_head,
        slMbqueue.aFilter);
  }
  _bmqClosefile(fp);
  return(0);
}

/**************************************************************
 ** ������: _bmqPrintmbpack
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqPrintmbpack()
{
  int    ilRc;
  long   i;
  FILE   *fp;
  struct mbfilepack slMbfilepack;

  printf("***�����ڴ����ݰ�(psgMbpack[])***\n");
  printf("ID     iMsglen lLink\n");
  for(i=1;i<=psgMbshm->lMbpacksize;i++)
  {
    if (psgMbpack[i-1].iMsglen)
      printf("%-6ld %7d %5ld\n",i,
        psgMbpack[i-1].iMsglen,
        psgMbpack[i-1].lLink);
  }

  printf("***�ļ��ڴ����ݰ�***\n");
  printf("ID     iMsglen     aLogfile lLogoffset\n");
  if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
  {
    printf("���ļ�����\n");
    exit(-1);
  }
  i = psgMbshm->lMbpacksize;

  for(;;)
  {
    ilRc = _bmqPackfile_pick(fp,&slMbfilepack,++i);
      if(ilRc == FAIL || ilRc == SHMFILEEND) break;
      if(ilRc == WITHOUTUSE) continue;
    printf("%-6ld %7ld %12s %10ld\n",i,
        slMbfilepack.lLoglen,slMbfilepack.aLogfile,slMbfilepack.lLogoffset);
  }
  _bmqClosefile(fp); 
  return(0);
}

/**************************************************************
 ** ������: _bmqPrintmbconnect
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqPrintmbconnect()
{
  int    i,ilRc;
  FILE   *fp;
  struct mbconnect slMbconnect;

  printf("***�����������ջ(psgMbconnect[])***\n");
  printf("ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
  for(i=1;i<=psgMbshm->iMbcontcount;i++)
  {
    if (psgMbconnect[i-1].cTag)
      printf("%-6d %-4d %-6d %-6d %-6d %-20s %-6ld\n",i,
        psgMbconnect[i-1].cTag,
        psgMbconnect[i-1].iLlink,
        psgMbconnect[i-1].iRlink,
        psgMbconnect[i-1].iPrior,
        psgMbconnect[i-1].aFilter,
     /* psgMbconnect[i-1].aMask, */
        psgMbconnect[i-1].lConnpid);
  }

  printf("***�������ӻ����ļ���¼***\n");
  printf("ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
  if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
  {
    printf("���ļ�����\n");
    exit(-1);
  }
  i = psgMbshm->iMbcontcount;
  for(;;)
  {
    ilRc = _bmqConnfile_pick(fp,&slMbconnect,++i);
      if(ilRc == FAIL || ilRc == SHMFILEEND) break;
      if(ilRc == WITHOUTUSE) continue;
    printf("%-6d %-4d %-6d %-6d %-6d %-20s %-6ld\n",i,
        slMbconnect.cTag,
        slMbconnect.iLlink,
        slMbconnect.iRlink,
        slMbconnect.iPrior,
        slMbconnect.aFilter,
     /* slMbconnect.aMask, */
        slMbconnect.lConnpid);
  }
  _bmqClosefile(fp);
  return(0);
}

/**************************************************************
 ** ������: _bmqLock
 ** ����:   �����źŵ���Դ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqLock(int iNum)
{
  struct sembuf slSembuf;
  int    ilRc;

  slSembuf.sem_num = iNum - 1;
  slSembuf.sem_op = -1;
  slSembuf.sem_flg = SEM_UNDO;

  if(igDebug >= 2)
   _bmqDebug("S1770 _bmqLock:%d ..",iNum);


  ilRc = semop(psgMbshm->lSemid,&slSembuf,1);
  if (ilRc == -1)
  {
    _bmqDebug("S1780 �źŵƲ�������semop P() errno:%d:%s",errno,strerror(errno)); 
    exit(-1);
  }

  if(igDebug >= 2)
    _bmqDebug("S1790 _bmqLock:%d ok",iNum);


  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqUnlock
 ** ����:   �ͷ��źŵ���Դ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqUnlock(int iNum)
{
  int ilRc;
  struct sembuf slSembuf;

  slSembuf.sem_num = iNum - 1;
  slSembuf.sem_op = 1;
  slSembuf.sem_flg = SEM_UNDO;
  ilRc = semop(psgMbshm->lSemid,&slSembuf,1);
  if (ilRc == -1)
  {
    _bmqDebug("S1800 semop P() errno:%d:%s",errno,strerror(errno));
    exit(-1);
  }
  if(igDebug >= 2)
    _bmqDebug("S1810 _bmqUnlock:%d ok",iNum);
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqProshut
 ** ����:   �رշ������շ��ػ����̡�����״̬��ؽ��̺Ϳ����ػ����� 
 ** ����:  xujun 
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
void _bmqProshut()
{
  if( igDebug >= 2 )
    _bmqDebug("S1820 ***���ú��� _bmqProshut()�ر��������****");

  /*�رշ������������̡�����״̬��ؽ��̺Ϳ����ػ�����*/
  /*
  if( psgMbshm->lBmqclsrcv_pid > 0 )
    kill(psgMbshm->lBmqclsrcv_pid,SIGTERM);
  if( psgMbshm->lBmqclssnd_pid > 0 )
    kill(psgMbshm->lBmqclssnd_pid,SIGTERM);
  del by xujun 2006.12.28
  ��ʹ��ͨ��������*/
  if( psgMbshm->lBmqclean_pid > 0 )
    kill(psgMbshm->lBmqclean_pid,SIGTERM);
  if( psgMbshm->lBmqcls_pid > 0 )
    kill(psgMbshm->lBmqcls_pid,SIGTERM);
  if( psgMbshm->lBmqgrpsnd_pid > 0 )
    kill(psgMbshm->lBmqgrpsnd_pid,SIGTERM);
  if( psgMbshm->lBmqgrprcv_pid > 0 )
    kill(psgMbshm->lBmqgrprcv_pid,SIGTERM);
  if( psgMbshm->lBmqgrprcvs_pid > 0 )
    kill(psgMbshm->lBmqgrprcvs_pid,SIGTERM);
  if( psgMbshm->lBmqgrprcvw_pid > 0 )
    kill(psgMbshm->lBmqgrprcvw_pid,SIGTERM);
  /* add by xujun 2006.12.28
     ����4���ļ������ػ����� */
  if( psgMbshm->lBmqfilecls_pid > 0 )
    kill(psgMbshm->lBmqfilecls_pid,SIGTERM);
  if( psgMbshm->lBmqfilemng_pid > 0 )
    kill(psgMbshm->lBmqfilemng_pid,SIGTERM);
  if( psgMbshm->lBmqfilesaf_pid > 0 )
    kill(psgMbshm->lBmqfilesaf_pid,SIGTERM);
  if( psgMbshm->lBmqfilelog_pid > 0 )
    kill(psgMbshm->lBmqfilelog_pid,SIGTERM);

  if( igDebug >= 2 )
    _bmqDebug("S1830 ***�������ú��� _bmqProshut()****");
}

/**************************************************************
 ** ������: _bmqQuit
 ** ����:   �ͷ��³�Q�����IPC��Դ
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqQuit()
{
  int   ilRc,ilRt;
 
  if( igDebug >= 2)
    _bmqDebug("S1840 ***���ú���_bmqQuit()�ͷ��³�Q�����IPC��Դ**");

  ilRt  =  0;

  /*ɾ����Ϣ����*/
  ilRc = msgctl(psgMbshm->lMsgid_grp,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1860 ɾ����Ϣ���� msgctl(RM-grp) errno:%d:%s",errno,strerror(errno));
    ilRt = -1161;
  }
  if(psgMbshm->lMsgid_grp_rcv)
  {
    ilRc = msgctl(psgMbshm->lMsgid_grp_rcv,IPC_RMID,0);
    if(ilRc < 0)
    {
      _bmqDebug("S1860 ɾ����Ϣ���� msgctl(RM-grp) errno:%d:%s",errno,strerror(errno));
      ilRt = -1161;
    }
  }
  /*
  ilRc = msgctl(psgMbshm->lMsgid_cls,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1861 ɾ����Ϣ���� msgctl(RM-cls) errno:%d:%s",errno,strerror(errno));
    ilRt = -1161;
  }
  del by xujun 2006.12.28
  ��ʹ��ͨ��������*/

  /*ɾ���źŵ�*/
  ilRc = semctl(psgMbshm->lSemid,0,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1870 ɾ���źŵƳ��� semctl(RM) errno:%d:%s",errno,strerror(errno));
    ilRt = -1171;
  }
  /*ɾ�������ڴ�*/
  ilRc = shmctl(igMb_sys_shm_id,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1880 ɾ�������ڴ���� shmctl(RM) errno:%d:%s",errno,strerror(errno));
    ilRt = -1153;
  }
  if( igDebug >= 2)
    _bmqDebug("S1890 ***�������ú���_bmqQuit()**");
  return(ilRt);
}

/**************************************************************
 ** ������: _bmqTimeout
 ** ����:   ������ȡ��Ϣ�����˳�ʱ��
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static void _bmqTimeout()
{
  int	ilRc;
 
  ilRc = _bmqShmConnect();
  if(ilRc == SUCCESS)
    psgMbconnect[igMb_sys_current_connect - 1].cTag = 1;
  igFlag = 1;
  alarm(0);
  signal(SIGALRM,SIG_DFL);
}

/**************************************************************
 ** ������: _bmqConn_judge
 ** ����:   �жϽ����Ƿ����������
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:lPid--����Pid
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqConn_judge(long lPid)
{
  int    ilConn_head;
  int    ilRc,i;
  FILE   *fp;
  struct mbconnect slMbconnect;
 
  for(i = 0; i < psgMbshm->iMbinfo; i++)
  {
    ilConn_head = psgMbinfo[i].iConn_head;
    while(ilConn_head)
    {
      if(ilConn_head > psgMbshm->iMbcontcount)
      {
        /*�����ӻ����ļ�*/
        if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
        {
          _bmqDebug("S1900 �����ӻ����ļ�����");
          return (FAIL);
        }
        /*ȡ��ָ����¼*/
        _bmqConnfile_pick(fp,&slMbconnect,ilConn_head);
        _bmqClosefile(fp);
  
        if(slMbconnect.lConnpid == lPid) return(FAIL);
        ilConn_head = slMbconnect.iRlink;
      }
      else
      {
        if(psgMbconnect[ilConn_head-1].lConnpid == lPid) return(FAIL);
        ilConn_head = psgMbconnect[ilConn_head-1].iRlink;
      }
    }
  }
 return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqConfig_load
 ** ����:   ����ϵͳ���ò���
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:aCfgType--��������   iGrpid--���
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqConfig_load(char *aCfgType,short iTmpid)
{
  char *alPath,alFile[51],alType[9];
  char alBuf[512],alBuf1[512],alResult[50],alResult1[50],alResult2[50]; 
  char alResult3[50],alResult4[50],alResult5[50],alResult6[50],alResult7[50];
  char alExpress[iFLDVALUELEN];
  FILE *fp;
  short  i = 0;
  static short iNum = 0;

  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S1910 ��ϵͳ��������[BMQ_PATH]ʧ��");
    return(FAIL);
  }  
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,SERVERCFGFILE);
  if ((fp = fopen(alFile, "r")) == NULL) return(FAIL);

  while (fgets(alBuf1, sizeof(alBuf), fp) != NULL)
  {
    _bmqTrim(alBuf1);
    if (strlen(alBuf1) == 0 || alBuf1[0] == '#') continue;
    if (memcmp(alBuf1,"BEGIN",5) != 0 ) continue;

    sscanf(alBuf1,"%s %s", alResult,alType);

    if(strcmp(alType,aCfgType) != 0)  continue;
    /*����ͨ������ͨѶ�˿�
    if(strcmp(alType,"C/S") == 0)
    {
     while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
     {
       _bmqTrim(alBuf);
       if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
       if ( memcmp( alBuf,"END",3) == 0 )
        {
          fclose(fp);
          return(SUCCESS);
        }
       sscanf(alBuf,"%s %s",alResult,alResult1);
       _bmqTrim(alResult1);
       igBmqtcpport = atoi(alResult1);
      }
    }
    del by xujun  2006.12.28 
    ��ʹ��ͨ��������*/
    /*next add by nh 20011128����TCPͨѶ�˿�*/
    if(strcmp(alType,"CLS") == 0)
    {
     while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
     {
       _bmqTrim(alBuf);
       if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
       if ( memcmp( alBuf,"END",3) == 0 )
        {
          fclose(fp);
          return(SUCCESS);
        }
       sscanf(alBuf,"%s %s",alResult,alResult1);
       _bmqTrim(alResult1);
       igBmqclsport = atoi(alResult1);
      }
    }
    if(strcmp(alType,"FILECLS") == 0)
    {
     while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
     {
       _bmqTrim(alBuf);
       if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
       if ( memcmp( alBuf,"END",3) == 0 )
        {
          fclose(fp);
          return(SUCCESS);
        }
                
       sscanf(alBuf,"%s %s",alResult,alResult1);
       _bmqTrim(alResult);
       _bmqTrim(alResult1);
       if (strcmp(alResult,"MBTCPPORT") == 0)
         igBmqclsport = atoi(alResult1);
       if (strcmp(alResult,"BLOCKSIZE") == 0)
         igFileTranBlockSize = atoi(alResult1);
      }
    }
    /*�����������IP��ͨѶ�˿�*/
    if(strcmp(alType,"GROUP") == 0)
    {
      while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
      {
        _bmqTrim(alBuf);
        if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
        if ( memcmp( alBuf,"END",3) == 0 )
        {
          fclose(fp);
          return(FINISH);
        }
        sscanf(alBuf,"%s %s %s %s %s",alResult,alResult1,alResult2,alResult3,alResult4);
        _bmqTrim(alResult);
        if( iTmpid == -1 )
        {
          if ( i != iNum ) 
          {
            i ++;
            continue;
          }
          _bmqTrim(alResult1);
          _bmqTrim(alResult2);
          _bmqTrim(alResult3);
          _bmqTrim(alResult4);
          if ( atoi(alResult) != psgMbshm->iMbgrpid )
          {
            igGroupid        = atoi(alResult);
            igBmqgrpport     = atol(alResult2);
            lgGroup_try_time = atol(alResult3);
            agCommode[0]     = alResult4[0];
            memcpy(agBmqhostip, alResult1, sizeof(alResult1));
            iNum ++ ;
            fclose(fp);
            return(SUCCESS);
          }
          i ++;
          iNum ++;
        }
        else
        {
          if( atoi(alResult) != iTmpid) continue;
          _bmqTrim(alResult1);
          _bmqTrim(alResult2);
          _bmqTrim(alResult3);
          _bmqTrim(alResult4);
          memcpy(agBmqhostip, alResult1, sizeof(alResult1));
          igBmqgrpport     = atol(alResult2);
          lgGroup_try_time = atol(alResult3);
          agCommode[0]     = alResult4[0];
          fclose(fp);
          return(SUCCESS);
        }
      }
    }
    /*next add by nh ����C�˵�IP��ַ�Ͷ˿�
    if(strcmp(alType,"CLINF") == 0)
    {
      while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
      {
        _bmqTrim(alBuf);
        if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
        if ( memcmp( alBuf,"END",3) == 0 )
        {
          fclose(fp);
          return(FINISH);
        }
        sscanf(alBuf,"%s %s %s %s",alResult,alResult1,alResult2,alResult3);
        _bmqTrim(alResult);
        ����Ϊ��1ʱ�ǳ�ʼ����������
        if( iTmpid == -1 )
        {
          if ( i != iNum ) 
          {
            i ++;
            continue;
          }
          _bmqTrim(alResult1);
          _bmqTrim(alResult2);
          igMBid = atoi(alResult);
          memcpy(agBmqclip, alResult1, sizeof(alResult1));
          igBmqclport = atol(alResult2);
          lgTry_time = atol(alResult3);
          psgMbinfo[igMBid - 1].iFlag  = 0;
          iNum ++ ;
          fclose(fp);
          return(SUCCESS);
        }	
        else
        {
          if( atoi(alResult) != iTmpid) continue;
          _bmqTrim(alResult1);
          _bmqTrim(alResult2);
          memcpy(agBmqclip, alResult1, sizeof(alResult1));
          igBmqclport = atol(alResult2);
        }
      }
    }
    ��ʹ��ͨ��������*/
        
    /*����BMQ����*/
    if(strcmp(alType,"BMQ") == 0)
    {
     while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
     {
      _bmqTrim(alBuf);
      if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
      if ( memcmp( alBuf,"END",3) == 0 )  
        { 
          fclose(fp);
          return(SUCCESS);
        }

      sscanf(alBuf,"%s %s",alResult,alResult1);
      _bmqTrim(alResult);
      _bmqTrim(alResult1);

      if (strcmp(alResult,"MBINFO") == 0)
         igMbinfo = atoi(alResult1);
      if (strcmp(alResult,"MBQUEUECOUNT") == 0)
         lgMbqueuecount = atol(alResult1);
      if (strcmp(alResult,"MBPACKSIZE") == 0)
         lgMbpacksize = atol(alResult1);
      if (strcmp(alResult,"MBCONTCOUNT") == 0)
         igMbcontcount = atoi(alResult1);
      if (strcmp(alResult,"MBFILECOUNT") == 0)
         igMbfilecount = atoi(alResult1);
      if (strcmp(alResult,"LOGFILESIZE") == 0)
         lgLogfilesize = atol(alResult1);
     }
    }
    /*���빲���ڴ�ID*/
    if(strcmp(alType,"SHMK") == 0)
    {
     while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
     {
      _bmqTrim(alBuf);
      if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
      if ( memcmp( alBuf,"END",3) == 0 ) 
        {
          fclose(fp);    
          return(SUCCESS);
         }
      sscanf(alBuf,"%s %s",alResult,alResult1);
      _bmqTrim(alResult);
      _bmqTrim(alResult1);
      if (strcmp(alResult,"SHMKEY_MB") == 0)
       {
         lgSHMKEY_MB = atol(alResult1);
        }
      }
     }
    /*��������פ��ʱ��ͷ�ֵ*/
    if(strcmp(alType,"MAIL") == 0)
    {
      while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
      {
        _bmqTrim(alBuf);
        if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
        if ( memcmp( alBuf,"END",3) == 0 ) 
        {
          fclose(fp);    
          return(SUCCESS);
        }
        sscanf(alBuf,"%s %s %s %s %s %s %s %s %s",
        alResult,alResult1,alResult2,alResult3,alResult4,alResult5,alResult6,alResult7,alExpress);
        _bmqTrim(alResult);
        _bmqTrim(alResult1);
        _bmqTrim(alResult2);
        _bmqTrim(alResult3);
        _bmqTrim(alResult4);
        _bmqTrim(alResult5);
        _bmqTrim(alResult6);
        _bmqTrim(alResult7);
        _bmqTrim(alExpress);
        psgMbinfo[atoi(alResult) - 1].lExisttime  = atol(alResult1);
        psgMbinfo[atoi(alResult) - 1].lMaxqueue   = atol(alResult2);
        psgMbinfo[atoi(alResult) - 1].iHload      = atol(alResult3);
        psgMbinfo[atoi(alResult) - 1].iLload      = atol(alResult4);
        psgMbinfo[atoi(alResult) - 1].iTimes      = atol(alResult5);
        psgMbinfo[atoi(alResult) - 1].iSendScanFlag = atol(alResult6);
        psgMbinfo[atoi(alResult) - 1].iRecvScanFlag = atol(alResult7);
        strcpy(psgMbinfo[atoi(alResult) - 1].aExpress,alExpress);        
      }
    }
    /*����ϵͳ��ѯʱ��*/
    if(strcmp(alType,"WTIME") == 0)
    {
      while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
      {
       _bmqTrim(alBuf);
       if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
       if ( memcmp( alBuf,"END",3) == 0 )
         {
           fclose(fp);
           return(SUCCESS);
         }
       sscanf(alBuf,"%s %s",alResult,alResult1);
       _bmqTrim(alResult1);
       lgWAITTIME = atol(alResult1);
      }
     }
    /*����DEBUG��־*/
    if(strcmp(alType,"DEBUG") == 0)
    {
      while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
      {
       _bmqTrim(alBuf);
       if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
       if ( memcmp( alBuf,"END",3) == 0 )
         {
           fclose(fp);
           return(SUCCESS);
         }
       sscanf(alBuf,"%s %s",alResult,alResult1);
       _bmqTrim(alResult1);
       igDebug = atoi(alResult1);
      }
     }
  }
  fclose(fp);
  return(FAIL);
} 

/**************************************************************
 ** ������: _bmqOpenfile
 ** ����:   �򿪻����ļ�
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqOpenfile(int iFileid,FILE **fp)
{
 char   alFile[101];
 char   *alPath;
 char   alBmqfile[3][101]={"/bmqlog/logfile/queue.rec","/bmqlog/logfile/pack.rec","/bmqlog/logfile/connect.rec"};

 if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S1920 ��ϵͳ��������[BMQ_PATH]ʧ��");
    return -1;
  }
 
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,alBmqfile[iFileid]);

  if ((*fp = fopen(alFile, "r+")) == NULL)
  {
    if ((*fp = fopen(alFile, "w+")) == NULL)
    {
      _bmqDebug("S1921 �ļ���ʧ��:%d,%s",errno,strerror(errno));
      return(FAIL);
    }
    else
      return(SUCCESS);
  }
  else
    return(SUCCESS);    
}

/**************************************************************
 ** ������: _bmqClosefile
 ** ����:   �رջ����ļ�
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqClosefile(FILE *fp)
{
  fclose(fp);
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqUnlinkfile
 ** ����:   ɾ�������ļ�
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static int _bmqUnlinkfile()
{
  int    i;
  char   *alPath,alFile[80];
  char   alBmqfile[4][101]={"/bmqlog/logfile/queue.rec","/bmqlog/logfile/pack.rec","/bmqlog/logfile/connect.rec","/bmqlog/debug/bmqstatfile.log"};
 
  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S1930 ��ϵͳ��������[BMQ_PATH]ʧ��");
    return -1;
  }

  for(i=0;i<4;i++)
  {
    memset(alFile,0x00,sizeof(alFile));
    strcpy(alFile,alPath);
    strcat(alFile,alBmqfile[i]);

    unlink(alFile);
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqGetfile
 ** ����:   �ӻ����ļ�ȡ��¼
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*int _bmqGetfile(FILE *fp, long lOffset, short iLen, char *aBuf)delete by wh*/
int _bmqGetfile(FILE *fp, long lOffset, TYPE_SWITCH iLen, char *aBuf)/*add by wh*/
{
  TYPE_SWITCH ilRc;/*add by wh*/

  if( igDebug >= 2)
    _bmqDebug("S1940 **���ú��� _bmqGetfile(fp,Offset,Len,aBuf)�ӻ����ļ�ȡ��¼**");
  if (lOffset < -1)
  {
    _bmqDebug("S1950 ƫ��������");
    return (FAIL);
  }
  fflush(fp);
  if (lOffset != OFFSETNULL)
  {
    ilRc = fseek(fp, lOffset, SEEK_SET);
    if (ilRc)
    {
      _bmqDebug("S1960 �ļ���λ����");
      return (FAIL);
    }
  }
  ilRc = fread(aBuf, sizeof(char), iLen, fp);
  if (feof(fp))
  {
    if(igDebug >= 2)
      _bmqDebug("S1970 �����ļ�βgetfile()");
    return (SHMFILEEND);
   }
  if (ilRc != iLen)
  {
    _bmqDebug("S1980 ���ļ�����getfile()");
    return (FAIL);
  }
  if( igDebug >= 2)
    _bmqDebug("S1990 **�������ú��� _bmqGetfile(...)**");

  return (SUCCESS);
}

/**************************************************************
 ** ������: _bmqPutfile
 ** ����:   �ż�¼�������ļ�
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*int _bmqPutfile(FILE *fp, long lOffset, short iLen, char *aBuf)delete by wh*/
int _bmqPutfile(FILE *fp, long lOffset, TYPE_SWITCH iLen, char *aBuf)/*add by wh*/
{
  /*short ilRc;delete by wh*/
  TYPE_SWITCH ilRc;/*add by wh*/

  if ( igDebug >= 2)
    _bmqDebug("S2000 **���ú��� _bmqPutfile(fp,Offset,Len,aBuf)**");

  if (lOffset < -1)
  {
    _bmqDebug("S2010 ƫ��������");
    return (FAIL);
  }
  fflush(fp);
  if (lOffset != OFFSETNULL)
  {
    ilRc = fseek(fp, lOffset, SEEK_SET);
    if (ilRc)
    {
      _bmqDebug("S2020 �ļ���λ����");
      _bmqDebug("S2021 �ļ���λ����errno:%d:%s",errno,strerror(errno));
      return (FAIL);
    }
  }
  ilRc = fwrite(aBuf, sizeof(char), iLen, fp);
  if (ilRc != iLen)
  {
    _bmqDebug("S2030 д�ļ�����");
    _bmqDebug("S1660 д�ļ�����errno:%d:%s",errno,strerror(errno));
    return (FAIL);
  }
  if( igDebug >= 2)
    _bmqDebug("S2040 **�������ú��� _bmqPutfile(..)**");

  return (SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueuefile_put
 ** ����:   ���һ�����м�¼�����ļ�
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iGrpid--���  iMbid--Ŀ������   iPrior--���ȼ�
             aFilter--���Ĺ�������  lPack_head--��������ͷ
             plQueue_id--��Ϣ����ID
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
static  int _bmqQueuefile_put(short iGrpid,short iMbid,short iPrior,char *aFilter,long lPack_head,long *plQueue_id)
{
  short  ilRc;
  /*short  ilSize;delete by wh*/
  TYPE_SWITCH ilSize;/*add by wh*/
  int    i;
  long   llOffset = 0;
  char   alBuf[iFLDVALUELEN];
  static FILE   *fp;
  struct mbqueue slMbqueue;

  if(igDebug >=2)
  {
    _bmqDebug("S2050 **����_bmqQueuefile_put()**");
    _bmqDebug("S2060 iGrpid=%d iMbid=%d iPrior=%d lPack_head=%ld ",iGrpid,iMbid,iPrior,lPack_head);
    _bmqDebughex(aFilter,iMASKLEN);
  }

  if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
  {
    _bmqDebug("S2070 ���ļ�����");
    return (FAIL);
  }

  ilSize = sizeof(struct mbqueue) + 1;
  i = 0;
  for(;;)
  { 
    i++;
    ilRc = _bmqGetfile(fp, OFFSETNULL, ilSize, alBuf);
    if (ilRc < 0)
    {
      _bmqDebug("S2080 ���ļ�����insertqueuelog()");
      _bmqClosefile(fp);
      return (FAIL);
    }
    llOffset = llOffset + ilSize;
    if (ilRc == SUCCESS)
        if (alBuf[0] == '0') break;
    if (ilRc == SHMFILEEND)  break;
  }
 
  *plQueue_id = i + psgMbshm->lMbqueuecount;

  /*����һ�����м�¼ */
  memset(&slMbqueue,0x00,sizeof(struct mbqueue));
  ilRc = _bmqQueue_add(i + psgMbshm->lMbqueuecount,iGrpid,iMbid,iPrior,
    aFilter,lPack_head,&slMbqueue);
  if(ilRc == FAIL)
  {
    _bmqDebug("S2090 ����һ�����м�¼���ļ�ERROR!");
    _bmqClosefile(fp);
    return(FAIL);
  }

  /*��¼д���ļ�*/
  alBuf[0] = '1';
  memcpy(alBuf + 1, (char *)&slMbqueue, ilSize - 1);
  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (i-1)*ilSize, ilSize, alBuf);
  if (ilRc)
  {
    _bmqDebug("S2100 д�ļ�����");
    _bmqClosefile(fp);
    return (FAIL);
  }

  _bmqClosefile(fp);

  if( igDebug >= 2)
    _bmqDebug("S2110 **��������_bmqQueuefile_put()**");

  return(SUCCESS);  

}

/**************************************************************
 ** ������: _bmqQueuefile_set
 ** ����:   ���л����ļ���¼����
 ** ����:  xujun
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fp--�ļ�ָ��  iTag--���ñ�־  lLlink--����
             lRlink--����  lOffsetid--��¼ƫ����
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqQueuefile_set(FILE *fp,short iTag, long lLlink,long lRlink,long lOffsetid)
{
  int    ilRc,ilSize;
  char   alBuf[iFLDVALUELEN];
  struct mbqueue slMbqueue;
  

  if (igDebug >= 2)
    _bmqDebug("S2120 **���ú���_bmqQueuefile_set(fp,Tag,.....)**");

   ilSize = sizeof(struct mbqueue) + 1;

   fseek(fp,0,SEEK_SET);
   ilRc = _bmqGetfile(fp, (lOffsetid-psgMbshm->lMbqueuecount-1)*ilSize, ilSize,alBuf);
   if (ilRc < 0)
    {
      _bmqDebug("S2130 ���ļ�����queuefile_put()");
      return (FAIL);
    }

   memset(&slMbqueue,0x00,sizeof(struct mbqueue));
   memcpy(&slMbqueue,alBuf + 1,ilSize - 1);

   if(iTag >= 0)
     alBuf[0] = '0';
   if(lLlink >= 0)
    slMbqueue.lLlink = lLlink;
   if(lRlink >= 0)
    slMbqueue.lRlink = lRlink;

   memcpy(alBuf + 1, (char *)&slMbqueue, ilSize - 1);

   fseek(fp,0,SEEK_SET);
   ilRc = _bmqPutfile(fp, (lOffsetid-psgMbshm->lMbqueuecount-1)*ilSize, ilSize,alBuf);
   if (ilRc)
    {
     _bmqDebug("S2140 д�ļ�����");
     return (FAIL);
    }

  if (igDebug >= 2)
    _bmqDebug("S2150 **�������ú���_bmqQueuefile_set(.....)**");
 return(SUCCESS);
}
  
/**************************************************************
 ** ������: _bmqConnfile_put
 ** ����:   �����ӻ����ļ�������һ����¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iMbid--�����
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
static int _bmqConnfile_put(short iMbid,long lMsgid)
{
  int   ilRc,ilSize;
  short i;
  char  alBuf[iFLDVALUELEN];
  FILE  *fp;
  struct mbconnect slMbconnect;

  if( igDebug >= 2)
    _bmqDebug("S2160 **���ú���_bmqConnfile_put(%d)**",iMbid);

  if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
  {
    _bmqDebug("S2170 �����ӻ����ļ�����!");
    return (FAIL);
  }

  ilSize = sizeof(struct mbconnect) + 1;
  i = 0;
  for(;;)
  { 
    i++;
    ilRc = _bmqGetfile(fp, OFFSETNULL, ilSize, alBuf);
    if (ilRc < 0)
    {
      _bmqDebug("S2180 ���ļ����� _bmqConnfile_put()");
      _bmqClosefile(fp);
      return (FAIL);
    }
    if (ilRc == SUCCESS)
        if (alBuf[0] == '0') break;
    if (ilRc == SHMFILEEND)  break;
  }

  igMb_sys_current_connect = i + psgMbshm->iMbcontcount;

  alBuf[0] = '1';

  memset(&slMbconnect,0x00,sizeof(struct mbconnect));

  /*�������Ӽ�¼����*/
  ilRc = _bmqConnect_update(iMbid,&slMbconnect);
  if(ilRc == FAIL)
  {
    _bmqDebug("S2190 ���������ļ���¼���ݳ���!");
    _bmqClosefile(fp);
    return(FAIL);
  }
  slMbconnect.lMsgid_recv = lMsgid;

  memcpy(alBuf + 1, (char *)&slMbconnect, ilSize - 1);

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (i-1)*ilSize, ilSize, alBuf);
  if (ilRc)
  {
     _bmqDebug("S2200 _bmqConnfile_put() д�ļ�����");
     _bmqClosefile(fp);
     return (FAIL);
  }

  _bmqClosefile(fp);

  if( igDebug >= 2)
    _bmqDebug("S2210 **�������ú���_bmqConnfile_put(%d)**",iMbid);

  return(SUCCESS);  
}

/**************************************************************
 ** ������: _bmqConnfile_set
 ** ����:   ���ӻ����ļ���¼����
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:cTag--���ñ�־  iLlink--����  iRlink--����
             aFilter--���Ĺ�������  aMask--��������
             iOffsetid--��¼ƫ����
             iFilterFlag            
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
static int _bmqConnfile_set(short cTag,short iLlink,short iRlink,char *aFilter,char *aMask,short iOffsetid,short iFilterFlag)
{
  int    ilRc,ilSize;
  char   alBuf[iFLDVALUELEN];
  struct mbconnect slMbconnect;
  FILE   *fp;

  if( igDebug >= 2)
    _bmqDebug("S2220 **���ú��� _bmqConnfile_set(..)���ӻ����ļ���¼����**");

  if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
  {
    _bmqDebug("S2230 ���ļ�����");
    return (FAIL);
  }

  ilSize = sizeof(struct mbconnect) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (iOffsetid-psgMbshm->iMbcontcount-1)*ilSize, ilSize,alBuf);
  if (ilRc < 0)
  {
    _bmqDebug("S2240 ���ļ����� _bmqConnfile_set()");
    return (FAIL);
  }

  memset(&slMbconnect,0x00,sizeof(struct mbconnect));
  memcpy(&slMbconnect,alBuf + 1,ilSize - 1);

  if(cTag >= 0)
    slMbconnect.cTag = cTag;
  if(iLlink >= 0)
    slMbconnect.iLlink = iLlink;
  if(iRlink >= 0)
    slMbconnect.iRlink = iRlink;
  if(iFilterFlag>=0)
  {
    memcpy(slMbconnect.aFilter,aFilter,iMASKLEN);
    memcpy(slMbconnect.aMask,aMask,iMASKLEN);
  }
  memcpy(alBuf + 1, (char *)&slMbconnect, ilSize - 1);

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (iOffsetid-psgMbshm->iMbcontcount-1)*ilSize, ilSize,alBuf);
  if (ilRc)
  {
    _bmqDebug("S2250 _bmqConnfile_set() д�ļ�����");
    return (FAIL);
  }
  _bmqClosefile(fp);

  if( igDebug >= 2)
    _bmqDebug("S2260 **�������ú���_bmqConnfile_set(..)**");

  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqQueuefile_pick
 ** ����:   �Ӷ��л����ļ���ȡһ��ָ����¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fp--�ļ�ָ��  slMbqueue--�ṹָ��  llOffset--��¼ƫ����
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
int _bmqQueuefile_pick(FILE *fp,struct mbqueue *slMbqueue,long llOffset)
{
  int ilRc,ilSize;
  char alBuf[iFLDVALUELEN];

  if( igDebug >= 2)
    _bmqDebug("S2270 **���ú���_bmqQueuefile_pick(fp,Mbqueue,%ld)**",llOffset);
 
   ilSize = sizeof(struct mbqueue) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (llOffset-psgMbshm->lMbqueuecount-1)*ilSize, 
    ilSize,alBuf);
  if (ilRc == FAIL)
  {
     _bmqDebug("S2280 ���ļ����� _bmqQueuefile_pick()");
     return (FAIL);
  }
  if( igDebug >= 2 )
    _bmqDebug("S2290 **�������ú��� _bmqQueuefile_pick(...)***");

  if(ilRc) return(ilRc);
  if(alBuf[0] == '0') return(WITHOUTUSE);
  memset(slMbqueue,0x00,sizeof(struct mbqueue));
  memcpy(slMbqueue,alBuf + 1,ilSize - 1);
  
  return(SUCCESS);
}
  
/**************************************************************
 ** ������: _bmqConnfile_pick
 ** ����:   �����ӻ����ļ���ȡһ��ָ����¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fp--�ļ�ָ��  slMbconnect--�ṹָ��  llOffset--��¼ƫ����
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
int _bmqConnfile_pick(FILE *fp,struct mbconnect *slMbconnect,long llOffset)
{
  int ilRc,ilSize;
  char alBuf[iFLDVALUELEN];

  if( igDebug >= 2)
    _bmqDebug("S2300 **���ú��� _bmqConnfile_pick(*fp,Mbconnect,%ld)",llOffset);
  ilSize = sizeof(struct mbconnect) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (llOffset-psgMbshm->iMbcontcount-1)*ilSize, 
    ilSize,alBuf);
  if (ilRc == FAIL)
  {
     _bmqDebug("S2310 ���ļ����� _bmqConnfile_pick()");
     return (FAIL);
  }
  if(ilRc) return(ilRc);
  if(alBuf[0] == '0') return(WITHOUTUSE);
  memset(slMbconnect,0x00,sizeof(struct mbconnect));
  memcpy(slMbconnect,alBuf + 1,ilSize - 1);
  if( igDebug >= 2)
    _bmqDebug("S2320 **�������ú��� _bmqConnfile_pick(...)**");
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqPackfile_put
 ** ����:   ���Ļ����ļ�����һ����¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:paMsgpack--����buf  iLoglen--���ĳ���  lFile_head--�ļ���¼ͷ
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
/*static int _bmqPackfile_put(char *paMsgpack,short iLoglen,long *lFile_head)delete by wh*/
static int _bmqPackfile_put(char *paMsgpack,TYPE_SWITCH iLoglen,long *lFile_head)/*add by wh*/
{
  short  ilRc;
  /*short  ilSize;delete by wh*/
  TYPE_SWITCH ilSize;/*add by wh*/
  long   llOffset = 0,i;
  char   alBuf[iFLDVALUELEN];
  FILE   *fp;
  struct mbfilepack slMbfilepack;

  if( igDebug >= 2)
    _bmqDebug("S2330 **���ú��� _bmqPackfile_put(Msgpack,%d,File_head)**",iLoglen);

  if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
  {
    _bmqDebug("S2340 ���ļ�����");
    return (FAIL);
  }

  ilSize = sizeof(struct mbfilepack) + 1; 
  i = 0;
  for(;;)
  {
    i++;
    ilRc = _bmqGetfile(fp, OFFSETNULL, ilSize, alBuf);
    if (ilRc < 0)
    {
      _bmqDebug("S2350 ���ļ����� _bmqFile_put()");
      _bmqClosefile(fp);
      return (FAIL);
    }
    llOffset = llOffset + ilSize;
    if (ilRc == SUCCESS)
      if (alBuf[0] == '0') break;
    if (ilRc == SHMFILEEND)  break;
  }
  alBuf[0] = '1';
  *lFile_head = i;

  memset(&slMbfilepack,0x00,sizeof(struct mbfilepack));
  slMbfilepack.lLoglen  =  iLoglen;
  
  ilRc = _bmqPutlog(paMsgpack,iLoglen,&slMbfilepack.lLogoffset,slMbfilepack.aLogfile);
   if (ilRc)
   {
     _bmqDebug("S2360 д�ļ� _bmqPutlog()���س���");
     _bmqClosefile(fp);
     return (FAIL);
   }

  memcpy(alBuf + 1, (char *)&slMbfilepack, ilSize - 1);

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (i-1)*ilSize, ilSize, alBuf);
  if (ilRc)
   {
     _bmqDebug("S2370 _bmqPackfile_put() д�ļ�����");
     _bmqClosefile(fp);
     return (FAIL);
   }

  _bmqClosefile(fp);
  if(igDebug >= 2)
  {
     _bmqDebug("S2380 ���汨�ĵ��ļ�:�ʼ�ͷ=%d,logfile=%s,logoffset=%ld,iLoglen=%d ",*lFile_head,slMbfilepack.aLogfile,slMbfilepack.lLogoffset,slMbfilepack.lLoglen);
  }
  if( igDebug >= 2)
    _bmqDebug("S2390 **�������ú��� _bmqPackfile_put(...)**");
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqPackfile_pick
 ** ����:   �ӱ��Ļ����ļ���ȡһ��ָ����¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fp--�ļ�ָ��  slMbfilepack--�ṹָ��  
             lPack_head--���ļ�¼����
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
int _bmqPackfile_pick(FILE *fp,struct mbfilepack *slMbfilepack,long lPack_head)
{
  int    ilRc,ilSize;
  char   alBuf[iFLDVALUELEN];

  if(igDebug >= 2)
    _bmqDebug("S2400 **���ú��� _bmqPackfile_pick(fp,Mbfilepack,%ld)**",lPack_head);

   ilSize = sizeof(struct mbfilepack) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (lPack_head-psgMbshm->lMbpacksize-1)*ilSize, ilSize,alBuf);
  if (ilRc == FAIL)
  {
    _bmqDebug("S2410 ���ļ����� _bmqPackfile_pick");
    return (FAIL);
  }

  if(ilRc) return(ilRc);
  if(alBuf[0] == '0') return(WITHOUTUSE); 
  memset(slMbfilepack,0x00,sizeof(struct mbfilepack));
  memcpy(slMbfilepack,alBuf + 1,ilSize - 1);
   if(igDebug >= 2)
     _bmqDebug("S2420 **�������ú��� _bmqPackfile_pick(..)**");
  return(SUCCESS);
}


/**************************************************************
 ** ������: _bmqPackfile_set
 ** ����:   ���Ļ����ļ���¼����
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fp--�ļ�ָ�� iTag--���ñ�־ iLoglen--���ĳ���
             lLogoffset--��¼ƫ����
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
/*int _bmqPackfile_set(FILE *fp,short iTag,short iLoglen,long lLogoffset)delete by wh*/
int _bmqPackfile_set(FILE *fp,short iTag,TYPE_SWITCH iLoglen,long lLogoffset)/*add by wh*/
{
  int ilRc,ilSize;
  char alBuf[iFLDVALUELEN];
  struct mbfilepack slMbfilepack;

  if(igDebug >= 2)
    _bmqDebug("S2430 **���ú��� _bmqPackfile_set(*fp,%d,%d,%ld)**",
      iTag,iLoglen,lLogoffset);
 ilSize = sizeof(struct mbfilepack) + 1;

 fseek(fp,0,SEEK_SET);
 ilRc = _bmqGetfile(fp, (lLogoffset-psgMbshm->lMbpacksize-1)*ilSize, ilSize,alBuf);
 if (ilRc < 0)
  {
    _bmqDebug("S2440 ���ļ����� _bmqPackfile_set()");
    return (FAIL);
  }

  memset(&slMbfilepack,0x00,sizeof(struct mbfilepack));
  memcpy(&slMbfilepack,alBuf + 1,ilSize - 1);

  if(iTag == 0)
    alBuf[0] =  '0';
  slMbfilepack.lLoglen = iLoglen;
  
  memcpy(alBuf + 1, (char *)&slMbfilepack, ilSize - 1);

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (lLogoffset-psgMbshm->lMbpacksize-1)*ilSize, ilSize,alBuf);
  if (ilRc)
   {
    _bmqDebug("S2450 _bmqPutfile()���س���!");
    return (FAIL);
   }

  if(igDebug >= 2)
    _bmqDebug("S2460 **�������ú��� _bmqPackfile_set(...)**");
 return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqShmConnect
 ** ����:   ���ӹ����ڴ�
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������    :
 ** ����ֵ: SUCCESS,������
***************************************************************/
int _bmqShmConnect()
{
  int ilRc;

  if( igDebug >= 2)
    _bmqDebug("S2470 ***���ú���  _bmqShmConnect()���ӹ����ڴ�***");
  /*������������ļ�SHMKEY*/
  ilRc = _bmqConfig_load("SHMK",0);
  if (ilRc < 0)
  {
    _bmqDebug("S2480 configload(SHMK) error");
    return(-1185);
  }

  /*����һ���Ѿ����ڵĹ����ڴ���*/
  ilRc = shmget(lgSHMKEY_MB,0,IPC_EXCL);
  if (ilRc < 0)
  {
    _bmqDebug("S2490 shmget()����errno:%d:%s",errno,strerror(errno));
    _bmqDebug("S2491 %s :����SHMK����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1150);
  }
  igMb_sys_shm_id = ilRc;

  if (shmdt((void *)agShmtmp_d) &&
    igMbopenflag == 1 && lgCurrent_pid == getpid()) 
  {
    _bmqDebug("S5010: [����/ϵͳ����] shmdt()����,errno=%d:%s",errno,strerror(errno));
    return(-1152);
  }

  agShmtmp_d = shmat(igMb_sys_shm_id,NULL,0);
  if (agShmtmp_d == (char *)-1)
  {
    _bmqDebug("S2500 shmat(..)���� errno:%d:%s",errno,strerror(errno)); 
    return(-1151);
  }

  /*�ṹָ�붨λ*/
  psgMbshm = (struct mbshm *)agShmtmp_d;
  psgMbinfo    = (struct mbinfo *)(agShmtmp_d + sizeof(struct mbshm));
  psgMbqueue   = (struct mbqueue *)(agShmtmp_d + sizeof(struct mbshm)
    + psgMbshm->lMbqueueoffset);
  psgMbpack    = (struct mbpack *)(agShmtmp_d + sizeof(struct mbshm)
    + psgMbshm->lMbpackoffset);
  psgMbconnect = (struct mbconnect *)(agShmtmp_d + sizeof(struct mbshm)
    + psgMbshm->lMbconnectoffset);
  psgMbfilemng = (struct monmsg *)(agShmtmp_d + sizeof(struct mbshm)
    + psgMbshm->lFileoffset);
  psgMbrouter  = (struct mbrouter *)(agShmtmp_d + sizeof(struct mbshm)
    + psgMbshm->lMbrouteroffset);

  if( igDebug >= 2)
    _bmqDebug("S2510 ***�������ú���  _bmqShmConnect()****");
  return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqShmDt
 ** ����:   �Ͽ������ڴ�
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������    :
 ** ����ֵ: SUCCESS,������
***************************************************************/
int _bmqShmDt()
{
  int  ilRc;

  /*�Ͽ������ڴ�*/
  ilRc = shmdt ( (void *)agShmtmp_d);
  if ( ilRc )
  {
    _bmqDebug("S2518 �Ͽ������ڴ����  errno:%d:%s",errno,strerror(errno));
    return(-1152);
  }
  psgMbshm = (struct mbshm *)NULL;
  return(SUCCESS);
}

/**************************************************************
 ** ������:  _bmqDaemon_creat
 ** ��  ��:  �����ػ�����
 ** ��  ��:  ���
 ** ��������:2001/08/08 
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  SUCCESS , FAIL
***************************************************************/
int  _bmqDaemon_creat()
{
  pid_t pid, ppid;

  if(igDebug >= 2 )
    _bmqDebug("S2520 ***���ú��� _bmqDaemon_creat()�����ػ�����***");

  /* ��Ϊ��̨���̣��Ҳ����׽��̣�����setsid����ģ�*/
  ppid = getpid();
  pid = fork();
  if (pid < 0) return (FAIL);
  if (pid > 0) exit(0);  
  while(kill(ppid, 0) == 0)  
    if (igDebug >= 3)
      _bmqDebug("S2530 (Daemon(),fork1):�������Ѿ��˳�");  

  /* �����µĽ����飬����Ϊ�׽��̣�ʹ֮����ԭ�����ն� */    
  ppid = getpid();
  setsid(); 
  signal(SIGHUP,SIG_IGN);
  
  /* �����׽��̣���ֹ�����ն���ϵ */
  pid = fork();
  if (pid < 0) return (FAIL);
  if (pid > 0) exit(0);  
  while(kill(ppid, 0) == 0)  
    if (igDebug >= 3)
      _bmqDebug("S2540 (Daemon(),fork2):�������Ѿ��˳�");  
    
  /* �ж���ԭ���ļ�ϵͳ����ϵ */    
  chdir("/");
  
  /* ����ļ�������Ϊ��������������������䴴���ļ���Ӱ�� */
  umask(0);  
  
  /* ��ֹ�ӽ��̼̳����ļ�������Щʼ�ձ��ִ򿪴Ӷ�����ĳЩ��ͻ */

  if(igDebug >= 2 )
    _bmqDebug("S2550 ***�������ú��� _bmqDaemon_creat()***");

  return (SUCCESS);
}

/**************************************************************
 ** ������:  _bmqGetmsgid
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2001/08/08 
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
int _bmqGetmsgid(short iConnectid)
{
  int    ilRc;
  FILE   *fp;
  struct mbconnect slMbconnect;

  if(iConnectid > psgMbshm->iMbcontcount)
  {
    /*ȡ����¼*/
    if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
    {
      _bmqDebug("S0260 �����ӻ����ļ�����");
      return (FAIL);
    }
    _bmqConnfile_pick(fp,&slMbconnect,iConnectid);

    _bmqClosefile(fp);
    return(slMbconnect.lMsgid_recv);
  }
  return(psgMbconnect[iConnectid-1].lMsgid_recv);
}
/**************************************************************
 ** ������:  _bmqGetfrommsg
 ** ��  ��:  
 ** ��  ��:  nh
 ** ��������:2001/11/29 
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
int _bmqGetfrommsg(short iMbid,short iPrior,long lMsgid)
{
  int		ilRc;
  char		alFilter[iMASKLEN];
  struct	recvmsg	slRecvmsg;

  if(igDebug >= 2)
  {
    _bmqDebug("S8800 ****���ú���bmqGetfrommsg(%d,%d,%ld)****",
			iMbid,iPrior,lMsgid);
  }

  for(;;)
  {
    ilRc = msgrcv(lMsgid,&slRecvmsg,sizeof(struct recvmsg),
      0,IPC_NOWAIT);
    if (ilRc < 0) 
    {
      if (errno == ENOMSG)
      {
        if(igDebug >= 1)
          _bmqDebug("S8801 ����Ϣ������û�ж�������");
      }
      break;	
    }
    else
    {
      memcpy(alFilter,psgMbqueue[slRecvmsg.lQueueid-1].aFilter,iMASKLEN);  
      ilRc = _bmqConn_select(iMbid,iPrior,alFilter,slRecvmsg.lQueueid);
      if(ilRc)
      {
        _bmqDebug("S8802 ****���ú���_bmqConn_select()����");
        return(FAIL);
      }
    }
  }
  return(SUCCESS);
}
/**************************************************************
 ** ������:  _bmqSendstat
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2002/07/28 
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
static int _bmqSendstat(short iMbid)
{  
  struct	mbstat	slMbstat;
  char		fname[100];
  FILE		*fp;
  
  sprintf(fname,"%s/bmqlog/debug/bmqstatfile.log",getenv("BMQ_PATH"));
  if (( fp = fopen(fname,"a+")) == NULL)  return(-1023);
  
  memset(&slMbstat,0x00,sizeof(struct mbstat));
  slMbstat.iOrgmb	= igMb_sys_current_mailbox;
  slMbstat.iDesmb	= iMbid;
  slMbstat.lSendpid	= getpid();
  time(&(slMbstat.lSendtime));
  
  fwrite((char *)(&slMbstat),sizeof(char),sizeof(struct mbstat),fp);

  fclose(fp);
  return(SUCCESS);
  	
}

/* add by xujun 2002.12.18 for file trans begin */
/**************************************************************
 ** ������:  _bmqPutindexfile
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2002/12/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
int _bmqPutindexfile(long lSerialno,char *aFilename)
{
  int		ilRc;
  char		alFile[80];
  FILE		*fp;
  char   	*alPath;
  struct	indexfile	slIndexfile;
 
  memset(&slIndexfile,0x00,sizeof(struct indexfile)); 
  slIndexfile.cTag	= '0';
  slIndexfile.lSerialno	= lSerialno;
  memcpy(slIndexfile.aFilename,aFilename,strlen(aFilename));
  
  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S3100: ��ϵͳ��������[BMQ_PATH]ʧ��");
    return(FAIL);
  }
 
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,BMQINDEXFILE);
  
  if ((fp = fopen(alFile, "r+")) == NULL)
  {
    if ((fp = fopen(alFile, "w+")) == NULL)
    {
      _bmqDebug("S3110: �ļ���ʧ��:%d,%s",errno,strerror(errno));
      return(FAIL);
    } 
  }
  fflush(fp);
  ilRc = fseek(fp, (lSerialno-1)*sizeof(struct indexfile), SEEK_SET);
  if (ilRc)
  {
    _bmqDebug("S3120: �ļ���λ����errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  
  ilRc = fwrite((char *)&slIndexfile, sizeof(char), sizeof(struct indexfile), fp);
  if (ilRc != sizeof(struct indexfile))
  {    
    _bmqDebug("S3130: д�ļ�����errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  
  fclose(fp);
  return(SUCCESS);	
}	

/**************************************************************
 ** ������:  _bmqGetindexfile
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2002/12/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
int _bmqGetindexfile(long lSerialno,char *aFilename)
{
  int		ilRc;
  char		alFile[80],*alPath;
  FILE		*fp;
  struct	indexfile	slIndexfile;
  
  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S3140: ��ϵͳ��������[BMQ_PATH]ʧ��");
    return(FAIL);
  }
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,BMQINDEXFILE);

  if ((fp = fopen(alFile, "r")) == NULL)
  {
    _bmqDebug("S3150: �����ļ���ʧ��:%d,%s",errno,strerror(errno));
    return(FAIL);
  }
    
  ilRc = fseek(fp, (lSerialno-1)*sizeof(struct indexfile), SEEK_SET);
  if (ilRc)
  {
    _bmqDebug("S3160: �ļ���λ����errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  fflush(fp);
  
  ilRc = fread((char *)&slIndexfile, sizeof(char), sizeof(struct indexfile), fp);
  if (ilRc != sizeof(struct indexfile))
  {
    _bmqDebug("S3170: �������ļ�����errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  
  strcpy(aFilename,slIndexfile.aFilename);
  fclose(fp);
  return(SUCCESS);	
}	

/**************************************************************
 ** ������:  bmqSendfile
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2002/12/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
int bmqSendfile(short iGrpid,char *aFilename,struct transparm sTransparm,
                long *lSerialno)
{
  int	ilRc;
  
  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S3180:%d",ilRc);
    return(ilRc);
  }
 
  /* �����ļ�Ψһ������ */
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  if(psgMbshm->lFileno >= MAXFILENO)
   psgMbshm->lFileno = 1;
  else
   psgMbshm->lFileno++;
  _bmqUnlock(LK_ALL);
  
  /* д�����ļ���¼ */ 
  ilRc = _bmqPutindexfile(psgMbshm->lFileno,aFilename);
  if( ilRc )
  {
    _bmqDebug("S3190: д������¼�ļ�����,ilRc=%d,Fileno=%ld,Filename=%s",
               ilRc,psgMbshm->lFileno,aFilename);
    return(FAIL);               
  }
  *lSerialno = psgMbshm->lFileno;
  /* _bmqDebug("S3200: *lSerialno=%ld",*lSerialno); */
  return(SUCCESS);	
}	

/**************************************************************
 ** ������:  bmqRecvfile
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2002/12/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
int bmqRecvfile(long lSerialno,FILE **fp)
{
  int	ilRc;  
  char  alFilename[80];
  
  /* ȡ���ļ�������¼ */   
  ilRc = _bmqGetindexfile(lSerialno,alFilename);
  if( ilRc )
  {
    _bmqDebug("S3210: ȡ��������¼�ļ�����,ilRc=%d,Fileno=%ld,Filename=%s",
               ilRc,lSerialno,alFilename);
    *fp = NULL;               
    return(FAIL);               
  }

  if ((*fp = fopen(alFilename, "r")) == NULL)
  {
    _bmqDebug("S3220: �ļ�[%s]��ʧ��:%d,%s",alFilename,errno,strerror(errno));
    *fp = NULL;
    return(FAIL); 
  }
  
  return(SUCCESS);	
}	

/**************************************************************
 ** ������:  bmqPutfile
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2002/12/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
/*int bmqPutfile(short iGrpid,short iMbid,short iPrior,long lType,
  char *aMsgbuf,short iMsglen,char *aFilename,struct transparm sTransparm)delete by wh*/
int bmqPutfile(short iGrpid,short iMbid,short iPrior,long lType,
  char *aMsgbuf,TYPE_SWITCH iMsglen,char *aFilename,struct transparm sTransparm)/*add by wh*/
{
  int	ilRc;
  long	llSerialno=0;
  
  if(aFilename != NULL)
  {
    ilRc = bmqSendfile(0,aFilename,sTransparm,&llSerialno);
    if(ilRc)
    {
      _bmqDebug("S3230: �����ļ�������¼ʧ��!");
      return(FAIL);
    }
  }
  
  ilRc = bmqPut(iGrpid,iMbid,iPrior,lType,llSerialno,aMsgbuf,iMsglen);
  if(ilRc)
  {
    _bmqDebug("S3240: [��������ʧ��]bmqPut error!");
    return(FAIL);	
  }
  
  return(SUCCESS);	
}

/**************************************************************
 ** ������:  bmqGetfile
 ** ��  ��:  
 ** ��  ��:  ���
 ** ��������:2002/12/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
/*int bmqGetfile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,
  char *aMsgbuf,short *piMsglen,short iTimeout,FILE **fp)delete by wh*/
int bmqGetfile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,
  char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout,FILE **fp)/*add by wh*/
{
  int	ilRc;
  
  ilRc = bmqGetw(piGrpid,piMbid,piPrior,plType,plClass,aMsgbuf,piMsglen,iTimeout);
  if(ilRc)
  {
    _bmqDebug("S3250: [��������ʧ��]bmqGetw error! ilRc=%d",ilRc);
    *fp = NULL;
    return(FAIL);
  }
    
  if(*plClass)
  {
    /* _bmqDebug("fzj2: *plClass=%ld",*plClass); */
    ilRc = bmqRecvfile(*plClass,fp);	
    if(ilRc)
    {
      _bmqDebug("S3260: [��������ʧ��]bmqRecvfile() error");
      return(FAIL);
    }    	
  }  	
     
  return(SUCCESS);	
}
/* add by xujun 2002.12.18 for file trans end */
/* add by wangpan */
/*int bmqGetfileX(short *piGrpid,short *piMbid,short *piPrior,long *plType,
   long *plClass, char *aMsgbuf,short *piMsglen,short iTimeout,char *aFileName) delete by wh*/
int bmqGetfileX(short *piGrpid,short *piMbid,short *piPrior,long *plType,
   long *plClass, char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout,char *aFileName)/*add by wh*/
{
  int	ilRc;
 
  ilRc = bmqGetw(piGrpid,piMbid,piPrior,plType,plClass,aMsgbuf,piMsglen,iTimeout);
  if(ilRc)
  {
    _bmqDebug("S3270: [��������ʧ��]bmqGetw error! ilRc=%d",ilRc);
    aFileName = NULL;
    return(ilRc);
  }
    
  if(*plClass)
  {
    ilRc = _bmqGetindexfile(*plClass,aFileName);
    if( ilRc )
    {
      _bmqDebug("S3280: ȡ��������¼�ļ�����,ilRc=%d,Fileno=%ld",
               ilRc,*plClass);
      aFileName[0] = 0x00;            
      return(ilRc);               
    }
  }  
     
  return(SUCCESS);	
}

/* add by wangpan end  */

/* add by xujun 2006.11.18 for file trans begin */
/*int bmqPutFile(short iGrpid,short iMbid,short iPrior,long lType,char *aMsgbuf,short iMsglen,char *aFilename)delete by wh*/
int bmqPutFile(short iGrpid,short iMbid,short iPrior,long lType,char *aMsgbuf,TYPE_SWITCH iMsglen,char *aFilename)/*add by wh*/
{
  int	ilRc;
  long	llSerialno = 0;
  char	alTmpFileName[100];
  char	alFileName[100];
  char  alVirName[100];
  char	alMsgbuf[iMBMAXPACKSIZE];
  struct transparm slTransparm;
  struct stat f_stat; 
  
  _bmqVdebug(2,"S3280 bmqPutFile(%d,%d,%d,%d,%d,%s)",iGrpid,iMbid,iPrior,lType,iMsglen,aFilename);
  memset(&sgMonMsg,0x00,sizeof(struct monmsg));
  if( stat( aFilename, &f_stat ) == -1 )
  {    
    _bmqDebug("S3281: �޷���ָ���ļ�[%s]!",aFilename);
    sgMonMsg.iSendFlag  = 5;
    strcpy(sgMonMsg.aFileName,aFilename);
    strcpy(sgMonMsg.aStatusDesc,"�޷���ָ���ļ�");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }

  /* add by xujun 2007.01.11
     ����ǰ��ⲡ��(������������˷��ͼ��) */
  if( psgMbinfo[igMb_sys_current_mailbox - 1].iSendScanFlag )
  { 
    memset(alVirName,0x00,sizeof(alVirName));
    ilRc = _bmqVirusScanByRoot(aFilename,sgRoot,alVirName);
    if(ilRc)
    {
      _bmqDebug("S3282: �ļ�[%s]����ɨ��ʧ��!",aFilename);
      sgMonMsg.iSendFlag  = 5;
      strcpy(sgMonMsg.aFileName,aFilename);
      strcpy(sgMonMsg.aStatusDesc,"����ɨ��ʧ��!");
      _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
      return(FAIL);
    }
    else if(strlen(alVirName))
    {
      _bmqDebug("S3283: �ļ�[%s]��������[%s]!",aFilename,alVirName);
      sgMonMsg.iSendFlag  = 5;
      strcpy(sgMonMsg.aFileName,aFilename);
      sprintf(sgMonMsg.aStatusDesc,"���ֲ���[%s]",alVirName);
      _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
      return(FAIL);
    }
  }

  slTransparm.bmqFile_comm	= _bmqFile_comm;
  slTransparm.bmqFile_msg	= _bmqFile_msg;
  
  if( iGrpid == 0 ) iGrpid = psgMbshm->iMbgrpid;
  if(psgMbshm->iMbgrpid == iGrpid) 
  {
    if(aFilename != NULL)
    {
      memset(alTmpFileName,0x00,sizeof(alTmpFileName));
      memset(alFileName,0x00,sizeof(alFileName));
      _bmqfilechr(aFilename,strlen(aFilename),alTmpFileName,'/');
      sprintf(alFileName,"%s/send/%s",getenv("BMQ_FILE_DIR"),alTmpFileName);
      ilRc = bmqSendfile(iGrpid,alFileName,slTransparm,&llSerialno);
      if(ilRc)
      {
        _bmqDebug("S3290: �����ļ�������¼ʧ��!");
        return(FAIL);
      }
    }
  }

  /* add by xujun 2006.11 begin
     ֧�ֿ����ļ�����,���ļ��������󷢵�ָ���������ģ��
  */
  sgMonMsg.iOrgGrpid  = psgMbshm->iMbgrpid;
  sgMonMsg.iOrgMbid   = igMb_sys_current_mailbox;
  sgMonMsg.iDesGrpid  = iGrpid;
  sgMonMsg.iDesMbid   = iMbid;
  sgMonMsg.lFileSize  = f_stat.st_size;
  sgMonMsg.lSendPid   = lgCurrent_pid;
  strcpy(sgMonMsg.aFileName,aFilename);
  ilRc = _bmqSendfile_G(iGrpid,aFilename,slTransparm,&llSerialno);
  if( ilRc )
  {
    _bmqDebug("S3300: �����ļ�����ʧ��,ilRc=[%d]",ilRc);
    /* ��SAFģ�鷢��ʧ�ܱ��� */
    memset(alMsgbuf,0x00,iMBMAXPACKSIZE);
    memcpy(alMsgbuf,(char *)&sgMonMsg,sizeof(struct monmsg));
    memcpy(alMsgbuf+sizeof(struct monmsg),aMsgbuf,iMsglen);
    bmqPut(0,iMBFILESAF,iPrior,lType,1,alMsgbuf,sizeof(struct monmsg)+iMsglen);
    return(ilRc);
  }
  /* add by xujun 2006.11 end
     ֧�ֿ����ļ����� */

  ilRc = bmqPut(iGrpid,iMbid,iPrior,lType,llSerialno,aMsgbuf,iMsglen);
  if(ilRc)
  {
    _bmqDebug("S3310: [��������ʧ��]bmqPut error!");
    return(FAIL);	
  }
  
  return(SUCCESS);	
}

int bmqRecvFile(short iGrpid,char *aFileName)
{
  int	ilRc;
  
  ilRc = bmqRecvfileByName(iGrpid,aFileName,'S');	
  if(ilRc)
  {
    _bmqDebug("S3320: [��������ʧ��]bmqRecvfileByName() ʧ��");
    return(ilRc);
  }  	
     
  return(SUCCESS);	
}

int bmqSendFile(short iGrpid,char *aFilename)
{
  int	ilRc;
  long	llSerialno = 0;
  struct transparm slTransparm;
  struct stat f_stat; 
  
  memset(&sgMonMsg,0x00,sizeof(struct monmsg));
  if( stat( aFilename, &f_stat ) == -1 )
  {
  	_bmqDebug("S3328: �޷���ָ���ļ�[%s]!",aFilename);
  	sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aFileName,aFilename);
    strcpy(sgMonMsg.aStatusDesc,"�޷���ָ���ļ�");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }
  
  slTransparm.bmqFile_comm	= _bmqFile_comm;
  slTransparm.bmqFile_msg	= _bmqFile_msg;
  
  sgMonMsg.iOrgGrpid  = psgMbshm->iMbgrpid;
  sgMonMsg.iOrgMbid   = igMb_sys_current_mailbox;
  sgMonMsg.iDesGrpid  = iGrpid;
  sgMonMsg.lFileSize  = f_stat.st_size;
  sgMonMsg.lSendPid   = lgCurrent_pid;
  strcpy(sgMonMsg.aFileName,aFilename);
  ilRc = _bmqSendfile_G(iGrpid,aFilename,slTransparm,&llSerialno);
  if( ilRc )
  {
    _bmqDebug("S3330: �ļ�����ʧ��,ilRc=[%d]",ilRc);
    /* ��SAFģ�鷢��ʧ�ܱ��� */
    bmqPut(0,iMBFILESAF,0,0,2,(char *)&sgMonMsg,sizeof(struct monmsg));
    return(ilRc);
  }

  return(SUCCESS);
}

/*int bmqGetFile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,short *piMsglen,short iTimeout,char *aFileName,char *alFilepath)delete by wh*/
int bmqGetFile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout,char *aFileName,char *alFilepath)/*add by wh*/
{
  int	ilRc;
  char	alVirName[100];
  char	alFilename[101];
  ilRc = bmqGetw(piGrpid,piMbid,piPrior,plType,plClass,aMsgbuf,piMsglen,iTimeout);
  if(ilRc)
  {
    _bmqDebug("S3340: [��������ʧ��]bmqGetw error! ilRc=%d",ilRc);
    return(FAIL);
  }
  if(*plClass)
  {
    /* add by xujun 2007.01.11
       ����ǰ��ⲡ��(������������˽��ռ��) */
    if( psgMbinfo[igMb_sys_current_mailbox - 1].iRecvScanFlag )
    { 
      /*ȡ���ļ�*/
      _bmqGetindexfile(*plClass,alFilename);
      memset(alVirName,0x00,sizeof(alVirName));
      memset(&sgMonMsg,0x00,sizeof(struct monmsg));
      ilRc = _bmqVirusScanByRoot(alFilename,sgRoot,alVirName);
      if(ilRc)
      {
        _bmqDebug("S3341: �ļ�[%s]����ɨ��ʧ��!",alFilename);
        sgMonMsg.iSendFlag  = 5;
        strcpy(sgMonMsg.aFileName,alFilename);
        strcpy(sgMonMsg.aStatusDesc,"����ɨ��ʧ��!");
        _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
        return(FAIL);
      }
      else if(strlen(alVirName))
      {
        _bmqDebug("S3342: �ļ�[%s]��������[%s]!",alFilename,alVirName);
        sgMonMsg.iSendFlag  = 5;
        strcpy(sgMonMsg.aFileName,alFilename);
        sprintf(sgMonMsg.aStatusDesc,"���ֲ���[%s]",alVirName);
        _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
        return(FAIL);
      }
    }
    ilRc = bmqRecvfileX(*plClass,aFileName);	
    if(ilRc)
    {
      _bmqDebug("S3350: [��������ʧ��]bmqRecvfileX() error");
      aFileName = NULL;
      return(FAIL);
    }
  }  	
  return(SUCCESS);	
}

/**************************************************************
 ** ������:  _bmqSendfile_G
 ** ����:    �����ļ�������Server
 ** ����:    ��� 	
 ** ��������: 2006/11/20
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: 0--�ɹ�
***************************************************************/
int _bmqSendfile_G(short iGrpid,char *aFilename,struct transparm sTransparm, long *lSerialno)
{
  int	ilRc,ilSockfd;
  /*short ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen;/*add by wh*/
  char	alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];
  char	alParm[10][iFLDVALUELEN];
  long  llOffset;
  FILE  *fq;
  char  alTmpFileName[100];
  char	alSendFileName[100];
  char  alOffset[20];
  char	alCommand[120];
  char  alResult[10];
  char  alInitfile[101];
  char	alFile[101];
  char alCompressFlag[10];
  char alCryptFlag[10];
  
  memset(alCompressFlag,0x00,sizeof(alCompressFlag));
  
  if( iGrpid == 0 ) iGrpid = psgMbshm->iMbgrpid;  
  /* ���鴫��ֱ�ӿ����ļ���sendĿ¼ */
  if(psgMbshm->iMbgrpid == iGrpid) 
  {
  	memset(alCommand,0x00,sizeof(alCommand));
    _bmqfilechr(aFilename,strlen(aFilename),alTmpFileName,'/');
    sprintf(alSendFileName,"%s/send/%s",getenv("BMQ_FILE_DIR"),alTmpFileName);
    sprintf(alCommand,"cp %s %s",aFilename,alSendFileName);
    system(alCommand);
  	sgMonMsg.iSendFlag  = 3;
    sgMonMsg.lSendSize  = sgMonMsg.lFileSize;
    ftime(&sgMonMsg.sBeginTime);
    strcpy(sgMonMsg.aStatusDesc,"�����ļ��������");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
  	return(SUCCESS);
  }

  /* ���ӵ������� */
  ilSockfd = _bmqConnectfileser(iGrpid);
  if ( ilSockfd < 0 )
  {
    _bmqDebug("S3360: --%d--���ӵ�����������!",ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"���ӷ�����ʧ��");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }
  
  if( ( *lSerialno == 0 ) && (psgMbshm->iMbgrpid != iGrpid) )
  {
    memset(alCommand,0x00,sizeof(alCommand));
    _bmqfilechr(aFilename,strlen(aFilename),alTmpFileName,'/');
    sprintf(alSendFileName,"%s/saf/%s",getenv("BMQ_FILE_DIR"),alTmpFileName);
    sprintf(alCommand,"ln %s %s",aFilename,alSendFileName);
    system(alCommand);
  }

  /* add by xujun 2006.11.15 ֧�ֶϵ����� begin */
  memset(alOffset,0x00,sizeof(alOffset)); 
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  _bmqfilechr(aFilename,strlen(aFilename),alFile,'/');
  sprintf(alTmpFileName,"%s/temp/%s.tpf",getenv("BMQ_FILE_DIR"),alFile);
  llOffset = 0;
  if (( fq = fopen(alTmpFileName,"r")) != NULL)
  {
    /*�ļ�����,�ϴδ���δ���,ȡ���ϴδ��͵�ƫ����*/
    ilRc = fread(alOffset,sizeof(char),sizeof(alOffset),fq);
    llOffset = atol(alOffset);
    fclose(fq);
  }
  /* add by xujun 2006.11.15 ֧�ֶϵ����� end */

  /* ��ȡ����,�Ƿ���Ҫ���ܺ�ѹ�� */
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"COMPRESSFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: ���ļ�%s��ȡ COMPRESSFLAG �ֶ�ʧ��!",alInitfile);
    alResult[0] = '0'; /*ȱʡΪ��ѹ��*/
  }
  strcpy(alCompressFlag,alResult);
  ilRc = _bmqGetfield(alInitfile,"CRYPTFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: ���ļ�%s��ȡ CRYPTFLAG �ֶ�ʧ��!",alInitfile);
    alResult[0] = '0'; /*ȱʡΪ������*/
  }
  strcpy(alCryptFlag,alResult);
  /* ��֯����� */
  ilMsglen = 29 + strlen(aFilename);
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 8;
  sprintf(alVal,"%5s",alCompressFlag);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%5s",alCryptFlag);
  memcpy(alMsgbuf+14,alVal,5);
  sprintf(alVal,"%10ld",llOffset);
  memcpy(alMsgbuf+19,alVal,10);
  memcpy(alMsgbuf+29,aFilename,strlen(aFilename));

  /* ���Ϳ�ʼ�����ļ�����ĵ������� */ 
  if(llOffset == 0)
  {
    strcpy(sgMonMsg.aStatusDesc,"��ʼ�����ļ�");
    sgMonMsg.iSendFlag  = 1;
  }
  else
  {
  	strcpy(sgMonMsg.aStatusDesc,"���������ļ�");
    sgMonMsg.iSendFlag  = 2;
  }
  sgMonMsg.lSendSize  = llOffset;
  sgMonMsg.fSendSpeed = 0.00;
  
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S3370: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    llOffset = 0;
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"��ʼ��������ʧ��");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }

  /* ������Ӧ���� */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S3380: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"���տ�ʼ����������Ӧ����ʧ��");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }

  if(memcmp(alMsgbuf,"OK",2))
  {
    _bmqDebug("S3390: �������ܾ������ļ�,retbuf=[%s]",alMsgbuf);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"�������ܾ������ļ�");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }

  if(!llOffset)
  {
    memcpy(alVal,alMsgbuf+2,15);
    alVal[15] = 0x00;
    *lSerialno = atol(alVal);
    sgMonMsg.lSerialno = *lSerialno;
  }
  memcpy(alVal,alMsgbuf+17,15);
  alVal[25] = 0x00;
  llOffset   = atol(alVal);  
  ftime(&sgMonMsg.sBeginTime);
  _bmqMonMsgSend(MONMSGCODE,sgMonMsg);

  _bmqfilechr(aFilename,strlen(aFilename),alTmpFileName,'/');
  sprintf(alSendFileName,"%s/saf/%s",getenv("BMQ_FILE_DIR"),alTmpFileName);
  strcpy(alParm[0],alSendFileName);
  sprintf(alParm[1],"%d",ilSockfd);
  sprintf(alParm[2],"%ld",llOffset);
  sprintf(alParm[3],"%5s",alCompressFlag);
  sprintf(alParm[4],"%5s",alCryptFlag);

  /* ���ú���ָ������ļ�Ԥ����ͷ��� */
  ilRc = sTransparm.bmqFile_msg(alParm);
  if(ilRc)
  {
    _bmqDebug("S3400: �ļ���Ϣ���������������˳����ͽ���!");
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"�ļ�Ԥ����ʧ��");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(ilRc);
  }

  ilRc = sTransparm.bmqFile_comm(alParm);
  if(ilRc)
  {
    _bmqDebug("S3410: �ļ����ͷ�������,�˳����ͽ���!���������ԭ������·���.");
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"�ļ�����ʧ��");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(ilRc);
  }
   
  /* ��֯����� */
  ilMsglen = 9;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 9;			/* �����־ 9=Sendfile end */  

  /* ���ͽ������ͱ�־�������� */
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S3420: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"���ͽ������ͱ�־ʧ��");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }
  
  /* ������Ӧ���� */
  ilMsglen = 2;
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S3430: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"���ͽ�����Ӧ����ʧ��");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }
  if(memcmp(alMsgbuf,"OK",2))
  {
    _bmqDebug("S3440: ���շ�δ����ȷ�����ļ�[%s],��Ҫ�ط�!",aFilename);
    ilMsglen = 16;
    ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
    if ( ilRc != 0 ) 
    {
      _bmqDebug("S3450: Receive from server error!!--%d",ilRc);
      close(ilSockfd);
      sgMonMsg.iSendFlag  = 4;
      strcpy(sgMonMsg.aStatusDesc,"���ͽ�����Ӧ����ʧ��");
      _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
      return (FAIL);
    }
    alMsgbuf[ilMsglen] = 0x0;
    if (( fq = fopen(alTmpFileName,"w+")) != NULL)
    {
      /*д�봫�͵�ƫ����*/
      ilRc = fwrite(alMsgbuf,sizeof(char),ilMsglen,fq);
      fclose(fq);
    }
    
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    sgMonMsg.lSendSize  = atol(alMsgbuf);
    strcpy(sgMonMsg.aStatusDesc,"���շ�δ����ȷ�����ļ�");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }

  close(ilSockfd);
  sgMonMsg.iSendFlag  = 3;
  sgMonMsg.lSendSize  = sgMonMsg.lFileSize;
  strcpy(sgMonMsg.aStatusDesc,"�ļ��������");
  ftime(&sgMonMsg.sEndTime);
  sgMonMsg.fSendSpeed = (double)sgMonMsg.lFileSize / 
                         ((sgMonMsg.sEndTime.time-sgMonMsg.sBeginTime.time)*1000 + 
                          (sgMonMsg.sEndTime.millitm -sgMonMsg.sBeginTime.millitm));
  _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  _bmqfilechr(aFilename,strlen(aFilename),alFile,'/');
  sprintf(alTmpFileName,"%s/saf/%s",getenv("BMQ_FILE_DIR"),alFile);
  unlink(alTmpFileName);
  return(SUCCESS);
}  
/* add by xujun 2006.11.18 for file trans begin */

/********************************************************************
 ** ������:bmqPutSavePack
 ** ����:  ��Źر�ʱ����ı���
 ** ����:  ���
 ** ��������: 2007/01/09
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iOrgGrpid-Դ����  iOrgMbid-Դ������  iMbid-Ŀ������  iPrior-���ȼ�
             aFilter-���Ĺ������� aMsgbuf-����Buf iMsglen-���ĳ��� 
 ** ����ֵ: SUCCESS
*********************************************************************/
/*int bmqPutSavePack(short iOrgGrpid,short iOrgMbid,short iMbid,short iPrior,char *aFilter,
  char *aMsgbuf,short iMsglen)delete by wh*/
int bmqPutSavePack(short iOrgGrpid,short iOrgMbid,short iMbid,short iPrior,char *aFilter,
  char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    ilRc;
  long   llPack_head,llQueue_id;

  if( igDebug >= 2)
  {
    _bmqDebug("S3460 ***���ú��� bmqPutSavePack(%d,%d,%d,%d,aFilter,aMsgbuf,%d)***",
      iOrgGrpid,iOrgMbid,iMbid,iPrior,iMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    
  }

  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() ) 
  {
    _bmqDebug("S3470 bmqPutSavePack() û�д������ִ�в���!!");
    return(1011);
  }
 
  /*�жϲ����Ϸ���*/
  if( iOrgGrpid < 0) return(1009);
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) ) return(1004);
  if( (iPrior < 0) || (iPrior > 99) ) return(1010);

   /*�ж��Ƿ��Ѿ�����ָ����ֵ*/
  if( psgMbinfo[iMbid-1].lMaxqueue && psgMbinfo[iMbid-1].lPendnum >= 
    psgMbinfo[iMbid-1].lMaxqueue )
  {
    return(1008);
  }
  
  /*�жϱ��ĳ����Ƿ񳬹����ֵ*/
  if(iMsglen > iMBMAXPACKSIZE)
  {
    _bmqDebug("S3480 ���ĳ���[%d]�������ֵ�� [%d]",iMsglen,iMBMAXPACKSIZE);
    return(1012);
  }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*�洢����*/
  ilRc = _bmqPack_put(aMsgbuf,iMsglen,&llPack_head);
  if (ilRc)
  {
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S3490 ����bmqPutSavePack()����ı�������aMsgbufΪ%s",aMsgbuf);
  }

  /*�洢����*/
  ilRc = _bmqQueue_put( 0, iMbid, iPrior, aFilter,llPack_head,&llQueue_id);
  if (ilRc)
  {
    _bmqDebug("S3500 �洢����_bmqQueue_put(...)����!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  /*�޸�������Ϣ*/
  psgMbinfo[iMbid-1].lPendnum++;
  /*�޸Ķ�����Ϣ*/
  psgMbqueue[llQueue_id-1].iOrg_grp     = iOrgGrpid;
  psgMbqueue[llQueue_id-1].iOrg_mailbox = iOrgMbid;

  _bmqUnlock(LK_ALL);

  if(igDebug >= 1)
  {
    _bmqDebug("S3510 ��[%d]����[%d]������[%d]����[%d]����",
      iOrgGrpid,iOrgMbid,iMbid,llPack_head); 
  }
  if( igDebug >= 2)
  {
    _bmqDebug("S3520 ***�������ú��� bmqPutSavePack(%d,%d,%d,%d,aFilter,aMsgbuf,%d)***",
    iOrgGrpid,iOrgMbid,iMbid,iPrior,iMsglen);
  }

  return(SUCCESS);
}

/* add by hxz 2007.01.19 for mulfiles send Begin */
int bmqMulPutFile(struct bmqMessage sendmulfile)
{
 short ilGrpid;
 short ilMbid;
 short ilPrior;
 long llType;
 char alVal[iMASKLEN + 80];
 char alMsgbuf[iMBMAXPACKSIZE+100];
 /*short ilMsglen=0;delete by wh*/
 TYPE_SWITCH ilMsglen=0;/*add by wh*/
 char alTopic[41];
 char alCommand[256];
 char alFilename[256],alFile[256],alPath[256],alSendFile[256];
 int	i,ilRc=0,ilfilenum=0;
 struct stat f_stat; 
 
 ilGrpid   = sendmulfile.iGrpid;
 ilMbid    = sendmulfile.iMbid;
 ilPrior   = sendmulfile.iPrior;
 llType    = sendmulfile.lType;
 ilfilenum = sendmulfile.iFilenum;
 strcpy(alTopic,sendmulfile.aTopic);
 _bmqTrim(alTopic);
 
 sprintf(alPath,"%s/temp",getenv("BMQ_FILE_DIR"));
 sprintf(alSendFile,"%s/temp/%s",getenv("BMQ_FILE_DIR"),alTopic);
 _bmqDelspace(alSendFile);
 unlink(alSendFile);
 
 memset(alMsgbuf,0x00,sizeof(alMsgbuf));
 memset(alVal,0x00,sizeof(alVal));     
 sprintf(alVal,"%40s",alTopic);
 memcpy(alMsgbuf,alVal,40);
 sprintf(alVal,"%5d",ilfilenum);
 memcpy(alMsgbuf+40,alVal,5);
 ilMsglen=45;
 for(i=0;i<ilfilenum;i++)
 {
   if( stat( sendmulfile.aFileName[i], &f_stat ) == -1 )
   {
      _bmqDebug("S0470: ���ܴ��ļ�[%s]",sendmulfile.aFileName[i]);
      return(-1);
   }
   memset(alFilename,0x00,sizeof(alFilename));
   memset(alVal,0x00,sizeof(alVal));     
   strcpy(alFilename,sendmulfile.aFileName[i]);
   _bmqTrim(alFilename);
   sprintf(alCommand,"ln %s %s",alFilename,alPath); 
   system(alCommand);
   _bmqfilechr(alFilename,strlen(alFilename),alFile,'/');
   sprintf(alFilename,"%s/%s",alPath,alFile); 
   sprintf(alVal,"%80s",alFile);       
   memcpy(alMsgbuf+ilMsglen,alVal,80);
   ilMsglen=ilMsglen+80;
   sprintf(alVal,"%16ld",f_stat.st_size);
   memcpy(alMsgbuf+ilMsglen,alVal,16);
   ilMsglen=ilMsglen+16;
   sprintf(alCommand,"bmqfiletar_r %s %s %s",alPath,alSendFile,alFile);
   system(alCommand);
   unlink(alFilename);   
 }    
 if(ilfilenum)
    ilRc = bmqPutFile(ilGrpid,ilMbid,ilPrior,llType,alMsgbuf,ilMsglen,alSendFile);
 else
    ilRc = bmqPut(ilGrpid,ilMbid,ilPrior,llType,0,alMsgbuf,ilMsglen);
 sprintf(alVal,"%05d",ilRc);
 memcpy(alMsgbuf+1,alVal,5);
 ilMsglen = 6;
 unlink(alSendFile);
 return(SUCCESS);
}
/* add by hxz 2007.01.19 for mulfiles send End */

/* add by hxz 2007.01.19 for mulfiles reveice Begin */
int bmqMulGetFile(struct bmqMessage *revmulfile,short iTimeout,char aFilepath[256])
{
  short piGrpid;
  short piMbid;
  short piPrior;
  long plType;
  long plClass;
  char aMsgbuf[iMBMAXPACKSIZE+100];
  /*short piMsglen;delete by wh*/
  TYPE_SWITCH piMsglen;/*add by wh*/
  short ilTimeout;
  char aFileName[100];
  char alFilepath[256];
  int	ilRc=0,i=0,ilVal;  
  char alVal[iMASKLEN + 80];
  char alCommand[256];
  
  memset(alCommand,0x00,sizeof(alCommand));
  memset(aFileName,0x00,100);
  piGrpid   = revmulfile->iGrpid;
  piMbid    = revmulfile->iMbid;
  piPrior   = revmulfile->iPrior;
  plType    = revmulfile->lType;
  plClass   = 0;
  ilTimeout = iTimeout;
  
  strcpy(alFilepath,aFilepath);
  memset(aMsgbuf,0x00,sizeof(aMsgbuf));
  ilRc = bmqGetFile(&piGrpid,&piMbid,&piPrior,&plType,&plClass,aMsgbuf,&piMsglen,ilTimeout,aFileName,alFilepath);
  if(ilRc)
  {
     _bmqDebug("S0610: [��������ʧ��]bmqGetFile error!");
     return(FAIL);	
  }
  revmulfile->iGrpid = piGrpid;
  revmulfile->iMbid  = piMbid;
  revmulfile->iPrior = piPrior;
  revmulfile->lType  = plType;
  revmulfile->lClass = plClass;
  memset(alVal,0x00,sizeof(alVal));
  memcpy(alVal,aMsgbuf+40,5);
  ilVal=atoi(alVal);
  revmulfile->iFilenum=ilVal;
  for(i=0;i<ilVal;i++)
  {
    memset(revmulfile->aFileName[i],0x00,100);
    memset(alVal,0x00,sizeof(alVal));
    memcpy(alVal,aMsgbuf+45+96*i,96);
    memcpy(revmulfile->aFileName[i],alVal,80);
    _bmqTrim(revmulfile->aFileName[i]);
  }
  sprintf(alCommand,"bmqfiletar_x %s %s",aFilepath,aFileName);
  system(alCommand);
  unlink(aFileName);
  return(SUCCESS);  
}

/* add by hxz 2007.01.19 for delete whitespace begin */
int _bmqDelspace(char *alstr)
{
 int i,n=0;
 char alTmpstr[iMBMAXPACKSIZE];
 memset(alTmpstr,0x00,sizeof(alTmpstr));
 _bmqTrim(alstr);
 for(i=0;i<strlen(alstr);i++) 
 {
  if(!(alstr[i]==' '))
  {
   alTmpstr[n]= alstr[i];
   n++;
  }
 }  
 strcpy(alstr,alTmpstr);
 return(0);
}
/* add by hxz 2007.01.19 for delete whitespace end */

/* add by xujun 2007.01.26 for msg route begin */
/**************************************************************
 ** ������:bmqSend
 ** ����:  ��ű���,����ָ��Ŀ��,���ݱ��Ĺؼ����Զ�·��
 ** ����:  ���
 ** ��������: 2007/01/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iPrior-���ȼ� iType-����Type  iClass-����Class 
             aMsgbuf-����Buf iMsglen-���ĳ��� 
 ** ����ֵ: SUCCESS
***************************************************************/
/*int bmqSend(short iPrior,long lType,long lClass,char *aMsgbuf,short iMsglen) delete by wh*/
int bmqSend(short iPrior,long lType,long lClass,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    ilRc;
  short  ilResultlen;               /*�������*/
  short  ilGrpid;                   /*Ŀ����*/
  short  ilMbid;                    /*Ŀ������*/
  char   alExpress[iFLDVALUELEN];   /*��������ʽ*/
  char   alTmpexp[iFLDVALUELEN];    /*�沨�����ʽ*/
  
  if(strlen(psgMbinfo[igMb_sys_current_mailbox - 1].aExpress) < 3)
  {
    _bmqDebug("S8010 ����[%d]��֧���Զ�·��,��������ʽ=[%s]",
      igMb_sys_current_mailbox,psgMbinfo[igMb_sys_current_mailbox - 1].aExpress);
    return(FAIL);
  }
  
  /*���㱨��������*/
  memset(alTmpexp,0x00,sizeof(alTmpexp));
  agMsgbody    = aMsgbuf;
  igMsgbodylen = iMsglen;
  /*ת��Ϊ�沨�����ʽ*/
  ilRc = _swExpN2T(psgMbinfo[igMb_sys_current_mailbox - 1].aExpress,alTmpexp);
  if(ilRc) 
  {
    _bmqDebug("S8020 [%s]ת��Ϊ�沨�����ʽʧ��",alExpress);
    return(FAIL);
  }
  _bmqVdebug(1,"S8030 �沨�����ʽ=[%s]",alTmpexp);
  
  /*����ƽ̨���ʽ*/
  memset(alExpress,0x00,sizeof(alExpress));
  ilRc = _swExpress(alTmpexp,alExpress,&ilResultlen);  
  if(ilRc)
  {
    _bmqDebug("S8040 _swExpress����ƽ̨���ʽ����");
    return(FAIL);
  }
  _bmqVdebug(1,"S8050 ����������=[%s]",alExpress);
  
  /*������������ȡĿ�����Ŀ������*/
  ilRc = _bmqGetRouter(igMb_sys_current_mailbox,alExpress,&ilGrpid,&ilMbid);
  if(ilRc)
  {
    _bmqDebug("S8060 ����Դ����[%d]��������[%s]��ȡ·��ʧ��",igMb_sys_current_mailbox,alExpress);
    return(FAIL);
  }
  _bmqVdebug(1,"S8070  Ŀ����=[%d],Ŀ������=[%d]",ilGrpid,ilMbid);

  return(bmqPut(ilGrpid,ilMbid,iPrior,lType,lClass,aMsgbuf,iMsglen));
}
/* add by xujun 2007.01.26 for msg route end */


/* add by hxz 2007.02.28 for get msg content Begin */
/**************************************************************
 ** ������:bmqJava_getfilter
 ** ����:  ��ȡ����(������)
 ** ����:  
 ** ��������: 
 ** ����޸�����:
 ** ������������:_bmqQueue_get,_bmqPack_get
 ** ȫ�ֱ���:
 ** ��������:piGrpid-Դ����       piMbid-Դ������  piPrior-���ȼ�
             aFilter-���Ĺ������� aMask-��������   aMsgbuf-����Buf
             piMsglen-���ĳ���
 ** ����ֵ: SUCCESS,������
***************************************************************/
/*int bmqJava_getfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,
  char *aMask,char *aMsgbuf,short *piMsglen) delete by wh*/
int bmqJava_getfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,
  char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen) /*add by wh*/
{
  int  ilRc;
  long llPack_head;
  
  
  printf("In bmqJava_getfilter\n");
  if(igDebug >= 2)
  {
  	printf("arm 1\n");
    _bmqDebug("S0420 ***���ú���:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      piGrpid,piMbid,piPrior,piMsglen);
     printf("***���ú���:bmqGet(%d,%d,%d,%s,%s,%s,%d)***\n",*piGrpid,*piMbid,*piPrior,aFilter,aMask,aMsgbuf,*piMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);    
  }
  printf("arm 2\n");
  /* �жϵ�ǰ�����Ƿ�������� */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGet() û�д������ִ�в���!!");
      return(1011);
    }
  printf("arm 3\n");
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  printf("arm 4\n");
  /*����������м�¼,ȡ����������ͷ*/
  ilRc = _bmqQueue_javaget(piGrpid,piMbid,piPrior,aFilter,aMask,&llPack_head);
  printf("arm 5\n");
  printf("ilRc=%d\n",ilRc);
  printf("llPack_head=%ld\n",llPack_head);
  if (ilRc)
  {
    if(ilRc != 1001)  	
      _bmqDebug("S0430 ȡ����������ͷ����!");
    /*_bmqUnlock(LK_ALL);*/
    /*return(ilRc);*/
  }
 
  /*�޸�������Ϣ*/
  /*
  psgMbinfo[igMb_sys_current_mailbox-1].lPendnum--;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime));
  */
  /*��ȡ����*/
  printf("arm 6\n");
  printf("llPack_head=%d\n",llPack_head);
  ilRc = _bmqPack_javaget(aMsgbuf,piMsglen,llPack_head);
  printf("aMsgbuf=%s\n",aMsgbuf);
  printf("arm 7\n");
  printf("ilRc=%d\n",ilRc);
  if( ilRc )
  {
    _bmqDebug("S0440 ��ȡ����ERROR!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  /*_bmqUnlock(LK_ALL);
  if(igDebug >= 1)
  {
    _bmqDebug("S0450 ����[%d]ȡ����[%d]����[%d]��[%d]����",
      igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
  }
  */

  if(igDebug >= 3)
  {
    _bmqDebug("S0460 ����[%d]�޸ĺ���ϢlPendnum[%d],lRecvpid[%d],lRecvtime[%d]",
      igMb_sys_current_mailbox,
      psgMbinfo[igMb_sys_current_mailbox-1].lPendnum,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime);

  }

  if(igDebug >= 2)
  {
    _bmqDebug("S0470 ***�������ú���:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S0480 ***����bmqGetȡ���ı�������aMsgbufΪ***");
    _bmqDebug("S0490 %s",aMsgbuf); 
  }

  return(SUCCESS);
}

/* add by hxz 2007.02.28 for get msg content end */

/* add by hxz 2007.02.28 for get msg content begin */
/**************************************************************
 ** ������: _bmqQueue_javaget
 ** ����:   ��ȡһ�����м�¼
 ** ����:   
 ** ��������: 
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:piGrpid--���   piMbid--Ŀ������?piPrior--���ȼ� 
             aFilter--���Ĺ�������  aMask--��������   
             plPack_head--��������ͷ
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
*****************************************************************************************************/
static int _bmqQueue_javaget(short *piGrpid,short *piMbid,short *piPrior,
char *aFilter,char *aMask,long *plPack_head)
{
  int    ilSize,ilRc,i,ilFilter_flag;
  FILE   *fp;
  long   llQueue_head;
  /*char   alBuf[iFLDVALUELEN];*/
  struct mbqueue slMbqueue;
  char alResult1[iMASKLEN], alResult2[iMASKLEN];
  printf("In _bmqQueue_javaget\n");
  printf("iMbid=%d\n",*piMbid);
  if( igDebug >=2)
  {
    _bmqDebug("S1180 ***������ȡһ�����м�¼����:_bmqQueue_get(%d,%d,%d,aFilter,aMask)***",
      piGrpid,piMbid,piPrior);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }
  printf("In _bmqQueue_javaget[1]\n");
  ilFilter_flag = 0;
  for(i=0;i<iMASKLEN;i++)
  {
    if(aFilter[i] != 0x00)
    {
      ilFilter_flag = 1;
      break;
    }
  }
  printf("In _bmqQueue_javaget[2]\n");
  /* ȡ��������Ϣ����ͷ */
  llQueue_head = psgMbinfo[igMb_sys_current_mailbox - 1].lQueue_head;
  ilSize       = sizeof(struct mbqueue) + 1;
  
  for(;;)
  {
  	printf("llQueue_head=%ld\n",llQueue_head);
    /* δ�ҵ�ָ����¼ */
    if (llQueue_head == 0)
    {
    	printf("starwolf\n");
      return(1001);
    }
    /* ��¼�ڻ����ļ��� */
    if (llQueue_head > psgMbshm->lMbqueuecount)
    {
    	printf("��¼�ڻ����ļ���\n");
      /* �򿪶��л����ļ� */
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1190 ���ļ�����");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_pick(fp,&slMbqueue,llQueue_head);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1200 ���л����ļ���ȡ��¼ʧ��!");
        _bmqClosefile(fp);
        _bmqUnlock(LK_ALL);
        return(FAIL);
      }

      /* ƥ����Ϣ Filter,Mask */
      
      ilRc = _bmqStrand(aFilter, aMask, iMASKLEN, alResult1);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      ilRc = _bmqStrand(slMbqueue.aFilter, aMask, iMASKLEN, alResult2);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      printf("alResult1=%s\n",alResult1);
      printf("alResult2=%s\n",alResult2);
      if (!ilFilter_flag || memcmp(alResult1, alResult2, iMASKLEN) == 0)
      {
        /*
        ilRc = _bmqQueue_del(igMb_sys_current_mailbox,&slMbqueue,piGrpid,piMbid,
                             piPrior,aFilter,plPack_head);
        if(ilRc == WITHOUTUSE)
        {
          _bmqDebug("S1208 ɾ���Ķ���[%d]û������",llQueue_head);
          return (FAIL);
        }
        */
        /* ��д�ļ� */
        /*
        alBuf[0] = '0';
        fseek(fp,0,SEEK_SET);
        ilRc = _bmqPutfile(fp, (llQueue_head-psgMbshm->lMbqueuecount-1)*ilSize, 
          ilSize,alBuf);
        if (ilRc)
        {
          _bmqDebug("S1210 д�ļ�����");
          _bmqClosefile(fp);
          return (FAIL);
        }
        */
        _bmqClosefile(fp);
        break;
      }
      else
      {
        llQueue_head = slMbqueue.lRlink;
        _bmqClosefile(fp);
      }
    }
    /* ��¼�ڹ����ڴ��� */
    else
    {
      /* ƥ����Ϣ Filter,Mask */
      printf("��¼�ڹ����ڴ���\n");
      ilRc = _bmqStrand(aFilter, aMask, iMASKLEN, alResult1);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      printf("��ʼȡ�ڴ��¼\n");
      ilRc = _bmqStrand(psgMbqueue[llQueue_head-1].aFilter,aMask,iMASKLEN,alResult2);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      printf("ȡ�ڴ��¼����\n");
      printf("alResult1=%s\n",alResult1);
      printf("alResult2=%s,aMask=%s,iMASKLEN=%d\n",alResult2,aMask,iMASKLEN);
      printf("ilFilter_flag=%d\n",ilFilter_flag);
      printf("memcmp(alResult1, alResult2, iMASKLEN)=%d\n",memcmp(alResult1, alResult2, iMASKLEN));
      if (!ilFilter_flag || memcmp(alResult1, alResult2, iMASKLEN) == 0)
      {
      	/*
        ilRc = _bmqQueue_del(igMb_sys_current_mailbox,
                             &psgMbqueue[llQueue_head-1],
                             piGrpid,piMbid,piPrior,aFilter,plPack_head);
        if(ilRc == WITHOUTUSE)
        {
          _bmqDebug("S0672 ɾ���Ķ���[%d]û������",llQueue_head);
          return (FAIL);
        }        
        */
        printf("Break\n");
        break;
        
      }
      else
        llQueue_head = psgMbqueue[llQueue_head-1].lRlink;
    } 
  }   
  printf("In _bmqQueue_javaget[3]\n");
  if( igDebug >=2)
  {
    _bmqDebug("S1220 ***�������ú���:_bmqQueue_get(...)***");
  }
  return(SUCCESS);
}
/* add by hxz 2007.02.28 for get msg content end */

/* add by hxz 2007.02.28 for get msg content begin */
/**************************************************************
 ** ������: _bmqPack_javaget
 ** ����:   ��ȡһ�����ļ�¼
 ** ����:   
 ** ��������: 
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:aMsgbuf--���Ĵ洢buf iMsglen--���ĳ���
             lPack_head--���Ĵ洢��¼����ͷ
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*int _bmqPack_javaget(char *aMsgbuf,short *piMsglen,long lPack_head) delete by wh*/
int _bmqPack_javaget(char *aMsgbuf,TYPE_SWITCH *piMsglen,long lPack_head)/*add by wh*/
{
  long    i;
  FILE    *fp;
  int     ilRc;
  struct  mbfilepack slMbfilepack;
  printf("In _bmqPack_javaget\n");
  if(igDebug >= 2) 
    _bmqDebug("S1090 ***���ú���:_bmqPack_get() lPack_head=%d***",lPack_head);

  /* add by xujun 2001.10.16*/
  printf("%ld\n",lPack_head);
  if (lPack_head <= 0)
  {
  	printf("�Ƿ��ı���ͷֵ\n");
    _bmqDebug("S0191 �Ƿ��ı���ͷֵ:%ld",lPack_head);
    return(FAIL);
  }
  /* add end by xujun*/

  if (lPack_head > psgMbshm->lMbpacksize )
  {
  	printf("ȡ�����Ĵ洢������¼\n");
    /*ȡ�����Ĵ洢������¼*/
    if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
    {
      _bmqDebug("S1100 ���ļ�����!");
      return (FAIL);
    }
    /* �ӱ��Ļ����ļ���ȡһ��ָ����¼ */
    _bmqPackfile_pick(fp,&slMbfilepack,lPack_head);

    *piMsglen = slMbfilepack.lLoglen;
    /* �ӱ��Ĵ洢�ļ�����ȡһ�����Ĵ洢��¼ */
    ilRc =  _bmqGetlog(slMbfilepack.aLogfile,*piMsglen,
      slMbfilepack.lLogoffset,aMsgbuf);
    if (ilRc) 
    {
      _bmqClosefile(fp);
      return(ilRc);
    }
    printf("hxz!!!!!!!!!!\n");
    printf("%s\n",aMsgbuf);
    /*���±��Ĵ洢������¼*/
    _bmqPackfile_set(fp,0,0,lPack_head);
    _bmqClosefile(fp);

    if(igDebug >= 2)
    {
      _bmqDebug("S1110 ���ļ���ȡ����:file=[%s],offset=[%ld],len=[%d]",
        slMbfilepack.aLogfile,
          slMbfilepack.lLogoffset,*piMsglen);
    }
    return(SUCCESS);
  }
  i = lPack_head;
  *piMsglen = 0;
  for(;;)
  {
    memcpy(aMsgbuf+(*piMsglen),psgMbpack[i-1].aPack,
      psgMbpack[i-1].iMsglen);
    *piMsglen = *piMsglen + psgMbpack[i-1].iMsglen;
    printf("piMsglen=%d\n",*piMsglen);
    if(igDebug >= 2)
    {
      _bmqDebug("S1120 ���ڴ���ȡ����:ID=[%d],len=[%d]",i,psgMbpack[i-1].iMsglen);
    }
    psgMbpack[i-1].iMsglen = 0;
    i = psgMbpack[i-1].lLink;

    if (i > psgMbshm->lMbpacksize)
    {
      /*ȡ�����Ĵ洢������¼*/
      if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
      {
        _bmqDebug("S1130 ���ļ�����");
        return (FAIL);
      }
      _bmqPackfile_pick(fp,&slMbfilepack,i);

      ilRc =  _bmqGetlog(slMbfilepack.aLogfile,slMbfilepack.lLoglen,
      slMbfilepack.lLogoffset,aMsgbuf+*piMsglen);
      if (ilRc)
      {
        _bmqDebug("S1140 �Ӽ�¼�ļ�����ȡ���ĳ���,_bmqGetlog(..)");
        _bmqClosefile(fp);
        return(ilRc);
      }
      /*���±��Ĵ洢������¼*/
      _bmqPackfile_set(fp,0,0,i);
      _bmqClosefile(fp);

      *piMsglen = *piMsglen + slMbfilepack.lLoglen;

      if(igDebug >= 2)
      {
        _bmqDebug("S1150 ���ļ���ȡ����:file=[%s],offset=[%ld],len=[%d]",
          slMbfilepack.aLogfile,
            slMbfilepack.lLogoffset,
              slMbfilepack.lLoglen);
      }
      if (igDebug >= 2) _bmqDebug("S1160 ***�������ú���:_bmqPack_get()***");
      return(SUCCESS);
    }

    if (i == 0) break;
  }
  if (igDebug >= 2) _bmqDebug("S1170 ***�������ú���:_bmqPack_get()***");
  return(SUCCESS);
}
/* add by hxz 2007.02.28 for get msg content end */

/**************************************************************
 ** ������:NbPut
 ** ����:  ��ű���
 ** ����: ʷ����
 ** ��������: 2000/10/25
 ** ����޸�����:2001/08/31
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iGrpid-Ŀ����   iMbid-Ŀ������   iPrior-���ȼ�
             iType-����Type  iClass-����Class aMsgbuf-����Buf
             iMsglen-���ĳ��� 
 ** ����ֵ: SUCCESS
***************************************************************/
/***************************************************************
ʵ��������Ⱥ�佻�׵Ŀ��鴫��
***************************************************************/
/*int NbPut(short iGrpid,short iMbid,short iPrior,int lType,int lClass,
  char *aMsgbuf,short iMsglen)delete by wh*/
int NbPut(short iGrpid,short iMbid,short iPrior,int lType,int lClass,
  char *aMsgbuf, TYPE_SWITCH iMsglen)/* add by wh*/
{
  char alFilter[iMASKLEN];
  int ilRc;
  
  memset(alFilter,0x00,sizeof(alFilter));
  memcpy(alFilter,(char *)&lType,sizeof(int));
  memcpy(alFilter+sizeof(int),(char *)&lClass,sizeof(int));
  ilRc = _bmqGrpasc_snd(iGrpid,iMbid,iPrior,alFilter,aMsgbuf,iMsglen);
  return(ilRc);
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
static int _bmqGrpcontser(char *alIp,short ilPort,char *aSendIp,long ilTime)
{
  int    sockfd = -1;
  struct sockaddr_in slServ_addr;
  struct sockaddr_in sCli_addr;
  
  int  ilRc,flag;
 
  /* �����׽��� */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    _bmqDebug("S0170 %s : Creat Socket Error! :errno: %d,%s",__FILE__,errno,strerror(errno));
    return (-1050);
  }
  memset((char *)&sCli_addr,0x00,sizeof(struct sockaddr_in));
  _bmqTrim(aSendIp);
  _bmqDebug("S0180 ʹ�� IP[%s]����",aSendIp);
  if (strlen(aSendIp)>0) {
	  sCli_addr.sin_family = AF_INET;
	  sCli_addr.sin_addr.s_addr = inet_addr(aSendIp);
	  
	  if (bind(sockfd,(struct sockaddr*)&sCli_addr,sizeof(struct sockaddr_in)) < 0)
	  {
	    _bmqDebug("bind IP[%s] socket����[%d]\n",aSendIp,errno);
	    return(-1050);
	  }
  }
  /*add by pc 20060329 ������·��־λ
  ilRc=setsockopt(sockfd,IPPROTO_IP,IP_TOS,(char*)&igLineNum,sizeof(igLineNum));
  if(ilRc<0)
  {
    _bmqDebug("S0171: setsockopt IPPROTO_IP,IP_TOS [%d ]error",igLineNum);
    close(sockfd);
    return (-1060);
  }
  add by pc 20060329 ������·��־λ*/  
  memset((char *)&slServ_addr, 0, sizeof(struct sockaddr_in));
  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( alIp );
  slServ_addr.sin_port = htons( ilPort );

  flag =fcntl( sockfd, F_GETFL, 0 );
  if ( flag<0 ){
    _bmqDebug("S0171: fcntl1 error");
    close(sockfd);
    return (-1060);
  }
  if ( fcntl( sockfd, F_SETFL, flag|O_NONBLOCK )<0 ){
    _bmqDebug("S0172: fcntl2 error");
    close(sockfd);
    return (-1060);
  }

  /*����ͨѶ�˿�*/
  ilRc = connect(sockfd,(struct sockaddr *)&slServ_addr,sizeof(slServ_addr));  
  if(ilRc < 0)
  {
    if(errno!=EINPROGRESS && errno !=0)
    {
      _bmqDebug("S0180 %s: Connect Server[%s][%d] Error! errno: %d,%s",__FILE__,alIp,ilPort,errno,strerror(errno));
      close(sockfd);
      return (-1060);
    }
  }
  if(ilRc != 0)
  {
    ilRc = _bmqTimeOut ( sockfd , ilTime );
    if(  ilRc != 0 )
    {
      _bmqDebug("S0181 %s: Connect Server[%s][%d] Error! errno: %d,%s",__FILE__,alIp,ilPort,errno,strerror(errno));
      _bmqDebug("S0182:ilRc=[%d][%ld]",ilRc,ilTime);
      close(sockfd);
      return(-1060);
    }
  }
  if ( fcntl( sockfd, F_SETFL, flag )<0 ){
     _bmqDebug("S0173: fcntl3 error");
     close(sockfd);
     return (-1060);
  }
 
  return sockfd;
}

/*20060220 �������Ӳ���*/
/*���÷����IP*/
int NbSetIp(char *ip)
{
	memset(agIp,0x00,sizeof(agIp));
	strcpy(agIp,ip);
	return 0;
}
/*���ñ���IP*/
int _NbSetSendIp(char *ip)
{
	memset(agSendIp,0x00,sizeof(agSendIp));
	strcpy(agSendIp,ip);
	return 0;
}
int NbSetPort(int port)
{
	igPort=port;
	return 0;
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
 short           *length;delete by wh*/
static int _bmqGrpSend(sockfd, buffer, length)
 int             sockfd;
 char   *buffer;
 TYPE_SWITCH *length; /*add by wh*/
{
  /*short   len=-1;delete by wh*/
  TYPE_SWITCH len=-1;/*add by wh*/
  /*short   ilSendlen=0; delete by wh*/
  TYPE_SWITCH ilSendlen=0;/*add by wh*/

 _bmqDebug("S0111: _bmqGrpSend send:%s,length[%d]len[%d]",buffer,*length,len); 
 _bmqDebughex(buffer,*length);


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
   _bmqDebug("S01 ztq: _bmqGrpSend ilSendlen:%d",ilSendlen); 
  return ( SUCCESS );
}

