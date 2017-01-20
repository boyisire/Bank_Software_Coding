/****************************************************************/
/* 模块编号    ：MONSAF                                         */ 
/* 模块名称    ：SAF处理                                        */
/* 版 本 号    ：V4.3.0                                         */
/* 作	 者    ：张辉                                           */
/* 建立日期    ：1999/11/22                                     */
/* 最后修改日期：2001/3/24                                      */
/* 模块用途    ：找到SAF超时的交易，进行SAF处理                 */
/* 本模块中包含如下函数及功能说明：                             */
/*		  (1) void main();	                        */
/*		  (2) void swQuit();  		                */
/*                (3) int swProcess_revend_tranlog(long,short); */
/*                (4) int swProcess_revend_proclog(long,short); */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/*   1999/12/15 增加了对应用报文SAF的处理                       */
/*   1999/12/27 修改了对同一tranid多个saf记录的处理             */
/*   2000/1/28  根据中信二期的设计进行了修改                    */
/*   2000/3/17  SAF只需要取出SAF报文,原封不动的发出             */
/*   2000/4/4   优化                                            */
/*   2001/3/15  增加稳定性                                      */
/*   2001/3/24  封版V3.01,程序共1178行                          */
/*   2001/4/27  修改死循环的BUG(1217)                           */
/****************************************************************/

/* switch定义 */
#include "switch.h"


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

/* POSTGRES数据库支持(结构必须重新定义) */
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
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
/*除了64位db2以外的其它数据库*/
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
/* 变量定义 */
short  i,j,ilRc,ilCount;
struct swt_proc_log pslSwt_proc_log[iMAXPROCNUM];

/* 函数原型 */
void swQuit(int);
int swProcess_revend_tranlog(long,short,char);
int swProcess_revend_proclog(long,short,char);
int swProcess_revfail_revsaf(long);

extern void  swLogInit(char*,char*);
extern int swShmselect_overtime_min_revsaf(long lTime, struct swt_rev_saf *psSwt_rev_saf);
extern int swShmupdate_swt_rev_saf(long lSaf_id, struct swt_rev_saf sSwt_rev_saf);
extern short swShmselect_route_d_last(short iQid, char *aTran_code);

