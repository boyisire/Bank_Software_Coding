#ifndef  _BMQH_
#define  _BMQH_ 1
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/timeb.h>
#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <math.h>
#include <pthread.h>
#include <stdarg.h>
#include "clamav.h"


#include "bmqtype.h"   /* add by wanghao */

#define SUCCESS        0
#define FAIL          -1
#define FINISH         2
#define WITHOUTUSE     3
#define OFFSETNULL     0
#define OVERFIELD      100
#define iFLDVALUELEN   512
#define SHMFILEEND     1
#define SHMNOTFOUND    100     /*没找到共享内存记录 */
/*  delete by wh #define iMBMAXPACKSIZE 8192    最大报文长度       */
#define iMBMAXPACKSIZE 65536    /*最大报文长度      add by wh */ 
#define DEFINE_THREADS 10      /*缺省线程数         */

#define iMBPERPACKSIZE 1024    /*单个队列存储区     */
#define iSEMNUM        2       /*信号量             */
#define LK_ALL         1       /*封锁进程的控制量   */
#define LK_ALL_2         2       /*封锁进程的控制量   */

#define iMASKLEN       20      /*Mask,Filter的长度  */

#define FILEKEY           "BMQPWD0"
#define SERVERCFGFILE     "/etc/bmq.ini"               /*服务器端配置文件 */
#define CLIENTCFGFILE     "/etc/bmqcl.ini"             /*客户端配置文件   */
#define BMQINDEXFILE      "/filetrans/bmqindex.rec"    /*索引记录文件     */
#define MAXFILENO         99999999L                    /*索引记录文件最大值*/
#define igSend_pack_size  1024                         /*文件传输拆包大小*/
#define MAC_LEN           8                            /*MAC长度*/
#define iFLDNAMELEN       20                           /*域名最大长度 */
#define MONMSGCODE        800800L                      /*监控信息代码*/
#define iMSGFILETRANS     800                          /*文件传输消息类别码*/
#define iMBFILEMNG        1                            /*文件传输管理邮箱*/
#define iMBAUDIT          2                            /*审计邮箱*/
#define iMBFILESAF        3                            /*文件传输SAF邮箱*/
#define iMBFILELOG        4                            /*文件传输LOG邮箱*/
#define iMAXFILENUM       10                           /*多文件传输最大文件个数*/
#define iMAXROUTEREC      500                          /*最大自动路由记录数*/
#define DEFAULTCONNUM     1                            /*跨组短连接尝试连接次数*/

long    lgSHMKEY_MB;                                   /*共享内存ID*/
long    lgWAITTIME;                                    /*系统轮询时间*/
short   igDebug;                                       /*Debug标志*/
short   igGroupid;                                     /*跨组组号*/
short   igBmqtcpport;                                  /*通道管理通讯端口*/
short   igBmqgrpport;                                  /*跨组TCP通讯端口*/
long    lgGroup_try_time;                              /*跨组尝试间隔时间*/
char    agBmqhostip[100];                              /*主机IP地址*/
char    agCommode[2];                                  /*通讯连接方式*/
short   igMb_sys_current_connect;                      /*当前链接*/
short   igMb_sys_current_group;                        /*当前组*/
short   igMb_sys_current_mailbox;                      /*当前邮箱*/
short   igMbopenflag;                                  /*邮箱OPEN标志*/
long    lgCurrent_pid;                                 /*当前连接PID*/
/*next add by nh*/
char    agBmqhostIP[100];                              /*TCP通讯主机IP地址*/
short   igBmqport;                                     /*本地TCP通讯端口*/
long    lgTry_time;                                    /*尝试间隔时间*/
char    agBmqIP[100];                                  /*本地IP地址*/
short   igMBid;                                        /*非本地邮箱号*/
char    agBmqclip[100];                                /*客户端IP地址*/
short   igBmqclport;                                   /*客户端TCP通讯端口*/
long    lgTry_time;                                    /*尝试间隔时间*/
short   igBmqclsport;                                  /*TCP通讯端口*/
int     igFileTranBlockSize;                           /*文件传输拆包大小*/                

