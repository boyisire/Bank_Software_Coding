#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <math.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <getopt.h>


/***********************公共宏定义***********************/
#define SUCC 0						//成功
#define FAIL -1						//失败
#define DAYEND_ENV "HISDAYENDPATH"				//日终主目录的环境变量名
#define MAX_FILE_SIZE        (6 * 1048576L)		//文件最大数
#define MAX_LINE_BUF 2048				//文件最大行
#define INIT(a) memset((void *)(a), 0x00, sizeof(a))	//初始化变量
#define INITS(a) memset((void *)&(a), 0x00, sizeof(a))	//初始化结构体
#define exists(filename) (access(filename,F_OK))  	//判断文件是否存在，返回值为0即文件存在.

/***********************公共函数声明***********************/
//执行系统Shell命令
int _SystemExec(char *Buf);
//屏蔽信号
void _SystemSignal();
//去空格
int _bswTrim(char *s);			
//日志打印
extern void _bswVdebug(int level, char *file, char *func, int lineno, char *fmt, ...);
//获取相应格式的时间
int _GetFormatTime( char *FormatTime, int iLen, const char *Format );
//读取配置文件
int _GetProfileString(char* FileName, char* Section, char* Index, char* GetValue);
//将整型日期转换为字符日期
void _longTodate(long ldate,char *day);
//字符日期将转换为整型日期
void _dateTolong(char *aTime, long *lTime);
//取得一日期减上天数后的日期数
int  _dateAddday(char *oldday, int days, char *ndate);
//二进制转10进制
int _erTOshi(char *str);
//判断用户密码是否为环境变量
int _isEnvPasswd(char *agEnvPasswd);

/*库函数定义*/
#ifndef _swVdebug
#define _swVdebug(_level_, ...)  _bswVdebug(_level_ , (char *)__FILE__, (char *)__func__, __LINE__, __VA_ARGS__)
#endif


/*引用全局函数*/
extern int rc;			//接受返回值，对返错无影响
extern int Ret;			//接受返回值，对返错有影响
extern char agDebugfile[128];	//全局日志文件名
extern int  cgDebug;		//全局日志级别

/*声明时间变量*/
time_t t;
