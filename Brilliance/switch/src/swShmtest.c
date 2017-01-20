#include <stdio.h>
#include <errno.h>
#include <string.h>
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <stdarg.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include "swapi.h"
#include "swConstant.h"
#include "swNdbstruct.h"

#include "swShm.h"

union semun{
  int val;
  struct semid_ds *buf;
  ushort *array;
};

/* 变量定义 */
int iMaxtranlog,iMaxproclog,iMaxsaflog;
char agCommand[200],agParam[200],agParam1[200],agDebugtest[100];
typedef struct{char aTname[20];int iIndex;} shm_table;
char agCommand_buf[200],agParam_buf[200],agParam1_buf[200]; /*added by fzj at 2002.03.04 */

 extern struct shmidx_s *psmShmidx_s;
 extern struct shmbuf_s smShmbuf_s;    /***add by wanghao 20150408 PSBC_V1.0***/
 extern struct shmidx_d *psmShmidx_d;


void swShmprintlinkinfo(FILE *afStream);
void swShmprintlinkinfo_proc_log(FILE *afStream);
void swShmprintlinkinfo_rev_saf(FILE *afStream);
void swShmprintimf(FILE *afStream);
void swShmprint8583(FILE *afStream);
void swShmprint8583E(FILE *afStream);
void swShmprintcode(FILE *afStream);
void swShmprintother(FILE *afStream);
void swShmprintfmt_m(FILE *afStream);
void swShmprintfmt_g(FILE *afStream);
void swShmprintfmt_d(FILE *afStream);
void swShmprintroute_m(FILE *afStream);
void swShmprintroute_g(FILE *afStream);
void swShmprintqueue(FILE *afStream);
void swShmprintmatchport(FILE *afStream);	/* added by fzj at 2002.02.28 */
void swShmprinttask(FILE *afStream);
void swShmprintconfig(FILE *afStream);
void swShmprintroute_d(FILE *afStream);
void swShmprinttran_log(FILE *afStream);
void swShmprintproc_log(FILE *afStream);
void swShmprintrev_saf(FILE *afStream);
void swShmprintall(FILE *afStream);
void swShmprintstatus_s(FILE *afStream);
void swShmprintstatus_d(FILE* afStream);
void swShmprinthelp();
void swShmgetcmd(char * aaPrompt);
void swShmprintsem(FILE * afStream);
int swShmreset_all();
int swShmreset_tran_log();
int swShmreset_proc_log();
int swShmreset_rev_saf();
int swShmprintport_info(FILE *afStream);
extern int _swExpT2N(char *aTstr,char *aNstr);

/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
void swDebugtest(char *fmt,...)
#endif
#ifdef OS_AIX
void swDebugtest(frm,va_list)
char *frm
va_dcl
#endif
#ifdef OS_HPUX
void swDebugtest(char *fmt,...)
#endif
#ifdef OS_SCO
void swDebugtest(char *fmt,...)
#endif
#ifdef OS_SOLARIS
void swDebugtest(char *fmt,...)
#endif
/*******************************************************************/
{
  va_list ap;
  FILE *flDebug;
  if((flDebug = fopen(agDebugtest,"a")) == NULL)
    return;
  if(*fmt)
  {
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
    va_start( ap, fmt );
#endif
#ifdef OS_AIX
    va_start(ap);
#endif
#ifdef OS_HPUX
    va_start( ap, fmt );
#endif
#ifdef OS_SCO
    va_start( ap, fmt );
#endif
#ifdef OS_SOLARIS
    va_start( ap, fmt );
#endif
    vfprintf(flDebug,fmt,ap);
    va_end(ap);
  }
  fclose(flDebug);
}
/*******************************************************************/
int swShmcheck_g()
{
  struct swt_sys_config slSwt_sys_config;
  int ilShmid;
  char *alShmtmp_s,*alShmtmp_d;
  static short ilCheckcount = 0;
  int i = 0;
   /* 读SWCONFIG.CFG配置文件 */
  if (swShmcfgload(&slSwt_sys_config) != 0)
  {
    swDebugtest("读SWCONFIG.CFG配置文件失败");
    return -1;
  }

  iMaxtranlog = slSwt_sys_config.iMaxtranlog;
  iMaxproclog = slSwt_sys_config.iMaxproclog;
  iMaxsaflog = slSwt_sys_config.iMaxsaflog;

  ilShmid = shmget((key_t)slSwt_sys_config.iShmkey,0,IPC_EXCL|0666);
  if (ilShmid < 0)
  {
    swDebugtest("动态表共享内存不存在,shmkey[%d],errno:%d:%s",slSwt_sys_config.iShmkey,errno,strerror(errno));
    return -1;
  }

  if(shmdt(psmShmidx_d) && ilCheckcount != 0)
  {
    swDebugtest("释放动态表共享内存指针出错!errno:%d:%s",errno,strerror(errno));
    return(-1); 
  }

  if ((alShmtmp_d = shmat(ilShmid,(char *)0,SHM_RND)) == (char *)-1)
  {
    swDebugtest("联接动态表共享内存出错shmid=[%d]!errno:%d:%s",ilShmid,errno,strerror(errno));
    return(-1); 
  }
  psmShmidx_d = (struct shmidx_d *)alShmtmp_d;

  for(i = 0;i < SHM_TRANLOG_BUCKET_NUM;i ++)
  {
  sgShmbuf_d.psShm_tran_log[i] = (struct shm_tran_log *)
    (alShmtmp_d + psmShmidx_d->sIdx_tran_log[i].lOffset);
  }
  sgShmbuf_d.psShm_proc_log = (struct shm_proc_log *)
    (alShmtmp_d + psmShmidx_d->sIdx_proc_log.lOffset);

  sgShmbuf_d.psShm_rev_saf = (struct shm_rev_saf *)
    (alShmtmp_d + psmShmidx_d->sIdx_rev_saf.lOffset);

  if (shmdt(psmShmidx_s) && ilCheckcount != 0)
  {
    swDebugtest("释放静态表共享内存指针出错");
    return(-1);
  }
  ilCheckcount ++;

  if ((alShmtmp_s = shmat(psmShmidx_d->iShmid, (char *)0, SHM_RND)) == (char *)-1)
  {
    swDebugtest("联接静态表共享内存出错");
    return(-1); 
  }

  psmShmidx_s = (struct shmidx_s *)alShmtmp_s;

  smShmbuf_s.psSwt_sys_imf = (struct swt_sys_imf *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_imf.lOffset);

  smShmbuf_s.psSwt_sys_queue = (struct swt_sys_queue *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_queue.lOffset);

/* === begin of added by fzj at 2002.02.28 === */
  smShmbuf_s.psSwt_sys_matchport = (struct swt_sys_matchport *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_matchport.lOffset);
/* === end of added by fzj at 2002.02.28 === */

  smShmbuf_s.psSwt_sys_task = (struct swt_sys_task *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_task.lOffset);

  smShmbuf_s.psShm_sys_fmt_m = (struct shm_sys_fmt_m *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_m.lOffset);

  smShmbuf_s.psShm_sys_fmt_g = (struct shm_sys_fmt_g *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_g.lOffset);

  smShmbuf_s.psSwt_sys_fmt_d = (struct swt_sys_fmt_d *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_fmt_d.lOffset);

  smShmbuf_s.psShm_sys_route_m= (struct shm_sys_route_m*)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_route_m.lOffset);

  smShmbuf_s.psShm_sys_route_g= (struct shm_sys_route_g*)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_route_g.lOffset);

  smShmbuf_s.psSwt_sys_route_d = (struct swt_sys_route_d *)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_route_d.lOffset);

  smShmbuf_s.psSwt_sys_8583= (struct swt_sys_8583*)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_8583.lOffset);
    
  smShmbuf_s.psSwt_sys_8583E= (struct swt_sys_8583E*)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_8583E.lOffset);

  smShmbuf_s.psSwt_sys_code= (struct swt_sys_code*)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_code.lOffset);

  smShmbuf_s.psSwt_sys_other= (struct swt_sys_other*)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_other.lOffset);

  smShmbuf_s.psSwt_sys_config= (struct swt_sys_config*)
    (alShmtmp_s + psmShmidx_s->sIdx_sys_config.lOffset);
  
  return 0;
}

/*******************************************************************/
int swShmgetusedcount_tran_log(int bucket)
{
  int i,ilCount1 = 0,ilCount2 = 0;
  i = psmShmidx_d->sIdx_tran_log[bucket].iElink;
  printf("i = %d\n",i);
  printf("1111111\n");
  if((i > 0) && (sgShmbuf_d.psShm_tran_log[bucket][i-1].sLinkinfo.iRlink != 0))
  {
    return(-1);
  }
  i = psmShmidx_d->sIdx_tran_log[bucket].iSlink;
  if((i > 0) &&(sgShmbuf_d.psShm_tran_log[bucket][i-1].sLinkinfo.iLlink != 0))
  {
    return(-1);
  }
  while(i)
  {
    ilCount1++;
    if(ilCount1 > iMaxtranlog)
      break;
    i = sgShmbuf_d.psShm_tran_log[bucket][i-1].sLinkinfo.iRlink;
  }
  for(i = 0;i < iMaxtranlog;i++)
  {
    if(sgShmbuf_d.psShm_tran_log[bucket][i].sLinkinfo.cTag == 1)
      ilCount2++;
  }
  
  if(ilCount1 == ilCount2)
    return(ilCount1);

  return(-1);
}


#if 0
int swShmgetusedcount_tran_log()
{
  int i,ilCount1 = 0,ilCount2 = 0;
  i = psmShmidx_d->sIdx_tran_log.iElink;
  if((i > 0) && (sgShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iRlink != 0))
    return(-1);
  i = psmShmidx_d->sIdx_tran_log.iSlink;
  if((i > 0) &&(sgShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iLlink != 0))
    return(-1);
  while(i)
  {
    ilCount1++;
    if(ilCount1 > iMaxtranlog)
      break;
    i = sgShmbuf_d.psShm_tran_log[i-1].sLinkinfo.iRlink;
  }
  for(i = 0;i < iMaxtranlog;i++)
  {
    if(sgShmbuf_d.psShm_tran_log[i].sLinkinfo.cTag == 1)
      ilCount2++;
  }
  if(ilCount1 == ilCount2)
    return(ilCount1);
  return(-1);
}

#endif
/*******************************************************************/
int swShmgetusedcount_proc_log()
{
  int i,ilCount1 = 0,ilCount2 = 0;
  i = psmShmidx_d->sIdx_proc_log.iSlink;
  while(i)
  {
    ilCount1++;
    if(ilCount1 > iMaxproclog)
      break;
    i = sgShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
  }
  for(i=0;i<iMaxproclog;i++)
    if(sgShmbuf_d.psShm_proc_log[i].sLinkinfo.cTag == 1)
      ilCount2++;
  if(ilCount1 == ilCount2)
    return(ilCount1);
  return(-1);
}

/*******************************************************************/
int swShmgetusedcount_rev_saf()
{
  int i,ilCount1 = 0,ilCount2 = 0;
  i = psmShmidx_d->sIdx_rev_saf.iSlink;
  while(i)
  {
    ilCount1++;
    if(ilCount1 > iMaxsaflog)
      break;
    i = sgShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
  }
  for(i=0;i<iMaxsaflog;i++)
    if(sgShmbuf_d.psShm_rev_saf[i].sLinkinfo.cTag == 1)
      ilCount2++;
  if(ilCount1 == ilCount2)
    return(ilCount1);
  return(-1); 
}

