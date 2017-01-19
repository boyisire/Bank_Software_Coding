/****************************************************************/
/* ģ����	��DT_Main					*/
/* ģ������	��������					*/
/* �� �� ��	��V1.0						*/
/* ��    ��	��Hu						*/
/* ��������    	��2015/3/6 11:32:47				*/
/* ����޸�����	��						*/
/* ģ����;	��������					*/
/* ���������ӵĿ�˵����						*/
/*	<1>DT_Func_Pub.c	���ù���������			*/
/*	<2>DT_Func_Db.sqc	�������ݿ������		*/
/*	<3>DT_Proc_DayEnd.sqc	���ղ�������ģ��		*/
/*	<4>DT_Proc_Mon.c	��ش���ģ��			*/
/* ��ģ���а������º���������˵����				*/
/*	<01>._bswTrim		ȥ�ո�				*/

/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2015/3/25 21:13:28 ����ģ�������				*/
/* 2015/3/26 21:10:22 ����ʱ�����ģ�����			*/
/* 2015/3/26 21:12:25 ��������ģ�����				*/
/* 2015/3/27 18:13:28 �ļ���ģ�������			*/
/* 2015/3/27 18:17:13 Makefile�������				*/
/* 2015/3/30 18:14:12 ����Ϊ��������ʱ����			*/
/* 2015/3/30 18:14:12 �α��ɾ�̬��Ϊ��̬			*/
/* ��ע��¼��							*/
/* 2015/3/27 18:13:28 ���ģ�������				*/
/* 2015/3/27 18:13:28 �����������ļ�������			*/
/* 2015/3/27 18:13:28 ���ݲ���ȡ������ʽ������			*/
/* 2015/3/27 18:13:28 �˵�����������				*/
/****************************************************************/
#include "DT_Pub.h"


/************************������������**************************/
extern int	glTestFlag;		//���Կ���
extern char	SysDate[10];		//ϵͳʱ��
extern char 	glDB_ServerFlag[2];	//���ݿ��ʶ
extern char	glSql_Sort[10];		//SQL����ʽ
extern char	glCfgFile[50];		//�����ļ�
extern int	glTabFlag;		//�Ƿ��Ե���ģʽִ�п���
extern char	glTabName[50];		//ִ�е���ģʽʱ�ı���


/*�ⲿ��������ڱ������ж���*/
const char* ProgramName;		//������
const char* agDB_ServerFlag=NULL;	//���ݿ��������ʶ
const char* agSql_Sort=NULL;		//SQL����ʽ
const char* agCfgFile=NULL;		//�ⲿ���������ļ���
const char* agTabName=NULL;		//�ⲿ�������
/**************************��������***************************/
//��ӡ����
void M_PrintUsage(FILE* stream, int exit_code);
 

