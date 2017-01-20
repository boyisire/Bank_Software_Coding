/****************************************************************/
/* ģ����    ��INIT                                           */
/* ģ������    �������ڴ��ʼ��ģ��                             */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ����־��                                         */
/* ��������    ��2001/6/7                                       */
/* ����޸����ڣ�2001/6/7                                       */
/* ģ����;    ����ʼ�������ڴ���źŵ�                         */
/* ��ģ���а������º���������˵����                             */
/*			 ��1��int main();                       */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼:                                                    */
/*  2001.6.7 ����                                               */
/*  2015.05 ��swt_sys_tran swt_sys_fmt_m swt_sys_fmt_g 
    swt_sys_route_m�ṹ֧��HASH ����ԭ�е�swt_sys_tran 
    swt_sys_fmt_m swt_sys_route_m�ṹע��                       */
/****************************************************************/
#include "switch.h"
#include <assert.h>
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
EXEC SQL INCLUDE sqlca;
#ifdef DB_POSTGRESQL
EXEC SQL INCLUDE swDbstruct.h;
#endif
#ifdef DB_DB2
EXEC SQL INCLUDE '../inc/swDbstruct.h';
#endif
#ifdef DB_INFORMIX
EXEC SQL INCLUDE 'swDbstruct.h';
#endif
#ifdef DB_ORACLE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_SYBASE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#else
#include "swDbstruct.h"
#endif

#include "swShm.h"
#include "swShmGroup.h"

#define SFTRANLOGID	0
#define SFPROCLOGID	1
#define MAX_SEC_TRANID 5000  /*ÿ���������*/

static int ilOptflag;		/* ������ʶ��0-��ʼ�� / 1-ˢ�� */
static int ilShmid_old;
static int ilShmid_del_d,ilShmid_del_s,ilSemid_del;
static long llTran_id_shm=0;

void swExit(int);

extern int swGrpGetProfile(char *FileName,char *Section,char *Index,char *GetValue);
int swGetGroupHostInfo2(int groupid,PB_GROUPINFO *pGrpInfo);

static int swGettranid( long *lserno );
static int swGetsafid( long *lserno );
static int swGetchoose1();
static int swGetchoose2();
extern int _swShmcheck();
extern short swShm_swt_sys_imf();
extern short swShm_swt_sys_queue();
extern short swShm_swt_sys_matchport();	/* added by fzj at 2002.02.28 */
extern short swShm_swt_sys_task();
extern short swShm_swt_sys_8583();
extern short swShm_swt_sys_8583E();
extern short swShm_swt_sys_code();
extern short swShm_swt_sys_config();
extern short swShm_swt_sys_other();
extern short swShm_swt_sys_fmt();
extern short swShm_swt_sys_route();
extern short swShm_swt_sys_tran();      /* add by gengling at 2015.03.19 one line PSBC_V1.0 */
extern short swShm_swt_sys_tran_ctrl(); /* add by gengling at 2015.05.04 one line PSBC_V1.0 */
extern short swShm_swt_sys_fmt_m();     /* add by gengling at 2015.05.12 one line PSBC_V1.0 */
extern short swShm_swt_sys_fmt_g();     /* add by gengling at 2015.05.15 one line PSBC_V1.0 */
extern short swShm_swt_sys_route_m();   /* add by baiqj at 2015.05.15 one line PSBC_V1.0    */

/**************add by wanghao 2015049******/
int lock_set(int fd,int type);
/*************end add **********************/


