/****************************************************************/
/* 模块编号    ：MONREV                                         */ 
/* 模块名称    ：事务处理监测自带邮箱                           */
/* 版 本 号    ：V4.3.0                                         */
/* 作	 者    ：张辉                                           */
/* 建立日期    ：1999/11/22                                     */
/* 最后修改日期：2001/3/24                                      */
/* 模块用途    ：处理冲正响应,即时冲正,停止冲正                 */
/* 本模块中包含如下函数及功能说明：                             */
/*		  (1) void main();                     		*/
/*                (2) void swQuit();                   		*/
/*                (3) int swProcess_proclog(long,short)	        */
/*                (4) int swProcess_revend_tranlog(long,short); */
/*                (5) int swProcess_revend_proclog(long,short); */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/*    1999/12/21 增加了对应用SAF响应报文的处理                  */
/*    2000/1/29  根据中信二期的设计加以修改                     */
/*    2000/4/5   优化                                           */
/*    2000/8/30  优化                                           */
/*    2001/3/12  Oracle移植                                     */
/*    2001/3/23  封版V3.01,程序共2720行                         */
/****************************************************************/

/* switch定义 */
#include "switch.h"

/* 常量定义 */
#define   DEBUG          1                   /* 调试信息开关 */

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

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* POSTGRES数据库支持(结构必须重新定义) */
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
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

