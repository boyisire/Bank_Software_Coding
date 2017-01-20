/****************************************************************/
/* ģ����    ��MONLOG                                         */ 
/* ģ������    ���������⽻����ˮ                           */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ���Ż�                                           */
/* ��������    ��1999/11/18                                     */
/* ����޸����ڣ�2001/3/24                                      */
/* ģ����;    ���ҵ���ʱ�Ľ���,����SAF,���г���                */
/* ��ģ���а������º���������˵����                             */
/*		  (1) void main();                     		*/
/*                (2) void swQuit();                   		*/
/*                (3) int swProcess_proclog(long,short,char)	        */
/*                (4) int swProcess_revend_tranlog(long,short,char); */
/*                (5) int swProcess_revend_proclog(long,short,char); */
/****************************************************************/
/****************************************************************/
/* �޸ļ�¼��                                                   */
/*  2000/1/27 �������Ŷ��ڵ���ƽ����޸�                        */
/*  2000/3/17 �޸�revmode,revfmtgroup�Ĵ���                     */
/*  2000/8/25 �Ż�                                              */
/*  2001/2/22 Oracle��ֲ                                        */
/*  2001/3/15 ��׳����ǿ                                        */
/*  2001/3/24 ���V3.01,����1346��                            */
/****************************************************************/
#include "switch.h"

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
#include "sw_shmapi_tranlog.h"

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
EXEC SQL BEGIN DECLARE SECTION;
  struct swt_proc_log_sql
  {
    sqlint64  tran_id;
    short     proc_step;
    sqlint64  proc_begin;
    short     q_target;
    short     proc_status;
    short     rev_mode;
    /*del by zcd 20141222
	short     rev_fmtgrp;
	***end of del by zcd 20141222*/
	/*add by zcd 20141222*/
    long     rev_fmtgrp;  
	/*end of add  by zcd 20141218*/
    char      org_file[11];
    sqlint64  org_offset;
    short     org_len;
  } slSwt_proc_log;
EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
EXEC SQL BEGIN DECLARE SECTION;
  struct swt_proc_log_sql
  {
    long  tran_id;
    short proc_step;
    long  proc_begin;
    short q_target;
    short proc_status;
    short rev_mode;
    /*del by zcd 20141222
	short     rev_fmtgrp;
	***end of del by zcd 20141222*/
	/*add by zcd 20141222*/
    long     rev_fmtgrp;  
	/*end of add  by zcd 20141218*/
    char  org_file[11];
    long  org_offset;
    short org_len;
  } slSwt_proc_log;
EXEC SQL END DECLARE SECTION;
#endif

#else
 struct swt_proc_log_sql
 {
   long	tran_id;
   short proc_step;
   long	proc_begin;
   short q_target;
   short proc_status;
   short rev_mode;
   short rev_fmtgrp;
   char	org_file[11];
   long	org_offset;
   short org_len;
 } slSwt_proc_log;

#endif

short  i,j,ilRc,ilCount;
struct swt_proc_log pslSwt_proc_log[iMAXPROCNUM];

/* ����ԭ�Ͷ��� */
void swQuit(int);
int swProcess_proclog(long,short,char);
int swProcess_revend_tranlog(long,short,char);
int swProcess_revend_proclog(long,short,char);
/*begin add by gf at 2004-04-07*/
int swSendRevresult_nounion(short);
/*end add*/

/**************************************************************
 ** ������      �� main
 ** ��  ��      �� ������
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/11/18
 ** ����޸����ڣ� 2000/4/5
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/
int main(int argc,char *argv[])
{
  short ilWaitTime;				/* ϵͳ�ȴ�ʱ�� */
  long  llCurrentTime;				/* ϵͳ��ǰʱ�� */
  short ilQtran_begin;				/* ����Դ������ */
  
  short ilPrior = 0,ilClass = 0,ilType = 0;	/* ������� */
  struct msgpack slMsgpack;			/* ���� */
  short  ilRevstep=0;			/* ����ģʽΪ3��4������ײ��� */
  
  char  alResult[iFLDVALUELEN + 1];
  short ilLength;
  
  short ilCount_main = 0; /* modify by qy 2001.11.10 */
  short ilClear_count = 0;                          /*������̻��Ѽ������*/
  long llClear_time;                            /*�������������ˮ��ʱ���־*/
  long  llProc_begin;
  short ilMoveproc_flag = 0;
  long  llRerev_inter = 0;                /* RESAF���ʱ�� */
  long  llRerev_time = 0;
  char  clLoc_flag;
  long llClear_time_prep;		/* add by szhengye 2004.2.19 */

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64 llSaf_id;
    sqlint64 llTran_id;
    sqlint64 llRerev_time_cal = 0;
    short ilTran_step;
    short ilTmpind;
    short ilProc_step;
    short ilProc_step_max;
  EXEC SQL END DECLARE SECTION;  
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long llSaf_id;
    long llTran_id;
    long llRerev_time_cal = 0;
    short ilTran_step;
    short ilTmpind;
    short ilProc_step;
    short ilProc_step_max;
  EXEC SQL END DECLARE SECTION;  
#endif
#else
  long llSaf_id;
  long llTran_id;
  long llRerev_time_cal = 0;
  short ilTran_step;
  short ilTmpind;
  short ilProc_step;
  short ilProc_step_max;
#endif


  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swMonlog Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swMonlog")) exit(FAIL);
  
  /* ���õ��Գ������� */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swMonlog.debug");

  /* ȡ��DEBUG��־ */
  if((cgDebug = _swDebugflag("swMonlog")) == FAIL)
  {
    fprintf(stderr,"ȡDEBUG��־ʧ��!\n");
    exit(FAIL);
  } 
  swLogInit("swMonlog", "swMonlog.debug");
  swVdebug(2,"S0010: �汾��[4.3.0]");
  swVdebug(2,"S0020: DEBUG�ļ�[%s]",agDebugfile);
  swVdebug(2,"S0030: DEBUG��־[%d]",cgDebug);
 
  /* ������ */
  ilRc = qattach(iMBMONREV);
  if(ilRc) 
  {
    swVdebug(0,"S0040: [����/����] qattach()����,������=%d",ilRc);
    exit(FAIL);
  }
  swVdebug(2,"S0050: ������ɹ�");

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  /* �����ݿ� */
  if(swDbopen())
  {
    swVdebug(0,"S0060: [����/���ݿ�] �����ݿ�ʧ��");
    swQuit(FAIL);
  } 
  swVdebug(2,"S0070: �����ݿ�ɹ�");
