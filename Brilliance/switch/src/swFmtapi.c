/****************************************************************/
/* 模块编号    ：swFmtapi                                       */
/* 模块名称    ：格式管理api                                    */
/* 版 本 号    ：V4.3.0                                         */
/* 作    者    ：史正烨                                         */
/* 建立日期    ：1999/11/18                                     */
/* 最后修改日期：2001/4/8                                       */
/* 模块用途    ：格式管理                                       */
/* 本模块中包含如下函数及功能说明：                             */
/*           (1) int swFormat(struct msgpack *psMsgpack);     */
/*           (2) int  swPreunpack();                            */
/*           (3) int  swCheckmsghead();                         */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/*   2000/3/13 增加对MAC的处理                                  */
/*   2000/8/24 优化,退出时FREE内存                              */
/*   2001/2/22 Oracle移植                                       */
/*   2001/2/27 增加DEBUG级别                                    */
/*   2001/3/24 封版V3.01,程序共1114行                           */
/*   2001/4/6  INFORMIX移植(1116)                               */
/*   2001/4/8  增加了对交易码为空的错误处理(1125)               */
/****************************************************************/

/* switch定义 */
#include "switch.h"

#include "swNdbstruct.h"
#include "swShm.h"

int swCmpFmtTrantype(short ilMsgformat,short ilTran_type,short flag );
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetXMFfromTDF(int iFmt_group,long lFmt_offset); */
int swGetXMFfromTDF(long iFmt_group,long lFmt_offset);
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetFMLfromTDF(short iFmt_group,long lFmt_offset,union preunpackbuf *prebuf); */
int swGetFMLfromTDF(long iFmt_group,long lFmt_offset,union preunpackbuf *prebuf);
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetSEPfromTDF(short iFmt_group,long lFmt_offset,union preunpackbuf *prebuf); */
int swGetSEPfromTDF(long iFmt_group,long lFmt_offset,union preunpackbuf *prebuf);
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetISOfromTDF(short iFmt_group,long lFmt_offset,union preunpackbuf *prebuf); */
int swGetISOfromTDF(long iFmt_group,long lFmt_offset,union preunpackbuf *prebuf);
int swSeppack(union preunpackbuf *prebuf,char *aMsgbody,unsigned int *iMsglen);
int swISOcheck (union preunpackbuf *prebuf, short tab_id);
int swIsopack(union preunpackbuf *prebuf,short qid,unsigned char *msgbody,unsigned int *msglen);
int sw8583pack(union preunpackbuf *prebuf,short qid,unsigned char *msgbody,unsigned int *msglen);
int swXMFpack(union preunpackbuf *prebuf,char *msgbody,unsigned int *msglen);
int swSepunpack(char *aMsgbody, unsigned int iMsglen, char *aRecs, char *aGrps, \
                char *aFlds, union preunpackbuf *prebuf);
int swIsounpack(unsigned char *msgbody,short msglen,short qid,union preunpackbuf *prebuf);
int sw8583unpack(unsigned char *msgbody,short msglen,short qid,union preunpackbuf *prebuf);

union  preunpackbuf psgUnpackbuf[iBUFFLDNUM]; /* TDF转换后的缓冲池 */
union  preunpackbuf psgMacneedbuf[iBUFFLDNUM];/* MAC计算缓冲池 */
char   agMac_check[iEXPRESSLEN];              /* MAC校验表达式 */
char   agMac_create[iEXPRESSLEN];             /* MAC生成表达式 */
short  ilRtncode;                             /* 返回码 */


/* 函数原型定义 */
int swFormat(struct msgpack *psMsgpack);
short swPreunpack(struct msgpack *psMsgpack);
int swCheckmsghead(struct msgpack * psMsgpack);