/**************************************************************
 ** 函数名      ： main
 ** 功  能      ： 主函数
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/11/22
 ** 最后修改日期： 2000/4/5
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 无
***************************************************************/
int main(int argc,char *argv[])
{
  /* 变量定义 */
  long  llCurrenttime;				/* 系统当前时间 */
  short ilWaitTime;				/* 轮循等待时间 */
  long  llSaf_id;				/* SAF记录号 */
  long  llTran_id;				/* 交易流水号 */
  short ilQ_id;					/* 目标信箱号 */
  short  ilPrior = 0,ilClass = 0,ilType = 0;	/* 邮箱参数 */
  struct msgpack slMsgpack;			/* 报文 */

  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swMonsaf Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swMonsaf")) exit(FAIL);

  /* 设置调试程序名称 */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swMonsaf.debug");

  /* 取得DEBUG标志 */
  if((cgDebug=_swDebugflag("swMonsaf"))==FAIL)
  {
    fprintf(stderr,"取DEBUG标志失败!\n");
    exit(FAIL);
  } 
  swLogInit("swMonsaf", "swMonsaf.debug");
  swVdebug(2,"S0010: 版本号[4.3.0]");
  swVdebug(2,"S0020: DEBUG文件[%s]",agDebugfile);
  swVdebug(2,"S0030: DEBUG标志[%d]",cgDebug);
 
  /* 打开邮箱 */
  if(qattach(iMBMONREV)) 
  {
    swVdebug(0,"S0040: [错误/邮箱] qattach()函数,错误码=-1,打开邮箱失败");
    exit(FAIL);
  }
  swVdebug(2,"S0050: 打开邮箱成功");
 
  /* 打开数据库 */
  if(swDbopen()==FAIL)
  {
    swVdebug(0,"S0060: [错误/数据库] 打开数据库失败");
    swQuit(FAIL);
  } 
  swVdebug(2,"S0070: 打开数据库成功");
 
  /* 信号屏蔽和设置 */
  signal(SIGTERM,swQuit);
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);

  /* 初始化共享内存指针 */
  if(swShmcheck() == FAIL)
  {
    swVdebug(0,"S0080: [错误/共享内存] 初始化共享内存指针失败");
    swQuit(FAIL);
  }
  swVdebug(2,"S0090: 初始化共享内存指针成功");
 
  /* 发送系统监控信息报文 */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: 发送104001信息报文,[swMonsaf]已经启动...");
 
  while(TRUE) 
  {
    /* 刷新共享内存状态 */
    if(swShmcheck() == FAIL) 
    {
      swVdebug(0,"S0110: [错误/共享内存] 刷新共享内存指针失败");
      swQuit(FAIL);
    }
  
    /* 取得系统轮循时间 */
    ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
    if(ilRc)
    {
      swVdebug(0,"S0120: [错误/共享内存] 取系统轮循时间失败");
      swQuit(FAIL);
    }
    ilWaitTime = sgSwt_sys_config.saf_inter;

    /* 查找超时的最小SAF记录记录 */
    time(&llCurrenttime);
	
/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/
    /* ilRc = swDbselect_overtime_min_revsaf(llCurrenttime,&sgSwt_rev_saf); */
	/* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	ilRc = swShmselect_overtime_min_revsaf(llCurrenttime,&sgSwt_rev_saf);
    if(ilRc == FAIL) 
    {
      swVdebug(0,"S0130: [错误/共享内存] 查找超时的最小SAF记录记录出错");
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
    swVdebug(2,"S0140: 找到超时的最小SAF记录记录,[tranid=%ld,safid=%ld]",llTran_id,llSaf_id);
 
    /* 取得SAF报文 */
    _swTrim(sgSwt_rev_saf.saf_file);
    memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
    ilRc = swGetlog(sgSwt_rev_saf.saf_file,sgSwt_rev_saf.saf_len,sgSwt_rev_saf.saf_offset,(char *)&slMsgpack);
    if(ilRc)
    {
      swVdebug(0,"S0150: [错误/其它] 取SAF报文失败");
      _swMsgsend(304010,NULL);
      goto LSAFFAIL;
    }
    swVdebug(2,"S0160: 取SAF报文成功");
 
    ilQ_id = slMsgpack.sMsghead.iDes_q;

    if(sgSwt_rev_saf.saf_flag[0] != '1')
      swVdebug(2,"S0170: 为冲正SAF报文");
    else
      swVdebug(2,"S0180: 为应用SAF报文");

    /* 计算新的SAF超时时间 */
    time(&llCurrenttime);
    sgSwt_rev_saf.saf_overtime = llCurrenttime + sgSwt_rev_saf.rev_overtime;
 
    /* 一直冲CASE */
    if(sgSwt_rev_saf.saf_num == -1)
    {
      swVdebug(2,"S0190: 重发次数[-1],一直冲"); 

/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/            
      /* ilRc = swDbupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf); */
      /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0200: [错误/共享内存] 更新SAF记录失败,[safid=%ld]",llSaf_id);
 	continue;
      }
/* #endif */ /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0210: SAF记录没有找到,[safid=%ld]",llSaf_id);
 	continue;
      }
      if(cgDebug>=2)
      {
        swDebugmsghead((char *)&slMsgpack);
        swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      }
      /* 发送冲正报文到前置进程 */
      ilRc = qwrite2((char *)&slMsgpack,sgSwt_rev_saf.saf_len,ilQ_id,ilPrior,ilClass,ilType);
      if(ilRc)
      {
        swVdebug(0,"S0220: [错误/邮箱] qwrite2()函数,错误码=%d, 发送冲正报文到前置进程失败,[qid=%d]",ilRc,ilQ_id);
        swMberror(ilRc,(char *)&slMsgpack);
        _swMsgsend(304001,(char *)&slMsgpack); 
      }
      swVdebug(2,"S0230: 发送冲正报文到前置进程成功,[qid=%d]",ilQ_id);
      _swMsgsend(199003,(char *)&slMsgpack);
      continue;
    }

    if(sgSwt_rev_saf.saf_num > 0)
    {
      swVdebug(2,"S0240: 重发次数[%d],继续冲",sgSwt_rev_saf.saf_num);
      
      /* 计算新的SAF重发次数 */
      sgSwt_rev_saf.saf_num = sgSwt_rev_saf.saf_num - 1;

/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */ 
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/     
      /* 更新SAF记录 */
      /* ilRc = swDbupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf); */
	  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmupdate_swt_rev_saf(llSaf_id,sgSwt_rev_saf);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0250: [错误/共享内存] 更新SAF记录失败,[safid=%ld]",llSaf_id);
        continue;
      }  
