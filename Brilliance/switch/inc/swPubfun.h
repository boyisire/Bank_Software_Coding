#ifndef  _swPubfun_H_
#define  _swPubfun_H_
#endif
/*�������Ϣ*/

struct mbinfo
{
  short iFlag;		   /*�������ñ�־*/
  short iConnect;          /*������*/
  long  lSendnum;          /*������Ϣ��*/
  long  lRecvnum;          /*������Ϣ��*/
  long  lPendnum;          /*��ǰ��Ϣ��*/
  long  lSendtime;         /*�����ʱ��*/
  long  lRecvtime;         /*������ʱ��*/
  long  lSendpid;          /*����ͽ���*/
  long  lRecvpid;          /*�����ս���*/
  long  lQueue_head;       /*��Ϣ����ͷ*/
  long  lQueue_tail;       /*��Ϣ����β*/
  short iConn_head;        /*��������ͷ*/
  short iConn_tail;        /*��������β*/
  long  lExisttime;        /*����פ��ʱ��*/
  long  lMaxqueue;         /*���䷧ֵ*/
  short iHload;            /*�ϸ��ص�*/
  short iLload;            /*�¸��ص�*/
  short iTimes;            /*�ٽ���������*/
  short iStatus;           /*״̬��־*/
  short iLastnum;          /*����״̬��������*/
};

/*
extern int32  qattach(short);
extern int32  qread(char *, short *, short *);
extern int32  qname2id(char *, short *);
extern int32  qname2id(char *, short *);
extern int32  qdetach(void);
extern int32  qattach2(short);
extern int32  qwrite2(char *, short, short, short, short, short);
extern int32  qread2(char *, short *, short *, short *, short *, short *);
extern int32  qfilter( int32, int32, int32, int32 * );
extern int32  qpending(short, short *);
extern int32  qwrite3(char *, short, short, short, short, short, short);
extern int32  qread3(char *,short *,short *,short *,short *,short *,short *);
*/

extern int  qattach(short);
extern int  qwrite(char *, unsigned int , short);
extern int  qreadnw(char *, unsigned int *, short *,short *, short *, short *,short);
extern int  qdetach();
extern int  qwrite2(char *, unsigned int , short, short, short, short);
extern int  qread2(char *, unsigned int *, short *, short *, short *, short *);
extern int  qwrite3(char *, unsigned int , short, short, short, short, short);
extern int  qread3(char *,unsigned int *,short *,short *,short *,short *,short *);
extern int qread3nw(char *msg_area, unsigned int *msg_size, short *qid,short *grpid,short *mpriority, short *mclass, short *mtype, short wtime);
extern int _qwrite2(char *msg_area, unsigned int msg_size, short qid,short mpriority, short mclass, short mtype);



int _swVersion(char *cmArgv1,char *cmArgv2);
/*******************************************************************
[��  ��]    _swVersion(char *cmArgv1,char *cmArgv2)
[˵  ��]    ��ʾ�汾��                  
[����ֵ]                
*******************************************************************/

extern int _swMatch( char *, char * );
/**************************************************************
[��  ��]  int _swMatch(char *regexpress, char *str)
[˵  ��]  str�ַ����Ƿ�ƥ��regexpress�ַ����������������ʽ,
          
[����ֵ]  TRUE:ƥ�� FALSE:��ƥ��
***************************************************************/

extern int _swTrim( char* );
/**************************************************************
[��  ��]  int  _swTrim(char *s)
[˵  ��]  �ú������ַ��� s ��ǰ��ո�β�س�ȥ��
[����ֵ]  ��
***************************************************************/

extern int _swTrimL( char* );
/**************************************************************
[��  ��]  int  _swTrimL(char *s)
[˵  ��]  �ú������ַ��� s ����߿ո�س�ȥ��
[����ֵ]  ��
***************************************************************/

extern int _swTrimR( char* );
/**************************************************************
[��  ��]  int  _swTrimR(char *s)
[˵  ��]  �ú������ַ��� s ���ұ߿ո�س�ȥ��
[����ֵ]  ��
***************************************************************/

extern int _swGetTime(char *);
/**************************************************************
[��  ��]  int _swGetTime(char *date)
[˵  ��]  ȡ��ǰϵͳʱ��
[����ֵ]  ��
***************************************************************/

