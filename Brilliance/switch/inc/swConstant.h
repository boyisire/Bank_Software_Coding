#ifndef  _swConstant_H_
#define  _swConstant_H_

/*#define DB_INFORMIX
#undef  DB_ORACLE
#undef  DB_DB2       */ 

/* ϵͳԼ�� */
#define SUCCESS		0
#define FAIL		-1
#define TRUE		1
#define FALSE		0
#define SHMFULL		9	/* �����ڴ����� */

/* D601�������� */
#define iMSGAPP		1
#define iMSGREV		51
#define iMSGREVEND	52
#define iMSGREVFAIL	53
#define iMSGUNPACK	101
#define iMSGPACK	102
#define iMSGREVUNPACK	111
#define iMSGREVPACK	112
#define iMSGSAVEHEAD	121
#define iMSGLOADHEAD	122
#define iMSGROUTE	201
#define iMSGMESSAGE	901
#define iMSGORDER	902
#define iMSGWINDOW	903
#define iMSGRETURN	904

/* D602��ʽת����ʽ */
#define cCONVYES	'1'
#define cCONVNO		'0'

/* D603·������ʽ */
#define cROUTEYES	'1'
#define cROUTENO	'0'

/* D604�����ַ��� */
#define cASCASC		'0'
#define cBCDBCD		'1'
#define cBCDASC		'2'
#define cASCBCD		'3'

/* D605������ˮ����״̬ */ 
#define iTRNING		0
#define iTRNEND		1
#define iTRNOVERTIME	2
#define iTRNREVING	3
#define iTRNREVEND	4
#define iTRNREVFAIL	5
#define iTRNREVWAIT	6
#define iTRNSAF		7

/* D606���ĸ�ʽ���� */
#define iFMTIMF		1
#define iFMTNOSEP	2
#define iFMTSEP		3
#define iFMT8583	4
#define iFMTXMF		5
#define iFMT8583E	6	/* added by fzj at 2002.01.23 */
#define iFMTUNKNOW	99

/* D607�ڲ���׼��������� */
#define cTYPESTRING	'1'
#define cTYPEINT	'2'
#define cTYPEFLOAT	'3'
#define cTYPEDATE	'4'

/* D608��Ϣ���ݴ��� */
#define cMSGOUT		'1'
#define cMSGIN		'2'
#define cMSGREVOUT	'3'
#define cMSGREVIN	'4'

/* D609��ʽת�����ʹ��� */
#define iIMF_IMF	0
#define iNOSEP_IMF	1
#define iSEP_IMF	2
#define i8583_IMF	3
#define iXMF_IMF	4
#define i8583E_IMF	5	/* added by fzj at 2002.01.23 */
#define iIMF_NOSEP	101
#define iIMF_SEP	102
#define iIMF_8583	103
#define iIMF_XMF	104
#define iIMF_8583E	105	/* added by fzj at 2002.01.23 */

/* D610�߼����� */
#define cTRUE		'1'
#define cFALSE		'0'

/* D611������ʽ */
#define cOTREVYES	'1'
#define cOTREVNO	'2'

/* D612������� */
#define cREVING		'0'
#define cREVSUCCESS	'1'
#define cREVFAIL	'2'

/* D614����״̬��ʶ */
#define cTASKRUNING	'1'
#define cTASKRUNED	'2'
#define cTASKDOWNING	'3'
#define cTASKDOWN	'4'
#define cTASKERROR	'5'

/* D615 ISO8583���ʽ���� */
enum ISO8583FieldType
{
  Efn = 1,
  Efb,
  Efan,
  Efxn,
  Efans,
  Eln,
  Elln,
  Ellan,
  Ellln,
  Elans,
  Ellans,
  Elllan,
  Elllans,
  Ellz,
  Elllz,
/* add by qy 2001.09.19 */
  Elllln,
  Ellllan,
  Ellllans,
  Ellllz
/* end of add by qy */
};

