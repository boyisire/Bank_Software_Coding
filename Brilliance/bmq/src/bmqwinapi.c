#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <termio.h>
#include <ctype.h>
#include <curses.h>
#include <time.h>
#include <errno.h>
#include "bmqCurses.h"

char	* _strtrim(char *);
char	* _strtrimr(char *);
char	* _strtriml(char *);
char	* _strtoup(char *);
char	* _datetime(char *); 
extern	func_t	funclist[];


/*****************************************************
# FUNCTION:	spw_initscr
# DESCRIBE:	Create a new window with border line
# RETURN:	WINDOW *
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
void spw_initscr()
{
    initscr();
    cbreak();
    nonl();
    noecho();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);
    refresh();
    return;
}


/*****************************************************
# FUNCTION:	spw_newbox
# DESCRIBE:	Create a new window with border line
# RETURN:	WINDOW *
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
WINDOW * spw_newbox( int high, int width, int wy, int wx )
{
    WINDOW *	win;

    win = newwin( high, width, wy, wx );
    box(win, 0, 0);

    wrefresh(win);
    return(win);
}



/**********************************************************
# FUNCTION:	spw_cursor
# DESCRIBE:	Get the next index of avalidable item
#		of menu
# PARAMETER:	indicate the directinon of moving cursor
#		0  means next
#		1  means previous
#		-1 means current;
# RETURN:	next index, if -1, means on avalidable item
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
**********************************************************/
int spw_cursor( menu_t * menu, int flag )
{
    short	i, cur;

    cur = menu->cursor;
    if(flag == -1) {
	if( (cur < 0) || (cur == menu->items-1) ) cur = 0;
	for(i=cur; i<menu->items; i++) {
	    if( menu->item[i].aflag ) {
		menu->cursor = i;
		return(i);
	    }
	}
	if( cur > 0 ) {
	    for(i=0; i<cur; i++) {
		if( menu->item[i].aflag ) {
		    menu->cursor = i;
		    return(i);
		}
	    }
	}
	return(-1);

    } else if(flag == 0) {
	if( (cur < 0) || (cur == menu->items-1) )
	    cur = 0;
	else
	    cur++;

	for(i=cur; i<menu->items; i++) {
	    if( menu->item[i].aflag ) {
		menu->cursor = i;
		return(i);
	    }
	}
	if( cur > 0 ) {
	    for(i=0; i<cur; i++) {
		if( menu->item[i].aflag ) {
		    menu->cursor = i;
		    return(i);
		}
	    }
	}
	return(-1);

    } else {
	if( (cur <= 0) || (cur > menu->items-1) )
	    cur = menu->items-1;
	else
	    cur--;

	for(i=cur; i>=0; i--) {
	    if( menu->item[i].aflag ) {
		menu->cursor = i;
		return(i);
	    }
	}
	if( cur < menu->items-1 ) {
	    for(i=menu->items-1;i>cur;i--) {
		if( menu->item[i].aflag ) {
		    menu->cursor = i;
		    return(i);
		}
	    }
	}
	return(-1);
    }
}




/*****************************************************
# FUNCTION:	spw_win
# DESCRIBE:	Create a Pop menu window
# RETURN:	index of choose
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
int spw_popwin( int y, int x, menu_t * menu )
{
    int		ch;
    short	i, w, h, idx;
    WINDOW *	win;

    w = menu->width+4;
    h = menu->items+2;
    menu->cursor = 0;		/* begin of select */
    win = spw_newbox( h, w, y, x );

    wstandend(win);
    for(i=0; i<menu->items; i++) {
	menu->item[i].y = i + 1;
	menu->item[i].x = 2;
	if( menu->item[i].aflag )
	    mvwaddstr(win,menu->item[i].y,menu->item[i].x,menu->item[i].name);
	else {
	    wattron(win, A_UNDERLINE);
	    mvwaddstr(win,menu->item[i].y,menu->item[i].x,menu->item[i].name);
	    wstandend(win);
	}
    }
    idx = spw_cursor(menu, -1);
    wstandout(win);
    mvwaddstr(win, menu->item[idx].y, menu->item[idx].x, menu->item[idx].name);
    wrefresh(win); 

    while( 1 ) {
	ch = getch();
	switch(ch) {
	    case CTRKEY_ENTER:
		wstandend(win);
		delwin(win);
		menu->cursor = idx;
		return(idx);
	    case CTRKEY_ESC:
		wstandend(win);
		delwin(win);
		return(-1);
	    case CTRKEY_UP:
		wstandend(win);
		mvwaddstr(win, menu->item[idx].y, menu->item[idx].x, \
		    menu->item[idx].name);
		wrefresh(win);
		wstandout(win);
		idx = spw_cursor(menu, 1);
		mvwaddstr(win, menu->item[idx].y, menu->item[idx].x, \
		    menu->item[idx].name);
		wrefresh(win);
		break;
	    case CTRKEY_DOWN:
		wstandend(win);
		mvwaddstr(win, menu->item[idx].y, menu->item[idx].x, \
		    menu->item[idx].name);
		wrefresh(win);
		wstandout(win);
		idx = spw_cursor(menu, 0);
		mvwaddstr(win, menu->item[idx].y, menu->item[idx].x, \
		    menu->item[idx].name);
		wrefresh(win);
		break;
	    default:
		wstandend(win);
		delwin(win);
		return(-1);
	}
    }

}

/*****************************************************
# FUNCTION:	spw_createframe
# DESCRIBE:	Create a main frame window
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
void spw_createframe( frame_t * frame, char * title )
{
    short	i;

    strcpy(frame->title, title);
    frame->menu_cur = 0;

    spw_initscr();

    frame->head = newwin(LINES_HEAD, COLS, 0, 0);
    wattron(frame->head, A_BOLD);
    i = ( COLS - strlen( frame->title ) ) / 2 -1;
    mvwaddstr(frame->head, 0, i, frame->title);
    wstandend(frame->head);
    wrefresh(frame->head);

    frame->main = newwin(LINES_MAIN, COLS, LINES_HEAD, 0);
    for(i=0;i<COLS;i++) mvwaddch(frame->main, 0, i, '_');  /* modify '_' to 0 */
    for(i=0;i<COLS;i++) mvwaddch(frame->main, LINES_MAIN-1, i, '_');  /* modify '_' to 0 */ 
    wrefresh(frame->main);

    frame->msg = newwin(LINES-LINES_HEAD-LINES_MAIN-1, COLS-2,\
	LINES_HEAD+LINES_MAIN, 1);
    wrefresh(frame->msg);
    scrollok(frame->msg, TRUE);

    wstandout(stdscr);
    wmove(stdscr, LINES-1, 1);
    wprintw(stdscr, "%-80s", frame->status_bar);
    wstandend(stdscr);
    wrefresh(stdscr);

    return;
}