/**************************************************************
 ** 函数名      ：swFormat
 ** 功  能      ：格式转换
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.8.24
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：psMsgpack ** 需格式转换的报文、转换后的报文 **
 ** 返回值      ：0  成功
 **              -1  失败
 **              -2  MAC校验出错，向系统监控发消息iMSSYSMON
 **              -3  不能识别的报文类型
***************************************************************/
int swFormat(struct msgpack *psMsgpack)
{
  short i;                           /* 临时计数变量 */
  char  alMsgtranflag[2];            /* 信息传递标识 */
  char  alMsgbody[iMSGMAXLEN];       /* 报文体 */
  unsigned int ilMsglen;
  char  alResult[iFLDVALUELEN + 1];  /* 表达式计算结果 */
  short ilLength;                    /* 表达式计算结果长度 */
/*  static short flag=0;  */

  short  ilQ_id;                   /* 邮箱号 */
  /*modify by zcd 20141226
  short  ilFmt_group;*/
  long  ilFmt_group;              /* 格式转换组编号 */
  long   llFmt_offset;
  struct swt_sys_queue slSwt_sys_queue;
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;

/*
  swVdebug(2,"S3095: before swLicense===============================");
  if (!flag)
  {
    if (_swLicense("swCtrl")) 
    {
      swVdebug(0,"S3000: swFormat无有效的LICENSE");
      return(-9);
    }
    flag = 1;
  }
*/
  swVdebug(4,"iMsgtype_tbl_1=%d",psMsgpack->sMsghead.iMsgtype);  
  swVdebug(2,"S3168: psMsgpack->iMsgformat=%d", psMsgpack->sMsghead.iMsgformat);
  if (swCheckmsghead(psMsgpack) != SUCCESS)
  {
    swVdebug(1,"S3002: [错误/函数调用] swCheckmsghead()函数,报文头错误!");
    return (-1);
  }
  swVdebug(2,"S3004: 检查报文头成功");
  swVdebug(4,"iMsgtype_tbl_2=%d",psMsgpack->sMsghead.iMsgtype);  
  swVdebug(2,"S3168: psMsgpack->iMsgformat=%d", psMsgpack->sMsghead.iMsgformat);
  switch ( psMsgpack->sMsghead.iMsgtype )
  {
    case iMSGUNPACK:
    case iMSGPACK:
    case iMSGREVPACK:
    case iMSGREVUNPACK:
      
      psgMsghead = &(psMsgpack->sMsghead);  /* add by szhengye 2001.10.16 */
      swVdebug(4,"iMsgtype_tbl_3=%d",psMsgpack->sMsghead.iMsgtype);  
      /* memcpy((char *)&sgMsghead, &(psMsgpack->sMsghead), 
        sizeof(struct msghead)); delete by szhengye 2001.10.16 */
   
      lgMsgoffset = 0;

      switch ( psMsgpack->sMsghead.iMsgtype )
      {
        /******************** 解包、冲正解包 ********************/
        case iMSGUNPACK:
        case iMSGREVUNPACK:

          psgUnpackbuf[0].sImfbuf.aFldname[0] = '\0';
          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
          
          /* 预解包 */
          ilRtncode = swPreunpack(psMsgpack);
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3006: [错误/函数调用] swPreunpack()函数,预解包失败,返回码=%d",ilRtncode);
             _swMsgsend(302027, (char *)psMsgpack);
            return (-1);
          }
          swVdebug(2,"S3008: 预解包成功");
       
          /* 如果是指定转换组,则进行TDF转换 */
          memset(agMac_check, 0x00, sizeof(agMac_check));
          memset(agMac_create, 0x00, sizeof(agMac_create));

          if (psMsgpack->sMsghead.iMsgtype == iMSGUNPACK)
          {
            if (psMsgpack->sMsghead.iFmtgroup)
            {
              ilFmt_group = psMsgpack->sMsghead.iFmtgroup;
              /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
              /* swVdebug(2,"S3010: 指定格式组[%d]", ilFmt_group); */
              swVdebug(2,"S3010: 指定格式组[%ld]", ilFmt_group);
               psMsgpack->sMsghead.iFmtgroup = 0;

              memset(&slSwt_sys_fmt_grp, 0x00, 
                sizeof(struct swt_sys_fmt_grp));
              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group, 
                &slSwt_sys_fmt_grp, &llFmt_offset);
              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(0,"S3012: [错误/共享内存] 指定格式转换组为[%d]不存在!",ilFmt_group); */
                swVdebug(1,"S3012: [错误/共享内存] 指定格式转换组为[%ld]不存在!",ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check, 
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create, 
                sizeof(agMac_create));

              goto GetFMLFromTDF;
            }
          }
          else if (psMsgpack->sMsghead.iMsgtype == iMSGREVUNPACK)
          {
            if (psMsgpack->sMsghead.iRevfmtgroup)
            {
              ilFmt_group = psMsgpack->sMsghead.iRevfmtgroup;
              /* del by genling at 2015.03.25 one line */
              /* swVdebug(2,"S3014: 指定冲正格式组[%d]", ilFmt_group); */
              swVdebug(2,"S3014: 指定冲正格式组[%ld]", ilFmt_group);
               psMsgpack->sMsghead.iFmtgroup = 0;
              psMsgpack->sMsghead.iRevfmtgroup = 0;

              memset(&slSwt_sys_fmt_grp, 0x00, 
                sizeof(struct swt_sys_fmt_grp));
              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group,
                &slSwt_sys_fmt_grp, &llFmt_offset);

              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(0,"S3012: [错误/共享内存] 指定格式转换组为[%d]不存在!",ilFmt_group); */
                swVdebug(1,"S3012: [错误/共享内存] 指定格式转换组为[%ld]不存在!",ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check,
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create,
                sizeof(agMac_create));

              goto GetFMLFromTDF;
            }
          }
          /* 计算出交易码 */
          ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iOrg_q, 
            &slSwt_sys_queue);
          if (ilRtncode != SUCCESS)
          {
            swVdebug(1,"S3018: [错误/共享内存] 此邮箱[%d]没配置!", psMsgpack->sMsghead.iOrg_q);
            swDebugmsghead((char *)psMsgpack);
            swDebughex((char *)psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
             return (-1);
          }
          swVdebug(2,"S3020: 邮箱[%d]解包交易码表达式=[%s]",
               psMsgpack->sMsghead.iOrg_q, slSwt_sys_queue.tc_unpack);

          memset(alResult, 0x00, sizeof(alResult));

          ilRtncode = _swExpress(slSwt_sys_queue.tc_unpack, 
            alResult, &ilLength);

          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3022: [错误/函数调用] _swExpress()函数,自定义函数[%s]计算出错!",
               slSwt_sys_queue.tc_unpack);
            _swMsgsend(302007, NULL); 
            return (-1);
          }

          if (alResult[0] == 0x00)
          {
            swVdebug(1,"S3024: [错误/其它] 交易码为空!");
             return (-1);
          }

          /* 给报文头赋值 */
          memset(psMsgpack->sMsghead.aTrancode, 0x00, 
	    sizeof(psMsgpack->sMsghead.aTrancode));
          memcpy(psMsgpack->sMsghead.aTrancode, alResult, ilLength);

	  swVdebug(2,"S3026: 算出交易码为[%10.10s]", 
               psMsgpack->sMsghead.aTrancode);

          /* 根据报文头中的报文类型决定信息传递标识 */
          memset(alMsgtranflag, 0x00, sizeof(alMsgtranflag));
          if (psMsgpack->sMsghead.iMsgtype == iMSGUNPACK)
            alMsgtranflag[0] = cMSGOUT; 
          else
            alMsgtranflag[0] = cMSGREVOUT;

          /* 根据报文头源信箱、交易码、交易步骤、信息传递标识定位格式转换组 */
          ilRtncode = swShmselect_swt_sys_fmt_m(psMsgpack->sMsghead.iOrg_q, 
            psMsgpack->sMsghead.aTrancode, psMsgpack->sMsghead.iTranstep, 
            alMsgtranflag, &slSwt_sys_fmt_grp, &llFmt_offset);

          if (ilRtncode != SUCCESS)
          {
            swVdebug(1,"S3028: [错误/共享内存] 格式转换组无法定位!:[%d],[%s],[%d],[%s]",
               psMsgpack->sMsghead.iOrg_q, psMsgpack->sMsghead.aTrancode,
              psMsgpack->sMsghead.iTranstep, alMsgtranflag);
            _swMsgsend(302003, NULL); 
            return (-1);
          }

          /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
          /* swVdebug(2,"S3030: 定位格式转换组为[%d]",slSwt_sys_fmt_grp.fmt_group); */
          swVdebug(2,"S3030: 定位格式转换组为[%ld]",slSwt_sys_fmt_grp.fmt_group);
           
          ilFmt_group = slSwt_sys_fmt_grp.fmt_group;

          /* 0 - XMF->IMF(解包) 1 - IMF->XMF(打包) */
          ilRtncode = swCmpFmtTrantype(psMsgpack->sMsghead.iMsgformat, 
            slSwt_sys_fmt_grp.tran_type, 0);

          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3032: [错误/函数调用] swCmpFmtTrantype()函数,报文转换类型与格式类型不匹配,返回码=%d",ilRtncode);
            /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
            /* swVdebug(0,"S3034: [错误/函数调用] 报文类型[%d],格式转换组[%d]类型[%d]!", */
            swVdebug(1,"S3034: [错误/函数调用] 报文类型[%d],格式转换组[%ld]类型[%d]!",
               psMsgpack->sMsghead.iMsgformat, 
              slSwt_sys_fmt_grp.fmt_group, 
              slSwt_sys_fmt_grp.tran_type);
            _swMsgsend(302006, NULL); 
            return (-1);
          }

