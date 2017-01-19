/****************************************************************/
/* 模块编号	：DT_Func_Pub					*/
/* 模块名称	：常用功能类函数				*/
/* 版 本 号	：V1.0						*/
/* 作    者	：Hu						*/
/* 建立日期    	：2015/3/6 11:32:47				*/
/* 最后修改日期	：						*/
/* 模块用途	：把常用功能统一封装为小函数，以方便以后使用	*/
/* 本模块中包含如下函数及功能说明：				*/
/*	<01>._SystemExe		执行操作系统命令		*/
/*	<02>._bswVdebug		日终记录函数(_swVdebug)		*/
/*	<03>._bswTrim		去空格				*/
/*	<04>._GetProfileString	读配置文件			*/
/*	<05>._GetFormatTime	格式化时间串			*/
/*	<06>._longtodate	将整型日期转化为字符日期	*/
/*	<07>._dateTolong	将字符日期转化为整型日期	*/
/*	<08>._dateAddday	取日期减天数后的日期数		*/
/*	<09>._erTOshi		二进制转十进制			*/
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2015/3/27 15:35:34 XXXXXXX                               	*/
/****************************************************************/
#include "DT_Pub.h"


/**************************公共变量定义**************************/
char agDebugfile[128];
int  cgDebug;
//定义时间函数取分钟数
time_t sysTimeSec;

int 	rc=0;		//接受返回值(可以任意使用)
int 	Ret=0;		//接受返回值(使用时需重新赋值，以后返回正确值)


/**************************************************************
 ** 函数名      : _SystemExec
 ** 功  能      : 执行操作系统命令
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 16:42:47
 ** 最后修改日期: 2015/3/6 16:42:49
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 执行代码
 ** 返回值      : FAIL:失败. SUCC:成功
***************************************************************/
int _SystemExec(char *Buf)
{	
	/*
	signal(SIGQUIT,SIG_IGN);
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGSTOP,SIG_IGN);
    signal(SIGUSR1,SIG_IGN);
    signal(SIGUSR2,SIG_IGN);
    signal(SIGCHLD,SIG_IGN);
    signal(SIGSEGV,SIG_IGN);
    rc=system(Buf);
    raise(SIGSEGV); */
	return system(Buf);
}
/**************************************************************
 ** 函数名      : _SystemSignal
 ** 功  能      : 屏弊信号
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 16:42:47
 ** 最后修改日期: 2015/3/6 16:42:49
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 执行代码
 ** 返回值      : FAIL:失败. SUCC:成功
***************************************************************/
void _SystemSignal()
{	
	signal(SIGQUIT,SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGTSTP,SIG_IGN);
	signal(SIGSTOP,SIG_IGN);
	signal(SIGUSR1,SIG_IGN);
	signal(SIGUSR2,SIG_IGN);
	signal(SIGCHLD,SIG_IGN);
	signal(SIGSEGV,SIG_IGN);
	//rc=system(Buf);
	//raise(SIGSEGV);
}


