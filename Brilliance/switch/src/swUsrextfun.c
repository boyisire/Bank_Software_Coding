/****************************************************************/
/* ģ���ţ�USREXTFUN                                          */
/* ģ�����ƣ���չƽ̨����                                       */
/* ��	 �ߣ�                                                   */
/* �������ڣ�2001/11/28                                         */
/* �޸����ڣ�2001/11/28                                         */
/* ģ����;��ʵ�ֶ���չ�����Ĵ���                               */
/* ��ģ���а������º���������˵����                             */
/****************************************************************/
/*           int  _ufUsrextfunDemo();                           */
/****************************************************************/
/* �޸ļ�¼��                                                   */
/****************************************************************/
/* switch���� */
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
 ** �� �� ����_ufUsrextfunDemo
 ** ��    �ܣ���չƽ̨�������� 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 
 ** �� �� ֵ�� 0
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
 ** �� �� ����_extufProPTSHMGETNAMEBYCODE
 ** ��    �ܣ�ͨ������ȡ�����Ľ��� 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] �½�����
 **            aParm[1] code����
 ** �� �� ֵ�� 0 �ɹ� aResult ���ص����Ľ���
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
		swVdebug(0,"S9001:_extufProPTSHMGETNAMEBYCODE �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	_GetShmNameByCode(aParm[0],aParm[1],aResult);
	swVdebug(3,"S8273: [��������] _extufProPTSHMGETNAMEBYCODE()������=0,���=[%s]",aResult);
	return(0);
}

#endif


/*���ݴ���ĸ�ʽ����������*/
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
					swVdebug(0,"S9001:swAmount �������[%s] [%s]���Ϸ���δ��������",aData,alFormat);
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
			/*ʵ������һ�У�ͬʱ���治������*/
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
					swVdebug(0,"S9001:swAmount �������[%s] [%s]���Ϸ���δ��������",aData,alFormat);
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
 ** �� �� ����_extufProPTGETMULDATA
 ** ��    �ܣ���������ϸ����(���򣬸��ݴ��������ʽ������) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] Ҫ������ַ���buff
 **            aParm[1] һ��Ҫ���������
 **            aParm[2] һ��Ҫ������������� 
 **            aParm[3] ���п��(�����߷ָ������� ��3�� 12|20|30|)
 ** �� �� ֵ�� 0 �ɹ� aResult �����Ķ����ϸ
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
		swVdebug(0,"S9001:_extufProPTGETMULDATA �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	swVdebug(2,"S9001:_extufProPTGETMULDATA �������buf[%s]col[%s]row[%s]w[%s]",aParm[0],aParm[1],aParm[2],aParm[3]);
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
		
	/*����ԭ�����г��ֵ� | �滻Ϊ 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [��������] _extufProPTGETMULDATA()������=0,���=[%s]",aResult);		
		return 0;
	}		
	iTotalLen=strlen(alTmp);
	/*�����Ƚṹ����*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMULDATA �����Ȳ���[%s]���Ϸ�",aColWidth);
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
		swVdebug(0,"S9001:_extufProPTGETMULDATA �����в���[%d]<>[%s]�����߸���",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETMULDATA �������ݳ���[%d]<[%d]��Ҫ���ݳ���",iTotalLen,iNeedLen);
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
	swVdebug(3,"S8273: [��������] _extufProPTGETMULDATA()������=0,���=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** �� �� ����_extufProPTCONVERTPIN
 ** ��    �ܣ�ת���ܺ���(��ѹ��) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ϵͳ����
 **            aParm[1] �ۺ�/����
 **            aParm[2] ����PINBLOCK  
 ** �� �� ֵ�� 0 �ɹ� aResult ת���ܺ��PINBLOCK
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
		swVdebug(0,"S9001:_extufProPTCONVERTPIN �����������[%d]���Ϸ�",iParmCount);
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
	swVdebug(3,"S8273: [��������] _extufProPTCONVERTPIN()������=0,���=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** �� �� ����_extufProPTCONVERTPINBCD
 ** ��    �ܣ�ת���ܺ���(ѹ��) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ϵͳ����
 **            aParm[1] �ۺ�/����
 **            aParm[2] ����PINBLOCK  
 ** �� �� ֵ�� 0 �ɹ� aResult ת���ܺ��PINBLOCK
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
		swVdebug(0,"S9001:_extufProPTCONVERTPIN �����������[%d]���Ϸ�",iParmCount);
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
	swVdebug(3,"S8273: [��������] _extufProPTCONVERTPINBCD()������=0,���=[%s]len[%d]",aResult,ilLen);
	return(0);
}


/**************************************************************
 ** �� �� ����_extufProPTMACGEN
 ** ��    �ܣ�����ϵͳ��������MAC
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ϵͳ����
 **            aParm[1] macblock
 ** �� �� ֵ�� 0 �ɹ� aResult �����MAC
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
		swVdebug(0,"S9001:_extufProPTMACGEN �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	memset(syscode,0x00,sizeof(syscode));
	memset(macblock,0x00,sizeof(macblock));
	memset(mac,0x00,sizeof(mac));
	strcpy(syscode,aParm[0]);
	/*2010/11/9 add by pc Ϊ�˴�����ƽ̨�ڲ�ת���ַ�*/
	_ufTtoN(aParm[1],macblock,&ilStrlen);
	swVdebug(3,"S8273: [��������] _ufTtoN(macblock)[%s]",macblock);
	/*2010/11/9 add by pc Ϊ�˴�����ƽ̨�ڲ�ת���ַ�*/		
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
	swVdebug(3,"S8273: [��������] _extufProPTMACGEN()������=0,���=[%s]",aResult);
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
	/*2009-11-13 Ϊ����Ӧ����пո�����,�Ƚ���߿ո�ȥ��*/
	_swTrimL(Data);
	ilLen=strlen(Data);
	/*2009-11-13 Ϊ����Ӧ����пո�����,�Ƚ���߿ո�ȥ��*/	
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
 ** �� �� ����_extufProPTGET8583MACBUF
 ** ��    �ܣ��õ�MACBUF����
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0]  �μ�MAC������� ��ʽΪ: 3|12|13|
 ** �� �� ֵ�� 0 �ɹ� aResult ת���ܺ��PINBLOCK
