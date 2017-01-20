#include "switch.h"
#include "swUsrfun.h"
#include "swSyscom.h"
#include "swNdbstruct.h"

Dataset sgDataset_match;
Dataset sgDataset_usrfun;

/* ◎  ●  ◇  ◆*/
int swGetfuninfo(char *aFuncname,Dataset slDataset,int *iParmcount);
int swGetkeyval(Dataset sDataset,int iRec,char paKeyval[][iFLDVALUELEN],int *iCount);
int swParseroute(char *aBuf,struct swt_sys_route_d * psSwt_sys_route_d);
short swParseTDF(char *aBuf, int iTrantype,struct swt_sys_fmt_d *psSwt_sys_fmt_d);
int swParseXMF(char *aBuf,struct swt_sys_fmt_d * psSwt_sys_fmt_d);
int OpenTable(keyrec *pKeys,short iKeyNum,char *aTableName,pDataset pSet);
int InitDataset(char * TableName,Dataset * psDataset);
void FreeDataset(Dataset * psDataset);

int swCheckparm(char *aBuf)
{
  char alParm[MAXVARNUM][iFLDVALUELEN];
  char alFunc[FUNCLEN];
  short ilParmcount;
  int ilCount;
  int ilRc,i;
  
  if (aBuf[0] == '\0') return(0);
  if ((strchr(aBuf,'(') == NULL) && (strchr(aBuf,')') == NULL)) return(0);
  ilParmcount=_swGetFuncAndParmN2T(aBuf,alFunc,alParm);
  _swTrim(alFunc);
  if (ilParmcount == -1) return(0);
  ilRc = swGetfuninfo(alFunc,sgDataset_usrfun,&ilCount);
  if (ilRc == -1) 
    return(-1);
  else if (ilRc == 100)
    printf("Warning:配置信息[%s]中的函数 [%s] 不存在,请检查!\n",aBuf,alFunc);

  /* 在SWUSRFUN.CFG 中参数个数配置为大于等于100的表示此函数参数为变参
   * 此值与100的差值表示此函数必须至少的参数个数
   * 如：SAD函数的参数个数为102,说明SAD函数的参数个数为变参,至少需要102-100=2
   * 个参数
　 */
  if(ilCount < 100)
  {
    if(ilCount != ilParmcount)
    printf("  Error:配置信息[%s]中的函数 [%s] 参数个数不正确,请检查!\n",aBuf,alFunc);
  }
  else if(ilCount >=100)
  {
    if(ilCount-100>ilParmcount)
    printf("  Error:配置信息[%s]中的函数 [%s] 参数个数不正确,请检查!\n",aBuf,alFunc);
  }

  for(i=0;i<ilParmcount;i++)
  {
    if(strchr(alParm[i],'(')!=NULL)
    {
      ilRc= swCheckparm(alParm[i]);
      if (ilRc) return(ilRc);
    }
  }
  return(0);
}

/***************************************************************
 ** 函数名:swGetfuninfo
 ** 功能:从配置文件SWUSRFUN.CFG获得给定函数的信息(参数个数等) 
 ** 作者:
 ** 建立日期: 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0:成功 / -1:失败
***************************************************************/
int swGetfuninfo(char *aFuncname,Dataset slDataset,int *iParmcount)
{
  int  i;
  short ilFindflag = 0; 
  
  for (i = 0;i<slDataset.iRecordNum;i++)
  {
    if (strcmp(slDataset.Records[i].Values[0].aValue,aFuncname) == 0)
    {
      ilFindflag = 1;
      *iParmcount = atoi(slDataset.Records[i].Values[2].aValue);	
      break;
    }
  }
  if (ilFindflag == 0)
    return(100);  /* not find this function */
  else
    return(0);
}