/*****************************************************
# FUNCTION:	spw_rebuildframe
# DESCRIBE:	Create a main frame window
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
void spw_rebuildframe( frame_t * frame )
{
    short	i;
    char	tmp[64];

    spw_initscr();

    frame->head = newwin(LINES_HEAD, COLS, 0, 0);
    wattron(frame->head, A_BOLD);
    i = ( COLS - strlen( frame->title ) ) / 2 -1;
    mvwaddstr(frame->head, 0, i, frame->title);
    wstandend(frame->head);
    wrefresh(frame->head);

    frame->main = newwin(LINES_MAIN, COLS, LINES_HEAD, 0);
    for(i=0;i<COLS;i++) mvwaddch(frame->main, LINES_MAIN-1, i, '_');  /* modify '-' to 0 */
    wmove(frame->main,1,0);
    wrefresh(frame->main); 
    
    frame->msg = newwin(LINES-LINES_HEAD-LINES_MAIN-1, COLS-2,\
	LINES_HEAD+LINES_MAIN, 1);
    wrefresh(frame->msg);
    scrollok(frame->msg, TRUE); 

    
    wstandout(stdscr);
    _datetime(tmp);
    wmove(stdscr, LINES-1, 1);
    wprintw(stdscr, "%-80s", frame->status_bar);
    wstandend(stdscr);
    wrefresh(stdscr);

    return;
}



/*****************************************************
# FUNCTION:	spw_drawframe
# DESCRIBE:	Clear the main frame window
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
void spw_drawframe( frame_t * frame )
{
    short	i;
    char	tmp[64];

    wclear(frame->msg);
    wrefresh(frame->msg);

    wclear(frame->head);
    wattron(frame->head, A_BOLD);
    i = ( COLS - strlen( frame->title ) ) / 2 -1;
    mvwaddstr(frame->head, 0, i, frame->title);
    wstandend(frame->head);
    wrefresh(frame->head);
    

    wclear(frame->main);
    for(i=0;i<COLS;i++) mvwaddch(frame->main, LINES_MAIN-1, i, '_');  /* modify '-' to 0 */
    wmove(frame->main,1,0);    
    wrefresh(frame->main);
    
    wstandout(stdscr);
    _datetime(tmp);
    wmove(stdscr, LINES-1, 1);
    wprintw(stdscr, "%-80s", frame->status_bar);
    wstandend(stdscr);
    wrefresh(stdscr);

    return;
}

/*****************************************************
# FUNCTION:	spw_clearframe
# DESCRIBE:	Clear the main frame window
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
void spw_clearframe( frame_t * frame )
{
    short	i;
    wclear(frame->main);
    wclear(frame->msg);
    for(i=0;i<COLS;i++) mvwaddch(frame->main, 0, i, '_');  /* modify '-' to 0 */
    for(i=0;i<COLS;i++) mvwaddch(frame->main, LINES_MAIN-1, i, '_');  /* modify '-' to 0 */
    wmove(frame->main,1,0);    
    wrefresh(frame->main);
    wrefresh(frame->msg);

    return;
}


/*****************************************************
# FUNCTION:	spw_closeframe
# DESCRIBE:	Close the main frame window
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
void spw_closeframe( frame_t * frame )
{
    delwin(frame->head);
    delwin(frame->main);
    delwin(frame->msg);

    clear();
    refresh();
    endwin();
}



/*****************************************************
# FUNCTION:	spw_menu
# DESCRIBE:	Show a menu, return one choose.
# RETURN:	index of choose.
# AUTHOR:	rime lee
# UPDATE:	2001/01/02
*****************************************************/
int spw_menu( frame_t * frame, menu_t * menu )
{
    int		ch;
    char	str[256];
    short	i, j, cnt, cur, cur_o, dflag, fst, bflag;

    wmove(frame->head,1,0);
    wclrtoeol(frame->head);

    sprintf(str, "[%s]", menu->name);
    i = ( COLS - strlen( str ) ) / 2 -1;
    mvwaddstr(frame->head, 1, i, str);

    cnt = (COLS - 8 - 4 ) / (menu->width + MENU_INTERVAL); /* mod by zjj */
    cur = spw_cursor(menu, -1);
    cur_o = cur;
    dflag = 1;
    bflag = 1;

    while(1) {
	fst = frame->menu_cur;
	if(dflag == 1) {
	    if( cur > fst + cnt - 1 )
		fst = cur - cnt + 1;
	    else if( cur < frame->menu_cur )
		fst = 0;
	} else {
	    if( cur < fst )
		fst = cur;
	    else if( cur >= fst + cnt -1 )
		fst = cur - cnt + 1;
	}
	if( fst != frame->menu_cur ) {
	    frame->menu_cur = fst;
	    fst = 1;
	} else fst = 0;

	if( fst || bflag ) {  
	    wattron(frame->head, A_BOLD);
	    bflag = 0;

	    wmove(frame->head, LINES_HEAD-1, 0);
	    wclrtoeol(frame->head);

	    if(frame->menu_cur == 0)
		mvwaddstr(frame->head, LINES_HEAD-1, 0, " MENU>>   ");
	    else
		mvwaddstr(frame->head, LINES_HEAD-1, 0, " MENU<<   ");

	    if( frame->menu_cur+cnt == menu->items )
		mvwaddstr(frame->head, LINES_HEAD-1, frame->head->_maxx-3,"  ");
	    else
		mvwaddstr(frame->head, LINES_HEAD-1, frame->head->_maxx-3,">>");

	    wstandend(frame->head);
            
	    for( i=0; i<cnt; i++ ) {
		j = i + frame->menu_cur;
		menu->item[j].y = LINES_HEAD-1;
		menu->item[j].x = i * (menu->width + MENU_INTERVAL) + 9;
		if( menu->item[j].aflag && (j != cur) )
		{
		    mvwaddstr(frame->head, menu->item[j].y, menu->item[j].x,\
			menu->item[j].name);
	        }
		else 
		{
		    wattron(frame->head, A_UNDERLINE);
		    mvwaddstr(frame->head, menu->item[j].y, menu->item[j].x,\
			menu->item[j].name);
		    wstandend(frame->head);
		}
	    }
	}
	
/*	mvwaddstr(frame->head, menu->item[cur_o].y, menu->item[cur_o].x, \
	   menu->item[cur_o].name);    */
	if ((((cur_o == 0) && (cur == menu->items - 1)) || ((cur == 0) && (cur_o == menu->items - 1)))&&(menu->items > 2))
	{
        }
	else
	  mvwaddstr(frame->head, menu->item[cur_o].y, menu->item[cur_o].x, \
	    menu->item[cur_o].name);		  
	wstandout(frame->head);
	mvwaddstr(frame->head, menu->item[cur].y, menu->item[cur].x, \
	    menu->item[cur].name); 
	wstandend(frame->head);
	wrefresh(frame->head);


	ch = getch();
	switch(ch) {
	    case CTRKEY_ENTER:
		menu->cursor = cur;
		return(cur);
	    case CTRKEY_ESC:
		return(-1);
	    case CTRKEY_UP:
	    case CTRKEY_LEFT:
		cur_o = cur;
		cur = spw_cursor(menu, 1);
		dflag = 0;
		break;
	    case CTRKEY_DOWN:
	    case CTRKEY_RIGHT:
	    
		cur_o = cur;
		cur = spw_cursor(menu, 0);
		dflag = 1;
		break;
	    default:
		break;
	}
    }
}



