#ifndef _SWSYSCOM_H
#define _SWSYSCOM_H  

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>
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
#include <sys/stat.h>
#include <unistd.h>

#define TABLENUM      50
#define FIELDNUM      315
#define MAXFLDNUM     40
#define MAXLINENUM    2048
/******delete by wanghao 20141212
#define iMSGMAXLEN    8192
****************************/

#define iMSGMAXLEN    65536 
#define MAXFLDLEN     512
#define FLDNAMELEN	21

#define MODE1		1
#define MODE2		2
#define	MODE3G	        3
#define	MODE3M	        4
#define MODE3D	        5
#define MODE4	        6
#define MODE5	        7
#define MODE6	        8
#define MODE7	        9
#define MODE8           10
#define MODE9           11
#define MODE10		12
#define MODE11          13
#define MODEMID         14

#define T_SHORT 		1
#define T_INT			2
#define T_LONG			3
#define T_FLOAT			4
#define T_DOUBLE		5
#define	T_PCHAR			6
#define T_CHAR			7

typedef struct
{
	char aAttribName[FLDNAMELEN];
	char aAttribVal[MAXFLDLEN];
}Attrib,*pAttrib;

typedef struct
{
	char aNodeName[FLDNAMELEN];
	char aNodeVal[MAXFLDLEN];
	short iAttrNum;
	short iChildNum;
	pAttrib Attribs;
	char *ChildNodes;
}Node,*pNode;

typedef struct
{
	short iTableID;
	short iFiledID;
	short iFieldType;
	char aFieldName[FLDNAMELEN];
	short iFieldWidth;
	short iFieldAttr;
}Field,*pField;

typedef struct
{
  char keyname[21];
  char keyval[MAXFLDLEN];
}keyrec;

typedef struct
{
	short iTableID;
	short iFieldNum;
	char aTableName[30];
	short iStoreMode;
}Table,*pTable;

typedef struct
{
	char aValue[MAXFLDLEN];
}FieldValue,*pFieldValue;

typedef struct
{
  short iRowid;
  short iHideFlag;
  char aContent[250];
  FieldValue Values[MAXFLDNUM];
}Record,*pRecord;

typedef struct
{
  short iFieldNum;
  short iRecordNum;
  short iStoreMode;
  char aTableName[31];
  char aFileName[100];
  pField Fields;
  pRecord Records;
}Dataset,*pDataset;

static char TypeName[7][10] = 
{
  "i2",
  "i4",
  "i4",
  "r4",
  "r8",
  "string",
  "i2"
};

