#include "switch.h"
#include "swSyscom.h"
#include "swNdbstruct.h"
#include "swShm.h"

#define MAXPACKLEN 8192
#define XMLHEAD "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>"
#define LOCKFILE "lockfile"
#define MAXFMTGROUP 30000

static short igTimeoutFlag = 0;

int GetMode3FileName(pDataset psDataset,keyrec   *pKeys,\
                                        short     iKeyNum,\
                                        char     *aFileName,\
                                        char     *aGname,\
                                        char     *aDname);
short swShmselect_swt_sys_task_all_p(struct swt_sys_task **psSwt_sys_task, 
        short *piCount);
int swShmselect_swt_sys_queue_all(struct swt_sys_queue * psSwt_sys_queue,
  short *iCount);
int swShmcheck();
int Insert(pRecord pRec,pDataset pSet);
int mbqOpen(short iMbid); 
int bmqGetmbinfo(short iMbid,struct mbinfo *plMbinfo);
int bmqClose();            
int swTcpPut(int sock,char *aMsgpack,unsigned int iMsglen);                             
int swTcpGet(int sock,char *aMsgpack,unsigned int *piMsglen,short iTimeout);

void swDebugset(short iFlag,pDataset pSet)
{
  /* short i,j; */
  return;
/*
  for(i = 0;i < pSet->iRecordNum;i++)
  {
    for(j =0;j < pSet->iFieldNum;j++)
    {
      swVdebug(1,"S0020: 记录名称[%s],记录值[%s]",
        pSet->Fields[j].aFieldName,
        pSet->Records[i].Values[j].aValue);
    }
  }
*/
}

void swDebugkeys(short iFlag,keyrec *pKeys,short iKeyNum)
{
  short i;

  swVdebug(iFlag,"S0030: ---------------子进程[%d]/关键字数[%d]------------",
                          getpid(),iKeyNum);
  for(i = 0;i < iKeyNum;i++)
  {
    swVdebug(iFlag,"S0040:      %s = %s          ",
          pKeys[i].keyname,pKeys[i].keyval);
  }
  swVdebug(iFlag,"S0050: ------------------关键字结束----------------");
}

void swDebugrecs(short iFlag,pRecord pRec,short iRecNum)
{
  short i,j;

  for(i = 0;i < iRecNum;i++)
  {
    swVdebug(1,"S0060: **************第%d条记录***************",i+1);
    for(j = 0;j < MAXFLDNUM;j++)
    {
      if(pRec[i].Values[j].aValue[0] == 0)continue;
      swVdebug(1,"S0070:         %s",pRec[i].Values[j].aValue);
    }
    swVdebug(1,"S0080: **************************************");
  }
  swVdebug(1,"S0090: -----------------记录结束-------------------");
  swVdebug(iFlag,"\n");
}

void swDebugnode(short iFlag,pNode pNd)
{
  short i;

  swVdebug(iFlag,"S0100: ^^^^^^^节点%s,%d个属性,%d个子节点^^^^^^^",
            pNd->aNodeName,pNd->iAttrNum,pNd->iChildNum);
  swVdebug(iFlag,"S0110: -----------------属性------------------");
  for(i = 0;i < pNd->iAttrNum;i++)
  {
    swVdebug(iFlag,"S0120:     %s = %s",
            pNd->Attribs[i].aAttribName,
            pNd->Attribs[i].aAttribVal);
  }
  for(i = 0;i < pNd->iChildNum; i++)
    swDebugnode(iFlag,&((pNode)pNd->ChildNodes)[i]);
  swVdebug(iFlag,"S0130: ^^^^^^^^^^^^^^^节点%s结束^^^^^^^^^^^^^^^^^",
         pNd->aNodeName);
}

/***************************************************/
/*  　　             XML函数　　   　　　　　　　　　　*/
/***************************************************/
/**************************************************
&、<、>的转义
***************************************************/
int swXmltrans(char *aXmlvalue,char *aBuf2)
{
  char alBuftmp[iMSGMAXLEN+1];
  int  i,j=0;
  memset(alBuftmp,0x00,sizeof(alBuftmp));
  for (i = 0;i < strlen(aXmlvalue);i++)
  {
    switch (aXmlvalue[i])
    {
    case '&':
      strcat(alBuftmp,"&amp;");
      j += 5;
      break;
    case '<':
      strcat(alBuftmp,"&lt;");
      j += 4;
      break;
    case '>':
      strcat(alBuftmp,"&gt;");
      j += 4;
      break;
    case '\"':
      strcat(alBuftmp,"&quot;");
      j+=6;
      break;
    case '\'':
      strcat(alBuftmp,"&apos;");
      j+=6;
      break;
    default:
      alBuftmp[j] = aXmlvalue[i];
      j++;                       
    }
  }
  strcpy(aBuf2,alBuftmp);
  return 0;
}

/*********************************************
&amp、&lt、&gt转义
**********************************************/
int swXmlback(char *aBuf,char *aBuf_rtn)
{
  char alBuffer[iMSGMAXLEN];
  char alTmpbuf[20];
  int  i,j = 0,k,ilTmp = 0,ilFlag = 0;
  
  memset(alTmpbuf,0x00,sizeof(alTmpbuf));
  memset(alBuffer,0x00,sizeof(alBuffer));
  ilTmp = 0;
  for (i =0;aBuf[i];i++)
  {
    if (aBuf[i] != '&')
    {
      if (ilTmp == 0)
      {
        alBuffer[j] = aBuf[i];
        j ++;
      }
      else
      {
        if (aBuf[i] == ';')
        {
          memset(alTmpbuf,0x00,sizeof(alTmpbuf));
          strncpy(alTmpbuf,aBuf + k + 1,i - k - 1);     
          if (strcmp(alTmpbuf,"amp") == 0)
            alBuffer[j]  = '&';
          else if (strcmp(alTmpbuf,"lt") == 0)
            alBuffer[j]  = '<';
          else if (strcmp(alTmpbuf,"gt") == 0)
            alBuffer[j]  = '>';
          else if (strcmp(alTmpbuf,"quos") == 0)
            alBuffer[j]  = '\"';
          else if (strcmp(alTmpbuf,"apos") == 0)
            alBuffer[j]  = '\'';
          j ++;
          ilTmp = 0;
         }
      }
    }
    else
    {
      ilTmp = 1;
      ilFlag = 1;
      k = i;
    }
  }
  if (ilTmp == 1)
  {
    return 134;
  }
  strcpy(aBuf_rtn,alBuffer);
  return 0;
}

int InitNode(pNode psNode)
{
  psNode->iChildNum = 0;
  psNode->iAttrNum = 0;
  return(0);
}

int DestroyNode(pNode psNode)
{
  int i;
  for(i = 0;i < psNode->iChildNum;i++)
  {
    DestroyNode(&((pNode)psNode->ChildNodes)[i]);
  }
  if(psNode->iAttrNum > 0)
    free(psNode->Attribs);
  if(psNode->iChildNum > 0)
    free(psNode->ChildNodes);
  psNode->iAttrNum = 0;
  psNode->iChildNum = 0;
  return(0);
}

