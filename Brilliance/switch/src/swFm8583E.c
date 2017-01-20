/****************************************************************/
/* ģ����    ��FM8583E                                        */ 
/* ģ������    ��8583E������                                   */
/* ��	 ��    ��                                      */
/* ��������    ��2002/1/29                                      */
/* ����޸����ڣ�2002/1/29                                      */
/* ģ����;    ��8583E���Ĵ������ĺ���                         */
/* ��ģ���а������º���������˵����                             */
/*                        (2) int  sw8583unpack();               */
/*                        (5) int  swBitMapIsOn();              */
/*                        (6) int  swSetIsoStru();              */
/*                        (7) int  swGetIsoStru();              */
/*                        (8) int  swIsdigit();                 */
/*                        (9) int  sw8583pack();                 */
/*                       (10) int  swSetBitmap();               */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*  1999/12/24 ��bitmap���͵Ĳ�ͬ���д���                       */
/*  2000/3/13  ���Ӷ�MAC�Ĵ���                                  */
/*  2000/11/11 ���Ӷ�ASCBCD���͵Ĵ���                           */
/*  2001/2/22  Oracle��ֲ                                       */
/*  2001/3/22  �Ż�                                             */
/*  2001/3/24  ���V3.01,����1607��                           */
/*  2001/4/3   DB2��ֲ,�޸���ASC2BCD������BUG(1613)             */
/*  2001/4/4   ��ASC2BCD������memset����(1621)                  */
/****************************************************************/

/* switch���� */
#include "switch.h"
#include "swNdbstruct.h"
#include "swShm.h"

extern int _ufNtoT(char *aNstr,unsigned int iLen,char *aTstr);

/* �������� */

extern union	preunpackbuf psgUnpackbuf[iBUFFLDNUM];
char agMacbuf[iMSGMAXLEN];	/* MAC buffer */
short igMacbuflen;		/* MAC length */

/* ����ԭ�� */
int  sw8583unpack();
int  sw8583pack();
static int  swBitMapIsOn();
static int  swSetBitmap();
/*static int  swIsdigit();  */
static int  swSetIsoStru();
static int  swGetIsoStru();
static int  swTranbuf();

/**************************************************************
 ** ������      ��sw8583unpack
 ** ��  ��      ��8583����Ԥ���
 ** ��  ��      ��
 ** ��������    ��1999/12/6
 ** ����޸����ڣ�1999/12/6
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��msgbody: 8583����
                  msglen : ���ĳ���
                  qid    : �����
                  prebuf : FMLԤ��������
 ** ����ֵ      ��SUCCESS,FAIL
***************************************************************/
int sw8583unpack(msgbody,msglen,qid,prebuf)
  unsigned char *msgbody;  
  unsigned int msglen;
  short qid;
  union preunpackbuf *prebuf;
{
  unsigned char  *plPointer;               /* ƫ��ָ�� */
  unsigned char  alBitmap[17];             /* BITMAP */
  unsigned char  alTmpbuf[iFLDVALUELEN];   /* ���� */
  int   i;                                 /* ѭ������,8583id���±� */
  int   k,ilRc;                                 /* ѭ������,prebuf���±� */
  short ilFlag;                            /* ��־ */
  unsigned int ilLen;                             /* ���� */
  short ilTableId;                         /* ��id */
  short ilRtncode;
  struct swt_sys_queue slSwt_sys_queue;
/*  struct swt_sys_8583  slSwt_sys_8583;  */
  struct swt_sys_8583E   slSwt_sys_8583E;
 /* short ilRtnlen = 0; */
  short  ilRtnlen = 0; 

  char  alValue[iMSGMAXLEN];
  char  alBuf[iEXPRESSLEN];
  char  alFldvaltmp[ iPERFLDVALUELEN + 1 ];
  unsigned char  alParm[3][iEXPRESSLEN];
  
  struct isoprebuf
  {
    short iFldid;
    short iFldlen;
    char  aFldvalue[iFLDVALUELEN + 1];
    char  aOrgfldlen[10];
    short iOrgfldlen; 
    char  aOrgfldvalue[iFLDVALUELEN + 1];
  } slIsobuf;

  memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
  prebuf[0].sIsobuf.iFldid = -1;
 
  plPointer = msgbody;
  
  /* ����msgtype */  
  slIsobuf.iFldid    = 1;  

  ilRtncode = swShmselect_swt_sys_queue(qid, &slSwt_sys_queue);
  if (ilRtncode != SUCCESS)
  {
    swVdebug(1,"S3424: [����/��������] swShmselect_swt_sys_queue()����,������=%d qid = %d",ilRtncode,qid);  	 
    return (FAIL);
  }
  ilTableId=slSwt_sys_queue.e8583tab_id;
  
  ilRtncode = swShmselect_swt_sys_8583E(ilTableId, 1, &slSwt_sys_8583E);  
  if (ilRtncode != SUCCESS) 
  { 	
    swVdebug(1,"S3426: [����/��������] swShmselect_swt_sys_8583E()����,������=%d,tab_id=%d  fld_id=1",ilRtncode,ilTableId);  	
    return (FAIL);     
  }
  
