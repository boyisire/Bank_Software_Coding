/****************************************************************/
/* 模块编号    ：SWAPI                                          */
/* 模块名称    ：前置程序API函数                                */
/* 版 本 号    ：V4.3.0                                         */
/* 作	 者    ：张辉                                           */
/* 建立日期    ：1999/11/22                                     */
/* 最后修改日期：2001/4/6                                       */
/* 模块用途    ：前置程序所用的函数                             */
/* 本模块中包含如下函数及功能说明：                             */
/*			 （1）int swNewtran();                  */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/****************************************************************/
/* UNIX系统定义 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <varargs.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

/*
#include "p_entry.h"
#include "p_return.h"
#include "p_symbol.h"
#include "p_typecl.h"
#include "p_msg.h"
*/

/* switch定义 */
#include "swConstant.h"
#include "swapisource.h"

/* 数据库定义 */
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
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
#else
#include "swDbstruct.h"
#endif

#include "swShm.h"
#include "swPubfun.h"

extern short swPreunpack(struct msgpack *psMsgpack);

/* added by fzj at 2002.01.23, begin */
extern char agMacbuf[iMSGMAXLEN];	/* MAC buffer */
extern short igMacbuflen;		/* MAC length */
/* added by fzj at 2002.01.23, end */

/***********************************************************************
 ** 函数名:       int swNewtran(struct msghead *) 生成新交易包函数
 ** 功  能:       生成新发交易的报文头，按照各个端口的性质生成报文头 
 ** 作  者:
 ** 建立日期:     2000.01.26
 ** 最后修改日期: 2000.01.26
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义:     struct msghead *psMsghead: 指向报文头的指针
 ** 返回值:       成功返回 0(SUCCESS), 
		  失败(取内部交易流水号时，对共享内存操作不成功)返回  -1(FAIL) 
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/
int swNewtran(struct msghead *psMsghead)
{
  short ilRc;
  long ilTranid;

  if (swShmisfull()) return(-2);
  
  /* 生成交易报文头 */
  memset(psMsghead,0x00,sizeof(struct msghead));
  psMsghead->cRouter = '1';
  psMsghead->cFormatter = '0';
  psMsghead->iMsgtype = 1;
  psMsghead->iTranstep = 1;
  psMsghead->iMsgformat = sgPortattrib.iMsgformat;
  psMsghead->iOrg_q = sgPortattrib.iQid;
  psMsghead->iDes_q = sgPortattrib.iQid;
  psMsghead->iBegin_q_id = sgPortattrib.iQid;
  psMsghead->aSysattr[0]='0'; /* add by nh 20020910 */
  psMsghead->call_flag = '0';/*add by baiqj20150319 ,PSBC_V1.0*/
  psMsghead->callstart.time = 0;
  psMsghead->callstart.millitm = 0;
  memset(psMsghead->timeconsum,0x00,sizeof(psMsghead->timeconsum));/*add by baiqj20150319,PSBC_V1.0*/

  time(&(psMsghead->lBegintime));
  
  /* 操作共享内存，产生新的交易流水号 */
  ilRc = swShmgettranid(&ilTranid);
  if(ilRc == FAIL) return(FAIL);
  psMsghead->lTranid = ilTranid;

  return(SUCCESS);
}

/***  下面连续五个为邮箱操作函数 ***/

/***********************************************************************
 ** 函数名:       int swMbopen( short qid ) 邮箱打开函数
 ** 功  能:       打开指定邮箱 
 ** 作  者:
 ** 建立日期:     2000.01.26
 ** 最后修改日期: 2000.01.26
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义:     short int qid: 所要打开的端口邮箱号
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/

int swMbopen( short qid )
{
  short ilRc ;

  ilRc = qattach( qid );
  return( ilRc );
}

/***********************************************************************
 ** 函数名:       int swMbclose() 邮箱关闭函数
 ** 功  能:       关闭邮箱 
 ** 作  者:
 ** 建立日期:     2000.01.26
 ** 最后修改日期: 2000.01.26
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义:     
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号 
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/

int swMbclose( )
{
  short ilRc ;

  ilRc = qdetach( );
  return( ilRc );
}

/***********************************************************************
 ** 函数名:       int swSendpack(short, char *, short, unsigned int, short, short ) 
		  报文发送函数
 ** 功  能:       将报文(含交换平台标准报文头)传送至指定的邮箱号 
 ** 作  者:
 ** 建立日期:     2000.01.26
 ** 最后修改日期: 2000.01.26
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义:     short qid: 传送至的信箱号 
		  char *msgpack: 要传送的报文内容(含交换平台标准报文头)
		  short priority: 优先级 
		  short msglen: 报文长度  
		  short class: 报文 Class 
		  short type: 报文 Type 
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/
/*modify by zcd 20141229
int swSendpack(short qid, char *msgpack, short msglen, short priority, 
	       short class, short type)*/
int swSendpack(short qid, char *msgpack, unsigned int msglen, short priority, 
	       short class, short type)
{
  short  ilRc;
  short ilOther_id1;
  short ilOther_grp1;
  short ilOther_q1;
 
  /* 发送报文 */
  swVdebug(4,"S0010: [函数调用] swSendpack()函数");
  if (qid<1000)
  {
     /* 本地邮箱 */
     ilRc= _qwrite2( msgpack, msglen, qid, priority, class, type );
  }
  else
  {
     ilOther_id1= qid;
     
    ilRc = swShmselect_swt_sys_other(ilOther_id1, &sgSwt_sys_other);
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(0,"S0020: [错误/共享内存] 外部节点[%d]没有设置",ilOther_id1);
      return(FAIL);
    }
    ilOther_grp1 = sgSwt_sys_other.other_grp;
    ilOther_q1 = sgSwt_sys_other.other_q;
    
    ilRc= qwrite3(msgpack,msglen,ilOther_q1,ilOther_grp1,priority,class,type);
  }
  swVdebug(4,"S0030: [函数返回] swSendpack()函数,返回码=%d",ilRc);
  return(ilRc);
}