/*************************************************
取一个元素
aXmlBuf : XML报文 (in)
iLen    : 报文长度(in)
piFlag  : (out)
			1---元素开始 <name>
			2---元素结束 </name>
			3---         <name/>
ppAttr	: 属性
piAttrNum : 属性数目
piPos	: 出错位置
**************************************************/
int GetElement(char *aXmlBuf,int iLen,int *piFlag,pNode pnode,int *piPos)
{
	short ilPos,ilPos1,ilPos2;
	char alElementName[30],alElementVal[300];
	char alAttrName[30],alAttrVal[300],alTmpBuf[400];
	short i,ilFlag,ilZpos;
        char alZbuf[10];
	
	pnode->iAttrNum = 0;
	*piFlag = -1;

	for(ilPos = 0;(((aXmlBuf[ilPos] == ' ')  ||
					(aXmlBuf[ilPos] == '\t')  ||
					(aXmlBuf[ilPos] == 0xa)	  ||
					(aXmlBuf[ilPos] == 0xd))  &&
					(ilPos < iLen));
					ilPos++);
	
	if(ilPos == iLen)
	{
		*piPos = 0;
		return(143);
	}
	
	if(aXmlBuf[ilPos] != '<')
	{
		*piPos = ilPos;
		return(141);
	}
	
	for(ilPos1 = ilPos;((aXmlBuf[ilPos1] != '>') &&
						(ilPos1 < iLen));ilPos1++);
						
	if(ilPos1 == iLen)
	{
		*piPos = iLen;
		return(142);
	}
	
	if(aXmlBuf[ilPos+1] == '/')
	{
		*piFlag = 3;
		ilPos++;
	}
	
	i = 0;
	alElementName[i] = '\0';
        ilZpos = 0;
        alZbuf[ilZpos] = 0;
	for(ilPos2 = ilPos+1;((aXmlBuf[ilPos2] != ' ') &&
		(!((aXmlBuf[ilPos2] == '/')&&(aXmlBuf[ilPos2+1] == '>'))) &&
                          (aXmlBuf[ilPos2] != '>') &&
						(aXmlBuf[ilPos2] != '\t')  &&
						(aXmlBuf[ilPos2] != 0xa)   &&
						(aXmlBuf[ilPos2] != 0xd)   &&
						(ilPos2 < ilPos1));ilPos2++)
	{
		if(aXmlBuf[ilPos2] == '<')
		{
			*piPos = ilPos2;
			return(145);
		}
                if((aXmlBuf[ilPos2] == '&') || (ilZpos > 0))
                {
                   alZbuf[ilZpos] = aXmlBuf[ilPos2];
                   ilZpos++;
                   alZbuf[ilZpos] = 0;
                   if(strcmp(alZbuf,"&amp;") == 0)
                   {
                     alElementName[i] = '&';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&lt;") == 0)
                   {
                     alElementName[i] = '<';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&gt;") == 0)
                   {
                     alElementName[i] = '>';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&quot;") == 0)
                   {
                     alElementName[i] = '\"';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&apos;") == 0)
                   {
                     alElementName[i] = '\'';
                     i++;
                     ilZpos = 0;
                   }
                   if(ilZpos > 5)
                   {
                     *piPos = i+1;
                     return(-1);
                   }
                   continue;
                } 
		alElementName[i] = aXmlBuf[ilPos2];
		i++;
	}
	if(i == 0)
	{
		*piPos = ilPos2;
		return(144);
	}
	alElementName[i] = '\0';
	strcpy(pnode->aNodeName,alElementName);
	if((aXmlBuf[ilPos2] == '/')&&(aXmlBuf[ilPos2+1] == '>'))
	{
		if(*piFlag == 3)
		{
			*piPos = ilPos2;
			return(146);
		}
		*piFlag = 2;
		*piPos = ilPos1+1;
		return(0);
	}
	if(*piFlag == 3)
	{
		for(;((aXmlBuf[ilPos2] == ' ')  ||
		 	 (aXmlBuf[ilPos2] == '\t')  ||
		 	 (aXmlBuf[ilPos2] == 0xa)   ||
		 	 (aXmlBuf[ilPos2] == 0xd));
		 	 ilPos2++);
		 if(aXmlBuf[ilPos2] == '>')
		 {
			*piPos = ilPos2 + 1;
		   return(0);
		 }
		 else
                 {
		   return(147);
                 }
	}
	
	while(ilPos2 < ilPos1)
	{
		alAttrVal[0] = 0;
		alAttrName[0] = 0;
		for(;((aXmlBuf[ilPos2] == ' ')  ||
		 	 (aXmlBuf[ilPos2] == '\t')  ||
		 	 (aXmlBuf[ilPos2] == 0xa)   ||
		 	 (aXmlBuf[ilPos2] == 0xd));
		 	 ilPos2++);
		i = 0;
		alTmpBuf[i] = '\0'; 
		ilFlag = 0;
                ilZpos = 0;
                alZbuf[ilZpos] = 0;
		for(;((aXmlBuf[ilPos2] != ' ')  &&
			  (aXmlBuf[ilPos2] != '=')	&&
	(!((aXmlBuf[ilPos2] == '/')&&(aXmlBuf[ilPos2+1] == '>')))  &&
			  (aXmlBuf[ilPos2] != '\t') && 
		 	  (aXmlBuf[ilPos2] != 0xa)  &&
		 	  (aXmlBuf[ilPos2] != 0xd)  &&
			  (ilPos2 < ilPos1));ilPos2++)
		{
			if(aXmlBuf[ilPos2] == '<')
			{
				*piPos = ilPos2;
				return(145);
			}
                if((aXmlBuf[ilPos2] == '&') || (ilZpos > 0))
                {
                   alZbuf[ilZpos] = aXmlBuf[ilPos2];
                   ilZpos++;
                   alZbuf[ilZpos] = 0;
                   if(strcmp(alZbuf,"&amp;") == 0)
                   {
                     alTmpBuf[i] = '&';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&lt;") == 0)
                   {
                     alTmpBuf[i] = '<';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&gt;") == 0)
                   {
                     alTmpBuf[i] = '>';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&quot;") == 0)
                   {
                     alTmpBuf[i] = '\"';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&apos;") == 0)
                   {
                     alTmpBuf[i] = '\'';
                     i++;
                     ilZpos = 0;
                   }
                   if(ilZpos > 5)
                   {
                     *piPos = i+1;
                     return(140);
                   }
                   continue;
                } 
			alTmpBuf[i] = aXmlBuf[ilPos2];
			i++;
		}
	if((aXmlBuf[ilPos2] == '/')&&(aXmlBuf[ilPos2+1] == '>'))
		{
			*piFlag = 2;
			break;
		}
		alTmpBuf[i] = 0;
		strcpy(alAttrName,alTmpBuf);
               
		for(;((aXmlBuf[ilPos2] != '=') && 
			  (ilPos2 < ilPos1));ilPos2++);
		if(ilPos2 == ilPos1) 
		{
			if(strcmp(alAttrName,"") == 0)
			{
				continue;
			}
			else
			{
				return(148);
			}
		}
		for(;((aXmlBuf[ilPos2] != '"') &&
			  (ilPos2 < ilPos1));ilPos2++);
		if(ilPos2 == ilPos1)
		{
			return(148);
		}
		ilPos2++;
		i = 0;
                ilZpos = 0;
                alZbuf[ilZpos] = 0; 
		for(;((aXmlBuf[ilPos2] != '"') &&
			  (ilPos2 < ilPos1));ilPos2++)
		{
                   if((aXmlBuf[ilPos2] == '&') || (ilZpos > 0))
                   {
                     alZbuf[ilZpos] = aXmlBuf[ilPos2];
                     ilZpos++;
                     alZbuf[ilZpos] = 0;
                     if(aXmlBuf[ilPos2] == ';') 
                     {
                       if(strcmp(alZbuf,"&amp;") == 0)
                       {
                         alAttrVal[i] = '&';
                         i++;
                         ilZpos = 0;
                       } 
                       if(strcmp(alZbuf,"&lt;") == 0)
                       {
                         alAttrVal[i] = '<';
                         i++;
                         ilZpos = 0;
                       }
                       if(strcmp(alZbuf,"&gt;") == 0)
                       {
                         alAttrVal[i] = '>';
                         i++;
                         ilZpos = 0;
                       }
                       if(strcmp(alZbuf,"&quot;") == 0)
                       {
                         alAttrVal[i] = '\"';
                         i++;
                         ilZpos = 0;
                       }
                       if(strcmp(alZbuf,"&apos;") == 0)
                       {
                         alAttrVal[i] = '\'';
                         i++;
                         ilZpos = 0;
                       }
                       if(ilZpos > 5)
                       {
                         *piPos = i+1;
                         return(140);
                       } 
                     }
                     continue;
                   }
			alAttrVal[i] = aXmlBuf[ilPos2];
			i++;
		}
		if(ilPos2 == ilPos1)
                {
		  return(148);
                }

		alAttrVal[i] = 0;

		pnode->iAttrNum++;
		if(pnode->iAttrNum == 1)
		  pnode->Attribs = (pAttrib)malloc(sizeof(Attrib));
		else
                  pnode->Attribs = (pAttrib)realloc(pnode->Attribs,
                      pnode->iAttrNum * sizeof(Attrib));
		
	strcpy(pnode->Attribs[pnode->iAttrNum-1].aAttribName,alAttrName);
		strcpy(pnode->Attribs[pnode->iAttrNum-1].aAttribVal,alAttrVal);
		ilPos2++;
		if(ilPos2 == ilPos1)
			break;
	}
	if(*piFlag == 2)
	{
		*piPos = ilPos1+1;
	}
	if(*piFlag == -1)
	{
		*piFlag = 1;
		ilPos2++;
		i = 0;
		alElementVal[i] = '\0';
                ilZpos = 0;
                alZbuf[ilZpos] = 0; 
		for(;(aXmlBuf[ilPos2] != '<')
                           /* &&
			 (ilPos2 < iLen) &&
			 (aXmlBuf[ilPos2] != ' ') &&
			 (aXmlBuf[ilPos2] != '\t') &&
			 (aXmlBuf[ilPos2] != 0xa) &&
			 (aXmlBuf[ilPos2] != 0xd))*/;ilPos2++)
		{
                  if((aXmlBuf[ilPos2] == '&') || (ilZpos > 0))
                  {
                   alZbuf[ilZpos] = aXmlBuf[ilPos2];
                   ilZpos++;
                   alZbuf[ilZpos] = 0;
                   if(strcmp(alZbuf,"&amp;") == 0)
                   {
                     alElementVal[i] = '&';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&lt;") == 0)
                   {
                     alElementVal[i] = '<';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&gt;") == 0)
                   {
                     alElementVal[i] = '>';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&quot;") == 0)
                   {
                     alElementVal[i] = '\"';
                     i++;
                     ilZpos = 0;
                   }
                   else if(strcmp(alZbuf,"&apos;") == 0)
                   {
                     alElementVal[i] = '\'';
                     i++;
                     ilZpos = 0;
                   }
                   if(ilZpos > 5)
                   {
                     *piPos = i+1;
                     return(140);
                   }
                    continue;
                  }
                  alElementVal[i] = aXmlBuf[ilPos2];
                  i++;
		}
		*piPos = ilPos2;
		alElementVal[i] = '\0';
		strcpy(pnode->aNodeVal,alElementVal);
	}	
	return(0);
}

int ReadXmlFile(char *aFileName,char **paBuf,int *piLen)
{
  FILE *fp;
  char *alBuf,alBufT[2048];
  size_t len,len1;

  fp = fopen(aFileName,"r");
  if(fp == NULL)
  {
    swVdebug(2,"S0140: [函数调用/ReadXmlFile] 打开XML文件出错.");
    return(345);
  }
  len = 0;
  while(TRUE)
  {
    swVdebug(2,"ddddddddddddddddd");
    if(feof(fp))break;
    len1 = fread(alBufT,1,2048,fp);
    swVdebug(2,"%d",len1);
    if(len1 <= 0)break;
    if(len == 0)
    {
      alBuf = (char*)malloc(len1 + 10);
    }
    else
    {
      alBuf = realloc(alBuf,len + len1 + 10 );
    }
    memcpy(&alBuf[len],alBufT,len1);
    len += len1;
  } 
  fclose(fp);
  if(len == 0)
  {
    swVdebug(3,"S0150: [函数调用/ReadXmlFile] 错误返回！文件长度为0.");
    return(456);
  }
  *paBuf = alBuf;
  *piLen = len;
  swVdebug(3,"Len = %d",len);
  swVdebug(3,"S0160: [函数调用/ReadXmlFile] 成功返回！");
  return 0;
}

int ParseXml(char * aXmlBuf,unsigned int iLen,pNode pRootNode,int *piPos)
{
  int ilPos = 0,ilFlag/*,ilAttrNum*/,ilRc,ilLen;
  char *p;
  short ilElementFlag = 0;
  Node TmpNode;
  pNode pslNode;
	
  pRootNode->iChildNum = 0;
	
  p = aXmlBuf;
  ilLen = iLen;
  (*piPos) = 0;
  while(1)
  {
    ilRc = GetElement(p,ilLen,&ilFlag,&TmpNode,&ilPos);
    if(ilRc)
      return(ilRc);
		
      switch(ilFlag)
      {
        case 1:
          if(ilElementFlag == 0)
          {
            ilElementFlag = 1;
            strcpy(pRootNode->aNodeName,TmpNode.aNodeName);
            strcpy(pRootNode->aNodeVal,TmpNode.aNodeVal);
            pRootNode->iAttrNum = TmpNode.iAttrNum;
            pRootNode->Attribs = TmpNode.Attribs;
            p += ilPos;
            ilLen = ilLen - ilPos + 1;
            (*piPos) += ilPos;
          }
          else
          {
            pRootNode->iChildNum++;
            if(pRootNode->iChildNum == 1)
              pRootNode->ChildNodes = (char*)malloc(sizeof(Node));
            else
              pRootNode->ChildNodes = (char*)realloc(pRootNode->ChildNodes,
                                 pRootNode->iChildNum * sizeof(Node));

           pslNode = &((pNode)pRootNode->ChildNodes)[pRootNode->iChildNum-1];
           pslNode->aNodeName[0] = 0;
           pslNode->aNodeVal[0] = 0;
           pslNode->iAttrNum = 0;
           pslNode->iChildNum = 0;
           ilRc = ParseXml(p,ilLen,pslNode,&ilPos);
           if(ilRc)
           {
             return(ilRc);
           }
           (*piPos) += ilPos;
           p += ilPos;
           ilLen = ilLen - ilPos + 1;
         }
         break;
       case 2:
	(*piPos) += ilPos;
	p += ilPos;
	ilLen = ilLen - ilPos + 1;
	if(ilElementFlag == 0)
	{
          ilElementFlag = 1;
	  strcpy(pRootNode->aNodeName,TmpNode.aNodeName);
          strcpy(pRootNode->aNodeVal,TmpNode.aNodeVal);
	  pRootNode->iAttrNum = TmpNode.iAttrNum;
	  pRootNode->Attribs = TmpNode.Attribs;
		
          return(0);
	}
	else
	{
          pRootNode->iChildNum++;
          if(pRootNode->iChildNum == 1)
            pRootNode->ChildNodes = (char*)malloc(sizeof(Node));
          else
            pRootNode->ChildNodes = (char *)realloc(pRootNode->ChildNodes,
                    pRootNode->iChildNum * sizeof(Node));
					
          pslNode = &((pNode)(pRootNode->ChildNodes))[pRootNode->iChildNum-1];
          memcpy(pslNode,&TmpNode,sizeof(Node));
          pslNode->iChildNum = 0;
        }
	break;
      case 3:
        (*piPos) += ilPos;
        if((ilElementFlag == 0) && 
                        (strcmp(TmpNode.aNodeName,pRootNode->aNodeName) != 0))
          return(149);
        return(0);
      default:
        return 150;
     }
  }
}

int ParseXmlFile(char * aFileName,pNode pRootNode,int *piPos)
{
  int ilRc,ilLen;
  char *alBuf;

  ilRc = ReadXmlFile(aFileName,&alBuf,&ilLen);
  if(ilRc)
    return ilRc;
  alBuf[ilLen] = 0;

  ilRc = ParseXml(alBuf,ilLen,pRootNode,piPos); 
  free(alBuf);
  if(ilRc)
  {
    swVdebug(2,"S0180: 解析XML文件[%s]出错,返回[%d],位置[%d]",aFileName,ilRc,*piPos);
    return ilRc;
  }
  swVdebug(3,"S0190: 解析XML文件[%s]成功.",aFileName);
  return(ilRc);
}

int WriteNodeToFile(FILE *fp,pNode psNode,short iSj)
{
  char alBuf[2048],alTmp[512],alTmp1[250],alTmp2[1024];
  short i;
  pNode pslNode;
  
  /*写入节点名*/
  alBuf[0] = 0;
  alTmp1[0] = 0;
  if(psNode->aNodeName[0] == 0)return(0);
  for(i = 0;i < iSj;i++)
    strcat(alTmp1," ");
  strcat(alBuf,alTmp1);
  swXmltrans(psNode->aNodeName,alTmp2);
  sprintf(alTmp,"<%s",alTmp2);
  strcat(alBuf,alTmp);
  /*如果有，写入属性列表*/
  for(i = 0;i < psNode->iAttrNum;i++)
  {
    swXmltrans(psNode->Attribs[i].aAttribVal,alTmp2);
    sprintf(alTmp,"<%s",alTmp2);
    sprintf(alTmp," %s=\"%s\" ",psNode->Attribs[i].aAttribName,
                    alTmp2);
    strcat(alBuf,alTmp);
  }
  /*如果没有子节点，写入结束标志返回，否则递归*/
  if(psNode->iChildNum == 0)
  {
    strcat(alBuf,"/>\n");
    fputs(alBuf,fp);
    return 0;
  }
  strcat(alBuf,">\n");
  fputs(alBuf,fp);
  for(i = 0;i < psNode->iChildNum;i++)
  {
    pslNode = &((pNode)psNode->ChildNodes)[i];
    WriteNodeToFile(fp,pslNode,iSj + 2);
  }
  strcpy(alBuf,alTmp1);
  sprintf(alTmp,"</%s>\n",psNode->aNodeName);
  strcat(alBuf,alTmp);
  fputs(alBuf,fp);
  return 0;
}

int WriteNodeToBuf(int sockfd,char *aBuf,int *piLen,pNode psNode,
                                                      short iRootFlag)
{
  char alBuf[2048],alTmp[512],alTmp2[1024];
  unsigned int i,ilLen;
  pNode pslNode;
  char *p;
  int ilRc;
  
  p = &aBuf[*piLen];
  p[0] = 0;
  /*写入节点名*/
  sprintf(alBuf,"<%s",psNode->aNodeName);
  /*如果有，写入属性列表*/
  for(i = 0;i < psNode->iAttrNum;i++)
  {
    swXmltrans(psNode->Attribs[i].aAttribVal,alTmp2);
    sprintf(alTmp," %s=\"%s\" ",psNode->Attribs[i].aAttribName,
                    alTmp2);
    strcat(alBuf,alTmp);
  }
  /*如果没有子节点，写入结束标志返回，否则递归*/
  if(psNode->iChildNum == 0)
  {
    strcat(alBuf,"/>\n");
    strcat(p,alBuf);
    *piLen += strlen(alBuf);
    if(iRootFlag)
  {
    if (cgDebug >= 2)  swDebughex(aBuf,*piLen); 
    ilRc = swTcpPut(sockfd,aBuf,*piLen);
    
    *piLen = 0;
    if(ilRc)
      return(ilRc);
  }
    return 0;
  }
  strcat(alBuf,">\n");
  strcat(p,alBuf);
  ilLen = strlen(alBuf);
  *piLen += ilLen;
  for(i = 0;i < psNode->iChildNum;i++)
  {
    pslNode = &((pNode)psNode->ChildNodes)[i];
    ilRc = WriteNodeToBuf(sockfd,aBuf,piLen,pslNode,0);
    if(ilRc)return(ilRc);
    if( *piLen > 7600)
    {
      if (cgDebug >= 2)  swDebughex(aBuf,*piLen); 
      ilRc = swTcpPut(sockfd,aBuf,*piLen);
      *piLen = 0;
      if(ilRc)
        return(ilRc);
    }
  }
  p = &aBuf[*piLen];
  sprintf(alBuf,"</%s>\n",psNode->aNodeName);
  ilLen = strlen(alBuf);
  *piLen += ilLen;
  strcat(p,alBuf);
/*  p[*piLen] = 0; */ /*delete by zjj 2004.03.08 */

  if(iRootFlag && (*piLen != 0))
  {
    if (cgDebug >= 2)   swDebughex(aBuf,*piLen); 
    ilRc = swTcpPut(sockfd,aBuf,*piLen);
    
    *piLen = 0;
    if(ilRc)
      return(ilRc);
  }
  return 0;
}

int WriteXmlFile(char *aFileName,pNode pRootNode)
{
  FILE *fp;
  
  fp = fopen(aFileName,"w+");
  if(fp == NULL)
  {
    return(345);
  }
  WriteNodeToFile(fp,pRootNode,0);
  fclose(fp);
  return 0;
}

/*************************************************
 取报文类型
 1------------Command
 2------------Info !impossible
 3------------Data
**************************************************/

int GetPacketType(pNode psRootNode)
{
  if(strcmp(psRootNode->aNodeName,"COMMAND") == 0)
    return(1);
  else if(strcmp(psRootNode->aNodeName,"INFO") == 0)
    return(2);
  else if(strcmp(psRootNode->aNodeName,"DATAPACKET") == 0)
    return(3);
  else return(0);
}

int GetNodeByName(pNode psParentNode,char * aNodeName,pNode* ppsNode)
{
  int i;
  pNode pslNode;

  for(i=0;i<psParentNode->iChildNum;i++)
  {
    pslNode = &((pNode)psParentNode->ChildNodes)[i];
    if(strcmp(pslNode->aNodeName,aNodeName) == 0)
    {
      *ppsNode = pslNode;
      return(0);
    }
  }
  return(151);
}

int GetAttrByName(pNode psNode,char *aAttrName,pAttrib *ppsAttr)
{
  int i;
  pAttrib pslAttrib;

  for(i=0;i<psNode->iAttrNum;i++)
  {
    pslAttrib = &psNode->Attribs[i];
    if(strcmp(pslAttrib->aAttribName,aAttrName) == 0)
    {
      *ppsAttr = pslAttrib;
      return(0);
    }
  }
  return(152);
}

/***************************************************/
/*  　　             TCP函数　　   　　　　　　　　　　*/
/***************************************************/
int TcpCli(char *aAddr,short iPort,int *pSock)
{
  struct sockaddr_in slCliaddr;
  struct hostent *host;
  int sock;

  sock = socket(AF_INET,SOCK_STREAM,0);
  if(sock == -1)
  {
    return(105);
  }
  if(!(host = gethostbyname(aAddr)))
  {
    if(!(host = gethostbyaddr(aAddr,strlen(aAddr),AF_INET)))
      return(106);
  }
  slCliaddr.sin_family = AF_INET;
  memcpy((char*)&slCliaddr.sin_addr,host->h_addr,host->h_length);
  slCliaddr.sin_port = htons(iPort);
  if(connect(sock,(struct sockaddr*)&slCliaddr,
        sizeof(struct sockaddr_in)) == -1)
    return(107);
  *pSock = sock;
  return(0);
}
int TcpSrv(short iPort,int *pSock)
{
  struct sockaddr_in slSrvaddr;
  int sock,ilRc;
  struct linger slLinger;
  
  slSrvaddr.sin_family = AF_INET;
  slSrvaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  slSrvaddr.sin_port = htons(iPort);
  sock = socket(AF_INET,SOCK_STREAM,0);
  if(sock == -1)
  {
    return(105);
  }
  
  slLinger.l_onoff  =1;
  slLinger.l_linger =1;
  
  ilRc = setsockopt(sock,SOL_SOCKET,SO_LINGER,&slLinger,
    sizeof(struct linger));
  if (ilRc==-1) 
  {
    close(sock);
    return(188);
  }
  
  if(bind(sock,(struct sockaddr*)&slSrvaddr,
             sizeof(struct sockaddr_in)) 
             == -1)
  {
    close(sock);
    return(108);
  }
  if(listen(sock,50) == -1)
  {
    close(sock);
    return(109);
  }
  *pSock = sock;
  return(0);
}

int TcpServe(int sock,int *piClisock,char *aCliaddr)
{
  struct sockaddr_in slClientSockaddr;
  int ilClientSock;

  #ifdef OS_SCO
    int ilLen;
  #else
    socklen_t ilLen;
  #endif

  ilLen = sizeof(struct sockaddr_in);
  ilClientSock = accept(sock,(struct sockaddr*)&slClientSockaddr,
                    &ilLen);

  if(ilClientSock != -1)
  {
    strcpy(aCliaddr,(char *)inet_ntoa(slClientSockaddr.sin_addr));
    *piClisock = ilClientSock;
    return(0); 
  }  
  return(110);
}

void TcpTimeout(int sig)
{
  igTimeoutFlag = 1;
}

int TcpGet(int sock,char *aMsgpack,unsigned int *piMsglen,short iTimeout)
{
  unsigned int ilRcvlen;

  if(iTimeout != 0)
  {
    igTimeoutFlag = 0;
    signal(SIGALRM,TcpTimeout);
    alarm(iTimeout);
  }
  ilRcvlen = recv(sock,aMsgpack,*piMsglen,0);
  if(iTimeout != 0)
  {
    alarm(0);
    signal(SIGALRM,SIG_DFL);
  }
  if(igTimeoutFlag == 1)
  {
    return(111);
  }
  if(ilRcvlen <= 0)
  {
    *piMsglen = 0;
    return(112);
  }  
  *piMsglen = ilRcvlen;
  return(0);
}

int swTcpGet(int sock,char *aMsgpack,unsigned int *piMsglen,short iTimeout)
{
  int ilRc;
  unsigned int ilTotal,ilBuflen;
  unsigned int ilCurlen = 0,ilLen;
  char alLenbuf[10];

  ilBuflen = *piMsglen;
  ilCurlen = 8;
  ilRc = TcpGet(sock,aMsgpack,&ilCurlen,iTimeout);
  if(ilRc)return(ilRc);

  strncpy(alLenbuf,aMsgpack,8);
  ilTotal = atoi(alLenbuf);
  if(ilTotal > ilBuflen)return(114);
  *piMsglen = ilTotal + 8;
 
  while(ilTotal > 0)
  {
    if(ilTotal > 8190)
      ilLen = 8190;
    else ilLen = ilTotal;
    ilRc = TcpGet(sock,&aMsgpack[ilCurlen],&ilLen,0); 
    if(ilRc)return(ilRc);
    ilCurlen += ilLen;
    ilTotal -= ilLen;
  } 
  return(0);
}

int swTcpPut(int sock,char *aMsgpack,unsigned int iMsglen)
{
  char alMsgpack[10000];
  short ilRc;
  
  sprintf(alMsgpack,"%8d",iMsglen);
  memcpy(&alMsgpack[8],aMsgpack,iMsglen);
  ilRc = send(sock,alMsgpack,iMsglen + 8,0);
  if(ilRc <= 0)
  {
    swVdebug(2,"S0200: swTcpPut 错误码 = [%d]",ilRc);
    return(117); 
  }
  swVdebug(3,"S0210: swTcpPut 成功返回.");
  return(0);
}

/**************************************************/
/*               文件的操作函数　      　　　　　　　 */
/**************************************************/
/*  add by zjj 2001.10.26    */
/*  前台系统运行状态图监控 */
/***************************************************************
 ** 函数名      : swWinmon()
 ** 功  能      : 
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int ReadMode7(pDataset pSet)
{
  short   ilCount_swt_sys_task;
  short   ilCount_swt_sys_queue;
  /* del by nh 20020807
  struct  swt_sys_task pslSwt_sys_task[iMAXTASKREC];*/
  struct swt_sys_task *pslSwt_sys_task;/* add by nh 20020807*/
  struct  swt_sys_queue pslSwt_sys_queue[400];
  short   ilRc,i;
  struct  mbinfo slMbinfo;
  unsigned char    clPort_status;
  char    alStatus[6];
  Record slRec;

  swVdebug(4,"S0220: [%d函数调用] ReadMode7,TableName = [%s]",
    getpid(),
    pSet->aTableName);
  /*  调用函数swShmcheck()，保证共享内存指针为最新  */
  if(strcmp(pSet->aTableName,"SWMONTASK") == 0)
  { 
    ilRc = swShmcheck();
    if (ilRc != 0) 
    { 
      swVdebug(0,"S0230: [错误/共享内存] swShmcheck共享内存状态检查出错!");
      return 160;
    }
    swVdebug(2,"S0240: [子进程%d] 共享内存检测 OK!",getpid());
    
    /* del by nh 20020807
    memset(pslSwt_sys_task,0x00,iMAXTASKREC * (sizeof(struct swt_sys_task))); */
    /* modify by nh 20020807
    ilRc = swShmselect_swt_sys_task_all_p(pslSwt_sys_task,&ilCount_swt_sys_task);*/
    ilRc = swShmselect_swt_sys_task_all_p(&pslSwt_sys_task,&ilCount_swt_sys_task);
    if (ilRc != 0)
    {
      swVdebug(1,"S0250: [错误/共享内存]  swShmselect_swt_sys_task_all_p从共享内存中读取数据出错!");
      return(161);
    }
    swVdebug(2,"S0260: [子进程%d] 从共享内存读取数据 OK!",getpid()); 
  
  /* 组织任务报文 */
    for(i = 0;i < ilCount_swt_sys_task;i++)
    {
      _swTrim(pslSwt_sys_task[i].task_name);
      strcpy(slRec.Values[0].aValue,pslSwt_sys_task[i].task_name);
      sprintf(slRec.Values[1].aValue,"%ld",pslSwt_sys_task[i].pid);
      _swTrim(pslSwt_sys_task[i].task_status);
      strcpy(slRec.Values[2].aValue,pslSwt_sys_task[i].task_status);
      _swTrim(pslSwt_sys_task[i].task_attr);
      strcpy(slRec.Values[3].aValue,pslSwt_sys_task[i].task_attr);
      
      sprintf(slRec.Values[4].aValue,"%ld",pslSwt_sys_task[i].start_time);
      sprintf(slRec.Values[5].aValue,"%d",pslSwt_sys_task[i].restart_num);
      sprintf(slRec.Values[6].aValue,"%d",pslSwt_sys_task[i].restart_max);    
      Insert(&slRec,pSet);
    }
  }
  
  if(strcmp(pSet->aTableName,"SWMONMB") == 0)
  {
  /* 组织邮箱报文 */
    i = 1;
    ilRc = bmqOpen(iMBSYSCON);
    if (ilRc)
    {
      swVdebug(0,"S0270: [错误/邮箱] 打开邮箱[%d]出错! error: %d", 
             iMBSYSCON, ilRc);
      return(162);
    }
    swVdebug(2,"S0280: 打开邮箱[ %d ]OK! ");
    memset(&slMbinfo,0x0,sizeof(struct mbinfo));
    while((ilRc = bmqGetmbinfo(i,&slMbinfo)) != 100)
    {
      if (ilRc == -1)
      {
        swVdebug(1,"S0290: bmqGetmbinfo(%d): error", i);
        return(-163);
      }
      if ( ilRc || slMbinfo.lSendnum || slMbinfo.lRecvnum || 
                 slMbinfo.lPendnum || slMbinfo.iConnect )
      {
        sprintf(slRec.Values[0].aValue,"%d",i);
        sprintf(slRec.Values[1].aValue,"%ld",slMbinfo.lSendnum);
        sprintf(slRec.Values[2].aValue,"%ld",slMbinfo.lRecvnum);   
        sprintf(slRec.Values[3].aValue,"%ld",slMbinfo.lPendnum);   
        sprintf(slRec.Values[4].aValue,"%d",slMbinfo.iConnect);   
        Insert(&slRec,pSet);
      }
      i++;
    }
    bmqClose();
  }
 
  if(strcmp(pSet->aTableName,"SWMONAPPS") == 0)
  {
  /* 组织端口报文 */
    ilRc = swShmcheck();
    if (ilRc != 0) 
    { 
      swVdebug(1,"S0300: 共享内存状态检查出错!");
      return 160;
    }
    swVdebug(2,"S0310: 共享内存检测 OK!");
  
    memset(pslSwt_sys_queue,0x00,400 * (sizeof(struct swt_sys_queue)));
    ilRc = swShmselect_swt_sys_queue_all(pslSwt_sys_queue,
            &ilCount_swt_sys_queue);
    if (ilRc != 0)
    {
      swVdebug(1,"S0320: 从共享内存中读取数据出错!");
      return(161);
    }
    swVdebug(2,"S0330: 从共享内存读取数据 OK!"); 
    for(i = 0;i < ilCount_swt_sys_queue;i++)
    {
      sprintf(slRec.Values[0].aValue,"%d",pslSwt_sys_queue[i].q_id);
      _swTrim(pslSwt_sys_queue[i].q_name);
      strcpy(slRec.Values[1].aValue,
          pslSwt_sys_queue[i].q_name);
      _swTrim(pslSwt_sys_queue[i].comm_attr);
      strcpy(slRec.Values[2].aValue,
          pslSwt_sys_queue[i].comm_attr);
      _swTrim(pslSwt_sys_queue[i].comm_type);
      strcpy(slRec.Values[3].aValue,
          pslSwt_sys_queue[i].comm_type);
   
      clPort_status = pslSwt_sys_queue[i].port_status;
      memset(alStatus,'1',5);
      if ((clPort_status & 0x80) == 0x00)  alStatus[0] = '0';
      if ((clPort_status & 0x40) == 0x00)  alStatus[1] = '0';  
      if ((clPort_status & 0x20) == 0x00)  alStatus[2] = '0';
      if ((clPort_status & 0x10) == 0x00 ) alStatus[3] = '0';
      if ((clPort_status & 0x08) == 0x00)  alStatus[4] = '0';  
      strcpy(slRec.Values[4].aValue,
           alStatus); 
      Insert(&slRec,pSet);
    }
  }
  return(0);
}
/*  end add by zjj 2001.10.25 */

int WriteMode7(pDataset pSet)
{
  return(199);
}

int ReadMode8(pDataset psSet)
{
  struct swt_sys_config slSwt_sys_config;
  int ilShmid;
  char *alShmtmp_d;
  static short ilCheckcount = 0;
  int ilMaxtranlog,ilMaxproclog,ilMaxsaflog,i;
  int ilTrning,ilTrnend,ilTrnovertime,ilTrnreving,ilTrnrevend,ilSafing,ilSaffail;
  Record slRec;
  
  ilTrning = 0;
  ilTrnend = 0;
  ilTrnovertime = 0;
  ilTrnreving = 0;
  ilTrnrevend = 0;
  ilSafing = 0;
  ilSaffail = 0;
  
   /* 读SWCONFIG.CFG配置文件 */
  if (swShmcfgload(&slSwt_sys_config) != 0)
  {
    swVdebug(2,"S0340: ReadMode8 读SWCONFIG出错!");
    return 1;
  }

  ilMaxtranlog = slSwt_sys_config.iMaxtranlog;
  ilMaxproclog = slSwt_sys_config.iMaxproclog;
  ilMaxsaflog = slSwt_sys_config.iMaxsaflog;

  ilShmid = shmget((key_t)slSwt_sys_config.iShmkey,0,IPC_EXCL|0666);
  if (ilShmid < 0)
  {
    swVdebug(2,"S0350: ReadMode8 连接静态共享内存失败!");
    return 2;
  }

  if(shmdt(psgShmidx_d) && ilCheckcount != 0)
  {
    swVdebug(2,"释放动态表共享内存指针出错");
    return(3); 
  }

  if ((alShmtmp_d = shmat(ilShmid,(char *)0,SHM_RND)) == (char *)-1)
  {
    swVdebug(2,"联接动态表共享内存出错");
    return(4); 
  }
  psgShmidx_d = (struct shmidx_d *)alShmtmp_d;

  sgShmbuf_d.psShm_tran_log = (struct shm_tran_log *)
    (alShmtmp_d + psgShmidx_d->sIdx_tran_log.lOffset);

  sgShmbuf_d.psShm_proc_log = (struct shm_proc_log *)
    (alShmtmp_d + psgShmidx_d->sIdx_proc_log.lOffset);

  sgShmbuf_d.psShm_rev_saf = (struct shm_rev_saf *)
    (alShmtmp_d + psgShmidx_d->sIdx_rev_saf.lOffset);

  i = psgShmidx_d->sIdx_tran_log.iElink;
  if((i > 0) && (sgShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iRlink != 0))
    return(5);
  i = psgShmidx_d->sIdx_tran_log.iSlink;
  if((i > 0) &&(sgShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iLlink != 0))
    return(6);
  while(i)
  {
    if(sgShmbuf_d.psShm_tran_log[i-1].sLinkinfo.cTag ==  1)
    {
      if(sgShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.tran_status == iTRNING)
        ilTrning++;
      if(sgShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.tran_status == iTRNEND)
        ilTrnend++;
      if(sgShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.tran_status == iTRNOVERTIME)
        ilTrnovertime++;
      if(sgShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.tran_status == iTRNREVING)
        ilTrnreving++;
      if(sgShmbuf_d.psShm_tran_log[i-1].sSwt_tran_log.tran_status == iTRNREVEND)
        ilTrnrevend++;
    }
    i = sgShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iRlink;
  }
  
  i = psgShmidx_d->sIdx_rev_saf.iSlink;
  while(i)
  {
    if(sgShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.cTag == 1)
    {
      if(sgShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_status[0] == '1')
        ilSafing++;
      if(sgShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_status[0] == '1')
        ilSaffail++;
    }
    i = sgShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  /* 将统计结果加入记录集 */
  sprintf(slRec.Values[0].aValue,"%d",ilMaxtranlog);
  sprintf(slRec.Values[1].aValue,"%d",ilMaxsaflog);
  sprintf(slRec.Values[2].aValue,"%d",ilTrning);
  sprintf(slRec.Values[3].aValue,"%d",ilTrnend);
  sprintf(slRec.Values[4].aValue,"%d",ilTrnovertime);
  sprintf(slRec.Values[5].aValue,"%d",ilTrnreving);
  sprintf(slRec.Values[6].aValue,"%d",ilTrnrevend);
  sprintf(slRec.Values[7].aValue,"%d",ilSafing);
  sprintf(slRec.Values[8].aValue,"%d",ilSaffail);
  Insert(&slRec,psSet);
  swVdebug(3,"S0360: ReadMode8 成功返回");
  return(0);
}

int WriteMode8(pDataset psSet)
{
  return(199);
}

int WriteMode9(pDataset psSet)
{
  return 0;
}

/***************************************************************
 ** 函数名      : InitRecord()
 ** 功  能      : 初始化一个记录结构
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
void InitRecord(pRecord pRec)
{
  int i;
  
  pRec->iHideFlag = 0;
  for(i=0;i < MAXFLDNUM;i++)
    pRec->Values[i].aValue[0] = 0;
}

/***************************************************************
 ** 函数名      : GetOnlyName()
 ** 功  能      : 去掉文件名中的路径和扩展名
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
char * GetOnlyName(char *filename)
{
  char *p,*p1;

  p = strrchr(filename,'/');
  if(p != NULL)
    p++;
  else
    p = filename;
  p1 = strrchr(p,'.');
  if(p1 != NULL)*p1 = 0;
  return(p);
}
/***************************************************************
 ** 函数名      : InitDataset()
 ** 功  能      : 初始化一个数据集 Dataset
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int InitDataset(char * TableName,Dataset * psDataset)
{
  int i,ilTableId;
  
  strcpy(psDataset->aTableName,TableName);
  psDataset->aFileName[0] = 0;	
  for(i=0;i<TABLENUM;i++)
  {
    if(strcmp(sgTables[i].aTableName,TableName) == 0)
    {
      psDataset->iRecordNum = 0;
      psDataset->iFieldNum = sgTables[i].iFieldNum;
      psDataset->iStoreMode = sgTables[i].iStoreMode;
      ilTableId = sgTables[i].iTableID;
      break;
    }
  }	
  if(i == TABLENUM) return(120);	
  for(i =0;i <FIELDNUM;i++)
  {
    if(sgFields[i].iTableID == ilTableId)
    {
      psDataset->Fields = &sgFields[i];
      break;
    }
  }
  psDataset->iRecordNum = 0;
  return(0);
}

/***************************************************************
 ** 函数名      : FreeDataset()
 ** 功  能      : 释放数据集中动态分配的内存
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
void FreeDataset(Dataset * psDataset)
{
  if(psDataset->iRecordNum > 0)
    free(psDataset->Records);
  psDataset->iRecordNum = 0;
}
/***************************************************************
 ** 函数名      : GetMode1Record
 ** 功  能      : 从模式一文件中读取一条记录
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int GetMode1Record(FILE * fp,pRecord pRec)
{
  char *p,*p1,*p2,alBuf[2048];
  short i,iFld = 0;
  
  alBuf[0] = 0;
  for(i=0;i<MAXFLDNUM;i++)
    pRec->Values[i].aValue[0] = 0;
  fgets(alBuf,2048,fp);
  _swTrim(alBuf);
  if(alBuf[0] == 0)
    return(100);
  if(alBuf[0] == '#')
  {
    pRec->iHideFlag = 1;
    p1 = &alBuf[1];
    _swTrim(p1);
  }
  else
  {
    pRec->iHideFlag = 0;
    p1 = alBuf;
  }
  if((p = strchr(p1,'#')) != NULL)
  {
    *p = 0;
    p++;
    strcpy(pRec->aContent,"#");
    strcat(pRec->aContent,p);
  }
  else
    pRec->aContent[0] = 0;
  if(p1[0] == '\"')
  {
    p1++;
    p = strchr(p1,'\"');  
  }
  else
  {
    p = strchr(p1,' ');
    p2 = strchr(p1,'	');
    if(p == NULL)p = p2;
    if((p != NULL)&&(p2 != NULL))
    {
      if (p > p2) p = p2; 
    }
  }
  while(p != NULL)
  {
    *p = 0;
    if(strcmp(p1,"-") == 0)
      pRec->Values[iFld].aValue[0] = 0;
    else
      strcpy(pRec->Values[iFld].aValue,p1);
    p1 = ++p;
    _swTrim(p1);
    if(p1[0] == '\"')
    {
      p1++;
      /*
      p = strchr(p1,'\"'); */
      p = strstr(p1,"\" ");
    }
    else
    {
      p = strchr(p1,' ');
      p2 = strchr(p1,'	');
      if(p == NULL)p = p2;
      if((p != NULL)&&(p2 != NULL))
      {
         if (p > p2) p = p2; 
      }
    }
    iFld++;
  }
  _swTrim(p1);
  if(strcmp(p1,"-") == 0)
    pRec->Values[iFld].aValue[0] = 0;
  else
    strcpy(pRec->Values[iFld].aValue,p1);
  return(0);
}
/***************************************************************
 ** 函数名      : ReadMode1
 ** 功  能      : 模式一文件读取
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int ReadMode1(pDataset psDataset)
{
  FILE   *flTblcfg;
  char   alFileName[300];
  Record slRec;
  int ilRc;

  swVdebug(4,"S0370: [函数调用/%d] ReadMode1,表名为[%s]",
          getpid(),psDataset->aTableName);
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/%s",
           getenv("SWITCH_CFGDIR"),psDataset->aTableName);
  */
  sprintf(alFileName,"%s/config/%s",agPath,psDataset->aTableName);

  strcpy(psDataset->aFileName,alFileName);
  if((flTblcfg = fopen(alFileName,"r")) == NULL)
  {
    if((flTblcfg = fopen(alFileName,"w+")) == NULL)
    {
      FreeDataset(psDataset);
      swVdebug(2,"S0380: [错误/系统调用/%d] fopen创建文件[%s]出错",
                                          getpid(),alFileName);
      swVdebug(2,"S0390: [函数返回/%d] ReadMode1 返回码:123",getpid());
      return(123);
    }
  }
			
  while(1)
  {
    ilRc = GetMode1Record(flTblcfg,&slRec);
    if(feof(flTblcfg))
      break;
    if(ilRc == 100)continue;
    Insert(&slRec,psDataset);
  }
  fclose(flTblcfg);
  swVdebug(2,"S0400: [函数返回/%d] ReadMode1 OK!",getpid());
  return(0);
}

/***************************************************************
 ** 函数名      : WriteMode1
 ** 功  能      : 模式一写文件
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int WriteMode1(pDataset psDataset,pRecord pRec,short iRecNum,
                                    pDataset pRtnSet,short flag)
{
  int i,j,k,ilFlag;
  char alLine[4096],alTmp[500],alFmt[10];
  FILE * fTable;
  Record slRec;
  char alTmpbuf[500];   /* add by zjj 2001.10.27  */
  
  swVdebug(2,"S0410: [函数调用/%d] WriteMode1(iRecNum=%d,flag=%d)",
             getpid(),iRecNum,flag);
  if(flag)
  {
/*
    sprintf(alTmp,"cp %s %s.BAK",psDataset->aFileName,psDataset->aFileName);
    system(alTmp);
*/  
    if((fTable = fopen(psDataset->aFileName,"w+")) == NULL)
    {
      swVdebug(1,"S0420: [错误/系统调用/%d] fopen创建文件%s出错 ",
                  getpid(),
                  psDataset->aFileName);
      swVdebug(2,"S0430: [函数返回/%d] WriteMode1 返回码:123",getpid());
      return(123);
    }
  }
  
  for(i = 0;i < psDataset->iRecordNum;i++)
  {
    InitRecord(&slRec);
    if(!flag)
    {
      ilFlag = 0;
      for(k = 0;k < iRecNum;k++)
      {
        ilFlag = 0;
        for(j =0;j <psDataset->iFieldNum;j++)
        {
          if(pRec[k].Values[j].aValue[0] != 0)
          {
            if(strcmp(pRec[k].Values[j].aValue,
                     psDataset->Records[i].Values[j].aValue) != 0)
            {
              ilFlag = 1;
              break;
            }
          }
        } 
        if(ilFlag == 0)break;
      }
      if(ilFlag)continue;
    }

    if(psDataset->Records[i].iHideFlag)
    {
      alLine[0] = '#';
      alLine[1] = 0;
    }
    else
      alLine[0] = 0;
    for(j = 0;j < psDataset->iFieldNum;j++)
    {
      switch(psDataset->Fields[j].iFieldType)
      {
       /* modify by zjj 2001.10.26  */
        case T_SHORT:
        case T_INT:
          strcpy(alFmt,"%-4s");
          break;
        case T_LONG:
        case T_FLOAT:
        case T_DOUBLE:
          strcpy(alFmt,"%-8s");
          break;
        case T_PCHAR:
          if(psDataset->Fields[j].iFieldWidth > 30)
            strcpy(alFmt,"%-30s");
          else
            sprintf(alFmt,"%%-%ds",psDataset->Fields[j].iFieldWidth * 3 / 4);
          break;
        case T_CHAR:
          strcpy(alFmt,"%-2s");
          break;
        default:
          swVdebug(2,"S0440: [函数返回/%d] WriteMode1 error:127",getpid());
          return(127);
        /* end modify by zjj 2001.10.26  */
      }
/*      strcpy(alFmt,"%s");      */    /*  delete by zjj 2001.10.26  */
      if(strcmp(psDataset->Records[i].Values[j].aValue,"") == 0)
        sprintf(alTmp,alFmt,"-");
      /*  add by zjj 2001.10.27  for if space in buffer then add ""   */
      else if ((strchr(psDataset->Records[i].Values[j].aValue,' ') != NULL) || \
        (strchr(psDataset->Records[i].Values[j].aValue,'	') != NULL))
      {
      	memset(alTmpbuf,0x00,sizeof(alTmpbuf));
      	sprintf(alTmpbuf,"\"%s\"",psDataset->Records[i].Values[j].aValue);
        sprintf(alTmp,alFmt,alTmpbuf);
      }
      /* end add by zjj 2001.10.27                                    */
      else
        sprintf(alTmp,alFmt,psDataset->Records[i].Values[j].aValue);
      strcat(alLine,alTmp);
      strcat(alLine," ");
    }
    strcat(alLine,psDataset->Records[i].aContent);
    if(flag)
    {
      strcat(alLine,"\n");    
      fputs(alLine,fTable);
    }
    else
    {
      strcpy(slRec.Values[0].aValue,alLine);
      Insert(&slRec,pRtnSet);
    }
  }
  if(flag)
    fclose(fTable);
  swVdebug(2,"S0450: [函数返回/%d] WriteMode1 OK!",getpid());
  return(0);
}

/***************************************************************
 ** 函数名      : ImportMode1
 ** 功  能      : 模式一导入
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int ImportMode1(pDataset pTset,pDataset pVset)
{
  int i;
  char alFileName[300],alLine[512];
  FILE *fp;
  
  swVdebug(2,"S0460: [函数调用/%d] ImportMode1 表名[%s]",
                  getpid(),pTset->aTableName);
  swDebugset(2,pVset);
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/%s",getenv("SWITCH_CFGDIR"),pTset->aTableName);
  */
  sprintf(alFileName,"%s/config/%s",agPath,pTset->aTableName);
  if((fp = fopen(alFileName,"a+")) == NULL)
  {
    swVdebug(1,"S0470: [错误/系统调用/%d] fopen打开文件(a+)[%s]",
                getpid(),alFileName);
    swVdebug(2,"S0480: [函数返回/%d] ImportMode1 返回码:123",getpid());
    return(123);
  }

  for(i = 0;i < pVset->iRecordNum;i++)
  {
    sprintf(alLine,"%s\n",pVset->Records[i].Values[0].aValue);
    fputs(alLine,fp);
  } 
  fclose(fp);
  swVdebug(2,"S0490: [函数返回/%d] ImportMode1 OK!",getpid());
  return(0);
}

/***************************************************************
 ** 函数名      : ExportMode1
 ** 功  能      : 模式一导出
 ** 作  者      : 
 ** 建立日期    : 2001/08/01
 ** 最后修改日期: 2001/08/01
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int ExportMode1(pRecord pRec,short iRecNum,pDataset pTset,pDataset pRtnSet)
{
  int ilRc;

  swVdebug(2,"S0500: [函数调用/%d] ExportMode1",getpid());
  ilRc = WriteMode1(pTset,pRec,iRecNum,pRtnSet,0); 
  swVdebug(2,"S0510: [函数返回/%d] ExportMode1 return code[%d]",
      getpid(),ilRc);
  return(ilRc);
}

int ImportMode2(pDataset pTset,pDataset pVset)
{
  int ilRc;

  swVdebug(2,"S0520: [函数调用/%d] ImportMode2",getpid());
  ilRc = ImportMode1(pTset,pVset);
  swVdebug(2,"S0530: [函数返回/%d] ImportMode2 RtnCode[%d]",
      getpid(),
      ilRc);
  return(ilRc);
}

int ImportMode3G(keyrec *pKeys,short iKeyNum,pDataset pTset,pDataset pVset)
{
  int i,ilRc;
  char alFileName[300],alLine[512],alGname[31],alDname[31];
  FILE *fp;

  swVdebug(2,"S0540: [函数调用/%d] ImportMode3G  TableName = [%s]",
        getpid(),
        pTset->aTableName);

  ilRc = GetMode3FileName(pTset,pKeys,iKeyNum,alFileName,alGname,alDname);
  if(ilRc)
  {
    swVdebug(1,"S0550: [错误/其它调用/%d] GetMode3FileName return [%d]",
           getpid(),ilRc);
    swVdebug(2,"S0560: [函数返回/%d] ImportMode3G RtnCode:[%d]",
        getpid(),ilRc);
    return(ilRc);
  }  

  if((fp = fopen(alFileName,"w+")) == NULL)
  {
    swVdebug(1,"S0570: [错误/系统调用/%d] fopen创建文件[%s]出错",
                      getpid(),alFileName);
    swVdebug(2,"S0580: [函数返回/%d] ImportMode3G 返回码=123",
         getpid());
    return(123);
  }

  for(i = 0;i < pVset->iRecordNum;i++)
  {
    sprintf(alLine,"%s\n",pVset->Records[i].Values[0].aValue);
    fputs(alLine,fp);
  } 
  fclose(fp);
  swVdebug(2,"S0590: [函数返回/%d] ImportMode3G 返回码=0",
         getpid());
  return(0);
}

int ImportMode3M(pDataset pTset,pDataset pVset)
{
  int ilRc;

  swVdebug(2,"S0600: [函数调用/%d] ImportMode3M",getpid());
  ilRc = ImportMode1(pTset,pVset);
  swVdebug(2,"S0610: [函数返回/%d] ImportMode3M,返回码=%d",
              getpid(),ilRc);
  return(ilRc);
}

int ExportMode3G(keyrec *pKeys,short iKeyNum, 
          pRecord pRec,short iRecNum,pDataset pTset,pDataset pRtnSet)
{
  char alFileName[300],alGname[31],alDname[31],alLine[512];
  Record slRec;
  int ilRc;
  FILE *fp;

  swVdebug(2,"S0620: [函数调用/%d] ExportMode3G",getpid());
  ilRc = GetMode3FileName(pTset,pKeys,iKeyNum,alFileName,alGname,alDname);
  if(ilRc)
  {
    swVdebug(1,"S0630: [错误/其它函数/%d] GetMode3FileName,返回码=%d",
               getpid(),ilRc);
    swVdebug(2,"S0640: [函数返回/%d] ExportMode3G,返回码=%d",getpid(),ilRc);
    return(ilRc);
  }

  if((fp = fopen(alFileName,"r")) == NULL) 
  {
    swVdebug(1,"S0650: [错误/系统调用/%d] fopen打开文件[%s]出错",
                      getpid(),alFileName);
    swVdebug(2,"S0660: [函数返回/%d] ExportMode3G,返回码=123",getpid());
    return(123);
  }

  while(fgets(alLine,512,fp) != NULL)
  {
    InitRecord(&slRec);
    strcpy(slRec.Values[0].aValue,alLine);
    Insert(&slRec,pRtnSet);
  }
  fclose(fp);
  swVdebug(2,"S0670: [函数返回/%d] ExportMode3G,返回码=0",getpid());
  return(0);
}

int ExportMode3M(
         pRecord pRec,short iRecNum,pDataset pTset,pDataset pRtnSet)
{
  int ilRc;
  
  swVdebug(2,"S0680: [函数调用] ExportMode3M");
  ilRc = WriteMode1(pTset,pRec,iRecNum,pRtnSet,0); 
  swVdebug(2,"S0690: [函数返回] ExportMode3M,返回码=%d",ilRc);
  return(ilRc);
}

int GetMode2Record(FILE * fp,pDataset pSet,pRecord pRec)
{
  int i,j,ilKeypos = 1,ilFlag = 0,ilFldNum;
  long llPos = 0;
  char *p,*p1,*p2,alBuf[1024];
  
  InitRecord(pRec);
  ilFldNum = pSet->iFieldNum;	
  while(fgets(alBuf,512,fp) != NULL)
  {
    _swTrim(alBuf);
    if(alBuf[0] == '#')
    {
      continue;
/*
      p1 = &alBuf[1];
      _swTrim(p1); 
*/
    }
    else
    {
      p1 = alBuf;
    }
    if(p1[0] == '[')
    {
      if(!ilFlag)
        ilFlag = 1;
      else
      {
        if(llPos != 0)
        {
          fseek(fp,llPos,0);
          return(0);
        }
      }
      
      if((p = strchr(p1,'#')) != NULL)
      {
        *p = 0;
      	p++;
      	strcpy(pRec->aContent,p);
      }
      if((p2 = strchr(p1,']')) != NULL)
        *p2 = 0; 
      p1++;
      p = strchr(p1,',');
      while(p != NULL)
      {
      	*p = 0;
      	_swTrim(p1);
      	p++;
      	j = ilKeypos;
        for(i=0;i < ilFldNum;i++)
        {
          if(pSet->Fields[i].iFieldAttr & 0x02)
          {
            j--;
            if(j == 0)
            {
              strcpy(pRec->Values[i].aValue,p1);
              ilKeypos++;
              break;
             }
           }
         }
         p1 = p;
         _swTrim(p1);
         p = strchr(p1,',');
       }
       j = ilKeypos;
       for(i=0;i < ilFldNum;i++)
       {
          if(pSet->Fields[i].iFieldAttr & 0x2)
          {
            j--;
            if(j == 0)
            {
              strcpy(pRec->Values[i].aValue,p1);
              ilKeypos++;
              break;
             }
           }
        }
        continue;
     }
    
    if(!ilFlag)ilFlag = 1;
    p = strchr(p1,'=');
    if(p == NULL)
      continue;
    *p = 0;
    p++;
    _swTrim(p1);
    _swTrim(p);
    
    for(i = 0;i < ilFldNum;i++)
    {
      if(!(pSet->Fields[i].iFieldAttr & 0x2))
      {
        if(strcmp(pSet->Fields[i].aFieldName,p1) == 0)
        {
          strcpy(pRec->Values[i].aValue,p);
          break;
        }
      }
    }
    llPos = ftell(fp);
  }
  return(0);
}

int ReadMode9(pDataset psSet)
{
  Dataset slSet_obj;
  int i,ilRc,ilFlag;
  char alCmdStr[512];
  char alComm[51];
  Record slRec;
  pRecord pslRec;
  FILE *fp;

  InitDataset("SWMONOBJ.CFG",&slSet_obj);
  if(ReadMode1(&slSet_obj))
    return 155;
  
  /*sprintf(alCmdStr,"ps -e -o comm -o pid -o %%cpu -o cputime -o vsz");*/
  sprintf(alCmdStr,"ps -e -o comm -o pid -o cpu -o cputime -o vsz");
  if((fp = popen(alCmdStr,"r")) == NULL)
    return 155;

  while(1)
  {
    ilRc = GetMode1Record(fp,&slRec);
    if(ilRc)
      break;
    ilFlag = 0;
    for(i = 0;i < slSet_obj.iRecordNum;i++)
    {
      pslRec = &slSet_obj.Records[i];
      if(strcmp(pslRec->Values[1].aValue,"3") != 0)
        continue; 
      
      strcpy(alComm,pslRec->Values[0].aValue);
      _swTrim(alComm);
      if(strcmp(alComm,slRec.Values[0].aValue) == 0)
      {
        ilFlag = 1;
        break;
      }
    }
    if(!ilFlag)continue;
    Insert(&slRec,psSet);
  }

  pclose(fp);

  return 0;
}
int ReadMode10(pDataset psDataset)
{
   FILE *flTblcfg;
   char alFileName[300],lineText[512];
   Record slRec;

   swVdebug(2,"S0700:[函数调用/%d]ReadMode10,表名为[%s]",getpid(),psDataset->aTableName);
   sprintf(alFileName,"%s/config/%s",getenv("SWITCH_CFGDIR"),psDataset->aTableName);
   strcpy(psDataset->aFileName,alFileName);
   if ((flTblcfg = fopen(alFileName,"r")) ==NULL)
   {
    if((flTblcfg = fopen(alFileName,"w+")) == NULL)
    {
      FreeDataset(psDataset);
      swVdebug(2,"S0710: [错误/系统调用/%d] fopen创建文件[%s]出错",
                                          getpid(),alFileName);
      swVdebug(2,"S0720: [函数返回/%d] ReadMode10 返回码:123",getpid());
      return(123);
    }
  }
  while(1)
  {
    /* /ilRc = GetMode1Record(flTblcfg,&slRec);     */
    if(feof(flTblcfg))
      break;
    if(fgets(lineText,512,flTblcfg) == NULL)break;
        
    strcpy(slRec.Values[0].aValue,lineText);
    Insert(&slRec,psDataset);
   }
  fclose(flTblcfg);
  swVdebug(2,"S0730: [函数返回/%d] ReadMode10 OK!",getpid());
  return(0);
}

int WriteMode10(pDataset pSet)   
 { 
     FILE *lfTblcfg;
     char alFileName[1024];	 
     char alBuf[1024];
     short i;
     sprintf(alFileName,"%s/config/SWMACRO.CFG",getenv("SWITCH_CFGDIR"));
     if((lfTblcfg = fopen(alFileName,"w+")) == NULL)
    {
      FreeDataset(pSet);
      swVdebug(2,"S0740: [错误/系统调用/%d] fopen创建文件[%s]出错",
                                          getpid(),alFileName);
      swVdebug(2,"S0750: [函数返回/%d] WriteMode10返回码:123",getpid());
      return(123);
    }
  
          
  swVdebug(2,"S0760: [函数调用/%d] WriteMode10",getpid());
  for(i = 0;i < pSet->iRecordNum;i++)
  {
    sprintf(alBuf,"%s",pSet->Records[i].Values[0].aValue);
    _swTrim(alBuf);
    strcat(alBuf,"\n");
    fputs(alBuf,lfTblcfg);
    
  }
  swVdebug(2,"S0770: [函数返回/%d] WriteMode10,返回码=0",getpid());
  fclose(lfTblcfg);
  return(0);
}

int ReadMode2(pDataset psDataset)
{
  char alFileName[300];
  int ilRc;
  FILE *fTblcfg;
  Record slRec;
  
  swVdebug(2,"S0780: [函数调用/%d] ReadMode2",getpid());
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/%s",getenv("SWITCH_CFGDIR"),psDataset->aTableName); */
  sprintf(alFileName,"%s/config/%s",agPath,psDataset->aTableName); 

  strcpy(psDataset->aFileName,alFileName);
  if((fTblcfg = fopen(alFileName,"r")) == NULL)
  {
    if((fTblcfg = fopen(alFileName,"w+")) == NULL)
    {
      FreeDataset(psDataset);
      swVdebug(1,"S0790: [错误/系统调用/%d] fopen创建文件[%s]出错",
                    getpid(),alFileName);
      swVdebug(2,"S0800: [函数返回/%d] ReadMode2,返回码=123",getpid());
      return(123);
    }
  }
	
  while(1)
  {
    if(feof(fTblcfg))break;
    ilRc = GetMode2Record(fTblcfg,psDataset,&slRec);
    Insert(&slRec,psDataset);
    if(feof(fTblcfg))
      break;
  }
  fclose(fTblcfg);
  swVdebug(2,"S0810: [函数返回/%d] ReadMode2,返回码=0",getpid());
  return(0);
}

int WriteMode2(pDataset psDataset,pRecord pRec,short iRecNum,
                                  pDataset pRtnSet,short flag)
{
  int i,j,k,ilFlag1;
  char alLine[4096];
  FILE * fTable;
  Record slRec;
  
  swVdebug(2,"S0820: [函数调用/%d] WriteMode2",getpid());
  if(flag)
  {
/*
    sprintf(alTmp,"cp %s %s.BAK",psDataset->aFileName,psDataset->aFileName);
    system(alTmp);
*/
  
    if((fTable = fopen(psDataset->aFileName,"w+")) == NULL)
    {
      swVdebug(1,"S0830: [错误/系统调用/%d] fopen创建文件[%s]出错",
                      getpid(),psDataset->aFileName);
      swVdebug(2,"S0840: [函数返回/%d] WriteMode2,返回码=123",getpid());
      return(123);
    }
  }
    
  for(i = 0;i < psDataset->iRecordNum;i++)
  {
    for(j =0;j < MAXFLDNUM;j++)
    {
      if(psDataset->Records[i].Values[j].aValue[0] == 0)break;
    }
    InitRecord(&slRec);
    ilFlag1 = 0;
    if(!flag)
    {
      for(k=0;k < iRecNum; k++)
      {
        ilFlag1 = 0;
        for(j = 0;j < psDataset->iFieldNum;j++)
        {
          if(pRec[k].Values[j].aValue[0] !=0)
          {
            if(strcmp(pRec[k].Values[j].aValue,
                psDataset->Records[i].Values[j].aValue) != 0)
            {
              ilFlag1 = 1;
              break;
            }
          }
        }
        if(ilFlag1 == 0)break;
      }
      if(ilFlag1)continue;
    }
    ilFlag1 = 0;
    strcpy(alLine,"[");
    for(j = 0;j < psDataset->iFieldNum;j++)
    {
      if(psDataset->Fields[j].iFieldAttr & 0x2)
      {
        swVdebug(2,"%s=%s",
                      psDataset->Fields[j].aFieldName,
                      psDataset->Records[i].Values[j].aValue);
        if(ilFlag1)
          strcat(alLine,",");
        else
          ilFlag1 = 1;
        strcat(alLine,psDataset->Records[i].Values[j].aValue);
      }
    }
    if(flag)
    {
      strcat(alLine,"]\n");
      if(strcmp(alLine,"[]\n") != 0)
        fputs(alLine,fTable);
    }
    else
    {
      strcat(alLine,"]\n");
      if(strcmp(alLine,"[]") != 0)
      {
        strcpy(slRec.Values[0].aValue,alLine);
        Insert(&slRec,pRtnSet);
      }
    }
    for(j = 0;j < psDataset->iFieldNum;j++)
    {
/*
      if((!(psDataset->Fields[j].iFieldAttr & 0x2)) && \
        (psDataset->Records[i].Values[j].aValue[0] != '\0'))
*/
  /* add by zjj 2001.10.27  */
      if(strcmp(psDataset->Records[i].Values[j].aValue,"-") == 0)continue;
      if(!(psDataset->Fields[j].iFieldAttr & 0x2) )
      {
        sprintf(alLine,"%s=%s",
                      psDataset->Fields[j].aFieldName,
                      psDataset->Records[i].Values[j].aValue);
        if(flag)
        {
          strcat(alLine,"\n");
          fputs(alLine,fTable);
        }
        else
        {
          strcpy(slRec.Values[0].aValue,alLine);
          Insert(&slRec,pRtnSet);
        }
      }
    }
  }
  if(flag)
    fclose(fTable);
  swVdebug(2,"S0850: [函数返回/%d] WriteMode2,返回码=0",getpid());
  return(0);
}

int ExportMode2(pRecord pRec,short iRecNum,pDataset pTset,pDataset pRtnSet)
{
  int ilRc;
  swVdebug(2,"S0860: [函数调用/%d] ExportMode2",getpid());
  ilRc = WriteMode2(pTset,pRec,iRecNum,pRtnSet,0);
  swVdebug(2,"S0870: [函数返回/%d] ExportMode2,返回码=%d",getpid(),ilRc);
  return(ilRc);
}

int ReadMode3G(pDataset psDataset)
{
  char alBuffer[300],alTmpFile[300],alFmtGrp[30],alExt[5],alCmd[64],alPath[151];
  char alTmp1[5];
  int ilRc,n,ilFlag=0;
  FILE *fTblcfg,*fPipe;
  Record slRec;
  
 /* add by zjj 2004.03.08 */
  DIR  *dp;
  struct dirent *dirp;
  char *p;
  char  alFmtgrpflag[MAXFMTGROUP + 1];
  /*del by zcd 20141222
  short ilMaxfmtgroup = 0;
  short ilTmp; 
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long  ilMaxfmtgroup = 0;
  long  ilTmp;
  /*end of add by zcd 20141222*/
   
/*end add by zjj 2004.03.08 */
  
  swVdebug(2,"S0800: [函数调用/%d] ReadMode3G",getpid());
  if(strcmp(psDataset->aTableName,"SWFORMATGRP") == 0)
  {
    strcpy(alExt,"FMT");
    /* modify by nh 20020924
    sprintf(alBuffer,"ls -1 %s/format*.FMT",getenv("SWITCH_CFGDIR"));*/
   /* sprintf(alBuffer,"ls -1 %s/format/ *.FMT",agPath);*/
    sprintf(alBuffer,"%s/format",agPath);
    ilFlag = 1;
  }
  else if(strcmp(psDataset->aTableName,"SWROUTEGRP") == 0)
  {
    strcpy(alExt,"SCR");
    /* modify by nh 20020924
    sprintf(alBuffer,"ls -1 %s/router*.SCR",getenv("SWITCH_CFGDIR")); */
    sprintf(alBuffer,"%s/router",agPath);
    ilFlag = 2; 
 }
  else if(strcmp(psDataset->aTableName,"SWFRONTGRP") == 0)
  {
    strcpy(alExt,"SCR");
    /* modify by nh 20020924
    sprintf(alBuffer,"ls -1 %s/preprocess*.SCR",getenv("SWITCH_CFGDIR"));*/
    sprintf(alBuffer,"%s/preprocess",agPath);
    ilFlag=3;
  }
  memset(alCmd,0x00,sizeof(alCmd));
  strcpy(alPath,alBuffer);

/* modify by zjj 2004.03.08 */
  dp = opendir(alPath);
  if(dp == NULL)
  {
    FreeDataset(psDataset);
    swVdebug(1,"S0790: [错误/系统调用] opendir(%s} 错误!",alPath);
    swVdebug(2,"S0800: [函数返回/%d] ReadMode3G,返回码=124",getpid());
    return(124);
  }
  memset(alFmtgrpflag,0x00,sizeof(alFmtgrpflag));
  while ((dirp = readdir(dp)) != NULL)
  { 
    memset(alTmpFile,0x00,sizeof(alTmpFile));
    if (dirp->d_name[0] == '.') continue;
    strcpy(alTmpFile,dirp->d_name);
    if ((p = strchr(alTmpFile,'.')) == NULL) continue;
    if (strcmp(p + 1,alExt) != 0) continue;  
    ilTmp = atoi(alTmpFile);
    if (ilTmp > MAXFMTGROUP) continue;
    alFmtgrpflag[ilTmp] = '1';  
    ilMaxfmtgroup =(ilMaxfmtgroup > ilTmp)?ilMaxfmtgroup:ilTmp;
  }	
  closedir(dp);
  
  for(ilTmp = 0;ilTmp <= ilMaxfmtgroup;ilTmp++)
  {
    if (alFmtgrpflag[ilTmp] == '\0') continue;
	/*modify by zcd 20141223
    sprintf(alBuffer,"%s/%d.%s",alPath,ilTmp,alExt);
    sprintf(alFmtGrp,"%d",ilTmp);*/
    sprintf(alBuffer,"%s/%ld.%s",alPath,ilTmp,alExt);
    sprintf(alFmtGrp,"%ld",ilTmp);     
    fTblcfg = fopen(alBuffer,"r");
    if(fTblcfg  == NULL)
    {
      FreeDataset(psDataset);
      swVdebug(1,"S0810: [错误/系统调用/%d] fopen打开文件[%s]出错",getpid(),alBuffer);
      swVdebug(2,"S0820: [函数返回/%d] ReadMode3G,返回码=123",getpid());
      return(123);
    }
    if(feof(fTblcfg)) break;
    ilRc =  GetMode2Record(fTblcfg,psDataset,&slRec);
    if(ilRc != 0) break;
    strcpy(slRec.Values[0].aValue,alFmtGrp);
    sprintf(slRec.Values[psDataset->iFieldNum-1].aValue,
                "%s.%s",alFmtGrp,alExt);
    Insert(&slRec,psDataset); 
    fclose(fTblcfg);
  }
  swVdebug(2,"S0830: [函数返回/%d] ReadMode3G,返回码=0",getpid());
  return(0);
/* end add by zjj 2004.03.08 */  
}

int GetMode3FileName(pDataset psDataset,keyrec   *pKeys,\
                                        short     iKeyNum,\
                                        char     *aFileName,\
                                        char     *aGname,\
                                        char     *aDname)
{
  int i;
  /*modify by zcd 201412
  short ilGrp;*/
  long ilGrp;
  char alFmtgrp[31],alBuffer[512];
  char alGrp1[31],alGrp2[31],alGrp3[31];
  
  if(iKeyNum > 1)
  {
    strcpy(alGrp1,"GRPNEW");
    strcpy(alGrp2,"GRPNEW");
    strcpy(alGrp3,"GRPNEW");
  }
  else
  {
    strcpy(alGrp1,"FMT_GROUP");
    strcpy(alGrp2,"ROUTE_GRP");
    strcpy(alGrp3,"PPQID");
  }
  
  alFmtgrp[0] = 0;
  
  for(i =0;i <iKeyNum;i++)
  {
    if((strcmp(pKeys[i].keyname,alGrp1) == 0)      ||
       (strcmp(pKeys[i].keyname,alGrp2) == 0)      ||
       (strcmp(pKeys[i].keyname,alGrp3) == 0))
    {
      strcpy(alFmtgrp,pKeys[i].keyval);
      break;
    }
  }
  if(alFmtgrp[0] == 0)
  { 
    return(136);
  }
  
  ilGrp = atoi(alFmtgrp);
  if(ilGrp <= 0)
    return(136);
  
  if((strcmp(psDataset->aTableName,"SWFORMATGRP") == 0)     ||
     (strcmp(psDataset->aTableName,"SWFORMATD") == 0))
  {
    /* modify by nh 20020924
    sprintf(alBuffer,"%s/format/%s.FMT",getenv("SWITCH_CFGDIR"),alFmtgrp);*/
    sprintf(alBuffer,"%s/format/%s.FMT",agPath,alFmtgrp);
    strcpy(aDname,"SWFORMATD");
    strcpy(aGname,"SWFORMATGRP");
  }
  else if((strcmp(psDataset->aTableName,"SWROUTEGRP") == 0) ||
          (strcmp(psDataset->aTableName,"SWROUTED") == 0))
  {
    /* modify by nh 20020924
    sprintf(alBuffer,"%s/router/%s.SCR",getenv("SWITCH_CFGDIR"),alFmtgrp);*/
    sprintf(alBuffer,"%s/router/%s.SCR",agPath,alFmtgrp);
    strcpy(aDname,"SWROUTED");
    strcpy(aGname,"SWROUTEGRP");
  }
  else if((strcmp(psDataset->aTableName,"SWFRONTGRP") == 0) ||
          (strcmp(psDataset->aTableName,"SWFRONTD") == 0))
  {
    /* modify by nh 20020924
    sprintf(alBuffer,"%s/preprocess/%s.SCR",getenv("SWITCH_CFGDIR"),alFmtgrp);*/
    sprintf(alBuffer,"%s/preprocess/%s.SCR",agPath,alFmtgrp);
    strcpy(aDname,"SWFRONTD");
    strcpy(aGname,"SWFRONTGRP");
  }
  
  strcpy(aFileName,alBuffer);
  return(0);
}

int WriteMode3GF(FILE * fp,pDataset pSet,keyrec *pKeys,short iKeyNum)
{
  short ilFlag,i,j;
  char alLine[512];
    
  swVdebug(2,"S0940: [函数调用/%d] WriteMode3GF",getpid());
  for(i = 0;i < pSet->iRecordNum;i++)
  {
    ilFlag = 0;
    for(j = 0;j < pSet->iFieldNum;j++)
    {
      if(strcmp(pKeys[0].keyname,pSet->Fields[j].aFieldName) == 0)
      {
        if(iKeyNum > 1)
          strcpy(alLine,pKeys[1].keyval);
        else
          strcpy(alLine,pKeys[0].keyval);
        if(strcmp(alLine,pSet->Records[i].Values[j].aValue)
                 == 0)
          ilFlag = 1;
        else
          ilFlag = 0;
        break;
      }
    }
    
    if(ilFlag)break;
  }
  if(!ilFlag)
  {
    swVdebug(2,"S0950: [函数返回/%s] WriteMode3GF,返回码=131(格式转换组未找到)",
                       getpid());
    return(131);    
  }
 
  for(j = 0;j < pSet->iFieldNum;j++)
  {
    if ((!(pSet->Fields[j].iFieldAttr & 0x2)) && \
/*       delete by lcw
        (pSet->Records[i].Values[j].aValue[0] != '\0') && 
*/
        (strcmp(pSet->Fields[j].aFieldName,"FILENAME") !=0) &&
        (strcmp(pSet->Records[i].Values[j].aValue,"-") !=0))
    {
      sprintf(alLine,"%s=%s\n",
                      pSet->Fields[j].aFieldName,
                      pSet->Records[i].Values[j].aValue);
      fputs(alLine,fp);
    }
  }
  swVdebug(2,"S0960: [函数返回/%d] WriteMode3GF,返回码=0",getpid());
  return(0);
}

int WriteMode3DF(FILE *fp,pDataset pSet)
{
  short i;
 
  swVdebug(2,"S0970: [函数调用/%d] WriteMode3DF",getpid()); 
  if(strcmp(pSet->aTableName,"SWFORMATD") == 0)
    fputs("[TDF]\n",fp);
  else if(strcmp(pSet->aTableName,"SWROUTED") == 0)
    fputs("[ROUTE]\n",fp);
  
  for(i = 0;i < pSet->iRecordNum;i++)
  {
    fputs(pSet->Records[i].Values[0].aValue,fp);
    fputs("\n",fp);
  }
  swVdebug(2,"S0980: [函数返回/%d] WriteMode3DF,返回码=0",getpid());
  return(0);
}

int ReadMode3D(keyrec *pKeys,short iKeyNum,pDataset psDataset)
{
  int i,ilFlag = 0,ilRc;
  char alLine[512],alFileName[300],alDname[31],alGname[31];
  FILE * fTable;
  Record slRec;
  
  swVdebug(2,"S0990: [函数调用/%d] ReadMode3D",getpid());
  
  ilRc = GetMode3FileName(psDataset,pKeys,1,alFileName,alGname,alDname);
  if(ilRc)return(ilRc);
  
  if((fTable = fopen(alFileName,"r")) == NULL)
  if((fTable = fopen(alFileName,"w+")) == NULL)
  {
    swVdebug(1,"S1000: [错误/系统调用/%d] fopen创建文件[%s]出错!",
            getpid(),alFileName);
    swVdebug(2,"S1010: [函数返回/%d] ReadMode3D,返回码=123",getpid());
    return(123);
  }
    
  i = 0;
  
  while(fgets(alLine,250,fTable) != NULL)
  {
    _swTrim(alLine);
    if(ilFlag == 0)
    {
      if((strcmp(alLine,"[TDF]") == 0) ||
         (strcmp(alLine,"[ROUTE]") == 0))
      {
        ilFlag = 1;
        continue;
      }
      if(alLine[0] == '[')
        ilFlag = 1;
    }
    if(ilFlag == 0)continue;
    InitRecord(&slRec);
    strcpy(slRec.Values[0].aValue,alLine);
    Insert(&slRec,psDataset);
  }
  swVdebug(2,"S1020: [函数返回/%d] ReadMode3D,返回码=0",getpid());
  fclose(fTable);   /* add by zjj 2004.03.08 */
  return(0);
}

int WriteMode3G(pDataset psDataset,keyrec *pKeys,short iKeyNum)
{
  int ilRc;
  FILE * fTable;
  Dataset slSet;
  char alGname[31],alDname[31],alFileName[300];/*,alTmp[50];*/
  
  swVdebug(2,"S1030: [函数调用/%d] WriteMode3G",getpid());
  ilRc = GetMode3FileName(psDataset,pKeys,1,alFileName,alGname,alDname);
  if(ilRc)return(ilRc);

  InitDataset(alDname,&slSet);
  ReadMode3D(pKeys,iKeyNum,(pDataset)&slSet);
  
 /* 
  sprintf(alTmp,"mv %s %s.BAK",alFileName,alFileName);
  system(alTmp);
 */
 
  if(iKeyNum > 1)
  {
    if((fTable = fopen(alFileName,"r")) != NULL)
    {
      fclose(fTable);
      swVdebug(2,"S1040: [函数返回/%d] WriteMode3G,返回码=135(文件已经存在)",
              getpid());
      return(135);
    }
  }
  ilRc = GetMode3FileName(psDataset,pKeys,iKeyNum,alFileName,alGname,alDname);
  if(ilRc)
  {
    swVdebug(2,"S1050: [函数返回/%d] WriteMode3G,返回码=%d",
        getpid(),ilRc);
    return(ilRc);
  }
  
  if((fTable = fopen(alFileName,"w+")) == NULL)
  {
    swVdebug(2,"S1060: [错误/%d] fopen打开文件[%s]出错!",
         getpid(),alFileName);
    swVdebug(2,"S1070: [函数返回/%d] WriteMode3G,返回码=123",getpid()); 
    return(123);
  }
    
  WriteMode3GF(fTable,psDataset,pKeys,iKeyNum);
  WriteMode3DF(fTable,&slSet);
  FreeDataset(&slSet);
  fclose(fTable);
  swVdebug(2,"S1080: [函数返回/%d] WriteMode3G,返回码=0",getpid());
  return(0);
}

int ReadMode3M(pDataset psDataset)
{
  int ilRc;
  
  swVdebug(2,"S1090: [函数调用/%d] ReadMode3M",getpid());
  ilRc = ReadMode1(psDataset);
  swVdebug(2,"S1100: [函数返回/%d] ReadMode3M,返回码=%d",
             getpid(),ilRc);
  return(ilRc);
}

int WriteMode3M(pDataset psDataset)
{
  int ilRc;
  
  swVdebug(2,"S1110: [函数调用/%d] WriteMode3M",getpid());
  ilRc = WriteMode1(psDataset,NULL,0,NULL,1);
  swVdebug(2,"S1120: [函数返回/%d],WriteMode3M,返回值=%d",
         getpid(),ilRc);
  return(ilRc);
}

int WriteMode3D(pDataset pSet,keyrec *pKeys,short iKeyNum)
{
  int ilRc;
  FILE * fTable;
  Dataset slSet;
  char alGname[31],alDname[31],alFileName[300];/*,alTmp[50];*/
  
  swVdebug(2,"S1130: [函数调用/%d] WriteMode3D",getpid());
  ilRc = GetMode3FileName(pSet,pKeys,iKeyNum,alFileName,alGname,alDname);
  if(ilRc)
  {
    swVdebug(2,"S1140: [错误/其他函数/%d] GetMode3FileName 返回码=%d",
            getpid(),ilRc);
    swVdebug(2,"S1150: [函数返回/%d] WriteMode3D,返回码=%d",
            getpid(),ilRc);
    return(ilRc);
  }
/*  
  sprintf(alTmp,"cp %s %s.BAK",alFileName,alFileName);
  system(alTmp);
 */ 

  InitDataset(alGname,&slSet);
  ReadMode3G(&slSet);
  
  ilRc = GetMode3FileName(pSet,pKeys,iKeyNum,alFileName,alGname,alDname);
  ilRc = GetMode3FileName(pSet,pKeys,iKeyNum,alFileName,alGname,alDname);
  if(ilRc)
  {
    swVdebug(2,"S1160: [错误/其他函数/%d] GetMode3FileName 返回码=%d",
            getpid(),ilRc);
    swVdebug(2,"S1170: [函数返回/%d] WriteMode3D,返回码=%d",
            getpid(),ilRc);
    return(ilRc);
  }
  if(iKeyNum > 1)
  {
    if((fTable = fopen(alFileName,"r")) != NULL)
    {
      fclose(fTable);
      swVdebug(2,"S1180: [函数返回/%d] WriteMode3D,返回码=135(文件已经存在)",
              getpid());
      return(135);
    }
  }

  if((fTable = fopen(alFileName,"w+")) == NULL)
  {
    swVdebug(2,"S1190: [错误/%d] fopen打开文件[%s]出错!",
         getpid(),alFileName);
    swVdebug(2,"S1200: [函数返回/%d] WriteMode3D,返回码=123",getpid()); 
    return(123);
  }
    
  WriteMode3GF(fTable,&slSet,pKeys,iKeyNum);
  WriteMode3DF(fTable,pSet);
  FreeDataset(&slSet);
  fclose(fTable);
  swVdebug(2,"S1210: [函数返回/%d] WriteMode3D,返回码=0",
      getpid());
  return(0);
}

int Insert(pRecord pRec,pDataset pSet)
{
  pSet->iRecordNum++;
  
  if(pSet->iRecordNum == 1)
    pSet->Records = (pRecord)malloc(
                          sizeof(Record));
  else
    pSet->Records = (pRecord)realloc(pSet->Records,
                 pSet->iRecordNum * sizeof(Record));
  memcpy(&pSet->Records[pSet->iRecordNum-1],pRec,sizeof(Record));
  return(0);
}

extern char **environ;

int ReadMode4(pDataset pSet)
{
  char **pp,alBuf[1024],*p,*p1;
  Record slRec; 

  swVdebug(2,"S1220: [函数调用/%d] ReadMode4",getpid());

  pp = environ;
  while(*pp)    
  {              
    InitRecord(&slRec);
    strcpy(alBuf,*pp);
    p = alBuf;
    pp++;    
    p1 = strchr(p,'=');
    if(p1 == NULL)continue; 
    *p1 = 0;
    p1++; 
    strcpy(slRec.Values[0].aValue,p);
    strcpy(slRec.Values[1].aValue,p1);
    Insert(&slRec,pSet);
  } 
  swVdebug(2,"S1230: [函数返回/%d] ReadMode4,返回码=0",getpid());
  return(0);
} 

int WriteMode4(pDataset pSet)
{
  short i;
  char alPutenvStr[512];
  
  swVdebug(2,"S1240: [函数调用/%d] WriteMode4",getpid());
  for(i = 0;i < pSet->iRecordNum;i++)
  {
    sprintf(alPutenvStr,"%s=%s",
           pSet->Records[i].Values[0].aValue,
           pSet->Records[i].Values[1].aValue);
    putenv(alPutenvStr);
  }
  swVdebug(2,"S1250: [函数返回/%d] WriteMode4,返回码=0",getpid());
  return(0);
}

int ReadMode5(pDataset pSet)
{
  char alCatalog[512],alBuf[1024],alLine[1024],*p,*p1,*p2;
  Record slRec;
  FILE *fp;
  short j;
  
  swVdebug(2,"S1260: [函数调用/%d] ReadMode5",getpid());

  /* modify by nh 20020924
  sprintf(pSet->aFileName,"%s/config/%s",getenv("SWITCH_CFGDIR"),pSet->aTableName); */
  sprintf(pSet->aFileName,"%s/config/%s",agPath,pSet->aTableName); 

  if((fp = fopen(pSet->aFileName,"r")) == NULL)
    if((fp = fopen(pSet->aFileName,"w+")) == NULL)
    {
      swVdebug(2,"S1270: [错误/系统调用/%d] fopen创建文件[%s]失败",
             getpid(),pSet->aFileName);
      swVdebug(2,"S1280: [函数返回/%d] ReadMode5,返回码=123",getpid());
      return(123);
    }
  alCatalog[0] = 0;

  while(fgets(alLine,512,fp) != NULL)
  {
    InitRecord(&slRec);
    strcpy(alBuf,alLine);
    _swTrim(alBuf);
    if(alBuf[0] == 0)continue;
    if(alBuf[0] == '#')continue;
    p = strchr(alBuf,'#');
    if(p != NULL)*p = 0;
    if(alBuf[0] == '[')
    {
      p = &alBuf[1];
      p1 = strchr(p,']');
      if(p1 != NULL)*p1 = 0;
      _swTrim(p);
      if(strcmp(alCatalog,p) !=0)
      {
        strcpy(slRec.Values[0].aValue,p);
        slRec.Values[1].aValue[0] = 0;
        Insert(&slRec,pSet);
        strcpy(alCatalog,p);
      }
      continue;
    }
    for(j = 0;j <MAXFLDNUM;j++)
      slRec.Values[j].aValue[0] = 0;
    strcpy(slRec.Values[0].aValue,alCatalog);
    p = alBuf;
    p1 = strchr(p,' ');
    p2 = strchr(p,'	');
    if(p1 == NULL)p1 = p2;
    else
    if((p2 != NULL ) && (p2 < p1)) p1 = p2;
    while(p1 != NULL)
    {
       *p1 = 0;
      _swTrim(p);  
      if(p[0] == 0)continue;
      strcpy(slRec.Values[1].aValue,p);
      Insert(&slRec,pSet);
      p = p1+1;
      _swTrim(p);
      p1 = strchr(p,' ');
      p2 = strchr(p,'	');
      if(p1 == NULL)p1 = p2;
      else
        if((p2 != NULL ) && (p2 < p1)) p1 = p2;
    }
    if(p[0] == 0)continue;
    strcpy(slRec.Values[1].aValue,p);
    Insert(&slRec,pSet);
  }
  
/*
  for(i=0;i<pSet->iRecordNum;i++)
  {
      pSet->Records[i].Values[0].aValue,
      pSet->Records[i].Values[1].aValue);
  }
*/
  fclose(fp);
  swVdebug(2,"S1290: [函数返回/%d] ReadMode5,返回码=0",getpid());
  return(0);
}

int WriteMode5(pDataset pSet)
{
  short i,ilLen,ilFlag = 0;
  char alLine[1024],*p,alCatalog[512],alFileName[300];
  pRecord pRec;
  FILE * fp;

  swVdebug(2,"S1300: [函数调用/%d] WriteMode5",getpid());
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/%s",getenv("SWITCH_CFGDIR"),pSet->aTableName);*/
  sprintf(alFileName,"%s/config/%s",agPath,pSet->aTableName);
 
/*
  sprintf(alCmd,"mv %s %s.BAK",alFileName,alFileName);
  system(alCmd);
*/
  if((fp = fopen(alFileName,"w+")) == NULL)
  {
    swVdebug(2,"S1310: [错误/系统调用/%d] fopen创建文件[%s]出错",
             getpid(),alFileName);
    swVdebug(2,"S1320: [函数返回/%d] WriteMode5,返回码=123",getpid());
    return(123);
  }

  alCatalog[0] = 0;
  alLine[0] = 0;
  p = alLine;

  for(i = 0;i < pSet->iRecordNum;i++)
  {
    pRec = &pSet->Records[i]; 
    if(strcmp(pRec->Values[0].aValue,alCatalog) != 0)
    {
      if(alCatalog[0] != 0 && ilFlag == 1)  /* add by zjj 2001.10.28 */
      {
        strcat(alLine,"\n");
        fputs(alLine,fp);
      }
      strcpy(alCatalog,pRec->Values[0].aValue);
      sprintf(alLine,"[%s]\n",alCatalog);
      fputs(alLine,fp);
      p = alLine;
      ilLen = 0;
      ilFlag = 0;
    }
    if(pRec->Values[1].aValue[0] != 0)
    {
      strcpy(p,pRec->Values[1].aValue);
      strcat(p," ");
      ilLen += strlen(p);
      p += strlen(p);
      ilFlag = 1;
    }
    if(ilLen > 75)
    {
       strcat(alLine,"\n");
       fputs(alLine,fp);
       p = alLine;
       alLine[0] = 0;
       ilLen = 0;
       ilFlag = 0;
    } 
  }
  if(ilFlag)
  {
    strcat(alLine,"\n");
    fputs(alLine,fp);
  }
  fclose(fp);
  swVdebug(2,"S1330: [函数返回/%d] WriteMode5,返回码=0",getpid());
  return(0);
}

int ReadMode6(keyrec * pKeys,short iKeyNum,pDataset pSet)
{
  FILE *fp;
  char alFileName[300],alBuf[4096],alLine[100],*p;
  unsigned char *pa,*ph;
  short ilPos,ilLen,ilReadLen,ilCurlen,i,j,k;
  Record slRec;
  
  swVdebug(2,"S1340: [函数调用/%d] ReadMode6",getpid());
  
  if(iKeyNum < 3)
  {
    swVdebug(2,"S1350: [函数返回] ReadMode6,返回码=157",getpid());
    return(157);
  }
  for(i = 0;i <iKeyNum;i++)
  {
    if(strcmp(pKeys[i].keyname,"filename") == 0)
    {
      sprintf(alFileName,"%s/%s",getenv("SWITCH_LOG_PATH"),pKeys[i].keyval);
    }
    if(strcmp(pKeys[i].keyname,"filepos") == 0)
    {
      ilPos = atoi(pKeys[i].keyval);
    }
    if(strcmp(pKeys[i].keyname,"length") == 0)
    {
      ilLen = atoi(pKeys[i].keyval);
    }
  } 
  if((fp = fopen(alFileName,"r")) == NULL)
  {
    swVdebug(2,"S1360: [函数返回/%d] ReadMode6,返回码=164(打开文件%s出错)",
          getpid(),alFileName);
    return(164);
  }

  fseek(fp,ilPos,SEEK_SET);

  while(ilLen > 0)
  {
    if(ilLen >= 4096)
      ilReadLen = 4096;
    else
      ilReadLen = ilLen;
    ilCurlen = fread(alBuf,1,ilReadLen,fp);
    swVdebug(3,"S1370: ilCurlen = [%d]",ilCurlen);
    if(ilCurlen <= 0)break;
    pa = (unsigned char *)alBuf;
    ph = (unsigned char *)alBuf;
    for(i = 0,j = ilCurlen / 16;i < j;i++)
    {
      p = alLine;
      sprintf(p, "M(%6.6d)=< ",i*16);
      p += strlen(p);
      for(k=0; k<16; k++) 
      {
        sprintf(p, "%02x ",*ph++);
        p += strlen(p);
      }
      sprintf(p,"> ");
      p += strlen(p);
      for(k=0; k<16; k++, pa++)
      {
        sprintf(p, "%c",(*pa>32) ? ((*pa<128) ? *pa: '*'):'.'); 
        p += strlen(p);
      }
      InitRecord(&slRec);
      strcpy(slRec.Values[0].aValue,alLine);
      Insert(&slRec,pSet);
    }
    if((i=ilCurlen%16) > 0)
    {
      p = alLine;
      sprintf(p,"M(%6.6d)=< ",ilCurlen-ilCurlen%16);
      p += strlen(p);
      for(k=0; k < i; k++)
      {
        sprintf(p, "%02x ",*ph++);
        p += strlen(p);
      }
      for(k=i; k < 16; k++)
      {
        sprintf(p, "   ");
        p += strlen(p); 
      }
      sprintf(p, "> ");
      p += strlen(p);
      for(k=0; k < i; k++, pa++)
      {
        sprintf(p, "%c",(*pa>32) ? ((*pa<128) ? *pa: '*'):'.'); 
        p += strlen(p);
      }
      InitRecord(&slRec);
      strcpy(slRec.Values[0].aValue,alLine);
      Insert(&slRec,pSet);
    }
    ilLen -= ilCurlen;
  } 
  fclose(fp);
  swVdebug(2,"S1380: [函数返回/%d] ReadMode6,返回码=0",getpid());
  return(0);
}

int WriteMode6(pDataset pSet)
{
  return(199);
}

int Delete(keyrec * pKeys,short iKeyNum,pDataset pSet)
{
  int i,j,k;
  short ilFlag;
  short ilFindflag = 0;    /*   add by zjj 2001.10.25   */

  swVdebug(2,"S1390: [函数调用] Delete");
  swDebugkeys(2,pKeys,iKeyNum);
  for(i = 0;i < pSet->iRecordNum;i++)
  {
    ilFlag = 0;
    for(k = 0; k < iKeyNum;k++)
    {
      for(j = 0;j < pSet->iFieldNum;j++)
      {
        if(strcmp(pKeys[k].keyname,pSet->Fields[j].aFieldName) == 0)
        {
          if(strcmp(pKeys[k].keyval,pSet->Records[i].Values[j].aValue)
                           == 0)
            ilFlag = 1;
          else
            ilFlag = 0;
          break;
        }
      }
      if(ilFlag == 0)
        break;
    }
    if (ilFlag) ilFindflag = 1;    /*  add by zjj 2001.10.25   */
/* delete by zjj 2001.10.25    */
/*    if(ilFlag)break;    
  } 
*/     

/*    delete by zjj 2001.10.25    */
/*  if(i == pSet->iRecordNum)     */
/*    return(121);                */
/*   end delete by zjj            */
  if (ilFlag)
  {
    for(j = i;j < pSet->iRecordNum - 1;j++)
    {
      memcpy(&pSet->Records[j],&pSet->Records[j+1],sizeof(Record));
    } 
    if(pSet->iRecordNum == 1)
    {
      free(pSet->Records);
    }
    i--;
    pSet->iRecordNum--;
  }
}
  if (ilFindflag == 0)
  {
    swVdebug(2,"S1400: [函数返回] Delete,返回码=121");
    return(121);
  }
  swVdebug(2,"S1410: [函数返回] Delete,返回码=0");
  return(0);
}


int UpdateAll(keyrec * pKeys,short iKeyNum,pRecord pRec,pDataset pSet)
{
   int i,j,k;
  short ilFlag,ilFlag1 = 0;

  swVdebug(2,"S1420: [函数调用] UpdateAll");
  swDebugkeys(2,pKeys,iKeyNum);
  swDebugrecs(2,pRec,1);

  for(i = 0;i < pSet->iRecordNum;i++)
  {
    ilFlag = 0;
    for(k = 0; k < iKeyNum;k++)
    {
      for(j = 0;j < pSet->iFieldNum;j++)
      {
        if(strcmp(pKeys[k].keyname,pSet->Fields[j].aFieldName) == 0)
        {
          if(strcmp(pKeys[k].keyval,pSet->Records[i].Values[j].aValue)
                   == 0)
            ilFlag = 1;
          else
            ilFlag = 0;
          break;
        }
      }
      if(ilFlag == 0)
        break;
    }

    if(!ilFlag)continue;
    ilFlag1 = 1;
    for(j = 0;j < pSet->iFieldNum;j++)
    {
      if((pSet->iStoreMode == MODE3G) && (j == 0))continue;
      if(strcmp(pRec->Values[j].aValue,"-") != 0)
        strcpy(pSet->Records[i].Values[j].aValue,
            pRec->Values[j].aValue);
    }
  }
  if(!ilFlag1)
  {
    swVdebug(2,"S1430: [函数返回] UpdateAll,返回码=121");
    return(121);
  }
  
  swVdebug(2,"S1440: [函数返回] UpdateAll,返回码=0");

  return(0);
}

int Update(keyrec * pKeys,short iKeyNum,pRecord pRec,pDataset pSet)
{
   int i,j,k;
  short ilFlag;

  swVdebug(3,"S1450: iKeyNum = [%d]",iKeyNum);
  for(i = 0;i < pSet->iRecordNum;i++)
  {
    ilFlag = 0;
    for(k = 0; k < iKeyNum;k++)
    {
      for(j = 0;j < pSet->iFieldNum;j++)
      {
        if(strcmp(pKeys[k].keyname,pSet->Fields[j].aFieldName) == 0)
        {
          if(strcmp(pKeys[k].keyval,pSet->Records[i].Values[j].aValue)
                   == 0)
            ilFlag = 1;
          else
            ilFlag = 0;
          break;
        }
      }
      if(ilFlag == 0)
        break;
    }

    if(ilFlag)break;
  }
  if(i == pSet->iRecordNum)
    return(121);
   for(j = 0;j < pSet->iFieldNum;j++)
   {
/*
     if(pRec->Values[j].aValue[0] != 0)
*/
       strcpy(pSet->Records[i].Values[j].aValue,
            pRec->Values[j].aValue);
   }
  return(0);
}

int CompareRec(pRecord pRec1,pRecord pRec2,pDataset pSet)
{
  short i,ilRec1,ilRec2;
  int ilRc;

  for(i = 0;i < pSet->iFieldNum;i++)
  {
    if(pSet->Fields[i].iFieldAttr > 1)
    {
      if(pSet->Fields[i].iFieldType == T_SHORT)
      {
        ilRec1 = atoi(pRec1->Values[i].aValue);
        ilRec2 = atoi(pRec2->Values[i].aValue);
        if(ilRec1 < ilRec2)return(1);
        else if(ilRec1 > ilRec2)return(2);
      }
      else
      {
        ilRc = strcmp(pRec1->Values[i].aValue,pRec2->Values[i].aValue);
        if(ilRc == 0)continue;
        if(ilRc > 0)return(2);
        if(ilRc < 0)return(1); 
      }
    } 
  }
  return(0);
}

void SwapRec(pDataset pSet,short Index1,short Index2)
{
  Record slRec;

  memcpy(&slRec,&pSet->Records[Index1],sizeof(Record));
  memcpy(&pSet->Records[Index1],&pSet->Records[Index2],
            sizeof(Record));
  memcpy(&pSet->Records[Index2],&slRec,sizeof(Record)); 
}

void SortSet(pDataset pSet,short num1,short num2)
{
  int i,j;
  short ilNum,ilNum1;
  Record slRec;

  swVdebug(2,"S1460: [函数调用] SortSet,num1 = %d,num2 = %d",num1,num2);
  if( (num1 <0) || (num2 >= pSet->iRecordNum))return;
  ilNum = (num2 - num1) / 2;  
  ilNum1 = num1 + ilNum;
 
  if(ilNum > 1)
  {
    memcpy(&slRec,&pSet->Records[ilNum1],sizeof(Record)); 
  
    i = num1;j = num2 + 1;

    memcpy(&pSet->Records[ilNum1],&pSet->Records[num1],sizeof(Record)); 
    memcpy(&pSet->Records[num1],&slRec,sizeof(Record)); 

    while(1)
    {
      while(CompareRec(&pSet->Records[++i],&slRec,pSet) == 1)
      {
        if(i >= num2)break;
      }
      while(CompareRec(&pSet->Records[--j],&slRec,pSet) == 2);
      {
        if(j <= num1)break; 
      }
      if(i >= j)break;
      SwapRec(pSet,i,j);
    }
    memcpy(&pSet->Records[num1],&pSet->Records[j],sizeof(Record)); 
    memcpy(&pSet->Records[j],&slRec,sizeof(Record)); 
    SortSet(pSet,num1,j);
    SortSet(pSet,j+1,num2); 
  }
  swVdebug(2,"S1470: [函数返回] SortSet,返回码=无");
}

int OpenTable(keyrec *pKeys,short iKeyNum,char *aTableName,pDataset pSet)
{
  int ilRc;
  
  swVdebug(2,"S1480: [函数调用/%d] OpenTable",getpid());
  swDebugkeys(2,pKeys,iKeyNum);
  ilRc = InitDataset(aTableName,pSet);
  if(ilRc)
  {
    return(ilRc);
  } 
  switch(pSet->iStoreMode)
  {
    case MODE1:
      ilRc = ReadMode1(pSet);
      break;
    case MODE2:
      ilRc = ReadMode2(pSet);
      break;
    case MODE3G:
      ilRc = ReadMode3G(pSet);
      break;
    case MODE3M:
      ilRc = ReadMode3M(pSet);
      break;
    case MODE3D:
      ilRc = ReadMode3D(pKeys,iKeyNum,pSet);
      break;
    case MODE4:
      ilRc = ReadMode4(pSet);
      break;
    case MODE5:
      ilRc = ReadMode5(pSet);
      break;
    case MODE6:
      ilRc = ReadMode6(pKeys,iKeyNum,pSet);
      break;
    case MODE7:
      ilRc = ReadMode7(pSet);
      break;
    case MODE8:
      ilRc = ReadMode8(pSet);
      break;
    case MODE9:
      ilRc = ReadMode9(pSet);
      break;
    case MODE10:
      ilRc = ReadMode10(pSet);
      break;
    default:
      swVdebug(2,"S1490: [函数返回/%d] OpenTable,返回码=128",getpid());
      return(128);  
  }
  
 /*
  if((!ilRc) && 
     (pSet->iStoreMode != MODE3D)    &&
     (pSet->iStoreMode != MODE6))
    SortSet(pSet,0,pSet->iRecordNum - 1);
  swVdebug(3,"S1500: ilRc = [%d]",ilRc);
 */
  /* add by zjj 2001.10.27  */
  if ((!ilRc) && (pSet->iStoreMode == MODE5))  SortSet(pSet,0,pSet->iRecordNum - 1); 
  swVdebug(2,"S1510: [函数返回/%d] OpenTable,返回码=%d",getpid(),ilRc);
  return(ilRc);
}

int WriteTable(keyrec *pKeys,short iKeyNum,char *aTableName,pDataset pSet)
{
  int ilRc;
  
  swVdebug(2,"S1520: [函数调用/%d] WriteTable",getpid());
  swDebugkeys(2,pKeys,iKeyNum);
  switch(pSet->iStoreMode)
  {
    case MODE1:
      ilRc = WriteMode1(pSet,NULL,0,NULL,1);
      break;
    case MODE2:
      ilRc = WriteMode2(pSet,NULL,0,NULL,1);
      break;
    case MODE3G:
      ilRc = WriteMode3G(pSet,pKeys,iKeyNum);
      break;
    case MODE3M:
      ilRc = WriteMode3M(pSet);
      break;
    case MODE3D:
      ilRc = WriteMode3D(pSet,pKeys,iKeyNum);
      break;
    case MODE4:
      ilRc = WriteMode4(pSet);
      break;
    case MODE5:
      ilRc = WriteMode5(pSet);
      break;
    case MODE6:
      ilRc = WriteMode6(pSet);
      break;
    case MODE7:
      ilRc = WriteMode7(pSet);
      break;
    case MODE8:
      ilRc = WriteMode8(pSet);
      break;
    case MODE10:
      ilRc = WriteMode10(pSet);
      break;
    default:
      swVdebug(2,"S1530: [函数返回/%d] WriteTable,返回码=128",getpid());
      return(128); 
  }
  swVdebug(2,"S1540: [函数返回/%d] WriteTable,返回码=%d",getpid(),ilRc);
  return(ilRc);
}

int ImportTable(char * aTableName,keyrec *pKeys,short iKeyNum,pDataset pSet)
{
  int ilRc;
  Dataset slSet;
  
  swVdebug(2,"S1550: [函数调用/%d] ImportTable",getpid());
  swDebugkeys(2,pKeys,iKeyNum);
  
  ilRc = InitDataset(aTableName,&slSet);
  if(ilRc)
  {
    swVdebug(1,"S1560: [错误/其他函数/%d] InitDataset,返回码=%d",
           getpid(),ilRc);
    swVdebug(2,"S1570: [函数返回/%d] ImportTable,返回码=%d",
               getpid(),ilRc);
    return(ilRc);
  } 

  switch(slSet.iStoreMode)
  {
    case MODE1:
      ilRc = ImportMode1(&slSet,pSet);
      break;
    case MODE2:
      ilRc = ImportMode2(&slSet,pSet);
      break;
    case MODE3G:
      ilRc = ImportMode3G(pKeys,iKeyNum,&slSet,pSet);
      break;
    case MODE3M:
      ilRc = ImportMode3M(&slSet,pSet);
      break;
    default:
      swDebug(1,"S0000: [函数返回/%d] ImportTable,返回码=170",getpid());
      ilRc = 170;
      break;
  }
  swVdebug(2,"S1580: [函数返回/%d] ImportTable 返回码=%d",getpid(),ilRc); 
  return(ilRc);
}

int  ExportTable(keyrec *pKeys,short iKeyNum,pRecord pRec,short iRecNum
          ,pDataset pSet,pDataset pRtnSet)
{
  int ilRc;

  swVdebug(2,"S1590: [函数调用/%d] ExportTable",getpid());
  swDebugkeys(2,pKeys,iKeyNum);
  
  switch(pSet->iStoreMode)
  {
    case MODE1:
      ilRc = ExportMode1(pRec,iRecNum,pSet,pRtnSet);
      break;
    case MODE2:
      ilRc = ExportMode2(pRec,iRecNum,pSet,pRtnSet);
      break;
    case MODE3G:
      ilRc = ExportMode3G(pKeys,iKeyNum,pRec,iRecNum,pSet,pRtnSet);
      break;
    case MODE3M:
      ilRc = ExportMode3M(pRec,iRecNum,pSet,pRtnSet);
      break;
    default:
      swVdebug(2,"S1600: [函数返回/%d] ExportTable 返回码=170",getpid()); 
      ilRc = 170;
  }
  swVdebug(2,"S1610: [函数返回/%d] ExportTable 返回码=%d",getpid(),ilRc); 
  return(ilRc);
}

int swFilelock_del(char *filename,char *ipaddr,int forceflag)
{
  FILE *fs,*flTmp;
  char linebuf[512];
  char alFilename[300],alIpaddr[20],alTime[80],alLockfile[300];
  char alTmpfile[300];
  int ilFlag = 0,ilTmp = 0;

  swVdebug(2,"S1620: [函数调用] swFilelock_del");
  /* modify by nh 20020924
  sprintf(alLockfile,"%s/config/%s",getenv("SWITCH_CFGDIR"),LOCKFILE);*/
  sprintf(alLockfile,"%s/config/%s",agPath,LOCKFILE);

  if((fs = fopen(alLockfile,"r")) == NULL)
    if((fs = fopen(alLockfile,"w+")) == NULL)
    {
      swVdebug(1,"S1630: [错误/系统调用] fopen创建文件%s出错!",
             alLockfile);
      swVdebug(2,"S1640: [函数返回] swFilelock_del,返回码=130");
      return(130);
    }
  /* modify by nh 20020924
  sprintf(alTmpfile,"%s/%s",getenv("SWITCH_CFGDIR"),TMPFILE);*/
  sprintf(alTmpfile,"%s/%s",agPath,TMPFILE);
  if((flTmp = fopen(alTmpfile,"w+")) == NULL)
  {
    fclose(fs);
    swVdebug(1,"S1650: [错误/系统调用] fopen创建文件%s出错!",
           alTmpfile);
    swVdebug(2,"S1660: [函数返回] swFilelock_del,返回码=129");
    return(129);
  }
 
  while(fgets(linebuf,299,fs) != NULL)
  {
    _swTrim(linebuf);
    if(linebuf[0] != '#')
    {
      sscanf(linebuf,"%s %s %s",
        alFilename,
        alIpaddr,alTime);
      if(strcmp(alFilename,filename) == 0)
      {
        ilTmp = 1;
        if(!forceflag)
        {
          if(strcmp(alIpaddr,ipaddr) == 0)
          {
            ilFlag = 1;
            continue;
          }
        }
        else
        {
          ilFlag = 1;
          continue;
        }
      }
    }
    strcat(linebuf,"\n");
    fputs(linebuf,flTmp);
  }
  fclose(fs);
  fclose(flTmp);
  if(!ilTmp)
  {
/*
    swVdebug(2,"S1480: [函数返回] swFilelock_del,返回码=155");
    return(155); 
*/   /*delete by zjj 2004.03.08  */
    return(0);  /*added by zjj 2004.03.08 */
  }
  if(!ilFlag)
  {
    swVdebug(2,"S1680: [函数返回] swFilelock_del,返回码=131");
    return(131);
  }
  sprintf(linebuf,"mv %s %s",alTmpfile,alLockfile);
  system(linebuf);
  swVdebug(2,"S1690: [函数返回] swFilelock_del,返回码=0");
  return(0);
}

int swFilelock_add(char *filename,char *ipaddr,int forceflag)
{
  FILE *fs,*flTmp;
  char linebuf[512];
  char alFilename[300],alIpaddr[20],alTime[80],alLockfile[300];
  char alTmpfile[300];

  swVdebug(2,"S1700: [函数调用] swFilelock_add");

  if((filename[0] == '\0') || (ipaddr[0] == '\0'))
  {
    swVdebug(2,"S1710: [函数返回] swFilelock_add,返回码=132");
    return(132);
  }

  /* modify by nh 20020924
  sprintf(alLockfile,"%s/config/%s",getenv("SWITCH_CFGDIR"),LOCKFILE);*/
  sprintf(alLockfile,"%s/config/%s",agPath,LOCKFILE);

  if((fs = fopen(alLockfile,"r")) == NULL)
  {
    /* add by zjj 2001.10.28  */
    if ((fs = fopen(alLockfile,"w+")) == NULL)
    /* end add by zjj 2001.10.28  */
    {
      swVdebug(2,"S1720: [函数返回] swFilelock_add,返回码=130");
      return(130);
    }
  }
  /* modify by nh 20020924
  sprintf(alTmpfile,"%s/config/%s",getenv("SWITCH_CFGDIR"),TMPFILE);*/
  sprintf(alTmpfile,"%s/config/%s",agPath,TMPFILE);

  if((flTmp = fopen(alTmpfile,"w+")) == NULL)
  {
    fclose(fs);
    swVdebug(2,"S1730: [函数返回] swFilelock_add,返回码=129");
    return(129);
  }
 
  while(fgets(linebuf,sizeof(linebuf),fs) != NULL)
  {
    _swTrim(linebuf);
    if(linebuf[0] != '#')
    {
      sscanf(linebuf,"%s %s %s",
        alFilename,
        alIpaddr,alTime);
      if(strcmp(alFilename,filename) == 0)
      {
        if (strcmp(alIpaddr,ipaddr) == 0)
        {
          fclose(flTmp);
          fclose(fs);
          swVdebug(2,"S1740: [函数返回] swFilelock_add,返回码=0");
          return 0;
        }
        else if(!forceflag)
        {
          fclose(fs);
          fclose(flTmp);
          swVdebug(2,"S1750: [函数返回] swFilelock_add,返回码=133");
          return(133);
        }
        else
          continue;
      }
    }
    strcat(linebuf,"\n");
    fputs(linebuf,flTmp);
  }
  fclose(fs);

  _swGetTime(alTime);
  sprintf(linebuf,"%15s %16s \"%s\"\n",
    filename,ipaddr,alTime);
  fputs(linebuf,flTmp);
  fclose(flTmp);
  sprintf(linebuf,"mv %s %s",alTmpfile,alLockfile);
  system(linebuf);
  swVdebug(2,"S1760: [函数返回] swFilelock_add,返回码=0");
  return(0);
}


int swFilelock_find(char *filename,char *ipaddr)
{
  FILE *fs;
  char linebuf[512];
  char alFilename[300],alIpaddr[20],alTime[80],alLockfile[300];
  int ilRc,ilFlag = 0;

  swVdebug(2,"S1770: [函数调用] swFilelock_find");
  /* modify by nh 20020924
  sprintf(alLockfile,"%s/config/%s",getenv("SWITCH_CFGDIR"),LOCKFILE);*/
  sprintf(alLockfile,"%s/config/%s",agPath,LOCKFILE);

  if((fs = fopen(alLockfile,"r")) == NULL)
    if((fs = fopen(alLockfile,"w+")) == NULL)
    {
      swVdebug(2,"S1780: [函数返回] swFilelock_find,返回码=130");
      return(130);
    }
  
  while(fgets(linebuf,sizeof(linebuf),fs) != NULL)
  {
    _swTrim(linebuf);
    if(linebuf[0] != '#')
    {
      sscanf(linebuf,"%s %s %s",
        alFilename,
        alIpaddr,alTime);
      if(strcmp(alFilename,filename) == 0)
      {
        if(strcmp(alIpaddr,ipaddr) == 0)
        {
            ilFlag = 1;
            break;
        }
      }
    }
  }
  fclose(fs);

  if(!ilFlag)
  {
    ilRc = swFilelock_add(filename,ipaddr,0);
    swVdebug(2,"S1790: [函数返回] swFilelock_find,返回码=%d",ilRc);
    return(ilRc);
  }
  else
  {
    swVdebug(2,"S1800: [函数返回] swFilelock_find,返回码=0");
    return(0);
  }
}

/*  function modify by zjj 2001.10.26  */
int swGetcfgval(char *aFldname,char *aFldval)
{
  short ilRc,i;
  Dataset slSet;

  InitDataset("SWCONFIG.CFG",&slSet);

  ilRc = ReadMode2(&slSet);
  if(ilRc)return(ilRc);
  for(i = 0;i < slSet.iFieldNum;i++)
  {
    if(strcmp(slSet.Fields[i].aFieldName,aFldname) == 0)
    {
      strcpy(aFldval,slSet.Records[0].Values[i].aValue);
      FreeDataset(&slSet);
      return(0);
    }
    
  }
  FreeDataset(&slSet);
  return(131);
}
/* end modify by zjj 2001.10.26   */

/* function add by zjj 2001.10.26   */
int swTosysmon(keyrec *pKeys,short iCmd)
{
  struct msgpack slMsgpack;
  char alVal[101],alAddr_udp[16];
  short ilPort_udp;
  int   ilSocket_udp,ilMsglen_udp,ilRc;
  
  memset(&slMsgpack,0x00,sizeof(slMsgpack));
  slMsgpack.sMsghead.iMsgtype = iCmd;

  if (pKeys[0].keyval == '\0') return 170;

  strcpy(slMsgpack.aMsgbody,pKeys[0].keyval);
  slMsgpack.sMsghead.iBodylen = strlen(pKeys[0].keyval);
  ilRc = swGetcfgval("SYSMON_PORT",alVal);
  if (ilRc != 0) 
  {
    swVdebug(1,"S1810: 从配置文件中取sysmon端口出错,ilRc = [%d]",ilRc);
    return (170);
  }
  ilPort_udp = atoi(alVal); 
  ilRc = swGetcfgval("SYSMON_IP",alVal);
  if (ilRc != 0) 
  {
    swVdebug(1,"S1820: 从配置文件中取sysmon地址出错,ilRc = [%d]",ilRc);
    return (171);
  }    
  strcpy(alAddr_udp,alVal);
  if ((ilSocket_udp = _swUDPCreate(0)) == -1) 
  {
    swVdebug(1,"S1830: 创建 socket_udp 出错!");
    return(172);
  }
  swVdebug(2,"S1840: 创建 Socket_UDP 成功!");    
  ilMsglen_udp = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);    
  ilRc = _swUDPPut(ilSocket_udp,alAddr_udp,ilPort_udp,(char *)&slMsgpack,ilMsglen_udp);
  if (ilRc)
  {
     swVdebug(1,"S1850: 向系统监控(swSysmon)发送前台报文失败!,UDPPUT  ilRc = %d",ilRc);
     _swUDPClose(ilSocket_udp);  
     return(173);
  }
  swVdebug(2,"S1860: 向系统监控(swSysmon)发送前台报文OK! UDPPUT ilRc = %d",ilRc);
  if (cgDebug >= 2) 
     swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);        
  _swUDPClose(ilSocket_udp);  
  return 0;
}
/* function end add by zjj 2001.10.26  */

