#ifndef _swUsrfun_H_
#define _swUsrfun_H_
#endif

/*delete by zcd 20141218
#define LIBXML2
***end of delete by zcd 20141218*/

/*add by zcd 20141218*/
#define LIBXML2
/*end of add by zcd 20141218*/

/*#define	TUXEDO*/

/* 常量定义 */
#define   USERFUNC_NUM   200                 /* 自定义函数个数 */
#define   MAXVARNUM      10                  /* 自定义函数最大参数个数 */
#define   FUNCLEN        20                  /* 函数名的最大长度 */

#define PROCLIST_END  {"", NULL, NULL}

typedef struct
{
  char  keyword[24];
  char  *(*fproc_old)(char alParm[][iFLDVALUELEN]);
  int   (*fproc)(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
} PROCLIST;

/* 变量定义 */
union  preunpackbuf psgPreunpackbuf[iBUFFLDNUM]; /* TDF转换后的缓冲池 */
union  preunpackbuf psgUnpackbuf[iBUFFLDNUM];	 /* TDF转换后的缓冲池 */
char   cgPreunpackflag;
struct msgpack sgMsgpack;
short  igMsglen_pp;
short  igFuncCnt;
short  igExtfuncCnt;

/* 函数原型 */
int _swExpress(char *aExpress, char *aResult, short *piResultlen);
int _swExpN2T(char *aNstr,char *aTstr);
int _swExpT2N(char *aTstr,char *aNstr);
int _swParseN2T(char *aResult,char *aStr);
int _swParseT2N(char *aResult,char *aStr);
int _swGetFuncAndParmN2T(char *aStr,char *aFunc,char aParm[][iFLDVALUELEN]);
int _swGetFuncAndParmT2N(char *aStr,char *aFunc,char aParm[][iFLDVALUELEN]);
int _swUsrfunSort();
int _swGetfuncPtr(char *aFunc);
int _swGetextfuncPtr(char *aFunc);

int _ufNtoT(char *aNstr,short iLen,char *aTstr);
int _ufTtoN(char *aTstr, char *aNstr, short *piLen);

int _ufProFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/* add by bmy 2002.8.1 */
int _ufProUFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProPackSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult); 	
int _ufProISO(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFLD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMSGHEAD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProVAR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProNFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMVAR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProREC(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMEMOSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProLFT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProRIG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSAD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProREP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFILL(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSPACE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTRIM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTRIML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTRIMR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProLEN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProADD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSUB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMULTI(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProDIV(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMOD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProMAT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSEQ(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProLT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProLE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProNE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProIF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProCASEIF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProCASEVAR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProNOT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProAND(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProOR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProHEX(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProCHR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSHORT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProINT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProLONG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProDOUBLE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProDTOC(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProUSHORT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProULONG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProGETTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTODATE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTOTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProPID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGETSTR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGETMSG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProDATEtoLONG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);         
int _ufProLONGTODATE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProLONGTOTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProFmlget(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFmlset(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTmpget(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTmpset(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGetmsghead(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSetmsghead(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufPropGETSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufPropSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufPropSETR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult); /* add 2002.8.1 */
int _ufPropGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufPropNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProVARID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProBITGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProBITSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProBITMAP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFMLPACKSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProA2E(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProE2A(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProA2B(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProB2A(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProLA2B(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

/* next 3 row add by nh */
int _ufProPORTSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProPORTGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProXMLDUMP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

#ifdef LIBXML
/* add by hzl 2003.11.29 */
int _swParesXpath(char *alXpath,char *alXpath_b,char *alXpath_e,int *ilNum);
int _swPreasProp(char *alXpath,char *alXpath_b,char *alXpath_e,int *ilNum,char *alXml);
int _swWeaveField(char *alName,char *alValue);
int _swWeavePro(char *alProname,char *alValue);
int _swRtranslate(char *alValue);
int _swTranslate(char *alValue);
/* end add by hzl */
/*add by zcd 20141218*/
#endif

#ifdef LIBXML2
/*LIBXML2不需要以上函数，因此在ifdef LIBXML2中未定义以上几个函数*/
/*end of add  by zcd 20141218 */
/* add by hzl 2003.11.29 */
int _ufProGETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGETVALUEM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProADDNODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProADDNODE_X(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProADDPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProXMLINIT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProXMLFREE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/* end add by hzl */

#endif

/*add by pc 支持TUXEDO FML报文	2008-2-10 20:50*/
#ifdef TUXEDO
int _ufProTPALLOC(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFADD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFADD32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFCHG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFCHG32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFGET32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFDEL32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFNUM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFNUM32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProTPFREE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProFOCCUR32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/*add by gsd 支持储蓄逻辑中*/

int _ufProPIFCHG32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProPIFGET32(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProPINOTEINFO(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
#endif
/*add by pc 支持TUXEDO FML报文	2008-2-10 20:50*/
int _ufProUPPER(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProLOWER(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

int _ufProCUSTONEW(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);

/* 函数名列表 */
#ifndef _swUsrextfun_H 
static PROCLIST proclist[] = {
       {"FML", NULL, _ufProFML},
       {"UFML", NULL, _ufProUFML}, /* add by bmy 2002.8.1 */
       {"NSP", NULL, _ufProNSP},
       {"SEP", NULL, _ufProSEP},
       {"PACKSEP", NULL, _ufProPackSEP},
       {"ISO", NULL, _ufProISO},
       {"MSGHEAD", NULL, _ufProMSGHEAD},
       {"VAR", NULL, _ufProVAR},
       {"MVAR", NULL, _ufProMVAR},
       {"MFML", NULL, _ufProMFML},
       {"NFML", NULL, _ufProNFML},
       {"REC", NULL, _ufProREC},
       {"FLD", NULL, _ufProFLD},
       {"MEMOSET", NULL, _ufProMEMOSET},

       {"LFT", NULL, _ufProLFT},
       {"RIG", NULL, _ufProRIG},
       {"MID", NULL, _ufProMID},
       {"SPACE", NULL, _ufProSPACE},
       {"TRIM", NULL, _ufProTRIM},
       {"FILL", NULL, _ufProFILL},
       {"REP", NULL, _ufProREP},
       {"LEN", NULL, _ufProLEN},
       {"TRIML", NULL, _ufProTRIML},
       {"TRIMR", NULL, _ufProTRIMR},
       
       {"SEQ", NULL, _ufProSEQ},
       {"GE", NULL, _ufProGE},
       {"GT", NULL, _ufProGT},
       {"LE", NULL, _ufProLE},
       {"LT", NULL, _ufProLT},
       {"NE", NULL, _ufProNE},
       {"IF" , NULL, _ufProIF},
       {"NOT", NULL, _ufProNOT},
       {"AND", NULL, _ufProAND},
       {"OR" , NULL, _ufProOR },
       {"MAT", NULL, _ufProMAT},

       {"ADD", NULL, _ufProADD},
       {"SUB", NULL, _ufProSUB},
       {"MULTI", NULL, _ufProMULTI},
       {"DIV", NULL, _ufProDIV},
       {"MOD", NULL, _ufProMOD},

       {"CASEIF", NULL, _ufProCASEIF},
       {"CHR", NULL, _ufProCHR},
       {"CASEVAR", NULL, _ufProCASEVAR},
       {"HEX", NULL, _ufProHEX},
       {"SHORT", NULL, _ufProSHORT},
       {"USHORT", NULL, _ufProUSHORT},
       {"INT", NULL, _ufProINT},
       {"LONG", NULL, _ufProLONG},
       {"ULONG", NULL, _ufProULONG},
       {"DOUBLE", NULL, _ufProDOUBLE},
       {"DTOC", NULL, _ufProDTOC},

       {"GETTIME", NULL, _ufProGETTIME},
       {"TODATE", NULL, _ufProTODATE},
       {"TOTIME", NULL, _ufProTOTIME},
       {"LONGTODATE", NULL, _ufProLONGTODATE},
       {"LONGTOTIME", NULL, _ufProLONGTOTIME},			     
       
       {"PID", NULL, _ufProPID},
       {"GETMSG", NULL, _ufProGETMSG},
       {"GETSTR", NULL, _ufProGETSTR},
       {"DTOL", NULL, _ufProDATEtoLONG},

       {"GETFML", NULL, _ufProFmlget},
       {"SETFML", NULL, _ufProFmlset},
       {"GETVAR", NULL, _ufProTmpget},
       {"SETVAR", NULL, _ufProTmpset},
       {"GETHEAD", NULL, _ufProGetmsghead},
       {"SETHEAD", NULL, _ufProSetmsghead},

       {"PGETSEP", NULL, _ufPropGETSEP},
       {"PSET", NULL, _ufPropSET},
       {"PSETR", NULL, _ufPropSETR}, 	/* add by bmy 2002.8.1 */
       {"PGET", NULL, _ufPropGET},
       {"PNSP", NULL, _ufPropNSP},
       {"VARID", NULL, _ufProVARID},
       {"BITSET", NULL, _ufProBITSET},
       {"BITGET", NULL, _ufProBITGET},
       {"BITMAP", NULL, _ufProBITMAP},
       {"FMLPACKSET", NULL, _ufProFMLPACKSET},
       {"A2E", NULL, _ufProA2E},
       {"E2A", NULL, _ufProE2A},
       
       {"A2B", NULL, _ufProA2B},
       {"B2A", NULL, _ufProB2A},
       {"LA2B", NULL, _ufProB2A},
       {"SAD", NULL, _ufProSAD},

       {"PORTSET", NULL, _ufProPORTSET},
       {"PORTGET", NULL, _ufProPORTGET},
/*delete by zcd 20141218
#ifdef LIBXML    
***end of delete by zcd 20141218*/

/*add by zcd 20141218*/
#ifdef LIBXML2
/*end of add  by zcd 20141218 */
   
       {"GETVALUE", NULL, _ufProGETVALUE},    /* add by hzl 2003.11.29 */
       {"GETVALUEM", NULL, _ufProGETVALUEM},    /* add by hzl 2003.11.29 */
       {"ADDNODE", NULL, _ufProADDNODE},
       {"ADDNODE_X", NULL, _ufProADDNODE_X},
       {"SETVALUE", NULL, _ufProSETVALUE},
       {"GETPROP", NULL, _ufProGETPROP},
       {"ADDPROP", NULL, _ufProADDPROP},
       {"SETPROP", NULL, _ufProSETPROP},
       {"XMLINIT",NULL, _ufProXMLINIT},
       
       {"XMLDUMP",NULL,_ufProXMLDUMP},
       {"XMLFREE",NULL,_ufProXMLFREE},
#endif

/*add by pc 支持TUXEDO FML报文	2008-2-10 20:50*/
#ifdef TUXEDO
       {"TPALLOC", NULL, _ufProTPALLOC},
       {"FADD", NULL, _ufProFADD},
       {"FADD32", NULL, _ufProFADD32},
       {"FCHG", NULL, _ufProFCHG},
       {"FCHG32", NULL, _ufProFCHG32},
       {"FGET", NULL, _ufProFGET},
       {"FGET32", NULL, _ufProFGET32},
       {"FNUM", NULL, _ufProFNUM},
       {"FNUM32", NULL, _ufProFNUM32},
       {"FOCCUR32", NULL, _ufProFOCCUR32},       
/*add by gsd 支持储蓄逻辑中*/

       {"PIFCHG32", NULL, _ufProPIFCHG32},
       {"PIFGET32", NULL, _ufProPIFGET32},
       {"PINOTEINFO", NULL, _ufProPINOTEINFO},
       {"TPFREE", NULL, _ufProTPFREE},
#endif
/*add by pc 支持TUXEDO FML报文	2008-2-10 20:50*/
       {"UPPER", NULL, _ufProUPPER},       
       {"LOWER", NULL, _ufProLOWER},
       {"CUSTONEW", NULL, _ufProCUSTONEW},
       PROCLIST_END
};
#endif

extern PROCLIST *pextproclist;
