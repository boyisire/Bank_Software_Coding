/****************************************************************/
/* 模块编号    ：SYSCON                                         */
/* 模块名称    ：任务主控                                       */
/* 版 本 号    ：V4.3.0                                         */
/* 作    者    ：张辉                                           */
/* 建立日期    ：1999/11/22                                     */
/* 最后修改日期：2001/8/20                                      */
/* 模块用途    ：事件管理器                                     */
/* 本模块中包含如下函数及功能说明：                             */
/*                         （1） void main();                   */
/*                         （2） int swProc_fun();              */
/*                         （3） int swShowhelp();              */
/*                         （4） int swGetsysconcfg();          */
/*                         （5） int swAnalyse();               */
/*                         （6） int Message();                 */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/*   2001/08/20 增加参数表控制                                  */

/* 头文件定义 */
#include "switch.h" 

/* add by gxz 2003.03.27 */
int swStop();         /* 停止交换平台                 */
/* add by gxz 2003.03.27 */

int swListtask();     /* 显示任务信息                 */
int swStarttask();    /* 启动任务                     */
int swStoptask();     /* 停止任务                     */
int swListmailbox();  /* 显示邮箱状态                 */
int swClrmailbox();   /* 清空邮箱                     */
int swListport();     /* 查看端口状态                 */
int swConup();        /* 置端口应用层UP               */
int swCondown();      /* 置端口应用层DOWN             */
int swConset();       /* 置端口状态为UP/DOWN          */
int swListtran1();    /* 显示正在处理的交易流C水      */
int swListtran2();    /* 显示当前交易日处理成功的交易 */
int swListtran3();    /* 显示冲正成功的交易           */
int swListtran4();    /* 显示冲正失败的交易           */
int swResaf();        /* RESAF 处理                   */
int swListsaf();      /* 显示 SAF 数据                */
int swClrsaf();       /* 清除 SAF                     */
/* === begin of added by fzj at 2002.03.2 === */
int swListporttran(); /* 显示端口交易数               */
int swResetporttran();/* 重置端口交易数               */
/* === end of added by fzj at 2002.03.2 === */
/* add by gengling at 2015.04.17 begin PSBC_V1.0 */
int swShmtranflowreset();    /* 重置某一笔交易流量控制的交易数 */
int swShmtranflowresetall(); /* 重置所有交易流量控制的交易数   */
/* add by gengling at 2015.04.17 end PSBC_V1.0 */

struct sFunction
{
  char alCmd_ch[100];      /* 函数表识,应该与配置文件中的名称相同      */
  int (*plFunc)();         /* 实际执行的函数名称,即在上面申明过的函数  */
};
/* 定义函数名对应表 */

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
  char aMenu[21];       /*命令菜单栏*/
  char aBief[2];        /*命令简称*/
  char aCmd[21];        /*命令全称*/
  char aProc[41];       /*进程/函数名*/
  char cRbz;            /*起用标志 0-卸载 1－叶函数 2-叶进程 3－过渡函数*/
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

  char  alKeycmd[1001];       /*键盘输入字符串*/
  char  alCmd[51];            /*键盘输入的字符串的命令部分*/
  char  palParm[10][101];     /*键盘输入的字符串的参数部分*/
  char  alCommand[257];
  int   ilRc,i;
  char  alParent[21];
  int   c;

  /* 打印版本号 */
  if ( argc > 1 )
    _swVersion("Syscon Version 4.3.0",argv[1]);

  /* 定义调试文件 */
  strncpy(agDebugfile,"swSyscon.debug",sizeof(agDebugfile));

  /* 取 DEBUG 标志 */
  if ((cgDebug = _swDebugflag("swSyscon")) == FAIL)
  {
    fprintf(stderr,"取DEBUG标志出错!\n");
    return(FAIL);
  }

  /* 以读方式打开命令配置文件 */
  sprintf(alFilename,"config/%s","SWSYSCON.CFG");

  if(argc >= 2)
  {
    if (memcmp(argv[1],"end",3) == 0 )
    {
      ilRc = swStop();
      swVdebug(0,"卸载交换平台进程...");
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
        fprintf(stdout,"Usage: swSyscon [-f 参数表] !\n");
        exit(-1);
    }
  } /* end while */

  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGCLD,SIG_IGN); 
  
  /* 参数初始化 */
  memset(alParent,0x00,sizeof(alParent));
  strcpy(slCurrentcmd.aCord,""); /* 初始化命令码为空 */

  /* 第一次进入syscon 显示主画面 */
  swShowhelp("");
    
  /* 进入主循环 */
  for(;;)
  {
    /*等待键盘输入命令存放在alKeycmd中
    解析键盘输入的字符串alKeycmd
    命令放在palParm[0]中,并将alCmd取小写
    参数放在palParm[]中.参数1放在palParm[1]中,参数2放在parmParm[2]中...
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
   
   /* if ( 键盘输入命令是"q"或"quit" ) */
    if ((strcmp(alCmd,"q") == 0) || (strcmp(alCmd,"quit") == 0))
    {
      /*  处理中层返回上一层的情况,命令代码长度 == 0 ) */
      if ( strlen(slCurrentcmd.aCord) == 0 )
      {
        /* 表示当前命令层次是在最外层,退出系统主控模块;返回unix操作系统;*/
        exit(0);
      }
      /* slCurrentcmd.aCord截去后2位  */
      slCurrentcmd.aCord[strlen(slCurrentcmd.aCord) - 2] = '\0';

      memset(slCurrentcmd.aCmd,0x00,sizeof(slCurrentcmd.aCmd));
      strcpy(alParent,slCurrentcmd.aCmd);

      swShowhelp(slCurrentcmd.aCord);
      continue;
    }

    /* if ( 键盘输入命令是"h"或"help" )  */
    if ((strcmp(alCmd,"h") == 0) || (strcmp(alCmd,"help") == 0) ||
      (strcmp(alCmd,"?") == 0))
    {
      /* 重显当前命令层次下的帮助菜单菜单 */
      swShowhelp(slCurrentcmd.aCord);
      continue;
    }

    ilRc = swGetsysconcfg(alCmd,&slCurrentcmd);
    if (ilRc)
    {
      printf("非法的命令\n");
      continue;
    }  
    if (slCurrentcmd.aProc[0] == '\0' )   /* 过渡函数  */
    {
      swShowhelp(slCurrentcmd.aCord);
      strcpy(alParent,slCurrentcmd.aCmd);
      continue;
    }
    if(slCurrentcmd.aProc[strlen(slCurrentcmd.aProc) - 1] == ')')
    {
      /*叶函数*/
      swProc_fun(slCurrentcmd.aProc,&(palParm[1]));
      slCurrentcmd.aCord[strlen(slCurrentcmd.aCord) - 2] = '\0';
      continue;
    }
    /*叶进程*/
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
** 函数名：swProc_fun
** 功  能： 根据键盘命令cmd,在结构数组slCommand中查找匹配
           alCmd_ch,如找到则执行函数指针plFunc所指的函数
           即运行：command[I].plFunc(alParm)
** 作  者： 李少军
** 建立日期：
** 最后修改日期：
** 调用其它函数：
** 全局变量：
** 参数含义：funame命令对应的函数名.  alParm 命令参数
** 返回值：void
***************************************************************/
int swProc_fun(char *aFunname,char alParm[][101])
{
  int i,ilRc;
  i = 0;

  swVdebug(4,"S0010: [函数调用] swProc_fun()");

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
      swVdebug(4,"S0020: [函数返回] swProc_fun()返回码=0");
      return 0;
    }
    i++;
  } 
  return(-1);
}
  