/**************************************************************
# FUNCTIOM:	spw_print
# DESCRIBE:	Print one message in message area.
# AUTHOR:	rime lee
# UPDATE:	2001/02/03
***************************************************************/
/*"ifdef" modified by qh 20070416*/
#ifdef _LINUXES_
void spw_print( frame_t * frame, char *msg, ...)
#else
void spw_print(frame,va_list)
frame_t * frame
char *msg
vs_dcl
#endif
{
    va_list	ap;
    char	tmp[1024];

/*"ifdef" modified by qh 20070416*/
#ifdef _LINUXES_
    va_start( ap, msg );
#else
    va_start(ap);
#endif
    vsprintf(tmp, msg, ap);
    va_end(ap);

    wprintw(frame->msg, tmp);
    wrefresh(frame->msg);

    return;
}



/**************************************************************
# FUNCTIOM:	spw_loadmenu
# DESCRIBE:	Load menu data from menu config file
# AUTHOR:	rime lee
# UPDATE:	2001/02/05
***************************************************************/
int spw_loadmenu( menu_t * menu, char * mfn )
{
    short	i, rc, width;
    char	* pa;
    char	itm[256], line[1024];
    FILE	* fp;
    char        alTmp[21];  /* add by zjj */
    
    /* menu name */
    strcpy(menu->name, "name");
    if( (rc = spw_getmenu("attribute", menu->name, mfn)) < 0 ) 
    {
      return(rc);
    }

    /* menu comment */
    strcpy(menu->comment, "comment");
    spw_getmenu("attribute", menu->comment, mfn);

    /* menu items detail */
    i = 0;
    width = 0;
    if( (fp = fopen(mfn, "r")) == NULL ) 
    {
      return(-1);
    }

    while( fgets(line, sizeof(line), fp) != NULL ) 
    {
	line[strlen(line) - 1] = 0x00;
	if( (pa = strchr( line, '#' )) != NULL ) *pa = 0x00;
	_strtrim(line);
	if( strlen(line) == 0 ) continue;
	if( (line[0] != '[') || (line[strlen(line)-1] != ']') ) continue;

	strcpy(itm, line+1);
	itm[strlen(itm)-1] = 0x00;
	strcpy( menu->item[i].name, itm);
	if(width < strlen(itm)) width = strlen(itm);
	_strtoup(itm);
	if(strcmp(itm, "ATTRIBUTE") == 0) continue;

	strcpy( menu->item[i].fun, "function");
	if( (rc = spw_getmenu(itm, menu->item[i].fun, mfn)) < 0 )
	    memset(menu->item[i].fun, 0x00, sizeof(menu->item[i].fun));

	if( (menu->item[i].type = spw_getmenu(itm, "type", mfn)) < 0 )
        {
            fclose(fp);
	    return(menu->item[i].type);
        }

	strcpy(alTmp, "win_type");

	if((rc = spw_getmenu(itm, alTmp, mfn)) < 0 )
	  menu->item[i].win_type = 0;
	else
	  menu->item[i].win_type = atoi(alTmp);

	strcpy( menu->item[i].iocode, "iocode");
	if( (rc = spw_getmenu(itm, menu->item[i].iocode, mfn)) < 0 )
	    memset(menu->item[i].iocode, 0x00, sizeof(menu->item[i].iocode));	  
	if( spw_getmenu(itm, "readonly", mfn) == 1 )
	    menu->item[i].aflag = 0;
	else
	    menu->item[i].aflag = 1;

	i++;
    }
    menu->type = 0;
    menu->items = i;
    menu->width = width;
    fclose(fp);

    return(i);
}



