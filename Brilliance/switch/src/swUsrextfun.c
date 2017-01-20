/****************************************************************/
/* 模块编号：USREXTFUN                                          */
/* 模块名称：扩展平台函数                                       */
/* 作	 者：                                                   */
/* 建立日期：2001/11/28                                         */
/* 修改日期：2001/11/28                                         */
/* 模块用途：实现对扩展函数的处理                               */
/* 本模块中包含如下函数及功能说明：                             */
/****************************************************************/
/*           int  _ufUsrextfunDemo();                           */
/****************************************************************/
/* 修改记录：                                                   */
/****************************************************************/
/* switch定义 */
#include <openssl/md5.h>
#include "switch.h"
#include "swUsrextfun.h"
#include "swUsrextfunapi.h"
#include "swNdbstruct.h"
#include "swShm.h"

#include "swShmGroup.h"

/********add by douya 20150417 start *******/
#include "appShmDef.h"
#include "swxml.h"
/********add by douya 20150417 end   *******/



#define  iMAXRECORD      3000

int igCodeflag=-1;
int igFormatHsmFlag=0;



typedef struct {
	char section[32];
	int start;
	int end;
} SECTION;

typedef struct {
	char code[20];
	char chinese[50];
} DETAIL;

typedef struct {
	int sectiontotal;
	SECTION section[50];
	DETAIL detail[500];
}SCODE;

SCODE sgCode;


int UnpackBCD( unsigned char *InBuf,  char *OutBuf, int Len )
{
	int rc = 0;
	unsigned char ch;
	register int i, active = 0;
	for ( i = 0; i < Len; i++ )
	{
		ch = *InBuf;

		if ( active )
		{
			(*OutBuf=(ch&0xF))<10 ? (*OutBuf+='0') : (*OutBuf+=('A'-10));
			InBuf++;
		}
		else
		{
			(*OutBuf=(ch&0xF0)>>4)<10 ? (*OutBuf+='0') : (*OutBuf+=('A'-10));
		}
		active ^= 1;
		if ( !isxdigit ( *OutBuf ) )	/* validate character */
		{
			rc = -1;
			break;
		}
		OutBuf++;
	}
	*OutBuf = 0;
	return ( rc );
}

int PackBCD( char *InBuf, unsigned char *OutBuf, int Len )
{
	int	    rc;		/* Return Value */
	register int     ActiveNibble;	/* Active Nibble Flag */
	char     CharIn;	/* Character from source buffer */
	unsigned char   CharOut;	/* Character from target buffer */

	rc = 0;		/* Assume everything OK. */
	ActiveNibble = 0;	/* Set Most Sign Nibble (MSN) */
	/* to Active Nibble. */

	for ( ; (Len > 0); Len--, InBuf++ )
	{
		CharIn = *InBuf;

		if ( !isxdigit ( CharIn ) )	/* validate character */
		{
			rc = -1;
		}
		else
		{
			if ( CharIn > '9')
			{
				CharIn += 9;	/* Adjust Nibble for A-F */
			}
		}
		if ( rc == 0 )
		{
			CharOut = *OutBuf;
			if ( ActiveNibble )
			{
				*OutBuf++ = (unsigned char)( ( CharOut & 0xF0) |
				    ( CharIn  & 0x0F)   );
			}
			else
			{
				*OutBuf = (unsigned char)( ( CharOut & 0x0F)   |
				    ( (CharIn & 0x0F) << 4)   );
			}
			ActiveNibble ^= 1;	/* Change Active Nibble */
		}
	}
	return rc;
}



/**************************************************************
 ** 函 数 名：_ufUsrextfunDemo
 ** 功    能：扩展平台函数范例 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： 
 ** 返 回 值： 0
***************************************************************/
int _ufUsrextfunDemo(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	char alTmp[iFLDVALUELEN];

	strcpy(alTmp,"this is a demo of usrextfun.");
	ilLen = strlen(alTmp);
	_ufNtoT(alTmp,ilLen,aResult);

	return(0);
}


void _GetNameByCode(char *section,char *code,char *aResult)
{
	int start;
	int end;
	int i;
	int ilRc;
	
	strcpy(aResult,"\0");
	
	start=0;
	end=0;
	for (i=0;i<sgCode.sectiontotal;i++){
swVdebug(2,"section[%d][%s]start[%d]end[%d]",i,sgCode.section[i].section,sgCode.section[i].start,sgCode.section[i].end);
		if (strcmp(sgCode.section[i].section,section)==0) {
			start=sgCode.section[i].start;
			end=sgCode.section[i].end;
			break;
		}	
	}
	if (end>0) {
		for (i=start;i<=end;i++){
swVdebug(2,"code[%d][%s]",i,sgCode.detail[i].code);
			
			if (strcmp(sgCode.detail[i].code,code)==0) {
				strcpy(aResult,sgCode.detail[i].chinese);
				break;
			}	
		}	
	}
	return;
}



