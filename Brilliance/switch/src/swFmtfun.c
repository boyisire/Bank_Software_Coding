/****************************************************************/
/* 模块编号    ：FMTFUN                                         */ 
/* 模块名称    ：Format公用函数                                 */
/* 版 本 号    ：V3.0.1                                         */
/* 作	 者    ：张辉                                           */
/* 建立日期    ：1999/12/7                                      */
/* 最后修改日期：2001/4/6                                       */
/* 模块用途    ：Format的函数                                   */
/* 本模块中包含如下函数及功能说明：                             */
/*                   (1)int swCmpFmtTrantype();                 */
/*                   (4)int swSepunpack();                      */
/*                   (6)int swGetFMLfromTDF();                  */
/*                   (7)int swGetISOfromTDF();                  */
/*                   (8)int swGetSEPfromTDF();                  */
/*                   (9)int swFMLcheck();                       */
/*                  (10)int swISOcheck();                       */
/*                  (11)int swSeppack();                        */
/*                  (15)int swHextoasc();                       */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/*   2000/8/24  优化                                            */
/*   2001/2/22  Oracle移植                                      */
/*   2001/2/27  增加DEBUG级别                                   */
/*   2001/3/24  封版V3.01,程序共1500行                          */
/*   2001/3/29  暂时对swGetFMLfromTDF的空域进行修改(1524)       */
/*   2001/4/6   INFORMIX移植(1526)                              */
/****************************************************************/

/* switch定义 */
#include "switch.h"

#include "swNdbstruct.h"
#include "swShm.h"

/* #define DEBUG */

/* 变量定义 */
short igQueuemax,igIso8583max;
extern union  preunpackbuf psgUnpackbuf[iBUFFLDNUM]; /* TDF转换后的缓冲池 */

int igFmtexitflag;    /* add cjc 20040509 */

/*int swSetXMFprebuffer(short iFldid,char *aFldvalue,unsigned int iFldlen); del by baiqj20150413 PSBC_V1.0*/
int swSetXMFprebuffer(long iFldid,char *aFldvalue,unsigned int iFldlen); /*add by baiqj20150413 PSBC_V1.0*/
int swXMFpack(union preunpackbuf *prebuf,char *msgbody,unsigned int *msglen);

/**************************************************************
 ** 函数名      ：swCmpFmtTrantype
 ** 功  能      ：判断报文格式类型与报文转换发式是否匹配 
 ** 作  者      ：史正烨 
 ** 建立日期    : 1999/11/22
 ** 最后修改日期：1999/12/9
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：SUCCESS - 匹配 FAIL - 不匹配
***************************************************************/
int swCmpFmtTrantype( ilMsgformat, ilTran_type, flag )
  short ilMsgformat;
  short ilTran_type;
  short flag;
{
  if ( flag == 0 )
  {
    switch (ilTran_type)
    {
      case iIMF_IMF:
        if (ilMsgformat == iFMTIMF) return(SUCCESS);
        else return(FAIL);
      case iNOSEP_IMF:
        if (ilMsgformat == iFMTNOSEP) return(SUCCESS);
        else return(FAIL);
      case i8583_IMF:
        if (ilMsgformat == iFMT8583) return(SUCCESS);
        else return(FAIL);
      case i8583E_IMF:
        if (ilMsgformat == iFMT8583E) return(SUCCESS);
        else return(FAIL);
      case iSEP_IMF:
        if (ilMsgformat == iFMTSEP) return(SUCCESS);
        else return(FAIL);
      case iXMF_IMF:
        if (ilMsgformat == iFMTXMF) return(SUCCESS);
        else return(FAIL);
      default:
        return(FAIL);
    }
  } else
  {
    switch (ilTran_type)
    {
      case iIMF_IMF:
        if (ilMsgformat == iFMTIMF) return(SUCCESS);
        else return(FAIL);
      case iIMF_NOSEP:
        if (ilMsgformat == iFMTNOSEP) return(SUCCESS);
        else return(FAIL);
      case iIMF_8583:
        if (ilMsgformat == iFMT8583) return(SUCCESS);
        else return(FAIL);
      case iIMF_8583E:
        if (ilMsgformat == iFMT8583E) return(SUCCESS);
        else return(FAIL);
      case iIMF_SEP:
        if (ilMsgformat == iFMTSEP) return(SUCCESS);
        else return(FAIL);
      case iIMF_XMF:
        if (ilMsgformat == iFMTXMF) return(SUCCESS);
        else return(FAIL);
      default:
        return(FAIL);
    }
  }
}