/**************************************************************
# FUNCTIOM:	spw_exec
# DESCRIBE:	Execute actions by configuration.
# Return:	= 0 --- execute successfully
#		!=0 --- undefined menu types, need to handle by user.
# AUTHOR:	rime lee
# UPDATE:	2001/03/20
***************************************************************/
int spw_exec(frame_t * frame, menu_t * pmenu)
{
    short    y, x, rc, cur, type;
    char     fun[MENU_MAXFUN], para[MENU_MAXFUN];
    menu_t   menu;
    int      ilRc;
    char     alValue[10][MENU_MAXFUN];
    int      ilWin_type;   /* add by zjj */  /* 1:popwin */
    /* add by zjj */
    char     alIocode[101];
    char     alFilename[128];
    char     alMenu_name[50];
    
    type = 0;  /* add by zjj 2001.11.26  */
    memset(alIocode,0x00,sizeof(alIocode));
    if(pmenu->cursor >= 0) 
    {
	cur = pmenu->cursor;
	type = pmenu->item[cur].type;
	strcpy( fun, pmenu->item[cur].fun );
	strcpy( para, pmenu->item[cur].para );
	ilWin_type = pmenu->item[cur].win_type;
	strcpy(alIocode,pmenu->item[cur].iocode);
	strcpy(alMenu_name,pmenu->item[cur].name);
    }

    switch(type) 
    {
	case TYPE_POPMENU:			/* POP MENU */
	    spw_clearframe(frame);
	    memset( &menu, 0x00, sizeof(menu_t));
         /* add by zjj 2001.12.17 */
            memset(alFilename,0x00,sizeof(alFilename));
            sprintf(alFilename,"%s/%s",agMenupath,fun);
	    rc = spw_loadmenu( &menu, alFilename);
        /* end add by zjj 2001.12.17  */
/*	    rc = spw_loadmenu( &menu, fun );  */
	    switch(rc) {
		case -1:	
		    spw_print(frame, "load menu file <%s> error",alFilename);
		    break;
		case -2:
		case -3:
		case -4:
		    spw_print(frame, "menu attributes error(%d)",rc);
		    break;
	    }
	    if( rc < 0 ) {
		spw_clearframe(frame);
		return(rc);
	    }
	    menu.type = 1;
	    if(pmenu->type == 1) {
		y = pmenu->item[cur].y;
		x = pmenu->item[cur].x + pmenu->width + 11;
	    } else {
		y = pmenu->item[cur].y+1;
		x = pmenu->item[cur].x;
	    }
	    rc = spw_popwin( y, x, &menu );
	    if(rc < 0) spw_clearframe(frame);
	    if(rc >= 0) rc = spw_exec(frame, &menu);
	    return(rc);
	case TYPE_MENU:			/* MENU */
	    spw_clearframe(frame);

	    /* load menu */
	    if(pmenu->cursor >= 0) {
		/* load cursor menu */
		memset( &menu, 0x00, sizeof(menu_t));
        /* add by zjj 2001.12.17 */
            memset(alFilename,0x00,sizeof(alFilename));
            sprintf(alFilename,"%s/%s",agMenupath,fun);
            rc = spw_loadmenu( &menu, alFilename);
       /* end add by zjj 2001.12.17  */

/*		rc = spw_loadmenu( &menu, fun );   */
		switch(rc) {
		    case -1:	
			spw_print(frame, "load menu file <%s> error",fun);
			break;
		    case -2:
		    case -3:
		    case -4:
			spw_print(frame, "menu attributes error(%d)",rc);
			break;
		}
		if( rc < 0 ) {
		    spw_clearframe(frame);
		    return(rc);
		}
	    } else {
		/* load parents menu itself */
		memcpy( &menu, pmenu, sizeof(menu_t));
	    }

	    /* execute menu */
	    menu.type = 0;
	    for(;;) {
		rc = spw_menu(frame, &menu);
		if(rc < 0) break;
		rc = spw_exec(frame, &menu);
		if(rc == TYPE_EXIT) break;
	    }
	    spw_clearframe(frame);
	    return(rc);
	case TYPE_COMMAND:			/* System Command */
	case TYPE_FUNCTION:
	    spw_redrawmain(&g_frame);
	    wrefresh(g_frame.main);
	    memset(alValue,0x00,sizeof(alValue));
	    if (alIocode[0] != '\0')
	    {
	      memset(alFilename,0x00,sizeof(alFilename));
	      sprintf(alFilename,"%s/input.mu",agMenupath);
	      spw_print(&g_frame,"请输入%s信息... ENTER--提交  ESC--退出 ↑↓--输入项目切换\n",alMenu_name);
	      ilRc = swGetvalue(alFilename,alIocode,ilWin_type,alValue);
	      if (ilRc != 0) 
	      {
	        return(ilRc);
	      }
            }
            if (type == TYPE_FUNCTION)
            {
	      rc = swProc_fun(fun,alValue);
	      return(rc);
	    }
	    else
	    {
	      clear();
	      refresh();
	      resetty();
	      endwin();
	      system("clear");
	      swProc_command(fun,alValue);
/*	      read(0, tmp, 1);  */
              getchar();
	      spw_rebuildframe(frame);  
	      spw_drawframe(frame);  
	      spw_redrawmain(&g_frame); 
	      wrefresh(g_frame.main);
	      break;	      	
	    }
	case TYPE_REFRESH:			/* Refresh */
	    spw_drawframe(frame);
	    break;
	case TYPE_EXIT:				/* Exit */
	    return(TYPE_EXIT);
/*	    
	case TYPE_DEMO:
	    spw_redrawmain(&g_frame);
	    wrefresh(g_frame.main);
	    memset(alValue,0x00,sizeof(alValue));
	    if (alIocode[0] != '\0')
	    {
	      memset(alFilename,0x00,sizeof(alFilename));
	      sprintf(alFilename,"%s/input.mu",agMenupath);
	      spw_print(&g_frame,"请输入%s信息... ENTER--提交  ESC--退出 ↑↓--输入项目切换\n",alMenu_name);
	      ilRc = swGetvalue(alFilename,alIocode,ilWin_type,alValue);
	      if (ilRc != 0) 
	      {
	        return(ilRc);
	      }
            }
	    rc = swProc_fun(fun,alValue);
	    return(rc);  */
	default:
	    return(type);
    }
    return(0);
}

/***********************************************************************
# FUNCTION:	spw_getmenu
# Example:	Example of config file
#		# my config		--- comment
#		[attr_word]		--- attribute word
#		cfg_word0 = YES		--- return 1
#		cfg_word1 = NO		--- return 0
#		cfg_word2 = "hello"	--- "hello" return by cfg
#		cfg_word2 : hello	--- "hello" return by cfg
#
#		Example of C:
#		char word3[256];	
#		wflag = pub("mycfg.ini", "attr_word", "cfg_word0");
#		strcpy(word, "cfg_word2");
#		wflag = pub("mycfg.ini", "attr_word", word);
#
# PARAMETER:	fname --- (passed) configuration filename.
#		attr ---  (passed) attribute word in [ ].
#			  e.g. [TABLE_ATTRIBUTE]
#		cfg  ---  (passed) config word
#			  (return) the value of config if it's a string
# AUTHOR:	rime lee
# UPDATE:	2000/12/20
***********************************************************************/
int spw_getmenu(char * attr, char * cfg, char * fname)
{
    int		locflag;
    char	buffer[1024],tmp[256];
    char	_attr[256],_cfg[256], ch;
    char	*pa;
    FILE	*fp;
    
    memset(buffer,0x00,sizeof(buffer));

    if( (fp=fopen( fname, "r")) == NULL )
    {
      return( -1 );
    }

    strcpy( _attr, attr );
    _strtrim( _attr );
    _strtoup( _attr );

    strcpy( _cfg, cfg );
    _strtrim( _cfg );
    _strtoup( _cfg );

    sprintf( tmp, "[%s]", _attr );

    locflag = 0;
    while( fgets(buffer, sizeof(buffer), fp) != NULL ) 
    {
	buffer[strlen(buffer) - 1] = 0x00;
	if( (pa = strchr( buffer, '#' )) != NULL ) *pa = 0x00;
	_strtrim(buffer);
	if( strlen(buffer) == 0 ) continue;

	if( locflag == 0 ) 
	{
	    if( buffer[0] != '[' ) continue;
	    _strtoup(buffer);
	    if( strcmp( buffer, tmp ) == 0 ) locflag = 1;
	    continue;
	} 
	else 
	{
	    if( buffer[0] == '[' ) 
	    {
	      fclose(fp);
	      return(-3);
	    }
	    if( (pa = strpbrk(buffer, "=:") ) == NULL ) continue;
	    ch = *pa;
	    *pa = 0x00;
	    pa++;
	    _strtrim(buffer);
	    _strtoup(buffer);
	    if( strcmp( buffer, _cfg ) != 0 ) continue;
	    strcpy(tmp, pa);
	    _strtrim(tmp); 
	    if( ch == ':' ) 
	    {
		strcpy( cfg, tmp );
		fclose(fp);
		return(0);
	    } 
	    else if( tmp[0] == '"' ) 
	    {
		if( (pa = strchr(tmp+1, '"')) != NULL ) *pa = 0x00;
		strcpy( cfg, tmp+1 );
		fclose(fp);
		return(0);
	    } 
	    else 
	    {
		if((strcmp(tmp, "YES") == 0) || (strcmp(tmp, "ON") == 0)) 
		{
		    fclose(fp);
		    return(1);
		} 
		else if((strcmp(tmp, "NO") == 0)||(strcmp(tmp, "OFF") == 0))
		{
		    fclose(fp);
		    return(0);
		} 
		else 
		{
		    fclose(fp);
		    return( atoi(tmp) );
		}
	    }
	}
    }
    fclose(fp);
    return( -2 );
}