/* added by fzj at 2002.01.23, begin */
/* ISO8583E�����Ͷ��� */
enum ISO8583EFieldType
{
  EA = 0x0001,	/* ��ĸ */
  ED = 0x0002,	/* ���� */
  EP = 0x0004,	/* �ɴ�ӡ�ַ� */
  ES = 0x0008,	/* �����ַ� */
  EB = 0x0010,	/* �������� */
  EX = 0x0020,	/* C/D+��� */
  EZ = 0x0040,	/* �ŵ����뼯 */
  EV = 0x8000	/* �ɱ䳤�� */
};
/* added by fzj at 2002.01.23, end */

/* D616 ���������ʽ���� */
#define cREVREQUEST	'1'
#define cREVRESPOND	'2'

/* D617 ·������ʶ */
#define cROUTECALL	'1'
#define cROUTESEND	'2'
#define cROLLBACK	'3'
#define cGOTO		'4'
#define cFMLSET		'5'
#define cEXECSQL	'6'
#define cFTPPUT 	'7'
#define cFTPGET 	'8'
#define cQPUT	        '9'
#define cQGET   	'a'
#define cDO             'Z'  /* add by zjj 2001.11.07 for do operation */

/* D618 �˿�״̬ */
#define cPORTUP		'1'
#define cPORTDOWN	'0'

/* D619 ����ƽ̨�ڲ��������� */
#define iMBECHOTEST     5   /* add by qy 2001.09.03 */
#define iMBSYSSHW       6
#define iMBFILETRAN     7
#define iMBDBSQL	8
#define iMBFILE 	9
#define iMBCTRL		11
#define iMBFORMAT	12
#define iMBROUTER	13
#define iMBMONREV	14
#define iMBTASK		15
#define iMBSYSMON	16
#define iMBSYSCON	17
#define iMBSYSCOM	18
#define iMBWINMNG	19
#define iMBWINMON	20

/* D620 ģ���ʶ */
#define iAPPCTL		1
#define iAPPFORMAT	2
#define iAPPROUTER	3
#define iAPPMONLOG	4
#define iAPPMONSAF	5
#define iAPPMONREV	6
#define iAPPTASK	7
#define iAPPSYSMON	8
#define iAPPSYSCON	9
#define iAPPSYSCOM	10
#define iAPPDB		11
#define iAPPFILE        12
#define iLOGSIZE	99

/* D623 ��Ϣ�����¼���ʶ */
#define iMSGTRANREV	1
#define iMSGTRANSMIT	2

/* D621 ϵͳ����Լ������ */
/***delete by wanghao 20141215*/
/*#define iMSGMAXLEN	8192 */	/* ������󳤶� */

#define iMSGMAXLEN  65536   /* ������󳤶� */
#define iBUFFLDNUM	500	/* Ԥ������� */


#define  iTASKTIMEOUT   10      /* �������ѯ���ʱ��(��) */

#define iKEYNUMBER      4       /* �ļ��йؼ��ָ���   */
/*#define iFLDNUMBER	30 */   /* �ļ�����������     */ /* modify by zjj 2002.01.30 */
#define iFLDNUMBER	50      /* �ļ�����������     */

#define iMAXQUEUEREC	200     /* swt_sys_queue  ��Ԥ���ռ� */
#define iMAXIMFREC	100     /* swt_sys_imf  ��Ԥ���ռ� */
#define iMAXTASKREC	50	/* swt_sys_task ��Ԥ���ռ� */
#define iMAXPROCNUM	10      /* swt_proc_log ��Ԥ���ռ� */

#define iMAXTRANLOGREC	1000    /* swt_tran_log ��Ԥ���ռ� lsj��*/
#define iMAXREVSAFREC	500     /* swt_rev_saf  ��Ԥ���ռ� lsj��*/

#define SHMNOTFOUND	100     /* û�ҵ������ڴ��¼ */
#define SHMFILEEND	1

/* for prep */
#define cppUNPACK	'1'
#define cppPACK		'2'
#define cppNEWTRAN	'3'
#define cppLOADHEAD	'4'
#define cppSAVEHEAD	'5'
#define cppREVISE	'6'
#define cppSQL		'7'
#define cppSEND		'8'
#define cppGOTO		'9'
#define cppRETURN	'a'
#define cppOTHER	'b'
#define cppCALLSERV     'c'
#define cppBEGINWORK    'd'
#define cppBEGINCALL    'e'
#define cppCOMMITWORK   'f'
#define cppCOMMITCALL   'g'
#define cppROLLBACKWORK 'h'
#define cppROLLBACKCALL 'i'

