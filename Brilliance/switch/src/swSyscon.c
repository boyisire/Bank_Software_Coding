/****************************************************************/
/* ģ����    ��SYSCON                                         */
/* ģ������    ����������                                       */
/* �� �� ��    ��V4.3.0                                         */
/* ��    ��    ���Ż�                                           */
/* ��������    ��1999/11/22                                     */
/* ����޸����ڣ�2001/8/20                                      */
/* ģ����;    ���¼�������                                     */
/* ��ģ���а������º���������˵����                             */
/*                         ��1�� void main();                   */
/*                         ��2�� int swProc_fun();              */
/*                         ��3�� int swShowhelp();              */
/*                         ��4�� int swGetsysconcfg();          */
/*                         ��5�� int swAnalyse();               */
/*                         ��6�� int Message();                 */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*   2001/08/20 ���Ӳ��������                                  */

/* ͷ�ļ����� */
#include "switch.h" 

/* add by gxz 2003.03.27 */
int swStop();         /* ֹͣ����ƽ̨                 */
/* add by gxz 2003.03.27 */

int swListtask();     /* ��ʾ������Ϣ                 */
int swStarttask();    /* ��������                     */
int swStoptask();     /* ֹͣ����                     */
int swListmailbox();  /* ��ʾ����״̬                 */
int swClrmailbox();   /* �������                     */
int swListport();     /* �鿴�˿�״̬                 */
int swConup();        /* �ö˿�Ӧ�ò�UP               */
int swCondown();      /* �ö˿�Ӧ�ò�DOWN             */
int swConset();       /* �ö˿�״̬ΪUP/DOWN          */
int swListtran1();    /* ��ʾ���ڴ���Ľ�����Cˮ      */
int swListtran2();    /* ��ʾ��ǰ�����մ���ɹ��Ľ��� */
int swListtran3();    /* ��ʾ�����ɹ��Ľ���           */
int swListtran4();    /* ��ʾ����ʧ�ܵĽ���           */
int swResaf();        /* RESAF ����                   */
int swListsaf();      /* ��ʾ SAF ����                */
int swClrsaf();       /* ��� SAF                     */
/* === begin of added by fzj at 2002.03.2 === */
int swListporttran(); /* ��ʾ�˿ڽ�����               */
int swResetporttran();/* ���ö˿ڽ�����               */
/* === end of added by fzj at 2002.03.2 === */
/* add by gengling at 2015.04.17 begin PSBC_V1.0 */
int swShmtranflowreset();    /* ����ĳһ�ʽ����������ƵĽ����� */
int swShmtranflowresetall(); /* �������н����������ƵĽ�����   */
/* add by gengling at 2015.04.17 end PSBC_V1.0 */

struct sFunction
{
  char alCmd_ch[100];      /* ������ʶ,Ӧ���������ļ��е�������ͬ      */
  int (*plFunc)();         /* ʵ��ִ�еĺ�������,���������������ĺ���  */
};
/* ���庯������Ӧ�� */

struct sFunction psgFunlist[] = {
  {"swListtask()",swListtask},
  {"swStarttask()",swStarttask},
  {"swStoptask()",swStoptask},
  {"swConup()",swConup},
  {"swCondown()",swCondown},
  {"swConset()",swConset},
  {"swListtran1()",swListtran1},
  {"swListtran2()",swListtran2},
  {"swListtran3()",swListtran3},
  {"swListtran4()",swListtran4},
  {"swListsaf()",swListsaf},
  {"swListport()",swListport},
  {"swListmailbox()",swListmailbox},
  {"swClrmailbox()",swClrmailbox},
  {"swResaf()",swResaf},
  {"swClrsaf()",swClrsaf},
/* === begin of added by fzj at 2002.03.02 === */
  {"swListporttran()",swListporttran},
  {"swResetporttran()",swResetporttran},
/* === end of added by fzj at 2002.03.02 === */
/* add by gengling at 2015.04.17 begin PSBC_V1.0 */
  {"swShmtranflowreset()",swShmtranflowreset},
  {"swShmtranflowresetall()",swShmtranflowresetall},
/* add by gengling at 2015.04.17 end PSBC_V1.0 */
  {"",NULL}
};

struct cmdtype
{
  char aCord[11];
  char aMenu[21];       /*����˵���*/
  char aBief[2];        /*������*/
  char aCmd[21];        /*����ȫ��*/
  char aProc[41];       /*����/������*/
  char cRbz;            /*���ñ�־ 0-ж�� 1��Ҷ���� 2-Ҷ���� 3�����ɺ���*/
};

char  alFilename[101]; 