  memset(slIsobuf.aFldvalue,0x00,sizeof(slIsobuf.aFldvalue));
  memset(slIsobuf.aOrgfldvalue,0x00,sizeof(slIsobuf.aOrgfldvalue));
  memset(alTmpbuf,0x00,sizeof(alTmpbuf));
  
  memcpy(alTmpbuf,plPointer,slSwt_sys_8583E.fld_len);
  memcpy(slIsobuf.aOrgfldvalue,alTmpbuf,slSwt_sys_8583E.fld_len);
  memcpy((unsigned char *)slIsobuf.aFldvalue,alTmpbuf,slSwt_sys_8583E.fld_len);
  slIsobuf.iFldlen  = slSwt_sys_8583E.fld_len;   
  
  if (slSwt_sys_8583E.fld_valunpk[0] != '\0')
  {
    _ufNtoT((char *)plPointer,slSwt_sys_8583E.fld_len,(char *)alParm[1]);
    alParm[2][0] = '\0';
    swTranbuf(slSwt_sys_8583E.fld_valunpk,alParm,alBuf);
    ilRc = _swExpress(alBuf,(char *)alTmpbuf,&ilRtnlen);
    if (ilRc == FAIL)
    {
      swVdebug(1,"S3428: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
      return(FAIL);
    }
    memcpy((unsigned char *)slIsobuf.aFldvalue,alTmpbuf,ilRtnlen);
    slIsobuf.iFldlen  = ilRtnlen; 
  }
  plPointer += slSwt_sys_8583E.fld_len;
    
  /* �ж�msgtype�Ϸ��� */
  /*  
  if(swIsdigit(slIsobuf.aFldvalue,slIsobuf.iFldlen)==FALSE)
  {
    swVdebug(0,"S3430: [����/��������] swIsdidit()����,msgtype format error");
    return(FAIL);
  } */ /* delete by zjj 2002.05.14 for qy suggestion */

  swVdebug(2,"S3432: OUT msgtype=%s",slIsobuf.aFldvalue);
  
   /* ��ISOmsgtype�� */
  swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
    slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);
         
  memset(agBitmap,0x00,33);
  memcpy(agBitmap,plPointer,32);
  swVdebug(2,"S3434: bitmap ag is ...");
    if(cgDebug>=1)swDebughex((char *)agBitmap,32);
  
