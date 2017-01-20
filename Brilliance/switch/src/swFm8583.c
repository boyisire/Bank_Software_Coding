/****************************************************************/
/* ģ����    ��FM8583                                         */
/* ģ������    ��8583������                                   */
/* ��     ��    ���Ż�                                           */
/* ��������    ��1999/12/7                                      */
/* ����޸����ڣ�2001/3/22                                      */
/* ģ����;    ��8583���Ĵ������ĺ���                         */
/* ��ģ���а������º���������˵����                             */
/*                        (2) int  swIsounpack();               */
/*                        (3) void swBCD2ASC();                 */
/*                        (4) void swASC2BCD();                 */
/*                        (5) int  swBitMapIsOn();              */
/*                        (6) int  swSetIsoStru();              */
/*                        (7) int  swGetIsoStru();              */
/*                        (8) int  swIsdigit();                 */
/*                        (9) int  swIsopack();                 */
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

struct swt_sys_imf *psgSwt_sys_imf;

extern union preunpackbuf psgUnpackbuf[iBUFFLDNUM];

/* ����ԭ�� */
int  swIsounpack();
int  swIsopack();
void swBCD2ASC();
void swASC2BCD();
int  swBitMapIsOn();
int  swSetBitmap();
/*int  swIsdigit();  */
int  swSetIsoStru();
int  swGetIsoStru();

#define B2D(x)  (x/16*10+x%16)
#define D2B(x)  (x/10*16+x%10)


enum FillType
{
	LEFTZERO=1,
	    RIGHTZERO,
	    LEFTSPACE,
	    RIGHTSPACE
};

/**************************************************************
 ** ������      ��swIsounpack
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
int swIsounpack(msgbody,msglen,qid,prebuf)
unsigned char *msgbody;
unsigned int msglen;
short qid;
union preunpackbuf *prebuf;
{
	unsigned char  *plPointer;               /* ƫ��ָ�� */
	unsigned char  alBitmap[17];             /* BITMAP */
	unsigned char  alTmpBitmap[17];          /* ��ʱ���� */
	unsigned char  alTmpBCD[iFLDVALUELEN];   /* BCD����ʱ���� */
	int   i;                                 /* ѭ������,8583id���±� */
	int   k;                                 /* ѭ������,prebuf���±� */
	short ilFlag;                            /* ��־ */
	unsigned int ilLen;                             /* ���� */
	short ilTableId;                         /* ��id */
	short ilRtncode;
	struct swt_sys_queue slSwt_sys_queue;
	struct swt_sys_8583  slSwt_sys_8583;
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
	slIsobuf.iFldlen   = 4;

	ilRtncode = swShmselect_swt_sys_queue(qid, &slSwt_sys_queue);

	if (ilRtncode != SUCCESS)
		return (FAIL);

	if (slSwt_sys_queue.tran_type[0] == '0') /* ASCII msgtype */
	{
		if (slSwt_sys_queue.code_type[0] == '0')  /* ASCII �� */
		{
			memset(slIsobuf.aFldvalue , 0x00,sizeof(slIsobuf.aFldvalue) );
			memcpy(slIsobuf.aFldvalue , plPointer ,4 );
			memset(slIsobuf.aOrgfldvalue , 0x00, sizeof(slIsobuf.aOrgfldvalue) );
			memcpy(slIsobuf.aOrgfldvalue , plPointer ,4 );
			plPointer += 4;
			swVdebug(2,"S3332: msgtype is %4.4s",slIsobuf.aFldvalue);
		}
		else /* EBCDIC�� */
		{
			memset(slIsobuf.aFldvalue , 0x00, sizeof(slIsobuf.aFldvalue) );

			swVdebug(3,"S3334: before etoa [%4.4s]",plPointer);
			if(cgDebug>=2)
				swDebughex((char *)plPointer,4);

			_swEtoA((unsigned char *)slIsobuf.aFldvalue,(unsigned char *)plPointer,4);
			memset(slIsobuf.aOrgfldvalue , 0x00, 
			    sizeof(slIsobuf.aOrgfldvalue) );
			memcpy(slIsobuf.aOrgfldvalue , plPointer ,4 );
			plPointer += 4;
			swVdebug(2,"S3336: msgtype is %4.4s",slIsobuf.aFldvalue);
		}
	}
	else /* BCD msgtype */
	{
		memset(slIsobuf.aFldvalue , 0x00, 
		    sizeof(slIsobuf.aFldvalue) );
		memcpy(alTmpBCD , plPointer , 2 );
		memset(slIsobuf.aOrgfldvalue , 0x00, 
		    sizeof(slIsobuf.aOrgfldvalue) );
		_ufNtoT((char *)alTmpBCD,2,(char *)slIsobuf.aOrgfldvalue );
		swBCD2ASC( alTmpBCD , slIsobuf.aFldvalue , 4 , LEFTZERO );
		plPointer += 2;
		swVdebug(2,"S3338: msgtype is %4.4s",slIsobuf.aFldvalue);
	}

	/* �ж�msgtype�Ϸ��� */
	/*
  if(swIsdigit(slIsobuf.aFldvalue,4)==FALSE)
  {
    swVdebug(0,"S3340: [����/��������] swIsdidit()����,msgtype format error");
      return(FAIL);
  } */ /* delete by zjj 2002.05.14 for qy suggestion */

	swVdebug(2,"S3342: OUT msgtype=%s",slIsobuf.aFldvalue);

	/* ��ISOmsgtype�� */
	swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
	    slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);

	memset(agBitmap,0x00,33);
	memcpy(agBitmap,plPointer,32);
	swVdebug(2,"S3344: bitmap ag is ...");
	if(cgDebug>=1) swDebughex((char *)agBitmap,32);

	if (slSwt_sys_queue.bitmap_type[0]=='1')  /* BCD Bitmap */
	{
		/* ����bitmap */
		memcpy(alBitmap, plPointer, 8);
		plPointer  += 8;
		ilFlag=1;
		igBitmaplen=8;

		/* ����Ƿ�����չbitmap */
		if( alBitmap[0] & 0x80 )
		{
			memcpy(alBitmap+8, plPointer, 8);
			plPointer  += 8;
			ilFlag=2;
			igBitmaplen=16;
		}
	}
	else /* ASC BITMAP */
	{
		/* ����bitmap */
		memcpy(alTmpBitmap, plPointer, 16);
		/* �����EBCDIC��,����Ҫ��������ת�� */
		swASC2BCD( alTmpBitmap, alBitmap , 16 , RIGHTZERO );
		plPointer  += 16;
		ilFlag=1;
		igBitmaplen=16;

		/* ����Ƿ�����չbitmap */
		if( alBitmap[0] & 0x80 )
		{
			memcpy(alTmpBitmap, plPointer, 16);
			swASC2BCD( alTmpBitmap, alBitmap+8 , 16 , RIGHTZERO );
			plPointer  += 16;
			ilFlag=2;
			igBitmaplen=32;
		}
	}

	if(cgDebug>=2)
		swDebughex((char *)alBitmap,ilFlag * 8 );
	/* check bitmap , Ӧ���ڽ��pcode֮��*/
	/* 
  if(swCheckBitmap(StandardBitmap, alBitmap)==FALSE)
  {
    swVdebug(0,"S3346: [����/��������] swCheckBitmap()����,bitmap not fit standard bitmap");
    return(FAIL);
  }
  */
	swVdebug(2,"S3348: Stanford:�����=[%d],�򳤶�=[%d],��ֵ=[%s]",
	    slIsobuf.iFldid,
	    slIsobuf.iFldlen,
	    slIsobuf.aFldvalue);

	k=1;
	ilTableId=slSwt_sys_queue.isotab_id;
	for(i=2;i<=64*ilFlag;i++)
	{
		if(swBitMapIsOn(alBitmap,i)==TRUE)
		{
			memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));

			slIsobuf.iFldid = i;

			ilRtncode = swShmselect_swt_sys_8583(ilTableId, i, 
			    &slSwt_sys_8583);

			if (ilRtncode != SUCCESS)
				return (FAIL);

			swVdebug(2,"S3350: ANN:k=[%d],id=[%d],fldtype=[%d],maxlen=[%d],fill=[%d],trantype=[%c]",k,i,
			    slSwt_sys_8583.fld_type, 
			    slSwt_sys_8583.fld_len,
			    slSwt_sys_8583.fld_attr,
			    slSwt_sys_queue.tran_type[i-1]);

			ilLen = swSetIsoStru( 
			    plPointer,
			    slSwt_sys_8583.fld_type, 
			    slSwt_sys_8583.fld_len,
			    slSwt_sys_queue.tran_type[i-1],
			    slSwt_sys_8583.fld_attr,
			    slIsobuf.aFldvalue, 
			    &(slIsobuf.iFldlen),
			    slIsobuf.aOrgfldlen,
			    slIsobuf.aOrgfldvalue,
			    &(slIsobuf.iOrgfldlen),
			    slSwt_sys_queue.code_type[0]
			    );
			/* ��ISO�� */
			swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
			    slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);

			if(ilLen==FAIL)
			{
				swVdebug(1,"S3352: [����/��������] swIsounpack()����,��8583��ʧ��,�⵽��[%d]����",i);
				return(FAIL);
			}
			swVdebug(2,"S3354: �����򳤶�=%d",slIsobuf.iFldlen);
			swVdebug(3,"��ֵ=...");
			if (cgDebug>=2)
				swDebughex(slIsobuf.aFldvalue,slIsobuf.iFldlen);
			k++;
			plPointer += ilLen;
			if(plPointer - msgbody > msglen)
			{
				swVdebug(1,"S3356: ���ĳ��ȳ���!");
				return(FAIL);
			}
		}
	}

	swVdebug(3,"S3358: ��8583�ɹ�,��������...");
	for(i=0;prebuf[i].sIsobuf.iFldid!=-1;i++)
	{
		swVdebug(3,"S3360:id=[%d],len=[%d],value=[%s],orglen[%s]orgvalue[%d][%s]",
		    prebuf[i].sIsobuf.iFldid,
		    prebuf[i].sIsobuf.iFldlen,
		    prebuf[i].sIsobuf.aFldvalue,
		    prebuf[i].sIsobuf.aOrgfldlen,
		    prebuf[i].sIsobuf.iOrgfldlen,
		    prebuf[i].sIsobuf.aOrgfldvalue
		    );
	}

	return(SUCCESS);
}

