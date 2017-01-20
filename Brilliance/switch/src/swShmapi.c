/****************************************************************/
/* ģ����    ��swShmapi                                       */
/* ģ������    �������ڴ�api                                    */
/* �� �� ��    ��V4.3.0                                         */
/* ��    ��    ��                                               */
/* ��������    ��2001/05/18                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    �������ڴ��������                               */
/* ��ģ���а������º���������˵����                             */
/****************************************************************/
/*		swShmcfgload()					*/
/*		swShmcheck()					*/
/*		swShmP()					*/
/*		swShmV()					*/
/*============== ��̬�� ========================================*/
/*		swShmselect_swt_sys_imf()			*/ 
/*		swShmselect_swt_sys_imf_all()			*/
/*		swShmupdate_swt_sys_queue()			*/
/*		swShmselect_swt_sys_queue()			*/ 
/*		swShmupdate_swt_sys_task()			*/
/*		swShmselect_swt_sys_task()			*/
/*		swShmselect_swt_sys_task_pid()			*/
/*		swShmselect_swt_sys_task_mrec_use_t()		*/
/*		swShmselect_swt_sys_task_mrec_status_t()	*/
/*		swShmselect_swt_sys_task_all_p()		*/
/*		swShmselect_swt_sys_task_all_q()		*/
/*		swShmselect_swt_sys_task_qid()	        	*/
/*		swShmselect_swt_sys_8583()			*/
/*		swShmselect_swt_sys_8583E()			*/
/*		swShmselect_swt_sys_code()			*/
/*		swShmselect_swt_sys_config()			*/
/*		swShmselect_swt_sys_other()			*/
/*		swShmselect_swt_sys_fmt_m()			*/
/*		swShmselect_swt_sys_fmt_grp()			*/
/*		swShmselect_swt_sys_fmt_grp_offset()		*/
/*		swShmselect_swt_sys_fmt_d_offset()		*/
/*		swShmselect_swt_sys_route_m()			*/
/*		swShmselect_swt_sys_route_d()			*/
/*		swShmselect_route_d_q_tran()			*/
/*      ����3������ΪHASH��                     */
/*      shm_swt_sys_fmt_m                       */
/*      shm_swt_sys_fmt_g                       */
/*      shm_swt_sys_route_m                     */
/*      swShmtranflowisfull                     */
/*      swShmGetTranDebugLevel                  */
/*============= ��ˮ�� =========================================*/
/*		swShmselect_swt_tran_log_all()			*/
/*		swShmselect_overtime_min_tranlog()		*/
/*		swShmselect_max_tran_begin()			*/
/*		swShmselect_max_tran_id()			*/
/*              swShmselect_swt_tran_log_clear()                */
/*		swShmsettranid()				*/
/*		swShmgettranid()				*/

/*		swShminsert_swt_proc_log()			*/
/*		swShmdelete_swt_proc_log()			*/
/*		swShmdelete_swt_proc_log_mrec()			*/
/*		swShmdelete_proclog_tran_q()			*/
/*		swShmupdate_swt_proc_log()			*/
/*		swShmupdate_setstat_proclog()			*/
/*		swShmselect_swt_proc_log()			*/
/*		swShmselect_swt_proc_log_mrec()			*/
/*              swShmselect_swt_proc_log_clear()                */
/*		swDbinsert_swt_rev_saf()			*/
/*		swDbdelete_swt_rev_saf()			*/
/*		swDbupdate_swt_rev_saf()			*/
/*		swDbselect_swt_rev_saf()			*/
/*		swDbselect_overtime_min_revsaf()		*/
/*		swDbselect_swt_rev_saf_all()			*/
/*		swDbselect_max_saf_begin()			*/
/*		swDbselect_max_saf_id()				*/
/*              swDbinsert_swt_tran_log()			*/
/*              swDbdelete_swt_tran_log()			*/
/*              swDbupdate_swt_tran_log()			*/
/*              swDbselect_swt_tran_log()			*/
/*              swDbinsert_swt_proc_log()			*/
/*              swDbdelete_swt_proc_log()			*/
/*              swDbupdate_swt_proc_log()			*/
/*              swDbselect_swt_proc_log()			*/
/*		swShmsetsafid()					*/
/*		swShmgetsafid()					*/
/*============= �˿ڲ������� ===================================*/
/*		short swCheckportid()				*/
/*		swPortoper()					*/
/* add by nh    swShmtrandisable()                              */
/****************************************************************/

#include "switch.h"
/* add by qzg*/
#include "swShmGroup.h"
#include "sw_lock.h"
#define SFTRANLOGID	0
#define SFPROCLOGID	1
#define SFSAFLOGID	2
#define OFFSETNULL 	-1

#define WITH_TIME_MSEC
//����ʱ��,�������ʱ��Ϣ
#ifdef WITH_TIME_MSEC

#include <sys/time.h>
#define _WITH_ITME_START struct timeval tv_start, tv_end; do{\
                    gettimeofday(&tv_start, NULL);\
                }while(0);
#define _WITH_ITME_END do{\
                    gettimeofday(&tv_end, NULL);\
                    float microsec = (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + \
                            (tv_end.tv_usec - tv_start.tv_usec);\
                    swVdebug(0, "[%s, func_name:%s, line:%d] took %0.6fs time\r\n", \
                            __FILE__, __func__, __LINE__, microsec/ 1000000); \
}while(0);
#else

#include <time.h>
#define _WITH_ITME_START time_t ct_start, ct_end; do{\
                    time (&ct_start);\
                }while(0);
#define _WITH_ITME_END do{\
                    time (&ct_end);\
                    swVdebug(0, "[%s, func_name:%s, line:%d] took %us time\r\n",\
                            __FILE__, __func__, __LINE__, ct_end-ct_start);\
}while(0);
#endif

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/

/* ���ݿⶨ�� */
EXEC SQL INCLUDE sqlca;
#ifdef DB_POSTGRESQL
EXEC SQL INCLUDE swDbstruct.h;
#endif
#ifdef DB_DB2
EXEC SQL INCLUDE '../inc/swDbstruct.h';
#endif
#ifdef DB_INFORMIX
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_ORACLE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_SYBASE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#else/*support database,Database Deprivation Project 2014-12*/
#include "swDbstruct.h"
#endif
#include "swNdbstruct.h"
#include "swShm.h"

/*2009-9-9 9:41 ������Ϣ�洢�ṹ��*/
struct GrpComInfo {
    char groupid[4];		/*���*/
    char ip[16];				/*��Ŷ�Ӧ����IP*/
    int port;						/*��Ŷ�Ӧ��������˿�*/
    int minserialno;		/*��Ŷ�Ӧ��ʼ��ˮ��*/
    int maxserialno;    /*��Ŷ�Ӧ��ֹ��ˮ��*/
};
/*2009-9-9 9:41 ������Ϣ�洢�ṹ��*/

/*static*/ struct swt_sys_config smSwt_sys_config;
/*static*/ struct shmidx_s *psmShmidx_s;   /* ��̬�����ڴ�Ŀ¼��ָ�� */
/*static*/ struct shmidx_d *psmShmidx_d;   /* ��̬�����ڴ�Ŀ¼��ָ�� */
/*static*/ struct shmbuf_s smShmbuf_s;     /* ��̬�����ڴ����ݶ�ָ�� */
/*static*/ struct shmbuf_d smShmbuf_d;     /* ��̬�����ڴ����ݶ�ָ�� */
extern unsigned long wait_count;
extern unsigned long lock_count;

static char amShmfile[3][101];
/*add by zcd 20141225*/
static char asw_tran_log_file[256];
static char asw_proc_log_file[256];
/*end of add by zcd 20141225*/
key_t iSHEGROUP=-1;
int igGrpShmid;

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
EXEC SQL BEGIN DECLARE SECTION;
short  igTmp;   
short  igQid;
short  igStatus;
short  igProc_step;
short  igCount;
char   agFilename[11];
char   agKey[151];
sqlint64   lgTime;
sqlint64   lgTranid;  
sqlint64   lgTran_begin;
sqlint64   lgSaf_id;
static struct swt_tran_log1
{
    sqlint64  tran_id;
    sqlint64  tran_begin;
    sqlint64  tran_end;
    sqlint64  tran_overtime;
    sqlint64  resq_offset;
    sqlint64  resp_offset;
    short tran_status;
    short q_tran_begin;
    short q_target;
    short resq_len;
    short resp_len;
    char  tc_tran_begin[11];
    char  resq_file[11];
    char  resp_file[11];
    char  rev_key[151];
    char  msghdkey[151];
    char  msghead[151];
    char  xastatus[21];           /* ��¼XA״̬ */
    char  a1[3];
}sgSwt_tran_log1;
static struct swt_proc_log1
{
    sqlint64      tran_id;
    sqlint64  proc_begin;
    sqlint64  org_offset;
    short q_target;
    short proc_step;
    short proc_status;
    short rev_mode;
    /*del by zcd 20141220
      short rev_fmtgrp;
     ***end of del by zcd 20141220*/
    /*add by zcd 20141220*/
    long  rev_fmtgrp;
    /*end of add by zcd 20141220*/
    short org_len;
    char  org_file[11];
    char  a1;
}sgSwt_proc_log1;
EXEC SQL END DECLARE SECTION;
#endif

#ifdef DATABASE
EXEC SQL BEGIN DECLARE SECTION;
short  igTmp;   
short  igQid;
short  igStatus;
short  igProc_step;
short  igCount;
char   agFilename[11];
char   agKey[151];
long   lgTime;
long   lgTranid;  
long   lgTran_begin;
long   lgSaf_id;
static struct swt_tran_log1
{
    long  tran_id;
    long  tran_begin;
    long  tran_end;
    long  tran_overtime;
    long  resq_offset;
    long  resp_offset;
    short tran_status;
    short q_tran_begin;
    short q_target;
    short resq_len;
    short resp_len;
    char  tc_tran_begin[11];
    char  resq_file[11];
    char  resp_file[11];
    char  rev_key[151];
    char  msghdkey[151];
    char  msghead[151];
    char  xastatus[21];           /* ��¼XA״̬ */
    char  a1[3];
}sgSwt_tran_log1;
static struct swt_proc_log1
{
    long      tran_id;
    long  proc_begin;
    long  org_offset;
    short q_target;
    short proc_step;
    short proc_status;
    short rev_mode;
    /*del by zcd 20141220
      short rev_fmtgrp;
     ***end of del by zcd 20141220*/
    /*add by zcd 20141220*/
    long  rev_fmtgrp;
    /*end of add by zcd 20141220*/  
    short org_len;
    char  org_file[11];
    char  a1;
}sgSwt_proc_log1;
EXEC SQL END DECLARE SECTION;
#endif

#endif   /*support database,Database Deprivation Project 2014-12*/

#if 0
/***************************************************************
 ** ������      : swShmcfgload
 ** ��  ��      : �������ڴ��ʼ������
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmcfgload(struct swt_sys_config * pSwt_sys_config)
{
    char alBuf[257],alResult[101],*alTmp;
    int i;
    FILE *fp;

    sprintf(amShmfile[0], "%s/log/shm/tranlog", getenv("SWITCH_DIR"));

    sprintf(amShmfile[1], "%s/log/shm/proclog", getenv("SWITCH_DIR"));

    sprintf(amShmfile[2], "%s/log/shm/saflog", getenv("SWITCH_DIR"));

    if ((fp = _swFopen("config/SWCONFIG.CFG", "r")) == NULL) return -1; 
    i = 0;
    while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
    {
        _swTrim(alBuf);
        if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
        if ((alTmp = strchr(alBuf,'#')) != NULL) *alTmp = '\0';
        _swTrim(alBuf);
        if ((alBuf[0] == '[') && (alBuf[strlen(alBuf) -1] == ']')) continue;
        if ((alTmp = strchr(alBuf, '=')) == NULL) continue;
        strcpy(alResult,alTmp + 1);
        _swChgvalue(alResult);
        *alTmp = '\0';
        _swTrim(alBuf);
        if (strcmp(alBuf,"SHMKEY") == 0)
            pSwt_sys_config->iShmkey = atoi(alResult);
        if (strcmp(alBuf,"LOG_INTER") == 0)
            pSwt_sys_config->log_inter = atoi(alResult);
        if (strcmp(alBuf,"SAF_INTER") == 0)
            pSwt_sys_config->saf_inter = atoi(alResult);
        /* ========= begin of add by dgm 2002.3.19 ========== */
        if (strcmp(alBuf,"REREV_INTER") == 0)
            pSwt_sys_config->rerev_inter = atol(alResult);
        /* ========== end of add by dgm 2002.3.19 =========== */
        if (strcmp(alBuf,"MAXTRANLOG") == 0)
            pSwt_sys_config->iMaxtranlog = atoi(alResult);
        if (strcmp(alBuf,"MAXPROCLOG") == 0)
            pSwt_sys_config->iMaxproclog = atoi(alResult);
        if (strcmp(alBuf,"MAXSAFLOG") == 0)
            pSwt_sys_config->iMaxsaflog = atoi(alResult);
        if (strcmp(alBuf,"SHMWAITTIME") == 0)
            pSwt_sys_config->iShmwaittime = atoi(alResult);
        if (strcmp(alBuf,"SYSMON_IP") == 0)
            strcpy(pSwt_sys_config->sysmonaddr,alResult);
        if (strcmp(alBuf,"SYSMON_PORT") == 0)
            pSwt_sys_config->sysmonport = atoi(alResult);
        if (strcmp(alBuf,"TRACE_TERM") == 0)
            strcpy(pSwt_sys_config->trace_term, alResult);
    }
    fclose(fp);
    return 0;
} 
#endif

/***************************************************************
 ** ������      : swShmcheck
 ** ��  ��      : ��鹲���ڴ�״̬
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmcheck()
{
    static int i = -1;
    static int ilShmid_sav = -1;
    int ilShmid;
    int old_shmid;/*add by dyw, 2015.03.27 PSBC_V1.0*/
    int k;
    char *alShmtmp_d, *alShmtmp_s;

    if (i == -1)
    {
        /* ��swt_sys_config������ */
        if (swShmcfgload(&smSwt_sys_config) != 0) return -1;

        /* ������̬�����ڴ� */
        ilShmid = shmget((key_t)smSwt_sys_config.iShmkey,0,IPC_EXCL|0666);
        if (ilShmid < 0) 
        {
            swVdebug(0,"S5014: [����/ϵͳ����] shmget()����,shmkey=[%d] errno=%d",smSwt_sys_config.iShmkey,errno);
            return -1;
        }

        /* ���Ӷ�̬�����ڴ� */
        if ((alShmtmp_d = shmat(ilShmid,(char *)0,SHM_RND)) == (char *)-1)
        {
            swVdebug(0,"S5015: [����/ϵͳ����] shmat()����,errno=%d",errno);
            return -1;
        }
        /*place the slock array*/
        pg_lock_array = (sw_slock_array_t*)(alShmtmp_d);
        /*move forward to skip sw_lock_array_t size*/
        alShmtmp_d += sizeof(sw_slock_array_t);


        /* �ö�̬�����ڴ�ָ�� */
        psmShmidx_d = (struct shmidx_d *)  alShmtmp_d ;

        for(k=0; k< SHM_TRANLOG_BUCKET_NUM; k++)
        {
            smShmbuf_d.psShm_tran_log[k] = (struct shm_tran_log *)
                (alShmtmp_d + psmShmidx_d->sIdx_tran_log[k].lOffset);
        }
        smShmbuf_d.psShm_proc_log = (struct shm_proc_log *)
            (alShmtmp_d + psmShmidx_d->sIdx_proc_log.lOffset);

        smShmbuf_d.psShm_rev_saf = (struct shm_rev_saf *)
            (alShmtmp_d + psmShmidx_d->sIdx_rev_saf.lOffset);
    }  
    /*������swInit��ʼ��ϵͳʱ����ֹ����������δ��ɳ�ʼ��֮ǰ����*/
    if (psmShmidx_d->iShmid == -1)
    {
        for (;;)
        {
            sleep(1);
            if (psmShmidx_d->iShmid != -1) break;
        }
    }
    /*=============================================================*/


    if (psmShmidx_d->iShmid != ilShmid_sav)
    {
        if (i == -1)
            i = 0;
        else
        {
            if (shmdt(psmShmidx_s))
            {
                swVdebug(0,"S5020: [����/ϵͳ����] shmdt()����,errno=%d",errno);
                return -1;
            }
        }
        if ((alShmtmp_s = shmat(psmShmidx_d->iShmid,(char *)0,SHM_RND)) == (char *)-1)
        {
            swVdebug(0,"S5025: [����/ϵͳ����] shmat()����,errno=%d",errno);
            return -1;
        }
        /* ���þ�̬�����ڴ�ָ�� */
        psmShmidx_s = (struct shmidx_s *)alShmtmp_s;

        smShmbuf_s.psSwt_sys_imf = (struct swt_sys_imf *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_imf.lOffset);
        smShmbuf_s.psSwt_sys_queue = (struct swt_sys_queue *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_queue.lOffset);
        /* === begin of added by fzj at 2002.02.28 === */
        smShmbuf_s.psSwt_sys_matchport = (struct swt_sys_matchport *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_matchport.lOffset);
        /* === end of added by fzj at 2002.02.28 === */
        smShmbuf_s.psSwt_sys_task = (struct swt_sys_task *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_task.lOffset);
        smShmbuf_s.psSwt_sys_8583 = (struct swt_sys_8583 *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_8583.lOffset);
        smShmbuf_s.psSwt_sys_8583E = (struct swt_sys_8583E *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_8583E.lOffset);		
        smShmbuf_s.psSwt_sys_code = (struct swt_sys_code *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_code.lOffset);
        smShmbuf_s.psSwt_sys_config = (struct swt_sys_config *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_config.lOffset);
        smShmbuf_s.psSwt_sys_other = (struct swt_sys_other *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_other.lOffset);
        smShmbuf_s.psShm_sys_fmt_m = (struct shm_sys_fmt_m *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_m.lOffset);
        smShmbuf_s.psShm_sys_fmt_g = (struct shm_sys_fmt_g *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_g.lOffset);
        smShmbuf_s.psSwt_sys_fmt_d = (struct swt_sys_fmt_d *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_d.lOffset);

        smShmbuf_s.psShm_sys_route_m = (struct shm_sys_route_m *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_route_m.lOffset);
        smShmbuf_s.psShm_sys_route_g = (struct shm_sys_route_g *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_route_g.lOffset);
        smShmbuf_s.psSwt_sys_route_d = (struct swt_sys_route_d *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_route_d.lOffset);
#ifndef HASH
        /* add by gengling at 2015.03.20 begin PSBC_V1.0 */
        smShmbuf_s.psSwt_sys_tran = (struct swt_sys_tran *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_tran.lOffset);
        /* add by gengling at 2015.03.20 end PSBC_V1.0 */
#endif
#ifdef HASH
        smShmbuf_s.psSwt_sys_tran_hash = (void *)((void*)smShmbuf_s.psSwt_sys_route_d + 
             psmShmidx_s->sIdx_sys_route_d.iCount * sizeof(struct swt_sys_route_d));
        swVdebug(5, "swShmcheck smShmbuf_s=[%p] smShmbuf_s.psSwt_sys_tran_hash=[%p] iCount=[%d]",
                &smShmbuf_s, smShmbuf_s.psSwt_sys_tran_hash, psmShmidx_s->sIdx_sys_route_d.iCount);

        long dataoffset=0;
        int number=0;
        memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
        memcpy(&number, smShmbuf_s.psSwt_sys_tran_hash+sizeof(long), sizeof(int));
        smShmbuf_s.psShm_sys_fmt_m_hash = smShmbuf_s.psSwt_sys_tran_hash + dataoffset + number*sizeof(struct swt_sys_tran);
        swVdebug(5, "swShmcheck smShmbuf_s=[%p] smShmbuf_s.psShm_sys_fmt_m_hash=[%p] psSwt_sys_tran_hash.number=[%d]",
                &smShmbuf_s, smShmbuf_s.psShm_sys_fmt_m_hash, number);

        dataoffset = 0;
        number = 0;
        memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_m_hash, sizeof(long));
        memcpy(&number, smShmbuf_s.psShm_sys_fmt_m_hash + sizeof(long), sizeof(int));
        smShmbuf_s.psShm_sys_fmt_g_hash = smShmbuf_s.psShm_sys_fmt_m_hash + dataoffset + number*sizeof(struct shm_sys_fmt_m);
        swVdebug(5, "swShmcheck smShmbuf_s=[%p] smShmbuf_s.psShm_sys_fmt_g_hash=[%p] psShm_sys_fmt_m_hash.number=[%d]",
                &smShmbuf_s, smShmbuf_s.psShm_sys_fmt_g_hash, number);
        
        dataoffset = 0;
        number = 0;
        memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_g_hash, sizeof(long));
        memcpy(&number, smShmbuf_s.psShm_sys_fmt_g_hash + sizeof(long), sizeof(int));
        smShmbuf_s.psShm_sys_route_m_hash = smShmbuf_s.psShm_sys_fmt_g_hash + dataoffset + number*sizeof(struct shm_sys_fmt_g);
        swVdebug(5, "swShmcheck smShmbuf_s=[%p] smShmbuf_s.psShm_sys_route_m_hash=[%p] psSwt_sys_fmt_g_hash.number=[%d]",
                &smShmbuf_s, smShmbuf_s.psShm_sys_route_m_hash, number);
#endif
        old_shmid =  ilShmid_sav; /*add by dyw, 2015.03.27, PSBC_V1.0*/
        ilShmid_sav = psmShmidx_d->iShmid;
        if(old_shmid != -1)
        {
            swReleaseZeroAttachedShm( old_shmid ); /*add by dyw, 2015.03.27, PSBC_V1.0*/
            swVdebug(4,"��̬�����ڴ�id��[%ld]�л�Ϊ[%ld]:", old_shmid, ilShmid_sav);
        }
        else
        {
            swVdebug(4,"��̬�����ڴ�id[%ld]:",  ilShmid_sav);
        }
    }
    return 0;
}

/***************************************************************
 ** ������      : _swShmcheck
 ** ��  ��      : ����swInit�ָ���������ʱ�ܵ��øÿ��еĺ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int _swShmcheck()
{
    static int ilFlag = 0;	
    char *alShmtmp_d, *alShmtmp_s;

    int k;

    if(ilFlag) return 0;

    /* ��swt_sys_config������ */
    if (swShmcfgload(&smSwt_sys_config) != 0) return -1;

    /*place the slock array*/
    for(k=0; k < iSEMNUM;k++)
    {
        S_LOCK_FREE( (pg_lock_array->lock) + k);   
    }
    /* �ö�̬�����ڴ�ָ�� */

    psmShmidx_d = psgShmidx_d;
    alShmtmp_d = (char *)psgShmidx_d;

    for(k=0; k< SHM_TRANLOG_BUCKET_NUM; k++) /*initialize the tranlog bucket, add by dyw*/
    {    	
        smShmbuf_d.psShm_tran_log[k] = (struct shm_tran_log *)
            (alShmtmp_d + psmShmidx_d->sIdx_tran_log[k].lOffset);
    }

    smShmbuf_d.psShm_proc_log = (struct shm_proc_log *)
        (alShmtmp_d + psmShmidx_d->sIdx_proc_log.lOffset);
    smShmbuf_d.psShm_rev_saf = (struct shm_rev_saf *)
        (alShmtmp_d + psmShmidx_d->sIdx_rev_saf.lOffset);



    /* �þ�̬�����ڴ�ָ�� */
    if(psgShmidx_s != NULL)
    {
        psmShmidx_s = psgShmidx_s;
        alShmtmp_s = (char *)psgShmidx_s;

        smShmbuf_s.psSwt_sys_imf = (struct swt_sys_imf *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_imf.lOffset);
        smShmbuf_s.psSwt_sys_queue = (struct swt_sys_queue *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_queue.lOffset);
        smShmbuf_s.psSwt_sys_matchport = (struct swt_sys_matchport *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_matchport.lOffset);
        smShmbuf_s.psSwt_sys_task = (struct swt_sys_task *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_task.lOffset);
        smShmbuf_s.psSwt_sys_8583 = (struct swt_sys_8583 *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_8583.lOffset);
        smShmbuf_s.psSwt_sys_8583E = (struct swt_sys_8583E *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_8583E.lOffset);		
        smShmbuf_s.psSwt_sys_code = (struct swt_sys_code *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_code.lOffset);
        smShmbuf_s.psSwt_sys_config = (struct swt_sys_config *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_config.lOffset);
        smShmbuf_s.psSwt_sys_other = (struct swt_sys_other *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_other.lOffset);
        smShmbuf_s.psShm_sys_fmt_m = (struct shm_sys_fmt_m *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_m.lOffset);
        smShmbuf_s.psShm_sys_fmt_g = (struct shm_sys_fmt_g *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_g.lOffset);
        smShmbuf_s.psSwt_sys_fmt_d = (struct swt_sys_fmt_d *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_d.lOffset);

        smShmbuf_s.psShm_sys_route_m = (struct shm_sys_route_m *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_route_m.lOffset);
        smShmbuf_s.psShm_sys_route_g = (struct shm_sys_route_g *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_route_g.lOffset);
        smShmbuf_s.psSwt_sys_route_d = (struct swt_sys_route_d *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_route_d.lOffset);
#ifndef HASH
        /* add by gengling at 2015.03.23 begin PSBC_V1.0 */
        smShmbuf_s.psSwt_sys_tran = (struct swt_sys_tran *)
            (alShmtmp_s + psmShmidx_s->sIdx_sys_tran.lOffset);
        /* add by gengling at 2015.03.23 end PSBC_V1.0 */
#endif
#ifdef HASH
        smShmbuf_s.psSwt_sys_tran_hash = (void *)((void*)smShmbuf_s.psSwt_sys_route_d + 
             psmShmidx_s->sIdx_sys_route_d.iCount * sizeof(struct swt_sys_route_d));
        swVdebug(5, "swShmcheck smShmbuf_s=[%p] smShmbuf_s.psSwt_sys_tran_hash=[%p] iCount=[%d]",
                &smShmbuf_s, smShmbuf_s.psSwt_sys_tran_hash, psmShmidx_s->sIdx_sys_route_d.iCount);

        long dataoffset=0;
        int number=0;
        memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
        memcpy(&number, smShmbuf_s.psSwt_sys_tran_hash+sizeof(long), sizeof(int));
        smShmbuf_s.psShm_sys_fmt_m_hash = smShmbuf_s.psSwt_sys_tran_hash + dataoffset + number*sizeof(struct swt_sys_tran);
        swVdebug(5, "_swShmcheck smShmbuf_s=[%p] smShmbuf_s.psShm_sys_fmt_m_hash=[%p] psSwt_sys_tran_hash.number",
                &smShmbuf_s, smShmbuf_s.psShm_sys_fmt_m_hash, number);

        dataoffset = 0;
        number = 0;
        memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_m_hash, sizeof(long));
        memcpy(&number, smShmbuf_s.psShm_sys_fmt_m_hash + sizeof(long), sizeof(int));
        smShmbuf_s.psShm_sys_fmt_g_hash = smShmbuf_s.psShm_sys_fmt_m_hash + dataoffset + number*sizeof(struct shm_sys_fmt_m);
        swVdebug(5, "swShmcheck smShmbuf_s=[%p] smShmbuf_s.psShm_sys_fmt_g_hash=[%p] psShm_sys_fmt_m_hash.number=[%d]",
                &smShmbuf_s, smShmbuf_s.psShm_sys_fmt_g_hash, number);
        
        dataoffset = 0;
        number = 0;
        memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_g_hash, sizeof(long));
        memcpy(&number, smShmbuf_s.psShm_sys_fmt_g_hash + sizeof(long), sizeof(int));
        smShmbuf_s.psShm_sys_route_m_hash = smShmbuf_s.psShm_sys_fmt_g_hash + dataoffset + number*sizeof(struct shm_sys_fmt_g);
        swVdebug(5, "swShmcheck smShmbuf_s=[%p] smShmbuf_s.psShm_sys_route_m_hash=[%p] psSwt_sys_fmt_g_hash.number=[%d]",
                &smShmbuf_s, smShmbuf_s.psShm_sys_route_m_hash, number);
