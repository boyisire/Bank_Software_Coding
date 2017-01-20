/****************************************************************/
/* ģ����    ��USRFUN                                         */ 
/* ģ������    ���Զ��庯������                                 */
/* ��	 ��    ���Ż�                                           */
/* ��������    ��1999/11/18                                     */
/* ����޸����ڣ�2001/2/27                                      */
/* ģ����;    ��ʵ�ֶ��Զ��庯���Ĵ���                         */
/* ��ģ���а������º���������˵����                             */
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
/* �޸ļ�¼��                                                   */
/****************************************************************/
/* switch���� */
#include "bmq.h"

int _swTrim();
int _swTimeLongToChs();
static short cgXmlInit=0;
/**************************************************************
 ** �� �� ����_swExpress
 ** ��    �ܣ����㺯�����ʽ
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺aExpress   ��������ʽ
 ** ����������aResult    : ����Ľ��
 **           piResultlen�����������
 ** �� �� ֵ��0(�ɹ�) / -1(ʧ��)
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
          return(-1); /* ������̫�� */
        }
        if(palStack[ilStack][0]=='(') break;
      }
      for(j=0;j<i;j++)
      {
        strcpy(palParm[j],palStack[ilStack+j+1]);
      }
      /* ����alResult */
      /* �״ε������� */
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
            _bmqVdebug(0,"S8000: [����/����] ƽ̨����{%s}����ֵ����",alFunc);  
            return(-1);
          }
        }
        else if(proclist[ilCnt].fproc_old!=NULL)
        {
          strcpy(alResult,(*(proclist[ilCnt].fproc_old))(palParm));
        }
        else
        {
          _bmqVdebug(0,"S8003: [����/����] ƽ̨����{%s}�ں����б���û�ж���",alFunc);
          return(-1);
        }  
      }
      else
      {
          _bmqVdebug(0,"S8012: [����/����] ƽ̨����{%s}�ں����б���û�ж���",alFunc);
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
    return(-1); /* ������̫�� */
  }
  _ufTtoN(palStack[0],aResult,piResultlen);
  return(0);
}