/*邮箱的信息*/
struct mbinfo
{
  /*next 1 row add by nh*/
  short iFlag;                  /*邮箱启用标志*/
  short iConnect;               /*连接数*/
  long  lSendnum;               /*发送消息数*/
  long  lRecvnum;               /*接收消息数*/    
  long  lPendnum;               /*当前消息数*/
  long  lSendtime;              /*最后发送时间*/
  long  lRecvtime;              /*最后接收时间*/
  long  lSendpid;               /*最后发送进程*/
  long  lRecvpid;               /*最后接收进程*/ 
  long  lQueue_head;            /*消息链表头*/
  long  lQueue_tail;            /*消息链表尾*/
  short iConn_head;             /*连接链表头*/
  short iConn_tail;             /*连接链表尾*/
  long  lExisttime;             /*报文驻留时间*/
  long  lMaxqueue;              /*邮箱阀值*/
  short iHload;                 /*上负载点*/
  short iLload;                 /*下负载点*/
  short iTimes;                 /*临界点持续次数*/    
  short iStatus;                /*状态标志*/
  short iLastnum;               /*负载状态持续次数*/
  /* add by xujun 2007.01.10 support scan virus */
  short iSendScanFlag;          /*发送邮件扫描病毒标志*/
  short iRecvScanFlag;          /*接收邮件扫描病毒标志*/
  char  aExpress[iFLDVALUELEN]; /*特征码表达式*/
};

/*邮箱队列的信息*/
struct mbqueue
{
  char  cTag;                   /*邮箱队列启用标志*/
  short iOrg_grp;               /*源发组*/        
  short iOrg_mailbox;           /*源发邮箱*/
  long  lLlink;                 /*队列左链*/
  long  lRlink;                 /*队列右链*/
  long  lSerialnum;             /*消息唯一ID*/ 
  short iPrior;                 /*优先级*/
  char  aFilter[iMASKLEN];      /*消息过滤条件*/
  /*char  aMask[iMASKLEN];      消息过滤掩码*/
  long  lPack_head;             /*报文存储区链表头*/
  long  lBegintime;             /*队列启用时间*/
};

/*队列存储区结构*/
struct mbpack
{
  /*short iMsglen;     delete by wh           当前区间报文长度*/
  TYPE_SWITCH iMsglen;                /*当前区间报文长度 add by wh*/
  long  lLink;                  /*存储区下链*/
  char  aPack[iMBPERPACKSIZE];  /*报文存储Buffer*/
};

/*报文索引文件结构*/
struct mbfilepack
{
  long  lLoglen;                /*报文长度*/
  char  aLogfile[12];           /*存储文件名*/
  long  lLogoffset;             /*报文存储偏移量*/
};


/*邮箱连接结构*/
struct mbconnect
{
  char  cTag;                   /*连接结构启用标志*/
  short iLlink;                 /*左链*/
  short iRlink;                 /*右链*/
  short iPrior;                 /*优先级*/
  char  aFilter[iMASKLEN];      /*消息过滤条件*/
  char  aMask[iMASKLEN];        /*消息过滤掩码*/
  long  lConnpid;               /*连接进程PID*/
  long  lMsgid_recv;            /*接收消息队列ID*/
};

/*邮箱报文文件存储结构*/
struct packsave
{ 
  short iMbid;/*当前所在邮箱号*/
  short iOrgGrpid;
  short iOrgMbid;
  short iPrior;
  char  aFilter[iMASKLEN];
  char  aMsgbuf[iMBMAXPACKSIZE];
  /*short iMsglen;	delete by wh*/	
  TYPE_SWITCH iMsglen;	/* add by wh*/	
}; 

