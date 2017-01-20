#ifndef _SWSHM_H
#define _SWSHM_H

#include "sw_lock.h"
#define iSEMNUM			13	/* �ź������� */		
#define iSEMNUM_TRANID		0
#define iSEMNUM_SAFID		9
#define iSEMNUM_PORT		10
/*#define iSEMNUM_TRANLOG	3 */	  /*delete by dyw, repalce by c_tranid_sem array*/
#define iSEMNUM_PROCLOG		11
#define iSEMNUM_REVSAF      12

#define SHM_TRANLOG_BUCKET_NUM  8
#define SHM_PROCLOG_BUCKET_NUM  8

#define SHMFULL			9	/* �����ڴ����� */


extern const char sw_tranid_sem_array[];

typedef struct sw_slock_array_t
{
	slock_t  lock[iSEMNUM];
}sw_slock_array_t;

sw_slock_array_t *pg_lock_array;


/* ��̬�����ڴ�Ŀ¼�γ�Ա�ṹ */
struct shmidx_item_s
{
  long lOffset;		/* ƫ���� */
  int iCount;		/* ��¼�� */
  char  a1[6];
};

/* ��̬�����ڴ����ݶγ�Ա�ṹ */
/* ��ʽת����·�ɽű��������������ݶγ�Ա�ṹΪ������Ϣ+���ݿ��ṹ */
/* ������̬�����ݶγ�Ա�ṹ��Ϊ���ݿ��ṹ */

/* ��ʽת���������ݶγ�Ա�ṹ */
struct shm_sys_fmt_m
{
  long lOffset;		/* ƫ������ָ������ж�Ӧ��¼ */
  struct swt_sys_fmt_m sSwt_sys_fmt_m;
};

/* ��ʽת��������ݶγ�Ա�ṹ */
struct shm_sys_fmt_g
{
  long lOffset;		/* ƫ������ָ����ϸ���и�����׼�¼ */
  int iCount;		/* �����ڸ���ļ�¼�� */ /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  char a1[6];
  struct swt_sys_fmt_grp sSwt_sys_fmt_g;
};

/* ·�ɽű��������ݶγ�Ա�ṹ */
struct shm_sys_route_m
{
  long lOffset;		/* ƫ������ָ������ж�Ӧ��¼ */
  struct swt_sys_route_m sSwt_sys_route_m;
};

/* ·�ɽű�������ݶγ�Ա�ṹ */
struct shm_sys_route_g
{
  long lOffset;		/* ƫ������ָ����ϸ���и�����׼�¼ */
  int iCount;		/* �����ڸ���ļ�¼�� */  /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  char  a1[6];
  struct swt_sys_route_grp sSwt_sys_route_g;
};

/* ��̬�����ڴ�Ŀ¼�νṹ */
struct shmidx_s
{
  struct shmidx_item_s sIdx_sys_imf;
  struct shmidx_item_s sIdx_sys_queue;
  struct shmidx_item_s sIdx_sys_matchport;	/* added by fzj at 2002.02.28 */
  struct shmidx_item_s sIdx_sys_task;
  struct shmidx_item_s sIdx_sys_8583;
  struct shmidx_item_s sIdx_sys_8583E;
  struct shmidx_item_s sIdx_sys_code;
  struct shmidx_item_s sIdx_sys_config;
  struct shmidx_item_s sIdx_sys_other;
  struct shmidx_item_s sIdx_sys_fmt_m;
  struct shmidx_item_s sIdx_sys_fmt_g;
  struct shmidx_item_s sIdx_sys_fmt_d;
  struct shmidx_item_s sIdx_sys_route_m;
  struct shmidx_item_s sIdx_sys_route_g;
  struct shmidx_item_s sIdx_sys_route_d;
  struct shmidx_item_s sIdx_sys_tran; /* add by gengling at 2015.03.18 one line PSBC_V1.0 */
  char aMemo[101];
  char a1[3];
};  

/* ��̬�����ڴ�Ŀ¼�γ�Ա�ṹ */
struct shmidx_item_d
{
  long lOffset;		/* ƫ���� */
  unsigned int  iSlink;		/* ������ */
  unsigned int  iElink;		/* β���� */	
  unsigned int  iFlink;		/* ������ */ /*modified by dyw, convert from short to unsigned int, 2014-12-30*/
  char a1[2];
  int max_used_log;
  int used_log;
};

/* ��̬�����ڴ����ݶγ�Ա�ṹ */
/* ��̬�����ݶγ�Ա�ṹΪ������Ϣ+���ݿ��ṹ */

