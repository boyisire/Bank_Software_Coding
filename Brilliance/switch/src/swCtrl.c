/****************************************************************/
/* 模块编号    ：CTRL                                           */
/* 模块名称    ：系统主控模块                                   */
/* 版 本 号    ：V4.3.0                                         */
/* 作    者    ：史正烨                                         */
/* 建立日期    ：1999/11/15                                     */
/* 最后修改日期：2001/4/6                                       */
/* 模块用途    ：交换平台报文集散中枢                           */
/* 本模块中包含如下函数及功能说明：                             */
/*                       （1）int main();                       */
/****************************************************************/

/****************************************************************/
/* 修改记录:                                                    */
/*  2000.2.19 将报文存储从文件改成数据库中的BLOB型字段(史正烨)  */
/*  2000.2.19 源发邮箱超时(冲正、通知)的支持(史正烨)            */
/*  2000.2.25 将报文存储从数据库改成文件存放(史正烨)            */
/*  2000.3.07 交换平台多级级联的支持                            */
/*  2001/2/27 增加DEBUG级别                                     */
/*  2001/3/24 封版V3.01,程序共1772行                            */
/*  2001/4/3  DB2移植(1797)                                     */
/*  2001/4/6  INFORMIX移植(1774)                                */
/*  2001/6/5  数据库操作改为共享内存操作                        */
/****************************************************************/

/* 头文件定义 */
#include        "switch.h"

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* 数据库定义 */
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

/* 常量定义 */
#define DEBUG		1
#define UNIONFLAG	1
#define MOVELOGFLAG	1

/* 变量定义 */
short ilRc;

/* 函数原型定义 */
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
  static  char  clLoc_flag;             /* 报文存放位置标志 add by xujun 2002.09.12 */

/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/

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
 ** 函数名: main()
 ** 功  能: 交换主控模块主函数
 ** 作  者: 史正烨、姜晓林
 ** 建立日期: 1999.11.15
 ** 最后修改日期: 2000.2.19
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:
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
  char  clLoc_flag;             /* 报文存放位置标志 add by xujun 2002.09.12 */

/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/

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

  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swCtrl Version 4.3.0",argv[1]);

  /* LICENSE */ 
  if (_swLicense("swCtrl")) exit(FAIL);
 
  /* 处理信号 */
  signal(SIGTERM,swQuit);
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);

  /* 设置调试程序名称 */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  /* strcpy(agDebugfile,"swCtrl.debug"); */
   /*  sprintf(agDebugfile,"swCtrl_%d.debug",getpid());*/

  swLogInit("swCtrl", NULL); /*add by dyw, 2015.03.27, PSBC_V1.0*/

  /* 读取DEBUG标志 */
  if((cgDebug = _swDebugflag("swCtrl")) == FAIL)
  {
    fprintf(stderr,"读取DEBUG标志出错!");
    exit(FAIL);
  }
  swVdebug(2,"S0010: 版本号[4.3.0]");
  swVdebug(2,"S0020: DEBUG文件[%s]",agDebugfile);
  swVdebug(2,"S0030: DEBUG标志[%d]",cgDebug);

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/
  /* 打开数据库 */
  if(swDbopen() != SUCCESS)
  {
    swVdebug(0,"S0040: [错误/数据库] swDbopen()打开数据库出错");
    exit(FAIL);
  }
  swVdebug(2,"S0050: 打开数据库成功");
#endif

  /* 打开邮箱 */
  ilRc = qattach(iMBCTRL);
  if(ilRc)
  {
    swVdebug(0,"S0060: [错误/邮箱] qattach()函数,返回码=%d",ilRc);
    exit(FAIL);
  }
  swVdebug(2,"S0070: 打开邮箱成功");
   
  /* 初始化共享内存指针 */
  if(swShmcheck() != SUCCESS)
  {
    swVdebug(0,"S0080: [错误/共享内存] 初始化共享内存指针失败");
    exit(FAIL);
  }
  swVdebug(2,"S0090: 初始化共享内存指针成功");
  
  /* 向系统监控发送信息报文 */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: 发送104001信息报文,主控模块已经启动...");
 
  /* 循环处理报文 */
  for(;;)
  {
    /* 阻塞读邮箱 */
    ilMsglen = iMSGMAXLEN;
    ilPriority = ilClass = ilType = 0;
    
    /* swVdebug(0,"S0101:  qread2()函数,"); */

    ilRc = qread2((char *)&slMsgpack,&ilMsglen,
      &ilOrgqid,&ilPriority,&ilClass,&ilType);
    /* swVdebug(0,"S0102:  qread2()函数,"); */

    if(ilRc != SUCCESS)
    {
      swVdebug(0,"S0110: [错误/邮箱] qread2()函数,返回码=%d",ilRc);
      swMberror(ilRc,(char *)&slMsgpack);
      swQuit(FAIL);
    }
    /*==begin==设置日志名和日志级别, cjh, 2015.03.23, PSBC_V1.0*/
    cgDebug=slMsgpack.sMsghead.log_level;


    memset(agDebugfile,0x00,sizeof(agDebugfile));
    memcpy(agDebugfile,slMsgpack.sMsghead.file_log_name,sizeof(slMsgpack.sMsghead.file_log_name));
    
    /*==end==设置日志名和日志级别, PSBC_V1.0 */
    /* 刷新共享内存指针 */
	
    if(swShmcheck() != SUCCESS)
    {
      swVdebug(0,"S0120: [错误/共享内存] 共享内存指针刷新失败");
      swQuit(FAIL);
    }


    ilPacksaveflag = 0; /* 报文已保存至LOG文件的标识 */
    clRevFlag = 0; /* 交易是否要冲正标识 */

    ftime(&fb);
    if(cgDebug>=2)
    {
      swVdebug(3,"S0130: [%d毫秒]收到报文:邮箱=[%d],优先级=[%d],class=[%d],type=[%d]",fb.millitm,ilOrgqid,ilPriority,ilClass,ilType);
      swDebugmsghead((char *)&slMsgpack);
      swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      if(slMsgpack.sMsghead.iMsgformat == iFMTIMF)
        swDebugfml((char *)&slMsgpack);
    }  
    swVDebugfml(2, (char*)&slMsgpack);

    /* 取得报文头信息 */
    llTranid = slMsgpack.sMsghead.lTranid;
    ilQdes = slMsgpack.sMsghead.iDes_q;
    ilQorg = slMsgpack.sMsghead.iOrg_q;
    memset(alTrancode,0x00,sizeof(alTrancode));
    memcpy(alTrancode,slMsgpack.sMsghead.aTrancode,
      sizeof(slMsgpack.sMsghead.aTrancode));
      
    /* 收到应用报文 */
    if(slMsgpack.sMsghead.iMsgtype == iMSGAPP)
    {
      swVdebug(2,"S0140: 报文类型为应用报文");

     // printf("S0140: 报文类型为应用报文\n");

      /* 报文头.SAF标识为真 */
      if(slMsgpack.sMsghead.cSafflag == cTRUE)
      {
        swVdebug(2,"S0150: 报文头.SAF标识为真");

        printf("S0150: 报文头.SAF标识为真\n");

        /* 将报文转发给事务管理模块处理 */
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,iMBMONREV,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(0,"S0160: [错误/邮箱] qwrite2(),写事务管理邮箱出错,返回码=%d",ilRc);

          //printf("S0160: [错误/邮箱] qwrite2(),写事务管理邮箱出错,返回码=%d",ilRc);

          swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S0170: 写事务管理邮箱成功");
        continue;
      }

      /* 查找交易流水 */
#ifdef DB_SUPPORT  /*support database 2014-12*/
      ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'a');
#else
	  ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'0'); /*only shm supported*/
