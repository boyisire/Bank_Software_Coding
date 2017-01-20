/****************************************************************/
/* ģ����    ��CTRL                                           */
/* ģ������    ��ϵͳ����ģ��                                   */
/* �� �� ��    ��V4.3.0                                         */
/* ��    ��    ��ʷ����                                         */
/* ��������    ��1999/11/15                                     */
/* ����޸����ڣ�2001/4/6                                       */
/* ģ����;    ������ƽ̨���ļ�ɢ����                           */
/* ��ģ���а������º���������˵����                             */
/*                       ��1��int main();                       */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼:                                                    */
/*  2000.2.19 �����Ĵ洢���ļ��ĳ����ݿ��е�BLOB���ֶ�(ʷ����)  */
/*  2000.2.19 Դ�����䳬ʱ(������֪ͨ)��֧��(ʷ����)            */
/*  2000.2.25 �����Ĵ洢�����ݿ�ĳ��ļ����(ʷ����)            */
/*  2000.3.07 ����ƽ̨�༶������֧��                            */
/*  2001/2/27 ����DEBUG����                                     */
/*  2001/3/24 ���V3.01,����1772��                            */
/*  2001/4/3  DB2��ֲ(1797)                                     */
/*  2001/4/6  INFORMIX��ֲ(1774)                                */
/*  2001/6/5  ���ݿ������Ϊ�����ڴ����                        */
/****************************************************************/

/* ͷ�ļ����� */
#include        "switch.h"

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

#include <assert.h>

extern int swFormat(struct msgpack *psMsgpack);
extern int swShmupdate_swt_tran_log_end(long lTran_id, struct swt_tran_log sSwt_tran_log,char cLoc_flag);

struct swt_proc_log pslSwt_proc_log[iMAXPROCNUM];

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
EXEC SQL BEGIN DECLARE SECTION;
  short igTmp;
EXEC SQL END DECLARE SECTION;  
#endif

/* �������� */
#define DEBUG		1
#define UNIONFLAG	1
#define MOVELOGFLAG	1

/* �������� */
short ilRc;

/* ����ԭ�Ͷ��� */
void swQuit(int);



/*===begin== static variable, move from main, by dyw, 2014-1-9 ===*/
  static  struct msgpack slMsgpack;
  static  struct msgpack *pslBlob;
  static  union  preunpackbuf pulPrebuf1[iBUFFLDNUM];
  static  struct timeb fb;

  /*modify by zcd 20141226
  static  short i,ilMsglen;*/
  static  unsigned int i,ilMsglen;
  static  short ilOrgqid,ilPriority,ilClass,ilType,ilResult;
  static  short ilResultlen;
  static  char  alResult[iFLDVALUELEN + 1];
  static  char  alPackbuffer[iMSGMAXLEN];
  static  short ilPacksaveflag;
  static  char  clRevFlag;

  static  short ilTran_status;
  static  long  llPackoffset;
  /*modify by zcd 20141226
  static  short ilPacklen;*/
  static  unsigned int ilPacklen;
  static  char  alPackfile[11];

  static  short ilQdes, ilQorg, ilQid;
  static  char  alTrancode[11];
  static  long  llTran_begin, llTran_overtime;
  static  short ilCount;
 
  static  char  alRevkey[iEXPRESSLEN];	/* add by fanzhijie 2002.01.04 */
  static  char  clLoc_flag;             /* ���Ĵ��λ�ñ�־ add by xujun 2002.09.12 */

/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
   static sqlint64  llTranid;
   static  short ilTranstep;
  EXEC SQL END DECLARE SECTION; 
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
  static  long  llTranid;
  static  short ilTranstep;
  EXEC SQL END DECLARE SECTION; 
#endif
#else
static long  llTranid;
static short ilTranstep;
#endif

/*===End== static variable, move from main, by dyw, 2014-1-9 ===*/


/**************************************************************
 ** ������: main()
 ** ��  ��: ��������ģ��������
 ** ��  ��: ʷ���ǡ�������
 ** ��������: 1999.11.15
 ** ����޸�����: 2000.2.19
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:
***************************************************************/
int main(int argc,char *argv[])
{
	#if 0  /* move to head as static variables, by dyw*/
  struct msgpack slMsgpack;
  struct msgpack *pslBlob;
  union  preunpackbuf pulPrebuf1[iBUFFLDNUM];
  struct timeb fb;

  /*modify by zcd 20141226
  short i,ilMsglen;*/
  unsigned int i,ilMsglen;
  short ilOrgqid,ilPriority,ilClass,ilType,ilResult,ilResultlen;
  char  alResult[iFLDVALUELEN + 1];
  char  alPackbuffer[iMSGMAXLEN];
  short ilPacksaveflag;
  char  clRevFlag;

  short ilTran_status;
  long  llPackoffset;
  /*modify by zcd 20141226
  short ilPacklen;*/
  unsigned int ilPacklen;
  char  alPackfile[11];

  short ilQdes, ilQorg, ilQid;
  char  alTrancode[11];
  long  llTran_begin, llTran_overtime;
  short ilCount;
  
  char  alRevkey[iEXPRESSLEN];	/* add by fanzhijie 2002.01.04 */
  char  clLoc_flag;             /* ���Ĵ��λ�ñ�־ add by xujun 2002.09.12 */

/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  llTranid;
    short ilTranstep;
  EXEC SQL END DECLARE SECTION; 
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llTranid;
    short ilTranstep;
  EXEC SQL END DECLARE SECTION; 
#endif
#else
long  llTranid;
short ilTranstep;
#endif
#endif

  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swCtrl Version 4.3.0",argv[1]);

  /* LICENSE */ 
  if (_swLicense("swCtrl")) exit(FAIL);
 
  /* �����ź� */
  signal(SIGTERM,swQuit);
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);

  /* ���õ��Գ������� */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  /* strcpy(agDebugfile,"swCtrl.debug"); */
   /*  sprintf(agDebugfile,"swCtrl_%d.debug",getpid());*/

  swLogInit("swCtrl", NULL); /*add by dyw, 2015.03.27, PSBC_V1.0*/

  /* ��ȡDEBUG��־ */
  if((cgDebug = _swDebugflag("swCtrl")) == FAIL)
  {
    fprintf(stderr,"��ȡDEBUG��־����!");
    exit(FAIL);
  }
  swVdebug(2,"S0010: �汾��[4.3.0]");
  swVdebug(2,"S0020: DEBUG�ļ�[%s]",agDebugfile);
  swVdebug(2,"S0030: DEBUG��־[%d]",cgDebug);

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/
  /* �����ݿ� */
  if(swDbopen() != SUCCESS)
  {
    swVdebug(0,"S0040: [����/���ݿ�] swDbopen()�����ݿ����");
    exit(FAIL);
  }
  swVdebug(2,"S0050: �����ݿ�ɹ�");