static Table sgTables[] = 
{
  {1,7,"SWIMF.CFG",MODE1},
  {2,36,"SWAPPS.CFG",MODE2},
  {3,6,"SWMATCH_FMT.CFG",MODE3M},
  {4,1,"SWFORMATD",MODE3D},
  {5,20,"SWTASK.CFG",MODE2},
  {6,1,"SWROUTED",MODE3D},
  {7,3,"SWROUTEGRP",MODE3G},
  {8,5,"SWMATCH_ROUTE.CFG",MODE3M},
  {9,7,"SWISO8583.CFG",MODE1},
  {10,6,"SWFORMATGRP",MODE3G},
  {11,4,"SWCODE.CFG",MODE1},
  {12,9,"SWMSGPACK.CFG",MODE1},
  {13,7,"SWMONTERM.CFG",MODE1},
  {14,22,"SWCONFIG.CFG",MODE2},
  /*** add by zjj ***/
  {15,2,"SWISO8583GRP.CFG",MODE1},
  {16,5,"SWHOSTS.CFG",MODE1},
  {17,5,"SWUSER.CFG",MODE1},
  {18,5,"SWUSRFUN.CFG",MODE1},
  {19,21,"SWCOMM.CFG",MODE1},
  {20,3,"SWCODECHAR.CFG",MODE1},
  {21,3,"SWCODEINT.CFG",MODE1},
  {22,5,"SWBATCH.CFG",MODE1},
  {23,4,"SWMONOBJ.CFG",MODE1},
  {24,5,"SWFRONTGRP",MODE3G},
  {25,1,"SWFRONTD",MODE3D},
  {26,2,"ENVIRON",MODE4}, 
  {27,3,"lockfile",MODE1},
  {28,2,"SWCATALOG_FMT.CFG",MODE5},
  {29,2,"SWCATALOG_ROUTE.CFG",MODE5},
  {30,1,"SWREADLOG",MODE6},
  {31,7,"SWMONTASK",MODE7}, 
  {32,5,"SWMONMB",MODE7}, 
  {33,5,"SWMONAPPS",MODE7}, 
  {34,11,"SWISO8583E.CFG",MODE1},
  {35,2,"SWISO8583EGRP.CFG",MODE1},
  {36,3,"SWMATCH_PORT.CFG",MODE1},
  {37,9,"SWTRANSTATE",MODE8},
  {38,5,"SWTASKSTATE",MODE9},
  {39,1,"SWMACRO.CFG",MODE10},
 /* add by zzl 2002_7_30(为综合交易平台) */
  {40,8,"USERINFO.CFG",MODE1},
  {41,2,"SUBMENU.CFG",MODE1},
  {42,4,"COMMINIT.CFG",MODE1},
  {43,2,"SWRIGH.CFG",MODE1},
  {44,4,"IBSTRANINFO.CFG",MODE1},
  {45,10,"TRANOUT.CFG",MODE2},
  {46,7,"TRANCOM.CFG",MODE1},
  {47,4,"TRANGROUP.CFG",MODE1},
  {48,2,"TRANMAIN.CFG",MODE1},
  {49,10,"TRANIN.CFG",MODE2},
  {50,1,"MIDSCRIPT",MODEMID},
 /* add by zzl 2002_7_30(为综合交易平台) end  */
};

