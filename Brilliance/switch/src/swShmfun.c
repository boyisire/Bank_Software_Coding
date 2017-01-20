#include "switch.h"

#include "swNdbstruct.h"
#include "swShm.h"

int swParseXMF(char *aBuf,struct swt_sys_fmt_d * psSwt_sys_fmt_d);
static int strtoupper(char *aBuf);
static int swGetrevmode(char *aBuf,char *aBuf_find,int *ilMode);
extern int _swExpN2T(char *aNstr,char *aTstr);
extern int _swExpT2N(char *aTstr,char *aNstr);
static int swGetimfname(char *aSrc,char *aName,char *aArray);
int swExpSign(char *aBuf);
/*del by zcd 20141222
int swCheckfmt(short **piFmt,char *aBuf);  
***end of del by zcd 20141222*/
/*add by zcd 20141222*/
int swCheckfmt(long **piFmt,char *aBuf); /* add by nh 20021009 */
/*end of add by zcd 20141222*/
int swCheckroute(short **piRoute,char *aBuf);/* add by nh 20021009 */
/******************************************************************/
/*          取配置文件记录                                        */
/*  short swGetcfgvalue(char *aFile, char *aKey, char *aResult);  */
/*          读共享内存初始化参数                                  */
/*  short swInitcfgload(struct swt_sys_config * psSwt_sys_config)   */
/*          读一条记录（用于混合格式存放的配置文件）              */
/*  short swGetitemmode3(FILE *fp,                                */
/*          int *piSegflag, char paKey[][iFLDVALUELEN],           */
/*          char paFldname[][iFLDNAMELEN],                        */
/*          char paFldvalue[][iFLDVALUELEN], char *aBuf)          */
/*          解析TDF语句                                           */
/*  short swParseTDF(char *aBuf,short iTran_type,                 */
/*          struct *psSwt_sys_fmt_d);                             */
/*          解析路由语句                                          */
/*  short swParseroute(char *aBuf,struct *psSwt_sys_fmt_d);       */
/*  short swShm_swt_sys_imf()                                     */
/*  short swShm_swt_sys_queue()                                   */
/*  short swShm_swt_sys_task()                                    */
/*  short swShm_swt_sys_8583()                                    */
/*  short swShm_swt_sys_8583E()                                    */
/*  short swShm_swt_sys_code()                                    */
/*  short swShm_swt_sys_config()                                  */
/*  short swShm_swt_sys_other()                                   */
/*  short swShm_swt_sys_fmt()                                     */
/*  short swShm_swt_sys_imf()     格式二模板                      */
/*  short swShm_swt_sys_task()    格式一模板                      */
/*  short swShm_swt_sys_fmt()                                     */
/*  short swShm_swt_sys_route()                                   */
/*  以下4个函数为HASH的                                           */
/*  short swShm_swt_sys_fmt_m()                                   */
/*  short swShm_swt_sys_fmt_g()                                   */
/*  short swShm_swt_sys_route_m()                                 */
/*  short swShm_swt_sys_tran_ctrl()                               */
/******************************************************************/


/***************************************************************
 ** 函数名      : swGetcfg
 ** 功  能      : 从配置文件中读取指定键值的值
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swGetcfgvalue(char *aFile, char *aKey, char *aResult)
{
  char alBuf[257], *alTmp;
  FILE *fp;
  
  if ((fp = fopen(aFile, "r")) == NULL) return -1;
  while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
  {
    _swTrim(alBuf);
    if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
    if ((alTmp = strchr(alBuf,'#')) != NULL) *alTmp = '\0';
    if ((alTmp = strchr(alBuf, '=')) == NULL) continue;
    strcpy(aResult,alTmp + 1);
    _swChgvalue(aResult);
    *alTmp = '\0';
    _swTrim(alBuf);
    if (strcmp(alBuf, aKey) == 0)
    {
      fclose(fp); 
      return 0;
    }
  }
  fclose(fp);
  return -1;
} 

/***************************************************************
 ** 函数名      : swInitcfgload
 ** 功  能      : 读共享内存初始化参数
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swInitcfgload(struct swt_sys_config * psSwt_sys_config)
{
  char alBuf[257], alResult[101],*alTmp;
  int i;
  FILE *fp;

  memset(psSwt_sys_config,0x00,sizeof(struct swt_sys_config));
  
  if ((fp = _swFopen("config/SWCONFIG.CFG", "r")) == NULL) return -1;
  i = 0;
  while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
  {
    _swTrim(alBuf);
    if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
    if ((alTmp = strchr(alBuf,'#')) != NULL) *alTmp = '\0';
    _swTrim(alBuf);
    if ((alBuf[0] == '[') && (alBuf[strlen(alBuf) -1] == ']')) continue;
    if ((alTmp = strchr(alBuf, '=')) == NULL) continue;
    strcpy(alResult,alTmp + 1);
    _swChgvalue(alResult);
    *alTmp = '\0';
    _swTrim(alBuf);
    if (strcmp(alBuf,"SHMKEY") == 0)
    {
      psSwt_sys_config->iShmkey = atoi(alResult);
      i ++;
    }  
    if (strcmp(alBuf,"MAXTRANLOG") == 0)
    {
      psSwt_sys_config->iMaxtranlog = atoi(alResult);
      i ++;
    }  
    if (strcmp(alBuf,"MAXPROCLOG") == 0)
    {
      psSwt_sys_config->iMaxproclog = atoi(alResult);
      i ++;
    }  
    if (strcmp(alBuf,"MAXSAFLOG") == 0)
    {
      psSwt_sys_config->iMaxsaflog = atoi(alResult);
      i ++;
    }
    if (strcmp(alBuf,"SHMWAITTIME") == 0)
    {
      psSwt_sys_config->iShmwaittime = atoi(alResult);
      i ++;
    } 

    if (strcmp(alBuf,"SYSMON_IP") == 0)
    {
      strcpy( psSwt_sys_config->sysmonaddr,alResult);
    } 

    if (strcmp(alBuf,"SYSMON_PORT") == 0)
    {
       psSwt_sys_config->sysmonport = atoi(alResult);
    }

    if (strcmp(alBuf,"MSG_NUM") == 0)
    {
       psSwt_sys_config->msg_num = atoi(alResult);
    }

    if (strcmp(alBuf,"ECHOTEST_NUM") == 0)
    {
       psSwt_sys_config->echotest_num = atoi(alResult);
    }

    if (strcmp(alBuf,"ECHOTEST_INTER") == 0)
    {
       psSwt_sys_config->echotest_inter = atoi(alResult);
    }

    if (strcmp(alBuf,"LOGCLEAR_COUNT") == 0)
    {
       psSwt_sys_config->log_clear_count = atoi(alResult);
    }

    if (strcmp(alBuf,"TRACE_TERM") == 0)
    {
      strcpy( psSwt_sys_config->trace_term,alResult);
    } 

    if (strcmp(alBuf,"LOG_INTER") == 0)
    {
       psSwt_sys_config->log_inter = atoi(alResult);
    }

    if (strcmp(alBuf,"SAF_INTER") == 0)
    {
       psSwt_sys_config->saf_inter = atoi(alResult);
    }

/* ========= begin of add by dgm 2002.3.19 ========== */
    if (strcmp(alBuf,"REREV_INTER") == 0)
    {
       psSwt_sys_config->rerev_inter = atol(alResult);
    }
/* ========== end of add by dgm 2002.3.19 =========== */

    if (strcmp(alBuf,"FTPMODE") == 0)
    {
      strcpy( psSwt_sys_config->ftpmode,alResult);
    } 

    if (strcmp(alBuf,"FTPOVERTIME") == 0)
    {
       psSwt_sys_config->ftpovertime = atoi(alResult);
    }

    if (strcmp(alBuf,"QPACKSIZE") == 0)
    {
       psSwt_sys_config->qpacksize = atoi(alResult);
    }

    if (strcmp(alBuf,"QOVERTIME") == 0)
    {
       psSwt_sys_config->qovertime = atoi(alResult);
    }

    if (strcmp(alBuf,"QRESENDNUM") == 0)
    {
       psSwt_sys_config->qresendnum = atoi(alResult);
    }

    if (strcmp(alBuf,"MAXTRANNUM") == 0)
    {
       psSwt_sys_config->iMaxtrannum = atol(alResult);
    }

  }
  if (i < 5) 
  {
    fclose(fp);
    return -1;
  }
  fclose(fp);
  return 0;
} 

/***************************************************************
 ** 函数名      : swGetitemmode3
 ** 功  能      : 读一条记录（用于混合格式存放的配置文件）
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swGetitemmode3(FILE *fp, int *piSegflag, char paKey[][iFLDVALUELEN],
  char paFldname[][iFLDNAMELEN], char paFldvalue[][iFLDVALUELEN], char *aBuf)
{
  static char palKey[iKEYNUMBER][iFLDVALUELEN];
  static int ilRec = 0, ilSegflag = 0, ilSegflag_old = 0;
  short i,j,ilPos,ilPos_old;
  char alBuf[iFLDVALUELEN];
  char *alTmp;
  int ilRc;

  if ((ilRec > 0) && (ilSegflag != ilSegflag_old))
    ilSegflag_old = ilSegflag;

  i = 0;
  paFldname[i][0] = '\0'; /* add by qy 2001.09.21 */
  while (1)
  {
    if (fgets(alBuf, sizeof(alBuf), fp) != NULL)
    {
/* ========= begin of add by dgm 2002.3.19 ========== */
    ilRc = swMacrorepl(alBuf);
    if(ilRc)
    {
      swVdebug(0,"S5005: [错误/其它] 宏定义替换失败! [buf=%s]",alBuf);
      return(FAIL);
    }
/* ========== end of add by dgm 2002.3.19 =========== */
      _swTrim(alBuf);
      if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
      if ((alTmp = strchr(alBuf, '#')) != NULL) *alTmp = '\0';
      _swTrim(alBuf);

      /* add by qy 2001.09.18 */
      if (ilRec == 0)
      {
         ilSegflag = 1;
         ilSegflag_old = ilSegflag; 
         ilRec ++;
      }
      /* end of add by qy 2001.09.18 */

      if ((alBuf[0] == '[') && (alBuf[strlen(alBuf) - 1] == ']'))
      {
        ilSegflag = 1;
        if (strchr(alBuf, ',') != NULL)
          ilSegflag = 2;
        if ((strstr(alBuf, "TDF") != NULL)||(strstr(alBuf, "ROUTE") != NULL))
          ilSegflag = 3;
        if (strstr(alBuf, "XMF") != NULL)
          ilSegflag = 3;
      
        /* add by qy 2001.09.18 */
        if (ilSegflag == 1) ilRec --;
        /* end of add by qy 2001.09.18 */ 
        
        for (j=0; j<iKEYNUMBER; j++)
          strcpy(paKey[j],palKey[j]);
          
        alBuf[strlen(alBuf) - 1] = ',';
        j = 0;
        ilPos = ilPos_old = 1;
        while (alBuf[ilPos] != '\0')
        {
          if (alBuf[ilPos] == ',' )
          {
            memcpy(palKey[j],alBuf + ilPos_old,ilPos - ilPos_old);
            palKey[j][ilPos - ilPos_old] = '\0';
            _swTrim(palKey[j]);
            ilPos_old = ilPos + 1;
            j ++;
          }  
          ilPos ++;
        }  
        if (ilRec > 0)
        {
          ilRec ++;
          *piSegflag = ilSegflag_old;
          
          return 0;
        }
        ilRec ++;
        ilSegflag_old = ilSegflag;
      }
      else
      {
        if ((ilSegflag == 1)||(ilSegflag == 2))
        {
          if ((alTmp = strchr(alBuf, '=')) == NULL) continue;
          strcpy(paFldvalue[i],alTmp + 1);
          *alTmp = '\0';
          strcpy(paFldname[i],alBuf);

          _swChgvalue(paFldvalue[i]);
          _swChgvalue(paFldname[i]);

          _swTrim(paFldvalue[i]);
          _swTrim(paFldname[i]);
          paFldname[i+1][0] = '\0';
          i ++;
        }
        else if (ilSegflag == 3 || ilSegflag == 4)
        {
          strcpy(aBuf,alBuf);
          *piSegflag = ilSegflag_old;
          return 0;
        }
        else
          return -1;
      } 
    }
    else
      break;
  }
  
  ilRec = 0;
  ilSegflag = ilSegflag_old = 0;
  *piSegflag = ilSegflag_old;

/*
  k ++;
  if (k == 2)
  {
    k = 0;
    return (-1);
  }
  else
*/
    return 0;
}  

/***************************************************************
 ** 函数名      : swParseTDF
 ** 功  能      : 解析TDF脚本
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swParseTDF(char *aBuf, int iTrantype, 
                 struct swt_sys_fmt_d *psSwt_sys_fmt_d)
{
  int ilRc;
  char *alTmp1, *alTmp2,*alTmp3;
  char alTmpexp[2 * iFLDVALUELEN + 1]; /* add by zjj 2001.11.29 */
    char alTmp[2 * iFLDVALUELEN + 1]; /* add by zjj 2004.02.28  */

  memset((char *)psSwt_sys_fmt_d, 0x00, sizeof(struct swt_sys_fmt_d));
  _swTrim(aBuf);
  
  memset(alTmp,0x00,sizeof(alTmp)); /* add by zjj 2004.02.28 */ 
  strcpy(alTmp,aBuf);   /* add by zjj 2004.02.28 */

  alTmp1 = strchr(aBuf, '=');
  if (alTmp1 == NULL) 
  {
    if( memcmp(aBuf,"TDF",3) && memcmp(aBuf,"tdf",3) )
      return -1;
    alTmp1 = strchr(aBuf, '(');
    if (alTmp1 == NULL)  return -1;
    alTmp2 = alTmp1 + 1;
    alTmp3 = strchr(alTmp2, ')');
    if (alTmp3 == NULL)  return -1;
    *alTmp3 = '\0';
  }
  else
  /* add by xujun end */
    alTmp2 = alTmp1 + 1;
  *alTmp1 = '\0';
  alTmp1 = aBuf;
  _swTrim(alTmp1);
  _swTrim(alTmp2);
  if (strlen(alTmp2) > 100) 
  {
    swVdebug(0,"S5010: [错误/其它] fld_express > 100");
     return -1;
  }

  /* next 2 row add by nh 2002.7.3 */
  ilRc = swExpSign(alTmp2);
  if (ilRc != SUCCESS) return(ilRc);
  
  /* add by zjj 2001.11.29  */
  ilRc = _swExpN2T(alTmp2,alTmpexp);
  if (ilRc != SUCCESS) return(ilRc);
  strcpy(psSwt_sys_fmt_d->fld_express, alTmpexp);
  /* end add by zjj 2001.11.29 */

  /* strcpy(psSwt_sys_fmt_d->fld_express, alTmp2);*/ /* delete by zjj */
  if (*alTmp1 == '*')
  {
     if (*(alTmp1+1) == '&')
     {
       alTmp2 = alTmp1 + 2;
       _swTrim(alTmp2);
       /*
       if ((iTrantype != 103) && (iTrantype != 105))
       {
         if (strlen(alTmp2) > iFLDNAMELEN)
         {
           swVdebug(0,"S5015: [错误/其它] imf_name > %d",iFLDNAMELEN); 
            return -1;
         }
         strcpy(psSwt_sys_fmt_d->imf_name, alTmp2);
       }
       else
       {
         psSwt_sys_fmt_d->fld_id = atol(alTmp2);  //modified by mzg 20150428 atoi->atol
       }  */  /* delete by zjj 2002.05.14 for qy suggestion */
       /* add by zjj 2002.05.14 for qy suggestion */
       if (strlen(alTmp2) > iFLDNAMELEN)
       {
         swVdebug(0,"S5020: [错误/其它] [%s] imf_name > %d",alTmp2,iFLDNAMELEN); 
          return -1;
       }
       strcpy(psSwt_sys_fmt_d->imf_name, alTmp2); 
       /* end add by zjj */      
       strcpy(psSwt_sys_fmt_d->fmt_flag, "3");
     }
     else
     {
       alTmp2 = alTmp1 + 1;
       _swTrim(alTmp2);
       if ((iTrantype != 103) && (iTrantype != 105))
       {
         if (strlen(alTmp2) > iFLDNAMELEN)
         {
           swVdebug(0,"S5025: [错误/其它] [%s] imf_name > %d", alTmp2,iFLDNAMELEN);
            return -1;
         }
         strcpy(psSwt_sys_fmt_d->imf_name, alTmp2);
       }
       else
       {
         psSwt_sys_fmt_d->fld_id = atol(alTmp2);//modified by mzg 20150428 atoi->atol
       }
       strcpy(psSwt_sys_fmt_d->fmt_flag, "1");
     }
     return 0;
  }
  if (*alTmp1 == '&')
  {
     alTmp2 = alTmp1 + 1;
     _swTrim(alTmp2);
/*     if ((iTrantype != 103) && (iTrantype != 105))
     {
       if (strlen(alTmp2) > iFLDNAMELEN)
       {
         swVdebug(0,"S5030: [错误/其它] imf_name > %d", iFLDNAMELEN);
          return -1;
       }
       strcpy(psSwt_sys_fmt_d->imf_name, alTmp2);
     }
     else
     {
       psSwt_sys_fmt_d->fld_id = atol(alTmp2); //modified by mzg 20150428 atoi->atol
     }  */  /* delete by zjj 2002.05.14 for qy suggestion */
     /* add by zjj 2002.05.14 for qy suggestion */
     if (strlen(alTmp2) > iFLDNAMELEN)
     {
       swVdebug(0,"S5035: [错误/其它] [%s] imf_name > %d", alTmp2, iFLDNAMELEN);
        return -1;
     }
     strcpy(psSwt_sys_fmt_d->imf_name, alTmp2);    
     /* end add by zjj */
     strcpy(psSwt_sys_fmt_d->fmt_flag, "2");
     return 0;
  }
  /* add by xujun 2001.09.07 */
  if ( ( memcmp(alTmp1,"TDF",3) == 0) || (memcmp(alTmp1,"tdf",3) == 0) )
  {
    strcpy(psSwt_sys_fmt_d->fmt_flag, "C");
    return 0;
  }
  /* add by xunjun end */
  
  if (alTmp1 != NULL)
  {
     alTmp2 = alTmp1;
     _swTrim(alTmp2);
     if ((iTrantype != 103) && (iTrantype != 105))
     {
       if (strlen(alTmp2) > iFLDNAMELEN)
       {
         swVdebug(0,"S5040: [错误/其它] [%s] imf_name > %d", alTmp2, iFLDNAMELEN);
          return -1;
       }
       strcpy(psSwt_sys_fmt_d->imf_name, alTmp2);
     }
     else
     {
       psSwt_sys_fmt_d->fld_id = atol(alTmp2); //modified by mzg 20150428 atoi->atol
     }
     strcpy(psSwt_sys_fmt_d->fmt_flag, "0");
     return 0;
  }
  /* add by zjj 2004.02.28 */
  swVdebug(0,"S5041: [错误/其它][%s] 脚本解析错误!", alTmp);    
  return -1;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_imf
 ** 功  能      : 读内部标准域列表[SWIMF.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_imf()
{
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc, ilCount;
  struct swt_sys_imf slSwt_sys_imf;
  FILE *fp;
  char alTmpexp[2 * iFLDVALUELEN + 1]; /* add by zjj 2001.11.29 */

  if ((fp = _swFopenlist("config/SWIMF.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5045: _swFopenlist(SWIMF.CFG): errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_imf = 0;
      return 0;
    }  
  }

  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0) break;

      ilCount ++;
      memset((char *)&slSwt_sys_imf,0x00,sizeof(struct swt_sys_imf));
      strcpy(slSwt_sys_imf.imf_name,palFldvalue[0]);
      strcpy(slSwt_sys_imf.imf_type,palFldvalue[1]);
      slSwt_sys_imf.imf_len = atoi(palFldvalue[2]);
      slSwt_sys_imf.imf_dec = atoi(palFldvalue[3]);

      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(palFldvalue[4]);
      if (ilRc != SUCCESS) return(ilRc);
  
      /* add by zjj 2001.11.29  */
      ilRc = _swExpN2T(palFldvalue[4],alTmpexp);
      if (ilRc != SUCCESS) return(ilRc);
      strcpy(slSwt_sys_imf.imf_check, alTmpexp);
      /* end add by zjj 2001.11.29  */

      /* strcpy(slSwt_sys_imf.imf_check, palFldvalue[4]);*/  /* delete by zjj */
      slSwt_sys_imf.imf_id = atoi(palFldvalue[5]);

      if (ilCount == 0) 
        return -1;
      else if (ilCount == 1)
      {  
        sgLcmidx.psSwt_sys_imf = (struct swt_sys_imf *)
      	  	malloc(ilCount * sizeof(struct swt_sys_imf));
        if ((sgLcmidx.psSwt_sys_imf) == NULL) return -1;
      } 
      else
      {
        sgLcmidx.psSwt_sys_imf = (struct swt_sys_imf *)
      	  realloc(sgLcmidx.psSwt_sys_imf, ilCount*sizeof(struct swt_sys_imf));
        if ((sgLcmidx.psSwt_sys_imf) == NULL) return -1;
      }
      memcpy((char *)&((sgLcmidx.psSwt_sys_imf)[ilCount-1]), 
        		(char *)&slSwt_sys_imf,sizeof(struct swt_sys_imf));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  
  sgLcmidx.iCount_swt_sys_imf = ilCount;  		
  return 0;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_queue
 ** 功  能      : 读应用端[SWAPPS.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_queue()
{
  char alFldvalue[iFLDVALUELEN];
  char palKey[iKEYNUMBER][iFLDVALUELEN];
  char palFldname[iFLDNUMBER][iFLDNAMELEN];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int  i,j,ilRc, ilCount;
  char alDeftmp[7];
  short ilDeflen;
  struct swt_sys_queue sSwt_sys_queue;
  FILE *fp;
  char alTmpexp[2 * iFLDVALUELEN + 1];  /* add by zjj 2001.11.29 */

  if ((fp = _swFopenlist("config/SWAPPS.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5050: _swFopenlist(SWAPPS.CFG): errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_queue = 0;
      return 0;
    }  
  }

  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode1(fp,palKey,palFldname,palFldvalue);
      if (ilRc < 0) break;

      ilCount ++;
      memset(&sSwt_sys_queue,0x00,sizeof(struct swt_sys_queue));

      if (ilCount == 1)
        sgLcmidx.psSwt_sys_queue = (struct swt_sys_queue *)
        		malloc(ilCount * sizeof(struct swt_sys_queue));
      else
        sgLcmidx.psSwt_sys_queue = (struct swt_sys_queue *)
        	realloc(sgLcmidx.psSwt_sys_queue,ilCount * sizeof(struct swt_sys_queue));
      if (sgLcmidx.psSwt_sys_queue == NULL) return -1;
    
      sSwt_sys_queue.q_id = atoi(palKey[0]);
    
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"PORTID",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.q_port_id = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"APPNAME",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.q_name, alFldvalue);
      
      /*  
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"q_statu",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.q_statu, alFldvalue);
      */
      sSwt_sys_queue.q_statu[0] = '1';
    
      /*  
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"down_cause",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.down_cause, alFldvalue);
      */
    
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MSGFORMAT",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.msg_format = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"OVERTIME",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.rev_overtime = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"REVNUM",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.rev_num = atoi(alFldvalue);
 
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"UNPACKEXP",alFldvalue);
      if (ilRc == 0)
      {    
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
    
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.tc_unpack, alTmpexp);
        /* strcpy(sSwt_sys_queue.tc_unpack, alFldvalue); */ /* delete by zjj */
      }
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"PACKEXP",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.tc_pack, alTmpexp);
      
   
   
        /* strcpy(sSwt_sys_queue.tc_pack, alFldvalue); */ /* delete by zjj */
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TRANTYPE",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.tran_type, alFldvalue);
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"DEFRCD",alFldvalue);
      if (ilRc == 0)
      {
        ilRc = swHextoasc(alFldvalue,alDeftmp,&ilDeflen);
        if (ilRc)
        {
          swVdebug(0,"S5055: [错误/函数调用] swHextoasc()函数,返回码=%d,记录分隔符定义错!",ilRc);
           return(FAIL);
        }
        memset(alFldvalue,0x00,7);
        memcpy(alFldvalue,alDeftmp,ilDeflen);
        strcpy(sSwt_sys_queue.def_rs, alFldvalue);
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"DEFGRP",alFldvalue);
      if (ilRc == 0)
      {
        ilRc = swHextoasc(alFldvalue,alDeftmp,&ilDeflen);
        if (ilRc)
        {
          swVdebug(0,"S5060: [错误/函数调用] swHextoasc()函数,返回码=%d,组分隔符定义错!",ilRc);
          return(FAIL);
        }
        memset(alFldvalue,0x00,7);
        memcpy(alFldvalue,alDeftmp,ilDeflen);
        strcpy(sSwt_sys_queue.def_gs, alFldvalue);
      }
     
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"DEFFLD",alFldvalue);
      if (ilRc == 0)
      {
        ilRc = swHextoasc(alFldvalue,alDeftmp,&ilDeflen);
        if (ilRc)
        {
          swVdebug(0,"S5065: [错误/函数调用] swHextoasc()函数,返回码=%d,域分隔符定义错!",ilRc);
          return(FAIL);
        }
        memset(alFldvalue,0x00,7);
        memcpy(alFldvalue,alDeftmp,ilDeflen);
        strcpy(sSwt_sys_queue.def_fs, alFldvalue);
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"REV_NOGOOD",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.rev_nogood, alFldvalue);
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"REV_NOM",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.rev_nomrev, alFldvalue);
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"ISOTABID",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.isotab_id = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"BITMAP_TYPE",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.bitmap_type, alFldvalue);
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MSGPRIOR",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.msg_prio, alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MSGHEADEXP",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        /* add by zjj 2001.11.29  */
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.key_express, alTmpexp);
        /* end add by zjj 2001.11.29 */

        /* strcpy(sSwt_sys_queue.key_express, alFldvalue); */ /* delete by zjj */
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"COMTYPE",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.comm_type, alFldvalue);
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"COMATTR",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.comm_attr, alFldvalue);
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"APPRESUEXP",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        /* add by zjj 2001.11.29  */
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.resu_express, alTmpexp);
        /* end add by zjj 2001.11.29 */

        /* strcpy(sSwt_sys_queue.resu_express, alFldvalue); */ /* del by zjj */
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"CODETYPE",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_queue.code_type, alFldvalue);
      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"REVRESUEXP",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        /* add by zjj 2001.11.29  */
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.resu_revexp, alTmpexp);
        /* end add by zjj 2001.11.29 */
 
        /* strcpy(sSwt_sys_queue.resu_revexp, alFldvalue); */ /* del by zjj */
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MAXTRAN",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.max_tran = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TRANSHM_MAX",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.transhm_max = atol(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"REJEGRP",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.reje_grp = atoi(alFldvalue);

/* ========= add by qy 2001/08/22 ============= */

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TRIGGER_FREQ",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.trigger_freq = atol(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TRIGGER_TERM",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        /* add by zjj 2001.11.29  */
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.trigger_term, alTmpexp);
        /* end add by zjj 2001.11.29 */

        /* strcpy(sSwt_sys_queue.trigger_term, alFldvalue); */ /* del by zjj */
      }
  
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"SETDOWN_OVERTIME",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        /* add by zjj 2001.11.29  */
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.setdown_overtime, alTmpexp);
        /* end add by zjj 2001.11.29 */

        /* strcpy(sSwt_sys_queue.setdown_overtime, alFldvalue); */ /* del by zjj */
      }
      
      /* sSwt_sys_queue.port_status = 1; */ 
      sSwt_sys_queue.port_status = 0xff;  
      
