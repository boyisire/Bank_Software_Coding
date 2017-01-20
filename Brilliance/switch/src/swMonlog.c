/****************************************************************/
/* 模块编号    ：MONLOG                                         */ 
/* 模块名称    ：事务处理监测交易流水                           */
/* 版 本 号    ：V4.3.0                                         */
/* 作	 者    ：张辉                                           */
/* 建立日期    ：1999/11/18                                     */
/* 最后修改日期：2001/3/24                                      */
/* 模块用途    ：找到超时的交易,产生SAF,进行冲正                */
/* 本模块中包含如下函数及功能说明：                             */
/*		  (1) void main();                     		*/
/*                (2) void swQuit();                   		*/
/*                (3) int swProcess_proclog(long,short,char)	        */
/*                (4) int swProcess_revend_tranlog(long,short,char); */
/*                (5) int swProcess_revend_proclog(long,short,char); */
/****************************************************************/
/****************************************************************/
/* 修改记录：                                                   */
/*  2000/1/27 根据中信二期的设计进行修改                        */
/*  2000/3/17 修改revmode,revfmtgroup的处理                     */
/*  2000/8/25 优化                                              */
/*  2001/2/22 Oracle移植                                        */
/*  2001/3/15 健壮性增强                                        */
/*  2001/3/24 封版V3.01,程序共1346行                            */
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
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
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

/* 函数原型定义 */
void swQuit(int);
int swProcess_proclog(long,short,char);
int swProcess_revend_tranlog(long,short,char);
int swProcess_revend_proclog(long,short,char);
/*begin add by gf at 2004-04-07*/
int swSendRevresult_nounion(short);
/*end add*/