#define iFLDNAMELEN	40	/* ������󳤶� */ /* modi 2004/3/31 */
#define iSHORTLEN	sizeof(short)	/* ������󳤶� */ /* modi 2004/3/31 */
#define iINTLEN		sizeof(int)	/* ������󳤶� */ /* modi 2004/3/31 */
#define iLONGLEN	sizeof(long)	/* ������󳤶� */ /* modi 2004/3/31 */
#define iUNINTLEN	sizeof(unsigned int)  /* ������󳤶� *//* modi 2014/12/30 zcd */

#define iPERFLDVALUELEN	512	/* ÿ����ֵ��󳤶� */
#define iFLDVALUELEN	2048	/* ��ֵ��󳤶� */
/*#define iEXPRESSLEN	152*/
#define iEXPRESSLEN	512	/* ƽ̨���ʽ���� */
#define SHMKEYLOGID	(key_t)81	/* ������ˮ�Ź����ڴ��ֵ */
#define SHMKEYSAFID	(key_t)83	/* SAF��ˮ�Ź����ڴ��ֵ */
#define SHMTASKLIST	(key_t)84	/* ��������ڴ��ֵ */
#define SHMKEYPORTID	(key_t)85	/* �˿ڱ����ڴ��ֵ */
#define SHMPERM		0666		/* �����ڴ�Ȩ�� */
#define SHMBUFLENLOGID	10		/* ������ˮ�Ź����ڴ��С */
#define SEMKEYLOGID	0x222		/* ������ˮ���ź�����ֵ */
#define SEMKEYSAFID	0x223		/* SAF��ˮ���ź�����ֵ */

#define SEMPERM		0666		/* �ź���Ȩ�� */
#define SEMKEYTRANLOG	0x205		/* ������־�ļ��ź�����ֵ */
#define SEMKEYREVLOG	0x206		/* ������Ӧ��־�ļ��ź�����ֵ */
#define SEMKEYPORTID	0x207		/* �˿�ͳ�Ʊ��ź�����ֵ */

#define iMSGHEADLEN	sizeof(struct msghead)	/* ����ͷ���� */
/*modified by cjh 20150408 ��ˮ������32λ�汾 8��9��Ϊ12��9 */
/*#define lMAXSERNO	99999999L*/   /* �����ˮ�� */
#define lMAXSERNO	999999999999L /* �����ˮ�� */
#define iISOFLDNUM	128		/* ISO8583��֧����������� */

#ifdef DB_INFORMIX
#ifndef  SQLNOTFOUND
#define SQLNOTFOUND 100
#endif
#define SQLDEADLOCK -911
#define SQLDUPKEY -100
#define SQLDUPKEY2 -268
#define SQLDUPKEY3 -239
#define SQLDBERROR 329
#endif

#ifdef DB_ORACLE
#define SQLNOTFOUND 1403
#define SQLDEADLOCK -911
#define SQLDUPKEY -1
#define SQLDBERROR -1041
#endif

#ifdef DB_SYBASE
#define SQLNOTFOUND 100
#define SQLDEADLOCK -911
#define SQLDUPKEY -2601
#define SQLDBERROR -33620275
#endif

/* add by zhanghui 2001/4/2 */

#ifdef DB_DB2
#define  SQLNOTFOUND 100
#define SQLDEADLOCK -911
#define SQLDUPKEY -803
#define SQLDBERROR -1032
#endif

#ifdef DB_ALTIBASE           
#define  SQLNOTFOUND 100
#define SQLDEADLOCK -911
#define SQLDUPKEY -803
#define SQLDBERROR -1
#endif

#ifdef DB_POSTGRESQL
#define SQLNOTFOUND 100
#define SQLDEADLOCK -911
#define SQLDUPKEY -100
#define SQLDUPKEY2 -268
#define SQLDBERROR 329
#endif

/* add by zhanghui 2001/4/2 */

#define BMQ__NOMOREMSG  1001
#define BMQ__TIMEOUT    1006

/*��־���*/
#define LOGFILESIZE 1048576L
#define SW_SYS_LOGFILE_NAME  "swSyslog.debug"
#endif
