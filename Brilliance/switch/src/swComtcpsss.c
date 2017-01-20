/****************************************************************/
/* ģ����    ��swComtcpsss                                    */
/* ģ������    : ͨ������-TCPͬ���̶�����server                 */
/* ��    ��    ��                                               */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/18                                      */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
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

  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpsss Version 4.3.0",argv[1]);

  /* ���õ��Գ������� */
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(2,"S0010: [����/����] ��������������Ϸ�!");
    exit(1);
  }
  
  /* �����ź� */
  swSiginit( );
    
  ilRc = swLoadcomcfg( argv[1] );
  if ( ilRc != 0 )
  {
    swVdebug(0,"S0020: [����/��������] swLoadcomcfg()����,[%s]װ��ͨѶ���ò�������,��������Ķ˿���,������=%d",argv[1],ilRc);
    exit( 1 );
  }
  
  swVdebug(2,"S0030: [%s]װ��ͨѶ���ò����ɹ�",argv[1]);
   
  memset((char *)&sgCli_addr,0x00,sizeof(struct sockaddr_in));

  /*������������Ӳ������ͻ���*/ 
  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);
  if (igSocket==-1) 
  {
    swVdebug(0,"S0040: [����/��������] swTcpconnect()����,�����������Ӵ���errno=%d[%s]",errno,strerror(errno));
    exit(-1);
  }
 
  swVdebug(2,"S0050: �������ӳɹ�");

  for(;;)
  {
    swVdebug(2,"S0060: forѭ������");
     
    ilAddrlen=sizeof(struct sockaddr_in);
    igSockfd=accept(igSocket,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if (igSockfd==-1)
    { 
      if (errno==EINTR)
        continue;
      swVdebug(1,"S0070: [����/ϵͳ����] accept()����,���Ӵ���!errno=%d[%s] ",errno,strerror(errno));
      close(igSocket); 
      exit(-1); 
    }
    swVdebug(2,"S0080: accept() SUCCESS!");
    switch(fork())
    {
      case -1:
        swVdebug(0,"S0090: fork�ӽ��̳���");
        close(igSocket);
        close(igSockfd);  
        exit( -1 );
      case 0:
        swVdebug(2,"S0100: �ӽ���");
        close(igSocket);
        swDoitsss();
  	exit(0);
      default:
        swVdebug(2,"S0110: ������");
   	close(igSockfd);
        break;
    }
  }
}  

