/****************************************************************/
/* 模块编号	：DT_Main					*/
/* 模块名称	：主程序					*/
/* 版 本 号	：V1.0						*/
/* 作    者	：Hu						*/
/* 建立日期    	：2015/3/6 11:32:47				*/
/* 最后修改日期	：						*/
/* 模块用途	：主程序					*/
/* 本程序连接的库说明：						*/
/*	<1>DT_Func_Pub.c	常用公共函数库			*/
/*	<2>DT_Func_Db.sqc	常用数据库操作库		*/
/*	<3>DT_Proc_DayEnd.sqc	日终操作处理模块		*/
/*	<4>DT_Proc_Mon.c	监控处理模块			*/
/* 本模块中包含如下函数及功能说明：				*/
/*	<01>._bswTrim		去空格				*/

/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2015/3/25 21:13:28 日终模块已完毕				*/
/* 2015/3/26 21:10:22 操作时间更新模块完毕			*/
/* 2015/3/26 21:12:25 参数配置模块完毕				*/
/* 2015/3/27 18:13:28 文件按模块拆分完毕			*/
/* 2015/3/27 18:17:13 Makefile修正完毕				*/
/* 2015/3/30 18:14:12 密码为环境变量时处理			*/
/* 2015/3/30 18:14:12 游标由静态改为动态			*/
/* 备注记录：							*/
/* 2015/3/27 18:13:28 监控模块待开发				*/
/* 2015/3/27 18:13:28 读传参配置文件待开发			*/
/* 2015/3/27 18:13:28 根据参数取表名方式待开发			*/
/* 2015/3/27 18:13:28 菜单操作待开发				*/
/****************************************************************/
#include "DT_Pub.h"


/************************变量定义引用**************************/
extern int	glTestFlag;		//测试开关
extern char	SysDate[10];		//系统时间
extern char 	glDB_ServerFlag[2];	//数据库标识
extern char	glSql_Sort[10];		//SQL排序方式
extern char	glCfgFile[50];		//配置文件
extern int	glTabFlag;		//是否以单表模式执行开关
extern char	glTabName[50];		//执行单表模式时的表名


/*外部传入参数在本程序中定义*/
const char* ProgramName;		//程序名
const char* agDB_ServerFlag=NULL;	//数据库服务器标识
const char* agSql_Sort=NULL;		//SQL排序方式
const char* agCfgFile=NULL;		//外部接受配置文件名
const char* agTabName=NULL;		//外部单表表名
/**************************函数声明***************************/
//打印帮助
void M_PrintUsage(FILE* stream, int exit_code);
 

