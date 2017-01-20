#include "swapi.h"
#include "swCompub.h"
#include "swPubfun.h"

/*����ԭ��*/
int swSiginit();
void swQuit(int);
int swReadcomcfg(char paFldvalue[][iFLDVALUELEN]);
int swComaddblankmsghead(char *,unsigned int *); 
int swComcheckcontpack(char *,unsigned int );
int swSwcheckcontpack(char *);
extern int _swExpN2T(char *aNstr,char *aTstr);

/****************************************************************/
/* �������    ��swSiginit                                      */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2000/5/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/
int swSiginit()
{
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGINT,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGSTOP,SIG_IGN);
  signal(SIGUSR1,SIG_IGN);
  signal(SIGUSR2,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
  signal(SIGCLD,SIG_IGN);
  signal(SIGTERM,swQuit);
  return(0);
}

/****************************************************************/
/* �������    ��swReadcomcfg                                   */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/
int swReadcomcfg(char paFldvalue[][iFLDVALUELEN])
{
  int  ilRc;
  char alTmpexp[2*iFLDVALUELEN+1+1];

  swVdebug(4,"S0010: [��������] swReadcomcfg()");
  strcpy( sgComcfg.aPort_name, paFldvalue[0] );
  strcpy( sgComcfg.aMode, paFldvalue[1] );
  sgComcfg.iMb_comm_id = atoi( paFldvalue[2] );
  sgComcfg.iMb_fore_id = atoi( paFldvalue[3] );
  sgComcfg.iTrytimes = atoi( paFldvalue[4] );
  sgComcfg.iTryfreq = atoi( paFldvalue[5] );
  sgComcfg.iTime_out = atoi( paFldvalue[6] );
  sgComcfg.iDebug = atoi( paFldvalue[7] );
  sgComcfg.iMsghead = atoi( paFldvalue[8] );
  strcpy( sgComcfg.aEnd_string, paFldvalue[9] );
  sgComcfg.iPre_msglen = atoi( paFldvalue[10] );
  
  /* strcpy( sgComcfg.aExp_len, paFldvalue[11] );  */
  /* strcpy( sgComcfg.aExp_cont, paFldvalue[12] ); */
  /* delete   by wangpan 2001/11/29                */
  /* modified by wangpan 2001/11/29                */
  memset(alTmpexp,0x00,sizeof(alTmpexp));
  ilRc = _swExpN2T(paFldvalue[11],alTmpexp);
  if(ilRc!=0) return(ilRc);  
  strcpy( sgComcfg.aExp_len,alTmpexp);

  memset(alTmpexp,0x00,sizeof(alTmpexp));
  ilRc = _swExpN2T(paFldvalue[12],alTmpexp);
  if(ilRc!=0) return(ilRc);  
  strcpy( sgComcfg.aExp_cont,alTmpexp);
  /* end modify  by wangpan 2001/11/29             */
  
  sgComcfg.iPrep_cont = atoi( paFldvalue[13] );
      
  cgDebug = sgComcfg.iDebug;

  swVdebug(3,"S0020: �˿�ͨѶ��������:");
  swVdebug(3,"S0030: sgComcfg.aPort_name  = [%s]",sgComcfg.aPort_name);
  swVdebug(3,"S0040: sgComcfg.aMode       = [%s]",sgComcfg.aMode);
  swVdebug(3,"S0050: sgComcfg.iMb_comm_id = [%d]",sgComcfg.iMb_comm_id);
  swVdebug(3,"S0060: sgComcfg.iMb_fore_id = [%d]",sgComcfg.iMb_fore_id);
  swVdebug(3,"S0070: sgComcfg.iTrytimes   = [%d]",sgComcfg.iTrytimes);
  swVdebug(3,"S0080: sgComcfg.iTryfreq    = [%d]",sgComcfg.iTryfreq);
  swVdebug(3,"S0090: sgComcfg.iTime_out   = [%d]",sgComcfg.iTime_out);
  swVdebug(3,"S0100: sgComcfg.iMsghead    = [%d]",sgComcfg.iMsghead);
  swVdebug(3,"S0110: sgComcfg.aEnd_string = [%s]",sgComcfg.aEnd_string);
  swVdebug(3,"S0120: sgComcfg.iPre_msglen = [%d]",sgComcfg.iPre_msglen);
  swVdebug(3,"S0130: sgComcfg.aExp_len    = [%s]",sgComcfg.aExp_len);
  swVdebug(3,"S0140: sgComcfg.aExp_cont   = [%s]",sgComcfg.aExp_cont);
  swVdebug(3,"S0150: sgComcfg.iPrep_cont  = [%d]",sgComcfg.iPrep_cont);
  swVdebug(3,"S0160: [��������] swReadcomcfg()������=0");
  return(0);
}
  
