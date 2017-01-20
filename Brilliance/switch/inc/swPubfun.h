#ifndef  _swPubfun_H_
#define  _swPubfun_H_
#endif
/*邮箱的信息*/

struct mbinfo
{
  short iFlag;		   /*邮箱启用标志*/
  short iConnect;          /*连接数*/
  long  lSendnum;          /*发送消息数*/
  long  lRecvnum;          /*接收消息数*/
  long  lPendnum;          /*当前消息数*/
  long  lSendtime;         /*最后发送时间*/
  long  lRecvtime;         /*最后接收时间*/
  long  lSendpid;          /*最后发送进程*/
  long  lRecvpid;          /*最后接收进程*/
  long  lQueue_head;       /*消息链表头*/
  long  lQueue_tail;       /*消息链表尾*/
  short iConn_head;        /*连接链表头*/
  short iConn_tail;        /*连接链表尾*/
  long  lExisttime;        /*报文驻留时间*/
  long  lMaxqueue;         /*邮箱阀值*/
  short iHload;            /*上负载点*/
  short iLload;            /*下负载点*/
  short iTimes;            /*临界点持续次数*/
  short iStatus;           /*状态标志*/
  short iLastnum;          /*负载状态持续次数*/
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
[语  法]    _swVersion(char *cmArgv1,char *cmArgv2)
[说  明]    显示版本号                  
[返回值]                
*******************************************************************/

extern int _swMatch( char *, char * );
/**************************************************************
[语  法]  int _swMatch(char *regexpress, char *str)
[说  明]  str字符串是否匹配regexpress字符串所定义的正则表达式,
          
[返回值]  TRUE:匹配 FALSE:不匹配
***************************************************************/

extern int _swTrim( char* );
/**************************************************************
[语  法]  int  _swTrim(char *s)
[说  明]  该函数将字符串 s 的前后空格及尾回车去掉
[返回值]  无
***************************************************************/

extern int _swTrimL( char* );
/**************************************************************
[语  法]  int  _swTrimL(char *s)
[说  明]  该函数将字符串 s 的左边空格回车去掉
[返回值]  无
***************************************************************/

extern int _swTrimR( char* );
/**************************************************************
[语  法]  int  _swTrimR(char *s)
[说  明]  该函数将字符串 s 的右边空格回车去掉
[返回值]  无
***************************************************************/

extern int _swGetTime(char *);
/**************************************************************
[语  法]  int _swGetTime(char *date)
[说  明]  取当前系统时间
[返回值]  无
***************************************************************/

extern int _swItoa( int , char* );
/**************************************************************
[语  法]  _swItoa(int n,char s[])
[说  明]  把整型n转换为字符串s        
[返回值]  无
***************************************************************/

extern int _swUdf_ltoa( long, char * );
/**************************************************************
[语  法]  int _swUdf_ltoa(long n, char s[])
[说  明]  long integer to strings
[返回值]  无
***************************************************************/

extern int _swStrToDate( char*, char* );
/**************************************************************
[语  法]  int  _swStrToDate(char *date1,char *date2)
[说  明]  八位日期date1(YYYYMMDD)转化为十位日期date2(YYYY/MM/DD)
[返回值]  -1:年的输入不对 | -2:月的输入不对 | -3:日的输入不对 |
           0:转换成功
***************************************************************/

extern int _swDateToStr( char* , char* );
/**************************************************************
[语  法]  int  _swDateToStr(char *date1,char *date2)
[说  明]  十位日期date1(YYYY/MM/DD)转化为八位日期date2(YYYYMMDD)
[返回值]  -1:年的输入不对 | -2:月的输入不对 | -3:日的输入不对 |
           0:转换成功
***************************************************************/

extern int _swStrCmp( char* , char* );
/**************************************************************
[语  法]  int  _swStrCmp(char *s1,char *s2)
[说  明]  在字符串s1中查找s2
[返回值]  -1:s1的长度小于s1的长度,不能查找 | 0:字符串s1中没有s2	
	  | 成功则返回s2在s1中的个数.
***************************************************************/

extern int _swStrReplace( char* , char* , char* ,char* );
/**************************************************************
[语  法]  int  _swStrReplace(char *s1,char *s2,char *s3,char *s4)
	  (要求strlen(s1)==strlen(s4) ; strlen(s2)=strlen(s3))
[说  明]将s1中的s2替换成指定的s3.,拷贝到字符串s4
[返回值]-2: strlen(s2) != strlen(s3) | -1: s1中没有s2  |
	  :成功则返回替换的个数
***************************************************************/

extern int swDberror(char *);
/**************************************************************
[语  法]  int swDberror(char *msgpack);
[说  明]  数据库错误处理.把错误信息记入debug文件,同时向监控发报文
          (msgpack). 若没有错误,则不记debug文件,不发监控报文.
          msgpack:报文(若不传递报文,输入(char *) 0 )
[返回值]  操作数据库的返回值
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
[语  法]  int swDebug(char *frm,va_alist)
[说  明]  按照frm的格式把va_alist的内容记跟踪日志.
[返回值]  -1:操作日志文件出错  | 0:成功
***************************************************************/

extern int swMberror(int ,char *);
/**************************************************************
[语  法]  int swMberror(int rc,char *msgpack)
[说  明]  邮箱错误处理.把错误信息码rc记入debug文件,同时向监控发报文
          msgpack 若没有错误,则不记debug文件,不发监控报文.
[返回值]  rc :邮箱操作返回码
***************************************************************/

extern int swDebughex(char *,unsigned int);
/**************************************************************
[语  法]  int swDebughex(char *msghead,short len)
[说  明]  打印报文msghead的十六进制到日志文件 .len是报文的长度
[返回值]  -1:不能打开日志文件  |  0:成功
***************************************************************/

extern int swDebugmsghead(char *);
/**************************************************************
[语  法]  int swDebugmsghead(char *bufhead)
[说  明]  打印报文头bufhead到日志文件.
[返回值]  -1:不能打开日志文件  |  0:成功
***************************************************************/

extern int _swMsgsend(long ,char * );
/**************************************************************
[语  法]  int _swMsgsend(long msgcode,char * msgpack)
[说  明]  本函数用于向监控模块发信息报文msgpack,msgcode是信息码 
[返回值]  成功返回:SUCCESS  否则返回qwrite的返回码
***************************************************************/

extern int _swOrdsend(long ,char * ,short);
/**************************************************************
[语  法]  int _swOrdsend(long msgcode,char * msgpack,short q_id)
[说  明]  本函数用于向相关模块发命令报文msgpack,msgcode是信息码 
          q_id为相关模块的邮箱
[返回值]  成功返回:SUCCESS  否则返回qwrite的返回码
***************************************************************/

extern int _swGetOneField(char *Buffer,int n,char *Field,char Seperator);
/**************************************************************
[语  法]  int _swGetOneField(char *Buffer,int n,char *Field,char Sep)
[说  明]  从 Buffer 中取第n个字段放入Field中。每个字段以Sep为分隔符
          n 从 1 开始。
[返回值]  返回Field的长度
***************************************************************/

extern int _swDebugflag(char *);
/**************************************************************
[语  法]  int _swDebugflag(int flag)
[说  明]  本函数用于在文件($HOME)/ini/debug.ini 中查找各模块的
          debug标志, 并把标志返回                
[返回值]  0:不写debug文件  | 1:写debug文件 | FAIL:取标志失败
***************************************************************/

extern int swDbopen();
/**************************************************************
[语  法]  int swDbopen()
[说  明]  本函数用于打开数据库switch                   
[返回值]  成功返回 0  | 失败返回 sqlca.sqlcode.
***************************************************************/

extern int swDbclose();
/**************************************************************
[语  法]  int swDbclose()
[说  明]  本函数用于关闭数据库switch                   
[返回值]  成功返回 0  | 失败返回 sqlca.sqlcode.
***************************************************************/

extern int swFmlunpack(char *,unsigned int,union preunpackbuf *);
/**************************************************************
[语  法]  int swFmlunpack(char *msgbody,short msglen,
                          union preunpackbuf *prebuf)
[说  明]  将FML报文条msgbody(不含报文头)拆包至预解包prebuf中.
          msglen是传入报文的长度
[返回值]  成功:0  |  失败(长度超长):-1 
***************************************************************/

extern int swFmlpack(union preunpackbuf *,char *,unsigned int*);
/**************************************************************
[语  法]  int swFmlpack(union preunpackbuf *prebuf,
                        char *msgbody,short *msglen)
[说  明]  将预解包prebuf打包至FML报文msgbody, msglen是FML报文的长度
[返回值]  成功:0  | 失败(长度超长):-1 
***************************************************************/

extern int swFmlget(union preunpackbuf *,char *,short *,char *);
/**************************************************************
[语  法]  int swFmlget(union preunpackbuf *prebuf,
                       char *fldname,short *fldlen,char *fldval)
[说  明]  得到FML预包prebuf中指定域名fldname的长度fldlen和域值fldval
[返回值]  成功:0   |  失败(长度超长):-1
***************************************************************/

extern int swFmlset(char *,short,char *,union preunpackbuf *);
/**************************************************************
[语  法]  int swFmlget(char *fldname,short fldlen,char *fldval,
                       union preunpackbuf prebuf)
[说  明]  设置FML预包prebuf中指定域名fldname的长度fldlen和域值fldval
[返回值]  成功:0  |  失败(长度超长):-1 
***************************************************************/

extern int _swFmlunion(union preunpackbuf *,union preunpackbuf *);
/**************************************************************
[语  法]  int _swFmlunion(union preunpackbuf *prebuf1,
                         union preunpackbuf *prebuf2);
[说  明]  对2个FML预解包取并集,并以第2个预解包为主. 
          prebuf1预解包1(存放结果); prebuf2预解包2(为主)
[返回值]  成功:0  |  失败(长度超长):-1 
***************************************************************/

extern int swDebugfml(char *);
/**************************************************************
[语  法]  int  _swDebugfml(char *msgpack)
[说  明]  在跟踪文件中打印FML报文的内容,msgpack	传入的报文(含报文头)
[返回值]  SUCCESS |  FAIL
***************************************************************/

extern int swGetlog(char *, unsigned int, long, char *);
/**************************************************************
[语  法]  int swGetlog(char * logfile,short loglen,long logoffset,char *msgpack)
[说  明]  根据报文在交易日志文件中的起始偏移量和报文的长度,
          得到交易流水日志中的报文
[返回值]  SUCCESS |  FAIL
***************************************************************/

extern int swPutlog(char *, unsigned int, long *, char *);
/**************************************************************
[语  法]  int swPutlog(char *msgpack,short loglen,long *logoffset,char * logfile)
[说  明]  将传入的报文内容存放到交易日志文件中,并返回报文存放
          的起始偏移位置
[返回值]  SUCCESS |  FAIL
***************************************************************/

extern int  _swClrMb(int);
/**************************************************************
[语  法]  int  _swClrMb(int iQid)
[说  明]  清邮箱
[返回值]  SUCCESS |  FAIL
***************************************************************/

int _swTimeLongToChs( long , char * ,char * );
/*******************************************************************
[语  法]    _swTimeLongToChs(lDateTime,pcDateTime,pcFormat)
[说  明]    将长整型的时间转换为字符串型                  
              iDateTime 长整型的时间                        
              pcFormat 时间的格式  pcDateTime 字符串型的时间              
[返回值]                
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
[语  法]  int swVdebug(char *frm,va_alist)
[说  明]  按照frm的格式把va_alist的内容记跟踪日志.
[返回值]  -1:操作日志文件出错  | 0:成功
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

/* 读取配置文件的操作函数 */
extern int swMacrorepl(char *);     /* 宏替换函数 */
extern short  swChgvalue(char *aValue);
extern short  swGetitemmode1(FILE *fp, char paKey[][iFLDVALUELEN],
	char paFldname[][iFLDNAMELEN], char paFldvalue[][iFLDVALUELEN]);
extern short  swGetvaluemode1(char paFldname[][iFLDNAMELEN],
	char paFldvalue[][iFLDVALUELEN], char *aFldname, char *aFldvalue);
extern short  swGetitemmode2(FILE *fp, char paFldvalue[][iFLDVALUELEN]);
extern int _swExpress(char *aExpress, char *aResult, short *piResultlen);
extern int _swExstr(char *alStr,char *cFlag);

/* UDP操作函数 */
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

/* 新晨Q API函数 */
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
/* 新晨Q API函数 */
