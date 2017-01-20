/****************************************************************/
/* 模块编号    ：swComtcpsss                                    */
/* 模块名称    : 通信例程-TCP同步短短连接server                 */
/* 作    者    ：                                               */
/* 建立日期    ：2000/5/18                                      */
/* 最后修改日期：2001/9/18                                      */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/*                        (1)int swComtcpsss()                  */
/*                        (1)int swDoitsss()                    */
/****************************************************************/
#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static  struct sockaddr_in sgCli_addr;
static  int    igSockfd;
static  int    igSocket;

void swQuit();
int swDoitsss();
extern int swSendpackl(short, char *, unsigned int, short, long, long);
extern int swRecvpackl(short *,char *,unsigned int *,short *,long *,long *,short);

int main( int argc , char **argv )
{
  int    ilRc;

  #ifdef OS_SCO
    int  ilAddrlen;
  #else
    socklen_t  ilAddrlen;
  #endif

  /* 取得版本号 */
  if(argc > 1)
    _swVersion("swComtcpsss Version 4.3.0",argv[1]);

  /* 设置调试程序名称 */
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(2,"S0010: [错误/其它] 传入参数数量不合法!");
    exit(1);
  }
  
  /* 处理信号 */
  swSiginit( );
    
  ilRc = swLoadcomcfg( argv[1] );
  if ( ilRc != 0 )
  {
    swVdebug(0,"S0020: [错误/函数调用] swLoadcomcfg()函数,[%s]装入通讯配置参数出错,请检查输入的端口名,返回码=%d",argv[1],ilRc);
    exit( 1 );
  }
  
  swVdebug(2,"S0030: [%s]装入通讯配置参数成功",argv[1]);
   
  memset((char *)&sgCli_addr,0x00,sizeof(struct sockaddr_in));

  /*建立服务端连接并侦听客户端*/ 
  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);
  if (igSocket==-1) 
  {
    swVdebug(0,"S0040: [错误/函数调用] swTcpconnect()函数,建立侦听连接错误：errno=%d[%s]",errno,strerror(errno));
    exit(-1);
  }
 
  swVdebug(2,"S0050: 建立连接成功");

  for(;;)
  {
    swVdebug(2,"S0060: for循环处理");
     
    ilAddrlen=sizeof(struct sockaddr_in);
    igSockfd=accept(igSocket,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if (igSockfd==-1)
    { 
      if (errno==EINTR)
        continue;
      swVdebug(1,"S0070: [错误/系统调用] accept()函数,连接错误!errno=%d[%s] ",errno,strerror(errno));
      close(igSocket); 
      exit(-1); 
    }
    swVdebug(2,"S0080: accept() SUCCESS!");
    switch(fork())
    {
      case -1:
        swVdebug(0,"S0090: fork子进程出错");
        close(igSocket);
        close(igSockfd);  
        exit( -1 );
      case 0:
        swVdebug(2,"S0100: 子进程");
        close(igSocket);
        swDoitsss();
  	exit(0);
      default:
        swVdebug(2,"S0110: 父进程");
   	close(igSockfd);
        break;
    }
  }
}  

