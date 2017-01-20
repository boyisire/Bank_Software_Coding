#define swVdebug(level, ...) _swVdebug(level, __FILE__, __LINE__, __VA_ARGS__)
#define   MSGMAXLEN     8192
int flag=-1;
char    agDebugfile[64];                /* 调试文件名 */
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
/*生成MAC用到的数据*/
        int tran_type;               /*交易类型*/
        int  algoType;               /*密钥算法类型*/
        char subjectID[64];          /*MAC密钥ID*/
        int  subjectIDLen;           /*MAC密钥ID长度*/
        char userData[2048];          /*用户数据*/
        int  userDataLen;            /*用户数据长度*/
        char macData[64];           /*MAC数据*/
        int macDataLen;             /*MAC数据长度*/
/*转加密用的的数据*/
        int srcAlgoType;          /*源加密算法标示*/
        int srcPinType;              /*源PIN运算模式*/
        char srcSubjectID[64];      /*源对称密钥ID*/
        int srcSubjectIDLen;        /*源对称密钥ID长度*/
        int destAlgoType;           /*目的加密算法标示*/
        int destPinType;            /*目的PIN运算模式*/
        char destSubjectID[64];     /*目的对称密钥ID*/
        int destSubjectIDLen ;      /*目的对称密钥ID长度*/
        char userAccount[64];           /*用户账号*/
        int userAccountLen;         /*用户帐号长度*/
        char srcPIN[64];            /*源PIN数据*/
        int srcPINLen;              /*源PIN数据长度*/
        char destPIN[64];           /*目的PIN数据*/
        int destPINLen;             /*目的PIN数据长度*/
	char account_flag[2];       /*判断是不是带账号转加密*/

/*des 转md5用到的数据*/

        int pinType;                /*PIN运算模式*/
        int hashAlgo;               /*摘要算法类型*/  
        char salt[128];             /*摘要因子*/ 
        int saltLen;                /*摘要因子长度*/ 
        char EncSubjectPIN[64];    /*加密的PIN数据*/ 
        int EncSubjectPINLen;       /*加密的PIN数据长度*/
        char hashPIN[64];           /*摘要口令*/
        int hashPINLen;             /*摘要口令长度*/
/*秘钥申请*/
        int workKeyType;            /*平台工作密钥类型*/        
        char preOutId[64];          /*加密传输密钥索引号*/
        int preOutIdLen;            /*加密传输密钥索引号长度*/
        char orgId[64];             /*工作密钥索引号*/ 
        int orgIdLen;               /*工作密钥索引号长度*/
        char workKey[64];           /*加密后的平台工作密钥*/
        char verifyKey[64];         /*平台工作密钥校验值*/
/*外部输入密钥生成MAC*/
        char encKey[32];
        char agDebugfile[64];       /* 调试文件名 */
};