#endif
      if(ilRc == FAIL)
      {
        swVdebug(1,"S0180: [错误/共享内存] 查找交易流水出错,[tranid=%ld]",llTranid);
        continue;
      }
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0190: 交易流水没有找到,[tranid=%ld]",llTranid);
        if(slMsgpack.sMsghead.iTranstep == 1) 
        {/*
#ifdef TRANREVLOG        
          EXEC SQL SELECT * FROM swt_tranrev_log WHERE tran_id = :llTranid;
          if(!sqlca.sqlcode)
          {
            swVdebug(1,"S0192: 正交易在反交易之后，不处理，tran_id=%ld",llTranid);
            continue;
          }
          else if(!(sqlca.sqlcode == SQLNOTFOUND))
          {
            swVdebug(0,"S0194: select from swt_tranrev_log failed, sqlcode=%d",sqlca.sqlcode);
            continue;
          }
#endif     */   
          /* 交易步骤为1时(新交易),填写交易流水主表 */
          swVdebug(2,"S0200: 交易步骤为1(新交易),追加交易流水");
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
		  	swVdebug(1,"S0205: [错误/共享内存]共享内存已满，无法处理新的交易,[tranid=%ld]",llTranid);
			continue;
		  }				
#endif

          if(clLoc_flag == '1') slMsgpack.sMsghead.aSysattr[0] = '1'; 
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0210: [错误/共享内存] 追加交易流水出错,[tranid=%ld]",llTranid);
            continue;
          }  
          swVdebug(2,"S0220: 追加交易流水成功,[tranid=%ld]",llTranid);
        }
        else
        {
          /* 交易步骤>1并且冲正模式为根据响应冲正 */
          swVdebug(2,"S0230: 交易步骤不为1,迟到响应,[tranid=%ld,step=%d]",
            llTranid,slMsgpack.sMsghead.iTranstep);
          if((slMsgpack.sMsghead.iRevmode != 2) && (slMsgpack.sMsghead.iRevmode != 6))
            continue;
		  
#ifdef DB_SUPPORT /*support database 2014-12*/
          swVdebug(2,"S0240: 并且冲正模式为根据响应冲正,补登待冲正流水");
          /* 查找历史交易流水 */
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
              swVdebug(2,"S0250: 没有找到历史交易流水,[tranid=%ld]",llTranid);
              continue;
            }  
            else
            {
              swVdebug(2,"S0260: 查找历史交易流水出错,[tranid=%ld]",llTranid);
              swDberror(NULL);
              continue;
            }    
          }
          swVdebug(2,"S0270: 找到历史交易流水,[tranid=%ld]",llTranid);
          if(sgSwt_tran_log.tran_status != iTRNREVEND)
          {
            swVdebug(2,"S0280: 交易状态不为冲正结束,[tranid=%ld]",llTranid);
            continue;
          } 
          /* 查找历史处理流水 */
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
            swVdebug(2,"S0290: 找到历史处理流水,无需补登,[tranid=%ld,step=%d]",llTranid,ilTranstep);
            continue;
          }
          if (sqlca.sqlcode != SQLNOTFOUND)
          {
            swVdebug(2,"S0300: 查找历史处理流水出错");
            swDberror(NULL);
            continue;
          }
          /* 历史交易流水存在，但历史处理流水不存在 */
          /* 将历史交易流水移到当前交易流水 */
#ifdef DB_INFORMIX
          EXEC SQL BEGIN WORK;
#endif          
          EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTranid;
          if(sqlca.sqlcode)
          {
            swVdebug(1,"S0310: 删除历史交易流水出错,[tranid=%ld]",llTranid);
            swDberror(NULL);
            EXEC SQL ROLLBACK WORK;
            continue;
          }
          swVdebug(2,"S0320: 删除历史交易流水成功,[tranid=%ld]",llTranid);
          clLoc_flag='1';
          ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
          if(clLoc_flag == '1') slMsgpack.sMsghead.aSysattr[0] = '1';
          if (ilRc != SUCCESS)
          {
            swVdebug(1,"S0330: [错误/共享内存] 追加交易流水出错,[tranid=%ld]",llTranid);
            EXEC SQL ROLLBACK WORK;
            continue;
          }  
          swVdebug(2,"S0340: 追加交易流水成功,tranid=%ld",llTranid);
          EXEC SQL COMMIT WORK;

          clRevFlag = 1;
