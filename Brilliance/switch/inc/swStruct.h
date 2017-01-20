#ifndef  _swStruct_H_
#define  _swStruct_H_
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
  ***end of delete by zcd 20141220*/
  /*add by zcd 20141220*/
  long	iFmtgroup;
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
  char call_flag; /*add by baiqj, PSBC_V1.0*/
  struct timeb  callstart; /*add by baiqj20150319, PSBC_V1.0*/
  char timeconsum[16];  /*add by baiqj 20150319, PSBC_V1.0*/
  int log_level;        /*add by chenjihui 20150319, PSBC_V1.0*/
  char file_log_name[32]; /*add by chenjihui 20150319, PSBC_V1.0*/
  char	aMemo[20];
  char  a1[7];
};

/* D402�ڲ����ı�����洢 */
struct imfbuf
{
  char aFldname[ iFLDNAMELEN];
  short iFldlen;
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
  /*short	iFldid; del by baiqj20150413 PSBC_V1.0*/
  long	iFldid; /*add by baiqj20150413 PSBC_V1.0*/
  short	iFldlen;
  short iOrgfldlen; /* add by qy 09.29 */
  char	aFldvalue[ iPERFLDVALUELEN];
  char	aOrgfldlen[10];
  char	aOrgfldvalue[iPERFLDVALUELEN];
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