/**************************************************************
# FUNCTIOM:	spw_obj_edit
# DESCRIBE:	Execute actions by configuration.
# Return:	= 0 --- execute successfully
#		!=0 --- undefined menu types, need to handle by user.
# AUTHOR:	rime lee
# UPDATE:	2001/03/20
***************************************************************/
int
spw_obj_edit(frame_t * frame, int y, int x, char * buf, char * lable, int len)
{
    short	i, ib, ie, j, flag;
    char	tmp[256],str[256];
    int 	ch;

    if(len > sizeof(str)-1) len = sizeof(str) - 1;
    memset(str, 0x00, sizeof(str));
    memset(str, 0x20, len);

    sprintf(tmp, "%s: [%s]", lable, str);
    ib = x + strlen(lable) + 3;
    ie = x + strlen(tmp)-1;
    mvwaddstr(frame->main, y, x, tmp);

    memset(str, 0x00, sizeof(str));
    i = ib;
    wmove(frame->main, y, i);
    wrefresh(frame->main);

    flag = 0;
    j = 0;
    while( (ch = getch()) != CTRKEY_ENTER ) {
	switch(ch) {
	    case CTRKEY_BACK:
		if(i == ib) break;
		i--;
		mvwaddch(frame->main, y, i, ' ');
		wmove(frame->main, y, i);
		wrefresh(frame->main);
		str[--j] = 0x00;
		break;
	    default:
		if(i == ie) {
		    str[j-1] = ch;
		    mvwaddch(frame->main, y, i-1, ch);
		} else {
		    str[j++] = ch;
		    mvwaddch(frame->main, y, i++, ch);
		}
		wmove(frame->main, y, i);
		wrefresh(frame->main);
		break;
	}
    }
    strcpy( buf, str );
    return(i-ib);
}



/**************************************************************
# FUNCTIOM:	spw_obj_pwd
# DESCRIBE:	password input componts
# Return:	
# AUTHOR:	rime lee
# UPDATE:	2001/03/20
***************************************************************/
int
spw_obj_pwd(frame_t * frame, int y, int x, char * buf, char * lable, int len)
{
    short	i, ib, ie, j, flag;
    char	tmp[256],str[256];
    int 	ch;

    if(len > sizeof(str)-1) len = sizeof(str) - 1;
    memset(str, 0x00, sizeof(str));
    memset(str, 0x20, len);

    sprintf(tmp, "%s: [%s]", lable, str);
    ib = x + strlen(lable) + 3;
    ie = x + strlen(tmp)-1;
    mvwaddstr(frame->main, y, x, tmp);

    memset(str, 0x00, sizeof(str));
    i = ib;
    wmove(frame->main, y, i);
    wrefresh(frame->main);

    flag = 0;
    j = 0;
    while( (ch = getch()) != CTRKEY_ENTER ) {
	switch(ch) {
	    case CTRKEY_BACK:
		if(i == ib) break;
		i--;
		mvwaddch(frame->main, y, i, ' ');
		wmove(frame->main, y, i);
		wrefresh(frame->main);
		str[--j] = 0x00;
		break;
	    default:
		if(i == ie) {
		    str[j-1] = ch;
		    mvwaddch(frame->main, y, i-1, '*');
		} else {
		    str[j++] = ch;
		    mvwaddch(frame->main, y, i++, '*');
		}
		wmove(frame->main, y, i);
		wrefresh(frame->main);
		break;
	}
    }
    strcpy( buf, str );
    return(i-ib);
}



/**************************************************************
# FUNCTIOM:	spw_obj_label
# DESCRIBE:	Print one message in message area.
# AUTHOR:	rime lee
# UPDATE:	2001/02/03
***************************************************************/
/*"ifdef" modified by qh 20070416*/
#ifdef _LINUXES_
void spw_obj_label( frame_t * frame, int y, int x, char *msg, ...)
#else
void spw_obj_label( frame, y, x, msg, va_list)
frame_t *frame
int y
int x
char *msg
va_dcl
#endif
{
    char	tmp[1024];
    va_list	ap;

#ifdef _LINUXES_
    va_start( ap, msg );
#else
    va_start(ap);
#endif
    vsprintf(tmp, msg, ap);
    va_end(ap);

    mvwaddstr(frame->main,y,x,tmp);
    wrefresh(frame->main);
    return;
}



/**************************************************************
# FUNCTION:	spw_getfunc
# Describe:	Get the index of function in FIT struct array
# Return:	Index value. if error then return -1.
# Author:	RIME
# Update:	2001/03/23
***************************************************************/
int spw_getfunc(char * funcname)
{
    int i;
    for(i=0; funclist[i].func!=NULL; i++)
        if(memcmp(funcname, funclist[i].name, strlen(funcname))==0) return(i);
    return(-1);
}

/*-----------------------------------------------------------------------*/

/************************************************************************
# Function:	_strtrimr
# Describe:	Delete the space characters in tail of string.
# Auther:	Rime Lee
# Update:	2000/04/08
************************************************************************/
char * _strtrimr(char * pStr)
{
    short	i;
    for(i=strlen(pStr)-1; i>=0; i--) {
	if( (pStr[i] != ' ') && (pStr[i] != '\t') )
	   break;
	else
	   pStr[i]='\0';
    }

    return(pStr);
}



