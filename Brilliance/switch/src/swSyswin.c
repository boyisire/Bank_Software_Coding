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
/*                         （6） int Message();                 */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/*   2001/08/20 增加参数表控制                                  */

/* 头文件定义 */
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <stdarg.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include "switch.h"
#include "swCurses.h"

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

/* 定义函数名对应表 */
func_t funclist[] = {
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
  {"",NULL}
};
void swQuit(int );

int main(int argc,char *argv[])
{
    short	rc;
    char	tmp[256];
    menu_t	menu;

  /* 打印版本号 */
  if ( argc > 1 )
    _swVersion("Syscon Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swSyscon")) return(FAIL);

  /* 定义调试文件 */
  strncpy(agDebugfile,"swSyscon.debug",sizeof(agDebugfile));

  /* 取 DEBUG 标志 */
  if ((cgDebug = _swDebugflag("swSyscon")) == FAIL)
  {
    fprintf(stderr,"取DEBUG标志出错!\n");
    return(FAIL);
  }

  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
/*  signal(SIGHUP,SIG_IGN);  */
  signal(SIGTSTP,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
  signal(SIGTERM,swQuit);
    /* Initialize */
    memset( &menu, 0x00, sizeof(menu_t));
    memset(agMenupath,0x00,sizeof(agMenupath));
    memset(tmp,0x00,sizeof(tmp));
    
    /* modify by nh 2002111 
    sprintf(agMenupath,"%s/curses",getenv("SWITCH_CFGDIR")); */
    sprintf(agMenupath,"%s/etc/curses",getenv("SWITCH_DIR"));
    sprintf(tmp, "%s/main.mu",agMenupath);
    
    rc = spw_loadmenu( &menu, tmp);
    
    if( rc < 0 ) 
    {
      printf("加载菜单配置文件错误 rc = [%d], %s\n",rc,tmp);
      return(0);
    }
    strcpy(g_frame.status_bar, "  新晨交换平台V4.3.0 |             [ESC]=退出           | 新晨集团南方软件基地  ");
    
    strcpy(tmp," ");
    spw_createframe(&g_frame,tmp);

    menu.cursor = -1;
    rc = spw_exec(&g_frame, &menu);

    spw_closeframe(&g_frame);
    return(0);
}

void swQuit(int sig)
{
  signal(sig,SIG_IGN);
  swVdebug(1,"swSyscon 停止运行........!");
  exit(0);
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
  char alTmp[1024];

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
  memset(alTmp,0x00,sizeof(alTmp));
  vsprintf(alTmp,format, args);
  va_end(args);

  wprintw(g_frame.main,alTmp);
/*  wrefresh(g_frame.main);   */
 
  return(0);
}
