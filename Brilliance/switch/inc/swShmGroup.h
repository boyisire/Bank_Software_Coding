#define MAXGROUPCOUNT 16
#define MAXMBCOUNT 300



/*跨组信息*/
typedef struct {
  int		iGrpID;          /*组号*/
  char	aGrpIP[15+1];    /*跨组IP地址*/	
  int		iGrpPort;	 /*端口号*/
  long		iBegSeqNo;			 /*起始流水号*/
  long		iEndSeqNo;		/*终止流水号*/
  int		iUseFlag;				 /*启用标志0:未启用 1：启用*/
 }PB_GROUPINFO;

/*邮箱信息*/
typedef struct 
{
	
  int 	iMbID;           /*邮箱号*/
  int		iMbStaus;			/*0.未知，1.UP,2.DOWN*/
}PB_GRO_MBINFO;

typedef struct 
{
	int		iGrp;				 /*组号*/
	int		iGrpStatus;  /*组状态0：未启用 1：启用 2：未知 3.未配置*/
	long	lRfrhTime;  /*刷新时间*/
	PB_GRO_MBINFO sMbInfo[MAXMBCOUNT]; 
}PB_HOST_MBINFO;

struct SHM_GROUPINFO{
  
  int		iSemid;																		/*信号量*/
	int		iGrpCount;		 														/*组统计*/
	int		iGrpID;																		/*本机组号*/
  PB_GROUPINFO sGrpInfo[MAXGROUPCOUNT];							/*密钥索引代码列表*/
  PB_HOST_MBINFO sGrpMbInfo[MAXGROUPCOUNT]; /*邮箱状态记录*/
};


struct SHM_GROUPINFO *psgGrpShm_pid;

int igGrpId;
int		igGroCount;
int		igGrpOldCount;  														
PB_GROUPINFO sgGrpInfo[MAXGROUPCOUNT];														
PB_HOST_MBINFO sgSinHostInfo;
PB_HOST_MBINFO sgCountHostInfo[MAXGROUPCOUNT];
PB_HOST_MBINFO sgOldCountHostInfo[MAXGROUPCOUNT];






