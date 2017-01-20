#include "switch.h"
/* add by qzg*/
#include "swShmGroup.h"

#define SFTRANLOGID	0
#define SFPROCLOGID	1
#define SFSAFLOGID	2
#define OFFSETNULL 	-1

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/

/* 数据库定义 */
EXEC SQL INCLUDE sqlca;
#ifdef DB_POSTGRESQL
EXEC SQL INCLUDE swDbstruct.h;
#endif
#ifdef DB_DB2
EXEC SQL INCLUDE '../inc/swDbstruct.h';
#endif
#ifdef DB_INFORMIX
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_ORACLE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_SYBASE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#else/*support database,Database Deprivation Project 2014-12*/
#include "swDbstruct.h"
#endif
#include "swNdbstruct.h"
#include "swShm.h"


/*2009-9-9 9:41 跨组信息存储结构体*/

struct GrpComInfo {
	char groupid[4];		/*组号*/
	char ip[16];				/*组号对应监听IP*/
	int port;						/*组号对应跨组监听端口*/
	int minserialno;		/*组号对应起始流水号*/
	int maxserialno;    /*组号对应终止流水号*/
};

/*2009-9-9 9:41 跨组信息存储结构体*/



/*static*/ extern struct swt_sys_config smSwt_sys_config;

/*static*/ extern struct shmidx_s *psmShmidx_s;   /* 静态表共享内存目录段指针 */
/*static*/ extern struct shmidx_d *psmShmidx_d;   /* 动态表共享内存目录段指针 */
/*static*/ extern struct shmbuf_s smShmbuf_s;     /* 静态表共享内存内容段指针 */
/*static*/ extern struct shmbuf_d smShmbuf_d;     /* 动态表共享内存内容段指针 */

static char amShmfile[3][101];
/*add by zcd 20141225*/
static asw_tran_log_file[256];
static asw_proc_log_file[256];
/*end of add by zcd 20141225*/
//20150115key_t iSHEGROUP=-1;
int igGrpShmid;

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/

/* "ifdef" added by qh 20070420  主要是针对db2 64位机上用.将long改为sqlint64*/
#ifdef DB_DB2_64
EXEC SQL BEGIN DECLARE SECTION;
  short  igTmp;   
  short  igQid;
  short  igStatus;
  short  igProc_step;
  short  igCount;
  char   agFilename[11];
  char   agKey[151];
  sqlint64   lgTime;
  sqlint64   lgTranid;  
  sqlint64   lgTran_begin;
  sqlint64   lgSaf_id;
  static struct swt_tran_log1
  {
    sqlint64  tran_id;
    sqlint64  tran_begin;
    sqlint64  tran_end;
    sqlint64  tran_overtime;
    sqlint64  resq_offset;
    sqlint64  resp_offset;
    short tran_status;
    short q_tran_begin;
    short q_target;
    short resq_len;
    short resp_len;
    char  tc_tran_begin[11];
    char  resq_file[11];
    char  resp_file[11];
    char  rev_key[151];
    char  msghdkey[151];
    char  msghead[151];
    char  xastatus[21];           /* 记录XA状态 */
    char  a1[3];
  }sgSwt_tran_log1;
  static struct swt_proc_log1
  {
    sqlint64      tran_id;
    sqlint64  proc_begin;
    sqlint64  org_offset;
    short q_target;
    short proc_step;
    short proc_status;
    short rev_mode;
	/*del by zcd 20141220
    short rev_fmtgrp;
	***end of del by zcd 20141220*/
	/*add by zcd 20141220*/
	long  rev_fmtgrp;
	/*end of add by zcd 20141220*/
    short org_len;
    char  org_file[11];
    char  a1;
  }sgSwt_proc_log1;
EXEC SQL END DECLARE SECTION;
#endif

#ifdef DATABASE
EXEC SQL BEGIN DECLARE SECTION;
  short  igTmp;   
  short  igQid;
  short  igStatus;
  short  igProc_step;
  short  igCount;
  char   agFilename[11];
  char   agKey[151];
  long   lgTime;
  long   lgTranid;  
  long   lgTran_begin;
  long   lgSaf_id;
  static struct swt_tran_log1
  {
    long  tran_id;
    long  tran_begin;
    long  tran_end;
    long  tran_overtime;
    long  resq_offset;
    long  resp_offset;
    short tran_status;
    short q_tran_begin;
    short q_target;
    short resq_len;
    short resp_len;
    char  tc_tran_begin[11];
    char  resq_file[11];
    char  resp_file[11];
    char  rev_key[151];
    char  msghdkey[151];
    char  msghead[151];
    char  xastatus[21];           /* 记录XA状态 */
    char  a1[3];
  }sgSwt_tran_log1;
  static struct swt_proc_log1
  {
    long      tran_id;
    long  proc_begin;
    long  org_offset;
    short q_target;
    short proc_step;
    short proc_status;
    short rev_mode;
	/*del by zcd 20141220
    short rev_fmtgrp;
	***end of del by zcd 20141220*/
	/*add by zcd 20141220*/
	long  rev_fmtgrp;
	/*end of add by zcd 20141220*/  
    short org_len;
    char  org_file[11];
    char  a1;
  }sgSwt_proc_log1;