/* ========= end of add by qy 2001/08/22 ====== */

/* ========= added by ph 2002/01/24 ============= */

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"E8583TAB_ID",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.e8583tab_id = atoi(alFldvalue);
        
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"BITMAP_LEN",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.bitmap_len = atoi(alFldvalue);
           
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"BITMAP_UNPK",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
     
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.bitmap_unpk, alTmpexp);
      }
  
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"BITMAP_PACK",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.bitmap_pack, alTmpexp);
      }
    
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MAC_EXPRESS",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.mac_express, alTmpexp);
      }
      
/* ========= end of being addedd by ph 2002/01/24 ====== */

/* === begin of added by fzj at 2002.02.28 === */      
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TRANBEGIN_MAX",alFldvalue);
      if (ilRc == 0)
      {
        sSwt_sys_queue.tranbegin_max = atoi(alFldvalue);
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TRANING_MAX",alFldvalue);
      if (ilRc == 0)
      {
        sSwt_sys_queue.traning_max = atoi(alFldvalue);
      }
/* === end of added by fzj at 2002.02.28 === */ 

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"REV_EXPRESS",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc);
      
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_queue.rev_express, alTmpexp);
      }

/* ========= end of being addedd by ph 2002/01/24 ====== */
      
      /* add by gengling at 2015.04.13 begin PSBC_V1.0 */
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MAX_FAIL_COUNT",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.max_fail_count = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"FAIL_CON_TRY_INTER",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_queue.fail_connect_try_interval = atoi(alFldvalue);
      /* add by gengling at 2015.04.13 end PSBC_V1.0 */

      memcpy(&(sgLcmidx.psSwt_sys_queue[ilCount - 1]),
    	  	&sSwt_sys_queue,sizeof(struct swt_sys_queue));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  

/* sort */
  for (i=1;i<=ilCount-1;i++)
  {
    for (j=0;j<ilCount-1;j++)
    {
      if (sgLcmidx.psSwt_sys_queue[j].q_id > sgLcmidx.psSwt_sys_queue[j+1].q_id)
      {
        memcpy(&sSwt_sys_queue,&sgLcmidx.psSwt_sys_queue[j],sizeof(struct swt_sys_queue));
        memcpy(&sgLcmidx.psSwt_sys_queue[j],&sgLcmidx.psSwt_sys_queue[j+1],sizeof(struct swt_sys_queue));
        memcpy(&sgLcmidx.psSwt_sys_queue[j+1],&sSwt_sys_queue,sizeof(struct swt_sys_queue));
      }
    }
  }
  sgLcmidx.iCount_swt_sys_queue = ilCount;
  return 0;
}

/* === begin of added by fzj at 2002.02.28 === */
/***************************************************************
 ** 函数名      : swShm_swt_sys_matchport
 ** 功  能      : 交易端口关联表[SWMATCH_PORT.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2002/02/28
 ** 最后修改日期: 2002/02/28
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_matchport()
{
  char alPort[11];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  char *alPos,*alPos1;
  int ilRc, i,j,ilCount,ilSortflag;
  struct swt_sys_matchport slSwt_sys_matchport;
  FILE *fp;

  if ((fp = _swFopenlist("config/SWMATCH_PORT.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5070: _swFopenlist(SWMATCH_PORT.CFG): errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_matchport = 0;
      return 0;
    }  
  }

  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0) break;
      if (palFldvalue[2][0] == '\0') continue;

      alPos = palFldvalue[2];
      while(1)
      {
        alPos1 = strchr(alPos,',');
        if(alPos1)
        {
          *alPos1 = '\0';
          strcpy(alPort,alPos);
          alPos = alPos1 + 1;
        } 
        else
          strcpy(alPort,alPos);

        ilCount ++;
        memset((char *)&slSwt_sys_matchport,0x00,sizeof(struct swt_sys_matchport));
        slSwt_sys_matchport.qid = atoi(palFldvalue[0]);
        strcpy(slSwt_sys_matchport.trancode,palFldvalue[1]);
        slSwt_sys_matchport.port = atoi(alPort);

        if (ilCount == 0) 
          return -1;
        else if (ilCount == 1)
        {  
          sgLcmidx.psSwt_sys_matchport = (struct swt_sys_matchport *)
      	  	malloc(ilCount * sizeof(struct swt_sys_matchport));
          if ((sgLcmidx.psSwt_sys_matchport) == NULL) return -1;
        } 
        else
        {
          sgLcmidx.psSwt_sys_matchport = (struct swt_sys_matchport *)
          	realloc(sgLcmidx.psSwt_sys_matchport, ilCount*sizeof(struct swt_sys_matchport));
          if ((sgLcmidx.psSwt_sys_matchport) == NULL) return -1;
        }

        memcpy((char *)&((sgLcmidx.psSwt_sys_matchport)[ilCount-1]), 
      	  	(char *)&slSwt_sys_matchport,sizeof(struct swt_sys_matchport));

        if(!alPos1) break;
      }
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  

/* sort */
  for (i=1;i<=ilCount-1;i++)
  {
    for (j=0;j<ilCount-1;j++)
    {
      ilSortflag = 0;
      if (sgLcmidx.psSwt_sys_matchport[j].qid > sgLcmidx.psSwt_sys_matchport[j+1].qid)
        ilSortflag = 1;
      else if (sgLcmidx.psSwt_sys_matchport[j].qid == sgLcmidx.psSwt_sys_matchport[j+1].qid)
      {
      	ilRc=strcmp(sgLcmidx.psSwt_sys_matchport[j].trancode,sgLcmidx.psSwt_sys_matchport[j+1].trancode);
      	if (ilRc > 0)
        { 
      	  if (strcmp("*",sgLcmidx.psSwt_sys_matchport[j+1].trancode)) ilSortflag = 1;
        }
      	else if (ilRc < 0)
        {
      	  if (!strcmp("*",sgLcmidx.psSwt_sys_matchport[j].trancode)) ilSortflag = 1;
        }
      }	  
      if (ilSortflag)
      {
        memcpy(&slSwt_sys_matchport,&sgLcmidx.psSwt_sys_matchport[j],sizeof(struct swt_sys_matchport));
        memcpy(&sgLcmidx.psSwt_sys_matchport[j],&sgLcmidx.psSwt_sys_matchport[j+1],sizeof(struct swt_sys_matchport));
        memcpy(&sgLcmidx.psSwt_sys_matchport[j+1],&slSwt_sys_matchport,sizeof(struct swt_sys_matchport));
      }
    }
  }
  sgLcmidx.iCount_swt_sys_matchport = ilCount; 
  return 0;
}
/* === end of added by fzj at 2002.02.28 === */

