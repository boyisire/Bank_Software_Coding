/****************************************************************/
/* ģ����	��DT_Func_Pub					*/
/* ģ������	�����ù����ຯ��				*/
/* �� �� ��	��V1.0						*/
/* ��    ��	��Hu						*/
/* ��������    	��2015/3/6 11:32:47				*/
/* ����޸�����	��						*/
/* ģ����;	���ѳ��ù���ͳһ��װΪС�������Է����Ժ�ʹ��	*/
/* ��ģ���а������º���������˵����				*/
/*	<01>._SystemExe		ִ�в���ϵͳ����		*/
/*	<02>._bswVdebug		���ռ�¼����(_swVdebug)		*/
/*	<03>._bswTrim		ȥ�ո�				*/
/*	<04>._GetProfileString	�������ļ�			*/
/*	<05>._GetFormatTime	��ʽ��ʱ�䴮			*/
/*	<06>._longtodate	����������ת��Ϊ�ַ�����	*/
/*	<07>._dateTolong	���ַ�����ת��Ϊ��������	*/
/*	<08>._dateAddday	ȡ���ڼ��������������		*/
/*	<09>._erTOshi		������תʮ����			*/
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2015/3/27 15:35:34 XXXXXXX                               	*/
/****************************************************************/
#include "DT_Pub.h"


/**************************������������**************************/
char agDebugfile[128];
int  cgDebug;
//����ʱ�亯��ȡ������
time_t sysTimeSec;

int 	rc=0;		//���ܷ���ֵ(��������ʹ��)
int 	Ret=0;		//���ܷ���ֵ(ʹ��ʱ�����¸�ֵ���Ժ󷵻���ȷֵ)


/**************************************************************
 ** ������      : _SystemExec
 ** ��  ��      : ִ�в���ϵͳ����
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 16:42:47
 ** ����޸�����: 2015/3/6 16:42:49
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : ִ�д���
 ** ����ֵ      : FAIL:ʧ��. SUCC:�ɹ�
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
 ** ������      : _SystemSignal
 ** ��  ��      : �����ź�
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 16:42:47
 ** ����޸�����: 2015/3/6 16:42:49
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : ִ�д���
 ** ����ֵ      : FAIL:ʧ��. SUCC:�ɹ�
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
 ** ������      : _bswVdebug
 ** ��  ��      : ��־��¼����(ָ���ļ���)
 ** ��  ��      :
 ** ��������    : 2012-10-10
 ** ����޸�����: 2012-12-12  ָ�����ļ���ΪDEDebugfile
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : class:   ����:���Լ���
 **               fmt:     ����:��ʽ��
 ** ����ֵ      : ��
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
	/* �����ļ�����Ϊ15λ */
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
	  /* �����ÿյ��ַ� */
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
	  /* �ָ��ַ����ƶ�ָ�� */
	  *(pbuf + MAX_LINE_BUF) = cc;
	  pbuf += MAX_LINE_BUF;
	  flg = 0;
	}
	    else break;
	}
	fclose(fp);
}



/**************************************************************
 ** ������      :  _GetFormatTime
 ** ��  ��      : ��ʽ��ʱ��
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 11:34:27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : ʱ��,����,��ʽ
 ** ����ֵ      : 0���ɹ�  ��0��ʧ��
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
 ** ������      :  _GetProfileString
 ** ��  ��      : �������ļ�ȡ��Ӧ����
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 11:34:27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : �ļ���,[�����ʶ],������,����ֵ
 ** ����ֵ      : 0���ɹ�  ��0��ʧ��
***************************************************************/
int _GetProfileString(char* FileName, char* Section, char* Index, char* GetValue)
{
	FILE *fp;
	int iSectionLen,iIndexLen,iFind;
	char buff[512];
	int i;

	iSectionLen=strlen(Section);
	iIndexLen=strlen(Index);

	/* �������ļ� */
	if ( ( fp = fopen (FileName,"r") ) == NULL )
		return (-1);
		
	/* Ѱ��ƥ������ */
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

	/* Ѱ��ƥ������ */
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
 ** ������      : _bswTrim
 ** ��  ��      : ȥ�ո�
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 11:34:27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : Ҫȥ�ո���ַ���
 ** ����ֵ      : 0���ɹ�  ��0��ʧ��
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
 ** ������      : _longtodate
 ** ��  ��      : ����������ת��Ϊ�ַ�����
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 11:34:27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :      ldate:     ���룺��������
 ** ����ֵ      : �ַ�����
***************************************************************/
void _longTodate(long ldate,char *day)
{
	struct tm lm ;
	memcpy(&lm,localtime(&ldate),sizeof(lm));
	/* modify by zjj 2004.12.04 add  +1  */
	sprintf(day,"%04d%02d%02d",lm.tm_year+1900,lm.tm_mon + 1,lm.tm_mday);
}

/**************************************************************
 ** ������      : _dateTolong
 ** ��  ��      : �ַ����ڽ�ת��Ϊ��������
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 11:34:27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : ���룺����
 ** ����ֵ      : �ַ�����
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
* �� �� ��: _dateAddday()
* ��    ��: ȡ��һ���ڼ����������������
* ��    ��: Hu
* ��������:
* �޸�����:
* ��    ��: ԭ����,����,��������,�µ�����;
* ��    ��:  0���ɹ�  ��0��ʧ��
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
 ** ������      : _erTOshi
 ** ��  ��      : ������תʮ����
 ** ��  ��      : Hu
 ** ��������    : 2015/3/25 10:34:21
 ** ����޸�����: 2015/3/25 10:34:23
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : ����:Ҫת���������ַ���
 **               ����:ת�����ֵ
 ** ����ֵ      : ��
***************************************************************/
int _erTOshi(char *str) 
{ 
	int i,j=0,k=0;
	for(i=strlen(str)-1;i>=0;i--)
		k+=pow(2,j++)*(str[i]-'0'); 
	return k;
} 

/**************************************************************
 ** ������      : _isEnvPasswd
 ** ��  ��      : �ж��û������Ƿ�Ϊ��������
 ** ��  ��      : Hu
 ** ��������    : 2015/3/30 17:54:15
 ** ����޸�����: 2015/3/30 17:54:17
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : ����:Ҫת���ַ���
 **               ����:���ܻ����������ֵ
 ** ����ֵ      : 0���ɹ�  ��0��ʧ��
***************************************************************/
int _isEnvPasswd(char *agEnvPasswd)
{
	char alEnvPasswd[32];
	/*�������ݿ����룺�����1λ=$,�ڶ�λ={,���һλ=}�����ж���Ϊ����������ֱ��ȡ��������ֵ�������¸�ֵ.*/	
	if(agEnvPasswd[0]=='$' && agEnvPasswd[1]=='{' && agEnvPasswd[strlen(agEnvPasswd)-1]=='}')
	{
		_swVdebug(0, "_isEnvPasswd:Ҫ���ܵĻ���������=��%s��",agEnvPasswd);
		INIT(alEnvPasswd);
		memcpy(alEnvPasswd,agEnvPasswd+2,strlen(agEnvPasswd)-3);
		INIT(agEnvPasswd);
		if(getenv(alEnvPasswd)==NULL)
			return FAIL;
		sprintf(agEnvPasswd,"%s",getenv(alEnvPasswd));
		//_swVdebug(0, "_isEnvPasswd:���ܺ������Ϊ=��%s��",agEnvPasswd);
	}
		
	return SUCC;
}
