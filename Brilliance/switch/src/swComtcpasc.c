/****************************************************************/
/* ģ����    ��swComtcpasc                                    */
/* ģ������    : ͨ������-TCP�첽������Client                   */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/18                                      */
/* ����޸����ڣ�2001/9/18                                      */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
/*			  (1)int swComtcpasc()                  */
/*                        (2)int swDoitasc()                    */
/*                        (3)int swQuit()                       */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static  int  igSockfd;

int swDoitasc(short iFlag, char *aMsgbuf ,unsigned int iMsglen );
void swQuit();

int main( int argc, char **argv )
{
  int    ilRc;
  unsigned int ilMsglen;
  short  ilQid,ilType,ilClass,ilPri;
  short  ilStatus,ilMode=0,ilContflag,ilMode1;
  char   alMsgbuf[iMSGMAXLEN];

  /* ȡ�ð汾�� */
  if(argc > 1)
    _swVersion("swComtcpasc Version 4.3.0",argv[1]);

  /* ���õ��Գ������� */   
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(0,"S0010: [����/����] ��������������Ϸ�!");
    exit(-1);
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
    swVdebug(0,"S0040: [����/����] ������[%d]ʧ��!",sgComcfg.iMb_comm_id);
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
  ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
  ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
  if ( ilRc )
  {
    swVdebug(0,"S0070: [����/��������] swPortset()����,�ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
    swMbclose();
    exit(-1);
  }
  swVdebug(2,"S0080: ���ö˿ڳɹ�!");
   
  for(;;) 
  { 
    swVdebug(2,"S0090: ѭ������!");
 
    /* ȡ�˿�ͨѶ״̬ */
    /* next modify by nh 2002.6.6
    ilRc =  swPortget( sgComcfg.iMb_comm_id, 1, &ilStatus );*/
    ilRc =  swPortget( sgComcfg.iMb_fore_id, 1, &ilStatus );
    if ( ilRc )
    {
      swVdebug(1,"S0100: [����/��������] swPortget()����,ȡ�˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
      exit(-1);
    }

    /* �ж϶˿�״̬ */
    if ( ilStatus == 0 )
    {
      swVdebug(1,"S0110: [����/����] �˿�״̬(ͨѶ��)Ϊdown,���ⷢ����!");
      continue;
    }
 
    /* Read from mq */
    ilPri = 0;
    ilClass = 0;
    ilType  = 0;
    ilMsglen = sizeof( alMsgbuf );
  
    swVdebug(2,"S0120: ��ʼ�������ȡ��Ϣ!");
      
    ilRc = swRecvpackw(&ilQid, alMsgbuf,&ilMsglen,&ilPri,&ilClass,&ilType);
    if ( ilRc )
    {  
      swVdebug(1,"S0130: [����/��������] swRecvpackw()����,������[%d]����,������=%d",sgComcfg.iMb_comm_id,ilRc);
      swMbclose();
      exit( -1 );
    }

    swVdebug(2,"S0140: ��ͨѶ��������ĳɹ�!");
    if (cgDebug >= 2)  
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* �ж����޺�����*/
    ilContflag = swSwcheckcontpack(alMsgbuf);
    if ( ilContflag == 1 )  /*�к���������*/
    {
      ilRc = swConnsend(ilMode ,&igSockfd, alMsgbuf, ilMsglen);
      /* next 9 rows add by nh 2001/12/12 */
      if(!ilRc)
      {
        /* next modify by nh 2002.6.6
        ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
        ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
        if ( ilRc )
        {
          swVdebug(1,"S0150: [����/��������] swPortset()����, �ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
          swMbclose();
          exit(-1);
        }
        swVdebug(2,"S0160: ���ö˿ڳɹ�");
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
 
    switch(fork()) 
    {
      case 0:
        swVdebug(2,"S0170: �ӽ��̴���!");
        setpgrp();
        ilRc = swDoitasc(ilMode1, alMsgbuf, ilMsglen); 
        /* next 9 rows add by nh 2001/12/12 */
        if(!ilRc)
        {
          /* next modify by nh 2002.6.6
          ilRc =  swPortset( sgComcfg.iMb_comm_id, 1, 1 );*/
          ilRc =  swPortset( sgComcfg.iMb_fore_id, 1, 1 );
          if ( ilRc )
          {
            swVdebug(1,"S0180: [����/��������] swPortset()����, �ö˿�[%d]״̬����,������=%d",sgComcfg.iMb_fore_id,ilRc);
            swMbclose();
            exit(-1);
          }
          swVdebug(2,"S0190: ���ö˿ڳɹ�");
        }
        exit(0);
      case -1:
        swVdebug(2,"S0200: Fork ���̳���!" );
        swMbclose();
        exit(-1);
      default:
        swVdebug(2,"S0210: �����̴���");
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

int swDoitasc(short iFlag, char *aMsgbuf ,unsigned int iMsglen )
{
  int    ilRc;
 
  ilRc = swConnsend( iFlag, &igSockfd, aMsgbuf, iMsglen );
  if ( ilRc )
  {
    close( igSockfd );
    return(-1);
  }
  close( igSockfd );
  return 0;
}

void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSockfd);
  exit(0);
}