/*******************************************************************/
void swShmprintall(FILE *afStream)
{
  FILE * flOutput = afStream;
  swShmprintstatus_s(flOutput);
  swShmprintimf(flOutput);
  swShmprintqueue(flOutput);
  swShmprintmatchport(flOutput);	/* added by fzj at 2002.02.28 */
  swShmprinttask(flOutput);
  swShmprint8583(flOutput);
  swShmprint8583E(flOutput);
  swShmprintcode(flOutput);
  swShmprintconfig(flOutput);
  swShmprintother(flOutput);
  swShmprintfmt_m(flOutput);
  swShmprintfmt_g(flOutput);
  swShmprintfmt_d(flOutput);
  swShmprintroute_m(flOutput);
  swShmprintroute_g(flOutput);
  swShmprintroute_d(flOutput);
  swShmprintsem(flOutput);
  swShmprintstatus_d(flOutput);
  swShmprintlinkinfo(flOutput);
  swShmprinttran_log(flOutput);
  swShmprintlinkinfo_proc_log(flOutput);
  swShmprintproc_log(flOutput);
  swShmprintlinkinfo_rev_saf(flOutput);
  swShmprintrev_saf(flOutput);
  swShmprintport_info(flOutput);
}

/*******************************************************************/
void swShmprintimf(FILE * afStream)
{
  int i,j,ilRc;
  char alTmpexp[2*iFLDVALUELEN+1]; /* add by wangpan 2001/11/30 */
  
  FILE * flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"---------------------------swt_sys_imf----------------------------\n");

  fprintf(flOutput,"swt_sys_imf表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_imf.iCount);

  fprintf(flOutput,
"imf_name    imf_type imf_len imf_dec  imf_check   imf_id  imf_china\n");

  j = 0;
  
  for(i=0;i<psmShmidx_s->sIdx_sys_imf.iCount;i++)
  {
    /* add by wangpan 2001/11/30                      */
    memset(alTmpexp,0x00,sizeof(alTmpexp));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_imf[i].imf_check,alTmpexp);
    /* if(ilRc!=0) return(ilRc);                      */
    /* end add by wangpan 2001/11/30                  */
    fprintf(flOutput,"%-13s%-9s%-9d%-9d%-11s%-9d%-10s\n",
    		smShmbuf_s.psSwt_sys_imf[i].imf_name,
    		smShmbuf_s.psSwt_sys_imf[i].imf_type,
    		smShmbuf_s.psSwt_sys_imf[i].imf_len,
    		smShmbuf_s.psSwt_sys_imf[i].imf_dec,
    		alTmpexp, 
    		smShmbuf_s.psSwt_sys_imf[i].imf_id,
    		smShmbuf_s.psSwt_sys_imf[i].imf_china
    		);
    
    /*
    fprintf(flOutput,"%-13s%-9s%-9d%-9d%-11s%-9d%-10s\n",
    		smShmbuf_s.psSwt_sys_imf[i].imf_name,
    		smShmbuf_s.psSwt_sys_imf[i].imf_type,
    		smShmbuf_s.psSwt_sys_imf[i].imf_len,
    		smShmbuf_s.psSwt_sys_imf[i].imf_dec,
    		smShmbuf_s.psSwt_sys_imf[i].imf_check, 
    		smShmbuf_s.psSwt_sys_imf[i].imf_id,
    		smShmbuf_s.psSwt_sys_imf[i].imf_china
    		);                                       */     
    j++;
    if(j>20)
    {
      j =0;
      if(afStream == stdout)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
        else if(strcmp(agCommand,"D") == 0)
        {
          i += 21;
          if(i >= psmShmidx_s->sIdx_sys_imf.iCount)
            i = psmShmidx_s->sIdx_sys_imf.iCount - 1;
        }
        else if(strcmp(agCommand,"Q") == 0)
          return;  
      }
  fprintf(flOutput,
"imf_name    imf_type imf_len imf_dec  imf_check   imf_id  imf_china\n");
    }
  }
  fprintf(flOutput,"\n\n");
}
/*******************************************************************/
void swShmprint8583(FILE *afStream)
{
  int i,j,ilRc;
  char alTmpexp[2*iFLDVALUELEN+1];   /* add by wangpan 2001/11/30 */
  FILE * flOutput;

    flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"----------------------------swt_sys_8583---------------------------\n");
 
  fprintf(flOutput,"swt_sys_8583表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_8583.iCount);

  fprintf(flOutput,
"tab_id fld_id fld_name        fld_type fld_attr fld_len  fld_rule\n");

  j = 0;

  for(i = 0;i < psmShmidx_s->sIdx_sys_8583.iCount;i++)
  {
    /* add by wangpan 2001/11/30                      */
    memset(alTmpexp,0x00,sizeof(alTmpexp));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_8583[i].fld_rule,alTmpexp);
    /* if(ilRc!=0) return(ilRc);                      */
    /* end add by wangpan 2001/11/30                  */
    
    //fprintf(flOutput,"%-7d%-7d%-18s%-8d%-8d%-8d%16s\n", modified by mzg 20150428
    fprintf(flOutput,"%-7d%-10ld%-18s%-8d%-8d%-8d%16s\n",
    		smShmbuf_s.psSwt_sys_8583[i].tab_id,
    		smShmbuf_s.psSwt_sys_8583[i].fld_id,
    		smShmbuf_s.psSwt_sys_8583[i].fld_name,
    		smShmbuf_s.psSwt_sys_8583[i].fld_type,
    		smShmbuf_s.psSwt_sys_8583[i].fld_attr,
    		smShmbuf_s.psSwt_sys_8583[i].fld_len,
    		alTmpexp);

    j++;
    if(j > 20)
    {
      j = 0;
      if(afStream == stdout)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
        else if(strcmp(agCommand,"D") == 0)
        {
          i += 21;
          if(i >= psmShmidx_s->sIdx_sys_8583.iCount)
            i = psmShmidx_s->sIdx_sys_8583.iCount - 1;
        }
        else if(strcmp(agCommand,"Q") == 0)
          return;
      }

  fprintf(flOutput,
"tab_id fld_id fld_name        fld_type fld_attr fld_len  fld_rule\n");
    }
  }
  fprintf(flOutput,"\n\n");
}


/*added by ph -- begin*/
/*******************************************************************/
void swShmprint8583E(FILE *afStream)
{
  int i,j,ilRc;
  char alTmp_lenunpk[iEXPRESSLEN]; 
  char alTmp_lenpack[iEXPRESSLEN];
  char alTmp_valunpk[iEXPRESSLEN];
  char alTmp_valpack[iEXPRESSLEN];
  char alTmp_rule[iEXPRESSLEN];  
  FILE * flOutput;

    flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"----------------------------swt_sys_8583E---------------------------\n");
 
  fprintf(flOutput,"swt_sys_8583E表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_8583E.iCount);

  fprintf(flOutput,
"tab_id fld_id fld_name        fld_type fld_len fld_len2\nlenunpk\nlenpack\nvalunpk\nvalpack\nfld_rule\n");

  j = 0;

  for(i = 0;i < psmShmidx_s->sIdx_sys_8583E.iCount;i++)
  {
    
    memset(alTmp_lenpack,0x00,sizeof(alTmp_lenunpk));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_8583E[i].fld_lenunpk,alTmp_lenunpk);
    
    memset(alTmp_lenpack,0x00,sizeof(alTmp_lenpack));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_8583E[i].fld_lenpack,alTmp_lenpack);
    
    memset(alTmp_valunpk,0x00,sizeof(alTmp_valunpk));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_8583E[i].fld_valunpk,alTmp_valunpk);
    
    memset(alTmp_valpack,0x00,sizeof(alTmp_valpack));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_8583E[i].fld_valpack,alTmp_valpack);
    
    memset(alTmp_rule,0x00,sizeof(alTmp_rule));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_8583E[i].fld_rule,alTmp_rule);

           
/*there are maybe some problems with the chapter*/   
//    fprintf(flOutput,"%-7d%-7d%-24s%-8ld%-7d%-7d\n%-151s\n%-151s\n%-151s\n%-151s\n%-151s\n", modified by mzg 20150428
    fprintf(flOutput,"%-7d%-10ld%-24s%-8ld%-7d%-7d\n%-151s\n%-151s\n%-151s\n%-151s\n%-151s\n",
    		smShmbuf_s.psSwt_sys_8583E[i].tab_id,
    		smShmbuf_s.psSwt_sys_8583E[i].fld_id,
    		smShmbuf_s.psSwt_sys_8583E[i].fld_name,
    		smShmbuf_s.psSwt_sys_8583E[i].fld_type,
    		smShmbuf_s.psSwt_sys_8583E[i].fld_len,
    		smShmbuf_s.psSwt_sys_8583E[i].fld_len2,
    		alTmp_lenunpk,
    		alTmp_lenpack,
    		alTmp_valunpk,
    		alTmp_valpack,
    		alTmp_rule);
 
    j++;
    if(j > 20)
    {
      j = 0;
      if(afStream == stdout)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
        else if(strcmp(agCommand,"D") == 0)
        {
          i += 21;
          if(i >= psmShmidx_s->sIdx_sys_8583E.iCount)
            i = psmShmidx_s->sIdx_sys_8583E.iCount - 1;
        }
        else if(strcmp(agCommand,"Q") == 0)
          return;
      }

  fprintf(flOutput,
"tab_id fld_id fld_name        fld_type fld_len fld_len2\nlenunpk\nlenpack\nvalunpk\nvalpack\nfld_rule\n");
    }
  }
  fprintf(flOutput,"\n\n");
}
/*added by ph -- end*/

/*******************************************************************/  
void swShmprintcode(FILE *afStream)
{
  int i,j;
  FILE * flOutput;
 
    flOutput = afStream;
  
  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"-----------------------------swt_sys_code--------------------------\n");

  fprintf(flOutput,"swt_sys_code表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_code.iCount);

  fprintf(flOutput,
"msg_code  msg_desc                                    msg_action msg_q\n");

  j = 0;

  for(i = 0;i < psmShmidx_s->sIdx_sys_code.iCount;i++)
  {
      fprintf(flOutput,"%-8ld%-48s%-10d%-8d\n",
      		smShmbuf_s.psSwt_sys_code[i].msg_code,
      		smShmbuf_s.psSwt_sys_code[i].msg_desc,
      		smShmbuf_s.psSwt_sys_code[i].msg_action,
      		smShmbuf_s.psSwt_sys_code[i].msg_q);

    j++;
    if(j > 20)
    {
      j = 0;
      if(afStream == stdout)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
          i = -1;
        }
        else if(strcmp(agCommand,"D") == 0)
        {
          i += 21;
          if(i >= psmShmidx_s->sIdx_sys_code.iCount)
            i = psmShmidx_s->sIdx_sys_code.iCount - 1;
        }
        else if(strcmp(agCommand,"Q") == 0)
          return;
      }

  fprintf(flOutput,
"msg_code  msg_desc                                    msg_action msg_q\n");
    }
  }
    
  fprintf(flOutput,"\n\n");
}

/*******************************************************************/

void swShmprintother(FILE *afStream)
{
  int i,j;
  FILE * flOutput;

    flOutput = afStream;
  
  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"-----------------------------swt_sys_other-------------------------\n");
  
  fprintf(flOutput,"swt_sys_other表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_other.iCount);
  		
  fprintf(flOutput,
"other_id other_name      local_q other_grp other_q\n");

  j = 0;

  for(i = 0;i < psmShmidx_s->sIdx_sys_other.iCount;i++)
  {
    fprintf(flOutput,"%-8d%-18s%-8d%-9d%-10d\n",
    		smShmbuf_s.psSwt_sys_other[i].other_id,
    		smShmbuf_s.psSwt_sys_other[i].other_name,
    		smShmbuf_s.psSwt_sys_other[i].local_q,
    		smShmbuf_s.psSwt_sys_other[i].other_grp,
    		smShmbuf_s.psSwt_sys_other[i].other_q);
    j++;
    if(j > 20)
    {
      j =0;
      if(afStream == stdout)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
         else if(strcmp(agCommand,"Q") == 0)
           return;
         else if(strcmp(agCommand,"U") == 0)
         {
           i += 21;
           if(i >= psmShmidx_s->sIdx_sys_other.iCount)
             i = psmShmidx_s->sIdx_sys_other.iCount - 1;
          }
        }
  fprintf(flOutput,
"other_id other_name      local_q other_grp other_q\n");
    }
  }
  
  fprintf(flOutput,"\n\n");
}

