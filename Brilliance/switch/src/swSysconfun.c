#include "switch.h"
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
EXEC SQL INCLUDE sqlca;
#ifdef DB_POSTGRESQL
EXEC SQL INCLUDE swDbstruct.h;
#endif
#ifdef DB_DB2
EXEC SQL INCLUDE '../inc/swDbstruct.h';
#endif
#ifdef DB_INFORMIX
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_ORACLE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_SYBASE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#else
#include "swDbstruct.h"
#endif
#include "swShm.h"

int Message();

/****add by wanghao 20150416 PSBC_V1.0***/
extern int swShmdelete_swt_rev_saf(long lSaf_id);
extern int swShmselect_swt_rev_saf_all(struct swt_rev_saf *psSwt_rev_saf, short *iCount);

/* add by gengling at 2013.04.17 two lines PSBC_V1.0 */
struct shmidx_s *psmShmidx_s;   /* 静态表共享内存目录段指针 */
struct shmbuf_s smShmbuf_s;     /* 静态表共享内存内容段指针 */

#define  TASKNAME       "swTask"

#define  iMSGLEN       80
#define  iMAXTASKCOUNT   100
#define  iMAXRECORD      3000

/**************************************************************
 ** 函数名      : swListtask
 ** 功  能      : 显示任务信息
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/23
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swListtask( )
{
  short ilRc;                      /* 返回码 */
  short ilTotalcount;              /* 总数 */
  short ilPagecount;               /* 本次显示数目 */
  short i;

  /* modify by nh 20020807
  struct swt_sys_task pslSwt_sys_task[iMAXTASKCOUNT];*/
  struct swt_sys_task *pslSwt_sys_task;
  char alState[10];
  char alDisptime[10];             /* 显示任务启动时间 */
  char alTmp[101];

  swVdebug(4,"S0010: [函数调用] swListtask()");

  /* 数据初始化 */
  ilTotalcount=0;

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_all_q(pslSwt_sys_task,&ilTotalcount);*/
  ilRc = swShmselect_swt_sys_task_all_q(&pslSwt_sys_task,&ilTotalcount);
  if (ilRc)
  {
    Message("执行swShmselect_swt_sys_task_all_q出错\n");
    return(FAIL);
  }

  if(ilTotalcount == iMAXTASKCOUNT)
  {
    Message("任务总数超过最大限制数[%d],请调整\n",iMAXTASKCOUNT);
    swVdebug(1,"S0020: [错误/其它] 任务总数超过最大限制,请调整!");
    return(FAIL);
  }

  Message("\n   任务名                       邮箱号  进程号  启动时间  状态   重启次数\n");
  Message(  "   ======                       ======  ======  ========  ====   ========\n");
  ilPagecount = 2 ;
  /* 提取记录 */
  for (i = 0;i<ilTotalcount;i++)
  {
    _swTrim(pslSwt_sys_task[i].task_name); 
    _swTrim(pslSwt_sys_task[i].task_file);

    if(pslSwt_sys_task[i].task_status[0]=='1')
      strcpy(alState, "BEG....");
    else if(pslSwt_sys_task[i].task_status[0]=='2')
      strcpy(alState, "UP");
    else if(pslSwt_sys_task[i].task_status[0]=='3')
      strcpy(alState, "END....");
    else if(pslSwt_sys_task[i].task_status[0]=='4')
      strcpy(alState, "DOWN");
    else if(pslSwt_sys_task[i].task_status[0]=='5')
      strcpy(alState, "ERROR");
    else 
      strcpy(alState, "UNKNOWN");

    swVdebug(3,"S0030: alState=[%s]",alState);
    if(pslSwt_sys_task[i].task_desc[0]!='\0')
    {
      sprintf(alTmp,"%s(%s)",pslSwt_sys_task[i].task_name,
                           pslSwt_sys_task[i].task_desc);
      if(strlen(alTmp)>30)
      {
        memcpy(alTmp+26,"...)",5);
      }
    }
    else
      strcpy(alTmp,pslSwt_sys_task[i].task_name);

    _swTimeLongToChs( pslSwt_sys_task[i].start_time,alDisptime,"HH:MM:SS"); 
    Message("  %-30.30s  %-4d  %-8d%-10.10s%-6.6s %4d|%-4d\n",
      alTmp,
      pslSwt_sys_task[i].q_id,
      pslSwt_sys_task[i].pid,
      alDisptime,
      alState,
      pslSwt_sys_task[i].restart_num,
      pslSwt_sys_task[i].restart_max);
    ilPagecount++;
    if(ilPagecount == 21)
    {
      printf("请按[RETURN]键继续...\n");
      if(getchar() == 'q') break;
      Message("\n   任务名                       邮箱号  进程号  启动时间  状态   重启次数\n");
      Message("   ======                       ======  ======  ========  ====   ========\n");
      ilPagecount = 2 ;
    }
  }

  Message("   总共有[%d]个任务\n",ilTotalcount);

  swVdebug(4,"S0040: [函数返回] swListtask()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swStarttask
 ** 功  能      : 启动任务
 ** 作  者      : 顾晓忠
 ** 建立日期    : 
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern  int swStarttask( char *taskname)
{
  char  alDbtname[21];              /* 任务名称 */
  long  llPid;                      /* 进程号 */
  short ilPidstatus;                /* 进程状态 */
  char  alRunfile[100];             /* 运行程序 */
  char  alProname[21];              /* 运行程序名称 */
  FILE  *plPopenfile;               /* 管道名称 */
  char  alFile1[51],alFile2[51],alFile3[51],alFind[51]; 
  short ilFound;                    /* 发现标志 */
  char  alFindTask[100];
  short ilRc;                       /* 返回码 */
  char  clKey;
 
  struct msgpack slMsgpack;

  swVdebug(4,"S0050: [函数调用] swStarttask(%s)",taskname);

  llPid = -1;
  ilPidstatus = 0;
  
  _swTrim(taskname);

  if ((strcmp(taskname,"\0") == 0)||(strcmp(taskname,TASKNAME)==0)) 
  {
    printf("确认要启动交换平台吗? (Y/N)");
    clKey = getchar();
    if (clKey == 'y' || clKey == 'Y')
      taskname = TASKNAME;
    else 
      return(FAIL); 
  }
  
  memset(alProname,0x00,sizeof(alProname));
  sprintf(alProname, "%s", taskname);

  /* 切换到SWITCH_DIR目录 */
  chdir(getenv("SWITCH_DIR"));

  swVdebug(2,"S0060: 切换到SWITCH_DIR目录");
  swVdebug(2,"S0070: 查看swTask进行是否已经启动");
  /* 查看swTask进行是否已经启动 */
  sprintf(alFindTask,"ps -e|grep %s",TASKNAME);
  ilFound = 0;
  plPopenfile = popen( alFindTask ,"r");
  while ( feof(plPopenfile) == 0) 
  {
    memset(alFile1,0x00,sizeof(alFile1));
    memset(alFile2,0x00,sizeof(alFile2));
    memset(alFile3,0x00,sizeof(alFile3));
    memset(alFind,0x00,sizeof(alFind));

    fscanf(plPopenfile,"%s %s %s %s",alFile1,alFile2,alFile3,alFind);
    if ( strcmp(alFind,TASKNAME) == 0 )
    {
      ilFound = 1;
      break;
    }
  }
  pclose(plPopenfile);

  if(!strcmp(taskname, TASKNAME))
  {
    if ( ilFound == 1 )         /*任务管理进程swTask已存在*/
    {
      Message("任务管理进程[%s]已在系统中运行!  Pid=[%s]\n", alFind, alFile1);
      return FAIL;
    }

    /*任务管理进程swTask不存在则启动swTask*/
    memset(alRunfile,0x00,sizeof(alRunfile));
    sprintf(alRunfile,"%s/bin/%s",getenv("SWITCH_DIR"), taskname);

    /* fork进程,执行swTask进程 */
    if( (llPid=fork()) == 0)
    {
      ilPidstatus = execl(alRunfile,alProname,(char * )0 );
      exit(SUCCESS);
    }
    if( ilPidstatus == 0)
    {
      ilRc = 0;
      Message("启动成功!\n");
    }
    else
    {
      Message("启动失败!\n");
      ilRc = -1;
    }
    return(ilRc);
  }
  /*启动非swTask进程*/
  if ( ilFound==0 )          /*交换平台进程swTask未启*/
  {
    Message("交换平台未启动，请先起交换平台\n");
    return FAIL;
  }

  /*检查该任务在任务管理器中是否存在*/
  strncpy(alDbtname,taskname,sizeof(alDbtname));

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_sys_task(alDbtname,&sgSwt_sys_task);
  if (ilRc)
  {
    Message("所要启动的任务[%s]在任务管理器中不存在\n",alDbtname);
    return(FAIL);
  }
  swVdebug(2,"S0080: 所要启动的任务在任务管理器中存在");

  /* 定位邮箱  */
  if ( qattach( iMBSYSCON ) != 0 )
  {
    swVdebug(1,"S0090: [错误/邮箱] qattach()函数,错误码=-1,定位本地邮箱出错(iMBSYSCON)");
    return(FAIL);
  }
  
  /* 启动指定进程 */
  swVdebug(2,"S0100: 开始启动任务[%s]...",taskname);

  /* 发送命令报文给任务管理器 */
  memset(&slMsgpack, 0x00, sizeof(struct msgpack));
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.lCode = 701;
  slMsgpack.sMsghead.iBodylen = strlen(taskname);
  strcpy(slMsgpack.aMsgbody, taskname);
  ilRc = _qwrite2((char *)&slMsgpack,slMsgpack.sMsghead.iBodylen + 
    sizeof(struct msghead),iMBTASK,0,0,0);

  if (ilRc != SUCCESS) 
  {
    swVdebug(1,"S0110: [错误/邮箱] _qwrite2()函数,错误码=-1,发送报文给任务管理器失败");
    return (FAIL);
  }

  swVdebug(2,"S0120: 发送报文给任务管理器成功");
  swVdebug(2,"S0130: 进程[%s]启动成功!", alProname);
  Message("进程[%s]启动成功!\n", alProname);

  /* 关闭邮箱*/
  qdetach();

  swVdebug(4,"S0140: [函数返回] swStarttask()返回码=0");
  return(SUCCESS);
}


/**************************************************************
 ** 函数名      : swStoptask
 ** 功  能      : 停止任务
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern  int  swStoptask( char *taskname )
{
  FILE *plPopenfile;                /* 管道名称 */
  char  alFile1[51],alFile2[51],alFile3[51],alFind[51]; 

  short ilFound;                    /* 发现标志 */
  char  alFindTask[100];

  short ilRc;                       /* 返回码 */
  char  clKey;

  struct msgpack slMsgpack;

  swVdebug(4,"S0150: [函数调用] swStoptask(%s)",taskname);

  _swTrim(taskname);

  if ((strcmp(taskname,"\0") == 0)||(strcmp(taskname,TASKNAME)==0))
  {
    printf("确认要退出交换平台吗? (Y/N)");
    clKey = getchar();
    if (clKey == 'y' || clKey == 'Y')
      taskname = TASKNAME;
    else
      return(FAIL);
  }
  /* 检查任务是否存在 */
  ilFound = 0;
  sprintf(alFindTask,"ps -e|grep %s",TASKNAME);
  plPopenfile = popen( alFindTask ,"r");

  while ( feof(plPopenfile) == 0 )
  {
    memset(alFile1,0x00,sizeof(alFile1));
    memset(alFile2,0x00,sizeof(alFile2));
    memset(alFile3,0x00,sizeof(alFile3));
    memset(alFind,0x00,sizeof(alFind));

    fscanf(plPopenfile,"%s %s %s %s",alFile1,alFile2,alFile3,alFind);
    if ( strcmp(alFind,TASKNAME) == 0 )
    {
      ilFound=1;
      break;
    }
  }
  pclose(plPopenfile);

  if ( ilFound ==0 )
  {
    Message("交换平台[%s]尚未运行! \n", TASKNAME);
    swVdebug(0,"S0160: [其它] 交换平台[%s]尚未运行! ", TASKNAME);
    return FAIL;
  }
  if ( strcmp( taskname,TASKNAME ) == 0 )
  {
    kill(atoi(alFile1),SIGTERM);
    Message("进程[%s]退出系统!  Pid=[%s]\n", alFind, alFile1);
    swVdebug(0,"进程[%s]退出系统!  Pid=[%s]\n", alFind, alFile1);
    return SUCCESS;
  }
  
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_sys_task(taskname,&sgSwt_sys_task);

  if (ilRc)
  {
    Message("所要停止的任务[%s]在任务管理器中不存在\n",taskname);
    return(FAIL);
  }
  
  if ( sgSwt_sys_task.pid <=0 ) 
  {
    Message("进程[%s]尚未运行! \n", taskname);
    swVdebug(0,"S0170: [其它] 进程[%s]尚未运行! ", taskname);
    return FAIL;
  }

  swVdebug(2,"S0180: 开始停止任务[%s]...",taskname);

  /* 定位邮箱  */
  if (qattach( iMBSYSCON ) != 0)
  {
    swVdebug(0,"S0190: [错误/邮箱] qattach()函数,错误码=-1,定位本地邮箱出错(iMBSYSCON)");
    return(FAIL);
  }

  /* 发送命令报文给任务管理器 */
  memset(&slMsgpack, 0x00, sizeof(struct msgpack));
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.lCode = 702;
  slMsgpack.sMsghead.iBodylen = strlen(taskname);
  strcpy(slMsgpack.aMsgbody, taskname);

  ilRc = _qwrite2((char *)&slMsgpack,slMsgpack.sMsghead.iBodylen + 
      sizeof(struct msghead),iMBTASK,0,0,0);
  if (ilRc != SUCCESS) 
  {
    swVdebug(1,"S0200: [错误/邮箱] _qwrite2()函数,错误码=%d,发送报文给任务管理器失败",ilRc);
    swMberror(ilRc,NULL);
    return (FAIL);
  }
  swVdebug(2,"S0210: 发送报文给任务管理器成功");
  Message("进程[%s]关闭成功!\n",taskname);
  swVdebug(2,"S0220: 进程[%s]关闭成功!", taskname);

  /* 关闭邮箱*/
  qdetach();

  swVdebug(4,"S0230: [函数返回] swStoptask()返回码=0");
  return(SUCCESS);
}


