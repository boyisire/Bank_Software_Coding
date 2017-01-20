#include "switch.h"
#include "swConstant.h"
#include  "swDbstruct.h"
#include  "swShm.h"
#include <pthread.h>
#include <sys/syscall.h>
#define gettid() syscall(__NR_gettid)
void sw_thread_log_destructor(void *info);

static pthread_key_t  log_info_key;

extern char procName[20];
char agSys_log_name[64];
typedef struct sw_log_info_t
{
	short debug_level;
	char log_file[64];
	long pid;
} sw_log_info_t;

/**************************************************************
 ** 函数名      : _swDebug_r
 ** 功  能      : 记跟踪日志:打印指定信息(线程安全)
 ** 作  者      : Deng Yuwei
 ** 建立日期    : 2015/04/03
 ** 最后修改日期:
 ** 调用其它函数: GetTime
 ** 全局变量    : 
 ** 参数含义    : frm,va_alist :用法格式同printf
 ** 返回值      : -1:操作日志文件出错. 0:成功
***************************************************************/
#ifdef OS_LINUX
int _swVdebug_r(short iDebug, char *filename,
		int line, char *frm, ...)
#endif

#ifdef OS_AIX
int _swVdebug_r(iDebug, filename, line, frm, va_list);
short iDebug  char *filename int line char *frm va_dcl
#endif

#ifdef OS_HPUX
int _swVdebug_r(short iDebug,  char *filename,
		int line, char *frm, ...)
#endif

#ifdef OS_SCO
int _swVdebug_r(short iDebug, char *filename,
		int line, char *frm, ...)
#endif
#ifdef OS_SOLARIS
int _swVdebug_r(short iDebug, char *filename,
		int line, char *frm, ...)
#endif
{
	static struct swt_sys_config sSwt_sys_config;
	static short ilTrace_flag = 0;
	static const long llLogfilesize = 8388608L*16;
	FILE *fp;
	char buf[iMSGMAXLEN], fname1[100], buf1[iMSGMAXLEN], alCmdbuf[200];
	va_list ap;
	struct tm *sttm;
	long llDatetime;
	short ilRn;
	struct stat file_stat;
	int rc;
	struct timeval curtime;
	char buf3[80];
	sw_log_info_t *log_info;

	log_info = (sw_log_info_t *)pthread_getspecific( log_info_key );
	if ( log_info->debug_level < iDebug) return (0);

	memset(buf, 0x00, sizeof(buf));
	memset(buf1, 0x00, sizeof(buf1));
	memset(fname1, 0x00, sizeof(fname1));

	if (*frm) {
#ifdef OS_LINUX
		va_start(ap, frm);
#endif
#ifdef OS_AIX
		va_start(ap);
#endif
#ifdef OS_HPUX
		va_start(ap, frm);
#endif
#ifdef OS_SCO
		va_start(ap, frm);
#endif
#ifdef OS_SOLARIS
		va_start(ap, frm);
#endif
		vsprintf(buf, frm, ap);
		va_end(ap);
	}

	if (iDebug == 0)
		sprintf(fname1, "%s/%s", getenv("SWITCH_DEBUG_PATH"),
			agSys_log_name);
	else
    {
		sprintf(fname1, "%s/%s", getenv("SWITCH_DEBUG_PATH"),log_info->log_file);
    }

	rc = stat(fname1, &file_stat);
	if (file_stat.st_size > llLogfilesize) {
		time(&llDatetime);
		sttm = localtime(&llDatetime);

		sprintf(alCmdbuf, "%s.%02d%02d%02d%02d%02d",
			fname1,
			sttm->tm_mon + 1,
			sttm->tm_mday,
			sttm->tm_hour, sttm->tm_min, sttm->tm_sec);
		if ((access(fname1, F_OK)) != -1)
			rename(fname1, alCmdbuf);
	}

	if ((fp = fopen(fname1, "a+")) == NULL)
		return (-1);
	_swGetTime(buf1);
	memset(buf3, 0x00, sizeof(buf3));
	gettimeofday(&curtime, NULL);
	snprintf(buf3, sizeof(buf3), ".%06ld|%s|%ld|%s|%05d", curtime.tv_usec,
		 procName, log_info->pid, filename, line);
	strcat(buf1, buf3);
	strcat(buf1, ": ");
	strcat(buf1, buf);
	fprintf(fp, "%s\n", buf1 + 5);
	fclose(fp);
	if (iDebug == 0) {
		if (ilTrace_flag == 0) {
			ilTrace_flag = 1;
			memset((char *)&sSwt_sys_config, 0x00,
			       sizeof(struct swt_sys_config));
			ilRn = swShmcfgload(&sSwt_sys_config);
			if (ilRn) {
				return (FAIL);
			}
			signal(SIGTTOU, SIG_IGN);
			if (freopen(sSwt_sys_config.trace_term, "a+", stderr) ==
			    NULL) {
			}
			fprintf(stderr, "%s\n", buf1 + 5);
		}
	}
	return (0);
}

int sw_thread_log_init()
{
	pthread_key_create( &log_info_key , sw_thread_log_destructor);
	return 0;
}