#endif

  /* ������ */
  ilRc = qattach(iMBCTRL);
  if(ilRc)
  {
    swVdebug(0,"S0060: [����/����] qattach()����,������=%d",ilRc);
    exit(FAIL);
  }
  swVdebug(2,"S0070: ������ɹ�");
   
  /* ��ʼ�������ڴ�ָ�� */
  if(swShmcheck() != SUCCESS)
  {
    swVdebug(0,"S0080: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    exit(FAIL);
  }
  swVdebug(2,"S0090: ��ʼ�������ڴ�ָ��ɹ�");
  
  /* ��ϵͳ��ط�����Ϣ���� */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: ����104001��Ϣ����,����ģ���Ѿ�����...");
 
  /* ѭ�������� */
  for(;;)
  {
    /* ���������� */
    ilMsglen = iMSGMAXLEN;
    ilPriority = ilClass = ilType = 0;
    
    /* swVdebug(0,"S0101:  qread2()����,"); */

    ilRc = qread2((char *)&slMsgpack,&ilMsglen,
      &ilOrgqid,&ilPriority,&ilClass,&ilType);
    /* swVdebug(0,"S0102:  qread2()����,"); */

    if(ilRc != SUCCESS)
    {
      swVdebug(0,"S0110: [����/����] qread2()����,������=%d",ilRc);
      swMberror(ilRc,(char *)&slMsgpack);
      swQuit(FAIL);
    }
    /*==begin==������־������־����, cjh, 2015.03.23, PSBC_V1.0*/
    cgDebug=slMsgpack.sMsghead.log_level;


    memset(agDebugfile,0x00,sizeof(agDebugfile));
    memcpy(agDebugfile,slMsgpack.sMsghead.file_log_name,sizeof(slMsgpack.sMsghead.file_log_name));
    
    /*==end==������־������־����, PSBC_V1.0 */
    /* ˢ�¹����ڴ�ָ�� */
	
    if(swShmcheck() != SUCCESS)
    {
      swVdebug(0,"S0120: [����/�����ڴ�] �����ڴ�ָ��ˢ��ʧ��");
      swQuit(FAIL);
    }


    ilPacksaveflag = 0; /* �����ѱ�����LOG�ļ��ı�ʶ */
    clRevFlag = 0; /* �����Ƿ�Ҫ������ʶ */

    ftime(&fb);
    if(cgDebug>=2)
    {
      swVdebug(3,"S0130: [%d����]�յ�����:����=[%d],���ȼ�=[%d],class=[%d],type=[%d]",fb.millitm,ilOrgqid,ilPriority,ilClass,ilType);
      swDebugmsghead((char *)&slMsgpack);
      swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      if(slMsgpack.sMsghead.iMsgformat == iFMTIMF)
        swDebugfml((char *)&slMsgpack);
    }  
    swVDebugfml(2, (char*)&slMsgpack);

    /* ȡ�ñ���ͷ��Ϣ */
    llTranid = slMsgpack.sMsghead.lTranid;
    ilQdes = slMsgpack.sMsghead.iDes_q;
    ilQorg = slMsgpack.sMsghead.iOrg_q;
    memset(alTrancode,0x00,sizeof(alTrancode));
    memcpy(alTrancode,slMsgpack.sMsghead.aTrancode,
      sizeof(slMsgpack.sMsghead.aTrancode));
      
    /* �յ�Ӧ�ñ��� */
    if(slMsgpack.sMsghead.iMsgtype == iMSGAPP)
    {
      swVdebug(2,"S0140: ��������ΪӦ�ñ���");

     // printf("S0140: ��������ΪӦ�ñ���\n");

      /* ����ͷ.SAF��ʶΪ�� */
      if(slMsgpack.sMsghead.cSafflag == cTRUE)
      {
        swVdebug(2,"S0150: ����ͷ.SAF��ʶΪ��");

        printf("S0150: ����ͷ.SAF��ʶΪ��\n");

        /* ������ת�����������ģ�鴦�� */
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,iMBMONREV,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(0,"S0160: [����/����] qwrite2(),д��������������,������=%d",ilRc);

          //printf("S0160: [����/����] qwrite2(),д��������������,������=%d",ilRc);

          swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S0170: д�����������ɹ�");
        continue;
      }

      /* ���ҽ�����ˮ */
#ifdef DB_SUPPORT  /*support database 2014-12*/
      ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'a');
#else
	  ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'0'); /*only shm supported*/
#endif
      if(ilRc == FAIL)
      {
        swVdebug(1,"S0180: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%ld]",llTranid);
        continue;
      }
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0190: ������ˮû���ҵ�,[tranid=%ld]",llTranid);
        if(slMsgpack.sMsghead.iTranstep == 1) 
        {/*
#ifdef TRANREVLOG        
          EXEC SQL SELECT * FROM swt_tranrev_log WHERE tran_id = :llTranid;
          if(!sqlca.sqlcode)
          {
            swVdebug(1,"S0192: �������ڷ�����֮�󣬲�����tran_id=%ld",llTranid);
            continue;
          }
          else if(!(sqlca.sqlcode == SQLNOTFOUND))
          {
            swVdebug(0,"S0194: select from swt_tranrev_log failed, sqlcode=%d",sqlca.sqlcode);
            continue;
          }
#endif     */   
          /* ���ײ���Ϊ1ʱ(�½���),��д������ˮ���� */
          swVdebug(2,"S0200: ���ײ���Ϊ1(�½���),׷�ӽ�����ˮ");
          memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
          sgSwt_tran_log.tran_id = llTranid;
          sgSwt_tran_log.tran_status = iTRNING;
          time(&llTran_begin);
          sgSwt_tran_log.tran_begin = llTran_begin;
          sgSwt_tran_log.q_tran_begin = slMsgpack.sMsghead.iOrg_q;
          sgSwt_tran_log.tran_overtime = 0;
          memcpy(sgSwt_tran_log.tc_tran_begin,slMsgpack.sMsghead.aTrancode,
            sizeof(slMsgpack.sMsghead.aTrancode));
          clLoc_flag='0';
          ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
	  swVdebug(3,"tbl_test sgSwt_tran_log.tran_id=%ld",llTranid);
		  
#ifdef ONLY_SHM_SUPPORTED    /*only shm supported*/
		  /*if only shm supported and ilRc == SHMFULL, the TRNX will set to be failed. 2014-12*/
		  if(ilRc == SHMFULL)
		  {
		  	swVdebug(1,"S0205: [����/�����ڴ�]�����ڴ��������޷������µĽ���,[tranid=%ld]",llTranid);
			continue;
		  }				
#endif

          if(clLoc_flag == '1') slMsgpack.sMsghead.aSysattr[0] = '1'; 
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0210: [����/�����ڴ�] ׷�ӽ�����ˮ����,[tranid=%ld]",llTranid);
            continue;
          }  
          swVdebug(2,"S0220: ׷�ӽ�����ˮ�ɹ�,[tranid=%ld]",llTranid);
        }
        else
        {
          /* ���ײ���>1���ҳ���ģʽΪ������Ӧ���� */
          swVdebug(2,"S0230: ���ײ��費Ϊ1,�ٵ���Ӧ,[tranid=%ld,step=%d]",
            llTranid,slMsgpack.sMsghead.iTranstep);
          if((slMsgpack.sMsghead.iRevmode != 2) && (slMsgpack.sMsghead.iRevmode != 6))
            continue;
		  
#ifdef DB_SUPPORT /*support database 2014-12*/
          swVdebug(2,"S0240: ���ҳ���ģʽΪ������Ӧ����,���Ǵ�������ˮ");
          /* ������ʷ������ˮ */
          memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
          EXEC SQL SELECT * INTO
            :sgSwt_tran_log.tran_id:igTmp,
            :sgSwt_tran_log.tran_begin:igTmp,
            :sgSwt_tran_log.tran_status:igTmp,
            :sgSwt_tran_log.tran_end:igTmp,
            :sgSwt_tran_log.tran_overtime:igTmp,
            :sgSwt_tran_log.q_tran_begin:igTmp,
            :sgSwt_tran_log.tc_tran_begin:igTmp,
            :sgSwt_tran_log.q_target:igTmp,
            :sgSwt_tran_log.resq_file:igTmp,
            :sgSwt_tran_log.resq_offset:igTmp,
            :sgSwt_tran_log.resq_len:igTmp,
            :sgSwt_tran_log.resp_file:igTmp,
            :sgSwt_tran_log.resp_offset:igTmp,
            :sgSwt_tran_log.resp_len:igTmp,
            :sgSwt_tran_log.rev_key:igTmp
          FROM swt_his_tran_log WHERE tran_id = :llTranid;
          if(sqlca.sqlcode)
          {
            if(sqlca.sqlcode == SQLNOTFOUND)
            {
              swVdebug(2,"S0250: û���ҵ���ʷ������ˮ,[tranid=%ld]",llTranid);
              continue;
            }  
            else
            {
              swVdebug(2,"S0260: ������ʷ������ˮ����,[tranid=%ld]",llTranid);
              swDberror(NULL);
              continue;
            }    
          }
          swVdebug(2,"S0270: �ҵ���ʷ������ˮ,[tranid=%ld]",llTranid);
          if(sgSwt_tran_log.tran_status != iTRNREVEND)
          {
            swVdebug(2,"S0280: ����״̬��Ϊ��������,[tranid=%ld]",llTranid);
            continue;
          } 
          /* ������ʷ������ˮ */
          memset(&sgSwt_proc_log,0x00,sizeof(struct swt_proc_log));
          ilTranstep = slMsgpack.sMsghead.iTranstep;
          
          /*nh 20060503
          EXEC SQL SELECT * INTO :sgSwt_proc_log FROM swt_his_proc_log
            WHERE tran_id = :llTranid and proc_step = :ilTranstep;*/
            
          EXEC SQL SELECT * INTO :sgSwt_proc_log.tran_id:igTmp,
                         :sgSwt_proc_log.proc_step:igTmp,
                         :sgSwt_proc_log.proc_begin:igTmp,
                         :sgSwt_proc_log.q_target:igTmp,
                         :sgSwt_proc_log.proc_status:igTmp,
                         :sgSwt_proc_log.rev_mode:igTmp,
                         :sgSwt_proc_log.rev_fmtgrp:igTmp,
                         :sgSwt_proc_log.org_file:igTmp,
                         :sgSwt_proc_log.org_offset:igTmp,
                         :sgSwt_proc_log.org_len:igTmp
          FROM swt_his_proc_log
          WHERE tran_id = :llTranid and proc_step = :ilTranstep;
          if(!sqlca.sqlcode)
          {
            swVdebug(2,"S0290: �ҵ���ʷ������ˮ,���貹��,[tranid=%ld,step=%d]",llTranid,ilTranstep);
            continue;
          }
          if (sqlca.sqlcode != SQLNOTFOUND)
          {
            swVdebug(2,"S0300: ������ʷ������ˮ����");
            swDberror(NULL);
            continue;
          }
          /* ��ʷ������ˮ���ڣ�����ʷ������ˮ������ */
          /* ����ʷ������ˮ�Ƶ���ǰ������ˮ */
#ifdef DB_INFORMIX
          EXEC SQL BEGIN WORK;
#endif          
          EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTranid;
          if(sqlca.sqlcode)
          {
            swVdebug(1,"S0310: ɾ����ʷ������ˮ����,[tranid=%ld]",llTranid);
            swDberror(NULL);
            EXEC SQL ROLLBACK WORK;
            continue;
          }
          swVdebug(2,"S0320: ɾ����ʷ������ˮ�ɹ�,[tranid=%ld]",llTranid);
          clLoc_flag='1';
          ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
          if(clLoc_flag == '1') slMsgpack.sMsghead.aSysattr[0] = '1';
          if (ilRc != SUCCESS)
          {
            swVdebug(1,"S0330: [����/�����ڴ�] ׷�ӽ�����ˮ����,[tranid=%ld]",llTranid);
            EXEC SQL ROLLBACK WORK;
            continue;
          }  
          swVdebug(2,"S0340: ׷�ӽ�����ˮ�ɹ�,tranid=%ld",llTranid);
          EXEC SQL COMMIT WORK;

          clRevFlag = 1;
#endif
        }
      }
      else if(sgSwt_tran_log.tran_status == iTRNREVING)
      { 
        if((slMsgpack.sMsghead.iRevmode == 2) || (slMsgpack.sMsghead.iRevmode == 6))
        {
          swVdebug(2,"S0350: ���ҳ���ģʽΪ������Ӧ����");
          ilTranstep = slMsgpack.sMsghead.iTranstep;
#ifdef DB_SUPPORT  /*support database 2014-12*/
          ilRc = swShmselect_swt_proc_log(llTranid,ilTranstep,&sgSwt_proc_log,'a');
#else
		  ilRc = swShmselect_swt_proc_log(llTranid,ilTranstep,&sgSwt_proc_log,'0');
#endif
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0360: [����/�����ڴ�] ���Ҵ�����ˮ����,tranid=%ld,step=%d",
              llTranid,ilTranstep);
            continue;
          }
          else if(!ilRc)
          {
            swVdebug(2,"S0370: �ҵ�������ˮ,tranid=%ld,step=%d,������",llTranid,ilTranstep);
            continue;
          }
        }
        swVdebug(2,"S0371: �ý�����ˮΪ���ڳ���״̬,�ó���flag,tranid=[%ld]",llTranid);
        clRevFlag = 1;
      }
      else if (sgSwt_tran_log.tran_status == iTRNREVFAIL)
      {
        swVdebug(2,"S0380: �ҵ�������ˮ,״̬Ϊ����ʧ��,������,tranid=%ld",
          llTranid);
        continue;
      }
      else if (sgSwt_tran_log.tran_status == iTRNREVEND)
      {
        swVdebug(2,"S0390: �ҵ�������ˮ,״̬Ϊ��������,tranid=%ld",llTranid);
        if((slMsgpack.sMsghead.iRevmode != 2) && (slMsgpack.sMsghead.iRevmode != 6))
          continue;

        swVdebug(2,"S0400: ���ҳ���ģʽΪ������Ӧ����");
        clRevFlag = 1;
      }
      else if (sgSwt_tran_log.tran_status == iTRNEND)
      {
        swVdebug(2,"S0410: �ҵ�������ˮ,״̬Ϊ���׽���,������,tranid=%ld",
          llTranid);
        continue;
      }
      else
      {
        swVdebug(2,"S0420: �ҵ�������ˮ,״̬Ϊ���ڽ���,tranid=%ld",llTranid);
         ilTran_status = sgSwt_tran_log.tran_status;
      }
      
      /* �鿴��ʽת����ʽ */
      if(slMsgpack.sMsghead.cFormatter == cCONVYES)
      {
        swVdebug(2,"S0430: �Զ����и�ʽת��");
        slMsgpack.sMsghead.iMsgtype = iMSGUNPACK;
        if(swFormat(&slMsgpack) != SUCCESS)
        {
          swVdebug(1,"S0440: ���ø�ʽת����������");
          continue;
        }  
        swVdebug(2,"S0450: ���ø�ʽת�������ɹ�");
      }
      else
      {
        swVdebug(2,"S0460: �������Զ���ʽת��");
        slMsgpack.sMsghead.iMsgtype = iMSGUNPACK;
      }
    }



    /* ������� */
    if(slMsgpack.sMsghead.iMsgtype == iMSGUNPACK)
    {
      /* ��¼��ʼ������ */
      if(slMsgpack.sMsghead.iTranstep == 1)
      {
        memcpy(slMsgpack.sMsghead.aBegin_trancode,slMsgpack.sMsghead.aTrancode,
          sizeof(slMsgpack.sMsghead.aTrancode));
      }
      /* ��鱨������ */
      if(slMsgpack.sMsghead.iMsgformat != iFMTIMF)
      {
        swVdebug(2,"S0470: ���ĸ�ʽ��Ϊƽ̨�ڲ�����FML��ʽ");
        continue;
      }
      
#ifdef UNIONFLAG
      /* ȡ����Ӧ���Ĵ���ļ���ƫ���������� */
      if (slMsgpack.sMsghead.iTranstep == 1)
        ilPacklen = 0;
      else
      {
        /* ���ҽ�����ˮ */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef DB_SUPPORT
        ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,clLoc_flag);
#else
		ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'0');
