/****************************************************************/
/* 模块编号    ：BMQFLIEMNG                                     */
/* 模块名称    ：文件传输管理模块                               */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军　                                         */
/* 建立日期    ：2006/12/26                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：管理文件发送的状态,                            */
/*               维护文件发送记录文件和正在发送文件状态表       */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/****************************************************************/
/*库函数定义*/
#include "bmq.h"

int _bmqAddmngrectofile(struct monmsg sFilemng)
{
  int		ilRc;
  char		*alPath;
  char		alFile[101];
  FILE		*fp;
  struct	tm *T;

  T = localtime(&sFilemng.sBeginTime.time);
  
  if ((alPath = getenv("BMQ_FILE_DIR")) == NULL)
  {
    _bmqDebug("S2000: 读系统环境变量[BMQ_PATH]失败");
    return(FAIL);
  }
  
  sprintf(alFile,"%s/mng/filemng.%04d%02d%02d",alPath,T->tm_year+1900,T->tm_mon+1,T->tm_mday);
  
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  fp = fopen(alFile,"a+");
  if(fp == NULL)
  {
  	fp = fopen(alFile,"w+");
  	if(fp == NULL)
  	{
      _bmqDebug("S2010: 无法打开文件传输记录文件[%s],%d:%s",alFile,errno,strerror(errno));
      _bmqUnlock(LK_ALL);
      return(FAIL);
    }
  }
  
  ilRc = fwrite((char *)&sFilemng, sizeof(char), sizeof(struct monmsg), fp);
  if(ilRc != sizeof(struct monmsg))
  {
    _bmqDebug("S2020: 写文件传输记录文件失败:%d:%s",errno,strerror(errno));
    fclose(fp);
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }
  
  fclose(fp);
  _bmqUnlock(LK_ALL);
  return(SUCCESS);	
}

int _bmqUpdatemngrecinfile(struct monmsg sFilemng)
{
  int		ilRc;
  char		*alPath;
  char		alFile[101];
  FILE		*fp;
  struct	tm *T;
  struct	monmsg slFilerec;

  T = localtime(&sFilemng.sBeginTime.time);
  
  if ((alPath = getenv("BMQ_FILE_DIR")) == NULL)
  {
    _bmqDebug("S2030: 读系统环境变量[BMQ_PATH]失败");
    return(FAIL);
  }
  
  sprintf(alFile,"%s/mng/filemng.%04d%02d%02d",alPath,T->tm_year+1900,T->tm_mon+1,T->tm_mday);
  
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  fp = fopen(alFile,"r+");
  if(fp == NULL)
  {
    _bmqDebug("S2040: 无法打开文件传输记录文件[%s],%d:%s",alFile,errno,strerror(errno));
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }
  
  memset(&slFilerec,0x00,sizeof(struct monmsg));
  while(!feof(fp))
  {
    ilRc = fread((char *)&slFilerec, sizeof(char), sizeof(struct monmsg), fp);
    if(ilRc != sizeof(struct monmsg))
    {
      _bmqDebug("S2050: 读取传输记录文件失败%d:%s",errno,strerror(errno));
      _bmqUnlock(LK_ALL);
      return(FAIL);
    }
    if(slFilerec.lSerialno == sFilemng.lSerialno)
    {
      fseek(fp,ftell(fp) - sizeof(struct monmsg),SEEK_SET);
      ilRc = fwrite((char *)&sFilemng, sizeof(char), sizeof(struct monmsg), fp);
      if(ilRc != sizeof(struct monmsg))
      {
        _bmqDebug("S2060: 写文件传输记录文件失败:%d:%s",errno,strerror(errno));
        fclose(fp);
        _bmqUnlock(LK_ALL);
        return(FAIL);
      }
      fclose(fp);
      _bmqUnlock(LK_ALL);
      return(SUCCESS);
    }
  }
  
  fclose(fp);
  _bmqUnlock(LK_ALL);
  return(FINISH);
}

int _bmqAddmngrectoshm(struct monmsg sFilemng)
{
  int		i;
  int		ilRc;
  
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  
  for(i = 0; i <= psgMbshm->iFilecount; i++ )
  {
    if(psgMbfilemng[i].iFlag == 0)/*找到未使用记录段*/
    {
      memcpy(&psgMbfilemng[i],&sFilemng,sizeof(struct monmsg));
      psgMbfilemng[i].iFlag = 1;
      _bmqUnlock(LK_ALL);
      return(SUCCESS);
    }    
  }
  
  _bmqUnlock(LK_ALL);
  return(FINISH);
}

