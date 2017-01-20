#ifndef _swUsrextfun_H
#define _swUsrextfun_H
#endif

#include "swUsrfun.h"

int _extufProPTGETMULDATA(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTCONVERTPIN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProMAC_APPLY(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTCONVERTPINBCD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGET8583MACBUF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGET8583MACBUFCX(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTMACGEN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTAMOUNT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGETFUNDTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGETFORMATMULDATA(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTPORTGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTSLEEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTMACRO(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGETLOANTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGETREMITTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGETFMTMULDATASEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _extufProPTSTRREPLACE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGETMATMULFILTRATE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _extufProPTGETUTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPTGETFMTMULDATASEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);



int _extufProPBGETFOREIGNTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

/*2010-6-27 add by pc 人行函数*/
int _extufProPBTRANISFULL(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/*2011/5/13 add by pc 处理不合法中文*/
int _extufProPBGETVALIDCHN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/*2011/5/23 处理新柜面上送的开通业务标志，使它和旧的系统兼容*/
int _extufProPBGETOPENFLAG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/* 扩展平台函数名列表 电话银行和网银很多公共平台函数相似。此处平台函数名不同，对应函数处理相同*/

int _extufProPTGETFORMATMULDATAEXT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/*add by yz 2011/6/13 14:34:05*/
int _extufProPBTransferIDNO(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProPBGET8583CHECK(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/*add by yz 2011/6/13 14:34:05*/


/*add by pc 2011/6/24 17:00 增加一个字符串替换函数，实现填充字符串*/
int _extufProSTRFILL(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/*add by pc 2011/6/24 17:00*/

int _extufProSETINT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProSETVARSTR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProMD5(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProGETINT(char aParm[][iFLDVALUELEN], short iParmCount, char *aResult);
/*外部输入密钥生成MAC*/
int _extufProMacByKey(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

/********************add by douya 2015-04-17  start************************************/
int shm_ag_province_read(char *p_index,char **p_result);
int shm_ag_err_chg_read(char *pChnlId,char *pErrno, char **p_result);
int _extufProUCS2STR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/********************add by douya 2015-04-17  end**************************************/
int _extufProHASH(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProGETCUSID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/********************add by dxb 20150706  start****************************************/
int _extufProGETINDEXMB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _extufProGETBASEMB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/*******************add by dxb  20150706  end ****************************************/
static PROCLIST extproclist[] = {
       {"PTGETMULDATA", NULL, _extufProPTGETMULDATA},
       {"PTCONVERTPIN", NULL, _extufProPTCONVERTPIN},
       {"PTMACAPPLY", NULL, _extufProMAC_APPLY},
	{"PTMACBYKEY", NULL, _extufProMacByKey},
       {"PTCONVERTPINBCD", NULL, _extufProPTCONVERTPINBCD},
       {"PTGET8583MACBUF", NULL, _extufProPTGET8583MACBUF},
       {"PTGET8583MACBUFCX", NULL, _extufProPTGET8583MACBUFCX},
       {"PTMACGEN", NULL, _extufProPTMACGEN},
       {"PTAMOUNT", NULL, _extufProPTAMOUNT},
       {"PTGETFUNDTRANCODE", NULL, _extufProPTGETFUNDTRANCODE},
       {"PTGETFORMATMULDATA", NULL, _extufProPTGETFORMATMULDATA},
       {"PTPORTGET", NULL, _extufProPTPORTGET},
       {"PTSLEEP", NULL, _extufProPTSLEEP},
       {"PTMACRO", NULL, _extufProPTMACRO},
       {"PTGETLOANTRANCODE", NULL, _extufProPTGETLOANTRANCODE},
       {"PTGETREMITTRANCODE", NULL, _extufProPTGETREMITTRANCODE},
       {"PTGETFMTMULDATASEP", NULL, _extufProPTGETFMTMULDATASEP},
       {"PTSTRREPLACE", NULL, _extufProPTSTRREPLACE},
       {"PTGETMATMULFILTRATE", NULL, _extufProPTGETMATMULFILTRATE},
       {"PTGETUTIME", NULL, _extufProPTGETUTIME},
       {"PTGETFORMATMULDATAEXT", NULL, _extufProPTGETFORMATMULDATAEXT},
	{"PBGETMULDATA", NULL, _extufProPTGETMULDATA},
	{"PBGET8583MACBUF", NULL, _extufProPTGET8583MACBUF},
	{"PBMACGEN", NULL, _extufProPTMACGEN},
	{"PBAMOUNT", NULL, _extufProPTAMOUNT},
	{"PBGETFORMATMULDATA", NULL, _extufProPTGETFORMATMULDATA},
	{"PBGETFORMATMULDATAEXT", NULL, _extufProPTGETFORMATMULDATAEXT},
	{"PBGET8583MACBUFCX", NULL, _extufProPTGET8583MACBUFCX},
	{"PBGETFOREIGNTRANCODE", NULL, _extufProPBGETFOREIGNTRANCODE},
	{"PBTRANISFULL", NULL,_extufProPBTRANISFULL},
	{"PBGETVALIDCHN", NULL,_extufProPBGETVALIDCHN},
	{"PBGETOPENFLAG", NULL,_extufProPBGETOPENFLAG},
{"PBTRANSFERIDNO", NULL,_extufProPBTransferIDNO},
{"PBGET8583CHECK", NULL,_extufProPBGET8583CHECK},

{"STRFILL", NULL,_extufProSTRFILL},

{"SETINT", NULL,_extufProSETINT},
{"SETVARSTR", NULL,_extufProSETVARSTR},
{"MD5", NULL,_extufProMD5},
{"GETINT", NULL, _extufProGETINT},
/********add by douya 2015-04-17***************/
{"UCS2STR",NULL,_extufProUCS2STR},
{"HASH",NULL,_extufProHASH},
{"GETCUSID",NULL,_extufProGETCUSID},
{"GETINDEXMB",NULL,_extufProGETINDEXMB},
{"GETBASEMB",NULL,_extufProGETBASEMB},
/********add by douya 2015-04-17***************/
       PROCLIST_END

};

PROCLIST *pextproclist = extproclist;