/***********************************************************************
 ** 函数名:       int swRecvpackw 报文阻塞接受函数
 ** 功  能:       以阻塞方式从当前邮箱接受报文 
 ** 作  者:
 ** 建立日期:     2000.01.26
 ** 最后修改日期: 2000.01.26
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义: short *qid : 返回报文发送的源邮箱 
	      char *msgpack: 要传送的报文内容(含交换平台标准报文头)
	      short *msglen: 收到的报文长度
	      short *priority: 收到的报文优先级 
	      short *class: 执行前作为报文 Class过滤条件，执行后返回报文 class
              short *type: 执行前作为报文 class 过滤条件，执行后返回报文 class 
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/
/*modiby by zcd 20141229
int swRecvpackw( short *qid, char *msgpack, short *msglen, short *priority,
		 short *class, short *type)*/
int swRecvpackw( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 short *class, short *type)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id2;
  short ilOther_grp2;
  short ilOther_q2;
  
  swVdebug(4,"S0040: [函数调用] swRecvpackw()函数");
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* 以阻塞方式读取邮箱 */ 
  //swVdebug(0,"########msglen is:%d\n",*msglen);
  ilRc = qread3((char *)msgpack,msglen,qid,&ilOther_grp2,priority,class,type);
  //swVdebug(0,"############after msglen is:%d\n",*msglen);
  //swVdebug(0,"############after strlen is:%s\n",strlen(msgpack));
  if (ilRc) return( ilRc );
  if (ilOther_grp2 == ilLocal_grp)
  {
    /* 本地邮箱发来记录 */
    swVdebug(4,"S0050: [函数返回] swRecvpackw()函数,返回码=0");
    return(SUCCESS);
  }
  ilOther_q2 = *qid;
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp2, ilOther_q2, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0060: [错误/共享内存] 外部节点[%d,%d]没有设置",ilOther_grp2,ilOther_q2);
    return(FAIL);
  }  
  ilOther_id2 = sgSwt_sys_other.other_id;
  
  /* 邮箱号采用外部节点号 */
  *qid = ilOther_id2;
  swVdebug(4,"S0070: [函数返回] swRecvpackw()函数,返回码=0");
  return(SUCCESS);
}

/***********************************************************************
 ** 函数名:       int swRecvpack 报文非阻塞接受函数
 ** 功  能:       以非阻塞方式从当前邮箱接受报文级连 
 ** 作  者:
 ** 建立日期:     2000.01.26
 ** 最后修改日期: 2000.01.26
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义: short *qid : 返回报文发送的源邮箱 
	      char *msgpack: 要传送的报文内容(含交换平台标准报文头)
	      short *msglen: 收到的报文长度
	      short *priority: 收到的报文优先级 
	      short *class: 执行前作为报文 Class过滤条件，执行后返回报文 class
              short *type: 执行前作为报文 type 过滤条件，执行后返回报文 type 
	      short timee: 接受超时时间 
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/
/*modify by zcd 20141229
int swRecvpack( short *qid, char *msgpack, short *msglen, short *priority,
		 short *class, short *type, short time)*/
int swRecvpack( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 short *class, short *type, short time)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id;
  short ilOther_grp;
  short ilOther_q;
   
  swVdebug(4,"S0080: [函数调用] swRecvpackw()函数"); 
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* 以非阻塞方式读取邮箱 */
  ilRc = qread3nw( msgpack, msglen, qid, &ilOther_grp,priority, class,  
		      type, time );
  if ( ilRc != SUCCESS )
  {
    swVdebug(4,"S0090: [函数返回] swRecvpack()函数,返回码=0");
    return( ilRc );
  }
  if (ilOther_grp == ilLocal_grp)
  {
    /* 本地邮箱发来记录 */
    swVdebug(4,"S0100: [函数返回] swRecvpack()函数,返回码=0");
    return(SUCCESS);
  }
  ilOther_q = *qid;
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp, ilOther_q, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0110: [错误/共享内存] 外部节点没有设置");
    return(FAIL);
  }  
  ilOther_id = sgSwt_sys_other.other_id;
    
  /* 邮箱号采用外部节点号 */
  *qid = ilOther_id;
  swVdebug(4,"S0120: [函数返回] swRecvpack()函数,返回码=0");
  return(SUCCESS);
}

/***********************************************************************
 ** 函数名:       int swSendpackl(short, char *, short, short, long, long ) 
		  报文发送函数
 ** 功  能:       将报文(含交换平台标准报文头)传送至指定的邮箱号 
 ** 作  者:
 ** 建立日期:     2001.11.1
 ** 最后修改日期: 
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义:     short qid: 传送至的信箱号 
		  char *msgpack: 要传送的报文内容(含交换平台标准报文头)
		  short priority: 优先级 
		  short msglen: 报文长度  
		  class: 报文 Class 
		  long type: 报文 Type 
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/
/*modify by zcd 20141229*/
int swSendpackl(short qid, char *msgpack, unsigned int msglen, short priority, 
	       long class, long type)
{
  short  ilRc;
  short ilOther_id1;
  short ilOther_grp1;
  short ilOther_q1;
   	
  /* 发送报文 */
  if (qid<1000)
  {
     /* 本地邮箱 */
     ilRc = bmqPut(0,qid,priority,type,class,msgpack, msglen);
  }
  else
  {
     ilOther_id1= qid;
     
    ilRc = swShmselect_swt_sys_other(ilOther_id1, &sgSwt_sys_other);
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(0,"S0130: [错误/共享内存] 外部节点[%d]没有设置",ilOther_id1);
      return(FAIL);
    }
    ilOther_grp1 = sgSwt_sys_other.other_grp;
    ilOther_q1 = sgSwt_sys_other.other_q;
    ilRc = bmqPut(ilOther_grp1,ilOther_q1,priority,type,class,msgpack,msglen);  
  }
  return(ilRc);
}


/***********************************************************************
 ** 函数名:       int swRecvpacklw 报文阻塞接受函数
 ** 功  能:       以阻塞方式从当前邮箱接受报文 
 ** 作  者:
 ** 建立日期:     2000.11.1
 ** 最后修改日期: 
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义: short *qid : 返回报文发送的源邮箱 
	      char *msgpack: 要传送的报文内容(含交换平台标准报文头)
	      short *msglen: 收到的报文长度
	      short *priority: 收到的报文优先级 
	      long *class: 执行前作为报文 Class过滤条件，执行后返回报文 class
              long *type: 执行前作为报文 class 过滤条件，执行后返回报文 class 
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/
/*modify by zcd 20141229

int swRecvpacklw( short *qid, char *msgpack, short *msglen, short *priority,
		 long *class, long *type)*/
