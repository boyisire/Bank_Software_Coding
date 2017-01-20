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
#define SHMNOTFOUND    100     /*û�ҵ������ڴ��¼ */
/*  delete by wh #define iMBMAXPACKSIZE 8192    ����ĳ���       */
#define iMBMAXPACKSIZE 65536    /*����ĳ���      add by wh */ 
#define DEFINE_THREADS 10      /*ȱʡ�߳���         */

#define iMBPERPACKSIZE 1024    /*�������д洢��     */
#define iSEMNUM        2       /*�ź���             */
#define LK_ALL         1       /*�������̵Ŀ�����   */
#define LK_ALL_2         2       /*�������̵Ŀ�����   */

#define iMASKLEN       20      /*Mask,Filter�ĳ���  */

#define FILEKEY           "BMQPWD0"
#define SERVERCFGFILE     "/etc/bmq.ini"               /*�������������ļ� */
#define CLIENTCFGFILE     "/etc/bmqcl.ini"             /*�ͻ��������ļ�   */
#define BMQINDEXFILE      "/filetrans/bmqindex.rec"    /*������¼�ļ�     */
#define MAXFILENO         99999999L                    /*������¼�ļ����ֵ*/
#define igSend_pack_size  1024                         /*�ļ���������С*/
#define MAC_LEN           8                            /*MAC����*/
#define iFLDNAMELEN       20                           /*������󳤶� */
#define MONMSGCODE        800800L                      /*�����Ϣ����*/
#define iMSGFILETRANS     800                          /*�ļ�������Ϣ�����*/
#define iMBFILEMNG        1                            /*�ļ������������*/
#define iMBAUDIT          2                            /*�������*/
#define iMBFILESAF        3                            /*�ļ�����SAF����*/
#define iMBFILELOG        4                            /*�ļ�����LOG����*/
#define iMAXFILENUM       10                           /*���ļ���������ļ�����*/
#define iMAXROUTEREC      500                          /*����Զ�·�ɼ�¼��*/
#define DEFAULTCONNUM     1                            /*��������ӳ������Ӵ���*/

long    lgSHMKEY_MB;                                   /*�����ڴ�ID*/
long    lgWAITTIME;                                    /*ϵͳ��ѯʱ��*/
short   igDebug;                                       /*Debug��־*/
short   igGroupid;                                     /*�������*/
short   igBmqtcpport;                                  /*ͨ������ͨѶ�˿�*/
short   igBmqgrpport;                                  /*����TCPͨѶ�˿�*/
long    lgGroup_try_time;                              /*���鳢�Լ��ʱ��*/
char    agBmqhostip[100];                              /*����IP��ַ*/
char    agCommode[2];                                  /*ͨѶ���ӷ�ʽ*/
short   igMb_sys_current_connect;                      /*��ǰ����*/
short   igMb_sys_current_group;                        /*��ǰ��*/
short   igMb_sys_current_mailbox;                      /*��ǰ����*/
short   igMbopenflag;                                  /*����OPEN��־*/
long    lgCurrent_pid;                                 /*��ǰ����PID*/
/*next add by nh*/
char    agBmqhostIP[100];                              /*TCPͨѶ����IP��ַ*/
short   igBmqport;                                     /*����TCPͨѶ�˿�*/
long    lgTry_time;                                    /*���Լ��ʱ��*/
char    agBmqIP[100];                                  /*����IP��ַ*/
short   igMBid;                                        /*�Ǳ��������*/
char    agBmqclip[100];                                /*�ͻ���IP��ַ*/
short   igBmqclport;                                   /*�ͻ���TCPͨѶ�˿�*/
long    lgTry_time;                                    /*���Լ��ʱ��*/
short   igBmqclsport;                                  /*TCPͨѶ�˿�*/
int     igFileTranBlockSize;                           /*�ļ���������С*/                

