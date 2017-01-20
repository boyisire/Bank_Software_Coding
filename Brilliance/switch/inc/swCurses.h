/***************************************************
 SPWindows api include file
 Author: Rime Lee
 Update: 2001/03/20
***************************************************/
#include <curses.h>

#define	MENU_MAXITEMS	15
#define	MENU_MAXWIDTH	40
#define MENU_INTERVAL	2
#define	MENU_MAXNAME	40
#define	MENU_MAXCMT	100
#define	MENU_MAXFUN	256

#define	TYPE_MENU	0
#define	TYPE_POPMENU	1
#define	TYPE_COMMAND	2
#define	TYPE_FUNCTION	3
#define TYPE_PASSWDFUNC 4
#define	TYPE_REFRESH	8
#define	TYPE_EXIT	9
/* #define	TYPE_DEMO       4  */


#define LINES_MAIN	17
#define LINES_HEAD	3

#define CTRKEY_UP	KEY_UP
#define CTRKEY_DOWN	KEY_DOWN
#define CTRKEY_RIGHT	KEY_RIGHT
#define CTRKEY_LEFT	KEY_LEFT
#define CTRKEY_ESC	27
#define CTRKEY_ENTER	13
#define CTRKEY_DEL	127
#define CTRKEY_BACK	263

typedef struct {
    short	y;
    short	x;
    short	aflag;
    char	name[MENU_MAXWIDTH];
    char	fun[MENU_MAXFUN];
    char	para[MENU_MAXFUN];
/*    char        parmdesc[MENU_MAXFUN];  */
/*    char        parmlen[MENU_MAXFUN];   */
    short       win_type;               /*  add by zjj  */
    char        iocode[MENU_MAXCMT];
    short	type;
}menu_item_t;

typedef struct {
    short	y;
    short	x;
    short	maxlen;
    short	len;
    char	label[MENU_MAXNAME];
    char	value[MENU_MAXFUN];
}edit_item_t;

typedef struct {
    short	items;
    short	cursor;
    short       maxvaluelen;   /*  add by zjj  */
    short       maxlabellen;   /*  add by zjj  */
    short	type;			/* 0 - label, 1 - edit */
    edit_item_t	item[MENU_MAXITEMS];
}edit_t;

typedef struct {
    char	name[MENU_MAXFUN];
    int		(* func)();
}func_t;

typedef struct {
    char	name[MENU_MAXNAME];
    char	comment[MENU_MAXCMT];
    short	items;
    short	width;
    short	cursor;
    short	type;			/* 0 - menu, 1 - pop */
    menu_item_t	item[MENU_MAXITEMS];
}menu_t;

typedef struct {
    char 	title[MENU_MAXCMT];
    char	status_bar[MENU_MAXCMT];
    short	menu_cur;
    WINDOW *	head;
    WINDOW *	main;
    WINDOW *	msg;
} frame_t;

extern	WINDOW * spw_newbox( int, int, int, int );
extern	int spw_cursor( menu_t *, int );
extern	int spw_getctrkey();
extern	void spw_createframe( frame_t *, char * );
extern	void spw_drawframe( frame_t * );
extern	void spw_closeframe( frame_t * );
extern	int spw_menu( frame_t *, menu_t * );
extern	int spw_popwin( int, int, menu_t * );
extern	int spw_loadmenu( menu_t *, char * );
extern	int spw_exec( frame_t *, menu_t * );
/*"ifdef" modified by qh 20070416*/
#ifdef OS_AIX
extern  void spw_print( , , va_list);
frame_t *
char *
va_dcl
extern  void spw_printtowin( , , va_list);
WINDOW *
char *
va_dcl
#endif
#ifdef OS_HPUX
extern  void spw_print( frame_t *, char *, ...);
extern  void spw_printtowin( WINDOW *, char *, ...);
#endif
#ifdef OS_LINUX
extern  void spw_print( frame_t *, char *, ...);
extern  void spw_printtowin( WINDOW *, char *, ...);
#endif
#ifdef OS_SCO
extern  void spw_print( frame_t *, char *, ...);
extern  void spw_printtowin( WINDOW *, char *, ...);
#endif
#ifdef OS_SOLARIS
extern  void spw_print( frame_t *, char *, ...);
extern  void spw_printtowin( WINDOW *, char *, ...);
#endif
extern int spw_edit_group(WINDOW *,edit_t *,int);
extern void spw_redrawhead(frame_t * );
extern int  spw_redrawmain( frame_t *);
extern void spw_redrawmsg( frame_t *);
extern  int swEditbuf(char *,char * ,edit_t *);
extern int swReadfld(FILE *fp,char *aSegname,char aValue[][256]);
extern int swGetvalue(char *aFilename,char *aIocode,int iFlag,char aValue[][256]);
extern int swEditoutput(WINDOW *,edit_t *,int);
extern int spw_getmenu(char * , char *, char *);
extern int swProc_fun(char *aFuncname,char alParm[][256]);
extern int swProc_command(char *aFuncname,char aParm[][256]);
extern int _swStrtrim(char *);
frame_t g_frame;
char agMenupath[100];