GetFMLFromTDF:

          lgMsgoffset = 0;
          psgUnpackbuf[0].sImfbuf.aFldname[0] = '\0';
          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';

          /* 根据相应格式转换组生成FML预置包 */
	  swVdebug(2,"S3036: 根据相应格式转换组生成FML预置包");
          if (psMsgpack->sMsghead.iMsgformat == iFMTXMF)
          {
            /* 从XMF报文根据TDF生成XMF报文 */
            ilRtncode = swGetXMFfromTDF(ilFmt_group,llFmt_offset);
          }
          else
          {
            ilRtncode = swGetFMLfromTDF(ilFmt_group,llFmt_offset,psgUnpackbuf);
          }
          /* if ( ilRtncode == FAIL ) modify by szhengye 2001.10.12 */
          if ( ilRtncode )
          {
            swVdebug(1,"S3038: [错误/函数调用] 得到标准域值失败,返回码=%d",ilRtncode);
             _swMsgsend(302015, NULL); 
            return (ilRtncode);
          }

	 swVdebug(2,"S3040: MAC检验表达式为[%s]", agMac_check);
 	 
	  /* 进行MAC校验 */
          #ifdef NEEDMACCHECK
	  _swTrim(agMac_check);
	  if (strlen(agMac_check)) /* need check mac */
	  {
	    swVdebug(2,"S3042: 需要进行MAC检验");
 	    memcpy( psgMacneedbuf,psgPreunpackbuf,sizeof(psgMacneedbuf));
	    memset(alResult,0x00,sizeof(alResult));
	    if (_swExpress( agMac_check, alResult,&ilLength)==FAIL)
	    {
	      swVdebug(1,"S3044: [错误/函数调用] _swExpress()函数,MAC校验出错!");
 	      _swMsgsend(302004, (char *)psMsgpack);
	      return (-1);
            }
            swVdebug(2,"S3046: swExpress 成功");
 	    if (alResult[0]!='1')  /* mac check error */
	    { 
	      swVdebug(2,"S3048: MAC校验失败!");
 
	      memset(alMsgbody,0x00,sizeof(alMsgbody));
              ilRtncode = swFmlpack( psgUnpackbuf , alMsgbody, &ilMsglen);
              if (ilRtncode == FAIL)
              {
                swVdebug(1,"S3050: [错误/函数调用] swFmlpack()函数,打包成FML报文失败!");
                 _swMsgsend(302016, NULL); 
                return (-1);
              }

              /* 生成新的报文头,及连接报文体 */
              psMsgpack->sMsghead.iMsgformat = iFMTIMF;
              psMsgpack->sMsghead.iBodylen = ilMsglen;
              memcpy(psMsgpack->aMsgbody, alMsgbody, ilMsglen );

              psMsgpack->sMsghead.iMsgtypeorg = psMsgpack->sMsghead.iMsgtype;
              psMsgpack->sMsghead.iMsgtype = iMSGMESSAGE;
              psMsgpack->sMsghead.lCode = 302099;
              time(&psMsgpack->sMsghead.lBegintime);

              swVdebug(2,"S3052: 发送信息报文(长度:%d)到系统监控", 
                   psMsgpack->sMsghead.iBodylen);
              swVdebug(3,"S3054: 报文开始");
               if (cgDebug >= 2)
              {
                swDebugmsghead((char *)&psMsgpack);
                swDebughex(psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
              }
              swVdebug(3,"S3056: 报文结束");
               /* return (-2);  mac 校验出错，向监控邮箱发消息 */
              _swMsgsend(302099, (char *)psMsgpack);
              return (-1);
	    }
	    else
	    {
	      swVdebug(2,"S3058: MAC校验成功");
             }
	  }
          else 
	    swVdebug(2,"S3060: 不需要进行MAC检验");
          #endif
 
          /* 对FML预解包进行IMF列表中的域校验 */
          #ifdef NEEDFMLCHECK
          ilRtncode = swFMLcheck(psgUnpackbuf);
          if (ilRtncode == FAIL)
          {
	    swVdebug(2,"S3062: FML域校验失败!");
             _swMsgsend(302024, NULL);
            return (-1);
          }
          #endif

          /* 将预解包打成FML格式报文 */
          
          swVdebug(3,"S3064: FML预置包内容为如下");
          for (i=0;i<5;i++)
          {
            swVdebug(3,"S3066: 域名=[%s],域长度=[%d],域值=[%s]",
              psgUnpackbuf[i].sImfbuf.aFldname,
              psgUnpackbuf[i].sImfbuf.iFldlen,
              psgUnpackbuf[i].sImfbuf.aFldvalue);
          }

          ilRtncode = swFmlpack( psgUnpackbuf , alMsgbody, &ilMsglen);
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3068: [错误/函数调用] swGetImfValueFormTdf()函数,打包成FML报文失败!");
              _swMsgsend(302016, NULL); 
            return (-1);
          }

          /* 生成新的报文头,及连接报文体 */
          psMsgpack->sMsghead.iMsgformat = iFMTIMF;
          psMsgpack->sMsghead.iBodylen = ilMsglen;
          memcpy(psMsgpack->aMsgbody, alMsgbody, ilMsglen );

          swVdebug(2,"S3070: 解包、冲正解包成功");
          swVdebug(3,"S3072: 解包、冲正解包报文BEGIN");
           if (cgDebug >= 2)
          {
            swDebugmsghead((char *)psMsgpack);
            swDebughex(psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
          }
          swVdebug(3,"S3074: 解包、冲正解包报文END");
           return 0;
          break;
        /******************** 打包、冲正打包 ********************/
        case iMSGPACK:
        case iMSGREVPACK:

          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';

          swVdebug(2,"S3076: 收到FML报文");
           if (cgDebug>=2) swDebugfml((char *)psMsgpack);
          agMsgbody = psMsgpack->aMsgbody;
          igMsgbodylen = psMsgpack->sMsghead.iBodylen;

          /* 将FML报文置入全局预置包中 */
          ilRtncode = swFmlunpack(psMsgpack->aMsgbody, 
            psMsgpack->sMsghead.iBodylen,psgPreunpackbuf);
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3078: [错误/函数调用] swFmlunpack()函数,预解包出错!");
             _swMsgsend(302010, NULL); 
            return (-1);
          }
          swVdebug(2,"S3080: 预解包成功");
          
           /* 根据源信箱号查询MAC位图 */
          ilQ_id = psMsgpack->sMsghead.iDes_q; 

          /* 置报文头报文格式类型 */
          ilRtncode = swShmselect_swt_sys_queue(ilQ_id, &slSwt_sys_queue);

          if (ilRtncode != SUCCESS)
          {
            swVdebug(1,"S3082: [错误/共享内存] 此邮箱[%d]没配置!", psMsgpack->sMsghead.iDes_q);
             return (-1);
          }
          psMsgpack->sMsghead.iMsgformat = slSwt_sys_queue.msg_format;

	  /* 如果是指定转换组,则进行TDF转换 */
          if ( psMsgpack->sMsghead.iMsgtype == iMSGPACK)
          {
	    if (psMsgpack->sMsghead.iFmtgroup)
	    {
              ilFmt_group = psMsgpack->sMsghead.iFmtgroup;
              /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
	          /* swVdebug(2,"S3084: 指定格式组[%d]", ilFmt_group); */
	          swVdebug(2,"S3084: 指定格式组[%ld]", ilFmt_group);
              psMsgpack->sMsghead.iFmtgroup = 0;

              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group,
                &slSwt_sys_fmt_grp, &llFmt_offset);

              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(1,"S3086: [错误/共享内存] 指定格式组[%d]不存在!", ilFmt_group); */
                swVdebug(1,"S3086: [错误/共享内存] 指定格式组[%ld]不存在!", ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check, 
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create, 
                sizeof(agMac_create));

              goto GetFMLFromTDF2;
	    }
          }
          else if (psMsgpack->sMsghead.iMsgtype == iMSGREVPACK)
          {
	    if (psMsgpack->sMsghead.iRevfmtgroup)
	    {
              ilFmt_group = psMsgpack->sMsghead.iRevfmtgroup;
              /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
	          /* swVdebug(2,"S3084: 指定格式组[%d]", ilFmt_group); */
	          swVdebug(2,"S3084: 指定格式组[%ld]", ilFmt_group);
              psMsgpack->sMsghead.iFmtgroup = 0;
              psMsgpack->sMsghead.iRevfmtgroup = 0;

              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group,
                &slSwt_sys_fmt_grp, &llFmt_offset);

              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(0,"S3086: [错误/共享内存] 指定格式组[%d]不存在!", ilFmt_group); */
                swVdebug(1,"S3086: [错误/共享内存] 指定格式组[%ld]不存在!", ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check,
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create,
                sizeof(agMac_create));

              goto GetFMLFromTDF2;
	    }
          }

          /* 根据报文头中的报文类型决定信息传递标识 */
          memset(alMsgtranflag, 0x00, sizeof( alMsgtranflag));
          if (psMsgpack->sMsghead.iMsgtype == iMSGPACK)
            alMsgtranflag[0] = cMSGIN; 
          else
            alMsgtranflag[0] = cMSGREVIN;
          swVdebug(2,"S3092: 邮箱[%d]打包交易码表达式=[%s]",
               psMsgpack->sMsghead.iDes_q, slSwt_sys_queue.tc_pack);
            
          /* 计算出交易码 */
          memset(alResult, 0x00, sizeof(alResult));

          ilRtncode = _swExpress(slSwt_sys_queue.tc_pack,
            alResult, &ilLength);

          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3094: [错误/函数调用] _swExpress()函数,计算出交易码的自定义函数[%s]计算出错!",
               slSwt_sys_queue.tc_pack); 
            _swMsgsend(302007, NULL); 
            return (-1);
          }
          memset(psMsgpack->sMsghead.aTrancode,0x00,
            sizeof(psMsgpack->sMsghead.aTrancode));
          memcpy(psMsgpack->sMsghead.aTrancode,alResult,ilLength);
          swVdebug(2,"S3096: result=[%s],length=[%d]",alResult,ilLength);
           swVdebug(2,"S3098: 算出的交易码为[%10.10s]",
               psMsgpack->sMsghead.aTrancode);

          /* 根据报文头信箱、交易码、交易步骤、信息传递标识定位格式转换组 */
          ilRtncode = swShmselect_swt_sys_fmt_m(psMsgpack->sMsghead.iDes_q, 
            psMsgpack->sMsghead.aTrancode, psMsgpack->sMsghead.iTranstep, 
            alMsgtranflag, &slSwt_sys_fmt_grp, &llFmt_offset ); 
            
          if (ilRtncode != SUCCESS)
          {
             swVdebug(1,"S3100: [错误/共享内存] 格式转换组无法定位!:[%d],[%s],[%d],[%s]",
                psMsgpack->sMsghead.iDes_q, psMsgpack->sMsghead.aTrancode,
               psMsgpack->sMsghead.iTranstep, alMsgtranflag); /* 2000.3.16 */
             _swMsgsend(302003, NULL); 
             return (-1);
          }

          ilFmt_group = slSwt_sys_fmt_grp.fmt_group;

          /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
          /* swVdebug(2,"S3102: 格式转换组为[%d]",ilFmt_group); */
          swVdebug(2,"S3102: 格式转换组为[%ld]",ilFmt_group);
 
          /* 根据报文转换类型与报文头报文格式类型是否匹配 */
          /* 0 - XMF->IMF(解包) 1 - IMF->XMF(打包) */
          ilRtncode = swCmpFmtTrantype(psMsgpack->sMsghead.iMsgformat,
            slSwt_sys_fmt_grp.tran_type, 1);

          if ( ilRtncode == FAIL )
          {
            swVdebug(1,"S3104: [错误/函数调用] swCmpFmtTrantype()函数,打包报文转换类型与格式类型不匹配!");
            swVdebug(1,"S3106: [错误/函数调用]报文类型[%d],格式转换组类型[%d]!",
               psMsgpack->sMsghead.iMsgformat,
              slSwt_sys_fmt_grp.tran_type);
            _swMsgsend(302006, NULL); 
            return (-1);
          }