static Field sgFields[] = 
{
/*********  swt_sys_imf ********/
  {1,1,T_PCHAR,"IMF_NAME",30,3},
  {1,2,T_PCHAR,"IMF_TYPE",2,0},
  {1,3,T_SHORT,"IMF_LEN",0,0},
  {1,4,T_SHORT,"IMF_DEC",0,0},
  {1,5,T_PCHAR,"IMF_CHECK",101,4},
  {1,6,T_SHORT,"IMF_ID",0,1},
  {1,7,T_PCHAR,"IMF_CHINA",41,1},
/********* swt_sys_queue ********/ 
  {2,1,T_SHORT,"QID",0,3},
  {2,2,T_SHORT,"PORTID",0,0},
  {2,3,T_PCHAR,"APPNAME",21,1},
  {2,4,T_SHORT,"MSGFORMAT",0,0},
  {2,5,T_PCHAR,"MSGPRIOR",2,0},
  {2,6,T_PCHAR,"COMTYPE",2,0},
  {2,7,T_PCHAR,"COMATTR",101,0}, 
  {2,8,T_SHORT,"OVERTIME",0,0},
  {2,9,T_SHORT,"REVNUM",0,0},
  {2,10,T_PCHAR,"REV_NOGOOD",2,0},
  {2,11,T_PCHAR,"REV_NOM",2,0},
  {2,12,T_PCHAR,"APPRESUEXP",101,4},
  {2,13,T_PCHAR,"REVRESUEXP",101,4},
  {2,14,T_PCHAR,"UNPACKEXP",101,4},
  {2,15,T_PCHAR,"PACKEXP",101,4},
  {2,16,T_PCHAR,"MSGHEADEXP",101,4},
  {2,17,T_PCHAR,"DEFRCD",7,0},
  {2,18,T_PCHAR,"DEFGRP",7,0},
  {2,19,T_PCHAR,"DEFFLD",7,0},
  {2,20,T_SHORT,"MAXTRAN",0,0},
  {2,21,T_PCHAR,"REJEGRP",0,0},
  {2,22,T_SHORT,"ISOTABID",0,0},
  {2,23,T_PCHAR,"BITMAP_TYPE",2,0},
  {2,24,T_PCHAR,"CODETYPE",2,0},
  {2,25,T_PCHAR,"TRANTYPE",129,0},
  {2,26,T_LONG,"TRIGGER_FREQ",0,0},
  {2,27,T_PCHAR,"TRIGGER_TERM",101,4},  
  {2,28,T_PCHAR,"SETDOWN_OVERTIME",101,4},  
  {2,29,T_SHORT,"E8583TAB_ID",0,0},  
  {2,30,T_SHORT,"BITMAP_LEN",101,0},  
  {2,31,T_PCHAR,"BITMAP_UNPK",101,0},  
  {2,32,T_PCHAR,"BITMAP_PACK",101,0},  
  {2,33,T_PCHAR,"MAC_EXPRESS",101,0},  
  {2,34,T_LONG,"TRANBEGIN_MAX",0,0},  
  {2,35,T_LONG,"TRANING_MAX",0,0},  
  {2,36,T_PCHAR,"REV_EXPRESS",101,0}, /* add by zjj 2002.03.06 */

/********* swt_sys_fmt_m *******/
  {3,1,T_SHORT,"FMT_GROUP",0,3},
  {3,2,T_SHORT,"Q_ID",0,3},
  {3,3,T_PCHAR,"TRAN_CODE",11,3},
  {3,4,T_SHORT,"TRAN_STEP",0,3},
  {3,5,T_PCHAR,"TRAN_TYPE",2,3},
  {3,6,T_PCHAR,"DESC",101,3},
/******** swt_sys_fmt_d ********/
  {4,1,T_PCHAR,"VALUE",301,0},
/******** swt_sys_task *********/  
  {5,1,T_PCHAR,"TASKNAME",21,3},
  {5,2,T_PCHAR,"TASKFILE",101,0},
  {5,3,T_SHORT,"QID",0,0},
  {5,4,T_SHORT,"STARTID",0,0},
  {5,5,T_SHORT,"STARTWAIT",0,0},
  {5,6,T_SHORT,"STOPID",0,0},
  {5,7,T_SHORT,"STOPWAIT",0,0},
  {5,8,T_LONG ,"PID",0,0},
  {5,9,T_PCHAR,"TASK_STATUS",2,0},
  {5,10,T_PCHAR,"RESTART_FLAG",2,0},
  {5,11,T_SHORT,"KILLID",0,0},
  {5,12,T_PCHAR,"TASKATTR",2,1},
  {5,13,T_LONG,"STARTTIME",0,0},
  {5,14,T_SHORT,"RESTART_NUM",0,0},
  {5,15,T_SHORT,"RESTART_MAX",0,0},
  {5,16,T_SHORT,"TASKPRIOR",0,0},
  {5,17,T_PCHAR,"TASKUSE",2,0},
  {5,18,T_PCHAR,"TASKTIMER",51,4},
  {5,19,T_PCHAR,"TASKFLAG",2,0},
  {5,20,T_PCHAR,"TASKDESC",101,1},
  /******** swt_sys_route_d ********/
  {6,1,T_PCHAR,"VALUE",301,0},

  /****** swt_sys_route_grp ********/
  {7,1,T_SHORT,"ROUTE_GRP",0,3},
  {7,2,T_PCHAR,"ROUTEDESC",41,1},
 /* 7,3,T_PCHAR,"TRAN_SORT",41,0, */
  {7,3,T_PCHAR,"FILENAME",31,1},
 /******* swt_sys_route_m *********/
  {8,1,T_SHORT,"ROUTE_GRP",0,2},
  {8,2,T_SHORT,"Q_ID",0,2},
  {8,3,T_PCHAR,"TRANCODE",11,2},
  {8,4,T_PCHAR,"DESC",101,1},
  {8,4,T_PCHAR,"FLAG",10,1},
  /****** SWT_SYS_8583 *********/
  {9,1,T_SHORT,"TAB_ID",0,3},
  {9,2,T_SHORT,"FLD_ID",0,3},
  {9,3,T_PCHAR,"FLD_NAME",21,3},
  {9,4,T_SHORT,"FLD_TYPE",0,0},
  {9,5,T_SHORT,"FLD_ATTR",0,0},
  {9,6,T_SHORT,"FLD_LEN",0,0},
  {9,7,T_PCHAR,"FLD_RULE",101,0},
  /******** swt_sys_fmt_grp *******/
  {10,1,T_SHORT,"FMT_GROUP",0,3},
  {10,2,T_PCHAR,"FMTDESC",41,1},
  {10,3,T_SHORT,"TRANTYPE",0,1},
  {10,4,T_PCHAR,"MAC_CREATE",101,4},
  {10,5,T_PCHAR,"MAC_CHECK",101,4},
  {10,6,T_PCHAR,"FILENAME",31,1},
/*  10,6,T_PCHAR,"TRAN_SORT",41,1, */
/****** swt_sys_code ********/
  {11,1,T_LONG,"MSG_CODE",0,3},
  {11,2,T_PCHAR,"MSG_DESC",41,1},
  {11,3,T_SHORT,"MSG_ACTION",0,0},
  {11,4,T_SHORT,"MSG_Q",0,0},
/******* swt_sys_msgpack **********/
  {12,1,T_SHORT,"MSG_TYPE",0,3},
  {12,2,T_SHORT,"FLD_ID",0,3},
  {12,3,T_PCHAR,"FLD_DESC",21,1},
  {12,4,T_PCHAR,"FLD_EXPRESS",51,4},
  {12,5,T_SHORT,"FLD_LEN",0,0},
  {12,6,T_SHORT,"FLD_DISPLAY_LEN",0,0},
  {12,7,T_PCHAR,"FLD_ALIGNMENT",2,0},
  {12,8,T_PCHAR,"STATOKEXP",51,4},
  {12,9,T_PCHAR,"STATERREXP",51,4},
/******* swt_sys_ttytype *********/  
  {13,1,T_SHORT,"TERM_ID",0,3},
  {13,2,T_PCHAR,"TTYNAME",21,1},
  {13,3,T_PCHAR,"TFLAG",2,0},
  {13,4,T_PCHAR,"TERM_DESC",21,1},
  {13,5,T_PCHAR,"MSG_DESC",21,1},
  {13,6,T_PCHAR,"LEXPRESS",101,4},
  {13,7,T_PCHAR,"BANKCODE",21,0},
/******** swt_sys_config **********/
  {14,1,T_SHORT,"LOG_INTER",0,0},
  {14,2,T_SHORT,"SAF_INTER",0,0},
  {14,3,T_INT,"SHMKEY",0,0},
  {14,4,T_SHORT,"SHMWAITTIME",0,0},
  {14,5,T_INT,"MAXTRANLOG",0,0},
  {14,6,T_INT,"MAXPROCLOG",0,0},
  {14,7,T_INT,"MAXSAFLOG",0,0},
  {14,8,T_PCHAR,"FTPMODE",11,0},
  {14,9,T_SHORT,"FTPOVERTIME",0,0},
  {14,10,T_SHORT,"QPACKSIZE",0,0},
  {14,11,T_SHORT,"QOVERTIME",0,0},
  {14,12,T_SHORT,"QRESENDNUM",0,0},
  {14,13,T_PCHAR,"SYSMON_IP",16,0},
  {14,14,T_LONG,"MSG_NUM",0,0},
  {14,15,T_SHORT,"SYSMON_PORT",0,0},
  {14,16,T_SHORT,"SYSCOM_PORT",0,0},
  {14,17,T_SHORT,"ECHOTEST_NUM",0,0},
  {14,18,T_SHORT,"ECHOTEST_INTER",0,0},
  {14,19,T_PCHAR,"TRACE_TERM",31,0},
  {14,20,T_LONG,"MAXTRANNUM",0,0},
  {14,21,T_SHORT,"LOGCLEAR_COUNT",0,0},
  {14,22,T_SHORT,"REREV_INTER",0,0},
  
  /****** add by zjj ******/
  {15,1,T_SHORT,"TAB_ID",0,3},
  {15,2,T_PCHAR,"TAB_DESC",41,1},

  {16,1,T_SHORT,"OTHER_ID",0,3},
  {16,2,T_PCHAR,"OTHER_NAME",21,1},
  {16,3,T_SHORT,"LOCAL_Q",0,0},
  {16,4,T_SHORT,"OTHER_GRP",0,0},
  {16,5,T_SHORT,"OTHER_Q",0,0},

   {17,1,T_PCHAR,"USER_CODE",11,3},
   {17,2,T_PCHAR,"USER_DESC",21,1},
   {17,3,T_PCHAR,"USER_DEPT",21,0},
   {17,4,T_PCHAR,"USER_PSWD",11,0},
   {17,5,T_PCHAR,"USER_RIGHT",51,0},

   {18,1,T_PCHAR,"FUN_NAME",41,3},
   {18,2,T_PCHAR,"FUN_DESC",41,1},
   {18,3,T_SHORT,"PARAM_NUM",0,0},
   {18,4,T_PCHAR,"PARAM_DESC",51,0},
   {18,5,T_PCHAR,"RTN_DESC",41,0},

   {19,1,T_PCHAR,"NAME",31,3},
   {19,2,T_PCHAR,"PROCESS",31,3},
   {19,3,T_SHORT,"COM_QID",0,0},
   {19,4,T_SHORT,"PREP_QID",0,0},
   {19,5,T_SHORT,"RETIME",0,0},
   {19,6,T_SHORT,"REFREQ",0,0},
   {19,7,T_SHORT,"TIMEOUT",0,0},
   {19,8,T_SHORT,"DEBUG_FLAG",0,0},
   {19,9,T_PCHAR,"MSGHEAD_FLAG",2,0},
   {19,10,T_PCHAR,"NEXT_FLAG",2,0},
   {19,11,T_SHORT,"PREREAD_LEN",0,0},
   {19,12,T_PCHAR,"END_FLAG",21,0},
   {19,13,T_PCHAR,"MSGLEN_EXP",101,4},
   {19,14,T_PCHAR,"NEXT_EXPRESS",101,4},
   {19,15,T_PCHAR,"PARAM1",101,0},
   {19,16,T_PCHAR,"PARAM2",101,0},
   {19,17,T_PCHAR,"PARAM3",101,0},
   {19,18,T_PCHAR,"PARAM4",101,0},
   {19,19,T_PCHAR,"PARAM5",101,0},
   {19,20,T_PCHAR,"PARAM6",101,0},
   {19,21,T_PCHAR,"PARAM7",101,0},

   {20,1,T_SHORT,"CODE_TYPE",0,3},
   {20,2,T_SHORT,"CODE_ID",0,3},
   {20,3,T_PCHAR,"CODE_DESC",41,1},

   {21,1,T_SHORT,"CODE_TYPE",0,3},
   {21,2,T_SHORT,"CODE_ID",0,3},
   {21,3,T_PCHAR,"CODE_DESC",41,1},   
 
   {22,1,T_SHORT,"Q_ID",0,3},
   {22,2,T_SHORT,"TRAN_STATUS",0,3},
   {22,3,T_PCHAR,"TRAN_CODE",10,3},
   {22,4,T_SHORT,"SMTRAN_ID",0,3},
   {22,5,T_PCHAR,"SQL_EXPRESS",101,1},
   
   {23,1,T_PCHAR,"OBJ_NAME",21,3},
   {23,2,T_PCHAR,"OBJ_TYPE",2,3},
   {23,3,T_SHORT,"OBJ_Q",0,1},
   {23,4,T_PCHAR,"OBJ_TASK",21,1},
   
/*   {24,1,T_PCHAR,"PPDESC_TMP",41,2}, */
   {24,3,T_SHORT,"PPQID",0,1},
   {24,2,T_PCHAR,"PPDESC",41,1},
 /*  {24,3,T_SHORT,"PPQID",0,1}, */
   {24,4,T_SHORT,"COMQID",0,1},
   {24,5,T_PCHAR,"DEBUG",2,1},
   {24,6,T_PCHAR,"FILENAME",31,1},
   
   {25,1,T_PCHAR,"VALUE",301,0},

   {26,1,T_PCHAR,"ENVNAME",301,1},
   {26,2,T_PCHAR,"ENVVALUE",301,1},
   
   {27,1,T_PCHAR,"FILENAME",31,1},
   {27,2,T_PCHAR,"IPADDRESS",20,1},
   {27,3,T_PCHAR,"LOCKTIME",31,1},

   {28,1,T_PCHAR,"CATALOG",301,3},
   {28,2,T_PCHAR,"FILENAME",31,3},

   {29,1,T_PCHAR,"CATALOG",301,3},
   {29,2,T_PCHAR,"FILENAME",31,3},

   {30,1,T_PCHAR,"VALUE",301,1},

  {31,1,T_PCHAR,"TASKNAME",21,0},
  {31,2,T_LONG ,"PID",0,0},
  {31,3,T_PCHAR,"TASK_STATUS",2,0},
  {31,4,T_PCHAR,"TASKATTR",2,1},
  {31,5,T_LONG,"STARTTIME",0,0},
  {31,6,T_SHORT,"RESTART_NUM",0,0},
  {31,7,T_SHORT,"RESTART_MAX",0,0},

  {32,1,T_SHORT,"MBID",0,0},
  {32,2,T_SHORT,"SENDNUM",0,0},
  {32,3,T_SHORT,"RECVNUM",0,0},
  {32,4,T_SHORT,"PENDNUM",0,0},
  {32,5,T_SHORT,"CONNECT",0,0},
  {33,1,T_SHORT,"QID",0,0},
  {33,2,T_PCHAR,"APPNAME",21,0},
  {33,3,T_PCHAR,"COMTYPE",2,0},
  {33,4,T_PCHAR,"COMATTR",101,0},
  {33,5,T_PCHAR,"STATUS",10,0},
  /****** SWT_SYS_8583E *********/
  {34,1,T_SHORT,"TAB_ID",0,3},
  {34,2,T_SHORT,"FLD_ID",0,3},
  {34,3,T_PCHAR,"FLD_NAME",21,3},
  {34,4,T_SHORT,"FLD_TYPE",0,0},
  {34,5,T_SHORT,"FLD_LEN",0,0},
  {34,6,T_SHORT,"FLD_LEN2",0,0},
  {34,7,T_PCHAR,"FLD_LENUNPK",101,0},
  {34,8,T_PCHAR,"FLD_LENPACK",101,0},
  {34,9,T_PCHAR,"FLD_VALUNPK",101,0},
  {34,10,T_PCHAR,"FLD_VALPACK",101,0},
  {34,11,T_PCHAR,"FLD_RULE",101,0},
  /**** SWT_SYS_8583E_GRP ****/
  {35,1,T_SHORT,"TAB_ID",0,3},
  {35,2,T_PCHAR,"TAB_DESC",41,1},
  /**** SWT_SYS_MATCH_PORT ****/
  {36,1,T_SHORT,"QID",0,0},
  {36,1,T_PCHAR,"TRANCODE",21,0},
  {36,2,T_PCHAR,"PORT",11,0},
  /**** Transaction Information ****/
  {37,1,T_LONG,"MAXTRAN",0,0},
  {37,2,T_LONG,"MAXSAF",0,0},
  {37,3,T_LONG,"TRNING",0,0},
  {37,4,T_LONG,"TRNEND",0,0},
  {37,5,T_LONG,"TRNOVERTIME",0,0},
  {37,6,T_LONG,"TRNREVING",0,0},
  {37,7,T_LONG,"TRNREVEND",0,0},
  {37,8,T_LONG,"SAFING",0,0},
  {37,9,T_LONG,"SAFFAIL",0,0},
  /**** Monitor task state *****/
  {38,1,T_PCHAR,"COMM",21,0},
  {38,2,T_PCHAR,"PID",11,0},
  {38,1,T_PCHAR,"CPU",11,0},
  {38,1,T_PCHAR,"CPUTIME",41,0},
  {38,1,T_PCHAR,"MEMORY",21,0},
  {39,1,T_PCHAR,"VALUE",200,0},
 /* add by zzl 2002_7_30(为综合交易平台) */
  /***    USERINFO.CFG  ***/
  {40,1,T_PCHAR,"BANKCODE",21,0},
  {40,2,T_PCHAR,"BANKDESC",21,0},
  {40,3,T_PCHAR,"BANKLEVEL",1,0},
  {40,4,T_PCHAR,"USERCODE",21,0},
  {40,5,T_PCHAR,"USERDESC",31,0},
  {40,6,T_PCHAR,"USERLEVEL",1,0},
  {40,7,T_PCHAR,"USERPSWD",31,0},
  {40,8,T_PCHAR,"USERRIGHT",31,0},
 /****    SUBMENU.CFG  ****/ 
  {41,1,T_PCHAR,"BANKCODE",21,0},
  {41,2,T_PCHAR,"RIGHT",41,0},
 /***     COMMINIT.CFG  ***/
  {42,1,T_PCHAR,"BANKCODE",21,0},
  {42,2,T_PCHAR,"PORTRANGE",51,0},
  {42,3,T_PCHAR,"TDFRANGE",51,0},
  {42,4,T_PCHAR,"SCRRANGE",51,0},
 /***     SWRIGH.CFG  ***/
  {43,1,T_PCHAR,"RIGHTED",21,0},
  {43,2,T_PCHAR,"RIGHT",21,0},
 /*** IBSTRANINFO.CFG  ***/
  {44,1,T_PCHAR,"TRANCODE",11,0},
  {44,2,T_PCHAR,"STEP",2,0},
  {44,3,T_PCHAR,"DESC",21,0},
  {44,4,T_PCHAR,"SAME",51,0},
 /***  TRANOUT.CFG ***/
  {45,1,T_PCHAR,"TRANNAME",512,3},
  {45,2,T_PCHAR,"BANKCODE",21,0},
  {45,3,T_PCHAR,"FIELDS",51,0},
  {45,4,T_PCHAR,"FIELDSNAME",1024,0},
  {45,5,T_PCHAR,"FIELDSTYPE",21,0},
  {45,6,T_PCHAR,"FIELDSLEN",11,0},
  {45,7,T_PCHAR,"EDITTYPE",21,0},
  {45,8,T_PCHAR,"FIELDVALUE",512,0},
  {45,9,T_PCHAR,"FIELDLAST",21,0},
  {45,10,T_PCHAR,"FIELDVISIBLEDEF",11,0},
  /***  TRANCOM.CFG ***/
  {46,1,T_PCHAR,"BANKCODE",21,0},
  {46,2,T_PCHAR,"COMNAME",512,0},
  {46,3,T_PCHAR,"SQID",21,0},
  {46,4,T_PCHAR,"DQID",21,0},
  {46,5,T_PCHAR,"QGROUP",51,0},
  {46,6,T_PCHAR,"OSTYPE",21,0},
  {46,7,T_PCHAR,"TIMEOUT",11,0},
 /***  TRANGROUP.CFG ***/
  {47,1,T_PCHAR,"BANKCODE",21,0},
  {47,2,T_PCHAR,"GROUPNAME",512,0},
  {47,3,T_PCHAR,"TRANNAME",512,0},
  {47,4,T_PCHAR,"COMNAME",128,0},
 /***  TRANMAIN.CFG ***/
  {48,1,T_PCHAR,"BANKCODE",21,0},
  {48,2,T_PCHAR,"MAINNAME",128,0},
 /***  TRANIN.CFG ***/
  {49,1,T_PCHAR,"TRANNAME",512,3},
  {49,2,T_PCHAR,"BANKCODE",21,0},
  {49,3,T_PCHAR,"FIELDS",128,0},
  {49,4,T_PCHAR,"FIELDSNAME",128,0},
  {49,5,T_PCHAR,"FIELDSTYPE",21,0},
  {49,6,T_PCHAR,"FIELDSLEN",11,0},
  {49,7,T_PCHAR,"EDITTYPE",21,0},
  {49,8,T_PCHAR,"FIELDVALUE",21,0},
  {49,9,T_PCHAR,"FIELDLAST",10,0},
  {49,10,T_PCHAR,"FIELDVISIBLEDEF",2,0},
  
  {50,1,T_PCHAR,"VALUE",301,0},
 /* end add by zzl 2002_7_30(为综合交易平台)  */
};
#define TMPFILE "swfilefun.tmp" 
#define LOCKFILE "lockfile"