/****************************************************************/
/* �������    ��swDoitsss                                      */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/18                                      */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   -1                                      */
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

  swVdebug(4,"S0120: [��������] swDoitssc()");
  /*��ÿͻ���������ַ*/ 
  strcpy(alCliaddr,inet_ntoa(sgCli_addr.sin_addr)); 

  swVdebug(2,"S0130: connect from %s port %d ",alCliaddr,
    ntohs(sgCli_addr.sin_port));
  
  /*��ÿͻ���������*/
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
    swVdebug(0,"S0140: [����/ϵͳ����] setsockopt()����,Connection with {%s} is aborted!errno=%d[%s]",
      alCliname,errno,strerror(errno));
    close(igSockfd);
    return(-1);
  }
  swVdebug(3,"S0150: setsockopt SUCCESS!");
 
  /*��ͨѶ����*/
  ilRc=swMbopen(sgComcfg.iMb_comm_id);
  if (ilRc)
  {
    swVdebug(0,"S0160: [����/����] ������[%d]����!", sgComcfg.iMb_comm_id );
    close( igSockfd );
    return(-1);
  }
      
  ilPri = 0 ;
  time(&llClass);
  llType = getpid();

  for(;;)
  {
    /*�ӿͻ��˽��ձ���*/
    ilMsglen = sizeof(alMsgbuf);
    
    ilRc = swTcprcv(igSockfd, alMsgbuf,&ilMsglen) ; 
    if (ilRc) 
    {
      swVdebug(1,"S0170: [����/��������] swTcprcv()����,�ӿͻ���{%s}���ձ��ĳ���,������=%d",alCliname,ilRc);
      close( igSockfd );
      return(-1);
    }
    swVdebug(2,"S0180: �ӿͻ���{%s}���ձ��ĳɹ�!Msglen=%d",alCliname,ilMsglen);
    if (cgDebug >= 2)  
      swDebughex(alMsgbuf,ilMsglen);
    /* ����ͨѶ��������־ */
    ilContflag = swComcheckcontpack(alMsgbuf,ilMsglen);

    if (sgComcfg.iMsghead == 0)
    {
      /* �ڱ���ǰ���ӿձ���ͷ */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }


    ilRc = swSendpackl(sgComcfg.iMb_fore_id,alMsgbuf,ilMsglen,
	ilPri, llClass, llType);
    if (ilRc) 
    {
      swVdebug(1,"S0190: [����/��������] swSendpackl()����,д���ĵ�ǰ������[%d]����,������=%d",sgComcfg.iMb_fore_id,ilRc);  
      swMbclose();
      close( igSockfd );
      return(-1);
    }

    swVdebug(2,"S0200: д���ĵ�ǰ������[%d]�ɹ�!",sgComcfg.iMb_fore_id);
    swVdebug(2,"S0210: ilPri=[%d],Class=[%ld],Type=[%ld]",ilPri,llClass,llType);
 
    if (ilContflag != 1) break;
  } /* end while */
 
  /* ���մ�ƽ̨���صı��� */
  for (;;) 
  {
    /*��ͨѶ����*/
    ilMsglen=sizeof(alMsgbuf);

    swVdebug(2,"S0220: ��ͨѶ����[%d]��ȡ����!",sgComcfg.iMb_comm_id);
    swVdebug(2,"S0230: ilPri=[%d],Class=[%ld],Type=[%ld]",ilPri,llClass,llType);
 
    ilRc = swRecvpackl(&ilQid,alMsgbuf,&ilMsglen,&ilPri,&llClass,&llType,
      sgComcfg.iTime_out ); 
    if (ilRc) 
    {
      if (ilRc==BMQ__TIMEOUT) 
      {
        swVdebug(1,"S0240: [����/��������] swRecvpackl()����,��ͨѶ����[%d]��ʱ!ilRc=%d",sgComcfg.iMb_comm_id,
          ilRc );  
        close( igSockfd );
        swMbclose();
        return(-1);
      }
      swVdebug(2,"S0250: ��ͨѶ����[%d]��Rcode=%d", sgComcfg.iMb_comm_id,
        ilRc ); 
      close( igSockfd );
      swMbclose();
      return(-1);
    }

    swVdebug(2,"S0260: ilMsglen=%d",ilMsglen);
    swVdebug(2,"S0270: ��ͨѶ���������:");
    if (cgDebug >= 3)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /*�ͻؿͻ��� */
    /*�ж����ޱ���ͷ*/
    if(sgComcfg.iMsghead == 0)
    {
      /* ��������ͷ���ⷢ */
      ilRc = swTcpsnd(igSockfd,alMsgbuf + sizeof(struct msghead),
        ilMsglen - sizeof(struct msghead));
    }
    else
    {
      ilRc = swTcpsnd(igSockfd,alMsgbuf,ilMsglen);
    }
    if (ilRc) 
    {
      swVdebug(1,"S0280: [����/��������] swTcpsnd()����,�ͻؿͻ��˳���,errno=%d[%s]",errno,
        strerror(errno) );
      close(igSockfd);
      swMbclose();
      return(-1);
    }
    swVdebug(2,"S0290: �ɹ��ͻؿͻ���-ilMsglen=%d\n",ilMsglen);
    if (cgDebug >= 3)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* next add by nh */
    /* ���ƽ̨���ر����Ƿ��к����� */
    ilContflag = swSwcheckcontpack(alMsgbuf);
    if (ilContflag == 0) break;
  }

  /*�ر�����*/
  ilRc=swMbclose();
  if (ilRc)
  {
    swVdebug(0,"S0300: [����/����] �ر�����[%d]����!", sgComcfg.iMb_comm_id );
    close (igSockfd );
    return(-1);
  }
  close ( igSockfd );
  swVdebug(4,"S0310: [��������] swDoitssc()������=0");
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
