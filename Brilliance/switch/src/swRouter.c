/****************************************************************/
/* 模块编号    ：ROUTER                                         */ 
/* 模块名称    ：路由管理                                       */
/* 版 本 号    ：V4.3.0                                         */
/* 作	 者    ：史正烨                                         */
/* 建立日期    ：1999/11/18                                     */
/* 最后修改日期：2001/4/3                                       */
/* 模块用途    ：智能化路由模块                                 */
/* 本模块中包含如下函数及功能说明：                             */
/*			 （1）int  main();                      */
/*			 （2）void swQuit();                    */
/*			 （3）int  swSaf_local();                     */
/****************************************************************/

/* switch定义 */
#include "switch.h"

#include "swNdbstruct.h"

#include "swShm.h"

/* 变量定义 */

/* 函数原型定义 */
void swQuit(int);
int swSaf_local(char * msgpack);

/**************************************************************
 ** 函数名      ：main
 ** 功  能      ：主函数 
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999/11/18
 ** 最后修改日期：2000/4/7
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：
***************************************************************/
int main(int argc,char *argv[])
{
  short   j, k;             /* 临时计数变量 */
  short   ilLine_maxid;        /* 路由脚本行数 */
  short   ilScriptexit;        /* 路由脚本退出标识 0-不成功 1-成功 */
  short   ilScriptexec;        /* 路由脚本执行标识 */
  short   ilResult;            /* 路由条件判断结果 0-不成功 1-成功 */
  unsigned int    ilReadSize;          /* 读到的长度 */   /*modified by dyw, 20150417, PSBC_V1.0*/
  short   ilOrgqid=0;          /* 源发邮箱号 */
  short   ilRtnCode;           /* 返回码 0-成功 非0-不成功 */
  short   ilPriority;          /* 信箱参数:优先级 */
  short   ilClass;             /* 信箱参数:类别 */
  short   ilType;              /* 信箱参数:类型 */
  short   ilBegin_q_id;        /* 交易始发信箱 */
  char    alTrancode[11];      /* 交易码 */
  short   ilResultlen;                             /* 表达式计算结果长度 */
  char    alResult[iFLDVALUELEN + 1];              /* 表达式计算结果 */
  struct  msgpack  slMsgpack;                      /* 报文定义 */
  struct  swt_sys_route_m slSwt_sys_route_m;       /* 路由主表共享内存指针 */
  struct  swt_sys_route_d pslSwt_sys_route_d[101];  /* 路由明细表共享内存指针 */

  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swRouter Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swRouter")) exit(FAIL);
 
  /* 设置调试程序名称 */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  /* strncpy(agDebugfile, "swRouter.debug", sizeof(agDebugfile)); delete by dyw, 2015.03.25, PSBC_V1.0*/


  /*==begin== add by cjh, 2015.03.25, PSBC_V1.0*/
/*
  strcpy(agDebugfile,"swSyslog.debug");
  memset(procName,0x00,sizeof(procName));
  strncpy(procName,"swRouter",sizeof(procName)-1);
*/
   swLogInit("swRouter", NULL); /*add by dyw, 2015.03.27, PSBC_V1.0*/
  /*==end== add by cjh, 2015.03.25, PSBC_V1.0*/

  /* 取得DEBUG标志 */
  if((cgDebug = _swDebugflag("swRouter"))==FAIL)
  {
    fprintf(stderr,"取得DEBUG标志出错!");
    exit(FAIL);
  } 

  /* 忽略SIGCLD、SIGINT、SIGQUIT、SIGHUP 信号 */
  signal( SIGTERM , swQuit );
  signal( SIGINT  , SIG_IGN );
  signal( SIGQUIT , SIG_IGN );
  signal( SIGTSTP , SIG_IGN );
  signal( SIGHUP  , SIG_IGN );
  swVdebug(2,"S0010: 忽略SIGCLD、SIGINT、SIGQUIT、SIGHUP 信号");
  
   /* 定位本地邮箱 */
  if(qattach(iMBROUTER)) 
  {
    swVdebug(0,"S0020: [错误/邮箱] qattach()函数,错误码=-1,初始化邮箱出错!");
    exit(FAIL);
  }
  swVdebug(2,"S0030: 定位本地邮箱成功");
 
  /* 初始化共享内存 */
  ilRtnCode = swShmcheck();
  if (ilRtnCode != SUCCESS)
  {
    swVdebug(0,"S0040: [错误/共享内存] 初始化共享内存指针失败");
    swQuit(FAIL);
  }

  /* 向系统监控发送信息报文 */
  _swMsgsend(103001,NULL);

  for( ; ; )
  {
    /* 从路由邮箱读入报文 */ 
    ilReadSize=iMSGMAXLEN;
    ilPriority = 0;
    ilClass = 0;
    ilType = 0;
    if( ( ilRtnCode = qread2( (char *)&slMsgpack, &ilReadSize, &ilOrgqid,
      &ilPriority,&ilClass,&ilType)) != SUCCESS ) 
    {
      swVdebug(0,"S0050: [错误/邮箱] qrea2()函数,错误码=%d,读邮箱出错",ilRtnCode);
      swMberror(ilRtnCode, NULL);
      swQuit(FAIL);
    }
    /*=====begin====设置日志名和日志级别, add by cjh, 2015.03.23, PSBC_V1.0=======*/
    cgDebug=slMsgpack.sMsghead.log_level;
    memset(agDebugfile,0x00,sizeof(agDebugfile));
    memcpy(agDebugfile,slMsgpack.sMsghead.file_log_name,sizeof(slMsgpack.sMsghead.file_log_name));
    /*======end=====设置日志名和日志级别, add by cjh , 2015.03.23, PSBC_V1.0======*/

    psgMsghead = &(slMsgpack.sMsghead);  /* add by szhengye 2001.10.16 */
    /* memcpy((char *)&sgMsghead,&slMsgpack.sMsghead,sizeof(struct msghead));
    delete by szhengye 2001.10.16 */
    
    if ((ilOrgqid != iMBCTRL) && (ilOrgqid != iMBDBSQL) && 
        (ilOrgqid!=iMBFILE) &&(ilOrgqid!=iMBFILETRAN))
    {
      swVdebug(0,"S0060: [错误/其他] swRouter只能处理swCtrl/swDb/swFile发来的路由[201]报文!");
      continue;
    }

    swVdebug(2,"S0070: 收到报文,源发邮箱=[%d],Priority=%d,Class=%d, Type=%d", ilOrgqid, ilPriority,ilClass,ilType);
    if (cgDebug >= 2)
    {
      swDebugmsghead((char *)&slMsgpack);
      if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
      {
         swDebugfml((char *)&slMsgpack);
         swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      }
      else 
         swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
    }

    /* 更新共享内存 */
    if((ilRtnCode=swShmcheck())!=SUCCESS)
    {
      swVdebug(0,"S0080: [错误/共享内存] 刷新内存指针出错");
      swQuit(FAIL);
    }
    if ( slMsgpack.sMsghead.iMsgtype != iMSGROUTE) 
    {
      /* 发管理报文(非路由报文)信息报文至后台监控（303001) */
      swVdebug(0,"S0090: [错误/其他] 报文类型不正确!");
      _swMsgsend( 303001, NULL);
      continue;
    }
    swVdebug(2,"S0100: 报文类型为路由报文");

    if ( slMsgpack.sMsghead.cRouter == cROUTENO )
    /* 报文头.路由请求方式 为 <强制不进行路由请求> */
    {
      swVdebug(2,"S0110: 路由请求方式为强制不进行路由请求");
      /* 将报文发回至报文源发邮箱 */ 
      if( ( ilRtnCode = qwrite2((char *)&slMsgpack, slMsgpack.sMsghead.iBodylen
           + iMSGHEADLEN, iMBCTRL,ilPriority,ilClass,ilType )) != SUCCESS) 
      {
	 swVdebug(0,"S0120: [错误/邮箱] qwrite2()函数,错误码=%d,发送路由报文到邮箱[%d]出错!",ilRtnCode,ilOrgqid);
         swMberror(ilRtnCode, NULL);
         swQuit(FAIL);
      }
      else
      {
        if (cgDebug >= 2)
        {
          swDebugmsghead((char *)&slMsgpack);
          if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
          {
            swDebugfml((char *)&slMsgpack);
            swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
          }
          else
            swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
        }
	swVdebug(2,"S0130: 发送路由报文到邮箱[%d]成功!",ilOrgqid);
        continue;
      }
    }
    swVdebug(2,"S0140: 路由请求方式为进行路由请求");

    /* 判断报文格式类型是否为内部格式报文 */ 
    if ( slMsgpack.sMsghead.iMsgformat != iFMTIMF )
    {
      /* 报文头.报文格式类型 不为 <FML内部报文格式> */
      swVdebug(0,"S0150: [错误/其他] 报文格式类型不为FML报文格式");
      _swMsgsend( 303004, NULL );
      continue;
    }
    /* 将报文中IMF各域分解至FML变量预解池（调预解IMF格式函数）中 */
    ilRtnCode = swFmlunpack( slMsgpack.aMsgbody,
      slMsgpack.sMsghead.iBodylen, psgPreunpackbuf );
    if ( ilRtnCode == FAIL )
    {
      swVdebug(0,"S0160: [错误/函数调用] swFmlunpack()函数,返回码=%d, FML报文解包出错!",ilRtnCode);
      _swMsgsend( 399001, NULL ); 
      continue;
    }
    swVdebug(2,"S0170: FML报文解包成功");
 
    /* 从报文头中取出起始邮箱、交易码 */
    ilBegin_q_id = slMsgpack.sMsghead.iBegin_q_id;
    memset (alTrancode,0x00,sizeof(alTrancode));
    memcpy(alTrancode ,slMsgpack.sMsghead.aBegin_trancode,\
      sizeof(slMsgpack.sMsghead.aBegin_trancode)); 
    swVdebug(2,"S0180: 从报文头中取出起始邮箱=[%d],交易码=[%s]",\
      ilBegin_q_id,alTrancode);
 
    /* 根据起始邮箱、交易码从内存路由主表中进行匹配得到路由组 */      
    if ( (ilRtnCode = swShmselect_swt_sys_route_m(ilBegin_q_id,alTrancode,
      &slSwt_sys_route_m))!=SUCCESS)
    {
      swVdebug(0,"S0190: [错误/共享内存] 从共享内存路由主表中批配路由脚本组出错[%d]",ilRtnCode);
      continue;
    }
                  
    swVdebug(2,"S0200: 匹配得到路由脚本组[%d]",slSwt_sys_route_m.route_grp);
    /* 根据路由组从内存中找到路由脚本段 */
    if ( (ilRtnCode = swShmselect_swt_sys_route_d( slSwt_sys_route_m.route_grp,
       pslSwt_sys_route_d,&ilLine_maxid)) !=SUCCESS)
    {
      swVdebug(0,"S0210: [错误/共享内存] 从共享内存中取路由脚本出错[%d]",ilRtnCode);
      continue;
    }
    swVdebug(2,"S0220: 从共享内存中取到路由脚本");

    /* 路由脚本执行退出标识置为假 */
    ilScriptexit = FALSE;
    /* 路由执行标识置为假 */
    ilScriptexec = FALSE;

   /************ add by baiqj, 2015.03.19 ,PSBC_V1.0 *****/
   /*judge if msg is return for CALL, if it is CALL return, calculate the time consume
    *判断报文是否是CALL的返回报文，如果是，则计算call耗时
    */
    if (slMsgpack.sMsghead.call_flag != '0')
    {
	sw_calc_call_cost(&slMsgpack.sMsghead.callstart,
			slSwt_sys_route_m.route_grp,
			slMsgpack.sMsghead.iRouteline ,
			slMsgpack.sMsghead.timeconsum,
			sizeof(slMsgpack.sMsghead.timeconsum)
			);
        slMsgpack.sMsghead.call_flag = '0';
        slMsgpack.sMsghead.callstart.time = 0;
        slMsgpack.sMsghead.callstart.millitm = 0;
    }

    /* 按行号为序遍历指定路由脚本段中的脚本 */
    for ( j=0; j<ilLine_maxid; j++ )
    {
      /* 移至行号大于报文头.上次脚本执行行号的记录 */
      if (pslSwt_sys_route_d[j].route_id <= slMsgpack.sMsghead.iRouteline) 
         continue;
      swVdebug(2,"S0230: 路由脚本执行行号[%d]",
	    pslSwt_sys_route_d[j].route_id);

      ilResult = FALSE;
      /* 路由判断条件为空 */
      if ( strlen( pslSwt_sys_route_d[j].route_cond ) == 0 )
        ilResult = TRUE;
      else
      {
        swVdebug(2,"S0240: 路由判断条件-%s",
           pslSwt_sys_route_d[j].route_cond);

	/* 调<路由判断函数(路由判断条件)>得到路由条件结果 */
        ilRtnCode=_swExpress(pslSwt_sys_route_d[j].route_cond,
          alResult,&ilResultlen );
        if (ilRtnCode == FAIL ) 
        {
          swVdebug(2,"S0250: 表达式[%s]计算出错",pslSwt_sys_route_d[j].route_cond);
          _swMsgsend( 303006, NULL ); 
	  ilScriptexec = TRUE;
          break; 
        }
	swVdebug(2,"S0260: 计算结果[%s] = [%s]",
           pslSwt_sys_route_d[j].route_cond,alResult);
        if ( alResult[0] != '0' )
            ilResult = TRUE;
      }

      /*若路由条件不成立，继续下一判断*/
      if (ilResult == FALSE )
      {
        swVdebug(2,"S0270: 当前路由条件不成立,执行下条语句");
        continue;
      }
      swVdebug(2,"S0280: 路由条件成立,执行当前语句");

      memset(alResult,0x00,iFLDVALUELEN + 1);
      slMsgpack.sMsghead.iRevmode = 0;

      /* SWITCH（路由语句标识）*/
      switch ( pslSwt_sys_route_d[j].oper_flag[0] )
      {
        case cROUTECALL:
          swVdebug(2,"S0290: 执行CALL语句");
          /* 路由执行标识 = 真 */
	  ilScriptexec = TRUE;
	  /* 根据目标信箱表达式计算目标信箱号 */
          ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,
              alResult,&ilResultlen);
          if( ilRtnCode == FAIL )
          {
            swVdebug(0,"S0300: [错误/函数调用] _swExpress()函数,返回码=%d,目标邮箱表达式计算错[%s]!",ilRtnCode,pslSwt_sys_route_d[j].q_target);
            _swMsgsend( 303006, NULL ); 
            ilScriptexit = TRUE;
               break; 
          }
          _swTrim(alResult);
	  swVdebug(2,"S0310: 执行路由动作CALL %s",alResult);

          slMsgpack.sMsghead.iDes_q = atoi(alResult);
          /* 检查目标邮箱是否合法 */
          if( swCheckportid(slMsgpack.sMsghead.iDes_q) == FAIL )
          {
            swVdebug(0,"S0320: [错误/函数调用] swCheckportid()函数,返回码=-1, 目标邮箱不合法,请检查邮箱号%s的配置",alResult);
            ilScriptexit = TRUE;
            break;
          }

          slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
          slMsgpack.sMsghead.iFmtgroup = pslSwt_sys_route_d[j].fmt_group;
          slMsgpack.sMsghead.iRevmode = pslSwt_sys_route_d[j].rev_mode;
          slMsgpack.sMsghead.iRevfmtgroup = pslSwt_sys_route_d[j].rev_fmtgrp;
	  
	  /*add by baiqj, 20150319,PSBC_V1.0, store the call start time to msghead*/
	  ftime( &(slMsgpack.sMsghead.callstart) );

	  slMsgpack.sMsghead.call_flag = '1';
      
          /* 发送报文到报文目标邮箱 */
          if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
             slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
             iMBCTRL,ilPriority,ilClass,ilType )) != SUCCESS)
          {
            swVdebug(2,"S0330: 发送报文至目标邮箱[%d]出错",iMBCTRL);
            swMberror(ilRtnCode, "错误:qwrite至目标邮箱出错!");
            swQuit(FAIL);
          }
          swVdebug(2,"S0340: 发送报文至目标邮箱[%d]成功",iMBCTRL);
          if (cgDebug >= 2)
          {
            swDebugmsghead((char *)&slMsgpack);
            if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
	    {
              swDebugfml((char *)&slMsgpack);
              swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
            else
              swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
          }

          /* 路由脚本执行退出标识置为真 */
          ilScriptexit = TRUE;
          break;

          case cROUTESEND:
            swVdebug(2,"S0350: 执行SEND语句");
            /* 根据目标信箱表达式计算目标信箱号 */
            ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,
                alResult,&ilResultlen);
            if( ilRtnCode == FAIL )
            {
              swVdebug(2,"S0360: 目标邮箱表达式计算出错[%s]!",
                pslSwt_sys_route_d[j].q_target);
              _swMsgsend( 303006, NULL ); 
              ilScriptexit = TRUE;
              break; 
            }
            _swTrim(alResult);
	    swVdebug(2,"S0370: 执行路由动作 SEND %s",alResult);

            slMsgpack.sMsghead.iDes_q = atoi(alResult);
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
            slMsgpack.sMsghead.iFmtgroup = pslSwt_sys_route_d[j].fmt_group;
            slMsgpack.sMsghead.iRevmode = pslSwt_sys_route_d[j].rev_mode;
            slMsgpack.sMsghead.iRevfmtgroup = pslSwt_sys_route_d[j].rev_fmtgrp;

            if (pslSwt_sys_route_d[j].end_flag[0] == '1')
            {
              /* 当前记录END处理标识为真 */
              slMsgpack.sMsghead.cEndflag = cTRUE;
	      /* 路由执行标识 = 真 */
	      ilScriptexec = TRUE;
	      /* 路由执行脚本退出标识置为真 */
	      ilScriptexit = TRUE;
            }

            /* 报文头.SAF处理标识=当前记录SAF处理标识 */
            slMsgpack.sMsghead.cSafflag = pslSwt_sys_route_d[j].saf_flag[0];
            if (slMsgpack.sMsghead.cSafflag == cTRUE)
	    {
              swVdebug(2,"S0380: 发送SAF报文");
	      ilRtnCode=swSaf_local((char *)&slMsgpack);
              if(ilRtnCode)
              {
                swVdebug(0,"S0390: [错误/函数调用] swSaf_local()函数,返回码=%d,发送SAF失败",ilRtnCode);
                ilScriptexit = TRUE;
                break;
              }
            }
	    else
	    {
              if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
                slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
                iMBCTRL,ilPriority,ilClass,ilType )) != SUCCESS)
              {
                swVdebug(2,"S0400: 发送报文至目标邮箱[%d]出错",iMBCTRL);
                swMberror(ilRtnCode, "错误:qwrite至目标邮箱出错!");
                swQuit(FAIL);
              }
              swVdebug(2,"S0410: 发送报文至目标邮箱[%d]成功",iMBCTRL);
              if (cgDebug >= 2)
              {
                swDebugmsghead((char *)&slMsgpack);
                if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
		{
                  swDebugfml((char *)&slMsgpack);
                  swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
                }
                else
                  swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }

            }
            slMsgpack.sMsghead.cSafflag = cFALSE;
            /* 报文头.交易步骤=报文头.交易步骤+1 */
            slMsgpack.sMsghead.iTranstep = slMsgpack.sMsghead.iTranstep + 1;
            break;

	  case cROLLBACK:
            swVdebug(2,"S0420: 执行ROLLBACK语句");
            /* 路由执行标识 = 真 */
            ilScriptexec = TRUE; 
            /* 路由执行脚本退出标识置为真 */
	    ilScriptexit = TRUE;
	    /* 发即时冲正信息报文(信息码是103003)至后台监控 */
	    _swMsgsend(103003,(char *)&slMsgpack);
	    /* 发冲正命令报文(信息码是601)至事务管理模块 */
	    slMsgpack.sMsghead.iMsgtype = iMSGORDER;
	    slMsgpack.sMsghead.lCode = 601;

	    ilRtnCode = qwrite ((char *)&slMsgpack,
              sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen,iMBMONREV);
            if (ilRtnCode != SUCCESS)
	    {
              swVdebug(0,"S0430: [错误/邮箱] qwrite()函数,错误码=%d,发送冲正报文至事务监控邮箱出错",ilRtnCode);
              swMberror(ilRtnCode, (char *)&slMsgpack);
              swQuit(FAIL);
            }
            swVdebug(2,"S0440:发送冲正报文至事务监控邮箱成功");
            if (cgDebug >= 2)
            {
              swDebugmsghead((char *)&slMsgpack);
              if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
              {
                swDebugfml((char *)&slMsgpack);
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }
              else
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
	    break;
          case cGOTO:
	    /* 遍历当前路由脚本段的所有语句以查找GOTO的目标语句行 */
	    swVdebug(2,"S0450: 路由动作: GOTO %d",
        	pslSwt_sys_route_d[j].next_id);
            for( k=0; k<ilLine_maxid ; k++ )
            if(pslSwt_sys_route_d[k].route_id == pslSwt_sys_route_d[j].next_id)
                break;
            if(pslSwt_sys_route_d[k].route_id != pslSwt_sys_route_d[j].next_id)
            {
	      swVdebug(2,"S0460: GOTO语句出界");
              /* 发送GOTO语句出界信息报文（303005） */
              _swMsgsend(303005,(char *)&slMsgpack);
	      /* 路由执行脚本退出标识置为真 */
              ilScriptexit = TRUE;
              break;
            }
            j = k-1;
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[k].route_id - 1;
            break;

          case cFMLSET:
	    swVdebug(2,"S0470: 路由动作 SET %s = %s",
               pslSwt_sys_route_d[j].fml_name, pslSwt_sys_route_d[j].q_target);
	    /* 根据表达式计算FMLSET的值 */
            ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,
               alResult,&ilResultlen);
            if( ilRtnCode == FAIL )
            {
              swVdebug(2,"S0480: 计算表达式[%s]出错",pslSwt_sys_route_d[j].q_target);
              _swMsgsend( 303006, NULL ); 
              ilScriptexit = TRUE;
              break; 
            }
	    swVdebug(2,"S0490: 计算结果 %s = %s",
               pslSwt_sys_route_d[j].fml_name,alResult);
            ilRtnCode = swFmlset(pslSwt_sys_route_d[j].fml_name,
               ilResultlen,alResult,psgPreunpackbuf);
            if (ilRtnCode != SUCCESS)
            {
              swVdebug(0,"S0500: [错误/函数调用] swFmlset()函数,返回码=%d,变量[%s]swFmlset出错!",ilRtnCode,pslSwt_sys_route_d[j].fml_name);
              ilScriptexit = TRUE;
              break;
            }

            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;

            ilRtnCode = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,
               &(slMsgpack.sMsghead.iBodylen));
            if ( ilRtnCode == FAIL )
            {
              swVdebug(0,"S0510: [错误/函数调用] swFmlpack()函数,返回码=%d,生成FML报文失败",ilRtnCode);
              _swMsgsend( 302016, NULL );
              ilScriptexit = TRUE;
              break;
            }
            break;
          case cEXECSQL:
	    ilScriptexec = TRUE;
            swVdebug(2,"S0520: 数据库操作 %s",
               pslSwt_sys_route_d[j].q_target);
            ilRtnCode = swFmlset("_SQLTEXT",
               strlen(pslSwt_sys_route_d[j].q_target),
               pslSwt_sys_route_d[j].q_target,psgPreunpackbuf);
            if (ilRtnCode)
            {
              swVdebug(0,"S0530: [错误/函数调用] swFmlset()函数,返回码=%d,置[_SQLTEXT]域失败",ilRtnCode);
              ilScriptexit = TRUE;
              break;
            }

            ilRtnCode = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,
               &(slMsgpack.sMsghead.iBodylen));
            if ( ilRtnCode == FAIL )
            {
              swVdebug(0,"S0540: [错误/函数调用] swFmlpack()函数,返回码=%d,生成FML报文失败",ilRtnCode);
              _swMsgsend( 302016, NULL );
              ilScriptexit = TRUE;

              break;
            }

	    /* 报文头.当前路由判断顺序号=该记录路由判断顺序号 */
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
            ilType = pslSwt_sys_route_d[j].next_id;
            if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
               slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
               iMBDBSQL,ilPriority,ilClass,ilType )) != SUCCESS)
            {
              swVdebug(2,"S0550: 发送报文至数据库操作邮箱出错");
              swMberror(ilRtnCode, "错误:qwrite至数据库操作邮箱出错!");
              swQuit(FAIL);
            }
            swVdebug(2,"S0560: 发送报文至数据库操作邮箱成功");
            if (cgDebug >= 2)
            {
              swDebugmsghead((char *)&slMsgpack);
              if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
              {
                swDebugfml((char *)&slMsgpack);
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }
              else
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
            ilScriptexit = TRUE;
            break;
          case cFTPPUT:  
          case cFTPGET:
          case cQPUT:
          case cQGET:  
            swVdebug(2,"S0570: 文件传输操作 ");
	    ilScriptexec = TRUE;
            ilRtnCode = swFmlset("_FILEMODE",
               strlen(pslSwt_sys_route_d[j].oper_flag),
               pslSwt_sys_route_d[j].oper_flag,psgPreunpackbuf);
            if (ilRtnCode)
            {
              swVdebug(0,"S0580: [错误/函数调用] swFmlset()函数,返回码=%d,置[_FILEMODE]域失败",ilRtnCode);
              ilScriptexit = TRUE;
              break;
            }

            ilRtnCode = swFmlset("_FILEPARM",
                strlen(pslSwt_sys_route_d[j].q_target),
                pslSwt_sys_route_d[j].q_target,psgPreunpackbuf);
            if (ilRtnCode)
            {
              swVdebug(0,"S0590: [错误/函数调用] swFmlset()函数,返回码=%d,置[_FILEPARM]域失败",ilRtnCode);
              ilScriptexit = TRUE;
              break;
            }

            ilRtnCode = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,
               &(slMsgpack.sMsghead.iBodylen));
            if ( ilRtnCode == FAIL )
            {
              swVdebug(0,"S0600: [错误/函数调用] swFmlpack()函数,返回码=%d,生成FML报文失败",ilRtnCode);
              _swMsgsend( 302016, NULL );
              ilScriptexit = TRUE;
              break;
            }

	    /* 报文头.当前路由判断顺序号=该记录路由判断顺序号 */
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;

            if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
               slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
               iMBFILE,ilPriority,ilClass,ilType )) != SUCCESS)
            {
              swVdebug(2,"S0610: 发送报文至文件管理邮箱出错");
              swMberror(ilRtnCode, "错误:qwrite至文件管理邮箱出错!");
              swQuit(FAIL);
            }
            swVdebug(2,"S0620: 发送报文至文件管理邮箱成功");
            if (cgDebug >= 2)
            {
              swDebugmsghead((char *)&slMsgpack);
              if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
	      {
                swDebugfml((char *)&slMsgpack);
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }
              else
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }

            ilScriptexit = TRUE;
            break;

          /*  add by zjj 2001.11.06  */
          case cDO: 
            swVdebug(2,"S0630: 执行 DO 操作!");
            ilScriptexec = TRUE;
            /* 执行表达式,出错则退出,否则执行下一条 */
            ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,\
              alResult,&ilResultlen);
            if (ilRtnCode == FAIL)
            {
              swVdebug(0,"S0640: [错误/函数调用] 执行DO操作[%s]失败!",\
                pslSwt_sys_route_d[j].q_target);
              _swMsgsend( 303006, NULL ); 
	      ilScriptexit = TRUE;
              break; 
            }
            swVdebug(2,"S0650: 执行 DO 操作结果OK!");
            /* 报文头.当前路由判断顺序号=该记录路由判断顺序号 */
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
            break;
          /* end add by zjj 2001.11.06 */

          default:
            swVdebug(0,"S0660: [错误/其他] 路由脚本标识出错[%d]", \
              pslSwt_sys_route_d[j].oper_flag[0]);
            ilScriptexit = TRUE;
            break;
       }
       /* 路由执行脚本退出标识置为真 */
       if (ilScriptexit)  break; 
       swVdebug(2,"S0670: 继续执行路由脚本");
     }

     if ( ilScriptexec == FALSE )
     /* 路由执行标识为假 */
     {
       /* 303003:没有路由判断条件成立 */
       swVdebug(0,"S0680: [错误/其他] 没有路由判断条件成立");
       _swMsgsend( 303003, NULL);
     }
     swVdebug(2,"S0690: 执行路由脚本成功");
  }
}