#endif
    }  		

    ilFlag = 1; 
    return 0;
}

/***************************************************************
 ** ������      : swShmP
 ** ��  ��      : ���źŵƽ���P����
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmP(int iSem_num)
{
#ifdef NOT_HAVE_TAS
    struct sembuf p_buf;

    p_buf.sem_num = iSem_num;
    p_buf.sem_op = -1;
    p_buf.sem_flg = SEM_UNDO;

    if (semop(psmShmidx_d->iSemid,&p_buf,1) == -1)
    {
        swVdebug(0,"S5030: [����/ϵͳ����] semop()����,errno=%d",errno);
        return(FAIL);
    }
    else
    {
        return(SUCCESS);
    }

#else
    S_LOCK((pg_lock_array->lock) + iSem_num );
    return SUCCESS;
#endif
}

/***************************************************************
 ** ������      : swShmV
 ** ��  ��      : ���źŵƽ���V����
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmV(int iSem_num)
{
#ifdef  NOT_HAVE_TAS
    struct sembuf v_buf;

    v_buf.sem_num = iSem_num;
    v_buf.sem_op = 1;
    v_buf.sem_flg = SEM_UNDO;

    if (semop(psmShmidx_d->iSemid,&v_buf,1) == -1)
    {
        swVdebug(0,"S5035: [����/ϵͳ����] semop()����,errno=%d",errno);
        return(FAIL);
    }
    else
        return(SUCCESS);
#else
    S_UNLOCK((pg_lock_array->lock) + iSem_num );
    /*
       swVdebug(0, "S5036: total lock ");
       swVdebug(0, "S5036: total lock count=[%ld], average wait=[%ld]", lock_count, wait_count/lock_count);
       if(lock_count%1000==0) swVdebug(0, "S5036: total lock count=%ld, average wait=[%ld]", lock_count, wait_count/lock_count);
     */
    return SUCCESS;
#endif
}

/*================= ��̬�����ڴ�������� ====================*/

/***************************************************************
 ** ������      : swShmselect_swt_sys_imf
 ** ��  ��      : swt_sys_imf����ݹؼ��ֲ���
 **             : 
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_imf(char * aImf_name, 
        struct swt_sys_imf * psSwt_sys_imf)
{
    int i;

    for(i = 0;i < psmShmidx_s->sIdx_sys_imf.iCount;i++)
    {
        if(strcmp(aImf_name,smShmbuf_s.psSwt_sys_imf[i].imf_name) == 0)
        {
            memcpy(psSwt_sys_imf,
                    &smShmbuf_s.psSwt_sys_imf[i],
                    sizeof(struct swt_sys_imf));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_imf.iCount)
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}

/**************************************************************
 ** ������	:  short swShmselect_swt_sys_imf_all()
 ** ��  ��	:  ����swt_sys_imf�����м�¼����¼��
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	:  SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_imf_all(struct swt_sys_imf *psSwt_sys_imf, 
        short *iImfcount)
{
    int i;
    short ilCount;

    if ((ilCount = psmShmidx_s->sIdx_sys_imf.iCount) > iMAXIMFREC)
        return (FAIL);

    for (i=0; i < ilCount; i++)
    {
        memcpy((char *)&psSwt_sys_imf[i], (char *)&smShmbuf_s.psSwt_sys_imf[i], 
                sizeof(struct swt_sys_imf));
    }
    *iImfcount = ilCount;

    return (SUCCESS);

}  

/***************************************************************
 ** ������      : swShmupdate_swt_sys_queue
 ** ��  ��      : swt_sys_queue����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmupdate_swt_sys_queue(short iQ_id, 
        struct swt_sys_queue sSwt_sys_queue)
{
    int i,ilTmp = 0;

    for (i = 0; i < psmShmidx_s->sIdx_sys_queue.iCount; i ++)
    {
        if (smShmbuf_s.psSwt_sys_queue[i].q_id == iQ_id)
        {
            if (ilTmp == 0) ilTmp = 1;
            memcpy((char *)&(smShmbuf_s.psSwt_sys_queue[i]), 
                    (char *)&sSwt_sys_queue, sizeof(struct swt_sys_queue));
            break;
        }
    }
    if (ilTmp == 1)
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}          

/***************************************************************
 ** ������      : swShmselect_swt_sys_queue
 ** ��  ��      : swt_sys_queue����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_queue(short iQ_id, 
        struct swt_sys_queue * psSwt_sys_queue)
{
    int i,ilTmp = 0;

    for (i = 0; i < psmShmidx_s->sIdx_sys_queue.iCount; i ++)
    {
        if (smShmbuf_s.psSwt_sys_queue[i].q_id == iQ_id)
        {
            if (ilTmp == 0) ilTmp = 1;
            memcpy((char *)psSwt_sys_queue, (char *)(&(smShmbuf_s.
                            psSwt_sys_queue[i])), sizeof(struct swt_sys_queue));
            break;
        }
    }
    if (ilTmp == 1)
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}          

/***************************************************************
 ** ������      : swShmselect_swt_sys_queue_all
 ** ��  ��      : swt_sys_queue�����ȫ��
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_queue_all(struct swt_sys_queue * psSwt_sys_queue,
        short *iCount)
{
    int i = 0;

    if(psmShmidx_s->sIdx_sys_queue.iCount > 0)
    {
        i = psmShmidx_s->sIdx_sys_queue.iCount;
        memcpy(psSwt_sys_queue,smShmbuf_s.psSwt_sys_queue,
                i*sizeof(struct swt_sys_queue));
    }
    *iCount = psmShmidx_s->sIdx_sys_queue.iCount;
    if(i == 0)
        return(SHMNOTFOUND);

    return(SUCCESS);
}

/**************************************************************
 ** ������	:  short swShmupdate_swt_sys_task()
 ** ��  ��	:  swt_sys_task���ؼ���task_name����������¼
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	:  SUCCESS , FAIL
 ***************************************************************/
short swShmupdate_swt_sys_task(char *aTaskname, 
        struct swt_sys_task sSwt_sys_task)
{
    short i;

    for (i=0; i < psmShmidx_s->sIdx_sys_task.iCount; i++)
    {
        if (strcmp(aTaskname, smShmbuf_s.psSwt_sys_task[i].task_name) == 0)
        {
            memcpy((char *)&(smShmbuf_s.psSwt_sys_task[i]), 
                    (char *)&sSwt_sys_task, sizeof(struct swt_sys_task));

            return (SUCCESS);
        } 
    }

    return (SHMNOTFOUND); 
} 

/***************************************************************
 ** ������      : swShmselect_swt_sys_task
 ** ��  ��      : swt_sys_task����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_task(char * aTask_name,
        struct swt_sys_task * psSwt_sys_task)
{
    int i;

    for(i = 0;i < psmShmidx_s->sIdx_sys_task.iCount;i++)
    {
        if(strcmp(aTask_name,smShmbuf_s.psSwt_sys_task[i].task_name) == 0)
        {
            memcpy(psSwt_sys_task,
                    &smShmbuf_s.psSwt_sys_task[i],
                    sizeof(struct swt_sys_task));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_task.iCount)
        return(SUCCESS);
    else
        return SHMNOTFOUND;
}

/**************************************************************
 ** ������	: short swShmselect_swt_sys_task_pid()
 ** ��  ��	: swt_sys_task��pid����
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_task_pid(int iPidcld, 
        struct swt_sys_task *psSwt_sys_task)
{
    short i;

    memset((char *)psSwt_sys_task, 0x00, sizeof(struct swt_sys_task));

    for (i=0; i < psmShmidx_s->sIdx_sys_task.iCount; i++)
    {
        if (iPidcld == smShmbuf_s.psSwt_sys_task[i].pid)
        {
            memcpy((char *)psSwt_sys_task, (char *)&(smShmbuf_s.psSwt_sys_task[i]),
                    sizeof(struct swt_sys_task));
            return (SUCCESS);
        } 
    }

    return (SHMNOTFOUND); 
} 

/**************************************************************
 ** ������	: short swShmselect_swt_sys_task_mrec_use_t()
 ** ��  ��	: swt_sys_task��task_use���� (�α����),��start_id����
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_task_mrec_use_t(char *aTaskuse,
        struct swt_sys_task **psSwt_sys_task1, short *piCount)
{
    short i, j, k, ilCount;
    struct swt_sys_task slSwt_sys_task;
    static struct swt_sys_task *psSwt_sys_task=NULL;     /* add by nh 20020807 */

    if(psSwt_sys_task)
    {
        free(psSwt_sys_task);
        psSwt_sys_task = NULL;
    }
    ilCount = 0;

    for (i=0; i < psmShmidx_s->sIdx_sys_task.iCount; i++)
    {
        if (strcmp(aTaskuse, smShmbuf_s.psSwt_sys_task[i].task_use) == 0)
        {
            if(ilCount==0)
                psSwt_sys_task=(struct swt_sys_task *)malloc(sizeof(struct swt_sys_task));
            else
                psSwt_sys_task=(struct swt_sys_task *)realloc(psSwt_sys_task,(ilCount+1)*sizeof(struct swt_sys_task));
            if(psSwt_sys_task==NULL)
            {
                swVdebug(0,"S5040:��̬�����ڴ�ʧ��!,%d,%s",errno,strerror(errno));
                return(-1);
            }  
            memcpy((char *)&(psSwt_sys_task[ilCount]), 
                    (char *)&(smShmbuf_s.psSwt_sys_task[i]), sizeof(struct swt_sys_task));
            ilCount ++;
        } 
    }
    *piCount = ilCount;
    k = 0;
    for (i=0; i < (ilCount - 1); i++)
    {
        k = 0;
        for (j=1; j < (ilCount - i); j++)
        {
            if (psSwt_sys_task[j - 1].start_id > psSwt_sys_task[j].start_id)
            {
                memcpy((char *)&slSwt_sys_task, (char *)&(psSwt_sys_task[j]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j]), (char *)&(psSwt_sys_task[j - 1]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j - 1]), (char *)&slSwt_sys_task, 
                        sizeof(struct swt_sys_task));
                k = 1;
            } 
        }
        if (k == 0) break;
    }

    *psSwt_sys_task1=psSwt_sys_task; /* add by nh 20020807 */
    return (SUCCESS); 
} 

/**************************************************************
 ** ������	: short swShmselect_swt_sys_task_mrec_status_t()
 ** ��  ��	: swt_sys_task��task_status���� (�α����),��start_id����
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_task_mrec_status_t(char *aTaskstatus, 
        struct swt_sys_task **psSwt_sys_task1, short *piCount)
{
    short i, j, k, ilCount;
    struct swt_sys_task slSwt_sys_task;
    static struct swt_sys_task *psSwt_sys_task=NULL;    /* add by nh 20020807 */

    if(psSwt_sys_task)
    {
        free(psSwt_sys_task);
        psSwt_sys_task = NULL;
    }
    ilCount = 0;
    for (i=0; i < psmShmidx_s->sIdx_sys_task.iCount; i++)
    {
        if (strcmp(aTaskstatus, smShmbuf_s.psSwt_sys_task[i].task_status) == 0)
        {
            if(ilCount==0)
                psSwt_sys_task=(struct swt_sys_task *)malloc(sizeof(struct swt_sys_task));
            else
                psSwt_sys_task=(struct swt_sys_task *)realloc(psSwt_sys_task,(ilCount+1)*sizeof(struct swt_sys_task));
            if(psSwt_sys_task==NULL)
            {
                swVdebug(0,"S5045:��̬�����ڴ�ʧ��!,%d,%s",errno,strerror(errno));
                return(-1);
            }  
            memcpy((char *)&(psSwt_sys_task[ilCount]), 
                    (char *)&(smShmbuf_s.psSwt_sys_task[i]), sizeof(struct swt_sys_task));
            ilCount ++;
        } 
    }
    *piCount = ilCount;

    k = 0;
    for (i=0; i < (ilCount - 1); i++)
    {
        k = 0;
        for (j=1; j < (ilCount - i); j++)
        {
            if (psSwt_sys_task[j - 1].start_id > psSwt_sys_task[j].start_id)
            {
                memcpy((char *)&slSwt_sys_task, (char *)&(psSwt_sys_task[j]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j]), (char *)&(psSwt_sys_task[j - 1]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j - 1]), (char *)&slSwt_sys_task, 
                        sizeof(struct swt_sys_task));
                k = 1;
            } 
        }
        if (k == 0) break;
    }
    *psSwt_sys_task1=psSwt_sys_task; /* add by nh 20020807 */
    return (SUCCESS); 
} 

/**************************************************************
 ** ������	: short swShmselect_swt_sys_task_all_p()
 ** ��  ��	: swt_sys_task������ȫ�� (�α����),��stop_id����
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_task_all_p(struct swt_sys_task **psSwt_sys_task1, 
        short *piCount)
{
    short i, j, k, ilCount;
    struct swt_sys_task slSwt_sys_task;
    static struct swt_sys_task *psSwt_sys_task=NULL;   /* add by nh 20020807 */

    if(psSwt_sys_task)
    {
        free(psSwt_sys_task);
        psSwt_sys_task = NULL;
    }
    ilCount = 0;

    for (i=0; i < psmShmidx_s->sIdx_sys_task.iCount; i++)
    {
        if(ilCount==0)
            psSwt_sys_task=(struct swt_sys_task *)malloc(sizeof(struct swt_sys_task));
        else
            psSwt_sys_task=(struct swt_sys_task *)realloc(psSwt_sys_task,(ilCount+1)*sizeof(struct swt_sys_task));
        if(psSwt_sys_task==NULL)
        {
            swVdebug(0,"S5050:��̬�����ڴ�ʧ��!,%d,%s",errno,strerror(errno));
            return(-1);
        }  
        memcpy((char *)&(psSwt_sys_task[ilCount]), 
                (char *)&(smShmbuf_s.psSwt_sys_task[i]), sizeof(struct swt_sys_task));
        ilCount ++;
    }
    *piCount = ilCount;

    k = 0;
    for (i=0; i < (ilCount - 1); i++)
    {
        k = 0;
        for (j=1; j < (ilCount - i); j++)
        {
            if (psSwt_sys_task[j - 1].stop_id > psSwt_sys_task[j].stop_id)
            {
                memcpy((char *)&slSwt_sys_task, (char *)&(psSwt_sys_task[j]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j]), (char *)&(psSwt_sys_task[j - 1]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j - 1]), (char *)&slSwt_sys_task, 
                        sizeof(struct swt_sys_task));
                k = 1;
            } 
        }
        if (k == 0) break;
    }  
    *psSwt_sys_task1=psSwt_sys_task; /* add by nh 20020807 */
    return (SUCCESS); 
} 

/**************************************************************
 ** ������	: short swShmselect_swt_sys_task_all_q()
 ** ��  ��	: swt_sys_task������ȫ�� (�α����),��q_id����
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_task_all_q(struct swt_sys_task **psSwt_sys_task1, 
        short *piCount)
{
    short i, j, k, ilCount;
    struct swt_sys_task slSwt_sys_task;
    static struct swt_sys_task *psSwt_sys_task=NULL;    /* add by nh 20020807 */

    if(psSwt_sys_task)
    {
        free(psSwt_sys_task);
        psSwt_sys_task = NULL;
    }
    ilCount = 0;

    for (i=0; i < psmShmidx_s->sIdx_sys_task.iCount; i++)
    {
        if(ilCount==0)
            psSwt_sys_task=(struct swt_sys_task *)malloc(sizeof(struct swt_sys_task));
        else
            psSwt_sys_task=(struct swt_sys_task *)realloc(psSwt_sys_task,(ilCount+1)*sizeof(struct swt_sys_task));
        if(psSwt_sys_task==NULL)
        {
            swVdebug(0,"S5055:��̬�����ڴ�ʧ��!,%d,%s",errno,strerror(errno));
            return(-1);
        }  
        memcpy((char *)&(psSwt_sys_task[ilCount]), 
                (char *)&(smShmbuf_s.psSwt_sys_task[i]), sizeof(struct swt_sys_task));
        ilCount ++;
    }
    *piCount = ilCount;

    k = 0;
    for (i=0; i < (ilCount - 1); i++)
    {
        k = 0;
        for (j=1; j < (ilCount - i); j++)
        {
            if (psSwt_sys_task[j - 1].q_id > psSwt_sys_task[j].q_id)
            {
                memcpy((char *)&slSwt_sys_task, (char *)&(psSwt_sys_task[j]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j]), (char *)&(psSwt_sys_task[j - 1]), 
                        sizeof(struct swt_sys_task));
                memcpy((char *)&(psSwt_sys_task[j - 1]), (char *)&slSwt_sys_task, 
                        sizeof(struct swt_sys_task));
                k = 1;
            } 
        }
        if (k == 0) break;
    }  
    *psSwt_sys_task1=psSwt_sys_task; /* add by nh 20020807 */
    return (SUCCESS); 
} 

/***************************************************************
 ** ������      : swShmselect_swt_sys_task_qid
 ** ��  ��      : swt_sys_task����ݹؼ��ֲ���
 ** ��  ��      : ������
 ** ��������    : 2001/09/11
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / 100:δ�ҵ���¼
 ***************************************************************/
int swShmselect_swt_sys_task_qid(short ilQid,short ilStatus,
        struct swt_sys_task * psSwt_sys_task)
{
    int i;
    for(i = 0;i < psmShmidx_s->sIdx_sys_task.iCount;i++)
    {
        if(( smShmbuf_s.psSwt_sys_task[i].q_id == ilQid )&& \
                ( smShmbuf_s.psSwt_sys_task[i].task_flag == ilStatus )&& \
                ( smShmbuf_s.psSwt_sys_task[i].task_use[0] == '1'))
        {
            memcpy(psSwt_sys_task,
                    &smShmbuf_s.psSwt_sys_task[i],
                    sizeof(struct swt_sys_task));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_task.iCount)
        return SUCCESS;
    else
        return SHMNOTFOUND;
}

/***************************************************************
 ** ������      : swShmselect_swt_sys_8583
 ** ��  ��      : swt_sys_8583����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_8583(short iTab_id,short iFld_id,
        struct swt_sys_8583 * psSwt_sys_8583)
{
    int i;
    for(i = 0;i < psmShmidx_s->sIdx_sys_8583.iCount;i++)
    {
        if((iTab_id == smShmbuf_s.psSwt_sys_8583[i].tab_id) &&
                (iFld_id == smShmbuf_s.psSwt_sys_8583[i].fld_id))
        {
            memcpy(psSwt_sys_8583,
                    &smShmbuf_s.psSwt_sys_8583[i],
                    sizeof(struct swt_sys_8583));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_8583.iCount)
        return SUCCESS;
    else
        return SHMNOTFOUND;
}

/***************************************************************
 ** ������      : swShmselect_swt_sys_8583E
 ** ��  ��      : swt_sys_8583E����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_8583E(short iTab_id,short iFld_id,
        struct swt_sys_8583E * psSwt_sys_8583E)
{
    int i;
    for(i = 0;i < psmShmidx_s->sIdx_sys_8583E.iCount;i++)
    {
        if((iTab_id == smShmbuf_s.psSwt_sys_8583E[i].tab_id) &&
                (iFld_id == smShmbuf_s.psSwt_sys_8583E[i].fld_id))
        {
            memcpy(psSwt_sys_8583E,
                    &smShmbuf_s.psSwt_sys_8583E[i],
                    sizeof(struct swt_sys_8583E));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_8583E.iCount)
        return SUCCESS;
    else
        return SHMNOTFOUND;
}

/***************************************************************
 ** ������      : swShmselect_swt_sys_code
 ** ��  ��      : swt_sys_code����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_code(long lMsg_code,
        struct swt_sys_code * psSwt_sys_code)
{
    int i;
    for(i = 0;i < psmShmidx_s->sIdx_sys_code.iCount;i++)
    {
        if(lMsg_code == smShmbuf_s.psSwt_sys_code[i].msg_code)
        {
            memcpy(psSwt_sys_code,
                    &smShmbuf_s.psSwt_sys_code[i],
                    sizeof(struct swt_sys_code));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_code.iCount)
        return SUCCESS;
    else
        return SHMNOTFOUND;
}

/***************************************************************
 ** ������      : swShmselect_swt_sys_config
 ** ��  ��      : ����[swt_sys_config]���¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_config(struct swt_sys_config *psSwt_sys_config)
{
    memcpy((char *)psSwt_sys_config,(char *)&smShmbuf_s.psSwt_sys_config[0],
            sizeof(struct swt_sys_config));
    return(SUCCESS);  
}    

/***************************************************************
 ** ������      : swShmselect_swt_sys_other
 ** ��  ��      : swt_sys_other����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/ 
int swShmselect_swt_sys_other(short iOther_id,
        struct swt_sys_other * psSwt_sys_other)
{
    int i;
    for(i = 0;i < psmShmidx_s->sIdx_sys_other.iCount;i++)
    {
        if(iOther_id == smShmbuf_s.psSwt_sys_other[i].other_id)
        {
            memcpy(psSwt_sys_other,
                    &smShmbuf_s.psSwt_sys_other[i],
                    sizeof(struct swt_sys_other));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_other.iCount)
        return SUCCESS;
    else
        return SHMNOTFOUND;
}

/***************************************************************
 ** ������      : swShmselect_swt_sys_other_grp_q
 ** ��  ��      : swt_sys_other����ݹؼ��ֲ���
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/ 
int swShmselect_swt_sys_other_grp_q(short iOther_grp, short iOther_q,
        struct swt_sys_other * psSwt_sys_other)
{
    int i;
    for(i = 0;i < psmShmidx_s->sIdx_sys_other.iCount;i++)
    {
        if((iOther_grp == smShmbuf_s.psSwt_sys_other[i].other_grp) && 
                (iOther_q == smShmbuf_s.psSwt_sys_other[i].other_q))
        {
            memcpy(psSwt_sys_other,
                    &smShmbuf_s.psSwt_sys_other[i],
                    sizeof(struct swt_sys_other));
            break;
        }
    }
    if(i < psmShmidx_s->sIdx_sys_other.iCount)
        return SUCCESS;
    else
        return SHMNOTFOUND;
}

#ifdef HASH
/*����������λ��ʽת�������������ڴ��еļ�¼*/
int shm_swt_sys_fmt_m(struct shm_sys_fmt_m *p_in, struct shm_sys_fmt_m **p_out)
{
    char idx_str[64];
    long record_offset = 0;
    long idx_offset = 0;
    long dataAreaOffset = 0;

    idx_offset =  sizeof(long) + sizeof(int);

    /*��������*/
    memset(idx_str, 0x00, sizeof(idx_str));

    snprintf(idx_str, sizeof(idx_str), "%d|%s|%d|%s", p_in->sSwt_sys_fmt_m.q_id,
            p_in->sSwt_sys_fmt_m.tran_code, p_in->sSwt_sys_fmt_m.tran_step,
            p_in->sSwt_sys_fmt_m.tran_type);

    /*ͨ������������λ��¼*/
    memcpy(&dataAreaOffset, smShmbuf_s.psShm_sys_fmt_m_hash, sizeof(long));
    if ((record_offset = idxSelect(smShmbuf_s.psShm_sys_fmt_m_hash + idx_offset, idx_str)) < 0)
    {
        memset(idx_str, 0x00, sizeof(idx_str));
        snprintf(idx_str, sizeof(idx_str), "%d|%s|0|%s", p_in->sSwt_sys_fmt_m.q_id,
                p_in->sSwt_sys_fmt_m.tran_code, p_in->sSwt_sys_fmt_m.tran_type);
        if ((record_offset = idxSelect(smShmbuf_s.psShm_sys_fmt_m_hash + idx_offset, idx_str)) < 0)
        {
            swVdebug(1, "shm_swt_sys_fmt_m idxSelect record_offset error: idx_str[%s], record_offset[%ld]",
                    idx_str, record_offset);
            *p_out = NULL;
            return SHMNOTFOUND;
        }
    }

    *p_out = (struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash + record_offset + dataAreaOffset);

    return 0;
}
#endif

#ifndef HASH
/**************************************************************
 ** ������      : swShmselect_swt_sys_fmt_m
 ** ��  ��      ����������š������롢���ײ��衢��Ϣ���ݱ�ʶ
 **               �õ�group�Լ�ƫ����
 **��λ��ʽת���飬 ����ƥ��(�����п�ģ��ƥ��)
 ** ��  ��      ��
 ** ��������    ��
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
 ***************************************************************/
int swShmselect_swt_sys_fmt_m(short iQ_id,char* aTrancode,short iTranstep,
        char* aMsgtranflag,struct swt_sys_fmt_grp *psSwt_sys_fmt_grp,
        long *lFmt_offset)
{
    char alTrancodetmp[11];
    long i;
    /*del by zcd 2014122
      int  ilFmt_group;
     ***end of del by zcd 20141222*/
    /*add by zcd 20141222*/
    long  ilFmt_group;
    /*end of add by zcd 20141222*/
    _WITH_ITME_START;
    memset(alTrancodetmp, 0x00, sizeof(alTrancodetmp));
    memcpy(alTrancodetmp, aTrancode, sizeof(alTrancodetmp) - 1);


    for(i=0; i <= psmShmidx_s->sIdx_sys_fmt_m.iCount; i++)
    {
        if (((smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.q_id == 0) ||
                    (smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.q_id == iQ_id)) &&
                ((!strcmp(smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_code, 
                          alTrancodetmp)) ||
                 (!strcmp(smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_code, 
                          "0"))) &&
                ((smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_step == 0) ||
                 (smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_step == 
                  iTranstep)) &&
                ((!strcmp(smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_type, 
                          "0")) ||
                 (smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_type[0] == 
                  aMsgtranflag[0])))
        {
            ilFmt_group = smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.fmt_group;
            *lFmt_offset = smShmbuf_s.psShm_sys_fmt_m[i].lOffset;

            if ((*lFmt_offset >= psmShmidx_s->sIdx_sys_fmt_g.iCount) || 
                    (ilFmt_group != 
                     smShmbuf_s.psShm_sys_fmt_g[*lFmt_offset].sSwt_sys_fmt_g.fmt_group))
                return (FAIL);

            memcpy((char *)psSwt_sys_fmt_grp, 
                    (char *)&(smShmbuf_s.psShm_sys_fmt_g[*lFmt_offset].sSwt_sys_fmt_g), 
                    sizeof(struct swt_sys_fmt_grp));

            memcpy(agMac_check, 
                    smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.mac_check, 
                    sizeof(agMac_check));
            memcpy(agMac_create, 
                    smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.mac_create, 
                    sizeof(agMac_create));
            _WITH_ITME_END;
            return(SUCCESS);
        }
    }
    return(SHMNOTFOUND);
}
#else
/**************************************************************
 ** ������      : swShmselect_swt_sys_fmt_m
 ** ��  ��      ����������š������롢���ײ��衢��Ϣ���ݱ�ʶ
 **               �õ�group�Լ�ƫ����
 **��λ��ʽת���飬 ����ƥ��(�����п�ģ��ƥ��)
 ** ��  ��      �� gengling
 ** ��������    ��2015.05.12
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
 ***************************************************************/
int swShmselect_swt_sys_fmt_m(short iQ_id,char* aTrancode,short iTranstep,
        char* aMsgtranflag,struct swt_sys_fmt_grp *psSwt_sys_fmt_grp,
        long *lFmt_offset)
{
    int iRc = 0;
    struct shm_sys_fmt_m p_in;
    struct shm_sys_fmt_m *p_out;

    long i; 
    long  ilFmt_group;
    _WITH_ITME_START;

    p_in.sSwt_sys_fmt_m.q_id = iQ_id;

    strcpy(p_in.sSwt_sys_fmt_m.tran_code, aTrancode);
    p_in.sSwt_sys_fmt_m.tran_step = iTranstep;
    strcpy(p_in.sSwt_sys_fmt_m.tran_type, aMsgtranflag);

    iRc = shm_swt_sys_fmt_m(&p_in, &p_out);
    if(iRc)
    {
        swVdebug(1, "shm_swt_sys_fmt_m error iRc[%d]", iRc);
        return(FAIL);
    }

    swVdebug(5, "swShmselect_swt_sys_fmt_m q_id[%d],tran_code[%s],tran_step[%d],tran_type[%s]",
            p_out->sSwt_sys_fmt_m.q_id, p_out->sSwt_sys_fmt_m.tran_code,
            p_out->sSwt_sys_fmt_m.tran_step, p_out->sSwt_sys_fmt_m.tran_type);

    ilFmt_group = p_out->sSwt_sys_fmt_m.fmt_group;
    *lFmt_offset = p_out->lOffset;

    long dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_g_hash, sizeof(long));