/**************************************************************
 ** 函数名      : swStop
 ** 功  能      : 停止任务
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern  int  swStop()
{
  FILE *plPopenfile;                /* 管道名称 */
  char  alFile1[51],alFile2[51],alFile3[51],alFind[51]; 

  short ilFound;                    /* 发现标志 */
  char  alFindTask[100];


  ilFound = 0;
  sprintf(alFindTask,"ps -e|grep %s",TASKNAME);
  plPopenfile = popen( alFindTask ,"r");

  while ( feof(plPopenfile) == 0 )
  {
    memset(alFile1,0x00,sizeof(alFile1));
    memset(alFile2,0x00,sizeof(alFile2));
    memset(alFile3,0x00,sizeof(alFile3));
    memset(alFind,0x00,sizeof(alFind));

    fscanf(plPopenfile,"%s %s %s %s",alFile1,alFile2,alFile3,alFind);
    if ( strcmp(alFind,TASKNAME) == 0 )
    {
      ilFound=1;
      break;
    }
  }
  pclose(plPopenfile);

  if ( ilFound ==0 )
  {
    swVdebug(0,"S0160: [其它] 交换平台[%s]尚未运行! ", TASKNAME);
    return FAIL;
  }
  kill(atoi(alFile1),SIGTERM);
  swVdebug(1,"进程[%s]退出系统!  Pid=[%s]\n", alFind, alFile1);
  return SUCCESS;
}