/************************************************************************
# Function:	_strtriml
# Describe:	Delete the space characters in head of string.
# Auther:	Rime Lee
# Update:	2000/04/08
************************************************************************/
char * _strtriml(char * pStr)
{
    short	i;
    char 	* pt;

    pt = pStr;
    while( ( (*pt == ' ') || (*pt == '\t') ) && (*pt != 0x00) ) pt++;

    i = strlen(pt);
    if( (i > 0) && (pt > pStr) ) memmove(pStr,pt,i+1);

    return(pStr);
}



/************************************************************************
# Function:	_strtrim
# Describe:	Delete the space character in the head and tail of string.
# Author:	Rime Lee
# Update:	2000/04/08
************************************************************************/
char * _strtrim(char *pStr)
{
   _strtrimr(pStr);
   _strtriml(pStr);

   return(pStr);
}



/**********************************************
** FUNCTION:	_strtoup
** DESCRIBE:	Exchang string to upper charater.
** CREATE BY:	RIME
** UPDATE:	2000/05/10
**********************************************/
char * _strtoup( char *pBuff )
{
   int i;
   for(i=0; i<strlen(pBuff); i++) pBuff[i] = toupper(pBuff[i]);
   return(pBuff);
}


/**********************************************************
# Function:	datetime
# Describe:	Get the filename from path name.
# Parameter:	fmtstr as example "yyyy-mm-dd www HH:MM:SS"
# Author:	Rime Lee
# Update:	2000/05/10
***********************************************************/
char * _datetime(char *des)
{
   time_t tt;
   struct tm * his;

   tt=time(NULL);
   his=localtime(&tt);

   sprintf(des,"%02d/%02d %02d:%02d",his->tm_mon+1,his->tm_mday,his->tm_hour,\
	his->tm_min);

   return(des);
}



/*==============  add by zjj 2001.11.29 =====================*/
#ifdef _LINUXES_
void spw_printtowin( WINDOW * win, char *msg, ...)
#else
void spw_printtowin( win, msg, va_list)
WINDOW * win
char *msg
va_dcl
#endif
{
    va_list	ap;
    char	tmp[1024];
#ifdef _LINUXES_
    va_start( ap, msg );
#else
    va_start(ap);
#endif
    vsprintf(tmp, msg, ap);
    va_end(ap);

    wprintw(win, tmp);
/*    wrefresh(win);  */
    return;
}

int swEditbuf(char *aFilename,char * aIocode,edit_t *edit_group)
{
  char alParm_label[MENU_MAXITEMS][256];
  FILE *fp;
  int  ilFld,ilRc;
  
  if ((fp = fopen(aFilename,"r")) == NULL)
  {
    return(-1);
   }
   ilFld = 0;  
   while(1)
   {
     ilRc = swReadfld(fp,aIocode,alParm_label);
     if (ilRc != 0)  break;
     strcpy(edit_group->item[ilFld].label,alParm_label[0]);
     edit_group->item[ilFld].len = atoi(alParm_label[1]);
     if (edit_group->maxlabellen < strlen(alParm_label[0])) 
       edit_group->maxlabellen = strlen(alParm_label[0]); 
     if (edit_group->maxvaluelen < atoi(alParm_label[1]))
       edit_group->maxvaluelen = atoi(alParm_label[1]);
       ilFld++;
    }
    edit_group->items = ilFld;
    fclose(fp);
    return(0);
}

int swReadfld(FILE *fp,char *aSegname,char aValue[][256])
{
  char alLine[300],*alTmp,alSegname[256],*alTmp1,*alTmp2;
  static int ilFindflag = 0;
  int ilFld ; 
  aValue[0][0] = '\0';
  
  ilFld = 0;
  while(1)
  {
    if (fgets(alLine,sizeof(alLine),fp) != NULL)
    {
      _swStrtrim(alLine);
      if ((alLine[0] == '#') || (alLine[0] == '\n') || (strlen(alLine) == 0)) continue;
      if ((alTmp = strchr(alLine,'#')) != NULL) *alTmp='\0';
      _swStrtrim(alLine);
      if ((alLine[0] == '[') && (alLine[strlen(alLine) - 1] == ']')) /* 是段 */
      {
        if (ilFindflag == 1) 
        {
          ilFindflag = 0;
          return(-1);	 /* 本段读取完毕 */
        }
        alLine[strlen(alLine) - 1] = '\0';
        strcpy(alSegname,alLine + 1);
        if (strcmp(alSegname,aSegname) == 0)
          ilFindflag = 1;
        else
      	  ilFindflag = 0;
      	continue;
      }
      else if (ilFindflag == 0)  /* 没找到，不是段 */
        continue;
      alTmp = alLine;  
      alTmp1 = alTmp; 
      while (1)
      {
        if ((*alTmp == ' ') || (*alTmp == '\t'))
        {
          alTmp2 = alTmp + 1;
          _swStrtrim(alTmp2);
          *alTmp = '\0';
          alTmp = alTmp2;
          ilFld++;
          _swStrtrim(alTmp1);
          strcpy(aValue[ilFld - 1],alTmp1);
          aValue[ilFld][0] = '\0';
          alTmp1 = alTmp;
         }
         else if ((*alTmp == '\0') || (*alTmp == '\n'))
         {
           ilFld++;
           strcpy(aValue[ilFld - 1],alTmp1);
           break;
         }
         else
           alTmp++;
      }
      if (ilFindflag == 0) return(100);
      return(0);
    }
    else
    {
      ilFindflag = 0;
      return(-1);
    }
  }  
}