    if ((*lFmt_offset >= psmShmidx_s->sIdx_sys_fmt_g_hash.iCount) || 
            (ilFmt_group != 
        ((struct shm_sys_fmt_g *)(((void *)(smShmbuf_s.psShm_sys_fmt_g_hash))+dataoffset))[*lFmt_offset].sSwt_sys_fmt_g.fmt_group))
        return (FAIL);

    memcpy((char *)psSwt_sys_fmt_grp, 
            (char *)&(((struct shm_sys_fmt_g *)(((void *)(smShmbuf_s.psShm_sys_fmt_g_hash))+dataoffset))[*lFmt_offset].sSwt_sys_fmt_g), 
            sizeof(struct swt_sys_fmt_grp));

    memcpy(agMac_check, 
            p_out->sSwt_sys_fmt_m.mac_check,
            sizeof(agMac_check));
    memcpy(agMac_create, 
            p_out->sSwt_sys_fmt_m.mac_create,
            sizeof(agMac_create));

    _WITH_ITME_END;
    return(0);
}
#endif

#ifdef HASH
/*����������λ��ʽת��������ڴ��еļ�¼*/
int shm_swt_sys_fmt_g(struct shm_sys_fmt_g *p_in, struct shm_sys_fmt_g **p_out)
{
    char idx_str[64];
    long record_offset = 0;
    long idx_offset = 0;
    long dataAreaOffset = 0;

    idx_offset =  sizeof(long) + sizeof(int);

    /*��������*/
    memset(idx_str, 0x00, sizeof(idx_str));

    snprintf(idx_str, sizeof(idx_str), "%ld", p_in->sSwt_sys_fmt_g.fmt_group);

    /*ͨ������������λ��¼*/
    memcpy(&dataAreaOffset, smShmbuf_s.psShm_sys_fmt_g_hash, sizeof(long));
    if ((record_offset = idxSelect(smShmbuf_s.psShm_sys_fmt_g_hash + idx_offset, idx_str)) < 0)
    {
        swVdebug(1, "shm_swt_sys_fmt_g idxSelect record_offset error: idx_str[%s], record_offset[%ld]",
                idx_str, record_offset);
        *p_out = NULL;
        return SHMNOTFOUND;
    }

    *p_out = (struct shm_sys_fmt_g *)(smShmbuf_s.psShm_sys_fmt_g_hash + record_offset + dataAreaOffset);

    return record_offset;
}
#endif

#ifndef HASH
/**************************************************************
 ** ������	: short swShmselect_swt_sys_fmt_grp()
 ** ��  ��	: swt_sys_fmt_grp���ؼ��ֲ��ҵõ�group�Լ�ƫ����
 ** ��λ��ʽת���飬����ָ����ʽת����ķ�ʽ
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
/*del by zcd 20141222
  short swShmselect_swt_sys_fmt_grp(int iFmt_group, 
  struct swt_sys_fmt_grp *psSwt_sys_fmt_grp, long *lFmt_offset)
 ***end of del by zcd 20141222*/
/*add by zcd 20141222*/
short swShmselect_swt_sys_fmt_grp(long iFmt_group, 
        struct swt_sys_fmt_grp *psSwt_sys_fmt_grp, long *lFmt_offset)
/*end of add by zcd 20141222*/
{
    long i;
    _WITH_ITME_START;

    for (i=0; i < psmShmidx_s->sIdx_sys_fmt_g.iCount; i++)
    {
        if (iFmt_group == 
                smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.fmt_group)
        {
            memcpy((char *)psSwt_sys_fmt_grp, 
                    (char *)&(smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g), 
                    sizeof(struct swt_sys_fmt_grp));
            *lFmt_offset = i; /* ��¼�˸�ʽת�����ڹ����ڴ��е�ƫ���� */
            _WITH_ITME_END;
            return (SUCCESS);
        }    
    }  

    return (SHMNOTFOUND);
} 
#else
/**************************************************************
 ** ������	: short swShmselect_swt_sys_fmt_grp()
 ** ��  ��	: swt_sys_fmt_grp���ؼ��ֲ��ҵõ�group�Լ�ƫ����
 ** ��λ��ʽת���飬����ָ����ʽת����ķ�ʽ
 ** ��  ��	: gengling
 ** ��������	: 2015.05.15
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_fmt_grp(long iFmt_group, 
        struct swt_sys_fmt_grp *psSwt_sys_fmt_grp, long *lFmt_offset)
{
    long iRc = 0;
    struct shm_sys_fmt_g p_in;
    struct shm_sys_fmt_g *p_out;
    _WITH_ITME_START;

    p_in.sSwt_sys_fmt_g.fmt_group = iFmt_group;

    iRc = shm_swt_sys_fmt_g(&p_in, &p_out);
    if(iRc == SHMNOTFOUND)
    {
        swVdebug(1, "shm_swt_sys_fmt_g error iRc[%d]", iRc);
        return (SHMNOTFOUND);
    }

    swVdebug(5, "swShmselect_swt_sys_fmt_grp iFmt_group[%ld]", p_out->sSwt_sys_fmt_g.fmt_group);

    if ((iFmt_group == p_out->sSwt_sys_fmt_g.fmt_group))
    {
        memcpy((char *)psSwt_sys_fmt_grp, (char *)&(p_out->sSwt_sys_fmt_g), sizeof(struct swt_sys_fmt_grp));
        *lFmt_offset = iRc/sizeof(struct shm_sys_fmt_g);
    }
    _WITH_ITME_END;
    return(0);
}
#endif

/**************************************************************
 ** ������	: short swShmselect_swt_sys_fmt_grp_offset()
 ** ��  ��	: swt_sys_fmt_grp��ƫ�������ҵõ�TDFƫ��������¼��
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, psSwt_sys_fmt_grp, 
        iFmt_d_count, lFmt_d_offset)
long   lFmt_offset;
struct swt_sys_fmt_grp *psSwt_sys_fmt_grp;
int  *iFmt_d_count; /*modified by baiqj20150505 PSBC_V1.0  short -> int*/
long   *lFmt_d_offset;
{
#ifndef HASH
    if ((lFmt_offset < 0) || 
            (lFmt_offset >= psmShmidx_s->sIdx_sys_fmt_g.iCount))
        return (FAIL);

    memcpy((char *)psSwt_sys_fmt_grp, 
            (char *)&(smShmbuf_s.psShm_sys_fmt_g[lFmt_offset].sSwt_sys_fmt_g), 
            sizeof(struct swt_sys_fmt_grp));

    /* ��Ӧ�� TDF �������� */  
    *iFmt_d_count = smShmbuf_s.psShm_sys_fmt_g[lFmt_offset].iCount;  

    /* ��¼�˸�ʽת�����һ�� TDF ����ڹ����ڴ��е�ƫ���� */
    *lFmt_d_offset = smShmbuf_s.psShm_sys_fmt_g[lFmt_offset].lOffset;
#else
    if ((lFmt_offset < 0) || 
            (lFmt_offset >= psmShmidx_s->sIdx_sys_fmt_g_hash.iCount))
        return (FAIL);

    long dataoffset = 0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_g_hash, sizeof(long));
    swVdebug(5, "dataoffset [%ld]", dataoffset);
    memcpy((char *)psSwt_sys_fmt_grp, 
            (char *)&(((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[lFmt_offset].sSwt_sys_fmt_g), 
            sizeof(struct swt_sys_fmt_grp));

    /* ��Ӧ�� TDF �������� */  
    //*iFmt_d_count = ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[lFmt_offset].iCount;  
    *iFmt_d_count = smShmbuf_s.psShm_sys_fmt_g[lFmt_offset].iCount;  

    /* ��¼�˸�ʽת�����һ�� TDF ����ڹ����ڴ��е�ƫ���� */
    //*lFmt_d_offset = ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[lFmt_offset].lOffset;
    *lFmt_d_offset = smShmbuf_s.psShm_sys_fmt_g[lFmt_offset].lOffset;
#endif

    if ((*iFmt_d_count < 0) || (*lFmt_d_offset < 0) || 
            (*lFmt_d_offset >= psmShmidx_s->sIdx_sys_fmt_d.iCount))
        return (SHMNOTFOUND);

    return (SUCCESS);

} 

/**************************************************************
 ** ������	: short swShmselect_swt_sys_fmt_d_offset()
 ** ��  ��	: swt_sys_fmt_d��ƫ�������ҵõ�TDF��Ϣ
 ** ��  ��	:
 ** ��������	:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���	:
 ** ��������	:
 ** ����ֵ	: SUCCESS , FAIL
 ***************************************************************/
short swShmselect_swt_sys_fmt_d_offset(long lFmt_d_offset, 
        struct swt_sys_fmt_d *psSwt_sys_fmt_d)
{
    if ((lFmt_d_offset < 0) || 
            (lFmt_d_offset >= psmShmidx_s->sIdx_sys_fmt_d.iCount))
        return (SHMNOTFOUND);

    /* ��Ӧ�� TDF ��� */    
    memcpy((char *)psSwt_sys_fmt_d, 
            (char *)&(smShmbuf_s.psSwt_sys_fmt_d[lFmt_d_offset]), 
            sizeof(struct swt_sys_fmt_d));

    return (SUCCESS);

} 

#ifdef HASH
/*����������λ·���������ڴ��еļ�¼*/
int shm_swt_sys_route_m(struct shm_sys_route_m *p_in, struct shm_sys_route_m **p_out)
{
    char idx_str[64];
    long record_offset = 0;
    long idx_offset = 0;
    long dataAreaOffset = 0;

    idx_offset =  sizeof(long) + sizeof(int);

    /*��������*/
    memset(idx_str, 0x00, sizeof(idx_str));

    snprintf(idx_str, sizeof(idx_str), "%d|%s", p_in->sSwt_sys_route_m.q_id,
            p_in->sSwt_sys_route_m.trancode);

    /*ͨ������������λ��¼*/
    memcpy(&dataAreaOffset, smShmbuf_s.psShm_sys_route_m_hash, sizeof(long));
    if ((record_offset = idxSelect(smShmbuf_s.psShm_sys_route_m_hash + idx_offset, idx_str)) < 0)
    {
        swVdebug(1, "shm_swt_sys_router_m idxSelect record_offset error: idx_str[%s], record_offset[%ld]",
                idx_str, record_offset);
        *p_out = NULL;
         return SHMNOTFOUND;
    }

    *p_out = (struct shm_sys_route_m *)(smShmbuf_s.psShm_sys_route_m_hash + record_offset + dataAreaOffset);

    return 0;
}
#endif

#ifndef HASH
/***************************************************************
 ** ������      : swShmselect_swt_sys_route_m
 ** ��  ��      : ��������źͽ�����ƥ�����
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_route_m(int iBegin_q_id,char *aTrancode,
        struct swt_sys_route_m  * pslSwt_sys_route_m)
{
    long i,llOffset;
    int  ilGroupid;
    _WITH_ITME_START;
    /*  �Ⱦ�ȷ����  */  
    for (i = 0;i < psmShmidx_s->sIdx_sys_route_m.iCount;i++)
    {
        if ((strcmp(smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.trancode,
                        aTrancode) == 0)  && 
                (smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.q_id==iBegin_q_id)) 
        {
            llOffset = smShmbuf_s.psShm_sys_route_m[i].lOffset;
            ilGroupid = smShmbuf_s.psShm_sys_route_g[llOffset].sSwt_sys_route_g.
                route_grp;
            if (llOffset > psmShmidx_s->sIdx_sys_route_g.iCount ||
                    smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.route_grp
                    != ilGroupid)
            {
                return(FAIL); 
            }

            memcpy((char *)pslSwt_sys_route_m,
                    (char *)(&(smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m)),
                    sizeof(struct swt_sys_route_m));
            _WITH_ITME_END;
            return(SUCCESS);
        }
    }
    /* ģ��ƥ�� */
    for (i = 0;i < psmShmidx_s->sIdx_sys_route_m.iCount;i++)
    {
        if (strcmp(smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.trancode,
                    "0") == 0  &&
                smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.q_id==iBegin_q_id)
        {
            llOffset = smShmbuf_s.psShm_sys_route_m[i].lOffset;
            ilGroupid = smShmbuf_s.psShm_sys_route_g[llOffset].sSwt_sys_route_g.
                route_grp;
            if (llOffset > psmShmidx_s->sIdx_sys_route_g.iCount ||
                    smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.route_grp
                    != ilGroupid)
            {
                return(FAIL);
            }

            memcpy((char *)pslSwt_sys_route_m,
                    (char *)(&(smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m)),
                    sizeof(struct swt_sys_route_m));
            _WITH_ITME_END;
            return(SUCCESS);
        }
    }
    return(SHMNOTFOUND);
}
#else
/***************************************************************
 ** ������      : swShmselect_swt_sys_route_m
 ** ��  ��      : ��������źͽ�����ƥ�����,ʹ��hash
 ** ��  ��      : baiqj
 ** ��������    : 2015/05/15
 ** ����޸�����: 2015/05/15
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_route_m(int iBegin_q_id,char *aTrancode,
        struct swt_sys_route_m  * pslSwt_sys_route_m)
{
   
    int iRc = 0;
    struct shm_sys_route_m p_in;
    struct shm_sys_route_m *p_out;
    
    
    long llOffset;
    int  ilGroupid;
    _WITH_ITME_START;
    /*  �Ⱦ�ȷ����  */  
    p_in.sSwt_sys_route_m.q_id = iBegin_q_id;
    strcpy(p_in.sSwt_sys_route_m.trancode,aTrancode);
    
    iRc = shm_swt_sys_route_m(&p_in, &p_out);
    if(0 == iRc)
    {
      swVdebug(5, "swShmselect_swt_sys_route_m q_id[%d],trancode[%s]",p_out->sSwt_sys_route_m.q_id,p_out->sSwt_sys_route_m.trancode);
      llOffset = p_out->lOffset;
      ilGroupid =  smShmbuf_s.psShm_sys_route_g[llOffset].sSwt_sys_route_g.route_grp;
      if (llOffset > psmShmidx_s->sIdx_sys_route_g.iCount ||p_out->sSwt_sys_route_m.route_grp != ilGroupid)
      {
         return(FAIL); 
      }

      memcpy((char *)pslSwt_sys_route_m,
            (char *)(&(p_out->sSwt_sys_route_m)),
            sizeof(struct swt_sys_route_m));
   
      _WITH_ITME_END;
      return(SUCCESS);
    }
    
    /* ģ��ƥ�� */
    memset(&p_in.sSwt_sys_route_m.trancode,0x00,sizeof(p_in.sSwt_sys_route_m.trancode));  
    strcpy(p_in.sSwt_sys_route_m.trancode,"0");
    iRc = shm_swt_sys_route_m(&p_in, &p_out);
    if(0 == iRc)
    {
        llOffset = p_out->lOffset;
        ilGroupid = smShmbuf_s.psShm_sys_route_g[llOffset].sSwt_sys_route_g.route_grp;
        if (llOffset > psmShmidx_s->sIdx_sys_route_g.iCount ||
                p_out->sSwt_sys_route_m.route_grp
                != ilGroupid)
        {
            return(FAIL);
        }

        memcpy((char *)pslSwt_sys_route_m,
                (char *)(&(p_out->sSwt_sys_route_m)),
                sizeof(struct swt_sys_route_m));
        _WITH_ITME_END;
        return(SUCCESS);
    }
    
    return(SHMNOTFOUND);
}
#endif

/***************************************************************
 ** ������      : swShmselect_swt_sys_route_d
 ** ��  ��      : ������Ų�ѯ·����ϸ��
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_sys_route_d(int iGroupid,
        struct swt_sys_route_d *pslSwt_sys_route_d,short *piCount)
{
    long i;
    short  ilCount = 0;
    for (i = 0;i < psmShmidx_s->sIdx_sys_route_d.iCount;i++) 
    {
        if (smShmbuf_s.psSwt_sys_route_d[i].route_grp == iGroupid)
        {
            ilCount ++;
            memcpy((char *)&pslSwt_sys_route_d[ilCount - 1],
                    (char *)&smShmbuf_s.psSwt_sys_route_d[i],
                    sizeof(struct swt_sys_route_d));
        }
    } 

    if (ilCount > 0)
    {
        *piCount = ilCount;
        return(SUCCESS);
    }
    else
        return(SHMNOTFOUND);
}

#ifndef HASH
/***************************************************************
 ** ������      : swShmselect_route_d_q_tran
 ** ��  ��      : ���Ҷ�Ӧ��·�ɱ��е�һ�����
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmselect_route_d_q_tran(short iQid, char *aTran_code, 
        struct swt_sys_route_d *psSwt_sys_route_d)
{
    int i;
    int il_count = 0;
    long ll_offset = -1;
    struct shm_sys_route_m sShm_sys_route_m; /* add by nh 20040412 */
    _WITH_ITME_START;

    for(i = 0;i < psmShmidx_s->sIdx_sys_route_m.iCount;i++)
    {
        if ((iQid == smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.q_id) &&
                (strcmp(aTran_code, 
                        smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.trancode) == 0))
        {
            /* modify by nh 20040412
               ll_offset = smShmbuf_s.psShm_sys_route_m[i].lOffset;*/
            memcpy(&sShm_sys_route_m,&smShmbuf_s.psShm_sys_route_m[i],sizeof(struct shm_sys_route_m));
            ll_offset = sShm_sys_route_m.lOffset;
            _WITH_ITME_END;
            break;
        }
    }
    if (ll_offset == -1)
        for(i = 0;i < psmShmidx_s->sIdx_sys_route_m.iCount;i++)
        {
            if ((iQid == smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.q_id) &&
                    (strcmp("0", 
                            smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.trancode) == 0))
            {
                /* modify by nh 20040412
                   ll_offset = smShmbuf_s.psShm_sys_route_m[i].lOffset;*/
                memcpy(&sShm_sys_route_m,&smShmbuf_s.psShm_sys_route_m[i],sizeof(struct shm_sys_route_m));
                ll_offset = sShm_sys_route_m.lOffset;
                _WITH_ITME_END;
                break;
            }
        }  

    if ((ll_offset < 0) || (ll_offset >= psmShmidx_s->sIdx_sys_route_g.iCount))
        return SHMNOTFOUND;

    il_count = smShmbuf_s.psShm_sys_route_g[ll_offset].iCount;
    ll_offset = smShmbuf_s.psShm_sys_route_g[ll_offset].lOffset;

    if ((ll_offset < 0) || (ll_offset + il_count > psmShmidx_s->sIdx_sys_route_d.iCount))
        return SHMNOTFOUND;  

    for(i=0;i<il_count;i++)
    {
        if(smShmbuf_s.psSwt_sys_route_d[ll_offset + i].route_id == 0)
        {
            memcpy(psSwt_sys_route_d,&smShmbuf_s.psSwt_sys_route_d[ll_offset+i],sizeof(struct swt_sys_route_d));
            return SUCCESS;
        }
    }    

    memset(psSwt_sys_route_d,0x00,sizeof(struct swt_sys_route_d));
    return SUCCESS;
}
#else
/***************************************************************
 ** ������      : swShmselect_route_d_q_tran(HASH)
 ** ��  ��      : ���Ҷ�Ӧ��·�ɱ��е�һ�����
 ** ��  ��      : baiqj
 ** ��������    : 2015/05/15
 ** ����޸�����: 2015/05/15
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmselect_route_d_q_tran(short iQid, char *aTran_code, 
        struct swt_sys_route_d *psSwt_sys_route_d)
{
    int iRc = 0, i;
    int il_count = 0;
    long ll_offset = -1;

    struct shm_sys_route_m p_in;
    struct shm_sys_route_m *p_out;
    _WITH_ITME_START;

    p_in.sSwt_sys_route_m.q_id = iQid;
    strcpy(p_in.sSwt_sys_route_m.trancode,aTran_code);

    iRc = shm_swt_sys_route_m(&p_in, &p_out);
    if(0 == iRc)
    {
        ll_offset = p_out->lOffset;
        _WITH_ITME_END;
    }

    if (ll_offset == -1)
    {
        memset(&p_in.sSwt_sys_route_m.trancode,0x00,sizeof(p_in.sSwt_sys_route_m.trancode));
        strcpy(p_in.sSwt_sys_route_m.trancode,"0");
        iRc = shm_swt_sys_route_m(&p_in, &p_out);
        if(0 == iRc)
        {
            ll_offset = p_out->lOffset;
            _WITH_ITME_END;
        }
    }

    if ((ll_offset < 0) || (ll_offset >= psmShmidx_s->sIdx_sys_route_g.iCount))
        return SHMNOTFOUND;

    il_count = smShmbuf_s.psShm_sys_route_g[ll_offset].iCount;
    ll_offset = smShmbuf_s.psShm_sys_route_g[ll_offset].lOffset;

    if ((ll_offset < 0) || (ll_offset + il_count > psmShmidx_s->sIdx_sys_route_d.iCount))
        return SHMNOTFOUND;  

    for(i=0;i<il_count;i++)
    {
        if(smShmbuf_s.psSwt_sys_route_d[ll_offset + i].route_id == 0)
        {
            memcpy(psSwt_sys_route_d,&smShmbuf_s.psSwt_sys_route_d[ll_offset+i],sizeof(struct swt_sys_route_d));
            return SUCCESS;
        }
    }    

    memset(psSwt_sys_route_d,0x00,sizeof(struct swt_sys_route_d));
    return SUCCESS;
}
#endif

/***************************************************************
 ** ������      : swShmsettranid
 ** ��  ��      : ���� Tran_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmsettranid(long lTran_id)
{

    if (swShmP(iSEMNUM_TRANID) == FAIL)
        return(FAIL);  

    psmShmidx_d->lTranid = lTran_id;

    if (swShmV(iSEMNUM_TRANID) == FAIL)
        return(FAIL);  

    return(SUCCESS);
}

/***************************************************************
 ** ������      : swShmgettranid
 ** ��  ��      : ��ȡ������ Tran_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmgettranid(long *lTran_id)
{
    int groupid;
    long tranid_tmp; 
    char lStr[13];
    groupid=0;
    /*��ȡ��������ID*/

    /*
       if (swShmP(iSEMNUM_TRANID) == FAIL)
       return(FAIL);  
     */
    /*modify by pc �����ȡ�³�Q��� 20150317 */
    groupid=atoi(getenv("BMQ_GROUP_ID"));
    /*modify by pc �����ȡ�³�Q��� 20150317 */


    if (swShmP(iSEMNUM_TRANID) == FAIL)
        return(FAIL);  

    if (psmShmidx_d->lTranid >= 9999999999L)
        psmShmidx_d->lTranid = 0;

    tranid_tmp = psmShmidx_d->lTranid;   /*add by dyw, 2015.0121*/
    psmShmidx_d->lTranid = psmShmidx_d->lTranid + 1;
    /*end of add by cjh 20141220 */
    if (swShmV(iSEMNUM_TRANID) == FAIL)
        return(FAIL);  
    sprintf(lStr,"%02ld%010ld",groupid,tranid_tmp);
    *lTran_id = atol(lStr);
    return(SUCCESS);
}

