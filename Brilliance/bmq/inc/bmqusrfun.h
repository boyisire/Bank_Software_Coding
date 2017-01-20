#ifndef _swUsrfun_H_
#define _swUsrfun_H_
#endif
#define SUCCESS         0
#define FAIL            -1
#define TRUE            1
#define FALSE           0

/*delete by zcd 20141218
#define LIBXML
***end of delete by zcd 20141218*/

/*add by zcd 20141218*/
#undef LIBXML
/*end of add by zcd 20141218*/

/* 常量定义 */
#define   USERFUNC_NUM   200                 /* 自定义函数个数 */
#define   MAXVARNUM      10                  /* 自定义函数最大参数个数 */
#define   FUNCLEN        20                  /* 函数名的最大长度 */
#define iSHORTLEN  sizeof(short)

#define PROCLIST_END  {"", NULL, NULL}

#include "bmqtype.h"

typedef struct
{
  char  keyword[24];
  char  *(*fproc_old)(char alParm[][iFLDVALUELEN]);
  int   (*fproc)(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
} PROCLIST;

/* 变量定义 */
char    *agMsgbody;
/*short   igMsgbodylen;delete by wh*/
TYPE_SWITCH igMsgbodylen;/*add by wh*/
short   lgMsgoffset;
struct msgpack sgMsgpack;
/*short  igMsglen_pp;delete by wh*/
TYPE_SWITCH igMsglen_pp;/*add by wh*/
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
/*int swFmlpackget(char *aMsgbody, short iMsglen, char *aFldname, short *piFldlen, char *aFldval);delete by wh*/
int swFmlpackget(char *aMsgbody, TYPE_SWITCH iMsglen, char *aFldname, short *piFldlen, char *aFldval);/*add by wh*/
/*short swPacksep(char *aMsgbody, short iMsglen, char *aRecs, char *aGrps, char *aFlds, short iRecid, short iGrpid, short iFldid, char *aFldval);delete by wh*/
short swPacksep(char *aMsgbody, TYPE_SWITCH iMsglen, char *aRecs, char *aGrps, char *aFlds, short iRecid, short iGrpid, short iFldid, char *aFldval);/*add by wh*/
int _swMatch(char *regexpress, char *str);

int _ufNtoT(char *aNstr,short iLen,char *aTstr);
int _ufTtoN(char *aTstr, char *aNstr, short *piLen);

int _ufProNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProPackSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult); 	

int _ufProLFT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProMID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProRIG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSAD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
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

int _ufPropGETSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufPropGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufPropNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProBITGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);


#ifdef LIBXML
/* add by hzl 2003.11.29 */
int _swParesXpath(char *alXpath,char *alXpath_b,char *alXpath_e,int *ilNum);
int _swPreasProp(char *alXpath,char *alXpath_b,char *alXpath_e,int *ilNum,char *alXml);
int _swWeaveField(char *alName,char *alValue);
int _swWeavePro(char *alProname,char *alValue);
int _swRtranslate(char *alValue);
int _swTranslate(char *alValue);
/* end add by hzl */

/* add by hzl 2003.11.29 */
int _ufProGETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGETVALUEM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProADDNODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProGETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProADDPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProXMLINIT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProSETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProXMLFREE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
int _ufProXMLDUMP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult);
/* end add by hzl */

#endif

/* 函数名列表 */
#ifndef _swUsrextfun_H 
static PROCLIST proclist[] = {
       {"NSP", NULL, _ufProNSP},
       {"PACKSEP", NULL, _ufProPackSEP},
       
       {"LFT", NULL, _ufProLFT},
       {"RIG", NULL, _ufProRIG},
       {"MID", NULL, _ufProMID},
       {"SPACE", NULL, _ufProSPACE},
       {"TRIM", NULL, _ufProTRIM},
       {"FILL", NULL, _ufProFILL},
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
       
       {"GETTIME", NULL, _ufProGETTIME},
       {"TODATE", NULL, _ufProTODATE},
       {"TOTIME", NULL, _ufProTOTIME},
       {"LONGTODATE", NULL, _ufProLONGTODATE},
       {"LONGTOTIME", NULL, _ufProLONGTOTIME},			     
       
       {"PID", NULL, _ufProPID},
       {"DTOL", NULL, _ufProDATEtoLONG},

       {"GETFML", NULL, _ufProFmlget},
       
       {"PGETSEP", NULL, _ufPropGETSEP},
       {"PGET", NULL, _ufPropGET},
       {"PNSP", NULL, _ufPropNSP},
       {"BITGET", NULL, _ufProBITGET},
       {"SAD", NULL, _ufProSAD},

       
#ifdef LIBXML       
       {"GETVALUE", NULL, _ufProGETVALUE},    /* add by hzl 2003.11.29 */
       {"GETVALUEM", NULL, _ufProGETVALUEM},    /* add by hzl 2003.11.29 */
       {"GETPROP", NULL, _ufProGETPROP},
       {"XMLINIT",NULL, _ufProXMLINIT},
       
       {"XMLFREE",NULL,_ufProXMLFREE},
#endif

       PROCLIST_END
};
#endif