int swProc_fun(char *funame,char alParm[][101]);
int swShowhelp(char *aCord);
int swAnalyse(char * alKeycmd,char palParm[][101]);
int swGetsysconcfg(char *alCmd,struct cmdtype *psCurrentcmd);
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
int Message(char *format, ...);
#endif
#ifdef OS_AIX
int Message(format,va_list);
char *format
va_dcl
#endif
#ifdef OS_HPUX
int Message(char *format, ...);
#endif
#ifdef OS_SCO
int Message(char *format, ...);
#endif
#ifdef OS_SOLARIS
int Message(char *format, ...);
#endif

int main(int argc,char *argv[])
{
  struct cmdtype slCurrentcmd; 

  char  alKeycmd[1001];       /*���������ַ���*/
  char  alCmd[51];            /*����������ַ����������*/
  char  palParm[10][101];     /*����������ַ����Ĳ�������*/
  char  alCommand[257];
  int   ilRc,i;
  char  alParent[21];
  int   c;

  /* ��ӡ�汾�� */
  if ( argc > 1 )
    _swVersion("Syscon Version 4.3.0",argv[1]);

  /* ��������ļ� */
  strncpy(agDebugfile,"swSyscon.debug",sizeof(agDebugfile));

  /* ȡ DEBUG ��־ */
  if ((cgDebug = _swDebugflag("swSyscon")) == FAIL)
  {
    fprintf(stderr,"ȡDEBUG��־����!\n");
    return(FAIL);
  }

  /* �Զ���ʽ�����������ļ� */
  sprintf(alFilename,"config/%s","SWSYSCON.CFG");

  if(argc >= 2)
  {
    if (memcmp(argv[1],"end",3) == 0 )
    {
      ilRc = swStop();
      swVdebug(0,"ж�ؽ���ƽ̨����...");
      exit(0);
    }
  }  
  while ( (c=getopt(argc,argv,"f:")) != -1 )
  {
    switch( c )
    {
      case 'f':
        strcpy(alFilename,optarg);
        _swTrim(alFilename);
        break;
      default:
        fprintf(stdout,"Usage: swSyscon [-f ������] !\n");
        exit(-1);
    }
  } /* end while */

  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGCLD,SIG_IGN); 
  
  /* ������ʼ�� */
  memset(alParent,0x00,sizeof(alParent));
  strcpy(slCurrentcmd.aCord,""); /* ��ʼ��������Ϊ�� */

  /* ��һ�ν���syscon ��ʾ������ */
  swShowhelp("");
    
  /* ������ѭ�� */
  for(;;)
  {
    /*�ȴ�����������������alKeycmd��
    ��������������ַ���alKeycmd
    �������palParm[0]��,����alCmdȡСд
    ��������palParm[]��.����1����palParm[1]��,����2����parmParm[2]��...
    */                           
    if ( strlen(slCurrentcmd.aCord) == 0 )
      printf("\n[syscon]>");
    else 
    {
      /* strtoupper(alParent); */
      printf("\n[syscon|%s]>",alParent);
    }

    if (fgets(alKeycmd,256,stdin) == NULL)
      exit(-1);
    /* gets(alKeycmd); */
   
    if (alKeycmd == NULL) continue;

    _swTrim(alKeycmd);
    if (strlen(alKeycmd) == 0) continue;

    swAnalyse(alKeycmd,palParm);
    strcpy(alCmd,palParm[0]);
   
   /* if ( ��������������"q"��"quit" ) */
    if ((strcmp(alCmd,"q") == 0) || (strcmp(alCmd,"quit") == 0))
    {
      /*  �����в㷵����һ������,������볤�� == 0 ) */
      if ( strlen(slCurrentcmd.aCord) == 0 )
      {
        /* ��ʾ��ǰ���������������,�˳�ϵͳ����ģ��;����unix����ϵͳ;*/
        exit(0);
      }
      /* slCurrentcmd.aCord��ȥ��2λ  */
      slCurrentcmd.aCord[strlen(slCurrentcmd.aCord) - 2] = '\0';

      memset(slCurrentcmd.aCmd,0x00,sizeof(slCurrentcmd.aCmd));
      strcpy(alParent,slCurrentcmd.aCmd);

      swShowhelp(slCurrentcmd.aCord);
      continue;
    }

    /* if ( ��������������"h"��"help" )  */
    if ((strcmp(alCmd,"h") == 0) || (strcmp(alCmd,"help") == 0) ||
      (strcmp(alCmd,"?") == 0))
    {
      /* ���Ե�ǰ�������µİ����˵��˵� */
      swShowhelp(slCurrentcmd.aCord);
      continue;
    }

    ilRc = swGetsysconcfg(alCmd,&slCurrentcmd);
    if (ilRc)
    {
      printf("�Ƿ�������\n");
      continue;
    }  
    if (slCurrentcmd.aProc[0] == '\0' )   /* ���ɺ���  */
    {
      swShowhelp(slCurrentcmd.aCord);
      strcpy(alParent,slCurrentcmd.aCmd);
      continue;
    }
    if(slCurrentcmd.aProc[strlen(slCurrentcmd.aProc) - 1] == ')')
    {
      /*Ҷ����*/
      swProc_fun(slCurrentcmd.aProc,&(palParm[1]));
      slCurrentcmd.aCord[strlen(slCurrentcmd.aCord) - 2] = '\0';
      continue;
    }
    /*Ҷ����*/
    strcpy(alCommand,slCurrentcmd.aProc);
    for (i=0;palParm[i+1][0] != '\0';i++)
    {
      strcat(alCommand," ");
      strcat(alCommand,palParm[i+1]);
    }
    system(alCommand);
    slCurrentcmd.aCord[strlen(slCurrentcmd.aCord) - 2] = '\0';
  }  /* for (::) */
}  /* main() */                   
   
  
/**************************************************************
** ��������swProc_fun
** ��  �ܣ� ���ݼ�������cmd,�ڽṹ����slCommand�в���ƥ��
           alCmd_ch,���ҵ���ִ�к���ָ��plFunc��ָ�ĺ���
           �����У�command[I].plFunc(alParm)
** ��  �ߣ� ���پ�
** �������ڣ�
** ����޸����ڣ�
** ��������������
** ȫ�ֱ�����
** �������壺funame�����Ӧ�ĺ�����.  alParm �������
** ����ֵ��void
***************************************************************/
int swProc_fun(char *aFunname,char alParm[][101])
{
  int i,ilRc;
  i = 0;

  swVdebug(4,"S0010: [��������] swProc_fun()");

  /*while( strcmp(psgFunlist[i].alCmd_ch,NULL))*/
  while( strlen(psgFunlist[i].alCmd_ch))
  {
    if ( strcmp(psgFunlist[i].alCmd_ch,aFunname) == 0 )
    {
      if ( alParm[0][0] == '\0' )
        ilRc = psgFunlist[i].plFunc(alParm[0]);
      else if ( alParm[1][0] != '\0' )
        ilRc = psgFunlist[i].plFunc(alParm[0],alParm[1]);
      else
       ilRc = psgFunlist[i].plFunc(alParm[0]);
      swVdebug(4,"S0020: [��������] swProc_fun()������=0");
      return 0;
    }
    i++;
  } 
  return(-1);
}
  