/* #endif */ /* del by gengling at 2015.04.03 one line PSBC_V1.0 */

      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0260: SAF记录没有找到,[safid=%ld]",llSaf_id);
        continue;
      }  
      swVdebug(2,"S0270: 更新SAF记录成功,[safid=%d]",llSaf_id);
      if(cgDebug>=2)
      {
        swDebugmsghead((char *)&slMsgpack);
        swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      }
      /* 发送报文到前置进程 */
      ilRc = qwrite2((char *)&slMsgpack,sgSwt_rev_saf.saf_len,ilQ_id,ilPrior,ilClass,ilType);
      if(ilRc)
      {
        swVdebug(0,"S0280: [错误/邮箱] qwrite2()函数,错误码=%d, 发送冲正报文到前置进程失败,[qid=%d]",ilRc,ilQ_id);
        swMberror(ilRc,(char *)&slMsgpack);
        _swMsgsend(304001,(char *)&slMsgpack); 
      }
      swVdebug(2,"S0290: 发送冲正报文到前置进程成功,[qid=%d]",ilQ_id);
      _swMsgsend(199003,(char *)&slMsgpack);
      continue;
    }

    swVdebug(2,"S0300: 重发次数[0],冲正失败");

LSAFFAIL:
    /* SAF失败处理 */
    ilRc = swProcess_revfail_revsaf(llSaf_id);
    if(ilRc == FAIL) continue;
 
    /* 如果是冲正saf,交易流水和处理流水移历史 */
    if(sgSwt_rev_saf.saf_flag[0] != '1')
    {
      /* 置该笔交易流水状态为冲正失败,移历史,删除之 */
      ilRc = swProcess_revend_tranlog(llTran_id,iTRNREVFAIL,'a');
      /*delete by gf for repeated rev respond to original port at 2004-04-07
        ilRc=swSendRevresult(slMsgpack,iMSGREVFAIL);
      */
      if(ilRc == FAIL) continue;
      
      /* 置该笔交易的所有处理流水状态为冲正失败,移历史,删除之 */
      ilRc = swProcess_revend_proclog(llTran_id,iTRNREVFAIL,'a');
      if(ilRc == FAIL) continue;
    }
  } 
}