/*******************************************************************/
/* �� �� ��       :  swBCD2ASC                                     */
/* ��    ��       :  ��һѹ����BCD��ת��ΪASCII���ִ�(0-9A-F)      */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  1998��10��12��                                */
/* ����޸�����   :  1998��10��12��                                */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  pcBCD BCD��                                   */
/*                   iLength 10��������λ��                        */
/*                   iCutFlag ����λʱ��ȡ��ʽ                     */
/*                   1:��ȥ0 2:��ȥ0 3:��ȥ�ո� 4:��ȥ�ո�         */
/* �� �� ֵ       :  pcASC ASCII�ַ���                             */
/*******************************************************************/
/* �޸ļ�¼       :                                                */
/*                                                                 */
/*******************************************************************/
void swBCD2ASC( pcBCD , pcASC , iLength , iCutFlag )
unsigned char pcBCD[];
unsigned char pcASC[];
int  iLength;
int  iCutFlag;
{
	int i;

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
	return;
}

void swBCD2ASC2( pcBCD , pcASC , iLength , iCutFlag )
unsigned char pcBCD[];
unsigned char pcASC[];
int  iLength;
int  iCutFlag;
{
	int i;

	if(iLength%2==0 || (iLength%2==1 && iCutFlag%2==0))
	{
		for(i=0; i<iLength; i++)
		{
			pcASC[i*2] = (unsigned char)pcBCD[i]/16+'0' ;
			/*if(pcASC[i*2] > '9') pcASC[i*2] = (unsigned char)pcASC[i*2] + 7;*/
			pcASC[i*2+1] = (unsigned char)pcBCD[i]%16+'0';
			/* if(pcASC[i*2+1] > '9') pcASC[i*2+1] = (unsigned char)pcASC[i*2+1] + 7; */
		}
		pcASC[iLength] = '\0';
	}
	else 
	{
		pcASC[0] = (unsigned char)pcBCD[0]%16+'0';
		for(i=1; i<iLength; i++ )
		{
			pcASC[i*2-1] = (unsigned char)pcBCD[i]/16+'0';
			/* if(pcASC[i*2-1] > '9') pcASC[i*2-1] = (unsigned char)pcASC[i*2-1] + 7; */
			pcASC[i*2] = (unsigned char)pcBCD[i]%16+'0';
			/* if(pcASC[i*2] > '9') pcASC[i*2] = (unsigned char)pcASC[i*2] + 7; */
		}
		pcASC[iLength] = '\0';
	}
	return;
}

/*******************************************************************/
/* �� �� ��       :  swASC2BCD                                     */
/* ��    ��       :  ��һASCII���ִ�ת��Ϊѹ����BCD��              */
/* ��    ��       :  �Ż�                                          */
/* ��������       :  1998��10��12��                                */
/* ����޸�����   :  1998��10��12��                                */
/* ������������   :                                                */
/* ȫ�ֱ���       :                                                */
/* ��������       :  pcASC ASCII�ַ���                             */
/*                   iLength 10��������λ��                        */
/*                   iCutFlag ����λʱ��0��ʽ                      */
/*                   1:���0 2:�Ҽ�0                               */
/* �� �� ֵ       :  pcBCD BCD��                                   */
/*******************************************************************/
/* �޸ļ�¼       :                                                */
/*                                                                 */
/*******************************************************************/
void swASC2BCD1( pcASC , pcBCD , iLength , iCutFlag )
unsigned char pcASC[];
unsigned char pcBCD[];
int  iLength;
int  iCutFlag;
{
	int i, j;

	/* ���λ��������, ��iCutFlag=1, ǰ�߲�0 */
	if( iLength % 2 == 1 && iCutFlag == LEFTZERO )
	{
		pcBCD[0] = pcASC[0] & 0x0f;
		i = 1;
		j = 1;
	}
	else 
	{
		i = 0;
		j = 0;
	}

	for( ; i < ( iLength + 1 ) / 2; i++, j += 2 )
	{
		pcBCD[i] =((pcASC[j] & 0x0f)<<4)|(pcASC[j+1]&0x0f);
		/*    pcBCD[i] += pcASC[ j + 1 ] & 0x0f; */
	}
}

