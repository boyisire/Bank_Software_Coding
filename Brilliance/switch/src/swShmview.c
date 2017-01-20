#include <stdio.h>
#include <string.h>
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <varargs.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "swapi.h"
#include "swConstant.h"
#include "swNdbstruct.h"

#include "swShm.h"

union semun{
  int val;
  struct semid_ds *buf;
  ushort *array;
};

/* 变量定义 */
int iMaxtranlog,iMaxproclog,iMaxsaflog;
char agCommand[200],agParam[200],agParam1[200],agDebugtest[100];
typedef struct{char aTname[20];int iIndex;} shm_table;
char agCommand_buf[200],agParam_buf[200],agParam1_buf[200]; /*added by fzj at 2002.03.04 */

/* 函数原型 */
void swShmprintlinkinfo(FILE *afStream);
void swShmprintlinkinfo_proc_log(FILE *afStream);
void swShmprintlinkinfo_rev_saf(FILE *afStream);
void swShmprintimf(FILE *afStream);
void swShmprint8583(FILE *afStream);
void swShmprint8583E(FILE *afStream);
void swShmprintcode(FILE *afStream);
void swShmprintother(FILE *afStream);
void swShmprintfmt_m(FILE *afStream);
void swShmprintfmt_g(FILE *afStream);
void swShmprintfmt_d(FILE *afStream);
void swShmprintroute_m(FILE *afStream);
void swShmprintroute_g(FILE *afStream);
void swShmprintqueue(FILE *afStream);
void swShmprintmatchport(FILE *afStream);	/* added by fzj at 2002.02.08 */
void swShmprinttask(FILE *afStream);
void swShmprintconfig(FILE *afStream);
void swShmprintroute_d(FILE *afStream);
void swShmprinttran_log(FILE *afStream);
void swShmprintproc_log(FILE *afStream);
void swShmprintrev_saf(FILE *afStream);
void swShmprintall(FILE *afStream);
void swShmprintstatus_s(FILE *afStream);
void swShmprintstatus_d(FILE* afStream);
void swShmprinthelp();
void swShmgetcmd(char * aaPrompt);
void swShmprintsem(FILE * afStream);
int swShmreset_all();
int swShmreset_tran_log();
int swShmreset_proc_log();
int swShmreset_rev_saf();
int swShmprintport_info(FILE *afStream);
int swShmcheck_g();