/*======================= swt_proc_log =========================*/

/***************************************************************
 ** ������      : swShminsert_swt_proc_log
 ** ��  ��      : ���봦����ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShminsert_swt_proc_log(struct swt_proc_log sSwt_proc_log,char cLoc_flag)
{
    int i,ilRc,ilElink;
    long llCurrentTime;

    /*���봦����ˮʱ����*/
    time(&llCurrentTime);
    sSwt_proc_log.proc_begin = llCurrentTime;

    if(cLoc_flag == '0')
    {
        if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

        i = psmShmidx_d->sIdx_proc_log.iFlink;
        for (;;)
        {
            if (smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.cTag != 1)  break;
            i++;
            if (i > smSwt_sys_config.iMaxproclog) i = 1;
            if (i == psmShmidx_d->sIdx_proc_log.iFlink) 
            {
                if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

#ifdef DB_SUPPORT  /*support database 2014-12*/
                ilRc = swDbinsert_swt_proc_log(sSwt_proc_log);
                if (ilRc == FAIL)
                    return(FAIL);
                else
                    return(SHMFULL);
#else
                return SHMFULL;
#endif
            }
        }

        memcpy((char *)(&(smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log)),
                (char *)&sSwt_proc_log,sizeof(struct swt_proc_log));

        /* ͳ���ڹ����ڴ浱�еĴ�����ˮ�� */
        psmShmidx_d->iRecinshm[SFPROCLOGID] ++;

        smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.cTag = 1;
        smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink = 0;
        ilElink = psmShmidx_d->sIdx_proc_log.iElink;
        smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iLlink = ilElink;
        if (ilElink > 0)
            smShmbuf_d.psShm_proc_log[ilElink-1].sLinkinfo.iRlink = i;

        if (psmShmidx_d->sIdx_proc_log.iSlink == 0)
            psmShmidx_d->sIdx_proc_log.iSlink = i;

        psmShmidx_d->sIdx_proc_log.iElink = i;

        if (i < smSwt_sys_config.iMaxproclog)
            psmShmidx_d->sIdx_proc_log.iFlink = i+1;
        else
            psmShmidx_d->sIdx_proc_log.iFlink = 1;  

        if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    }
    else if( cLoc_flag == '1' )
    {
#ifdef DB_SUPPORT  /*support database 2014-12*/
        ilRc = swDbinsert_swt_proc_log(sSwt_proc_log);
        if (ilRc == FAIL) return(FAIL);
#else
        return FAIL;
#endif
    }

    return(SUCCESS);
}

/***************************************************************
 ** ������      : swShmdelete_swt_proc_log
 ** ��  ��      : ɾ��������ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmdelete_swt_proc_log(long lTran_id,short iProc_step,char cLoc_flag)
{
    int i,ilRc,ilLlink,ilRlink;

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

        i = psmShmidx_d->sIdx_proc_log.iSlink;
        while(i)
        {
            if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTran_id && 
                    smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.proc_step == iProc_step)
            {
                /* ͳ���ڹ����ڴ浱�еĴ�����ˮ�� */
                psmShmidx_d->iRecinshm[SFPROCLOGID] --;

                ilLlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iLlink;
                ilRlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;

                if ((psmShmidx_d->sIdx_proc_log.iSlink == i) && 
                        (psmShmidx_d->sIdx_proc_log.iElink == i))
                {
                    psmShmidx_d->sIdx_proc_log.iSlink = 0;
                    psmShmidx_d->sIdx_proc_log.iElink = 0;
                }
                else
                {
                    if (psmShmidx_d->sIdx_proc_log.iSlink == i)
                    {
                        if (ilRlink > 0)
                            smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = 0; 
                        psmShmidx_d->sIdx_proc_log.iSlink = ilRlink;
                    }
                    else
                        if (psmShmidx_d->sIdx_proc_log.iElink == i)
                        {
                            if (ilLlink > 0)
                                smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = 0;
                            psmShmidx_d->sIdx_proc_log.iElink = ilLlink;
                        }
                        else
                        {
                            smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = ilRlink;
                            smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = ilLlink;
                        }
                }
                psmShmidx_d->sIdx_proc_log.iFlink = i;
                smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.cTag = 0;

                if(swShmV(iSEMNUM_PROCLOG) == FAIL)
                    return(FAIL);
                else
                    return(SUCCESS);
            }
            i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        ilRc = swDbdelete_swt_proc_log(lTran_id,iProc_step);
        return(ilRc);
    }
#endif
    return(SHMNOTFOUND);
} 

/***************************************************************
 ** ������      : swShmdelete_swt_proc_log_mrec
 ** ��  ��      : ɾ����ˮ�Ŷ�Ӧ�����д�����ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmdelete_swt_proc_log_mrec(long lTran_id,char cLoc_flag)
{
    int i,ilLlink,ilRlink,ilRc; 
    int ilTmp=0;

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);      

        i = psmShmidx_d->sIdx_proc_log.iSlink;
        while (i)
        {
            if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTran_id)
            {
                /* ͳ���ڹ����ڴ浱�еĴ�����ˮ�� */
                psmShmidx_d->iRecinshm[SFPROCLOGID] --;        

                if(ilTmp == 0) ilTmp = 1;
                ilLlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iLlink;
                ilRlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
                if ((psmShmidx_d->sIdx_proc_log.iSlink == i) && 
                        (psmShmidx_d->sIdx_proc_log.iElink == i))
                {
                    psmShmidx_d->sIdx_proc_log.iSlink = 0;
                    psmShmidx_d->sIdx_proc_log.iElink = 0;
                }
                else
                {      
                    if (psmShmidx_d->sIdx_proc_log.iSlink == i)
                    {
                        if (ilRlink > 0)
                            smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = 0;
                        psmShmidx_d->sIdx_proc_log.iSlink = ilRlink;
                    }
                    else
                        if (psmShmidx_d->sIdx_proc_log.iElink == i)
                        {
                            if (ilLlink > 0)
                                smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = 0;
                            psmShmidx_d->sIdx_proc_log.iElink = ilLlink;
                        }
                        else
                        {
                            smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = ilRlink;
                            smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = ilLlink;
                        }
                }
                smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.cTag = 0;
                psmShmidx_d->sIdx_proc_log.iFlink = i;

            }
            i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        ilRc = swDbdelete_swt_proc_log_mrec(lTran_id);
        return(ilRc);
    }
#endif
    if(ilTmp == 1)
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmdelete_proclog_tran_q
 ** ��  ��      : ɾ��ָ������ָ����������Ӧ�����д�����ˮ
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmdelete_proclog_tran_q(long lTran_id,short iQid,char cLoc_flag)
{
    int i,ilLlink,ilRlink,ilRc; 
    int ilTmp = 0;

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

        i = psmShmidx_d->sIdx_proc_log.iSlink;
        while (i)
        {
            if((smShmbuf_d.psShm_proc_log[i - 1].sSwt_proc_log.tran_id == lTran_id) &&
                    (smShmbuf_d.psShm_proc_log[i - 1].sSwt_proc_log.q_target == iQid))
            {
                /* ͳ���ڹ����ڴ浱�еĴ�����ˮ�� */
                psmShmidx_d->iRecinshm[SFPROCLOGID] --;

                if(ilTmp == 0) ilTmp = 1;
                swVdebug(0,"S5080: ilTmp=[%d]",ilTmp);
                ilLlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iLlink;
                ilRlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
                if ((psmShmidx_d->sIdx_proc_log.iSlink == i) && 
                        (psmShmidx_d->sIdx_proc_log.iElink == i))
                {
                    psmShmidx_d->sIdx_proc_log.iSlink = 0;
                    psmShmidx_d->sIdx_proc_log.iElink = 0;
                }
                else
                {            
                    if (psmShmidx_d->sIdx_proc_log.iSlink == i)
                    {
                        if (ilRlink > 0)
                            smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = 0;
                        psmShmidx_d->sIdx_proc_log.iSlink = ilRlink;
                    }
                    else
                        if (psmShmidx_d->sIdx_proc_log.iElink == i)
                        {
                            if (ilLlink > 0)
                                smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = ilRlink;
                            psmShmidx_d->sIdx_proc_log.iElink = ilLlink;
                        }
                        else
                        {
                            smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = ilRlink;
                            smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = ilLlink;
                        }
                }
                smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.cTag = 0;
                psmShmidx_d->sIdx_proc_log.iFlink = i;

            }
            i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
            swVdebug(0,"aaaaaa i=[%d]",i);
        }

        if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        ilRc = swDbdelete_proc_tran_q(lTran_id, iQid);
        return(ilRc);
    }
#endif
    if(ilTmp == 1)
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmupdate_swt_proc_log
 ** ��  ��      : ���´�����ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmupdate_swt_proc_log(long lTran_id,short iProc_step,
        struct swt_proc_log sSwt_proc_log,char cLoc_flag)
{
    int i,ilRc;

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

        i = psmShmidx_d->sIdx_proc_log.iSlink;
        while (i)
        {
            if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTran_id && 
                    smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.proc_step == iProc_step)
            {
                memcpy((char *)(&(smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log)),
                        (char *)&sSwt_proc_log,sizeof(struct swt_proc_log));

                if(swShmV(iSEMNUM_PROCLOG) == FAIL)
                {
                    return(FAIL);
                }
                else
                {
                    return(SUCCESS);    
                }
            }
            i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        ilRc = swDbupdate_swt_proc_log(lTran_id, iProc_step,sSwt_proc_log);
        return(ilRc);
    }
#endif
    return(SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmupdate_setstat_proclog 
 ** ��  ��      �����´�����ˮ״̬
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmupdate_setstat_proclog(long lTran_id,short iStatus,char cLoc_flag)
{
    int i,ilRc;
    int ilFlag = 0;

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

        i = psmShmidx_d->sIdx_proc_log.iSlink;
        while (i)
        {
            if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTran_id)
            {
                smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.proc_status = iStatus;
                ilFlag = 1;
            }
            i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
        if( ilFlag )
            return(SUCCESS);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        ilRc = swDbupdate_setstat_proclog(lTran_id, iStatus);
        return(ilRc);
    }
#endif
    return(SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmselect_swt_proc_log   
 ** ��  ��      : ���Ҵ�����ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_proc_log(long lTranid,short iProc_step,
        struct swt_proc_log *psSwt_proc_log,char cLoc_flag)
{
    int i,ilRc;

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

        i = psmShmidx_d->sIdx_proc_log.iSlink;
        while(i)
        {
            if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTranid &&
                    smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.proc_step == iProc_step) 
            {
                memcpy((char *)psSwt_proc_log,(char *)(&smShmbuf_d.psShm_proc_log[i-1].\
                            sSwt_proc_log),sizeof(struct swt_proc_log));
                if(swShmV(iSEMNUM_PROCLOG) == FAIL)
                    return(FAIL);
                else
                    return(SUCCESS);
            }
            i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/
    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        ilRc = swDbselect_swt_proc_log(lTranid,iProc_step, psSwt_proc_log);
        return(ilRc);
    }
#endif

    return(SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmselect_swt_proc_log_mrec
 ** ��  ��      : ����ָ��������ˮ�Ų���REVMODE=3��4
 **             : �Ĵ�����ˮ��¼(���ܻ᷵�ض�ʼ�¼)
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_swt_proc_log_mrec(long lTran_id, struct swt_proc_log *
        psSwt_proc_log, short *piCount)
{
    short ilCount = 0;
    int i,ilRc,ilTmp=0;

    if (swShmP(iSEMNUM_PROCLOG) == FAIL)  return(FAIL);
    i = psmShmidx_d->sIdx_proc_log.iSlink;
    while (i)
    {
        if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTran_id)
        {
            ilCount ++;
            ilTmp = 1;
            memcpy((char *)&(psSwt_proc_log[ilCount-1]), (char *)&(smShmbuf_d.
                        psShm_proc_log[i-1].sSwt_proc_log), sizeof(struct swt_proc_log));
        }
        i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
    }
    if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

    *piCount = ilCount;

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if ((psmShmidx_d->iRecinfile[SFPROCLOGID]) > 0)
    {
        ilRc = swDbselect_swt_proc_log_mrec(lTran_id, psSwt_proc_log, piCount);
        if (ilRc == SUCCESS)  ilTmp = 1;
        else if(ilRc == FAIL) return(FAIL);
    }	
#endif

    if (ilTmp) 
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmselect_swt_proc_log_clear   
 ** ��  ��      : ����ָ��ʱ����ǰ�Ĵ�����ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/09/13
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmselect_swt_proc_log_clear(long lProc_begin, 
        struct swt_proc_log *psSwt_proc_log,char *pcLoc_flag)
{
    int i,ilRc,ilTmp = 0;

    if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

    i = psmShmidx_d->sIdx_proc_log.iSlink;
    while(i)
    {
        if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.proc_begin <= lProc_begin)
        {
            memcpy((char *)psSwt_proc_log,(char *)(&smShmbuf_d.psShm_proc_log[i-1].\
                        sSwt_proc_log),sizeof(struct swt_proc_log));
            ilTmp = 1;      
            break;
        }
        i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
    }
    if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    if(ilTmp == 1)
        *pcLoc_flag = '0';

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if( !ilTmp )
    {
        ilRc = swDbselect_swt_proc_log_clear(lProc_begin, psSwt_proc_log);
        if (ilRc == SUCCESS)
        {
            ilTmp = 1;      
            *pcLoc_flag = '1';
        }
        else if(ilRc == FAIL) return(FAIL);
    }
#endif

    if (ilTmp == 1)
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmselect_max_proclog_tran
 ** ��  ��      : ����ĳ�ʽ��׵��������ˮ
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmselect_max_proclog_tran(long lTran_id,
        struct swt_proc_log *psSwt_proc_log) 
{
    int i,j,ilRc,ilTmp = 0;
    short ilProc_step = -1;
    struct swt_proc_log slSwt_proc_log;

    if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

    i = psmShmidx_d->sIdx_proc_log.iSlink;
    while (i)
    {
        if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTran_id &&
                smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.proc_step > ilProc_step)
        {
            if (ilTmp == 0) ilTmp = 1;
            j = i;    
            ilProc_step = smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.proc_step;
        }
        i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
    }
    if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);
    if (ilTmp == 1)
        memcpy((char *)psSwt_proc_log, (char *)(&(smShmbuf_d.psShm_proc_log[j-1].
                        sSwt_proc_log)), sizeof(struct swt_proc_log));

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if ((psmShmidx_d->iRecinfile[SFPROCLOGID]) > 0)
    {
        ilRc = swDbselect_max_proclog_tran(lTran_id, &slSwt_proc_log);
        if (ilRc == SUCCESS)
        {
            if (ilTmp == 1)
            {
                if (slSwt_proc_log.proc_step > (psSwt_proc_log->proc_step))
                {
                    memcpy((char *)psSwt_proc_log, (char *)&slSwt_proc_log, 
                            sizeof(struct swt_proc_log));
                }
            }
            else
            {
                memcpy((char *)psSwt_proc_log, (char *)&slSwt_proc_log, 
                        sizeof(struct swt_proc_log));  
                ilTmp = 1;
            }
        }
        else if (ilRc == FAIL) return(FAIL);
    } 
#endif

    if (ilTmp == 1)
        return(SUCCESS);
    else
        return (SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmfilefind_proc_log
 ** ��  ��      : �ж�ָ��org_file�ļ�¼�Ƿ���� 
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 1:���� 0:������
 ***************************************************************/
int swShmfilefind_proc_log(char *aOrgfile)
{
    int i,ilRc,ilTmp = FALSE;

    if (swShmP(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

    i = psmShmidx_d->sIdx_proc_log.iSlink;
    while(i)
    {
        if(strcmp(smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.org_file,
                    aOrgfile) == 0)
        {
            ilTmp = TRUE;
            break;
        }
        i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
    }
    if (swShmV(iSEMNUM_PROCLOG) == FAIL) return(FAIL);

#ifdef DB_SUPPORT  /*support database 2014-12*/

    /* �����ݿ��в��� */
    if ((ilTmp == FALSE) && ((psmShmidx_d->iRecinfile[SFTRANLOGID]) > 0))
    {
        ilRc = swDbfilefind_proc_log(aOrgfile);
        if (ilRc == SUCCESS)  
            ilTmp = TRUE;
        else if (ilRc == FAIL) return(FAIL);
    }    
#endif

    return(ilTmp);
}

/*======================= swt_rev_saf =========================*/

#ifdef DB_SUPPORT

/***************************************************************
 ** ������      : swDbinsert_swt_rev_saf
 ** ��  ��      : ����SAF��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbinsert_swt_rev_saf(struct swt_rev_saf sSwt_rev_saf)
{
    sgSwt_rev_saf1 = sSwt_rev_saf;

#ifdef DB_INFORMIX    
    EXEC SQL BEGIN WORK;
#endif
    /* modify by gf at 2004-04-26
       EXEC SQL INSERT INTO swt_rev_saf VALUES (:sgSwt_rev_saf1);
     */
    EXEC SQL INSERT INTO swt_rev_saf VALUES 
        (:sgSwt_rev_saf1.saf_id,
         :sgSwt_rev_saf1.tran_id,
         :sgSwt_rev_saf1.proc_step,
         :sgSwt_rev_saf1.saf_begin,
         :sgSwt_rev_saf1.saf_overtime,
         :sgSwt_rev_saf1.rev_overtime,
         :sgSwt_rev_saf1.saf_num,
         :sgSwt_rev_saf1.saf_status,
         :sgSwt_rev_saf1.saf_flag,
         :sgSwt_rev_saf1.saf_file,
         :sgSwt_rev_saf1.saf_offset,
         :sgSwt_rev_saf1.saf_len);
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    { 
        EXEC SQL COMMIT WORK;
        return(SUCCESS);
    }    
}

/***************************************************************
 ** ������      : swDbdelete_swt_rev_saf
 ** ��  ��      : ɾ��SAF��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbdelete_swt_rev_saf(long lSaf_id)
{
    lgSaf_id = lSaf_id;

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif  

    EXEC SQL DELETE FROM swt_rev_saf WHERE saf_id = :lgSaf_id;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);   
    }                 
    else
    {
        EXEC SQL COMMIT WORK;
        return(SUCCESS);
    }
}

/***************************************************************
 ** ������      : swDbupdate_swt_rev_saf
 ** ��  ��      : ����SAF��ˮ
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbupdate_swt_rev_saf(long lSaf_id,struct swt_rev_saf sSwt_rev_saf)
{
    sgSwt_rev_saf1 = sSwt_rev_saf;
    lgSaf_id = lSaf_id;

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif 

    EXEC SQL UPDATE swt_rev_saf SET saf_id = :sgSwt_rev_saf1.saf_id,
         tran_id = :sgSwt_rev_saf1.tran_id,  
         proc_step = :sgSwt_rev_saf1.proc_step,
         saf_begin = :sgSwt_rev_saf1.saf_begin,
         saf_overtime = :sgSwt_rev_saf1.saf_overtime,
         rev_overtime = :sgSwt_rev_saf1.rev_overtime,
         saf_num = :sgSwt_rev_saf1.saf_num,
         saf_status = :sgSwt_rev_saf1.saf_status,
         saf_flag = :sgSwt_rev_saf1.saf_flag,
         saf_file = :sgSwt_rev_saf1.saf_file,
         saf_offset = :sgSwt_rev_saf1.saf_offset,
         saf_len = :sgSwt_rev_saf1.saf_len
             WHERE saf_id = :lgSaf_id;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    { 
        EXEC SQL COMMIT WORK;   
        return(SUCCESS);
    }
}

/***************************************************************
 ** ������      : swDbselect_swt_rev_saf
 ** ��  ��      : ����SAF��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbselect_swt_rev_saf(long lSaf_id,struct swt_rev_saf *psSwt_rev_saf)
{
    lgSaf_id = lSaf_id;

    EXEC SQL SELECT * INTO :sgSwt_rev_saf1.saf_id:igTmp,
         :sgSwt_rev_saf1.tran_id:igTmp,
         :sgSwt_rev_saf1.proc_step:igTmp,
         :sgSwt_rev_saf1.saf_begin:igTmp,                   
         :sgSwt_rev_saf1.saf_overtime:igTmp,
         :sgSwt_rev_saf1.rev_overtime:igTmp,
         :sgSwt_rev_saf1.saf_num:igTmp,
         :sgSwt_rev_saf1.saf_status:igTmp,
         :sgSwt_rev_saf1.saf_flag:igTmp,
         :sgSwt_rev_saf1.saf_file:igTmp,
         :sgSwt_rev_saf1.saf_offset:igTmp,
         :sgSwt_rev_saf1.saf_len:igTmp
             FROM swt_rev_saf WHERE saf_id = :lgSaf_id;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        memcpy((char *)psSwt_rev_saf,(char *)&sgSwt_rev_saf1,
                sizeof(struct swt_rev_saf));
        return(SUCCESS);
    }  
}

/***************************************************************
 ** ������      : swDbselect_swt_rev_saf_all
 ** ��  ��      : ���ҽ�����ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbselect_swt_rev_saf_all(struct swt_rev_saf *psSwt_rev_saf, 
        short *iCount)
{
    EXEC SQL DECLARE clCur_saf1 CURSOR FOR 
        SELECT * FROM swt_rev_saf;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    EXEC SQL OPEN clCur_saf1; 
    if( sqlca.sqlcode )
    {
        swDberror(NULL);
        return(FAIL);
    }
    *iCount = 0;
    for(;;)
    {
        memset((char *)&sgSwt_rev_saf1,0x00,sizeof(struct swt_rev_saf));
        EXEC SQL FETCH clCur_saf1 INTO :sgSwt_rev_saf1.saf_id:igTmp,
             :sgSwt_rev_saf1.tran_id:igTmp,
             :sgSwt_rev_saf1.proc_step:igTmp,
             :sgSwt_rev_saf1.saf_begin:igTmp,
             :sgSwt_rev_saf1.saf_overtime:igTmp,
             :sgSwt_rev_saf1.rev_overtime:igTmp,
             :sgSwt_rev_saf1.saf_num:igTmp,
             :sgSwt_rev_saf1.saf_status:igTmp,
             :sgSwt_rev_saf1.saf_flag:igTmp,
             :sgSwt_rev_saf1.saf_file:igTmp,
             :sgSwt_rev_saf1.saf_offset:igTmp,
             :sgSwt_rev_saf1.saf_len:igTmp;
        if(sqlca.sqlcode == SQLNOTFOUND)
            break;
        else if(sqlca.sqlcode)
        {
            swDberror(NULL);
            return(FAIL);
        }
        else
        {
            memcpy((char *)&(psSwt_rev_saf[*iCount]), 
                    (char *)&sgSwt_rev_saf1, sizeof(struct swt_rev_saf));
            (*iCount) ++;
        }	
    }
    EXEC SQL CLOSE clCur_saf1;
    return(SUCCESS);                   
}

/***************************************************************
 ** ������      : swDbselect_overtime_min_revsaf
 ** ��  ��      : ���ҳ�ʱ����СSAF��ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbselect_overtime_min_revsaf(long lTime, 
        struct swt_rev_saf *psSwt_rev_saf)
{
    lgTime = lTime;

    EXEC SQL SELECT * INTO :sgSwt_rev_saf1.saf_id:igTmp,
         :sgSwt_rev_saf1.tran_id:igTmp,
         :sgSwt_rev_saf1.proc_step:igTmp,
         :sgSwt_rev_saf1.saf_begin:igTmp,
         :sgSwt_rev_saf1.saf_overtime:igTmp,
         :sgSwt_rev_saf1.rev_overtime:igTmp,
         :sgSwt_rev_saf1.saf_num:igTmp,
         :sgSwt_rev_saf1.saf_status:igTmp,
         :sgSwt_rev_saf1.saf_flag:igTmp,
         :sgSwt_rev_saf1.saf_file:igTmp,
         :sgSwt_rev_saf1.saf_offset:igTmp,
         :sgSwt_rev_saf1.saf_len:igTmp
             FROM swt_rev_saf WHERE saf_id IN(
                     SELECT min(saf_id) FROM swt_rev_saf
                     WHERE saf_overtime <> 0 AND saf_overtime < :lgTime 
                     AND saf_status   =  '0');           
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        memcpy((char *)psSwt_rev_saf,(char *)&sgSwt_rev_saf1,
                sizeof(struct swt_rev_saf));
        return(SUCCESS);
    }  
}

/***************************************************************
 ** ������      : swDbselect_max_saf_begin
 ** ��  ��      : ��ȡ SAF ��ˮ���н���ʱ�����
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swDbselect_max_saf_begin(long *lSaf_begin)
{
    EXEC SQL SELECT max(saf_begin) INTO :sgSwt_rev_saf1.saf_begin:igTmp                   
        FROM swt_rev_saf;           
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        *lSaf_begin = sgSwt_rev_saf1.saf_begin;
        return(SUCCESS);
    }  
}

/***************************************************************
 ** ������      : swDbselect_max_saf_id
 ** ��  ��      : ��ȡ SAF ��ˮ���н���ʱ����� saf_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swDbselect_max_saf_id(long lSaf_begin,long *lSaf_id)
{
    lgTime = lSaf_begin;

    EXEC SQL SELECT max(saf_id) INTO :sgSwt_rev_saf1.saf_id:igTmp                   
        FROM swt_rev_saf WHERE saf_begin = :lgTime;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        *lSaf_id = sgSwt_rev_saf1.saf_id;
        return(SUCCESS);
    }  
}

/***************************************************************
 ** ������      : swDbselect_fail_rev_saf
 ** ��  ��      : ��ȡ SAF ��ˮ����safʧ�ܵ�tran_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbselect_fail_rev_saf(long * pTranid,short *iCount)
{
    EXEC SQL DECLARE clCur_saf2 CURSOR FOR 
        SELECT tran_id FROM swt_rev_saf WHERE saf_status='2';
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    EXEC SQL OPEN clCur_saf2; 
    if( sqlca.sqlcode )
    {
        swDberror(NULL);
        return(FAIL);
    }
    *iCount = 0;
    for(;;)
    {    
        EXEC SQL FETCH clCur_saf2 INTO :sgSwt_rev_saf1.tran_id:igTmp;                 
        if(sqlca.sqlcode == SQLNOTFOUND)
            break;
        else if(sqlca.sqlcode)
        {
            swDberror(NULL);
            return(FAIL);
        }
        else
        {
            pTranid[*iCount] = sgSwt_rev_saf1.tran_id;
            (*iCount) ++;
        }	
    }
    EXEC SQL CLOSE clCur_saf2;
    return(SUCCESS);
}

/***************************************************************
 ** ������      : swDbupdate_fail_rev_saf
 ** ��  ��      : ����SAF��ˮ����safʧ�ܵ�saf_status����saf_numΪ1
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swDbupdate_fail_rev_saf(long lTranid)
{
    lgTranid = lTranid;

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif 

    EXEC SQL UPDATE swt_rev_saf SET saf_status='0',saf_num=1
        WHERE saf_status='2' AND (:lgTranid=-1 OR tran_id=:lgTranid);
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        return(SUCCESS);     
    }      	                               
}

#endif

/***************************************************************
 ** ������      : swShmsetsafid
 ** ��  ��      : ���� Saf_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmsetsafid(long lSaf_id)
{
    if (swShmP(iSEMNUM_SAFID) == FAIL) return(FAIL);  

    psmShmidx_d->lSafid = lSaf_id;

    if (swShmV(iSEMNUM_SAFID) == FAIL) return(FAIL);  

    return(SUCCESS);
}

/***************************************************************
 ** ������      : swShmgetsafid
 ** ��  ��      : ��ȡ������ Saf_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmgetsafid(long *lSaf_id)
{
    if (swShmP(iSEMNUM_SAFID) == FAIL) return(FAIL);  

    if (psmShmidx_d->lSafid >= lMAXSERNO)
        psmShmidx_d->lSafid = 0;

    *lSaf_id = psmShmidx_d->lSafid + 1;
    psmShmidx_d->lSafid = *lSaf_id;

    if (swShmV(iSEMNUM_SAFID) == FAIL) return(FAIL);  

    return(SUCCESS);
}

/*======================= �˿ڲ������� =======================*/

/**************************************************************
 ** ������      : swCheckportid
 ** ��  ��      : �ж϶˿��Ƿ���
 ** ��  ��      : 
 ** ��������    : 2001/3/9
 ** ����޸�����: 2001/3/10
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : Qid      :�˿ں�
 ** ����ֵ      : �ɹ�����SUCCESS,���󷵻�FAIL
 ***************************************************************/
short swCheckportid(short iQid)
{
    short i;

    for (i = 0; i < psmShmidx_s->sIdx_sys_queue.iCount; i ++)
    {
        if (smShmbuf_s.psSwt_sys_queue[i].q_id == iQid)
            return(SUCCESS);
    }
    return(FAIL);
}
/*�������ļ���������*/

/***************************************************************
 ** ������      : swShmopenfile
 ** ��  ��      : ���ļ�
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmopenfile(int iFileid, FILE **fp)
{
    if ((*fp = fopen(amShmfile[iFileid], "r+")) == NULL)
    {
        if ((*fp = fopen(amShmfile[iFileid], "w+")) == NULL)
            return(FAIL);
        else
            return(SUCCESS);
    }
    else
        return(SUCCESS);    
}    

/***************************************************************
 ** ������      : swShmclosefile
 ** ��  ��      : �ر��ļ�
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmclosefile(int iFileid, FILE *fp)
{
    fclose(fp);
    if (psmShmidx_d->iRecinfile[iFileid] == 0)
        unlink(amShmfile[iFileid]);
    return(SUCCESS);  
}   

/***************************************************************
 ** ������      : swShmgetfile
 ** ��  ��      : ���ļ�(��ƫ������һ������)
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
/*modify by zcd 20141229
  short swShmgetfile(FILE *fp, long lOffset, short iLen, char *aBuf)*/
short swShmgetfile(FILE *fp, long lOffset, unsigned int iLen, char *aBuf)
{
    short ilRc;

    if (lOffset < -1)
    {
        swVdebug(0,"S5085: [����/����] ƫ��������");
        return (FAIL);
    }
    fflush(fp);
    if (lOffset != OFFSETNULL)
    {
        ilRc = fseek(fp, lOffset, SEEK_SET);
        if (ilRc)
        {
            swVdebug(0,"S5090: [����/ϵͳ����] fseek()����,errno=%d,�ļ���λ����",errno);
            return (FAIL);
        }
    }
    ilRc = fread(aBuf, sizeof(char), iLen, fp);
    if (feof(fp))
    {
        swVdebug(3,"S5095: �����ļ�β");
        return (SHMFILEEND);
    }
    if (ilRc != iLen)
    {
        swVdebug(0,"S5100: [����/ϵͳ����] fread()����,errno=%d,���ļ�����",errno);
        return (FAIL);
    }
    return (SUCCESS);

}

/***************************************************************
 ** ������      : swShmputfile
 ** ��  ��      : д�ļ�(��ƫ����дһ������)
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
/*modify by zcd 20141229
  short swShmputfile(FILE *fp, long lOffset, short iLen, char *aBuf)*/
short swShmputfile(FILE *fp, long lOffset, unsigned int iLen, char *aBuf)
{
    short ilRc;

    if (lOffset < -1)
    {
        swVdebug(0,"S5105: [����/����] ƫ��������");
        return (FAIL);
    }
    fflush(fp);
    if (lOffset != OFFSETNULL)
    {
        ilRc = fseek(fp, lOffset, SEEK_SET);
        if (ilRc)
        {
            swVdebug(0,"S5110: [����/ϵͳ����] fseek()����,errno=%d,�ļ���λ����",errno);
            return (FAIL);
        }
    }
    ilRc = fwrite(aBuf, sizeof(char), iLen, fp);
    if (ilRc != iLen)
    {
        swVdebug(0,"S5115: [����/ϵͳ����] fwrite()����,errno=%d,д�ļ�����",errno);
        return (FAIL);
    }

    return (SUCCESS);

}

/***************************************************************
 ** ������      : swShmfinsert_swt_tran_log
 ** ��  ��      : ���뽻����ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfinsert_swt_tran_log(struct swt_tran_log sSwt_tran_log)
{
    short j,ilRc;
    short ilSize;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5135: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5140: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
            if (alBuf[0] == '0') break;
        if (ilRc == SHMFILEEND)  break;
    }

    alBuf[0] = '1';
    memcpy(alBuf + 1, (char *)&sSwt_tran_log, ilSize - 1);

    llOffset = llOffset - ilSize;
    ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
    if (ilRc)
    {
        swVdebug(0,"S5145: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
        swShmclosefile(SFTRANLOGID, fp);
        return (FAIL);
    }
    psmShmidx_d->iRecinfile[SFTRANLOGID] ++;

    /* === begin of added by fzj at 2002.02.28 === */
    /* ͳ�ƶ˿�Դ�������� */
    for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
    {
        if (smShmbuf_s.psSwt_sys_queue[j].q_id == sSwt_tran_log.q_tran_begin)
        {
            smShmbuf_s.psSwt_sys_queue[j].tranbegin_num += 1;
            if (sSwt_tran_log.q_target <=0) break;
        }
        if (smShmbuf_s.psSwt_sys_queue[j].q_id == sSwt_tran_log.q_target)
        {
            smShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
            if (sSwt_tran_log.q_tran_begin<=0) break;
        }
    }
    /* === end of added by fzj at 2002.02.28 === */

    swShmclosefile(SFTRANLOGID, fp);

    return (SUCCESS);
}

/***************************************************************
 ** ������      : swShmfdelete_swt_tran_log
 ** ��  ��      : ɾ��������ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfdelete_swt_tran_log(long lTran_id)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;
    short ilQorg,ilQdes;
    int j;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5150: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5155: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if (slSwt_tran_log.tran_id == lTran_id) 
                {
                    ilQorg = slSwt_tran_log.q_tran_begin;
                    ilQdes = slSwt_tran_log.q_target;
                    ilTmp = TRUE;
                    break;
                }  
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    if (ilTmp)
    {
        alBuf[0] = '0';

        llOffset = llOffset - ilSize;
        ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
        if (ilRc)
        {
            swVdebug(0,"S5160: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }

        psmShmidx_d->iRecinfile[SFTRANLOGID] --;

        /* === begin of added by fzj at 2002.02.28 === */
        /* ͳ�ƶ˿�Դ�������� */
        for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
        {
            if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQorg)
            {
                smShmbuf_s.psSwt_sys_queue[j].tranbegin_num -= 1;
                if (ilQdes <= 0) break;
            }
            if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
            {
                smShmbuf_s.psSwt_sys_queue[j].traning_num -= 1;
                if (ilQorg <= 0) break;
            }
        }
        /* === end of added by fzj at 2002.02.28 === */

        swShmclosefile(SFTRANLOGID, fp);

        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFTRANLOGID, fp);
        return (SHMNOTFOUND); 
    }  
}