/***************************************************************
 ** 函数名      : swShm_swt_sys_task
 ** 功  能      : 读任务进程[SWTASK.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode1
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_task()
{
  char palKey[iKEYNUMBER][iFLDVALUELEN];
  char alFldvalue[iFLDVALUELEN];
  char palFldname[iFLDNUMBER][iFLDNAMELEN];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  short ilRc,ilCount;
  struct swt_sys_task sSwt_sys_task;
  FILE *fp;
  char alTmpexp[2 * iFLDVALUELEN + 1]; /* add by zjj 2001.11.29 */

  if ((fp = _swFopenlist("config/SWTASK.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5075: _swFopen(SWTASK.CFG): errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_task = 0;
      return 0;
    }  
  }

  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode1(fp,palKey,palFldname,palFldvalue);
      if (ilRc < 0) break;

      ilCount ++;
      memset(&sSwt_sys_task,0x00,sizeof(struct swt_sys_task));

      if (ilCount == 1)
        sgLcmidx.psSwt_sys_task = (struct swt_sys_task *)
      	  	malloc(ilCount * sizeof(struct swt_sys_task));
      else
        sgLcmidx.psSwt_sys_task = (struct swt_sys_task *)
      	  realloc(sgLcmidx.psSwt_sys_task,ilCount * sizeof(struct swt_sys_task));
      if (sgLcmidx.psSwt_sys_task == NULL) return -1;
    
      strcpy(sSwt_sys_task.task_name,palKey[0]);

/* added by fzj at 2002.4.16 */
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TASKDESC",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_task.task_desc,alFldvalue);
/* end */
    
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TASKFILE",alFldvalue);
      if (ilRc == 0)
      {
        /* modify by zjj 2001.11.20 默认路径支持 */
        _swTrim(alFldvalue);
        if (alFldvalue[0] == '/') 
          strcpy(sSwt_sys_task.task_file,alFldvalue);
        else
          sprintf(sSwt_sys_task.task_file,"%s/bin/%s",getenv("SWITCH_DIR"),\
            alFldvalue);
        /* end modify by zjj 2001.11.20 */
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"QID",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.q_id = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"STARTID",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.start_id = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"STARTWAIT",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.start_wait = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"STOPID",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.stop_id = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"STOPWAIT",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.stop_wait = atoi(alFldvalue);

      /*
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"pid",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.pid = atoi(alFldvalue);
      */
    
      /*
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"task_status",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_task.task_status,alFldvalue);
      */
    
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"RESTART_FLAG",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_task.restart_flag,alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"KILLID",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.kill_id = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TASKATTR",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_task.task_attr,alFldvalue);

      /*
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"start_time",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.start_time = atoi(alFldvalue);
      */
    
      /*
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"restart_num",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.restart_num = atoi(alFldvalue);
      */
    
      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"RESTART_MAX",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.restart_max = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TASKPRIOR",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.task_priority = atoi(alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TASKUSE",alFldvalue);
      if (ilRc == 0)
        strcpy(sSwt_sys_task.task_use,alFldvalue);

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TASKTIMER",alFldvalue);
      if (ilRc == 0)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alFldvalue);
        if (ilRc != SUCCESS) return(ilRc); 
      
        /* add by zjj 2001.11.29 */
        ilRc = _swExpN2T(alFldvalue,alTmpexp);
        if (ilRc != SUCCESS) return(ilRc);
        strcpy(sSwt_sys_task.task_timer,alTmpexp);
        /* end by zjj 2001.11.29 */
      
        /* strcpy(sSwt_sys_task.task_timer,alFldvalue); */ /* delete by zjj */
      }

      ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TASKFLAG",alFldvalue);
      if (ilRc == 0)
        sSwt_sys_task.task_flag = atoi(alFldvalue);

      memcpy(&(sgLcmidx.psSwt_sys_task[ilCount - 1]),
    	  	&sSwt_sys_task,sizeof(struct swt_sys_task));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  
  sgLcmidx.iCount_swt_sys_task = ilCount;
  return 0;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_8583
 ** 功  能      : 读ISO 8583[SWISO8583.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_8583()
{
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc,ilCount;
  struct swt_sys_8583 slSwt_sys_8583;
  FILE *fp;
  char alTmpexp[2 * iFLDVALUELEN + 1];  /* add by zjj 2001.11.29 */

  if ((fp = _swFopenlist("config/SWISO8583.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5080: _swFopenlist(SWISO8583.CFG),errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_8583 = 0;
      return 0;
    }  
  }
    
  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0) break;

      ilCount ++;
      memset((char *)&slSwt_sys_8583,0x00,sizeof(struct swt_sys_8583));
      slSwt_sys_8583.tab_id = atoi(palFldvalue[0]);
      slSwt_sys_8583.fld_id = atoi(palFldvalue[1]);
      strcpy(slSwt_sys_8583.fld_name, palFldvalue[2]);
      slSwt_sys_8583.fld_type = atoi(palFldvalue[3]);
      slSwt_sys_8583.fld_attr = atoi(palFldvalue[4]);
      slSwt_sys_8583.fld_len = atoi(palFldvalue[5]);

      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(palFldvalue[6]);
      if (ilRc != SUCCESS) return(ilRc);
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(palFldvalue[6],alTmpexp);
      if (ilRc != 0) return(ilRc);
      strcpy(slSwt_sys_8583.fld_rule, alTmpexp);
      /* end add by zjj 2001.11.29 */

      /* strcpy(slSwt_sys_8583.fld_rule, palFldvalue[6]); */ /* del by zjj */
    
      if (ilCount == 1)
        sgLcmidx.psSwt_sys_8583 = (struct swt_sys_8583 *)
      	  	malloc(ilCount * sizeof(struct swt_sys_8583));
      else
        sgLcmidx.psSwt_sys_8583 = (struct swt_sys_8583 *)
      	  realloc(sgLcmidx.psSwt_sys_8583, ilCount*sizeof(struct swt_sys_8583));
      if ((sgLcmidx.psSwt_sys_8583) == NULL) return -1;

      memcpy((char *)&((sgLcmidx.psSwt_sys_8583)[ilCount-1]), 
        		(char *)&slSwt_sys_8583,sizeof(struct swt_sys_8583));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }
  sgLcmidx.iCount_swt_sys_8583 = ilCount;  		
  return 0;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_8583E
 ** 功  能      : 读ISO 8583E[SWISO8583E.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_8583E()
{
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc,ilCount,i;
  struct swt_sys_8583E slSwt_sys_8583E;
  FILE *fp;
  char alTmpexp[2 * iFLDVALUELEN + 1]; 
  char alTmp[iEXPRESSLEN];
  char alType[20];
  static long lgType = 0;

  if ((fp = _swFopenlist("config/SWISO8583E.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5085: _swFopenlist(SWISO8583E.CFG): errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_8583E = 0;
      return 0;
    }  
  }
    
  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0) break;
    
      lgType=0;
      ilCount ++;
      memset((char *)&slSwt_sys_8583E,0x00,sizeof(struct swt_sys_8583E));
      slSwt_sys_8583E.tab_id = atoi(palFldvalue[0]);
      slSwt_sys_8583E.fld_id = atoi(palFldvalue[1]);
      strcpy(slSwt_sys_8583E.fld_name, palFldvalue[2]);
      strcpy(alTmp, palFldvalue[3]);
      for(i=strlen(palFldvalue[3]);i>=0;i--)
      {
       if( alTmp[i] == '|' || i == 0 )
       {  
         if (i)
         {
           strcpy(alType,&alTmp[i+1]);
         }
         else strcpy(alType,alTmp);
       
         if(strncmp(alType,"EA",2) == 0)
           lgType|=EA;
         else if(strncmp(alType,"ED",2) == 0)
           lgType|=ED;
         else if(strncmp(alType,"EP",2) == 0)
           lgType|=EP;
         else if(strncmp(alType,"ES",2) == 0)
           lgType|= ES;
         else if(strncmp(alType,"EB",2) == 0)
           lgType|=EB;
         else if(strncmp(alType,"EX",2) == 0)
           lgType|=EX;   
         else if(strncmp(alType,"EZ",2) == 0)
           lgType|=EZ;  
         else if(strncmp(alType,"EV",2) == 0)
           lgType|=EV; 
         else 
           swVdebug(0,"S5090: 不可辨识的域类型 [%s] ", alType);
         alTmp[i]='\0';
        }
      }
      slSwt_sys_8583E.fld_type = lgType;
      slSwt_sys_8583E.fld_len = atoi(palFldvalue[4]);
      slSwt_sys_8583E.fld_len2 = atoi(palFldvalue[5]);

      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(palFldvalue[6]);
      if (ilRc != SUCCESS) return(ilRc);
    
      ilRc = _swExpN2T(palFldvalue[6],alTmpexp);
      if (ilRc != 0) return(ilRc);
      strcpy(slSwt_sys_8583E.fld_lenunpk, alTmpexp);
    
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(palFldvalue[7]);
      if (ilRc != SUCCESS) return(ilRc);
    
      ilRc = _swExpN2T(palFldvalue[7],alTmpexp);
      if (ilRc != 0) return(ilRc);
      strcpy(slSwt_sys_8583E.fld_lenpack, alTmpexp);
    
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(palFldvalue[8]);
      if (ilRc != SUCCESS) return(ilRc);
    
      ilRc = _swExpN2T(palFldvalue[8],alTmpexp);
      if (ilRc != 0) return(ilRc);
      strcpy(slSwt_sys_8583E.fld_valunpk, alTmpexp);

      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(palFldvalue[9]);
      if (ilRc != SUCCESS) return(ilRc);
    
      ilRc = _swExpN2T(palFldvalue[9],alTmpexp);
      if (ilRc != 0) return(ilRc);
      strcpy(slSwt_sys_8583E.fld_valpack, alTmpexp);

      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(palFldvalue[10]);
      if (ilRc != SUCCESS) return(ilRc);
    
      ilRc = _swExpN2T(palFldvalue[10],alTmpexp);
      if (ilRc != 0) return(ilRc);
      strcpy(slSwt_sys_8583E.fld_rule, alTmpexp);
     
      if (ilCount == 1)
        sgLcmidx.psSwt_sys_8583E = (struct swt_sys_8583E *)
      	  	malloc(ilCount * sizeof(struct swt_sys_8583E));
      else
        sgLcmidx.psSwt_sys_8583E = (struct swt_sys_8583E *)
      	  realloc(sgLcmidx.psSwt_sys_8583E, ilCount*sizeof(struct swt_sys_8583E));
      if ((sgLcmidx.psSwt_sys_8583E) == NULL) return -1;

      memcpy((char *)&((sgLcmidx.psSwt_sys_8583E)[ilCount-1]), 
       		(char *)&slSwt_sys_8583E,sizeof(struct swt_sys_8583E));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  
  sgLcmidx.iCount_swt_sys_8583E = ilCount;  		
  return 0;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_code
 ** 功  能      : 读信息代码[SWCODE.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_code()
{
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc,ilCount;
  struct swt_sys_code slSwt_sys_code;
  FILE *fp;

  if ((fp = _swFopenlist("config/SWCODE.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5095: _swFopenlist(SWCODE.CFG),errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_code = 0;
      return 0;
    }  
  }
    
  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0) break;

      ilCount ++;
      memset(&slSwt_sys_code,0x00,sizeof(struct swt_sys_code));

      slSwt_sys_code.msg_code = atol(palFldvalue[0]);
      strcpy(slSwt_sys_code.msg_desc, palFldvalue[1]);
      slSwt_sys_code.msg_action = atoi(palFldvalue[2]);
      slSwt_sys_code.msg_q = atoi(palFldvalue[3]);

      if (ilCount == 1)
        sgLcmidx.psSwt_sys_code = (struct swt_sys_code *)
      	  	malloc(ilCount * sizeof(struct swt_sys_code));
      else
        sgLcmidx.psSwt_sys_code = (struct swt_sys_code *)
      	  realloc(sgLcmidx.psSwt_sys_code, ilCount*sizeof(struct swt_sys_code));
      if ((sgLcmidx.psSwt_sys_code) == NULL) return -1;

      memcpy((char *)&((sgLcmidx.psSwt_sys_code)[ilCount-1]), 
       		(char *)&slSwt_sys_code,sizeof(struct swt_sys_code));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }
  sgLcmidx.iCount_swt_sys_code = ilCount;  		
  return 0;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_config
 ** 功  能      : 读数据库表[swt_sys_config]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swInitcfgload
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_config()
{
  short ilRc;

  sgLcmidx.psSwt_sys_config = (struct swt_sys_config *)
	malloc(sizeof(struct swt_sys_config));

  if ((sgLcmidx.psSwt_sys_config) == NULL) return(FAIL);

  memset(&(sgLcmidx.psSwt_sys_config[0]),0x00,
    sizeof(struct swt_sys_config));
    
  ilRc = swInitcfgload(&(sgLcmidx.psSwt_sys_config[0]));

  if (ilRc)
  {
    return(FAIL);
  }

  sgLcmidx.iCount_swt_sys_config = 1;

  return 0;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_other
 ** 功  能      : 读外部节点[SWHOSTS.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_other()
{
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc,ilCount;
  struct swt_sys_other slSwt_sys_other;
  FILE *fp;

  if ((fp = _swFopenlist("config/SWHOSTS.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5100: _swFopenlist(SWHOSTS.CFG),errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_other = 0;
      return 0;
    }  
  }
    
  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0) break;

      ilCount ++;
      memset(&slSwt_sys_other,0x00,sizeof(struct swt_sys_other));

      slSwt_sys_other.other_id = atoi(palFldvalue[0]);
      strcpy(slSwt_sys_other.other_name, palFldvalue[1]);
      slSwt_sys_other.local_q = atoi(palFldvalue[2]);
      slSwt_sys_other.other_grp = atoi(palFldvalue[3]);
      slSwt_sys_other.other_q = atoi(palFldvalue[4]);

      if (ilCount == 1)
        sgLcmidx.psSwt_sys_other = (struct swt_sys_other *)
      	  	malloc(ilCount * sizeof(struct swt_sys_other));
      else
        sgLcmidx.psSwt_sys_other = (struct swt_sys_other *)
      	  realloc(sgLcmidx.psSwt_sys_other, ilCount*sizeof(struct swt_sys_other));
      if ((sgLcmidx.psSwt_sys_other) == NULL) return -1;

      memcpy((char *)&((sgLcmidx.psSwt_sys_other)[ilCount-1]), 
        		(char *)&slSwt_sys_other,sizeof(struct swt_sys_other));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }
  sgLcmidx.iCount_swt_sys_other = ilCount;
  return 0;
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_fmt
 ** 功  能      : 读TDF[*.FMT]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode1
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_fmt()
{
  char *alTmp;
  /*modify by zcd 20141224
  char alPath[100], alFile[80], alBuf[257], alBuf_rc[257];*/
  char alPath[100], alFile[80], alBuf[257], alBuf_rc[iEXPRESSLEN+1];
  char palKey[iKEYNUMBER][iFLDVALUELEN];
  char palFldname[iFLDNUMBER][iFLDNAMELEN];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  char alFldvalue[iFLDVALUELEN];
  char alFiletmp[80],alFiletmp2[80];
  int ilCount_fmt_m, ilCount_fmt_g, ilCount_fmt_d,ilScriptline;
  int i, j, k, ilRc, ilSegflag, ilTrantype, ilSwapflag, ilFlag;
  int pre_alloc=0;
  /*del by zcd 20141222
  short ilFmtgrp_id;  
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long ilFmtgrp_id; 
  /*mend of add by zcd 20141218*/
  short ilPriority;
  struct shm_sys_fmt_m slShm_sys_fmt_m;
  FILE *fp, *pp;
  char alTmp1[5];
  int n;
  
  /* buf */
  struct buf
  {
    char cFlag;
    struct swt_sys_fmt_d sSwt_sys_fmt_d;
  };

  /* tag */
  struct tag
  {
    char cTag;
    short iBufid;
    struct tag *psNxtptr;
  };

  char alCond[101],alSwitch[101];
  struct buf *pslBuf;
  struct tag *pslTag=NULL,*pslNewptr,*pslTmpptr;
  short ilScriptcount=0;
  char clFindflag; 
  char alTmpexp[2 * iFLDVALUELEN + 1]; /* add by zjj 2001.11.29 */
  static short *pilFmt=NULL;      /* add by nh 20021009 */
  
  if ((pp = _swPopen("format","r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5105: [错误/其它] popen()函数,errno=%d,打开管道文件出错!",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_fmt_m = 0;
      sgLcmidx.iCount_swt_sys_fmt_g = 0;
      sgLcmidx.iCount_swt_sys_fmt_d = 0;
      return 0;
    }
  }

  sprintf(alPath,"%s/format",getenv("SWITCH_CFGDIR"));
  ilCount_fmt_m = ilCount_fmt_g = ilCount_fmt_d = 0;
  while (!feof(pp))
  {
    memset(alFiletmp,0x00,sizeof(alFiletmp));
    fscanf(pp,"%s",alFiletmp); 
    if ((alFiletmp[0]=='/')&&(alFiletmp[strlen(alFiletmp)-1]==':'))
    {
      strcpy(alPath,alFiletmp);
      alPath[strlen(alPath)-1] = 0;
      continue;
    }  
    if (strlen(alFiletmp) < 4) continue;
    memset(alTmp1,0x00,sizeof(alTmp1));
    for(n=0;n<4;n++)
    {
      alTmp1[n] = toupper(alFiletmp[strlen(alFiletmp)-4+n]);
    }
    if(strcmp(alTmp1,".FMT")) continue;
/*    swCheckfmt(&pilFmt,alFiletmp); delete by zjj 2004.02.28*/ /* add by nh 20021009 */
    sprintf(alFile,"%s/%s",alPath,alFiletmp);
    if ((fp = fopen(alFile, "r")) == NULL) continue;
    swVdebug(2,"S5110: 格式转换文件: %s",alFile);
    memset(alBuf, 0x00, sizeof(alBuf));
    ilSegflag = 0;
    i = 0;
    ilScriptline = 0;

    /* add by qy 2001.09.18 */
    strcpy(alFiletmp2, alFiletmp);
    if (strlen(alFiletmp) == 0) continue;
    if ((alTmp = strchr(alFiletmp2, '.')) == NULL) 
    {
      swVdebug(0,"S5115: [错误/其它] 文件名[%s]不正确", alFiletmp);
      fclose(fp);
      continue;
    }
    *alTmp = '\0';
    ilFlag = 0;
    for (j=0;j<strlen(alFiletmp2);j++)
    {
      if ((alFiletmp2[j] > '9') || (alFiletmp2[j] < '0'))
      {
        swVdebug(0,"S5120: [错误/其它] 文件名[%s]不正确", alFiletmp);
        ilFlag = 1;
        fclose(fp);
        break;
      }
    }
    if (ilFlag == 1) continue;
        
    /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
    /* ilFmtgrp_id = atoi(alFiletmp2); */
    ilFmtgrp_id = atol(alFiletmp2);
    if (ilFmtgrp_id == 0) 
    {
      swVdebug(0,"S5125: [错误/其它] 格式转换组号不能为[0],文件名为[%s]", alFiletmp);
      fclose(fp);
       continue;
    }
    /* end of add by qy 2001.09.18 */
    
    while (1)
    {
      ilRc = swGetitemmode3(fp, &ilSegflag, palKey, palFldname, palFldvalue, alBuf_rc);
      if (ilRc != 0 || ilSegflag == 0) break;
      
      if (ilSegflag == 1)
      {
        ilCount_fmt_g ++;
        if (ilCount_fmt_g == 1)
          sgLcmidx.psShm_sys_fmt_g = (struct shm_sys_fmt_g *)
          	malloc(sizeof(struct shm_sys_fmt_g));
        else
          sgLcmidx.psShm_sys_fmt_g = (struct shm_sys_fmt_g *)
          	realloc(sgLcmidx.psShm_sys_fmt_g, 
                ilCount_fmt_g * sizeof(struct shm_sys_fmt_g));
        if ((sgLcmidx.psShm_sys_fmt_g) == NULL)
        {
          swVdebug(0,"S5130: [错误/其它] malloc / realloc error!");
           goto ErrorExit;
        }
        memset((char *)&((sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1]), 0x00,
        	sizeof(struct shm_sys_fmt_g));
        /* delete by qy 2001.09.18 */
        /*
        (sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].
        	sSwt_sys_fmt_g.fmt_group = atoi(palKey[0]);
        */
        /* end of delete by qy */

        (sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].
        	sSwt_sys_fmt_g.fmt_group = ilFmtgrp_id;
        	
        ilRc = _swGetvaluemode1(palFldname,palFldvalue,"FMTDESC",alFldvalue);
        if (ilRc == 0)
          strcpy((sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].sSwt_sys_fmt_g.
          	fmt_desc, alFldvalue);	

        ilRc = _swGetvaluemode1(palFldname,palFldvalue,"TRANTYPE",alFldvalue);
        if (ilRc == 0)
        {
          (sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].sSwt_sys_fmt_g.
          	tran_type = atoi(alFldvalue);
          ilTrantype = atoi(alFldvalue);
        }  		

        ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MAC_CREATE",alFldvalue);
        if (ilRc == 0)
        {
          /* next 2 row add by nh 2002.7.3 */
          ilRc = swExpSign(alFldvalue);
          if (ilRc != SUCCESS) return(ilRc);
    
          /* add by zjj 2001.11.29 */
          ilRc = _swExpN2T(alFldvalue,alTmpexp);
          if (ilRc != 0) return(ilRc);
          strcpy((sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].sSwt_sys_fmt_g.
          	mac_create, alTmpexp);	
          /* end add by zjj 2001.11.29 */
        }

        ilRc = _swGetvaluemode1(palFldname,palFldvalue,"MAC_CHECK",alFldvalue);
        if (ilRc == 0)
        {
          /* next 2 row add by nh 2002.7.3 */
          ilRc = swExpSign(alFldvalue);
          if (ilRc != SUCCESS) return(ilRc);
          
          /* add by zjj 2001.11.29 */
          ilRc = _swExpN2T(alFldvalue,alTmpexp);
          if (ilRc != 0) return(ilRc);
          strcpy((sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].sSwt_sys_fmt_g.
          	mac_check, alTmpexp);
          /* end add by zjj 2001.11.29 */
        }	
        
        /*  	
        ilRc = _swGetvaluemode1(palFldname,palFldvalue,"tran_sort",alFldvalue);
        if (ilRc == 0)
          strcpy((sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].sSwt_sys_fmt_g.
          	tran_sort, alFldvalue);
        */	
      }
      else if (ilSegflag == 3)
      {
      	/*   delete by zjj 2001.11.20  
        memset(&slSwt_sys_fmt_d, 0x00, sizeof(struct swt_sys_fmt_d));
        if (ilTrantype != iXMF_IMF && ilTrantype != iIMF_XMF)  
        {
          if (swParseTDF(alBuf_rc, ilTrantype, &slSwt_sys_fmt_d) != 0)
          {
            swVdebug(0,"S5135: [错误/函数调用] swParseTDF()函数,文件名: %s",alFile);
            swVdebug(0,"S5140: 文件名: %s",alFile);
            goto ErrorExit;
          }
        }
        else  
        {
          if (swParseXMF(alBuf_rc, &slSwt_sys_fmt_d) != 0)
          {
            swVdebug(0,"S5145: [错误/函数调用] swParseXMF()函数,文件名: %s",alFile);
            goto ErrorExit;
          }
        }
        ilScriptline ++;
        end delete by zjj 2001.11.20 */
        /* add by zjj 2001.11.20  */
        i++;
        if (i == 1)
        {
          pslBuf = (struct buf *)malloc(sizeof(struct buf));
          pre_alloc = 1;
        }
        else
        {
            pslBuf = (struct buf *)realloc(pslBuf, i*sizeof(struct buf));
        }
        if (pslBuf == NULL)
        {
          swVdebug(0,"S5150: [错误/其它] buffer malloc error");
          goto ErrorExit;
        }
        if (ilTrantype != iXMF_IMF && ilTrantype != iIMF_XMF)  
        {
          if (swParseTDF(alBuf_rc, ilTrantype, &pslBuf[i-1].sSwt_sys_fmt_d) != 0)
          {
            swVdebug(0,"S5155: [错误/函数调用] swParseTDF()函数,文件名: %s",alFile);
            swVdebug(0,"S5160: 文件名: %s",alFile);
            goto ErrorExit;
          }
        }
        else  
        {
          if (swParseXMF(alBuf_rc, &pslBuf[i-1].sSwt_sys_fmt_d) != 0)
          {
            swVdebug(0,"S5165: [错误/函数调用] swParseXMF()函数,文件名: %s",alFile);
            goto ErrorExit;
          }
        }
        pslBuf[i-1].cFlag = 1;      
       }
    }
    ilScriptcount = i;
    /* 扫描整个格式转换脚本，增加对switch,while语句的支持 */
    /* swDebug("S0540:ilScriptcount = %d", ilScriptcount); */
    if (ilTrantype == iXMF_IMF || ilTrantype == iIMF_XMF) 
    { 
      for (i=0; i<ilScriptcount; i++)
      {
        switch (pslBuf[i].sSwt_sys_fmt_d.fmt_flag[0])
        {
          case 'J' :	/* WHILE */
          case 'K' :	/* CONTINUE */
          case 'L' :    /* EXIT */
          case 'N' :    /* SWITCH */
          case 'O' :	/* CASE */
          case 'Q' :	/* DEFAULT */
          case 'P' :    /* BREAK */
            /* 压入堆栈 */
            pslNewptr = (struct tag *)malloc(sizeof(struct tag));
            if (pslNewptr == NULL)
            {
              swVdebug(0,"S5170: [错误/其它] stack malloc error");
               goto ErrorExit;
            }
            pslNewptr->cTag = pslBuf[i].sSwt_sys_fmt_d.fmt_flag[0];
            pslNewptr->iBufid = i;
            pslNewptr->psNxtptr = pslTag;
            pslTag = pslNewptr;
            break;
          case 'M' :	/* ENDWHILE */
            /* 从栈顶开始查找第一个WHILE,取得脚本行号 */
            pslTmpptr = pslTag;
            clFindflag = 0;
            while (pslTmpptr != NULL)
            {
              /* if (pslTmpptr->cTag == 'W')	delete by bmy 2002.3.12 */
              if (pslTmpptr->cTag == 'J')	/* WHILE */ /* add by bmy 2002.3.12 */
              {
                j = pslTmpptr->iBufid;
                clFindflag = 1;
                break;
              }
              pslTmpptr = pslTmpptr->psNxtptr;
            }
            if (!clFindflag)
            {
              swVdebug(0,"S5175: [错误/其它] WHILE 语法错误");
              goto ErrorExit;
            }  
            /* 处理当前ENDWHILE */
            /* pslBuf[i].sSwt_sys_fmt_d.fmt_flag[0] = cGOTO; delete by bmy 2002.3.12 */
            pslBuf[i].sSwt_sys_fmt_d.fmt_flag[0] = 'A';  /* add by bmy 2002.3.12 */
            pslBuf[i].sSwt_sys_fmt_d.fld_id = pslBuf[j].sSwt_sys_fmt_d.id;
            /* 从栈顶开始处理CONTINUE,EXIT,WHILE直到第一个WHILE */
            while (pslTag != NULL)
            {
              k = pslTag->iBufid;
              if (pslTag->cTag == 'K')	/* CONTINUE */
              {
                /* pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = cGOTO; delete by bmy 2002.3.12 */
                pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'A';  /* add by bmy 2002.3.12 */
                pslBuf[k].sSwt_sys_fmt_d.fld_id = pslBuf[j].sSwt_sys_fmt_d.id;
                pslTmpptr = pslTag;
                pslTag = pslTag->psNxtptr;
                free(pslTmpptr);
              }
              else if (pslTag->cTag == 'L')	/* EXIT */
              {
                /* modify by bmy 2002.03.12 */
                if (i == (ilScriptcount - 1)) 
                {
                  pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'F'; /* RETURN */
                  pslBuf[k].sSwt_sys_fmt_d.fld_id = 0;
                }
                else
                {
                  /* pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = cGOTO; bmy */
                  pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'A';
                  pslBuf[k].sSwt_sys_fmt_d.fld_id = pslBuf[i+1].sSwt_sys_fmt_d.id;
                }
                /* end of modify by bmy */

                pslTmpptr = pslTag;
                pslTag = pslTag->psNxtptr;
                free(pslTmpptr);
              }
              else if (pslTag->cTag == 'J')	/* WHILE */
              {
                /* modify by bmy 2002.03.12 */
                if (i == (ilScriptcount - 1))
                {
                  pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'F'; /* RETURN */
                  pslBuf[k].sSwt_sys_fmt_d.fld_id = 0;
                }
                else
                {
                  /* pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = cGOTO; qy */
                  pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'A';
                  pslBuf[k].sSwt_sys_fmt_d.fld_id = pslBuf[i+1].sSwt_sys_fmt_d.id;
                }
                /* end of modify by bmy */

                /* add by zjj 2001.11.29 */
                alCond[0] = 0;
                strcpy(alCond, pslBuf[k].sSwt_sys_fmt_d.fmt_cond);
                sprintf(alCond,"NOT(%s)",pslBuf[k].sSwt_sys_fmt_d.fmt_cond);
                pslBuf[k].sSwt_sys_fmt_d.fmt_cond[0] = 0;
                
                /* next 2 row add by nh 2002.7.3 */
                ilRc = swExpSign(alCond);
                if (ilRc != SUCCESS) return(ilRc);
          
                ilRc = _swExpN2T(alCond,alTmpexp);
                if (ilRc != 0) return(ilRc);
                strcpy(pslBuf[k].sSwt_sys_fmt_d.fmt_cond,alTmpexp);
               /* end add by zjj 2001.11.29 */ 

               /* 
                strcpy(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, "NOT(");
                strcat(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, alCond);
                strcat(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, ")");
               */  /* delete by zjj 2001.11.19 */
                /*swVdebug(0,"S5180: [错误/其它] ccc: %s", pslBuf[k].sSwt_sys_fmt_d.fmt_cond);*/
                pslTmpptr = pslTag;
                pslTag = pslTag->psNxtptr;
                free(pslTmpptr);
                break;
              }
              else
              {
                swVdebug(0,"S5185: [错误/其它] 语法错误");
                goto ErrorExit;
              }  
            }
            break;
          case 'R' :	/* ENDSWITCH */
            /* 从栈顶开始查找第一个SWITCH,取得脚本条件表达式 */
            pslTmpptr = pslTag;
            clFindflag = 0;
            while (pslTmpptr != NULL)
            {
              if (pslTmpptr->cTag == 'N')	/* SWITCH */
              {
                alSwitch[0] = 0;
                strcpy(alSwitch, pslBuf[pslTmpptr->iBufid].sSwt_sys_fmt_d.
                fmt_cond);
                clFindflag = 1;
                break;
              }
              pslTmpptr = pslTmpptr->psNxtptr;
            }
            if (!clFindflag)
            {
              swVdebug(0,"S5190: [错误/其它] SWITCH 语法错误");
              goto ErrorExit;
            }  
            /* 从栈顶开始处理CASE,BREAK,DEFAULT,SWITCH直到第一个SWITCH */
            j = i +1;
            while (pslTag != NULL)
            {
              swVdebug(3,"S5195:  SWITCH语法关键字标识[%c]", pslTag->cTag);
              k = pslTag->iBufid; 
              if (pslTag->cTag == 'P')	/* BREAK */
              {
                /* modify by bmy 2002.03.12 */
                if (i == (ilScriptcount - 1))
                {
                  pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'F'; /* RETURN */
                  pslBuf[k].sSwt_sys_fmt_d.fld_id = 0;
                }
                else
                {
                  /* pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = cGOTO; bmy */
                  pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'A';
                  pslBuf[k].sSwt_sys_fmt_d.fld_id = pslBuf[i+1].sSwt_sys_fmt_d.id;
                }
                /* end of modify by bmy */

                pslTmpptr = pslTag;
                pslTag = pslTag->psNxtptr;
                free(pslTmpptr);
              }
              else if (pslTag->cTag == 'Q')	/* DEFAULT */
              {
                if ((j = k + 1) == i) j ++;
                alCond[0] = 0;
                pslBuf[k].cFlag = 0;
                pslTmpptr = pslTag;
                pslTag = pslTag->psNxtptr;
                free(pslTmpptr);
              }
              else if (pslTag->cTag == 'O')	/* CASE */
              {
                /* pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = cGOTO; delete by bmy 2002.3.12 */
                pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'A';  /* add by bmy 2002.3.12 */
                pslBuf[k].sSwt_sys_fmt_d.fld_id = pslBuf[j].sSwt_sys_fmt_d.id;
              /*
                alCond[0] = 0;
                strcpy(alCond, pslBuf[k].sSwt_sys_fmt_d.fmt_cond);
                pslBuf[k].sSwt_sys_fmt_d.fmt_cond[0] = 0;
                strcpy(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, "NOT(SEQ(");
                strcat(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, alSwitch);
                strcat(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, ",");
                strcat(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, alCond);
                strcat(pslBuf[k].sSwt_sys_fmt_d.fmt_cond, "))"); 
              */ /* delete by zjj 2001.11.29 */
          
               /* add by zjj 2001.11.29 */
                alCond[0] = 0;
                sprintf(alCond,"NOT(SEQ(%s,%s))",\
                  alSwitch,pslBuf[k].sSwt_sys_fmt_d.fmt_cond);
                
                /* next 2 row add by nh 2002.7.3 */
                ilRc = swExpSign(alCond);
                if (ilRc != SUCCESS) return(ilRc);
          
                ilRc = _swExpN2T(alCond,alTmpexp);
                if (ilRc != 0) return(ilRc); 
                pslBuf[k].sSwt_sys_fmt_d.fmt_cond[0] = 0;
                strcpy(pslBuf[k].sSwt_sys_fmt_d.fmt_cond,alTmpexp);
               /* end add by zjj 2001.11.29  */

                j = k;
                pslTmpptr = pslTag;
                pslTag = pslTag->psNxtptr;
                free(pslTmpptr);
              }  
              else if (pslTag->cTag == 'N')	/* SWITCH */
              {
                /* pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = cGOTO; delete by bmy 2002.3.12 */
                pslBuf[k].sSwt_sys_fmt_d.fmt_flag[0] = 'A';  /* add by bmy 2002.3.12 */
                pslBuf[k].sSwt_sys_fmt_d.fld_id = pslBuf[j].sSwt_sys_fmt_d.id;
                pslBuf[k].sSwt_sys_fmt_d.fmt_cond[0] = 0;
                pslTmpptr = pslTag;
                pslTag = pslTag->psNxtptr;
                free(pslTmpptr);
                break;
              }
              else
              {
                swVdebug(0,"S5200: [错误/其它] 语法错误");
                goto ErrorExit;
              }  
            }
            pslBuf[i].cFlag = 0; 
            break;
          default:
            break;
        }
      }
    }  /*  end if */

  for(i=0;i < ilScriptcount;i++)
  {
    if (pslBuf[i].cFlag == 0) continue;
    ilScriptline ++;
    pslBuf[i].sSwt_sys_fmt_d.fmt_group = (sgLcmidx.psShm_sys_fmt_g)
        [ilCount_fmt_g - 1].sSwt_sys_fmt_g.fmt_group;
    ilCount_fmt_d ++;

    if (ilCount_fmt_d == 1)
      sgLcmidx.psSwt_sys_fmt_d = (struct swt_sys_fmt_d *)
        malloc(sizeof(struct swt_sys_fmt_d));
    else
      sgLcmidx.psSwt_sys_fmt_d = (struct swt_sys_fmt_d *)
        realloc(sgLcmidx.psSwt_sys_fmt_d,
          ilCount_fmt_d * (sizeof(struct swt_sys_fmt_d)));
    if (sgLcmidx.psSwt_sys_fmt_d == NULL)
    {
      swVdebug(0,"S5205: [错误/其它] malloc / realloc Error!");
      goto ErrorExit; 
    }
    if (ilScriptline == 1)
    {
      (sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].lOffset = 
       ilCount_fmt_d - 1;
    }
    (sgLcmidx.psShm_sys_fmt_g)[ilCount_fmt_g - 1].iCount = 
      ilScriptline;
      
    memcpy(&(sgLcmidx.psSwt_sys_fmt_d)[ilCount_fmt_d - 1],
      &pslBuf[i].sSwt_sys_fmt_d,sizeof(struct swt_sys_fmt_d));
    }
    if (ilScriptcount > 0)
    {
      free(pslBuf);
      free(pslTag);
    }
    fclose(fp);
  }
  pclose(pp);
  pp = NULL;
  /* add by nh 20021009 */
  free(pilFmt);
  pilFmt=NULL;  /* end add */
  sgLcmidx.iCount_swt_sys_fmt_g = ilCount_fmt_g;
  sgLcmidx.iCount_swt_sys_fmt_d = ilCount_fmt_d;
  /* end add by zjj 2001.11.20  */


#ifndef HASH
  /* add by dgm 2001.09.26 */
  if ((fp = _swFopenlist("config/SWMATCH_FMT.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5210: _swFopenlist(SWMATCH_FMT.CFG),errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_fmt_m = 0;
      return 0;
    }  
  }
  
  ilCount_fmt_m = 0;
  while (fp)
  {
    while(1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0)
        break;
      ilCount_fmt_m ++;
      if (ilCount_fmt_m == 1)
        sgLcmidx.psShm_sys_fmt_m = (struct shm_sys_fmt_m *)
            malloc(sizeof(struct shm_sys_fmt_m));
      else
        sgLcmidx.psShm_sys_fmt_m = (struct shm_sys_fmt_m *)
 	    realloc(sgLcmidx.psShm_sys_fmt_m,
              ilCount_fmt_m * sizeof(struct shm_sys_fmt_m));
      if ((sgLcmidx.psShm_sys_fmt_m) == NULL)
      {
        swVdebug(0,"S5215: [错误/其它] malloc or realloc error!");
        goto ErrorExit;
      }
      memset((char *)&((sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1]), 0x00, 
       	sizeof(struct shm_sys_fmt_m));

      ilPriority = 0;

      (sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].sSwt_sys_fmt_m.q_id = 
         	atoi(palFldvalue[1]);
      if (atoi(palFldvalue[1]) == 0) 
        ilPriority ++ ; 

      _swTrim(palFldvalue[2]);
      strcpy((sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].sSwt_sys_fmt_m.
    	  tran_code, palFldvalue[2]);
      if ((palFldvalue[2][0] == '\0') || (strcmp(palFldvalue[2], "0") == 0))
        ilPriority ++ ; 

      (sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].sSwt_sys_fmt_m.tran_step =
  	  atoi(palFldvalue[3]);
      if (atoi(palFldvalue[3]) == 0) 
        ilPriority ++ ; 

      _swTrim(palFldvalue[4]);
      strcpy((sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].
    	  sSwt_sys_fmt_m.tran_type, palFldvalue[4]);
      if ((palFldvalue[4][0] == '\0') || (strcmp(palFldvalue[4], "0") == 0))
        ilPriority ++ ; 

      (sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].sSwt_sys_fmt_m.
          fmt_prior = ilPriority;

      /* del by gengling at 2015.03.25 two lines PSBC_V1.0 */
      /* (sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].sSwt_sys_fmt_m.fmt_group =
  	  atoi(palFldvalue[0]); */
      (sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].sSwt_sys_fmt_m.fmt_group =
  	  atol(palFldvalue[0]);
    
      for(i = 0;i < sgLcmidx.iCount_swt_sys_fmt_g;i ++)
      {
        /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
        /* if (atoi(palFldvalue[0]) == (sgLcmidx.psShm_sys_fmt_g)[i].sSwt_sys_fmt_g.fmt_group) */
        if (atol(palFldvalue[0]) == (sgLcmidx.psShm_sys_fmt_g)[i].sSwt_sys_fmt_g.fmt_group)
        {
          (sgLcmidx.psShm_sys_fmt_m)[ilCount_fmt_m - 1].lOffset = i;
          break;
        }
      }
      if (i == sgLcmidx.iCount_swt_sys_fmt_g)
      {
        /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
        /* swVdebug(0,"S5220: [错误/其它] swShm_swt_sys_fmt error 模糊匹配组号[%d]不存在! ", atoi(palFldvalue[0])); */
        swVdebug(0,"S5220: [错误/其它] swShm_swt_sys_fmt error 模糊匹配组号[%ld]不存在! ", atol(palFldvalue[0]));
        goto ErrorExit;
      }
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  
  sgLcmidx.iCount_swt_sys_fmt_m = ilCount_fmt_m;
  
/*end of add by dgm 2001.09.26 */

  /* 排序 */
  /* swt_sys_fmt_m: order by fmt_prior,q_id desc,tran_code desc,tran_step desc,tran_type desc */
  for (i = 1; i <= ilCount_fmt_m - 1; i ++)
  {
    for (j = 0; j < ilCount_fmt_m - 1; j ++)
    {
      ilSwapflag = 0;
      if (sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.fmt_prior > sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.fmt_prior)
      {
      	ilSwapflag = 1;
      }
      else if (sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.fmt_prior == sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.fmt_prior)
      {
        if (sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.q_id < sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.q_id)
          ilSwapflag = 1;
        else if (sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.q_id == sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.q_id)
        {
          ilRc = strcmp(sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.tran_code,sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.tran_code);
          if (ilRc < 0)
            ilSwapflag = 1;
          else if (ilRc == 0)
          {
            if (sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.tran_step < sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.tran_step)
              ilSwapflag = 1;
            else if (sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.tran_step == sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.tran_step)
            {
              ilRc = strcmp(sgLcmidx.psShm_sys_fmt_m[j].sSwt_sys_fmt_m.tran_type,sgLcmidx.psShm_sys_fmt_m[j+1].sSwt_sys_fmt_m.tran_type);
              if (ilRc < 0)
                ilSwapflag = 1;
            }    
          }
        }
      }
      if (ilSwapflag == 1)
      {
        memcpy(&slShm_sys_fmt_m,&sgLcmidx.psShm_sys_fmt_m[j],sizeof(struct shm_sys_fmt_m));
        memcpy(&sgLcmidx.psShm_sys_fmt_m[j],&sgLcmidx.psShm_sys_fmt_m[j+1],sizeof(struct shm_sys_fmt_m));
        memcpy(&sgLcmidx.psShm_sys_fmt_m[j+1],&slShm_sys_fmt_m,sizeof(struct shm_sys_fmt_m));
      }
    }
  }  
#endif

  return 0;

  ErrorExit :
    if ((sgLcmidx.psShm_sys_fmt_g) != NULL) 
    {
      free(sgLcmidx.psShm_sys_fmt_g);
      sgLcmidx.psShm_sys_fmt_g = NULL;
    }
    if ((sgLcmidx.psShm_sys_fmt_m) != NULL)
    {
      free(sgLcmidx.psShm_sys_fmt_m);
      sgLcmidx.psShm_sys_fmt_m = NULL;
    }
    if ((sgLcmidx.psSwt_sys_fmt_d) != NULL)
    {
      free(sgLcmidx.psSwt_sys_fmt_d);
      sgLcmidx.psSwt_sys_fmt_d = NULL;
    }
    fclose(fp);
    if (pp != NULL)
    {
      pclose(pp);
      pp = NULL;
    }
    return -1;
}

#ifdef HASH
/***************************************************************
 ** 函数名      : swShm_swt_sys_fmt_m
 ** 功  能      : 读SWMATCH_FMT.CFG配置文件
 ** 作  者      : gengling
 ** 建立日期    : 2015/05/12
 ** 最后修改日期: 
 ** 调用其它函数: swGetitemmode1
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_fmt_m(int *pCount, void *pHash)
{
  short ilPriority;
  int i;
  char szCmd[128];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc, ilCount;
  struct shm_sys_fmt_m slShm_sys_fmt_m;
  char tmp[128];
  int tabSize, dataSize, indexSize;
  int recordNumber, recordSize, keySize;
  int rc = 0;
  FILE *pfp = NULL;
  FILE *fp = NULL;

  memset(szCmd, 0x00, sizeof(szCmd));
  memset(tmp, 0x00, sizeof(tmp));
  sprintf(szCmd, "wc -l %s/config/SWMATCH_FMT.CFG", getenv("SWITCH_CFGDIR"));

  if((pfp = popen(szCmd, "r")) == NULL)
  {
      swVdebug(1, "popen err [%s]", szCmd);
      return -1;
  }

  fgets(tmp, 128, pfp);
  _swTrim(tmp);
  rc = atoi(tmp);

  recordNumber = rc;
  recordSize = sizeof(struct shm_sys_fmt_m);
  keySize = 64;

  indexSize = idxSize(keySize, recordNumber);
  dataSize = recordNumber * recordSize;
  tabSize = indexSize + dataSize + sizeof(long) + sizeof(int);

  *pCount=tabSize;
  swVdebug(5, "swShm_swt_sys_fmt_m tabSize[%d],recordNumber[%d],indexSize[%d]", tabSize, recordNumber, indexSize);

  if (pHash == NULL)
      return 0;

  long dataAreaOffset = 0; // the first area is the offset of data area
  dataAreaOffset = sizeof(long) +sizeof(int) + indexSize;

  memcpy(pHash, (void *)&dataAreaOffset, sizeof(long));
  memcpy(pHash + sizeof(long), (void *)&recordNumber, sizeof(int));//the  second area is total record number of data
  char * indexPtr = pHash + sizeof(long)+sizeof(int); // the third area is index area
  char * dataPtr = indexPtr + indexSize;  // the last area is data area

  // init index area
  idxInit(indexPtr, keySize, recordNumber);

  if ((fp = _swFopenlist("config/SWMATCH_FMT.CFG", "r")) == NULL)
  {
      if (errno != ENOENT)
      {
          swVdebug(0,"S5095: _swFopenlist(SWMATCH_FMT.CFG),errno=%d,strerror(errno)",errno);
          return -1;
      }
      else
      {
          sgLcmidx.iCount_swt_sys_fmt_m = 0;
          return 0;
      }  
  }

  ilCount = 0;
  int offset = 0;
  char keyData[64];
  while (fp)
  {
      while (1)
      {
          ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
          if (ilRc < 0) break;

          ilCount ++;
          memset(&slShm_sys_fmt_m, 0x00, sizeof(struct shm_sys_fmt_m));

          ilPriority = 0;

          slShm_sys_fmt_m.sSwt_sys_fmt_m.q_id = atoi(palFldvalue[1]);
          if (atoi(palFldvalue[1]) == 0)
              ilPriority ++ ;

          _swTrim(palFldvalue[2]);
          strcpy(slShm_sys_fmt_m.sSwt_sys_fmt_m.tran_code, palFldvalue[2]);
          if ((palFldvalue[2][0] == '\0') || (strcmp(palFldvalue[2], "0") == 0))
              ilPriority ++ ; 

          slShm_sys_fmt_m.sSwt_sys_fmt_m.tran_step = atoi(palFldvalue[3]);
          if (atoi(palFldvalue[3]) == 0)
              ilPriority ++ ;

          _swTrim(palFldvalue[4]);
          strcpy(slShm_sys_fmt_m.sSwt_sys_fmt_m.tran_type, palFldvalue[4]);
          if ((palFldvalue[4][0] == '\0') || (strcmp(palFldvalue[4], "0") == 0))
              ilPriority ++ ;

          slShm_sys_fmt_m.sSwt_sys_fmt_m.fmt_prior = ilPriority;

          slShm_sys_fmt_m.sSwt_sys_fmt_m.fmt_group = atol(palFldvalue[0]);

          for(i = 0;i < sgLcmidx.iCount_swt_sys_fmt_g;i ++)
          {
            if (atol(palFldvalue[0]) == (sgLcmidx.psShm_sys_fmt_g)[i].sSwt_sys_fmt_g.fmt_group)
            {
              slShm_sys_fmt_m.lOffset = i;
              break;
            }
          }
          if (i == sgLcmidx.iCount_swt_sys_fmt_g)
          {
            swVdebug(0,"S5220: [错误/其它] swShm_swt_sys_fmt_m error 模糊匹配组号[%ld]不存在! ", atol(palFldvalue[0]));
            goto ErrorExit;
          }

          memset(keyData, 0x00, sizeof(keyData));
          sprintf(keyData, "%d|%s|%d|%s", slShm_sys_fmt_m.sSwt_sys_fmt_m.q_id, slShm_sys_fmt_m.sSwt_sys_fmt_m.tran_code,
                  slShm_sys_fmt_m.sSwt_sys_fmt_m.tran_step, slShm_sys_fmt_m.sSwt_sys_fmt_m.tran_type);
          swVdebug(5, "swShm_swt_sys_fmt_m keyData [%s]", keyData);

          memcpy(dataPtr + offset, &slShm_sys_fmt_m, recordSize);
          if ((rc = idxInsert(indexPtr, keyData, offset)) < 0)
          {
              swVdebug(1, "swShm_swt_sys_fmt_m idxInsert error keyData[%s] offset[%d] rc[%d]", keyData, offset, rc);
          }
          else
          {
              swVdebug(5, "swShm_swt_sys_fmt_m idxInsert right keyData[%s] offset[%d]", keyData, offset);
          }
          offset += recordSize;
      }
      fclose(fp);
      fp = _swFopenlist(NULL, "r");
  }
  sgLcmidx.iCount_swt_sys_fmt_m = ilCount;

  return 0;

  ErrorExit :
    if ((sgLcmidx.psShm_sys_fmt_g) != NULL) 
    {
      free(sgLcmidx.psShm_sys_fmt_g);
      sgLcmidx.psShm_sys_fmt_g = NULL;
    }
    if ((sgLcmidx.psShm_sys_fmt_m) != NULL)
    {
      free(sgLcmidx.psShm_sys_fmt_m);
      sgLcmidx.psShm_sys_fmt_m = NULL;
    }
    if ((sgLcmidx.psSwt_sys_fmt_d) != NULL)
    {
      free(sgLcmidx.psSwt_sys_fmt_d);
      sgLcmidx.psSwt_sys_fmt_d = NULL;
    }
    fclose(fp);
    return -1;
}
#endif

#ifdef HASH
/***************************************************************
 ** 函数名      : swShm_swt_sys_fmt_g
 ** 功  能      : 读格式转换组表配置文件hash[*FMT.CFG]
 ** 作  者      : gengling
 ** 建立日期    : 2015/05/15
 ** 最后修改日期: 
 ** 调用其它函数: swGetitemmode1
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_fmt_g(int *pCount, void *pHash)
{
    int i;
    char szCmd[128];
    char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
    int ilRc, ilCount;
    struct shm_sys_fmt_g slShm_sys_fmt_g;
    char tmp[128];
    int tabSize, dataSize, indexSize;
    int recordNumber, recordSize, keySize;
    int rc = 0;
    FILE *pfp = NULL;
    FILE *fp = NULL;

    char *alTmp;
    char alPath[100], alFile[80], alBuf[257], alBuf_rc[iEXPRESSLEN+1];
    char palKey[iKEYNUMBER][iFLDVALUELEN];
    char palFldname[iFLDNUMBER][iFLDNAMELEN];
    char alFldvalue[iFLDVALUELEN];
    char alFiletmp[80],alFiletmp2[80];
    int ilCount_fmt_g,ilScriptline;
    int j,ilFlag,ilSegflag,ilTrantype;
    long ilFmtgrp_id;
    FILE *pp = NULL;
    char alTmp1[5];
    int n;

    memset(szCmd, 0x00, sizeof(szCmd));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(szCmd, "cd %s/format;ls -1 *FMT|wc -l", getenv("SWITCH_CFGDIR"));

    if((pfp = popen(szCmd, "r")) == NULL)
    {
        swVdebug(1, "popen err [%s]", szCmd);
        return -1;
    }

    fgets(tmp, 128, pfp);
    _swTrim(tmp);
    rc = atoi(tmp);

    recordNumber = rc;
    recordSize = sizeof(struct shm_sys_fmt_g);
    keySize = 64;

    indexSize = idxSize(keySize, recordNumber);
    dataSize = recordNumber * recordSize;
    tabSize = indexSize + dataSize + sizeof(long) + sizeof(int);

    *pCount=tabSize;
    swVdebug(5, "swShm_swt_sys_fmt_g tabSize[%d],recordNumber[%d],indexSize[%d]", tabSize, recordNumber, indexSize);

    if (pHash == NULL)
        return 0;

    long dataAreaOffset = 0; // the first area is the offset of data area
    dataAreaOffset = sizeof(long) + sizeof(int) + indexSize;

    memcpy(pHash, (void *)&dataAreaOffset, sizeof(long));
    memcpy(pHash + sizeof(long), (void *)&recordNumber, sizeof(int));//the  second area is total record number of data
    char * indexPtr = pHash + sizeof(long) + sizeof(int); // the third area is index area
    char * dataPtr = indexPtr + indexSize;  // the last area is data area

    // init index area
    idxInit(indexPtr, keySize, recordNumber);

    ilCount = 0;
    int offset = 0;
    char keyData[64];

    char alTmpexp[2 * iFLDVALUELEN + 1]; /* add by zjj 2001.11.29 */
    static short *pilFmt=NULL;      /* add by nh 20021009 */

    if ((pp = _swPopen("format","r")) == NULL)
    {
        if (errno != ENOENT)
        {
            swVdebug(0,"S5105: [错误/其它] popen()函数,errno=%d,打开管道文件出错!",errno);
            return -1;
        }
        else
        {
            sgLcmidx.iCount_swt_sys_fmt_g = 0;
            return 0;
        }
    }

    sprintf(alPath,"%s/format",getenv("SWITCH_CFGDIR"));
    while (!feof(pp))
    {
        memset(alFiletmp,0x00,sizeof(alFiletmp));
        fscanf(pp,"%s",alFiletmp); 
        if ((alFiletmp[0]=='/')&&(alFiletmp[strlen(alFiletmp)-1]==':'))
        {
            strcpy(alPath,alFiletmp);
            alPath[strlen(alPath)-1] = 0;
            continue;
        }  
        if (strlen(alFiletmp) < 4) continue;
        memset(alTmp1,0x00,sizeof(alTmp1));
        for(n=0;n<4;n++)
        {
            alTmp1[n] = toupper(alFiletmp[strlen(alFiletmp)-4+n]);
        }
        if(strcmp(alTmp1,".FMT")) continue;
        /*    swCheckfmt(&pilFmt,alFiletmp); delete by zjj 2004.02.28*/ /* add by nh 20021009 */
        sprintf(alFile,"%s/%s",alPath,alFiletmp);
        if ((fp = fopen(alFile, "r")) == NULL) continue;
        swVdebug(2,"S5110: 格式转换文件: %s",alFile);
        memset(alBuf, 0x00, sizeof(alBuf));
        ilSegflag = 0;
        i = 0;
        ilScriptline = 0;

        /* add by qy 2001.09.18 */
        strcpy(alFiletmp2, alFiletmp);
        if (strlen(alFiletmp) == 0) continue;
        if ((alTmp = strchr(alFiletmp2, '.')) == NULL) 
        {
            swVdebug(0,"S5115: [错误/其它] 文件名[%s]不正确", alFiletmp);
            fclose(fp);
            continue;
        }
        *alTmp = '\0';
        ilFlag = 0;
        for (j=0;j<strlen(alFiletmp2);j++)
        {
            if ((alFiletmp2[j] > '9') || (alFiletmp2[j] < '0'))
            {
                swVdebug(0,"S5120: [错误/其它] 文件名[%s]不正确", alFiletmp);
                ilFlag = 1;
                fclose(fp);
                break;
            }
        }
        if (ilFlag == 1) continue;

        /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
        /* ilFmtgrp_id = atoi(alFiletmp2); */
        ilFmtgrp_id = atol(alFiletmp2);
        if (ilFmtgrp_id == 0) 
        {
            swVdebug(0,"S5125: [错误/其它] 格式转换组号不能为[0],文件名为[%s]", alFiletmp);
            fclose(fp);
            continue;
        }
        /* end of add by qy 2001.09.18 */

        while (1)
        {
            ilRc = swGetitemmode3(fp, &ilSegflag, palKey, palFldname, palFldvalue, alBuf_rc);
            if (ilRc != 0 || ilSegflag == 0) break;

            if (ilSegflag == 1)
            {
                ilCount ++;
                memset(&slShm_sys_fmt_g, 0x00, sizeof(struct shm_sys_fmt_g));

                slShm_sys_fmt_g.sSwt_sys_fmt_g.fmt_group = ilFmtgrp_id;

                ilRc = _swGetvaluemode1(palFldname, palFldvalue, "FMTDESC", alFldvalue);
                if (ilRc == 0)
                    strcpy(slShm_sys_fmt_g.sSwt_sys_fmt_g.fmt_desc, alFldvalue);

                ilRc = _swGetvaluemode1(palFldname, palFldvalue, "TRANTYPE", alFldvalue);
                if (ilRc == 0)
                {
                    slShm_sys_fmt_g.sSwt_sys_fmt_g.tran_type = atoi(alFldvalue);
                    ilTrantype = atoi(alFldvalue);
                }

                ilRc = _swGetvaluemode1(palFldname, palFldvalue, "MAC_CREATE", alFldvalue);
                if (ilRc == 0)
                {
                    /* next 2 row add by nh 2002.7.3 */
                    ilRc = swExpSign(alFldvalue);
                    if (ilRc != SUCCESS) return(ilRc);

                    /* add by zjj 2001.11.29 */
                    ilRc = _swExpN2T(alFldvalue, alTmpexp);
                    if (ilRc != 0) return(ilRc);

                    strcpy(slShm_sys_fmt_g.sSwt_sys_fmt_g.mac_create, alTmpexp);
                    /* end add by zjj 2001.11.29 */
                }

                ilRc = _swGetvaluemode1(palFldname, palFldvalue, "MAC_CHECK", alFldvalue);
                if (ilRc == 0)
                {
                    /* next 2 row add by nh 2002.7.3 */
                    ilRc = swExpSign(alFldvalue);
                    if (ilRc != SUCCESS) return(ilRc);

                    /* add by zjj 2001.11.29 */
                    ilRc = _swExpN2T(alFldvalue, alTmpexp);
                    if (ilRc != 0) return(ilRc);

                    strcpy(slShm_sys_fmt_g.sSwt_sys_fmt_g.mac_check, alTmpexp);
                    /* end add by zjj 2001.11.29 */
                }

                memset(keyData, 0x00, sizeof(keyData));
                sprintf(keyData, "%ld", slShm_sys_fmt_g.sSwt_sys_fmt_g.fmt_group);
                swVdebug(5, "swShm_swt_sys_fmt_g keyData [%s]", keyData);

                memcpy(dataPtr + offset, (char *)&slShm_sys_fmt_g, recordSize);
                if ((rc = idxInsert(indexPtr, keyData, offset)) < 0)
                {
                    swVdebug(1, "swShm_swt_sys_fmt_g idxInsert error keyData[%s] offset[%d] rc[%d]", keyData, offset, rc);
                }
                else
                {
                    swVdebug(5, "swShm_swt_sys_fmt_g idxInsert right keyData[%s] offset[%d] ilCount[%d]", keyData, offset, ilCount);
                }
                offset += recordSize;
            }
        }
        fclose(fp);
        fp = _swFopenlist(NULL, "r");
    }
    pclose(pp);
    pp = NULL;
    sgLcmidx.iCount_swt_sys_fmt_g = ilCount;

    return 0;
}
#endif

/***************************************************************
 ** 函数名      : swParseroute
 ** 功  能      : 解析route脚本
 ** 作  者      :
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swParseroute(char *aBuf,struct swt_sys_route_d * psSwt_sys_route_d)
{
  char *alTmp1,*alTmp2,alTmp3[20],alTmp4[1024],*alTmp5;
  int  ilRc,ilCount;
  long ilMode; /*modified by baiqj20150428 PSBC_V1.0 int -> long*/
  char alTmpexp[2 * iFLDVALUELEN + 1];  /* add by zjj 2001.11.29 */
  char alTmp[2 * iFLDVALUELEN + 1];  /* add by zjj 2004.02.28 */

  memset(psSwt_sys_route_d,0x00,sizeof(struct swt_sys_route_d));
  memset(alTmp4,0x00,sizeof(alTmp4));

  _swTrim(aBuf);
  
  /* add by zjj 2004.02.28 */
  memset(alTmp,0x00,sizeof(alTmp));
  strcpy(alTmp,aBuf);
  /* end add by zjj 2004.02.28 */
    
  alTmp5 = strchr(aBuf,' ');
  if (alTmp5 == NULL) 
  {
    swVdebug(0,"S5225: [错误/其它] 路由脚本[%s]中行号未定义!",alTmp);
     return(-1);
  }
  alTmp1 = alTmp5 + 1;
  *alTmp5 = '\0';
  _swTrim(alTmp1);  
  psSwt_sys_route_d->route_id = atoi(aBuf); 

  /* 是路由脚本段 */
  /* modify by nh 20040422 
  if (atoi(aBuf) == 0) */
  if (atoi(aBuf) == 0 || atoi(aBuf) == 9999)
  {
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"WITH") == 0)  
    {
      alTmp1 = alTmp1 + 4;
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3,"REVMODE");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->rev_mode = ilMode;
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3,"REVGROUP");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->rev_fmtgrp = ilMode;
    }
  }
  else
  /*  处理路由脚本  */
  {
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,3);
    strtoupper(alTmp3);

    /* 是条件执行语句  */  
    if (strcmp(alTmp3,"IF ") == 0)
    {
      alTmp1 = alTmp1 + 3;
      _swTrim(alTmp1);
      memset(alTmp4,0x00,sizeof(alTmp4));
      strcpy(alTmp4,alTmp1);
      strtoupper(alTmp4);
      alTmp2 = alTmp1;
      alTmp5 = strstr(alTmp4," THEN ");
      if (alTmp5 == NULL) 
      {
        swVdebug(0,"S5230: [错误/其它] 路由脚本[%s]定义错误(有IF无THEN)!",alTmp);
         return(-1);
      }
      ilCount = alTmp5 - alTmp4;
      *(alTmp1 + ilCount) = '\0';
      alTmp1 = alTmp1 + ilCount + 6;
      _swTrim(alTmp1);
      _swTrim(alTmp2);

      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp2);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
          
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp2,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_route_d->route_cond,alTmpexp);
      /* end add by zjj 2001.11.29 */

      /* strcpy(psSwt_sys_route_d->route_cond,alTmp2); */ /* delete by zjj */
    }

   /*  处理 CALL方式  */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"CALL ") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"1");
      alTmp1 = alTmp1 + 5;
      _swTrim(alTmp1);
      memset(alTmp4,0x00,sizeof(alTmp4));
      strcpy(alTmp4,alTmp1);
      strtoupper(alTmp4);
      alTmp5 = strstr(alTmp4,"WITH");
      if (alTmp5 == NULL)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alTmp1);
        if (ilRc != SUCCESS) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
      
        /* add by zjj 2001.11.29 */
        ilRc = _swExpN2T(alTmp1,alTmpexp);
        if (ilRc != 0) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
        strcpy(psSwt_sys_route_d->q_target,alTmpexp);
        /* end add by zjj 2001.11.29 */

       /*  strcpy(psSwt_sys_route_d->q_target,alTmp1); */ /* del by zjj */
      }
      else
      {
	alTmp2 = alTmp1;
        ilCount = alTmp5 - alTmp4;
        *(alTmp1 + ilCount) = '\0';
	alTmp1 = alTmp1 + ilCount + 4;
	_swTrim(alTmp2);

        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alTmp2);
        if (ilRc != SUCCESS) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
          
        /* add by zjj 2001.11.29 */
        ilRc = _swExpN2T(alTmp2,alTmpexp);
        if (ilRc != 0) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
	strcpy(psSwt_sys_route_d->q_target,alTmpexp);
        /* end add by zjj 2001.11.29 */

	/* strcpy(psSwt_sys_route_d->q_target,alTmp2); */ /* del by zjj */
      }  
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3," GROUP ");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->fmt_group = ilMode;
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3,"REVMODE");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->rev_mode = ilMode; 
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3,"REVGROUP");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->rev_fmtgrp = ilMode;
    }

    /*  处理SEND方式 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"SEND ") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"2"); 
      alTmp1 = alTmp1 + 5;
      _swTrim(alTmp1);
      memset(alTmp4,0x00,sizeof(alTmp4));
      strcpy(alTmp4,alTmp1);
      strtoupper(alTmp4);
      alTmp5 = strstr(alTmp4,"WITH");
      if (alTmp5 == NULL)
      {
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alTmp1);
        if (ilRc != SUCCESS) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
          
        /* add by zjj 2001.11.29 */
        ilRc = _swExpN2T(alTmp1,alTmpexp);
        if (ilRc != 0) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
        strcpy(psSwt_sys_route_d->q_target,alTmpexp);
        /* end add by zjj 2001.11.29 */

        /*  strcpy(psSwt_sys_route_d->q_target,alTmp1); */ /* del by zjj */
      }
      else
      {
        alTmp2 = alTmp1;
        ilCount = alTmp5 - alTmp4;
        *(alTmp1 + ilCount) = '\0';
        alTmp1 = alTmp1 + ilCount + 4;
        _swTrim(alTmp2);
       
        /* next 2 row add by nh 2002.7.3 */
        ilRc = swExpSign(alTmp2);
        if (ilRc != SUCCESS) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
        
        /* add by zjj 2001.11.29 */
        ilRc = _swExpN2T(alTmp2,alTmpexp);
        if (ilRc != 0) 
        {
          swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
          return(ilRc);
        }
        strcpy(psSwt_sys_route_d->q_target,alTmpexp);
        /* end add by zjj 2001.11.29 */

        /*  strcpy(psSwt_sys_route_d->q_target,alTmp2); */ /* del by zjj */
      }
      alTmp5 = strstr(alTmp4,"SAF"); 
      if (alTmp5 != NULL)   
        strcpy(psSwt_sys_route_d->saf_flag,"1");
      alTmp5 = strstr(alTmp4,"END");
      if (alTmp5 != NULL)  
        strcpy(psSwt_sys_route_d->end_flag,"1");
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3," GROUP ");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->fmt_group = ilMode;
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3,"REVMODE");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->rev_mode = ilMode;
      memset(alTmp3,0x00,sizeof(alTmp3));
      strcpy(alTmp3,"REVGROUP");
      swGetrevmode(alTmp1,alTmp3,&ilMode);
      psSwt_sys_route_d->rev_fmtgrp = ilMode;
    }

    /*  ROLLBACK */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,8);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"ROLLBACK") == 0) 
      strcpy(psSwt_sys_route_d->oper_flag,"3");

    /* GOTO 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"GOTO ") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"4");
      alTmp2 = alTmp1 + 5;
      _swTrim(alTmp2);
      psSwt_sys_route_d->next_id = atoi(alTmp2);
    }

    /*  域名赋值 SET */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,3);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"SET") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"5");
      alTmp1 = alTmp1 + 3;
      _swTrim(alTmp1);
      alTmp5 = strchr(alTmp1,'=');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5235: [错误/其它] SET 语法错误[%s]!",alTmp);
         return(-1);
      }
      alTmp2 = alTmp5 + 1;
      *alTmp5 = '\0';
      _swTrim(alTmp1);
      _swTrim(alTmp2);
      strcpy(psSwt_sys_route_d->fml_name,alTmp1);
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp2);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp2,alTmpexp);
      if (ilRc != 0)
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_route_d->q_target,alTmpexp);
      /* end add by zjj 2001.11.29 */      
      
      /*strcpy(psSwt_sys_route_d->q_target,alTmp2); */ /* del by zjj */
    }

    /*  SQL语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"EXEC") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"6");
      alTmp5 = strchr(alTmp1,'(');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5240: [错误/其它] EXEC 语法错误[%s]!",alTmp);
         return(-1);
      }
      alTmp1 = alTmp5 + 1;
      alTmp5 = strchr(alTmp1,')');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5245: [错误/其它] EXEC 语法错误[%s]!",alTmp);
         return(-1);
      }
      alTmp2 = alTmp5 + 1; 
      *alTmp5 = '\0';
      _swTrim(alTmp1);
      _swTrim(alTmp2);
      psSwt_sys_route_d->next_id = atoi(alTmp1);
      strcpy(psSwt_sys_route_d->q_target,alTmp2);
    }
   
    /*  FTPPUT 方式 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,6);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"FTPPUT") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"7");
      alTmp5 = strchr(alTmp1,'(');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5250: [错误/其它] FTPPUT 语法错误[%s]!",alTmp);
         return(-1);
      }
      alTmp1 = alTmp5 + 1;
      alTmp5 = strchr(alTmp1,')');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5255: [错误/其它] FTPPUT 语法错误[%s]!",alTmp);
         return(-1);
      }
      *alTmp5 = '\0';
      _swTrim(alTmp1);
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp1);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp1,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_route_d->q_target,alTmpexp);
      /* end add by zjj 2001.11.29 */

      /*  strcpy(psSwt_sys_route_d->q_target,alTmp1); */ /* del by zjj */
     }
   
    /* FTPGET 方式 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,6);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"FTPGET") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"8");
      alTmp5 = strchr(alTmp1,'(');
      if (alTmp5 == NULL)  
      {
        swVdebug(0,"S5260: [错误/其它] FTPGET 语法错误[%s]!",alTmp);
         return(-1);
      }
      alTmp1 = alTmp5 + 1;
      alTmp5 = strchr(alTmp1,')');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5265: [错误/其它] FTPGET 语法错误[%s]!",alTmp);
         return(-1);
      }
      *alTmp5 = '\0';
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp1);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp1,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_route_d->q_target,alTmpexp);
      /* end add by zjj 2001.11.29 */

      /*  strcpy(psSwt_sys_route_d->q_target,alTmp1); */ /* del by zjj */
    }

    /*  QPUT 方式 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"QPUT") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"9");
      alTmp5 = strchr(alTmp1,'(');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5270: [错误/其它] QPUT 语法错误[%s]!",alTmp);
         return(-1);
      }
      alTmp1 = alTmp5 + 1;
      alTmp5 = strchr(alTmp1,')');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5275: [错误/其它] QPUT 语法错误[%s]!",alTmp);
         return(-1);
      }
      *alTmp5 = '\0';
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp1);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp1,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_route_d->q_target,alTmpexp);
      /* end add by zjj 2001.11.29 */

      /*  strcpy(psSwt_sys_route_d->q_target,alTmp1); */ /* del by zjj */
    }
    
    /* QGET 方式 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"QGET") == 0)
    {
      strcpy(psSwt_sys_route_d->oper_flag,"a");
      alTmp5 = strchr(alTmp1,'(');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5280: [错误/其它] QGET 语法错误[%s]!",alTmp);
         return(-1);
      }
      alTmp1 = alTmp5 + 1;
      alTmp5 = strchr(alTmp1,')');
      if (alTmp5 == NULL)
      {
        swVdebug(0,"S5285: [错误/其它] QGET 语法错误[%s]!",alTmp);
         return(-1);
      }
      *alTmp5 = '\0';

      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp1);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp1,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_route_d->q_target,alTmpexp);
      /* end add by zjj 2001.11.29 */

      /*  strcpy(psSwt_sys_route_d->q_target,alTmp1); */ /* del by zjj */      
    }

    /* DO 操作  add by zjj 2001.11.06 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,2);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"DO") == 0)
    {
      psSwt_sys_route_d->oper_flag[0] = 'Z';
      alTmp1 = alTmp1 + 2;
      _swTrim(alTmp1);
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp1);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp1,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_route_d->q_target,alTmpexp);
      /* end add by zjj 2001.11.29 */

      /*  strcpy(psSwt_sys_route_d->q_target,alTmp1); */ /* del by zjj */      
    }
    /* end add by zjj 2001.11.06 */
    
    /* WHILE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"WHILE") == 0)
    {
      psSwt_sys_route_d->oper_flag[0] = 'W';
      alTmp1 = alTmp1 + 5;
      _swTrim(alTmp1);
      /* 由于此处的表达式是中间临时表达式，所以不用做逆波兰转换 by zjj 2001.11.29*/
      strcpy(psSwt_sys_route_d->route_cond,alTmp1);
    }

    /* CONTINUE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,8);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"CONTINUE") == 0)
      psSwt_sys_route_d->oper_flag[0] = 'C';

    /* EXIT */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"EXIT") == 0)
      psSwt_sys_route_d->oper_flag[0] = 'X';
      
    /* ENDWHILE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,8);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"ENDWHILE") == 0)
      psSwt_sys_route_d->oper_flag[0] = 'E';
    
    /* SWITCH */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,6);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"SWITCH") == 0)
    {
      psSwt_sys_route_d->oper_flag[0] = 'S';
      alTmp1 = alTmp1 + 6;
      _swTrim(alTmp1);
      /* 由于此处的表达式是中间临时表达式，所以不用做逆波兰转换 by zjj 2001.11.29*/      
      strcpy(psSwt_sys_route_d->route_cond,alTmp1);
    }

    /* CASE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"CASE") == 0)
    {
      psSwt_sys_route_d->oper_flag[0] = 'A';
      alTmp1 = alTmp1 + 4;
      _swTrim(alTmp1);
      /* 由于此处的表达式是中间临时表达式，所以不用做逆波兰转换 by zjj 2001.11.29*/      
      strcpy(psSwt_sys_route_d->route_cond,alTmp1);
    }

    /* BREAK */  
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"BREAK") == 0)
      psSwt_sys_route_d->oper_flag[0] = 'B';

    /* DEFAULT */  
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,7);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"DEFAULT") == 0)
      psSwt_sys_route_d->oper_flag[0] = 'D';

    /* ENDSWITCH */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,9);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"ENDSWITCH") == 0)
      psSwt_sys_route_d->oper_flag[0] = 'N';

  }
  return(0);
}

