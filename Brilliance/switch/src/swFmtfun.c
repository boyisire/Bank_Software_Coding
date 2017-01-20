/****************************************************************/
/* ģ����    ��FMTFUN                                         */ 
/* ģ������    ��Format���ú���                                 */
/* �� �� ��    ��V3.0.1                                         */
/* ��	 ��    ���Ż�                                           */
/* ��������    ��1999/12/7                                      */
/* ����޸����ڣ�2001/4/6                                       */
/* ģ����;    ��Format�ĺ���                                   */
/* ��ģ���а������º���������˵����                             */
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
/* �޸ļ�¼��                                                   */
/*   2000/8/24  �Ż�                                            */
/*   2001/2/22  Oracle��ֲ                                      */
/*   2001/2/27  ����DEBUG����                                   */
/*   2001/3/24  ���V3.01,����1500��                          */
/*   2001/3/29  ��ʱ��swGetFMLfromTDF�Ŀ�������޸�(1524)       */
/*   2001/4/6   INFORMIX��ֲ(1526)                              */
/****************************************************************/

/* switch���� */
#include "switch.h"

#include "swNdbstruct.h"
#include "swShm.h"

/* #define DEBUG */

/* �������� */
short igQueuemax,igIso8583max;
extern union  preunpackbuf psgUnpackbuf[iBUFFLDNUM]; /* TDFת����Ļ���� */

int igFmtexitflag;    /* add cjc 20040509 */

/*int swSetXMFprebuffer(short iFldid,char *aFldvalue,unsigned int iFldlen); del by baiqj20150413 PSBC_V1.0*/
int swSetXMFprebuffer(long iFldid,char *aFldvalue,unsigned int iFldlen); /*add by baiqj20150413 PSBC_V1.0*/
int swXMFpack(union preunpackbuf *prebuf,char *msgbody,unsigned int *msglen);

/**************************************************************
 ** ������      ��swCmpFmtTrantype
 ** ��  ��      ���жϱ��ĸ�ʽ�����뱨��ת����ʽ�Ƿ�ƥ�� 
 ** ��  ��      ��ʷ���� 
 ** ��������    : 1999/11/22
 ** ����޸����ڣ�1999/12/9
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��SUCCESS - ƥ�� FAIL - ��ƥ��
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
 ** ������      ��swSepunpack
 ** ��  ��      ���зָ�������Ԥ��� 
 ** ��  ��      ��ʷ����
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
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
        swVdebug(0,"S3194: [����/��������] swSepset()����,[%d][%d][%d] len[%d]val[%s],������=%d", ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval,ilRc);
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
        swVdebug(0,"S3196: [����/��������] swSepset()����,[%d][%d][%d] len[%d]val[%s],������=%d",ilRecid, ilGrpid, ilFldid, ilFldlen, alFldval,ilRc);
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
        swVdebug(0,"S3198: [����/��������] swSepset()����,[%d][%d][%d] len[%d]val[%s],������=%d", 
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
    swVdebug(0,"S3200: [����/��������] swSepset()����,[%d][%d][%d] len[%d]val[%s],������=%d", 
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
 ** ������      ��swGetFMLfromTDF
 ** ��  ��      �����ݸ�ʽת����Ӹ�ʽת����ϸ��õ�FMLԤ��� 
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��iFmt_group:����TDFת�����õĸ�ʽת����
 **               iFmt_offset:��ʽת�����ڹ����ڴ��е�ƫ����
 **               prebuf:����TDFת��������FMLԤ�������ŵ�buffer
 ** ����ֵ      ��SUCCESS:�ɹ�/FAIL:ʧ��
***************************************************************/
int swGetFMLfromTDF(iFmt_group, lFmt_offset, prebuf)
 /*del by zcd 20141222
  int  iFmt_group;   
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long   iFmt_group;       /* ָ����ʽת���� passed */
  /*end of add by zcd 20141222*/
  long  lFmt_offset;      /* ��ʽת�����ڹ����ڴ��е�ƫ���� passed*/
  union preunpackbuf *prebuf; /* Ԥ�ð����� */
{
  unsigned int ilRtncode,i,j,ilFldnamelen;
  short ilFldlen,ilLength;
  char  alResult[iFLDVALUELEN];
  char  alFldval[iFLDVALUELEN];
  char  alFldname[iFLDNAMELEN + 1];
  long  llFmt_d_offset;  /* TDF ���ƫ���� */
  long  llFmt_d_offset_org;
//  short ilFmt_d_count;   /* TDF ������� */
  int ilFmt_d_count;   /* TDF ������� */
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;

  psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
  j = 0;
  
  /* ilFmt_d_count  = TDF ������� */
  /* llFmt_d_offset = ��һ��TDF���ƫ���� */
  ilRtncode = swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, 
    &slSwt_sys_fmt_grp, &ilFmt_d_count, &llFmt_d_offset_org);
  
  if (ilRtncode != SUCCESS)
  {
    return (FAIL);
  }
  
  for(i=0; i < ilFmt_d_count; i++)
  {
    /* TDF ���ƫ���� */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* ȡ���� TDF �����Ϣ */
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
        /* �ظ���ʶ */
        ilRtncode = swFmlget(psgVarbuf, "_RECCOUNT", 
          &ilFldlen, alFldval);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3204: [����/��������] swFmlget()����,�м����_RECCOUNTȡֵ����,������=%d",ilRtncode);
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
            swVdebug(0,"S3206: [����/��������] _swExpress()���� fail[%s],������=%d", 
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
              swVdebug(0,"S3208: [����/��������] swFmlset()����,�м����[%s]����,������=%d",alFldname,ilRtncode);
               return(FAIL);
            }
          }
        }
      }
        break;
      case '2':
        /* �м���� */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL)
        {
          swVdebug(0,"S3210: [����/��������] _swExpress()����,�м����[%s]�������,������=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        ilRtncode = swFmlset(slSwt_sys_fmt_d.imf_name, 
          ilLength, alResult, psgVarbuf);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3212: [����/��������] swFmlset()����,�м����[%s]����,������=%d",
              slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        else
        {
          swVdebug(2,"S3214: GetFMLfromTDF():�м����[%s]swFmlset�ɹ�[%s]",
                slSwt_sys_fmt_d.imf_name, alResult);
        }
        break;
      default:
        /* ��ͨTDF��� */
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
 ** ������      ��swGetISOfromTDF
 ** ��  ��      �����ݸ�ʽת����Ӹ�ʽת����ϸ��õ�ISOԤ��� 
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��fmt_group:����TDFת�����õĸ�ʽת����
 **               prebug:����TDFת��������FMLԤ�������ŵ�buffer
 ** ����ֵ      ��SUCCESS:�ɹ�/FAIL:ʧ��
***************************************************************/
int swGetISOfromTDF(iFmt_group, lFmt_offset, prebuf)
  /*del by zcd 20141222
  int  iFmt_group;   
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long   iFmt_group;       /* ָ����ʽת���� passed */
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
  long  llFmt_d_offset;   /* TDF ���ƫ���� */
  long  llFmt_d_offset_org;   /* TDF ���ƫ���� */
  int ilFmt_d_count;    /* TDF ������� */  /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;  
  char alOrgfldlen[10],alOrgfldvalue[iFLDVALUELEN+1];
  short ilOrgfldlen = 0;
  
  psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
  j = 0;
  
  /* ilFmt_d_count  = TDF ������� */
  /* llFmt_d_offset = ��һ��TDF���ƫ���� */
  ilRtncode = swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, 
    &slSwt_sys_fmt_grp, &ilFmt_d_count, &llFmt_d_offset_org);
  
  if (ilRtncode != SUCCESS)
  {
    return (FAIL);
  }  
  
  for(i=0; i < ilFmt_d_count; i++)
  {
    /* TDF ���ƫ���� */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* ȡ���� TDF �����Ϣ */
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
        /* �ظ���ʶ */
        ilRtncode = swFmlget(psgVarbuf,"_RECCOUNT",&ilFldlen,alFldval);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3216: [����/��������] swFmlget()����,�м����_RECCOUNTȡֵ����,������=%d",ilRtncode);
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
            swVdebug(0,"S3218: [����/��������] _swExpress()���� fail[%s],������=%d",slSwt_sys_fmt_d.fld_express,ilRtncode);
             return(FAIL);
          }
          if (slSwt_sys_fmt_d.fmt_flag[0] == '1')
          {
            /* Add by szhengye:���򳤶�Ϊ0��FML�� */
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
              swVdebug(0,"S3220: [����/��������] swFmlset()����,�м����[%s]swFmlset����,������=%d",alFldname,ilRtncode);
               return(FAIL);
            }
          }
        }
      }
        /* Add by szhengye:���򳤶�Ϊ0��FML�� */
        if (slSwt_sys_fmt_d.fmt_flag[0] == '1')
        {
          if (ilPos > 0) j ++;
        }
        break;
      case '2':
        /* �м���� */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL)
        {
          swVdebug(0,"S3222: [����/��������] �м����[%s]�������,������=%d",slSwt_sys_fmt_d.imf_name,ilRtncode);
           return(FAIL);
        }
        ilRtncode = swFmlset(slSwt_sys_fmt_d.imf_name,
          ilLength,alResult,psgVarbuf);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3224: [����/��������] swFmlset()����,�м����[%s]��ֵ����,������=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        else
        {
          swVdebug(3,"S3226:�м����[%s]swFmlset�ɹ�!", 
               slSwt_sys_fmt_d.imf_name);
        }
	break;
      default:
        /* ��ͨTDF��� */