/*******************************************************************/

void swShmprintfmt_m(FILE *afStream)
{
  int i,j;
  
  FILE * flOutput;

  flOutput = afStream;
  
  fprintf(flOutput,"\n\n");
  
  fprintf(flOutput,
"-------------------------------swt_sys_fmt_m----------------------------\n");

#ifndef HASH
  fprintf(flOutput,"swt_sys_fmt_m表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_fmt_m.iCount);
#else
  fprintf(flOutput,"swt_sys_fmt_m表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_fmt_m_hash.iCount);
#endif
  		
  fprintf(flOutput,
		"q_id tran_code  tran_step tran_type"\
		" fmt_group fmt_prior\n");
  j = 0;
  long dataoffset;

#ifndef HASH
  for (i = 0;i < psmShmidx_s->sIdx_sys_fmt_m.iCount;i++)
#else
  for (i = 0;i < psmShmidx_s->sIdx_sys_fmt_m_hash.iCount;i++)
#endif
  {
#ifndef HASH
    /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
    /* fprintf(flOutput,"%-5d%-12s%-10d%-10s%-10d%-10d\n",*/
    /* mod by gengling at 2015.05.25 one line fmt_group 10->11 PSBC_V1.0 */
    fprintf(flOutput,"%-5d%-12s%-10d%-10s%-11ld%-10d\n",
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.q_id,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_code,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_step,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_type,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.fmt_group,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.fmt_prior);
#else
    dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_m_hash, sizeof(long));
    fprintf(flOutput,"%-5d%-12s%-10d%-10s%-11ld%-10d\n",
        ((struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash+dataoffset))[i].sSwt_sys_fmt_m.q_id,
        ((struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash+dataoffset))[i].sSwt_sys_fmt_m.tran_code,
        ((struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash+dataoffset))[i].sSwt_sys_fmt_m.tran_step,
        ((struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash+dataoffset))[i].sSwt_sys_fmt_m.tran_type,
        ((struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash+dataoffset))[i].sSwt_sys_fmt_m.fmt_group,
        ((struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash+dataoffset))[i].sSwt_sys_fmt_m.fmt_prior);
#endif
   
    
    /* delete by wangpan 2001/11/30                              */
    /*fprintf(flOutput,"%-5d%-12s%-10d%-10s%-11s%-11s%-10d%-10d\n",
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.q_id,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_code,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_step,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.tran_type,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.mac_create,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.mac_check,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.fmt_group,
    	smShmbuf_s.psShm_sys_fmt_m[i].sSwt_sys_fmt_m.fmt_prior); */
    /* end delete                                                */    
    j++;
    if(j > 20)
    {
      j =0;
      if(stdout == afStream)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
         else if(strcmp(agCommand,"Q") == 0)
           return;
         else if(strcmp(agCommand,"U") == 0)
         {
           i += 21;
#ifndef HASH
           if(i >= psmShmidx_s->sIdx_sys_fmt_m.iCount)
             i = psmShmidx_s->sIdx_sys_fmt_m.iCount - 1;
#else
           if(i >= psmShmidx_s->sIdx_sys_fmt_m_hash.iCount)
             i = psmShmidx_s->sIdx_sys_fmt_m_hash.iCount - 1;
#endif
          }
        }

  fprintf(flOutput,
                "q_id tran_code  tran_step tran_type"\
                " fmt_group fmt_prior\n");
    }
  }
  
  fprintf(flOutput,"\n\n");
}

/*******************************************************************/
void swShmprintfmt_g(FILE *afStream)
{
  int i,j,ilRc;
  char alTmpexp1[2*iFLDVALUELEN+1];   /* add by wangpan 2001/11/30 */
  char alTmpexp2[2*iFLDVALUELEN+1];   /* add by wangpan 2001/11/30 */
  FILE * flOutput;

  flOutput = afStream;
  
  fprintf(flOutput,"\n\n");
  
  fprintf(flOutput,
"-----------------------swt_sys_fmt_grp-------------------\n");

#ifndef HASH
  fprintf(flOutput,"swt_sys_fmt_grp表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_fmt_g.iCount);
#else
  fprintf(flOutput,"swt_sys_fmt_grp表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_fmt_g_hash.iCount);
#endif
  		
  fprintf(flOutput,"fmt_group fmt_desc                    offset count tran_type"\
    " mac_create mac_check tran_sort\n");
  j = 0;
  long dataoffset;
 		
#ifndef HASH
  for(i = 0;i < psmShmidx_s->sIdx_sys_fmt_g.iCount;i++)
#else
  for(i = 0;i < psmShmidx_s->sIdx_sys_fmt_g_hash.iCount;i++)
#endif
  {
    /* add by wangpan 2001/11/30                      */
    memset(alTmpexp1,0x00,sizeof(alTmpexp1));
    ilRc = _swExpT2N(smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.mac_create,alTmpexp1);
    memset(alTmpexp2,0x00,sizeof(alTmpexp2));
    ilRc = _swExpT2N(smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.mac_check,alTmpexp2);
    /* if(ilRc!=0) return(ilRc);                      */
    /* end add by wangpan 2001/11/30                  */
    
#ifndef HASH
    /*fprintf(flOutput,"%-4d%-38s%-7ld%-6d%-4d%-8s%-8s%-10s\n", */
    fprintf(flOutput,"%-10ld%-38s%-7ld%-6d%-4d%-8s%-8s%-10s\n",   /*modified by baiqj20150428 PSBC_V1.0 %-4d -> %-10ld*/
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.fmt_group,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.fmt_desc,
        smShmbuf_s.psShm_sys_fmt_g[i].lOffset,
        smShmbuf_s.psShm_sys_fmt_g[i].iCount,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.tran_type,
    	alTmpexp1,
    	alTmpexp2,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.tran_sort);
#else
    dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_g_hash, sizeof(long));
    fprintf(flOutput,"%-10ld%-48s%-7ld%-6d%-4d%-8s%-8s%-10s\n",   /*modified by baiqj20150428 PSBC_V1.0 %-4d -> %-10ld*/
            ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[i].sSwt_sys_fmt_g.fmt_group,
            ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[i].sSwt_sys_fmt_g.fmt_desc,
            ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[i].lOffset,
            ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[i].iCount,
            ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[i].sSwt_sys_fmt_g.tran_type,
            alTmpexp1,
            alTmpexp2,
            ((struct shm_sys_fmt_g *)((smShmbuf_s.psShm_sys_fmt_g_hash)+dataoffset))[i].sSwt_sys_fmt_g.tran_sort);
#endif
    	
    /* delete by wangpan 2001/11/30
    fprintf(flOutput,"%-4d%-38s%-7ld%-6d%-4d%-8s%-8s%-10s\n",
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.fmt_group,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.fmt_desc,
        smShmbuf_s.psShm_sys_fmt_g[i].lOffset,
        smShmbuf_s.psShm_sys_fmt_g[i].iCount,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.tran_type,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.mac_create,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.mac_check,
    	smShmbuf_s.psShm_sys_fmt_g[i].sSwt_sys_fmt_g.tran_sort);
    */
    j++;
    if(j > 20)
    { 
      j = 0;
      if(stdout == afStream)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
         else if(strcmp(agCommand,"Q") == 0)
           return;
         else if(strcmp(agCommand,"U") == 0)
         {
           i += 21;
#ifndef HASH
           if(i >= psmShmidx_s->sIdx_sys_fmt_g.iCount)
             i = psmShmidx_s->sIdx_sys_fmt_g.iCount - 1;
#else
           if(i >= psmShmidx_s->sIdx_sys_fmt_g_hash.iCount)
             i = psmShmidx_s->sIdx_sys_fmt_g_hash.iCount - 1;
#endif
          }
        }

      fprintf(flOutput,"fmt_group fmt_desctran_type"\
        " mac_create mac_check tran_sort\n");
    }
  }
  
  fprintf(flOutput,"\n\n");
}

/*******************************************************************/
void swShmprintfmt_d(FILE *afStream)
{
  int i,j,ilRc;
  char alTmpexp1[2*iFLDVALUELEN+1];   /* add by wangpan 2001/11/30 */
  char alTmpexp2[2*iFLDVALUELEN+1];   /* add by wangpan 2001/11/30 */
  FILE * flOutput;
  
    flOutput = afStream;

  fprintf(flOutput,"\n\n");
  
  fprintf(flOutput,
"-----------------------swt_sys_fmt_d-------------------\n");

  fprintf(flOutput,"swt_sys_fmt_d表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_fmt_d.iCount);
  		
  fprintf(flOutput,"fmt_group id  imf_name     fld_id fld_express"\
    "                      fmt_flag\nfmt_cond\n");
  j = 0;
 		
  for(i = 0;i < psmShmidx_s->sIdx_sys_fmt_d.iCount;i++)
  {
    /* add by wangpan 2001/11/30                      */
    memset(alTmpexp1,0x00,sizeof(alTmpexp1));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_fmt_d[i].fld_express,alTmpexp1);
    memset(alTmpexp2,0x00,sizeof(alTmpexp2));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_fmt_d[i].fmt_cond,alTmpexp2);
    /* if(ilRc!=0) return(ilRc);                      */
    /* end add by wangpan 2001/11/30                  */
    
    /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
    /* fprintf(flOutput,"%-10d%-5d%-12s%-7d%-34s%-10s\n%-40s\n", */
    //fprintf(flOutput,"%-10ld%-5d%-12s%-7d%-34s%-10s\n%-40s\n", modified by mzg 20150428
    /* mod by gengling at 2015.05.22 one line fmt_group 10->11 PSBC_V1.0 */
    fprintf(flOutput,"%-11ld%-5d%-12s%-10ld%-34s%-10s\n%-40s\n",
    	smShmbuf_s.psSwt_sys_fmt_d[i].fmt_group,
    	smShmbuf_s.psSwt_sys_fmt_d[i].id,
    	smShmbuf_s.psSwt_sys_fmt_d[i].imf_name,
    	smShmbuf_s.psSwt_sys_fmt_d[i].fld_id,
    	alTmpexp1,
    	smShmbuf_s.psSwt_sys_fmt_d[i].fmt_flag,
    	alTmpexp2);
    
    /* delete by wangpan 
    //fprintf(flOutput,"%-10d%-5d%-12s%-7d%-34s%-10s\n%-40s\n", modified by mzg 20150428
    fprintf(flOutput,"%-10d%-5d%-12s%-10ld%-34s%-10s\n%-40s\n",
    	smShmbuf_s.psSwt_sys_fmt_d[i].fmt_group,
    	smShmbuf_s.psSwt_sys_fmt_d[i].id,
    	smShmbuf_s.psSwt_sys_fmt_d[i].imf_name,
    	smShmbuf_s.psSwt_sys_fmt_d[i].fld_id,
    	smShmbuf_s.psSwt_sys_fmt_d[i].fld_express,
    	smShmbuf_s.psSwt_sys_fmt_d[i].fmt_flag,
    	smShmbuf_s.psSwt_sys_fmt_d[i].fmt_cond);
    */
    j++;
    if(j > 20)
    {
      j = 0;
      if(stdout == afStream)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
         else if(strcmp(agCommand,"Q") == 0)
           return;
         else if(strcmp(agCommand,"U") == 0)
         {
           i += 21;
           if(i >= psmShmidx_s->sIdx_sys_fmt_d.iCount)
             i = psmShmidx_s->sIdx_sys_fmt_d.iCount - 1;
          }
        }

  fprintf(flOutput,"fmt_group id  imf_name     fld_id fld_express"\
    "                      fmt_flag\nfmt_cond\n");
    }
  }
  
  fprintf(flOutput,"\n\n");
}
/*******************************************************************/
void swShmprintroute_m(FILE *afStream)
{
  int i,j;
  FILE * flOutput;

  flOutput = afStream;
  
  fprintf(flOutput,"\n\n");
  
  fprintf(flOutput,
"--------------------------swt_sys_route_m-----------------------\n");

#ifndef HASH
  fprintf(flOutput,"swt_sys_route_m表记录数:%d\n偏移量:%ld\n",
  		psmShmidx_s->sIdx_sys_route_m.iCount,
                psmShmidx_s->sIdx_sys_route_m.lOffset);
#else
  fprintf(flOutput,"swt_sys_route_m表记录数:%d\n偏移量:%ld\n",
  		psmShmidx_s->sIdx_sys_route_m_hash.iCount,
                psmShmidx_s->sIdx_sys_route_m_hash.lOffset);
#endif
  		
  fprintf(flOutput,
"q_id        trancode              route_grp \n");

  j = 0;
  long dataoffset;

#ifndef HASH
  for (i = 0;i < psmShmidx_s->sIdx_sys_route_m.iCount;i++)
#else
  for (i = 0;i < psmShmidx_s->sIdx_sys_route_m_hash.iCount;i++)
#endif
  {
#ifndef HASH
    fprintf(flOutput,"%-12d%-22s%-10d\n",
    	smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.q_id,
    	smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.trancode,
    	smShmbuf_s.psShm_sys_route_m[i].sSwt_sys_route_m.route_grp);
#else
    dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_route_m_hash, sizeof(long));
    fprintf(flOutput,"%-12d%-22s%-10d\n",
            ((struct shm_sys_route_m *)(smShmbuf_s.psShm_sys_route_m_hash+dataoffset))[i].sSwt_sys_route_m.q_id,
            ((struct shm_sys_route_m *)(smShmbuf_s.psShm_sys_route_m_hash+dataoffset))[i].sSwt_sys_route_m.trancode,
            ((struct shm_sys_route_m *)(smShmbuf_s.psShm_sys_route_m_hash+dataoffset))[i].sSwt_sys_route_m.route_grp);
#endif
    j++;
    if(j > 20)
    {
      j = 0;
      if(stdout == afStream)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
         else if(strcmp(agCommand,"Q") == 0)
           return;
         else if(strcmp(agCommand,"U") == 0)
         {
           i += 21;
#ifndef HASH
           if(i >= psmShmidx_s->sIdx_sys_route_m.iCount)
             i = psmShmidx_s->sIdx_sys_route_m.iCount - 1;
#else
           if(i >= psmShmidx_s->sIdx_sys_route_m_hash.iCount)
             i = psmShmidx_s->sIdx_sys_route_m_hash.iCount - 1;
#endif
          }
        }

  fprintf(flOutput,
"q_id        trancode              route_grp \n");
    }
  }
  
  fprintf(flOutput,"\n\n");
}