/*共享内存结构*/
struct mbshm
{
  long    lMsgid_grp;                   /*组间通讯消息队列ID*/
  long    lMsgid_grp_rcv;               /*跨组报文存储消息队列*/
  /*del by xujun 2006.12.28 不使用通道管理器
  long    lMsgid_cls;                   C/S间通讯消息队列ID*/
  long    lSemid;                       /*信号灯ID*/
  long    lMbpack_free;                 /*报文存储区空闲位*/
  long    lMbqueue_free;                /*邮箱队列区空闲位*/
  long    lgMb_sys_queue_serial_num;    /*消息的唯一ID*/
  short   iMbconnect_free;              /*连接区空闲位*/
  long    lBmqcls_pid;                  /*服务器端侦听守护进程*/
  /*
  long    lBmqclssnd_pid;               *服务器端通讯守护发送进程 PID*
  long    lBmqclsrcv_pid;               *服务器端通讯守护接收进程 PID*
  long    lBmqclcrcv_pid;               *客户端接收进程守护*
  long    lBmqclcsnd_pid;               *客户端发送守护进程*
  del by xujun 2006.12.28
  不使用通道管理器*/
  long    lBmqclean_pid;                /*邮箱状态监控进程PID*/
  long    lBmqgrpsnd_pid;               /*跨组通讯守护发送进程 PID*/
  long    lBmqgrprcv_pid;               /*长链接跨组通讯守护接收进程 PID*/
  long    lBmqgrprcvs_pid;              /*短链接跨组通讯守护接收进程 PID*/
  long    lBmqgrprcvw_pid;              /*短链接报文存储守护进程 PID*/
  long    lBmqfilecls_pid;              /*文件服务守护进程*/
  long    lBmqfilemng_pid;              /*文件服务管理守护进程*/
  long    lBmqfilesaf_pid;              /*文件服务SAF守护进程*/
  long    lBmqfilelog_pid;              /*文件服务LOG守护进程*/
  long    lMbqueueoffset;               /*邮箱队列区偏移量*/
  long    lMbpackoffset;                /*邮箱报文区偏移量*/
  long    lMbconnectoffset;             /*邮箱连接区偏移量*/
  long    lFileoffset;                  /*文件传输区偏移量*/
  long    lMbrouteroffset;              /*自动路由配置偏移量*/
  short   iMbinfo;                      /*邮箱数目*/
  long    lMbqueuecount;                /*邮箱队列数目*/
  long    lMbpacksize;                  /*对列存储区大小*/
  short   iMbcontcount;                 /*最大邮箱连接数目*/
  short   iFilecount;                   /*最大同时发送文件数*/
  short   iRouternum;                   /*配置的路由数目*/
  long    lLogfilesize;                 /*日志文件大小*/
  short   iMbgrpid;                     /*本组组号*/
  short   iStatflag;                    /*统计启用标志*/
  long    lFileno;                      /*索引文件号*/
  short   iCLSConnectNum;               /*C/S Cli链接数*/
};

/*所收信息的结构*/
struct recvmsg
{
  long   mtype;                         /*消息Type*/
  long   lQueueid;                      /*队列ID*/
  long   lSerialnum;                    /*唯一消息ID*/
};

/*跨组守护进程消息数据结构*/
struct grprecvmsg
{
  long  mtype;                          /*消息Type*/
  long  lPack_head;                     /*报文头*/
  short iOrg_group;                     /*源发组*/
  short iOrg_mailbox;                   /*源发邮箱*/
  short iPrior;                         /*优先级*/
  char  aFilter[iMASKLEN];              /*消息过滤条件*/
  short iDes_group;                     /*目标组*/
  short iDes_mailbox;                   /*目标邮箱*/
};

/*next 12 rows add by nh*/
/*服务器端守护进程消息数据结构*/
struct clmbinf
{
  long  mtype;                          /*消息Type*/
  long  lPack_head;                     /*报文头*/
  short iOrg_group;                     /*源发组*/
  short iOrg_mailbox;                   /*源发邮箱*/
  short iPrior;                         /*优先级*/
  char  aFilter[iMASKLEN];              /*消息过滤条件*/
  short iDes_group;                     /*目标组*/
  short iDes_mailbox;                   /*目标邮箱*/
};