#endif
        }
      }
      else if(sgSwt_tran_log.tran_status == iTRNREVING)
      { 
        if((slMsgpack.sMsghead.iRevmode == 2) || (slMsgpack.sMsghead.iRevmode == 6))
        {
          swVdebug(2,"S0350: 并且冲正模式为根据响应冲正");
          ilTranstep = slMsgpack.sMsghead.iTranstep;
#ifdef DB_SUPPORT  /*support database 2014-12*/
          ilRc = swShmselect_swt_proc_log(llTranid,ilTranstep,&sgSwt_proc_log,'a');
#else
		  ilRc = swShmselect_swt_proc_log(llTranid,ilTranstep,&sgSwt_proc_log,'0');
#endif
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0360: [错误/共享内存] 查找处理流水出错,tranid=%ld,step=%d",
              llTranid,ilTranstep);
            continue;
          }
          else if(!ilRc)
          {
            swVdebug(2,"S0370: 找到处理流水,tranid=%ld,step=%d,不处理",llTranid,ilTranstep);
            continue;
          }
        }
        swVdebug(2,"S0371: 该交易流水为正在冲正状态,置冲正flag,tranid=[%ld]",llTranid);
        clRevFlag = 1;
      }
      else if (sgSwt_tran_log.tran_status == iTRNREVFAIL)
      {
        swVdebug(2,"S0380: 找到交易流水,状态为冲正失败,不处理,tranid=%ld",
          llTranid);
        continue;
      }
      else if (sgSwt_tran_log.tran_status == iTRNREVEND)
      {
        swVdebug(2,"S0390: 找到交易流水,状态为冲正结束,tranid=%ld",llTranid);
        if((slMsgpack.sMsghead.iRevmode != 2) && (slMsgpack.sMsghead.iRevmode != 6))
          continue;

        swVdebug(2,"S0400: 并且冲正模式为根据响应冲正");
        clRevFlag = 1;
      }
      else if (sgSwt_tran_log.tran_status == iTRNEND)
      {
        swVdebug(2,"S0410: 找到交易流水,状态为交易结束,不处理,tranid=%ld",
          llTranid);
        continue;
      }
      else
      {
        swVdebug(2,"S0420: 找到交易流水,状态为正在交易,tranid=%ld",llTranid);
         ilTran_status = sgSwt_tran_log.tran_status;
      }
      
      /* 查看格式转换方式 */
      if(slMsgpack.sMsghead.cFormatter == cCONVYES)
      {
        swVdebug(2,"S0430: 自动进行格式转换");
        slMsgpack.sMsghead.iMsgtype = iMSGUNPACK;
        if(swFormat(&slMsgpack) != SUCCESS)
        {
          swVdebug(1,"S0440: 调用格式转换函数出错");
          continue;
        }  
        swVdebug(2,"S0450: 调用格式转换函数成功");
      }
      else
      {
        swVdebug(2,"S0460: 不进行自动格式转换");
        slMsgpack.sMsghead.iMsgtype = iMSGUNPACK;
      }
    }



    /* 解包报文 */
    if(slMsgpack.sMsghead.iMsgtype == iMSGUNPACK)
    {
      /* 记录起始交易码 */
      if(slMsgpack.sMsghead.iTranstep == 1)
      {
        memcpy(slMsgpack.sMsghead.aBegin_trancode,slMsgpack.sMsghead.aTrancode,
          sizeof(slMsgpack.sMsghead.aTrancode));
      }
      /* 检查报文类型 */
      if(slMsgpack.sMsghead.iMsgformat != iFMTIMF)
      {
        swVdebug(2,"S0470: 报文格式不为平台内部报文FML格式");
        continue;
      }
      
#ifdef UNIONFLAG
      /* 取得响应报文存放文件、偏移量、长度 */
      if (slMsgpack.sMsghead.iTranstep == 1)
        ilPacklen = 0;
      else
      {
        /* 查找交易流水 */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef DB_SUPPORT
        ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,clLoc_flag);
#else
		ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'0');
#endif
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0480: [错误/共享内存] 查找交易流水出错,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0490: [错误/共享内存] 交易流水没有找到,[tranid=%ld]",llTranid);
          continue;
        }
        swVdebug(2,"S0500: 找到交易流水,[tranid=%ld]",llTranid);
        memcpy(alPackfile,sgSwt_tran_log.resp_file,sizeof(alPackfile));
        llPackoffset = sgSwt_tran_log.resp_offset;
        ilPacklen = sgSwt_tran_log.resp_len;
      }

      if(ilPacklen)
      {
        ilRc = swGetlog(alPackfile,ilPacklen,llPackoffset,alPackbuffer);
        if (ilRc == FAIL)  
        {
          swVdebug(1,"S0510: [错误/函数调用] swGetlog()函数,取报文出错");
          continue;
        }
        swVdebug(2,"S0520: 取报文成功");  
        pslBlob = (struct msgpack *)alPackbuffer;
        swFmlunpack(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,pulPrebuf1);
swVdebug(2,"------------------------1------------------------");
        swFmlunpack(pslBlob->aMsgbody,pslBlob->sMsghead.iBodylen,psgPreunpackbuf);

swVdebug(2,"------------------------2------------------------");
            
        /* 将读出的报文与当前报文进行并集操作 */
        _swFmlunion(psgPreunpackbuf,pulPrebuf1);
swVdebug(2,"------------------------3------------------------");
        swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,&ilMsglen);
        slMsgpack.sMsghead.iBodylen = ilMsglen;
        ilMsglen = sizeof(struct msghead) + ilMsglen;
        swVdebug(2,"S0530: 报文并集成功");
      }

      /* 保存报文 */
      if(ilPacksaveflag == 0)
      {
        ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
        ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,alPackfile);
        if(ilRc == FAIL)        
        {
          swVdebug(1,"S0540: [错误/函数调用] swPutlog()函数,保存报文出错");
          continue;
        }
        swVdebug(2,"S0550: 保存报文成功");
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
            swVdebug(1,"S0560: [错误/函数调用] swPutlog()函数,保存报文出错");
            continue;
          }
          swVdebug(2,"S0570: 保存报文成功");
          ilPacksaveflag = 1;
        }
        
        /* 更新交易流水(查找->赋值->更新) */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef DB_SUPPORT
        ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,clLoc_flag);
#else
		ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,'0');
