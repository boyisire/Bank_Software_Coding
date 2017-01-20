/****************************************************************/
/* 模块编号    ：swComtcpssc                                    */
/* 模块名称    : 通信例程-TCP同步短短连接Client                 */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/15                                      */
/* 最后修改日期：2001/9/18                                      */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/*			  (1)int32 swComtcpssc()                */
/*                        (2)int32 swDoitssc()                  */
/*                        (4)int32 swQuit()                     */
/****************************************************************/
#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static int    igSockfd;

void swQuit(int);
int swDoitssc(short iFlag,char *aMsgbuf,unsigned int iMsglen);
extern int swSendpackl(short, char *, unsigned int, short, long, long);
extern int swRecvpackl(short *,char *,unsigned int *,short *,long *,long *,short);

int main( int argc, char **argv )
{
  int    ilRc;
  unsigned int ilMsglen;
  short  ilQid,ilType,ilClass,ilPri;
  short  ilStatus,ilMode=0,ilContflag,ilMode1;
  char   alMsgbuf[iMSGMAXLEN];
  
  /* 取得版本号 */
  if(argc > 1)
    _swVersion("swComtcpssc Version 4.3.0",argv[1]);
 
  /* 设置调试程序名称 */  
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    printf("Usage: swComtcpssc portname \n");
    exit(-1);
  }
  
  /* 处理信号 */
  swSiginit( );
 
  /* 读入并处理参数 */
  ilRc = swLoadcomcfg( argv[1] );
  if ( ilRc )
  {
    swVdebug(0,"S0010: [错误/函数调用] swLoadcomcfg()函数,[%s]装入通讯配置参数出错,请检查输入的端口名,返回码=%d",argv[1],ilRc);
    exit(-1);
  }
  swVdebug(2,"S0020: [%s]装入通讯配置参数成功",argv[1]);
   
  ilRc = swMbopen(sgComcfg.iMb_comm_id);
  if ( ilRc )
  {  
    swVdebug(0,"S0030: [错误/邮箱] 打开邮箱[%d]失败!ilRc=%d",sgComcfg.iMb_comm_id,ilRc);
    exit(-1);
  }
  swVdebug(2,"S0040: 打开邮箱[%d]成功!",sgComcfg.iMb_comm_id);
 
  /* 置端口通讯层为UP */
  ilRc = swShmcheck();
  if (ilRc !=SUCCESS )
  {
    swVdebug(0,"S0050: [错误/共享内存] 共享内存检测出错!");
    swMbclose();
    return(FAIL);
  }

  /* next modify by nh 2002.6.6
  ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
  ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
  if ( ilRc )
  {
    swVdebug(0,"S0060: [错误/函数调用] swPortset()函数,置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
    swMbclose();
    exit(-1);
  }
  swVdebug(2,"S0070: 设置端口成功");

  for(;;) 
  {
    swVdebug(2,"S0080: 循环处理");
    /* 取端口通讯状态 */
    /* next modify by nh 2002.6.6
    ilRc =  swPortget( sgComcfg.iMb_comm_id,1,&ilStatus );*/
    ilRc =  swPortget( sgComcfg.iMb_fore_id,1,&ilStatus );
    if ( ilRc )
    {
      swVdebug(0,"S0090: [错误/函数调用] swPortget()函数,取端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
      exit(-1);
    }

    /* 判断端口状态 */
    if ( ilStatus == 0 )
    {
      swVdebug(0,"S0100: [错误/其它] 端口状态(通讯层)为down,不外发报文!");
      continue;
    }

    /* 读通讯邮箱  */
    ilPri = 0;
    ilClass = 0;
    ilType  = 0;
    ilMsglen = sizeof( alMsgbuf );

    swVdebug(2,"S0110: 等待从通讯邮箱[%d]接收报文!",sgComcfg.iMb_comm_id);
    swVdebug(2,"S0120: ilPri=[%d],Class=[%d],Type=[%d]",ilPri,ilClass,ilType);
 
    ilRc = swRecvpackw(&ilQid,alMsgbuf,&ilMsglen,&ilPri,&ilClass,&ilType);
    if ( ilRc )
    {  
      swVdebug(1,"S0130: [错误/函数调用] swRecvpackw()函数,读邮箱[%d]出错,返回码=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      exit( -1 );
    }
    
    swVdebug(2,"S0140: 从通讯邮箱[%d]接收报文成功!",sgComcfg.iMb_comm_id);
    swVdebug(2,"S0150: ilPri=[%d],Class=[%d],Type=[%d]",ilPri,ilClass,ilType);
 
    if (cgDebug >= 2)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* 判断有无后续包*/
    ilContflag = swSwcheckcontpack(alMsgbuf);
    if ( ilContflag == 1 )  /*有后续包处理*/
    {
      ilRc = swConnsend( ilMode ,&igSockfd, alMsgbuf, ilMsglen );
      /* next 9 rows add by nh */
      if(!ilRc)
      {
        /* next modify by nh 2002.6.6
        ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
        ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
        if ( ilRc )
        {
          swVdebug(1,"S0160: [错误/函数调用] swPortset()函数, 置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
          swMbclose();
          exit(-1);
        }
        swVdebug(2,"S0170: 设置端口成功");
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
    /* fork子进程 */
    switch(fork()) 
    { 
      case 0:
        swVdebug(2,"S0180: 子进程处理");
        /*next modify by nh*/
        ilRc = swDoitssc(ilMode1,alMsgbuf,ilMsglen); 
        if(!ilRc)
        {
          /* next modify by nh 2002.6.6
          ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
          ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
          if ( ilRc )
          {
            swVdebug(1,"S0190: [错误/函数调用] swPortset()函数, 置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
            swMbclose();
            exit(-1);
          }
          swVdebug(2,"S0200: 设置端口成功");
        }
        exit(0);
      case -1:
        swVdebug(1,"S0210: Fork 进程出错!" );
        swMbclose();
        exit(-1);
      default:
        swVdebug(2,"S0220: 父进程处理");
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

int swDoitssc(short iFlag,char *aMsgbuf,unsigned int iMsglen)
{
  int    ilRc;
  short  ilContflag;
  struct msghead *pslMsghead;

  swVdebug(4,"S0230: [函数调用] swDoitssc(%d,%d)",iFlag,iMsglen);
  ilRc = swConnsend( iFlag, &igSockfd, aMsgbuf, iMsglen );
  if ( ilRc )
  {
    close( igSockfd );
    return(-1);
  }

  pslMsghead = (struct msghead *)aMsgbuf;
  /* 打开通讯邮箱 */
  ilRc = swMbopen( sgComcfg.iMb_comm_id );
  if ( ilRc )
  {
    swVdebug(1,"S0240: [错误/邮箱] 打开邮箱[%d]失败!",sgComcfg.iMb_comm_id);
    close( igSockfd );
    return(-1);
  }
  for( ; ; )
  {
    /* 从服务端口接收报文 */
    swVdebug(2,"S0250: 等待接收服务端的数据" );
 
    if (sgComcfg.iMsghead == 0)
    {
      /* 通讯返回不带报文头 */
      ilRc = swTcprcv(igSockfd,aMsgbuf+sizeof(struct msghead),&iMsglen); 
      if ( ilRc ) 
      { 
        swVdebug(1,"S0260: [错误/函数调用] swTcprcv()函数,从服务端接收出错-errno=[%d,%s]",errno,
          strerror(errno));
        swMbclose();
        close( igSockfd );
        return(-1);
      }
      /* 计算通讯后续包标志 */
      ilContflag = swComcheckcontpack(aMsgbuf+sizeof(struct msghead),iMsglen);
      pslMsghead->iBodylen = iMsglen;
      iMsglen = iMsglen + sizeof(struct msghead);
    }
    else
    {
      /* 通讯返回带报文头 */
      ilRc = swTcprcv(igSockfd,aMsgbuf,&iMsglen); 
      if ( ilRc ) 
      { 
        swVdebug(1,"S0270: [错误/函数调用] swTcprcv()函数,从服务端接收出错,errno=%d[%s]",errno,
          strerror(errno));
        swMbclose();
        close( igSockfd );
        return(-1);
      }

      /* 计算通讯后续包标志 */
      ilContflag = swComcheckcontpack(aMsgbuf,iMsglen);
      pslMsghead->iBodylen = iMsglen - sizeof(struct msghead);
    }

    swVdebug(2,"S0280: swTcprcv SUCCESS!");
    swVdebug(2,"S0290: 从端口接收报文成功!iMsglen = %d",iMsglen);
 
    if( cgDebug >= 2 )
      swDebughex( aMsgbuf,iMsglen);

    /* 写到前置邮箱 */
    ilRc = swSendpack(sgComcfg.iMb_fore_id,aMsgbuf,iMsglen,0,0,0);
    if (ilRc)
    {
      swVdebug(1,"S0300: [错误/函数调用] swSendpack()函数,写前置邮箱[%d]出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
      swMbclose();
      close( igSockfd );
      return(-1);
    }
    swVdebug(2,"S0310: 写前置邮箱[%d]成功!",sgComcfg.iMb_fore_id);
    if (ilContflag != 1) break;
  } /* end while */

  swMbclose();
  close(igSockfd);
  swVdebug(4,"S0320: [函数返回] swDoitssc()返回码=0");
  return(0);
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSockfd);
  exit(0);
}

