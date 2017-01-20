/* �ṩ���ù��̣�
1	void ASCII2EBCDIC(unsigned char* c)
	˵������һ���ַ�תΪEBCDIC��,�����뼰���ɼ��붼תΪ�ո�
        ������
	   unsigned char* c
		�������ʱָ����ת���ַ�,���������ת����EBCDIC��.
	������
		char c1;
		c1='1';			Now c1 equal to 0x31, '1' in ASCII
		ASCII2EBCDIC(&c1);
		...                     Now c1 equal to 0xF1, '1' in EBCDIC

2	StringPCToHost(char* szHZ,int nLength)
	˵����
		�Ѵ�������PC���뺺�֡�Ӣ���ַ�������ת����IBM�������뺺��
		��CODE PAGE 1388�����ַ������֡�����ָ�����ȵ��ַ�����

	������
	  szHZ:	
		�������ʱָ����ת���ַ��������غ�������ת����
		�Ӻ����롢�����������������ַ�����
	  nLength��
		ת������ַ�bytes����������������Ϊ�˳��ȣ�ԭ��������ضϣ�
		������ո�
	������"abCD32G" -> X(15)
		StringPCToHost("abCD32G",15);
	      "����·19��4¥5��701" -> X(40)
		StringPCToHost("����·19��4¥5��701",40);

3	unsigned int PCString2HostNumber(unsigned char* szPCString,enum HostNumberTypeTag HostNumberType)
	˵����	���ַ���������תΪ����COBOL��ʽ����ֵ���ݡ��༭��ʽҪԤ��ת�ã�
		����"95.2"ҪתΪS9(3)V9(2),Ҫ��ת��"09520"�ڴ�����������
	����ֵ��szPCString�з��ص�����COBOL��ʽ�����ֽڳ�,��1��ʼ��
		���������0x00�����Բ�����'\0'Ϊ���ݽ�β��������0
	����ֵ��
	char* szPCString��
		����ʱ���Ҫת�����ַ�����'\0'��β�����غ�ָ��ת��������ݡ�
	enum HostNumberTypeTag HostNumberType:
		���룺COBOL��ʽ��ֵ�����͡���enum HostNumberTypeTag֮һ��
	������2345.5 -> S9(13)V9(2) COMP-3
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
	SIGNED_DISPLAY,		/* S9 ֮��		*/
	SIGNED_COMP_3,		/* S9 COMP-3 ֮��	*/
	UNSIGNED_DISPLAY,	/* 9 ֮��		*/
	UNSIGNED_COMP_3,	/* 9 COMP-3 ֮��	*/
	SIGNED_COMP		/* S9 COMP֮��		*/
};

int StringPCToHost(char* szHZ,int nLength);
short ChineseInbound(unsigned char *pointer,short length);
short PCString2HostNumber(unsigned char* szPCString,enum HostNumberTypeTag HostNumberType);
int EtoA( char *aStr,char *aBuf,char *aType,short iLen1,short iLen2);