***************************************************************/
int _extufProPTGET8583MACBUF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int ilRc;
	short ilTableId;                         /* ��id */
	short ilFldId;                           /* ��id */
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
		swVdebug(0,"S9001:_extufProPTGET8583MACBUF �������[%s]���Ϸ�",aMacField);
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
	swVdebug(2,"��[%d]����μ�Mac����",ilTotalNum);

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
	swVdebug(3,"S8273: [��������] _extufProPTGET8583MACBUF()������=0,���=[%s]len[%d]",aResult,iTotalLen);
	return(0);
}

/**************************************************************
 ** �� �� ����_extufProPTGET8583MACBUFCX
 ** ��    �ܣ��õ�MACBUF�����������򲻲μӼ���
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0]  �μ�MAC������� ��ʽΪ: 3|12|13|
 ** �� �� ֵ�� 0 �ɹ� aResult ת���ܺ��PINBLOCK
***************************************************************/
int _extufProPTGET8583MACBUFCX(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int ilRc;
	short ilTableId;                         /* ��id */
	short ilFldId;                           /* ��id */
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
		swVdebug(0,"S9001:_extufProPTGET8583MACBUF �������[%s]���Ϸ�",aMacField);
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
	swVdebug(2,"��[%d]����μ�Mac����",ilTotalNum);

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
	swVdebug(3,"S8273: [��������] _extufProPTGET8583MACBUF()������=0,���=[%s]len[%d]",aResult,iTotalLen);
	return(0);
}