/*客户端守护进程消息数据结构*/
struct clMB
{
  long  mtype;                          /*消息Type*/
  long  lPack_head;                     /*报文头*/
  short iOrg_group;                     /*源发组*/
  short iOrg_mailbox;                   /*源发邮箱*/
  short iPrior;                         /*优先级*/
  char  aFilter[iMASKLEN];              /*消息过滤条件*/
  short iDes_group;                     /*目标组*/
  short iDes_mailbox;                   /*目标邮箱*/
};


struct mbmanger
{
  short iMsgType;                       /*消息类型*/
  short iMbid;                          /*邮箱号*/
  short iStatus;                        /*负载状态*/
};

struct mbstat
{
  short   iOrgmb;                       /*发送邮箱*/
  short   iDesmb;                       /*接收邮箱*/
  long    lSendtime;                    /*发送时间*/
  long    lSendpid;                     /*发送进程*/
};

/* add struct indexfile&transparm by xujun 2002.12.18 */
struct indexfile
{
  char    cTag;                         /*启用标志 */    
  long    lSerialno;                    /*文件索引号 */
  char    aFilename[80];                /*文件名 */
  char    aMemo[20];                    /*备用字段 */
};

struct transparm
{
  int  (*bmqFile_comm)( char aParm[][iFLDVALUELEN]);    /*通讯函数 */
  int  (*bmqFile_msg)( char aParm[][iFLDVALUELEN]);     /*报文处理函数 */
};
/* add by xujun for send monitor msg */
struct msghead
{
  long  lTranid;
  long  lBegintime;
  long  lCode;
  long  lSafid;
  long  lSwitchtranid;
  short iBegin_q_id;
  short iTranstep;
  short iMsgtype;
  short iMsgtypeorg;
  short iOrg_q;
  short iDes_q;
  short iFmtgroup;
  short iRevmode;
  short iRevfmtgroup;
  short iSwitchgroup;
  short iRouteline;
  /*short iBodylen; delete by wh*/
  TYPE_SWITCH iBodylen; /*add by wh 20150107*/
  short iMsgformat;
  char  cFormatter;
  char  cRouter;
  char  cCodetype;
  char  cEndflag;
  char  cSafflag;
  char  aBegin_trancode[10];
  char  aTrancode[10];
  char  aSysattr[10];  /* add by nh 20020910 */
  char  aMemo[20];
  char  a1[7];
};

/* 交换平台报文 */
struct msgpack
{
  struct msghead sMsghead;
  char  aMsgbody[ iMBMAXPACKSIZE - sizeof(struct msghead) ];
};
struct monmsg
{
  short    iFlag;                /*启用标志*/
  long     lMsgCode;             /*信息代码*/
  short    iSendFlag;            /*发送标志 1:开始发送 2:正在发送 3:发送完成 4:发送异常 5:发送失败 */
  char     aStatusDesc[30];      /*发送状态描述*/
  long     lSerialno;            /*文件序列号*/
  char     aFileName[80];        /*文件名*/
  short    iOrgGrpid;            /*源发组*/
  short    iOrgMbid;             /*源发邮箱*/
  short    iDesGrpid;            /*目标组*/
  short    iDesMbid;             /*目标邮箱*/
  struct   timeb sBeginTime;     /*开始发送时间*/
  struct   timeb sEndTime;       /*结束发送时间*/
  long     lFileSize;            /*文件大小(字节数)*/
  long     lSendSize;            /*已发送文件大小(字节数)*/
  double   fSendSpeed;           /*发送速率KB/秒*/
  long     lSendPid;             /*发送进程pid*/
};
struct    monmsg    sgMonMsg; 

/* add by hxz for mulfile rev and send message */
struct bmqMessage
{ 
  short iGrpid;
  short iMbid;
  short iPrior;
  long  lType;
  long  lClass;
  char  aTopic[41];
  char  aMsgbuf[iMBMAXPACKSIZE];
  /*short iMsglen; delete by wh*/
  TYPE_SWITCH iMsglen;  /* add by wh*/
  short iFilenum;
  char  aFileName[iMAXFILENUM][100];
};

