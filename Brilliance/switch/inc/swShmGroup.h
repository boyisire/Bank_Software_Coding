#define MAXGROUPCOUNT 16
#define MAXMBCOUNT 300



/*������Ϣ*/
typedef struct {
  int		iGrpID;          /*���*/
  char	aGrpIP[15+1];    /*����IP��ַ*/	
  int		iGrpPort;	 /*�˿ں�*/
  long		iBegSeqNo;			 /*��ʼ��ˮ��*/
  long		iEndSeqNo;		/*��ֹ��ˮ��*/
  int		iUseFlag;				 /*���ñ�־0:δ���� 1������*/
 }PB_GROUPINFO;

/*������Ϣ*/
typedef struct 
{
	
  int 	iMbID;           /*�����*/
  int		iMbStaus;			/*0.δ֪��1.UP,2.DOWN*/
}PB_GRO_MBINFO;

typedef struct 
{
	int		iGrp;				 /*���*/
	int		iGrpStatus;  /*��״̬0��δ���� 1������ 2��δ֪ 3.δ����*/
	long	lRfrhTime;  /*ˢ��ʱ��*/
	PB_GRO_MBINFO sMbInfo[MAXMBCOUNT]; 
}PB_HOST_MBINFO;

struct SHM_GROUPINFO{
  
  int		iSemid;																		/*�ź���*/
	int		iGrpCount;		 														/*��ͳ��*/
	int		iGrpID;																		/*�������*/
  PB_GROUPINFO sGrpInfo[MAXGROUPCOUNT];							/*��Կ���������б�*/
  PB_HOST_MBINFO sGrpMbInfo[MAXGROUPCOUNT]; /*����״̬��¼*/
};


struct SHM_GROUPINFO *psgGrpShm_pid;

int igGrpId;
int		igGroCount;
int		igGrpOldCount;  														
PB_GROUPINFO sgGrpInfo[MAXGROUPCOUNT];														
PB_HOST_MBINFO sgSinHostInfo;
PB_HOST_MBINFO sgCountHostInfo[MAXGROUPCOUNT];
PB_HOST_MBINFO sgOldCountHostInfo[MAXGROUPCOUNT];