/**************************************************************
 ** �� �� ����_swExpN2T
 ** ��    �ܣ����������ʽת��Ϊ�沨�����ʽ
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����_swParseN2T
 ** ȫ�ֱ�����
 ** �������壺aNstr: ������������ʽ
 ** ��    ���aaTstr: ����沨�����ʽ
 ** �� �� ֵ��0(�ɹ�) / -1(ʧ��)
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
 ** �� �� ����_swExpT2N
 ** ��    �ܣ����沨�����ʽת��Ϊ�������ʽ
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����_swParseT2N
 ** ȫ�ֱ�����
 ** �������壺aTstr: �����沨�����ʽ
 ** ��    ���aaNstr: ������������ʽ
 ** �� �� ֵ��0(�ɹ�) / -1(ʧ��)
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
 ** �� �� ����_swParseN2T
 ** ��    �ܣ��������ʽ���沨�����ʽ�ĵݹ����
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����_swGetFuncAndParmN2T
 ** ȫ�ֱ�����
 ** �������壺aResult: ��������
 ** ��    ���aaStr   : ������ʽ
 ** �� �� ֵ��0(�ɹ�) / -1(ʧ��)
***************************************************************/
int _swParseN2T(char *aResult,char *aStr)
{
  int i,ilRc,ilParmCount=0;
  char alParm[MAXVARNUM][iFLDVALUELEN];
  char alFunc[FUNCLEN],alTmp[iFLDVALUELEN];

  ilParmCount=_swGetFuncAndParmN2T(aStr,alFunc,alParm);
  if (ilParmCount == -1)
  {
    _bmqVdebug(0,"S8990: [����/ƽ̨����]: _swParseN2T(),���ʽ[%s]���Ϸ�!",aStr);
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
 ** �� �� ����_swParseT2N
 ** ��    �ܣ��沨�����ʽ���������ʽ�ĵݹ����
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����_swGetFuncAndParmT2N
 ** ȫ�ֱ�����
 ** �������壺aResult: ��������
 ** ��    ���aaStr   : ������ʽ
 ** �� �� ֵ��0(�ɹ�) / -1(ʧ��)
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
 ** �� �� ����_swGetFuncAndParmN2T
 ** ��    �ܣ������������ʽ���е��������Ͳ����б�
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺aStr ��������ʽ��
 ** ��    ���aaFunc�����صĺ�����
 **           aParm�����صĲ�����
 ** �� �� ֵ�����еĲ�������?
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
    _bmqVdebug(0,"S8015: [����/ƽ̨����{%s}]: _swGetFuncAndParmN2T()",aStr);
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
            _bmqVdebug(0,"S8018: [����/ƽ̨����]: _swGetFuncAndParmN2T() ������������!"); 
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
    _bmqVdebug(0,"S8021: [����/ƽ̨����]: _swGetFuncAndParmN2T()");
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
 ** �� �� ����_swGetFuncAndParmT2N
 ** ��    �ܣ������沨�����ʽ���еĺ������Ͳ����б�
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺aStr ��������ʽ��
 ** ��    ���aaFunc�����صĺ�����
 **           aParm�����صĲ�����
 ** �� �� ֵ�����еĲ�������?
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
    _bmqVdebug(0,"S8024: [����/ƽ̨����]: _swGetFuncAndParmN2T()");
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
    _bmqVdebug(0,"S8027: [����/ƽ̨����]: _swGetFuncAndParmN2T()");
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
 ** �� �� ����_swUsrfunSort
 ** ��    �ܣ�����ָ�����鰴��������������
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺
 ** �� �� ֵ��0
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
 ** �� �� ����_swGetfuncPtr
 ** ��    �ܣ���ȡ�û�����ָ�������±�
 ** ��    �ߣ�
 ** �������ڣ�2001/11/23
 ** �޸����ڣ�2001/11/23
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺
 ** �� �� ֵ���û�����ָ�������±�(�ɹ�) / -1(ʧ��)
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
 ** ������      �� _ufNtoT
 ** ��  ��      �� ��δ��ת����ַ�������ת��
 ** ��  ��      �� ʷ����
 ** ��������    �� 2000/2/16
 ** ����޸����ڣ� 2000/2/16
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aNstr:δ��ת����ַ���
 **                iLen :δ��ת����ַ�������
 **                aTstr:�Ѿ�ת����ַ���
 ** ����ֵ      :  �Ѿ�ת����ַ���
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
 ** ������      �� _ufTtoN
 ** ��  ��      �� ���Ѿ�ת����ַ������з�ת��
 ** ��  ��      �� ʷ����
 ** ��������    �� 2000/2/16
 ** ����޸����ڣ� 2000/2/16
 ** ������������:
 ** ȫ�ֱ���    ��
 ** ��������    :  alTstr:�Ѿ�ת����ַ���
 **                alNstr:δ��ת����ַ���
 **                pilLen :δ��ת����ַ�������
 ** ����ֵ      :  δ��ת����ַ���
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
 ** ������      �� _ufProNSP
 ** ��  ��      �� ȡ�ö�����ʽ���ĵ���ֵ
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ��ʼλ��
                   aParm[1] : �򳤶�
 ** ����ֵ      �� ��ֵ
***************************************************************/
int _ufProNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen=0; 
  short ilOffset;
  _bmqVdebug(3,"S8648: [��������] _ufProNSP(%s,%s)",aParm[0],aParm[1]);

  _ufTtoN(aParm[0],alTmp,&ilLen);
  ilOffset = atoi(alTmp) - 1;
  if (ilOffset<0) ilOffset=0;

  _ufTtoN(aParm[1],alTmp,&ilLen);
  ilLen = atoi(alTmp);
  /* add by zjj 2004.03.15 */
  if (ilLen >= iFLDVALUELEN)
  {
    _bmqVdebug(0,"S8649: [����/����] _ufPropNSP,ȡֵ����[%d]������ֵ��󳤶�[%d]",ilLen,iFLDVALUELEN);
    return(FAIL);
  }
  /*end add by zjj */

/* delete by szhengye 2001.8.30
  if ((ilOffset+ilLen)>igMsgbodylen)
    ilLen=igMsgbodylen - ilOffset;
  if (ilLen<0) ilLen=0;
*/

  _ufNtoT(agMsgbody+ilOffset,ilLen,aResult);


  _bmqVdebug(3,"S8054: [��������] _ufProNSP()������=0,���=%s",aResult);
  return(0);

}



unsigned char _ufStr2Char(a)
  char *a;
{
  short j;
  unsigned char b;
  
  _bmqVdebug(3,"S8084: [��������] _ufStr2Char(%s)",a);
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

  _bmqVdebug(3,"S8087: [��������] _ufStr2Char()������=%d",b);
  return(b);
}
  



/**************************************************************
 ** ������      �� _ufProMID
 ** ��  ��      �� ȡ�Ӵ� 
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/11/18
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : Դ���ݴ�
                   aParm[1] : ��ʼλ��
                   aParm[2] : ��ȡ����
 ** ����ֵ      �� ������Ӵ�
***************************************************************/
int _ufProMID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  short i,ilPos,ilStrlen,ilLen;

  _bmqVdebug(3,"S8150: [��������] _ufProMID(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  ilPos=atoi(aParm[1]);
  ilLen=atoi(aParm[2]);
  _ufTtoN(aParm[0],aResult,&ilStrlen);

  if (ilStrlen<(ilPos+ilLen-1))
     ilLen = ilStrlen - ilPos;

  for(i=0;i<ilLen;i++)
     alTmpstr[i]=aResult[ilPos+i-1];
  alTmpstr[i] = '\0';

  _ufNtoT(alTmpstr,i,aResult);

  _bmqVdebug(3,"S8153: [��������] _ufProMID()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      �� _ufProLFT
 ** ��  ��      �� ȡ���Ӵ� 
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : Դ���ݴ�
                   aParm[1] : ��ȡ����
 ** ����ֵ      �� ������Ӵ�
***************************************************************/
int  _ufProLFT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  short ilLen,ilStrlen; 

  _bmqVdebug(3,"S8156: [��������] _ufProLFT(%s,%s)",aParm[0],aParm[1]);
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);

  ilLen=atoi(aParm[1]);
  if(ilStrlen<ilLen) ilLen=ilStrlen;

  _ufNtoT(alTmpstr,ilLen,aResult);

  _bmqVdebug(3,"S8159: [��������] _ufProLFT()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      �� _ufProRIG
 ** ��  ��      �� ȡ���Ӵ� 
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : Դ���ݴ�
                   aParm[1] : ��ȡ����
 ** ����ֵ      �� ������Ӵ�
***************************************************************/
int _ufProRIG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmpstr[iFLDVALUELEN];
  short ilLen,ilStrlen; 

  _bmqVdebug(3,"S8162: [��������] _ufProRIG(%s,%s)",aParm[0],aParm[1]);
      
  _ufTtoN(aParm[0],alTmpstr,&ilStrlen);

  ilLen=atoi(aParm[1]);
  if(ilStrlen<ilLen) ilLen=ilStrlen;

  _ufNtoT(alTmpstr+ilStrlen-ilLen,ilLen,aResult);
  _bmqVdebug(3,"S8165: [��������] _ufProRIG()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      �� _ufProFILL
 ** ��  ��      �� ����ַ���
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ���ݴ�1
                   aParm[1] : ���ݴ�2
 ** ����ֵ      �� ����Ĵ�
***************************************************************/
int _ufProFILL(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char cFiller, cJustify;
  short iFilllen; 
  char alFillbuf[iFLDVALUELEN];
  short ilBuflen; 

  _bmqVdebug(3,"S8168: [��������] _ufProFILL(%s,%s)",aParm[0],aParm[1]);

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

  _bmqVdebug(3,"S8171: [��������] _ufProFILL()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      �� _ufProSAD
 ** ��  ��      �� �����������ݴ�
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ���ݴ�1
                   aParm[1] : ���ݴ�2
 ** ����ֵ      �� ����Ĵ�
***************************************************************/
int _ufProSAD(char aParm[][iFLDVALUELEN],short iParmCount, char *aResult)
{
  int i,j;
  short ilStrlen; 
  char alTmp[iFLDVALUELEN];

  _bmqVdebug(3,"S8174: [��������] _ufProSAD(%s,%s)",aParm[0],aParm[1]);

  j=0;
  for(i=0;i<iParmCount;i++)
  {
    /* if(aParm[i][0]=='\0') break; */
    _ufTtoN(aParm[i],alTmp+j,&ilStrlen);
    j = j + ilStrlen;
  }
  alTmp[j]='\0';
  
  _ufNtoT(alTmp,j,aResult);

  _bmqVdebug(3,"S8177: [��������] _ufProSAD()������=0,���=%s",aResult);
  
  return(0);
}


/**************************************************************
 ** ������      �� _ufProLEN
 ** ��  ��      �� ������ֵ
 ** ��  ��      �� ʷ���� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �ַ���
 ** ����ֵ      �� �ַ�������
***************************************************************/
int _ufProLEN(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen; 
  
  _bmqVdebug(3,"S8180: [��������] _ufProLEN(%s)",aParm[0]);
      
  _ufTtoN(aParm[0],aResult,&ilStrlen);

  sprintf(aResult,"%d",ilStrlen);

  _bmqVdebug(3,"S8183: [��������] _ufProLEN()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      �� _ufProTRIM
 ** ��  ��      �� ȥ��ǰ��ո�
 ** ��  ��      �� ʷ���� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �ַ���
 ** ����ֵ      �� �ַ�������
***************************************************************/
int _ufProTRIM(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
  char alTmp[iFLDVALUELEN];

  _bmqVdebug(3,"S8186: [��������] _ufProTRIM(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrim(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8189: [��������] _ufProTRIM()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      : _swTrim
 ** ��  ��      : �ú������ַ��� s ��ǰ��ո�β�س�ȥ��
 ** ��  ��      : llx
 ** ��������    : 1999/11/4
 ** ����޸�����: 2000/3/10
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : s   :�ַ���
 ** ����ֵ      : SUCCESS
		  ת������ַ�����s����
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
 ** ������      : _swTrimL
 ** ��  ��      : �ú������ַ��� s ��ǰ�ո�β�س�ȥ��
 ** ��  ��      : lijunwen
 ** ��������    : 2000/3/29
 ** ����޸�����: 2000/3/29
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : s   :�ַ���
 ** ����ֵ      : SUCCESS
		  ת������ַ�����s����
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
 ** ������      : _swTrimR
 ** ��  ��      : �ú������ַ��� s �ĺ�ո�β�س�ȥ��
 ** ��  ��      : lijunwen
 ** ��������    : 2000/3/29
 ** ����޸�����: 2000/3/29
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : s   :�ַ���
 ** ����ֵ      : SUCCESS
		  ת������ַ�����s����
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
 ** ������      �� _ufProTRIML
 ** ��  ��      �� ȥ����߿ո�
 ** ��  ��      �� lijunwen 
 ** ��������    �� 2000/3/29
 ** ����޸����ڣ� 2000/3/29
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �ַ���
 ** ����ֵ      �� �ַ�������
***************************************************************/
int _ufProTRIML(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen; 
  char alTmp[iFLDVALUELEN];

  _bmqVdebug(3,"S8192: [��������] _ufProTRIML(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrimL(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8195: [��������] _ufProTRIML()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProTRIMR
 ** ��  ��      �� ȥ���ұ߿ո�
 ** ��  ��      �� lijunwen 
 ** ��������    �� 2000/3/29
 ** ����޸����ڣ� 2000/3/29
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �ַ���
 ** ����ֵ      �� �ַ�������
***************************************************************/
int _ufProTRIMR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen; 
  char alTmp[iFLDVALUELEN];

  _bmqVdebug(3,"S8198: [��������] _ufProTRIMR(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilStrlen);
  _swTrimR(alTmp);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8201: [��������] _ufProTRIMR()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      : _ufProSPACE
 ** ��  ��      : 
 ** ��  ��      : 
 ** ��������    �� 2000/3/30
 ** ����޸����ڣ� 2000/3/30
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm����
 ** ����ֵ      :  �ո� 
***************************************************************/
int _ufProSPACE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  /*short ilStrlen; delete by wh*/
  TYPE_SWITCH ilStrlen; /*add by wh*/
 
  _bmqVdebug(3,"S8204: [��������] _ufProSPACE(%s)",aParm[0]);

  ilStrlen=atoi((char *)aParm[0]);

  memset(aResult, ' ', ilStrlen);
  aResult[ilStrlen] = 0;

  _bmqVdebug(3,"S8207: [��������] _ufProSPACE()������=0,���=%s",aResult);
  return 0;
}

/* ============ add by dgm 2001/10/12 ========== */
int _ufProADD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
	/*
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;

  _bmqVdebug(3,"S8225: [��������] _ufProADD(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
   
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
    _bmqVdebug(3,"S8228: [��������] _ufProADD()������=0,���=%s",aResult);
    return 0;
  }
  if (ilNum == 0) ilNum = -1;
  *(p1 + ilNum+1) = '\0';
   
  _bmqVdebug(3,"S8231: [��������] _ufProADD()������=0,���=%s",aResult);
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
  
  
  _bmqVdebug(3,"S8240: [��������] _ufProSUB(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);
   
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
    _bmqVdebug(3,"S8243: [��������] _ufProSUB()������=0,���=%s",aResult);
    return 0;
  }
 if (ilNum == 0) ilNum = -1;
 *(p1 + ilNum+1) = '\0';
   
  _bmqVdebug(3,"S8246: [��������] _ufProSUB()������=0,���=%s",aResult);
*/
  return 0;
}
int _ufProMULTI(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
/*
  short ilC1 = 0, ilC2 = 0, ilNum;
  double dlTmp1,dlTmp2, dlTmp;
  char alFmt[20], *p1;
  
  _bmqVdebug(3,"S8255: [��������] _ufProMULTI(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

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
    _bmqVdebug(3,"S8258: [��������] _ufProMULTI()������=0,���=%s",aResult);
    return 0;
  }
  if (ilNum == 0) ilNum = -1;
    *(p1 + ilNum+1) = '\0';
*/
  _bmqVdebug(3,"S8261: [��������] _ufProMULTI()������=0,���=%s",aResult);
  return 0;
}
/**************************************************************
 ** ������      �� _ufProDIV
 ** ��  ��      : ������������
 ** ��  ��      : 
 ** ��������    ��2001/09/25
 ** ����޸����ڣ�2001/09/25
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] :������
 **             :  aParm[1] :����
 ** ����ֵ      :  �̵��ַ�����ʾ
***************************************************************/
int _ufProDIV(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  ldiv_t slResult;
  long llNum1,llNum2;
  long llRt;

  _bmqVdebug(3,"S8264: [��������] _ufProDIV(%s,%s)",aParm[0],aParm[1]);

  llRt=0;
  llNum1=atol(aParm[0]);
  llNum2=atol(aParm[1]);
  if (llNum2!=0)
  {
    slResult=ldiv(llNum1,llNum2);
    llRt=slResult.quot;
  }
  sprintf(aResult,"%ld",llRt);

  _bmqVdebug(3,"S8267: [��������] _ufProDIV()������=0,���=%s",aResult);
  return 0;
}


/**************************************************************
 ** ������      �� _ufProMOD
 ** ��  ��      : ����������ģ��
 ** ��  ��      : ����� 
 ** ��������    ��2000/6/09
 ** ����޸����ڣ�2000/6/09
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] :������
 **             :  aParm[1] :����
 ** ����ֵ      :  ģ�����ַ�����ʾ
***************************************************************/
int _ufProMOD(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  ldiv_t slResult;
  long llNum1,llNum2;
  long llRt;

  _bmqVdebug(3,"S8270: [��������] _ufProMOD(%s,%s)",aParm[0],aParm[1]);

  llRt=0;
  llNum1=atol(aParm[0]);
  llNum2=atol(aParm[1]);
  if (llNum2!=0)
  {
    slResult=ldiv(llNum1,llNum2);
    llRt=slResult.rem;
  }
  sprintf(aResult,"%ld",llRt);

  _bmqVdebug(3,"S8273: [��������] _ufProMOD()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProSEQ
 ** ��  ��      �� �Ƚ��������ݴ��Ƿ����
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ���ݴ�1
                   aParm[1] : ���ݴ�2
 ** ����ֵ      �� ��ֵ
***************************************************************/
int _ufProSEQ(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  _bmqVdebug(3,"S8276: [��������] _ufProSEQ(%s,%s)",aParm[0],aParm[1]);

  if (!strcmp(aParm[0],aParm[1]))
     strcpy(aResult,"1");
  else
     strcpy(aResult,"0");

  _bmqVdebug(3,"S8279: [��������] _ufProSEQ()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProGT
 ** ��  ��      �� �ж�ǰ���Ƿ���ں���
 ** ��  ��      �� ����Ӣ
 ** ��������    �� 2000/5/25
 ** ����޸����ڣ� 2000/5/25
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� 
 ** ����ֵ      �� �����򷵻�1,���򷵻�0
***************************************************************/
int _ufProGT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  _bmqVdebug(3,"S8282: [��������] _ufProGT(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a>b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8285: [��������] _ufProGT()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProGE
 ** ��  ��      �� �ж�ǰ���Ƿ���ڵ��ں���
 ** ��  ��      �� ����Ӣ
 ** ��������    �� 2000/5/25
 ** ����޸����ڣ� 2000/5/25
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� 
 ** ����ֵ      �� ���ڵ����򷵻�1,���򷵻�0
***************************************************************/
int _ufProGE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  _bmqVdebug(3,"S8288: [��������] _ufProGE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a>=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8291: [��������] _ufProGE()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProLT
 ** ��  ��      �� �ж�ǰ���Ƿ�С�ں���
 ** ��  ��      �� ����Ӣ
 ** ��������    �� 2000/5/25
 ** ����޸����ڣ� 2000/5/25
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� 
 ** ����ֵ      �� С���򷵻�1,���򷵻�0
***************************************************************/
int _ufProLT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  _bmqVdebug(3,"S8294: [��������] _ufProLT(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a<b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8297: [��������] _ufProLT()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProLE
 ** ��  ��      �� �ж�ǰ���Ƿ�С�ڵ��ں���
 ** ��  ��      �� ����Ӣ
 ** ��������    �� 2000/5/25
 ** ����޸����ڣ� 2000/5/25
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� 
 ** ����ֵ      �� С�ڵ����򷵻�1,���򷵻�0
***************************************************************/
int _ufProLE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  _bmqVdebug(3,"S8300: [��������] _ufProLE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a<=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8303: [��������] _ufProLE()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProNE
 ** ��  ��      �� �ж�ǰ���Ƿ񲻵��ں���
 ** ��  ��      �� ����Ӣ
 ** ��������    �� 2000/5/25
 ** ����޸����ڣ� 2000/5/25
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� 
 ** ����ֵ      �� �������򷵻�1,���򷵻�0
***************************************************************/
int _ufProNE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  double a,b;

  _bmqVdebug(3,"S8306: [��������] _ufProNE(%s,%s)",aParm[0],aParm[1]);

  a=atof(aParm[0]);
  b=atof(aParm[1]);
  if(a!=b)
    strcpy(aResult,"1");
  else
    strcpy(aResult,"0");

  _bmqVdebug(3,"S8309: [��������] _ufProNE()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProNOT
 ** ��  ��      �� �߼���
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ����
 ** ����ֵ      �� �棺1; �٣�0
***************************************************************/
int _ufProNOT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  _bmqVdebug(3,"S8318: [��������] _ufProNOT(%s)",aParm[0]);

  if ( aParm[0][0]=='0')
     strcpy(aResult,"1");
  else
     strcpy(aResult,"0");

  _bmqVdebug(3,"S8321: [��������] _ufProNOT()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProAND
 ** ��  ��      �� �߼���
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ����
 ** ����ֵ      �� �棺1; �٣�0
***************************************************************/
int _ufProAND(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;
  short ilFlag=1;
  char alTmp[iFLDVALUELEN];
  short ilStrlen;

  _bmqVdebug(3,"S8324: [��������] _ufProAND(%s)",aParm[0]);
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

  _bmqVdebug(3,"S8330: [��������] _ufProAND()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProOR
 ** ��  ��      �� �߼���
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ����
 ** ����ֵ      �� �棺1; �٣�0
***************************************************************/
int _ufProOR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;
  short ilFlag=0;
  short ilStrlen;
  char  alTmp[iFLDVALUELEN];

  _bmqVdebug(3,"S8333: [��������] _ufProOR(%s)",aParm[0]);
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

  _bmqVdebug(3,"S8339: [��������] _ufProOR()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProIF
 ** ��  ��      �� ������ֵ
 ** ��  ��      �� �Ż� 
 ** ��������    �� 1999/12/2
 ** ����޸����ڣ� 1999/12/2
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �߼�����
                   aParm[1] : ����Ϊ��ʱ��Ӧ����ֵ
                   aParm[2] : ����Ϊ��ʱ��Ӧ����ֵ
 ** ����ֵ      �� ����2�����3
***************************************************************/
int _ufProIF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen; 

  _bmqVdebug(3,"S8342: [��������] _ufProIF(%s,%s,%s)",aParm[0],aParm[1],aParm[2]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);

  if (((*aResult)!='0')&&((*aResult)!='\0')) 
    strcpy(aResult,aParm[1]);
  else 
    strcpy(aResult,aParm[2]);

  _bmqVdebug(3,"S8345: [��������] _ufProIF()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProCASEIF
 ** ��  ��      �� ����CASE
 ** ��  ��      �� ʷ���� 
 ** ��������    �� 2000.5.24
 ** ����޸����ڣ� 2000.5.24
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� ����λ��Ϊ1��0,ż��λ��Ϊ��Ӧ��ֵ,���λΪdefault 
 ** ����ֵ      �� ����1��Ӧ��ֵ
***************************************************************/
int _ufProCASEIF(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;

  _bmqVdebug(3,"S8348: [��������] _ufProCASEIF(%s)",aParm[0]);
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

  _bmqVdebug(3,"S8354: [��������] _ufProCASEIF()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProCASEVAR
 ** ��  ��      �� ֵCASE
 ** ��  ��      �� ʷ���� 
 ** ��������    �� 2000.5.24
 ** ����޸����ڣ� 2000.5.24
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ����һλΪ����,�������ֵ,�����Ӧ��ֵ
 ** ����ֵ      ����һλΪ��,�򷵻�����Ӧ��ֵ 
***************************************************************/
int _ufProCASEVAR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int   i;

  _bmqVdebug(3,"S8357: [��������] _ufProCASEVAR(%s)",aParm[0]);
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

  _bmqVdebug(3,"S8363: [��������] _ufProCASEVAR()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      ��_ufProCHR
 ** ��  ��      ��ȡ��16����ֵ
 ** ��  ��      ������Ӣ 
 ** ��������    ��2000/5/10
 ** ����޸����ڣ�2000/5/10
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] : ��1
 ** ����ֵ      �� ����Ĵ�
***************************************************************/
int _ufProCHR(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int iltmp;

  _bmqVdebug(3,"S8366: [��������] _ufProCHR(%s)",aParm[0]);
  
  iltmp=atoi(aParm[0]);
  if(iltmp<0||iltmp>255) return(-1);
  sprintf(aResult,"%c",iltmp);

  _bmqVdebug(3,"S8369: [��������] _ufProCHR()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      ��_ufProSHORT
 ** ��  ��      �����ַ�ת��Ϊ������
 ** ��  ��      ������Ӣ 
 ** ��������    ��2000/5/17
 ** ����޸����ڣ�2000/9/17
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] :  �ַ� һ��������ռ�����ֽ�
 ** ����ֵ      �� ���������   ��"12"ת��Ϊ0x00,0x0c
***************************************************************/
int _ufProSHORT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen; 
  short sltmp;

  _bmqVdebug(3,"S8372: [��������] _ufProSHORT(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  sltmp=atoi(aResult);
  memcpy(alTmp,&sltmp,2);

  alTmp[2]='\0';
  _ufNtoT(alTmp,2,aResult);

  _bmqVdebug(3,"S8375: [��������] _ufProSHORT()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      ��_ufProINT
 ** ��  ��      �����ַ�ת��Ϊ����
 ** ��  ��      ������Ӣ 
 ** ��������    ��2000/5/17
 ** ����޸����ڣ�2000/5/17
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] :  �ַ� һ������ռ�ĸ��ֽ�
 ** ����ֵ      �� ���������   ��"12"ת��Ϊ0x00,0x00,0x00,0x0c
***************************************************************/
int _ufProINT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen;
  int   iltmp;

  _bmqVdebug(3,"S8378: [��������] _ufProINT(%s)",aParm[0]);
 
  _ufTtoN(aParm[0],aResult,&ilLen);
  iltmp=atoi(aResult);
  memcpy(alTmp,&iltmp,4);

  alTmp[4]='\0';
  _ufNtoT(alTmp,4,aResult);

  _bmqVdebug(3,"S8381: [��������] _ufProINT()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      ��_ufProLONG
 ** ��  ��      �����ַ�ת��Ϊ��������
 ** ��  ��      ������Ӣ 
 ** ��������    ��2000/5/17
 ** ����޸����ڣ�2000/9/14
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] :  �ַ� һ��������ռ�ĸ��ֽ�
 ** ����ֵ      �� ����ĳ�����  
***************************************************************/
int _ufProLONG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  short ilLen; 
  long lltmp;

  _bmqVdebug(3,"S8384: [��������] _ufProLONG(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  lltmp=atol(aResult);
  memcpy(alTmp,&lltmp,4);

  alTmp[4]='\0';
  _ufNtoT(alTmp,4,aResult);

  _bmqVdebug(3,"S8387: [��������] _ufProLONG()������=0,���=%s",aResult);
  return 0;
}
  
/**************************************************************
 ** ������      ��_ufProDOUBLE
 ** ��  ��      �����ַ�ת��Ϊ˫���ȸ�����
 ** ��  ��      ������Ӣ 
 ** ��������    ��2000/5/17
 ** ����޸����ڣ�2000/9/14
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] :�����ַ���С���� һ��˫���ȸ�����ռ�˸��ֽ�
 ** ����ֵ      �� �����˫���ȸ�������  
***************************************************************/
int _ufProDOUBLE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  double dltmp;
  short  ilLen; 

  _bmqVdebug(3,"S8390: [��������] _ufProDOUBLE(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilLen);
  dltmp=atof(aResult)/100;

  memcpy(alTmp,&dltmp,8);

  alTmp[8]='\0';
  _ufNtoT(alTmp,8,aResult);

  _bmqVdebug(3,"S8393: [��������] _ufProDOUBLE()������=0,���=%s",aResult);
  return 0;
}

/*************************************************************
 ** ������      ��_ufProHEX
 ** ��  ��      ��ȡ��16����ֵ
 ** ��  ��      ������Ӣ 
 ** ��������    ��2000/5/10
 ** ����޸����ڣ�2000/5/10
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0] : ��
 ** ����ֵ      �� ����Ĵ�
***************************************************************/
int _ufProHEX(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  
  int iltmp;
  char alResult[iFLDVALUELEN];

  _bmqVdebug(3,"S8402: [��������] _ufProHEX(%s)",aParm[0]);

  iltmp=_ufStr2Char(aParm[0]);
  sprintf(alResult,"%c",iltmp);

  _ufNtoT(alResult,sizeof(char),aResult);

  _bmqVdebug(3,"S8405: [��������] _ufProHEX()������=0,���=%s",aResult);
  return 0;

}

  
/**************************************************************
 ** ������      ��_ufProUSHORT
 ** ��  ��      ����short�͵���ֵת�����ַ���
 ** ��  ��      ���Ż�
 ** ��������    ��2000/5/13
 ** ����޸����ڣ�2000/5/13
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0]:��ֵ
 ** ����ֵ      �� �ַ��ʹ�
***************************************************************/
int _ufProUSHORT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alOrig[iFLDVALUELEN];
  short ilLen;
  unsigned short *pilNum;

  _bmqVdebug(3,"S8408: [��������] _ufProUSHORT(%s)",aParm[0]);
 
  _ufTtoN(aParm[0],alOrig,&ilLen);
  alOrig[ilLen]='\0';
  pilNum = (unsigned short *)(alOrig);
  sprintf(aResult,"%d",*pilNum);

  _bmqVdebug(3,"S8411: [��������] _ufProUSHORT()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      ��_ufProULONG
 ** ��  ��      ����long����ֵת�����ַ���
 ** ��  ��      ���Ż�
 ** ��������    ��2000/5/13
 ** ����޸����ڣ�2000/5/13
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  aParm[0]:��ֵ
 ** ����ֵ      �� �ַ��ʹ�
***************************************************************/
int _ufProULONG(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alTmp[iFLDVALUELEN];
  unsigned long *pllValue;
  short ilLen; 

  _bmqVdebug(3,"S8414: [��������] _ufProULONG(%s)",aParm[0]);

  _ufTtoN(aParm[0],alTmp,&ilLen);
  alTmp[ilLen] = '\0';
  pllValue=(unsigned long*)alTmp;
  sprintf(aResult,"%ld",*pllValue);

  _bmqVdebug(3,"S8417: [��������] _ufProULONG()������=0,���=%s",aResult);
  return 0;
}

/**************************************************************
 ** ������      �� _ufProGETTIME
 ** ��  ��      �� ȡ��ǰʱ��
 ** ��  ��      �� 
 ** ��������    �� 2000/3/20
 ** ����޸����ڣ� 2000/3/20
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ��ʽ
 ** ����ֵ      �� �ַ�������
***************************************************************/
int _ufProGETTIME(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  char alTmp[iFLDVALUELEN];
  long llCurrentTime;
  short ilStrlen; 

  _bmqVdebug(3,"S8420: [��������] _ufProGETTIME(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  time(&llCurrentTime);
  _swTimeLongToChs(llCurrentTime,alTmp,aResult);
  _ufNtoT(alTmp,strlen(alTmp),aResult);

  _bmqVdebug(3,"S8423: [��������] _ufProGETTIME()������=0,���=%s",aResult);
  return(0);
}
/*******************************************************************/
/* �� �� ��       :  _swTimeLongToChs                               */
/* ��    ��       :  �������͵�ʱ��ת��Ϊ�ַ�����                  */
/* ��������       :  iDateTime �����͵�ʱ��                        */
/*                   pcFormat ʱ��ĸ�ʽ                           */
/* �� �� ֵ       :  pcDateTime �ַ����͵�ʱ��                     */
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
 ** ������      �� _ufProTODATE
 ** ��  ��      ��
 ** ��  ��      ��
 ** ��������    �� 2000/3/30
 ** ����޸����ڣ� 2000/3/30
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[] : Դ���ݴ�
 ** ����ֵ      �� ��ת���������Ӵ�
***************************************************************/
int _ufProTODATE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilStrlen;
  long llDatetime;
  struct tm *tmT;

  _bmqVdebug(3,"S8426: [��������] _ufProTODATE(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  llDatetime = atol(aResult);
  tmT = localtime(&llDatetime);

  sprintf(aResult,"%04d%02d%02d",tmT->tm_year+1900,tmT->tm_mon+1,tmT->tm_mday);
  aResult[8] = '\0';
  
  _bmqVdebug(3,"S8429: [��������] _ufProTODATE()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      �� _ufProTOTIME
 ** ��  ��      : 
 ** ��  ��      ��
 ** ��������    �� 2000/3/30
 ** ����޸����ڣ� 2000/3/30
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[] : Դ���ݴ�
 ** ����ֵ      �� 0-SUCCESS
***************************************************************/
int _ufProTOTIME(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  long llDatetime;
  short ilStrlen;
  struct tm *tmT;
 
  _bmqVdebug(3,"S8432: [��������] _ufProTOTIME(%s)",aParm[0]);

  _ufTtoN(aParm[0],aResult,&ilStrlen);
  llDatetime = atol(aResult);
  tmT = localtime(&llDatetime);
  
  sprintf(aResult,"%02d:%02d:%02d", tmT->tm_hour, tmT->tm_min, tmT->tm_sec);
/* ============ delete by dgm 2001/10/15 ===========
  aResult[6] = '\0';
========= end of delete by dgm 2001/10/15 ======== */
  
  _bmqVdebug(3,"S8435: [��������] _ufProTOTIME()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      ��_ufProPID
 ** ��  ��      ���õ������̵Ľ��̺�
 ** ��  ��      ������Ӣ 
 ** ��������    ��2000/5/12
 ** ����޸����ڣ�2000/5/12
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    :  ��
 ** ����ֵ      �� 0-SUCCESS
***************************************************************/
int _ufProPID(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  long llPid;
  
  _bmqVdebug(3,"S8438: [��������] _ufProPID()");

  llPid=getpid();
  sprintf(aResult,"%ld",llPid);

  _bmqVdebug(3,"S8441: [��������] _ufProPID()������=0,���=%s",aResult);
  return(0);
}



/**************************************************************
 ** ������      �� _ufProDATEtoLONG
 ** ��  ��      ��
 ** ��  ��      ��
 ** ��������    �� 2000/3/30
 ** ����޸����ڣ� 2000/3/30
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[] : ��ת���������Ӵ�yyyymmdd
 ** ����ֵ      �� 0-SUCCESS  
***************************************************************/
int _ufProDATEtoLONG(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  short ilStrlen;
  unsigned long  llDatetime;
  struct tm tmT={0,0,0};
  char alYear[5];
  char alMon[3];
  char alDate[3];

  _bmqVdebug(3,"S8453: [��������] _ufProDATEtoLONG(%s)",aParm[0]);
   
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
  
  _bmqVdebug(3,"S8459: [��������] _ufProDATEtoLONG()������=0,���=%s",aResult);
  return(0);
}



/**************************************************************
 ** ������      �� _ufProLONGTODATE
 ** ��  ��      �� change long to date  *aResult ��ת���������Ӵ�
 ** ��  ��      �� lijunwen
 ** ��������    �� 2000/3/30
 ** ����޸����ڣ� 2000/3/30
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[] : Դ���ݴ�
 ** ����ֵ      �� 0- success
***************************************************************/
int _ufProLONGTODATE(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  long llDatetime;
  struct tm *tmT;
 
  _bmqVdebug(3,"S8474: [��������] _ufProLONGTODATE(%s)",aParm[0]);

  llDatetime = atol(aParm[0]);
  
  tmT = localtime( &llDatetime );
  sprintf(aResult,"%04d-%02d-%02d",tmT->tm_year+1900,tmT->tm_mon+1,tmT->tm_mday);

  _bmqVdebug(3,"S8477: [��������] _ufProLONGTODATE()������=0,���=%s",aResult);
  return(0);
}

/**************************************************************
 ** ������      �� _ufProLONGTOTIME
 ** ��  ��      �� change long to time 
 ** ��  ��      �� lijunwen
 ** ��������    �� 2000/3/30
 ** ����޸����ڣ� 2000/3/30
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[] : Դ���ݴ�
 ** ����ֵ      �� 0--success
***************************************************************/
int _ufProLONGTOTIME(char aParm[][iFLDVALUELEN],short iParmCount,char * aResult)
{
  long llDatetime;
  struct tm *tmT;
 
  _bmqVdebug(3,"S8480: [��������] _ufProLONGTOTIME(%s)",aParm[0]);

  llDatetime = atol(aParm[0]);
  tmT = localtime( &llDatetime );
  
  sprintf(aResult,"%02d:%02d:%02d", tmT->tm_hour, tmT->tm_min, tmT->tm_sec);
  
  _bmqVdebug(3,"S8483: [��������] _ufProLONGTOTIME()������=0,���=%s",aResult);
  return(0);
}

/* ���Ӷ�ͨ��ǰ��֧�� */
/**************************************************************
 ** ������      : _ufProFmlget ȡFMLԤ�����ֵ����
 ** ��  ��      : �õ�FMLԤ����ָ�������ĳ��Ⱥ���ֵ
 ** ��  ��      : 
 ** ��������    : 1999.11.23
 ** ����޸�����: 1999.11.23
 ** ������������:
 ** ȫ�ֱ���    : psgPreunpackbuf	FMLԤ���
 ** ��������    : aParm[0]	����
 ** ����ֵ      :
***************************************************************/
int _ufProFmlget(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short ilRc;
  short /*ilMsglen,*/ilFldlen; /*delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  char  alFldname[iFLDNAMELEN+1];
  char  alTmp[iFLDVALUELEN+1];
  
  _bmqVdebug(3,"S8486: [��������] _ufProFmlget(%s)",aParm[0]);

  strcpy(alFldname, aParm[0]);
  ilMsglen = sgMsgpack.sMsghead.iBodylen;
  ilRc = swFmlpackget(sgMsgpack.aMsgbody, ilMsglen, alFldname, &ilFldlen, alTmp);
  if (ilRc) 
  {
    _bmqVdebug(0,"S8489: [����/��������] swFmlpackget()����,������=%d",ilRc);
    return(FAIL);
  }
  _ufNtoT(alTmp,ilFldlen,aResult);

  _bmqVdebug(3,"S8492: [��������] _ufProFmlget()������=0,���=%s",aResult);
  return(SUCCESS);
}
/**************************************************************
 ** ������: swFmlpackget
 ** ��  ��: �õ�FML������ָ�������ĳ��Ⱥ���ֵ
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
      _bmqVdebug(0,"S6060: [����/����] FML������������[%s]",alFldname);
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
 ** ������      �� _ufProPackSEP
 ** ��  ��      �� ���ݼ�¼ID����ID����IDȡ����ֵ
 ** ��  ��      ��
 ** ��������    �� 2001.9.7
 ** ����޸����ڣ� 
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : ��¼�ָ���
                   aParm[1] : ��ָ���
                   aParm[2] : ��ָ���
                   aParm[3] : ��¼ID
                   aParm[4] : ��ID
                   aParm[5] : ��ID
 ** ����ֵ      �� 0:�ɹ� / -1:ʧ��
***************************************************************/
int _ufProPackSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char alRecs[6], alGrps[6], alFlds[6];
  short ilRc, ilRtncode, ilRecid, ilGrpid, ilFldid,ilMsglen;
/* ========== add by dgm 2001/10/16 ============ */
  short ilLen; 
  char  alTmp[iFLDVALUELEN+1];
/* ======= end of add by dgm 2001/10/16 ======== */

  _bmqVdebug(3,"S8612: [��������] _ufProPackSEP(%s,%s,%s,%s,%s,%s)",aParm[0],aParm[1],aParm[2],aParm[3],aParm[4],aParm[5]);

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
    _bmqVdebug(0,"S8615: ����swPacksep����");
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
    _bmqVdebug(0,"S8624: [����/��������] swPacksep()����,������=%d",ilRc);
    return(FAIL);
  }

  ilLen = strlen(alTmp);
  _ufNtoT(alTmp,ilLen,aResult);
/* ======= end of add by dgm 2001/10/16 ======== */

  _bmqVdebug(3,"S8627: [��������] _ufProPackSEP()������=0,���=%s",aResult);
  return(SUCCESS);
}
/**************************************************************
 ** ������      ��swPacksep
 ** ��  ��      ��ȡ�зָ�������ָ������ֵ
 ** ��  ��      ��
 ** ��������    ��2001.9.7
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��0:�ɹ� / -1:ʧ��
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
 ** ������      �� _ufPropGETSEP
 ** ��  ��      ��
 ** ��  ��      :
 ** ��������    �� 2001.9.7
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    �� agMsgbody : ����
                   lgMsgoffset : ��ǰָ��
 ** ��������    �� aParm[0] : ��ָ��� 
                   aParm[1] : �ָ�����
 ** ����ֵ      �� 0:�ɹ� / -1:ʧ��
***************************************************************/
int _ufPropGETSEP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  short i,j,ilLen,ilRc; 
  char  *alPos,*alPos2,*alTmp;
  char  alRecs[6];

  _bmqVdebug(3,"S8630: [��������] _ufPropGETSEP(%s,%s)",aParm[0],aParm[1]);

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
  
  _bmqVdebug(3,"S8633: [��������] _ufPropGETSEP()������=0,���=%s",aResult);
  return(SUCCESS);
}

/*********************************************************************
 ** ������      �� _ufPropGET
 ** ��  ��      ��
 ** ��  ��      :
 ** ��������    �� 2001.9.24
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    �� agMsgbody : ����
                   lgMsgoffset : ��ǰָ��
                   iMSGMAXLEN : ������󳤶�
 ** ��������    ��

 ** ����ֵ      �� 0:�ɹ� / -1:ʧ��
************************************************************************/
int _ufPropGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  _bmqVdebug(3,"S8642: [��������] _ufPropGET()");
  sprintf(aResult,"%hd",lgMsgoffset);
  _bmqVdebug(3,"S8645: [��������] _ufPropGET()������=0,���=%s",aResult);
  return(SUCCESS);
}
/********************************************************************
 ** ������      �� _ufPropNSP
 ** ��  ��      ��
 ** ��  ��      :
 ** ��������    �� 2001.9.24
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    �� agMsgbody : ����
                   lgMsgoffset : ��ǰָ��
 ** ��������    ��  
                   
 ** ����ֵ      �� 0:�ɹ� / -1:ʧ��
**********************************************************************/
int _ufPropNSP(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  char *alPos;
  char alResult[iFLDVALUELEN+1];
  /*short ilLen; delete by wh*/
  TYPE_SWITCH ilLen; /*add by wh*/
  
  _bmqVdebug(3,"S8648: [��������] _ufPropNSP(%s)",aParm[0]);

  ilLen = atoi(aParm[0]);
  /* add by zjj 2004.03.15 */
  if (ilLen >= iFLDVALUELEN)
  {
    _bmqVdebug(3,"S8649: [����/����] _ufPropNSP,ȡֵ����[%d]������ֵ��󳤶�[%d]",ilLen,iFLDVALUELEN);
    return(FAIL);
  }
  /* end add by zjj */
  alPos = agMsgbody;
  alPos = alPos + lgMsgoffset;
  
  memcpy(alResult,alPos,ilLen);
  alResult[ilLen] = 0x00;
  _ufNtoT(alResult,ilLen,aResult);
  lgMsgoffset = lgMsgoffset + ilLen;

  _bmqVdebug(3,"S8651: [��������] _ufPropNSP()������=0,���=%s",aResult);
  return(SUCCESS);
}



/* ============ add by dgm 2001/09/24 ========== */
/**************************************************************
 ** ������      �� _ufProBITGET
 ** ��  ��      �� ȡ�ַ����е�ĳλ
 ** ��  ��      ��
 ** ��������    �� 2001.9.24
 ** ����޸����ڣ� 
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] : �ַ���
                   aParm[1] : λ
 ** ����ֵ      �� 0:�ɹ� / -1:ʧ��
***************************************************************/
int _ufProBITGET(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  unsigned char clTmp, clMask = 1;
  char alStr[20];
  short ilRtncode, iLen, ilBit; 
  
  _bmqVdebug(3,"S8663: [��������] _ufProBITGET(%s,%s)",aParm[0],aParm[1]);
  
  _ufTtoN(aParm[0], alStr, &ilRtncode);
  _ufTtoN(aParm[1], aResult, &ilRtncode);
  ilBit = atoi(aResult);
  
  iLen = strlen(alStr);
  if (ilBit > 8 * iLen || ilBit <= 0)
  {
    _bmqVdebug(0,"S8666: [����/����] Ҫȡ��λ�����ڣ�");
    return(FAIL);
  }
  
  clMask = clMask << (8 - ((ilBit - 1) % 8 + 1));
  clTmp = clMask;
  
  if (alStr[(ilBit - 1) / 8] & clTmp)
    strcpy(aResult, "1");
  else 
    strcpy(aResult, "0");
    
  _bmqVdebug(3,"S8669: [��������] _ufProBITGET()������=0,���=%s",aResult);
  return(SUCCESS); 
   
}

#ifdef LIBXML

int swXmlinit()
{
  short ilRc;
  
  ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);
  if(ilRc)
  {
    if(ilRc == 3)  /*�Ѿ���ʼ��, ǿ���ͷ�*/
    {
      xmlFree();
      /* add by nh 20040607 */
      cgXmlInit = 0;
      /* end add */
      ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);
      if(ilRc) 
      {
        _bmqVdebug(0,"S8771: [��������] _ufProXMLINIT ����, %s",
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
 ** �� �� ����_ufProGETVALUE
 ** ��    �ܣ����ʱȡ��XMLԪ�ص�ֵ
 ** ��  ��      �� cgx
 ** ��������    �� 2002/8/28
 ** ����޸����ڣ� 2004/1/8
 ** ���ú�����
 ** ȫ�ֱ�����agMsgbody igMsgbodylen
 ** �������壺aXpath:XPATH��λ�ַ���
 ** �� �� ֵ��aValue:��ֵ
***************************************************************/
int _ufProGETVALUE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int  ilRc;
  short ilLen;
  char alXpath[iFLDVALUELEN];
  short ilFlag = 0;

  _ufTtoN(aParm[0],alXpath,&ilLen);
  _bmqVdebug(3,"S8783: [��������] _ufProGETVALUE (%s)",alXpath);
  if(iParmCount == 0)
  {
     _bmqVdebug(0,"S8784: [��������] _ufProGETVALUE ����, ����Ϊ��");
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
  _bmqVdebug(1,"S8791: [��������]_ufProGETVALUE ,��ʼ���ɹ�");
  /* modify by nh 20040419 
  ilRc = xmlGetValue(alXpath,aResult);*/
  ilRc = xmlGetNode(alXpath,aResult,NULL);
  if(ilRc)
  {
     _bmqDebug("S8790: [��������] _ufProGETVALUE ����, %s",
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
 ** �� �� ����_ufProGETVALUE
 ** ��    �ܣ����ʱȡ��XMLԪ�ص�ֵ
 ** ��  ��      �� cgx
 ** ��������    �� 2002/8/28
 ** ����޸����ڣ� 2004/1/8
 ** ���ú�����
 ** ȫ�ֱ�����agMsgbody igMsgbodylen
 ** �������壺aXpath:XPATH��λ�ַ���
 ** �� �� ֵ��aValue:��ֵ
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
    _bmqVdebug(0,"S0000: [����/����] �ű�[%s]���д���",alTmp);
    return(FAIL);
  }
  ilRc = _swExpress(alTmpexp,alTmp1,&ilResultlen);  
  if(ilRc != SUCCESS)
  {
    _bmqVdebug(1,"S0260: [��������] _ufProGETVALUEM ����ƽ̨���ʽ����");
    return(FAIL);
  }
  _bmqVdebug(1,"S0270: ����ƽ̨���ʽ�ɹ�,result=[%s]",alTmp1);
  for(i=0;i<strlen(alXpath);i++)
  {
    if(alXpath[i] == '[')
      break;
  }
  alXpath[i+1]=0;
  strcat(alXpath,alTmp1);
  strcat(alXpath,"]");
              
  _bmqVdebug(3,"S8783: [��������] _ufProGETVALUEM (%s)",alXpath);
  if(iParmCount == 0)
  {
     _bmqVdebug(0,"S8784: [��������] _ufProGETVALUEM ����, ����Ϊ��");
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
  _bmqVdebug(1,"S8791: [��������]_ufProGETVALUEM ,��ʼ���ɹ�");
  /* modify by nh 20040419 
  ilRc = xmlGetValue(alXpath,aResult);*/
  ilRc = xmlGetNode(alXpath,aResult,NULL);
  if(ilRc != SUCCESS && ilRc != 2)
  {
     _bmqDebug("S8790: [��������] _ufProGETVALUEM ����, %s",
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
 ** �� �� ����_ufProGETPROP()
 ** ��    �ܣ�ȡ��ָ���������Ե�ֵ
 ** ��  ��      �� cgx
 ** ��������    �� 2002/8/28
 ** ����޸����ڣ� 2004/1/8
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺aXpath:��λ�ַ���
 **           aProname:������
 ** �� �� ֵ��0(�ɹ�) / -1(ʧ��)
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
      _bmqVdebug(0,"S8789: [��������]_ufProGETVALUE ����, ��������");
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
      _bmqDebug("S8789: [��������]_ufProGETPROP ����, %s",
         xmlDescription(ilRc));
      return(FAIL);
   }
   if(ilFlag == 1) 
     swXmlfree();
   return(SUCCESS);
}


/**************************************************************
 ** ������      �� _ufProXMLINIT()
 ** ��  ��      �� ���ʱ����XML����
 ** ��  ��      �� cgx
 ** ��������    �� 2002/8/28
 ** ����޸����ڣ� 2004/1/8
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� aParm[0] :���ڵ���
 ** ����ֵ      �� 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLINIT(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  int ilRc;
  short ilLen; 
  char alBuf[iFLDVALUELEN];
  char alTmp[10];

  _bmqVdebug(3,"S8771: [��������]_ufProXMLINIT");

  if(iParmCount == 2)
  {
    _ufTtoN(aParm[1],alBuf,&ilLen);
    ilRc = xmlpackInit("1.0",alBuf);
    if(ilRc)
    {
      if(ilRc == 3)  /*�Ѿ���ʼ��, ǿ���ͷ�*/
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
    /*��Ӹ��ڵ�*/
    _ufTtoN(aParm[0],alBuf,&ilLen);
    strcpy(alTmp,"");
    ilRc = xmlAddNode("/",alBuf,alTmp);
    if(ilRc) 
    {
      _bmqVdebug(0,"S9346: [��������]_ufProXMLINIT() ����, %s",
        xmlDescription(ilRc));
      return(FAIL);
    }
  }
  else if(iParmCount == 1)
  {
    _ufTtoN(aParm[0],alBuf,&ilLen); /* �б���ͷ����Ҫƫ�� nh 20060504 */
    ilLen = atoi(alBuf);
    ilRc = xmlunpackInit(agMsgbody+ilLen,igMsgbodylen-ilLen);
    if(ilRc)
    {
      if(ilRc == 3)  /*�Ѿ���ʼ��, ǿ���ͷ�*/
      {
        xmlFree();
        /* add by nh 20040607 */
        cgXmlInit = 0;
        /* end add */
        ilRc = xmlunpackInit(agMsgbody+ilLen,igMsgbodylen-ilLen);
        if(ilRc) 
        {
          _bmqVdebug(0,"S8771: [��������] _ufProXMLINIT ����, %s",
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
      if(ilRc == 3)  /*�Ѿ���ʼ��, ǿ���ͷ�*/
      {
        xmlFree();
        /* add by nh 20040607 */
        cgXmlInit = 0;
        /* end add */
        ilRc = xmlunpackInit(agMsgbody,igMsgbodylen);
        if(ilRc) 
        {
          _bmqVdebug(0,"S8771: [��������] _ufProXMLINIT ����, %s",
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
  _bmqVdebug(1,"S9345: [���ý���]_ufProXMLINIT()����=0,���=%s",aResult);
  return(SUCCESS);
}

/**************************************************************
 ** ������      �� _ufProXMLFREE()
 ** ��  ��      �� �ͷ�XML��Դ
 ** ��  ��      �� cgx
 ** ��������    �� 2002/8/28
 ** ����޸����ڣ� 2004/1/8
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    �� 
 ** ����ֵ      �� 0-SUCCESS -1-FAIL
***************************************************************/
int _ufProXMLFREE(char aParm[][iFLDVALUELEN],short iParmCount,char *aResult)
{
  _bmqVdebug(3,"S8783: [��������] _ufProXMLFREE");
  xmlFree();
  aResult[0] = 0x00;
  cgXmlInit = 0;
  return(SUCCESS);
}
#endif

