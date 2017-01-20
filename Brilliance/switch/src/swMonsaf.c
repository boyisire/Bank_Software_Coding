/****************************************************************/
/* ģ����    ��MONSAF                                         */ 
/* ģ������    ��SAF����                                        */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ���Ż�                                           */
/* ��������    ��1999/11/22                                     */
/* ����޸����ڣ�2001/3/24                                      */
/* ģ����;    ���ҵ�SAF��ʱ�Ľ��ף�����SAF����                 */
/* ��ģ���а������º���������˵����                             */
/*		  (1) void main();	                        */
/*		  (2) void swQuit();  		                */
/*                (3) int swProcess_revend_tranlog(long,short); */
/*                (4) int swProcess_revend_proclog(long,short); */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*   1999/12/15 �����˶�Ӧ�ñ���SAF�Ĵ���                       */
/*   1999/12/27 �޸��˶�ͬһtranid���saf��¼�Ĵ���             */
/*   2000/1/28  �������Ŷ��ڵ���ƽ������޸�                    */
/*   2000/3/17  SAFֻ��Ҫȡ��SAF����,ԭ�ⲻ���ķ���             */
/*   2000/4/4   �Ż�                                            */
/*   2001/3/15  �����ȶ���                                      */
/*   2001/3/24  ���V3.01,����1178��                          */
/*   2001/4/27  �޸���ѭ����BUG(1217)                           */
/****************************************************************/

/* switch���� */
#include "switch.h"


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

/* POSTGRES���ݿ�֧��(�ṹ�������¶���) */
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/

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
/*����64λdb2������������ݿ�*/
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
/*add by zcd 20141225*/
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
/*end of add*/
/* �������� */
short  i,j,ilRc,ilCount;
struct swt_proc_log pslSwt_proc_log[iMAXPROCNUM];

/* ����ԭ�� */
void swQuit(int);
int swProcess_revend_tranlog(long,short,char);
int swProcess_revend_proclog(long,short,char);
int swProcess_revfail_revsaf(long);

extern void  swLogInit(char*,char*);
extern int swShmselect_overtime_min_revsaf(long lTime, struct swt_rev_saf *psSwt_rev_saf);
extern int swShmupdate_swt_rev_saf(long lSaf_id, struct swt_rev_saf sSwt_rev_saf);
extern short swShmselect_route_d_last(short iQid, char *aTran_code);