int spw_edit_group(WINDOW *arg_win,edit_t *edit_group,int iFlag)
{
  short	j, k,ilTmp = 100,ilSpace_num = 0;
  int start_x,start_y,ilPos_y,ilPos_x_begin,ilPos_x_end,ilPos_x,i;
  char	tmp[256],str[256],alLabel_tmp[256];
  int  ilWin_x,ilWin_y,ilWin_width,ilWin_high;
  int 	ch;
  WINDOW *win;
  
  if (iFlag == 1)  /* popwin */
  {
    start_x = 2;  /* label start pos */
    start_y = 1;
    ilWin_width = edit_group->maxvaluelen + edit_group->maxlabellen + 3 + 2 * start_x + 1;
    ilWin_high = edit_group->items + 2;
    ilWin_x = (COLS - ilWin_width)/2;
    ilWin_y = (LINES - ilWin_high)/2;
    win = newwin(ilWin_high,ilWin_width,ilWin_y,ilWin_x);
    box(win,0,0);
  }
  else 
  {
    start_x = (COLS - edit_group->maxvaluelen - edit_group->maxlabellen - 4) / 2 ;
    start_y = (LINES_MAIN - edit_group->items)/2;
    win = arg_win;
  }
    
  wstandend(win);

  memset(str, 0x00, sizeof(str));
  memset(str, 0x20, edit_group->maxvaluelen); 
  for(k=0;k<edit_group->items;k++)
  {
    ilSpace_num = edit_group->maxlabellen - strlen(edit_group->item[k].label);
    memset(alLabel_tmp,0x00,sizeof(alLabel_tmp));
    memset(alLabel_tmp,0x20,ilSpace_num);
    memcpy(alLabel_tmp + ilSpace_num,edit_group->item[k].label,strlen(edit_group->item[k].label));
    sprintf(tmp, "%s: [%s]", alLabel_tmp, str); 
    mvwaddstr(win,k +  start_y, start_x, tmp);
    wrefresh(win);
  }
  wmove(win, start_y ,start_x + edit_group->maxlabellen  + 3);
  wrefresh(win);

  k = 0;
  j = 0;
  i = start_x + edit_group->maxlabellen + 3;
  memset(str,0x00,sizeof(str));
  
  while(1)
  {
    ch = getch();
    switch(ch) 
    {
    	
      case CTRKEY_BACK:
        wstandend(win);
        ilPos_x_begin = start_x + edit_group->maxlabellen + 3;
        ilPos_y = start_y + k;
	if(i == ilPos_x_begin) break;
	i--;
	mvwaddch(win, ilPos_y, i, ' ');
	wmove(win, ilPos_y, i);
	wrefresh(win);
	str[--j] = 0x00;   
	break;  
      case CTRKEY_ENTER:
        strcpy(edit_group->item[k].value,str);
	ilTmp = 0;
	wstandend(win);
	if (iFlag == 1) delwin(win);  /* 是popwin才释放 */
	break;
      case CTRKEY_ESC:    
	ilTmp = -1;
	wstandend(win);
	if (iFlag == 1) delwin(win);  /* 是popwin才释放 */
	break;
      case CTRKEY_LEFT:
        ilPos_x_begin = start_x + edit_group->maxlabellen + 3;
        ilPos_y = start_y + k;
	if(i == ilPos_x_begin) break;
	i--;
        j--;
        wmove(win,start_y + k,i); 
        wrefresh(win);   
        break;
      case CTRKEY_UP:
	if (k > 0) 
	{
	  strcpy(edit_group->item[k].value,str);
          k = k - 1;
          ilPos_x_begin = start_x + edit_group->maxlabellen + 3;
          ilPos_y = start_y + k; 
          if (strlen(edit_group->item[k].value) > 0)
          {
            ilPos_x = ilPos_x_begin + strlen(edit_group->item[k].value);
            strcpy(str,edit_group->item[k].value);
            j = strlen(edit_group->item[k].value);
            mvwaddstr(win,ilPos_y,ilPos_x_begin, edit_group->item[k].value);
            i = ilPos_x;
          }
          else
          {
            mvwaddch(win,ilPos_y,ilPos_x_begin, ' ');
            wmove(win,ilPos_y,ilPos_x_begin);
            memset(str,0x00,sizeof(str));
            j = 0;
            i = ilPos_x_begin;
          }
	  wrefresh(win);
	}
	else
	  beep();
	break;
      case CTRKEY_RIGHT:
        ilPos_x_begin = start_x + edit_group->maxlabellen + 3;
        ilPos_y = start_y + k;
	if(i == (ilPos_x_begin + edit_group->item[k].len)) break;
	i++;
        j++;
        wmove(win,start_y + k,i); 
        wrefresh(win);   
        break;
      case CTRKEY_DOWN:
	if (k < (edit_group->items - 1))
	{
	  strcpy(edit_group->item[k].value,str);
          k = k + 1;
          ilPos_x_begin = start_x + edit_group->maxlabellen + 3;
          ilPos_y = start_y + k;
          if (strlen(edit_group->item[k].value) > 0)
          {
            ilPos_x = ilPos_x_begin + strlen(edit_group->item[k].value);
            strcpy(str,edit_group->item[k].value);
            j = strlen(edit_group->item[k].value);            
            mvwaddstr(win,ilPos_y,ilPos_x_begin, edit_group->item[k].value);
            i = ilPos_x;
          }
          else
          {
            mvwaddch(win,ilPos_y,ilPos_x_begin, ' ');
            wmove(win,ilPos_y,ilPos_x_begin);
            memset(str,0x00,sizeof(str));
            j = 0;
            i = ilPos_x_begin;
          }
	  wrefresh(win);
	}
	else
	  beep();
	break;
      case '\t':
        beep();
        break;
      default:
        if ((ch < 0x20) || (ch > 0x7E)) continue;
        wstandout(win);
        ilPos_x_end = start_x + edit_group->maxlabellen + 3 + \
          edit_group->item[k].len;
        ilPos_y = start_y + k;
	if(i == ilPos_x_end) 
	{
          str[j-1] = ch;
          mvwaddch(win, ilPos_y, i-1, ch);
        } 
        else 
        {
          str[j++] = ch;
          mvwaddch(win, ilPos_y, i++, ch);
	}
	wmove(win, ilPos_y, i);
	wrefresh(win);
	break;
    }
    if ((ilTmp == 0) || (ilTmp == -1))
    {
/*   if (iFlag == 1)  delwin(win);
      wrefresh(win); */
      return(ilTmp);	
    }
  }
}

int spw_parseparm(char *aBuf,char aParm[][MENU_MAXFUN],int *iCount)
{
  int i,ilPos,ilCount; 
  char alTmp[256];
     
  ilPos = 0;
  ilCount=0;
  while(1)
  {
    if(aBuf[ilPos]==0x00) break;
    for(i=0;;i++)
    {
      if(aBuf[ilPos]==';'||aBuf[ilPos]==0x00) break;   
      alTmp[i] = aBuf[ilPos]; 
      ilPos++;
    }  
    alTmp[i]=0x00;
    if(strlen(alTmp)>0) 
    {
      strcpy(aParm[ilCount],alTmp);
      ilCount++;  
    }
    ilPos++;
  }
  *iCount = ilCount;
  return(0);
}

void spw_redrawhead(frame_t * frame)
{
    short i;
    frame->head = newwin(LINES_HEAD, COLS, 0, 0);  
/*    wclear(frame->head);   */
    wattron(frame->head, A_BOLD);
    i = ( COLS - strlen( frame->title ) ) / 2 -1;
    mvwaddstr(frame->head, 0, i, frame->title);
    wstandend(frame->head);
    wrefresh(frame->head);
	
}

int  spw_redrawmain( frame_t * frame)
{
    short	i;    
 
    frame->main = newwin(LINES_MAIN, COLS, LINES_HEAD, 0);
 /*   wclear(frame->main);   */
    for(i=0;i<COLS;i++) mvwaddch(frame->main, 0, i, '_');  
    for(i=0;i<COLS;i++) mvwaddch(frame->main, LINES_MAIN-1, i, '_');
    wmove(frame->main,1,0);    
/*    wrefresh(frame->main);  */
    return(0);
}

