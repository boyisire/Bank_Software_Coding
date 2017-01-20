/****************************************************************/
/* 模块编号    ：swComtcpass                                    */
/* 模块名称    : 通信例程-TCP同步短短连接server                 */
/* 作    者    ：                                               */
/* 建立日期    ：2000/5/18                                      */
/* 最后修改日期：2001/9/                                        */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/*                        (1)int swComtcpass()                  */
/*                        (2)int swDoitass()                    */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static struct sockaddr_in sgCli_addr;
static int  igSockfd,igSocket;

int swDoitass();
void swQuit();
extern int swSendpackl(short, char *, unsigned int, short, long, long);

int main(int argc, char **argv)
{
  int    ilRc;

  #ifdef OS_SCO
    int  ilAddrlen;
  #else
    socklen_t  ilAddrlen;
  #endif

  /* 取得版本号 */
  if(argc > 1)
    _swVersion("swComtcpass Version 4.3.0",argv[1]);
  
  /* 设置调试程序名称 */ 
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(0,"S0010: [错误/其它] 传入参数数量不合法!");
    exit( 1 );
  }
  
  /* 处理信号 */
  swSiginit( );
  
  ilRc = swLoadcomcfg( argv[1] );
  if (ilRc)
  {
    swVdebug(0,"S0020: [错误/函数调用] swLoadcomcfg()函数,[%s]装入通讯配置参数出错,请检查输入的端口名,返回码=%d",argv[1],ilRc);
    exit( -1 );
  }

  swVdebug(2,"S0030: [%s]装入通讯配置参数成功",argv[1]);
 
  memset ((char *)&sgCli_addr, 0x00, sizeof(struct sockaddr_in) );

  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);
  if (igSocket == -1)
  {
    swVdebug(0,"S0040: [错误/函数调用] swTcpconnect()连接出错!");
    exit( -1 );
  }
  swVdebug(2,"S0050: 建立连接成功");
    
  for (;;)
  {
    swVdebug(2,"S0060: 循环处理");
 
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(igSocket,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      swVdebug(0,"S0070: [错误/系统调用] 连接错误 errno=%d[%s]",errno,strerror(errno));
      close( igSockfd ); 
      exit( -1 ); 
    }
    swVdebug(2,"S0080: accept() SUCCESS");
     
    switch ( fork() )
    {
      case -1:
        swVdebug(2,"S0090: fork子进程出错!");
        close( igSocket );
        close( igSockfd );  
  	exit( -1 );
      case 0:	
        swVdebug(2,"S0100: 子进程处理");
        close( igSocket );
        swDoitass();
 	exit(0);
      default:
        swVdebug(2,"S0110: 父进程处理");
        close( igSockfd );
        break;
    }/* end switch */
  } /* end for */
}  

/****************************************************************/
/* 函数编号    ：swDoitass                                      */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/18                                      */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   -1                                      */
/****************************************************************/
int swDoitass()
{
  struct hostent *pslHp;
  struct linger slLinger;

  char   alCliname[80];
  char   alCliaddr[16];
  int    ilRc;
  short  ilContflag;
  unsigned int  ilMsglen;
  short  ilPri;
  long   llClass,llType; 
  char   alMsgbuf[iMSGMAXLEN];

  swVdebug(4,"S0120: [函数调用] swDoitass()");
  /*获得客户端主机地址*/ 
  memset(alCliaddr,0x0,sizeof(alCliaddr));
  strcpy(alCliaddr,inet_ntoa(sgCli_addr.sin_addr)); 
  swVdebug(2,"S0130: connect from %s port %d ",alCliaddr,
    ntohs(sgCli_addr.sin_port));
  
  /*获得客户端主机名*/
  pslHp=gethostbyaddr((char *)&sgCli_addr.sin_addr,sizeof(struct in_addr),
    sgCli_addr.sin_family);
  if (pslHp==NULL)
    strcpy(alCliname,inet_ntoa(sgCli_addr.sin_addr));
  else
    strcpy(alCliname,pslHp->h_name);
  
  swVdebug(2,"S0140: gethostbyaddr SUCCESS");
         
  slLinger.l_onoff  =1;
  slLinger.l_linger =1;
  
  ilRc=setsockopt(igSockfd,SOL_SOCKET,SO_LINGER,&slLinger,
    sizeof(struct linger));
  if ( ilRc == -1 ) 
  {
    swVdebug(2,"S0150: Connection with {%s} is aborted!--errno[%d,%s]", \
      alCliname,errno,strerror(errno));
    close(igSockfd);
    return(-1);
  }
  swVdebug(2,"S0160: setsockopt SUCCESS");
  
  /*打开通讯邮箱*/
  ilRc = swMbopen( sgComcfg.iMb_comm_id );
  if(ilRc)
  {
    swVdebug(0,"S0170: [错误/邮箱] 打开邮箱[%d]出错,返回码=%d", sgComcfg.iMb_comm_id,ilRc );
    close( igSockfd );
    swMbclose();
    return(-1);
  }
  swVdebug(2,"S0180: 打开邮箱成功");
  
  ilPri = 0 ;
  time(&llClass);
  llType = getpid();

  for (;;)      
  {
    /*从客户端接收报文*/
    ilMsglen = sizeof(alMsgbuf);

    ilRc = swTcprcv(igSockfd, alMsgbuf,&ilMsglen); 
    if (ilRc) 
    {
      swVdebug(1,"S0190: [错误/函数调用] swTcprcv()函数,从客户端{%s}接收报文出错,返回码=%d",alCliname,ilRc);
      close( igSockfd );
      swMbclose();
      return(-1);
    }
    swVdebug(2,"S0200: 从客户端{%s}接收报文完毕!len=%d",alCliname,ilMsglen);
    if (cgDebug >= 2)
      swDebughex(alMsgbuf,ilMsglen);
 
    /* 计算通讯后续包标志 */
    ilContflag = swComcheckcontpack(alMsgbuf,ilMsglen);
 
    /*处理有无报文头*/ 
    if( sgComcfg.iMsghead == 0)
    {
    /* 在报文前增加空报文头 */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }

    ilRc = swSendpackl(sgComcfg.iMb_fore_id, alMsgbuf, ilMsglen, ilPri, 
      llClass, llType);
    if (ilRc)
    {
      swVdebug(1,"S0210: [错误/函数调用] swSendpackl()函数,写报文到邮箱[%d]出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);    
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }

    swVdebug(2,"S0220: 已成功发送到邮箱[%d],ilPri=[%d],Class=[%ld],Type=[%ld]",\
      sgComcfg.iMb_fore_id, ilPri, llClass, llType );

    if (ilContflag != 1) break;
  } /* end for */
  
  swMbclose();
  close ( igSockfd );
  swVdebug(4,"S0230: [函数返回] swDoitass()返回码=0");
  return 0;
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSocket);
  close(igSockfd);
  exit(0);
}