/**************************************************************
 ** 函数名      ：swSepunpack
 ** 功  能      ：有分隔符报文预解包 
 ** 作  者      ：史正烨
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：
***************************************************************/
int swSepunpack(char *aMsgbody, unsigned int iMsglen, char *aRecs, char *aGrps, \
                char *aFlds, union preunpackbuf *prebuf)
{
  unsigned int ilRc, ilFldlen;
  char alFldval[iFLDVALUELEN+1];
  short i,j=0,ilPos,ilPosbegin=0,ilRs,ilGs,ilFs,ilRecid=0,ilGrpid=0,ilFldid=0;
  unsigned int ilLen;

  for(i=0;i<=5;i++)
  {
    if(aRecs[i]=='\0') break;
  }
  ilRs = i;
  
  for(i=0;i<=5;i++)
  {
    if(aGrps[i]=='\0') break;
  }
  ilGs = i;
  
  for(i=0;i<=5;i++)
  {
    if(aFlds[i]=='\0') break;
  }

  ilFs = i;
  ilPos = 0;

  while(ilPos < iMsglen)
  {
    if ((memcmp(aMsgbody + ilPos,aRecs,ilRs) == 0) && (ilRs > 0))
    {
      ilLen = ilRs;
      ilFldlen = ilPos - ilPosbegin;
      memcpy(alFldval, aMsgbody + ilPosbegin, ilFldlen);
      ilRc = swSepset(ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval, prebuf);
      if (ilRc != 0)
      {
        swVdebug(0,"S3194: [错误/函数调用] swSepset()函数,[%d][%d][%d] len[%d]val[%s],返回码=%d", ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval,ilRc);
        return(FAIL);
      }
      ilPos = ilPos + ilLen;
      ilPosbegin = ilPos;
      j ++;
      ilRecid ++;
      ilGrpid = 0;
      ilFldid = 0;
    }
    else if ((memcmp(aMsgbody + ilPos,aGrps,ilGs) == 0) && (ilGs > 0))
    {
      ilLen = ilGs;
      ilFldlen = ilPos - ilPosbegin;
      memcpy(alFldval, aMsgbody + ilPosbegin, ilFldlen);
      ilRc = swSepset(ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval, prebuf);
      if (ilRc != 0)
      {
        swVdebug(0,"S3196: [错误/函数调用] swSepset()函数,[%d][%d][%d] len[%d]val[%s],返回码=%d",ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval,ilRc);
        return(FAIL);
      }
      ilPos = ilPos + ilLen;
      ilPosbegin = ilPos;
      j ++;
      ilGrpid ++;
      ilFldid = 0;
    }
    else if ((memcmp(aMsgbody + ilPos,aFlds,ilFs) == 0) && (ilFs > 0))
    {
      ilLen = ilFs;
      ilFldlen = ilPos - ilPosbegin;
      memcpy(alFldval, aMsgbody + ilPosbegin, ilFldlen);
      ilRc = swSepset(ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval, prebuf);
      if (ilRc != 0)
      {
        swVdebug(0,"S3198: [错误/函数调用] swSepset()函数,[%d][%d][%d] len[%d]val[%s],返回码=%d", 
          ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval,ilRc);
        return(FAIL);
      }
      ilPos = ilPos + ilLen;
      ilPosbegin = ilPos;
      j ++;
      ilFldid ++;
    }
    else ilPos ++;
  }

  ilFldlen = ilPos - ilPosbegin;
  memcpy(alFldval, aMsgbody + ilPosbegin, ilFldlen);
  ilRc = swSepset(ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval, prebuf);
  if (ilRc != 0)
  {
    swVdebug(0,"S3200: [错误/函数调用] swSepset()函数,[%d][%d][%d] len[%d]val[%s],返回码=%d", 
          ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval,ilRc);
    return(FAIL);
  }
  for(i=0;prebuf[i].sSepbuf.iRecid != -1;i++) swVdebug(3,"S3202: %d,%d,%d,%s",
      prebuf[i].sSepbuf.iRecid,
      prebuf[i].sSepbuf.iGrpid,
      prebuf[i].sSepbuf.iFldid,
      prebuf[i].sSepbuf.aFldvalue);
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ：swGetFMLfromTDF
 ** 功  能      ：根据格式转换组从格式转换明细表得到FML预解包 
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：iFmt_group:进行TDF转换所用的格式转换组
 **               iFmt_offset:格式转换组在共享内存中的偏移量
 **               prebuf:进行TDF转换后生成FML预解包所存放的buffer
 ** 返回值      ：SUCCESS:成功/FAIL:失败
***************************************************************/
int swGetFMLfromTDF(iFmt_group, lFmt_offset, prebuf)
 /*del by zcd 20141222
  int  iFmt_group;   
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long   iFmt_group;       /* 指定格式转换组 passed */
  /*end of add by zcd 20141222*/
  long  lFmt_offset;      /* 格式转换组在共享内存中的偏移量 passed*/
  union preunpackbuf *prebuf; /* 预置包数组 */
{
  unsigned int ilRtncode,i,j,ilFldnamelen;
  short ilFldlen,ilLength;
  char  alResult[iFLDVALUELEN];
  char  alFldval[iFLDVALUELEN];
  char  alFldname[iFLDNAMELEN + 1];
  long  llFmt_d_offset;  /* TDF 语句偏移量 */
  long  llFmt_d_offset_org;
//  short ilFmt_d_count;   /* TDF 语句条数 */
  int ilFmt_d_count;   /* TDF 语句条数 */
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;

  psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
  j = 0;
  
  /* ilFmt_d_count  = TDF 语句条数 */
  /* llFmt_d_offset = 第一条TDF语句偏移量 */
  ilRtncode = swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, 
    &slSwt_sys_fmt_grp, &ilFmt_d_count, &llFmt_d_offset_org);
  
  if (ilRtncode != SUCCESS)
  {
    return (FAIL);
  }
  
  for(i=0; i < ilFmt_d_count; i++)
  {
    /* TDF 语句偏移量 */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* 取此条 TDF 语句信息 */
    ilRtncode = swShmselect_swt_sys_fmt_d_offset(llFmt_d_offset, 
      &slSwt_sys_fmt_d);
    
    if (ilRtncode != SUCCESS)
    {
      return (FAIL);
    }
    switch (slSwt_sys_fmt_d.fmt_flag[0])
    {
      case '1':
      case '3':
        /* 重复标识 */
        ilRtncode = swFmlget(psgVarbuf, "_RECCOUNT", 
          &ilFldlen, alFldval);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3204: [错误/函数调用] swFmlget()函数,中间变量_RECCOUNT取值错误,返回码=%d",ilRtncode);
           return(FAIL);
        }

        /* add by szhengye 2001.6.1 */
        alFldval[ilFldlen] = '\0';
        /* add by szhengye 2001.6.1 */

      if (atoi(alFldval))
      {
        for (igRec=1; igRec <= atoi(alFldval); igRec++)
        {
          memset(alResult, 0x00, iFLDVALUELEN);
          ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express, 
            alResult, &ilLength);
          if(ilRtncode == FAIL) 
          {
            swVdebug(0,"S3206: [错误/函数调用] _swExpress()函数 fail[%s],返回码=%d", 
               slSwt_sys_fmt_d.fld_express,ilRtncode);
            return(FAIL);
          }
          if (slSwt_sys_fmt_d.fmt_flag[0] == '1')
          {
            if (ilLength <= 0) continue;

            strcpy(alFldname,slSwt_sys_fmt_d.imf_name);
            ilFldnamelen = strlen(slSwt_sys_fmt_d.imf_name);

            alFldname[ilFldnamelen] = '#';
            alFldname[ilFldnamelen + 1] = igRec;
            alFldname[ilFldnamelen + 2] = 0;

            ilRtncode = swFmlset(alFldname,ilLength,alResult,prebuf);
            if (ilRtncode == FAIL) return(FAIL);
          }
          else
          {
            if (ilLength <= 0) continue;

            strcpy(alFldname,slSwt_sys_fmt_d.imf_name);
            ilFldnamelen = strlen(slSwt_sys_fmt_d.imf_name);

            alFldname[ilFldnamelen] = '#';
            alFldname[ilFldnamelen + 1] = igRec;
            alFldname[ilFldnamelen + 2] = 0;
            
            ilRtncode = swFmlset(alFldname, ilLength, alResult, psgVarbuf);
            if (ilRtncode != SUCCESS)
            {
              swVdebug(0,"S3208: [错误/函数调用] swFmlset()函数,中间变量[%s]出错,返回码=%d",alFldname,ilRtncode);
               return(FAIL);
            }
          }
        }
      }
        break;
      case '2':
        /* 中间变量 */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL)
        {
          swVdebug(0,"S3210: [错误/函数调用] _swExpress()函数,中间变量[%s]计算出错,返回码=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        ilRtncode = swFmlset(slSwt_sys_fmt_d.imf_name, 
          ilLength, alResult, psgVarbuf);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3212: [错误/函数调用] swFmlset()函数,中间变量[%s]出错,返回码=%d",
              slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        else
        {
          swVdebug(2,"S3214: GetFMLfromTDF():中间变量[%s]swFmlset成功[%s]",
                slSwt_sys_fmt_d.imf_name, alResult);
        }
        break;
      default:
        /* 普通TDF语句 */
        if (slSwt_sys_fmt_d.imf_name == '\0') continue;
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express, 
          alResult, &ilLength);
        if (ilRtncode == FAIL) return(FAIL);

        if (ilLength < 0) continue;

	if (ilLength == 0) 
	{
          alResult[0] = '\0';
        }

        ilRtncode = swFmlset(slSwt_sys_fmt_d.imf_name,ilLength,alResult,prebuf);
        if (ilRtncode == FAIL) return(FAIL);
        break;
    }
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ：swGetISOfromTDF
 ** 功  能      ：根据格式转换组从格式转换明细表得到ISO预解包 
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：fmt_group:进行TDF转换所用的格式转换组
 **               prebug:进行TDF转换后生成FML预解包所存放的buffer
 ** 返回值      ：SUCCESS:成功/FAIL:失败
