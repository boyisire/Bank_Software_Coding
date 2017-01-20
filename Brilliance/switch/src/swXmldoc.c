#include "xmldef.h"

#define _XML_DEBUG____

XMLHEAD sgXmlhead;           /*XML头结构*/
XMLHUNTBUF sgHuntBuf;        /*搜索缓冲*/
/****************************************
**  资源定义
****************************************/
XMLNODE _sgNode[MAXNODE];    /*节点资源*/
char _cgNode[MAXNODE];
XMLPROP _sgProp[MAXNODE];    /*属性资源*/
char _cgProp[MAXNODE];

char cgunpackInit=0x00;      /*解包初始化标志*/
char cgpackInit=0x00;        /*打包初始化标志*/

char cgInitRule=0x00;        /*表达式初始化标志*/
int igErrData=0;             /*附加错误数据*/
int igLevel=0;               /*深度记录, 解包时使用*/
int igDepth=0;               /*深度记录，打包时使用*/
int igDumpFile=0;            /*导出到文件标志*/

char *agMsgbuf=NULL;
int igMsgbuflen=0;

/*******************************************************
  函数声明 
  以"xml"开头的函数提供给外部调用
  以"_xm"开头的函数只供内部调用
*******************************************************/
int xmlunpackInit(char *aMsgbody,int iMsgbodylen);
int xmlpackInit(char *aVersion,char *aEncoding);
int xmlAddNode(char *aXpath,char *aNode,char *aValue);
int xmlSetNode(char *aXpath,char *aValue);
int xmlAddProp(char *aXpath,char *aProp,char *aValue);
int xmlSetProp(char *aXpath,char *aProp,char *aValue);
int xmlDump(char *aMsgbuf,int *iMsgbuflen);
int xmlDumpFile(char *aFileName);
char *xmlDescription(int i);
int xmlGet(char *aPath,char *aResult,char **pointer);
int xmlGetValue(char *aPath,char *aResult);/* add by nh 20040419 */
void xmlAtoC(char *aBuf);
void xmlCtoA(char *aBuf);
void xmlFree();

int _xmVarRule(char *aVar,char *aRule);
int _xmspechk(char cTmp);
int _xmchrchk(char cTmp);
int _xmkeychk(char cTmp);
void _xmInitRuleTable();
int _xmExpCheck(XMLNODE *sParent,char *aExp,int *iOffset);
int _xmTrim(char *aBuf);
int _xmParseXpath(char *aPath);
XMLNODE *_xmAllocNode();
void _xmFreeProp(XMLPROP *sProp);
void _xmFreeNode(XMLNODE *sNode);
XMLPROP *_xmAllocProp();
int _xmExpress(char *aSrc,char *aBuf);
int _xmVarCheck(char *aVar,int *iOffset);
int _xmExpRule(char *aExp,char aRule[][EXPLEN+1],int *iRuleCount);
int _xmAppendPropValue(XMLPROP *sProp,char *aValue);
XMLPROP *_xmAppendProp(XMLNODE *sOwner,char *aName);
int _xmAppendNodeValue(XMLNODE *sNode,char *aValue);
XMLNODE *_xmAppendNode(XMLNODE *sParent,char *aName,int iType);
int _xmStackCheck(char *aMacro,char *aValue);
XMLNODE *_xmAppendHead(char *aName);
int _xmDumpNode(XMLNODE *sNode,int *iOffset);

int xmlGetNode(char *aXpath,char *aResult,char **pointer);
int xmlGetProp(char *aXpath,char *aName,char *aResult);
int _swParsexml(char *aXpath,char *aPath,short *iNum);

/**************************************************************
 ** 函数名      :   xmlDescription()
 ** 功  能      :   取错误描述
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :   输入：i -- 错误代码
 ** 返回值      :   (无)
***************************************************************/
char *xmlDescription(int i)
{
   char alBuf[ERR_LEN+20+1];
   char alTmp[20];
   int ilLen;

   memset(alBuf,0x00,sizeof(alBuf));
   memset(alTmp,0x00,sizeof(alTmp));
   if(i < 1 || i > ERR_LAST) i = ERR_CRITICAL;
   strcpy(alBuf,agDescription[i]);
   if(i == ERR_INVALID_CHAR)
   {
      if(igErrData >=0 && igErrData < igMsgbuflen)
      {
         ilLen = igMsgbuflen - igErrData;
         if(ilLen > 8) ilLen = 8;
         memcpy(alTmp,agMsgbuf+igErrData,ilLen);
         alTmp[ilLen] = 0x00;
         strcat(alBuf,"[");
         strcat(alBuf,alTmp);
         strcat(alBuf,"]");
      }
      return((char *)alBuf);
   }

   if(i == ERR_NODE_NOT_MATCH)
   {
      if(igErrData >=0 && igErrData <= MAXNODE)
      {
         strcat(alBuf,"[");
         strcat(alBuf,_sgNode[igErrData].aNodename);
         strcat(alBuf,"]");
      }
      return((char *)alBuf);
   }

   return((char *)alBuf);
}

/**************************************************************
 ** 函数名      :   _xmTrim()
 ** 功  能      :   删除字符串两边的 \r\t\n 以及空格
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :   输入：aBuf  -- 需要处理的字符串
 ** 返回值      :   (无)
***************************************************************/
int _xmTrim(char *s)
{
  short	i, l, r, len;
  char *tmp;
  
  tmp = s;
  for(len=0; tmp[len]; len++);
  if (len==0) return SUCCESS;
  for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
  if(l==len)
  {
    s[0]='\0';
    return SUCCESS;
  }
  for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
  for(i=l; i<=r; i++) tmp[i-l]=tmp[i];
  tmp[r-l+1]='\0';
  return SUCCESS;
}
/**************************************************************
 ** 函数名      :   xmlAtoC()
 ** 功  能      :   字符串反转义
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :   cgExtraCh agExtraCh
 ** 参数含义    :   输入：aBuf  -- 需要处理的字符串
 ** 返回值      :   (无)
***************************************************************/
void xmlAtoC(char *aBuf)
{
   char *alTmp;
   int i=0;
   int ilLen,ilTmpLen;
   ilLen = strlen(aBuf);
   if(ilLen <= 0) return;

   while(strcmp(agExtraCh[i],"[END]"))
   {
      alTmp = strstr(aBuf,agExtraCh[i]);
      while(NULL != alTmp)
      {
         *alTmp = cgExtraCh[i];
         ilTmpLen = ilLen - (alTmp-aBuf+strlen(agExtraCh[i]));
         memcpy(alTmp+1,alTmp+strlen(agExtraCh[i]),ilTmpLen+1);
         ilLen = strlen(aBuf);
         alTmp = strstr(aBuf,agExtraCh[i]);
      }
      i += 1;
   }
   return;
}

/**************************************************************
 ** 函数名      :   xmlCtoA()
 ** 功  能      :   字符串转义
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :   cgExtraCh agExtraCh
 ** 参数含义    :   输入：aBuf  -- 需要处理的字符串
 ** 返回值      :   (无)
***************************************************************/
void xmlCtoA_old(char *aBuf)
{

   char *alTmp;
   char alBuf[NODEVALUE+1];
   int i=0,ilLen;
   ilLen = strlen(aBuf);
   if(ilLen <= 0) return;  	  	   
   while(cgExtraCh[i] != 0x00)
   {  	  	
       alTmp = strchr(aBuf,cgExtraCh[i]);            
       while(NULL != alTmp)
       {
          *alTmp = 0x00;
          alTmp += 1;                 
          strcpy(alBuf,aBuf);        
          strcat(alBuf,agExtraCh[i]);                 
          strcat(alBuf,alTmp);                    
          strcpy(aBuf,alBuf);                   
          alTmp = strchr(aBuf,cgExtraCh[i]);      
       }
       i += 1;
   }     
   return;
}
void xmlCtoA(char *aBuf)
{

   char *alTmp;
   char *pos;
   char alBuf[NODEVALUE+1];
   int i=0,ilLen;
   ilLen = strlen(aBuf);
   if(ilLen <= 0) return;  	  	   
   while(cgExtraCh[i] != 0x00)
   {  	  	
       alTmp = strchr(aBuf,cgExtraCh[i]); 
       pos=alTmp;
       while(NULL != alTmp)
       {
          *alTmp = 0x00;
          alTmp += 1;                 
          strcpy(alBuf,aBuf);        
          strcat(alBuf,agExtraCh[i]);                 
          strcat(alBuf,alTmp);                    
          strcpy(aBuf,alBuf);   
          
       pos+=strlen(agExtraCh[i]);
                          
          alTmp = strchr(pos,cgExtraCh[i]);  
          
       pos=alTmp;    
       }
       i += 1;
   }     
   return;
}