/**************************************************************
 ** 函数名      : main
 ** 功  能      : 主函数
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 16:42:47
 ** 最后修改日期: 2015/3/6 16:42:49
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : int argc, char *argv[]
 ** 返回值      : FAIL:失败. SUCC:成功
***************************************************************/
int main(int argc, char *argv[])
{
	/*****参数处理部********/
	
	const char* output_filename = NULL;
	const char* input_filename = NULL;
	
	int	verbose = 0;	//版本号
	int	DayEndFlag=0;	//是否进行日终操作
	int	next_option;	//参数个数
	
	
/*-------------------------------关于:getopt_long() 说明---------------------------------------*/
/*1.一个字符串，包括所需要的短选项字符，如果选项后有参数，字符后加一个":"符号。
 *	本例中，这个字符串应该为"ho:v"。(因为-o后面有参数filename,所以字符后面需要加":")。
 *2. 一个包含长选项字符串的结构体数组，每一个结构体包含4个域:
 *	第一个域为长选项字符串，
 *	第二个域是一个标识，只能为0或1，分别代表没有选项或有选项。
 *	第三个域永远为NULL。
 *	第四个选项域为对应的短选项字符串。
 *	结构体数组的最后一个元素全部位NULL和0，标识结束。
 */
	
	//程序名
	ProgramName = argv[0];	
	//短项参数
	const char* const short_options = "HDd:s:c:l:Tt:V";	
	//长项参数
	const struct option long_options[] = {
		{"Help",0, NULL, 'H'},
		{"DayEnd", 0, NULL, 'D'},
		{"dbFlag", 1, NULL, 'd'},
		{"sqlSort",1, NULL, 's'},
		{"agCfgFile", 1, NULL, 'c'},
		{"logLevel", 1, NULL, 'l'},
		{"Test", 0, NULL, 'T'},
		{"tab", 1, NULL, 't'},
		{"Version", 0, NULL, 'V'},
		{NULL, 0, NULL, 0}    /* Required at end of array. */
	};

	/*-------------------------------------------------------参数处理分隔线--------------------------------------------Begin*/	
	do {
		next_option = getopt_long(argc, argv, short_options,long_options, NULL);
		switch (next_option)
		{
		case 'H':    /* -H or --Help */	 
			 M_PrintUsage(stdout, 0);	
		case 'D':    /* -D or --DayEnd */	 
			 DayEndFlag =1; 
			 break;
		case 'd':    /* -d or --dbFlag */	        	 
			 agDB_ServerFlag = optarg;	        
			 INIT(glDB_ServerFlag);	
			 sprintf(glDB_ServerFlag,"%s",agDB_ServerFlag);
			 _swVdebug(0,"glDB_ServerFlag=%s\n",glDB_ServerFlag);
			 break;
		case 's':    /* -c or --agCfgFile */
		        agSql_Sort = optarg;
		        INIT(glSql_Sort);
			sprintf(glSql_Sort,"%s",agSql_Sort);
		        _swVdebug(0,"glSql_Sort=%s\n",glSql_Sort);		       
		        break;	
		case 'c':    /* -c or --agCfgFile */
		        output_filename = optarg;
		        _swVdebug(0,"agCfgFile=%s\n",output_filename);		       
		        break;
		case 'l':    /* -l or --logLevel */
		        input_filename = optarg;
		        _swVdebug(0,"input_filename=%s\n",input_filename);
		        break;	
		case 'T':    /* -T or --Test */
		        glTestFlag=1;
		        _swVdebug(0,"glTestFlag=%d\n",glTestFlag);
		        break;	
		case 't':    /* -t or --tab */
		        agTabName=optarg;	//单表模式表名.		        
		        glTabFlag=1;		//是否以单表模式执行.
		        INIT(glTabName);
			sprintf(glTabName,"%s",agTabName);
		        break;	
		case 'V':    /* -v or --Version */
		        verbose = 1.0;
		        printf("Version=%d\n",verbose);
		        break;	
		case '?': 
			M_PrintUsage(stderr, 1);	
		case -1:    /* Done with options. */
			break;	
		default:    /* Something else: unexpected. */
			abort();
		}
	} while (next_option != -1);
	/*-------------------------------------------------------参数处理分隔线--------------------------------------------End*/
		
	int rc=0;
	char alTmp[512];
	char LogFile[128];	//日志文件
	char LogDirs[128];	//日志目录 
        
	/* 获取当前系统日期YYYYMMDD存在SysDate中 */
        time(&t);          
	memset(SysDate, 0x00, sizeof(SysDate));                     
	strftime(SysDate, sizeof(SysDate), "%Y%m%d", localtime(&t));

	/* 配置文件*/
	memset(glCfgFile, 0x00, sizeof(glCfgFile));
	sprintf(glCfgFile, "%s/etc/HisDayEnd.ini", getenv(DAYEND_ENV));
	
	/* 日志文件名 */
	if(strcmp(glDB_ServerFlag,"A")==0){
		memset(LogFile, 0x00, sizeof(LogFile));
		sprintf(LogFile, "log/%s_%s_A.%s.debug", "DBTOOLS","DayEnd",SysDate);
		sprintf(agDebugfile, "%s", LogFile);
	}else if(strcmp(glDB_ServerFlag,"B")==0){
		memset(LogFile, 0x00, sizeof(LogFile));
		sprintf(LogFile, "log/%s_%s_B.%s.debug", "DBTOOLS","DayEnd",SysDate);
		sprintf(agDebugfile, "%s", LogFile);
	}else if(strcmp(glDB_ServerFlag,"Y")==0){
		memset(LogFile, 0x00, sizeof(LogFile));
		sprintf(LogFile, "log/%s_%s_Y.%s.debug", "DBTOOLS","DayEnd",SysDate);
		sprintf(agDebugfile, "%s", LogFile);
	}else {
		memset(LogFile, 0x00, sizeof(LogFile));
		sprintf(LogFile, "log/%s_%s.%s.debug", "DBTOOLS","DayEnd",SysDate);
		sprintf(agDebugfile, "%s", LogFile);
	}
	
	/* DEBUG */
	memset(alTmp, 0x00, sizeof(alTmp));
	if (_GetProfileString(glCfgFile, "FILECFG", "DEBUG", alTmp) < 0)
	{
		_swVdebug(0, "读 DEBUG 出错! 使用默认值[5]!当前配置文件[%s]",glCfgFile);
		sprintf(alTmp, "5");		
	}
	_bswTrim(alTmp);
	cgDebug = atoi(alTmp);	
	
	if(argc == 1) {
		printf("亲！您想让我做什么呢？给点提示呗！\n需要帮助请使用：%s --Help \n",argv[0]);
		return FAIL;
	}else{
		_swVdebug(0, "当前日志文件为[%s]   日志级别为[%d]!",LogFile,cgDebug);
		_swVdebug(0, "当前配置文件为[%s]",glCfgFile);
		printf("开始\n");
		/*历史日终处理程序*/
		if (DayEndFlag){
			_swVdebug(0, "-------------------->>>日终程序执行开始<<<---------------------------");
			rc=Proc_DayEndMain();
		 	_swVdebug(0, "-------------------->>>日终程序执行结束.返回值【%d】<<<---------------\n",rc);
		 }	
		 printf("结束\n");
		/*菜单模式*/
		/*常用SQL快捷处理*/
	}
	
	return SUCC;
}


/**************************************************************
 ** 函数名      : M_PrintUsage
 ** 功  能      : 打印帮助信息
 ** 作  者      : Hu
 ** 建立日期    : 2015/3/6 16:42:47
 ** 最后修改日期: 2015/3/6 16:42:49
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : FAIL:失败. SUCC:成功
***************************************************************/
void M_PrintUsage(FILE* stream, int exit_code)
{
    fprintf(stream, "Usage: %s Options [InputInfo]\n",ProgramName);
    fprintf(stream, 
        "	-H	--Help					帮助信息.\n"
        "	-D	--DayEnd				执行历史日终程序.\n"
        "	-d	--dbFlag	Flag			数据库标识[A | B | H | Y | X-除前面外].\n"
        "	-T	--Test					测试模式执行(只打印执行语句，不做实际操作).\n"
        "	-t	--tab					传入表名，取配置文件中的表项,执行相关操作.\n"
        "	-s	--sqlSort	SortMode		游标排序方式[DESC|ASC].\n"
        "	-c	--agCfgFile	FileName		读取配置文件[绝对路径].\n"
        "	-l	--logLevel	Level			日志级别[0-5].\n"
        "	-V	--Version				获取当前程序版本号.\n");
    exit (exit_code);
}