void spw_redrawmsg( frame_t * frame)
{

    frame->msg = newwin(LINES-LINES_HEAD-LINES_MAIN-1, COLS-2,\
	LINES_HEAD+LINES_MAIN, 1);
    wrefresh(frame->msg); 
    scrollok(frame->msg, TRUE);  
/*    wclear(frame->msg);  */
    
    wstandout(stdscr);
    wmove(stdscr, LINES-1, 1);
    wprintw(stdscr, "%-80s", frame->status_bar);
    wstandend(stdscr);
    wrefresh(stdscr);

    return;
}

int swProc_fun(char *aFuncname,char alParm[][256])
{
  int i,ilRc;
  for(i=0; funclist[i].func!=NULL; i++)
  {
    if(memcmp(aFuncname, funclist[i].name, strlen(aFuncname))==0)
    {
    /*
      if ( alParm[0][0] == '\0' )
        ilRc = (* funclist[i].func)(alParm[0]);
      else if ( alParm[1][0] != '\0' )
        ilRc = (* funclist[i].func)(alParm[0],alParm[1]);
      else */
       ilRc = (* funclist[i].func)(alParm);
      return 0;
    }
  } 
  return(-1);
}

int swProc_command(char *aFuncname,char aParm[][256])
{
  char  alCmd[300];

  if (aParm[0][0] == '\0') /* 没有参数 */
    strcpy(alCmd,aFuncname);
  else if (aParm[1][0] != 0 && aParm[2][0] == '\0')  /* 有参数则从参数值从　1 号域开始,0 号固定为段值　*/
  {
    sprintf(alCmd,"%s  %s",aFuncname,aParm[1]);
  }
  else if (aParm[2][0] != '\0' && aParm[3][0] == '\0')
  {
    sprintf(alCmd,"%s  %s  %s",aFuncname,aParm[1],aParm[2]);
  }
  else if (aParm[3][0] != '\0' && aParm[4][0] == '\0')
    sprintf(alCmd,"%s  %s  %s  %s",aFuncname,aParm[1],aParm[2],aParm[3]);
  else
    return(-1); 
    
  system(alCmd);  
  return(0);
}

int swGetvalue(char *aFilename,char *aIocode,int iFlag,char aValue[][256])
{
  edit_t edit_group;
  int ilTmp = 0;
  int ilRc,i;
  
  memset(&edit_group,0x00,sizeof(edit_t));
  ilRc = swEditbuf(aFilename,aIocode,&edit_group); /* 读配置文件,生成edit_group */
  if (ilRc != 0) 
  {
    spw_print(&g_frame,"读取配置文件出错!ilRc=[%d] Iocode = %s\n",ilRc,aIocode);
    spw_print(&g_frame,"filename = %s\n",aFilename);
    return(ilRc);
  }
  if (edit_group.items == 0) return(100);
  ilRc = spw_edit_group(g_frame.main,&edit_group,iFlag); /* 根据edit_group,生成输入窗口 */
  spw_redrawmain(&g_frame); 
  wrefresh(g_frame.main); 
  if (ilRc != 0) 
  {
    memset(&edit_group,0x00,sizeof(edit_t));
    spw_redrawmsg(&g_frame);
    return(ilRc);
  }
  if (edit_group.items > 0)
  {
    strcpy(aValue[0],aIocode);  /* 0号域为段名 */	
    for (i = 0;i<edit_group.items;i++)
    { 
      ilTmp++;
      strcpy(aValue[ilTmp],edit_group.item[i].value); 
    }
    aValue[ilTmp + 1][0] = 0x00;
  }
  return(0);	
}


int swEditoutput(WINDOW *arg_win,edit_t *edit_group,int iFlag)
{
  short	k,ilSpace_num = 0;
  int start_x,start_y;
  char	tmp[256],alLabel_tmp[256];
  int  ilWin_x,ilWin_y,ilWin_width,ilWin_high;
  int 	ch;
  WINDOW *win;
  
  if (iFlag == 1)  /* popwin */
  {
    start_x = 2;  /* label start pos */
    start_y = 1;
    ilWin_width = edit_group->maxvaluelen + edit_group->maxlabellen + 3 + 2 * start_x + 1;
    ilWin_high = edit_group->items + 2;
    ilWin_x = (COLS - ilWin_width)/2;
    ilWin_y = (LINES - ilWin_high)/2;
    win = newwin(ilWin_high,ilWin_width,ilWin_y,ilWin_x);
    box(win,0,0);
  }
  else 
  {
    start_x = (COLS - edit_group->maxvaluelen - edit_group->maxlabellen - 4) / 2 ;
    start_y = (LINES_MAIN - edit_group->items)/2;
    win = arg_win;
  }
  wstandend(win);
  for(k=0;k<edit_group->items;k++)
  {
    ilSpace_num = edit_group->maxlabellen - strlen(edit_group->item[k].label);
    memset(alLabel_tmp,0x00,sizeof(alLabel_tmp));
    memset(alLabel_tmp,0x20,ilSpace_num);
    memcpy(alLabel_tmp + ilSpace_num,edit_group->item[k].label,strlen(edit_group->item[k].label));
    sprintf(tmp, "%s: [%s]", alLabel_tmp, edit_group->item[k].value); 
    mvwaddstr(win,k +  start_y, start_x, tmp);
    wrefresh(win);
  }
  wmove(win, start_y ,start_x + edit_group->maxlabellen  + 3);
  wrefresh(win);
  while(1)
  {
    ch = getch();
    switch(ch) 
    {
      case CTRKEY_ENTER: 
      case CTRKEY_ESC:    
	wstandend(win);
	if (iFlag ==1) delwin(win);  
	return(0);
      default:
        printf("%c",0x07);
    }
  }
}

/**************************************************************
 ** 函数名      : _swStrtrim
 ** 功  能      : 该函数将字符串 s 的前后空格及尾回车去掉
 ** 作  者      : llx
 ** 建立日期    : 1999/11/4
 ** 最后修改日期: 2000/3/10
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : s   :字符串
 ** 返回值      : SUCCESS
		  转换后的字符串从s返回
***************************************************************/
int _swStrtrim(s)
char *s;
{
  short	i, l, r, len;
  for(len=0; s[len]; len++);
  for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
  if(l==len)
  {
    s[0]='\0';
    return(0);
  }
  for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
  for(i=l; i<=r; i++) s[i-l]=s[i];
  s[r-l+1]='\0';
  return(0);
}