/*ȥ�������У�û�õ��ַ�*/
/*���磺 1.2300->1.23   */
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
#     example: PTAMOUNT( 502.18, V ) --->   result: [5.0218]	��ɴ���� 100����
#     example: PTAMOUNT( 502.18, X ) --->   result: [50218]   ��ɴ���� 100����
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
		swVdebug(0,"S9001:_ufProPTAMOUNT �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	
	strcpy(alTmp, aParm[0]);
	_swTrim(alTmp);
	_DealAmount(alTmp);
	strcpy(fmt, aParm[1]);

	ilRc=swAmount(aResult, alTmp, fmt);
	if (ilRc){
		swVdebug(0,"S9001:swAmount �������[%s] [%s]���Ϸ�",alTmp,fmt);
		strcpy(aResult,"");
		return 0;
	}	
	
	swVdebug(3,"S8273: [��������] _ufProPTAMOUNT()������=0,���=[%s]len[%d]",aResult,strlen(aResult));
	
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
			swVdebug(0,"���������������");
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
			sprintf(amt,"��%s",tmp);
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
			swVdebug(0,"���������������");
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
		"Ԫ","��","��"	};
	static char numb2[][3]={
		"Ǫ","��","ʰ"	};
	static char chn[][3]={
		"��","Ҽ","��","��","��","��","½","��","��","��"	};

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
			strcpy(endbuf1, "��");
		} else {
			if (endbuf[0] == '0')
			{
				if (atoi(prechar) != 0) strcat(endbuf1, "��");
				strcat(endbuf1, chn[endbuf[1]-'0']);
				strcat(endbuf1, "��");
			} else if ((endbuf[1] == '0') || (endbuf[1] == 0x00)) {
				strcat(endbuf1, chn[endbuf[0]-'0']);
				strcat(endbuf1, "��");
			} else {
				strcat(endbuf1, chn[endbuf[0]-'0']);
				strcat(endbuf1, "��");
				strcat(endbuf1, chn[endbuf[1]-'0']);
				strcat(endbuf1, "��");
			}
		}
	} else {
		strcpy(endbuf1, "��");
		strcpy(prechar, curbuf);
	}

	numlen = strlen(prechar);
	if (atoi(prechar) == 0)
	{
		if (atoi(endbuf) == 0) strcat(outbuf, "��Ԫ");
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
				if (j == 0) strcat(outbuf, "Ԫ");
				continue;
			}
			numlen = strlen(prebuf[j]);
			if (numlen == 3)
			{
				if (memcmp(prebuf[j]+1, "00", 2) == 0)
				{
					strcat(outbuf, chn[prebuf[j][0]-'0']);
					strcat(outbuf, "��");
					strcat(outbuf, numb1[j]);
					continue;
				}
			} else if (numlen == 4) {
				if (memcmp(prebuf[j]+1, "000", 3) == 0)
				{
					strcat(outbuf, chn[prebuf[j][0] - '0']);
					strcat(outbuf, "Ǫ");
					strcat(outbuf, numb1[j]);
					continue;
				}
				if (memcmp(prebuf[j] + 2, "00", 2) == 0)
				{
					strcat(outbuf, chn[prebuf[j][0] - '0']);
					if (prebuf[j][0] != '0') strcat(outbuf, "Ǫ");
					strcat(outbuf, chn[prebuf[j][1] - '0']);
					strcat(outbuf, "��");
					strcat(outbuf, numb1[j]);
					continue;
				}
			}

			for (k=0; k<numlen; k++)
			{
				if (prebuf[j][k] == '0')
				{
					if (k == 0)
						strcat(outbuf, "��");
					else if ((prebuf[j][k-1] == '0') || (k == numlen-1))
						continue;
					else
						strcat(outbuf, "��");
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
 ** �� �� ����_extufProPTGETFUNDTRANCODE
 ** ��    �ܣ���û������룭���ڻ���Ӧ���Ľ�����ָ�ʽת������
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ISO(1)-�������� - 0200 �ɹ�Ӧ��9999������Ӧ�� 0800 ��Կ��������
 **            aParm[1]:ISO(3)-ԭʼ������
 **            aParm[2]:ISO(39)-��Ӧ��
 ** �� �� ֵ�� 0 �ɹ� aResult �����MAC
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
		swVdebug(0,"S9001:_extufProPTGETFUNDTRANCODE �����������[%d]���Ϸ�",iParmCount);
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
		/*Ӧ����Ӧ*/
		if (atoi(respcode)==0){
			/*�ɹ�����Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"0",1);
		}
		else {
			/*������Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"1",1);			
		}	
	}
	else if (atoi(msgtype)==9999){
		/*��ȫ����*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"9",1);		
	}	
	else {
		/*����Ӧ�룬����Կ����������*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"0",1);
	}	
	strcpy(aResult,aTrancode);

	swVdebug(3,"S8273: [��������] _extufProPTGETFUNDTRANCODE()������=0,���=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** �� �� ����_extufProPTGETLOANTRANCODE
 ** ��    �ܣ�����Ŵ������룭�����Ŵ�Ӧ���Ľ�����ָ�ʽת������
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ISO(1)-�������� - 0200 �ɹ�Ӧ��9999������Ӧ�� 0800 ��Կ��������
 **            aParm[1]:ISO(3)-ԭʼ������
 **            aParm[2]:ISO(39)-��Ӧ��
 ** �� �� ֵ�� 0 �ɹ� aResult �����MAC
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
		swVdebug(0,"S9001:_extufProPTGETLOANTRANCODE �����������[%d]���Ϸ�",iParmCount);
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
		/*Ӧ����Ӧ*/
		if (atoi(respcode)==0){
			/*�ɹ�����Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,4);
			memcpy(aTrancode+6,"0",1);
		}
		else {
			/*������Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,4);
			memcpy(aTrancode+6,"1",1);			
		}	
	}
	else if (atoi(msgtype)==9999){
		/*�Ŵ�ͨѶ����*/
		memcpy(aTrancode,msgtype,2);
		/*memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"9",1);	*/	
	}	
	else {
		/*����Ӧ�룬����Կ����������*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,4);
		memcpy(aTrancode+6,"0",1);
	}	
	strcpy(aResult,aTrancode);

	swVdebug(3,"S8273: [��������] _extufProPTGETLOANTRANCODE()������=0,���=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** �� �� ����_extufProPTGETREMITTRANCODE
 ** ��    �ܣ���û�ҽ����룭���ڻ��Ӧ���Ľ�����ָ�ʽת������
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ISO(1)-�������� - 0200 �ɹ�Ӧ��9999������Ӧ�� 0800 ��Կ��������
 **            aParm[1]:ISO(3)-ԭʼ������
 **            aParm[2]:ISO(39)-��Ӧ��
 ** �� �� ֵ�� 0 �ɹ� aResult �����MAC
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
		swVdebug(0,"S9001:_extufProPTGETREMITTRANCODE �����������[%d]���Ϸ�",iParmCount);
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
		/*��ҹ���֪ͨ�ཻ��*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		/*if(strcmp(msgtype,"0420") == 0)
			memcpy(aTrancode+8,"1",1);
		else
			memcpy(aTrancode+8,"0",1);*/
		/*Ӧ����Ӧ*/
		if (atoi(respcode)==0){
			/*�ɹ�����Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"0",1);
		}
		else {
			/*������Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"1",1);			
		}	
	}
	else if(strcmp(msgtype,"9999")==0){
		/*���ͨѶ���󼰾ܾ�����*/
		memcpy(aTrancode,msgtype,2);
		memcpy(aTrancode+2,procode,6);
		memcpy(aTrancode+8,"9",1);		
	}
	else{
		/*Ӧ����Ӧ*/
		if (atoi(respcode)==0){
			/*�ɹ�����Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"0",1);
		}
		else {
			/*������Ӧ*/
			memcpy(aTrancode,msgtype,2);
			memcpy(aTrancode+2,procode,6);
			memcpy(aTrancode+8,"1",1);			
		}	
	}
	strcpy(aResult,aTrancode);

	swVdebug(3,"S8273: [��������] _extufProPTGETREMITTRANCODE()������=0,���=[%s]",aResult);
	return(0);
}


/**************************************************************
 ** �� �� ����_extufProPTGETFORMATMULDATA
 ** ��    �ܣ���������ϸ����(����ʽ�����) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] Ҫ������ַ���buff
 **            aParm[1] һ��Ҫ���������
 **            aParm[2] һ��Ҫ������������� 
 **            aParm[3] ���п��(�����߷ָ������� ��3�� 12|20|30|)
 **            aParm[4] ���и�ʽת��(�����߷ָ������� ��3�� M:D12|C:FUND||)
 **            ÿ��ǰ������ĸ����ת������M: ������ת�� ���ת����ʽ
 **                                        C: �������ת�� ���ת���йؼ���
 **                                        A: �������ת���������������ݣ����ں�������һ���������� ����ؼ���
 **            aParm[5] �����Ƿ���Ҫ(���� ��3�У�ֻҪǰ���� 1|1|0|)
 ** �� �� ֵ�� 0 �ɹ� aResult �����Ķ����ϸ
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETFORMATMULDATA �������buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

	/*����ԭ�����г��ֵ� | �滻Ϊ 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [��������] _extufProPTGETFORMATMULDATA()������=0,���=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*�����Ƚṹ����*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����Ȳ���[%s]���Ϸ�",aColWidth);
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����в���[%d]<>[%s]�����߸���",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �������ݳ���[%d]<[%d]��Ҫ���ݳ���",iTotalLen,iNeedLen);
		return -3;
	}
	
	/*�����ʽ��*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����ʽ����[%s]���Ϸ�",aFormat);
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����в���[%d]<>[%s]��ʽ�������߸���[%d]",col,aFormat2,i);
		return -2;
	}	
	/*������Ҫ������*/
	memset(aNeed,0x00,sizeof(aNeed));
	strcpy(aNeed,aParm[5]);	
	
	spos=0;
	dpos=0;
	memset(aDealStr,0x00,sizeof(aDealStr));

	for (j=0;j<row;j++){
		for (i=0;i<col;i++){
			/*���ݸ�ʽ����һ������*/
			if (aNeed[i*2]=='1') {
				/*��Ҫ��������*/
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

	swVdebug(3,"S8273: [��������] _extufProPTGETFORMATMULDATA()������=0,���=[%s]",aResult);
	return(0);
}

/**************************************************************
 ** �� �� ����_extufProPTGETFORMATMULDATAEXT
 ** ��    �ܣ���������ϸ����(����ʽ�����,֧�ֱ䳤�ֶ�) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] Ҫ������ַ���buff
 **            aParm[1] һ��Ҫ���������
 **            aParm[2] һ��Ҫ������������� 
 **            aParm[3] ���п��(�����߷ָ������� ��3�� 12|20|30|)
 **            aParm[4] ���и�ʽת��(�����߷ָ������� ��3�� M:D12|C:FUND||)
 **            ÿ��ǰ������ĸ����ת������M: ������ת�� ���ת����ʽ
 **                                        C: �������ת�� ���ת���йؼ���
 **                                        A: �������ת���������������ݣ����ں�������һ���������� ����ؼ���
 **            aParm[5] �����Ƿ���Ҫ(���� ��3�У�ֻҪǰ���� 1|1|0|)
 ** �� �� ֵ�� 0 �ɹ� aResult �����Ķ����ϸ
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETFORMATMULDATA �������buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

	/*����ԭ�����г��ֵ� | �滻Ϊ 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [��������] _extufProPTGETFORMATMULDATA()������=0,���=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*�����Ƚṹ����*/
	/*�����Ƚṹ����*/
	memset((char *)&sColWidth,0x00,sizeof(sColWidth));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		printf("S9001:_extufProPTGETFORMATMULDATA �����Ȳ���[%s]���Ϸ�\n",aColWidth);
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
		printf("S9001:_extufProPTGETFORMATMULDATA �����в���[%d]<>[%s]��ʽ�������߸���[%d]\n",col,aColWidth2,i);
		return -2;
	}	
	
	/*�����ʽ��*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����ʽ����[%s]���Ϸ�",aFormat);
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����в���[%d]<>[%s]��ʽ�������߸���[%d]",col,aFormat2,i);
		return -2;
	}	
	/*������Ҫ������*/
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

	swVdebug(3,"S8273: [��������] _extufProPTGETFORMATMULDATA()������=0,���=[%s]",aResult);
	return(0);
}

/*2011-06-14�����г���ģʽ aColWidth ����ʼλ��iPos �Ӵ������ַ���aDealData��
ȡ��aOneData  ֧�ֱ䳤�ֶεĽ�ȡ 
2λ�䳤��󳤶�60 ����Ϊ 2:60
3λ�䳤��󳤶�125 ����Ϊ 3:125
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
 ** �� �� ����_ufUsrextfunPORTGET
 ** ��    �ܣ���չƽ̨��������
 ** ��    �ߣ�  
 ** �������ڣ�2008/04/13
 ** �޸����ڣ�
 ** ���ú�����          
 ** ȫ�ֱ�����                  
 ** �������壺����1:ǰ�ö˿�  ����2:�˿ڲ��
 ** �� �� ֵ�� 0        
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
		swVdebug(0,"swPortget()����,ȡ�˿�[%d]״̬����,������=%d",iQid,ilRc);
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
 ** �� �� ����_extufProPTSLEEP
 ** ��    �ܣ���ʱ 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ��ʱʱ��
 ** �� �� ֵ�� 0 �ɹ� aResult 
***************************************************************/
int _extufProPTSLEEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	
	strcpy(aResult,"\0");
	if (iParmCount!=1) {
		swVdebug(0,"S9001:_extufProPTSLEEP �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));
	strcpy(alTmp,aParm[0]);

	if (atoi(alTmp)>1000){
		swVdebug(0,"_extufProPTSLEEP ˯��ʱ��[%d]����",atoi(alTmp));
		return -1;
	}	
	sleep(atoi(alTmp));
	ilLen=strlen(alTmp);
	strcpy(aResult,alTmp);
	swVdebug(3,"S8273: [��������] _extufProPTSLEEP()������=0,���=[%s]len[%d]",aResult,ilLen);
	return(0);
}

/**************************************************************
 ** �� �� ��: _extufProPTMACRO
 ** ��    �ܣ���ȱʡֵ�ĺ��滻
 ** ��    �ߣ� 
 ** �������ڣ�2006/12/18
 ** �޸����ڣ�2001/12/18
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 
 ** �� �� ֵ�� 0
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
		swVdebug(0,"S9001:_extufProPTMACRO �����������[%d]���Ϸ�",iParmCount);
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
	
	/* Ѱ��ƥ������ */
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
	
	/* Ѱ��ƥ������ */
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
 ** �� �� ����_extufProPTSTRREPLACE
 ** ��    �ܣ��ַ����滻 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] �����Դ�ַ���
 **            aParm[1] ���滻������
 **            aParm[2] �滻�ɵ�����
 ** �� �� ֵ�� 0 �ɹ� aResult 
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
		swVdebug(0,"S9001:_extufProPTSTRREPLACE �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	

	memset(alTmp,0x00,sizeof(alTmp));
	strcpy(alTmp,aParm[0]);
	ilLen=strlen(alTmp);
	
	if ((ilSlen=strlen(aParm[1]))<=0){
		strcpy(aResult,alTmp);
		swVdebug(3,"S8273: [��������] _extufProPTSTRREPLACE()������=0,���=[%s]len[%d]",aResult,ilLen);		
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
	swVdebug(3,"S8273: [��������] _extufProPTSTRREPLACE()������=0,���=[%s]len[%d]",aResult,ilLen);
	return(0);
}


/*���ݴ���Ĺ��˴���������*/
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
 ** �� �� ����_extufProPTGETMATMULFILTRATE
 ** ��    �ܣ���������ϸ����(����ʽ�����) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] Ҫ������ַ���buff
 **            aParm[1] һ��Ҫ���������
 **            aParm[2] һ��Ҫ������������� 
 **            aParm[3] ���п��(�����߷ָ������� ��3�� 12|20|30|)
 **            aParm[4] ���и�ʽת��(�����߷ָ������� ��3�� M:D12|C:FUND||)
 **            ÿ��ǰ������ĸ����ת������M: ������ת�� ���ת����ʽ
 **                                        C: �������ת�� ���ת���йؼ���
 **                                        A: �������ת���������������ݣ����ں�������һ���������� ����ؼ���
 **            aParm[5] �����Ƿ���Ҫ(���� ��3�У�ֻҪǰ���� 1|1|0|)
 ** �� �� ֵ�� 0 �ɹ� aResult �����Ķ����ϸ
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
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETMATMULFILTRATE �������buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]fi[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5],aParm[6]);

	/*����ԭ�����г��ֵ� | �滻Ϊ 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [��������] _extufProPTGETMATMULFILTRATE()������=0,���=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*�����Ƚṹ����*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE �����Ȳ���[%s]���Ϸ�",aColWidth);
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
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE �����в���[%d]<>[%s]�����߸���",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE �������ݳ���[%d]<[%d]��Ҫ���ݳ���",iTotalLen,iNeedLen);
		return -3;
	}
	
	/*�����ʽ��*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE �����ʽ����[%s]���Ϸ�",aFormat);
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
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE �����в���[%d]<>[%s]��ʽ�������߸���[%d]",col,aFormat2,i);
		return -2;
	}	
	/*������Ҫ������*/
	memset(aNeed,0x00,sizeof(aNeed));
	strcpy(aNeed,aParm[5]);	
	
	
/*������˴�*/
	memset((char *)&sFiltrate,0x00,sizeof(sFiltrate));
	memset(aFiltrate,0x00,sizeof(aFiltrate));
	strcpy(aFiltrate,aParm[6]);
	strcpy(aFiltrate2,aParm[6]);	
	p=strtok(aFiltrate,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE ���������������[%s]���Ϸ�",aFiltrate);
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
		swVdebug(0,"S9001:_extufProPTGETMATMULFILTRATE �����в���[%d]<>[%s]���������������߸���[%d]",col,aFiltrate2,i);
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
			/*���ݸ�ʽ����һ������*/
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
					/*��Ҫ��������*/
	
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

	swVdebug(3,"S8273: [��������] _extufProPTGETMATMULFILTRATE()������=0,���=[%s]",aResult);
	return(0);
}



/**************************************************************
 ** �� �� ����_extufProPTGETUTIME
 ** ��    �ܣ��õ���ǰ���뼶ʱ��
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 
 ** �� �� ֵ�� 0 �ɹ� aResult ���ص����Ľ���
***************************************************************/
int _extufProPTGETUTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	struct timeval curtime;	
	gettimeofday(&curtime,NULL);	
	sprintf(aResult,"%d.%03d",curtime.tv_sec,curtime.tv_usec/1000);	
	return(0);
}
/*****************************************************************************/
/*****************���¶�������ϵͳʹ�õ�ƽ̨����************************/