/*�������Ϣ*/
struct mbinfo
{
  /*next 1 row add by nh*/
  short iFlag;                  /*�������ñ�־*/
  short iConnect;               /*������*/
  long  lSendnum;               /*������Ϣ��*/
  long  lRecvnum;               /*������Ϣ��*/    
  long  lPendnum;               /*��ǰ��Ϣ��*/
  long  lSendtime;              /*�����ʱ��*/
  long  lRecvtime;              /*������ʱ��*/
  long  lSendpid;               /*����ͽ���*/
  long  lRecvpid;               /*�����ս���*/ 
  long  lQueue_head;            /*��Ϣ����ͷ*/
  long  lQueue_tail;            /*��Ϣ����β*/
  short iConn_head;             /*��������ͷ*/
  short iConn_tail;             /*��������β*/
  long  lExisttime;             /*����פ��ʱ��*/
  long  lMaxqueue;              /*���䷧ֵ*/
  short iHload;                 /*�ϸ��ص�*/
  short iLload;                 /*�¸��ص�*/
  short iTimes;                 /*�ٽ���������*/    
  short iStatus;                /*״̬��־*/
  short iLastnum;               /*����״̬��������*/
  /* add by xujun 2007.01.10 support scan virus */
  short iSendScanFlag;          /*�����ʼ�ɨ�財����־*/
  short iRecvScanFlag;          /*�����ʼ�ɨ�財����־*/
  char  aExpress[iFLDVALUELEN]; /*��������ʽ*/
};

/*������е���Ϣ*/
struct mbqueue
{
  char  cTag;                   /*����������ñ�־*/
  short iOrg_grp;               /*Դ����*/        
  short iOrg_mailbox;           /*Դ������*/
  long  lLlink;                 /*��������*/
  long  lRlink;                 /*��������*/
  long  lSerialnum;             /*��ϢΨһID*/ 
  short iPrior;                 /*���ȼ�*/
  char  aFilter[iMASKLEN];      /*��Ϣ��������*/
  /*char  aMask[iMASKLEN];      ��Ϣ��������*/
  long  lPack_head;             /*���Ĵ洢������ͷ*/
  long  lBegintime;             /*��������ʱ��*/
};

/*���д洢���ṹ*/
struct mbpack
{
  /*short iMsglen;     delete by wh           ��ǰ���䱨�ĳ���*/
  TYPE_SWITCH iMsglen;                /*��ǰ���䱨�ĳ��� add by wh*/
  long  lLink;                  /*�洢������*/
  char  aPack[iMBPERPACKSIZE];  /*���Ĵ洢Buffer*/
};

/*���������ļ��ṹ*/
struct mbfilepack
{
  long  lLoglen;                /*���ĳ���*/
  char  aLogfile[12];           /*�洢�ļ���*/
  long  lLogoffset;             /*���Ĵ洢ƫ����*/
};


/*�������ӽṹ*/
struct mbconnect
{
  char  cTag;                   /*���ӽṹ���ñ�־*/
  short iLlink;                 /*����*/
  short iRlink;                 /*����*/
  short iPrior;                 /*���ȼ�*/
  char  aFilter[iMASKLEN];      /*��Ϣ��������*/
  char  aMask[iMASKLEN];        /*��Ϣ��������*/
  long  lConnpid;               /*���ӽ���PID*/
  long  lMsgid_recv;            /*������Ϣ����ID*/
};

/*���䱨���ļ��洢�ṹ*/
struct packsave
{ 
  short iMbid;/*��ǰ���������*/
  short iOrgGrpid;
  short iOrgMbid;
  short iPrior;
  char  aFilter[iMASKLEN];
  char  aMsgbuf[iMBMAXPACKSIZE];
  /*short iMsglen;	delete by wh*/	
  TYPE_SWITCH iMsglen;	/* add by wh*/	
}; 