extern int _swItoa( int , char* );
/**************************************************************
[��  ��]  _swItoa(int n,char s[])
[˵  ��]  ������nת��Ϊ�ַ���s        
[����ֵ]  ��
***************************************************************/

extern int _swUdf_ltoa( long, char * );
/**************************************************************
[��  ��]  int _swUdf_ltoa(long n, char s[])
[˵  ��]  long integer to strings
[����ֵ]  ��
***************************************************************/

extern int _swStrToDate( char*, char* );
/**************************************************************
[��  ��]  int  _swStrToDate(char *date1,char *date2)
[˵  ��]  ��λ����date1(YYYYMMDD)ת��Ϊʮλ����date2(YYYY/MM/DD)
[����ֵ]  -1:������벻�� | -2:�µ����벻�� | -3:�յ����벻�� |
           0:ת���ɹ�
***************************************************************/

extern int _swDateToStr( char* , char* );
/**************************************************************
[��  ��]  int  _swDateToStr(char *date1,char *date2)
[˵  ��]  ʮλ����date1(YYYY/MM/DD)ת��Ϊ��λ����date2(YYYYMMDD)
[����ֵ]  -1:������벻�� | -2:�µ����벻�� | -3:�յ����벻�� |
           0:ת���ɹ�
***************************************************************/

extern int _swStrCmp( char* , char* );
/**************************************************************
[��  ��]  int  _swStrCmp(char *s1,char *s2)
[˵  ��]  ���ַ���s1�в���s2
[����ֵ]  -1:s1�ĳ���С��s1�ĳ���,���ܲ��� | 0:�ַ���s1��û��s2	
	  | �ɹ��򷵻�s2��s1�еĸ���.
***************************************************************/

extern int _swStrReplace( char* , char* , char* ,char* );
/**************************************************************
[��  ��]  int  _swStrReplace(char *s1,char *s2,char *s3,char *s4)
	  (Ҫ��strlen(s1)==strlen(s4) ; strlen(s2)=strlen(s3))
[˵  ��]��s1�е�s2�滻��ָ����s3.,�������ַ���s4
[����ֵ]-2: strlen(s2) != strlen(s3) | -1: s1��û��s2  |
	  :�ɹ��򷵻��滻�ĸ���
***************************************************************/

extern int swDberror(char *);
/**************************************************************
[��  ��]  int swDberror(char *msgpack);
[˵  ��]  ���ݿ������.�Ѵ�����Ϣ����debug�ļ�,ͬʱ���ط�����
          (msgpack). ��û�д���,�򲻼�debug�ļ�,������ر���.
          msgpack:����(�������ݱ���,����(char *) 0 )
[����ֵ]  �������ݿ�ķ���ֵ
***************************************************************/

/*"ifdef" modified by qh 20070416*/
#ifdef OS_AIX
extern int swDebug(frm, va_list);
char *frm
va_dcl
#endif
#ifdef OS_HPUX
extern int swDebug(char *frm, ...);
#endif
#ifdef OS_LINUX
extern int swDebug(char *frm, ...);
#endif
#ifdef OS_SCO
extern int swDebug(char *frm, ...);
#endif
#ifdef OS_SOLARIS
extern int swDebug(char *frm, ...);
#endif
/**************************************************************
[��  ��]  int swDebug(char *frm,va_alist)
[˵  ��]  ����frm�ĸ�ʽ��va_alist�����ݼǸ�����־.
[����ֵ]  -1:������־�ļ�����  | 0:�ɹ�
***************************************************************/

extern int swMberror(int ,char *);
/**************************************************************
[��  ��]  int swMberror(int rc,char *msgpack)
[˵  ��]  ���������.�Ѵ�����Ϣ��rc����debug�ļ�,ͬʱ���ط�����
          msgpack ��û�д���,�򲻼�debug�ļ�,������ر���.
[����ֵ]  rc :�������������
***************************************************************/