/*******************************************************************/
void swShmprintroute_g(FILE *afStream)
{
  int i,j;
  FILE * flOutput;

  flOutput = afStream;
  
  fprintf(flOutput,"\n\n");
  
  fprintf(flOutput,
"-----------------------swt_sys_route_grp-------------------\n");

  fprintf(flOutput,"swt_sys_route_grp表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_route_g.iCount);
  		
  fprintf(flOutput,
"route_group         route_desc    			offset     count\n");

  j = 0;
  		
  for(i = 0;i < psmShmidx_s->sIdx_sys_route_g.iCount;i++)
  {
    fprintf(flOutput,"%-20d%-38s%-10ld%-10d\n",
    	smShmbuf_s.psShm_sys_route_g[i].sSwt_sys_route_g.route_grp,
    	smShmbuf_s.psShm_sys_route_g[i].sSwt_sys_route_g.route_desc,
        smShmbuf_s.psShm_sys_route_g[i].lOffset,
        smShmbuf_s.psShm_sys_route_g[i].iCount);
    j++;
    if(j > 20)
    {
      j = 0;
      if(stdout == afStream)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
         else if(strcmp(agCommand,"Q") == 0)
           return;
         else if(strcmp(agCommand,"U") == 0)
         {
           i += 21;
           if(i >= psmShmidx_s->sIdx_sys_route_g.iCount)
             i = psmShmidx_s->sIdx_sys_route_g.iCount - 1;
          }
        }

  fprintf(flOutput,
"route_group         route_desc    			offset     count\n");
    }
  }
  
  fprintf(flOutput,"\n\n");
}

/*******************************************************************/
void swShmprintqueue(FILE *afStream)
{
  int i,ilRc;
  char alTmpexp[2*iFLDVALUELEN+1]; /* add by wangpan 2001/11/30 */ 
  FILE * flOutput;
 
  flOutput = afStream;
  
  fprintf(flOutput,"\n\n");
  
  fprintf(flOutput,
"-----------------------swt_sys_queue-------------------\n");

  fprintf(flOutput,"swt_sys_queue表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_queue.iCount);
  
  for(i = 0;i < psmShmidx_s->sIdx_sys_queue.iCount;i++)
  {
        fprintf(flOutput,"第%d条记录:\n",i+1);
  	fprintf(flOutput,"QID  = %d\n",
  		smShmbuf_s.psSwt_sys_queue[i].q_id);
	fprintf(flOutput,"PORTID = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].q_port_id);
	fprintf(flOutput,"APPNAME = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].q_name);
	/*
	fprintf(flOutput,"q_statu = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].q_statu);
	fprintf(flOutput,"down_cause = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].down_cause);
	*/
	fprintf(flOutput,"MSGFORMAT = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].msg_format);
	fprintf(flOutput,"OVERTIME = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].rev_overtime);
	fprintf(flOutput,"REVNUM = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].rev_num);
	
	/* add by wangpan 2001/11/30                      */
        memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].tc_unpack,alTmpexp);
        /* if(ilRc!=0) return(ilRc);                      */
        fprintf(flOutput,"UNPACKEXP = %s\n",alTmpexp);		
        /*fprintf(flOutput,"UNPACKEXP = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].tc_unpack); */
	/* end add by wangpan 2001/11/30                  */	
	
	/* add by wangpan 2001/11/30                      */
        memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].tc_pack,alTmpexp);
        /* if(ilRc!=0) return(ilRc);                      */
        fprintf(flOutput,"PACKEXP = %s\n",alTmpexp);
	/*fprintf(flOutput,"PACKEXP = %s\n", 
		smShmbuf_s.psSwt_sys_queue[i].tc_pack);   */
        /* end add by wangpan 2001/11/30                  */
	
	/*fprintf(flOutput,"tran_type = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].tran_type); */
	
	fprintf(flOutput,"DEFRCD = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].def_rs);

	fprintf(flOutput,"DEFGRP = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].def_gs);
	fprintf(flOutput,"DEFFLD = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].def_fs);
	fprintf(flOutput,"REV_NOGOOD = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].rev_nogood);
	fprintf(flOutput,"REV_NOM = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].rev_nomrev);
	fprintf(flOutput,"ISOTABID = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].isotab_id);
	fprintf(flOutput,"BITMAP_TYPE = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].bitmap_type);
	fprintf(flOutput,"MSGPRIOR = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].msg_prio);
	
	/* add by wangpan 2001/11/30                       */
        memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].key_express,alTmpexp);
        /* if(ilRc!=0) return(ilRc);                       */
        fprintf(flOutput,"MSGHEADEXP = %s\n",alTmpexp);
	/*fprintf(flOutput,"MSGHEADEXP = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].key_express);*/
	/* end add by wangpan 2001/11/30                   */
		
	fprintf(flOutput,"COMTYPE = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].comm_type);
	fprintf(flOutput,"COMATTR = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].comm_attr);
		
	/* add by wangpan 2001/11/30                        */
        memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].resu_express,alTmpexp);
        /* if(ilRc!=0) return(ilRc);                        */
        fprintf(flOutput,"APPRESUEXP = %s\n",alTmpexp);
        /*fprintf(flOutput,"APPRESUEXP = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].resu_express);*/
	/* end add by wangpan 2001/11/30                    */
	
	fprintf(flOutput,"CODETYPE = %s\n",
		smShmbuf_s.psSwt_sys_queue[i].code_type);
	
	
        memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].resu_revexp,alTmpexp);
	fprintf(flOutput,"REVRESUEXP = %s\n",alTmpexp);
		
	fprintf(flOutput,"MAXTRAN = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].max_tran);
		
/* add by ph 2002.10.16 */
	fprintf(flOutput,"TRANSHM_MAX = %ld\n",
		smShmbuf_s.psSwt_sys_queue[i].transhm_max);
		
/* add by ph 2002.10.16 */

	fprintf(flOutput,"REJEGRP = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].reje_grp);
		
        fprintf(flOutput,"E8583TAB_ID = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].e8583tab_id);
	
	fprintf(flOutput,"BITMAP_LEN = %d\n",
		smShmbuf_s.psSwt_sys_queue[i].bitmap_len);
		
	memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].bitmap_unpk,alTmpexp);
	fprintf(flOutput,"BITMAP_UNPK = %s\n",alTmpexp);
	
	memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].bitmap_pack,alTmpexp);
	fprintf(flOutput,"BITMAP_PACK = %s\n",alTmpexp);
	
	memset(alTmpexp,0x00,sizeof(alTmpexp));
        ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_queue[i].mac_express,alTmpexp);
	fprintf(flOutput,"MAC_EXPRESS = %s\n",alTmpexp);

/* === begin of added by fzj at 2002.02.28 === */
	fprintf(flOutput,"TRANBEGIN_NUM = %ld\n",smShmbuf_s.psSwt_sys_queue[i].tranbegin_num);
	fprintf(flOutput,"TRANBEGIN_MAX = %ld\n",smShmbuf_s.psSwt_sys_queue[i].tranbegin_max);
	fprintf(flOutput,"TRANING_NUM = %ld\n",smShmbuf_s.psSwt_sys_queue[i].traning_num);
	fprintf(flOutput,"TRANING_MAX = %ld\n",smShmbuf_s.psSwt_sys_queue[i].traning_max);
/* === end of added by fzj at 2002.02.28 === */
	
        if(stdout == afStream)
        {
          swShmgetcmd(":");
          if(strcmp(agCommand,"U") == 0)
          {
            i -= 2;
            if(i < -1)
              i = -1;
           }
           else
           if(strcmp(agCommand,"Q") == 0)
             return;
         }
  }
  fprintf(flOutput,"\n\n");
}

/* === begin of added by fzj at 2002.02.28 === */
/*******************************************************************/
void swShmprintmatchport(FILE * afStream)
{
  int i,j;
  FILE * flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"---------------------------swt_sys_matchport----------------------------\n");

  fprintf(flOutput,"swt_sys_matchport表记录数:%d\n",
  		psmShmidx_s->sIdx_sys_matchport.iCount);

  fprintf(flOutput, "qid       trancode       port\n");

  j = 0;
  
  for(i=0;i<psmShmidx_s->sIdx_sys_matchport.iCount;i++)
  {
    fprintf(flOutput,"%-10d%-15s%d\n",
		smShmbuf_s.psSwt_sys_matchport[i].qid,    
    		smShmbuf_s.psSwt_sys_matchport[i].trancode,
    		smShmbuf_s.psSwt_sys_matchport[i].port
    		);
    j++;
    if(j>20)
    {
      j =0;
      if(afStream == stdout)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
        else if(strcmp(agCommand,"D") == 0)
        {
          i += 21;
          if(i >= psmShmidx_s->sIdx_sys_matchport.iCount)
            i = psmShmidx_s->sIdx_sys_matchport.iCount - 1;
        }
        else if(strcmp(agCommand,"Q") == 0)
          return;  
      }
  fprintf(flOutput, "qid\ttrancode\tport\n");
    }
  }
  fprintf(flOutput,"\n\n");
}
/* === end of added by fzj at 2002.02.28 === */

/*******************************************************************/
void swShmprinttask(FILE* afStream)
{
  int i,ilRc;
  char alTmpexp[2*iFLDVALUELEN+1];   /* add by wangpan 2001/11/30 */
  FILE * flOutput;

   flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"-----------------swt_sys_task---------------\n");
  fprintf(flOutput,"swt_sys_task表的记录数:%d\n",
		psmShmidx_s->sIdx_sys_task.iCount);

  for(i = 0;i < psmShmidx_s->sIdx_sys_task.iCount;i++)
  {
    fprintf(flOutput,"第%d条记录:\n",i+1);
    
    /* add by wangpan 2001/11/30                      */
    memset(alTmpexp,0x00,sizeof(alTmpexp));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_task[i].task_timer,alTmpexp);
    /* if(ilRc!=0) return(ilRc);                      */
    /* end add by wangpan 2001/11/30                  */
    
    fprintf(flOutput,
		"TASKNAME   = %s\n"\
                "TASKDESC   = %s\n"\
		"TASKFILE   = %s\n"\
		"QID        = %d\n"\
		"STARTID    = %d\n"\
		"STARTWAIT  = %d\n"\
		"STOPID     = %d\n"\
/*		"pid         = %d\n"\           */
/*		"task_status = %s\n"\           */
		"RESTARTFLAG= %s\n"\
		"KILLID     = %d\n"\
		"TASKATTR   = %s\n"\
/*		"start_time  = %d\n"\           */
/*		"restart_num = %d\n"\           */
		"RESTART_MAX = %d\n"\
		"TASKPRIOR  = %d\n"\
		"TASKFLAG   = %d\n"\
		"TASKUSE    = %s\n"\
		"TASKTIME   = %s\n\n\n",
		smShmbuf_s.psSwt_sys_task[i].task_name,
                smShmbuf_s.psSwt_sys_task[i].task_desc,
		smShmbuf_s.psSwt_sys_task[i].task_file,
		smShmbuf_s.psSwt_sys_task[i].q_id,
		smShmbuf_s.psSwt_sys_task[i].start_id,
		smShmbuf_s.psSwt_sys_task[i].start_wait,
		smShmbuf_s.psSwt_sys_task[i].stop_id,
/*		smShmbuf_s.psSwt_sys_task[i].pid,                */    
/*		smShmbuf_s.psSwt_sys_task[i].task_status,        */
		smShmbuf_s.psSwt_sys_task[i].restart_flag,
		smShmbuf_s.psSwt_sys_task[i].kill_id,
		smShmbuf_s.psSwt_sys_task[i].task_attr,
/*		smShmbuf_s.psSwt_sys_task[i].start_time,         */
/*		smShmbuf_s.psSwt_sys_task[i].restart_num,        */
		smShmbuf_s.psSwt_sys_task[i].restart_max,
		smShmbuf_s.psSwt_sys_task[i].task_priority,
		smShmbuf_s.psSwt_sys_task[i].task_flag,
		smShmbuf_s.psSwt_sys_task[i].task_use,
		alTmpexp /* add by wangpan 2001/11/30 */
		);
    if(stdout == afStream)
    {
      swShmgetcmd(":");
      if(strcmp(agCommand,"U") == 0)
      {
        i -= 2;
        if(i < -1)
          i = -1;
      }
      else
      if(strcmp(agCommand,"Q") == 0)
        return;
     }
  }
  
  fprintf(flOutput,"\n\n");
}