int main(int argc, char **argv)
{
    long llSize;
    int i,j,ilRc,ilShmid,ilSemid,ilFlag=-1, k;
    int ilCount_queue_old,ilCount_task_old;
    int ilCount_tran_old; /* add by gengling at 2015.03.18 ilCount_tran_old PSBC_V1.0 */
    char *alShmtmp_s,*alShmtmp_d,alTime[15],alRefreshtimes[5];
    struct swt_sys_queue *pslSwt_sys_queue;
    struct swt_sys_task *pslSwt_sys_task;
    struct swt_sys_tran *pslSwt_sys_tran; /*add by gengling at 2015.03.18 one line PSBC_V1.0 */
    long llSerno,llTime;
    struct tm *T;
    char alSize[10],alFile[100];
    FILE *fp = NULL;
    char clLoc_flag;      /* add by nh 20020923 */
#ifdef HASH
    int indexSize = 0;
    int fmt_m_indexSize = 0;
    int fmt_g_indexSize = 0;
    int route_m_indexSize = 0;
#endif

    /************add by wanghao 20150408******/

    if((argc == 2) && (strcmp(argv[1],"-f") == 0))
    {
        ilRc = clearShm();
        if(ilRc)
        {
            printf("����̬�����ڴ�ʧ�ܣ�\n");
            exit(-1);
        }

        printf("����̬�����ڴ�ɹ���\n");

        return 0;
    }

    /***************end add*************************/

    /* add by zjj 2004.02.26 */
    union semun {
        int val;
        struct semid_ds *buf;
        unsigned short *array;
    } semunArg;
    unsigned short semunArray = 1;
    int ilSemval = 1;
    /*end add */

    signal(SIGTERM,SIG_IGN);

    /* ���õ��Գ������� */
    memset(agDebugfile,0x00,sizeof(agDebugfile));
    strcpy(agDebugfile,"swInit.debug");

    /* ȡ��DEBUG��־ */
    if ((cgDebug = _swDebugflag("swInit")) == FAIL)
    {
        printf("�޷�ȡ��DEBUG��־!\n");
        exit(FAIL);
    }
    swLogInit("swInit", NULL);
    swVdebug(2,"S0000: ȡDebug��־ok!");

    /* ��ϵͳ������[swt_sys_config] */
    if (swInitcfgload(&sgSwt_sys_config) != SUCCESS)
    {
        swVdebug(0,"S0010: [����/����] ��ϵͳ������[swt_sys_config]����");
        exit(FAIL);
    } 
    swVdebug(2,"S0000: ��ϵͳ������swt_sys_config�ɹ�!");

    ilShmid_del_d = -1;
    ilShmid_del_s = -1;
    ilSemid_del = -1;

    /* ���㶯̬�����ڴ��С */
    llSize = /*sizeof(sw_slock_array_t) +*/ sizeof(sw_slock_array_t)+ sizeof(struct shmidx_d) + 
        sgSwt_sys_config.iMaxtranlog * sizeof(struct shm_tran_log) +
        sgSwt_sys_config.iMaxproclog * sizeof(struct shm_proc_log) +
        sgSwt_sys_config.iMaxsaflog * sizeof(struct shm_rev_saf);
    printf("llSize is :%d\n",llSize);
    printf("\n������̬�����ڴ�: %ldM\n", llSize/(1024*1024) ); 
    /* ������̬�����ڴ� */
    ilShmid = shmget((key_t)sgSwt_sys_config.iShmkey,(size_t)llSize,IPC_CREAT|IPC_EXCL|0666);
    if (ilShmid >= 0)
    {
        swVdebug(2,"S0000: �����ڴ治����,�����ɹ�!");
        ilShmid_del_d = ilShmid;
        /* �ò�����ʶΪ��ʼ������ */
        ilOptflag = 0;

        /* �����ݿ� */
#ifdef DB_SUPPORT
        if((ilRc = swDbopen()))
        {
            swVdebug(0,"S0020: [����/���ݿ�] �����ݿ����");
            swExit(FAIL);
        }
        swVdebug(2,"S0000: �����ݿ�ɹ�!");
#endif

        /* ���ᶯ̬�����ڴ� */
        if ((alShmtmp_d = shmat(ilShmid,(char *)0,SHM_RND)) == (char *)-1)
        {
            swVdebug(0,"S0030: [����/ϵͳ����] shmat()����,errno=%d,���ᶯ̬�����ڴ����",errno);
            swExit(FAIL);
        }
        swVdebug(2,"S0000: ���ӹ����ڴ�[%ld]�ɹ�!",ilShmid);

        /* �ö�̬�����ڴ�Ŀ¼�� */
        memset(alShmtmp_d,0x00,llSize);
        /*	memset(alShmtmp_d, 0x00, llSize);*/

        /*==begin==add by dyw, initialize the lock==*/
        pg_lock_array = (sw_slock_array_t*)( alShmtmp_d); /*add by dyw, 20150120, for the use of spin_lock*/
        for(k=0 ; k < iSEMNUM; k++)
        {
            S_INIT_LOCK(pg_lock_array->lock + k);
        }
        /*==end==add by dyw, initialize the lock==*/

        psgShmidx_d = (struct shmidx_d *)( alShmtmp_d + sizeof(sw_slock_array_t) );  /*changed by dyw, for the use of spin lock*/

        memset(&(psgShmidx_d->aMemo[37]),0x00,sizeof(alSize));
        sprintf(alSize,"%-9ld",llSize);
        memcpy(&(psgShmidx_d->aMemo[37]),alSize,sizeof(alSize));

        /* swt_tran_log�� */
        for(k=0; k < SHM_TRANLOG_BUCKET_NUM; k++)  
        {
            /*add by dyw, for multiple tranlog list*/
            psgShmidx_d->sIdx_tran_log[k].lOffset = sizeof(struct shmidx_d)+
                sizeof(struct shm_tran_log) *((sgSwt_sys_config.iMaxtranlog/SHM_TRANLOG_BUCKET_NUM)*k);
            psgShmidx_d->sIdx_tran_log[k].iSlink = 0;
            psgShmidx_d->sIdx_tran_log[k].iElink = 0;
            psgShmidx_d->sIdx_tran_log[k].iFlink = 1;
            psgShmidx_d->sIdx_tran_log[k].max_used_log = 0;
            psgShmidx_d->sIdx_tran_log[k].used_log = 0;
        }

        /* swt_proc_log�� */
        psgShmidx_d->sIdx_proc_log.lOffset =  sizeof(struct shmidx_d)+
            sgSwt_sys_config.iMaxtranlog * sizeof(struct shm_tran_log);
        psgShmidx_d->sIdx_proc_log.iSlink = 0;
        psgShmidx_d->sIdx_proc_log.iElink = 0;
        psgShmidx_d->sIdx_proc_log.iFlink = 1;

        /* delete by xujun 2002.09.12 swt_rev_saf�� */
        psgShmidx_d->sIdx_rev_saf.lOffset = psgShmidx_d->sIdx_proc_log.lOffset 
            + sgSwt_sys_config.iMaxproclog * sizeof(struct shm_proc_log);
        psgShmidx_d->sIdx_rev_saf.iSlink = 0;
        psgShmidx_d->sIdx_rev_saf.iElink = 0;
        psgShmidx_d->sIdx_rev_saf.iFlink = 1;

        sw_init_free_list();

        /* �ý���ˢ�±����ڴ�״̬Ϊ����ˢ�� 0:δˢ�� 1:��ˢ�� 2:��ˢ�� */
        psgShmidx_d->iShmid = -1;

        /* SHMSHM,SHMFILE��¼������ */
        psgShmidx_d->iRecinshm[0] = 0;
        psgShmidx_d->iRecinshm[1] = 0;
        psgShmidx_d->iRecinshm[2] = 0;
        psgShmidx_d->iRecinfile[0] = 0;
        psgShmidx_d->iRecinfile[1] = 0;
        psgShmidx_d->iRecinfile[2] = 0;

        /* ��������ʼ���źŵ� */
        ilSemid = semget(IPC_PRIVATE,iSEMNUM,IPC_CREAT|IPC_EXCL|0666);
        if (ilSemid < 0)
        {
            swVdebug(0,"S0040: [����/ϵͳ����] semget()����,errno=%d,�����źŵ�ʧ��",errno);
            swExit(FAIL);
        }
        swVdebug(2,"S0000: �����źŵƳɹ�!");
        ilSemid_del = ilSemid;
        /*add by zjj 2004.02.26 for 64λ��*/
        semunArg.array = &semunArray;
        semunArg.val = ilSemval;
        /*end add by zjj   */
        for (i=0;i<iSEMNUM;i++)
        {
            /*      ilRc = semctl(ilSemid,i,SETVAL,1);  */
            ilRc = semctl(ilSemid,i,SETVAL,semunArg);
            if (ilRc == FAIL)
            {
                swVdebug(0,"S0050: [����/ϵͳ����] semctl()����,errno=%d, ��ʼ���źŵ�ʧ��",errno);
                swExit(FAIL);
            }
        }
        swVdebug(2,"S0000: �źŵƸ�ֵ��ʼ���ɹ�!");
        psgShmidx_d->iSemid = ilSemid;

        /* ��ʼ��ƽ̨��ˮ�� */
        /*    
              printf("��ʼ��ƽ̨��ˮ��...\n");
              if(swGettranid(&llSerno) == FAIL)
              {
              swVdebug(0,"S0060: [����/��������] swGettranid()����,������=-1, ��ʼ��ƽ̨��ˮ�ų���");
              swExit(FAIL);
              }
              psgShmidx_d->lTranid = llSerno;
         */    

        /* ��ʼ��SAF��ˮ�� */
        /*    
              printf("��ʼ��SAF��ˮ��...\n");
              if(swGetsafid(&llSerno) == FAIL)
              {
              swVdebug(0,"S0070: [����/��������] swGetsafid()����,������=-1, ��ʼ��SAF��ˮ�ų���");
              swExit(FAIL);
              }
              psgShmidx_d->lSafid = llSerno;
         */    
    }
    else if (errno == EEXIST)
    {
        swVdebug(2,"S0000: �ù����ڴ��Ѿ�����");
        printf("ˢ����......\n");
        /* �ò�����־Ϊˢ�� */
        ilOptflag = 1;

        /* ���Ӷ�̬�����ڴ� */
        ilShmid = shmget((key_t)sgSwt_sys_config.iShmkey,0,IPC_EXCL|0666);
        if (ilShmid < 0)
        {
            swVdebug(0,"S0080: [����/ϵͳ����] shmget()����,errno=%d, ���Ӷ�̬�����ڴ�ʧ��,shmget:[key=%x]",errno,sgSwt_sys_config.iShmkey);
            swExit(FAIL);
        }

        if ((alShmtmp_d = shmat(ilShmid,(char *)0,SHM_RND)) == (char *)-1)
        {
            swVdebug(0,"S0090: [����/ϵͳ����] shmat()����,errno=%d,���Ӷ�̬�����ڴ�ʧ��",errno);
            swExit(FAIL);
        } 
        swVdebug(2,"S0000: ���ӹ����ڴ�[%ld]�ɹ�!",ilShmid);

        psgShmidx_d =  (struct shmidx_d *) (  alShmtmp_d + sizeof(sw_slock_array_t) ); /*modified by dyw, 20150121*/

        /* �ý���ˢ�±�״̬δ��ˢ��,���ȴ�ϵͳ�趨ʱ�� */
        ilShmid_old = psgShmidx_d->iShmid;
        /* psgShmidx_d->iShmid = -1;            delte by wanghao 20150324 for double shm PSBC_V1.0****/
        sleep(sgSwt_sys_config.iShmwaittime);



        /* ˢ��ǰ����׼��̬����Ϣ */
        if ((alShmtmp_s = shmat(ilShmid_old,(char *)0,SHM_RND)) == (char *)-1)
        {
            swVdebug(0,"S0100: [����/ϵͳ����] shmat()����,errno=%d,���Ӿ�̬�����ڴ�ʧ��",errno);
            swExit(FAIL);
        } 
        psgShmidx_s = (struct shmidx_s *)alShmtmp_s;

        /* ����swt_sys_queue��Ϣ */
        ilCount_queue_old = psgShmidx_s->sIdx_sys_queue.iCount;
        //printf("iCount is:%d\n",ilCount_queue_old);
        sgShmbuf_s.psSwt_sys_queue = (struct swt_sys_queue *)
            (alShmtmp_s + psgShmidx_s->sIdx_sys_queue.lOffset);
        pslSwt_sys_queue = (struct swt_sys_queue *)malloc
            (ilCount_queue_old * sizeof(struct swt_sys_queue));
        if (pslSwt_sys_queue == NULL)
        {
            swVdebug(0,"S0110: [����/����] ��̬�����ڴ�ʧ��");
            swExit(FAIL);
        }
        memcpy(pslSwt_sys_queue, sgShmbuf_s.psSwt_sys_queue,
                ilCount_queue_old * sizeof(struct swt_sys_queue));

        /* ����swt_sys_task��Ϣ */
        ilCount_task_old = psgShmidx_s->sIdx_sys_task.iCount;
        sgShmbuf_s.psSwt_sys_task = (struct swt_sys_task *)
            (alShmtmp_s + psgShmidx_s->sIdx_sys_task.lOffset);
        pslSwt_sys_task = (struct swt_sys_task *)malloc
            (ilCount_task_old * sizeof(struct swt_sys_task));
        if (pslSwt_sys_task == NULL)
        {
            swVdebug(0,"S0120: [����/����] ��̬�����ڴ�ʧ��");
            swExit(FAIL);
        }
        memcpy(pslSwt_sys_task, sgShmbuf_s.psSwt_sys_task,
                ilCount_task_old * sizeof(struct swt_sys_task));

#ifndef HASH
        /* add by gengling at 2015.03.18 begin PSBC_V1.0 */
        /* ����swt_sys_tran��Ϣ */
        ilCount_tran_old = psgShmidx_s->sIdx_sys_tran.iCount;
        sgShmbuf_s.psSwt_sys_tran = (struct swt_sys_tran *)
            (alShmtmp_s + psgShmidx_s->sIdx_sys_tran.lOffset);
        pslSwt_sys_tran = (struct swt_sys_tran *)malloc
            (ilCount_tran_old * sizeof(struct swt_sys_tran));
        if (pslSwt_sys_tran == NULL)
        {
            swVdebug(0,"S0120: [����/����] ��̬�����ڴ�ʧ��");
            swExit(FAIL);
        }
        memcpy(pslSwt_sys_tran, sgShmbuf_s.psSwt_sys_tran,
                ilCount_tran_old * sizeof(struct swt_sys_tran));
        /* add by gengling at 2015.03.18 end PSBC_V1.0 */
#endif
    }
    else
    {
        swVdebug(0,"S0130: [����/ϵͳ����] shmget()����,errno=%d, ������̬�����ڴ�ʧ��",errno);
        exit(FAIL);
    }

    /* �����ݿ�������ļ� */
    printf("װ��ϵͳ��׼���[SWIMF.CFG]...\n"); 
    if ((ilRc = swShm_swt_sys_imf()) != 0) swExit(FAIL);
    printf("װ��Ӧ�ö˿ڱ�[SWAPPS.CFG]...\n");
    if ((ilRc = swShm_swt_sys_queue()) != 0) swExit(FAIL);
    /* === begin of added by fzj at 2002.02.28 === */
    printf("װ�뽻�׶˿ڹ�����[SWMATCH_PORT.CFG]...\n");
    if ((ilRc = swShm_swt_sys_matchport()) != 0) swExit(FAIL);
    /* === end of added by fzj at 2002.02.28 === */
    printf("װ����������[SWTASK.CFG]...\n");
    if ((ilRc = swShm_swt_sys_task()) != 0) swExit(FAIL);
    printf("װ��ISO8583���ñ�[SWISO8583.CFG]...\n");
    if ((ilRc = swShm_swt_sys_8583()) != 0) swExit(FAIL);
    if ((ilRc = swShm_swt_sys_8583E()) != 0) swExit(FAIL);
    printf("װ��ϵͳ�����[SWCODE.CFG]...\n");
    if ((ilRc = swShm_swt_sys_code()) != 0) swExit(FAIL);
    printf("װ��ϵͳ������[SWCONFIG.CFG]...\n");
    if ((ilRc = swShm_swt_sys_config()) != 0) swExit(FAIL);
    printf("װ���ⲿ�ڵ��[SWOTHER.CFG]...\n");
    if ((ilRc = swShm_swt_sys_other()) != 0) swExit(FAIL);
    printf("װ���ʽת�����ñ�[SWFORMAT.CFG]...\n");
    if ((ilRc = swShm_swt_sys_fmt()) != 0) swExit(FAIL);
    printf("װ��·�����ñ�[SWROUTE.CFG]...\n");
    if ((ilRc = swShm_swt_sys_route()) != 0) swExit(FAIL);
#ifndef HASH
    /* add by gengling at 2015.03.19 begin PSBC_V1.0 */
    printf("װ�뽻���������Ʊ�[SWTRAN_CTL.CFG]...\n");
    if ((ilRc = swShm_swt_sys_tran()) != 0) swExit(FAIL);
    /* add by gengling at 2015.03.19 end PSBC_V1.0 */
#endif
#ifdef HASH
    printf("װ�뽻���������Ʊ�hash[SWTRAN_CTL.CFG]...\n");
    if ((ilRc = swShm_swt_sys_tran_ctrl(&indexSize, NULL)) != 0) swExit(FAIL);
    printf("װ���ʽת����������hash[SWMATCH_FMT.CFG]...\n");
    if ((ilRc = swShm_swt_sys_fmt_m(&fmt_m_indexSize, NULL)) != 0) swExit(FAIL);
    printf("װ���ʽת�����hash[*FMT.CFG]...\n");
    if ((ilRc = swShm_swt_sys_fmt_g(&fmt_g_indexSize, NULL)) != 0) swExit(FAIL);
    printf("װ��·����������hash[SWMATCH_ROUTE.CFG]...\n");
    if ((ilRc = swShm_swt_sys_route_m(&route_m_indexSize, NULL)) != 0) swExit(FAIL);
#endif

    /* ���Ϊˢ��״̬���ָ�ˢ��ǰ׼��̬����Ϣ */
    if (ilOptflag == 1)
    {
        /* ��ɾ��ԭ���ľ�̬�����ڴ� */
        /* ilRc = shmctl(ilShmid_old,IPC_RMID,0);
           if(ilRc)
           {
           swVdebug(0,"S0140: [����/ϵͳ����] shmctl()����,errno=%d, %s",errno,strerror(errno));
           swExit(FAIL);
           }
           delete by wanghao20150324 for double shm PSBC_v1.0*/
        /* �ָ�swt_sys_queue��Ϣ */
#if 1 
        for (i=0; i<sgLcmidx.iCount_swt_sys_queue; i++)
        {
            for (j=0; j<ilCount_queue_old; j++)
            {
                if (sgLcmidx.psSwt_sys_queue[i].q_id == pslSwt_sys_queue[j].q_id)
                {
                    sgLcmidx.psSwt_sys_queue[i].port_status = 
                        pslSwt_sys_queue[j].port_status;
                    /* === begin of added by fzj at 2002.02.28 === */
                    sgLcmidx.psSwt_sys_queue[i].tranbegin_num = 
                        pslSwt_sys_queue[j].tranbegin_num;
                    sgLcmidx.psSwt_sys_queue[i].traning_num = 
                        pslSwt_sys_queue[j].traning_num;
                    /* === end of added by fzj at 2002.02.28 === */
                    break;
                }    
            }      
        }        
#endif
        free(pslSwt_sys_queue);

        /* �ָ�swt_sys_task��Ϣ */  
        for (i=0; i<sgLcmidx.iCount_swt_sys_task; i++)
        {
            for (j=0; j<ilCount_task_old; j++)
            {
                if (strcmp(sgLcmidx.psSwt_sys_task[i].task_name,
                            pslSwt_sys_task[j].task_name) == 0)
                {
                    sgLcmidx.psSwt_sys_task[i].pid = pslSwt_sys_task[j].pid;
                    strcpy(sgLcmidx.psSwt_sys_task[i].task_status,
                            pslSwt_sys_task[j].task_status);
                    sgLcmidx.psSwt_sys_task[i].start_time = pslSwt_sys_task[j].start_time;
                    sgLcmidx.psSwt_sys_task[i].restart_num = pslSwt_sys_task[j].restart_num;
                    break;
                }
            }  
        }    
        free(pslSwt_sys_task);
    }

    /* ���㾲̬�����ڴ��С */
    llSize = sizeof(struct shmidx_s) +
        sgLcmidx.iCount_swt_sys_imf * sizeof(struct swt_sys_imf) +
        sgLcmidx.iCount_swt_sys_queue * sizeof(struct swt_sys_queue) +
        /* added by fzj at 2002.02.28 */
        sgLcmidx.iCount_swt_sys_matchport * sizeof(struct swt_sys_matchport) +
        /* added by fzj at 2002.02.28 */
        sgLcmidx.iCount_swt_sys_task * sizeof(struct swt_sys_task) +
        sgLcmidx.iCount_swt_sys_8583 * sizeof(struct swt_sys_8583) +
        sgLcmidx.iCount_swt_sys_8583E * sizeof(struct swt_sys_8583E) +
        sgLcmidx.iCount_swt_sys_code * sizeof(struct swt_sys_code) +
        sgLcmidx.iCount_swt_sys_config * sizeof(struct swt_sys_config) +
        sgLcmidx.iCount_swt_sys_other * sizeof(struct swt_sys_other) +
        sgLcmidx.iCount_swt_sys_fmt_m * sizeof(struct shm_sys_fmt_m) +
        sgLcmidx.iCount_swt_sys_fmt_g * sizeof(struct shm_sys_fmt_g) +
        sgLcmidx.iCount_swt_sys_fmt_d * sizeof(struct swt_sys_fmt_d) +  
        sgLcmidx.iCount_swt_sys_route_m * sizeof(struct shm_sys_route_m) +
        sgLcmidx.iCount_swt_sys_route_g * sizeof(struct shm_sys_route_g) +
        /* mod by gengling at 2015.03.26 one line ;->+ PSBC_V1.0 */
        sgLcmidx.iCount_swt_sys_route_d * sizeof(struct swt_sys_route_d) +
#ifndef HASH
        /* add by gengling at 2015.03.26 begin PSBC_V1.0 */
        sgLcmidx.iCount_swt_sys_tran * sizeof(struct swt_sys_tran);
        /* add by gengling at 2015.03.26 end PSBC_V1.0 */
#else
        + indexSize + fmt_m_indexSize + fmt_g_indexSize + route_m_indexSize;
#endif

#ifndef HASH
    /*2009-9-15 13:50 ADD BY PC*/

    swVdebug(0,"sgLcmidx.iCount_swt_sys_imf         =[%d]",         sgLcmidx.iCount_swt_sys_imf      );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_queue       =[%d]",         sgLcmidx.iCount_swt_sys_queue    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_matchport   =[%d]",         sgLcmidx.iCount_swt_sys_matchport);
    swVdebug(0,"sgLcmidx.iCount_swt_sys_task        =[%d]",         sgLcmidx.iCount_swt_sys_task     );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_8583        =[%d]",         sgLcmidx.iCount_swt_sys_8583     );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_8583E       =[%d]",         sgLcmidx.iCount_swt_sys_8583E    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_code        =[%d]",         sgLcmidx.iCount_swt_sys_code     );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_config      =[%d]",         sgLcmidx.iCount_swt_sys_config   );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_other       =[%d]",         sgLcmidx.iCount_swt_sys_other    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_fmt_m       =[%d]",         sgLcmidx.iCount_swt_sys_fmt_m    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_fmt_g       =[%d]",         sgLcmidx.iCount_swt_sys_fmt_g    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_fmt_d       =[%d]",         sgLcmidx.iCount_swt_sys_fmt_d    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_route_m     =[%d]",         sgLcmidx.iCount_swt_sys_route_m  );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_route_g     =[%d]",         sgLcmidx.iCount_swt_sys_route_g  );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_route_d     =[%d]",         sgLcmidx.iCount_swt_sys_route_d  );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_tran        =[%d]",         sgLcmidx.iCount_swt_sys_tran     );
    /*2009-9-15 13:50 ADD BY PC*/