#endif
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0580: [错误/共享内存] 查找交易流水出错,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0590: 交易流水没有找到,[tranid=%ld]",llTranid);
          continue;
        }
        swVdebug(2,"S0600: 找到交易流水,[tranid=%ld]",llTranid);
        memcpy(sgSwt_tran_log.resq_file,alPackfile,sizeof(alPackfile));
        sgSwt_tran_log.resq_offset = llPackoffset;
        sgSwt_tran_log.resq_len = ilPacklen;
        memcpy(sgSwt_tran_log.resp_file,alPackfile,sizeof(alPackfile));
        sgSwt_tran_log.resp_offset = llPackoffset;
        sgSwt_tran_log.resp_len = ilPacklen;

/* add by fanzhijie 2002.01.04 */
        /* 源发邮箱超时处理 */     
        ilQid = slMsgpack.sMsghead.iBegin_q_id;
        memset(alTrancode,0x00,sizeof(alTrancode));
        memcpy(alTrancode,slMsgpack.sMsghead.aBegin_trancode,
          sizeof(slMsgpack.sMsghead.aTrancode));

        ilRc = swShmselect_route_d_q_tran(ilQid,alTrancode,&sgSwt_sys_route_d);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0610: [错误/共享内存] 查找路由脚本段出错,[qid=%d,trancode=%s]",ilQid,alTrancode);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0620: 无法定位路由脚本段,[qid=%d,trancode=%s]",ilQid,alTrancode);
          continue;
        }  
        slMsgpack.sMsghead.iRevmode = sgSwt_sys_route_d.rev_mode;
        slMsgpack.sMsghead.iRevfmtgroup = sgSwt_sys_route_d.rev_fmtgrp;
        swVdebug(2,"S0630: 源发邮箱超时处理成功,[qid=%d,trancode=%s]",ilQid,alTrancode);

        /* 主动冲正 */
        alRevkey[0] = '\0';
        if(sgSwt_sys_route_d.route_cond[0] != '\0')
          strcpy(alRevkey,sgSwt_sys_route_d.route_cond);
        else
        {
          ilRc = swShmselect_swt_sys_queue(ilQorg,&sgSwt_sys_queue);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0640: [错误/共享内存] 读端口设置出错,[qid=%d]",ilQorg);
            continue;
          }
          if(ilRc == SHMNOTFOUND)
          {
            swVdebug(1,"S0650: [错误/共享内存] 端口没有设置,[qid=%d]",ilQorg);
            continue;
          }
          swVdebug(2,"S0660: 读端口设置成功,[qid=%d]",ilQorg);
          strcpy(alRevkey,sgSwt_sys_queue.rev_express);
        }
        
        if(alRevkey[0] != '\0')
        {
          ilRc = swFmlunpack(slMsgpack.aMsgbody,
            slMsgpack.sMsghead.iBodylen,psgPreunpackbuf);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0670: [错误/函数调用] swFmlunpack()函数,FML报文解包出错,返回码=%d",ilRc);
            _swMsgsend(399001,NULL);
            continue;
          }

          /* 计算键值 */
          ilRc = _swExpress(alRevkey,alResult,&ilResultlen);
          if(ilRc == FAIL) 
          {
            swVdebug(1,"S0680: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alRevkey,ilRc);
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
          swVdebug(1,"S0690: [错误/共享内存] 更新交易流水出错,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0700: [错误/共享内存] 交易流水没有找到,[tranid=%ld]",llTranid);
          continue;
        }  
        swVdebug(2,"S0710: 更新交易流水成功,[tranid=%ld]",llTranid);
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
          swVdebug(1,"S0720: [错误/共享内存] 更新交易流水出错,[tranid=%ld]",llTranid);
          continue;
        }