/**************************************************************
 ** ������      �� main
 ** ��  ��      �� ������
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/11/22
 ** ����޸����ڣ� 2000/4/5
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/
int main(int argc,char *argv[])
{
  /* �������� */
  long  llCurrenttime;				/* ϵͳ��ǰʱ�� */
  short ilWaitTime;				/* ��ѭ�ȴ�ʱ�� */
  long  llSaf_id;				/* SAF��¼�� */
  long  llTran_id;				/* ������ˮ�� */
  short ilQ_id;					/* Ŀ������� */
  short  ilPrior = 0,ilClass = 0,ilType = 0;	/* ������� */
  struct msgpack slMsgpack;			/* ���� */

  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swMonsaf Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swMonsaf")) exit(FAIL);

  /* ���õ��Գ������� */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swMonsaf.debug");

  /* ȡ��DEBUG��־ */
  if((cgDebug=_swDebugflag("swMonsaf"))==FAIL)
  {
    fprintf(stderr,"ȡDEBUG��־ʧ��!\n");
    exit(FAIL);
  } 
  swLogInit("swMonsaf", "swMonsaf.debug");
  swVdebug(2,"S0010: �汾��[4.3.0]");
  swVdebug(2,"S0020: DEBUG�ļ�[%s]",agDebugfile);
  swVdebug(2,"S0030: DEBUG��־[%d]",cgDebug);
 
  /* ������ */
  if(qattach(iMBMONREV)) 
  {
    swVdebug(0,"S0040: [����/����] qattach()����,������=-1,������ʧ��");
    exit(FAIL);
  }
  swVdebug(2,"S0050: ������ɹ�");
 
  /* �����ݿ� */
  if(swDbopen()==FAIL)
  {
    swVdebug(0,"S0060: [����/���ݿ�] �����ݿ�ʧ��");
    swQuit(FAIL);
  } 
  swVdebug(2,"S0070: �����ݿ�ɹ�");
 
  /* �ź����κ����� */
  signal(SIGTERM,swQuit);
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);

  /* ��ʼ�������ڴ�ָ�� */
  if(swShmcheck() == FAIL)
  {
    swVdebug(0,"S0080: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    swQuit(FAIL);
  }
  swVdebug(2,"S0090: ��ʼ�������ڴ�ָ��ɹ�");
 
  /* ����ϵͳ�����Ϣ���� */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: ����104001��Ϣ����,[swMonsaf]�Ѿ�����...");
 
  while(TRUE) 
  {
    /* ˢ�¹����ڴ�״̬ */
    if(swShmcheck() == FAIL) 
    {
      swVdebug(0,"S0110: [����/�����ڴ�] ˢ�¹����ڴ�ָ��ʧ��");
      swQuit(FAIL);
    }
  
    /* ȡ��ϵͳ��ѭʱ�� */
    ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
    if(ilRc)
    {
      swVdebug(0,"S0120: [����/�����ڴ�] ȡϵͳ��ѭʱ��ʧ��");
      swQuit(FAIL);
    }
    ilWaitTime = sgSwt_sys_config.saf_inter;

    /* ���ҳ�ʱ����СSAF��¼��¼ */
    time(&llCurrenttime);
	
/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/
    /* ilRc = swDbselect_overtime_min_revsaf(llCurrenttime,&sgSwt_rev_saf); */
	/* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	ilRc = swShmselect_overtime_min_revsaf(llCurrenttime,&sgSwt_rev_saf);
    if(ilRc == FAIL) 
    {
      swVdebug(0,"S0130: [����/�����ڴ�] ���ҳ�ʱ����СSAF��¼��¼����");
      swQuit(FAIL);
    }
/* #endif */ /* del by gengling at 2015.04.03 PSBC_V1.0 */
    if(ilRc == SHMNOTFOUND) 
    {
      sleep(ilWaitTime);
      continue;
    }
    llSaf_id = sgSwt_rev_saf.saf_id;
    llTran_id = sgSwt_rev_saf.tran_id;
    swVdebug(2,"S0140: �ҵ���ʱ����СSAF��¼��¼,[tranid=%ld,safid=%ld]",llTran_id,llSaf_id);
 
    /* ȡ��SAF���� */
    _swTrim(sgSwt_rev_saf.saf_file);
    memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
    ilRc = swGetlog(sgSwt_rev_saf.saf_file,sgSwt_rev_saf.saf_len,sgSwt_rev_saf.saf_offset,(char *)&slMsgpack);
    if(ilRc)
    {
      swVdebug(0,"S0150: [����/����] ȡSAF����ʧ��");
      _swMsgsend(304010,NULL);
      goto LSAFFAIL;
    }
    swVdebug(2,"S0160: ȡSAF���ĳɹ�");
 
    ilQ_id = slMsgpack.sMsghead.iDes_q;

    if(sgSwt_rev_saf.saf_flag[0] != '1')
      swVdebug(2,"S0170: Ϊ����SAF����");
    else
      swVdebug(2,"S0180: ΪӦ��SAF����");

    /* �����µ�SAF��ʱʱ�� */
    time(&llCurrenttime);
    sgSwt_rev_saf.saf_overtime = llCurrenttime + sgSwt_rev_saf.rev_overtime;
 
    /* һֱ��CASE */
    if(sgSwt_rev_saf.saf_num == -1)
    {
      swVdebug(2,"S0190: �ط�����[-1],һֱ��"); 

/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/            
      /* ilRc = swDbupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf); */
      /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0200: [����/�����ڴ�] ����SAF��¼ʧ��,[safid=%ld]",llSaf_id);
 	continue;
      }
/* #endif */ /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0210: SAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
 	continue;
      }
      if(cgDebug>=2)
      {
        swDebugmsghead((char *)&slMsgpack);
        swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      }
      /* ���ͳ������ĵ�ǰ�ý��� */
      ilRc = qwrite2((char *)&slMsgpack,sgSwt_rev_saf.saf_len,ilQ_id,ilPrior,ilClass,ilType);
      if(ilRc)
      {
        swVdebug(0,"S0220: [����/����] qwrite2()����,������=%d, ���ͳ������ĵ�ǰ�ý���ʧ��,[qid=%d]",ilRc,ilQ_id);
        swMberror(ilRc,(char *)&slMsgpack);
        _swMsgsend(304001,(char *)&slMsgpack); 
      }
      swVdebug(2,"S0230: ���ͳ������ĵ�ǰ�ý��̳ɹ�,[qid=%d]",ilQ_id);
      _swMsgsend(199003,(char *)&slMsgpack);
      continue;
    }

    if(sgSwt_rev_saf.saf_num > 0)
    {
      swVdebug(2,"S0240: �ط�����[%d],������",sgSwt_rev_saf.saf_num);
      
      /* �����µ�SAF�ط����� */
      sgSwt_rev_saf.saf_num = sgSwt_rev_saf.saf_num - 1;

/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */ 
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/     
      /* ����SAF��¼ */
      /* ilRc = swDbupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf); */
	  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0250: [����/�����ڴ�] ����SAF��¼ʧ��,[safid=%ld]",llSaf_id);
        continue;
      }  