#endif
 
  /* �ź����κ����� */
  signal(SIGTERM,swQuit);
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  
  /* ��ʼ�������ڴ�ָ�� */
  if(swShmcheck()) 
  {
    swVdebug(0,"S0080: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    swQuit(FAIL);
  }
  swVdebug(2,"S0090: ��ʼ�������ڴ�ָ��ɹ�");
 
  /* ��ϵͳ��ط�����Ϣ���� */
  swVdebug(2,"S0100: ����104001��Ϣ����,[swMonlog]�Ѿ�����...");
  _swMsgsend(104001,NULL);
 
  time(&llClear_time_prep);		/* add by szhengye 2004.2.19 */
  while (TRUE) 
  {
    /* ˢ�¹����ڴ�ָ�� */
    if(swShmcheck()) 
    {
      swVdebug(0,"S0110: [����/�����ڴ�] ˢ�¹����ڴ�ָ��ʧ��");
      swQuit(FAIL);
    }
    /* ��ȡϵͳ��ѭʱ�� */
    ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
    if(ilRc)
    {
      swVdebug(0,"S0120: [����/�����ڴ�] ��ȡϵͳ��ѭʱ��ʧ��,������=%d",ilRc);
      swQuit(FAIL);
    }
    ilWaitTime = sgSwt_sys_config.log_inter;
    ilClear_count = sgSwt_sys_config.log_clear_count;
    llRerev_inter = sgSwt_sys_config.rerev_inter;
    
    if(ilClear_count <= 0) goto NORMALPROCESS;
 
    /* �����ڴ��������� */
    llClear_time = ilWaitTime * ilClear_count;
    time(&llCurrentTime);
    
    /* modify by szhengye 2004.2.19
    if(++ilClear_count_cal == ilClear_count)
    */
#if 0
    if ((llCurrentTime - llClear_time_prep) >= llClear_time)
    {
      /* modify by nh 20040216
      llTran_begin = llCurrentTime - llClear_time;
      llProc_begin = llCurrentTime - llClear_time; */
      llTran_begin = llClear_time_prep;
      llProc_begin = llClear_time_prep;
      llClear_time_prep = llCurrentTime;
      swVdebug(2,"S0125: ��ʼ������ڵĽ�����ˮ,��ǰʱ��[%ld],����[%ld]��ǰ�Ľ�����ˮ!",llCurrentTime,llTran_begin);

      /* ��������ˮ */
      while(TRUE)
      {
        /* ���ҹ��ڵĽ�����ˮ */
        ilRc = swShmselect_swt_tran_log_clear(llTran_begin, &sgSwt_tran_log,&clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0130: [����/�����ڴ�] ���ҽ�����ˮ����,[tran_bgein<=%ld]",llTran_begin);
          break;
        }
        if(ilRc == SHMNOTFOUND)
          break;

        swVdebug(2,"S0140: �ҵ����ڵĽ�����ˮ,[tran_id=%ld],����״̬[%d]",sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status);
        llTran_id = sgSwt_tran_log.tran_id;

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
        EXEC SQL BEGIN WORK;
#endif
        /*�ƽ�����ˮ����ʷ*/ 
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
        if(sqlca.sqlcode)
        {
          swVdebug(0,"S0150: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld]",llTran_id);
          swDberror(NULL);
        }
        swVdebug(2,"S0160: ׷����ʷ������ˮ�ɹ�,[tranid=%ld]",llTran_id);
#else
		sw_put_tranlog(&sgSwt_tran_log);
		/*todo*/
#endif

        /* ɾ��������ˮ */
        ilRc = swShmdelete_swt_tran_log(llTran_id,clLoc_flag);
        if(ilRc == FAIL)
          swVdebug(0,"S0170: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld]",llTran_id);
        else if(ilRc == SHMNOTFOUND)
          swVdebug(2,"S0180: ������ˮû���ҵ�,[tranid=%ld]",llTran_id);
        else
          swVdebug(2,"S0190: ɾ��������ˮ�ɹ�,[tranid=%ld]",llTran_id);

        swVdebug(2,"S0195: ��ʼ����ý�����ˮ[tranid=%ld]��Ӧ�Ĵ�����ˮ!",llTran_id);
        /* �ƴ�����ˮ����ʷ */
        ilRc = swShmselect_swt_proc_log_mrec(llTran_id,pslSwt_proc_log,&ilCount_main);
        if(ilRc == FAIL)
          swVdebug(0,"S0200: [����/�����ڴ�] ���Ҵ�����ˮ����,[tranid=%ld,step=*]",llTran_id);
        else if(ilRc == SHMNOTFOUND)
          swVdebug(2,"S0210: û���ҵ�������ˮ,[tranid=%ld]",llTran_id);
        else
        {
          swVdebug(2,"S0220: �ҵ�������ˮ,[tranid=%ld,step=*]",llTran_id);
          for (i = 0; i < ilCount_main; i ++)
          {
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
            memcpy(&slSwt_proc_log,&pslSwt_proc_log[i],sizeof(struct swt_proc_log));
            EXEC SQL INSERT INTO swt_his_proc_log VALUES
             (:slSwt_proc_log.tran_id,
              :slSwt_proc_log.proc_step,
              :slSwt_proc_log.proc_begin,
              :slSwt_proc_log.q_target,
              :slSwt_proc_log.proc_status,
              :slSwt_proc_log.rev_mode,
              :slSwt_proc_log.rev_fmtgrp,
              :slSwt_proc_log.org_file,
              :slSwt_proc_log.org_offset,
              :slSwt_proc_log.org_len);
            if(sqlca.sqlcode)
            {
              swVdebug(0,"S0230: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld,step=%d]",llTran_id,slSwt_proc_log.proc_step);
              swDberror(NULL);
            }
            swVdebug(2,"S0240: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d,proc_status=%d,proc_begin=%ld]",llTran_id,slSwt_proc_log.proc_step,slSwt_proc_log.proc_status,slSwt_proc_log.proc_begin);
#else
		    sw_put_proclog(&pslSwt_proc_log[i]);
#endif
          }

          /* ɾ��������ˮ */
          ilRc = swShmdelete_swt_proc_log_mrec(llTran_id,clLoc_flag);
          if(ilRc == FAIL)
            swVdebug(0,"S0250: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld,step=*]",llTran_id);
          else if(ilRc == SHMNOTFOUND)
            swVdebug(2,"S0260: ������ˮû���ҵ�,[tranid=%ld,step=*]",llTran_id);
          else
            swVdebug(2,"S0270: ɾ��������ˮ�ɹ�,[tranid=%ld,step=*]",llTran_id);
        }
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        EXEC SQL COMMIT WORK;
#endif
      }     
      swVdebug(2,"S0271: ���ڵĽ�����ˮ�������!");     

      /*��������ˮ*/
      swVdebug(2,"S0275: ��ʼ������ڵĴ�����ˮ,��ǰʱ��[%ld],����[%ld]��ǰ�Ĵ�����ˮ!",llCurrentTime,llProc_begin);      
      while(TRUE)
      { 
      	/* ���ҹ��ڵĴ�����ˮ */
        ilRc = swShmselect_swt_proc_log_clear(llProc_begin, &sgSwt_proc_log,&clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0280: [����/�����ڴ�] ���ҹ��ڵĴ�����ˮ����,[proc_begin<%ld]",llProc_begin);
          break;
        }
        if(ilRc == SHMNOTFOUND)
          break;
          
        llTran_id = sgSwt_proc_log.tran_id;
        ilProc_step = sgSwt_proc_log.proc_step;
        swVdebug(2,"S0290: �ҵ����ڵĴ�����ˮ,[tranid=%ld,step=%d,proc_status=%d,proc_begin=%ld]",llTran_id,ilProc_step,sgSwt_proc_log.proc_status,sgSwt_proc_log.proc_begin);

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
        EXEC SQL BEGIN WORK;
#endif
        /* ������ˮ����ʷ */
        memcpy(&slSwt_proc_log,&sgSwt_proc_log,sizeof(struct swt_proc_log));
        EXEC SQL INSERT INTO swt_his_proc_log VALUES
         (:slSwt_proc_log.tran_id,
          :slSwt_proc_log.proc_step,
          :slSwt_proc_log.proc_begin,
          :slSwt_proc_log.q_target,
          :slSwt_proc_log.proc_status,
          :slSwt_proc_log.rev_mode,
          :slSwt_proc_log.rev_fmtgrp,
          :slSwt_proc_log.org_file,
          :slSwt_proc_log.org_offset,
          :slSwt_proc_log.org_len);

        if(sqlca.sqlcode)
        {
          swVdebug(0,"S0300: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          swDberror(NULL);
        }
#else
		sw_put_proclog(sgSwt_proc_log);
#endif
        /* ɾ��������ˮ */
        ilRc = swShmdelete_swt_proc_log(llTran_id, ilProc_step,clLoc_flag);
        if(ilRc == FAIL)
          swVdebug(0,"S0310: [����/�����ڴ�] ɾ��������ˮ����, [tranid=%ld, procstep=d%]",llTran_id, ilProc_step);
        else if(ilRc == SHMNOTFOUND)
          swVdebug(2,"S0320: [����/�����ڴ�] Ҫɾ���Ĵ�����ˮû���ҵ�, [tranid=%ld, procstep=d%]",llTran_id, ilProc_step);
        else
          swVdebug(2,"S0330: ɾ��������ˮ�ɹ�, [tranid=%ld, step=d%]",llTran_id, ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        EXEC SQL COMMIT WORK;
#endif
      }
      swVdebug(2,"S0335: ���ڵĴ�����ˮ�������!"); 
      /* del by nh 20040219 ilClear_count_cal = 0;*/
    }
#endif
NORMALPROCESS:

    /* ȡ�õ�ǰϵͳʱ�� */
    time(&llCurrentTime);

    /* ���ҳ�ʱ����С������ˮ��¼ */
    /* ���ҳ�ʱ����С������ˮ��¼,������Ϊ���ڳ��� */
    /* modify by cy 20040324 
    ilRc = swShmselect_overtime_min_tranlog(llCurrentTime,&sgSwt_tran_log,&clLoc_flag);*/
    ilRc = swShmselect_overtime_min_tranlog_update(llCurrentTime,&sgSwt_tran_log,&clLoc_flag);
    if(ilRc == FAIL) 
    {
      swVdebug(0,"S0340: [����/�����ڴ�] ���ҳ�ʱ����С������ˮ��¼����");
      swQuit(FAIL);
    }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    if(ilRc == SHMNOTFOUND) 
    {
      /* �Ƿ����RESAF���� */
      if(llRerev_inter <= 0)
      {
        sleep(ilWaitTime);
        continue;
      }

      time(&llCurrentTime);

      if(llCurrentTime < llRerev_time)
      {
        if(llCurrentTime + llRerev_inter < llRerev_time)
          llRerev_time = llCurrentTime + llRerev_inter;

        sleep(ilWaitTime);
        continue;
      }

      llRerev_time_cal = llCurrentTime - llRerev_inter;

      /* ����ʷ������ˮ���в�����С��RESAF������ˮ��¼ */
      /* del by gengling 20150114 */
#if 0
      EXEC SQL SELECT MIN(saf_id) INTO :llSaf_id:ilTmpind
           FROM swt_rev_saf WHERE saf_flag = '2'
                            AND   saf_overtime <= :llRerev_time_cal;
      if (sqlca.sqlcode)
      {
        swVdebug(0,"S0350: [����/���ݿ�] SELECT MIN(saf_id) FROM swt_rev_saf ...error");
        swDberror(NULL);
        continue;
      }
#endif
      /* end of del */

      /* add by gengling 20150114 */
      ilRc = swShmselect_saf_id_min_revsaf(llRerev_time_cal, &sgSwt_rev_saf, &clLoc_flag);
      if(ilRc == FAIL) 
      {
        swVdebug(0,"S0350: [����/�����ڴ�] SELECT MIN(saf_id) FROM swt_rev_saf ...error");
        swDberror(NULL);
        continue;
      }
      ilTmpind = sgSwt_rev_saf.saf_id;
      llSaf_id = sgSwt_rev_saf.saf_id;
      swVdebug(2,"S0: ���ҳ���saf��ʱ��saf_id��С��¼,[MIN(saf_id)=%ld]", ilTmpind); /* will */
      /* end of add */
  
      if(ilTmpind < 0)   /* û�ҵ� */
      {
        /* del by gengling 20150114 */
#if 0
        EXEC SQL SELECT MIN(saf_overtime) INTO :llRerev_time_cal:ilTmpind
             FROM swt_rev_saf WHERE saf_flag = '2';
        if (sqlca.sqlcode)
        {
          swVdebug(0,"S0360: [����/���ݿ�] SELECT MIN(saf_overtime) FROM swt_rev_saf ...error");
          swDberror(NULL);
          continue;
        }
#endif
        /* end of del */

        /*add by gengling 20150114*/
        ilRc = swShmselect_saf_overtime_min_revsaf(&sgSwt_rev_saf, &clLoc_flag);
        if(ilRc == FAIL) 
        {
          swVdebug(0,"S0360: [����/�����ڴ�] MIN(saf_overtime) FROM swt_rev_saf ...error");
          swDberror(NULL);
          continue;
        }
        ilTmpind = sgSwt_rev_saf.saf_overtime;
        llRerev_time_cal = sgSwt_rev_saf.saf_overtime;
        swVdebug(2,"S0: ���ҳ���saf saf_overtime�ֶ���С�ļ�¼,[MIN(saf_overtime)=%ld %ld]", ilTmpind, llSaf_id); /* will */
        /*end of add */

        if(ilTmpind < 0)
        {
          llRerev_time = llCurrentTime + llRerev_inter;
          sleep(ilWaitTime);
          continue;
        }

        llRerev_time = llRerev_time_cal + llRerev_inter;
        sleep(ilWaitTime);
        continue;
      }

      swVdebug(2,"S0370: ����ʷSAF�����ҵ���ҪREREV�ĳ���ʧ�ܼ�¼,[safid=%ld]",llSaf_id);
      /* modify by gf at 2004-04-26
      EXEC SQL SELECT * INTO :sgSwt_rev_saf
           FROM swt_rev_saf WHERE saf_id = :llSaf_id;
      */
      /* del by gengling 20150114 */
#if 0
      EXEC SQL SELECT * INTO 
        :sgSwt_rev_saf.saf_id:ilTmpind,
        :sgSwt_rev_saf.tran_id:ilTmpind,
        :sgSwt_rev_saf.proc_step:ilTmpind,
        :sgSwt_rev_saf.saf_begin:ilTmpind,
        :sgSwt_rev_saf.saf_overtime:ilTmpind,
        :sgSwt_rev_saf.rev_overtime:ilTmpind,
        :sgSwt_rev_saf.saf_num:ilTmpind,
        :sgSwt_rev_saf.saf_status:ilTmpind,
        :sgSwt_rev_saf.saf_flag:ilTmpind,
        :sgSwt_rev_saf.saf_file:ilTmpind,
        :sgSwt_rev_saf.saf_offset:ilTmpind,
        :sgSwt_rev_saf.saf_len:ilTmpind
        FROM swt_rev_saf
        WHERE saf_id = :llSaf_id;
      if(sqlca.sqlcode == SQLNOTFOUND)
      {
        swVdebug(2,"S0380: ��ʷSAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
        continue;
      }
      if(sqlca.sqlcode)
      {
        swVdebug(0,"S0390: [����/���ݿ�] ������ʷSAF��¼����,[safid=%ld]",llSaf_id);
        swDberror(NULL);
        continue;
      }
#endif
      /* end of del */

      /* add by gengling 20150114 */
      ilRc = swShmselect_swt_rev_saf(llSaf_id, &sgSwt_rev_saf);
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0380: ��ʷSAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
        continue;
      }
      if(ilRc == FAIL) 
      {
        swVdebug(0,"S0390: [����/�����ڴ�] ������ʷSAF��¼����,[safid=%ld]",llSaf_id);
        swDberror(NULL);
        continue;
      }
      /* end of add */

      llTran_id = sgSwt_rev_saf.tran_id;
      swVdebug(2,"S0400: �ҵ���ʷSAF��¼,[safid=%ld],[tranid=%ld]",llSaf_id,llTran_id);

      /* ȡ��SAF���� */
      memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
      _swTrim(sgSwt_rev_saf.saf_file);
      ilRc = swGetlog(sgSwt_rev_saf.saf_file,sgSwt_rev_saf.saf_len,sgSwt_rev_saf.saf_offset,(char *)&slMsgpack);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0410: [����/����] ȡ������ˮԭʼ����ʧ��");
        _swMsgsend(304011,NULL);
        continue;
      }
      swVdebug(2,"S0420: ȡ��SAF��ˮԭʼ����");

      /* �ñʽ�����ˮ����ʷ���Ƶ���ǰ�� */
      memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
      EXEC SQL SELECT * INTO
           :sgSwt_tran_log.tran_id:ilTmpind,
           :sgSwt_tran_log.tran_begin:ilTmpind,
           :sgSwt_tran_log.tran_status:ilTmpind,
           :sgSwt_tran_log.tran_end:ilTmpind,
           :sgSwt_tran_log.tran_overtime:ilTmpind,
           :sgSwt_tran_log.q_tran_begin:ilTmpind,
           :sgSwt_tran_log.tc_tran_begin:ilTmpind,
           :sgSwt_tran_log.q_target:ilTmpind,
           :sgSwt_tran_log.resq_file:ilTmpind,
           :sgSwt_tran_log.resq_offset:ilTmpind,
           :sgSwt_tran_log.resq_len:ilTmpind,
           :sgSwt_tran_log.resp_file:ilTmpind,
           :sgSwt_tran_log.resp_offset:ilTmpind,
           :sgSwt_tran_log.resp_len:ilTmpind,
           :sgSwt_tran_log.rev_key:ilTmpind
           FROM swt_his_tran_log WHERE tran_id = :llTran_id;
      if(sqlca.sqlcode == SQLNOTFOUND)
      {
        swVdebug(2,"S0430: ��ʷ�����޴˽�����ˮ,[tranid=%ld]",llTran_id);
        continue;
      }
      else if(sqlca.sqlcode)
      {
        swVdebug(0,"S0440: [����/���ݿ�] ������ʷ������ˮ����,[tranid=%ld]",llTran_id);
        swDberror((char *)&slMsgpack);
        continue;
      }
      swVdebug(2,"S0450: �ҵ���ʷ������ˮ,[tranid=%ld]",llTran_id);
      if(sgSwt_tran_log.tran_status != iTRNREVFAIL)
      {
        /* del by gengling 20150114 */
      #if 0
        EXEC SQL DELETE FROM swt_rev_saf WHERE saf_id = :llSaf_id;
        if(sqlca.sqlcode)
        {
          swVdebug(0,"S0460: [����/���ݿ�] ɾ����ʷSAF��¼����,[safid=%ld]",llSaf_id);
          EXEC SQL ROLLBACK WORK;
          swDberror(NULL);
        }
        EXEC SQL COMMIT WORK;
      #endif
        /* end of del */

        /* add by gengling 20150114 */
        ilRc = swShmdelete_swt_rev_saf(llSaf_id);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0460: [����/���ݿ�] ɾ����ʷSAF��¼����,[safid=%ld]",llSaf_id);
          swDberror(NULL);
        }
        /* end of add */

        continue;
      }  /* end modify */
      
      sgSwt_tran_log.tran_status = iTRNREVING;
      time(&sgSwt_tran_log.tran_begin);

      clLoc_flag='1';
      ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0470: [����/�����ڴ�] ׷�ӽ�����ˮ����,[tranid=%ld]",llTran_id);
        continue;
      }
      swVdebug(2,"S0480: ׷�ӽ�����ˮ�ɹ�,[tranid=%ld]",llTran_id);

#ifdef DB_INFORMIX
      EXEC SQL BEGIN WORK;
#endif
      EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTran_id;
      if(sqlca.sqlcode)
      {
        EXEC SQL ROLLBACK WORK;
        swVdebug(0,"S0490: [����/���ݿ�] ɾ����ʷ������ˮʧ��,[tranid=%ld]",llTran_id);
        swDberror(NULL);
        continue;
      }
      swVdebug(2,"S0500: ɾ����ʷ������ˮ�ɹ�,[tranid=%ld]",llTran_id);

      /* ��Ӧ�Ĵ�����ˮ����ʷ���Ƶ���ǰ�� */
      EXEC SQL SELECT MAX(proc_step) INTO :ilProc_step_max:ilTmpind
           FROM swt_his_proc_log WHERE tran_id = :llTran_id;
      if (sqlca.sqlcode)
      {
        EXEC SQL ROLLBACK WORK;
        swVdebug(0,"S0510: SELECT MAX(proc_step) FROM swt_his_proc_log ... error");
        swDberror(NULL);
        continue;
      }
      if(ilTmpind < 0)
      {
        EXEC SQL ROLLBACK WORK;
        swVdebug(2,"û���ҵ���ʷ������ˮ,[tranid=%ld]",llTran_id);
        continue;
      }

      ilMoveproc_flag = 0;
      for(ilProc_step = ilProc_step_max; ilProc_step > 0; ilProc_step --)
      {
        memset((char *)&sgSwt_proc_log,0x00,sizeof(sgSwt_proc_log));
        EXEC SQL SELECT * INTO
             :sgSwt_proc_log.tran_id:ilTmpind,
             :sgSwt_proc_log.proc_step:ilTmpind,
             :sgSwt_proc_log.proc_begin:ilTmpind,
             :sgSwt_proc_log.q_target:ilTmpind,
             :sgSwt_proc_log.proc_status:ilTmpind,
             :sgSwt_proc_log.rev_mode:ilTmpind,
             :sgSwt_proc_log.rev_fmtgrp:ilTmpind,
             :sgSwt_proc_log.org_file:ilTmpind,
             :sgSwt_proc_log.org_offset:ilTmpind,
             :sgSwt_proc_log.org_len:ilTmpind
             FROM swt_his_proc_log WHERE tran_id = :llTran_id
                                   AND   proc_step = :ilProc_step
                                   AND   proc_status = 5
                                   AND   (rev_mode = 1 OR rev_mode = 2);
        if(sqlca.sqlcode == SQLNOTFOUND) continue;
        if(sqlca.sqlcode)
        {
          EXEC SQL ROLLBACK WORK;
          swVdebug(0,"S0520: SELECT * FROM swt_his_proc_log WHERE ... error");
          swDberror(NULL);
          ilMoveproc_flag = -1;
          break;
        }

        if(ilMoveproc_flag == 0)
          sgSwt_proc_log.proc_status = iTRNREVING;

        ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'1');
        if (ilRc == FAIL)
        {
          EXEC SQL ROLLBACK WORK;
          swVdebug(0,"S0530: [����/�����ڴ�] ׷�Ӵ�����ˮ����,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          ilMoveproc_flag = -1;
          break;
        }
        swVdebug(2,"S0540: ׷�Ӵ�����ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

        EXEC SQL DELETE FROM swt_his_proc_log WHERE tran_id = :llTran_id
                                              AND   proc_step = :ilProc_step;
        if(sqlca.sqlcode)
        {
          EXEC SQL ROLLBACK WORK;
          swVdebug(0,"S0550: DELETE FROM swt_his_proc_log WHERE ... error");
          swDberror(NULL);
          ilMoveproc_flag = -1;
          break;
        }
        swVdebug(2,"S0560: ɾ����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

        ilMoveproc_flag ++;

      }
      swVdebug(2,"S0570: ������ˮ�ʹ�����ˮ����ʷ�Ƶ���ǰ���");
      continue;
    }
/*add by zcd 20150111*/
#else
    if(ilRc == SHMNOTFOUND) 
    {
        swVdebug(2,"S0111:�����ڴ�û���ҵ���Ҫ��ʱ����ˮ");
        sleep(ilWaitTime);
        continue;
    }
/*end of add */
#endif

    llTran_id = sgSwt_tran_log.tran_id;
    ilQtran_begin = sgSwt_tran_log.q_tran_begin;
    
    swVdebug(2,"S0580: �ҵ���ʱ����С������ˮ��¼,[tranid=%ld]",llTran_id);
 
    /* ECHOTEST����Ĳ�ѯ���� */
    if(sgSwt_tran_log.q_tran_begin == iMBECHOTEST)
    {
      swVdebug(2,"S0590: ECHOTEST����Ĳ�ѯ����");
      
      /* ȡĿ�Ķ˿����� */
      ilRc = swShmselect_swt_sys_queue(sgSwt_tran_log.q_target,&sgSwt_sys_queue);
      if(ilRc)
        swVdebug(0,"S0600: [����/��������] swShmselect_swt_sys_queue����! ȡĿ�Ķ˿�����ʧ��,[qid=%d,ilRc=%d]",sgSwt_tran_log.q_target,ilRc);
      else
      {
        ilRc = _swExpress(sgSwt_sys_queue.setdown_overtime,alResult,&ilLength);
        if((ilRc == SUCCESS) && (alResult[0] == '1'))
        {
          swPortset(sgSwt_tran_log.q_target,4,0); /* �˿�ECHOTEST��״̬��down */
          swVdebug(2,"S0610: �ö˿�ECHOTEST��״̬DOWN,[qid=%d]",sgSwt_tran_log.q_target);
        }  
      }   
      /* fzj: 2002.9.17: ���׳�ʱ���ط�199009���� */
      memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
      _swTrim(sgSwt_tran_log.resp_file);
      ilRc = swGetlog(sgSwt_tran_log.resp_file,sgSwt_tran_log.resp_len,
                    sgSwt_tran_log.resp_offset,(char *)&slMsgpack);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0620: [����/����] ȡ������ˮ��Ӧ����ʧ��");
        _swMsgsend(304011,NULL);
      }
      else
        _swMsgsend(199009,(char *)&slMsgpack);
      /**/

      /* �ý�����ˮ״̬Ϊ�����ɹ�,����ʷ��*/
      ilRc = swProcess_revend_tranlog(sgSwt_tran_log.tran_id,iTRNREVEND,clLoc_flag);
      /*begin add by gf at 2004-04-07*/
      ilRc=swSendRevresult_nounion(iMSGREVEND);
      /*end add*/
      continue;
    }

    /* �ý�����ˮ״̬Ϊ���ڳ��� */
    /* del by cy 20040324
    sgSwt_tran_log.tran_status = iTRNREVING;
    ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0630: [����/�����ڴ�] ���½�����ˮʧ��,[tranid=%ld]",llTran_id);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0640: ������ˮû���ҵ�,[tranid=%ld]",llTran_id);
      continue;
    }  
    end del */
    swVdebug(2,"S0650: �ý�����ˮ״̬Ϊ���ڳ���,[tranid=%ld]",llTran_id);
    swVdebug(2,"S0660: ���½�����ˮ�ɹ�,[tranid=%ld]",llTran_id);

    swVdebug(2,"S0670: ��ʼ����REVMODE=3��4�����...");
   
    /* �������REVMODE=3����4�����,��ʾ��Ҫ����֪ͨ����,���������Ƚ����� 
     * ����ȫ�����ͳ�ȥ.�����÷��ڽ��׳�ʱ���ظ�ԭ�������п���ʹ��.����� 
���� * ���÷�����:��·�ɽű��εĵ�һ�м����������(������ʾ):
     *    [27,0001333333] WITH REVMODE 4,REVGOUP 999
     *    10 CALL 28
     *    20 SEND 27 WITH END
     * ��������������ʱ,���س�������REVMODE 4�ڴ�����ˮ�м�¼һ��,
     * �������׳�ʱ֮��,�ɱ�����˴����ñ��ĸ�����Ӧ�ĸ�ʽת�������
     * ģ��ƥ���Գ������ĵ���ʽ���͸�ǰ�ó���.ǰ�ó�����Զ��������Ӧ
     * �Ĵ���. swMonrev�еļ�ʱ����Ҳ�����ƵĻ���.
     */

    ilRevstep=0;
    
    /* ����REVMODE=3��4�Ĵ�����ˮ��¼ */
    ilRc = swShmselect_swt_proc_log_mrec(llTran_id,pslSwt_proc_log,&ilCount_main); /* qy */
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0680: [����/�����ڴ�] ����REVMODE=3��4�Ĵ�����ˮʧ��");
      continue;
    }

    /* ѭ��ȡ��������ˮ��¼������֪ͨ���� */
    j = 0;
    for (i = 0; i < ilCount_main; i ++) /* qy */
    {
      /* ֻ�������ģʽΪ3��4�Ĵ�����ˮ */
      if(pslSwt_proc_log[i].rev_mode != 3 && pslSwt_proc_log[i].rev_mode != 4)
        continue;
      
      j++;  
      ilTran_step = pslSwt_proc_log[i].proc_step; 
      swVdebug(2,"S0690: �ҵ�REVMODE=3��4�Ĵ�����ˮ,[tranid=%ld,step=%d]",llTran_id,ilTran_step); 
       
      if(ilRevstep < pslSwt_proc_log[i].proc_step)
        ilRevstep = pslSwt_proc_log[i].proc_step;
    
      /* ȡ��ԭʼ���� */
      memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
      _swTrim(pslSwt_proc_log[i].org_file);
      ilRc = swGetlog(pslSwt_proc_log[i].org_file,pslSwt_proc_log[i].org_len,
        pslSwt_proc_log[i].org_offset,(char *)&slMsgpack);
      if(ilRc == FAIL) 
      {
        swVdebug(0,"S0700: [����/����] ȡ������ˮԭʼ����ʧ��");
        _swMsgsend(304011,NULL);
        continue;
      }
      swVdebug(2,"S0710: ȡ������ˮԭʼ���ĳɹ�");
 
      /* ��֪֯ͨ����ͷ */
      slMsgpack.sMsghead.iMsgtype = iMSGREV;
      slMsgpack.sMsghead.cSafflag = '0';
      slMsgpack.sMsghead.iRevfmtgroup = pslSwt_proc_log[i].rev_fmtgrp;
      slMsgpack.sMsghead.iDes_q = pslSwt_proc_log[i].q_target;
      slMsgpack.sMsghead.aSysattr[0] = clLoc_flag;

      if(cgDebug>=2)
      {
        swDebugmsghead((char *)&slMsgpack);
        swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      }
 
      /* ����֪ͨ���� */
      ilRc = qwrite2((char *)&slMsgpack,
        slMsgpack.sMsghead.iBodylen + sizeof(struct msghead),
        pslSwt_proc_log[i].q_target,ilPrior,ilClass,ilType);
      if(ilRc)
      {
	swVdebug(0,"S0720: [����/����] qwrite2()����,������=%d, ����֪ͨ���ĵ�ǰ�ý���ʧ��,[qid=%d]",ilRc, pslSwt_proc_log[i].q_target); 
 	_swMsgsend(304012,NULL);
        continue;
      }
      swVdebug(2,"S0730: ����֪ͨ���ĵ�ǰ�ý���,[qid=%d]",
        pslSwt_proc_log[i].q_target); 
      _swMsgsend(199003,(char *)&slMsgpack);  
 
      /* ���ú���swProcess_swt_proc_log(),
         �øñʴ�����ˮ״̬Ϊ��������,����ʷ,ɾ��֮ */
      ilRc = swProcess_proclog(llTran_id,ilTran_step,slMsgpack.sMsghead.aSysattr[0]);
      if(ilRc == FAIL) break;
    }
    if(ilRc == FAIL) continue;
    swVdebug(2,"S0740: ��ɶ�REVMODE=3��4�Ĵ���,��[%d]��",j); 

    /* ����REVMODE=1��2���������ˮ */
    ilTran_step = 0;
    ilRc = swShmselect_max_proclog_tran(llTran_id,&sgSwt_proc_log);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0750: [����/�����ڴ�] ����REVMODE=1��2���������ˮ����,[tranid=%ld]",llTran_id);
      continue;
    }
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0760: û���ҵ�REVMODE=1��2�Ĵ�����ˮ,�����ɹ�,[tranid=%ld]",llTran_id);
      /* �ñʽ�����ˮ״̬�ó�������,����ʷ,ɾ��֮ */   
      swProcess_revend_tranlog(llTran_id,iTRNREVEND,clLoc_flag); 
      /*begin add by gf at 2004-04-07*/
      ilRc=swSendRevresult_nounion(iMSGREVEND);
      /*end add*/
      continue;
    }
    
    /* ���д�����ˮ,���г��� */
    ilTran_step = sgSwt_proc_log.proc_step;
    swVdebug(2,"S0770: �ҵ�REVMODE=1��2���������ˮ,[tranid=%ld,step=%d]",llTran_id,ilTran_step);

    /* ȡ��ԭʼ���� */
    memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
    _swTrim(sgSwt_proc_log.org_file);
    swVdebug(0,"---------filename[%s]-------------------",sgSwt_proc_log.org_file);
    ilRc = swGetlog(sgSwt_proc_log.org_file,sgSwt_proc_log.org_len,
      sgSwt_proc_log.org_offset,(char *)&slMsgpack);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0780: [����/����] ȡ������ˮԭʼ����ʧ��");
      _swMsgsend(304011,NULL);
      continue;
    }
    swVdebug(2,"S0790: ȡ�ô�����ˮԭʼ����");
 
    /* ȡ�ö˿����� */
    ilRc = swShmselect_swt_sys_queue(sgSwt_proc_log.q_target, &sgSwt_sys_queue);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0800: [����/�����ڴ�] ȡ�˿�����ʧ��,[qid=%d]",sgSwt_proc_log.q_target);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0810: �˿�û������,[qid=%d]",sgSwt_proc_log.q_target);
      continue;
    }  
    swVdebug(2,"S0820: ȡ�˿����óɹ�,[qid=%d]",sgSwt_proc_log.q_target);
     
    /* ��֧��������� */ 
    if(sgSwt_sys_queue.rev_nogood[0] == cTRUE && ilRevstep < sgSwt_proc_log.proc_step)
    {
      swVdebug(2,"S0830: ��֧���������,�����ȴ�");
 
      /* �ô�����ˮ״̬Ϊ�����ȴ� */
      sgSwt_proc_log.proc_status = iTRNREVWAIT;
      ilRc = swShmupdate_swt_proc_log(llTran_id,ilTran_step,sgSwt_proc_log,clLoc_flag);
      if(ilRc == FAIL)
        swVdebug(0,"S0840: [����/�����ڴ�] ���´�����ˮʧ��,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      if(ilRc == SHMNOTFOUND)
        swVdebug(2,"S0850: ������ˮû���ҵ�,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      swVdebug(2,"S0860: ���´�����ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilTran_step);  
      continue;  
    }

    /* ֧��������� */
    /* �ô�����ˮ״̬Ϊ���ڳ��� */
    swVdebug(2,"S0870: ���´�����ˮ״̬Ϊ���ڳ���,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
    sgSwt_proc_log.proc_status = iTRNREVING;
    ilRc = swShmupdate_swt_proc_log(llTran_id,ilTran_step,sgSwt_proc_log,clLoc_flag);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0880: [����/�����ڴ�] ���´�����ˮʧ��,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0890: ������ˮû���ҵ�,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      continue;
    }  
    swVdebug(2,"S0900: ���´�����ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
     
    /* ȡ��SAF��¼�� */
    memset((char *)&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
    if(swShmgetsafid(&llSaf_id) == FAIL)
    {
      swVdebug(0,"S0910: [����/�����ڴ�] ȡSAF��ˮ��ʧ��");
      continue;
    }
    swVdebug(2,"S0920: ȡSAF��ˮ�ųɹ�,[safid=%ld]",llSaf_id);
 
    /* ��֯SAF����ͷ */
    slMsgpack.sMsghead.lTranid = llTran_id;
    slMsgpack.sMsghead.iTranstep = ilTran_step;
    slMsgpack.sMsghead.iMsgtypeorg = slMsgpack.sMsghead.iMsgtype;
    slMsgpack.sMsghead.iMsgtype = iMSGREV;
    slMsgpack.sMsghead.iDes_q = sgSwt_proc_log.q_target;
    slMsgpack.sMsghead.lSafid = llSaf_id;
    slMsgpack.sMsghead.cSafflag = '1';
    slMsgpack.sMsghead.iRevfmtgroup = sgSwt_proc_log.rev_fmtgrp;
    slMsgpack.sMsghead.aSysattr[0] = clLoc_flag;

    /* ����SAF���� */
    sgSwt_rev_saf.saf_id = llSaf_id;
    sgSwt_rev_saf.tran_id = llTran_id;
    sgSwt_rev_saf.proc_step = ilTran_step;
    time(&sgSwt_rev_saf.saf_begin);
    sgSwt_rev_saf.rev_overtime = sgSwt_sys_queue.rev_overtime;
    sgSwt_rev_saf.saf_overtime = sgSwt_rev_saf.saf_begin + 
      sgSwt_sys_queue.rev_overtime;
    if(sgSwt_sys_queue.rev_num <= 0)
      sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;
    else
      sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num - 1;
    sgSwt_rev_saf.saf_status[0] = '0';
    sgSwt_rev_saf.saf_flag[0] = '2';  
    memset(sgSwt_rev_saf.saf_file,0x00,sizeof(sgSwt_rev_saf.saf_file));
    sgSwt_rev_saf.saf_offset = 0;
    sgSwt_rev_saf.saf_len = sgSwt_proc_log.org_len;

    ilRc = swPutlog((char *)&slMsgpack, sgSwt_rev_saf.saf_len, 
      &sgSwt_rev_saf.saf_offset, sgSwt_rev_saf.saf_file);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0930: [����/����] ����SAF���ĳ���");
      continue;
    }
    swVdebug(2,"S0940: ����SAF���ĳɹ�");
 
    /* ׷��SAF��¼ */
//#ifdef DB_SUPPORT   /*nead review*/ /* del by gengling 20150115 one line */
    /* del by gengling 20150114 */
    // ilRc = swDbinsert_swt_rev_saf(sgSwt_rev_saf); /*unsolved*/
    /* end of del */
    /* add by gengling 20150114 */
    ilRc = swShminsert_swt_rev_saf(sgSwt_rev_saf);
    /* end of add */
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0950: [����/���ݿ�] ׷��SAF��¼����,[safid=%ld]",llSaf_id);
      continue;
    }
    swVdebug(2,"S0960: ׷��SAF��¼�ɹ�,[safid=%ld]",llSaf_id);
//#endif /* del by gengling 20150115 one line */

    sgSwt_tran_log.q_target = sgSwt_proc_log.q_target;
    ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0970: [����/�����ڴ�] ���½�����ˮʧ��,[tranid=%ld]",llTran_id);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0980: ������ˮû���ҵ�,[tranid=%ld]",llTran_id);
      continue;
    }  
    swVdebug(2,"S0990: �ý�����ˮĿ������Ϊ%d,[tranid=%ld]",sgSwt_tran_log.q_target,llTran_id);
    swVdebug(2,"S1000: ���½�����ˮ�ɹ�,[tranid=%ld]",llTran_id);
 
    /* ���ͳ���������ǰ�ó��� */
    swVdebug(3,"S1010: ���ͳ������ĵ�ǰ�ó���ʼ");
    if(cgDebug>=2)
    {
      swDebugmsghead((char *)&slMsgpack);
      swDebughex(slMsgpack.aMsgbody, slMsgpack.sMsghead.iBodylen);
    }
    ilRc = qwrite2((char *)&slMsgpack,sgSwt_proc_log.org_len,
      sgSwt_proc_log.q_target,ilPrior,ilClass,ilType);
    if(ilRc == FAIL)
    {
      swVdebug(2,"S1020: ���ͳ���������ǰ�ý���ʧ��,[qid=%d]",sgSwt_proc_log.q_target);
      swMberror(ilRc,(char *)&slMsgpack);
      _swMsgsend(304001,(char *)&slMsgpack); 
    }
    swVdebug(2,"S1030: ���ͳ���������ǰ�ý��̳ɹ�,[qid=%d]",sgSwt_proc_log.q_target);
    _swMsgsend(199003,(char *)&slMsgpack);
  } 
}