/**************************************************************
 ** 函数名      ： swProcess_revend_tranlog
 ** 功  能      ： 置某笔交易流水状态为冲正结束或冲正失败,
 **             :  移历史,删除之 
 ** 作  者      ：  
 ** 建立日期    ： 1999/11/19
 ** 最后修改日期： 1999/11/19
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-成功 / 1-失败
***************************************************************/
int swProcess_revend_tranlog(long lTran_id,short iStatus,char aLoc_flag)
{
  swVdebug(4,"S0310: [函数调用] swProcess_revend_tranlog(%ld,%d)",lTran_id,iStatus);

  /* 更新交易流水状态 */
  ilRc = swShmupdate_setstat_tranlog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0320: [错误/共享内存] 更新交易流水出错,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0330: 交易流水没有找到,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S0340: 置交易流水状态,更新交易流水成功,[tranid=%ld]",lTran_id);
  
  
  /* 交易流水移到历史 */
  ilRc = swShmselect_swt_tran_log(lTran_id,&sgSwt_tran_log,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0350: [错误/共享内存] 查找交易流水出错,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0360: 交易流水没有找到,[tranid=%ld]",lTran_id);
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
    swVdebug(0,"S0370: [错误/数据库] 追加历史交易流水失败,[tranid=%ld]",lTran_id);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S0380: 追加历史交易流水成功,[tranid=%ld]",lTran_id);
#endif

  /* 删除交易流水 */
  ilRc = swShmdelete_swt_tran_log(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0390: [错误/共享内存] 删除交易流水出错,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0400: 交易流水没有找到,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S0410: 删除交易流水成功,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S0420: [函数返回] swProcess_revend_tranlog()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ： swProcess_revend_proclog
 ** 功  能      ： 置某笔交易的所有处理流水状态为冲正结束或
 **             :  冲正失败,移历史,删除之
 ** 作  者      ：  
 ** 建立日期    ： 1999/11/19
 ** 最后修改日期： 1999/11/19
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-成功 / 1-失败
***************************************************************/
int swProcess_revend_proclog(long lTran_id,short iStatus,char aLoc_flag)
{
  swVdebug(4,"S0430: [函数调用] swProcess_revend_proclog(%ld,%d)",lTran_id,iStatus);

  /* 更新处理流水状态 */
  ilRc = swShmupdate_setstat_proclog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0440: [错误/共享内存] 更新处理流水出错,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0450: 处理流水没有找到,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S0460: 置处理流水状态,更新处理流水成功,[tranid=%ld]",lTran_id);
  
  /* 处理流水移到历史 */
  ilRc = swShmselect_swt_proc_log_mrec(lTran_id,pslSwt_proc_log,&ilCount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0470: [错误/共享内存] 查找处理流水出错,[tranid=%ld]",lTran_id);
    return (FAIL);
  }  
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0480: 处理流水没有找到,[tranid=%ld]",lTran_id);
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
      swVdebug(0,"S0490: [错误/数据库] 追加历史处理流水失败,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return (FAIL);
    }
#endif
    swVdebug(2,"S0500: 追加历史处理流水成功,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
  }  
 
  /* 删除处理流水 */
  ilRc = swShmdelete_swt_proc_log_mrec(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0510: [错误/共享内存] 删除处理流水出错,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0520: 处理流水没有找到,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S0530: 删除处理流水成功,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S0540: [函数返回] swProcess_revend_proclog()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ： swProcess_revfail_revsaf
 ** 功  能      ： 置某笔SAF记录状态为SAF失败,
 **             :  移数据库表,删除之 
 ** 作  者      ：  
 ** 建立日期    ： 1999/11/19
 ** 最后修改日期： 1999/11/19
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-成功 / 1-失败
***************************************************************/
int swProcess_revfail_revsaf(long lSaf_id)
{
  swVdebug(4,"S0550: [函数调用] swProcess_revfail_revsaf(%ld)",lSaf_id);
  /* 更新SAF记录状态 */
  sgSwt_rev_saf.saf_status[0] = '2';	/* SAF失败 */
/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */  /*support database,Database Deprivation Project 2014-12*/

  /* 更新SAF记录 */
  /* ilRc = swDbupdate_swt_rev_saf(lSaf_id,sgSwt_rev_saf); */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  ilRc = swShmupdate_swt_rev_saf(lSaf_id,sgSwt_rev_saf);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0560: [错误/共享内存] 更新SAF记录出错,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
/* #endif */ /* del by gengling at 2015.04.03 one line PSBC_V1.0 */

  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S0570: SAF记录没有找到,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
  swVdebug(2,"S0580: 置SAF记录状态为SAF失败,[safid=%ld]",lSaf_id);
  swVdebug(2,"S0590: 更新SAF记录成功,[safid=%ld]",lSaf_id);
  swVdebug(4,"S0600: [函数返回] swProcess_revfail_revsaf()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ： swQuit
 ** 功  能      ： 信号处理函数
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/11/19
 ** 最后修改日期： 1999/11/19
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 无
***************************************************************/
void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  
  swVdebug(2,"S0610: [swMonsaf]已经停止...");
  swDbclose();
  ilRc=qdetach();
  if(ilRc)
  {
    swVdebug(0,"S0620: [错误/邮箱] qdetach()函数,错误码=%d",ilRc);
    exit(FAIL);
  }
  exit(sig);
}

/*begin add by gf at 2004-04-07*/
/**************************************************************
 ** 函数名      ： swSendRevresult
 ** 功  能      ： 发送冲正结果给源发端口
 **             :  
 ** 作  者      ：  
 ** 建立日期    ： 1999/11/19
 ** 最后修改日期： 1999/11/19
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-成功 / 1-失败
***************************************************************/
int swSendRevresult(struct msgpack sMsgpack,short iMsgtype)
{
  short ilRc;
  
  /*查找路由组中有无语句
   *9999 WITH REVMODE 4
   */
  ilRc=swShmselect_route_d_last(sMsgpack.sMsghead.iBegin_q_id,sMsgpack.sMsghead.aTrancode);
  if(ilRc)
  {
    swVdebug(2,"S9000: 没有返回冲正结果路由语句");
    return(FAIL);
  }

  sMsgpack.sMsghead.iMsgtype = iMsgtype;
  
  /* 发送冲正结果通知报文 */
  ilRc = qwrite2((char *)&sMsgpack,
     sMsgpack.sMsghead.iBodylen + sizeof(struct msghead), 
        sMsgpack.sMsghead.iBegin_q_id,0,0,0);
  if(ilRc)
  {
    swVdebug(0,"S0720: [错误/邮箱] qwrite2()函数,错误码=%d, 发送通知报文到前置进程失败,[qid=%d]",ilRc, sMsgpack.sMsghead.iBegin_q_id); 
    return(FAIL);
  }
  swVdebug(2,"S0730: 发送通知报文到前置进程成功,[qid=%d]",sMsgpack.sMsghead.iBegin_q_id);
  return(SUCCESS);
}
/*end add*/     