/* #endif */ /* del by gengling at 2015.04.03 one line PSBC_V1.0 */

      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0260: SAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
        continue;
      }  
      swVdebug(2,"S0270: ����SAF��¼�ɹ�,[safid=%d]",llSaf_id);
      if(cgDebug>=2)
      {
        swDebugmsghead((char *)&slMsgpack);
        swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      }
      /* ���ͱ��ĵ�ǰ�ý��� */
      ilRc = qwrite2((char *)&slMsgpack,sgSwt_rev_saf.saf_len,ilQ_id,ilPrior,ilClass,ilType);
      if(ilRc)
      {
        swVdebug(0,"S0280: [����/����] qwrite2()����,������=%d, ���ͳ������ĵ�ǰ�ý���ʧ��,[qid=%d]",ilRc,ilQ_id);
        swMberror(ilRc,(char *)&slMsgpack);
        _swMsgsend(304001,(char *)&slMsgpack); 
      }
      swVdebug(2,"S0290: ���ͳ������ĵ�ǰ�ý��̳ɹ�,[qid=%d]",ilQ_id);
      _swMsgsend(199003,(char *)&slMsgpack);
      continue;
    }

    swVdebug(2,"S0300: �ط�����[0],����ʧ��");

LSAFFAIL:
    /* SAFʧ�ܴ��� */
    ilRc = swProcess_revfail_revsaf(llSaf_id);
    if(ilRc == FAIL) continue;
 
    /* ����ǳ���saf,������ˮ�ʹ�����ˮ����ʷ */
    if(sgSwt_rev_saf.saf_flag[0] != '1')
    {
      /* �øñʽ�����ˮ״̬Ϊ����ʧ��,����ʷ,ɾ��֮ */
      ilRc = swProcess_revend_tranlog(llTran_id,iTRNREVFAIL,'a');
      /*delete by gf for repeated rev respond to original port at 2004-04-07
        ilRc=swSendRevresult(slMsgpack,iMSGREVFAIL);
      */
      if(ilRc == FAIL) continue;
      
      /* �øñʽ��׵����д�����ˮ״̬Ϊ����ʧ��,����ʷ,ɾ��֮ */
      ilRc = swProcess_revend_proclog(llTran_id,iTRNREVFAIL,'a');
      if(ilRc == FAIL) continue;
    }
  } 
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
int swProcess_revend_tranlog(long lTran_id,short iStatus,char aLoc_flag)
{
  swVdebug(4,"S0310: [��������] swProcess_revend_tranlog(%ld,%d)",lTran_id,iStatus);

  /* ���½�����ˮ״̬ */
  ilRc = swShmupdate_setstat_tranlog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0320: [����/�����ڴ�] ���½�����ˮ����,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0330: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S0340: �ý�����ˮ״̬,���½�����ˮ�ɹ�,[tranid=%ld]",lTran_id);
  
  
  /* ������ˮ�Ƶ���ʷ */
  ilRc = swShmselect_swt_tran_log(lTran_id,&sgSwt_tran_log,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0350: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0360: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  
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
    swVdebug(0,"S0370: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld]",lTran_id);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S0380: ׷����ʷ������ˮ�ɹ�,[tranid=%ld]",lTran_id);
#endif

  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_tran_log(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0390: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0400: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S0410: ɾ��������ˮ�ɹ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S0420: [��������] swProcess_revend_tranlog()������=0");
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
int swProcess_revend_proclog(long lTran_id,short iStatus,char aLoc_flag)
{
  swVdebug(4,"S0430: [��������] swProcess_revend_proclog(%ld,%d)",lTran_id,iStatus);

  /* ���´�����ˮ״̬ */
  ilRc = swShmupdate_setstat_proclog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0440: [����/�����ڴ�] ���´�����ˮ����,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0450: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S0460: �ô�����ˮ״̬,���´�����ˮ�ɹ�,[tranid=%ld]",lTran_id);
  
  /* ������ˮ�Ƶ���ʷ */
  ilRc = swShmselect_swt_proc_log_mrec(lTran_id,pslSwt_proc_log,&ilCount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0470: [����/�����ڴ�] ���Ҵ�����ˮ����,[tranid=%ld]",lTran_id);
    return (FAIL);
  }  
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0480: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }  




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
      swVdebug(0,"S0490: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return (FAIL);
    }
#endif
    swVdebug(2,"S0500: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
  }  
 
  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_proc_log_mrec(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0510: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0520: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S0530: ɾ��������ˮ�ɹ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S0540: [��������] swProcess_revend_proclog()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      �� swProcess_revfail_revsaf
 ** ��  ��      �� ��ĳ��SAF��¼״̬ΪSAFʧ��,
 **             :  �����ݿ��,ɾ��֮ 
 ** ��  ��      ��  
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/11/19
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� 0-�ɹ� / 1-ʧ��
***************************************************************/
int swProcess_revfail_revsaf(long lSaf_id)
{
  swVdebug(4,"S0550: [��������] swProcess_revfail_revsaf(%ld)",lSaf_id);
  /* ����SAF��¼״̬ */
  sgSwt_rev_saf.saf_status[0] = '2';	/* SAFʧ�� */
/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */  /*support database,Database Deprivation Project 2014-12*/

  /* ����SAF��¼ */
  /* ilRc = swDbupdate_swt_rev_saf(lSaf_id,sgSwt_rev_saf); */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  ilRc = swShmupdate_swt_rev_saf(lSaf_id,sgSwt_rev_saf);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0560: [����/�����ڴ�] ����SAF��¼����,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
/* #endif */ /* del by gengling at 2015.04.03 one line PSBC_V1.0 */

  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0570: SAF��¼û���ҵ�,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
  swVdebug(2,"S0580: ��SAF��¼״̬ΪSAFʧ��,[safid=%ld]",lSaf_id);
  swVdebug(2,"S0590: ����SAF��¼�ɹ�,[safid=%ld]",lSaf_id);
  swVdebug(4,"S0600: [��������] swProcess_revfail_revsaf()������=0");
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
  
  swVdebug(2,"S0610: [swMonsaf]�Ѿ�ֹͣ...");
  swDbclose();
  ilRc=qdetach();
  if(ilRc)
  {
    swVdebug(0,"S0620: [����/����] qdetach()����,������=%d",ilRc);
    exit(FAIL);
  }
  exit(sig);
}

/*begin add by gf at 2004-04-07*/
/**************************************************************
 ** ������      �� swSendRevresult
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
int swSendRevresult(struct msgpack sMsgpack,short iMsgtype)
{
  short ilRc;
  
  /*����·�������������
   *9999 WITH REVMODE 4
   */
  ilRc=swShmselect_route_d_last(sMsgpack.sMsghead.iBegin_q_id,sMsgpack.sMsghead.aTrancode);
  if(ilRc)
  {
    swVdebug(2,"S9000: û�з��س������·�����");
    return(FAIL);
  }

  sMsgpack.sMsghead.iMsgtype = iMsgtype;
  
  /* ���ͳ������֪ͨ���� */
  ilRc = qwrite2((char *)&sMsgpack,
     sMsgpack.sMsghead.iBodylen + sizeof(struct msghead), 
        sMsgpack.sMsghead.iBegin_q_id,0,0,0);
  if(ilRc)
  {
    swVdebug(0,"S0720: [����/����] qwrite2()����,������=%d, ����֪ͨ���ĵ�ǰ�ý���ʧ��,[qid=%d]",ilRc, sMsgpack.sMsghead.iBegin_q_id); 
    return(FAIL);
  }
  swVdebug(2,"S0730: ����֪ͨ���ĵ�ǰ�ý��̳ɹ�,[qid=%d]",sMsgpack.sMsghead.iBegin_q_id);
  return(SUCCESS);
}
/*end add*/     
