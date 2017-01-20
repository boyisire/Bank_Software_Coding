/****************************************************************/
/* ģ����    ��swComtcpalos                                   */
/* ģ������    : ͨ������-TCP�첽�����ӵ���server               */
/* ��    ��    ��                                               */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/18                                      */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
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

  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpals Version 4.3.0",argv[1]);

  /* ���õ��Գ������� */
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(0,"S0010: [����/����] ��������������Ϸ�!");
    exit(1);
  }
  
  /* �����ź� */
  swSiginit( );
 

  ilRc = swLoadcomcfg( argv[1] );
  if ( ilRc )
  {
    swVdebug(0,"S0020: [����/��������] swLoadcomcfg()����,[%s]װ��ͨѶ���ò�������,��������Ķ˿���,������=%d",argv[1],ilRc);
    exit( -1 );
  }
  swVdebug(2,"S0030: [%s]װ��ͨѶ���ò����ɹ�!",argv[1]);
   
  memset ((char *)&slCli_addr, 0x00, sizeof(struct sockaddr_in) );

  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);  
  if (igSocket == -1)
  {
    swVdebug(0,"S0040: [����/��������] swTcpconnect()����,�������������ʧ��!");
    exit(-1);
  }
  swVdebug(2,"S0050: �������ӳɹ�!");
 
  /*��������*/
  ilAddrlen = sizeof(struct sockaddr_in);

  igSockfd = accept((int)igSocket,(struct sockaddr*)&slCli_addr,&ilAddrlen);
  if (igSockfd == -1)
  {
    swVdebug(2,"S0060: [����/ϵͳ����] accept()���Ӵ���,errno=%d[%s]",errno,strerror(errno));
    close(igSocket);
    exit(-1);
  }
  
  /*��ÿͻ���������*/
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
    swVdebug(0,"S0080: [����/ϵͳ����] Connect error!errno=%d[%s]",errno,strerror(errno));
    close (igSocket);
    close (igSockfd);
    exit(-1);
  }

  swVdebug(2,"S0090: setsockopt SUCCESS!");
 
  ilRc = swMbopen( sgComcfg.iMb_comm_id );
  if ( ilRc )
  {
    swVdebug(0,"S0100: [����/����] ������[%d]����!",sgComcfg.iMb_comm_id );
    close(igSocket);
    close(igSockfd);
    exit(-1);
  }
  swVdebug(2,"S0110: ������ɹ�!");

  for(;;)
  {
    /*�ӿͻ��˽��ձ���*/
    ilMsglen = sizeof( alMsgbuf );

    ilRc = swTcprcv(igSockfd, alMsgbuf, &ilMsglen);
    if ( ilRc )
    {
      swVdebug(1,"S0120: [����/��������] swTcprcv()����,�ӿͻ���{%s}���ձ��ĳ���,������=%d",alCliname,ilRc);
      close(igSocket);
      close(igSockfd);
      swMbclose();
      exit(-1);
    }

    swVdebug(2,"S0130: �ӿͻ��˽��ձ��ĳɹ�!");

    if (cgDebug >= 2)  
      swDebughex(alMsgbuf,ilMsglen);

    /*�������ޱ���ͷ*/ 
    if (sgComcfg.iMsghead == 0)
    {
      /* �ڱ���ǰ���ӿձ���ͷ */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }
    swVdebug(2,"S0140: �ӿͻ���{%s}���ճɹ�!ilMsglen=%d",alCliname,ilMsglen);
  
    /*д��ǰ������*/
    ilPri = 0 ;
    ilClass = 0 ;
    ilType  = 0 ;

    ilRc=swSendpack(sgComcfg.iMb_fore_id,alMsgbuf,ilMsglen,ilPri,ilClass,
      ilType);
    if ( ilRc )
    {
      swVdebug(1,"S0150: [����/��������] swSendpack()����,д���ĵ�����[%d]����,������=%d",sgComcfg.iMb_fore_id,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit(-1);
    }

    swVdebug(2,"S0160: д���ĵ�����[%d]�ɹ�!",sgComcfg.iMb_fore_id );
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

  