/***************************************************************
 ** ������      : swShmfupdate_swt_tran_log
 ** ��  ��      : ���½�����ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfupdate_swt_tran_log(long lTran_id, 
        struct swt_tran_log sSwt_tran_log)
{
    short ilRc,j;
    short ilSize;
    short ilTmp = FALSE;
    short ilQdes,ilQdes_old;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5165: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5170: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if (slSwt_tran_log.tran_id == lTran_id) 
                {
                    /* === begin of added by fzj at 2002.02.28 === */
                    ilQdes_old = slSwt_tran_log.q_target;
                    ilQdes = sSwt_tran_log.q_target;
                    if ((ilQdes > 0) && (ilQdes != ilQdes_old))
                    {
                        for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
                        {
                            if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
                            {
                                smShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
                                if (ilQdes_old <= 0) break;
                            }
                            if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes_old)
                            {
                                smShmbuf_s.psSwt_sys_queue[j].traning_num -= 1;
                                if (ilQdes <= 0) break;
                            }
                        }
                    }
                    /* === end of added by fzj at 2002.02.28 === */
                    ilTmp = TRUE;
                    break;
                }  
            }
        }  
        if (ilRc == SHMFILEEND)  break;  /* not found */
    }

    if (ilTmp)
    {
        memcpy(alBuf + 1, (char *)&sSwt_tran_log, ilSize - 1);

        llOffset = llOffset - ilSize;
        ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
        if (ilRc)
        {
            swVdebug(0,"S5175: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }

        swShmclosefile(SFTRANLOGID, fp);
        return (SUCCESS);
    }
    else
    {  
        swShmclosefile(SFTRANLOGID, fp);
        return (SHMNOTFOUND);
    }  
}

/***************************************************************
 ** ������      : swShmfupdate_setstat_tranlog 
 ** ��  ��      �����½�����ˮ״̬
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfupdate_setstat_tranlog(long lTran_id, short iStatus)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5180: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5185: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if (slSwt_tran_log.tran_id == lTran_id) 
                {
                    ilTmp = TRUE;
                    break;
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break;  /* not found */
    }

    if (ilTmp)
    {
        slSwt_tran_log.tran_status = iStatus;
        memcpy(alBuf + 1, (char *)&slSwt_tran_log, ilSize - 1);

        llOffset = llOffset - ilSize;
        ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
        if (ilRc)
        {
            swVdebug(0,"S5190: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        swShmclosefile(SFTRANLOGID, fp);
        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFTRANLOGID, fp);
        return (SHMNOTFOUND);
    }    
}

/***************************************************************
 ** ������      : swShmfselect_swt_tran_log
 ** ��  ��      : ���ҽ�����ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_swt_tran_log(long lTran_id, 
        struct swt_tran_log *psSwt_tran_log)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5195: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5200: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if (slSwt_tran_log.tran_id == lTran_id)
                {
                    ilTmp = TRUE;
                    break;
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFTRANLOGID, fp);

    if (ilTmp)
    {
        memcpy((char *)psSwt_tran_log, (char *)&slSwt_tran_log, ilSize - 1);
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }    
}

/***************************************************************
 ** ������      : swShmfselect_swt_tran_log_all
 ** ��  ��      : ���ҽ�����ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_swt_tran_log_all(struct swt_tran_log *psSwt_tran_log,
        short *iCount)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5205: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5210: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                memcpy((char *)&(psSwt_tran_log[*iCount]), 
                        (char *)&slSwt_tran_log, ilSize - 1);
                (*iCount) ++;
                if (*iCount >= iMAXTRANLOGREC)
                {
                    swShmclosefile(SFTRANLOGID, fp);
                    return (FAIL);
                }
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFTRANLOGID, fp);

    return (SUCCESS);  

}

/***************************************************************
 ** ������      : swShmfselect_overtime_min_tranlog
 ** ��  ��      : ���ҳ�ʱ����С������ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_overtime_min_tranlog(long lTime,
        struct swt_tran_log *psSwt_tran_log)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    long  llTran_id = lMAXSERNO;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5215: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5220: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);

                if ((slSwt_tran_log.tran_status == 0) &&
                        (slSwt_tran_log.tran_overtime < lTime) &&
                        (slSwt_tran_log.tran_overtime != 0) &&
                        (slSwt_tran_log.tran_id < llTran_id))
                {
                    llOffset_tmp = llOffset;
                    llTran_id = slSwt_tran_log.tran_id;
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    llOffset_tmp = llOffset_tmp - ilSize;
    if (llOffset_tmp >= 0)
    {
        ilRc = swShmgetfile(fp, llOffset_tmp, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5225: [����/ϵͳ����] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        memcpy((char *)psSwt_tran_log, alBuf + 1, ilSize - 1);

        swShmclosefile(SFTRANLOGID, fp);
        return (SUCCESS);
    }  
    else
    {
        swShmclosefile(SFTRANLOGID, fp);
        return (SHMNOTFOUND);
    }

}

/***************************************************************
 ** ������      : swShmfselect_max_tran_begin
 ** ��  ��      : ��ȡ������ˮ���н���ʱ�����
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_max_tran_begin(long *lTran_begin)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    long  llTmp_begin = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5230: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5235: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);

                if (slSwt_tran_log.tran_begin > llTmp_begin)
                {
                    llOffset_tmp = llOffset;
                    llTmp_begin = slSwt_tran_log.tran_begin;
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    llOffset_tmp = llOffset_tmp - ilSize;
    if (llOffset_tmp >= 0)
    {
        ilRc = swShmgetfile(fp, llOffset_tmp, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5240: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
        *lTran_begin = slSwt_tran_log.tran_begin;

        swShmclosefile(SFTRANLOGID, fp);
        return (SUCCESS);
    }  
    else
    {
        *lTran_begin = 0;
        swShmclosefile(SFTRANLOGID, fp);
        return (SHMNOTFOUND);
    }

}

/***************************************************************
 ** ������      : swShmfselect_max_tran_id
 ** ��  ��      : ��ȡ������ˮ���н���ʱ�����Ľ�����ˮ��
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_max_tran_id(long lTran_begin,long *lTran_id)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llTran_id_tmp = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5245: [����/��������] swShmopenfile����,������=%d,���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5250: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if ((slSwt_tran_log.tran_begin == lTran_begin) &&
                        (slSwt_tran_log.tran_id > llTran_id_tmp))
                {
                    llTran_id_tmp = slSwt_tran_log.tran_id;
                    ilTmp = TRUE;
                    /* break; */
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFTRANLOGID, fp);

    if (ilTmp)
    {
        *lTran_id = llTran_id_tmp;
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }  
}

/***************************************************************
 ** ������      : swShmffilefind_tran_log
 ** ��  ��      : �ж��Ƿ����ָ��resq_file��resp_file�ļ�¼ 
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 1:���� 0:������
 ***************************************************************/
int swShmffilefind_tran_log(char * aFilename)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5255: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5260: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);

                if ((strcmp(slSwt_tran_log.resq_file, aFilename) == 0) || 
                        (strcmp(slSwt_tran_log.resp_file, aFilename) == 0))
                {
                    ilTmp = TRUE;
                    break;
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFTRANLOGID, fp);

    return (ilTmp);

}

/* =================== file_swt_proc_log ====================== */

/***************************************************************
 ** ������      : swShmf2shm_swt_proc_log
 ** ��  ��      : ���ļ���һ��������ˮ�������ڴ�(ɾ��ʱ��)
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : iPos    �ڹ����ڴ��е�λ��
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmf2shm_swt_proc_log(short iPos)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5265: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5270: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);

                llOffset_tmp = llOffset - ilSize;

                ilTmp = TRUE;
                alBuf[0] = '0';

                ilRc = swShmputfile(fp, llOffset_tmp, ilSize, alBuf);
                if (ilRc)
                {
                    swVdebug(0,"S5275: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
                    swShmclosefile(SFPROCLOGID, fp);
                    return (FAIL);
                }    
                break;        
            }  
        }  
        if (ilRc == SHMFILEEND)  break;
    }

    if (ilTmp)
    {
        psmShmidx_d->iRecinfile[SFPROCLOGID] --;

        memcpy((char *)(&(smShmbuf_d.psShm_proc_log[iPos].sSwt_proc_log)),
                (char *)&slSwt_proc_log, ilSize - 1);

        swShmclosefile(SFPROCLOGID, fp);

        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFPROCLOGID, fp);
        return (SHMNOTFOUND);
    }    
}

/***************************************************************
 ** ������      : swShmfinsert_swt_proc_log
 ** ����        : ���봦����ˮ��¼
 ** ����        :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfinsert_swt_proc_log(struct swt_proc_log sSwt_proc_log)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5280: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;

    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5285: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
            if (alBuf[0] == '0') break;
        if (ilRc == SHMFILEEND)  break;
    }

    alBuf[0] = '1';
    memcpy(alBuf + 1, (char *)&sSwt_proc_log, ilSize - 1);

    llOffset = llOffset - ilSize;
    ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
    if (ilRc)
    {
        swVdebug(0,"S5290: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);

        swShmclosefile(SFPROCLOGID, fp);
        return (FAIL);
    }
    psmShmidx_d->iRecinfile[SFPROCLOGID] ++;

    swShmclosefile(SFPROCLOGID, fp);

    return (SUCCESS);
}

/***************************************************************
 ** ������      : swShmfdelete_swt_proc_log
 ** ��  ��      : ɾ��������ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfdelete_swt_proc_log(long lTran_id,short iProc_step)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5295: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5300: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);
                if ((slSwt_proc_log.tran_id == lTran_id) &&
                        (slSwt_proc_log.proc_step == iProc_step))
                {
                    ilTmp = TRUE;
                    break;
                }
            }
        }
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    if (ilTmp)
    {
        alBuf[0] = '0';
        llOffset = llOffset - ilSize;
        ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
        if (ilRc)
        {
            swVdebug(0,"S5305: [����/��������] swShmputfile����,������=%d,д�ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }

        psmShmidx_d->iRecinfile[SFPROCLOGID] --;

        swShmclosefile(SFPROCLOGID, fp);

        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFPROCLOGID, fp);
        return (SHMNOTFOUND);
    }
}

/***************************************************************
 ** ������      : swShmfdelete_swt_proc_log_mrec
 ** ��  ��      : ɾ����ˮ�Ŷ�Ӧ�����д�����ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfdelete_swt_proc_log_mrec(long lTran_id)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5310: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5315: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);

                if (slSwt_proc_log.tran_id == lTran_id) 
                {
                    llOffset_tmp = llOffset - ilSize;

                    ilTmp = TRUE;
                    alBuf[0] = '0';

                    ilRc = swShmputfile(fp, llOffset_tmp, ilSize, alBuf);
                    if (ilRc)
                    {
                        swVdebug(0,"S5320: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
                        swShmclosefile(SFPROCLOGID, fp);
                        return (FAIL);
                    }
                    psmShmidx_d->iRecinfile[SFPROCLOGID] --;
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFPROCLOGID, fp);

    if (ilTmp)
        return (SUCCESS);
    else
        return (SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmfdelete_proclog_tran_q
 ** ��  ��      : ɾ��ָ������ָ����������Ӧ�����д�����ˮ
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfdelete_proclog_tran_q(long lTran_id, short iQid)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5325: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5330: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);

                if ((slSwt_proc_log.tran_id == lTran_id) && 
                        (slSwt_proc_log.q_target == iQid))
                {
                    llOffset_tmp = llOffset - ilSize;

                    ilTmp = TRUE;
                    alBuf[0] = '0';

                    ilRc = swShmputfile(fp, llOffset_tmp, ilSize, alBuf);
                    if (ilRc)
                    {
                        swVdebug(0,"S5335: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
                        swShmclosefile(SFPROCLOGID, fp);
                        return (FAIL);
                    }
                    psmShmidx_d->iRecinfile[SFPROCLOGID] --;
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFPROCLOGID, fp);

    if (ilTmp)
        return (SUCCESS);
    else
        return (SHMNOTFOUND);
}

/***************************************************************
 ** ������      	: swShmfupdate_swt_proc_log
 ** ��  ��      	: ���´�����ˮ��¼
 ** ��  ��      	:
 ** ��������    	: 2001/05/31
 ** ����޸�����	: 2001/05/31
 ** ������������	:
 ** ȫ�ֱ���    	:
 ** ��������    	:
 ** ����ֵ	: 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfupdate_swt_proc_log(long lTran_id,short iProc_step,
        struct swt_proc_log sSwt_proc_log)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5340: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5345: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);
                if ((slSwt_proc_log.tran_id == lTran_id) &&
                        (slSwt_proc_log.proc_step == iProc_step))
                {
                    ilTmp = TRUE;
                    break;
                }
            }
        }
        if (ilRc == SHMFILEEND)  break;  /* not found */
    }
    if (ilTmp)
    {
        memcpy(alBuf + 1, (char *)&sSwt_proc_log, ilSize - 1);

        llOffset = llOffset - ilSize;
        ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
        if (ilRc)
        {
            swVdebug(0,"S5350: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }

        swShmclosefile(SFPROCLOGID, fp);
        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFPROCLOGID, fp);
        return (SHMNOTFOUND);
    }
}

