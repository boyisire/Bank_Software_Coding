/****************************************************************/
/* ģ����    ��MONREV                                         */ 
/* ģ������    �����������Դ�����                           */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ���Ż�                                           */
/* ��������    ��1999/11/22                                     */
/* ����޸����ڣ�2001/3/24                                      */
/* ģ����;    �����������Ӧ,��ʱ����,ֹͣ����                 */
/* ��ģ���а������º���������˵����                             */
/*		  (1) void main();                     		*/
/*                (2) void swQuit();                   		*/
/*                (3) int swProcess_proclog(long,short)	        */
/*                (4) int swProcess_revend_tranlog(long,short); */
/*                (5) int swProcess_revend_proclog(long,short); */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*    1999/12/21 �����˶�Ӧ��SAF��Ӧ���ĵĴ���                  */
/*    2000/1/29  �������Ŷ��ڵ���Ƽ����޸�                     */
/*    2000/4/5   �Ż�                                           */
/*    2000/8/30  �Ż�                                           */
/*    2001/3/12  Oracle��ֲ                                     */
/*    2001/3/23  ���V3.01,����2720��                         */
/****************************************************************/

/* switch���� */
#include "switch.h"

/* �������� */
#define   DEBUG          1                   /* ������Ϣ���� */

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

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* POSTGRES���ݿ�֧��(�ṹ�������¶���) */
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
EXEC SQL BEGIN DECLARE SECTION;
  struct swt_proc_log_sql
  {
    sqlint64  tran_id;
    short proc_step;
    sqlint64  proc_begin;
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
    sqlint64  org_offset;
    short org_len;
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
/*add by zcd 20141224*/
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
/*end of add by zcd 20141224*/
short  i,j,ilRc,ilCount;
struct swt_proc_log pslSwt_proc_log[iMAXPROCNUM];

/* ����ԭ�Ͷ��� */
void swQuit(int);
int swProcess_proclog(long,short,char);
int swProcess_revend_tranlog(long,short,char,struct swt_proc_log,struct msgpack);
int swProcess_revend_proclog(long,short,char);
int swProcess_revfail_revsaf(long);
int swRevSuccess(struct msgpack msgpack);
/*begin add by gf at 2004-04-07*/
int swSendRevresult_nounion(short);
int swSendRevresult_union(long,struct msgpack,short,char);
/*end add*/

extern void swLogInit(char *,char*);
extern int swShmselect_swt_rev_saf(long lSaf_id, struct swt_rev_saf *psSwt_rev_saf);
extern int swShmdelete_swt_rev_saf(long lSaf_id);
extern short sw_put_proclog(struct swt_proc_log *proc_log);
extern int swShminsert_swt_rev_saf(struct swt_rev_saf sSwt_rev_saf);

/**************************************************************
 ** ������      �� main
 ** ��  ��      �� ������
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/11/22
 ** ����޸����ڣ� 1999/4/5
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/
int main(int argc,char **argv)
{
  unsigned int    ilMsglen;                          /* ���ĳ��� */
  short   ilOrigQid;           /* ԭ������� */
  short   ilPrior=0,ilClass=0,ilType=0;      /* ������� */
  struct  msgpack slMsgpack;                 /* ���� */
  short   ilTran_step;                       /* ����ͷ���ײ��� */
  long    llCurrenttime;                     /* ϵͳ��ǰʱ�� */
  short   ilFlag = 0;
  char    clLoc_flag;

  /* 2002.11.13 fzj add */
  struct  swt_proc_log slSwt_proc_log1;               /* ������ˮ */
  union  preunpackbuf pulPrebuf1[iBUFFLDNUM]; /* Ԥ��� */
  union  preunpackbuf pulPrebuf2[iBUFFLDNUM]; /* Ԥ��� */
  /**/

  /* 2003.02.23 gxz add */
  struct  swt_proc_log slSwt_proc_log_view;           /* ������ˮ */
  /**/
  
  /* ���ݿ�������� */
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  llSaf_id;                          /* SAF��ˮ�� */
    sqlint64  llTran_id;                         /* ��ˮ�� */
    short ilProc_step;                       /* ���ײ��� */
    short ilProc_step_next;                  /* ��һ���ײ��� */
    short ilProc_step_max;
    short ilTranstat;			     /* ����״̬ */			
    short ilTmp;			     /* ָʾ���� */                       
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llSaf_id;                          /* SAF��ˮ�� */
    long  llTran_id;                         /* ��ˮ�� */
    short ilProc_step;                       /* ���ײ��� */
    short ilProc_step_next;                  /* ��һ���ײ��� */
    short ilProc_step_max;
    short ilTranstat;			     /* ����״̬ */			
    short ilTmp;			     /* ָʾ���� */                       
  EXEC SQL END DECLARE SECTION;
#endif
/*add by zcd 20141224*/
#else
	long  llSaf_id; 						 /* SAF��ˮ�� */
	long  llTran_id;						 /* ��ˮ�� */
	short ilProc_step;						 /* ���ײ��� */
	short ilProc_step_next; 				 /* ��һ���ײ��� */
	short ilProc_step_max;
	short ilTranstat;				 /* ����״̬ */ 		
	short ilTmp;
#endif
/*end of add by zcd 20141224*/
  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swMonrev Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swMonrev")) exit(FAIL);
  
  /* ���õ��Գ������� */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swMonrev.debug");

  /* ȡ��DEBUG��־ */
  if((cgDebug=_swDebugflag("swMonrev"))==FAIL)
  {
    fprintf(stderr,"ȡDEBUG��־ʧ��!\n");
    exit(FAIL);
  } 
  swLogInit("swMonrev", "swMonrev.debug");
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
 
  /* ��ϵͳ��ط�����Ϣ���� */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: ����104001��Ϣ����,[swMonrev]�Ѿ�����...");
 
  /* ѭ������ */
  while (TRUE)
  {	
    ilMsglen = sizeof(struct msgpack);
    ilPrior = ilClass = ilType = 0;
    ilRc = qread2((char *)&slMsgpack,&ilMsglen,&ilOrigQid,&ilPrior,&ilClass,&ilType);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0110: [����/����] qread2()����,������=%d,���������",ilRc);
      swMberror(ilRc,(char *)&slMsgpack);
      swQuit(FAIL);
    }
    swVdebug(2,"S0120: ������ɹ�");
 
    /* ˢ�¹����ڴ�ָ�� */
    if(swShmcheck() == FAIL) 
    {
      swVdebug(0,"S0130: [����/�����ڴ�] ˢ�¹����ڴ�ָ��ʧ��");
      swQuit(FAIL);
    }
    
    swVdebug(3,"S0140: ����[%d]��������,��������Ϊ[%d],���ĳ���Ϊ[%d]",
      ilOrigQid,slMsgpack.sMsghead.iMsgtype,ilMsglen);
    if(cgDebug>=2) swDebugmsghead((char *)&slMsgpack);
    if(cgDebug>=2) swDebughex((char *)slMsgpack.aMsgbody,ilMsglen-sizeof(struct msghead));
 
    if(ilMsglen == 0)
    {
      swVdebug(2,"S0150: �յ��ձ���");
      continue;
    }

    clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
    switch (slMsgpack.sMsghead.iMsgtype)
    {
      case iMSGREV:  /* ������Ӧ���� */
        llTran_id   = slMsgpack.sMsghead.lTranid;
        ilProc_step = slMsgpack.sMsghead.iTranstep;
	llSaf_id    = slMsgpack.sMsghead.lSafid;
        swVdebug(2,"S0160: ������Ӧ����,[tranid=%ld,step=%d,safid=%ld]",llTran_id,ilProc_step,llSaf_id);
        
        /* �жϴ�����ˮ�Ƿ����,����״̬Ϊ���ڳ��� */
        ilRc = swShmselect_swt_proc_log(llTran_id,ilProc_step,&sgSwt_proc_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0170: [����/�����ڴ�] ���Ҵ�����ˮʧ��,[tranid=%ld,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0180: ������ˮû���ҵ�,[tranid=%d,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);
          continue;
        }
        
        /* ADD BY GXZ 2003.02.23 Begin */
        slSwt_proc_log_view = sgSwt_proc_log;
        /* ADD BY GXZ 2003.02.23 End */
        
        if(sgSwt_proc_log.proc_status != iTRNREVING)
        {
          swVdebug(2,"S0190: ������ˮ״̬��Ϊ���ڳ���,[tranid=%d,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);
          continue;
        }   
        swVdebug(2,"S0200: �ҵ�������ˮ,״̬Ϊ���ڳ���,[tranid=%d,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);

        /* �������������ʽ */
        ilRc = swRevSuccess(slMsgpack);
        if(ilRc == FAIL)
        {
          swVdebug(2,"S0210: ����ʧ�ܴ���");
           
          /* ��SAF��¼״̬ΪSAFʧ��,����ʷ */
          /*modify by cjh 20150114*/
          ilRc = swShmselect_swt_rev_saf(llSaf_id,&sgSwt_rev_saf);
          /*modify by cjh 20150114 end*/
          if(ilRc == FAIL)
          {
            swVdebug(0,"S0220: [����/�����ڴ�] ����SAF��¼����,[safid=%ld]",llSaf_id);
            continue;
          }  

          if(ilRc == SHMNOTFOUND)
          {
            swVdebug(2,"S0230: SAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
            continue;
          }
          swVdebug(2,"S0240: �ҵ�SAF��¼,[safid=%ld]",llSaf_id);
          
          ilRc = swProcess_revfail_revsaf(llSaf_id);
          if(ilRc == FAIL) continue;

          /* �øñʽ��׵����д�����ˮ״̬Ϊ����ʧ��,����ʷ,ɾ��֮ */          
          ilRc = swProcess_revend_proclog(llTran_id,iTRNREVFAIL,clLoc_flag);
          if(ilRc == FAIL) continue;
          
          /* �ý�����ˮ״̬Ϊ����ʧ��,����ʷ,ɾ��֮ */
          ilRc = swProcess_revend_tranlog(llTran_id,iTRNREVFAIL,clLoc_flag,slSwt_proc_log_view,slMsgpack);
          /*begin add by gf at 2004-04-07*/
          if(sgSwt_rev_saf.saf_num > 0)
            continue;
          ilRc=swSendRevresult_union(llTran_id,slMsgpack,iMSGREVFAIL,clLoc_flag);
          /*end add*/

          continue;
        }
 
        /* ɾ��SAF��¼ */
        /*modify by cjh 20150114*/
        ilRc = swShmdelete_swt_rev_saf(llSaf_id);
        /*modify by cjh 20150114 end*/
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0250: [����/�����ڴ�] ɾ��SAF��¼����,[safid=%ld]",llSaf_id);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0260: SAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
          continue;
        }  
	swVdebug(2,"S0270: ɾ��SAF��¼�ɹ�,[safid=%ld]",llSaf_id);


        /* 2002.11.13 fzj add */
        slSwt_proc_log1 = sgSwt_proc_log; /* ���ڱ��Ĳ��� */
        /**/
        /* ���汨�� */
	sgSwt_proc_log.org_len = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
        ilRc = swPutlog((char *)&slMsgpack,sgSwt_proc_log.org_len,
          &(sgSwt_proc_log.org_offset),sgSwt_proc_log.org_file);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0280: [����/����] ���汨�ĳ���");
          continue;
        }
        swVdebug(2,"S0290: ���汨�ĳɹ�");
 
        sgSwt_proc_log.proc_status = iTRNREVEND;
        ilRc = swShmupdate_swt_proc_log(llTran_id,ilProc_step,sgSwt_proc_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0300: [����/�����ڴ�] ���´�����ˮʧ��,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0310: ������ˮû���ҵ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          continue;
        }  
        swVdebug(2,"S0320: �ô�����ˮ״̬Ϊ��������,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
        swVdebug(2,"S0330: ���´�����ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
        EXEC SQL BEGIN WORK;
#endif
        /* ����ʷ */
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
          swVdebug(0,"S0340: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          swDberror(NULL);
          EXEC SQL ROLLBACK WORK;
          continue;
        }
        swVdebug(2,"S0350: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#else
		sw_put_proclog(&sgSwt_proc_log);
#endif
        /* ɾ��֮ */
        ilRc = swShmdelete_swt_proc_log(llTran_id,ilProc_step,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0360: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
          EXEC SQL ROLLBACK WORK;
#endif
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0370: ������ˮû���ҵ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
          EXEC SQL ROLLBACK WORK;
#endif
          continue;
        }  
        swVdebug(2,"S0380: ɾ��������ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        EXEC SQL COMMIT WORK;
#endif

        swVdebug(2,"S0390: ������һ�ʴ�����ˮ");
 
        /* ����һ�ʴ�����ˮ */
        ilRc = swShmselect_max_proclog_tran(llTran_id,&sgSwt_proc_log);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0400: [����/�����ڴ�] ���Ҵ�����ˮ����,[tranid=%ld]",llTran_id);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {  
          swVdebug(2,"S0410: ������ˮû���ҵ�,������������");
           
          /* �øñʽ�����ˮ״̬Ϊ��������,����ʷ,ɾ��֮ */
          ilRc = swProcess_revend_tranlog(llTran_id,iTRNREVEND,clLoc_flag,slSwt_proc_log_view,slMsgpack);
          /*begin add by gf at 2004-04-07*/
	  swVdebug(2,"add by pc before swSendRevresult_union [%d] Ok",llTran_id);
      /* delete by zhanghui 20150111 
          ilRc=swSendRevresult_union(llTran_id,slMsgpack,iMSGREVEND,clLoc_flag);
          */
	  swVdebug(2,"add by pc swSendRevresult_union [%d] Ok",llTran_id);
          /*end add*/
          continue;
        }

	ilProc_step_next = sgSwt_proc_log.proc_step;
	swVdebug(2,"S0420: �ҵ���һ�ʴ�����ˮ,[tranid=%ld,step=%d]",llTran_id,ilProc_step_next);
	/*add by pc 2009-1-20 */
if (ilProc_step_next>=ilProc_step){
swVdebug(2,"S0488: ��һ������>=��ǰ����,������������");
 
/* �øñʽ�����ˮ״̬Ϊ��������,����ʷ,ɾ��֮ */
ilRc = swProcess_revend_tranlog(llTran_id,iTRNREVEND,clLoc_flag,slSwt_proc_log_view,slMsgpack);
ilRc=swSendRevresult_union(llTran_id,slMsgpack,iMSGREVEND,clLoc_flag);

/*end add*/
continue;	
}	
	/*add by pc 2009-1-20 */
	/* ȡ�ñ��� */
        memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
        _swTrim(sgSwt_proc_log.org_file);
        ilRc = swGetlog(sgSwt_proc_log.org_file,sgSwt_proc_log.org_len,
          sgSwt_proc_log.org_offset,(char *)&slMsgpack);
        if(ilRc == FAIL)  
        {
          swVdebug(0,"S0430: [����/����] ȡ������ˮ����ʧ��[%s]",sgSwt_proc_log.org_file);
          _swMsgsend(304011,NULL);
          continue;
        }
        slMsgpack.sMsghead.aSysattr[0] = clLoc_flag; /* add by nh 20021114 */
        swVdebug(2,"S0440: ȡ������ˮ���ĳɹ�[%s]",sgSwt_proc_log.org_file);
 
/* 2002.11.13 fzj add
 * ��������=����(��ǰ����ԭ����+��һ����ԭ����+��һ���������Ӧ����)
 */
        /* ��һ���������Ӧ���ļ�����ʱ��Ԥ��� */
        /*delete by gxz 2003.02.28
        memcpy((char *)pulPrebuf2,(char *)psgPreunpackbuf,sizeof(psgPreunpackbuf));
        */
        /* ��ǰ����ԭ����Ԥ��� */
        /*delete by gxz 2003.02.28
        ilRc = swFmlunpack(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,psgPreunpackbuf);
        if(ilRc)
        {
          swVdebug(0,"S0450: swFmlunpack() failed");
          continue;
        }
        */
        /* ȡ��һ����ԭ���� */ 
        /*delete by gxz 2003.02.28
        memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
        _swTrim(slSwt_proc_log1.org_file);
        ilRc = swGetlog(slSwt_proc_log1.org_file,slSwt_proc_log1.org_len,slSwt_proc_log1.org_offset,(char *)&slMsgpack);
        if(ilRc)
        {
          swVdebug(0,"S0460: swGetlog() failed");
          continue;
        }
        slMsgpack.sMsghead.aSysattr[0] = clLoc_flag;*/ /* add by nh 20021114 */
        
        /* ��һ����ԭ����Ԥ��� */
        /*delete by gxz 2003.02.28
        ilRc = swFmlunpack(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,pulPrebuf1);
        if(ilRc)
        {
          swVdebug(0,"S0470: swFmlunpack() failed");
          continue;
        }
        */
        /* ��ǰ����ԭ��������һ����ԭ���Ĳ��� */ 
        /*delete by gxz 2003.02.28
        ilRc = _swFmlunion(psgPreunpackbuf,pulPrebuf1);
        if(ilRc)
        {
          swVdebug(0,"S0480: _swFmlunion() failed");
          continue;
        }
        */
        /* ������һ���������Ӧ���Ĳ��� */
        /*delete by gxz 2003.02.28
        ilRc = _swFmlunion(psgPreunpackbuf,pulPrebuf2);
        if(ilRc)
        {
          swVdebug(0,"S0490: _swFmlunion() failed");
          continue;
        }
        swVdebug(2,"S0500: ���Ĳ����ɹ�");
        */
        /* ������ı��Ĵ�� */
        /*delete by gxz 2003.02.28
        ilRc = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,&ilMsglen);
        if(ilRc)
        {
          swVdebug(0,"S0510: swFmlpack() failed");
          continue;
        }
        slMsgpack.sMsghead.iBodylen = ilMsglen;
        ilMsglen = sizeof(struct msghead) + ilMsglen;
        swVdebug(2,"S0520: ��֯�������ĳɹ�");
        */
/**/

	/* ��ʱֻʣ��revmode��1��2�����,3��4������Ѿ���swMonlog��swMonrev
           �ļ�ʱ�������������˴��� */

        /* ���´�����ˮ״̬Ϊ���ڳ��� */
        sgSwt_proc_log.proc_status = iTRNREVING;
        ilRc = swShmupdate_swt_proc_log(llTran_id,ilProc_step_next,sgSwt_proc_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0530: [����/�����ڴ�] ���´�����ˮ����,[tranid=%ld,step=%d]",llTran_id,ilProc_step_next);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0540: ������ˮû���ҵ�,[tranid=%ld,step=%d,loc=%c]",llTran_id,ilProc_step_next,clLoc_flag);
          continue;
        }
        swVdebug(2,"S0550: �ô�����ˮ״̬Ϊ���ڳ���,���´�����ˮ�ɹ�,[tranid=%ld,step=%d,loc=[%c]]",llTran_id,ilProc_step_next,clLoc_flag);  
 
        /* ȡ��SAF��ˮ�� */
        memset((char *)&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
        if(swShmgetsafid(&llSaf_id) == FAIL)
        {
          swVdebug(0,"S0560: [����/�����ڴ�] ȡSAF��ˮ��ʧ��");
          continue;
        }
        swVdebug(2,"S0570: ȡ��SAF��ˮ��,[safid=%ld]",llSaf_id);
 
        /* ��֯����ͷ */
        slMsgpack.sMsghead.lTranid      = llTran_id;
        slMsgpack.sMsghead.iTranstep    = ilProc_step_next;
        slMsgpack.sMsghead.iMsgtypeorg  = slMsgpack.sMsghead.iMsgtype;
        slMsgpack.sMsghead.iMsgtype     = iMSGREV;
        slMsgpack.sMsghead.iDes_q       = sgSwt_proc_log.q_target;
        slMsgpack.sMsghead.lSafid       = llSaf_id;
        slMsgpack.sMsghead.cSafflag     = '1';
        slMsgpack.sMsghead.iRevfmtgroup = sgSwt_proc_log.rev_fmtgrp;
        slMsgpack.sMsghead.aSysattr[0] = clLoc_flag; /* add by nh 20021114 */

        /* ��¼SAF������ˮ */
        sgSwt_rev_saf.saf_id = llSaf_id;
        sgSwt_rev_saf.tran_id = llTran_id;
        sgSwt_rev_saf.proc_step = ilProc_step_next;
        sgSwt_rev_saf.saf_status[0] = '0';
        time(&sgSwt_rev_saf.saf_begin);
        sgSwt_rev_saf.rev_overtime=sgSwt_sys_queue.rev_overtime;
        sgSwt_rev_saf.saf_overtime = sgSwt_rev_saf.saf_begin + 
          sgSwt_sys_queue.rev_overtime;
        if(sgSwt_sys_queue.rev_num <= 0)
          sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;
        else
          sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num - 1;
        sgSwt_rev_saf.saf_status[0] = '0';
        sgSwt_rev_saf.saf_flag[0] = '2';  
        memset(sgSwt_rev_saf.saf_file,0x00,sizeof(sgSwt_rev_saf.saf_file));
        sgSwt_rev_saf.saf_offset=0;
        sgSwt_rev_saf.saf_len=sgSwt_proc_log.org_len;

        ilRc = swPutlog((char *)&slMsgpack,sgSwt_rev_saf.saf_len, 
          &sgSwt_rev_saf.saf_offset,sgSwt_rev_saf.saf_file);
        if( ilRc == FAIL)
        {
          swVdebug(0,"S0580: [����/����] ����SAF���ĳ���");
          continue;
        }
        swVdebug(2,"S0590: ����SAF���ĳɹ�");

       ilRc = swShmselect_swt_tran_log(llTran_id,&sgSwt_tran_log,clLoc_flag);
       if (ilRc == FAIL)
       {
         swVdebug(0,"S0600: ���ҽ�����ˮ����,[tranid=%ld]",llTran_id);
         continue;
       }
       if (ilRc == SHMNOTFOUND)
       {
         swVdebug(2,"S0610: û���ҵ�������ˮ,[tranid=%ld]",llTran_id);
         continue;
       }
       swVdebug(2,"S0620: ���ҽ�����ˮ�ɹ�,[tranid=%ld]",llTran_id);
       sgSwt_tran_log.q_target = sgSwt_proc_log.q_target;
       ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
       if (ilRc == FAIL)
       {
         swVdebug(0,"S0630: ���½�����ˮ����,[tranid=%ld]",llTran_id);
         continue;
       }
       if (ilRc == SHMNOTFOUND)
       {
         swVdebug(2,"S0640: Ҫ���µĽ�����ˮû���ҵ�,[tranid=%ld]",llTran_id);
         continue;
       }
       swVdebug(2,"S0650: ���½�����ˮĿ������Ϊ%d�ɹ�,[tranid=%ld]",sgSwt_tran_log.q_target,llTran_id);
 
        /* ׷��SAF��¼ */
       /*modify by cjh 20150114*/
        ilRc = swShminsert_swt_rev_saf(sgSwt_rev_saf);  
        /*modify by cjh 20150114 end*/
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0660: [����/�����ڴ�] ׷��SAF��¼����,[safid=%ld]",llSaf_id);
          continue;
        }  
        swVdebug(2,"S0670: ׷��SAF��¼�ɹ�,[safid=%ld]",llSaf_id);
        /* ���ͳ���������ǰ�ý��� */
        swVdebug(3,"S0680: ���ͳ������Ŀ�ʼ");
        if(cgDebug>=2)
        {
          swDebugmsghead((char *)&slMsgpack);
          swDebughex(slMsgpack.aMsgbody, slMsgpack.sMsghead.iBodylen);
        }
        swVdebug(3,"S0690: ���ͳ������Ľ���");
        ilRc = qwrite2((char *)&slMsgpack,sgSwt_proc_log.org_len,sgSwt_proc_log.q_target,ilPrior,ilClass,ilType);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0700: [����/��������] qwrite2 ����! ���ͳ���������ǰ�ý���ʧ��,[qid=%d]",sgSwt_proc_log.q_target);
          swMberror(ilRc,(char *)&slMsgpack);
          _swMsgsend(304001,(char *)&slMsgpack); 
        }
        else
        {
          swVdebug(2,"S0710: ���ͳ���������ǰ�ý��̳ɹ�,[qid=%d]",sgSwt_proc_log.q_target);
          _swMsgsend(199003,(char *)&slMsgpack);
        }  
        continue;

      case iMSGORDER:
        swVdebug(2,"S0720: �յ������[%d]",slMsgpack.sMsghead.lCode);
        switch(slMsgpack.sMsghead.lCode)
        {
          case 601:	/* ��ʱ���� */
            llTran_id = slMsgpack.sMsghead.lTranid;
            ilTran_step = slMsgpack.sMsghead.iTranstep;
            swVdebug(2,"S0730: ��ʼ��ʱ��������,[tranid=%ld,step=%d]...",llTran_id,ilTran_step);
            /* ���ҽ�����ˮ */
            ilRc = swShmselect_swt_tran_log(llTran_id,&sgSwt_tran_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S0740: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%ld]",llTran_id);
              continue;
            }
			
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
            if(ilRc == SHMNOTFOUND)
            {
              /* û���ҵ�,����ʷ��ȥ�� */
              swVdebug(2,"S0750: ������ˮû���ҵ�,����ʷ����ȥ��,[tranid=%ld]",llTran_id);
	          ilTranstat = iTRNREVEND ;
              EXEC SQL SELECT * INTO 
                :sgSwt_tran_log.tran_id:ilTmp,
                :sgSwt_tran_log.tran_begin:ilTmp,
                :sgSwt_tran_log.tran_status:ilTmp,
                :sgSwt_tran_log.tran_end:ilTmp,
                :sgSwt_tran_log.tran_overtime:ilTmp,
                :sgSwt_tran_log.q_tran_begin:ilTmp,
                :sgSwt_tran_log.tc_tran_begin:ilTmp,
                :sgSwt_tran_log.q_target:ilTmp,
                :sgSwt_tran_log.resq_file:ilTmp,
                :sgSwt_tran_log.resq_offset:ilTmp,
                :sgSwt_tran_log.resq_len:ilTmp, 
                :sgSwt_tran_log.resp_file:ilTmp,
                :sgSwt_tran_log.resp_offset:ilTmp,
                :sgSwt_tran_log.resp_len:ilTmp,
                :sgSwt_tran_log.rev_key:ilTmp
              FROM swt_his_tran_log
              WHERE tran_id = :llTran_id and tran_status <> :ilTranstat;
              if(sqlca.sqlcode == SQLNOTFOUND)
              {
                swVdebug(2,"S0760: ��ʷ�����޴˽�����ˮ,�����Ѿ�������,[tranid=%ld]",llTran_id);
                _swMsgsend(304002,(char *)&slMsgpack);
                continue;
              }
              else if(sqlca.sqlcode) 
              {
                swVdebug(0,"S0770: [����/���ݿ�] ������ʷ������ˮ����,[tranid=%ld]",llTran_id);
                swDberror((char *)&slMsgpack);
                continue;
              }
              else /* ����ʷ�����ҵ�������ˮ,�Ƶ���ǰ�� */
              {
                swVdebug(2,"S0780: ����ʷ�����ҵ��˽�����ˮ,����ʷ�Ƶ���ǰ");
                swVdebug(3,"S0790: ����״̬Ϊ[%d],���׷�������Ϊ[%d]",sgSwt_tran_log.tran_status,sgSwt_tran_log.q_tran_begin);
                ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
                if(ilRc == FAIL)
                {
                  swVdebug(0,"S0800: [����/�����ڴ�] ׷�ӽ�����ˮ����,[tranid=%ld]",llTran_id); 
                  continue;
                }  
#ifdef DB_INFORMIX
                EXEC SQL BEGIN WORK;
#endif
                EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTran_id;
                if(sqlca.sqlcode ) 
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(0,"S0810: [����/���ݿ�] ɾ����ʷ������ˮʧ��,[tranid=%ld]",llTran_id);
 	          swDberror((char *)&slMsgpack);
                  continue;
                }
                /* �ƴ�����ˮ */
                
                EXEC SQL SELECT MAX(proc_step) into :ilProc_step_max:ilTmp
                  FROM swt_his_proc_log WHERE tran_id = :llTran_id;
                if (sqlca.sqlcode)
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(0,"S0820: SELECT MAX(proc_step) FROM swt_his_proc_log ... error");
                  swDberror(NULL);
                  continue;
                }
                if (ilTmp < 0)
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(2,"S0830: û���ҵ���ʷ������ˮ,[tranid=%ld]",llTran_id);
                  continue;
                }  
                ilTranstat = iTRNREVEND;
                ilFlag = 0;
                for (ilProc_step = ilProc_step_max; ilProc_step > 0; ilProc_step--)
                {
                  memset((char *)&sgSwt_proc_log,0x00,sizeof(sgSwt_proc_log));
                  EXEC SQL SELECT * INTO
                    :sgSwt_proc_log.tran_id:ilTmp,
                    :sgSwt_proc_log.proc_step:ilTmp,
                    :sgSwt_proc_log.proc_begin:ilTmp, 
                    :sgSwt_proc_log.q_target:ilTmp, 
                    :sgSwt_proc_log.proc_status:ilTmp,
                    :sgSwt_proc_log.rev_mode:ilTmp,
                    :sgSwt_proc_log.rev_fmtgrp:ilTmp,
	            :sgSwt_proc_log.org_file:ilTmp,
	            :sgSwt_proc_log.org_offset:ilTmp,
	            :sgSwt_proc_log.org_len:ilTmp
	            FROM swt_his_proc_log
	            WHERE tran_id = :llTran_id AND proc_step = :ilProc_step AND proc_status <> :ilTranstat;
	          if (sqlca.sqlcode == SQLNOTFOUND) continue;
                  if (sqlca.sqlcode)
                  {
                    EXEC SQL ROLLBACK WORK;
                    swVdebug(0,"S0840: SELECT * FROM swt_his_proc_log WHERE ... error");
                    swDberror(NULL);
                    ilFlag = -1;
                    break;
                  }
                  ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,clLoc_flag);
                  if (ilRc == FAIL)
                  {
                    EXEC SQL ROLLBACK WORK;
                    swVdebug(0,"S0850: [����/�����ڴ�] ׷�Ӵ�����ˮ����,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                    ilFlag = -1;
                    break;
                  }
                  swVdebug(2,"S0860: ׷�Ӵ�����ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                  EXEC SQL DELETE FROM swt_his_proc_log WHERE tran_id = :llTran_id AND proc_step = :ilProc_step;
                  if (sqlca.sqlcode)
                  {
                    EXEC SQL ROLLBACK WORK;
                    swVdebug(0,"S0870: DELETE FROM swt_his_proc_log WHERE ... error");
                    swDberror(NULL);
                    ilFlag = -1;
                    break;
                  }
                  swVdebug(2,"S0880: ɾ����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                  ilFlag++;
                }
                if (ilFlag < 0) continue;
                EXEC SQL COMMIT WORK;
                swVdebug(2,"S0890: ������ˮ�ʹ�����ˮ����ʷ�Ƶ���ǰ���");
              }  
            }
#endif  /*support database,Database Deprivation Project 2014-12*/
		   
            /* �ý�����ˮ״̬Ϊ���ڳ��� */
            sgSwt_tran_log.tran_status = iTRNREVING;
            ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S0900: [����/�����ڴ�] ���½�����ˮʧ��,tranid=%ld",llTran_id);
              continue;
            }  
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S0910: ������ˮû���ҵ�,tranid=%ld",llTran_id);
              continue;
            }  
            swVdebug(2,"S0920: �ý�����ˮ״̬Ϊ���ڳ���,[tranid=%ld]",llTran_id);
            swVdebug(2,"S0930: ���½�����ˮ�ɹ�,[tranid=%ld]",llTran_id);
 
	    /* ɾ����ʱ���� */
	    ilRc = swShmselect_swt_proc_log(llTran_id,1,&sgSwt_proc_log,clLoc_flag);
	    if(ilRc == FAIL)
	    {
	      swVdebug(0,"S0940: [����/�����ڴ�] ���Ҵ�����ˮ����,[tranid=%ld,step=1]",llTran_id);
 	      continue;
	    }
	    if(ilRc == SUCCESS)
	    {
	      if(sgSwt_proc_log.rev_mode <= 4)
	      {
	        ilRc = swShmdelete_swt_proc_log(llTran_id,1,clLoc_flag);
	        if(ilRc == FAIL)
	        {
	          swVdebug(0,"S0950: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	        if(ilRc == SHMNOTFOUND)
	        {
	          swVdebug(2,"S0960: ������ˮû���ҵ�,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	      }
	      else
	      {
	        sgSwt_proc_log.rev_mode = 4;
	        ilRc = swShmupdate_swt_proc_log(llTran_id,1,sgSwt_proc_log,clLoc_flag);
	        if(ilRc == FAIL)
	        {
	          swVdebug(0,"S0970: [����/�����ڴ�] ���´�����ˮ����,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	        if(ilRc == SHMNOTFOUND)
	        {
	          swVdebug(2,"S0980: ������ˮû���ҵ�,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	      }   
            }
            swVdebug(2,"S0990: ɾ����Դ���˿ڵĳ�ʱ����,[tranid=%ld,step=1]",llTran_id);	     

            swVdebug(2,"S1000: ��ʼ����revmode=3��4�����...");
            
            /* ����REVMODE=3��4�Ĵ�����ˮ��¼ */
            ilRc = swShmselect_swt_proc_log_mrec(llTran_id,pslSwt_proc_log,&ilCount);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1010: [����/�����ڴ�] ����REVMODE=3��4�Ĵ�����ˮʧ��");
              continue;
            }
     
            /* ѭ��ȡ��������ˮ��¼������֪ͨ���� */
            j = 0;
            for(i = 0; i < ilCount; i ++)
            {
              /* ֻ�������ģʽΪ3��4�Ĵ�����ˮ */
              if(pslSwt_proc_log[i].rev_mode != 3 && pslSwt_proc_log[i].rev_mode != 4)
                continue;
                
              j++;
              ilProc_step = pslSwt_proc_log[i].proc_step;
              swVdebug(2,"S1020: �ҵ�REVMODE=3��4�Ĵ�����ˮ,[tranid=%ld,step=%d]",llTran_id,ilProc_step);       
              
              /* ȡ��ԭʼ���� */
              memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
              _swTrim(pslSwt_proc_log[i].org_file);
              ilRc = swGetlog(pslSwt_proc_log[i].org_file,pslSwt_proc_log[i].org_len,
                pslSwt_proc_log[i].org_offset,(char *)&slMsgpack);
              if(ilRc == FAIL) 
              {
                swVdebug(0,"S1030: [����/����] ȡ������ˮԭʼ����ʧ��");
                _swMsgsend(304011,NULL);
                continue;
              }
              swVdebug(2,"S1040: ȡ������ˮԭʼ���ĳɹ�");
 
              /* ��֯������֪ͨ���� */
              slMsgpack.sMsghead.iMsgtype = iMSGREV;
              slMsgpack.sMsghead.cSafflag = '0';
              slMsgpack.sMsghead.iRevfmtgroup = pslSwt_proc_log[i].rev_fmtgrp;
              slMsgpack.sMsghead.iDes_q = pslSwt_proc_log[i].q_target;
              slMsgpack.sMsghead.aSysattr[0]=clLoc_flag;/* add by nh 20021114 */
              if(cgDebug>=2)
              {
                swDebugmsghead((char *)&slMsgpack);
                swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }
              ilRc = qwrite2((char *)&slMsgpack,slMsgpack.sMsghead.iBodylen + sizeof(struct msghead),
                pslSwt_proc_log[i].q_target,ilPrior,ilClass,ilType);
              if(ilRc)
              {
	        swVdebug(0,"S1050: [����/����] qwrite2()����,������=%d, ����֪ͨ���ĵ�ǰ�ý���ʧ��,[qid=%d]",ilRc,pslSwt_proc_log[i].q_target); 
 	        _swMsgsend(304012,NULL);
                continue;
              }
              swVdebug(2,"S1060: ����֪ͨ���ĵ�ǰ�ý��̳ɹ�,[qid=%d]",pslSwt_proc_log[i].q_target); 
              _swMsgsend(199003,(char *)&slMsgpack);
 
              /* �øñʴ�����ˮ״̬Ϊ��������,����ʷ,ɾ��֮ */
              ilRc = swProcess_proclog(llTran_id,pslSwt_proc_log[i].proc_step,clLoc_flag);
              if(ilRc == FAIL) break;
            }
            if(ilRc == FAIL) continue;
            swVdebug(2,"S1070: ��ɶ�REVMODE=3��4�Ĵ���,��[%d]��",j);

            /* ����REVMODE=1��2���������ˮ */
            ilRc = swShmselect_max_proclog_tran(llTran_id,&sgSwt_proc_log);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1080: [����/�����ڴ�] ����REVMODE=1��2���������ˮ����,[tranid=%ld]",llTran_id);
              continue;
            }
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1090: û���ҵ�REVMODE=1��2�Ĵ�����ˮ,�����ɹ�,[tranid=%ld]",llTran_id);
              /* �ñʽ�����ˮ״̬�ó�������,����ʷ,ɾ��֮ */
              swProcess_revend_tranlog(llTran_id,iTRNREVEND,clLoc_flag,slSwt_proc_log_view,slMsgpack);
              /*begin add by gf at 2004-04-07*/
              ilRc=swSendRevresult_nounion(iMSGREVEND);
              /*end add*/
              continue;
            }
            /* ���д�����ˮ,���г��� */
            ilProc_step = sgSwt_proc_log.proc_step;
            swVdebug(2,"S1100: �ҵ�REVMODE=1��2���������ˮ,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
            /* ȡ��ԭʼ���� */
            memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
            _swTrim(sgSwt_proc_log.org_file);
            ilRc = swGetlog(sgSwt_proc_log.org_file,sgSwt_proc_log.org_len,
              sgSwt_proc_log.org_offset,(char *)&slMsgpack);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1110: [����/����] ȡ������ˮԭʼ����ʧ��");
              _swMsgsend(304011,NULL);
              continue;
            } 
            swVdebug(2,"S1120: ȡ������ˮԭʼ���ĳɹ�");
             
            /*add by gxz 2003.8.23 begin*/
                /* ȡ�ö˿����� */
            ilRc = swShmselect_swt_sys_queue(sgSwt_proc_log.q_target, &sgSwt_sys_queue);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1112: [����/�����ڴ�] ȡ�˿�����ʧ��,[qid=%d]",sgSwt_proc_log.q_target);
              continue;
            }  
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1114: �˿�û������,[qid=%d]",sgSwt_proc_log.q_target);
              continue;
            }  
            swVdebug(2,"S1116: ȡ�˿����óɹ�,[qid=%d]",sgSwt_proc_log.q_target);
           /*add by gxz 2003.8.23 end */
            /* ȡ��SAF��ˮ�� */
            memset((char *)&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
            ilRc = swShmgetsafid(&llSaf_id);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1130: [����/�����ڴ�] ȡSAF��ˮ�ų���");
              continue;
            }
            swVdebug(2,"S1140: ȡSAF��ˮ�ųɹ�,[safid=%ld]",llSaf_id);
 
            /* ��֯SAF����ͷ */
            slMsgpack.sMsghead.lTranid = llTran_id;
            slMsgpack.sMsghead.iTranstep = sgSwt_proc_log.proc_step;
            slMsgpack.sMsghead.iMsgtypeorg = slMsgpack.sMsghead.iMsgtype;
            slMsgpack.sMsghead.iMsgtype = iMSGREV;
            slMsgpack.sMsghead.iDes_q = sgSwt_proc_log.q_target;
            slMsgpack.sMsghead.lSafid = llSaf_id;
            slMsgpack.sMsghead.cSafflag = '1';
            slMsgpack.sMsghead.iRevfmtgroup = sgSwt_proc_log.rev_fmtgrp;
            slMsgpack.sMsghead.aSysattr[0]=clLoc_flag;/* add by nh 20021114 */

            /* ����SAF���� */
            sgSwt_rev_saf.saf_id = llSaf_id;
            sgSwt_rev_saf.tran_id = llTran_id;
            sgSwt_rev_saf.proc_step = sgSwt_proc_log.proc_step;
            sgSwt_rev_saf.saf_status[0] = '0';
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

            ilRc = swPutlog((char *)&slMsgpack,sgSwt_rev_saf.saf_len, 
              &sgSwt_rev_saf.saf_offset, sgSwt_rev_saf.saf_file);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1150: [����/����] ����SAF���ĳ���,[safid=%d]",llSaf_id);
              continue;
            }
            swVdebug(2,"S1160: ����SAF���ĳɹ�,[safid=%d]",llSaf_id);
            
            /* ׷��SAF��¼ */
            /*modify by cjh 20150114*/
            ilRc = swShminsert_swt_rev_saf(sgSwt_rev_saf);  /*unsolved*/
            /*modify by cjh 20150114 end*/
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1170: [����/�����ڴ�] ׷��SAF��¼����,[safid=%ld]",llSaf_id);
              continue;
            }  
            sgSwt_proc_log.proc_status = iTRNREVING;
            ilRc = swShmupdate_swt_proc_log(llTran_id,ilProc_step,sgSwt_proc_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1180: [����/�����ڴ�] ���´�����ˮ����,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
              continue;
            }
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1190: ������ˮû���ҵ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
              continue;
            }
            swVdebug(2,"S1200: �ô�����ˮ״̬Ϊ���ڳ���,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
            swVdebug(2,"S1210: ���´�����ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

            sgSwt_tran_log.q_target = sgSwt_proc_log.q_target;
            ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1220: [����/�����ڴ�] ���½�����ˮʧ��,[tranid=%ld]",llTran_id);
              continue;
            }  
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1230: ������ˮû���ҵ�,[tranid=%ld]",llTran_id);
              continue;
            }  
            swVdebug(2,"S1240: �ý�����ˮĿ������Ϊ%d,[tranid=%ld]",sgSwt_tran_log.q_target,llTran_id);
            swVdebug(2,"S1250: ���½�����ˮ�ɹ�,[tranid=%ld]",llTran_id);

            /* ���ͳ���������ǰ�ý��� */
            if(cgDebug>=2)
            {
              swDebugmsghead((char *)&slMsgpack);
              swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
            ilRc = qwrite2((char *)&slMsgpack,sgSwt_proc_log.org_len,sgSwt_proc_log.q_target,ilPrior,ilClass,ilType);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1260: [����/����] qwrite2 ����! ���ͳ���������ǰ�ý���ʧ��,[qid=%d]",sgSwt_proc_log.q_target);
              swMberror(ilRc,(char *)&slMsgpack);
              _swMsgsend(304001,(char *)&slMsgpack); 
            }
            else
            {
              swVdebug(2,"S1270: ���ͳ���������ǰ�ý��̳ɹ�,[qid=%d]",sgSwt_proc_log.q_target);
              _swMsgsend(199003,(char *)&slMsgpack);
            }  
            continue;
            break;

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/
          case 604:	/* RESAF */
            llSaf_id = slMsgpack.sMsghead.lSafid;
            swVdebug(2,"S1280: ��ʼRESAF����,[safid=%ld]...",llSaf_id);
            memset(&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
            /* modify by gf at 2004-04-26
            EXEC SQL SELECT * INTO :sgSwt_rev_saf FROM swt_rev_saf
              WHERE saf_id = :llSaf_id;
            */
            EXEC SQL SELECT * INTO 
              :sgSwt_rev_saf.saf_id:ilTmp,
              :sgSwt_rev_saf.tran_id:ilTmp,
              :sgSwt_rev_saf.proc_step:ilTmp,
              :sgSwt_rev_saf.saf_begin:ilTmp,
              :sgSwt_rev_saf.saf_overtime:ilTmp,
              :sgSwt_rev_saf.rev_overtime:ilTmp,
              :sgSwt_rev_saf.saf_num:ilTmp,
              :sgSwt_rev_saf.saf_status:ilTmp,
              :sgSwt_rev_saf.saf_flag:ilTmp,
              :sgSwt_rev_saf.saf_file:ilTmp,
              :sgSwt_rev_saf.saf_offset:ilTmp,
              :sgSwt_rev_saf.saf_len:ilTmp 
              FROM swt_rev_saf
              WHERE saf_id = :llSaf_id;
            if(sqlca.sqlcode == SQLNOTFOUND)
            {
              swVdebug(2,"S1290: ��ʷSAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
              continue;
            }
            if(sqlca.sqlcode)
            {
              swDberror(NULL);
              swVdebug(0,"S1300: [����/���ݿ�] ������ʷSAF��¼����,[safid=%ld]",llSaf_id);
              continue;
            }
            swVdebug(2,"S1310: �ҵ���ʷSAF��¼,[safid=%ld]",llSaf_id);

            /*ȡ��SAF����*/
            memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
            ilRc = swGetlog(sgSwt_rev_saf.saf_file,sgSwt_rev_saf.saf_len,
              sgSwt_rev_saf.saf_offset,(char *)&slMsgpack);
            if(ilRc)
            {
              swVdebug(0,"S1320: [����/����] ȡSAF����ʧ��");
              _swMsgsend(304010,NULL);
              continue;
            }  
            slMsgpack.sMsghead.aSysattr[0] = clLoc_flag;/* add by nh 20021114 */
            swVdebug(2,"S1330: ȡSAF���ĳɹ�",llSaf_id,sgSwt_rev_saf.saf_file);
             
            /*ȡ��Ŀ��˿�����*/
            ilRc = swShmselect_swt_sys_queue(slMsgpack.sMsghead.iDes_q,&sgSwt_sys_queue);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1340: [����/�����ڴ�] ȡĿ��˿�����ʧ��,[qid=%d]",slMsgpack.sMsghead.iDes_q);
              continue;
            }
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1350: Ŀ��˿�û������,[qid=%d]",slMsgpack.sMsghead.iDes_q);
              continue;
            }
            swVdebug(2,"S1360: ȡ��Ŀ��˿�����,[qid=%d]",slMsgpack.sMsghead.iDes_q);
             
            time(&llCurrenttime);
            
            /* RESAF���� */
            if(sgSwt_rev_saf.saf_flag[0] == '1') /*Ӧ��SAF*/
            {
              /*��SAF��¼����ʷ���ƻص�ǰ��*/
              sgSwt_rev_saf.saf_status[0] = '0';
              sgSwt_rev_saf.saf_overtime = llCurrenttime + sgSwt_sys_queue.rev_overtime;
              sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;
#ifdef DB_INFORMIX
              EXEC SQL BEGIN WORK;
#endif
              EXEC SQL update swt_rev_saf set saf_status='0',saf_overtime=:sgSwt_rev_saf.saf_overtime,saf_num=:sgSwt_rev_saf.saf_num where saf_id=:llSaf_id;
              if(sqlca.sqlcode)
              {
                swVdebug(0,"S1370: [����/���ݿ�] update��ʷSAF��¼����,[safid=%ld]",llSaf_id);
                EXEC SQL ROLLBACK WORK;                
                continue;
              }
              swVdebug(2,"S1380: update��ʷSAF��¼�ɹ�,[safid=%ld]",llSaf_id);  
              EXEC SQL COMMIT WORK;
            }
            else /*����SAF*/
            {
              /*��������ˮ����ʷ���ƻص�ǰ�� */
              llTran_id = sgSwt_rev_saf.tran_id;
              ilProc_step = sgSwt_rev_saf.proc_step;
              
              memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
              EXEC SQL SELECT * INTO
                :sgSwt_tran_log.tran_id:ilTmp,
                :sgSwt_tran_log.tran_begin:ilTmp,
                :sgSwt_tran_log.tran_status:ilTmp,
                :sgSwt_tran_log.tran_end:ilTmp,
                :sgSwt_tran_log.tran_overtime:ilTmp,
                :sgSwt_tran_log.q_tran_begin:ilTmp,
                :sgSwt_tran_log.tc_tran_begin:ilTmp,
                :sgSwt_tran_log.q_target:ilTmp,
                :sgSwt_tran_log.resq_file:ilTmp,
                :sgSwt_tran_log.resq_offset:ilTmp,
                :sgSwt_tran_log.resq_len:ilTmp,
                :sgSwt_tran_log.resp_file:ilTmp,
                :sgSwt_tran_log.resp_offset:ilTmp,
                :sgSwt_tran_log.resp_len:ilTmp,
                :sgSwt_tran_log.rev_key:ilTmp
              FROM swt_his_tran_log
              WHERE tran_id = :llTran_id;
              if(sqlca.sqlcode == SQLNOTFOUND)
              {
                swVdebug(2,"S1410: ��ʷ������ˮû���ҵ�,[tranid=%ld]",llTran_id);
                continue;
              }
              if(sqlca.sqlcode)
              {
                swVdebug(0,"S1420: [����/���ݿ�] ������ʷ������ˮ����,[tranid=%ld]",llTran_id);
                swDberror(NULL);
                continue;
              }
              swVdebug(2,"S1430: �ҵ���ʷ������ˮ,[tranid=%ld]",llTran_id);

              if(sgSwt_tran_log.tran_status != iTRNREVFAIL) continue;

              sgSwt_tran_log.tran_status = iTRNREVING;
              time(&sgSwt_tran_log.tran_begin);
              clLoc_flag='1';
              ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
              if(ilRc == FAIL)
              {
                swVdebug(0,"S1440: [����/�����ڴ�] ׷�ӽ�����ˮ����,[tranid=%ld]",llTran_id);
                continue;
              }
              swVdebug(2,"S1450: ׷�ӽ�����ˮ�ɹ�,[tranid=%ld]",llTran_id);
#ifdef DB_INFORMIX
              EXEC SQL BEGIN WORK;
#endif
              EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTran_id;
              if(sqlca.sqlcode)
              {
                EXEC SQL ROLLBACK WORK;
                swVdebug(0,"S1460: [����/���ݿ�] ɾ����ʷ������ˮʧ��,[tranid=%ld]",llTran_id);
                swDberror(NULL);
                continue;
              }
              swVdebug(2,"S1470: ɾ����ʷ������ˮ�ɹ�,[tranid=%ld]",llTran_id);
              /*��������ˮ�ƻص�ǰ��*/
              
              EXEC SQL SELECT MAX(proc_step) into :ilProc_step_max:ilTmp
                FROM swt_his_proc_log WHERE tran_id = :llTran_id;
              if (sqlca.sqlcode)
              {
                EXEC SQL ROLLBACK WORK;
                swVdebug(0,"S1480: SELECT MAX(proc_step) FROM swt_his_proc_log ... error");
                swDberror(NULL);
                continue;
              }
              if (ilTmp < 0)
              {
                EXEC SQL ROLLBACK WORK;
                swVdebug(2,"û���ҵ���ʷ������ˮ,[tranid=%ld]",llTran_id);
                continue;
              }  
              ilTranstat = iTRNREVFAIL;
              ilFlag = 0;
              for (ilProc_step = ilProc_step_max; ilProc_step > 0; ilProc_step--)
              {
                memset((char *)&sgSwt_proc_log,0x00,sizeof(sgSwt_proc_log));
                EXEC SQL SELECT * INTO
                  :sgSwt_proc_log.tran_id:ilTmp,
                  :sgSwt_proc_log.proc_step:ilTmp,
                  :sgSwt_proc_log.proc_begin:ilTmp, 
                  :sgSwt_proc_log.q_target:ilTmp, 
                  :sgSwt_proc_log.proc_status:ilTmp,
                  :sgSwt_proc_log.rev_mode:ilTmp,
                  :sgSwt_proc_log.rev_fmtgrp:ilTmp,
	          :sgSwt_proc_log.org_file:ilTmp,
	          :sgSwt_proc_log.org_offset:ilTmp,
	          :sgSwt_proc_log.org_len:ilTmp
	          FROM swt_his_proc_log
	          WHERE tran_id = :llTran_id AND proc_step = :ilProc_step
	          AND proc_status = :ilTranstat AND (rev_mode = 1 OR rev_mode = 2);
	        if (sqlca.sqlcode == SQLNOTFOUND) continue;
                if (sqlca.sqlcode)
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(0,"S1490: SELECT * FROM swt_his_proc_log WHERE ... error");
                  swDberror(NULL);
                  ilFlag = -1;
                  break;
                }

                if(ilFlag == 0)
                  sgSwt_proc_log.proc_status = iTRNREVING;

                ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,clLoc_flag);
                if (ilRc == FAIL)
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(0,"S1500: [����/�����ڴ�] ׷�Ӵ�����ˮ����,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                  ilFlag = -1;
                  break;
                }
                swVdebug(2,"S1510: ׷�Ӵ�����ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                EXEC SQL DELETE FROM swt_his_proc_log WHERE tran_id = :llTran_id AND proc_step = :ilProc_step;
                if (sqlca.sqlcode)
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(0,"S1520: DELETE FROM swt_his_proc_log WHERE ... error");
                  swDberror(NULL);
                  ilFlag = -1;
                  break;
                }
                swVdebug(2,"S1530: ɾ����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                ilFlag++;
              }
              if (ilFlag < 0) continue;
              swVdebug(2,"S1540: ������ˮ�ʹ�����ˮ����ʷ�Ƶ���ǰ���");


              /* ��SAF��¼����ʷ���ƻص�ǰ�� */
              sgSwt_rev_saf.saf_status[0] = '0';
              sgSwt_rev_saf.saf_overtime = llCurrenttime + sgSwt_sys_queue.rev_overtime;
              sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;

#ifdef DB_INFORMIX
              EXEC SQL BEGIN WORK;
#endif
              EXEC SQL update swt_rev_saf set saf_status='0',saf_overtime=:sgSwt_rev_saf.saf_overtime,saf_num=:sgSwt_rev_saf.saf_num where saf_id=:llSaf_id;
              if(sqlca.sqlcode)
              {
                swVdebug(0,"S1550: [����/���ݿ�] update��ʷSAF��¼����,[safid=%ld]",llSaf_id);
                EXEC SQL ROLLBACK WORK;                
                continue;
              }
              swVdebug(2,"S1560: update��ʷSAF��¼�ɹ�,[safid=%ld]",llSaf_id);  
              EXEC SQL COMMIT WORK;
            }    
            break;
#endif /*support database,Database Deprivation Project 2014-12*/                        
          default:
            /* �յ�δ֪����� */
            swVdebug(2,"S1570: δ֪�����");
            _swMsgsend(304007,(char *)&slMsgpack);
            break;
        }
        break;
      
      case iMSGAPP: 
        llTran_id = slMsgpack.sMsghead.lTranid;
        ilProc_step = slMsgpack.sMsghead.iTranstep;
	llSaf_id = slMsgpack.sMsghead.lSafid;

	/* �յ�SAF��Ӧ���� */
        if(slMsgpack.sMsghead.cSafflag == cTRUE)
        {
          swVdebug(2,"S1580: �յ�Ӧ��SAF��Ӧ����,[tranid=%ld,procstep=%d,safid=%ld]",llTran_id,ilProc_step,llSaf_id);

          /*modify by cjh 20150114*/
		  ilRc = swShmselect_swt_rev_saf(llSaf_id,&sgSwt_rev_saf);
          /*modify by cjh 20150114 end*/
          if(ilRc == FAIL)
          {
            swVdebug(0,"S1590: [����/�����ڴ�] ����SAF��¼ʧ��,[safid=%ld]",llSaf_id);
            continue;
          }
          if(ilRc == SHMNOTFOUND)
          {
            swVdebug(2,"S1600: SAF��¼û���ҵ�,[safid=%ld]",llSaf_id);
            continue;
          }
          if(sgSwt_rev_saf.saf_flag[0] != '1')
          {
            swVdebug(2,"S1610: SAF��¼״̬��Ϊ����SAF,[safid=%ld]",llSaf_id);
            continue;
          }
          swVdebug(2,"S1620: �ҵ�SAF��¼,����״̬Ϊ����SAF,[safid=%ld]",llSaf_id);
		  /*modify by cjh 20150114*/
          ilRc = swShmdelete_swt_rev_saf(llSaf_id);
          /*modify by cjh 20150114 end*/
          if(ilRc == FAIL)
            swVdebug(0,"S1630: [����/�����ڴ�] ɾ��SAF��¼ʧ��,[safid=%d]",llSaf_id);
          if(ilRc == SHMNOTFOUND)
            swVdebug(2,"S1640: SAF��¼û���ҵ�,[safid=%d]",llSaf_id);
          swVdebug(2,"S1650: ɾ��SAF��¼�ɹ�,[safid=%d]",llSaf_id);
          continue;
        }
        else
        {
          /* �յ�δ֪���� */
          swVdebug(2,"S1660: �յ�δ֪����");
          _swMsgsend(304007,(char *)&slMsgpack);
          break;
        }

      default:
        /* �յ�δ֪���� */
        swVdebug(2,"S1670: �յ�δ֪����");
        _swMsgsend(304007,(char *)&slMsgpack);
        break;
    }
  }
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
  
  swVdebug(2,"S1680: [swMonrev]�Ѿ�ֹͣ...");
  swDbclose();
  ilRc=qdetach();
  if(ilRc)
  {
    swVdebug(0,"S1690: [����/����] qdetach()����,������=%d",ilRc);
    exit(FAIL);
  }
  exit(sig);
}