/**************************************************************
 ** ������      : main
 ** ��  ��      : ������
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 16:42:47
 ** ����޸�����: 2015/3/6 16:42:49
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : int argc, char *argv[]
 ** ����ֵ      : FAIL:ʧ��. SUCC:�ɹ�
***************************************************************/
int main(int argc, char *argv[])
{
	/*****��������********/
	
	const char* output_filename = NULL;
	const char* input_filename = NULL;
	
	int	verbose = 0;	//�汾��
	int	DayEndFlag=0;	//�Ƿ�������ղ���
	int	next_option;	//��������
	
	
/*-------------------------------����:getopt_long() ˵��---------------------------------------*/
/*1.һ���ַ�������������Ҫ�Ķ�ѡ���ַ������ѡ����в������ַ����һ��":"���š�
 *	�����У�����ַ���Ӧ��Ϊ"ho:v"��(��Ϊ-o�����в���filename,�����ַ�������Ҫ��":")��
 *2. һ��������ѡ���ַ����Ľṹ�����飬ÿһ���ṹ�����4����:
 *	��һ����Ϊ��ѡ���ַ�����
 *	�ڶ�������һ����ʶ��ֻ��Ϊ0��1���ֱ����û��ѡ�����ѡ�
 *	����������ԶΪNULL��
 *	���ĸ�ѡ����Ϊ��Ӧ�Ķ�ѡ���ַ�����
 *	�ṹ����������һ��Ԫ��ȫ��λNULL��0����ʶ������
 */
	
	//������
	ProgramName = argv[0];	
	//�������
	const char* const short_options = "HDd:s:c:l:Tt:V";	
	//�������
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

	/*-------------------------------------------------------��������ָ���--------------------------------------------Begin*/	
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
		        agTabName=optarg;	//����ģʽ����.		        
		        glTabFlag=1;		//�Ƿ��Ե���ģʽִ��.
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
	/*-------------------------------------------------------��������ָ���--------------------------------------------End*/
		
	int rc=0;
	char alTmp[512];
	char LogFile[128];	//��־�ļ�
	char LogDirs[128];	//��־Ŀ¼ 
        
	/* ��ȡ��ǰϵͳ����YYYYMMDD����SysDate�� */
        time(&t);          
	memset(SysDate, 0x00, sizeof(SysDate));                     
	strftime(SysDate, sizeof(SysDate), "%Y%m%d", localtime(&t));

	/* �����ļ�*/
	memset(glCfgFile, 0x00, sizeof(glCfgFile));
	sprintf(glCfgFile, "%s/etc/HisDayEnd.ini", getenv(DAYEND_ENV));
	
	/* ��־�ļ��� */
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
		_swVdebug(0, "�� DEBUG ����! ʹ��Ĭ��ֵ[5]!��ǰ�����ļ�[%s]",glCfgFile);
		sprintf(alTmp, "5");		
	}
	_bswTrim(alTmp);
	cgDebug = atoi(alTmp);	
	
	if(argc == 1) {
		printf("�ף�����������ʲô�أ�������ʾ�£�\n��Ҫ������ʹ�ã�%s --Help \n",argv[0]);
		return FAIL;
	}else{
		_swVdebug(0, "��ǰ��־�ļ�Ϊ[%s]   ��־����Ϊ[%d]!",LogFile,cgDebug);
		_swVdebug(0, "��ǰ�����ļ�Ϊ[%s]",glCfgFile);
		printf("��ʼ\n");
		/*��ʷ���մ������*/
		if (DayEndFlag){
			_swVdebug(0, "-------------------->>>���ճ���ִ�п�ʼ<<<---------------------------");
			rc=Proc_DayEndMain();
		 	_swVdebug(0, "-------------------->>>���ճ���ִ�н���.����ֵ��%d��<<<---------------\n",rc);
		 }	
		 printf("����\n");
		/*�˵�ģʽ*/
		/*����SQL��ݴ���*/
	}
	
	return SUCC;
}


/**************************************************************
 ** ������      : M_PrintUsage
 ** ��  ��      : ��ӡ������Ϣ
 ** ��  ��      : Hu
 ** ��������    : 2015/3/6 16:42:47
 ** ����޸�����: 2015/3/6 16:42:49
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : FAIL:ʧ��. SUCC:�ɹ�
***************************************************************/
void M_PrintUsage(FILE* stream, int exit_code)
{
    fprintf(stream, "Usage: %s Options [InputInfo]\n",ProgramName);
    fprintf(stream, 
        "	-H	--Help					������Ϣ.\n"
        "	-D	--DayEnd				ִ����ʷ���ճ���.\n"
        "	-d	--dbFlag	Flag			���ݿ��ʶ[A | B | H | Y | X-��ǰ����].\n"
        "	-T	--Test					����ģʽִ��(ֻ��ӡִ����䣬����ʵ�ʲ���).\n"
        "	-t	--tab					���������ȡ�����ļ��еı���,ִ����ز���.\n"
        "	-s	--sqlSort	SortMode		�α�����ʽ[DESC|ASC].\n"
        "	-c	--agCfgFile	FileName		��ȡ�����ļ�[����·��].\n"
        "	-l	--logLevel	Level			��־����[0-5].\n"
        "	-V	--Version				��ȡ��ǰ����汾��.\n");
    exit (exit_code);
}
