/****************************************************************/
/* 模块编号    ：FM8583                                         */
/* 模块名称    ：8583打包解包                                   */
/* 作     者    ：张辉                                           */
/* 建立日期    ：1999/12/7                                      */
/* 最后修改日期：2001/3/22                                      */
/* 模块用途    ：8583报文打包解包的函数                         */
/* 本模块中包含如下函数及功能说明：                             */
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
/* 修改记录：                                                   */
/*  1999/12/24 对bitmap类型的不同进行处理                       */
/*  2000/3/13  增加对MAC的处理                                  */
/*  2000/11/11 增加对ASCBCD类型的处理                           */
/*  2001/2/22  Oracle移植                                       */
/*  2001/3/22  优化                                             */
/*  2001/3/24  封版V3.01,程序共1607行                           */
/*  2001/4/3   DB2移植,修改了ASC2BCD函数的BUG(1613)             */
/*  2001/4/4   对ASC2BCD增加了memset处理(1621)                  */
/****************************************************************/

/* switch定义 */
#include "switch.h"

#include "swNdbstruct.h"
#include "swShm.h"

extern int _ufNtoT(char *aNstr,unsigned int iLen,char *aTstr);

/* 变量定义 */

struct swt_sys_imf *psgSwt_sys_imf;

extern union preunpackbuf psgUnpackbuf[iBUFFLDNUM];

/* 函数原型 */
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
 ** 函数名      ：swIsounpack
 ** 功  能      ：8583报文预解包
 ** 作  者      ：
 ** 建立日期    ：1999/12/6
 ** 最后修改日期：1999/12/6
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：msgbody: 8583报文
                  msglen : 报文长度
                  qid    : 信箱号
                  prebuf : FML预解包缓冲池
 ** 返回值      ：SUCCESS,FAIL
***************************************************************/
int swIsounpack(msgbody,msglen,qid,prebuf)
unsigned char *msgbody;
unsigned int msglen;
short qid;
union preunpackbuf *prebuf;
{
	unsigned char  *plPointer;               /* 偏移指针 */
	unsigned char  alBitmap[17];             /* BITMAP */
	unsigned char  alTmpBitmap[17];          /* 临时变量 */
	unsigned char  alTmpBCD[iFLDVALUELEN];   /* BCD码临时变量 */
	int   i;                                 /* 循环变量,8583id的下标 */
	int   k;                                 /* 循环变量,prebuf的下标 */
	short ilFlag;                            /* 标志 */
	unsigned int ilLen;                             /* 长度 */
	short ilTableId;                         /* 表id */
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

	/* 解析msgtype */
	slIsobuf.iFldid    = 1;
	slIsobuf.iFldlen   = 4;

	ilRtncode = swShmselect_swt_sys_queue(qid, &slSwt_sys_queue);

	if (ilRtncode != SUCCESS)
		return (FAIL);

	if (slSwt_sys_queue.tran_type[0] == '0') /* ASCII msgtype */
	{
		if (slSwt_sys_queue.code_type[0] == '0')  /* ASCII 码 */
		{
			memset(slIsobuf.aFldvalue , 0x00,sizeof(slIsobuf.aFldvalue) );
			memcpy(slIsobuf.aFldvalue , plPointer ,4 );
			memset(slIsobuf.aOrgfldvalue , 0x00, sizeof(slIsobuf.aOrgfldvalue) );
			memcpy(slIsobuf.aOrgfldvalue , plPointer ,4 );
			plPointer += 4;
			swVdebug(2,"S3332: msgtype is %4.4s",slIsobuf.aFldvalue);
		}
		else /* EBCDIC码 */
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

	/* 判断msgtype合法性 */
	/*
  if(swIsdigit(slIsobuf.aFldvalue,4)==FALSE)
  {
    swVdebug(0,"S3340: [错误/函数调用] swIsdidit()函数,msgtype format error");
      return(FAIL);
  } */ /* delete by zjj 2002.05.14 for qy suggestion */

	swVdebug(2,"S3342: OUT msgtype=%s",slIsobuf.aFldvalue);

	/* 置ISOmsgtype域 */
	swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
	    slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);

