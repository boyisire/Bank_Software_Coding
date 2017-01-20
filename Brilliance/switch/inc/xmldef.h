/********************************************************
XML��������� Version 1.0     cgx 2004-01-08
********************************************************/
#include <stdio.h>
#include <string.h>
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <stdarg.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include <fcntl.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <time.h>


/*���ڵ���Ŀ*/
#define MAXNODE 512
/*�ڵ����Ƴ���*/
#define NODENAME 128
/*�ڵ�ֵ����*/
#define NODEVALUE 2048
/*�������Ƴ���*/
#define PROPNAME 128
/*����ֵ����*/
#define PROPVALUE 128
/*�ڵ�������*/
#define XMLMAXLEVEL 10
/*·����󳤶�*/
#define XPATHLEN NODENAME * XMLMAXLEVEL + 10

/********************************************************
** XML�洢�ṹ����
********************************************************/
/*XML�ڵ�ṹ*/
struct _xmlnode
{
   struct _xmlnode *sParentNode;  /*���ڵ�*/
   struct _xmlnode *sBrotherPrev; /*��һ���ֵܽڵ�*/
   struct _xmlnode *sBrotherNext; /*��һ���ֵܽڵ�*/
   struct _xmlnode *sChildNode;   /*�ӽڵ�*/
   struct _xmlprop *sProp;        /*����*/
   short iPropCount;              /*���Ը���*/
   short iNodeType ;              /*�ڵ�����   ֦�ڵ㣬Ҷ���*/
   char aNodename[NODENAME+1];    /*�ڵ�����*/
   char aNodevalue[NODEVALUE+1];  /*�ڵ�ֵ*/
   short flag;                    /*��չ��ʶ*/
};
/*XML�ڵ����Խṹ*/
struct _xmlprop
{
   struct _xmlnode *sOwnerNode;   /*���������ڵ�*/
   struct _xmlprop *spPropnext;   /*��һ����*/
   char aPropname[PROPNAME+1];    /*��������*/
   char aPropvalue[PROPVALUE+1];  /*����ֵ*/
   short flag;                    /*��չ��ʶ*/
};
/*XMLͷ�ṹ*/
struct _xmlhead
{
   struct _xmlnode *shead;
   struct _xmlnode *sNode;
   struct _xmlnode *sNodeLevel[XMLMAXLEVEL];  /*�ּ��ڵ�ָ��*/
   short iNLevelCount[XMLMAXLEVEL];           /*�ּ��ڵ���Ŀ*/
   short iNodeLevel;              /*������*/
   short iNodeCount;              /*�ڵ���Ŀ*/
   short iPropCount;              /*������Ŀ*/
};

/*XML��������ṹ*/
struct _xmlhuntbuf
{
   char aXpath[XMLMAXLEVEL][NODENAME+1]; /*·��*/
   char aProp[2][PROPVALUE+1]; /*���Լ�ֵ*/
   int iLevel;                 /*�ڵ���� ��aXpath��һά���ֵ*/
   int iNum;                   /*�� iNum ��ͬ���ƽڵ�*/
   char cRoot;                 /*�Ƿ����·��*/
   struct _xmlnode *sNode;    /*��Ӧ·���Ľڵ�ָ��*/
};

typedef struct _xmlprop XMLPROP;
typedef struct _xmlnode XMLNODE;
typedef struct _xmlhuntbuf XMLHUNTBUF;
typedef struct _xmlhead XMLHEAD;


/****************************************************
** �ڵ�����
**
** ˵��: ����XMLʱ����ݽڵ�������XMLNODE��iNodeType
**       Ŀǰ��1.0�汾�У������ط���δʹ�ø�ֵ
****************************************************/
#define NOD_BRANCH 1   /*��ֵ���ӽڵ�*/
#define NOD_NORMAL 2   /*��ֵ���ӽڵ�*/
#define NOD_LEAF   3   /*��ֵ���ӽڵ�*/
#define NOD_HEAD  5    /*ͷ�ڵ�*/


/*����ֵ����*/
#define SUCCESS 0
#define FAIL    -1

