/****************************************************************/
/* ģ����    ��BMQFLIESAF                                     */
/* ģ������    ���ļ�����SAFģ��                                */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �������                                         */
/* ��������    ��2006/12/26                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���������͹����г����쳣���ļ���ɼ������͵ķ��� */
/*               ����ָ��ʱ��δ����ɷ���ʱ���׳�����ʧ����Ϣ   */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/****************************************************************/
/*�⺯������*/
#include "bmq.h"

/*static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,short iMsglen);delete by wh*/
static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,TYPE_SWITCH iMsglen);/*add by wh*/
extern int _bmqFile_comm(char aParm[][iFLDVALUELEN]);
extern int _bmqFile_msg(char aParm[][iFLDVALUELEN]);
extern int _bmqGetfield(char *fname,char *key,int key_n,char *field);

/* ������ */
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
  
  /*��ʾ�汾��Ϣ*/
  _bmqShowversion(argc,argv);

  /*�����ź�*/
  _bmqSignalinit();
  
  /* �����ػ����� */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010: �����ػ�����ʧ�ܣ��˳�!");
    exit(FAIL);
  }
  
  /*������������ļ�DEBUGFLAG*/
  ilRc = _bmqConfig_load("DEBUG",0);
  if (ilRc < 0)
  { 
    _bmqDebug("S0020: ����DEBUG����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",getenv("BMQ_PATH"));
    exit(FAIL);
  }
  
  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030: ���ӹ����ڴ�������:%d",ilRc);
    exit(FAIL);
  }
  
  /*�������pid*/
  psgMbshm->lBmqfilesaf_pid = getpid();
  
  /*���ļ�SAF����*/
  ilRc = bmqOpen(iMBFILESAF);
  if(ilRc)
  { 
    _bmqDebug("S0040: ���ļ������������:%d",ilRc);
    exit(FAIL);
  }
  
  while(1)
  {
    llType = llClass = ilTimeout = 0;
    ilRc = bmqGetw(&ilGrpid,&ilMbid,&ilPorior,&llType,&llClass,alMsgbuf,&ilMsglen,ilTimeout);
    if(ilRc)
    {
      _bmqDebug("S0050: ������ȡ�ļ�����SAF���ĳ���:%d",ilRc);
      continue;
    }
    
    if(ilMsglen < sizeof(struct monmsg))
    {
      _bmqDebug("S0060: �յ��Ĳ����ļ�����SAF����,����֮.[%d]<>[%d]",ilMsglen,sizeof(struct monmsg));
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
          _bmqDebug("S0070 %s:�ӽ��̴������!",__FILE__);  
        }
        exit(0);
      default:
        break;
    }    
  } 
}

/**************************************************************
 ** ������: _bmqFilesafproc
 ** ����:   �ļ��ϵ�����SAFģ��
 ** ����:	���
 ** ��������: 2006/12/28
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
/*static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,short iMsglen)delete by wh*/
static int _bmqFilesafproc(short iPorior,long lType,long lClass,char *aMsgbuf,TYPE_SWITCH iMsglen)/*add by wh*/
{
  int		ilRc;
  int		ilTryTimes;				/*�������Դ���,ȱʡΪ������*/
  int		ilInterVal;				/*���Լ��ʱ��,ȱʡΪ10��  */
  long		llSerialno = 0;
  char		alResult[10];
  char		alInitfile[101];
  char		alFile[100];
  char		alTmpFileName[100];
  struct 	transparm slTransparm;
  struct 	stat f_stat; 
  
  /*���ļ�SAF����*/
  ilRc = bmqOpen(iMBFILESAF);
  if(ilRc)
  { 
    _bmqDebug("S0040: ���ļ������������:%d",ilRc);
    return(FAIL);
  }
  
  memset(&sgMonMsg,0x00,sizeof(struct monmsg));
  memcpy(&sgMonMsg,aMsgbuf,sizeof(struct monmsg));
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  _bmqfilechr(sgMonMsg.aFileName,strlen(sgMonMsg.aFileName),alFile,'/');
  sprintf(alTmpFileName,"%s/saf/%s",getenv("BMQ_FILE_DIR"),alFile);
  
  if( stat( alTmpFileName, &f_stat ) == -1 )
  {
    _bmqDebug("S0100: �޷���ָ���ļ�[%s]!",alFile);
    sgMonMsg.iSendFlag  = 5;
    strcpy(sgMonMsg.aStatusDesc,"�޷���ָ���ļ�");
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
    _bmqDebug("S0110: ���ļ�%s��ȡ FILETRYTIMES �ֶ�ʧ��!",alInitfile);
    ilTryTimes = -1;
  }
  else
    ilTryTimes = atoi(alResult);
  ilRc = _bmqGetfield(alInitfile,"FILEINTERVAL",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0110: ���ļ�%s��ȡ FILEINTERVAL �ֶ�ʧ��!",alInitfile);
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
    /* ���³��Է��� */
    if( (ilTryTimes == -1) || (ilTryTimes > 0) )
    {
      sleep(ilInterVal);
      if(ilTryTimes>0)
        ilTryTimes--;
      goto FILESAF;
    }
    bmqClose();
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"�����ļ�ʧ��,�����·���");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    unlink(sgMonMsg.aFileName);
    return(FAIL);
  }
  else if(ilRc == -2)
  {
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"���ܴ���ʱ�ļ�");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    return(FAIL);
  } 	
  
  if(lClass == 1)
  {
    ilRc = bmqPut(sgMonMsg.iDesGrpid,sgMonMsg.iDesMbid,iPorior,lType,llSerialno,
                  aMsgbuf+sizeof(struct monmsg),iMsglen-sizeof(struct monmsg));
    if(ilRc)
    {
      _bmqDebug("S0120: [��������ʧ��]bmqPut error!");
      bmqClose();
      unlink(sgMonMsg.aFileName);
      return(FAIL);	
    }
  }
  
  bmqClose();
  unlink(sgMonMsg.aFileName);
  return(SUCCESS); 
}
