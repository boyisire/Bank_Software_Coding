/****************************************************************/
/* ģ����    ��swComtcpslc                                    */
/* ģ������    : ͨ������-TCPͬ��������Client                   */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/16                                      */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
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

  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpslc Version 4.3.0",argv[1]);

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
    swVdebug(0,"S0020: [����/��������] swLoadcomcfg()����,[%s]װ��ͨѶ���ò�������,��������Ķ˿�,������=%d",argv[1],ilRc);
    exit( 1 );
  }
  swVdebug(2,"S0030: [%s]װ��ͨѶ���ò����ɹ�!",argv[1]);
   
  ilRc = swMbopen(sgComcfg.iMb_comm_id);
  if ( ilRc )
  {
    swVdebug(0,"S0040: [����/����] ������[%d]ʧ��",sgComcfg.iMb_comm_id);
    exit( -1 );
  }
      
  swVdebug(2,"S0050: ������[%d]�ɹ�!",sgComcfg.iMb_comm_id);
   
  /* �ö˿�ͨѶ��ΪUP */
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S0060: [����/�����ڴ�] �����ڴ������!");
    return(FAIL);
  }

  /* next modify by nh 2002.6.6
  ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
  ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
  if ( ilRc )
  {
    swVdebug(0,"S0070: [����/��������] swPortset()����,�ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
    swMbclose();
    exit(-1);
  }
  swVdebug(2,"S0080: ���ö˿ڳɹ�!");
  swVdebug(2,"S0090: ��ʼ���ӵ�������");
   
  /*���ӵ�������*/
  igSockfd = swConnectser( sgTcpcfg.aPartner_addr, sgTcpcfg.iPartner_port );
  if ( igSockfd < 0 )
  {
    igSockfd=swCheck_comm();
    if ( igSockfd < 0 )
    {
      swVdebug(0,"S0100: [����/��������] swCheck_comm()����,���ӵ�����������!");
      swMbclose();
      exit(-1);
    }
  }
  swVdebug(2,"S0110: ���ӵ�������{%s}�Ķ˿�[%u]�ɹ�!", \
    sgTcpcfg.aPartner_addr , ntohs(slCli_addr.sin_port));
  
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  
  ilRc=getsockname(igSockfd,(struct sockaddr *)&slCli_addr,&ilAddrlen);
  if ( ilRc == -1 )
  {
    swVdebug(0,"S0120: [����/ϵͳ����] getsockname()����,unable to read socket address,errno=%d[%s]",errno,strerror(errno));
    close( igSockfd );
    exit( -1 );
  }
  swVdebug(2,"S0130: getsockname SUCCESS!");
 
  for( ; ; )
  {
    swVdebug(2,"S0140: ѭ������");
  
    /* Read from mq */
    ilPri = 0;
    ilClass = 0;
    ilType  = 0;
    ilMsglen = sizeof( alMsgbuf );
    
    swVdebug(2,"S0150: ��ʼ�������ȡ��Ϣ!");
      
    ilRc = swRecvpackw(&ilQid, alMsgbuf,&ilMsglen,&ilPri,&ilClass,&ilType);
    if ( ilRc )
    {
      swVdebug(1,"S0160: [����/��������] swRecvpackw()����,��ȡ����[%d]ʧ��,������=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      close( igSockfd );
      exit( -1 );
    }

    swVdebug(2,"S0170: ��ȡ����[%d]�ɹ�! len=%d",ilQid,ilMsglen);
    if (cgDebug >= 2)  
    {
      swDebughex( alMsgbuf , ilMsglen ) ;         
    }

    pslMsghead = (struct msghead *)alMsgbuf; 

    alarm( 0 );
    signal(SIGALRM , swTimeout);
    alarm( sgComcfg.iTime_out );

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
      swVdebug(1,"S0180: [����/��������] swTcpsnd���ͱ��ĵ�����������,errno=%d[%s]",errno,strerror(errno));
      close( igSockfd );
      igSockfd=swCheck_comm();
      if ( igSockfd < 0 )
      {
        swVdebug(1,"S0190: [����/��������] swCheck_comm()����");
        swMbclose();
        signal(SIGALRM,SIG_IGN);
        alarm(0);
        exit(-1);
      }
    }
    swVdebug(2,"S0200: �Ӷ˿ڷ��ͱ��ĳɹ�! ilMsglen=[%d]",ilMsglen );
    if( cgDebug >= 2 )
    {
      swDebughex( alMsgbuf,ilMsglen);
    }

    /* �ӷ���˿ڽ��ձ��� */
    ilMsglen = sizeof(alMsgbuf);
    swVdebug(2,"S0210: �ȴ����շ���˵�����." );
 
    if(sgComcfg.iMsghead == 0)
    {
      /* ͨѶ���ز�������ͷ */
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
      swVdebug(1,"S0220: [����/��������] swTcprcv()����,�ӷ���˽��ճ���,errno=%d[%s]",errno,
        strerror(errno));
      signal(SIGALRM,SIG_IGN);
      alarm(0);
      close( igSockfd );
      igSockfd=swCheck_comm();
      if ( igSockfd < 0 )
      {
        swVdebug(1,"S0230: [����/��������] swCheck_comm()����");
        swMbclose();
        exit(-1);
      }
    }
    swVdebug(2,"S0240: �ӷ����������������! ilMsglen = %d",ilMsglen );
    if (cgDebug >= 2)  
    {
      swDebughex( alMsgbuf, ilMsglen );
    }

    /* Write to mq */
    ilRc = swSendpack(sgComcfg.iMb_fore_id, alMsgbuf,
      ilMsglen,ilPri,ilClass,ilType ); 
    if ( ilRc )
    {
      swVdebug(1,"S0250: [����/��������] swSendpack()����,д����[%s]ʧ��,������=%d",sgComcfg.iMb_fore_id,ilRc);
      signal(SIGALRM,SIG_IGN);
      alarm(0);
      exit( -1 );
    }
    swVdebug(2,"S0260: д����[%d]�ɹ�! len=%d",sgComcfg.iMb_fore_id,ilMsglen);
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
  swVdebug(0,"S0270: ͨѶ���̵ȴ���ʱ!");
  exit(2);
}
