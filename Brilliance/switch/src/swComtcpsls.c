/****************************************************************/
/* ģ����    ��swComtcpsls                                    */
/* ģ������    : ͨ������-TCPͬ��������server                   */
/* ��    ��    ��     �                                         */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/18                                      */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
/*                        (1)int swComtcpsls()                  */
/*                        (2)int swDoitsls()                    */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static long    igSockfd;
static long    igSocket;

void swQuit();
int swDoitsls();

int main( int argc , char **argv )
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
  char   alCliaddr[16];
  unsigned int ilQid,ilMsglen,ilPri,ilClass,ilType;
  char   alMsgbuf[iMSGMAXLEN];
  struct sockaddr_in slCli_addr;
  
  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpsls Version 4.3.0",argv[1]);

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
  if ( ilRc != 0 )
  {
    swVdebug(0,"S0020: [����/��������] swLoadcomcfg()����[%s]װ��ͨѶ���ò�������,��������Ķ˿���,������=%d",argv[1],ilRc);
    exit( 1 );
  }
  
  swVdebug(2,"S0030: [%s]װ��ͨѶ���ò����ɹ�",argv[1]);
    
  memset(alCliaddr,0x0,sizeof(alCliaddr));
  memset((char *)&slCli_addr, 0x00, sizeof(struct sockaddr_in) );

  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);
  if (igSocket == -1)
  {
    swVdebug(0,"S0040: [����/��������] swTcpconnect()ERROR!");
    exit( -1 );
  }
  swVdebug(2,"S0050: �������ӳɹ�");
   
  /*��ͨѶ����*/
  ilRc = swMbopen( sgComcfg.iMb_comm_id ); 
  if ( ilRc )
  {
    swVdebug(0,"S0060: [����/����] ������[%d]����!",sgComcfg.iMb_comm_id );
    close( igSocket );
    exit( -1 );
  }
  swVdebug(2,"S0070: ������ɹ�");
  
  /*��������*/
  swVdebug(2,"S0080: ��ʼ����");
  ilAddrlen = sizeof(struct sockaddr_in);
  
  igSockfd  = accept(igSocket,(struct sockaddr*)&slCli_addr,&ilAddrlen);
  if ( igSockfd == -1)
  {
    swVdebug(0,"S0090: [����/ϵͳ����] ���Ӵ���,errno=%d[%s]",errno,strerror(errno));
    close(igSocket);
    swMbclose();
    exit( -1 );
  }
  swVdebug(2,"S0100: accept() SUCCESS!");
 
  strcpy(alCliaddr,inet_ntoa(slCli_addr.sin_addr)); 
  
  swVdebug(2,"S0110: alCliaddr=%s",alCliaddr);
  
  /*��ÿͻ���������*/
  pslHp = gethostbyaddr( (char *) &slCli_addr.sin_addr, sizeof(struct in_addr),
    slCli_addr.sin_family);
  if ( pslHp == NULL)
    strcpy( alCliname, inet_ntoa( slCli_addr.sin_addr ) );
  else
    strcpy( alCliname, pslHp->h_name );

  swVdebug(2,"S0120: gethostbyaddr SUCCESS");
    
  slLinger.l_onoff  =1;
  slLinger.l_linger =0;
  
  ilRc = setsockopt(igSockfd,SOL_SOCKET,SO_LINGER,&slLinger,
    sizeof(struct linger));
  if ( ilRc == -1 )
  {
    swVdebug(0,"S0130: [����/ϵͳ����] setsockopt(),Connect error!errno=[%d,%s]",errno,strerror(errno));
    close( igSocket );
    close( igSockfd );
    swMbclose();
    exit( -1 );
  }
  swVdebug(2,"S0140: setsockopt SUCCESS!");
 
  for(;;)
  {
    /*�ӿͻ��˽��ձ���*/
    ilMsglen = sizeof( alMsgbuf );
  
    ilRc = swTcprcv(igSockfd, alMsgbuf, &ilMsglen );
    if ( ilRc )
    {
      swVdebug(1,"S0150: [����/��������] swTcprcv()����,�ӿͻ���{%s}���ձ��ĳ���,������=%d",alCliname,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }
    swVdebug(2,"S0160: �ӿͻ��˽��ձ��ĳɹ�");
    if (cgDebug >= 2) 
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* next add by nh*/
    if (sgComcfg.iMsghead == 0)
    {
      /* �ڱ���ǰ���ӿձ���ͷ */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }

    swVdebug(2,"S0170: �ӿͻ���{%s}���ձ������!len=%d",alCliname,ilMsglen);
 
    ilPri = 0 ;
    ilClass = 0 ;
    ilType  = 0 ;
 
    ilRc = swSendpack(sgComcfg.iMb_fore_id, alMsgbuf, ilMsglen, ilPri, 
      ilClass, ilType);
    if (ilRc)
    {
      swVdebug(1,"S0180: [����/��������] swSendpack()����,д���ĵ�����[%d]����,������=%d",ilQid,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }

      swVdebug(2,"S0190: д���ĵ�����ɹ�");
      swVdebug(2,"S0200: Writed to Qid=[%d],ilPri=[%d],Class=[%d],Type=[%d]",\
        ilQid, ilPri, ilClass, ilType );
        
    /*��ͨѶ����*/
    swVdebug(2,"S0210: �ȴ���ͨѶ����");
  
    ilMsglen = sizeof( alMsgbuf );
    ilPri = 0 ;
    ilClass = 0 ;
    ilType = 0 ;
 
    ilRc = swRecvpack( &ilQid, alMsgbuf, &ilMsglen, &ilPri, &ilClass,\
      &ilType , sgComcfg.iTime_out );
    if ( ilRc)
    {
      if ( ilRc == BMQ__TIMEOUT)
      {
        swVdebug(1,"S0220: [����/��������] swRecvpack()����,������[%d]��ʱ,������=%d",ilQid,ilRc);
        continue;
      }
      swVdebug(1,"S0230: [����/��������] swRecvpack()����,������[%d]����,������=%d",ilQid,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit(-1);
    }
    swVdebug(2,"S0240: swRecvpack() SUCCESS");
    if (cgDebug >= 2)
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
    if ( ilRc )
    {
      swVdebug(1,"S0250: [����/��������] swTcpsnd()���� �ͻؿͻ��˳���,������=%d",ilRc );
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }
    swVdebug(2,"S0260: �ѳɹ��ͻؿͻ���!ilMsglen = %d",ilMsglen);
    if (cgDebug >= 2) 
    {
      swDebughex(alMsgbuf,ilMsglen);
    }
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