int swRecvpacklw( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 long *class, long *type)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id2;
  short ilOther_grp2=0;
  short ilOther_q2;
  
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* 以阻塞方式读取邮箱 */ 
  ilRc = bmqGetw(&ilOther_grp2,qid,priority,type,class,(char *)msgpack,msglen,0);
  if (ilRc) return( ilRc );
  if (ilOther_grp2 == ilLocal_grp)
  {
    /* 本地邮箱发来记录 */
    return(SUCCESS);
  }
  ilOther_q2 = *qid;              
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp2, ilOther_q2, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0140: [错误/共享内存] 外部节点[%d,%d]没有设置",ilOther_grp2,ilOther_q2);
    return(FAIL);
  }  
  ilOther_id2 = sgSwt_sys_other.other_id;
  
  /* 邮箱号采用外部节点号 */
   *qid = ilOther_id2; 
  return(SUCCESS);
}

/***********************************************************************
 ** 函数名:       int swRecvpackl 报文非阻塞接受函数
 ** 功  能:       以非阻塞方式从当前邮箱接受报文级连 
 ** 作  者:
 ** 建立日期:     2001.11.1
 ** 最后修改日期: 
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义: short *qid : 返回报文发送的源邮箱 
	      char *msgpack: 要传送的报文内容(含交换平台标准报文头)
	      short *msglen: 收到的报文长度
	      short *priority: 收到的报文优先级 
	      long *class: 执行前作为报文 Class过滤条件，执行后返回报文 class
              long *type: 执行前作为报文 type 过滤条件，执行后返回报文 type 
	      short timee: 接受超时时间 
 ** 返回值:       成功返回 0(SUCCESS), 失败返回 MessageQ 内部错误号
*************************************************************************
 ** 修改记录 ** 
*************************************************************************/
/*modify by zcd 20141229
int swRecvpackl( short *qid, char *msgpack, short *msglen, short *priority,
		 long *class, long *type, short time)*/

int swRecvpackl( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 long *class, long *type, short time)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id;
  short ilOther_grp;
  short ilOther_q;
    
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* 以非阻塞方式读取邮箱 */
  if (time == 0)
    ilRc = bmqGet(&ilOther_grp,qid,priority,type,class,(char *)msgpack,msglen);
  else
    ilRc = bmqGetw(&ilOther_grp,qid,priority,type,class,(char *)msgpack,msglen,time);

  if ( ilRc != SUCCESS ) return( ilRc );

  if (ilOther_grp == ilLocal_grp)
  {
    /* 本地邮箱发来记录 */
    return(SUCCESS);
  }
  ilOther_q = *qid;
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp, ilOther_q, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0150: [错误/共享内存] 外部节点没有设置");
    return(FAIL);
  }  
  ilOther_id = sgSwt_sys_other.other_id;
    
  /* 邮箱号采用外部节点号 */
  *qid = ilOther_id;
  return(SUCCESS);
}


/************************************************************************
 ** 函数名:       int swTranrev ( long	tran_id )
 ** 功  能:       停止交易正常步骤，发送冲正请求报文至事务管理的事务处理
                  自带邮箱(iMBMONREV)。 如果报文类型为冲正请求报文，则所
                  发报文为停止冲正报文；否则发送即时冲正报文。
 ** 作  者:       路长胜
 ** 建立日期:     2000.01.27
 ** 最后修改日期: 2000.01.27
 ** 调用其它函数: swMsgsend (), qwrite (), swMberror (), swDberror ()
 ** 全局变量:     无
 ** 参数含义:     tran_id -- 交换平台内部流水号
 ** 返回值:       成功返回 0 (SUCCESS)，失败返回 -1 (FAIL)
 ************************************************************************
 ** 修改记录 ** 
*************************************************************************/
int swTranrev(long tran_id)
{
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  llTranid;
    sqlint64  llTime;
    short ilTran_status;
    short ilTmp1;
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llTranid;
    long  llTime;
    short ilTran_status;
    short ilTmp1;
  EXEC SQL END DECLARE SECTION;
#endif
#else
  long  llTranid;
  long  llTime;
  short ilTran_status;
  short ilTmp1;
#endif

  int ilRc, ilMsglen = 0;
  struct msgpack slMsgpack;
  
  swVdebug(4,"S0160: [函数调用] swTranrev(%ld)函数",tran_id);

  memset ((char*)&slMsgpack, 0, sizeof (struct msgpack));

  llTranid = tran_id;

  /* 查流水状态,从SHM和DB中 */
  #ifdef DB_SUPPORT
  ilRc = swShmselect_swt_tran_log(llTranid, &sgSwt_tran_log,'a');
  #else
  ilRc = swShmselect_swt_tran_log(llTranid, &sgSwt_tran_log,'0'); /*2014-12-16*/
  #endif
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0170: [错误/共享内存] 查找此交易流水[%ld]时出错",llTranid);
    return(FAIL);
  }
  
  if ( ilRc == SHMNOTFOUND ) 
  {
    #ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL SELECT tran_status INTO :ilTran_status:ilTmp1 FROM swt_his_tran_log
      WHERE tran_id = :llTranid;
    if (sqlca.sqlcode == SQLNOTFOUND)
    {
      swVdebug(0,"S0180: [错误/数据库] sqlcode=%ld",sqlca.sqlcode);
#ifdef TRANREVLOG
      time(&llTime);      
      EXEC SQL INSERT INTO swt_tranrev_log VALUES(:llTranid,:llTime);
      if(sqlca.sqlcode)
      {
        if (!(sqlca.sqlcode==SQLDUPKEY||sqlca.sqlcode==SQLDUPKEY2||sqlca.sqlcode==SQLDUPKEY3))
        { 
          swVdebug(0,"S0185: insert into swt_tran_rev_log failed, sqlcode=%d",sqlca.sqlcode);
        }  
      }  
#endif      
      return FAIL;
    }
    else if(sqlca.sqlcode)
    {
      swDberror ((char *)&slMsgpack);
      return FAIL;
    }
    else
    {
      if (ilTran_status == iTRNREVEND) 
      {
        swVdebug(4,"S0190: [函数返回] swTranrev()函数,返回码=0");
        return(SUCCESS);
      }
    }
	#endif
  }
  else 
  {
    ilTran_status = sgSwt_tran_log.tran_status;
    
    if (ilTran_status == iTRNREVEND) 
    {
      swVdebug(4,"S0200: [函数返回] swTranrev()函数,返回码=0");
      return(SUCCESS);
    }
  }

  swVdebug(2,"S0210: ilTran_status=%d",ilTran_status);

  slMsgpack.sMsghead.lTranid = tran_id;
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.aSysattr[0] = '0';
  ilMsglen = sizeof (struct msghead);

  switch ( ilTran_status ) {
    case iTRNING:
    case iTRNSAF:
    case iTRNOVERTIME:
    case iTRNREVFAIL:
    case iTRNEND:
      slMsgpack.sMsghead.lCode = 601;
      break;
    case iTRNREVING:
/* delete by szhengye 2000.3.20
      slMsgpack.sMsghead.lCode = 602;
delete by szhengye 2000.3.20 */
      swVdebug(4,"S0220: [函数返回] swTranrev()函数,返回码=0");
      return SUCCESS;
      break;
    default:
      swVdebug(4,"S0230: [函数返回] swTranrev()函数,返回码=0");
      return SUCCESS;
      break;
  }

  /* send command */
  ilRc = qwrite ((char *)&slMsgpack, ilMsglen, iMBMONREV);
  if(ilRc) {
    swMberror (ilRc, (char *)&slMsgpack);
    return FAIL;
  }
  swVdebug(4,"S0240: [函数返回] swTranrev()函数,返回码=0");
  return SUCCESS;
}

