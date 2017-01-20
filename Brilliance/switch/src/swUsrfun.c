/****************************************************************/
/* 模块编号    ：USRFUN                                         */ 
/* 模块名称    ：自定义函数处理                                 */
/* 作	 者    ：张辉                                           */
/* 建立日期    ：1999/11/18                                     */
/* 最后修改日期：2001/2/27                                      */
/* 模块用途    ：实现对自定义函数的处理                         */
/* 本模块中包含如下函数及功能说明：                             */
/****************************************************************/
/*       int  _swExpress();                                     */
/*       int  _swExpN2T();                                      */
/*       int  _swExpT2N();                                      */
/*       int  _swParseN2T();                                    */
/*       int  _swParseT2N();                                    */
/*       int  _swGetFuncAndParmN2T();                           */
/*       int  _swGetFuncAndParmT2N();                           */
/*       int  _swUsrfunSort();                                  */
/*       int  _swGetfuncPtr();                                  */
/*       int  _swGetextfuncPtr();                               */
/*--------------------------------------------------------------*/   
/*       int  _ufNtoT();                                        */      
/*       int  _ufTtoN();                                        */
/*--------------------------------------------------------------*/
/*       int  _ufProFML();                                      */
/*       int  _ufProNSP();                                      */
/*       int  _ufProSEP();                                      */
/*       int  _ufProISO();                                      */
/*       int  _ufProMSGHEAD();                                  */
/*       int  _ufProVAR();                                      */
/*       int  _ufProMVAR();                                     */
/*       int  _ufProNFML();                                     */
/*       int  _ufProMFML();                                     */
/*       int  _ufProREC();                                      */
/*       int  _ufProFLD();                                      */
/*       int  _ufProMEMOSET();                                  */
/*--------------------------------------------------------------*/
/*       int  _ufProLFT();                                      */
/*       int  _ufProRIG();                                      */
/*       int  _ufProMID();                                      */
/*       int  _ufProSAD();                                      */
/*       int  _ufProSPACE();                                    */
/*       int  _ufProTRIM();                                     */
/*       int  _ufProTRIML();                                    */
/*       int  _ufProTRIMR();                                    */
/*       int  _ufProFILL();                                     */
/*       int  _ufProREP();                                      */
/*       int  _ufProLEN();																			*/	  
/*			 int	_ufProUPPER();				                            */
/*			 int  _ufProLOWER();																		*/
/*--------------------------------------------------------------*/
/*       int  _ufProSEQ();                                      */
/*       int  _ufProMAT();                                      */
/*       int  _ufProGT();                                       */
/*       int  _ufProGE();                                       */
/*       int  _ufProLT();                                       */
/*       int  _ufProNE()                                        */
/*       int  _ufProHEX()                                       */ 
/*       int  _ufProLE();                                       */
/*       int  _ufProIF();                                       */
/*       int  _ufProCASEIF();                                   */
/*       int  _ufProCASEVAR();                                  */
/*       int  _ufProNOT();                                      */
/*       int  _ufProAND();                                      */
/*       int  _ufProOR();                                       */
/*--------------------------------------------------------------*/
/*       int  _ufProADD();                                      */
/*       int  _ufProMOD();                                      */
/*--------------------------------------------------------------*/
/*       int  _ufProCHR();                                      */
/*       int  _ufProSHORT();                                    */
/*       int  _ufProLONG();                                     */
/*       int  _ufProINT();                                      */
/*       int  _ufProDOUBLE();                                   */
/*       int  _ufProDTOC();                                     */
/*       int  _ufProUSHORT();                                   */
/*       int  _ufProULONG();                                    */
/*--------------------------------------------------------------*/
/*       int  _ufProGETTIME();                                  */
/*       int  _ufProTODATE();                                   */
/*       int  _ufProTOTIME();                                   */
/*--------------------------------------------------------------*/
/*       int  _ufProPID();                                      */
/*       int  _ufProGETSTR();                                   */
/*       int  _ufProGETMSG();                                   */
/*       int  _ufProDATEtoLONG();                               */
/*       int  _ufProPackSEP();                                  */
/*       int  _ufPropGETSEP();                                  */
/*       int  _ufProBITSET();                                   */
/*       int  _ufProBITGET();                                   */
/*--------------------------------------------------------------*/
/*       int  _ufProPORTSET();                                  */
/*       int  _ufProPORTGET();                                  */
/****************************************************************/
/* 修改记录：                                                   */
/****************************************************************/
/* switch定义 */
#include "switch.h"
#include "swUsrfun.h"

/*add by zcd 20141218*/
#ifdef LIBXML2
#include "swxml.h"
#endif
/*end of add by zcd 20141218*/


static short cgXmlInit=0;
/**************************************************************
 ** 函 数 名：_swExpress
 ** 功    能：计算函数表达式
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：aExpress   ：输入表达式
 ** 　　　　　aResult    : 输出的结果
 **           piResultlen：输出串长度
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _swExpress(char *aExpress, char *aResult,  short *piResultlen)
{
  static int ilFlag=0;
  int i,j,ilStack,ilPoint,ilCnt,ilParmCount,ilRc;
  char palStack[256][iFLDVALUELEN];
  char alField[iFLDVALUELEN];
  char *alFunc;
  char alResult[iFLDVALUELEN];
  char palParm[10][iFLDVALUELEN];

  ilStack=0;
  ilPoint=0;
   
  for(;;)
  {
   
    for(i=0;;i++)
    {
      if((aExpress[ilPoint]==',')||(aExpress[ilPoint]=='\0')) break;
      alField[i]=aExpress[ilPoint];
      ilPoint++;
    }
    
    alField[i]='\0';
    if(alField[0]==')')
    {
      alFunc=alField+1;
      for(i=0;;i++)
      {
        ilStack--;
        if(ilStack<0)
        {
          return(-1); /* 右括号太多 */
        }
        if(palStack[ilStack][0]=='(') break;
      }

      for(j=0;j<i;j++)
      {
        strcpy(palParm[j],palStack[ilStack+j+1]);
      }
    
      /* 计算alResult */
      /* 首次调用排序 */
      if(!ilFlag)
      {
        _swUsrfunSort();
        ilFlag=1;
      }
      ilCnt=_swGetfuncPtr(alFunc);
      ilParmCount=i;
      if(ilCnt>=0)
      {
        if(proclist[ilCnt].fproc!=NULL)
        {
          ilRc=(*(proclist[ilCnt].fproc))(palParm,ilParmCount,alResult);
          if(ilRc)
          {
            swVdebug(0,"S8000: [错误/其它] 平台函数{%s}返回值出错",alFunc);  
            return(-1);
          }
        }
        else if(proclist[ilCnt].fproc_old!=NULL)
        {
          strcpy(alResult,(*(proclist[ilCnt].fproc_old))(palParm));
        }
        else
        {
          swVdebug(0,"S8003: [错误/其它] 平台函数{%s}在函数列表中没有定义",alFunc);
          return(-1);
        }  
      }
      else
      {
        /* 扩展平台函数 */
        ilCnt= _swGetextfuncPtr(alFunc);
        ilParmCount=i;
        if (ilCnt>=0)
        {
          if(pextproclist[ilCnt].fproc!=NULL)
          {
/*swVdebug(0,"1 PC ADD [%s] palParm[%s][%s] ilParmCount[%d] alResult[%s]",alFunc,palParm[0],palParm[1],ilParmCount,alResult);            */

          	
            ilRc=(*(pextproclist[ilCnt].fproc))(palParm,ilParmCount,alResult);
            if(ilRc)
            {
              swVdebug(0,"S8006: [错误/其它] 平台函数{%s}返回值出错",alFunc);  
              return(-1);
            }
/*swVdebug(0,"2 PC ADD [%s] palParm[%s][%s] ilParmCount[%d] alResult[%s]",alFunc,palParm[0],palParm[1],ilParmCount,alResult); */

          }
          else if(pextproclist[ilCnt].fproc_old!=NULL)
          {
            strcpy(alResult,(*(pextproclist[ilCnt].fproc_old))(palParm));
          }
          else
          {
            swVdebug(0,"S8009: [错误/其它] 平台函数{%s}在函数列表中没有定义",alFunc);
            return(-1);
          }  
        }
        else
        {
          swVdebug(0,"S8012: [错误/其它] 平台函数{%s}在函数列表中没有定义",alFunc);
          return(-1);
        }
      }
/*swVdebug(0,"PC ADD palStack[%d] [%s]",ilStack,alResult);*/

			/*add by pc 2007-4-7 08:19下午*/
			alResult[strlen(alResult)]=0;
			memset(palStack[ilStack],0x00,sizeof(palStack[ilStack]));
			/*add by pc 2007-4-7 08:19下午*/
      strcpy(palStack[ilStack],alResult);
      ilStack ++;
    }
    else
    {
      strcpy(palStack[ilStack],alField);
      ilStack ++;
    }
    if(aExpress[ilPoint]=='\0') break;
    ilPoint ++;
  }
  if(ilStack!=1)
  {
    return(-1); /* 左括号太多 */
  }
  _ufTtoN(palStack[0],aResult,piResultlen);
  return(0);
}

/**************************************************************
 ** 函 数 名：_swExpN2T
 ** 功    能：将正常表达式转换为逆波兰表达式
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：_swParseN2T
 ** 全局变量：
 ** 参数含义：aNstr: 输入的正常表达式
 ** 　    　aTstr: 输出逆波兰表达式
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _swExpN2T(char *aNstr,char *aTstr)
{
  int ilRc;

  if(strchr(aNstr,'(')==NULL)
    strcpy(aTstr,aNstr);
  else
  {
    ilRc=_swParseN2T(aTstr,aNstr);
    if(ilRc) return(ilRc);
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_swExpT2N
 ** 功    能：将逆波兰表达式转换为正常表达式
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：_swParseT2N
 ** 全局变量：
 ** 参数含义：aTstr: 输入逆波兰表达式
 ** 　    　aNstr: 输出的正常表达式
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _swExpT2N(char *aTstr,char *aNstr)
{
  int ilRc;

  if(strchr(aTstr,'(')==NULL)
    strcpy(aNstr,aTstr);
  else
  {
    ilRc = _swParseT2N(aNstr,aTstr);
    if(ilRc) return(ilRc);
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_swParseN2T
 ** 功    能：正常表达式到逆波兰表达式的递归解析
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：_swGetFuncAndParmN2T
 ** 全局变量：
 ** 参数含义：aResult: 输出结果串
 ** 　    　aStr   : 输入表达式
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _swParseN2T(char *aResult,char *aStr)
{
  int i,ilRc,ilParmCount=0;
  char alParm[MAXVARNUM][iFLDVALUELEN];
  char alFunc[FUNCLEN],alTmp[iFLDVALUELEN];

  ilParmCount=_swGetFuncAndParmN2T(aStr,alFunc,alParm);
  if (ilParmCount == -1)
  {
    swVdebug(0,"S8990: [错误/平台函数]: _swParseN2T(),表达式[%s]不合法!",aStr);
    return(-1);    	
  }  
  _swTrim(alFunc);
  for(i=0;i<ilParmCount;i++)
  {
    if(strchr(alParm[i],'(')!=NULL)
    {
      ilRc=_swParseN2T(alTmp,alParm[i]);
      if(ilRc) return(ilRc);
      strcpy(alParm[i],alTmp);
    }
  }
  aResult[0]='\0';
  strcat(aResult,"(,");
  for(i=0;i<ilParmCount;i++)
  {
    strcat(aResult,alParm[i]);
    strcat(aResult,",");
  }
  strcat(aResult,")");
  strcat(aResult,alFunc);
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_swParseT2N
 ** 功    能：逆波兰表达式到正常表达式的递归解析
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：_swGetFuncAndParmT2N
 ** 全局变量：
 ** 参数含义：aResult: 输出结果串
 ** 　    　aStr   : 输入表达式
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _swParseT2N(char *aResult,char *aStr)
{
  int i,ilRc,ilParmCount=0;
  char alParm[MAXVARNUM][iFLDVALUELEN];
  char alFunc[FUNCLEN],alTmp[iFLDVALUELEN];

  ilParmCount=_swGetFuncAndParmT2N(aStr,alFunc,alParm);
  _swTrim(alFunc);
  for(i=0;i<ilParmCount;i++)
  {
    if(strchr(alParm[i],'(')!=NULL)
    {
      ilRc=_swParseT2N(alTmp,alParm[i]);
      if (ilRc) return(ilRc);
      strcpy(alParm[i],alTmp);
    }
  }
  aResult[0]='\0';
  strcat(aResult,alFunc);
  strcat(aResult,"(");
  for(i=0;i<ilParmCount;i++)
  {
    strcat(aResult,alParm[i]);
    strcat(aResult,",");
  }
  aResult[strlen(aResult)-1]=')';
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_swGetFuncAndParmN2T
 ** 功    能：分离正常表达式串中到函数名和参数列表
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：aStr ：输入表达式串
 ** 　    　aFunc：返回的函数名
 **           aParm：返回的参数表
 ** 返 回 值：串中的参数个数?
***************************************************************/
int _swGetFuncAndParmN2T(char *aStr,char *aFunc,char aParm[][iFLDVALUELEN])
{
  int i,ilStrlen;
  int ilParmCount=0;
  int ilLevel=0,ilEndflag=0;
  int ilPos=0,ilStartpos=0,ilEndpos=0;
  char alTmp[iFLDVALUELEN];
  
  ilStrlen=strlen(aStr);
  for(i=0;i<ilStrlen;i++)
  {
    switch(aStr[i])
    {
      case '(':
        if(ilLevel==0)
        {
          ilStartpos=i;
        }
        ilLevel++;
        break;
      case ')':
        ilLevel--;
        if(ilLevel<=0)
        {
          ilEndpos=i;
          ilEndflag=1;
        }
        break;
      default:
        break;
    }
    if(ilEndflag==1) break;
  }
  if(ilLevel!=0||i==ilStrlen)
  {
    swVdebug(0,"S8015: [错误/平台函数{%s}]: _swGetFuncAndParmN2T()",aStr);
    return(-1);
  }
  memcpy(aFunc,aStr,ilStartpos);
  aFunc[ilStartpos]='\0';
  memcpy(alTmp,aStr+ilStartpos+1,ilEndpos-ilStartpos-1);
  alTmp[ilEndpos-ilStartpos-1]='\0';
  _swTrim(alTmp);
  ilStrlen=strlen(alTmp);
  ilLevel=0;
  ilPos=0;
  for(i=0;i<ilStrlen;i++)
  {
    switch(alTmp[i])
    {
      case '(':
        ilLevel++;
        break;
      case ')':
        ilLevel--;
        break;
      case ',':
        if(ilLevel==0)
        {
          memcpy(aParm[ilParmCount],alTmp+ilPos,i-ilPos);
          aParm[ilParmCount][i-ilPos]='\0';
          _swTrim(aParm[ilParmCount]);
          ilPos=i+1;
          ilParmCount++;
          if(ilParmCount >= MAXVARNUM)
          {
            swVdebug(0,"S8018: [错误/平台函数]: _swGetFuncAndParmN2T() 参数个数超限!"); 
            return(-1);
          }

        }
        break;
      default:
        break;
    }
    if(ilLevel<0) break;
  }
  if(ilLevel!=0)
  {
    swVdebug(0,"S8021: [错误/平台函数]: _swGetFuncAndParmN2T()");
    return(-1);
  }
  if(ilParmCount>0||ilStrlen>0)
  {
    memcpy(aParm[ilParmCount],alTmp+ilPos,ilStrlen-ilPos);
    aParm[ilParmCount][ilStrlen-ilPos]='\0';
    _swTrim(aParm[ilParmCount]);
    aParm[ilParmCount+1][0]='\0';
    ilParmCount++;
  }
  return(ilParmCount);
}

/**************************************************************
 ** 函 数 名：_swGetFuncAndParmT2N
 ** 功    能：分离逆波兰表达式串中的函数名和参数列表
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：aStr ：输入表达式串
 ** 　    　aFunc：返回的函数名
 **           aParm：返回的参数表
 ** 返 回 值：串中的参数个数?
***************************************************************/
int _swGetFuncAndParmT2N(char *aStr,char *aFunc,char aParm[][iFLDVALUELEN])
{
  int i,ilStrlen;
  int ilParmCount=0;
  int ilLevel=0,ilEndflag=0;
  int ilPos=0,ilStartpos=0,ilEndpos=0;
  char alTmp[iFLDVALUELEN];

  ilStrlen=strlen(aStr);
  for(i=0;i<ilStrlen;i++)
  {
    switch(aStr[i])
    {
      case '(':
        if(ilLevel==0)
        {
          ilStartpos=i;
        }
        ilLevel++;
        break;
      case ')':
        ilLevel--;
        if(ilLevel<=0)
        {
          ilEndpos=i;
          ilEndflag=1;
        }
        break;
      default:
        break;
    }
    if(ilEndflag==1) break;
  }
  if(ilLevel!=0||i==ilStrlen)
  {
    swVdebug(0,"S8024: [错误/平台函数]: _swGetFuncAndParmN2T()");
    return(-1);
  }
  memcpy(aFunc,aStr+ilEndpos+1,ilStrlen-ilEndpos);
  aFunc[ilStrlen-ilEndpos]='\0';
  if((ilEndpos-ilStartpos-3)>0)
  {
    memcpy(alTmp,aStr+ilStartpos+2,ilEndpos-ilStartpos-3);
    alTmp[ilEndpos-ilStartpos-3]='\0';
  }
  else
    return(ilParmCount);
  _swTrim(alTmp);
  ilStrlen = strlen(alTmp);
  ilLevel=0;
  ilPos=0;
  for(i=0;i<ilStrlen;i++)
  {
    switch(alTmp[i])
    {
      case '(':
        ilLevel++;
        break;
      case ')':
        ilLevel--;
        break;
      case ',':
        if(ilLevel==0&&alTmp[i+1]!='\0')
        {
          memcpy(aParm[ilParmCount],alTmp+ilPos,i-ilPos);
          aParm[ilParmCount][i-ilPos]='\0';
          ilPos=i+1;
          ilParmCount++;
        }
        break;
      default:
        break;
    }
    if(ilLevel<0) break;
  }
  if(ilLevel!=0)
  {
    swVdebug(0,"S8027: [错误/平台函数]: _swGetFuncAndParmN2T()");
    return(-1);
  }
  if(ilParmCount>0||ilStrlen>0)
  {
    memcpy(aParm[ilParmCount],alTmp+ilPos,ilStrlen-ilPos);
    aParm[ilParmCount][ilStrlen-ilPos]='\0';
    aParm[ilParmCount+1][0]='\0';
    ilParmCount++;
  }
  return(ilParmCount);
}

/**************************************************************
 ** 函 数 名：_swUsrfunSort
 ** 功    能：函数指针数组按函数名升序排序
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：
 ** 返 回 值：0
***************************************************************/
int _swUsrfunSort()
{
  int i,j;
  PROCLIST slProclist,slExtProclist;

  for(igFuncCnt=0;proclist[igFuncCnt].keyword[0];igFuncCnt++)
  {
  }
  for(i=0;i<igFuncCnt-1;i++)
  {
    for(j=0;j<igFuncCnt-i-1;j++)
    {
      if(strcmp(proclist[j].keyword,proclist[j+1].keyword)>0)
      {
        slProclist=proclist[j];
        proclist[j]=proclist[j+1];
        proclist[j+1]=slProclist;
      }
    }
  }

  for(igExtfuncCnt=0;pextproclist[igExtfuncCnt].keyword[0];igExtfuncCnt++)
  {
  }
  for(i=0;i<igExtfuncCnt-1;i++)
  {
  for(j=0;j<igExtfuncCnt-i-1;j++)
    {
      if(strcmp(pextproclist[j].keyword,pextproclist[j+1].keyword)>0)
      {
        slExtProclist=pextproclist[j];
        pextproclist[j]=pextproclist[j+1];
        pextproclist[j+1]=slExtProclist;
      }
    }
  }
  return(0);
}

/**************************************************************
 ** 函 数 名：_swGetfuncPtr
 ** 功    能：获取用户函数指针数组下标
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：
 ** 返 回 值：用户函数指针数组下标(成功) / -1(失败)
***************************************************************/
int _swGetfuncPtr(char *aFunc)
{
  int ilLeft=0,ilRight,ilMiddle,ilRc;
  
  ilRight=igFuncCnt-1;
  while(ilLeft<=ilRight)
  {
    ilMiddle=(ilLeft+ilRight)/2;
    ilRc=strcmp(aFunc,proclist[ilMiddle].keyword);
    if(ilRc>0) ilLeft=ilMiddle+1;
    else if(ilRc<0) ilRight=ilMiddle-1;
    else return(ilMiddle);
  }
  return(-1);
}

/**************************************************************
 ** 函 数 名：_swGetextfuncPtr
 ** 功    能：获取扩展用户函数指针数组下标
 ** 作    者：
 ** 建立日期：2001/11/23
 ** 修改日期：2001/11/23
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：
 ** 返 回 值：扩展用户函数指针数组下标(成功) / -1(失败)
***************************************************************/
int _swGetextfuncPtr(char *aFunc)
{
  int ilLeft=0,ilRight,ilMiddle,ilRc;

  ilRight=igExtfuncCnt-1;
  while(ilLeft<=ilRight)
  {
    ilMiddle=(ilLeft+ilRight)/2;
    ilRc=strcmp(aFunc,pextproclist[ilMiddle].keyword);
    if(ilRc>0) ilLeft=ilMiddle+1;
    else if(ilRc<0) ilRight=ilMiddle-1;
    else return(ilMiddle);
  }
  return(-1);
}

/**************************************************************
 ** 函数名      ： _ufNtoT
 ** 功  能      ： 将未经转义的字符串进行转义
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000/2/16
 ** 最后修改日期： 2000/2/16
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aNstr:未经转义的字符串
 **                iLen :未经转义的字符串长度
 **                aTstr:已经转义的字符串
 ** 返回值      :  已经转义的字符串
***************************************************************/
int _ufNtoT(char *aNstr,short iLen,char *aTstr)
{
  short i,j = 0;

  swVdebug(3,"S8030: [函数调用] _ufNtoT(%d)",iLen);

  for(i=0;i<iLen;i++)
  {
    switch(aNstr[i])
    {
      case '\0':
	memcpy(aTstr + j,"~0",2);
	j = j + 2;
	break;
      case '(':
	memcpy(aTstr + j,"~1",2);
	j = j + 2;
	break;
      case ')':
	memcpy(aTstr + j,"~2",2);
	j = j + 2;
	break;
      case ',':
	memcpy(aTstr + j,"~3",2);
	j = j + 2;
	break;
      case '~':
	memcpy(aTstr + j,"~~",2);
	j = j + 2;
	break;
      /* next 4 rows add by nh 2002.7.3 */
      case '+':
	memcpy(aTstr + j,"~4",2);
	j = j + 2;
	break;
      default:
	aTstr[j] = aNstr[i];
	j ++;
    }
  }
  aTstr[j] = '\0';

  swVdebug(3,"S8033: [函数返回] _ufNtoT()返回码=0,结果=%s,",aTstr);

  return 0;
}