/***************************************************************
 ** ������      	: swShmfupdate_setstat_proclog
 ** ��  ��      	�����´�����ˮ״̬
 ** ��  ��      	:
 ** ��������    	: 2001/05/31
 ** ����޸�����	: 2001/05/31
 ** ������������	:
 ** ȫ�ֱ���    	:
 ** ��������    	:
 ** ����ֵ      	: 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfupdate_setstat_proclog(long lTran_id,short iStatus)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5355: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5360: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);

                if (slSwt_proc_log.tran_id == lTran_id)
                {
                    llOffset_tmp = llOffset - ilSize;

                    ilTmp = TRUE;
                    alBuf[0] = '1';
                    slSwt_proc_log.proc_status = iStatus;
                    memcpy(alBuf + 1, (char *)&slSwt_proc_log, ilSize - 1);

                    ilRc = swShmputfile(fp, llOffset_tmp, ilSize, alBuf);
                    if (ilRc)
                    {
                        swVdebug(0,"S5365: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
                        swShmclosefile(SFPROCLOGID, fp);
                        return (FAIL);
                    }
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFPROCLOGID, fp);

    if (ilTmp)
        return (SUCCESS);
    else
        return (SHMNOTFOUND);
}

/***************************************************************
 ** ������      : swShmfselect_swt_proc_log
 ** ��  ��      : ���Ҵ�����ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfselect_swt_proc_log(long lTran_id,short iProc_step,
        struct swt_proc_log *psSwt_proc_log)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5370: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5375: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);
                if ((slSwt_proc_log.tran_id == lTran_id) &&
                        (slSwt_proc_log.proc_step == iProc_step))
                {
                    ilTmp = TRUE;
                    break;
                }
            }
        }
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFPROCLOGID, fp);

    if (ilTmp)
    {
        memcpy((char *)psSwt_proc_log, (char *)&slSwt_proc_log, ilSize - 1);
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }
}

/***************************************************************
 ** ������      : swShmfselect_swt_proc_log_mrec
 ** ��  ��      : ����ָ��������ˮ�Ų���REVMODE=3��4
 **             : �Ĵ�����ˮ��¼(���ܻ᷵�ض�ʼ�¼)
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfselect_swt_proc_log_mrec(long lTran_id, struct swt_proc_log *
        psSwt_proc_log, short *piCount)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5380: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5385: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);

                if (slSwt_proc_log.tran_id == lTran_id)
                {
                    memcpy((char *)&(psSwt_proc_log[*piCount]), 
                            (char *)&slSwt_proc_log, ilSize - 1);
                    (*piCount) ++;
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFPROCLOGID, fp);

    return (SUCCESS);  

}

/***************************************************************
 ** ������      : swShmfselect_max_proclog_tran
 ** ��  ��      : ����ĳ�ʽ��׵��������ˮ
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfselect_max_proclog_tran(long lTran_id,
        struct swt_proc_log *psSwt_proc_log)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    short ilProc_step = -1;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5390: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5395: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);

                if ((slSwt_proc_log.tran_id == lTran_id) &&
                        (slSwt_proc_log.proc_step > ilProc_step))
                {
                    llOffset_tmp = llOffset;
                    ilTmp = TRUE;
                    ilProc_step = slSwt_proc_log.proc_step;
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    llOffset_tmp = llOffset_tmp - ilSize;
    if (llOffset_tmp >= 0)
    {
        ilRc = swShmgetfile(fp, llOffset_tmp, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5400: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        memcpy((char *)psSwt_proc_log, alBuf + 1, ilSize - 1);

        swShmclosefile(SFPROCLOGID, fp);
        return (SUCCESS);
    }  
    else
    {
        swShmclosefile(SFPROCLOGID, fp);
        return (SHMNOTFOUND);
    }  

}

/***************************************************************
 ** ������      : swShmffilefind_proc_log
 ** ��  ��      : �ж�ָ��org_file�ļ�¼�Ƿ����
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 1:���� 0:������
 ***************************************************************/
int swShmffilefind_proc_log(char *aOrgfile)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = swShmopenfile(SFPROCLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5405: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5410: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFPROCLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);

                if (strcmp(slSwt_proc_log.org_file, aOrgfile) == 0)
                {
                    ilTmp = TRUE;
                    break;
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFPROCLOGID, fp);

    return (ilTmp);

}

/* =================== file_swt_rev_saf ====================== */

/***************************************************************
 ** ������      : swShmf2shm_swt_rev_saf
 ** ��  ��      : ���ļ���һ��saf��ˮ�������ڴ�(ɾ��ʱ��)
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : iPos    �ڹ����ڴ��е�λ��
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmf2shm_swt_rev_saf(short iPos)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5415: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5420: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);

                llOffset_tmp = llOffset - ilSize;

                ilTmp = TRUE;
                alBuf[0] = '0';

                ilRc = swShmputfile(fp, llOffset_tmp, ilSize, alBuf);
                if (ilRc)
                {
                    swVdebug(0,"S5425: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
                    swShmclosefile(SFSAFLOGID, fp);
                    return (FAIL);
                }    
                break;      
            }  
        }  
        if (ilRc == SHMFILEEND)  break;
    }

    if (ilTmp)
    {
        psmShmidx_d->iRecinfile[SFSAFLOGID] --;

        memcpy((char *)(&(smShmbuf_d.psShm_rev_saf[iPos].sSwt_rev_saf)),
                (char *)&slSwt_rev_saf, ilSize - 1);

        swShmclosefile(SFSAFLOGID, fp);

        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFSAFLOGID, fp);
        return (SHMNOTFOUND);
    }    
}

/***************************************************************
 ** ������      : swShmfinsert_swt_rev_saf
 ** ��  ��      : ����SAF��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ: 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfinsert_swt_rev_saf(struct swt_rev_saf sSwt_rev_saf)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5430: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5435: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
            if (alBuf[0] == '0') break;
        if (ilRc == SHMFILEEND)  break;
    }

    alBuf[0] = '1';
    memcpy(alBuf + 1, (char *)&sSwt_rev_saf, ilSize - 1);

    llOffset = llOffset - ilSize;
    ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
    if (ilRc)
    {
        swVdebug(0,"S5440: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
        swShmclosefile(SFSAFLOGID, fp);
        return (FAIL);
    }
    psmShmidx_d->iRecinfile[SFSAFLOGID] ++;

    swShmclosefile(SFSAFLOGID, fp);

    return (SUCCESS);
}

/***************************************************************
 ** ������      : swShmfdelete_swt_rev_saf
 ** ��  ��      : ɾ��SAF��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfdelete_swt_rev_saf(long lSaf_id)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5445: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5450: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);
                if (slSwt_rev_saf.saf_id == lSaf_id)
                {
                    ilTmp = TRUE;
                    break;
                }
            }
        }
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    if (ilTmp)
    {
        alBuf[0] = '0';

        llOffset = llOffset - ilSize;
        ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
        if (ilRc)
        {
            swVdebug(0,"S5455: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }

        psmShmidx_d->iRecinfile[SFSAFLOGID] --;

        swShmclosefile(SFSAFLOGID, fp);

        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFSAFLOGID, fp);
        return (SHMNOTFOUND);
    }
}

/***************************************************************
 ** ������      : swShmfupdate_swt_rev_saf
 ** ����        : ���½�����ˮ��¼
 ** ����        :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfupdate_swt_rev_saf(long lsaf_id, struct swt_rev_saf sSwt_rev_saf)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5460: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5465: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);
                if (slSwt_rev_saf.saf_id ==lsaf_id)
                {
                    ilTmp = TRUE;
                    break;
                }
            }
        }
        if (ilRc == SHMFILEEND)  break;  /* not found */
    }

    if (ilTmp)
    {
        memcpy(alBuf + 1, (char *)&sSwt_rev_saf, ilSize - 1);

        llOffset = llOffset - ilSize;
        ilRc = swShmputfile(fp, llOffset, ilSize, alBuf);
        if (ilRc)
        {
            swVdebug(0,"S5470: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }

        swShmclosefile(SFSAFLOGID, fp);
        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFSAFLOGID, fp);
        return (SHMNOTFOUND);
    }
}

/***************************************************************
 ** ������      : swShmfselect_swt_rev_saf
 ** ����        : ���ҽ�����ˮ��¼
 ** ����        :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_swt_rev_saf(long lsaf_id,
        struct swt_rev_saf *psSwt_rev_saf)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5475: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5480: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);
                if (slSwt_rev_saf.saf_id == lsaf_id)
                {
                    ilTmp = TRUE;
                    break;
                }
            }
        }
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFSAFLOGID, fp);

    if (ilTmp)
    {
        memcpy((char *)psSwt_rev_saf, (char *)&slSwt_rev_saf, ilSize - 1);
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }
}

/***************************************************************
 ** ������      : swShmfselect_swt_rev_saf_all
 ** ��  ��      : ��������saf��ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_swt_rev_saf_all(struct swt_rev_saf *psSwt_rev_saf,
        short *iCount)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5485: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5490: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);
                memcpy((char *)&(psSwt_rev_saf[*iCount]), 
                        (char *)&slSwt_rev_saf, ilSize - 1);
                (*iCount) ++;
                if (*iCount >= iMAXREVSAFREC)
                {
                    swShmclosefile(SFSAFLOGID, fp);
                    return (FAIL);
                }
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFSAFLOGID, fp);

    return (SUCCESS);  

}

/***************************************************************
 ** ������      : swShmfselect_overtime_min_revsaf
 ** ��  ��      : ���ҳ�ʱ����СSAF��ˮ��¼
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_overtime_min_revsaf(long lTime,
        struct swt_rev_saf *psSwt_rev_saf)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    long  llSaf_id = lMAXSERNO;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5495: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5500: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);

                if ((slSwt_rev_saf.saf_status[0] == '0') &&
                        (slSwt_rev_saf.saf_overtime < lTime) &&
                        (slSwt_rev_saf.saf_overtime != 0) &&
                        (slSwt_rev_saf.saf_id < llSaf_id))
                {
                    llOffset_tmp = llOffset;
                    llSaf_id = slSwt_rev_saf.saf_id;
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    llOffset_tmp = llOffset_tmp - ilSize;
    if (llOffset_tmp >= 0)
    {
        ilRc = swShmgetfile(fp, llOffset_tmp, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5505: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        memcpy((char *)psSwt_rev_saf, alBuf + 1, ilSize - 1);

        swShmclosefile(SFSAFLOGID, fp);
        return (SUCCESS);
    }  
    else
    {
        swShmclosefile(SFSAFLOGID, fp);
        return (SHMNOTFOUND);
    }

}

/***************************************************************
 ** ������      : swShmfselect_max_saf_begin
 ** ��  ��      : ��ȡ SAF ��ˮ���н���ʱ�����
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_max_saf_begin(long *lSaf_begin)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    long  llTmp_begin = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5510: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5515: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);

                if (slSwt_rev_saf.saf_begin > llTmp_begin)
                {
                    llOffset_tmp = llOffset;
                    llTmp_begin = slSwt_rev_saf.saf_begin;
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    llOffset_tmp = llOffset_tmp - ilSize;
    if (llOffset_tmp >= 0)
    {
        ilRc = swShmgetfile(fp, llOffset_tmp, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5520: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);
        *lSaf_begin = slSwt_rev_saf.saf_begin;

        swShmclosefile(SFSAFLOGID, fp);
        return (SUCCESS);
    }  
    else
    {
        *lSaf_begin = 0;
        swShmclosefile(SFSAFLOGID, fp);
        return (SHMNOTFOUND);
    }

}

/***************************************************************
 ** ������      : swShmfselect_max_saf_id
 ** ��  ��      : ��ȡ SAF ��ˮ���н���ʱ����� saf_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmfselect_max_saf_id(long lSaf_begin,long *lSaf_id)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llSaf_id_tmp = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5525: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5530: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);
                if ((slSwt_rev_saf.saf_begin == lSaf_begin) &&
                        (slSwt_rev_saf.saf_id > llSaf_id_tmp))
                {
                    llSaf_id_tmp = slSwt_rev_saf.saf_id;
                    ilTmp = TRUE;
                    /* break; */
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFSAFLOGID, fp);

    if (ilTmp)
    {
        *lSaf_id = llSaf_id_tmp;
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }  
}

/***************************************************************
 ** ������      : swShmfselect_fail_rev_saf
 ** ��  ��      : ��ȡ SAF ��ˮ����safʧ�ܵ�tran_id
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfselect_fail_rev_saf(long * pTranid,short *iCount)
{
    short ilRc;
    short ilSize;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5535: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5540: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);

                if (slSwt_rev_saf.saf_status[0] == '2')
                {
                    llOffset_tmp = llOffset;

                    pTranid[*iCount] = slSwt_rev_saf.tran_id;
                    (*iCount) ++;
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFSAFLOGID, fp);
    return (SUCCESS);

}

/***************************************************************
 ** ������      : swShmfupdate_fail_rev_saf
 ** ��  ��      : ����SAF��ˮ����safʧ�ܵ�saf_status����saf_numΪ1
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfupdate_fail_rev_saf(long lTranid)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_rev_saf slSwt_rev_saf;

    if ((ilRc = swShmopenfile(SFSAFLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5545: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_rev_saf) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5550: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFSAFLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_rev_saf, alBuf + 1, ilSize - 1);

                if ((slSwt_rev_saf.saf_status[0] == '2') && 
                        ((lTranid == -1) || (lTranid == slSwt_rev_saf.tran_id)))
                {
                    llOffset_tmp = llOffset - ilSize;

                    ilTmp = TRUE;
                    strcpy(slSwt_rev_saf.saf_status, "0");
                    slSwt_rev_saf.saf_num = 1;
                    memcpy(alBuf + 1, (char *)&slSwt_rev_saf, ilSize -1);

                    ilRc = swShmputfile(fp, llOffset_tmp, ilSize, alBuf);
                    if (ilRc)
                    {
                        swVdebug(0,"S5555: [����/��������] swShmputfile()����,������=%d,д�ļ�����",ilRc);
                        swShmclosefile(SFSAFLOGID, fp);
                        return (FAIL);
                    }
                }
            }
        }  
        if (ilRc == SHMFILEEND)  break; /* end */
    }

    swShmclosefile(SFSAFLOGID, fp);

    if (ilTmp)
        return (SUCCESS);
    else
        return (SHMNOTFOUND);

}

int swShmisfull()
{
    long llMax,llCnt;

    llMax = smShmbuf_s.psSwt_sys_config[0].iMaxtrannum;
    llCnt = psmShmidx_d->iRecinshm[SFTRANLOGID];
    if (llMax <= 0) return(0);
    if (llCnt < llMax) return(0);
    swVdebug(0,"S5560: [����/����] �Ѵﵽ���������������[%ld]",llMax);
    return(-1);
}

/* === begin of added by fzj at 2002.02.28 === */
int swShmtranisfull(short iQid)
{
    int i;

    for (i=0; i<psmShmidx_s->sIdx_sys_queue.iCount; i++)
    {
        if (smShmbuf_s.psSwt_sys_queue[i].q_id == iQid)
        {
            if ((smShmbuf_s.psSwt_sys_queue[i].tranbegin_max>0) &&
                    (smShmbuf_s.psSwt_sys_queue[i].tranbegin_num>=smShmbuf_s.psSwt_sys_queue[i].tranbegin_max))
                return(-1);
            else
                break;
        }
    }
    return(0);
}

#ifndef HASH
/* add by gengling at 2013.03.18 begin PSBC_V1.0 */
/************************************************************
 **��  ��: �����������޵��ж�
 **��  ��: iQid      - ������� Դ������
           aTrancode - ������
 **����ֵ: 0  - �ɹ� û�г���
           -1 - ʧ�� ������
 ************************************************************/
int swShmtranflowisfull(short iQid, char *aTrancode)
{
    int i;

    swVdebug(4, "iQid=[%d],aTrancode=[%s]", iQid, aTrancode);
    for (i=0; i<psmShmidx_s->sIdx_sys_tran.iCount; i++)
    {
        if (smShmbuf_s.psSwt_sys_tran[i].sys_id == iQid &&
                !strncmp(smShmbuf_s.psSwt_sys_tran[i].tran_code, aTrancode, sizeof(smShmbuf_s.psSwt_sys_tran[i].tran_code)))
        {
            if ((smShmbuf_s.psSwt_sys_tran[i].tranning_max>0) &&
                    (smShmbuf_s.psSwt_sys_tran[i].tranning_num>=smShmbuf_s.psSwt_sys_tran[i].tranning_max))
            {
                swVdebug(1, "tran flow is full ����[%d][%s]������[%ld]�����������[%ld]", iQid, aTrancode, \
                        smShmbuf_s.psSwt_sys_tran[i].tranning_num, smShmbuf_s.psSwt_sys_tran[i].tranning_max);
                return(-1);
            }
            else
                break;
        }
    }
    return(0);
}
/* add by gengling at 2013.03.18 end PSBC_V1.0 */
#else
/************************************************************
 **��  ��: �����������޵��ж�
 **��  ��: iQid      - ������� Դ������
           aTrancode - ������
 **����ֵ: 0  - �ɹ� û�г���
           -1 - ʧ�� ������
           -2 - ʧ�� ��SWTRAN_CTL.CFG��û���ҵ�
 ************************************************************/
int swShmtranflowisfull(short iQid, char *aTrancode)
{
    int iRc = 0;
    struct swt_sys_tran p_in;
    struct swt_sys_tran *p_out;

    p_in.sys_id = iQid;
    strcpy(p_in.tran_code, aTrancode);

    iRc = shm_swt_sys_tran(&p_in, &p_out);
    if(iRc)
    {
        swVdebug(1, "shm_swt_sys_tran error iRc[%d]", iRc);
        return(-2);
    }

    swVdebug(5, "swShmtranflowisfull sys_id[%d],tran_code[%s],tran_name[%s],priority[%d],status[%s],tranning_max[%ld],debug_level[%d],tranning_num[%ld]",
            p_out->sys_id, p_out->tran_code, p_out->tran_name, p_out->priority, p_out->status,
            p_out->tranning_max, p_out->debug_level, p_out->tranning_num); 

    if ((p_out->tranning_max > 0) && (p_out->tranning_num >= p_out->tranning_max))
    {
        swVdebug(1, "tran flow is full ����[%d][%s]������[%ld]�����������[%ld]", iQid, aTrancode, \
                p_out->tranning_num, p_out->tranning_max);
        return(-1);
    }
    return(0);
}
#endif

/*2009-9-9 11:06 �����ṩ����ԭ��
  int swShmportisAlldown(int groupid,short iQid,char *aTrancode)
  {
  return -4;
  }
  2009-9-9 11:06 �����ṩ����ԭ��*/

int swShmportisdown(short iQid,char *aTrancode)
{
    int i,ilRc,ilTmp=0;
    struct swt_sys_queue slSwt_sys_queue;

    /* ��ѯ���׶˿ڹ������е����м�¼ */
    for (i=0; i<psmShmidx_s->sIdx_sys_matchport.iCount; i++)
    {
        /* ��ɸѡ�������ж˿���������ļ�¼ */
        if (smShmbuf_s.psSwt_sys_matchport[i].qid != iQid) 
        {
            if (ilTmp) return(0);
            continue;
        }
        /* ��ɸѡ�������н�����������ļ�¼ */
        if (strcmp(aTrancode, smShmbuf_s.psSwt_sys_matchport[i].trancode))
        {
            if (strcmp("*", smShmbuf_s.psSwt_sys_matchport[i].trancode))
                continue;
            else
                if (ilTmp) return(0);
        }    
        /* ��ѯ��������(Դ���˿ڡ�Դ��������)�����˿ڵĶ˿���Ϣswt_sys_queue */
        ilRc = swShmselect_swt_sys_queue(smShmbuf_s.psSwt_sys_matchport[i].port,&slSwt_sys_queue);
        /* ִ��swShmselect_swt_sys_queue����ʧ�ܷ��� */
        if (ilRc) return(-1);
        swVdebug(2, "port status:%d, port id=%d",slSwt_sys_queue.port_status, slSwt_sys_queue.q_id );
        /* �����˿ڵ��κ�һ��˿�״̬down����-3(�˿�״̬down) 0xF8=11111000 */
        if ((slSwt_sys_queue.port_status & 0xF8) != 0xF8) return(-5);
        /* �жϹ����˿��Ƿ����ö˿ڴ����������� */
        if (slSwt_sys_queue.traning_max == 0) continue;
        /* ��������˿ڽ����������򷵻�-4(�˿ڽ��׳���) */
        if (slSwt_sys_queue.traning_num >= slSwt_sys_queue.traning_max) return(-4);
        if (ilTmp == 0) ilTmp = 1;
    }
    return(0);
}

/***************************************************************
 ** ������      : swShmfselect_qid_revkey_tranlog
 ** ��  ��      : ���ҽ�����ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfselect_qid_revkey_tranlog(int iQid, char *aKey, 
        struct swt_tran_log *psSwt_tran_log)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5575: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5580: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if (slSwt_tran_log.q_tran_begin == iQid)
                {
                    if (strcmp(slSwt_tran_log.rev_key,aKey) == 0)
                    {
                        ilTmp = TRUE;
                        break;
                    }
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFTRANLOGID, fp);

    if (ilTmp)
    {
        memcpy((char *)psSwt_tran_log, (char *)&slSwt_tran_log, ilSize - 1);
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }    
}

/***************************************************************
 ** ������      : swShmfselect_qid_msghdkey_tranlog
 ** ��  ��      : ���ҽ�����ˮ��¼
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
int swShmfselect_qid_msghdkey_tranlog(int iQid, char *aKey, 
        struct swt_tran_log *psSwt_tran_log)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5585: [����/��������] swShmopenfile()����,������=%d, ���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5590: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if (slSwt_tran_log.q_target == iQid)
                {
                    if (strcmp(slSwt_tran_log.msghdkey,aKey) == 0)
                    {
                        ilTmp = TRUE;
                        break;
                    }  
                }
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    swShmclosefile(SFTRANLOGID, fp);

    if (ilTmp)
    {
        memcpy((char *)psSwt_tran_log, (char *)&slSwt_tran_log, ilSize - 1);
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }    
}

/***************************************************************
 ** ������      : swShmportcheck
 ** ��  ��      : �˿�״̬�ж�
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmportcheck(short iQid,char * aMsghead,short  *iResult)
{
    struct msghead slMsghead;
    short ilRc;
    unsigned char alTmp1 = 0;
    struct swt_sys_queue slSwt_sys_queue;

    memcpy(&slMsghead,aMsghead,sizeof(struct msghead)); 
    ilRc = swShmselect_swt_sys_queue(iQid, &slSwt_sys_queue);
    if (ilRc != SUCCESS)
    {
        swVdebug(0,"S5595: [����/����] �˿�[%d]������", iQid);
        return(FAIL);
    } 
    alTmp1 = slSwt_sys_queue.port_status;
    if ((alTmp1 & 0xf8) == 0xf8) /*����5λȫΪUP��*/
        *iResult = 1;
    else
    {
        if (slMsghead.iBegin_q_id == iMBECHOTEST)
        {
            if (alTmp1 & 0x08)  /* �˹���Ԥ��ΪUP */
                *iResult = 1;
            else
                *iResult = 0;
        }
        else
            *iResult = 0;
    }
    return(SUCCESS); 
}

/***************************************************************
 ** ������      : swTranisdisable
 ** ��  ��      : ���׾ܾ�
 ** ��  ��      : nh
 ** ��������    : 2002/03/19
 ** ����޸�����: 2002/03/19
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 1:�ܾ�,0:��������,-1:����
 ***************************************************************/
int swTranisdisable(short iQid,char * aTrancode)
{
    short ilRc;
    struct  swt_sys_route_m slSwt_sys_route_m;

    if ( (ilRc = swShmselect_swt_sys_route_m(iQid,aTrancode,
                    &slSwt_sys_route_m))!=SUCCESS)
    {
        swVdebug(0,"S5600: [����/�����ڴ�] �ӹ����ڴ�·��������ƥ�佻�׿��س���[%d]",ilRc);
        return(-1);
    }

    swVdebug(2,"S5605: ƥ��õ����׿���[%d]",slSwt_sys_route_m.flag);
    if(slSwt_sys_route_m.flag == 1)
    {
        return(1);
    }
    else
    {
        return(0);
    }
}

#ifdef DB_SUPPORT   /*support database 2014-12*/

/********* ���ݿ��������*****************/
/***************************************************************
 ** ������      : swDbselect_swt_tran_log
 ** ��  ��      : �����ݿ��в��ҽ�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_swt_tran_log(long lTranid,struct swt_tran_log *psSwt_tran_log)
{ 
    lgTranid = lTranid;

    EXEC SQL SELECT * INTO :sgSwt_tran_log1.tran_id:igTmp,
         :sgSwt_tran_log1.tran_begin:igTmp,
         :sgSwt_tran_log1.tran_status:igTmp,
         :sgSwt_tran_log1.tran_end:igTmp,
         :sgSwt_tran_log1.tran_overtime:igTmp,
         :sgSwt_tran_log1.q_tran_begin:igTmp,
         :sgSwt_tran_log1.tc_tran_begin:igTmp,
         :sgSwt_tran_log1.q_target:igTmp,
         :sgSwt_tran_log1.resq_file:igTmp,
         :sgSwt_tran_log1.resq_offset:igTmp,
         :sgSwt_tran_log1.resq_len:igTmp,
         :sgSwt_tran_log1.resp_file:igTmp,
         :sgSwt_tran_log1.resp_offset:igTmp,
         :sgSwt_tran_log1.resp_len:igTmp,
         :sgSwt_tran_log1.rev_key:igTmp,
         :sgSwt_tran_log1.msghdkey:igTmp,
         :sgSwt_tran_log1.msghead:igTmp,
         :sgSwt_tran_log1.xastatus:igTmp
             FROM swt_tran_log WHERE tran_id = :lgTranid;
    if(sqlca.sqlcode == SQLNOTFOUND) 
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        /* *psSwt_tran_log = sgSwt_tran_log1; */
        memcpy((char *)psSwt_tran_log,(char *)&sgSwt_tran_log1,sizeof(struct swt_tran_log));
        return(SUCCESS);	
    }	
}

/***************************************************************
 ** ������      : swDbselect_swt_tran_log
 ** ��  ��      : �����ݿ��в��ҽ�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : ??????????????����
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_max_tranid(long *lTranid)
{ 
    EXEC SQL SELECT max(tran_id) INTO :lgTranid:igTmp
        FROM swt_tran_log;
    if(sqlca.sqlcode == SQLNOTFOUND) 
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        *lTranid = lgTranid;
        return(SUCCESS);	
    }	
}

/***************************************************************
 ** ������      : swDbselect_qid_revkey_tranlog
 ** ��  ��      : �����ݿ��в��ҽ�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_qid_revkey_tranlog(short iQid, char *aRevkey,
        struct swt_tran_log *psSwt_tran_log)
{ 
    igQid = iQid;
    strcpy(agKey,aRevkey);

    EXEC SQL SELECT * INTO :sgSwt_tran_log1.tran_id:igTmp,
         :sgSwt_tran_log1.tran_begin:igTmp,
         :sgSwt_tran_log1.tran_status:igTmp,
         :sgSwt_tran_log1.tran_end:igTmp,
         :sgSwt_tran_log1.tran_overtime:igTmp,
         :sgSwt_tran_log1.q_tran_begin:igTmp,
         :sgSwt_tran_log1.tc_tran_begin:igTmp,
         :sgSwt_tran_log1.q_target:igTmp,
         :sgSwt_tran_log1.resq_file:igTmp,
         :sgSwt_tran_log1.resq_offset:igTmp,
         :sgSwt_tran_log1.resq_len:igTmp,
         :sgSwt_tran_log1.resp_file:igTmp,
         :sgSwt_tran_log1.resp_offset:igTmp,
         :sgSwt_tran_log1.resp_len:igTmp,
         :sgSwt_tran_log1.rev_key:igTmp,
         :sgSwt_tran_log1.msghdkey:igTmp,
         :sgSwt_tran_log1.msghead:igTmp,
         :sgSwt_tran_log1.xastatus:igTmp
             FROM swt_tran_log WHERE  q_tran_begin = :igQid AND rev_key = :agKey;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        /* *psSwt_tran_log = sgSwt_tran_log1; */
        memcpy((char *)psSwt_tran_log,(char *)&sgSwt_tran_log1,sizeof(struct swt_tran_log));
        return(SUCCESS);	
    }	
}