/* 函数原型定义 */
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
 ** 函数名      ： main
 ** 功  能      ： 主函数
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/11/22
 ** 最后修改日期： 1999/4/5
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 无
***************************************************************/
int main(int argc,char **argv)
{
  unsigned int    ilMsglen;                          /* 报文长度 */
  short   ilOrigQid;           /* 原发信箱号 */
  short   ilPrior=0,ilClass=0,ilType=0;      /* 邮箱参数 */
  struct  msgpack slMsgpack;                 /* 报文 */
  short   ilTran_step;                       /* 报文头交易步骤 */
  long    llCurrenttime;                     /* 系统当前时间 */
  short   ilFlag = 0;
  char    clLoc_flag;

  /* 2002.11.13 fzj add */
  struct  swt_proc_log slSwt_proc_log1;               /* 处理流水 */
  union  preunpackbuf pulPrebuf1[iBUFFLDNUM]; /* 预解池 */
  union  preunpackbuf pulPrebuf2[iBUFFLDNUM]; /* 预解池 */
  /**/

  /* 2003.02.23 gxz add */
  struct  swt_proc_log slSwt_proc_log_view;           /* 处理流水 */
  /**/
  
  /* 数据库变量定义 */
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  llSaf_id;                          /* SAF流水号 */
    sqlint64  llTran_id;                         /* 流水号 */
    short ilProc_step;                       /* 交易步骤 */
    short ilProc_step_next;                  /* 下一交易步骤 */
    short ilProc_step_max;
    short ilTranstat;			     /* 交易状态 */			
    short ilTmp;			     /* 指示变量 */                       
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llSaf_id;                          /* SAF流水号 */
    long  llTran_id;                         /* 流水号 */
    short ilProc_step;                       /* 交易步骤 */
    short ilProc_step_next;                  /* 下一交易步骤 */
    short ilProc_step_max;
    short ilTranstat;			     /* 交易状态 */			
    short ilTmp;			     /* 指示变量 */                       
  EXEC SQL END DECLARE SECTION;
#endif
/*add by zcd 20141224*/
#else
	long  llSaf_id; 						 /* SAF流水号 */
	long  llTran_id;						 /* 流水号 */
	short ilProc_step;						 /* 交易步骤 */
	short ilProc_step_next; 				 /* 下一交易步骤 */
	short ilProc_step_max;
	short ilTranstat;				 /* 交易状态 */ 		
	short ilTmp;
#endif
/*end of add by zcd 20141224*/
  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swMonrev Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swMonrev")) exit(FAIL);
  
  /* 设置调试程序名称 */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swMonrev.debug");

  /* 取得DEBUG标志 */
  if((cgDebug=_swDebugflag("swMonrev"))==FAIL)
  {
    fprintf(stderr,"取DEBUG标志失败!\n");
    exit(FAIL);
  } 
  swLogInit("swMonrev", "swMonrev.debug");
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
 
  /* 向系统监控发送信息报文 */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: 发送104001信息报文,[swMonrev]已经启动...");
 
  /* 循环处理 */
  while (TRUE)
  {	
    ilMsglen = sizeof(struct msgpack);
    ilPrior = ilClass = ilType = 0;
    ilRc = qread2((char *)&slMsgpack,&ilMsglen,&ilOrigQid,&ilPrior,&ilClass,&ilType);
    if(ilRc == FAIL)
    {
      swVdebug(0,"S0110: [错误/邮箱] qread2()函数,错误码=%d,读邮箱出错",ilRc);
      swMberror(ilRc,(char *)&slMsgpack);
      swQuit(FAIL);
    }
    swVdebug(2,"S0120: 读邮箱成功");
 
    /* 刷新共享内存指针 */
    if(swShmcheck() == FAIL) 
    {
      swVdebug(0,"S0130: [错误/共享内存] 刷新共享内存指针失败");
      swQuit(FAIL);
    }
    
    swVdebug(3,"S0140: 邮箱[%d]发来报文,报文类型为[%d],报文长度为[%d]",
      ilOrigQid,slMsgpack.sMsghead.iMsgtype,ilMsglen);
    if(cgDebug>=2) swDebugmsghead((char *)&slMsgpack);
    if(cgDebug>=2) swDebughex((char *)slMsgpack.aMsgbody,ilMsglen-sizeof(struct msghead));
 
    if(ilMsglen == 0)
    {
      swVdebug(2,"S0150: 收到空报文");
      continue;
    }

    clLoc_flag = slMsgpack.sMsghead.aSysattr[0];
    switch (slMsgpack.sMsghead.iMsgtype)
    {
      case iMSGREV:  /* 冲正响应报文 */
        llTran_id   = slMsgpack.sMsghead.lTranid;
        ilProc_step = slMsgpack.sMsghead.iTranstep;
	llSaf_id    = slMsgpack.sMsghead.lSafid;
        swVdebug(2,"S0160: 冲正响应报文,[tranid=%ld,step=%d,safid=%ld]",llTran_id,ilProc_step,llSaf_id);
        
        /* 判断处理流水是否存在,并且状态为正在冲正 */
        ilRc = swShmselect_swt_proc_log(llTran_id,ilProc_step,&sgSwt_proc_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0170: [错误/共享内存] 查找处理流水失败,[tranid=%ld,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0180: 处理流水没有找到,[tranid=%d,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);
          continue;
        }
        
        /* ADD BY GXZ 2003.02.23 Begin */
        slSwt_proc_log_view = sgSwt_proc_log;
        /* ADD BY GXZ 2003.02.23 End */
        
        if(sgSwt_proc_log.proc_status != iTRNREVING)
        {
          swVdebug(2,"S0190: 处理流水状态不为正在冲正,[tranid=%d,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);
          continue;
        }   
        swVdebug(2,"S0200: 找到处理流水,状态为正在冲正,[tranid=%d,step=%d,loc=%c]",llTran_id,ilProc_step,clLoc_flag);

        /* 计算冲正结果表达式 */
        ilRc = swRevSuccess(slMsgpack);
        if(ilRc == FAIL)
        {
          swVdebug(2,"S0210: 冲正失败处理");
           
          /* 置SAF记录状态为SAF失败,移历史 */
          /*modify by cjh 20150114*/
          ilRc = swShmselect_swt_rev_saf(llSaf_id,&sgSwt_rev_saf);
          /*modify by cjh 20150114 end*/
          if(ilRc == FAIL)
          {
            swVdebug(0,"S0220: [错误/共享内存] 查找SAF记录出错,[safid=%ld]",llSaf_id);
            continue;
          }  

          if(ilRc == SHMNOTFOUND)
          {
            swVdebug(2,"S0230: SAF记录没有找到,[safid=%ld]",llSaf_id);
            continue;
          }
          swVdebug(2,"S0240: 找到SAF记录,[safid=%ld]",llSaf_id);
          
          ilRc = swProcess_revfail_revsaf(llSaf_id);
          if(ilRc == FAIL) continue;

          /* 置该笔交易的所有处理流水状态为冲正失败,移历史,删除之 */          
          ilRc = swProcess_revend_proclog(llTran_id,iTRNREVFAIL,clLoc_flag);
          if(ilRc == FAIL) continue;
          
          /* 置交易流水状态为冲正失败,移历史,删除之 */
          ilRc = swProcess_revend_tranlog(llTran_id,iTRNREVFAIL,clLoc_flag,slSwt_proc_log_view,slMsgpack);
          /*begin add by gf at 2004-04-07*/
          if(sgSwt_rev_saf.saf_num > 0)
            continue;
          ilRc=swSendRevresult_union(llTran_id,slMsgpack,iMSGREVFAIL,clLoc_flag);
          /*end add*/

          continue;
        }
 
        /* 删除SAF记录 */
        /*modify by cjh 20150114*/
        ilRc = swShmdelete_swt_rev_saf(llSaf_id);
        /*modify by cjh 20150114 end*/
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0250: [错误/共享内存] 删除SAF记录出错,[safid=%ld]",llSaf_id);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0260: SAF记录没有找到,[safid=%ld]",llSaf_id);
          continue;
        }  
	swVdebug(2,"S0270: 删除SAF记录成功,[safid=%ld]",llSaf_id);


        /* 2002.11.13 fzj add */
        slSwt_proc_log1 = sgSwt_proc_log; /* 用于报文并集 */
        /**/
        /* 保存报文 */
	sgSwt_proc_log.org_len = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
        ilRc = swPutlog((char *)&slMsgpack,sgSwt_proc_log.org_len,
          &(sgSwt_proc_log.org_offset),sgSwt_proc_log.org_file);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0280: [错误/其它] 保存报文出错");
          continue;
        }
        swVdebug(2,"S0290: 保存报文成功");
 
        sgSwt_proc_log.proc_status = iTRNREVEND;
        ilRc = swShmupdate_swt_proc_log(llTran_id,ilProc_step,sgSwt_proc_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0300: [错误/共享内存] 更新处理流水失败,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0310: 处理流水没有找到,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          continue;
        }  
        swVdebug(2,"S0320: 置处理流水状态为冲正结束,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
        swVdebug(2,"S0330: 更新处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
#ifdef DB_INFORMIX
        EXEC SQL BEGIN WORK;
#endif
        /* 移历史 */
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
          swVdebug(0,"S0340: [错误/数据库] 追加历史处理流水失败,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
          swDberror(NULL);
          EXEC SQL ROLLBACK WORK;
          continue;
        }
        swVdebug(2,"S0350: 追加历史处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#else
		sw_put_proclog(&sgSwt_proc_log);
#endif
        /* 删除之 */
        ilRc = swShmdelete_swt_proc_log(llTran_id,ilProc_step,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0360: [错误/共享内存] 删除处理流水出错,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
          EXEC SQL ROLLBACK WORK;
#endif
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0370: 处理流水没有找到,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
          EXEC SQL ROLLBACK WORK;
#endif
          continue;
        }  
        swVdebug(2,"S0380: 删除处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
        EXEC SQL COMMIT WORK;
#endif

        swVdebug(2,"S0390: 查找下一笔处理流水");
 
        /* 找下一笔处理流水 */
        ilRc = swShmselect_max_proclog_tran(llTran_id,&sgSwt_proc_log);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0400: [错误/共享内存] 查找处理流水出错,[tranid=%ld]",llTran_id);
          continue;
        }
        if(ilRc == SHMNOTFOUND)
        {  
          swVdebug(2,"S0410: 处理流水没有找到,冲正结束处理");
           
          /* 置该笔交易流水状态为冲正结束,移历史,删除之 */
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
	swVdebug(2,"S0420: 找到下一笔处理流水,[tranid=%ld,step=%d]",llTran_id,ilProc_step_next);
	/*add by pc 2009-1-20 */
if (ilProc_step_next>=ilProc_step){
swVdebug(2,"S0488: 下一步步骤>=当前步骤,冲正结束处理");
 
/* 置该笔交易流水状态为冲正结束,移历史,删除之 */
ilRc = swProcess_revend_tranlog(llTran_id,iTRNREVEND,clLoc_flag,slSwt_proc_log_view,slMsgpack);
ilRc=swSendRevresult_union(llTran_id,slMsgpack,iMSGREVEND,clLoc_flag);

/*end add*/
continue;	
}	
	/*add by pc 2009-1-20 */
	/* 取得报文 */
        memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
        _swTrim(sgSwt_proc_log.org_file);
        ilRc = swGetlog(sgSwt_proc_log.org_file,sgSwt_proc_log.org_len,
          sgSwt_proc_log.org_offset,(char *)&slMsgpack);
        if(ilRc == FAIL)  
        {
          swVdebug(0,"S0430: [错误/其它] 取处理流水报文失败[%s]",sgSwt_proc_log.org_file);
          _swMsgsend(304011,NULL);
          continue;
        }
        slMsgpack.sMsghead.aSysattr[0] = clLoc_flag; /* add by nh 20021114 */
        swVdebug(2,"S0440: 取处理流水报文成功[%s]",sgSwt_proc_log.org_file);
 
/* 2002.11.13 fzj add
 * 冲正报文=并集(当前步骤原报文+上一步骤原报文+上一步骤冲正响应报文)
 */
        /* 上一步骤冲正响应报文计算结果时已预解包 */
        /*delete by gxz 2003.02.28
        memcpy((char *)pulPrebuf2,(char *)psgPreunpackbuf,sizeof(psgPreunpackbuf));
        */
        /* 当前步骤原报文预解包 */
        /*delete by gxz 2003.02.28
        ilRc = swFmlunpack(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,psgPreunpackbuf);
        if(ilRc)
        {
          swVdebug(0,"S0450: swFmlunpack() failed");
          continue;
        }
        */
        /* 取上一步骤原报文 */ 
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
        
        /* 上一步骤原报文预解包 */
        /*delete by gxz 2003.02.28
        ilRc = swFmlunpack(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,pulPrebuf1);
        if(ilRc)
        {
          swVdebug(0,"S0470: swFmlunpack() failed");
          continue;
        }
        */
        /* 当前步骤原报文与上一步骤原报文并集 */ 
        /*delete by gxz 2003.02.28
        ilRc = _swFmlunion(psgPreunpackbuf,pulPrebuf1);
        if(ilRc)
        {
          swVdebug(0,"S0480: _swFmlunion() failed");
          continue;
        }
        */
        /* 再与上一步骤冲正响应报文并集 */
        /*delete by gxz 2003.02.28
        ilRc = _swFmlunion(psgPreunpackbuf,pulPrebuf2);
        if(ilRc)
        {
          swVdebug(0,"S0490: _swFmlunion() failed");
          continue;
        }
        swVdebug(2,"S0500: 报文并集成功");
        */
        /* 并集后的报文打包 */
        /*delete by gxz 2003.02.28
        ilRc = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,&ilMsglen);
        if(ilRc)
        {
          swVdebug(0,"S0510: swFmlpack() failed");
          continue;
        }
        slMsgpack.sMsghead.iBodylen = ilMsglen;
        ilMsglen = sizeof(struct msghead) + ilMsglen;
        swVdebug(2,"S0520: 组织冲正报文成功");
        */
/**/

	/* 这时只剩下revmode是1和2的情况,3和4的情况已经在swMonlog和swMonrev
           的即时冲正部分先做了处理 */

        /* 更新处理流水状态为正在冲正 */
        sgSwt_proc_log.proc_status = iTRNREVING;
        ilRc = swShmupdate_swt_proc_log(llTran_id,ilProc_step_next,sgSwt_proc_log,clLoc_flag);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0530: [错误/共享内存] 更新处理流水出错,[tranid=%ld,step=%d]",llTran_id,ilProc_step_next);
          continue;
        }  
        if(ilRc == SHMNOTFOUND)
        {
          swVdebug(2,"S0540: 处理流水没有找到,[tranid=%ld,step=%d,loc=%c]",llTran_id,ilProc_step_next,clLoc_flag);
          continue;
        }
        swVdebug(2,"S0550: 置处理流水状态为正在冲正,更新处理流水成功,[tranid=%ld,step=%d,loc=[%c]]",llTran_id,ilProc_step_next,clLoc_flag);  
 
        /* 取得SAF流水号 */
        memset((char *)&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
        if(swShmgetsafid(&llSaf_id) == FAIL)
        {
          swVdebug(0,"S0560: [错误/共享内存] 取SAF流水号失败");
          continue;
        }
        swVdebug(2,"S0570: 取得SAF流水号,[safid=%ld]",llSaf_id);
 
        /* 组织报文头 */
        slMsgpack.sMsghead.lTranid      = llTran_id;
        slMsgpack.sMsghead.iTranstep    = ilProc_step_next;
        slMsgpack.sMsghead.iMsgtypeorg  = slMsgpack.sMsghead.iMsgtype;
        slMsgpack.sMsghead.iMsgtype     = iMSGREV;
        slMsgpack.sMsghead.iDes_q       = sgSwt_proc_log.q_target;
        slMsgpack.sMsghead.lSafid       = llSaf_id;
        slMsgpack.sMsghead.cSafflag     = '1';
        slMsgpack.sMsghead.iRevfmtgroup = sgSwt_proc_log.rev_fmtgrp;
        slMsgpack.sMsghead.aSysattr[0] = clLoc_flag; /* add by nh 20021114 */

        /* 记录SAF报文流水 */
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
          swVdebug(0,"S0580: [错误/其它] 保存SAF报文出错");
          continue;
        }
        swVdebug(2,"S0590: 保存SAF报文成功");

       ilRc = swShmselect_swt_tran_log(llTran_id,&sgSwt_tran_log,clLoc_flag);
       if (ilRc == FAIL)
       {
         swVdebug(0,"S0600: 查找交易流水出错,[tranid=%ld]",llTran_id);
         continue;
       }
       if (ilRc == SHMNOTFOUND)
       {
         swVdebug(2,"S0610: 没有找到交易流水,[tranid=%ld]",llTran_id);
         continue;
       }
       swVdebug(2,"S0620: 查找交易流水成功,[tranid=%ld]",llTran_id);
       sgSwt_tran_log.q_target = sgSwt_proc_log.q_target;
       ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
       if (ilRc == FAIL)
       {
         swVdebug(0,"S0630: 更新交易流水出错,[tranid=%ld]",llTran_id);
         continue;
       }
       if (ilRc == SHMNOTFOUND)
       {
         swVdebug(2,"S0640: 要更新的交易流水没有找到,[tranid=%ld]",llTran_id);
         continue;
       }
       swVdebug(2,"S0650: 更新交易流水目标邮箱为%d成功,[tranid=%ld]",sgSwt_tran_log.q_target,llTran_id);
 
        /* 追加SAF记录 */
       /*modify by cjh 20150114*/
        ilRc = swShminsert_swt_rev_saf(sgSwt_rev_saf);  
        /*modify by cjh 20150114 end*/
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0660: [错误/共享内存] 追加SAF记录出错,[safid=%ld]",llSaf_id);
          continue;
        }  
        swVdebug(2,"S0670: 追加SAF记录成功,[safid=%ld]",llSaf_id);
        /* 发送冲正报文至前置进程 */
        swVdebug(3,"S0680: 发送冲正报文开始");
        if(cgDebug>=2)
        {
          swDebugmsghead((char *)&slMsgpack);
          swDebughex(slMsgpack.aMsgbody, slMsgpack.sMsghead.iBodylen);
        }
        swVdebug(3,"S0690: 发送冲正报文结束");
        ilRc = qwrite2((char *)&slMsgpack,sgSwt_proc_log.org_len,sgSwt_proc_log.q_target,ilPrior,ilClass,ilType);
        if(ilRc == FAIL)
        {
          swVdebug(0,"S0700: [错误/函数调用] qwrite2 出错! 发送冲正报文至前置进程失败,[qid=%d]",sgSwt_proc_log.q_target);
          swMberror(ilRc,(char *)&slMsgpack);
          _swMsgsend(304001,(char *)&slMsgpack); 
        }
        else
        {
          swVdebug(2,"S0710: 发送冲正报文至前置进程成功,[qid=%d]",sgSwt_proc_log.q_target);
          _swMsgsend(199003,(char *)&slMsgpack);
        }  
        continue;

      case iMSGORDER:
        swVdebug(2,"S0720: 收到命令报文[%d]",slMsgpack.sMsghead.lCode);
        switch(slMsgpack.sMsghead.lCode)
        {
          case 601:	/* 即时冲正 */
            llTran_id = slMsgpack.sMsghead.lTranid;
            ilTran_step = slMsgpack.sMsghead.iTranstep;
            swVdebug(2,"S0730: 开始即时冲正处理,[tranid=%ld,step=%d]...",llTran_id,ilTran_step);
            /* 查找交易流水 */
            ilRc = swShmselect_swt_tran_log(llTran_id,&sgSwt_tran_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S0740: [错误/共享内存] 查找交易流水出错,[tranid=%ld]",llTran_id);
              continue;
            }
			
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
            if(ilRc == SHMNOTFOUND)
            {
              /* 没有找到,到历史中去找 */
              swVdebug(2,"S0750: 交易流水没有找到,到历史表中去找,[tranid=%ld]",llTran_id);
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
                swVdebug(2,"S0760: 历史表中无此交易流水,或者已经被冲正,[tranid=%ld]",llTran_id);
                _swMsgsend(304002,(char *)&slMsgpack);
                continue;
              }
              else if(sqlca.sqlcode) 
              {
                swVdebug(0,"S0770: [错误/数据库] 查找历史交易流水出错,[tranid=%ld]",llTran_id);
                swDberror((char *)&slMsgpack);
                continue;
              }
              else /* 在历史表中找到交易流水,移到当前库 */
              {
                swVdebug(2,"S0780: 在历史表中找到此交易流水,从历史移到当前");
                swVdebug(3,"S0790: 交易状态为[%d],交易发起方信箱为[%d]",sgSwt_tran_log.tran_status,sgSwt_tran_log.q_tran_begin);
                ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
                if(ilRc == FAIL)
                {
                  swVdebug(0,"S0800: [错误/共享内存] 追加交易流水出错,[tranid=%ld]",llTran_id); 
                  continue;
                }  
#ifdef DB_INFORMIX
                EXEC SQL BEGIN WORK;
#endif
                EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTran_id;
                if(sqlca.sqlcode ) 
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(0,"S0810: [错误/数据库] 删除历史交易流水失败,[tranid=%ld]",llTran_id);
 	          swDberror((char *)&slMsgpack);
                  continue;
                }
                /* 移处理流水 */
                
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
                  swVdebug(2,"S0830: 没有找到历史处理流水,[tranid=%ld]",llTran_id);
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
                    swVdebug(0,"S0850: [错误/共享内存] 追加处理流水出错,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                    ilFlag = -1;
                    break;
                  }
                  swVdebug(2,"S0860: 追加处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                  EXEC SQL DELETE FROM swt_his_proc_log WHERE tran_id = :llTran_id AND proc_step = :ilProc_step;
                  if (sqlca.sqlcode)
                  {
                    EXEC SQL ROLLBACK WORK;
                    swVdebug(0,"S0870: DELETE FROM swt_his_proc_log WHERE ... error");
                    swDberror(NULL);
                    ilFlag = -1;
                    break;
                  }
                  swVdebug(2,"S0880: 删除历史处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                  ilFlag++;
                }
                if (ilFlag < 0) continue;
                EXEC SQL COMMIT WORK;
                swVdebug(2,"S0890: 交易流水和处理流水从历史移到当前完成");
              }  
            }
#endif  /*support database,Database Deprivation Project 2014-12*/
		   
            /* 置交易流水状态为正在冲正 */
            sgSwt_tran_log.tran_status = iTRNREVING;
            ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S0900: [错误/共享内存] 更新交易流水失败,tranid=%ld",llTran_id);
              continue;
            }  
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S0910: 交易流水没有找到,tranid=%ld",llTran_id);
              continue;
            }  
            swVdebug(2,"S0920: 置交易流水状态为正在冲正,[tranid=%ld]",llTran_id);
            swVdebug(2,"S0930: 更新交易流水成功,[tranid=%ld]",llTran_id);
 
	    /* 删除超时报文 */
	    ilRc = swShmselect_swt_proc_log(llTran_id,1,&sgSwt_proc_log,clLoc_flag);
	    if(ilRc == FAIL)
	    {
	      swVdebug(0,"S0940: [错误/共享内存] 查找处理流水出错,[tranid=%ld,step=1]",llTran_id);
 	      continue;
	    }
	    if(ilRc == SUCCESS)
	    {
	      if(sgSwt_proc_log.rev_mode <= 4)
	      {
	        ilRc = swShmdelete_swt_proc_log(llTran_id,1,clLoc_flag);
	        if(ilRc == FAIL)
	        {
	          swVdebug(0,"S0950: [错误/共享内存] 删除处理流水出错,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	        if(ilRc == SHMNOTFOUND)
	        {
	          swVdebug(2,"S0960: 处理流水没有找到,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	      }
	      else
	      {
	        sgSwt_proc_log.rev_mode = 4;
	        ilRc = swShmupdate_swt_proc_log(llTran_id,1,sgSwt_proc_log,clLoc_flag);
	        if(ilRc == FAIL)
	        {
	          swVdebug(0,"S0970: [错误/共享内存] 更新处理流水出错,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	        if(ilRc == SHMNOTFOUND)
	        {
	          swVdebug(2,"S0980: 处理流水没有找到,[tranid=%ld,step=1]",llTran_id);
 	          continue;
	        }
	      }   
            }
            swVdebug(2,"S0990: 删除对源发端口的超时处理,[tranid=%ld,step=1]",llTran_id);	     

            swVdebug(2,"S1000: 开始处理revmode=3和4的情况...");
            
            /* 查找REVMODE=3和4的处理流水记录 */
            ilRc = swShmselect_swt_proc_log_mrec(llTran_id,pslSwt_proc_log,&ilCount);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1010: [错误/共享内存] 查找REVMODE=3和4的处理流水失败");
              continue;
            }
     
            /* 循环取出处理流水记录并发送通知报文 */
            j = 0;
            for(i = 0; i < ilCount; i ++)
            {
              /* 只处理冲正模式为3或4的处理流水 */
              if(pslSwt_proc_log[i].rev_mode != 3 && pslSwt_proc_log[i].rev_mode != 4)
                continue;
                
              j++;
              ilProc_step = pslSwt_proc_log[i].proc_step;
              swVdebug(2,"S1020: 找到REVMODE=3或4的处理流水,[tranid=%ld,step=%d]",llTran_id,ilProc_step);       
              
              /* 取得原始报文 */
              memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
              _swTrim(pslSwt_proc_log[i].org_file);
              ilRc = swGetlog(pslSwt_proc_log[i].org_file,pslSwt_proc_log[i].org_len,
                pslSwt_proc_log[i].org_offset,(char *)&slMsgpack);
              if(ilRc == FAIL) 
              {
                swVdebug(0,"S1030: [错误/其它] 取处理流水原始报文失败");
                _swMsgsend(304011,NULL);
                continue;
              }
              swVdebug(2,"S1040: 取处理流水原始报文成功");
 
              /* 组织并发送通知报文 */
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
	        swVdebug(0,"S1050: [错误/邮箱] qwrite2()函数,错误码=%d, 发送通知报文到前置进程失败,[qid=%d]",ilRc,pslSwt_proc_log[i].q_target); 
 	        _swMsgsend(304012,NULL);
                continue;
              }
              swVdebug(2,"S1060: 发送通知报文到前置进程成功,[qid=%d]",pslSwt_proc_log[i].q_target); 
              _swMsgsend(199003,(char *)&slMsgpack);
 
              /* 置该笔处理流水状态为冲正结束,移历史,删除之 */
              ilRc = swProcess_proclog(llTran_id,pslSwt_proc_log[i].proc_step,clLoc_flag);
              if(ilRc == FAIL) break;
            }
            if(ilRc == FAIL) continue;
            swVdebug(2,"S1070: 完成对REVMODE=3和4的处理,共[%d]笔",j);

            /* 查找REVMODE=1和2的最大处理流水 */
            ilRc = swShmselect_max_proclog_tran(llTran_id,&sgSwt_proc_log);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1080: [错误/共享内存] 查找REVMODE=1和2的最大处理流水出错,[tranid=%ld]",llTran_id);
              continue;
            }
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1090: 没有找到REVMODE=1或2的处理流水,冲正成功,[tranid=%ld]",llTran_id);
              /* 该笔交易流水状态置冲正结束,移历史,删除之 */
              swProcess_revend_tranlog(llTran_id,iTRNREVEND,clLoc_flag,slSwt_proc_log_view,slMsgpack);
              /*begin add by gf at 2004-04-07*/
              ilRc=swSendRevresult_nounion(iMSGREVEND);
              /*end add*/
              continue;
            }
            /* 还有处理流水,进行冲正 */
            ilProc_step = sgSwt_proc_log.proc_step;
            swVdebug(2,"S1100: 找到REVMODE=1或2的最大处理流水,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
            /* 取得原始报文 */
            memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
            _swTrim(sgSwt_proc_log.org_file);
            ilRc = swGetlog(sgSwt_proc_log.org_file,sgSwt_proc_log.org_len,
              sgSwt_proc_log.org_offset,(char *)&slMsgpack);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1110: [错误/其它] 取处理流水原始报文失败");
              _swMsgsend(304011,NULL);
              continue;
            } 
            swVdebug(2,"S1120: 取处理流水原始报文成功");
             
            /*add by gxz 2003.8.23 begin*/
                /* 取得端口设置 */
            ilRc = swShmselect_swt_sys_queue(sgSwt_proc_log.q_target, &sgSwt_sys_queue);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1112: [错误/共享内存] 取端口设置失败,[qid=%d]",sgSwt_proc_log.q_target);
              continue;
            }  
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1114: 端口没有设置,[qid=%d]",sgSwt_proc_log.q_target);
              continue;
            }  
            swVdebug(2,"S1116: 取端口设置成功,[qid=%d]",sgSwt_proc_log.q_target);
           /*add by gxz 2003.8.23 end */
            /* 取得SAF流水号 */
            memset((char *)&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
            ilRc = swShmgetsafid(&llSaf_id);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1130: [错误/共享内存] 取SAF流水号出错");
              continue;
            }
            swVdebug(2,"S1140: 取SAF流水号成功,[safid=%ld]",llSaf_id);
 
            /* 组织SAF报文头 */
            slMsgpack.sMsghead.lTranid = llTran_id;
            slMsgpack.sMsghead.iTranstep = sgSwt_proc_log.proc_step;
            slMsgpack.sMsghead.iMsgtypeorg = slMsgpack.sMsghead.iMsgtype;
            slMsgpack.sMsghead.iMsgtype = iMSGREV;
            slMsgpack.sMsghead.iDes_q = sgSwt_proc_log.q_target;
            slMsgpack.sMsghead.lSafid = llSaf_id;
            slMsgpack.sMsghead.cSafflag = '1';
            slMsgpack.sMsghead.iRevfmtgroup = sgSwt_proc_log.rev_fmtgrp;
            slMsgpack.sMsghead.aSysattr[0]=clLoc_flag;/* add by nh 20021114 */

            /* 保存SAF报文 */
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
              swVdebug(0,"S1150: [错误/其它] 保存SAF报文出错,[safid=%d]",llSaf_id);
              continue;
            }
            swVdebug(2,"S1160: 保存SAF报文成功,[safid=%d]",llSaf_id);
            
            /* 追加SAF记录 */
            /*modify by cjh 20150114*/
            ilRc = swShminsert_swt_rev_saf(sgSwt_rev_saf);  /*unsolved*/
            /*modify by cjh 20150114 end*/
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1170: [错误/共享内存] 追加SAF记录出错,[safid=%ld]",llSaf_id);
              continue;
            }  
            sgSwt_proc_log.proc_status = iTRNREVING;
            ilRc = swShmupdate_swt_proc_log(llTran_id,ilProc_step,sgSwt_proc_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1180: [错误/共享内存] 更新处理流水出错,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
              continue;
            }
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1190: 处理流水没有找到,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
              continue;
            }
            swVdebug(2,"S1200: 置处理流水状态为正在冲正,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
            swVdebug(2,"S1210: 更新处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);

            sgSwt_tran_log.q_target = sgSwt_proc_log.q_target;
            ilRc = swShmupdate_swt_tran_log(llTran_id,sgSwt_tran_log,clLoc_flag);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1220: [错误/共享内存] 更新交易流水失败,[tranid=%ld]",llTran_id);
              continue;
            }  
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1230: 交易流水没有找到,[tranid=%ld]",llTran_id);
              continue;
            }  
            swVdebug(2,"S1240: 置交易流水目标邮箱为%d,[tranid=%ld]",sgSwt_tran_log.q_target,llTran_id);
            swVdebug(2,"S1250: 更新交易流水成功,[tranid=%ld]",llTran_id);

            /* 发送冲正报文至前置进程 */
            if(cgDebug>=2)
            {
              swDebugmsghead((char *)&slMsgpack);
              swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
            ilRc = qwrite2((char *)&slMsgpack,sgSwt_proc_log.org_len,sgSwt_proc_log.q_target,ilPrior,ilClass,ilType);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1260: [错误/邮箱] qwrite2 出错! 发送冲正报文至前置进程失败,[qid=%d]",sgSwt_proc_log.q_target);
              swMberror(ilRc,(char *)&slMsgpack);
              _swMsgsend(304001,(char *)&slMsgpack); 
            }
            else
            {
              swVdebug(2,"S1270: 发送冲正报文至前置进程成功,[qid=%d]",sgSwt_proc_log.q_target);
              _swMsgsend(199003,(char *)&slMsgpack);
            }  
            continue;
            break;

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/
          case 604:	/* RESAF */
            llSaf_id = slMsgpack.sMsghead.lSafid;
            swVdebug(2,"S1280: 开始RESAF处理,[safid=%ld]...",llSaf_id);
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
              swVdebug(2,"S1290: 历史SAF记录没有找到,[safid=%ld]",llSaf_id);
              continue;
            }
            if(sqlca.sqlcode)
            {
              swDberror(NULL);
              swVdebug(0,"S1300: [错误/数据库] 查找历史SAF记录出错,[safid=%ld]",llSaf_id);
              continue;
            }
            swVdebug(2,"S1310: 找到历史SAF记录,[safid=%ld]",llSaf_id);

            /*取得SAF报文*/
            memset((char *)&slMsgpack,0x00,sizeof(slMsgpack));
            ilRc = swGetlog(sgSwt_rev_saf.saf_file,sgSwt_rev_saf.saf_len,
              sgSwt_rev_saf.saf_offset,(char *)&slMsgpack);
            if(ilRc)
            {
              swVdebug(0,"S1320: [错误/其它] 取SAF报文失败");
              _swMsgsend(304010,NULL);
              continue;
            }  
            slMsgpack.sMsghead.aSysattr[0] = clLoc_flag;/* add by nh 20021114 */
            swVdebug(2,"S1330: 取SAF报文成功",llSaf_id,sgSwt_rev_saf.saf_file);
             
            /*取得目标端口设置*/
            ilRc = swShmselect_swt_sys_queue(slMsgpack.sMsghead.iDes_q,&sgSwt_sys_queue);
            if(ilRc == FAIL)
            {
              swVdebug(0,"S1340: [错误/共享内存] 取目标端口设置失败,[qid=%d]",slMsgpack.sMsghead.iDes_q);
              continue;
            }
            if(ilRc == SHMNOTFOUND)
            {
              swVdebug(2,"S1350: 目标端口没有设置,[qid=%d]",slMsgpack.sMsghead.iDes_q);
              continue;
            }
            swVdebug(2,"S1360: 取得目标端口设置,[qid=%d]",slMsgpack.sMsghead.iDes_q);
             
            time(&llCurrenttime);
            
            /* RESAF处理 */
            if(sgSwt_rev_saf.saf_flag[0] == '1') /*应用SAF*/
            {
              /*将SAF记录由历史表移回当前表*/
              sgSwt_rev_saf.saf_status[0] = '0';
              sgSwt_rev_saf.saf_overtime = llCurrenttime + sgSwt_sys_queue.rev_overtime;
              sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;
#ifdef DB_INFORMIX
              EXEC SQL BEGIN WORK;
#endif
              EXEC SQL update swt_rev_saf set saf_status='0',saf_overtime=:sgSwt_rev_saf.saf_overtime,saf_num=:sgSwt_rev_saf.saf_num where saf_id=:llSaf_id;
              if(sqlca.sqlcode)
              {
                swVdebug(0,"S1370: [错误/数据库] update历史SAF记录出错,[safid=%ld]",llSaf_id);
                EXEC SQL ROLLBACK WORK;                
                continue;
              }
              swVdebug(2,"S1380: update历史SAF记录成功,[safid=%ld]",llSaf_id);  
              EXEC SQL COMMIT WORK;
            }
            else /*冲正SAF*/
            {
              /*将交易流水由历史表移回当前表 */
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
                swVdebug(2,"S1410: 历史交易流水没有找到,[tranid=%ld]",llTran_id);
                continue;
              }
              if(sqlca.sqlcode)
              {
                swVdebug(0,"S1420: [错误/数据库] 查找历史交易流水出错,[tranid=%ld]",llTran_id);
                swDberror(NULL);
                continue;
              }
              swVdebug(2,"S1430: 找到历史交易流水,[tranid=%ld]",llTran_id);

              if(sgSwt_tran_log.tran_status != iTRNREVFAIL) continue;

              sgSwt_tran_log.tran_status = iTRNREVING;
              time(&sgSwt_tran_log.tran_begin);
              clLoc_flag='1';
              ilRc = swShminsert_swt_tran_log(sgSwt_tran_log,&clLoc_flag);
              if(ilRc == FAIL)
              {
                swVdebug(0,"S1440: [错误/共享内存] 追加交易流水出错,[tranid=%ld]",llTran_id);
                continue;
              }
              swVdebug(2,"S1450: 追加交易流水成功,[tranid=%ld]",llTran_id);
