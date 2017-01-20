/****************************************************************/
/* 模块编号    ：swComtcpalos                                   */
/* 模块名称    : 通信例程-TCP异步长连接单工server               */
/* 作    者    ：                                               */
/* 建立日期    ：2000/5/18                                      */
/* 最后修改日期：2001/9/18                                      */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/*                        (1)short swComtcpalos()               */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static  long    igSockfd;
static  long    igSocket;

void  swQuit();

int main( int argc, char **argv )
{
  int    ilRc;

  #ifdef OS_SCO
    int  ilAddrlen;
  #else
    socklen_t  ilAddrlen;
  #endif

  struct hostent *pslHp;
  struct linger slLinger;
  char   alCliname[80];
  unsigned int ilMsglen,ilPri,ilClass,ilType;
  struct sockaddr_in slCli_addr;
  char   alMsgbuf[iMSGMAXLEN];

  /* 取得版本号 */
  if(argc > 1)
    _swVersion("swComtcpals Version 4.3.0",argv[1]);

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
    swVdebug(0,"S0020: [错误/函数调用] swLoadcomcfg()函数,[%s]装入通讯配置参数出错,请检查输入的端口名,返回码=%d",argv[1],ilRc);
    exit( -1 );
  }
  swVdebug(2,"S0030: [%s]装入通讯配置参数成功!",argv[1]);
   
  memset ((char *)&slCli_addr, 0x00, sizeof(struct sockaddr_in) );

  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);  
  if (igSocket == -1)
  {
    swVdebug(0,"S0040: [错误/函数调用] swTcpconnect()函数,建立服务端连接失败!");
    exit(-1);
  }
  swVdebug(2,"S0050: 建立连接成功!");
 
  /*接收连接*/
  ilAddrlen = sizeof(struct sockaddr_in);

  igSockfd = accept((int)igSocket,(struct sockaddr*)&slCli_addr,&ilAddrlen);
  if (igSockfd == -1)
  {
    swVdebug(2,"S0060: [错误/系统调用] accept()连接错误,errno=%d[%s]",errno,strerror(errno));
    close(igSocket);
    exit(-1);
  }
  
  /*获得客户端主机名*/
  pslHp = gethostbyaddr( (char *) &slCli_addr.sin_addr, sizeof(struct in_addr),
    slCli_addr.sin_family);

  if (pslHp == NULL)
    strcpy( alCliname, inet_ntoa( slCli_addr.sin_addr ) );
  else
    strcpy( alCliname, pslHp->h_name );

  swVdebug(2,"S0070: gethostbyaddr SUCCESS!");

  slLinger.l_onoff  = 1;
  slLinger.l_linger = 0;

  ilRc = setsockopt(igSockfd,SOL_SOCKET,SO_LINGER,&slLinger,
    sizeof(struct linger));
  if ( ilRc == -1 )
  {
    swVdebug(0,"S0080: [错误/系统调用] Connect error!errno=%d[%s]",errno,strerror(errno));
    close (igSocket);
    close (igSockfd);
    exit(-1);
  }

  swVdebug(2,"S0090: setsockopt SUCCESS!");
 
  ilRc = swMbopen( sgComcfg.iMb_comm_id );
  if ( ilRc )
  {
    swVdebug(0,"S0100: [错误/邮箱] 打开邮箱[%d]出错!",sgComcfg.iMb_comm_id );
    close(igSocket);
    close(igSockfd);
    exit(-1);
  }
  swVdebug(2,"S0110: 打开邮箱成功!");

  for(;;)
  {
    /*从客户端接收报文*/
    ilMsglen = sizeof( alMsgbuf );

    ilRc = swTcprcv(igSockfd, alMsgbuf, &ilMsglen);
    if ( ilRc )
    {
      swVdebug(1,"S0120: [错误/函数调用] swTcprcv()函数,从客户端{%s}接收报文出错,返回码=%d",alCliname,ilRc);
      close(igSocket);
      close(igSockfd);
      swMbclose();
      exit(-1);
    }

    swVdebug(2,"S0130: 从客户端接收报文成功!");

    if (cgDebug >= 2)  
      swDebughex(alMsgbuf,ilMsglen);

    /*处理有无报文头*/ 
    if (sgComcfg.iMsghead == 0)
    {
      /* 在报文前增加空报文头 */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }
    swVdebug(2,"S0140: 从客户端{%s}接收成功!ilMsglen=%d",alCliname,ilMsglen);
  
    /*写入前置邮箱*/
    ilPri = 0 ;
    ilClass = 0 ;
    ilType  = 0 ;

    ilRc=swSendpack(sgComcfg.iMb_fore_id,alMsgbuf,ilMsglen,ilPri,ilClass,
      ilType);
    if ( ilRc )
    {
      swVdebug(1,"S0150: [错误/函数调用] swSendpack()函数,写报文到邮箱[%d]出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit(-1);
    }

    swVdebug(2,"S0160: 写报文到邮箱[%d]成功!",sgComcfg.iMb_fore_id );
   }
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSocket);
  close(igSockfd);
  exit(0);
}

  