/************************************************************************
 ** 函数名:       int swRevbykey(int iQid, char *aKey)
 ** 功  能:       停止交易正常步骤，发送冲正请求报文至事务管理的事务处理
                  自带邮箱(iMBMONREV)。 如果报文类型为冲正请求报文，则所
                  发报文为停止冲正报文；否则发送即时冲正报文。
 ** 作  者:       fzj
 ** 建立日期:     2000.01.27
 ** 最后修改日期: 2000.01.27
 ** 调用其它函数: swMsgsend (), qwrite (), swMberror (), swDberror ()
 ** 全局变量:     无
 ** 参数含义:     tran_id -- 交换平台内部流水号
 ** 返回值:       成功返回 0 (SUCCESS)，失败返回 -1 (FAIL)
 ************************************************************************
 ** 修改记录 ** 
*************************************************************************/
int swRevbykey(int iQid, char *aKey)
{
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64 llTran_id
    short ilQid;
    char  alKey[151];
    short ilTran_stat;
    short ilTmp2;
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llTran_id;
    short ilQid;
    char  alKey[151];
    short ilTran_stat;
    short ilTmp2;
  EXEC SQL END DECLARE SECTION;
#endif
#else
  long  llTran_id;
  short ilQid;
  char  alKey[151];
  short ilTran_stat;
  short ilTmp2;
#endif
  int ilRc, ilMsglen = 0;
  struct msgpack slMsgpack;
  
  swVdebug(4,"S0250: [函数调用] swRevbykey(%d,%s)函数",iQid,aKey);

  ilQid = iQid;
  memcpy(alKey,aKey,sizeof(alKey));
  memset ((char*)&slMsgpack, 0, sizeof (struct msgpack));

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL SELECT tran_id,tran_status INTO :llTran_id:ilTmp2,:ilTran_stat:ilTmp2
    FROM swt_his_tran_log WHERE q_tran_begin = :ilQid AND rev_key = :alKey;
  if (sqlca.sqlcode == SQLNOTFOUND)
  {
    ilRc = swShmselect_qid_revkey_tranlog(ilQid,alKey,&sgSwt_tran_log);
    /*
    ilRc = swShmselect_swt_tran_log(llTran_id, &sgSwt_tran_log);
    */
    if (ilRc == FAIL)
      swVdebug(0,"S0260: [错误/共享内存] 查找交易流水出错");
    else if (ilRc == SQLNOTFOUND)
      swVdebug(2,"S0270: [警告/共享内存] 找不到交易流水");
    return FAIL;
  }
  else if(sqlca.sqlcode) 
  {
    swDberror ((char *)&slMsgpack);
    return FAIL;
  }
  else
  {
    if (ilTran_stat == iTRNREVEND) 
    {
      swVdebug(4,"S0280: [函数返回] swRevbykey()函数,返回码=0");
      return(SUCCESS);
    }
  }
#endif

  swVdebug(2,"S0290: ilTran_id = %ld,ilTran_stat=%d",llTran_id,ilTran_stat);

  slMsgpack.sMsghead.lTranid = llTran_id;
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  ilMsglen = sizeof (struct msghead);

  switch ( ilTran_stat ) {
    case iTRNING:
    case iTRNSAF:
    case iTRNOVERTIME:
    case iTRNREVFAIL:
    case iTRNEND:
      slMsgpack.sMsghead.lCode = 601;
      break;
    case iTRNREVING:
      swVdebug(4,"S0300: [函数返回] swRevbykey()函数,返回码=0");
      return SUCCESS;
      break;
    default:
      swVdebug(4,"S0310: [函数返回] swRevbykey()函数,返回码=0");
      return SUCCESS;
      break;
  }

  /* send command */
  ilRc = qwrite ((char *)&slMsgpack, ilMsglen, iMBMONREV);
  if(ilRc) {
    swMberror (ilRc, (char *)&slMsgpack);
    return FAIL;
  }
  swVdebug(4,"S0320: [函数返回] swRevbykey()函数,返回码=0");
  return SUCCESS;
}