  /* ���ݶ˿������еĻ���bitmap����ȡֵ��*/
  memset(alBitmap,0x00,sizeof(alBitmap));
  memcpy(alBitmap,plPointer,slSwt_sys_queue.bitmap_len);
  ilFlag = 1;
  /* ������ʽ */
  if (slSwt_sys_queue.bitmap_unpk[0] != '\0')
  {
    _ufNtoT((char *)alBitmap,slSwt_sys_queue.bitmap_len,(char *)alParm[1]);
    alParm[2][0] = '\0';
    swTranbuf(slSwt_sys_queue.bitmap_unpk,alParm,alBuf);
    ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
    if (ilRc == FAIL)
    {
      swVdebug(1,"S3436: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
      return(FAIL);
    }
    memcpy(alBitmap,(unsigned char *)alFldvaltmp,ilRtnlen);
  }
  else
    ilRtnlen = slSwt_sys_queue.bitmap_len;

  plPointer += slSwt_sys_queue.bitmap_len;
  /* ȡ��չλ��*/
  if (alBitmap[0] & 0x80)
  {
     ilFlag = 2;
     memcpy(alBitmap + ilRtnlen,plPointer,slSwt_sys_queue.bitmap_len);
     if (slSwt_sys_queue.bitmap_unpk[0] != '\0')
     {
       _ufNtoT((char *)plPointer,slSwt_sys_queue.bitmap_len,(char *)alParm[1]);
       alParm[2][0] = '\0';
       swTranbuf(slSwt_sys_queue.bitmap_unpk,alParm,alBuf);
       ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
       if (ilRc == FAIL)
       {
         swVdebug(1,"S3438: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
         return(FAIL);
       }    
       memcpy(alBitmap+ilRtnlen,(unsigned char *)alFldvaltmp,ilRtnlen);
     }
     plPointer += slSwt_sys_queue.bitmap_len;     
  }

  if(cgDebug>=2) swDebughex((char *)alBitmap,8 * ilFlag);
  
  swVdebug(2,"S3440: Stanford:�����=[%d],�򳤶�=[%d],��ֵ=[%s]",
          slIsobuf.iFldid,
        slIsobuf.iFldlen,
        slIsobuf.aFldvalue); 
  
  k=1;   
  for(i=2;i<=64*ilFlag;i++)
  {
    if(swBitMapIsOn(alBitmap,i)==TRUE)
    {
      memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
      
      slIsobuf.iFldid = i;

      ilRtncode = swShmselect_swt_sys_8583E(ilTableId, i, 
        &slSwt_sys_8583E);
          
      if (ilRtncode != SUCCESS) return (FAIL);      
        
     swVdebug(2,"S3442: ANN:k=[%d],id=[%d],fldtype=[%d],maxlen=[%d],\
       fld_len2=[%d],fld_lenunpk=[%s],fld_lenpack=[%s],fld_valunpk=[%s],fld_valpack=[%s]",k,i,
          slSwt_sys_8583E.fld_type, 
          slSwt_sys_8583E.fld_len,
/*          slSwt_sys_8583E.fld_attr,  */
/*          slSwt_sys_queue.tran_type[i-1],  */
          slSwt_sys_8583E.fld_len2,
          slSwt_sys_8583E.fld_lenunpk,
          slSwt_sys_8583E.fld_lenpack,
          slSwt_sys_8583E.fld_valunpk,
          slSwt_sys_8583E.fld_valpack);

      ilLen = swSetIsoStru( 
        plPointer,
        slSwt_sys_8583E.fld_type, 
        slSwt_sys_8583E.fld_len,
/*        slSwt_sys_queue.tran_type[i-1], 
        slSwt_sys_8583E.fld_attr, */
        slIsobuf.aFldvalue, 
        &(slIsobuf.iFldlen),
	slIsobuf.aOrgfldlen,
	slIsobuf.aOrgfldvalue,
	&(slIsobuf.iOrgfldlen),
/*        slSwt_sys_queue.code_type[0] */
        slSwt_sys_8583E.fld_len2,
        slSwt_sys_8583E.fld_lenunpk,
        slSwt_sys_8583E.fld_valunpk);
     /* ��ISO�� */
     swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
       slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);

      if(ilLen==FAIL)
      {
        swVdebug(1,"S3444: [����/��������] sw8583unpack()����,��8583��ʧ��,�⵽��[%d]����",i);
          return(FAIL);
      }
      swVdebug(2,"S3446: �����򳤶�=%d",slIsobuf.iFldlen);
      if (cgDebug>=2)
        swDebughex(slIsobuf.aFldvalue,slIsobuf.iFldlen);
      k++;
      plPointer += ilLen; 
      if(plPointer - msgbody > msglen)
      {
	swVdebug(1,"S3448: ���ĳ��ȳ���!,msglen = %d  plPointer - msgbody = %d",msglen,plPointer - msgbody);
  	return(FAIL);
      }
    }
  }

  swVdebug(3,"S3450: ��8583�ɹ�,��������...");
  for(i=0;prebuf[i].sIsobuf.iFldid!=-1;i++)
  {
    swVdebug(3,"S3452:id=[%d],len=[%d],value=[%s],orglen[%s]orgvalue[%d][%s]",
      prebuf[i].sIsobuf.iFldid,
      prebuf[i].sIsobuf.iFldlen,
      prebuf[i].sIsobuf.aFldvalue,
      prebuf[i].sIsobuf.aOrgfldlen,
      prebuf[i].sIsobuf.iOrgfldlen,
      prebuf[i].sIsobuf.aOrgfldvalue
    );
  }
  
  /* ���� macbuf */
  if (slSwt_sys_queue.mac_express[0] != '\0')
  {
/*    memcpy(psgMacneedbuf,psgPreunpackbuf,sizeof(psgMacneedbuf)); */
    ilRc = _swExpress(slSwt_sys_queue.mac_express,alValue,&ilRtnlen);
    if (ilRc == FAIL)  return(FAIL);
    memcpy(agMacbuf,alValue,ilRtnlen);
    igMacbuflen = ilRtnlen;  
  }
  else
  {
/*    memcpy(psgMacneedbuf,psgPreunpackbuf,sizeof(psgMacneedbuf)); */
    agMacbuf[0] = '\0';
    igMacbuflen = ilRtnlen;
  }
  return(SUCCESS);
}
    
/*******************************************************************/
/* �� �� ��       :  swBitMapIsOn                                  */
/* ��    ��       :  �ж�bitmap����Ӧλ�Ƿ���Ч                    */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  1998��10��13��                                */
/* ����޸�����   :  1998��10��13��                                */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  pBitMap bitmap����Ӧ��ָ��                    */ 
/*                   iCount  10��������λ��                        */
/* �� �� ֵ       :  TRUE:��Ч FALSE:��Ч                          */
/*******************************************************************/
/* �޸ļ�¼       :                                                */
/*                                                                 */
/*******************************************************************/
static int swBitMapIsOn( pBitMap , iCount )
  unsigned char  *pBitMap;
  int   iCount;
{
  if(iCount >=1 && iCount<=64)
    return( ( pBitMap[ (iCount - 1)/ 8 ] >> \
      ( 7 - (iCount-1) % 8 ) ) & 0x01 );
  else if(iCount >=65 && iCount <=128 )
  {
    iCount -= 64;
    return( ( pBitMap[ (iCount - 1)/ 8 + 8 ] >> \
      (7 - (iCount-1) % 8)) & 0x01 );
  }
  else
    return(FALSE);
}

/*******************************************************************/
/* �� �� ��       :  swSetIsoStru                                  */
/* ��    ��       :  8583���                                      */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  1998��12��8��                                 */
/* ����޸�����   :  1998��12��8��                                 */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  dest_buf : Ԥ�����                           */
/*                   org_buf  : 8583��                             */
/*                   df_type  : ������                             */ 
/*                   maxlen   : �����󳤶�                       */
/*                   bcd_flag : BCD,ASCII��־                      */
/*                   reallen  : ���ʵ�ʳ���                       */
/*                   fillflag : BCD����־                        */
/*                   org_len  : ԭʼ���ȴ�                         */
/*                   org_value: ԭʼֵ��                           */
/*                   ebcdic   : EBCDIC��ʶ                         */
/* �� �� ֵ       :  ����8583�����򳤶�(��������λ),�����򷵻�FAIL */ 
/*******************************************************************/
/* �޸ļ�¼       :                                                */
/*                                                                 */
/*******************************************************************/
static int swSetIsoStru(org_buf,df_type,maxlen,dest_buf,reallen,org_len,org_value,iOrg_len,iLen_len,aFld_lenunpk,aFld_valunpk)