/**************************************************************
 ** 函数名      ： main
 ** 功  能      ： 主函数
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/11/18
 ** 最后修改日期： 2000/4/5
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 无
***************************************************************/
int main(int argc,char *argv[])
{
  short ilWaitTime;				/* 系统等待时间 */
  long  llCurrentTime;				/* 系统当前时间 */
  short ilQtran_begin;				/* 交易源发邮箱 */
  
  short ilPrior = 0,ilClass = 0,ilType = 0;	/* 信箱参数 */
  struct msgpack slMsgpack;			/* 报文 */
  short  ilRevstep=0;			/* 冲正模式为3和4的最大交易步骤 */
  
  char  alResult[iFLDVALUELEN + 1];
  short ilLength;
  
  short ilCount_main = 0; /* modify by qy 2001.11.10 */
  short ilClear_count = 0;                          /*清理进程唤醒间隔次数*/
  long llClear_time;                            /*清理进程清理流水的时间标志*/
  long  llProc_begin;
  short ilMoveproc_flag = 0;
  long  llRerev_inter = 0;                /* RESAF间隔时间 */
  long  llRerev_time = 0;
  char  clLoc_flag;
  long llClear_time_prep;		/* add by szhengye 2004.2.19 */

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
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


  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swMonlog Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swMonlog")) exit(FAIL);
  
  /* 设置调试程序名称 */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swMonlog.debug");

  /* 取得DEBUG标志 */
  if((cgDebug = _swDebugflag("swMonlog")) == FAIL)
  {
    fprintf(stderr,"取DEBUG标志失败!\n");
    exit(FAIL);
  } 
  swLogInit("swMonlog", "swMonlog.debug");
  swVdebug(2,"S0010: 版本号[4.3.0]");
  swVdebug(2,"S0020: DEBUG文件[%s]",agDebugfile);
  swVdebug(2,"S0030: DEBUG标志[%d]",cgDebug);
 
  /* 打开邮箱 */
  ilRc = qattach(iMBMONREV);
  if(ilRc) 
  {
    swVdebug(0,"S0040: [错误/邮箱] qattach()函数,错误码=%d",ilRc);
    exit(FAIL);
  }
  swVdebug(2,"S0050: 打开邮箱成功");

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  /* 打开数据库 */
  if(swDbopen())
  {
    swVdebug(0,"S0060: [错误/数据库] 打开数据库失败");
    swQuit(FAIL);
  } 
  swVdebug(2,"S0070: 打开数据库成功");
#endif
 
  /* 信号屏蔽和设置 */
  signal(SIGTERM,swQuit);
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  
  /* 初始化共享内存指针 */
  if(swShmcheck()) 
  {
    swVdebug(0,"S0080: [错误/共享内存] 初始化共享内存指针失败");
    swQuit(FAIL);
  }
  swVdebug(2,"S0090: 初始化共享内存指针成功");
 
  /* 向系统监控发送信息报文 */
  swVdebug(2,"S0100: 发送104001信息报文,[swMonlog]已经启动...");
  _swMsgsend(104001,NULL);
 
  time(&llClear_time_prep);		/* add by szhengye 2004.2.19 */
  while (TRUE) 
  {
    /* 刷新共享内存指针 */
    if(swShmcheck()) 
    {
      swVdebug(0,"S0110: [错误/共享内存] 刷新共享内存指针失败");
      swQuit(FAIL);
    }
    /* 读取系统轮循时间 */
    ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
    if(ilRc)
    {
      swVdebug(0,"S0120: [错误/共享内存] 读取系统轮循时间失败,返回码=%d",ilRc);
      swQuit(FAIL);
    }
    ilWaitTime = sgSwt_sys_config.log_inter;
    ilClear_count = sgSwt_sys_config.log_clear_count;
    llRerev_inter = sgSwt_sys_config.rerev_inter;
    
    if(ilClear_count <= 0) goto NORMALPROCESS;
 
    /* 共享内存清理例程 */
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
      swVdebug(2,"S0125: 开始清理过期的交易流水,当前时间[%ld],清理[%ld]以前的交易流水!",llCurrentTime,llTran_begin);

      /* 清理交易流水 */
      while(TRUE)
      {
        /* 查找过期的交易流水 */
        ilRc = swShmselect_swt_tran_log_clear(llTran_begin, &sgSwt_tran_log,&clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0130: [错误/共享内存] 查找交易流水出错,[tran_bgein<=%ld]",llTran_begin);
          break;
        }
        if(ilRc == SHMNOTFOUND)
          break;

        swVdebug(2,"S0140: 找到过期的交易流水,[tran_id=%ld],交易状态[%d]",sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status);
        llTran_id = sgSwt_tran_log.tran_id;

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
        EXEC SQL BEGIN WORK;
#endif
        /*移交易流水至历史*/ 
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
          swVdebug(0,"S0150: [错误/数据库] 追加历史交易流水失败,[tranid=%ld]",llTran_id);
          swDberror(NULL);
        }
        swVdebug(2,"S0160: 追加历史交易流水成功,[tranid=%ld]",llTran_id);
#else
		sw_put_tranlog(&sgSwt_tran_log);
		/*todo*/
#endif

        /* 删除交易流水 */
        ilRc = swShmdelete_swt_tran_log(llTran_id,clLoc_flag);
        if(ilRc == FAIL)
          swVdebug(0,"S0170: [错误/共享内存] 删除交易流水出错,[tranid=%ld]",llTran_id);
        else if(ilRc == SHMNOTFOUND)
          swVdebug(2,"S0180: 交易流水没有找到,[tranid=%ld]",llTran_id);
        else
          swVdebug(2,"S0190: 删除交易流水成功,[tranid=%ld]",llTran_id);

        swVdebug(2,"S0195: 开始清理该交易流水[tranid=%ld]对应的处理流水!",llTran_id);
        /* 移处理流水至历史 */
        ilRc = swShmselect_swt_proc_log_mrec(llTran_id,pslSwt_proc_log,&ilCount_main);
        if(ilRc == FAIL)
          swVdebug(0,"S0200: [错误/共享内存] 查找处理流水出错,[tranid=%ld,step=*]",llTran_id);
        else if(ilRc == SHMNOTFOUND)
          swVdebug(2,"S0210: 没有找到处理流水,[tranid=%ld]",llTran_id);
        else
        {
          swVdebug(2,"S0220: 找到处理流水,[tranid=%ld,step=*]",llTran_id);
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
              swVdebug(0,"S0230: [错误/数据库] 追加历史处理流水失败,[tranid=%ld,step=%d]",llTran_id,slSwt_proc_log.proc_step);
              swDberror(NULL);
            }
            swVdebug(2,"S0240: 追加历史处理流水成功,[tranid=%ld,step=%d,proc_status=%d,proc_begin=%ld]",llTran_id,slSwt_proc_log.proc_step,slSwt_proc_log.proc_status,slSwt_proc_log.proc_begin);
#else
		    sw_put_proclog(&pslSwt_proc_log[i]);
#endif
          }

          /* 删除处理流水 */
          ilRc = swShmdelete_swt_proc_log_mrec(llTran_id,clLoc_flag);
          if(ilRc == FAIL)
            swVdebug(0,"S0250: [错误/共享内存] 删除处理流水出错,[tranid=%ld,step=*]",llTran_id);
          else if(ilRc == SHMNOTFOUND)
            swVdebug(2,"S0260: 处理流水没有找到,[tranid=%ld,step=*]",llTran_id);
          else
            swVdebug(2,"S0270: 删除处理流水成功,[tranid=%ld,step=*]",llTran_id);
        }
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        EXEC SQL COMMIT WORK;
#endif
      }     
      swVdebug(2,"S0271: 过期的交易流水清理完毕!");     

      /*清理处理流水*/
      swVdebug(2,"S0275: 开始清理过期的处理流水,当前时间[%ld],清理[%ld]以前的处理流水!",llCurrentTime,llProc_begin);      
      while(TRUE)
      { 
      	/* 查找过期的处理流水 */
        ilRc = swShmselect_swt_proc_log_clear(llProc_begin, &sgSwt_proc_log,&clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0280: [错误/共享内存] 查找过期的处理流水出错,[proc_begin<%ld]",llProc_begin);
          break;
        }
        if(ilRc == SHMNOTFOUND)
          break;
          
        llTran_id = sgSwt_proc_log.tran_id;
        ilProc_step = sgSwt_proc_log.proc_step;
        swVdebug(2,"S0290: 找到过期的处理流水,[tranid=%ld,step=%d,proc_status=%d,proc_begin=%ld]",llTran_id,ilProc_step,sgSwt_proc_log.proc_status,sgSwt_proc_log.proc_begin);

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
        EXEC SQL BEGIN WORK;
#endif
        /* 处理流水移历史 */
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
          swVdebug(0,"S0300: [错误/数据库] 追加历史处理流水失败,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          swDberror(NULL);
        }
#else
		sw_put_proclog(sgSwt_proc_log);
#endif
        /* 删除处理流水 */
        ilRc = swShmdelete_swt_proc_log(llTran_id, ilProc_step,clLoc_flag);
        if(ilRc == FAIL)
          swVdebug(0,"S0310: [错误/共享内存] 删除处理流水出错, [tranid=%ld, procstep=d%]",llTran_id, ilProc_step);
        else if(ilRc == SHMNOTFOUND)
          swVdebug(2,"S0320: [错误/共享内存] 要删除的处理流水没有找到, [tranid=%ld, procstep=d%]",llTran_id, ilProc_step);
        else
          swVdebug(2,"S0330: 删除处理流水成功, [tranid=%ld, step=d%]",llTran_id, ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        EXEC SQL COMMIT WORK;
#endif
      }
      swVdebug(2,"S0335: 过期的处理流水清理完毕!"); 
      /* del by nh 20040219 ilClear_count_cal = 0;*/
    }