#if 0
/**************************************************************
 ** 函 数 名：_extufProPTSHMGETNAMEBYCODE
 ** 功    能：通过代码取得中文解释 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 章节名字
 **            aParm[1] code代码
 ** 返 回 值： 0 成功 aResult 返回的中文解释
***************************************************************/
int _extufProPTSHMGETNAMEBYCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char code[32+1];
	char section[32+1];
	int start;
	int end;
	int i;
	
	strcpy(aResult,"\0");
	if (iParmCount!=2) {
		swVdebug(0,"S9001:_extufProPTSHMGETNAMEBYCODE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	_GetShmNameByCode(aParm[0],aParm[1],aResult);
	swVdebug(3,"S8273: [函数返回] _extufProPTSHMGETNAMEBYCODE()返回码=0,结果=[%s]",aResult);
	return(0);
}

#endif


/*根据传入的格式串处理数据*/
void _DealOneData(char *aOneData,char *aFormat){
	char aTmp[32];
	char aData[1024];
	char alFormat[32];
	char *p;
	int iFlag;
	int formatlen=0;
	int ilRc;
	int start;
	int end;
		
	memset(aTmp,0x00,sizeof(aTmp));
	memset(aData,0x00,sizeof(aData));
	_swTrim(aOneData);
	strcpy(aData,aOneData);
	strcpy(aTmp,aFormat);
	_swTrim(aTmp);
	if (strlen(aTmp)==0) aTmp[0]='N';
swVdebug(2,"add by pc aFormat[%s]",aFormat);	
	p=strchr(aFormat,':');
	if (p!=NULL) {
		formatlen=strlen(aFormat)-(p-aFormat)-1;
		memset(alFormat,0x00,sizeof(alFormat));
		memcpy(alFormat,p+1,formatlen);
	}	
swVdebug(2,"type[%c]format[%s]",toupper(aTmp[0]),alFormat);	
	switch (toupper(aTmp[0])){
		case 'M':
			if (formatlen==0) strcat(aOneData,"|");
			else {
				_swTrim(aData);
				_DealAmount(aData);
				ilRc=swAmount(aOneData,aData,alFormat);	
				if (ilRc) {
					swVdebug(0,"S9001:swAmount 传入参数[%s] [%s]不合法，未处理数据",aData,alFormat);
					strcat(aOneData,"|");
					break;
				}
				_swTrim(aOneData);
				strcat(aOneData,"|");
			}		
			break;
		case 'C':
			if (formatlen==0) strcat(aOneData,"|");
			else {
				/*_GetNameByCode(alFormat,aOneData,aData);
				_GetShmNameByCode(alFormat,aOneData,aData);
				strcpy(aOneData,aData);*/
				strcat(aOneData,"|");
			}				
			break;
		case 'A':
			if (formatlen==0) strcat(aOneData,"|");
			else {
				/*_GetNameByCode(alFormat,aOneData,aData);
				_GetShmNameByCode(alFormat,aOneData,aData);
				strcat(aOneData,"|");				
				strcat(aOneData,aData);*/
				strcat(aOneData,"|");
			}				
			break;
		case 'R':
			/*实现增加一列，同时后面不加竖线*/
			if (formatlen==0) {
				/*strcat(aOneData,"|");*/
			}
			else {
				/*_GetNameByCode(alFormat,aOneData,aData);
				_GetShmNameByCode(alFormat,aOneData,aData);
				strcat(aOneData,"|");				
				strcat(aOneData,aData);*/
				/*strcat(aOneData,"|");*/
			}				
			break;			
		case 'N':
			if (formatlen==0) strcat(aOneData,"|");
			else {
				_swTrim(aData);
				_DealAmount(aData);

				memset(aTmp,0x00,sizeof(aTmp));
				
/*				swVdebug(5,"aData[%s]",aData);*/
				if (aData[0]=='-'){
					strcpy(aTmp,"-");
					aData[0]='0';
				}
				else {
					strcpy(aTmp,"+");
				}		
				ilRc=swAmount(aOneData,aData,alFormat);	
				if (ilRc) {
					swVdebug(0,"S9001:swAmount 传入参数[%s] [%s]不合法，未处理数据",aData,alFormat);
					strcat(aOneData,"|");
					break;
				}
/*				swVdebug(5,"aOneData[%s]aData[%s]alFormat[%s]",aOneData,aData,alFormat);*/
				_swTrim(aOneData);
				if (aTmp[0]=='-'){
					strcpy(aData,aOneData);
					sprintf(aOneData,"-%s|",aData);
				}	
				else	
					strcat(aOneData,"|");
			}		
			break;			
		default:
			strcat(aOneData,"|");
			break;
	}
	return;
}
/**************************************************************
 ** 函 数 名：_extufProPTGETMULDATA
 ** 功    能：处理多笔明细数据(规则，根据传入参数格式化数据) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 要处理的字符串buff
 **            aParm[1] 一共要处理多少列
 **            aParm[2] 一共要处理多少行数据 
 **            aParm[3] 各列宽度(用竖线分隔，例如 有3列 12|20|30|)
 ** 返 回 值： 0 成功 aResult 处理后的多笔明细
***************************************************************/
int _extufProPTGETMULDATA(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int i,j;
	int col;
	int row;
	int colwidth[100];
	char alTmp[iFLDVALUELEN];
	char aColWidth[256];
	char aColWidth2[256];
	char aDealStr[iFLDVALUELEN];
	char aOneData[512];	
	char aFormat[4];
	short iTotalLen=0;
	short iNeedLen=0;

	char aTmp[32];
	char *p;
	int spos;
	int dpos;
	if (iParmCount!=4) {
		strcpy(aResult,"\0");
		swVdebug(0,"S9001:_extufProPTGETMULDATA 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	swVdebug(2,"S9001:_extufProPTGETMULDATA 传入参数buf[%s]col[%s]row[%s]w[%s]",aParm[0],aParm[1],aParm[2],aParm[3]);
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
		
	/*处理原报文中出现的 | 替换为 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [函数返回] _extufProPTGETMULDATA()返回码=0,结果=[%s]",aResult);		
		return 0;
	}		
	iTotalLen=strlen(alTmp);
	/*处理宽度结构数组*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMULDATA 传入宽度参数[%s]不合法",aColWidth);
		return -1;
	}
	colwidth[0]=atoi(p);
	iNeedLen+=colwidth[0];
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		colwidth[i]=atoi(p);
		iNeedLen+=colwidth[i];
		i++;
	}
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETMULDATA 传入列参数[%d]<>[%s]中竖线个数",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETMULDATA 传入数据长度[%d]<[%d]需要数据长度",iTotalLen,iNeedLen);
		return -3;
	}

	spos=0;
	dpos=0;
	memset(aDealStr,0x00,sizeof(aDealStr));

	for (j=0;j<row;j++){
		for (i=0;i<col;i++){
			memset(aOneData,0x00,sizeof(aOneData));
			memcpy(aOneData,alTmp+spos,colwidth[i]);
			memset(aFormat,0x00,sizeof(aFormat));
			strcpy(aFormat,"O");
			_DealOneData(aOneData,aFormat);
			memcpy(aDealStr+dpos,aOneData,strlen(aOneData));				
			dpos+=strlen(aOneData);
			spos+=colwidth[i];			
			
			/*
			memcpy(aDealStr+dpos,alTmp+spos,colwidth[i]);
			dpos+=colwidth[i];
			spos+=colwidth[i];
			memcpy(aDealStr+dpos,"|",1);
			dpos++;
			*/
			swVdebug(3,"S9001 colwidth[%d]=[%d] aDealStr[%s]",i,colwidth[i],aDealStr);
		}
	}

	aDealStr[dpos]=0;
	_ufNtoT(aDealStr,dpos,aResult);
	swVdebug(3,"S8273: [函数返回] _extufProPTGETMULDATA()返回码=0,结果=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** 函 数 名：_extufProPTCONVERTPIN
 ** 功    能：转加密函数(不压缩) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 系统代字
 **            aParm[1] 折号/卡号
 **            aParm[2] 密文PINBLOCK  
 ** 返 回 值： 0 成功 aResult 转加密后的PINBLOCK
***************************************************************/
int _extufProPTCONVERTPIN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char syscode[64+1];
	char pan[19+1];
	char pin_in[16+1];
	char pin_out[16+1];
	char syscodeb[64+1];
	char aflag[2];


	strcpy(aResult,"\0");
	if (iParmCount!=5) {
		swVdebug(0,"S9001:_extufProPTCONVERTPIN 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(syscode,0x00,sizeof(syscode));
	memset(pan,0x00,sizeof(pan));
	memset(pin_in,0x00,sizeof(pin_in));
	memset(pin_out,0x00,sizeof(pin_out));
	memset(syscodeb,0x00,sizeof(syscodeb));
	memset(aflag,0x00,sizeof(aflag));

	strcpy(syscode,aParm[0]);
	strcpy(pan,aParm[1]);
	strcpy(pin_in,aParm[2]);
	strcpy(syscodeb,aParm[3]);
	strcpy(aflag,aParm[4]);
	swVdebug(3,"syscodeb_test=%s",syscodeb);
	swVdebug(3,"aflag_test=%s",aflag);
//	ilRc=HsmPINConvert(syscode, pan,pin_in,pin_out);
	ilRc=HsmPINConvert_New(syscode, pan,pin_in,pin_out,syscodeb,aflag);
	if (ilRc){
		swVdebug(0,"S9001:HsmPINConvert error[%d]",ilRc);
		igFormatHsmFlag=-99;
		return -2;
	}
	pin_out[16]=0;	
	strcpy(aResult,pin_out);
	swVdebug(3,"S8273: [函数返回] _extufProPTCONVERTPIN()返回码=0,结果=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** 函 数 名：_extufProPTCONVERTPINBCD
 ** 功    能：转加密函数(压缩) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 系统代字
 **            aParm[1] 折号/卡号
 **            aParm[2] 密文PINBLOCK  
 ** 返 回 值： 0 成功 aResult 转加密后的PINBLOCK
***************************************************************/
int _extufProPTCONVERTPINBCD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char syscode[64+1];
	char pan[19+1];
	char pin_in[16+1];
	char pin_out[16+1];
	char syscodeb[64+1];
	char aflag[2];


	strcpy(aResult,"\0");
	if (iParmCount!=5) {
		swVdebug(0,"S9001:_extufProPTCONVERTPIN 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(syscode,0x00,sizeof(syscode));
	memset(pan,0x00,sizeof(pan));
	memset(pin_in,0x00,sizeof(pin_in));
	memset(pin_out,0x00,sizeof(pin_out));
	memset(syscodeb,0x00,sizeof(syscodeb));
	memset(aflag,0x00,sizeof(aflag));

	strcpy(syscode,aParm[0]);
	strcpy(pan,aParm[1]);
	strcpy(pin_in,aParm[2]);
	strcpy(syscodeb,aParm[3]);
	strcpy(aflag,aParm[4]);
	swVdebug(3,"syscodeb_test=%s",syscodeb);
	swVdebug(3,"aflag_test=%s",aflag);
//	ilRc=HsmPINConvert(syscode, pan,pin_in,pin_out);
	ilRc=HsmPINConvert_New(syscode, pan,pin_in,pin_out,syscodeb,aflag);
	if (ilRc){
		swVdebug(0,"S9001:HsmPINConvert error[%d]",ilRc);
		igFormatHsmFlag=-99;
		return -2;
	}
	pin_out[16]=0;	
	swVdebug(3,"HsmPINConvert [%s] [%s] [%s] -> [%s]",syscode, pan,pin_in,pin_out);
	PackBCD( pin_out, pin_in, 16 );
	_ufNtoT(pin_in,8,aResult);
	ilLen=8;
	swVdebug(3,"S8273: [函数返回] _extufProPTCONVERTPINBCD()返回码=0,结果=[%s]len[%d]",aResult,ilLen);
	return(0);
}


/**************************************************************
 ** 函 数 名：_extufProPTMACGEN
 ** 功    能：根据系统代字生成MAC
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 系统代字
 **            aParm[1] macblock
 ** 返 回 值： 0 成功 aResult 计算的MAC
***************************************************************/
int _extufProPTMACGEN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	short ilStrlen;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char syscode[64+1];
	char macblock[iFLDVALUELEN];
	char mac[16+1];

	igFormatHsmFlag=0;
	strcpy(aResult,"\0");
	if (iParmCount!=2) {
		swVdebug(0,"S9001:_extufProPTMACGEN 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(syscode,0x00,sizeof(syscode));
	memset(macblock,0x00,sizeof(macblock));
	memset(mac,0x00,sizeof(mac));
	strcpy(syscode,aParm[0]);
	/*2010/11/9 add by pc 为了处理交换平台内部转义字符*/
	_ufTtoN(aParm[1],macblock,&ilStrlen);
	swVdebug(3,"S8273: [函数返回] _ufTtoN(macblock)[%s]",macblock);
	/*2010/11/9 add by pc 为了处理交换平台内部转义字符*/		
	/*strcpy(macblock,aParm[1]);*/

//	ilRc=HsmMACGen(syscode, macblock,strlen(macblock),mac);
	ilRc=HsmMACGen_New(syscode, macblock,strlen(macblock),mac);
	if (ilRc){
		swVdebug(0,"S9001:HsmMACGen error[%d]",ilRc);
		igFormatHsmFlag=-88;		
		return -2;
	}
	strcpy(aResult,mac);
	mac[16]=0;
	swVdebug(3,"S8273: [函数返回] _extufProPTMACGEN()返回码=0,结果=[%s]",aResult);
	return(0);
}

int _PTDealOneBuf(char *aSrc,int ilLen,int type,char *aDesc){

	int j,Isspace;
	int index;

	char Data[iFLDVALUELEN];
	char MacBuf[iFLDVALUELEN];
	memset(Data,0x00,sizeof(Data));
	memset(MacBuf,0x00,sizeof(MacBuf));
	index=0;
	Isspace =0;
	strcpy(Data,aSrc);
	/*2009-11-13 为了适应左边有空格的情况,先将左边空格去除*/
	_swTrimL(Data);
	ilLen=strlen(Data);
	/*2009-11-13 为了适应左边有空格的情况,先将左边空格去除*/	
	for (j=0;j<ilLen;j++)
	{
		if((Data[j]<='Z' && Data[j]>='A')||(Data[j]<='z' && Data[j]>='a')) {
			MacBuf[index]= toupper(Data[j]);
			Isspace =0;
		}
		else if (Data[j]<='9' && Data[j]>='0') {
			MacBuf[index] = Data[j];
			Isspace =0;
		}
		else if (Data[j] ==',' || Data[j] =='.') {
			MacBuf[index] = Data[j];
			Isspace =0;
		}
		else if (Data[j] ==' ' && Isspace ==0) {
			MacBuf[index] =Data[j];
			Isspace =1;
		}
		/*2009-12-29 10:04:05 add by qzg*/
		else
		{
			continue;
		}	
		index++;
	}
	if (!Isspace) {
		MacBuf[index] = ' ';
		index++;
	}
	MacBuf[index] = '\0';
	if ((type==Elln)||(type==Ellan)||(type==Ellz)||(type==Ellans)){
		sprintf(aDesc,"%02d",ilLen);
		strcat(aDesc,MacBuf);
	}
	else if((type==Ellln)||(type==Elllan)||(type==Elllz)||(type==Elllans)){
		sprintf(aDesc,"%03d",ilLen);
		strcat(aDesc,MacBuf);
	}
	else if((type==Elllln)||(type==Ellllan)||(type==Ellllz)||(type==Ellllans)){
		sprintf(aDesc,"%04d",ilLen);
		strcat(aDesc,MacBuf);
	}
	else
		strcpy(aDesc,MacBuf);
	swVdebug(2,"in[%s] out[%s]",aSrc,aDesc);
	return 0;
}
int _PTDealOneBufCX(char *aSrc,int ilLen,char *aDesc){

	int j,Isspace;
	int index;

	char Data[iFLDVALUELEN];
	char MacBuf[iFLDVALUELEN];
	memset(Data,0x00,sizeof(Data));
	memset(MacBuf,0x00,sizeof(MacBuf));
	index=0;
	Isspace =0;
	strcpy(Data,aSrc);
	for (j=0;j<ilLen;j++)
	{
		if((Data[j]<='Z' && Data[j]>='A')||(Data[j]<='z' && Data[j]>='a')) {
			MacBuf[index]= toupper(Data[j]);
			Isspace =0;
		}
		else if (Data[j]<='9' && Data[j]>='0') {
			MacBuf[index] = Data[j];
			Isspace =0;
		}
		else if (Data[j] ==',' || Data[j] =='.') {
			MacBuf[index] = Data[j];
			Isspace =0;
		}
		else if (Data[j] ==' ' && Isspace ==0) {
			MacBuf[index] =Data[j];
			Isspace =1;
		}
		index++;
	}
	if (!Isspace) {
		MacBuf[index] = ' ';
		index++;
	}
	MacBuf[index] = '\0';
	strcpy(aDesc,MacBuf);
	swVdebug(2,"in[%s] out[%s]",aSrc,aDesc);
	return 0;
}

/**************************************************************
 ** 函 数 名：_extufProPTGET8583MACBUF
 ** 功    能：得到MACBUF函数
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0]  参加MAC计算的域 形式为: 3|12|13|
 ** 返 回 值： 0 成功 aResult 转加密后的PINBLOCK
***************************************************************/
int _extufProPTGET8583MACBUF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int ilRc;
	short ilTableId;                         /* 表id */
	short ilFldId;                           /* 域id */
	char aMacField[256];
	int sField[128];
	int ilTotalNum;
	char alTmp[iFLDVALUELEN];
	char *p;
	int i;
	int ilLen;
	int iTotalLen;
	struct isoprebuf
	{
		short iFldid;
		short iFldlen;
		char  aFldvalue[iFLDVALUELEN + 1];
		char  aOrgfldlen[10];
		short iOrgfldlen;
		char  aOrgfldvalue[iFLDVALUELEN + 1];
	} slIsobuf;

	struct swt_sys_queue slSwt_sys_queue;
	struct swt_sys_8583  slSwt_sys_8583;

	ilRc=swShmselect_swt_sys_queue(psgMsghead->iDes_q, &slSwt_sys_queue);
	if (ilRc){
		swVdebug(0,"S9001:_extufProPTGET8583MACBUF  swShmselect_swt_sys_queue get qid[%d] error[%d]",psgMsghead->iDes_q,
		    ilRc);
		return -2;
	}
	ilTableId=slSwt_sys_queue.isotab_id;

	strcpy(aMacField,aParm[0]);
	p=strtok(aMacField,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGET8583MACBUF 传入参数[%s]不合法",aMacField);
		return -3;
	}
	sField[0]=atoi(p);
	ilTotalNum=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		sField[ilTotalNum]=atoi(p);
		ilTotalNum++;
	}
	for (i=0;i<ilTotalNum;i++)
	{
		swVdebug(3,"[%d] [%d]",i,sField[i]);
	}
	swVdebug(2,"共[%d]个域参加Mac运算",ilTotalNum);

	iTotalLen=0;
	memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
	for (i=0;i<ilTotalNum;i++) {
		ilRc = swShmselect_swt_sys_8583(ilTableId,sField[i], &slSwt_sys_8583);
		if (ilRc != SUCCESS){
			swVdebug(0,"S9001:_extufProPTGET8583MACBUF  swShmselect_swt_sys_8583 get tableid[%d]filed[%d] error[%d]",
			    ilTableId,sField[i],ilRc);
			return (-4);
		}
		swVdebug(2,"S9001 psgMsghead->iMsgtype[%d]",psgMsghead->iMsgtype);
		
		if (psgMsghead->iMsgtype == iMSGUNPACK)
			ilRc=swIsoget(psgPreunpackbuf,sField[i],&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
		else	 
			ilRc=swIsoget(psgUnpackbuf,sField[i],&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
			
		if (ilRc){
			swVdebug(0,"S9001:_extufProPTGET8583MACBUF  swIsoget get field[%d] error[%d]",sField[i],ilRc);
/*			return (-4);*/
		}
		swVdebug(2,"ISO[%d] len[%d] value[%s] type[%d]",sField[i],slIsobuf.iFldlen,slIsobuf.aFldvalue,slSwt_sys_8583.fld_type);
		memset(alTmp,0x00,sizeof(alTmp));
		_PTDealOneBuf(slIsobuf.aFldvalue,slIsobuf.iFldlen,slSwt_sys_8583.fld_type,alTmp);
		ilLen=strlen(alTmp);
		memcpy(aResult+iTotalLen,alTmp,ilLen);
		iTotalLen+=ilLen;
	}
	aResult[iTotalLen]=0;
	swVdebug(3,"S8273: [函数返回] _extufProPTGET8583MACBUF()返回码=0,结果=[%s]len[%d]",aResult,iTotalLen);
	return(0);
}

/**************************************************************
 ** 函 数 名：_extufProPTGET8583MACBUFCX
 ** 功    能：得到MACBUF函数，长度域不参加计算
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0]  参加MAC计算的域 形式为: 3|12|13|
 ** 返 回 值： 0 成功 aResult 转加密后的PINBLOCK
***************************************************************/
int _extufProPTGET8583MACBUFCX(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int ilRc;
	short ilTableId;                         /* 表id */
	short ilFldId;                           /* 域id */
	char aMacField[256];
	int sField[128];
	int ilTotalNum;
	char alTmp[iFLDVALUELEN];
	char *p;
	int i;
	int ilLen;
	int iTotalLen;
	struct isoprebuf
	{
		short iFldid;
		short iFldlen;
		char  aFldvalue[iFLDVALUELEN + 1];
		char  aOrgfldlen[10];
		short iOrgfldlen;
		char  aOrgfldvalue[iFLDVALUELEN + 1];
	} slIsobuf;


	strcpy(aMacField,aParm[0]);
	p=strtok(aMacField,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGET8583MACBUF 传入参数[%s]不合法",aMacField);
		return -3;
	}
	sField[0]=atoi(p);
	ilTotalNum=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		sField[ilTotalNum]=atoi(p);
		ilTotalNum++;
	}
	for (i=0;i<ilTotalNum;i++)
	{
		swVdebug(3,"[%d] [%d]",i,sField[i]);
	}
	swVdebug(2,"共[%d]个域参加Mac运算",ilTotalNum);

	iTotalLen=0;
	memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
	for (i=0;i<ilTotalNum;i++) {
		
		if (psgMsghead->iMsgtype == iMSGUNPACK)
			ilRc=swIsoget(psgPreunpackbuf,sField[i],&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
		else	 
			ilRc=swIsoget(psgUnpackbuf,sField[i],&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
			
		if (ilRc){
			swVdebug(0,"S9001:_extufProPTGET8583MACBUF  swIsoget get field[%d] error[%d]",sField[i],ilRc);
/*			return (-4);*/
		}
		swVdebug(2,"ISO[%d] len[%d] value[%s] type[%d]",sField[i],slIsobuf.iFldlen,slIsobuf.aFldvalue);
		memset(alTmp,0x00,sizeof(alTmp));
		_PTDealOneBufCX(slIsobuf.aFldvalue,slIsobuf.iFldlen,alTmp);
		ilLen=strlen(alTmp);
		memcpy(aResult+iTotalLen,alTmp,ilLen);
		iTotalLen+=ilLen;
	}
	aResult[iTotalLen]=0;
	swVdebug(3,"S8273: [函数返回] _extufProPTGET8583MACBUF()返回码=0,结果=[%s]len[%d]",aResult,iTotalLen);
	return(0);
}

/*去掉数字中，没用的字符*/
/*例如： 1.2300->1.23   */
/*       12.000->12     */

int _DealAmount(char *aScr)
{
	int iLen;
	int i;
	char aTmp[128];
	char *p;
	
	p=strchr(aScr,'.');
	if (p==NULL)
		return 0;
	memset(aTmp,0x00,sizeof(aTmp));
	strcpy(aTmp,aScr);
	iLen=strlen(aTmp);
	
	for(i=iLen-1;i>0;i--) {

		if (aTmp[i]=='0'){ 
				aTmp[i]=0;
		}		
		else if (aTmp[i]=='.'){
				aTmp[i]=0;
				strcpy(aScr,aTmp);
				return 0;
		}	
		else {
			strcpy(aScr,aTmp);
			return 0;
		}	
	}
	strcpy(aScr,aTmp);
	return 0;		
}

/**************************************************************
# Function:	_extufProPTAMOUNT
# Describe:	Deal with money format
# Example:
#   	example: PTAMOUNT( 502.18, N12 ) ---> result: [000000050218]
# 		example: PTAMOUNT( 50218, D8.2 ) ---> result: [  502.18]
# 		example: PTAMOUNT( 502.18, 8.2 ) ---> result: [  502.18]
# 		example: PTAMOUNT( 502.18, 0.3 ) ---> result: [502.180]
# 		example: PTAMOUNT( 50218, -D8.2 ) --->result: [502.18  ]
#		  example: PTAMOUNT( 502.18, C ) --->   result: [chinese]
#     example: PTAMOUNT( 502.18, V ) --->   result: [5.0218]	完成储蓄除 100功能
#     example: PTAMOUNT( 502.18, X ) --->   result: [50218]   完成储蓄乘 100功能
# Author:	rime
# Update:	2000/11/8
**************************************************************/
int _extufProPTAMOUNT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	char alTmp[iFLDVALUELEN];
	char alBuf[iFLDVALUELEN];
	char fmt[iFLDVALUELEN];
	int ilRc;
	
	if (iParmCount!=2) {
		swVdebug(0,"S9001:_ufProPTAMOUNT 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	
	strcpy(alTmp, aParm[0]);
	_swTrim(alTmp);
	_DealAmount(alTmp);
	strcpy(fmt, aParm[1]);

	ilRc=swAmount(aResult, alTmp, fmt);
	if (ilRc){
		swVdebug(0,"S9001:swAmount 传入参数[%s] [%s]不合法",alTmp,fmt);
		strcpy(aResult,"");
		return 0;
	}	
	
	swVdebug(3,"S8273: [函数返回] _ufProPTAMOUNT()返回码=0,结果=[%s]len[%d]",aResult,strlen(aResult));
	
	return(0);
}

short isnum(char * sInt)
{
    short i;
    for (i = 0;i < strlen(sInt);i++) {
        if ( (sInt[i] < '0') || (sInt[i] > '9') )
            return (FALSE);
    }
    return (TRUE);
}

/**************************************************************
# Function:	amount
# Describe:	Deal with format of money
# Author:	Rime Lee
# Update:	2001/11/28
**************************************************************/
int swAmount(char * amt, char * scr, char * format)
{
	short	i, nint, ndec, len, flag, iflag,iflag1;
	char	tmp[512], scrb[512];
	char	buf[512], fmt[512];
	char	cint[512], cdec[512];
	char	*pa;
	char *p;
	double lld;
	
	strcpy(tmp, scr);
	_swTrim(tmp);
	strcpy(scrb, tmp);
	strcpy(fmt, format);
	_swTrim(fmt);
    
    iflag=0;
    if(tmp[0]=='-'){
    	iflag=1;
    	tmp[0]='0';
    }
    
    
	amt[0] = '\0';
	
	if ((fmt[0]=='X') || (fmt[0]=='x')) {
/*
		strcat(tmp,"00");
		p=strchr(tmp,'.');
		if (p==NULL) {
			strcpy(amt, tmp);
			return(0);			
		}
		else {
			memcpy(p,p+1,2);
			*(p+2)='.';
		}
		_DealAmount(tmp);
		strcpy(amt,tmp);*/
	
    lld=atof(tmp);
    lld=lld*100;
    memset(tmp,0x00,sizeof(tmp));
    sprintf(tmp,"%lf",lld);
    _DealAmount(tmp);
    
    if(iflag){
    	amt[0]='-';
    	strcpy(amt+1,tmp);
    }
    else
    	strcpy(amt,tmp);
    			
		return(0);
	}
	if ((fmt[0]=='V') || (fmt[0]=='v')) {
    lld=atof(tmp);
    lld=lld/100;
    memset(tmp,0x00,sizeof(tmp));
    sprintf(tmp,"%lf",lld);
    _DealAmount(tmp);
    
    if(iflag){
    	amt[0]='-';
    	strcpy(amt+1,tmp);
    }
    else
    	strcpy(amt,tmp);
    	
    return(0);
	}	
	
	if ((fmt[0]=='N') || (fmt[0]=='n')) {
		lld=atof(tmp);
		lld=lld*100;
		memset(tmp,0x00,sizeof(tmp));
		sprintf(tmp,"%lf",lld);
		_DealAmount(tmp);
	    
		if ((pa = strchr(tmp, '.')) != NULL){
			* pa = 0x00;
			strcpy(cint, tmp);  
		} 
		else {
			strcpy(cint,tmp);   
		}
		
		nint = atoi(fmt+1);
		memset(tmp, 0x30, nint);
		if (nint<strlen(cint)){
			swVdebug(0,"送入金额超过长度限制");
			return -1;
		}
		memcpy(tmp+nint-strlen(cint), cint, strlen(cint));
		tmp[nint]=0;
		
		if(iflag){
			amt[0]='-';
			strcpy(amt+1,tmp);
		}
		else
			strcpy(amt,tmp);
    		
		return 0;
	}	
	if ((pa = strchr(tmp, '.')) != NULL)
	{
		* pa = 0x00;
		strcpy(cint, tmp);
		strcpy(cdec, pa+1);
		if (!isnum(cint)) return(-1);
		if (!isnum(cdec)) return(-1);
	} else {
		strcpy(cint,tmp);
		cdec[0] = 0x00;
		if (!isnum(cint)) return(-1);
	}


	_swTrim(fmt);
	if(fmt[0]=='-')
	{
		flag=1;
		memmove(fmt,fmt+1,strlen(fmt));
	}
	else flag=0;

	if( (fmt[0] == 'N') || (fmt[0] == 'n') ) {
		ndec = -1;
		nint = atoi(fmt+1);
		flag=2;
	} else if( (fmt[0] == 'C') || (fmt[0] == 'c') ) {
		_curntoc(buf, scrb);
		
		if(iflag){
			sprintf(amt,"负%s",tmp);
		}
		else
			strcpy(amt,buf);
			
		return(0);
	} else if( (pa = strchr(fmt, '.')) == NULL ) {
		ndec = 0;
		nint = atoi( fmt );
		if( ndec < 0 ) ndec = 0;
	} else {
		* pa = 0x00;
		if( (fmt[0] == 'D') || (fmt[0] == 'd') )
			nint = atoi( fmt+1 );
		else
			nint = atoi( fmt );
		ndec = atoi( pa+1 );
		if( ndec < 0 ) ndec = 0;
		if( (fmt[0] == 'D') || (fmt[0] == 'd') ) {
			len = strlen(cint);
			if(len <= ndec) {
				sprintf(tmp,"%%0%dd",ndec+1);
				sprintf(cint,tmp,atoi(cint));
				strcpy(tmp,cint);
			}
			i = strlen(tmp)-ndec;
			memcpy( cint, tmp, i );
			cint[i] = 0x00;
			strcpy( cdec, tmp+i );
		}
	}
	if( nint < 0 ) nint = 0;

	if( ndec == -1 )
		strcat(cint, cdec);
	else {
		memset(fmt, 0x30, ndec);
		memcpy(fmt, cdec, strlen(cdec));
		fmt[ndec] = 0x00;
		if(ndec>0) strcat(cint, ".");
		strcat(cint, fmt);
	}

	if(nint < strlen(cint)) nint = strlen(cint);
	memset(tmp, 0x00, sizeof(tmp));
	if(flag==1)
	{
		memset(tmp, 0x20, nint);
		memcpy(tmp, cint, strlen(cint));
	}
	else if (flag==2)
	{

		memset(tmp, 0x30, nint);
		memcpy(tmp+nint-strlen(cint), cint, strlen(cint));
		tmp[nint]=0;

		if(iflag){
			amt[0]='-';
			strcpy(amt+1,tmp);
		}
		else
			strcpy(amt,tmp);

		return(0);
	}		
	else {
		memset(tmp, 0x20, nint-strlen(cint));
		strcat(tmp, cint);
	}
	
	iflag1=0;
	for(i=0; i<strlen(tmp); i++) {
		if( (tmp[i] != '0') && (tmp[i] != ' ') ) {
			if( (tmp[i]=='.') && (tmp[i-1]==' ') ) {
				tmp[i-1] = '0';
				iflag1=1;
			}
			
			if(iflag){
				if(iflag1)
					tmp[i-2]='-';
					
				if(tmp[i-1]==' ')
					tmp[i-1]='-';
			}			
			break;
		} else {
			if( tmp[i+1] == 0x00 ) {
				tmp[i] = '0';
				break;
			} else
				tmp[i] = ' ';
		}
	}

	strcpy(amt, tmp);
	return(0);
}

int swAmount_old(char * amt, char * scr, char * format)
{
	short	i, nint, ndec, len, flag;
	char	tmp[512], scrb[512];
	char	buf[512], fmt[512];
	char	cint[512], cdec[512];
	char	*pa;
	char *p;
	double lld;
	
	strcpy(tmp, scr);
	_swTrim(tmp);
	strcpy(scrb, tmp);
	strcpy(fmt, format);
	_swTrim(fmt);

	amt[0] = '\0';
	
	if ((fmt[0]=='X') || (fmt[0]=='x')) {
/*
		strcat(tmp,"00");
		p=strchr(tmp,'.');
		if (p==NULL) {
			strcpy(amt, tmp);
			return(0);			
		}
		else {
			memcpy(p,p+1,2);
			*(p+2)='.';
		}
		_DealAmount(tmp);
		strcpy(amt,tmp);*/
	
    lld=atof(tmp);
    lld=lld*100;
    memset(tmp,0x00,sizeof(tmp));
    sprintf(tmp,"%lf",lld);
    _DealAmount(tmp);
    strcpy(amt,tmp);		
		return(0);
	}
	if ((fmt[0]=='V') || (fmt[0]=='v')) {
    lld=atof(tmp);
    lld=lld/100;
    memset(tmp,0x00,sizeof(tmp));
    sprintf(tmp,"%lf",lld);
    _DealAmount(tmp);
    strcpy(amt,tmp);
    return(0);
	}	
	
	if ((fmt[0]=='N') || (fmt[0]=='n')) {
		lld=atof(tmp);
		lld=lld*100;
		memset(tmp,0x00,sizeof(tmp));
		sprintf(tmp,"%lf",lld);
		_DealAmount(tmp);
	    
		if ((pa = strchr(tmp, '.')) != NULL){
			* pa = 0x00;
			strcpy(cint, tmp);  
		} 
		else {
			strcpy(cint,tmp);   
		}
		
		nint = atoi(fmt+1);
		memset(tmp, 0x30, nint);
		if (nint<strlen(cint)){
			swVdebug(0,"送入金额超过长度限制");
			return -1;
		}
		memcpy(tmp+nint-strlen(cint), cint, strlen(cint));
		tmp[nint]=0;
		strcpy(amt, tmp);	
		return 0;
	}	
	if ((pa = strchr(tmp, '.')) != NULL)
	{
		* pa = 0x00;
		strcpy(cint, tmp);
		strcpy(cdec, pa+1);
		if (!isnum(cint)) return(-1);
		if (!isnum(cdec)) return(-1);
	} else {
		strcpy(cint,tmp);
		cdec[0] = 0x00;
		if (!isnum(cint)) return(-1);
	}


	_swTrim(fmt);
	if(fmt[0]=='-')
	{
		flag=1;
		memmove(fmt,fmt+1,strlen(fmt));
	}
	else flag=0;

	if( (fmt[0] == 'N') || (fmt[0] == 'n') ) {
		ndec = -1;
		nint = atoi(fmt+1);
		flag=2;
	} else if( (fmt[0] == 'C') || (fmt[0] == 'c') ) {
		_curntoc(buf, scrb);
		strcpy(amt, buf);
		return(0);
	} else if( (pa = strchr(fmt, '.')) == NULL ) {
		ndec = 0;
		nint = atoi( fmt );
		if( ndec < 0 ) ndec = 0;
	} else {
		* pa = 0x00;
		if( (fmt[0] == 'D') || (fmt[0] == 'd') )
			nint = atoi( fmt+1 );
		else
			nint = atoi( fmt );
		ndec = atoi( pa+1 );
		if( ndec < 0 ) ndec = 0;
		if( (fmt[0] == 'D') || (fmt[0] == 'd') ) {
			len = strlen(cint);
			if(len <= ndec) {
				sprintf(tmp,"%%0%dd",ndec+1);
				sprintf(cint,tmp,atoi(cint));
				strcpy(tmp,cint);
			}
			i = strlen(tmp)-ndec;
			memcpy( cint, tmp, i );
			cint[i] = 0x00;
			strcpy( cdec, tmp+i );
		}
	}
	if( nint < 0 ) nint = 0;

	if( ndec == -1 )
		strcat(cint, cdec);
	else {
		memset(fmt, 0x30, ndec);
		memcpy(fmt, cdec, strlen(cdec));
		fmt[ndec] = 0x00;
		if(ndec>0) strcat(cint, ".");
		strcat(cint, fmt);
	}

	if(nint < strlen(cint)) nint = strlen(cint);
	memset(tmp, 0x00, sizeof(tmp));
	if(flag==1)
	{
		memset(tmp, 0x20, nint);
		memcpy(tmp, cint, strlen(cint));
	}
	else if (flag==2)
	{

		memset(tmp, 0x30, nint);
		memcpy(tmp+nint-strlen(cint), cint, strlen(cint));
		tmp[nint]=0;
		strcpy(amt, tmp);
		return(0);
	}		
	else {
		memset(tmp, 0x20, nint-strlen(cint));
		strcat(tmp, cint);
	}

	for(i=0; i<strlen(tmp); i++) {
		if( (tmp[i] != '0') && (tmp[i] != ' ') ) {
			if( (tmp[i]=='.') && (tmp[i-1]==' ') ) tmp[i-1] = '0';
			break;
		} else {
			if( tmp[i+1] == 0x00 ) {
				tmp[i] = '0';
				break;
			} else
				tmp[i] = ' ';
		}
	}

	strcpy(amt, tmp);
	return(0);
}




/**************************************************************
* Function:	_curntoc
* Describe:	exchange money to chinese mode
* Update:	2001/11/28
**************************************************************/
int _curntoc(char * outbuf, char * curbuf)
{
	int		numlen, i, j, k;
	char	prebuf[3][64];
	char	prechar[64], endbuf[64], endbuf1[64];
	char	* ptr;
	static char numb1[][3]={
		"元","万","亿"	};
	static char numb2[][3]={
		"仟","佰","拾"	};
	static char chn[][3]={
		"零","壹","贰","叁","肆","伍","陆","柒","捌","玖"	};

	outbuf[0] = 0x00;
	endbuf[0] = 0x00;
	endbuf1[0] = 0x00;

	if ((ptr = strchr(curbuf, '.')) != NULL)
	{
		j = ptr - curbuf;
		memcpy(prechar, curbuf, j);
		prechar[j] = 0x00;
		strncpy(endbuf, ptr+1, 2);
		endbuf[2] = 0x00;

		if ((strcmp(endbuf, "00") == 0) || (strcmp(endbuf, "0") == 0))
		{
			strcpy(endbuf1, "整");
		} else {
			if (endbuf[0] == '0')
			{
				if (atoi(prechar) != 0) strcat(endbuf1, "零");
				strcat(endbuf1, chn[endbuf[1]-'0']);
				strcat(endbuf1, "分");
			} else if ((endbuf[1] == '0') || (endbuf[1] == 0x00)) {
				strcat(endbuf1, chn[endbuf[0]-'0']);
				strcat(endbuf1, "角");
			} else {
				strcat(endbuf1, chn[endbuf[0]-'0']);
				strcat(endbuf1, "角");
				strcat(endbuf1, chn[endbuf[1]-'0']);
				strcat(endbuf1, "分");
			}
		}
	} else {
		strcpy(endbuf1, "整");
		strcpy(prechar, curbuf);
	}

	numlen = strlen(prechar);
	if (atoi(prechar) == 0)
	{
		if (atoi(endbuf) == 0) strcat(outbuf, "零元");
	} else {
		k = j = 0;
		for (i=0; i<3; i++)
		{
			if(numlen == 0)
				break;
			else if (numlen > 4)
				k = 4;
			else
				k = numlen;
			j = numlen - k;
			memcpy(prebuf[i], prechar + j, k);
			prebuf[i][k] = 0x00;
			numlen -= k;
		}

		for (j=i-1; j>=0; j--)
		{
			if (atoi(prebuf[j]) == 0)
			{
				if (j == 0) strcat(outbuf, "元");
				continue;
			}
			numlen = strlen(prebuf[j]);
			if (numlen == 3)
			{
				if (memcmp(prebuf[j]+1, "00", 2) == 0)
				{
					strcat(outbuf, chn[prebuf[j][0]-'0']);
					strcat(outbuf, "佰");
					strcat(outbuf, numb1[j]);
					continue;
				}
			} else if (numlen == 4) {
				if (memcmp(prebuf[j]+1, "000", 3) == 0)
				{
					strcat(outbuf, chn[prebuf[j][0] - '0']);
					strcat(outbuf, "仟");
					strcat(outbuf, numb1[j]);
					continue;
				}
				if (memcmp(prebuf[j] + 2, "00", 2) == 0)
				{
					strcat(outbuf, chn[prebuf[j][0] - '0']);
					if (prebuf[j][0] != '0') strcat(outbuf, "仟");
					strcat(outbuf, chn[prebuf[j][1] - '0']);
					strcat(outbuf, "佰");
					strcat(outbuf, numb1[j]);
					continue;
				}
			}

			for (k=0; k<numlen; k++)
			{
				if (prebuf[j][k] == '0')
				{
					if (k == 0)
						strcat(outbuf, "零");
					else if ((prebuf[j][k-1] == '0') || (k == numlen-1))
						continue;
					else
						strcat(outbuf, "零");
				} else {
					strcat(outbuf, chn[prebuf[j][k] - '0']);
					if ((numlen == 4) && (k < 3))
						strcat(outbuf, numb2[k]);
					else if ((numlen < 4) && (k < numlen-1))
						strcat(outbuf, numb2[k + 4 - numlen]);
				}
			}
			strcat(outbuf, numb1[j]);
		}
	}

	strcat(outbuf, endbuf1);
	return(strlen(outbuf));
}

/**************************************************************
 ** 函 数 名：_extufProPTGETFUNDTRANCODE
 ** 功    能：获得基金交易码－用于基金应答后的解包区分格式转换动作
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] ISO(1)-交易类型 - 0200 成功应答，9999，错误应答 0800 密钥交换请求
 **            aParm[1]:ISO(3)-原始交易码
 **            aParm[2]:ISO(39)-响应码
 ** 返 回 值： 0 成功 aResult 计算的MAC
***************************************************************/
int _extufProPTGETFUNDTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char msgtype[4+1];
	char procode[6+1];
	char respcode[4+1];
	char aTrancode[11+1];
	if (iParmCount!=3) {
		swVdebug(0,"S9001:_extufProPTGETFUNDTRANCODE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(msgtype,0x00,sizeof(msgtype));
	memset(procode,0x00,sizeof(procode));	
	memset(respcode,0x00,sizeof(respcode));	
	memset(aTrancode,0x00,sizeof(aTrancode));
	strcpy(msgtype,aParm[0]);
	strcpy(procode,aParm[1]);
	strcpy(respcode,aParm[2]);
	_swTrim(respcode);
	
	if (atoi(msgtype)==200) {
		/*应用响应*/
		if (atoi(respcode)==0){
			/*成功的响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"0",1);
		}
		else {
			/*错误响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"1",1);			
		}	
	}
	else if (atoi(msgtype)==9999){
		/*安全错误*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"9",1);		
	}	
	else {
		/*无响应码，是密钥交换请求交易*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"0",1);
	}	
	strcpy(aResult,aTrancode);

	swVdebug(3,"S8273: [函数返回] _extufProPTGETFUNDTRANCODE()返回码=0,结果=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** 函 数 名：_extufProPTGETLOANTRANCODE
 ** 功    能：获得信贷交易码－用于信贷应答后的解包区分格式转换动作
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] ISO(1)-交易类型 - 0200 成功应答，9999，错误应答 0800 密钥交换请求
 **            aParm[1]:ISO(3)-原始交易码
 **            aParm[2]:ISO(39)-响应码
 ** 返 回 值： 0 成功 aResult 计算的MAC
***************************************************************/
int _extufProPTGETLOANTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char msgtype[4+1];
	char procode[6+1];
	char respcode[4+1];
	char aTrancode[11+1];
	if (iParmCount!=3) {
		swVdebug(0,"S9001:_extufProPTGETLOANTRANCODE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(msgtype,0x00,sizeof(msgtype));
	memset(procode,0x00,sizeof(procode));	
	memset(respcode,0x00,sizeof(respcode));	
	memset(aTrancode,0x00,sizeof(aTrancode));
	strcpy(msgtype,aParm[0]);
	strcpy(procode,aParm[1]);
	strcpy(respcode,aParm[2]);
	_swTrim(respcode);
	
	if (atoi(msgtype)==210) {
		/*应用响应*/
		if (atoi(respcode)==0){
			/*成功的响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,4);
			memcpy(aTrancode+6,"0",1);
		}
		else {
			/*错误响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,4);
			memcpy(aTrancode+6,"1",1);			
		}	
	}
	else if (atoi(msgtype)==9999){
		/*信贷通讯错误*/
		memcpy(aTrancode,msgtype,2);
		/*memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"9",1);	*/	
	}	
	else {
		/*无响应码，是密钥交换请求交易*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,4);
		memcpy(aTrancode+6,"0",1);
	}	
	strcpy(aResult,aTrancode);

	swVdebug(3,"S8273: [函数返回] _extufProPTGETLOANTRANCODE()返回码=0,结果=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** 函 数 名：_extufProPTGETREMITTRANCODE
 ** 功    能：获得汇兑交易码－用于汇兑应答后的解包区分格式转换动作
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] ISO(1)-交易类型 - 0200 成功应答，9999，错误应答 0800 密钥交换请求
 **            aParm[1]:ISO(3)-原始交易码
 **            aParm[2]:ISO(39)-响应码
 ** 返 回 值： 0 成功 aResult 计算的MAC
***************************************************************/
int _extufProPTGETREMITTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char msgtype[4+1];
  char msgtype_2[2+1];
	char procode[6+1];
	char respcode[4+1];
	char aTrancode[11+1];
	if (iParmCount!=3) {
		swVdebug(0,"S9001:_extufProPTGETREMITTRANCODE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(msgtype,0x00,sizeof(msgtype));
	memset(msgtype_2,0x00,sizeof(msgtype_2));
	memset(procode,0x00,sizeof(procode));	
	memset(respcode,0x00,sizeof(respcode));	
	memset(aTrancode,0x00,sizeof(aTrancode));
	strcpy(msgtype,aParm[0]);
	strcpy(procode,aParm[1]);
	strcpy(respcode,aParm[2]);
	_swTrim(respcode);
	
	memcpy(msgtype_2,msgtype,2);
	if(strcmp(msgtype_2,"08") == 0
			||strcmp(msgtype_2,"05") == 0 
			|| strcmp(msgtype_2, "04") == 0)
	{
		/*汇兑管理通知类交易*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		/*if(strcmp(msgtype,"0420") == 0)
			memcpy(aTrancode+8,"1",1);
		else
			memcpy(aTrancode+8,"0",1);*/
		/*应用响应*/
		if (atoi(respcode)==0){
			/*成功的响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"0",1);
		}
		else {
			/*错误响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"1",1);			
		}	
	}
	else if(strcmp(msgtype,"9999")==0){
		/*汇兑通讯错误及拒绝错误*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"9",1);		
	}
	else{
		/*应用响应*/
		if (atoi(respcode)==0){
			/*成功的响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"0",1);
		}
		else {
			/*错误响应*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"1",1);			
		}	
	}
	strcpy(aResult,aTrancode);

	swVdebug(3,"S8273: [函数返回] _extufProPTGETREMITTRANCODE()返回码=0,结果=[%s]",aResult);
	return(0);
}


/**************************************************************
 ** 函 数 名：_extufProPTGETFORMATMULDATA
 ** 功    能：处理多笔明细数据(带格式处理的) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 要处理的字符串buff
 **            aParm[1] 一共要处理多少列
 **            aParm[2] 一共要处理多少行数据 
 **            aParm[3] 各列宽度(用竖线分隔，例如 有3列 12|20|30|)
 **            aParm[4] 各列格式转换(用竖线分隔，例如 有3列 M:D12|C:FUND||)
 **            每列前两个字母代表转换规则：M: 代表金额转换 后跟转换格式
 **                                        C: 代表代码转换 后跟转换列关键字
 **                                        A: 代表代码转换，保留此列数据，并在后面增加一列中文描述 后跟关键字
 **            aParm[5] 各列是否需要(例如 有3列，只要前两列 1|1|0|)
 ** 返 回 值： 0 成功 aResult 处理后的多笔明细
***************************************************************/
int _extufProPTGETFORMATMULDATA(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int i,j;
	int col;
	int row;
	int colwidth[100];
	char alTmp[iFLDVALUELEN];
	char aColWidth[256];
	char aColWidth2[256];
	char aDealStr[iFLDVALUELEN];
	short iTotalLen=0;
	short iNeedLen=0;
	char sFormat[100][32];
	char aFormat[512];
	char aFormat2[512];	
	char aOneData[512];
	char aNeed[128];
	
	char aTmp[32];
	char *p;
	int spos;
	int dpos;
	
	if (iParmCount!=6) {
		strcpy(aResult,"\0");
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETFORMATMULDATA 传入参数buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

	/*处理原报文中出现的 | 替换为 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [函数返回] _extufProPTGETFORMATMULDATA()返回码=0,结果=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*处理宽度结构数组*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入宽度参数[%s]不合法",aColWidth);
		return -1;
	}
	colwidth[0]=atoi(p);
	iNeedLen+=colwidth[0];
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		colwidth[i]=atoi(p);
		iNeedLen+=colwidth[i];
		i++;
	}
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入列参数[%d]<>[%s]中竖线个数",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入数据长度[%d]<[%d]需要数据长度",iTotalLen,iNeedLen);
		return -3;
	}
	
	/*处理格式串*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入格式参数[%s]不合法",aFormat);
		return -1;
	}
	strcpy(sFormat[0],p);
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)>0) strcpy(sFormat[i],p);
		i++;
		if (i==col) break;
	}	
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入列参数[%d]<>[%s]格式串中竖线个数[%d]",col,aFormat2,i);
		return -2;
	}	
	/*处理需要数据域*/
	memset(aNeed,0x00,sizeof(aNeed));
	strcpy(aNeed,aParm[5]);	
	
	spos=0;
	dpos=0;
	memset(aDealStr,0x00,sizeof(aDealStr));

	for (j=0;j<row;j++){
		for (i=0;i<col;i++){
			/*根据格式处理一个数据*/
			if (aNeed[i*2]=='1') {
				/*需要此列数据*/
				memset(aOneData,0x00,sizeof(aOneData));
				memcpy(aOneData,alTmp+spos,colwidth[i]);
swVdebug(3,"add by pc aOneData[%s]aFormat[%s]",aOneData,sFormat[i]);				
				_DealOneData(aOneData,sFormat[i]);
				memcpy(aDealStr+dpos,aOneData,strlen(aOneData));				
				dpos+=strlen(aOneData);
			}
			spos+=colwidth[i];
			/*
			memcpy(aDealStr+dpos,alTmp+spos,colwidth[i]);
			dpos+=colwidth[i];
			spos+=colwidth[i];
			memcpy(aDealStr+dpos,"|",1);
			dpos++;
			*/
					swVdebug(3,"S9001 colwidth[%d]=[%d] aDealStr[%s]",i,colwidth[i],aDealStr);
		}
	}

	aDealStr[dpos]=0;
	_ufNtoT(aDealStr,dpos,aResult);

	swVdebug(3,"S8273: [函数返回] _extufProPTGETFORMATMULDATA()返回码=0,结果=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** 函 数 名：_extufProPTGETFORMATMULDATAEXT
 ** 功    能：处理多笔明细数据(带格式处理的,支持变长字段) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 要处理的字符串buff
 **            aParm[1] 一共要处理多少列
 **            aParm[2] 一共要处理多少行数据 
 **            aParm[3] 各列宽度(用竖线分隔，例如 有3列 12|20|30|)
 **            aParm[4] 各列格式转换(用竖线分隔，例如 有3列 M:D12|C:FUND||)
 **            每列前两个字母代表转换规则：M: 代表金额转换 后跟转换格式
 **                                        C: 代表代码转换 后跟转换列关键字
 **                                        A: 代表代码转换，保留此列数据，并在后面增加一列中文描述 后跟关键字
 **            aParm[5] 各列是否需要(例如 有3列，只要前两列 1|1|0|)
 ** 返 回 值： 0 成功 aResult 处理后的多笔明细
***************************************************************/
int _extufProPTGETFORMATMULDATAEXT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int i,j;
	int col;
	int row;
	int colwidth[100];
	char alTmp[iFLDVALUELEN];
	char aColWidth[256];
	char aColWidth2[256];
	char aDealStr[iFLDVALUELEN];
	short iTotalLen=0;
	short iNeedLen=0;
	char sFormat[100][32];
	char sColWidth[100][32];
	char aFormat[512];
	char aFormat2[512];	
	char aOneData[512];
	char aNeed[128];
	
	char aTmp[32];
	char *p;
	int spos;
	int dpos;
	int ilRc=0;
	
	if (iParmCount!=6) {
		strcpy(aResult,"\0");
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETFORMATMULDATA 传入参数buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

	/*处理原报文中出现的 | 替换为 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [函数返回] _extufProPTGETFORMATMULDATA()返回码=0,结果=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*处理宽度结构数组*/
	/*处理宽度结构数组*/
	memset((char *)&sColWidth,0x00,sizeof(sColWidth));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		printf("S9001:_extufProPTGETFORMATMULDATA 传入宽度参数[%s]不合法\n",aColWidth);
		return -1;
	}

	strcpy(sColWidth[0],p);
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		if (strlen(p)>0) strcpy(sColWidth[i],p);
		i++;
	}	
	if (i!=col){
		printf("S9001:_extufProPTGETFORMATMULDATA 传入列参数[%d]<>[%s]格式串中竖线个数[%d]\n",col,aColWidth2,i);
		return -2;
	}	
	
	/*处理格式串*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入格式参数[%s]不合法",aFormat);
		return -1;
	}
	strcpy(sFormat[0],p);
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)>0) strcpy(sFormat[i],p);
		i++;
		if (i==col) break;
	}	
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入列参数[%d]<>[%s]格式串中竖线个数[%d]",col,aFormat2,i);
		return -2;
	}	
	/*处理需要数据域*/
	memset(aNeed,0x00,sizeof(aNeed));
	strcpy(aNeed,aParm[5]);	
	
	spos=0;
	dpos=0;
	memset(aDealStr,0x00,sizeof(aDealStr));

	for (j=0;j<row;j++){
		for (i=0;i<col;i++){
			/*
			if (aNeed[i*2]=='1') {
				memset(aOneData,0x00,sizeof(aOneData));
				memcpy(aOneData,alTmp+spos,colwidth[i]);
				swVdebug(3,"add by pc aOneData[%s]aFormat[%s]",aOneData,sFormat[i]);				
				_DealOneData(aOneData,sFormat[i]);
				memcpy(aDealStr+dpos,aOneData,strlen(aOneData));				
				dpos+=strlen(aOneData);
			}
			spos+=colwidth[i];
			swVdebug(3,"S9001 colwidth[%d]=[%d] aDealStr[%s]",i,colwidth[i],aDealStr);
			*/
			memset(aOneData,0x00,sizeof(aOneData));
			ilRc=_GetOneData(alTmp,sColWidth[i],aOneData,&spos);
			if(ilRc){
				swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA GetOneData ERR ERRNO[%d]",ilRc);
				return -5;
			}
			
			swVdebug(3,"add by pc aOneData[%s]aFormat[%s]",aOneData,sFormat[i]);				
			_DealOneData(aOneData,sFormat[i]);
				
			if (aNeed[i*2]=='1') {
				memcpy(aDealStr+dpos,aOneData,strlen(aOneData));				
				dpos+=strlen(aOneData);
			}
		}
	}

	aDealStr[dpos]=0;
	_ufNtoT(aDealStr,dpos,aResult);

	swVdebug(3,"S8273: [函数返回] _extufProPTGETFORMATMULDATA()返回码=0,结果=[%s]",aResult);
	return(0);
}

/*2011-06-14根据列长度模式 aColWidth 和起始位置iPos 从待处理字符串aDealData中
取出aOneData  支持变长字段的截取 
2位变长最大长度60 配置为 2:60
3位变长最大长度125 配置为 3:125
11:41:11*/
int _GetOneData(char *aDealData, char *aColWidth, char *aOneData, int *iPos)
{
	char alTmp[iFLDVALUELEN];
	char aTmp[32];
	int  iCollen,iDatelen;
	int  iMaxlen,ilen,iType;
	char *p;
	
	memset(alTmp,0x00,sizeof(alTmp));
	memset(aOneData,0x00,sizeof(aOneData));
	
	strcpy(alTmp,aDealData);
	
	_swTrim(alTmp);
	_swTrim(aColWidth);
	
	iDatelen=strlen(alTmp);
	swVdebug(5,"S9101: _GetOneData iPos[%d],aColWidth[%s]",*iPos,aColWidth);	
	
	if(strlen(aColWidth)==0){
		swVdebug(0,"S9101: _GetOneData aColWidth is NULL ");		
		return (-1);
	}
	
	p=strchr(aColWidth,':');
	if (p!=NULL) {
		/*2011-06-11 get len type 02:32:52*/
		memset(aTmp,0x00,sizeof(aTmp));
		memcpy(aTmp,aColWidth,p-aColWidth);
		iType=atoi(aTmp);
		if(iType<=0){
			swVdebug(0,"S9101: _GetOneData iType[%d] <= 0",iType);	
			return (-2);
		}
		
		/*2011-06-11 get max len 02:33:33*/
		ilen=strlen(aColWidth)-(p-aColWidth)-1;
		memset(aTmp,0x00,sizeof(aTmp));
		memcpy(aTmp,p+1,ilen);
		
		iMaxlen=atoi(aTmp);
		
		swVdebug(5,"S9101: _GetOneData iType[%d] aColWidth[%s]",iType,aColWidth);	
				
		memset(aTmp,0x00,sizeof(aTmp));
		memcpy(aTmp,alTmp+*iPos,iType);
		ilen=atoi(aTmp);
		if(ilen>iMaxlen){
			swVdebug(0,"S9101: _GetOneData ilen[%d]>iMaxlen[%d]",ilen,iMaxlen);	
			return (-3);
		}
		iCollen=ilen;
		*iPos+=iType;
		
		
	}
	else
		iCollen=atoi(aColWidth);
	
	if(*iPos+iCollen>iDatelen){
		swVdebug(0,"S9101: _GetOneData iPos[%d]+iCollen[%d]>iDatelen[%d]",*iPos,iCollen,iDatelen);	
		return (-4);
	}
		
	memcpy(aOneData,alTmp+*iPos,iCollen);
	*iPos+=iCollen;
	
	return (0);	
}

/**************************************************************
 ** 函 数 名：_ufUsrextfunPORTGET
 ** 功    能：扩展平台函数范例
 ** 作    者：  
 ** 建立日期：2008/04/13
 ** 修改日期：
 ** 调用函数：          
 ** 全局变量：                  
 ** 参数含义：参数1:前置端口  参数2:端口层次
 ** 返 回 值： 0        
***************************************************************/
int _extufProPTPORTGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{                               
	short   iQid,iFloor,iStatus;
        int     ilRc;
        int     irec;     	
	iQid = atoi(aParm[0]);
	iFloor = atoi(aParm[1]);

	ilRc =  swPortget( iQid,iFloor,&iStatus );
	if ( ilRc )
	{
		swVdebug(0,"swPortget()函数,取端口[%d]状态出错,返回码=%d",iQid,ilRc);
		strcpy(aResult,"0");
      		return(-1);
	}
	swVdebug(3,"iStatus_test=%d",iStatus);
	irec=_sw_is_port_failure_timeout(iQid);
	if(irec == 0)
		{
			sprintf(aResult,"%d",1);
		}
	else	
			sprintf(aResult,"%d",iStatus);

	swVdebug(3,"iStatus_test_1=%d",iStatus);
	swVdebug(3,"irec_test=%d",irec);

        return(0);
}


/**************************************************************
 ** 函 数 名：_extufProPTSLEEP
 ** 功    能：延时 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 延时时间
 ** 返 回 值： 0 成功 aResult 
***************************************************************/
int _extufProPTSLEEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	
	strcpy(aResult,"\0");
	if (iParmCount!=1) {
		swVdebug(0,"S9001:_extufProPTSLEEP 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));
	strcpy(alTmp,aParm[0]);

	if (atoi(alTmp)>1000){
		swVdebug(0,"_extufProPTSLEEP 睡眠时间[%d]超长",atoi(alTmp));
		return -1;
	}	
	sleep(atoi(alTmp));
	ilLen=strlen(alTmp);
	strcpy(aResult,alTmp);
	swVdebug(3,"S8273: [函数返回] _extufProPTSLEEP()返回码=0,结果=[%s]len[%d]",aResult,ilLen);
	return(0);
}

/**************************************************************
 ** 函 数 名: _extufProPTMACRO
 ** 功    能：带缺省值的宏替换
 ** 作    者： 
 ** 建立日期：2006/12/18
 ** 修改日期：2001/12/18
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： 
 ** 返 回 值： 0
***************************************************************/	
int _extufProPTMACRO(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	FILE *fp;
	int iIndexLen;
	int iSectionLen;
	int iFind;
	int i;
	char aBuf[512];
	
	_swTrim(aParm[0]);
	_swTrim(aParm[1]);
	iIndexLen = strlen(aParm[0]);
	iSectionLen = strlen(aParm[1]);
	
	if (iParmCount!=2) {
		strcpy(aResult,"\0");
		swVdebug(0,"S9001:_extufProPTMACRO 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
		
	if((iIndexLen == 0)||(iSectionLen == 0))
		return(-1);
	
	if ((fp = _swFopenlist("config/PTMACRO.CFG", "r")) == NULL)
	{
		if (errno != ENOENT)
			return(FAIL);
		else
			return(SUCCESS);
	}
	
	/* 寻找匹配条件 */
	iFind =-2;
	while( fgets(aBuf,sizeof(aBuf) - 1,fp)) {
		if (*aBuf == '#')
			continue;
		if ( (*aBuf=='[') && (*(aBuf+iIndexLen+1)==']') && \
		    (!memcmp(aBuf+1,aParm[0],iIndexLen)) ) {
			iFind =0;
			break;
		}
	}
	if (iFind) {
		fclose(fp);
		return (-2);
	}
	
	/* 寻找匹配数据 */
	iFind =-3;
	while( fgets(aBuf,sizeof(aBuf),fp)) {

		if (*aBuf=='[' )
		{
			fclose(fp);
			strcpy(aResult,"");
			return (0);
		}

		else if (*aBuf=='#')
			continue;

		else if ( (*aBuf==*aParm[1]) && ( *(aBuf+iSectionLen)=='=') && \
			(!memcmp(aBuf,aParm[1],iSectionLen)) ) {
			for (i = iSectionLen;i<= strlen(aBuf);i++){			
				if (aBuf[i] == '#'|| aBuf[i]=='\n'){
					aBuf[i] = 0x00;					
					break;
				}
			}
			iFind = 0;
			strcpy(aResult,aBuf+iSectionLen+1);
			aResult[strlen(aResult)]=0;
			_swTrim(aResult);
			break;
		}
	}	
	fclose(fp);
	if (iFind!=0)
		strcpy(aResult,"");
  swVdebug(3,"S8889: value=[%s],len=[%d]",aResult,strlen(aResult));
  swVdebug(2,"S8890: _extufProPTMACRO success!");
  		
	return 0;
}


/**************************************************************
 ** 函 数 名：_extufProPTSTRREPLACE
 ** 功    能：字符串替换 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 传入的源字符串
 **            aParm[1] 被替换的内容
 **            aParm[2] 替换成的内容
 ** 返 回 值： 0 成功 aResult 
***************************************************************/
int _extufProPTSTRREPLACE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	short ilSlen,ilDlen;
	char *p,*start;
	strcpy(aResult,"\0");
	if (iParmCount!=3) {
		swVdebug(0,"S9001:_extufProPTSTRREPLACE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	

	memset(alTmp,0x00,sizeof(alTmp));
	strcpy(alTmp,aParm[0]);
	ilLen=strlen(alTmp);
	
	if ((ilSlen=strlen(aParm[1]))<=0){
		strcpy(aResult,alTmp);
		swVdebug(3,"S8273: [函数返回] _extufProPTSTRREPLACE()返回码=0,结果=[%s]len[%d]",aResult,ilLen);		
	}	
	ilDlen=strlen(aParm[2]);
/*	start=strstr(alTmp,aParm[1]);*/
	start=alTmp;
	while ((p=strstr(start,aParm[1]))!=NULL) {
		ilLen=strlen(start);
		memcpy(p+ilDlen,p+ilSlen,ilLen-(p-start+ilSlen));
		memcpy(p,aParm[2],ilDlen);
		ilLen=ilLen-ilSlen+ilDlen;
		*(start+ilLen)=0;
		start=p+ilDlen;
	}
	
	ilLen=strlen(alTmp);
	strcpy(aResult,alTmp);
	swVdebug(3,"S8273: [函数返回] _extufProPTSTRREPLACE()返回码=0,结果=[%s]len[%d]",aResult,ilLen);
	return(0);
}


/*根据传入的过滤串处理数据*/
int _DealDataFiltrate(char *aOneData,char *aFormat){
	char aTmp[32];
	char aData[1024];
	char alFormat[32];
	char *p;
	int iFlag;
	int formatlen=0;
	int ilRc;
	int start;
	int end;
	char sFiltrate[100][32];
	char aFiltFlag[32];
	int  iFiltNum;
	int iIncFlag; 
	int iReturn;
	int i;
	
	
	
		
	memset(aTmp,0x00,sizeof(aTmp));
	memset(aData,0x00,sizeof(aData));
	memset(aFiltFlag,0x00,sizeof(aFiltFlag));
	_swTrim(aOneData);
	strcpy(aData,aOneData);
	/*
	strcpy(aTmp,aFormat);
	_swTrim(aTmp);
	if (strlen(aTmp)==0) aTmp[0]='N';
	*/
swVdebug(2,"add by pc aFormat[%s]",aFormat);	
	iFiltNum=-1;
	p=strtok(aFormat,":");
	if (p!=NULL) {
		strcpy(aFiltFlag,p);
		_swTrim(aFiltFlag);
		
		while ((p=strtok(NULL,":"))!=NULL) {
				
			iFiltNum++;
			strcpy(sFiltrate[iFiltNum],p);
			_swTrim(sFiltrate[iFiltNum]);
			
			
		}
		
	}	
	
	switch (toupper(aFiltFlag[0])){
		case 'H':
			iIncFlag=0;
			for (i=0;i<=iFiltNum;i++)
			{
				swVdebug(5 ,"sFiltrate[%s] aData[%s]iFiltNum[%d]",sFiltrate[i],aData,iFiltNum);	
				if (strcmp(sFiltrate[i],aData) == 0)
				{
					iIncFlag=1;
					break;
				}
			}
			if (iIncFlag == 1) 
			{
				iReturn=0;
			}
			else
			{
				iReturn=1;
			}		
			break;
		case 'N':
			iIncFlag=0;
			for (i=0;i<=iFiltNum;i++)
			{
				if (strcmp(sFiltrate[i],aData) == 0)
				{
					iIncFlag=1;
					break;
				}
			}
			if (iIncFlag == 1) 
			{
				iReturn=1;
			}
			else
			{
				iReturn=0;
			}	
			break;
			
		default:
			iReturn=0;
			break;
	}
	return iReturn;
}



/**************************************************************
 ** 函 数 名：_extufProPTGETMATMULFILTRATE
 ** 功    能：处理多笔明细数据(带格式处理的) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 要处理的字符串buff
 **            aParm[1] 一共要处理多少列
 **            aParm[2] 一共要处理多少行数据 
 **            aParm[3] 各列宽度(用竖线分隔，例如 有3列 12|20|30|)
 **            aParm[4] 各列格式转换(用竖线分隔，例如 有3列 M:D12|C:FUND||)
 **            每列前两个字母代表转换规则：M: 代表金额转换 后跟转换格式
 **                                        C: 代表代码转换 后跟转换列关键字
 **                                        A: 代表代码转换，保留此列数据，并在后面增加一列中文描述 后跟关键字
 **            aParm[5] 各列是否需要(例如 有3列，只要前两列 1|1|0|)
 ** 返 回 值： 0 成功 aResult 处理后的多笔明细
***************************************************************/
int _extufProPTGETMATMULFILTRATE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int i,j;
	int col;
	int row;
	int colwidth[100];
	char alTmp[iFLDVALUELEN];
	char aColWidth[256];
	char aColWidth2[256];
	char aDealStr[iFLDVALUELEN];
	short iTotalLen=0;
	short iNeedLen=0;
	char sFormat[100][32];
	char aFormat[512];
	char aFormat2[512];	
	char aOneData[512];
	char aNeed[128];
	char aDealLine[iFLDVALUELEN];
	char sFiltrate[100][32];
	char aFiltrate[512];
	char aFiltrate2[512]; 
	
	char aTmp[32];
	char *p;
	int spos;
	int dpos;
	int lpos;
	int iFilFlag;
	
	if (iParmCount!=7) {
		strcpy(aResult,"\0");
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETMATMULFILTRATE 传入参数buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]fi[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5],aParm[6]);

	/*处理原报文中出现的 | 替换为 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [函数返回] _extufProPTGETMATMULFILTRATE()返回码=0,结果=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*处理宽度结构数组*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入宽度参数[%s]不合法",aColWidth);
		return -1;
	}
	colwidth[0]=atoi(p);
	iNeedLen+=colwidth[0];
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		colwidth[i]=atoi(p);
		iNeedLen+=colwidth[i];
		i++;
	}
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入列参数[%d]<>[%s]中竖线个数",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入数据长度[%d]<[%d]需要数据长度",iTotalLen,iNeedLen);
		return -3;
	}
	
	/*处理格式串*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入格式参数[%s]不合法",aFormat);
		return -1;
	}
	strcpy(sFormat[0],p);
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)>0) strcpy(sFormat[i],p);
		i++;
		if (i==col) break;
	}	
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入列参数[%d]<>[%s]格式串中竖线个数[%d]",col,aFormat2,i);
		return -2;
	}	
	/*处理需要数据域*/
	memset(aNeed,0x00,sizeof(aNeed));
	strcpy(aNeed,aParm[5]);	
	
	
/*处理过滤串*/
	memset((char *)&sFiltrate,0x00,sizeof(sFiltrate));
	memset(aFiltrate,0x00,sizeof(aFiltrate));
	strcpy(aFiltrate,aParm[6]);
	strcpy(aFiltrate2,aParm[6]);	
	p=strtok(aFiltrate,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入过滤条件参数[%s]不合法",aFiltrate);
		return -1;
	}
	strcpy(sFiltrate[0],p);
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)>0) strcpy(sFiltrate[i],p);
		i++;
		if (i==col) break;
	}	
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE 传入列参数[%d]<>[%s]过滤条件串中竖线个数[%d]",col,aFiltrate2,i);
		return -2;
	}	
	
	
	
	spos=0;
	dpos=0;
	memset(aDealStr,0x00,sizeof(aDealStr));
	
	for (j=0;j<row;j++){
		lpos=0;
		memset(aDealLine,0x00,sizeof(aDealLine));
		iFilFlag=0;
		for (i=0;i<col;i++){
			/*根据格式处理一个数据*/
			memset(aOneData,0x00,sizeof(aOneData));
			memcpy(aOneData,alTmp+spos,colwidth[i]);
			if (iFilFlag == 0)
			{
				memset(aTmp,0x00,sizeof(aTmp));
				strcpy(aTmp,sFiltrate[i]);
				iFilFlag=_DealDataFiltrate(aOneData,aTmp);
				if (iFilFlag > 0)
				{
					memset(aDealLine,0x00,sizeof(aDealLine));
					lpos=0;
				}
			}
			
			if (iFilFlag == 0)
			{
				if (aNeed[i*2]=='1') {
					/*需要此列数据*/
	
					swVdebug(3,"add by pc aOneData[%s]aFormat[%s]",aOneData,sFormat[i]);				
					_DealOneData(aOneData,sFormat[i]);
					memcpy(aDealLine+lpos,aOneData,strlen(aOneData));				
					lpos+=strlen(aOneData);					
				}
			}	
			spos+=colwidth[i];
			
		}
		swVdebug(3,"S9001 lpos=[%d] iFilFlag=[%d] aDealLine[%s]",lpos,iFilFlag,aDealLine);
		memcpy(aDealStr+dpos,aDealLine,strlen(aDealLine));				
		dpos+=strlen(aDealLine);
	}

	aDealStr[dpos]=0;
	_ufNtoT(aDealStr,dpos,aResult);

	swVdebug(3,"S8273: [函数返回] _extufProPTGETMATMULFILTRATE()返回码=0,结果=[%s]",aResult);
	return(0);
}



/**************************************************************
 ** 函 数 名：_extufProPTGETUTIME
 ** 功    能：得到当前毫秒级时间
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： 
 ** 返 回 值： 0 成功 aResult 返回的中文解释
***************************************************************/
int _extufProPTGETUTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	struct timeval curtime;	
	gettimeofday(&curtime,NULL);	
	sprintf(aResult,"%d.%03d",curtime.tv_sec,curtime.tv_usec/1000);	
	return(0);
}
/*****************************************************************************/
/*****************以下定义网银系统使用的平台函数************************/

#if 0
/**************************************************************
 ** 函 数 名：_extufProPBCHGERRCODE
 ** 功    能：转加密函数(压缩) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 系统代字
 **            aParm[1] 错误代码
 ** 返 回 值： 0 成功 aResult 转加后的前台错误代码
***************************************************************/
int _extufProPBCHGERRCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char syscode[4+1];
	char errcode[8+1];   /*后台响应码*/
	char retcode[8+1];   /*网银内部响应码*/
	char errmsg[80+1];
	
	strcpy(aResult,"\0");
	if (iParmCount!=2) {
		swVdebug(0,"S9001:_extufProPBCHGERRCODE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(syscode,0x00,sizeof(syscode));
	memset(retcode,0x00,sizeof(retcode));
	memset(errmsg,0x00,sizeof(errmsg));
	strcpy(syscode,aParm[0]);
	strcpy(errcode,aParm[1]);
	
	ilRc=PbGetRecocde(syscode, errcode,retcode,errmsg);
	if (ilRc){
		swVdebug(0,"S9001:PbGetRecocde error[%d]",ilRc);
		return -2;
	}
	
	if (psgMsghead->iMsgtype == iMSGUNPACK) 
		ilRc = swFmlset("retcode",strlen(retcode),retcode,psgUnpackbuf);
	else
		ilRc = swFmlset("retcode",strlen(retcode),retcode,psgPreunpackbuf);
	if (ilRc){
		swVdebug(0,"S9001 swFmlset retcode error[%d]",ilRc);
		return -3;
	}

		
	if (psgMsghead->iMsgtype == iMSGUNPACK) 	
		ilRc = swFmlset("errmsg",strlen(errmsg),errmsg,psgUnpackbuf);
	else	
		ilRc = swFmlset("errmsg",strlen(errmsg),errmsg,psgPreunpackbuf);
	if (ilRc){
		swVdebug(0,"S9001 swFmlset errmsg error[%d]",ilRc);
		return -3;
	}	
	swVdebug(3,"PbGetRecocde [%s] [%s] -> [%s][%s]",syscode, errcode,retcode,errmsg);
	ilLen=strlen(retcode);
	strcpy(aResult,retcode);
	swVdebug(3,"S8273: [函数返回] _extufProPBCHGERRCODE()返回码=0,结果=[%s]len[%d]",aResult,ilLen);

	return(0);
}


#endif


/**************************************************************
 ** 函 数 名：_extufProPBGETFOREIGNTRANCODE
 ** 功    能：获得外币交易码－用于外币应答后的解包区分格式转换动作
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] ISO(1)-交易类型 - 0210 成功应答，0200 正常请求，
 **                                       9999 错误应答(报文头响应码异常)
 **            aParm[1]:ISO(4)-原始交易码
 **            aParm[2]:ISO(39)-响应码
 ** 返 回 值： 0 成功 aResult计算出的交易码
 **            正常的请求和应答，交易码不变
 **            报文头响应码异常，交易码+“9”
 **            39号域响应码异常，交易码+“1”
***************************************************************/
int _extufProPBGETFOREIGNTRANCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char msgtype[4+1];
	char trancode[6+1];
	char retcode[6+1];	/* 2009-11-23 17:10 */
	char alTrancode[11+1];
	int  ilMsgtype;
	if (iParmCount!=3) {
		swVdebug(0,"S9001:_extufProPBGETFOREIGNTRANCODE 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(msgtype,0x00,sizeof(msgtype));
	memset(trancode,0x00,sizeof(trancode));	
	memset(retcode,0x00,sizeof(retcode));	
	memset(alTrancode,0x00,sizeof(alTrancode));
	strcpy(msgtype,aParm[0]);
	strcpy(trancode,aParm[1]);
	strcpy(retcode,aParm[2]);
	_swTrim(retcode);
	
	ilMsgtype = atoi(msgtype);
	/*应用响应*/
	if ( ilMsgtype==210 ) 
	{
		if ( atoi(retcode)==0 )
		{
			/*成功响应*/
			strcpy(alTrancode,trancode);
		}
		else
		{
			/*错误响应*/
			strcpy(alTrancode,trancode);
			strcat(alTrancode,"1");			
		}	
	}
	else if ( ilMsgtype==200 )
	{
		/*正常请求*/
		strcpy(alTrancode,trancode);
	}
	else if ( ilMsgtype==300 )
	{
		/*文件下发通知请求交易*/
		strcpy(alTrancode,trancode);
	}	
	else if ( ilMsgtype==9999 )
	{
		/*安全错误、格式错误*/
		strcpy(alTrancode,trancode);
		strcat(alTrancode,"9");		
	}
	else
	{
		swVdebug(3,"S8488: _extufProPBGETFOREIGNTRANCODE msgtype[%s] error",msgtype);
	}
	strcpy(aResult,alTrancode);

	swVdebug(3,"S8273: [函数返回] _extufProPBGETFOREIGNTRANCODE()返回码=0,结果=[%s]",aResult);
	return(0);
}

/*根据传入的格式串处理数据*/
void _DealOneDataSep(char *aOneData,char *aFormat,char *Sep){
	char aTmp[32];
	char aData[1024];
	char alFormat[32];
	char *p;
	int iFlag;
	int formatlen=0;
	int ilRc;
	int start;
	int end;
		
	memset(aTmp,0x00,sizeof(aTmp));
	memset(aData,0x00,sizeof(aData));
	_swTrim(aOneData);
	strcpy(aData,aOneData);
	strcpy(aTmp,aFormat);
	_swTrim(aTmp);
	if (strlen(aTmp)==0) aTmp[0]='N';
swVdebug(2,"add by pc aFormat[%s]",aFormat);	
	p=strchr(aFormat,':');
	if (p!=NULL) {
		formatlen=strlen(aFormat)-(p-aFormat)-1;
		memset(alFormat,0x00,sizeof(alFormat));
		memcpy(alFormat,p+1,formatlen);
	}	
swVdebug(2,"type[%c]format[%s]",toupper(aTmp[0]),alFormat);	
	switch (toupper(aTmp[0])){
		case 'M':
			if (formatlen==0) strcat(aOneData,Sep);
			else {
				_swTrim(aData);
				_DealAmount(aData);
				ilRc=swAmount(aOneData,aData,alFormat);	
				if (ilRc) {
					swVdebug(0,"S9001:swAmount 传入参数[%s] [%s]不合法，未处理数据",aData,alFormat);
					strcat(aOneData,Sep);
					break;
				}
				_swTrim(aOneData);
				strcat(aOneData,Sep);
			}		
			break;
		case 'C':
			if (formatlen==0) strcat(aOneData,Sep);
			else {
				/*_GetNameByCode(alFormat,aOneData,aData);
				_GetShmNameByCode(alFormat,aOneData,aData);
				strcpy(aOneData,aData);*/
				strcat(aOneData,Sep);
			}				
			break;
		case 'A':
			if (formatlen==0) strcat(aOneData,Sep);
			else {
				/*_GetNameByCode(alFormat,aOneData,aData);
				_GetShmNameByCode(alFormat,aOneData,aData);
				strcat(aOneData,Sep);				
				strcat(aOneData,aData);*/
				strcat(aOneData,Sep);
			}				
			break;
		case 'R':
			/*实现增加一列，同时后面不加竖线*/
			if (formatlen==0) {
				/*strcat(aOneData,Sep);*/
			}
			else {
				/*_GetNameByCode(alFormat,aOneData,aData);
				_GetShmNameByCode(alFormat,aOneData,aData);
				strcat(aOneData,Sep);				
				strcat(aOneData,aData);*/
				/*strcat(aOneData,Sep);*/
			}				
			break;			
		case 'N':
			if (formatlen==0) strcat(aOneData,Sep);
			else {
				_swTrim(aData);
				_DealAmount(aData);

				memset(aTmp,0x00,sizeof(aTmp));
				
/*				swVdebug(5,"aData[%s]",aData);*/
				if (aData[0]=='-'){
					strcpy(aTmp,"-");
					aData[0]='0';
				}
				else {
					strcpy(aTmp,"+");
				}		
				ilRc=swAmount(aOneData,aData,alFormat);	
				if (ilRc) {
					swVdebug(0,"S9001:swAmount 传入参数[%s] [%s]不合法，未处理数据",aData,alFormat);
					strcat(aOneData,Sep);
					break;
				}
/*				swVdebug(5,"aOneData[%s]aData[%s]alFormat[%s]",aOneData,aData,alFormat);*/
				_swTrim(aOneData);
				if (aTmp[0]=='-'){
					strcpy(aData,aOneData);
					sprintf(aOneData,"-%s|",aData);
				}	
				else	
					strcat(aOneData,Sep);
			}		
			break;			
		default:
			strcat(aOneData,Sep);
			break;
	}
	return;
}

/**************************************************************
 ** 函 数 名：_extufProPTGETFMTMULDATASEP
 ** 功    能：处理多笔明细数据(带格式处理的，自定义分隔符) 
 ** 作    者： 
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 要处理的字符串buff
 **            aParm[1] 一共要处理多少列
 **            aParm[2] 一共要处理多少行数据 
 **            aParm[3] 各列宽度(用竖线分隔，例如 有3列 12|20|30|)
 **            aParm[4] 各列格式转换(用竖线分隔，例如 有3列 M:D12|C:FUND||)
 **            每列前两个字母代表转换规则：M: 代表金额转换 后跟转换格式
 **                                        C: 代表代码转换 后跟转换列关键字
 **                                        A: 代表代码转换，保留此列数据，并在后面增加一列中文描述 后跟关键字
 **            aParm[5] 各列是否需要(例如 有3列，只要前两列 1|1|0|)
 **            aParm[6] 分隔符号 (不传入默认是空格竖线分隔)
 ** 返 回 值： 0 成功 aResult 处理后的多笔明细
***************************************************************/
int _extufProPTGETFMTMULDATASEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int i,j;
	int col;
	int row;
	int colwidth[100];
	char alTmp[iFLDVALUELEN];
	char aColWidth[256];
	char aColWidth2[256];
	char aDealStr[iFLDVALUELEN];
	short iTotalLen=0;
	short iNeedLen=0;
	char sFormat[100][32];
	char aFormat[512];
	char aFormat2[512];	
	char aOneData[512];
	char aNeed[128];
	char aSep[8];
	
	char aTmp[32];
	char *p;
	int spos;
	int dpos;
	
	if (iParmCount<6) {
		strcpy(aResult,"\0");
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETFORMATMULDATA 传入参数buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

	/*处理原报文中出现的 | 替换为 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [函数返回] _extufProPTGETFORMATMULDATA()返回码=0,结果=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*处理宽度结构数组*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入宽度参数[%s]不合法",aColWidth);
		return -1;
	}
	colwidth[0]=atoi(p);
	iNeedLen+=colwidth[0];
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		colwidth[i]=atoi(p);
		iNeedLen+=colwidth[i];
		i++;
	}
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入列参数[%d]<>[%s]中竖线个数",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入数据长度[%d]<[%d]需要数据长度",iTotalLen,iNeedLen);
		return -3;
	}
	
	/*处理格式串*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入格式参数[%s]不合法",aFormat);
		return -1;
	}
	strcpy(sFormat[0],p);
	i=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)>0) strcpy(sFormat[i],p);
		i++;
		if (i==col) break;
	}	
	if (i!=col){
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA 传入列参数[%d]<>[%s]格式串中竖线个数[%d]",col,aFormat2,i);
		return -2;
	}	
	/*处理需要数据域*/
	memset(aNeed,0x00,sizeof(aNeed));
	strcpy(aNeed,aParm[5]);	
	/*得到分隔符号*/
	memset(aSep,0x00,sizeof(aSep));
	if (iParmCount==7){
		strcpy(aSep,aParm[6]);
	}
	else {
		/*新函数的需求是为了加空格竖线，为了格式转换变动最小，此处默认值为" |"*/
		strcpy(aSep," |");
	}
	
	spos=0;
	dpos=0;
	memset(aDealStr,0x00,sizeof(aDealStr));

	for (j=0;j<row;j++){
		for (i=0;i<col;i++){
			/*根据格式处理一个数据*/
			if (aNeed[i*2]=='1') {
				/*需要此列数据*/
				memset(aOneData,0x00,sizeof(aOneData));
				memcpy(aOneData,alTmp+spos,colwidth[i]);
swVdebug(3,"add by pc aOneData[%s]aFormat[%s]",aOneData,sFormat[i]);				
				_DealOneDataSep(aOneData,sFormat[i],aSep);
				memcpy(aDealStr+dpos,aOneData,strlen(aOneData));				
				dpos+=strlen(aOneData);
			}
			spos+=colwidth[i];
			/*
			memcpy(aDealStr+dpos,alTmp+spos,colwidth[i]);
			dpos+=colwidth[i];
			spos+=colwidth[i];
			memcpy(aDealStr+dpos,"|",1);
			dpos++;
			*/
					swVdebug(3,"S9001 colwidth[%d]=[%d] aDealStr[%s]",i,colwidth[i],aDealStr);
		}
	}

	aDealStr[dpos]=0;
	_ufNtoT(aDealStr,dpos,aResult);

	swVdebug(3,"S8273: [函数返回] _extufProPTGETFORMATMULDATA()返回码=0,结果=[%s]",aResult);
	return(0);
}









/*2010-6-27 add by pc 为人行网银增加新的平台函数*/

int swTrancodeIsFull(short q_target,char *aTrancode, int limitNum)
{
  short ilTotalcount;              /* 总数 */
  short ilRc;       /* 返回码 */
  int ilCurrNum=0;
  int i;
  
  struct swt_tran_log pslSwt_tran_log[iMAXRECORD];
/*  
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"共享内存检测出错!\n");
    return(FAIL);
  }
*/
  ilRc = swShmselect_swt_tran_log_all(pslSwt_tran_log,&ilTotalcount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"[错误/共享内存] 执行swShmselect_swt_tran_log_all失败");
    return(FAIL);
  }
  
  /*
正在处理	0
处理完毕	1
处理超时	2
正在进行冲正	3
冲正完毕	4
冲正失败	5
冲正等待	6
SAF处理	7
  */
  swVdebug(3,"交易流水总数[%d] q_target[%d] aTrancode[%s] limitNum[%d]",ilTotalcount,q_target,aTrancode,limitNum);
  for(i=0;i<ilTotalcount;i++)
  {
/*
  	swVdebug(2,"q_target[%d] tc_tran_begin[%s] tran_status[%d]",pslSwt_tran_log[i].q_target,pslSwt_tran_log[i].tc_tran_begin,pslSwt_tran_log[i].tran_status);
*/
  	/*轮训正在处理的交易，检查目标邮箱与交易码与传入相同的正在处理的数量*/
  	if ((pslSwt_tran_log[i].q_target==q_target)&&\
  		 (strstr(aTrancode,pslSwt_tran_log[i].tc_tran_begin)!=NULL) &&\
  		 pslSwt_tran_log[i].tran_status==0)
  	{
  		ilCurrNum++;
  		if (ilCurrNum>limitNum) {
  			swVdebug(0,"交易并发超过限制%d",limitNum);
  			return -1;
  		}
  	}
  }
  swVdebug(3,"当前交易数[%d]",ilCurrNum);
  return 0; 
}



/**************************************************************
 ** 函 数 名：_extufProPBTRANISFULL
 ** 功    能：取某交易指定邮箱是否达到并发数
 ** 作    者：  
 ** 建立日期：2009/09/17
 ** 修改日期：
 ** 调用函数：          
 ** 全局变量：                  
 ** 参数含义：参数1:指定后台邮箱  参数2:指定交易码 参数3:并发数
 ** 返 回 值： 0        
***************************************************************/
int _extufProPBTRANISFULL(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{                               
	short   iQid;
	char aTrancode[10+1];
	int ilLimit;
	int     ilRc;
          	
	iQid = atoi(aParm[0]);
	memset(aTrancode,0x00,sizeof(aTrancode));
	strcpy(aTrancode,aParm[1]);
	ilLimit= atoi(aParm[2]);
	
	ilRc =  swTrancodeIsFull( iQid,aTrancode,ilLimit );
	if ( ilRc )
	{
		strcpy(aResult,"1");
	    return(0);
	}
	
	strcpy(aResult,"0");
	return(0);
}

/*2011/5/13 add by pc 处理返回地址中有半个汉字问题*/

int IsVaildChnStr(char *s)
{
	int len;
	int count;
	
	unsigned char *p;
	
	len=strlen(s);
	p=(unsigned char *)s;
	
	
	if (len>iFLDVALUELEN) return -1;
	count=0;
	
	
	while ((*p!='\0')&&(count<iFLDVALUELEN)) {
	/*	fprintf(stderr,"*p=[%c][%d]\n",*p,*p);*/
		if ((*p>160) && (*p<=255)) {
			count++;
	
		}
		else {
			if ((count%2)==1){
	/*			fprintf(stderr,"含有不合法中文1");*/
				return -1;			
			}
			else
					count=0;
		}		
		p++;
	}/*while*/
	fprintf(stderr,"count=%d\n",count);
	if ((count%2)==1){
	/*	fprintf(stderr,"含有不合法中文");*/
		return -1;
	}
	else {
		/*	fprintf(stderr,"无不合法中文");*/
			return 0;
		}

}

/**************************************************************
 ** 函 数 名：_extufProPBGETVALIDCHN
 ** 功    能：处理不合法的中文字符串
 ** 作    者：  
 ** 建立日期：2011/5/13
 ** 修改日期：
 ** 调用函数：          
 ** 全局变量：                  
 ** 参数含义：参数1:待处理字符串
 ** 返 回 值： 0 成功 失败
***************************************************************/
int _extufProPBGETVALIDCHN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{                               
	int     ilRc;
          	

	ilRc =  IsVaildChnStr( aParm[0] );
	if ( ilRc )
	{
		strcpy(aResult,"含有不合法中文字符");
	  return(0);
	}
	
	strcpy(aResult,aParm[0]);
	return(0);
}

/*2011/5/23 处理新柜面上送的开通业务标志，使它和旧的系统兼容*/

/*
新电子银行柜面上送开通标志－电话银行部分       旧电话银行柜面上送开通标志
1	转账	0：不办理,  1：办理，                  第1位 电话转账            
2	汇款	0：不办理,  1：办理                    第2位 汇款业务            
3	支付	0：不办理,  1：办理                    第3位 保险业务            
4	缴费	0：不办理,  1：办理                    第4位 缴费充值            
5	投资理财	0：不办理,  1：办理                第5位 第三方存管          
6	外币业务	0：不办理,  1：办理                第6位 理财业务            
7	指定转账	0：不办理,  1：办理                第7位 外汇业务            
8	指定汇款	0：不办理,  1：办理                第8位 关联标志            
9	保留	0：不关联,  1：关联                    其他预留。                
10	保留	0：不办理， 1：办理
*/

/**************************************************************
 ** 函 数 名：_extufProPBGETOPENFLAG
 ** 功    能：处理开通标志字段，使其和以前兼容
 ** 作    者：  
 ** 建立日期：2011/5/23
 ** 修改日期：
 ** 调用函数：          
 ** 全局变量：                  
 ** 参数含义： 参数1:开通功能 ISO(20)
 ** 返 回 值： 0 成功 失败
***************************************************************/
int _extufProPBGETOPENFLAG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{       
	                        
	swVdebug(3,"S8273: [函数返回] _extufProPBGETOPENFLAG() [%s][%s]",aParm[0]);	
  
  /*卡折标志*/
  aResult[0]=aParm[0][9];
  /*转账*/
  aResult[1]=aParm[0][6];
	/*汇款*/
	aResult[2]=aParm[0][7];
	/*保险*/
	aResult[3]=aParm[0][4];
	/*缴费充值*/
	aResult[4]=aParm[0][3];
	/*第三方存管*/
	aResult[5]=aParm[0][4];
	/*理财业务*/
	aResult[6]=aParm[0][4];
	/*外汇业务*/
	aResult[7]=aParm[0][5];
	/*关联标志*/
	aResult[8]=aParm[0][8];
	/*保留 － 保留字段，只能取 10*/
	aResult[9]=aParm[0][10];

	aResult[10]=aParm[0][10];
	aResult[11]=aParm[0][11];
	aResult[12]=aParm[0][12];
	aResult[13]=aParm[0][13];
	aResult[14]=aParm[0][14];
	aResult[15]=aParm[0][15];		
	aResult[16]=0;
	
	swVdebug(3,"S8273: [函数返回] _extufProPBGETOPENFLAG()返回码=0,结果=[%s]",aResult);	
	
	return(0);
}


/**************************************************************
 ** 函 数 名：_extufProPBTransferID
 ** 功    能：把15位身份证 转为18位身份证 
 ** 作    者：YZ 
 ** 建立日期：2011/6/13
 ** 修改日期：
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 15位身份证 
 ** 返 回 值： 0 成功 aResult 18位身份证 
***************************************************************/
int _extufProPBTransferIDNO(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	char Input[18+1];
	char Output[18+1];        
  const int Wi[18] = {7, 9, 10, 5, 8,4, 2, 1, 6, 3, 7, 9, 10, 5, 8, 4, 2, 1};
  const char ai[11] = {'1', '0', 'X', '9', '8', '7', '6', '5', '4', '3', '2'};
  int i,sum,mod;

	strcpy(aResult,"\0");
	if (iParmCount!=1) {
		swVdebug(0,"S9001:_extufProPBTransferIDNO 传入参数个数[%d]不合法",iParmCount);
		return -1;
	}

	memset(Input, 0x00, sizeof(Input));
	memset(Output, 0x00, sizeof(Output));
	
	strcpy(Input,aParm[0]);
  strncpy(Output, Input, 15);

  /* Output的7-15位依次后移2位 */
  for(i=16; i>7; i--)
  {
		Output[i] = Output[i-2];
  }

  /* Output的7，8位插入'1'，'9' */
  Output[6] = '1';
  Output[7] = '9';

  /* 求前17位位权和 */
  sum = 0;
  mod = 0;
  for(i=0; i<17; i++)
  {
		sum = sum + (Output[i]-48)*Wi[i];
  }

  /* 校验码字符值 */
  mod = sum % 11;
  Output[17] = ai[mod];
  
	strcpy(aResult,Output);
	swVdebug(3,"S8273: [函数返回] _extufProPBTransferIDNO()返回码=0,结果=[%s]",aResult);  
  return(0);
}

/**************************************************************
 ** 函 数 名： _extufProPBGET8583CHECK
 ** 功    能：得到位元是否存在
 ** 作    者：YZ
 ** 建立日期：2011/6/20 15:06:07
 ** 修改日期：
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0]  参加位元检查的域 形式为: 3|12|13|
 ** 返 回 值： 0 成功 
               返位元号 失败
***************************************************************/
int _extufProPBGET8583CHECK(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int ilRc;
	char aIsoField[256];
	int sField[128];
	int ilTotalNum;
	char *p;
	int i;
	int iTotalLen;
	struct isoprebuf
	{
		short iFldid;
		short iFldlen;
		char  aFldvalue[iFLDVALUELEN + 1];
		char  aOrgfldlen[10];
		short iOrgfldlen;
		char  aOrgfldvalue[iFLDVALUELEN + 1];
	} slIsobuf;

	strcpy(aIsoField,aParm[0]);
	p=strtok(aIsoField,"|");
	if (p==NULL) {
		swVdebug(0,"_extufProPBGET8583CHECK 传入参数[%s]不合法",aIsoField);
		return -3;
	}
	sField[0]=atoi(p);
	/*循环扫描传入参数，获知要检查的位元号，并附给位元数据对应位置*/
	ilTotalNum=1;
	while ((p=strtok(NULL,"|"))!=NULL) {
		if (strlen(p)<=0) break;
		sField[ilTotalNum]=atoi(p);
		ilTotalNum++;
	}
	for (i=0;i<ilTotalNum;i++)
	{
		swVdebug(3,"[%d] [%d]",i,sField[i]);
	}
	swVdebug(3,"共[%d]个域参与检查",ilTotalNum);

	iTotalLen=0;
	memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
	for (i=0;i<ilTotalNum;i++) {
		if (psgMsghead->iMsgtype == iMSGUNPACK)
			ilRc=swIsoget(psgPreunpackbuf,sField[i],&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
		else	 
			ilRc=swIsoget(psgUnpackbuf,sField[i],&(slIsobuf.iFldlen),slIsobuf.aFldvalue);

		if (ilRc){
			swVdebug(0,"_extufProPBGET8583CHECK  swIsoget get field[%d] error[%d]",sField[i],ilRc);
			return sField[i];
		}
		
	}
	strcpy(aResult,"0");
	swVdebug(3,"[函数返回] _extufProPBGET8583CHECK()返回码=0,结果=[%s]len[%d]",aResult,iTotalLen);
	return(0);
}

/**************************************************************
 ** 函数名      ： _extufProSTRFILL
 ** 功  能      ： 用字符串填充字符串
 ** 作  者      ： PC 
 ** 建立日期    ： 2011/6/24 15:58
 ** 最后修改日期： 2011/6/24 15:58
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 源数据串
 **                aParm[1] : 填充用字符串
 **                aParm[2] : 补充后长度
 **                aParm[3] : 左补右补标志
 ** 返回值      ： 所求的串
***************************************************************/
int _extufProSTRFILL(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char aFiller[iFLDVALUELEN];
  char cJustify;
  short iFilllen;
  char alFillbuf[iFLDVALUELEN];
  short ilBuflen;
  short istart;
	short iSublen;
	int i;
	
  swVdebug(3,"S8168: [函数调用] _extufProSTRFILL(%s,%s,%s,%s)",aParm[0],aParm[1],aParm[2],aParm[3]);
  iFilllen=atoi(aParm[2]);

	cJustify=aParm[3][0];	
	istart=0;
	if(cJustify=='L') {
		for(i=0;i<iFilllen/strlen(aParm[1]);i++){
			memcpy(alFillbuf+istart,aParm[1],strlen(aParm[1]));
			istart+=strlen(aParm[1]);
		}
	}
	else {
		istart=strlen(aParm[0]);		
		for(i=0;i<iFilllen/strlen(aParm[1]);i++){
			memcpy(alFillbuf+istart,aParm[1],strlen(aParm[1]));
			istart+=strlen(aParm[1]);
		}		
	}
	if (i==0){
		memcpy(alFillbuf+istart,aParm[1],strlen(aParm[1]));
	}
	alFillbuf[iFilllen]=0;
	ilBuflen=strlen(aParm[0]);

	
	if (iFilllen>=ilBuflen) {
    if (cJustify=='R')
      memcpy(alFillbuf, aParm[0], ilBuflen);
    else if(cJustify=='L')
      memcpy(alFillbuf+(iFilllen-ilBuflen), aParm[0], ilBuflen);
    else
      return(-1);
	  alFillbuf[iFilllen]='\0';
	  _ufNtoT(alFillbuf,iFilllen,aResult);        
   }
   else {
   		memcpy(alFillbuf,aParm[0],ilBuflen);
		  alFillbuf[ilBuflen]='\0';
		  _ufNtoT(alFillbuf,ilBuflen,aResult);   		
   }     
  swVdebug(3,"S8171: [函数返回] _extufProSTRFILL()返回码=0,结果=%s",aResult);
  return(0);
}


/*****************************add by pc 2013/1/6 23:11 for GSMP ***********************/

/**************************************************************
 ** 函数名      ： _extufProSETINT
 ** 功  能      ： 字符串转为2进制类型值
 ** 作  者      ： PC 
 ** 建立日期    ： 2013/1/6 22:38
 ** 最后修改日期： 2013/1/6 22:38
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
 **                aParm[1] : 打包的位数 1,2,4
 ** 返回值      ： 所求的串
***************************************************************/
int _extufProSETINT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	char c;
	short s,s2;
	int i,i2;
	char aTmp[8];
	
	memset(aTmp,0x00,sizeof(aTmp));
	if (atoi(aParm[1])==1){
		c=atoi(aParm[0]);
		memcpy(aTmp,(char *)&c,1);
		_ufNtoT(aTmp,1,aResult);
	}
	else if (atoi(aParm[1])==2){
		s=atoi(aParm[0]);
		s2=htons(s);
		memcpy(aTmp,(char *)&s2,2);
		_ufNtoT(aTmp,2,aResult);
	}
	else if (atoi(aParm[1])==4){
		i=atoi(aParm[0]);
		i2=htonl(i);
		memcpy(aTmp,(char *)&i2,4);
		_ufNtoT(aTmp,4,aResult);
	}
	return(0);
}

/**************************************************************
 ** 函数名      ： _extufProSETVARSTR
 ** 功  能      ： 字符串转为带4字节长度位的字符串
 ** 作  者      ： PC 
 ** 建立日期    ： 2013/1/6 22:38
 ** 最后修改日期： 2013/1/6 22:38
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
 ** 返回值      ： 所求的串
***************************************************************/
int _extufProSETVARSTR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	char aTmp[iFLDVALUELEN];
	char aParm0[iFLDVALUELEN];
	int len,len2;
        short ilStrlen;
        
	memset(aParm0,0x00,sizeof(aParm0));
        _ufTtoN(aParm[0],aParm0,&ilStrlen);

	
	len=ilStrlen;
	len2=htonl(len);
	memset(aTmp,0x00,sizeof(aTmp));
	memcpy(aTmp,(char *)&len2,4);
	memcpy(aTmp+4,aParm0,len);
	
	_ufNtoT(aTmp,4+len,aResult);
	
	return(0);
}
#if 0
int _extufProSETVARSTR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	char aTmp[iFLDVALUELEN];
	int len,len2;

	
	len=strlen(aParm[0]);
	len2=htonl(len);
	memset(aTmp,0x00,sizeof(aTmp));
	memcpy(aTmp,(char *)&len2,4);
	memcpy(aTmp+4,aParm[0],len);
	
	_ufNtoT(aTmp,4+len,aResult);
	
	return(0);
}
#endif

/**************************************************************
 ** 函数名      ： _extufProMD5
 ** 功  能      ： MD5算法
 ** 作  者      ： PC
 ** 建立日期    ： 2013/1/6 22:38
 ** 最后修改日期： 2013/1/6 22:38
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 加密用的字符串
 ** 参数含义    ： aParm[1] : 加密用的字符串的长度
  ** 返回值      ： 所求的串
***************************************************************/
int _extufProMD5(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
        char aTmp[iFLDVALUELEN];
        char alTmp[iFLDVALUELEN];
        int len,ilLen;
        
        char *s;
         
        swVdebug(3, "S0180: [函数调用] _extufProMD5(%s,%s)", aParm[0], aParm[1]);
        
        memset(alTmp,0x00,sizeof(alTmp));
        
        _ufTtoN(aParm[0], alTmp, &ilLen);

        len=atoi(aParm[1]);
        s=MD5(alTmp,len,NULL);

        memset(aTmp,0x00,sizeof(aTmp));
        memcpy(aTmp,s,16);

        _ufNtoT(aTmp, 16, aResult);
        swVdebug(3, "S0200: [函数返回] _extufProMD5()");

        return(0);
}

/**************************************************************
 ** 函数名      ： _extufProGETINT
 ** 功  能      ： 取4字节整行(网络转主机字节序)
 ** 作  者      ： douya
 ** 建立日期    ： 2013/1/6 22:38
 ** 最后修改日期： 2013/1/6 22:38
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 取数开始位置
 ** 参数含义    ： aParm[1] : 截取的长度
  ** 返回值      ： 所求的串
***************************************************************/
int _extufProGETINT(char aParm[][iFLDVALUELEN], short iParmCount, char *aResult)
{
        char alTmp[iFLDVALUELEN];

        unsigned int h32;
        unsigned int n32;
  int ilLen = 0;
  int ilOffset = 0;

   swVdebug(3, "S0180: [函数调用] _extufProGETINT(%s,%s)", aParm[0], aParm[1]);

   _ufTtoN(aParm[0], alTmp, &ilLen);
  ilOffset = atoi(alTmp) - 1;
  if (ilOffset < 0)
    ilOffset = 0;
        ilLen = 4;
        memcpy((char *)&n32,agMsgbody + ilOffset,ilLen);
        h32 = ntohl(n32);

        sprintf(aResult,"%d",h32);

  swVdebug(3, "S0200: [函数返回] _extufProGETINT()返回码=0,结果=%s", aResult);
  return (0);

}


/**************************************************************
 ** 函 数 名：_extufProMAC_APPLY
 ** 功    能：根据系统代字申请秘钥
 ** 作    者：    
 ** 建立日期：2015/04/10
 ** 修改日期：2015/04/10
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 系统代字
 **            aParm[1] macblock
 ** 返 回 值： 0 成功 aResult 计算的MAC
***************************************************************/
int _extufProMAC_APPLY(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
    short ilLen=0;
    short ilStrlen;
    int ilRc;
    char alTmp[iFLDVALUELEN];
    char syscode[64+1];
    char macblock[iFLDVALUELEN];
    char mac[16+1];

    igFormatHsmFlag=0;
    strcpy(aResult,"\0");
    if (iParmCount!=2) {
        swVdebug(0,"S9001:_extufProMAC_APPLY 传入参数个数[%d]不合法",iParmCount);
        return -1;
    }
    memset(syscode,0x00,sizeof(syscode));
    memset(macblock,0x00,sizeof(macblock));
    memset(mac,0x00,sizeof(mac));
    strcpy(syscode,aParm[0]);

    _ufTtoN(aParm[1],macblock,&ilStrlen);
    swVdebug(3,"S8273: [函数返回] _ufTtoN(macblock)[%s]",macblock);

    ilRc=HsmMAC_APPLY_New(syscode, macblock,strlen(macblock),mac);
    if (ilRc){ 
        swVdebug(0,"S9001:HsmMAC_APPLY error[%d]",ilRc);
        igFormatHsmFlag=-88;        
        return -2;
    }
    strcpy(aResult,mac);
    mac[16]=0;
    swVdebug(3,"S8273: [函数返回] _extufProMAC_APPLY()返回码=0,结果=[%s]",aResult);
    return(0);
}



/**************************************************************
 ** 函 数 名：_extufProMacByKey
 ** 功    能：根据外部输入密钥生成MAC
 ** 作    者：
 ** 建立日期：2001/11/28
 ** 修改日期：2001/11/28
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 系统代字
 **                        aParm[1] 密钥的密文
 **					aParm[2]  macbuf
 ** 返 回 值： 0 成功 aResult 计算的MAC
***************************************************************/

int _extufProMacByKey(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
    short ilLen=0;
    short ilStrlen;
    int ilRc;
    char alTmp[iFLDVALUELEN];
    
    char syscode[64+1];
    char encKey[32];
    char userData[2048];
    char macData[16+1];

    igFormatHsmFlag=0;
    strcpy(aResult,"\0");
    if (iParmCount!=3) {
        swVdebug(0,"S9001:_extufProMacByKey 传入参数个数[%d]不合法",iParmCount);
        return -1;
    }
    memset(syscode,0x00,sizeof(syscode));
    memset(encKey,0x00,sizeof(encKey));
    memset(userData,0x00,sizeof(userData));
    memset(macData,0x00,sizeof(macData));
  
    strcpy(syscode,aParm[0]);
    strcpy(encKey,aParm[1]);
    strcpy(userData,aParm[2]);

    ilRc=HsmGenMacByKey(syscode,encKey ,userData,macData);
    if (ilRc){
        swVdebug(0,"S9001:HsmMAC_APPLY error[%d]",ilRc);
        igFormatHsmFlag=-88;
        return -2;
    }
    swVdebug(3,"HsmGenMacByKey [%s] [%s] [%s] -> [%s]",syscode, encKey,userData,macData);
    strcpy(aResult,macData);
    macData[16]=0;
    swVdebug(3,"S8273: [函数返回] _extufProMAC_APPLY()返回码=0,结果=[%s]",aResult);
    return(0);
}




int ntohs_buf(uint16_t *inbuf, size_t inlen, uint16_t *outbuf, size_t *outlen)
{
        int i;
        for(i=0; i<inlen; ++i) {
                if(i > (*outlen)) {
                        break;
                }
                outbuf[i] = ntohs(inbuf[i]);
        }
        (*outlen) = i;

        return 0;
}


/**************************************************************
 ** 函 数 名：_extufProUCS2STR
 ** 功    能: 将ucs2编码报文转换为GBK 编码
 ** 作    者：
 ** 建立日期：2015/05/08
 ** 修改日期：2015/05/08
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] UCS2格式的字符串
 ** 返 回 值： 0 成功 aResult 转换后的结果
***************************************************************/

int _extufProUCS2STR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{       
        int ilLen=0;
	int ilStrlen;
	int ilRc;
	char ucs2_buf[iFLDVALUELEN];
	char out_buf[iFLDVALUELEN];
	xmlChar *tmpbuf = NULL;
	char *buf2;
        size_t out_len;	
	int i;

	char respcode[64];
        int respcodelen;
        int itmp;
        char altmp[1024];
        int itmplen;
        char reserrinfo_ucs2[1024];
        char reserrinfo_gbk[1024];
        char *pucs2;
        char *pgbk;
        size_t  ucs2_len;
        size_t  gbk_len;
        iconv_t cd;	


	strcpy(aResult,"\0");

	

	memset(reserrinfo_ucs2, 0x00, sizeof(reserrinfo_ucs2));
	memset(ucs2_buf,0x00,sizeof(ucs2_buf));
	_ufTtoN(aParm[0],altmp,&itmplen);	

	 swVdebug(3,"S8273: [函数返回] _ufTtoN(altmp)[%s]",altmp);	


	ucs2_len = (sizeof(reserrinfo_ucs2) -1)/2;
	ntohs_buf((uint16_t *)altmp, itmplen, (uint16_t *)reserrinfo_ucs2, &ucs2_len);

	//memset(reserrinfo_ucs2, 0x00, sizeof(reserrinfo_ucs2));
	memset(reserrinfo_gbk, 0x00, sizeof(reserrinfo_gbk));

	//ucs2_len = sizeof(reserrinfo_ucs2)-1;
	//swFmlpackget(slMsgpack.aMsgbody, slMsgpack.sMsghead.iBodylen, "ERRMSG", &ucs2_len, reserrinfo_ucs2);
	pucs2 = reserrinfo_ucs2;
	pgbk = reserrinfo_gbk;
	gbk_len = sizeof(reserrinfo_gbk)-1;
	cd = iconv_open("GB18030", "UCS-2");
	ucs2_len *= 2;
	//iconv（cd, &in, (size_t *)&inlen, &out,&outlen）
	ilRc = iconv(cd, &pucs2, &ucs2_len, &pgbk, &gbk_len);
	if(ilRc < 0) {
		swVdebug(1, "UCS2 CONVER TO GB18030 ERROR errno[%d], errmsg[%s]", errno, strerror(errno));
		iconv_close(cd);
	} else {
		swVdebug(5, "UCS2 CONVER TO GB18030 SUCCESS resperrinfo[%s]", reserrinfo_gbk);
		iconv_close(cd);
	}
	

	out_len=strlen(reserrinfo_gbk);
	_ufNtoT(reserrinfo_gbk,out_len,aResult);	

	swVdebug(3,"S8273: [函数返回] _extufProUCS2STR()返回码=0,结果=[%s]",aResult);
//#endif

	
	return 0;
}





/** add by douya 2015-04-17   end*******************/

int _extufProHASH(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{               
    unsigned int b    = 378551;
    unsigned int a    = 63689;
    unsigned int hash = 0;
    unsigned int i    = 0;


    for(i = 0; aParm[0][i] != '\0'; i++)
    {
       hash = hash * a + aParm[0][i];
       a = a * b;
    }
    sprintf(aResult,"%u",hash);
    return 0;
}       

char *FqGetTime( char *NowTime, int Len, const char *insertformat )
{
        time_t clock;

        clock = time((time_t *)0);
        strftime (NowTime,Len,insertformat,localtime(&clock));

        return (NowTime);
}

int _extufProGETCUSID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{               
    char t[14+1];
    char aTmp[32];
    memset(t, 0x00, sizeof(t));
    memset(aTmp,0x00,sizeof(aTmp));
    FqGetTime(t, 16, "%Y%m%d%H%M%S");
    sprintf(aTmp,"%ld",timetolong(t));
    memcpy(aResult,aTmp+1,9);
    return 0;
}       

int _extufProGETINDEXMB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{   
	  int ilRc;
	  char aMb[5+1];
	        
    ilRc = shm_ptr_check(1);
    if(ilRc)
    {
        swVdebug(0,"error shm_ptr_check");
        return(-1);
    }
    
    memset(aMb,0x00,sizeof(aMb));
    shm_index_table_route(aParm[0],aMb);
    _swTrim(aMb); 
    swVdebug(3,"aParm[%s] MB[%s]",aParm[0],aMb);  
    sprintf(aResult,"%s",aMb);
    return 0;
}   

int _extufProGETBASEMB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{   
	  int ilRc;
	  char aMb[5+1];
	        
    ilRc = shm_ptr_check(1);
    if(ilRc)
    {
        swVdebug(0,"error shm_ptr_check");
        return(-1);
    }
    
    memset(aMb,0x00,sizeof(aMb));
    shm_base_table_route(aParm[0],aMb);
    _swTrim(aMb); 
    swVdebug(3,"aParm[%s] MB[%s]",aParm[0],aMb);  
    sprintf(aResult,"%s",aMb);
    return 0;
}      