/* ������Ϣ�ṹ */
struct linkinfo
{
  char cTag;		/* ���ñ�ʶ */
  char a1[3];
  unsigned int iLlink;		/* ������ */
  unsigned int iRlink;		/* ������ */ /*modified by dyw, convert from short to unsigned to int, 2014-12-30*/
};

/* ������ˮ�����ݶγ�Ա�ṹ */
struct shm_tran_log
{
  struct linkinfo sLinkinfo;
  struct swt_tran_log sSwt_tran_log;
};

/* ������ˮ�����ݶγ�Ա�ṹ */
struct shm_proc_log
{
  struct linkinfo sLinkinfo;
  struct swt_proc_log sSwt_proc_log;
};

/* saf��ˮ�����ݶγ�Ա�ṹ */
struct shm_rev_saf
{
  struct linkinfo sLinkinfo;
  struct swt_rev_saf sSwt_rev_saf;
};

/* ��̬�����ڴ�Ŀ¼�νṹ */
struct shmidx_d
{
  struct shmidx_item_d sIdx_tran_log[SHM_TRANLOG_BUCKET_NUM];
  struct shmidx_item_d sIdx_proc_log;
  struct shmidx_item_d sIdx_rev_saf;
  int  iShmid;
  int  iSemid;
  long lTranid;
  long lSafid;
  long iRecinshm[3];
  long iRecinfile[3];
  char aMemo[101];
  char a1[3];
};

/* ��̬�����ڴ����ݶ�ָ��ṹ */
struct shmbuf_s
{
  struct swt_sys_imf *psSwt_sys_imf;
  struct swt_sys_queue *psSwt_sys_queue;
  struct swt_sys_matchport *psSwt_sys_matchport;	/* added by fzj at 2002.02.28 */
  struct swt_sys_task *psSwt_sys_task;
  struct swt_sys_8583 *psSwt_sys_8583;
  struct swt_sys_8583E *psSwt_sys_8583E;
  struct swt_sys_code *psSwt_sys_code;
  struct swt_sys_config *psSwt_sys_config;
  struct swt_sys_other *psSwt_sys_other;
  struct shm_sys_fmt_m *psShm_sys_fmt_m;
  struct shm_sys_fmt_g *psShm_sys_fmt_g;
  struct swt_sys_fmt_d *psSwt_sys_fmt_d;
  struct shm_sys_route_m *psShm_sys_route_m;
  struct shm_sys_route_g *psShm_sys_route_g;
  struct swt_sys_route_d *psSwt_sys_route_d;
  struct swt_sys_tran *psSwt_sys_tran; /* add by gengling at 2015.03.18 one line PSBC_V1.0 */
#ifdef HASH
  void *psSwt_sys_tran_hash; /* 2015.05.04 gl0 */
#endif
};

/* ��̬�����ڴ����ݶ�ָ��ṹ */
struct shmbuf_d
{
  struct shm_tran_log *psShm_tran_log[SHM_TRANLOG_BUCKET_NUM];
  struct shm_proc_log *psShm_proc_log;
  struct shm_rev_saf *psShm_rev_saf;
};

/* ���ض�̬�ڴ�ָ��Ŀ¼ */
struct lcmidx
{
  short iCount_swt_sys_queue;
  short iCount_swt_sys_imf;
  short iCount_swt_sys_matchport;	/* added by fzj at 2002.02.28 */
  short iCount_swt_sys_task;
  short iCount_swt_sys_8583;
  short iCount_swt_sys_config;
  short iCount_swt_sys_8583E;
  short iCount_swt_sys_fmt_m;
  short iCount_swt_sys_other;
  short iCount_swt_sys_fmt_g;
  short iCount_swt_sys_route_m;
  short iCount_swt_sys_route_d;
  int iCount_swt_sys_fmt_d;
  short iCount_swt_sys_code;
  short iCount_swt_sys_route_g;
  short iCount_swt_sys_tran; /* add by gengling at 2015.03.18 one line PSBC_V1.0 */
/*  char  a1[2];*/
  struct swt_sys_imf *psSwt_sys_imf;
  struct swt_sys_queue *psSwt_sys_queue;
  struct swt_sys_matchport *psSwt_sys_matchport;	/* added by fzj at 2002.02.28 */
  struct swt_sys_task *psSwt_sys_task;
  struct swt_sys_8583 *psSwt_sys_8583;
  struct swt_sys_8583E *psSwt_sys_8583E;
  struct swt_sys_code *psSwt_sys_code;
  struct swt_sys_config *psSwt_sys_config;
  struct swt_sys_other *psSwt_sys_other;
  struct shm_sys_fmt_m *psShm_sys_fmt_m;
  struct shm_sys_fmt_g *psShm_sys_fmt_g;
  struct swt_sys_fmt_d *psSwt_sys_fmt_d;
  struct shm_sys_route_m *psShm_sys_route_m;
  struct shm_sys_route_g *psShm_sys_route_g;
  struct swt_sys_route_d *psSwt_sys_route_d;
  struct swt_sys_tran *psSwt_sys_tran; /* add by gengling at 2015.03.18 one line PSBC_V1.0 */
#ifdef HASH
  void *psSwt_sys_tran_hash; /* 2015.05.06 gl0 */
#endif
};