extern int swDebughex(char *,unsigned int);
/**************************************************************
[��  ��]  int swDebughex(char *msghead,short len)
[˵  ��]  ��ӡ����msghead��ʮ�����Ƶ���־�ļ� .len�Ǳ��ĵĳ���
[����ֵ]  -1:���ܴ���־�ļ�  |  0:�ɹ�
***************************************************************/

extern int swDebugmsghead(char *);
/**************************************************************
[��  ��]  int swDebugmsghead(char *bufhead)
[˵  ��]  ��ӡ����ͷbufhead����־�ļ�.
[����ֵ]  -1:���ܴ���־�ļ�  |  0:�ɹ�
***************************************************************/

extern int _swMsgsend(long ,char * );
/**************************************************************
[��  ��]  int _swMsgsend(long msgcode,char * msgpack)
[˵  ��]  ��������������ģ�鷢��Ϣ����msgpack,msgcode����Ϣ�� 
[����ֵ]  �ɹ�����:SUCCESS  ���򷵻�qwrite�ķ�����
***************************************************************/

extern int _swOrdsend(long ,char * ,short);
/**************************************************************
[��  ��]  int _swOrdsend(long msgcode,char * msgpack,short q_id)
[˵  ��]  ���������������ģ�鷢�����msgpack,msgcode����Ϣ�� 
          q_idΪ���ģ�������
[����ֵ]  �ɹ�����:SUCCESS  ���򷵻�qwrite�ķ�����
***************************************************************/

extern int _swGetOneField(char *Buffer,int n,char *Field,char Seperator);
/**************************************************************
[��  ��]  int _swGetOneField(char *Buffer,int n,char *Field,char Sep)
[˵  ��]  �� Buffer ��ȡ��n���ֶη���Field�С�ÿ���ֶ���SepΪ�ָ���
          n �� 1 ��ʼ��
[����ֵ]  ����Field�ĳ���
***************************************************************/

extern int _swDebugflag(char *);
/**************************************************************
[��  ��]  int _swDebugflag(int flag)
[˵  ��]  �������������ļ�($HOME)/ini/debug.ini �в��Ҹ�ģ���
          debug��־, ���ѱ�־����                
[����ֵ]  0:��дdebug�ļ�  | 1:дdebug�ļ� | FAIL:ȡ��־ʧ��
***************************************************************/

extern int swDbopen();
/**************************************************************
[��  ��]  int swDbopen()
[˵  ��]  ���������ڴ����ݿ�switch                   
[����ֵ]  �ɹ����� 0  | ʧ�ܷ��� sqlca.sqlcode.
***************************************************************/

extern int swDbclose();
/**************************************************************
[��  ��]  int swDbclose()
[˵  ��]  ���������ڹر����ݿ�switch                   
[����ֵ]  �ɹ����� 0  | ʧ�ܷ��� sqlca.sqlcode.
***************************************************************/

extern int swFmlunpack(char *,unsigned int,union preunpackbuf *);
/**************************************************************
[��  ��]  int swFmlunpack(char *msgbody,short msglen,
                          union preunpackbuf *prebuf)
[˵  ��]  ��FML������msgbody(��������ͷ)�����Ԥ���prebuf��.
          msglen�Ǵ��뱨�ĵĳ���
[����ֵ]  �ɹ�:0  |  ʧ��(���ȳ���):-1 
***************************************************************/

extern int swFmlpack(union preunpackbuf *,char *,unsigned int*);
/**************************************************************
[��  ��]  int swFmlpack(union preunpackbuf *prebuf,
                        char *msgbody,short *msglen)
[˵  ��]  ��Ԥ���prebuf�����FML����msgbody, msglen��FML���ĵĳ���
[����ֵ]  �ɹ�:0  | ʧ��(���ȳ���):-1 
***************************************************************/

extern int swFmlget(union preunpackbuf *,char *,short *,char *);
/**************************************************************
[��  ��]  int swFmlget(union preunpackbuf *prebuf,
                       char *fldname,short *fldlen,char *fldval)
[˵  ��]  �õ�FMLԤ��prebuf��ָ������fldname�ĳ���fldlen����ֵfldval
[����ֵ]  �ɹ�:0   |  ʧ��(���ȳ���):-1
***************************************************************/

