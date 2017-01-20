/****************************************************************/
/* 模块编号	：BMQAPI						*/
/* 模块名称	：新晨Q API库					*/
/* 版 本 号	：V2.0						*/
/* 作    者	：史正烨						*/
/* 建立日期	：2000/10/26					*/
/* 最后修改日期	：2002/07/26					*/
/* 模块用途	：新晨Q服务器端API函数				*/
/* 本模块中包含如下函数及功能说明：				*/
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
/* 修改记录：(Maintenance by XuJun)				*/
/* 2001.11.30 封版V2.0,程序共5054行				*/
/* 2002.07.26 增加发送统计功能,程序共5103行			*/
/****************************************************************/

/* 库函数定义 */
#include	"bmq.h"

/* 变量定义 */
static	int	igFlag;				/* 标志参数 */
static	short	igMbinfo;			/* 邮箱数目 */
static	long	lgMbqueuecount;			/* 邮箱队列数目 */
static	long	lgMbpacksize;			/* 对列存储区大小 */
static	short	igMbcontcount;			/* 最大邮箱连接数目 */
static	short	igMbfilecount;			/* 最大同时发送文件数 */
static	long	lgLogfilesize;			/* 日志文件大小 */
static	char	*agShmtmp_d;			/* 共享内存地址 */
static	int   igMb_sys_shm_id;		/* 当前共享内存id */

#define DEFAULTCONTIME 10 	/*缺省连接超时时间*/

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


/* 函数申明 */
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
 ** 函数名:_bmqShowversion
 ** 功能: 显示版本号
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义    :
 ** 返回值: SUCCESS
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
 ** 函数名:_bmqStartup
 ** 功能: 启动新晨Q
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:_bmqConfig_load,_bmqUnlinkfile
 ** 全局变量:
 ** 参数含义    :
 ** 返回值: 成功：SUCCESS 错误：错误码
