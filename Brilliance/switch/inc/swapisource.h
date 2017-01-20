/*modified by cjh 20141220 流水号升级32位版本 8个9改为7个9*/
/*#define lMAXSERNO	99999999L*/      /* 最大流水号 */
/*#define lMAXSERNO	9999999L*/       /* 最大流水号 */
/*modified by cjh 20150408 流水号升级32位版本 7个9改为12个9 */
#define lMAXSERNO	999999999999L       /* 最大流水号 */

extern	char    agDebugfile[64];                /* 调试文件名 */

#ifndef  _swStruct_H_
#define  _swStruct_H_
#include <sys/timeb.h>
struct _portattrib
{
  long* plTranid;
  long* plSafid;
  short	iQid;
  short	iMsgformat;
  int	iSemtranid;
  int	iSemsafid;
  char  a1[4];
} sgPortattrib;

/* D401应用报文控制头 */
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
   /*del by zcd 20141220
  short	iFmtgroup;
  ***end of del by zcd 20141220*/
  /*add by zcd 20141220*/
  long iFmtgroup; 
  /*end of add by zcd 20141220*/
  short	iRevmode;
  /*del by zcd 20141220
  short	iRevfmtgroup;
  short iSwitchgroup;
  ***end of del by zcd 20141220*/
  /*add by zcd 20141220*/
  long iRevfmtgroup;
  long iSwitchgroup;
  /*end of add by zcd 20141220*/
  
  short	iRouteline;
 /*modify by zcd 20141230
  short	iBodylen;*/
  int iBodylen;
  //char  cAgdebugf[64];   /*   add by tbl 20150320 */
  short	iMsgformat;
  char	cFormatter;
  char	cRouter;
  char	cCodetype;
  char	cEndflag;
  char	cSafflag;
  char	aBegin_trancode[10];
  char	aTrancode[10];
  /* bit 1: ='0'-存于共享内存 ='1' 存于数据库 */
  char  aSysattr[10];  /* add by nh 20020910 */
  char call_flag; /*add by baiqj20150319, PSBC_V1.0*/
  struct timeb  callstart; /*add by baiqj20150319, PSBC_V1.0*/
  char  timeconsum[16]; /*add by baiqj 20150319, PSBC_V1.0*/
  int log_level;        /*add by cjh 20150319, PSBC_V1.0*/
  char file_log_name[32]; /*add by cjh 20150319, PSBC_V1.0*/
  char	aMemo[20];
  char  a1[7];
};

/* D402内部报文变量域存储 */
struct imfbuf
{
  char aFldname[ iFLDNAMELEN]; /* iFLDNAMELEN为8的倍数 */
  /*modify by zcd 20141230
  short iFldlen;*/
  unsigned int iFldlen;
  char aFldvalue[ iPERFLDVALUELEN];
  char  a1[6];
};

/* D403有分隔符外部报文预解包变量域存储 */
struct sepbuf
{
  short	iRecid;
  short	iGrpid;
  short	iFldid;
  short iFldlen; /* add by qy 09.29 */
  char	aFldvalue[ iPERFLDVALUELEN];
};

/* D404 8583外部报文预解包变量域存储 */
struct isobuf
{
  /*short	iFldid;  del by baiqj20150413 PSBC_V1.0*/
  long	iFldid;  /* add by baiqj20150413 PSBC_V1.0*/
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

/* D405交换平台报文 */
struct msgpack
{
  struct msghead sMsghead;
  char	aMsgbody[ iMSGMAXLEN - sizeof(struct msghead) ];
};

/* 预解包定义 */
union preunpackbuf
{
  struct imfbuf sImfbuf;
  struct sepbuf sSepbuf;
  struct isobuf sIsobuf;
};
#endif