/****************************************************************/
/* 函数编号    ：swDoitsss                                      */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/18                                      */
/* 最后修改日期：2001/9/18                                      */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   -1                                      */
/****************************************************************/
int swDoitsss( )
{
  struct hostent *pslHp;
  struct linger slLinger;
  
  char  alCliname[80];
  char  alCliaddr[16];
  int   ilRc;
  short ilContflag;
  unsigned int ilMsglen;
  short ilQid,ilPri;
  long  llClass,llType; 
  char  alMsgbuf[iMSGMAXLEN];

  swVdebug(4,"S0120: [函数调用] swDoitssc()");
  /*获得客户端主机地址*/ 
  strcpy(alCliaddr,inet_ntoa(sgCli_addr.sin_addr)); 

  swVdebug(2,"S0130: connect from %s port %d ",alCliaddr,
    ntohs(sgCli_addr.sin_port));
  
  /*获得客户端主机名*/
  pslHp=gethostbyaddr((char *)&sgCli_addr.sin_addr,sizeof(struct in_addr),
    sgCli_addr.sin_family);
  if (pslHp == NULL)
    strcpy(alCliname,inet_ntoa(sgCli_addr.sin_addr));
  else
    strcpy(alCliname,pslHp->h_name);

  slLinger.l_onoff  =1;
  slLinger.l_linger =1;
  
  ilRc = setsockopt(igSockfd,SOL_SOCKET,SO_LINGER,&slLinger,
    sizeof(struct linger));
  if (ilRc==-1) 
  {
    swVdebug(0,"S0140: [错误/系统调用] setsockopt()函数,Connection with {%s} is aborted!errno=%d[%s]",
      alCliname,errno,strerror(errno));
    close(igSockfd);
    return(-1);
  }
  swVdebug(3,"S0150: setsockopt SUCCESS!");
 
  /*打开通讯邮箱*/
  ilRc=swMbopen(sgComcfg.iMb_comm_id);
  if (ilRc)
  {
    swVdebug(0,"S0160: [错误/邮箱] 打开邮箱[%d]出错!", sgComcfg.iMb_comm_id );
    close( igSockfd );
    return(-1);
  }
      
  ilPri = 0 ;
  time(&llClass);
  llType = getpid();

  for(;;)
  {
    /*从客户端接收报文*/
    ilMsglen = sizeof(alMsgbuf);
    
    ilRc = swTcprcv(igSockfd, alMsgbuf,&ilMsglen) ; 
    if (ilRc) 
    {
      swVdebug(1,"S0170: [错误/函数调用] swTcprcv()函数,从客户端{%s}接收报文出错,返回码=%d",alCliname,ilRc);
      close( igSockfd );
      return(-1);
    }
    swVdebug(2,"S0180: 从客户端{%s}接收报文成功!Msglen=%d",alCliname,ilMsglen);
    if (cgDebug >= 2)  
      swDebughex(alMsgbuf,ilMsglen);
    /* 计算通讯后续包标志 */
    ilContflag = swComcheckcontpack(alMsgbuf,ilMsglen);

    if (sgComcfg.iMsghead == 0)
    {
      /* 在报文前增加空报文头 */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }


    ilRc = swSendpackl(sgComcfg.iMb_fore_id,alMsgbuf,ilMsglen,
	ilPri, llClass, llType);
    if (ilRc) 
    {
      swVdebug(1,"S0190: [错误/函数调用] swSendpackl()函数,写报文到前置邮箱[%d]出错,返回码=%d",sgComcfg.iMb_fore_id,ilRc);  
      swMbclose();
      close( igSockfd );
      return(-1);
    }

    swVdebug(2,"S0200: 写报文到前置邮箱[%d]成功!",sgComcfg.iMb_fore_id);
    swVdebug(2,"S0210: ilPri=[%d],Class=[%ld],Type=[%ld]",ilPri,llClass,llType);
 
    if (ilContflag != 1) break;
  } /* end while */
 
  /* 接收从平台返回的报文 */
  for (;;) 
  {
    /*读通讯邮箱*/
    ilMsglen=sizeof(alMsgbuf);

    swVdebug(2,"S0220: 从通讯邮箱[%d]收取报文!",sgComcfg.iMb_comm_id);
    swVdebug(2,"S0230: ilPri=[%d],Class=[%ld],Type=[%ld]",ilPri,llClass,llType);
 
    ilRc = swRecvpackl(&ilQid,alMsgbuf,&ilMsglen,&ilPri,&llClass,&llType,
      sgComcfg.iTime_out ); 
    if (ilRc) 
    {
      if (ilRc==BMQ__TIMEOUT) 
      {
        swVdebug(1,"S0240: [错误/函数调用] swRecvpackl()函数,读通讯邮箱[%d]超时!ilRc=%d",sgComcfg.iMb_comm_id,
          ilRc );  
        close( igSockfd );
        swMbclose();
        return(-1);
      }
      swVdebug(2,"S0250: 读通讯邮箱[%d]错Rcode=%d", sgComcfg.iMb_comm_id,
        ilRc ); 
      close( igSockfd );
      swMbclose();
      return(-1);
    }

    swVdebug(2,"S0260: ilMsglen=%d",ilMsglen);
    swVdebug(2,"S0270: 从通讯邮箱读报文:");
    if (cgDebug >= 3)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /*送回客户端 */
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
    if (ilRc) 
    {
      swVdebug(1,"S0280: [错误/函数调用] swTcpsnd()函数,送回客户端出错,errno=%d[%s]",errno,
        strerror(errno) );
      close(igSockfd);
      swMbclose();
      return(-1);
    }
    swVdebug(2,"S0290: 成功送回客户端-ilMsglen=%d\n",ilMsglen);
    if (cgDebug >= 3)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* next add by nh */
    /* 检查平台返回报文是否有后续包 */
    ilContflag = swSwcheckcontpack(alMsgbuf);
    if (ilContflag == 0) break;
  }

  /*关闭邮箱*/
  ilRc=swMbclose();
  if (ilRc)
  {
    swVdebug(0,"S0300: [错误/邮箱] 关闭邮箱[%d]出错!", sgComcfg.iMb_comm_id );
    close (igSockfd );
    return(-1);
  }
  close ( igSockfd );
  swVdebug(4,"S0310: [函数返回] swDoitssc()返回码=0");
  return(0);
}


void swQuit(int sig)
{ 
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSocket);
  close(igSockfd);
  exit(0);
}
