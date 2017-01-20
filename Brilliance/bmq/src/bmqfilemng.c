/****************************************************************/
/* ģ����    ��BMQFLIEMNG                                     */
/* ģ������    ���ļ��������ģ��                               */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �������                                         */
/* ��������    ��2006/12/26                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    �������ļ����͵�״̬,                            */
/*               ά���ļ����ͼ�¼�ļ������ڷ����ļ�״̬��       */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/****************************************************************/
/*�⺯������*/
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
    _bmqDebug("S2000: ��ϵͳ��������[BMQ_PATH]ʧ��");
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
      _bmqDebug("S2010: �޷����ļ������¼�ļ�[%s],%d:%s",alFile,errno,strerror(errno));
      _bmqUnlock(LK_ALL);
      return(FAIL);
    }
  }
  
  ilRc = fwrite((char *)&sFilemng, sizeof(char), sizeof(struct monmsg), fp);
  if(ilRc != sizeof(struct monmsg))
  {
    _bmqDebug("S2020: д�ļ������¼�ļ�ʧ��:%d:%s",errno,strerror(errno));
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
    _bmqDebug("S2030: ��ϵͳ��������[BMQ_PATH]ʧ��");
    return(FAIL);
  }
  
  sprintf(alFile,"%s/mng/filemng.%04d%02d%02d",alPath,T->tm_year+1900,T->tm_mon+1,T->tm_mday);
  
  ilRc = _bmqLock(LK_ALL);
  if( ilRc ) return(ilRc);
  fp = fopen(alFile,"r+");
  if(fp == NULL)
  {
    _bmqDebug("S2040: �޷����ļ������¼�ļ�[%s],%d:%s",alFile,errno,strerror(errno));
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }
  
  memset(&slFilerec,0x00,sizeof(struct monmsg));
  while(!feof(fp))
  {
    ilRc = fread((char *)&slFilerec, sizeof(char), sizeof(struct monmsg), fp);
    if(ilRc != sizeof(struct monmsg))
    {
      _bmqDebug("S2050: ��ȡ�����¼�ļ�ʧ��%d:%s",errno,strerror(errno));
      _bmqUnlock(LK_ALL);
      return(FAIL);
    }
    if(slFilerec.lSerialno == sFilemng.lSerialno)
    {
      fseek(fp,ftell(fp) - sizeof(struct monmsg),SEEK_SET);
      ilRc = fwrite((char *)&sFilemng, sizeof(char), sizeof(struct monmsg), fp);
      if(ilRc != sizeof(struct monmsg))
      {
        _bmqDebug("S2060: д�ļ������¼�ļ�ʧ��:%d:%s",errno,strerror(errno));
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
    if(psgMbfilemng[i].iFlag == 0)/*�ҵ�δʹ�ü�¼��*/
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

/* ������ */
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
  psgMbshm->lBmqfilemng_pid = getpid();
  
  /*���ļ���������*/
  ilRc = bmqOpen(iMBFILEMNG);
  if(ilRc)
  { 
    _bmqDebug("S0040: ���ļ������������:%d",ilRc);
    exit(FAIL);
  }
  
  /*������ȡ�ļ����䱨��*/
  while(1)
  {
  	llType = llClass = ilTimeout = 0;
    ilRc = bmqGetw(&ilGrpid,&ilMbid,&ilPorior,&llType,&llClass,alMsgbuf,&ilMsglen,ilTimeout);
    if(ilRc)
    {
      _bmqDebug("S0050: ������ȡ�ļ����䱨�ĳ���:%d",ilRc);
      continue;
    }
    
    if(ilMsglen != sizeof(struct monmsg))
    {
      _bmqDebug("S0060: �յ��Ĳ����ļ����������,����֮.[%d]<>[%d]",ilMsglen,sizeof(struct monmsg));
      continue;
    }
      
    memset(&slFilemng,0x00,sizeof(struct monmsg));
    memcpy((char *)&slFilemng,alMsgbuf,ilMsglen);
    
    switch(slFilemng.iSendFlag)
    {
      case 1:/*��ʼ�����ļ�*/
        /*�ڷ��ͼ�¼������һ����¼*/
        ilRc = _bmqAddmngrectofile(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0070: �ڷ��ͼ�¼������һ����¼ʧ��");         
        }
        /*�ڹ����ڴ�������һ������״̬��¼*/
        ilRc = _bmqAddmngrectoshm(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0080: �ڹ����ڴ�������һ������״̬��¼ʧ��");          
        }
        break;
      case 2:/*���ڷ����ļ�*/
      case 4:/*�����쳣*/
        /*���¹����ڴ��з���״̬��¼*/
        ilRc = _bmqUpdatemngrecinshm(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0090: ���¹����ڴ��з���״̬��¼ʧ��,serialno=[%ld]",slFilemng.lSerialno);         
        }
        break;
      case 3:/*��������ļ�*/
      case 5:/*����ʧ���ļ�*/
        /*ɾ�������ڴ��з���״̬��¼*/
        ilRc = _bmqDelmngrecinshm(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0100: ɾ�������ڴ��з���״̬��¼ʧ��serialno=[%ld]",slFilemng.lSerialno);        
        }
        /*���¼�¼�ļ��з��ͼ�¼*/
        ilRc = _bmqUpdatemngrecinfile(slFilemng);
        if(ilRc)
        {
          _bmqDebug("S0110: ���¼�¼�ļ��з��ͼ�¼ʧ��");          
        }
        break;
      default:
        _bmqDebug("S0130: �յ�δ֪״̬[%d]�ı���!",slFilemng.iSendFlag);
        continue; 
    }
    /*��Ϣ���,���͵����ݽ���ƽ̨��غ����ģ��*/
    _bmqMonMsgPut(slFilemng);
  } 
}