#endif
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0480: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0490: [����/�����ڴ�] ������ˮû���ҵ�,[tranid=%ld]",llTranid);
          continue;
        }
        swVdebug(2,"S0500: �ҵ�������ˮ,[tranid=%ld]",llTranid);
        memcpy(alPackfile,sgSwt_tran_log.resp_file,sizeof(alPackfile));
        llPackoffset = sgSwt_tran_log.resp_offset;
        ilPacklen = sgSwt_tran_log.resp_len;
      }

      if(ilPacklen)
      {
        ilRc = swGetlog(alPackfile,ilPacklen,llPackoffset,alPackbuffer);
        if (ilRc == FAIL)  
        {
          swVdebug(1,"S0510: [����/��������] swGetlog()����,ȡ���ĳ���");
          continue;
        }
        swVdebug(2,"S0520: ȡ���ĳɹ�");  
        pslBlob = (struct msgpack *)alPackbuffer;
        swFmlunpack(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,pulPrebuf1);
swVdebug(2,"------------------------1------------------------");
        swFmlunpack(pslBlob->aMsgbody,pslBlob->sMsghead.iBodylen,psgPreunpackbuf);

swVdebug(2,"------------------------2------------------------");
            
        /* �������ı����뵱ǰ���Ľ��в������� */
        _swFmlunion(psgPreunpackbuf,pulPrebuf1);
swVdebug(2,"------------------------3------------------------");
        swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,&ilMsglen);
        slMsgpack.sMsghead.iBodylen = ilMsglen;
        ilMsglen = sizeof(struct msghead) + ilMsglen;
        swVdebug(2,"S0530: ���Ĳ����ɹ�");
      }

      /* ���汨�� */
      if(ilPacksaveflag == 0)
      {
        ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
        ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,alPackfile);
        if(ilRc == FAIL)        
        {
          swVdebug(1,"S0540: [����/��������] swPutlog()����,���汨�ĳ���");
          continue;
        }
        swVdebug(2,"S0550: ���汨�ĳɹ�");
        ilPacksaveflag = 1;
      }