void sw_thread_log_alloc()
{
    sw_log_info_t *info =  (sw_log_info_t*) malloc( sizeof(sw_log_info_t)) ;
    pthread_setspecific(log_info_key, info);
}

int sw_thread_log_set_attr(char *log_file, int debug_level)
{
    sw_log_info_t *info;
    info = (sw_log_info_t *)pthread_getspecific( log_info_key );
	info->debug_level =  debug_level;
	strcpy( info->log_file , log_file);
	info->pid = gettid();
	return 0;
}

void sw_thread_log_destructor(void *info)
{
	free(info);
}

/******************************************************
 *
 *       void swLogInit(char *pprocName, char *sys_log_name)
 *       初始化日志相关的变量
 *       author: dyw
 *       crate date: 2015-03-27
 *  
 ******************************************************/
void  swLogInit(char *pprocName, char *sys_log_name)
{
  /* 设置调试程序名称 */

  memset( procName, 0x00, sizeof(procName) );
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  memset(agSys_log_name, 0x00, sizeof(agSys_log_name));
  
  strncpy(procName,pprocName,sizeof(procName)-1);
  if ( sys_log_name == NULL ) 
  {
  		strcpy(agSys_log_name, SW_SYS_LOGFILE_NAME);
  }
  else
  {
 	 strcpy(agSys_log_name, sys_log_name);
  }
  strncpy( agDebugfile,agSys_log_name, sizeof( agDebugfile) - 1 );
  g_current_pid = getpid();
//  sw_thread_log_init();
}


/**************************************************************
 ** 函数名      : swDebug
 ** 功  能      : 记跟踪日志:打印指定信息
 ** 作  者      : lv lixiao
 ** 建立日期    : 1999/11/19
 ** 最后修改日期:
 ** 调用其它函数: GetTime
 ** 全局变量    : agDebugfile
 ** 参数含义    : frm,va_alist :用法格式同printf
 ** 返回值      : -1:操作日志文件出错. 0:成功
***************************************************************/
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* int swVdebug(short iDebug,char *frm, ...) */
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
int _swVdebug(short iDebug,char *filename,int line,char *frm, ...)
#endif
#ifdef OS_AIX
/* del by gengling at 2015.03.31 three lines PSBC_V1.0 */
/* int swVdebug(frm,va_list)
char *frm
va_dcl */
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
int _swVdebug(iDebug,filename,line,frm,va_list);
short iDebug
char *filename
int line
char *frm
va_dcl
#endif
#ifdef OS_HPUX
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* int swVdebug(short iDebug,char *frm, ...) */
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
int _swVdebug(short iDebug,char *filename,int line,char *frm, ...)
#endif
#ifdef OS_SCO
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* int swVdebug(short iDebug,char *frm, ...) */
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
int _swVdebug(short iDebug,char *filename,int line,char *frm, ...)
#endif
#ifdef OS_SOLARIS
/* del by gengling at 2015.03.31 one line PSBC_V1.0 */
/* int swVdebug(short iDebug,char *frm, ...) */
/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
int _swVdebug(short iDebug,char *filename,int line,char *frm, ...)
#endif
{
  static struct swt_sys_config sSwt_sys_config;
  static short ilTrace_flag = 0;
  static long llLogfilesize = 0;
  FILE *fp;
  /*modify by pc 2008-3-19 10:36*/
  char buf[iMSGMAXLEN],fname1[100],buf1[iMSGMAXLEN],alCmdbuf[200];
  /*modify by pc 2008-3-19 10:36*/
  /*char buf[1024],fname1[100],buf1[1024],alCmdbuf[200];*/
  va_list  ap;
  struct tm  *sttm;
  int ilRc;
  long llDatetime;
  short ilRn;
  struct stat sgbuf;
  int rc;
  
  if ( cgDebug < iDebug ) return(0);

  memset(buf,0x00,sizeof(buf));
  memset(buf1,0x00,sizeof(buf1));
  memset(fname1,0x00,sizeof(fname1));
  if (*frm) 
  {
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
    va_start( ap, frm );
#endif
#ifdef OS_AIX
    va_start(ap);
#endif
#ifdef OS_HPUX
    va_start( ap, frm );
#endif
#ifdef OS_SCO
    va_start( ap, frm );
#endif
#ifdef OS_SOLARIS
    va_start( ap, frm );
#endif
    vsprintf(buf, frm, ap);
    va_end(ap);
  }

  if (!llLogfilesize)
  {
    ilRc = 0; 
    if ((ilRc == FAIL) || (ilRc == 0)) llLogfilesize = 8388608L;
    else llLogfilesize = ilRc * 1048576L;
  }

  /*0级日志统一输出到系统日志文件中add by cjh 20150317, PSBC_V1.0*/
  if( iDebug == 0 )
  	sprintf(fname1,"%s/%s",getenv("SWITCH_DEBUG_PATH"),"swSyslog.debug");
  else
  	sprintf(fname1,"%s/%s",getenv("SWITCH_DEBUG_PATH"),agDebugfile);
	
  rc=stat(fname1, &sgbuf);
  if (sgbuf.st_size > llLogfilesize)
  {
    time(&llDatetime);
    sttm = localtime( &llDatetime );

    sprintf(alCmdbuf,"%s.%02d%02d%02d%02d%02d",
      fname1,
      sttm->tm_mon+1,
      sttm->tm_mday,
      sttm->tm_hour,
      sttm->tm_min,
      sttm->tm_sec );
    if ((access(fname1,F_OK)) != -1)
      rename(fname1,alCmdbuf);
  }

  if (( fp = fopen(fname1,"a+")) == NULL)  return(-1);
  _swGetTime(buf1);
/*add by pc*/
        struct timeval curtime;
//char buf3[36];

/*** modify by wanghao 20150330 PSBC_V1.0***/
char buf3[80];
memset(buf3,0x00, sizeof(buf3));
/****end modify****/
gettimeofday(&curtime,NULL);
/*== begin ==add by cjh, 2015.03.26, PSBC_V1.0*/
/* mod by gengling at 2015.03.31 one line PSBC_V1.0 */
/* snprintf(buf3,sizeof(buf3),".%06d|%16s|%08s|%05d",curtime.tv_usec, procName, __FILE__, __LINE__); */
snprintf(buf3,sizeof(buf3),".%06ld|%s|%ld|%s|%05d",curtime.tv_usec, procName, gettid(), filename, line);
/*== begin ==add by cjh, 2015.03.26, PSBC_V1.0*/
strcat(buf1,buf3);
/*add by pc*/
  strcat(buf1,": ");
  strcat(buf1,buf);
  fprintf(fp,"%s\n",buf1 + 5);
  fclose(fp);
  if ( iDebug == 0 )
  {
    if ( ilTrace_flag == 0 )
    {
      ilTrace_flag = 1;
      memset((char *)&sSwt_sys_config,0x00,sizeof(struct swt_sys_config));
      ilRn = swShmcfgload(&sSwt_sys_config);
      if (ilRn)
      {
        swDebug("S6291: [错误/函数调用] swShmcfgload()函数,返回码=%d",ilRn);
        return(FAIL);
      }
      signal(SIGTTOU,SIG_IGN);
      if (freopen(sSwt_sys_config.trace_term,"a+",stderr)==NULL)
      {
        swDebug("S6292: [错误/其它] %s 未连接或权限不够,请修改该终端的权限.",
          sSwt_sys_config.trace_term);
      }
    }
    fprintf(stderr,"%s\n",buf1 + 5);
  }

  return(0);
}



