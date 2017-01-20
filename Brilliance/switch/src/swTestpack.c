#include <stdio.h>
#include <sys/timeb.h>
#include "swapi.h"
#define iFLDNUMBER2	200

char cgDebug=2;

extern int swRecvpacklw(short *, char *, unsigned int *, short *, long *, long *);
extern int swSendpackl(short, char *, unsigned int , short, long, long);

int swLoadpack(char *aKey, char *, unsigned int *);
void swQuit(int sig);

int main(int argc,char **argv)
{
  short ilQ_org,ilQ_dest;
  unsigned int ilMsglen,ilMsgreturn;
  long  llClass,llType;
  short i,ilRc,ilPriority;
  short ilQid,ilSendnum;
  long lltime1,lltime2,llTimetmp,llTimetmp2;
  struct msgpack slMsgpack;
  char  alKey[200];
  struct timeb  fb;

  strcpy(agDebugfile,"swTestpack.debug");
  if(argc !=5) {
    fprintf(stderr, "ʹ�÷���: swTestpack ���ļ��� ���ʹ��� Դ���� Ŀ������\n");
    exit(-1);
  }

  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  /* signal(SIGINT,SIG_IGN); */
  signal(SIGTSTP,SIG_IGN);
  signal(SIGSTOP,SIG_IGN);
  signal(SIGUSR1,SIG_IGN);
  signal(SIGUSR2,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
  signal(SIGCLD,SIG_IGN);
  signal(SIGTERM,swQuit);

  strcpy(alKey, argv[1]);
  ilSendnum = atoi(argv[2]);
  ilQ_org = atoi(argv[3]);
  ilQ_dest = atoi(argv[4]);

  /* ���1��FML���� */


  memset((char *)&slMsgpack,0x00,sizeof(struct msgpack));
  _swTrim(alKey);
  ilRc = swLoadpack(alKey, slMsgpack.aMsgbody, &ilMsglen);
  if (ilRc)
  {
    swVdebug(1,"S0000: load [%s] wrong ", alKey);
    exit(-1);
  }
  ilMsglen += sizeof(struct msghead);
  swVdebug(3,"��ɵı�����Ϊ: ");
  if (cgDebug>1)
    swDebughex((char *)&slMsgpack,ilMsglen);

  fprintf(stderr,"This is a test pack!\n");
  printf("%s\n",stderr);
  ilRc = swMbopen(ilQ_org);
  if (ilRc)
  {
    fprintf(stderr,"������[%d]����!\n",ilQ_org);
    exit(-1);
  }

  time(&lltime1);
  ftime(&fb);
  llTimetmp = fb.millitm;
  llTimetmp2 = fb.time;
  fprintf(stderr,"��ʼ���ͱ���...\n");
  for(i=1;i<=ilSendnum;i++)
  {
    ilPriority = 0;
    llClass = 0L;
    llType = 0L;
    ilRc = swSendpackl(ilQ_dest,(char *)&slMsgpack,ilMsglen,
      ilPriority,llClass,llType);
    if (ilRc)
    {
      fprintf(stderr,"���ķ���ʱ����: %d",ilRc);
      exit(-1);
    }
    /*fprintf(stderr,"����������: \n[%d][%s]\n",ilMsglen,slMsgpack.aMsgbody);*/
  }
  fprintf(stderr,"��ʼ���ձ���...\n");
  for(i=1;i<=ilSendnum;i++)
  {
    ilMsgreturn = iMSGMAXLEN;
    ilPriority = 0;
    llClass = 0L;
    llType = 0L;
    ilRc = swRecvpacklw(&ilQid,(char *)&slMsgpack,&ilMsgreturn,
      &ilPriority,&llClass,&llType);
    if (ilRc)
    {
      fprintf(stderr,"���ձ���ʱ����: %d",ilRc);
      exit(-1);
    }
    /*fprintf(stderr,"���յ���Ӧ����: \n[%d][%s]\n",ilMsgreturn,slMsgpack.aMsgbody);*/
  }
    
  fprintf(stderr,"���ձ������!!!\n");
  fprintf(stderr,"swTest is ok!!!\n");

  swVdebug(3,"��[%d]�����յ��ı���:",ilQid);
  if (cgDebug>1)
    swDebughex((char *)&slMsgpack,ilMsgreturn);

  time(&lltime2);
  ftime(&fb);
 
  fprintf(stderr,"ʹ��ʱ��(s) = [%ld]\n",(lltime2-lltime1));
  fprintf(stderr,"ʹ��ʱ��(m) = [%ld]\n",(fb.millitm-llTimetmp)+(fb.time-llTimetmp2)*1000);
  swVdebug(3,"ʹ��ʱ��(s) = [%ld]",(lltime2-lltime1));
  swVdebug(3,"ʹ��ʱ��(m) = [%ld]",(fb.millitm-llTimetmp)+(fb.time-llTimetmp2)*1000);
  
  /* �ر�ǰ�ý��������˿� */
  swMbclose();
  exit(0);
}

int swLoadpack(char *aFile, char *aMsgpack, unsigned int *piMsglen)
{
  short ilRc;
  char alFile[200];
  FILE *fp;
  
  memset(alFile,0x00,sizeof(alFile));
  sprintf(alFile,"%s/file/",getenv("HOME"));
  strcat(alFile,aFile);
  if ((fp = fopen(alFile, "r")) == NULL)
  {
    swVdebug(1,"S1010: ���ļ���pack.fml ����");
    return -1;
  }
  *piMsglen = fread(aMsgpack,1,8192,fp);
  if (*piMsglen <= 0)
  {
    fclose(fp); 
    return -1;
  }

  fclose(fp);
  return 0;
}
void swQuit(int iSig)
{
  swMbclose();
  exit(0);
}

