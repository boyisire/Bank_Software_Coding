/*************+��������+***************/
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

/****************+�궨��+***************/
#define RECODE_NUM 500			//���鳤��
#define OPERFLAG_LEN 9			//������ʶ�ĳ���
#define EXPDP_CPUNUM 1			//�����ݱõ���ʱ��ʹ�õ�CPU����

/*************+�ⲿ��������+************/
extern char 	CfgFile[128];		//�����ļ�
extern char	SysDate_Count[20];	//ͨ����������õ�����
extern char	TabSpaceName[50];	//��ռ���
extern time_t 	sysTimeSec;		//����ʱ�亯��ȡ������
extern char	glMonServerIp[14];	//��ط�����IP
extern int	glMonServerPort;	//��ط������˿�

/**********+ȫ�ֱ�����������+***********/
char 	StrBuf[1024+1];			//�����Buf���������ַ���
int	RecNum=0;			//��¼�����еļ�¼��
char	SysDate[20];			//��ǰϵͳ����
char	SysDate_1[20];			//��ǰ��������(��ϵͳ���ڼ�1��)
char 	DmpFileDir[128];		//Dmp�ļ����Ŀ¼
char 	DmpLogsDir[128];		//Dmp��־���Ŀ¼
char 	DmpFile[128];			//Dmp�ļ���
char 	DmpLogs[128];			//Dmp�ļ���
char 	TabNameTmp[50];			//��ʱ����
char 	MonCodeStr[10];			//��ط�����

int 	DayNum=0;			//��������
time_t	Date_Begin;			//����ʼʱ��
time_t	Date_End;			//�������ʱ��


/******���ⲿ��������������ļ����*****/
char	glCfgFile[128];			//�����ļ�
char	glUserName[20];			//���ñ��������ݿ��û���
char	glPassWord[20];			//���ñ��������ݿ���  ��
char	glTnsNames[20];			//���ñ��������ݿ����Ӵ�
char	glDB_ServerFlag[2];		//���ݿ��ʶ
char	glSql_Sort[10];			//���ݿ�SQL����
int	glTestFlag=0;			//���Կ��� (1�򿪣�0�ر�)
int	glTabFlag=0;			//�Ƿ��Ե���ģʽִ�п���
char	glTabName[50];			//ִ�е���ģʽʱ�ı���
