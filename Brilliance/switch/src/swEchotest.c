#include "swapi.h" 
#include "swConstant.h"

#include "swNdbstruct.h"

#include "swPubfun.h"
#include "swShm.h"

void swQuit(int sig);
char cgDebug;

/**************************************************************
 ** 函数名      ： main
 ** 功  能      ： 主函数
 ** 作  者      ： 
 ** 建立日期    ： 2001/08/13
 ** 最后修改日期： 
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 无
***************************************************************/
int main(int argc, char *argv[])
{
  short ilWaitTime;                             /* 系统等待时间 */
  long  llCurrentTime;                          /* 系统当前时间 */
  short ilRc, i, ilCount, ilResultlen;
  unsigned int ilMsglen;
  char  alTrancode[11];
  char  alResult[iFLDVALUELEN+1];

  short ilPriority = 0, ilClass = 0, ilType = 0;        /* 信箱参数 */
  struct msgpack slMsgpack;                             /* 报文 */
  struct swt_sys_queue pslSwt_sys_queue[iMAXQUEUEREC];	/* 端口 */

  FILE *plPopenfile;                            /* add by dgm 2001.9.5 */
  short ilPopen;                                /* add by dgm 2001.9.5 */

  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swEchotest Version 4.3.0",argv[1]);

  /* LICENSE */
  /*delete by guofeng 20030109 
  if (_swLicense("swEchotest")) exit(FAIL);
  */
  
  /* 设置调试程序名称 */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strcpy(agDebugfile, "swEchotest.debug");

  /* 取得DEBUG标志 */
  if ((cgDebug = _swDebugflag("swEchotest")) == FAIL)
  {
    printf("S0010 : 取DEBUG标志失败");
    exit(FAIL);
  }
  swVdebug(2,"S0010: 取得版本号[4.3.0]");
  swVdebug(2,"S0020: 设置调试程序名称[%s]", agDebugfile);
  swVdebug(2,"S0030: 取得DEBUG标志[%d]", cgDebug);

  /* add by dgm 2001.9.5 */
  plPopenfile = popen("ps -e|grep swEchotest|wc", "r");
  fscanf(plPopenfile, "%hd", &ilPopen);  
  if (ilPopen > 1)
  {
    swVdebug(0,"S0040: [错误/其它] swEchotest进程已存在！");
    pclose(plPopenfile);
    exit(FAIL);
  }
  else
    pclose(plPopenfile);
  /* end of add by dgm 2001.9.5 */
  

  /* 定位本地邮箱 */
  if (swMbopen(iMBECHOTEST))
  {
    swVdebug(0,"S0050: [错误/邮箱] 定位本地邮箱失败");
    exit(FAIL);
  }
  swVdebug(2,"S0060: 定位本地邮箱成功");
  
  /* 初始化共享内存指针 */
  if(swShmcheck() != SUCCESS)
  {
    swVdebug(0,"S0070: [错误/共享内存] 初始化共享内存指针出错");
    exit(FAIL);
  }
  swVdebug(2,"S0080:初始化共享内存指针成功");


  /* 处理信号 */
  signal(SIGTERM, swQuit);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  swVdebug(2,"S0090: signal()处理信号");

  /* 向系统监控发送信息报文 */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: 发送104001信息,[swEchotest]已经启动...");

  /* 初始化 */
  ilRc = swInit(iMBECHOTEST);
  if (ilRc == FAIL) 
  {
    swVdebug(0,"S0110: [错误/函数调用] swInit()函数,初始化Echotest进程失败");
    swQuit(FAIL);
  }  
  /* 取得当前系统时间 */
  time(&llCurrentTime);
  ilRc = swShmselect_swt_sys_queue_all(pslSwt_sys_queue, &ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0120: [错误/共享内存] 取端口配置表失败");
    exit(FAIL);
  }
  for (i=0; i<ilCount; i++)
  {
    if (pslSwt_sys_queue[i].trigger_freq > 0)
    {
      pslSwt_sys_queue[i].trigger_time = llCurrentTime + 
        pslSwt_sys_queue[i].trigger_freq;
      ilRc = swShmupdate_swt_sys_queue(pslSwt_sys_queue[i].q_id,
        pslSwt_sys_queue[i]);
      if (ilRc != SUCCESS)
      {
        swVdebug(1,"S0130: [错误/共享内存] 更新端口[%d]出错", pslSwt_sys_queue[i].q_id);
        swQuit(FAIL);
      }
    }
  }

  /* 轮询端口表 */
  while (TRUE)
  {
    /* 检测共享内存状态 */
    if (swShmcheck() == FAIL)
    {
      swVdebug(0,"S0140: [错误/共享内存] 检测共享内存状态失败");
      swQuit(FAIL);
    }

    /* 读取系统轮循时间 */
    ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
    if (ilRc == FAIL)
    {
      swVdebug(1,"S0150: [错误/共享内存] 读取系统轮循时间失败");
      swQuit(FAIL);
    }
    ilWaitTime = sgSwt_sys_config.echotest_inter;

    sleep(ilWaitTime);

    /* 清本地邮箱 */
    if ((ilRc = _swClrMb(iMBECHOTEST)) != SUCCESS)
    {
      swVdebug(1,"S0160: [错误/邮箱] 清本地邮箱[%d]出错,错误码=%d", iMBECHOTEST,ilRc);
      exit(FAIL);
    }
    swVdebug(2,"S0170: 清本地邮箱[%d]成功", iMBECHOTEST);

    ilRc = swShmselect_swt_sys_queue_all(pslSwt_sys_queue, &ilCount);
    if (ilRc != SUCCESS)
    {
      swVdebug(1,"S0180: [错误/共享内存] 取端口配置表失败");
      swQuit(FAIL);
    }

    /* 取得当前系统时间 */
    time(&llCurrentTime);

    for (i=0; i<ilCount; i++)
    {
      if ((pslSwt_sys_queue[i].trigger_freq > 0) && 
          (llCurrentTime >= pslSwt_sys_queue[i].trigger_time))
      {
        /* 计算触发时机表达式 */
        ilRc = _swExpress(pslSwt_sys_queue[i].trigger_term, alResult, 
          &ilResultlen);
        if (ilRc == FAIL)
        {
          swVdebug(1,"S0190: [错误/函数调用] _swExpress()函数,计算应用端[%d]触发时机表达式出错[%s]", 
            pslSwt_sys_queue[i].q_id,
            pslSwt_sys_queue[i].trigger_term);
          _swMsgsend(303006, NULL);
          continue;
        }
        
        if (alResult[0] == '1')
        {
          /* 组织Echotest报文，发给主控 */
          swNewtran(&(slMsgpack.sMsghead));
       
          /* 交易码为端口号 */
          sprintf(alTrancode, "%d", pslSwt_sys_queue[i].q_id);
          
          memcpy(slMsgpack.sMsghead.aTrancode, alTrancode, 
            strlen(alTrancode));
          memcpy(slMsgpack.sMsghead.aBegin_trancode, alTrancode, 
            strlen(alTrancode));

	  swVdebug(2,"S0200:[%s]",slMsgpack.sMsghead.aTrancode);
          slMsgpack.sMsghead.iMsgtype = 101;  /* 解包报文 */
          slMsgpack.sMsghead.iBodylen = 0;    /* 空包 */
          slMsgpack.sMsghead.iOrg_q = iMBECHOTEST;    /* 源发邮箱 */
          
          if (slMsgpack.sMsghead.cFormatter == '0')
          /* 不自动进行格式转换 */
          {        
            ilRc = swFormat(&slMsgpack);
            if (ilRc < 0)
            {
              swVdebug(1,"S0210: [错误/函数调用] swFormat()函数,解包失败,返回码=%d",ilRc);
              continue;
            }
          }  
          slMsgpack.sMsghead.iMsgtype = 1; /* 应用报文iMSGAPP */
        
          /* 发送应用报文到交换主控 */
          ilMsglen = sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen;
          ilPriority = 0;
          ilClass = 0;
          ilType = 0;
          ilRc = swSendpack(iMBCTRL, (char *)&slMsgpack, ilMsglen, 
            ilPriority, ilClass, ilType);
          if (ilRc) 
          {
            swVdebug(1,"S0220: [错误/函数调用] swSendpack()函数,发送应用报文到交换主控失败,返回码=%d",ilRc);
            swQuit(FAIL);
          }
        }
        /* 更新触发时间 */
        pslSwt_sys_queue[i].trigger_time = llCurrentTime +
          pslSwt_sys_queue[i].trigger_freq;
        ilRc = swShmupdate_swt_sys_queue(pslSwt_sys_queue[i].q_id,
          pslSwt_sys_queue[i]);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0230: [错误/共享内存] 更新端口[%d]出错", pslSwt_sys_queue[i].q_id);
          swQuit(FAIL);
        }
      }
    }
  }
}
/**************************************************************
 ** 函数名      ：swQuit
 ** 功  能      ：释放相关资源,关闭数据库 
 ** 作  者      :
 ** 建立日期    ：2001/07/09
 ** 最后修改日期：2001/07/09
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：
***************************************************************/
void swQuit(int sig)
{
  short ilRc;

  signal(SIGTERM, SIG_IGN);

  /* 释放资源 */
/*  EXEC SQL close database; */
  ilRc = swMbclose();
  if (ilRc) 
  {
    swVdebug(0,"S0240: 释放资源出错!");
    exit(-1);
  }
  exit(0);
} 
