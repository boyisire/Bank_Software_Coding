#ifndef _SWSHM_H
#define _SWSHM_H

#include <sys/sem.h>
#include <sys/shm.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>

#define iSEMNUM                 6
#define iSEMNUM_TRANLOG		3
#define SUCCESS                 0
#define FAIL                    -1 


/* 动态表共享内存目录段成员结构 */
struct shmidx_item_d
{
  long lOffset;		/* 偏移量 */
  short iSlink;		/* 首链接 */
  short iElink;		/* 尾链接 */	
  short iFlink;		/* 空链接 */
};

/* 动态表共享内存内容段成员结构 */
/* 动态表内容段成员结构为链接信息+数据库表结构 */

/* 链接信息结构 */
struct linkinfo
{
  char cTag;		/* 启用标识 */
  short iLlink;		/* 左链接 */
  short iRlink;		/* 右链接 */
};

/* 动态表共享内存目录段结构 */
struct shmidx_d
{
  struct shmidx_item_d sIdxd_sna;
  int  iSemid;
  char aMemo[101];
};

struct swt_tran_sts
{
   short    group_lu;          /*LU组号*/
   char     lu_name[20 + 1];   /*LU 名称*/
   short    group_status;      /*0：未启动  1：启动*/
   long     transing_num;      /*正在进行的交易*/
   long     finish_num;        /*完成的交易*/
   long     success_num;       /*成功的交易*/
   long     fail_num;          /*失败的交易*/
};

/* 动态表共享内存内容段指针结构 */
struct shmbuf_d
{
  struct shm_tran_sts *psShm_tran_sts;
};

/* 交易流水表内容段成员结构 */
struct shm_tran_sts
{
  struct linkinfo sLinkinfo;
  struct swt_tran_sts sSwt_tran_sts;
};


/* 全局变量定义 */

struct shmidx_d *psgShmidx_d;	/* 动态表共享内存目录段指针 */
struct shmbuf_d sgShmbuf_d;	/* 动态表共享内存内容段指针 */
char agDebugfile[64];                /* 调试文件名 */
char cgDebug;                        /* 记调试日志标志 */


int swShmcheck();
int swShmP(int);
int swShmV(int);

#endif

