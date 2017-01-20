/****************************************************************/
/* 模块编号    ：FM8583E                                        */ 
/* 模块名称    ：8583E打包解包                                   */
/* 作	 者    ：                                      */
/* 建立日期    ：2002/1/29                                      */
/* 最后修改日期：2002/1/29                                      */
/* 模块用途    ：8583E报文打包解包的函数                         */
/* 本模块中包含如下函数及功能说明：                             */
/*                        (2) int  sw8583unpack();               */
/*                        (5) int  swBitMapIsOn();              */
/*                        (6) int  swSetIsoStru();              */
/*                        (7) int  swGetIsoStru();              */
/*                        (8) int  swIsdigit();                 */
/*                        (9) int  sw8583pack();                 */
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

extern union	preunpackbuf psgUnpackbuf[iBUFFLDNUM];
char agMacbuf[iMSGMAXLEN];	/* MAC buffer */
short igMacbuflen;		/* MAC length */

/* 函数原型 */
int  sw8583unpack();
int  sw8583pack();
static int  swBitMapIsOn();
static int  swSetBitmap();
/*static int  swIsdigit();  */
static int  swSetIsoStru();
static int  swGetIsoStru();
static int  swTranbuf();

/**************************************************************
 ** 函数名      ：sw8583unpack
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
int sw8583unpack(msgbody,msglen,qid,prebuf)
  unsigned char *msgbody;  
  unsigned int msglen;
  short qid;
  union preunpackbuf *prebuf;
{
  unsigned char  *plPointer;               /* 偏移指针 */
  unsigned char  alBitmap[17];             /* BITMAP */
  unsigned char  alTmpbuf[iFLDVALUELEN];   /* 变量 */
  int   i;                                 /* 循环变量,8583id的下标 */
  int   k,ilRc;                                 /* 循环变量,prebuf的下标 */
  short ilFlag;                            /* 标志 */
  unsigned int ilLen;                             /* 长度 */
  short ilTableId;                         /* 表id */
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
  
  /* 解析msgtype */  
  slIsobuf.iFldid    = 1;  

  ilRtncode = swShmselect_swt_sys_queue(qid, &slSwt_sys_queue);
  if (ilRtncode != SUCCESS)
  {
    swVdebug(1,"S3424: [错误/函数调用] swShmselect_swt_sys_queue()函数,返回码=%d qid = %d",ilRtncode,qid);  	 
    return (FAIL);
  }
  ilTableId=slSwt_sys_queue.e8583tab_id;
  
  ilRtncode = swShmselect_swt_sys_8583E(ilTableId, 1, &slSwt_sys_8583E);  
  if (ilRtncode != SUCCESS) 
  { 	
    swVdebug(1,"S3426: [错误/函数调用] swShmselect_swt_sys_8583E()函数,返回码=%d,tab_id=%d  fld_id=1",ilRtncode,ilTableId);  	
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
      swVdebug(1,"S3428: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
      return(FAIL);
    }
    memcpy((unsigned char *)slIsobuf.aFldvalue,alTmpbuf,ilRtnlen);
    slIsobuf.iFldlen  = ilRtnlen; 
  }
  plPointer += slSwt_sys_8583E.fld_len;
    
  /* 判断msgtype合法性 */
  /*  
  if(swIsdigit(slIsobuf.aFldvalue,slIsobuf.iFldlen)==FALSE)
  {
    swVdebug(0,"S3430: [错误/函数调用] swIsdidit()函数,msgtype format error");
    return(FAIL);
  } */ /* delete by zjj 2002.05.14 for qy suggestion */

  swVdebug(2,"S3432: OUT msgtype=%s",slIsobuf.aFldvalue);
  
   /* 置ISOmsgtype域 */
  swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
    slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);
         
  memset(agBitmap,0x00,33);
  memcpy(agBitmap,plPointer,32);
  swVdebug(2,"S3434: bitmap ag is ...");
    if(cgDebug>=1)swDebughex((char *)agBitmap,32);
  
  /* 根据端口配置中的基本bitmap长度取值　*/
  memset(alBitmap,0x00,sizeof(alBitmap));
  memcpy(alBitmap,plPointer,slSwt_sys_queue.bitmap_len);
  ilFlag = 1;
  /* 解包表达式 */
  if (slSwt_sys_queue.bitmap_unpk[0] != '\0')
  {
    _ufNtoT((char *)alBitmap,slSwt_sys_queue.bitmap_len,(char *)alParm[1]);
    alParm[2][0] = '\0';
    swTranbuf(slSwt_sys_queue.bitmap_unpk,alParm,alBuf);
    ilRc = _swExpress(alBuf,alFldvaltmp,&ilRtnlen);
    if (ilRc == FAIL)
    {
      swVdebug(1,"S3436: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
      return(FAIL);
    }
    memcpy(alBitmap,(unsigned char *)alFldvaltmp,ilRtnlen);
  }
  else
    ilRtnlen = slSwt_sys_queue.bitmap_len;

  plPointer += slSwt_sys_queue.bitmap_len;
  /* 取扩展位　*/
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
         swVdebug(1,"S3438: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
         return(FAIL);
       }    
       memcpy(alBitmap+ilRtnlen,(unsigned char *)alFldvaltmp,ilRtnlen);
     }
     plPointer += slSwt_sys_queue.bitmap_len;     
  }

  if(cgDebug>=2) swDebughex((char *)alBitmap,8 * ilFlag);
  
  swVdebug(2,"S3440: Stanford:域序号=[%d],域长度=[%d],域值=[%s]",
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
     /* 置ISO域 */
     swIsoset(slIsobuf.iFldid,slIsobuf.iFldlen,slIsobuf.aFldvalue,
       slIsobuf.aOrgfldlen,slIsobuf.iOrgfldlen,slIsobuf.aOrgfldvalue,prebuf);

      if(ilLen==FAIL)
      {
        swVdebug(1,"S3444: [错误/函数调用] sw8583unpack()函数,解8583包失败,解到域[%d]出错",i);
          return(FAIL);
      }
      swVdebug(2,"S3446: 返回域长度=%d",slIsobuf.iFldlen);
      if (cgDebug>=2)
        swDebughex(slIsobuf.aFldvalue,slIsobuf.iFldlen);
      k++;
      plPointer += ilLen; 
      if(plPointer - msgbody > msglen)
      {
	swVdebug(1,"S3448: 报文长度出错!,msglen = %d  plPointer - msgbody = %d",msglen,plPointer - msgbody);
  	return(FAIL);
      }
    }
  }

  swVdebug(3,"S3450: 解8583成功,报文如下...");
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
  
  /* 生成 macbuf */
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
  if (df_type & EV)  /* 变长　*/
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
         swVdebug(1,"S3454: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
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
        swVdebug(1,"S3460: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
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
  else  /* 定长　*/
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
        swVdebug(1,"S3462: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
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
  unsigned char alParm[3][iEXPRESSLEN];  /* 存放　$1 ,$2的值　*/

  memset(tmp_buf, 0x00, sizeof(tmp_buf));
  memset(tmp_buf2, 0x00, sizeof(tmp_buf2));
  
  if (df_type & EV)  /* 变长　*/
  {
    if(reallen > maxlen) reallen = maxlen;
    if(reallen < 0) return(FAIL);
    /* 根据域长度位取得域长度　*/
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
        swVdebug(1,"S3464: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
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
        swVdebug(1,"S3466: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
        return(FAIL);
      }         
      memcpy(dest_buf + ilLen_len,(unsigned char *)alFldvaltmp,ilRtnlen);
      *iOrg_len = ilRtnlen;
    }
    _ufNtoT((char *)dest_buf,ilLen_len,(char *)org_len);
    _ufNtoT((char *)dest_buf+ilLen_len,*iOrg_len,(char *)org_value);
    return(ilLen_len + *iOrg_len);
  }
  else  /* 定长 */
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
        swVdebug(1,"S3468: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
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
 ** 函数名      ：sw8583pack
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
int sw8583pack(prebuf,qid,msgbody,msglen)
  union  preunpackbuf *prebuf;
  short  qid;
  unsigned char  *msgbody;
  unsigned int *msglen;
{
  unsigned char  *plPointer;               /* 偏移指针 */
  unsigned char  alBitmap[17];             /* BITMAP */
  unsigned char  alTmpBitmap[33];          /* 临时变量 */
  unsigned char  alTmpbuf[100];            /* 临时变量 */
  int   i,k;                             /* 循环变量 */
  short ilFlag;                            /* 标志 */
  unsigned int ilLen;                             /* 长度 */
  short ilTableId;                         /* 表id */
  short ilFldId;                           /* 域id */
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

  swVdebug(3,"S3470:收到8583预置报如下...");
  for(i=0;prebuf[i].sIsobuf.iFldid!=-1;i++)
  {
    swVdebug(3,"S3472: 域序号=[%d],域长度=[%d],域值=[%s]",
      prebuf[i].sIsobuf.iFldid,
      prebuf[i].sIsobuf.iFldlen,
      prebuf[i].sIsobuf.aFldvalue);
  }
 
  plPointer = msgbody;
  swVdebug(3,"S3474: start");
  ilRtncode = swShmselect_swt_sys_queue(qid, &slSwt_sys_queue);
  
  if (ilRtncode != SUCCESS) 
  {
    swVdebug(1,"S3476: [错误/函数调用] swShmselect_swt_sys_queue()函数,返回码=%d qid = %d",ilRtncode,qid);
    return (FAIL);  
  }
  ilTableId=slSwt_sys_queue.e8583tab_id;
  
  ilRtncode = swShmselect_swt_sys_8583E(ilTableId,1,&slSwt_sys_8583E);     
  if (ilRtncode != SUCCESS) 
  {
    swVdebug(1,"S3478: [错误/函数调用] swShmselect_swt_sys_8583E()函数,返回码=%d tab_id = %d fld_id = %d",ilRtncode,ilTableId,1);  	
    return (FAIL);  
  }
    
  memset((char *)&slIsobuf,0x00,sizeof(struct isoprebuf));
  ilRtncode = swIsoget(prebuf,1,&(slIsobuf.iFldlen),slIsobuf.aFldvalue);
 
  /* 建msgtype */
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
      swVdebug(1,"S3480: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRtncode);
      return(FAIL);
    }         	
    if (ilRtnlen != slSwt_sys_8583E.fld_len) return(FAIL);
    memcpy(alTmpbuf,(unsigned char *)alFldvaltmp,ilRtnlen);	
  }
 
  memcpy(plPointer,alTmpbuf,slSwt_sys_8583E.fld_len);
  plPointer += slSwt_sys_8583E.fld_len;
  
  /* 组织bitmap */
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
      swVdebug(1,"S3486: [错误/函数调用] _swExpress()函数,计算结果表达式出错[%s],返回码=%d",alBuf,ilRc);
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

  /* 组织8583报文,应该保证prebuf是按照iFldid排列的 */

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

  swVdebug(3,"S3502: 打8583包成功,长度为%d",*msglen);
    if(cgDebug>=2)swDebughex((char *)msgbody,*msglen);
  /* 生成 macbuf */
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
/* 函 数 名       :  swTranbuf                                 */
/* 功    能       :  替换字符串中的$1,$2                           */
/* 作    者       :  张辉                                          */
/* 建立日期       :  2002.01.29                                 */
/* 最后修改日期   :  2002.01.29                                 */
/* 调用其它函数   :                                                */
/* 全局变量       :                                                */
/* 参数含义       :  aExp ---　表达式　　aValue --- $的值　　       */
/*                  aReturn ---　转换后的表达式　            　　*/ 
/* 返 回 值       :  SUCCESS,FAIL                                  */
/*******************************************************************/
static int swTranbuf(char *aExp,char aValue[][iEXPRESSLEN],char *aReturn)
{
  short i,j = 0;

  /* aValue为已经转意字符　*/
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