GetFMLFromTDF2:

          lgMsgoffset = 0;
          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';

          /* 根据报文格式类型调响应的预填函数 */
          switch (psMsgpack->sMsghead.iMsgformat)
          {
            case iFMTIMF:        /* 内部报文预填函数 */

              psgUnpackbuf[0].sImfbuf.aFldname[0] = '\0';

              /* 根据相应格式转换组生成FML预置包 */
	      swVdebug(2,"S3108: 开始 swGetFMLfromTDF...");
               ilRtncode = swGetFMLfromTDF(ilFmt_group,
                 llFmt_offset,psgUnpackbuf);
	      swVdebug(2,"S3110: swGetFMLfromTDF..成功");
                 break;
            case iFMTNOSEP:      /* 无分隔符报文预填函数 */
            case iFMTSEP:        /* 有分隔符报文预填函数 */

              psgUnpackbuf[0].sSepbuf.iRecid = -1;

              /* 根据相应格式转换组生成有无分隔符报文预置包 */
	      swVdebug(2,"S3112: begin swGetSEPfromTDF...");
               ilRtncode = swGetSEPfromTDF(ilFmt_group,
                llFmt_offset,psgUnpackbuf);
	      swVdebug(2,"S3114: swGetSEPfromTDF...ok");
                 break;
            case iFMT8583:       /* ISO8583报文预填函数 */
            case iFMT8583E:

              psgUnpackbuf[0].sIsobuf.iFldid = -1;
              psgUnpackbuf[0].sIsobuf.iFldlen = -1;
              psgUnpackbuf[0].sIsobuf.iOrgfldlen = -1;

              /* 根据相应格式转换组生成8583预置包 */
	      swVdebug(2,"S3116: begin swGetISOfromTDF...");
               ilRtncode = swGetISOfromTDF(ilFmt_group,
                llFmt_offset,psgUnpackbuf);
	      swVdebug(2,"S3118: swGetISOfromTDF...ok");
                break;
            case iFMTXMF:       /* XMF报文预填函数 */

              psgUnpackbuf[0].sIsobuf.iFldid = -1;

              /* 根据相应格式转换组生成XMF预置包 */
	      swVdebug(2,"S3120: begin swGetXMFfromTDF...");
              ilRtncode = swGetXMFfromTDF(ilFmt_group,llFmt_offset);
	      swVdebug(2,"S3122: swGetXMFfromTDF...ok");
	      
                break;
          } 
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3124: [错误/函数调用] swGetImfValueFormTdf()函数,得到标准域值失败!");
             _swMsgsend(302015, NULL);
            return (-1);
          }
	  #ifdef NEEDFMLCHECK
          if (psMsgpack->sMsghead.iMsgformat == iFMTIMF)
          {
            /* 对FML预解包进行IMF列表中的域校验 */
            ilRtncode = swFMLcheck(psgUnpackbuf);
            if (ilRtncode == FAIL)
            {
	      swVdebug(1,"S3126: [错误/函数调用] swFMLcheck()函数,IMF域校验出错!");
               _swMsgsend(302024, NULL);
              return (-1);
            }
          }
	  #endif

	  swVdebug(2,"S3128: 开始打包...");
          /* 根据报文格式类型决定组包 */
          switch (psMsgpack->sMsghead.iMsgformat)
          { 
            case iFMTIMF:        /* 内部报文组包 */
              ilRtncode = swFmlpack( psgUnpackbuf, alMsgbody, &ilMsglen );
	      swVdebug(2,"S3130: swFmlpack():打包完毕,报文体长度[%d]",ilMsglen);
               break;
            case iFMTNOSEP:      /* 无分隔符报文组包 */
            case iFMTSEP:        /* 有分隔符报文组包 */
              ilRtncode = swSeppack(psgUnpackbuf,alMsgbody,&ilMsglen );
	      swVdebug(2,"S3132: swSeppack():打包完毕,报文体长度[%d]",ilMsglen);
               break;
            case iFMT8583:       /* ISO8583报文组包 */
              /* ISO8583后进行域校验 */
              ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iDes_q,
                &slSwt_sys_queue);
              if (ilRtncode != SUCCESS)
              {
                return (-1);
              }

              #ifdef NEEDISOCHECK
              ilRtncode = swISOcheck(psgUnpackbuf, slSwt_sys_queue.isotab_id); 

              if (ilRtncode == FAIL)
              {
                swVdebug(1,"S3134: [错误/函数调用] swISOcheck()函数,ISO报文检查失败!");
                 _swMsgsend(302026, (char *)psMsgpack);
                return (-1);
              }
              #endif

              ilRtncode = swIsopack(psgUnpackbuf,
                psMsgpack->sMsghead.iDes_q,(unsigned char *)alMsgbody,
                &ilMsglen);

	      swVdebug(2,"S3136: swIsopack():打包完毕,报文体长度[%d]", ilMsglen);
 	      
              swVdebug(3,"S3138: 报文体开始"); 
 	      if (cgDebug>=2)
              {
		swDebughex(alMsgbody,ilMsglen);
	      }
	      swVdebug(3,"S3140: 报文体结束");
 
	      _swTrim(agMac_create);
              if (strlen(agMac_create))
	      {
                memcpy(psgMacneedbuf ,psgUnpackbuf,sizeof(psgMacneedbuf));
	        memset(alResult,0x00,sizeof(alResult));
	        if (_swExpress(agMac_create, alResult,&ilLength)==FAIL)
	        {
		  swVdebug(1,"S3142: [错误/函数调用] _swExpress()函数,生成MAC出错(表达式)!");
 		  _swMsgsend(302005, (char *)psMsgpack);
		  return (-1);
                }
	        if (ilLength==0)  /* mac create error */
	        {
		  swVdebug(1,"S3144: [错误/其它] MAC生成出错!");
 		  _swMsgsend(302005, (char *)psMsgpack);
		  return (-1);
	        }
		/* 追加到报文之后 */
		psMsgpack->sMsghead.iBodylen=ilMsglen+16;
	        memcpy((char *)psMsgpack+ilMsglen,(unsigned char*)alResult,16);
	      }

              break;
            case iFMT8583E:
              ilRtncode = sw8583pack(psgUnpackbuf,
                psMsgpack->sMsghead.iDes_q,(unsigned char *)alMsgbody,
                &ilMsglen);

              swVdebug(2,"S3146: sw8583pack(): 打包完毕,报文体长度[%d]",ilMsglen);

              swVdebug(3,"S3148: 报文体开始");
              if (cgDebug>=2)
              {
                swDebughex(alMsgbody,ilMsglen);
              }
              swVdebug(3,"S3150: 报文体结束");
              break;
            case iFMTXMF:        /* XMF报文组包 */
              ilRtncode = swXMFpack(psgUnpackbuf,alMsgbody,&ilMsglen );
	      swVdebug(2,"S3152: swXMFpack():打包完毕,报文体长度[%d]",ilMsglen);
               break;
            default:
              swVdebug(1,"S3154: [错误/其它] 不能识别的报文类型!");
               _swMsgsend(302002, NULL); 
              return (-1);
          }
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3156: [错误/函数调用] swFmlpack()函数,内部报文组包失败!");
             _swMsgsend(302020, NULL); 
            return (-1);
          }

          /* 生成新的报文头,及连接报文体 */
          psMsgpack->sMsghead.iBodylen = ilMsglen;
          memcpy( psMsgpack->aMsgbody, alMsgbody, ilMsglen );
						 
          swVdebug(2,"S3158: 打包、冲正打包成功");
           swVdebug(3,"S3160: 打包、冲正打包报文BEGIN");
           if (cgDebug >= 2)
          {
            swDebugmsghead((char *)psMsgpack);
            swDebughex(psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
          }
          swVdebug(3,"S3162: 打包、冲正打包报文END");
           return 0;
          break;

        default:
          swVdebug(1,"S3164: [错误/其它] 不能识别的报文类型!");
           _swMsgsend(302002, NULL); 
          return (-3); /* 不能识别的报文类型 */
      }
      break;
    default:
      swVdebug(1,"S3166: [错误/其它] 不能识别的报文类型!");
       _swMsgsend(302002, NULL); 
      return (-3); /* 不能识别的报文类型 */      
  }
}