#endif



    /* �����µľ�̬�����ڴ� */
    ilShmid = shmget(IPC_PRIVATE,(size_t)llSize,IPC_CREAT|IPC_EXCL|0666);
    if (ilShmid < 0)
    {
        swVdebug(0,"S0150: [����/ϵͳ����] shmget()����,errno=%d %s llSize = [%ld]",errno,strerror(errno),llSize);
        ilOptflag = 0;
        swExit(FAIL);
    }



    /*******************add by wanghao 20150408 PSBC_V1.0*********************/

    if(ilOptflag == 1)
    {
        ilRc = writeShmid(ilShmid);
        if(ilRc)
        {
            printf("��¼��̬�����ڴ�id����\n");
        }
    }

    /*******************end add**************************************/



    ilShmid_del_s = ilShmid;
    if ((alShmtmp_s = shmat(ilShmid, (char *)0, SHM_RND)) == (char *)-1)
    {
        swVdebug(0,"S0160: [����/ϵͳ����] shmat()����,errno=%d %s",errno,strerror(errno));
        ilOptflag = 0;
        swExit(FAIL);
    }  
    memset(alShmtmp_s,0x00,llSize);
    psgShmidx_s = (struct shmidx_s *)alShmtmp_s;


    /* ����̬�����ڴ��Сд�붯̬�����ڴ�Ŀ¼�� */
    memset(&(psgShmidx_d->aMemo[47]),0x00,sizeof(alSize));
    sprintf(alSize,"%-9ld",llSize);
    memcpy(&(psgShmidx_d->aMemo[47]),alSize,sizeof(alSize));

    /* �þ�̬�����ڴ�Ŀ¼�� */
    /* swt_sys_imf */
    psgShmidx_s->sIdx_sys_imf.lOffset = sizeof(struct shmidx_s);
    psgShmidx_s->sIdx_sys_imf.iCount = sgLcmidx.iCount_swt_sys_imf;

    /* swt_sys_queue */
    psgShmidx_s->sIdx_sys_queue.lOffset = psgShmidx_s->sIdx_sys_imf.lOffset
        + psgShmidx_s->sIdx_sys_imf.iCount * sizeof(struct swt_sys_imf);
    psgShmidx_s->sIdx_sys_queue.iCount = sgLcmidx.iCount_swt_sys_queue;

    /* === begin of added by fzj at 2002.02.28 === */
    /* swt_sys_matchport */                 
    psgShmidx_s->sIdx_sys_matchport.lOffset = psgShmidx_s->sIdx_sys_queue.lOffset
        + psgShmidx_s->sIdx_sys_queue.iCount * sizeof(struct swt_sys_queue);
    psgShmidx_s->sIdx_sys_matchport.iCount = sgLcmidx.iCount_swt_sys_matchport;
    /* === end of added by fzj at 2002.02.28 === */

    /* swt_sys_task */                 
    psgShmidx_s->sIdx_sys_task.lOffset = psgShmidx_s->sIdx_sys_matchport.lOffset
        + psgShmidx_s->sIdx_sys_matchport.iCount * sizeof(struct swt_sys_matchport);
    psgShmidx_s->sIdx_sys_task.iCount = sgLcmidx.iCount_swt_sys_task;

    /* swt_sys_8583 */
    psgShmidx_s->sIdx_sys_8583.lOffset = psgShmidx_s->sIdx_sys_task.lOffset
        + psgShmidx_s->sIdx_sys_task.iCount * sizeof(struct swt_sys_task);
    psgShmidx_s->sIdx_sys_8583.iCount = sgLcmidx.iCount_swt_sys_8583;

    /* swt_sys_8583E */
    psgShmidx_s->sIdx_sys_8583E.lOffset = psgShmidx_s->sIdx_sys_8583.lOffset
        + psgShmidx_s->sIdx_sys_8583.iCount * sizeof(struct swt_sys_8583);
    psgShmidx_s->sIdx_sys_8583E.iCount = sgLcmidx.iCount_swt_sys_8583E;

    /* swt_sys_code */
    psgShmidx_s->sIdx_sys_code.lOffset = psgShmidx_s->sIdx_sys_8583E.lOffset
        + psgShmidx_s->sIdx_sys_8583E.iCount * sizeof(struct swt_sys_8583E);
    psgShmidx_s->sIdx_sys_code.iCount = sgLcmidx.iCount_swt_sys_code;

    /* swt_sys_config */
    psgShmidx_s->sIdx_sys_config.lOffset = psgShmidx_s->sIdx_sys_code.lOffset
        + psgShmidx_s->sIdx_sys_code.iCount * sizeof(struct swt_sys_code);
    psgShmidx_s->sIdx_sys_config.iCount = sgLcmidx.iCount_swt_sys_config;

    /* swt_sys_other */
    psgShmidx_s->sIdx_sys_other.lOffset = psgShmidx_s->sIdx_sys_config.lOffset
        + psgShmidx_s->sIdx_sys_config.iCount * sizeof(struct swt_sys_config);
    psgShmidx_s->sIdx_sys_other.iCount = sgLcmidx.iCount_swt_sys_other;