#endif
NORMALPROCESS:

    /* 取得当前系统时间 */
    time(&llCurrentTime);

    /* 查找超时的最小交易流水记录 */
    /* 查找超时的最小交易流水记录,并更新为正在冲正 */
    /* modify by cy 20040324 
    ilRc = swShmselect_overtime_min_tranlog(llCurrentTime,&sgSwt_tran_log,&clLoc_flag);*/
    ilRc = swShmselect_overtime_min_tranlog_update(llCurrentTime,&sgSwt_tran_log,&clLoc_flag);
    if(ilRc == FAIL) 
    {
      swVdebug(0,"S0340: [错误/共享内存] 查找超时的最小交易流水记录出错");
      swQuit(FAIL);
    }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    if(ilRc == SHMNOTFOUND) 
    {
      /* 是否进行RESAF处理 */
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

      /* 从历史交易流水表中查找最小的RESAF交易流水记录 */
      /* del by gengling 20150114 */
#if 0
      EXEC SQL SELECT MIN(saf_id) INTO :llSaf_id:ilTmpind
           FROM swt_rev_saf WHERE saf_flag = '2'
                            AND   saf_overtime <= :llRerev_time_cal;
      if (sqlca.sqlcode)
      {
        swVdebug(0,"S0350: [错误/数据库] SELECT MIN(saf_id) FROM swt_rev_saf ...error");
        swDberror(NULL);
        continue;
      }
#endif
      /* end of del */

      /* add by gengling 20150114 */
      ilRc = swShmselect_saf_id_min_revsaf(llRerev_time_cal, &sgSwt_rev_saf, &clLoc_flag);
      if(ilRc == FAIL) 
      {
        swVdebug(0,"S0350: [错误/共享内存] SELECT MIN(saf_id) FROM swt_rev_saf ...error");
        swDberror(NULL);
        continue;
      }
      ilTmpind = sgSwt_rev_saf.saf_id;
      llSaf_id = sgSwt_rev_saf.saf_id;
      swVdebug(2,"S0: 查找冲正saf超时的saf_id最小记录,[MIN(saf_id)=%ld]", ilTmpind); /* will */
      /* end of add */
  
      if(ilTmpind < 0)   /* 没找到 */
      {
        /* del by gengling 20150114 */
#if 0
        EXEC SQL SELECT MIN(saf_overtime) INTO :llRerev_time_cal:ilTmpind
             FROM swt_rev_saf WHERE saf_flag = '2';
        if (sqlca.sqlcode)
        {
          swVdebug(0,"S0360: [错误/数据库] SELECT MIN(saf_overtime) FROM swt_rev_saf ...error");
          swDberror(NULL);
          continue;
        }
#endif
        /* end of del */

        /*add by gengling 20150114*/
        ilRc = swShmselect_saf_overtime_min_revsaf(&sgSwt_rev_saf, &clLoc_flag);
        if(ilRc == FAIL) 
        {
          swVdebug(0,"S0360: [错误/共享内存] MIN(saf_overtime) FROM swt_rev_saf ...error");
          swDberror(NULL);
          continue;
        }
        ilTmpind = sgSwt_rev_saf.saf_overtime;
        llRerev_time_cal = sgSwt_rev_saf.saf_overtime;
        swVdebug(2,"S0: 查找冲正saf saf_overtime字段最小的记录,[MIN(saf_overtime)=%ld %ld]", ilTmpind, llSaf_id); /* will */
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

      swVdebug(2,"S0370: 在历史SAF表中找到需要REREV的冲正失败记录,[safid=%ld]",llSaf_id);
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
        swVdebug(2,"S0380: 历史SAF记录没有找到,[safid=%ld]",llSaf_id);
        continue;
      }
      if(sqlca.sqlcode)
      {
        swVdebug(0,"S0390: [错误/数据库] 查找历史SAF记录出错,[safid=%ld]",llSaf_id);
        swDberror(NULL);
        continue;
      }
#endif
      /* end of del */

      /* add by gengling 20150114 */
      ilRc = swShmselect_swt_rev_saf(llSaf_id, &sgSwt_rev_saf);
      if(ilRc == SHMNOTFOUND)
      {
        swVdebug(2,"S0380: 历史SAF记录没有找到,[safid=%ld]",llSaf_id);
        continue;
      }
      if(ilRc == FAIL) 
      {
        swVdebug(0,"S0390: [错误/共享内存] 查找历史SAF记录出错,[safid=%ld]",llSaf_id);
        swDberror(NULL);
        continue;
      }
      /* end of add */

      llTran_id = sgSwt_rev_saf.tran_id;
      swVdebug(2,"S0400: 找到历史SAF记录,[safid=%ld],[tranid=%ld]",llSaf_id,llTran_id);

      /* 取得SAF报文 */
      memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
      _swTrim(sgSwt_rev_saf.saf_file);
      ilRc = swGetlog(sgSwt_rev_saf.saf_file,sgSwt_rev_saf.saf_len,sgSwt_rev_saf.saf_offset,(char *)&slMsgpack);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0410: [错误/其它] 取处理流水原始报文失败");
        _swMsgsend(304011,NULL);
        continue;
      }
      swVdebug(2,"S0420: 取得SAF流水原始报文");

      /* 该笔交易流水从历史表移到当前表 */
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
        swVdebug(2,"S0430: 历史表中无此交易流水,[tranid=%ld]",llTran_id);
        continue;
      }
      else if(sqlca.sqlcode)
      {
        swVdebug(0,"S0440: [错误/数据库] 查找历史交易流水出错,[tranid=%ld]",llTran_id);
        swDberror((char *)&slMsgpack);
        continue;
      }
      swVdebug(2,"S0450: 找到历史交易流水,[tranid=%ld]",llTran_id);
      if(sgSwt_tran_log.tran_status != iTRNREVFAIL)
      {
        /* del by gengling 20150114 */
      #if 0
        EXEC SQL DELETE FROM swt_rev_saf WHERE saf_id = :llSaf_id;
        if(sqlca.sqlcode)
        {
          swVdebug(0,"S0460: [错误/数据库] 删除历史SAF记录出错,[safid=%ld]",llSaf_id);
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
          swVdebug(0,"S0460: [错误/数据库] 删除历史SAF记录出错,[safid=%ld]",llSaf_id);
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
        swVdebug(0,"S0470: [错误/共享内存] 追加交易流水出错,[tranid=%ld]",llTran_id);
        continue;
      }
      swVdebug(2,"S0480: 追加交易流水成功,[tranid=%ld]",llTran_id);

#ifdef DB_INFORMIX
      EXEC SQL BEGIN WORK;
#endif
      EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTran_id;
      if(sqlca.sqlcode)
      {
        EXEC SQL ROLLBACK WORK;
        swVdebug(0,"S0490: [错误/数据库] 删除历史交易流水失败,[tranid=%ld]",llTran_id);
        swDberror(NULL);
        continue;
      }
      swVdebug(2,"S0500: 删除历史交易流水成功,[tranid=%ld]",llTran_id);

      /* 相应的处理流水从历史表移到当前表 */
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
        swVdebug(2,"没有找到历史处理流水,[tranid=%ld]",llTran_id);
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
          swVdebug(0,"S0530: [错误/共享内存] 追加处理流水出错,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          ilMoveproc_flag = -1;
          break;
        }
        swVdebug(2,"S0540: 追加处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

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
        swVdebug(2,"S0560: 删除历史处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

        ilMoveproc_flag ++;

      }
      swVdebug(2,"S0570: 交易流水和处理流水从历史移到当前完成");
      continue;
    }
/*add by zcd 20150111*/
#else
    if(ilRc == SHMNOTFOUND) 
    {
        swVdebug(2,"S0111:共享内存没有找到需要超时的流水");
        sleep(ilWaitTime);
        continue;
    }
/*end of add */
#endif

    llTran_id = sgSwt_tran_log.tran_id;
    ilQtran_begin = sgSwt_tran_log.q_tran_begin;
    
    swVdebug(2,"S0580: 找到超时的最小交易流水记录,[tranid=%ld]",llTran_id);
 
    /* ECHOTEST发起的查询交易 */
    if(sgSwt_tran_log.q_tran_begin == iMBECHOTEST)
    {
      swVdebug(2,"S0590: ECHOTEST发起的查询交易");
      
      /* 取目的端口设置 */
      ilRc = swShmselect_swt_sys_queue(sgSwt_tran_log.q_target,&sgSwt_sys_queue);
      if(ilRc)
        swVdebug(0,"S0600: [错误/函数调用] swShmselect_swt_sys_queue出错! 取目的端口设置失败,[qid=%d,ilRc=%d]",sgSwt_tran_log.q_target,ilRc);
      else
      {
        ilRc = _swExpress(sgSwt_sys_queue.setdown_overtime,alResult,&ilLength);
        if((ilRc == SUCCESS) && (alResult[0] == '1'))
        {
          swPortset(sgSwt_tran_log.q_target,4,0); /* 端口ECHOTEST层状态置down */
          swVdebug(2,"S0610: 置端口ECHOTEST层状态DOWN,[qid=%d]",sgSwt_tran_log.q_target);
        }  
      }   
      /* fzj: 2002.9.17: 交易超时向监控发199009报文 */
      memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
      _swTrim(sgSwt_tran_log.resp_file);
      ilRc = swGetlog(sgSwt_tran_log.resp_file,sgSwt_tran_log.resp_len,
                    sgSwt_tran_log.resp_offset,(char *)&slMsgpack);
      if(ilRc == FAIL)
      {
        swVdebug(0,"S0620: [错误/其它] 取交易流水响应报文失败");
        _swMsgsend(304011,NULL);
      }
      else
        _swMsgsend(199009,(char *)&slMsgpack);
      /**/

      /* 置交易流水状态为冲正成功,移历史；*/
      ilRc = swProcess_revend_tranlog(sgSwt_tran_log.tran_id,iTRNREVEND,clLoc_flag);
      /*begin add by gf at 2004-04-07*/
      ilRc=swSendRevresult_nounion(iMSGREVEND);
      /*end add*/
      continue;
    }

    /* 置交易流水状态为正在冲正 */
    /* del by cy 20040324
    sgSwt_tran_log.tran_status = iTRNREVING;
    ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0630: [错误/共享内存] 更新交易流水失败,[tranid=%ld]",llTran_id);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0640: 交易流水没有找到,[tranid=%ld]",llTran_id);
      continue;
    }  
    end del */
    swVdebug(2,"S0650: 置交易流水状态为正在冲正,[tranid=%ld]",llTran_id);
    swVdebug(2,"S0660: 更新交易流水成功,[tranid=%ld]",llTran_id);

    swVdebug(2,"S0670: 开始处理REVMODE=3和4的情况...");
   
    /* 如果遇到REVMODE=3或者4的情况,表示需要发送通知报文,本程序首先将此类 
     * 报文全部发送出去.这种用法在交易超时返回给原发信箱中可以使用.具体的 
　　 * 配置方法是:在路由脚本段的第一行加入以下语句(如下所示):
     *    [27,0001333333] WITH REVMODE 4,REVGOUP 999
     *    10 CALL 28
     *    20 SEND 27 WITH END
     * 当交易正常进行时,主控程序会根据REVMODE 4在处理流水中记录一条,
     * 而当交易超时之后,由本程序此处将该报文根据相应的格式转换组或者
     * 模糊匹配以冲正报文的形式发送给前置程序.前置程序可以对其进行相应
     * 的处理. swMonrev中的即时冲正也是类似的机制.
     */

    ilRevstep=0;
    
    /* 查找REVMODE=3和4的处理流水记录 */
    ilRc = swShmselect_swt_proc_log_mrec(llTran_id,pslSwt_proc_log,&ilCount_main); /* qy */
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0680: [错误/共享内存] 查找REVMODE=3和4的处理流水失败");
      continue;
    }

    /* 循环取出处理流水记录并发送通知报文 */
    j = 0;
    for (i = 0; i < ilCount_main; i ++) /* qy */
    {
      /* 只处理冲正模式为3或4的处理流水 */
      if(pslSwt_proc_log[i].rev_mode != 3 && pslSwt_proc_log[i].rev_mode != 4)
        continue;
      
      j++;  
      ilTran_step = pslSwt_proc_log[i].proc_step; 
      swVdebug(2,"S0690: 找到REVMODE=3或4的处理流水,[tranid=%ld,step=%d]",llTran_id,ilTran_step); 
       
      if(ilRevstep < pslSwt_proc_log[i].proc_step)
        ilRevstep = pslSwt_proc_log[i].proc_step;
    
      /* 取得原始报文 */
      memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
      _swTrim(pslSwt_proc_log[i].org_file);
      ilRc = swGetlog(pslSwt_proc_log[i].org_file,pslSwt_proc_log[i].org_len,
        pslSwt_proc_log[i].org_offset,(char *)&slMsgpack);
      if(ilRc == FAIL) 
      {
        swVdebug(0,"S0700: [错误/其它] 取处理流水原始报文失败");
        _swMsgsend(304011,NULL);
        continue;
      }
      swVdebug(2,"S0710: 取处理流水原始报文成功");
 
      /* 组织通知报文头 */
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
 
      /* 发送通知报文 */
      ilRc = qwrite2((char *)&slMsgpack,
        slMsgpack.sMsghead.iBodylen + sizeof(struct msghead),
        pslSwt_proc_log[i].q_target,ilPrior,ilClass,ilType);
      if(ilRc)
      {
	swVdebug(0,"S0720: [错误/邮箱] qwrite2()函数,错误码=%d, 发送通知报文到前置进程失败,[qid=%d]",ilRc, pslSwt_proc_log[i].q_target); 
 	_swMsgsend(304012,NULL);
        continue;
      }
      swVdebug(2,"S0730: 发送通知报文到前置进程,[qid=%d]",
        pslSwt_proc_log[i].q_target); 
      _swMsgsend(199003,(char *)&slMsgpack);  
 
      /* 调用函数swProcess_swt_proc_log(),
         置该笔处理流水状态为冲正结束,移历史,删除之 */
      ilRc = swProcess_proclog(llTran_id,ilTran_step,slMsgpack.sMsghead.aSysattr[0]);
      if(ilRc == FAIL) break;
    }
    if(ilRc == FAIL) continue;
    swVdebug(2,"S0740: 完成对REVMODE=3和4的处理,共[%d]笔",j); 

    /* 查找REVMODE=1和2的最大处理流水 */
    ilTran_step = 0;
    ilRc = swShmselect_max_proclog_tran(llTran_id,&sgSwt_proc_log);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0750: [错误/共享内存] 查找REVMODE=1和2的最大处理流水出错,[tranid=%ld]",llTran_id);
      continue;
    }
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0760: 没有找到REVMODE=1或2的处理流水,冲正成功,[tranid=%ld]",llTran_id);
      /* 该笔交易流水状态置冲正结束,移历史,删除之 */   
      swProcess_revend_tranlog(llTran_id,iTRNREVEND,clLoc_flag); 
      /*begin add by gf at 2004-04-07*/
      ilRc=swSendRevresult_nounion(iMSGREVEND);
      /*end add*/
      continue;
    }
    
    /* 还有处理流水,进行冲正 */
    ilTran_step = sgSwt_proc_log.proc_step;
    swVdebug(2,"S0770: 找到REVMODE=1或2的最大处理流水,[tranid=%ld,step=%d]",llTran_id,ilTran_step);

    /* 取得原始报文 */
    memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
    _swTrim(sgSwt_proc_log.org_file);
    swVdebug(0,"---------filename[%s]-------------------",sgSwt_proc_log.org_file);
    ilRc = swGetlog(sgSwt_proc_log.org_file,sgSwt_proc_log.org_len,
      sgSwt_proc_log.org_offset,(char *)&slMsgpack);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0780: [错误/其它] 取处理流水原始报文失败");
      _swMsgsend(304011,NULL);
      continue;
    }
    swVdebug(2,"S0790: 取得处理流水原始报文");
 
    /* 取得端口设置 */
    ilRc = swShmselect_swt_sys_queue(sgSwt_proc_log.q_target, &sgSwt_sys_queue);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0800: [错误/共享内存] 取端口设置失败,[qid=%d]",sgSwt_proc_log.q_target);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0810: 端口没有设置,[qid=%d]",sgSwt_proc_log.q_target);
      continue;
    }  
    swVdebug(2,"S0820: 取端口设置成功,[qid=%d]",sgSwt_proc_log.q_target);
     
    /* 不支持善意冲正 */ 
    if(sgSwt_sys_queue.rev_nogood[0] == cTRUE && ilRevstep < sgSwt_proc_log.proc_step)
    {
      swVdebug(2,"S0830: 不支持善意冲正,冲正等待");
 
      /* 置处理流水状态为冲正等待 */
      sgSwt_proc_log.proc_status = iTRNREVWAIT;
      ilRc = swShmupdate_swt_proc_log(llTran_id,ilTran_step,sgSwt_proc_log,clLoc_flag);
      if(ilRc == FAIL)
        swVdebug(0,"S0840: [错误/共享内存] 更新处理流水失败,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      if(ilRc == SHMNOTFOUND)
        swVdebug(2,"S0850: 处理流水没有找到,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      swVdebug(2,"S0860: 更新处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilTran_step);  
      continue;  
    }

    /* 支持善意冲正 */
    /* 置处理流水状态为正在冲正 */
    swVdebug(2,"S0870: 更新处理流水状态为正在冲正,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
    sgSwt_proc_log.proc_status = iTRNREVING;
    ilRc = swShmupdate_swt_proc_log(llTran_id,ilTran_step,sgSwt_proc_log,clLoc_flag);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0880: [错误/共享内存] 更新处理流水失败,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0890: 处理流水没有找到,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
      continue;
    }  
    swVdebug(2,"S0900: 更新处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilTran_step);
     
    /* 取得SAF记录号 */
    memset((char *)&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
    if(swShmgetsafid(&llSaf_id) == FAIL)
    {
      swVdebug(0,"S0910: [错误/共享内存] 取SAF流水号失败");
      continue;
    }
    swVdebug(2,"S0920: 取SAF流水号成功,[safid=%ld]",llSaf_id);
 
    /* 组织SAF报文头 */
    slMsgpack.sMsghead.lTranid = llTran_id;
    slMsgpack.sMsghead.iTranstep = ilTran_step;
    slMsgpack.sMsghead.iMsgtypeorg = slMsgpack.sMsghead.iMsgtype;
    slMsgpack.sMsghead.iMsgtype = iMSGREV;
    slMsgpack.sMsghead.iDes_q = sgSwt_proc_log.q_target;
    slMsgpack.sMsghead.lSafid = llSaf_id;
    slMsgpack.sMsghead.cSafflag = '1';
    slMsgpack.sMsghead.iRevfmtgroup = sgSwt_proc_log.rev_fmtgrp;
    slMsgpack.sMsghead.aSysattr[0] = clLoc_flag;

    /* 保存SAF报文 */
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
      swVdebug(0,"S0930: [错误/其它] 保存SAF报文出错");
      continue;
    }
    swVdebug(2,"S0940: 保存SAF报文成功");
 
    /* 追加SAF记录 */
//#ifdef DB_SUPPORT   /*nead review*/ /* del by gengling 20150115 one line */
    /* del by gengling 20150114 */
    // ilRc = swDbinsert_swt_rev_saf(sgSwt_rev_saf); /*unsolved*/
    /* end of del */
    /* add by gengling 20150114 */
    ilRc = swShminsert_swt_rev_saf(sgSwt_rev_saf);
    /* end of add */
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0950: [错误/数据库] 追加SAF记录出错,[safid=%ld]",llSaf_id);
      continue;
    }
    swVdebug(2,"S0960: 追加SAF记录成功,[safid=%ld]",llSaf_id);
//#endif /* del by gengling 20150115 one line */

    sgSwt_tran_log.q_target = sgSwt_proc_log.q_target;
    ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0970: [错误/共享内存] 更新交易流水失败,[tranid=%ld]",llTran_id);
      continue;
    }  
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(2,"S0980: 交易流水没有找到,[tranid=%ld]",llTran_id);
      continue;
    }  
    swVdebug(2,"S0990: 置交易流水目标邮箱为%d,[tranid=%ld]",sgSwt_tran_log.q_target,llTran_id);
    swVdebug(2,"S1000: 更新交易流水成功,[tranid=%ld]",llTran_id);
 
    /* 发送冲正报文至前置程序 */
    swVdebug(3,"S1010: 发送冲正报文到前置程序开始");
    if(cgDebug>=2)
    {
      swDebugmsghead((char *)&slMsgpack);
      swDebughex(slMsgpack.aMsgbody, slMsgpack.sMsghead.iBodylen);
    }
    ilRc = qwrite2((char *)&slMsgpack,sgSwt_proc_log.org_len,
      sgSwt_proc_log.q_target,ilPrior,ilClass,ilType);
    if(ilRc == FAIL)
    {
      swVdebug(2,"S1020: 发送冲正报文至前置进程失败,[qid=%d]",sgSwt_proc_log.q_target);
      swMberror(ilRc,(char *)&slMsgpack);
      _swMsgsend(304001,(char *)&slMsgpack); 
    }
    swVdebug(2,"S1030: 发送冲正报文至前置进程成功,[qid=%d]",sgSwt_proc_log.q_target);
    _swMsgsend(199003,(char *)&slMsgpack);
  } 
}