/**************************************************************
 ** 函数名      : swConup
 ** 功  能      : 置端口应用层状态为UP
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/27
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swConup(char *alPortid)
{
  short ilPid;
  short ilRc;       /* 返回码 */

  swVdebug(4,"S0240: [函数调用] swConup(%s)",alPortid);

  _swTrim(alPortid);
  if (strcmp(alPortid,"\0") == 0)
  {
    Message("Usage: U   portid\n");
    return(FAIL);
  }
  ilPid = atoi(alPortid);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_sys_queue(ilPid,&sgSwt_sys_queue);

  if(ilRc == SHMNOTFOUND)
  {
    Message("没有端口[%d]的记录\n",ilPid);
    return(FAIL);
  }
  if(ilRc)
  {
    Message("选择端口表出错\n");
    swVdebug(1,"S0250: [错误/共享内存] 选择端口表出错");
    return(FAIL);
  }
  /*置端口应用层状态为UP*/
  ilRc =  swPortset( ilPid, 2, 1 );
  if ( ilRc )
  {
    Message("强置端口[%d]状态出错\n",ilPid);
    swVdebug(1,"S0260: [错误/其它] 强置端口[%d]状态出错",ilPid);
    return(FAIL);
  }
  
  swVdebug(2,"S0270: 更新[%d]端口为UP",ilPid);

  swVdebug(4,"S0280: [函数返回] swConup()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swCondown
 ** 功  能      : 置端口应用层状态为DOWN
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/27
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swCondown(char *alPid)
{
  short ilRc;       /* 返回码 */
  short ilPid;

  swVdebug(4,"S0290: [函数调用] swCondown(%s)",alPid);

  _swTrim(alPid);
  if (strcmp(alPid,"\0") == 0)
  {
    Message("Usage: D    portid\n");
    return(FAIL);
  }

  ilPid = atoi(alPid);
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }
  ilRc = swShmselect_swt_sys_queue(ilPid,&sgSwt_sys_queue);

  if(ilRc == SHMNOTFOUND)
  {
    Message("没有端口[%d]的记录\n",ilPid);
    return(FAIL);
  }
  if(ilRc)
  {
    Message("选择端口表出错\n");
    swVdebug(1,"S0300: [错误/共享内存] 选择端口表出错");
    return(FAIL);
  }

  ilRc =  swPortset( ilPid, 2, 0 );
  if ( ilRc )
  {
    Message("强置端口[%d]状态出错\n",ilPid);
    swVdebug(1,"S0310: [错误/其它] 强置端口[%d]状态出错",ilPid);
    return(FAIL);
  }

  swVdebug(2,"S0320: 更新[%d]端口为DOWN",ilPid);

  swVdebug(4,"S0330: [函数返回] swCondown()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swConset
 ** 功  能      : 置端口状态为UP/DOWN
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/27
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swConset(char *alPid,char *alValue)
{
  short ilRc;       /* 返回码 */
  short ilPid;
  int   ilNum;
  int   i;
  
  swVdebug(4,"S0340: [函数调用] swConset(%s,%s)",alPid,alValue);

  _swTrim(alPid);
  if ((strcmp(alPid,"\0") == 0)&&(strcmp(alValue,"\0") == 0))
  {
    Message("Usage: T  portid  value\n");
    return(FAIL);
  }

  /* 判断输入的状态值是否为正确数值 */ 
  ilNum = strlen(alValue);
  for(i=0;i<ilNum;i++) 
  { 
    if ( (alValue[i]!='0')&&(alValue[i]!='1') )
    {
      Message("输入的状态值非法\n");
      return(FAIL);
    }
  } 
     
  ilPid = atoi(alPid);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }
  ilRc = swShmselect_swt_sys_queue(ilPid,&sgSwt_sys_queue);

  if(ilRc == SHMNOTFOUND)
  {
    Message("没有端口[%d]的记录\n",ilPid);
    return(FAIL);
  }
  if(ilRc)
  {
    Message("选择端口表出错\n");
    swVdebug(1,"S0350: [错误/共享内存] 选择端口表出错");
    return(FAIL);
  }
  
  for (i=0;i<ilNum;i++)
  {
    if ( alValue[i] == '0' )
      ilRc =  swPortset( ilPid, i+1, 0 );
    else
      ilRc =  swPortset( ilPid, i+1, 1 );
    if ( ilRc )
    {
      Message("强置端口[%d]状态出错\n",ilPid);
      swVdebug(1,"S0360: [错误/其它] 强置端口[%d]状态出错",ilPid);
      return(FAIL);
    }
  }

  swVdebug(2,"S0370: 更新[%d]端口为DOWN",ilPid);

  swVdebug(4,"S0380: [函数返回] swConset()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swListtran1
 ** 功  能      : 显示正在处理的交易流水
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swListtran1()
{
  short ilRc;       /* 返回码 */
  char lsTmp1[10],lsTmp2[10],lsTmp3[10];
  char clKey;

  short ilTotalcount;              /* 总数 */
  short ilPagecount;               /* 本次显示数目 */
  short i;
  struct swt_tran_log pslSwt_tran_log[iMAXRECORD];

  swVdebug(4,"S0390: [函数调用] swListtran1()");

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_tran_log_all(pslSwt_tran_log,&ilTotalcount);
  if(ilRc == FAIL)
  {
    Message("执行swShmselect_swt_tran_log_all失败\n");
    swVdebug(1,"S0400: [错误/共享内存] 执行swShmselect_swt_tran_log_all失败");
    return(FAIL);
  }

  /* 显示 */
  Message("\n                  交易开   交易结   交易超    发起     发起交\n");
  Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  for(i=0;i<ilTotalcount;i++)
  {
    _swTimeLongToChs(pslSwt_tran_log[i].tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(pslSwt_tran_log[i].tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(pslSwt_tran_log[i].tran_overtime,lsTmp3,"HH:MM:SS");

    Message( "   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      pslSwt_tran_log[i].tran_id,pslSwt_tran_log[i].tran_status,\
      lsTmp1,lsTmp2,lsTmp3,pslSwt_tran_log[i].q_tran_begin,\
      pslSwt_tran_log[i].tc_tran_begin );
  
    ilPagecount++;
    if(ilPagecount == 20) 
    {
      printf("请按[RETURN]键继续...\n");
      clKey = getchar();
      if (clKey == 'q' || clKey == 'Q')  break;
      Message("\n                  交易开   交易结   交易超    发起     发起交\n");
      Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
      Message("   ============================================================\n");
      ilPagecount = 2;
    }
  }

  Message("   共有[%d]条记录\n",ilTotalcount);

  swVdebug(4,"S0410: [函数返回] swListtran1()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swListtran2
 ** 功  能      : 查询当前交易日处理成功的交易 
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swListtran2()
{ 
  short ilRn;       /* 返回码 */


  swVdebug(4,"S0420: [函数调用] swListtran2()");

  /* 打开数据库 */
  ilRn=swDbopen();
  if (ilRn) 
  {
    Message("打开数据库失败,sqlca.sqlcode=[%d]\n",ilRn);
    swVdebug(0,"S0430: [错误/数据库] 打开数据库失败,sqlca.sqlcode=[%d]",ilRn);
    return(FAIL); 
  }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL DECLARE tran_log_2 CURSOR FOR SELECT  *
    FROM swt_his_tran_log WHERE tran_status = 1 ORDER BY tran_id;
  if(sqlca.sqlcode)
  {
    Message("定义游标失败,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0440: [错误/数据库] 定义游标失败,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  EXEC SQL OPEN tran_log_2;
  if(sqlca.sqlcode)
  {
    Message("打开游标失败,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0450: [错误/数据库] 打开游标失败,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  ilTotalcount = 0;

  Message("\n                  交易开   交易结   交易超    发起     发起交\n");
  Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  while(TRUE)
  {
    memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
    /* EXEC SQL FETCH tran_log_2 INTO :sgSwt_tran_log; delete by szhengye 2002.3.12 */
    EXEC SQL FETCH tran_log_2 INTO 
           :sgSwt_tran_log.tran_id,
           :sgSwt_tran_log.tran_begin,
           :sgSwt_tran_log.tran_status,
           :sgSwt_tran_log.tran_end,
           :sgSwt_tran_log.tran_overtime,
           :sgSwt_tran_log.q_tran_begin,
           :sgSwt_tran_log.tc_tran_begin,
           :sgSwt_tran_log.q_target,
           :sgSwt_tran_log.resq_file,
           :sgSwt_tran_log.resq_offset,
           :sgSwt_tran_log.resq_len, 
           :sgSwt_tran_log.resp_file,
           :sgSwt_tran_log.resp_offset,
           :sgSwt_tran_log.resp_len,
           :sgSwt_tran_log.rev_key;	/* add by szhengye 2002.3.12 */    

    if (sqlca.sqlcode) break;

    _swTimeLongToChs(sgSwt_tran_log.tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_overtime,lsTmp3,"HH:MM:SS");
   
    Message("   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status,lsTmp1,lsTmp2,\
      lsTmp3,sgSwt_tran_log.q_tran_begin, sgSwt_tran_log.tc_tran_begin);

    ilTotalcount++;
    ilPagecount++;
    if(ilPagecount == 20)
    {
      printf("请按[RETURN]键继续...\n");
      clKey = getchar();
      if(clKey == 'q' || clKey == 'Q')
        break;
      Message("\n                  交易开   交易结   交易超    发起     发起交\n");
      Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
      Message("   ============================================================\n");
      ilPagecount=2;
    }
  }
  EXEC SQL CLOSE tran_log_2;
  swDbclose();

  Message("   共有[%d]条记录\n",ilTotalcount);
#endif

  swVdebug(4,"S0460: [函数返回] swListtran2()返回码=0");
  return(SUCCESS); 
}

/**************************************************************
 ** 函数名      : swListtran3
 ** 功  能      : 查询冲正成功的交易 
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swListtran3()
{ 
  short ilRn;       /* 返回码 */


  swVdebug(4,"S0470: [函数调用] swListtran3()");

  /* 打开数据库 */
  ilRn=swDbopen();
  if (ilRn) 
  {
    Message("打开数据库失败,sqlca.sqlcode=[%d]\n",ilRn);
    swVdebug(1,"S0480: [错误/数据库] 打开数据库失败,sqlca.sqlcode=[%d]",ilRn);
    return(FAIL); 
  }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL DECLARE tran_log_3 CURSOR FOR SELECT  *
    FROM swt_his_tran_log WHERE tran_status = 4 ORDER BY tran_id;
  if(sqlca.sqlcode)
  {
    Message("定义游标失败,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0490: [错误/数据库] 定义游标失败,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  EXEC SQL OPEN tran_log_3;
  if(sqlca.sqlcode)
  {
    Message("打开游标失败,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0500: [错误/数据库] 打开游标失败,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  ilTotalcount = 0;

  Message("\n                  交易开   交易结   交易超    发起     发起交\n");
  Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  while(TRUE)
  {
    memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
    /* EXEC SQL FETCH tran_log_3 INTO :sgSwt_tran_log; delete by szhengye 2002.3.12 */
    EXEC SQL FETCH tran_log_3 INTO 
           :sgSwt_tran_log.tran_id,
           :sgSwt_tran_log.tran_begin,
           :sgSwt_tran_log.tran_status,
           :sgSwt_tran_log.tran_end,
           :sgSwt_tran_log.tran_overtime,
           :sgSwt_tran_log.q_tran_begin,
           :sgSwt_tran_log.tc_tran_begin,
           :sgSwt_tran_log.q_target,
           :sgSwt_tran_log.resq_file,
           :sgSwt_tran_log.resq_offset,
           :sgSwt_tran_log.resq_len, 
           :sgSwt_tran_log.resp_file,
           :sgSwt_tran_log.resp_offset,
           :sgSwt_tran_log.resp_len,
           :sgSwt_tran_log.rev_key;	/* add by szehgnye 2002.3.12 */    

    if (sqlca.sqlcode) break;

    _swTimeLongToChs(sgSwt_tran_log.tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_overtime,lsTmp3,"HH:MM:SS");
    Message("   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status,\
      lsTmp1,lsTmp2,lsTmp3,sgSwt_tran_log.q_tran_begin,\
      sgSwt_tran_log.tc_tran_begin);

    ilTotalcount++;
    ilPagecount++;
    if(ilPagecount == 20)
    {
      printf("请按[RETURN]键继续...\n");
      clKey = getchar();
      if (clKey == 'q' || clKey == 'Q')
        break;
      Message("\n                  交易开   交易结   交易超    发起     发起交\n");
      Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
      Message("   ============================================================\n");
      ilPagecount = 2;
    }
  }
  EXEC SQL CLOSE tran_log_3;
  swDbclose();

  Message("   共有[%d]条记录\n",ilTotalcount);
#endif

  swVdebug(4,"S0510: [函数返回] swListtran3()返回码=0");
  return(SUCCESS); 
}

/**************************************************************
 ** 函数名      : swListtran4
 ** 功  能      : 查询冲正失败的交易 
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swListtran4()
{ 
  short ilRn;       /* 返回码 */


  swVdebug(4,"S0520: [函数调用] swListtran4()");

  /* 打开数据库 */
  ilRn=swDbopen();
  if (ilRn) 
  {
    Message("打开数据库失败,sqlca.sqlcode=[%d]\n",ilRn);
    swVdebug(1,"S0530: [错误/数据库] 打开数据库失败,sqlca.sqlcode=[%d]",ilRn);
    return(FAIL); 
  }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL DECLARE tran_log_4 CURSOR FOR SELECT  *
    FROM swt_his_tran_log WHERE tran_status = 5 ORDER BY tran_id;
  if(sqlca.sqlcode)
  {
    Message("定义游标失败,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0540: [错误/数据库] 定义游标失败,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  EXEC SQL OPEN tran_log_4;
  if(sqlca.sqlcode)
  {
    Message("打开游标失败,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0550: [错误/数据库] 打开游标失败,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  ilTotalcount=0;

  Message("\n                  交易开   交易结   交易超    发起     发起交\n");
  Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  while(TRUE)
  {
    memset(&sgSwt_tran_log, 0x00, sizeof(struct swt_tran_log));

    /* EXEC SQL FETCH tran_log_4 INTO :sgSwt_tran_log; delete by szhengye 2002.3.12 */
    EXEC SQL FETCH tran_log_3 INTO 
           :sgSwt_tran_log.tran_id,
           :sgSwt_tran_log.tran_begin,
           :sgSwt_tran_log.tran_status,
           :sgSwt_tran_log.tran_end,
           :sgSwt_tran_log.tran_overtime,
           :sgSwt_tran_log.q_tran_begin,
           :sgSwt_tran_log.tc_tran_begin,
           :sgSwt_tran_log.q_target,
           :sgSwt_tran_log.resq_file,
           :sgSwt_tran_log.resq_offset,
           :sgSwt_tran_log.resq_len, 
           :sgSwt_tran_log.resp_file,
           :sgSwt_tran_log.resp_offset,
           :sgSwt_tran_log.resp_len,
           :sgSwt_tran_log.rev_key;	/* add by szehgnye 2002.3.12 */    

    if (sqlca.sqlcode) break;

    _swTimeLongToChs(sgSwt_tran_log.tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_overtime,lsTmp3,"HH:MM:SS");
    Message("   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status,\
      lsTmp1,lsTmp2,lsTmp3,sgSwt_tran_log.q_tran_begin,\
      sgSwt_tran_log.tc_tran_begin);

    ilTotalcount++;
    ilPagecount++;
    if(ilPagecount == 20)
    {
      printf("请按[RETURN]键继续...\n");
      clKey=getchar();
      if(clKey=='q'||clKey=='Q')
        break;
      Message("\n                  交易开   交易结   交易超    发起     发起交\n");
      Message("     流水号 状态  始时间   束时间   时时间    邮箱       易码\n");
      Message("   ============================================================\n");
      ilPagecount=2;
    }
  }
  EXEC SQL CLOSE tran_log_4;
  swDbclose();

  Message("   共有[%d]条记录\n",ilTotalcount);
#endif

  swVdebug(4,"S0560: [函数返回] swListtran4()返回码=0");
  return(SUCCESS); 
}

/**************************************************************
 ** 函数名      : swResaf
 ** 功  能      : RESAF处理
 ** 作  者      : 张辉
 ** 建立日期    : 1999/11/25
 ** 最后修改日期: 2001/3/29
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swResaf(char *alSafid)
{
  short ilRc;     
  struct msghead slMsghead;

  swVdebug(4,"S0570: [函数调用] swResaf(%s)",alSafid);

  _swTrim(alSafid);
  if (strcmp(alSafid,"\0") == 0)
  {
    Message("Usage: R  Safid\n");
    return(FAIL);
  }

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  /* 定位邮箱  */
  if (qattach( iMBSYSCON ) != 0)
  {
    swVdebug(1,"S0580: [错误/邮箱] qattach()函数,错误码=-1,定位本地邮箱出错(iMBSYSCON)");
    return(FAIL);
  }

  /* 发送命令报文给任务管理器 */
  memset(&slMsghead, 0x00, sizeof(struct msghead));
  slMsghead.iMsgtype = iMSGORDER;
  slMsghead.lCode = 604;
  slMsghead.iBodylen = 0;
  slMsghead.lSafid = atol(alSafid);

  ilRc = _qwrite2((char *)&slMsghead,sizeof(struct msghead),iMBMONREV,0,0,0);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0590: [错误/邮箱] _qwrite2()函数,错误码=%d,发送报文给MONREV失败",ilRc);
    swMberror(ilRc,NULL);
    return (FAIL);
  }

  qdetach();

  Message("重置 SAF 成功!\n");

  swVdebug(4,"S0600: [函数返回] swResaf()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swListsaf
 ** 功  能      : 显示SAF数据 
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swListsaf()
{
  char lsTmp1[10],lsTmp2[10];
  short ilRc;       /* 返回码 */
  char clKey;

  short ilPagecount;               /* 本次显示数目 */
  short i,ilCount;
  struct swt_rev_saf pslSwt_rev_saf[iMAXRECORD];

  swVdebug(4,"S0610: [函数调用] swListsaf()");

  /* 打开数据库 */
  ilRc=swDbopen();
  if (ilRc) 
  {
    Message("打开数据库失败,sqlca.sqlcode=[%d]\n",ilRc);
    swVdebug(0,"S0620: [错误/数据库] 打开数据库失败,sqlca.sqlcode=[%d]",ilRc);
    return(FAIL); 
  }
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  /* modify by nh 20020923
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);*/
/* del by gengling at 2015.04.03 two lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/
  /* ilRc = swDbselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount); */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);
  if (ilRc == SHMNOTFOUND)
  {
    Message("SAF 表中未找到记录!\n");
    swVdebug(1,"S0630: [错误/共享内存] SAF 表中未找到记录!");
    return(FAIL);
  }
  if (ilRc != SUCCESS)
  {
    Message("查找 SAF 记录失败!\n");
    swVdebug(1,"S0640: [错误/共享内存] 查找 SAF 记录失败!");
    return(FAIL);
  }
  swDbclose();
/* #endif */ /* del by gengling at 2015.04.03 PSBC_V1.0 */
 
  Message("\n                SAF开     SAF超   剩余冲\n");
  Message("  流水号 状态  始时间    时时间   正次数\n");
  Message("===========================================\n");
  ilPagecount = 2;

  for(i=0;i<ilCount;i++)
  {
    _swTimeLongToChs(pslSwt_rev_saf[i].saf_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(pslSwt_rev_saf[i].saf_overtime,lsTmp2,"HH:MM:SS");
    Message("%8d   %1.1s %9.9s %9.9s %4d\n", pslSwt_rev_saf[i].tran_id,\
      pslSwt_rev_saf[i].saf_status,lsTmp1,lsTmp2,pslSwt_rev_saf[i].saf_num);
    ilPagecount++;
    if(ilPagecount == 20) 
    {
      printf("请按[RETURN]键继续...\n");
      clKey = getchar();
      if (clKey == 'q' || clKey == 'Q')  break;
      Message("                SAF开     SAF超   剩余冲\n");
      Message("  流水号 状态  始时间    时时间   正次数\n");
      Message("===========================================\n");
      ilPagecount = 2;
    }
  }

  Message("共有[%d]条记录\n",ilCount);

  swVdebug(4,"S0650: [函数返回] swListsaf()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swClrsaf
 ** 功  能      : 清除SAF
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swClrsaf(char *alTranid )
{
  struct swt_rev_saf pslSwt_rev_saf[iMAXRECORD];
  short ilRc,i,ilCount,j = 0,ilAnswer;      
  long  llSaf_id,llTranid;
  
  swVdebug(4,"S0660: [函数调用] swClrsaf(%s)",alTranid);

  _swTrim(alTranid);
  if (strcmp(alTranid,"\0") == 0) 
  {
    Message("Usage: C    Tranid\n");
    Message("if Tranid = -1,Clear All\n");
    return(FAIL);
  }
  
  llTranid = atol(alTranid); 
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  /* modify by nh 20020923
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);*/
/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/
  /* ilRc = swDbselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount); */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);
  if (ilRc == SHMNOTFOUND)
  {
    Message("SAF 表中未找到记录!\n");
    swVdebug(1,"S0670: [错误/共享内存] SAF 表中未找到记录!");
    return(FAIL);
  }
  if (ilRc != SUCCESS)
  {
    Message("查找 SAF 记录失败!\n");
    swVdebug(1,"S0680: [错误/共享内存] 查找 SAF 记录失败!");
    return(FAIL);
  }

  if(llTranid == -1) 
  {
    printf("确认要清空表中的所有记录吗? (Y/N)");
    ilAnswer = getchar();
    if (ilAnswer != 'y' && ilAnswer != 'Y') return(FAIL); 

    for(i=0;i<ilCount;i++)
    {
      /* modify by nh 20020923 
      ilRc = swShmdelete_swt_rev_saf(pslSwt_rev_saf[i].saf_id); */
	  /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
      /* ilRc = swDbdelete_swt_rev_saf(pslSwt_rev_saf[i].saf_id); */
	  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmdelete_swt_rev_saf(pslSwt_rev_saf[i].saf_id);
      if (ilRc != SUCCESS)
      {
        swVdebug(1,"S0690: [错误/共享内存] 执行swShmdelete_swt_rev_saf出错");
        Message("删除 SAF 记录失败!\n");
        Message("删除了[%d]条SAF记录]\n",ilCount);
        return(FAIL);
      }
    }
    Message("删除了[%d]条SAF记录\n",ilCount);

    swVdebug(4,"S0700: [函数返回] swClrsaf()返回码=0");
    return(SUCCESS);
  }
  j=0;
  for (i = 1;i < ilCount;i++)
  {
    if (pslSwt_rev_saf[i].tran_id == llTranid)
    {
      llSaf_id = pslSwt_rev_saf[i].saf_id;
      /* modify by nh 20020923 
      ilRc = swShmdelete_swt_rev_saf(llSaf_id); */
	  /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
      /* ilRc = swDbdelete_swt_rev_saf(llSaf_id); */
	  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmdelete_swt_rev_saf(llSaf_id);
      if (ilRc != SUCCESS)
      {
        swVdebug(1,"S0710: [错误/共享内存] 执行swShmdelete_swt_rev_saf出错");
        Message("删除 SAF 记录出错!\n");
        Message("已删除[%d]条SAF记录!\n",j);
        return(FAIL);
      }
      j ++;
    }
  }
  Message("删除[%d]条SAF记录!\n",j);
/* #endif */ /* del by gengling at 2015.04.03 PSBC_V1.0 */

  swVdebug(4,"S0720: [函数返回] swClrsaf()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swListport
 ** 功  能      : 查看端口状态
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swListport(char *alQ_id)
{
  short i,j,ilQ_id_where;
  short ilCount,ilTotalcount,ilRc,ilTmp = 0;
  struct swt_sys_queue  slSwt_sys_queue[400];
  char  alPortname[21];
  int   ilPagecount;
  int   ilTemp[8];

  swVdebug(4,"S0730: [函数调用] swListport(%s)",alQ_id);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  Message("\n     端口号    名称       通讯层  应用层    Q层   Echotest层  人工干预层\n");
  Message("  ======================================================================\n");
  ilPagecount=2;
  ilTotalcount=0;

  if (strcmp(alQ_id,"\0") == 0)
    ilQ_id_where = 0;
  else
    ilQ_id_where = atoi(alQ_id);

  ilRc = swShmselect_swt_sys_queue_all(slSwt_sys_queue,&ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0740: [错误/共享内存] 执行 swShmselect_swt_sys_queue_all 失败!"); 
    Message("执行 swShmselect_swt_sys_queue_all 失败!\n");
    return(FAIL);
  }
  
  for (i= 0;i < ilCount;i++)
  {
    if (ilQ_id_where > 0)
      if (slSwt_sys_queue[i].q_id != ilQ_id_where) 
          continue;
    
    for(j=0;j<8;j++)
    {
      if ( slSwt_sys_queue[i].port_status & (128>>j) )
        ilTemp[j]=1;
      else
        ilTemp[j]=0;
    }

    if (strlen(slSwt_sys_queue[i].q_name) > 20)
    {
      memcpy(alPortname,slSwt_sys_queue[i].q_name,17);
      alPortname[17] = '\0';
      strcat(alPortname,"...");
    }
    else
      strncpy(alPortname,slSwt_sys_queue[i].q_name,20);
 
    Message("  %-6d%-20s  %-8d%-8d%-8d%-8d%-8d\n", slSwt_sys_queue[i].q_id,
      alPortname,ilTemp[0],ilTemp[1],ilTemp[2],ilTemp[3],ilTemp[4]);
    ilPagecount++;
    ilTotalcount++;
    if ((ilTmp == 0) && (ilQ_id_where > 0))
    {
      ilTmp = 1;
      break;
    }

    if(ilPagecount == 21)
    {
      printf("请按[RETURN]键继续...\n");
      if(getchar() == 'q') break;

      Message("\n     端口号    名称       通讯层  应用层    Q层   Echotest层  人工干预层\n");
      Message("  ======================================================================\n");
      ilPagecount=2;
    }
  }

  if ((ilTmp == 0) && (ilQ_id_where >0))
  {
    Message("端口未定义!\n");
    return(FAIL);
  }

  Message("   总共有[%d]个信箱信息\n",ilTotalcount);

  swVdebug(4,"S0750: [函数返回] swListport()返回码=0");
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : swClrmailbox
 ** 功  能      : 清空邮箱
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/23
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swClrmailbox(char *alQid)
{
  int ilRc,ilQid;
  short ilAnswer;

  swVdebug(4,"S0760: [函数调用] swClrmailbox(%s)",alQid);

  _swTrim(alQid);
  if (strcmp(alQid,"\0") == 0)
  {
    Message("Usage: C   q_id\n");
    Message("if q_id = 0,clear all Mailbox\n");
    return(FAIL);
  } 

  ilQid = atoi(alQid);
  if (ilQid == 0)
  {
    printf("确认要清空所有邮箱吗? (Y/N)");
    ilAnswer = getchar();
    if (ilAnswer != 'y' && ilAnswer != 'Y')  return(FAIL);
  }
 
  ilRc = bmqOpen(iMBSYSCON);
  if (ilRc)
  {
    swVdebug(0,"S0770: [错误/邮箱] bmqOpen()函数,错误码=%d",ilRc);
    return(FAIL);
  }

  ilRc = bmqClearmb(ilQid);
  if (ilRc)
  {
    Message("清空邮箱失败\n");
    ilRc = bmqClose();
    if (ilRc)
    {
      swVdebug(0,"S0780: [错误/邮箱] bmqClose()函数,错误码=%d",ilRc);
      return(FAIL);
    }
    return(FAIL);
  }
  Message("清空邮箱成功\n");
  ilRc = bmqClose();
  if (ilRc)
  {
    swVdebug(0,"S0790: [错误/邮箱] bmqClose()函数,错误码=%d",ilRc);
    return(FAIL);
  }

  swVdebug(4,"S0800: [函数返回] swClrmailbox()返回码=0");
  return(SUCCESS);
}
   
/**************************************************************
 ** 函数名      : swListmailbox
 ** 功  能      : 显示邮箱信息
 ** 作  者      : 顾晓忠
 ** 建立日期    : 2001/08/23
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
extern int swListmailbox()
{

  short ilRc,i;
  char  clKey;
  struct mbinfo slMbinfo;

  short ilTotalcount=0;              /* 总数 */
  short ilPagecount =0;              /* 本次显示数目 */

  swVdebug(4,"S0810: [函数调用] swListmailbox()");

  Message("\n      邮箱号    接收条数    发送条数    余留条数    连接数  \n");
  Message("   ==========================================================\n");
  ilPagecount = 2;
  
  i = 1;
  ilRc = bmqOpen(iMBSYSCON);
  if (ilRc)
  {
    swVdebug(0,"S0820: [错误/邮箱] bmqOpen()函数,错误码=%d",ilRc);
    return(FAIL);
  }
 
  memset(&slMbinfo,0x0,sizeof(struct mbinfo));

  while((ilRc = bmqGetmbinfo(i,&slMbinfo)) != 100)
  {
    if (ilRc == -1)
    {
      swVdebug(1,"S0830: [错误/邮箱] bmqGetmbinfo()函数,错误码=%d",ilRc);
      return(FAIL);
    }
    if ( ilRc || slMbinfo.lSendnum || slMbinfo.lRecvnum || slMbinfo.lPendnum || slMbinfo.iConnect )
    {    
      Message("       %-7d    %-8d    %-7d     %-7d     %-6d\n", \
        i, slMbinfo.lRecvnum, slMbinfo.lSendnum, slMbinfo.lPendnum,
        slMbinfo.iConnect);

      ilPagecount++;
      ilTotalcount++;
      if (ilPagecount == 21)
      {
        printf("请按[RETURN]键继续...\n");
        clKey = getchar();
        if(clKey == 'q' || clKey == 'Q')  break;
        Message("\n      邮箱号    接收条数    发送条数    余留条数    连接数  \n");
        Message("   ==========================================================\n");
        ilPagecount = 2;
      }
    }
    i++;
    memset(&slMbinfo,0x0,sizeof(struct mbinfo));
  }

  Message("   总共有[%d]个信箱信息\n",ilTotalcount);
  ilRc = bmqClose();
  if (ilRc)
  {
    swVdebug(0,"S0840: [错误/邮箱] bmqClose()函数,错误码=%d",ilRc);
    return(FAIL);
  }

  swVdebug(4,"S0850: [函数返回] swListmailbox()返回码=0");
  return(SUCCESS);
}

/* === begin of added by fzj at 2002.03.02 === */
/**************************************************************
 ** 函数名      : swListporttran
 ** 功  能      : 查看端口交易数
 ** 作  者      : fanzhijie
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swListporttran(char *alQ_id)
{
  short i,ilQ_id_where;
  short ilCount,ilTotalcount,ilRc,ilTmp = 0;
  struct swt_sys_queue  slSwt_sys_queue[400];
  int   ilPagecount;

  swVdebug(4,"S0860: [函数调用] swListport(%s)",alQ_id);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  Message("\n  端口号  源发交易数  最大源发交易数  处理交易数  最大处理交易数\n");
  Message("  ==============================================================\n");
  ilPagecount=2;
  ilTotalcount=0;

  if (strcmp(alQ_id,"\0") == 0)
    ilQ_id_where = 0;
  else
    ilQ_id_where = atoi(alQ_id);

  ilRc = swShmselect_swt_sys_queue_all(slSwt_sys_queue,&ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0870: [错误/共享内存] 执行 swShmselect_swt_sys_queue_all 失败!"); 
    Message("执行 swShmselect_swt_sys_queue_all 失败!\n");
    return(FAIL);
  }
  
  for (i= 0;i < ilCount;i++)
  {
    if (ilQ_id_where > 0)
      if (slSwt_sys_queue[i].q_id != ilQ_id_where) continue;

    Message("  %6d  %10d  %12d  %10d  %12d\n", slSwt_sys_queue[i].q_id, slSwt_sys_queue[i].tranbegin_num, slSwt_sys_queue[i].tranbegin_max, slSwt_sys_queue[i].traning_num, slSwt_sys_queue[i].traning_max);

    ilPagecount++;
    ilTotalcount++;
    if ((ilTmp == 0) && (ilQ_id_where > 0))
    {
      ilTmp = 1;
      break;
    }

    if(ilPagecount == 21)
    {
      printf("请按[RETURN]键继续...\n");
      if(getchar() == 'q') break;
      Message("\n  端口号  源发交易数  最大源发交易数  处理交易数  最大处理交易数\n");
      Message("  ==============================================================\n");
      ilPagecount=2;
    }
  }

  if ((ilTmp == 0) && (ilQ_id_where >0))
  {
    Message("端口未定义!\n");
    return(FAIL);
  }

  Message("   总共有[%d]个信箱信息\n",ilTotalcount);

  swVdebug(4,"S0880: [函数返回] swListporttran()返回码=0");
  return(SUCCESS);
}
/* === end of added by fzj at 2002.03.02 === */

/* === begin of added by fzj at 2002.03.02 === */
/**************************************************************
 ** 函数名      : swResetporttran
 ** 功  能      : 重置端口交易数
 ** 作  者      : fanzhijie
 ** 建立日期    : 2001/08/20
 ** 最后修改日期: 
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : SUCCESS/FAIL
***************************************************************/
int swResetporttran()
{
  int ilRc;

  swVdebug(4,"S0890: [函数调用] swResetporttran()");

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("共享内存检测出错!\n");
    return(FAIL);
  }

  ilRc = swShmresetporttran();
  if (ilRc)
  {
    Message("重置端口交易数出错!\n");
    return(FAIL);
  }
  Message("重置端口交易数成功 !\n");
  return(SUCCESS);
}
/* === end of added by fzj at 2002.03.02 === */

/* add by gengling at 2013.04.17 begin PSBC_V1.0 */
/************************************************************
 **功  能: 重置某一笔交易流量控制的交易数
 **参  数: iQid      - 输入参数 源发邮箱
           aTrancode - 交易码
 **返回值: 0  - 成功
 ************************************************************/
int swShmtranflowreset(char *iQid, char *aTrancode)
{
    int i;
    short ilRc;
    
    swVdebug(4, "[函数调用] swShmtranflowreset(%s,%s)", iQid, aTrancode);

    ilRc = swShmcheck();
    if (ilRc != SUCCESS)
    {
        Message("共享内存检测出错!\n");
        return(FAIL);
    }

#ifndef HASH
    for (i=0; i<psmShmidx_s->sIdx_sys_tran.iCount; i++)
    {
        if (smShmbuf_s.psSwt_sys_tran[i].sys_id == atoi(iQid) &&
                !strncmp(smShmbuf_s.psSwt_sys_tran[i].tran_code, aTrancode, sizeof(smShmbuf_s.psSwt_sys_tran[i].tran_code)))
        {
            smShmbuf_s.psSwt_sys_tran[i].tranning_num = 0;
            swVdebug(2, "重置(%s,%s)交易数为0", iQid, aTrancode);
            break;
        }
    }
#else
    long dataoffset=0;
    struct swt_sys_tran *pslTranid;
    memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
    pslTranid = (struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset);

    for (i=0; i<psmShmidx_s->sIdx_sys_tran_hash.iCount; i++)
    {
        if (pslTranid[i].sys_id == atoi(iQid) &&
                !strncmp(pslTranid[i].tran_code, aTrancode, sizeof(pslTranid[i].tran_code)))
        {
            pslTranid[i].tranning_num = 0;
            swVdebug(2, "重置(%s,%s)交易数为0", iQid, aTrancode);
            break;
        }
    }
#endif
    swVdebug(4, "[函数返回] swShmtranflowreset()返回码=0");
    return(SUCCESS);
}

/************************************************************
 **功  能: 重置所有交易流量控制的交易数
 **参  数: 无
 **返回值: 0  - 成功
 ************************************************************/
int swShmtranflowresetall()
{
    int i;
    short ilRc;
    
    swVdebug(4, "[函数调用] swShmtranflowresetall()");

    ilRc = swShmcheck();
    if (ilRc != SUCCESS)
    {
        Message("共享内存检测出错!\n");
        return(FAIL);
    }

#ifndef HASH
    for (i=0; i<psmShmidx_s->sIdx_sys_tran.iCount; i++)
    {
        smShmbuf_s.psSwt_sys_tran[i].tranning_num = 0;
        swVdebug(2, "重置(%d,%s)交易数为0", smShmbuf_s.psSwt_sys_tran[i].sys_id, smShmbuf_s.psSwt_sys_tran[i].tran_code);
    }
#else
    long dataoffset=0;
    struct swt_sys_tran *pslTranid;
    memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
    pslTranid = (struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset);

    for (i=0; i<psmShmidx_s->sIdx_sys_tran_hash.iCount; i++)
    {
            pslTranid[i].tranning_num = 0;
            swVdebug(2, "重置(%d,%s)交易数为0", pslTranid[i].sys_id, pslTranid[i].tran_code);
    }
#endif
    swVdebug(4, "[函数返回] swShmtranflowresetall()返回码=0");
    return(SUCCESS);
}
/* add by gengling at 2013.04.17 end PSBC_V1.0 */
