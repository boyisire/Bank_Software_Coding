#include "swapi.h"
#include "swCompub.h"
#include "swPubfun.h"

/*函数原型*/
int swSiginit();
void swQuit(int);
int swReadcomcfg(char paFldvalue[][iFLDVALUELEN]);
int swComaddblankmsghead(char *,unsigned int *); 
int swComcheckcontpack(char *,unsigned int );
int swSwcheckcontpack(char *);
extern int _swExpN2T(char *aNstr,char *aTstr);

/****************************************************************/
/* 函数编号    ：swSiginit                                      */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2000/5/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
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
/* 函数编号    ：swReadcomcfg                                   */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/
int swReadcomcfg(char paFldvalue[][iFLDVALUELEN])
{
  int  ilRc;
  char alTmpexp[2*iFLDVALUELEN+1+1];

  swVdebug(4,"S0010: [函数调用] swReadcomcfg()");
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

  swVdebug(3,"S0020: 端口通讯公共参数:");
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
  swVdebug(3,"S0160: [函数返回] swReadcomcfg()返回码=0");
  return(0);
}
  
/****************************************************************/
/* 函数编号    ：swComaddblankmsghead                           */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2001/9/                                        */
/* 函数用途    ：加空报文头                                     */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/
int swComaddblankmsghead(char *aBuf,unsigned int *piMsglen) 
{
  struct msgpack slMsgpack;
  swVdebug(4,"S0170: [函数调用] swComaddblankmsghead()");
  memcpy(slMsgpack.aMsgbody,aBuf,*piMsglen);

  slMsgpack.sMsghead.iBodylen = *piMsglen;
  slMsgpack.sMsghead.iMsgtype = 1;
  *piMsglen = *piMsglen + sizeof(struct msghead);
  memcpy(aBuf,(char *)&slMsgpack,*piMsglen);
  swVdebug(4,"S0180: [函数返回] swComaddblankmsghead()返回码=0");
  return(0);
}

/****************************************************************/
/* 函数编号    ：swComcheckcontpack                             */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2001/9/15                                      */
/* 最后修改日期：                                               */
/* 函数用途    ：检测通讯有无后续包                             */
/* 函数返回值  : 0---无后续包                                   */
/*               1---有后续包                                   */
/*               -1---出错                                      */
/****************************************************************/
int swComcheckcontpack(char *aBuf,unsigned int iMsglen)
{
  unsigned int ilRc,ilResultlen;
  char alResult[iFLDVALUELEN];

  swVdebug(4,"S0190: [函数调用] swComcheckcontpack(%d)",iMsglen);
  if (!sgComcfg.aExp_cont[0]) return(0);
  /* 设置了后续包平台表达式 */
  agMsgbody = aBuf;
  igMsgbodylen = iMsglen;
  ilRc = _swExpress(sgComcfg.aExp_cont,alResult,&ilResultlen);

  if (ilRc)
  {
    swVdebug(2,"S0200: 计算平台表达式[%s]出错",sgComcfg.aExp_cont);
    return(-1);
  }
  if (alResult[0] == '0') 
  {
    swVdebug(4,"S0210: [函数返回] swComcheckcontpack()返回码=0");
    return(0);
  }
  return(1);
}

/****************************************************************/
/* 函数编号    ：swSwcheckcontpack                              */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2001/9/15                                      */
/* 最后修改日期：                                               */
/* 函数用途    ：检测前置发送过来的有无后续包aMemo[19]          */
/* 函数返回值  : 0---无后续包                                   */
/*               1---有后续包                                   */
/*               -1---出错                                      */
/****************************************************************/
int swSwcheckcontpack(char *aBuf)
{
  struct msghead *pslMsghead;
  
  swVdebug(4,"S0220: [函数调用] swSwcheckcontpack()");
  if( sgComcfg.iPrep_cont != 1) 
  {
    swVdebug(4,"S0230: [函数返回] swSwcheckcontpack()返回码=0");
    return(0);
  }

  pslMsghead = (struct msghead *)aBuf;
  if (pslMsghead->aMemo[19] == '1') 
  {
    swVdebug(4,"S0240: [函数返回] swSwcheckcontpack()返回码=1");
    return(1);
  }
  swVdebug(4,"S0250: [函数返回] swSwcheckcontpack()返回码=0");
  return(0);
  
}