void swASC2BCD( unsigned char *InBuf, unsigned char  *OutBuf, int Len ,int ActiveNibble)
{
	int      rc;          /* Return Value */
	char     CharIn;      /* Character from source buffer */
	u_char   CharOut;      /* Character from target buffer */
	rc = 0;                 /* Assume everything OK. */
	/* ActiveNibble = 0; */     /* Set Most Sign Nibble (MSN) */
	/* to Active Nibble. */

	/* add by zhanghui 2001/4/4 */
	memset(OutBuf,0x00,(Len+1)/2);
	/* add by zhanghui 2001/4/4 */

	if(Len % 2 == 1)
	{
		/* delete by zhanghui 2001/4/3 BUG
    ActiveNibble = ( ActiveNibble == LEFTZERO||ActiveNibble==LEFTSPACE )? 0 : 1;
  * delete by zhanghui 2001/4/3 BUG */
		/* add by zhanghui 2001/4/3 FIXBUG */
		ActiveNibble = ( ActiveNibble == LEFTZERO||ActiveNibble==LEFTSPACE )? 1 : 0;
		/* add by zhanghui 2001/4/3 FIXBUG */
	}
	else
		ActiveNibble = 0;

	for ( ; (Len > 0); Len--, InBuf++ )
	{
		CharIn = *InBuf;

		if ( !isxdigit ( CharIn ) )    /* validate character */
		{
			rc = -1;
		}
		else
		{
			if ( CharIn > '9')
			{
				CharIn += 9;      /* Adjust Nibble for A-F */
			}
		}

		if ( rc == 0 )
		{
			CharOut = *OutBuf;
			if ( ActiveNibble )
			{
				*OutBuf++ = (unsigned char)( ( CharOut & 0xF0) | ( CharIn  & 0x0F) );
			}
			else
			{
				*OutBuf = (unsigned char)( ( CharOut & 0x0F) | ( (CharIn & 0x0F) << 4) );
			}
			ActiveNibble ^= 1;      /* Change Active Nibble */
		}
	}

	return;
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
int swBitMapIsOn( pBitMap , iCount )
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
int swSetIsoStru(org_buf,df_type,maxlen, bcd_flag,fillflag,
dest_buf,reallen,
org_len,org_value,iOrg_len,
ebcdic)
unsigned char *org_buf;
int   df_type;
int   maxlen;
char  bcd_flag;
int   fillflag;
unsigned char  *dest_buf;
short *reallen;
unsigned char  *org_len;
short *iOrg_len;
unsigned char  *org_value;
char  ebcdic;
{
	int     len;
	static   char  tmp_buf[512];

	memset(tmp_buf, 0x00, sizeof(tmp_buf));
	if(ebcdic == '1')   /* EBCDIC���ת�� */
	{
		/* ����ֻ����bcd_flag=ASCASC�����,
       ��ʱ����BCDBCD,BCDASC���д��� 
       ���Ӷ�ASCBCD�Ĵ���*/
		if(bcd_flag != cASCASC && bcd_flag !=cASCBCD )
		{
			swVdebug(1,"S3362: [����/����] EBCDIC���bcd_flagֻ����'0' or '3'");
			return(FAIL);
		}

		switch(df_type)
		{
		case Efb:
			if(maxlen%8)
				return(FAIL);
			*reallen=maxlen/8;
			memcpy(dest_buf, org_buf, maxlen/8);
			_ufNtoT((char *)org_buf,maxlen/8,(char *)org_value);
			*iOrg_len = maxlen/8;
			return(maxlen/8);

		case Efn:
		case Efxn:
		case Efan:
		case Efans:
			*reallen=maxlen;
			memcpy(tmp_buf, org_buf, maxlen);
			_swEtoA((unsigned char *)dest_buf,(unsigned char *)tmp_buf,maxlen);
			_ufNtoT((char *)org_buf,maxlen,(char *)org_value);
			*iOrg_len = maxlen;
			return(maxlen);

		case Eln:
		case Elans:
			_swEtoA((unsigned char *)tmp_buf,(unsigned char *)org_buf,1);
			len = (tmp_buf[0]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(tmp_buf, org_buf+1, len);
			/* ���Ӷ�bcd_flag=cASCASC �Ĵ���,2000/07/02 */
			if ( bcd_flag == cASCBCD)
				memcpy(dest_buf,tmp_buf,len);
			else
				_swEtoA((unsigned char *)dest_buf,(unsigned char *)tmp_buf,len);
			_ufNtoT((char *)org_buf,1,(char *)org_len);
			_ufNtoT((char *)org_buf+1,len,(char *)org_value);
			*iOrg_len = len;
			return(len+1);

		case Elln: /* added 2000/3/8 */
		case Ellz:
		case Ellan:
		case Ellans:
			_swEtoA((unsigned char *)tmp_buf,(unsigned char *)org_buf,2);
			len = (tmp_buf[0]-'0');
			len = len * 10 + (tmp_buf[1]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(tmp_buf, org_buf+2, len);
			/* ���Ӷ�bcd_flag=cASCBCD �Ĵ���,2000/07/02 */
			if (bcd_flag == cASCBCD)
				memcpy(dest_buf,tmp_buf,len);
			else
				_swEtoA((unsigned char *)dest_buf,(unsigned char *)tmp_buf,len);
			_ufNtoT((char *)org_buf,2,(char *)org_len);
			_ufNtoT((char *)org_buf+2,len,(char *)org_value);
			*iOrg_len = len;
			return(len+2);

		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			_swEtoA((unsigned char *)tmp_buf,(unsigned char *)org_buf,3);
			len = (tmp_buf[0]-'0');
			len = len * 10 + (tmp_buf[1]-'0');
			len = len * 10 + (tmp_buf[2]-'0');
			if(len<0|| len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(tmp_buf, org_buf+3, len);
			/* ���Ӷ�bcd_flag=cASCBCD �Ĵ���,2000/07/02 */
			if (bcd_flag == cASCBCD)
				memcpy(dest_buf,tmp_buf,len);
			else
				_swEtoA((unsigned char *)dest_buf,(unsigned char *)tmp_buf,len);
			_ufNtoT((char *)org_buf,3,(char *)org_len);
			_ufNtoT((char *)org_buf+3,len,(char *)org_value);
			*iOrg_len = len;
			return(len+3);

			/* add by qy 2001.09.19 */

		case Elllln: /* added 2000/3/8 */
		case Ellllz:
		case Ellllan:
		case Ellllans:
			_swEtoA((unsigned char *)tmp_buf,(unsigned char *)org_buf,4);
			len = (tmp_buf[0]-'0');
			len = len * 10 + (tmp_buf[1]-'0');
			len = len * 10 + (tmp_buf[2]-'0');
			len = len * 10 + (tmp_buf[3]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(tmp_buf, org_buf+4, len);
			/* ���Ӷ�bcd_flag=cASCBCD �Ĵ���,2000/07/02 */
			if (bcd_flag == cASCBCD)
				memcpy(dest_buf,tmp_buf,len);
			else
				_swEtoA((unsigned char *)dest_buf,(unsigned char *)tmp_buf,len);
			_ufNtoT((char *)org_buf,4,(char *)org_len);
			_ufNtoT((char *)org_buf+4,len,(char *)org_value);
			*iOrg_len = len;
			return(len+4);

			/* end of add by qy 2001.09.19 */

		default:
			return(FAIL);
		}
	}  /* end of ebcdic */

	/* the following is process ASCII */

	/* �����Ǵ���EBCDIC������,���ڿ��ܲ�ȫ 2001/3/22 */
	/* �����Ǵ���ASCII������,
     ��Ϊbcd_flag=cBCDBCD,cASCASC,cBCDASC,cASCBCD������� */
	if  ( bcd_flag==cBCDBCD)
	{
		switch(df_type)
		{
		case Efb:
			if(maxlen%8)
				return(FAIL);
			*reallen=maxlen/8;
			memcpy(dest_buf, org_buf, maxlen/8);
			_ufNtoT((char *)org_buf,maxlen/8,(char *)org_value);
			*iOrg_len = maxlen/8;
			return(maxlen/8);

		case Efn:
		case Efan:
			/* =========== delete by qy 2001.08.20 ============= 
      case Efxn:
   =========== end of delete by qy 2001.08.20 ====== */
			*reallen=maxlen;
			swBCD2ASC( org_buf , dest_buf , maxlen , fillflag);
			_ufNtoT((char *)org_buf,(maxlen+1)/2,(char *)org_value);
			*iOrg_len = (maxlen+1)/2;
			return((maxlen+1)/2);

			/* =========== add by qy 2001.08.20 ============= */
		case Efxn:
			*reallen=maxlen;
			memcpy(dest_buf, org_buf, 1);
			if (dest_buf[0] != 'C' && dest_buf[0] != 'D')
			{
				swVdebug(1,"S3364: [����/����]Efxn���͵ĵ�һλӦΪ[C]��[D]");
				return(FAIL);
			}
			swBCD2ASC(org_buf+1 , dest_buf+1 , maxlen-1 , fillflag);
			_ufNtoT((char *)org_buf,maxlen/2+1,(char *)org_value);
			*iOrg_len = maxlen/2+1;
			return(maxlen/2+1);
			/* =========== end of add by qy 2001.08.20 ====== */

		case Eln:
		case Elln:
		case Ellz:
		case Ellan:
		case Ellans:

			len = B2D(org_buf[0]);
			*reallen=len;
			swBCD2ASC2( org_buf+1, dest_buf, len , fillflag);
			_ufNtoT((char *)org_buf,1,(char *)org_len);
			_ufNtoT((char *)org_buf+1,(len+1)/2+1,(char *)org_value);
			*iOrg_len = (len+1)/2+1;
			return((len+1)/2+1);

			/* modify by nh 20070622    
      case Ellln:
      case Elllz:
      */
		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			len = B2D(org_buf[0]);
			len = len * 100 + B2D(org_buf[1]);
			*reallen=len;
			swBCD2ASC2( org_buf+2, dest_buf, len , fillflag);
			_ufNtoT((char *)org_buf,2,(char *)org_len);
			_ufNtoT((char *)org_buf+2,(len+1)/2+2,(char *)org_value);
			*iOrg_len = (len+1)/2+2;
			return((len+1)/2+2);

		default:
			return(FAIL);
		}
	} /*  end of BCDBCD */
	else if( bcd_flag==cASCASC) /* ASCII�� */
	{
		switch(df_type) {
		case Efb:   /**added by JLQ**/
			if(maxlen%8)
				return(FAIL);
			*reallen=maxlen/8;
			memcpy(dest_buf, org_buf, maxlen/8);
			_ufNtoT((char *)org_buf,maxlen/8,(char *)org_value);
			*iOrg_len = maxlen/8;
			return(maxlen/8);

		case Efn:
		case Efxn:
		case Efan:
		case Efans:
			*reallen=maxlen;
			memcpy(dest_buf, org_buf, maxlen);
			_ufNtoT((char *)org_buf,maxlen,(char *)org_value);
			*iOrg_len = maxlen;
			return(maxlen);

		case Eln:
		case Elans:
			len = (org_buf[0]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(dest_buf, org_buf+1, len);
			_ufNtoT((char *)org_buf,1,(char *)org_len);
			_ufNtoT((char *)org_buf+1,len,(char *)org_value);
			*iOrg_len = len;
			return(len+1);

		case Elln: /* added 2000/3/8 */
		case Ellz:
		case Ellan:
		case Ellans:
			len = (org_buf[0]-'0');
			len = len * 10 + (org_buf[1]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(dest_buf, org_buf+2, len);
			_ufNtoT((char *)org_buf,2,(char *)org_len);
			_ufNtoT((char *)org_buf+2,len,(char *)org_value);
			*iOrg_len = len;
			return(len+2);

		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			len = (org_buf[0]-'0');
			len = len * 10 + (org_buf[1]-'0');
			len = len * 10 + (org_buf[2]-'0');
			if(len<0|| len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(dest_buf, org_buf+3, len);
			_ufNtoT((char *)org_buf,3,(char *)org_len);
			_ufNtoT((char *)org_buf+3,len,(char *)org_value);
			*iOrg_len = len;
			return(len+3);

		default:
			return(FAIL);
		}
	} /* end of ASCASC */
	/* add by zhanghui 2001/3/21 */
	else if( bcd_flag==cBCDASC)
	{
		switch(df_type)
		{
		case Efb:
			if(maxlen%8)
				return(FAIL);
			*reallen=maxlen/8;
			memcpy(dest_buf, org_buf, maxlen/8);
			_ufNtoT((char *)org_buf,maxlen/8,(char *)org_value);
			*iOrg_len = maxlen/8;
			return(maxlen/8);

		case Efn:
		case Efxn:
		case Efan:
		case Efans:
			*reallen=maxlen;
			memcpy(dest_buf, org_buf, maxlen);
			_ufNtoT((char *)org_buf,maxlen,(char *)org_value);
			*iOrg_len = maxlen;
			return(maxlen);

		case Eln:
		case Elans:
			len = (org_buf[0]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;
			memcpy(dest_buf, org_buf+1, len);
			_ufNtoT((char *)org_buf,1,(char *)org_len);
			_ufNtoT((char *)org_buf+1,len,(char *)org_value);
			*iOrg_len = len;
			return(len+1);

		case Elln: /* added 2000/3/8 */
		case Ellz:
		case Ellan:
		case Ellans:
			len = B2D(org_buf[0]);
			*reallen=len;
			memcpy(dest_buf, org_buf+1, len);
			_ufNtoT((char *)org_buf,1,(char *)org_len);
			_ufNtoT((char *)org_buf+1,len,(char *)org_value);
			*iOrg_len = len;
			return(len+1);

		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			len = B2D(org_buf[0]);
			len = len * 100 + B2D(org_buf[1]);
			*reallen=len;
			memcpy(dest_buf, org_buf+2, len);
			_ufNtoT((char *)org_buf,2,(char *)org_len);
			_ufNtoT((char *)org_buf+2,len,(char *)org_value);
			*iOrg_len = len;
			return(len+2);

		default:
			return(FAIL);
		}
	} /* end of BCDASC */
	else /* ASCBCD */
	{
		switch(df_type) {
		case Efb:
			if(maxlen%8)
				return(FAIL);
			*reallen=maxlen/8;
			memcpy(dest_buf, org_buf, maxlen/8);
			_ufNtoT((char *)org_buf,maxlen/8,(char *)org_value);
			*iOrg_len = maxlen/8;
			return(maxlen/8);

		case Efn:
		case Efxn:
			*reallen=maxlen;
			swBCD2ASC( org_buf , dest_buf , maxlen , fillflag);
			_ufNtoT((char *)org_buf,(maxlen+1)/2,(char *)org_value);
			*iOrg_len = (maxlen+1)/2;
			return((maxlen+1)/2);

		case Efan:
		case Efans:
			*reallen=maxlen;
			memcpy(dest_buf, org_buf, maxlen);
			_ufNtoT((char *)org_buf,maxlen,(char *)org_value);
			*iOrg_len = maxlen;
			return(maxlen);

		case Eln:
		case Elans:  /*����1λASC,��ֵBCD */
			len = (org_buf[0]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;

			swBCD2ASC( org_buf+1, dest_buf, len , fillflag);
			_ufNtoT((char *)org_buf,1,(char *)org_len); /*ԭʼ����*/
			_ufNtoT((char *)org_buf+1,(len+1)/2,(char *)org_value); /*ԭʼ��ֵ*/
			*iOrg_len = (len+1)/2;
			return((len+1)/2+1); /*����8583���ĳ���*/

		case Elln:
		case Ellz:
		case Ellan:
		case Ellans: /*������λASC,ֵBCD */
			len = (org_buf[0]-'0');
			len = len * 10 + (org_buf[1]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;

			swBCD2ASC( org_buf+2, dest_buf, len , fillflag);
			_ufNtoT((char *)org_buf,2,(char *)org_len); /*ԭʼ����*/
			_ufNtoT((char *)org_buf+2,(len+1)/2,(char *)org_value);  /*ԭʼ��ֵ */
			*iOrg_len = (len+1)/2;
			return((len+1)/2+2);  /*����8583���ĳ���*/

		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			len = (org_buf[0]-'0');
			len = len * 10 + (org_buf[1]-'0');
			len = len * 10 + (org_buf[2]-'0');
			if(len<0|| len>maxlen)
				return(FAIL);
			*reallen=len;

			swBCD2ASC( org_buf+3, dest_buf, len , fillflag);
			_ufNtoT((char *)org_buf,3,(char *)org_len);  /*ԭʼ����*/
			_ufNtoT((char *)org_buf+3,(len+1)/2,(char *)org_value);  /*ԭʼ��ֵ*/
			*iOrg_len = (len+1)/2;
			return((len+1)/2+3);  /*����8583���ĳ���*/

		default:
			return(FAIL);
		}
	} /* end of ASCBCD */
	/* add by zhanghui 2001/3/21 */
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
int swGetIsoStru(org_buf, df_type, maxlen,bcd_flag,
reallen,fillflag,dest_buf,
org_len,org_value,iOrg_len,
ebcdic)
unsigned char *org_buf;  /***modified by JLQ****/
int   df_type;
int   maxlen;
char  bcd_flag;
int   reallen;
int   fillflag;
unsigned char *dest_buf;
unsigned char *org_len;
unsigned char *org_value;
short *iOrg_len;
char  ebcdic;
{
	register int    i;
	static   unsigned char tmp_buf[512];
	static   unsigned char tmp_buf2[512];

	memset(tmp_buf, 0x00, sizeof(tmp_buf));
	memset(tmp_buf2, 0x00, sizeof(tmp_buf2));

	if(ebcdic =='1')   /* ����ΪEBCDIC */
	{
		/* ����ֻ����ASCII����� */
		switch(df_type)
		{
		case Efb:
			if(maxlen%8)
				return(FAIL);
			reallen=maxlen/8;
			memcpy(dest_buf, org_buf,reallen);
			_ufNtoT((char *)dest_buf,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(maxlen/8);

		case Efn:
		case Efxn:
		case Efan:
		case Efans:
			if (reallen>=maxlen)
			{
				reallen=maxlen;
				memcpy(tmp_buf,org_buf,reallen);
			}
			else
			{
				if(fillflag==LEFTZERO)
				{
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i]='0';
					}
					memcpy(tmp_buf+maxlen-reallen,org_buf,reallen);
					swVdebug(3,"S3366: 1maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else if(fillflag ==RIGHTZERO)
				{
					memcpy(tmp_buf,org_buf,reallen);
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i+reallen]='0';
					}
					swVdebug(3,"S3368: 2maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else if(fillflag == LEFTSPACE)
				{
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i]=' ';
					}
					memcpy(tmp_buf+maxlen-reallen,org_buf,reallen);
					swVdebug(3,"S3370: 1maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else
				{
					memcpy(tmp_buf,org_buf,reallen);
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i+reallen]=' ';
					}
					swVdebug(3,"S3372: 2maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
			}
			reallen=maxlen;
			_swAtoE((unsigned char *)dest_buf,(unsigned char *)tmp_buf,maxlen);
			_ufNtoT((char *)dest_buf,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(reallen);

		case Elans:
		case Eln:
			if(reallen>maxlen)reallen=maxlen;
			if (reallen<0)
				return(FAIL);
			tmp_buf[0]=reallen+'0';
			_swAtoE((unsigned char *)dest_buf,(unsigned char *)tmp_buf,1);
			/* ���Ӷ�bcd_flag=cASCBCD �Ĵ���,2000/07/02 */
			if (bcd_flag == cASCBCD)
				memcpy(dest_buf+1,org_buf,reallen);
			else
				_swAtoE((unsigned char *)(dest_buf+1),(unsigned char *)org_buf,reallen);
			_ufNtoT((char *)dest_buf,1,(char *)org_len);
			_ufNtoT((char *)dest_buf+1,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(reallen+1);
			break;

		case Elln:
		case Ellan:
		case Ellans:
		case Ellz:
			if(reallen>maxlen)reallen=maxlen;
			if(reallen<0)
				return(FAIL);
			tmp_buf[0]=reallen/10+'0';
			tmp_buf[1]=reallen%10+'0';
			_swAtoE((unsigned char *)dest_buf,(unsigned char *)tmp_buf,2);
			/* ���Ӷ�bcd_flag=cASCBCD �Ĵ���,2000/07/02 */
			if (bcd_flag == cASCBCD)
				memcpy(dest_buf+2,org_buf,reallen);
			else
				_swAtoE((unsigned char *)(dest_buf+2),(unsigned char *)org_buf,reallen);
			_ufNtoT((char *)dest_buf,2,(char *)org_len);
			_ufNtoT((char *)dest_buf+2,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(reallen+2);

		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			if(reallen>maxlen)reallen=maxlen;
			if(reallen<0)
				return(FAIL);
			tmp_buf[0]=reallen/100+'0';
			tmp_buf[1]=(reallen-reallen/100*100)/10+'0';
			tmp_buf[2]=reallen%10+'0';
			_swAtoE((unsigned char *)dest_buf,(unsigned char *)tmp_buf,3);
			/* ���Ӷ�bcd_flag=cASCBCD �Ĵ���,2000/07/02 */
			if (bcd_flag == cASCBCD)
				memcpy(dest_buf+3,org_buf,reallen);
			else
				_swAtoE((unsigned char *)(dest_buf+3),(unsigned char *)org_buf,reallen);
			_ufNtoT((char *)dest_buf,3,(char *)org_len);
			_ufNtoT((char *)dest_buf+3,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(reallen+3);

			/* add by qy 2001.09.19 */

		case Elllln:
		case Ellllan:
		case Ellllans:
		case Ellllz:
			if(reallen>maxlen)reallen=maxlen;
			if(reallen<0)
				return(FAIL);
			tmp_buf[0]=reallen/1000+'0';
			tmp_buf[1]=(reallen-reallen/1000*1000)/100+'0';
			tmp_buf[2]=(reallen-reallen/100*100)/10+'0';
			tmp_buf[3]=(reallen-reallen/100*100)%10+'0';
			_swAtoE((unsigned char *)dest_buf,(unsigned char *)tmp_buf,4);
			/* ���Ӷ�bcd_flag=cASCBCD �Ĵ���,2000/07/02 */
			if (bcd_flag == cASCBCD)
				memcpy(dest_buf+4,org_buf,reallen);
			else
				_swAtoE((unsigned char *)(dest_buf+4),(unsigned char *)org_buf,reallen);
			_ufNtoT((char *)dest_buf,4,(char *)org_len);
			_ufNtoT((char *)dest_buf+4,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(reallen+4);

			/* end of add by qy 2001.09.19 */

		default:
			return(FAIL);
		}
	} /* end of ebcdic */

	/* the following is process ASCII */

	if (bcd_flag==cBCDBCD)  /* ���Ⱥ�ֵ��ΪBCD */
	{
		switch(df_type)
		{
		case Efb:   /***added by JLQ***/
			if(maxlen%8)
				return(FAIL);
			reallen=maxlen/8;
			memcpy(dest_buf, org_buf,reallen);
			_ufNtoT((char *)dest_buf,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(maxlen/8);

		case Efn:
		case Efan:
		case Efans:
			if (reallen>=maxlen)
			{
				reallen=maxlen;
				memcpy(tmp_buf,org_buf,reallen);
			}
			else
			{
				if(fillflag==LEFTZERO)
				{
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i]='0';
					}
					memcpy(tmp_buf+maxlen-reallen,org_buf,reallen);
					swVdebug(2,"S3374: 1maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else
				{
					memcpy(tmp_buf,org_buf,reallen);
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i+reallen]='0';
					}
					swVdebug(2,"S3376: 2maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
			}
			reallen=maxlen;
			swASC2BCD( tmp_buf, dest_buf , maxlen , fillflag);
			_ufNtoT((char *)dest_buf,(reallen+1)/2,(char *)org_value);
			*iOrg_len = (reallen+1)/2;
			return((reallen+1)/2);

			/* ============== add by qy 2001.08.15 =============== */
		case Efxn:
			if ((org_buf[0] != 'C') && (org_buf[0] != 'D'))
			{
				swVdebug(1,"S3378: Efxn���͵ĵ�һλӦΪ[C]��[D]");
				return(FAIL);
			}
			if (reallen>=maxlen)
			{
				reallen=maxlen;
				memcpy(tmp_buf,org_buf,reallen);
			}
			else
			{
				if(fillflag==LEFTZERO)
				{
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i]='0';
					}
					memcpy(tmp_buf+maxlen-reallen,org_buf,reallen);
					swVdebug(2,"S3380: 1maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else
				{
					memcpy(tmp_buf,org_buf,reallen);
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i+reallen]='0';
					}
					swVdebug(2,"S3382: 2maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
			}
			reallen=maxlen;
			memcpy(dest_buf,tmp_buf,1);
			swASC2BCD(tmp_buf+1, dest_buf+1 , maxlen-1 , fillflag);
			_ufNtoT((char *)dest_buf,(reallen)/2+1,(char *)org_value);
			*iOrg_len = (reallen)/2+1;
			return(reallen/2+1);
			/* ============== end of add 2001/08/15 ================ */

		case Eln:
		case Elln:
		case Ellz:  /* modify by zhanghui 2001/3/22 */
		case Ellan:
		case Ellans:
			if(reallen>maxlen)reallen=maxlen;
			if (reallen==0)
			{
				reallen=maxlen;
				dest_buf[0] = D2B(reallen);
				for (i=1;i<(reallen+1)/2;i++)
					dest_buf[i]=0;
			}
			else
			{
				dest_buf[0] = D2B(reallen);
				swASC2BCD1(org_buf,dest_buf+1,reallen,fillflag);
			}
			if(cgDebug>=2)swDebughex((char *)org_buf, reallen);
			if(cgDebug>=2)swDebughex((char *)dest_buf, (reallen+1)/2);
			_ufNtoT((char *)dest_buf,1,(char *)org_len);
			_ufNtoT((char *)dest_buf+1,(reallen+1)/2,(char *)org_value);
			*iOrg_len = (reallen+1)/2;
			return((reallen+1)/2+1);

			/* modify by nh 20070622    
      case Ellln:
      case Elllz:
      */
		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			if(reallen>maxlen)reallen=maxlen;
			if (reallen==0)
			{
				reallen=maxlen;
				dest_buf[0] = D2B((reallen/100));
				dest_buf[1] = D2B((reallen%100));
				for (i=0;i<(reallen+1)/2;i++)
					dest_buf[i+2]=0;
			}
			else 
			{
				dest_buf[0] = D2B((reallen/100));
				dest_buf[1] = D2B((reallen%100));
				swASC2BCD1(org_buf,dest_buf+2,reallen,fillflag);
			}
			_ufNtoT((char *)dest_buf,2,(char *)org_len);
			_ufNtoT((char *)dest_buf+2,(reallen+1)/2,(char *)org_value);
			*iOrg_len = (reallen+1)/2;
			return((reallen+1)/2+2);

		default:
			return(FAIL);
		}
	}
	/* ��BCD��,��l,ll,lll������³���BCDASC,ASCASC,����ֻ��ΪASC */
	/* ����ASCBCD�����.2000/11/12 zhanghui */
	else if (bcd_flag==cBCDASC || bcd_flag==cASCASC)
	{
		switch(df_type)
		{
		case Efb:   /***added by JLQ***/
			if(maxlen%8)
				return(FAIL);
			reallen=maxlen/8;
			memcpy(dest_buf, org_buf,reallen);
			_ufNtoT((char *)dest_buf,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(maxlen/8);
		case Efn:
		case Efxn:
		case Efan:
		case Efans:
			if (reallen>=maxlen)
			{
				reallen=maxlen;
				memcpy(tmp_buf,org_buf,reallen);
			}
			else
			{
				if(fillflag==LEFTZERO)
				{
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i]='0';
					}
					memcpy(tmp_buf+maxlen-reallen,org_buf,reallen);
					swVdebug(2,"S3384: 1maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else if(fillflag ==RIGHTZERO)
				{
					memcpy(tmp_buf,org_buf,reallen);
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i+reallen]='0';
					}
					swVdebug(2,"S3386: 2maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else if(fillflag == LEFTSPACE)
				{
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i]=' ';
					}
					memcpy(tmp_buf+maxlen-reallen,org_buf,reallen);
					swVdebug(2,"S3388: 1maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
				else
				{
					memcpy(tmp_buf,org_buf,reallen);
					for(i=0;i<maxlen-reallen;i++)
					{
						tmp_buf[i+reallen]=' ';
					}
					swVdebug(2,"S3390: 2maxlen=%d, reallen=%d,tmpbuf=[%s]",maxlen,reallen,tmp_buf);
				}
			}
			reallen=maxlen;
			memcpy(dest_buf,tmp_buf,maxlen);
			_ufNtoT((char *)dest_buf,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(reallen);

		case Elans:
		case Eln:
			if (bcd_flag==cBCDASC)
			{
				if(reallen>maxlen) reallen=maxlen;
				if (reallen<0)
					return(FAIL);
				dest_buf[0]=reallen;
				memcpy(dest_buf+1, org_buf, reallen);
				_ufNtoT((char *)dest_buf,1,(char *)org_len);
				_ufNtoT((char *)dest_buf+1,reallen,(char *)org_value);
				*iOrg_len = reallen;
				return(reallen+1);
			}
			else
			{
				if(reallen>maxlen)reallen=maxlen;
				if (reallen<0)
					return(FAIL);
				dest_buf[0]=reallen+'0';
				memcpy(dest_buf+1, org_buf, reallen);
				_ufNtoT((char *)dest_buf,1,(char *)org_len);
				_ufNtoT((char *)dest_buf+1,reallen,(char *)org_value);
				*iOrg_len = reallen;
				return(reallen+1);
			}
			break;

		case Elln:
		case Ellan:
		case Ellans:
		case Ellz:
			if (bcd_flag==cBCDASC)
			{
				if(reallen>maxlen)reallen=maxlen;
				if(reallen<0)
					return(FAIL);
				dest_buf[0]=(reallen/10)*16+(reallen%10);
				memcpy(dest_buf+1, org_buf, reallen);
				_ufNtoT((char *)dest_buf,1,(char *)org_len);
				_ufNtoT((char *)dest_buf+1,reallen,(char *)org_value);
				*iOrg_len = reallen;
				return(reallen+1);
			}
			else
			{
				if(reallen>maxlen)reallen=maxlen;
				if(reallen<0)
					return(FAIL);
				dest_buf[0]=reallen/10+'0';
				dest_buf[1]=reallen%10+'0';
				memcpy(dest_buf+2, org_buf, reallen);
				_ufNtoT((char *)dest_buf,2,(char *)org_len);
				_ufNtoT((char *)dest_buf+2,reallen,(char *)org_value);
				*iOrg_len = reallen;
				return(reallen+2);
			}

		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			if (bcd_flag==cBCDASC)
			{
				if(reallen>maxlen)reallen=maxlen;
				if(reallen<0)
					return(FAIL);
				dest_buf[0]=(((reallen/100)/10)<<4)|((reallen/100)%10);
				dest_buf[1]=(((reallen%100)/10)<<4)|((reallen%100)%10);
				memcpy(dest_buf+2, org_buf, reallen);
				_ufNtoT((char *)dest_buf,2,(char *)org_len);
				_ufNtoT((char *)dest_buf+2,reallen,(char *)org_value);
				*iOrg_len = reallen;
				return(reallen+2);
			}
			else
			{
				if(reallen>maxlen)reallen=maxlen;
				if(reallen<0)
					return(FAIL);
				dest_buf[0]=reallen/100+'0';
				dest_buf[1]=(reallen-reallen/100*100)/10+'0';
				dest_buf[2]=reallen%10+'0';
				memcpy(dest_buf+3, org_buf, reallen);
				_ufNtoT((char *)dest_buf,3,(char *)org_len);
				_ufNtoT((char *)dest_buf+3,reallen,(char *)org_value);
				*iOrg_len = reallen;
				return(reallen+3);
			}
		default:
			return(FAIL);
		}
	} /* end of BCDASC and ASCASC */
	/* add by zhanghui 2001/3/22 */
	else /* ASCBCD */
	{
		switch(df_type)
		{
		case Efb:   /***added by JLQ***/
			if(maxlen%8)
				return(FAIL);
			reallen=maxlen/8;
			memcpy(dest_buf, org_buf,reallen);
			_ufNtoT((char *)dest_buf,reallen,(char *)org_value);
			*iOrg_len = reallen;
			return(maxlen/8);
		case Elans:
		case Eln:
			if(reallen>maxlen)reallen=maxlen;
			if (reallen<0)
				return(FAIL);
			else if (reallen==0)
			{
				reallen=maxlen;
				dest_buf[0]=reallen+'0';
				for (i=0;i<(reallen+1)/2;i++)
					dest_buf[i+1]=0;
			}
			else /* reallen>0 */
			{
				dest_buf[0]=reallen+'0';
				swASC2BCD(org_buf,dest_buf+1,reallen,fillflag);
			}
			if(cgDebug > 1)
			{
				swDebughex((char *)org_buf, reallen);
				swDebughex((char *)dest_buf, (reallen+1)/2);
			}

			_ufNtoT((char *)dest_buf,1,(char *)org_len);
			_ufNtoT((char *)dest_buf+1,(reallen+1)/2,(char *)org_value);
			*iOrg_len = (reallen+1)/2;
			return((reallen+1)/2+1);

		case Elln:
		case Ellan:
		case Ellans:
		case Ellz:
			if(reallen>maxlen)reallen=maxlen;
			if(reallen<0)
				return(FAIL);
			else if (reallen==0)
			{
				reallen=maxlen;
				dest_buf[0]=reallen/10+'0';
				dest_buf[1]=reallen%10+'0';
				for (i=0;i<(reallen+1)/2+1;i++)
					dest_buf[i+2]=0;
			}
			else /* reallen > 0 */
			{
				dest_buf[0]=reallen/10+'0';
				dest_buf[1]=reallen%10+'0';
				swASC2BCD(org_buf,dest_buf+2,reallen,fillflag);
			}
			if(cgDebug > 1)
			{
				swDebughex((char *)org_buf, reallen);
				swDebughex((char *)dest_buf, (reallen+1)/2);
			}
			_ufNtoT((char *)dest_buf,2,(char *)org_len);
			_ufNtoT((char *)dest_buf+2,(reallen+1)/2,(char *)org_value);
			*iOrg_len = (reallen+1)/2;
			return((reallen+1)/2+2);

		case Ellln:
		case Elllan:
		case Elllans:
		case Elllz:
			if(reallen>maxlen)reallen=maxlen;
			if(reallen<0)
				return(FAIL);
			else if (reallen==0)
			{
				reallen=maxlen;
				dest_buf[0]=reallen/100+'0';
				dest_buf[1]=(reallen-reallen/100*100)/10+'0';
				dest_buf[2]=reallen%10+'0';
				for (i=0;i<(reallen+1)/2;i++)
					dest_buf[i+3]=0;
			}
			else
			{
				dest_buf[0]=reallen/100+'0';
				dest_buf[1]=(reallen-reallen/100*100)/10+'0';
				dest_buf[2]=reallen%10+'0';
				swASC2BCD(org_buf,dest_buf+3,reallen,fillflag);
			}
			_ufNtoT((char *)dest_buf,3,(char *)org_len);
			_ufNtoT((char *)dest_buf+3,(reallen+1)/2,(char *)org_value);
			*iOrg_len = (reallen+1)/2;
			return((reallen+1)/2+3);

		default:
			return(FAIL);
		}
	} /* end of ASCBCD */
	/* add by zhanghui 2001/3/22 */
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
 ** ������      ��swIsopack
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
int swIsopack(prebuf,qid,msgbody,msglen)
union  preunpackbuf *prebuf;
short  qid;
unsigned char  *msgbody;
unsigned int *msglen;
{
	unsigned char  *plPointer;               /* ƫ��ָ�� */
	unsigned char  alBitmap[17];             /* BITMAP */
	unsigned char  alTmpBitmap[33];          /* ��ʱ���� */
	unsigned char  alTmpBCD[100];            /* BCD����ʱ���� */
	int   i,k;                             /* ѭ������ */
	short ilFlag;                            /* ��־ */
	unsigned int ilLen;                             /* ���� */
	short ilTableId;                         /* ��id */
	short ilFldId;                           /* ��id */
	short ilRtncode;
	struct swt_sys_queue slSwt_sys_queue;
	struct swt_sys_8583  slSwt_sys_8583;
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
	char alFldid[130];  /* add by zjj 2002.05.14 for qy suggestion */
	/* end of add by qy */

	alFldid[129] = '\0'; /* add by zjj 2002.05.14 for qy suggestion */
	swVdebug(3,"S3392:�յ�8583Ԥ�ñ�����...");
	for(i=0;prebuf[i].sIsobuf.iFldid!=-1;i++)
	{
		swVdebug(3,"S3394: �����=[%d],�򳤶�=[%d],��ֵ=[%s]",
		    prebuf[i].sIsobuf.iFldid,
		    prebuf[i].sIsobuf.iFldlen,
		    prebuf[i].sIsobuf.aFldvalue);
	}

	plPointer = msgbody;

	swVdebug(3,"S3396: start");

	ilRtncode = swShmselect_swt_sys_queue(qid, &slSwt_sys_queue);

	if (ilRtncode != SUCCESS)
		return (FAIL);

	memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));

	ilRtncode = swIsoget(prebuf,1,&(slIsobuf.iFldlen),slIsobuf.aFldvalue);

	/* ��msgtype */
	if (slSwt_sys_queue.tran_type[0] != '1')
	{
		if (slSwt_sys_queue.code_type[0] == '1')
		{
			_swAtoE((unsigned char *)plPointer,(unsigned char *)slIsobuf.aFldvalue,4);
			plPointer += 4;
		}
		else
		{
			memcpy(plPointer,slIsobuf.aFldvalue,4);
			plPointer += 4;
		}
	}
	else /* msgtype is BCD */
	{
		swASC2BCD((unsigned char *)slIsobuf.aFldvalue , alTmpBCD, 
		    4 , LEFTZERO );
		memcpy(plPointer,alTmpBCD,2);
		plPointer += 2;
	}

	/* ��֯bitmap */
	memset(alBitmap, 0x00, sizeof(alBitmap));
	memset(alFldid, '0', sizeof(alFldid));
	alFldid[128] = '\0';
	ilFlag=1;

	for(k=1;;k++)
	{
		if(prebuf[k].sIsobuf.iFldid == -1)
			break;
		if(prebuf[k].sIsobuf.iFldlen == 0)
			continue;

		alFldid[prebuf[k].sIsobuf.iFldid] = '1';

		swSetBitmap((unsigned char *)&alBitmap[0],prebuf[k].sIsobuf.iFldid);
		if(prebuf[k].sIsobuf.iFldid>64) ilFlag=2;
		swVdebug(2,"S3398: swIsopack():set bitmap id=[%d]",prebuf[k].sIsobuf.iFldid);
	}

	if(ilFlag==2)  /* need extend bitmap */
	{
		alBitmap[0]|=0x80;
		swVdebug(2,"S3400: need extend bitmap...for id=[%d]",prebuf[k-1].sIsobuf.iFldid);
	}
	if(strlen(agMac_create)!=0)
	{
		if(ilFlag==1)
			alBitmap[7]|=0x01;
		else 
			alBitmap[15]|=0x01;
	}
	if (slSwt_sys_queue.bitmap_type[0]=='1')  /* BCD Bitmap */
	{

		memcpy(plPointer,alBitmap,8*ilFlag);
		plPointer +=8*ilFlag;
		igBitmaplen=ilFlag*8;
	}
	else /* ASC bitmap */
	{
		igBitmaplen=ilFlag*16;
		swBCD2ASC(alBitmap,alTmpBitmap,ilFlag*16,RIGHTZERO);

		swVdebug(3,"S3402: The ASC Bitmap is ...");
		if(cgDebug>=2)swDebughex((char *)alTmpBitmap,16*ilFlag);
		swVdebug(3,"S3404:The ASC Bitmap end");
		memcpy(plPointer,alTmpBitmap,16*ilFlag);
		plPointer +=16*ilFlag;
	}

	memset(agBitmap,0x00,33);
	memcpy(agBitmap,alTmpBitmap,igBitmaplen);

	ilTableId=slSwt_sys_queue.isotab_id;
	/* ��֯8583����,Ӧ�ñ�֤prebuf�ǰ���iFldid���е� */

	for(k=2;alFldid[k]!='\0';k++)
	{
		if (alFldid[k] == '0') continue;

		memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));

		slIsobuf.iFldid = k;
		ilRtncode = swIsoget(prebuf,k,&(slIsobuf.iFldlen),slIsobuf.aFldvalue);

		ilFldId = k;
		if(ilFldId == 0)
			break;

		ilRtncode = swShmselect_swt_sys_8583(ilTableId, ilFldId, 
		    &slSwt_sys_8583);

		if (ilRtncode != SUCCESS)
			return (FAIL);

		/*  if(strlen(prebuf[k].sIsobuf.aFldvalue) == 0) */
		/*    continue; */
		swVdebug(2,"S3406: k=[%d],id=[%d],value=[%s],fldtype=[%d],maxlen=[%d],bcd=[%c],fill=[%d],reallen=[%d]", k,
		    ilFldId, 
		    slIsobuf.aFldvalue, 
		    slSwt_sys_8583.fld_type, 
		    slSwt_sys_8583.fld_len,
		    slSwt_sys_queue.tran_type[ilFldId-1] ,
		    slSwt_sys_8583.fld_attr,
		    slIsobuf.iFldlen);

		ilLen = swGetIsoStru( 
		    slIsobuf.aFldvalue, 
		    slSwt_sys_8583.fld_type, 
		    slSwt_sys_8583.fld_len,
		    slSwt_sys_queue.tran_type[ilFldId-1] ,
		    slIsobuf.iFldlen,
		    slSwt_sys_8583.fld_attr,
		    plPointer,
		    /* added by zh 2000/3/13 */
		slIsobuf.aOrgfldlen,
		    slIsobuf.aOrgfldvalue,
		    &(slIsobuf.iOrgfldlen),
		    slSwt_sys_queue.code_type[0]
		    );

		swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
		    slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);

		swVdebug(3,"S3408:ilLen=%d qid=%d",ilLen,qid);
		swVdebug(3,"S3410:k=%d  iFldlen=%d",k,slIsobuf.iFldlen);
		swVdebug(3,"S3412:after pack. orglen=(%s) orgbuffer=(%s)",
		    slIsobuf.aOrgfldlen,slIsobuf.aOrgfldvalue);
		if(ilLen==FAIL)
		{
			swVdebug(1,"S3414: swGetIsoStru fail in field [%d]",
			    slIsobuf.iFldid);
			return(FAIL);
		}
		/*
#ifdef DEBUG
  if(cgDebug>=2)
    swVdebug(0,"S3416:next id=%d",slIsobuf.iFldid);
   if(cgDebug>=2) swDebughex(msgbody,plPointer-msgbody+ilLen);
#endif
*/
		plPointer += ilLen;
	}

	/* need to add mac */

	*msglen=plPointer - msgbody;

	swVdebug(3,"S3418: ��8583���ɹ�,����Ϊ%d",*msglen);
	if(cgDebug>=2)swDebughex((char *)msgbody,*msglen);
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
int swSetBitmap( pBitMap , iCount )
unsigned char  pBitMap[];
int   iCount;
{
	if(iCount >=2 && iCount<=64)
	{
		pBitMap[(iCount-1)/8] = pBitMap[(iCount-1)/8] | ( 0x01 << ( 7-(iCount-1) % 8));
		swVdebug(3,"S3420: \n");
		if(cgDebug>=2)swDebughex((char *)pBitMap,16);
		return(SUCCESS);
	}
	else if(iCount >=65 && iCount <=128 )
	{
		iCount -=64;
		pBitMap[(iCount-1)/8+8] = pBitMap[(iCount-1)/8+8] | ( 0x01 <<( 7-(iCount-1) % 8  ));
		swVdebug(3,"S3422: \n");
		if(cgDebug>=2)swDebughex((char *)pBitMap,16);
		return(SUCCESS);
	}
	else
		return(FAIL);
}