#endif          
      }
      ilResult = TRUE;
      
      /* 交易有冲正模式 */
      if(slMsgpack.sMsghead.iRevmode)
      {
        swVdebug(2,"S0730: 交易有冲正模式");
         
        ilRc = swShmselect_swt_sys_queue(ilQorg,&sgSwt_sys_queue);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0740: [错误/共享内存] 读端口设置出错,[qid=%d]",ilQorg);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0750: [错误/共享内存] 端口没有设置,[qid=%d]",ilQorg);
          continue;
        }
        swVdebug(2,"S0760: 读端口设置成功,[qid=%d]",ilQorg);
         
        /* 结果表达式不为空 */
        if(strlen(sgSwt_sys_queue.resu_express)>0 && slMsgpack.sMsghead.iTranstep>1)
        {
          /* 解内部报文 */
          ilResult = FALSE;
          ilRc = swFmlunpack(slMsgpack.aMsgbody,
            slMsgpack.sMsghead.iBodylen,psgPreunpackbuf);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0770: [错误/函数调用] swFmlunpack()函数,FML报文解包出错,返回码=%d",ilRc);
            _swMsgsend(399001,NULL);
            continue;
          }

          /* 计算结果表达式 */
          ilRc = _swExpress(sgSwt_sys_queue.resu_express,alResult,&ilResultlen);
          if(ilRc == FAIL) 
          {
            swVdebug(1,"S0780: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",sgSwt_sys_queue.resu_express,ilRc);
            _swMsgsend(303006,NULL );
            continue;
          }
          else
            if(alResult[0] != '0') ilResult = TRUE;
        }
        else
          ilResult = TRUE;
      }

      /* 冲正模式 = 1或3根据请求冲正、通知 */
      if((ilResult != TRUE) && ((slMsgpack.sMsghead.iRevmode == 1)
        ||(slMsgpack.sMsghead.iRevmode == 3)))
      {
        swVdebug(2,"S0790: 返回报文不成功,且冲正模式为1或3,删除已记录的处理流水");
        ilTranstep = slMsgpack.sMsghead.iTranstep;
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
        ilRc = swShmdelete_swt_proc_log(llTranid,ilTranstep,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0800: [错误/共享内存] 删除处理流水出错,[tranid=%ld,step=%d]",llTranid,ilTranstep);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0810: 处理流水没有找到,[tranid=%ld,step=%d]",llTranid,ilTranstep);
          continue;
        }  
        swVdebug(2,"S0820: 删除处理流水成功,[tranid=%ld,step=%d]",llTranid,ilTranstep);
      }

      /* 冲正模式 = 2或4,根据响应冲正、通知 */
      if((ilResult == TRUE) && ((slMsgpack.sMsghead.iRevmode == 2) || 
        (slMsgpack.sMsghead.iRevmode == 4)))
      {
        swVdebug(2,"S0830: 返回报文成功,且冲正模式为2或4,填写处理流水");
        /* 填写处理流水主表 */
        memset((char *)&sgSwt_proc_log,0x00,sizeof(struct swt_proc_log));
        sgSwt_proc_log.tran_id = llTranid;
        sgSwt_proc_log.proc_step = slMsgpack.sMsghead.iTranstep;
        sgSwt_proc_log.proc_status = iTRNING;
        sgSwt_proc_log.q_target = ilQorg;
        sgSwt_proc_log.rev_mode = slMsgpack.sMsghead.iRevmode;
        sgSwt_proc_log.rev_fmtgrp = slMsgpack.sMsghead.iRevfmtgroup;

        /* 读取端口设置 */
        ilRc = swShmselect_swt_sys_queue(sgSwt_proc_log.q_target,
          &sgSwt_sys_queue);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S0840: [错误/共享内存] 读取端口设置出错,[qid=%d]",sgSwt_proc_log.q_target);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S0850: 端口没有设置,[qid=%d]",sgSwt_proc_log.q_target);
          continue;
        }
        swVdebug(2,"S0860: 读取端口设置成功,[qid=%d]",sgSwt_proc_log.q_target);       

        /* 目标邮箱不支持多次冲正,删除此笔交易、此邮箱的处理流水主表记录 */
        if(sgSwt_sys_queue.rev_nomrev[0] == cTRUE)
        {
          swVdebug(2,"S0870: 目标邮箱不支持多次冲正,删除已记录的多笔处理流水");
          clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
          ilRc = swShmdelete_proclog_tran_q(sgSwt_proc_log.tran_id,
            sgSwt_proc_log.q_target,clLoc_flag);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S0880: [错误/共享内存] 删除处理流水出错,[tranid=%ld,qid=%d,step=*]",
              sgSwt_proc_log.tran_id, sgSwt_proc_log.q_target);
            continue;
          }
          swVdebug(2,"S0890: 删除处理流水成功,[tranid=%ld,qid=%d,step=*]",
            sgSwt_proc_log.tran_id, sgSwt_proc_log.q_target);          
        }

        if(ilResult)
        {
          /* 保存报文 */
          if(ilPacksaveflag == 0)
          {
            ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
            ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,
              alPackfile);

            if(ilRc == FAIL)  
            {
              swVdebug(1,"S0900: [错误/函数调用] swPutlog()函数,保存报文出错");
              continue;
            }
            swVdebug(2,"S0910: 保存报文成功");
            ilPacksaveflag = 1;
          }
          memcpy(sgSwt_proc_log.org_file,alPackfile,sizeof(alPackfile));
          sgSwt_proc_log.org_offset = llPackoffset;
          sgSwt_proc_log.org_len = ilPacklen;
	  
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
          assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
          /* 追加处理流水 */          
          /* 交易流水在共享内存中 */
          if(slMsgpack.sMsghead.aSysattr[0] == '0')
          {
            ilRc=0;
            /* 正在处理交易数 < 端口转移处理的最大交易数 */
            /*MODI BY GXZ 2003.1.7*/
            if( sgSwt_sys_queue.transhm_max <= 0 || sgSwt_sys_queue.traning_num <= sgSwt_sys_queue.transhm_max )
            {
              ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'0');
			  
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
              if(ilRc == SHMFULL) 	              
	      {
                swVdebug(1,"S0919: [错误/共享内存] 共享内存已满[tranid=%ld,step=%d]",
                sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
                continue;
              }
#endif
              if(ilRc == FAIL)
              {
                swVdebug(1,"S0920: [错误/共享内存] 追加处理流水出错,[tranid=%ld,step=%d]",
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
                  swVdebug(1,"S0931: [错误/共享内存] 转移流水到数据库出错,[tranid=%ld]",llTranid);
                  continue;
                }
                slMsgpack.sMsghead.aSysattr[0] = '1';
                swVdebug(2,"S0941: 交易流水和处理流水成功转移到数据库 [tranid=%ld]",llTranid);
              }
              /*ADD BY GXZ 2003.1.7 End*/
            }
            /*处理流水共享内存满或满足交易转移条件,所有交易和处理流水移数据库*/
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
                swVdebug(1,"S0930: [错误/共享内存] 转移流水到数据库出错,[tranid=%ld]",llTranid);
                continue;
              }
              slMsgpack.sMsghead.aSysattr[0] = '1';
              swVdebug(2,"S0940: 交易流水和处理流水成功转移到数据库 [tranid=%ld]",llTranid);
            }
          }
          /* 交易流水在数据库中,或发生数据转移(此时本笔处理流水还没有记录)时，
             处理流水插入数据库中 */
             
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
          assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
          if(slMsgpack.sMsghead.aSysattr[0] == '1')
          {
            ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'1');
            if(ilRc == FAIL)
            {
              swVdebug(1,"S0950: [错误/共享内存] 追加处理流水出错,[tranid=%ld,step=%d]",
                sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
              continue;
            }
          }          
          swVdebug(2,"S0960: 追加处理流水成功,[tranid=%ld,step=%d,location=%c]",
            sgSwt_proc_log.tran_id,sgSwt_proc_log.proc_step,slMsgpack.sMsghead.aSysattr[0]);
        }
      }

      /* 发送601即时冲正命令报文给swMonrev */
      if(clRevFlag)
      {
        if(ilResult == TRUE)
        {
          /* 向事务模块发冲正命令报文 */
          _swMsgsend(101004,(char *)&slMsgpack);
          _swOrdsend(601,(char *)&slMsgpack,iMBMONREV);
          swVdebug(2,"S0970: 向事务模块发即时冲正命令报文");
        }
        continue;
      }

      /* 检查路由方式 */
      if(slMsgpack.sMsghead.cRouter == cROUTEYES)
      {
        swVdebug(2,"S0980: 自动进行路由");
        slMsgpack.sMsghead.iMsgtype = iMSGROUTE;
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,iMBROUTER,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(1,"S0990: [错误/邮箱] qwrite2()函数,发送报文给路由模块失败,错误码=%d",ilRc);
          swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S1000: 发送报文给路由模块成功");
        continue;
      }
      else
      {
        swVdebug(2,"S1010: 不自动进行路由");
        slMsgpack.sMsghead.iMsgtype = iMSGROUTE;
      }
    }

    /* 路由报文 */
    if(slMsgpack.sMsghead.iMsgtype == iMSGROUTE)
    {
      swVdebug(2,"S1020: 报文类型为路由报文");
/*
      if(slMsgpack.sMsghead.iTranstep == 1)
      {
        _swMsgsend(199001,(char *)&slMsgpack);
      }
      else
        _swMsgsend(199002,(char *)&slMsgpack);
del by baiqj20150127*/   
      /* 更新交易流水(查找->赋值->更新) */
	  
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
        swVdebug(1,"S1030: [错误/共享内存] 查找交易流水出错,[tranid=%d]",llTranid);
        continue;
      }
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S1040: 没有找到交易流水,[tranid=%d]",llTranid);
        continue;
      }
      swVdebug(2,"S1050: 找到交易流水,[tranid=%ld]",llTranid);
       
      /* 读取端口设置 */
      ilRc = swShmselect_swt_sys_queue(ilQdes,&sgSwt_sys_queue);
      if(ilRc == FAIL)
      {
        swVdebug(1,"S1060: [错误/共享内存] 读取端口设置出错,[qid=%d]",ilQdes);
        continue;
      }
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S1070: [错误/共享内存] 端口没有设置,[qid=%d]",ilQdes);
        continue;
      }
      swVdebug(2,"S1080: 读取端口设置成功,[qid=%d]",ilQdes);
       
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
        swVdebug(1,"S1090: [错误/共享内存] 更新交易流水出错,[tranid=%ld]",llTranid);
        continue;
      }   
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S1100: 没有找到交易流水,[tranid=%ld]",llTranid);
        continue;
      }   
      swVdebug(2,"S1110: 更新交易流水成功,[tranid=%ld]",llTranid); 
 
      slMsgpack.sMsghead.iTranstep += 1;
      swVdebug(2,"S1120: 报文头.交易步骤+1[%d]",slMsgpack.sMsghead.iTranstep);

      /* 报文头.超时处理模式 = 1或3,根据请求冲正、通知 */
      if((slMsgpack.sMsghead.iRevmode == 1)||(slMsgpack.sMsghead.iRevmode == 3)) 
      {
        swVdebug(2,"S1130: 超时处理模式为根据请求冲正、通知");
         
        /* 填写处理流水主表 */
        memset((char *)&sgSwt_proc_log,0x00,sizeof(struct swt_proc_log));
        sgSwt_proc_log.tran_id = llTranid;
        sgSwt_proc_log.proc_step = slMsgpack.sMsghead.iTranstep;
        sgSwt_proc_log.proc_status = iTRNING;
        sgSwt_proc_log.q_target = ilQdes;
        sgSwt_proc_log.rev_mode = slMsgpack.sMsghead.iRevmode;
        sgSwt_proc_log.rev_fmtgrp = slMsgpack.sMsghead.iRevfmtgroup;

        /* 目标邮箱不支持多次冲正, 删除此笔交易、此邮箱的处理流水主表记录 */
        if(sgSwt_sys_queue.rev_nomrev[0] == cTRUE)
        {
          swVdebug(2,"S1140: 目标邮箱不支持多次冲正,删除已记录的多笔处理流水");
          clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
		  
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
		  assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif

          ilRc = swShmdelete_proclog_tran_q(llTranid,ilQdes,clLoc_flag);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S1150: [错误/共享内存] 删除处理流水出错,[tranid=%ld,qid=%d,step=*]",llTranid,ilQdes);
            continue;
          }
          swVdebug(2,"S1160: 删除处理流水成功,[tranid=%ld,qid=%d,step=*]",llTranid,ilQdes); 
        }

        /* 保存报文 */
        if(ilPacksaveflag == 0)
        {
          ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
          ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,alPackfile);
          if(ilRc == FAIL)  
          {
            swVdebug(1,"S1170: [错误/函数调用] swPutlog()函数,保存报文出错,返回码=%d",ilRc);
            continue;
          }
          swVdebug(2,"S1180: 保存报文成功");
          ilPacksaveflag = 1;
        }
        memcpy(sgSwt_proc_log.org_file,alPackfile,sizeof(alPackfile));
        sgSwt_proc_log.org_offset = llPackoffset;
        sgSwt_proc_log.org_len = ilPacklen;
        /* 追加处理流水 */          
        /* 交易流水在共享内存中 */
        if(slMsgpack.sMsghead.aSysattr[0] == '0')
        {
          ilRc=0;
          /* 正在处理交易数 < 端口转移处理的最大交易数 */
          /*Delete By GXZ 2003.1.7 
          if( sgSwt_sys_queue.traning_num <= sgSwt_sys_queue.transhm_max )*/
          /*Add By GXZ 2003.1.7 Begin*/
          if( sgSwt_sys_queue.transhm_max <= 0 || sgSwt_sys_queue.traning_num <= sgSwt_sys_queue.transhm_max )
          {
            ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'0');
            if(ilRc == FAIL)
            {
              swVdebug(1,"S1190: [错误/共享内存] 追加处理流水出错,[tranid=%ld,step=%d]",sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
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
                swVdebug(1,"S1201: [错误/共享内存] 转移流水到数据库出错,[tranid=%ld]",llTranid);
                continue;
              }
              slMsgpack.sMsghead.aSysattr[0] = '1';
              swVdebug(2,"S1211: 交易流水和处理流水成功转移到数据库 [tranid=%ld]",llTranid);
            }
  #endif   /*add by baiqj20150415 PSBC_V1.0*/
  /* #ifdef DB_SUPPORT  support database 2014-12 del by baiqj20150415 PSBC_V1.0*/   
    #ifdef ONLY_SHM_SUPPORTED  /*add by baiqj20150415 PSBC_V1.0*/
              if(ilRc == SHMFULL) 	              
	      {
                swVdebug(1,"S1212: [错误/共享内存] swt_proc_log共享内存区已满[tranid=%ld]",llTranid);
                continue;
              }
     #endif
            /*ADD BY GXZ 2003.1.7 End*/
          }
          /* 处理流水共享内存满或满足交易转移条件,所有交易和处理流水移数据库 */
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
              swVdebug(1,"S1200: [错误/共享内存] 转移流水到数据库出错,[tranid=%ld]",llTranid);
              continue;
            }
            slMsgpack.sMsghead.aSysattr[0] = '1';
            swVdebug(2,"S1210: 交易流水和处理流水成功转移到数据库 [tranid=%ld]",
                     llTranid);
