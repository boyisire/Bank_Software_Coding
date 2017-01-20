#ifndef __SWAPI_H__
#define __SWAPI_H__

/* UNIXϵͳ���� */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <stdarg.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/timeb.h>

#define iMSGMAXLEN      65536     /* ������󳤶� */

/**********delete by wanghao 20141215********/
//#define iMSGMAXLEN      8192    /* ������󳤶� */
#define iBUFFLDNUM      500     /* Ԥ������� */
#define iFLDNAMELEN     40      /* ������󳤶� */ /*���Ҫ�ı�,ע��ҪΪ8�ı��� */
#define iPERFLDVALUELEN 512     /* ��ֵ��󳤶� */
#define iFLDVALUELEN    2048     /* ��ֵ��󳤶� */
/*#define iFLDNUMBER	30 */     /* �ļ����������� */  
#define iFLDNUMBER	50      /* �ļ����������� */  /* modify by zjj 2002.01.30 */

#define iMSGAPP		1
#define iMSGREV		51
#define iMSGUNPACK	101
#define iMSGPACK	102
#define iMSGREVUNPACK	111
#define iMSGREVPACK	112
#define iMSGSAVEHEAD	121
#define iMSGLOADHEAD	122
#define iMSGROUTE	201
#define iMSGMESSAGE	901
#define iMSGORDER	902

#define SUCCESS		0
#define FAIL		-1

struct _portattrib
{
  long* plTranid;
  long* plSafid;
  short	iQid;
  short	iMsgformat;
  int	iSemtranid;
  int	iSemsafid;
  char  a1[4];
};

#ifndef EXTERN
char    agDebugfile[64];                /* �����ļ��� */
char	cgDebug;
struct _portattrib	sgPortattrib;
char   procName[20]; /* add by cjh at 2015.04.01 one line PSBC_V1.0 */
#else
extern	char    agDebugfile[64];                /* �����ļ��� */
extern	char	cgDebug;
extern	struct _portattrib	sgPortattrib;
char   procName[20]; /* add by cjh at 2015.04.01 one line PSBC_V1.0 */
#endif

/* D401Ӧ�ñ��Ŀ���ͷ */
struct msghead 
{
  long	lTranid;
  long	lBegintime;
  long	lCode;
  long	lSafid;
  long	lSwitchtranid;
  short	iBegin_q_id;
  short	iTranstep;
  short	iMsgtype;
  short	iMsgtypeorg;
  short	iOrg_q;
  short	iDes_q;
  /*delete by zcd 20141220
  short	iFmtgroup;
  ***end of delete by zcd 201220 */
  /*add by zcd 20141219*/
  long iFmtgroup;
  /*end of add by zcd 20141220*/
  
  short	iRevmode;

  /*delete by zcd 20141220
  short	iRevfmtgroup;
  short iSwitchgroup;
  ***end of delete by zcd 201220 */
  /*add by zcd 20141219*/
  long iRevfmtgroup;
  long iSwitchgroup;
  /*end of add by zcd 20141220*/
  short	iRouteline;
  /*modify by zcd 20141230
  short	iBodylen;*/
  int iBodylen;
//  char  cAgdebugf[64];  /*   add by tbl 20150320 */
  short	iMsgformat;
  char	cFormatter;
  char	cRouter;
  char	cCodetype;
  char	cEndflag;
  char	cSafflag;
  char	aBegin_trancode[10];
  char	aTrancode[10];
  /* bit 1: ='0'-���ڹ����ڴ� ='1' �������ݿ� */
  char  aSysattr[10];  /* add by nh 20020910 */
  char call_flag; /*add by baiqj20150319, PSBC_V1.0*/
  struct timeb  callstart; /*add by baiqj20150319, PSBC_V1.0*/
  char  timeconsum[16]; /*add by baiqj 20150319, PSBC_V1.0*/
  int log_level;        /*add by cjh 20150319, PSBC_V1.0*/
  char file_log_name[32]; /*add by cjh 20150319, PSBC_V1.0*/
  char	aMemo[20];
  char  a1[7];
};

/* D402�ڲ����ı�����洢 */
struct imfbuf
{
  char aFldname[ iFLDNAMELEN];
  /*modify by zcd 20141230
  short iFldlen;*/
  unsigned int iFldlen;
  char aFldvalue[ iPERFLDVALUELEN];
  char  a1[6];
};