/**************************************************************
** ��������swShowhelp
** ��  �ܣ� ��ѯfp��ָ�������ļ���ѯ���뵱ǰ����CMDTYPE��ƥ
            ��������������в�������ʾ���slCurrentcmd=NULL
            ����ʾ��һ������˵�
** ��  �ߣ� ���پ�
** �������ڣ�
** ����޸����ڣ�2001.08.20
** ��������������
** ȫ�ֱ����� alFilename
** �������壺 * slCurrentcmd�ǵ�ǰ���� 
** ����ֵ�� ����0,�ɹ���1
***************************************************************/
int swShowhelp(char *aCord)
{  
  char palFldvalue[12][iFLDVALUELEN];
  FILE *fp;
  unsigned int ilRc,ilLen;
  
  swVdebug(4,"S0030: [��������] swShowhelp()");

  fp = _swFopen(alFilename,"r");
  if (fp == NULL)
  {
    swVdebug(0,"S0040: [����/ϵͳ����] fopen()����,errno=%d,�����ļ��޷���!",errno);
    Message("�����ļ��޷���!\n");
    exit(-1);
  }
  swVdebug(2,"S0050: �����ļ��� OK!");
  
  printf("\n        [�����]    [ȫ��]  [��д] [����]\n");
  printf("        ------------------------------------\n");       
  ilLen = strlen(aCord) + 2;
  for (;;)
  {
    ilRc = _swGetitemmode2(fp,palFldvalue,12);
    if (ilRc) break;
    if (strncmp(aCord,palFldvalue[0],strlen(aCord)) == 0)
    {
      if (strlen(palFldvalue[0]) == ilLen)
      {
  	printf("\t%s\n",palFldvalue[1]);
      }
    }
  }
  printf("        �����˵�       Help    h\n");
  printf("        ������һ��     Quit    q\n");
  fclose(fp);

  swVdebug(4,"S0060: [��������] swShowhelp()������=0");
  return(0);
}
  