#endif /*add by baiqj20150415 PSBC_V1.0*/
/*#ifdef DB_SUPPORT 	  del by baiqj20150415 PSBC_V1.0*/
#ifdef ONLY_SHM_SUPPORTED 	  /*add by baiqj20150415 PSBC_V1.0*/
            swVdebug(1,"S1211: [错误/共享内存] 正在处理交易数 > 端口转移处理的最大交易数,[tranid=%ld]",llTranid);
            continue;
#endif
          }
        }
        /* 交易流水在数据库中,或发生数据转移(此时本笔处理流水还没有记录)时，
           处理流水插入数据库中 */

#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
        assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif           
        /*if(slMsgpack.sMsghead.aSysattr[0] == '1') del by baiqj20150415 PSBC_V1.0*/
        if(slMsgpack.sMsghead.aSysattr[0] != '1') /*add by baiqj20150415 PSBC_V1.0*/
        {
          swVdebug(2,"S1220: 追加处理流水,[tranid=%ld,step=%d]",
              sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
          /*ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'1');  del by baiqj20150415 PSBC_V1.0*/
          ilRc = swShminsert_swt_proc_log(sgSwt_proc_log,'0');  /*add by baiqj20150415 PSBC_V1.0*/
          if(ilRc == FAIL)
          {
            swVdebug(1,"S1230: [错误/共享内存] 追加处理流水出错,[tranid=%ld,step=%d]",
              sgSwt_proc_log.tran_id, sgSwt_proc_log.proc_step);
            continue;
          }
        }          
        swVdebug(2,"S1240: 追加处理流水成功,[tranid=%ld,step=%d,location=%c]",
          sgSwt_proc_log.tran_id,sgSwt_proc_log.proc_step,slMsgpack.sMsghead.aSysattr[0]);
       }

      /* 报文头.交易结束标识为真 */
      if(slMsgpack.sMsghead.cEndflag == cTRUE)
      {
        swVdebug(2,"S1250: 报文头.交易结束标识为真");
        /* 保存报文 */
        if (ilPacksaveflag == 0)
        {
          ilPacklen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
          ilRc = swPutlog((char *)&slMsgpack,ilPacklen,&llPackoffset,alPackfile);
          if(ilRc == FAIL)
          {
            swVdebug(1,"S1260: [错误/函数调用] swPutlog()函数,保存报文出错,返回码=%d",ilRc);
            continue;
          }
          swVdebug(2,"S1270: 保存报文成功");
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
          swVdebug(1,"S1280: [错误/共享内存] 更新交易流水出错,[tranid=%ld]",llTranid);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          /* modify by cy 20040324
          swVdebug(2,"S1290: 没有找到交易流水,[tranid=%ld]",llTranid);*/
          swVdebug(2,"S1290: 没有找到正在交易的原流水,[tranid=%ld]",llTranid);
          continue;
        }  
        swVdebug(2,"S1300: 更新交易流水成功,[tranid=%ld]",llTranid);
      }

      /*  格式转换方式 */
      if(slMsgpack.sMsghead.cFormatter == cCONVYES)
      {
        swVdebug(2,"S1310: 自动进行格式转换");
        slMsgpack.sMsghead.iMsgtype = iMSGPACK;
        ilRc = swFormat(&slMsgpack);
        if(ilRc)
        {
          swVdebug(1,"S1320: [错误/函数调用] swFormat()函数,调用格式转换函数出错,返回码=%d",ilRc);
          continue;
        }  
        swVdebug(2,"S1330: 调用格式转换函数成功");
      }
      else
      {
        swVdebug(2,"S1340: 不自动进行格式转换");
        slMsgpack.sMsghead.iMsgtype = iMSGPACK;
      }
    }
 
    /* 打包报文 */
    if(slMsgpack.sMsghead.iMsgtype == iMSGPACK)
    {
      swVdebug(2,"S1350: 报文类型为打包报文");
      slMsgpack.sMsghead.iMsgtype = iMSGAPP;
      if(slMsgpack.sMsghead.iDes_q == 0)
      {
        swVdebug(1,"S1360: [错误/其它] 报文头.目标邮箱没有设置");
        continue;
      }
      /* 发送报文至前置邮箱 */
      ilRc = qwrite2((char *)&slMsgpack,ilMsglen,slMsgpack.sMsghead.iDes_q,
        ilPriority,ilClass,ilType);
      if(ilRc)
      {
        swVdebug(1,"S1370: [错误/邮箱] 发送报文给前置进程出错,[qid=%d],错误码=%d",slMsgpack.sMsghead.iDes_q,ilRc);
        swMberror(ilRc,(char *)&slMsgpack);
        swQuit(FAIL);
      }
      swVdebug(2,"S1380: 发送报文给前置进程成功,[qid=%d]",slMsgpack.sMsghead.iDes_q);
      /* 报文头.交易结束标识为真 */
      if(slMsgpack.sMsghead.cEndflag == cTRUE)
      {
        swVdebug(2,"S1390: 交易正常结束,交易、处理流水移历史");
		
#ifdef MOVELOGFLAG
        /* 交易流水移历史 */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
        assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
        ilRc = swShmselect_swt_tran_log(llTranid,&sgSwt_tran_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S1400: [错误/共享内存] 查找交易流水出错,[tranid=%ld]",llTranid);
           continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S1410: [错误/共享内存] 没有找到交易流水,[tranid=%ld],[clLoc_flag=%c]",llTranid,clLoc_flag);
           continue;
        }
        swVdebug(2,"S1420: 找到交易流水,[tranid=%ld]",llTranid);

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
            swVdebug(1,"S1430: [错误/数据库] 交易流水已被移至历史,sqlcode=%d",sqlca.sqlcode);
            EXEC SQL ROLLBACK WORK;
            continue;
          }
          swVdebug(1,"S1440: [错误/数据库] INSERT INTO swt_his_tran_log,\
            tranid=%d,sqlca.sqlcode=%d",llTranid,sqlca.sqlcode);
          swDberror((char *)&slMsgpack);
          EXEC SQL ROLLBACK WORK;
          continue;
        }
        if(sqlca.sqlerrd[2] == 0) 
        {
          swVdebug(1,"S1450: [错误/数据库] 交易流水已被移至历史,sqlcode=%d",sqlca.sqlerrd[2]);
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
		swVdebug(0,"S1456: [错误/文件系统] 交易移除到tranlog日志中报错,[tranid=%ld]",llTranid);
		continue;
	}
    */
