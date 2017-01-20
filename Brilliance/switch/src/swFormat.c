/****************************************************************/
/* 模块编号    ：FORMAT                                         */
/* 模块名称    ：格式管理                                       */
/* 版 本 号    ：V4.3.0                                         */
/* 作    者    ：史正烨                                         */
/* 建立日期    ：1999/11/18                                     */
/* 最后修改日期：2001/4/8                                       */
/* 模块用途    ：格式管理                                       */
/* 本模块中包含如下函数及功能说明：                             */
/*			 （1）void main();                      */
/*			 （2）void swQuit();                    */
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

/* 函数原型定义 */
void   swQuit(int);
short  swFormat(struct msgpack *psMsgpack);

/**************************************************************
 ** 函数名      ：main
 ** 功  能      ：格式管理模块主程序
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999.8.24
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：无
***************************************************************/
int main(int argc, char *argv[])
{
  short ilRtncode;                   /* 返回码 */
  unsigned int ilReadSize;                  /* 读到的长度 */
  struct msgpack  slMsgpack;    
  short ilOrigid=0;
  short ilPrior=0;    
  short ilClass=0;
  short ilType =0;

  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swFormat Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swFormat")) exit(FAIL);

  /* 设置调试程序名称 */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strncpy(agDebugfile, "swFormat.debug", sizeof(agDebugfile));

  /* 取得DEBUG标志 */
  if ((cgDebug = _swDebugflag("swFormat")) == FAIL)
  {
    fprintf(stderr, "main():_swDebugflag():无法取得DEBUG标志!\n");
    exit(FAIL);
  } 

  /* 忽略SIGCLD、SIGINT、SIGQUIT、SIGHUP信号 */
  signal( SIGTERM , swQuit  );
  signal( SIGINT  , SIG_IGN );
  signal( SIGQUIT , SIG_IGN );
  signal( SIGTSTP , SIG_IGN );
  signal( SIGHUP  , SIG_IGN );

  /* 定位本地邮箱 */
  if (qattach(iMBFORMAT)) 
  {
    swVdebug(0,"S0010: [错误/邮箱] qattach()函数");
     exit(FAIL);
  }

  ilRtncode = swShmcheck(); /* 初始化共享内存指针 */

  if (ilRtncode != SUCCESS)
  {
    swVdebug(0,"S0020: [错误/共享内存] 初始化共享内存指针失败");
     swQuit(FAIL);
  }

  /* 发管理报文(格式管理启动)至后台监控 */
  _swMsgsend(102001, NULL);
  swVdebug(2,"S0030: 格式管理模块已经启动......");

  for( ; ; )
  {
    ilPrior = 0;
    ilClass = 0;
    ilType  = 0;

    /* 读取邮箱中的信息 */ 
    /* memset((char *)&slMsgpack, 0x00, iMSGMAXLEN ); */
    ilReadSize = iMSGMAXLEN;
    if ((ilRtncode = qread2((char *)&slMsgpack, &ilReadSize, 
          &ilOrigid, &ilPrior, &ilClass, &ilType)) != SUCCESS) 
    {
      swVdebug(0,"S0040: [错误/函数调用] qread2()函数,返回码=%d",ilRtncode);
       swMberror(ilRtncode, NULL);
      swQuit(FAIL);
    }

    ilRtncode = swShmcheck(); /* 检测共享内存情况，有变动重新刷新指针 */

    if (ilRtncode != SUCCESS)
    {
      swVdebug(0,"S0050: [错误/函数调用] 共享内存指针刷新失败");
       swQuit(FAIL);
    }
    swVdebug(2,"S0060: qread2():收到信箱[%d]发来的报文,优先级=[%d],class=[%d],type=[%d]", ilOrigid, ilPrior, ilClass, ilType);
     swVdebug(3,"S0070: 报文开始");
     if (cgDebug >= 2)
    {
      swDebugmsghead((char *)&slMsgpack);
      swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
    } 
    swVdebug(3,"S0080: 报文结束"); 
 
    ilRtncode = swFormat(&slMsgpack); /* 格式转换函数 */

    switch (ilRtncode)
    {
      case 0: 
        /* 格式转换成功 */
        /* 发送报文至发送者的信箱 */
        swVdebug(2,"S0090: 发送报文(长度=%d)到邮箱[%d]!",
             slMsgpack.sMsghead.iBodylen,ilOrigid);
        swVdebug(3,"S0100: 发送报文开始");     
         if (cgDebug >= 2)
        {
          swDebugmsghead((char *)&slMsgpack);
          swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
         
        }
        swVdebug(3,"S0110: 发送报文结束");
 
        ilRtncode = qwrite2((char *)&slMsgpack,
          slMsgpack.sMsghead.iBodylen+sizeof(struct msghead), ilOrigid,
          ilPrior, ilClass, ilType);
        if (ilRtncode != SUCCESS) 
        {
          swVdebug(0,"S0120: [错误/函数调用] qwrite2()函数,返回码=%d",ilRtncode);
           swMberror(ilRtncode, NULL);
          swQuit(FAIL);
        }
        else
        swVdebug(2,"S0130: 发送打包报文到[%d]信箱成功!", ilOrigid);
         break;
      case -1:
        /* 各种类型错误 */

        /* 报文头错误 */
        /* 预解包失败 */
        /* 自定义函数计算出错(计算出交易码) */
        /* 交易码为空 */
        /* 无法定位格式转换组 */
        /* 报文转换类型与格式类型不匹配 */
        /* 根据相应格式转换组生成FML预置包出错 */
        /* MAC校验出错 */
        
        swVdebug(0,"S0140: [错误/其它] 各种类型错误");
         break;
      case -3:
        /* 不能识别的报文类型 */
        swVdebug(0,"S0150: [错误/其它] 不能识别的报文类型");
         break;
      default:
        break;
    }
  }
}

 /**************************************************************
 ** 函数名      ：swQuit
 ** 功  能      ：释放相关资源 
 ** 作  者      ：史正烨 
 ** 建立日期    ：1999/11/22
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： SUCCESS, FAIL
***************************************************************/
void swQuit(int ilSig)
{
  short ilRtncode;
  signal(ilSig, SIG_IGN);

  ilRtncode = qdetach();
  if (ilRtncode < 0)
  {
    swVdebug(0,"S0160: [错误/邮箱] qdetach()函数,错误码=%d",ilRtncode);
     exit(FAIL);
  }

  /* 向系统监控发送信息报文 */
  ilRtncode = _swMsgsend(102002, NULL);

  exit(ilSig);
}