***************************************************************/
int swGetISOfromTDF(iFmt_group, lFmt_offset, prebuf)
  /*del by zcd 20141222
  int  iFmt_group;   
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long   iFmt_group;       /* 指定格式转换组 passed */
  /*end of add by zcd 20141222*/
  long  lFmt_offset;
  union preunpackbuf *prebuf;
{
  int ilRtncode; /*change by dyw*/
  unsigned int i,j,ilPos,ilFldnamelen; /*change by dyw*/
  short ilLength;
  short  ilFldlen; /*change by dyw*/
  short ilFldlen_tmp; /*change by dyw*/
  char alResult[iFLDVALUELEN];
  char alFldval[iFLDVALUELEN];
  char alFldvalue[iFLDVALUELEN];
  char alFldname[iFLDNAMELEN + 1];
  long  llFmt_d_offset;   /* TDF 语句偏移量 */
  long  llFmt_d_offset_org;   /* TDF 语句偏移量 */
  int ilFmt_d_count;    /* TDF 语句条数 */  /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;  
  char alOrgfldlen[10],alOrgfldvalue[iFLDVALUELEN+1];
  short ilOrgfldlen = 0;
  
  psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
  j = 0;
  
  /* ilFmt_d_count  = TDF 语句条数 */
  /* llFmt_d_offset = 第一条TDF语句偏移量 */
  ilRtncode = swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, 
    &slSwt_sys_fmt_grp, &ilFmt_d_count, &llFmt_d_offset_org);
  
  if (ilRtncode != SUCCESS)
  {
    return (FAIL);
  }  
  
  for(i=0; i < ilFmt_d_count; i++)
  {
    /* TDF 语句偏移量 */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* 取此条 TDF 语句信息 */
    ilRtncode = swShmselect_swt_sys_fmt_d_offset(llFmt_d_offset, 
      &slSwt_sys_fmt_d);
    
    if (ilRtncode != SUCCESS)
    {
      return (FAIL);
    }    
    
    switch (slSwt_sys_fmt_d.fmt_flag[0])
    {
      case '1':
      case '3':
        /* 重复标识 */
        ilRtncode = swFmlget(psgVarbuf,"_RECCOUNT",&ilFldlen,alFldval);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3216: [错误/函数调用] swFmlget()函数,中间变量_RECCOUNT取值错误,返回码=%d",ilRtncode);
           return(FAIL);
        }

        /* add by szhengye 2001.6.1 */
        alFldval[ilFldlen] = '\0';
        /* add by szhengye 2001.6.1 */

        memset(&prebuf[j],0x00,sizeof(union preunpackbuf));
        ilPos = 0;
        prebuf[j].sIsobuf.iFldlen = 0;
      if(atoi(alFldval))
      {
        for (igRec=1;igRec<=atoi(alFldval);igRec++)
        {
          memset(alResult,0x00,iFLDVALUELEN);
          ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
            alResult,&ilLength);
          if(ilRtncode == FAIL) 
          {
            swVdebug(0,"S3218: [错误/函数调用] _swExpress()函数 fail[%s],返回码=%d",slSwt_sys_fmt_d.fld_express,ilRtncode);
             return(FAIL);
          }
          if (slSwt_sys_fmt_d.fmt_flag[0] == '1')
          {
            /* Add by szhengye:不打长度为0的FML域 */
            if(ilLength <= 0) continue;

            alOrgfldlen[0] = '\0';
            ilOrgfldlen = 0;
            alOrgfldvalue[0] = '\0';
            ilRtncode = swIsoget(prebuf,slSwt_sys_fmt_d.fld_id,
              &ilFldlen_tmp,alFldvalue); /*change by dyw, use other tmp variable to replace 'ilFldlen' */
            
            memcpy(alFldvalue + ilPos,alResult,ilLength);
            ilLength = ilLength + ilFldlen_tmp;

            swIsoset(slSwt_sys_fmt_d.fld_id,ilLength,alFldvalue,
              alOrgfldlen,ilOrgfldlen,alOrgfldvalue,prebuf);

/* delete by qy 09.29 
            prebuf[j].sIsobuf.iFldid = slSwt_sys_fmt_d.fld_id;
            prebuf[j].sIsobuf.iFldlen = prebuf[j].sIsobuf.iFldlen + ilLength;
            memcpy(prebuf[j].sIsobuf.aFldvalue + ilPos,alResult,ilLength);
*/
            ilPos = ilPos + ilLength;
          }
          else
          {
            ilFldnamelen = strlen(slSwt_sys_fmt_d.imf_name);
            strncpy(alFldname,
              slSwt_sys_fmt_d.imf_name,ilFldnamelen);
            alFldname[ilFldnamelen] = '#';
            alFldname[ilFldnamelen + 1] = igRec;
            alFldname[ilFldnamelen + 2] = 0;
            ilRtncode = swFmlset(alFldname,ilLength,alResult,psgVarbuf);
            if (ilRtncode != SUCCESS)
            {
              swVdebug(0,"S3220: [错误/函数调用] swFmlset()函数,中间变量[%s]swFmlset出错,返回码=%d",alFldname,ilRtncode);
               return(FAIL);
            }
          }
        }
      }
        /* Add by szhengye:不打长度为0的FML域 */
        if (slSwt_sys_fmt_d.fmt_flag[0] == '1')
        {
          if (ilPos > 0) j ++;
        }
        break;
      case '2':
        /* 中间变量 */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL)
        {
          swVdebug(0,"S3222: [错误/函数调用] 中间变量[%s]计算出错,返回码=%d",slSwt_sys_fmt_d.imf_name,ilRtncode);
           return(FAIL);
        }
        ilRtncode = swFmlset(slSwt_sys_fmt_d.imf_name,
          ilLength,alResult,psgVarbuf);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3224: [错误/函数调用] swFmlset()函数,中间变量[%s]赋值出错,返回码=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        else
        {
          swVdebug(3,"S3226:中间变量[%s]swFmlset成功!", 
               slSwt_sys_fmt_d.imf_name);
        }
	break;
      default:
        /* 普通TDF语句 */