static char agErrorMessage[][50] =
{
/*100*/    "100:取端口出错",
/*101*/    "101:取DEBUG标志错",
/*102*/    "102:无命令码",
/*103*/    "103:无表名",
/*104*/    "104:无效命令码",
/*105*/    "105:创建Socket出错",
/*106*/    "106:无效主机名或IP地址",
/*107*/    "107:连接主机（connect调用）出错",
/*108*/    "108:BIND出错",
/*109*/    "109:listen出错",
/*110*/    "110:accept出错",
/*111*/    "111:recv超时",
/*112*/    "112:recv接收长度小于或等于0",
/*113*/    "113:报文长度域不是8字节",
/*114*/    "114:报文长度超过缓冲",
/*115*/    "115:报文中长度域小于8",
/*116*/    "116:报文长度域不正确",
/*117*/    "117:send出错",
/*118*/    "118:",
/*119*/    "119:",
/*120*/    "120:未定义的表名",
/*121*/    "121:记录未找到",
/*122*/    "122:未定义环境变量SWITCH_DIR",
/*123*/    "123:打开配置文件出错",
/*124*/    "124:打开管道出错",
/*125*/    "125:此函数读复合数据",
/*126*/    "126:无法打开复合文件",
/*127*/    "127:无效字段类型",
/*128*/    "128:无效存储模式",
/*129*/    "129:打开临时文件出错",
/*130*/    "130:打开锁文件出错",
/*131*/    "131:记录未找到",
/*132*/    "132:不能加入空记录到锁文件",
/*133*/    "133:文件已被其他人锁定",
/*134*/    "134:文件未被任何人锁定",
/*135*/    "135:组已经存在",
/*136*/    "136:未传送组号",
/*137*/    "137:您没有锁定该文件!",
/*138*/    "138:",
/*139*/    "139:",
/*140*/    "140:无效XML转义",
/*141*/    "141:XML\"<\"错",
/*142*/    "142:XML找不到匹配的\"<\"",
/*143*/    "143:XML无有效的内容",
/*144*/    "144:元素名为空",
/*145*/    "145:非法\"<\"字符",
/*146*/    "146:非法\"/\"字符",
/*147*/    "147:非法字符",
/*148*/    "148:无法解析属性值",
/*149*/    "149:元素名不匹配",
/*150*/    "150:解析错",
/*151*/    "151:找不到指定的节点",
/*152*/    "152:找不到指定的属性",
/*153*/    "153:无效信息类型",
/*154*/    "154:未知错误",
/*155*/    "155:",
"156:",
"157:",
"158:",
"159:",
"160:共享内存刷新出错!",
"161:从共享内存中读取数据出错!",
"162:打开邮箱出错,检查Q是否启动!",
"163:",
"164:打开日志文件出错!",
"165:",
"166:",
"167:",
"168:",
"169:",
"170:",
"171:",
"172:",
"173:",
"174:",
"175:",
"176:",
"177:",
"178:",
"179:",
"180:",
"181:",
"182:",
"183:",
"184:",
"185:",
"186:",
"187:",
"188:",
"189:",
"190:",
"191:",
"192:",
"193:",
"194:",
"195:",
"196:",
"197:",
"198:",
"199:",
"200:",
};

#endif 