/***************************************************************
 ** 函数名: swCheckfldname
 ** 功能:check域名是否合法(对模式一) 
 ** 作者:
 ** 建立日期: 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: 
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0:成功 / -1:失败
***************************************************************/
int swCheckfldname(char *aTablename)
{
  int ilRc;
  char palKey[iKEYNUMBER][iFLDVALUELEN];
  char palFldname[iFLDNUMBER][iFLDNAMELEN];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  short i,j,ilFindflag = 0;
  Dataset slDataset;
  FILE *fp;
  char alFilename[120];

  ilRc =  InitDataset(aTablename, &slDataset);
  if (ilRc == 120) 
  {
    printf("  Error:无效的配置文件 [%s]!(头文件中找不到相关的信息)\n",aTablename);
    return(-1);
  }
  memset(alFilename,0x00,sizeof(alFilename));
  sprintf(alFilename,"%s/config/%s",getenv("SWITCH_CFGDIR"),aTablename);
  if ((fp = fopen(alFilename,"r")) == NULL)
  {
    if (errno == ENOENT)
      printf("  Error:文件　[%s] 不存在!\n",alFilename);
    else
      printf("  Error:打开文件 [%s] 出错 ilRc = %d !\n",alFilename,errno);
    FreeDataset(&slDataset);
    return(-999);   /* -999 打开文件出错 */
  }
  if (slDataset.iStoreMode == MODE2)
  {
    i = 0;
    while(1)
    {
      ilRc = _swGetitemmode1(fp,palKey,palFldname,palFldvalue);
      if (ilRc < 0) break;
      while(palFldname[i][0])
      {
      	ilFindflag = 0;
      	for(j = 0;j <slDataset.iFieldNum;j++)
      	{
          if (strcmp(palFldname[i],slDataset.Fields[j].aFieldName) == 0)
          {
            ilFindflag = 1;
            break;
          } 
        } 
        if (ilFindflag == 0)  printf("  Error:配置表中的域名 [%s] 为非法域名,请检查!\n",palFldname[i]);
        i++;
      }
    }
  }
  fclose(fp);
  FreeDataset(&slDataset);
  return(0);
}


/***************************************************************
 ** 函数名: swGetrecord_all
 ** 功能:读取配置表中的所有记录(模式一和模式二)
 ** 作者:
 ** 建立日期: 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0:成功 / -1:失败
***************************************************************/
int swGetrecord_all(keyrec *pKeys,short iKeynum,char *aTablename,Dataset *pDataset)
{
  int ilRc;

  ilRc = InitDataset(aTablename,pDataset);
  if (ilRc != 0) 
  {
    printf("  Error:无效的配置文件 [%s]!(头文件中找不到相关的信息)\n",aTablename);
    return(ilRc);
  }
  ilRc = OpenTable(pKeys,iKeynum,aTablename,pDataset);
  return(0);
}

/***************************************************************
 ** 函数名:swCheckkey 
 ** 功能:关键字验证(模式一和模式二)
 ** 作者:
 ** 建立日期: 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0:成功 / -1:失败
***************************************************************/
int swCheckkey(Dataset *pDataset)
{
  int ilRc,i,j,k,ilKeycount = 0;
  char palKeyval[5][iFLDVALUELEN];
  char palTmp[5][iFLDVALUELEN];
  Dataset slDataset;
  int ilTmp;

  memcpy(&slDataset,pDataset,sizeof(Dataset));
  for(i = 0; i<pDataset->iRecordNum; i++)
  {
    ilRc = swGetkeyval(slDataset,i,palKeyval,&ilKeycount);
    if (ilRc == 100)
    {
      printf("  Error:关键字未定义\n");
      return(-1);
    }
    for(j = 0; j<pDataset->iRecordNum;j++)
    {
      if (j==i) continue;
      ilRc = swGetkeyval(slDataset,j,palTmp,&ilTmp);
      for(k = 0;k<ilTmp;k++)
      {
        if (strcmp(palTmp[k],palKeyval[k]) != 0) break;  
      }  
      if (k == ilTmp)
      {
        printf("  Error:关键字重复  ");
        for(k = 0;k<ilTmp;k++)
        {
           printf("[%s] ",palKeyval[k]);
        }
        printf("\n");
      }
    }
  }
  return(0);
}

/***************************************************************
 ** 函数名:swCheckkey
 ** 功能:取配置文件中某条记录的关键字(模式一和模式二)
 ** 作者:
 ** 建立日期: 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0:成功 / -1:失败
***************************************************************/
int swGetkeyval(Dataset sDataset,int iRec,char paKeyval[][iFLDVALUELEN],int *iCount)
{
  int i;
  int ilKeycount = 0;
  
  for(i=0;i<sDataset.iFieldNum;i++)
  {
    if (sDataset.Fields[i].iFieldAttr & 0x02)
    {
      strcpy(paKeyval[i],sDataset.Records[iRec].Values[i].aValue);
      ilKeycount++;
      paKeyval[ilKeycount][0] = '\0';
    }
  }
  /* iCount 关键字的个数 */
  *iCount = ilKeycount;
  if (ilKeycount == 0)
    return(100);
  else
    return(0);  
}

int swCheckparm_config(Dataset *psDataset)
{
  int i,j;
  int ilRc;
  for (i = 0;i<psDataset->iRecordNum;i++)
  {
    for(j = 0;j<psDataset->iFieldNum;j++)
    {  
      if (psDataset->Fields[j].iFieldAttr & 0x04)
      {  
        ilRc = swCheckparm(psDataset->Records[i].Values[j].aValue);  
      } 
    }
  }
  return(0);
}

int swCheck_config(char *aFilename)
{
  Dataset slDataset,*pslDataset;
  int ilRc;


   
  printf("\n校验新晨交换平台系统配置文件[%s]\n",aFilename);
  /*
  memset(alFilename,0x00,sizeof(alFilename));
  sprintf(alFilename,"%s/config/%s",getenv("SWITCH_CFGDIR"),aFilename);
  if ((fp = fopen(alFilename,"r")) == NULL)
  {
    if (errno == ENOENT)
      printf("  Error:文件　[%s] 不存在!\n",alFilename);
    else
      printf("  Error:打开文件 [%s] 出错 ilRc = %d !\n",alFilename,errno);
    return(-999); 
  } 
  fclose(fp);  */   
  ilRc = swGetrecord_all((keyrec *)NULL,0,aFilename,&slDataset);
  if (ilRc != 0) 
  {
    printf("  Error:读入配置文件时出错[OpenTable]!\n\n");
    if (ilRc != 120) FreeDataset(&slDataset);
    return(-1);
  }
  pslDataset = &slDataset;
  ilRc = swCheckparm_config(pslDataset);
  ilRc = swCheckkey(pslDataset);
  ilRc = swCheckfldname(aFilename);
  FreeDataset(&slDataset);
  return(0);
}

int swCheck_config_all()
{
  int ilRc;
  FILE *pp;
  char alPath[80],alCmd[100],alFiletmp[51];

  memset(alPath,0x00,sizeof(alPath));
  sprintf(alPath,"%s/config",getenv("SWITCH_CFGDIR"));
  memset(alCmd,0x00,sizeof(alCmd));
  strcpy(alCmd,"ls ");
  strcat(alCmd,alPath);
  if ((pp = popen(alCmd, "r")) == NULL)
  {
    if (errno != ENOENT)
    {
       printf("  Error:[错误/系统调用] popen()函数,errno=%d,打开管道文件出错!",errno);
       return -1;
    }
    else
      return 0;
  }
  while (feof(pp) == 0)
  {
    memset(alFiletmp,0x00,sizeof(alFiletmp));
    fscanf(pp,"%s",alFiletmp);
    if ((alFiletmp[0] == '\0') || (strstr(alFiletmp,".CFG") == NULL)) continue;
    ilRc = swCheck_config(alFiletmp);
  }
  pclose(pp);
  return(0);
	
}