/* add new */
        if (slSwt_sys_fmt_d.fld_id == 0) continue;
/* add end */
        /* memset(&prebuf[j],0x00,sizeof(union preunpackbuf)); */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL) 
        {
          swVdebug(0,"S3228: [����/��������] _swExpress()���� fail[%s],������=%d ", 
             slSwt_sys_fmt_d.fld_express,ilRtncode);
          return(FAIL);
        }
        /* Add by szhengye:���򳤶�Ϊ0��FML�� */
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
 ** ������      ��swGetSEPfromTDF
 ** ��  ��      �����ݸ�ʽת����Ӹ�ʽת����ϸ��õ���/�޷ָ�����Ԥ��� 
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��fmt_group:����TDFת�����õĸ�ʽת����
 **               prebug:����TDFת��������FMLԤ�������ŵ�buffer
 ** ����ֵ      ��SUCCESS:�ɹ�/FAIL:ʧ��
***************************************************************/
int swGetSEPfromTDF(iFmt_group, lFmt_offset, prebuf)
 /*del by zcd 20141222
  int  iFmt_group;   
  ***end of del by zcd 20141222*/
  /*add by zcd 20141222*/
  long   iFmt_group;       /* ָ����ʽת���� passed */
  /*end of add by zcd 20141222*/
  long  lFmt_offset;      /* ��ʽת�����ڹ����ڴ��е�ƫ���� passed*/
  union preunpackbuf *prebuf;
{
  unsigned int ilRtncode,i,j,ilFldnamelen;
  short ilFldlen;
  short ilRecid=0,ilGrpid=0,ilFldid=0;
  char alResult[iFLDVALUELEN+1];
  char alFldval[iFLDVALUELEN+1];
  char alFldname[iFLDNAMELEN + 1];
  short ilLength;
  long  llFmt_d_offset;  /* TDF ���ƫ���� */
  long  llFmt_d_offset_org;
  int ilFmt_d_count;   /* TDF ������� */  /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;

  psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
  j = 0;
  
  /* ilFmt_d_count  = TDF ������� */
  /* llFmt_d_offset = ��һ��TDF���ƫ���� */
  ilRtncode = swShmselect_swt_sys_fmt_grp_offset(lFmt_offset, 
    &slSwt_sys_fmt_grp, &ilFmt_d_count, &llFmt_d_offset_org);
  
  if (ilRtncode != SUCCESS)
  {
    return (FAIL);
  }
  
  for(i=0; i < ilFmt_d_count; i++)
  {
    /* TDF ���ƫ���� */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* ȡ���� TDF �����Ϣ */
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
        /* �ظ���ʶ */
        ilRtncode = swFmlget(psgVarbuf,"_RECCOUNT",&ilFldlen,alFldval);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3230: [����/��������] swFmlget()����,�м����_RECCOUNTȡֵ����,������=%d",ilRtncode);
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
            swVdebug(0,"S3232: [����/��������] _swExpress()���� fail[%s],������=%d", 
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
              swVdebug(0,"S3234: [����/��������] swFmlset()����,�м����[%s]��ֵ����,������=%d",alFldname,ilRtncode);
               return(FAIL);
            }
          }
        }
      }
        break;
      case '2':
        /* �м���� */
        memset(alResult,0x00,iFLDVALUELEN);
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult,&ilLength);
        if(ilRtncode == FAIL)
        {
          swVdebug(0,"S3236: [����/��������] �м����[%s]�������,������=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        ilRtncode = swFmlset(slSwt_sys_fmt_d.imf_name,
          ilLength,alResult,psgVarbuf);
        if (ilRtncode != SUCCESS)
        {
          swVdebug(0,"S3238: [����/��������] �м����[%s]��ֵ����,������=%d", 
             slSwt_sys_fmt_d.imf_name,ilRtncode);
          return(FAIL);
        }
        else
        {
          swVdebug(2,"S3240: swGetSEPfromTDF():�м����[%s]swFmlset�ɹ�!",
            slSwt_sys_fmt_d.imf_name);
        }
        break;
      default:
        /* ��ͨTDF��� */
        ilRtncode = _swExpress(slSwt_sys_fmt_d.fld_express,
          alResult, &ilLength);
        if(ilRtncode == FAIL) 
        {
          swVdebug(0,"S3242: [����/��������] _swExpress fail[%s],������=%d", 
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
 ** ������      ��swFMLcheck
 ** ��  ��      ������˳��ŵı�׼��ֵ(�ڴ˹����У����ݱ�׼����
 **               ���ڲ���׼���б��е�������ý����û��Զ��庯��У��
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
***************************************************************/
int swFMLcheck (union preunpackbuf *prebuf )
{
  short i, j, ilRtncode;
  char  alResult[iFLDVALUELEN + 1];
  char  alFldvalue[iFLDVALUELEN + 1];
  char  alFldname[iFLDNAMELEN + 1];
  short ilFldlen,ilLength;
  short ilImfcount;  /* swt_sys_imf ���¼�� */
  /* struct swt_sys_imf *pslSwt_sys_imf;  �׼�¼��ַ */
  struct swt_sys_imf pslSwt_sys_imf[iMAXIMFREC];
  
  /* �õ� swt_sys_imf �����м�¼����¼�� */
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
          swVdebug(0,"S3244: [����/��������] ��У����ʽ%s����,������=%d",pslSwt_sys_imf[j].imf_check,ilRtncode);
           _swMsgsend(302023,NULL);
          return(FAIL);
        }
        if (alResult[0] != '1')
        {
          swVdebug(0,"S3246: [����/����] ��У�����: ��[%s],ֵ[%s],У��[%s],���=%d", alFldname, alFldvalue, pslSwt_sys_imf[j].imf_check, alResult[0]);
          return(FAIL);
        }
        break;
      }
    }
  }
  return ( SUCCESS );
}