void swQuit(int sock)
{
  signal(SIGTERM,SIG_IGN);
  close(sock);
  swVdebug(1,"S1870: syscom exit!");
  exit (0);
}

int swAddLock(char *aAddr,char *aTableName)
{
  return(swFilelock_add(aTableName,aAddr,0));
}

int swDeleteLock(char *aAddr,char *aTableName)
{
  return(swFilelock_del(aTableName,aAddr,0));
}

int CheckLock(char *aAddr,char *aTableName)
{ 
  return(swFilelock_find(aTableName,aAddr));
}

void swErrorInfo(int sockfd,int Errno)
{
  char alMsgpack[1024];
  unsigned int ilLen;

  strcpy(alMsgpack,XMLHEAD);
  strcat(alMsgpack,"\n");
  strcat(alMsgpack,"<INFO success=\"no\" message=\"");
  strcat(alMsgpack,agErrorMessage[Errno - 100]);
  strcat(alMsgpack,"\" />");
  ilLen = strlen(alMsgpack);
  swTcpPut(sockfd,alMsgpack,ilLen);
}

void swSuccessInfo(int sockfd)
{
  char alMsgpack[1024];
  unsigned int ilLen;

  strcpy(alMsgpack,XMLHEAD);
  strcat(alMsgpack,"\n");
  strcat(alMsgpack,"<INFO success=\"yes\" message=\"\" />");
  ilLen = strlen(alMsgpack);
  swTcpPut(sockfd,alMsgpack,ilLen);
}