/*******************************************************************/
void swShmprintconfig(FILE* afStream)
{
  int i,j;
  FILE * flOutput;

    flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,
"----------------swt_sys_config--------------\n");
  fprintf(flOutput,"swt_sys_config表的记录数:%d\n",
		psmShmidx_s->sIdx_sys_config.iCount);

  j = 0;
  for(i = 0;i < psmShmidx_s->sIdx_sys_config.iCount;i++)
  {
    fprintf(flOutput,
                      "log_inter = %d\n"\
                      "saf_inter = %d\n"\
                      "iShmkey = %d\n"\
                      "iMaxtranlog = %d\n"\
                      "iMaxproclog = %d\n"\
                      "iMaxsaflog = %d\n"\
                      "iShmwaittime = %d\n"\
                      "iMaxtrannum = %ld\n"\
                      "sysmonaddr = %s\n"\
                      "sysmonport = %d\n"\
                      "msg_num = %ld\n"\
                      "echotest_num = %ld\n"\
                      "echotest_inter = %d\n"\
                      "log_clear_count = %d\n"\
                      "trace_term = %s\n"\
                      "ftpmode = %s\n"\
                      "ftpovertime = %d\n"\
                      "qpacksize = %d\n"\
                      "qovertime = %d\n"\
                      "qresendnum = %d\n\n",
                      smShmbuf_s.psSwt_sys_config[i].log_inter,
                      smShmbuf_s.psSwt_sys_config[i].saf_inter,
                      smShmbuf_s.psSwt_sys_config[i].iShmkey,
                      smShmbuf_s.psSwt_sys_config[i].iMaxtranlog,
                      smShmbuf_s.psSwt_sys_config[i].iMaxproclog,
                      smShmbuf_s.psSwt_sys_config[i].iMaxsaflog,
                      smShmbuf_s.psSwt_sys_config[i].iShmwaittime,
                      smShmbuf_s.psSwt_sys_config[i].iMaxtrannum,
                      smShmbuf_s.psSwt_sys_config[i].sysmonaddr,
                      smShmbuf_s.psSwt_sys_config[i].sysmonport,
                      smShmbuf_s.psSwt_sys_config[i].msg_num,
                      smShmbuf_s.psSwt_sys_config[i].echotest_num,
                      smShmbuf_s.psSwt_sys_config[i].echotest_inter,
                      smShmbuf_s.psSwt_sys_config[i].log_clear_count,
                      smShmbuf_s.psSwt_sys_config[i].trace_term,
                      smShmbuf_s.psSwt_sys_config[i].ftpmode,
                      smShmbuf_s.psSwt_sys_config[i].ftpovertime,
                      smShmbuf_s.psSwt_sys_config[i].qpacksize,
                      smShmbuf_s.psSwt_sys_config[i].qovertime,
                      smShmbuf_s.psSwt_sys_config[i].qresendnum);
    j++;
    if(j > 20)
    {
      j = 0;
      if(stdout == afStream)
      {
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
          i -= 42;
          if(i < -1)
            i = -1;
        }
        else
        if(strcmp(agCommand,"Q") == 0)
          return;
      } 
    }
  }
  fprintf(flOutput,"\n\n");
}
/*******************************************************************/
void swShmprintroute_d(FILE* afStream)
{
  int i,ilRc;
  char alTmpexp[2*iFLDVALUELEN+1];   /* add by wangpan 2001/11/30 */
  FILE * flOutput;

    flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,"----------swt_sys_route_d----------\n");
  fprintf(flOutput,"swt_sys_route_d表记录数:%d\n",
		psmShmidx_s->sIdx_sys_route_d.iCount);

  for(i = 0;i < psmShmidx_s->sIdx_sys_route_d.iCount;i++)
  {
    fprintf(flOutput,"第%d条记录:\n",i+1);
    
    /* add by wangpan 2001/11/30                      */
    memset(alTmpexp,0x00,sizeof(alTmpexp));
    ilRc = _swExpT2N(smShmbuf_s.psSwt_sys_route_d[i].route_cond,alTmpexp);
    /* if(ilRc!=0) return(ilRc);                      */
    /* end add by wangpan 2001/11/30                  */
    
    /*modified by baiqj20150428 PSBC_V1.0 rev_fmtgrp %d -> %ld*/
	fprintf(flOutput,
		"route_grp      = %d\n"\
		"route_id       = %d\n"\
		"route_cond     = %s\n"\
		"q_target       = %s\n"\
		"next_id        = %d\n"\
		"oper_flag      = %s\n"\
		"saf_flag       = %s\n"\
		"end_flag       = %s\n"\
        /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
		/* "fmt_group      = %d\n"\ */
		"fmt_group      = %ld\n"\
		"rev_mode       = %d\n"\
		"rev_fmtgrp     = %ld\n"\
		"fml_name       = %s\n\n\n",
		smShmbuf_s.psSwt_sys_route_d[i].route_grp,
		smShmbuf_s.psSwt_sys_route_d[i].route_id,
		alTmpexp,  
		smShmbuf_s.psSwt_sys_route_d[i].q_target,
		smShmbuf_s.psSwt_sys_route_d[i].next_id,
		smShmbuf_s.psSwt_sys_route_d[i].oper_flag,
		smShmbuf_s.psSwt_sys_route_d[i].saf_flag,
		smShmbuf_s.psSwt_sys_route_d[i].end_flag,
		smShmbuf_s.psSwt_sys_route_d[i].fmt_group,
		smShmbuf_s.psSwt_sys_route_d[i].rev_mode,
		smShmbuf_s.psSwt_sys_route_d[i].rev_fmtgrp,
		smShmbuf_s.psSwt_sys_route_d[i].fml_name);
    if(afStream == stdout)
    {
      swShmgetcmd(":");
      if(strcmp(agCommand,"U") == 0)
      {
        i -= 2;
        if(i < -1)
          i = -1;
      }
      else
      if(strcmp(agCommand,"Q") == 0)
        return;
     }
  }
  fprintf(flOutput,"\n\n");
}

/* add by gengling at 2015.05.21 begin PSBC_V1.0 */
/*******************************************************************/
void swShmprinttran(FILE* afStream)
{
  int i,j;
  FILE * flOutput;

  flOutput = afStream;

  fprintf(flOutput,"\n\n");
  fprintf(flOutput,"----------swt_sys_tran----------\n");
#ifndef HASH
  fprintf(flOutput,"swt_sys_tran表记录数:%d\n",
		psmShmidx_s->sIdx_sys_tran.iCount);
#else
  fprintf(flOutput,"swt_sys_tran表记录数:%d\n",
		psmShmidx_s->sIdx_sys_tran_hash.iCount);
#endif

  fprintf(flOutput,
          "sys_id tran_code tran_name                                             \
          priority status tranning_max debug_level tranning_num\n");
  j = 0;
  long dataoffset;

#ifndef HASH
  for(i = 0;i < psmShmidx_s->sIdx_sys_tran.iCount;i++)
#else
  for(i = 0;i < psmShmidx_s->sIdx_sys_tran_hash.iCount;i++)
#endif
  {
#ifndef HASH
	fprintf(flOutput, "%-5d%-12s%-64s%-10d%-6s%-13ld%-12d%ld\n",
		smShmbuf_s.psSwt_sys_tran[i].sys_id,
		smShmbuf_s.psSwt_sys_tran[i].tran_code,
		smShmbuf_s.psSwt_sys_tran[i].tran_name,
		smShmbuf_s.psSwt_sys_tran[i].priority,
		smShmbuf_s.psSwt_sys_tran[i].status,
		smShmbuf_s.psSwt_sys_tran[i].tranning_max,
		smShmbuf_s.psSwt_sys_tran[i].debug_level,
		smShmbuf_s.psSwt_sys_tran[i].tranning_num);
#else
    dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
	fprintf(flOutput, "%-5d%-12s%-64s%-10d%-6s%-13ld%-12d%ld\n",
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].sys_id,
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].tran_code,
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].tran_name,
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].priority,
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].status,
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].tranning_max,
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].debug_level,
		((struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset))[i].tranning_num);