/***************************************************************
 ** 函数名      : strtoupper
 ** 功  能      : 字符串转换为大写
 ** 作  者      :
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
static int strtoupper(char *aBuf)
{
  while (*aBuf != '\0')
  {
    *aBuf = toupper(*aBuf);
    aBuf++;
  }
  return(0);
}

/***************************************************************
 ** 函数名      : swGetrevmode
 ** 功  能      : 解析路由脚本获得 REVMODE,REVGROUP,GROUP等
 ** 作  者      :
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
static int swGetrevmode(char *aBuf,char *aBuf_find,int *ilMode)
{
  char *alTmp1,*alTmp2,alTmp3[1024];
  memset(alTmp3,0x00,sizeof(alTmp3));
  strcpy(alTmp3,aBuf);
  strtoupper(alTmp3);
  alTmp2 = strstr(alTmp3,aBuf_find);
  if (alTmp2 == NULL) 
  { 
     *ilMode = 0;  
     return(0);
  }
  alTmp1 = alTmp3;
  alTmp1 = alTmp2 + strlen(aBuf_find);
  *(alTmp1 - 1) = '\0';
  alTmp2 = strchr(alTmp1,',');
  
  if (alTmp2 == NULL) 
  {
    *ilMode = atoi(alTmp1); 
  }
  else
  {
    *alTmp2 = '\0';
    _swTrim(alTmp1);
    *ilMode = atoi(alTmp1);
  }
  return(0);
}

/***************************************************************
 ** 函数名      : swShm_swt_sys_route
 ** 功  能      : 读路由[*.SCR]配置文件
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_route()
{
  char *alTmp;
  char alPath[100],alFile[80],alBuf[257],alBuf_rc[257];
  char palKey[iKEYNUMBER][iFLDVALUELEN];
  char palFldname[iFLDNUMBER][iFLDNAMELEN];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  char alFldvalue[iFLDVALUELEN]; 
  char alFiletmp[80],alFiletmp2[80];
  int  ilCount_route_g,ilCount_route_m,ilCount_route_d,ilScriptline;
  int  i,j,k,ilRc,ilSegflag,ilSwapflag,ilFlag,m;
  short ilRoutegrp_id;
  struct swt_sys_route_d slSwt_sys_route_d;
  struct shm_sys_route_m slShm_sys_route_m;
  FILE *fp,*pp;
  char alTmp1[5];
  int n;

  /* buf */
  struct buf
  {
    char cFlag;
    struct swt_sys_route_d sSwt_sys_route_d;
  };

  /* tag */
  struct tag
  {
    char cTag;
    short iBufid;
    struct tag *psNxtptr;
  };

  char alCond[101],alSwitch[101];
  struct buf *pslBuf;
  struct tag *pslTag=NULL,*pslNewptr,*pslTmpptr;
  short ilScriptcount=0;
  char clFindflag; 
  char alTmpexp[2 * iFLDVALUELEN + 1]; /* add by zjj 2001.11.29 */
  static short *pilRoute=NULL;      /* add by nh 20021009 */

  if ((pp = _swPopen("router","r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5290: [错误/系统调用] popen()函数,errno=%d,打开管道文件出错!",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_route_m = 0;
      sgLcmidx.iCount_swt_sys_route_g = 0; 
      sgLcmidx.iCount_swt_sys_route_d = 0;
      return 0;
    }
  }

  sprintf(alPath,"%s/router",getenv("SWITCH_CFGDIR"));
  ilCount_route_g = ilCount_route_m = ilCount_route_d = 0;
  while (feof(pp) == 0)
  {
    memset(alFiletmp,0x00,sizeof(alFiletmp));
    fscanf(pp,"%s",alFiletmp);
    if ((alFiletmp[0]=='/')&&(alFiletmp[strlen(alFiletmp)-1]==':'))
    {
      strcpy(alPath,alFiletmp);
      alPath[strlen(alPath)-1] = 0;
      continue;
    }  
    if (strlen(alFiletmp) < 4) continue;
    memset(alTmp1,0x00,sizeof(alTmp1));
    for(n=0;n<4;n++)
    {
      alTmp1[n] = toupper(alFiletmp[strlen(alFiletmp)-4+n]);
    }
    if(strcmp(alTmp1,".SCR")) continue;
/*    swCheckroute(&pilRoute,alFiletmp); delete by zjj 2004.02.28*//* add by nh 20021009 */
    sprintf(alFile,"%s/%s",alPath,alFiletmp);
    if ((fp = fopen(alFile,"r")) == NULL)  continue;
    swVdebug(2,"S5295: 路由脚本文件: %s",alFile);
    i = 0;
    memset(alBuf,0x00,sizeof(alBuf));
    ilSegflag = 0;
    ilScriptline = 0;

    /* add by qy 2001.09.18 */
    strcpy(alFiletmp2, alFiletmp);
    if ((alTmp = strchr(alFiletmp2, '.')) == NULL)
    {
      swVdebug(0,"S5300: [错误/其它] 文件名[%s]不正确", alFiletmp);
      fclose(fp);
      continue;
    }   
    *alTmp = '\0';
    ilFlag = 0;
    for (m=0;m<strlen(alFiletmp2);m++)
    {
      if ((alFiletmp2[m] > '9') || (alFiletmp2[m] < '0'))
      {
        swVdebug(0,"S5305: [错误/其它] 文件名[%s]不正确", alFiletmp);
        ilFlag = 1;
        fclose(fp);
        break;
      }
    }
    if (ilFlag == 1) continue;

    ilRoutegrp_id = atoi(alFiletmp2);
    /* end of add by qy 2001.09.18 */

    while (1)
    {
      ilRc = swGetitemmode3(fp,&ilSegflag,palKey,palFldname,
                            palFldvalue,alBuf_rc);
      if (ilRc != 0 || ilSegflag == 0) break;
      if (ilSegflag == 1)
      {
        ilCount_route_g++;
	if (ilCount_route_g == 1)
	  sgLcmidx.psShm_sys_route_g = (struct shm_sys_route_g *)
	    malloc(sizeof(struct shm_sys_route_g));
        else
	  sgLcmidx.psShm_sys_route_g = (struct shm_sys_route_g *)
	    realloc(sgLcmidx.psShm_sys_route_g,
	      ilCount_route_g * sizeof(struct shm_sys_route_g));
        if (sgLcmidx.psShm_sys_route_g == NULL)
	{
          swVdebug(0,"S5310: [错误/其它] malloc / realloc Error!");
 	  goto ErrorExit;
	}
	memset((char *)&(sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1]),
	  0x00,sizeof(struct shm_sys_route_g));

        /* delete by qy 2001.09.18 */
        /*
        sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1].
	  sSwt_sys_route_g.route_grp = atoi(palKey[0]);
        */
        /* end of delete by qy */

        sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1].
	  sSwt_sys_route_g.route_grp = ilRoutegrp_id;
        
	ilRc = _swGetvaluemode1(palFldname,palFldvalue,"ROUTEDESC",alFldvalue);
	if (ilRc == 0)
	  strcpy(sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1].
	    sSwt_sys_route_g.route_desc,alFldvalue);

        /*
        ilRc = _swGetvaluemode1(palFldname,palFldvalue,"tran_sort",alFldvalue);
	if (ilRc == 0)
	  strcpy(sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1].
	    sSwt_sys_route_g.tran_sort,alFldvalue);
        */
      }
