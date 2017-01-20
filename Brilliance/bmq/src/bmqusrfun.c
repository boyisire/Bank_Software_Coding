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
/*       int  _ufProLEN();                                      */
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
#include "bmq.h"

int _swTrim();
int _swTimeLongToChs();
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
int _swExpress(char *aExpress, char *aResult, short *piResultlen)
/*int _swExpress(char *aExpress, char *aResult, TYPE_SWITCH *piResultlen) add by wh*/
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
            _bmqVdebug(0,"S8000: [错误/其它] 平台函数{%s}返回值出错",alFunc);  
            return(-1);
          }
        }
        else if(proclist[ilCnt].fproc_old!=NULL)
        {
          strcpy(alResult,(*(proclist[ilCnt].fproc_old))(palParm));
        }
        else
        {
          _bmqVdebug(0,"S8003: [错误/其它] 平台函数{%s}在函数列表中没有定义",alFunc);
          return(-1);
        }  
      }
      else
      {
          _bmqVdebug(0,"S8012: [错误/其它] 平台函数{%s}在函数列表中没有定义",alFunc);
          return(-1);
      }
      
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
    _bmqVdebug(0,"S8990: [错误/平台函数]: _swParseN2T(),表达式[%s]不合法!",aStr);
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
    _bmqVdebug(0,"S8015: [错误/平台函数{%s}]: _swGetFuncAndParmN2T()",aStr);
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
            _bmqVdebug(0,"S8018: [错误/平台函数]: _swGetFuncAndParmN2T() 参数个数超限!"); 
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
    _bmqVdebug(0,"S8021: [错误/平台函数]: _swGetFuncAndParmN2T()");
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
    _bmqVdebug(0,"S8024: [错误/平台函数]: _swGetFuncAndParmN2T()");
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
    _bmqVdebug(0,"S8027: [错误/平台函数]: _swGetFuncAndParmN2T()");
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
  PROCLIST slProclist;

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
  /*short i,j = 0; delete by wh*/
  TYPE_SWITCH i,j = 0; /*add by wh*/

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
/*int _ufTtoN(char *aTstr, char *aNstr, TYPE_SWITCH *piLen) add by wh*/
{
  short i=0,j=0;

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

  return 0;
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
  _bmqVdebug(3,"S8648: [函数调用] _ufProNSP(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0],alTmp,&ilLen);
  ilOffset = atoi(alTmp) - 1;
  if (ilOffset<0) ilOffset=0;

  _ufTtoN(aParm[1],alTmp,&ilLen);
  ilLen = atoi(alTmp);
  /* add by zjj 2004.03.15 */
  if (ilLen >= iFLDVALUELEN)
  {
    _bmqVdebug(0,"S8649: [错误/其他] _ufPropNSP,取值长度[%d]超过域值最大长度[%d]",ilLen,iFLDVALUELEN);
    return(FAIL);
  }
  /*end add by zjj */

/* delete by szhengye 2001.8.30
  if ((ilOffset+ilLen)>igMsgbodylen)
    ilLen=igMsgbodylen - ilOffset;
  if (ilLen<0) ilLen=0;
*/

  _ufNtoT(agMsgbody+ilOffset,ilLen,aResult);


  _bmqVdebug(3,"S8054: [函数返回] _ufProNSP()返回码=0,结果=%s",aResult);
  return(0);

}



unsigned char _ufStr2Char(a)
  char *a;
{
  short j;
  unsigned char b;
  
  _bmqVdebug(3,"S8084: [函数调用] _ufStr2Char(%s)",a);
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

  _bmqVdebug(3,"S8087: [函数返回] _ufStr2Char()返回码=%d",b);
  return(b);
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

  _bmqVdebug(3,"S8150: [函数调用] _ufProMID(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  ilPos=atoi(aParm[1]);
  ilLen=atoi(aParm[2]);
  _ufTtoN(aParm[0],aResult,&ilStrlen);

  if (ilStrlen<(ilPos+ilLen-1))
     ilLen = ilStrlen - ilPos;

  for(i=0;i<ilLen;i++)
     alTmpstr[i]=aResult[ilPos+i-1];
  alTmpstr[i] = '\0';

  _ufNtoT(alTmpstr,i,aResult);

  _bmqVdebug(3,"S8153: [函数返回] _ufProMID()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8156: [函数调用] _ufProLFT(%s,%s)",aParm[0],aParm[1]);
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);

  ilLen=atoi(aParm[1]);
  if(ilStrlen<ilLen) ilLen=ilStrlen;

  _ufNtoT(alTmpstr,ilLen,aResult);

  _bmqVdebug(3,"S8159: [函数返回] _ufProLFT()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8162: [函数调用] _ufProRIG(%s,%s)",aParm[0],aParm[1]);
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);

  ilLen=atoi(aParm[1]);
  if(ilStrlen<ilLen) ilLen=ilStrlen;

  _ufNtoT(alTmpstr+ilStrlen-ilLen,ilLen,aResult);
  _bmqVdebug(3,"S8165: [函数返回] _ufProRIG()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8168: [函数调用] _ufProFILL(%s,%s)",aParm[0],aParm[1]);

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

  _bmqVdebug(3,"S8171: [函数返回] _ufProFILL()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8174: [函数调用] _ufProSAD(%s,%s)",aParm[0],aParm[1]);

  j=0;
  for(i=0;i<iParmCount;i++)
  {
    /* if(aParm[i][0]=='\0') break; */
    _ufTtoN(aParm[i],alTmp+j,&ilStrlen);
    j = j + ilStrlen;
  }
  alTmp[j]='\0';
  
  _ufNtoT(alTmp,j,aResult);

  _bmqVdebug(3,"S8177: [函数返回] _ufProSAD()返回码=0,结果=%s",aResult);
  
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
  
  _bmqVdebug(3,"S8180: [函数调用] _ufProLEN(%s)",aParm[0]);
      
  _ufTtoN(aParm[0],aResult,&ilStrlen);

  sprintf(aResult,"%d",ilStrlen);

  _bmqVdebug(3,"S8183: [函数返回] _ufProLEN()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8186: [函数调用] _ufProTRIM(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrim(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8189: [函数返回] _ufProTRIM()返回码=0,结果=%s",aResult);
  return 0;
}