  unsigned char		*org_buf;
  int			df_type; 
  int			maxlen;
  unsigned char		*dest_buf;
  short			*reallen;
  unsigned char		*org_len;
  short			*iOrg_len;
  unsigned char		*org_value;
  short			iLen_len;
  char			*aFld_lenunpk;
  char			*aFld_valunpk;
  
{
  int	 len;
  static   char  tmp_buf[512];
  short  ilLen_len ;  /*ilRtnlen; */
  short ilRtnlen; /*changed  by dyw*/
  int ilRc;
  char alBuf[iEXPRESSLEN];
  char  alFldvaltmp[ iPERFLDVALUELEN + 1 ];
  unsigned char alParm[3][iEXPRESSLEN];

  memset(tmp_buf, 0x00, sizeof(tmp_buf));
  if (df_type & EV)  /* �䳤��*/
  {
    memcpy((unsigned char *)tmp_buf,(unsigned char *)org_buf,iLen_len);
    if (aFld_lenunpk[0])
    { 
       _ufNtoT((char *)org_buf,iLen_len,(char *)alParm[0]);
       alParm[1][0] = '\0';
       swTranbuf(aFld_lenunpk,alParm,alBuf);
       swVdebug(3,"iLen_len=%d",iLen_len);
       swVdebug(3,"alParm=%x %x",alParm[0][0],alParm[0][1]);
       if(cgDebug >= 2) 
         swDebughex((char *)alParm[0],4);
       swVdebug(3,"alBuf=%s",alBuf);
       ilRc = _swExpress(alBuf,tmp_buf,&ilRtnlen);
       if (ilRc == FAIL)
       {
         swVdebug(1,"S3454: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
         return(FAIL);
       }    
    }
    ilLen_len = iLen_len;  
    len = atoi(tmp_buf);
    swVdebug(3,"S3456: tmp_buf=%s",tmp_buf);
    swVdebug(3,"S3458: len=%d",len);
    if (len < 0 || len > maxlen)  return(FAIL);
    *iOrg_len=len;
    
    memcpy(dest_buf,org_buf + ilLen_len,len);
    if (aFld_valunpk[0])
    {
      _ufNtoT((char *)(org_buf + ilLen_len),len,(char *)alParm[1]);
      alParm[2][0] = '\0';
      swTranbuf(aFld_valunpk,alParm,alBuf);
      ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
      if (ilRc == FAIL)
      {
        swVdebug(1,"S3460: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
        return(FAIL);
      }   
      memcpy(dest_buf,(unsigned char *)alFldvaltmp,ilRtnlen);
      *reallen = ilRtnlen;
    }
    else
      *reallen = len;
    _ufNtoT((char *)org_buf,ilLen_len,(char *)org_len);
    _ufNtoT((char *)org_buf + ilLen_len,len,(char *)org_value);
    return(len+iLen_len);
  }
  else  /* ������*/
  {
    if (df_type & EB)
    {
      if (maxlen%8) return(FAIL);
      maxlen = maxlen/8;
    } 
    *reallen=maxlen;
    memcpy(dest_buf, org_buf, maxlen);
    if (aFld_valunpk[0])
    {
      /*_ufNtoT((char *)(org_buf + ilLen_len),len,(char *)alParm[1]);*/
      _ufNtoT((char *)org_buf,maxlen,(char *)alParm[1]);
      alParm[2][0] = '\0';
      swTranbuf(aFld_valunpk,alParm,alBuf);
      ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
      if (ilRc == FAIL)
      {
        swVdebug(1,"S3462: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
        return(FAIL);
      }   
      memcpy(dest_buf,(unsigned char *)alFldvaltmp,ilRtnlen);
      *reallen = ilRtnlen;
    }
    _ufNtoT((char *)org_buf,maxlen,(char *)org_value);
    *iOrg_len = maxlen;
    return(maxlen);
  }
}

/*******************************************************************/
/* �� �� ��       :  swGetIsoStru                                  */
/* ��    ��       :  8583���                                      */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  1998��12��8��                                 */
/* ����޸�����   :  1998��12��8��                                 */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  dest_buf : 8583��                             */
/*                   org_buf  : Ԥ�����                           */
/*                   df_type  : ������                             */ 
/*                   maxlen   : ����󳤶�                         */
/*                   bcd_flag : BCD,ASCII��־                      */
/*                   reallen  : �򳤶�                             */
/*                   fillflag : BCD����־                        */
/*                   org_len  : ԭʼ���ȴ�                         */
/*                   org_value: ԭʼֵ��                           */
/* �� �� ֵ       :  �������ʵ�ʳ���(�г���λ),�����򷵻�FAIL     */ 
/*******************************************************************/
/* �޸ļ�¼       :                                                */
/*                                                                 */
/*******************************************************************/
static int swGetIsoStru(org_buf, df_type, maxlen,/*bcd_flag*/
		 reallen,/*fillflag*/dest_buf,
		 org_len,org_value,iOrg_len,
		 /*ebcdic*/iLen_len,aFld_lenpack,aFld_valpack)
  unsigned char *org_buf;  /***modified by JLQ****/
  int   df_type; 
  int   maxlen;
/*  char  bcd_flag;  */
  int   reallen;
/*  int   fillflag;  */
  unsigned char *dest_buf;  
  unsigned char *org_len;
  unsigned char *org_value;
  short *iOrg_len;
 /* char  ebcdic; */
  short iLen_len;
  char *aFld_lenpack;
  char *aFld_valpack;
{
  static   unsigned char tmp_buf[512];
  static   unsigned char tmp_buf2[512];
/*  short ilRtnlen = 0;  */ 
  short  ilRtnlen = 0;  /* changed by dyw*/ 

  int ilRc;
  char alBuf[iEXPRESSLEN];
  char  alFldvaltmp[ iPERFLDVALUELEN + 1 ];  
  short ilLen_len;
  unsigned char alParm[3][iEXPRESSLEN];  /* ��š�$1 ,$2��ֵ��*/

  memset(tmp_buf, 0x00, sizeof(tmp_buf));
  memset(tmp_buf2, 0x00, sizeof(tmp_buf2));
  
  if (df_type & EV)  /* �䳤��*/
  {
    if(reallen > maxlen) reallen = maxlen;
    if(reallen < 0) return(FAIL);
    /* �����򳤶�λȡ���򳤶ȡ�*/
    memset(dest_buf,0x00,sizeof(dest_buf));
    sprintf((char *)dest_buf,"%d",reallen);
    ilLen_len = iLen_len;
    if (aFld_lenpack[0])
    {
      _ufNtoT((char *)dest_buf,strlen((char *)dest_buf),(char *)alParm[0]);
      alParm[1][0] = '\0';
      swTranbuf(aFld_lenpack,alParm,alBuf);
      ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
      if (ilRc == FAIL)
      {
        swVdebug(1,"S3464: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
        return(FAIL);
      }   
      memcpy(dest_buf,(unsigned char *)alFldvaltmp,ilRtnlen);  
    }
   
    memcpy(dest_buf + ilLen_len,org_buf,reallen);
    *iOrg_len = reallen;
    if (aFld_valpack[0])
    {
      _ufNtoT((char *)org_buf,reallen,(char *)alParm[1]);
      alParm[2][0] = '\0';
      swTranbuf(aFld_valpack,alParm,alBuf);
      ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
      if (ilRc == FAIL)
      {
        swVdebug(1,"S3466: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
        return(FAIL);
      }         
      memcpy(dest_buf + ilLen_len,(unsigned char *)alFldvaltmp,ilRtnlen);
      *iOrg_len = ilRtnlen;
    }
    _ufNtoT((char *)dest_buf,ilLen_len,(char *)org_len);
    _ufNtoT((char *)dest_buf+ilLen_len,*iOrg_len,(char *)org_value);
    return(ilLen_len + *iOrg_len);
  }
  else  /* ���� */
  {
    if (df_type & EB)
    {
      if (maxlen%8) return(FAIL);
      maxlen = maxlen/8;
    } 
    /* reallen = maxlen; */
    memcpy(dest_buf,org_buf,reallen);
    /* *iOrg_len = maxlen; */
    *iOrg_len = reallen;
    if (aFld_valpack[0] != '\0')
    {
      _ufNtoT((char *)org_buf,reallen,(char *)alParm[1]);
      alParm[2][0] = '\0';
      swTranbuf(aFld_valpack,alParm,alBuf);
      ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
      if (ilRc == FAIL)
      {
        swVdebug(1,"S3468: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
        return(FAIL);
      }     
      memcpy(dest_buf,(unsigned char *)alFldvaltmp,ilRtnlen);
      *iOrg_len = ilRtnlen;
    }
    else
      *iOrg_len = reallen;    

    _ufNtoT((char *)dest_buf,*iOrg_len,(char *)org_value);
    return(*iOrg_len);  
  }
}

/*******************************************************************/
/* �� �� ��       :  swIsdigit                                     */
/* ��    ��       :  �ж��ַ���ǰ����λ�Ƿ����������              */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  1998��10��15��                                */
/* ����޸�����   :  1998��10��15��                                */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  pcString �ַ���                               */
/*                   iLength ����                                  */
/* �� �� ֵ       :  ����,����TRUE , ���򷵻�FALSE                 */
/*******************************************************************/
/* �޸ļ�¼       :                                                */
/*                                                                 */
/*******************************************************************/
/*
int swIsdigit( pcString , iLength )
  char    *pcString;
  int     iLength;
{

  int     iCount;
  int     iFlag;

  iFlag = TRUE ;

  for( iCount = 0 ; iCount < iLength ; iCount ++ )
  {
    if( isdigit( pcString [ iCount ] ) == 0 ) iFlag = FALSE ;
  }

  return(iFlag);
}
*/
/**************************************************************
 ** ������      ��sw8583pack
 ** ��  ��      ��8583���Ĵ��
 ** ��  ��      ��
 ** ��������    ��1999/12/8
 ** ����޸����ڣ�1999/12/8
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��prebuf : FMLԤ��������
                  qid    : �����
                  msgbody: 8583����
                  msglen : ���ĳ���
 ** ����ֵ      ��SUCCESS,FAIL
***************************************************************/
int sw8583pack(prebuf,qid,msgbody,msglen)
  union  preunpackbuf *prebuf;
  short  qid;
  unsigned char  *msgbody;
  unsigned int *msglen;
{
  unsigned char  *plPointer;               /* ƫ��ָ�� */
  unsigned char  alBitmap[17];             /* BITMAP */
  unsigned char  alTmpBitmap[33];          /* ��ʱ���� */
  unsigned char  alTmpbuf[100];            /* ��ʱ���� */
  int   i,k;                             /* ѭ������ */
  short ilFlag;                            /* ��־ */
  unsigned int ilLen;                             /* ���� */
  short ilTableId;                         /* ��id */
  short ilFldId;                           /* ��id */
  short ilRtncode;
  struct swt_sys_queue slSwt_sys_queue;
  struct swt_sys_8583E  slSwt_sys_8583E;
  /*short ilRtnlen; */
  short  ilRtnlen; /*change by dyw*/
  char alValue[iMSGMAXLEN];
  int ilRc;
  char alBuf[iEXPRESSLEN];
  char  alFldvaltmp[ iPERFLDVALUELEN + 1 ];
 /* add by qy 09.29 */
  struct isoprebuf
  {
    short iFldid;
    short iFldlen;
    char  aFldvalue[iFLDVALUELEN + 1];
    char  aOrgfldlen[10];
    short iOrgfldlen; 
    char  aOrgfldvalue[iFLDVALUELEN + 1];
  } slIsobuf;
  char alFldid[129];
 /* end of add by qy */
  unsigned char alParm[3][iEXPRESSLEN];

  swVdebug(3,"S3470:�յ�8583Ԥ�ñ�����...");
  for(i=0;prebuf[i].sIsobuf.iFldid!=-1;i++)
  {
    swVdebug(3,"S3472: �����=[%d],�򳤶�=[%d],��ֵ=[%s]",
      prebuf[i].sIsobuf.iFldid,
      prebuf[i].sIsobuf.iFldlen,
      prebuf[i].sIsobuf.aFldvalue);
  }
 
  plPointer = msgbody;
  swVdebug(3,"S3474: start");
  ilRtncode = swShmselect_swt_sys_queue(qid, &slSwt_sys_queue);
  
  if (ilRtncode != SUCCESS) 
  {
    swVdebug(1,"S3476: [����/��������] swShmselect_swt_sys_queue()����,������=%d qid = %d",ilRtncode,qid);
    return (FAIL);  
  }
  ilTableId=slSwt_sys_queue.e8583tab_id;
  
  ilRtncode = swShmselect_swt_sys_8583E(ilTableId,1,&slSwt_sys_8583E);     
  if (ilRtncode != SUCCESS) 
  {
    swVdebug(1,"S3478: [����/��������] swShmselect_swt_sys_8583E()����,������=%d tab_id = %d fld_id = %d",ilRtncode,ilTableId,1);  	
    return (FAIL);  
  }
    
  memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
  ilRtncode = swIsoget(prebuf,1,&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
 
  /* ��msgtype */
  memset(alTmpbuf,0x00,sizeof(alTmpbuf));
  memcpy(alTmpbuf,(unsigned char *)slIsobuf.aFldvalue,slIsobuf.iFldlen);
  
  if (slSwt_sys_8583E.fld_valpack[0] != '\0')
  {
 /*   swTranbuf(slSwt_sys_8583E.fld_valpack,alTmpbuf,alBuf); */ /* delete by zjj 2002.05.14 for qy suggestion */
 /* add by zjj 2002.05.14 for qy suggestion */
    _ufNtoT((char*)slIsobuf.aFldvalue,slIsobuf.iFldlen,(char *)alParm[1]);
    alParm[2][0] = '\0';
    swTranbuf(slSwt_sys_8583E.fld_valpack,alParm,alBuf);
 /* end add by zjj */
    ilRtncode = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
    if (ilRtncode == FAIL)
    {
      swVdebug(1,"S3480: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRtncode);
      return(FAIL);
    }         	
    if (ilRtnlen != slSwt_sys_8583E.fld_len) return(FAIL);
    memcpy(alTmpbuf,(unsigned char *)alFldvaltmp,ilRtnlen);	
  }
 
  memcpy(plPointer,alTmpbuf,slSwt_sys_8583E.fld_len);
  plPointer += slSwt_sys_8583E.fld_len;
  
  /* ��֯bitmap */
  memset(alBitmap, 0x00, sizeof(alBitmap));
  memset(alFldid, '0', sizeof(alFldid));
  alFldid[128] = '\0';
  ilFlag=1;
  
  for(k=1;;k++)
  {
    if(prebuf[k].sIsobuf.iFldid == -1) break;
    if(prebuf[k].sIsobuf.iFldlen == 0) continue;
    alFldid[prebuf[k].sIsobuf.iFldid] = '1';  
    swSetBitmap((unsigned char *)&alBitmap[0],prebuf[k].sIsobuf.iFldid);
    if(prebuf[k].sIsobuf.iFldid>64) ilFlag=2;
    swVdebug(2,"S3482: sw8583pack():set bitmap id=[%d]",prebuf[k].sIsobuf.iFldid);
  }

  if(ilFlag==2)  /* need extend bitmap */
  {
    alBitmap[0]|=0x80;
    swVdebug(2,"S3484: need extend bitmap...for id=[%d]",prebuf[k-1].sIsobuf.iFldid);
  }
  if(strlen(agMac_create)!=0)
  {
    if(ilFlag==1)
      alBitmap[7]|=0x01;
    else 
      alBitmap[15]|=0x01;
  }
  if (slSwt_sys_queue.bitmap_pack[0])
  {
    _ufNtoT((char *)alBitmap,strlen((char *)alBitmap),(char *)alParm[1]);
    alParm[2][0] = '\0';
    swTranbuf(slSwt_sys_queue.bitmap_pack,alParm,alBuf);
    ilRc = _swExpress(alBuf,(char *)alTmpBitmap,&ilRtnlen);
    if (ilRc == FAIL)
    {
      swVdebug(1,"S3486: [����/��������] _swExpress()����,���������ʽ����[%s],������=%d",alBuf,ilRc);
      return(FAIL);
    }   
    igBitmaplen = ilRtnlen;	
    swVdebug(3,"S3488: The ASC Bitmap is ...");
    if(cgDebug>=2)swDebughex((char *)alTmpBitmap,ilRtnlen);
    swVdebug(3,"S3490:The ASC Bitmap end");    
    memcpy(plPointer,alTmpBitmap,ilRtnlen);
    plPointer += ilRtnlen;    
  }
  else
  {
    memcpy(plPointer,alBitmap,8*ilFlag);
    plPointer +=8*ilFlag;
    igBitmaplen=ilFlag*8;    	
  }
  
  memset(agBitmap,0x00,33);
  memcpy(agBitmap,alTmpBitmap,igBitmaplen);

  /* ��֯8583����,Ӧ�ñ�֤prebuf�ǰ���iFldid���е� */

  for(k=2;alFldid[k]!='\0';k++)
  {
    if (alFldid[k] == '0') continue;   
   
    memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
    
    slIsobuf.iFldid = k;
    ilRtncode = swIsoget(prebuf,k,&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
    
    ilFldId = k;
    if(ilFldId == 0) break;

    ilRtncode = swShmselect_swt_sys_8583E(ilTableId, ilFldId, 
      &slSwt_sys_8583E);
          
    if (ilRtncode != SUCCESS) return (FAIL);    
            
  /*  if(strlen(prebuf[k].sIsobuf.aFldvalue) == 0) */
  /*    continue; */
  swVdebug(2,"S3492: k=[%d],id=[%d],value=[%s],fldtype=[%d],maxlen=[%d],reallen=[%d],fld_len2=[%d],\
     fld_lenunpk=[%s],fld_lenpack=[%s],fld_valunpk=[%s],fld_valpack=[%s]", k,
          ilFldId, 
        slIsobuf.aFldvalue, 
        slSwt_sys_8583E.fld_type, 
        slSwt_sys_8583E.fld_len,
/*        slSwt_sys_queue.tran_type[ilFldId-1] , */
/*        slSwt_sys_8583.fld_attr, */
        slIsobuf.iFldlen,
        slSwt_sys_8583E.fld_len2,
        slSwt_sys_8583E.fld_lenunpk,
        slSwt_sys_8583E.fld_lenpack,
        slSwt_sys_8583E.fld_valunpk,
        slSwt_sys_8583E.fld_valpack);

    ilLen = swGetIsoStru( 
      slIsobuf.aFldvalue, 
      slSwt_sys_8583E.fld_type, 
      slSwt_sys_8583E.fld_len,
/*      slSwt_sys_queue.tran_type[ilFldId-1] , */
      slIsobuf.iFldlen,
/*      slSwt_sys_8583E.fld_attr,  */
      plPointer,
      slIsobuf.aOrgfldlen,
      slIsobuf.aOrgfldvalue,
      &(slIsobuf.iOrgfldlen),
/*      slSwt_sys_queue.code_type[0] */
      slSwt_sys_8583E.fld_len2,slSwt_sys_8583E.fld_lenpack,slSwt_sys_8583E.fld_valpack);
    
    swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
     slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);
     
    swVdebug(3,"S3494:ilLen=%d qid=%d",ilLen,qid);
    swVdebug(3,"S3496:k=%d  iFldlen=%d",k,slIsobuf.iFldlen);
    swVdebug(3,"S3498:after pack. orglen=(%s) orgbuffer=(%s)",
      slIsobuf.aOrgfldlen,slIsobuf.aOrgfldvalue);
    if(ilLen==FAIL)
    {
      swVdebug(1,"S3500: swGetIsoStru fail in field [%d]",
          slIsobuf.iFldid);
      return(FAIL);
    }
    plPointer += ilLen; 
  }

/* need to add mac */

  *msglen=plPointer - msgbody;

  swVdebug(3,"S3502: ��8583���ɹ�,����Ϊ%d",*msglen);
    if(cgDebug>=2)swDebughex((char *)msgbody,*msglen);
  /* ���� macbuf */
  if (slSwt_sys_queue.mac_express[0] != '\0')
  {
/*    memcpy(psgMacneedbuf ,psgUnpackbuf,sizeof(psgMacneedbuf));  */	
    ilRc = _swExpress(slSwt_sys_queue.mac_express,alValue,&ilRtnlen);
    if (ilRc == FAIL)  return(FAIL);
    memcpy(agMacbuf,alValue,ilRtnlen);
    igMacbuflen = ilRtnlen;  
  }
  else
  {
/*    memcpy(psgMacneedbuf ,psgUnpackbuf,sizeof(psgMacneedbuf));    */	
    agMacbuf[0] = '\0';
    igMacbuflen = ilRtnlen;
  }    
  return(SUCCESS);
}
    
/*******************************************************************/
/* �� �� ��       :  swSetBitmap                                   */
/* ��    ��       :  ����bitmap����Ӧλ                            */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  1998��12��8��                                 */
/* ����޸�����   :  1998��12��8��                                 */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  pBitMap bitmap����Ӧ��ָ��                    */ 
/*                   iCount  10��������λ��                        */
/* �� �� ֵ       :  SUCCESS,FAIL                                  */
/*******************************************************************/
/* �޸ļ�¼       :                                                */
/*                                                                 */
/*******************************************************************/
static int swSetBitmap( pBitMap , iCount )
  unsigned char  pBitMap[];
  int   iCount;
{
  if(iCount >=2 && iCount<=64)
  {
    pBitMap[(iCount-1)/8] = pBitMap[(iCount-1)/8] | ( 0x01 << ( 7-(iCount-1) % 8));
    swVdebug(3,"S3504: \n");
      if(cgDebug>=2)swDebughex((char *)pBitMap,16);
    return(SUCCESS);
  }
  else if(iCount >=65 && iCount <=128 )
  {
    iCount -=64;
    pBitMap[(iCount-1)/8+8] = pBitMap[(iCount-1)/8+8] | ( 0x01 <<( 7-(iCount-1) % 8  ));
    swVdebug(3,"S3506: \n");
      if(cgDebug>=2)swDebughex((char *)pBitMap,16);
    return(SUCCESS);
  }
  else
    return(FAIL);
}

/*******************************************************************/
/* �� �� ��       :  swTranbuf                                 */
/* ��    ��       :  �滻�ַ����е�$1,$2                           */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  2002.01.29                                 */
/* ����޸�����   :  2002.01.29                                 */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  aExp ---�����ʽ����aValue --- $��ֵ����       */
/*                  aReturn ---��ת����ı��ʽ��            ����*/ 
/* �� �� ֵ       :  SUCCESS,FAIL                                  */
/*******************************************************************/
static int swTranbuf(char *aExp,char aValue[][iEXPRESSLEN],char *aReturn)
{
  short i,j = 0;

  /* aValueΪ�Ѿ�ת���ַ���*/
  for (i=0;i<strlen(aExp);i++)
  {
    if (aExp[i] == '$' && aExp[i+1] == '1')
    {
      memcpy(aReturn + j,aValue[0],strlen(aValue[0]));
      j += strlen(aValue[0]) ;
      i++;
    }
    else if (aExp[i] == '$' && aExp[i+1] == '2')
    {
      memcpy(aReturn + j,aValue[1],strlen(aValue[1]));
      j += strlen(aValue[1]) ;
      i++;    
    }
    else 
    {
      aReturn[j] = aExp[i];
      j++;
    }	
  }
  aReturn[j] = '\0';
  return(0);
}