/*
      else if (ilSegflag == 2)
      {
        ilCount_route_m++;
	if (ilCount_route_m == 1)
	  sgLcmidx.psShm_sys_route_m = (struct shm_sys_route_m *)
	    malloc(sizeof(struct shm_sys_route_m));
        else
	  sgLcmidx.psShm_sys_route_m = (struct shm_sys_route_m *)
	    realloc(sgLcmidx.psShm_sys_route_m,
	      ilCount_route_m * sizeof(struct shm_sys_route_m));
        if (sgLcmidx.psShm_sys_route_m == NULL)
        {
          swVdebug(0,"S5315: [错误/其它] malloc / realloc Error !");
           goto ErrorExit;
        }
        memset((char*)&((sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1]),
          0x00,sizeof(struct shm_sys_route_m));

        (sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].lOffset = 
          ilCount_route_g - 1;

        (sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].sSwt_sys_route_m.
          route_grp = (sgLcmidx.psShm_sys_route_g)[ilCount_route_g - 1].
          sSwt_sys_route_g.route_grp;

        (sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].sSwt_sys_route_m.
          q_id = atoi(palKey[0]);

        strcpy((sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].
         sSwt_sys_route_m.trancode,palKey[1]); 
      }
*/
      else if (ilSegflag == 3)
      {
        /* 增加对WHILE和SWITCH的支持 */
        i++;
        if (i == 1)
          pslBuf = (struct buf *)malloc(sizeof(struct buf));
        else
          pslBuf = (struct buf *)realloc(pslBuf, i*sizeof(struct buf));
        if (pslBuf == NULL)
        {
          swVdebug(0,"S5320: [错误/其它] buffer malloc error");
          goto ErrorExit;
        }
        if (swParseroute(alBuf_rc, &pslBuf[i-1].sSwt_sys_route_d) != 0)
        {
          swVdebug(0,"S5325: [错误/函数调用] swParseroute()函数,返回码=-1,解析文件[%s]出错!",alFile);
          goto ErrorExit;
        }
        pslBuf[i-1].cFlag = 1;
      }
    }

    ilScriptcount = i;
    /* 扫描路由脚本 */
    /* swDebug("S0540:ilScriptcount = %d", ilScriptcount); */
    for (i=0; i<ilScriptcount; i++)
    {
      switch (pslBuf[i].sSwt_sys_route_d.oper_flag[0])
      {
        case 'W' :	/* WHILE */
        case 'C' :	/* CONTINUE */
        case 'X' :    /* EXIT */
        case 'S' :    /* SWITCH */
        case 'A' :	/* CASE */
        case 'D' :	/* DEFAULT */
        case 'B' :    /* BREAK */
          /* 压入堆栈 */
          pslNewptr = (struct tag *)malloc(sizeof(struct tag));
          if (pslNewptr == NULL)
          {
            swVdebug(0,"S5330: [错误/其它] stack malloc error");
             goto ErrorExit;
          }
          pslNewptr->cTag = pslBuf[i].sSwt_sys_route_d.oper_flag[0];
          pslNewptr->iBufid = i;
          pslNewptr->psNxtptr = pslTag;
          pslTag = pslNewptr;
          break;
        case 'E' :	/* ENDWHILE */
          /* 从栈顶开始查找第一个WHILE,取得脚本行号 */
          pslTmpptr = pslTag;
          clFindflag = 0;
          while (pslTmpptr != NULL)
          {
            if (pslTmpptr->cTag == 'W')	/* WHILE */
            {
              j = pslTmpptr->iBufid;
              clFindflag = 1;
              break;
            }
            pslTmpptr = pslTmpptr->psNxtptr;
          }
          if (!clFindflag)
          {
            swVdebug(0,"S5335: [错误/其它] WHILE 语法错误");
             goto ErrorExit;
          }  
          /* 处理当前ENDWHILE */
          pslBuf[i].sSwt_sys_route_d.oper_flag[0] = cGOTO;
          pslBuf[i].sSwt_sys_route_d.next_id = pslBuf[j].sSwt_sys_route_d.route_id;
          /* 从栈顶开始处理CONTINUE,EXIT,WHILE直到第一个WHILE */
          while (pslTag != NULL)
          {
            k = pslTag->iBufid;
            if (pslTag->cTag == 'C')	/* CONTINUE */
            {
              pslBuf[k].sSwt_sys_route_d.oper_flag[0] = cGOTO;
              pslBuf[k].sSwt_sys_route_d.next_id = pslBuf[j].sSwt_sys_route_d.route_id;
              pslTmpptr = pslTag;
              pslTag = pslTag->psNxtptr;
              free(pslTmpptr);
            }
            else if (pslTag->cTag == 'X')	/* EXIT */
            {
              pslBuf[k].sSwt_sys_route_d.oper_flag[0] = cGOTO;
              pslBuf[k].sSwt_sys_route_d.next_id = pslBuf[i+1].sSwt_sys_route_d.route_id;
              pslTmpptr = pslTag;
              pslTag = pslTag->psNxtptr;
              free(pslTmpptr);
            }
            else if (pslTag->cTag == 'W')	/* WHILE */
            {
              pslBuf[k].sSwt_sys_route_d.oper_flag[0] = cGOTO;
              pslBuf[k].sSwt_sys_route_d.next_id = pslBuf[i+1].sSwt_sys_route_d.route_id;
              /*
              alCond[0] = 0;
              strcpy(alCond, pslBuf[k].sSwt_sys_route_d.route_cond);
              pslBuf[k].sSwt_sys_route_d.route_cond[0] = 0;
              strcpy(pslBuf[k].sSwt_sys_route_d.route_cond, "NOT(");
              strcat(pslBuf[k].sSwt_sys_route_d.route_cond, alCond);
              strcat(pslBuf[k].sSwt_sys_route_d.route_cond, ")");
              */  /* delete by zjj 2001.11.29 */
              
              /* add by zjj 2001.11.29 */
              alCond[0] = 0;
              sprintf(alCond,"NOT(%s)",pslBuf[k].sSwt_sys_route_d.route_cond);
              
              /* next 2 row add by nh 2002.7.3 */
              ilRc = swExpSign(alCond);
              if (ilRc != SUCCESS) return(ilRc);
      
              ilRc = _swExpN2T(alCond,alTmpexp);
              if (ilRc != 0) return(ilRc);
              pslBuf[k].sSwt_sys_route_d.route_cond[0] = 0;              
              sprintf(pslBuf[k].sSwt_sys_route_d.route_cond,alTmpexp);
              /* end add by zjj 2001.11.29 */
              
              swVdebug(0,"S5340: [错误/其它] ccc: %s", pslBuf[k].sSwt_sys_route_d.route_cond);
               pslTmpptr = pslTag;
              pslTag = pslTag->psNxtptr;
              free(pslTmpptr);
              break;
            }
            else
            {
              swVdebug(0,"S5345: [错误/其它] 语法错误");
               goto ErrorExit;
            }  
          }
          break;
        case 'N' :	/* ENDSWITCH */
          /* 从栈顶开始查找第一个SWITCH,取得脚本条件表达式 */
          pslTmpptr = pslTag;
          clFindflag = 0;
          while (pslTmpptr != NULL)
          {
            if (pslTmpptr->cTag == 'S')	/* SWITCH */
            {
              alSwitch[0] = 0;
              strcpy(alSwitch, pslBuf[pslTmpptr->iBufid].sSwt_sys_route_d.
              route_cond);
              clFindflag = 1;
              break;
            }
            pslTmpptr = pslTmpptr->psNxtptr;
          }
          if (!clFindflag)
          {
            swVdebug(0,"S5350: [错误/其它] SWITCH 语法错误");
             goto ErrorExit;
          }  
          /* 从栈顶开始处理CASE,BREAK,DEFAULT,SWITCH直到第一个SWITCH */
          j = i +1;
          while (pslTag != NULL)
          {
            swVdebug(0,"S5355:  [错误/其它] %c", pslTag->cTag);
             k = pslTag->iBufid; 
            if (pslTag->cTag == 'B')	/* BREAK */
            {
              pslBuf[k].sSwt_sys_route_d.oper_flag[0] = cGOTO;
              pslBuf[k].sSwt_sys_route_d.next_id = pslBuf[i+1].sSwt_sys_route_d.route_id;
              pslTmpptr = pslTag;
              pslTag = pslTag->psNxtptr;
              free(pslTmpptr);
            }
            else if (pslTag->cTag == 'D')	/* DEFAULT */
            {
              if ((j = k + 1) == i) j ++;
              alCond[0] = 0;
              pslBuf[k].cFlag = 0;
              pslTmpptr = pslTag;
              pslTag = pslTag->psNxtptr;
              free(pslTmpptr);
            }
            else if (pslTag->cTag == 'A')	/* CASE */
            {
              pslBuf[k].sSwt_sys_route_d.oper_flag[0] = cGOTO;
              pslBuf[k].sSwt_sys_route_d.next_id = pslBuf[j].sSwt_sys_route_d.route_id;
            /*
              alCond[0] = 0;
              strcpy(alCond, pslBuf[k].sSwt_sys_route_d.route_cond);
              pslBuf[k].sSwt_sys_route_d.route_cond[0] = 0;
              strcpy(pslBuf[k].sSwt_sys_route_d.route_cond, "NOT(SEQ(");
              strcat(pslBuf[k].sSwt_sys_route_d.route_cond, alSwitch);
              strcat(pslBuf[k].sSwt_sys_route_d.route_cond, ",");
              strcat(pslBuf[k].sSwt_sys_route_d.route_cond, alCond);
              strcat(pslBuf[k].sSwt_sys_route_d.route_cond, "))"); 
            */ /* delete by zjj 2001.11.29 */
            
             /* add by zjj 2001.11.29 */
              alCond[0] = 0;
              sprintf(alCond,"NOT(SEQ(%s,%s))",\
                alSwitch,pslBuf[k].sSwt_sys_route_d.route_cond);
              pslBuf[k].sSwt_sys_route_d.route_cond[0] = 0;
              sprintf(pslBuf[k].sSwt_sys_route_d.route_cond,alCond);
              /* end add by zjj 2001.11.29 */
              
              j = k;
              pslTmpptr = pslTag;
              pslTag = pslTag->psNxtptr;
              free(pslTmpptr);
            }  
            else if (pslTag->cTag == 'S')	/* SWITCH */
            {
              pslBuf[k].sSwt_sys_route_d.oper_flag[0] = cGOTO;
              pslBuf[k].sSwt_sys_route_d.next_id = pslBuf[j].sSwt_sys_route_d.route_id;
              pslBuf[k].sSwt_sys_route_d.route_cond[0] = 0;
              pslTmpptr = pslTag;
              pslTag = pslTag->psNxtptr;
              free(pslTmpptr);
              break;
            }
            else
            {
              swVdebug(0,"S5360: [错误/其它] 语法错误");
               goto ErrorExit;
            }  
          }
          pslBuf[i].cFlag = 0; 
          break;
        default:
          break;
      }
    }
    for (i=0; i<ilScriptcount; i++)
    {
      if (pslBuf[i].cFlag == 0) continue;
      ilScriptline ++;
      pslBuf[i].sSwt_sys_route_d.route_grp = (sgLcmidx.psShm_sys_route_g)
        [ilCount_route_g - 1].sSwt_sys_route_g.route_grp;
      ilCount_route_d ++;
      if (ilCount_route_d == 1)
        sgLcmidx.psSwt_sys_route_d = (struct swt_sys_route_d *)
          malloc(sizeof(struct swt_sys_route_d));
      else
        sgLcmidx.psSwt_sys_route_d = (struct swt_sys_route_d *)
          realloc(sgLcmidx.psSwt_sys_route_d,
            ilCount_route_d * (sizeof(struct swt_sys_route_d)));
      if (sgLcmidx.psSwt_sys_route_d == NULL)
      {
        swVdebug(0,"S5365: [错误/其它] malloc / realloc Error!");
         goto ErrorExit; 
      }
       
      if (ilScriptline == 1)
        (sgLcmidx.psShm_sys_route_g)[ilCount_route_g - 1].lOffset = 
          ilCount_route_d - 1;
      
      (sgLcmidx.psShm_sys_route_g)[ilCount_route_g - 1].iCount = 
        ilScriptline;
        
      memcpy(&(sgLcmidx.psSwt_sys_route_d)[ilCount_route_d - 1],
        &pslBuf[i].sSwt_sys_route_d,sizeof(struct swt_sys_route_d));
    }
    if (ilScriptcount > 0)
    {
      free(pslBuf);
      free(pslTag);
    }

    /* 排序 */
    /* swt_sys_route_d: order by route_id (route_grp = 当前组) */
    for (i = 1; i <= sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1].iCount - 1; i ++)
    {
      for (j = 0; j < sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1].iCount - 1; j ++)
      {
        k = j + sgLcmidx.psShm_sys_route_g[ilCount_route_g - 1].lOffset;
        if (sgLcmidx.psSwt_sys_route_d[k].route_id > sgLcmidx.psSwt_sys_route_d[k+1].route_id)
        {
          memcpy(&slSwt_sys_route_d,&sgLcmidx.psSwt_sys_route_d[k],sizeof(struct swt_sys_route_d));
          memcpy(&sgLcmidx.psSwt_sys_route_d[k],&sgLcmidx.psSwt_sys_route_d[k+1],sizeof(struct swt_sys_route_d));
          memcpy(&sgLcmidx.psSwt_sys_route_d[k+1],&slSwt_sys_route_d,sizeof(struct swt_sys_route_d));
        }
      }
    }
      
    fclose(fp);
  }
  pclose(pp);
  pp = NULL;
  /* add by nh 20021009 */
  free(pilRoute);
  pilRoute=NULL;  /* end add */
  sgLcmidx.iCount_swt_sys_route_m = ilCount_route_m;
  sgLcmidx.iCount_swt_sys_route_g = ilCount_route_g; 
  sgLcmidx.iCount_swt_sys_route_d = ilCount_route_d;
 