/**************************************************************
** 函数名：swShowhelp
** 功  能： 轮询fp所指的配置文件中询找与当前命令CMDTYPE相匹
            配的所有子命令行并加以显示如果slCurrentcmd=NULL
            则显示第一层命令菜单
** 作  者： 李少军
** 建立日期：
** 最后修改日期：2001.08.20
** 调用其它函数：
** 全局变量： alFilename
** 参数含义： * slCurrentcmd是当前命令 
** 返回值： 出错－0,成功－1
***************************************************************/
int swShowhelp(char *aCord)
{  
  char palFldvalue[12][iFLDVALUELEN];
  FILE *fp;
  unsigned int ilRc,ilLen;
  
  swVdebug(4,"S0030: [函数调用] swShowhelp()");

  fp = _swFopen(alFilename,"r");
  if (fp == NULL)
  {
    swVdebug(0,"S0040: [错误/系统调用] fopen()函数,errno=%d,配置文件无法打开!",errno);
    Message("配置文件无法打开!\n");
    exit(-1);
  }
  swVdebug(2,"S0050: 配置文件打开 OK!");
  
  printf("\n        [命令功能]    [全称]  [简写] [参数]\n");
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
  printf("        帮助菜单       Help    h\n");
  printf("        返回上一层     Quit    q\n");
  fclose(fp);

  swVdebug(4,"S0060: [函数返回] swShowhelp()返回码=0");
  return(0);
}
  
/**************************************************************
** 函数名： swGetsysconcfg
** 功  能： 在fp所指的配置文件中询找与当前命令相匹配的命令行
** 作  者： 
** 建立日期：
** 最后修改日期：2001.08.20
** 调用其它函数：
** 全局变量： alFilename
** 参数含义： * alCmd是当前命令,*psCurrentcmd是返回值
** 返回值： 出错－0,成功－1
***************************************************************/
     
int swGetsysconcfg(char *alCmd,struct cmdtype *psCurrentcmd)
{
  struct cmdtype slCmdtmp;
  char palFldvalue[12][iFLDVALUELEN];
  FILE *fp;
  unsigned int ilRc,ilLen;
  short ilLen_cmd;
   
  swVdebug(4,"S0070: [函数调用] swGetsysconcfg()");

  memset((char *)&slCmdtmp,0x00,sizeof(struct cmdtype));

  fp = _swFopen(alFilename,"r");
  if (fp == NULL)
  {
    swVdebug(0,"S0080: [错误/系统调用] fopen()函数,errno=%d,配置文件无法打开!",errno);
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

      swVdebug(4,"S0090: [函数返回] swGetsysconcfg()返回码=0");
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

  swVdebug(4,"S0100: [函数调用] swAnalyse()");

  alTmp = strtok(alKeycmd," ");

  /*while ((strcmp(alTmp,"\0" ) != 0) && (i<=10))*/
  while ((alTmp != NULL) && (i<=10))
  {
     strcpy(palParm[i],alTmp);
     alTmp = strtok((char *)0," ");
     i++;
  }
  palParm[i][0] = '\0';

  swVdebug(4,"S0110: [函数返回] swAnalyse()返回码=0");
  return(0);
}

/**************************************************************
 ** 函数名      : Message
 ** 功  能      : 根据格式显示相应信息
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : format   : 格式
 va_alist ：参数信息
 ** 返回值      : SUCCESS/FAIL
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

  /* 在屏幕上显示有关信息 */
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