EXEC SQL END DECLARE SECTION;
#endif

#endif   /*support database,Database Deprivation Project 2014-12*/
/***************************************************************
 ** 函数名      : swShminsert_swt_rev_saf
 ** 功  能      : 插入冲正记录
 ** 作  者      : cjh
 ** 建立日期    : 20150112
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swShminsert_swt_rev_saf(struct swt_rev_saf sSwt_rev_saf)
{
  int i,j,ilElink;
  
  if (swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);

  i = psmShmidx_d->sIdx_rev_saf.iFlink;
  for (;;)
  {
    if (smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.cTag != 1) break;
    i++;
    if (i > smSwt_sys_config.iMaxsaflog) i = 1;
    if (i == psmShmidx_d->sIdx_rev_saf.iFlink)
    {
      if (swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);
      swVdebug(4,"S5060: Shm is full");
      return(FAIL);
    }       
  }
  memcpy((char *)(&(smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf)),
    (char *)&sSwt_rev_saf, sizeof(struct swt_rev_saf));
  
  /* 统计在共享内存当中的冲正数 */
  psmShmidx_d->iRecinshm[SFSAFLOGID] ++;
  
  smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.cTag = 1;
  smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink = 0;
  ilElink =  psmShmidx_d->sIdx_rev_saf.iElink;
  smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iLlink = ilElink;
  if (ilElink > 0)
    smShmbuf_d.psShm_rev_saf[ilElink-1].sLinkinfo.iRlink = i;
  
  if (psmShmidx_d->sIdx_rev_saf.iSlink == 0)
    psmShmidx_d->sIdx_rev_saf.iSlink = i;
  
  psmShmidx_d->sIdx_rev_saf.iElink = i;
  
  if (i < smSwt_sys_config.iMaxsaflog)
    psmShmidx_d->sIdx_rev_saf.iFlink = i+1;
  else
    psmShmidx_d->sIdx_rev_saf.iFlink = 1;  
  if (swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  
  return(SUCCESS);
}

/***************************************************************
 ** 函数名      : swShmdelete_swt_rev_saf
 ** 功  能      : 删除冲正记录
 ** 作  者      : cjh
 ** 建立日期    : 20150112
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swShmdelete_swt_rev_saf(long lSaf_id)
{
  int i,j,ilLlink,ilRlink;

  if (swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while (i)
  {
    if (smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_id == lSaf_id)
    {
      /* 统计在共享内存当中的冲正数 */
      psmShmidx_d->iRecinshm[SFSAFLOGID] --;
  
      ilLlink = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iLlink;
      ilRlink = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
      if ((psmShmidx_d->sIdx_rev_saf.iSlink == i) &&
         (psmShmidx_d->sIdx_rev_saf.iElink == i)) 
      {
        psmShmidx_d->sIdx_rev_saf.iSlink = 0;
        psmShmidx_d->sIdx_rev_saf.iElink = 0;
      }
      else
      {
        if (psmShmidx_d->sIdx_rev_saf.iSlink == i)
        {
          if(ilRlink > 0)
            smShmbuf_d.psShm_rev_saf[ilRlink-1].sLinkinfo.iLlink = 0;
          psmShmidx_d->sIdx_rev_saf.iSlink = ilRlink;
        }
        else if (psmShmidx_d->sIdx_rev_saf.iElink == i)
        {
          if(ilLlink > 0)
            smShmbuf_d.psShm_rev_saf[ilLlink-1].sLinkinfo.iRlink = 0;
          psmShmidx_d->sIdx_rev_saf.iElink = ilLlink;
        }
        else
        {
          smShmbuf_d.psShm_rev_saf[ilLlink-1].sLinkinfo.iRlink = ilRlink;
          smShmbuf_d.psShm_rev_saf[ilRlink-1].sLinkinfo.iLlink = ilLlink;
        }
      }
      psmShmidx_d->sIdx_rev_saf.iFlink = i;
      smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.cTag = 0;    
      if(swShmV(iSEMNUM_REVSAF) == FAIL)
        return(FAIL);
      else
        return(SUCCESS);          
    }
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  if (swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);

  return (SHMNOTFOUND);
} 