#ifndef HASH
/* add by dgm 2001.09.26 */
  if ((fp = _swFopenlist("config/SWMATCH_ROUTE.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5370: _swFopen(SWMATCH_ROUTE.CFG),errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_route_m = 0;
      return 0;
    }  
  }
  
  ilCount_route_m = 0;
  while (fp)
  {
    while(1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0)
        break;
      ilCount_route_m ++;
      if (ilCount_route_m == 1)
        sgLcmidx.psShm_sys_route_m = (struct shm_sys_route_m *)
	    malloc(sizeof(struct shm_sys_route_m));
      else
        sgLcmidx.psShm_sys_route_m = (struct shm_sys_route_m *)
            realloc(sgLcmidx.psShm_sys_route_m,
                ilCount_route_m * sizeof(struct shm_sys_route_m));
      if (sgLcmidx.psShm_sys_route_m == NULL)
      {
        swVdebug(0,"S5375: [错误/其它] malloc / realloc Error !");
        goto ErrorExit;
      }
      memset((char*)&((sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1]),
          0x00,sizeof(struct shm_sys_route_m));
    
      (sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].sSwt_sys_route_m.route_grp =
  	  atoi(palFldvalue[0]);

      (sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].sSwt_sys_route_m.q_id = 
         	atoi(palFldvalue[1]);
    
      _swTrim(palFldvalue[2]);
      strcpy((sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].sSwt_sys_route_m.
    	  trancode, palFldvalue[2]);

      /* next add by nh 2002.3.19 */
      (sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].sSwt_sys_route_m.flag = atoi(palFldvalue[3]);
      /* end add by nh */

      
      for(i = 0;i < sgLcmidx.iCount_swt_sys_route_g;i ++)
      {
        if (atoi(palFldvalue[0]) == (sgLcmidx.psShm_sys_route_g)[i].sSwt_sys_route_g.route_grp)
        {
          (sgLcmidx.psShm_sys_route_m)[ilCount_route_m - 1].lOffset = i;
          break;
        }
      }
      if (i == sgLcmidx.iCount_swt_sys_route_g)
      {
        swVdebug(0,"S5380: [错误/其它] swShm_swt_sys_route error 模糊匹配组号[%d]不存在! ", atoi(palFldvalue[0]));
        goto ErrorExit;
      }
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  
  sgLcmidx.iCount_swt_sys_route_m = ilCount_route_m;
/*end of add by dgm 2001.09.26 */

  
  /* 排序 */
  /* swt_sys_route_m: order by q_id, trancode */
  for (i = 1; i <= ilCount_route_m - 1; i ++)
  {
    for (j = 0; j < ilCount_route_m - 1; j ++)
    {
      ilSwapflag = 0;
      if (sgLcmidx.psShm_sys_route_m[j].sSwt_sys_route_m.q_id > sgLcmidx.psShm_sys_route_m[j+1].sSwt_sys_route_m.q_id)
        ilSwapflag = 1;
      else if (sgLcmidx.psShm_sys_route_m[j].sSwt_sys_route_m.q_id == sgLcmidx.psShm_sys_route_m[j+1].sSwt_sys_route_m.q_id)
      {
        ilRc = strcmp(sgLcmidx.psShm_sys_route_m[j].sSwt_sys_route_m.trancode,sgLcmidx.psShm_sys_route_m[j+1].sSwt_sys_route_m.trancode);
        if (ilRc > 0)
          ilSwapflag = 1;
      }
      if (ilSwapflag == 1)
      {
        memcpy(&slShm_sys_route_m,&sgLcmidx.psShm_sys_route_m[j],sizeof(struct shm_sys_route_m));
        memcpy(&sgLcmidx.psShm_sys_route_m[j],&sgLcmidx.psShm_sys_route_m[j+1],sizeof(struct shm_sys_route_m));
        memcpy(&sgLcmidx.psShm_sys_route_m[j+1],&slShm_sys_route_m,sizeof(struct shm_sys_route_m));
      }
    }
  }
#endif

  return 0;

  ErrorExit:
    if ((sgLcmidx.psShm_sys_route_g) != NULL) 
    {
      free(sgLcmidx.psShm_sys_route_g);
      sgLcmidx.psShm_sys_route_g = NULL;
    } 
    if ((sgLcmidx.psShm_sys_route_m) != NULL) 
    {
      free(sgLcmidx.psShm_sys_route_m);
      sgLcmidx.psShm_sys_route_m = NULL;
    }
    if ((sgLcmidx.psSwt_sys_route_d) != NULL) 
    {
      free(sgLcmidx.psSwt_sys_route_d);
      sgLcmidx.psSwt_sys_route_d = NULL;
    }
    fclose(fp);
    if (pp != NULL) 
    {
      pclose(pp);
      pp = NULL;
    }
    return -1;
}

#ifdef HASH
/***************************************************************
 ** 函数名      : swShm_swt_sys_route_m
 ** 功  能      : 读路由[SWMATCH_ROUTE.CFG]配置文件
 ** 作  者      : baiqj
 ** 建立日期    : 2015/05/15
 ** 最后修改日期: 2015/05/15
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_route_m(int *pCount,void *pHash)
{
  short ilPriority;
  int i;
  char szCmd[128];
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc, ilCount;
  struct shm_sys_route_m slShm_sys_route_m;
  char tmp[128];
  int tabSize, dataSize, indexSize;
  int recordNumber, recordSize, keySize;
  int rc = 0;
  FILE *pfp = NULL;
  FILE *fp = NULL;

  memset(szCmd, 0x00, sizeof(szCmd));
  memset(tmp, 0x00, sizeof(tmp));
  sprintf(szCmd, "wc -l %s/config/SWMATCH_ROUTE.CFG", getenv("SWITCH_CFGDIR"));

  if((pfp = popen(szCmd, "r")) == NULL)
  {
      swVdebug(1, "popen err [%s]", szCmd);
      return -1;
  }

  fgets(tmp, 128, pfp);
  _swTrim(tmp);
  rc = atoi(tmp);

  recordNumber = rc;
  recordSize = sizeof(struct shm_sys_route_m);
  keySize = 64;

  indexSize = idxSize(keySize, recordNumber);
  dataSize = recordNumber * recordSize;
  tabSize = indexSize + dataSize + sizeof(long) + sizeof(int);

  *pCount=tabSize;
  swVdebug(5, "swShm_swt_sys_route_m tabSize[%d],recordNumber[%d],indexSize[%d]", tabSize, recordNumber, indexSize);

  if (pHash == NULL)
      return 0;

  long dataAreaOffset = 0; // the first area is the offset of data area
  dataAreaOffset = sizeof(long) +sizeof(int) + indexSize;

  memcpy(pHash, (void *)&dataAreaOffset, sizeof(long));
  memcpy(pHash + sizeof(long), (void *)&recordNumber, sizeof(int));//the  second area is total record number of data
  char * indexPtr = pHash + sizeof(long)+sizeof(int); // the third area is index area
  char * dataPtr = indexPtr + indexSize;  // the last area is data area

  // init index area
  idxInit(indexPtr, keySize, recordNumber);

  if ((fp = _swFopenlist("config/SWMATCH_ROUTE.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5370: _swFopen(SWMATCH_ROUTE.CFG),errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_route_m = 0;
      return 0;
    }  
  }
  
  ilCount = 0;
  int offset = 0;
  char keyData[64];
  while (fp)
  {
    while(1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0)
        break;
      ilCount ++;
      memset(&slShm_sys_route_m,0x00,sizeof(struct shm_sys_route_m));

    
      slShm_sys_route_m.sSwt_sys_route_m.route_grp = atoi(palFldvalue[0]);

      slShm_sys_route_m.sSwt_sys_route_m.q_id = atoi(palFldvalue[1]);
    
      _swTrim(palFldvalue[2]);
      strcpy(slShm_sys_route_m.sSwt_sys_route_m.trancode, palFldvalue[2]);

      /* next add by nh 2002.3.19 */
      slShm_sys_route_m.sSwt_sys_route_m.flag = atoi(palFldvalue[3]);
      /* end add by nh */

      
      for(i = 0;i < sgLcmidx.iCount_swt_sys_route_g;i ++)
      {
        if (atoi(palFldvalue[0]) == (sgLcmidx.psShm_sys_route_g)[i].sSwt_sys_route_g.route_grp)
        {
          slShm_sys_route_m.lOffset = i;
          break;
        }
      }
      if (i == sgLcmidx.iCount_swt_sys_route_g)
      {
        swVdebug(0,"S5380: [错误/其它] swShm_swt_sys_route_n error 模糊匹配组号[%d]不存在! ", atoi(palFldvalue[0]));
        goto ErrorExit;
      }
    
      memset(keyData, 0x00, sizeof(keyData));
      /*根据邮箱号和交易码匹配组号*/
      sprintf(keyData, "%d|%s",slShm_sys_route_m.sSwt_sys_route_m.q_id,slShm_sys_route_m.sSwt_sys_route_m.trancode);
      swVdebug(5, "swShm_swt_sys_route_m keyData [%s]", keyData);
      
       memcpy(dataPtr + offset, (char *)&slShm_sys_route_m, recordSize);
       if ((rc = idxInsert(indexPtr, keyData, offset)) < 0)
       {
           swVdebug(1, "swShm_swt_sys_route_m idxInsert error keyData[%s] offset[%d] rc[%d]", keyData, offset, rc);
       }
       else
       {
           swVdebug(5, "swShm_swt_sys_fmt_m idxInsert right keyData[%s] offset[%d]", keyData, offset);
       }
       offset += recordSize;
      
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }  
  sgLcmidx.iCount_swt_sys_route_m = ilCount;