/**************************************************************
 ** 函数名      ： _ufTtoN
 ** 功  能      ： 将已经转义的字符串进行反转义
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000/2/16
 ** 最后修改日期： 2000/2/16
 ** 调用其它函数:
 ** 全局变量    ：
 ** 参数含义    :  alTstr:已经转义的字符串
 **                alNstr:未经转义的字符串
 **                pilLen :未经转义的字符串长度
 ** 返回值      :  未经转义的字符串
***************************************************************/
int _ufTtoN(char *aTstr, char *aNstr, short *piLen)
{
  short i=0,j=0;

  swVdebug(3,"S8036: [函数调用] _ufTtoN(%s)",aTstr);

  while(aTstr[i] != '\0')
  {
    if (aTstr[i] == '~')
    {
      switch(aTstr[i+1])
      {
	case '0':
	  aNstr[j] = '\0';
	  i = i + 2;
	  j ++;
	  continue;
	case '1':
	  aNstr[j] = '(';
	  i = i + 2;
	  j ++;
	  continue;
	case '2':
	  aNstr[j] = ')';
	  i = i + 2;
	  j ++;
	  continue;
	case '3':
	  aNstr[j] = ',';
	  i = i + 2;
	  j ++;
	  continue;
 	/* next 5 rows add by nh 2002.7.3 */
	case '4':
	  aNstr[j] = '+';
	  i = i + 2;
	  j ++;
	  continue;
	case '~':
	  aNstr[j] = '~';
	  i = i + 2;
	  j ++;
	  continue;
        default:
	  break;
      }
    }
    aNstr[j] = aTstr[i];
    i ++;
    j ++;
  }
  aNstr[j] = '\0';
  *piLen = j;

  swVdebug(3,"S8039: [函数返回] _ufTtoN()返回码=0,结果串长度=%d",j);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProFML
 ** 功  能      ： 根据标准域名取得标准域值
 ** 作  者      ： 史正烨
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ： psgPreunpackbuf : 预解包缓冲池
 ** 参数含义    ： aParm[0] : 标准域名
 ** 返回值      ： 标准域值
***************************************************************/
int _ufProFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRtncode;
  short ilLen=0;
  char alTmp[iFLDVALUELEN];

  swVdebug(3,"S8042: [函数调用] _ufProFML(%s)",aParm[0]);
     
  strcpy(alTmp,aParm[0]); 
  ilRtncode=swFmlget(psgPreunpackbuf,aParm[0],&ilLen,alTmp);
/*
  if (ilRtncode==FAIL)
  {
    swVdebug(0,"S8045: [错误/函数调用] swFmlget()函数,返回码=%d",ilRtncode);
    return(-1);
  }
*/
  if (ilRtncode==FAIL) aResult[0] = '\0';

  _ufNtoT(alTmp,ilLen,aResult);
  swVdebug(3,"S8048: [函数返回] _ufProFML()返回码=0,结果=%s",aResult);

  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProNSP
 ** 功  能      ： 取得定长格式报文的域值
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 起始位置
                   aParm[1] : 域长度
 ** 返回值      ： 域值
***************************************************************/
int _ufProNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen=0;
  short ilOffset;

  swVdebug(3,"S8051: [函数调用] _ufProNSP(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0],alTmp,&ilLen);
  ilOffset = atoi(alTmp) - 1;
  if (ilOffset<0) ilOffset=0;

  _ufTtoN(aParm[1],alTmp,&ilLen);
  ilLen = atoi(alTmp);
  /* add by zjj 2004.03.15 */
  if (ilLen >= iFLDVALUELEN)
  {
    swVdebug(3,"S8649: [错误/其他] _ufPropNSP,取值长度[%d]超过域值最大长度[%d]",ilLen,iFLDVALUELEN);
    return(FAIL);
  }
  /*end add by zjj */

/* delete by szhengye 2001.8.30
  if ((ilOffset+ilLen)>igMsgbodylen)
    ilLen=igMsgbodylen - ilOffset;
  if (ilLen<0) ilLen=0;
*/

  _ufNtoT(agMsgbody+ilOffset,ilLen,aResult);

  swVdebug(3,"S8054: [函数返回] _ufProNSP()返回码=0,结果=%s",aResult);
  return(0);

}

/**************************************************************
 ** 函数名      ： _ufProSEP
 ** 功  能      ： 根据记录ID、组ID、域ID取得域值
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 记录ID
                   aParm[1] : 组ID
                   aParm[2] : 域ID
 ** 返回值      ： 域值
***************************************************************/
int _ufProSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilRtncode,ilRecid,ilGrpid,ilFldid,ilLen;
  char alTmp[iFLDVALUELEN+1];

  swVdebug(3,"S8057: [函数调用] _ufProSEP(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],aResult,&ilRtncode);
  ilRecid = atoi(aResult);
  _ufTtoN(aParm[1],aResult,&ilRtncode);
  ilGrpid = atoi(aResult);
  _ufTtoN(aParm[2],aResult,&ilRtncode);
  ilFldid = atoi(aResult);

  ilRtncode = swSepget(psgPreunpackbuf,ilRecid,ilGrpid,ilFldid,&ilLen,alTmp);

  _ufNtoT(alTmp,ilLen,aResult);

  swVdebug(3,"S8060: [函数返回] _ufProSEP()返回码=%d,结果=%s",ilRtncode,aResult);
  return(ilRtncode);
}

/**************************************************************
 ** 函数名      ： _ProVSEP
 ** 功  能      ： 根据记录分隔符,偏移量取得域值
 ** 作  者      ： 张辉 
 ** 建立日期    ： 2000/6/29
 ** 最后修改日期： 2000/6/29
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： alParam[0] : 偏移量
                   alParam[1] : 记录分隔符
 ** 返回值      ： 域值
***************************************************************/
/*char* _ufProVSEP(char alParam[][iFLDVALUELEN])
{
  char  clFind;
  char  alTmpstr[iFLDVALUELEN];
  short ilLen=0;
  short ilRtncode,ilOffset;
  short i;
  unsigned char  clSep;
  unsigned char _ufStr2Char();

  swVdebug(3,"S8063: _ufProVSEP():start param0=[%s],param1=[%s]",alParam[0],alParam[1]);

  if (igMsgbodylen<=0) {
     swVdebug(0,"S8066: _ProVSEP: error igMsgbodylen=[%d]\n",igMsgbodylen);
     return(NULL);
  }
  memset(alTmpstr, 0x00, sizeof(alTmpstr));
  ilOffset=atoi(alParam[0])-1;

  clSep=_ufStr2Char(alParam[1]);
  swVdebug(1,"S8069: alparam[1]=[%s],sep=[%c]",alParam[1],clSep);

  ilLen=0;
  clFind=0;
  for(i=ilOffset;i<igMsglen;i++)
  {
     swVdebug(1,"S8072: now check i=%d,agMsgbody[i]=%c",i,agMsgbody[i]);
     if (agMsgbody[i]==clSep) {
        clFind=1;
        ilLen=i-ilOffset;
        break;
    }
  }
  swVdebug(3,"S8075: ilLen=%d",ilLen);
  if(clFind==1&&ilLen==0) return(NULL);
  if(ilLen==0) ilLen=igMsglen-ilOffset;
  if ((ilOffset+ilLen)>iMSGMAXLEN) return(NULL);

  swVdebug(3,"S8078: _ProVSEP=...");
  _ufNtoT(agMsgbody + ilOffset,ilLen,alTmpstr);

  swVdebug(3,"S8081: _ProVSEP: _NtoT return ={%s}\n",alTmpstr);

  return(alTmpstr);
}            */

unsigned char _ufStr2Char(a)
  char *a;
{
  short j;
  unsigned char b;
  
  swVdebug(3,"S8084: [函数调用] _ufStr2Char(%s)",a);
  j=0;
  if(a[0]>='a'&&a[0]<='f')j=a[0]-87;
  else if(a[0]>='A'&&a[0]<='F')j=a[0]-55;
  else j=a[0]-48;

  b=j;
  if(a[1]==0x00)  
    return(j);
 
  j*=16;
  if(a[1]>='a'&&a[1]<='f')b=j+a[1]-87;
  else if(a[1]>='A'&&a[1]<='F')b=j+a[1]-55;
  else b=j+a[1]-48;

  swVdebug(3,"S8087: [函数返回] _ufStr2Char()返回码=%d",b);
  return(b);
}
  
/**************************************************************
 ** 函数名      ： _ufProISO
 ** 功  能      ： 根据8583顺序号取得该域的值
 ** 作  者      ： 史正烨
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 8583顺序号
 ** 返回值      ： 8583域值
***************************************************************/
int _ufProISO(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilFldlen;
  short ilRtncode,ilFldid;
  char  alTmp[iFLDVALUELEN+1];

  swVdebug(3,"S8090: [函数调用] _ufProISO(%s)",aParm[0]);
      
  ilFldid = atoi(aParm[0]);
  ilRtncode = swIsoget(psgPreunpackbuf,ilFldid,&ilFldlen,alTmp);

  _ufNtoT(alTmp,ilFldlen,aResult);

  swVdebug(3,"S8093: [函数返回] _ufProISO()返回码=%d,结果=%s",ilRtncode,aResult);
/*  return(ilRtncode); */
  return(SUCCESS);  /* modify by zjj 2002.05.14 for qy suggestion */
}

/**************************************************************
 ** 函数名      ： _ufProMSGHEAD
 ** 功  能      ： 取当前报文头的值
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000/2/24
 ** 最后修改日期： 2000/2/24
 ** 调用其它函数：
 ** 全局变量    ： psgMsghead
 ** 参数含义    :  报文头的域名称
 ** 返回值      ： 
***************************************************************/
int _ufProMSGHEAD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(3,"S8096: [函数调用] _ufProMSGHEAD(%s)",aParm[0]);
     swVdebug(1,"SSSSSSSS 20150415"); 
swVdebug(3,"add by pc 20150415 pa0[%s] [%s] [%p]",aParm[0],psgMsghead->aBegin_trancode,psgMsghead);
  if (!strcmp(aParm[0],"lTranid"))
  {
     sprintf(aResult,"%ld",psgMsghead->lTranid);
  }
  else if(!strcmp(aParm[0],"lBegintime"))
  {
     sprintf(aResult,"%ld",psgMsghead->lBegintime);
  }
  else if(!strcmp(aParm[0],"iBegin_q_id"))
  {
     sprintf(aResult,"%d",psgMsghead->iBegin_q_id);
  }
  else if(!strcmp(aParm[0],"aBegin_trancode"))
  {
     sprintf(aResult,"%.10s",psgMsghead->aBegin_trancode);
  }
  else if(!strcmp(aParm[0],"aTrancode"))
  {
     sprintf(aResult,"%.10s",psgMsghead->aTrancode);
  }
 else if(!strcmp(aParm[0],"iTranstep"))
  {
     sprintf(aResult,"%1d",psgMsghead->iTranstep);
  }
  else if(!strcmp(aParm[0],"iMsgtype"))
  {
     sprintf(aResult,"%1d",psgMsghead->iMsgtype);
  }
  else if(!strcmp(aParm[0],"iMsgtypeorg"))
  {
     sprintf(aResult,"%1d",psgMsghead->iMsgtypeorg);
  }
  else if(!strcmp(aParm[0],"iOrg_q"))
  {
     sprintf(aResult,"%1d",psgMsghead->iOrg_q);
  }
  else if(!strcmp(aParm[0],"iDes_q"))
  {
     sprintf(aResult,"%1d",psgMsghead->iDes_q);
  }
  else if(!strcmp(aParm[0],"cFormatter"))
  {
     if (psgMsghead->cFormatter!=0x00)
  sprintf(aResult,"%c",psgMsghead->cFormatter);
     else
        strcpy(aResult," ");
  }
  else if(!strcmp(aParm[0],"cRouter"))
  {
     if (psgMsghead->cRouter!=0)
        sprintf(aResult,"%c",psgMsghead->cRouter);
     else
        strcpy(aResult," ");
  }
  else if(!strcmp(aParm[0],"cCodetype"))
  {
     if (psgMsghead->cCodetype!=0)
        sprintf(aResult,"%c",psgMsghead->cCodetype);
     else
        strcpy(aResult," ");
  }
  else if(!strcmp(aParm[0],"iMsgformat"))
  {
     sprintf(aResult,"%1d",psgMsghead->iMsgformat);
  }
  else if(!strcmp(aParm[0],"cEndflag"))
 {
     if (psgMsghead->cEndflag!=0)
        sprintf(aResult,"%c",psgMsghead->cEndflag);
     else
        strcpy(aResult," ");
  }
  else if(!strcmp(aParm[0],"iRouteline"))
  {
     sprintf(aResult,"%d",psgMsghead->iRouteline);
  }
  else if(!strcmp(aParm[0],"lCode"))
  {
     sprintf(aResult,"%ld",psgMsghead->lCode);
  }
  else if(!strcmp(aParm[0],"iBodylen"))
  {
     sprintf(aResult,"%d",psgMsghead->iBodylen);
  }
  else if(!strcmp(aParm[0],"cSafflag"))
  {
     if (psgMsghead->cSafflag)
        sprintf(aResult,"%c",psgMsghead->cSafflag);
     else
        strcpy(aResult," ");
  }
  else if(!strcmp(aParm[0],"lSafid"))
  {
     sprintf(aResult,"%ld",psgMsghead->lSafid);
  }
  else if(!strcmp(aParm[0],"iFmtgroup"))
  {  
     /*modify by zcd 20141228
     sprintf(aResult,"%d",psgMsghead->iFmtgroup);*/
     sprintf(aResult,"%ld",psgMsghead->iFmtgroup);
  }
  else if(!strcmp(aParm[0],"iRevmode"))
  {
     sprintf(aResult,"%d",psgMsghead->iRevmode);
  }
  else if(!strcmp(aParm[0],"iRevfmtgroup"))
  {
     /*modify by zcd 20141228
     sprintf(aResult,"%d",psgMsghead->iRevfmtgroup);*/
     sprintf(aResult,"%ld",psgMsghead->iRevfmtgroup);
  }
/* ========== add by dgm 2001/10/16 ============ */
  else if(!strcmp(aParm[0],"iSwitchgroup"))
  { 
     /*modify by zcd 20141228
     sprintf(aResult,"%d",psgMsghead->iSwitchgroup);*/
     sprintf(aResult,"%ld",psgMsghead->iSwitchgroup);
  }
  else if(!strcmp(aParm[0],"lSwitchtranid"))
  {
     sprintf(aResult,"%ld",psgMsghead->lSwitchtranid);
  }
/* ======= end of add by dgm 2001/10/16 ======== */
  else if(!strcmp(aParm[0],"aMemo"))
  {
     _ufNtoT(psgMsghead->aMemo,sizeof(psgMsghead->aMemo),aResult);
  }
 else
  {
     swVdebug(0,"S8099: [错误/其它] MSGHEAD(%s)不存在",aParm[0]);
  }

  swVdebug(3,"S8102: [函数返回] _ufProMSGHEAD()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProVAR
 ** 功  能      ： 取中间变量的值
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000/5/8
 ** 最后修改日期： 2000/5/8
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 中间变量名
 ** 返回值      ： 字符串函数
***************************************************************/
int  _ufProVAR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilFldlen;
  short ilRtncode;
  char alFldval[iFLDVALUELEN];
  
  swVdebug(3,"S8105: [函数调用] _ufProVAR(%s)",aParm[0]);
      
  ilRtncode = swFmlget(psgVarbuf,aParm[0],&ilFldlen,alFldval);
  _ufNtoT(alFldval,ilFldlen,aResult);

  swVdebug(3,"S8108: [函数返回] _ufProVAR()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProMVAR
 ** 功  能      ： 取中间变量的值(多记录)
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000.5.22
 ** 最后修改日期： 2000.5.22
 ** 调用其它函数：
 ** 全局变量    ： psgVarbuf : 中间变量缓冲池
 ** 参数含义    ： aParm[0] : 变量名
 **                aParm[1] : 记录号
 ** 返回值      ： 中间变量域值
***************************************************************/
int _ufProMVAR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  char alFldname[iFLDNAMELEN + 1];
  short ilLen=0;
  int   ilRtncode;
  short ilFldnamelen;

  swVdebug(3,"S8111: [函数调用] _ufProMVAR(%s,%s)",aParm[0],aParm[1]);
      
  ilFldnamelen = strlen(aParm[0]);
  strcpy(alFldname,aParm[0]);
  alFldname[ilFldnamelen] = '#';
  alFldname[ilFldnamelen + 1] = atoi(aParm[1]);
  alFldname[ilFldnamelen + 2] = '\0';

  ilRtncode=swFmlget(psgVarbuf, alFldname,&ilLen,alTmpstr);
  if(ilRtncode==FAIL)
  {
    swVdebug(0,"S8114: [错误/函数调用] swFmlget()函数,返回码=%d",ilRtncode);
    return(-1);
  }

  _ufNtoT(alTmpstr,ilLen,aResult);
  swVdebug(3,"S8117: [函数返回] _ufProMVAR()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProMFML
 ** 功  能      ： 根据标准域名取得标准域值(多记录)
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000.5.8
 ** 最后修改日期： 2000.5.8
 ** 调用其它函数：
 ** 全局变量    ： psgUnpackbuf : 预解包缓冲池
 ** 参数含义    ： aParm[0] : 标准域名
 **                aParm[1] : 记录号
 ** 返回值      ： 标准域值
***************************************************************/
int _ufProMFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRtncode;
  char alTmpstr[iFLDVALUELEN];
  char alFldname[iFLDNAMELEN + 1];
  short ilLen=0;
  short ilFldnamelen;

  swVdebug(3,"S8120: [函数调用] _ufProMFML(%s,%s)",aParm[0],aParm[1]);

  ilFldnamelen = strlen(aParm[0]);
  strcpy(alFldname,aParm[0]);
  alFldname[ilFldnamelen] = '#';
  alFldname[ilFldnamelen+1] = atoi(aParm[1]);
  alFldname[ilFldnamelen+2] = '\0';

  ilRtncode=swFmlget(psgUnpackbuf, alFldname,&ilLen,alTmpstr);
  if(ilRtncode==FAIL)
  {
    swVdebug(0,"S8123: [错误/函数调用] swFmlget()函数,返回码=%d",ilRtncode);
    return(-1);
  }

  _ufNtoT(alTmpstr,ilLen,aResult);
  swVdebug(3,"S8126: [函数返回] _ufProMFML()返回码=0,结果=%s",aResult);

  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProNFML
 ** 功  能      ： 根据标准域名取得标准域值(多记录)
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000.5.8
 ** 最后修改日期： 2000.5.8
 ** 调用其它函数：
 ** 全局变量    ： psgPreunpackbuf : 预解包缓冲池
 ** 参数含义    ： aParm[0] : 标准域名
 **                aParm[1] : 记录号
 ** 返回值      ： 标准域值
***************************************************************/
int _ufProNFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  char alFldname[iFLDNAMELEN + 1];
  short ilLen=0;
  int   ilRtncode;
  short ilFldnamelen;

  swVdebug(3,"S8129: [函数调用] _ufProNFML(%s,%s)",aParm[0],aParm[1]);

  ilFldnamelen = strlen(aParm[0]);
  strcpy(alFldname,aParm[0]);
  alFldname[ilFldnamelen] = '#';
  alFldname[ilFldnamelen + 1] = atoi(aParm[1]);
  alFldname[ilFldnamelen + 2] = '\0';

  ilRtncode=swFmlget(psgPreunpackbuf, alFldname,&ilLen,alTmpstr);
  if(ilRtncode==FAIL)
  {
    swVdebug(0,"S8132: [错误/函数调用] swFmlget()函数,返回码=%d",ilRtncode);
    return(-1);
  }

  _ufNtoT(alTmpstr,ilLen,aResult);
  swVdebug(3,"S8135: [函数返回] _ufProNFML()返回码=0,结果=%s",aResult);

  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProFLD
 ** 功  能      ： 取当前处理域的值
 ** 作  者      ： 史正烨
 ** 建立日期    ： 1999/12/7
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ： agCurfld
 ** 参数含义    :  (无)
 ** 返回值      ： agCurfld
***************************************************************/
int _ufProFLD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(3,"S8138: [函数调用] _ufProFLD()");

  _ufNtoT(agCurfld,strlen(agCurfld),aResult);
  swVdebug(3,"S8141: [函数返回] _ufProFLD()返回码=0,结果=%s",aResult);

  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProREC
 ** 功  能      ： 多记录TDF时取当前记录号
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2000.5.8
 ** 最后修改日期： 2000.5.8
 ** 调用其它函数：
 ** 全局变量    ： psgPreunpackbuf : 预解包缓冲池
 ** 参数含义    :
 ** 返回值      ： 记录号
***************************************************************/
int _ufProREC(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(3,"S8144: [函数调用] _ufProREC()");
      
  sprintf(aResult,"%d",igRec);

  swVdebug(3,"S8147: [函数返回] _ufProREC()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProMID
 ** 功  能      ： 取子串 
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/11/18
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 源数据串
                   aParm[1] : 起始位置
                   aParm[2] : 所取长度
 ** 返回值      ： 所求的子串
***************************************************************/
int _ufProMID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  short i,ilPos,ilStrlen,ilLen;

  swVdebug(3,"S8150: [函数调用] _ufProMID(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  ilPos=atoi(aParm[1]);
  ilLen=atoi(aParm[2]);
  _ufTtoN(aParm[0],aResult,&ilStrlen);

  if (ilStrlen<(ilPos+ilLen-1))
/* update by gsd  ilLen = ilStrlen - ilPos;
           如果从只定字符位取时,应包涵该字符取数据就要多取一位

        */
        ilLen = ilStrlen - (ilPos-1);
  for(i=0;i<ilLen;i++)
     alTmpstr[i]=aResult[ilPos+i-1];
  alTmpstr[i] = '\0';

  _ufNtoT(alTmpstr,i,aResult);

  swVdebug(3,"S8153: [函数返回] _ufProMID()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProLFT
 ** 功  能      ： 取左子串 
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 源数据串
                   aParm[1] : 所取长度
 ** 返回值      ： 所求的子串
***************************************************************/
int  _ufProLFT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  short ilLen,ilStrlen;

  swVdebug(3,"S8156: [函数调用] _ufProLFT(%s,%s)",aParm[0],aParm[1]);
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);

  ilLen=atoi(aParm[1]);
  if(ilStrlen<ilLen) ilLen=ilStrlen;

  _ufNtoT(alTmpstr,ilLen,aResult);

  swVdebug(3,"S8159: [函数返回] _ufProLFT()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProRIG
 ** 功  能      ： 取右子串 
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 源数据串
                   aParm[1] : 所取长度
 ** 返回值      ： 所求的子串
***************************************************************/
int _ufProRIG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  short ilLen,ilStrlen;

  swVdebug(3,"S8162: [函数调用] _ufProRIG(%s,%s)",aParm[0],aParm[1]);
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);

  ilLen=atoi(aParm[1]);
  if(ilStrlen<ilLen) ilLen=ilStrlen;

  _ufNtoT(alTmpstr+ilStrlen-ilLen,ilLen,aResult);
  swVdebug(3,"S8165: [函数返回] _ufProRIG()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProFILL
 ** 功  能      ： 填充字符串
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 数据串1
                   aParm[1] : 数据串2
 ** 返回值      ： 所求的串
***************************************************************/
int _ufProFILL(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char cFiller, cJustify;
  short iFilllen;
  char alFillbuf[iFLDVALUELEN];
  short ilBuflen;

  swVdebug(3,"S8168: [函数调用] _ufProFILL(%s,%s)",aParm[0],aParm[1]);

  if(iFilllen>sizeof(alFillbuf))
    iFilllen=sizeof(alFillbuf);
    
  _ufTtoN(aParm[1],aResult,&ilBuflen);
  cFiller=aResult[0];
  iFilllen=atoi(aParm[2]);
  cJustify=aParm[3][0];
  memset(alFillbuf, cFiller, iFilllen);

  _ufTtoN(aParm[0],aResult,&ilBuflen);
  if (iFilllen < ilBuflen)
    memcpy(alFillbuf,aResult,iFilllen);
  else
  {
    if (cJustify=='R')
      memcpy(alFillbuf, aResult, ilBuflen);
    else if(cJustify=='L')
      memcpy(alFillbuf+(iFilllen-ilBuflen), aResult, ilBuflen);
    else
        return(-1);
  }

  alFillbuf[iFilllen]='\0';
  _ufNtoT(alFillbuf,iFilllen,aResult);

  swVdebug(3,"S8171: [函数返回] _ufProFILL()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProSAD
 ** 功  能      ： 连接两个数据串
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 数据串1
                   aParm[1] : 数据串2
 ** 返回值      ： 所求的串
***************************************************************/
int _ufProSAD(char aParm[][iFLDVALUELEN],short iParmCount, char *aResult)
{
  int i,j;
  short ilStrlen;
  char alTmp[iFLDVALUELEN];

  swVdebug(3,"S8174: [函数调用] _ufProSAD(%s,%s)",aParm[0],aParm[1]);


  memset(alTmp,0x00,sizeof(alTmp));
  j=0;
  for(i=0;i<iParmCount;i++)
  {
    /* if(aParm[i][0]=='\0') break; */
    _ufTtoN(aParm[i],alTmp+j,&ilStrlen);
    j = j + ilStrlen;
  }
  alTmp[j]='\0';
 
  _ufNtoT(alTmp,j,aResult);


  swVdebug(3,"S8177: [函数返回] _ufProSAD()返回码=0,结果=%s",aResult);
  
  return(0);
}


/**************************************************************
 ** 函数名      ： _ufProLEN
 ** 功  能      ： 条件赋值
 ** 作  者      ： 史正烨 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
 ** 返回值      ： 字符串函数
***************************************************************/
int _ufProLEN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
  
  swVdebug(3,"S8180: [函数调用] _ufProLEN(%s)",aParm[0]);
      
  _ufTtoN(aParm[0],aResult,&ilStrlen);

  sprintf(aResult,"%d",ilStrlen);

  swVdebug(3,"S8183: [函数返回] _ufProLEN()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProTRIM
 ** 功  能      ： 去除前后空格
 ** 作  者      ： 史正烨 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
 ** 返回值      ： 字符串函数
***************************************************************/
int _ufProTRIM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
  char alTmp[iFLDVALUELEN];

  swVdebug(3,"S8186: [函数调用] _ufProTRIM(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrim(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  swVdebug(3,"S8189: [函数返回] _ufProTRIM()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProTRIML
 ** 功  能      ： 去除左边空格
 ** 作  者      ： lijunwen 
 ** 建立日期    ： 2000/3/29
 ** 最后修改日期： 2000/3/29
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
 ** 返回值      ： 字符串函数
***************************************************************/
int _ufProTRIML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
  char alTmp[iFLDVALUELEN];

  swVdebug(3,"S8192: [函数调用] _ufProTRIML(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrimL(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  swVdebug(3,"S8195: [函数返回] _ufProTRIML()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProTRIMR
 ** 功  能      ： 去除右边空格
 ** 作  者      ： lijunwen 
 ** 建立日期    ： 2000/3/29
 ** 最后修改日期： 2000/3/29
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
 ** 返回值      ： 字符串函数
***************************************************************/
int _ufProTRIMR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
  char alTmp[iFLDVALUELEN];

  swVdebug(3,"S8198: [函数调用] _ufProTRIMR(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrimR(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  swVdebug(3,"S8201: [函数返回] _ufProTRIMR()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      : _ufProSPACE
 ** 功  能      : 
 ** 作  者      : 
 ** 建立日期    ： 2000/3/30
 ** 最后修改日期： 2000/3/30
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm长度
 ** 返回值      :  空格串 
***************************************************************/
int _ufProSPACE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
 
  swVdebug(3,"S8204: [函数调用] _ufProSPACE(%s)",aParm[0]);

  ilStrlen=atoi((char *)aParm[0]);

  memset(aResult, ' ', ilStrlen);
  aResult[ilStrlen] = 0;

  swVdebug(3,"S8207: [函数返回] _ufProSPACE()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProREP
 ** 功  能      ：重复域赋值
 ** 作  者      ：张辉
 ** 建立日期    ：2000/5/18
 ** 最后修改日期：2000/5/18
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0]:字符
                   aParm[1]:个数
 ** 返回值      ： 字符型串
***************************************************************/
int _ufProREP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  char alStr[iFLDVALUELEN];
  short ilCount;
  short ilStrlen;
  short ilOffset;
  short i;

  swVdebug(3,"S8210: [函数调用] _ufProREP(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0],alStr,&ilStrlen);
  alStr[ilStrlen]='\0';
  ilCount=atoi(aParm[1]);

  ilOffset=0;
  for(i=0;i<ilCount;i++)
  {
    memcpy(alTmpstr+ilOffset,alStr,ilStrlen);
    ilOffset+=ilStrlen;
    if ((ilOffset+ilStrlen)>=sizeof(alTmpstr))
    {
      swVdebug(0,"S8213: [错误/其它] 目标串越界");
      break;
    }
  }

  _ufNtoT(alTmpstr,ilCount*ilStrlen,aResult);
  alTmpstr[ilCount*ilStrlen]='\0';

  swVdebug(3,"S8216: [函数返回] _ufProREP()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProADD
 ** 功  能      ：将两个数相加 
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/9
 ** 最后修改日期： 2000/5/9
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] : 数1
                   aParm[1] : 数2
 ** 返回值      ： 所求的串
***************************************************************/
/* ============ delete by dgm 2001/10/12 ========== 
int _ufProADD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int i,ilDotflag,ilDotnum;
  double dlTmp1,dlTmp2,dlTmp,dlDot;
 
  for(i=0;i<iParmCount;i++)
    swVdebug(3,"S8219: _ufProADD: aParm[%d]={%s}\n",i,aParm[i]);

  if(strchr(aParm[0],'.')==NULL && strchr(aParm[1],'.')==NULL)
    ilDotflag = 0;
  else
    ilDotflag = 1;
    
  if(iParmCount >= 3)
    ilDotnum = atoi(aParm[2]);
  else
  {
    if(ilDotflag == 0)
      ilDotnum = 0;
    else
      ilDotnum = 2;
  }    
  
  dlTmp1 = atof(aParm[0]);
  dlTmp2 = atof(aParm[1]);
  dlTmp = dlTmp1 + dlTmp2;
  
  dlDot = 5.0 / pow(10,ilDotnum + 1);
  dlTmp = dlTmp + dlDot;

  sprintf(aResult,"%.10lf",dlTmp);
  if(ilDotnum)
    *(strchr(aResult,'.') + ilDotnum + 1) = '\0';
  else
    *strchr(aResult,'.') = '\0';
    
  swVdebug(3,"S8222: _ufProADD() : aResult={%s}\n",aResult);

  return 0;
}
========= end of delete by dgm 2001/10/12 ======== */ 

/* ============ add by dgm 2001/10/12 ========== */
int _ufProADD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;

  swVdebug(3,"S8225: [函数调用] _ufProADD(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
   
  dlTmp1 = atof(aParm[0]);
  dlTmp2 = atof(aParm[1]);
  if (iParmCount >= 3)
    ilNum = atoi(aParm[2]);
  else
  {
    ilC1 = ilC2 = 0;
    if (strchr(aParm[0],'.')  != NULL)
      ilC1 = strchr(aParm[0], '\0') - strchr(aParm[0], '.') - 1;
    if (strchr(aParm[1],'.')  != NULL)
      ilC2 = strchr(aParm[1], '\0') - strchr(aParm[1], '.') - 1;
  
    if (ilC1 >= ilC2)
      ilNum = ilC1;
    else
      ilNum = ilC2; 
  }
    
  dlTmp = dlTmp1 + dlTmp2;

  if (dlTmp >= 0)
    dlTmp += 5.0 / pow(10,ilNum + 1);
  else
    dlTmp -= 5.0 / pow(10,ilNum + 1);

  sprintf(alFmt, "%%.%dlf", ilNum + 3);
  sprintf(aResult, alFmt, dlTmp);
    
  p1 = strchr(aResult, '.');
  if (p1 ==NULL)
  {
    swVdebug(3,"S8228: [函数返回] _ufProADD()返回码=0,结果=%s",aResult);
    return 0;
  }
  if (ilNum == 0) ilNum = -1;
  *(p1 + ilNum+1) = '\0';
   
  swVdebug(3,"S8231: [函数返回] _ufProADD()返回码=0,结果=%s",aResult);

  return 0;
}
/* ========= end of add by dgm 2001/10/12 ====== */

/**************************************************************
 ** 函数名      ： _ufProSUB
 ** 功  能      ：将两个数相减
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/13
 ** 最后修改日期：2000/5/13
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] : 数1
                   aParm[1] : 数2
 ** 返回值      ： 所求的串
***************************************************************/
/* ============ delete by dgm 2001/10/12 ========== 
int _ufProSUB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int i,ilDotflag,ilDotnum;
  double dlTmp1,dlTmp2,dlTmp,dlDot;
  
  for(i=0;i<iParmCount;i++)
    swVdebug(3,"S8234: _ufProSUB: aParm[%d]={%s}",i,aParm[i]);
   
  if(strchr(aParm[0],'.')==NULL && strchr(aParm[1],'.')==NULL)
    ilDotflag = 0;
  else
    ilDotflag = 1;
    
  if(iParmCount >= 3)
    ilDotnum = atoi(aParm[2]);
  else
  {
    if(ilDotflag == 0)
      ilDotnum = 0;
    else
      ilDotnum = 2;
  }    

  dlTmp1 = atof(aParm[0]);
  dlTmp2 = atof(aParm[1]);
  dlTmp  = dlTmp1 - dlTmp2;
  
  dlDot = 5.0 / pow(10,ilDotnum + 1);
  dlTmp = dlTmp + dlDot;
   
  sprintf(aResult,"%.10lf",dlTmp);
  if(ilDotnum)
    *(strchr(aResult,'.') + ilDotnum + 1) = '\0';
  else
    *strchr(aResult,'.') = '\0';

  swVdebug(3,"S8237: _ufProSUB():aResult=[%s]\n",aResult);

  return 0;
}
========= end of delete by dgm 2001/10/12 ======== */ 

/* ============ add by dgm 2001/10/12 ========== */
int _ufProSUB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;
  
  
  swVdebug(3,"S8240: [函数调用] _ufProSUB(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
   
  dlTmp1 = atof(aParm[0]);
  dlTmp2 = atof(aParm[1]);
  if (iParmCount >= 3)
    ilNum = atoi(aParm[2]);
  else
  {
    ilC1 = ilC2 = 0;
    if (strchr(aParm[0],'.')  != NULL)
      ilC1 = strchr(aParm[0], '\0') - strchr(aParm[0], '.') - 1;
    if (strchr(aParm[1],'.')  != NULL)
      ilC2 = strchr(aParm[1], '\0') - strchr(aParm[1], '.') - 1;
  
    if (ilC1 >= ilC2)
      ilNum = ilC1;
    else
      ilNum = ilC2; 
  }
    
  dlTmp = dlTmp1 - dlTmp2;

  if (dlTmp >= 0)
    dlTmp += 5.0 / pow(10,ilNum + 1);
  else
    dlTmp -= 5.0 / pow(10,ilNum + 1);

  sprintf(alFmt, "%%.%dlf", ilNum + 3);
  sprintf(aResult, alFmt, dlTmp);
    
  p1 = strchr(aResult, '.');
  if (p1 ==NULL)
  {
    swVdebug(3,"S8243: [函数返回] _ufProSUB()返回码=0,结果=%s",aResult);
    return 0;
  }
 if (ilNum == 0) ilNum = -1;
 *(p1 + ilNum+1) = '\0';
   
  swVdebug(3,"S8246: [函数返回] _ufProSUB()返回码=0,结果=%s",aResult);

  return 0;
}
/* ========= end of add by dgm 2001/10/12 ====== */

/**************************************************************
 ** 函数名      ： _ufProMULTI
 ** 功  能      ：将两个数相乘 
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/9
 ** 最后修改日期： 2000/5/9
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] : 数1
                   aParm[1] : 数2
 ** 返回值      ： 所求的串
***************************************************************/
/* ============ delete by dgm 2001/10/12 ========== 
int _ufProMULTI(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int i,ilDotflag,ilDotnum;
  double dlTmp1,dlTmp2,dlTmp,dlDot;
  
  for(i=0;i<iParmCount;i++)
    swVdebug(3,"S8249: _ufProMULTI: aParm[%d]={%s}",i,aParm[i]);
   
  if(strchr(aParm[0],'.')==NULL && strchr(aParm[1],'.')==NULL)
    ilDotflag = 0;
  else
    ilDotflag = 1;
    
  if(iParmCount >= 3)
    ilDotnum = atoi(aParm[2]);
  else
  {
    if(ilDotflag == 0)
      ilDotnum = 0;
    else
      ilDotnum = 2;
  }           

  dlTmp1=atof(aParm[0]);
  dlTmp2=atof(aParm[1]);
  dlTmp=dlTmp1*dlTmp2;
  
  dlDot = 5.0 / pow(10,ilDotnum + 1);
  dlTmp = dlTmp + dlDot;

  sprintf(aResult,"%.10lf",dlTmp);
  if(ilDotnum)
    *(strchr(aResult,'.') + ilDotnum + 1) = '\0';
  else
    *strchr(aResult,'.') = '\0';
  
  swVdebug(3,"S8252: _ufProMULTI():aResult=[%s]\n",aResult);

  return 0;
}
========= end of delete by dgm 2001/10/12 ======== */ 

/* ============ add by dgm 2001/10/12 ========== */
int _ufProMULTI(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;
  
  
  swVdebug(3,"S8255: [函数调用] _ufProMULTI(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  dlTmp1 = atof(aParm[0]);
  dlTmp2 = atof(aParm[1]);
  if (iParmCount >= 3)
    ilNum = atoi(aParm[2]);
  else
  {
    ilC1 = ilC2 = 0;
    if (strchr(aParm[0],'.')  != NULL)
      ilC1 = strchr(aParm[0], '\0') - strchr(aParm[0], '.') - 1;
    if (strchr(aParm[1],'.')  != NULL)
      ilC2 = strchr(aParm[1], '\0') - strchr(aParm[1], '.') - 1;
  
    ilNum = ilC1 + ilC2; 
  }
    
  dlTmp = dlTmp1 * dlTmp2;

  if (dlTmp >= 0)
    dlTmp += 5.0 / pow(10,ilNum + 1);
  else
    dlTmp -= 5.0 / pow(10,ilNum + 1);
      
  sprintf(alFmt, "%%.%dlf", ilNum + 3);
  sprintf(aResult, alFmt, dlTmp);
  
  p1 = strchr(aResult, '.');
  if (p1 ==NULL)
  {
    swVdebug(3,"S8258: [函数返回] _ufProMULTI()返回码=0,结果=%s",aResult);
    return 0;
  }
  if (ilNum == 0) ilNum = -1;
    *(p1 + ilNum+1) = '\0';

  swVdebug(3,"S8261: [函数返回] _ufProMULTI()返回码=0,结果=%s",aResult);
  return 0;
}
/* ========= end of add by dgm 2001/10/12 ====== */

/* ============ add by dgm 2001/09/25 ========== */
/**************************************************************
 ** 函数名      ： _ufProDIV
 ** 功  能      : 求两个数的商
 ** 作  者      : 
 ** 建立日期    ：2001/09/25
 ** 最后修改日期：2001/09/25
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] :被除数
 **             :  aParm[1] :除数
 ** 返回值      :  商的字符串表示
***************************************************************/
int _ufProDIV(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  ldiv_t slResult;
  long llNum1,llNum2;
  long llRt;

  swVdebug(3,"S8264: [函数调用] _ufProDIV(%s,%s)",aParm[0],aParm[1]);

  llRt=0;
  llNum1=atol(aParm[0]);
  llNum2=atol(aParm[1]);
  if (llNum2!=0)
  {
    slResult=ldiv(llNum1,llNum2);
    llRt=slResult.quot;
  }
  sprintf(aResult,"%ld",llRt);

  swVdebug(3,"S8267: [函数返回] _ufProDIV()返回码=0,结果=%s",aResult);
  return 0;
}

/* ========= end of add by dgm 2001/09/25 ====== */

/**************************************************************
 ** 函数名      ： _ufProMOD
 ** 功  能      : 求两个数的模数
 ** 作  者      : 杨金舟 
 ** 建立日期    ：2000/6/09
 ** 最后修改日期：2000/6/09
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] :被除数
 **             :  aParm[1] :除数
 ** 返回值      :  模数的字符串表示
***************************************************************/
int _ufProMOD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  ldiv_t slResult;
  long llNum1,llNum2;
  long llRt;

  swVdebug(3,"S8270: [函数调用] _ufProMOD(%s,%s)",aParm[0],aParm[1]);

  llRt=0;
  llNum1=atol(aParm[0]);
  llNum2=atol(aParm[1]);
  if (llNum2!=0)
  {
    slResult=ldiv(llNum1,llNum2);
    llRt=slResult.rem;
  }
  sprintf(aResult,"%ld",llRt);

  swVdebug(3,"S8273: [函数返回] _ufProMOD()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProSEQ
 ** 功  能      ： 比较两个数据串是否相等
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 数据串1
                   aParm[1] : 数据串2
 ** 返回值      ： 域值
***************************************************************/
int _ufProSEQ(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(3,"S8276: [函数调用] _ufProSEQ(%s,%s)",aParm[0],aParm[1]);

  if (!strcmp(aParm[0],aParm[1]))
     strcpy(aResult,"1");
  else
     strcpy(aResult,"0");

  swVdebug(3,"S8279: [函数返回] _ufProSEQ()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProGT
 ** 功  能      ： 判断前者是否大于后者
 ** 作  者      ： 刘雄英
 ** 建立日期    ： 2000/5/25
 ** 最后修改日期： 2000/5/25
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 
 ** 返回值      ： 大于则返回1,否则返回0
***************************************************************/
int _ufProGT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  swVdebug(3,"S8282: [函数调用] _ufProGT(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a>b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  swVdebug(3,"S8285: [函数返回] _ufProGT()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProGE
 ** 功  能      ： 判断前者是否大于等于后者
 ** 作  者      ： 刘雄英
 ** 建立日期    ： 2000/5/25
 ** 最后修改日期： 2000/5/25
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 
 ** 返回值      ： 大于等于则返回1,否则返回0
***************************************************************/
int _ufProGE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  swVdebug(3,"S8288: [函数调用] _ufProGE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a>=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  swVdebug(3,"S8291: [函数返回] _ufProGE()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProLT
 ** 功  能      ： 判断前者是否小于后者
 ** 作  者      ： 刘雄英
 ** 建立日期    ： 2000/5/25
 ** 最后修改日期： 2000/5/25
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 
 ** 返回值      ： 小于则返回1,否则返回0
***************************************************************/
int _ufProLT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  swVdebug(3,"S8294: [函数调用] _ufProLT(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a<b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  swVdebug(3,"S8297: [函数返回] _ufProLT()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProLE
 ** 功  能      ： 判断前者是否小于等于后者
 ** 作  者      ： 刘雄英
 ** 建立日期    ： 2000/5/25
 ** 最后修改日期： 2000/5/25
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 
 ** 返回值      ： 小于等于则返回1,否则返回0
***************************************************************/
int _ufProLE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  swVdebug(3,"S8300: [函数调用] _ufProLE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a<=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  swVdebug(3,"S8303: [函数返回] _ufProLE()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProNE
 ** 功  能      ： 判断前者是否不等于后者
 ** 作  者      ： 刘雄英
 ** 建立日期    ： 2000/5/25
 ** 最后修改日期： 2000/5/25
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 
 ** 返回值      ： 不等于则返回1,否则返回0
***************************************************************/
int _ufProNE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  swVdebug(3,"S8306: [函数调用] _ufProNE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a!=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  swVdebug(3,"S8309: [函数返回] _ufProNE()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProMAT
 ** 功  能      ： 正则匹配计算
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 正则表达式
                   aParm[1] : 参加比较的串
 ** 返回值      ： TRUE:"1",FALSE:"0"
***************************************************************/
int _ufProMAT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp1[iFLDVALUELEN];
  char alTmp2[iFLDVALUELEN];
  short ilRtncode,ilStrlen;
 
  swVdebug(3,"S8312: [函数调用] _ufProMAT(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0],alTmp1,&ilStrlen);
  _ufTtoN(aParm[1],alTmp2,&ilStrlen);
  ilRtncode=_swMatch(alTmp1,alTmp2);
  if(ilRtncode==TRUE)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  swVdebug(3,"S8315: [函数返回] _ufProMAT()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProNOT
 ** 功  能      ： 逻辑非
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 条件
 ** 返回值      ： 真：1; 假：0
***************************************************************/
int _ufProNOT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(3,"S8318: [函数调用] _ufProNOT(%s)",aParm[0]);

  if ( aParm[0][0]=='0')
     strcpy(aResult,"1");
  else
     strcpy(aResult,"0");

  swVdebug(3,"S8321: [函数返回] _ufProNOT()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProAND
 ** 功  能      ： 逻辑与
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 条件
 ** 返回值      ： 真：1; 假：0
***************************************************************/
int _ufProAND(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;
  short ilFlag=1;
  char alTmp[iFLDVALUELEN];
  short ilStrlen;

  swVdebug(3,"S8324: [函数调用] _ufProAND(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    swVdebug(3,"S8327:                     ,%s",aParm[i]);

  for(i=0;i<iParmCount;i++)
  {
    _ufTtoN(aParm[i],alTmp,&ilStrlen);
/*    if ((alTmp[0]=='0')||(alTmp[0]='\0'))  */
    if ((alTmp[0]=='0')||(alTmp[0]=='\0'))  /* modify by zjj 2002.05.14 for qy suggestion */
    {
      ilFlag=0;
      break;
    }
  }

/*  if (ilFlag==0) */
  if (ilFlag)      /* modify by zjj 2002.05.14 for qy suggestion */
     strcpy(aResult,"1");
  else
     strcpy(aResult,"0");

  swVdebug(3,"S8330: [函数返回] _ufProAND()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProOR
 ** 功  能      ： 逻辑或
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 条件
 ** 返回值      ： 真：1; 假：0
***************************************************************/
int _ufProOR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;
  short ilFlag=0;
  short ilStrlen;
  char  alTmp[iFLDVALUELEN];

  swVdebug(3,"S8333: [函数调用] _ufProOR(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    swVdebug(3,"S8336:                    ,%s",aParm[i]);
 
  for(i=0;i<iParmCount;i++)
  {
    _ufTtoN(aParm[i],alTmp,&ilStrlen);
    if ((alTmp[0]!='0')&&(alTmp[0]!='\0')) 
    {
      ilFlag=1;
      break;
    }
  }

  if (ilFlag==0)
     strcpy(aResult,"0");
  else 
     strcpy(aResult,"1");

  swVdebug(3,"S8339: [函数返回] _ufProOR()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProIF
 ** 功  能      ： 条件赋值
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/12/2
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 逻辑条件
                   aParm[1] : 条件为真时所应赋的值
                   aParm[2] : 条件为假时所应赋的值
 ** 返回值      ： 参数2或参数3
***************************************************************/
int _ufProIF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;

  swVdebug(3,"S8342: [函数调用] _ufProIF(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);

  if (((*aResult)!='0')&&((*aResult)!='\0')) 
    strcpy(aResult,aParm[1]);
  else 
    strcpy(aResult,aParm[2]);

  swVdebug(3,"S8345: [函数返回] _ufProIF()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProCASEIF
 ** 功  能      ： 条件CASE
 ** 作  者      ： 史正烨 
 ** 建立日期    ： 2000.5.24
 ** 最后修改日期： 2000.5.24
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 奇数位置为1或0,偶数位置为对应的值,最后位为default 
 ** 返回值      ： 返回1对应的值
***************************************************************/
int _ufProCASEIF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;

  swVdebug(3,"S8348: [函数调用] _ufProCASEIF(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    swVdebug(3,"S8351:                        ,%s",aParm[i]);

  for(i=0;i<iParmCount;i=i+2)
  {
    if (aParm[i+1][0] == '\0')
    {
       strcpy(aResult,aParm[i]);
       break;
    }
    if (aParm[i][0] == '1')
    {
       strcpy(aResult,aParm[i+1]);
       break;
    }
  }

  swVdebug(3,"S8354: [函数返回] _ufProCASEIF()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProCASEVAR
 ** 功  能      ： 值CASE
 ** 作  者      ： 史正烨 
 ** 建立日期    ： 2000.5.24
 ** 最后修改日期： 2000.5.24
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：第一位为变量,后跟变量值,及其对应的值
 ** 返回值      ：第一位为几,则返回其后对应的值 
***************************************************************/
int _ufProCASEVAR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;

  swVdebug(3,"S8357: [函数调用] _ufProCASEVAR(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    swVdebug(3,"S8360:                         ,%s",aParm[i]);

  for(i=1;i<iParmCount;i=i+2)
  {
    if (strcmp(aParm[0],aParm[i]) == 0)
    {
       strcpy(aResult,aParm[i+1]);
       break;
    }
/** add by bmy 2002.8.1 **/
    if (i==(iParmCount-1))
    {
       strcpy(aResult,aParm[i]);
       break;
    }
/** end of add **/
    
  }

  swVdebug(3,"S8363: [函数返回] _ufProCASEVAR()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ：_ufProCHR
 ** 功  能      ：取得16进制值
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/10
 ** 最后修改日期：2000/5/10
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] : 数1
 ** 返回值      ： 所求的串
***************************************************************/
int _ufProCHR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int iltmp;

  swVdebug(3,"S8366: [函数调用] _ufProCHR(%s)",aParm[0]);
  
  iltmp=atoi(aParm[0]);
  if(iltmp<0||iltmp>255) return(-1);
  sprintf(aResult,"%c",iltmp);

  swVdebug(3,"S8369: [函数返回] _ufProCHR()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ：_ufProSHORT
 ** 功  能      ：将字符转换为短整数
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/17
 ** 最后修改日期：2000/9/17
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] :  字符 一个短整形占两个字节
 ** 返回值      ： 所求的整数   如"12"转换为0x00,0x0c
***************************************************************/
int _ufProSHORT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen;
  short sltmp;

  swVdebug(3,"S8372: [函数调用] _ufProSHORT(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  sltmp=atoi(aResult);
  memcpy(alTmp,&sltmp,2);

  alTmp[2]='\0';
  _ufNtoT(alTmp,2,aResult);

  swVdebug(3,"S8375: [函数返回] _ufProSHORT()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ：_ufProINT
 ** 功  能      ：将字符转换为整数
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/17
 ** 最后修改日期：2000/5/17
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] :  字符 一个整形占四个字节
 ** 返回值      ： 所求的整数   如"12"转换为0x00,0x00,0x00,0x0c
***************************************************************/
int _ufProINT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen;
  int   iltmp;

  swVdebug(3,"S8378: [函数调用] _ufProINT(%s)",aParm[0]);
 
  _ufTtoN(aParm[0],aResult,&ilLen);
  iltmp=atoi(aResult);
  memcpy(alTmp,&iltmp,4);

  alTmp[4]='\0';
  _ufNtoT(alTmp,4,aResult);

  swVdebug(3,"S8381: [函数返回] _ufProINT()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ：_ufProLONG
 ** 功  能      ：将字符转换为长整形数
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/17
 ** 最后修改日期：2000/9/14
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] :  字符 一个长整形占四个字节
 ** 返回值      ： 所求的长整数  
***************************************************************/
int _ufProLONG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen;
  long lltmp;

  swVdebug(3,"S8384: [函数调用] _ufProLONG(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  lltmp=atol(aResult);
  memcpy(alTmp,&lltmp,4);

  alTmp[4]='\0';
  _ufNtoT(alTmp,4,aResult);

  swVdebug(3,"S8387: [函数返回] _ufProLONG()返回码=0,结果=%s",aResult);
  return 0;
}
  
/**************************************************************
 ** 函数名      ：_ufProDOUBLE
 ** 功  能      ：将字符转换为双精度浮点型
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/17
 ** 最后修改日期：2000/9/14
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] :数字字符无小数点 一个双精度浮点型占八个字节
 ** 返回值      ： 所求的双精度浮点型数  
***************************************************************/
int _ufProDOUBLE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  double dltmp;
  short  ilLen;

  swVdebug(3,"S8390: [函数调用] _ufProDOUBLE(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  dltmp=atof(aResult)/100;

  memcpy(alTmp,&dltmp,8);

  alTmp[8]='\0';
  _ufNtoT(alTmp,8,aResult);

  swVdebug(3,"S8393: [函数返回] _ufProDOUBLE()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ：_ufProDTOC
 ** 功  能      ：将双精度浮点型转换成字符型
 ** 作  者      : 
 ** 建立日期    ：2000/5/17
 ** 最后修改日期：2000/9/14
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] :数字字符无小数点 一个双精度浮点型占八个字节
 ** 返回值      ： 字符 
***************************************************************/
int _ufProDTOC(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  char alFormat[10];
  int  i;
  int ilDotflag;
  int ilDotcnt;
  double dlTmp;
  short  ilLen;

  swVdebug(3,"S8396: [函数调用] _ufProDTOC(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  ilDotcnt=atoi(aParm[1]);
  ilDotflag=atoi(aParm[2]);
  _ufTtoN(aParm[0],alTmp,&ilLen);
  alTmp[8]='\0';
  dlTmp=atof(alTmp);
  if (ilDotflag) 
  {
    sprintf(alFormat,"%%.%dlf",ilDotcnt);
    sprintf(aResult,alFormat,dlTmp);
  }
  else 
  {
    i=(int)pow(10,ilDotcnt);
    i=(int)(dlTmp*i);
    sprintf(alFormat,"%%0%dd",ilDotcnt+1);
    sprintf(aResult,alFormat,i);
  }
  
  swVdebug(3,"S8399: [函数返回] _ufProDTOC()返回码=0,结果=%s",aResult);
  return(0);
}

/*************************************************************
 ** 函数名      ：_ufProHEX
 ** 功  能      ：取得16进制值
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/10
 ** 最后修改日期：2000/5/10
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0] : 串
 ** 返回值      ： 所求的串
***************************************************************/
int _ufProHEX(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  
  int iltmp;
  char alResult[iFLDVALUELEN];

  swVdebug(3,"S8402: [函数调用] _ufProHEX(%s)",aParm[0]);

  iltmp=_ufStr2Char(aParm[0]);
  sprintf(alResult,"%c",iltmp);

  _ufNtoT(alResult,sizeof(char),aResult);

  swVdebug(3,"S8405: [函数返回] _ufProHEX()返回码=0,结果=%s",aResult);
  return 0;

}

  
/**************************************************************
 ** 函数名      ：_ufProUSHORT
 ** 功  能      ：将short型的数值转换成字符型
 ** 作  者      ：张辉
 ** 建立日期    ：2000/5/13
 ** 最后修改日期：2000/5/13
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0]:数值
 ** 返回值      ： 字符型串
***************************************************************/
int _ufProUSHORT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alOrig[iFLDVALUELEN];
  short ilLen;
  unsigned short *pilNum;

  swVdebug(3,"S8408: [函数调用] _ufProUSHORT(%s)",aParm[0]);
 
  _ufTtoN(aParm[0],alOrig,&ilLen);
  alOrig[ilLen]='\0';
  pilNum = (unsigned short *)(alOrig);
  sprintf(aResult,"%d",*pilNum);

  swVdebug(3,"S8411: [函数返回] _ufProUSHORT()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ：_ufProULONG
 ** 功  能      ：将long的数值转换成字符型
 ** 作  者      ：张辉
 ** 建立日期    ：2000/5/13
 ** 最后修改日期：2000/5/13
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0]:数值
 ** 返回值      ： 字符型串
***************************************************************/
int _ufProULONG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  unsigned long *pllValue;
  short ilLen;

  swVdebug(3,"S8414: [函数调用] _ufProULONG(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilLen);
  alTmp[ilLen] = '\0';
  pllValue=(unsigned long*)alTmp;
  sprintf(aResult,"%ld",*pllValue);

  swVdebug(3,"S8417: [函数返回] _ufProULONG()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      ： _ufProGETTIME
 ** 功  能      ： 取当前时间
 ** 作  者      ： 
 ** 建立日期    ： 2000/3/20
 ** 最后修改日期： 2000/3/20
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 格式
 ** 返回值      ： 字符串函数
***************************************************************/
int _ufProGETTIME(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  char alTmp[iFLDVALUELEN];
  long llCurrentTime;
  short ilStrlen;

  swVdebug(3,"S8420: [函数调用] _ufProGETTIME(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  time(&llCurrentTime);
  _swTimeLongToChs(llCurrentTime,alTmp,aResult);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  swVdebug(3,"S8423: [函数返回] _ufProGETTIME()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProTODATE
 ** 功  能      ：
 ** 作  者      ：
 ** 建立日期    ： 2000/3/30
 ** 最后修改日期： 2000/3/30
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[] : 源数据串
 ** 返回值      ： 所转换的日期子串
***************************************************************/
int _ufProTODATE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
  long llDatetime;
  struct tm *tmT;

  swVdebug(3,"S8426: [函数调用] _ufProTODATE(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  llDatetime = atol(aResult);
  tmT = localtime(&llDatetime);

  sprintf(aResult,"%04d%02d%02d",tmT->tm_year+1900,tmT->tm_mon+1,tmT->tm_mday);
  aResult[8] = '\0';
  
  swVdebug(3,"S8429: [函数返回] _ufProTODATE()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProTOTIME
 ** 功  能      : 
 ** 作  者      ：
 ** 建立日期    ： 2000/3/30
 ** 最后修改日期： 2000/3/30
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[] : 源数据串
 ** 返回值      ： 0-SUCCESS
***************************************************************/
int _ufProTOTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  long llDatetime;
  short ilStrlen;
  struct tm *tmT;
 
  swVdebug(3,"S8432: [函数调用] _ufProTOTIME(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  llDatetime = atol(aResult);
  tmT = localtime(&llDatetime);
  
  sprintf(aResult,"%02d:%02d:%02d", tmT->tm_hour, tmT->tm_min, tmT->tm_sec);
/* ============ delete by dgm 2001/10/15 ===========
  aResult[6] = '\0';
========= end of delete by dgm 2001/10/15 ======== */
  
  swVdebug(3,"S8435: [函数返回] _ufProTOTIME()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ：_ufProPID
 ** 功  能      ：得到本进程的进程号
 ** 作  者      ：刘雄英 
 ** 建立日期    ：2000/5/12
 ** 最后修改日期：2000/5/12
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  无
 ** 返回值      ： 0-SUCCESS
***************************************************************/
int _ufProPID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  long llPid;
  
  swVdebug(3,"S8438: [函数调用] _ufProPID()");

  llPid=getpid();
  sprintf(aResult,"%ld",llPid);

  swVdebug(3,"S8441: [函数返回] _ufProPID()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProGETMSG
 ** 功  能      : 
 ** 作  者      ： 
 ** 建立日期    ： 2000/3/20
 ** 最后修改日期： 2000/3/20
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： alParam[0] : 格式
 ** 返回值      ： 0-success
***************************************************************/
int _ufProGETMSG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
/*
  EXEC SQL BEGIN DECLARE SECTION;
    long llMsg_code;
    char alMsg_desc[41];
  EXEC SQL END DECLARE SECTION;

  llMsg_code = atol(aParm[0]);
  EXEC SQL SELECT msg_desc
             INTO :alMsg_desc
             FROM swt_sys_code 
            WHERE msg_code = :llMsg_code;
  if (sqlca.sqlcode) 
  _ufNtoT(alMsg_desc,strlen(alMsg_desc),aResult);

  swVdebug(3,"S8444: _ufProGETMSG: aResult = {%s}\n",aResult);
*/
  return(0);
}

/**************************************************************
 ** 函数名      ：_ufProGETSTR
 ** 功  能      ：返回码对应的解释串
 ** 作  者      ：张辉
 ** 建立日期    ：2000/5/18
 ** 最后修改日期：2000/5/18
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0]:返回码
                   aParm[1]:系统标识码 
                      T:telbank P:pubfund S:save A:acct
 ** 返回值      ： 字符型串
***************************************************************/
int _ufProGETSTR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
/*
  EXEC SQL BEGIN DECLARE SECTION;
    char alSys_id[2];
    char alReturn_code[11];
    char alExpress[41];
  EXEC SQL END DECLARE SECTION;

  sprintf(alSys_id,'\0');
  sprintf(alReturn_code,'\0');
  sprintf(alExpress,'\0');

  alSys_id[0]=aParm[0][0];
  memcpy(alReturn_code,aParm[1],strlen(aParm[1]));

  EXEC SQL SELECT express 
    INTO :alExpress 
    FROM return_code 
    WHERE sys_id = :alSys_id 
    AND return_code = :alReturn_code;
  strcpy(aResult,(char *)alExpress);
  if(sqlca.sqlcode==100)
  {
    strcpy(aResult,"返回码未定义");
  }
  else if(sqlca.sqlcode != 0)
  {
    swVdebug(0,"S8447: sqlca.sqlcode=%d",sqlca.sqlcode);
    strcpy(aResult,"数据库故障");
  }

  swVdebug(3,"S8450: GETSTR(%s,%s)={%s}",aParm[0],aParm[1],aResult);
*/
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProDATEtoLONG
 ** 功  能      ：
 ** 作  者      ：
 ** 建立日期    ： 2000/3/30
 ** 最后修改日期： 2000/3/30
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[] : 需转换的日期子串yyyymmdd
 ** 返回值      ： 0-SUCCESS  
***************************************************************/
int _ufProDATEtoLONG(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  short ilStrlen;
  unsigned long  llDatetime;
  struct tm tmT={0,0,0};
  char alYear[5];
  char alMon[3];
  char alDate[3];

  swVdebug(3,"S8453: [函数调用] _ufProDATEtoLONG(%s)",aParm[0]);
   
  _ufTtoN(aParm[0],aResult,&ilStrlen);
  memcpy(alYear,aResult,4);
  alYear[4] = '\0';
  memcpy(alMon,aResult+4,2);  
  alMon[2] = '\0';
  memcpy(alDate,aResult+6,2);  
  alDate[2] = '\0';
  tmT.tm_year=atoi(alYear)-1900;
  tmT.tm_mon=atoi(alMon)-1;
  tmT.tm_mday=atoi(alDate);
  llDatetime=mktime(&tmT);
  swVdebug(4,"S8456: llDate=[%u]",llDatetime);
  _ufNtoT((char*)&llDatetime,sizeof(long),aResult);
  
  swVdebug(3,"S8459: [函数返回] _ufProDATEtoLONG()返回码=0,结果=%s",aResult);
  return(0);
}


/************************************************************
 ** 函数名      ：_ufProMEMOSET
 ** 功  能      ：给报文头中的memo子段赋值
 ** 作  者      ：
 ** 建立日期    ：2000/12/18
 ** 最后修改日期：2000/12/18
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    :  aParm[0]: 位置
                   aParm[1]: 值
 ** 返回值      ： 字符型串
***************************************************************/
int _ufProMEMOSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilOffset,ilStrlen;

  swVdebug(3,"S8462: [函数调用] _ufProMEMOSET(%s,%s)",aParm[0],aParm[1]);

  ilOffset=atoi(aParm[0]);
  if (ilOffset>20)
  {
    swVdebug(0,"S8465: [错误/其它] 便移量=%d超过20",ilOffset);
    return(-1);
  }

  _ufTtoN(aParm[1],aResult,&ilStrlen);
  if (ilOffset+ilStrlen>20)
  {
    swVdebug(0,"S8468: [错误/其它] 便移量+值的长度=%d超过20",ilOffset+ilStrlen);
    ilStrlen=20-ilOffset;
  }
  memcpy(psgMsghead->aMemo+ilOffset-1,aResult,ilStrlen);

  swVdebug(3,"S8471: [函数返回] _ufProMEMOSET()返回码=0");
  return(0);
}

/**************************************************************
 ** 函数名	: _ProBPCOUNT
 ** 功  能	: 取得交易发起方端口正在处理的交易笔数
 ** 作  者	: byz
 ** 建立日期	: 2001.3.16
 ** 最后修改日期: 20010.3.16
 ** 调用其它函数:
 ** 全局变量	:
 ** 参数含义	: iQid前置邮箱号
 ** 返回值	: TRUE:"1",FALSE:"0" 
***************************************************************/
/*
char *_ufProBPCOUNT(char alParam[][iFLDVALUELEN])
{
  int   i;
  int   Qid;
  char  sMaxNum[10];

  Qid = atoi(alParam[0]);
  memset(sMaxNum,0x00,sizeof(sMaxNum));
  for(i=0;i<iMAXPORTCOUNT;i++)
  {
    if (Qid==psgPortattrib[i].iQ_id )
    {
      sprintf(sMaxNum,"%d",psgPortattrib[i].lTrning);
      return(sMaxNum);
    }
  }
  return("0");
}
*/
/**************************************************************
 ** 函数名      ： _ufProLONGTODATE
 ** 功  能      ： change long to date  *aResult 所转换的日期子串
 ** 作  者      ： lijunwen
 ** 建立日期    ： 2000/3/30
 ** 最后修改日期： 2000/3/30
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[] : 源数据串
 ** 返回值      ： 0- success
***************************************************************/
int _ufProLONGTODATE(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  long llDatetime;
  struct tm *tmT;
 
  swVdebug(3,"S8474: [函数调用] _ufProLONGTODATE(%s)",aParm[0]);

  llDatetime = atol(aParm[0]);
  
  tmT = localtime( &llDatetime );
  sprintf(aResult,"%04d-%02d-%02d",tmT->tm_year+1900,tmT->tm_mon+1,tmT->tm_mday);

  swVdebug(3,"S8477: [函数返回] _ufProLONGTODATE()返回码=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProLONGTOTIME
 ** 功  能      ： change long to time 
 ** 作  者      ： lijunwen
 ** 建立日期    ： 2000/3/30
 ** 最后修改日期： 2000/3/30
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[] : 源数据串
 ** 返回值      ： 0--success
***************************************************************/
int _ufProLONGTOTIME(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  long llDatetime;
  struct tm *tmT;
 
  swVdebug(3,"S8480: [函数调用] _ufProLONGTOTIME(%s)",aParm[0]);

  llDatetime = atol(aParm[0]);
  tmT = localtime( &llDatetime );
  
  sprintf(aResult,"%02d:%02d:%02d", tmT->tm_hour, tmT->tm_min, tmT->tm_sec);
  
  swVdebug(3,"S8483: [函数返回] _ufProLONGTOTIME()返回码=0,结果=%s",aResult);
  return(0);
}

/* 增加对通用前置支持 */
/**************************************************************
 ** 函数名      : _ufProFmlget 取FML预解包中值函数
 ** 功  能      : 得到FML预包中指定域名的长度和域值
 ** 作  者      : 
 ** 建立日期    : 1999.11.23
 ** 最后修改日期: 1999.11.23
 ** 调用其它函数:
 ** 全局变量    : psgPreunpackbuf	FML预解池
 ** 参数含义    : aParm[0]	域名
 ** 返回值      :
***************************************************************/
int _ufProFmlget(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilRc;
  short ilFldlen;
  int ilMsglen; /* 20150109 zhanghui */
  char  alFldname[iFLDNAMELEN+1];
  char  alTmp[iFLDVALUELEN+1];
  
  swVdebug(3,"S8486: [函数调用] _ufProFmlget(%s)",aParm[0]);

  strcpy(alFldname, aParm[0]);
  ilMsglen = sgMsgpack.sMsghead.iBodylen;
  ilRc = swFmlpackget(sgMsgpack.aMsgbody, ilMsglen, alFldname, &ilFldlen, alTmp);
  if (ilRc) 
  {
    swVdebug(0,"S8489: [错误/函数调用] swFmlpackget()函数,返回码=%d",ilRc);
    return(FAIL);
  }
  _ufNtoT(alTmp,ilFldlen,aResult);

  swVdebug(3,"S8492: [函数返回] _ufProFmlget()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : _ufProFmlset 设置FML预解包中值函数
 ** 功  能      : 设置FML预包中指定域名的长度和域值
 ** 作  者      : 
 ** 建立日期    : 1999.11.23
 ** 最后修改日期: 1999.11.23
 ** 调用其它函数:
 ** 全局变量    : psgPreunpackbuf	FML预解池
 ** 参数含义    : aParm[0]	域名
 **               aParm[1]	域值
 ** 返回值      :
***************************************************************/
int _ufProFmlset(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilRc;
  int ilMsglen; /* 20150109 zhanghui */
  short ilFldlen;
  char  alFldname[iFLDNAMELEN+1];
  char  alFldval[iFLDVALUELEN+1];

  swVdebug(3,"S8495: [函数调用] _ufProFmlset(%s,%s)",aParm[0],aParm[1]);

  strcpy(alFldname, aParm[0]);
  strcpy(alFldval, aParm[1]);
  ilFldlen = strlen(alFldval);
  ilMsglen = sgMsgpack.sMsghead.iBodylen;
  ilRc = swFmlpackset(sgMsgpack.aMsgbody, &ilMsglen, alFldname, ilFldlen, alFldval);
  if (ilRc) 
  {
    swVdebug(0,"S8498: [错误/函数调用] swFmlpackset()函数,返回码=%d",ilRc);
    return(FAIL);
  }
  sgMsgpack.sMsghead.iBodylen = ilMsglen;

  swVdebug(3,"S8501: [函数返回] _ufProFmlset()返回码=0");
  return(SUCCESS);
}
/**************************************************************
 ** 函数名      : _ufProTmpget 
 ** 功  能      : 取临时变量池中值函数
 ** 作  者      : 
 ** 建立日期    : 1999.11.23
 ** 最后修改日期: 1999.11.23
 ** 调用其它函数:
 ** 全局变量    : psgVarbuf	临时变量存放池
 ** 参数含义    : aParm[0]	临时变量名
 ** 返回值      :
***************************************************************/
int  _ufProTmpget(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short  ilLen = 0, ilRc;
  char alFldname[iFLDNAMELEN+1];
  char alTmp[iFLDVALUELEN];
  
  swVdebug(3,"S8504: [函数调用] _ufProTmpget(%s)",aParm[0]);

  strcpy(alFldname, aParm[0]);
  
  ilRc = swFmlget(psgVarbuf,alFldname,&ilLen,alTmp);
  if (ilRc)
  {
    swVdebug(0,"S8507: [错误/函数调用] swFmlget()函数,返回码=%d",ilRc);
    return(FAIL);
  }
  
  _ufNtoT(alTmp,ilLen,aResult);

  swVdebug(3,"S8510: [函数返回] _ufProTmpget()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/**************************************************************
 ** 函数名      : _ufProTmpset 
 ** 功  能      : 置临时变量池中值函数
 ** 作  者      : 
 ** 建立日期    : 1999.11.23
 ** 最后修改日期: 1999.11.23
 ** 调用其它函数:
 ** 全局变量    : psgVarbuf	临时变量存放池
 ** 参数含义    : aParm[0]	域名
 **               aParm[1]	域值
 ** 返回值      :
***************************************************************/
int _ufProTmpset(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilRc;
  char  alFldname[iFLDNAMELEN+1];
  short ilFldlen;
  char  alFldval[iFLDVALUELEN+1];

  swVdebug(3,"S8513: [函数调用] _ufProTmpset(%s,%s)",aParm[0],aParm[1]);

  strcpy(alFldname, aParm[0]);
  strcpy(alFldval, aParm[1]);
  ilFldlen = strlen(alFldval);

  ilRc = swFmlset(alFldname, ilFldlen, alFldval, psgVarbuf);
  if (ilRc) 
  {
    swVdebug(0,"S8516: [错误/函数调用] swFmlset()函数,返回码=%d",ilRc);
    return(FAIL);
  }

  swVdebug(3,"S8519: [函数返回] _ufProTmpset()返回码=0");
  return(SUCCESS);
}
/***********************************************************************
 ** 函数名      :  _ufProGetmsghead
 ** 功能        :  取报文头信息
 ** 作者:
 ** 建立日期    :  2000.01.26
 ** 最后修改日期:  2000.01.26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :  aParm[0]	报文头关键字
 ** 返回值      :  成功返回 0(SUCCESS), 失败返回 (FAIL)
************************************************************************/
int _ufProGetmsghead(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char  alTmp[20];  
  
  swVdebug(3,"S8522: [函数调用] _ufProGetmsghead(%s)",aParm[0]);

  strcpy(alTmp, aParm[0]);

  _swTrim(alTmp);
  
  if (strcmp(alTmp, "TRANID") == 0)
  {
    sprintf(aResult, "%ld", sgMsgpack.sMsghead.lTranid);
    swVdebug(3,"S8525: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "BEGINTIME") == 0)
  {
    sprintf(aResult, "%ld", sgMsgpack.sMsghead.lBegintime);
    swVdebug(3,"S8528: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "BGEINGQID") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iBegin_q_id);
    swVdebug(3,"S8531: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "BEGINTRANCODE") == 0)
  {
    sprintf(aResult, "%s", sgMsgpack.sMsghead.aBegin_trancode);
    swVdebug(3,"S8534: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "TRANCODE") == 0)
  {
    sprintf(aResult, "%s", sgMsgpack.sMsghead.aTrancode);
    swVdebug(3,"S8537: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "TRANSTEP") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iTranstep);
    swVdebug(3,"S8540: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MSGTYPE") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iMsgtype);
    swVdebug(3,"S8543: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MSGTYPEORG") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iMsgtypeorg);
    swVdebug(3,"S8546: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "ORGQ") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iOrg_q);
    swVdebug(3,"S8549: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "DESQ") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iDes_q);
    swVdebug(3,"S8552: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "FORMATTER") == 0)
  {
    sprintf(aResult, "%c", sgMsgpack.sMsghead.cFormatter);
    swVdebug(3,"S8555: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "ROUTER") == 0)
  {
    sprintf(aResult, "%c", sgMsgpack.sMsghead.cRouter);
    swVdebug(3,"S8558: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "CODETYPE") == 0)
  {
    sprintf(aResult, "%c", sgMsgpack.sMsghead.cCodetype);
    swVdebug(3,"S8561: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MSGFORMAT") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iMsgformat);
    swVdebug(3,"S8564: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "ENDFLAG") == 0)
  {
    sprintf(aResult, "%c", sgMsgpack.sMsghead.cEndflag);
    swVdebug(3,"S8567: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "ROUTELINE") == 0)
  {
/* ============ delete by dgm 2001/10/16 ========== 
    sprintf(aResult, "%c", sgMsgpack.sMsghead.iRouteline);
========= end of delete by dgm 2001/10/16 ======= */
/* ========== add by dgm 2001/10/16 ============ */
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iRouteline);
/* ========= end of add by dgm 2001/10/16 ====== */
    swVdebug(3,"S8570: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "CODE") == 0)
  {
    sprintf(aResult, "%ld", sgMsgpack.sMsghead.lCode);
    swVdebug(3,"S8573: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "BODYLEN") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iBodylen);
    swVdebug(3,"S8576: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "SAFFLAG") == 0)
  {
    sprintf(aResult, "%c", sgMsgpack.sMsghead.cSafflag);
    swVdebug(3,"S8579: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "SAFID") == 0)
  {
    sprintf(aResult, "%ld", sgMsgpack.sMsghead.lSafid);
    swVdebug(3,"S8582: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "FMTGROUP") == 0)
  {
   /*modify by zcd 20141223
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iFmtgroup);*/
    sprintf(aResult, "%ld", sgMsgpack.sMsghead.iFmtgroup);
    swVdebug(3,"S8585: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "REVMODE") == 0)
  {
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iRevmode);
    swVdebug(3,"S8588: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "REVFMTGROUP") == 0)
  {
    /*modify by zcd 20141223
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iRevfmtgroup);*/
    sprintf(aResult, "%ld", sgMsgpack.sMsghead.iRevfmtgroup);
    swVdebug(3,"S8591: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "SWITCHGROUP") == 0)
  {
    /*modify by zcd 20141223
    sprintf(aResult, "%d", sgMsgpack.sMsghead.iSwitchgroup);*/
    sprintf(aResult, "%ld",sgMsgpack.sMsghead.iSwitchgroup);
    swVdebug(3,"S8594: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "SWITCHTRANID") == 0)
  {
    sprintf(aResult, "%ld", sgMsgpack.sMsghead.lSwitchtranid);
    swVdebug(3,"S8597: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MEMO") == 0)
  {
    sprintf(aResult, "%s", sgMsgpack.sMsghead.aMemo);
    swVdebug(3,"S8600: [函数返回] _ufProGetmsghead()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }  

  swVdebug(0,"S8603: [错误/其它] 报文头中没有该域[%s]",alTmp);
  return(FAIL);  
}  

/***********************************************************************
 ** 函数名      :  _ufProSetmsghead
 ** 功能        :  置报文头信息
 ** 作者:
 ** 建立日期    :  2000.01.26
 ** 最后修改日期:  2000.01.26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :  aParm[0]	报文头关键字
                   aParm[1]	值
 ** 返回值      :  成功返回 0(SUCCESS), 失败返回 (FAIL)
************************************************************************/
int _ufProSetmsghead(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char  alTmp[20], alValue[40];  
  
  swVdebug(3,"S8606: [函数调用] _ufProSetmsghead(%s,%s)",aParm[0],aParm[1]);

  strcpy(alTmp, aParm[0]);
  strcpy(alValue, aParm[1]);

  _swTrim(alTmp);
  _swTrim(alValue);
  
  if (strcmp(alTmp, "TRANID") == 0)
  {
    sgMsgpack.sMsghead.lTranid = atol(alValue);
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "BEGINTIME") == 0)
  {
    sgMsgpack.sMsghead.lBegintime = atol(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "BGEINGQID") == 0)
  {
    sgMsgpack.sMsghead.iBegin_q_id = atoi(alValue);
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "BEGINTRANCODE") == 0)
  {
    strcpy(sgMsgpack.sMsghead.aBegin_trancode, alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "TRANCODE") == 0)
  {
    strcpy(sgMsgpack.sMsghead.aTrancode, alValue);
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "TRANSTEP") == 0)
  {
    sgMsgpack.sMsghead.iTranstep = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MSGTYPE") == 0)
  {
    sgMsgpack.sMsghead.iMsgtype = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MSGTYPEORG") == 0)
  {
    sgMsgpack.sMsghead.iMsgtypeorg = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "ORGQ") == 0)
  {
    sgMsgpack.sMsghead.iOrg_q = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "DESQ") == 0)
  {
    sgMsgpack.sMsghead.iDes_q = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "FORMATTER") == 0)
  {
    sgMsgpack.sMsghead.cFormatter = alValue[0];
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "ROUTER") == 0)
  {
    sgMsgpack.sMsghead.cRouter = alValue[0];
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "CODETYPE") == 0)
  {
    sgMsgpack.sMsghead.cCodetype = alValue[0];
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MSGFORMAT") == 0)
  {
    sgMsgpack.sMsghead.iMsgformat = atoi(alValue);
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "ENDFLAG") == 0)
  {
    sgMsgpack.sMsghead.cEndflag = alValue[0];
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "ROUTELINE") == 0)
  {
    sgMsgpack.sMsghead.iRouteline = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "CODE") == 0)
  {
    sgMsgpack.sMsghead.lCode = atol(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "BODYLEN") == 0)
  {
    sgMsgpack.sMsghead.iBodylen = atoi(alValue);
    return(SUCCESS);
  }  

  if (strcmp(alTmp, "SAFFLAG") == 0)
  {
    sgMsgpack.sMsghead.cSafflag = alValue[0];
    return(SUCCESS);
  }
  
  if (strcmp(alTmp, "SAFID") == 0)
  {
    sgMsgpack.sMsghead.lSafid = atol(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "FMTGROUP") == 0)
  {
    sgMsgpack.sMsghead.iFmtgroup = atol(alValue); /*modified by baiqj20150428 PSBC_V1.0 atoi -> atol*/
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "REVMODE") == 0)
  {
    sgMsgpack.sMsghead.iRevmode = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "REVFMTGROUP") == 0)
  {
    sgMsgpack.sMsghead.iRevfmtgroup = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "SWITCHGROUP") == 0)
  {
    sgMsgpack.sMsghead.iSwitchgroup = atoi(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "SWITCHTRANID") == 0)
  {
    sgMsgpack.sMsghead.lSwitchtranid = atol(alValue);
    return(SUCCESS);
  }  
  
  if (strcmp(alTmp, "MEMO") == 0)
  {
    strcpy(sgMsgpack.sMsghead.aMemo, alValue);
    return(SUCCESS);
  }  

  swVdebug(0,"S8609: [错误/其它] 报文头中没有该域[%s]",alTmp);
  return(FAIL);  
}


/* ========= add by dgm 2001/09/07 ====== */


/**************************************************************
 ** 函数名      ： _ufProPackSEP
 ** 功  能      ： 根据记录ID、组ID、域ID取得域值
 ** 作  者      ：
 ** 建立日期    ： 2001.9.7
 ** 最后修改日期： 
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 记录分隔符
                   aParm[1] : 组分隔符
                   aParm[2] : 域分隔符
                   aParm[3] : 记录ID
                   aParm[4] : 组ID
                   aParm[5] : 域ID
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufProPackSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alRecs[6], alGrps[6], alFlds[6];
  short ilRc, ilRtncode, ilRecid, ilGrpid, ilFldid, ilMsglen;
/* ========== add by dgm 2001/10/16 ============ */
  short ilLen;
  char  alTmp[iFLDVALUELEN+1];
/* ======= end of add by dgm 2001/10/16 ======== */

  swVdebug(3,"S8612: [函数调用] _ufProPackSEP(%s,%s,%s,%s,%s,%s)",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

  _ufTtoN(aParm[0], alRecs, &ilRtncode);
  _ufTtoN(aParm[1], alGrps, &ilRtncode);
  _ufTtoN(aParm[2], alFlds, &ilRtncode);
  
  _ufTtoN(aParm[3], aResult, &ilRtncode);
  ilRecid = atoi(aResult);
  _ufTtoN(aParm[4], aResult, &ilRtncode);
  ilGrpid = atoi(aResult);
  _ufTtoN(aParm[5], aResult, &ilRtncode);
  ilFldid = atoi(aResult);
  
  ilMsglen = sgMsgpack.sMsghead.iBodylen;

/* ============ delete by dgm 2001/10/16 =========
  ilRc = swPacksep(sgMsgpack.aMsgbody, ilMsglen, alRecs, alGrps, alFlds, \
                   ilRecid, ilGrpid, ilFldid, aResult);
  if (ilRc != 0)
  {
    swVdebug(0,"S8615: 调用swPacksep错误");
    swVdebug(0,"S8618: _ufProPackSEP: alRecs = %s, alGrps = %s, alFlds = %s\n", \
            alRecs, alGrps, alFlds);
    swVdebug(0,"S8621: _ufProPackSEP: SEP(%d,%d,%d)={%s}\n",ilRecid,ilGrpid,ilFldid,aResult);
    return(FAIL);
  }
========= end of delete by dgm 2001/10/16 ======= */
  
/* ========== add by dgm 2001/10/16 ============ */
  ilRc = swPacksep(sgMsgpack.aMsgbody, ilMsglen, alRecs, alGrps, alFlds, \
                   ilRecid, ilGrpid, ilFldid, alTmp);
  if (ilRc != 0)
  {
    swVdebug(0,"S8624: [错误/函数调用] swPacksep()函数,返回码=%d",ilRc);
    return(FAIL);
  }

  ilLen = strlen(alTmp);
  _ufNtoT(alTmp,ilLen,aResult);
/* ======= end of add by dgm 2001/10/16 ======== */

  swVdebug(3,"S8627: [函数返回] _ufProPackSEP()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}

/* ========= end of add by dgm 2001/09/07 ====== */

/**************************************************************
 ** 函数名      ： _ufPropGETSEP
 ** 功  能      ：
 ** 作  者      :
 ** 建立日期    ： 2001.9.7
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： agMsgbody : 报文
                   lgMsgoffset : 当前指针
 ** 参数含义    ： aParm[0] : 域分隔符 
                   aParm[1] : 分隔个数
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufPropGETSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short i,j,ilLen,ilRc;
  char  *alPos,*alPos2,*alTmp;
  char  alRecs[6];

  swVdebug(3,"S8630: [函数调用] _ufPropGETSEP(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0], alRecs, &ilRc);
  j = atoi(aParm[1]);
  alPos = agMsgbody;
  alPos = alPos + lgMsgoffset;

  ilLen = strlen(aParm[0]);
  for (i=0;i<j;i++)
  {
    alTmp = strstr(alPos, alRecs);
    if (alTmp == NULL) return(FAIL);
    alPos2 = alPos;
    alPos = alTmp + ilLen;
  } 
  memcpy(aResult,alPos2,alPos - alPos2 - ilLen);
  aResult[alPos - alPos2 - ilLen] = '\0';
  lgMsgoffset = lgMsgoffset + (alPos - alPos2);
  
  swVdebug(3,"S8633: [函数返回] _ufPropGETSEP()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/*********************************************************************
 ** 函数名      ： _ufPropSET
 ** 功  能      ：
 ** 作  者      :
 ** 建立日期    ： 2001.9.24
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： agMsgbody : 报文
                   lgMsgoffset : 当前指针
                   iMSGMAXLEN : 报文最大长度
 ** 参数含义    ：  
                   
 ** 返回值      ： 0:成功 / -1:失败
************************************************************************/
int _ufPropSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilOffset;

  swVdebug(3,"S8636: [函数调用] _ufPropSET(%s)",aParm[0]);

  ilOffset = atoi(aParm[0]);
  
  if (ilOffset < 0 || ilOffset > iMSGMAXLEN)
  {
    return(FAIL);
  }
  lgMsgoffset = ilOffset;
  
  strcpy(aResult,"1");
  
  swVdebug(3,"S8639: [函数返回] _ufPropSET()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/*********************************************************************
 ** 函数名      ： _ufPropGET
 ** 功  能      ：
 ** 作  者      :
 ** 建立日期    ： 2001.9.24
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： agMsgbody : 报文
                   lgMsgoffset : 当前指针
                   iMSGMAXLEN : 报文最大长度
 ** 参数含义    ：

 ** 返回值      ： 0:成功 / -1:失败
************************************************************************/
int _ufPropGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(3,"S8642: [函数调用] _ufPropGET()");
  sprintf(aResult,"%hd",lgMsgoffset);
  swVdebug(3,"S8645: [函数返回] _ufPropGET()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/********************************************************************
 ** 函数名      ： _ufPropNSP
 ** 功  能      ：
 ** 作  者      :
 ** 建立日期    ： 2001.9.24
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： agMsgbody : 报文
                   lgMsgoffset : 当前指针
 ** 参数含义    ：  
                   
 ** 返回值      ： 0:成功 / -1:失败
**********************************************************************/
int _ufPropNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char *alPos;
  char alResult[iFLDVALUELEN+1];
  short ilLen;
  
  swVdebug(3,"S8648: [函数调用] _ufPropNSP(%s)",aParm[0]);

  ilLen = atoi(aParm[0]);
  /* add by zjj 2004.03.15 */
  if (ilLen >= iFLDVALUELEN)
  {
    swVdebug(3,"S8649: [错误/其他] _ufPropNSP,取值长度[%d]超过域值最大长度[%d]",ilLen,iFLDVALUELEN);
    return(FAIL);
  }
  /* end add by zjj */
  alPos = agMsgbody;
  alPos = alPos + lgMsgoffset;
  
  memcpy(alResult,alPos,ilLen);
  alResult[ilLen] = 0x00;
  _ufNtoT(alResult,ilLen,aResult);
  lgMsgoffset = lgMsgoffset + ilLen;

  swVdebug(3,"S8651: [函数返回] _ufPropNSP()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/********************************************************************
 ** 函数名      ： _ufProVARID
 ** 功  能      ：
 ** 作  者      :
 ** 建立日期    ： 2001.9.24
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： iBUFFLDNUM : 预解包最大池
 ** 参数含义    ：  
                   
 ** 返回值      ： 0:成功 / -1:失败
********************************************************************/

int _ufProVARID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilId;
  
  swVdebug(3,"S8654: [函数调用] _ufProVARID(%s)",aParm[0]);

  ilId = atoi(aParm[0]);
  if(ilId >= iBUFFLDNUM) 
  {
    swVdebug(0,"S8657: [错误/其它] 变量ID=%d不存在",ilId);
    return(FAIL);
  }
  else
  {
    _ufNtoT(psgVaridbuf[ilId].sImfbuf.aFldvalue,
      psgVaridbuf[ilId].sImfbuf.iFldlen,aResult);

    swVdebug(3,"S8660: [函数返回] _ufProVARID()返回码=0,结果=%s",aResult);
    return(SUCCESS);
  }
}


/* ============ add by dgm 2001/09/24 ========== */
/**************************************************************
 ** 函数名      ： _ufProBITGET
 ** 功  能      ： 取字符串中的某位
 ** 作  者      ：
 ** 建立日期    ： 2001.9.24
 ** 最后修改日期： 
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
                   aParm[1] : 位
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufProBITGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  unsigned char clTmp, clMask = 1;
  char alStr[20];
  short ilRtncode, iLen, ilBit;
  
  swVdebug(3,"S8663: [函数调用] _ufProBITGET(%s,%s)",aParm[0],aParm[1]);
  
  _ufTtoN(aParm[0], alStr, &ilRtncode);
  _ufTtoN(aParm[1], aResult, &ilRtncode);
  ilBit = atoi(aResult);
  
  iLen = strlen(alStr);
  if (ilBit > 8 * iLen || ilBit <= 0)
  {
    swVdebug(0,"S8666: [错误/其它] 要取的位不存在！");
    return(FAIL);
  }
  
  clMask = clMask << (8 - ((ilBit - 1) % 8 + 1));
  clTmp = clMask;
  
  if (alStr[(ilBit - 1) / 8] & clTmp)
    strcpy(aResult, "1");
  else 
    strcpy(aResult, "0");
    
  swVdebug(3,"S8669: [函数返回] _ufProBITGET()返回码=0,结果=%s",aResult);
  return(SUCCESS); 
   
}

/**************************************************************
 ** 函数名      ： _ufProBITSET
 ** 功  能      ： 设置字符串中的某位为1/0
 ** 作  者      ：
 ** 建立日期    ： 2001.9.24
 ** 最后修改日期： 
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 字符串
                   aParm[1] : 位
                   aParm[2] : 1/0
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufProBITSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  unsigned char clTmp, clMask = 1;
  char alStr[20], alResult[20];
  short ilRtncode, iLen, ilBit, ilVal;
  
  swVdebug(3,"S8672: [函数调用] _ufProBITSET(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
  
  _ufTtoN(aParm[0], alStr, &iLen);
  _ufTtoN(aParm[1], aResult, &ilRtncode);
  ilBit = atoi(aResult);
  _ufTtoN(aParm[2], aResult, &ilRtncode);
  ilVal = atoi(aResult);
  
  if (ilBit > 8 * iLen || ilBit <= 0)
  {
    swVdebug(0,"S8675: [错误/其它] 要置的位不存在！");
    return(FAIL);
  }
  
  memcpy(alResult, alStr, sizeof(alStr));
  clMask = clMask << (8 - ((ilBit - 1) % 8 + 1));
  clTmp = clMask;
  
  if (ilVal == 1)
    alResult[(ilBit - 1) / 8] = alResult[(ilBit - 1) / 8] | clTmp;
  else if (ilVal == 0)
  {
    clTmp = ~clTmp;
    alResult[(ilBit - 1) / 8] = alResult[(ilBit - 1) / 8] & clTmp;
  }
  else
  {
    swVdebug(0,"S8678: [错误/其它] 要置的值错误，只能为1/0 ！");
    return(FAIL);
  }
    
  _ufNtoT(alResult, iLen , aResult);
    
  swVdebug(3,"S8681: [函数返回] _ufProBITSET()返回码=0,结果=%s",aResult);
  return(SUCCESS); 
   
}

/* ========= end of add by dgm 2001/09/24 ====== */

/**************************************************************
 ** 函数名      ： _ufProFMLPACKSET
 ** 功  能      ：
 ** 作  者      :
 ** 建立日期    ： 2001.9.7
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： agMsgbody : 报文
 ** 参数含义    ： aParm[0] : 域名
                   aParm[1] : 域指
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufProFMLPACKSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilLen,ilResultlen;
  short ilRc;
  char alFldname[iFLDNAMELEN+1],alFldvalue[iFLDVALUELEN+1];
  char alResult[iFLDVALUELEN+1];

  swVdebug(3,"S8684: [函数调用] _ufProFMLPACKSET(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0], alFldname, &ilRc);
  _ufTtoN(aParm[1], alFldvalue, &ilRc);

  ilLen = strlen(alFldvalue);

  ilResultlen = strlen(alFldname);
  memcpy(alResult,alFldname,ilResultlen);

  alResult[ilResultlen] = '\0';
  ilResultlen ++;

  memcpy(alResult+ilResultlen,(char *)&ilLen,sizeof(short));
  ilResultlen = ilResultlen + sizeof(short);

  memcpy(alResult+ilResultlen,alFldvalue,strlen(alFldvalue));
  ilResultlen = ilResultlen + strlen(alFldvalue);
  alResult[ilResultlen] = '\0';

  _ufNtoT(alResult,ilResultlen,aResult);

  swVdebug(3,"S8687: [函数返回] _ufProFMLPACKSET()返回码=0,结果=%s",aResult);
  return(SUCCESS);
 
}
/**************************************************************
 ** 函数名      ： _ufProBITMAP
 ** 功  能         生成BITMAP
 ** 作  者      :
 ** 建立日期    ： 2001.9.7
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： psgUnpackbuf : 报文预置包
 ** 参数含义    ： 
                 
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufProBITMAP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  unsigned char clMask;
  int i,ilNum,ilMod,ilId,ilFlag=0,ilResultlen;
  char alResult[21];

  swVdebug(3,"S8690: [函数调用] _ufProBITMAP()");

  memset(alResult,0x00,21);

  ilResultlen = 8;
  i = 0;
  while(1)
  {
    ilId = psgUnpackbuf[i].sIsobuf.iFldid;
    i++;

    if (ilId == -1) break;
    if (ilId == 0) continue;

    if (ilId > 64 && ilFlag != 1)
    {
      ilFlag = 1;
      ilResultlen = 16;
      clMask = 1 << 7;
      alResult[0] = alResult[0] | clMask;
    }
    ilNum = (ilId-1)/8;
    ilMod = (ilId - 1)%8 + 1;
    clMask = 1 << (8 - ilMod);
    alResult[ilNum] = alResult[ilNum] | clMask; 
  }

  _ufNtoT(alResult,ilResultlen,aResult);

  swVdebug(3,"S8693: [函数返回] _ufProBITMAP()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/**************************************************************
 ** 函数名      ： _ufProA2E
 ** 功  能    
 ** 作  者      :
 ** 建立日期    ： 2001.9.7
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： psgUnpackbuf : 报文预置包
 ** 参数含义    ： aParm[0] 源串
 ** 参数含义    ： aParm[l] 长度
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufProA2E(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilLen;
  char alTmp[iFLDVALUELEN+1],alResult[iFLDVALUELEN+1];

  swVdebug(3,"S8696: [函数调用] _ufProA2E(%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0],alTmp,&ilLen);

/** delete by bmy 2002.8.1
  ilLen = atoi(aParm[1]);
 ** end of delete **/
 
/* delete by gxz 2001.10.9
  _swAtoE(alTmp,alResult,ilLen);*/
/* add by gxz 2001.10.9 */
  _swAtoE((unsigned char *)alResult,(unsigned char *)alTmp,ilLen);
/* add by gxz 2001.10.9 */
  
  _ufNtoT(alResult,ilLen,aResult);

  swVdebug(3,"S8699: [函数返回] _ufProA2E()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ： _ufProE2A
 ** 功  能    
 ** 作  者      :
 ** 建立日期    ： 2001.9.7
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： psgUnpackbuf : 报文预置包
 ** 参数含义    ： aParm[0] 源串
 ** 参数含义    ： aParm[l] 长度
 ** 返回值      ： 0:成功 / -1:失败
***************************************************************/
int _ufProE2A(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilLen;
  char alTmp[iFLDVALUELEN+1],alResult[iFLDVALUELEN+1];

  swVdebug(3,"S8702: [函数调用] _ufProE2A(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0],alTmp,&ilLen);

/** delete by bmy 2002.8.1
  ilLen = atoi(aParm[1]);
 ** end of delete **/
 
/*
  _swEtoA(alTmp,alResult,ilLen);  delete by gxz 2001.10.9 */
/* add by gxz 2001.10.9 */
  _swEtoA((unsigned char *)alResult,(unsigned char *)alTmp,ilLen);
/* add by gxz 2001.10.9 */

  _ufNtoT(alResult,ilLen,aResult);

  swVdebug(3,"S8705: [函数返回] _ufProE2A()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/*******************************************************************/
/* 函 数 名       :  _ufProA2B                                     */
/* 功    能       :  将一ASCII数字串转换为压缩的BCD数              */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年10月12日                                */
/* 最后修改日期   :  1998年10月12日                                */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  aParm[0] 源串                                 */ 
/*                   aParm[1] 长度                                 */
/*                   aParm[2] 填充标志                             */
/*                   奇数位时添0方式                               */
/*                   1:左加0 2:右加0                               */
/* 返 回 值       :  pcBCD BCD数                                   */
/*******************************************************************/
int _ufProA2B(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilResultlen;
  short ilLen,ilActiveNibble;
  char  alInBuf[iFLDVALUELEN+1],alOutBuf[iFLDVALUELEN+1];
  int   rc;    
  char  *OutBuf,*InBuf;      
  char  CharIn;      
  u_char   CharOut; 
  
  swVdebug(3,"S8708: [函数调用] _ufProA2B(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  rc = 0;  

  _ufTtoN(aParm[0],alInBuf,&ilLen);
  InBuf = alInBuf;
  
  ilLen = atoi(aParm[1]);
  ilActiveNibble = atoi(aParm[2]);
  
  memset(alOutBuf,0x00,(ilLen+1)/2);
  OutBuf = alOutBuf;
  
  ilResultlen = (ilLen+1)/2;

  if(ilLen % 2 == 1)
  {
    ilActiveNibble = (ilActiveNibble == 1 || ilActiveNibble==3)? 1 : 0;
  }
  else
    ilActiveNibble = 0;

  for (;(ilLen > 0);ilLen--,InBuf++)
  {
    CharIn = *InBuf;
    
    if (!isxdigit(CharIn))    /* validate character */
    {
      rc = -1;
    }
    else
    {
      if (CharIn > '9')
      {
        CharIn += 9;      /* Adjust Nibble for A-F */
      }
    }

    if (rc == 0)
    {
      CharOut = *OutBuf;
      if (ilActiveNibble)    
      {
        *OutBuf++ = (unsigned char)((CharOut & 0xF0) | ( CharIn  & 0x0F));
      }
      else
      {
        *OutBuf = (unsigned char)((CharOut & 0x0F) | ((CharIn & 0x0F) << 4));
      }
      ilActiveNibble ^= 1;      /* Change Active Nibble */
    }
  }
  
  _ufNtoT(alOutBuf,ilResultlen,aResult);

  swVdebug(3,"S8711: [函数返回] _ufProA2B()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}

/*******************************************************************/
/* 函 数 名       :  _ufProB2A                                    */
/* 功    能       :  将一压缩的BCD数转换为ASCII数字串(0-9A-F)      */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年10月12日                                */
/* 最后修改日期   :  1998年10月12日                                */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  aParm[0] 源串                                 */
/*                   aParm[1] 长度                                 */
/*                   aParm[2] 奇数位时截取方式                     */
/*                   1:左去0 2:右去0 3:左去空格 4:右去空格         */
/* 返 回 值       :  pcASC ASCII字符串                             */ 
/*******************************************************************/
/* 修改记录       :                                                */
/*                                                                 */
/*******************************************************************/
int _ufProB2A(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char pcBCD[iFLDVALUELEN+1];
  char pcASC[iFLDVALUELEN+1];
  short  iLength;
  int  i, iCutFlag;

  swVdebug(3,"S8714: [函数调用] _ufProB2A(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],pcBCD,&iLength);
  
  iLength = atoi(aParm[1]);
  iCutFlag = atoi(aParm[2]);
    
  if(iLength%2==0 || (iLength%2==1 && iCutFlag%2==0))
  {
     for(i=0; i<iLength; i++)
     {
       pcASC[i*2] = (unsigned char)pcBCD[i]/16+'0' ;
       if(pcASC[i*2] > '9') pcASC[i*2] = (unsigned char)pcASC[i*2] + 7;
       pcASC[i*2+1] = (unsigned char)pcBCD[i]%16+'0';
       if(pcASC[i*2+1] > '9') pcASC[i*2+1] = (unsigned char)pcASC[i*2+1] + 7;
     }
     pcASC[iLength] = '\0';
  }
  else 
  {
     pcASC[0] = (unsigned char)pcBCD[0]%16+'0';
     for(i=1; i<iLength; i++ )
     {
       pcASC[i*2-1] = (unsigned char)pcBCD[i]/16+'0';
       if(pcASC[i*2-1] > '9') pcASC[i*2-1] = (unsigned char)pcASC[i*2-1] + 7;
       pcASC[i*2] = (unsigned char)pcBCD[i]%16+'0'; 
       if(pcASC[i*2] > '9') pcASC[i*2] = (unsigned char)pcASC[i*2] + 7;
     }
     pcASC[iLength] = '\0';
  }
  
  _ufNtoT(pcASC,iLength,aResult);

  swVdebug(3,"S8717: [函数返回] _ufProB2A()返回码=0,结果=%s",aResult);
  return(SUCCESS);  
}

/*******************************************************************/
/* 函 数 名       :  _ufProPORTSET                                 */
/* 功    能       :  设置端口状态                                  */
/* 作    者       :  nh                                            */
/* 建立日期       :  2002年2月6日                                  */
/* 最后修改日期   :  2002年2月6日                                  */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  aParm[0] 端口号                               */
/*                   aParm[1] 层次                                 */
/*                   aParm[2] 设置的状态                           */
/* 返 回 值       :  0:SUCCESS -1:FAIL                             */ 
/*******************************************************************/
/* 修改记录       :                                                */
/*******************************************************************/
int _ufProPORTSET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilRc;
  short ilPort,ilLevel,ilStatus;

  swVdebug(3,"S8720: [函数调用] _ufProPORTSET(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  ilPort = atoi(aParm[0]);
  ilLevel = atoi(aParm[1]);
  ilStatus = atoi(aParm[2]);
 
  ilRc = swPortset(ilPort, ilLevel, ilStatus);
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S8723: [错误/函数调用] swPortset()函数,返回码=%d",ilRc);
    return(FAIL);
  }

  aResult[0]=0;

  return(SUCCESS);
}

/*******************************************************************/
/* 函 数 名       :  _ufProPORTGET                                 */
/* 功    能       :  取得端口状态                                  */
/* 作    者       :  nh                                            */
/* 建立日期       :  2002年2月6日                                  */
/* 最后修改日期   :  2002年2月6日                                  */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  aParm[0] 端口号                               */
/*                   aParm[1] 层次                                 */
/* 返 回 值       :  当前端口的状态                                */ 
/*******************************************************************/
/* 修改记录       :                                                */
/*******************************************************************/
int _ufProPORTGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilRc;
  short ilPort,ilLevel,ilTmp;

  swVdebug(3,"S8726: [函数调用] _ufProPORTGET(%s,%s)",aParm[0],aParm[1]);

  ilPort = atoi(aParm[0]);
  ilLevel = atoi(aParm[1]);
 
  ilRc = swPortget(ilPort, ilLevel, &ilTmp);
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S8729: [错误/函数调用] swPortget()函数,返回码=%d",ilRc);
    return(FAIL);
  }

  sprintf(aResult,"%d",ilTmp);

  return(SUCCESS);
}

/*******************************************************************/
/* 函 数 名  :  _ufProLA2B                                         */
/* 作 者  :                                                        */
/* 建立日期  :  2002年2月6日                                       */
/* 最后修改日期  :  2002年2月6日                                   */
/* 调用其它函数  :                                                 */
/* 全局变量  :                                                     */
/* 参数含义  :                                                     */
/* 返 回 值  :                                                     */
/*******************************************************************/
/* 修改记录  :                                                     */
/*******************************************************************/
int _ufProLA2B(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  int ilLen=atoi(aParm[0]);

  if (ilLen%2 ==1)
    sprintf(alTmp,"%d",ilLen/2+1);
  else
    sprintf(alTmp,"%d",ilLen/2);
  _ufNtoT(alTmp,strlen(alTmp),aResult);
  return(0);
}

/** add by bmy 2002.8.1 **/


/*********************************************************************
 ** 函数名      ： _ufPropSETR
 ** 功  能      ：
 ** 作  者      :
 ** 建立日期    ： 2002/4/20
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： agMsgbody : 报文
                   lgMsgoffset : 当前指针
                   iMSGMAXLEN : 报文最大长度
 ** 参数含义    ：  
                   
 ** 返回值      ： 0:成功 / -1:失败
************************************************************************/
int _ufPropSETR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilOffset;

  swVdebug(3,"S8732: [函数调用] _ufPropSET(%s)",aParm[0]);

  ilOffset = atoi(aParm[0]);
  
  if (ilOffset < 0 || ilOffset > iMSGMAXLEN)
  {
    return(FAIL);
  }
  lgMsgoffset += ilOffset;
  
  strcpy(aResult,"1");
  
  swVdebug(3,"S8735: [函数返回] _ufPropSET()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ： _ufProUFML
 ** 功  能      ： 根据标准域名取得标准域值
 ** 作  者      ： 史正烨
 ** 建立日期    ： 2002/4/27
 ** 最后修改日期： 2002/4/27
 ** 调用其它函数：
 ** 全局变量    ： psgUnpackbuf : 预置包缓冲池
 ** 参数含义    ： aParm[0] : 标准域名
 ** 返回值      ： 标准域值
***************************************************************/
int _ufProUFML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRtncode;
  short ilLen=0;
  char alTmp[iFLDVALUELEN];

  swVdebug(3,"S8738: [函数调用] _ufProFML(%s)",aParm[0]);
     
  strcpy(alTmp,aParm[0]); 
  ilRtncode=swFmlget(psgUnpackbuf,aParm[0],&ilLen,alTmp);
/*
  if (ilRtncode==FAIL)
  {
    swVdebug(0,"S8741: [错误/函数调用] swFmlget()函数,返回码=%d",ilRtncode);
    return(-1);
  }
*/
  if (ilRtncode==FAIL) aResult[0] = '\0';

  _ufNtoT(alTmp,ilLen,aResult);
  swVdebug(3,"S8744: [函数返回] _ufProFML()返回码=0,结果=%s",aResult);

  return(0);
}

#ifdef LIBXML

int swXmlinit()
{
  short ilRc;  
swVdebug(3,"add by pc 20150417 [%10.10s]",agMsgbody);
  ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);  
  if(ilRc)
  {
  	swVdebug(2,"xmlunpackInit ilRc[%d]",ilRc);
    if(ilRc == 3)  /*已经初始化, 强行释放*/
    {
      xmlFree();
      /* add by nh 20040607 */
      cgXmlInit = 0;
      /* end add */     
      ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);
      if(ilRc) 
      {
        swVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %s",
          xmlDescription(ilRc));
        return(FAIL);
      }
    }   
    else
      return(FAIL);
  }
  cgXmlInit = 1;
  return(SUCCESS);
}

int swXmlfree()
{
  xmlFree();
  cgXmlInit = 0;
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_ufProGETVALUE
 ** 功    能：解包时取得XML元素的值
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2004/1/8
 ** 调用函数：
 ** 全局变量：agMsgbody igMsgbodylen
 ** 参数含义：aXpath:XPATH定位字符串
 ** 返 回 值：aValue:域值
***************************************************************/
int _ufProGETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRc;
  short ilLen;
  char alXpath[iFLDVALUELEN];
  short ilFlag = 0;
  xmlChar *path_tmp = NULL;

  _ufTtoN(aParm[0],alXpath,&ilLen);
  swVdebug(3,"S8783: [函数调用] _ufProGETVALUE (%s)..",alXpath);
  if(iParmCount == 0)
  {
     swVdebug(0,"S8784: [函数调用] _ufProGETVALUE 错误, 参数为空");
     return(FAIL);
  }
swVdebug(5,"cgXmlInit[%d]",cgXmlInit);
  if(cgXmlInit == 0)
  { 	
    ilRc = swXmlinit();  
    if (ilRc)
    {
      swVdebug(0,"S8785: GETVALUE() swXmlinit error[%d]",ilRc);
      return(FAIL);
    }   
    ilFlag = 1;
    swVdebug(1,"S8786: GETVALUE() swXmlinit success");
  }
  swVdebug(5,"S8791: [函数调用]_ufProGETVALUE ,初始化成功");
  ilRc = xmlGetNode(alXpath,aResult,NULL);
  if((ilRc != SUCCESS) && (ilRc != 2))
  {
  /*add by pc 2007-4-7 06:11下午*/	
  if(ilFlag == 1)
    swXmlfree();
  /*add by pc 2007-4-7 06:11下午*/    	
     swDebug("S8790: [函数调用] _ufProGETVALUEN 错误, %s",xmlDescription(ilRc));
     return(FAIL);
  }
  if(ilRc == 2)
   	strcpy(aResult,"");
     
  swVdebug(3,"S8804: value=[%s],len=[%d]",aResult,strlen(aResult));
  swVdebug(2,"S8810: _ufProGETVALUE success!");
  if(ilFlag == 1)
    swXmlfree();
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_ufProGETVALUE
 ** 功    能：解包时取得XML元素的值
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2004/1/8
 ** 调用函数：
 ** 全局变量：agMsgbody igMsgbodylen
 ** 参数含义：aXpath:XPATH定位字符串
 ** 返 回 值：aValue:域值
***************************************************************/
int _ufProGETVALUEM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRc;
  short ilLen,ilResultlen;
  char alXpath[iFLDVALUELEN+1],alTmp[100],alTmp1[100],alTmpexp[100];
  short ilFlag = 0;
  int i;
  
  memset(alXpath,0x00,sizeof(alXpath));
	memset(alTmp,0x00,sizeof(alTmp));
	memset(alTmp1,0x00,sizeof(alTmp1));
	memset(alTmpexp,0x00,sizeof(alTmpexp));
  _ufTtoN(aParm[0],alXpath,&ilLen);
  
  strcpy(alTmp,strstr(alXpath,"[")+1);
  /*2008-1-29 add by pc 不应该任何情况下-1 目的是为了去掉最后的中括号，这里进行判断，如果最后一位是 中括号再处理*/
  /*alTmp[strlen(alTmp)-1] = 0;*/
  if (alTmp[strlen(alTmp)-1]==']')
  	alTmp[strlen(alTmp)-1] = 0;
  else
  	alTmp[strlen(alTmp)] = 0;
  /*2008-1-29 add by pc*/
  ilRc = _swExpN2T(alTmp,alTmpexp);
  if (ilRc != 0) 
  {
    swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
    return(FAIL);
  }
  ilRc = _swExpress(alTmpexp,alTmp1,&ilResultlen);  
  if(ilRc != SUCCESS)
  {
    swVdebug(1,"S0260: [函数调用] _ufProGETVALUEM 计算平台表达式出错");
    return(FAIL);
  }
  swVdebug(1,"S0270: 计算平台表达式成功,result=[%s]",alTmp1);
  for(i=0;i<strlen(alXpath);i++)
  {
    if(alXpath[i] == '[')
      break;
  }
  alXpath[i+1]=0;
  strcat(alXpath,alTmp1);
  strcat(alXpath,"]");
              
  swVdebug(3,"S8783: [函数调用] _ufProGETVALUEM (%s)",alXpath);
  if(iParmCount == 0)
  {
     swVdebug(0,"S8784: [函数调用] _ufProGETVALUEM 错误, 参数为空");
     return(FAIL);
  }
  if(cgXmlInit == 0)
  {
    ilRc = swXmlinit();
    if (ilRc)
    {
      swVdebug(0,"S8785: GETVALUE() swXmlinit error");
      return(FAIL);
    }
    ilFlag = 1;
    swVdebug(1,"S8786: GETVALUE() swXmlinit success");
  }
  swVdebug(5,"S8791: [函数调用]_ufProGETVALUEM ,初始化成功");
  /* modify by nh 20040419 
  ilRc = xmlGetValue(alXpath,aResult);*/
  ilRc = xmlGetNode(alXpath,aResult,NULL);
  if(ilRc != SUCCESS && ilRc != 2)
  {
  /*add by pc 2007-4-7 06:11下午*/	
  if(ilFlag == 1)
    swXmlfree();
  /*add by pc 2007-4-7 06:11下午*/   	
     swDebug("S8790: [函数调用] _ufProGETVALUEM 错误, %s",
       xmlDescription(ilRc));
     
     return(FAIL);
  }
  if(ilRc == 2)
   strcpy(aResult,"");
  
  swVdebug(3,"S8804: value=[%s],len=[%d]",aResult,strlen(aResult));
  swVdebug(2,"S8810: _ufProGETVALUEM success!");
  if(ilFlag == 1)
    swXmlfree();
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_ufProSETVALUE
 ** 功    能：解包时取得XML元素的值
 ** 作    者：cgx
 ** 建立日期：2003/11/17
 ** 修改日期：
 ** 调用函数：_swParesXpath()
 ** 全局变量：sgMsgpack.aMsgbody
 ** 参数含义：aXpath:XPATH定位字符串   aValue:域值
 ** 返 回 值：
***************************************************************/
int _ufProSETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc;
  short ilLen;
  char alXpath[iFLDVALUELEN],alValue[iFLDVALUELEN];
  
  if(iParmCount < 2)
  {
     swVdebug(0,"S9355: [调用函数]_ufProSETVALUE 出错，参数错误");
     return(FAIL);
  }
  swVdebug(1,"S9355: [函数调用]_ufProSETVALUE(%s,%s)", aParm[0],aParm[1]);
  
  _ufTtoN(aParm[0],alXpath,&ilLen);
  _ufTtoN(aParm[1],alValue,&ilLen);
  
  if(!strcmp(alValue,"NULL") || !strcmp(alValue,"null"))
    alValue[0] = 0;

  ilRc = xmlSetNode(alXpath,alValue);
  if(ilRc)
  {
     swVdebug(0,"S9355: [调用函数]_ufProSETVALUE 出错，%s",
        xmlDescription(ilRc));
     return(FAIL);
  }
  
  aResult[0]=0;
  swVdebug(5,"S9355:[调用结束]_ufProSETVALUE()返回=0,结果=%s",aResult);
  return(0);
}

/**************************************************************
 ** 函 数 名：_ufProGETPROP()
 ** 功    能：取出指定域名属性的值
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2004/1/8
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：aXpath:定位字符串
 **           aProname:属性名
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProGETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
   char alXpath[iFLDVALUELEN+1];
   char alProp[iFLDVALUELEN+1];
   short ilpathlen,ilproplen;
   int ilRc;
   short ilFlag = 0;
   
   if(iParmCount != 2)
   {
      swVdebug(0,"S8789: [函数调用]_ufProGETVALUE 错误, 参数错误");
      return(FAIL);
   }
   if(cgXmlInit == 0)
   {
     ilRc = swXmlinit();
     if (ilRc)
     {
       swVdebug(0,"S8792: GETVALUE() swXmlinit error");
       return(FAIL);
     }
     ilFlag = 1;
     swVdebug(1,"S8795: GETVALUE() swXmlinit success");
   }
   _ufTtoN(aParm[0],alXpath,&ilpathlen);
   _ufTtoN(aParm[1],alProp,&ilproplen);

   /* modify by nh 20040419 
   strcat(alXpath,"[@");
   strcat(alXpath,alProp);
   strcat(alXpath,"]");
   
   ilRc = xmlGetValue(alXpath,aResult);*/
   ilRc = xmlGetProp(alXpath,alProp,aResult);
   if(ilRc)
   {
      swDebug("S8789: [函数调用]_ufProGETPROP 错误, %s",
         xmlDescription(ilRc));
      return(FAIL);
   }
   if(ilFlag == 1) 
     swXmlfree();
   return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_ufProADDPROP()
 ** 功    能：打包时置XML域属性的值
 ** 作    者：cgx
 ** 建立日期：2003/11/18
 ** 修改日期：
 ** 调用函数：_swParesXpath(),_swWeaveField
 ** 全局变量：
 ** 参数含义：aXpath:XPATH定位字符串
 **           aName:新增属性名
 **           aValue:新增属性的值
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProADDPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc; 
  short ilLen;
  char alXpath[iFLDVALUELEN];
  char alProp[iFLDVALUELEN];
  char alValue[iFLDVALUELEN];
  char *pValue=NULL;

  if(iParmCount < 3)
  {
     swVdebug(0,"S9330:[调用函数]_ufProADDPROP 出错，参数错误");
     return(FAIL);
  }

  swVdebug(1,"S9330:[调用函数]_ufProADDPROP (%s,%s,%s)",
    aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],alXpath,&ilLen);
  _ufTtoN(aParm[1],alProp,&ilLen);
  if(iParmCount > 2)
  {
     _ufTtoN(aParm[2],alValue,&ilLen);
     if(!strcmp(alValue,"NULL") || !strcmp(alValue,"null"))
        alValue[0]=0x00;
     pValue = alValue;
  }
    
  ilRc = xmlAddProp(alXpath,alProp,pValue);
  if(ilRc)
  {
     swDebug("S9315: [函数调用]_ufProADDPROP() 出错, %s",
        xmlDescription(ilRc));
     return(FAIL);
  }

  swVdebug(5,"S9315: [调用结束]_ufProADDPROP()返回=0,结果=%d",ilRc);
  return(0);
}

/**************************************************************
 ** 函 数 名：_ufProSETPROP()
 ** 功    能：打包置XML某域的属性值
 ** 作    者：cgx
 ** 建立日期：2003/11/24
 ** 修改日期：
 ** 调用函数：_swParesXpath()
 ** 全局变量：
 ** 参数含义：aXpath:定位字符串
 **           aProname:属性名
 **           aValue:属性值
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProSETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc;
  short ilLen;
  char alXpath[iFLDVALUELEN];
  char alProp[iFLDVALUELEN];
  char alValue[iFLDVALUELEN];

  if(iParmCount < 3)
  {
     swVdebug(0,"S9330:[调用函数]_ufProSETPROP 出错，参数错误");
     return(FAIL);
  }

  swVdebug(1,"S9330:[调用函数]_ufProSETPROP (%s,%s,%s)",
    aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],alXpath,&ilLen); 
  _ufTtoN(aParm[1],alProp,&ilLen);
  _ufTtoN(aParm[2],alValue,&ilLen);
  
  ilRc = xmlSetProp(alXpath,alProp,alValue);
  if(ilRc)
  {
     swVdebug(0,"S9330:[调用函数]_ufProSETPROP 出错，%s",
       xmlDescription(ilRc));
     return(FAIL);
  }
  aResult[0]='\0';
  swVdebug(5,"S9335:[调用结束]_ufProSETPROP()返回=0,结果=%s",aResult);
  return(0);
}


/**************************************************************
 ** 函数名      ： _ufProXMLDUMP
 ** 功  能      ： 打包时生成XML报文
 ** 作  者      ： nh
 ** 建立日期    ： 2002/8/20
 ** 最后修改日期： 2002/8/28
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLDUMP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilLen;
  short ilHeadlen;
  int  ilPos=0;
  int ilFldid = 2;
  int ilRc;
  char agXmlbuf[iMSGMAXLEN];
  char alHead[iMSGMAXLEN]; /*for 报文头 */
  
  
  swVdebug(1,"S8912: [函数调用]_ufProXMLDUMP()");
  ilRc = xmlDump(agXmlbuf,&ilLen);
  if(ilRc)
  {
      swVdebug(0,"S8913: [函数调用]_ufProXMLDUMP 出错, %s",
         xmlDescription(ilRc));
      return(FAIL);
  }
  swVdebug(3,"S8914: [函数调用]_ufProXMLDUMP [%d][%s]",ilLen,agXmlbuf);
  if(iParmCount == 1)
  {	
  	_ufTtoN(aParm[0],alHead,&ilHeadlen); 
    strcat(alHead,agXmlbuf);
    ilLen += ilHeadlen;
  }
  else
  	strcpy(alHead,agXmlbuf);
  while(1)
  {
    if (ilLen <= iPERFLDVALUELEN)
    {
      psgUnpackbuf[ilFldid-1].sIsobuf.iFldid = ilFldid;
      psgUnpackbuf[ilFldid-1].sIsobuf.iFldlen = ilLen;
      memcpy(psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue,alHead+ilPos,ilLen);
      psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue[ilLen] = 0;
      break;
    }
    psgUnpackbuf[ilFldid-1].sIsobuf.iFldid = ilFldid;
    psgUnpackbuf[ilFldid-1].sIsobuf.iFldlen = iPERFLDVALUELEN;
    memcpy(psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue,alHead+ilPos,
       iPERFLDVALUELEN);
    psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue[iPERFLDVALUELEN] = '\0';
    ilPos = ilPos + iPERFLDVALUELEN;
    ilLen = ilLen - iPERFLDVALUELEN;
    ilFldid ++;
  }
  psgUnpackbuf[ilFldid].sIsobuf.iFldid = -1;
  aResult[0]=0;
  swVdebug(5,"S8912: [函数返回] _ufProXMLDUMP()返回码=0");
  return(0);
}

/**************************************************************
 ** 函数名      ： _ufProXMLINIT()
 ** 功  能      ： 打包时生成XML报文
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2004/1/8
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] :根节点名
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLINIT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc;
  short ilLen;
  char alBuf[iFLDVALUELEN];
  char alTmp[10];

  swVdebug(3,"S8771: [函数调用]_ufProXMLINIT");
  if(iParmCount == 2)
  {
    _ufTtoN(aParm[1],alBuf,&ilLen);
    ilRc = xmlpackInit("1.0",alBuf);
    if(ilRc)
    {
      if(ilRc == 3)  /*已经初始化, 强行释放*/
      {
        xmlFree();
        /* add by nh 20040607 */
        cgXmlInit = 0;
        /* end add */
        ilRc = xmlpackInit("1.0",alBuf);
        if(ilRc) return(FAIL);
      }
      else
        return(FAIL);
    }   
    /*添加根节点*/
    _ufTtoN(aParm[0],alBuf,&ilLen);
    strcpy(alTmp,"");
    ilRc = xmlAddNode("/",alBuf,alTmp);   
    if(ilRc) 
    {
      swVdebug(0,"S9346: [函数调用]_ufProXMLINIT() 出错, %s",
        xmlDescription(ilRc));
      return(FAIL);
    }
  }
  else if(iParmCount == 1)
  {  	
    _ufTtoN(aParm[0],alBuf,&ilLen); /* 有报文头，需要偏移 nh 20060504 */
    ilLen = atoi(alBuf);
    ilRc = xmlunpackInit(agMsgbody+ilLen,igMsgbodylen-ilLen);   
    if(ilRc)
    {
      if(ilRc == 3)  /*已经初始化, 强行释放*/
      {
        xmlFree();
        /* add by nh 20040607 */
        cgXmlInit = 0;
        /* end add */
        ilRc = xmlunpackInit(agMsgbody+ilLen,igMsgbodylen-ilLen);      
        if(ilRc) 
        {
          swVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %s",
            xmlDescription(ilRc));
          return(FAIL);
        }
      }
      else
        return(FAIL);
    }
    cgXmlInit = 1;
  }
  else
  { 	
    ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);   
    if(ilRc)
    {
      if(ilRc == 3)  /*已经初始化, 强行释放*/
      {
        xmlFree();
        /* add by nh 20040607 */
        cgXmlInit = 0;
        /* end add */     
        ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);        
        if(ilRc) 
        {
          swVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %s",
            xmlDescription(ilRc));
          return(FAIL);
        }
      }
      else
        return(FAIL);
      cgXmlInit = 1;
    }
  }
  aResult[0]=0;
  swVdebug(5,"S9345: [调用结束]_ufProXMLINIT()返回=0,结果=%s",aResult);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ： _ufProXMLFREE()
 ** 功  能      ： 释放XML资源
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2004/1/8
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLFREE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(3,"S8783: [函数调用] _ufProXMLFREE");
  xmlFree();
  aResult[0] = 0x00;
  cgXmlInit = 0;
  return(SUCCESS);
}
#endif


/*add by zcd 20141218*/
#ifdef LIBXML2
/**************************************************************
 ** 函数名      ： swXmlinit()
 ** 功  能      ： xml环境初始化
 ** 作  者      ： 
 ** 建立日期    ： 
 ** 最后修改日期： 2014/12/16
 ** 调用其它函数：
 ** 全局变量    ： psgUnpackbuf : 预置包缓冲池
 ** 参数含义    ： aParm[0] : 标准域名
 ** 返回值      ： 标准域值
***************************************************************/
int swXmlinit()
{
  short ilRc;  
 
  if(NULL == agMsgbody || igMsgbodylen <= 0) {
	swVdebug(2,"传入的报文非法[%d] [%10.10s]",igMsgbodylen,agMsgbody);
	return(11);
   }
  /*delete by zcd 20141218
  ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);
  ***end of delete by zcd 20141218*/

  /*add by zcd 20141218*/
  ilRc = XMLInitFromBuf(agMsgbody);
  /*end of add by zcd 20141218*/
  if(ilRc)
  {
    swVdebug(5,"XMLInitFromBuf ilRc[%d]",ilRc);
    if(ilRc == 3)  /*已经初始化, 强行释放*/
    {
      XMLFree();
      /* add by nh 20040607 */
      cgXmlInit = 0;
      /* end add */     
      ilRc = XMLInitFromBuf(agMsgbody);
      if(ilRc) 
      {
        swVdebug(0,"S8771: [函数调用] _XMLInitFromBuf 出错, %d",
         ilRc);
        return(FAIL);
      }
    }   
    else
      return(FAIL);
  }
  cgXmlInit = 1;
  return(SUCCESS);
}

int swXmlfree()
{
  XMLFree();
  cgXmlInit = 0;
  return(SUCCESS);
}

/**************************************************************
 ** 函 数 名：_ufProGETVALUE
 ** 功    能：解包时取得XML元素的值
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2014/12/16
 ** 调用函数：
 ** 全局变量：agMsgbody igMsgbodylen
 ** 参数含义：aXpath:XPATH定位字符串
 ** 返 回 值：aValue:域值
***************************************************************/
extern int encflag;
int _ufProGETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRc;
  short ilLen;
  char alXpath[iFLDVALUELEN];
  short ilFlag = 0;

  _ufTtoN(aParm[0],alXpath,&ilLen);
  swVdebug(3,"S8783: [函数调用] _ufProGETVALUE (%s)",alXpath);
  if(iParmCount == 0)
  {
     swVdebug(0,"S8784: [函数调用] _ufProGETVALUE 错误, 参数为空");
     return(FAIL);
  }
swVdebug(3,"add by pc cgXmlInit[%d] 20150517",cgXmlInit);
 swVdebug(3,"encflag_2=%d",encflag);
  if(cgXmlInit == 0)
  { 	
    ilRc = swXmlinit();  
    if (ilRc)
    {
      swVdebug(0,"S8785: GETVALUE() swXmlinit error[%d]",ilRc);
      return(FAIL);
    }   
    ilFlag = 1;
    swVdebug(1,"S8786: GETVALUE() swXmlinit success");
  }
  swVdebug(5,"S8791: [函数调用]_ufProGETVALUE ,初始化成功");
  ilRc=StrToUTF8(alXpath);
  if(ilRc)                                             
	  swVdebug(3,"StrToUTF8 error %d",ilRc);    

  /*add by zcd 20141218*/
  ilRc = XMLGetText2(alXpath,aResult,0);
  /*end of add by zcd 20141218*/
  if((ilRc != SUCCESS) && (ilRc != 2))
  {
  /*add by pc 2007-4-7 06:11下午*/	
  /*del by zcd 20141224*/
   if(ilFlag == 1)
    swXmlfree();
  /*add by pc 2007-4-7 06:11下午*/    	
     swDebug("S8790: [函数调用] _ufProGETVALUEN 错误, %d",ilRc);
     return(FAIL);
  }
  if(ilRc == 2)
   	strcpy(aResult,"");
     
  swVdebug(3,"S8804: value=[%s],len=[%d]",aResult,strlen(aResult));
  swVdebug(3,"S8810: _ufProGETVALUE success!");
  /*del by zcd 20141224*/
  if(ilFlag == 1)
    swXmlfree();
  return(SUCCESS);
}

/****************************************************
 ** 函 数 名：getvalue()
 ** 功    能：按|解析字符串
 ** 作    者：tbl
 ** 建立日期：2003/11/18
 ** 调用函数：getvalue
 ** 全局变量：
 ** 参数含义：**buffer 传进来的字符串
 **            result  传送出去的值
 **           
 ** 返 回 值：0(成功) / -1(失败)
****************************************************/
int getvalue(char **buffer,char ** result)
{
	int len=0;
	char *p;
	p=strstr(*buffer,"|"); 
	len=strlen(*buffer)-strlen(p);
	if((*result=(char *)malloc(len)) == NULL){
		swVdebug(3,"mallloc error !!");
		return -1;
	}
	else		  	
		memset(*result,0x00,len+1);
	memcpy(*result,*buffer,len);
	p=strstr(*buffer,"|");
	*buffer=p+1;
	return 0;
}
/**************************************************************
 ** 函 数 名：_ufProGETVALUEM
 ** 功    能：解包时取得XML元素的值
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2014/12/16 by zcd
 ** 调用函数：
 ** 全局变量：agMsgbody igMsgbodylen
 ** 参数含义：aXpath:XPATH定位字符串
 ** 返 回 值：aValue:域值
***************************************************************/
int _ufProGETVALUEM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRc;
  short ilLen,ilResultlen;
  char alXpath[iFLDVALUELEN+1],alTmp[100],alTmp1[100],alTmpexp[100];
  short ilFlag = 0;
  int i;
  
  memset(alXpath,0x00,sizeof(alXpath));
	memset(alTmp,0x00,sizeof(alTmp));
	memset(alTmp1,0x00,sizeof(alTmp1));
	memset(alTmpexp,0x00,sizeof(alTmpexp));
  _ufTtoN(aParm[0],alXpath,&ilLen);
  
  strcpy(alTmp,strstr(alXpath,"[")+1);
  /*2008-1-29 add by pc 不应该任何情况下-1 目的是为了去掉最后的中括号，这里进行判断，如果最后一位是 中括号再处理*/
  /*alTmp[strlen(alTmp)-1] = 0;*/
  if (alTmp[strlen(alTmp)-1]==']')
  	alTmp[strlen(alTmp)-1] = 0;
  else
  	alTmp[strlen(alTmp)] = 0;
  /*2008-1-29 add by pc*/
  ilRc = _swExpN2T(alTmp,alTmpexp);
  if (ilRc != 0) 
  {
    swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
    return(FAIL);
  }
  ilRc = _swExpress(alTmpexp,alTmp1,&ilResultlen);  
  if(ilRc != SUCCESS)
  {
    swVdebug(1,"S0260: [函数调用] _ufProGETVALUEM 计算平台表达式出错");
    return(FAIL);
  }
  swVdebug(5,"S0270: 计算平台表达式成功,result=[%s]",alTmp1);
  for(i=0;i<strlen(alXpath);i++)
  {
    if(alXpath[i] == '[')
      break;
  }
  alXpath[i+1]=0;
  strcat(alXpath,alTmp1);
  strcat(alXpath,"]");
              
  swVdebug(5,"S8783: [函数调用] _ufProGETVALUEM (%s)",alXpath);
  if(iParmCount == 0)
  {
     swVdebug(0,"S8784: [函数调用] _ufProGETVALUEM 错误, 参数为空");
     return(FAIL);
  }
  if(cgXmlInit == 0)
  {
    ilRc = swXmlinit();
    if (ilRc)
    {
      swVdebug(0,"S8785: GETVALUE() swXmlinit error");
      return(FAIL);
    }
    ilFlag = 1;
    swVdebug(5,"S8786: GETVALUE() swXmlinit success");
  }
  swVdebug(5,"S8791: [函数调用]_ufProGETVALUEM ,初始化成功");
  /* delete by zcd 20141218
  ilRc = xmlGetValue(alXpath,aResult); 
  ***end of delete by zcd 20141218*/
  
  /*add by zcd 20141218*/
  ilRc = XMLGetText2(alXpath,aResult,0);
  /*end of add by zcd 20141218*/
  if(ilRc != SUCCESS && ilRc != 2)
  {
  /*add by pc 2007-4-7 06:11下午*/	
  /*del by zcd 20141224*/
  if(ilFlag == 1)
    swXmlfree();
  /*add by pc 2007-4-7 06:11下午*/   	
     swDebug("S8790: [函数调用] _ufProGETVALUEM 错误, %d",
       ilRc);
     
     return(FAIL);
  }
  if(ilRc == 2)
   strcpy(aResult,"");
  
  swVdebug(5,"S8804: value=[%s],len=[%d]",aResult,strlen(aResult));
  swVdebug(5,"S8810: _ufProGETVALUEM success!");
  /*del by zcd 20141224*/
  if(ilFlag == 1)
    swXmlfree();
  return(SUCCESS);
}


/**************************************************************
 ** 函 数 名：_ufProADDNODE_X()
 ** 功    能：打包时置XML域的值
 ** 作    者：cgx
 ** 建立日期：2003/11/18
 ** 修改日期：2014/12/16 by zcd
 ** 调用函数：_swParesXpath(),_swWeaveField
 ** 全局变量：
 ** 参数含义：aXpath:XPATH定位字符串
 **           aName:新增域名
 **           aValue:新增域名的域值
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProADDNODE_X(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int ilRc; 
	short ilLen;
	char alXpath_tmp[iFLDVALUELEN];
	char alNode_tmp[iFLDVALUELEN];
	char alValue_tmp[iFLDVALUELEN];

	char alXpath_tmp_s[iFLDVALUELEN];
	char alNode_tmp_s[iFLDVALUELEN];
	char alValue_tmp_s[iFLDVALUELEN];
	char muldata_tmp_s[iFLDVALUELEN];
	char alXpath1[iFLDVALUELEN];
	char alTmp[100];
	char *pValue_tmp=NULL;
	char *pMuldata_tmp=NULL;

	char col_tmp[iFLDVALUELEN];
	char row_tmp[iFLDVALUELEN];
	char muldata_tmp[iFLDVALUELEN];
	short col_num=0;
	short row_num=0;
	int rec;
	char * result;	
	char * result_muldata;
	int i=0;
	int j=0;
	int num;
	if(iParmCount < 4)
	{
		swVdebug(0,"S9314: [函数调用]_ufProADDNODE() 出错，参数错误");
		return(FAIL);
	}

	_ufTtoN(aParm[0],alXpath_tmp,&ilLen);
	_ufTtoN(aParm[1],alNode_tmp,&ilLen);
	_ufTtoN(aParm[2],alValue_tmp,&ilLen);

	if(!strcmp(alValue_tmp,"NULL") || !strcmp(alValue_tmp,"null"))
		alValue_tmp[0]=0x00;
	pValue_tmp = alValue_tmp;

	_ufTtoN(aParm[3],col_tmp,&ilLen);
	_ufTtoN(aParm[4],row_tmp,&ilLen);
	_ufTtoN(aParm[5],muldata_tmp,&ilLen);
	swVdebug(3,"muldata_tmp=%s",muldata_tmp);
	col_num=atoi(col_tmp);
	row_num=atoi(row_tmp);

	for(0;i<row_num;i++){
		memset(aParm[0],0x00,sizeof(aParm[0]));
		memset(aParm[1],0x00,sizeof(aParm[1]));
		memset(aParm[2],0x00,sizeof(aParm[2]));
		memset(muldata_tmp_s,0x00,sizeof(muldata_tmp_s));
		/*循环节点*/                      
		sprintf(alNode_tmp_s,"%s%s%d%s",alNode_tmp,"[",i+1,"]");

		pMuldata_tmp=muldata_tmp;
		memcpy(aParm[0],alXpath_tmp,sizeof(aParm[0]));
		memcpy(aParm[1],alNode_tmp_s,sizeof(aParm[1]));
		swVdebug(3,"alNode_tmp_s=%s",alNode_tmp_s);
		rec=_ufProADDNODE(aParm,3,aResult);
		if(rec)
		{
			swVdebug(3,"s_ufProADDNODE 取开始节点失败rec=%d",rec);
			return -2;
		}

		for(j=0;j<col_num;j++){    
			memset(aParm[0],0x00,sizeof(aParm[0]));
			memset(aParm[1],0x00,sizeof(aParm[1]));
			memset(aParm[2],0x00,sizeof(aParm[2]));
			memset(alXpath_tmp_s,0x00,sizeof(alXpath_tmp_s));
			memset(alNode_tmp_s,0x00,sizeof(alNode_tmp_s));
			memset(alValue_tmp_s,0x00,sizeof(alValue_tmp_s));

			sprintf(alXpath_tmp_s,"%s%s%s%s%d%s",alXpath_tmp,"/",alNode_tmp,"[",i+1,"]");
			/*取得node节点*/
			/*当col的值送长的时候，做判断*/
			num=strlen(pMuldata_tmp);
			if(num==0)
			{
				swVdebug(3,"数据已经取完,但是col error  pMuldata_tmp=[%s],col_num=[%d]",pMuldata_tmp,col_num);
				return -9;
				} 
			rec=getvalue(&pMuldata_tmp,&result_muldata);
			swVdebug(3,"rec_test=[%d]",rec);
			if(rec)
			{
				swVdebug(3,"S00 getvalue 取值失败rec=%d",rec);	
				free(result_muldata);
				return -4;
				}
			sprintf(alNode_tmp_s,"%s",result_muldata);                               
			swVdebug(3,"result_muldata=[%s]",result_muldata);
			/*取得多笔里面的数值*/
			rec=getvalue(&pValue_tmp,&result);
                        if(rec)
                        {
                                swVdebug(3,"S01 getvalue 取值失败rec=%d",rec);
				free(result);
                                return -5;
                                }
			if(result==NULL)
			{
				swVdebug(3,"没有取到节点，可能送的col送错了result=[%s]",result);
				return -8;
				free(result);
			}
			swVdebug(3,"result=[%s]",result);
			sprintf(alValue_tmp_s,"%s",result);

			memcpy(aParm[0],alXpath_tmp_s,sizeof(aParm[0]));
			memcpy(aParm[1],alNode_tmp_s,sizeof(aParm[1]));
			memcpy(aParm[2],alValue_tmp_s,sizeof(aParm[2]));
			rec=_ufProADDNODE(aParm,3,aResult);
			if(rec)
			{
				swVdebug(3,"s_ufProADDNODE 取开始节点失败_1rec=%d",rec);
				free(result);
				return -3;
			}
			free(result);
		}
	}



}


/**************************************************************
 ** 函 数 名：_ufProADDNODE()
 ** 功    能：打包时置XML域的值
 ** 作    者：cgx
 ** 建立日期：2003/11/18
 ** 修改日期：2014/12/16 by zcd
 ** 调用函数：_swParesXpath(),_swWeaveField
 ** 全局变量：
 ** 参数含义：aXpath:XPATH定位字符串
 **           aName:新增域名
 **           aValue:新增域名的域值
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProADDNODE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc; 
  short ilLen;
  char alXpath[iFLDVALUELEN];
  char alNode[iFLDVALUELEN];
  char alValue[iFLDVALUELEN];
  char alXpath1[iFLDVALUELEN];
  char alTmp[100];
  char *pValue=NULL;
  
  if(iParmCount < 3)
  {
     swVdebug(0,"S9314: [函数调用]_ufProADDNODE() 出错，参数错误");
     return(FAIL);
  }

  _ufTtoN(aParm[0],alXpath,&ilLen);
  _ufTtoN(aParm[1],alNode,&ilLen);
  if(iParmCount > 2)
  {
     _ufTtoN(aParm[2],alValue,&ilLen);
     if(!strcmp(alValue,"NULL") || !strcmp(alValue,"null"))
        alValue[0]=0x00;
     pValue = alValue;
  }
 
  /*add by zcd 20141218*/
  memset(alXpath1,0x00,sizeof(alXpath1));
  memset(alTmp,0x00,sizeof(alTmp)); 
  strcpy(alTmp,"/");
  sprintf(alXpath1,"%s%s%s",alXpath,alTmp,alNode);
  swVdebug(3,"alXpath1=%s",alXpath1);
  StrToUTF8(alXpath1);
  ilRc = XMLPutNode(alXpath1,pValue,0);
  /*end o add by zcd 20141218*/
  if(ilRc)
  {
     swVdebug(0,"S9315: [函数调用]_ufProADDNODE() 出错, %d",ilRc);
     return(FAIL);
  }
  aResult[0]=0;
  swVdebug(5,"S9315: [调用结束]_ufProADDNODE()返回=0,结果=%s",aResult);
  return(0);
}



/**************************************************************
 ** 函 数 名：_ufProSETVALUE
 ** 功    能：解包时取得XML元素的值
 ** 作    者：cgx
 ** 建立日期：2003/11/17
 ** 修改日期：2014/12/16 zcd
 ** 调用函数：_swParesXpath()
 ** 全局变量：sgMsgpack.aMsgbody
 ** 参数含义：aXpath:XPATH定位字符串   aValue:域值
 ** 返 回 值：
***************************************************************/
int _ufProSETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc;
  short ilLen;
  char alXpath[iFLDVALUELEN],alValue[iFLDVALUELEN];
  
  if(iParmCount < 2)
  {
     swVdebug(0,"S9355: [调用函数]_ufProSETVALUE 出错，参数错误");
     return(FAIL);
  }
  swVdebug(5,"S9355: [函数调用]_ufProSETVALUE(%s,%s)", aParm[0],aParm[1]);
  
  _ufTtoN(aParm[0],alXpath,&ilLen);
  _ufTtoN(aParm[1],alValue,&ilLen);
  
  if(!strcmp(alValue,"NULL") || !strcmp(alValue,"null"))
    alValue[0] = 0;
  /*delete by zcd 20141218
  ilRc = xmlSetNode(alXpath,alValue);
  ***end of delete by zcd 20141218*/
  
  /*add by zcd 20141218*/
  ilRc = XMLSetText(alXpath,alValue,0);
  /*end of add by zcd 20141218*/
  if(ilRc)
  {
     swVdebug(0,"S9355: [调用函数]_ufProSETVALUE 出错，%d",
        ilRc);
     return(FAIL);
  }
  
  aResult[0]=0;
  swVdebug(5,"S9355:[调用结束]_ufProSETVALUE()返回=0,结果=%s",aResult);
  return(0);
}




/**************************************************************
 ** 函 数 名：_ufProGETPROP()
 ** 功    能：取出指定域名属性的值
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2014/12/16 
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：aXpath:定位字符串
 **           aProname:属性名
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProGETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
   char alXpath[iFLDVALUELEN+1];
   char alProp[iFLDVALUELEN+1];
   short ilpathlen,ilproplen;
   int ilRc;
   short ilFlag = 0;
   
   if(iParmCount != 2)
   {
      swVdebug(0,"S8789: [函数调用]_ufProGETVALUE 错误, 参数错误");
      return(FAIL);
   }
   if(cgXmlInit == 0)
   {
     ilRc = swXmlinit();
     if (ilRc)
     {
       swVdebug(0,"S8792: GETVALUE() swXmlinit error");
       return(FAIL);
     }
     ilFlag = 1;
     swVdebug(5,"S8795: GETVALUE() swXmlinit success");
   }
   _ufTtoN(aParm[0],alXpath,&ilpathlen);
   _ufTtoN(aParm[1],alProp,&ilproplen);

   /* modify by nh 20040419 
   strcat(alXpath,"[@");
   strcat(alXpath,alProp);
   strcat(alXpath,"]");
   
   ilRc = xmlGetValue(alXpath,aResult);*/

   /*delete by zcd 20141218
   ilRc = xmlGetProp(alXpath,alProp,aResult);
   ***end of delete by zcd 20141218*/
   
   /*add by zcd 20141218*/
   ilRc = XMLGetProp(alXpath,alProp,aResult,0);
   /*end of add by zcd 20141218*/
   if(ilRc)
   {
      swDebug("S8789: [函数调用]_ufProGETPROP 错误, %d", ilRc);
      return(FAIL);
   }
   /*del by zcd 20141224
   if(ilFlag == 1) 
     swXmlfree();*/
   return(SUCCESS);
}



/**************************************************************
 ** 函 数 名：_ufProADDPROP()
 ** 功    能：打包时置XML域属性的值
 ** 作    者：cgx
 ** 建立日期：2003/11/18
 ** 修改日期：2014/12/16
 ** 调用函数：_swParesXpath(),_swWeaveField
 ** 全局变量：
 ** 参数含义：aXpath:XPATH定位字符串
 **           aName:新增属性名
 **           aValue:新增属性的值
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProADDPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc; 
  short ilLen;
  char alXpath[iFLDVALUELEN];
  char alProp[iFLDVALUELEN];
  char alValue[iFLDVALUELEN];
  char *pValue=NULL;

  if(iParmCount < 3)
  {
     swVdebug(0,"S9330:[调用函数]_ufProADDPROP 出错，参数错误");
     return(FAIL);
  }

  swVdebug(5,"S9330:[调用函数]_ufProADDPROP (%s,%s,%s)",
    aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],alXpath,&ilLen);
  _ufTtoN(aParm[1],alProp,&ilLen);
  if(iParmCount > 2)
  {
     _ufTtoN(aParm[2],alValue,&ilLen);
     if(!strcmp(alValue,"NULL") || !strcmp(alValue,"null"))
        alValue[0]=0x00;
     pValue = alValue;
  }

  /*delete by zcd 20141218
  ilRc = xmlAddProp(alXpath,alProp,pValue);
  ***end of delete by zcd 20141218*/
  
  
  /*add by zcd 20141218*/
  ilRc = XMLSetProp(alXpath,alProp,pValue,0);
  /*end of add by zcd 20141218*/
  if(ilRc)
  {
     swDebug("S9315: [函数调用]_ufProADDPROP() 出错, %d",
        ilRc);
     return(FAIL);
  }

  swVdebug(5,"S9315: [调用结束]_ufProADDPROP()返回=0,结果=%d",ilRc);
  return(0);
}



/**************************************************************
 ** 函 数 名：_ufProSETPROP()
 ** 功    能：打包置XML某域的属性值
 ** 作    者：cgx
 ** 建立日期：2003/11/24
 ** 修改日期：2014/12/16
 ** 调用函数：_swParesXpath()
 ** 全局变量：
 ** 参数含义：aXpath:定位字符串
 **           aProname:属性名
 **           aValue:属性值
 ** 返 回 值：0(成功) / -1(失败)
***************************************************************/
int _ufProSETPROP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc;
  short ilLen;
  char alXpath[iFLDVALUELEN];
  char alProp[iFLDVALUELEN];
  char alValue[iFLDVALUELEN];

  if(iParmCount < 3)
  {
     swVdebug(0,"S9330:[调用函数]_ufProSETPROP 出错，参数错误");
     return(FAIL);
  }

  swVdebug(5,"S9330:[调用函数]_ufProSETPROP (%s,%s,%s)",
    aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],alXpath,&ilLen); 
  _ufTtoN(aParm[1],alProp,&ilLen);
  _ufTtoN(aParm[2],alValue,&ilLen);

  /*delete by zcd 20141218
  ilRc = xmlSetProp(alXpath,alProp,alValue);
  ***end of add by  zcd 20141218 */
  
  /*add by zcd 20141218*/
  ilRc = XMLSetProp(alXpath,alProp,alValue,0);
  /*end of add by zcd 20141218*/
  if(ilRc)
  {
     swVdebug(0,"S9330:[调用函数]_ufProSETPROP 出错，%d",
       ilRc);
     return(FAIL);
  }
  aResult[0]='\0';
  swVdebug(5,"S9335:[调用结束]_ufProSETPROP()返回=0,结果=%s",aResult);
  return(0);
}


/**************************************************************
 ** 函数名      ： _ufProXMLDUMP
 ** 功  能      ： 打包时生成XML报文
 ** 作  者      ： nh
 ** 建立日期    ： 2002/8/20
 ** 最后修改日期： 2014/12/16
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLDUMP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilLen;
  short ilHeadlen;
  int  ilPos=0;
  int ilFldid = 2;
  int ilRc;
  char agXmlbuf[iMSGMAXLEN];
  char alHead[iMSGMAXLEN]; /*for 报文头 */
  
  
  swVdebug(5,"S8912: [函数调用]_ufProXMLDUMP()");

  /*delete by zcd 20141218
  ilRc = xmlDump(agXmlbuf,&ilLen);
  ***end of delete by zcd 20141218*/

  /*add by zcd 20141218*/
  ilLen = XMLDumpToBuf(agXmlbuf,1);
  if( ilLen <=0 )
  {
      swVdebug(0,"S8913: [函数调用]_ufProXMLDUMP 出错, 打包长度 %d",
         ilLen);
      return(FAIL);
  }
  /*end of add by zcd 20141218*/
  swVdebug(5,"S8914: [函数调用]_ufProXMLDUMP [%d][%s]",ilLen,agXmlbuf);
  if(iParmCount == 1)
  {	
  	_ufTtoN(aParm[0],alHead,&ilHeadlen); 
    strcat(alHead,agXmlbuf);
    ilLen += ilHeadlen;
  }
  else
  	strcpy(alHead,agXmlbuf);
  while(1)
  {
    if (ilLen <= iPERFLDVALUELEN)
    {
      psgUnpackbuf[ilFldid-1].sIsobuf.iFldid = ilFldid;
      psgUnpackbuf[ilFldid-1].sIsobuf.iFldlen = ilLen;
      memcpy(psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue,alHead+ilPos,ilLen);
      psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue[ilLen] = 0;
      break;
    }
    psgUnpackbuf[ilFldid-1].sIsobuf.iFldid = ilFldid;
    psgUnpackbuf[ilFldid-1].sIsobuf.iFldlen = iPERFLDVALUELEN;
    memcpy(psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue,alHead+ilPos,
       iPERFLDVALUELEN);
    psgUnpackbuf[ilFldid-1].sIsobuf.aFldvalue[iPERFLDVALUELEN] = '\0';
    ilPos = ilPos + iPERFLDVALUELEN;
    ilLen = ilLen - iPERFLDVALUELEN;
    ilFldid ++;
  }
  psgUnpackbuf[ilFldid].sIsobuf.iFldid = -1;
  aResult[0]=0;
  swVdebug(5,"S8912: [函数返回] _ufProXMLDUMP()返回码=0");
  return(0);
}



/**************************************************************
 ** 函数名      ： _ufProXMLINIT()
 ** 功  能      ： 打包时生成XML报文
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2014/12/18
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] :根节点名
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLINIT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc;
  short ilLen;
  char alBuf[iFLDVALUELEN],alBuf1[iFLDVALUELEN];
  char alTmp[10];

  swVdebug(5,"S8771: [函数调用]_ufProXMLINIT");
  if(iParmCount == 2)
  {
    _ufTtoN(aParm[1],alBuf,&ilLen);
	_ufTtoN(aParm[0],alBuf1,&ilLen);
	/*delete by zcd 20141218
	xmlpackInit("1.0",alBuf);
	***end of delete by zcd 20141218*/

	/*add by zcd 20141218*/
 	strcpy(alTmp,"");
    ilRc =XMLInit("1.0",alBuf,alBuf1);
    if(ilRc)
    {
      if(ilRc == 3)  /*已经初始化, 强行释放*/
      {
        XMLFree();
        cgXmlInit = 0;
        ilRc =XMLInit("1.0",alBuf,alBuf1);
        if(ilRc) return(FAIL);
      }
      else
        return(FAIL);
    }   
    /*end of add by zcd 20141218*/
  }
  else if(iParmCount == 1)
  {  	
    _ufTtoN(aParm[0],alBuf,&ilLen); /* 有报文头，需要偏移 nh 20060504 */

    
    ilLen = atoi(alBuf);
    
    /*delete by zcd 20141218
    ilRc = xmlunpackInit(agMsgbody+ilLen,igMsgbodylen-ilLen); 
    ***end of add by zcd 20141218*/

      
    /*add by zcd 20141218*/
    ilRc = XMLInitFromBuf(agMsgbody+ilLen);
    /*end of add by zcd 20141218*/
    if(ilRc)
    {
      if(ilRc == 3)  /*已经初始化, 强行释放*/
      {
        XMLFree();
        /* add by nh 20040607 */
        cgXmlInit = 0;
        /* end add */
        /*delete by zcd 20141218
    	ilRc = xmlunpackInit(agMsgbody+ilLen,igMsgbodylen-ilLen); 
    	***end of delete by zcd 20141218*/ 

    	/*add by zcd 20141218*/  
    	ilRc = XMLInitFromBuf(agMsgbody+ilLen);
    	/*end of add by zcd 20141218*/
        if(ilRc) 
        {
          swVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %d",
            ilRc);
          return(FAIL);
        }
      }
      else
        return(FAIL);
    }
    cgXmlInit = 1;
  }
  else
  { 	
    /*delete by zcd 20141218
    ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);  
    ***end of delete by zcd 20141218*/
    
    /*add by zcd 20141218*/  
    ilRc = XMLInitFromBuf(agMsgbody);	 
    /*end of add by zcd 20141218*/
    if(ilRc)
    {
      if(ilRc == 3)  /*已经初始化, 强行释放*/
      {
        XMLFree();
        /* add by nh 20040607 */
        cgXmlInit = 0;
        /* end add */

        /*add by zcd 20141218    
        ilRc = xmlunpackInit(agMsgbody,igMsgbodylen); 
        ***end of add by zcd 20141218*/ 
        /*add by zcd 20141218*/    
   		 ilRc = XMLInitFromBuf(agMsgbody);	
   		/*end of add by zcd 20141218*/  
        if(ilRc) 
        {
          swVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %d",
           ilRc);
          return(FAIL);
        }
      }
      else
        return(FAIL);
      cgXmlInit = 1;
    }
  }
  aResult[0]=0;
  swVdebug(5,"S9345: [调用结束]_ufProXMLINIT()返回=0,结果=%s",aResult);
  return(SUCCESS);
}


/**************************************************************
 ** 函数名      ： _ufProXMLFREE()
 ** 功  能      ： 释放XML资源
 ** 作  者      ： cgx
 ** 建立日期    ： 2002/8/28
 ** 最后修改日期： 2004/1/8
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： 
 ** 返回值      ： 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLFREE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  swVdebug(5,"S8783: [函数调用] _ufProXMLFREE");
  XMLFree();
  aResult[0] = 0x00;
  cgXmlInit = 0;
  return(SUCCESS);
}
#endif
/*end of add by zcd 20141218*/

/**************************************************************
 ** 函 数 名：_ufProUPPER
 ** 功    能：将小写字母转化为大写字母
 ** 作    者：qiaozg
 ** 建立日期：2007-8-2 
 ** 修改日期：
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：
 ** 　　　　　
 **           
 ** 返 回 值：SUCCESS(成功) / -1(失败)
***************************************************************/
int _ufProUPPER(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int i=0;
	char alTmpstr[iFLDVALUELEN];
  short ilStrlen;

  swVdebug(3,"S8785: [函数调用] _ufProUPPER");
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);
  for(i=0;i<ilStrlen;i++)
  {
  	if( (alTmpstr[i]>='a') && (alTmpstr[i]<='z')) alTmpstr[i]-=32;
  }
  _ufNtoT(alTmpstr,ilStrlen,aResult);
  swVdebug(3,"S8787: [函数返回] _ufProUPPER()返回码=0,结果=%s",aResult);
  return (SUCCESS);
}

/**************************************************************
 ** 函 数 名：_ufProLOWER
 ** 功    能：将小写字母转化为大写字母
 ** 作    者：qiaozg
 ** 建立日期：2007-8-2 
 ** 修改日期：
 ** 调用函数：
 ** 全局变量：
 ** 参数含义：
 ** 　　　　　
 **           
 ** 返 回 值：SUCCESS(成功) / -1(失败)
***************************************************************/
int _ufProLOWER(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	int i=0;
	char alTmpstr[iFLDVALUELEN];
  short ilStrlen;

  swVdebug(3,"S8789: [函数调用] _ufProLOWER");
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);
  for(i=0;i<ilStrlen;i++)
  {
  	if( (alTmpstr[i]>='A') && (alTmpstr[i]<='Z')) alTmpstr[i]+=32;
  }
  _ufNtoT(alTmpstr,ilStrlen,aResult);
  swVdebug(3,"S8791: [函数返回] _ufProLOWER()返回码=0,结果=%s",aResult);
  return (SUCCESS);
}





/******************************************************************************************


校验位生成算法C程序：



     *函数功能: 生成邮政金融客户信息平台客户编号校验位

     *输入参数: const char *szCustId  客户编号前13位

     *输出参数: 无

     *返 回 值: 客户编号校验位

*******************************************************************************************/


char _ufProCusChkbit(const char *szCustId)
{
    int iLen = 13;

    int i = 0;

    int tem = 10;

    int iNum;

    //以下算法为生成客户号校验位生成算法

    iLen = strlen(szCustId);

    if (iLen > 13) iLen = 13;

    for ( i = 0; i < iLen ; i++ )

    {

      iNum = szCustId[i] - '0';

      tem += iNum;

      tem %= 10;

      if ( tem == 0 ) tem = 10;

      tem *= 2;

      tem %= 11;

    }

    if ( tem != 1 )

    {

      tem = 11 - tem;

    }

    else

    {

      tem = 0;

    }

    return (tem + '0');

}



/**************************************************************
 ** 函数名      ： _ufProCUSTONEW
 ** 功  能      ： 取子串 
 ** 作  者      ： 张辉 
 ** 建立日期    ： 1999/11/18
 ** 最后修改日期： 1999/12/2
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ： aParm[0] : 源数据串
                  
 ** 返回值      ： 所求的子串
***************************************************************/
int _ufProCUSTONEW(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char setbuf[14 + 1];
  char alTmp[iFLDVALUELEN];
  
  short i, j;
  short ilLen=0;
  
  int  ilRtncode;
    
    
  swVdebug(3,"S8150: [函数调用] _ufProCUSTONEW(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
  memset(alTmp,0x00,sizeof(alTmp));
  memset(setbuf, 0x00, sizeof(setbuf));
  

  ilRtncode=swFmlget(psgPreunpackbuf,aParm[0],&ilLen,alTmp);
  
  if (ilRtncode==FAIL)
  {
    swVdebug(0,"S8045: [错误/函数调用] swFmlget()函数,返回码=%d",ilRtncode);
    return(-1);
  }
  swVdebug(3,"S8160: [函数调用] _ufProCUSTONEW() altmp= [%s]",  alTmp);

    j = 0;
	  for (i =0; i < 13; i++)
	  {
	  	  
	  	  if (i > 0 && i < 5)
	  	  {
	  	  	  setbuf[i] = '0';
	  	  }
	  	  else
	  	  {
	  	  	 /* setbuf[i] = (*alTmp)++;*/
	  	  	  setbuf[i] = alTmp[j];
	  	  	  
	  	  		j++;  
	  	  }
	  	  swVdebug(3,"S8160: [函数调用] _ufProCUSTONEW() setbuf= [%s]",  setbuf);
	  	  
	   }
	   
	   setbuf[13] = _ufProCusChkbit(setbuf);
	   memset(aResult, 0x00, sizeof(aResult));
	   strcpy(aResult, setbuf);

  
  

  swVdebug(3,"S8153: [函数返回] _ufProCUSTONEW()返回码=0,结果=%s",aResult);
  return(0);
}