/***************************************************************
 ** 函数名      : swShmupdate_swt_rev_saf
 ** 功  能      : 更新冲正记录
 ** 作  者      : cjh
 ** 建立日期    : 20150112
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swShmupdate_swt_rev_saf(long lSaf_id, struct swt_rev_saf sSwt_rev_saf)
{
  int i,j;

  if (swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while (i)
  {
    if (smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_id == lSaf_id)
    {
      memcpy((char *)(&(smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf)),
        (char *)&sSwt_rev_saf, sizeof(struct swt_rev_saf));

      if(swShmV(iSEMNUM_REVSAF) == FAIL)
        return(FAIL);
      else
        return(SUCCESS);  
    }
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  if (swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);

  return (SHMNOTFOUND);
}

/***************************************************************
 ** 函数名      : swShmselect_swt_rev_saf
 ** 功  能      : 查找冲正记录
 ** 作  者      : 
 ** 建立日期    : 20150112
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swShmselect_swt_rev_saf(long lSaf_id, struct swt_rev_saf *psSwt_rev_saf)
{
  int i;
       
  if(swShmP(iSEMNUM_REVSAF) == FAIL)
    return(FAIL);
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while (i)
  {
    if(smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_id == lSaf_id)
    {
      memcpy((char *)psSwt_rev_saf, (char *)(&(smShmbuf_d.psShm_rev_saf
        [i-1].sSwt_rev_saf)), sizeof(struct swt_rev_saf));
      if(swShmV(iSEMNUM_REVSAF) == FAIL)
        return(FAIL);
      else
        return(SUCCESS);  
    }
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  if(swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);

  return(SHMNOTFOUND);
}

/***************************************************************
 ** 函数名      : swShmselect_swt_rev_saf_all
 ** 功  能      : 查找所有冲正记录
 ** 作  者      : cjh
 ** 建立日期    : 20150112
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swShmselect_swt_rev_saf_all(struct swt_rev_saf *psSwt_rev_saf, short *iCount)
{
  int i,ilRc;
  short ilCount = 0;
  
  if(swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);
    
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while(i)
  {
    if(ilCount >= iMAXREVSAFREC)
    {
      swShmV(iSEMNUM_REVSAF);
      return(FAIL);
    }
    memcpy((char *)&(psSwt_rev_saf[ilCount]), 
      (char *)(&(smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf)), 
      sizeof(struct swt_rev_saf));
    ilCount ++;
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  if(swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  *iCount = ilCount;
  swVdebug(2,"ilCount=[%d]",ilCount);
   
  if(*iCount > 0)
    return(SUCCESS);
  else
    return(SHMNOTFOUND);
}

/***************************************************************
 ** 函数名      : swShmselect_overtime_min_revsaf
 ** 功  能      : 查找超时的最小冲正记录
 ** 作  者      : cjh
 ** 建立日期    : 20150112
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swShmselect_overtime_min_revsaf(long lTime, struct swt_rev_saf *psSwt_rev_saf)
{
  int i,j,ilTmp = 0;
  long ilSaf_id = lMAXSERNO;
  struct swt_rev_saf slSwt_rev_saf;

  if(swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);
    
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while(i)
  {
    if(smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_status[0] == '0' &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_overtime < lTime &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_overtime != 0 &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_id < ilSaf_id)
    {
      if (ilTmp == 0) ilTmp = 1;
      j = i;
      ilSaf_id = smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_id;
    }
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  if(ilTmp == 1)
  {
    memcpy((char *)psSwt_rev_saf, (char *)(&(smShmbuf_d.psShm_rev_saf[j-1].
      sSwt_rev_saf)), sizeof(struct swt_rev_saf));
  }
  if(swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);

  if (ilTmp == 1)
  {
    return(SUCCESS);
  }
  else
    return(SHMNOTFOUND);
}

/***************************************************************
 ** 函数名      : swShmselect_fail_rev_saf
 ** 功  能      : 获取 SAF 流水表中saf失败的tran_id
 ** 作  者      : cjh
 ** 建立日期    : 20150112
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
***************************************************************/
int swShmselect_fail_rev_saf(long * pTranid,short *iCount)
{
  int i;
  *iCount = 0;
  if(swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while(i)
  {
    if(smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_status[0] == '2')
    {
      pTranid[*iCount] = smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.tran_id;
      (*iCount) ++;
    }
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  if(swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  
  if(*iCount > 0)
    return(SUCCESS);
  else
    return(SHMNOTFOUND);
}

/***************************************************************
 ** 函数名      : swShmselect_saf_overtime_min_revsaf 
 ** 功  能      : 查找冲正saf saf_overtime字段最小的记录
 ** 作  者      : gengling
 ** 建立日期    : 2015/01/13
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : psSwt_rev_saf输出参数 pcLoc_flag输出参数
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_saf_overtime_min_revsaf(struct swt_rev_saf *psSwt_rev_saf,char *pcLoc_flag)
{
  int i,j,ilRc,ilTmp = 0;
  long ilSaf_overtime;
  struct swt_rev_saf slSwt_rev_saf;
  int count = 1;

  if(swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);
    
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while(i)
  {
    if((count == 1 && smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_flag[0] == '2' &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_overtime > 0) || \
       (count != 1 && smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_flag[0] == '2' &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_overtime < ilSaf_overtime))
    {
      if (ilTmp == 0) ilTmp = 1;
      j = i;
      ilSaf_overtime= smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_overtime;
    }
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
    count++;
  }
  if(ilTmp == 1)
  {
    memcpy((char *)psSwt_rev_saf, (char *)(&(smShmbuf_d.psShm_rev_saf[j-1].
      sSwt_rev_saf)), sizeof(struct swt_rev_saf));
    *pcLoc_flag = '0';
  }
  if(swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  
#if 0
  /* 从数据库中查找 */
  if((psmShmidx_d->iRecinfile[SFSAFLOGID]) > 0)
  {
    ilRc = swDbselect_saf_overtime_min_revsaf(&slSwt_rev_saf);
    if(ilRc == SUCCESS)
    {
      if(ilTmp == 1)
      {
        if(slSwt_rev_saf.saf_overtime < (psSwt_rev_saf->saf_overtime))
        {
          memcpy((char *)psSwt_rev_saf, (char *)&slSwt_rev_saf, 
            sizeof(struct swt_rev_saf));
          *pcLoc_flag = '1';
        }
      }
      else
      {
        memcpy((char *)psSwt_rev_saf, (char *)&slSwt_rev_saf, 
          sizeof(struct swt_rev_saf));  
        ilTmp = 1;
        *pcLoc_flag = '1';
      }
    }
    else if(ilRc == FAIL) return(FAIL);
  }
#endif
  if (ilTmp == 1)
  {
    return(SUCCESS);
  }
  else
    return(SHMNOTFOUND);
}

/***************************************************************
 ** 函数名      : swShmselect_saf_id_min_revsaf 
 ** 功  能      : 查找冲正saf超时的saf_id最小记录
 ** 作  者      : gengling
 ** 建立日期    : 2015/01/13
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : psSwt_rev_saf输出参数 pcLoc_flag输出参数
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_saf_id_min_revsaf(long lTime, 
  struct swt_rev_saf *psSwt_rev_saf,char *pcLoc_flag)
{
  int i,j,ilRc,ilTmp = 0;
  long ilSaf_id = lMAXSERNO;
  struct swt_rev_saf slSwt_rev_saf;

  if(swShmP(iSEMNUM_REVSAF) == FAIL) return(FAIL);
    
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while(i)
  {
    if(smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_flag[0] == '2' &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_overtime <= lTime &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_overtime != 0 &&
       smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_id < ilSaf_id)
    {
      if (ilTmp == 0) ilTmp = 1;
      j = i;
      ilSaf_id = smShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf.saf_id;
    }
    i = smShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  if(ilTmp == 1)
  {
    memcpy((char *)psSwt_rev_saf, (char *)(&(smShmbuf_d.psShm_rev_saf[j-1].
      sSwt_rev_saf)), sizeof(struct swt_rev_saf));
    *pcLoc_flag = '0';
  }
  if(swShmV(iSEMNUM_REVSAF) == FAIL) return(FAIL);
  
#if 0
  /* 从数据库中查找 */
  if((psmShmidx_d->iRecinfile[SFSAFLOGID]) > 0)
  {
    ilRc = swDbselect_saf_id_min_revsaf(lTime, &slSwt_rev_saf);
    if(ilRc == SUCCESS)
    {
      if(ilTmp == 1)
      {
        if(slSwt_rev_saf.saf_id < (psSwt_rev_saf->saf_id))
        {
          memcpy((char *)psSwt_rev_saf, (char *)&slSwt_rev_saf, 
            sizeof(struct swt_rev_saf));
          *pcLoc_flag = '1';
        }
      }
      else
      {
        memcpy((char *)psSwt_rev_saf, (char *)&slSwt_rev_saf, 
          sizeof(struct swt_rev_saf));  
        ilTmp = 1;
        *pcLoc_flag = '1';
      }
    }
    else if(ilRc == FAIL) return(FAIL);
  }
#endif
  if (ilTmp == 1)
  {
    return(SUCCESS);
  }
  else
    return(SHMNOTFOUND);
}