/*�����ڴ�ṹ*/
struct mbshm
{
  long    lMsgid_grp;                   /*���ͨѶ��Ϣ����ID*/
  long    lMsgid_grp_rcv;               /*���鱨�Ĵ洢��Ϣ����*/
  /*del by xujun 2006.12.28 ��ʹ��ͨ��������
  long    lMsgid_cls;                   C/S��ͨѶ��Ϣ����ID*/
  long    lSemid;                       /*�źŵ�ID*/
  long    lMbpack_free;                 /*���Ĵ洢������λ*/
  long    lMbqueue_free;                /*�������������λ*/
  long    lgMb_sys_queue_serial_num;    /*��Ϣ��ΨһID*/
  short   iMbconnect_free;              /*����������λ*/
  long    lBmqcls_pid;                  /*�������������ػ�����*/
  /*
  long    lBmqclssnd_pid;               *��������ͨѶ�ػ����ͽ��� PID*
  long    lBmqclsrcv_pid;               *��������ͨѶ�ػ����ս��� PID*
  long    lBmqclcrcv_pid;               *�ͻ��˽��ս����ػ�*
  long    lBmqclcsnd_pid;               *�ͻ��˷����ػ�����*
  del by xujun 2006.12.28
  ��ʹ��ͨ��������*/
  long    lBmqclean_pid;                /*����״̬��ؽ���PID*/
  long    lBmqgrpsnd_pid;               /*����ͨѶ�ػ����ͽ��� PID*/
  long    lBmqgrprcv_pid;               /*�����ӿ���ͨѶ�ػ����ս��� PID*/
  long    lBmqgrprcvs_pid;              /*�����ӿ���ͨѶ�ػ����ս��� PID*/
  long    lBmqgrprcvw_pid;              /*�����ӱ��Ĵ洢�ػ����� PID*/
  long    lBmqfilecls_pid;              /*�ļ������ػ�����*/
  long    lBmqfilemng_pid;              /*�ļ���������ػ�����*/
  long    lBmqfilesaf_pid;              /*�ļ�����SAF�ػ�����*/
  long    lBmqfilelog_pid;              /*�ļ�����LOG�ػ�����*/
  long    lMbqueueoffset;               /*���������ƫ����*/
  long    lMbpackoffset;                /*���䱨����ƫ����*/
  long    lMbconnectoffset;             /*����������ƫ����*/
  long    lFileoffset;                  /*�ļ�������ƫ����*/
  long    lMbrouteroffset;              /*�Զ�·������ƫ����*/
  short   iMbinfo;                      /*������Ŀ*/
  long    lMbqueuecount;                /*���������Ŀ*/
  long    lMbpacksize;                  /*���д洢����С*/
  short   iMbcontcount;                 /*�������������Ŀ*/
  short   iFilecount;                   /*���ͬʱ�����ļ���*/
  short   iRouternum;                   /*���õ�·����Ŀ*/
  long    lLogfilesize;                 /*��־�ļ���С*/
  short   iMbgrpid;                     /*�������*/
  short   iStatflag;                    /*ͳ�����ñ�־*/
  long    lFileno;                      /*�����ļ���*/
  short   iCLSConnectNum;               /*C/S Cli������*/
};

/*������Ϣ�Ľṹ*/
struct recvmsg
{
  long   mtype;                         /*��ϢType*/
  long   lQueueid;                      /*����ID*/
  long   lSerialnum;                    /*Ψһ��ϢID*/
};

/*�����ػ�������Ϣ���ݽṹ*/
struct grprecvmsg
{
  long  mtype;                          /*��ϢType*/
  long  lPack_head;                     /*����ͷ*/
  short iOrg_group;                     /*Դ����*/
  short iOrg_mailbox;                   /*Դ������*/
  short iPrior;                         /*���ȼ�*/
  char  aFilter[iMASKLEN];              /*��Ϣ��������*/
  short iDes_group;                     /*Ŀ����*/
  short iDes_mailbox;                   /*Ŀ������*/
};

/*next 12 rows add by nh*/
/*���������ػ�������Ϣ���ݽṹ*/
struct clmbinf
{
  long  mtype;                          /*��ϢType*/
  long  lPack_head;                     /*����ͷ*/
  short iOrg_group;                     /*Դ����*/
  short iOrg_mailbox;                   /*Դ������*/
  short iPrior;                         /*���ȼ�*/
  char  aFilter[iMASKLEN];              /*��Ϣ��������*/
  short iDes_group;                     /*Ŀ����*/
  short iDes_mailbox;                   /*Ŀ������*/
};