#ifndef HASH
    /* swt_sys_fmt_m */
    psgShmidx_s->sIdx_sys_fmt_m.lOffset = psgShmidx_s->sIdx_sys_other.lOffset
        + psgShmidx_s->sIdx_sys_other.iCount * sizeof(struct swt_sys_other);
    psgShmidx_s->sIdx_sys_fmt_m.iCount = sgLcmidx.iCount_swt_sys_fmt_m;

    /* swt_sys_fmt_g */
    psgShmidx_s->sIdx_sys_fmt_g.lOffset = psgShmidx_s->sIdx_sys_fmt_m.lOffset
        + psgShmidx_s->sIdx_sys_fmt_m.iCount * sizeof(struct shm_sys_fmt_m);
    psgShmidx_s->sIdx_sys_fmt_g.iCount = sgLcmidx.iCount_swt_sys_fmt_g;

    /* swt_sys_fmt_d */
    psgShmidx_s->sIdx_sys_fmt_d.lOffset = psgShmidx_s->sIdx_sys_fmt_g.lOffset
        + psgShmidx_s->sIdx_sys_fmt_g.iCount * sizeof(struct shm_sys_fmt_g);
    psgShmidx_s->sIdx_sys_fmt_d.iCount = sgLcmidx.iCount_swt_sys_fmt_d;

    /* swt_sys_route_m */
    psgShmidx_s->sIdx_sys_route_m.lOffset = psgShmidx_s->sIdx_sys_fmt_d.lOffset
        + psgShmidx_s->sIdx_sys_fmt_d.iCount * sizeof(struct swt_sys_fmt_d);
    psgShmidx_s->sIdx_sys_route_m.iCount = sgLcmidx.iCount_swt_sys_route_m;

    /* swt_sys_route_g */
    psgShmidx_s->sIdx_sys_route_g.lOffset = psgShmidx_s->sIdx_sys_route_m.lOffset
        + psgShmidx_s->sIdx_sys_route_m.iCount * sizeof(struct shm_sys_route_m);
    psgShmidx_s->sIdx_sys_route_g.iCount = sgLcmidx.iCount_swt_sys_route_g;

    /* swt_sys_route_d */
    psgShmidx_s->sIdx_sys_route_d.lOffset = psgShmidx_s->sIdx_sys_route_g.lOffset
        + psgShmidx_s->sIdx_sys_route_g.iCount * sizeof(struct shm_sys_route_g);
    psgShmidx_s->sIdx_sys_route_d.iCount = sgLcmidx.iCount_swt_sys_route_d;

    /* add by gengling at 2015.03.18 four lines PSBC_V1.0 */
    /* swt_sys_tran */
    psgShmidx_s->sIdx_sys_tran.lOffset = psgShmidx_s->sIdx_sys_route_d.lOffset
        + psgShmidx_s->sIdx_sys_route_d.iCount * sizeof(struct swt_sys_route_d);
    psgShmidx_s->sIdx_sys_tran.iCount = sgLcmidx.iCount_swt_sys_tran;
#endif

#ifdef HASH
    /* swt_sys_fmt_g */
    psgShmidx_s->sIdx_sys_fmt_g.lOffset = psgShmidx_s->sIdx_sys_other.lOffset
        + psgShmidx_s->sIdx_sys_other.iCount * sizeof(struct swt_sys_other);
    psgShmidx_s->sIdx_sys_fmt_g.iCount = sgLcmidx.iCount_swt_sys_fmt_g;

    /* swt_sys_fmt_d */
    psgShmidx_s->sIdx_sys_fmt_d.lOffset = psgShmidx_s->sIdx_sys_fmt_g.lOffset
        + psgShmidx_s->sIdx_sys_fmt_g.iCount * sizeof(struct shm_sys_fmt_g);
    psgShmidx_s->sIdx_sys_fmt_d.iCount = sgLcmidx.iCount_swt_sys_fmt_d;

    /* swt_sys_route_g */
    psgShmidx_s->sIdx_sys_route_g.lOffset = psgShmidx_s->sIdx_sys_fmt_d.lOffset
        + psgShmidx_s->sIdx_sys_fmt_d.iCount * sizeof(struct swt_sys_fmt_d);
    psgShmidx_s->sIdx_sys_route_g.iCount = sgLcmidx.iCount_swt_sys_route_g;

    /* swt_sys_route_d */
    psgShmidx_s->sIdx_sys_route_d.lOffset = psgShmidx_s->sIdx_sys_route_g.lOffset
        + psgShmidx_s->sIdx_sys_route_g.iCount * sizeof(struct shm_sys_route_g);
    psgShmidx_s->sIdx_sys_route_d.iCount = sgLcmidx.iCount_swt_sys_route_d;

    /* swt_sys_tran */
    psgShmidx_s->sIdx_sys_tran_hash.lOffset = psgShmidx_s->sIdx_sys_route_d.lOffset
        + psgShmidx_s->sIdx_sys_route_d.iCount * sizeof(struct swt_sys_route_d);

    /* swt_sys_fmt_m_hash */
    psgShmidx_s->sIdx_sys_fmt_m_hash.lOffset = psgShmidx_s->sIdx_sys_tran_hash.lOffset
        + indexSize;

    /* swt_sys_fmt_g_hash */
    psgShmidx_s->sIdx_sys_fmt_g_hash.lOffset = psgShmidx_s->sIdx_sys_fmt_m_hash.lOffset
        + fmt_m_indexSize;

    /* swt_sys_route_m_hash */
    psgShmidx_s->sIdx_sys_route_m_hash.lOffset = psgShmidx_s->sIdx_sys_fmt_g_hash.lOffset
        + fmt_g_indexSize;
#endif

    /* ��sgShmbuf_s */
    sgShmbuf_s.psSwt_sys_imf = (struct swt_sys_imf *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_imf.lOffset);
    sgShmbuf_s.psSwt_sys_queue = (struct swt_sys_queue *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_queue.lOffset);
    /* === begin of added by fzj at 2002.02.28 === */
    sgShmbuf_s.psSwt_sys_matchport = (struct swt_sys_matchport *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_matchport.lOffset);
    /* === end of added by fzj at 2002.02.28 === */
    sgShmbuf_s.psSwt_sys_task = (struct swt_sys_task *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_task.lOffset);
    sgShmbuf_s.psSwt_sys_8583 = (struct swt_sys_8583 *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_8583.lOffset);
    sgShmbuf_s.psSwt_sys_8583E = (struct swt_sys_8583E *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_8583E.lOffset);  
    sgShmbuf_s.psSwt_sys_code = (struct swt_sys_code *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_code.lOffset);
    sgShmbuf_s.psSwt_sys_config = (struct swt_sys_config *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_config.lOffset);
    sgShmbuf_s.psSwt_sys_other = (struct swt_sys_other *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_other.lOffset);
    sgShmbuf_s.psShm_sys_fmt_m = (struct shm_sys_fmt_m *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_fmt_m.lOffset);
    sgShmbuf_s.psShm_sys_fmt_g = (struct shm_sys_fmt_g *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_fmt_g.lOffset);
    sgShmbuf_s.psSwt_sys_fmt_d = (struct swt_sys_fmt_d *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_fmt_d.lOffset);
    sgShmbuf_s.psShm_sys_route_m = (struct shm_sys_route_m *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_route_m.lOffset);
    sgShmbuf_s.psShm_sys_route_g = (struct shm_sys_route_g *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_route_g.lOffset);
    sgShmbuf_s.psSwt_sys_route_d = (struct swt_sys_route_d *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_route_d.lOffset);
