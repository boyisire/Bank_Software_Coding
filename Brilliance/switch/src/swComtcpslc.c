/****************************************************************/
/* 模块编号    ：swComtcpslc                                    */
/* 模块名称    : 通信例程-TCP同步长连接Client                   */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/18                                      */
/* 最后修改日期：2001/9/16                                      */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/*			  (1)int swComtcpslc()                  */
/*                        (2)int swTimeout()                    */
/*                        (3)int swQuit()                       */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

int  swDoitslc();
void swQuit();
void swTimeout();

static  long    igSockfd;

int main( int argc, char **argv )
{
  int    ilRc;

  #ifdef OS_SCO
    int  ilAddrlen;
  #else
    socklen_t  ilAddrlen;
  #endif

  unsigned int ilMsglen;
  short  ilQid,ilType,ilClass,ilPri;
  char   alMsgbuf[iMSGMAXLEN];
  struct sockaddr_in slCli_addr;
  struct msghead *pslMsghead;

  /* 取得版本号 */
  if(argc > 1)
    _swVersion("swComtcpslc Version 4.3.0",argv[1]);

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
    exit( 1 );
  }
  swVdebug(2,"S0030: [%s]装入通讯配置参数成功!",argv[1]);
   
  ilRc = swMbopen(sgComcfg.iMb_comm_id);
  if ( ilRc )
  {
    swVdebug(0,"S0040: [错误/邮箱] 打开邮箱[%d]失败",sgComcfg.iMb_comm_id);
    exit( -1 );
  }
      
  swVdebug(2,"S0050: 打开邮箱[%d]成功!",sgComcfg.iMb_comm_id);
   
  /* 置端口通讯层为UP */
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S0060: [错误/共享内存] 共享内存检测出错!");
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
  swVdebug(2,"S0090: 开始连接到服务器");
   
  /*连接到服务器*/
  igSockfd = swConnectser( sgTcpcfg.aPartner_addr, sgTcpcfg.iPartner_port );
  if ( igSockfd < 0 )
  {
    igSockfd=swCheck_comm();
    if ( igSockfd < 0 )
    {
      swVdebug(0,"S0100: [错误/函数调用] swCheck_comm()函数,连接到服务器出错!");
      swMbclose();
      exit(-1);
    }
  }
  swVdebug(2,"S0110: 连接到服务器{%s}的端口[%u]成功!", \
    sgTcpcfg.aPartner_addr , ntohs(slCli_addr.sin_port));
  
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  
  ilRc=getsockname(igSockfd,(struct sockaddr *)&slCli_addr,&ilAddrlen);
  if ( ilRc == -1 )
  {
    swVdebug(0,"S0120: [错误/系统调用] getsockname()函数,unable to read socket address,errno=%d[%s]",errno,strerror(errno));
    close( igSockfd );
    exit( -1 );
  }
  swVdebug(2,"S0130: getsockname SUCCESS!");
 
  for( ; ; )
  {
    swVdebug(2,"S0140: 循环处理");
  
    /* Read from mq */
    ilPri = 0;
    ilClass = 0;
    ilType  = 0;
    ilMsglen = sizeof( alMsgbuf );
    
    swVdebug(2,"S0150: 开始从邮箱读取信息!");
      
    ilRc = swRecvpackw(&ilQid, alMsgbuf,&ilMsglen,&ilPri,&ilClass,&ilType);
    if ( ilRc )
    {
      swVdebug(1,"S0160: [错误/函数调用] swRecvpackw()函数,读取邮箱[%d]失败,返回码=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      close( igSockfd );
      exit( -1 );
    }

    swVdebug(2,"S0170: 读取邮箱[%d]成功! len=%d",ilQid,ilMsglen);
    if (cgDebug >= 2)  
    {
      swDebughex( alMsgbuf , ilMsglen ) ;         
    }

    pslMsghead = (struct msghead *)alMsgbuf; 

    alarm( 0 );
    signal(SIGALRM , swTimeout);
    alarm( sgComcfg.iTime_out );

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

    if ( ilRc )
    {
      swVdebug(1,"S0180: [错误/函数调用] swTcpsnd发送报文到服务器出错,errno=%d[%s]",errno,strerror(errno));
      close( igSockfd );
      igSockfd=swCheck_comm();
      if ( igSockfd < 0 )
      {
        swVdebug(1,"S0190: [错误/函数调用] swCheck_comm()函数");
        swMbclose();
        signal(SIGALRM,SIG_IGN);
        alarm(0);
        exit(-1);
      }
    }
    swVdebug(2,"S0200: 从端口发送报文成功! ilMsglen=[%d]",ilMsglen );
    if( cgDebug >= 2 )
    {
      swDebughex( alMsgbuf,ilMsglen);
    }

    /* 从服务端口接收报文 */
    ilMsglen = sizeof(alMsgbuf);
    swVdebug(2,"S0210: 等待接收服务端的数据." );
 
    if(sgComcfg.iMsghead == 0)
    {
      /* 通讯返回不带报文头 */
      ilRc = swTcprcv(igSockfd,alMsgbuf + sizeof(struct msghead),&ilMsglen);
      pslMsghead->iBodylen = ilMsglen;
      ilMsglen = ilMsglen + sizeof(struct msghead);
    }
    else
    {
      ilRc = swTcprcv(igSockfd,alMsgbuf,&ilMsglen);
      pslMsghead->iBodylen = ilMsglen - sizeof(struct msghead);
    }
    if ( ilRc )
    {
      swVdebug(1,"S0220: [错误/函数调用] swTcprcv()函数,从服务端接收出错,errno=%d[%s]",errno,
        strerror(errno));
      signal(SIGALRM,SIG_IGN);
      alarm(0);
      close( igSockfd );
      igSockfd=swCheck_comm();
      if ( igSockfd < 0 )
      {
        swVdebug(1,"S0230: [错误/函数调用] swCheck_comm()函数");
        swMbclose();
        exit(-1);
      }
    }
    swVdebug(2,"S0240: 从服务器接收数据完毕! ilMsglen = %d",ilMsglen );
    if (cgDebug >= 2)  
    {
      swDebughex( alMsgbuf, ilMsglen );
    }

    /* Write to mq */
    ilRc = swSendpack(sgComcfg.iMb_fore_id, alMsgbuf,
      ilMsglen,ilPri,ilClass,ilType ); 
    if ( ilRc )
    {
      swVdebug(1,"S0250: [错误/函数调用] swSendpack()函数,写邮箱[%s]失败,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
      signal(SIGALRM,SIG_IGN);
      alarm(0);
      exit( -1 );
    }
    swVdebug(2,"S0260: 写邮箱[%d]成功! len=%d",sgComcfg.iMb_fore_id,ilMsglen);
    signal(SIGALRM,SIG_IGN);
    alarm(0);
  }
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSockfd);
  exit(0);
}

void swTimeout(int sig)
{
  signal(SIGALRM,SIG_IGN);
  alarm( 0 );
  swMbclose();
  close(igSockfd);
  swVdebug(0,"S0270: 通讯进程等待超时!");
  exit(2);
}