/**************************************************************
 ** 函数名      : _swTrim
 ** 功  能      : 该函数将字符串 s 的前后空格及尾回车去掉
 ** 作  者      : llx
 ** 建立日期    : 1999/11/4
 ** 最后修改日期: 2000/3/10
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : s   :字符串
 ** 返回值      : SUCCESS
		  转换后的字符串从s返回
***************************************************************/
int _swTrim(s)
char *s;
{
  /*short	i, l, r, len; delete by wh*/
  TYPE_SWITCH i, l, r, len; /*add by wh*/

  for(len=0; s[len]; len++);
  for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
  if(l==len)
  {
    s[0]='\0';
    return(SUCCESS);
  }
  for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
  for(i=l; i<=r; i++) s[i-l]=s[i];
  s[r-l+1]='\0';
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : _swTrimL
 ** 功  能      : 该函数将字符串 s 的前空格及尾回车去掉
 ** 作  者      : lijunwen
 ** 建立日期    : 2000/3/29
 ** 最后修改日期: 2000/3/29
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : s   :字符串
 ** 返回值      : SUCCESS
		  转换后的字符串从s返回
***************************************************************/
int _swTrimL(s)
char *s;
{
  /*short	i, l, r, len; delete by wh*/
  TYPE_SWITCH i, l, r, len; /*add by wh*/

  for(len=0; s[len]; len++);
  for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
  if(l==len)
  {
    s[0]='\0';
    return(SUCCESS);
  }
  r = len -1;
  /*for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);*/
  for(i=l; i<=r; i++) s[i-l]=s[i];
  s[r-l+1]='\0';
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      : _swTrimR
 ** 功  能      : 该函数将字符串 s 的后空格及尾回车去掉
 ** 作  者      : lijunwen
 ** 建立日期    : 2000/3/29
 ** 最后修改日期: 2000/3/29
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : s   :字符串
 ** 返回值      : SUCCESS
		  转换后的字符串从s返回
***************************************************************/
int _swTrimR(s)
char *s;
{
  /*short	i, l, r, len; delete by wh*/
  TYPE_SWITCH i, l, r, len; /*add by wh*/

  for(len=0; s[len]; len++);
/*
  for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
  if(l==len) 
  { 
    s[0]='\0'; 
    return(SUCCESS); 
  }
*/
  l = 0;
  for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
  for(i=l; i<=r; i++) s[i-l]=s[i];
  s[r-l+1]='\0';

  return(SUCCESS);
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

  _bmqVdebug(3,"S8192: [函数调用] _ufProTRIML(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrimL(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8195: [函数返回] _ufProTRIML()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8198: [函数调用] _ufProTRIMR(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrimR(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8201: [函数返回] _ufProTRIMR()返回码=0,结果=%s",aResult);
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
  /*short ilStrlen; delete by wh*/
  TYPE_SWITCH ilStrlen; /*add by wh*/
 
  _bmqVdebug(3,"S8204: [函数调用] _ufProSPACE(%s)",aParm[0]);

  ilStrlen=atoi((char *)aParm[0]);

  memset(aResult, ' ', ilStrlen);
  aResult[ilStrlen] = 0;

  _bmqVdebug(3,"S8207: [函数返回] _ufProSPACE()返回码=0,结果=%s",aResult);
  return 0;
}

/* ============ add by dgm 2001/10/12 ========== */
int _ufProADD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	/*
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;

  _bmqVdebug(3,"S8225: [函数调用] _ufProADD(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
   
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
    _bmqVdebug(3,"S8228: [函数返回] _ufProADD()返回码=0,结果=%s",aResult);
    return 0;
  }
  if (ilNum == 0) ilNum = -1;
  *(p1 + ilNum+1) = '\0';
   
  _bmqVdebug(3,"S8231: [函数返回] _ufProADD()返回码=0,结果=%s",aResult);
*/
  return 0;
}
/* ========= end of add by dgm 2001/10/12 ====== */

/* ============ add by dgm 2001/10/12 ========== */
int _ufProSUB(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	/*
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;
  
  
  _bmqVdebug(3,"S8240: [函数调用] _ufProSUB(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
   
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
    _bmqVdebug(3,"S8243: [函数返回] _ufProSUB()返回码=0,结果=%s",aResult);
    return 0;
  }
 if (ilNum == 0) ilNum = -1;
 *(p1 + ilNum+1) = '\0';
   
  _bmqVdebug(3,"S8246: [函数返回] _ufProSUB()返回码=0,结果=%s",aResult);
*/
  return 0;
}
int _ufProMULTI(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
/*
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;
  
  _bmqVdebug(3,"S8255: [函数调用] _ufProMULTI(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

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
    _bmqVdebug(3,"S8258: [函数返回] _ufProMULTI()返回码=0,结果=%s",aResult);
    return 0;
  }
  if (ilNum == 0) ilNum = -1;
    *(p1 + ilNum+1) = '\0';
*/
  _bmqVdebug(3,"S8261: [函数返回] _ufProMULTI()返回码=0,结果=%s",aResult);
  return 0;
}
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

  _bmqVdebug(3,"S8264: [函数调用] _ufProDIV(%s,%s)",aParm[0],aParm[1]);

  llRt=0;
  llNum1=atol(aParm[0]);
  llNum2=atol(aParm[1]);
  if (llNum2!=0)
  {
    slResult=ldiv(llNum1,llNum2);
    llRt=slResult.quot;
  }
  sprintf(aResult,"%ld",llRt);

  _bmqVdebug(3,"S8267: [函数返回] _ufProDIV()返回码=0,结果=%s",aResult);
  return 0;
}


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

  _bmqVdebug(3,"S8270: [函数调用] _ufProMOD(%s,%s)",aParm[0],aParm[1]);

  llRt=0;
  llNum1=atol(aParm[0]);
  llNum2=atol(aParm[1]);
  if (llNum2!=0)
  {
    slResult=ldiv(llNum1,llNum2);
    llRt=slResult.rem;
  }
  sprintf(aResult,"%ld",llRt);

  _bmqVdebug(3,"S8273: [函数返回] _ufProMOD()返回码=0,结果=%s",aResult);
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
  _bmqVdebug(3,"S8276: [函数调用] _ufProSEQ(%s,%s)",aParm[0],aParm[1]);

  if (!strcmp(aParm[0],aParm[1]))
     strcpy(aResult,"1");
  else
     strcpy(aResult,"0");

  _bmqVdebug(3,"S8279: [函数返回] _ufProSEQ()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8282: [函数调用] _ufProGT(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a>b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8285: [函数返回] _ufProGT()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8288: [函数调用] _ufProGE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a>=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8291: [函数返回] _ufProGE()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8294: [函数调用] _ufProLT(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a<b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8297: [函数返回] _ufProLT()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8300: [函数调用] _ufProLE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a<=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8303: [函数返回] _ufProLE()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8306: [函数调用] _ufProNE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a!=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8309: [函数返回] _ufProNE()返回码=0,结果=%s",aResult);
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
  _bmqVdebug(3,"S8318: [函数调用] _ufProNOT(%s)",aParm[0]);

  if ( aParm[0][0]=='0')
     strcpy(aResult,"1");
  else
     strcpy(aResult,"0");

  _bmqVdebug(3,"S8321: [函数返回] _ufProNOT()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8324: [函数调用] _ufProAND(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    _bmqVdebug(3,"S8327:                     ,%s",aParm[i]);

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

  _bmqVdebug(3,"S8330: [函数返回] _ufProAND()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8333: [函数调用] _ufProOR(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    _bmqVdebug(3,"S8336:                    ,%s",aParm[i]);
 
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

  _bmqVdebug(3,"S8339: [函数返回] _ufProOR()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8342: [函数调用] _ufProIF(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);

  if (((*aResult)!='0')&&((*aResult)!='\0')) 
    strcpy(aResult,aParm[1]);
  else 
    strcpy(aResult,aParm[2]);

  _bmqVdebug(3,"S8345: [函数返回] _ufProIF()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8348: [函数调用] _ufProCASEIF(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    _bmqVdebug(3,"S8351:                        ,%s",aParm[i]);

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

  _bmqVdebug(3,"S8354: [函数返回] _ufProCASEIF()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8357: [函数调用] _ufProCASEVAR(%s)",aParm[0]);
  for(i=1;i<iParmCount;i++)
    _bmqVdebug(3,"S8360:                         ,%s",aParm[i]);

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

  _bmqVdebug(3,"S8363: [函数返回] _ufProCASEVAR()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8366: [函数调用] _ufProCHR(%s)",aParm[0]);
  
  iltmp=atoi(aParm[0]);
  if(iltmp<0||iltmp>255) return(-1);
  sprintf(aResult,"%c",iltmp);

  _bmqVdebug(3,"S8369: [函数返回] _ufProCHR()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8372: [函数调用] _ufProSHORT(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  sltmp=atoi(aResult);
  memcpy(alTmp,&sltmp,2);

  alTmp[2]='\0';
  _ufNtoT(alTmp,2,aResult);

  _bmqVdebug(3,"S8375: [函数返回] _ufProSHORT()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8378: [函数调用] _ufProINT(%s)",aParm[0]);
 
  _ufTtoN(aParm[0],aResult,&ilLen);
  iltmp=atoi(aResult);
  memcpy(alTmp,&iltmp,4);

  alTmp[4]='\0';
  _ufNtoT(alTmp,4,aResult);

  _bmqVdebug(3,"S8381: [函数返回] _ufProINT()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8384: [函数调用] _ufProLONG(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  lltmp=atol(aResult);
  memcpy(alTmp,&lltmp,4);

  alTmp[4]='\0';
  _ufNtoT(alTmp,4,aResult);

  _bmqVdebug(3,"S8387: [函数返回] _ufProLONG()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8390: [函数调用] _ufProDOUBLE(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  dltmp=atof(aResult)/100;

  memcpy(alTmp,&dltmp,8);

  alTmp[8]='\0';
  _ufNtoT(alTmp,8,aResult);

  _bmqVdebug(3,"S8393: [函数返回] _ufProDOUBLE()返回码=0,结果=%s",aResult);
  return 0;
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

  _bmqVdebug(3,"S8402: [函数调用] _ufProHEX(%s)",aParm[0]);

  iltmp=_ufStr2Char(aParm[0]);
  sprintf(alResult,"%c",iltmp);

  _ufNtoT(alResult,sizeof(char),aResult);

  _bmqVdebug(3,"S8405: [函数返回] _ufProHEX()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8408: [函数调用] _ufProUSHORT(%s)",aParm[0]);
 
  _ufTtoN(aParm[0],alOrig,&ilLen);
  alOrig[ilLen]='\0';
  pilNum = (unsigned short *)(alOrig);
  sprintf(aResult,"%d",*pilNum);

  _bmqVdebug(3,"S8411: [函数返回] _ufProUSHORT()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8414: [函数调用] _ufProULONG(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilLen);
  alTmp[ilLen] = '\0';
  pllValue=(unsigned long*)alTmp;
  sprintf(aResult,"%ld",*pllValue);

  _bmqVdebug(3,"S8417: [函数返回] _ufProULONG()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8420: [函数调用] _ufProGETTIME(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  time(&llCurrentTime);
  _swTimeLongToChs(llCurrentTime,alTmp,aResult);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8423: [函数返回] _ufProGETTIME()返回码=0,结果=%s",aResult);
  return(0);
}
/*******************************************************************/
/* 函 数 名       :  _swTimeLongToChs                               */
/* 功    能       :  将长整型的时间转换为字符串型                  */
/* 参数含义       :  iDateTime 长整型的时间                        */
/*                   pcFormat 时间的格式                           */
/* 返 回 值       :  pcDateTime 字符串型的时间                     */
/*******************************************************************/
int _swTimeLongToChs( lDateTime , pcDateTime , pcFormat )
long    lDateTime;
char   *pcDateTime;
char   *pcFormat;
{
  struct tm  *sttm;

  memset( pcDateTime, 0x00, sizeof( pcDateTime ) );

  sttm = localtime( &lDateTime );

  if( strcmp( pcFormat, "YYMMDD" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%02d%02d%02d",
/* ======== delete by dgm 2001/10/15 ========== 
		    sttm->tm_year,
===== end of delete by dgm 2001/10/15 ====== */

/* ======== add by dgm 2001/10/15 ========== */
		    sttm->tm_year > 99 ? sttm->tm_year - 100 : sttm->tm_year,
/* ===== end of add by dgm 2001/10/15 ====== */
		    sttm->tm_mon+1,
		    sttm->tm_mday);
    }
    else
    {
      strcpy(pcDateTime,"------");
    }
  }
  else if ( strcmp( pcFormat, "YYYYMMDD" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%04d%02d%02d",
		    sttm->tm_year+1900,
		    sttm->tm_mon+1,
		    sttm->tm_mday);
    }
    else
    {
      strcpy(pcDateTime,"--------");
    }
  }
  else if ( strcmp( pcFormat, "YYYYMMDDHHMMSS" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%04d%02d%02d%02d%02d%02d",
		    sttm->tm_year+1900,
		    sttm->tm_mon+1,
		    sttm->tm_mday,
		    sttm->tm_hour,
		    sttm->tm_min,
		    sttm->tm_sec );
    }
    else
    {
      strcpy(pcDateTime,"--------------");
    }
  }
  else if ( strcmp( pcFormat, "YYYY-MM-DD HH:MM:SS" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%04d-%02d-%02d %02d:%02d:%02d",
		    sttm->tm_year+1900,
		    sttm->tm_mon+1,
		    sttm->tm_mday,
		    sttm->tm_hour,
		    sttm->tm_min,
		    sttm->tm_sec );
    }
    else
    {  
      strcpy(pcDateTime,"----/--/-- --:--:--");
    }
  }
  else if ( strcmp( pcFormat, "HHMMSS" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%02d%02d%02d",
		    sttm->tm_hour,
		    sttm->tm_min,
		    sttm->tm_sec );
    }
    else
    {
      strcpy(pcDateTime,"------");
    }
  }
  else if ( strcmp( pcFormat, "HH:MM:SS" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%02d:%02d:%02d",
		    sttm->tm_hour,
		    sttm->tm_min,
		    sttm->tm_sec );
    }
    else
    {
      strcpy(pcDateTime,"--:--:--");
    }
  }
  else if ( strcmp( pcFormat, "MMDD" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%02d%02d",
		    sttm->tm_mon+1,
		    sttm->tm_mday);
    }
    else
    {
      strcpy(pcDateTime,"----");
    }
  }
  else if ( strcmp( pcFormat, "MMDDHHMMSS" ) == 0 )
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%02d%02d%02d%02d%02d",
		    sttm->tm_mon+1,
		    sttm->tm_mday,
		    sttm->tm_hour,
		    sttm->tm_min,
		    sttm->tm_sec );
    }
    else
    {
      strcpy(pcDateTime,"----------");
    }
  }
  else
  {
    if (lDateTime)
    {
      sprintf( pcDateTime,"%04d-%02d-%02d %02d:%02d:%02d",
		    sttm->tm_year+1900,
		    sttm->tm_mon+1,
		    sttm->tm_mday,
		    sttm->tm_hour,
		    sttm->tm_min,
		    sttm->tm_sec );
    }
    else
    {
      strcpy(pcDateTime,"----/--/-- --:--:--");
    }
  }
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

  _bmqVdebug(3,"S8426: [函数调用] _ufProTODATE(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  llDatetime = atol(aResult);
  tmT = localtime(&llDatetime);

  sprintf(aResult,"%04d%02d%02d",tmT->tm_year+1900,tmT->tm_mon+1,tmT->tm_mday);
  aResult[8] = '\0';
  
  _bmqVdebug(3,"S8429: [函数返回] _ufProTODATE()返回码=0,结果=%s",aResult);
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
 
  _bmqVdebug(3,"S8432: [函数调用] _ufProTOTIME(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  llDatetime = atol(aResult);
  tmT = localtime(&llDatetime);
  
  sprintf(aResult,"%02d:%02d:%02d", tmT->tm_hour, tmT->tm_min, tmT->tm_sec);
/* ============ delete by dgm 2001/10/15 ===========
  aResult[6] = '\0';
========= end of delete by dgm 2001/10/15 ======== */
  
  _bmqVdebug(3,"S8435: [函数返回] _ufProTOTIME()返回码=0,结果=%s",aResult);
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
  
  _bmqVdebug(3,"S8438: [函数调用] _ufProPID()");

  llPid=getpid();
  sprintf(aResult,"%ld",llPid);

  _bmqVdebug(3,"S8441: [函数返回] _ufProPID()返回码=0,结果=%s",aResult);
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

  _bmqVdebug(3,"S8453: [函数调用] _ufProDATEtoLONG(%s)",aParm[0]);
   
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
  _bmqVdebug(4,"S8456: llDate=[%u]",llDatetime);
  _ufNtoT((char*)&llDatetime,sizeof(long),aResult);
  
  _bmqVdebug(3,"S8459: [函数返回] _ufProDATEtoLONG()返回码=0,结果=%s",aResult);
  return(0);
}



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
 
  _bmqVdebug(3,"S8474: [函数调用] _ufProLONGTODATE(%s)",aParm[0]);

  llDatetime = atol(aParm[0]);
  
  tmT = localtime( &llDatetime );
  sprintf(aResult,"%04d-%02d-%02d",tmT->tm_year+1900,tmT->tm_mon+1,tmT->tm_mday);

  _bmqVdebug(3,"S8477: [函数返回] _ufProLONGTODATE()返回码=0,结果=%s",aResult);
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
 
  _bmqVdebug(3,"S8480: [函数调用] _ufProLONGTOTIME(%s)",aParm[0]);

  llDatetime = atol(aParm[0]);
  tmT = localtime( &llDatetime );
  
  sprintf(aResult,"%02d:%02d:%02d", tmT->tm_hour, tmT->tm_min, tmT->tm_sec);
  
  _bmqVdebug(3,"S8483: [函数返回] _ufProLONGTOTIME()返回码=0,结果=%s",aResult);
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
  short /*ilMsglen,*/ilFldlen; /*delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char  alFldname[iFLDNAMELEN+1];
  char  alTmp[iFLDVALUELEN+1];
  
  _bmqVdebug(3,"S8486: [函数调用] _ufProFmlget(%s)",aParm[0]);

  strcpy(alFldname, aParm[0]);
  ilMsglen = sgMsgpack.sMsghead.iBodylen;
  ilRc = swFmlpackget(sgMsgpack.aMsgbody, ilMsglen, alFldname, &ilFldlen, alTmp);
  if (ilRc) 
  {
    _bmqVdebug(0,"S8489: [错误/函数调用] swFmlpackget()函数,返回码=%d",ilRc);
    return(FAIL);
  }
  _ufNtoT(alTmp,ilFldlen,aResult);

  _bmqVdebug(3,"S8492: [函数返回] _ufProFmlget()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/**************************************************************
 ** 函数名: swFmlpackget
 ** 功  能: 得到FML报文中指定域名的长度和域值
***************************************************************/
/*int swFmlpackget(char *aMsgbody, short iMsglen, char *aFldname, short *piFldlen, char *aFldval) delete by wh*/
int swFmlpackget(char *aMsgbody, TYPE_SWITCH iMsglen, char *aFldname, short *piFldlen, char *aFldval) /*add by wh*/
{
  /*short  ilPos=0,ilLen; delete by wh*/
  TYPE_SWITCH ilPos=0,ilLen;
  char * alFldname;
	
  while (ilPos < iMsglen)
  {
    alFldname = aMsgbody + ilPos;
    if (strlen(alFldname) > iFLDNAMELEN)
    {
      _bmqVdebug(0,"S6060: [错误/其它] FML报文域名过长[%s]",alFldname);
       return(FAIL);
    }
    if (strcmp(alFldname, aFldname) == 0)
    {
      ilPos = ilPos + strlen(alFldname) + 1;
      memcpy(piFldlen,aMsgbody + ilPos,iSHORTLEN);
      /* modify by nh 20040412
      *piFldlen = *(short *)(aMsgbody + ilPos); */
      ilPos = ilPos + sizeof(short); 
      memcpy(aFldval, aMsgbody + ilPos, *piFldlen);
      return(SUCCESS);
    }
    else
    {
      ilPos = ilPos + strlen(alFldname) + 1;
      memcpy(&ilLen,aMsgbody + ilPos,iSHORTLEN);
      /* modify by nh 20040412
      ilLen = *(short *)(aMsgbody + ilPos); */
      ilPos = ilPos + sizeof(short) + ilLen;
    }
  }
  return FAIL;
} 

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
  short ilRc, ilRtncode, ilRecid, ilGrpid, ilFldid,ilMsglen;
/* ========== add by dgm 2001/10/16 ============ */
  short ilLen; 
  char  alTmp[iFLDVALUELEN+1];
/* ======= end of add by dgm 2001/10/16 ======== */

  _bmqVdebug(3,"S8612: [函数调用] _ufProPackSEP(%s,%s,%s,%s,%s,%s)",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

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
    _bmqVdebug(0,"S8615: 调用swPacksep错误");
    _bmqVdebug(0,"S8618: _ufProPackSEP: alRecs = %s, alGrps = %s, alFlds = %s\n", \
            alRecs, alGrps, alFlds);
    _bmqVdebug(0,"S8621: _ufProPackSEP: SEP(%d,%d,%d)={%s}\n",ilRecid,ilGrpid,ilFldid,aResult);
    return(FAIL);
  }
========= end of delete by dgm 2001/10/16 ======= */
  
/* ========== add by dgm 2001/10/16 ============ */
  ilRc = swPacksep(sgMsgpack.aMsgbody, ilMsglen, alRecs, alGrps, alFlds, \
                   ilRecid, ilGrpid, ilFldid, alTmp);
  if (ilRc != 0)
  {
    _bmqVdebug(0,"S8624: [错误/函数调用] swPacksep()函数,返回码=%d",ilRc);
    return(FAIL);
  }

  ilLen = strlen(alTmp);
  _ufNtoT(alTmp,ilLen,aResult);
/* ======= end of add by dgm 2001/10/16 ======== */

  _bmqVdebug(3,"S8627: [函数返回] _ufProPackSEP()返回码=0,结果=%s",aResult);
  return(SUCCESS);
}
/**************************************************************
 ** 函数名      ：swPacksep
 ** 功  能      ：取有分隔符报文指定的域值
 ** 作  者      ：
 ** 建立日期    ：2001.9.7
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：0:成功 / -1:失败
***************************************************************/
/*short swPacksep(char *aMsgbody, short iMsglen, char *aRecs, char *aGrps, char *aFlds, short iRecid, short iGrpid, short iFldid, char *aFldval) delete by wh*/
short swPacksep(char *aMsgbody, TYPE_SWITCH iMsglen, char *aRecs, char *aGrps, char *aFlds, short iRecid, short iGrpid, short iFldid, char *aFldval)
{
  short i,j=0,/*ilPos,ilPosbegin=0,*/ilRs,ilGs,ilFs,ilRsid=0,ilGsid=0,ilFsid=0;
  /*short ilLen; delete by wh*/
  TYPE_SWITCH ilLen; /*add by wh*/
  TYPE_SWITCH ilPos, ilPosbegin=0;/*add by wh 20150107*/

  for (i = 0; i <= 5; i++)
  {
    if (aRecs[i] == '\0') break;
  }
  ilRs = i;
  
  for (i = 0; i <= 5; i++)
  {
    if (aGrps[i] == '\0') break;
  }
  ilGs = i;
  
  for (i = 0; i <= 5; i++)
  {
    if (aFlds[i] == '\0') break;
  }

  ilFs = i;
  ilPos = 0;

  while (ilPos < iMsglen)
  {
    if ((memcmp(aMsgbody + ilPos,aRecs,ilRs) == 0) && (ilRs > 0))
    {
      ilLen = ilRs;
      if ((ilRsid == iRecid) && (ilGsid == iGrpid) && (ilFsid == iFldid))
      {
        memset(aFldval, 0x00, iFLDVALUELEN +1);
        memcpy(aFldval, aMsgbody + ilPosbegin, ilPos - ilPosbegin);
        return(SUCCESS);
      }
      
      ilPos = ilPos + ilLen;
      ilPosbegin = ilPos;
      j ++;
      ilRsid ++;
      ilGsid = 0;
      ilFsid = 0;
    }
    else if ((memcmp(aMsgbody + ilPos,aGrps,ilGs) == 0) && (ilGs > 0))
    {
      ilLen = ilGs;
      if ((ilRsid == iRecid) && (ilGsid == iGrpid) && (ilFsid == iFldid))
      {
        memset(aFldval, 0x00, iFLDVALUELEN +1);
        memcpy(aFldval, aMsgbody + ilPosbegin, ilPos - ilPosbegin);
        return(SUCCESS);
      }
      
      ilPos = ilPos + ilLen;
      ilPosbegin = ilPos;
      j ++;
      ilGsid ++;
      ilFsid = 0;
    }
    else if ((memcmp(aMsgbody + ilPos,aFlds,ilFs) == 0) && (ilFs > 0))
    {
      ilLen = ilFs;
      if ((ilRsid == iRecid) && (ilGsid == iGrpid) && (ilFsid == iFldid))
      {
        memset(aFldval, 0x00, iFLDVALUELEN +1);
        memcpy(aFldval, aMsgbody + ilPosbegin, ilPos - ilPosbegin);
        return(SUCCESS);
      }
      
      ilPos = ilPos + ilLen;
      ilPosbegin = ilPos;
      j ++;
      ilFsid ++;
    }
    else ilPos ++;
  }

  if ((ilRsid == iRecid) && (ilGsid == iGrpid) && (ilFsid == iFldid))
  {
    memset(aFldval, 0x00, iFLDVALUELEN +1);
    memcpy(aFldval, aMsgbody + ilPosbegin, ilPos - ilPosbegin);
    return(SUCCESS);
  }
  
  return(FAIL);
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

  _bmqVdebug(3,"S8630: [函数调用] _ufPropGETSEP(%s,%s)",aParm[0],aParm[1]);

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
  
  _bmqVdebug(3,"S8633: [函数返回] _ufPropGETSEP()返回码=0,结果=%s",aResult);
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
  _bmqVdebug(3,"S8642: [函数调用] _ufPropGET()");
  sprintf(aResult,"%hd",lgMsgoffset);
  _bmqVdebug(3,"S8645: [函数返回] _ufPropGET()返回码=0,结果=%s",aResult);
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
  /*short ilLen; delete by wh*/
  TYPE_SWITCH ilLen; /*add by wh*/
  
  _bmqVdebug(3,"S8648: [函数调用] _ufPropNSP(%s)",aParm[0]);

  ilLen = atoi(aParm[0]);
  /* add by zjj 2004.03.15 */
  if (ilLen >= iFLDVALUELEN)
  {
    _bmqVdebug(3,"S8649: [错误/其他] _ufPropNSP,取值长度[%d]超过域值最大长度[%d]",ilLen,iFLDVALUELEN);
    return(FAIL);
  }
  /* end add by zjj */
  alPos = agMsgbody;
  alPos = alPos + lgMsgoffset;
  
  memcpy(alResult,alPos,ilLen);
  alResult[ilLen] = 0x00;
  _ufNtoT(alResult,ilLen,aResult);
  lgMsgoffset = lgMsgoffset + ilLen;

  _bmqVdebug(3,"S8651: [函数返回] _ufPropNSP()返回码=0,结果=%s",aResult);
  return(SUCCESS);
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
  
  _bmqVdebug(3,"S8663: [函数调用] _ufProBITGET(%s,%s)",aParm[0],aParm[1]);
  
  _ufTtoN(aParm[0], alStr, &ilRtncode);
  _ufTtoN(aParm[1], aResult, &ilRtncode);
  ilBit = atoi(aResult);
  
  iLen = strlen(alStr);
  if (ilBit > 8 * iLen || ilBit <= 0)
  {
    _bmqVdebug(0,"S8666: [错误/其它] 要取的位不存在！");
    return(FAIL);
  }
  
  clMask = clMask << (8 - ((ilBit - 1) % 8 + 1));
  clTmp = clMask;
  
  if (alStr[(ilBit - 1) / 8] & clTmp)
    strcpy(aResult, "1");
  else 
    strcpy(aResult, "0");
    
  _bmqVdebug(3,"S8669: [函数返回] _ufProBITGET()返回码=0,结果=%s",aResult);
  return(SUCCESS); 
   
}