#endif

      if(slMsgpack.sMsghead.iTranstep == 1)
      {
        if(ilPacksaveflag == 0)
        {
          ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
          ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,alPackfile);
          if(ilRc == FAIL)        
          {
            swVdebug(1,"S0560: [����/��������] swPutlog()����,���汨�ĳ���");
            continue;
          }
          swVdebug(2,"S0570: ���汨�ĳɹ�");
          ilPacksaveflag = 1;
        }
        
        /* ���½�����ˮ(����->��ֵ->����) */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef DB_SUPPORT
        ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,clLoc_flag);
#else
		ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'0');
#endif
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0580: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0590: ������ˮû���ҵ�,[tranid=%ld]",llTranid);
          continue;
        }
        swVdebug(2,"S0600: �ҵ�������ˮ,[tranid=%ld]",llTranid);
        memcpy(sgSwt_tran_log.resq_file,alPackfile,sizeof(alPackfile));
        sgSwt_tran_log.resq_offset = llPackoffset;
        sgSwt_tran_log.resq_len = ilPacklen;
        memcpy(sgSwt_tran_log.resp_file,alPackfile,sizeof(alPackfile));
        sgSwt_tran_log.resp_offset = llPackoffset;
        sgSwt_tran_log.resp_len = ilPacklen;

/* add by fanzhijie 2002.01.04 */
        /* Դ�����䳬ʱ���� */     
        ilQid = slMsgpack.sMsghead.iBegin_q_id;
        memset(alTrancode,0x00,sizeof(alTrancode));
        memcpy(alTrancode,slMsgpack.sMsghead.aBegin_trancode,
          sizeof(slMsgpack.sMsghead.aTrancode));

        ilRc = swShmselect_route_d_q_tran(ilQid,alTrancode,&sgSwt_sys_route_d);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0610: [����/�����ڴ�] ����·�ɽű��γ���,[qid=%d,trancode=%s]",ilQid,alTrancode);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0620: �޷���λ·�ɽű���,[qid=%d,trancode=%s]",ilQid,alTrancode);
          continue;
        }  
        slMsgpack.sMsghead.iRevmode = sgSwt_sys_route_d.rev_mode;
        slMsgpack.sMsghead.iRevfmtgroup = sgSwt_sys_route_d.rev_fmtgrp;
        swVdebug(2,"S0630: Դ�����䳬ʱ����ɹ�,[qid=%d,trancode=%s]",ilQid,alTrancode);

        /* �������� */
        alRevkey[0] = '\0';
        if(sgSwt_sys_route_d.route_cond[0] != '\0')
          strcpy(alRevkey,sgSwt_sys_route_d.route_cond);
        else
        {
          ilRc = swShmselect_swt_sys_queue(ilQorg,&sgSwt_sys_queue);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0640: [����/�����ڴ�] ���˿����ó���,[qid=%d]",ilQorg);
            continue;
          }
          if(ilRc == SHMNOTFOUND)
          {
            swVdebug(1,"S0650: [����/�����ڴ�] �˿�û������,[qid=%d]",ilQorg);
            continue;
          }
          swVdebug(2,"S0660: ���˿����óɹ�,[qid=%d]",ilQorg);
          strcpy(alRevkey,sgSwt_sys_queue.rev_express);
        }
        
        if(alRevkey[0] != '\0')
        {
          ilRc = swFmlunpack(slMsgpack.aMsgbody,
            slMsgpack.sMsghead.iBodylen,psgPreunpackbuf);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0670: [����/��������] swFmlunpack()����,FML���Ľ������,������=%d",ilRc);
            _swMsgsend(399001,NULL);
            continue;
          }

          /* �����ֵ */
          ilRc = _swExpress(alRevkey,alResult,&ilResultlen);
          if(ilRc == FAIL) 
          {
            swVdebug(1,"S0680: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alRevkey,ilRc);
            _swMsgsend(303006,NULL );
            continue;
          }
          else
            strcpy(sgSwt_tran_log.rev_key,alResult);
        }