/* add by xujun for auto router send */
struct mbrouter
{
  short iOrgMbid ;               /*源发邮箱号*/
  char  aValue[iFLDVALUELEN];    /*特征码值*/
  short iDesGrpid;               /*目标组*/
  short iDesMbid;                /*目标邮箱*/
  short iOpenFlag;               /*开关标志*/ 
  char  aDesc[100];              /*描述信息*/
};

/*共享内存区指针*/
struct mbshm     *psgMbshm;                
struct mbinfo    *psgMbinfo;             
struct mbqueue   *psgMbqueue;          
struct mbpack    *psgMbpack;           
struct mbconnect *psgMbconnect;
struct monmsg    *psgMbfilemng;
struct mbrouter  *psgMbrouter;

/* 新晨Q API函数 */
/*int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,short *piMsglen); delete by wh*/
int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen);/* add by wh*/
/*int bmqJava_getfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,short *piMsglen);delete by wh*/
int bmqJava_getfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen);/* add by wh*/
int bmqGetmbinfo(short iMbid,struct mbinfo *plMbinfo);
int bmqClearmb(short iMbid);
/*int bmqPutfilter(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen);delete by wh*/
int bmqPutfilter(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen);/*add by wh*/
/*int bmqGetwfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char*aMask,char *aMsgbuf,short *piMsglen,short iTimeout);delete by wh*/
int bmqGetwfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char*aMask,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout);/*add by wh*/
int bmqOpen(short iMbid);
int bmqClose();
/*int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,char *aMsgbuf,short iMsglen);delete by wh*/
int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,char *aMsgbuf,TYPE_SWITCH iMsglen); /* add by wh*/
/*int bmqGet(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,short *piMsglen); delete by wh*/
int bmqGet(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen);/*add by wh*/
/*int bmqGetFile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,short *piMsglen,short iTimeout,char *aFileName,char *alFilepath); delete by wh*/
int bmqGetFile(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout,char *aFileName,char *alFilepath);/*add by wh*/
/*int bmqPutFile(short iGrpid,short iMbid,short iPrior,long lType,char *aMsgbuf,short iMsglen,char *aFilename);delete by wh*/
int bmqPutFile(short iGrpid,short iMbid,short iPrior,long lType,char *aMsgbuf,TYPE_SWITCH iMsglen,char *aFilename);/*add by wh*/
int bmqMulPutFile(struct bmqMessage sendmulfile);
int bmqMulGetFile(struct bmqMessage *revmulfile,short iTimeout,char *alFilepath);
int _bmqDelspace(char *alstr);
/*int bmqGetw(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,short *piMsglen,short iTimeout);delete by wh*/
int bmqGetw(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,TYPE_SWITCH *piMsglen,short iTimeout);/*add by wh*/
int bmqSendfile(short iGrpid,char *aFilename,struct transparm sTransparm,long *lSerialno);
int bmqRecvfile(long lSerialno,FILE **fp);
int _bmqGetindexfile(long lSerialno,char *aFilename);
/*int bmqPutSavePack(short iOrgGrpid,short iOrgMbid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen);delete by wh*/
int bmqPutSavePack(short iOrgGrpid,short iOrgMbid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen);/*add by wh*/
int _bmqDelspace(char *alstr);
/* 新晨Q API函数 */