int swGetroute_d(int iRoute_group,struct swt_sys_route_d **psSwt_sys_route_d,int *iCount)
{
  int  ilCount_route_d = 0,ilScript = 0;
  struct swt_sys_route_d slSwt_sys_route_d;
  struct swt_sys_route_d *pslSwt_sys_route_d=NULL;
  char alFilename[120],alLine[300],*alTmp;
  FILE *fp;
 
  memset(alFilename,0x00,sizeof(alFilename));
  sprintf(alFilename,"%s/router/%d.SCR",getenv("SWITCH_CFGDIR"),iRoute_group);
  if ((fp = fopen(alFilename,"r")) == NULL)
  {
    if (errno == ENOENT)
      printf("  Error:文件　[%s] 不存在!\n",alFilename);
    else
      printf("  Error:打开文件 [%s] 出错,errno=%d !\n",alFilename,errno);
    return(-999);   
  }  
  while (fgets(alLine,sizeof(alLine),fp) != NULL)
  {
    _swTrim(alLine);
    if ((alLine[0] == '\0') || (alLine[0] == '#'))  continue;
    if ((alTmp = strchr(alLine,'#')) != NULL) *alTmp = '\0';
    if ((alLine[0] == '[') && (alLine[strlen(alLine)-1] == ']') && (strstr(alLine,"ROUTE") != NULL))
    {
      ilScript = 1;
      continue;
    }
    if (ilScript == 1)
    {
      if (swParseroute(alLine, &slSwt_sys_route_d) != 0)
      {
        printf("  Error:[错误/函数调用] swParseroute()函数,返回码=-1,解析文件出错!buf = [%s]\n",alLine);
        fclose(fp);
        return(-1);
      }
      ilCount_route_d++;
      if (ilCount_route_d == 1)
        pslSwt_sys_route_d = (struct swt_sys_route_d *)malloc(sizeof(struct swt_sys_route_d));         
      else
        pslSwt_sys_route_d = (struct swt_sys_route_d *)
        realloc(pslSwt_sys_route_d,ilCount_route_d * sizeof(struct swt_sys_route_d)); 
      if (psSwt_sys_route_d == NULL)
      {
        printf("  Error:内存分配出错 errno %d : %s!\n",errno,strerror(errno));
        fclose(fp);
        return(-1);
      }                            
      memcpy(&pslSwt_sys_route_d[ilCount_route_d - 1],
       &slSwt_sys_route_d,sizeof(struct swt_sys_route_d));     	
    }
  }
  *psSwt_sys_route_d = pslSwt_sys_route_d;
  *iCount = ilCount_route_d;
  fclose(fp);
  return(0);
}


/***************************************************************
 ** 函数名:swMatch
 ** 功能：在匹配文件中查找个定的组是否存在
 ** 作者:
 ** 建立日期: 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 0:成功 / -1:失败
***************************************************************/
int swMatch(int iGroup_id,Dataset slDataset)
{
  int ilFindflag = 0,i;

  for(i = 0;i < slDataset.iRecordNum;i++)
  {
    if (iGroup_id == atoi(slDataset.Records[i].Values[0].aValue))
    {
      ilFindflag = 1;
      break;	
    }
  }
  if (ilFindflag == 1)
    return(0);
  else
    return(100);
}  

int swGetgroup(char *aFilename,char *aExt,int *iGroup_id)
{
  int i;
  char *alTmp;
  char alFiletmp[100];
  
  strcpy(alFiletmp,aFilename);

  if ((alTmp = strstr(alFiletmp,aExt)) == NULL) return(-1);
  *alTmp = '\0';
  
  for(i = 0;i<strlen(alFiletmp);i++)
  {
    if ((alFiletmp[i] > '9') || (alFiletmp[i] < '0'))
    {
      return(-1);
    }
  }
  *iGroup_id = atoi(alFiletmp);
  return(0);
}