/*end of add by dgm 2001.09.26 */
 return 0;

 
  ErrorExit:
    if ((sgLcmidx.psShm_sys_route_g) != NULL) 
    {
      free(sgLcmidx.psShm_sys_route_g);
      sgLcmidx.psShm_sys_route_g = NULL;
    } 
    if ((sgLcmidx.psShm_sys_route_m) != NULL) 
    {
      free(sgLcmidx.psShm_sys_route_m);
      sgLcmidx.psShm_sys_route_m = NULL;
    }
    if ((sgLcmidx.psSwt_sys_route_d) != NULL) 
    {
      free(sgLcmidx.psSwt_sys_route_d);
      sgLcmidx.psSwt_sys_route_d = NULL;
    }
    fclose(fp);
    return -1;

}
#endif

/* add by gengling at 2015.03.19 begin PSBC_V1.0 */
/***************************************************************
 ** 函数名      : swShm_swt_sys_tran
 ** 功  能      : 读信息代码[SWTRAN_CTL.CFG]配置文件
 ** 作  者      : 
 ** 建立日期    : 2015/03/19
 ** 最后修改日期: 2015/03/19
 ** 调用其它函数: swGetitemmode2
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
short swShm_swt_sys_tran()
{
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  int ilRc,ilCount;
  struct swt_sys_tran slSwt_sys_tran;
  FILE *fp;
  if ((fp = _swFopenlist("config/SWTRAN_CTL.CFG", "r")) == NULL)
  {
    if (errno != ENOENT)
    {
      swVdebug(0,"S5095: _swFopenlist(SWTRAN_CTL.CFG),errno=%d,strerror(errno)",errno);
      return -1;
    }
    else
    {
      sgLcmidx.iCount_swt_sys_tran = 0;
      return 0;
    }  
  }
    
  ilCount = 0;
  while (fp)
  {
    while (1)
    {
      ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
      if (ilRc < 0) break;

      ilCount ++;
      memset(&slSwt_sys_tran,0x00,sizeof(struct swt_sys_tran));

      slSwt_sys_tran.sys_id = atoi(palFldvalue[0]);
      strcpy(slSwt_sys_tran.tran_code, palFldvalue[1]);
      strcpy(slSwt_sys_tran.tran_name, palFldvalue[2]);
      slSwt_sys_tran.priority = atoi(palFldvalue[3]);
      strcpy(slSwt_sys_tran.status, palFldvalue[4]);
      slSwt_sys_tran.tranning_max= atol(palFldvalue[5]);
      slSwt_sys_tran.debug_level = (char) atoi( palFldvalue[6] ); /*modified by dyw, 2015.03.27, PSBC_V1.0*/

      if (ilCount == 1)
        sgLcmidx.psSwt_sys_tran = (struct swt_sys_tran *)
      	  	malloc(ilCount * sizeof(struct swt_sys_tran));
      else
        sgLcmidx.psSwt_sys_tran = (struct swt_sys_tran *)
      	  realloc(sgLcmidx.psSwt_sys_tran, ilCount*sizeof(struct swt_sys_tran));
      if ((sgLcmidx.psSwt_sys_tran) == NULL) return -1;

      memcpy((char *)&((sgLcmidx.psSwt_sys_tran)[ilCount-1]), 
       		(char *)&slSwt_sys_tran,sizeof(struct swt_sys_tran));
    }
    fclose(fp);
    fp = _swFopenlist(NULL, "r");
  }
  sgLcmidx.iCount_swt_sys_tran = ilCount;  		
  return 0;
}
/* add by gengling at 2015.03.19 end PSBC_V1.0 */

#ifdef HASH
short swShm_swt_sys_tran_ctrl(int *pCount, void *pHash)
{
    char szCmd[128];
    char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
    int ilRc, ilCount;
    struct swt_sys_tran slSwt_sys_tran;
    char tmp[128];
    int tabSize, dataSize, indexSize;
    int recordNumber, recordSize, keySize;
    int rc = 0;
    FILE *pfp = NULL;
    FILE *fp = NULL;

    memset(szCmd, 0x00, sizeof(szCmd));
    memset(tmp, 0x00, sizeof(tmp));
    sprintf(szCmd, "wc -l %s/config/SWTRAN_CTL.CFG", getenv("SWITCH_CFGDIR"));

    if((pfp = popen(szCmd, "r")) == NULL)
    {
        swVdebug(1, "popen err [%s]", szCmd);
        return -1;
    }

    fgets(tmp, 128, pfp);
    _swTrim(tmp);
    rc = atoi(tmp);

    recordNumber = rc;
    recordSize = sizeof(struct swt_sys_tran);
    keySize = 64;

    indexSize = idxSize(keySize, recordNumber);
    dataSize = recordNumber * recordSize;
    tabSize = indexSize + dataSize + sizeof(long) + sizeof(int);

    *pCount=tabSize;
    swVdebug(5, "tabSize[%d],recordNumber[%d],indexSize[%d]", tabSize, recordNumber, indexSize);

    if (pHash == NULL)
        return 0;

    long dataAreaOffset; // the first area is the offset of data area
    dataAreaOffset = sizeof(long) + sizeof(int) + indexSize;

    // 拷贝sizeof(long)个字节对应数据的偏移量到smShmbuf_s.psSwt_sys_tran_hash的首地址
    memcpy(pHash, (void *)&dataAreaOffset, sizeof(long));
    // 拷贝sizeof(int)个字节对应的数据记录个数到smShmbuf_s.psSwt_sys_tran_hash+sizeof(long)的地址
    memcpy(pHash+sizeof(long), (void *)&recordNumber, sizeof(int));// the second area is the number of total data area

    char * indexPtr = pHash + sizeof(long) + sizeof(int); // the third area is index area
    char * dataPtr = indexPtr + indexSize;  // the last area is data area

    // init index area
    idxInit(indexPtr, keySize, recordNumber);

    if ((fp = _swFopenlist("config/SWTRAN_CTL.CFG", "r")) == NULL)
    {
        if (errno != ENOENT)
        {
            swVdebug(0,"S5095: _swFopenlist(SWTRAN_CTL.CFG),errno=%d,strerror(errno)",errno);
            return -1;
        }
        else
        {
            sgLcmidx.iCount_swt_sys_tran = 0;
            return 0;
        }  
    }

    ilCount = 0;
    int offset = 0;
    char keyData[64];
    while (fp)
    {
        while (1)
        {
            ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
            if (ilRc < 0) break;

            ilCount ++;
            memset(&slSwt_sys_tran,0x00,sizeof(struct swt_sys_tran));

            slSwt_sys_tran.sys_id = atoi(palFldvalue[0]);
            strcpy(slSwt_sys_tran.tran_code, palFldvalue[1]);
            strcpy(slSwt_sys_tran.tran_name, palFldvalue[2]);
            slSwt_sys_tran.priority = atoi(palFldvalue[3]);
            strcpy(slSwt_sys_tran.status, palFldvalue[4]);
            slSwt_sys_tran.tranning_max= atol(palFldvalue[5]);
            slSwt_sys_tran.debug_level = (char) atoi( palFldvalue[6] ); /*modified by dyw, 2015.03.27, PSBC_V1.0*/

            memset(keyData, 0x00, sizeof(keyData));
            _swTrim(slSwt_sys_tran.tran_code);
            sprintf(keyData, "%d|%s", slSwt_sys_tran.sys_id, slSwt_sys_tran.tran_code);
            swVdebug(5, "keyData [%s]", keyData);

            memcpy(dataPtr + offset, &slSwt_sys_tran, recordSize);
            if ((rc = idxInsert(indexPtr, keyData, offset)) < 0)
            {
                swVdebug(1, "idxInsert error keyData[%s] offset[%d] rc[%d]", keyData, offset, rc);
            }
            else
            {
                swVdebug(5, "idxInsert right keyData[%s] offset[%d]", keyData, offset);
            }
            offset += recordSize;
        }
        fclose(fp);
        fp = _swFopenlist(NULL, "r");
    }
    sgLcmidx.iCount_swt_sys_tran = ilCount;
    return 0;
}
#endif