/* #ifdef DB_SUPPORT */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */
/************************************************************************
 ** 函数名:       int swSaf ( char * msgpack )
 ** 功  能:       将报文放入SAF表，以SAF机制进行发送。目标邮箱为报文头.iDes_q
 ** 作  者:       史正烨
 ** 建立日期:     2000.02.27
 ** 最后修改日期: 2000.02.27
 ** 调用其它函数: 
 ** 全局变量:     无
 ** 参数含义:     msgpack -- 报文内容
 ** 返回值:       成功返回 0 (SUCCESS)，失败返回 -1 (FAIL)
 ************************************************************************
 ** 修改记录 ** 
*************************************************************************/
int swSaf(char * msgpack)
{
  short ilRc;
  long llSafid;
  struct msgpack * pslMsgpack;
  /* EXEC SQL BEGIN DECLARE SECTION; */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */
    short ilDes_q;                      /* 邮箱号 */
  /* EXEC SQL END DECLARE SECTION; */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */

  swVdebug(4,"S0330: [函数调用] swSaf()函数");
  memset(&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
  pslMsgpack = (struct msgpack *)msgpack;
  ilDes_q = pslMsgpack->sMsghead.iDes_q;

  /* 读邮箱端口设置 */
  memset((char *)&sgSwt_sys_queue,0x00,sizeof(struct swt_sys_queue));
  ilRc = swShmselect_swt_sys_queue(ilDes_q,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0340: [错误/共享内存] 读邮箱[%d]设置出错",ilDes_q);
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0350: [错误/共享内存] 邮箱[%d]没有设置",ilDes_q);
    return(FAIL);
  }  
  
  if(swShmgetsafid(&llSafid) == FAIL)
  {
    swVdebug(0,"S0360: [错误/共享内存] 读取SAF流水号出错");
    return(FAIL);
  }  
  
  pslMsgpack->sMsghead.lSafid = llSafid;
  
  pslMsgpack->sMsghead.cSafflag = '1';

  sgSwt_rev_saf.saf_id = pslMsgpack->sMsghead.lSafid;
  sgSwt_rev_saf.tran_id = 0;
  sgSwt_rev_saf.proc_step = 0;
  time(&(sgSwt_rev_saf.saf_begin));
  sgSwt_rev_saf.saf_overtime = sgSwt_rev_saf.saf_begin + sgSwt_sys_queue.rev_overtime;
  sgSwt_rev_saf.rev_overtime = sgSwt_sys_queue.rev_overtime;
  if (sgSwt_sys_queue.rev_num > 0)
    sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num - 1;
  else
    sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;
  
  sgSwt_rev_saf.saf_len = 
    sizeof(struct msghead) + pslMsgpack->sMsghead.iBodylen;
  if (swPutlog((char *)pslMsgpack,
    sgSwt_rev_saf.saf_len,&(sgSwt_rev_saf.saf_offset),
    sgSwt_rev_saf.saf_file) == -1)
  {
    swVdebug(0,"S0370: [错误/函数调用] swPutlog()函数,存报文出错");
    return(FAIL);
  }
  sgSwt_rev_saf.saf_status[0] = '0';
  sgSwt_rev_saf.saf_flag[0] = '1';

  /* 插入一条SAF记录到数据库中 */
  /* del by gengling at 2015.04.02 one line PSBC_V1.0 */
  /* if ( (ilRc=swDbinsert_swt_rev_saf(sgSwt_rev_saf)) != SUCCESS) */ /*unsolved*/
  /* add by gengling at 2015.04.02 one line PSBC_V1.0 */
  if ( (ilRc=swShminsert_swt_rev_saf(sgSwt_rev_saf)) != SUCCESS)
  /* end of add */
  {
    swVdebug(0,"S0380: [错误/共享内存] 插入共享内存SAF记录出错");
    return(FAIL);
  }

  /* 将报文发送给前置程序 */
  pslMsgpack->sMsghead.iOrg_q = ilDes_q;
  pslMsgpack->sMsghead.iDes_q = 0;
  /* lijunwen at 2000-4-18:  qwrite2 改为 _qwrite2 */
  ilRc=_qwrite2(msgpack,sizeof(struct msghead)
    + pslMsgpack->sMsghead.iBodylen, ilDes_q,1,0,0);
  if(ilRc)
  {
    swVdebug(0,"S0390: [错误/函数调用] _qwrite2()函数,发送SAF报文失败,返回码=%d",ilRc);
    swMberror(ilRc,msgpack);
    return(FAIL);
  }
  swVdebug(4,"S0400: [函数返回] swSaf()函数,返回码=0");
  return(SUCCESS);
}

/* #endif */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */

/*************************************************************************
 ** 函数名:       int swInit ( short qid )
 ** 功  能:       对前置进程的环境进行初始化, 包括前置属性的设置，共享内存
                  的建立等等。
 ** 作  者:       路长胜
 ** 建立日期:     2000.01.27
 ** 最后修改日期: 2000.01.27
 ** 调用其它函数: swDbopen (), swDbclose ()
 ** 全局变量:     无
 ** 参数含义:     qid -- 前置进程邮箱号
 ** 返回值:       成功返回 0 (SUCCESS)，失败返回 -1 (FAIL)
 *************************************************************************
 ** 修改记录 ** 
**************************************************************************/
int swInit(short iQid)
{
  short ilRc;

  swVdebug(4,"S0410: [函数调用] swInit(%d)函数",iQid);
  /* 初始化共享内存指针 */
  if(swShmcheck() == FAIL)
  {
    swVdebug(0,"S0420: [错误/共享内存] 初始化共享内存指针失败");
    return(FAIL);
  }
  
  /* 读邮箱端口设置 */
  memset((char *)&sgSwt_sys_queue,0x00,sizeof(struct swt_sys_queue));
  ilRc = swShmselect_swt_sys_queue(iQid,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0430: [错误/共享内存] 读邮箱[%d]设置出错",iQid);
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0440: [错误/共享内存] 邮箱[%d]没有设置",iQid);
    return(FAIL);
  }  

  /* 初始化sgPortattrib */
  sgPortattrib.iQid = iQid;
  sgPortattrib.iMsgformat = sgSwt_sys_queue.msg_format;
  swVdebug(4,"S0450: [函数返回] swInit()函数,返回码=0");
  return SUCCESS;
}
/**************************************************************
 ** 函数名: swGetheadkey
 ** 功  能: 取报文头关键字
 ** 作  者: 史正烨
 ** 建立日期: 2000.2.27
 ** 最后修改日期: 2000.2.27
 ** 调用其它函数: swPreunpack()
 ** 全局变量:
 ** 参数含义: iQid	进行保存报文头操作的前置邮箱号
 **           psMsgpack	报文
 **           aHead_key	报文头关键字
 ** 返回值：SUCCESS-成功/FAIL-失败
***************************************************************/
int swGetheadkey(short iQid, struct msgpack *psMsgpack, char *aExpress, char *aHeadkey)
{
  /* delete by gxz 2001.10.17 
  int ilRc, ilLength; delete by gxz 2001.10.17 */

  int ilRc;
/*modify by zcd 20141229
 short ilLength;*/
  unsigned int ilLength;
  struct swt_sys_queue slSwt_sys_queue;
  /* add by gxz 2001.10.17 */
  short ilMsgformat;
  /* add by gxz 2001.10.17 */

  /* 读应用端口设置 */
  swVdebug(4,"S0460: [函数调用] swGetheadkey()函数");
  ilRc = swShmselect_swt_sys_queue(iQid, &slSwt_sys_queue);
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S0470: [错误/共享内存] 读应用端口[%d]设置失败",iQid);
    return (FAIL);
  }
  /* 预解包 */
  /* add by gxz 2001.10.17 */
  ilMsgformat = psMsgpack->sMsghead.iMsgformat; 
  /* add by gxz 2001.10.17 */
  psMsgpack->sMsghead.iMsgformat = slSwt_sys_queue.msg_format;
  ilRc = swPreunpack(psMsgpack);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0480: [错误/函数调用] swPreunpack(),预解包失败,返回码=%d",ilRc);
    _swMsgsend(302027, (char *)psMsgpack);
    return(FAIL);
  }
  /* 计算报文头关键字 */
  ilRc = _swExpress(aExpress, aHeadkey, &ilLength);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0490: [错误/函数调用] _swExpress(),计算报文头关键字%s失败,返回码=%d",aExpress,ilRc);
    _swMsgsend(302028, (char *)psMsgpack);
    return (FAIL);
  }
  aHeadkey[ilLength] = 0;
  psMsgpack->sMsghead.iMsgformat = ilMsgformat; 
  swVdebug(4,"S0500: [函数返回] swGetheadkey()函数,返回码=0");
  return (SUCCESS);
}

