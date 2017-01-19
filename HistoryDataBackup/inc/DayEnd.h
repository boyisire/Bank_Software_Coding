/*************+函数声明+***************/
int Proc_DayEndMain();
void Proc_DayEnd();
int Proc_DataLoding();
int Proc_DB_Exp();
int Proc_DB_Imp();
int Proc_OL_TransDetail();
int Proc_UpdateDate();
int Proc_Monitor();
int Proc_Tab();
int Proc_MvImpdpFile();

/****************+宏定义+***************/
#define RECODE_NUM 500			//数组长度
#define OPERFLAG_LEN 9			//操作标识的长度
#define EXPDP_CPUNUM 1			//用数据泵导数时，使用的CPU个数

/*************+外部变量引用+************/
extern char 	CfgFile[128];		//配置文件
extern char	SysDate_Count[20];	//通过计算后所得的日期
extern char	TabSpaceName[50];	//表空间名
extern time_t 	sysTimeSec;		//定义时间函数取分钟数
extern char	glMonServerIp[14];	//监控服务器IP
extern int	glMonServerPort;	//监控服务器端口

/**********+全局变量变量定义+***********/
char 	StrBuf[1024+1];			//定义大Buf用来传递字符串
int	RecNum=0;			//记录数组中的记录数
char	SysDate[20];			//当前系统日期
char	SysDate_1[20];			//当前清理日期(即系统日期减1天)
char 	DmpFileDir[128];		//Dmp文件存放目录
char 	DmpLogsDir[128];		//Dmp日志存放目录
char 	DmpFile[128];			//Dmp文件名
char 	DmpLogs[128];			//Dmp文件名
char 	TabNameTmp[50];			//临时表名
char 	MonCodeStr[10];			//监控返回码

int 	DayNum=0;			//日期天数
time_t	Date_Begin;			//程序开始时间
time_t	Date_End;			//程序结束时间


/******与外部传入参数和配置文件相关*****/
char	glCfgFile[128];			//配置文件
char	glUserName[20];			//配置表所在数据库用户名
char	glPassWord[20];			//配置表所在数据库密  码
char	glTnsNames[20];			//配置表所在数据库连接串
char	glDB_ServerFlag[2];		//数据库标识
char	glSql_Sort[10];			//数据库SQL排序
int	glTestFlag=0;			//测试开关 (1打开，0关闭)
int	glTabFlag=0;			//是否以单表模式执行开关
char	glTabName[50];			//执行单表模式时的表名