/* add new */
        if (slSwt_sys_fmt_d.fld_id == 0) continue;
/* add end */
        /* memset(&prebuf[j],0x00,sizeof(union preunpackbuf)); */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL) 
        {
          swVdebug(0,"S3228: [错误/函数调用] _swExpress()函数 fail[%s],返回码=%d ", 
             slSwt_sys_fmt_d.fld_express,ilRtncode);
          return(FAIL);
        }
        /* Add by szhengye:不打长度为0的FML域 */
        if(ilLength <= 0) continue;
/* test  */
        if(!strcmp(alResult,"~U"))
          continue;
/* test end */
        alOrgfldlen[0] = '\0';
        ilOrgfldlen = 0;
        alOrgfldvalue[0] = '\0';

        swIsoset(slSwt_sys_fmt_d.fld_id,ilLength,alResult,
          alOrgfldlen,ilOrgfldlen,alOrgfldvalue,prebuf);
        j ++;
        break;
    }
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ：swGetSEPfromTDF
 ** 功  能      ：根据格式转换组从格式转换明细表得到有/无分隔符的预解包 
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：fmt_group:进行TDF转换所用的格式转换组
 **               prebug:进行TDF转换后生成FML预解包所存放的buffer
 ** 返回值      ：SUCCESS:成功/FAIL:失败
***************************************************************/
int swGetSEPfromTDF(iFmt_group, lFmt_offset, prebuf)
 /*del by zcd 20141222
  int  iFmt_group;   
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long   iFmt_group;       /* 指定格式转换组 passed */
  /*end of add by zcd 20141222*/
  long  lFmt_offset;      /* 格式转换组在共享内存中的偏移量 passed*/
  union preunpackbuf *prebuf;
{
  unsigned int ilRtncode,i,j,ilFldnamelen;
  short ilFldlen;
  short ilRecid=0,ilGrpid=0,ilFldid=0;
  char alResult[iFLDVALUELEN+1];
  char alFldval[iFLDVALUELEN+1];
  char alFldname[iFLDNAMELEN + 1];
  short ilLength;
  long  llFmt_d_offset;  /* TDF 语句偏移量 */
  long  llFmt_d_offset_org;
  int ilFmt_d_count;   /* TDF 语句条数 */  /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;

  psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
  j = 0;
  
