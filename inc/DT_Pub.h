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


/***********************�����궨��***********************/
#define SUCC 0						//�ɹ�
#define FAIL -1						//ʧ��
#define DAYEND_ENV "HISDAYENDPATH"				//������Ŀ¼�Ļ���������
#define MAX_FILE_SIZE        (6 * 1048576L)		//�ļ������
#define MAX_LINE_BUF 2048				//�ļ������
#define INIT(a) memset((void *)(a), 0x00, sizeof(a))	//��ʼ������
#define INITS(a) memset((void *)&(a), 0x00, sizeof(a))	//��ʼ���ṹ��
#define exists(filename) (access(filename,F_OK))  	//�ж��ļ��Ƿ���ڣ�����ֵΪ0���ļ�����.

/***********************������������***********************/
//ִ��ϵͳShell����
int _SystemExec(char *Buf);
//�����ź�
void _SystemSignal();
//ȥ�ո�
int _bswTrim(char *s);			
//��־��ӡ
extern void _bswVdebug(int level, char *file, char *func, int lineno, char *fmt, ...);
//��ȡ��Ӧ��ʽ��ʱ��
int _GetFormatTime( char *FormatTime, int iLen, const char *Format );
//��ȡ�����ļ�
int _GetProfileString(char* FileName, char* Section, char* Index, char* GetValue);
//����������ת��Ϊ�ַ�����
void _longTodate(long ldate,char *day);
//�ַ����ڽ�ת��Ϊ��������
void _dateTolong(char *aTime, long *lTime);
//ȡ��һ���ڼ����������������
int  _dateAddday(char *oldday, int days, char *ndate);
//������ת10����
int _erTOshi(char *str);
//�ж��û������Ƿ�Ϊ��������
int _isEnvPasswd(char *agEnvPasswd);

/*�⺯������*/
#ifndef _swVdebug
#define _swVdebug(_level_, ...)  _bswVdebug(_level_ , (char *)__FILE__, (char *)__func__, __LINE__, __VA_ARGS__)
#endif


/*����ȫ�ֺ���*/
extern int rc;			//���ܷ���ֵ���Է�����Ӱ��
extern int Ret;			//���ܷ���ֵ���Է�����Ӱ��
extern char agDebugfile[128];	//ȫ����־�ļ���
extern int  cgDebug;		//ȫ����־����

/*����ʱ�����*/
time_t t;
