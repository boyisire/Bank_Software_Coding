#include <sys/syscall.h>
#include "switch.h"
#include "swDbstruct.h"
#include "swShm.h"
#include "swlog.h"

int _swGetTime(char *date);
FILE *_swFopen(const char *file, const char *mode);
int _swTrim(char *s);
short _swChgvalue(char *aValue);
short swShmcfgload(struct swt_sys_config * pSwt_sys_config);

static char amShmfile[3][101];

/**************************************************************
 ** ������      : _swGetTime
 ** ��  ��      : ȡ��ǰϵͳʱ��, ��ʽYYYY-MM-DD HH:MM:SS.
 ** ��  ��      : llx
 ** ��������    : 1999/11/4
 ** ����޸�����: 2000/3/10
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : date   :���ص��ַ���
 ** ����ֵ      : SUCCESS
***************************************************************/
int _swGetTime(char *date)
{
  long	 Time;
  struct tm *T;

  time(&Time);
  T=localtime(&Time);
  sprintf(date, "%04d-%02d-%02d %02d:%02d:%02d",
    T->tm_year+1900,T->tm_mon+1,T->tm_mday,
    T->tm_hour, T->tm_min, T->tm_sec);
  return(SUCCESS);
}


FILE *_swFopen(const char *file, const char *mode)
{
  char alEnv[256],alFile[256],*p,*s=alEnv;
  FILE *fp;

  if(*file=='/')
  {
    fp=fopen(file,mode);
    return(fp);
  }
  
  if(getenv("SWITCH_CFGDIR")==NULL)
  {
    swVdebug(0,"��������SWITCH_CFGDIRû������!");
    return(NULL);
  }  
  else
    strcpy(alEnv,getenv("SWITCH_CFGDIR"));
    
  while(*s)
  {
    if((p=strchr(s,':'))!=NULL)
    {
      *p=0;
      sprintf(alFile,"%s/%s",s,file);
      s=p+1;
      swVdebug(5,"S6340: _swFopen(): %s",alFile);
      if((fp=fopen(alFile,mode))!=NULL) return(fp);
      if(errno!=ENOENT) return(NULL);
    }
    else
    {
      sprintf(alFile,"%s/%s",s,file);
      fp=fopen(alFile,mode);
      swVdebug(5,"S6350: _swFopen(): %s",alFile);
      return(fp);
    }
  }
  return(NULL);
}


/**************************************************************
 ** ������      : _swTrim
 ** ��  ��      : �ú������ַ��� s ��ǰ��ո�β�س�ȥ��
 ** ��  ��      : llx
 ** ��������    : 1999/11/4
 ** ����޸�����: 2000/3/10
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : s   :�ַ���
 ** ����ֵ      : SUCCESS
		  ת������ַ�����s����
***************************************************************/
int _swTrim(s)
char *s;
{
  short	i, l, r, len;

  for(len=0; s[len]; len++);
  for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
  if(l==len)
  {
    s[0]='\0';
    return(SUCCESS);
  }
  for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
  for(i=l; i<=r; i++) s[i-l]=s[i];
  s[r-l+1]='\0';
  return(SUCCESS);
}


/***************************************************************
 ** ������      : _swChgvalue
 ** ��  ��      : ����Լ���Ĺ���ȡ�ַ�����ֵ
 ** ��  ��      :
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
***************************************************************/
short _swChgvalue(char *aValue)
{
  char alBuf[257];

  _swTrim(aValue);
  if ((aValue[0] == '"') && (aValue[strlen(aValue) - 1] == '"'))
  {
    strcpy(alBuf,aValue);
    strcpy(aValue,alBuf+1);
    aValue[strlen(aValue) - 1] = '\0';
  }
  return 0;
}


/***************************************************************
 ** ������      : swShmcfgload
 ** ��  ��      : �������ڴ��ʼ������
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
 ***************************************************************/
short swShmcfgload(struct swt_sys_config * pSwt_sys_config)
{
    char alBuf[257],alResult[101],*alTmp;
    int i;
    FILE *fp;

    sprintf(amShmfile[0], "%s/log/shm/tranlog", getenv("SWITCH_DIR"));

    sprintf(amShmfile[1], "%s/log/shm/proclog", getenv("SWITCH_DIR"));

    sprintf(amShmfile[2], "%s/log/shm/saflog", getenv("SWITCH_DIR"));

    if ((fp = _swFopen("config/SWCONFIG.CFG", "r")) == NULL) return -1; 
    i = 0;
    while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
    {
        _swTrim(alBuf);
        if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
        if ((alTmp = strchr(alBuf,'#')) != NULL) *alTmp = '\0';
        _swTrim(alBuf);
        if ((alBuf[0] == '[') && (alBuf[strlen(alBuf) -1] == ']')) continue;
        if ((alTmp = strchr(alBuf, '=')) == NULL) continue;
        strcpy(alResult,alTmp + 1);
        _swChgvalue(alResult);
        *alTmp = '\0';
        _swTrim(alBuf);
        if (strcmp(alBuf,"SHMKEY") == 0)
            pSwt_sys_config->iShmkey = atoi(alResult);
        if (strcmp(alBuf,"LOG_INTER") == 0)
            pSwt_sys_config->log_inter = atoi(alResult);
        if (strcmp(alBuf,"SAF_INTER") == 0)
            pSwt_sys_config->saf_inter = atoi(alResult);
        /* ========= begin of add by dgm 2002.3.19 ========== */
        if (strcmp(alBuf,"REREV_INTER") == 0)
            pSwt_sys_config->rerev_inter = atol(alResult);
        /* ========== end of add by dgm 2002.3.19 =========== */
        if (strcmp(alBuf,"MAXTRANLOG") == 0)
            pSwt_sys_config->iMaxtranlog = atoi(alResult);
        if (strcmp(alBuf,"MAXPROCLOG") == 0)
            pSwt_sys_config->iMaxproclog = atoi(alResult);
        if (strcmp(alBuf,"MAXSAFLOG") == 0)
            pSwt_sys_config->iMaxsaflog = atoi(alResult);
        if (strcmp(alBuf,"SHMWAITTIME") == 0)
            pSwt_sys_config->iShmwaittime = atoi(alResult);
        if (strcmp(alBuf,"SYSMON_IP") == 0)
            strcpy(pSwt_sys_config->sysmonaddr,alResult);
        if (strcmp(alBuf,"SYSMON_PORT") == 0)
            pSwt_sys_config->sysmonport = atoi(alResult);
        if (strcmp(alBuf,"TRACE_TERM") == 0)
            strcpy(pSwt_sys_config->trace_term, alResult);
    }
    fclose(fp);
    return 0;
} 