	memset(agBitmap,0x00,33);
	memcpy(agBitmap,plPointer,32);
	swVdebug(2,"S3344: bitmap ag is ...");
	if(cgDebug>=1) swDebughex((char *)agBitmap,32);

	if (slSwt_sys_queue.bitmap_type[0]=='1')  /* BCD Bitmap */
	{
		/* 解析bitmap */
		memcpy(alBitmap, plPointer, 8);
		plPointer  += 8;
		ilFlag=1;
		igBitmaplen=8;

		/* 检查是否有扩展bitmap */
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
		/* 解析bitmap */
		memcpy(alTmpBitmap, plPointer, 16);
		/* 如果是EBCDIC码,可能要进行码制转换 */
		swASC2BCD( alTmpBitmap, alBitmap , 16 , RIGHTZERO );
		plPointer  += 16;
		ilFlag=1;
		igBitmaplen=16;

		/* 检查是否有扩展bitmap */
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
	/* check bitmap , 应该在解出pcode之后*/
	/* 
  if(swCheckBitmap(StandardBitmap, alBitmap)==FALSE)
  {
    swVdebug(0,"S3346: [错误/函数调用] swCheckBitmap()函数,bitmap not fit standard bitmap");
    return(FAIL);
  }
  */
	swVdebug(2,"S3348: Stanford:域序号=[%d],域长度=[%d],域值=[%s]",
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
			/* 置ISO域 */
			swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
			    slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);

			if(ilLen==FAIL)
			{
				swVdebug(1,"S3352: [错误/函数调用] swIsounpack()函数,解8583包失败,解到域[%d]出错",i);
				return(FAIL);
			}
			swVdebug(2,"S3354: 返回域长度=%d",slIsobuf.iFldlen);
			swVdebug(3,"域值=...");
			if (cgDebug>=2)
				swDebughex(slIsobuf.aFldvalue,slIsobuf.iFldlen);
			k++;
			plPointer += ilLen;
			if(plPointer - msgbody > msglen)
			{
				swVdebug(1,"S3356: 报文长度出错!");
				return(FAIL);
			}
		}
	}

	swVdebug(3,"S3358: 解8583成功,报文如下...");
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
/* 函 数 名       :  swBCD2ASC                                     */
/* 功    能       :  将一压缩的BCD数转换为ASCII数字串(0-9A-F)      */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年10月12日                                */
/* 最后修改日期   :  1998年10月12日                                */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  pcBCD BCD数                                   */
/*                   iLength 10进制数字位数                        */
/*                   iCutFlag 奇数位时截取方式                     */
/*                   1:左去0 2:右去0 3:左去空格 4:右去空格         */
/* 返 回 值       :  pcASC ASCII字符串                             */
/*******************************************************************/
/* 修改记录       :                                                */
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
/* 函 数 名       :  swASC2BCD                                     */
/* 功    能       :  将一ASCII数字串转换为压缩的BCD数              */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年10月12日                                */
/* 最后修改日期   :  1998年10月12日                                */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  pcASC ASCII字符串                             */
/*                   iLength 10进制数字位数                        */
/*                   iCutFlag 奇数位时添0方式                      */
/*                   1:左加0 2:右加0                               */
/* 返 回 值       :  pcBCD BCD数                                   */
/*******************************************************************/
/* 修改记录       :                                                */
/*                                                                 */
/*******************************************************************/
void swASC2BCD1( pcASC , pcBCD , iLength , iCutFlag )
unsigned char pcASC[];
unsigned char pcBCD[];
int  iLength;
int  iCutFlag;
{
	int i, j;

	/* 如果位数是奇数, 且iCutFlag=1, 前边补0 */
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
/* 函 数 名       :  swBitMapIsOn                                  */
/* 功    能       :  判断bitmap的相应位是否有效                    */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年10月13日                                */
/* 最后修改日期   :  1998年10月13日                                */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  pBitMap bitmap串对应的指针                    */
/*                   iCount  10进制数字位数                        */
/* 返 回 值       :  TRUE:有效 FALSE:无效                          */
/*******************************************************************/
/* 修改记录       :                                                */
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
/* 函 数 名       :  swSetIsoStru                                  */
/* 功    能       :  8583解包                                      */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年12月8日                                 */
/* 最后修改日期   :  1998年12月8日                                 */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  dest_buf : 预解包域                           */
/*                   org_buf  : 8583域                             */
/*                   df_type  : 域类型                             */
/*                   maxlen   : 域的最大长度                       */
/*                   bcd_flag : BCD,ASCII标志                      */
/*                   reallen  : 域的实际长度                       */
/*                   fillflag : BCD填充标志                        */
/*                   org_len  : 原始长度串                         */
/*                   org_value: 原始值串                           */
/*                   ebcdic   : EBCDIC标识                         */
/* 返 回 值       :  返回8583包的域长度(包括长度位),错误则返回FAIL */
/*******************************************************************/
/* 修改记录       :                                                */
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
	if(ebcdic == '1')   /* EBCDIC码的转换 */
	{
		/* 以下只处理bcd_flag=ASCASC的情况,
       暂时不对BCDBCD,BCDASC进行处理 
       增加对ASCBCD的处理*/
		if(bcd_flag != cASCASC && bcd_flag !=cASCBCD )
		{
			swVdebug(1,"S3362: [错误/其它] EBCDIC码的bcd_flag只能是'0' or '3'");
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
			/* 增加对bcd_flag=cASCASC 的处理,2000/07/02 */
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
			/* 增加对bcd_flag=cASCBCD 的处理,2000/07/02 */
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
			/* 增加对bcd_flag=cASCBCD 的处理,2000/07/02 */
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
			/* 增加对bcd_flag=cASCBCD 的处理,2000/07/02 */
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

	/* 上面是处理EBCDIC码的情况,现在可能不全 2001/3/22 */
	/* 下面是处理ASCII码的情况,
     分为bcd_flag=cBCDBCD,cASCASC,cBCDASC,cASCBCD四种情况 */
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
				swVdebug(1,"S3364: [错误/其它]Efxn类型的第一位应为[C]或[D]");
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
	else if( bcd_flag==cASCASC) /* ASCII码 */
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
		case Elans:  /*长度1位ASC,域值BCD */
			len = (org_buf[0]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;

			swBCD2ASC( org_buf+1, dest_buf, len , fillflag);
			_ufNtoT((char *)org_buf,1,(char *)org_len); /*原始长度*/
			_ufNtoT((char *)org_buf+1,(len+1)/2,(char *)org_value); /*原始域值*/
			*iOrg_len = (len+1)/2;
			return((len+1)/2+1); /*返回8583包的长度*/

		case Elln:
		case Ellz:
		case Ellan:
		case Ellans: /*长度两位ASC,值BCD */
			len = (org_buf[0]-'0');
			len = len * 10 + (org_buf[1]-'0');
			if(len<0||len>maxlen)
				return(FAIL);
			*reallen=len;

			swBCD2ASC( org_buf+2, dest_buf, len , fillflag);
			_ufNtoT((char *)org_buf,2,(char *)org_len); /*原始长度*/
			_ufNtoT((char *)org_buf+2,(len+1)/2,(char *)org_value);  /*原始域值 */
			*iOrg_len = (len+1)/2;
			return((len+1)/2+2);  /*返回8583包的长度*/

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
			_ufNtoT((char *)org_buf,3,(char *)org_len);  /*原始长度*/
			_ufNtoT((char *)org_buf+3,(len+1)/2,(char *)org_value);  /*原始域值*/
			*iOrg_len = (len+1)/2;
			return((len+1)/2+3);  /*返回8583包的长度*/

		default:
			return(FAIL);
		}
	} /* end of ASCBCD */
	/* add by zhanghui 2001/3/21 */
}

/*******************************************************************/
/* 函 数 名       :  swGetIsoStru                                  */
/* 功    能       :  8583打包                                      */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年12月8日                                 */
/* 最后修改日期   :  1998年12月8日                                 */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  dest_buf : 8583域                             */
/*                   org_buf  : 预解包域                           */
/*                   df_type  : 域类型                             */
/*                   maxlen   : 域最大长度                         */
/*                   bcd_flag : BCD,ASCII标志                      */
/*                   reallen  : 域长度                             */
/*                   fillflag : BCD填充标志                        */
/*                   org_len  : 原始长度串                         */
/*                   org_value: 原始值串                           */
/* 返 回 值       :  返回域的实际长度(有长度位),错误则返回FAIL     */
/*******************************************************************/
/* 修改记录       :                                                */
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

	if(ebcdic =='1')   /* 码制为EBCDIC */
	{
		/* 以下只处理ASCII的情况 */
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
			/* 增加对bcd_flag=cASCBCD 的处理,2000/07/02 */
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
			/* 增加对bcd_flag=cASCBCD 的处理,2000/07/02 */
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
			/* 增加对bcd_flag=cASCBCD 的处理,2000/07/02 */
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
			/* 增加对bcd_flag=cASCBCD 的处理,2000/07/02 */
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

	if (bcd_flag==cBCDBCD)  /* 长度和值均为BCD */
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
				swVdebug(1,"S3378: Efxn类型的第一位应为[C]或[D]");
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
	/* 非BCD码,在l,ll,lll的情况下出现BCDASC,ASCASC,其他只能为ASC */
	/* 增加ASCBCD的情况.2000/11/12 zhanghui */
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
/* 函 数 名       :  swIsdigit                                     */
/* 功    能       :  判断字符串前若干位是否由数字组成              */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年10月15日                                */
/* 最后修改日期   :  1998年10月15日                                */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  pcString 字符串                               */
/*                   iLength 长度                                  */
/* 返 回 值       :  若是,返回TRUE , 否则返回FALSE                 */
/*******************************************************************/
/* 修改记录       :                                                */
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
 ** 函数名      ：swIsopack
 ** 功  能      ：8583报文打包
 ** 作  者      ：
 ** 建立日期    ：1999/12/8
 ** 最后修改日期：1999/12/8
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：prebuf : FML预解包缓冲池
                  qid    : 信箱号
                  msgbody: 8583报文
                  msglen : 报文长度
 ** 返回值      ：SUCCESS,FAIL
***************************************************************/
int swIsopack(prebuf,qid,msgbody,msglen)
union  preunpackbuf *prebuf;
short  qid;
unsigned char  *msgbody;
unsigned int *msglen;
{
	unsigned char  *plPointer;               /* 偏移指针 */
	unsigned char  alBitmap[17];             /* BITMAP */
	unsigned char  alTmpBitmap[33];          /* 临时变量 */
	unsigned char  alTmpBCD[100];            /* BCD码临时变量 */
	int   i,k;                             /* 循环变量 */
	short ilFlag;                            /* 标志 */
	unsigned int ilLen;                             /* 长度 */
	short ilTableId;                         /* 表id */
	short ilFldId;                           /* 域id */
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
	swVdebug(3,"S3392:收到8583预置报如下...");
	for(i=0;prebuf[i].sIsobuf.iFldid!=-1;i++)
	{
		swVdebug(3,"S3394: 域序号=[%d],域长度=[%d],域值=[%s]",
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

	/* 建msgtype */
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

	/* 组织bitmap */
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
	/* 组织8583报文,应该保证prebuf是按照iFldid排列的 */

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

	swVdebug(3,"S3418: 打8583包成功,长度为%d",*msglen);
	if(cgDebug>=2)swDebughex((char *)msgbody,*msglen);
	return(SUCCESS);
}

/*******************************************************************/
/* 函 数 名       :  swSetBitmap                                   */
/* 功    能       :  设置bitmap的相应位                            */
/* 作    者       :  张辉                                          */
/* 建立日期       :  1998年12月8日                                 */
/* 最后修改日期   :  1998年12月8日                                 */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  pBitMap bitmap串对应的指针                    */
/*                   iCount  10进制数字位数                        */
/* 返 回 值       :  SUCCESS,FAIL                                  */
/*******************************************************************/
/* 修改记录       :                                                */
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