/* ȫ�ֱ������� */
struct shmidx_s *psgShmidx_s;	/* ��̬�����ڴ�Ŀ¼��ָ�� */
struct shmidx_d *psgShmidx_d;	/* ��̬�����ڴ�Ŀ¼��ָ�� */
struct shmbuf_s sgShmbuf_s;	/* ��̬�����ڴ����ݶ�ָ�� */
struct shmbuf_d sgShmbuf_d;	/* ��̬�����ڴ����ݶ�ָ�� */
struct lcmidx sgLcmidx;		/* ���ض�̬�ڴ�ָ��Ŀ¼ */ 

short swShmcfgload(struct swt_sys_config *);
short swInitcfgload(struct swt_sys_config *);
short swChgvalue(char *);
int Trim(char *);
int swShmcheck();
int swShmP(int);
int swShmV(int);
short swShmselect_swt_sys_imf_all(struct swt_sys_imf *,short *); 
int swShmupdate_swt_sys_queue(short,struct swt_sys_queue); 
int swShmselect_swt_sys_queue(short, struct swt_sys_queue *); 
int swShmselect_swt_sys_queue_all(struct swt_sys_queue *,short*);
short swShmupdate_swt_sys_task(char *,struct swt_sys_task); 
int swShmselect_swt_sys_task(char *,struct swt_sys_task *);
short swShmselect_swt_sys_task_pid(int, struct swt_sys_task *); 
/* modify by nh 20020807
short swShmselect_swt_sys_task_mrec_use_t(char *,struct swt_sys_task*,short *); 
short swShmselect_swt_sys_task_all_p(struct swt_sys_task *,short*); 
short swShmselect_swt_sys_task_all_q(struct swt_sys_task *,short *); */
short swShmselect_swt_sys_task_mrec_use_t(char *,struct swt_sys_task**,short *); 
short swShmselect_swt_sys_task_all_p(struct swt_sys_task **,short*); 
short swShmselect_swt_sys_task_all_q(struct swt_sys_task **,short *); 

/* add by gxz 2001.09.11 */
int swShmselect_swt_sys_task_qid(short,short, struct swt_sys_task *); 
/* add by gxz 2001.09.11 */
int swShmselect_swt_sys_8583(short,short,struct swt_sys_8583 *);
int swShmselect_swt_sys_8583E(short,short,struct swt_sys_8583E *);

int swShmselect_swt_sys_code(long, struct swt_sys_code *);
int swShmselect_swt_sys_config(struct swt_sys_config *);
int swShmselect_swt_sys_other(short,struct swt_sys_other *);
int swShmselect_swt_sys_fmt_m(short,char *,short,char*,
  struct swt_sys_fmt_grp *p,long*);
/*modify by zcd 20141222
short swShmselect_swt_sys_fmt_grp(int, struct swt_sys_fmt_grp *,long *);*/
short swShmselect_swt_sys_fmt_grp(long, struct swt_sys_fmt_grp *,long *); 
short swShmselect_swt_sys_fmt_grp_offset(long,struct swt_sys_fmt_grp*,
  int*,long*);   /*modified by baiqj20150505 PSBC_V1.0 short -> int */
short swShmselect_swt_sys_fmt_d_offset(long,struct swt_sys_fmt_d *); 
int swShmselect_swt_sys_route_m(int,char *,struct swt_sys_route_m *);
int swShmselect_swt_sys_route_d(int,struct swt_sys_route_d *,short*);
short swShmselect_route_d_q_tran(short, char *,struct swt_sys_route_d*); 
int swShminsert_swt_tran_log(struct swt_tran_log,char *cLoc_flag);
int swShmdelete_swt_tran_log(long,char);
int swShmupdate_swt_tran_log(long,struct swt_tran_log,char);
int swShmupdate_setstat_tranlog(long,short,char);
int swShmselect_swt_tran_log(long,struct swt_tran_log *,char);
int swShmselect_swt_tran_log_all(struct swt_tran_log *,short*); 
int swShmselect_overtime_min_tranlog(long, 
  struct swt_tran_log *,char *);