/**************************************************************
 ** ������      �� swProcess_proclog
 ** ��  ��      �� ��ĳ�ʴ�����ˮ״̬Ϊ��������,
 **             :  ����ʷ,ɾ��֮ 
 ** ��  ��      ��  
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/11/19
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� 0-�ɹ� / 1-ʧ��
***************************************************************/
int swProcess_proclog(long lTran_id,short iProc_step,char cLoc_flag)
{
  swVdebug(4,"S1040: [��������] swProcess_proclog(%ld,%d,%c)",
           lTran_id,iProc_step,cLoc_flag);
  swVdebug(2,"S1050: �ô˱ʴ�����ˮ״̬Ϊ��������,����ʷ,ɾ��֮");
  /* �ô�����ˮ״̬Ϊ�������� */
  pslSwt_proc_log[i].proc_status = iTRNREVEND;
  ilRc = swShmupdate_swt_proc_log(lTran_id,iProc_step,pslSwt_proc_log[i],cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1060: [����/�����ڴ�] ���´�����ˮ����,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1070: ������ˮû���ҵ�,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
    return (FAIL);
  }
  swVdebug(2,"S1080: ���´�����ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,
    iProc_step);

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
  EXEC SQL BEGIN WORK;
#endif
  /* ������ˮ�Ƶ���ʷ */
  memcpy(&slSwt_proc_log,&pslSwt_proc_log[i],sizeof(struct swt_proc_log));
  EXEC SQL INSERT INTO swt_his_proc_log VALUES
    (:slSwt_proc_log.tran_id,
     :slSwt_proc_log.proc_step,
     :slSwt_proc_log.proc_begin,
     :slSwt_proc_log.q_target,
     :slSwt_proc_log.proc_status,
     :slSwt_proc_log.rev_mode,
     :slSwt_proc_log.rev_fmtgrp,
     :slSwt_proc_log.org_file,
     :slSwt_proc_log.org_offset,
     :slSwt_proc_log.org_len);

  if(sqlca.sqlcode != 0)
  {
    swVdebug(0,"S1090: [����/���ݿ�] ׷����ʷ������ˮ����,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S1100: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,
    iProc_step);
#else
  sw_put_proclog(&slSwt_proc_log);
#endif
  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_proc_log(lTran_id,iProc_step,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1110: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
#ifdef DB_SUPPORT
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1120: ������ˮû���ҵ�,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1130: ɾ��������ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,
    iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif   
  swVdebug(4,"S1140: [��������] swProcess_proclog()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      �� swProcess_revend_tranlog
 ** ��  ��      �� ��ĳ�ʽ�����ˮ״̬Ϊ�������������ʧ��,
 **             :  ����ʷ,ɾ��֮ 
 ** ��  ��      ��  
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/11/19
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� 0-�ɹ� / 1-ʧ��
***************************************************************/
int swProcess_revend_tranlog(long lTran_id,short iStatus,char cLoc_flag)
{
  swVdebug(4,"S1150: [��������] swProcess_revend_tranlog(%ld,%d,%c)",
           lTran_id,iStatus,cLoc_flag);
  swVdebug(2,"S1160: �ô˱ʽ�����ˮ״̬Ϊ��������,����ʷ,ɾ��֮");
  /* ���½�����ˮ״̬ */
  ilRc = swShmupdate_setstat_tranlog(lTran_id,iStatus,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1170: [����/�����ڴ�] ���½�����ˮʧ��,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1180: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1190: ���½�����ˮ�ɹ�,[tranid=%ld]",lTran_id);


  /* ������ˮ����ʷ */
  ilRc = swShmselect_swt_tran_log(lTran_id,&sgSwt_tran_log,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1200: [����/�����ڴ�] ���ҽ�����ˮʧ��,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1210: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1220: ���ҽ�����ˮ�ɹ�,[tranid=%ld]",lTran_id);


#ifdef MOVELOGFLAG

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
  if(sqlca.sqlcode != 0)
  {
    swVdebug(0,"S1230: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld]",lTran_id);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S1240: ׷����ʷ������ˮ�ɹ�,[tranid=%ld]",lTran_id);
/*add by zcd 20141225*/
#else
  sw_put_tranlog(&sgSwt_tran_log);
#endif
#endif 
/*end of add by zcd 20141225*/
  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_tran_log(lTran_id,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1250: [����/�����ڴ�] ɾ��������ˮʧ��,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1260: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1270: ɾ��������ˮ�ɹ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/  
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S1280: [��������] swProcess_revend_tranlog()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      �� swProcess_revend_proclog
 ** ��  ��      �� ��ĳ�ʽ��׵����д�����ˮ״̬Ϊ����������
 **             :  ����ʧ��,����ʷ,ɾ��֮
 ** ��  ��      ��  
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/11/19
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� 0-�ɹ� / 1-ʧ��
***************************************************************/
int swProcess_revend_proclog(long lTran_id,short iStatus,char cLoc_flag)
{
  swVdebug(4,"S1290: [��������] swProcess_revend_proclog(%ld,%d)",lTran_id,iStatus);
  /* ���´�����ˮ״̬ */
  ilRc = swShmupdate_setstat_proclog(lTran_id,iStatus,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1300: [����/�����ڴ�] ���´�����ˮʧ��,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1310: ���״���û���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1320: ���´�����ˮ�ɹ�,[tranid=%ld]",lTran_id);
  
  /* ������ˮ�Ƶ���ʷ */
  ilRc = swShmselect_swt_proc_log_mrec(lTran_id,pslSwt_proc_log,&ilCount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1330: [����/�����ڴ�] ���Ҵ�����ˮʧ��,[tranid=%ld]",lTran_id);
    return (FAIL);
  }  
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1340: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }  
  swVdebug(2,"S1350: ���Ҵ�����ˮ�ɹ�,[tranid=%ld]",lTran_id);

#ifdef DB_INFORMIX
  EXEC SQL BEGIN WORK;
#endif
  for (i = 0; i < ilCount; i ++)
  {
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    memcpy(&slSwt_proc_log,&pslSwt_proc_log[i],sizeof(struct swt_proc_log));
    EXEC SQL INSERT INTO swt_his_proc_log VALUES
      (:slSwt_proc_log.tran_id,
       :slSwt_proc_log.proc_step,
       :slSwt_proc_log.proc_begin,
       :slSwt_proc_log.q_target,
       :slSwt_proc_log.proc_status,
       :slSwt_proc_log.rev_mode,
       :slSwt_proc_log.rev_fmtgrp,
       :slSwt_proc_log.org_file,
       :slSwt_proc_log.org_offset,
       :slSwt_proc_log.org_len);

    if(sqlca.sqlcode != 0)
    {
      swVdebug(0,"S1360: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld]",lTran_id);
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return (FAIL);
    }
#else
	sw_put_proclog(&pslSwt_proc_log[i]);
#endif
    swVdebug(2,"S1370: ׷����ʷ������ˮ�ɹ�,[tranid=%ld]",lTran_id);
  }  
 
  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_proc_log_mrec(lTran_id,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1380: [����/�����ڴ�] ɾ��������ˮʧ��,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1390: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1400: ɾ��������ˮ�ɹ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
  EXEC SQL COMMIT WORK;
#endif

  swVdebug(4,"S1410: [��������] swProcess_revend_proclog()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      �� swQuit
 ** ��  ��      �� �źŴ�����
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/11/19
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/
void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  
  /* �ر����ݿ����� */
  swDbclose();

  /* �ر��������� */
  ilRc=qdetach();
  if(ilRc<0)
  {
    swVdebug(0,"S1420: [����/����] qdetach()����,������=%d",ilRc);
    exit(FAIL);
  }
  swVdebug(2,"S1430: �ر��������ӳɹ�");
  swVdebug(2,"S1440: [swMonlog]�Ѿ�ֹͣ...");

  exit(sig);
}

/*begin add by gf at 2004-04-07*/

/**************************************************************
 ** ������      �� swSendRevresult_nounion
 ** ��  ��      �� ���ͳ��������Դ���˿�
 **             :  
 ** ��  ��      ��  
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/11/19
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� 0-�ɹ� / 1-ʧ��
***************************************************************/
int swSendRevresult_nounion(short iMsgtype)
{
  short ilRc;
  struct msgpack slMsgpack1;
  
  memset((char *)&slMsgpack1,0x00,sizeof(slMsgpack1));
  _swTrim(sgSwt_tran_log.resp_file);
  ilRc = swGetlog(sgSwt_tran_log.resp_file,sgSwt_tran_log.resp_len,
  sgSwt_tran_log.resp_offset,(char *)&slMsgpack1);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0630: [����/����] ȡ������ˮ��Ӧ����ʧ��");
    return(FAIL);
  }
    
  /*����·�������������
   *9999 WITH REVMODE 4
   */
  ilRc=swShmselect_route_d_last(slMsgpack1.sMsghead.iBegin_q_id,slMsgpack1.sMsghead.aTrancode);
  if(ilRc)
  {
    swVdebug(2,"S9000: û�з��س������·�����");
    return(FAIL);
  }

  slMsgpack1.sMsghead.iMsgtype = iMsgtype;
  
  /* ���ͳ������֪ͨ���� */
  ilRc = qwrite2((char *)&slMsgpack1,  \
     slMsgpack1.sMsghead.iBodylen + sizeof(struct msghead), \
        slMsgpack1.sMsghead.iBegin_q_id,0,0,0);
  if(ilRc)
  {
    swVdebug(0,"S0720: [����/����] qwrite2()����,������=%d, ����֪ͨ���ĵ�ǰ�ý���ʧ��,[qid=%d]",ilRc, slMsgpack1.sMsghead.iBegin_q_id); 
    return(FAIL);
  }
  swVdebug(2,"S0730: ����֪ͨ���ĵ�ǰ�ý��̳ɹ�,[qid=%d]",slMsgpack1.sMsghead.iBegin_q_id);
  return(SUCCESS);
}
/*end add*/     
