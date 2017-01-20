/****************************************************************/
/* 模块编号    ：BMQFLIELOG                                     */
/* 模块名称    ：文件传输异常中断处理模块                       */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军　                                         */
/* 建立日期    ：2006/12/26                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：轮询正在发送的内存记录段，如果发现有发送进程   */
/*               已经不存在的记录,该模块负责完成继续发送服务    */
/* 备注        : 原发送中的报文将丢失                           */		
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/****************************************************************/
/*库函数定义*/
#include "bmq.h"

static int _bmqFilelogproc(struct monmsg sMonmsg);
extern int _bmqFile_comm(char aParm[][iFLDVALUELEN]);
extern int _bmqFile_msg(char aParm[][iFLDVALUELEN]);
extern int _bmqGetfield(char *fname,char *key,int key_n,char *field);
int		igTryTimes;				/*尝试重试次数,缺省为无限制*/
int		igInterVal;				/*尝试间隔时间,缺省为10秒  */

/* 主程序 */
int main(int argc,char **argv)
{
  int		i;
  int		ilRc;
  char		alResult[10];
  char		alInitfile[101];
  
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
  /* 读取系统轮循时间 */
  ilRc = _bmqConfig_load("WTIME",0);
  if (ilRc == -1)
  {
    _bmqDebug("S0040 %s :载入WTIME参数失败，请查看参数文件%s/etc/bmq.ini",__FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"FILETRYTIMES",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0110: 从文件%s中取 FILETRYTIMES 字段失败!",alInitfile);
    igTryTimes = -1;
  }
  else
    igTryTimes = atoi(alResult);
  ilRc = _bmqGetfield(alInitfile,"FILEINTERVAL",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0110: 从文件%s中取 FILEINTERVAL 字段失败!",alInitfile);
    igInterVal = 10;
  }
  else
    igInterVal = atoi(alResult);
  
  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030: 连接共享内存区出错:%d",ilRc);
    exit(FAIL);
  }

  /*保存进程pid*/
  psgMbshm->lBmqfilelog_pid = getpid();
  
  while(1)
  {
    /*等待指定轮询时间*/
    sleep(lgWAITTIME);
    
    for(i = 0; i <= psgMbshm->iFilecount; i++ )
    {
      if(psgMbfilemng[i].iFlag !=1) continue;
    
      /*判断发送进程是否存在*/
      if( kill(psgMbfilemng[i].lSendPid,0) != 0 )
      {
      	/*防止和SAF模块冲突*/
      	sleep(1);
      	if( kill(psgMbfilemng[i].lSendPid,0) == 0 ) continue;
      	
        /*发现发送进程已不存在的记录(发送进程异常中断)*/
        psgMbfilemng[i].iFlag = 2;
        switch ( fork() )
        {
          case -1:
            exit( -1 );
          case 0:
            if ( _bmqFilelogproc(psgMbfilemng[i]) != 0 )
            {
              _bmqDebug("S0070 %s:子进程处理出错!",__FILE__);  
            }
            exit(0);
          default:
            break;
        }        
      }
    }   
  } 
}

/**************************************************************
 ** 函数名: _bmqFilelogproc
 ** 功能:   文件发送异常中断
 ** 作者:	徐军
 ** 建立日期: 2006/12/28
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: SUCCESS--成功，FAIL--失败
***************************************************************/
static int _bmqFilelogproc(struct monmsg sMonmsg)
{
  int		ilRc;
  long		llSerialno = 0; 
  char		alOffset[16];
  char		alFile[100];
  char		alTmpFileName[100];
  struct 	transparm slTransparm;
  struct 	stat f_stat; 
  FILE		*fq;
  
  ilRc = bmqOpen(iMBFILELOG);
  if(ilRc)
  { 
    _bmqDebug("S0040: 打开文件管理邮箱出错:%d",ilRc);
    return(FAIL);
  }
  
  memset(&sgMonMsg,0x00,sizeof(struct monmsg));
  memcpy(&sgMonMsg,&sMonmsg,sizeof(struct monmsg));
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
  
  memset(alTmpFileName,0x00,sizeof(alTmpFileName));
  sprintf(alTmpFileName,"%s/temp/%s.tpf",getenv("BMQ_FILE_DIR"),alFile);
  /*临时文件不存在,新建之*/
  if (( fq = fopen(alTmpFileName,"w+")) == NULL)
  {
    _bmqDebug("S0180: 不能创建临时文件[%s] errno:%d:%s",alTmpFileName,errno,strerror(errno));
    return(-1);
  }
  memset(alOffset,0x00,sizeof(alOffset));
  sprintf(alOffset,"%ld",sgMonMsg.lSendSize);
  fwrite(alOffset,sizeof(char),sizeof(alOffset),fq);
  fclose(fq);
  
  slTransparm.bmqFile_comm	= _bmqFile_comm;
  slTransparm.bmqFile_msg	= _bmqFile_msg;
  
FILESAF:
  llSerialno = sgMonMsg.lSerialno;
  ilRc = _bmqSendfile_G(sgMonMsg.iDesGrpid,sgMonMsg.aFileName,slTransparm,&llSerialno);
  if( ilRc == -1 )
  {
    /* 重新尝试发送 */
    if( (igTryTimes == -1) || (igTryTimes > 0) )
    {
      sleep(igInterVal);
      if(igTryTimes>0)
        igTryTimes--;
      goto FILESAF;
    }
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"发送文件失败,需重新发送");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    bmqClose();
    return(FAIL);
  }
  else if(ilRc == -2)
  {
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"无法打开指定文件");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    bmqClose();
    return(FAIL);
  }
  
  ilRc = bmqPut(sgMonMsg.iDesGrpid,sgMonMsg.iDesMbid,0,0,llSerialno,"AAAA",4);
  if(ilRc)
  {
    _bmqDebug("S0120: [函数调用失败]bmqPut error!");
    bmqClose();
    return(FAIL);	
  }
  
  bmqClose();
  return(SUCCESS); 
}