int swCheck_route(char *aFilename)
{
  int ilRc,ilCount = 0,i,j,ilRoute_grp;
  struct swt_sys_route_d *pslSwt_sys_route_d;
  char alTmp[2*iFLDVALUELEN+1];
  char alFile[120];
  char alCmd[100];
  
  printf("\n校验新晨交换平台路由配置文件[%s]\n",aFilename);
  ilRc = swGetgroup(aFilename,".SCR",&ilRoute_grp);
  if (ilRc !=0) 
  {
    printf("  Error:文件名称 [%s] 非法!\n",aFilename);
    return(-1);
  }
  memset(alFile,0x00,sizeof(alFile));
  memset(alCmd,0x00,sizeof(alCmd));
  sprintf(alFile,"%s/router/%s",getenv("SWITCH_CFGDIR"),aFilename);
  sprintf(alCmd,"swRouteYacc < %s",alFile);  
  
  system(alCmd); 
  
  pslSwt_sys_route_d = (struct swt_sys_route_d *) NULL;
  ilRc = swGetroute_d(ilRoute_grp,&pslSwt_sys_route_d,&ilCount);
  if (ilRc != 0) 
  {
    if (pslSwt_sys_route_d != NULL)
    {
      free(pslSwt_sys_route_d);
      pslSwt_sys_route_d = NULL;
    }
    return(-1);
  }
  for(i = 0;i<ilCount;i++)
  {
    /* 行号校验　*/
    for(j = 0;j<ilCount;j++)
    {
      if (i==j) continue;
      if (pslSwt_sys_route_d[i].route_id == pslSwt_sys_route_d[j].route_id)
        printf("  Error:行号重复:%d\n",pslSwt_sys_route_d[i].route_id);
    }
    /* GOTO 行号校验 */
    if (pslSwt_sys_route_d[i].oper_flag[0] == cGOTO)
    {
      for(j = 0;j<ilCount;j++)
      {
        if (pslSwt_sys_route_d[j].route_id == pslSwt_sys_route_d[i].next_id)
          break;
      }
      if (j == ilCount) printf("  Error:GOTO行号不存在:%d\n",pslSwt_sys_route_d[i].next_id);
    }
    /* 域表达式校验　*/
    if (pslSwt_sys_route_d[i].route_cond[0] != '\0')
    {
      if (pslSwt_sys_route_d[i].oper_flag[0] == 'W' || \
        pslSwt_sys_route_d[i].oper_flag[0] == 'S' || pslSwt_sys_route_d[i].oper_flag[0] == 'A')
        ilRc = swCheckparm(pslSwt_sys_route_d[i].route_cond);
      else
      {
        ilRc = _swExpT2N(pslSwt_sys_route_d[i].route_cond,alTmp);
        if (ilRc != 0)
          printf("  Error:调用函数_swExpT2N出错!str = %s ilRc = %d\n",pslSwt_sys_route_d[i].route_cond,ilRc);
        else
          ilRc = swCheckparm(alTmp);
      }
    }
    if (pslSwt_sys_route_d[i].q_target[0] != '\0')
    {
      ilRc = _swExpT2N(pslSwt_sys_route_d[i].q_target,alTmp);
      if (ilRc != 0)
        printf("  Error:调用函数_swExpT2N出错!str = %s ilRc = %d\n",pslSwt_sys_route_d[i].q_target,ilRc);  
      else
        ilRc = swCheckparm(alTmp);
    }
  }
  /* 匹配校验　*/
  ilRc = swMatch(ilRoute_grp,sgDataset_match);
  if (ilRc == 100) printf("Warning:路由对应的匹配信息不存在!\n");
  if (pslSwt_sys_route_d != NULL) free(pslSwt_sys_route_d);
  return(0);
}