/* D403�зָ����ⲿ����Ԥ���������洢 */
struct sepbuf
{
  short	iRecid;
  short	iGrpid;
  short	iFldid;
   /*modify by zcd 20141230
 short iFldlen;*/
  unsigned int iFldlen; /* add by qy 09.29 */
  char	aFldvalue[ iPERFLDVALUELEN];
};

/* D404 8583�ⲿ����Ԥ���������洢 */
struct isobuf
{
  /*short	iFldid;  del by baiqj20150413 PSBC_V1.0*/
  long	iFldid;  /*add by baiqj20150413 PSBC_V1.0*/
  /*modify by zcd 20141230
  short	iFldlen;*/
  unsigned int iFldlen;
  short iOrgfldlen; /* add by qy 09.29 */
  char  a1[2];
  char	aFldvalue[ iPERFLDVALUELEN];
  char	aOrgfldlen[10];
  char	aOrgfldvalue[iPERFLDVALUELEN];
  char  a2[6];
};

/* D405����ƽ̨���� */
struct msgpack
{
  struct msghead sMsghead;
  char	aMsgbody[ iMSGMAXLEN - sizeof(struct msghead) ];
};

/* Ԥ������� */
union preunpackbuf
{
  struct imfbuf sImfbuf;
  struct sepbuf sSepbuf;
  struct isobuf sIsobuf;
};

/* API�������� */
#ifdef __cplusplus
extern "C"
{
#endif

extern int swMbopen(short);
extern int swMbclose();
extern int swSendpack(short, char *, unsigned int, short, short, short);
extern int swRecvpackw(short *, char *, unsigned int * ,short * , short *, short *);
extern int swRecvpack(short *,char *,unsigned int *,short *, short *, short *, short);
extern int swSendpackl(short, char *, unsigned int, short, long, long);
extern int swRecvpacklw(short *, char *, unsigned int* ,short * , long *, long *);
extern int swRecvpackl(short *,char *,unsigned int*,short *, long *, long *, short);
extern int swNewtran(struct msghead *);
extern int swNewtranhead(char *,struct msghead *);	/* added by fzj at 2002.02.28 */
extern int swTranrev(long);
extern int swSaf(char *);
extern int swInit(short);
extern int swFmlunpack(char *,unsigned int,union preunpackbuf *);
extern int swFmlpack(union preunpackbuf *,char *,unsigned int *);
extern int swFmlget(union preunpackbuf *,char *e,  short *,char *);
extern int swFmlset(char *,short ,char *, union preunpackbuf *);
/*modify by zcd 20141230*/
extern int swFmlpackget(char *, int, char *, short  *, char *);
extern int swFmlpackset(char *, int*, char *, short  , char *);
/*end of modify */
extern int swGetheadkey(short, struct msgpack *, char *, char *);
extern int swSavemsghead(short,char *,struct msghead *);
extern int swLoadmsghead(short,char *,struct msghead *);
/*"ifdef" modified by qh 20070416*/
#ifdef OS_AIX
extern int swDebug(frm, va_list);
char *frm
va_dcl
/* del by gengling at 2015.03.31 four lines PSBC_V1.0 */
/* extern int swVdebug(iDebug,frm, va_list);
short iDebug
char *frm
va_dcl */
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(iDebug,filename,line,frm, va_list);
short iDebug
char *filename
int line
char *frm
va_dcl
#endif
#ifdef OS_HPUX
extern int swDebug(char *frm, ...);
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
#ifdef OS_LINUX
extern int swDebug(char *frm, ...);
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
#ifdef OS_SCO
extern int swDebug(char *frm, ...);
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
#ifdef OS_SOLARIS
extern int swDebug(char *frm, ...);
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
extern int swDebughex(char *,unsigned int);
extern int swDebugmsghead(char *);
extern int swDebugfml(char *);
extern int swFormat(struct msgpack *);
extern int swShmcheck();
extern int swPortset(short, short, short);
extern int swPortget(short, short, short *);
extern int swRevbykey(int iQid, char *aKey);
extern int swSavetranhead(short iQid,char *aMsghdkey,struct msghead *psMsghead);
extern int swLoadtranhead(short iQid,char *aMsghdkey,struct msghead *psMsghead);
extern int swGetmacbuf(char *aMacbuf, short *iMacbuflen);
extern int swDbopen();
extern int swDbclose();

/*add by dyw, PSBC_V1.0*/
extern void swLogInit(char *proc_name, char *sys_log_name);
extern int _swDebug_r(short , char *, int , char *, ...);
extern int sw_thread_log_init();
extern int sw_thread_log_set_attr(char * , int );

#ifdef __cplusplus
}
#endif

#endif
