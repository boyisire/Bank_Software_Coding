/**************************************************************/
/* 模块编号    ：swComtcpalc                                  */
/* 模块名称    : 通信例程-TCP异步长连接Client                 */
/* 作	 者    ：                                             */
/* 建立日期    ：2000/5/18                                    */
/* 最后修改日期：2001/9/18                                    */
/* 模块用途    ：                                             */
/* 本模块中包含如下函数及功能说明：                           */
/*			  (1)short swComtcpaloc()             */
/*                        (2)short swDoitaloc()               */
/*                        (3)short swTimeout()                */
/*                        (4)short swQuit()                   */
/**************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

void  swQuit( );

static long igSockfd;

int main(int argc,char **argv)
{
  int    ilRc;
  #ifdef OS_SCO
    int  ilAddrlen;
  #else
    socklen_t  ilAddrlen;
  #endif
  unsigned int ilQid,ilMsglen,ilType,ilClass,ilPri;
  char   alMsgbuf[iMSGMAXLEN];
  struct sockaddr_in slCli_addr;
  struct msghead *pslMsghead;

  /* 取得版本号 */
  if(argc > 1)
    _swVersion("swComtcpalc Version 4.3.0",argv[1]);
  
  /* 设置调试程序名称 */
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(0,"S0010: [错误/其它] 传入参数数量不合法!");
    exit(1);
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
    swVdebug(0,"S0040: [错误/函数调用] swMbopen()函数,打开邮箱[%d]出错,返回码=%d",sgComcfg.iMb_comm_id,ilRc);
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
  ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 ); */
  ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 ); 
  if ( ilRc )
  {
    swVdebug(0,"S0070: [错误/函数调用] swPortset()函数,置端口[%d]状态出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
    swMbclose();
    exit(-1);
  }
  swVdebug(2,"S0080: 设置端口成功!");

  /*连接到服务器*/
  igSockfd = swConnectser( sgTcpcfg.aPartner_addr, sgTcpcfg.iPartner_port );
  if ( igSockfd < 0 )
  {
    igSockfd=swCheck_comm();
    if ( igSockfd <0 )
    {
      swVdebug(0,"S0090: [错误/函数调用] swCheck_comm()函数,连接到服务器出错!",sgComcfg.iMb_comm_id);
      swMbclose();
      close(igSockfd);
      exit(-1);
    }
  }	
  swVdebug(2,"S0100: 连接到服务器{%s}的端口[%u]成功!",\
    sgTcpcfg.aPartner_addr ,ntohs(sgTcpcfg.iPartner_port));
  
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));

  ilRc=getsockname(igSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen);
  if(ilRc == -1)
  {
    swVdebug(0,"S0110: [错误/系统调用] unable to read socket address");
    swMbclose();
    close(igSockfd);
    exit(-1);
  }
  swVdebug(2,"S0120: getsockname SUCCESS!"); 
   
  for(;;) 
  {
    /* Read from mq */
    ilPri = 0;
    ilClass = 0;
    ilType  = 0;
    ilMsglen = sizeof( alMsgbuf );
  
    swVdebug(2,"S0130: 开始从邮箱接收信息 !");
 
    ilRc = swRecvpackw(&ilQid, (char * )&alMsgbuf,&ilMsglen,
      &ilPri,&ilClass,&ilType);
    if ( ilRc )
    {
      swVdebug(1,"S0140: [错误/函数调用] swRecvpackw()函数,读邮箱[%d]出错,返回码=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      close( igSockfd );
      exit( -1 );
    }

    swVdebug(2,"S0150: 从邮箱[%d]收到长度为[%d]的信息!",ilMsglen,ilQid);
 
    if (cgDebug >= 2)  
      swDebughex( alMsgbuf , ilMsglen );      
    
    pslMsghead = (struct msghead *)alMsgbuf;

    /*判断有无报文头*/
    if(sgComcfg.iMsghead == 0)
    {
      /* 不带报文头往外发 */
      ilRc = swTcpsnd(igSockfd,alMsgbuf + sizeof(struct msghead),
        ilMsglen - sizeof(struct msghead));
    }
    else
    {
      ilRc = swTcpsnd(igSockfd,alMsgbuf,ilMsglen);
    }
    if ( ilRc < 0 )
    {
      close(igSockfd);
      igSockfd=swCheck_comm();
      if ( igSockfd <0 )
      { 
        swVdebug(1,"S0160: [错误/函数调用] swCheck_comm()函数,报文发送到服务端出错!");
        continue;
      }
    }
    swVdebug(2,"S0170: 报文已成功发送到服务端!");
   }
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSockfd);
  exit(0);
}