/* add by fanzhijie 2002.01.04 */

        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
        ilRc = swShmupdate_swt_tran_log(llTranid,sgSwt_tran_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0690: [����/�����ڴ�] ���½�����ˮ����,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0700: [����/�����ڴ�] ������ˮû���ҵ�,[tranid=%ld]",llTranid);
          continue;
        }  
        swVdebug(2,"S0710: ���½�����ˮ�ɹ�,[tranid=%ld]",llTranid);
      }
      else
      {
#ifdef UNIONFLAG
        memcpy(sgSwt_tran_log.resp_file,alPackfile,sizeof(alPackfile));
        sgSwt_tran_log.resp_offset = llPackoffset;
        sgSwt_tran_log.resp_len = ilPacklen;
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
        ilRc = swShmupdate_swt_tran_log(llTranid,sgSwt_tran_log,clLoc_flag);
        if(ilRc)
        {
          swVdebug(1,"S0720: [����/�����ڴ�] ���½�����ˮ����,[tranid=%ld]",llTranid);
          continue;
        }
#endif          
      }
      ilResult = TRUE;
      
      /* �����г���ģʽ */
      if(slMsgpack.sMsghead.iRevmode)
      {
        swVdebug(2,"S0730: �����г���ģʽ");
         
        ilRc = swShmselect_swt_sys_queue(ilQorg,&sgSwt_sys_queue);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0740: [����/�����ڴ�] ���˿����ó���,[qid=%d]",ilQorg);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0750: [����/�����ڴ�] �˿�û������,[qid=%d]",ilQorg);
          continue;
        }
        swVdebug(2,"S0760: ���˿����óɹ�,[qid=%d]",ilQorg);
         
        /* ������ʽ��Ϊ�� */
        if(strlen(sgSwt_sys_queue.resu_express)>0 && slMsgpack.sMsghead.iTranstep>1)
        {
          /* ���ڲ����� */
          ilResult = FALSE;
          ilRc = swFmlunpack(slMsgpack.aMsgbody,
            slMsgpack.sMsghead.iBodylen,psgPreunpackbuf);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0770: [����/��������] swFmlunpack()����,FML���Ľ������,������=%d",ilRc);
            _swMsgsend(399001,NULL);
            continue;
          }

          /* ���������ʽ */
          ilRc = _swExpress(sgSwt_sys_queue.resu_express,alResult,&ilResultlen);
          if(ilRc == FAIL) 
          {
            swVdebug(1,"S0780: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",sgSwt_sys_queue.resu_express,ilRc);
            _swMsgsend(303006,NULL );
            continue;
          }
          else
            if(alResult[0] != '0') ilResult = TRUE;
        }
        else
          ilResult = TRUE;
      }

      /* ����ģʽ = 1��3�������������֪ͨ */
      if((ilResult != TRUE) && ((slMsgpack.sMsghead.iRevmode == 1)
        ||(slMsgpack.sMsghead.iRevmode == 3)))
      {
        swVdebug(2,"S0790: ���ر��Ĳ��ɹ�,�ҳ���ģʽΪ1��3,ɾ���Ѽ�¼�Ĵ�����ˮ");
        ilTranstep = slMsgpack.sMsghead.iTranstep;
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
        ilRc = swShmdelete_swt_proc_log(llTranid,ilTranstep,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0800: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld,step=%d]",llTranid,ilTranstep);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0810: ������ˮû���ҵ�,[tranid=%ld,step=%d]",llTranid,ilTranstep);
          continue;
        }  
        swVdebug(2,"S0820: ɾ��������ˮ�ɹ�,[tranid=%ld,step=%d]",llTranid,ilTranstep);
      }

      /* ����ģʽ = 2��4,������Ӧ������֪ͨ */
      if((ilResult == TRUE) && ((slMsgpack.sMsghead.iRevmode == 2) || 
        (slMsgpack.sMsghead.iRevmode == 4)))
      {
        swVdebug(2,"S0830: ���ر��ĳɹ�,�ҳ���ģʽΪ2��4,��д������ˮ");
        /* ��д������ˮ���� */
        memset((char *)&sgSwt_proc_log,0x00,sizeof(struct swt_proc_log));
        sgSwt_proc_log.tran_id = llTranid;
        sgSwt_proc_log.proc_step = slMsgpack.sMsghead.iTranstep;
        sgSwt_proc_log.proc_status = iTRNING;
        sgSwt_proc_log.q_target = ilQorg;
        sgSwt_proc_log.rev_mode = slMsgpack.sMsghead.iRevmode;
        sgSwt_proc_log.rev_fmtgrp = slMsgpack.sMsghead.iRevfmtgroup;

        /* ��ȡ�˿����� */
        ilRc = swShmselect_swt_sys_queue(sgSwt_proc_log.q_target,
          &sgSwt_sys_queue);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0840: [����/�����ڴ�] ��ȡ�˿����ó���,[qid=%d]",sgSwt_proc_log.q_target);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0850: �˿�û������,[qid=%d]",sgSwt_proc_log.q_target);
          continue;
        }
        swVdebug(2,"S0860: ��ȡ�˿����óɹ�,[qid=%d]",sgSwt_proc_log.q_target);       

        /* Ŀ�����䲻֧�ֶ�γ���,ɾ���˱ʽ��ס�������Ĵ�����ˮ�����¼ */
        if(sgSwt_sys_queue.rev_nomrev[0] == cTRUE)
        {
          swVdebug(2,"S0870: Ŀ�����䲻֧�ֶ�γ���,ɾ���Ѽ�¼�Ķ�ʴ�����ˮ");
          clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
          ilRc = swShmdelete_proclog_tran_q(sgSwt_proc_log.tran_id,
            sgSwt_proc_log.q_target,clLoc_flag);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0880: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld,qid=%d,step=*]",
              sgSwt_proc_log.tran_id, sgSwt_proc_log.q_target);
            continue;
          }
          swVdebug(2,"S0890: ɾ��������ˮ�ɹ�,[tranid=%ld,qid=%d,step=*]",
            sgSwt_proc_log.tran_id, sgSwt_proc_log.q_target);          
        }

        if(ilResult)
        {
          /* ���汨�� */
          if(ilPacksaveflag == 0)
          {
            ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
            ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,
              alPackfile);

            if(ilRc == FAIL)  
            {
              swVdebug(1,"S0900: [����/��������] swPutlog()����,���汨�ĳ���");
              continue;
            }
            swVdebug(2,"S0910: ���汨�ĳɹ�");
            ilPacksaveflag = 1;
          }
          memcpy(sgSwt_proc_log.org_file,alPackfile,sizeof(alPackfile));
          sgSwt_proc_log.org_offset = llPackoffset;
          sgSwt_proc_log.org_len = ilPacklen;
	  
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
          assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
          /* ׷�Ӵ�����ˮ */          
          /* ������ˮ�ڹ����ڴ��� */
          if(slMsgpack.sMsghead.aSysattr[0] == '0')
          {
            ilRc=0;
            /* ���ڴ������� < �˿�ת�ƴ����������� */
            /*MODI BY GXZ 2003.1.7*/
            if( sgSwt_sys_queue.transhm_max <= 0 || sgSwt_sys_queue.traning_num <= sgSwt_sys_queue.transhm_max )
            {
              ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'0');
			  
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
              if(ilRc == SHMFULL) 	              
	      {
                swVdebug(1,"S0919: [����/�����ڴ�] �����ڴ�����[tranid=%ld,step=%d]",
                sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
                continue;
              }
#endif
              if(ilRc == FAIL)
              {
                swVdebug(1,"S0920: [����/�����ڴ�] ׷�Ӵ�����ˮ����,[tranid=%ld,step=%d]",
                  sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
                continue;
              }
              /*ADD BY GXZ 2003.1.7 Begin*/
#ifdef DB_SUPPORT  /*support database 2014-12*/
#endif
              if(ilRc == SHMFULL)
              {
                ilRc = swShm2db_swt_tran_proc_log(llTranid);
                if(ilRc == FAIL)
                {
                  swVdebug(1,"S0931: [����/�����ڴ�] ת����ˮ�����ݿ����,[tranid=%ld]",llTranid);
                  continue;
                }
                slMsgpack.sMsghead.aSysattr[0] = '1';
                swVdebug(2,"S0941: ������ˮ�ʹ�����ˮ�ɹ�ת�Ƶ����ݿ� [tranid=%ld]",llTranid);
              }
              /*ADD BY GXZ 2003.1.7 End*/
            }
            /*������ˮ�����ڴ��������㽻��ת������,���н��׺ʹ�����ˮ�����ݿ�*/
            /*Delete By GXZ 2003.1.7
            if((ilRc==SHMFULL) || (sgSwt_sys_queue.traning_num > sgSwt_sys_queue.transhm_max))*/
            /*ADD BY GXZ 2003.1.7 Begin*/
#ifdef DB_SUPPORT  /*support database 2014-12*/
#endif
            else
            /*ADD BY GXZ 2003.1.7 End*/
            {
              ilRc = swShm2db_swt_tran_proc_log(llTranid);   /*need review again*/
              if(ilRc == FAIL)
              {
                swVdebug(1,"S0930: [����/�����ڴ�] ת����ˮ�����ݿ����,[tranid=%ld]",llTranid);
                continue;
              }
              slMsgpack.sMsghead.aSysattr[0] = '1';
              swVdebug(2,"S0940: ������ˮ�ʹ�����ˮ�ɹ�ת�Ƶ����ݿ� [tranid=%ld]",llTranid);
            }
          }
          /* ������ˮ�����ݿ���,��������ת��(��ʱ���ʴ�����ˮ��û�м�¼)ʱ��
             ������ˮ�������ݿ��� */
             
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
          assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
          if(slMsgpack.sMsghead.aSysattr[0] == '1')
          {
            ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'1');
            if(ilRc == FAIL)
            {
              swVdebug(1,"S0950: [����/�����ڴ�] ׷�Ӵ�����ˮ����,[tranid=%ld,step=%d]",
                sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
              continue;
            }
          }          
          swVdebug(2,"S0960: ׷�Ӵ�����ˮ�ɹ�,[tranid=%ld,step=%d,location=%c]",
            sgSwt_proc_log.tran_id,sgSwt_proc_log.proc_step,slMsgpack.sMsghead.aSysattr[0]);
        }
      }

      /* ����601��ʱ��������ĸ�swMonrev */
      if(clRevFlag)
      {
        if(ilResult == TRUE)
        {
          /* ������ģ�鷢��������� */
          _swMsgsend(101004,(char *)&slMsgpack);
          _swOrdsend(601,(char *)&slMsgpack,iMBMONREV);
          swVdebug(2,"S0970: ������ģ�鷢��ʱ���������");
        }
        continue;
      }

      /* ���·�ɷ�ʽ */
      if(slMsgpack.sMsghead.cRouter == cROUTEYES)
      {
        swVdebug(2,"S0980: �Զ�����·��");
        slMsgpack.sMsghead.iMsgtype = iMSGROUTE;
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,iMBROUTER,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(1,"S0990: [����/����] qwrite2()����,���ͱ��ĸ�·��ģ��ʧ��,������=%d",ilRc);
          swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S1000: ���ͱ��ĸ�·��ģ��ɹ�");
        continue;
      }
      else
      {
        swVdebug(2,"S1010: ���Զ�����·��");
        slMsgpack.sMsghead.iMsgtype = iMSGROUTE;
      }
    }

    /* ·�ɱ��� */
    if(slMsgpack.sMsghead.iMsgtype == iMSGROUTE)
    {
      swVdebug(2,"S1020: ��������Ϊ·�ɱ���");
/*
      if(slMsgpack.sMsghead.iTranstep == 1)
      {
        _swMsgsend(199001,(char *)&slMsgpack);
      }
      else
        _swMsgsend(199002,(char *)&slMsgpack);
del by baiqj20150127*/   
      /* ���½�����ˮ(����->��ֵ->����) */
	  
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
      assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif	  
      clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef DB_SUPPORT
      ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,clLoc_flag);