#endif
        /* 删除交易流水 */
        clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
#ifdef ONLY_SHM_SUPPORTED  /* only shm supported */
	assert(slMsgpack.sMsghead.aSysattr[0] != '1');
#endif
        ilRc = swShmdelete_swt_tran_log(llTranid,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(1,"S1460: [错误/共享内存] 删除交易流水出错,[tranid=%ld]",llTranid);
          #ifdef DB_SUPPORT  /*support database 2014-12*/
          EXEC SQL ROLLBACK WORK;
          #endif
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(1,"S1470: [错误/共享内存] 没有找到交易流水,[tranid=%ld],[clLoc_flag=%c]",llTranid,clLoc_flag);
          #ifdef DB_SUPPORT  /*support database 2014-12*/
          EXEC SQL ROLLBACK WORK;
          #endif
          continue;
        }
        swVdebug(2,"S1480: 删除交易流水成功,[tranid=%ld]",llTranid);
 
        /* 移处理流水至历史 */
        
        ilRc = swShmselect_swt_proc_log_mrec(llTranid,pslSwt_proc_log,&ilCount);
        if (ilRc == FAIL)
        {
          swVdebug(1,"S1490: [错误/共享内存] 查找处理流水出错,[tranid=%ld,step=*]",llTranid);
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
            swVdebug(1,"S1500: [错误/数据库] 追加历史处理流水失败,[tranid=%ld,step=%d],sqlcode=%d",llTranid,sgSwt_proc_log.proc_step,sqlca.sqlcode);
            swDberror(NULL);
            EXEC SQL ROLLBACK WORK;
            break;
          }
          swVdebug(2,"S1510: 追加历史处理流水成功,[tranid=%ld,step=%d]",llTranid,sgSwt_proc_log.proc_step);