#if 0
/**************************************************************
 ** �� �� ����_extufProPBCHGERRCODE
 ** ��    �ܣ�ת���ܺ���(ѹ��) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ϵͳ����
 **            aParm[1] �������
 ** �� �� ֵ�� 0 �ɹ� aResult ת�Ӻ��ǰ̨�������
***************************************************************/
int _extufProPBCHGERRCODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	short ilLen=0;
	int ilRc;
	char alTmp[iFLDVALUELEN];
	char syscode[4+1];
	char errcode[8+1];   /*��̨��Ӧ��*/
	char retcode[8+1];   /*�����ڲ���Ӧ��*/
	char errmsg[80+1];
	
	strcpy(aResult,"\0");
	if (iParmCount!=2) {
		swVdebug(0,"S9001:_extufProPBCHGERRCODE �����������[%d]���Ϸ�",iParmCount);
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
	swVdebug(3,"S8273: [��������] _extufProPBCHGERRCODE()������=0,���=[%s]len[%d]",aResult,ilLen);

	return(0);
}


#endif


/**************************************************************
 ** �� �� ����_extufProPBGETFOREIGNTRANCODE
 ** ��    �ܣ������ҽ����룭�������Ӧ���Ľ�����ָ�ʽת������
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ISO(1)-�������� - 0210 �ɹ�Ӧ��0200 ��������
 **                                       9999 ����Ӧ��(����ͷ��Ӧ���쳣)
 **            aParm[1]:ISO(4)-ԭʼ������
 **            aParm[2]:ISO(39)-��Ӧ��
 ** �� �� ֵ�� 0 �ɹ� aResult������Ľ�����
 **            �����������Ӧ�𣬽����벻��
 **            ����ͷ��Ӧ���쳣��������+��9��
 **            39������Ӧ���쳣��������+��1��
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
		swVdebug(0,"S9001:_extufProPBGETFOREIGNTRANCODE �����������[%d]���Ϸ�",iParmCount);
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
	/*Ӧ����Ӧ*/
	if ( ilMsgtype==210 ) 
	{
		if ( atoi(retcode)==0 )
		{
			/*�ɹ���Ӧ*/
			strcpy(alTrancode,trancode);
		}
		else
		{
			/*������Ӧ*/
			strcpy(alTrancode,trancode);
			strcat(alTrancode,"1");			
		}	
	}
	else if ( ilMsgtype==200 )
	{
		/*��������*/
		strcpy(alTrancode,trancode);
	}
	else if ( ilMsgtype==300 )
	{
		/*�ļ��·�֪ͨ������*/
		strcpy(alTrancode,trancode);
	}	
	else if ( ilMsgtype==9999 )
	{
		/*��ȫ���󡢸�ʽ����*/
		strcpy(alTrancode,trancode);
		strcat(alTrancode,"9");		
	}
	else
	{
		swVdebug(3,"S8488: _extufProPBGETFOREIGNTRANCODE msgtype[%s] error",msgtype);
	}
	strcpy(aResult,alTrancode);

	swVdebug(3,"S8273: [��������] _extufProPBGETFOREIGNTRANCODE()������=0,���=[%s]",aResult);
	return(0);
}