/**************************************************************
 ** 函数名      ： swProcess_proclog
 ** 功  能      ： 置某笔处理流水状态为冲正结束,
 **             :  移历史,删除之 
 ** 作  者      ：  
 ** 建立日期    ： 1999/11/19
 ** 最后修改日期： 1999/11/19
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-成功 / 1-失败
***************************************************************/
int swProcess_proclog(long lTran_id,short iProc_step,char cLoc_flag)
{
  swVdebug(4,"S1040: [函数调用] swProcess_proclog(%ld,%d,%c)",
           lTran_id,iProc_step,cLoc_flag);
  swVdebug(2,"S1050: 置此笔处理流水状态为冲正结束,移历史,删除之");
  /* 置处理流水状态为冲正结束 */
  pslSwt_proc_log[i].proc_status = iTRNREVEND;
  ilRc = swShmupdate_swt_proc_log(lTran_id,iProc_step,pslSwt_proc_log[i],cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1060: [错误/共享内存] 更新处理流水出错,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1070: 处理流水没有找到,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
    return (FAIL);
  }
  swVdebug(2,"S1080: 更新处理流水成功,[tranid=%ld,step=%d]",lTran_id,
    iProc_step);

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
  EXEC SQL BEGIN WORK;
#endif
  /* 处理流水移到历史 */
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
    swVdebug(0,"S1090: [错误/数据库] 追加历史处理流水出错,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S1100: 追加历史处理流水成功,[tranid=%ld,step=%d]",lTran_id,
    iProc_step);
#else
  sw_put_proclog(&slSwt_proc_log);
#endif
  /* 删除处理流水 */
  ilRc = swShmdelete_swt_proc_log(lTran_id,iProc_step,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1110: [错误/共享内存] 删除处理流水出错,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
#ifdef DB_SUPPORT
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1120: 处理流水没有找到,[tranid=%ld,step=%d]",lTran_id,
      iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1130: 删除处理流水成功,[tranid=%ld,step=%d]",lTran_id,
    iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif   
  swVdebug(4,"S1140: [函数返回] swProcess_proclog()返回码=0");
  return(SUCCESS);
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
int swProcess_revend_tranlog(long lTran_id,short iStatus,char cLoc_flag)
{
  swVdebug(4,"S1150: [函数调用] swProcess_revend_tranlog(%ld,%d,%c)",
           lTran_id,iStatus,cLoc_flag);
  swVdebug(2,"S1160: 置此笔交易流水状态为冲正结束,移历史,删除之");
  /* 更新交易流水状态 */
  ilRc = swShmupdate_setstat_tranlog(lTran_id,iStatus,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1170: [错误/共享内存] 更新交易流水失败,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1180: 交易流水没有找到,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1190: 更新交易流水成功,[tranid=%ld]",lTran_id);


  /* 交易流水移历史 */
  ilRc = swShmselect_swt_tran_log(lTran_id,&sgSwt_tran_log,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1200: [错误/共享内存] 查找交易流水失败,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1210: 交易流水没有找到,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1220: 查找交易流水成功,[tranid=%ld]",lTran_id);


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
    swVdebug(0,"S1230: [错误/数据库] 追加历史交易流水失败,[tranid=%ld]",lTran_id);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S1240: 追加历史交易流水成功,[tranid=%ld]",lTran_id);
/*add by zcd 20141225*/
#else
  sw_put_tranlog(&sgSwt_tran_log);
#endif
#endif 
/*end of add by zcd 20141225*/
  /* 删除交易流水 */
  ilRc = swShmdelete_swt_tran_log(lTran_id,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1250: [错误/共享内存] 删除交易流水失败,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1260: 交易流水没有找到,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1270: 删除交易流水成功,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/  
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S1280: [函数返回] swProcess_revend_tranlog()返回码=0");
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
int swProcess_revend_proclog(long lTran_id,short iStatus,char cLoc_flag)
{
  swVdebug(4,"S1290: [函数调用] swProcess_revend_proclog(%ld,%d)",lTran_id,iStatus);
  /* 更新处理流水状态 */
  ilRc = swShmupdate_setstat_proclog(lTran_id,iStatus,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1300: [错误/共享内存] 更新处理流水失败,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1310: 交易处理没有找到,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1320: 更新处理流水成功,[tranid=%ld]",lTran_id);
  
  /* 处理流水移到历史 */
  ilRc = swShmselect_swt_proc_log_mrec(lTran_id,pslSwt_proc_log,&ilCount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1330: [错误/共享内存] 查找处理流水失败,[tranid=%ld]",lTran_id);
    return (FAIL);
  }  
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1340: 处理流水没有找到,[tranid=%ld]",lTran_id);
    return (FAIL);
  }  
  swVdebug(2,"S1350: 查找处理流水成功,[tranid=%ld]",lTran_id);

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
      swVdebug(0,"S1360: [错误/数据库] 追加历史处理流水失败,[tranid=%ld]",lTran_id);
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return (FAIL);
    }
#else
	sw_put_proclog(&pslSwt_proc_log[i]);
#endif
    swVdebug(2,"S1370: 追加历史处理流水成功,[tranid=%ld]",lTran_id);
  }  
 
  /* 删除处理流水 */
  ilRc = swShmdelete_swt_proc_log_mrec(lTran_id,cLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1380: [错误/共享内存] 删除处理流水失败,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1390: 处理流水没有找到,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1400: 删除处理流水成功,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/  
  EXEC SQL COMMIT WORK;
#endif

  swVdebug(4,"S1410: [函数返回] swProcess_revend_proclog()返回码=0");
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
  
  /* 关闭数据库连接 */
  swDbclose();

  /* 关闭邮箱连接 */
  ilRc=qdetach();
  if(ilRc<0)
  {
    swVdebug(0,"S1420: [错误/邮箱] qdetach()函数,错误码=%d",ilRc);
    exit(FAIL);
  }
  swVdebug(2,"S1430: 关闭邮箱连接成功");
  swVdebug(2,"S1440: [swMonlog]已经停止...");

  exit(sig);
}

/*begin add by gf at 2004-04-07*/

/**************************************************************
 ** 函数名      ： swSendRevresult_nounion
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
    swVdebug(0,"S0630: [错误/其它] 取交易流水响应报文失败");
    return(FAIL);
  }
    
  /*查找路由组中有无语句
   *9999 WITH REVMODE 4
   */
  ilRc=swShmselect_route_d_last(slMsgpack1.sMsghead.iBegin_q_id,slMsgpack1.sMsghead.aTrancode);
  if(ilRc)
  {
    swVdebug(2,"S9000: 没有返回冲正结果路由语句");
    return(FAIL);
  }

  slMsgpack1.sMsghead.iMsgtype = iMsgtype;
  
  /* 发送冲正结果通知报文 */
  ilRc = qwrite2((char *)&slMsgpack1,  \
     slMsgpack1.sMsghead.iBodylen + sizeof(struct msghead), \
        slMsgpack1.sMsghead.iBegin_q_id,0,0,0);
  if(ilRc)
  {
    swVdebug(0,"S0720: [错误/邮箱] qwrite2()函数,错误码=%d, 发送通知报文到前置进程失败,[qid=%d]",ilRc, slMsgpack1.sMsghead.iBegin_q_id); 
    return(FAIL);
  }
  swVdebug(2,"S0730: 发送通知报文到前置进程成功,[qid=%d]",slMsgpack1.sMsghead.iBegin_q_id);
  return(SUCCESS);
}
/*end add*/     