#else
	  ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'0');
#endif
      if(ilRc == FAIL)
      {
        swVdebug(1,"S1030: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%d]",llTranid);
        continue;
      }
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S1040: û���ҵ�������ˮ,[tranid=%d]",llTranid);
        continue;
      }
      swVdebug(2,"S1050: �ҵ�������ˮ,[tranid=%ld]",llTranid);
       
      /* ��ȡ�˿����� */
      ilRc = swShmselect_swt_sys_queue(ilQdes,&sgSwt_sys_queue);
      if(ilRc == FAIL)
      {
        swVdebug(1,"S1060: [����/�����ڴ�] ��ȡ�˿����ó���,[qid=%d]",ilQdes);
        continue;
      }
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S1070: [����/�����ڴ�] �˿�û������,[qid=%d]",ilQdes);
        continue;
      }
      swVdebug(2,"S1080: ��ȡ�˿����óɹ�,[qid=%d]",ilQdes);
       
      time(&llTran_begin);
      llTran_overtime = sgSwt_sys_queue.rev_overtime + llTran_begin;
      sgSwt_tran_log.tran_overtime = llTran_overtime;
      sgSwt_tran_log.q_target = ilQdes;

#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
          assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif      
      clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
      ilRc = swShmupdate_swt_tran_log(llTranid,sgSwt_tran_log,clLoc_flag);
      if(ilRc == FAIL)
      {
        swVdebug(1,"S1090: [����/�����ڴ�] ���½�����ˮ����,[tranid=%ld]",llTranid);
        continue;
      }   
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S1100: û���ҵ�������ˮ,[tranid=%ld]",llTranid);
        continue;
      }   
      swVdebug(2,"S1110: ���½�����ˮ�ɹ�,[tranid=%ld]",llTranid); 
 
      slMsgpack.sMsghead.iTranstep += 1;
      swVdebug(2,"S1120: ����ͷ.���ײ���+1[%d]",slMsgpack.sMsghead.iTranstep);

      /* ����ͷ.��ʱ����ģʽ = 1��3,�������������֪ͨ */
      if((slMsgpack.sMsghead.iRevmode == 1)||(slMsgpack.sMsghead.iRevmode == 3)) 
      {
        swVdebug(2,"S1130: ��ʱ����ģʽΪ�������������֪ͨ");
         
        /* ��д������ˮ���� */
        memset((char *)&sgSwt_proc_log,0x00,sizeof(struct swt_proc_log));
        sgSwt_proc_log.tran_id = llTranid;
        sgSwt_proc_log.proc_step = slMsgpack.sMsghead.iTranstep;
        sgSwt_proc_log.proc_status = iTRNING;
        sgSwt_proc_log.q_target = ilQdes;
        sgSwt_proc_log.rev_mode = slMsgpack.sMsghead.iRevmode;
        sgSwt_proc_log.rev_fmtgrp = slMsgpack.sMsghead.iRevfmtgroup;

        /* Ŀ�����䲻֧�ֶ�γ���, ɾ���˱ʽ��ס�������Ĵ�����ˮ�����¼ */
        if(sgSwt_sys_queue.rev_nomrev[0] == cTRUE)
        {
          swVdebug(2,"S1140: Ŀ�����䲻֧�ֶ�γ���,ɾ���Ѽ�¼�Ķ�ʴ�����ˮ");
          clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
		  
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
		  assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif

          ilRc = swShmdelete_proclog_tran_q(llTranid,ilQdes,clLoc_flag);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S1150: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld,qid=%d,step=*]",llTranid,ilQdes);
            continue;
          }
          swVdebug(2,"S1160: ɾ��������ˮ�ɹ�,[tranid=%ld,qid=%d,step=*]",llTranid,ilQdes); 
        }

        /* ���汨�� */
        if(ilPacksaveflag == 0)
        {
          ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
          ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,alPackfile);
          if(ilRc == FAIL)  
          {
            swVdebug(1,"S1170: [����/��������] swPutlog()����,���汨�ĳ���,������=%d",ilRc);
            continue;
          }
          swVdebug(2,"S1180: ���汨�ĳɹ�");
          ilPacksaveflag = 1;
        }
        memcpy(sgSwt_proc_log.org_file,alPackfile,sizeof(alPackfile));
        sgSwt_proc_log.org_offset = llPackoffset;
        sgSwt_proc_log.org_len = ilPacklen;
        /* ׷�Ӵ�����ˮ */          
        /* ������ˮ�ڹ����ڴ��� */
        if(slMsgpack.sMsghead.aSysattr[0] == '0')
        {
          ilRc=0;
          /* ���ڴ������� < �˿�ת�ƴ����������� */
          /*Delete By GXZ 2003.1.7 
          if( sgSwt_sys_queue.traning_num <= sgSwt_sys_queue.transhm_max )*/
          /*Add By GXZ 2003.1.7 Begin*/
          if( sgSwt_sys_queue.transhm_max <= 0 || sgSwt_sys_queue.traning_num <= sgSwt_sys_queue.transhm_max )
          {
            ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'0');
            if(ilRc == FAIL)
            {
              swVdebug(1,"S1190: [����/�����ڴ�] ׷�Ӵ�����ˮ����,[tranid=%ld,step=%d]",sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
              continue;
            }
   #ifdef DB_SUPPORT  /*support database 2014-12*/    
/*#endif   del by baiqj20150415 PSBC_V1.0*/
            /*ADD BY GXZ 2003.1.7 Begin*/
            if(ilRc == SHMFULL) 
            {
              ilRc = swShm2db_swt_tran_proc_log(llTranid);
              if(ilRc == FAIL)
              {
                swVdebug(1,"S1201: [����/�����ڴ�] ת����ˮ�����ݿ����,[tranid=%ld]",llTranid);
                continue;
              }
              slMsgpack.sMsghead.aSysattr[0] = '1';
              swVdebug(2,"S1211: ������ˮ�ʹ�����ˮ�ɹ�ת�Ƶ����ݿ� [tranid=%ld]",llTranid);
            }
  #endif   /*add by baiqj20150415 PSBC_V1.0*/
  /* #ifdef DB_SUPPORT  support database 2014-12 del by baiqj20150415 PSBC_V1.0*/   
    #ifdef ONLY_SHM_SUPPORTED  /*add by baiqj20150415 PSBC_V1.0*/
              if(ilRc == SHMFULL) 	              
	      {
                swVdebug(1,"S1212: [����/�����ڴ�] swt_proc_log�����ڴ�������[tranid=%ld]",llTranid);
                continue;
              }
     #endif
            /*ADD BY GXZ 2003.1.7 End*/
          }
          /* ������ˮ�����ڴ��������㽻��ת������,���н��׺ʹ�����ˮ�����ݿ� */
          /*Delete By GXZ 2003.1.7 
          if( (ilRc == SHMFULL) || 
              ( sgSwt_sys_queue.traning_num > sgSwt_sys_queue.transhm_max ) )*/
          /*ADD BY GXZ 2003.1.7 Begin*/
          else
          /*ADD BY GXZ 2003.1.7 End*/
          {
#ifdef DB_SUPPORT 		  	/*support database 2014-12*/
/*#endif  del by baiqj20150415 PSBC_V1.0 */
            ilRc = swShm2db_swt_tran_proc_log(llTranid);
            if(ilRc == FAIL)
            {
              swVdebug(1,"S1200: [����/�����ڴ�] ת����ˮ�����ݿ����,[tranid=%ld]",llTranid);
              continue;
            }
            slMsgpack.sMsghead.aSysattr[0] = '1';
            swVdebug(2,"S1210: ������ˮ�ʹ�����ˮ�ɹ�ת�Ƶ����ݿ� [tranid=%ld]",
                     llTranid);
#endif /*add by baiqj20150415 PSBC_V1.0*/
/*#ifdef DB_SUPPORT 	  del by baiqj20150415 PSBC_V1.0*/
#ifdef ONLY_SHM_SUPPORTED 	  /*add by baiqj20150415 PSBC_V1.0*/
            swVdebug(1,"S1211: [����/�����ڴ�] ���ڴ������� > �˿�ת�ƴ�����������,[tranid=%ld]",llTranid);
            continue;
#endif
          }
        }
        /* ������ˮ�����ݿ���,��������ת��(��ʱ���ʴ�����ˮ��û�м�¼)ʱ��
           ������ˮ�������ݿ��� */