/*�ͻ����ػ�������Ϣ���ݽṹ*/
struct clMB
{
  long  mtype;                          /*��ϢType*/
  long  lPack_head;                     /*����ͷ*/
  short iOrg_group;                     /*Դ����*/
  short iOrg_mailbox;                   /*Դ������*/
  short iPrior;                         /*���ȼ�*/
  char  aFilter[iMASKLEN];              /*��Ϣ��������*/
  short iDes_group;                     /*Ŀ����*/
  short iDes_mailbox;                   /*Ŀ������*/
};


struct mbmanger
{
  short iMsgType;                       /*��Ϣ����*/
  short iMbid;                          /*�����*/
  short iStatus;                        /*����״̬*/
};

struct mbstat
{
  short   iOrgmb;                       /*��������*/
  short   iDesmb;                       /*��������*/
  long    lSendtime;                    /*����ʱ��*/
  long    lSendpid;                     /*���ͽ���*/
};

/* add struct indexfile&transparm by xujun 2002.12.18 */
struct indexfile
{
  char    cTag;                         /*���ñ�־ */    
  long    lSerialno;                    /*�ļ������� */
  char    aFilename[80];                /*�ļ��� */
  char    aMemo[20];                    /*�����ֶ� */
};

struct transparm
{
  int  (*bmqFile_comm)( char aParm[][iFLDVALUELEN]);    /*ͨѶ���� */
  int  (*bmqFile_msg)( char aParm[][iFLDVALUELEN]);     /*���Ĵ����� */
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

/* ����ƽ̨���� */
struct msgpack
{
  struct msghead sMsghead;
  char  aMsgbody[ iMBMAXPACKSIZE - sizeof(struct msghead) ];
};
struct monmsg
{
  short    iFlag;                /*���ñ�־*/
  long     lMsgCode;             /*��Ϣ����*/
  short    iSendFlag;            /*���ͱ�־ 1:��ʼ���� 2:���ڷ��� 3:������� 4:�����쳣 5:����ʧ�� */
  char     aStatusDesc[30];      /*����״̬����*/
  long     lSerialno;            /*�ļ����к�*/
  char     aFileName[80];        /*�ļ���*/
  short    iOrgGrpid;            /*Դ����*/
  short    iOrgMbid;             /*Դ������*/
  short    iDesGrpid;            /*Ŀ����*/
  short    iDesMbid;             /*Ŀ������*/
  struct   timeb sBeginTime;     /*��ʼ����ʱ��*/
  struct   timeb sEndTime;       /*��������ʱ��*/
  long     lFileSize;            /*�ļ���С(�ֽ���)*/
  long     lSendSize;            /*�ѷ����ļ���С(�ֽ���)*/
  double   fSendSpeed;           /*��������KB/��*/
  long     lSendPid;             /*���ͽ���pid*/
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
  short iOrgMbid ;               /*Դ�������*/
  char  aValue[iFLDVALUELEN];    /*������ֵ*/
  short iDesGrpid;               /*Ŀ����*/
  short iDesMbid;                /*Ŀ������*/
  short iOpenFlag;               /*���ر�־*/ 
  char  aDesc[100];              /*������Ϣ*/
};

/*�����ڴ���ָ��*/
struct mbshm     *psgMbshm;                
struct mbinfo    *psgMbinfo;             
struct mbqueue   *psgMbqueue;          
struct mbpack    *psgMbpack;           
struct mbconnect *psgMbconnect;
struct monmsg    *psgMbfilemng;
struct mbrouter  *psgMbrouter;

/* �³�Q API���� */
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
/* �³�Q API���� */

/* �³�Q ϵͳ�ڲ����� */
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
/* �³�Q ϵͳ�ڲ����� */

#include "bmqusrfun.h"
#endif
