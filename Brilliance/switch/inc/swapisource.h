/*modified by cjh 20141220 ��ˮ������32λ�汾 8��9��Ϊ7��9*/
/*#define lMAXSERNO	99999999L*/      /* �����ˮ�� */
/*#define lMAXSERNO	9999999L*/       /* �����ˮ�� */
/*modified by cjh 20150408 ��ˮ������32λ�汾 7��9��Ϊ12��9 */
#define lMAXSERNO	999999999999L       /* �����ˮ�� */

extern	char    agDebugfile[64];                /* �����ļ��� */

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
  char aFldname[ iFLDNAMELEN]; /* iFLDNAMELENΪ8�ı��� */
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
  short iFldlen; /* add by qy 09.29 */
  char	aFldvalue[ iPERFLDVALUELEN];
};

/* D404 8583�ⲿ����Ԥ���������洢 */
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
#endif