/*���ݴ���ĸ�ʽ����������*/
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
					swVdebug(0,"S9001:swAmount �������[%s] [%s]���Ϸ���δ��������",aData,alFormat);
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
			/*ʵ������һ�У�ͬʱ���治������*/
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
					swVdebug(0,"S9001:swAmount �������[%s] [%s]���Ϸ���δ��������",aData,alFormat);
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
 ** �� �� ����_extufProPTGETFMTMULDATASEP
 ** ��    �ܣ���������ϸ����(����ʽ����ģ��Զ���ָ���) 
 ** ��    �ߣ� 
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] Ҫ������ַ���buff
 **            aParm[1] һ��Ҫ���������
 **            aParm[2] һ��Ҫ������������� 
 **            aParm[3] ���п��(�����߷ָ������� ��3�� 12|20|30|)
 **            aParm[4] ���и�ʽת��(�����߷ָ������� ��3�� M:D12|C:FUND||)
 **            ÿ��ǰ������ĸ����ת������M: ������ת�� ���ת����ʽ
 **                                        C: �������ת�� ���ת���йؼ���
 **                                        A: �������ת���������������ݣ����ں�������һ���������� ����ؼ���
 **            aParm[5] �����Ƿ���Ҫ(���� ��3�У�ֻҪǰ���� 1|1|0|)
 **            aParm[6] �ָ����� (������Ĭ���ǿո����߷ָ�)
 ** �� �� ֵ�� 0 �ɹ� aResult �����Ķ����ϸ
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}
	memset(alTmp,0x00,sizeof(alTmp));	
	_ufTtoN(aParm[0],alTmp,&ilLen);
	
	swVdebug(2,"S9001:_extufProPTGETFORMATMULDATA �������buf[%s]col[%s]row[%s]w[%s]f[%s]n[%s]",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

	/*����ԭ�����г��ֵ� | �滻Ϊ 0x20 */
	ilLen = strlen(alTmp);
	for (i=0;i<ilLen;i++){
		if (alTmp[i]=='|')
			alTmp[i]=0x20;
	}
	col=atoi(aParm[1]);
	row=atoi(aParm[2]);
	if (row==0){
		strcpy(aResult,"\0");
		swVdebug(3,"S8273: [��������] _extufProPTGETFORMATMULDATA()������=0,���=[%s]",aResult);		
		return 0;
	}	
	iTotalLen=strlen(alTmp);
	/*�����Ƚṹ����*/
	memset(aColWidth,0x00,sizeof(aColWidth));
	memset(aColWidth2,0x00,sizeof(aColWidth2));
	strcpy(aColWidth,aParm[3]);
	strcpy(aColWidth2,aParm[3]);
	memset(aTmp,0x00,sizeof(aTmp));

	p=strtok(aColWidth,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����Ȳ���[%s]���Ϸ�",aColWidth);
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����в���[%d]<>[%s]�����߸���",col,aColWidth2);
		return -2;
	}
	if (iTotalLen<iNeedLen) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �������ݳ���[%d]<[%d]��Ҫ���ݳ���",iTotalLen,iNeedLen);
		return -3;
	}
	
	/*�����ʽ��*/
	memset((char *)&sFormat,0x00,sizeof(sFormat));
	memset(aFormat,0x00,sizeof(aFormat));
	strcpy(aFormat,aParm[4]);
	strcpy(aFormat2,aParm[4]);	
	p=strtok(aFormat,"|");
	if (p==NULL) {
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����ʽ����[%s]���Ϸ�",aFormat);
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
		swVdebug(0,"S9001:_extufProPTGETFORMATMULDATA �����в���[%d]<>[%s]��ʽ�������߸���[%d]",col,aFormat2,i);
		return -2;
	}	
	/*������Ҫ������*/
	memset(aNeed,0x00,sizeof(aNeed));
	strcpy(aNeed,aParm[5]);	
	/*�õ��ָ�����*/
	memset(aSep,0x00,sizeof(aSep));
	if (iParmCount==7){
		strcpy(aSep,aParm[6]);
	}
	else {
		/*�º�����������Ϊ�˼ӿո����ߣ�Ϊ�˸�ʽת���䶯��С���˴�Ĭ��ֵΪ" |"*/
		strcpy(aSep," |");
	}
	
	spos=0;
	dpos=0;
	memset(aDealStr,0x00,sizeof(aDealStr));

	for (j=0;j<row;j++){
		for (i=0;i<col;i++){
			/*���ݸ�ʽ����һ������*/
			if (aNeed[i*2]=='1') {
				/*��Ҫ��������*/
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

	swVdebug(3,"S8273: [��������] _extufProPTGETFORMATMULDATA()������=0,���=[%s]",aResult);
	return(0);
}









/*2010-6-27 add by pc Ϊ�������������µ�ƽ̨����*/

int swTrancodeIsFull(short q_target,char *aTrancode, int limitNum)
{
  short ilTotalcount;              /* ���� */
  short ilRc;       /* ������ */
  int ilCurrNum=0;
  int i;
  
  struct swt_tran_log pslSwt_tran_log[iMAXRECORD];
/*  
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"�����ڴ������!\n");
    return(FAIL);
  }
*/
  ilRc = swShmselect_swt_tran_log_all(pslSwt_tran_log,&ilTotalcount);
  if(ilRc == FAIL)
  {
    swVdebug(0,"[����/�����ڴ�] ִ��swShmselect_swt_tran_log_allʧ��");
    return(FAIL);
  }
  
  /*
���ڴ���	0
�������	1
����ʱ	2
���ڽ��г���	3
�������	4
����ʧ��	5
�����ȴ�	6
SAF����	7
  */
  swVdebug(3,"������ˮ����[%d] q_target[%d] aTrancode[%s] limitNum[%d]",ilTotalcount,q_target,aTrancode,limitNum);
  for(i=0;i<ilTotalcount;i++)
  {
/*
  	swVdebug(2,"q_target[%d] tc_tran_begin[%s] tran_status[%d]",pslSwt_tran_log[i].q_target,pslSwt_tran_log[i].tc_tran_begin,pslSwt_tran_log[i].tran_status);
*/
  	/*��ѵ���ڴ���Ľ��ף����Ŀ�������뽻�����봫����ͬ�����ڴ��������*/
  	if ((pslSwt_tran_log[i].q_target==q_target)&&\
  		 (strstr(aTrancode,pslSwt_tran_log[i].tc_tran_begin)!=NULL) &&\
  		 pslSwt_tran_log[i].tran_status==0)
  	{
  		ilCurrNum++;
  		if (ilCurrNum>limitNum) {
  			swVdebug(0,"���ײ�����������%d",limitNum);
  			return -1;
  		}
  	}
  }
  swVdebug(3,"��ǰ������[%d]",ilCurrNum);
  return 0; 
}



/**************************************************************
 ** �� �� ����_extufProPBTRANISFULL
 ** ��    �ܣ�ȡĳ����ָ�������Ƿ�ﵽ������
 ** ��    �ߣ�  
 ** �������ڣ�2009/09/17
 ** �޸����ڣ�
 ** ���ú�����          
 ** ȫ�ֱ�����                  
 ** �������壺����1:ָ����̨����  ����2:ָ�������� ����3:������
 ** �� �� ֵ�� 0        
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

/*2011/5/13 add by pc �����ص�ַ���а����������*/

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
	/*			fprintf(stderr,"���в��Ϸ�����1");*/
				return -1;			
			}
			else
					count=0;
		}		
		p++;
	}/*while*/
	fprintf(stderr,"count=%d\n",count);
	if ((count%2)==1){
	/*	fprintf(stderr,"���в��Ϸ�����");*/
		return -1;
	}
	else {
		/*	fprintf(stderr,"�޲��Ϸ�����");*/
			return 0;
		}

}