/* 新晨Q 系统内部函数 */
#ifdef _LINUXES_
int _bmqDebug(char *frm, ...);
int _bmqVdebug(short iDebug,char *frm, ...);
#else
int _bmqDebug();
int _bmqVdebug();
#endif
/*int _bmqDebughex(char * buff,short buflen);delete by wh*/
int _bmqDebughex(char * buff,TYPE_SWITCH buflen);/*add by wh*/
void _bmqSignalinit();
void _bmqMac();
void _bmqfilechr(char *,short,char *,char);
int  _bmqTrim();
int  _bmqShmConnect();
int  _bmqOpenfile(int iFileid,FILE **fp);
int  _bmqConnfile_pick(FILE *fp,struct mbconnect *slMbconnect,long llOffset);
int  _bmqClosefile(FILE *fp);
int  _bmqLock(int iNum);
int  _bmqUnlock(int iNum);
int  _bmqQueuefile_pick(FILE *fp,struct mbqueue *slMbqueue,long llOffset);
/*int  _bmqPack_get(char *aMsgbuf,short *piMsglen,long lPack_head);delete by wh*/
int  _bmqPack_get(char *aMsgbuf,TYPE_SWITCH *piMsglen,long lPack_head);/* add by wh*/
/*int  _bmqPack_javaget(char *aMsgbuf,short *piMsglen,long lPack_head);delete by wh*/
int  _bmqPack_javaget(char *aMsgbuf,TYPE_SWITCH *piMsglen,long lPack_head);/*add by wh*/
int  _bmqShowversion(int iArgc,char **wcArgv);
int  _bmqStartup();
int  _bmqConfig_load(char *aCfgType,short iGrpid);
int  _bmqClscfg_load(char *aCfgType);
int  _bmqTimeBuf(time_t * lt,char *timebuf);
void _bmqProshut();
int  _bmqShutdown();
int  _bmqDaemon_creat();
int  _bmqConnect_del(short iMbid,struct mbconnect *psMbconnect);
/*int  _bmqPutfile(FILE *fp, long lOffset, short iLen, char *aBuf);delete by wh*/
int  _bmqPutfile(FILE *fp, long lOffset, TYPE_SWITCH iLen, char *aBuf);/*add by wh*/
int  _bmqQueue_del(short iMbid,struct mbqueue *slMbqueue,short *piGrpid,short *piMbid,short *piPrior,char *aFilter,long *plPack_head);
int  _bmqGetmsgid(short iConnectid);
int _bmqCrypt(char *inbuf,int inlen,char *key);
int _bmqCrypt_all(char *inbuf,int inlen,char *key,char *press);
/*int  _bmqFmlpackget(char *aMsgbody, short iMsglen, char *aFldname, short *piFldlen, char *aFldval);delete by wh*/
int  _bmqFmlpackget(char *aMsgbody, TYPE_SWITCH iMsglen, char *aFldname, TYPE_SWITCH *piFldlen, char *aFldval);/*add by wh*/
/*int  _bmqFmlpackset(char *aMsgbody, short *piMsglen, char *aFldname, short iFldlen, char *aFldval);delete by wh*/
int  _bmqFmlpackset(char *aMsgbody, TYPE_SWITCH *piMsglen, char *aFldname, TYPE_SWITCH iFldlen, char *aFldval);/*add by wh*/
int  _bmqMsgsend(long code,char *msg);
int  _bmqGettime(char *date);
int  _bmqTimeBuf(time_t * lt,char *timebuf);
int  _bmqMonMsgSend(long lMsgCode,struct monmsg sMonMsg);
void _bmqMonMsgPut(struct monmsg sMonMsg);
int  _bmqSendfile_G(short iGrpid,char *aFilename,struct transparm sTransparm, long *lSerialno);
int _bmqLoadVirusDbDir(char *aPath,struct cl_node **sRoot,unsigned int *iVirusNum);
int _bmqVirusScanByRoot(char *aFileName,struct cl_node *sRoot,char *aVirName);
void _bmqFreeVirusRoot(struct cl_node *sRoot);
int _bmqScanVirus(char *aVirusDbDir,char *aFileName,char *aVirName);
void _bmqLokiEnc( char *data );
void _bmqLokiDec( char *data );
/*int _bmqGrpasc_snd(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,short iMsglen);delete by wh*/
int _bmqGrpasc_snd(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,TYPE_SWITCH iMsglen);/*add by wh*/
int _bmqLoadRouterinfo();
int _bmqGetTypeExpress(short iMbid,char *aExpress);
int _bmqGetRouter(short iMbid,char *aExpress,short *piGrpid,short *piMbid);
int _bmqGrpcontserS(char *aIp,short iPort,long lTime);
//int _bmqGrpSend(int,char *,short *);
//int _comTcpReceive(int,unsigned char *,short *);
/* 新晨Q 系统内部函数 */

#include "bmqusrfun.h"
#endif
