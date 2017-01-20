/****************************************************************/
/* 模块编号    ：BMQFLIESAF                                     */
/* 模块名称    ：文件传输SAF模块                                */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军　                                         */
/* 建立日期    ：2006/12/26                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：正常发送过程中出线异常的文件完成继续发送的服务 */
/*               超过指定时间未能完成发送时，抛出发送失败信息   */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/****************************************************************/
/*库函数定义*/
#include "bmq.h"

/*static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,short iMsglen);delete by wh*/
static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,TYPE_SWITCH iMsglen);/*add by wh*/
extern int _bmqFile_comm(char aParm[][iFLDVALUELEN]);
extern int _bmqFile_msg(char aParm[][iFLDVALUELEN]);
extern int _bmqGetfield(char *fname,char *key,int key_n,char *field);

/* 主程序 */
int main(int argc,char **argv)
{
  int		ilRc;
  short		ilGrpid;
  short		ilMbid;
  short		ilPorior;
  short		ilTimeout;
  /*short		ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen;/*add by wh*/
  long		llType;
  long		llClass;
  char		alMsgbuf[iMBMAXPACKSIZE];
  struct	monmsg slFilemng;
  
  /*显示版本信息*/
  _bmqShowversion(argc,argv);

  /*设置信号*/
  _bmqSignalinit();
  
  /* 创建守护进程 */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010: 创建守护进程失败，退出!");
    exit(FAIL);
  }
  
  /*读入参数配置文件DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  { 
    _bmqDebug("S0020: 载入DEBUG参数失败，请查看参数文件%s/etc/bmq.ini",getenv("BMQ_PATH"));
    exit(FAIL);
  }
  
  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030: 连接共享内存区出错:%d",ilRc);
    exit(FAIL);
  }
  
  /*保存进程pid*/
  psgMbshm->lBmqfilesaf_pid = getpid();
  
  /*打开文件SAF邮箱*/
  ilRc = bmqOpen(iMBFILESAF);
  if(ilRc)
  { 
    _bmqDebug("S0040: 打开文件管理邮箱出错:%d",ilRc);
    exit(FAIL);
  }
  
  while(1)
  {
    llType = llClass = ilTimeout = 0;
    ilRc = bmqGetw(&ilGrpid,&ilMbid,&ilPorior,&llType,&llClass,alMsgbuf,&ilMsglen,ilTimeout);
    if(ilRc)
    {
      _bmqDebug("S0050: 阻塞读取文件传输SAF报文出错:%d",ilRc);
      continue;
    }
    
    if(ilMsglen < sizeof(struct monmsg))
    {
      _bmqDebug("S0060: 收到的不是文件传输SAF报文,丢弃之.[%d]<>[%d]",ilMsglen,sizeof(struct monmsg));
      continue;
    }
    
    switch ( fork() )
    {
      case -1:
        exit( -1 );
      case 0:
        memset(&slFilemng,0x00,sizeof(struct monmsg));
        memcpy((char *)&slFilemng,alMsgbuf,ilMsglen);
        if ( _bmqFilesafproc(ilPorior,llType,llClass,alMsgbuf,ilMsglen) != 0 )
        {
          _bmqDebug("S0070 %s:子进程处理出错!",__FILE__);  
        }
        exit(0);
      default:
        break;
    }    
  } 
}

/**************************************************************
 ** 函数名: _bmqFilesafproc
 ** 功能:   文件断点续传SAF模块
 ** 作者:	徐军
 ** 建立日期: 2006/12/28
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
/*static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,short iMsglen)delete by wh*/
static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int		ilRc;
  int		ilTryTimes;				/*尝试重试次数,缺省为无限制*/
  int		ilInterVal;				/*尝试间隔时间,缺省为10秒  */
  long		llSerialno = 0;
  char		alResult[10];
  char		alInitfile[101];
  char		alFile[100];
  char		alTmpFileName[100];
  struct 	transparm slTransparm;
  struct 	stat f_stat; 
  
  /*打开文件SAF邮箱*/
  ilRc = bmqOpen(iMBFILESAF);
  if(ilRc)
  { 
    _bmqDebug("S0040: 打开文件管理邮箱出错:%d",ilRc);
    return(FAIL);
  }
  
  memset(&sgMonMsg,0x00,sizeof(struct monmsg));
  memcpy(&sgMonMsg,aMsgbuf,sizeof(struct monmsg));
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  _bmqfilechr(sgMonMsg.aFileName,strlen(sgMonMsg.aFileName),alFile,'/');
  sprintf(alTmpFileName,"%s/saf/%s",getenv("BMQ_FILE_DIR"),alFile);
  
  if( stat( alTmpFileName, &f_stat ) == -1 )
  {
    _bmqDebug("S0100: 无法打开指定文件[%s]!",alFile);
    sgMonMsg.iSendFlag  = 5;
    strcpy(sgMonMsg.aStatusDesc,"无法打开指定文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    bmqClose();
    return(FAIL);
  }
  
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"FILETRYTIMES",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0110: 从文件%s中取 FILETRYTIMES 字段失败!",alInitfile);
    ilTryTimes = -1;
  }
  else
    ilTryTimes = atoi(alResult);
  ilRc = _bmqGetfield(alInitfile,"FILEINTERVAL",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0110: 从文件%s中取 FILEINTERVAL 字段失败!",alInitfile);
    ilInterVal = 10;
  }
  else
    ilInterVal = atoi(alResult);
  
  sgMonMsg.lSendPid         = getpid();
  slTransparm.bmqFile_comm	= _bmqFile_comm;
  slTransparm.bmqFile_msg	= _bmqFile_msg;
  
FILESAF:
  llSerialno = sgMonMsg.lSerialno;
  ilRc = _bmqSendfile_G(sgMonMsg.iDesGrpid,sgMonMsg.aFileName,slTransparm,&llSerialno);
  if( ilRc == -1 )
  {
    /* 重新尝试发送 */
    if( (ilTryTimes == -1) || (ilTryTimes > 0) )
    {
      sleep(ilInterVal);
      if(ilTryTimes>0)
        ilTryTimes--;
      goto FILESAF;
    }
    bmqClose();
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"发送文件失败,需重新发送");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    unlink(sgMonMsg.aFileName);
    return(FAIL);
  }
  else if(ilRc == -2)
  {
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"不能打开临时文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  } 	
  
  if(lClass == 1)
  {
    ilRc = bmqPut(sgMonMsg.iDesGrpid,sgMonMsg.iDesMbid,iPorior,lType,llSerialno,
                  aMsgbuf+sizeof(struct monmsg),iMsglen-sizeof(struct monmsg));
    if(ilRc)
    {
      _bmqDebug("S0120: [函数调用失败]bmqPut error!");
      bmqClose();
      unlink(sgMonMsg.aFileName);
      return(FAIL);	
    }
  }
  
  bmqClose();
  unlink(sgMonMsg.aFileName);
  return(SUCCESS); 
}
