/****************************************************************/
/* ģ����    ��BMQFLIELOG                                     */
/* ģ������    ���ļ������쳣�жϴ���ģ��                       */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �������                                         */
/* ��������    ��2006/12/26                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ����ѯ���ڷ��͵��ڴ��¼�Σ���������з��ͽ���   */
/*               �Ѿ������ڵļ�¼,��ģ�鸺����ɼ������ͷ���    */
/* ��ע        : ԭ�����еı��Ľ���ʧ                           */		
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/****************************************************************/
/*�⺯������*/
#include "bmq.h"

static int _bmqFilelogproc(struct monmsg sMonmsg);
extern int _bmqFile_comm(char aParm[][iFLDVALUELEN]);
extern int _bmqFile_msg(char aParm[][iFLDVALUELEN]);
extern int _bmqGetfield(char *fname,char *key,int key_n,char *field);
int		igTryTimes;				/*�������Դ���,ȱʡΪ������*/
int		igInterVal;				/*���Լ��ʱ��,ȱʡΪ10��  */

/* ������ */
int main(int argc,char **argv)
{
  int		i;
  int		ilRc;
  char		alResult[10];
  char		alInitfile[101];
  
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
  /* ��ȡϵͳ��ѭʱ�� */
  ilRc = _bmqConfig_load("WTIME",0);
  if (ilRc == -1)
  {
    _bmqDebug("S0040 %s :����WTIME����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",__FILE__,getenv("BMQ_PATH"));
    exit(FAIL);
  }
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"FILETRYTIMES",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0110: ���ļ�%s��ȡ FILETRYTIMES �ֶ�ʧ��!",alInitfile);
    igTryTimes = -1;
  }
  else
    igTryTimes = atoi(alResult);
  ilRc = _bmqGetfield(alInitfile,"FILEINTERVAL",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0110: ���ļ�%s��ȡ FILEINTERVAL �ֶ�ʧ��!",alInitfile);
    igInterVal = 10;
  }
  else
    igInterVal = atoi(alResult);
  
  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0030: ���ӹ����ڴ�������:%d",ilRc);
    exit(FAIL);
  }

  /*�������pid*/
  psgMbshm->lBmqfilelog_pid = getpid();
  
  while(1)
  {
    /*�ȴ�ָ����ѯʱ��*/
    sleep(lgWAITTIME);
    
    for(i = 0; i <= psgMbshm->iFilecount; i++ )
    {
      if(psgMbfilemng[i].iFlag !=1) continue;
    
      /*�жϷ��ͽ����Ƿ����*/
      if( kill(psgMbfilemng[i].lSendPid,0) != 0 )
      {
      	/*��ֹ��SAFģ���ͻ*/
      	sleep(1);
      	if( kill(psgMbfilemng[i].lSendPid,0) == 0 ) continue;
      	
        /*���ַ��ͽ����Ѳ����ڵļ�¼(���ͽ����쳣�ж�)*/
        psgMbfilemng[i].iFlag = 2;
        switch ( fork() )
        {
          case -1:
            exit( -1 );
          case 0:
            if ( _bmqFilelogproc(psgMbfilemng[i]) != 0 )
            {
              _bmqDebug("S0070 %s:�ӽ��̴������!",__FILE__);  
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
 ** ������: _bmqFilelogproc
 ** ����:   �ļ������쳣�ж�
 ** ����:	���
 ** ��������: 2006/12/28
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
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
    _bmqDebug("S0040: ���ļ������������:%d",ilRc);
    return(FAIL);
  }
  
  memset(&sgMonMsg,0x00,sizeof(struct monmsg));
  memcpy(&sgMonMsg,&sMonmsg,sizeof(struct monmsg));
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
  
  memset(alTmpFileName,0x00,sizeof(alTmpFileName));
  sprintf(alTmpFileName,"%s/temp/%s.tpf",getenv("BMQ_FILE_DIR"),alFile);
  /*��ʱ�ļ�������,�½�֮*/
  if (( fq = fopen(alTmpFileName,"w+")) == NULL)
  {
    _bmqDebug("S0180: ���ܴ�����ʱ�ļ�[%s] errno:%d:%s",alTmpFileName,errno,strerror(errno));
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
    /* ���³��Է��� */
    if( (igTryTimes == -1) || (igTryTimes > 0) )
    {
      sleep(igInterVal);
      if(igTryTimes>0)
        igTryTimes--;
      goto FILESAF;
    }
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"�����ļ�ʧ��,�����·���");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    bmqClose();
    return(FAIL);
  }
  else if(ilRc == -2)
  {
    sgMonMsg.iSendFlag  = 5;
  	strcpy(sgMonMsg.aStatusDesc,"�޷���ָ���ļ�");
    _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
    bmqClose();
    return(FAIL);
  }
  
  ilRc = bmqPut(sgMonMsg.iDesGrpid,sgMonMsg.iDesMbid,0,0,llSerialno,"AAAA",4);
  if(ilRc)
  {
    _bmqDebug("S0120: [��������ʧ��]bmqPut error!");
    bmqClose();
    return(FAIL);	
  }
  
  bmqClose();
  return(SUCCESS); 
}


