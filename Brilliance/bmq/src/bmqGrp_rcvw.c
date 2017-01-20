/****************************************************************/
/* 模块编号    ：BMQGRP_RCVW                                    */
/* 模块名称    ：跨组级联短连接报文存储模块                     */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2007/01/28                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：新晨Q 跨组级联短连接报文存储                   */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/*                                                              */
/****************************************************************/

/*库函数定义*/
#include "bmq.h"

struct Msgbuf{
  long   mtype;
  struct grprecvmsg rcvmsg;
  long   msglen;
  char   data[7168];
}sgMsgbuf;

int main(int argc,char **argv)
{
  int    ilRc;
  
  /*显示新晨Q版本信息*/
  _bmqShowversion(argc,argv);

  /*设置信号*/
  signal(SIGQUIT,SIG_IGN);
  
  /* 创建守护进程 */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010 %s:创建守护进程失败!",__FILE__);
    exit(FAIL);
  }
 
  /*读入参数配置文件DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0020 %s :载入DEBUG参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }
   
  /*连接共享内存*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030 %s:连接共享内存区出错:%d",__FILE__,ilRc);
    exit(FAIL);
  }

  psgMbshm->lBmqgrprcvw_pid = getpid();

  _bmqVdebug(1,"S0040 Begin start grp_rcvw...");
  /* 启动清空消息队列 */
  while( msgrcv(psgMbshm->lMsgid_grp_rcv,(char *)&sgMsgbuf,sizeof(sgMsgbuf),0,IPC_NOWAIT) >= 0  );

  _bmqVdebug(1,"S0050 clear msg[%ld] ok!",psgMbshm->lMsgid_grp_rcv);

  for( ; ; )
  {
    memset(&sgMsgbuf,0x00,sizeof(sgMsgbuf));

    sgMsgbuf.mtype = 99; 
    /* 阻塞读消息队列 */
    ilRc = msgrcv (psgMbshm->lMsgid_grp_rcv, (char *)&sgMsgbuf,sizeof(sgMsgbuf),0,0);
    if (ilRc < 0 ) {
       _bmqDebug("S0040 读消息队列出错[%d] ,errno:%d,qid[%ld]", ilRc,errno,psgMbshm->lMsgid_grp_rcv );
       if(psgMbshm->lMsgid_grp_rcv == 0) exit(-1);
       continue;
    }
    if( igDebug >= 3) _bmqDebughex(sgMsgbuf.data,sgMsgbuf.msglen);
    
    /*报文存入*/
    igMb_sys_current_group   = sgMsgbuf.rcvmsg.iOrg_group;
    igMb_sys_current_mailbox = sgMsgbuf.rcvmsg.iOrg_mailbox;
    igMbopenflag             = 1;  
    lgCurrent_pid            = getpid();
    
    ilRc = bmqPutfilter(0,sgMsgbuf.rcvmsg.iDes_mailbox,sgMsgbuf.rcvmsg.iPrior,sgMsgbuf.rcvmsg.aFilter, \
      sgMsgbuf.data, sgMsgbuf.msglen );
    if(ilRc)
    {
      _bmqDebug("S0050 报文存入[%d]错误!",sgMsgbuf.rcvmsg.iDes_mailbox);
    }
  }
}