#ifdef DB_INFORMIX
              EXEC SQL BEGIN WORK;
#endif
              EXEC SQL DELETE FROM swt_his_tran_log WHERE tran_id = :llTran_id;
              if(sqlca.sqlcode)
              {
                EXEC SQL ROLLBACK WORK;
                swVdebug(0,"S1460: [错误/数据库] 删除历史交易流水失败,[tranid=%ld]",llTran_id);
                swDberror(NULL);
                continue;
              }
              swVdebug(2,"S1470: 删除历史交易流水成功,[tranid=%ld]",llTran_id);
              /*将处理流水移回当前表*/
              
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
                swVdebug(2,"没有找到历史处理流水,[tranid=%ld]",llTran_id);
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
                  swVdebug(0,"S1500: [错误/共享内存] 追加处理流水出错,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                  ilFlag = -1;
                  break;
                }
                swVdebug(2,"S1510: 追加处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                EXEC SQL DELETE FROM swt_his_proc_log WHERE tran_id = :llTran_id AND proc_step = :ilProc_step;
                if (sqlca.sqlcode)
                {
                  EXEC SQL ROLLBACK WORK;
                  swVdebug(0,"S1520: DELETE FROM swt_his_proc_log WHERE ... error");
                  swDberror(NULL);
                  ilFlag = -1;
                  break;
                }
                swVdebug(2,"S1530: 删除历史处理流水成功,[tranid=%ld,step=%d]",llTran_id,ilProc_step);
                ilFlag++;
              }
              if (ilFlag < 0) continue;
              swVdebug(2,"S1540: 交易流水和处理流水从历史移到当前完成");


              /* 将SAF记录由历史表移回当前表 */
              sgSwt_rev_saf.saf_status[0] = '0';
              sgSwt_rev_saf.saf_overtime = llCurrenttime + sgSwt_sys_queue.rev_overtime;
              sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;