#endif

    j++;
    if(j > 20)
    {
      j =0;
      if(stdout == afStream)
      {
          swShmgetcmd(":");
          if(strcmp(agCommand,"U") == 0)
          {
              i -= 42;
              if(i < -1)
                  i = -1;
          }
          else if(strcmp(agCommand,"Q") == 0)
              return;
          else if(strcmp(agCommand,"U") == 0)
          {
              i += 21;
#ifndef HASH
              if(i >= psmShmidx_s->sIdx_sys_tran.iCount)
                  i = psmShmidx_s->sIdx_sys_tran.iCount - 1;
#else
              if(i >= psmShmidx_s->sIdx_sys_tran_hash.iCount)
                  i = psmShmidx_s->sIdx_sys_tran_hash.iCount - 1;
#endif
          }
      }

      fprintf(flOutput,
          "sys_id tran_code tran_name                                             \
          priority status tranning_max debug_level tranning_num\n");
    }
  }
  fprintf(flOutput,"\n\n");
}
/* add by gengling at 2015.05.21 end PSBC_V1.0 */

/*******************************************************************/
void swShmgetcmd(char * aaPrompt)
{ 
  int i;
  char alBuffer[250];
  char alTmp[20];

  memset(alBuffer,0,sizeof(alBuffer));
  memset(agCommand,0,sizeof(agCommand));
  memset(agParam,0,sizeof(agParam));
  memset(agParam1,0,sizeof(agParam1));

  /* printf(aaPrompt); */
  strcpy(alTmp,aaPrompt);
  printf(alTmp);

  fgets(alBuffer,150,stdin);

  for(i=0;i<strlen(alBuffer);i++) 
    alBuffer[i]=toupper(alBuffer[i]);
 
  sscanf(alBuffer,"%s %s %s",agCommand,agParam,agParam1);
  if(agCommand[0] == '\0')
    return;
  if((strcmp(agCommand,"QUIT")==0)||(strcmp(agCommand,"Q")==0))
    return;
  
  if((strcmp(agCommand,"r")==0)||(strcmp(agCommand,"R")==0))
  {
    strcpy(agCommand,agCommand_buf);
    strcpy(agParam,agParam_buf);
    strcpy(agParam1,agParam1_buf);
  }
  else
  {
    strcpy(agCommand_buf,agCommand);
    strcpy(agParam_buf,agParam);
    strcpy(agParam1_buf,agParam1);
  }   
}

/*******************************************************************/
void swShmprinthelp()
{
  printf("pt-----------切换输出介质.\n");
  printf("   参数:\n");
  printf("     s(screen)------屏幕\n");
  printf("     f(file)--------文件\n");
  printf("print--------打印数据。\n");
  printf("   参数:\n");
  printf("     all----------所有\n");
  printf("     tablename----只打印指定的表\n");
  printf("     sshm---------静态共享内存状态\n");
  printf("     dshm---------动态共享内存状态\n");
  printf("help----------打印本菜单.\n");
}
                             