/****************************************************
** ������붨��
** 
** ˵��: ���ڴ���ERR_CRITICAL�Ĵ���ֵ�������ش���
**       �����˳�����
**
** ��������ֵ��Ҫ���صĸ���ֵ�洢��igErrData��
** ���帽��ֵ��������Բ鿴 xmlDescription����
****************************************************/
#define ERR_INVALID_CHAR     1   /* �Ƿ��ַ� */
#define ERR_NOT_FOUND        2   /* δ�ҵ�ƥ�� */
#define ERR_ALREADY_INIT     3   /* �ظ���ʼ�� */
#define ERR_CRITICAL         10  /* ���ش�����һ�����ķֽ�ֵ */
#define ERR_INVALID_PARM     11  /* �����Ƿ� */
#define ERR_INVALID_RULE     12  /* ������� */
#define ERR_NODE_NOT_MATCH   13  /* ǰ��ڵ㲻ƥ�� */
#define ERR_NOT_ENOUGH_RES   14  /* ��Դ���� */
#define ERR_WRONG_PATH       15  /* ·������ */
#define ERR_NOT_PREPARE      16  /* δ��� */
#define ERR_DEPTH_TOO_BIG    17  /* �ڵ���ȳ������� ���Ϊ10 */
#define ERR_VALUE_TOO_LONG   18  /* ���ȳ������� */
#define ERR_STATE_NOTSET     19  /* ״̬δ���� */
#define ERR_STATE_NOTFREE    20  /* ��һ�δ��/���������δ���� */
#define ERR_INVALID_NOD_TYPE 21  /* Ҷ�ڵ㲻�ܲ���ڵ� */
#define ERR_NODE_EXIST       22  /* �ڵ��Ѿ����� */
#define ERR_PROP_EXIST       23  /* �����Ѿ����� */
#define ERR_OPEN_FILE        24  /* ���ļ�ʧ�� */
#define ERR_LAST             25

/*��������*/
#define ERR_LEN 100
const char agDescription[][ERR_LEN+1] =
   { "",
     "�Ƿ��ַ�",
     "δ�ҵ�ƥ��",
     "�ظ���ʼ��",
     "","","","","","",
     "δ֪���ش���",  /* 10 */
     "�����Ƿ�",
     "�������",
     "ǰ��ڵ㲻ƥ��",
     "��Դ����",
     "·������",
     "δִ��Ԥ���",
     "�ڵ���ȳ�������,���Ϊ10 ",
     "��ǰ���ݳ��ȳ�������",
     "״̬δ����",
     "��һ�δ��/���������δ����",
     "Ҷ�ڵ㲻�ܲ���ڵ�",
     "�ڵ��Ѿ�����",
     "�����Ѿ�����",
     "���ļ�ʧ��",""
   };


/*************************************************
 ת���ַ�����
*************************************************/
/* &amp; &  &lt; <  &gt; >  &quot;\"  &apos;\' */
/*ת���ַ���ѯ��*/
char agExtraCh[][10]={"&amp;","&lt;","&gt;","&quot;","&apos;","[END]"};
char cgExtraCh[]={'&','<','>','\"','\'',0x00};



/************************************************************
**
**
**XML�﷨�������
**
**
************************************************************/

/******************************************************
** ��������
**
**    $ ��ʾƥ����ĸ�����֣��»��߼���
**    @ ��ʾ�س���ո���������
**    ! ��ʾ�������ַ�����<> " '
******************************************************/
#define EXPLEN 100
char agRuleVar[][EXPLEN+1]={
   "#V_NN","$",
   "#V_NV","!",
   "#V_PN","$",
   "#V_PV","!",
   "#V_VS","\"",
   "#V_SP"," ",
   "#V_CR",{'\r',0x00},
   "#V_LF",{'\n',0x00},
   "#V_CRLF",{'\r','\n',0x00},
   "#V_CL","@",
   "#V_LT","<",
   "#V_GT",">",
   "#V_NEB","</",
   "#V_NEF","/>",
   "#V_QN","?",
   "#V_KEY","xml",
   "#V_EQ","=",
   "[END]"
   };
/*
 ������Ҫ����NODE�ڵ�ı����У�V_NN V_KEY
 ��Ҫ�������Խڵ�ı�����:      V_PN
*/


/**************************************************
 ����������

 ��һλ�洢�������������һλ�洢agRuleVar�ַ�������
**************************************************/
#define V_LEN 50
int igVarpos[V_LEN+1];


/*********************************************************
** ���ʽ����
**
** һ���ӱ��ʽָ���ڶ���ĺ꣬���������ɵı��ʽΪ���ϱ��ʽ
** 
*********************************************************/
char agRuleExp[][EXPLEN+1]={
   "#E_PP", "&V_SP&V_SP*&V_PN&V_SP*&V_EQ&V_SP*&V_VS&V_PV&V_VS",
   "#E_HD", "&V_LT&V_QN&V_KEY&E_PP*&V_QN&V_GT&V_CL*",
   "#E_ND",
         "&V_LT&V_NN&E_PP*&V_GT&V_CL*&E_ND&E_ND*&V_NEB&V_NN&V_GT&V_CL*",
	 "&V_LT&V_NN&E_PP*&V_GT&V_NV*&V_NEB&V_NN&V_GT&V_CL*",
	 "&V_LT&V_NN&E_PP*&V_NEF&V_CL*",
   "#E_MN",
	 "&V_CL*&E_HD&E_ND&E_ND*",
   "[END]"
};

/*********************************************************
** ���ʽ������
**
** ��һλ�洢���ʽ���������һλ�洢agRuleExp�ַ�������
*********************************************************/
#define E_LEN V_LEN
int igExppos[E_LEN+1];