/**************************************************************
 ** �� �� ����_extufProPBGETVALIDCHN
 ** ��    �ܣ������Ϸ��������ַ���
 ** ��    �ߣ�  
 ** �������ڣ�2011/5/13
 ** �޸����ڣ�
 ** ���ú�����          
 ** ȫ�ֱ�����                  
 ** �������壺����1:�������ַ���
 ** �� �� ֵ�� 0 �ɹ� ʧ��
***************************************************************/
int _extufProPBGETVALIDCHN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{                               
	int     ilRc;
          	

	ilRc =  IsVaildChnStr( aParm[0] );
	if ( ilRc )
	{
		strcpy(aResult,"���в��Ϸ������ַ�");
	  return(0);
	}
	
	strcpy(aResult,aParm[0]);
	return(0);
}

/*2011/5/23 �����¹������͵Ŀ�ͨҵ���־��ʹ���;ɵ�ϵͳ����*/

/*
�µ������й������Ϳ�ͨ��־���绰���в���       �ɵ绰���й������Ϳ�ͨ��־
1	ת��	0��������,  1������                  ��1λ �绰ת��            
2	���	0��������,  1������                    ��2λ ���ҵ��            
3	֧��	0��������,  1������                    ��3λ ����ҵ��            
4	�ɷ�	0��������,  1������                    ��4λ �ɷѳ�ֵ            
5	Ͷ�����	0��������,  1������                ��5λ ���������          
6	���ҵ��	0��������,  1������                ��6λ ���ҵ��            
7	ָ��ת��	0��������,  1������                ��7λ ���ҵ��            
8	ָ�����	0��������,  1������                ��8λ ������־            
9	����	0��������,  1������                    ����Ԥ����                
10	����	0�������� 1������
*/