void swShmprintstatus_s(FILE* afStream)
{
  fprintf(afStream,"**********静态表共享内存**********\n");
  fprintf(afStream,"**********   目录段     **********\n");

  fprintf(afStream,"swt_sys_imf表:     偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_imf.lOffset, psmShmidx_s->sIdx_sys_imf.iCount);
    psmShmidx_s->sIdx_sys_imf.lOffset, psmShmidx_s->sIdx_sys_imf.iCount);

  fprintf(afStream,"swt_sys_queue表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_queue.lOffset,psmShmidx_s->sIdx_sys_queue.iCount);
    psmShmidx_s->sIdx_sys_queue.lOffset,psmShmidx_s->sIdx_sys_queue.iCount);

  fprintf(afStream,"swt_sys_task表:    偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_task.lOffset, psmShmidx_s->sIdx_sys_task.iCount);
    psmShmidx_s->sIdx_sys_task.lOffset, psmShmidx_s->sIdx_sys_task.iCount);

  fprintf(afStream,"swt_sys_8583表:    偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_8583.lOffset, psmShmidx_s->sIdx_sys_8583.iCount);
    psmShmidx_s->sIdx_sys_8583.lOffset, psmShmidx_s->sIdx_sys_8583.iCount);

  fprintf(afStream,"swt_sys_8583E表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_8583E.lOffset, psmShmidx_s->sIdx_sys_8583E.iCount);
    psmShmidx_s->sIdx_sys_8583E.lOffset, psmShmidx_s->sIdx_sys_8583E.iCount);

  fprintf(afStream,"swt_sys_code表:    偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_code.lOffset, psmShmidx_s->sIdx_sys_code.iCount);
    psmShmidx_s->sIdx_sys_code.lOffset, psmShmidx_s->sIdx_sys_code.iCount);

  fprintf(afStream,"swt_sys_config表:  偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_config.lOffset, psmShmidx_s->sIdx_sys_config.iCount);
    psmShmidx_s->sIdx_sys_config.lOffset, psmShmidx_s->sIdx_sys_config.iCount);

  fprintf(afStream,"swt_sys_other表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_other.lOffset, psmShmidx_s->sIdx_sys_other.iCount);
    psmShmidx_s->sIdx_sys_other.lOffset, psmShmidx_s->sIdx_sys_other.iCount);

#ifndef HASH
  fprintf(afStream,"swt_sys_fmt_m表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    psmShmidx_s->sIdx_sys_fmt_m.lOffset, psmShmidx_s->sIdx_sys_fmt_m.iCount);

  fprintf(afStream,"swt_sys_fmt_g表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_fmt_g.lOffset, psmShmidx_s->sIdx_sys_fmt_g.iCount);
    psmShmidx_s->sIdx_sys_fmt_g.lOffset, psmShmidx_s->sIdx_sys_fmt_g.iCount);

  fprintf(afStream,"swt_sys_fmt_d表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    //psmShmidx_s->sIdx_sys_fmt_d.lOffset, psmShmidx_s->sIdx_sys_fmt_d.iCount);
    psmShmidx_s->sIdx_sys_fmt_d.lOffset, psmShmidx_s->sIdx_sys_fmt_d.iCount);

  fprintf(afStream,"swt_sys_route_m表: 偏移量 = [%-8ld]    记录数 = [%-4d]\n",
 //psmShmidx_s->sIdx_sys_route_m.lOffset, psmShmidx_s->sIdx_sys_route_m.iCount);
 psmShmidx_s->sIdx_sys_route_m.lOffset, psmShmidx_s->sIdx_sys_route_m.iCount);

  fprintf(afStream,"swt_sys_route_g表: 偏移量 = [%-8ld]   记录数 = [%-4d]\n",
 psmShmidx_s->sIdx_sys_route_g.lOffset, psmShmidx_s->sIdx_sys_route_g.iCount);

  fprintf(afStream,"swt_sys_route_d表: 偏移量 = [%-8ld]   记录数 = [%-4d]\n",
 psmShmidx_s->sIdx_sys_route_d.lOffset, psmShmidx_s->sIdx_sys_route_d.iCount);
  
  /* add by gengling at 2015.04.17 two lines PSBC_V1.0 */
  fprintf(afStream,"swt_sys_tran表:    偏移量 = [%-8ld]   记录数 = [%-4d]\n",
  psmShmidx_s->sIdx_sys_tran.lOffset, psmShmidx_s->sIdx_sys_tran.iCount);
#else
  fprintf(afStream,"swt_sys_fmt_g表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    psmShmidx_s->sIdx_sys_fmt_g.lOffset, psmShmidx_s->sIdx_sys_fmt_g.iCount);

  fprintf(afStream,"swt_sys_fmt_d表:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    psmShmidx_s->sIdx_sys_fmt_d.lOffset, psmShmidx_s->sIdx_sys_fmt_d.iCount);

  fprintf(afStream,"swt_sys_route_g表: 偏移量 = [%-8ld]    记录数 = [%-4d]\n",
 //psmShmidx_s->sIdx_sys_route_g.lOffset, psmShmidx_s->sIdx_sys_route_g.iCount);
 psmShmidx_s->sIdx_sys_route_g.lOffset, psmShmidx_s->sIdx_sys_route_g.iCount);

  fprintf(afStream,"swt_sys_route_d表: 偏移量 = [%-8ld]    记录数 = [%-4d]\n",
 //psmShmidx_s->sIdx_sys_route_d.lOffset, psmShmidx_s->sIdx_sys_route_d.iCount);
 psmShmidx_s->sIdx_sys_route_d.lOffset, psmShmidx_s->sIdx_sys_route_d.iCount);
  
  fprintf(afStream,"swt_sys_tran hash:    偏移量 = [%-8ld]    记录数 = [%-4d]\n",
  psmShmidx_s->sIdx_sys_tran_hash.lOffset, psmShmidx_s->sIdx_sys_tran_hash.iCount);

  fprintf(afStream,"swt_sys_fmt_m hash:   偏移量 = [%-8ld]    记录数 = [%-4d]\n",
    psmShmidx_s->sIdx_sys_fmt_m_hash.lOffset, psmShmidx_s->sIdx_sys_fmt_m_hash.iCount);

  fprintf(afStream,"swt_sys_fmt_g hash:   偏移量 = [%-8ld]   记录数 = [%-4d]\n",
    psmShmidx_s->sIdx_sys_fmt_g_hash.lOffset, psmShmidx_s->sIdx_sys_fmt_g_hash.iCount);

  fprintf(afStream,"swt_sys_route_m hash: 偏移量 = [%-8ld]   记录数 = [%-4d]\n",
  psmShmidx_s->sIdx_sys_route_m_hash.lOffset, psmShmidx_s->sIdx_sys_route_m_hash.iCount);
#endif
}

/*******************************************************************/
void swShmprintstatus_d(FILE* afStream)
{
  char alTmp[101];
  int ilUse,ilUnuse;
  int i;

  fprintf(afStream,"**********动态表共享内存**********\n");
  fprintf(afStream,"**********   目录段     **********\n");
  fprintf(afStream,"iShmid = [%d]\n",psmShmidx_d->iShmid);
  fprintf(afStream,"iSemid = [%d]\n",psmShmidx_d->iSemid);
  memset(alTmp,0x00,sizeof(alTmp));
  memcpy(alTmp,&(psmShmidx_d->aMemo[9]),14);
  fprintf(afStream,"创建时间 = [%s]\n",alTmp);
  memset(alTmp,0x00,sizeof(alTmp));
  memcpy(alTmp,&(psmShmidx_d->aMemo[23]),14);
  fprintf(afStream,"上次刷新时间 = [%s]\n",alTmp);
  memset(alTmp,0x00,sizeof(alTmp));
  memcpy(alTmp,&(psmShmidx_d->aMemo[5]),4);
  fprintf(afStream,"刷新次数 = [%d]\n",atoi(alTmp));
  fprintf(afStream,"\n\n");

  fprintf(afStream,"sIdx_tran_log状态:\n");
  for(i = 0;i < SHM_TRANLOG_BUCKET_NUM;i ++)
  {  
  fprintf(afStream,"  偏移量:      %ld\n",psmShmidx_d->sIdx_tran_log[i].lOffset);
  fprintf(afStream,"  首链接:      %d\n",psmShmidx_d->sIdx_tran_log[i].iSlink);
  fprintf(afStream,"  尾链接:      %d\n",psmShmidx_d->sIdx_tran_log[i].iElink);
  fprintf(afStream,"  空链接:      %d\n",psmShmidx_d->sIdx_tran_log[i].iFlink);
  printf("22222222\n");
  ilUse = swShmgetusedcount_tran_log(i);
  printf("3333333\n");
  if(ilUse == -1)
    fprintf(afStream,"tran log 链表出错!!!\n");
  else
  {
    ilUnuse = iMaxtranlog -ilUse;
    fprintf(afStream,"    使用:      %d\n",ilUse);
    fprintf(afStream,"  未使用:      %d\n",ilUnuse);
  }
  }
  fprintf(afStream,"sIdx_proc_log状态:\n");
  fprintf(afStream,"  偏移量:      %ld\n",psmShmidx_d->sIdx_proc_log.lOffset);
  fprintf(afStream,"  首链接:      %d\n",psmShmidx_d->sIdx_proc_log.iSlink);
  fprintf(afStream,"  尾链接:      %d\n",psmShmidx_d->sIdx_proc_log.iElink);
  fprintf(afStream,"  空链接:      %d\n",psmShmidx_d->sIdx_proc_log.iFlink);
  ilUse = swShmgetusedcount_proc_log();
  if(ilUse == -1)
    fprintf(afStream,"proc log链表出错!!!\n");
  else
  {
    ilUnuse = iMaxproclog -ilUse;
    fprintf(afStream,"    使用:      %d\n",ilUse);
    fprintf(afStream,"  未使用:      %d\n",ilUnuse);
  }

  fprintf(afStream,"sIdx_rev_saf状态:\n");
  fprintf(afStream,"  偏移量:      %ld\n",psmShmidx_d->sIdx_rev_saf.lOffset);
  fprintf(afStream,"  首链接:      %d\n",psmShmidx_d->sIdx_rev_saf.iSlink);
  fprintf(afStream,"  尾链接:      %d\n",psmShmidx_d->sIdx_rev_saf.iElink);
  fprintf(afStream,"  空链接:      %d\n",psmShmidx_d->sIdx_rev_saf.iFlink);
  ilUse = swShmgetusedcount_rev_saf();
  if(ilUse == -1)
    fprintf(afStream,"rev saf链表出错!!!\n");
  else
  {
    ilUnuse = iMaxsaflog -ilUse;
    fprintf(afStream,"    使用:      %d\n",ilUse);
    fprintf(afStream,"  未使用:      %d\n",ilUnuse);
  }
  fprintf(afStream,"RECINSHM[tranlog] = %ld\n",psmShmidx_d->iRecinshm[0]);
  fprintf(afStream,"RECINSHM[proclog] = %ld\n",psmShmidx_d->iRecinshm[1]);
  fprintf(afStream,"RECINSHM[saflog] = %ld\n",psmShmidx_d->iRecinshm[2]);
  fprintf(afStream,"RECINFILE[tranlog] = %ld\n",psmShmidx_d->iRecinfile[0]);
  fprintf(afStream,"RECINFILE[proclog] = %ld\n",psmShmidx_d->iRecinfile[1]);
  fprintf(afStream,"RECINFILE[saflog] = %ld\n",psmShmidx_d->iRecinfile[2]);
}

/*******************************************************************/
void swShmprinttran_log(FILE *afStream)
{
  int i;
  struct shm_tran_log * pslShm_tran_log[SHM_TRANLOG_BUCKET_NUM];

  fprintf(afStream,"\n\n");
  fprintf(afStream,"----------------swt_tran_log-------------\n");
  for(i = 0;i < SHM_TRANLOG_BUCKET_NUM;i ++)
  {
  if(!psmShmidx_d->sIdx_tran_log[i].iSlink)return;
  pslShm_tran_log[i] = sgShmbuf_d.psShm_tran_log[i]
			 + psmShmidx_d->sIdx_tran_log[i].iSlink - 1;
  while(pslShm_tran_log[i]->sLinkinfo.cTag == 1)
  {
    fprintf(afStream,
		"tran_id          = %ld\n"\
		"tran_begin       = %ld\n"\
		"tran_status      = %hd\n"\
		"tran_end         = %ld\n"\
		"tran_overtime    = %ld\n"\
		"q_tran_begin     = %hd\n"\
		"tc_tran_begin    = %s\n"\
		"q_target         = %hd\n"\
		"resq_file        = %s\n"\
		"resq_offset      = %ld\n"\
		"resq_len         = %hd\n"\
		"resp_file        = %s\n"\
		"resp_offset      = %ld\n"\
		"resp_len         = %hd\n"\
                "rev_key          = %s\n"\
                "msghdkey         = %s\n"\
                "msghead          = %s\n"\
                "xastatus         = %s\n\n\n\n",\
		pslShm_tran_log[i]->sSwt_tran_log.tran_id,
		pslShm_tran_log[i]->sSwt_tran_log.tran_begin,
		pslShm_tran_log[i]->sSwt_tran_log.tran_status,
		pslShm_tran_log[i]->sSwt_tran_log.tran_end,
		pslShm_tran_log[i]->sSwt_tran_log.tran_overtime,
		pslShm_tran_log[i]->sSwt_tran_log.q_tran_begin,
		pslShm_tran_log[i]->sSwt_tran_log.tc_tran_begin,
		pslShm_tran_log[i]->sSwt_tran_log.q_target,
		pslShm_tran_log[i]->sSwt_tran_log.resq_file,
		pslShm_tran_log[i]->sSwt_tran_log.resq_offset,
		pslShm_tran_log[i]->sSwt_tran_log.resq_len,
		pslShm_tran_log[i]->sSwt_tran_log.resp_file,
		pslShm_tran_log[i]->sSwt_tran_log.resp_offset,
		pslShm_tran_log[i]->sSwt_tran_log.resp_len,
		pslShm_tran_log[i]->sSwt_tran_log.rev_key,
		pslShm_tran_log[i]->sSwt_tran_log.msghdkey,
		pslShm_tran_log[i]->sSwt_tran_log.msghead,
		pslShm_tran_log[i]->sSwt_tran_log.xastatus);
    if(pslShm_tran_log[i]->sLinkinfo.iRlink == 0)
      break;
    if(stdout == afStream)
    {
      swShmgetcmd(":");
      if(strcmp(agCommand,"U") == 0)
      {
        if(pslShm_tran_log[i]->sLinkinfo.iLlink != 0)
          pslShm_tran_log[i] = sgShmbuf_d.psShm_tran_log[i] + 
                 pslShm_tran_log[i]->sLinkinfo.iLlink - 1;
         continue;
      }
      else
      if(strcmp(agCommand,"Q") == 0)
        return;
    }
    if(pslShm_tran_log[i]->sLinkinfo.iRlink == 0)
      break;
    pslShm_tran_log[i] = sgShmbuf_d.psShm_tran_log[i] +
                pslShm_tran_log[i]->sLinkinfo.iRlink - 1;

  } 
  fprintf(afStream,"\n\n");
  }
}

/*******************************************************************/
void swShmprintproc_log(FILE *afStream)
{
  struct shm_proc_log * pslShm_proc_log;

  pslShm_proc_log = sgShmbuf_d.psShm_proc_log +
		psmShmidx_d->sIdx_proc_log.iSlink - 1;
  
  fprintf(afStream,"\n\n");
  fprintf(afStream,"-------------swt_proc_log--------------\n");
  if(!psmShmidx_d->sIdx_proc_log.iSlink)return;
  
  while(pslShm_proc_log->sLinkinfo.cTag == 1)
  {
    /*modified by baiqj20150428 PSBC_V1.0 rev_fmtgrp %hd -> %ld*/
    fprintf(afStream,
		"tran_id         = %ld\n"\
		"proc_step       = %hd\n"\
		"proc_begin      = %ld\n"\
		"q_target        = %hd\n"\
		"proc_status     = %hd\n"\
		"rev_mode        = %hd\n"\
		"rev_fmtgrp      = %ld\n"\
		"org_file        = %s\n"\
		"org_offset      = %ld\n"\
		"org_len         = %hd\n\n\n",
                pslShm_proc_log->sSwt_proc_log.tran_id,
		pslShm_proc_log->sSwt_proc_log.proc_step,
		pslShm_proc_log->sSwt_proc_log.proc_begin,
		pslShm_proc_log->sSwt_proc_log.q_target,
		pslShm_proc_log->sSwt_proc_log.proc_status,
		pslShm_proc_log->sSwt_proc_log.rev_mode, 
		pslShm_proc_log->sSwt_proc_log.rev_fmtgrp,
		pslShm_proc_log->sSwt_proc_log.org_file,
		pslShm_proc_log->sSwt_proc_log.org_offset,
		pslShm_proc_log->sSwt_proc_log.org_len);
    if(pslShm_proc_log->sLinkinfo.iRlink == 0)
      break;
    if(stdout == afStream)
    {
      swShmgetcmd(":");
      if(strcmp(agCommand,"U") == 0)
      {
        if(pslShm_proc_log->sLinkinfo.iLlink != 0)
          pslShm_proc_log = sgShmbuf_d.psShm_proc_log + 
                pslShm_proc_log->sLinkinfo.iLlink - 1;
        continue;
      }
      else
      if(strcmp(agCommand,"Q") == 0)
        return;
    }
    if(pslShm_proc_log->sLinkinfo.iRlink == 0)
      break;
    pslShm_proc_log = sgShmbuf_d.psShm_proc_log + 
		pslShm_proc_log->sLinkinfo.iRlink - 1;
  }
  fprintf(afStream,"\n\n");
}

/*******************************************************************/
void swShmprintrev_saf(FILE *afStream)
{
  struct shm_rev_saf * pslShm_rev_saf;

  pslShm_rev_saf = sgShmbuf_d.psShm_rev_saf + 
		psmShmidx_d->sIdx_rev_saf.iSlink - 1;

  fprintf(afStream,"\n\n");
  fprintf(afStream,"------------swt_rev_saf-----------\n");
  if(!psmShmidx_d->sIdx_rev_saf.iSlink)return;

  while(pslShm_rev_saf->sLinkinfo.cTag == 1)
  {
    fprintf(afStream,
		"saf_id           = %ld\n"\
		"tran_id          = %ld\n"\
		"proc_step        = %hd\n"\
		"saf_begin        = %ld\n"\
		"saf_overtime     = %ld\n"\
		"rev_overtime     = %hd\n"\
		"saf_num          = %hd\n"\
		"saf_status       = %s\n"\
		"saf_flag         = %s\n"\
                "saf_file         = %s\n"\
		"saf_offset       = %ld\n"\
		"saf_len          = %hd\n\n\n",
		pslShm_rev_saf->sSwt_rev_saf.saf_id,
		pslShm_rev_saf->sSwt_rev_saf.tran_id,
		pslShm_rev_saf->sSwt_rev_saf.proc_step,
		pslShm_rev_saf->sSwt_rev_saf.saf_begin,
		pslShm_rev_saf->sSwt_rev_saf.saf_overtime,
		pslShm_rev_saf->sSwt_rev_saf.rev_overtime,
		pslShm_rev_saf->sSwt_rev_saf.saf_num,
		pslShm_rev_saf->sSwt_rev_saf.saf_status,
		pslShm_rev_saf->sSwt_rev_saf.saf_flag,
		pslShm_rev_saf->sSwt_rev_saf.saf_file,
		pslShm_rev_saf->sSwt_rev_saf.saf_offset,
		pslShm_rev_saf->sSwt_rev_saf.saf_len);
    if(pslShm_rev_saf->sLinkinfo.iRlink == 0)
      break;
    if(afStream == stdout)
    {
      swShmgetcmd(":");
      if(strcmp(agCommand,"U") == 0)
      {
        if(pslShm_rev_saf->sLinkinfo.iLlink != 0)
          pslShm_rev_saf = sgShmbuf_d.psShm_rev_saf +
                pslShm_rev_saf->sLinkinfo.iLlink - 1;
        continue;
      }
      else
      if(strcmp(agCommand,"Q") == 0)
        return;
    }
    if(pslShm_rev_saf->sLinkinfo.iRlink == 0)
      break;
    pslShm_rev_saf = sgShmbuf_d.psShm_rev_saf +
		pslShm_rev_saf->sLinkinfo.iRlink - 1;
  }
  fprintf(afStream,"\n\n");
}

/*******************************************************************/
void swShmprintsem(FILE *afStream)
{
  struct semid_ds slSemid_ds_buf;
  union semun ulSem_un;
  ushort ilSemval[iSEMNUM];
  int ilRc;
  
  ulSem_un.buf = &slSemid_ds_buf;
  ulSem_un.array = ilSemval;
  ilRc = semctl(psmShmidx_d->iSemid,0,GETALL,ulSem_un);
  if(ilRc != 0)
  {
    printf("读信号灯出错!\n");
    return;
  }
  fprintf(afStream,"\n\n");
  fprintf(afStream,"信号灯状态:\n");
  fprintf(afStream,"  平台流水号   %d\n"\
                   "  SAF流水号    %d\n"\
                   "  端口状态     %d\n"\
                   "  交易流水     %d\n"\
                   "  处理流水     %d\n"\
                   "  SAF流水      %d\n",
                   ilSemval[0],
                   ilSemval[1],
                   ilSemval[2],
                   ilSemval[3],
                   ilSemval[4],
                   ilSemval[5]);
  fprintf(afStream,"\n\n");
}

/*******************************************************************/
void swShmprintlinkinfo(FILE *afStream)
{
  int i,j;
  
  fprintf(afStream,"------------link_tran_log-----------\n");
    fprintf(afStream,
"启用标识       左链接          右链接         tran_id        q_target\n");

  j = 0;
  for(i=0;i<iMaxtranlog;i++)
  {
    fprintf(afStream,
"%8d     %8d        %8d        %8ld        %8d\n",
      sgShmbuf_d.psShm_tran_log[i]->sLinkinfo.cTag,
      sgShmbuf_d.psShm_tran_log[i]->sLinkinfo.iLlink,
      sgShmbuf_d.psShm_tran_log[i]->sLinkinfo.iRlink,
      sgShmbuf_d.psShm_tran_log[i]->sSwt_tran_log.tran_id,
      sgShmbuf_d.psShm_tran_log[i]->sSwt_tran_log.q_target);
      j++;
      if(stdout == afStream)
    {
      if(j > 20)
      {
        j = 0;
        swShmgetcmd(":");
        if(strcmp(agCommand,"U") == 0)
        {
           i -= 42;
           if(i < -1)
             i = -1;
        }
        else
        if(strcmp(agCommand,"Q") == 0)
          break;
    fprintf(afStream,
"启用标识       左链接          右链接         tran_id        q_target\n");

      }
    }
  }
}

/*******************************************************************/
void swShmprintlinkinfo_proc_log(FILE *afStream)
{
  int i,j;

  fprintf(afStream,"------------link_proc_log-----------\n");
    fprintf(afStream,
"启用标识       左链接          右链接         proc_id        proc_step\n");

  j = 0;

  for(i=0;i<iMaxproclog;i++)
  {
    fprintf(afStream,
"%8d     %8d        %8d        %8ld        %8d\n",
      sgShmbuf_d.psShm_proc_log[i].sLinkinfo.cTag,
      sgShmbuf_d.psShm_proc_log[i].sLinkinfo.iLlink,
      sgShmbuf_d.psShm_proc_log[i].sLinkinfo.iRlink,
      sgShmbuf_d.psShm_proc_log[i].sSwt_proc_log.tran_id,
      sgShmbuf_d.psShm_proc_log[i].sSwt_proc_log.proc_step);
      j++;
      if(stdout == afStream)
    {
      if(j > 20)
      {
 	j = 0;
 	swShmgetcmd(":");
 	if(strcmp(agCommand,"U") == 0)
 	{
 	  i -= 42;
 	  if(i < -1)
 	  i = -1;
	 }
	 else
 	if(strcmp(agCommand,"Q") == 0)
 	  break;
   	 fprintf(afStream,
"启用标识       左链接          右链接         proc_id        q_target\n");

      }
    }
  }
}

/*******************************************************************/
void swShmprintlinkinfo_rev_saf(FILE *afStream)
{
  int i,j;

  fprintf(afStream,"------------link_rev_saf-----------\n");
    fprintf(afStream,
"启用标识       左链接          右链接         saf_id\n");

  j = 0;

  for(i=0;i<iMaxsaflog;i++)
  {
    fprintf(afStream,
"%8d     %8d        %8d        %8ld\n",
      sgShmbuf_d.psShm_rev_saf[i].sLinkinfo.cTag,
      sgShmbuf_d.psShm_rev_saf[i].sLinkinfo.iLlink,
      sgShmbuf_d.psShm_rev_saf[i].sLinkinfo.iRlink,
      sgShmbuf_d.psShm_rev_saf[i].sSwt_rev_saf.saf_id);
      j++;
      if(stdout == afStream)
    {
      if(j > 20)
      {
 j = 0;
 swShmgetcmd(":");
 if(strcmp(agCommand,"U") == 0)
 {
 i -= 42;
 if(i < -1)
 i = -1;
 }
 else
 if(strcmp(agCommand,"Q") == 0)
 break;
 fprintf(afStream,
"启用标识       左链接          右链接         saf_id\n");

      }
    }
  }
}

/*******************************************************************/
int swShmreset_tran_log()
{
  int i,j;
  char alFile[101];
 for(j = 0;j < SHM_TRANLOG_BUCKET_NUM;j ++)
 {
  if(swShmP(j) == -1)
    return(-1);
  psmShmidx_d->sIdx_tran_log[j].iSlink = 0;
  psmShmidx_d->sIdx_tran_log[j].iElink = 0;
  psmShmidx_d->sIdx_tran_log[j].iFlink = 1;

  memset(sgShmbuf_d.psShm_tran_log[j],0,iMaxtranlog*sizeof(struct shm_tran_log));
  psmShmidx_d->iRecinshm[0] = 0;
  sprintf(alFile,"%s/log/shm/tranlog",getenv("SWITCH_DIR"));
  unlink(alFile);
  psmShmidx_d->iRecinfile[0] = 0;

  for (i=0; i<psmShmidx_s->sIdx_sys_queue.iCount; i++)
  {
    smShmbuf_s.psSwt_sys_queue[i].tranbegin_num = 0;
    smShmbuf_s.psSwt_sys_queue[i].traning_num = 0;
  }

  if(swShmV(j) == -1)
    return -1;
  }
  return 0;
}

/*******************************************************************/
int swShmreset_proc_log()
{
  char alFile[101];

  if(swShmP(iSEMNUM_PROCLOG) == -1)
    return -1;

  psmShmidx_d->sIdx_proc_log.iSlink = 0;
  psmShmidx_d->sIdx_proc_log.iElink = 0;
  psmShmidx_d->sIdx_proc_log.iFlink = 1;

  memset(sgShmbuf_d.psShm_proc_log,0,iMaxproclog*sizeof(struct shm_proc_log));
  psmShmidx_d->iRecinshm[1] = 0;
  sprintf(alFile,"%s/log/shm/proclog",getenv("SWITCH_DIR"));
  unlink(alFile);
  psmShmidx_d->iRecinfile[1] = 0;

  if(swShmV(iSEMNUM_PROCLOG) == -1)
    return -1;
  return 0;
}

/*******************************************************************/
int swShmreset_rev_saf()
{
  char alFile[101];

  if(swShmP(iSEMNUM_REVSAF) == -1)
    return -1;

  psmShmidx_d->sIdx_rev_saf.iSlink = 0;
  psmShmidx_d->sIdx_rev_saf.iElink = 0;
  psmShmidx_d->sIdx_rev_saf.iFlink = 1;

  memset(sgShmbuf_d.psShm_rev_saf,0,iMaxsaflog*sizeof(struct shm_rev_saf));
  psmShmidx_d->iRecinshm[2] = 0;
  sprintf(alFile,"%s/log/shm/saflog",getenv("SWITCH_DIR"));
  unlink(alFile);
  psmShmidx_d->iRecinfile[2] = 0;
  if(swShmV(iSEMNUM_REVSAF) == -1)
    return -1;
  return 0;
}

/*******************************************************************/
int swShmreset_all()
{
  if(swShmreset_tran_log()) return -1;
  if(swShmreset_proc_log()) return -1;
  if(swShmreset_rev_saf()) return -1;
  return 0;
}

/*******************************************************************/
int swShmprintport_info(FILE *afStream)
{
  int i,j;
  FILE *flOutput=afStream;
  short ilPort;
  short ilSt1;
  short ilSt2;
  short ilSt3;
  short ilSt4;
  short ilSt5; 
  j=0;
  
  if(agParam1[0]!=0)
  {
    int k,num[4],sum;
    for(k=0;k<4;k++) num[k]=(int)agParam1[k]-48;
    if(num[1]==-48)
      sum=num[0];
    else if(num[2]==-48)
      sum=num[0]*10+num[1];
    else
      sum=num[0]*100+num[1]*10+num[2];
    
    if(num[0]<0||num[0]>9||((num[1]<0||num[1]>9)&&num[1]!=-48)||((num[2]<0||num[2]>9)&&num[2]!=-48)||num[3]!=-48) 
    {
      printf("无效参数\n"); return 1;
    }
    
    for(k=0;k<psmShmidx_s->sIdx_sys_queue.iCount;k++)
      if(smShmbuf_s.psSwt_sys_queue[k].q_id==sum) 
      {
        ilPort= smShmbuf_s.psSwt_sys_queue[k].q_id;
        swPortget( ilPort, 1,&ilSt1);
        swPortget( ilPort, 2,&ilSt2); 
        swPortget( ilPort, 3,&ilSt3);
        swPortget( ilPort, 4,&ilSt4);
        swPortget( ilPort, 5,&ilSt5);
        fprintf(flOutput,
"端口号   通讯层  应用层  Q层  Echotest层  人工干预层\n");
        fprintf(flOutput,
"============================================================\n");
        fprintf(flOutput,
"%hd        %hd      %hd       %hd    %hd           %hd\n",ilPort,ilSt1,ilSt2,ilSt3,ilSt4,ilSt5);

/* ======== delete by qy 2001/08/22 ============
        smShmbuf_s.psSwt_sys_queue[k].sPortattrib.lTrning);
   ======== end of delete by qy 2001/08/22 ============ */
        break;
      }
    if(k==psmShmidx_s->sIdx_sys_queue.iCount)
      printf("该邮箱不存在\n");
  }
  else
  {

        fprintf(flOutput,
"端口号   通讯层  应用层  Q层  Echotest层  人工干预层\n");
        fprintf(flOutput,
"============================================================\n");
    for(i=0;i<psmShmidx_s->sIdx_sys_queue.iCount;i++)
    {
      ilPort= smShmbuf_s.psSwt_sys_queue[i].q_id;
      swPortget( ilPort, 1,&ilSt1);
      swPortget( ilPort, 2,&ilSt2);
      swPortget( ilPort, 3,&ilSt3);
      swPortget( ilPort, 4,&ilSt4);
      swPortget( ilPort, 5,&ilSt5);

      fprintf(flOutput,
"%hd        %hd      %hd       %hd    %hd           %hd\n",ilPort,ilSt1,ilSt2,ilSt3,ilSt4,ilSt5);

/* ========= delete by qy 2001/08/22 ================ 
      smShmbuf_s.psSwt_sys_queue[i].sPortattrib.lTrning);
   ========= end of delete by qy 2001/08/22 ================ */
      j++;
      if(flOutput==stdout)
      {
         if(j>20)
         {
           j=0;
           swShmgetcmd(":");
           if(strcmp(agCommand,"U")==0)
           {
             i-=42;
             if(i<-1)
             i=-1;
           }
           else
           if(strcmp(agCommand,"Q")==0)
           break;
           fprintf(flOutput,\
"端口号   通讯层  应用层  Q层  Echotest层  人工干预层\n");
         }
      }    
    }
  }
  return 0;
}
/*******************************************************************/