#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
        assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif           
        /*if(slMsgpack.sMsghead.aSysattr[0] == '1') del by baiqj20150415 PSBC_V1.0*/
        if(slMsgpack.sMsghead.aSysattr[0] != '1') /*add by baiqj20150415 PSBC_V1.0*/
        {
          swVdebug(2,"S1220: ׷�Ӵ�����ˮ,[tranid=%ld,step=%d]",
              sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
          /*ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'1');  del by baiqj20150415 PSBC_V1.0*/
          ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'0');  /*add by baiqj20150415 PSBC_V1.0*/
          if(ilRc == FAIL)
          {
            swVdebug(1,"S1230: [����/�����ڴ�] ׷�Ӵ�����ˮ����,[tranid=%ld,step=%d]",
              sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
            continue;
          }
        }          
        swVdebug(2,"S1240: ׷�Ӵ�����ˮ�ɹ�,[tranid=%ld,step=%d,location=%c]",
          sgSwt_proc_log.tran_id,sgSwt_proc_log.proc_step,slMsgpack.sMsghead.aSysattr[0]);
       }

      /* ����ͷ.���׽�����ʶΪ�� */
      if(slMsgpack.sMsghead.cEndflag == cTRUE)
      {
        swVdebug(2,"S1250: ����ͷ.���׽�����ʶΪ��");
        /* ���汨�� */
        if (ilPacksaveflag == 0)
        {
          ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
          ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,alPackfile);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S1260: [����/��������] swPutlog()����,���汨�ĳ���,������=%d",ilRc);
            continue;
          }
          swVdebug(2,"S1270: ���汨�ĳɹ�");
          ilPacksaveflag = 1;
        }
        
        time(&sgSwt_tran_log.tran_end);
        sgSwt_tran_log.tran_status = iTRNEND;
        memcpy(sgSwt_tran_log.resp_file,alPackfile,sizeof(alPackfile));
        sgSwt_tran_log.resp_offset = llPackoffset;
        sgSwt_tran_log.resp_len = ilPacklen;
        memcpy(sgSwt_tran_log.tc_tran_begin,slMsgpack.sMsghead.aBegin_trancode,
          sizeof(slMsgpack.sMsghead.aBegin_trancode));

        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
        assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif

        /* modify by cy 20040324
        ilRc = swShmupdate_swt_tran_log(llTranid,sgSwt_tran_log,clLoc_flag); */
        ilRc = swShmupdate_swt_tran_log_end(llTranid,sgSwt_tran_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S1280: [����/�����ڴ�] ���½�����ˮ����,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          /* modify by cy 20040324
          swVdebug(2,"S1290: û���ҵ�������ˮ,[tranid=%ld]",llTranid);*/
          swVdebug(2,"S1290: û���ҵ����ڽ��׵�ԭ��ˮ,[tranid=%ld]",llTranid);
          continue;
        }  
        swVdebug(2,"S1300: ���½�����ˮ�ɹ�,[tranid=%ld]",llTranid);
      }

      /*  ��ʽת����ʽ */
      if(slMsgpack.sMsghead.cFormatter == cCONVYES)
      {
        swVdebug(2,"S1310: �Զ����и�ʽת��");
        slMsgpack.sMsghead.iMsgtype = iMSGPACK;
        ilRc = swFormat(&slMsgpack);
        if(ilRc)
        {
          swVdebug(1,"S1320: [����/��������] swFormat()����,���ø�ʽת����������,������=%d",ilRc);
          continue;
        }  
        swVdebug(2,"S1330: ���ø�ʽת�������ɹ�");
      }
      else
      {
        swVdebug(2,"S1340: ���Զ����и�ʽת��");
        slMsgpack.sMsghead.iMsgtype = iMSGPACK;
      }
    }
 
    /* ������� */
    if(slMsgpack.sMsghead.iMsgtype == iMSGPACK)
    {
      swVdebug(2,"S1350: ��������Ϊ�������");
      slMsgpack.sMsghead.iMsgtype = iMSGAPP;
      if(slMsgpack.sMsghead.iDes_q == 0)
      {
        swVdebug(1,"S1360: [����/����] ����ͷ.Ŀ������û������");
        continue;
      }
      /* ���ͱ�����ǰ������ */
      ilRc = qwrite2((char *)&slMsgpack,ilMsglen,slMsgpack.sMsghead.iDes_q,
        ilPriority,ilClass,ilType);
      if(ilRc)
      {
        swVdebug(1,"S1370: [����/����] ���ͱ��ĸ�ǰ�ý��̳���,[qid=%d],������=%d",slMsgpack.sMsghead.iDes_q,ilRc);
        swMberror(ilRc,(char *)&slMsgpack);
        swQuit(FAIL);
      }
      swVdebug(2,"S1380: ���ͱ��ĸ�ǰ�ý��̳ɹ�,[qid=%d]",slMsgpack.sMsghead.iDes_q);
      /* ����ͷ.���׽�����ʶΪ�� */
      if(slMsgpack.sMsghead.cEndflag == cTRUE)
      {
        swVdebug(2,"S1390: ������������,���ס�������ˮ����ʷ");
		
#ifdef MOVELOGFLAG
        /* ������ˮ����ʷ */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
        assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
        ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S1400: [����/�����ڴ�] ���ҽ�����ˮ����,[tranid=%ld]",llTranid);
           continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S1410: [����/�����ڴ�] û���ҵ�������ˮ,[tranid=%ld],[clLoc_flag=%c]",llTranid,clLoc_flag);
           continue;
        }
        swVdebug(2,"S1420: �ҵ�������ˮ,[tranid=%ld]",llTranid);

#ifdef DB_SUPPORT  /*database deprivation project 2014-12*/

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
           :sgSwt_tran_log.rev_key);	/* add by fanzhijie 2002.01.04 */
        if(sqlca.sqlcode)
        {
          if(sqlca.sqlcode == SQLDUPKEY)
          {
            swVdebug(1,"S1430: [����/���ݿ�] ������ˮ�ѱ�������ʷ,sqlcode=%d",sqlca.sqlcode);
            EXEC SQL ROLLBACK WORK;
            continue;
          }
          swVdebug(1,"S1440: [����/���ݿ�] INSERT INTO swt_his_tran_log,\
            tranid=%d,sqlca.sqlcode=%d",llTranid,sqlca.sqlcode);
          swDberror((char *)&slMsgpack);
          EXEC SQL ROLLBACK WORK;
          continue;
        }
        if(sqlca.sqlerrd[2] == 0) 
        {
          swVdebug(1,"S1450: [����/���ݿ�] ������ˮ�ѱ�������ʷ,sqlcode=%d",sqlca.sqlerrd[2]);
          EXEC SQL ROLLBACK WORK;
          continue;
        }