/**************************************************************
 ** 函数名      : _bswVdebug
 ** 功  能      : 日志记录函数(指定文件名)
 ** 作  者      :
 ** 建立日期    : 2012-10-10
 ** 最后修改日期: 2012-12-12  指定了文件名为DEDebugfile
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : class:   输入:调试级别
 **               fmt:     输入:格式串
 ** 返回值      : 无
***************************************************************/
void _bswVdebug(int level, char *file, char *func, int lineno, char *fmt, ...)
/*
#ifdef _LINUXES_
	void bswVdebug(int level, char *file, char *func, int lineno, char *fmt, ...)
#else
	void bswVdebug(level,file, func, lineno, fmt, va_alist)
	int level;
	char *file;
	char *func;
	int lineno;
	char *fmt;
	va_dcl
#endif
*/
{
	FILE *fp = NULL;
	struct stat sgbuf;
	struct timeval tv;
	va_list ap;
	    char SWDebugBuffer[10240];
	    char bswDebugFile[255];
	
	gettimeofday(&tv, NULL);
	struct tm *ptm = localtime(&(tv.tv_sec));
	int rc = 0;
	char tmpname[256];
	
	if ((level > cgDebug) || (level < 0)) {
	return;
	}
	SWDebugBuffer[0] = '\0';
	if (fmt != NULL) {
	#ifdef _LINUXES_
	va_start(ap,fmt);
	#else
	va_start(ap);
	#endif
	vsnprintf(SWDebugBuffer, sizeof(SWDebugBuffer), fmt, ap);
	va_end(ap);
	if (strlen(SWDebugBuffer) > 10000) {
	  SWDebugBuffer[9997] = '.';
	  SWDebugBuffer[9998] = '.';
	  SWDebugBuffer[9999] = '.';
	  SWDebugBuffer[10000] = 0x00;
	}
	}
	snprintf(bswDebugFile, sizeof(bswDebugFile), "%s/%s", getenv(DAYEND_ENV), agDebugfile);
	if ((fp = fopen(bswDebugFile, "a+")) == NULL) {
	return;
	}
	if ((rc = stat(bswDebugFile, &sgbuf)) < 0) {
	fclose(fp);
	return;
	}
	/* 控制文件长度为15位 */
	if (sgbuf.st_size > MAX_FILE_SIZE) {
	snprintf(tmpname, sizeof(tmpname), "%s.%02d%02d%02d", bswDebugFile, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	rename(bswDebugFile, tmpname);
	}
	char alTmp[256];
	char *ptr = file;
	if (strlen(file) > 15) {
	memcpy(alTmp, ptr + strlen(file) - 15, 15);
	alTmp[15] = 0x00;
	ptr = alTmp;
	}
	char *pbuf = SWDebugBuffer;
	char cc;
	char flg;
	flg = 0;
	while (1) {
	if (strlen(pbuf) > MAX_LINE_BUF) {
	  /* 保存置空的字符 */
	  cc = *(pbuf + MAX_LINE_BUF) ;
	  *(pbuf + MAX_LINE_BUF) = 0x00;
	  flg = 1;
	}
	if (level != 0) {
	  fprintf(fp, "%02d-%02d|%02d:%02d:%02d|%06ld| %s\n", ptm->tm_mon + 1, \
	        ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec, pbuf);
	}
	else {
	  fprintf(fp, "%02d-%02d|%02d:%02d:%02d|%06ld|%15s|%04d| %s\n", ptm->tm_mon + 1, \
	        ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec, tv.tv_usec, ptr, lineno, pbuf);
	}
	if (flg) {
	  /* 恢复字符并移动指针 */
	  *(pbuf + MAX_LINE_BUF) = cc;
	  pbuf += MAX_LINE_BUF;
	  flg = 0;
	}
	    else break;
	}
	fclose(fp);
}



/**************************************************************
 ** 函数名      :  _GetFormatTime
 ** 功  能      : 格式化时间
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 11:34:27
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 时间,长度,格式
 ** 返回值      : 0：成功  非0：失败
***************************************************************/
int _GetFormatTime( char *FormatTime, int iLen, const char *Format )
{
	time_t	clock;
	int	iRc;

	clock = time ( (time_t *) 0 );
	iRc= strftime ( FormatTime, iLen , Format , (struct tm *)localtime ( &clock ) );
	return (iRc);
}


/**************************************************************
 ** 函数名      :  _GetProfileString
 ** 功  能      : 读配置文件取相应参数
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 11:34:27
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 文件名,[区间标识],变量名,变量值
 ** 返回值      : 0：成功  非0：失败
***************************************************************/
int _GetProfileString(char* FileName, char* Section, char* Index, char* GetValue)
{
	FILE *fp;
	int iSectionLen,iIndexLen,iFind;
	char buff[512];
	int i;

	iSectionLen=strlen(Section);
	iIndexLen=strlen(Index);

	/* 打开配置文件 */
	if ( ( fp = fopen (FileName,"r") ) == NULL )
		return (-1);
		
	/* 寻找匹配条件 */
	iFind =-2;
	while( fgets(buff,sizeof(buff) - 1, fp)) {
		if ( *buff == '#' || *buff == ';' )
			continue;

		if ( (*buff=='[') && (*(buff+iSectionLen+1)==']') && (!memcmp(buff+1, Section, iSectionLen)) ) {
			break;
		}
	}
	if (!iFind) {
		fclose(fp);
		return ( -2 );
	}

	/* 寻找匹配数据 */
	iFind =-3;
	while( fgets(buff,sizeof(buff),fp)) {
		if (*buff=='[' )
			break;
		if (*buff=='#'||*buff==';')
			continue;

		if ( (*buff==*Index) && ( *(buff+iIndexLen)=='=') &&(!memcmp(buff,Index,iIndexLen)) )
		{
			for (i = iIndexLen;i<= strlen(buff);i++){
			/* 
				if (buff[i] == ';'|| buff[i] == '#'|| buff[i]=='\n'){
			*/
				if (buff[i] == ' ' || buff[i] == '\n'){
					buff[i] = 0x00;
					break;
				}
    	}

			strcpy( GetValue,buff+iIndexLen+1);
			iFind = strlen(GetValue) ;
			GetValue[iFind]=0;
			break;
		}
	}
 	fclose(fp);
	return (iFind);
}



/**************************************************************
 ** 函数名      : _bswTrim
 ** 功  能      : 去空格
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 11:34:27
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 要去空格的字符串
 ** 返回值      : 0：成功  非0：失败
***************************************************************/
int _bswTrim(char *s)
{
	short i, l, r, len;
	for(len=0; s[len]; len++);
	for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
	if(l==len)
	{
		s[0]='\0';
		return 0;
	}
	for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
	for(i=l; i<=r; i++) s[i-l]=s[i];
	s[r-l+1]='\0';
	return 0;
}


/**************************************************************
 ** 函数名      : _longtodate
 ** 功  能      : 将整型日期转换为字符日期
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 11:34:27
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :      ldate:     输入：整型日期
 ** 返回值      : 字符日期
***************************************************************/
void _longTodate(long ldate,char *day)
{
	struct tm lm ;
	memcpy(&lm,localtime(&ldate),sizeof(lm));
	/* modify by zjj 2004.12.04 add  +1  */
	sprintf(day,"%04d%02d%02d",lm.tm_year+1900,lm.tm_mon + 1,lm.tm_mday);
}

/**************************************************************
 ** 函数名      : _dateTolong
 ** 功  能      : 字符日期将转换为整型日期
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 11:34:27
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 输入：日期
 ** 返回值      : 字符日期
***************************************************************/
void _dateTolong(char *aTime, long *lTime)
{
	struct tm tmT;
	char alYear[5];
	char alMon[3];
  char alDate[3];
	char alHour[2],alMin[3],alSec[3];
 
	memcpy(alYear,aTime,4);
	alYear[4] = '\0';
	memcpy(alMon,aTime+4,2);
	alMon[2] = '\0';
	memcpy(alDate,aTime+6,2);
	alDate[2] = '\0';

	tmT.tm_year=atoi(alYear)-1900;
	tmT.tm_mon=atoi(alMon)-1;
	tmT.tm_mday=atoi(alDate);

	tmT.tm_hour=0;
	tmT.tm_min=0;
	tmT.tm_sec=0;
	*lTime=mktime(&tmT);	
}


/********************************************************
* 函 数 名: _dateAddday()
* 功    能: 取得一日期减上天数后的日期数
* 作    者: Hu
* 建立日期:
* 修改日期:
* 输    入: 原日期,天数,加上天数,新的日期;
* 输    出:  0：成功  非0：失败
*********************************************************/
int  _dateAddday(char *oldday, int days, char *ndate)
{
	char day[9];
	long lDay;
	memset(day, 0x00, 9);
	_dateTolong(oldday, &lDay);
	_longTodate(lDay-3600*24*days, day);
	sprintf(ndate, "%8s", day);
	return(0);
}

/**************************************************************
 ** 函数名      : _erTOshi
 ** 功  能      : 二进制转十进制
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/25 10:34:21
 ** 最后修改日期: 2015/3/25 10:34:23
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 输入:要转换的数字字符串
 **               输入:转换后的值
 ** 返回值      : 无
***************************************************************/
int _erTOshi(char *str) 
{ 
	int i,j=0,k=0;
	for(i=strlen(str)-1;i>=0;i--)
		k+=pow(2,j++)*(str[i]-'0'); 
	return k;
} 

/**************************************************************
 ** 函数名      : _isEnvPasswd
 ** 功  能      : 判断用户密码是否为环境变量
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/30 17:54:15
 ** 最后修改日期: 2015/3/30 17:54:17
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 输入:要转换字符串
 **               输入:解密环境变量后的值
 ** 返回值      : 0：成功  非0：失败
***************************************************************/
int _isEnvPasswd(char *agEnvPasswd)
{
	char alEnvPasswd[32];
	/*处理数据库密码：如果第1位=$,第二位={,最后一位=}，则判定其为环境变量，直接取环境变量值，并重新赋值.*/	
	if(agEnvPasswd[0]=='$' && agEnvPasswd[1]=='{' && agEnvPasswd[strlen(agEnvPasswd)-1]=='}')
	{
		_swVdebug(0, "_isEnvPasswd:要解密的环境变量串=【%s】",agEnvPasswd);
		INIT(alEnvPasswd);
		memcpy(alEnvPasswd,agEnvPasswd+2,strlen(agEnvPasswd)-3);
		INIT(agEnvPasswd);
		if(getenv(alEnvPasswd)==NULL)
			return FAIL;
		sprintf(agEnvPasswd,"%s",getenv(alEnvPasswd));
		//_swVdebug(0, "_isEnvPasswd:解密后的密码为=【%s】",agEnvPasswd);
	}
		
	return SUCC;
}