#ifndef HASH
    /* add by gengling at 2015.03.18 begin PSBC_V1.0 */
    sgShmbuf_s.psSwt_sys_tran = (struct swt_sys_tran *)
        (alShmtmp_s + psgShmidx_s->sIdx_sys_tran.lOffset);
    /* add by gengling at 2015.03.18 end PSBC_V1.0 */
#endif
#ifdef HASH
    sgShmbuf_s.psSwt_sys_tran_hash = (void *)((void *)sgShmbuf_s.psSwt_sys_route_d + 
            sgLcmidx.iCount_swt_sys_route_d * sizeof(struct swt_sys_route_d));
    sgShmbuf_s.psShm_sys_fmt_m_hash = sgShmbuf_s.psSwt_sys_tran_hash + indexSize;
    sgShmbuf_s.psShm_sys_fmt_g_hash = sgShmbuf_s.psShm_sys_fmt_m_hash + fmt_m_indexSize;
    sgShmbuf_s.psShm_sys_route_m_hash = sgShmbuf_s.psShm_sys_fmt_g_hash + fmt_g_indexSize;
#endif

    /* �þ�̬�����ڴ����ݶ� */
    for (i = 0; i < psgShmidx_s->sIdx_sys_imf.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_imf[i]), 
                &(sgLcmidx.psSwt_sys_imf[i]), sizeof(struct swt_sys_imf));

    for (i = 0; i < psgShmidx_s->sIdx_sys_queue.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_queue[i]), 
                &(sgLcmidx.psSwt_sys_queue[i]), sizeof(struct swt_sys_queue));

    /* === begin of added by fzj at 2002.02.28 === */  
    for (i = 0; i < psgShmidx_s->sIdx_sys_matchport.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_matchport[i]), 
                &(sgLcmidx.psSwt_sys_matchport[i]), sizeof(struct swt_sys_matchport));
    /* === end of added by fzj at 2002.02.28 === */  

    for (i = 0; i < psgShmidx_s->sIdx_sys_task.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_task[i]), 
                &(sgLcmidx.psSwt_sys_task[i]), sizeof(struct swt_sys_task));

    for (i = 0; i < psgShmidx_s->sIdx_sys_8583.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_8583[i]), 
                &(sgLcmidx.psSwt_sys_8583[i]), sizeof(struct swt_sys_8583));

    for (i = 0; i < psgShmidx_s->sIdx_sys_8583E.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_8583E[i]), 
                &(sgLcmidx.psSwt_sys_8583E[i]), sizeof(struct swt_sys_8583E));

    for (i = 0; i < psgShmidx_s->sIdx_sys_code.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_code[i]), 
                &(sgLcmidx.psSwt_sys_code[i]), sizeof(struct swt_sys_code));

    for (i = 0; i < psgShmidx_s->sIdx_sys_config.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_config[i]),
                &(sgLcmidx.psSwt_sys_config[i]), sizeof(struct swt_sys_config));

    for (i = 0; i < psgShmidx_s->sIdx_sys_other.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_other[i]), 
                &(sgLcmidx.psSwt_sys_other[i]), sizeof(struct swt_sys_other));

    for (i = 0; i < psgShmidx_s->sIdx_sys_fmt_m.iCount; i ++)
        memcpy(&(sgShmbuf_s.psShm_sys_fmt_m[i]), 
                &(sgLcmidx.psShm_sys_fmt_m[i]), sizeof(struct shm_sys_fmt_m));

    for (i = 0; i < psgShmidx_s->sIdx_sys_fmt_g.iCount; i ++)
        memcpy(&(sgShmbuf_s.psShm_sys_fmt_g[i]), 
                &(sgLcmidx.psShm_sys_fmt_g[i]), sizeof(struct shm_sys_fmt_g));

    for (i = 0; i < psgShmidx_s->sIdx_sys_fmt_d.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_fmt_d[i]), 
                &(sgLcmidx.psSwt_sys_fmt_d[i]), sizeof(struct swt_sys_fmt_d));

    for (i = 0; i < psgShmidx_s->sIdx_sys_route_m.iCount; i ++)
        memcpy(&(sgShmbuf_s.psShm_sys_route_m[i]), 
                &(sgLcmidx.psShm_sys_route_m[i]), sizeof(struct shm_sys_route_m));

    for (i = 0; i < psgShmidx_s->sIdx_sys_route_g.iCount; i ++)
        memcpy(&(sgShmbuf_s.psShm_sys_route_g[i]), 
                &(sgLcmidx.psShm_sys_route_g[i]), sizeof(struct shm_sys_route_g));

    for (i = 0; i < psgShmidx_s->sIdx_sys_route_d.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_route_d[i]), 
                &(sgLcmidx.psSwt_sys_route_d[i]), sizeof(struct swt_sys_route_d));

#ifndef HASH
    /* add by gengling at 2015.03.18 begin PSBC_V1.0 */
    for (i = 0; i < psgShmidx_s->sIdx_sys_tran.iCount; i ++)
        memcpy(&(sgShmbuf_s.psSwt_sys_tran[i]), 
                &(sgLcmidx.psSwt_sys_tran[i]), sizeof(struct swt_sys_tran));
    /* add by gengling at 2015.03.18 end PSBC_V1.0 */
#endif

#ifdef HASH
    memset(sgShmbuf_s.psSwt_sys_tran_hash, 0x00, indexSize);
    if ((ilRc = swShm_swt_sys_tran_ctrl(&indexSize, sgShmbuf_s.psSwt_sys_tran_hash)) != 0) swExit(FAIL);
    memset(sgShmbuf_s.psShm_sys_fmt_m_hash, 0x00, fmt_m_indexSize);
    if ((ilRc = swShm_swt_sys_fmt_m(&fmt_m_indexSize, sgShmbuf_s.psShm_sys_fmt_m_hash)) != 0) swExit(FAIL);
    memset(sgShmbuf_s.psShm_sys_fmt_g_hash, 0x00, fmt_g_indexSize);
    if ((ilRc = swShm_swt_sys_fmt_g(&fmt_g_indexSize, sgShmbuf_s.psShm_sys_fmt_g_hash)) != 0) swExit(FAIL);
    memset(sgShmbuf_s.psShm_sys_route_m_hash, 0x00, route_m_indexSize);
    if ((ilRc = swShm_swt_sys_route_m(&route_m_indexSize, sgShmbuf_s.psShm_sys_route_m_hash)) != 0) swExit(FAIL);
#endif

#ifdef HASH
    psgShmidx_s->sIdx_sys_tran_hash.iCount = sgLcmidx.iCount_swt_sys_tran;
    psgShmidx_s->sIdx_sys_fmt_m_hash.iCount = sgLcmidx.iCount_swt_sys_fmt_m;
    psgShmidx_s->sIdx_sys_fmt_g_hash.iCount = sgLcmidx.iCount_swt_sys_fmt_g;
    psgShmidx_s->sIdx_sys_route_m_hash.iCount = sgLcmidx.iCount_swt_sys_route_m;

    /*2009-9-15 13:50 ADD BY PC*/
    swVdebug(0,"sgLcmidx.iCount_swt_sys_imf         =[%d]",         sgLcmidx.iCount_swt_sys_imf      );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_queue       =[%d]",         sgLcmidx.iCount_swt_sys_queue    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_matchport   =[%d]",         sgLcmidx.iCount_swt_sys_matchport);
    swVdebug(0,"sgLcmidx.iCount_swt_sys_task        =[%d]",         sgLcmidx.iCount_swt_sys_task     );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_8583        =[%d]",         sgLcmidx.iCount_swt_sys_8583     );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_8583E       =[%d]",         sgLcmidx.iCount_swt_sys_8583E    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_code        =[%d]",         sgLcmidx.iCount_swt_sys_code     );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_config      =[%d]",         sgLcmidx.iCount_swt_sys_config   );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_other       =[%d]",         sgLcmidx.iCount_swt_sys_other    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_fmt_m       =[%d]",         sgLcmidx.iCount_swt_sys_fmt_m    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_fmt_g       =[%d]",         sgLcmidx.iCount_swt_sys_fmt_g    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_fmt_d       =[%d]",         sgLcmidx.iCount_swt_sys_fmt_d    );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_route_m     =[%d]",         sgLcmidx.iCount_swt_sys_route_m  );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_route_g     =[%d]",         sgLcmidx.iCount_swt_sys_route_g  );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_route_d     =[%d]",         sgLcmidx.iCount_swt_sys_route_d  );
    swVdebug(0,"sgLcmidx.iCount_swt_sys_tran        =[%d]",         sgLcmidx.iCount_swt_sys_tran     );
    /*2009-9-15 13:50 ADD BY PC*/