#ifdef OS_LINUX
int swDebug(char *frm, ...)
#endif
#ifdef OS_AIX
int swDbug(frm,va_list)
char *frm
va_dcl
#endif
#ifdef OS_HPUX
int swDebug(char *frm, ...)
#endif
#ifdef OS_SCO
int swDebug(char *frm, ...)
#endif
#ifdef OS_SOLARIS
int swDebug(char *frm, ...)
#endif
{
  static long llLogfilesize = 0;
  FILE *fp;
  char buf[1024],fname1[100],buf1[1024];
  va_list  ap;
  struct tm  *sttm;
  int ilRc;
  long llDatetime;
  struct stat sgbuf;
  int rc;
  char alFilename2[100]; 

  memset(buf,0x00,sizeof(buf));
  memset(buf1,0x00,sizeof(buf1));
  memset(fname1,0x00,sizeof(fname1));
  if (*frm) 
  {
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
    va_start( ap, frm );
#endif
#ifdef OS_AIX
    va_start(ap);
#endif
#ifdef OS_HPUX
    va_start( ap, frm );
#endif
#ifdef OS_SCO
    va_start( ap, frm );
#endif
#ifdef OS_SOLARIS
    va_start( ap, frm );
#endif
    vsprintf(buf, frm, ap);
    va_end(ap);
  }

  if (!llLogfilesize)
  {
    ilRc = 0; 
    if ((ilRc == FAIL) || (ilRc == 0)) llLogfilesize = 8388608L;
    else llLogfilesize = ilRc * 1048576L;
  }

  sprintf(fname1,"%s/%s",getenv("SWITCH_DEBUG_PATH"),agDebugfile);
  rc=stat(fname1, &sgbuf);
  if (sgbuf.st_size > llLogfilesize)
  {
    time(&llDatetime);
    sttm = localtime( &llDatetime );
    sprintf(alFilename2,"%s.%02d%02d%02d%02d%02d",
      fname1,
      sttm->tm_mon+1,
      sttm->tm_mday,
      sttm->tm_hour,
      sttm->tm_min,
      sttm->tm_sec );
    if ((access(fname1,F_OK)) != -1)
      rename(fname1,alFilename2);  /* 先不判断执行结果 */      
  }

  if (( fp = fopen(fname1,"a+")) == NULL)  return(-1);
  _swGetTime(buf1);
  strcat(buf1,": ");
  strcat(buf1,buf);
  fprintf(fp,"%s\n",buf1 + 5);
  fclose(fp);
  return(0);
}