/***************************************************************
 ** ������      : swDbdelete_swt_tran_log
 ** ��  ��      : �����ݿ���ɾ��������ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbdelete_swt_tran_log(long lTranid)
{
    lgTranid = lTranid;

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif    

    EXEC SQL DELETE FROM swt_tran_log WHERE tran_id = :lgTranid;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        psmShmidx_d->iRecinfile[SFTRANLOGID]--;
        return(SUCCESS);
    }
}

/***************************************************************
 ** ������      : swDbupdate_swt_tran_log
 ** ��  ��      : �����ݿ��и��½�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbupdate_swt_tran_log(long lTranid,struct swt_tran_log sSwt_tran_log)
{ 
    lgTranid = lTranid;
    /* sgSwt_tran_log1 = sSwt_tran_log; */
    memcpy((char *)&sgSwt_tran_log1,(char *)&sSwt_tran_log,sizeof(struct swt_tran_log));

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif

    EXEC SQL UPDATE swt_tran_log SET tran_id = :sgSwt_tran_log1.tran_id,
         tran_begin = :sgSwt_tran_log1.tran_begin,
         tran_status = :sgSwt_tran_log1.tran_status,
         tran_end = :sgSwt_tran_log1.tran_end ,
         tran_overtime=:sgSwt_tran_log1.tran_overtime,
         q_tran_begin = :sgSwt_tran_log1.q_tran_begin,
         tc_tran_begin=:sgSwt_tran_log1.tc_tran_begin,
         q_target = :sgSwt_tran_log1.q_target , 
         resq_file = :sgSwt_tran_log1.resq_file ,
         resq_offset = :sgSwt_tran_log1.resq_offset ,
         resq_len = :sgSwt_tran_log1.resq_len ,
         resp_file = :sgSwt_tran_log1.resp_file ,
         resp_offset = :sgSwt_tran_log1.resp_offset , 
         resp_len = :sgSwt_tran_log1.resp_len ,
         rev_key = :sgSwt_tran_log1.rev_key ,
         msghdkey = :sgSwt_tran_log1.msghdkey ,
         msghead = :sgSwt_tran_log1.msghead ,
         xastatus = :sgSwt_tran_log1.xastatus
             WHERE tran_id = :lgTranid;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        return(SUCCESS);	
    }
}

/***************************************************************
 ** ������      : swDbupdate_setstat_tranlog
 ** ��  ��      : �����ݿ��и��½�����ˮ��¼��״̬
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbupdate_setstat_tranlog(long lTranid,short iStatus)
{  
    lgTranid = lTranid;
    igStatus = iStatus;

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif

    EXEC SQL UPDATE swt_tran_log SET tran_status = :igStatus WHERE tran_id = :lgTranid;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        return(SUCCESS);
    }
}

/***************************************************************
 ** ������      : swDbinsert_swt_tran_log
 ** ��  ��      : �����ݿ��в��뽻����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbinsert_swt_tran_log(struct swt_tran_log sSwt_tran_log)
{
    /* sgSwt_tran_log1 = sSwt_tran_log; */
    memcpy((char *)&sgSwt_tran_log1,(char *)&sSwt_tran_log,sizeof(struct swt_tran_log));

#ifdef DB_INFORMIX    
    EXEC SQL BEGIN WORK;
#endif  
    /* modify by gf at 2004-04-26
       EXEC SQL INSERT INTO swt_tran_log VALUES (:sgSwt_tran_log1);
     */
    EXEC SQL INSERT INTO swt_tran_log VALUES 
        (:sgSwt_tran_log1.tran_id,
         :sgSwt_tran_log1.tran_begin,
         :sgSwt_tran_log1.tran_status,
         :sgSwt_tran_log1.tran_end,
         :sgSwt_tran_log1.tran_overtime,
         :sgSwt_tran_log1.q_tran_begin,
         :sgSwt_tran_log1.tc_tran_begin,
         :sgSwt_tran_log1.q_target,
         :sgSwt_tran_log1.resq_file,
         :sgSwt_tran_log1.resq_offset,
         :sgSwt_tran_log1.resq_len,
         :sgSwt_tran_log1.resp_file,
         :sgSwt_tran_log1.resp_offset,
         :sgSwt_tran_log1.resp_len,
         :sgSwt_tran_log1.rev_key,
         :sgSwt_tran_log1.msghdkey,
         :sgSwt_tran_log1.msghead,
         :sgSwt_tran_log1.xastatus);
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    { 
        EXEC SQL COMMIT WORK;
        psmShmidx_d->iRecinfile[SFTRANLOGID]++;
        return(SUCCESS);
    }               
}

/***************************************************************
 ** ������      : swDbinsert_swt_proc_log
 ** ��  ��      : �����ݿ��в��봦����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbinsert_swt_proc_log(struct swt_proc_log sSwt_proc_log)
{
    /* sgSwt_proc_log1 = sSwt_proc_log; */
    memcpy((char *)&sgSwt_proc_log1,(char *)&sSwt_proc_log,\
            sizeof(struct swt_proc_log));

#ifdef DB_INFORMIX    
    EXEC SQL BEGIN WORK;
#endif  
    /* modify by gf at 2004-04-26
       EXEC SQL INSERT INTO swt_proc_log VALUES (:sgSwt_proc_log1); 
     */
    EXEC SQL INSERT INTO swt_proc_log(tran_id,proc_step,proc_begin,q_target,proc_status,rev_mode,
            rev_fmtgrp,org_file,org_offset,org_len) VALUES 
        (:sgSwt_proc_log1.tran_id,
         :sgSwt_proc_log1.proc_step,
         :sgSwt_proc_log1.proc_begin,
         :sgSwt_proc_log1.q_target,
         :sgSwt_proc_log1.proc_status,
         :sgSwt_proc_log1.rev_mode,
         :sgSwt_proc_log1.rev_fmtgrp,
         :sgSwt_proc_log1.org_file,
         :sgSwt_proc_log1.org_offset,
         :sgSwt_proc_log1.org_len);
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    { 
        EXEC SQL COMMIT WORK;
        psmShmidx_d->iRecinfile[SFPROCLOGID]++;
        return(SUCCESS);
    }
}

/***************************************************************
 ** ������      : swDbselect_swt_proc_log
 ** ��  ��      : �����ݿ��в��Ҵ�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_swt_proc_log(long lTranid,short iProc_step,
        struct swt_proc_log *psSwt_proc_log)
{ 
    lgTranid = lTranid;
    igProc_step = iProc_step;

    EXEC SQL SELECT * INTO :sgSwt_proc_log1.tran_id:igTmp,
         :sgSwt_proc_log1.proc_step:igTmp,
         :sgSwt_proc_log1.proc_begin:igTmp,
         :sgSwt_proc_log1.q_target:igTmp,
         :sgSwt_proc_log1.proc_status:igTmp,
         :sgSwt_proc_log1.rev_mode:igTmp,
         :sgSwt_proc_log1.rev_fmtgrp:igTmp,
         :sgSwt_proc_log1.org_file:igTmp,
         :sgSwt_proc_log1.org_offset:igTmp,
         :sgSwt_proc_log1.org_len:igTmp
             FROM swt_proc_log
             WHERE tran_id = :lgTranid and proc_step = :igProc_step;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        /* *psSwt_proc_log = sgSwt_proc_log1; */
        memcpy((char *)psSwt_proc_log,(char *)&sgSwt_proc_log1,\
                sizeof(struct swt_proc_log));
        return(SUCCESS);	
    }	
}

/***************************************************************
 ** ������      : swDbselect_swt_proc_log_mrec
 ** ��  ��      : �����ݿ��в��Ҵ�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_swt_proc_log_mrec(long lTranid,
        struct swt_proc_log *psSwt_proc_log,short *iCount)
{ 
    lgTranid = lTranid;

    EXEC SQL DECLARE clCur CURSOR FOR 
        SELECT * FROM swt_proc_log WHERE tran_id = :lgTranid;
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    EXEC SQL OPEN clCur; 
    if( sqlca.sqlcode )
    {
        swDberror(NULL);
        return(FAIL);
    }

    for(;;)
    {
        memset((char *)&sgSwt_proc_log1,0x00,sizeof(struct swt_proc_log));
        EXEC SQL FETCH clCur INTO :sgSwt_proc_log1.tran_id:igTmp,
             :sgSwt_proc_log1.proc_step:igTmp,
             :sgSwt_proc_log1.proc_begin:igTmp,
             :sgSwt_proc_log1.q_target:igTmp,
             :sgSwt_proc_log1.proc_status:igTmp,
             :sgSwt_proc_log1.rev_mode:igTmp,
             :sgSwt_proc_log1.rev_fmtgrp:igTmp,
             :sgSwt_proc_log1.org_file:igTmp,
             :sgSwt_proc_log1.org_offset:igTmp,
             :sgSwt_proc_log1.org_len:igTmp;
        if(sqlca.sqlcode == SQLNOTFOUND)
            break;
        else if(sqlca.sqlcode)
        {
            swDberror(NULL);
            return(FAIL);
        }
        else
        {
            memcpy((char *)&(psSwt_proc_log[*iCount]), 
                    (char *)&sgSwt_proc_log1, sizeof(struct swt_proc_log));
            (*iCount) ++;
        }	
    }
    EXEC SQL CLOSE clCur;    
    return(SUCCESS);
}

/***************************************************************
 ** ������      : swDbdelete_swt_proc_log
 ** ��  ��      : �����ݿ���ɾ��������ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbdelete_swt_proc_log(long lTranid,short iProc_step)
{
    lgTranid = lTranid;
    igProc_step = iProc_step;

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif

    EXEC SQL DELETE FROM swt_proc_log
        WHERE  tran_id = :lgTranid and proc_step = :igProc_step;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        psmShmidx_d->iRecinfile[SFPROCLOGID] --; 
        return(SUCCESS);	
    }
}

/***************************************************************
 ** ������      : swDbupdate_swt_proc_log
 ** ��  ��      : �����ݿ��и��´�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbupdate_swt_proc_log(long lTranid,short iProc_step,
        struct swt_proc_log sSwt_proc_log)  
{ 
    lgTranid = lTranid;
    igProc_step = iProc_step;
    /* sgSwt_proc_log1 = sSwt_proc_log; */
    memset((char *)&sgSwt_proc_log1,0x00,sizeof(struct swt_proc_log));
    memcpy((char *)&sgSwt_proc_log1,(char *)&sSwt_proc_log,\
            sizeof(struct swt_proc_log));

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif

    EXEC SQL UPDATE swt_proc_log SET tran_id = :sgSwt_proc_log1.tran_id, 
         proc_step = :sgSwt_proc_log1.proc_step ,
         proc_begin = :sgSwt_proc_log1.proc_begin,
         q_target = :sgSwt_proc_log1.q_target , 
         proc_status = :sgSwt_proc_log1.proc_status ,
         rev_mode = :sgSwt_proc_log1.rev_mode ,
         rev_fmtgrp = :sgSwt_proc_log1.rev_fmtgrp ,
         org_file = :sgSwt_proc_log1.org_file ,
         org_offset = :sgSwt_proc_log1.org_offset , 
         org_len = :sgSwt_proc_log1.org_len 
             WHERE tran_id = :lgTranid and proc_step = :igProc_step;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        return(SUCCESS);	
    }
}

/***************************************************************
 ** ������      : swDbupdate_setstat_proclog
 ** ��  ��      : �����ݿ��и��´�����ˮ��¼��״̬
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbupdate_setstat_proclog(long lTranid,short iStatus)
{  
    lgTranid = lTranid;
    igStatus = iStatus;

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif

    EXEC SQL UPDATE swt_proc_log SET proc_status = :igStatus
        WHERE  tran_id = :lgTranid;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        return(SUCCESS);	
    }
}

/***************************************************************
 ** ������      : swDbdelete_swt_proc_log_mrec
 ** ��  ��      : �����ݿ���ɾ����ˮ�Ŷ�Ӧ�����д�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbdelete_swt_proc_log_mrec(long lTranid)
{ 
    lgTranid = lTranid;

    EXEC SQL SELECT count(*) into :igCount:igTmp FROM swt_proc_log 
        WHERE tran_id = :lgTranid;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    if(igCount == 0)
        return(SUCCESS);

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif 

    EXEC SQL DELETE FROM swt_proc_log WHERE tran_id = :lgTranid;
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        psmShmidx_d->iRecinfile[SFPROCLOGID] -= igCount;
        return(SUCCESS);	
    }
}

/***************************************************************
 ** ������      : swDbdelete_proc_tran_q
 ** ��  ��      : �����ݿ���ɾ��ָ������ָ����������Ӧ�����д�����ˮ
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbdelete_proc_tran_q(long lTranid,short iQid)
{
    lgTranid = lTranid;
    igQid = iQid;

    EXEC SQL SELECT count(*) into :igCount:igTmp FROM swt_proc_log 
        WHERE tran_id = :lgTranid and q_target = :igQid;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    if(igCount == 0)
        return(SUCCESS);

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif

    EXEC SQL DELETE FROM swt_proc_log
        WHERE tran_id = :lgTranid and q_target = :igQid;
    if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else
    {
        EXEC SQL COMMIT WORK;
        psmShmidx_d->iRecinfile[SFPROCLOGID] -= igCount;
        return(SUCCESS);
    }
}

/***************************************************************
 ** ������      : swDbselect_max_proclog_tran
 ** ��  ��      : �����ݿ��в���ĳ�ʽ��׵��������ˮ
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_max_proclog_tran(long lTranid,struct swt_proc_log *psSwt_proc_log)
{
    lgTranid = lTranid;

    EXEC SQL SELECT * INTO :sgSwt_proc_log1.tran_id:igTmp,
         :sgSwt_proc_log1.proc_step:igTmp,
         :sgSwt_proc_log1.proc_begin:igTmp,
         :sgSwt_proc_log1.q_target:igTmp,
         :sgSwt_proc_log1.proc_status:igTmp,
         :sgSwt_proc_log1.rev_mode:igTmp,
         :sgSwt_proc_log1.rev_fmtgrp:igTmp,
         :sgSwt_proc_log1.org_file:igTmp,
         :sgSwt_proc_log1.org_offset:igTmp,
         :sgSwt_proc_log1.org_len:igTmp
             FROM   swt_proc_log
             WHERE  proc_step in(
                     SELECT max(proc_step) FROM swt_proc_log WHERE tran_id = :lgTranid)
             AND tran_id = :lgTranid;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        /* *psSwt_proc_log = sgSwt_proc_log1; */
        memcpy((char *)psSwt_proc_log,
                (char *)&sgSwt_proc_log1, sizeof(struct swt_proc_log));
        return(SUCCESS);	
    }	
}

/***************************************************************
 ** ������      : swDbselect_swt_tran_log_all
 ** ��  ��      : �����ݿ��в������еĽ�����ˮ
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_swt_tran_log_all(struct swt_tran_log *psSwt_tran_log,short *iCount)
{
    EXEC SQL DECLARE clCur2 CURSOR FOR 
        SELECT * FROM swt_tran_log;
    if( sqlca.sqlcode )
    {
        swDberror(NULL);
        return(FAIL);
    }
    EXEC SQL OPEN clCur2; 
    if( sqlca.sqlcode )
    {
        swDberror(NULL);
        return(FAIL);
    }
    for(;;)
    {
        memset((char *)&sgSwt_tran_log1,0x00,sizeof(struct swt_tran_log));
        EXEC SQL FETCH clCur2 INTO :sgSwt_tran_log1.tran_id:igTmp,
             :sgSwt_tran_log1.tran_begin:igTmp,
             :sgSwt_tran_log1.tran_status:igTmp,
             :sgSwt_tran_log1.tran_end:igTmp,
             :sgSwt_tran_log1.tran_overtime:igTmp,
             :sgSwt_tran_log1.q_tran_begin:igTmp,
             :sgSwt_tran_log1.tc_tran_begin:igTmp,
             :sgSwt_tran_log1.q_target:igTmp,
             :sgSwt_tran_log1.resq_file:igTmp,
             :sgSwt_tran_log1.resq_offset:igTmp,
             :sgSwt_tran_log1.resq_len:igTmp,
             :sgSwt_tran_log1.resp_file:igTmp,
             :sgSwt_tran_log1.resp_offset:igTmp,
             :sgSwt_tran_log1.resp_len:igTmp,
             :sgSwt_tran_log1.rev_key:igTmp,
             :sgSwt_tran_log1.msghdkey:igTmp,
             :sgSwt_tran_log1.msghead:igTmp,
             :sgSwt_tran_log1.xastatus:igTmp ;
        if(sqlca.sqlcode == SQLNOTFOUND)
            break;
        else if(sqlca.sqlcode)
        {
            swDberror(NULL);
            return(FAIL);
        }
        else
        {
            memcpy((char *)&(psSwt_tran_log[*iCount]), 
                    (char *)&sgSwt_tran_log1, sizeof(struct swt_tran_log));
            (*iCount) ++;
        }	
    }
    EXEC SQL CLOSE clCur2;
    return(SUCCESS);
}

/***************************************************************
 ** ������      : swDbselect_max_tran_begin
 ** ��  ��      : �����ݿ��в������Ľ���ʱ��
 ** ��  ��      : xujun
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_max_tran_begin(long *lTran_begin)
{  
    EXEC SQL SELECT max(tran_begin) INTO :sgSwt_tran_log1.tran_begin:igTmp                  
        FROM   swt_tran_log;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        *lTran_begin = sgSwt_tran_log1.tran_begin;
        return(SUCCESS);           
    }
}

/***************************************************************
 ** ������      : swDbselect_max_tran_id
 ** ��  ��      : �����ݿ��в���ͬһ����ʱ��������ID
 ** ��  ��      : xujun
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_max_tran_id(long lTran_begin,long *lTran_id)
{  
    lgTran_begin = lTran_begin;

    EXEC SQL SELECT max(tran_id) INTO :sgSwt_tran_log1.tran_id:igTmp
        FROM swt_tran_log WHERE tran_begin = :lgTran_begin ;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        *lTran_id = sgSwt_tran_log1.tran_id;
        return(SUCCESS);           
    }	
}

/***************************************************************
 ** ������      : swDbselect_overtime_min_tranlog
 ** ��  ��      : �����ݿ��в��ҳ�ʱ����С������ˮ��¼
 ** ��  ��      : xujun
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_overtime_min_tranlog(long lTime,struct swt_tran_log *psSwt_tran_log)
{
    lgTime = lTime;

    EXEC SQL SELECT * INTO :sgSwt_tran_log1.tran_id:igTmp,
         :sgSwt_tran_log1.tran_begin:igTmp,
         :sgSwt_tran_log1.tran_status:igTmp,
         :sgSwt_tran_log1.tran_end:igTmp,
         :sgSwt_tran_log1.tran_overtime:igTmp,
         :sgSwt_tran_log1.q_tran_begin:igTmp,
         :sgSwt_tran_log1.tc_tran_begin:igTmp,
         :sgSwt_tran_log1.q_target:igTmp,
         :sgSwt_tran_log1.resq_file:igTmp,
         :sgSwt_tran_log1.resq_offset:igTmp,
         :sgSwt_tran_log1.resq_len:igTmp,
         :sgSwt_tran_log1.resp_file:igTmp,
         :sgSwt_tran_log1.resp_offset:igTmp,
         :sgSwt_tran_log1.resp_len:igTmp,
         :sgSwt_tran_log1.rev_key:igTmp,
         :sgSwt_tran_log1.msghdkey:igTmp,
         :sgSwt_tran_log1.msghead:igTmp,
         :sgSwt_tran_log1.xastatus:igTmp
             FROM swt_tran_log WHERE tran_id IN(
                     SELECT min(tran_id) FROM swt_tran_log 
                     WHERE  tran_status = 0 AND tran_overtime <> 0 
                     AND tran_overtime < :lgTime);
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        memcpy((char *)psSwt_tran_log, 
                (char *)&sgSwt_tran_log1, sizeof(struct swt_tran_log));
        return(SUCCESS);           
    }	
}

/***************************************************************
 ** ������      : swDbselect_swt_tran_log_clear
 ** ��  ��      : �����ݿ��в���ָ��ʱ����ǰ�Ľ�����ˮ��¼
 ** ��  ��      : xujun
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_swt_tran_log_clear(long lTran_begin,struct swt_tran_log *psSwt_tran_log)
{  
    lgTran_begin = lTran_begin;

    EXEC SQL SELECT * INTO :sgSwt_tran_log1.tran_id:igTmp,
         :sgSwt_tran_log1.tran_begin:igTmp,
         :sgSwt_tran_log1.tran_status:igTmp,
         :sgSwt_tran_log1.tran_end:igTmp,
         :sgSwt_tran_log1.tran_overtime:igTmp,
         :sgSwt_tran_log1.q_tran_begin:igTmp,
         :sgSwt_tran_log1.tc_tran_begin:igTmp,
         :sgSwt_tran_log1.q_target:igTmp,
         :sgSwt_tran_log1.resq_file:igTmp,
         :sgSwt_tran_log1.resq_offset:igTmp,
         :sgSwt_tran_log1.resq_len:igTmp,
         :sgSwt_tran_log1.resp_file:igTmp,
         :sgSwt_tran_log1.resp_offset:igTmp,
         :sgSwt_tran_log1.resp_len:igTmp,
         :sgSwt_tran_log1.rev_key:igTmp,
         :sgSwt_tran_log1.msghdkey:igTmp,
         :sgSwt_tran_log1.msghead:igTmp,
         :sgSwt_tran_log1.xastatus:igTmp
             FROM swt_tran_log WHERE  tran_id IN(
                     SELECT min(tran_id) FROM swt_tran_log WHERE tran_begin < :lgTran_begin);                   
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        memcpy((char *)psSwt_tran_log, 
                (char *)&sgSwt_tran_log1, sizeof(struct swt_tran_log));
        return(SUCCESS);           
    }
}

/***************************************************************
 ** ������      : swDbselect_swt_proc_log_clear
 ** ��  ��      : �����ݿ��в���ָ��ʱ����ǰ�Ľ�����ˮ��¼
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbselect_swt_proc_log_clear(long lTran_begin,struct swt_proc_log *psSwt_proc_log)
{  
    lgTran_begin = lTran_begin;

    EXEC SQL SELECT min(tran_id) INTO :lgTranid:igTmp
        FROM swt_proc_log WHERE proc_begin < :lgTran_begin;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swVdebug(0,"select min(tran_id) from swt_proc_log error[%ld]",sqlca.sqlcode);
        swDberror(NULL);
        return(FAIL);
    }

    EXEC SQL SELECT * INTO :sgSwt_proc_log1.tran_id:igTmp,
         :sgSwt_proc_log1.proc_step:igTmp,
         :sgSwt_proc_log1.proc_begin:igTmp,
         :sgSwt_proc_log1.q_target:igTmp,
         :sgSwt_proc_log1.proc_status:igTmp,
         :sgSwt_proc_log1.rev_mode:igTmp,
         :sgSwt_proc_log1.rev_fmtgrp:igTmp,
         :sgSwt_proc_log1.org_file:igTmp,
         :sgSwt_proc_log1.org_offset:igTmp,
         :sgSwt_proc_log1.org_len:igTmp
             FROM swt_proc_log WHERE proc_step IN(
                     SELECT max(proc_step) FROM swt_proc_log WHERE tran_id = :lgTranid)
             AND tran_id = :lgTranid;                   
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swVdebug(0,"select max(proc_step) from swt_proc_log error[%ld]",sqlca.sqlcode);
        swDberror(NULL);
        return(FAIL);
    }
    else
    {
        memcpy((char *)psSwt_proc_log, 
                (char *)&sgSwt_proc_log1, sizeof(struct swt_proc_log));
        return(SUCCESS);           
    }	 
}
/*add by zcd 20141229*/
#endif