#else
        /*database deprivation project 2014-12*/
        /*
    sw_init_logfile();
	ilRc = sw_put_tranlog(&sgSwt_tran_log); 
    */
        /*if db not supported, tranlog will write to file system specified by switch itself*/
        /*
	if(ilRc == FAIL)
	{
		swVdebug(0,"S1456: [����/�ļ�ϵͳ] �����Ƴ���tranlog��־�б���,[tranid=%ld]",llTranid);
		continue;
	}
    */
#endif
        /* ɾ��������ˮ */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
	assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
        ilRc = swShmdelete_swt_tran_log(llTranid,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S1460: [����/�����ڴ�] ɾ��������ˮ����,[tranid=%ld]",llTranid);
          #ifdef DB_SUPPORT  /*support database 2014-12*/
          EXEC SQL ROLLBACK WORK;
          #endif
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S1470: [����/�����ڴ�] û���ҵ�������ˮ,[tranid=%ld],[clLoc_flag=%c]",llTranid,clLoc_flag);
          #ifdef DB_SUPPORT  /*support database 2014-12*/
          EXEC SQL ROLLBACK WORK;
          #endif
          continue;
        }
        swVdebug(2,"S1480: ɾ��������ˮ�ɹ�,[tranid=%ld]",llTranid);
 
        /* �ƴ�����ˮ����ʷ */
        
        ilRc = swShmselect_swt_proc_log_mrec(llTranid,pslSwt_proc_log,&ilCount);
        if (ilRc == FAIL)
        {
          swVdebug(1,"S1490: [����/�����ڴ�] ���Ҵ�����ˮ����,[tranid=%ld,step=*]",llTranid);
          #ifdef DB_SUPPORT  /*support database 2014-12*/
          EXEC SQL ROLLBACK WORK;
          #endif
          return (FAIL);
        }  
 
        for (i = 0; i < ilCount; i ++)
        {
#ifdef DB_SUPPORT
          memcpy((char *)&sgSwt_proc_log,
           (char *)&(pslSwt_proc_log[i]),sizeof(struct swt_proc_log));

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

          if (sqlca.sqlcode != 0)
          {
            swVdebug(1,"S1500: [����/���ݿ�] ׷����ʷ������ˮʧ��,[tranid=%ld,step=%d],sqlcode=%d",llTranid,sgSwt_proc_log.proc_step,sqlca.sqlcode);
            swDberror(NULL);
            EXEC SQL ROLLBACK WORK;
            break;
          }
          swVdebug(2,"S1510: ׷����ʷ������ˮ�ɹ�,[tranid=%ld,step=%d]",llTranid,sgSwt_proc_log.proc_step);
#else
          /*modify by zcd 20141229
      sw_init_logfile();
	  ilRc= sw_put_proclog(&(pslSwt_proc_log[i]));
	  if(ilRc == FAIL)
	  {
	  	swVdebug(0,"S1505: [����/�ļ�ϵͳ] �����Ƴ���tranlog��־�б���,[tranid=%ld]",llTranid);
		continue;
	  }		  
      */
#endif
        }
		
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported, Database Deprivation Project, 2014-12 */
	assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif	
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
        ilRc = swShmdelete_swt_proc_log_mrec(llTranid,clLoc_flag);
        if (ilRc == FAIL)
        {
          swVdebug(1,"S1520: [����/�����ڴ�] ɾ��������ˮʧ��,[tranid=%ld]",llTranid);
          #ifdef DB_SUPPORT  /*support database 2014-12*/
          EXEC SQL ROLLBACK WORK;
          #endif
          return (FAIL);
        }
        swVdebug(2,"S1530: ɾ��������ˮ�ɹ�,[tranid=%ld]",llTranid);

 /*need review again*/
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        /* ɾ������ͷ�洢���еĴ˽�����ˮ�ű��ļ�¼ */
        EXEC SQL DELETE FROM swt_msghead WHERE tran_id = :llTranid;
        if(sqlca.sqlcode && (sqlca.sqlcode != SQLNOTFOUND))
        {
          swVdebug(1,"S1540: [����/���ݿ�] DELETE FROM swt_msghead,sqlcode=%d",sqlca.sqlcode);
          swDberror((char *)&slMsgpack);
          EXEC SQL ROLLBACK WORK;
          continue;
        } 
        swVdebug(2,"S1550: ɾ��swt_msghead�иñʽ�����ˮ��¼");
        EXEC SQL COMMIT WORK;
#endif

#endif
      }
      continue;
    }

    /* �������� */
    if(slMsgpack.sMsghead.iMsgtype == iMSGREV)
    {
      swVdebug(2,"S1560: �յ���������");
      if(ilQid == iMBMONREV)
      {
        swVdebug(2,"S1570: ��MONREV����,ת����ǰ�ý���...");
         
        /* ���ͱ��� */ 
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,ilQdes,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(1,"S1580: [����/����] qwrite2()����,д����[%d]����,������=%d",ilQdes,ilRc);
          swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S1590: д����[%d]�ɹ�",ilQdes);
      }
      else
      {
        swVdebug(2,"S1600: ��ǰ�ý��̷���,ת����MONREV...");
         
        /* ���ͱ��� */
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,iMBMONREV,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(1,"S1610: [����/����] д����[iMBMONREV]����,������=%d",ilRc);
           swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S1620: д����[iMBMONREV]�ɹ�");
       }
      continue;
    }
    
    switch(slMsgpack.sMsghead.iMsgtype)
    {
      case iMSGORDER:
        swVdebug(2,"S1630: �յ������,[code=%d]",slMsgpack.sMsghead.lCode);
       case iMSGAPP:
      case iMSGUNPACK:
      case iMSGROUTE:
      case iMSGPACK:
      case iMSGREV:
        swVdebug(2,"S1640: ���Ľ��ճɹ�!************************����********"); 
         continue;
      default:
        swVdebug(2,"S1650: �յ�δ֪����");
        _swMsgsend(301001,(char *)&slMsgpack);
        continue;
    }
  }
}

/**************************************************************
 ** ������      : swQuit
 ** ��  ��      : �˳�ģ�鴦��
 ** ��  ��      : ʷ����
 ** ��������    : 1999/11/25
 ** ����޸�����: 2001/3/23
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : sig       :�ź�
 ** ����ֵ      : void     
***************************************************************/
void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  
  swVdebug(2,"S1660: ����ģ���Ѿ�ֹͣ...");

#ifdef DB_SUPPORT
 
  /* �ر����ݿ����� */
  swDbclose();

#endif

  /* �ر��������� */
  ilRc = qdetach();
  if(ilRc < 0)
  {
    swVdebug(0,"S1670: [����/����] �ر��������Ӵ���,������=[%d]",ilRc);
     exit(FAIL);
  }

  exit(sig);
}
