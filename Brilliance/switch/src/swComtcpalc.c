/**************************************************************/
/* ģ����    ��swComtcpalc                                  */
/* ģ������    : ͨ������-TCP�첽������Client                 */
/* ��	 ��    ��                                             */
/* ��������    ��2000/5/18                                    */
/* ����޸����ڣ�2001/9/18                                    */
/* ģ����;    ��                                             */
/* ��ģ���а������º���������˵����                           */
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

  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpalc Version 4.3.0",argv[1]);
  
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
    exit( -1 );
  }
  swVdebug(2,"S0030: [%s]װ��ͨѶ���ò����ɹ�!",argv[1]);
   
  ilRc = swMbopen(sgComcfg.iMb_comm_id);
  if ( ilRc )
  {
    swVdebug(0,"S0040: [����/��������] swMbopen()����,������[%d]����,������=%d",sgComcfg.iMb_comm_id,ilRc);
    exit( -1 );
  }
      
  swVdebug(2,"S0050: ������[%d]�ɹ�!",sgComcfg.iMb_comm_id);
   
  /* �ö˿�ͨѶ��ΪUP */
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S0060: [����/�����ڴ�] �����ڴ������!");
    swMbclose();
    return(FAIL);
  }

  /* next modify by nh 2002.6.6 
  ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 ); */
  ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 ); 
  if ( ilRc )
  {
    swVdebug(0,"S0070: [����/��������] swPortset()����,�ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
    swMbclose();
    exit(-1);
  }
  swVdebug(2,"S0080: ���ö˿ڳɹ�!");

  /*���ӵ�������*/
  igSockfd = swConnectser( sgTcpcfg.aPartner_addr, sgTcpcfg.iPartner_port );
  if ( igSockfd < 0 )
  {
    igSockfd=swCheck_comm();
    if ( igSockfd <0 )
    {
      swVdebug(0,"S0090: [����/��������] swCheck_comm()����,���ӵ�����������!",sgComcfg.iMb_comm_id);
      swMbclose();
      close(igSockfd);
      exit(-1);
    }
  }	
  swVdebug(2,"S0100: ���ӵ�������{%s}�Ķ˿�[%u]�ɹ�!",\
    sgTcpcfg.aPartner_addr ,ntohs(sgTcpcfg.iPartner_port));
  
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));

  ilRc=getsockname(igSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen);
  if(ilRc == -1)
  {
    swVdebug(0,"S0110: [����/ϵͳ����] unable to read socket address");
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
  
    swVdebug(2,"S0130: ��ʼ�����������Ϣ !");
 
    ilRc = swRecvpackw(&ilQid, (char * )&alMsgbuf,&ilMsglen,
      &ilPri,&ilClass,&ilType);
    if ( ilRc )
    {
      swVdebug(1,"S0140: [����/��������] swRecvpackw()����,������[%d]����,������=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      close( igSockfd );
      exit( -1 );
    }

    swVdebug(2,"S0150: ������[%d]�յ�����Ϊ[%d]����Ϣ!",ilMsglen,ilQid);
 
    if (cgDebug >= 2)  
      swDebughex( alMsgbuf , ilMsglen );      
    
    pslMsghead = (struct msghead *)alMsgbuf;

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
    if ( ilRc < 0 )
    {
      close(igSockfd);
      igSockfd=swCheck_comm();
      if ( igSockfd <0 )
      { 
        swVdebug(1,"S0160: [����/��������] swCheck_comm()����,���ķ��͵�����˳���!");
        continue;
      }
    }
    swVdebug(2,"S0170: �����ѳɹ����͵������!");
   }
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSockfd);
  exit(0);
}