/**************************************************************
** �������� swGetsysconcfg
** ��  �ܣ� ��fp��ָ�������ļ���ѯ���뵱ǰ������ƥ���������
** ��  �ߣ� 
** �������ڣ�
** ����޸����ڣ�2001.08.20
** ��������������
** ȫ�ֱ����� alFilename
** �������壺 * alCmd�ǵ�ǰ����,*psCurrentcmd�Ƿ���ֵ
** ����ֵ�� ����0,�ɹ���1
***************************************************************/
     
int swGetsysconcfg(char *alCmd,struct cmdtype *psCurrentcmd)
{
  struct cmdtype slCmdtmp;
  char palFldvalue[12][iFLDVALUELEN];
  FILE *fp;
  unsigned int ilRc,ilLen;
  short ilLen_cmd;
   
  swVdebug(4,"S0070: [��������] swGetsysconcfg()");

  memset((char *)&slCmdtmp,0x00,sizeof(struct cmdtype));

  fp = _swFopen(alFilename,"r");
  if (fp == NULL)
  {
    swVdebug(0,"S0080: [����/ϵͳ����] fopen()����,errno=%d,�����ļ��޷���!",errno);
    return -1;
  }
  _swTrim(alCmd);
  ilLen_cmd = strlen(alCmd); 
  while (*alCmd != '\0')
  {
    *alCmd = tolower(*alCmd);
    alCmd ++ ;
  }
  alCmd = alCmd - ilLen_cmd;

  ilLen = strlen(psCurrentcmd->aCord) + 2;
  for (;;)
  {
    ilRc = _swGetitemmode2(fp,palFldvalue,12);
    if (ilRc) break;

    strcpy(slCmdtmp.aCord,palFldvalue[0]);
    strcpy(slCmdtmp.aMenu,palFldvalue[1]);
    strcpy(slCmdtmp.aBief,palFldvalue[2]);
    strcpy(slCmdtmp.aCmd,palFldvalue[3]);
    strcpy(slCmdtmp.aProc,palFldvalue[4]);

    if (((strcmp(slCmdtmp.aCmd,alCmd) == 0) &&
      (strncmp(slCmdtmp.aCord,psCurrentcmd->aCord,
      strlen(psCurrentcmd->aCord)) == 0) &&
      (strlen(slCmdtmp.aCord) == ilLen)) ||
      ((strlen(alCmd) == 1) &&
      (alCmd[0] == slCmdtmp.aBief[0]) &&
      (strlen(slCmdtmp.aCord) == ilLen) &&
      (strncmp(slCmdtmp.aCord,psCurrentcmd->aCord,
      strlen(psCurrentcmd->aCord)) == 0)))
    {
      memcpy(psCurrentcmd,(char *)&slCmdtmp,sizeof(struct cmdtype));
      fclose(fp);

      swVdebug(4,"S0090: [��������] swGetsysconcfg()������=0");
      return(0);
    }
  }
  fclose(fp);
  return(-1);
} 

int swAnalyse(char * alKeycmd,char palParm[][101])
{
  int i = 0;
  char * alTmp;

  swVdebug(4,"S0100: [��������] swAnalyse()");

  alTmp = strtok(alKeycmd," ");

  /*while ((strcmp(alTmp,"\0" ) != 0) && (i<=10))*/
  while ((alTmp != NULL) && (i<=10))
  {
     strcpy(palParm[i],alTmp);
     alTmp = strtok((char *)0," ");
     i++;
  }
  palParm[i][0] = '\0';

  swVdebug(4,"S0110: [��������] swAnalyse()������=0");
  return(0);
}

/**************************************************************
 ** ������      : Message
 ** ��  ��      : ���ݸ�ʽ��ʾ��Ӧ��Ϣ
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : format   : ��ʽ
 va_alist ��������Ϣ
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
int Message(char *format, ...)
#endif
#ifdef OS_AIX
int Message(format,va_list)
char *format
va_dcl
#endif
#ifdef OS_HPUX
int Message(char *format, ...)
#endif
#ifdef OS_SCO
int Message(char *format, ...)
#endif
#ifdef OS_SOLARIS
int Message(char *format, ...)
#endif
{
  va_list args;

  /* ����Ļ����ʾ�й���Ϣ */
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
    va_start( args, format );
#endif
#ifdef OS_AIX
    va_start(args);
#endif
#ifdef OS_HPUX
    va_start( args, format );
#endif
#ifdef OS_SCO
    va_start( args, format );
#endif
#ifdef OS_SOLARIS
    va_start( args, format );
#endif
  vprintf(format, args);
  va_end(args);
  return(SUCCESS);
}


