/* 提供调用过程：
1	void ASCII2EBCDIC(unsigned char* c)
	说明：把一个字符转为EBCDIC码,控制码及不可见码都转为空格
        参数：
	   unsigned char* c
		进入程序时指向需转换字符,出程序后已转换成EBCDIC码.
	范例：
		char c1;
		c1='1';			Now c1 equal to 0x31, '1' in ASCII
		ASCII2EBCDIC(&c1);
		...                     Now c1 equal to 0xF1, '1' in EBCDIC

2	StringPCToHost(char* szHZ,int nLength)
	说明：
		把传进来的PC编码汉字、英文字符、数字转换成IBM主机编码汉字
		（CODE PAGE 1388），字符和数字。返回指定长度的字符串。

	参数：
	  szHZ:	
		进入程序时指向需转换字符串，返回后里面是转换后，
		加好引入、引出码后的主机编码字符串。
	  nLength：
		转换后的字符bytes数，本函数传出串为此长度，原串超长则截断，
		不足填空格。
	范例："abCD32G" -> X(15)
		StringPCToHost("abCD32G",15);
	      "复兴路19号4楼5门701" -> X(40)
		StringPCToHost("复兴路19号4楼5门701",40);

3	unsigned int PCString2HostNumber(unsigned char* szPCString,enum HostNumberTypeTag HostNumberType)
	说明：	把字符串型数字转为主机COBOL格式的数值数据。编辑格式要预先转好，
		比如"95.2"要转为S9(3)V9(2),要先转成"09520"在传给本函数。
	返回值：szPCString中返回的主机COBOL格式数据字节长,从1开始。
		（因可能有0x00，所以不能以'\0'为数据结尾）出错返回0
	输入值：
	char* szPCString：
		进入时存放要转换的字符串，'\0'结尾。返回后指向转换后的数据。
	enum HostNumberTypeTag HostNumberType:
		输入：COBOL格式数值的类型。是enum HostNumberTypeTag之一。
	范例：2345.5 -> S9(13)V9(2) COMP-3
		nHostVariableBytes=PCString2HostNumber("000000000234550",SIGNED_COMP_3);
	      2345.5 -> S9(13)V9(2) 
		nHostVariableBytes=PCString2HostNumber("000000000234550",UNSIGNED_DISPLAY);
	      123 -> S9(8)
		nHostVariableBytes=PCString2HostNumber("00000123",SIGNED_DISPLAY);
	      123 -> S9(4) COMP
		nHostVariableBytes=PCString2HostNumber("0123",SIGNED_COMP);
*/

#define TRUE 1
#define FALSE 0
#define UCHAR unsigned char
enum HostNumberTypeTag{
	SIGNED_DISPLAY,		/* S9 之类		*/
	SIGNED_COMP_3,		/* S9 COMP-3 之类	*/
	UNSIGNED_DISPLAY,	/* 9 之类		*/
	UNSIGNED_COMP_3,	/* 9 COMP-3 之类	*/
	SIGNED_COMP		/* S9 COMP之类		*/
};

int StringPCToHost(char* szHZ,int nLength);
short ChineseInbound(unsigned char *pointer,short length);
short PCString2HostNumber(unsigned char* szPCString,enum HostNumberTypeTag HostNumberType);
int EtoA( char *aStr,char *aBuf,char *aType,short iLen1,short iLen2);

