/********************************************************
XML解析器设计 Version 1.0     cgx 2004-01-08
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


/*最大节点数目*/
#define MAXNODE 512
/*节点名称长度*/
#define NODENAME 128
/*节点值长度*/
#define NODEVALUE 2048
/*属性名称长度*/
#define PROPNAME 128
/*属性值长度*/
#define PROPVALUE 128
/*节点最大深度*/
#define XMLMAXLEVEL 10
/*路径最大长度*/
#define XPATHLEN NODENAME * XMLMAXLEVEL + 10

/********************************************************
** XML存储结构定义
********************************************************/
/*XML节点结构*/
struct _xmlnode
{
   struct _xmlnode *sParentNode;  /*父节点*/
   struct _xmlnode *sBrotherPrev; /*上一个兄弟节点*/
   struct _xmlnode *sBrotherNext; /*下一个兄弟节点*/
   struct _xmlnode *sChildNode;   /*子节点*/
   struct _xmlprop *sProp;        /*属性*/
   short iPropCount;              /*属性个数*/
   short iNodeType ;              /*节点类型   枝节点，叶结点*/
   char aNodename[NODENAME+1];    /*节点名称*/
   char aNodevalue[NODEVALUE+1];  /*节点值*/
   short flag;                    /*扩展标识*/
};
/*XML节点属性结构*/
struct _xmlprop
{
   struct _xmlnode *sOwnerNode;   /*属性所属节点*/
   struct _xmlprop *spPropnext;   /*下一属性*/
   char aPropname[PROPNAME+1];    /*属性名称*/
   char aPropvalue[PROPVALUE+1];  /*属性值*/
   short flag;                    /*扩展标识*/
};
/*XML头结构*/
struct _xmlhead
{
   struct _xmlnode *shead;
   struct _xmlnode *sNode;
   struct _xmlnode *sNodeLevel[XMLMAXLEVEL];  /*分级节点指针*/
   short iNLevelCount[XMLMAXLEVEL];           /*分级节点数目*/
   short iNodeLevel;              /*结点深度*/
   short iNodeCount;              /*节点数目*/
   short iPropCount;              /*属性数目*/
};

/*XML搜索缓冲结构*/
struct _xmlhuntbuf
{
   char aXpath[XMLMAXLEVEL][NODENAME+1]; /*路径*/
   char aProp[2][PROPVALUE+1]; /*属性及值*/
   int iLevel;                 /*节点深度 即aXpath第一维最大值*/
   int iNum;                   /*第 iNum 个同名称节点*/
   char cRoot;                 /*是否绝对路径*/
   struct _xmlnode *sNode;    /*对应路径的节点指针*/
};

typedef struct _xmlprop XMLPROP;
typedef struct _xmlnode XMLNODE;
typedef struct _xmlhuntbuf XMLHUNTBUF;
typedef struct _xmlhead XMLHEAD;


/****************************************************
** 节点类型
**
** 说明: 解析XML时会根据节点类型置XMLNODE的iNodeType
**       目前的1.0版本中，其它地方并未使用该值
****************************************************/
#define NOD_BRANCH 1   /*无值有子节点*/
#define NOD_NORMAL 2   /*有值无子节点*/
#define NOD_LEAF   3   /*无值无子节点*/
#define NOD_HEAD  5    /*头节点*/


/*返回值定义*/
#define SUCCESS 0
#define FAIL    -1

/****************************************************
** 错误代码定义
** 
** 说明: 对于大于ERR_CRITICAL的错误值属于严重错误
**       必须退出解析
**
** 函数返回值需要返回的附加值存储在igErrData中
** 具体附加值的意义可以查看 xmlDescription函数
****************************************************/
#define ERR_INVALID_CHAR     1   /* 非法字符 */
#define ERR_NOT_FOUND        2   /* 未找到匹配 */
#define ERR_ALREADY_INIT     3   /* 重复初始化 */
#define ERR_CRITICAL         10  /* 严重错误与一般错误的分界值 */
#define ERR_INVALID_PARM     11  /* 参数非法 */
#define ERR_INVALID_RULE     12  /* 规则错误 */
#define ERR_NODE_NOT_MATCH   13  /* 前后节点不匹配 */
#define ERR_NOT_ENOUGH_RES   14  /* 资源不足 */
#define ERR_WRONG_PATH       15  /* 路径错误 */
#define ERR_NOT_PREPARE      16  /* 未解包 */
#define ERR_DEPTH_TOO_BIG    17  /* 节点深度超出限制 最大为10 */
#define ERR_VALUE_TOO_LONG   18  /* 长度超出限制 */
#define ERR_STATE_NOTSET     19  /* 状态未设置 */
#define ERR_STATE_NOTFREE    20  /* 上一次打包/解包操作尚未结束 */
#define ERR_INVALID_NOD_TYPE 21  /* 叶节点不能插入节点 */
#define ERR_NODE_EXIST       22  /* 节点已经存在 */
#define ERR_PROP_EXIST       23  /* 属性已经存在 */
#define ERR_OPEN_FILE        24  /* 打开文件失败 */
#define ERR_LAST             25

/*错误描述*/
#define ERR_LEN 100
const char agDescription[][ERR_LEN+1] =
   { "",
     "非法字符",
     "未找到匹配",
     "重复初始化",
     "","","","","","",
     "未知严重错误",  /* 10 */
     "参数非法",
     "规则错误",
     "前后节点不匹配",
     "资源不足",
     "路径错误",
     "未执行预解包",
     "节点深度超出限制,最大为10 ",
     "当前数据长度超出限制",
     "状态未设置",
     "上一次打包/解包操作尚未结束",
     "叶节点不能插入节点",
     "节点已经存在",
     "属性已经存在",
     "打开文件失败",""
   };


/*************************************************
 转义字符对照
*************************************************/
/* &amp; &  &lt; <  &gt; >  &quot;\"  &apos;\' */
/*转义字符查询表*/
char agExtraCh[][10]={"&amp;","&lt;","&gt;","&quot;","&apos;","[END]"};
char cgExtraCh[]={'&','<','>','\"','\'',0x00};



/************************************************************
**
**
**XML语法分析设计
**
**
************************************************************/

/******************************************************
** 变量定义
**
**    $ 表示匹配字母，数字，下划线集合
**    @ 表示回车与空格的任意组合
**    ! 表示非特殊字符集，<> " '
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
 以上需要分配NODE节点的变量有：V_NN V_KEY
 需要分配属性节点的变量有:      V_PN
*/


/**************************************************
 变量索引表

 第一位存储变量个数，最后一位存储agRuleVar字符串个数
**************************************************/
#define V_LEN 50
int igVarpos[V_LEN+1];


/*********************************************************
** 表达式定义
**
** 一个子表达式指存在定义的宏，而多个宏组成的表达式为复合表达式
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
** 表达式索引表
**
** 第一位存储表达式个数，最后一位存储agRuleExp字符串个数
*********************************************************/
#define E_LEN V_LEN
int igExppos[E_LEN+1];