int main(int argc, char **argv)
{
  int ilRc;
  short ilCount;
  long wlAbc[500];
  FILE *flDebugfile;
  FILE *flOutput;

  sprintf(agDebugtest,"%s/swShmview.debug",getenv("SWITCH_DEBUG_PATH"));

  if((flDebugfile = fopen(agDebugtest,"a")) == NULL)
  {
    printf("can't open file to write.\n");
    exit(-1);
  }
  flOutput = stdout;
  agCommand_buf[0] = '\0';
  agParam_buf[0] = '\0';
  agParam1_buf[0] = '\0';
  for(;;)
  { 
/*  	 
    if(swShmcheck())
    {
    printf("共享内存初始化swShmcheck失败。\n");
    exit(-1);
    }
*/  /* delete by zjj 2004.02.28 */       
    /*if(swShmcheck_g()) delete by wanghao 20150408*/
    if(swShmcheck())
    {
    printf("共享内存初始化swShmcheck_g失败。\n");
    exit(-1);
    }
    
    swShmgetcmd("swShmview>");
    
    if((strcmp(agCommand,"QUIT") == 0) || (strcmp(agCommand,"Q") == 0))
    {
        fclose(flDebugfile);
        exit(0);
    }
    else
    if((strcmp(agCommand,"HELP") == 0) || (strcmp(agCommand,"?") == 0))
    {
        swShmprinthelp();
    }
    else
    if(strcmp(agCommand,"PT") == 0)
    {
        if((strcmp(agParam,"S") == 0) || (strcmp(agParam,"SCREEN") == 0))
        {
            flOutput = stdout;
            printf("OK!切换输出到屏幕.\n");
        }
        else
        if((strcmp(agParam,"F") == 0) || (strcmp(agParam,"FILE") == 0))
        {
            flOutput = flDebugfile;
            printf("OK!切换输出到文件.\n");
        }
        else
        {  
            printf("无效的参数!\n");
        }
    }
    else
    if(strcmp(agCommand,"STAT") == 0)
    {
      /* fzj:
      i = atoi(agParam);
      if(i > 0)
      {
        fprintf(flOutput,"\n共享内存状态:  %c\n\n",
          sgShmbuf_d.aShmstatus[i-1]);
      }
      */
    }
    else
    if(strcmp(agCommand,"RESET") == 0)
    {
      if(strcmp(agParam,"TRAN") == 0)
        swShmreset_tran_log();
      else
      if(strcmp(agParam,"PROC") == 0)
        swShmreset_proc_log();
      else
      if(strcmp(agParam,"SAF") == 0)
        swShmreset_rev_saf();
      else
        swShmreset_all();
    }
    else
    if((strcmp(agCommand,"PRINT") == 0) || (strcmp(agCommand,"P") == 0))
    {
        if(strcmp(agParam,"ALL") == 0)
        {
            swShmprintall(flOutput);
        }
        else 
        if(strcmp(agParam,"SSHM") == 0)
            swShmprintstatus_s(flOutput);
        else
        if(strcmp(agParam,"DSHM") == 0)
            swShmprintstatus_d(flOutput);
        else
        if(strcmp(agParam,"SWT_SYS_IMF") == 0)
            swShmprintimf(flOutput);
        else 
        if(strcmp(agParam,"SWT_SYS_QUEUE") == 0)
            swShmprintqueue(flOutput);
        else 
/* === begin of added by fzj at 2002.02.08 === */
        if(strcmp(agParam,"SWT_SYS_MATCHPORT") == 0)
            swShmprintmatchport(flOutput);
        else 
/* === end of added by fzj at 2002.02.08 === */
        if(strcmp(agParam,"SWT_SYS_TASK") == 0)
            swShmprinttask(flOutput);
        else 
        if(strcmp(agParam,"SWT_SYS_8583") == 0)
            swShmprint8583(flOutput);
        else 
        if(strcmp(agParam,"SWT_SYS_8583E") == 0)
            swShmprint8583E(flOutput);
        else
        if(strcmp(agParam,"SWT_SYS_CODE") == 0)
            swShmprintcode(flOutput);
        else
        if(strcmp(agParam,"SWT_SYS_CONFIG") == 0)
            swShmprintconfig(flOutput);
        else 
        if(strcmp(agParam,"SWT_SYS_OTHER") == 0)
            swShmprintother(flOutput);
        else 
        if(strcmp(agParam,"SWT_SYS_FMT_M") == 0)
            swShmprintfmt_m(flOutput);
        else 
        /* mod by gengling at 2015.05.21 one line SWT_SYS_FMT_GRP->SWT_SYS_FMT_G PSBC_V1.0 */
        if(strcmp(agParam,"SWT_SYS_FMT_G") == 0)
            swShmprintfmt_g(flOutput);
        else 
        if(strcmp(agParam,"SWT_SYS_FMT_D") == 0)
            swShmprintfmt_d(flOutput);
        else
        if(strcmp(agParam,"SWT_SYS_ROUTE_M") == 0)
            swShmprintroute_m(flOutput);
        else 
        /* mod by gengling at 2015.05.22 one line SWT_SYS_ROUTE_GRP->SWT_SYS_ROUTE_G PSBC_V1.0 */
        if(strcmp(agParam,"SWT_SYS_ROUTE_G") == 0)
            swShmprintroute_g(flOutput);
        else 
        if(strcmp(agParam,"SWT_SYS_ROUTE_D") == 0)
            swShmprintroute_d(flOutput);
        else
        if(strcmp(agParam,"SWT_TRAN_LOG") == 0)
          swShmprinttran_log(flOutput);
        else
        if(strcmp(agParam,"SWT_PROC_LOG") == 0)
          swShmprintproc_log(flOutput);
        else
        if(strcmp(agParam,"SWT_REV_SAF") == 0)
          swShmprintrev_saf(flOutput);
        else 
        if(strcmp(agParam,"SEM") == 0)
          swShmprintsem(flOutput);
        else
        if(strcmp(agParam,"LINK_TRAN_LOG") == 0)
          swShmprintlinkinfo(flOutput); 
        else
        if(strcmp(agParam,"LINK_PROC_LOG") == 0)
          swShmprintlinkinfo_proc_log(flOutput); 
        else
        if(strcmp(agParam,"LINK_REV_SAF") == 0)
          swShmprintlinkinfo_rev_saf(flOutput); 
        else
        if(strcmp(agParam,"SWT_PORT_INFO")==0)
          swShmprintport_info(flOutput);
        /* add by gengling at 2015.05.21 begin PSBC_V1.0 */
        else
        if(strcmp(agParam,"SWT_SYS_TRAN")==0)
          swShmprinttran(flOutput);
        /* add by gengling at 2015.05.21 end PSBC_V1.0 */
        else
          printf("无效参数\n");
      } 
      else
      if(strcmp(agCommand,"ABOUT") == 0)
      {
        /* modify by nh 20020923
        ilRc = swShmselect_fail_rev_saf(wlAbc,&ilCount); */
#if 0 
        ilRc = swDbselect_fail_rev_saf(wlAbc,&ilCount);
        printf("ilRc = %d,ilCount = %d\n",ilRc,ilCount);
#endif
      }
      else  
      {
        if(strcmp(agCommand,"") != 0)
          printf("无效命令%s\n",agCommand);
      }
  } 
}


