#define swVdebug(level, ...) _swVdebug(level, __FILE__, __LINE__, __VA_ARGS__)
#define   MSGMAXLEN     8192
int flag=-1;
char    agDebugfile[64];                /* �����ļ��� */
struct config
{
	int iport;
	int iportnum;
	char aip[32];
	time_t oldtime;
	time_t freshtime;
};
struct config extconfig;
struct msgMacPack
{
/*����MAC�õ�������*/
        int tran_type;               /*��������*/
        int  algoType;               /*��Կ�㷨����*/
        char subjectID[64];          /*MAC��ԿID*/
        int  subjectIDLen;           /*MAC��ԿID����*/
        char userData[2048];          /*�û�����*/
        int  userDataLen;            /*�û����ݳ���*/
        char macData[64];           /*MAC����*/
        int macDataLen;             /*MAC���ݳ���*/
/*ת�����õĵ�����*/
        int srcAlgoType;          /*Դ�����㷨��ʾ*/
        int srcPinType;              /*ԴPIN����ģʽ*/
        char srcSubjectID[64];      /*Դ�Գ���ԿID*/
        int srcSubjectIDLen;        /*Դ�Գ���ԿID����*/
        int destAlgoType;           /*Ŀ�ļ����㷨��ʾ*/
        int destPinType;            /*Ŀ��PIN����ģʽ*/
        char destSubjectID[64];     /*Ŀ�ĶԳ���ԿID*/
        int destSubjectIDLen ;      /*Ŀ�ĶԳ���ԿID����*/
        char userAccount[64];           /*�û��˺�*/
        int userAccountLen;         /*�û��ʺų���*/
        char srcPIN[64];            /*ԴPIN����*/
        int srcPINLen;              /*ԴPIN���ݳ���*/
        char destPIN[64];           /*Ŀ��PIN����*/
        int destPINLen;             /*Ŀ��PIN���ݳ���*/
	char account_flag[2];       /*�ж��ǲ��Ǵ��˺�ת����*/

/*des תmd5�õ�������*/

        int pinType;                /*PIN����ģʽ*/
        int hashAlgo;               /*ժҪ�㷨����*/  
        char salt[128];             /*ժҪ����*/ 
        int saltLen;                /*ժҪ���ӳ���*/ 
        char EncSubjectPIN[64];    /*���ܵ�PIN����*/ 
        int EncSubjectPINLen;       /*���ܵ�PIN���ݳ���*/
        char hashPIN[64];           /*ժҪ����*/
        int hashPINLen;             /*ժҪ�����*/
/*��Կ����*/
        int workKeyType;            /*ƽ̨������Կ����*/        
        char preOutId[64];          /*���ܴ�����Կ������*/
        int preOutIdLen;            /*���ܴ�����Կ�����ų���*/
        char orgId[64];             /*������Կ������*/ 
        int orgIdLen;               /*������Կ�����ų���*/
        char workKey[64];           /*���ܺ��ƽ̨������Կ*/
        char verifyKey[64];         /*ƽ̨������ԿУ��ֵ*/
/*�ⲿ������Կ����MAC*/
        char encKey[32];
        char agDebugfile[64];       /* �����ļ��� */
};