/**************************************************************
 ** 函数名      ：swQuit
 ** 功  能      ：退出并释放相关资源 
 ** 作  者      ：吴春庚 
 ** 建立日期    ：1999/11/22
 ** 最后修改日期：1999.11.22
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： SUCCESS, FAIL
***************************************************************/
void swQuit(int sig)
{
  short  ilRtnCode;                 /* 返回码 0-成功 非0-不成功 */
  signal(SIGTERM,SIG_IGN);
  swVdebug(2,"S0700: [swRouter]已经停止!");
  qdetach();
  qattach(iMBROUTER);

  /* 向系统监控发送信息报文 */
  ilRtnCode=_swMsgsend(103002,NULL);
  qdetach();
  if(ilRtnCode!=0)
    exit(ilRtnCode);
  else
    exit(sig);
}

/************************************************************************
 ** 函数名:       swSaf_local
 ** 功  能:       将报文放入SAF表，以SAF机制进行发送
 ** 作  者:       史正烨
 ** 建立日期:     2000.02.27
 ** 最后修改日期: 2000.02.27
 ** 调用其它函数: 
 ** 全局变量:     无
 ** 参数含义:     msgpack -- 报文内容
 ** 返回值:       成功返回 SUCCESS，失败返回 FAIL
 ************************************************************************
 ** 修改记录 ** 
*************************************************************************/
int swSaf_local(char * msgpack)
{
  short ilRtnCode;                    /* 返回码 */
  struct msgpack *pslMsgpack;    
  struct msgpack slMsgpack;          /* 报文定义 add by szhengye 2001.10.10 */
  short ilDes_q;                      /* 目标邮箱号 */
  short ilRev_overtime,ilRev_num;     /* 冲正超时时间、冲正次数 */
  struct swt_sys_queue slSwt_sys_queue;  
  long  llSafid;

  memset(&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
  pslMsgpack = (struct msgpack *)msgpack; 
  memcpy((char *)&slMsgpack,msgpack,pslMsgpack->sMsghead.iBodylen + sizeof(struct msghead));
  ilDes_q = pslMsgpack->sMsghead.iDes_q;

  /* 从共享内存中找出目标邮箱配置 */
  if ( (ilRtnCode=swShmselect_swt_sys_queue(ilDes_q,&slSwt_sys_queue))!=SUCCESS)
  {
    swVdebug(2,"S0710: 共享内存中未找到目标邮箱配置[%d][%d]",
      ilRtnCode,ilDes_q);
    return(FAIL);
  }

  /* 取目标邮箱的冲正时间和冲正次数 */ 
  ilRev_overtime=slSwt_sys_queue.rev_overtime;
  ilRev_num=slSwt_sys_queue.rev_num;     

  /* 从共享内存中取到SAF流水号 */
  if ( (ilRtnCode=swShmgetsafid(&llSafid))!=SUCCESS)
  {
    swVdebug(2,"S0720: 操作共享内存SAF流水号出错");
    return(FAIL);
  }
  swVdebug(2,"S0730: 操作共享内存产生新的SAF流水号");

  slMsgpack.sMsghead.lSafid = llSafid;
  slMsgpack.sMsghead.cSafflag = '1';
  slMsgpack.sMsghead.iMsgtype = iMSGAPP;

  /* 组织一条SAF记录 */
  sgSwt_rev_saf.saf_id = slMsgpack.sMsghead.lSafid;
  sgSwt_rev_saf.tran_id = 0;
  sgSwt_rev_saf.proc_step = 0;
  time(&(sgSwt_rev_saf.saf_begin));
  sgSwt_rev_saf.saf_overtime = sgSwt_rev_saf.saf_begin + ilRev_overtime;
  sgSwt_rev_saf.rev_overtime = ilRev_overtime;
  if (ilRev_num > 0)
    sgSwt_rev_saf.saf_num = ilRev_num - 1;
  else
    sgSwt_rev_saf.saf_num = ilRev_num;
  sgSwt_rev_saf.saf_len = 
    sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen;

  /* 将报文写入LOG文件 */
  if (swPutlog((char *)&slMsgpack,
    sgSwt_rev_saf.saf_len,&(sgSwt_rev_saf.saf_offset),
    sgSwt_rev_saf.saf_file) == -1)
  {
    swVdebug(0,"S0740: [错误/函数调用] swPutlog()函数,返回码=-1,存报文出错");
    return(FAIL);
  }
  sgSwt_rev_saf.saf_status[0] = '0';
  sgSwt_rev_saf.saf_flag[0] = '1';

/* #ifdef DB_SUPPORT */ /* del by gengling at 2015.04.03 PSBC_V1.0 */
  /* ADDFLAG */
  /* 插入一条SAF记录到数据库中 modify shm->db by xujun 2002.09.13 */
  /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
  /* if ( (ilRtnCode=swDbinsert_swt_rev_saf(sgSwt_rev_saf)) !=SUCCESS) */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  if ( (ilRtnCode=swShminsert_swt_rev_saf(sgSwt_rev_saf)) != SUCCESS)
  {
    swVdebug(2,"S0750: 插入SAF记录出错,[saf_id=%ld,tran_id=%ld]",
             sgSwt_rev_saf.saf_id,sgSwt_rev_saf.tran_id);
    return(FAIL);
  }
  swVdebug(2,"S0760: 向共享内存插入一条SAF记录 ");
/* #endif */ /* del by gengling at 2015.04.03 PSBC_V1.0 */
  slMsgpack.sMsghead.iOrg_q = ilDes_q;
  slMsgpack.sMsghead.iDes_q = 0;

  /* 将报文发送给前置程序 */
  ilRtnCode=qwrite2((char *)&slMsgpack,sizeof(struct msghead)
    + slMsgpack.sMsghead.iBodylen, ilDes_q,1,0,0);
  if(ilRtnCode)
  {
    swVdebug(0,"S0770: [错误/邮箱] qwrite2()函数,错误码=%d,发送SAF报文失败!",ilRtnCode);
    swMberror(ilRtnCode,msgpack);
    swQuit(FAIL);
  }
  if (cgDebug >= 2)
  {
    swDebugmsghead((char *)&slMsgpack);
    if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
    {
      swDebugfml((char *)&slMsgpack);
      swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
    } 
    else
      swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
  }

  return(SUCCESS);
}

/****************************************************************************************
 * 		calculate the time consume by CALL in route
 * 计算call开始时间与当前时间的时间间隔，即耗时，并依次输出到consume中
 * consume同时保存了本次call之前，交易的所有call的耗时
 * 输出的格式为:
 * 路由号:第一次CALL所在的行号,耗时|第二次CALL的行号,耗时|....|第N次CALL的行号,耗时
 *****************************************************************************************/
int  sw_calc_call_cost(struct timeb*  start_time, int route_id , int route_line, char *consume, int size)
{
	struct timeb  ret_time;
	time_t sec;
	long  interval;
	int i = 0;
	ftime(&ret_time);

    interval  = (ret_time.time-start_time->time)*1000L + (ret_time.millitm-start_time->millitm);
	
    for(;  (i<size) && (consume[i]!='\0'); i++) ;
	
	if( (size - i) <= 2 ) 
	{
		printf("size=%d\n", size);
		return -2;
	}
	
	if( i != 0 )
	{
		consume[i++] = '|';
	}
	else
	{
		snprintf(consume,(size) ,"%d:",route_id);
		i += strlen(consume);
	}
	
	if(interval > 1000*30*3) /*大于3分钟，即call超时过大*/
	{
		snprintf(consume + i, (size-i), "%d,MAX", route_line, (interval/1000));
		return 0;
        }
	
	if(interval > 1000) 
	{
		snprintf(consume + i, (size-i), "%d,%.3fs", route_line, (interval/1000));
	}
	else
	{
		snprintf(consume + i, (size -i) , "%d,%d",route_line, interval);
	}

	return 0;
}


/****************************************************************************************
 *获得msg中最近一次call的耗时，时间精确到毫秒，对msghead的timeconsum域值拆分获得要求输出
 * 输入：msghd    报文头
 * 输出:
 * time：耗时     routid ：路由组号   lineid：call所在行号  
 *timeconsum 的格式：路由号:第一次CALL所在的行号,耗时|第二次CALL的行号,耗时|....|第N次CALL的行号,耗时
 *add by baiqj20150323,PSBC_V1.0
 *****************************************************************************************/

//int sw_get_calltime_consum(char* timeconsum,int *time,int * routid,int *lineid)
int sw_get_calltime_consum(struct msghead *msghd,int *time,int * routid,int *lineid)
{
    char tembuf[10];
    char tembuf1[10];
    char tembuf2[10];/*the last call info*/
    char *buf1;
    char *buf2;
    char buf3[10];
    char *ptr;
    int i = 0,j = 0;
    buf1 = strtok(msghd->timeconsum,"|");
//    buf1 = strtok(timeconsum,"|");
    while(buf1 != NULL)
    {
        if(0 == i)
        {
            strncpy(tembuf,buf1,strlen(buf1));/*tembuf = 1250:10,24*/
            tembuf[strlen(buf1)] = '\0';
        }
        memset(tembuf2,0x00,sizeof(tembuf2));
        strncpy(tembuf2,buf1,strlen(buf1));  /*the last time consume data*/
        i ++;
        buf1 = strtok(NULL,"|");
    }

    buf2 = strtok(tembuf,":");   /*split tembuf = 1250:10,24 to get routid 1250*/
    while(buf2 != NULL)
    {
        if(0 == j)
            *routid = atoi(buf2);
        if(1 == j)
        {
            strncpy(tembuf1,buf2,strlen(buf2));
            tembuf1[strlen(buf2)] = '\0';
        }
        buf2 = strtok(NULL,":");

        j ++;
    }

    if(1 == i) /*only one call,split 10,24*/
    {
        j = 0;
        buf1 = strtok(tembuf1,",");
        while(buf1 != NULL)
        {
            memset(buf3,0x00,sizeof(buf3));
            strncpy(buf3,buf1,strlen(buf1));
            if(0 == j)
                *lineid = atoi(buf1);
            else if(1 == j)  /*calculate the result from second to milisecond*/
            {

                ptr = strchr(buf3,'s');
                if(ptr)
                {

                    *ptr = '\0';
                    *time = atoi(buf3)*1000;
                }
                else 
                    *time = atoi(buf1);
            }
                j ++;
            buf1 = strtok(NULL,",");
        }
    }
    else if(i > 1)/*i calls,split the last time consume data:tembuf2  20,15s*/
    {
        j = 0;
        buf1 = strtok(tembuf2,",");
        while(buf1 != NULL)
        {
            memset(buf3,0x00,sizeof(buf3));
            strncpy(buf3,buf1,strlen(buf1));
            if(0 == j)
                *lineid = atoi(buf1);
            else if(1 == j)
            {
                ptr = strchr(buf3,'s');
                if(ptr)
                {
                    *ptr = '\0';
                    *time = atoi(buf3)*1000;
                }
                else 
                    *time = atoi(buf1);
            }
            j ++;
            buf1 = strtok(NULL,",");
        }
    }
    return 0;
}