#ifdef DB_INFORMIX
              EXEC SQL BEGIN WORK;
#endif
              EXEC SQL update swt_rev_saf set saf_status='0',saf_overtime=:sgSwt_rev_saf.saf_overtime,saf_num=:sgSwt_rev_saf.saf_num where saf_id=:llSaf_id;
              if(sqlca.sqlcode)
              {
                swVdebug(0,"S1550: [错误/数据库] update历史SAF记录出错,[safid=%ld]",llSaf_id);
                EXEC SQL ROLLBACK WORK;                
                continue;
              }
              swVdebug(2,"S1560: update历史SAF记录成功,[safid=%ld]",llSaf_id);  
              EXEC SQL COMMIT WORK;
            }    
            break;
#endif /*support database,Database Deprivation Project 2014-12*/                        
          default:
            /* 收到未知命令报文 */
            swVdebug(2,"S1570: 未知命令报文");
            _swMsgsend(304007,(char *)&slMsgpack);
            break;
        }
        break;
      
      case iMSGAPP: 
        llTran_id = slMsgpack.sMsghead.lTranid;
        ilProc_step = slMsgpack.sMsghead.iTranstep;
	llSaf_id = slMsgpack.sMsghead.lSafid;

	/* 收到SAF响应报文 */
        if(slMsgpack.sMsghead.cSafflag == cTRUE)
        {
          swVdebug(2,"S1580: 收到应用SAF响应报文,[tranid=%ld,procstep=%d,safid=%ld]",llTran_id,ilProc_step,llSaf_id);

          /*modify by cjh 20150114*/
		  ilRc = swShmselect_swt_rev_saf(llSaf_id,&sgSwt_rev_saf);
          /*modify by cjh 20150114 end*/
          if(ilRc == FAIL)
          {
            swVdebug(0,"S1590: [错误/共享内存] 查找SAF记录失败,[safid=%ld]",llSaf_id);
            continue;
          }
          if(ilRc == SHMNOTFOUND)
          {
            swVdebug(2,"S1600: SAF记录没有找到,[safid=%ld]",llSaf_id);
            continue;
          }
          if(sgSwt_rev_saf.saf_flag[0] != '1')
          {
            swVdebug(2,"S1610: SAF记录状态不为正在SAF,[safid=%ld]",llSaf_id);
            continue;
          }
          swVdebug(2,"S1620: 找到SAF记录,并且状态为正在SAF,[safid=%ld]",llSaf_id);
		  /*modify by cjh 20150114*/
          ilRc = swShmdelete_swt_rev_saf(llSaf_id);
          /*modify by cjh 20150114 end*/
          if(ilRc == FAIL)
            swVdebug(0,"S1630: [错误/共享内存] 删除SAF记录失败,[safid=%d]",llSaf_id);
          if(ilRc == SHMNOTFOUND)
            swVdebug(2,"S1640: SAF记录没有找到,[safid=%d]",llSaf_id);
          swVdebug(2,"S1650: 删除SAF记录成功,[safid=%d]",llSaf_id);
          continue;
        }
        else
        {
          /* 收到未知报文 */
          swVdebug(2,"S1660: 收到未知报文");
          _swMsgsend(304007,(char *)&slMsgpack);
          break;
        }

      default:
        /* 收到未知报文 */
        swVdebug(2,"S1670: 收到未知报文");
        _swMsgsend(304007,(char *)&slMsgpack);
        break;
    }
  }
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
  
  swVdebug(2,"S1680: [swMonrev]已经停止...");
  swDbclose();
  ilRc=qdetach();
  if(ilRc)
  {
    swVdebug(0,"S1690: [错误/邮箱] qdetach()函数,错误码=%d",ilRc);
    exit(FAIL);
  }
  exit(sig);
}

