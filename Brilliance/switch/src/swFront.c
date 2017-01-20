/****************************************************************/
/* 模块编号    ：swFront                                        */ 
/* 模块名称    ：通用前置进程                                   */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/10/05                                     */
/* 最后修改日期：2001/7/12                                      */
/* 模块用途    ：前置进程                                       */
/* 本模块中包含如下函数及功能说明：                             */
/*	        (1) void main();                                */
/*              (2) void swQuit(int);                           */
/****************************************************************/
/* 头文件定义 */
#include <signal.h>
#include "swapi.h"
#include "swConstant.h"

char cgDebug=0;
/* 函数原型 */
void swQuit(int);

extern int swRecvpacklw(short *, char *, unsigned int *, short *, long *, long *);
extern int swSendpackl(short, char *, unsigned int, short, long, long);
extern int swDbopen();
extern int swDbclose();
 
/**************************************************************
 ** 函数名      ： main
 ** 功  能      ： 主函数
 ** 作  者      ： nh 
 ** 建立日期    ：
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ： 无
***************************************************************/
int main(argc,argv)
 int argc;
 char **argv;
{
  struct msgpack slMsgpack;             /* 报文 */
  short  ilQid_fore,ilQid_comm;         /*前置邮箱和通讯邮箱*/
  short  ilRc;                          /* 返回码 */
  unsigned int ilMsglen;                      /* 信息长度 */
  short  ilQid;                         /* 信箱号 */
  short  ilPriority;
  long   llClass,llType;
  short  ilStaflag = 0;
  short  ilTypeflag = 0;
  int    c;

  /* 处理信号 */
  signal(SIGTERM,swQuit);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGCLD, SIG_IGN); 

  c=0; 
  while((c=getopt(argc,argv,"f:c:d:ts")) != -1)
  {
    switch( c )
    {
      case 'f':  /* 定义前置邮箱 */
        ilQid_fore=atoi(optarg);
        break;
      case 'c':  /* 定义通讯邮箱 */
        ilQid_comm=atoi(optarg);
        break;
      case 'd':  /* 定义TRACE级别 */
        cgDebug=atoi(optarg);
        break;
      case 't':  /* 定义启用type,class标识 */
        ilTypeflag = 1;
	break;
      case 's':  /* 定义交易源发端口标识 */
        ilStaflag = 1;
        break;
      default:
        break;
    }
  }
  
  if ((ilQid_fore&&ilQid_comm)==0) 
  {
    printf("使用方法: swFront -f 前置邮箱 -c 通讯邮箱 [-d TRACE级别] [-t] [-s]\n");
    printf("           -t: 启用报文过滤条件(type,class)\n");
    printf("           -s: 交易源发端口\n");
    exit( 0 );
  }

  /* 设置调试信息文件 */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  sprintf(agDebugfile,"swFront_%d.debug",ilQid_fore);

  swVdebug(2,"S0000: 前置邮箱号=%d",ilQid_fore);
  swVdebug(2,"S0010: 通讯邮箱号=%d",ilQid_comm);
  swVdebug(2,"S0020: TRACE级别=%d",cgDebug); 
  swVdebug(2,"S0030: 启用type,class标识=%d",ilTypeflag);
  swVdebug(2,"S0030: 交易源发端口标识=%d",ilStaflag);

  swVdebug(2,"S0030: FLDNAMELEN=%d",iFLDNAMELEN);
  /* 连接平台共享内存 */
  ilRc = swShmcheck();
  if (ilRc)
  {
    swVdebug(0,"S0040: 检测共享内存出错!");
    exit(-1);
  }
  swVdebug(2,"S0045: 连接共享内存成功");

  /* 初始化前置进程 */
  ilRc = swInit(ilQid_fore);
  if (ilRc) 
  {
    swVdebug(0,"S0050: swInit():初始化前置进程失败!");
    exit(-1);
  }
  swVdebug(2,"S0060: 初始化前置进程(%d)成功",ilQid_fore);

  /* 初始化前置进程的信箱 */
  ilRc = swMbopen(ilQid_fore);
  if (ilRc) 
  {
    swVdebug(0,"S0070: swMbopen():邮箱初始化失败!");
    exit(-1);
  }
  swVdebug(2,"S0080: 初始化前置进程邮箱(%d)成功",ilQid_fore);

  /* 连接数据库 */
  ilRc = swDbopen();
  if (ilRc) 
  {
    swVdebug(0,"S0090: swDbopen():失败!");
    exit(-1);
  }

  /* 循环读取邮箱的报文，并进行相应的处理 */
  for(;;)
  {
    ilQid=0;
    llClass = 0;
    llType = 0;
    ilPriority = 0;
    ilMsglen = iMSGMAXLEN;

    /* 接收报文 */
    ilRc = swRecvpacklw(&ilQid,(char *)&slMsgpack,&ilMsglen,&ilPriority, \
      &llClass,&llType);
    if (ilRc) 
    {
      swVdebug(1,"S0100: swRecvpackw():接收报文出错,ilRc=[%d]",ilRc);
      swQuit(-1);
    }
    swVdebug(2,"S0110: 接收到从邮箱[%d]发来的报文,prior=[%d],class=[%d],type=[%d]",ilQid,ilPriority,llClass,llType);

/* 如果支持配置动态刷新,需要去掉注释 */
    ilRc = swShmcheck();
    if (ilRc)
    {
      swVdebug(2,"S0120: 检测共享内存出错!");
      swQuit(-1);
    }
/**/

/*
    if (ilQid==iMBCTRL)  
      swVdebug(2,"S0130: 处理CTRL信箱发来的报文");

    if ((ilQid==ilQid_comm)&&(ilStaflag)) 
      swVdebug(2,"S0140: 新交易开始:处理外部通讯信箱发来的报文");
*/

    swVdebug(3,"S0150: 报文BEGIN.....");
    if(cgDebug >= 2)
      swDebughex((char *)&slMsgpack,ilMsglen);
    swVdebug(3,"S0160: 报文END.....");

    /* 需要加上平台内部的报文头(新交易), 并保存llCass,llType*/
    if ((ilQid==ilQid_comm)&&(ilStaflag)) 
    {
      swVdebug(2,"S0170: 生成新交易");
/* 如果通讯发来报文不含报文头,需要取消注释
      memmove((char *)&slMsgpack+sizeof(struct msghead),(char *)&slMsgpack,ilMsglen);
      ilMsglen = ilMsglen + sizeof(struct msghead);
*/
      swNewtran(&(slMsgpack.sMsghead)); 
      slMsgpack.sMsghead.iBodylen = ilMsglen - sizeof(struct msghead);
    }
    /* 保存llClass,llType*/
    if ((ilQid==ilQid_comm)&&(ilTypeflag))
    {
      swVdebug(2,"S0180: 保存type,class到报文头中的aMemo域");
      memcpy(slMsgpack.sMsghead.aMemo,(char *)&llClass,sizeof(long));
      memcpy(slMsgpack.sMsghead.aMemo + sizeof(long),(char *)&llType,sizeof(long));
    }

    if(cgDebug >= 2)
      swDebugmsghead((char *)&(slMsgpack.sMsghead));
    if(cgDebug >= 2)
      swDebughex(slMsgpack.aMsgbody,ilMsglen-sizeof(struct msghead));

    switch(slMsgpack.sMsghead.iMsgtype)
    {
      case iMSGAPP:  /* 应用报文 */
        if (ilQid == ilQid_comm)
        {
          /* 通讯端口发来的应用报文*/
          swVdebug(2,"S0210: 收到通讯端口[%d]的应用报文",ilQid_comm);

          /* 置报文类型为解包报文,以准备进行解包操作 */
          slMsgpack.sMsghead.iMsgtype = iMSGUNPACK;
          /* 将源邮箱置成目标邮箱 */
          slMsgpack.sMsghead.iOrg_q = slMsgpack.sMsghead.iDes_q;
          /* 目标邮箱置成0 */
          slMsgpack.sMsghead.iDes_q = 0;
          slMsgpack.sMsghead.iBodylen = ilMsglen - sizeof(struct msghead);
          /* 报文头中的报文格式置成端口指定的报文格式(8583,SEP,NOSEP,FML,XMF) */
          slMsgpack.sMsghead.iMsgformat = sgPortattrib.iMsgformat;
	  swVdebug(2,"S0220: 调用swFormat函数进行解包操作");
           
          ilRc=swFormat(&slMsgpack);
          if (ilRc)
          {
            swVdebug(2,"S0230: 调用格式转换函数swFormat()出错,返回码=%d",ilRc);
            continue;
          }
       
/*
          ilRc = swFmlpackget(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,"a18",&ilFldlen, alTmp);
          if ( ilRc )
          {
            swVdebug(0,"S0340: 应用报文中无域[a19]");
            continue;
          }
          swVdebug(2,"错误代码[%s]",alTmp);
          ilRc = swFmlpackget(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,"a19",&ilFldlen, alTmp);
          if ( ilRc )
          {
            swVdebug(2,"S0340: 应用报文中无域[a19]");
            continue;
          }
          swVdebug(2,"错误提示:[%s]",alTmp);
*/

          /* 置报文类型为应用报文 */
          slMsgpack.sMsghead.iMsgtype = iMSGAPP;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);

	  swVdebug(2,"S0225: 发送应用报文到交换主控");
          ilRc = swSendpackl(iMBCTRL,(char *)&slMsgpack,ilMsglen,ilPriority, 
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0240: 发送到主控邮箱[%d]出错[%d]",iMBCTRL,ilRc);
            continue;
          }
          continue;
        }
        else if(ilQid==iMBCTRL) 
        {
          /* 主控发来的应用报文 */
          swVdebug(2,"S0260: 收到主控发来的应用报文");  

          /* 置报文类型为打包报文,以准备进行打包操作 */
          slMsgpack.sMsghead.iMsgtype = iMSGPACK; 

	  swVdebug(2,"S0265: 调用swFormat函数进行打包操作");
          ilRc=swFormat(&slMsgpack);
          if (ilRc)
          {
            swVdebug(2,"S0270: 调用格式转换函数swFormat()出错,返回码=%d",ilRc);
            continue;
          }

          /* 置报文类型为应用报文 */
          slMsgpack.sMsghead.iMsgtype = iMSGAPP;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);
          if (ilTypeflag == 1)
          {
            /* 在报文头aMemo域中取出type,class域 */
	    /*
            llClass = *((long *)slMsgpack.sMsghead.aMemo);
            llType = *((long *)(slMsgpack.sMsghead.aMemo + sizeof(long)));
            */
   memcpy(&llClass,slMsgpack.sMsghead.aMemo,sizeof(long));
   memcpy(&llType,slMsgpack.sMsghead.aMemo + sizeof(long),sizeof(long));
          }
          else
          {
            llClass = 0;
            llType = 0;
          }
          swVdebug(2,"S0000: 发送应用报文到通讯邮箱[%d]: class=[%ld],type=[%ld]",ilQid_comm,llClass,llType);
          ilRc = swSendpackl(ilQid_comm,(char *)&slMsgpack,ilMsglen,ilPriority,
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0280: 发送报文给通讯邮箱[%d]出错[%d]",ilQid_comm,ilRc);
            continue;
          }
          continue;
        }
        else 
        {
          swVdebug(2,"S0300: 收到未知信箱[%d]发来的应用报文",ilQid);
          continue;
        }
        break;

      case iMSGREV:   /* 冲正报文 */
        if ((ilQid==iMBMONREV) || (ilQid==iMBCTRL))
        {
          swVdebug(2,"S0310: 收到邮箱[%d]发来的冲正报文",ilQid);
          /* 置报文类型为冲正打包,以进行打包操作 */
          slMsgpack.sMsghead.iMsgtype = iMSGREVPACK;     

	  swVdebug(2,"S0265: 调用swFormat函数进行冲正打包操作");
          ilRc=swFormat(&slMsgpack);
          if (ilRc)
          {
            swVdebug(2,"S0320: 调用格式转换函数swFormat()出错,返回码=%d",ilRc);
            continue;
          }

          /* 置报文类型为冲正报文 */
          slMsgpack.sMsghead.iMsgtype = iMSGREV;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);
          if (ilTypeflag == 1)
          {
	  /*
            llClass = *((long *)slMsgpack.sMsghead.aMemo);
            llType = *((long *)(slMsgpack.sMsghead.aMemo + sizeof(long)));
          */
   memcpy(&llClass,slMsgpack.sMsghead.aMemo,sizeof(long));
   memcpy(&llType,slMsgpack.sMsghead.aMemo + sizeof(long),sizeof(long));
          }
          else
          {
            llClass = 0;
            llType = 0;
          }
          swVdebug(2,"S0000: 发往通讯邮箱[%d]: class=[%ld],type=[%ld]",ilQid_comm,llClass,llType);

          ilRc = swSendpackl(ilQid_comm,(char *)&slMsgpack,ilMsglen,ilPriority,
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0330: 发送冲正报文到通讯邮箱[%d]失败![%d]",
              ilQid_comm,ilRc );
            continue;
          }
          continue;
        }
        else if (ilQid == ilQid_comm) 
        {
          swVdebug(2,"S0350: 收到通讯端口[%d]的冲正报文",ilQid_comm);
          /* 置报文类型为冲正解包,以进行解包操作 */
          slMsgpack.sMsghead.iMsgtype = iMSGREVUNPACK;
          slMsgpack.sMsghead.iOrg_q=slMsgpack.sMsghead.iDes_q;
          slMsgpack.sMsghead.iDes_q=0;
          slMsgpack.sMsghead.iBodylen=ilMsglen-sizeof(struct msghead);
          slMsgpack.sMsghead.iMsgformat=sgPortattrib.iMsgformat;

	  swVdebug(2,"S0265: 调用swFormat函数进行冲正打包操作");
          ilRc=swFormat(&slMsgpack);
          if (ilRc != SUCCESS)
 	  {
            swVdebug(2,"S0360: 调用格式转换函数swFormat()出错,code=%d",ilRc);
            continue;
          }

          slMsgpack.sMsghead.iMsgtype = iMSGREV;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);

          swVdebug(2,"S0000: 发送冲正报文到交换主控");
          ilRc = swSendpackl(iMBCTRL,(char *)&slMsgpack,ilMsglen,ilPriority,
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0370: 发送冲正报文给主控邮箱出错,返回码=%d",ilRc);
            continue;
          }
          continue;
        }
        else
        {
          swVdebug(2,"S0390: 收到未知邮箱[%d]发来的冲正报文",ilQid);
          continue;
        }
        break;

      default :
        swVdebug(2,"S0400: 收到未知报文类型[%d]的报文",slMsgpack.sMsghead.iMsgtype);
        break;
    }
  }
}

/**************************************************************
 ** 函数名      ：swQuit
 ** 功  能      ：释放相关资源,关闭数据库 
 ** 作  者      :
 ** 建立日期    ：2000/5/8
 ** 最后修改日期：2000/5/8
 ** 调用其它函数：
 ** 全局变量    ：
 ** 参数含义    ：
 ** 返回值      ：
***************************************************************/
void swQuit(int sig)
{
  short ilRtncode;

  signal(SIGTERM,SIG_IGN);

  ilRtncode = swDbclose();
  if (ilRtncode)
  {
     swVdebug(0,"S0410: 关闭数据库失败!");
  }

  /* 释放资源 */
  ilRtncode=swMbclose();
  if (ilRtncode) 
  {
    swVdebug(0,"S0420: 关闭邮箱出错!");
  }
  exit(0);
} 
