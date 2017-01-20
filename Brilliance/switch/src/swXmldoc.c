#include "xmldef.h"

#define _XML_DEBUG____

XMLHEAD sgXmlhead;           /*XMLͷ�ṹ*/
XMLHUNTBUF sgHuntBuf;        /*��������*/
/****************************************
**  ��Դ����
****************************************/
XMLNODE _sgNode[MAXNODE];    /*�ڵ���Դ*/
char _cgNode[MAXNODE];
XMLPROP _sgProp[MAXNODE];    /*������Դ*/
char _cgProp[MAXNODE];

char cgunpackInit=0x00;      /*�����ʼ����־*/
char cgpackInit=0x00;        /*�����ʼ����־*/

char cgInitRule=0x00;        /*���ʽ��ʼ����־*/
int igErrData=0;             /*���Ӵ�������*/
int igLevel=0;               /*��ȼ�¼, ���ʱʹ��*/
int igDepth=0;               /*��ȼ�¼�����ʱʹ��*/
int igDumpFile=0;            /*�������ļ���־*/

char *agMsgbuf=NULL;
int igMsgbuflen=0;

/*******************************************************
  �������� 
  ��"xml"��ͷ�ĺ����ṩ���ⲿ����
  ��"_xm"��ͷ�ĺ���ֻ���ڲ�����
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
 ** ������      :   xmlDescription()
 ** ��  ��      :   ȡ��������
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :   ���룺i -- �������
 ** ����ֵ      :   (��)
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
 ** ������      :   _xmTrim()
 ** ��  ��      :   ɾ���ַ������ߵ� \r\t\n �Լ��ո�
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :   ���룺aBuf  -- ��Ҫ������ַ���
 ** ����ֵ      :   (��)
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
 ** ������      :   xmlAtoC()
 ** ��  ��      :   �ַ�����ת��
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :   cgExtraCh agExtraCh
 ** ��������    :   ���룺aBuf  -- ��Ҫ������ַ���
 ** ����ֵ      :   (��)
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
 ** ������      :   xmlCtoA()
 ** ��  ��      :   �ַ���ת��
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :   cgExtraCh agExtraCh
 ** ��������    :   ���룺aBuf  -- ��Ҫ������ַ���
 ** ����ֵ      :   (��)
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
 ** ������      :   _xmParseXpath()
 ** ��  ��      :   ����·����sgHuntBuf��
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmchrchk()  _xmkeychk() 
 ** ȫ�ֱ���    :   sgHuntBuf
 ** ��������    :   ���룺aPath -- ·����
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
 ** ˵        ��:   aPath���� ·��[��n��ͬ���ڵ�] /root/text[2]
 **                 ��        ·��[@��������=����ֵ] /root/text[@abc=123]
                        --���һ������
 **                  ·������Ϊ���·��,���ֻ��Ϊ��
 **                 ���磺��һ��Ϊ"/root/text",�ڶ���Ϊ"/root/value"
 **                 ���Ա��Ϊ"value"
***************************************************************/
int _xmParseXpath(char *aPath)
{
   char alBuf[10];
   char *clTmp;
   XMLNODE *slNode;
   int ilLen=0,n;
   int ilNum;

   /* ��־״̬*/
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
      /*ȡ·��*/
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
      /*ȡ����*/
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
         /*ȡ��������*/
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
         /*ȡ����ֵ*/
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
 ** ������      :   xmlGet()
 ** ��  ��      :   ȡָ���ڵ�ֵ������ֵ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmParseXpath() 
 ** ȫ�ֱ���    :   sgHuntBuf sgXmlhead
 ** ��������    :   ����: aPath  -- XML·��
 **                       aResult -- ��ѯ���
 **                 ���: pointer -- �������Ի�ڵ�ָ��
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
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
      {  /*ȡ��iNum��ͬ���ƽڵ�*/
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
         else if(strlen(sgHuntBuf.aProp[0]) > 0)       /*ȡ����*/
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

   /*ȡֵ Xpath ��: "/root/txcode[@prop]" */
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
 ** ������      :   xmlGetValue()
 ** ��  ��      :   ȡָ���ڵ�ֵ������ֵ
 ** ��  ��      :   nh
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmParseXpath() 
 ** ȫ�ֱ���    :   sgHuntBuf sgXmlhead
 ** ��������    :   ����: aPath  -- XML·��
 **                       aResult -- ��ѯ���
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
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
      {  /*ȡ��iNum��ͬ���ƽڵ�*/
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
         else if(strlen(sgHuntBuf.aProp[0]) > 0)       /*ȡ����*/
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

   /*ȡֵ Xpath ��: "/root/txcode[@prop]" */
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
 ** ������      :   xmlpackInit()
 ** ��  ��      :   XML����Ԥ���
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:  _xmAppendHead _xmAppendNode 
 **                _xmAppendProp _xmAppendPropValue
 ** ȫ�ֱ���    :   cgpackInit 
 ** ��������    :   ���룺aVersion  -- XML�汾(��"1.0")
 **                       aEncoding -- XML����(��"UTF-8")
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
***************************************************************/
int xmlpackInit(char *aVersion,char *aEncoding)
{
   int ilRc;
   XMLNODE *slNode;
   XMLPROP *slProp;
   if(NULL == aVersion || NULL == aEncoding) return(ERR_INVALID_PARM);
   /*�ж����ݳ����Ƿ񳬳�����*/
   if(strlen(aVersion) > PROPVALUE || strlen(aEncoding) > PROPVALUE)
      return(ERR_VALUE_TOO_LONG);
   if(cgpackInit)  return(ERR_ALREADY_INIT);
   /* ���XMLͷ�ڵ� */
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
 ** ������      :   xmlunpackInit()
 ** ��  ��      :   XML����Ԥ���
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmInitRuleTable()  _xmExpCheck()
 ** ȫ�ֱ���    :   sgXmlhead agMsgbuf igMsgbuflen cgunpackInit
 ** ��������    :   ���룺aMsgbody    -- XML��������
 **                       iMsgbodylen -- aMsgbody����
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
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
   /*�������*/
   _xmInitRuleTable();

   	/* add by lyg 2006-12-18 13:59 */
iMsgbodylen = strlen(aMsgbody);   

/*add by pc �滻����ֻ�ܴ�ͷ֮�����Լ�����ͷλ�õ����*/
aStart=aMsgbody;
aPos=strchr(aMsgbody,'>');
if (aPos!=NULL) {
	iPos=aPos-aStart;
}	
/*add by pc �滻����ֻ�ܴ�ͷ֮�����Լ�����ͷλ�õ����*/
	
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
   		swVdebug(1,"_xmExpCheck У��ʧ��[%d]",ilRc);
   		return(ilRc);
   }
   cgunpackInit = 0x01; 
   return(SUCCESS);
}


/**************************************************************
 ** ������      :   xmlAddNode()
 ** ��  ��      :   XML�����ӽڵ�
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   xmlGet()  _xmAppendNode()  _xmAppendNodeValue()
 ** ȫ�ֱ���    :   
 ** ��������    :   ���룺aXpath -- ·��
 **                       aNode  -- �ڵ�����
 **                       aValue -- �ڵ�ֵ
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
***************************************************************/
int xmlAddNode(char *aXpath,char *aNode,char *aValue)
{
   int ilRc;
   char alBuf[NODEVALUE+1];
   XMLNODE *slParent,*slNode;  
   
   memset(alBuf,0x00,sizeof(alBuf));
   if(NULL == aNode || NULL == aXpath) return(ERR_INVALID_PARM);
   /*�ж�ֵ�����Ƿ񳬳�����*/
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
      /*���ڵ�����*/
      if(slParent->iNodeType != NOD_BRANCH &&
         slParent->iNodeType != NOD_NORMAL)
         return(ERR_INVALID_NOD_TYPE);
   }
   /*����slParent�ڵ����Ƿ���ͬ���ڵ�*/
   /*�����������ͬ���ӽڵ�,��Ҫȡ��ע��*/
/*
   slNode = slParent->sChildNode;
   while(slNode != NULL)
   {
      if(!strcmp(slNode->aNodename,aNode))
        return(ERR_NODE_EXIST);
      slNode = slNode->sBrotherNext;
   }
*/
   /*���ֵΪ�գ������Ҷ�ڵ�*/
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
 ** ������      :   xmlSetNode()
 ** ��  ��      :   ���Ľڵ�ֵ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   xmlGet()  
 ** ȫ�ֱ���    :   sgHuntBuf 
 ** ��������    :   ���룺aXpath -- ·��
 **                       aValue -- �ڵ�ֵ
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
***************************************************************/
int xmlSetNode(char *aXpath,char *aValue)
{
   int ilRc;
   char alBuf[NODEVALUE+1];
   XMLNODE *slNode;
   
   if(NULL == aXpath || NULL == aValue)
      return(ERR_INVALID_PARM);
   /*�ж�ֵ�Ƿ񳬳�����*/
   if(strlen(aXpath) > XPATHLEN || strlen(aValue) > NODEVALUE)
      return(ERR_VALUE_TOO_LONG);

   /* ���ҽڵ� */
   /*ilRc = xmlGet(aXpath,alBuf,(char **)&slNode);*/
   ilRc = xmlGetNode(aXpath,alBuf,(char **)&slNode);
   if(ilRc) return(ilRc);

   /* ���aXpath�� "/root/txcode" �� "/root/txcode[2]" 
      ��"/root/txcode[@prop1=value]"��ʽ���򷵻ص�ָ��
      �ǽڵ�ָ�룬�����"/root/txcode[@prop1]"��ʽ����
���������ص�ָ��������ָ�룬��ʱ������������Ȼ����
   */
   if(sgHuntBuf.iNum == 0 &&
      sgHuntBuf.aProp[0][0] != 0 && sgHuntBuf.aProp[1][0] == 0)
      return(ERR_WRONG_PATH);

   strcpy(slNode->aNodevalue,aValue);
   return(SUCCESS);
}

/**************************************************************
 ** ������      :   xmlAddProp()
 ** ��  ��      :   ��ӽڵ�����
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmAppendProp()  xmlGet()
 ** ȫ�ֱ���    :   sgHuntBuf
 ** ��������    :   ���룺aXpath -- ·��
 **                       aValue -- �ڵ�ֵ
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
 ** ˵  ��      :   aXpath����������"/root/txcode[2]" ��
 **                 "/root/txcode" �� "/root/txcode[@prop=123]"����ʽ
***************************************************************/
int xmlAddProp(char *aXpath,char *aProp,char *aValue)
{
   int ilRc;
   XMLNODE *slNode;
   XMLPROP *slProp;
   char alBuf[NODEVALUE+1];

   if(NULL == aXpath || NULL == aProp || NULL == aValue) 
      return(ERR_INVALID_PARM);
   /*�ж�ֵ�����Ƿ񳬳�����*/
   if(strlen(aProp) > PROPNAME || strlen(aValue) > PROPVALUE ||
      strlen(aXpath) > XPATHLEN)
      return(ERR_VALUE_TOO_LONG);
   
   /*ilRc = xmlGet(aXpath,alBuf,(char **)&slNode);*/
   ilRc = xmlGetNode(aXpath,alBuf,(char **)&slNode);
   if(ilRc) return(ilRc);

   /* ���aXpath�� "/root/txcode" �� "/root/txcode[2]" 
      ��"/root/txcode[@prop1=value]"��ʽ���򷵻ص�ָ��
      �ǽڵ�ָ�룬�����"/root/txcode[@prop1]"��ʽ����
���������ص�ָ��������ָ�룬��ʱ������������Ȼ����
   */
   if(sgHuntBuf.iNum == 0 &&
      sgHuntBuf.aProp[0][0] != 0 && sgHuntBuf.aProp[1][0] ==0)
      return(ERR_WRONG_PATH);
   
   /*���Ҹýڵ����Ƿ��Ѿ����ڸ����� */
   slProp = slNode->sProp;
   while(slProp != NULL)
   {
      if(!strcmp(slProp->aPropname,aProp))
         return(ERR_PROP_EXIST);
      slProp = slProp->spPropnext;
   }
   /*�������*/
   slProp = _xmAppendProp(slNode,aProp);
   if(NULL == slProp) return(igErrData);
   strcpy(slProp->aPropvalue,aValue);
   return(SUCCESS);
}


/**************************************************************
 ** ������      :   xmlSetProp()
 ** ��  ��      :   �޸Ľڵ�����ֵ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   xmlGet()   
 ** ȫ�ֱ���    :   sgHuntBuf
 ** ��������    :   ���룺aXpath -- ·��(�ɰ�������)
 **                       aProp  -- ��������(���ΪNULL����Ҫ������aXpath��)
 **                       aValue -- ����ֵ
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
 ** ˵  ��      :   aXpath����������"/root/txcode[2]" ��
 **                 "/root/txcode" �� "/root/txcode[@prop=123]"����ʽ
***************************************************************/
int xmlSetProp(char *aXpath,char *aProp,char *aValue)
{
   int ilRc;
   XMLPROP *slProp;
   XMLNODE *slNode;
   char alBuf[NODEVALUE+1];
   

   if(NULL == aXpath || NULL == aValue) 
      return(ERR_INVALID_PARM);
   /*�ж�ֵ�����Ƿ񳬳�����*/
   if(strlen(aValue) > PROPVALUE || strlen(aXpath) > XPATHLEN)
      return(ERR_VALUE_TOO_LONG);

   /*ilRc = xmlGet(aXpath,alBuf,(char **)&slNode);*/
   ilRc = xmlGetNode(aXpath,alBuf,(char **)&slNode);
   if(ilRc) return(ilRc);
   /* ���aXpath�� "/root/txcode" �� "/root/txcode[2]" 
      ��"/root/txcode[@prop1=value]"��ʽ���򷵻ص�ָ��
      �ǽڵ�ָ�룬�����"/root/txcode[@prop1]"��ʽ����
���������ص�ָ��������ָ�룬��ʱ������������Ȼ����
   */
   if(sgHuntBuf.iNum == 0 &&
      sgHuntBuf.aProp[0][0] != 0 && sgHuntBuf.aProp[1][0] == 0)
      return(ERR_WRONG_PATH);

   /*����������������Ϊ�գ����Xpath��ȡ, ��ʱҪ��Xpath������
    "/root/txcode[@prop1=123]"�ĸ�ʽ*/
   if(NULL == aProp)
   {
      if(sgHuntBuf.iNum > 0 || sgHuntBuf.aProp[0][0] == 0)
        return(ERR_WRONG_PATH);
      strcpy(alBuf,sgHuntBuf.aProp[0]);
   }
   else 
      strcpy(alBuf,aProp);
   /*�ڵ�ǰ�ڵ�����������*/
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
 ** ������      :   _xmDumpNode()
 ** ��  ��      :   ����xml�ڵ㵽agMsgbuf��
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmCtoA()
 ** ȫ�ֱ���    :   agMsgbuf
 ** ��������    :   ���룺sNode   -- ��Ҫ�����Ľڵ�ָ��
 **                       iOffset -- ��ǰbufferλ��
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
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
      /*Ҷ�ڵ�*/
      if(slNode->iNodeType == NOD_LEAF)
      {
         *(agMsgbuf + ilOffset++) = '/';
         *(agMsgbuf + ilOffset++) = '>';
         if(igDumpFile) *(agMsgbuf + ilOffset++) = '\n'; 
         slNode = slNode->sBrotherNext;
         continue;
      }
      /*ͷ�ڵ�*/
      if(slNode->iNodeType == NOD_HEAD)
      {
         *(agMsgbuf + ilOffset++) = '?';
         *(agMsgbuf + ilOffset++) = '>';
         if(igDumpFile) *(agMsgbuf + ilOffset++) = '\n'; 
         slNode = slNode->sBrotherNext;
         continue;
      }
      /*֦�ڵ�*/
      *(agMsgbuf + ilOffset++) = '>';
      strcpy(alBuf,slNode->aNodevalue);

/*ת��*/
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
 ** ������      :   xmlDump()
 ** ��  ��      :   ����XML��ָ��buffer��
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmDumpNode()  
 ** ȫ�ֱ���    :   sgXmlhead agMsgbuf
 ** ��������    :   ���룺aMsgbuf    -- �洢XML
 **                       iMsgbuflen -- �����aMsgbuf����
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
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
 ** ������      :   xmlDumpFile()
 ** ��  ��      :   ����XML��ָ���ļ���
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   xmlDump()
 ** ȫ�ֱ���    :   igDumpFile
 ** ��������    :   ���룺aFileName -- �ļ���
 ** ����ֵ      :    0 -- �ɹ�
 **                  (������xmldef.h)
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
 ** ������      :   _xmAllocNode()
 ** ��  ��      :   Ԥ�������ڵ���Դ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :   _sgNode _cgNode
 ** ��������    :   (��)
 ** ����ֵ      :   �ڵ�ָ�� -- �ɹ�  NULL -- ʧ��
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
 ** ������      :   _xmFreeProp()
 ** ��  ��      :   �ͷ�������Դ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :   _cgProp
 ** ��������    :   ����:  sProp -- Ҫ�ͷŵ�����ָ��
 ** ����ֵ      :   (��)
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
 ** ������      :   _xmFreeNode()
 ** ��  ��      :   �ͷŽڵ���Դ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmFreeProp()  
 ** ȫ�ֱ���    :   _sgNode cgNode sgXmlhead
 ** ��������    :   ����:  sNode -- Ҫ�ͷŵĽڵ�ָ��
 ** ����ֵ      :   (��)
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
 ** ������      :   _xmAllocProp()
 ** ��  ��      :   Ԥ�������������Դ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :   _cgProp _sgProp
 ** ��������    :   (��)
 ** ����ֵ      :    ����ָ�� -- �ɹ�  NULL -- ʧ��
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
 ** ������      :   xmlFree()
 ** ��  ��      :   �ͷ���Դ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :    sgXmlhead sgHuntBuf _sgProp _sgNode 
 **                  _cgProp _cgNode cgunpackInit cgpackInit
 **                  igErrData igLevel agMsgbuf igMsgbuflen
 ** ��������    :
 ** ��   ��   ֵ:   (��)
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
 ** ������      :   _xmInitRuleTable()
 ** ��  ��      :   ��ʼ��igVarpos��igExppos
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :   agRuleVar,igVarpos,agRuleExp,igExppos
 ** ��������    :   
 ** ����ֵ      :   (��)
***************************************************************/
void _xmInitRuleTable()
{
   int ilKeyCount=0,ilStrCount=0;
   /*��ʼ��igVarpos*/
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
   /*��ʼ��igExppos*/
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
 ** ������      :   _xmExpress()
 ** ��  ��      :   ��aSrc��ȡһ�����ʽ���������aBuf
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������: 
 ** ȫ�ֱ���    :   
 ** ��������    :   ���룺 aSrc -- ���ʽ��
 **                 ����� aBuf -- ���������ӱ��ʽ
 ** ��   ��   ֵ:    0 -- �ɹ�
 **                 -1 -- ʧ��
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
 ** ������      :   _xmVarCheck()
 ** ��  ��      :   ���aMsgbuf�����ַ�����Ӧ��aVar����
 **                 ���﷨�Ƿ���ȷ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmVarRule()  _xmchrchk() _xmspechk()  _xmkeychk()
 ** ȫ�ֱ���    :
 ** ��������    :   ���룺iOffset -- ��Ҫ����agMsgbufλ��
 **                       aVar    -- ������
 ** ����ֵ      :    0 -- �ɹ�
 **                 (������xmldef.h)
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
   /*�Ѿ���������β��ֱ�ӷ���һ��һ�����*/
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

   /*ָ��δ�ƶ�������ƥ��ʧ��*/
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


/*add by pc 2008-11-10 ֧�������ֶ�*/
int isChineseChar( const char ch)
{
	return (ch & 0x80);
}

/*���$����*/
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

/*���@����*/
int _xmspechk(char cTmp)
{
    if(cTmp == 0x0d || cTmp == 0x0a || cTmp == ' ' || cTmp == '\t') return(SUCCESS);
   return(FAIL);
}
/*���!����*/
int _xmkeychk(char cTmp)
{
   if(cTmp == '<' || cTmp == '>' || cTmp == '\'' || cTmp == '\"')
      return(FAIL);
   return(SUCCESS);
}

/**************************************************************
 ** ������      :   _xmVarRule()
 ** ��  ��      :   ȡָ�������Ĺ���
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :   agRuleVar
 ** ��������    :   ���룺aVar  -- ��Ҫȡ����ı���
 **                 �����aRule -- ����ַ
 ** ��   ��   ֵ:    0 -- �ɹ�
 **                  (������xmldef.h)
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
      if(!strcmp(agRuleVar[igVarpos[n]]+1,aVar+1)) /*�ҵ�ƥ��*/
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
 ** ��   ��   ��:   _xmExpRule()
 ** ��        ��:   ȡָ�����ʽ�Ĺ���
 ** ��        ��:
 ** ��������    :
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :   ���룺aExp  -- ��Ҫȡ����ı���
 **                 �����aRule -- ����ַ
 **                       iRuleCount -- ������Ŀ
 ** ��   ��   ֵ:    0 -- �ɹ�
 **                  ERR_NOT_FOUND -- δ�ҵ�ƥ��
 **                  (������xmldef.h)
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
      if(!strcmp(agRuleExp[igExppos[n]]+1,aExp+1)) /*�ҵ�ƥ��*/
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
 ** ������      :   _xmAppendPropValue()
 ** ��  ��      :   �������ֵ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   
 ** ȫ�ֱ���    :   
 ** ��������    :   ���룺aName -- ��������
 **                       sProp -- ����ָ�� 
 ** ��   ��   ֵ:    0 -- �ɹ�
 **                 -1 -- ʧ��
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
 ** ������      :   _xmAppendProp()
 ** ��  ��      :   �������
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmAllocProp()
 ** ȫ�ֱ���    :   sgXmlhead
 ** ��������    :   ���룺aName  -- ��������
 **                       sOwner -- ӵ�����ԵĽڵ�ָ�� 
 ** ��   ��   ֵ:    0 -- �ɹ�
 **                 -1 -- ʧ��
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
 ** ������      :   _xmAppendNodeValue()
 ** ��  ��      :   ��ӽڵ�ֵ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   xmlAtoC() _xmTrim()
 ** ȫ�ֱ���    :   
 ** ��������    :   ���룺aValue  -- �ڵ�ֵ
 **                       sNode   -- �ڵ�ָ��
 ** ����ֵ      :    0 -- �ɹ�
 **                 -1 -- ʧ��
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
   xmlAtoC(aValue);  /*��ת��*/  
   strcpy(sNode->aNodevalue,aValue);
   return(SUCCESS);
}


/**************************************************************
 ** ��   ��   ��:   _xmAppendNode()
 ** ��        ��:   ��ӽڵ�
 ** ��        ��:
 ** ��������    :
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :   ���룺 sParent ���ڵ�
 **                        aName   -- �ڵ�����
 **                        iType   -- �ڵ�����
 ** ��   ��   ֵ:    0 -- �ɹ�
 **                 -1 -- ʧ��
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
      /*�洢�ּ��ڵ�ָ��*/
      sgXmlhead.sNodeLevel[igLevel] = slNode;
      /*�洢�ּ��ڵ���Ŀ*/
      sgXmlhead.iNLevelCount[igLevel] += 1;
      /*�洢�ڵ����*/
      if(sgXmlhead.iNodeLevel < igLevel)
         sgXmlhead.iNodeLevel = igLevel;
   }
   return(slNode);
}


/**************************************************************
 ** ������      :   _xmAppendHead()
 ** ��  ��      :   ���ͷ�ڵ�
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmAllocNode() 
 ** ȫ�ֱ���    :   sgXmlhead
 ** ��������    :   ���룺aName -- �ڵ�����
 ** ����ֵ      :   ��ӵĽڵ�ָ�� -- �ɹ� NULL -- ʧ��
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
 ** ������      :   _xmStackCheck()
 ** ��  ��      :   ���ڵ�ǰ�������Ƿ���ͬ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :   ���룺aMacro -- ����꼰��Ӧ�ڵ����Ƶ�����
                          aValue -- Ҫ���Ľڵ�����
 ** ��   ��   ֵ:    0 -- �ɹ�
 **                 -1 -- ʧ��
 ** ˵        ��:   aMacro�洢��һ�������Ľڵ��ֵ
 **                 �ٴ������ڵ�ʱ���ڵ�ֵ�����е�ֵ�Ƚ�
 **                 �����ͬ��˵���ڵ�������������
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
 ** ������      :   _xmExpCheck()
 ** ��  ��      :   ���agMsgbuf�����ַ�����Ӧ�ڱ��ʽaExp
 **                 ���﷨�Ƿ���ȷ
 ** ��  ��      :   cgx
 ** ��������    :   2004-01-08
 ** ����޸�����:   2004-01-14
 ** ������������:   _xmExpRule()     _xmExpress()     _xmVarCheck()
 **                 _xmStackCheck()  _xmAppendNode()  _xmAppendHead()
 **                 _xmAppendProp()  _xmAppendNodeValue() 
 **                 _xmAppendPropValue()  _xmExpCheck()  _xmFreeNode()
 ** ȫ�ֱ���    :   agMsgbuf igMsgbuflen  igErrData
 ** ��������    :   ���룺iOffset -- ��Ҫ����agMsgbufλ��
 **                       sParent -- ���ʽ�����Ľڵ�ָ��
 **                       aExp    -- ���ʽ
 ** ����ֵ      :    0 -- �ɹ�
 **                 (������xmldef.h)
***************************************************************/
int _xmExpCheck(XMLNODE *sParent,char *aExp,int *iOffset)
{
   int ilOffset=0,ilOld=0;
   int m,n,ilRc;
   char alBuf[EXPLEN+1];               /* �洢�ӱ��ʽ */
   char alValue[NODEVALUE+1];          /* �洢�ڵ�����*/
   int ilBuflen;
   char alMacro[NODENAME+1];           /* ��ջ ��¼��һ���ڵ�����*/
   char alRule[5][EXPLEN+1];           /* ȡ������ */
   int ilRulelen;
   int ilRuleCount;

   XMLNODE *slNode;
   XMLPROP *slProp;

   /*������*/
   if(NULL == aExp || *iOffset < 0)
   {
#ifdef _XML_DEBUG
      printf("[_xmExpCheck] %s %d\n",aExp,*iOffset);
#endif
      return(ERR_INVALID_PARM);
   }
   /*��������β��ֱ�ӷ���һ��һ�����*/
   if(*iOffset >= igMsgbuflen)
   {
      igErrData = igMsgbuflen;
#ifdef _XML_DEBUG
      printf("ExpCheck [%d] \n",igMsgbuflen);
#endif
      return(ERR_INVALID_CHAR);
   }

   slNode = NULL;

   /*��ʼ����ջ*/
   alMacro[0] = 0x00;

   /*ȡ����*/
   ilRc = _xmExpRule(aExp,alRule,&ilRuleCount);
   if(ilRc)
      return (ERR_INVALID_RULE);
   m = 0;
   ilOld = *iOffset;

RULECHECK:
   ilRulelen = strlen(alRule[m]);
   for(n=0;n<ilRulelen;n++)
   {  	
      ilRc = _xmExpress(alRule[m]+n,alBuf);   /*ȡ�����еı��ʽ*/
      if(ilRc) return(ERR_INVALID_RULE);
      ilBuflen = strlen(alBuf);
      if(alBuf[ilBuflen-1] == '*') alBuf[--ilBuflen] = 0x00;
      n = n + ilBuflen - 1;
      if(alBuf[1] == 'V')   /*����*/
      {      	
         ilOffset = *iOffset;
         ilRc = _xmVarCheck(alBuf,iOffset);
         /*ƥ��ʧ�� ���������� */
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
         /*ƥ��ɹ�*/
         if(alRule[m][n+1] == '*')
         {
            n -= ilBuflen;
/*            continue;*/
         }
         /*�ж�ֵ�����Ƿ񳬳�����*/
         if((*iOffset-ilOffset) > NODEVALUE) 
         {
            igErrData = ilOffset;
            return(ERR_VALUE_TOO_LONG);
         }
         /*ȡ��ֵ*/
         memcpy(alValue,agMsgbuf+ilOffset,*iOffset-ilOffset);
         alValue[*iOffset-ilOffset] = 0x00; 
         /*�ǽڵ�����****************/
         if(!strcmp(alBuf,"&V_NN")) 
         {        	
            /*�ж�ֵ�����Ƿ񳬳�����*/
            if(strlen(alValue) > NODENAME) 
            {
               igErrData = ilOffset;
               return(ERR_VALUE_TOO_LONG);
            }             
            /*�Ƚ���һ��ֵ*/
            ilRc = _xmStackCheck(alMacro,alValue);
            if(ilRc == ERR_NOT_FOUND)  /*δ����֮ǰ��ͬ���ڵ�*/
            {
               slNode = _xmAppendNode(sParent,alValue,m+1);  /*��ӵ�����*/
         
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

         /*ͷ�ڵ�*/
         else if(!strcmp(alBuf,"&V_KEY"))
         {         	
            /*�ж�ֵ�����Ƿ񳬳�����*/
            if(strlen(alValue) > NODENAME) 
            {
               igErrData = ilOffset;
               return(ERR_VALUE_TOO_LONG);
            }
            slNode = _xmAppendHead(alValue);
            if(NULL == slNode) return(igErrData);
         }

         /*��������****************/
         else if(!strcmp(alBuf,"&V_PN"))
         {         	
            /*�ж�ֵ�����Ƿ񳬳�����*/
            if(strlen(alValue) > NODENAME) 
            {
               igErrData = ilOffset;
               return(ERR_VALUE_TOO_LONG);
            }
            /*�������*/
            slProp = _xmAppendProp(sParent,alValue);
            if(NULL == slProp) return(igErrData);
         }

         /*�ڵ�ֵ****************/
         else if(!strcmp(alBuf,"&V_NV"))
         {         	
            /*��ӽڵ�ֵ*/
            ilRc = _xmAppendNodeValue(slNode,alValue);
            if(ilRc) return(ilRc);
         }

         /*����ֵ****************/
         else if(!strcmp(alBuf,"&V_PV"))
         {          	
            /*�������ֵ*/
            ilRc = _xmAppendPropValue(slProp,alValue);
            if(ilRc) return(ilRc);
         }
         else
            continue;

      }  /*end if 'V' */
      else if(alBuf[1] == 'E')
      {     	
         ilOffset = *iOffset;
         igLevel += 1;  /*��ȵ���*/
         if(igLevel > XMLMAXLEVEL) return(ERR_DEPTH_TOO_BIG);
         ilRc = _xmExpCheck(slNode,alBuf,iOffset);      
         igLevel -= 1;  /*��ȵݼ�*/
         /*ƥ��ʧ�� �� ��������*/
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
         /*ƥ��ɹ�*/
         if(alRule[m][n+1] == '*')
         {         	
            n -= ilBuflen;
            continue;
         }
      } /* end if 'E' */
      else
         return(ERR_INVALID_RULE);
   }  /*end for*/

/*������ִ�е�����ʱ˵����������ƥ��ɹ�*/
   return(SUCCESS);

ERRCHK:
   /*��ƥ��ʧ�ܶ����ұ��ʽ����ʱ�������������Ƿ�����������
     ���û�У��ͷ��ش���
     ����ж�������ҵ�ǰ������ɹ�ʱ�����������ټ��*/
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
 ** ������      �� _swParsexml
 ** ��  ��      �� ����XPATH
 ** ��  ��      �� nh
 ** ��������    �� 2002/8/20
 ** ����޸����ڣ� 2002/8/28
 ** ��������������
 ** ȫ�ֱ���    �� 
 ** ��������    �� aParm[0] : XPATH
 									 aPath: ��ǰ·��PATH�Ľڵ���
 									 iNum: ��i��ͬ���ڵ�
 ** ����ֵ      �� 0-SUCCESS -1-FAIL
***************************************************************/
int _swParsexml(char *aXpath,char *aPath,short *iNum)
{
  int i=0,j;
  char alTmp[101];
  char alBuf[101];
  
  if(aXpath[0]=='\0')
  { 
#ifdef _XML_DEBUG  	
    printf("S0450: _swParsexml() �����XPATHΪ��!\n");
#endif
    aXpath[0]=0;
    return(SUCCESS);
  }
  if(aXpath[0] != '/')
  {
#ifdef _XML_DEBUG  	
    printf("S0460: _swParsexml() �������XPATH����ȷ,����!\n");
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
    /* �ҳ����·����ֵ */
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
 ** ������      �� xmlGetNode
 ** ��  ��      �� ȡxml�ڵ������ֵ
 ** ��  ��      �� nh
 ** ��������    �� 2002/8/20
 ** ����޸����ڣ� 2002/8/28
 ** ��������������
 ** ȫ�ֱ���    �� 
 ** ��������    �� aParm[0] : XPATH
 									 aResult: ȡ�õĽڵ�ֵ 
 									 pointer: �ڵ�ָ��
 ** ����ֵ      �� 0-SUCCESS -1-FAIL
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
    printf("S0200: ��������_swParsexml()����\n");
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
 ** ������      �� xmlGetProp
 ** ��  ��      �� ȡxml�ڵ���������Ե�ֵ
 ** ��  ��      �� nh
 ** ��������    �� 2002/8/20
 ** ����޸����ڣ� 2002/8/28
 ** ��������������
 ** ȫ�ֱ���    �� 
 ** ��������    �� aParm[0] : XPATH
 									 aName: ��������
 									 aResult:  ����ֵ
 ** ����ֵ      �� 0-SUCCESS -1-FAIL
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
  	printf("��λ�ڵ����,����XPATH�Ƿ���ȷ\n");
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
      printf("���ļ�ʧ��!\n");
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
   /* ��Ӹ� �ڵ� */
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
   
   /* ��ӽڵ� */
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
   
   /* ��ӽڵ� */
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
   
      /*����ļ�����*/
   printf("*** DUMP file TEST ***\n");
   ilRc = xmlDumpFile("./output1.xml");
   if(ilRc)
   {
      printf("dump file error [%s]\n",xmlDescription(ilRc));
      exit(0);
   }
   
      
   /*���buffer����*/
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