/**************************************************************
 ** 函数名      ： swRevSuccess
 ** 功  能      ： 检查冲正报文成功与否
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/11/29
 ** 最后修改日期： 1999/11/29
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： msgpack ：交易报文
 ** 返回值      ： SUCCESS, FAIL
***************************************************************/
int swRevSuccess(struct msgpack msgpack)
{
  char  alResult[iFLDVALUELEN + 1];  /* 表达式计算结果 */
  short ilLength;                    /* 表达式计算结果长度 */

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
  swVdebug(4,"S1700: [函数调用] swRevSuccess()");

  ilQid = msgpack.sMsghead.iOrg_q;
  ilRc = swShmselect_swt_sys_queue(ilQid,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1710: [错误/共享内存] swShmselect_swt_sys_queue():FAIL");
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1720: 端口没有设置,[qid=%d]",ilQid);
    return(FAIL);
  }  
  
  ilRc = swFmlunpack(msgpack.aMsgbody,msgpack.sMsghead.iBodylen,psgPreunpackbuf);
  if( ilRc == FAIL )
  {
    swVdebug(0,"S1730: [错误/函数调用] swFmlunpack()函数,返回码=%d",ilRc);
    _swMsgsend( 302010, NULL ); 
    return(FAIL);
  }
  
  memcpy(alResu_revexp,sgSwt_sys_queue.resu_revexp,sizeof(alResu_revexp));
  if(alResu_revexp[0] == '\0')
  {
    swVdebug(2,"S1740: 冲正响应报文返回结果正确");
    swVdebug(4,"S1750: [函数返回] swRevSuccess()返回码=0");
    return(SUCCESS);
  }
  if(_swExpress(alResu_revexp, alResult,&ilLength)==FAIL)
  {
    swVdebug(0,"S1760: [错误/函数调用] _swExpress()函数,返回码=-1");
    _swMsgsend(777777,NULL);
    return(FAIL);
  }

  if(strcmp(alResult,"1"))
  {
    swVdebug(2,"S1770: 冲正响应报文返回结果错误");
    return(FAIL);
  }
  else
  {
    swVdebug(2,"S1780: 冲正响应报文返回结果成功");
    swVdebug(4,"S1790: [函数返回] swRevSuccess()返回码=0");
    return(SUCCESS);
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
int swProcess_proclog(long lTran_id,short iProc_step,char aLoc_flag)
{
  swVdebug(4,"S1800: [函数调用] swProcess_proclog(%ld,%d)",lTran_id,iProc_step);
  /* 置处理流水状态为冲正结束 */
  pslSwt_proc_log[i].proc_status = 4;
  ilRc = swShmupdate_swt_proc_log(lTran_id,iProc_step,pslSwt_proc_log[i],aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1810: [错误/共享内存] 更新处理流水失败,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1820: 处理流水没有找到,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    return (FAIL);
  }
  swVdebug(2,"S1830: 置处理流水状态,更新处理流水成功,[tranid=%ld,step=%d]",lTran_id,iProc_step);

#ifdef DB_SUPPORT /*support database,Database Deprivation Project 2014-12*/

#ifdef DB_INFORMIX
  EXEC SQL BEGIN WORK;
#endif
  /* 处理流水移历史 */
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
    swVdebug(0,"S1840: [错误/数据库] 追加历史处理流水失败,[tranid=%ld,step=%d]",lTran_id,iProc_step);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
  swVdebug(2,"S1850: 追加历史处理流水成功,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#else
  sw_put_proclog(&pslSwt_proc_log[i]);
#endif
 
  /* 删除处理流水 */
  ilRc = swShmdelete_swt_proc_log(lTran_id,iProc_step,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1860: [错误/共享内存] 删除处理流水失败,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1870: 处理流水没有找到,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S1880: 删除处理流水成功,[tranid=%ld,step=%d]",lTran_id,iProc_step);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif

  swVdebug(4,"S1890: [函数返回] swProcess_proclog()返回码=0");
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
int swProcess_revend_tranlog(long lTran_id,short iStatus,char aLoc_flag,struct swt_proc_log sSwt_proc_log,struct msgpack sMsgpack)
{
  /* ADD BY GXZ 2003.02.23 */
  struct msgpack sMsgpack1;
  short  iMsglen;
  union  preunpackbuf puPrebuf[iBUFFLDNUM];
  union  preunpackbuf puPrebuf1[iBUFFLDNUM];
  /**/
    
  swVdebug(4,"S1900: [函数调用] swProcess_revend_tranlog(%ld,%d)",lTran_id,iStatus);

  /* ADD BY GXZ 2003.02.23  Begin*/
  /* 取得报文 */
  memset((char *)&sMsgpack1,0x00,sizeof(sMsgpack1));
  _swTrim(sSwt_proc_log.org_file);
  ilRc = swGetlog(sSwt_proc_log.org_file,sSwt_proc_log.org_len,sSwt_proc_log.org_offset,(char *)&sMsgpack1);
  if(ilRc == FAIL)  
  {
    swVdebug(0,"S0430: [错误/其它] 取处理流水报文失败[%s]",sSwt_proc_log.org_file);
  }
  else
    swVdebug(2,"S0440: 取处理流水报文成功[%s]",sSwt_proc_log.org_file);
 
   /* 当前步骤原报文预解包 */
  ilRc = swFmlunpack(sMsgpack1.aMsgbody,sMsgpack1.sMsghead.iBodylen,puPrebuf);
  if(ilRc)
  {
    swVdebug(0,"S0450: swFmlunpack() failed");
  }

  /* 响应报文预解包 */
  ilRc = swFmlunpack(sMsgpack.aMsgbody,sMsgpack.sMsghead.iBodylen,puPrebuf1);
  if(ilRc)
  {
    swVdebug(0,"S0470: swFmlunpack() failed");
  }

  /* 报文并集 */ 
  ilRc = _swFmlunion(puPrebuf,puPrebuf1);
  if(ilRc)
  {
    swVdebug(0,"S0480: _swFmlunion() failed");
  }
  else
    swVdebug(2,"S0500: 报文并集成功");

  /* 并集后的报文打包 */
  ilRc = swFmlpack(puPrebuf,sMsgpack.aMsgbody,&iMsglen);
  if(ilRc)
  {
    swVdebug(0,"S0510: swFmlpack() failed");
  }
  sMsgpack.sMsghead.iBodylen = iMsglen;
  iMsglen = sizeof(struct msghead) + iMsglen;
  swVdebug(2,"S0520: 组织冲正报文成功");
        
  /* ADD BY GXZ 2003.02.23 End */
  
  /* 更新交易流水状态 */
  ilRc = swShmupdate_setstat_tranlog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1910: [错误/共享内存] 更新交易流水出错,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1920: 交易流水没有找到,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S1930: 置交易流水状态,更新交易流水成功,[tranid=%ld]",lTran_id);
  
  /* 交易流水移到历史 */
  ilRc = swShmselect_swt_tran_log(lTran_id,&sgSwt_tran_log,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1940: [错误/共享内存] 查找交易流水出错,[tranid=%ld]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1950: 交易流水没有找到,[tranid=%ld]",lTran_id);
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
    swVdebug(0,"S1960: [错误/数据库] 追加历史交易流水失败,[tranid=%ld]",lTran_id);
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return (FAIL);
  }
/*add by zcd 20141225*/
#else
  sw_put_tranlog(&sgSwt_tran_log);
#endif
  swVdebug(2,"S1970: 追加历史交易流水成功,[tranid=%ld]",lTran_id);
/*end of add by zcd 20141225*/

  /* 删除交易流水 */
  ilRc = swShmdelete_swt_tran_log(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S1980: [错误/共享内存] 删除交易流水出错,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S1990: 交易流水没有找到,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S2000: 删除交易流水成功,[tranid=%ld]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
  sMsgpack.sMsghead.iOrg_q = 999;
  _swMsgsend(99999,(char *)&sMsgpack);    
  swVdebug(4,"S2010: [函数返回] swProcess_revend_tranlog()返回码=0");
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
  swVdebug(4,"S2020: [函数调用] swProcess_revend_proclog(%ld,%d)",lTran_id,iStatus);

  /* 更新处理流水状态 */
  ilRc = swShmupdate_setstat_proclog(lTran_id,iStatus,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2030: [错误/共享内存] 更新处理流水出错,[tranid=%ld,step=*]",lTran_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2040: 处理流水没有找到,[tranid=%ld,step=*]",lTran_id);
    return (FAIL);
  }
  swVdebug(2,"S2050: 置处理流水状态,更新交易流水成功,[tranid=%ld,step=*]",lTran_id);
  
  /* 处理流水移到历史 */
  ilRc = swShmselect_swt_proc_log_mrec(lTran_id,pslSwt_proc_log,&ilCount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2060: [错误/共享内存] 查找处理流水出错,[tranid=%ld,step=*]",lTran_id);
    return (FAIL);
  }  
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2070: 处理流水没有找到,[tranid=%ld,step=*]",lTran_id);
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
      swVdebug(0,"S2080: [错误/数据库] 追加历史处理流水失败,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return (FAIL);
    }
/*ad by zcd 20141225*/
#else
    sw_put_proclog(&pslSwt_proc_log[i]);
#endif /*support database,Database Deprivation Project 2014-12*/ 
  /*end of add by zcd 20141225*/
     swVdebug(2,"S2090: 追加历史处理流水成功,[tranid=%ld,step=%d]",lTran_id,slSwt_proc_log.proc_step);
  }  
 
  /* 删除处理流水 */
  ilRc = swShmdelete_swt_proc_log_mrec(lTran_id,aLoc_flag);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2100: [错误/共享内存] 删除处理流水失败,[tranid=%ld,step=*]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/ 
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2110: 处理流水没有找到,[tranid=%ld,step=*]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/ 
    EXEC SQL ROLLBACK WORK;
#endif
    return (FAIL);
  }
  swVdebug(2,"S2120: 删除处理流水成功,[tranid=%ld,step=*]",lTran_id);
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
#endif
   
  swVdebug(4,"S2130: [函数返回] swProcess_revend_proclog()返回码=0");
  return(SUCCESS);
}


/**************************************************************
 ** 函数名      ： swProcess_revfail_revsaf
 ** 功  能      ： 置某笔SAF流水状态为SAF失败,
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
  swVdebug(4,"S2140: [函数调用] swProcess_revfail_revsaf(%ld)",lSaf_id);
  /* 更新SAF记录状态 */
  sgSwt_rev_saf.saf_status[0] = '2';	/* SAF失败 */
  /*modify by cjh 20150114*/
  ilRc = swShmupdate_swt_rev_saf(lSaf_id,sgSwt_rev_saf);
  /*modify by cjh 20150114 end */
  if(ilRc == FAIL)
  {
    swVdebug(0,"S2150: [错误/共享内存] 更新SAF记录出错,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(2,"S2160: SAF记录没有找到,[safid=%ld]",lSaf_id);
    return (FAIL);
  }
  swVdebug(2,"S2170: 置SAF记录状态为SAF失败,[safid=%ld]",lSaf_id);
  swVdebug(2,"S2180: 更新SAF记录成功,[safid=%ld]",lSaf_id);
 
  swVdebug(4,"S2190: [函数返回] swProcess_revfail_revsaf()返回码=0");
  return(SUCCESS);
}

/*begin add by gf at 2004-04-07*/
/**************************************************************
 ** 函数名      ： swSendRevresult_union
 ** 功  能      ： 发送冲正结果给源发端口(需要报文并集)
 **             :  
 ** 作  者      ：  
 ** 建立日期    ： 1999/11/19
 ** 最后修改日期： 1999/11/19
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-成功 / 1-失败
***************************************************************/
int swSendRevresult_union(long lTranid,struct msgpack sMsgpack,short iMsgtype,char cLoc_flag)
{
  short ilRc;
  unsigned int ilMsglen; /*changed by dyw, PSBC_V1.0*/
  struct msgpack slMsgpack1;
  union  preunpackbuf pulPrebuf1[iBUFFLDNUM];
  union   preunpackbuf pulPreunpackbuf[iBUFFLDNUM];
  
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
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
    char  xastatus[21];           /* 记录XA状态 */
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
    char  xastatus[21];           /* 记录XA状态 */
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
    char  xastatus[21];           /* 记录XA状态 */
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
    swVdebug(0,"S0430: 历史表中无此交易流水,[tranid=%ld]",llTran_id1);
    return(FAIL);
  }
  else if(sqlca.sqlcode)
  {
    swVdebug(0,"S0440: [错误/数据库] 查找历史交易流水出错,[tranid=%ld]",llTran_id1);
    return(FAIL);
  }
#endif

  swVdebug(2,"S0450: 找到历史交易流水,[tranid=%ld]",llTran_id1);
  memset((char *)&slMsgpack1,0x00,sizeof(slMsgpack1));
  _swTrim(slTranlog.resp_file);
  ilRc = swGetlog(slTranlog.resp_file,slTranlog.resp_len,
  slTranlog.resp_offset,(char *)&slMsgpack1);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0630: [错误/其它] 取交易流水响应报文失败");
    return(FAIL);
  }
  swVdebug(2,"S0630: 取原始报文成功");
   
  /*报文并集*/
  swFmlunpack(sMsgpack.aMsgbody,sMsgpack.sMsghead.iBodylen,pulPrebuf1);
  swFmlunpack(slMsgpack1.aMsgbody,slMsgpack1.sMsghead.iBodylen,pulPreunpackbuf);
      
  /* 将原始报文与当前报文进行并集操作 */
  _swFmlunion(pulPreunpackbuf,pulPrebuf1);
  swFmlpack(pulPreunpackbuf,sMsgpack.aMsgbody,&ilMsglen);
  sMsgpack.sMsghead.iBodylen = ilMsglen;
  ilMsglen = sizeof(struct msghead) + ilMsglen;
  swVdebug(2,"S0640: 报文并集成功");  
   
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


 /**************************************************************
 ** 函数名      ： swSendRevresult_nounion
 ** 功  能      ： 发送冲正结果给源发端口(不需要报文并集)
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