/****************************************************************/
/* �������    ��swComaddblankmsghead                           */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2001/9/                                        */
/* ������;    ���ӿձ���ͷ                                     */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/
int swComaddblankmsghead(char *aBuf,unsigned int *piMsglen) 
{
  struct msgpack slMsgpack;
  swVdebug(4,"S0170: [��������] swComaddblankmsghead()");
  memcpy(slMsgpack.aMsgbody,aBuf,*piMsglen);

  slMsgpack.sMsghead.iBodylen = *piMsglen;
  slMsgpack.sMsghead.iMsgtype = 1;
  *piMsglen = *piMsglen + sizeof(struct msghead);
  memcpy(aBuf,(char *)&slMsgpack,*piMsglen);
  swVdebug(4,"S0180: [��������] swComaddblankmsghead()������=0");
  return(0);
}

/****************************************************************/
/* �������    ��swComcheckcontpack                             */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2001/9/15                                      */
/* ����޸����ڣ�                                               */
/* ������;    �����ͨѶ���޺�����                             */
/* ��������ֵ  : 0---�޺�����                                   */
/*               1---�к�����                                   */
/*               -1---����                                      */
/****************************************************************/
int swComcheckcontpack(char *aBuf,unsigned int iMsglen)
{
  unsigned int ilRc,ilResultlen;
  char alResult[iFLDVALUELEN];

  swVdebug(4,"S0190: [��������] swComcheckcontpack(%d)",iMsglen);
  if (!sgComcfg.aExp_cont[0]) return(0);
  /* �����˺�����ƽ̨���ʽ */
  agMsgbody = aBuf;
  igMsgbodylen = iMsglen;
  ilRc = _swExpress(sgComcfg.aExp_cont,alResult,&ilResultlen);

  if (ilRc)
  {
    swVdebug(2,"S0200: ����ƽ̨���ʽ[%s]����",sgComcfg.aExp_cont);
    return(-1);
  }
  if (alResult[0] == '0') 
  {
    swVdebug(4,"S0210: [��������] swComcheckcontpack()������=0");
    return(0);
  }
  return(1);
}

/****************************************************************/
/* �������    ��swSwcheckcontpack                              */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2001/9/15                                      */
/* ����޸����ڣ�                                               */
/* ������;    �����ǰ�÷��͹��������޺�����aMemo[19]          */
/* ��������ֵ  : 0---�޺�����                                   */
/*               1---�к�����                                   */
/*               -1---����                                      */
/****************************************************************/
int swSwcheckcontpack(char *aBuf)
{
  struct msghead *pslMsghead;
  
  swVdebug(4,"S0220: [��������] swSwcheckcontpack()");
  if( sgComcfg.iPrep_cont != 1) 
  {
    swVdebug(4,"S0230: [��������] swSwcheckcontpack()������=0");
    return(0);
  }

  pslMsghead = (struct msghead *)aBuf;
  if (pslMsghead->aMemo[19] == '1') 
  {
    swVdebug(4,"S0240: [��������] swSwcheckcontpack()������=1");
    return(1);
  }
  swVdebug(4,"S0250: [��������] swSwcheckcontpack()������=0");
  return(0);
  
}