  /* ilFmt_d_count  = TDF 语句条数 */
  /* llFmt_d_offset = 第一条TDF语句偏移量 */
  ilRtncode = swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, 
    &slSwt_sys_fmt_grp, &ilFmt_d_count, &llFmt_d_offset_org);
  
  if (ilRtncode != SUCCESS)
  {
    return (FAIL);
  }
  
  for(i=0; i < ilFmt_d_count; i++)
  {
    /* TDF 语句偏移量 */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* 取此条 TDF 语句信息 */
    ilRtncode = swShmselect_swt_sys_fmt_d_offset(llFmt_d_offset, 
      &slSwt_sys_fmt_d);
    
    if (ilRtncode != SUCCESS)
    {
      return (FAIL);
    }
    switch (slSwt_sys_fmt_d.fmt_flag[0])
    {
      case '1':
      case '3':
        /* 重复标识 */
        ilRtncode = swFmlget(psgVarbuf,"_RECCOUNT",&ilFldlen,alFldval);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3230: [错误/函数调用] swFmlget()函数,中间变量_RECCOUNT取值错误,返回码=%d",ilRtncode);
           return(FAIL);
        }

        /* add by szhengye 2001.6.1 */
        alFldval[ilFldlen] = '\0';
        /* add by szhengye 2001.6.1 */

      if(atoi(alFldval))
      {
        for (igRec=1;igRec<=atoi(alFldval);igRec++)
        {
          memset(alResult,0x00,iFLDVALUELEN);
          ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
            alResult, &ilLength);
          if(ilRtncode == FAIL) 
          {
            swVdebug(0,"S3232: [错误/函数调用] _swExpress()函数 fail[%s],返回码=%d", 
               slSwt_sys_fmt_d.fld_express,ilRtncode);
            return(FAIL);
          }
          if (slSwt_sys_fmt_d.fmt_flag[0] == '1')
          {
            ilFldid = j + 1;
            swSepset(ilRecid,ilGrpid,ilFldid,ilLength,alResult,prebuf);
            j ++;
          }
          else
          {
            ilFldnamelen = strlen(slSwt_sys_fmt_d.imf_name);
            strncpy(alFldname,
              slSwt_sys_fmt_d.imf_name, ilFldnamelen);
            alFldname[ilFldnamelen] = '#';
            alFldname[ilFldnamelen + 1] = igRec;
            alFldname[ilFldnamelen + 2] = 0;
            ilRtncode = swFmlset(alFldname,ilLength,alResult,psgVarbuf);
            if (ilRtncode != SUCCESS)
            {
              swVdebug(0,"S3234: [错误/函数调用] swFmlset()函数,中间变量[%s]赋值出错,返回码=%d",alFldname,ilRtncode);
               return(FAIL);
            }
          }
        }
      }
        break;
      case '2':
        /* 中间变量 */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL)
        {
          swVdebug(0,"S3236: [错误/函数调用] 中间变量[%s]计算出错,返回码=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        ilRtncode = swFmlset(slSwt_sys_fmt_d.imf_name,
          ilLength,alResult,psgVarbuf);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3238: [错误/函数调用] 中间变量[%s]赋值出错,返回码=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        else
        {
          swVdebug(2,"S3240: swGetSEPfromTDF():中间变量[%s]swFmlset成功!",
            slSwt_sys_fmt_d.imf_name);
        }
        break;
      default:
        /* 普通TDF语句 */
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult, &ilLength);
        if(ilRtncode == FAIL) 
        {
          swVdebug(0,"S3242: [错误/函数调用] _swExpress fail[%s],返回码=%d", 
            slSwt_sys_fmt_d.fld_express,ilRtncode);
          return(FAIL);
        }
        if (ilLength == 0)
        {
          alResult[0] = '\0';
        }
        ilFldid = j + 1;
        swSepset(ilRecid,ilGrpid,ilFldid,ilLength,alResult,prebuf);
        j ++;
        break;
    }
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ：swFMLcheck
 ** 功  能      ：根据顺序号的标准域值(在此过程中，根据标准域名
 **               在内部标准域列表中的相关配置进行用户自定义函数校验
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：
***************************************************************/
int swFMLcheck (union preunpackbuf *prebuf )
{
  short i, j, ilRtncode;
  char  alResult[iFLDVALUELEN + 1];
  char  alFldvalue[iFLDVALUELEN + 1];
  char  alFldname[iFLDNAMELEN + 1];
  short ilFldlen,ilLength;
  short ilImfcount;  /* swt_sys_imf 表记录数 */
  /* struct swt_sys_imf *pslSwt_sys_imf;  首记录地址 */
  struct swt_sys_imf pslSwt_sys_imf[iMAXIMFREC];
  
  /* 得到 swt_sys_imf 表所有记录及记录数 */
  ilRtncode = swShmselect_swt_sys_imf_all(pslSwt_sys_imf, &ilImfcount);
    
  if (ilRtncode != SUCCESS)
    return (FAIL);
  alFldname[0] = '\0';  
  for(i=0; prebuf[i].sImfbuf.aFldname[0] != '\0'; i++)
  {

    if (i >= iBUFFLDNUM) break;

    if (strcmp(prebuf[i].sImfbuf.aFldname,alFldname) == 0) continue;
    strcpy(alFldname,prebuf[i].sImfbuf.aFldname);

    for(j=0; j < ilImfcount; j++)
    {
      if(strcmp(alFldname,pslSwt_sys_imf[j].imf_name) == 0)
      {
        if (strlen(pslSwt_sys_imf[j].imf_check) == 0) break;

        ilRtncode = swFmlget(prebuf,alFldname,&ilFldlen,alFldvalue);

        memcpy(agCurfld,alFldvalue,iFLDVALUELEN);

        ilRtncode = _swExpress(pslSwt_sys_imf[j].imf_check,alResult,&ilLength);
        if (ilRtncode == FAIL)
        {
          swVdebug(0,"S3244: [错误/函数调用] 域校验表达式%s错误,返回码=%d",pslSwt_sys_imf[j].imf_check,ilRtncode);
           _swMsgsend(302023,NULL);
          return(FAIL);
        }
        if (alResult[0] != '1')
        {
          swVdebug(0,"S3246: [错误/其它] 域校验出错: 域[%s],值[%s],校验[%s],结果=%d", alFldname, alFldvalue, pslSwt_sys_imf[j].imf_check, alResult[0]);
          return(FAIL);
        }
        break;
      }
    }
  }
  return ( SUCCESS );
}

/**************************************************************
 ** 函数名      ：swISOcheck
 ** 功  能      ：根据ISO8583的用户自定义函数进行check,
 **               根据8583域类型进行check
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.12.9
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：
***************************************************************/
int swISOcheck (union preunpackbuf *prebuf, short tab_id)
{
  short  i,ilRtncode;
  char   alResult[iFLDVALUELEN + 1];
 /* short  ilLength; */
  short   ilLength;  /*changed by dyw*/

  char   *alFld_rule;
  struct swt_sys_8583 slSwt_sys_8583;

  for(i=0;prebuf[i].sIsobuf.iFldid != -1;i++)
  {
    if (i>=iBUFFLDNUM) break;
    
    /* 根据 tab_id、fldid 找swt_sys_8583表中对应记录 */
    ilRtncode = swShmselect_swt_sys_8583(tab_id, prebuf[i].sIsobuf.iFldid, 
      &slSwt_sys_8583);
      
    if (ilRtncode != SUCCESS)
      return (FAIL);
    
    alFld_rule = slSwt_sys_8583.fld_rule;
    /* swVdebug(0,"S3248:%d,%s",prebuf[i].sIsobuf.iFldid,slSwt_sys_8583.fld_name); */
     if (strlen(alFld_rule) > 0)
    {
      ilRtncode = _swExpress(alFld_rule,alResult,&ilLength);
      if (ilRtncode == FAIL )
      {
        swVdebug(0,"S3250: [错误/函数调用] _swExpress()函数,ISO8583域校验表达式%s错误,返回码=%d",alFld_rule,ilRtncode);
         _swMsgsend(302025,NULL);
        return(FAIL);
      }
      if (alResult[0] != '1')
      {
         swVdebug(0,"S3252: [错误/其它] 域校验出错: 域[%d],值[%s],校验[%s],结果=%c",
           prebuf[i].sIsobuf.iFldid,
          prebuf[i].sIsobuf.aFldvalue,alFld_rule,alResult[0]);
        return(FAIL);
      }
    }
    switch (slSwt_sys_8583.fld_type)
    {
      case Efn:
      case Eln:
      case Elln:
      case Ellln:
      case Elllln:
        ilRtncode = _swMatch("[0-9 ]*",prebuf[i].sIsobuf.aFldvalue);
        if (ilRtncode == FALSE) 
        {
          swVdebug(0,"S3254: [错误/函数调用] _swMatch()函数,域校验出错:域[%d],值[%s]应为数字,返回码=%d", prebuf[i].sIsobuf.iFldid, prebuf[i].sIsobuf.aFldvalue, ilRtncode);
          return(FAIL);
        }
        break;
      case Efan:
      case Ellan:
      case Elllan:
      case Ellllan:
        ilRtncode = _swMatch("[a-zA-Z0-9 ]*",prebuf[i].sIsobuf.aFldvalue);
        if (ilRtncode == FALSE) 
        {
          swVdebug(0,"S3256: [错误/函数调用] _swMatch()函数,域校验出错: 域[%d],值[%s]应为数字或字母,返回码=%d", prebuf[i].sIsobuf.iFldid, prebuf[i].sIsobuf.aFldvalue, ilRtncode);
          return(FAIL);
        }
        break;
    }
  }
  return ( SUCCESS );
}
/**************************************************************
 ** 函数名      ：swSeppack
 ** 功  能      ：有分隔符报文打包
 ** 作  者      ：史正烨
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：prebuf:预解包数组  aMsgbody:打成包的存放地
 **               iMsglen:返回打成包的长度
 ** 返回值      ：SUCCESS:成功  FAIL:失败
***************************************************************/
int swSeppack(union preunpackbuf *prebuf,char *aMsgbody,unsigned int *iMsglen)
{
  short i,ilOffset = 0;

  for(i=0;((prebuf[i].sSepbuf.iRecid != -1) && (i < iBUFFLDNUM));i++)
  {
    memcpy(aMsgbody + ilOffset,prebuf[i].sSepbuf.aFldvalue,
      prebuf[i].sSepbuf.iFldlen);
    ilOffset = ilOffset + prebuf[i].sSepbuf.iFldlen;
  }
  *iMsglen = ilOffset;
  swVdebug(2,"S3258: prebuf number = %d",i);
   return(SUCCESS);
}

/**************************************************************
 ** 函数名      ：swGetXMFfromTDF
 ** 功  能      ：根据格式转换组从格式转换明细表得到XMF预解包 
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：iFmt_group:进行TDF转换所用的格式转换组
 **               iFmt_offset:格式转换组在共享内存中的偏移量
 ** 返回值      ：SUCCESS:成功/FAIL:失败
***************************************************************/
/*del by zcd 20141222
int swGetXMFfromTDF(int iFmt_group,long lFmt_offset)
***end of del by zcd 20141222*/
/*add by zcd 20141222*/
int swGetXMFfromTDF(long iFmt_group,long lFmt_offset)
/*end of add by zcd 20141222*/
{
  int ilRc,i,j;/*ilScript;  del by baiqj20150413 PSBC_V1.0*/
  long ilScript; /*add by baiqj20150413 PSBC_V1.0*/
  short  ilResultlen; /*changed by dyw,*/
  char  alResult[iFLDVALUELEN];
  long  llFmt_d_offset;  /* TDF 语句偏移量 */
  long  llFmt_d_offset_org;
  long  llFmt_offset;
  long  llMsgoffset;
  int ilFmt_d_count;   /* TDF 语句条数 */  /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;
  char  alMsgbody_save[iMSGMAXLEN];
  unsigned int ilMsgbodylen_save;

  /* psgVarbuf[0].sImfbuf.aFldname[0] = '\0'; */
  j = 0;
  
  /* ilFmt_d_count  = TDF 语句条数 */
  /* llFmt_d_offset = 第一条TDF语句偏移量 */
  ilRc = swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, 
    &slSwt_sys_fmt_grp, &ilFmt_d_count, &llFmt_d_offset_org);
  
  if (ilRc != SUCCESS)
  {
    return (FAIL);
  }
  
  lgMsgoffset = 0;
  
  igFmtexitflag=0;  /* add by cjc 20040509  */

  for(i=0; i < ilFmt_d_count; i++)
  {    
  	
    /*  add by cjc 20040509 begin */
    if (igFmtexitflag)
    {
      return 0;            
    }
    /*  add by cjc 20040509 end */
  
    /* TDF 语句偏移量 */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* 取此条 TDF 语句信息 */
    ilRc = swShmselect_swt_sys_fmt_d_offset(llFmt_d_offset, 
      &slSwt_sys_fmt_d);
    
    if (ilRc != SUCCESS)
    {
      return (FAIL);
    }

Label_XMFGOTO:

    if (slSwt_sys_fmt_d.fmt_cond[0])
    {
      ilRc = _swExpress(slSwt_sys_fmt_d.fmt_cond,alResult,&ilResultlen);
      if (ilRc)
      {
        swVdebug(0,"S3260: [错误/函数调用] _swExpress()函数,条件表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fmt_cond,ilRc);
        return(FAIL);
      }
      if (alResult[0] == '0')
      {
        swVdebug(2,"S3262: TDF条件表达式不满足:%s",slSwt_sys_fmt_d.fmt_cond);
        continue;
      }
    }
    switch (slSwt_sys_fmt_d.fmt_flag[0])
    {
      case 'A':		/* GOTO语句:{GOTO 行号} */
        ilScript = slSwt_sys_fmt_d.fld_id;  /* modifyed by dgm 2002.06.17 */
        swVdebug(2,"S3264:GOTO语句:{GOTO %d}",ilScript);
        for(j=0; j < ilFmt_d_count; j++)
        {
          /* 取此条 TDF 语句信息 */
          ilRc = swShmselect_swt_sys_fmt_d_offset(llFmt_d_offset_org + j, 
            &slSwt_sys_fmt_d);
    
          if (ilRc != SUCCESS) return (FAIL);
        
          if (slSwt_sys_fmt_d.id == ilScript)
          {
            i = j;
            goto Label_XMFGOTO;
          }
        }
        swVdebug(0,"S3266: [错误/其它] 目标语句不存在[GOTO %d]",ilScript);
        return(FAIL);

      case 'B':		/* 给中间变量赋值(按域ID):{VARID 域ID=表达式} */
        swVdebug(2,"S3268: 给中间变量(按域ID)赋值:{VARID %ld=%s}",
          slSwt_sys_fmt_d.fld_id,slSwt_sys_fmt_d.fld_express);/*modified by baiqj20150413 PSBC_V1.0*/
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3270: [错误/函数调用] 表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        if (slSwt_sys_fmt_d.fld_id >= iBUFFLDNUM)
        {
          swVdebug(0,"S3272: [错误/其它] 中间变量域ID越界(>=%d)",iBUFFLDNUM);
          return(FAIL);
        }
        psgVaridbuf[slSwt_sys_fmt_d.fld_id].sImfbuf.iFldlen = ilResultlen;
        memcpy(psgVaridbuf[slSwt_sys_fmt_d.fld_id].sImfbuf.aFldvalue,
          alResult,ilResultlen);
        continue;

      case 'C':		/* 中间变量赋值:{VAR 变量名=表达式} */

        /* add by bmy 2002.03.12 */
        if (slSwt_sys_fmt_d.imf_array[0])
        {
          swVdebug(2,"S3274: 给中间变量(按域名)赋值:{VAR %s[%s]=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.imf_array,slSwt_sys_fmt_d.fld_express);
          ilRc = swFmlget(psgVarbuf, slSwt_sys_fmt_d.imf_array,
            &ilResultlen, alResult);
          if (ilRc != SUCCESS)
          {
            swVdebug(0,"S3276: [错误/函数调用] swFmlget()函数,取中间变量[%s]出错",slSwt_sys_fmt_d.imf_array);
            return(FAIL);
          }
          ilResultlen = strlen(slSwt_sys_fmt_d.imf_name);
          slSwt_sys_fmt_d.imf_name[ilResultlen] = '#';
          slSwt_sys_fmt_d.imf_name[ilResultlen+1] = atoi(alResult);
          slSwt_sys_fmt_d.imf_name[ilResultlen+2] = 0x00;
          if ((ilResultlen+2) > iFLDNAMELEN)
          {
             swVdebug(0,"S3278: [错误/其它] FML域名[%s]长度超出最大长度[%d]",
               slSwt_sys_fmt_d.imf_name, iFLDNAMELEN);
            return(FAIL);
          }
        }
        else
        {
          swVdebug(2,"S3280: 给中间变量(按域名)赋值:{VAR %s=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.fld_express);
        }
        
        /* end of add by bmy */

        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3282: [错误/函数调用] 表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        ilRc = swFmlset(slSwt_sys_fmt_d.imf_name,ilResultlen,alResult,
          psgVarbuf);
        if (ilRc != SUCCESS)
        {
          swVdebug(0,"S3284: 中间变量[%s]赋值出错!",
            slSwt_sys_fmt_d.imf_name);
          return(FAIL);
        }
        continue;

      case 'D':         /* 置FML报文域:{SET 域名=表达式} */
        
        /* add by bmy 2002.03.12 */
        if (slSwt_sys_fmt_d.imf_array[0])
        {
          swVdebug(2,"S3286: 置FML报文域:{SET %s[%s]=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.imf_array,slSwt_sys_fmt_d.fld_express);
          ilRc = swFmlget(psgVarbuf, slSwt_sys_fmt_d.imf_array,
            &ilResultlen, alResult);
          if (ilRc != SUCCESS)
          {
            swVdebug(0,"S3288: [错误/函数调用] swFmlget()函数,取中间变量[%s]出错",slSwt_sys_fmt_d.imf_array);
            return(FAIL);
          }
          ilResultlen = strlen(slSwt_sys_fmt_d.imf_name);
          slSwt_sys_fmt_d.imf_name[ilResultlen] = '#';
          slSwt_sys_fmt_d.imf_name[ilResultlen+1] = atoi(alResult);
          slSwt_sys_fmt_d.imf_name[ilResultlen+2] = 0x00;
          if ((ilResultlen+2) > iFLDNAMELEN)
          {
             swVdebug(0,"S3290: [错误/其它] FML域名[%s]长度超出最大长度[%d]",
               slSwt_sys_fmt_d.imf_name, iFLDNAMELEN);
            return(FAIL);
          }
        }
        else
        {
          swVdebug(2,"S3292: 置FML报文域:{SET %s=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.fld_express);
        }
        
        /* end of add by bmy */

        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3294: [错误/函数调用] _swExpress()函数,表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        ilRc = swFmlset(slSwt_sys_fmt_d.imf_name,ilResultlen,alResult,
          psgUnpackbuf);
        if (ilRc)
        {
          swVdebug(0,"S3296: [错误/函数调用] swFmlset()函数,置域[%s]赋值出错,返回码=%d",
            slSwt_sys_fmt_d.imf_name,ilRc);
          return(FAIL);
        }
        continue;

      case 'E':		/* 调用TDF(递归):{TDF 格式转换组,[表达式]} */
        swVdebug(2,"S3298: 调用TDF(递归):{TDF %ld,%s}",
          slSwt_sys_fmt_d.fld_id,slSwt_sys_fmt_d.fld_express); /*modify by baiqj20150413 PSBC_V1.0*/
        if (slSwt_sys_fmt_d.fld_express[0])
        {
          ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
          if (ilRc)
          {
            swVdebug(0,"S3300: [错误/函数调用] _swExpress()函数,表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fld_express,ilRc);
            return(FAIL);
          }
        }
        /* 保存现场 */
        if (slSwt_sys_fmt_d.fld_express[0])
        {
          memcpy(alMsgbody_save,agMsgbody,igMsgbodylen);
          ilMsgbodylen_save = igMsgbodylen;
          memcpy(agMsgbody,alResult,ilResultlen);
          igMsgbodylen = ilResultlen;
          llMsgoffset = lgMsgoffset;
        }

        ilRc = swShmselect_swt_sys_fmt_grp(slSwt_sys_fmt_d.fld_id,
          &slSwt_sys_fmt_grp,&llFmt_offset);
        if (ilRc)
        {
          swVdebug(0,"S3302: [错误/共享内存] 指定格式转换组为[%ld]不存在,返回码=%d",
            slSwt_sys_fmt_d.fld_id,ilRc);   /*modified by baiqj20150413 PSBC_V1.0*/
          return(FAIL);
        }

        /* 递归调用swGetXMFfromTDF函数 */
/*        
        if (slSwt_sys_fmt_d.fld_express[0])
        {
*//* delete by zjj 2004.03.11 */       	
          ilRc = swGetXMFfromTDF(slSwt_sys_fmt_d.fld_id,llFmt_offset);
          if (ilRc)
          {
            swVdebug(0,"S3304: [错误/函数调用] swGetXMFfromTDF()函数,返回码=%d",ilRc);
            return(FAIL);
          }
/*          
        }
*/  /* delete by zjj 2004.03.11 */

        if (slSwt_sys_fmt_d.fld_express[0])
        {
          /* 恢复现场 */
          igMsgbodylen = ilMsgbodylen_save;
          memcpy(agMsgbody,alMsgbody_save,igMsgbodylen);
          lgMsgoffset = llMsgoffset;
        }
        continue;

      case 'F':		/* RETURN(返回码): {RETURN 返回码} */
        swVdebug(2,"S3306: RETURN(返回码):{RETURN %ld}",slSwt_sys_fmt_d.fld_id);/*modified by baiqj20150413 PSBC_V1.0*/
        return(slSwt_sys_fmt_d.fld_id);

      case 'G':		/* 计算平台函数表达式:{TRACE 表达式} */
        swVdebug(2,"S3308: 计算平台函数表达式:{TRACE %s}",
          slSwt_sys_fmt_d.fld_express);
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3310: [错误/函数调用] _swExpress()函数,表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          continue;
        }
        alResult[ilResultlen] = '\0';
        swDebug("S:TRACE:%s",alResult);
        continue;

      case 'H':		/* 执行平台平台式(DO):{DO 平台表达式} */
        swVdebug(2,"S3312: 执行平台平台式(DO):{DO %s}",
          slSwt_sys_fmt_d.fld_express);
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3314: [错误/函数调用] _swExpress()函数,表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        ilRc = swFmlset("_RETURN",ilResultlen,alResult,psgVarbuf);
        if (ilRc)
        {
          swVdebug(0,"S3316: [错误/函数调用] swFmlset()函数,中间变量[_RETURN]赋值出错,返回码=%d",ilRc);
          return(FAIL);
        }
        continue;

      case 'I':		/* 置打包报文域: {FIELD 域ID=平台表达式} */
        swVdebug(2,"S3318: 置打包报文域:{FIELD %ld=%s}",
          slSwt_sys_fmt_d.fld_id,slSwt_sys_fmt_d.fld_express);   /*modified by baiqj20150413 PSBC_V1.0*/
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3320: [错误/函数调用] _swExpress()函数,表达式%s错误,返回码=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }

        ilRc = swSetXMFprebuffer(slSwt_sys_fmt_d.fld_id,alResult,ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3322: [错误/函数调用] swSetXMFprebuffer()函数，置打包预置域[%ld]出错,返回码=%d",
            slSwt_sys_fmt_d.fld_id,ilRc);   /*modified by baiqj20150413 PSBC_V1.0*/
          return(FAIL);
        }
        continue;
      /* begin add by nh 20040511 */
      case 'L':        /*EXIT*/
        swVdebug(2,"S3323: EXIT TDF ");
        igFmtexitflag=1;
        continue;
      /* end add */
      default:
        swVdebug(0,"S3324: [错误/其它] 未知的TDF脚本动作('%c',%d)",
          slSwt_sys_fmt_d.fmt_flag[0],slSwt_sys_fmt_d.fmt_flag[0]);
        return(FAIL);
    }
  }
  return(0);
}
/**************************************************************
 ** 函数名      ：swSetXMprebuffer
 ** 功  能      ：置XMF报文域
 ** 作  者      ：史正烨
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：iFldid:域id  aFldvalue:域值  iFldlen:域长度
 ** 返回值      ：SUCCESS:成功  FAIL:失败
***************************************************************/
/*int swSetXMFprebuffer(short iFldid,char *aFldvalue,unsigned int iFldlen) del by baiqj20150413 PSBC_V1.0*/
int swSetXMFprebuffer(long iFldid,char *aFldvalue,unsigned int iFldlen) /*add by baiqj20150413 PSBC_V1.0*/
{
  int i,ilPoint;
  int ilCount=0,ilFldlen,ilPos=0;

  for(i=0;i<iBUFFLDNUM;i++)
  {
    if (psgUnpackbuf[i].sIsobuf.iFldid == -1) break;
  }

  if (i>=iBUFFLDNUM)
  {
    swVdebug(0,"S3326: [错误/其它] 预置包溢出![%d]>[%d]",i,iBUFFLDNUM);
    return(-1);
  }

  ilPoint = i;
  ilFldlen = iFldlen;
  ilCount = (ilFldlen - 1)/iPERFLDVALUELEN + 1;

  if ((i+ilCount)>=iBUFFLDNUM)
  {
    swVdebug(0,"S3328: [错误/其它] 预置包溢出![%d]>[%d]",i+ilCount,iBUFFLDNUM);
    return(-1);
  }

  for (;;)
  {
    if ((psgUnpackbuf[ilPoint-1].sIsobuf.iFldid > iFldid) &&
      (ilPoint > 0))
    {
      memcpy((char *)&psgUnpackbuf[ilPoint+ilCount-1],
        (char *)&psgUnpackbuf[ilPoint-1],
        sizeof(union preunpackbuf));
      ilPoint --;
      continue;
    }
    for(;;)
    {
      if (ilFldlen <= iPERFLDVALUELEN)
      {
        psgUnpackbuf[ilPoint].sIsobuf.iFldid = iFldid;
        psgUnpackbuf[ilPoint].sIsobuf.iFldlen = ilFldlen;
        memcpy(psgUnpackbuf[ilPoint].sIsobuf.aFldvalue,
          aFldvalue+ilPos,ilFldlen);
        psgUnpackbuf[ilPoint].sIsobuf.aFldvalue[ilFldlen] = '\0';
        break;
      }
      psgUnpackbuf[ilPoint].sIsobuf.iFldid = iFldid;
      psgUnpackbuf[ilPoint].sIsobuf.iFldlen = iPERFLDVALUELEN;
      memcpy(psgUnpackbuf[ilPoint].sIsobuf.aFldvalue,
        aFldvalue+ilPos,iPERFLDVALUELEN);
      psgUnpackbuf[ilPoint].sIsobuf.aFldvalue[iPERFLDVALUELEN] = '\0';
      ilPos = ilPos + iPERFLDVALUELEN;
      ilFldlen = ilFldlen - iPERFLDVALUELEN;
      ilPoint ++;
    }
    psgUnpackbuf[i+ilCount].sIsobuf.iFldid = -1;  /* 预置包结束标志 */
    break;
  }

  return(0);
}


/**************************************************************
 ** 函数名      ：swXMFpack
 ** 功  能      ：XMF报文打包
 ** 作  者      ：史正烨
 ** 建立日期    ：1999.12.6
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：prebuf:预解包数组  msgbody:打成包的存放地
 **               msglen:返回打成包的长度
 ** 返回值      ：SUCCESS:成功  FAIL:失败
***************************************************************/
int swXMFpack(union preunpackbuf *prebuf,char *msgbody,unsigned int *msglen)
{
  short i,ilOffset = 0;

  for(i=0;(prebuf[i].sIsobuf.iFldid != -1);i++)
  {
    swVdebug(3,"S3330: id[%d] len[%d] value[%s]", 
      prebuf[i].sIsobuf.iFldid,
      prebuf[i].sIsobuf.iFldlen,
      prebuf[i].sIsobuf.aFldvalue);
  }
  for(i=0;((prebuf[i].sIsobuf.iFldid != -1) && (i < iBUFFLDNUM));i++)
  {
    memcpy(msgbody + ilOffset,prebuf[i].sIsobuf.aFldvalue,
      prebuf[i].sIsobuf.iFldlen);
    ilOffset = ilOffset + prebuf[i].sIsobuf.iFldlen;
  }
  *msglen = ilOffset;
  return(SUCCESS);
}