/***************************************************************
 ** 函数名      : swParseXMF
 ** 功  能      : 解析XMF报文TDF脚本
 ** 作  者      :
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swParseXMF(char *aBuf,struct swt_sys_fmt_d * psSwt_sys_fmt_d)
{
  /*modify by zcd 20141224
  char *alTmp1,*alTmp2,alTmp3[20],alTmp4[1024],*alTmp5;*/
  char *alTmp1,*alTmp2,alTmp3[20],alTmp4[iEXPRESSLEN],*alTmp5;
  int  ilCount,ilRc;
  char alTmpexp[2 * iFLDVALUELEN + 1];
  char alTmp[2 * iFLDVALUELEN + 1];  /* add by zjj 2004.02.28 */

  memset(psSwt_sys_fmt_d,0x00,sizeof(struct swt_sys_fmt_d));
  memset(alTmp4,0x00,sizeof(alTmp4));

  _swTrim(aBuf);

  
  memset(alTmp,0x00,sizeof(alTmp)); /* add by zjj 2004.02.28 */ 
  strcpy(alTmp,aBuf);  /* add by zjj 2004.02.28 */  
  alTmp5 = strchr(aBuf,' ');
  if (alTmp5 == NULL) 
  {
    swVdebug(0,"S5385: [错误/其它] 路由脚本[%s]中行号未定义!",alTmp);
    return(-1);
  }
  alTmp1 = alTmp5 + 1;
  *alTmp5 = '\0';
  _swTrim(alTmp1);  
  psSwt_sys_fmt_d->id = atoi(aBuf); 

  /*  处理路由脚本  */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,3);
    strtoupper(alTmp3);

    /* 是条件执行语句  */  
    if (strcmp(alTmp3,"IF ") == 0)
    {
      alTmp1 = alTmp1 + 3;
      _swTrim(alTmp1);
      memset(alTmp4,0x00,sizeof(alTmp4));
      strcpy(alTmp4,alTmp1);
      strtoupper(alTmp4);
      alTmp2 = alTmp1;
      alTmp5 = strstr(alTmp4," THEN ");
      if (alTmp5 == NULL) 
      {
        swVdebug(0,"S5390: [错误/其它] 格式转换[%s]脚本定义错误(有IF无THEN)!",alTmp);
        return(-1);
      }
      ilCount = alTmp5 - alTmp4;
      *(alTmp1 + ilCount) = '\0';
      alTmp1 = alTmp1 + ilCount + 6;
      _swTrim(alTmp1);
      _swTrim(alTmp2);
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp2);
      if (ilRc != SUCCESS)
      {
      	swVdebug(0,"S55392: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp2,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S55392: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fmt_cond,alTmpexp); 
      /* end add by zjj 2001.11.29 */
      
    /* strcpy(psSwt_sys_fmt_d->fmt_cond,alTmp2); */ /* delete by zjj */
    }

    /* GOTO 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"GOTO ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"A");
      alTmp2 = alTmp1 + 5;
      _swTrim(alTmp2);
      psSwt_sys_fmt_d->fld_id = atol(alTmp2); //modified by mzg 20150428
      return(0);
    }

    /* VARID */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,6);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"VARID ") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'B';
      alTmp1 = alTmp1 + 6;
      _swTrim(alTmp1);
      if ((alTmp5 = strchr(alTmp1, '=')) == NULL)
      {
        swVdebug(0, "S5395: [错误/其它] [%s] 中表达式[%s]错误",alTmp, alTmp1);      	
        return(FAIL);
      }
      *alTmp5 = '\0';
      alTmp5 ++;
      _swTrim(alTmp5);
      psSwt_sys_fmt_d->fld_id = atol(alTmp1); //modified by mzg 20150428
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp5);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S55396: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp5,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S55397: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fld_express,alTmpexp);
      /* end add by zjj 2001.11.29 */
      
   /* strcpy(psSwt_sys_fmt_d->fld_express,alTmp5); */ /* del by zjj */
      return(0);
    }

    /* VAR 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"VAR ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"C");
      alTmp1 = alTmp1 + 4;
      _swTrim(alTmp1);
      if ((alTmp5 = strchr(alTmp1, '=')) == NULL)
      {
      	swVdebug(0, "S5400: [错误/其它] [%s] 表达式[%s]错误", alTmp, alTmp1);
        return(FAIL);
      }
      *alTmp5 = '\0';
      alTmp5 ++;
      _swTrim(alTmp1);
      _swTrim(alTmp5);
      
      ilRc = swGetimfname(alTmp1,psSwt_sys_fmt_d->imf_name,
        psSwt_sys_fmt_d->imf_array);   /* add by bmy 2002.3.12 */

      /* strcpy(psSwt_sys_fmt_d->imf_name,alTmp1); delete by bmy 2002.3.12 */
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp5);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S5401: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp5,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S55402: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fld_express,alTmpexp);
      /* end add by zjj 2001.11.29 */
      
   /* strcpy(psSwt_sys_fmt_d->fld_express,alTmp5); */ /* del by zjj */
      return(0);
    }

    /* SET 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"SET ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"D");
      alTmp1 = alTmp1 + 4;
      _swTrim(alTmp1);
      if ((alTmp5 = strchr(alTmp1, '=')) == NULL)
      {
      	swVdebug(0, "S5405: [错误/其它] [%s] 表达式[%s]错误", alTmp, alTmp1);
        return(FAIL);
      }
      *alTmp5 = '\0';
      alTmp5 ++;
      _swTrim(alTmp1);
      _swTrim(alTmp5);

      ilRc = swGetimfname(alTmp1,psSwt_sys_fmt_d->imf_name,
        psSwt_sys_fmt_d->imf_array);   /* add by bmy 2002.3.12 */

      /* strcpy(psSwt_sys_fmt_d->imf_name,alTmp1); delete by bmy 2002.3.12 */
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp5);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp5,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fld_express,alTmpexp);
      /* end add by zjj 2001.11.29 */
      
   /* strcpy(psSwt_sys_fmt_d->fld_express,alTmp5); */ /* del by zjj */
      return(0);
    }

    /* TDF 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"TDF ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"E");
      alTmp1 = alTmp1 + 4;
      _swTrim(alTmp1);
      if ((alTmp5 = strchr(alTmp1, ',')) == NULL)
      {
        psSwt_sys_fmt_d->fld_id = atol(alTmp1); //modified by mzg 20150428
        psSwt_sys_fmt_d->fld_express[0] = '\0';
      }
      else
      {
        *alTmp5 = '\0';
        alTmp5 ++;
        psSwt_sys_fmt_d->fld_id = atol(alTmp1); //modified by mzg 20150428
        
       /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp5);
      if (ilRc != SUCCESS)
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp5,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fld_express,alTmpexp);
      /* end add by zjj 2001.11.29 */
      
   /* strcpy(psSwt_sys_fmt_d->fld_express,alTmp5); */ /* del by zjj */
      } 
      return(0);
    }

    /* RETURN 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,7);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"RETURN ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"F");
      alTmp1 = alTmp1 + 7;
      _swTrim(alTmp1);
      psSwt_sys_fmt_d->fld_id = atol(alTmp1); //modified by mzg 20150428
      return(0);
    }

    /* TRACE 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,6);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"TRACE ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"G");
      alTmp1 = alTmp1 + 6;
      _swTrim(alTmp1);
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp1);
      if (ilRc != SUCCESS)
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp1,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fld_express,alTmpexp);
      /* end add by zjj 2001.11.29 */
      
   /* strcpy(psSwt_sys_fmt_d->fld_express,alTmp1); */ /* del by zjj */
      return(0);
    }

    /* DO 语句 */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,3);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"DO ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"H");
      alTmp1 = alTmp1 + 3;
      _swTrim(alTmp1);
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp1);
      if (ilRc != SUCCESS)
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
      	return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp1,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S0000: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fld_express,alTmpexp);
      /* end add by zjj 2001.11.29 */
      
   /* strcpy(psSwt_sys_fmt_d->fld_express,alTmp1); */ /* del by zjj */
      return(0);
    }

    /* FIELD 语句 */ 
    memset(alTmp3,0x00,sizeof(alTmp3)); 
    strncpy(alTmp3,alTmp1,6); 
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"FIELD ") == 0)
    {
      strcpy(psSwt_sys_fmt_d->fmt_flag,"I");
      alTmp1 = alTmp1 + 6;
      _swTrim(alTmp1);
      if ((alTmp5 = strchr(alTmp1, '=')) == NULL)
      {
      	swVdebug(0,"S5410: [错误/其它] [%s]表达式[%s]错误", alTmp, alTmp1);
        return(FAIL);
      }
      *alTmp5 = '\0';
      alTmp5 ++;
      _swTrim(alTmp5);
      psSwt_sys_fmt_d->fld_id = atol(alTmp1); //modified by mzg 20150428
      
      /* next 2 row add by nh 2002.7.3 */
      ilRc = swExpSign(alTmp5);
      if (ilRc != SUCCESS) 
      {
      	swVdebug(0,"S5411: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      
      /* add by zjj 2001.11.29 */
      ilRc = _swExpN2T(alTmp5,alTmpexp);
      if (ilRc != 0) 
      {
      	swVdebug(0,"S5412: [错误/其它] 脚本[%s]中有错误",alTmp);
        return(ilRc);
      }
      strcpy(psSwt_sys_fmt_d->fld_express,alTmpexp);
      /* end add by zjj 2001.11.29 */
      
   /* strcpy(psSwt_sys_fmt_d->fld_express,alTmp5); */ /* del by zjj */
      return(0);
    }
/* add by zjj 2001.11.20  */    
    /* WHILE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"WHILE") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'J';
      alTmp1 = alTmp1 + 5;
      _swTrim(alTmp1);
      /* 由于此处的表达式是中间临时表达式，所以不用做逆波兰转换 by zjj 2001.11.29*/
      strcpy(psSwt_sys_fmt_d->fmt_cond,alTmp1);
      return(0);
    }

    /* CONTINUE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,8);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"CONTINUE") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'K';
      return(0);
    }

    /* EXIT */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"EXIT") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'L';
      return(0);
    }
      
    /* ENDWHILE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,8);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"ENDWHILE") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'M';
      return(0);
    }
    
    /* SWITCH */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,6);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"SWITCH") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'N';
      alTmp1 = alTmp1 + 6;
      _swTrim(alTmp1);
      /* 由于此处的表达式是中间临时表达式，所以不用做逆波兰转换 by zjj 2001.11.29*/
      strcpy(psSwt_sys_fmt_d->fmt_cond,alTmp1);
      return(0);
    }

    /* CASE */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,4);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"CASE") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'O';
      alTmp1 = alTmp1 + 4;
      _swTrim(alTmp1);
      /* 由于此处的表达式是中间临时表达式，所以不用做逆波兰转换 by zjj 2001.11.29*/
      strcpy(psSwt_sys_fmt_d->fmt_cond,alTmp1);
      return(0);
    }

    /* BREAK */  
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,5);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"BREAK") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'P';
      return(0);
    }

    /* DEFAULT */  
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,7);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"DEFAULT") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'Q';
      return(0);
    }

    /* ENDSWITCH */
    memset(alTmp3,0x00,sizeof(alTmp3));
    strncpy(alTmp3,alTmp1,9);
    strtoupper(alTmp3);
    if (strcmp(alTmp3,"ENDSWITCH") == 0)
    {
      psSwt_sys_fmt_d->fmt_flag[0] = 'R';
      return(0);
    }
/* end add by zjj 2001.11.20 */
  swVdebug(0, "S0000: [错误/其它] 解析脚本[%s]错误!", alTmp);
  return(-1);
}

/* add by bmy 2002.03.12 */
static int swGetimfname(char *aSrc,char *aName,char *aArray)
{
  char *alTmp1, *alTmp2, alSrc[iEXPRESSLEN+1];

  aArray[0] = '\0';
  strcpy(alSrc, aSrc);
  if ((alTmp1 = strchr(alSrc, '[')) != NULL)
  {
    *alTmp1 = '\0';
    alTmp1 ++;
    if ((alTmp2 = strchr(alTmp1, ']')) != NULL)
    {
      *alTmp2 = '\0';
      _swTrim(alTmp1);
      strcpy(aArray, alTmp1);
      _swTrim(alSrc);
      strcpy(aName, alSrc);
    }
    else
    {
      strcpy(aName,aSrc);
    }
  }
  else
  {
    strcpy(aName, alSrc);
  }
  return 0;
}
/* end of add by bmy */

/***************************************************************
 ** 函数名      : swParseL
 ** 功  能      : 解析运算符左边的串
 ** 作  者      : 
 ** 建立日期    : 2002/07/3
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swPraseL(char *aOrg,char *aDes,unsigned int iLen,short *iFlag)
{
  int i,ilLevel=1;
  int ilEnd=0,ilFlag=0;

  swVdebug(4,"S5415: begin swPraseL(%s,%s,%d,%d)...",aOrg,aDes,iLen,*iFlag);
  for(i=iLen-1;i>=0;i--)
  { 
    if(i == 0 && aOrg[i] != '(')
    {
      ilEnd = i;
      ilFlag = 2;
      break;
    }
    else
    {
      switch(aOrg[i])
      {
        case ')':
          ilLevel ++;
          break;
        case '(':
          ilLevel --;
          if(ilLevel <= 0)
          {
            ilEnd = i+1;
            ilFlag = 1;
          }
          break;
        case ',':
          if(ilLevel <= 1)
          {
            ilEnd = i+1;
            ilFlag = 1;
          }
          break;
        case '+':
        /* 预留接口
        case '-':
        case '*':
        case '/':
        */
          if(ilLevel <= 1)
          {
            ilEnd = i+1;
            ilFlag = 1;
          }
          break;
        default:
          break;
      }
    }
    if(ilFlag == 1)
      break;
  }
  if(ilLevel < 0 || i < 0)
  {
    swVdebug(0,"S5420: swPraseL error");
    return(-1);
  }
  if(ilFlag == 2)
  {
    memcpy(aDes,aOrg+ilEnd,iLen - ilEnd);
    aDes[iLen-ilEnd]=0;
    *iFlag = ilEnd;
  }
  else
  {
    *iFlag = ilEnd - 1;
    memcpy(aDes,aOrg+ilEnd,iLen - ilEnd);
    aDes[iLen-ilEnd]=0;
  }
   
  swVdebug(4,"S5425: end swPraseL: aDes=[%s],iFlag=[%d]",aDes,*iFlag);
  return(0);
}

/***************************************************************
 ** 函数名      : swParseR
 ** 功  能      : 解析运算符右边的串
 ** 作  者      : nh
 ** 建立日期    : 2002/07/3
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swPraseR(char *aOrg,char *aDes,unsigned int iLen,short *iFlag)
{
  int i,ilLevel=1;
  int ilEnd=0,ilFlag=0;
  
  swVdebug(4,"S5430:begin swPraseR(%s,%s,%d,%d)...",aOrg,aDes,iLen,*iFlag);
  for(i=iLen+1;;i++)
  { 
    if(i > strlen(aOrg))
      return(-1);
    else if(i == strlen(aOrg) && aOrg[i] != ')' )
    {
      ilEnd = i;
      ilFlag = 2;
      break;
    }
    else
    {
      switch(aOrg[i])
      {
        case '(':
          ilLevel ++;
          break;
        case ')':
          ilLevel --;
          if(ilLevel <= 0)
          {
            ilEnd = i-1;
            ilFlag = 1;
          }
          break;
        case ',':
          if(ilLevel <= 1)
          {
            ilEnd = i-1;
            ilFlag = 1;
          }
          break;
        case '+':
        /* 预留接口
        case '-':
        case '*':
        case '/':
        */
          if(ilLevel <= 1)
          {
            ilEnd = i-1;
            ilFlag = 1;
          }
          break;
        default:
          break;
      }
    }
    if(ilFlag == 1)
      break;
  }
  if(ilLevel < 0 || i > strlen(aOrg) + 1)
  {
    swVdebug(0,"S5435: swPraseR error");
    return(-1);
  }
  if(ilFlag == 2)
  {
    memcpy(aDes,aOrg+iLen+1,ilEnd - iLen);
    aDes[ilEnd-iLen]=0;
    *iFlag = ilEnd;
  }
  else
  {
    *iFlag = ilEnd + 1;
    memcpy(aDes,aOrg+iLen+1,ilEnd - iLen);
    aDes[ilEnd-iLen]=0;
  }
  
  swVdebug(4,"S5440: end swPraseR: aDes=[%s],iFlag=[%d]",aDes,*iFlag);
  return(0);
}
     
/***************************************************************
 ** 函数名      : swUnite
 ** 功  能      : 合并运算符两边的串
 ** 作  者      : nh
 ** 建立日期    : 2002/07/3
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swUnite(char *aBufL,char *aBufR,char cFlag,char *aResult)
{
  swVdebug(4,"S5445: begin swUnite([%s],[%s],[%c],[%s]).",aBufL,aBufR,cFlag,aResult);
  switch(cFlag)
  {
    case '+':
      sprintf(aResult,"%s%s%s%s%s","SAD(",aBufL,",",aBufR,")");
      break;
    /* 预留
    case '-':
      sprintf(aResult,"%s%s%s%s%s","SUB(",aBufL,",",aBufR,")");
      break;
    case '*':
      sprintf(aResult,"%s%s%s%s%s","MULT(",aBufL,",",aBufR,")");
      break;
    case '/':
      sprintf(aResult,"%s%s%s%s%s","DIV(",aBufL,",",aBufR,")");
      break;
    */
    default:
      break;
  }
  
  swVdebug(4,"S5450: end swUnite:aResult=[%s].",aResult);
  return(0);
}

/***************************************************************
 ** 函数名      : swParseR
 ** 功  能      : 解析运算符，目前只有'+'
 ** 作  者      : nh
 ** 建立日期    : 2002/07/3
 ** 最后修改日期: 
 ** 调用其它函数: swPraseL();swPraseR();swUnite();
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swExpSign(char *aBuf)
{
  int i=0,j=0,k=0;
  int ilTmp[101];
  short ilEnd,ilBegin;
  char alResult[101];
  char alTmp[101];
  char alBufL[101];
  char alBufR[101];
  int ilFlag=0;
  char *alSign="( )+";
  int ilLength=0;
  short ilRc;

#ifdef NOTSIGN
  return 0;
#endif  

  swVdebug(4,"S5455: begin swExpSign:org=[%s]",aBuf);

  if(strchr(aBuf,'+') == NULL)
  {
    swVdebug(4,"S5460: no sign,return OK!");
    return(0);
  }
  
  for(i=0;i<101;i++)
    ilTmp[i]=0;

  for(i=0;aBuf[i]!= 0;i++)
  {
    switch(aBuf[i])
    {
      case '(':
        /*
        if(i == 0)
          ilFlag += 100;
        else if(strchr(alSign,aBuf[i-1]) != NULL)  
        */
        ilFlag += 100;
        break;
      case ')':
        ilFlag -= 100;
        break;
      case '+':
      /* case '-': */
        if(i == 0)
          return(-1);
        else
          ilTmp[i] = 1 + ilFlag;
        break;
      /*
      case '*':
      case '/':
        if(i == 0)
          return(-1);
        else
          ilTmp[i] = 10 + ilFlag;
        break;
      */
      default:
        break;
    }
  }

  for(i=1;i<101;i++)
  {
    if(ilTmp[i] > k && ilTmp[i] != 100)
    {
      k=ilTmp[i];
      j=i;
    }
  }
  if(j==0)
    return(0);
  else
  {
    ilRc = swPraseL(aBuf,alBufL,j,&ilBegin);
    if(ilRc)
    {
      swVdebug(0,"S5465: swExpSign() praseL error");
      return(-1);
    }
    swVdebug(4,"S5470: swExpSign praseL:aBuf=[%s],alBufL=[%s],j=[%d],ilBegin=[%d]",aBuf,alBufL,j,ilBegin);        
    
    swPraseR(aBuf,alBufR,j,&ilEnd);
    if(ilRc)
    {
      swVdebug(0,"S5475: swExpSign() praseR error");
      return(-1);
    }    
    swVdebug(4,"S5480: swExpSign praseR:aBuf=[%s],alBufR=[%s],j=[%d],ilEnd=[%d]",aBuf,alBufR,j,ilEnd);
    
    swUnite(alBufL,alBufR,aBuf[j],alResult);
    if(ilRc)
    {
      swVdebug(0,"S5485: swExpSign() unite error");
      return(-1);
    }
    swVdebug(4,"S5490: swExpSign unite:alBufL=[%s],alBufR=[%s],alResult=[%s]",alBufL,alBufR,alResult);
    
    memset(alTmp,0x00,sizeof(alTmp));
    if(ilBegin != 0)
    {
      if(aBuf[ilBegin] != '(' || (strchr(alSign,aBuf[ilBegin-1	]) == NULL))
        ilBegin += 1;
      memcpy(alTmp,aBuf,ilBegin);
    }
    
    memcpy(alTmp+ilBegin,alResult,strlen(alResult));
    if(aBuf[ilEnd]==')')
      memcpy(alTmp+ilBegin+strlen(alResult),aBuf + ilEnd + 1,strlen(aBuf)-ilEnd-1);
    else
      memcpy(alTmp+ilBegin+strlen(alResult),aBuf + ilEnd,strlen(aBuf)-ilEnd); 
    ilLength = strlen(aBuf)-ilEnd + ilBegin + strlen(alResult);
    strcpy(aBuf,alTmp);
    aBuf[ilLength]=0;
  }
  swVdebug(4,"S5495: swExpSign endbuf=[%s]",aBuf);

  ilRc=swExpSign(aBuf);
  if(ilRc)
  {
    swVdebug(0,"S5500: swExpSign() error");
    return(-1);
  }
  
  swVdebug(4,"S5505: end swExpSign() return SUCCESS");  
  return(0);
}

/*del by zcd 20141222
int swCheckfmt(short **piFmt,char *aBuf)
***end of del by zcd 20141222*/
/*add by zcd 20141222*/
int swCheckfmt(long **piFmt,char *aBuf)
/*end of add by zcd 20141222*/
{
  /*del by zcd 20141222
  short ilFmt=0,i=0;
  ***end of del by zcd 20141222*/
  long ilFmt,i=0;
  char alTmp[80];

  strcpy(alTmp,aBuf);
  if((strstr(alTmp,".FMT"))==NULL)
  {
    swVdebug(0,"S5510: swCheckfmt() 输入的格式转换组号[%s]错误",aBuf);
    return(-1);
  }
  alTmp[strlen(alTmp)-4]=0;
  ilFmt=atoi(alTmp);
  if(ilFmt <= 0)
  {
    swVdebug(0,"S5515: swCheckfmt(),fmt error");
    return(-1);
  }
  if(*piFmt == NULL)
  {
	/*del by zcd 20141222
    *piFmt=(short *)malloc(sizeof(short));
	***end of del by zcd 20141222*/
	/*add by zcd 20141222*/
	*piFmt=(long *)malloc(sizeof(long));
	/*end of add by zcd 20141222*/
    (*piFmt)[i]=ilFmt;
    return(0);
  }
  while((*piFmt)[i] != 0)
  {
    if((*piFmt)[i] == ilFmt)
    {
      fprintf(stdout,"警告: 格式转换组号[%s]重复\n",aBuf);
      return(-1);
    }
    i++;
  }
  /*del by zcd 20141222
  *piFmt=(short *)realloc(*piFmt,(i+1)*sizeof(short));
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  *piFmt=(long *)realloc(*piFmt,(i+1)*sizeof(long));
  /*end of add by zcd 20141222*/ 
  if(*piFmt == NULL)
  {
    swVdebug(0,"S5520: swCheckfmt() realloc error");
    return(-1);
  }
  (*piFmt)[i]=ilFmt;

  return(0);
}

int swCheckroute(short **piRoute,char *aBuf)
{
  short ilRoute=0,i=0;
  char alTmp[80];

  strcpy(alTmp,aBuf);
  if((strstr(alTmp,".SCR"))==NULL)
  {
    swVdebug(0,"S5525: swCheckroute() 输入的路由组号[%s]错误",aBuf);
    return(-1);
  }
  alTmp[strlen(alTmp)-4]=0;
  ilRoute=atoi(alTmp);
  if(ilRoute <= 0)
  {
    swVdebug(0,"S5530: swCheckroute(),route error");
    return(-1);
  }
  if(*piRoute == NULL)
  {
    *piRoute=(short *)malloc(sizeof(short));
    (*piRoute)[i]=ilRoute;
    return(0);
  }
  while((*piRoute)[i] != 0)
  {
    if((*piRoute)[i] == ilRoute)
    {
      fprintf(stdout,"警告: 路由组号[%s]重复\n",aBuf);
      return(-1);
    }
    i++;
  }
  *piRoute=(short *)realloc(*piRoute,(i+1)*sizeof(short));
  if(*piRoute == NULL)
  {
    swVdebug(0,"S5535: swCheckroute() realloc error");
    return(-1);
  }
  (*piRoute)[i]=ilRoute;
  return(0);
}