void swEndInfo(int sockfd)
{
  char alMsgpack[1024];
  unsigned int ilLen;

  strcpy(alMsgpack,XMLHEAD);
  strcat(alMsgpack,"\n");
  strcat(alMsgpack,"<INFO success=\"end\" message=\"\" />");
  ilLen = strlen(alMsgpack);
  swTcpPut(sockfd,alMsgpack,ilLen);
}

pNode AddChildNode(pNode psNode)
{
  pNode pslNode;
  
  psNode->iChildNum++;
  if(psNode->iChildNum == 1)
    psNode->ChildNodes = (char*)malloc(sizeof(Node));
  else
    psNode->ChildNodes = (char*)realloc(psNode->ChildNodes,psNode->iChildNum * sizeof(Node));
  
  pslNode = &((pNode)psNode->ChildNodes)[psNode->iChildNum - 1];
  pslNode->iChildNum = 0;
  pslNode->iAttrNum = 0;
  return pslNode;
}

pAttrib AddAttrib(pNode psNode,char *aName,char *aVal)
{
  pAttrib pslAttr;
  
  psNode->iAttrNum++;
  
  if(psNode->iAttrNum == 1)
    psNode->Attribs = (pAttrib)malloc(sizeof(Attrib));
  else
    psNode->Attribs = (pAttrib)realloc(psNode->Attribs,
                               psNode->iAttrNum * sizeof(Attrib));
  
  pslAttr = &psNode->Attribs[psNode->iAttrNum - 1];
  strcpy(pslAttr->aAttribName,aName);
  strcpy(pslAttr->aAttribVal,aVal);
  return(pslAttr);
}

