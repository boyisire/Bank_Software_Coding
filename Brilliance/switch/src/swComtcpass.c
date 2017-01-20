/****************************************************************/
/* ģ����    ��swComtcpass                                    */
/* ģ������    : ͨ������-TCPͬ���̶�����server                 */
/* ��    ��    ��                                               */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/                                        */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
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

  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpass Version 4.3.0",argv[1]);
  
  /* ���õ��Գ������� */ 
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(0,"S0010: [����/����] ��������������Ϸ�!");
    exit( 1 );
  }
  
  /* �����ź� */
  swSiginit( );
  
  ilRc = swLoadcomcfg( argv[1] );
  if (ilRc)
  {
    swVdebug(0,"S0020: [����/��������] swLoadcomcfg()����,[%s]װ��ͨѶ���ò�������,��������Ķ˿���,������=%d",argv[1],ilRc);
    exit( -1 );
  }

  swVdebug(2,"S0030: [%s]װ��ͨѶ���ò����ɹ�",argv[1]);
 
  memset ((char *)&sgCli_addr, 0x00, sizeof(struct sockaddr_in) );

  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);
  if (igSocket == -1)
  {
    swVdebug(0,"S0040: [����/��������] swTcpconnect()���ӳ���!");
    exit( -1 );
  }
  swVdebug(2,"S0050: �������ӳɹ�");
    
  for (;;)
  {
    swVdebug(2,"S0060: ѭ������");
 
    ilAddrlen = sizeof(struct sockaddr_in);
    igSockfd  = accept(igSocket,(struct sockaddr*)&sgCli_addr, &ilAddrlen);
    if ( igSockfd == -1)
    {
      swVdebug(0,"S0070: [����/ϵͳ����] ���Ӵ��� errno=%d[%s]",errno,strerror(errno));
      close( igSockfd ); 
      exit( -1 ); 
    }
    swVdebug(2,"S0080: accept() SUCCESS");
     
    switch ( fork() )
    {
      case -1:
        swVdebug(2,"S0090: fork�ӽ��̳���!");
        close( igSocket );
        close( igSockfd );  
  	exit( -1 );
      case 0:	
        swVdebug(2,"S0100: �ӽ��̴���");
        close( igSocket );
        swDoitass();
 	exit(0);
      default:
        swVdebug(2,"S0110: �����̴���");
        close( igSockfd );
        break;
    }/* end switch */
  } /* end for */
}  

/****************************************************************/
/* �������    ��swDoitass                                      */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   -1                                      */
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

  swVdebug(4,"S0120: [��������] swDoitass()");
  /*��ÿͻ���������ַ*/ 
  memset(alCliaddr,0x0,sizeof(alCliaddr));
  strcpy(alCliaddr,inet_ntoa(sgCli_addr.sin_addr)); 
  swVdebug(2,"S0130: connect from %s port %d ",alCliaddr,
    ntohs(sgCli_addr.sin_port));
  
  /*��ÿͻ���������*/
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
  
  /*��ͨѶ����*/
  ilRc = swMbopen( sgComcfg.iMb_comm_id );
  if(ilRc)
  {
    swVdebug(0,"S0170: [����/����] ������[%d]����,������=%d", sgComcfg.iMb_comm_id,ilRc );
    close( igSockfd );
    swMbclose();
    return(-1);
  }
  swVdebug(2,"S0180: ������ɹ�");
  
  ilPri = 0 ;
  time(&llClass);
  llType = getpid();

  for (;;)      
  {
    /*�ӿͻ��˽��ձ���*/
    ilMsglen = sizeof(alMsgbuf);

    ilRc = swTcprcv(igSockfd, alMsgbuf,&ilMsglen); 
    if (ilRc) 
    {
      swVdebug(1,"S0190: [����/��������] swTcprcv()����,�ӿͻ���{%s}���ձ��ĳ���,������=%d",alCliname,ilRc);
      close( igSockfd );
      swMbclose();
      return(-1);
    }
    swVdebug(2,"S0200: �ӿͻ���{%s}���ձ������!len=%d",alCliname,ilMsglen);
    if (cgDebug >= 2)
      swDebughex(alMsgbuf,ilMsglen);
 
    /* ����ͨѶ��������־ */
    ilContflag = swComcheckcontpack(alMsgbuf,ilMsglen);
 
    /*�������ޱ���ͷ*/ 
    if( sgComcfg.iMsghead == 0)
    {
    /* �ڱ���ǰ���ӿձ���ͷ */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }

    ilRc = swSendpackl(sgComcfg.iMb_fore_id, alMsgbuf, ilMsglen, ilPri, 
      llClass, llType);
    if (ilRc)
    {
      swVdebug(1,"S0210: [����/��������] swSendpackl()����,д���ĵ�����[%d]����,������=%d",sgComcfg.iMb_fore_id,ilRc);    
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }

    swVdebug(2,"S0220: �ѳɹ����͵�����[%d],ilPri=[%d],Class=[%ld],Type=[%ld]",\
      sgComcfg.iMb_fore_id, ilPri, llClass, llType );

    if (ilContflag != 1) break;
  } /* end for */
  
  swMbclose();
  close ( igSockfd );
  swVdebug(4,"S0230: [��������] swDoitass()������=0");
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