#endif

    /* �ͷŶ�̬������ڴ�ռ� */
    /* �����˳�ʱ�Զ��ͷ���Դ */

    /* ����swShutdownʱ���������Ľ������� */
    /* �����ˢ���������öδ��� */
    if(ilOptflag) goto LDONOTHING; 
    _swShmcheck();

    printf("�����ϴιر�ʱ����ˮ����.....\n");
    sprintf(alFile,"%s/log/shm/tranlog.shm",getenv("SWITCH_DIR"));
    if((fp=fopen(alFile,"r"))!= NULL)
    {
        if(ilFlag == -1) ilFlag = swGetchoose1();
        if(ilFlag==1) /* ���빲���ڴ� */
        {
            while(1)
            {
                ilRc = fread((char *)&sgSwt_tran_log,sizeof(char),sizeof(struct swt_tran_log),fp);
                if(feof(fp)) break;
                if(ilRc != sizeof(struct swt_tran_log))
                {
                    swVdebug(0,"S0170: ���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
                    if(swGetchoose2())
                    {
                        fclose(fp);
                        swExit(FAIL);
                    }  
                    else
                        break;
                }
                sgSwt_tran_log.tran_status = iTRNREVFAIL;
                if(sgSwt_tran_log.tran_id > llTran_id_shm) llTran_id_shm = sgSwt_tran_log.tran_id;
                /* ������ˮ���빲���ڴ��� */
                /* modify by nh 20020923
                   ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,'0',&j); */
                clLoc_flag='0';
                ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
                if(ilRc)
                {
                    swVdebug(0,"S0180: ���ú���swShminsert_swt_tran_log()����!");
                    fclose(fp);
                    swExit(FAIL);
                } 
                /* add by xujun 2002.09.12 begin ... */
                /* ͳ���ڹ����ڴ浱�еĽ�����ˮ�� */
                psgShmidx_d->iRecinshm[SFTRANLOGID] ++;          
                /* del by gengling at 2015.04.16 #if 0 PSBC_V1.0 */
#if 0
                /* ͳ�ƶ˿�Դ�������� */
                for (j=0; j<psgShmidx_s->sIdx_sys_queue.iCount; j++)
                {   
                    if (sgShmbuf_s.psSwt_sys_queue[j].q_id == sgSwt_tran_log.q_tran_begin)
                    {
                        sgShmbuf_s.psSwt_sys_queue[j].tranbegin_num += 1;	            
                        if (sgSwt_tran_log.q_target<=0) break;
                    }
                    if (sgShmbuf_s.psSwt_sys_queue[j].q_id == sgSwt_tran_log.q_target)
                    {
                        sgShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
                        if (sgSwt_tran_log.q_tran_begin<=0) break;
                    }
                }
#endif
                /* add by xujun 2002.09.12 end. */   
            }
        }
        else if(ilFlag==2) /* �������ݿ� */
        {
            while(!feof(fp))
            {
                ilRc = fread((char *)&sgSwt_tran_log,sizeof(char),sizeof(struct swt_tran_log),fp);
                if(feof(fp)) break;
                if(ilRc != sizeof(struct swt_tran_log))
                {
                    swVdebug(0,"S0190: ���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
                    fclose(fp);
                    break;
                }
                sgSwt_tran_log.tran_status = iTRNREVFAIL;

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
                EXEC SQL BEGIN WORK;
#endif 
                EXEC SQL INSERT INTO swt_his_tran_log VALUES
                    (:sgSwt_tran_log.tran_id,
                     :sgSwt_tran_log.tran_begin,
                     :sgSwt_tran_log.tran_status,
                     :sgSwt_tran_log.tran_end,
                     :sgSwt_tran_log.tran_overtime,
                     :sgSwt_tran_log.q_tran_begin,
                     :sgSwt_tran_log.tc_tran_begin,
                     :sgSwt_tran_log.q_target,
                     :sgSwt_tran_log.resq_file,
                     :sgSwt_tran_log.resq_offset,
                     :sgSwt_tran_log.resq_len, 
                     :sgSwt_tran_log.resp_file,
                     :sgSwt_tran_log.resp_offset,
                     :sgSwt_tran_log.resp_len,
                     :sgSwt_tran_log.rev_key);
                if (sqlca.sqlcode)
                {
                    swVdebug(2,"S0200: ���ݿ����: INSERT INTO swt_his_tran_log, \
                            [tranid=%ld]",sgSwt_tran_log.tran_id);
                    swDberror(NULL);
                } 
                else
                    swVdebug(2,"S0210: ׷����ʷ������ˮ�ɹ�,[tranid=%ld]",
                            sgSwt_tran_log.tran_id);
#endif
            }
#ifdef DB_SUPPORT
            EXEC SQL COMMIT WORK; 
#endif

        }
        else 
        {
            /* add by xujun 2002.09.12
               ����Ա�������ݲ����лָ���ɾ�����ݿ���еļ�¼��
               ������ɽ������ݵĲ�һ����
             */
            printf(" ���ݲ����лָ�,DELETE FROM swt_tran_log!\n");

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
            EXEC SQL BEGIN WORK;
#endif 
            EXEC SQL DELETE FROM swt_tran_log WHERE 1 <> 2;
            if (sqlca.sqlcode)
            {
                swVdebug(0,"S0220: ���ݿ����: DELETE FROM swt_tran_log");          
                swDberror(NULL);
            } 
            else
            {
                EXEC SQL COMMIT WORK; 
                swVdebug(2,"S0230: ���ݲ����лָ�,DELETE FROM swt_tran_log SUCCESS");
            }    
            /* add by xujun 2002.09.12 end. */ 
#endif

        }
        fclose(fp);
    }

    sprintf(alFile,"%s/log/shm/proclog.shm",getenv("SWITCH_DIR"));
    if((fp=fopen(alFile,"r"))!= NULL)
    {
        if(ilFlag == -1) ilFlag = swGetchoose1();
        if(ilFlag==1) /* ���빲���ڴ� */
        {
            while(1)
            {
                ilRc = fread((char *)&sgSwt_proc_log,sizeof(char),sizeof(struct swt_proc_log),fp);
                if(feof(fp)) break;
                if(ilRc != sizeof(struct swt_proc_log))
                {
                    swVdebug(0,"S0240: ���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
                    if(swGetchoose2())
                    {
                        fclose(fp);
                        swExit(FAIL);
                    }  
                    else
                        break;
                }
                if ((sgSwt_proc_log.proc_status == iTRNING)||(sgSwt_proc_log.proc_status == iTRNREVING))
                    sgSwt_proc_log.proc_status = iTRNREVFAIL;
                ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'0');
                if(ilRc)
                {
                    swVdebug(0,"S0250: ���ú���swShminsert_swt_proc_log()����!");
                    fclose(fp);
                    swExit(FAIL);
                }
                /* add by xujun 2002.09.12 begin ... */
                /* ͳ���ڹ����ڴ浱�еĴ�����ˮ�� */
                psgShmidx_d->iRecinshm[SFPROCLOGID] ++;            
                /* add by xujun 2002.09.12 end. */    
            }
        }
        else if(ilFlag==2) /* �������ݿ� */
        {
            while(!feof(fp))
            {
                ilRc = fread((char *)&sgSwt_proc_log,sizeof(char),sizeof(struct swt_proc_log),fp);
                if(feof(fp)) break;
                if(ilRc != sizeof(struct swt_proc_log))
                {
                    swVdebug(0,"S0260: ���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
                    fclose(fp);
                    break;
                }
                if ((sgSwt_proc_log.proc_status == iTRNING)||(sgSwt_proc_log.proc_status == iTRNREVING))
                    sgSwt_proc_log.proc_status = iTRNREVFAIL;

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/

#ifdef DB_INFORMIX
                EXEC SQL BEGIN WORK;
#endif 
                EXEC SQL INSERT INTO swt_his_proc_log VALUES
                    (:sgSwt_proc_log.tran_id,
                     :sgSwt_proc_log.proc_step,
                     :sgSwt_proc_log.proc_begin,
                     :sgSwt_proc_log.q_target,
                     :sgSwt_proc_log.proc_status,
                     :sgSwt_proc_log.rev_mode,
                     :sgSwt_proc_log.rev_fmtgrp,
                     :sgSwt_proc_log.org_file,
                     :sgSwt_proc_log.org_offset,
                     :sgSwt_proc_log.org_len);

                if (sqlca.sqlcode)
                {
                    swVdebug(2,"S0270: INSERT INTO swt_his_proc_log, [tranid=%ld,step=%d]", \
                            sgSwt_proc_log.tran_id,sgSwt_proc_log.proc_step);
                    swDberror(NULL);  
                } 
                else
                    swVdebug(2,"S0280: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",
                            sgSwt_proc_log.tran_id,sgSwt_proc_log.proc_step);
#endif
            }
#ifdef DB_SUPPORT
            EXEC SQL COMMIT WORK;
#endif


        }
        else
        {
            /* add by xujun 2002.09.12
               ����Ա�������ݲ����лָ���ɾ�����ݿ���еļ�¼��
               ������ɽ������ݵĲ�һ����
             */
            printf(" ���ݲ����лָ�,DELETE FROM swt_PROC_log!\n");
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/

#ifdef DB_INFORMIX
            EXEC SQL BEGIN WORK;
#endif 
            EXEC SQL DELETE FROM swt_proc_log WHERE 1 <> 2;
            if (sqlca.sqlcode)
            {
                swVdebug(0,"S0290: ���ݿ����: DELETE FROM swt_proc_log");
                swDberror(NULL);
            } 
            else
            {
                EXEC SQL COMMIT WORK; 
                swVdebug(2,"S0300: ���ݲ����лָ�,DELETE FROM swt_proc_log SUCCESS");
            }    
            /* add by xujun 2002.09.12 end. */   
#endif
        }
        fclose(fp);
    }

    /*add by cjh 20150119 */
    sprintf(alFile,"%s/log/shm/saflog.shm",getenv("SWITCH_DIR"));
    if((fp=fopen(alFile,"r"))!= NULL)
    {
        if(ilFlag == -1) ilFlag = swGetchoose1();
        if(ilFlag==1) /* ���빲���ڴ� */
        {
            while(1)
            {
                ilRc = fread((char *)&sgSwt_rev_saf,sizeof(char),sizeof(struct swt_rev_saf),fp);
                if(feof(fp)) break;
                if(ilRc != sizeof(struct swt_rev_saf))
                {
                    swVdebug(0,"S0310: ���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
                    if(swGetchoose2())
                    {
                        fclose(fp);
                        swExit(FAIL);
                    }  
                    else
                        break;
                }
                ilRc = swShminsert_swt_rev_saf(sgSwt_rev_saf);
                if(ilRc)
                {
                    swVdebug(0,"S0320: ���ú���swShminsert_swt_rev_saf()����!");
                    fclose(fp);
                    swExit(FAIL);
                } 
            }
        }
        else if(ilFlag==2) /* �������ݿ� */
        {
            while(!feof(fp))
            {
                ilRc = fread((char *)&sgSwt_rev_saf,sizeof(char),sizeof(struct swt_rev_saf),fp);
                if(feof(fp)) break;
                if(ilRc != sizeof(struct swt_rev_saf))
                {
                    swVdebug(0,"S0330: ���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
                    break;
                }
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
                EXEC SQL INSERT INTO swt_rev_saf VALUES
                    (:sgSwt_rev_saf.saf_id,
                     :sgSwt_rev_saf.tran_id,
                     :sgSwt_rev_saf.proc_step,
                     :sgSwt_rev_saf.saf_begin,
                     :sgSwt_rev_saf.saf_overtime,
                     :sgSwt_rev_saf.rev_overtime,
                     :sgSwt_rev_saf.saf_num,
                     :sgSwt_rev_saf.saf_status,
                     :sgSwt_rev_saf.saf_flag,
                     :sgSwt_rev_saf.saf_file,
                     :sgSwt_rev_saf.saf_offset,
                     :sgSwt_rev_saf.saf_len);

                if (sqlca.sqlcode)
                {
                    swVdebug(2,"S0340: INSERT INTO swt_rev_saf, [safid=%ld,tranid=%ld,step=%d]", sgSwt_rev_saf.saf_id,sgSwt_rev_saf.tran_id,sgSwt_rev_saf.proc_step);
                    swDberror(NULL);
                }
                else
                    swVdebug(2,"S0350: ׷��SAF��ɹ�,[safid=%ld,tranid=%ld,step=%d]",
                            sgSwt_rev_saf.saf_id,sgSwt_rev_saf.tran_id,sgSwt_rev_saf.proc_step); 
#endif
            }
#ifdef DB_SUPPORT
            EXEC SQL COMMIT WORK;  
#endif
        }
        else {}; /* �������� */
        fclose(fp);
    }
    /*add by cjh 20150119 end*/

    /* ��ʼ��ƽ̨��ˮ�� */
    printf("��ʼ��ƽ̨��ˮ��...\n");
    if(swGettranid(&llSerno) == FAIL)
    {
        swVdebug(0,"S0360: [����/��������] swGettranid()����,������=-1, ��ʼ��ƽ̨��ˮ�ų���");
        swExit(FAIL);
    }
    psgShmidx_d->lTranid = llSerno;
    swVdebug(0,"S0000: ��ʼ��ƽ̨��ˮ�ųɹ�!tranid=[%ld]",llSerno);

    /* ��ʼ��SAF��ˮ�� */
    printf("��ʼ��SAF��ˮ��...\n");
    if(swGetsafid(&llSerno) == FAIL)
    {
        swVdebug(0,"S0370: [����/��������] swGetsafid()����,������=-1, ��ʼ��SAF��ˮ�ų���");
        swExit(FAIL);
    }
    psgShmidx_d->lSafid = llSerno;

LDONOTHING:

    /* �ù����ڴ�״̬ */
    memcpy(psgShmidx_d->aMemo,"OK",2);

    /* �ù����ڴ�ˢ�´�����ʱ�� */
    time(&llTime);
    T = localtime(&llTime);
    memset(alTime,0x00,sizeof(alTime));
    sprintf(alTime,"%04d%02d%02d%02d%02d%02d",T->tm_year+1900,T->tm_mon+1,
            T->tm_mday,T->tm_hour,T->tm_min,T->tm_sec);

    memcpy(alRefreshtimes,(psgShmidx_d->aMemo)+5,4);
    if (psgShmidx_d->aMemo[5] == '\0')
    {
        memcpy((psgShmidx_d->aMemo)+5,"0001",4);
        memcpy((psgShmidx_d->aMemo)+9,alTime,14);
    }  
    else
    {
        memset(alRefreshtimes,0x00,sizeof(alRefreshtimes));
        memcpy(alRefreshtimes,(psgShmidx_d->aMemo)+5,4);
        sprintf(alRefreshtimes,"%04d",atoi(alRefreshtimes)+1);
        memcpy((psgShmidx_d->aMemo)+5,alRefreshtimes,4);
        memcpy((psgShmidx_d->aMemo)+23,alTime,14);
    }  

    /* �ù����ڴ�״̬Ϊ��ˢ�� */

    if (ilOptflag == 0)
    {
        sprintf(alFile,"%s/log/shm/tranlog.shm",getenv("SWITCH_DIR"));
        unlink(alFile);
        sprintf(alFile,"%s/log/shm/proclog.shm",getenv("SWITCH_DIR"));
        unlink(alFile);
        /* delete by xujun 2002.09.12
           sprintf(alFile,"%s/log/shm/saflog.shm",getenv("SWITCH_DIR"));
           unlink(alFile);
         */

        /* �ر����ݿ����� */
#ifdef DB_SUPPORT
        swDbclose();
#endif
        printf("�����ڴ��ʼ���ɹ�!\n");
    }
    else
        printf("�����ڴ�ˢ�³ɹ�!\n");

    psgShmidx_d->iShmid = ilShmid;

    exit(SUCCESS);
}

void swExit(int flag)
{
    if (ilOptflag == 0)
    {
        /* �ر����ݿ����� */
#ifdef DB_SUPPORT
        swDbclose();
#endif
        /* �ͷ�IPC��Դ */
        if (ilShmid_del_d >= 0)
            shmctl(ilShmid_del_d,IPC_RMID,0);
        if (ilSemid_del >= 0)
            semctl(ilSemid_del,0,IPC_RMID,0);
        if (ilShmid_del_s >= 0)
            shmctl(ilShmid_del_s,IPC_RMID,0);

        printf("�����ڴ��ʼ������!\n");
    }  
    else
    {
        psgShmidx_d->iShmid = ilShmid_old;
        printf("�����ڴ�ˢ�³���!\n");
    }
    exit(flag);
}


/**************************************************************************
 ** ��������swGettranid
 ** ��  �ܣ� to get serialnum for a new tran.
 ** ��  �ߣ� 
 ** �������ڣ�
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ�����
 ** �������壺
 ** ����ֵ��
 **************************************************************************/
static int swGettranid( long *lserno )
{
    /*del by cjh 20141220 �޸���ˮ�ų�ʼ����ʽ
    "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64
#ifdef DB_DB2_64
EXEC SQL BEGIN DECLARE SECTION;
sqlint64 llTran_id;
sqlint64 llTran_begin;
sqlint64 llHis_tran_id;
sqlint64 llHis_tran_begin;
short ilTmp;
sqlint64 iBegSeqNo;
sqlint64 iEndSeqNo;
EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
EXEC SQL BEGIN DECLARE SECTION;
long llTran_id;
long llTran_begin;
long llHis_tran_id;
long llHis_tran_begin;
short ilTmp;
long iBegSeqNo;
long iEndSeqNo;
EXEC SQL END DECLARE SECTION;
#endif


    2009-9-9 9:46	�ֲ��洢������Ϣ�ṹ��
    PB_GROUPINFO sGrpComInfo;
    int groupid;
    int ilRc;
    2009-9-9 9:46	�ֲ��洢������Ϣ�ṹ��


    2009-9-9 9:28 ���ݼ�Ⱥ�������ȡ��ˮ�ŷ�Χ
    memset((char*)&sGrpComInfo,0x00,sizeof(PB_GROUPINFO));
    groupid=0;
    ��ȡ��������ID
    groupid=atoi(getenv("BMQ_GROUP_ID"));

    ilRc=swGetGroupHostInfo2(groupid,&sGrpComInfo);
    if (ilRc) {
    sGrpComInfo.iBegSeqNo=0;
    sGrpComInfo.iEndSeqNo=lMAXSERNO;
    }	
    swVdebug(3,"minserialno[%d] maxserialno[%d]",sGrpComInfo.iBegSeqNo,sGrpComInfo.iEndSeqNo);
    iBegSeqNo=sGrpComInfo.iBegSeqNo;
    iEndSeqNo=sGrpComInfo.iEndSeqNo;


    swVdebug(4,"S0380: [��������] swGettranid()");

    llTran_id = 0;
    llHis_tran_id = 0;


    EXEC SQL select max(tran_begin) into :llHis_tran_begin:ilTmp
    from swt_his_tran_log
    where tran_id>=:iBegSeqNo and tran_id<=:iEndSeqNo;
    if( sqlca.sqlcode )
    {
    swVdebug(0,"S0390: [����/���ݿ�] select max(tran_begin) into :llHis_tran_begin:ilTmp from swt_his_tran_log,sqlcode=%d",sqlca.sqlcode);
    return(FAIL);
    }
    swVdebug(0,"max(tran_begin)[%d] in swt_his_tran_log",llHis_tran_begin);
    if(ilTmp < 0)
    {
    llHis_tran_id = 0;
    }
    else
    {
    EXEC SQL SELECT max(tran_id) INTO :llHis_tran_id:ilTmp
    FROM swt_his_tran_log WHERE tran_begin = :llHis_tran_begin
    and  tran_id>=:iBegSeqNo and tran_id<=:iEndSeqNo;
    if( sqlca.sqlcode )
    {
    swVdebug(0,"S0400: [����/���ݿ�] SELECT max(tran_id) INTO :llHis_tran_id:ilTmp FROM swt_his_tran_log WHERE tran_begin = :llHis_tran_begin,sqlcode=%d",sqlca.sqlcode);
    swDberror(NULL);
    return(FAIL);
    }
    swVdebug(0,"max(tran_id)[%d] in swt_his_tran_log",llHis_tran_id);    
    }

    EXEC SQL select max(tran_begin) into :llTran_begin:ilTmp
    from swt_tran_log
    where  tran_id>=:iBegSeqNo and tran_id<=:iEndSeqNo;
    if( sqlca.sqlcode )
    {
    swVdebug(0,"S0410: [����/���ݿ�] select max(tran_begin) into :llTran_begin:ilTmp from swt_tran_log,sqlcode=%d",sqlca.sqlcode);
    return(FAIL);
    }
    swVdebug(0,"max(tran_begin)[%d] in swt_tran_log",llTran_begin);
    if(ilTmp < 0)
    {
    llTran_id = 0;
    }
    else
    {
    EXEC SQL SELECT max(tran_id) INTO :llTran_id:ilTmp
    FROM swt_tran_log WHERE tran_begin = :llTran_begin
    and  tran_id>=:iBegSeqNo and tran_id<=:iEndSeqNo;
    if( sqlca.sqlcode )
    {
    swVdebug(0,"S0420: [����/���ݿ�] SELECT max(tran_id) INTO :llTran_id:ilTmp FROM swt_tran_log WHERE tran_begin = :llTran_begin,sqlcode=%d",sqlca.sqlcode);
    swDberror(NULL);
    return(FAIL);
    }
    swVdebug(0,"max(tran_id)[%d] in swt_tran_log",llHis_tran_id);     
    }
    swVdebug(0,"llHis_tran_id[%d] llTran_id_shm[%d] llTran_id[%d]",llHis_tran_id,llTran_id_shm,llTran_id);     

    if(llHis_tran_id > llTran_id) llTran_id = llHis_tran_id;
    if(llTran_id_shm > llTran_id) llTran_id = llTran_id_shm;
    *lserno =	((llTran_id + 50000)- iBegSeqNo) % (iEndSeqNo-iBegSeqNo)+iBegSeqNo;
    end of del by cjh 20141220 */

        /*add by cjh 20141220 �޸���ˮ�ų�ʼ����ʽ*/
        FILE *fp;
    int fd,i;
    long tranid;
    int udptime; //����ʱ��
    char aFilename[100];
    char buf[50],buf1[50];

    memset(aFilename,0x00,sizeof(aFilename));
    sprintf( aFilename,"%s/config/SWBUFSPE.CFG",getenv("SWITCH_CFGDIR") );
    if((fp=fopen(aFilename,"r"))==NULL)
    {
        swVdebug(0,"S0400��[����/�ļ�]�����ļ���ʧ��");
        return(FAIL);
    }
    fscanf(fp,"%d",&udptime);
    swVdebug(0,"S0410:�����ļ�����ʱ��udptime[%d]",udptime);
    fclose(fp);
    fd = open("/tmp/tranid.ini",O_RDONLY);
    if(fd < 0)
    {
        *lserno = 1;
        swVdebug(0,"S0420��[����/�ļ�]�����ļ���ʧ�ܣ���ˮ�ų�ʼ��Ϊ1");
    } 
    else
    {     
        lock_set(fd,F_RDLCK);
        memset(buf,0x00,sizeof(buf));
        memset(buf1,0x00,sizeof(buf1));
        read(fd,buf,50);
        for(i=0;i<strlen(buf);i++)
        {
            if(buf[i]=='|')
                break;
        }
        memcpy(buf1,buf,i);
        tranid = atol(buf1);
        printf("tranid=[%ld]\n",tranid);
        lock_set(fd,F_UNLCK);
        close(fd);
        *lserno = tranid + udptime*MAX_SEC_TRANID +1;
    }
    /*end of add by cjh 20141220 */

    swVdebug(0,"S0430: [��������] swGettranid()������=0 *lserno[%d]",*lserno);
    return (SUCCESS);
}

/**************************************************************************
 ** ��������swGetsafid
 ** ��  �ܣ� to get serialnum for a new tran.
 ** ��  �ߣ� 
 ** �������ڣ�
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ�����
 ** �������壺
 ** ����ֵ��
 **************************************************************************/
static int swGetsafid( long *lserno )
{
    /* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64
#ifdef DB_SUPPORT  *support database,Database Deprivation Project 2014-12
#ifdef DB_DB2_64
EXEC SQL BEGIN DECLARE SECTION;
sqlint64 llSaf_id;
sqlint64 llSaf_begin;
short ilTmp1;
EXEC SQL END DECLARE SECTION;
#endif
     *modify  by zcd 20141224*
#ifdef DATABASE
EXEC SQL BEGIN DECLARE SECTION;
long llSaf_id;
long llSaf_begin;
short ilTmp1;
EXEC SQL END DECLARE SECTION;
#endif
#else
long llSaf_id;
long llSaf_begin;
short ilTmp1;
#endif
     *end of modify by zcd 20141224*

     swVdebug(4,"S0440: [��������] swGetsafid()");

     llSaf_id = 0;

#ifdef DB_SUPPORT  *support database,Database Deprivation Project 2014-12*  
EXEC SQL select max(saf_begin) into :llSaf_begin:ilTmp1
from swt_rev_saf;
if( sqlca.sqlcode )
{
swVdebug(0,"S0450: [����/���ݿ�] select max(saf_begin) into :llSaf_begin:ilTmp1 from swt_rev_saf,sqlcode=%d",sqlca.sqlcode);
return(FAIL);
}
if(ilTmp1< 0)
{
llSaf_id = 0;
}
else
{
EXEC SQL SELECT max(saf_id) INTO :llSaf_id:ilTmp1
FROM swt_rev_saf WHERE saf_begin = :llSaf_begin;
if( sqlca.sqlcode )
{
swVdebug(0,"S0460: [����/���ݿ�] SELECT max(saf_id) INTO :llSaf_id:ilTmp1 FROM swt_rev_saf WHERE saf_begin = :llSaf_begin,sqlcode=%d",sqlca.sqlcode);
swDberror(NULL);
return(FAIL);
}
}
     *lserno = llSaf_id;
     *add by zcd 20141224*
#else
short ilRc;
     *
     sw_init_logfile();
     ilRc = sw_get_max_saf_id(lserno);
     if(ilRc == FAIL) *lserno = 0;
     *
     *lserno = 0;
#endif
     *end of add by zcd 20141224*/

/*add by cjh 20150113 �޸ĳ����ų�ʼ����ʽ*/
FILE *fp; 
int fd,i;
long safid;
int udptime; //����ʱ��
char aFilename[100];
char buf[50],buf1[50];

memset(aFilename,0x00,sizeof(aFilename));
sprintf( aFilename,"%s/config/SWBUFSPE.CFG",getenv("SWITCH_CFGDIR") );
if((fp=fopen(aFilename,"r"))==NULL)
{
    swVdebug(0,"S0400��[����/�ļ�]�����ļ���ʧ��");
    return(FAIL);
}
fscanf(fp,"%d",&udptime);
swVdebug(0,"S0410:�����ļ�����ʱ��udptime[%d]",udptime);
fclose(fp);
/*�����ļ��� modify by cjh 20150203
  if((fp=fopen("/tmp/tranid.ini","r"))==NULL)
  {
 *lserno = 1;
 swVdebug(0,"S0420��[����/�ļ�]�����ļ���ʧ�ܣ������ų�ʼ��??");
 }
 else
 {
 fgets(buf,1024,fp);
 fgets(buf,1024,fp);
 safid = atol(buf);
 printf("safid=[%ld]\n",safid);
 fclose(fp);
 *lserno = safid + udptime*MAX_SEC_TRANID +1;
 }*/
fd = open("/tmp/tranid.ini",O_RDONLY);
if(fd < 0)
{
    *lserno = 1;
    swVdebug(0,"S0420��[����/�ļ�]�����ļ���ʧ�ܣ���ˮ�ų�ʼ��Ϊ1");
}
else
{
    lock_set(fd,F_RDLCK);
    memset(buf,0x00,sizeof(buf));
    memset(buf1,0x00,sizeof(buf1));
    read(fd,buf,50);
    for(i=0;i<strlen(buf);i++)
    {
        if(buf[i]=='|')
            break;
    }
    memcpy(buf1,buf+i+1,strlen(buf)-i-1);
    safid = atol(buf1);
    printf("safid=[%ld]\n",safid);
    lock_set(fd,F_UNLCK);
    close(fd);
    *lserno = safid + udptime*MAX_SEC_TRANID +1;
}    
/*end of add by cjh 20150113 */
swVdebug(4,"S0470: [��������] swGetsafid()������=0");
return (SUCCESS);
}

int lock_set(int fd,int type)
{
    struct flock lock;
    lock.l_type=type;
    lock.l_start=0;
    lock.l_whence=SEEK_SET;
    lock.l_len=0;
    lock.l_pid=-1;

    fcntl(fd,F_GETLK,&lock);
    if(lock.l_type!=F_UNLCK)
    {

        if(lock.l_type==F_RDLCK)          //���ļ����ж�ȡ��
        {
            swVdebug(0,"Read lock already set by %d\n",lock.l_pid);
        }
        else if(lock.l_type==F_WRLCK)    //���ļ�����д����
        {
            swVdebug(0,"Write lock already set by %d\n",lock.l_pid);
        }
    }

    lock.l_type=type;

    if((fcntl(fd,F_SETLKW,&lock))<0)
    {
        swVdebug(0,"Lock failed:type=%d\n",lock.l_type);
        return -1;
    }
    switch(lock.l_type)
    {
        case F_RDLCK:
            {
                swVdebug(4,"Read lock set by %d\n",getpid());
            }
            break;
        case F_WRLCK:
            {
                swVdebug(4,"Write lock set by %d\n",getpid());
            }
            break;
        case F_UNLCK:
            {
                swVdebug(4,"Release lock by %d\n",getpid());
            }
            break;
        default:
            break;
    }
    return 0;
}

static int swGetchoose1()
{
    char ch;

    printf("�����ϴ�ж��ʱ���������Ľ�������!\n");
    printf("ϵͳ�Զ�����!\n");
    return 0;
    /*  
        printf("��ѡ��[1-���� 2-���빲���ڴ� 3-�������ݿ�]:");
     */  
    while(1)
    {
        ch = getchar();
        switch(ch)
        {
            case '1':
                return 0;
            case '2':
                return 1;
            case '3':
                return 2;  
            default:
                break;
        }
    }
}

static int swGetchoose2()
{
    char ch;

    printf("�Ƿ����?[y/n], ע��: �������,ʣ�µĽ������ݽ��޷��ָ�!");
    printf("ϵͳ�Զ�����!\n");
    return 0;  

    while(1)
    {
        ch = getchar();
        switch(ch)
        {
            case 'Y':
            case 'y':
                return 0;
            case 'N':
            case 'n':
                return 1;
            default:
                break;
        }
    }
}