short swShmselect_max_tran_begin(long *);
short swShmselect_max_tran_id(long ,long *);
short swShmselect_swt_tran_log_clear(long, struct swt_tran_log *,char *);  /* add by dgm 2001.09.13 */
short swShmsettranid(long );
short swShmgettranid(long *);
int swShmfilefind_tran_log(char * );
int swShminsert_swt_proc_log(struct swt_proc_log,char);
int swShmdelete_swt_proc_log(long ,short,char );
int swShmdelete_swt_proc_log_mrec(long,char);
short swShmdelete_proclog_tran_q(long ,short,char );
int swShmupdate_swt_proc_log(long ,short ,
  struct swt_proc_log,char);
int swShmupdate_setstat_proclog(long ,short,char);
int swShmselect_swt_proc_log(long ,short ,
  struct swt_proc_log *,char);
int swShmselect_swt_proc_log_mrec(long , struct swt_proc_log *
  , short *);
short swShmselect_swt_proc_log_clear(long, struct swt_proc_log *,char *);  /* add by dgm 2001.09.13 */
int swShmselect_max_proclog_tran(long ,
  struct swt_proc_log *); 
int swShmfilefind_proc_log(char *);

/* modify by xujun 2002.09.12 begin */
int swDbinsert_swt_rev_saf(struct swt_rev_saf );
int swDbdelete_swt_rev_saf(long );
int swDbupdate_swt_rev_saf(long ,struct swt_rev_saf );
int swDbselect_swt_rev_saf(long ,struct swt_rev_saf *);
int swDbselect_swt_rev_saf_all(struct swt_rev_saf *, 
  short *);
int swDbselect_overtime_min_revsaf(long , 
  struct swt_rev_saf *);
short swDbselect_max_saf_begin(long *);
short swDbselect_max_saf_id(long ,long *);
int swDbselect_fail_rev_saf(long * ,short *);
int swDbupdate_fail_rev_saf(long );
/* modify by xujun 2002.09.12 end */
int swDbselect_qid_revkey_tranlog(short iQid, char *aRevkey,struct swt_tran_log *psSwt_tran_log);
short swShmsetsafid(long );
short swShmgetsafid(long *);
short swCheckportid(short );
short swPortoper(int ,int );


short swShmopenfile(int, FILE **);
short swShmclosefile(int, FILE *);
short swShmgetfile(FILE *, long, unsigned int, char *);
short swShmputfile(FILE *, long, unsigned int, char *);
short swShmf2shm_swt_tran_log(short);
short swShmfinsert_swt_tran_log(struct swt_tran_log);
short swShmfdelete_swt_tran_log(long );
short swShmfupdate_swt_tran_log(long , struct swt_tran_log);
short swShmfupdate_setstat_tranlog(long , short);
short swShmfselect_swt_tran_log(long , struct swt_tran_log *);
short swShmfselect_swt_tran_log_all(struct swt_tran_log *,  short *);
short swShmfselect_overtime_min_tranlog(long ,struct swt_tran_log *);
short swShmfselect_max_tran_begin(long *);
short swShmfselect_max_tran_id(long ,long *);
int   swShmffilefind_tran_log(char *);
short swShmf2shm_swt_proc_log(short );
short swShmfinsert_swt_proc_log(struct swt_proc_log);
short swShmfdelete_swt_proc_log(long ,short);
int   swShmfdelete_swt_proc_log_mrec(long );
short swShmfdelete_proclog_tran_q(long , short);
int   swShmfupdate_swt_proc_log(long ,short , struct swt_proc_log);
int   swShmfupdate_setstat_proclog(long ,short);