#ifdef LIBXML

int swXmlinit()
{
  short ilRc;
  
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
        _bmqVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %s",
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

  _ufTtoN(aParm[0],alXpath,&ilLen);
  _bmqVdebug(3,"S8783: [函数调用] _ufProGETVALUE (%s)",alXpath);
  if(iParmCount == 0)
  {
     _bmqVdebug(0,"S8784: [函数调用] _ufProGETVALUE 错误, 参数为空");
     return(FAIL);
  }
  if(cgXmlInit == 0)
  {
    ilRc = swXmlinit();
    if (ilRc)
    {
      _bmqVdebug(0,"S8785: GETVALUE() swXmlinit error");
      return(FAIL);
    }
    ilFlag = 1;
    _bmqVdebug(1,"S8786: GETVALUE() swXmlinit success");
  }
  _bmqVdebug(1,"S8791: [函数调用]_ufProGETVALUE ,初始化成功");
  /* modify by nh 20040419 
  ilRc = xmlGetValue(alXpath,aResult);*/
  ilRc = xmlGetNode(alXpath,aResult,NULL);
  if(ilRc)
  {
     _bmqDebug("S8790: [函数调用] _ufProGETVALUE 错误, %s",
       xmlDescription(ilRc));
     return(FAIL);
  }
  _bmqVdebug(3,"S8804: value=[%s],len=[%d]",aResult,strlen(aResult));
  _bmqVdebug(2,"S8810: _ufProGETVALUE success!");
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
  char alXpath[iFLDVALUELEN],alTmp[100],alTmp1[100],alTmpexp[100];
  short ilFlag = 0;
  int i;
  
  _ufTtoN(aParm[0],alXpath,&ilLen);
  
  strcpy(alTmp,strstr(alXpath,"[")+1);
  alTmp[strlen(alTmp)-1] = 0;
  ilRc = _swExpN2T(alTmp,alTmpexp);
  if (ilRc != 0) 
  {
    _bmqVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
    return(FAIL);
  }
  ilRc = _swExpress(alTmpexp,alTmp1,&ilResultlen);  
  if(ilRc != SUCCESS)
  {
    _bmqVdebug(1,"S0260: [函数调用] _ufProGETVALUEM 计算平台表达式出错");
    return(FAIL);
  }
  _bmqVdebug(1,"S0270: 计算平台表达式成功,result=[%s]",alTmp1);
  for(i=0;i<strlen(alXpath);i++)
  {
    if(alXpath[i] == '[')
      break;
  }
  alXpath[i+1]=0;
  strcat(alXpath,alTmp1);
  strcat(alXpath,"]");
              
  _bmqVdebug(3,"S8783: [函数调用] _ufProGETVALUEM (%s)",alXpath);
  if(iParmCount == 0)
  {
     _bmqVdebug(0,"S8784: [函数调用] _ufProGETVALUEM 错误, 参数为空");
     return(FAIL);
  }
  if(cgXmlInit == 0)
  {
    ilRc = swXmlinit();
    if (ilRc)
    {
      _bmqVdebug(0,"S8785: GETVALUE() swXmlinit error");
      return(FAIL);
    }
    ilFlag = 1;
    _bmqVdebug(1,"S8786: GETVALUE() swXmlinit success");
  }
  _bmqVdebug(1,"S8791: [函数调用]_ufProGETVALUEM ,初始化成功");
  /* modify by nh 20040419 
  ilRc = xmlGetValue(alXpath,aResult);*/
  ilRc = xmlGetNode(alXpath,aResult,NULL);
  if(ilRc != SUCCESS && ilRc != 2)
  {
     _bmqDebug("S8790: [函数调用] _ufProGETVALUEM 错误, %s",
       xmlDescription(ilRc));
     
     return(FAIL);
  }
  if(ilRc == 2)
   strcpy(aResult,"NULL");
  
  _bmqVdebug(3,"S8804: value=[%s],len=[%d]",aResult,strlen(aResult));
  _bmqVdebug(2,"S8810: _ufProGETVALUEM success!");
  if(ilFlag == 1)
    swXmlfree();
  return(SUCCESS);
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
      _bmqVdebug(0,"S8789: [函数调用]_ufProGETVALUE 错误, 参数错误");
      return(FAIL);
   }
   if(cgXmlInit == 0)
   {
     ilRc = swXmlinit();
     if (ilRc)
     {
       _bmqVdebug(0,"S8792: GETVALUE() swXmlinit error");
       return(FAIL);
     }
     ilFlag = 1;
     _bmqVdebug(1,"S8795: GETVALUE() swXmlinit success");
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
      _bmqDebug("S8789: [函数调用]_ufProGETPROP 错误, %s",
         xmlDescription(ilRc));
      return(FAIL);
   }
   if(ilFlag == 1) 
     swXmlfree();
   return(SUCCESS);
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

  _bmqVdebug(3,"S8771: [函数调用]_ufProXMLINIT");

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
      _bmqVdebug(0,"S9346: [函数调用]_ufProXMLINIT() 出错, %s",
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
          _bmqVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %s",
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
          _bmqVdebug(0,"S8771: [函数调用] _ufProXMLINIT 出错, %s",
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
  _bmqVdebug(1,"S9345: [调用结束]_ufProXMLINIT()返回=0,结果=%s",aResult);
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
  _bmqVdebug(3,"S8783: [函数调用] _ufProXMLFREE");
  xmlFree();
  aResult[0] = 0x00;
  cgXmlInit = 0;
  return(SUCCESS);
}
#endif