int swCheck_route_all()
{
  int ilRc,ilCount = 0;
  FILE *pp;
  char alPath[80],alCmd[100],alFiletmp[51];

  memset(alPath,0x00,sizeof(alPath));
  sprintf(alPath,"%s/router",getenv("SWITCH_CFGDIR"));
  memset(alCmd,0x00,sizeof(alCmd));
  strcpy(alCmd,"ls ");
  strcat(alCmd,alPath);
  if ((pp = popen(alCmd, "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      printf("  Error:[错误/系统调用] popen()函数,errno=%d,打开管道文件出错!",errno);
       return -1;
    }
    else
      return 0;
  }
  while (feof(pp) == 0)
  {
    memset(alFiletmp,0x00,sizeof(alFiletmp));
    fscanf(pp,"%s",alFiletmp);
    if (alFiletmp[0] == '\0') continue;
    ilCount++;
    ilRc = swCheck_route(alFiletmp);
  }
  printf("\n********************* 共 [%d] 个路由校验完毕! ***************** \n\n",ilCount);
  pclose(pp);
  return(0);
}
/*modify by zcd 20141224
int swGetformat(int iFmt_group,struct swt_sys_fmt_grp *psSwt_sys_fmt_grp,\
struct swt_sys_fmt_d **psSwt_sys_fmt_d,int *iCount)*/
int swGetformat(long iFmt_group,struct swt_sys_fmt_grp *psSwt_sys_fmt_grp,\
struct swt_sys_fmt_d **psSwt_sys_fmt_d,int *iCount)
{
  /*modify by zcd 20141224
  char alFilename[120],alLine[300],*alTmp,*alTmp1;*/
  char alFilename[255],alLine[iiEXPRESSLEN],*alTmp,*alTmp1;
  FILE *fp;
  int ilFlag = 0,ilScript = 0,ilTran_type;
  int ilCount_fmt_d = 0 ;
  struct swt_sys_fmt_d slSwt_sys_fmt_d;
  struct swt_sys_fmt_d *pslSwt_sys_fmt_d = NULL;  

  memset(alFilename,0x00,sizeof(alFilename));
  /*modify by zcd 20141224
  sprintf(alFilename,"%s/format/%d.FMT",getenv("SWITCH_CFGDIR"),iFmt_group);*/
  sprintf(alFilename,"%s/format/%ld.FMT",getenv("SWITCH_CFGDIR"),iFmt_group);
  if ((fp = fopen(alFilename,"r")) == NULL)
  {
    if (errno == ENOENT)
      printf("  Error:文件　[%s] 不存在!\n",alFilename);
    else
      printf("  Error:打开文件 [%s] 出错 ilRc = %d !\n",alFilename,errno);
    return(-999);   
  }
  while (fgets(alLine,sizeof(alLine),fp) != NULL)
  {
    _swTrim(alLine);   
    if ((alLine[0] == '\0') || (alLine[0] == '#'))  continue;
    if ((alTmp = strchr(alLine,'#')) != NULL) *alTmp = '\0';
    if ((alLine[0] == '[') && (alLine[strlen(alLine)-1] == ']') && (strstr(alLine,"TDF") != NULL))
    {
      if (ilFlag == 0) 
      {
        printf("  TRANTYPE 未配置!\n");
        break;
      }
      ilScript = 1;
      continue;
    }
    if (ilScript == 0)
    {
      if ((alTmp = strchr(alLine,'=')) == NULL) continue;
      alTmp1 = alTmp + 1;
      *alTmp = '\0';
      _swTrim(alLine);
      _swTrim(alTmp1);
      if (strcmp(alLine,"TRANTYPE") == 0)  
      {
        psSwt_sys_fmt_grp->tran_type = atoi(alTmp1); 
        ilTran_type = atoi(alTmp1);
        ilFlag = 1;
      }
      if (strcmp(alLine,"MAC_CREATE") == 0) strcpy(psSwt_sys_fmt_grp-> mac_create,alTmp1);
      if (strcmp(alLine,"MAC_CHECK") == 0) strcpy(psSwt_sys_fmt_grp-> mac_check,alTmp1);
    }
    else 
    { 	
      if (ilTran_type != iXMF_IMF && ilTran_type != iIMF_XMF) 
      {     	
/*      	if (iFmt_group == 191) printf("  alLine = %s\n",alLine);  */
        if (swParseTDF(alLine,ilTran_type,&slSwt_sys_fmt_d) != 0)
        {
          printf("  Error:错误/函数调用] swParseTDF()函数,返回码=-1,解析文件出错!buf = [%s]\n",alLine);
          fclose(fp);
          return(-1);
        }
      }
      else
      {
        if (swParseXMF(alLine,&slSwt_sys_fmt_d) != 0)
        {
          printf("  Error:[错误/函数调用] swParseXMF()函数,返回码=-1,解析文件出错! buf = [%s]\n",alLine);
          fclose(fp);
          return(-1);
        } 
      }

      ilCount_fmt_d ++;
      if (ilCount_fmt_d == 1)
        pslSwt_sys_fmt_d = (struct swt_sys_fmt_d *)malloc(sizeof(struct swt_sys_fmt_d));
      else
        pslSwt_sys_fmt_d = (struct swt_sys_fmt_d *)
          realloc(pslSwt_sys_fmt_d,ilCount_fmt_d * (sizeof(struct swt_sys_fmt_d)));
      if (pslSwt_sys_fmt_d == NULL)
      {
        swVdebug(0,"S0010: [错误/其它] malloc / realloc Error!");
        fclose(fp);
        return(-1);
      } 
             	      
      memcpy(&pslSwt_sys_fmt_d[ilCount_fmt_d - 1],
       &slSwt_sys_fmt_d,sizeof(struct swt_sys_fmt_d)); 
    }
  }
  *psSwt_sys_fmt_d = pslSwt_sys_fmt_d;
  *iCount = ilCount_fmt_d;  
  fclose(fp);
  return(0);
}

int swCheck_format(char *aFilename)
{
  int ilRc,ilCount = 0,i,j;
  struct swt_sys_fmt_d *pslSwt_sys_fmt_d;
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  char alTmp[2*iFLDVALUELEN+1];
  /*modify by zcd 20141224
  int  ilFmt_group;*/
  long ilFmt_group;
  char alFile[120],alCmd[100];
  
  printf("\n校验新晨交换平台格式转换配置文件[%s]\n",aFilename);
  ilRc = swGetgroup(aFilename,".FMT",&ilFmt_group);
  if (ilRc != 0)
  {
    printf("   Error:文件名称[%s]不合法\n",aFilename);
    return(-1);	
  }
  pslSwt_sys_fmt_d = (struct swt_sys_fmt_d *)NULL;
  ilRc = swGetformat(ilFmt_group,&slSwt_sys_fmt_grp,&pslSwt_sys_fmt_d,&ilCount);
  if (ilRc != 0) 
  {
    if (pslSwt_sys_fmt_d != NULL) 
    {
      free(pslSwt_sys_fmt_d);
      pslSwt_sys_fmt_d = NULL;
    }
    return(-1);
  }
  memset(alFile,0x00,sizeof(alFile));
  memset(alCmd,0x00,sizeof(alCmd));
  
  sprintf(alFile,"%s/format/%s",getenv("SWITCH_CFGDIR"),aFilename);
  if (slSwt_sys_fmt_grp.tran_type == iXMF_IMF || slSwt_sys_fmt_grp.tran_type == iIMF_XMF)
    sprintf(alCmd,"swXMFYacc < %s",alFile);
  else
    sprintf(alCmd,"swFormatYacc < %s",alFile);  
  system(alCmd); 
  
  for(i = 0;i<ilCount;i++)
  {
    if (slSwt_sys_fmt_grp.tran_type == iXMF_IMF && slSwt_sys_fmt_grp.tran_type == iIMF_XMF)
    {
      for(j = 0;j<ilCount;j++)
      {
        if (i==j) continue;
        if (pslSwt_sys_fmt_d[i].id == pslSwt_sys_fmt_d[j].id)
          printf("  Error:行号重复:%d\n",pslSwt_sys_fmt_d[i].id);
      }
      if (strcmp(pslSwt_sys_fmt_d[i].fmt_flag,"A") == 0)
      {
        for(j = 0;j<ilCount;j++)
        {
          if (pslSwt_sys_fmt_d[i].fld_id == pslSwt_sys_fmt_d[j].id)
            break;
        }
        if (j == ilCount) printf("  Error:GOTO行号不存在:%d\n",pslSwt_sys_fmt_d[i].fld_id);
      }
    }
    if (pslSwt_sys_fmt_d[i].fmt_cond[0] != '\0')
    {
      if (pslSwt_sys_fmt_d[i].fmt_flag[0] == 'J' || \
        pslSwt_sys_fmt_d[i].fmt_flag[0] == 'N' || pslSwt_sys_fmt_d[i].fmt_flag[0] == 'O')
      	 ilRc = swCheckparm(pslSwt_sys_fmt_d[i].fmt_cond);
      else
      {
        ilRc = _swExpT2N(pslSwt_sys_fmt_d[i].fmt_cond,alTmp);
        if (ilRc != 0)
          printf("  Error:调用函数_swExpT2N出错!str = %s ilRc = %d\n",pslSwt_sys_fmt_d[i].fmt_cond,ilRc);
        else
          ilRc = swCheckparm(alTmp);
      }
    }
    if (pslSwt_sys_fmt_d[i].fld_express[0] != '\0') 
    {
      ilRc = _swExpT2N(pslSwt_sys_fmt_d[i].fld_express,alTmp);
      if (ilRc != 0)
        printf("  Error:调用函数_swExpT2N出错!str = %s ilRc = %d\n",pslSwt_sys_fmt_d[i].fld_express,ilRc);  
      else
        ilRc = swCheckparm(alTmp);  
    }
  }
  if (pslSwt_sys_fmt_d != NULL)  free(pslSwt_sys_fmt_d);
  ilRc = swMatch(ilFmt_group,sgDataset_match);
  if (ilRc == 100) printf("Warning:格式对应的匹配信息不存在!\n"); 
  return(0);
}

int swCheck_format_all()
{
  int ilRc,ilCount = 0;
  FILE *pp;
  char alPath[80],alCmd[100],alFiletmp[51];
  memset(alPath,0x00,sizeof(alPath));
  sprintf(alPath,"%s/format",getenv("SWITCH_CFGDIR"));
  memset(alCmd,0x00,sizeof(alCmd));
  strcpy(alCmd,"ls ");
  strcat(alCmd,alPath);
  if ((pp = popen(alCmd, "r")) == NULL)
  {
    if (errno != ENOENT)
    {
       printf("  Error:[错误/系统调用] popen()函数,errno=%d,打开管道文件出错!",errno);
       return -1;
    }
    else
      return 0;
  }
  while (feof(pp) == 0)
  {
    memset(alFiletmp,0x00,sizeof(alFiletmp));
    fscanf(pp,"%s",alFiletmp);
    if (alFiletmp[0] == '\0') continue;
    ilCount++;
    ilRc = swCheck_format(alFiletmp);
  }
  printf ("\n********************** 共[%d] 个格式校验完毕! ****************\n\n",ilCount);  
  pclose(pp);
  return(0);
}

int swCheck_init()
{
  printf("使用方法 : swCheckcfg config|format|router|all\n");
  printf("  config  [filename]  平台系统配置文件校验(config目录下)\n");
  printf("  format  [filename]  格式转换配置文件校验(format目录下)\n");
  printf("  router  [filename]  路由脚本配置文件校验(router目录下)\n");
  printf("  all                 所有配置文件校验\n");
  return(0);
}

int main(int argc,char **argv)
{
  int ilRc;	
	
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strncpy(agDebugfile, "swCheckcfg.debug", sizeof(agDebugfile));

  if (argc < 2)
  { 
    swCheck_init();
    return(-1);
  }

  ilRc = swGetrecord_all((keyrec *)NULL,0,"SWUSRFUN.CFG",&sgDataset_usrfun);
  if (ilRc != 0) 
  {
    printf("读配置文件[SWUSRFUN.CFG]时出错[OpenTable]!\n\n");
    if (ilRc != 120) FreeDataset(&sgDataset_usrfun);
    return(-1);
  }  
  if (strcmp(argv[1],"config") == 0)
  {
    if (argc == 3)
      swCheck_config(argv[2]);
    else
      swCheck_config_all();
    FreeDataset(&sgDataset_usrfun);      
    return(0);  
  }
  if (strcmp(argv[1],"format") == 0)
  {
    ilRc = swGetrecord_all((keyrec *)NULL,0,"SWMATCH_FMT.CFG",&sgDataset_match);
    if (ilRc != 0) 
    {
      printf("读配置文件[SWMATCH_FMT.CFG]时出错[OpenTable]!\n\n");
      if (ilRc != 120) FreeDataset(&sgDataset_match);
      FreeDataset(&sgDataset_usrfun);
      return(-1);
    }
    if (argc == 3)
      swCheck_format(argv[2]);
    else
      swCheck_format_all(); 

    FreeDataset(&sgDataset_match);
    FreeDataset(&sgDataset_usrfun);  
    return(0);
  }
  if (strcmp(argv[1],"router") == 0)
  {
    ilRc = swGetrecord_all((keyrec *)NULL,0,"SWMATCH_ROUTE.CFG",&sgDataset_match);
    if (ilRc != 0) 
    {
      printf("读入配置文件[SWMATCH_ROUTE.CFG]时出错[OpenTable]!\n\n");
      if (ilRc != 120) FreeDataset(&sgDataset_match);
      FreeDataset(&sgDataset_usrfun);
      return(-1);
    }  	
    if (argc == 3)
      swCheck_route(argv[2]);
    else
      swCheck_route_all();
    FreeDataset(&sgDataset_usrfun);      
    FreeDataset(&sgDataset_match);    
    return(0);
  }
  
  if (strcmp(argv[1],"all") == 0)
  {
    swCheck_config_all();
    ilRc = swGetrecord_all((keyrec *)NULL,0,"SWMATCH_FMT.CFG",&sgDataset_match);
    if (ilRc != 0) 
    {
      printf("读配置文件[SWMATCH_FMT.CFG]时出错[OpenTable]!\n\n");
      if (ilRc != 120) FreeDataset(&sgDataset_match);
      FreeDataset(&sgDataset_usrfun);
      return(-1);
    }
    swCheck_format_all();
    ilRc = swGetrecord_all((keyrec *)NULL,0,"SWMATCH_ROUTE.CFG",&sgDataset_match);
    if (ilRc != 0) 
    {
      printf("读入配置文件[SWMATCH_FMT.CFG]时出错[OpenTable]!\n\n");
      if (ilRc != 120) FreeDataset(&sgDataset_match);
      FreeDataset(&sgDataset_usrfun);
      return(-1);
    }    
    swCheck_route_all();
    FreeDataset(&sgDataset_usrfun);
    FreeDataset(&sgDataset_match);
    return(0);
  }  
  printf("无效的参数!\n");
  swCheck_init();
  FreeDataset(&sgDataset_usrfun);
  return(0); 
}
