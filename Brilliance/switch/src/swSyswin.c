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
/*                         ��6�� int Message();                 */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*   2001/08/20 ���Ӳ��������                                  */

/* ͷ�ļ����� */
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

/* ���庯������Ӧ�� */
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

  /* ��ӡ�汾�� */
  if ( argc > 1 )
    _swVersion("Syscon Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swSyscon")) return(FAIL);

  /* ��������ļ� */
  strncpy(agDebugfile,"swSyscon.debug",sizeof(agDebugfile));

  /* ȡ DEBUG ��־ */
  if ((cgDebug = _swDebugflag("swSyscon")) == FAIL)
  {
    fprintf(stderr,"ȡDEBUG��־����!\n");
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
      printf("���ز˵������ļ����� rc = [%d], %s\n",rc,tmp);
      return(0);
    }
    strcpy(g_frame.status_bar, "  �³�����ƽ̨V4.3.0 |             [ESC]=�˳�           | �³������Ϸ��������  ");
    
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
  swVdebug(1,"swSyscon ֹͣ����........!");
  exit(0);
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
  char alTmp[1024];

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
  memset(alTmp,0x00,sizeof(alTmp));
  vsprintf(alTmp,format, args);
  va_end(args);

  wprintw(g_frame.main,alTmp);
/*  wrefresh(g_frame.main);   */
 
  return(0);
}