/**************************************************************
 ** 函数名      :   _xmParseXpath()
 ** 功  能      :   解析路径到sgHuntBuf中
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmchrchk()  _xmkeychk() 
 ** 全局变量    :   sgHuntBuf
 ** 参数含义    :   输入：aPath -- 路径串
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
 ** 说        明:   aPath规则 路径[第n个同名节点] /root/text[2]
 **                 或        路径[@属性名称=属性值] /root/text[@abc=123]
                        --最多一个属性
 **                  路径允许为相对路径,深度只能为次
 **                 例如：上一次为"/root/text",第二次为"/root/value"
 **                 可以表达为"value"
***************************************************************/
int _xmParseXpath(char *aPath)
{
   char alBuf[10];
   char *clTmp;
   XMLNODE *slNode;
   int ilLen=0,n;
   int ilNum;

   /* 标志状态*/
   char clPath=0x00,clProp=0x00;
   char clPname=0x00,clPvalue=0x00,clNum=0x00;

   if(NULL == aPath) return(FAIL);
   memset(alBuf,0x00,sizeof(alBuf));
   slNode = sgHuntBuf.sNode;
   memset(&sgHuntBuf,0x00,sizeof(XMLHUNTBUF));
   sgHuntBuf.sNode = slNode;

   for(n=0;n<strlen(aPath);n++)
   {
      clTmp = aPath + n;
      /*取路径*/
      if(clPath)
      {
         if(*clTmp == '/')
         {
            if(ilLen <= 0)
               return(FAIL);
            else
            {
               sgHuntBuf.iLevel += 1;
               ilLen = 0;
               continue;
            }
         }
         if(*clTmp == '[')
         {
            if(ilLen <= 0)
               return(FAIL);
            else
            {
               sgHuntBuf.aXpath[sgHuntBuf.iLevel][ilLen] = 0x00;
               clPath = 0x00;
               clProp = 0x01;
               ilLen = 0;
               continue;
            }
         }
         if(_xmchrchk(*clTmp))
            return(FAIL);
         sgHuntBuf.aXpath[sgHuntBuf.iLevel][ilLen++] = *clTmp;
         continue;
      } /* end if clPath */
      /*取属性*/
      if(clProp)
      {
         if(clNum)
         {
            if(*clTmp == ']')
            {
               if(ilLen <= 0)
                  return(FAIL);
               else
               {
                  sgHuntBuf.aProp[0][ilLen] = 0x00;
                  ilNum = atoi(alBuf);
                  if(ilNum <= 0 || ilNum > 9) return(FAIL);
                  sgHuntBuf.iNum = ilNum;
                  break;
               }
            }
            if(*clTmp >= '0' && *clTmp <= '9')
               alBuf[ilLen++] = *clTmp;
            else
               return(FAIL);
            continue;
         }
         /*取属性名称*/
         if(clPname)
         {
            if(*clTmp == '=')
            {
               if(ilLen <= 0)
                  return(FAIL);
               else
               {
                  clPname = 0x00;
                  clPvalue = 0x01;
                  ilLen = 0;
                  continue;
               }
            }
            if(*clTmp == ']')
            {
               if(ilLen <=0) 
                  return(FAIL);
               else
               {
                  sgHuntBuf.aProp[1][0]=0x00;
                  break;
               }
            }
            if(_xmchrchk(*clTmp)) return(FAIL);
            sgHuntBuf.aProp[0][ilLen++] = *clTmp;
            continue;
         }
         /*取属性值*/
         if(clPvalue)
         {
            if(*clTmp == ']')
            {
               if(ilLen <= 0)
                  return(FAIL);
               else
               {
                  sgHuntBuf.aProp[1][ilLen] = 0x00;
                  break;
               }
            }
            if(_xmkeychk(*clTmp)) return(FAIL);
            sgHuntBuf.aProp[1][ilLen++] = *clTmp;
            continue;
         }
 
         if(*clTmp == '@')
         {
            clPname = 0x01;
            ilLen = 0;
            continue;
         }
         else if(*clTmp >= '0' && *clTmp <= '9')
         {
            clNum = 0x01;
            n -= 1;
            continue;
         }
         else
            return(FAIL);
      }
 
      if(*clTmp == ' ' && clPath == 0x00 && clProp == 0x00)
         continue;
      else
      {
         clPath = 0x01;
         if(*clTmp == '/')
            sgHuntBuf.cRoot = 0x01;
         else
         {
            n -= 1;
            sgHuntBuf.cRoot = 0x00;
         }
      }
   }  /*end for*/
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlGet()
 ** 功  能      :   取指定节点值或属性值
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmParseXpath() 
 ** 全局变量    :   sgHuntBuf sgXmlhead
 ** 参数含义    :   输入: aPath  -- XML路径
 **                       aResult -- 查询结果
 **                 输出: pointer -- 返回属性或节点指针
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlGet(char *aPath,char *aResult,char **pointer)
{
   int ilRc,ilCount=0;
   XMLNODE *slNode;
   XMLPROP *slProp;
   int n;

   if(NULL == aPath || NULL == aResult) return(ERR_INVALID_PARM);
   if(strlen(aPath) > XPATHLEN) return(ERR_VALUE_TOO_LONG);
   ilRc = _xmParseXpath(aPath);
   if(ilRc) return(ERR_WRONG_PATH);

   if(sgXmlhead.iNodeCount <= 0) return(ERR_NOT_PREPARE);

   ilCount = sgHuntBuf.iNum;
   if(!sgHuntBuf.cRoot)
   {
      slNode = sgHuntBuf.sNode->sChildNode;
      if(NULL == slNode) slNode = sgXmlhead.sNode;
   }
   else
      slNode = sgXmlhead.sNode;

   for(n=0;n<=sgHuntBuf.iLevel;n++)
   {
      do
      {
         if(!strcmp(slNode->aNodename,sgHuntBuf.aXpath[n]))
            break;
         else
            slNode = slNode->sBrotherNext;
      } while(slNode != NULL);

      if(NULL == slNode) return(ERR_NOT_FOUND);

      if(n == sgHuntBuf.iLevel)
      {  /*取第iNum个同名称节点*/
         if(sgHuntBuf.iNum > 0)
         {
            if(ilCount > 1)
            {
               ilCount -= 1;
               slNode = slNode->sBrotherNext;
               if(NULL == slNode) return(ERR_NOT_FOUND);
               n -= 1;
               continue;
            }
            else
               break;
         }
         else if(strlen(sgHuntBuf.aProp[0]) > 0)       /*取属性*/
         {
            slProp = slNode->sProp;
            if(NULL == slProp) return(ERR_NOT_FOUND);
            do
            {
               if(strcmp(slProp->aPropname,sgHuntBuf.aProp[0]))
               {
                  slProp = slProp->spPropnext;
                  continue;
               }
               if(strlen(sgHuntBuf.aProp[1]) > 0)
               {
                  if(!strcmp(slProp->aPropvalue,sgHuntBuf.aProp[1]))
                     break;
               }  
               else
                  break;
            } while(slProp != NULL);

            if(NULL == slProp) 
            {
               slNode = slNode->sBrotherNext;
               if(NULL == slNode) return(ERR_NOT_FOUND);
               n -= 1;
               continue;             /*return(ERR_NOT_FOUND);*/
            }
         }
         break;
      }
      else
         slNode = slNode->sChildNode;
   }
   if(n > sgHuntBuf.iLevel) return(ERR_NOT_FOUND);

   /*取值 Xpath 例: "/root/txcode[@prop]" */
   if(strlen(sgHuntBuf.aProp[0])>0 && strlen(sgHuntBuf.aProp[1])==0)
   {
      if(pointer != NULL) *pointer = (char *)slProp;
      strcpy(aResult,slProp->aPropvalue);
   }
   else
   {
      if(pointer != NULL) *pointer = (char *)slNode;
      strcpy(aResult,slNode->aNodevalue);
   }
   sgHuntBuf.sNode = slNode->sParentNode;
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlGetValue()
 ** 功  能      :   取指定节点值或属性值
 ** 作  者      :   nh
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmParseXpath() 
 ** 全局变量    :   sgHuntBuf sgXmlhead
 ** 参数含义    :   输入: aPath  -- XML路径
 **                       aResult -- 查询结果
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlGetValue(char *aPath,char *aResult)
{
   int ilRc,ilCount=0;
   XMLNODE *slNode;
   XMLPROP *slProp;
   int n;

   if(NULL == aPath || NULL == aResult) return(ERR_INVALID_PARM);
   if(strlen(aPath) > XPATHLEN) return(ERR_VALUE_TOO_LONG);
   ilRc = _xmParseXpath(aPath);
   if(ilRc) return(ERR_WRONG_PATH);

   if(sgXmlhead.iNodeCount <= 0) return(ERR_NOT_PREPARE);

   ilCount = sgHuntBuf.iNum;
   if(!sgHuntBuf.cRoot)
   {
      slNode = sgHuntBuf.sNode->sChildNode;
      if(NULL == slNode) slNode = sgXmlhead.sNode;
   }
   else
      slNode = sgXmlhead.sNode;

   for(n=0;n<=sgHuntBuf.iLevel;n++)
   {
      do
      {
         if(!strcmp(slNode->aNodename,sgHuntBuf.aXpath[n]))
            break;
         else
            slNode = slNode->sBrotherNext;
      } while(slNode != NULL);

      if(NULL == slNode) return(ERR_NOT_FOUND);

      if(n == sgHuntBuf.iLevel)
      {  /*取第iNum个同名称节点*/
         if(sgHuntBuf.iNum > 0)
         {
            if(ilCount > 1)
            {
               ilCount -= 1;
               slNode = slNode->sBrotherNext;
               if(NULL == slNode) return(ERR_NOT_FOUND);
               n -= 1;
               continue;
            }
            else
               break;
         }
         else if(strlen(sgHuntBuf.aProp[0]) > 0)       /*取属性*/
         {
            slProp = slNode->sProp;
            if(NULL == slProp) return(ERR_NOT_FOUND);
            do
            {
               if(strcmp(slProp->aPropname,sgHuntBuf.aProp[0]))
               {
                  slProp = slProp->spPropnext;
                  continue;
               }
               if(strlen(sgHuntBuf.aProp[1]) > 0)
               {
                  if(!strcmp(slProp->aPropvalue,sgHuntBuf.aProp[1]))
                     break;
               }  
               else
                  break;
            } while(slProp != NULL);

            if(NULL == slProp) 
            {
               slNode = slNode->sBrotherNext;
               if(NULL == slNode) return(ERR_NOT_FOUND);
               n -= 1;
               continue;             /*return(ERR_NOT_FOUND);*/
            }
         }
         break;
      }
      else
         slNode = slNode->sChildNode;
   }
   if(n > sgHuntBuf.iLevel) return(ERR_NOT_FOUND);

   /*取值 Xpath 例: "/root/txcode[@prop]" */
   if(strlen(sgHuntBuf.aProp[0])>0 && strlen(sgHuntBuf.aProp[1])==0)
   {
      strcpy(aResult,slProp->aPropvalue);
   }
   else
   {
      strcpy(aResult,slNode->aNodevalue);
   }
   sgHuntBuf.sNode = slNode->sParentNode;
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlpackInit()
 ** 功  能      :   XML报文预解包
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:  _xmAppendHead _xmAppendNode 
 **                _xmAppendProp _xmAppendPropValue
 ** 全局变量    :   cgpackInit 
 ** 参数含义    :   输入：aVersion  -- XML版本(例"1.0")
 **                       aEncoding -- XML编码(例"UTF-8")
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlpackInit(char *aVersion,char *aEncoding)
{
   int ilRc;
   XMLNODE *slNode;
   XMLPROP *slProp;
   if(NULL == aVersion || NULL == aEncoding) return(ERR_INVALID_PARM);
   /*判断数据长度是否超出限制*/
   if(strlen(aVersion) > PROPVALUE || strlen(aEncoding) > PROPVALUE)
      return(ERR_VALUE_TOO_LONG);
   if(cgpackInit)  return(ERR_ALREADY_INIT);
   /* 添加XML头节点 */
   slNode = _xmAppendHead("xml");
   if(NULL == slNode) return(igErrData);
   slProp = _xmAppendProp(slNode,"version");
   if(NULL == slProp) return(igErrData);
   ilRc = _xmAppendPropValue(slProp,aVersion);
   if(ilRc) return(ilRc);
   slProp = _xmAppendProp(slNode,"encoding");
   if(NULL == slProp) return(igErrData);
   ilRc = _xmAppendPropValue(slProp,aEncoding);
   if(ilRc) return(ilRc);
   cgpackInit = 0x01;
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlunpackInit()
 ** 功  能      :   XML报文预解包
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmInitRuleTable()  _xmExpCheck()
 ** 全局变量    :   sgXmlhead agMsgbuf igMsgbuflen cgunpackInit
 ** 参数含义    :   输入：aMsgbody    -- XML报文数据
 **                       iMsgbodylen -- aMsgbody长度
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlunpackInit(char *aMsgbody,int iMsgbodylen)
{
   int ilRc;
   int i;
   int ilOffset=0;
   char *alTmp;
   
   char *aStart;
   char *aPos;
   int iPos;
      
   if(NULL == aMsgbody || iMsgbodylen <= 0) return(ERR_INVALID_PARM);
   if(cgunpackInit)  return(ERR_ALREADY_INIT);
   /*解包操作*/
   _xmInitRuleTable();

   	/* add by lyg 2006-12-18 13:59 */
iMsgbodylen = strlen(aMsgbody);   

/*add by pc 替换引号只能从头之后，所以加上求头位置的语句*/
aStart=aMsgbody;
aPos=strchr(aMsgbody,'>');
if (aPos!=NULL) {
	iPos=aPos-aStart;
}	
/*add by pc 替换引号只能从头之后，所以加上求头位置的语句*/
	
   for(i=0;i<iMsgbodylen;i++)
   {
      if(aMsgbody[i] == '\t')
      	  aMsgbody[i] = ' ';
      else if(aMsgbody[i] == '\'')
      {
      	  memmove(aMsgbody+i+5,aMsgbody+i,iMsgbodylen-i);
      	  memcpy(aMsgbody+i,"&apos;",6);
      	  iMsgbodylen += 5;
      	  i += 5;
      }
      else if ((aMsgbody[i] == '\"')&&(i>iPos))
      {
      	  memmove(aMsgbody+i+5,aMsgbody+i,iMsgbodylen-i);
      	  memcpy(aMsgbody+i,"&quot;",6);
      	  iMsgbodylen += 5;
      	  i += 5;
      }      
      else 
      	  continue;
   }  
  /* add by lyg 2006-12-18 13:59 */
  
   agMsgbuf = aMsgbody;
   igMsgbuflen = iMsgbodylen;
   agMsgbuf[igMsgbuflen]=0;
   ilRc = _xmExpCheck(NULL,"&E_MN",&ilOffset);
   if(ilRc) {
   		swVdebug(1,"_xmExpCheck 校验失败[%d]",ilRc);
   		return(ilRc);
   }
   cgunpackInit = 0x01; 
   return(SUCCESS);
}


/**************************************************************
 ** 函数名      :   xmlAddNode()
 ** 功  能      :   XML打包添加节点
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   xmlGet()  _xmAppendNode()  _xmAppendNodeValue()
 ** 全局变量    :   
 ** 参数含义    :   输入：aXpath -- 路径
 **                       aNode  -- 节点名称
 **                       aValue -- 节点值
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlAddNode(char *aXpath,char *aNode,char *aValue)
{
   int ilRc;
   char alBuf[NODEVALUE+1];
   XMLNODE *slParent,*slNode;  
   
   memset(alBuf,0x00,sizeof(alBuf));
   if(NULL == aNode || NULL == aXpath) return(ERR_INVALID_PARM);
   /*判断值长度是否超出限制*/
   if(strlen(aNode) > NODENAME || strlen(aValue) > NODEVALUE ||
      strlen(aXpath) > XPATHLEN)
      return(ERR_VALUE_TOO_LONG); 
   if(!strcmp(aXpath,"/"))
      slParent = NULL;
   else
   {
   	  /*ilRc = xmlGet(aXpath,alBuf,(char **)&slParent);*/
      memset(alBuf,0x00,sizeof(alBuf));
 
   	  ilRc = xmlGetNode(aXpath,alBuf,(char **)&slParent);
      if(ilRc) return(ilRc);
      /*检查节点类型*/
      if(slParent->iNodeType != NOD_BRANCH &&
         slParent->iNodeType != NOD_NORMAL)
         return(ERR_INVALID_NOD_TYPE);
   }
   /*检查该slParent节点下是否有同名节点*/
   /*如果不可以有同名子节点,需要取消注释*/
/*
   slNode = slParent->sChildNode;
   while(slNode != NULL)
   {
      if(!strcmp(slNode->aNodename,aNode))
        return(ERR_NODE_EXIST);
      slNode = slNode->sBrotherNext;
   }
*/
   /*如果值为空，则插入叶节点*/
   if(NULL == aValue)
   {  
      slNode = _xmAppendNode(slParent,aNode,NOD_LEAF);
      
      if(NULL == slNode) return(igErrData);
   }
   else
   {    	
      slNode = _xmAppendNode(slParent,aNode,NOD_BRANCH);
      if(NULL == slNode) return(igErrData);
      ilRc = _xmAppendNodeValue(slNode,aValue);
      if(ilRc) return(ilRc);
   }
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlSetNode()
 ** 功  能      :   更改节点值
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   xmlGet()  
 ** 全局变量    :   sgHuntBuf 
 ** 参数含义    :   输入：aXpath -- 路径
 **                       aValue -- 节点值
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlSetNode(char *aXpath,char *aValue)
{
   int ilRc;
   char alBuf[NODEVALUE+1];
   XMLNODE *slNode;
   
   if(NULL == aXpath || NULL == aValue)
      return(ERR_INVALID_PARM);
   /*判断值是否超出限制*/
   if(strlen(aXpath) > XPATHLEN || strlen(aValue) > NODEVALUE)
      return(ERR_VALUE_TOO_LONG);

   /* 查找节点 */
   /*ilRc = xmlGet(aXpath,alBuf,(char **)&slNode);*/
   ilRc = xmlGetNode(aXpath,alBuf,(char **)&slNode);
   if(ilRc) return(ilRc);

   /* 如果aXpath是 "/root/txcode" 或 "/root/txcode[2]" 
      或"/root/txcode[@prop1=value]"形式，则返回的指针
      是节点指针，如果是"/root/txcode[@prop1]"形式，则
　　　返回的指针是属性指针，此时如果对其操作必然出错
   */
   if(sgHuntBuf.iNum == 0 &&
      sgHuntBuf.aProp[0][0] != 0 && sgHuntBuf.aProp[1][0] == 0)
      return(ERR_WRONG_PATH);

   strcpy(slNode->aNodevalue,aValue);
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlAddProp()
 ** 功  能      :   添加节点属性
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmAppendProp()  xmlGet()
 ** 全局变量    :   sgHuntBuf
 ** 参数含义    :   输入：aXpath -- 路径
 **                       aValue -- 节点值
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
 ** 说  明      :   aXpath必须是类似"/root/txcode[2]" 或
 **                 "/root/txcode" 或 "/root/txcode[@prop=123]"的形式
***************************************************************/
int xmlAddProp(char *aXpath,char *aProp,char *aValue)
{
   int ilRc;
   XMLNODE *slNode;
   XMLPROP *slProp;
   char alBuf[NODEVALUE+1];

   if(NULL == aXpath || NULL == aProp || NULL == aValue) 
      return(ERR_INVALID_PARM);
   /*判断值长度是否超出限制*/
   if(strlen(aProp) > PROPNAME || strlen(aValue) > PROPVALUE ||
      strlen(aXpath) > XPATHLEN)
      return(ERR_VALUE_TOO_LONG);
   
   /*ilRc = xmlGet(aXpath,alBuf,(char **)&slNode);*/
   ilRc = xmlGetNode(aXpath,alBuf,(char **)&slNode);
   if(ilRc) return(ilRc);

   /* 如果aXpath是 "/root/txcode" 或 "/root/txcode[2]" 
      或"/root/txcode[@prop1=value]"形式，则返回的指针
      是节点指针，如果是"/root/txcode[@prop1]"形式，则
　　　返回的指针是属性指针，此时如果对其操作必然出错
   */
   if(sgHuntBuf.iNum == 0 &&
      sgHuntBuf.aProp[0][0] != 0 && sgHuntBuf.aProp[1][0] ==0)
      return(ERR_WRONG_PATH);
   
   /*查找该节点下是否已经存在该属性 */
   slProp = slNode->sProp;
   while(slProp != NULL)
   {
      if(!strcmp(slProp->aPropname,aProp))
         return(ERR_PROP_EXIST);
      slProp = slProp->spPropnext;
   }
   /*添加属性*/
   slProp = _xmAppendProp(slNode,aProp);
   if(NULL == slProp) return(igErrData);
   strcpy(slProp->aPropvalue,aValue);
   return(SUCCESS);
}


/**************************************************************
 ** 函数名      :   xmlSetProp()
 ** 功  能      :   修改节点属性值
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   xmlGet()   
 ** 全局变量    :   sgHuntBuf
 ** 参数含义    :   输入：aXpath -- 路径(可包含属性)
 **                       aProp  -- 属性名称(如果为NULL则需要包含在aXpath中)
 **                       aValue -- 属性值
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
 ** 说  明      :   aXpath必须是类似"/root/txcode[2]" 或
 **                 "/root/txcode" 或 "/root/txcode[@prop=123]"的形式
***************************************************************/
int xmlSetProp(char *aXpath,char *aProp,char *aValue)
{
   int ilRc;
   XMLPROP *slProp;
   XMLNODE *slNode;
   char alBuf[NODEVALUE+1];
   

   if(NULL == aXpath || NULL == aValue) 
      return(ERR_INVALID_PARM);
   /*判断值长度是否超出限制*/
   if(strlen(aValue) > PROPVALUE || strlen(aXpath) > XPATHLEN)
      return(ERR_VALUE_TOO_LONG);

   /*ilRc = xmlGet(aXpath,alBuf,(char **)&slNode);*/
   ilRc = xmlGetNode(aXpath,alBuf,(char **)&slNode);
   if(ilRc) return(ilRc);
   /* 如果aXpath是 "/root/txcode" 或 "/root/txcode[2]" 
      或"/root/txcode[@prop1=value]"形式，则返回的指针
      是节点指针，如果是"/root/txcode[@prop1]"形式，则
　　　返回的指针是属性指针，此时如果对其操作必然出错
   */
   if(sgHuntBuf.iNum == 0 &&
      sgHuntBuf.aProp[0][0] != 0 && sgHuntBuf.aProp[1][0] == 0)
      return(ERR_WRONG_PATH);

   /*如果传入的属性名称为空，则从Xpath中取, 这时要求Xpath必须是
    "/root/txcode[@prop1=123]"的格式*/
   if(NULL == aProp)
   {
      if(sgHuntBuf.iNum > 0 || sgHuntBuf.aProp[0][0] == 0)
        return(ERR_WRONG_PATH);
      strcpy(alBuf,sgHuntBuf.aProp[0]);
   }
   else 
      strcpy(alBuf,aProp);
   /*在当前节点下搜索属性*/
   slProp = slNode->sProp;
   while(slProp != NULL)
   {
      if(!strcmp(slProp->aPropname,aProp))
         break;
      slProp = slProp->spPropnext;
   }
   if(NULL == slProp) return(ERR_NOT_FOUND);
   strcpy(slProp->aPropvalue,aValue);
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   _xmDumpNode()
 ** 功  能      :   导出xml节点到agMsgbuf中
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmCtoA()
 ** 全局变量    :   agMsgbuf
 ** 参数含义    :   输入：sNode   -- 需要导出的节点指针
 **                       iOffset -- 当前buffer位置
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int _xmDumpNode(XMLNODE *sNode,int *iOffset)
{
   XMLNODE *slNode;
   XMLPROP *slProp;
   char alBuf[NODEVALUE+1];
   int ilOffset;
   int ilRc;

   if(NULL == sNode || *iOffset < 0) 
      return(ERR_INVALID_PARM);
   ilOffset = *iOffset;
   slNode = sNode;
   while(slNode != NULL)
   {
      if(igDumpFile) 
      {
         memset(agMsgbuf+ilOffset,' ',igDepth);
         ilOffset += igDepth;
      }
      *(agMsgbuf+ilOffset++) = '<';
      if(slNode->iNodeType == NOD_HEAD)
         *(agMsgbuf+ilOffset++) = '?';
      
      strcpy(agMsgbuf+ilOffset,slNode->aNodename);
      ilOffset += strlen(slNode->aNodename);
      slProp = slNode->sProp;
      while(slProp != NULL)
      {
         *(agMsgbuf + ilOffset++) = ' ';
         strcpy(agMsgbuf+ilOffset,slProp->aPropname);
         ilOffset += strlen(slProp->aPropname);
         *(agMsgbuf + ilOffset++) = '=';
         *(agMsgbuf + ilOffset++) = '\"';
         strcpy(agMsgbuf+ilOffset,slProp->aPropvalue);
         ilOffset += strlen(slProp->aPropvalue);
         *(agMsgbuf + ilOffset++) = '\"';
         slProp = slProp->spPropnext;
     
      }
      /*叶节点*/
      if(slNode->iNodeType == NOD_LEAF)
      {
         *(agMsgbuf + ilOffset++) = '/';
         *(agMsgbuf + ilOffset++) = '>';
         if(igDumpFile) *(agMsgbuf + ilOffset++) = '\n'; 
         slNode = slNode->sBrotherNext;
         continue;
      }
      /*头节点*/
      if(slNode->iNodeType == NOD_HEAD)
      {
         *(agMsgbuf + ilOffset++) = '?';
         *(agMsgbuf + ilOffset++) = '>';
         if(igDumpFile) *(agMsgbuf + ilOffset++) = '\n'; 
         slNode = slNode->sBrotherNext;
         continue;
      }
      /*枝节点*/
      *(agMsgbuf + ilOffset++) = '>';
      strcpy(alBuf,slNode->aNodevalue);

/*转义*/
      xmlCtoA(alBuf);  

      strcpy(agMsgbuf+ilOffset,alBuf);
      ilOffset += strlen(alBuf);


      if(slNode->sChildNode != NULL)
      {
         if(igDumpFile) *(agMsgbuf + ilOffset++) = '\n';
         igDepth += 2;
         ilRc = _xmDumpNode(slNode->sChildNode,&ilOffset);
         igDepth -= 2;
         if(ilRc) return(ilRc);
      }
   /*   else  del by nh 
      { */
      /*}*/
      *(agMsgbuf + ilOffset++) = '<';
      *(agMsgbuf + ilOffset++) = '/';
      strcpy(agMsgbuf+ilOffset,slNode->aNodename);
      ilOffset += strlen(slNode->aNodename);
      *(agMsgbuf + ilOffset++) = '>';
      if(igDumpFile) *(agMsgbuf + ilOffset++) = '\n';  

      slNode = slNode->sBrotherNext;
   } /*end while slNode*/ 
   *iOffset = ilOffset;
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlDump()
 ** 功  能      :   导出XML到指定buffer中
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmDumpNode()  
 ** 全局变量    :   sgXmlhead agMsgbuf
 ** 参数含义    :   输入：aMsgbuf    -- 存储XML
 **                       iMsgbuflen -- 打包后aMsgbuf长度
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlDump(char *aMsgbuf,int *iMsgbuflen)
{
   int ilRc;
   int ilOffset=0;

   if(NULL == aMsgbuf) return(ERR_INVALID_PARM);

   agMsgbuf = aMsgbuf;
   igDepth = 0;
   ilRc = _xmDumpNode(sgXmlhead.shead,&ilOffset);
  
   if(ilRc) return(ilRc);
   ilRc = _xmDumpNode(sgXmlhead.sNode,&ilOffset);
  
   if(ilRc) return(ilRc);
   agMsgbuf[ilOffset]=0;
  
   *iMsgbuflen = ilOffset;
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   xmlDumpFile()
 ** 功  能      :   导出XML到指定文件中
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   xmlDump()
 ** 全局变量    :   igDumpFile
 ** 参数含义    :   输入：aFileName -- 文件名
 ** 返回值      :    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int xmlDumpFile(char *aFileName)
{
   char alMsgbuf[NODEVALUE+1];
   int ilMsgbuflen;
   int ilRc;
   FILE *fp;
  
   fp = fopen(aFileName,"w+");
   if(NULL == fp) return(ERR_OPEN_FILE);
   igDumpFile = 1;
   ilRc = xmlDump(alMsgbuf,&ilMsgbuflen);
   igDumpFile = 0;
   if(ilRc)
   {
      fclose(fp);
      return(ilRc);
   }
   fwrite(alMsgbuf,ilMsgbuflen,1,fp);
   fclose(fp);
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   _xmAllocNode()
 ** 功  能      :   预解包分配节点资源
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :   _sgNode _cgNode
 ** 参数含义    :   (无)
 ** 返回值      :   节点指针 -- 成功  NULL -- 失败
***************************************************************/
XMLNODE *_xmAllocNode()
{
   int i=0;
   while(_cgNode[i] != 0x00 && i < MAXNODE) i += 1;
   if(MAXNODE == i)
      return (NULL);
   _cgNode[i] = 0x01;
   memset(&_sgNode[i],0x00,sizeof(XMLNODE));
   _sgNode[i].flag = i;
   return(&_sgNode[i]);
}

/**************************************************************
 ** 函数名      :   _xmFreeProp()
 ** 功  能      :   释放属性资源
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :   _cgProp
 ** 参数含义    :   输入:  sProp -- 要释放的属性指针
 ** 返回值      :   (无)
***************************************************************/
void _xmFreeProp(XMLPROP *sProp)
{
   if(sProp == NULL) return;
   if(sProp->flag >=0 && sProp->flag < MAXNODE)
   {
      _cgProp[sProp->flag] = 0x00;
      sgXmlhead.iPropCount -= 1;
   }
   sProp->sOwnerNode = NULL;
   sProp->spPropnext = NULL;
   sProp->aPropname[0] = 0;
   sProp->aPropvalue[0] = 0;
   sProp->flag = 0;
   sProp = NULL;
}


/**************************************************************
 ** 函数名      :   _xmFreeNode()
 ** 功  能      :   释放节点资源
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmFreeProp()  
 ** 全局变量    :   _sgNode cgNode sgXmlhead
 ** 参数含义    :   输入:  sNode -- 要释放的节点指针
 ** 返回值      :   (无)
***************************************************************/
void _xmFreeNode(XMLNODE *sNode)
{
   XMLPROP *slProp,*slProp2;
   if(sNode == NULL) return;
   slProp = sNode->sProp;
   if(slProp != NULL)
   {
      while(slProp->spPropnext != NULL)
      {
         slProp2 = slProp;
         slProp = slProp->spPropnext;
         _xmFreeProp(slProp2);
      }
   }  
   _xmFreeProp(slProp);
   if(sNode->flag >=0 && sNode->flag < MAXNODE)
   {
      _cgNode[sNode->flag] = 0x00;
      sgXmlhead.iNodeCount -= 1;
      if(igLevel >=0 && igLevel < XMLMAXLEVEL)
      {      	
         if(sgXmlhead.sNodeLevel[igLevel] == sNode)
                sgXmlhead.sNodeLevel[igLevel] = NULL;                
             if(sNode->sBrotherPrev !=NULL)
                sNode->sBrotherPrev->sBrotherNext = NULL;
  
			/* add by lyg 2006-12-14 11:02 */
			if(sNode->sParentNode != NULL){				
			/* add by lyg 2006-12-14 11:02 */	             
             if(sNode->sParentNode->sChildNode == sNode)
                sNode->sParentNode->sChildNode = NULL;
            }               
             if(sgXmlhead.sNode == sNode)
                  sgXmlhead.sNode = NULL;
             sgXmlhead.iNLevelCount[igLevel]   -= 1;            
      }
      sNode->sParentNode = NULL;     
      sNode->sBrotherPrev = NULL;
      sNode->sBrotherNext = NULL;    
      sNode->sChildNode = NULL;     
      sNode->sProp = NULL;    
      sNode->iPropCount = 0;
      sNode->iNodeType = 0;
      sNode->aNodename[0] = 0;
      sNode->aNodevalue[0] = 0;
      sNode->flag = 0;
   } 
   sNode = NULL;
}

/**************************************************************
 ** 函数名      :   _xmAllocProp()
 ** 功  能      :   预解包分配属性资源
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :   _cgProp _sgProp
 ** 参数含义    :   (无)
 ** 返回值      :    属性指针 -- 成功  NULL -- 失败
***************************************************************/
XMLPROP *_xmAllocProp()
{
   int i=0;
   while(_cgProp[i] != 0x00 && i<MAXNODE) i += 1;
   if(MAXNODE == i)
      return (NULL);
   _cgProp[i] = 0x01;
   _sgProp[i].flag = i;
   return(&_sgProp[i]);
}


/**************************************************************
 ** 函数名      :   xmlFree()
 ** 功  能      :   释放资源
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :    sgXmlhead sgHuntBuf _sgProp _sgNode 
 **                  _cgProp _cgNode cgunpackInit cgpackInit
 **                  igErrData igLevel agMsgbuf igMsgbuflen
 ** 参数含义    :
 ** 返   回   值:   (无)
***************************************************************/
void xmlFree()
{
   memset(&sgXmlhead,0x00,sizeof(XMLHEAD));
   memset(&sgHuntBuf,0x00,sizeof(XMLHUNTBUF));
   memset(&_sgNode[0],0x00,sizeof(XMLNODE)*MAXNODE);
   memset(&_sgProp[0],0x00,sizeof(XMLPROP)*MAXNODE);
   memset(_cgNode,0x00,sizeof(_cgNode));
   memset(_cgProp,0x00,sizeof(_cgProp));
   cgunpackInit = 0x00;
   cgpackInit = 0x00;
   igErrData = 0;
   igLevel = 0;
   agMsgbuf = NULL;
   igMsgbuflen = 0;
}

/**************************************************************
 ** 函数名      :   _xmInitRuleTable()
 ** 功  能      :   初始化igVarpos和igExppos
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :   agRuleVar,igVarpos,agRuleExp,igExppos
 ** 参数含义    :   
 ** 返回值      :   (无)
***************************************************************/
void _xmInitRuleTable()
{
   int ilKeyCount=0,ilStrCount=0;
   /*初始化igVarpos*/
   if(cgInitRule != 0x00) return;
   while(ilStrCount<V_LEN)
   {
      if(agRuleVar[ilStrCount][0] == '#')
      {
         ilKeyCount += 1;
         igVarpos[ilKeyCount] = ilStrCount++;
      }
      else if(!strcmp(agRuleVar[ilStrCount],"[END]"))
         break;
      else
         ilStrCount += 1;
   }
   igVarpos[0] = ilKeyCount;
   igVarpos[V_LEN] = ilStrCount;

   ilStrCount = 0;
   ilKeyCount = 0;
   /*初始化igExppos*/
   while(ilStrCount<E_LEN)
   {
      if(agRuleExp[ilStrCount][0] == '#')
      {
         ilKeyCount += 1;
         igExppos[ilKeyCount] = ilStrCount++;
      }
      else if(!strcmp(agRuleExp[ilStrCount],"[END]"))
         break;
      else
         ilStrCount += 1;
   }
   igExppos[0] = ilKeyCount;
   igExppos[E_LEN] = ilStrCount;

   igLevel = 0;
   cgInitRule = 0x01;
}

/**************************************************************
 ** 函数名      :   _xmExpress()
 ** 功  能      :   从aSrc中取一个表达式或变量存入aBuf
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数: 
 ** 全局变量    :   
 ** 参数含义    :   输入： aSrc -- 表达式串
 **                 输出： aBuf -- 搜索到的子表达式
 ** 返   回   值:    0 -- 成功
 **                 -1 -- 失败
***************************************************************/
int _xmExpress(char *aSrc,char *aBuf)
{
   int n=0;
   while(*(aSrc+n) != 0x00)
   {
      if(n > 0 && (*(aSrc+n) == '&')) break;
      *(aBuf+n) = *(aSrc+n);
      n += 1;
   }
   *(aBuf+n) = 0x00;
   return(-1 * (n==0));
}

/**************************************************************
 ** 函数名      :   _xmVarCheck()
 ** 功  能      :   检查aMsgbuf处的字符串对应于aVar变量
 **                 的语法是否正确
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmVarRule()  _xmchrchk() _xmspechk()  _xmkeychk()
 ** 全局变量    :
 ** 参数含义    :   输入：iOffset -- 需要检查的agMsgbuf位置
 **                       aVar    -- 变量名
 ** 返回值      :    0 -- 成功
 **                 (其它见xmldef.h)
***************************************************************/
int _xmVarCheck(char *aVar,int *iOffset)
{
   char *alTmp;
   int ilRc,n,ilLen;
   char alRule[EXPLEN+1];

   if(*iOffset < 0 || NULL == aVar)
   {
#ifdef _XML_DEBUG
      printf("[_xmVarCheck] %s %d\n",aVar,*iOffset);
#endif
      return(ERR_INVALID_PARM);
   }
   /*已经搜索到结尾，直接返回一个一般错误*/
   if(*iOffset >= igMsgbuflen)
   {
      igErrData = igMsgbuflen;
#ifdef _XML_DEBUG
      printf("VarCheck 1\n");
#endif
      return(ERR_INVALID_CHAR);
   }

   ilRc = _xmVarRule(aVar,alRule);
   alTmp = agMsgbuf + *iOffset;
   ilLen = strlen(alRule);
   for(n=0;n<ilLen;n++)
   {
      if(alRule[n] == '$')
      {
         ilRc = _xmchrchk(*alTmp);
         if(ilRc) break;
      }
      else if(alRule[n] == '@')
      {
         ilRc = _xmspechk(*alTmp);
         if(ilRc) break;
      }
      else if(alRule[n] == '!')
      {
         ilRc = _xmkeychk(*alTmp);
         if(ilRc) break;
      }
      else
      {
         if(*alTmp != alRule[n])
         {
            igErrData = alTmp - agMsgbuf;
#ifdef _XML_DEBUG
        printf("VarCheck 2\n");
#endif
            return(ERR_INVALID_CHAR);
         }
         alTmp += 1;
         continue;
      }
      alTmp += 1;
      n -= 1;
   }  /*end for*/

   /*指针未移动，表明匹配失败*/
   if(alTmp == (agMsgbuf + *iOffset))
   {
      igErrData = alTmp - agMsgbuf;
#ifdef _XML_DEBUG
      printf("VarCheck 3\n");
#endif
      return(ERR_INVALID_CHAR);
   }
   else
   {
      *iOffset = alTmp - agMsgbuf;
      return(SUCCESS);
   }
}


/*add by pc 2008-11-10 支持中文字段*/
int isChineseChar( const char ch)
{
	return (ch & 0x80);
}

/*检查$规则*/
int _xmchrchk(char cTmp)
{
/*
   if((cTmp >= '0' && cTmp <= '9') ||
      (cTmp >= 'a' && cTmp <= 'z') ||
      (cTmp >= 'A' && cTmp <= 'Z') ||
      cTmp == '-' ||
      cTmp == '_') return(SUCCESS);
*/  
   if((cTmp >= '0' && cTmp <= '9') ||
      (cTmp >= 'a' && cTmp <= 'z') ||
      (cTmp >= 'A' && cTmp <= 'Z') ||
      cTmp == '-' ||cTmp == '_'||
      isChineseChar(cTmp)) return(SUCCESS);
          
   return(FAIL);
}

/*检查@规则*/
int _xmspechk(char cTmp)
{
    if(cTmp == 0x0d || cTmp == 0x0a || cTmp == ' ' || cTmp == '\t') return(SUCCESS);
   return(FAIL);
}
/*检查!规则*/
int _xmkeychk(char cTmp)
{
   if(cTmp == '<' || cTmp == '>' || cTmp == '\'' || cTmp == '\"')
      return(FAIL);
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   _xmVarRule()
 ** 功  能      :   取指定变量的规则串
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :   agRuleVar
 ** 参数含义    :   输入：aVar  -- 需要取规则的变量
 **                 输出：aRule -- 串地址
 ** 返   回   值:    0 -- 成功
 **                  (其它见xmldef.h)
***************************************************************/
int _xmVarRule(char *aVar,char *aRule)
{
   int ilKeyCount;
   int n;
   if(NULL == aVar || NULL == aRule)
   {
#ifdef _XML_DEBUG
      printf("[_xmVarRule] %s %s\n",aVar,aRule);
#endif
      return(ERR_INVALID_PARM);
   }
    ilKeyCount = igVarpos[0];
   for(n=1;n<=ilKeyCount;n++)
   {
      if(!strcmp(agRuleVar[igVarpos[n]]+1,aVar+1)) /*找到匹配*/
      {
         strcpy(aRule,agRuleVar[igVarpos[n]+1]);
         break;
      }
   }
   if(n > ilKeyCount) 
      return(ERR_INVALID_RULE);
   return(SUCCESS);
}

/**************************************************************
 ** 函   数   名:   _xmExpRule()
 ** 功        能:   取指定表达式的规则
 ** 作        者:
 ** 建立日期    :
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :   输入：aExp  -- 需要取规则的变量
 **                 输出：aRule -- 串地址
 **                       iRuleCount -- 规则数目
 ** 返   回   值:    0 -- 成功
 **                  ERR_NOT_FOUND -- 未找到匹配
 **                  (其它见xmldef.h)
***************************************************************/
int _xmExpRule(char *aExp,char aRule[][EXPLEN+1],int *iRuleCount)
{
   int ilStrCount,ilKeyCount;
   int n,m;
   if(NULL == aExp || NULL == iRuleCount)
   {
#ifdef _XML_DEBUG
      printf("[_xmExpRule] %s %s %d\n",aExp,aRule[0],*iRuleCount);
#endif
      return(ERR_INVALID_PARM);
   }
   ilStrCount = igExppos[E_LEN];
   ilKeyCount = igExppos[0];
   for(n=1,m=0;n<=ilKeyCount;n++)
   {
      if(!strcmp(agRuleExp[igExppos[n]]+1,aExp+1)) /*找到匹配*/
      {
          m=igExppos[n]+1;
          break;
      }
   }
   if(n > ilKeyCount)
      return(ERR_INVALID_RULE);
   else if(n  == ilKeyCount)
      *iRuleCount = ilStrCount - igExppos[n] - 1;
   else
      *iRuleCount = igExppos[n+1] - igExppos[n] - 1;

   for(n=0;n<*iRuleCount;n++)
      strcpy(aRule[n],agRuleExp[m+n]);
   return(SUCCESS);
}


/**************************************************************
 ** 函数名      :   _xmAppendPropValue()
 ** 功  能      :   添加属性值
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   
 ** 全局变量    :   
 ** 参数含义    :   输入：aName -- 属性名称
 **                       sProp -- 属性指针 
 ** 返   回   值:    0 -- 成功
 **                 -1 -- 失败
***************************************************************/
int _xmAppendPropValue(XMLPROP *sProp,char *aValue)
{
   if(NULL == sProp)
   {
#ifdef _XML_DEBUG
      printf("[_xmAppendPropValue]\n");
#endif
      return(ERR_INVALID_PARM);
   }
   strcpy(sProp->aPropvalue,aValue);
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      :   _xmAppendProp()
 ** 功  能      :   添加属性
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmAllocProp()
 ** 全局变量    :   sgXmlhead
 ** 参数含义    :   输入：aName  -- 属性名称
 **                       sOwner -- 拥有属性的节点指针 
 ** 返   回   值:    0 -- 成功
 **                 -1 -- 失败
***************************************************************/
XMLPROP *_xmAppendProp(XMLNODE *sOwner,char *aName)
{
   XMLPROP *slProp,*slTmp;

   if(NULL == sOwner)
   {
#ifdef _XML_DEBUG
      printf("[_xmAppendProp] %d %s\n",sOwner, aName);
#endif
      igErrData = ERR_INVALID_PARM;
      return(NULL);
   }
   slProp = _xmAllocProp();
   if(slProp == NULL)
   {
      igErrData = ERR_NOT_ENOUGH_RES;
      return(NULL);
   }

   slProp->sOwnerNode = sOwner;
   strcpy(slProp->aPropname,aName);

   if(NULL == sOwner->sProp)
      sOwner->sProp = slProp;
   else
   {
      slTmp = sOwner->sProp;
      while(NULL != slTmp->spPropnext) slTmp = slTmp->spPropnext;
      slTmp->spPropnext = slProp;
   }
   sOwner->iPropCount++;
   sgXmlhead.iPropCount++;
   return(slProp);
}

/**************************************************************
 ** 函数名      :   _xmAppendNodeValue()
 ** 功  能      :   添加节点值
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   xmlAtoC() _xmTrim()
 ** 全局变量    :   
 ** 参数含义    :   输入：aValue  -- 节点值
 **                       sNode   -- 节点指针
 ** 返回值      :    0 -- 成功
 **                 -1 -- 失败
***************************************************************/
int _xmAppendNodeValue(XMLNODE *sNode,char *aValue)
{
   if(NULL == sNode)
   {
#ifdef _XML_DEBUG
      printf("[_xmAppendNodeValue]\n");
#endif
      return(ERR_INVALID_PARM);
   }  
   _xmTrim(aValue);   
   xmlAtoC(aValue);  /*反转义*/  
   strcpy(sNode->aNodevalue,aValue);
   return(SUCCESS);
}


/**************************************************************
 ** 函   数   名:   _xmAppendNode()
 ** 功        能:   添加节点
 ** 作        者:
 ** 建立日期    :
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :   输入： sParent 父节点
 **                        aName   -- 节点名称
 **                        iType   -- 节点类型
 ** 返   回   值:    0 -- 成功
 **                 -1 -- 失败
***************************************************************/
XMLNODE *_xmAppendNode(XMLNODE *sParent,char *aName,int iType)
{
   XMLNODE *slTmp,*slNode;

   slNode = _xmAllocNode();
   if(NULL == slNode)
   {
#ifdef _XML_DEBUG
      printf("[_xmAppendNode] %s\n",aName);
#endif
      igErrData = ERR_NOT_ENOUGH_RES;
      return(NULL);
   }
   strcpy(slNode->aNodename,aName);
   slNode->iNodeType = iType;
   if(NULL == sParent)
      sgXmlhead.sNode = slNode;
   else
   {
      slNode->sParentNode = sParent;
      slTmp = sParent->sChildNode;
      if(slTmp == NULL)
          sParent->sChildNode = slNode;
      else
      {
         while(slTmp->sBrotherNext != NULL) slTmp = slTmp->sBrotherNext;
         slTmp->sBrotherNext = slNode;
         slNode->sBrotherPrev = slTmp;
      }
   }  
   sgXmlhead.iNodeCount++;
   if(igLevel < XMLMAXLEVEL && NULL == sgXmlhead.sNodeLevel[igLevel])
   {
      /*存储分级节点指针*/
      sgXmlhead.sNodeLevel[igLevel] = slNode;
      /*存储分级节点数目*/
      sgXmlhead.iNLevelCount[igLevel] += 1;
      /*存储节点深度*/
      if(sgXmlhead.iNodeLevel < igLevel)
         sgXmlhead.iNodeLevel = igLevel;
   }
   return(slNode);
}


/**************************************************************
 ** 函数名      :   _xmAppendHead()
 ** 功  能      :   添加头节点
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmAllocNode() 
 ** 全局变量    :   sgXmlhead
 ** 参数含义    :   输入：aName -- 节点名称
 ** 返回值      :   添加的节点指针 -- 成功 NULL -- 失败
***************************************************************/
XMLNODE *_xmAppendHead(char *aName)
{
   XMLNODE *slTmp,*slNode;

   slNode = _xmAllocNode();
   if(slNode == NULL)
   {
      igErrData = ERR_NOT_ENOUGH_RES;
      return(NULL);
   }

   slNode->iNodeType = NOD_HEAD;
   strcpy(slNode->aNodename,aName);
   slNode->iPropCount = 0;

   if(NULL == sgXmlhead.shead)
      sgXmlhead.shead = slNode;
   else
   {
      slTmp = sgXmlhead.shead;
      while(slTmp->sBrotherNext != NULL)
         slTmp = slTmp->sBrotherNext;
      slTmp->sBrotherNext = slNode;
      slNode->sBrotherPrev = slTmp;
   }
   sgXmlhead.iNodeCount += 1;
   return(slNode);
}

/**************************************************************
 ** 函数名      :   _xmStackCheck()
 ** 功  能      :   检查节点前后名称是否相同
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :   输入：aMacro -- 保存宏及对应节点名称的数组
                          aValue -- 要检查的节点名称
 ** 返   回   值:    0 -- 成功
 **                 -1 -- 失败
 ** 说        明:   aMacro存储第一个遇到的节点的值
 **                 再次遇到节点时将节点值与其中的值比较
 **                 如果相同，说明节点结束，否则出错
***************************************************************/
int _xmStackCheck(char *aMacro,char *aValue)
{

   if(NULL == aMacro || NULL == aValue)
   {
#ifdef _XML_DEBUG
      printf("[_xmStackCheck]%s %s\n",aMacro,aValue);
#endif
      return(ERR_INVALID_PARM);
   }
   if(aMacro[0] == 0x00)
   {
      strcpy(aMacro,aValue);
      return(ERR_NOT_FOUND);
   }
   else
   {
      if(!strcmp(aMacro,aValue))
      {
       /*  printf("aMacro [%s] aValue [%s]\n",aMacro,aValue);*/
         *aMacro = 0x00;
         return(SUCCESS);
      }
      else
         return(ERR_NODE_NOT_MATCH);
   }
}


/**************************************************************
 ** 函数名      :   _xmExpCheck()
 ** 功  能      :   检查agMsgbuf处的字符串对应于表达式aExp
 **                 的语法是否正确
 ** 作  者      :   cgx
 ** 建立日期    :   2004-01-08
 ** 最后修改日期:   2004-01-14
 ** 调用其它函数:   _xmExpRule()     _xmExpress()     _xmVarCheck()
 **                 _xmStackCheck()  _xmAppendNode()  _xmAppendHead()
 **                 _xmAppendProp()  _xmAppendNodeValue() 
 **                 _xmAppendPropValue()  _xmExpCheck()  _xmFreeNode()
 ** 全局变量    :   agMsgbuf igMsgbuflen  igErrData
 ** 参数含义    :   输入：iOffset -- 需要检查的agMsgbuf位置
 **                       sParent -- 表达式从属的节点指针
 **                       aExp    -- 表达式
 ** 返回值      :    0 -- 成功
 **                 (其它见xmldef.h)
***************************************************************/
int _xmExpCheck(XMLNODE *sParent,char *aExp,int *iOffset)
{
   int ilOffset=0,ilOld=0;
   int m,n,ilRc;
   char alBuf[EXPLEN+1];               /* 存储子表达式 */
   char alValue[NODEVALUE+1];          /* 存储节点名称*/
   int ilBuflen;
   char alMacro[NODENAME+1];           /* 堆栈 记录第一个节点名称*/
   char alRule[5][EXPLEN+1];           /* 取规则串用 */
   int ilRulelen;
   int ilRuleCount;

   XMLNODE *slNode;
   XMLPROP *slProp;

   /*检查参数*/
   if(NULL == aExp || *iOffset < 0)
   {
#ifdef _XML_DEBUG
      printf("[_xmExpCheck] %s %d\n",aExp,*iOffset);
#endif
      return(ERR_INVALID_PARM);
   }
   /*搜索到结尾，直接返回一个一般错误*/
   if(*iOffset >= igMsgbuflen)
   {
      igErrData = igMsgbuflen;
#ifdef _XML_DEBUG
      printf("ExpCheck [%d] \n",igMsgbuflen);
#endif
      return(ERR_INVALID_CHAR);
   }

   slNode = NULL;

   /*初始化堆栈*/
   alMacro[0] = 0x00;

   /*取规则*/
   ilRc = _xmExpRule(aExp,alRule,&ilRuleCount);
   if(ilRc)
      return (ERR_INVALID_RULE);
   m = 0;
   ilOld = *iOffset;

RULECHECK:
   ilRulelen = strlen(alRule[m]);
   for(n=0;n<ilRulelen;n++)
   {  	
      ilRc = _xmExpress(alRule[m]+n,alBuf);   /*取规则中的表达式*/
      if(ilRc) return(ERR_INVALID_RULE);
      ilBuflen = strlen(alBuf);
      if(alBuf[ilBuflen-1] == '*') alBuf[--ilBuflen] = 0x00;
      n = n + ilBuflen - 1;
      if(alBuf[1] == 'V')   /*变量*/
      {      	
         ilOffset = *iOffset;
         ilRc = _xmVarCheck(alBuf,iOffset);
         /*匹配失败 或其它错误 */
         if(ilRc)
         {        	
            if(ilRc > ERR_CRITICAL) return(ilRc);            	
            if(alRule[m][n+1] == '*')
            {           	
               n += 1;
               continue;
            }
            else {            	
               goto ERRCHK;
            }   
         }
         /*匹配成功*/
         if(alRule[m][n+1] == '*')
         {
            n -= ilBuflen;
/*            continue;*/
         }
         /*判断值长度是否超出限制*/
         if((*iOffset-ilOffset) > NODEVALUE) 
         {
            igErrData = ilOffset;
            return(ERR_VALUE_TOO_LONG);
         }
         /*取出值*/
         memcpy(alValue,agMsgbuf+ilOffset,*iOffset-ilOffset);
         alValue[*iOffset-ilOffset] = 0x00; 
         /*是节点名称****************/
         if(!strcmp(alBuf,"&V_NN")) 
         {        	
            /*判断值长度是否超出限制*/
            if(strlen(alValue) > NODENAME) 
            {
               igErrData = ilOffset;
               return(ERR_VALUE_TOO_LONG);
            }             
            /*比较上一次值*/
            ilRc = _xmStackCheck(alMacro,alValue);
            if(ilRc == ERR_NOT_FOUND)  /*未发现之前有同名节点*/
            {
               slNode = _xmAppendNode(sParent,alValue,m+1);  /*添加到链表*/
         
               if(NULL == slNode) return(igErrData);
            }
            else if(ilRc != SUCCESS)
            {
               if(ilRc == ERR_NODE_NOT_MATCH)
                      igErrData = slNode->flag;
               return(ilRc);
            }
            else
               continue;
         } /*end if "&V_NN" */

         /*头节点*/
         else if(!strcmp(alBuf,"&V_KEY"))
         {         	
            /*判断值长度是否超出限制*/
            if(strlen(alValue) > NODENAME) 
            {
               igErrData = ilOffset;
               return(ERR_VALUE_TOO_LONG);
            }
            slNode = _xmAppendHead(alValue);
            if(NULL == slNode) return(igErrData);
         }

         /*属性名称****************/
         else if(!strcmp(alBuf,"&V_PN"))
         {         	
            /*判断值长度是否超出限制*/
            if(strlen(alValue) > NODENAME) 
            {
               igErrData = ilOffset;
               return(ERR_VALUE_TOO_LONG);
            }
            /*添加属性*/
            slProp = _xmAppendProp(sParent,alValue);
            if(NULL == slProp) return(igErrData);
         }

         /*节点值****************/
         else if(!strcmp(alBuf,"&V_NV"))
         {         	
            /*添加节点值*/
            ilRc = _xmAppendNodeValue(slNode,alValue);
            if(ilRc) return(ilRc);
         }

         /*属性值****************/
         else if(!strcmp(alBuf,"&V_PV"))
         {          	
            /*添加属性值*/
            ilRc = _xmAppendPropValue(slProp,alValue);
            if(ilRc) return(ilRc);
         }
         else
            continue;

      }  /*end if 'V' */
      else if(alBuf[1] == 'E')
      {     	
         ilOffset = *iOffset;
         igLevel += 1;  /*深度递增*/
         if(igLevel > XMLMAXLEVEL) return(ERR_DEPTH_TOO_BIG);
         ilRc = _xmExpCheck(slNode,alBuf,iOffset);      
         igLevel -= 1;  /*深度递减*/
         /*匹配失败 或 其它错误*/
         if(ilRc)
         {
            if(ilRc > ERR_CRITICAL) return(ilRc);
            if(alRule[m][n+1] == '*')
            {        	
               n += 1;
               continue;
            }
            goto ERRCHK;
         }
         /*匹配成功*/
         if(alRule[m][n+1] == '*')
         {         	
            n -= ilBuflen;
            continue;
         }
      } /* end if 'E' */
      else
         return(ERR_INVALID_RULE);
   }  /*end for*/

/*当程序执行到这里时说明该条规则匹配成功*/
   return(SUCCESS);

ERRCHK:
   /*当匹配失败而并且表达式错误时会跳到这里检查是否还有其它规则
     如果没有，就返回错误
     如果有多个规则，且当前规则检查成功时，其它规则不再检查*/
   igErrData = *iOffset;
   /*
   if((m+1) == ilRuleCount)
   {
      *iOffset = ilOld;
      return(ilRc);
   } */
   m += 1;
   *iOffset = ilOld;  
   alMacro[0]=0x00;   
   _xmFreeNode(slNode);  
   if(m == ilRuleCount)
      return(ilRc);
      
   else{  	
      goto RULECHECK;
	}
}

/**************************************************************
 ** 函数名      ： _swParsexml
 ** 功  能      ： 解析XPATH
 ** 作  者      ： nh
 ** 建立日期    ： 2002/8/20
 ** 最后修改日期： 2002/8/28
 ** 调用其它函数：
 ** 全局变量    ： 
 ** 参数含义    ： aParm[0] : XPATH
 									 aPath: 当前路径PATH的节点名
 									 iNum: 第i个同名节点
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int _swParsexml(char *aXpath,char *aPath,short *iNum)
{
  int i=0,j;
  char alTmp[101];
  char alBuf[101];
  
  if(aXpath[0]=='\0')
  { 
#ifdef _XML_DEBUG  	
    printf("S0450: _swParsexml() 输入的XPATH为空!\n");
#endif
    aXpath[0]=0;
    return(SUCCESS);
  }
  if(aXpath[0] != '/')
  {
#ifdef _XML_DEBUG  	
    printf("S0460: _swParsexml() 所输入的XPATH不正确,请检查!\n");
#endif    
    return(SUCCESS);
  }

  for(i=1;i<strlen(aXpath);i++)
  {
    if(aXpath[i]=='/')
      break;
  }
  if(i > 1)
  {
    memcpy(aPath,aXpath+1,i-1);
    aPath[i-1]=0;
    /* 找出相对路径的值 */
    if(aPath[i-2]==']')
    { 
      for(j=i-2;j>0;j--)
      {
        if(aXpath[j]=='[')
          break;
      }    
      if(i-2 > j)
      {
        memcpy(alBuf,aPath+j,i-j-2);
        alBuf[i-j-2]=0;
        *iNum=atoi(alBuf)-1;        
        if(*iNum <= 0)
          *iNum=0;
      }      
      aPath[j-1]=0;
    }
  }
  if(strlen(aXpath) > i)
  {
    memcpy(alTmp,aXpath+i,strlen(aXpath)-i);
    alTmp[strlen(aXpath)-i]=0;
    strcpy(aXpath,alTmp);
  }
  else
    aXpath[0]=0;
   
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ： xmlGetNode
 ** 功  能      ： 取xml节点树里的值
 ** 作  者      ： nh
 ** 建立日期    ： 2002/8/20
 ** 最后修改日期： 2002/8/28
 ** 调用其它函数：
 ** 全局变量    ： 
 ** 参数含义    ： aParm[0] : XPATH
 									 aResult: 取得的节点值 
 									 pointer: 节点指针
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int xmlGetNode(char *aXpath,char *aResult,char **pointer)
{
  int  ilRc;
  char alPath[100],*alTmp;
  short ilNum=0,ilFlag=0,ilTmp=0;
  XMLNODE *slNode;

  ilRc=_swParsexml((char *)aXpath,(char *)alPath,&ilNum);
  if(ilRc)
  {
#ifdef _XML_DEBUG  	
    printf("S0200: 函数调用_swParsexml()出错\n");
#endif    
    return(FAIL);
  } 
  slNode = sgXmlhead.sNode;
  if ( slNode == NULL ){ /*add by gxz 2008.04.13*/
	aResult[0] = 0x0;
	return(SUCCESS);
  }
  if(aXpath[0] == 0)
  {
  	strcpy(aResult,slNode->aNodevalue);
    if(NULL != pointer) *pointer = (char *)slNode;
    return(SUCCESS);
  }

  strcpy(aResult,slNode->aNodevalue);
  while((alTmp=strchr(aXpath,'/')) != NULL)
  {
  	ilTmp = 0;
    ilNum = 0;
    ilRc=_swParsexml((char *)aXpath,(char *)alPath,&ilNum);
    if(ilRc)
    {
      return(FAIL);
    }     
    if(NULL != slNode->sChildNode)
    {
    	ilTmp=0;
      slNode=slNode->sChildNode;
    }
    else
      break;
    ilFlag = 0;
   
    while (NULL != slNode) 
    {     	
      if ((!strcmp(slNode->aNodename, alPath)))
      {     	
        if(ilFlag == ilNum)
        {
          ilTmp=1;
          break;
        }
        ilFlag++; 
      }
      if(NULL != slNode->sBrotherNext)
      {
        slNode = slNode->sBrotherNext;      
      }
      else 
        break;
    }
    if(ilTmp == 0)
    		break;
  }
  
  if(ilTmp != 1)
  {
    return(ERR_NOT_FOUND);
  }
  strcpy(aResult,slNode->aNodevalue);
  if(NULL != pointer) *pointer = (char *)slNode;
  return(SUCCESS);
}


/**************************************************************
 ** 函数名      ： xmlGetProp
 ** 功  能      ： 取xml节点树里的属性的值
 ** 作  者      ： nh
 ** 建立日期    ： 2002/8/20
 ** 最后修改日期： 2002/8/28
 ** 调用其它函数：
 ** 全局变量    ： 
 ** 参数含义    ： aParm[0] : XPATH
 									 aName: 属性名称
 									 aResult:  属性值
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int xmlGetProp(char *aXpath,char *aName,char *aResult)
{
  int  ilRc;
  short ilTmp=0;
  XMLNODE *slNode;
  XMLPROP *slProp;
  char alBuf[NODEVALUE+1];
  
  ilRc = xmlGetNode(aXpath,alBuf,(char **)&slNode);
  if(ilRc)
  { 
#ifdef _XML_DEBUG  	
  	printf("定位节点出差,请检查XPATH是否正确\n");
#endif  	
  	return(ilRc);
  }
 
  slProp = slNode->sProp;
  if(NULL == slProp) return(ERR_NOT_FOUND);
  do
  {
     if(!strcmp(slProp->aPropname,aName))
     {
       strcpy(aResult,slProp->aPropvalue);
       return(SUCCESS);
     }
     slProp = slProp->spPropnext; 
  } while(slProp != NULL);
  
  return(ERR_NOT_FOUND);
}

/****************************************
=========================================
=========================================
=========================================
*****************************************/


#ifdef __XML_TEST

int main(int argc,char *argv[])
{

   char agMsgbody[2048];
   int igMsgbodylen;
   char alTmp[200],alTmp1[200];

   FILE *fhd;

   int ilRc;
   char alBuf[2048],alXpath[100];

   _xmInitRuleTable();

   fhd = fopen("./test.xml","r");
   if(NULL == fhd)
   {
      printf("打开文件失败!\n");
      exit(-1);
   }
   memset(agMsgbody,0x00,sizeof(agMsgbody));
   fread(agMsgbody,2048,1,fhd);
   igMsgbodylen=strlen(agMsgbody);
   fclose(fhd);
   printf("[%d] \n%s\n",igMsgbodylen,agMsgbody);
   ilRc = xmlpackInit("1.0","UTF-8");
   if(ilRc)
   {
      printf("xmlInit Error %s\n",xmlDescription(ilRc));
      exit(0);
   }
   fprintf(stderr,"xmlInit Ok\n");
   fflush(stderr);
   /* 添加根 节点 */
   strcpy(alXpath,"/");
   strcpy(alBuf,"ap");
   strcpy(alTmp,"");
   ilRc = xmlAddNode(alXpath,alBuf,alTmp);
   if(ilRc) 
   {
      printf("xmlAddNode0 ERROR [%s]\n",xmlDescription(ilRc));
      exit(0);
   }
   printf("xmlAddNode ap Success /ap\n");
   
   /* 添加节点 */
   strcpy(alBuf,"body");
   strcpy(alXpath,"/ap");
/*
   strcpy(alTmp,"111");
*/
   strcpy(alTmp,"");
   ilRc = xmlAddNode(alXpath,alBuf,alTmp);
   if(ilRc) 
   {
      printf("xmlAddNode1 ERROR [%s]\n",xmlDescription(ilRc));
      exit(0);
   }
   printf("xmlAddNode body Success /ap\n");
   
   /* 添加节点 */
/*
   strcpy(alXpath,"/ap/body/ErpRequest/ErpReqHead");
   strcpy(alTmp,"PCTEST");
   ilRc = xmlAddNode(alXpath,"HeadCoCode",alTmp);
   if(ilRc) 
   {
      printf("xmlAddNode2 ERROR [%s]\n",xmlDescription(ilRc));
      exit(0);
   }
   printf("xmlAddNode other Success /ap\n");
 */  
   
      /*打包文件测试*/
   printf("*** DUMP file TEST ***\n");
   ilRc = xmlDumpFile("./output1.xml");
   if(ilRc)
   {
      printf("dump file error [%s]\n",xmlDescription(ilRc));
      exit(0);
   }
   
      
   /*打包buffer测试*/
   printf("*** DUMP buffer TEST ***\n");
   memset(agMsgbody,0x00,2048);
   ilRc = xmlDump(agMsgbody,&igMsgbodylen);
   if(ilRc)
   {
      printf("dump error [%s]\n",xmlDescription(ilRc));
      exit(0);
   }
   printf("[%d]\n%s\n",igMsgbodylen,agMsgbody);

   exit(0);
}

#endif