/*读取交易信息树*/
/* 函数AddContent从相关路由脚本中读出描述加入交易信息中 */

void AddContent(pNode psNode)
{
  int i,ilRc;
  pNode pslNode;
  pAttrib pslAttr;
  char alContent[512],alQid[20],alTrancode[101];
  Dataset slSet;
  pRecord pslRec;

  /* 打开路由匹配表 */
  InitDataset("SWMATCH_ROUTE.CFG",&slSet);
  ilRc = OpenTable(NULL,0,"SWMATCH_ROUTE.CFG",&slSet);
  if(ilRc)       /* 打开失败 */
  {
    swVdebug(2,"S0000 : 打开匹配表失败!");
    return; 
  }
  
  if((strcmp(psNode->aNodeName,"TRANS") == 0) || 
          (strcmp(psNode->aNodeName,"CATALOG") == 0))
  {
    for(i = 0;i < psNode->iChildNum;i++)
    {
      pslNode = &((pNode)psNode->ChildNodes)[i];
      AddContent(pslNode);
    }
  }
  else if(strcmp(psNode->aNodeName,"TRANINFO") == 0)
  {
    strcpy(alContent,"无描述");
    ilRc = GetAttrByName(psNode,"QID",&pslAttr);
    if(!ilRc)
    {
      strcpy(alQid,pslAttr->aAttribVal);
    }
    ilRc = GetAttrByName(psNode,"TRANCODE",&pslAttr);
    if(!ilRc)
      strcpy(alTrancode,pslAttr->aAttribVal);
    for(i = 0; i < slSet.iRecordNum;i++)
    {
      pslRec = &slSet.Records[i];
      if((strcmp(pslRec->Values[1].aValue,alQid) == 0) &&
            (strcmp(pslRec->Values[2].aValue,alTrancode) == 0))
      {
        strcpy(alContent,pslRec->Values[3].aValue);
        break;
      }
    }
    ilRc = GetAttrByName(psNode,"CONTENT",&pslAttr);
    if(ilRc)
    {
      AddAttrib(psNode,"CONTENT",alContent);
    }
    else
    {
      strcpy(pslAttr->aAttribVal,alContent);
    }
  }
  FreeDataset(&slSet);
}