int swShmfselect_swt_proc_log(long ,short ,struct swt_proc_log *);
int swShmfselect_swt_proc_log_mrec(long , struct swt_proc_log *,short *);
int swShmfselect_max_proclog_tran(long ,struct swt_proc_log *);
int swShmffilefind_proc_log(char *);
short swShmf2shm_swt_rev_saf(short );
int swShmfinsert_swt_rev_saf(struct swt_rev_saf );
int swShmfdelete_swt_rev_saf(long );
short swShmfupdate_swt_rev_saf(long ,struct swt_rev_saf );
short swShmfselect_swt_rev_saf(long ,struct swt_rev_saf *);
short swShmfselect_swt_rev_saf_all(struct swt_rev_saf *,short *);
short swShmfselect_overtime_min_revsaf(long ,struct swt_rev_saf *);
short swShmfselect_max_saf_begin(long *);
short swShmfselect_max_saf_id(long ,long *);
int swShmfselect_fail_rev_saf(long *,short *);
int swShmfupdate_fail_rev_saf(long );
int swShmisfull();
/* === begin of added by fzj at 2002.03.02 === */
int swShmtranisfull(short);
int swShmportisdown(short, char *);
int swShmresetporttran();
/* === end of added by fzj at 2002.03.02 === */
int swShmselect_swt_sys_other_grp_q(short iOther_grp, short iOther_q,struct swt_sys_other * psSwt_sys_other);
/* modify by nh 20020807
short swShmselect_swt_sys_task_mrec_use_t(char *aTaskuse,struct swt_sys_task *psSwt_sys_task, short *piCount);
short swShmselect_swt_sys_task_mrec_status_t(char *aTaskstatus,struct swt_sys_task *psSwt_sys_task, short *piCount); */
short swShmselect_swt_sys_task_mrec_use_t(char *aTaskuse,struct swt_sys_task **psSwt_sys_task, short *piCount);
short swShmselect_swt_sys_task_mrec_status_t(char *aTaskstatus,struct swt_sys_task **psSwt_sys_task, short *piCount);

int swShmselect_qid_revkey_tranlog(int iQid, char *aKey, struct swt_tran_log *psSwt_tran_log);
int swShmselect_qid_msghdkey_tranlog(int iQid, char *aKey, struct swt_tran_log  *psSwt_tran_log);
int swShmfselect_qid_revkey_tranlog(int iQid, char *aKey, struct swt_tran_log  *psSwt_tran_log);
int swShmfselect_qid_msghdkey_tranlog(int iQid, char *aKey, struct swt_tran_log *psSwt_tran_log);

short swShmportcheck(short iQid,char * aMsghead,short  *iResult); /*add by zjj 2002.02.07 */
int swTranisdisable(short iQid,char * aTrancode); /* add by nh 2002.3.19 */

/* ���ݿ�������� add by xujun 2002.09.11 */
int swDbselect_swt_tran_log(long,struct swt_tran_log *);
int swDbdelete_swt_tran_log(long);
int swDbupdate_swt_tran_log(long ,struct swt_tran_log);
int swDbupdate_setstat_tranlog(long,short);
int swDbinsert_swt_tran_log(struct swt_tran_log sSwt_tran_log);
int swDbinsert_swt_proc_log(struct swt_proc_log sSwt_proc_log);
int swDbselect_swt_proc_log(long lTranid,short iProc_step,
  struct swt_proc_log *psSwt_proc_log);
int swDbselect_swt_proc_log_mrec(long lTranid,
  struct swt_proc_log *psSwt_proc_log,short *iCount);
int swDbdelete_swt_proc_log(long lTranid,short iProc_step);
int swDbupdate_swt_proc_log(long lTranid,short iProc_step,
  struct swt_proc_log sSwt_proc_log);
int swDbupdate_setstat_proclog(long lTranid,short iStatus);
int swDbdelete_swt_proc_log_mrec(long lTranid);
int swDbdelete_proc_tran_q(long lTranid,short iQid);
int swDbselect_max_proclog_tran(long lTranid,struct swt_proc_log *psSwt_proc_log);
int swDbselect_swt_tran_log_all(struct swt_tran_log *psSwt_tran_log,short *iCount);
int swDbselect_max_tran_begin(long *lTran_begin);
int swDbselect_max_tran_id(long lTran_begin,long *lTran_id);
int swDbselect_overtime_min_tranlog(long lTime,struct swt_tran_log *psSwt_tran_log);
int swDbselect_swt_tran_log_clear(long lTran_begin,struct swt_tran_log *psSwt_tran_log);
int swDbselect_swt_proc_log_clear(long lTran_begin,struct swt_proc_log *psSwt_proc_log);
int swShm2db_swt_tran_proc_log(long lTranid);
int swDbfilefind_tran_log(char *aFilename);
int swDbfilefind_proc_log(char *aFilename);
int swDbselect_max_tranid(long *llTranid);
/* add by xujun 2002.09.11 end */
int sw_slock_init();


/***add by wanghao 20150417 PSBC_V1.0***/
int delShmid(int size);
int readShmid(char *buf);
int writeShmid(int shmid);
int clearShm();
int swReleaseZeroAttachedShm(int shmid);
int swShmGetTranDebugLevel(short iQid, char *aTrancode);
short sw_init_free_list();
int swShminsert_swt_rev_saf(struct swt_rev_saf sSwt_rev_saf);
short sw_put_proclog(struct swt_proc_log *proc_log);
/*****end add ******/
#endif