extern int swFmlset(char *,short,char *,union preunpackbuf *);
/**************************************************************
[��  ��]  int swFmlget(char *fldname,short fldlen,char *fldval,
                       union preunpackbuf prebuf)
[˵  ��]  ����FMLԤ��prebuf��ָ������fldname�ĳ���fldlen����ֵfldval
[����ֵ]  �ɹ�:0  |  ʧ��(���ȳ���):-1 
***************************************************************/

extern int _swFmlunion(union preunpackbuf *,union preunpackbuf *);
/**************************************************************
[��  ��]  int _swFmlunion(union preunpackbuf *prebuf1,
                         union preunpackbuf *prebuf2);
[˵  ��]  ��2��FMLԤ���ȡ����,���Ե�2��Ԥ���Ϊ��. 
          prebuf1Ԥ���1(��Ž��); prebuf2Ԥ���2(Ϊ��)
[����ֵ]  �ɹ�:0  |  ʧ��(���ȳ���):-1 
***************************************************************/

extern int swDebugfml(char *);
/**************************************************************
[��  ��]  int  _swDebugfml(char *msgpack)
[˵  ��]  �ڸ����ļ��д�ӡFML���ĵ�����,msgpack	����ı���(������ͷ)
[����ֵ]  SUCCESS |  FAIL
***************************************************************/

extern int swGetlog(char *, unsigned int, long, char *);
/**************************************************************
[��  ��]  int swGetlog(char * logfile,short loglen,long logoffset,char *msgpack)
[˵  ��]  ���ݱ����ڽ�����־�ļ��е���ʼƫ�����ͱ��ĵĳ���,
          �õ�������ˮ��־�еı���
[����ֵ]  SUCCESS |  FAIL
***************************************************************/

extern int swPutlog(char *, unsigned int, long *, char *);
/**************************************************************
[��  ��]  int swPutlog(char *msgpack,short loglen,long *logoffset,char * logfile)
[˵  ��]  ������ı������ݴ�ŵ�������־�ļ���,�����ر��Ĵ��
          ����ʼƫ��λ��
[����ֵ]  SUCCESS |  FAIL
***************************************************************/

extern int  _swClrMb(int);
/**************************************************************
[��  ��]  int  _swClrMb(int iQid)
[˵  ��]  ������
[����ֵ]  SUCCESS |  FAIL
***************************************************************/

int _swTimeLongToChs( long , char * ,char * );
/*******************************************************************
[��  ��]    _swTimeLongToChs(lDateTime,pcDateTime,pcFormat)
[˵  ��]    �������͵�ʱ��ת��Ϊ�ַ�����                  
              iDateTime �����͵�ʱ��                        
              pcFormat ʱ��ĸ�ʽ  pcDateTime �ַ����͵�ʱ��              
[����ֵ]                
*******************************************************************/
/*add by gxz */

/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
#define swVdebug(level, ...) _swVdebug(level, __FILE__, __LINE__, __VA_ARGS__)

/*"ifdef" modified by qh 20070416*/
#ifdef OS_AIX
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
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
#ifdef OS_LINUX
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
#ifdef OS_SCO
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
#ifdef OS_SOLARIS
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* extern int swVdebug(short iDebug,char *frm, ...); */ 
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
extern int _swVdebug(short iDebug,char *filename,int line,char *frm, ...);
#endif
/**************************************************************
[��  ��]  int swVdebug(char *frm,va_alist)
[˵  ��]  ����frm�ĸ�ʽ��va_alist�����ݼǸ�����־.
[����ֵ]  -1:������־�ļ�����  | 0:�ɹ�
***************************************************************/
/*add by gxz*/
/*modify by zcd 20141230*/
extern int swFmlpackset(char *aMsgbody, int *piMsglen, char *aFldname, short iFldlen, char *aFldval);  /* modi ZH */
extern int _swAtoE(unsigned char *d,unsigned char *s,unsigned int len);
extern int _swEtoA(unsigned char *d,unsigned char *s,unsigned int len);