/**************************************************************
 ** �� �� ����_extufProPBGETOPENFLAG
 ** ��    �ܣ�����ͨ��־�ֶΣ�ʹ�����ǰ����
 ** ��    �ߣ�  
 ** �������ڣ�2011/5/23
 ** �޸����ڣ�
 ** ���ú�����          
 ** ȫ�ֱ�����                  
 ** �������壺 ����1:��ͨ���� ISO(20)
 ** �� �� ֵ�� 0 �ɹ� ʧ��
***************************************************************/
int _extufProPBGETOPENFLAG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{       
	                        
	swVdebug(3,"S8273: [��������] _extufProPBGETOPENFLAG() [%s][%s]",aParm[0]);	
  
  /*���۱�־*/
  aResult[0]=aParm[0][9];
  /*ת��*/
  aResult[1]=aParm[0][6];
	/*���*/
	aResult[2]=aParm[0][7];
	/*����*/
	aResult[3]=aParm[0][4];
	/*�ɷѳ�ֵ*/
	aResult[4]=aParm[0][3];
	/*���������*/
	aResult[5]=aParm[0][4];
	/*���ҵ��*/
	aResult[6]=aParm[0][4];
	/*���ҵ��*/
	aResult[7]=aParm[0][5];
	/*������־*/
	aResult[8]=aParm[0][8];
	/*���� �� �����ֶΣ�ֻ��ȡ 10*/
	aResult[9]=aParm[0][10];

	aResult[10]=aParm[0][10];
	aResult[11]=aParm[0][11];
	aResult[12]=aParm[0][12];
	aResult[13]=aParm[0][13];
	aResult[14]=aParm[0][14];
	aResult[15]=aParm[0][15];		
	aResult[16]=0;
	
	swVdebug(3,"S8273: [��������] _extufProPBGETOPENFLAG()������=0,���=[%s]",aResult);	
	
	return(0);
}


/**************************************************************
 ** �� �� ����_extufProPBTransferID
 ** ��    �ܣ���15λ���֤ תΪ18λ���֤ 
 ** ��    �ߣ�YZ 
 ** �������ڣ�2011/6/13
 ** �޸����ڣ�
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] 15λ���֤ 
 ** �� �� ֵ�� 0 �ɹ� aResult 18λ���֤ 
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
		swVdebug(0,"S9001:_extufProPBTransferIDNO �����������[%d]���Ϸ�",iParmCount);
		return -1;
	}

	memset(Input, 0x00, sizeof(Input));
	memset(Output, 0x00, sizeof(Output));
	
	strcpy(Input,aParm[0]);
  strncpy(Output, Input, 15);

  /* Output��7-15λ���κ���2λ */
  for(i=16; i>7; i--)
  {
		Output[i] = Output[i-2];
  }

  /* Output��7��8λ����'1'��'9' */
  Output[6] = '1';
  Output[7] = '9';

  /* ��ǰ17λλȨ�� */
  sum = 0;
  mod = 0;
  for(i=0; i<17; i++)
  {
		sum = sum + (Output[i]-48)*Wi[i];
  }

  /* У�����ַ�ֵ */
  mod = sum % 11;
  Output[17] = ai[mod];
  
	strcpy(aResult,Output);
	swVdebug(3,"S8273: [��������] _extufProPBTransferIDNO()������=0,���=[%s]",aResult);  
  return(0);
}

/**************************************************************
 ** �� �� ���� _extufProPBGET8583CHECK
 ** ��    �ܣ��õ�λԪ�Ƿ����
 ** ��    �ߣ�YZ
 ** �������ڣ�2011/6/20 15:06:07
 ** �޸����ڣ�
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0]  �μ�λԪ������ ��ʽΪ: 3|12|13|
 ** �� �� ֵ�� 0 �ɹ� 
               ��λԪ�� ʧ��
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
		swVdebug(0,"_extufProPBGET8583CHECK �������[%s]���Ϸ�",aIsoField);
		return -3;
	}
	sField[0]=atoi(p);
	/*ѭ��ɨ�贫���������֪Ҫ����λԪ�ţ�������λԪ���ݶ�Ӧλ��*/
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
	swVdebug(3,"��[%d]���������",ilTotalNum);

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
	swVdebug(3,"[��������] _extufProPBGET8583CHECK()������=0,���=[%s]len[%d]",aResult,iTotalLen);
	return(0);
}

/**************************************************************
 ** ������      �� _extufProSTRFILL
 ** ��  ��      �� ���ַ�������ַ���
 ** ��  ��      �� PC 
 ** ��������    �� 2011/6/24 15:58
 ** ����޸����ڣ� 2011/6/24 15:58
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : Դ���ݴ�
 **                aParm[1] : ������ַ���
 **                aParm[2] : ����󳤶�
 **                aParm[3] : ���Ҳ���־
 ** ����ֵ      �� ����Ĵ�
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
	
  swVdebug(3,"S8168: [��������] _extufProSTRFILL(%s,%s,%s,%s)",aParm[0],aParm[1],aParm[2],aParm[3]);
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
  swVdebug(3,"S8171: [��������] _extufProSTRFILL()������=0,���=%s",aResult);
  return(0);
}


/*****************************add by pc 2013/1/6 23:11 for GSMP ***********************/