int ReadTrans(int sockfd)
{
  char alBuf[8193],alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;

  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);

  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5310: invoke ParseXmlFile File = %s ilRc = %d",alFileName,ilRc);  
  if(ilRc)
  {
    swVdebug(2,"S1114 : ReadTrans 解析XML文件失败!返回[%d]",ilRc);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
 /*  Delete   
  AddContent(&slRootNode);
 */ 
  ilLen = 0;
  ilRc = WriteNodeToBuf(sockfd,alBuf,&ilLen,&slRootNode,1);
  swVdebug(2,"S5204: invoke WriteNodeToBuf  ilRc = %d",ilRc);  
  DestroyNode(&slRootNode);
  
  swVdebug(2,"ilRc = %d",ilRc);
  if(ilRc)
    return(ilRc);
  swEndInfo(sockfd); 
  return(0);
}

int FindTranNode(pNode psNode,char* aQid,char* aTranCode,pNode *ppsNode)
{
  int i,ilRc;
  pNode pslNode;
  pAttrib pslAttr;
  char alQid[20],alTranCode[101];
  
  if((strcmp(psNode->aNodeName,"CATALOG") == 0) ||
     (strcmp(psNode->aNodeName,"TRANS") == 0))
  {
    for(i = 0;i < psNode->iChildNum;i++)
    {
      pslNode = &((pNode)psNode->ChildNodes)[i];
      if(FindTranNode(pslNode,aQid,aTranCode,ppsNode) == 0)
        return 0;
    }
  }
  else if(strcmp(psNode->aNodeName,"TRANINFO") == 0)
  {
    ilRc = GetAttrByName(psNode,"QID",&pslAttr);
    if(ilRc)
      return -1;
    strcpy(alQid,pslAttr->aAttribVal);
    ilRc = GetAttrByName(psNode,"TRANCODE",&pslAttr);
    if(ilRc)
      return -1;
    strcpy(alTranCode,pslAttr->aAttribVal);
/*    swVdebug(4,"S5599 : 解析XML alQid = %s alTranCode = %s",alQid,alTranCode);    */
    if((strcmp(aQid,alQid) == 0) && (strcmp(aTranCode,alTranCode) == 0))
    {
      *ppsNode = psNode;
      swVdebug(2,"S5556: 找到结点 Qid = %s TranCode = %s",aQid,aTranCode);
      return 0;
    }
  }
  return -1;
}

/*读取交易信息*/
int FindNode(pNode psPN,char *aSt,pNode *ppsNode)
{
  char alT1[1024],alT2[1024],*p;
  int i;
  pNode pslNode,pslNode1;
  pAttrib pslAttr;
  short ilFlag;
  
  swVdebug(2,"S0899 : FindNode aSt = %s",aSt);
  pslNode = psPN;
  strcpy(alT1,aSt);
  p = strchr(alT1,'|');
  while(p != NULL)
  {
    *p = 0;
    p++;
    strcpy(alT2,alT1);
    strcpy(alT1,p);
    ilFlag = 0;
    for(i = 0;i < pslNode->iChildNum;i++)
    {
      pslNode1 = &((pNode)pslNode->ChildNodes)[i];
      if(GetAttrByName(pslNode1,"CONTENT",&pslAttr) == 0)
      {
      	if(strcmp(pslAttr->aAttribVal,alT2) == 0)
      	{
          ilFlag = 1;
          pslNode = pslNode1;
          break;
        }
      }
    }
    if(ilFlag == 0)
    {
      swVdebug(2,"S0999 : FindNode Not Found!");
      return(678);
    }
    p = strchr(alT1,'|');
  }
  strcpy(alT2,alT1);
  ilFlag = 0;
  for(i = 0;i < pslNode->iChildNum;i++)
  {
    pslNode1 = &((pNode)pslNode->ChildNodes)[i];
    if(GetAttrByName(pslNode1,"GROUP",&pslAttr) == 0)
    {
      if(strcmp(pslAttr->aAttribVal,alT2) == 0)
      {
        ilFlag = 1;
        pslNode = pslNode1;
        break;
      }
    }
    if(GetAttrByName(pslNode1,"CONTENT",&pslAttr) == 0)
    {
      if(strcmp(pslAttr->aAttribVal,alT2) == 0)
      {
        ilFlag = 1;
        pslNode = pslNode1;
        break;
      }
    }
  }
  if(ilFlag == 0)
  {
    swVdebug(2,"S0999 : FindNode Not Found!");
    return(678);
  }
  *ppsNode = pslNode;
  return(0);
}

int ReadTranInfo(int sockfd,char *aQid,char* aTranCode)
{
  char alFileName[1024],alBuf1[8193];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode;
  
  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);
  swVdebug(2,"S5565: ParseXmlFile %s",alFileName);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5303: invoke ParseXmlFile ilRc = %d",ilRc);  
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(2,"S5555: 调用FindTranNode,Qid = %s TranCode = %s",aQid,aTranCode);
  ilRc = FindTranNode(&slRootNode,aQid,aTranCode,&pslNode);
  swVdebug(2,"S5304: invoke FindTranNode aQid = %s aTranCode = %s ilRc = %d",aQid,aTranCode,ilRc);  
  if(ilRc)
  {
    swErrorInfo(sockfd,151);
    return(151);
  }
  ilLen = 0;
  swVdebug(2,"S5556: WriteNodeToBuf!!");
  ilRc = WriteNodeToBuf(sockfd,alBuf1,&ilLen,pslNode,1);
  swVdebug(2,"S5305: invoke WriteNodeToBuf ilRc = %d",ilRc);  
  DestroyNode(&slRootNode);
  
  if(ilRc)
    return(ilRc);
  swEndInfo(sockfd); 
  return(0);
}

int FindRouteNode(pNode psPN,char *aQid,char* aTranCode,pNode *ppsNode)
{
  int i;
  pNode pslNode,pslNode1;
  pAttrib pslAttr;
  char alQid[20],alTranCode[101];
  
  
  pslNode = psPN;

  for(i = 0;i < pslNode->iChildNum;i++)
  {
    alQid[0] = 0;
    alTranCode[0] = 0;
    pslNode1 = &((pNode)pslNode->ChildNodes)[i];
    if(GetAttrByName(pslNode1,"QID",&pslAttr) == 0)
    {
      strcpy(alQid,pslAttr->aAttribVal);
    }
    if(GetAttrByName(pslNode1,"TRANCODE",&pslAttr) == 0)
    {
      strcpy(alTranCode,pslAttr->aAttribVal);
    }
    if((strcmp(alQid,aQid) == 0) && (strcmp(alTranCode,aTranCode) == 0))
    {
      *ppsNode = pslNode1;
      return 0;
    }
  }
  return(678);
}

int ReadRouteTDF(int sockfd,char *aQid,char* aTranCode)
{
  char alFileName[1024],alBuf1[8193];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode;
  
  sprintf(alFileName,"%s/config/SWROUTETDF.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5431 : invoke ParseXmlFile   ilRc = %d",ilRc);    
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(3,"S5555 : Parse File OK!");
  ilRc = FindRouteNode(&slRootNode,aQid,aTranCode,&pslNode);
  swVdebug(2,"S5432: invoke FindRouteNode aQid = %s aTranCode = %s ilRc = %d",aQid,aTranCode,ilRc);
  if(ilRc)
  {
    DestroyNode(&slRootNode);
    swEndInfo(sockfd); 
    return(ilRc);
  }
  swVdebug(3,"S6666 : Find Route Node!");
  ilLen = 0;
  ilRc = WriteNodeToBuf(sockfd,alBuf1,&ilLen,pslNode,1);
  swVdebug(2,"S5434: invoke WriteNodeToBuf inRc = %d",ilRc);  
  DestroyNode(&slRootNode);
  if(ilRc)
    return(ilRc);
  swEndInfo(sockfd); 
  return(0);
}

int AttachTDF2Route(int sockfd,char *aRouteQid,char* aRouteTranCode,char *aFmtGrp,char *aDesc,
          char *aQid,char *aTranCode,char *aTranStep,char *aTranType)
{
  char alFileName[1024];
  int i,ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode,pTmp;
  pAttrib pslAttr;
  /* add by zjj 2002.08.20 */
  char alFmtGrp[10];
  char alQid[10];
  char alTranCode[21];
  char alTranStep[10];
  char alTranType[10];
  /*end add by zjj 2002.08.20 */
  
  sprintf(alFileName,"%s/config/SWROUTETDF.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5421 : invoke ParseXmlFile   ilRc = %d",ilRc);    
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(3,"S5555 : Parse File OK!");
  ilRc = FindRouteNode(&slRootNode,aRouteQid,aRouteTranCode,&pslNode);
  swVdebug(2,"S5522 : invoke FindTranNode  aRouteQid = %s aRouteTranCode = %s ilRc = %d",aRouteQid,aRouteTranCode,ilRc);         
  if(ilRc)
  {
    pslNode = AddChildNode(&slRootNode);
    strcpy(pslNode->aNodeName,"ROUTE");
    AddAttrib(pslNode,"QID",aRouteQid);
    AddAttrib(pslNode,"TRANCODE",aRouteTranCode);
  }
  
  /* 增加格式关联的修改功能 */
  for(i = 0;i < pslNode->iChildNum;i++)
  {
    pTmp = &((pNode)pslNode->ChildNodes)[i];
/*    
    ilRc = GetAttrByName(pTmp,"GROUP",&pslAttr);
    if(!ilRc)
    {
      if(strcmp(pslAttr->aAttribVal,aFmtGrp) == 0) 
      {
        DestroyNode(&slRootNode);
        swErrorInfo(sockfd,100);
        return(100);
      }
    }
*/  /* delete by zjj 2002.08.20 */    
/* add by zjj 2002.08.20 */
      ilRc = GetAttrByName(pTmp,"GROUP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alFmtGrp,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pTmp,"QID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alQid,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pTmp,"TRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranCode,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pTmp,"TRANSTEP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranStep,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pTmp,"TRANTYPE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranType,pslAttr->aAttribVal);
      }
      if (strcmp(alFmtGrp,aFmtGrp) == 0 && strcmp(alQid,aQid) == 0 && \
           strcmp(alTranCode,aTranCode) == 0 && strcmp(alTranStep,aTranStep) == 0 && \
           strcmp(alTranType,aTranType) == 0)
      {
        pTmp->aNodeName[0] = 0;
        break;
      }    
/* end add by zjj 2002.08.20 */      
  } 

  pslNode = AddChildNode(pslNode);
  strcpy(pslNode->aNodeName,"TDF");
  AddAttrib(pslNode,"GROUP",aFmtGrp);
  AddAttrib(pslNode,"DESC",aDesc);
  AddAttrib(pslNode,"QID",aQid);
  AddAttrib(pslNode,"TRANCODE",aTranCode);
  AddAttrib(pslNode,"TRANSTEP",aTranStep);
  AddAttrib(pslNode,"TRANTYPE",aTranType);
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5424 : invoke WriteXmlFile   ilRc = %d",ilRc);   
  DestroyNode(&slRootNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(2,"S5425: Send cuccess info");
  swSuccessInfo(sockfd);
  return(0);
}

int DettachTDFFromRoute(int sockfd,char *aRouteQid,char* aRouteTranCode,char *aFmtGrp,char *aQid,char *aTranCode,char *aTranStep,char * aTranType)
{
  char alFileName[1024];
  int ilRc;
  int ilLen,i,ilFlag;
  Node slRootNode;
  pNode pslNode,pslNode1;
  pAttrib pslAttr;
  
  /* add by zjj 2002.08.20 */
  char alFmtGrp[12];
  char alQid[12];
  char alTranCode[21];
  char alTranStep[12];
  char alTranType[12];
  /* end add by zjj 2002.08.20 */  
  
  sprintf(alFileName,"%s/config/SWROUTETDF.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5411 : invoke ParseXmlFile   ilRc = %d",ilRc);    
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(3,"S5555 : Parse File OK!");
  ilRc = FindRouteNode(&slRootNode,aRouteQid,aRouteTranCode,&pslNode);
  swVdebug(2,"S5512 : invoke FindTranNode  aRouteQid = %s aRouteTranCode = %s ilRc = %d",aRouteQid,aRouteTranCode,ilRc);       
  if(ilRc)
  {
    swVdebug(2,"S0909 : Dettach TDF Node Not Found!");
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  ilFlag = 0;

  swVdebug(4,"ZJJ11aFmtGrp = %s alQid = %s aTranCode = %s aTranStep = %s aTranType = %s",\
  aFmtGrp,aQid,aTranCode,aTranStep,aTranType);
 
  for(i = 0;i < pslNode->iChildNum;i++)
  {
    pslNode1 = &((pNode)pslNode->ChildNodes)[i];
    ilRc = GetAttrByName(pslNode1,"GROUP",&pslAttr);
    if(ilRc)continue;
/* delete by zjj 2002.08.20 */  
/*  
    if(strcmp(pslAttr->aAttribVal,aFmtGrp) == 0)
    {
      ilFlag = 1;
      break;
    }
*/    
/* end delete by zjj 2002.08.20 */    
/* add by zjj 2002.08.20 */
    ilRc = GetAttrByName(pslNode1,"GROUP",&pslAttr);
    if(ilRc == 0)
    {
      strcpy(alFmtGrp,pslAttr->aAttribVal);
    }
      
      ilRc = GetAttrByName(pslNode1,"QID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alQid,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode1,"TRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranCode,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode1,"TRANSTEP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranStep,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode1,"TRANTYPE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranType,pslAttr->aAttribVal);
      }

  swVdebug(4,"alFmtGrp = %s alQid = %s alTranCode = %s alTranStep = %s alTranType = %s",\
  alFmtGrp,alQid,alTranCode,alTranStep,alTranType);
  
        
      if (strcmp(alFmtGrp,aFmtGrp) == 0 && strcmp(alQid,aQid) == 0 && \
           strcmp(alTranCode,aTranCode) == 0 && strcmp(alTranStep,aTranStep) == 0 && \
           strcmp(alTranType,aTranType) == 0)
    {
      ilFlag = 1;
      break;
    }           
/* end add by zjj 2002.08.20 */    
  }
  
  if(ilFlag == 0)
  {
    swVdebug(2,"S0909 : Dettach TDF Node Not Found!");
    swErrorInfo(sockfd,444);
    return(444);
  }
  
  pslNode1->aNodeName[0] = 0;
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5414 : invoke WriteXmlFile   ilRc = %d",ilRc);   
  DestroyNode(&slRootNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(2,"S5405: Send cuccess info");
  swSuccessInfo(sockfd);
  return(0);
}

void AddCatalog(int sockfd,char *aParent,char *aContent)
{
  char alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode,pslNode1;
  
  swVdebug(3,"S3333 : AddCatalog--aParent = [%s],aContent = [%s]",aParent,aContent);
  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5401 : invoke ParseXmlFile   ilRc = %d",ilRc);    
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return;
  }
  
  pslNode = &slRootNode;
  
  if(aParent[0] != 0)
  {
    ilRc = FindNode(&slRootNode,aParent,&pslNode);
    swVdebug(2,"S5403 : invoke FindNode  aParent = %s  ilRc = %d",aParent,ilRc);    
    if(ilRc)
    {
      DestroyNode(&slRootNode);
      swErrorInfo(sockfd,ilRc);
      return;
    }
  }
  
  pslNode1 = AddChildNode(pslNode);
  strcpy(pslNode1->aNodeName,"CATALOG");
  AddAttrib(pslNode1,"CONTENT",aContent);
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5404 : invoke WriteXmlFile   ilRc = %d",ilRc);   
  DestroyNode(&slRootNode);
  
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return;
  }
  swVdebug(3,"S8899 : OK AddCatalog");
  swVdebug(2,"S5404: send cuccess info");
  swSuccessInfo(sockfd); 
}

void AddTran(int sockfd,char *aCatalog,char *aContent,
                 char *aQid,char *aTranCode,char *aRouteGrp)
{
  char alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode,pslNode1;
  
  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5501 : invoke ParseXmlFile   ilRc = %d",ilRc);   
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return;
  }
  
  ilRc = FindTranNode(&slRootNode,aQid,aTranCode,&pslNode);
  swVdebug(2,"S5502 : invoke FindTranNode  aQid = %s aTranCode = %s ilRc = %d",aQid,aTranCode,ilRc);     
  if(!ilRc) /* Exist!! */
  {
    DestroyNode(&slRootNode);
    swErrorInfo(sockfd,100);
    return;
  }
  
  pslNode = &slRootNode;
  
  if(aCatalog[0] != 0)
  {
    ilRc = FindNode(&slRootNode,aCatalog,&pslNode);
    swVdebug(2,"S5503 : invoke FindNode  aCatalog = %s  ilRc = %d",aCatalog,ilRc);
    if(ilRc)
    {
      DestroyNode(&slRootNode);
      swErrorInfo(sockfd,ilRc);
      return;
    }
  }
  
  pslNode1 = AddChildNode(pslNode);
  strcpy(pslNode1->aNodeName,"TRANINFO");
  AddAttrib(pslNode1,"CONTENT",aContent);
  AddAttrib(pslNode1,"QID",aQid);
  AddAttrib(pslNode1,"TRANCODE",aTranCode);
  AddAttrib(pslNode1,"GROUP",aRouteGrp);
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5504 : invoke WriteXmlFile   ilRc = %d",ilRc);    
  DestroyNode(&slRootNode);

  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return;
  }
  swVdebug(2,"Send cuccess info");
  swSuccessInfo(sockfd); 
}

