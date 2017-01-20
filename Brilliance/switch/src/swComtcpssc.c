/****************************************************************/
/* ģ����    ��swComtcpssc                                    */
/* ģ������    : ͨ������-TCPͬ���̶�����Client                 */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/15                                      */
/* ����޸����ڣ�2001/9/18                                      */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
/*			  (1)int32 swComtcpssc()                */
/*                        (2)int32 swDoitssc()                  */
/*                        (4)int32 swQuit()                     */
/****************************************************************/
#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static int    igSockfd;

void swQuit(int);
int swDoitssc(short iFlag,char *aMsgbuf,unsigned int iMsglen);
extern int swSendpackl(short, char *, unsigned int, short, long, long);
extern int swRecvpackl(short *,char *,unsigned int *,short *,long *,long *,short);

int main( int argc, char **argv )
{
  int    ilRc;
  unsigned int ilMsglen;
  short  ilQid,ilType,ilClass,ilPri;
  short  ilStatus,ilMode=0,ilContflag,ilMode1;
  char   alMsgbuf[iMSGMAXLEN];
  
  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpssc Version 4.3.0",argv[1]);
 
  /* ���õ��Գ������� */  
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    printf("Usage: swComtcpssc portname \n");
    exit(-1);
  }
  
  /* �����ź� */
  swSiginit( );
 
  /* ���벢������� */
  ilRc = swLoadcomcfg( argv[1] );
  if ( ilRc )
  {
    swVdebug(0,"S0010: [����/��������] swLoadcomcfg()����,[%s]װ��ͨѶ���ò�������,��������Ķ˿���,������=%d",argv[1],ilRc);
    exit(-1);
  }
  swVdebug(2,"S0020: [%s]װ��ͨѶ���ò����ɹ�",argv[1]);
   
  ilRc = swMbopen(sgComcfg.iMb_comm_id);
  if ( ilRc )
  {  
    swVdebug(0,"S0030: [����/����] ������[%d]ʧ��!ilRc=%d",sgComcfg.iMb_comm_id,ilRc);
    exit(-1);
  }
  swVdebug(2,"S0040: ������[%d]�ɹ�!",sgComcfg.iMb_comm_id);
 
  /* �ö˿�ͨѶ��ΪUP */
  ilRc = swShmcheck();
  if (ilRc !=SUCCESS )
  {
    swVdebug(0,"S0050: [����/�����ڴ�] �����ڴ������!");
    swMbclose();
    return(FAIL);
  }

  /* next modify by nh 2002.6.6
  ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
  ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
  if ( ilRc )
  {
    swVdebug(0,"S0060: [����/��������] swPortset()����,�ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
    swMbclose();
    exit(-1);
  }
  swVdebug(2,"S0070: ���ö˿ڳɹ�");

  for(;;) 
  {
    swVdebug(2,"S0080: ѭ������");
    /* ȡ�˿�ͨѶ״̬ */
    /* next modify by nh 2002.6.6
    ilRc =  swPortget( sgComcfg.iMb_comm_id,1,&ilStatus );*/
    ilRc =  swPortget( sgComcfg.iMb_fore_id,1,&ilStatus );
    if ( ilRc )
    {
      swVdebug(0,"S0090: [����/��������] swPortget()����,ȡ�˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
      exit(-1);
    }

    /* �ж϶˿�״̬ */
    if ( ilStatus == 0 )
    {
      swVdebug(0,"S0100: [����/����] �˿�״̬(ͨѶ��)Ϊdown,���ⷢ����!");
      continue;
    }

    /* ��ͨѶ����  */
    ilPri = 0;
    ilClass = 0;
    ilType  = 0;
    ilMsglen = sizeof( alMsgbuf );

    swVdebug(2,"S0110: �ȴ���ͨѶ����[%d]���ձ���!",sgComcfg.iMb_comm_id);
    swVdebug(2,"S0120: ilPri=[%d],Class=[%d],Type=[%d]",ilPri,ilClass,ilType);
 
    ilRc = swRecvpackw(&ilQid,alMsgbuf,&ilMsglen,&ilPri,&ilClass,&ilType);
    if ( ilRc )
    {  
      swVdebug(1,"S0130: [����/��������] swRecvpackw()����,������[%d]����,������=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      exit( -1 );
    }
    
    swVdebug(2,"S0140: ��ͨѶ����[%d]���ձ��ĳɹ�!",sgComcfg.iMb_comm_id);
    swVdebug(2,"S0150: ilPri=[%d],Class=[%d],Type=[%d]",ilPri,ilClass,ilType);
 
    if (cgDebug >= 2)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* �ж����޺�����*/
    ilContflag = swSwcheckcontpack(alMsgbuf);
    if ( ilContflag == 1 )  /*�к���������*/
    {
      ilRc = swConnsend( ilMode ,&igSockfd, alMsgbuf, ilMsglen );
      /* next 9 rows add by nh */
      if(!ilRc)
      {
        /* next modify by nh 2002.6.6
        ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
        ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
        if ( ilRc )
        {
          swVdebug(1,"S0160: [����/��������] swPortset()����, �ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
          swMbclose();
          exit(-1);
        }
        swVdebug(2,"S0170: ���ö˿ڳɹ�");
      }
      ilMode = 1;
      continue;
    }
    if ( ilMode == 1 )   /*�������е����һ����*/
    {
      ilMode1 = 1 ;
      ilMode = 0 ;
    }
    else                 /*�޺�����*/
    {
      ilMode1 = 0;
      ilMode = 0 ;
    }
    /* fork�ӽ��� */
    switch(fork()) 
    { 
      case 0:
        swVdebug(2,"S0180: �ӽ��̴���");
        /*next modify by nh*/
        ilRc = swDoitssc(ilMode1,alMsgbuf,ilMsglen); 
        if(!ilRc)
        {
          /* next modify by nh 2002.6.6
          ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
          ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
          if ( ilRc )
          {
            swVdebug(1,"S0190: [����/��������] swPortset()����, �ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
            swMbclose();
            exit(-1);
          }
          swVdebug(2,"S0200: ���ö˿ڳɹ�");
        }
        exit(0);
      case -1:
        swVdebug(1,"S0210: Fork ���̳���!" );
        swMbclose();
        exit(-1);
      default:
        swVdebug(2,"S0220: �����̴���");
        break;                  
    }
  }
}
/****************************************************************/
/* �������    ��swDoitass                                      */
/* ��������    ��                                               */
/* ��    ��    ��                                               */
/* ��������    ��2001/9/15                                      */
/* ����޸����ڣ�                                               */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   -1                                      */
/****************************************************************/

int swDoitssc(short iFlag,char *aMsgbuf,unsigned int iMsglen)
{
  int    ilRc;
  short  ilContflag;
  struct msghead *pslMsghead;

  swVdebug(4,"S0230: [��������] swDoitssc(%d,%d)",iFlag,iMsglen);
  ilRc = swConnsend( iFlag, &igSockfd, aMsgbuf, iMsglen );
  if ( ilRc )
  {
    close( igSockfd );
    return(-1);
  }

  pslMsghead = (struct msghead *)aMsgbuf;
  /* ��ͨѶ���� */
  ilRc = swMbopen( sgComcfg.iMb_comm_id );
  if ( ilRc )
  {
    swVdebug(1,"S0240: [����/����] ������[%d]ʧ��!",sgComcfg.iMb_comm_id);
    close( igSockfd );
    return(-1);
  }
  for( ; ; )
  {
    /* �ӷ���˿ڽ��ձ��� */
    swVdebug(2,"S0250: �ȴ����շ���˵�����" );
 
    if (sgComcfg.iMsghead == 0)
    {
      /* ͨѶ���ز�������ͷ */
      ilRc = swTcprcv(igSockfd,aMsgbuf+sizeof(struct msghead),&iMsglen); 
      if ( ilRc ) 
      { 
        swVdebug(1,"S0260: [����/��������] swTcprcv()����,�ӷ���˽��ճ���-errno=[%d,%s]",errno,
          strerror(errno));
        swMbclose();
        close( igSockfd );
        return(-1);
      }
      /* ����ͨѶ��������־ */
      ilContflag = swComcheckcontpack(aMsgbuf+sizeof(struct msghead),iMsglen);
      pslMsghead->iBodylen = iMsglen;
      iMsglen = iMsglen + sizeof(struct msghead);
    }
    else
    {
      /* ͨѶ���ش�����ͷ */
      ilRc = swTcprcv(igSockfd,aMsgbuf,&iMsglen); 
      if ( ilRc ) 
      { 
        swVdebug(1,"S0270: [����/��������] swTcprcv()����,�ӷ���˽��ճ���,errno=%d[%s]",errno,
          strerror(errno));
        swMbclose();
        close( igSockfd );
        return(-1);
      }

      /* ����ͨѶ��������־ */
      ilContflag = swComcheckcontpack(aMsgbuf,iMsglen);
      pslMsghead->iBodylen = iMsglen - sizeof(struct msghead);
    }

    swVdebug(2,"S0280: swTcprcv SUCCESS!");
    swVdebug(2,"S0290: �Ӷ˿ڽ��ձ��ĳɹ�!iMsglen = %d",iMsglen);
 
    if( cgDebug >= 2 )
      swDebughex( aMsgbuf,iMsglen);

    /* д��ǰ������ */
    ilRc = swSendpack(sgComcfg.iMb_fore_id,aMsgbuf,iMsglen,0,0,0);
    if (ilRc)
    {
      swVdebug(1,"S0300: [����/��������] swSendpack()����,дǰ������[%d]����,������=%d",sgComcfg.iMb_fore_id,ilRc);
      swMbclose();
      close( igSockfd );
      return(-1);
    }
    swVdebug(2,"S0310: дǰ������[%d]�ɹ�!",sgComcfg.iMb_fore_id);
    if (ilContflag != 1) break;
  } /* end while */

  swMbclose();
  close(igSockfd);
  swVdebug(4,"S0320: [��������] swDoitssc()������=0");
  return(0);
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSockfd);
  exit(0);
}