int _bmqUpdatemngrecinshm(struct monmsg sFilemng)
{
  int		i;
  int		ilRc;
  int		ilFlag;
  
  for(i = 0; i <= psgMbshm->iFilecount; i++ )
  {
    if(psgMbfilemng[i].iFlag == 0) continue;
    
    if(psgMbfilemng[i].lSerialno == sFilemng.lSerialno)
    {
      ilRc = _bmqLock(LK_ALL);
        if( ilRc ) return(ilRc);
      ilFlag = psgMbfilemng[i].iFlag;
      memcpy(&psgMbfilemng[i],&sFilemng,sizeof(struct monmsg));
      psgMbfilemng[i].iFlag = ilFlag;
      _bmqUnlock(LK_ALL);
      return(SUCCESS);
    }    
  } 
  
  return(FINISH);
}

int _bmqDelmngrecinshm(struct monmsg sFilemng)
{
  int		i;
  int		ilRc;
  
  for(i = 0; i <= psgMbshm->iFilecount; i++ )
  {
    if(psgMbfilemng[i].iFlag == 0) continue;
  
    if(psgMbfilemng[i].lSerialno == sFilemng.lSerialno)
    {
      ilRc = _bmqLock(LK_ALL);
        if( ilRc ) return(ilRc);
      psgMbfilemng[i].iFlag = 0;
      _bmqUnlock(LK_ALL);
      return(SUCCESS);
    }    
  } 
  
  return(FINISH);
}

/* 主程序 */
int main(int argc,char **argv)
{
  int		ilRc;
  short		ilGrpid;
  short		ilMbid;
  short		ilPorior;
  short		ilTimeout;
  /*short		ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
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
  psgMbshm->lBmqfilemng_pid = getpid();
  
  /*打开文件管理邮箱*/
  ilRc = bmqOpen(iMBFILEMNG);
  if(ilRc)
  { 
    _bmqDebug("S0040: 打开文件管理邮箱出错:%d",ilRc);
    exit(FAIL);
  }
  
  /*阻塞读取文件传输报文*/
  while(1)
  {
  	llType = llClass = ilTimeout = 0;
    ilRc = bmqGetw(&ilGrpid,&ilMbid,&ilPorior,&llType,&llClass,alMsgbuf,&ilMsglen,ilTimeout);
    if(ilRc)
    {
      _bmqDebug("S0050: 阻塞读取文件传输报文出错:%d",ilRc);
      continue;
    }
    
    if(ilMsglen != sizeof(struct monmsg))
    {
      _bmqDebug("S0060: 收到的不是文件传输管理报文,丢弃之.[%d]<>[%d]",ilMsglen,sizeof(struct monmsg));
      continue;
    }
      
    memset(&slFilemng,0x00,sizeof(struct monmsg));
    memcpy((char *)&slFilemng,alMsgbuf,ilMsglen);
    
    switch(slFilemng.iSendFlag)
    {
      case 1:/*开始发送文件*/
        /*在发送记录中增加一条记录*/
        ilRc = _bmqAddmngrectofile(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0070: 在发送记录中增加一条记录失败");         
        }
        /*在共享内存中增加一条发送状态记录*/
        ilRc = _bmqAddmngrectoshm(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0080: 在共享内存中增加一条发送状态记录失败");          
        }
        break;
      case 2:/*正在发送文件*/
      case 4:/*发送异常*/
        /*更新共享内存中发送状态记录*/
        ilRc = _bmqUpdatemngrecinshm(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0090: 更新共享内存中发送状态记录失败,serialno=[%ld]",slFilemng.lSerialno);         
        }
        break;
      case 3:/*发送完成文件*/
      case 5:/*发送失败文件*/
        /*删除共享内存中发送状态记录*/
        ilRc = _bmqDelmngrecinshm(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0100: 删除共享内存中发送状态记录失败serialno=[%ld]",slFilemng.lSerialno);        
        }
        /*更新记录文件中发送记录*/
        ilRc = _bmqUpdatemngrecinfile(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0110: 更新记录文件中发送记录失败");          
        }
        break;
      default:
        _bmqDebug("S0130: 收到未知状态[%d]的报文!",slFilemng.iSendFlag);
        continue; 
    }
    /*信息组包,发送到数据交换平台监控和审计模块*/
    _bmqMonMsgPut(slFilemng);
  } 
}