#else
          /*modify by zcd 20141229
      sw_init_logfile();
	  ilRc= sw_put_proclog(&(pslSwt_proc_log[i]));
	  if(ilRc == FAIL)
	  {
	  	swVdebug(0,"S1505: [错误/文件系统] 交易移除到tranlog日志中报错,[tranid=%ld]",llTranid);
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
          swVdebug(1,"S1520: [错误/共享内存] 删除处理流水失败,[tranid=%ld]",llTranid);
          #ifdef DB_SUPPORT  /*support database 2014-12*/
          EXEC SQL ROLLBACK WORK;
          #endif
          return (FAIL);
        }
        swVdebug(2,"S1530: 删除处理流水成功,[tranid=%ld]",llTranid);

 /*need review again*/
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        /* 删除报文头存储表中的此交易流水号报文记录 */
        EXEC SQL DELETE FROM swt_msghead WHERE tran_id = :llTranid;
        if(sqlca.sqlcode && (sqlca.sqlcode != SQLNOTFOUND))
        {
          swVdebug(1,"S1540: [错误/数据库] DELETE FROM swt_msghead,sqlcode=%d",sqlca.sqlcode);
          swDberror((char *)&slMsgpack);
          EXEC SQL ROLLBACK WORK;
          continue;
        } 
        swVdebug(2,"S1550: 删除swt_msghead中该笔交易流水记录");
        EXEC SQL COMMIT WORK;
#endif

#endif
      }
      continue;
    }

    /* 冲正报文 */
    if(slMsgpack.sMsghead.iMsgtype == iMSGREV)
    {
      swVdebug(2,"S1560: 收到冲正报文");
      if(ilQid == iMBMONREV)
      {
        swVdebug(2,"S1570: 从MONREV发来,转发至前置进程...");
         
        /* 发送报文 */ 
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,ilQdes,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(1,"S1580: [错误/邮箱] qwrite2()函数,写邮箱[%d]出错,错误码=%d",ilQdes,ilRc);
          swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S1590: 写邮箱[%d]成功",ilQdes);
      }
      else
      {
        swVdebug(2,"S1600: 从前置进程发来,转发给MONREV...");
         
        /* 发送报文 */
        ilRc = qwrite2((char *)&slMsgpack,ilMsglen,iMBMONREV,
          ilPriority,ilClass,ilType);
        if(ilRc)
        {
          swVdebug(1,"S1610: [错误/邮箱] 写邮箱[iMBMONREV]出错,错误码=%d",ilRc);
           swMberror(ilRc,(char *)&slMsgpack);
          swQuit(FAIL);
        }
        swVdebug(2,"S1620: 写邮箱[iMBMONREV]成功");
       }
      continue;
    }
    
    switch(slMsgpack.sMsghead.iMsgtype)
    {
      case iMSGORDER:
        swVdebug(2,"S1630: 收到命令报文,[code=%d]",slMsgpack.sMsghead.lCode);
       case iMSGAPP:
      case iMSGUNPACK:
      case iMSGROUTE:
      case iMSGPACK:
      case iMSGREV:
        swVdebug(2,"S1640: 报文接收成功!************************结束********"); 
         continue;
      default:
        swVdebug(2,"S1650: 收到未知报文");
        _swMsgsend(301001,(char *)&slMsgpack);
        continue;
    }
  }
}

/**************************************************************
 ** 函数名      : swQuit
 ** 功  能      : 退出模块处理
 ** 作  者      : 史正烨
 ** 建立日期    : 1999/11/25
 ** 最后修改日期: 2001/3/23
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : sig       :信号
 ** 返回值      : void     
***************************************************************/
void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  
  swVdebug(2,"S1660: 主控模块已经停止...");

#ifdef DB_SUPPORT
 
  /* 关闭数据库连接 */
  swDbclose();

#endif

  /* 关闭邮箱连接 */
  ilRc = qdetach();
  if(ilRc < 0)
  {
    swVdebug(0,"S1670: [错误/邮箱] 关闭邮箱连接错误,错误码=[%d]",ilRc);
     exit(FAIL);
  }

  exit(sig);
}