int EditCatalog(int sockfd,char *aCatalogOld,char *aContent)
{
  char alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode;
  pAttrib pslAttr;
  
  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5510 : invoke ParseXmlFile   ilRc = %d",ilRc); 
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  ilRc = FindNode(&slRootNode,aCatalogOld,&pslNode);
  swVdebug(2,"S5512 : invoke FindNode CatalogOld = %s  ilRc = %d",aCatalogOld,ilRc);      
  if(ilRc)
  {
    DestroyNode(&slRootNode);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  if(strcmp(pslNode->aNodeName,"CATALOG") != 0)
  {
    DestroyNode(&slRootNode);
    swErrorInfo(sockfd,678);
    return(678);
  }
  
  ilRc = GetAttrByName(pslNode,"CONTENT",&pslAttr);
  if(ilRc)
  {
    AddAttrib(pslNode,"CONTENT",aContent);
  }
  else
    strcpy(pslAttr->aAttribVal,aContent);
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5522 : invoke WriteXmlFile ilRc = %d",ilRc);      
  DestroyNode(&slRootNode);
  
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(2,"S5500: Send cuccess info");
  swSuccessInfo(sockfd); 
  return(0);
}

int EditTran(int sockfd,char *aRouteQid,char* aRouteTranCode,char *aContent,
                 char *aQid,char *aTranCode,char *aRouteGrp)
{
  char alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode;
  pAttrib pslAttr;
  
  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5520 : invoke ParseXmlFile ilRc = %d",ilRc);  
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  ilRc = FindTranNode(&slRootNode,aRouteQid,aRouteTranCode,&pslNode);
  swVdebug(2,"S5521 : invoke FindTranNode aRouteQid = %s aRouteTranCode = %s ilRc = %d",aRouteQid,aRouteTranCode,ilRc);  
  if(ilRc)
  {
    DestroyNode(&slRootNode);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  if(strcmp(pslNode->aNodeName,"TRANINFO") != 0)
  {
    DestroyNode(&slRootNode);
    swErrorInfo(sockfd,678);
    return(678);
  }
  
  if(aQid[0] != 0)
  {
    ilRc = GetAttrByName(pslNode,"QID",&pslAttr);
    if(ilRc)
    {
      AddAttrib(pslNode,"QID",aQid);
    }
    else
      strcpy(pslAttr->aAttribVal,aQid);
  }
  
  if(aTranCode[0] != 0)
  {
    ilRc = GetAttrByName(pslNode,"TRANCODE",&pslAttr);
    if(ilRc)
    {
      AddAttrib(pslNode,"TRANCODE",aTranCode);
    }
    else
      strcpy(pslAttr->aAttribVal,aTranCode);
  }
  
  if(aContent[0] != 0)
  {
    ilRc = GetAttrByName(pslNode,"CONTENT",&pslAttr);
    if(ilRc)
    {
      AddAttrib(pslNode,"CONTENT",aContent);
    }
    else
      strcpy(pslAttr->aAttribVal,aContent);
  }
  
  if(aRouteGrp[0] != 0)
  {
    ilRc = GetAttrByName(pslNode,"GROUP",&pslAttr);
    if(ilRc)
    {
      AddAttrib(pslNode,"GROUP",aRouteGrp);
    }
    else
      strcpy(pslAttr->aAttribVal,aRouteGrp);
  }
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5522 : invoke WriteXmlFile ilRc = %d",ilRc);    
  DestroyNode(&slRootNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return ilRc;
  }
  swVdebug(2,"Send cuccuess info");
  swSuccessInfo(sockfd); 
  return 0;
}

int DeleteCatalog(int sockfd,char *aCatalog)
{
  char alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode;
  
  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5530 : invoke ParseXmlFile ilRc = %d",ilRc);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  ilRc = FindNode(&slRootNode,aCatalog,&pslNode);
  swVdebug(2,"S5531 : invoke FindNode ilRc = %d",ilRc);  
  if(ilRc)
  {
    DestroyNode(&slRootNode);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  pslNode->aNodeName[0] = 0;
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5532 : invoke WriteXmlFile ilRc = %d",ilRc);   
  DestroyNode(&slRootNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(2,"S5533: send success info");
  swSuccessInfo(sockfd); 
  return(0);

}

int DeleteTran(int sockfd,char* aQid,char* aTranCode)
{
  char alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;
  pNode pslNode;
  
  sprintf(alFileName,"%s/config/SWTRANINFO.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  swVdebug(2,"S5540 : invoke ParseXmlFile ilRc = %d",ilRc);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  ilRc = FindTranNode(&slRootNode,aQid,aTranCode,&pslNode);
  swVdebug(2,"S5541 : invoke FindTranNode aQid = %s aTranCode = %s ilRc = %d",aQid,aTranCode,ilRc);
  if(ilRc)
  {
    DestroyNode(&slRootNode);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  pslNode->aNodeName[0] = 0;
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  swVdebug(2,"S5542 : invoke WriteXmlFile,ilRc = %d",ilRc);
  DestroyNode(&slRootNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  swVdebug(2,"S5543 : send cucess info");
  swSuccessInfo(sockfd); 
  return(0);
}

int BuildCatalog(pDataset pSet,short iCataFlag,
                                              keyrec * pKeys,short iKeyNum,
                                              char * aXmlBuf,
                                              unsigned int *piLen,
                                              short *piEnd,
                                              short *piRecNo)
{
  short i,j,ilFlag,k;
  char *p;
  short ilPos;
  char alBuf[512],alXmlbuf[1024];

  swVdebug(2,"S2020: [函数调用] BuildCatalog");
  (*piEnd) = 1;
  (*piLen) = 0;
  
  p = aXmlBuf;
  
  if((*piRecNo) == 1)
  {
    strcpy(p,
         "<?xml version=\"1.0\" encoding=\"GB2312\" standalone=\"yes\"?>\n"); 
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;
  
    strcpy(p,"<DATAPACKET Version=\"2.0\">\n");
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;
   
    strcpy(p,"<METADATA>\n");
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;
  
    strcpy(p,"<FIELDS>\n");
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;

    for(j = 0;j < pSet->iFieldNum;j++)
    {
      if(iCataFlag)
      if(!(pSet->Fields[j].iFieldAttr & 0x1))
        continue;
      if(pSet->Fields[j].iFieldType == T_PCHAR)
      {
        swVdebug(2,"J = %d",j);
        sprintf(alBuf,"<FIELD attrname=\"%s\" fieldtype=\"%s\""\
                    " WIDTH=\"%d\" />\n",
                    pSet->Fields[j].aFieldName,
                    TypeName[pSet->Fields[j].iFieldType-1],
                    pSet->Fields[j].iFieldWidth);
      }
      else
      {
        sprintf(alBuf,"<FIELD attrname=\"%s\" fieldtype=\"%s\" />\n",
                    pSet->Fields[j].aFieldName,
                    TypeName[pSet->Fields[j].iFieldType-1]);
      }
      strcpy(p,alBuf);
      ilPos = strlen(p);
      (*piLen) += ilPos; 
      p += ilPos;
    }

    strcpy(p,"</FIELDS>\n");
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;

    strcpy(p,"</METADATA>\n");
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;


    /* modify by zjj 2001.10.25      */
    sprintf(p,"<ROWDATA count=\"%d\">\n",pSet->iRecordNum);
    /* end modify by zjj 2001.10.25  */
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;
  }

  for(i = (*piRecNo)-1;i < pSet->iRecordNum;i++)
  {
    if(pSet->iStoreMode != MODE3D)
    if(pSet->Records[i].iHideFlag)
      continue;
    ilFlag = 1;
    for(k = 0; k < iKeyNum;k++)
    {
      for(j = 0;j < pSet->iFieldNum;j++)
      {
        if(strcmp(pKeys[k].keyname,pSet->Fields[j].aFieldName) == 0)
        {
          if(strcmp(pKeys[k].keyval,pSet->Records[i].Values[j].aValue)
                           == 0)
            ilFlag = 1;
          else
            ilFlag = 0;
          break;
        }
      }
      if(ilFlag == 0)
        break;
    }
    if(ilFlag == 0)continue;
    strcpy(p,"<ROW ");
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;
    for(j = 0;j < pSet->iFieldNum;j++)
    {
      if(iCataFlag)
      {
        if(!(pSet->Fields[j].iFieldAttr & 0x1))
          continue;
      }
      swXmltrans( pSet->Records[i].Values[j].aValue,alXmlbuf);
      sprintf(alBuf,"%s=\"%s\" ",
                    pSet->Fields[j].aFieldName,
                    alXmlbuf);                   
      swVdebug(2,"%s",alBuf);
      strcpy(p,alBuf);
      ilPos = strlen(p);
      (*piLen) += ilPos; 
      p += ilPos;
    }  
    strcpy(p,"/>\n");
    ilPos = strlen(p);
    (*piLen) += ilPos; 
    p += ilPos;
    if((*piLen) > 7000)
    {
      (*piEnd) = 0;
      (*piRecNo) = i+2;
      break;
    }
  }
  if((*piEnd) == 0)
  {
    swVdebug(2,"S2030: [函数返回] BuildCatalog,长度为%d,返回码=0",*piLen);
    return(0);
  }
  strcpy(p,"</ROWDATA>\n");
  ilPos = strlen(p);
  (*piLen) += ilPos; 
  p += ilPos;

  
  strcpy(p,"</DATAPACKET>\n");
  ilPos = strlen(p);
  (*piLen) += ilPos; 
  p += ilPos;

  swVdebug(2,"S2040: [函数返回] BuildCatalog,长度为%d,返回码=0",*piLen);
  return(0);
}

int ReadCatalog(int sockfd,keyrec *pKeys,short iKeyNum,pDataset pslDataset)
{
  unsigned int ilEnd,ilRecno,ilMsglen;
  char alMsgpack[MAXPACKLEN + 1];
  int ilRc;

  swVdebug(2,"S2050: [函数调用] ReadCatalog");
  ilEnd = 0;
  ilRecno = 1; 
  while(!ilEnd)
  {
    ilRc = BuildCatalog(pslDataset,1,pKeys,
                            iKeyNum,
                            alMsgpack,
                            &ilMsglen,
                            &ilEnd,
                            &ilRecno);
    if(ilRc)break;
    ilRc = swTcpPut(sockfd,alMsgpack,ilMsglen); 
    if(ilRc)break;
  }
  swVdebug(2,"S2060: [函数返回] ReadCatalog,返回码=%d",ilRc);
  return(ilRc);
}

int ReadContent(int sockfd,keyrec *pKeys,short iKeyNum,pDataset pslDataset)
{
  unsigned int ilEnd,ilRecno,ilMsglen;
  char alMsgpack[MAXPACKLEN + 1];
  int ilRc;

  swVdebug(2,"S2070: [函数调用] ReadContent");
  ilEnd = 0;
  ilRecno = 1; 
  while(!ilEnd)
  {
    ilRc = BuildCatalog(pslDataset,0,pKeys,
                            iKeyNum,
                            alMsgpack,
                            &ilMsglen,
                            &ilEnd,
                            &ilRecno);
    if(ilRc)
    {
      break;
    }
    ilRc = swTcpPut(sockfd,alMsgpack,ilMsglen); 
    if(ilRc)
    {
      break; 
    }
  }
  swVdebug(2,"S2080: [函数返回] ReadCatalog,返回码=%d",ilRc);
  return(ilRc);
}

void swAddRecord(pRecord pRec,short iRecNum,pDataset pslDataset)
{
  short i;

  swVdebug(2,"S2090: [函数调用] swAddRecord");
  swDebugrecs(2,pRec,iRecNum);
  if((pslDataset->iStoreMode == MODE3D)||
     (pslDataset->iStoreMode == MODE10))
  {
    FreeDataset(pslDataset);
    pslDataset->iRecordNum = 0;
  }
  
  for(i = 0;i < iRecNum;i++)
  {
    Insert(&pRec[i],pslDataset);
  }
  swDebugset(2,pslDataset);
  swVdebug(2,"S2100: [函数返回] swAddRecord,返回码=无");
}

int swUpdateRecord(keyrec *pKeys,
                           short    iKeyNum,
                           pRecord  pRec,
                           short    iRecNum,
                           pDataset pslDataset)
{
  short ilKeyNum;
  int ilRc;

  swVdebug(2,"S2110: [函数调用] swUpdateRecord");
  ilKeyNum = iKeyNum;
  if((pslDataset->iStoreMode == MODE3G)||
    (pslDataset->iStoreMode == MODE3D))
  {
    ilKeyNum++;
    strcpy(pKeys[ilKeyNum -1].keyname,"GRPNEW");
    strcpy(pKeys[ilKeyNum -1].keyval,pRec->Values[0].aValue);
  }
  if(pslDataset->iStoreMode == MODE3D)
  {
    swAddRecord(pRec,iRecNum,pslDataset);
    swVdebug(2,"S2120: [函数返回] swUpdateRecord,返回码=0");
    return(0);
  }
  ilRc = UpdateAll(pKeys,ilKeyNum,pRec,pslDataset);
  swDebugset(2,pslDataset);
  
  swVdebug(2,"S2130: [函数返回] swUpdateRecord,返回码=%d",ilRc);
  return(ilRc);
}

int swDeleteRecord(keyrec *pKeys,short iKeyNum,pDataset pslDataset)
{
  int ilRc;

  swVdebug(2,"S2140: [函数调用] swDeleteRecord");
  ilRc = Delete(pKeys,iKeyNum,pslDataset);
  swDebugset(2,pslDataset);
  swVdebug(2,"S2150: [函数返回] swDeleteRecord,返回码=%d",ilRc);
  return(ilRc);
}

void swProcessData(int sockfd,short  iCmdCode,
                             char   *aTableName,
                             keyrec *pKeys,
                             short   iKeyNum,
                             pRecord pRec,
                             short   iRecNum,
                             char    *aAddr)
{
  int      ilRc; 
  Dataset  slDataset,*pslDataset;
  char     alFileName[300],alExt[10],alCmd[512];
  char     alGname[20],alDname[20];  /* add by zjj  */

  swVdebug(2,"S2160: [函数调用] swProcessData");
  InitDataset(aTableName,&slDataset);
  ilRc = OpenTable(pKeys,iKeyNum,aTableName,&slDataset);
  pslDataset = &slDataset;
  /* swDebugset(2,pslDataset);*/

  if((pslDataset->iStoreMode == MODE3G)   ||
     (pslDataset->iStoreMode == MODE3D))
  {
    if((strcmp(pslDataset->aTableName,"SWFORMATGRP") == 0)     ||
     (strcmp(pslDataset->aTableName,"SWFORMATD") == 0))
    {
      strcpy(alExt,"FMT");
    }
    else
    {
      strcpy(alExt,"SCR");
    }
    sprintf(alFileName,"%s.%s",pKeys[0].keyval,alExt);
  }
  else
    strcpy(alFileName,aTableName);

  if(!ilRc)
  {
    switch(iCmdCode)
    {
      /*  add by zjj 2001.10.26    */
      case 903:  
      case 904:
        ilRc = swTosysmon(pKeys,iCmdCode);
        break;
      /*  end add by zjj 2001.10.26 */
      case 914:
      case 915:
      case 907:   
        ilRc = ReadCatalog(sockfd,pKeys,iKeyNum,pslDataset);
        break;
      case 905:   /* add by zjj 2001.10.26  */
      case 908:  
        ilRc = ReadContent(sockfd,pKeys,
                           iKeyNum,
                           pslDataset);
        break;
      case 920:
        ilRc = ReadContent(sockfd,pKeys,0,pslDataset);
        break;
      case 910:
        ilRc = CheckLock(aAddr,alFileName);
        if(ilRc)break;
        swAddRecord(pRec,iRecNum,pslDataset);
        ilRc = WriteTable(pKeys,iKeyNum,aTableName,pslDataset);
        break;
      case 911:
        ilRc = CheckLock(aAddr,alFileName);
        if(ilRc)
        {
           break;
        }
        ilRc = swUpdateRecord(pKeys,iKeyNum,pRec,iRecNum,pslDataset);
        if(ilRc)
        {
          break;
        }
        ilRc = WriteTable(pKeys,iKeyNum,aTableName,pslDataset);
        if((pslDataset->iStoreMode == MODE3G)&&
                 (strcmp(pKeys[0].keyval,pRec->Values[0].aValue) != 0))
        {
          if(strcmp(pslDataset->aTableName,"SWFORMATGRP") ==0)
          {
            /* modify by nh 20020924
            sprintf(alCmd,"mv %s/format/%s %s/format/%s.FMT",getenv("SWITCH_CFGDIR"),alFileName,getenv("SWITCH_CFGDIR"),pRec->Values[0].aValue); */
            sprintf(alCmd,"mv %s/format/%s %s/format/%s.FMT",agPath,alFileName,agPath,pRec->Values[0].aValue);
          }
          if(strcmp(pslDataset->aTableName,"SWROUTEGRP") ==0)
          {
            /* modify by nh 20020924
            sprintf(alCmd,"mv %s/router/%s %s/router/%s.SCR",getenv("SWITCH_CFGDIR"),alFileName,getenv("SWITCH_CFGDIR"),pRec->Values[0].aValue);  */
            sprintf(alCmd,"mv %s/router/%s %s/router/%s.SCR",agPath,alFileName,agPath,pRec->Values[0].aValue); 
          }
          if(strcmp(pslDataset->aTableName,"SWFRONTGRP") ==0)
          {
            /* modify by nh 20020924
            sprintf(alCmd,"mv %s/preprocess/%s %s/preprocess/%s.SCR",getenv("SWITCH_CFGDIR"),alFileName,getenv("SWITCH_CFGDIR"),pRec->Values[0].aValue);*/
            sprintf(alCmd,"mv %s/preprocess/%s %s/preprocess/%s.SCR",agPath,alFileName,agPath,pRec->Values[0].aValue);
          }
          system(alCmd);
        }
        break;
      case 912:
        ilRc = CheckLock(aAddr,alFileName);
        if(ilRc)break;
/*  add by zjj 2001.10.23  */
        if ((strcmp(pslDataset->aTableName,"SWFORMATGRP") == 0) || 
           (strcmp(pslDataset->aTableName,"SWROUTEGRP") == 0) ||
           (strcmp(pslDataset->aTableName,"SWFRONTGRP") == 0))
        {
          ilRc = GetMode3FileName(pslDataset,pKeys,1,alFileName,\
            alGname,alDname); 
          if (ilRc) break;
          memset(alCmd,0x00,sizeof(alCmd));
          sprintf(alCmd,"rm -f %s",alFileName); 
          system(alCmd);
          free(pslDataset->Records);
          break;
        }
/*   end add by zjj 2001.10.23 */
        ilRc = swDeleteRecord(pKeys,iKeyNum,pslDataset);
        if(ilRc)break;
        ilRc = WriteTable(pKeys,iKeyNum,aTableName,pslDataset);
        break;
      default:
        ilRc = 104;
        break;
    }
    if(ilRc)
    {
      swErrorInfo(sockfd,ilRc);
      swVdebug(2,"%s",agErrorMessage[ilRc-100]);
    }
    else
    {
      if((iCmdCode == 907)  || (iCmdCode == 905) || 
         (iCmdCode == 908)  || (iCmdCode == 920) || 
         (iCmdCode == 914)  ||
         (iCmdCode == 915))
      {
        swEndInfo(sockfd);
      }  
      else
        swSuccessInfo(sockfd);
    }
      
  }
  else
  {
    swErrorInfo(sockfd,ilRc);
  }

  FreeDataset(&slDataset);
  swVdebug(3,"S2170: [函数返回] swProcessData");
}

void swAdminLock(int sockfd,pNode pNode1)
{
  Dataset slSet,*pSet;
  keyrec slKey;
  pNode pTempNode,pTempNode1;
  pAttrib pAttr;
  short i;
  int ilRc;
  
  swVdebug(2,"S2180: [函数调用] swAdminLock");
  pSet = &slSet;
  InitDataset(LOCKFILE,pSet);
  ilRc = OpenTable(NULL,0,LOCKFILE,pSet);
  swDebugset(2,pSet);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"S2190: [函数返回] swAdminLock");
    return;
  }
  ilRc = GetNodeByName(pNode1,"ROWDATA",&pTempNode); 
  if(!ilRc)
  {
    for(i=0;i<pTempNode->iChildNum;i++)
    {
      pTempNode1 = &((pNode)pTempNode->ChildNodes)[i];
      ilRc = GetAttrByName(pTempNode1,"FILENAME",&pAttr);
      if(!ilRc)
      {
        strcpy(slKey.keyname,"FILENAME");
        strcpy(slKey.keyval,pAttr->aAttribVal);
        ilRc = Delete(&slKey,1,pSet);
        if(ilRc)
          break; 
      }
    }
  }
  if(ilRc)
    swErrorInfo(sockfd,ilRc);
  else
  {
    swSuccessInfo(sockfd);
    WriteTable(NULL,0,LOCKFILE,pSet);
  }
  FreeDataset(pSet);
  swVdebug(2,"S2200: [函数返回] swAdminLock");
}

int swImportTable(int sockfd,char *aTableName,keyrec *pKeys,short iKeyNum,
                        pRecord pRec,short iRecNum)
{
  Dataset slSet;
  int ilRc,i;

  swVdebug(2,"S2210: [函数调用] swImportTable");
  swDebugkeys(2,pKeys,iKeyNum);
  swDebugrecs(2,pRec,iRecNum);
  InitDataset("SWREADLOG",&slSet);

  for(i=0;i<iRecNum;i++)
  {
    Insert(&pRec[i],&slSet);
  } 
  swDebugset(2,&slSet);
  ilRc = ImportTable(aTableName,pKeys,iKeyNum,&slSet);
  swVdebug(2,"S2220: [函数返回] swImportTable,返回码=%d",ilRc);
  return(ilRc);
}

int swExportTable(int sockfd,char *aTableName,keyrec *pKeys,short iKeyNum,
                        pRecord pRec,short iRecNum)
{
  Dataset slSet,slRtnSet,*pSet;
  int ilRc;

  swVdebug(2,"S2230: [函数调用] swExportTable");
  swDebugkeys(2,pKeys,iKeyNum);
  swDebugrecs(2,pRec,iRecNum);
  pSet = &slSet;
  InitDataset(aTableName,pSet);
  ilRc = OpenTable(pKeys,iKeyNum,aTableName,pSet);
  if(ilRc)
  {
    swVdebug(2,"S2240: [函数返回] swExportTable,返回码=%d",ilRc);
    return(ilRc);
  }
 
  swDebugset(2,pSet);

  InitDataset("SWREADLOG",&slRtnSet); 

  ilRc = ExportTable(pKeys,iKeyNum,pRec,iRecNum,pSet,&slRtnSet);
  if(ilRc)
  {
    swVdebug(2,"S2250: [函数返回] swExportTable,返回码=%d",ilRc);
    return(ilRc);
  }
  swDebugset(2,&slRtnSet);
  ilRc = ReadContent(sockfd,NULL,0,&slRtnSet);
  swVdebug(2,"S2260: [函数返回] swExportTable,返回码=%d",ilRc);
  return(ilRc); 
}

/* 读报警事件配置 */
int ReadEvents(int sockfd)
{
  char alBuf[8193],alFileName[1024];
  int ilRc;
  int ilLen;
  Node slRootNode;
  
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/SWALERT.XML",getenv("SWITCH_CFGDIR")); */
  sprintf(alFileName,"%s/config/SWALERT.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  if(ilRc)
  {
    swVdebug(2,"S2270:  解析XML文件失败!返回[%d]",ilRc);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  
  ilLen = 0;
  ilRc = WriteNodeToBuf(sockfd,alBuf,&ilLen,&slRootNode,1);
  DestroyNode(&slRootNode);
  
  swVdebug(2,"ilRc = %d",ilRc);
  if(ilRc)
    return(ilRc);
  swEndInfo(sockfd); 
  return(0);
}

/* GetCatalogNode 找到(创建)分类代码相应的节点 */

pNode GetCatalogNode(pNode psNode,char *aCataCode,char *aCataDesc)
{
  int i,ilFlag,ilRc;
  pNode pslNode;
  pAttrib pslAttr;
  
  ilFlag = 0;
  for(i = 0;i < psNode->iChildNum;i++)
  {
    pslNode = &((pNode)psNode->ChildNodes)[i];
    ilRc = GetAttrByName(pslNode,"CODE",&pslAttr);
    if(ilRc)continue;
    if(strcmp(pslAttr->aAttribVal,aCataCode) == 0)
    {
      return pslNode;
    }
  }
  pslNode = AddChildNode(psNode);
  strcpy(pslNode->aNodeName,"CATALOG");
  AddAttrib(pslNode,"CODE",aCataCode);
  AddAttrib(pslNode,"DESC",aCataDesc);
  return pslNode;
}

/* 增加一个报警事件 */
/* psNode是经解析后的前台命令报文 */
int AddEvent(int sockfd,pNode psNode)
{
  pNode pslNode,pslNode1,pslTmp;
  int ilRc;
  int ilLen;
  char alFileName[1024],alCataCode[10],alCataDesc[100];
  Node slRootNode;
  pAttrib  pslAttr;
  
    /* 将配置文件读入并解析 */
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/SWALERT.XML",getenv("SWITCH_CFGDIR")); */
  sprintf(alFileName,"%s/config/SWALERT.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  if(ilRc)
  {
    swVdebug(2,"S2280: AddEvent 解析XML文件失败!返回[%d]",ilRc);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  /* 找到（创建）该事件分类节点 */
  ilRc = GetNodeByName(psNode,"ROWDATA",&pslNode1);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到ROWDATA节点!");
    return ilRc;
  }
  ilRc = GetNodeByName(pslNode1,"ROW",&pslNode1);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到ROW节点!");
    return ilRc;
  }
  
  pslNode = pslNode1; 
 
 /*
  ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到KEYS节点!");
    return ilRc;
  }
  */
  ilRc = GetAttrByName(pslNode,"CATACODE",&pslAttr);
  if(ilRc)
  {
    swVdebug(2,"S2290: 未传送分类!");
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  strcpy(alCataCode,pslAttr->aAttribVal);
  
  ilRc = GetAttrByName(pslNode,"CATADESC",&pslAttr);
  if(ilRc)
  {
    alCataDesc[0] = 0;
  }
  strcpy(alCataDesc,pslAttr->aAttribVal);
  
  pslNode = GetCatalogNode(&slRootNode,alCataCode,alCataDesc);
  
  /* 将事件加入该分类节点 */
  
  pslTmp = AddChildNode(pslNode);
  strcpy(pslTmp->aNodeName,"EVENT");
  ilRc = GetAttrByName(pslNode1,"CODE",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"CODE",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"DESC",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"DESC",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"TYPE",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"TYPE",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"TRIG",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"TRIG",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"MATCHFLD",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"MATCHFLD",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"MATCHVAL",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"MATCHVAL",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"KEYFLD",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"KEYFLD",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"SHELL",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"SHELL",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"PARAM",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"PARAM",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"TIMES",&pslAttr);
  if(!ilRc)
  {
    AddAttrib(pslTmp,"TIMES",pslAttr->aAttribVal);
  }
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  
  DestroyNode(&slRootNode);
  
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"S2300: 写入XML文件出错");
    return ilRc;
  }
  swVdebug(3,"S2310: OK AddEvent");
  swSuccessInfo(sockfd); 
  return 0;
}

/* 读某个事件 */
int ReadEvent(int sockfd,pNode psNode)
{
  pNode pslNode,pslNode1;
  int ilRc,i,ilFlag;
  int ilLen;
  char alFileName[1024],alCataCode[30],alCode[100],alBuf1[8193];
  Node slRootNode;
  pAttrib  pslAttr;
  
    /* 将配置文件读入并解析 */
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/SWALERT.XML",getenv("SWITCH_CFGDIR")); */
  sprintf(alFileName,"%s/config/SWALERT.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  if(ilRc)
  {
    swVdebug(2,"S2320: AddEvent 解析XML文件失败!返回[%d]",ilRc);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  /* 找到（创建）该事件分类节点 */
  ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到KEYS节点!");
    return ilRc;
  }
  ilRc = GetAttrByName(pslNode,"CATACODE",&pslAttr);
  if(ilRc)
  {
    swVdebug(2,"S2330: 未传送分类!");
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  strcpy(alCataCode,pslAttr->aAttribVal);
  
  ilRc = GetAttrByName(pslNode,"CODE",&pslAttr);
  if(ilRc)
  {
    swVdebug(2,"S2340: 未传送事件代码!");
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  strcpy(alCode,pslAttr->aAttribVal);
  
  pslNode = GetCatalogNode(&slRootNode,alCataCode,"");
  /* 找到事件节点 */
  ilFlag = 0;
  for(i = 0;i < pslNode->iChildNum;i++)
  {
    pslNode1 = &((pNode)pslNode->ChildNodes)[i];
    ilRc = GetAttrByName(pslNode1,"CODE",&pslAttr);
    if(ilRc)continue;
    if(strcmp(pslAttr->aAttribVal,alCode) == 0)
    {
      ilFlag = 1;
      break;
    }
  }
  if(ilFlag == 0)
  {
    swVdebug(2,"S2350: 事件节点未找到!");
    swErrorInfo(sockfd,114);
    return(114);
  }
  ilLen = 0;
  ilRc = WriteNodeToBuf(sockfd,alBuf1,&ilLen,pslNode1,1);
  swVdebug(2,"%s",alBuf1);
  DestroyNode(&slRootNode);
  
  if(ilRc)
    return(ilRc);
  swEndInfo(sockfd); 
  return(0);
}

int EditAttrib(pNode psNode,char *aName,char *aValue)
{
  pAttrib pslAttr;
  int ilRc;
  
  ilRc = GetAttrByName(psNode,aName,&pslAttr);
  if(ilRc)
  {
    pslAttr = AddAttrib(psNode,aName,aValue);
  }
  strcpy(pslAttr->aAttribVal,aValue);
  return 0;
}

/* 修改一个报警事件 */
/* psNode是经解析后的前台命令报文 */
int EditEvent(int sockfd,pNode psNode)
{
  pNode pslNode,pslNode1,pslTmp;
  int ilRc,i;
  int ilLen,ilFlag;
  char alFileName[1024],alCataCode[10],alCode[100],alCataDesc[512];
  Node slRootNode;
  pAttrib  pslAttr;
  
    /* 将配置文件读入并解析 */
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/SWALERT.XML",getenv("SWITCH_CFGDIR"));*/
  sprintf(alFileName,"%s/config/SWALERT.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  if(ilRc)
  {
    swVdebug(2,"S2360: AddEvent 解析XML文件失败!返回[%d]",ilRc);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  /* 找到（创建）该事件分类节点 */
  ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到KEYS节点!");
    return ilRc;
  }
  ilRc = GetAttrByName(pslNode,"CATACODE",&pslAttr);
  if(ilRc)
  {
    swVdebug(2,"S2370: 未传送分类!");
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  strcpy(alCataCode,pslAttr->aAttribVal);
  
  ilRc = GetAttrByName(pslNode,"CODE",&pslAttr);
  if(ilRc)
  {
    alCode[0] = 0;
  }
  strcpy(alCode,pslAttr->aAttribVal);
  
  pslNode = GetCatalogNode(&slRootNode,alCataCode,alCataDesc);
  
  /* 找到事件节点 */
  ilFlag = 0;
  for(i = 0;i < pslNode->iChildNum;i++)
  {
    pslTmp = &((pNode)pslNode->ChildNodes)[i];
    ilRc = GetAttrByName(pslTmp,"CODE",&pslAttr);
    if(ilRc)continue;
    if(strcmp(pslAttr->aAttribVal,alCode) == 0)
    {
      ilFlag = 1;
      break;
    }
  }
  if(ilFlag == 0)
  {
    swVdebug(2,"S2380: 事件节点未找到!");
    swErrorInfo(sockfd,114);
    return(114);
  }
 
  /* 修改事件 */
  ilRc = GetNodeByName(psNode,"ROWDATA",&pslNode1);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到ROWDATA节点!");
    return ilRc;
  }
  ilRc = GetNodeByName(pslNode1,"ROW",&pslNode1);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到ROW节点!");
    return ilRc;
  }
  
  ilRc = GetAttrByName(pslNode1,"CODE",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"CODE",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"DESC",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"DESC",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"TYPE",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"TYPE",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"TRIG",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"TRIG",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"MATCHFLD",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"MATCHFLD",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"MATCHVAL",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"MATCHVAL",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"KEYFLD",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"KEYFLD",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"SHELL",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"SHELL",pslAttr->aAttribVal);
  }
  ilRc = GetAttrByName(pslNode1,"PARAM",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"PARAM",pslAttr->aAttribVal);
  }
  
  ilRc = GetAttrByName(pslNode1,"TIMES",&pslAttr);
  if(!ilRc)
  {
    EditAttrib(pslTmp,"TIMES",pslAttr->aAttribVal);
  }
  
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  
  DestroyNode(&slRootNode);
  
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"S2390: 写入XML文件出错");
    return ilRc;
  }
  swVdebug(3,"S2400: OK AddEvent");
  swSuccessInfo(sockfd); 
  return 0;
}

/* 删除一个报警事件 */
/* psNode是经解析后的前台命令报文 */
int DeleteEvent(int sockfd,pNode psNode)
{
  pNode pslNode,pslTmp;
  int ilRc,i;
  int ilLen,ilFlag;
  char alFileName[1024],alCataCode[10],alCode[100];
  Node slRootNode;
  pAttrib  pslAttr;
  
    /* 将配置文件读入并解析 */
  /* modify by nh 20020924
  sprintf(alFileName,"%s/config/SWALERT.XML",getenv("SWITCH_CFGDIR"));*/
  sprintf(alFileName,"%s/config/SWALERT.XML",agPath);
  ilRc = ParseXmlFile(alFileName,&slRootNode,&ilLen);
  if(ilRc)
  {
    swVdebug(2,"S2410: AddEvent 解析XML文件失败!返回[%d]",ilRc);
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  /* 找到（创建）该事件分类节点 */
  ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"未找到KEYS节点!");
    return ilRc;
  }
  ilRc = GetAttrByName(pslNode,"CATACODE",&pslAttr);
  if(ilRc)
  {
    swVdebug(2,"S2420: 未传送分类!");
    swErrorInfo(sockfd,ilRc);
    return(ilRc);
  }
  strcpy(alCataCode,pslAttr->aAttribVal);
  
  ilRc = GetAttrByName(pslNode,"CODE",&pslAttr);
  if(ilRc)
  {
    swVdebug(2,"S2430: 未传送事件代码!");
    swErrorInfo(sockfd,ilRc);
    return(ilRc);;
  }
  strcpy(alCode,pslAttr->aAttribVal);
  
  pslNode = GetCatalogNode(&slRootNode,alCataCode,"");
  
  /* 找到事件节点 */
  ilFlag = 0;
  for(i = 0;i < pslNode->iChildNum;i++)
  {
    pslTmp = &((pNode)pslNode->ChildNodes)[i];
    ilRc = GetAttrByName(pslTmp,"CODE",&pslAttr);
    if(ilRc)continue;
    if(strcmp(pslAttr->aAttribVal,alCode) == 0)
    {
      ilFlag = 1;
      break;
    }
  }
  if(ilFlag == 0)
  {
    swVdebug(2,"S2440: 事件节点未找到!");
    swErrorInfo(sockfd,114);
    return(114);
  }
 
  /* 删除事件 */
  pslTmp->aNodeName[0] = 0;
  
  /* 写入文件 */
  ilRc = WriteXmlFile(alFileName,&slRootNode);
  DestroyNode(&slRootNode);
  
  if(ilRc)
  {
    swErrorInfo(sockfd,ilRc);
    swVdebug(2,"S2450: 写入XML文件出错");
    return ilRc;
  }
  swVdebug(3,"S2460: OK AddEvent");
  swSuccessInfo(sockfd); 
  return 0;
}


void swProcessTrans(int sockfd,short iCmdCode,pNode psNode)
{
  pAttrib pslAttr;
  pNode pslNode;
  int ilRc;
  char alCatalog[512],alContent[512],alQid[11],alTranCode[101],alGroup[21];
  char alRouteGrp[10],alFmtGrp[10],alDesc[512],alTranStep[101],alTranType[101];
  char alRouteQid[20],alRouteTranCode[101];
  
  alRouteQid[0] = 0;
  alRouteTranCode[0] = 0;
  alCatalog[0] = 0;
  alContent[0] = 0;
  alQid[0] = 0;
  alTranCode[0] = 0;
  alGroup[0] = 0;
  
  alRouteGrp[0] = 0;
  alFmtGrp[0] = 0;
  alDesc[0] = 0;
  
  switch(iCmdCode)
  {
    case 1901:        /* 读交易树 */
      swVdebug(2,"S0901:读交易树内容!");
      ReadTrans(sockfd);
      break;
    case 1902:        /* 读交易信息 */
     swVdebug(2,"S0901:读交易信息!");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      ilRc = GetAttrByName(pslNode,"QID",&pslAttr);
      if(ilRc != 0)
      {
        swErrorInfo(sockfd,34);
        return;
      }
      strcpy(alQid,pslAttr->aAttribVal);
      ilRc = GetAttrByName(pslNode,"TRANCODE",&pslAttr);
      if(ilRc != 0)
      {
        swErrorInfo(sockfd,34);
        return;
      }
      strcpy(alTranCode,pslAttr->aAttribVal);
      swVdebug(2,"S0789 : TRANSKEY = %s",pslAttr->aAttribVal);
      swVdebug(2,"S1902: invoke ReadTraninfo begin");
      ReadTranInfo(sockfd,alQid,alTranCode);
      break;
    case 1903:       /* 读路由相关TDF */
     swVdebug(2,"S0904: 读路由相关TDF");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      ilRc = GetAttrByName(pslNode,"ROUTEQID",&pslAttr);
      if(ilRc != 0)
      {
        swErrorInfo(sockfd,23);
        return;
      }
      strcpy(alQid,pslAttr->aAttribVal);
      ilRc = GetAttrByName(pslNode,"ROUTETRANCODE",&pslAttr);
      if(ilRc != 0)
      {
        swErrorInfo(sockfd,23);
        return;
      }
      strcpy(alTranCode,pslAttr->aAttribVal);
      swVdebug(2,"S1903: invoke ReadRouteTDF begin");
      ReadRouteTDF(sockfd,alQid,alTranCode);
      break;
    case 1904:       /* 新建交易分类 */
     swVdebug(2,"S0905: 新建交易分类");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      
      ilRc = GetAttrByName(pslNode,"PARENT",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alCatalog,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"CONTENT",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alContent,pslAttr->aAttribVal);
      }
      swVdebug(2,"S1904: invoke AddCatalog begin");
      AddCatalog(sockfd,alCatalog,alContent);
      break;
    case 1905:     /* 新建交易 */
     swVdebug(2,"S1905: 新建交易");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      
      ilRc = GetAttrByName(pslNode,"CATALOG",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alCatalog,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"CONTENT",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alContent,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"QID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alQid,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranCode,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"GROUP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alGroup,pslAttr->aAttribVal);
      }
      swVdebug(2,"S1905: invoke AddTran begin");
      AddTran(sockfd,alCatalog,alContent,alQid,alTranCode,alGroup);
      break;
    case 1906:     /* 修改分类 */
      swVdebug(2,"S0904: 修改分类");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      
      ilRc = GetAttrByName(pslNode,"CATALOGOLD",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alCatalog,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"CATALOGNEW",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alContent,pslAttr->aAttribVal);
      }
      swVdebug(2,"S1906: invoke EditCatalog begin");
      EditCatalog(sockfd,alCatalog,alContent);
      break;
    case 1907:    /* 修改交易 */
     swVdebug(2,"S0904: 修改交易");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      
      ilRc = GetAttrByName(pslNode,"ROUTEQID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alRouteQid,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"ROUTETRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alRouteTranCode,pslAttr->aAttribVal);
      }

      ilRc = GetAttrByName(pslNode,"CONTENT",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alContent,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"QID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alQid,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranCode,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"GROUP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alGroup,pslAttr->aAttribVal);
      }
      swVdebug(2,"S1907: invoke EditTran begin");
      EditTran(sockfd,alRouteQid,alRouteTranCode,
                  alContent,alQid,alTranCode,alGroup);
      break;
    case 1908:     /* 删除交易分类 */
     swVdebug(2,"S0904: 删除交易分类");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      
      ilRc = GetAttrByName(pslNode,"CATALOG",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alCatalog,pslAttr->aAttribVal);
      }
      swVdebug(2,"S1908: invoke DeleteCatalog begin");
      DeleteCatalog(sockfd,alCatalog);
      break;
    case 1909:          /* 删除交易 */
     swVdebug(2,"S0904: 删除交易");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      
      ilRc = GetAttrByName(pslNode,"QID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alQid,pslAttr->aAttribVal);
      }
      ilRc = GetAttrByName(pslNode,"TRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranCode,pslAttr->aAttribVal);
      }
      swVdebug(2,"S1909: invoke DeleteTran begin");
      DeleteTran(sockfd,alQid,alTranCode);
      break;
    case 1910:          /* 将TDF与交易关联 */
     swVdebug(2,"S1910: 读TDF与交易关联 ");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      ilRc = GetAttrByName(pslNode,"ROUTEQID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alRouteQid,pslAttr->aAttribVal);
      }
      ilRc = GetAttrByName(pslNode,"ROUTETRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alRouteTranCode,pslAttr->aAttribVal);
      }
      ilRc = GetAttrByName(pslNode,"FMTGRP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alFmtGrp,pslAttr->aAttribVal);
      }
      ilRc = GetAttrByName(pslNode,"DESC",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alDesc,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"QID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alQid,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranCode,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANSTEP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranStep,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANTYPE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranType,pslAttr->aAttribVal);
      }
      swVdebug(2,"S1910: invoke AttachTDF2Route begin");
      AttachTDF2Route(sockfd,alRouteQid,alRouteTranCode,alFmtGrp,alDesc,alQid,alTranCode,alTranStep,alTranType);
      break;
    case 1911:        /* 拆除TDF和路由组的关联 */
      swVdebug(2,"S1911: 拆除TDF和路由组的关联");
      ilRc = GetNodeByName(psNode,"KEYS",&pslNode);
      if(ilRc)
      {
        swErrorInfo(sockfd,23);
        break;
      }
      ilRc = GetAttrByName(pslNode,"ROUTEQID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alRouteQid,pslAttr->aAttribVal);
      }
      ilRc = GetAttrByName(pslNode,"ROUTETRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alRouteTranCode,pslAttr->aAttribVal);
      }
      ilRc = GetAttrByName(pslNode,"FMTGRP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alFmtGrp,pslAttr->aAttribVal);
      }