/**************************************************************
 ** ������      �� swRevSuccess
 ** ��  ��      �� ���������ĳɹ����
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/11/29
 ** ����޸����ڣ� 1999/11/29
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� msgpack �����ױ���
 ** ����ֵ      �� SUCCESS, FAIL
***************************************************************/
int swRevSuccess(struct msgpack msgpack)
{
  char  alResult[iFLDVALUELEN + 1];  /* ���ʽ������ */
  short ilLength;                    /* ���ʽ���������� */

#ifdef DB_SUPPORT
  EXEC SQL BEGIN DECLARE SECTION;
    char  alResu_revexp[101];
    short ilQid;
  EXEC SQL END DECLARE SECTION;
/*ad by zcd 20141224*/
#else
  char  alResu_revexp[101];
  short ilQid;
#endif
/*end of add by zcd 20141224*/
  swVdebug(4,"S1700: [��������] swRevSuccess()");

  ilQid = msgpack.sMsghead.iOrg_q;
  ilRc = swShmselect_swt_sys_queue(ilQid,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1710: [����/�����ڴ�] swShmselect_swt_sys_queue():FAIL");
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1720: �˿�û������,[qid=%d]",ilQid);
    return(FAIL);
  }  
  
  ilRc = swFmlunpack(msgpack.aMsgbody,msgpack.sMsghead.iBodylen,psgPreunpackbuf);
  if( ilRc == FAIL )
  {
    swVdebug(0,"S1730: [����/��������] swFmlunpack()����,������=%d",ilRc);
    _swMsgsend( 302010, NULL ); 
    return(FAIL);
  }
  
  memcpy(alResu_revexp,sgSwt_sys_queue.resu_revexp,sizeof(alResu_revexp));
  if(alResu_revexp[0] == '\0')
  {
    swVdebug(2,"S1740: ������Ӧ���ķ��ؽ����ȷ");
    swVdebug(4,"S1750: [��������] swRevSuccess()������=0");
    return(SUCCESS);
  }
  if(_swExpress(alResu_revexp, alResult,&ilLength)==FAIL)
  {
    swVdebug(0,"S1760: [����/��������] _swExpress()����,������=-1");
    _swMsgsend(777777,NULL);
    return(FAIL);
  }

  if(strcmp(alResult,"1"))
  {
    swVdebug(2,"S1770: ������Ӧ���ķ��ؽ������");
    return(FAIL);
  }
  else
  {
    swVdebug(2,"S1780: ������Ӧ���ķ��ؽ���ɹ�");
    swVdebug(4,"S1790: [��������] swRevSuccess()������=0");
    return(SUCCESS);
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
int swProcess_proclog(long lTran_id,short iProc_step,char aLoc_flag)
{
  swVdebug(4,"S1800: [��������] swProcess_proclog(%ld,%d)",lTran_id,iProc_step);
  /* �ô�����ˮ״̬Ϊ�������� */
  pslSwt_proc_log[i].proc_status = 4;
  ilRc = swShmupdate_swt_proc_log(lTran_id,iProc_step,pslSwt_proc_log[i],aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1810: [����/�����ڴ�] ���´�����ˮʧ��,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1820: ������ˮû���ҵ�,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    return (FAIL);
  }
  swVdebug(2,"S1830: �ô�����ˮ״̬,���´�����ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,iProc_step);

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/

#ifdef DB_INFORMIX
  EXEC SQL BEGIN WORK;
#endif
  /* ������ˮ����ʷ */
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
    swVdebug(0,"S1840: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S1850: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#else
  sw_put_proclog(&pslSwt_proc_log[i]);
#endif
 
  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_proc_log(lTran_id,iProc_step,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1860: [����/�����ڴ�] ɾ��������ˮʧ��,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1870: ������ˮû���ҵ�,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1880: ɾ��������ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif

  swVdebug(4,"S1890: [��������] swProcess_proclog()������=0");
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
int swProcess_revend_tranlog(long lTran_id,short iStatus,char aLoc_flag,struct swt_proc_log sSwt_proc_log,struct msgpack sMsgpack)
{
  /* ADD BY GXZ 2003.02.23 */
  struct msgpack sMsgpack1;
  short  iMsglen;
  union  preunpackbuf puPrebuf[iBUFFLDNUM];
  union  preunpackbuf puPrebuf1[iBUFFLDNUM];
  /**/
    
  swVdebug(4,"S1900: [��������] swProcess_revend_tranlog(%ld,%d)",lTran_id,iStatus);

  /* ADD BY GXZ 2003.02.23  Begin*/
  /* ȡ�ñ��� */
  memset((char *)&sMsgpack1,0x00,sizeof(sMsgpack1));
  _swTrim(sSwt_proc_log.org_file);
  ilRc = swGetlog(sSwt_proc_log.org_file,sSwt_proc_log.org_len,sSwt_proc_log.org_offset,(char *)&sMsgpack1);
  if(ilRc == FAIL)  
  {
    swVdebug(0,"S0430: [����/����] ȡ������ˮ����ʧ��[%s]",sSwt_proc_log.org_file);
  }
  else
    swVdebug(2,"S0440: ȡ������ˮ���ĳɹ�[%s]",sSwt_proc_log.org_file);
 
   /* ��ǰ����ԭ����Ԥ��� */
  ilRc = swFmlunpack(sMsgpack1.aMsgbody,sMsgpack1.sMsghead.iBodylen,puPrebuf);
  if(ilRc)
  {
    swVdebug(0,"S0450: swFmlunpack() failed");
  }

  /* ��Ӧ����Ԥ��� */
  ilRc = swFmlunpack(sMsgpack.aMsgbody,sMsgpack.sMsghead.iBodylen,puPrebuf1);
  if(ilRc)
  {
    swVdebug(0,"S0470: swFmlunpack() failed");
  }

  /* ���Ĳ��� */ 
  ilRc = _swFmlunion(puPrebuf,puPrebuf1);
  if(ilRc)
  {
    swVdebug(0,"S0480: _swFmlunion() failed");
  }
  else
    swVdebug(2,"S0500: ���Ĳ����ɹ�");

  /* ������ı��Ĵ�� */
  ilRc = swFmlpack(puPrebuf,sMsgpack.aMsgbody,&iMsglen);
  if(ilRc)
  {
    swVdebug(0,"S0510: swFmlpack() failed");
  }
  sMsgpack.sMsghead.iBodylen = iMsglen;
  iMsglen = sizeof(struct msghead) + iMsglen;
  swVdebug(2,"S0520: ��֯�������ĳɹ�");
        
  /* ADD BY GXZ 2003.02.23 End */
  
  /* ���½�����ˮ״̬ */
  ilRc = swShmupdate_setstat_tranlog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1910: [����/�����ڴ�] ���½�����ˮ����,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1920: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1930: �ý�����ˮ״̬,���½�����ˮ�ɹ�,[tranid=%ld]",lTran_id);
  
  /* ������ˮ�Ƶ���ʷ */
  ilRc = swShmselect_swt_tran_log(lTran_id,&sgSwt_tran_log,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1940: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1950: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
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
    swVdebug(0,"S1960: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld]",lTran_id);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
/*add by zcd 20141225*/
#else
  sw_put_tranlog(&sgSwt_tran_log);
#endif
  swVdebug(2,"S1970: ׷����ʷ������ˮ�ɹ�,[tranid=%ld]",lTran_id);
/*end of add by zcd 20141225*/

  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_tran_log(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1980: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1990: ������ˮû���ҵ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S2000: ɾ��������ˮ�ɹ�,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
  sMsgpack.sMsghead.iOrg_q = 999;
  _swMsgsend(99999,(char *)&sMsgpack);    
  swVdebug(4,"S2010: [��������] swProcess_revend_tranlog()������=0");
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
  swVdebug(4,"S2020: [��������] swProcess_revend_proclog(%ld,%d)",lTran_id,iStatus);

  /* ���´�����ˮ״̬ */
  ilRc = swShmupdate_setstat_proclog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2030: [����/�����ڴ�] ���´�����ˮ����,[tranid=%ld,step=*]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2040: ������ˮû���ҵ�,[tranid=%ld,step=*]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S2050: �ô�����ˮ״̬,���½�����ˮ�ɹ�,[tranid=%ld,step=*]",lTran_id);
  
  /* ������ˮ�Ƶ���ʷ */
  ilRc = swShmselect_swt_proc_log_mrec(lTran_id,pslSwt_proc_log,&ilCount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2060: [����/�����ڴ�] ���Ҵ�����ˮ����,[tranid=%ld,step=*]",lTran_id);
    return (FAIL);
  }  
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2070: ������ˮû���ҵ�,[tranid=%ld,step=*]",lTran_id);
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
      swVdebug(0,"S2080: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return (FAIL);
    }
/*ad by zcd 20141225*/
#else
    sw_put_proclog(&pslSwt_proc_log[i]);
#endif /*support database,Database Deprivation Project 2014-12*/ 
  /*end of add by zcd 20141225*/
     swVdebug(2,"S2090: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
  }  
 
  /* ɾ��������ˮ */
  ilRc = swShmdelete_swt_proc_log_mrec(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2100: [����/�����ڴ�] ɾ��������ˮʧ��,[tranid=%ld,step=*]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/ 
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2110: ������ˮû���ҵ�,[tranid=%ld,step=*]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/ 
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S2120: ɾ��������ˮ�ɹ�,[tranid=%ld,step=*]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S2130: [��������] swProcess_revend_proclog()������=0");
  return(SUCCESS);
}


/**************************************************************
 ** ������      �� swProcess_revfail_revsaf
 ** ��  ��      �� ��ĳ��SAF��ˮ״̬ΪSAFʧ��,
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
  swVdebug(4,"S2140: [��������] swProcess_revfail_revsaf(%ld)",lSaf_id);
  /* ����SAF��¼״̬ */
  sgSwt_rev_saf.saf_status[0] = '2';	/* SAFʧ�� */
  /*modify by cjh 20150114*/
  ilRc = swShmupdate_swt_rev_saf(lSaf_id,sgSwt_rev_saf);
  /*modify by cjh 20150114 end */
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2150: [����/�����ڴ�] ����SAF��¼����,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2160: SAF��¼û���ҵ�,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
  swVdebug(2,"S2170: ��SAF��¼״̬ΪSAFʧ��,[safid=%ld]",lSaf_id);
  swVdebug(2,"S2180: ����SAF��¼�ɹ�,[safid=%ld]",lSaf_id);
 
  swVdebug(4,"S2190: [��������] swProcess_revfail_revsaf()������=0");
  return(SUCCESS);
}

/*begin add by gf at 2004-04-07*/
/**************************************************************
 ** ������      �� swSendRevresult_union
 ** ��  ��      �� ���ͳ��������Դ���˿�(��Ҫ���Ĳ���)
 **             :  
 ** ��  ��      ��  
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/11/19
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� 0-�ɹ� / 1-ʧ��
***************************************************************/
int swSendRevresult_union(long lTranid,struct msgpack sMsgpack,short iMsgtype,char cLoc_flag)
{
  short ilRc;
  unsigned int ilMsglen; /*changed by dyw, PSBC_V1.0*/
  struct msgpack slMsgpack1;
  union  preunpackbuf pulPrebuf1[iBUFFLDNUM];
  union   preunpackbuf pulPreunpackbuf[iBUFFLDNUM];
  
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
  struct swt_tran_log_tmp
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
  }slTranlog;
    /*struct swt_tran_log slTranlog;*/
    sqlint64 llTran_id1;
    short ilTmp1;
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
  struct swt_tran_log_tmp
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
  }slTranlog;
    /*struct swt_tran_log slTranlog;*/
    long llTran_id1;
    short ilTmp1;
  EXEC SQL END DECLARE SECTION;
#endif
/*add by zcd 20141225*/
#else

  struct swt_tran_log_tmp
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
  }slTranlog;
    /*struct swt_tran_log slTranlog;*/
    long llTran_id1;
    short ilTmp1;
#endif
/*end of add by zcd 20141225*/
  llTran_id1 = lTranid;
  /*
swVdebug(4,"add by pc ...........11[%d]",llTran_id1); 
exec sql select count(*) into :ilTmp1 from swt_his_tran_log WHERE tran_id = :llTran_id1;

swVdebug(4,"add by pc ...........1111[%d] count[%d] sqlcode[%d] [%s]",llTran_id1,ilTmp1,sqlca.sqlcode,sqlca.sqlerrm.sqlerrmc);    
	*/


#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/

  memset(&slTranlog,0x00,sizeof(struct swt_tran_log));
  EXEC SQL SELECT * INTO
    :slTranlog.tran_id:ilTmp1,
    :slTranlog.tran_begin:ilTmp1,
    :slTranlog.tran_status:ilTmp1,
    :slTranlog.tran_end:ilTmp1,
    :slTranlog.tran_overtime:ilTmp1,
    :slTranlog.q_tran_begin:ilTmp1,
    :slTranlog.tc_tran_begin:ilTmp1,
    :slTranlog.q_target:ilTmp1,
    :slTranlog.resq_file:ilTmp1,
    :slTranlog.resq_offset:ilTmp1,
    :slTranlog.resq_len:ilTmp1,
    :slTranlog.resp_file:ilTmp1,
    :slTranlog.resp_offset:ilTmp1,
    :slTranlog.resp_len:ilTmp1,
    :slTranlog.rev_key:ilTmp1
    FROM swt_his_tran_log WHERE tran_id = :llTran_id1;
/*swVdebug(4,"add by pc ...........22");    	*/
  if(sqlca.sqlcode == SQLNOTFOUND)
  {
    swVdebug(0,"S0430: ��ʷ�����޴˽�����ˮ,[tranid=%ld]",llTran_id1);
    return(FAIL);
  }
  else if(sqlca.sqlcode)
  {
    swVdebug(0,"S0440: [����/���ݿ�] ������ʷ������ˮ����,[tranid=%ld]",llTran_id1);
    return(FAIL);
  }
#endif

  swVdebug(2,"S0450: �ҵ���ʷ������ˮ,[tranid=%ld]",llTran_id1);
  memset((char *)&slMsgpack1,0x00,sizeof(slMsgpack1));
  _swTrim(slTranlog.resp_file);
  ilRc = swGetlog(slTranlog.resp_file,slTranlog.resp_len,
  slTranlog.resp_offset,(char *)&slMsgpack1);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0630: [����/����] ȡ������ˮ��Ӧ����ʧ��");
    return(FAIL);
  }
  swVdebug(2,"S0630: ȡԭʼ���ĳɹ�");
   
  /*���Ĳ���*/
  swFmlunpack(sMsgpack.aMsgbody,sMsgpack.sMsghead.iBodylen,pulPrebuf1);
  swFmlunpack(slMsgpack1.aMsgbody,slMsgpack1.sMsghead.iBodylen,pulPreunpackbuf);
      
  /* ��ԭʼ�����뵱ǰ���Ľ��в������� */
  _swFmlunion(pulPreunpackbuf,pulPrebuf1);
  swFmlpack(pulPreunpackbuf,sMsgpack.aMsgbody,&ilMsglen);
  sMsgpack.sMsghead.iBodylen = ilMsglen;
  ilMsglen = sizeof(struct msghead) + ilMsglen;
  swVdebug(2,"S0640: ���Ĳ����ɹ�");  
   
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


 /**************************************************************
 ** ������      �� swSendRevresult_nounion
 ** ��  ��      �� ���ͳ��������Դ���˿�(����Ҫ���Ĳ���)
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