***************************************************************/
int _bmqStartup()
{
  int   i,ilRc;
  long  llSize;
  short ilArray[iSEMNUM];
  char  *alShmtmp_d; 
  char  alFp_name[80];
  char  alPro_name[20];

  /*读入参数配置文件*/
  ilRc = _bmqConfig_load("BMQ",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0010 %s :载入BMQ参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1184);
  }
  ilRc = _bmqConfig_load("SHMK",0); 
  if (ilRc < 0)
  {
    _bmqDebug("S0020 %s :载入SHMK参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1185);
  }
 
  /*计算共享内存大小*/
  llSize=sizeof(struct mbshm)+sizeof(struct mbinfo)*igMbinfo \
    +sizeof(struct mbqueue)*lgMbqueuecount \
    +sizeof(struct mbpack)*lgMbpacksize \
    +sizeof(struct mbconnect)*igMbcontcount \
    +sizeof(struct monmsg)*igMbfilecount \
    +sizeof(struct mbrouter)*iMAXROUTEREC;

   /*创建共享内存*/
  ilRc = shmget(lgSHMKEY_MB,llSize,IPC_CREAT|IPC_EXCL|0600);
  if (ilRc < 0)
  {
    if(errno == EEXIST) return(1000);
    _bmqDebug("S0030 创建共享内存出错,errno:%d:%s",errno,strerror(errno));
    return(-1150);
  }
  igMb_sys_shm_id = ilRc;


  /*取得共享内存地址*/
  alShmtmp_d = shmat(igMb_sys_shm_id,NULL,0);/*NULL--系统推荐*/
  if (alShmtmp_d == (char *)-1)
  {
    _bmqDebug("S0040 取得共享内存地址出错shmat errno:%d:%s",errno,strerror(errno));
    ilRc = shmctl(igMb_sys_shm_id,IPC_RMID,0);
    if(ilRc < 0)
    {
      _bmqDebug("S0050 删除共享内存出错 shmctl(RM) errno:%d:%s",errno,strerror(errno));
      return(-1153);
    }
    return(-1151);
  }

  /*初始化共享内存结构*/
  memset(alShmtmp_d,0x00,llSize);
  psgMbshm = (struct mbshm *)alShmtmp_d;  
  psgMbshm->iMbinfo          = igMbinfo;
  psgMbshm->lMbqueuecount    = lgMbqueuecount;
  psgMbshm->lMbpacksize      = lgMbpacksize;
  psgMbshm->iMbcontcount     = igMbcontcount;
  psgMbshm->iFilecount       = igMbfilecount;
  psgMbshm->lLogfilesize     = lgLogfilesize;
  psgMbshm->iRouternum       = 0;
 
  /*计算各区偏移*/
  psgMbshm->lMbqueueoffset   =  sizeof(struct mbinfo)*psgMbshm->iMbinfo;
  psgMbshm->lMbpackoffset    =  psgMbshm->lMbqueueoffset
    + sizeof(struct mbqueue)*psgMbshm->lMbqueuecount;
  psgMbshm->lMbconnectoffset =  psgMbshm->lMbpackoffset 
    + sizeof(struct mbpack)*psgMbshm->lMbpacksize;
  psgMbshm->lFileoffset      =  psgMbshm->lMbconnectoffset 
    + sizeof(struct mbconnect)*psgMbshm->iMbcontcount;
  psgMbshm->lMbrouteroffset  =  psgMbshm->lFileoffset 
    + sizeof(struct monmsg)*psgMbshm->iFilecount;

  /*建立共享内存的内容区*/
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

  /* 导入定义的路由配置信息(可以不存在配置文件和信息，故不对该项目进行错误判断) */
  _bmqLoadRouterinfo();
  /*
  while(1)
  {
    ilRc = _bmqConfig_load("CLINF",-1);
    if (ilRc < 0)
    {
      _bmqDebug("S0010 %s :载入CLINF参数失败，请查看参数文件%s/etc/bmq.ini",
        __FILE__,getenv("BMQ_PATH"));
      return(-1000);
    }
    if (ilRc == FINISH) break;
  }
  del by xujun 2006.12.28
  不使用通道管理器*/

   /*载入邮箱驻留时间和阀值参数*/
  ilRc = _bmqConfig_load("MAIL",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0060 %s :载入MAIL参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    _bmqQuit();
    return(-1183);
  }

  /*创建组间通讯消息队列*/
  psgMbshm->lMsgid_grp = msgget(IPC_PRIVATE,IPC_CREAT|0600);
  if (psgMbshm->lMsgid_grp < 0)
  {
    _bmqDebug("S0080 创建组间通讯消息队列错误 msgget errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1160);
  }
  psgMbshm->lMsgid_grp_rcv = 0;
  /*next 9 rows add by nh
    创建C/S间通讯消息队列
  psgMbshm->lMsgid_cls = msgget(IPC_PRIVATE,IPC_CREAT|0600);
  if (psgMbshm->lMsgid_cls < 0)
  {
    _bmqDebug("S0080 创建C/S间通讯消息队列错误 msgget errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1000);
  }
  del by xujun 2006.12.28
  不使用通道管理器*/

  /*创建信号灯并初始化*/
  psgMbshm->lSemid = semget(IPC_PRIVATE,iSEMNUM,IPC_CREAT|0600);
  if ( psgMbshm->lSemid == -1 )
  {
    _bmqDebug("S0090 创建信号灯错误semget errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1170);
  }

  for (i=1;i<=iSEMNUM;i++) ilArray[i-1]=1; /*iSEMNUM=2 信号量*/
  ilRc = semctl(psgMbshm->lSemid,iSEMNUM,SETALL,ilArray);
  if (ilRc==-1)
  {
    _bmqDebug("S0100 信号灯初始化错误semctl SETALL errno:%d:%s",errno,strerror(errno));
    _bmqQuit();
    return(-1170); 
  }
   /*初始化唯一消息ID*/
  psgMbshm->lgMb_sys_queue_serial_num = 0;
  igMbopenflag = 0;

  /*取出BMQ_GROUP*/
  if ((psgMbshm->iMbgrpid = atoi(getenv("BMQ_GROUP_ID")) ) <= 0)
  {
    _bmqDebug("S0110 系统环境变量[BMQ_GROUP_ID]没有设置或设置值小于零!");
    _bmqQuit();
    return(-1190);
  }
  /*取出本组的IP地址和端口号*/
  ilRc = _bmqConfig_load("GROUP",psgMbshm->iMbgrpid);
  if (ilRc < 0)
  {
    _bmqDebug("S0040 %s :载入GROUP[%d]参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,psgMbshm->iMbgrpid,getenv("BMQ_PATH"));
    return(-1190);
  }
  
  /*清空溢出缓存文件*/
  ilRc = _bmqUnlinkfile();
  if(ilRc == FAIL)
  {
    _bmqDebug("S0120 清空溢出缓存文件错误,请查看系统环境变量BMQ_PATH设置");
    _bmqQuit();
    return(-1191);
  }
  
  /*启动服务器守护进程、邮箱状态监控进程和跨组守护进程*/
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
  不使用通道管理器*/
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
    增加4个文件发送守护进程*/
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
 ** 函数名:_bmqShutdown
 ** 功能: 关闭新晨Q
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 * 调用其它函数:_bmqConfig_load;_bmqProshut
 ** 全局变量:
 ** 参数含义    :
 ** 返回值: 成功：SUCCESS 错误：错误码
***************************************************************/
int _bmqShutdown()
{
  int  ilRc,i;
  FILE   *fp;
  short  ilConn_head,ilConn_head_tmp;
  struct mbconnect slMbconnect;

  /*连接共享内存*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0130 连接共享内存区出错:%d",ilRc);
    return(ilRc);
  }

  /*关闭守护进程*/
  _bmqProshut();
  
  /*保存内存中的数据*/
  
  /*删除消息队列，信号灯，共享内存*/
  for( i=0; i<psgMbshm->iMbinfo; i++)
  {
    if(psgMbinfo[i].iConnect == 0) continue;
    ilConn_head = psgMbinfo[i].iConn_head; 
 
    while(ilConn_head)
    { 
      /*记录在文件中*/
      if(ilConn_head > psgMbshm->iMbcontcount)
      {
        /*打开连接缓存文件*/
        if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
        {
          _bmqDebug("S0120 %s:打开连接缓存文件出错",__FILE__);
          break;
        }
        /*取出指定记录*/
        ilRc = _bmqConnfile_pick(fp,&slMbconnect,ilConn_head);
        ilConn_head_tmp = slMbconnect.iRlink;
        /*判断进程是否存在*/
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
    _bmqDebug("S0140 新晨Q Shutdown, _bmqQuit() 发生错误!");
    return(ilRc);
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名:bmqOpen
 ** 功能: 打开指定邮箱
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:_bmqConfig_load,_bmqConn_judge,_bmqConnfile_put
 ** 全局变量:
 ** 参数含义    :iMbid--邮箱号
 ** 返回值: 成功：SUCCESS 错误：错误码
***************************************************************/
int bmqOpen(short iMbid)
{
  int    ilRc;
  int    ilNum;
  long   llMsgid;

  /*读入参数配置文件DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0150 %s :载入DEBUG参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1187);
  }

  if(igDebug >=2)
  {
    _bmqDebug("S0160 ****调用函数bmqOpen(%d)****",iMbid);
  }

  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0170 连接共享内存区出错:%d",ilRc);
    return(ilRc);
  }
  
  /* iMbid 是否超出范围 */
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) ) 
  {
    _bmqDebug("S0180 邮箱[%d]超出范围 1~%d",iMbid,psgMbshm->iMbinfo);
    return(1004);
  }
  /* next 5 rows add by nh 判断是否本地邮箱 */
  if( psgMbinfo[iMbid - 1].iFlag  == 0 )
  {
    _bmqDebug("S0180 邮箱[%d]为非本地邮箱",iMbid);
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

  /*判断该进程是否已经申请过连接*/
  ilRc = _bmqConn_judge(getpid());
  if(ilRc == FAIL)
  {
    _bmqDebug("S0190 该进程已经申请过邮箱[%d]连接",iMbid);
    _bmqUnlock(LK_ALL);
    return(1007);
  }

  /*置当前邮箱*/
  igMb_sys_current_mailbox = iMbid;
  igMb_sys_current_group   = psgMbshm->iMbgrpid;
  if(igDebug >=3)
  {
    _bmqDebug("S0200 置当前邮箱为[%d]",iMbid);  
  } 

  /*创建消息队列*/
  llMsgid = msgget(IPC_PRIVATE,IPC_CREAT|0600);
  if (llMsgid < 0)
  {
    _bmqDebug("S0070 创建消息队列错误 msgget errno:%d:%s",errno,strerror(errno));
    _bmqUnlock(LK_ALL);
    return(-1160);
  }

  /*在邮箱连接区中增加一条记录*/
  ilRc = _bmqConnect_add(iMbid,llMsgid);
  if(ilRc == FAIL)
  {
    _bmqDebug("S0210 在邮箱连接区中增加一条记录失败!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  /*设置邮箱打开标志*/
  igMbopenflag  = 1;
  lgCurrent_pid = getpid();
  _bmqUnlock(LK_ALL);

  if(igDebug >= 2)
  {
    _bmqDebug("S0220 ****结束调用函数bmqOpen(%d)****",iMbid); 
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名:bmqClose
 ** 功能: 关闭当前邮箱
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:_bmqConnect_del,_bmqConnfile_pick
 ** 全局变量:
 ** 参数含义    :
 ** 返回值: 成功：SUCCESS 错误：FAIL
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
    _bmqDebug("S0230 ***调用函数bmqClose()***");    
  }

  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGetw() 没有打开邮箱就执行操作!!");
      return(1011);
    }
  
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /* 取出当前邮箱和连接*/
  ilSize = sizeof(struct mbconnect) + 1;
  i      = igMb_sys_current_mailbox - 1;
  j      = igMb_sys_current_connect - 1;

  /*当前连接在共享内存中*/
  if( igMb_sys_current_connect <= psgMbshm->iMbcontcount )
  {
    /*删除当前连接节点*/
    ilRc = _bmqConnect_del(i,&psgMbconnect[j]);
    if(ilRc ==  FAIL) 
    {
      _bmqDebug("S0240 删除当前连接节点错误_bmqConnect_del()"); 
      _bmqUnlock(LK_ALL);
      return(FAIL); 
    }

    if(igDebug >= 2)
    {
      _bmqDebug("S0250 关闭邮箱后,邮箱[%d]连接ID:head=%d,tail=%d",i+1,
         psgMbinfo[i].iConn_head,
           psgMbinfo[i].iConn_tail);
    }
  }
  /*当前连接在缓存文件中*/
  else
  {
    /*取出记录*/
    if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
    {
      _bmqDebug("S0260 打开连接缓存文件出错");
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
    _bmqConnfile_pick(fp,&slMbconnect,igMb_sys_current_connect);
 
    /*删除当前连接节点*/
    ilRc = _bmqConnect_del(i,&slMbconnect);
    if(ilRc ==  FAIL)
    {
       _bmqDebug("S0270 删除当前连接节点 _bmqConnect_del()出错"); 
       _bmqUnlock(LK_ALL);
       _bmqClosefile(fp);
       return(FAIL);
    }
   
    /*重置缓存文件记录*/
    alBuf[0] = '0';  
    fseek(fp,0,SEEK_SET);
    ilRc = _bmqPutfile(fp, (igMb_sys_current_connect-psgMbshm->iMbcontcount-1)*ilSize, ilSize,alBuf);
    if (ilRc)
    {
      _bmqDebug("S0280 重置连接缓存文件记录写文件出错");
      _bmqClosefile(fp);
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
 
    _bmqClosefile(fp);
  }

  igMbopenflag  = 0;
  lgCurrent_pid = 0;
  _bmqUnlock(LK_ALL);

  /*断开共享内存*/
  ilRc = shmdt ( (void *)agShmtmp_d);
  if ( ilRc ) 
  {
    _bmqDebug("S0070 断开共享内存出错！ msgget errno:%d:%s",errno,strerror(errno));
    return(-1152);
  }
  psgMbshm = (struct mbshm *)NULL;

  if(sgRoot) _bmqFreeVirusRoot(sgRoot);

  if(igDebug >= 2)
  {
    _bmqDebug("S0290 ***结束调用函数bmqClose()***");    
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名:  bmqClearmb
 ** 功  能:  清理邮箱
 ** 作  者:  徐军
 ** 建立日期:    2001.08.08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: iMbid--指定清理邮箱号 0表示清理所有邮箱
 ** 返回值:  SUCCESS , FAIL
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
    _bmqDebug("S0300 ****调用函数bmqClearmb(%d)****",iMbid);    
  }

  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqClearmb() 没有打开邮箱就执行操作!!");
      return(1011);
    }

  if ((iMbid > psgMbshm->iMbinfo) || (iMbid < 0))
  {
    _bmqDebug("S0310 邮箱号错误,其范围应为1到%d\n",psgMbshm->iMbinfo);
    return(1004);
  }
  /* next 5 rows add by nh 判断是否本地邮箱
     next 1 rows modi by xujun iMbid=0不做判断 */
  if( iMbid && ( psgMbinfo[iMbid - 1].iFlag  == 0 ) )
  {
    _bmqDebug("S0180 邮箱[%d]为非本地邮箱",iMbid);
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
          _bmqDebug("S0320 打开队列缓存文件_bmqOpenfile()出错");
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        ilRc = _bmqQueuefile_pick(fp, &slMbqueue, llLink);
        if (ilRc == FAIL) 
        { 
          _bmqDebug("S0330 队列缓存文件中取记录_bmqQueuefile_pick()失败!");
          _bmqUnlock(LK_ALL);
          _bmqClosefile(fp);
          return(FAIL);
        }
        if(slMbqueue.lPack_head > 0)
        {
          ilRc = _bmqPack_get(alBuf,&ilMsglen,slMbqueue.lPack_head);
          if (ilRc == FAIL) 
          { 
            _bmqDebug("S0340 取报文 _bmqPack_get()失败!");
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
            _bmqDebug("S0350 取报文_bmqPack_get()失败!");
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
          _bmqDebug("S0360 打开文件出错");
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        ilRc = _bmqQueuefile_pick(fp, &slMbqueue, llLink);
        if (ilRc == FAIL) 
        {
          _bmqDebug("S0370 队列缓存文件中取记录_bmqQueuefile_pick()失败!");      
          _bmqClosefile(fp);
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        ilRc = _bmqQueuefile_set(fp, 0, 0, 0, llLink);
        if (ilRc == FAIL) 
        {
          _bmqDebug("S0380 队列缓存文件记录设置_bmqQueuefile_set()失败!");      
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
      _bmqDebug("S0390 ****结束调用函数bmqClearmb(%d)****",iMbid);    
    }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名:  bmqGetmbinfo
 ** 功  能:  获取指定邮箱信息
 ** 作  者:  徐军
 ** 建立日期:    2001.08.08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: iMbid--邮箱号 plMbinfo--邮箱结构指针
 ** 返回值:  SUCCESS , FAIL
***************************************************************/
int bmqGetmbinfo(short iMbid,struct mbinfo *plMbinfo)
{
  if (igDebug >= 2)
  {
    _bmqDebug("S0400 ****调用函数bmqGetmbinfo(%d,struct mbinfo *plMbinfo)****",
      iMbid);    
  }

  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGetmbinfo() 没有打开邮箱就执行操作!!");
      return(1011);
    }


  /* iMbid 是否超出范围 */
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) )
  { 
    return(OVERFIELD);
  }  

  /* next 5 rows add by nh 判断是否本地邮箱*/
  if( psgMbinfo[iMbid - 1].iFlag  == 0 )
  {
    _bmqDebug("S0180 邮箱[%d]为非本地邮箱",iMbid);
    return(1013);
  }

  memcpy(plMbinfo,&psgMbinfo[iMbid-1],sizeof(struct mbinfo));
 
  if (igDebug >= 2)
  {
    _bmqDebug("S0410 ****结束调用函数bmqGetmbinfo()****");    
  }

  if( psgMbinfo[iMbid-1].iConnect == 0 ) 
    return(SUCCESS);
  else
    return 1;
}

/**************************************************************
 ** 函数名:bmqGet
 ** 功能:  提取报文(非阻塞)
 ** 作者:  史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:_bmqQueue_get,_bmqPack_get
 ** 全局变量:
 ** 参数含义:piGrpid-源发组   piMbid-源发邮箱    piPrior-优先级
             piType-报文Type  piClass-报文Class  aMsgbuf-报文Buf
             piMsglen-报文长度
 ** 返回值: SUCCESS,错误码
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
 ** 函数名:bmqGetfilter
 ** 功能:  提取报文(非阻塞)
 ** 作者:  史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:_bmqQueue_get,_bmqPack_get
 ** 全局变量:
 ** 参数含义:piGrpid-源发组       piMbid-源发邮箱  piPrior-优先级
             aFilter-报文过滤条件 aMask-报文掩码   aMsgbuf-报文Buf
             piMsglen-报文长度
 ** 返回值: SUCCESS,错误码
***************************************************************/
/*int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,short *piMsglen)delete by wh*/
int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen)/*add by wh*/
{
  int  ilRc;
  long llPack_head;

  if(igDebug >= 2)
  {
    _bmqDebug("S0420 ***调用函数:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }

  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGet() 没有打开邮箱就执行操作!!");
      return(1011);
    }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*查找邮箱队列记录,取出报文链接头*/
  ilRc = _bmqQueue_get(piGrpid,piMbid,piPrior,aFilter,aMask,&llPack_head);
  if (ilRc)
  {
    if(ilRc != 1001)  	
      _bmqDebug("S0430 取出报文链接头出错!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }
 
  /*修改邮箱信息*/
  psgMbinfo[igMb_sys_current_mailbox-1].lPendnum--;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime));

  /*提取报文*/
  ilRc = _bmqPack_get(aMsgbuf,piMsglen,llPack_head);
  if( ilRc )
  {
    _bmqDebug("S0440 提取报文ERROR!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  _bmqUnlock(LK_ALL);
  if(igDebug >= 1)
  {
    _bmqDebug("S0450 邮箱[%d]取走组[%d]邮箱[%d]的[%d]报文",
      igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
  }


  if(igDebug >= 3)
  {
    _bmqDebug("S0460 邮箱[%d]修改后信息lPendnum[%d],lRecvpid[%d],lRecvtime[%d]",
      igMb_sys_current_mailbox,
      psgMbinfo[igMb_sys_current_mailbox-1].lPendnum,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime);

  }

  if(igDebug >= 2)
  {
    _bmqDebug("S0470 ***结束调用函数:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S0480 ***函数bmqGet取出的报文内容aMsgbuf为***");
    _bmqDebug("S0490 %s",aMsgbuf); 
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名:bmqGetw
 ** 功能:  提取报文(阻塞)
 ** 作者:  史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:_bmqQueue_get,_bmqPack_get
 ** 全局变量:
 ** 参数含义:piGrpid-源发组   piMbid-源发邮箱   piPrior-优先级
             piType-报文Type  piClass-报文Class aMsgbuf-报文Buf
             piMsglen-报文长度   iTimeout-阻塞读间隔时间
 ** 返回值: SUCCESS,错误码
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
 ** 函数名:bmqGetw
 ** 功能:  提取报文(阻塞)
 ** 作者:  史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:_bmqQueue_get,_bmqPack_get
 ** 全局变量:
 ** 参数含义:piGrpid-源发组        piMbid-源发邮箱  piPrior-优先级
             aFilter-报文过滤条件  aMask-报文掩码   aMsgbuf-报文Buf
             piMsglen-报文长度   iTimeout-阻塞读间隔时间
 ** 返回值: SUCCESS,错误码
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
    _bmqDebug("S0500 ***调用函数:bmqGetw(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }

  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() ) 
    {
      _bmqDebug("S0510 bmqGetw() 没有打开邮箱就执行操作!!"); 
      return(1011);
    }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*以非阻塞方式读取一次,读到报文则返回*/
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
      _bmqDebug("S0508 提取报文错误!");
      _bmqUnlock(LK_ALL);
      return(FAIL);
    }

    _bmqUnlock(LK_ALL);

    if(igDebug >= 1)
    {
      _bmqDebug("S0510 邮箱[%d]调用函数:bmqGetw()以非阻塞方式取走组[%d]邮箱[%d][%d]报文",igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
    }
    if(igDebug >= 2)
    {
      _bmqDebug("S0520 ***结束以非阻塞方式调用函数:bmqGetw(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
        *piGrpid,*piMbid,*piPrior,*piMsglen);
    }
    if(igDebug >= 3)
    {
      _bmqDebug("S0530 *非阻塞方式调用函数bmqGetw()取出的报文内容aMsgbuf为*");
      _bmqDebug("S0540 %s",aMsgbuf); 
    }

    return(SUCCESS);
  }


  j = igMb_sys_current_connect - 1;

  /*设置阻塞读条件 cTag==2正在阻塞读*/
  ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
  if(ilRc == FAIL)
  {
    _bmqDebug("S0550 设置阻塞读条件出错ERROR!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  llMsgid = _bmqGetmsgid(igMb_sys_current_connect);
  if(llMsgid == FAIL)
  {
    _bmqDebug("S0550 取消息队列ID错误!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  _bmqUnlock(LK_ALL);

Tagmsgrcv:
  if(igDebug >= 2)
    _bmqDebug("S0560 从消息队列(收)中阻塞读:mtype=%d",j+1);

  /*设置阻塞读消息时间*/
  if (iTimeout != 0)
  {
    signal(SIGALRM,_bmqTimeout);
    alarm(iTimeout);
  }
  igFlag = 0;

  /*在组内消息队列上阻塞读取消息*/
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
      _bmqDebug("S0570 在组内消息队列上阻塞读取消息出错 msgrcv() errno:%d:%s",errno,strerror(errno)); 
      goto Tagmsgrcv;
    }
    _bmqDebug("S0580 在组内消息队列上阻塞读取消息出错 msgrcv(),errno:%d:%s",errno,strerror(errno)); 
    _bmqUnlock(LK_ALL);  
    return(-1140);
  }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  j = igMb_sys_current_connect-1;

  if(igDebug >= 2)
  {
    _bmqDebug("S0590 从消息队列(收)中读出消息:lQueueid=%d,lSerialnum=%ld",
      slRecvmsg.lQueueid,slRecvmsg.lSerialnum);
  }

  i = slRecvmsg.lQueueid - 1;

  for (;;)
  {
    /*判断消息合法性*/
    if (i >= psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S0600 打开文件出错_bmqOpenfile()");
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
      _bmqDebug("S0610 读出消息为错误消息,消息作废:mb=%d mbqueue.cTag=%d,mbqueue.lSerialnum=%ld,消息包中lSerialnum=%ld",i,
        psgMbqueue[i].cTag,psgMbqueue[i].lSerialnum, slRecvmsg.lSerialnum);
      _bmqDebug("S0620 从消息队列(收)中非阻塞读:mtype=%d",j+1);
    }

    /*读出消息为错误消息,消息作废从消息队列(收)中非阻塞读*/
    ilRc = msgrcv(llMsgid,&slRecvmsg,sizeof(struct recvmsg),
      0,IPC_NOWAIT);
    if (ilRc < 0) 
    {
      if (errno == ENOMSG)
      {
        if(igDebug >= 2)
          _bmqDebug("S0630 将连接[%d]的cTag置为有条件读状态",j+1);

        /*设置阻塞读条件 cTag==2正在阻塞读*/
        ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
        if(ilRc == FAIL)
        {
          _bmqDebug("S0640 设置阻塞读条件ERROR!");
          _bmqUnlock(LK_ALL);
          return(FAIL);
        }
        _bmqUnlock(LK_ALL);

        goto Tagmsgrcv;
      }
      _bmqDebug("S0650 从消息队列(收)中非阻塞读出错 msgrcv errno:%d:%s",errno,strerror(errno)); 
      _bmqUnlock(LK_ALL);
      return(-1140);
    }
  }

  /*读到合法消息，重置连接记录*/
  if(igMb_sys_current_connect > psgMbshm->iMbcontcount)
  {
    ilRc = _bmqConnfile_set(1, -1, -1, aFilter, aMask, igMb_sys_current_connect,-1);
    if(ilRc == FAIL)
    {
      _bmqDebug("S0660 _bmqConnfile_set() 返回出错");
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
  }
  else
    psgMbconnect[j].cTag = 1;

  /*删除队列中记录*/
  if(slRecvmsg.lQueueid > psgMbshm->lMbqueuecount)
  {
  	
    /* 打开队列缓存文件 */
    ilSize = sizeof(struct mbqueue) + 1;
    if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
    {
      _bmqDebug("S0670 打开文件出错!");
      _bmqUnlock(LK_ALL);
      return (FAIL);
    }
    _bmqQueuefile_pick(fp,&slMbqueue,slRecvmsg.lQueueid); 
    
    ilRc = _bmqQueue_del(igMb_sys_current_mailbox,&slMbqueue, piGrpid, piMbid, 
                         piPrior, aFilter,&llPack_head);
    if(ilRc == WITHOUTUSE)
    {
      _bmqDebug("S0672 删除的队列[%d]没有启用",slRecvmsg.lQueueid);
      /*设置阻塞读条件 cTag==2正在阻塞读*/
      ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
      _bmqUnlock(LK_ALL);
      if(ilRc == FAIL)
      {
        _bmqDebug("S0674 设置阻塞读条件ERROR!");
        return(FAIL);
      }
      goto Tagmsgrcv;
    }

    /* 重写文件 */
    alBuf[0] = '0';
    fseek(fp,0,SEEK_SET);
    ilRc = _bmqPutfile(fp, (slRecvmsg.lQueueid-psgMbshm->lMbqueuecount-1)
      *ilSize, ilSize,alBuf);
    if (ilRc)
    {
      _bmqDebug("S0680 写文件出错");
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
      _bmqDebug("S0682 删除的队列[%d]没有启用",i+1);
      /*设置阻塞读条件 cTag==2正在阻塞读*/
      ilRc = _bmqConnWait_set(igMb_sys_current_connect,aFilter,aMask);
      _bmqUnlock(LK_ALL);
      if(ilRc == FAIL)
      {
        _bmqDebug("S0684 设置阻塞读条件ERROR!");
        return(FAIL);
      }
      goto Tagmsgrcv;
    }
  }

  if(igDebug >= 2)
  {
    _bmqDebug("S0690 从邮件队列[%d]中读出消息:org_group=%d,org_q=%d,pack_head=%ld,prior=%d",
      i+1,*piGrpid,*piMbid,llPack_head,*piPrior);
  }

  psgMbinfo[igMb_sys_current_mailbox-1].lPendnum--;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime));

   /*提取报文*/
  ilRc = _bmqPack_get(aMsgbuf,piMsglen,llPack_head);
  if(ilRc)
  {
    _bmqDebug("S0692 提取报文错误!");
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }

  _bmqUnlock(LK_ALL);

  if(igDebug >= 1)
  {
    _bmqDebug("S0700 邮箱[%d]调用函数:bmqGetw()以阻塞方式取走组[%d]邮箱[%d]的[%d]报文",igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
  }
  if(igDebug >= 2)
  {
    _bmqDebug("S0710 ***结束调用函数:bmqGetw(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S0720 ***函数bmqGetw()取出的报文内容aMsgbuf为***");
    _bmqDebug("S0730 %s",aMsgbuf); 
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名:bmqPut
 ** 功能:  存放报文
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iGrpid-目标组   iMbid-目标邮箱   iPrior-优先级
             iType-报文Type  iClass-报文Class aMsgbuf-报文Buf
             iMsglen-报文长度 
 ** 返回值: SUCCESS
***************************************************************/
/*int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,
  char *aMsgbuf,short iMsglen)delete by wh*/
int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,
  char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  char alFilter[iMASKLEN];
  int ilRc;
  
  _bmqVdebug(2,"S0735 ***调用函数 bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",iGrpid,iMbid,iPrior,iMsglen); 
  
  lType = htonl(lType);
  lClass = htonl(lClass);
  
  memset(alFilter,0x00,sizeof(alFilter));
  memcpy(alFilter,(char *)&lType,sizeof(long));
  memcpy(alFilter+sizeof(long),(char *)&lClass,sizeof(long));
  ilRc = bmqPutfilter(iGrpid,iMbid,iPrior,alFilter,aMsgbuf,iMsglen);
  return(ilRc);
}
/********************************************************************
 ** 函数名:bmqPutfilter
 ** 功能:  存放报文
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iGrpid-目标组        iMbid-目标邮箱  iPrior-优先级
             aFilter-报文过滤条件 aMsgbuf-报文Buf iMsglen-报文长度 
 ** 返回值: SUCCESS
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
    _bmqDebug("S0740 ***调用函数 bmqPutfilter(%d,%d,%d,aFilter,aMsgbuf,%d)***",
      iGrpid,iMbid,iPrior,iMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    
  }

  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() ) 
  {
    _bmqDebug("S0510 bmqPut() 没有打开邮箱就执行操作!!");
    return(1011);
  }
 
  /*判断参数合法性*/
  if( iGrpid < 0) return(1009);
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) ) return(1004);
  if( (iPrior < 0) || (iPrior > 99) ) return(1010);

   /*判断是否已经超过指定阀值*/
  if( psgMbinfo[iMbid-1].lMaxqueue && psgMbinfo[iMbid-1].lPendnum >= 
    psgMbinfo[iMbid-1].lMaxqueue )
  {
    return(1008);
  }
  
  /*判断报文长度是否超过最大值*/
  if(iMsglen > iMBMAXPACKSIZE)
  {
    _bmqDebug("S0750 报文长度[%d]超过最大值－ [%d]",iMsglen,iMBMAXPACKSIZE);
    return(1012);
  }
  
  if( iGrpid == 0 ) iGrpid = psgMbshm->iMbgrpid;
 
  	
  if(psgMbshm->iMbgrpid != iGrpid)
  {
    ilRc = _bmqConfig_load("GROUP",iGrpid);
    if (ilRc < 0)
    {  
      _bmqDebug("0751 :载入GROUP[%d]参数失败",iGrpid);
      return(FAIL);
    }
  }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*本组或者长连接跨组时存储报文*/
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
      _bmqDebug("S0760 函数bmqPut()存入的报文内容aMsgbuf为:",aMsgbuf);
      _bmqDebughex(aMsgbuf,iMsglen);
    }
  }

  /*如果跨组，守护进程发送触发消息*/
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
    
    /*目标组使用短连接 */
    if( agCommode[0] == 'S' ) 
    {
      ilRc = _bmqGrpasc_snd(iGrpid,iMbid,iPrior,aFilter,aMsgbuf,iMsglen);
      return(ilRc);      
    }
    /*目标组使用长连接*/
    ilRc = _bmqGrpmsg_snd(llPack_head,iPrior,aFilter,iGrpid,iMbid);
    if(ilRc) return(ilRc);

    if(igDebug >= 1)
    {
      _bmqDebug("S0770 邮箱[%d]往组[%d]邮箱[%d]存入[%d]报文",
        igMb_sys_current_mailbox,iGrpid,iMbid,llPack_head); 
    }
    if( igDebug >= 2)
    {
      _bmqDebug("S0780 ***结束调用函数 bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",
        iGrpid,iMbid,iPrior,iMsglen);
    }

    return(SUCCESS);
  }
  /*非本地邮箱发送到客户端
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
      _bmqDebug("S0771 邮箱[%d]往组[%d]邮箱[%d]存入[%d]报文",
        igMb_sys_current_mailbox,iGrpid,iMbid,llPack_head);
    }
    if( igDebug >= 2)
    {
      _bmqDebug("S0781 ***结束调用函数 bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",
        iGrpid,iMbid,iPrior,iMsglen);
    }

    return(SUCCESS);
  }
  del by xujun
  不使用通道管理器*/

  /*存储队列*/
  ilRc = _bmqQueue_put( iGrpid, iMbid, iPrior, aFilter,llPack_head,&llQueue_id);
  if (ilRc)
  {
    _bmqDebug("S0790 存储队列_bmqQueue_put(...)出错!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  /*修改邮箱信息*/
  psgMbinfo[iMbid-1].lPendnum++;

  /*if ( igMb_sys_current_group == psgMbshm->iMbgrpid )*/
  psgMbinfo[igMb_sys_current_mailbox-1].lSendnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lSendpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lSendtime));
  
  if(psgMbshm->iStatflag)
  {
    _bmqSendstat(iMbid);
  }

  /*向阻塞读的进程发送通知消息*/
  ilRc = _bmqConn_select(iMbid, iPrior, aFilter, llQueue_id);
  _bmqUnlock(LK_ALL);

  if(igDebug >= 1)
  {
    _bmqDebug("S0800 邮箱[%d]往组[%d]邮箱[%d]存入[%d]报文",
      igMb_sys_current_mailbox,iGrpid,iMbid,llPack_head); 
  }
  if( igDebug >= 2)
  {
    _bmqDebug("S0810 ***结束调用函数 bmqPut(%d,%d,%d,aFilter,aMsgbuf,%d)***",
    iGrpid,iMbid,iPrior,iMsglen);
  }
  if(ilRc == FAIL)
  {
    _bmqDebug("S0798 向阻塞读的进程发送通知消息出错!");
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqGrpasc_snd
 ** 功能:   发送跨组报文(短连接方式)
 ** 作者:   徐军
 ** 建立日期: 2007/01/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
/*int _bmqGrpasc_snd(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen)delete by wh*/
int _bmqGrpasc_snd(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    z;			/* zcb 需要多次建链*/
  
  int	 ilRc,ilLentmp;
  int    ilSockfd;                 /*套接字*/
  /*short  ilMsglen_grp,ilLen;    delete by wh 报文长度*/
  TYPE_SWITCH ilMsglen_grp;/*ilLen;*/    /*报文长度 add by wh*/
  char   alVal[11];
  char   alMsgbuf[iMBMAXPACKSIZE+100];       /*报文存储数组*/
  char   alMsgbuf_grp[iMBMAXPACKSIZE+100];   /*报文发送数组*/
  
  /*char   alTmpbuf[iMBMAXPACKSIZE+100];	add by pc 临时buf*/
  
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
  /*连接到服务器*/
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
      _bmqDebug("S0120 _bmqGrpasc_snd()连接到服务器出错!! IP=[%s] Port=[%d] Timeout[%d]",
        agIp,igPort,igTimeOut);
      _bmqDebug("循环建链,次数:%d",z);
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
  
  if( igDebug >= 2) _bmqDebug("S0121 _bmqGrpasc_snd()连接到服务器IP=[%s] Port=[%d]成功",agIp,igPort);

  /*取得SOCKET信息*/
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( ilSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0130 _bmqGrpasc_snd()unable to read socket address ilSockfd [%d] errno[%d]",ilSockfd,errno);
    close(ilSockfd);
    return(FAIL);
  }

  /*设置SOCKET选项*/
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
  sGrpmsg.iOrg_mailbox = igMb_sys_current_mailbox;	/*这个是否是21*/
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

    /*组织报文*/
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

	/*2006-8-21 10:07 增加回执标志*/
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
    /*add by pc 20060311 飞行报文修改4位 总超时时间信息*/
/*add by pc 应该设置发送超时选项*/ 
	if( igDebug >= 2 )
	{
		_bmqDebug("最后发送内容");
		_bmqDebughex(alMsgbuf_grp,ilMsglen_grp);
	}	

    /*发送报文*/
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
	/*2006-8-21 等待假回执*/
	if (igAdmitFlag==1) {
		memset(alMsgbuf,0x00,sizeof(alMsgbuf));
		/*recv超时时间应该设置*/
		rwto.tv_sec = 30;
		rwto.tv_usec= 0;
		
		ilRc = setsockopt(ilSockfd,SOL_SOCKET,SO_RCVTIMEO,&rwto,sizeof(rwto));
		if ( ilRc < 0) {
			close(ilSockfd);
			_bmqDebug("S165 setsockopt SO_RCVTIMEO错误 [%d]",errno);
			return(FAIL);			
		}
		ilRc=recv(ilSockfd,alMsgbuf,sizeof(alMsgbuf),0);
		
		if( igDebug >= 2 ) {
			_bmqDebug("收到完整假回执内容开始...");
			_bmqDebughex(alMsgbuf,ilRc);
			_bmqDebug("收到完整假回执内容结束...");
		}
		if (ilRc>0) {
			if (memcmp(alMsgbuf,"HZ",2)!=0) {
				close(ilSockfd);
				_bmqDebug("S170 收到的假回执内容错误 [%s]",alMsgbuf);
				return(FAIL);			
			}
			else {
				_bmqDebug("S0000 收到假回执成功");
			}	
		}
		else {
			close(ilSockfd);
			_bmqDebug("S0180 没收到假回执[%d]",ilRc);
			return(FAIL);
		}
	}
	/*2006-8-21 等待假回执*/
	
    close(ilSockfd);	/*是否可以不关，用于下次发送*/
    return(SUCCESS);
}

/*int _bmqGrpasc_snd_old(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen)delete by wh*/
int _bmqGrpasc_snd_old(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    ilTrytimes;                         /*尝试连接次数*/
  int    ilRc;                               /*返回值*/
  int    ilSockfd;                           /*套接字*/
  /*short  ilMsglen_grp;                       delete by wh 报文长度*/
  TYPE_SWITCH ilMsglen_grp;                       /*报文长度 add by wh*/
  char   alMacbuf[MAC_LEN];                  /*报文MAC*/
  char   alVal[11];                          /*临时变量*/
  char   alMsgbuf[20];                       /*IP地址BUF*/
  char   alMsgbuf_grp[iMBMAXPACKSIZE+100];   /*报文发送数组*/
  struct grprecvmsg slGrpmsg;                /*跨组消息*/
  struct linger ilLinger;                    /*socket变量*/
  struct sockaddr_in slCli_addr;
#ifdef OS_SCO
  int     ilAddrlen;
#else
  socklen_t    ilAddrlen;
#endif

  _bmqVdebug(2,"S0118 发送跨组报文(短连接方式),grp=[%d],mb=[%d]",iGrpid,iMbid);

  /*连接到目标服务器*/
  for(ilTrytimes=0;ilTrytimes<DEFAULTCONNUM;ilTrytimes++) 
  {
	  ilSockfd = _bmqGrpcontserS(agBmqhostip,igBmqgrpport,lgGroup_try_time);
    if ( ilSockfd < 0 )
    {
      _bmqDebug("S0120 _bmqGrpasc_snd()连接到服务器出错!! IP=[%s] Port=[%d] Timeout[%d]",
        agBmqhostip,igBmqgrpport,lgGroup_try_time);
      _bmqDebug("S0121 循环建链,次数:%d",ilTrytimes);
      continue;
    }
    else
      break;
  }
  
  if(ilTrytimes == DEFAULTCONNUM)
  {
    _bmqDebug("S0122 _bmqGrpasc_snd()连接到服务器失败!! IP=[%s] Port=[%d]",
      agBmqhostip,igBmqgrpport);
    return(FAIL);	
  }
  _bmqVdebug(2,"S0123 _bmqGrpasc_snd()连接到服务器IP=[%s] Port=[%d]成功",
    agBmqhostip,igBmqgrpport);

  /*取得SOCKET信息*/
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( ilSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0130 _bmqGrpasc_snd()unable to read socket address");
    close(ilSockfd);
    return(FAIL);
  }

  /*设置SOCKET选项*/
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
  
  /* 发送身份确认信息,以组号和IP地址(密码串)组成源串 */
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
    _bmqDebug("S0141 发送身份确认信息失败!");
    close(ilSockfd);
    return(FAIL);
  }
  memset(alMsgbuf,0x00,sizeof(alMsgbuf));
  ilMsglen_grp = 5;
  ilRc = _comTcpReceive(ilSockfd,alMsgbuf,&ilMsglen_grp);
  if( ilRc )
  {
    _bmqDebug("S0142 接收身份确认应答信息失败!");
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"UNDEF",5))
  {
    _bmqDebug("S0143 服务器IP=[%s]端未定义本组信息!!!",agBmqhostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"UNMAC",5))
  {
    _bmqDebug("S0144 服务器IP=[%s]未通过身份确认信息!!!",agBmqhostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(!memcmp(alMsgbuf,"IPERR",5))
  {
    _bmqDebug("S0145 服务器IP=[%s]未通过身份确认信息!!!",agBmqhostip);
    close(ilSockfd);
    return(FAIL);
  }
  else if(memcmp(alMsgbuf,"AAAAA",5))
  {
    _bmqDebug("S0146 其他错误");
    close(ilSockfd);
    return(FAIL);
  }
  
  /*组织报文*/
  slGrpmsg.mtype        = iGrpid;
  slGrpmsg.lPack_head   = 0;
  slGrpmsg.iOrg_group   = igMb_sys_current_group;
  slGrpmsg.iOrg_mailbox = igMb_sys_current_mailbox;
  slGrpmsg.iPrior       = iPrior;
  memcpy(slGrpmsg.aFilter,aFilter,iMASKLEN);
  slGrpmsg.iDes_group   = iGrpid;
  slGrpmsg.iDes_mailbox = iMbid;
  
  /*网络字节序转换*/ 
  slGrpmsg.mtype        = htonl(slGrpmsg.mtype);
  slGrpmsg.lPack_head   = htonl(slGrpmsg.lPack_head);
  slGrpmsg.iOrg_group   = htons(slGrpmsg.iOrg_group);
  slGrpmsg.iOrg_mailbox = htons(slGrpmsg.iOrg_mailbox);
  slGrpmsg.iPrior       = htons(slGrpmsg.iPrior);
  slGrpmsg.iDes_group   = htons(slGrpmsg.iDes_group);
  slGrpmsg.iDes_mailbox = htons(slGrpmsg.iDes_mailbox);

  /*组织报文*/
  memset(alMsgbuf_grp,0x00,sizeof(alMsgbuf_grp));
  ilMsglen_grp = iMsglen + sizeof(struct grprecvmsg) + 8 + MAC_LEN;
  sprintf(alVal,"%08d",htons(ilMsglen_grp));
  memcpy(alMsgbuf_grp,alVal,8);
  memcpy(alMsgbuf_grp+8, (char *)&slGrpmsg, sizeof(struct grprecvmsg));
  memcpy(alMsgbuf_grp+8+sizeof(struct grprecvmsg), aMsgbuf, iMsglen);
  _bmqMac(aMsgbuf,iMsglen,alMacbuf);
  memcpy(alMsgbuf_grp+ilMsglen_grp-MAC_LEN,alMacbuf,MAC_LEN);

  /*发送报文*/
  if( igDebug >= 3 )
	{
		_bmqDebug("发送报文内容(%d):",ilMsglen_grp);
		_bmqDebughex(alMsgbuf_grp,ilMsglen_grp);
	}
  ilRc = _bmqGrpSend(ilSockfd,alMsgbuf_grp,&ilMsglen_grp);
  if ( ilRc != 0 )
  {
    _bmqDebug("S0150 : _bmqGrpasc_snd()Send to server ERROR!!");
    close(ilSockfd);
    return(FAIL);
  }

  _bmqVdebug(2,"S0152 发送跨组报文(短连接方式)完成!");
  /*关闭连接*/
  close(ilSockfd);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqGrpmsg_snd
 ** 功能:   发送跨组触发报文
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:lPack_head--报文头链 iPrior--优先级 aFilter--报文过滤条件
             iDes_grp--目标组 iMbid--目标邮箱
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqGrpmsg_snd(long lPack_head,short iPrior,char *aFilter,
  short iDes_grp,short iMbid)
{
  int ilRc;
  struct grprecvmsg  slRecv_grp;

  if(igDebug >= 2)
  {
    _bmqDebug("S0820 *****调用跨组发送触发报文函数_bmqGrpmsg_snd(...)****");
  };
  if( igDebug >= 3)
  {
    _bmqDebug("S0830 从组[%d]邮箱[%d]往组[%d]邮箱[%d]发触发消息,lPack_head,Prior=%d",
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
     _bmqDebug("S0840 跨组发送消息出错 msgsnd(....) errno:%d:%s",errno,strerror(errno)); 
     return(-1130);
  }
 
  if(igDebug >= 2)
  {
    _bmqDebug("S0850 *****结束调用跨组发送触发报文函数_bmqGrpmsg_snd(...)****");
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqClsmsg_snd
 ** 功能:   发送C/S触发报文
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:lPack_head--报文头链 iPrior--优先级 aFilter--报文过滤条件
             iDes_grp--目标组 iMbid--目标邮箱
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************
static int _bmqClsmsg_snd(long lPack_head,short iPrior,char *aFilter,
  short iDes_grp,short iMbid)
{
  int ilRc;
  struct clmbinf  slRecv_cls;

  if(igDebug >= 2)
  {
    _bmqDebug("S0821 *****调用C/S发送触发报文函数_bmqClsmsg_snd(...)****");
  };
  if( igDebug >= 3)
  {
    _bmqDebug("S0830 从组[%d]邮箱[%d]往组[%d]邮箱[%d]发触发消息,lPack_head,Prior=%d",
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
     _bmqDebug("S0841 C/S发送消息出错 msgsnd(....) errno:%d:%s",errno,strerror(errno)); 
     return(-1130);
  }
 
  if(igDebug >= 2)
  {
    _bmqDebug("S0851 *****结束调用C/S发送触发报文函数_bmqClsmsg_snd(...)****");
  }
  return(SUCCESS);
}
del by xujun 2006.12.28
不使用通道管理器*/

/**************************************************************
 ** 函数名: _bmqConn_select
 ** 功能:   选择符合条件的阻塞读的进程发送通知消息
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iMbid--目标邮箱 iPrior--优先级 aFilter--报文过滤条件
             lQueue_id--消息队列ID
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S0860 ***调用函数:mbconn_select(%d,%d,aFilter,%ld)***",
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
        _bmqDebug("S0880 打开文件出错");
        return (FAIL);
      }
      _bmqConnfile_pick(fp,&slMbconnect,ilLink);
      if(igDebug >= 2)
      {
        _bmqDebug("S0870 检查连接[%d]是否与邮件检索条件匹配:tag=%d,\
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
          _bmqDebug("S0890 连接[%d]与邮件检索条件匹配:tag=%d,\
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
          _bmqDebug("S0900 写文件出错");
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
        _bmqDebug("S0875 检查连接[%d]是否与邮件检索条件匹配:tag=%d,\
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
            _bmqDebug("S0910 连接[%d]与邮件检索条件匹配:tag=%d, \
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
      _bmqDebug("S0920 打开文件出错");
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
    _bmqDebug("S0930 往消息队列中发送消息,mtype=%d Queue_id=%d serialnum=%d",
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
    _bmqDebug("S0950 *****结束调用函数:mbconn_select(%d,%d,aFilter,%ld)***",
      iMbid,iPrior,lQueue_id);
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueue_put
 ** 功能:   存储一条队列记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iGrpid--组号          iMbid--目标邮箱 iPrior--优先级 
             aFilter--报文过滤条件 lPack_head--报文链表头
             lQueue_id--消息队列ID
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqQueue_put (short iGrpid,short iMbid,short iPrior,char *aFilter,
  long lPack_head,long *plQueue_id)
{
  int   ilRc,i;

  if(igDebug >= 2)
  {
    _bmqDebug("S0960 ***调用函数 _bmqQueue_put(%d,%d,%d,aFilter,%ld,queue_id)Q_free=%ld maxqueue=%ld***",
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
        _bmqDebug("S0970 向文件中写入队列出错,_bmqQueuefile_put(..).");
        return(1003);
      }

  if( igDebug >= 2)
  {
    _bmqDebug("S0980 ***队列存到文件,结束调用函数:_bmqQueue_put(...)***");
  }
      return(SUCCESS);
    }
  }
  
  if(igDebug >=2)
     _bmqDebug("S0990 使用空闲队列[%d]",i+1);
  *plQueue_id = i+1;
  psgMbshm->lMbqueue_free = i + 1;

  /*增加一条队列记录*/
  ilRc = _bmqQueue_add(i + 1,iGrpid,iMbid,iPrior,aFilter,lPack_head,&psgMbqueue[i]);  
  if(ilRc == FAIL)
    {
      _bmqDebug("S1000 向内存写入队列记录出错,_bmqQueue_add(...).");  
      return(1013);
    }

  if( igDebug >= 2)
  {
    _bmqDebug("S1010 ***队列存到内存,结束调用函数:_bmqQueue_put(...)***");
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqPack_put
 ** 功能:   增加一条报文记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:aMsgbuf--报文存储buf iMsglen--报文长度 
             plPack_head--报文存储记录链表头
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S1020 ***调用函数:_bmqPack_put(Msgbuf,%d,Pack_head) Pack_free=[%d]***",iMsglen,psgMbshm->lMbpack_free);

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
          _bmqDebug("S1030 报文内存段已满,存放在文件段");
        ilRc = _bmqPackfile_put(aMsgbuf+k*iMBPERPACKSIZE,ilMsglen,&llFile_head);
                if(ilRc == FAIL) 
        {
          _bmqDebug("S1040 报文写入缓存文件失败");
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
          _bmqDebug("S1050 ***结束调用函数:_bmqPack_put(...)***");
        }
        return(SUCCESS);
      }
      continue;
    }

    /*存放在内存段*/
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
        _bmqDebug("S1060 保存报文到内存:内存段ID=%d,邮件头=%d,msglen=%d",
          i+1,*plPack_head,psgMbpack[i].iMsglen);
      }
      break;
    }
    if(igDebug >= 2)
    {
      _bmqDebug("S1070 保存报文到内存:内存段ID=%d,邮件头=%d,msglen=%d",
        i+1,*plPack_head,psgMbpack[i].iMsglen); 
    }

    k ++;
    j = i;
  }
  psgMbshm->lMbpack_free = i;

  if(igDebug >= 2)
    {
      _bmqDebug("S1080 ***结束调用函数:_bmqPack_put(...)***");
    }

  return(SUCCESS);
}
/**************************************************************
 ** 函数名: _bmqPack_get
 ** 功能:   提取一条报文记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:aMsgbuf--报文存储buf iMsglen--报文长度
             lPack_head--报文存储记录链表头
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
/*int _bmqPack_get(char *aMsgbuf,short *piMsglen,long lPack_head)delete by wh*/
int _bmqPack_get(char *aMsgbuf,TYPE_SWITCH *piMsglen,long lPack_head)/*add by wh*/
{
  long    i;
  FILE    *fp;
  int     ilRc;
  struct  mbfilepack slMbfilepack;
 
  if(igDebug >= 2) 
    _bmqDebug("S1090 ***调用函数:_bmqPack_get() lPack_head=%d***",lPack_head);

  /* add by xujun 2001.10.16*/
  if (lPack_head <= 0)
  {
    _bmqDebug("S0191 非法的报文头值:%ld",lPack_head);
    return(FAIL);
  }
  /* add end by xujun*/

  if (lPack_head > psgMbshm->lMbpacksize )
  {
    /*取出报文存储索引记录*/
    if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
    {
      _bmqDebug("S1100 打开文件出错!");
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
    /*更新报文存储索引记录*/
    _bmqPackfile_set(fp,0,0,lPack_head);
    _bmqClosefile(fp);

    if(igDebug >= 2)
    {
      _bmqDebug("S1110 从文件中取报文:file=[%s],offset=[%ld],len=[%d]",
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
      _bmqDebug("S1120 从内存中取报文:ID=[%d],len=[%d]",i,psgMbpack[i-1].iMsglen);
    }
    psgMbpack[i-1].iMsglen = 0;
    i = psgMbpack[i-1].lLink;

    if (i > psgMbshm->lMbpacksize)
    {
      /*取出报文存储索引记录*/
      if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
      {
        _bmqDebug("S1130 打开文件出错");
        return (FAIL);
      }
      _bmqPackfile_pick(fp,&slMbfilepack,i);

      ilRc =  _bmqGetlog(slMbfilepack.aLogfile,slMbfilepack.lLoglen,
      slMbfilepack.lLogoffset,aMsgbuf+*piMsglen);
      if (ilRc)
      {
        _bmqDebug("S1140 从记录文件中提取报文出错,_bmqGetlog(..)");
        _bmqClosefile(fp);
        return(ilRc);
      }
      /*更新报文存储索引记录*/
      _bmqPackfile_set(fp,0,0,i);
      _bmqClosefile(fp);

      *piMsglen = *piMsglen + slMbfilepack.lLoglen;

      if(igDebug >= 2)
      {
        _bmqDebug("S1150 从文件中取报文:file=[%s],offset=[%ld],len=[%d]",
          slMbfilepack.aLogfile,
            slMbfilepack.lLogoffset,
              slMbfilepack.lLoglen);
      }
      if (igDebug >= 2) _bmqDebug("S1160 ***结束调用函数:_bmqPack_get()***");
      return(SUCCESS);
    }

    if (i == 0) break;
  }
  if (igDebug >= 2) _bmqDebug("S1170 ***结束调用函数:_bmqPack_get()***");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueue_get
 ** 功能:   提取一条队列记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:piGrpid--组号   piMbid--目标邮箱?piPrior--优先级 
             aFilter--报文过滤条件  aMask--报文掩码   
             plPack_head--报文链表头
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S1180 ***调用提取一条队列记录函数:_bmqQueue_get(%d,%d,%d,aFilter,aMask)***",
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

  /* 取出邮箱消息链表头 */
  llQueue_head = psgMbinfo[igMb_sys_current_mailbox - 1].lQueue_head;
  ilSize       = sizeof(struct mbqueue) + 1;
  
  for(;;)
  {
    /* 未找到指定记录 */
    if (llQueue_head == 0)
      return(1001);
     
    /* 记录在缓存文件中 */
    if (llQueue_head > psgMbshm->lMbqueuecount)
    {
      /* 打开队列缓存文件 */
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1190 打开文件出错");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_pick(fp,&slMbqueue,llQueue_head);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1200 队列缓存文件中取记录失败!");
        _bmqClosefile(fp);
        _bmqUnlock(LK_ALL);
        return(FAIL);
      }

      /* 匹配消息 Filter,Mask */
      
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
          _bmqDebug("S1208 删除的队列[%d]没有启用",llQueue_head);
          return (FAIL);
        }

        /* 重写文件 */
        alBuf[0] = '0';
        fseek(fp,0,SEEK_SET);
        ilRc = _bmqPutfile(fp, (llQueue_head-psgMbshm->lMbqueuecount-1)*ilSize, 
          ilSize,alBuf);
        if (ilRc)
        {
          _bmqDebug("S1210 写文件出错");
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
    /* 记录在共享内存中 */
    else
    {
      /* 匹配消息 Filter,Mask */
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
          _bmqDebug("S0672 删除的队列[%d]没有启用",llQueue_head);
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
    _bmqDebug("S1220 ***结束调用函数:_bmqQueue_get(...)***");
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueue_del
 ** 功能:   从链中删除一个队列记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iMbid--当前邮箱? slMbqueue--队列结构 
             piGrpid--组号     iMbid--邮箱号
             piPrior--优先级   aFilter--报文过滤条件
             lPack_head--报文链表头  
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqQueue_del(short iMbid,struct mbqueue *slMbqueue,short *piGrpid,
  short *piMbid,short *piPrior,char *aFilter,long *plPack_head)
{
  long  llRlink,llLlink;
  int   ilRc;
  FILE  *fp;
 
  if( igDebug >=2)
  {
    _bmqDebug("S1230 调用(从链中删除一个队列记录)函数 _bmqQueue_del(%d,%d,%d,aFilter)",
      *piGrpid,*piMbid,*piPrior);
    _bmqDebughex(aFilter,iMASKLEN);
  }

  if( !slMbqueue->cTag )
  {
    _bmqDebug("S1232 队列没有启用,报文可能已被其他进程取走");
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
  

  /* 前面有队列 */
  if (llLlink)
  {
    if(llLlink > psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1240 打开文件出错");
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
  /* 队列头 */
  else
  {
    psgMbinfo[iMbid - 1].lQueue_head = llRlink;
  }

  /* 后面有队列 */
  if (llRlink)
  {
    if(llRlink > psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1250 打开文件出错");
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
  /* 队列尾 */
  else
  {
    psgMbinfo[iMbid - 1].lQueue_tail = llLlink;
  }
  if ( igDebug >= 2 )
    {
      _bmqDebug("S1260 ***结束调用函数  _bmqQueue_del(...)***");
    }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueue_add
 ** 功能:   在链中增加一个队列记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:lQueue_id--对列记录ID?iGrpid--组号 iMbid--邮箱号
             iPrior--优先级 aFilter--报文过滤条件
             lPack_head--报文链表头  
             psMbqueue--队列结构指针
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqQueue_add(long lQueue_id,short iGrpid,short iMbid,short iPrior,
  char *aFilter,long lPack_head,struct mbqueue *psMbqueue)
{
  int    ilRc;
  long   llQueue_head,llQueue_tail;
  
  if(igDebug >= 2)
  {
    _bmqDebug("S1270 ***调用函数_bmqQueue_add(%ld,%d,%d,%d,%ld,aFilter,%ld)****",
      lQueue_id,iGrpid,iMbid,iPrior,lPack_head);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebug("S1280 函数_bmqQueue_add(..)功能:在链中增加一个队列记录");
  }
  
  psgMbshm->lgMb_sys_queue_serial_num ++;

  /* 结构赋值 */
  psMbqueue->cTag         = 1;
  psMbqueue->lSerialnum   = psgMbshm->lgMb_sys_queue_serial_num;
  psMbqueue->iPrior       = iPrior;
  memcpy(psMbqueue->aFilter,aFilter,iMASKLEN);
  psMbqueue->iOrg_grp     = igMb_sys_current_group;
  psMbqueue->iOrg_mailbox = igMb_sys_current_mailbox;
  psMbqueue->lPack_head   = lPack_head;
  time(&(psMbqueue->lBegintime));

  if(igDebug >= 2)
    _bmqDebug("S1290 置新数据包流水号[%d]",psgMbshm->lgMb_sys_queue_serial_num);

   llQueue_head = psgMbinfo[iMbid-1].lQueue_head;  
   llQueue_tail = psgMbinfo[iMbid-1].lQueue_tail;  

  /* 队列排序 */
  ilRc = _bmqQueue_sort(iMbid,lQueue_id,llQueue_head,llQueue_tail,psMbqueue);
  if(ilRc == FAIL)
  {
    _bmqDebug("S1300 队列排序ERROR!");
    return(FAIL);
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueue_sort
 ** 功能:   插入记录在队列链表中按优先级排序
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iMbid--目标邮箱 lQueue_id--消息队列ID 
             psMbqueue--队列结构指针
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S1310 ***调用函数 _bmqQueue_sort(%d,%ld,%ld,Mbqueue)***",iMbid,lQueue_id,lQueue_head);
  }
    
  /*空队列,插在队列头*/
  if (lQueue_head == 0)
  {
    psMbqueue->lLlink = 0;
    psMbqueue->lRlink = 0;
    psgMbinfo[iMbid-1].lQueue_head = lQueue_id;
    psgMbinfo[iMbid-1].lQueue_tail = lQueue_id;

    if(igDebug >= 2)
    {
      _bmqDebug("S1320 空队列,插在队列头,邮箱的队列头[%d] %d %d",
        psgMbinfo[iMbid-1].lQueue_head,
          psMbqueue->lLlink,
            psMbqueue->lRlink);
    }
    return(SUCCESS);
  }
  
  ilPrior = psMbqueue->iPrior;
  ilSize  = sizeof(struct mbqueue) + 1;
  /*按优先级排序*/
  for(;;)
  {
    if( lQueue_tail > psgMbshm->lMbqueuecount )
    {
      /*打开文件取出记录进行比较*/
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1330 打开文件出错");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_pick(fp,&slMbqueue,lQueue_tail);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1340 队列缓存文件中取记录 _bmqQueuefile_pick(..)返回错误");
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
            _bmqDebug("S1350 [%d]插在当前队列[%d]前",lQueue_id,lQueue_head);
          _bmqQueueInsert_Before(iMbid,lQueue_id,psMbqueue->lRlink);
        }
        else
        {
          psMbqueue->lLlink = 0;
          psMbqueue->lRlink = lQueue_tail;
          slMbqueue.lLlink  = lQueue_id;
          psgMbinfo[iMbid-1].lQueue_head = lQueue_id;
        }
        /*写文件记录*/
        alBuf[0] = '1';
        memcpy(alBuf + 1, (char *)&slMbqueue, ilSize - 1);
        fseek(fp,0,SEEK_SET);
        ilRc = _bmqPutfile(fp, (lQueue_tail-psgMbshm->lMbqueuecount-1)*ilSize,
          ilSize, alBuf);
        if (ilRc)
        {
          _bmqDebug("S1360 写文件出错!");
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
          _bmqDebug("S1370 [%d]插在当前队列[%d]前",lQueue_id,lQueue_head);
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
    _bmqDebug("S1380 ***结束调用函数 _bmqQueue_sort(...)***");
  }

  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueueInsert_Before
 ** 功能:   记录插在当前队列前
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iMbid--目标邮箱?lQueue_id--消息队列ID
             lQueue_Llink--队列记录左链
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqQueueInsert_Before(short iMbid,long lQueue_id,long lQueue_Rlink)
{
  int    ilRc;
  FILE   *fp;

  if( igDebug >= 2)
  {
    _bmqDebug("S1390 ****调用函数_bmqQueueInsert_Before(%d,%ld,%ld)****",
      iMbid,lQueue_id,lQueue_Rlink);
  }   
  if(lQueue_Rlink)
  {
    if(lQueue_Rlink > psgMbshm->lMbqueuecount)
    {
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1400 打开文件出错");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_set(fp, -1, lQueue_id,-1, lQueue_Rlink);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1410 _bmqQueuefile_set(..)返回错误!"); 
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
      _bmqDebug("S1420 邮箱的队列头[%d]",psgMbinfo[iMbid-1].lQueue_head);
    psgMbinfo[iMbid-1].lQueue_tail = lQueue_id;
  }
  if( igDebug >= 2)
  {
    _bmqDebug("S1430 ****结束调用函数_bmqQueueInsert_Before(..)****");
  }
  return(0);
}

/**************************************************************
 ** 函数名: _bmqQueue_move
 ** 功能:   从文件中移动一条记录到共享内存
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:fp--队列文件指针   lQueue_id--消息队列ID
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
/* delete by szhengye 2001.10.29
static int _bmqQueue_move(FILE *fp,long lQueueid)
{
  int    ilRc;
  long   llQueue_head;
  short  ilPrior,i;
  struct mbqueue slMbqueue;

  if(igDebug >= 2)
    _bmqDebug("S1440 ***调用函数 _bmqQueue_move(%d)***",lQueueid);
 
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
           _bmqDebug("S1450 ***结束调用函数 _bmqQueue_move(..)***");
        return(FINISH);
      }
      llQueue_head = psgMbqueue[llQueue_head - 1].lRlink;
    }
  }

  if(igDebug >= 2)
    _bmqDebug("S1460 ***结束调用函数 _bmqQueue_move(..)***");
  return(SUCCESS);
}
 delete by szhengye 2001.10.29 */

/**************************************************************
 ** 函数名: _bmqConnect_add
 ** 功能:   增加一条连接记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:_bmqConnfile_put,_bmqConnect_update
 ** 全局变量:
 ** 参数含义:iMbid--当前邮箱号
 ** 返回值: SUCCESS,FAIL
***************************************************************/
int _bmqConnect_add(short iMbid,long lMsgid)
{
  int ilRc,i;
  
  if( igDebug >= 2)
  {
    _bmqDebug("S1470 ***调用函数 _bmqConnect_add(%d)*** ",iMbid);
  }
 
  psgMbinfo[iMbid-1].iConnect ++;
    
  i = psgMbshm->iMbconnect_free;

  /*找出空闲区*/
  for(;;)
  {
    if (i>=psgMbshm->iMbcontcount) i = 0;
    if (psgMbconnect[i].cTag == 0) break;
    i ++;
    /*共享内存区满*/
    if (i == psgMbshm->iMbconnect_free)
    {
      /*写入缓存文件*/
      ilRc = _bmqConnfile_put(iMbid,lMsgid);
      if(ilRc == FAIL) 
      {
        _bmqDebug("S1480 写入连接区缓存文件错误!");
        return(1005);
      }
      return(SUCCESS);
    }
  }

  psgMbshm->iMbconnect_free = i + 1;
  igMb_sys_current_connect  = i + 1;

  /*更新连接区记录内容*/
  ilRc = _bmqConnect_update(iMbid,&psgMbconnect[i]);
  if(ilRc == FAIL)
  {
    _bmqDebug("S1490 更新连接区记录内容出错!");
    return(FAIL);  
  }
  psgMbconnect[i].lMsgid_recv = lMsgid;

  if( igDebug >= 2)
  {
    _bmqDebug("S1500 ***结束调用函数 _bmqConnect_add(%d)*** ",iMbid);
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqConnect_del
 ** 功能:   删除一条连接记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iMbid--当前邮箱   psMbconnect--连接结构指针
 ** 返回值: SUCCESS,FAIL
***************************************************************/
int _bmqConnect_del(short iMbid,struct mbconnect *psMbconnect)
{
  int    ilRc;
  short  ilLlink,ilRlink;
  
  if( igDebug >= 2)
    _bmqDebug("S1510 ****调用函数_bmqConnect_del(%d,*psMbconnect)****",iMbid);

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
      /*更新记录右链*/
      ilRc = _bmqConnfile_set(-1,-1,ilRlink,psMbconnect->aFilter,psMbconnect->aMask,ilLlink,-1);
      if(ilRc == FAIL)
      {
        _bmqDebug("S1520 更新记录右链函数 _bmqConnfile_set() 返回出错");
        return (FAIL);
      }
    }
    else
      psgMbconnect[ilLlink-1].iRlink = ilRlink;
  }
  else
  {
    /*连接链表头*/
    psgMbinfo[iMbid].iConn_head = ilRlink;
  }
  
  if (ilRlink)
  {
    if(ilRlink > psgMbshm->iMbcontcount)
    {
      /*更新记录左链*/
      ilRc = _bmqConnfile_set(-1,ilLlink,-1,psgMbconnect->aFilter,psgMbconnect->aMask,ilRlink,-1);
      if(ilRc == FAIL)
      {
        _bmqDebug("S1530 更新记录左链_bmqConnfile_set() 返回出错");
        return (FAIL);
      }
    }
    else
      psgMbconnect[ilRlink-1].iLlink = ilLlink;
  }
  else
  {
    /*连接链表尾*/
    psgMbinfo[iMbid].iConn_tail = ilLlink;
  }
  /* next 7 rows add by nh 2001.11.29*/
  ilRc = _bmqGetfrommsg(iMbid+1,psMbconnect->iPrior,psMbconnect->lMsgid_recv);
  if(ilRc)
  {
    _bmqDebug("S0000 调用函数_bmqGetfrommsg()出错");
    return(FAIL);
  }

  ilRc = msgctl(psMbconnect->lMsgid_recv,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1850 删除消息对列 msgctl(RM-recv) 出错 errno:%d:%s",errno,strerror(errno));
    return(FAIL);
  }
  psMbconnect->lMsgid_recv = 0;
  
  if( igDebug >= 2)
    _bmqDebug("S1540 ***结束调用函数_bmqConnect_del(%d,*psMbconnect)***",iMbid);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      :_bmqStrand
 ** 功能        : 
 ** 作者        : 
 ** 建立日期    :2001/9/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : SUCCESS
***************************************************************/
int _bmqStrand(char *aBuf1,char *aBuf2,short iLen,char *aResult)
{
  short i;
  for(i = 0;i < iLen; i++)
    *(aResult++) = *(aBuf1++) & *(aBuf2++);
    
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqConnect_update
 ** 功能:   更新连接记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iMbid--当前邮箱   psMbconnect--连接结构指针
 ** 返回值: SUCCESS,FAIL
***************************************************************/
static int _bmqConnect_update(short iMbid,struct mbconnect *psMbconnect)
{
  int  ilRc;
 
  if( igDebug>=2 )
    _bmqDebug("S1550 ***调用函数_bmqConnect_update(%d, *psMbconnect)***",iMbid);       
  psMbconnect->cTag = 1;
  psMbconnect->lConnpid = getpid();

  /*连接在对尾*/
  if (psgMbinfo[iMbid-1].iConn_tail != 0)
  {
    /*尾链在缓存文件中*/
    if(psgMbinfo[iMbid-1].iConn_tail > psgMbshm->iMbcontcount)
    {
      ilRc = _bmqConnfile_set(-1,-1,igMb_sys_current_connect,psMbconnect->aFilter,
                psMbconnect->aMask,psgMbinfo[iMbid-1].iConn_tail,-1);
      if(ilRc == FAIL)
      {
        _bmqDebug("S1560 _bmqConnfile_set() 返回出错");
        return (FAIL);
      }
    }
    else
      psgMbconnect[psgMbinfo[iMbid-1].iConn_tail-1].iRlink =
        igMb_sys_current_connect;
  }
  psMbconnect->iLlink = psgMbinfo[iMbid-1].iConn_tail;
  psMbconnect->iRlink = 0;

  /*设置头链和尾链*/
  if (psgMbinfo[iMbid-1].iConn_head == 0)
    psgMbinfo[iMbid-1].iConn_head = igMb_sys_current_connect; 
    
  psgMbinfo[iMbid-1].iConn_tail = igMb_sys_current_connect; 

  if(igDebug >= 2)
  {
    _bmqDebug("S1570 打开邮箱后,邮箱连接ID:head=%d,tail=%d,Llink=%d,Rlink=%d",
       psgMbinfo[iMbid-1].iConn_head, psgMbinfo[iMbid-1].iConn_tail,
         psMbconnect->iLlink,psMbconnect->iRlink);
  }
  if( igDebug>=2 )
    _bmqDebug("S1580 ***调用函数_bmqConnect_update(%d, *psMbconnect)***",iMbid);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名:_bmqConnWait_set
 ** 功能:  设置阻塞读取条件
 ** 作者:  徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:_bmqConnfile_set
 ** 全局变量:
 ** 参数含义:iConid--连接记录ID aFilter--报文过滤条件
 	     aMask--报文掩码 
 ** 返回值: SUCCESS,FAIL
***************************************************************/
int _bmqConnWait_set(short iConid,char *aFilter,char *aMask)
{
  int  ilRc;
  
  if( igDebug>=2 )
  {
    _bmqDebug("S1590 ***调用函数 _bmqConnWait_set(%d,aFilter,aMask)***",
              iConid);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);
  }

  if(iConid > psgMbshm->iMbcontcount)
 {
    /*当前连接记录在文件中*/
    /*设置阻塞读条件 aFilter aMask */
    ilRc = _bmqConnfile_set(2, -1, -1, aFilter,aMask,iConid,0);
    if(ilRc == FAIL)
    {
      _bmqDebug("S1600 _bmqConnfile_set() 返回出错");
      return (FAIL);
    }
  }
  /*当前连接在内存*/
  else
  {
    psgMbconnect[iConid - 1].cTag = 2;
    memcpy(psgMbconnect[iConid - 1].aFilter,aFilter,iMASKLEN);
    memcpy(psgMbconnect[iConid - 1].aMask,aMask,iMASKLEN);
  }

  if( igDebug>=2 )
  {
    _bmqDebug("S1610 ***结束调用函数 _bmqConnWait_set(..)***");
  }
  return(0);
}

/**************************************************************
 ** 函数名: _bmqGetlog
 ** 功能:   从报文存储文件中提取一条报文存储记录
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:logfile--log文件名 loglen--log报文长度
             logoffset--文件偏移量  msgpack--报文存储buf
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
/*static int _bmqGetlog(char * logfile,short loglen,long logoffset,char *msgpack) delete by wh*/
static int _bmqGetlog(char * logfile,TYPE_SWITCH loglen,long logoffset,char *msgpack)
{
  FILE *fp;
  long llLen;
  char amFp_name[80];

  if(igDebug >= 2)
    _bmqDebug("S1620 ***调用函数_bmqGetlog(logfile,loglen,logoffset,msgpack**");

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
    _bmqDebug("S1630 ***调用函数 _bmqGetlog(....)***");

  if (llLen >= 0) return (0);
  else return (-1120);
}

/**************************************************************
 ** 函数名: _bmqPutlog
 ** 功能:   向报文存储文件中存放一条报文存储记录
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:logfile--log文件名 loglen--log报文长度
             logoffset--文件偏移量  msgpack--报文存储buf
 ** 返回值: SUCCESS--成功，FAIL--失败
****************************************************************/
static int _bmqPutlog(char *msgpack,short loglen,long *logoffset,char * logfile)
{
  static short ilFileid = 1;  /*动态文件名后缀数值初始值*/
  long   llLen;
  FILE   *fp;
  long   ilPid;
  char   amFp_name[80];

  if (igDebug >=2 )
    _bmqDebug("S1640 **调用函数 _bmqPutlog(msgpack,loglen,logoffset,logfile)*");
  ilPid = getpid();
  memset(amFp_name,0x00,80);
  sprintf(amFp_name,"%s/bmqlog/logfile/bmq%ld.%d",
    getenv("BMQ_PATH"),ilPid,ilFileid);

  for(;;)
  {
    fp=fopen(amFp_name,"a+");
    if(fp==NULL) 
    {
      _bmqDebug("S1650 无法打开文件[%s]",amFp_name);
      _bmqDebug("S1651 打开文件出错errno:%d:%s",errno,strerror(errno));
      return(-1023);
    }
    fflush(fp);
    llLen=fseek(fp, 0, SEEK_END);
    if ( llLen ) 
    {
      _bmqDebug("S1660 记日志文件出错:fseek():error");
      _bmqDebug("S1661 定位日志文件出错errno:%d:%s",errno,strerror(errno));
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
    _bmqDebug("S1670 写日志文件出错errno:%d:%s",errno,strerror(errno));
    return(-1120);
  }

  fclose(fp);

  if(igDebug >= 2)
    _bmqDebug("S1680 **结束调用 _bmqPutlog(...)**");

  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqDebugmbinfo
 ** 功能:   记录邮箱区Debug信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqDebugmbinfo()
{
  int i;

  _bmqDebug("S1690 ***邮箱信息(psgMbinfo[])***");
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
 ** 函数名: _bmqDebugmbqueue
 ** 功能:   记录队列区Debug信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqDebugmbqueue()
{
  int i;

  _bmqDebug("S1710 ***邮件队列(psgMbqueue[])***");
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
 ** 函数名: _bmqDebugmbpack
 ** 功能:   记录报文区Debug信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqDebugmbpack()
{
  int i;

  _bmqDebug("S1730 ***队列内存数据包(psgMbpack[])***");
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
 ** 函数名: _bmqDebugmbconnect
 ** 功能:   记录链接区Debug信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqDebugmbconnect()
{
  int i;

  _bmqDebug("S1750 ***邮箱接收条件栈(psgMbconnect[])***");
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
 ** 函数名: _bmqPrintmbinfo
 ** 功能:   打印邮箱区信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqPrintmbinfo()
{
  int i;

  printf("***邮箱信息(psgMbinfo[])***\n");
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
 ** 函数名: _bmqPrintmbqueue
 ** 功能:   打印队列区信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqPrintmbqueue()
{
  int    ilRc;
  long   i;
  FILE   *fp;
  struct mbqueue slMbqueue;

  printf("***邮件队列(psgMbqueue[])***\n");
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
  printf("***邮件队列缓存文件记录***\n");
  printf("ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
  if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
  {
    printf("打开文件出错\n");
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
 ** 函数名: _bmqPrintmbpack
 ** 功能:   打印报文区信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqPrintmbpack()
{
  int    ilRc;
  long   i;
  FILE   *fp;
  struct mbfilepack slMbfilepack;

  printf("***队列内存数据包(psgMbpack[])***\n");
  printf("ID     iMsglen lLink\n");
  for(i=1;i<=psgMbshm->lMbpacksize;i++)
  {
    if (psgMbpack[i-1].iMsglen)
      printf("%-6ld %7d %5ld\n",i,
        psgMbpack[i-1].iMsglen,
        psgMbpack[i-1].lLink);
  }

  printf("***文件内存数据包***\n");
  printf("ID     iMsglen     aLogfile lLogoffset\n");
  if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
  {
    printf("打开文件出错\n");
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
 ** 函数名: _bmqPrintmbconnect
 ** 功能:   打印链接区信息
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqPrintmbconnect()
{
  int    i,ilRc;
  FILE   *fp;
  struct mbconnect slMbconnect;

  printf("***邮箱接收条件栈(psgMbconnect[])***\n");
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

  printf("***邮箱连接缓存文件记录***\n");
  printf("ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
  if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
  {
    printf("打开文件出错\n");
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
 ** 函数名: _bmqLock
 ** 功能:   分配信号灯资源
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S1780 信号灯操作出错semop P() errno:%d:%s",errno,strerror(errno)); 
    exit(-1);
  }

  if(igDebug >= 2)
    _bmqDebug("S1790 _bmqLock:%d ok",iNum);


  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqUnlock
 ** 功能:   释放信号灯资源
 ** 作者:   shizhengye
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
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
 ** 函数名: _bmqProshut
 ** 功能:   关闭服务器收发守护进程、邮箱状态监控进程和跨组守护进程 
 ** 作者:  xujun 
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
void _bmqProshut()
{
  if( igDebug >= 2 )
    _bmqDebug("S1820 ***调用函数 _bmqProshut()关闭五个进程****");

  /*关闭服务器侦听进程、邮箱状态监控进程和跨组守护进程*/
  /*
  if( psgMbshm->lBmqclsrcv_pid > 0 )
    kill(psgMbshm->lBmqclsrcv_pid,SIGTERM);
  if( psgMbshm->lBmqclssnd_pid > 0 )
    kill(psgMbshm->lBmqclssnd_pid,SIGTERM);
  del by xujun 2006.12.28
  不使用通道管理器*/
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
     增加4个文件服务守护进程 */
  if( psgMbshm->lBmqfilecls_pid > 0 )
    kill(psgMbshm->lBmqfilecls_pid,SIGTERM);
  if( psgMbshm->lBmqfilemng_pid > 0 )
    kill(psgMbshm->lBmqfilemng_pid,SIGTERM);
  if( psgMbshm->lBmqfilesaf_pid > 0 )
    kill(psgMbshm->lBmqfilesaf_pid,SIGTERM);
  if( psgMbshm->lBmqfilelog_pid > 0 )
    kill(psgMbshm->lBmqfilelog_pid,SIGTERM);

  if( igDebug >= 2 )
    _bmqDebug("S1830 ***结束调用函数 _bmqProshut()****");
}

/**************************************************************
 ** 函数名: _bmqQuit
 ** 功能:   释放新晨Q申请的IPC资源
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqQuit()
{
  int   ilRc,ilRt;
 
  if( igDebug >= 2)
    _bmqDebug("S1840 ***调用函数_bmqQuit()释放新晨Q申请的IPC资源**");

  ilRt  =  0;

  /*删除消息对列*/
  ilRc = msgctl(psgMbshm->lMsgid_grp,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1860 删除消息对列 msgctl(RM-grp) errno:%d:%s",errno,strerror(errno));
    ilRt = -1161;
  }
  if(psgMbshm->lMsgid_grp_rcv)
  {
    ilRc = msgctl(psgMbshm->lMsgid_grp_rcv,IPC_RMID,0);
    if(ilRc < 0)
    {
      _bmqDebug("S1860 删除消息对列 msgctl(RM-grp) errno:%d:%s",errno,strerror(errno));
      ilRt = -1161;
    }
  }
  /*
  ilRc = msgctl(psgMbshm->lMsgid_cls,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1861 删除消息对列 msgctl(RM-cls) errno:%d:%s",errno,strerror(errno));
    ilRt = -1161;
  }
  del by xujun 2006.12.28
  不使用通道管理器*/

  /*删除信号灯*/
  ilRc = semctl(psgMbshm->lSemid,0,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1870 删除信号灯出错 semctl(RM) errno:%d:%s",errno,strerror(errno));
    ilRt = -1171;
  }
  /*删除共享内存*/
  ilRc = shmctl(igMb_sys_shm_id,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1880 删除共享内存出错 shmctl(RM) errno:%d:%s",errno,strerror(errno));
    ilRt = -1153;
  }
  if( igDebug >= 2)
    _bmqDebug("S1890 ***结束调用函数_bmqQuit()**");
  return(ilRt);
}

/**************************************************************
 ** 函数名: _bmqTimeout
 ** 功能:   阻塞读取消息队列退出时间
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
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
 ** 函数名: _bmqConn_judge
 ** 功能:   判断进程是否申请过连接
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:lPid--进程Pid
 ** 返回值: SUCCESS--成功，FAIL--失败
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
        /*打开连接缓存文件*/
        if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
        {
          _bmqDebug("S1900 打开连接缓存文件出错");
          return (FAIL);
        }
        /*取出指定记录*/
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
 ** 函数名: _bmqConfig_load
 ** 功能:   载入系统配置参数
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:aCfgType--参数类型   iGrpid--组号
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S1910 读系统环境变量[BMQ_PATH]失败");
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
    /*载入通道管理通讯端口
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
    不使用通道管理器*/
    /*next add by nh 20011128载入TCP通讯端口*/
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
    /*载入跨组连接IP和通讯端口*/
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
    /*next add by nh 载入C端的IP地址和端口
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
        参数为－1时是初始化本地邮箱
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
    不使用通道管理器*/
        
    /*载入BMQ参数*/
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
    /*载入共享内存ID*/
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
    /*载入邮箱驻留时间和阀值*/
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
    /*载入系统轮询时间*/
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
    /*载入DEBUG标志*/
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
 ** 函数名: _bmqOpenfile
 ** 功能:   打开缓存文件
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqOpenfile(int iFileid,FILE **fp)
{
 char   alFile[101];
 char   *alPath;
 char   alBmqfile[3][101]={"/bmqlog/logfile/queue.rec","/bmqlog/logfile/pack.rec","/bmqlog/logfile/connect.rec"};

 if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S1920 读系统环境变量[BMQ_PATH]失败");
    return -1;
  }
 
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,alBmqfile[iFileid]);

  if ((*fp = fopen(alFile, "r+")) == NULL)
  {
    if ((*fp = fopen(alFile, "w+")) == NULL)
    {
      _bmqDebug("S1921 文件打开失败:%d,%s",errno,strerror(errno));
      return(FAIL);
    }
    else
      return(SUCCESS);
  }
  else
    return(SUCCESS);    
}

/**************************************************************
 ** 函数名: _bmqClosefile
 ** 功能:   关闭缓存文件
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqClosefile(FILE *fp)
{
  fclose(fp);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqUnlinkfile
 ** 功能:   删除缓存文件
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqUnlinkfile()
{
  int    i;
  char   *alPath,alFile[80];
  char   alBmqfile[4][101]={"/bmqlog/logfile/queue.rec","/bmqlog/logfile/pack.rec","/bmqlog/logfile/connect.rec","/bmqlog/debug/bmqstatfile.log"};
 
  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S1930 读系统环境变量[BMQ_PATH]失败");
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
 ** 函数名: _bmqGetfile
 ** 功能:   从缓存文件取记录
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
/*int _bmqGetfile(FILE *fp, long lOffset, short iLen, char *aBuf)delete by wh*/
int _bmqGetfile(FILE *fp, long lOffset, TYPE_SWITCH iLen, char *aBuf)/*add by wh*/
{
  TYPE_SWITCH ilRc;/*add by wh*/

  if( igDebug >= 2)
    _bmqDebug("S1940 **调用函数 _bmqGetfile(fp,Offset,Len,aBuf)从缓存文件取记录**");
  if (lOffset < -1)
  {
    _bmqDebug("S1950 偏移量出错");
    return (FAIL);
  }
  fflush(fp);
  if (lOffset != OFFSETNULL)
  {
    ilRc = fseek(fp, lOffset, SEEK_SET);
    if (ilRc)
    {
      _bmqDebug("S1960 文件定位出错");
      return (FAIL);
    }
  }
  ilRc = fread(aBuf, sizeof(char), iLen, fp);
  if (feof(fp))
  {
    if(igDebug >= 2)
      _bmqDebug("S1970 读到文件尾getfile()");
    return (SHMFILEEND);
   }
  if (ilRc != iLen)
  {
    _bmqDebug("S1980 读文件出错getfile()");
    return (FAIL);
  }
  if( igDebug >= 2)
    _bmqDebug("S1990 **结束调用函数 _bmqGetfile(...)**");

  return (SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqPutfile
 ** 功能:   放记录到缓存文件
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
/*int _bmqPutfile(FILE *fp, long lOffset, short iLen, char *aBuf)delete by wh*/
int _bmqPutfile(FILE *fp, long lOffset, TYPE_SWITCH iLen, char *aBuf)/*add by wh*/
{
  /*short ilRc;delete by wh*/
  TYPE_SWITCH ilRc;/*add by wh*/

  if ( igDebug >= 2)
    _bmqDebug("S2000 **调用函数 _bmqPutfile(fp,Offset,Len,aBuf)**");

  if (lOffset < -1)
  {
    _bmqDebug("S2010 偏移量出错");
    return (FAIL);
  }
  fflush(fp);
  if (lOffset != OFFSETNULL)
  {
    ilRc = fseek(fp, lOffset, SEEK_SET);
    if (ilRc)
    {
      _bmqDebug("S2020 文件定位出错");
      _bmqDebug("S2021 文件定位出错errno:%d:%s",errno,strerror(errno));
      return (FAIL);
    }
  }
  ilRc = fwrite(aBuf, sizeof(char), iLen, fp);
  if (ilRc != iLen)
  {
    _bmqDebug("S2030 写文件出错");
    _bmqDebug("S1660 写文件出错errno:%d:%s",errno,strerror(errno));
    return (FAIL);
  }
  if( igDebug >= 2)
    _bmqDebug("S2040 **结束调用函数 _bmqPutfile(..)**");

  return (SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueuefile_put
 ** 功能:   存放一条队列记录缓存文件
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iGrpid--组号  iMbid--目标邮箱   iPrior--优先级
             aFilter--报文过滤条件  lPack_head--报文链表头
             plQueue_id--消息队列ID
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S2050 **调用_bmqQueuefile_put()**");
    _bmqDebug("S2060 iGrpid=%d iMbid=%d iPrior=%d lPack_head=%ld ",iGrpid,iMbid,iPrior,lPack_head);
    _bmqDebughex(aFilter,iMASKLEN);
  }

  if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
  {
    _bmqDebug("S2070 打开文件出错");
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
      _bmqDebug("S2080 读文件出错insertqueuelog()");
      _bmqClosefile(fp);
      return (FAIL);
    }
    llOffset = llOffset + ilSize;
    if (ilRc == SUCCESS)
        if (alBuf[0] == '0') break;
    if (ilRc == SHMFILEEND)  break;
  }
 
  *plQueue_id = i + psgMbshm->lMbqueuecount;

  /*增加一条队列记录 */
  memset(&slMbqueue,0x00,sizeof(struct mbqueue));
  ilRc = _bmqQueue_add(i + psgMbshm->lMbqueuecount,iGrpid,iMbid,iPrior,
    aFilter,lPack_head,&slMbqueue);
  if(ilRc == FAIL)
  {
    _bmqDebug("S2090 增加一条队列记录入文件ERROR!");
    _bmqClosefile(fp);
    return(FAIL);
  }

  /*记录写入文件*/
  alBuf[0] = '1';
  memcpy(alBuf + 1, (char *)&slMbqueue, ilSize - 1);
  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (i-1)*ilSize, ilSize, alBuf);
  if (ilRc)
  {
    _bmqDebug("S2100 写文件出错");
    _bmqClosefile(fp);
    return (FAIL);
  }

  _bmqClosefile(fp);

  if( igDebug >= 2)
    _bmqDebug("S2110 **结束调用_bmqQueuefile_put()**");

  return(SUCCESS);  

}

/**************************************************************
 ** 函数名: _bmqQueuefile_set
 ** 功能:   队列缓存文件记录设置
 ** 作者:  xujun
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:fp--文件指针  iTag--启用标志  lLlink--左链
             lRlink--右链  lOffsetid--记录偏移量
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
int _bmqQueuefile_set(FILE *fp,short iTag, long lLlink,long lRlink,long lOffsetid)
{
  int    ilRc,ilSize;
  char   alBuf[iFLDVALUELEN];
  struct mbqueue slMbqueue;
  

  if (igDebug >= 2)
    _bmqDebug("S2120 **调用函数_bmqQueuefile_set(fp,Tag,.....)**");

   ilSize = sizeof(struct mbqueue) + 1;

   fseek(fp,0,SEEK_SET);
   ilRc = _bmqGetfile(fp, (lOffsetid-psgMbshm->lMbqueuecount-1)*ilSize, ilSize,alBuf);
   if (ilRc < 0)
    {
      _bmqDebug("S2130 读文件出错queuefile_put()");
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
     _bmqDebug("S2140 写文件出错");
     return (FAIL);
    }

  if (igDebug >= 2)
    _bmqDebug("S2150 **结束调用函数_bmqQueuefile_set(.....)**");
 return(SUCCESS);
}
  
/**************************************************************
 ** 函数名: _bmqConnfile_put
 ** 功能:   在连接缓存文件中增加一条记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iMbid--邮箱号
 ** 返回值: SUCCESS,FAIL
***************************************************************/
static int _bmqConnfile_put(short iMbid,long lMsgid)
{
  int   ilRc,ilSize;
  short i;
  char  alBuf[iFLDVALUELEN];
  FILE  *fp;
  struct mbconnect slMbconnect;

  if( igDebug >= 2)
    _bmqDebug("S2160 **调用函数_bmqConnfile_put(%d)**",iMbid);

  if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
  {
    _bmqDebug("S2170 打开连接缓存文件出错!");
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
      _bmqDebug("S2180 读文件出错 _bmqConnfile_put()");
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

  /*更新连接记录内容*/
  ilRc = _bmqConnect_update(iMbid,&slMbconnect);
  if(ilRc == FAIL)
  {
    _bmqDebug("S2190 更新连接文件记录内容出错!");
    _bmqClosefile(fp);
    return(FAIL);
  }
  slMbconnect.lMsgid_recv = lMsgid;

  memcpy(alBuf + 1, (char *)&slMbconnect, ilSize - 1);

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (i-1)*ilSize, ilSize, alBuf);
  if (ilRc)
  {
     _bmqDebug("S2200 _bmqConnfile_put() 写文件出错");
     _bmqClosefile(fp);
     return (FAIL);
  }

  _bmqClosefile(fp);

  if( igDebug >= 2)
    _bmqDebug("S2210 **结束调用函数_bmqConnfile_put(%d)**",iMbid);

  return(SUCCESS);  
}

/**************************************************************
 ** 函数名: _bmqConnfile_set
 ** 功能:   连接缓存文件记录设置
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:cTag--启用标志  iLlink--左链  iRlink--右链
             aFilter--报文过滤条件  aMask--报文掩码
             iOffsetid--记录偏移量
             iFilterFlag            
 ** 返回值: SUCCESS,FAIL
***************************************************************/
static int _bmqConnfile_set(short cTag,short iLlink,short iRlink,char *aFilter,char *aMask,short iOffsetid,short iFilterFlag)
{
  int    ilRc,ilSize;
  char   alBuf[iFLDVALUELEN];
  struct mbconnect slMbconnect;
  FILE   *fp;

  if( igDebug >= 2)
    _bmqDebug("S2220 **调用函数 _bmqConnfile_set(..)连接缓存文件记录设置**");

  if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
  {
    _bmqDebug("S2230 打开文件出错");
    return (FAIL);
  }

  ilSize = sizeof(struct mbconnect) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (iOffsetid-psgMbshm->iMbcontcount-1)*ilSize, ilSize,alBuf);
  if (ilRc < 0)
  {
    _bmqDebug("S2240 读文件出错 _bmqConnfile_set()");
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
    _bmqDebug("S2250 _bmqConnfile_set() 写文件出错");
    return (FAIL);
  }
  _bmqClosefile(fp);

  if( igDebug >= 2)
    _bmqDebug("S2260 **结束调用函数_bmqConnfile_set(..)**");

  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqQueuefile_pick
 ** 功能:   从队列缓存文件中取一条指定记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:fp--文件指针  slMbqueue--结构指针  llOffset--记录偏移量
 ** 返回值: SUCCESS,FAIL
***************************************************************/
int _bmqQueuefile_pick(FILE *fp,struct mbqueue *slMbqueue,long llOffset)
{
  int ilRc,ilSize;
  char alBuf[iFLDVALUELEN];

  if( igDebug >= 2)
    _bmqDebug("S2270 **调用函数_bmqQueuefile_pick(fp,Mbqueue,%ld)**",llOffset);
 
   ilSize = sizeof(struct mbqueue) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (llOffset-psgMbshm->lMbqueuecount-1)*ilSize, 
    ilSize,alBuf);
  if (ilRc == FAIL)
  {
     _bmqDebug("S2280 读文件出错 _bmqQueuefile_pick()");
     return (FAIL);
  }
  if( igDebug >= 2 )
    _bmqDebug("S2290 **结束调用函数 _bmqQueuefile_pick(...)***");

  if(ilRc) return(ilRc);
  if(alBuf[0] == '0') return(WITHOUTUSE);
  memset(slMbqueue,0x00,sizeof(struct mbqueue));
  memcpy(slMbqueue,alBuf + 1,ilSize - 1);
  
  return(SUCCESS);
}
  
/**************************************************************
 ** 函数名: _bmqConnfile_pick
 ** 功能:   从链接缓存文件中取一条指定记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:fp--文件指针  slMbconnect--结构指针  llOffset--记录偏移量
 ** 返回值: SUCCESS,FAIL
***************************************************************/
int _bmqConnfile_pick(FILE *fp,struct mbconnect *slMbconnect,long llOffset)
{
  int ilRc,ilSize;
  char alBuf[iFLDVALUELEN];

  if( igDebug >= 2)
    _bmqDebug("S2300 **调用函数 _bmqConnfile_pick(*fp,Mbconnect,%ld)",llOffset);
  ilSize = sizeof(struct mbconnect) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (llOffset-psgMbshm->iMbcontcount-1)*ilSize, 
    ilSize,alBuf);
  if (ilRc == FAIL)
  {
     _bmqDebug("S2310 读文件出错 _bmqConnfile_pick()");
     return (FAIL);
  }
  if(ilRc) return(ilRc);
  if(alBuf[0] == '0') return(WITHOUTUSE);
  memset(slMbconnect,0x00,sizeof(struct mbconnect));
  memcpy(slMbconnect,alBuf + 1,ilSize - 1);
  if( igDebug >= 2)
    _bmqDebug("S2320 **结束调用函数 _bmqConnfile_pick(...)**");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqPackfile_put
 ** 功能:   报文缓存文件放入一条记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:paMsgpack--报文buf  iLoglen--报文长度  lFile_head--文件记录头
 ** 返回值: SUCCESS,FAIL
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
    _bmqDebug("S2330 **调用函数 _bmqPackfile_put(Msgpack,%d,File_head)**",iLoglen);

  if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
  {
    _bmqDebug("S2340 打开文件出错");
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
      _bmqDebug("S2350 读文件出错 _bmqFile_put()");
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
     _bmqDebug("S2360 写文件 _bmqPutlog()返回出错");
     _bmqClosefile(fp);
     return (FAIL);
   }

  memcpy(alBuf + 1, (char *)&slMbfilepack, ilSize - 1);

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqPutfile(fp, (i-1)*ilSize, ilSize, alBuf);
  if (ilRc)
   {
     _bmqDebug("S2370 _bmqPackfile_put() 写文件出错");
     _bmqClosefile(fp);
     return (FAIL);
   }

  _bmqClosefile(fp);
  if(igDebug >= 2)
  {
     _bmqDebug("S2380 保存报文到文件:邮件头=%d,logfile=%s,logoffset=%ld,iLoglen=%d ",*lFile_head,slMbfilepack.aLogfile,slMbfilepack.lLogoffset,slMbfilepack.lLoglen);
  }
  if( igDebug >= 2)
    _bmqDebug("S2390 **结束调用函数 _bmqPackfile_put(...)**");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqPackfile_pick
 ** 功能:   从报文缓存文件中取一条指定记录
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:fp--文件指针  slMbfilepack--结构指针  
             lPack_head--报文记录链首
 ** 返回值: SUCCESS,FAIL
***************************************************************/
int _bmqPackfile_pick(FILE *fp,struct mbfilepack *slMbfilepack,long lPack_head)
{
  int    ilRc,ilSize;
  char   alBuf[iFLDVALUELEN];

  if(igDebug >= 2)
    _bmqDebug("S2400 **调用函数 _bmqPackfile_pick(fp,Mbfilepack,%ld)**",lPack_head);

   ilSize = sizeof(struct mbfilepack) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (lPack_head-psgMbshm->lMbpacksize-1)*ilSize, ilSize,alBuf);
  if (ilRc == FAIL)
  {
    _bmqDebug("S2410 读文件出错 _bmqPackfile_pick");
    return (FAIL);
  }

  if(ilRc) return(ilRc);
  if(alBuf[0] == '0') return(WITHOUTUSE); 
  memset(slMbfilepack,0x00,sizeof(struct mbfilepack));
  memcpy(slMbfilepack,alBuf + 1,ilSize - 1);
   if(igDebug >= 2)
     _bmqDebug("S2420 **结束调用函数 _bmqPackfile_pick(..)**");
  return(SUCCESS);
}


/**************************************************************
 ** 函数名: _bmqPackfile_set
 ** 功能:   报文缓存文件记录设置
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:fp--文件指针 iTag--启用标志 iLoglen--报文长度
             lLogoffset--记录偏移量
 ** 返回值: SUCCESS,FAIL
***************************************************************/
/*int _bmqPackfile_set(FILE *fp,short iTag,short iLoglen,long lLogoffset)delete by wh*/
int _bmqPackfile_set(FILE *fp,short iTag,TYPE_SWITCH iLoglen,long lLogoffset)/*add by wh*/
{
  int ilRc,ilSize;
  char alBuf[iFLDVALUELEN];
  struct mbfilepack slMbfilepack;

  if(igDebug >= 2)
    _bmqDebug("S2430 **调用函数 _bmqPackfile_set(*fp,%d,%d,%ld)**",
      iTag,iLoglen,lLogoffset);
 ilSize = sizeof(struct mbfilepack) + 1;

 fseek(fp,0,SEEK_SET);
 ilRc = _bmqGetfile(fp, (lLogoffset-psgMbshm->lMbpacksize-1)*ilSize, ilSize,alBuf);
 if (ilRc < 0)
  {
    _bmqDebug("S2440 读文件出错 _bmqPackfile_set()");
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
    _bmqDebug("S2450 _bmqPutfile()返回出错!");
    return (FAIL);
   }

  if(igDebug >= 2)
    _bmqDebug("S2460 **结束调用函数 _bmqPackfile_set(...)**");
 return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqShmConnect
 ** 功能:   连接共享内存
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义    :
 ** 返回值: SUCCESS,错误码
***************************************************************/
int _bmqShmConnect()
{
  int ilRc;

  if( igDebug >= 2)
    _bmqDebug("S2470 ***调用函数  _bmqShmConnect()连接共享内存***");
  /*读入参数配置文件SHMKEY*/
  ilRc = _bmqConfig_load("SHMK",0);
  if (ilRc < 0)
  {
    _bmqDebug("S2480 configload(SHMK) error");
    return(-1185);
  }

  /*访问一个已经存在的共享内存区*/
  ilRc = shmget(lgSHMKEY_MB,0,IPC_EXCL);
  if (ilRc < 0)
  {
    _bmqDebug("S2490 shmget()出错errno:%d:%s",errno,strerror(errno));
    _bmqDebug("S2491 %s :载入SHMK参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    return(-1150);
  }
  igMb_sys_shm_id = ilRc;

  if (shmdt((void *)agShmtmp_d) &&
    igMbopenflag == 1 && lgCurrent_pid == getpid()) 
  {
    _bmqDebug("S5010: [错误/系统调用] shmdt()函数,errno=%d:%s",errno,strerror(errno));
    return(-1152);
  }

  agShmtmp_d = shmat(igMb_sys_shm_id,NULL,0);
  if (agShmtmp_d == (char *)-1)
  {
    _bmqDebug("S2500 shmat(..)出错 errno:%d:%s",errno,strerror(errno)); 
    return(-1151);
  }

  /*结构指针定位*/
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
    _bmqDebug("S2510 ***结束调用函数  _bmqShmConnect()****");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: _bmqShmDt
 ** 功能:   断开共享内存
 ** 作者:   徐军
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义    :
 ** 返回值: SUCCESS,错误码
***************************************************************/
int _bmqShmDt()
{
  int  ilRc;

  /*断开共享内存*/
  ilRc = shmdt ( (void *)agShmtmp_d);
  if ( ilRc )
  {
    _bmqDebug("S2518 断开共享内存出错  errno:%d:%s",errno,strerror(errno));
    return(-1152);
  }
  psgMbshm = (struct mbshm *)NULL;
  return(SUCCESS);
}

/**************************************************************
 ** 函数名:  _bmqDaemon_creat
 ** 功  能:  创建守护进程
 ** 作  者:  徐军
 ** 建立日期:2001/08/08 
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  SUCCESS , FAIL
***************************************************************/
int  _bmqDaemon_creat()
{
  pid_t pid, ppid;

  if(igDebug >= 2 )
    _bmqDebug("S2520 ***调用函数 _bmqDaemon_creat()创建守护进程***");

  /* 成为后台进程，且不是首进程（这是setsid必须的）*/
  ppid = getpid();
  pid = fork();
  if (pid < 0) return (FAIL);
  if (pid > 0) exit(0);  
  while(kill(ppid, 0) == 0)  
    if (igDebug >= 3)
      _bmqDebug("S2530 (Daemon(),fork1):父进程已经退出");  

  /* 创建新的进程组，并成为首进程，使之脱离原来的终端 */    
  ppid = getpid();
  setsid(); 
  signal(SIGHUP,SIG_IGN);
  
  /* 不是首进程，防止再与终端联系 */
  pid = fork();
  if (pid < 0) return (FAIL);
  if (pid > 0) exit(0);  
  while(kill(ppid, 0) == 0)  
    if (igDebug >= 3)
      _bmqDebug("S2540 (Daemon(),fork2):父进程已经退出");  
    
  /* 切断与原有文件系统的联系 */    
  chdir("/");
  
  /* 清除文件掩码是为了消除进程自身掩码对其创建文件的影响 */
  umask(0);  
  
  /* 防止子进程继承来文件而是这些始终保持打开从而产生某些冲突 */

  if(igDebug >= 2 )
    _bmqDebug("S2550 ***结束调用函数 _bmqDaemon_creat()***");

  return (SUCCESS);
}

/**************************************************************
 ** 函数名:  _bmqGetmsgid
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2001/08/08 
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
***************************************************************/
int _bmqGetmsgid(short iConnectid)
{
  int    ilRc;
  FILE   *fp;
  struct mbconnect slMbconnect;

  if(iConnectid > psgMbshm->iMbcontcount)
  {
    /*取出记录*/
    if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
    {
      _bmqDebug("S0260 打开连接缓存文件出错");
      return (FAIL);
    }
    _bmqConnfile_pick(fp,&slMbconnect,iConnectid);

    _bmqClosefile(fp);
    return(slMbconnect.lMsgid_recv);
  }
  return(psgMbconnect[iConnectid-1].lMsgid_recv);
}
/**************************************************************
 ** 函数名:  _bmqGetfrommsg
 ** 功  能:  
 ** 作  者:  nh
 ** 建立日期:2001/11/29 
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
***************************************************************/
int _bmqGetfrommsg(short iMbid,short iPrior,long lMsgid)
{
  int		ilRc;
  char		alFilter[iMASKLEN];
  struct	recvmsg	slRecvmsg;

  if(igDebug >= 2)
  {
    _bmqDebug("S8800 ****调用函数bmqGetfrommsg(%d,%d,%ld)****",
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
          _bmqDebug("S8801 从消息队列中没有读到内容");
      }
      break;	
    }
    else
    {
      memcpy(alFilter,psgMbqueue[slRecvmsg.lQueueid-1].aFilter,iMASKLEN);  
      ilRc = _bmqConn_select(iMbid,iPrior,alFilter,slRecvmsg.lQueueid);
      if(ilRc)
      {
        _bmqDebug("S8802 ****调用函数_bmqConn_select()出错");
        return(FAIL);
      }
    }
  }
  return(SUCCESS);
}
/**************************************************************
 ** 函数名:  _bmqSendstat
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/07/28 
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
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
 ** 函数名:  _bmqPutindexfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
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
    _bmqDebug("S3100: 读系统环境变量[BMQ_PATH]失败");
    return(FAIL);
  }
 
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,BMQINDEXFILE);
  
  if ((fp = fopen(alFile, "r+")) == NULL)
  {
    if ((fp = fopen(alFile, "w+")) == NULL)
    {
      _bmqDebug("S3110: 文件打开失败:%d,%s",errno,strerror(errno));
      return(FAIL);
    } 
  }
  fflush(fp);
  ilRc = fseek(fp, (lSerialno-1)*sizeof(struct indexfile), SEEK_SET);
  if (ilRc)
  {
    _bmqDebug("S3120: 文件定位出错errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  
  ilRc = fwrite((char *)&slIndexfile, sizeof(char), sizeof(struct indexfile), fp);
  if (ilRc != sizeof(struct indexfile))
  {    
    _bmqDebug("S3130: 写文件出错errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  
  fclose(fp);
  return(SUCCESS);	
}	

/**************************************************************
 ** 函数名:  _bmqGetindexfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
***************************************************************/
int _bmqGetindexfile(long lSerialno,char *aFilename)
{
  int		ilRc;
  char		alFile[80],*alPath;
  FILE		*fp;
  struct	indexfile	slIndexfile;
  
  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S3140: 读系统环境变量[BMQ_PATH]失败");
    return(FAIL);
  }
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,BMQINDEXFILE);

  if ((fp = fopen(alFile, "r")) == NULL)
  {
    _bmqDebug("S3150: 索引文件打开失败:%d,%s",errno,strerror(errno));
    return(FAIL);
  }
    
  ilRc = fseek(fp, (lSerialno-1)*sizeof(struct indexfile), SEEK_SET);
  if (ilRc)
  {
    _bmqDebug("S3160: 文件定位出错errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  fflush(fp);
  
  ilRc = fread((char *)&slIndexfile, sizeof(char), sizeof(struct indexfile), fp);
  if (ilRc != sizeof(struct indexfile))
  {
    _bmqDebug("S3170: 读索引文件出错errno:%d:%s",errno,strerror(errno));
    fclose(fp);
    return (FAIL);
  }
  
  strcpy(aFilename,slIndexfile.aFilename);
  fclose(fp);
  return(SUCCESS);	
}	

/**************************************************************
 ** 函数名:  bmqSendfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
***************************************************************/
int bmqSendfile(short iGrpid,char *aFilename,struct transparm sTransparm,
                long *lSerialno)
{
  int	ilRc;
  
  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S3180:%d",ilRc);
    return(ilRc);
  }
 
  /* 生成文件唯一索引号 */
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  if(psgMbshm->lFileno >= MAXFILENO)
   psgMbshm->lFileno = 1;
  else
   psgMbshm->lFileno++;
  _bmqUnlock(LK_ALL);
  
  /* 写索引文件记录 */ 
  ilRc = _bmqPutindexfile(psgMbshm->lFileno,aFilename);
  if( ilRc )
  {
    _bmqDebug("S3190: 写索引记录文件错误,ilRc=%d,Fileno=%ld,Filename=%s",
               ilRc,psgMbshm->lFileno,aFilename);
    return(FAIL);               
  }
  *lSerialno = psgMbshm->lFileno;
  /* _bmqDebug("S3200: *lSerialno=%ld",*lSerialno); */
  return(SUCCESS);	
}	

/**************************************************************
 ** 函数名:  bmqRecvfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
***************************************************************/
int bmqRecvfile(long lSerialno,FILE **fp)
{
  int	ilRc;  
  char  alFilename[80];
  
  /* 取出文件索引记录 */   
  ilRc = _bmqGetindexfile(lSerialno,alFilename);
  if( ilRc )
  {
    _bmqDebug("S3210: 取出索引记录文件错误,ilRc=%d,Fileno=%ld,Filename=%s",
               ilRc,lSerialno,alFilename);
    *fp = NULL;               
    return(FAIL);               
  }

  if ((*fp = fopen(alFilename, "r")) == NULL)
  {
    _bmqDebug("S3220: 文件[%s]打开失败:%d,%s",alFilename,errno,strerror(errno));
    *fp = NULL;
    return(FAIL); 
  }
  
  return(SUCCESS);	
}	

/**************************************************************
 ** 函数名:  bmqPutfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
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
      _bmqDebug("S3230: 生成文件索引记录失败!");
      return(FAIL);
    }
  }
  
  ilRc = bmqPut(iGrpid,iMbid,iPrior,lType,llSerialno,aMsgbuf,iMsglen);
  if(ilRc)
  {
    _bmqDebug("S3240: [函数调用失败]bmqPut error!");
    return(FAIL);	
  }
  
  return(SUCCESS);	
}

/**************************************************************
 ** 函数名:  bmqGetfile
 ** 功  能:  
 ** 作  者:  徐军
 ** 建立日期:2002/12/18
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:  
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
    _bmqDebug("S3250: [函数调用失败]bmqGetw error! ilRc=%d",ilRc);
    *fp = NULL;
    return(FAIL);
  }
    
  if(*plClass)
  {
    /* _bmqDebug("fzj2: *plClass=%ld",*plClass); */
    ilRc = bmqRecvfile(*plClass,fp);	
    if(ilRc)
    {
      _bmqDebug("S3260: [函数调用失败]bmqRecvfile() error");
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
    _bmqDebug("S3270: [函数调用失败]bmqGetw error! ilRc=%d",ilRc);
    aFileName = NULL;
    return(ilRc);
  }
    
  if(*plClass)
  {
    ilRc = _bmqGetindexfile(*plClass,aFileName);
    if( ilRc )
    {
      _bmqDebug("S3280: 取出索引记录文件错误,ilRc=%d,Fileno=%ld",
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
    _bmqDebug("S3281: 无法打开指定文件[%s]!",aFilename);
    sgMonMsg.iSendFlag  = 5;
    strcpy(sgMonMsg.aFileName,aFilename);
    strcpy(sgMonMsg.aStatusDesc,"无法打开指定文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }

  /* add by xujun 2007.01.11
     发送前检测病毒(如果邮箱设置了发送检测) */
  if( psgMbinfo[igMb_sys_current_mailbox - 1].iSendScanFlag )
  { 
    memset(alVirName,0x00,sizeof(alVirName));
    ilRc = _bmqVirusScanByRoot(aFilename,sgRoot,alVirName);
    if(ilRc)
    {
      _bmqDebug("S3282: 文件[%s]病毒扫描失败!",aFilename);
      sgMonMsg.iSendFlag  = 5;
      strcpy(sgMonMsg.aFileName,aFilename);
      strcpy(sgMonMsg.aStatusDesc,"病毒扫描失败!");
      _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
      return(FAIL);
    }
    else if(strlen(alVirName))
    {
      _bmqDebug("S3283: 文件[%s]检测出病毒[%s]!",aFilename,alVirName);
      sgMonMsg.iSendFlag  = 5;
      strcpy(sgMonMsg.aFileName,aFilename);
      sprintf(sgMonMsg.aStatusDesc,"发现病毒[%s]",alVirName);
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
        _bmqDebug("S3290: 生成文件索引记录失败!");
        return(FAIL);
      }
    }
  }

  /* add by xujun 2006.11 begin
     支持跨组文件传输,将文件传送请求发到指定的组服务模块
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
    _bmqDebug("S3300: 跨组文件传输失败,ilRc=[%d]",ilRc);
    /* 向SAF模块发送失败报文 */
    memset(alMsgbuf,0x00,iMBMAXPACKSIZE);
    memcpy(alMsgbuf,(char *)&sgMonMsg,sizeof(struct monmsg));
    memcpy(alMsgbuf+sizeof(struct monmsg),aMsgbuf,iMsglen);
    bmqPut(0,iMBFILESAF,iPrior,lType,1,alMsgbuf,sizeof(struct monmsg)+iMsglen);
    return(ilRc);
  }
  /* add by xujun 2006.11 end
     支持跨组文件传输 */

  ilRc = bmqPut(iGrpid,iMbid,iPrior,lType,llSerialno,aMsgbuf,iMsglen);
  if(ilRc)
  {
    _bmqDebug("S3310: [函数调用失败]bmqPut error!");
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
    _bmqDebug("S3320: [函数调用失败]bmqRecvfileByName() 失败");
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
  	_bmqDebug("S3328: 无法打开指定文件[%s]!",aFilename);
  	sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aFileName,aFilename);
    strcpy(sgMonMsg.aStatusDesc,"无法打开指定文件");
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
    _bmqDebug("S3330: 文件传输失败,ilRc=[%d]",ilRc);
    /* 向SAF模块发送失败报文 */
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
    _bmqDebug("S3340: [函数调用失败]bmqGetw error! ilRc=%d",ilRc);
    return(FAIL);
  }
  if(*plClass)
  {
    /* add by xujun 2007.01.11
       发送前检测病毒(如果邮箱设置了接收检测) */
    if( psgMbinfo[igMb_sys_current_mailbox - 1].iRecvScanFlag )
    { 
      /*取出文件*/
      _bmqGetindexfile(*plClass,alFilename);
      memset(alVirName,0x00,sizeof(alVirName));
      memset(&sgMonMsg,0x00,sizeof(struct monmsg));
      ilRc = _bmqVirusScanByRoot(alFilename,sgRoot,alVirName);
      if(ilRc)
      {
        _bmqDebug("S3341: 文件[%s]病毒扫描失败!",alFilename);
        sgMonMsg.iSendFlag  = 5;
        strcpy(sgMonMsg.aFileName,alFilename);
        strcpy(sgMonMsg.aStatusDesc,"病毒扫描失败!");
        _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
        return(FAIL);
      }
      else if(strlen(alVirName))
      {
        _bmqDebug("S3342: 文件[%s]检测出病毒[%s]!",alFilename,alVirName);
        sgMonMsg.iSendFlag  = 5;
        strcpy(sgMonMsg.aFileName,alFilename);
        sprintf(sgMonMsg.aStatusDesc,"发现病毒[%s]",alVirName);
        _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
        return(FAIL);
      }
    }
    ilRc = bmqRecvfileX(*plClass,aFileName);	
    if(ilRc)
    {
      _bmqDebug("S3350: [函数调用失败]bmqRecvfileX() error");
      aFileName = NULL;
      return(FAIL);
    }
  }  	
  return(SUCCESS);	
}

/**************************************************************
 ** 函数名:  _bmqSendfile_G
 ** 功能:    发送文件到跨组Server
 ** 作者:    徐军 	
 ** 建立日期: 2006/11/20
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0--成功
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
  /* 本组传输直接拷贝文件到send目录 */
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
    strcpy(sgMonMsg.aStatusDesc,"本组文件发送完成");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
  	return(SUCCESS);
  }

  /* 连接到服务器 */
  ilSockfd = _bmqConnectfileser(iGrpid);
  if ( ilSockfd < 0 )
  {
    _bmqDebug("S3360: --%d--连接到服务器出错!",ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"连接服务器失败");
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

  /* add by xujun 2006.11.15 支持断点续传 begin */
  memset(alOffset,0x00,sizeof(alOffset)); 
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  _bmqfilechr(aFilename,strlen(aFilename),alFile,'/');
  sprintf(alTmpFileName,"%s/temp/%s.tpf",getenv("BMQ_FILE_DIR"),alFile);
  llOffset = 0;
  if (( fq = fopen(alTmpFileName,"r")) != NULL)
  {
    /*文件存在,上次传送未完成,取出上次传送点偏移量*/
    ilRc = fread(alOffset,sizeof(char),sizeof(alOffset),fq);
    llOffset = atol(alOffset);
    fclose(fq);
  }
  /* add by xujun 2006.11.15 支持断点续传 end */

  /* 读取参数,是否需要加密和压缩 */
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"COMPRESSFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 COMPRESSFLAG 字段失败!",alInitfile);
    alResult[0] = '0'; /*缺省为不压缩*/
  }
  strcpy(alCompressFlag,alResult);
  ilRc = _bmqGetfield(alInitfile,"CRYPTFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: 从文件%s中取 CRYPTFLAG 字段失败!",alInitfile);
    alResult[0] = '0'; /*缺省为不加密*/
  }
  strcpy(alCryptFlag,alResult);
  /* 组织命令报文 */
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

  /* 发送开始发送文件命令报文到服务器 */ 
  if(llOffset == 0)
  {
    strcpy(sgMonMsg.aStatusDesc,"开始发送文件");
    sgMonMsg.iSendFlag  = 1;
  }
  else
  {
  	strcpy(sgMonMsg.aStatusDesc,"继续发送文件");
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
    strcpy(sgMonMsg.aStatusDesc,"开始发送请求失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }

  /* 接收响应报文 */
  ilMsglen = sizeof(alMsgbuf);
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S3380: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"接收开始发送请求响应报文失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }

  if(memcmp(alMsgbuf,"OK",2))
  {
    _bmqDebug("S3390: 服务器拒绝接收文件,retbuf=[%s]",alMsgbuf);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"服务器拒绝接收文件");
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

  /* 调用函数指针完成文件预处理和发送 */
  ilRc = sTransparm.bmqFile_msg(alParm);
  if(ilRc)
  {
    _bmqDebug("S3400: 文件信息交互处理函数错误，退出发送进程!");
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"文件预处理失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(ilRc);
  }

  ilRc = sTransparm.bmqFile_comm(alParm);
  if(ilRc)
  {
    _bmqDebug("S3410: 文件发送发生错误,退出发送进程!请查明错误原因后重新发送.");
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"文件发送失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(ilRc);
  }
   
  /* 组织命令报文 */
  ilMsglen = 9;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 9;			/* 命令标志 9=Sendfile end */  

  /* 发送结束传送标志给服务器 */
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S3420: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"发送结束传送标志失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }
  
  /* 接收响应报文 */
  ilMsglen = 2;
  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S3430: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    strcpy(sgMonMsg.aStatusDesc,"发送结束响应接收失败");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return (FAIL);
  }
  if(memcmp(alMsgbuf,"OK",2))
  {
    _bmqDebug("S3440: 接收方未能正确接收文件[%s],需要重发!",aFilename);
    ilMsglen = 16;
    ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
    if ( ilRc != 0 ) 
    {
      _bmqDebug("S3450: Receive from server error!!--%d",ilRc);
      close(ilSockfd);
      sgMonMsg.iSendFlag  = 4;
      strcpy(sgMonMsg.aStatusDesc,"发送结束响应接收失败");
      _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
      return (FAIL);
    }
    alMsgbuf[ilMsglen] = 0x0;
    if (( fq = fopen(alTmpFileName,"w+")) != NULL)
    {
      /*写入传送点偏移量*/
      ilRc = fwrite(alMsgbuf,sizeof(char),ilMsglen,fq);
      fclose(fq);
    }
    
    close(ilSockfd);
    sgMonMsg.iSendFlag  = 4;
    sgMonMsg.lSendSize  = atol(alMsgbuf);
    strcpy(sgMonMsg.aStatusDesc,"接收方未能正确接收文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  }

  close(ilSockfd);
  sgMonMsg.iSendFlag  = 3;
  sgMonMsg.lSendSize  = sgMonMsg.lFileSize;
  strcpy(sgMonMsg.aStatusDesc,"文件发送完成");
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
 ** 函数名:bmqPutSavePack
 ** 功能:  存放关闭时保存的报文
 ** 作者:  徐军
 ** 建立日期: 2007/01/09
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iOrgGrpid-源发组  iOrgMbid-源发邮箱  iMbid-目标邮箱  iPrior-优先级
             aFilter-报文过滤条件 aMsgbuf-报文Buf iMsglen-报文长度 
 ** 返回值: SUCCESS
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
    _bmqDebug("S3460 ***调用函数 bmqPutSavePack(%d,%d,%d,%d,aFilter,aMsgbuf,%d)***",
      iOrgGrpid,iOrgMbid,iMbid,iPrior,iMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    
  }

  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() ) 
  {
    _bmqDebug("S3470 bmqPutSavePack() 没有打开邮箱就执行操作!!");
    return(1011);
  }
 
  /*判断参数合法性*/
  if( iOrgGrpid < 0) return(1009);
  if( (iMbid > psgMbshm->iMbinfo) || (iMbid <= 0) ) return(1004);
  if( (iPrior < 0) || (iPrior > 99) ) return(1010);

   /*判断是否已经超过指定阀值*/
  if( psgMbinfo[iMbid-1].lMaxqueue && psgMbinfo[iMbid-1].lPendnum >= 
    psgMbinfo[iMbid-1].lMaxqueue )
  {
    return(1008);
  }
  
  /*判断报文长度是否超过最大值*/
  if(iMsglen > iMBMAXPACKSIZE)
  {
    _bmqDebug("S3480 报文长度[%d]超过最大值－ [%d]",iMsglen,iMBMAXPACKSIZE);
    return(1012);
  }

  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);

  /*存储报文*/
  ilRc = _bmqPack_put(aMsgbuf,iMsglen,&llPack_head);
  if (ilRc)
  {
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S3490 函数bmqPutSavePack()存入的报文内容aMsgbuf为%s",aMsgbuf);
  }

  /*存储队列*/
  ilRc = _bmqQueue_put( 0, iMbid, iPrior, aFilter,llPack_head,&llQueue_id);
  if (ilRc)
  {
    _bmqDebug("S3500 存储队列_bmqQueue_put(...)出错!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  /*修改邮箱信息*/
  psgMbinfo[iMbid-1].lPendnum++;
  /*修改队列信息*/
  psgMbqueue[llQueue_id-1].iOrg_grp     = iOrgGrpid;
  psgMbqueue[llQueue_id-1].iOrg_mailbox = iOrgMbid;

  _bmqUnlock(LK_ALL);

  if(igDebug >= 1)
  {
    _bmqDebug("S3510 组[%d]邮箱[%d]往邮箱[%d]存入[%d]报文",
      iOrgGrpid,iOrgMbid,iMbid,llPack_head); 
  }
  if( igDebug >= 2)
  {
    _bmqDebug("S3520 ***结束调用函数 bmqPutSavePack(%d,%d,%d,%d,aFilter,aMsgbuf,%d)***",
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
      _bmqDebug("S0470: 不能打开文件[%s]",sendmulfile.aFileName[i]);
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
     _bmqDebug("S0610: [函数调用失败]bmqGetFile error!");
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
 ** 函数名:bmqSend
 ** 功能:  存放报文,无需指定目标,根据报文关键字自动路由
 ** 作者:  徐军
 ** 建立日期: 2007/01/25
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iPrior-优先级 iType-报文Type  iClass-报文Class 
             aMsgbuf-报文Buf iMsglen-报文长度 
 ** 返回值: SUCCESS
***************************************************************/
/*int bmqSend(short iPrior,long lType,long lClass,char *aMsgbuf,short iMsglen) delete by wh*/
int bmqSend(short iPrior,long lType,long lClass,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int    ilRc;
  short  ilResultlen;               /*结果长度*/
  short  ilGrpid;                   /*目标组*/
  short  ilMbid;                    /*目标邮箱*/
  char   alExpress[iFLDVALUELEN];   /*特征码表达式*/
  char   alTmpexp[iFLDVALUELEN];    /*逆波兰表达式*/
  
  if(strlen(psgMbinfo[igMb_sys_current_mailbox - 1].aExpress) < 3)
  {
    _bmqDebug("S8010 邮箱[%d]不支持自动路由,特征码表达式=[%s]",
      igMb_sys_current_mailbox,psgMbinfo[igMb_sys_current_mailbox - 1].aExpress);
    return(FAIL);
  }
  
  /*计算报文特征码*/
  memset(alTmpexp,0x00,sizeof(alTmpexp));
  agMsgbody    = aMsgbuf;
  igMsgbodylen = iMsglen;
  /*转换为逆波兰表达式*/
  ilRc = _swExpN2T(psgMbinfo[igMb_sys_current_mailbox - 1].aExpress,alTmpexp);
  if(ilRc) 
  {
    _bmqDebug("S8020 [%s]转换为逆波兰表达式失败",alExpress);
    return(FAIL);
  }
  _bmqVdebug(1,"S8030 逆波兰表达式=[%s]",alTmpexp);
  
  /*计算平台表达式*/
  memset(alExpress,0x00,sizeof(alExpress));
  ilRc = _swExpress(alTmpexp,alExpress,&ilResultlen);  
  if(ilRc)
  {
    _bmqDebug("S8040 _swExpress计算平台表达式出错");
    return(FAIL);
  }
  _bmqVdebug(1,"S8050 报文特征码=[%s]",alExpress);
  
  /*根据特征码提取目标组和目标邮箱*/
  ilRc = _bmqGetRouter(igMb_sys_current_mailbox,alExpress,&ilGrpid,&ilMbid);
  if(ilRc)
  {
    _bmqDebug("S8060 根据源邮箱[%d]的特征码[%s]提取路由失败",igMb_sys_current_mailbox,alExpress);
    return(FAIL);
  }
  _bmqVdebug(1,"S8070  目标组=[%d],目标邮箱=[%d]",ilGrpid,ilMbid);

  return(bmqPut(ilGrpid,ilMbid,iPrior,lType,lClass,aMsgbuf,iMsglen));
}
/* add by xujun 2007.01.26 for msg route end */


/* add by hxz 2007.02.28 for get msg content Begin */
/**************************************************************
 ** 函数名:bmqJava_getfilter
 ** 功能:  提取报文(非阻塞)
 ** 作者:  
 ** 建立日期: 
 ** 最后修改日期:
 ** 调用其它函数:_bmqQueue_get,_bmqPack_get
 ** 全局变量:
 ** 参数含义:piGrpid-源发组       piMbid-源发邮箱  piPrior-优先级
             aFilter-报文过滤条件 aMask-报文掩码   aMsgbuf-报文Buf
             piMsglen-报文长度
 ** 返回值: SUCCESS,错误码
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
    _bmqDebug("S0420 ***调用函数:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      piGrpid,piMbid,piPrior,piMsglen);
     printf("***调用函数:bmqGet(%d,%d,%d,%s,%s,%s,%d)***\n",*piGrpid,*piMbid,*piPrior,aFilter,aMask,aMsgbuf,*piMsglen);
    _bmqDebughex(aFilter,iMASKLEN);
    _bmqDebughex(aMask,iMASKLEN);    
  }
  printf("arm 2\n");
  /* 判断当前进程是否打开了邮箱 */
  if( igMbopenflag != 1 || lgCurrent_pid != getpid() )
   {
      _bmqDebug("S0510 bmqGet() 没有打开邮箱就执行操作!!");
      return(1011);
    }
  printf("arm 3\n");
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  printf("arm 4\n");
  /*查找邮箱队列记录,取出报文链接头*/
  ilRc = _bmqQueue_javaget(piGrpid,piMbid,piPrior,aFilter,aMask,&llPack_head);
  printf("arm 5\n");
  printf("ilRc=%d\n",ilRc);
  printf("llPack_head=%ld\n",llPack_head);
  if (ilRc)
  {
    if(ilRc != 1001)  	
      _bmqDebug("S0430 取出报文链接头出错!");
    /*_bmqUnlock(LK_ALL);*/
    /*return(ilRc);*/
  }
 
  /*修改邮箱信息*/
  /*
  psgMbinfo[igMb_sys_current_mailbox-1].lPendnum--;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvnum++;
  psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid = getpid();
  time(&(psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime));
  */
  /*提取报文*/
  printf("arm 6\n");
  printf("llPack_head=%d\n",llPack_head);
  ilRc = _bmqPack_javaget(aMsgbuf,piMsglen,llPack_head);
  printf("aMsgbuf=%s\n",aMsgbuf);
  printf("arm 7\n");
  printf("ilRc=%d\n",ilRc);
  if( ilRc )
  {
    _bmqDebug("S0440 提取报文ERROR!");
    _bmqUnlock(LK_ALL);
    return(ilRc);
  }

  /*_bmqUnlock(LK_ALL);
  if(igDebug >= 1)
  {
    _bmqDebug("S0450 邮箱[%d]取走组[%d]邮箱[%d]的[%d]报文",
      igMb_sys_current_mailbox,*piGrpid,*piMbid,llPack_head);
  }
  */

  if(igDebug >= 3)
  {
    _bmqDebug("S0460 邮箱[%d]修改后信息lPendnum[%d],lRecvpid[%d],lRecvtime[%d]",
      igMb_sys_current_mailbox,
      psgMbinfo[igMb_sys_current_mailbox-1].lPendnum,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvpid,
      psgMbinfo[igMb_sys_current_mailbox-1].lRecvtime);

  }

  if(igDebug >= 2)
  {
    _bmqDebug("S0470 ***结束调用函数:bmqGet(%d,%d,%d,aFilter,aMask,aMsgbuf,%d)***",
      *piGrpid,*piMbid,*piPrior,*piMsglen);
  }
  if(igDebug >= 3)
  {
    _bmqDebug("S0480 ***函数bmqGet取出的报文内容aMsgbuf为***");
    _bmqDebug("S0490 %s",aMsgbuf); 
  }

  return(SUCCESS);
}

/* add by hxz 2007.02.28 for get msg content end */

/* add by hxz 2007.02.28 for get msg content begin */
/**************************************************************
 ** 函数名: _bmqQueue_javaget
 ** 功能:   提取一条队列记录
 ** 作者:   
 ** 建立日期: 
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:piGrpid--组号   piMbid--目标邮箱?piPrior--优先级 
             aFilter--报文过滤条件  aMask--报文掩码   
             plPack_head--报文链表头
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S1180 ***调用提取一条队列记录函数:_bmqQueue_get(%d,%d,%d,aFilter,aMask)***",
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
  /* 取出邮箱消息链表头 */
  llQueue_head = psgMbinfo[igMb_sys_current_mailbox - 1].lQueue_head;
  ilSize       = sizeof(struct mbqueue) + 1;
  
  for(;;)
  {
  	printf("llQueue_head=%ld\n",llQueue_head);
    /* 未找到指定记录 */
    if (llQueue_head == 0)
    {
    	printf("starwolf\n");
      return(1001);
    }
    /* 记录在缓存文件中 */
    if (llQueue_head > psgMbshm->lMbqueuecount)
    {
    	printf("记录在缓存文件中\n");
      /* 打开队列缓存文件 */
      if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
      {
        _bmqDebug("S1190 打开文件出错");
        return (FAIL);
      }
      ilRc = _bmqQueuefile_pick(fp,&slMbqueue,llQueue_head);
      if (ilRc == FAIL)
      {
        _bmqDebug("S1200 队列缓存文件中取记录失败!");
        _bmqClosefile(fp);
        _bmqUnlock(LK_ALL);
        return(FAIL);
      }

      /* 匹配消息 Filter,Mask */
      
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
          _bmqDebug("S1208 删除的队列[%d]没有启用",llQueue_head);
          return (FAIL);
        }
        */
        /* 重写文件 */
        /*
        alBuf[0] = '0';
        fseek(fp,0,SEEK_SET);
        ilRc = _bmqPutfile(fp, (llQueue_head-psgMbshm->lMbqueuecount-1)*ilSize, 
          ilSize,alBuf);
        if (ilRc)
        {
          _bmqDebug("S1210 写文件出错");
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
    /* 记录在共享内存中 */
    else
    {
      /* 匹配消息 Filter,Mask */
      printf("记录在共享内存中\n");
      ilRc = _bmqStrand(aFilter, aMask, iMASKLEN, alResult1);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      printf("开始取内存记录\n");
      ilRc = _bmqStrand(psgMbqueue[llQueue_head-1].aFilter,aMask,iMASKLEN,alResult2);
      if (ilRc !=SUCCESS)
      {  
        _bmqDebug("S0000: _bmqStrand error!");
        return(FAIL);
      }
      printf("取内存记录结束\n");
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
          _bmqDebug("S0672 删除的队列[%d]没有启用",llQueue_head);
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
    _bmqDebug("S1220 ***结束调用函数:_bmqQueue_get(...)***");
  }
  return(SUCCESS);
}
/* add by hxz 2007.02.28 for get msg content end */

/* add by hxz 2007.02.28 for get msg content begin */
/**************************************************************
 ** 函数名: _bmqPack_javaget
 ** 功能:   提取一条报文记录
 ** 作者:   
 ** 建立日期: 
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:aMsgbuf--报文存储buf iMsglen--报文长度
             lPack_head--报文存储记录链表头
 ** 返回值: SUCCESS--成功，FAIL--失败
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
    _bmqDebug("S1090 ***调用函数:_bmqPack_get() lPack_head=%d***",lPack_head);

  /* add by xujun 2001.10.16*/
  printf("%ld\n",lPack_head);
  if (lPack_head <= 0)
  {
  	printf("非法的报文头值\n");
    _bmqDebug("S0191 非法的报文头值:%ld",lPack_head);
    return(FAIL);
  }
  /* add end by xujun*/

  if (lPack_head > psgMbshm->lMbpacksize )
  {
  	printf("取出报文存储索引记录\n");
    /*取出报文存储索引记录*/
    if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
    {
      _bmqDebug("S1100 打开文件出错!");
      return (FAIL);
    }
    /* 从报文缓存文件中取一条指定记录 */
    _bmqPackfile_pick(fp,&slMbfilepack,lPack_head);

    *piMsglen = slMbfilepack.lLoglen;
    /* 从报文存储文件中提取一条报文存储记录 */
    ilRc =  _bmqGetlog(slMbfilepack.aLogfile,*piMsglen,
      slMbfilepack.lLogoffset,aMsgbuf);
    if (ilRc) 
    {
      _bmqClosefile(fp);
      return(ilRc);
    }
    printf("hxz!!!!!!!!!!\n");
    printf("%s\n",aMsgbuf);
    /*更新报文存储索引记录*/
    _bmqPackfile_set(fp,0,0,lPack_head);
    _bmqClosefile(fp);

    if(igDebug >= 2)
    {
      _bmqDebug("S1110 从文件中取报文:file=[%s],offset=[%ld],len=[%d]",
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
      _bmqDebug("S1120 从内存中取报文:ID=[%d],len=[%d]",i,psgMbpack[i-1].iMsglen);
    }
    psgMbpack[i-1].iMsglen = 0;
    i = psgMbpack[i-1].lLink;

    if (i > psgMbshm->lMbpacksize)
    {
      /*取出报文存储索引记录*/
      if ((ilRc = _bmqOpenfile(1, &fp)) != SUCCESS)
      {
        _bmqDebug("S1130 打开文件出错");
        return (FAIL);
      }
      _bmqPackfile_pick(fp,&slMbfilepack,i);

      ilRc =  _bmqGetlog(slMbfilepack.aLogfile,slMbfilepack.lLoglen,
      slMbfilepack.lLogoffset,aMsgbuf+*piMsglen);
      if (ilRc)
      {
        _bmqDebug("S1140 从记录文件中提取报文出错,_bmqGetlog(..)");
        _bmqClosefile(fp);
        return(ilRc);
      }
      /*更新报文存储索引记录*/
      _bmqPackfile_set(fp,0,0,i);
      _bmqClosefile(fp);

      *piMsglen = *piMsglen + slMbfilepack.lLoglen;

      if(igDebug >= 2)
      {
        _bmqDebug("S1150 从文件中取报文:file=[%s],offset=[%ld],len=[%d]",
          slMbfilepack.aLogfile,
            slMbfilepack.lLogoffset,
              slMbfilepack.lLoglen);
      }
      if (igDebug >= 2) _bmqDebug("S1160 ***结束调用函数:_bmqPack_get()***");
      return(SUCCESS);
    }

    if (i == 0) break;
  }
  if (igDebug >= 2) _bmqDebug("S1170 ***结束调用函数:_bmqPack_get()***");
  return(SUCCESS);
}
/* add by hxz 2007.02.28 for get msg content end */

/**************************************************************
 ** 函数名:NbPut
 ** 功能:  存放报文
 ** 作者: 史正烨
 ** 建立日期: 2000/10/25
 ** 最后修改日期:2001/08/31
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:iGrpid-目标组   iMbid-目标邮箱   iPrior-优先级
             iType-报文Type  iClass-报文Class aMsgbuf-报文Buf
             iMsglen-报文长度 
 ** 返回值: SUCCESS
***************************************************************/
/***************************************************************
实现网银集群间交易的跨组传送
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
 ** 函数名: _bmqGrpcontser
 ** 功能:   连接服务器
 ** 作者:
 ** 建立日期: 2001/08/08
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:alIp--服务器IP地址  ilPort--通讯端口
 ** 返回值: 0--成功
***************************************************************/
static int _bmqGrpcontser(char *alIp,short ilPort,char *aSendIp,long ilTime)
{
  int    sockfd = -1;
  struct sockaddr_in slServ_addr;
  struct sockaddr_in sCli_addr;
  
  int  ilRc,flag;
 
  /* 创建套接字 */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    _bmqDebug("S0170 %s : Creat Socket Error! :errno: %d,%s",__FILE__,errno,strerror(errno));
    return (-1050);
  }
  memset((char *)&sCli_addr,0x00,sizeof(struct sockaddr_in));
  _bmqTrim(aSendIp);
  _bmqDebug("S0180 使用 IP[%s]发送",aSendIp);
  if (strlen(aSendIp)>0) {
	  sCli_addr.sin_family = AF_INET;
	  sCli_addr.sin_addr.s_addr = inet_addr(aSendIp);
	  
	  if (bind(sockfd,(struct sockaddr*)&sCli_addr,sizeof(struct sockaddr_in)) < 0)
	  {
	    _bmqDebug("bind IP[%s] socket出错[%d]\n",aSendIp,errno);
	    return(-1050);
	  }
  }
  /*add by pc 20060329 增加线路标志位
  ilRc=setsockopt(sockfd,IPPROTO_IP,IP_TOS,(char*)&igLineNum,sizeof(igLineNum));
  if(ilRc<0)
  {
    _bmqDebug("S0171: setsockopt IPPROTO_IP,IP_TOS [%d ]error",igLineNum);
    close(sockfd);
    return (-1060);
  }
  add by pc 20060329 增加线路标志位*/  
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

  /*连接通讯端口*/
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

/*20060220 设置连接参数*/
/*设置服务端IP*/
int NbSetIp(char *ip)
{
	memset(agIp,0x00,sizeof(agIp));
	strcpy(agIp,ip);
	return 0;
}
/*设置本地IP*/
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
 ** 函数名: _comGrpSend
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