/*  add by zjj 2002.08.20 */
      ilRc = GetAttrByName(pslNode,"QID",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alQid,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANCODE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranCode,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANSTEP",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranStep,pslAttr->aAttribVal);
      }
      
      ilRc = GetAttrByName(pslNode,"TRANTYPE",&pslAttr);
      if(ilRc == 0)
      {
        strcpy(alTranType,pslAttr->aAttribVal);
      }       
/* end add by zjj 2002.08.20 */    
     swVdebug(2,"S1911: invoke DettachTDFFromRoute begin");      
      DettachTDFFromRoute(sockfd,alRouteQid,alRouteTranCode,alFmtGrp,alQid,alTranCode,alTranStep,alTranType);      
      break;
    case 1920:
      ReadEvents(sockfd);
      break;
    case 1921:
      ReadEvent(sockfd,psNode);
      break;
    case 1922:
      AddEvent(sockfd,psNode);
      break;
    case 1923:
      EditEvent(sockfd,psNode);
      break;
    case 1924:
      DeleteEvent(sockfd,psNode);
      break;
    default:
      swErrorInfo(sockfd,104);
      break;
  }
}

void swClsprocess(int sockfd)
{
  char   alMsgpack[MAXPACKLEN+1],alTableName[30],aAddr[20];
  int    i,j,k,ilErrorPos,ilCmdCode,ilKeyNum = 0,ilRecNum = 0;
  unsigned int ilMsglen;
  int    ilRc;
  Node   RootNode,*pTempNode,*pTempNode1;
  Attrib *pAttr;
  keyrec slKeys[10];
  pRecord pRec;
  Dataset slSet;
  
  swVdebug(2,"S2490: [函数调用] swClsprocess");
  
  pRec = NULL;  /* add by zjj 20004.03.06 */
  for(i=0;i<10;i++)
  {
    slKeys[i].keyname[0] = 0;
    slKeys[i].keyval[0] = 0;
  }

  ilMsglen = MAXPACKLEN; 
  ilRc = swTcpGet(sockfd,alMsgpack,&ilMsglen,0);
  if(ilRc)
  {
    swVdebug(2,"swTcpGet error.[%s]",agErrorMessage[ilRc-100]);
    swErrorInfo( sockfd,ilRc);
    swVdebug(2,"S2500: [函数返回] swClsprocess");
    return;
  }
 /* swDebughex_l(3,alMsgpack,ilMsglen);  */
  if (cgDebug > 1) swDebughex(alMsgpack,ilMsglen);
  InitNode(&RootNode);

  ilRc = ParseXml((char*)(alMsgpack+8),ilMsglen,&RootNode,&ilErrorPos);
  if(ilRc)
  {
    swVdebug(2,"S2510: ParseXml error.[%s],position = [%d]",
          agErrorMessage[ilRc-100],ilErrorPos);
    swErrorInfo( sockfd,ilRc);
    DestroyNode(&RootNode);
    return;
  }
   
  swDebugnode(1,&RootNode);
  ilRc = GetAttrByName(&RootNode,"code",&pAttr);
  if(ilRc)
  {
    swVdebug(2,"S2520: No command code!.");
    swErrorInfo( sockfd,102);
    DestroyNode(&RootNode);
    return;
  }
  ilCmdCode = atoi(pAttr->aAttribVal);
  
  if(ilCmdCode > 1900)
  {
    swProcessTrans(sockfd,ilCmdCode,&RootNode);
    return;
  }
  
  ilRc = GetAttrByName(&RootNode,"tablename",&pAttr);
  if(!ilRc)
  {
    strcpy(alTableName,pAttr->aAttribVal);
    swVdebug(2,"S2530: Table name = [%s]",alTableName);
    InitDataset(alTableName,&slSet);
  }
  else
    alTableName[0] = 0;
 
  ilRc = GetAttrByName(&RootNode,"ipaddr",&pAttr);
  if(!ilRc)
  {
    strcpy(aAddr,pAttr->aAttribVal);
  }
  else
    aAddr[0] = 0;

  ilKeyNum = 0;
  ilRc = GetNodeByName(&RootNode,"KEYS",&pTempNode);
  if(!ilRc)
  {
    ilKeyNum = pTempNode->iAttrNum;
    for(i =0;i <ilKeyNum;i++)
    {
      strcpy(slKeys[i].keyname,pTempNode->Attribs[i].aAttribName);
      strcpy(slKeys[i].keyval,pTempNode->Attribs[i].aAttribVal);
    }
  }

  ilRc = GetNodeByName(&RootNode,"ROWDATA",&pTempNode);
  if(!ilRc)
  {
    ilRecNum = pTempNode->iChildNum;
    pRec = (pRecord)malloc(ilRecNum * sizeof(Record));
    for(i = 0;i < ilRecNum;i++)
      InitRecord(&pRec[i]);
    for(i =0;i <ilRecNum;i++)
    {
      for(j=0;j<MAXFLDNUM;j++)
      {
        strcpy(pRec[i].Values[j].aValue,"-");
      }
      pTempNode1 = &((pNode)(pTempNode->ChildNodes))[i];
      if(ilCmdCode == 918)
      {
          strcpy(pRec[i].Values[0].aValue,
                      pTempNode1->Attribs[0].aAttribVal);
      }
      else
      for(j =0;j <pTempNode1->iAttrNum;j++)
      {
        if(alTableName[0] == 0)
        {
          strcpy(pRec[i].Values[j].aValue,
                      pTempNode1->Attribs[j].aAttribVal);
        }
        else
        {
          for(k = 0;k < slSet.iFieldNum;k++)
          {
            if(strcmp(pTempNode1->Attribs[j].aAttribName,
                      slSet.Fields[k].aFieldName) == 0)
            {
               swVdebug(3,"S2540: %s = %s",
                      pTempNode1->Attribs[j].aAttribName,
                      slSet.Fields[k].aFieldName);
               strcpy(pRec[i].Values[k].aValue,
                       pTempNode1->Attribs[j].aAttribVal);
               break; 
            }
          }
        }
      }
    }
  }
  
  switch(ilCmdCode)
  {
    case 909:
      ilRc = swAddLock(aAddr,alTableName);
      if(ilRc)
      {
        swErrorInfo(sockfd,ilRc);
      }
      else
        swSuccessInfo(sockfd);
      break;
    case 913:
       ilRc = swDeleteLock(aAddr,alTableName);
      if(ilRc)
      {
        swErrorInfo(sockfd,ilRc);
      }
      else
        swSuccessInfo(sockfd);
      break;
    case 914:
    case 915:
    case 905:   /* add by zjj  2001.10.26 */
    case 903:   /* add by zjj  2001.10.26 */
    case 904:   /* add by zjj  2001.10.26 */
    case 920:   /* add by zjj  2001.10.26 */
    case 907:
    case 908:
    case 910:
    case 911:
    case 912:
      if(ilCmdCode == 915)
        strcpy(alTableName,LOCKFILE);
      if(ilCmdCode == 914)
        strcpy(alTableName,"ENVIRON");
      /* add by zjj 2001.10.26      */  
      if ((ilCmdCode == 903) || (ilCmdCode == 904))  
        strcpy(alTableName,"ENVIRON");   
      /* end add by zjj 2001.10.26  */
      if(alTableName[0] == 0)
      {
        swVdebug(2,"S2550: TableName NULL!");
        swErrorInfo(sockfd,154);
        break;
      }
       swProcessData(sockfd,      ilCmdCode,
                                  alTableName,
                                  slKeys,
                                  ilKeyNum,
                                  pRec,
                                  ilRecNum,
                                  aAddr);
      break;
    case 916:
        swAdminLock(sockfd,&RootNode);
      break;
    case 917:
      ilRc = swExportTable(sockfd,alTableName,slKeys,ilKeyNum,pRec,ilRecNum);
      /*   add by zjj 2001.10.25  */
      if (ilRc)
        swErrorInfo(sockfd,ilRc);
      else
        swEndInfo(sockfd);
      /*  end add  */
      break;
    case 918:
      ilRc = swImportTable(sockfd,alTableName,slKeys,ilKeyNum,pRec,ilRecNum);
      if(ilRc)
      {
        swErrorInfo(sockfd,ilRc);
      }
      else
        swEndInfo(sockfd);
      break;
    default:
      swVdebug(2,"S2560: Invalid Command");
      swErrorInfo(sockfd,104);
      break;
  }
  DestroyNode(&RootNode);
  free(pRec);
  swVdebug(2,"S2570: [函数返回] swClsprocess");
}