/***************************************************************
 ** ������      : swShm2db_swt_tran_proc_log
 ** ��  ��      : ��ָ�����׵Ľ�����ˮ�ʹ�����ˮ�ɹ����ڴ��Ƶ����ݿ�
 ** ��  ��      : nh
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swShm2db_swt_tran_proc_log(long lTranid)    /*need review again*/
{
    int i,j,ilRc,ilLlink,ilRlink,ilTmp=0;
    short ilQorg,ilQdes;
    struct swt_tran_log slSwt_tran_log;
    struct swt_proc_log slSwt_proc_log;
#if 0

    if (swShmP(iSEMNUM_TRANLOG) == FAIL)
        return(FAIL);

    /* ת�ƽ�����ˮ */ 
    i = psmShmidx_d->sIdx_tran_log.iSlink;
    while (i)
    {
        if (smShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.tran_id == lTranid)
        {
            if (ilTmp == 0) ilTmp = 1;
            memcpy((char *)&slSwt_tran_log, (char *)(&(smShmbuf_d.psShm_tran_log
                            [i-1].sSwt_tran_log)), sizeof(struct swt_tran_log));
            /*add by zcd 20141229 DB_SUPPORT*/
#ifdef DB_SUPPORT
            ilRc = swDbinsert_swt_tran_log(slSwt_tran_log);
            if(ilRc)
                return(FAIL);
#endif
            /* ͳ�ƶ˿�Դ�������� */
            ilQorg = smShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.q_tran_begin;
            ilQdes = smShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.q_target;
            for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
            {
                if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQorg)
                {
                    smShmbuf_s.psSwt_sys_queue[j].tranbegin_num -= 1;
                    if (ilQdes<=0) break;
                }
                if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
                {
                    smShmbuf_s.psSwt_sys_queue[j].traning_num -= 1;
                    if (ilQorg<=0) break;
                }
            }             
            /* ͳ���ڹ����ڴ浱�еĽ�����ˮ�� */
            psmShmidx_d->iRecinshm[SFTRANLOGID] --;

            ilLlink = smShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iLlink;
            ilRlink = smShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iRlink;
            if ((psmShmidx_d->sIdx_tran_log.iSlink == i) &&
                    (psmShmidx_d->sIdx_tran_log.iElink == i)) 
            {
                psmShmidx_d->sIdx_tran_log.iSlink = 0;
                psmShmidx_d->sIdx_tran_log.iElink = 0;
            }
            else
            {
                if (psmShmidx_d->sIdx_tran_log.iSlink == i)
                {
                    if(ilRlink > 0)
                        smShmbuf_d.psShm_tran_log[ilRlink-1].sLinkinfo.iLlink = 0;
                    psmShmidx_d->sIdx_tran_log.iSlink = ilRlink;
                }
                else if (psmShmidx_d->sIdx_tran_log.iElink == i)
                {
                    if(ilLlink > 0)
                        smShmbuf_d.psShm_tran_log[ilLlink-1].sLinkinfo.iRlink = 0;
                    psmShmidx_d->sIdx_tran_log.iElink = ilLlink;
                }
                else
                {
                    smShmbuf_d.psShm_tran_log[ilLlink-1].sLinkinfo.iRlink = ilRlink;
                    smShmbuf_d.psShm_tran_log[ilRlink-1].sLinkinfo.iLlink = ilLlink;
                }
            }
            psmShmidx_d->sIdx_tran_log.iFlink = i;
            smShmbuf_d.psShm_tran_log[i-1].sLinkinfo.cTag = 0;    
            break;        
        }
        i = smShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iRlink;
    }
    /* ת�ƴ�����ˮ */
    i = psmShmidx_d->sIdx_proc_log.iSlink;
    while (i)
    {
        if (smShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log.tran_id == lTranid)
        {
            if(ilTmp == 0 ) ilTmp = 1;

            memset((char *)&slSwt_proc_log,0x00,sizeof(struct swt_proc_log));
            memcpy((char *)&slSwt_proc_log, (char *)&(smShmbuf_d.
                        psShm_proc_log[i-1].sSwt_proc_log), sizeof(struct swt_proc_log));
            /*add by zcd 20141229 DB_SUPPORT*/
#ifdef DB_SUPPORT
            ilRc = swDbinsert_swt_proc_log(slSwt_proc_log);
            if(ilRc)
                return(FAIL);
#endif
            /* ͳ���ڹ����ڴ浱�еĴ�����ˮ�� */
            psmShmidx_d->iRecinshm[SFPROCLOGID] --;

            ilLlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iLlink;
            ilRlink = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;

            if ((psmShmidx_d->sIdx_proc_log.iSlink == i) && 
                    (psmShmidx_d->sIdx_proc_log.iElink == i))
            {
                psmShmidx_d->sIdx_proc_log.iSlink = 0;
                psmShmidx_d->sIdx_proc_log.iElink = 0;
            }
            else
            {
                if (psmShmidx_d->sIdx_proc_log.iSlink == i)
                {
                    if (ilRlink > 0)
                        smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = 0; 
                    psmShmidx_d->sIdx_proc_log.iSlink = ilRlink;
                }
                else
                    if (psmShmidx_d->sIdx_proc_log.iElink == i)
                    {
                        if (ilLlink > 0)
                            smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = 0;
                        psmShmidx_d->sIdx_proc_log.iElink = ilLlink;
                    }
                    else
                    {
                        smShmbuf_d.psShm_proc_log[ilLlink-1].sLinkinfo.iRlink = ilRlink;
                        smShmbuf_d.psShm_proc_log[ilRlink-1].sLinkinfo.iLlink = ilLlink;
                    }
            }
            psmShmidx_d->sIdx_proc_log.iFlink = i;
            smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.cTag = 0; 
        }
        i = smShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
    }

    if (swShmV(iSEMNUM_TRANLOG) == FAIL)
        return(FAIL);
#endif 
    return(SUCCESS);
}

/*add by zcd 20141229 DB_SUPPORT*/
#ifdef DB_SUPPORT
/***************************************************************
 ** ������      : swDbfilefind_tran_log
 ** ��  ��      : �����ݿ��в���ָ���ļ�����TRANLOG
 ** ��  ��      : xujun
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbfilefind_tran_log(char *aFilename)
{
    strcpy(agFilename,aFilename);

    EXEC SQL SELECT max(tran_id) INTO :sgSwt_tran_log1.tran_id:igTmp 
        FROM swt_tran_log 
        WHERE resq_file = :agFilename OR resp_file = :agFilename;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {    
        return(SUCCESS);           
    }	  
}

/***************************************************************
 ** ������      : swDbfilefind_proc_log
 ** ��  ��      : �����ݿ��в���ָ���ļ�����PROC_LOG
 ** ��  ��      : xujun
 ** ��������    : 2002/09/10
 ** ����޸�����: 2002/09/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbfilefind_proc_log(char *aFilename)
{
    strcpy(agFilename,aFilename);

    EXEC SQL SELECT max(tran_id) INTO :sgSwt_proc_log1.tran_id:igTmp
        FROM swt_proc_log WHERE org_file = :agFilename;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        return(FAIL);
    }
    else
    {    
        return(SUCCESS);           
    }	  
}

#endif

#ifdef DB_SUPPORT
/***************************************************************
 ** ������      : swDbupdate_swt_tran_log_end
 ** ��  ��      : �����ݿ��и��½�����ˮ��¼
 ** ��  ��      : cy
 ** ��������    : 2004/03/24
 ** ����޸�����: 2004/03/24
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ�,-1:����
 ***************************************************************/
int swDbupdate_swt_tran_log_end(long lTranid,struct swt_tran_log sSwt_tran_log)
{ 
    lgTranid = lTranid;
    /* sgSwt_tran_log1 = sSwt_tran_log; */
    memcpy((char *)&sgSwt_tran_log1,(char *)&sSwt_tran_log,sizeof(struct swt_tran_log));

#ifdef DB_INFORMIX
    EXEC SQL BEGIN WORK;
#endif

    EXEC SQL UPDATE swt_tran_log SET tran_id = :sgSwt_tran_log1.tran_id,
         tran_begin = :sgSwt_tran_log1.tran_begin,
         tran_status = :sgSwt_tran_log1.tran_status,
         tran_end = :sgSwt_tran_log1.tran_end ,
         tran_overtime=:sgSwt_tran_log1.tran_overtime,
         q_tran_begin = :sgSwt_tran_log1.q_tran_begin,
         tc_tran_begin=:sgSwt_tran_log1.tc_tran_begin,
         q_target = :sgSwt_tran_log1.q_target , 
         resq_file = :sgSwt_tran_log1.resq_file ,
         resq_offset = :sgSwt_tran_log1.resq_offset ,
         resq_len = :sgSwt_tran_log1.resq_len ,
         resp_file = :sgSwt_tran_log1.resp_file ,
         resp_offset = :sgSwt_tran_log1.resp_offset , 
         resp_len = :sgSwt_tran_log1.resp_len ,
         rev_key = :sgSwt_tran_log1.rev_key ,
         msghdkey = :sgSwt_tran_log1.msghdkey ,
         msghead = :sgSwt_tran_log1.msghead ,
         xastatus = :sgSwt_tran_log1.xastatus
             /*           WHERE tran_id = :lgTranid and tran_status <> iTRNREVING;*/
             WHERE tran_id = :lgTranid and tran_status <> 3;
    if(sqlca.sqlcode == SQLNOTFOUND)
        return(SHMNOTFOUND);
    else if(sqlca.sqlcode)
    {
        swDberror(NULL);
        EXEC SQL ROLLBACK WORK;
        return(FAIL);
    }
    else 
    {
        EXEC SQL COMMIT WORK;
        return(SUCCESS);	
    }
}

#endif

#ifndef HASH
/*begin add by gf for rev result inform at 2004-04-07*/
/***************************************************************
 ** ������      : swShmselect_route_d_last
 ** ��  ��      : ���Ҷ�Ӧ��·�ɱ����Ƿ����к�Ϊ[9999]�ҷ��������ĵ����
 ** ��  ��      :
 ** ��������    : 2001/05/31
 ** ����޸�����: 2001/05/31
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmselect_route_d_last(short iQid, char *aTran_code)
{
    int i;
    int il_count = 0;
    long ll_offset = -1;
    _WITH_ITME_START;

    for(i = 0;i < psmShmidx_s->sIdx_sys_route_m.iCount;i++)
    {
        if ((iQid == smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.q_id) &&
                (strcmp(aTran_code, 
                        smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.trancode) == 0))
        {
            ll_offset = smShmbuf_s.psShm_sys_route_m[i].lOffset;
            _WITH_ITME_END;
            break;
        }
    }
    if (ll_offset == -1)
        for(i = 0;i < psmShmidx_s->sIdx_sys_route_m.iCount;i++)
        {
            if ((iQid == smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.q_id) &&
                    (strcmp("0", 
                            smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.trancode) == 0))
            {
                ll_offset = smShmbuf_s.psShm_sys_route_m[i].lOffset;
                _WITH_ITME_END;
                break;
            }
        }  

    if ((ll_offset < 0) || (ll_offset >= psmShmidx_s->sIdx_sys_route_g.iCount))
        return SHMNOTFOUND;

    il_count = smShmbuf_s.psShm_sys_route_g[ll_offset].iCount;
    ll_offset = smShmbuf_s.psShm_sys_route_g[ll_offset].lOffset;

    if ((ll_offset < 0) || (ll_offset + il_count > psmShmidx_s->sIdx_sys_route_d.iCount))
        return SHMNOTFOUND;  

    for(i=0;i<il_count;i++)
    {
        if(smShmbuf_s.psSwt_sys_route_d[ll_offset + i].route_id == 9999)
        {
            if(smShmbuf_s.psSwt_sys_route_d[ll_offset+i].rev_mode == 4)
                return SUCCESS;
        }
    }    
    if(i==il_count)
        return FAIL;
}    
/*end add*/
#else
/***************************************************************
 ** ������      : swShmselect_route_d_last(HASH)
 ** ��  ��      : ���Ҷ�Ӧ��·�ɱ����Ƿ����к�Ϊ[9999]�ҷ��������ĵ����
 ** ��  ��      : baiqj
 ** ��������    : 2015/05/15
 ** ����޸�����: 2015/05/15
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmselect_route_d_last(short iQid, char *aTran_code)
{
    int iRc = 0, i;
    int il_count = 0;
    long ll_offset = -1;
    struct shm_sys_route_m p_in;
    struct shm_sys_route_m *p_out;
    _WITH_ITME_START;

    p_in.sSwt_sys_route_m.q_id = iQid;
    strcpy(p_in.sSwt_sys_route_m.trancode,aTran_code);

    iRc = shm_swt_sys_route_m(&p_in, &p_out);
    if(0 == iRc)
    {
        ll_offset = p_out->lOffset;
        _WITH_ITME_END;
    }

    if (ll_offset == -1)
    {
        memset(&p_in.sSwt_sys_route_m.trancode,0x00,sizeof(p_in.sSwt_sys_route_m.trancode));
        strcpy(p_in.sSwt_sys_route_m.trancode,"0");
        iRc = shm_swt_sys_route_m(&p_in, &p_out);
        if(0 == iRc)
        {
            ll_offset = p_out->lOffset;
            _WITH_ITME_END;
        }
    }

    if ((ll_offset < 0) || (ll_offset >= psmShmidx_s->sIdx_sys_route_g.iCount))
        return SHMNOTFOUND;

    il_count = smShmbuf_s.psShm_sys_route_g[ll_offset].iCount;
    ll_offset = smShmbuf_s.psShm_sys_route_g[ll_offset].lOffset;

    if ((ll_offset < 0) || (ll_offset + il_count > psmShmidx_s->sIdx_sys_route_d.iCount))
        return SHMNOTFOUND;  

    for(i=0;i<il_count;i++)
    {
        if(smShmbuf_s.psSwt_sys_route_d[ll_offset + i].route_id == 9999)
        {
            if(smShmbuf_s.psSwt_sys_route_d[ll_offset+i].rev_mode == 4)
                return SUCCESS;
        }
    }    
    if(i==il_count)
        return FAIL;
}   
#endif

    short	
sw_put_tranlog(struct swt_tran_log *tran_log)
{
    int ilRc;
    FILE  *fp;
    char buf[1024];
    if ( (fp = fopen(asw_tran_log_file, "a+")) == NULL)
    {
        printf("error\n");
        return FAIL;
    }
    memset(buf, 0x00, sizeof(buf));
    memcpy(buf+1, tran_log ,sizeof(struct swt_tran_log));
    buf[0] = '1';
    ilRc = fwrite( buf, (sizeof(struct swt_tran_log) + 1), 1 , fp);
    if (ilRc )
    {
        return (FAIL);
    }
    fclose(fp);
    return (SUCCESS);
}

    short
sw_put_proclog(struct swt_proc_log *proc_log)
{
    int ilRc;
    FILE  *fp;
    char buf[256];
    if ( (fp = fopen(asw_proc_log_file, "a+")) == NULL)
    {
        printf("error\n");
        return FAIL;
    }
    memset(buf, 0x00, sizeof(buf));
    memcpy(buf+1, proc_log ,sizeof(struct swt_proc_log));
    buf[0] = '1';
    ilRc = fwrite( buf, (sizeof(struct swt_proc_log) + 1), 1 , fp);
    if (ilRc )
    {
        return (FAIL);
    }
    fclose(fp);
    return (SUCCESS);
}

    short
sw_init_logfile()
{
    sprintf(asw_tran_log_file, "%s/log/shm/tranlog", getenv("SWITCH_DIR"));
    sprintf(asw_proc_log_file, "%s/log/shm/proclog", getenv("SWITCH_DIR"));
    return SUCCESS;
}

    short
sw_get_max_tran_id(long *max_tranid)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llTran_id_tmp = 0;
    char  alBuf[1024];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = fopen(asw_tran_log_file, "r")) != SUCCESS)
    {
        //   swVdebug(0,"S5245: [����/��������] swShmopenfile����,������=%d,���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            //   swVdebug(0,"S5250: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            fclose(fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
                if ( slSwt_tran_log.tran_id > llTran_id_tmp )
                {
                    llTran_id_tmp = slSwt_tran_log.tran_id;
                    ilTmp = TRUE;
                    /* break; */
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    fclose(fp);

    if (ilTmp)
    {
        *max_tranid = llTran_id_tmp;
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }  
}

    short
sw_get_max_saf_id(long *max_tranid)
{

    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    long  llOffset = 0;
    long  llTran_id_tmp = 0;
    char  alBuf[1024];
    FILE  *fp;
    struct swt_proc_log slSwt_proc_log;

    if ((ilRc = fopen(asw_tran_log_file, "r")) != SUCCESS)
    {
        //   swVdebug(0,"S5245: [����/��������] swShmopenfile����,������=%d,���ļ�����",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_proc_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            //   swVdebug(0,"S5250: [����/��������] swShmgetfile()����,������=%d,���ļ�����",ilRc);
            fclose(fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {    
                memcpy((char *)&slSwt_proc_log, alBuf + 1, ilSize - 1);
                if ( slSwt_proc_log.tran_id > llTran_id_tmp )
                {
                    llTran_id_tmp = slSwt_proc_log.tran_id;
                    ilTmp = TRUE;
                    /* break; */
                }  
            }  
        }  
        if (ilRc == SHMFILEEND)  break; /* not found */
    }

    fclose(fp);

    if (ilTmp)
    {
        *max_tranid = llTran_id_tmp;
        return (SUCCESS);
    }
    else
    {
        return (SHMNOTFOUND);
    }  
}

    short
sw_shm_create()
{
    return 0;
}

/***************************************************************
 ** ������      : sw_init_free_list
 ** ��  ��      : ��ʼ��������ˮ���нڵ�
 ** ��  ��      : 
 ** ��������    : 2015/01/14
 ** ����޸�����: 2015/01/14
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short sw_init_free_list()
{
    int i, k;
    psmShmidx_d = psgShmidx_d ;
    for(k=0; k< SHM_TRANLOG_BUCKET_NUM; k++)
    {
        smShmbuf_d.psShm_tran_log[k] = (struct shm_tran_log *)
            ( (char*)psmShmidx_d + psmShmidx_d->sIdx_tran_log[k].lOffset);
        /*
           smShmbuf_d.psShm_rev_saf = (struct shm_rev_saf *)
           ( (char*)psmShmidx_d + psmShmidx_d->sIdx_rev_saf.lOffset);
         */

        for( i=0; i < (sgSwt_sys_config.iMaxtranlog/SHM_TRANLOG_BUCKET_NUM) - 1 ; i++ )
        {
            smShmbuf_d.psShm_tran_log[k][i].sLinkinfo.iRlink = i + 2;
        }
        psgShmidx_d->sIdx_tran_log[k].iFlink  = 1;
        smShmbuf_d.psShm_tran_log[k][i].sLinkinfo.iRlink = 0;
    }

    smShmbuf_d.psShm_proc_log = (struct shm_proc_log *)
        ( (char*)psmShmidx_d + psmShmidx_d->sIdx_proc_log.lOffset);
    for( i=0; i < sgSwt_sys_config.iMaxproclog - 1 ; i++ )
    {
        smShmbuf_d.psShm_proc_log[i].sLinkinfo.iRlink = i + 2;
    }
    smShmbuf_d.psShm_proc_log[i].sLinkinfo.iRlink = 0;
    psgShmidx_d->sIdx_proc_log.iFlink  = 1;

    /*
       for( i=0; i < sgSwt_sys_config.iMaxsaflog - 1 ; i++ )
       {
       smShmbuf_d.psShm_rev_saf[i].sLinkinfo.iRlink = i + 1;
       }
       smShmbuf_d.psShm_rev_saf[i].sLinkinfo.iRlink = 0;
     */
    return SUCCESS;
}

/* 
   function name :swisShmNonAttachProc 
input: shmid
function: check if share memory has no process attached,
and if YES, release the share memory
�������ܣ��ͷ�ָ��shmid�Ĺ����ڴ�
����    ��shmid �����ڴ�id
����ֵ  ��-1 �� ��������
-2 :  ������ɾ��
0 �� �ͷųɹ�
����    ������
ʱ��    ��20150312
 */
int   swReleaseZeroAttachedShm(int shmid)
{
    struct shmid_ds buf;
    int ilRc;

    if(shmid != -1)
    {
        ilRc = shmctl(shmid, IPC_STAT, &buf);
        if(ilRc < 0)
        {
            swVdebug(0,"��ȡshmid[%d]��״̬����\n",shmid);
            return -1;
        }

        if(0 == buf.shm_nattch)
        {
            ilRc = shmctl(shmid, IPC_RMID, NULL);
            if(ilRc < 0)
            {
                swVdebug(0,"ɾ��ָ����shmid[%d]��ָ���Ĺ����ڴ����", shmid);
                return -1;
            }
            swVdebug(4, "shm%d���ͷ�..", shmid);
        }
        else
        {
            swVdebug(4,"ָ����shmid[%d]��ָ��Ĺ����ڴ滹��[%d]���������ӣ������ͷ�!",shmid, buf.shm_nattch);
            return 0;
        }


        return 0;
    }
}

#ifndef HASH
/* add by dyw at 2015.03.27 begin PSBC_V1.0 */
/************************************************************
 **��  ��: ���� 
 **��  ��: iQid      - ������� Դ������
 aTrancode - ������
 **����ֵ: 0  - �ɹ� û�г���
 -1 - ʧ�� ������
 ************************************************************/
int swShmGetTranDebugLevel(short iQid, char *aTrancode)
{
    int i;
    for (i=0; i<psmShmidx_s->sIdx_sys_tran.iCount; i++)
    {
        if (smShmbuf_s.psSwt_sys_tran[i].sys_id == iQid &&
                !strncmp(smShmbuf_s.psSwt_sys_tran[i].tran_code, aTrancode, sizeof(smShmbuf_s.psSwt_sys_tran[i].tran_code)))
        {
            return smShmbuf_s.psSwt_sys_tran[i].debug_level;
            break;
        }
    }
    return(0);
}
/* add by dyw at 2015.03.27 end PSBC_V1.0 */
#else
/* add by gengling at 2015.05.08 begin PSBC_V1.0 */
/************************************************************
 **��  ��: ������־���� 
 **��  ��: iQid      - ������� Դ������
           aTrancode - ������
 **����ֵ: p_out->debug_level - �ɹ� ��־����
           3                  - û���� Ĭ����־����3
 ************************************************************/
int swShmGetTranDebugLevel(short iQid, char *aTrancode)
{
    int iRc = 0;
    struct swt_sys_tran p_in;
    struct swt_sys_tran *p_out;

    p_in.sys_id = iQid;
    strcpy(p_in.tran_code, aTrancode);

    iRc = shm_swt_sys_tran(&p_in, &p_out);
    if(iRc)
    {
        swVdebug(1, "shm_swt_sys_tran error iRc[%d]", iRc);
        return(3);
    }

    swVdebug(5, "swShmGetTranDebugLevel sys_id[%d],tran_code[%s],tran_name[%s],priority[%d],status[%s],tranning_max[%ld],debug_level[%d],tranning_num[%ld]",
            p_out->sys_id, p_out->tran_code, p_out->tran_name, p_out->priority, p_out->status,
            p_out->tranning_max, p_out->debug_level, p_out->tranning_num); 

    return p_out->debug_level;
}
/* add by gengling at 2015.05.08 end PSBC_V1.0 */
#endif

/************add by wanghao 20150409 PSBC_V1.0****/
int clearShm()
{

    int ilRc;
    char buf[100];
    int shmid_delete;

    ilRc = swShmcheck();
    if(ilRc)
    {
        swVdebug(2,"check failed\n");
        return (-1);
    }

    while(1)
    {
        memset(buf, 0x00, sizeof(buf));
        ilRc = readShmid(buf);
        if(ilRc)
        {
            swVdebug(0,"��ȡ�ļ�shmid.txt����\n");
            return -1;
        }

        shmid_delete = atoi(buf);

        if(shmid_delete == psmShmidx_d->iShmid)
        {
            break;
        }

        delShmid(strlen(buf));

        //swVdebug(0,"shmid is:%d",shmid_delete);
        ilRc = swReleaseZeroAttachedShm(shmid_delete);
        if(ilRc == -1)
        {
            swVdebug(0,"ɾ����̬�����ڴ�[%d]���ɹ���",shmid_delete);
        }
        else if(ilRc == -2)
        {
            swVdebug(0,"��̬�����ڴ�[%d]���н��������ţ�����ɾ����",shmid_delete);
        }
        else
        {
            swVdebug(3,"��̬�����ڴ�[%d]ɾ���ɹ�!",shmid_delete);
            //swVdebug(0,"��̬�����ڴ�[%d]ɾ���ɹ�!",shmid_delete);
        }

    }
    return 0;
}

int writeShmid(int shmid)
{
    FILE *fp;
    char fileName[100];
    char *envPath;

    envPath = getenv("SWITCH_DIR");
    if(envPath == NULL) 
    {
        swVdebug(0,"getenv����\n");
        return -1;
    }

    memset(fileName, 0x00, sizeof(fileName));
    sprintf(fileName,"%s/tmp/shmid.txt",envPath);

    fp = fopen(fileName,"a");
    if(NULL == fp)
    {
        swVdebug(0,"open shmid.txt failed for write");
        return -1;
    }

    fprintf(fp,"%d\n", shmid);

    fclose(fp);
    return 0;

}

int readShmid(char *buf)
{

    FILE *fp;
    char fileName[100];
    char *envPath;
    char tmp[100];

    envPath = getenv("SWITCH_DIR");
    if(envPath == NULL) 
    {
        swVdebug(0,"getenv����\n");
        return -1;
    }

    memset(fileName, 0x00, sizeof(fileName));
    sprintf(fileName,"%s/tmp/shmid.txt",envPath);

    fp = fopen(fileName,"r");
    if(NULL == fp)
    {
        swVdebug(0,"open shmid.txt failed for read");
        return -1;
    }

    memset(tmp, 0x00, sizeof(tmp));
    if((fgets(tmp, sizeof(tmp), fp)) == NULL)
    {
        if(feof(fp))
        {
            return -2; //�����ļ���β
        }
    }

    strncpy(buf, tmp, strlen(tmp));
    //buf[strlen(buf) - 1 ] = '\0';   //����س���
    //swVdebug(0,"in readShmid buf is:%s",buf);

    fclose(fp);
    return 0;
}

int delShmid(int size)
{
    FILE *fp;
    char fileName[100];
    char *envPath;

    char readBuf[2048];
    char tmpBuf[2048];

    envPath = getenv("SWITCH_DIR");
    if(envPath == NULL) 
    {
        swVdebug(0,"getenv����\n");
        return -1;
    }

    memset(fileName, 0x00, sizeof(fileName));
    sprintf(fileName,"%s/tmp/shmid.txt",envPath);

    fp = fopen(fileName,"rb+");
    if(NULL == fp)
    {
        swVdebug(0,"open shmid.txt failed for delete");
        return -1;
    }

    memset(readBuf, 0x00, sizeof(readBuf));
    memset(tmpBuf, 0x00, sizeof(tmpBuf));

    fread(readBuf,1,sizeof(readBuf),fp);

    strcpy(tmpBuf,readBuf+size);
    fseek(fp,0,0);
    int fd = fileno(fp);
    ftruncate(fd,strlen(readBuf+size));
    write(fd, tmpBuf, strlen(tmpBuf));

    fclose(fp);

    return 0;

}
/****************end add by wanghao 20150409***********/