/**************************************************************
 ** 函数名      ：swPreunpack
 ** 功  能      ：报文预解包
 ** 作  者      ：张辉
 ** 建立日期    ：2000/2/18
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ： psgPreunpackbuf 预解包缓冲区
 **             ： agMsgbody 无分隔符报文存放区
 ** 参数含义    ： psMsgpack->需要预解包的报文
 ** 返回值      ： SUCCESS, FAIL
 **************************************************************/
short swPreunpack(struct msgpack *psMsgpack)
{
  struct swt_sys_queue slSwt_sys_queue;

  swVdebug(2,"S3168: psMsgpack->iMsgformat=%d",
       psMsgpack->sMsghead.iMsgformat);

  /* delete by szhengye
  memset(psgPreunpackbuf,0x00,sizeof(union preunpackbuf)*iBUFFLDNUM);
  */

  switch(psMsgpack->sMsghead.iMsgformat)
  {
    case iFMTIMF:
      agMsgbody = psMsgpack->aMsgbody;
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      psgPreunpackbuf[0].sImfbuf.aFldname[0] = '\0';
      ilRtncode = swFmlunpack(psMsgpack->aMsgbody, 
        psMsgpack->sMsghead.iBodylen,psgPreunpackbuf);
      if ( ilRtncode == FAIL )
      {
        swVdebug(1,"S3170: [错误/函数调用] swFmlunpack()函数,FML报文预解包失败!");
         _swMsgsend(302010, NULL); 
	return(FAIL);
      }
      swVdebug(2,"S3172: FML报文预解包成功");
       break;

    case iFMTNOSEP:  /* 无分隔符、XMF报文不预解包 */
    case iFMTXMF:
      agMsgbody = psMsgpack->aMsgbody; /* add by szhengye 2001.9.11 */
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      swVdebug(2,"S3174: 无分隔符、XMF报文不预解包");
       break;

    case iFMTSEP:
      /* delete by szhengye 2001.9.11
      memcpy(agMsgbody,psMsgpack->aMsgbody, psMsgpack->sMsghead.iBodylen); */
      agMsgbody = psMsgpack->aMsgbody; /* add by szhengye 2001.9.11 */
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;

      ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iOrg_q,
        &slSwt_sys_queue);
      if (ilRtncode != SUCCESS)
      {
        return (-1);
      }

      psgPreunpackbuf[0].sSepbuf.iRecid = -1;

      ilRtncode = swSepunpack( psMsgpack->aMsgbody,
        psMsgpack->sMsghead.iBodylen,
        slSwt_sys_queue.def_rs,
        slSwt_sys_queue.def_gs,
        slSwt_sys_queue.def_fs,
        psgPreunpackbuf);

      if ( ilRtncode == FAIL )
      {
        swVdebug(1,"S3176: [错误/函数调用] swSepunpack()函数,SEP报文预解包失败!");
         _swMsgsend(302011, NULL); 
	return(FAIL);
      }
      swVdebug(2,"S3178: SEP报文预解包成功");
       break;

    case iFMT8583:
      agMsgbody = psMsgpack->aMsgbody;
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      psgPreunpackbuf[0].sIsobuf.iFldid = -1;
      ilRtncode = swIsounpack((unsigned char *)psMsgpack->aMsgbody, 
        psMsgpack->sMsghead.iBodylen,
        psMsgpack->sMsghead.iOrg_q,
        psgPreunpackbuf);
      if ( ilRtncode == FAIL )
      {
        swVdebug(1,"S3180: [错误/函数调用] swIsounpack()函数,ISO报文预解包失败!");
         _swMsgsend(302012, NULL); 
	return(FAIL);
      }
      ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iOrg_q,
        &slSwt_sys_queue);
      if (ilRtncode != SUCCESS)
      {
        return (-1);
      }
      
      /* ISO8583解包后进行域校验 */
      #ifdef NEEDISOCHECK
      ilRtncode = swISOcheck(psgPreunpackbuf, slSwt_sys_queue.isotab_id);

      if (ilRtncode == FAIL)
      {
        swVdebug(1,"S3182: [错误/函数调用] swISOcheck()函数,ISO报文预解包检查失败!");
         _swMsgsend(302026, (char *)psMsgpack);
	return(FAIL);
      }
      #endif

      swVdebug(2,"S3184: ISO报文预解包成功");
 
      break;

    case iFMT8583E:
      agMsgbody = psMsgpack->aMsgbody;
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      psgPreunpackbuf[0].sIsobuf.iFldid = -1;
      ilRtncode = sw8583unpack((unsigned char *)psMsgpack->aMsgbody,
        psMsgpack->sMsghead.iBodylen, 
        psMsgpack->sMsghead.iOrg_q,
        psgPreunpackbuf);
      if (ilRtncode == FAIL)
      {
        swVdebug(1,"S3186: [错误/函数调用] sw8583unpack()函数, ISO8583报文预解包失败!");
        _swMsgsend(302012, NULL);
        return(FAIL);
      }
      swVdebug(2,"S3188: ISO8583报文预解包成功");

      break;

    default:
     swVdebug(1,"S3190: [错误/其它] 不能识别的报文格式!");
       _swMsgsend(302002, (char *)psMsgpack);
      return(FAIL);
      break;
  }
  return(SUCCESS);
}

/**************************************************************
 ** 函数名      ：swCheckmsghead
 ** 功  能      ：报文头检查
 ** 作  者      ：张辉
 ** 建立日期    ：2000/3/14
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：psMsgpack->需要预解包的报文
 ** 返回值      ：SUCCESS, FAIL
 **************************************************************/
int swCheckmsghead(struct msgpack * psMsgpack)
{
  switch(psMsgpack->sMsghead.iMsgtype)
  {
    case iMSGAPP:
    case iMSGREV:
    case iMSGUNPACK:
    case iMSGPACK:
    case iMSGREVUNPACK:
    case iMSGREVPACK:
    case iMSGSAVEHEAD:
    case iMSGLOADHEAD:
    case iMSGROUTE:
    case iMSGMESSAGE:
    case iMSGORDER:
      break;
    default:
      swVdebug(1,"S3192: [错误/其它] 不能识别的报文类型!");
       return(FAIL);
      break;
  }
  return(SUCCESS);
}