/**************************************************************
 ** 函数名: swSavemsghead
 ** 功  能: 保存报文头
 ** 作  者: 史正烨
 ** 建立日期: 2000.2.27
 ** 最后修改日期: 2000.2.27
 ** 调用其它函数: swPutlog
 ** 全局变量:
 ** 参数含义: iQid	进行保存报文头操作的前置邮箱号
 **           aHead_key	报文头关键字
 **           psMsghead	报文头
 ** 返回值: SUCCESS-成功/FAIL-失败
***************************************************************/
int swSavemsghead(short iQid,char * aHead_key,struct msghead *psMsghead)
{
  int ilRtncode;
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  tran_id;
    sqlint64  saf_id;
    sqlint64  head_offset;
    short q_id;
    char  head_key[51];
    short proc_step;
    char  head_file[11];
    short head_len;
  EXEC SQL END   DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  tran_id;
    long  saf_id;
    long  head_offset;
    short q_id;
    char  head_key[51];
    short proc_step;
    char  head_file[11];
    short head_len;
  EXEC SQL END   DECLARE SECTION;
#endif
#else
  long	tran_id;
  long	saf_id;
  long	head_offset;
  short q_id;
  char	head_key[51];
  short proc_step;
  char	head_file[11];
  short head_len;
#endif


  swVdebug(4,"S0510: [函数调用] swSavemsghead()函数");
  memset(head_key,0x00,sizeof(head_key));
  memset(head_file,0x00,sizeof(head_file));
  ilRtncode = swPutlog((char *)psMsghead,sizeof(struct msghead),
    &head_offset,head_file);
  if (ilRtncode == FAIL) return(FAIL);

  q_id = iQid;
  strcpy(head_key, aHead_key);
  
  swVdebug(2,"S0520: 保存报文头关键字 =q_id[%d]key[%s]",q_id,head_key); 

  tran_id = psMsghead->lTranid;
  proc_step = psMsghead->iTranstep;
  saf_id = psMsghead->lSafid;
  head_len = sizeof(struct msghead);
  
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
  EXEC SQL INSERT INTO swt_msghead VALUES 
    (:q_id,
     :head_key,
     :tran_id,
     :proc_step,
     :saf_id,
     :head_file,
     :head_offset,
     :head_len);
#ifdef DB_INFORMIX
  if (sqlca.sqlcode == SQLDUPKEY || sqlca.sqlcode == SQLDUPKEY2 || sqlca.sqlcode == SQLDUPKEY3)
#else
  if (sqlca.sqlcode == SQLDUPKEY)
#endif
  {
    EXEC SQL DELETE FROM swt_msghead WHERE q_id = :q_id
      AND head_key = :head_key;
    if (sqlca.sqlcode)
    {
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return(FAIL);
    }
    EXEC SQL INSERT INTO swt_msghead VALUES 
      (:q_id,
       :head_key,
       :tran_id,
       :proc_step,
       :saf_id,
       :head_file,
       :head_offset,
       :head_len);
  }
  if (sqlca.sqlcode) 
  {
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return(FAIL);
  }
  EXEC SQL COMMIT WORK;
#endif
  swVdebug(4,"S0530: [函数返回] swSavemsghead()函数,返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: swLoadmsghead
 ** 功  能: 取报文头
 ** 作  者: 史正烨
 ** 建立日期: 2000.2.27
 ** 最后修改日期: 2000.2.27
 ** 调用其它函数: swGetlog
 ** 全局变量:
 ** 参数含义: iQid	进行保存报文头操作的前置邮箱号
 **           aHead_key	报文头关键字
 **           psMsghead	报文头
 ** 返回值: SUCCESS-成功/FAIL-失败/100-没有找到
***************************************************************/
int swLoadmsghead(short iQid,char * aHead_key,struct msghead *psMsghead)
{
  int ilRtncode;
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  load_offset;
    short load_q;
    char  load_key[51];
    char  load_file[11];
    short load_len;
    short ilTmp3;
  EXEC SQL END   DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  load_offset;
    short load_q;
    char  load_key[51];
    char  load_file[11];
    short load_len;
    short ilTmp3;
  EXEC SQL END   DECLARE SECTION;
#endif
#else
  long  load_offset;
  short load_q;
  char  load_key[51];
  char  load_file[11];
  short load_len;
  short ilTmp3;
#endif
  
  swVdebug(4,"S0540: [函数调用] swLoadmsghead()函数");
  load_q = iQid;
  memset(load_key,0x00,sizeof(load_key));
  memset(load_file,0x00,sizeof(load_file));
  strcpy(load_key, aHead_key);
  
  swVdebug(2,"S0550: 取出报文头关键字 = %s",load_key); /* 11 */
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL SELECT head_file,head_offset,head_len
    INTO :load_file:ilTmp3,
	 :load_offset:ilTmp3,
         :load_len:ilTmp3 
    FROM swt_msghead
    WHERE q_id = :load_q AND head_key =:load_key;
  if (sqlca.sqlcode == 100) 
    return(SQLNOTFOUND);
  else if (sqlca.sqlcode) 
  {
  	swVdebug(0,"根据q_id[%d]head_key[%s]查找失败",load_q,load_key);
    swDberror(NULL);
    return(FAIL);
  }

  _swTrim(load_file);
  ilRtncode = swGetlog(load_file,load_len,load_offset,(char *)psMsghead);
  if (ilRtncode != SUCCESS)
  {
    swVdebug(0,"S0560: [错误/函数调用] swGetlog()函数,返回码=%d",ilRtncode);
    return(FAIL);
  }
#endif
  swVdebug(4,"S0570: [函数返回] swLoadmsghead()函数,返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名: swBpcount
 ** 功  能: 取得发起方端口正在处理的交易笔数
 ** 作  者: byz
 ** 建立日期: 2001.3.16
 ** 最后修改日期: 2001.3.16
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: iQid      前置邮箱号
 ** 返回值: SUCCESS-成功 FAIL-失败
***************************************************************/
/*
int swBpcount(short Qid)
{
  int i;
  
  for(i=0;i<iMAXPORTCOUNT;i++)
  {
    if (Qid==psgPortattrib[i].iQ_id ) 
    {
      return(psgPortattrib[i].lTrning);
    }
  }
  return(FAIL);
}
*/

/* ========= delete by qy 2001/08/22 ==========

int swBpcount(short iQid)
{
  int ilRc;

  [[* 读邮箱端口设置 *]]
  memset((char *)&sgSwt_sys_queue,0x00,sizeof(struct swt_sys_queue));
  ilRc = swShmselect_swt_sys_queue(iQid,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"读邮箱[%d]设置出错",iQid);
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"邮箱[%d]没有设置",iQid);
    return(FAIL);
  }  
  
  return(sgSwt_sys_queue.sPortattrib.lTrning);
}

============ end of delete by qy 2001/08/22 === */

/***************************************************************
 ** 函数名      : swSavetranhead
 ** 功  能      : 保存报文头
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swSavetranhead(short iQid,char *aMsghdkey,struct msghead *psMsghead)
{
  int ilRc;
  struct swt_tran_log slSwt_tran_log;
  
  ilRc = swShmselect_swt_tran_log(psMsghead->lTranid,&slSwt_tran_log,
           psMsghead->aSysattr[0]);
  if (ilRc) return(ilRc);
  if (slSwt_tran_log.q_target != iQid) return(-1);
  /*del by baiqj20150408 PSBC_V1.0  
  memcpy(slSwt_tran_log.msghdkey,aMsghdkey,iEXPRESSLEN);*/
  memcpy(slSwt_tran_log.msghdkey,aMsghdkey,sizeof(slSwt_tran_log.msghdkey)-1);
  memcpy(slSwt_tran_log.msghead,(char *)psMsghead,sizeof(struct msghead));
  ilRc = swShmupdate_swt_tran_log(psMsghead->lTranid,
           slSwt_tran_log,psMsghead->aSysattr[0]);
  return(ilRc);
}

/***************************************************************
 ** 函数名      : swLoadtranhead
 ** 功  能      : 取报文头
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swLoadtranhead(short iQid,char *aMsghdkey,struct msghead *psMsghead)
{
  int ilRc;
  struct swt_tran_log slSwt_tran_log;
  
  ilRc = swShmselect_qid_msghdkey_tranlog(iQid,aMsghdkey,&slSwt_tran_log);
  if (ilRc == SUCCESS) 
    memcpy((char *)psMsghead,slSwt_tran_log.msghead,sizeof(struct msghead));
  return (ilRc);
}

/* added by fzj at 2002.01.23, begin */
/***************************************************************
 ** 函数名: swGetmacbuf
 ** 功能: 取MAC buffer
 ** 作者:
 ** 建立日期: 2002/01/23
 ** 最后修改日期: 2002/01/23
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0:成功 / -1:失败
***************************************************************/
int swGetmacbuf(char *aMacbuf, short *iMacbuflen)
{
  if (igMacbuflen <= 0) return(-1);
  memcpy(aMacbuf, agMacbuf, igMacbuflen);
  *iMacbuflen = igMacbuflen;
  return(0);
} 
/* added by fzj at 2002.01.23, end */

/* === begin of added by fzj at 2002.02.28 === */
/*****************************************************************************
 ** 函数名:       int swNewtran(struct msghead *) 生成新交易包函数
 ** 功  能:       生成新发交易的报文头，按照各个端口的性质生成报文头 
 ** 作  者:
 ** 建立日期:     2000.01.26
 ** 最后修改日期: 2000.01.26
 ** 调用其它函数:  
 ** 全局变量:     
 ** 参数含义:     struct msghead *psMsghead: 指向报文头的指针
 ** 返回值:       成功返回 0(SUCCESS), 
		  -1：失败(取内部交易流水号时，对共享内存操作不成功)
		  -2：平台最大交易数超限
		  -3：端口发起交易数超限
		  -4：关联端口处理交易数超限
		  -5：关联端口状态down
                  -6: 交易流量超限 //add by gengling at 2015.03.18 one line PSBC_V1.0 
	          -7: 没有在SWTRAN_CTL.CFG 中配置
******************************************************************************
 ** 修改记录 ** 
*****************************************************************************/
int swNewtranhead(char *aTrancode, struct msghead *psMsghead)
{
  int ilRc;
  long ilTranid;

  memset(psMsghead,0x00,sizeof(struct msghead));

  if (swShmisfull()) return(-2);
  if (swShmtranisfull(sgPortattrib.iQid)) return(-3);
  /* add by gengling at 2015.03.18 one line PSBC_V1.0 */
  ilRc=swShmtranflowisfull(sgPortattrib.iQid, aTrancode);
  if (ilRc <0  ){
		if(ilRc == -2)
			return(-7);
		else
			return(-6);
	} 
	
  
#if 0 
  if (aTrancode != NULL)
  {
  	/*2009-9-9 10:58 del by pc 增加其他跨组机器判断*/
    if ((ilRc = swShmportisdown(sgPortattrib.iQid,aTrancode))) 
    {
        swVdebug(0, "port is down [%d][%s]ret=[%d]", sgPortattrib.iQid, aTrancode, ilRc);
        return(ilRc);
    }
  }
#endif

  /* 生成交易报文头 */
  psMsghead->cRouter = '1';
  psMsghead->cFormatter = '0';
  psMsghead->iMsgtype = 1;
  psMsghead->iTranstep = 1;
  psMsghead->iMsgformat = sgPortattrib.iMsgformat;
  psMsghead->iOrg_q = sgPortattrib.iQid;
  psMsghead->iDes_q = sgPortattrib.iQid;
  psMsghead->iBegin_q_id = sgPortattrib.iQid;
  psMsghead->aSysattr[0]='0'; /* add by nh 20020910 */
  psMsghead->call_flag = '0';/*add by baiqj20150319,PSBC_V1.0*/
  psMsghead->callstart.time = 0;
  psMsghead->callstart.millitm = 0;
  memset(psMsghead->timeconsum,0x00,sizeof(psMsghead->timeconsum));/*add by baiqj20150319,PSBC_V1.0*/
  time(&(psMsghead->lBegintime));
  
  /* 操作共享内存，产生新的交易流水号 */
  ilRc = swShmgettranid(&ilTranid);
  if(ilRc == FAIL) return(FAIL);
  psMsghead->lTranid = ilTranid;

  return(SUCCESS);
}
/* === end of added by fzj at 2002.02.28 === */
/************************************************************************
 ** 函数名:       int swTranrevStatus ( long	tran_id )
 ** 功  能:       停止交易正常步骤，发送冲正请求报文至事务管理的事务处理
                  自带邮箱(iMBMONREV)。 如果报文类型为冲正请求报文，则所
                  发报文为停止冲正报文；否则发送即时冲正报文。
 ** 作  者:       路长胜
 ** 建立日期:     2000.01.27
 ** 最后修改日期: 2000.01.27
 ** 调用其它函数: swMsgsend (), qwrite (), swMberror (), swDberror ()
 ** 全局变量:     无
 ** 参数含义:     tran_id -- 交换平台内部流水号
 ** 返回值:       成功返回 0 (SUCCESS)，失败返回 -1 (FAIL)
 ************************************************************************
 ** 修改记录 ** 
*************************************************************************/
int swTranrevStatus(long tran_id)
{
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  llTranid1;
    sqlint64  llTime1;
    short ilTran_status1;
    short ilTmp;
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llTranid1;
    long  llTime1;
    short ilTran_status1;
    short ilTmp;
  EXEC SQL END DECLARE SECTION;
#endif
#else
  long  llTranid1;
  long  llTime1;
  short ilTran_status1;
  short ilTmp;
#endif
  int ilRc, ilMsglen = 0;
  struct msgpack slMsgpack;
  
  swVdebug(4,"S0160: [函数调用] swTranrev(%ld)函数",tran_id);

  memset ((char*)&slMsgpack, 0, sizeof (struct msgpack));

  llTranid1 = tran_id;

  /* 查流水状态,从SHM和DB中 */
  #ifdef DB_SUPPORT
  ilRc = swShmselect_swt_tran_log(llTranid1, &sgSwt_tran_log,'a');
  #else
  ilRc = swShmselect_swt_tran_log(llTranid1, &sgSwt_tran_log,'0'); 
  #endif
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0170: [错误/共享内存] 查找此交易流水[%ld]时出错",llTranid1);
    return(FAIL);
  }
  
  if ( ilRc == SHMNOTFOUND ) 
  {
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL SELECT tran_status INTO :ilTran_status1:ilTmp FROM swt_his_tran_log
      WHERE tran_id = :llTranid1;
    if (sqlca.sqlcode == SQLNOTFOUND)
    {
      swVdebug(0,"S0180: [错误/数据库] sqlcode=%ld",sqlca.sqlcode);
#ifdef TRANREVLOG
      time(&llTime1);      
      EXEC SQL INSERT INTO swt_tranrev_log VALUES(:llTranid1,:llTime1);
      if(sqlca.sqlcode)
      {
        if (!(sqlca.sqlcode==SQLDUPKEY||sqlca.sqlcode==SQLDUPKEY2||sqlca.sqlcode==SQLDUPKEY3))
        { 
          swVdebug(0,"S0185: insert into swt_tran_rev_log failed, sqlcode=%d",sqlca.sqlcode);
        }  
      }  
#endif      
      return FAIL;
    }
    else if(sqlca.sqlcode)
    {
      swDberror ((char *)&slMsgpack);
      return FAIL;
    }
    else
    {
      if (ilTran_status1 == iTRNREVEND) 
      {
        swVdebug(4,"S0190: [函数返回] swTranrev()函数,返回码=0");
        return(ilTran_status1);
      }
    }
#endif
  }
  else 
  {
    ilTran_status1 = sgSwt_tran_log.tran_status;
    
    if (ilTran_status1 == iTRNREVEND) 
    {
      swVdebug(4,"S0200: [函数返回] swTranrev()函数,返回码=0");
      return(ilTran_status1);
    }
  }

  swVdebug(2,"S0210: ilTran_status1=%d",ilTran_status1);

  slMsgpack.sMsghead.lTranid = tran_id;
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.aSysattr[0] = '0';
  ilMsglen = sizeof (struct msghead);

  switch ( ilTran_status1 ) {
    case iTRNING:
    case iTRNSAF:
    case iTRNOVERTIME:
    case iTRNREVFAIL:
    case iTRNEND:
      slMsgpack.sMsghead.lCode = 601;
      break;
    case iTRNREVING:
/* delete by szhengye 2000.3.20
      slMsgpack.sMsghead.lCode = 602;
delete by szhengye 2000.3.20 */
      swVdebug(4,"S0220: [函数返回] swTranrev()函数,返回码=0");
      return ilTran_status1;
      break;
    default:
      swVdebug(4,"S0230: [函数返回] swTranrev()函数,返回码=0");
      return(FAIL);
      break;
  }

  /* send command */
  ilRc = qwrite ((char *)&slMsgpack, ilMsglen, iMBMONREV);
  if(ilRc) {
    swMberror (ilRc, (char *)&slMsgpack);
    return FAIL;
  }
  swVdebug(4,"S0240: [函数返回] swTranrev()函数,返回码=0");
  return ilTran_status1;
}
