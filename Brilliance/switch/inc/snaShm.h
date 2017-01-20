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


/* ��̬�����ڴ�Ŀ¼�γ�Ա�ṹ */
struct shmidx_item_d
{
  long lOffset;		/* ƫ���� */
  short iSlink;		/* ������ */
  short iElink;		/* β���� */	
  short iFlink;		/* ������ */
};

/* ��̬�����ڴ����ݶγ�Ա�ṹ */
/* ��̬�����ݶγ�Ա�ṹΪ������Ϣ+���ݿ��ṹ */

/* ������Ϣ�ṹ */
struct linkinfo
{
  char cTag;		/* ���ñ�ʶ */
  short iLlink;		/* ������ */
  short iRlink;		/* ������ */
};

/* ��̬�����ڴ�Ŀ¼�νṹ */
struct shmidx_d
{
  struct shmidx_item_d sIdxd_sna;
  int  iSemid;
  char aMemo[101];
};

struct swt_tran_sts
{
   short    group_lu;          /*LU���*/
   char     lu_name[20 + 1];   /*LU ����*/
   short    group_status;      /*0��δ����  1������*/
   long     transing_num;      /*���ڽ��еĽ���*/
   long     finish_num;        /*��ɵĽ���*/
   long     success_num;       /*�ɹ��Ľ���*/
   long     fail_num;          /*ʧ�ܵĽ���*/
};

/* ��̬�����ڴ����ݶ�ָ��ṹ */
struct shmbuf_d
{
  struct shm_tran_sts *psShm_tran_sts;
};

/* ������ˮ�����ݶγ�Ա�ṹ */
struct shm_tran_sts
{
  struct linkinfo sLinkinfo;
  struct swt_tran_sts sSwt_tran_sts;
};


/* ȫ�ֱ������� */

struct shmidx_d *psgShmidx_d;	/* ��̬�����ڴ�Ŀ¼��ָ�� */
struct shmbuf_d sgShmbuf_d;	/* ��̬�����ڴ����ݶ�ָ�� */
char agDebugfile[64];                /* �����ļ��� */
char cgDebug;                        /* �ǵ�����־��־ */


int swShmcheck();
int swShmP(int);
int swShmV(int);

#endif