/**************************************************************
 ** ������      ��swISOcheck
 ** ��  ��      ������ISO8583���û��Զ��庯������check,
 **               ����8583�����ͽ���check
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.12.9
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
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
    
    /* ���� tab_id��fldid ��swt_sys_8583���ж�Ӧ��¼ */
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
        swVdebug(0,"S3250: [����/��������] _swExpress()����,ISO8583��У����ʽ%s����,������=%d",alFld_rule,ilRtncode);
         _swMsgsend(302025,NULL);
        return(FAIL);
      }
      if (alResult[0] != '1')
      {
         swVdebug(0,"S3252: [����/����] ��У�����: ��[%d],ֵ[%s],У��[%s],���=%c",
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
          swVdebug(0,"S3254: [����/��������] _swMatch()����,��У�����:��[%d],ֵ[%s]ӦΪ����,������=%d", prebuf[i].sIsobuf.iFldid, prebuf[i].sIsobuf.aFldvalue, ilRtncode);
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
          swVdebug(0,"S3256: [����/��������] _swMatch()����,��У�����: ��[%d],ֵ[%s]ӦΪ���ֻ���ĸ,������=%d", prebuf[i].sIsobuf.iFldid, prebuf[i].sIsobuf.aFldvalue, ilRtncode);
          return(FAIL);
        }
        break;
    }
  }
  return ( SUCCESS );
}
/**************************************************************
 ** ������      ��swSeppack
 ** ��  ��      ���зָ������Ĵ��
 ** ��  ��      ��ʷ����
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��prebuf:Ԥ�������  aMsgbody:��ɰ��Ĵ�ŵ�
 **               iMsglen:���ش�ɰ��ĳ���
 ** ����ֵ      ��SUCCESS:�ɹ�  FAIL:ʧ��
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
 ** ������      ��swGetXMFfromTDF
 ** ��  ��      �����ݸ�ʽת����Ӹ�ʽת����ϸ��õ�XMFԤ��� 
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��iFmt_group:����TDFת�����õĸ�ʽת����
 **               iFmt_offset:��ʽת�����ڹ����ڴ��е�ƫ����
 ** ����ֵ      ��SUCCESS:�ɹ�/FAIL:ʧ��
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
  long  llFmt_d_offset;  /* TDF ���ƫ���� */
  long  llFmt_d_offset_org;
  long  llFmt_offset;
  long  llMsgoffset;
  int ilFmt_d_count;   /* TDF ������� */  /*modified by baiqj20150505 PSBC_V1.0 short -> int*/
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;
  struct swt_sys_fmt_d   slSwt_sys_fmt_d;
  char  alMsgbody_save[iMSGMAXLEN];
  unsigned int ilMsgbodylen_save;

  /* psgVarbuf[0].sImfbuf.aFldname[0] = '\0'; */
  j = 0;
  
  /* ilFmt_d_count  = TDF ������� */
  /* llFmt_d_offset = ��һ��TDF���ƫ���� */
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
  
    /* TDF ���ƫ���� */
    llFmt_d_offset = llFmt_d_offset_org + i;   
    
    /* ȡ���� TDF �����Ϣ */
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
        swVdebug(0,"S3260: [����/��������] _swExpress()����,�������ʽ%s����,������=%d",slSwt_sys_fmt_d.fmt_cond,ilRc);
        return(FAIL);
      }
      if (alResult[0] == '0')
      {
        swVdebug(2,"S3262: TDF�������ʽ������:%s",slSwt_sys_fmt_d.fmt_cond);
        continue;
      }
    }
    switch (slSwt_sys_fmt_d.fmt_flag[0])
    {
      case 'A':		/* GOTO���:{GOTO �к�} */
        ilScript = slSwt_sys_fmt_d.fld_id;  /* modifyed by dgm 2002.06.17 */
        swVdebug(2,"S3264:GOTO���:{GOTO %d}",ilScript);
        for(j=0; j < ilFmt_d_count; j++)
        {
          /* ȡ���� TDF �����Ϣ */
          ilRc = swShmselect_swt_sys_fmt_d_offset(llFmt_d_offset_org + j, 
            &slSwt_sys_fmt_d);
    
          if (ilRc != SUCCESS) return (FAIL);
        
          if (slSwt_sys_fmt_d.id == ilScript)
          {
            i = j;
            goto Label_XMFGOTO;
          }
        }
        swVdebug(0,"S3266: [����/����] Ŀ����䲻����[GOTO %d]",ilScript);
        return(FAIL);

      case 'B':		/* ���м������ֵ(����ID):{VARID ��ID=���ʽ} */
        swVdebug(2,"S3268: ���м����(����ID)��ֵ:{VARID %ld=%s}",
          slSwt_sys_fmt_d.fld_id,slSwt_sys_fmt_d.fld_express);/*modified by baiqj20150413 PSBC_V1.0*/
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3270: [����/��������] ���ʽ%s����,������=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        if (slSwt_sys_fmt_d.fld_id >= iBUFFLDNUM)
        {
          swVdebug(0,"S3272: [����/����] �м������IDԽ��(>=%d)",iBUFFLDNUM);
          return(FAIL);
        }
        psgVaridbuf[slSwt_sys_fmt_d.fld_id].sImfbuf.iFldlen = ilResultlen;
        memcpy(psgVaridbuf[slSwt_sys_fmt_d.fld_id].sImfbuf.aFldvalue,
          alResult,ilResultlen);
        continue;

      case 'C':		/* �м������ֵ:{VAR ������=���ʽ} */

        /* add by bmy 2002.03.12 */
        if (slSwt_sys_fmt_d.imf_array[0])
        {
          swVdebug(2,"S3274: ���м����(������)��ֵ:{VAR %s[%s]=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.imf_array,slSwt_sys_fmt_d.fld_express);
          ilRc = swFmlget(psgVarbuf, slSwt_sys_fmt_d.imf_array,
            &ilResultlen, alResult);
          if (ilRc != SUCCESS)
          {
            swVdebug(0,"S3276: [����/��������] swFmlget()����,ȡ�м����[%s]����",slSwt_sys_fmt_d.imf_array);
            return(FAIL);
          }
          ilResultlen = strlen(slSwt_sys_fmt_d.imf_name);
          slSwt_sys_fmt_d.imf_name[ilResultlen] = '#';
          slSwt_sys_fmt_d.imf_name[ilResultlen+1] = atoi(alResult);
          slSwt_sys_fmt_d.imf_name[ilResultlen+2] = 0x00;
          if ((ilResultlen+2) > iFLDNAMELEN)
          {
             swVdebug(0,"S3278: [����/����] FML����[%s]���ȳ�����󳤶�[%d]",
               slSwt_sys_fmt_d.imf_name, iFLDNAMELEN);
            return(FAIL);
          }
        }
        else
        {
          swVdebug(2,"S3280: ���м����(������)��ֵ:{VAR %s=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.fld_express);
        }
        
        /* end of add by bmy */

        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3282: [����/��������] ���ʽ%s����,������=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        ilRc = swFmlset(slSwt_sys_fmt_d.imf_name,ilResultlen,alResult,
          psgVarbuf);
        if (ilRc != SUCCESS)
        {
          swVdebug(0,"S3284: �м����[%s]��ֵ����!",
            slSwt_sys_fmt_d.imf_name);
          return(FAIL);
        }
        continue;

      case 'D':         /* ��FML������:{SET ����=���ʽ} */
        
        /* add by bmy 2002.03.12 */
        if (slSwt_sys_fmt_d.imf_array[0])
        {
          swVdebug(2,"S3286: ��FML������:{SET %s[%s]=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.imf_array,slSwt_sys_fmt_d.fld_express);
          ilRc = swFmlget(psgVarbuf, slSwt_sys_fmt_d.imf_array,
            &ilResultlen, alResult);
          if (ilRc != SUCCESS)
          {
            swVdebug(0,"S3288: [����/��������] swFmlget()����,ȡ�м����[%s]����",slSwt_sys_fmt_d.imf_array);
            return(FAIL);
          }
          ilResultlen = strlen(slSwt_sys_fmt_d.imf_name);
          slSwt_sys_fmt_d.imf_name[ilResultlen] = '#';
          slSwt_sys_fmt_d.imf_name[ilResultlen+1] = atoi(alResult);
          slSwt_sys_fmt_d.imf_name[ilResultlen+2] = 0x00;
          if ((ilResultlen+2) > iFLDNAMELEN)
          {
             swVdebug(0,"S3290: [����/����] FML����[%s]���ȳ�����󳤶�[%d]",
               slSwt_sys_fmt_d.imf_name, iFLDNAMELEN);
            return(FAIL);
          }
        }
        else
        {
          swVdebug(2,"S3292: ��FML������:{SET %s=%s}",
            slSwt_sys_fmt_d.imf_name,slSwt_sys_fmt_d.fld_express);
        }
        
        /* end of add by bmy */

        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3294: [����/��������] _swExpress()����,���ʽ%s����,������=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        ilRc = swFmlset(slSwt_sys_fmt_d.imf_name,ilResultlen,alResult,
          psgUnpackbuf);
        if (ilRc)
        {
          swVdebug(0,"S3296: [����/��������] swFmlset()����,����[%s]��ֵ����,������=%d",
            slSwt_sys_fmt_d.imf_name,ilRc);
          return(FAIL);
        }
        continue;

      case 'E':		/* ����TDF(�ݹ�):{TDF ��ʽת����,[���ʽ]} */
        swVdebug(2,"S3298: ����TDF(�ݹ�):{TDF %ld,%s}",
          slSwt_sys_fmt_d.fld_id,slSwt_sys_fmt_d.fld_express); /*modify by baiqj20150413 PSBC_V1.0*/
        if (slSwt_sys_fmt_d.fld_express[0])
        {
          ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
          if (ilRc)
          {
            swVdebug(0,"S3300: [����/��������] _swExpress()����,���ʽ%s����,������=%d",slSwt_sys_fmt_d.fld_express,ilRc);
            return(FAIL);
          }
        }
        /* �����ֳ� */
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
          swVdebug(0,"S3302: [����/�����ڴ�] ָ����ʽת����Ϊ[%ld]������,������=%d",
            slSwt_sys_fmt_d.fld_id,ilRc);   /*modified by baiqj20150413 PSBC_V1.0*/
          return(FAIL);
        }

        /* �ݹ����swGetXMFfromTDF���� */
/*        
        if (slSwt_sys_fmt_d.fld_express[0])
        {
*//* delete by zjj 2004.03.11 */       	
          ilRc = swGetXMFfromTDF(slSwt_sys_fmt_d.fld_id,llFmt_offset);
          if (ilRc)
          {
            swVdebug(0,"S3304: [����/��������] swGetXMFfromTDF()����,������=%d",ilRc);
            return(FAIL);
          }
/*          
        }
*/  /* delete by zjj 2004.03.11 */

        if (slSwt_sys_fmt_d.fld_express[0])
        {
          /* �ָ��ֳ� */
          igMsgbodylen = ilMsgbodylen_save;
          memcpy(agMsgbody,alMsgbody_save,igMsgbodylen);
          lgMsgoffset = llMsgoffset;
        }
        continue;

      case 'F':		/* RETURN(������): {RETURN ������} */
        swVdebug(2,"S3306: RETURN(������):{RETURN %ld}",slSwt_sys_fmt_d.fld_id);/*modified by baiqj20150413 PSBC_V1.0*/
        return(slSwt_sys_fmt_d.fld_id);

      case 'G':		/* ����ƽ̨�������ʽ:{TRACE ���ʽ} */
        swVdebug(2,"S3308: ����ƽ̨�������ʽ:{TRACE %s}",
          slSwt_sys_fmt_d.fld_express);
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3310: [����/��������] _swExpress()����,���ʽ%s����,������=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          continue;
        }
        alResult[ilResultlen] = '\0';
        swDebug("S:TRACE:%s",alResult);
        continue;

      case 'H':		/* ִ��ƽ̨ƽ̨ʽ(DO):{DO ƽ̨���ʽ} */
        swVdebug(2,"S3312: ִ��ƽ̨ƽ̨ʽ(DO):{DO %s}",
          slSwt_sys_fmt_d.fld_express);
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3314: [����/��������] _swExpress()����,���ʽ%s����,������=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }
        ilRc = swFmlset("_RETURN",ilResultlen,alResult,psgVarbuf);
        if (ilRc)
        {
          swVdebug(0,"S3316: [����/��������] swFmlset()����,�м����[_RETURN]��ֵ����,������=%d",ilRc);
          return(FAIL);
        }
        continue;

      case 'I':		/* �ô��������: {FIELD ��ID=ƽ̨���ʽ} */
        swVdebug(2,"S3318: �ô��������:{FIELD %ld=%s}",
          slSwt_sys_fmt_d.fld_id,slSwt_sys_fmt_d.fld_express);   /*modified by baiqj20150413 PSBC_V1.0*/
        ilRc = _swExpress(slSwt_sys_fmt_d.fld_express,alResult,&ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3320: [����/��������] _swExpress()����,���ʽ%s����,������=%d",slSwt_sys_fmt_d.fld_express,ilRc);
          return(FAIL);
        }

        ilRc = swSetXMFprebuffer(slSwt_sys_fmt_d.fld_id,alResult,ilResultlen);
        if (ilRc)
        {
          swVdebug(0,"S3322: [����/��������] swSetXMFprebuffer()�������ô��Ԥ����[%ld]����,������=%d",
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
        swVdebug(0,"S3324: [����/����] δ֪��TDF�ű�����('%c',%d)",
          slSwt_sys_fmt_d.fmt_flag[0],slSwt_sys_fmt_d.fmt_flag[0]);
        return(FAIL);
    }
  }
  return(0);
}
/**************************************************************
 ** ������      ��swSetXMprebuffer
 ** ��  ��      ����XMF������
 ** ��  ��      ��ʷ����
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��iFldid:��id  aFldvalue:��ֵ  iFldlen:�򳤶�
 ** ����ֵ      ��SUCCESS:�ɹ�  FAIL:ʧ��
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
    swVdebug(0,"S3326: [����/����] Ԥ�ð����![%d]>[%d]",i,iBUFFLDNUM);
    return(-1);
  }

  ilPoint = i;
  ilFldlen = iFldlen;
  ilCount = (ilFldlen - 1)/iPERFLDVALUELEN + 1;

  if ((i+ilCount)>=iBUFFLDNUM)
  {
    swVdebug(0,"S3328: [����/����] Ԥ�ð����![%d]>[%d]",i+ilCount,iBUFFLDNUM);
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
    psgUnpackbuf[i+ilCount].sIsobuf.iFldid = -1;  /* Ԥ�ð�������־ */
    break;
  }

  return(0);
}


/**************************************************************
 ** ������      ��swXMFpack
 ** ��  ��      ��XMF���Ĵ��
 ** ��  ��      ��ʷ����
 ** ��������    ��1999.12.6
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��prebuf:Ԥ�������  msgbody:��ɰ��Ĵ�ŵ�
 **               msglen:���ش�ɰ��ĳ���
 ** ����ֵ      ��SUCCESS:�ɹ�  FAIL:ʧ��
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