extern short _swPortset(short, short, short);
extern short _swPortget(short, short, short *);
/*modify by zcd 20141230*/
extern int swFmlpackget(char *, int, char *, short *, char *); /* modi ZH */
extern short swPacksep(char *, unsigned int, char *, char *, char *, short, short, short, char *);
extern short _swChgvalue(char *aValue);
extern short _swGetitemmode2(FILE *fp, char paFldvalue[][iFLDVALUELEN], short iFldmaxnum);
extern short _swGetitemmode1(FILE *fp, char paKey[][iFLDVALUELEN],char paFldname[][iFLDNAMELEN], char paFldvalue[][iFLDVALUELEN]);
extern short _swGetvaluemode1(char paFldname[][iFLDNAMELEN],char paFldvalue[][iFLDVALUELEN], const char *aFldname, char *aFldvalue);
extern int swHextoasc( char *InBuf, char *OutBuf, short * Outlen);
extern int swPortget(short iPort, short iLevel, short *iStatus);
extern int swPortset(short iPort, short iLevel, short iStatus);
extern short _swChgvalue(char *aValue);
extern int _swLicense(char *);

/* ��ȡ�����ļ��Ĳ������� */
extern int swMacrorepl(char *);     /* ���滻���� */
extern short  swChgvalue(char *aValue);
extern short  swGetitemmode1(FILE *fp, char paKey[][iFLDVALUELEN],
	char paFldname[][iFLDNAMELEN], char paFldvalue[][iFLDVALUELEN]);
extern short  swGetvaluemode1(char paFldname[][iFLDNAMELEN],
	char paFldvalue[][iFLDVALUELEN], char *aFldname, char *aFldvalue);
extern short  swGetitemmode2(FILE *fp, char paFldvalue[][iFLDVALUELEN]);
extern int _swExpress(char *aExpress, char *aResult, short *piResultlen);
extern int _swExstr(char *alStr,char *cFlag);

/* UDP�������� */
int _swUDPCreate(int);
int _swUDPClose(int);
int _swUDPGet(int ,char *,int *,char*,unsigned int*);
/*int _swUDPPut(int ,char *,short ,char *,short );*/
int _swUDPPut(int ,char *,int ,char *,unsigned int );

int swSepget(union preunpackbuf *prebuf,short iRecid,short iGrpid,short iFldid,
  short *fldlen,char *fldval);
int swSepset(short iRecid,short iGrpid,short iFldid,
  short fldlen,char *fldval,union preunpackbuf *prebuf);
/*int swIsoget(union preunpackbuf *prebuf,short iFldid,short *fldlen,char *fldval); del by baiqj20150413 PSBC_V1.0*/
int swIsoget(union preunpackbuf *prebuf,long iFldid,short *fldlen,char *fldval); /*add by baiqj20150413 PSBC_V1.0*/
/*int swIsoset(short iFldid,short fldlen,char *fldval,char *orglenval,short orglen,char *orgfldval,union preunpackbuf *prebuf); del by baiqj20150413 PSBC_V1.0*/
int swIsoset(long iFldid,short fldlen,char *fldval,char *orglenval,short orglen,char *orgfldval,union preunpackbuf *prebuf);/* add by baiqj20150413 PSBC_V1.0*/

FILE *_swFopen(const char *file, const char *mode);
FILE *_swPopen(const char *path, const char *mode);
FILE *_swFopenlist(const char *file, const char *mode);

/* �³�Q API���� */
int bmqGetfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char *aMask,char *aMsgbuf,unsigned int*piMsglen);
int bmqGetmbinfo(short iMbid,struct mbinfo *plMbinfo);
int bmqClearmb(short iMbid);
int bmqPutfilter(short iGrpid,short iMbid,short iPrior,char *aFilter,char *aMsgbuf,unsigned int iMsglen);
int bmqGetwfilter(short *piGrpid,short *piMbid,short *piPrior,char *aFilter,char*aMask,char *aMsgbuf,unsigned int *piMsglen,short iTimeout);
int bmqOpen(short iMbid);
int bmqClose();
int bmqPut(short iGrpid,short iMbid,short iPrior,long lType,long lClass,char *aMsgbuf,unsigned int iMsglen);
int bmqGet(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,unsigned int *piMsglen);
int bmqGetw(short *piGrpid,short *piMbid,short *piPrior,long *plType,long *plClass,char *aMsgbuf,unsigned int *piMsglen,short iTimeout);
/* �³�Q API���� */
