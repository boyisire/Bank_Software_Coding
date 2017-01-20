/****************************************************************/
/* 模块编号    ：swComtcpasc                                    */
/* 模块名称    : 通信例程-TCP异步短连接Client                   */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/18                                      */
/* 最后修改日期：2001/9/18                                      */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/*			  (1)int swComtcpasc()                  */
/*                        (2)int swDoitasc()                    */
/*                        (3)int swQuit()                       */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static  int  igSockfd;

int swDoitasc(short iFlag, char *aMsgbuf ,unsigned int iMsglen );
void swQuit();

int main( int argc, char **argv )
{
  int    ilRc;
  unsigned int ilMsglen;
  short  ilQid,ilType,ilClass,ilPri;
  short  ilStatus,ilMode=0,ilContflag,ilMode1;
  char   alMsgbuf[iMSGMAXLEN];

  /* 取得版本号 */
  if(argc > 1)
    _swVersion("swComtcpasc Version 4.3.0",argv[1]);

  /* 设置调试程序名称 */   
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(0,"S0010: [错误/其它] 传入参数数量不合法!");
    exit(-1);
  }
  
  /* 处理信号 */
  swSiginit( );
    
  ilRc = swLoadcomcfg( argv[1] );
  if ( ilRc )
  {
    swVdebug(0,"S0020: [错误/函数调用] swLoadcomcfg()函数,[%s]装入通讯配置参数出错,请检查输入的端口,返回码=%d",argv[1],ilRc);
    exit( -1 );
  }
  swVdebug(2,"S0030: [%s]装入通讯配置参数成功!",argv[1]);
   
  ilRc = swMbopen(sgComcfg.iMb_comm_id);
  if ( ilRc )
  {  
    swVdebug(0,"S0040: [错误/邮箱] 打开邮箱[%d]失败!",sgComcfg.iMb_comm_id);
    exit( -1 );
  }
        
  swVdebug(2,"S0050: 打开邮箱[%d]成功!",sgComcfg.iMb_comm_id);
   
  /* 置端口通讯层为UP */
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S0060: [错误/共享内存] 共享内存检测出错!");
    swMbclose();
    return(FAIL);
  }

  /* next modify by nh 2002.6.6
  ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
  ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
  if ( ilRc )
  {
    swVdebug(0,"S0070: [错误/函数调用] swPortset()函数,置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
    swMbclose();
    exit(-1);
  }
  swVdebug(2,"S0080: 设置端口成功!");
   
  for(;;) 
  { 
    swVdebug(2,"S0090: 循环处理!");
 
    /* 取端口通讯状态 */
    /* next modify by nh 2002.6.6
    ilRc =  swPortget( sgComcfg.iMb_comm_id, 1, &ilStatus );*/
    ilRc =  swPortget( sgComcfg.iMb_fore_id, 1, &ilStatus );
    if ( ilRc )
    {
      swVdebug(1,"S0100: [错误/函数调用] swPortget()函数,取端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
      exit(-1);
    }

    /* 判断端口状态 */
    if ( ilStatus == 0 )
    {
      swVdebug(1,"S0110: [错误/其它] 端口状态(通讯层)为down,不外发报文!");
      continue;
    }
 
    /* Read from mq */
    ilPri = 0;
    ilClass = 0;
    ilType  = 0;
    ilMsglen = sizeof( alMsgbuf );
  
    swVdebug(2,"S0120: 开始从邮箱读取信息!");
      
    ilRc = swRecvpackw(&ilQid, alMsgbuf,&ilMsglen,&ilPri,&ilClass,&ilType);
    if ( ilRc )
    {  
      swVdebug(1,"S0130: [错误/函数调用] swRecvpackw()函数,读邮箱[%d]出错,返回码=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      exit( -1 );
    }

    swVdebug(2,"S0140: 从通讯邮箱读报文成功!");
    if (cgDebug >= 2)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* 判断有无后续包*/
    ilContflag = swSwcheckcontpack(alMsgbuf);
    if ( ilContflag == 1 )  /*有后续包处理*/
    {
      ilRc = swConnsend(ilMode ,&igSockfd, alMsgbuf, ilMsglen);
      /* next 9 rows add by nh 2001/12/12 */
      if(!ilRc)
      {
        /* next modify by nh 2002.6.6
        ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
        ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
        if ( ilRc )
        {
          swVdebug(1,"S0150: [错误/函数调用] swPortset()函数, 置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
          swMbclose();
          exit(-1);
        }
        swVdebug(2,"S0160: 设置端口成功");
      }
      ilMode = 1;
      continue;
    }
    if ( ilMode == 1 )   /*后续包中的最后一个包*/
    {
      ilMode1 = 1 ;
      ilMode = 0 ;
    }
    else                 /*无后续包*/
    {
      ilMode1 = 0;
      ilMode = 0 ;
    }
 
    switch(fork()) 
    {
      case 0:
        swVdebug(2,"S0170: 子进程处理!");
        setpgrp();
        ilRc = swDoitasc(ilMode1, alMsgbuf, ilMsglen); 
        /* next 9 rows add by nh 2001/12/12 */
        if(!ilRc)
        {
          /* next modify by nh 2002.6.6
          ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
          ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
          if ( ilRc )
          {
            swVdebug(1,"S0180: [错误/函数调用] swPortset()函数, 置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
            swMbclose();
            exit(-1);
          }
          swVdebug(2,"S0190: 设置端口成功");
        }
        exit(0);
      case -1:
        swVdebug(2,"S0200: Fork 进程出错!" );
        swMbclose();
        exit(-1);
      default:
        swVdebug(2,"S0210: 父进程处理");
        break;                  
    }
  }
}
/****************************************************************/
/* 函数编号    ：swDoitass                                      */
/* 函数名称    ：                                               */
/* 作    者    ：                                               */
/* 建立日期    ：2001/9/15                                      */
/* 最后修改日期：                                               */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   -1                                      */
/****************************************************************/

int swDoitasc(short iFlag, char *aMsgbuf ,unsigned int iMsglen )
{
  int    ilRc;
 
  ilRc = swConnsend( iFlag, &igSockfd, aMsgbuf, iMsglen );
  if ( ilRc )
  {
    close( igSockfd );
    return(-1);
  }
  close( igSockfd );
  return 0;
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSockfd);
  exit(0);
}


