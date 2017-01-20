/****************************************************************/
/* ģ����    ��BMQGRP_RCVW                                    */
/* ģ������    �����鼶�������ӱ��Ĵ洢ģ��                     */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2007/01/28                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���³�Q ���鼶�������ӱ��Ĵ洢                   */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*                                                              */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"

struct Msgbuf{
  long   mtype;
  struct grprecvmsg rcvmsg;
  long   msglen;
  char   data[7168];
}sgMsgbuf;

int main(int argc,char **argv)
{
  int    ilRc;
  
  /*��ʾ�³�Q�汾��Ϣ*/
  _bmqShowversion(argc,argv);

  /*�����ź�*/
  signal(SIGQUIT,SIG_IGN);
  
  /* �����ػ����� */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010 %s:�����ػ�����ʧ��!",__FILE__);
    exit(FAIL);
  }
 
  /*������������ļ�DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  {
    _bmqDebug("S0020 %s :����DEBUG����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }
   
  /*���ӹ����ڴ�*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030 %s:���ӹ����ڴ�������:%d",__FILE__,ilRc);
    exit(FAIL);
  }

  psgMbshm->lBmqgrprcvw_pid = getpid();

  _bmqVdebug(1,"S0040 Begin start grp_rcvw...");
  /* ���������Ϣ���� */
  while( msgrcv(psgMbshm->lMsgid_grp_rcv,(char *)&sgMsgbuf,sizeof(sgMsgbuf),0,IPC_NOWAIT) >= 0  );

  _bmqVdebug(1,"S0050 clear msg[%ld] ok!",psgMbshm->lMsgid_grp_rcv);

  for( ; ; )
  {
    memset(&sgMsgbuf,0x00,sizeof(sgMsgbuf));

    sgMsgbuf.mtype = 99; 
    /* ��������Ϣ���� */
    ilRc = msgrcv (psgMbshm->lMsgid_grp_rcv, (char *)&sgMsgbuf,sizeof(sgMsgbuf),0,0);
    if (ilRc < 0 ) {
       _bmqDebug("S0040 ����Ϣ���г���[%d] ,errno:%d,qid[%ld]", ilRc,errno,psgMbshm->lMsgid_grp_rcv );
       if(psgMbshm->lMsgid_grp_rcv == 0) exit(-1);
       continue;
    }
    if( igDebug >= 3) _bmqDebughex(sgMsgbuf.data,sgMsgbuf.msglen);
    
    /*���Ĵ���*/
    igMb_sys_current_group   = sgMsgbuf.rcvmsg.iOrg_group;
    igMb_sys_current_mailbox = sgMsgbuf.rcvmsg.iOrg_mailbox;
    igMbopenflag             = 1;  
    lgCurrent_pid            = getpid();
    
    ilRc = bmqPutfilter(0,sgMsgbuf.rcvmsg.iDes_mailbox,sgMsgbuf.rcvmsg.iPrior,sgMsgbuf.rcvmsg.aFilter, \
      sgMsgbuf.data, sgMsgbuf.msglen );
    if(ilRc)
    {
      _bmqDebug("S0050 ���Ĵ���[%d]����!",sgMsgbuf.rcvmsg.iDes_mailbox);
    }
  }
}