/**************************************************************
 ** ������      �� _extufProSETINT
 ** ��  ��      �� �ַ���תΪ2��������ֵ
 ** ��  ��      �� PC 
 ** ��������    �� 2013/1/6 22:38
 ** ����޸����ڣ� 2013/1/6 22:38
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �ַ���
 **                aParm[1] : �����λ�� 1,2,4
 ** ����ֵ      �� ����Ĵ�
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
 ** ������      �� _extufProSETVARSTR
 ** ��  ��      �� �ַ���תΪ��4�ֽڳ���λ���ַ���
 ** ��  ��      �� PC 
 ** ��������    �� 2013/1/6 22:38
 ** ����޸����ڣ� 2013/1/6 22:38
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �ַ���
 ** ����ֵ      �� ����Ĵ�
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
 ** ������      �� _extufProMD5
 ** ��  ��      �� MD5�㷨
 ** ��  ��      �� PC
 ** ��������    �� 2013/1/6 22:38
 ** ����޸����ڣ� 2013/1/6 22:38
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �����õ��ַ���
 ** ��������    �� aParm[1] : �����õ��ַ����ĳ���
  ** ����ֵ      �� ����Ĵ�
***************************************************************/
int _extufProMD5(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
        char aTmp[iFLDVALUELEN];
        char alTmp[iFLDVALUELEN];
        int len,ilLen;
        
        char *s;
         
        swVdebug(3, "S0180: [��������] _extufProMD5(%s,%s)", aParm[0], aParm[1]);
        
        memset(alTmp,0x00,sizeof(alTmp));
        
        _ufTtoN(aParm[0], alTmp, &ilLen);

        len=atoi(aParm[1]);
        s=MD5(alTmp,len,NULL);

        memset(aTmp,0x00,sizeof(aTmp));
        memcpy(aTmp,s,16);

        _ufNtoT(aTmp, 16, aResult);
        swVdebug(3, "S0200: [��������] _extufProMD5()");

        return(0);
}

/**************************************************************
 ** ������      �� _extufProGETINT
 ** ��  ��      �� ȡ4�ֽ�����(����ת�����ֽ���)
 ** ��  ��      �� douya
 ** ��������    �� 2013/1/6 22:38
 ** ����޸����ڣ� 2013/1/6 22:38
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ȡ����ʼλ��
 ** ��������    �� aParm[1] : ��ȡ�ĳ���
  ** ����ֵ      �� ����Ĵ�
***************************************************************/
int _extufProGETINT(char aParm[][iFLDVALUELEN], short iParmCount, char *aResult)
{
        char alTmp[iFLDVALUELEN];

        unsigned int h32;
        unsigned int n32;
  int ilLen = 0;
  int ilOffset = 0;

   swVdebug(3, "S0180: [��������] _extufProGETINT(%s,%s)", aParm[0], aParm[1]);

   _ufTtoN(aParm[0], alTmp, &ilLen);
  ilOffset = atoi(alTmp) - 1;
  if (ilOffset < 0)
    ilOffset = 0;
        ilLen = 4;
        memcpy((char *)&n32,agMsgbody + ilOffset,ilLen);
        h32 = ntohl(n32);

        sprintf(aResult,"%d",h32);

  swVdebug(3, "S0200: [��������] _extufProGETINT()������=0,���=%s", aResult);
  return (0);

}


/**************************************************************
 ** �� �� ����_extufProMAC_APPLY
 ** ��    �ܣ�����ϵͳ����������Կ
 ** ��    �ߣ�    
 ** �������ڣ�2015/04/10
 ** �޸����ڣ�2015/04/10
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ϵͳ����
 **            aParm[1] macblock
 ** �� �� ֵ�� 0 �ɹ� aResult �����MAC
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
        swVdebug(0,"S9001:_extufProMAC_APPLY �����������[%d]���Ϸ�",iParmCount);
        return -1;
    }
    memset(syscode,0x00,sizeof(syscode));
    memset(macblock,0x00,sizeof(macblock));
    memset(mac,0x00,sizeof(mac));
    strcpy(syscode,aParm[0]);

    _ufTtoN(aParm[1],macblock,&ilStrlen);
    swVdebug(3,"S8273: [��������] _ufTtoN(macblock)[%s]",macblock);

    ilRc=HsmMAC_APPLY_New(syscode, macblock,strlen(macblock),mac);
    if (ilRc){ 
        swVdebug(0,"S9001:HsmMAC_APPLY error[%d]",ilRc);
        igFormatHsmFlag=-88;        
        return -2;
    }
    strcpy(aResult,mac);
    mac[16]=0;
    swVdebug(3,"S8273: [��������] _extufProMAC_APPLY()������=0,���=[%s]",aResult);
    return(0);
}



/**************************************************************
 ** �� �� ����_extufProMacByKey
 ** ��    �ܣ������ⲿ������Կ����MAC
 ** ��    �ߣ�
 ** �������ڣ�2001/11/28
 ** �޸����ڣ�2001/11/28
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ϵͳ����
 **                        aParm[1] ��Կ������
 **					aParm[2]  macbuf
 ** �� �� ֵ�� 0 �ɹ� aResult �����MAC
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
        swVdebug(0,"S9001:_extufProMacByKey �����������[%d]���Ϸ�",iParmCount);
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
    swVdebug(3,"S8273: [��������] _extufProMAC_APPLY()������=0,���=[%s]",aResult);
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
 ** �� �� ����_extufProUCS2STR
 ** ��    ��: ��ucs2���뱨��ת��ΪGBK ����
 ** ��    �ߣ�
 ** �������ڣ�2015/05/08
 ** �޸����ڣ�2015/05/08
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] UCS2��ʽ���ַ���
 ** �� �� ֵ�� 0 �ɹ� aResult ת����Ľ��
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

	 swVdebug(3,"S8273: [��������] _ufTtoN(altmp)[%s]",altmp);	


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
	//iconv��cd, &in, (size_t *)&inlen, &out,&outlen��
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

	swVdebug(3,"S8273: [��������] _extufProUCS2STR()������=0,���=[%s]",aResult);
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