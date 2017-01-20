/****************************************************************/
/* 模块编号    ：FILE                                           */ 
/* 模块名称    ：文件传输                                       */
/* 版 本 号    ：V4.3.0                                         */
/* 作	 者    ：童赛丽                                         */
/* 建立日期    ：2001/05/21                                     */
/* 最后修改日期：2001/05/21                                     */
/* 模块用途    ：交换平台内部文件传输                           */
/* 本模块中包含如下函数及功能说明：                             */
/*			 （1）int  main();                      */
/*			 （2）int  swQuit();                    */
/*			 （3）int  swParase();                  */
/*			 （4）void swAlarm();                   */
/*			 （5）int swPackandqwrite( )            */
/****************************************************************/
/****************************************************************/
/* 修改记录：                                                   */
/****************************************************************/

/* switch定义 */
#include "switch.h"
#include "swNdbstruct.h"
#include "swShm.h"

#ifdef OS_SCO
#include <net/libftp.h>
#endif

#ifdef OS_AIX
#include "swftp.h"
#endif

#ifdef OS_LINUX
#include "swftp.h"
#endif

/* 变量定义 */
char agFilecode[3];   
struct msgpack sgMsgpack;   /* 报文定义 */
 
/* FTP文件传输参数结构 */
struct fileFtp
{
  char  aHost[21];
  char  aUser[21];
  char  aPasswd[21];
  char  aRempath[51];
  char  aLocalpath[51];
  char  aTranmode[7];
  short iOvertime;
};

/* Q文件传输参数结构 */
struct fileQ
{
  short iGroup;
  char  aRempath[51];
  char  aLocalpath[51];
  unsigned int iPacksize;
  short iOvertime;
  short iSafnum;
};

union filep
{
  struct fileFtp sFileftp;
  struct fileQ   sFileQ;
};  

/* Q请求包头结构 */
struct qreq
{
  char  cPackmode;  /* 1-QPUT请示包 2-QPUT响应包 3-QGET请求包 4-QGET响应包 */
  char  aRempath[51];
  short iPackid;
  unsigned int iPacksize;
}; 
     
/* Q响应包头结构 */
struct qres
{
  char  cPackmode;  /* 1-QPUT请示包 2-QPUT响应包 3-QGET请求包 4-QGET响应包 */
  char  cEndflag;
  char  cRescode;
  short iPackid;
  unsigned int iPacksize;
};

/* 函数定义 */
void swQuit(int); 
void swAlarm( );
int  swParse(char *,char *,union filep *);
int  swPackandqwrite( );
int  swClrMB1(short,short,short);
/**************************************************************
 ** 函数名      ：main
 ** 功  能      ：主函数 
 ** 作  者      ：童赛丽
 ** 建立日期    ：2001/05/21
 ** 最后修改日期：2001/05/21
 ** 调用其它函数：
 ** 全局变量    ：psgPreunpack
 ** 参数含义    ：
 ** 返回值      ：
***************************************************************/
int main(int argc,char *argv[])
{
  short  ilPriority;          /* 信箱参数:优先级 */
  short  ilClass;             /* 信箱参数:类别 */
  short  ilType;              /* 信箱参数:类型 */
  short  ilOrgqid=0;          /* 源发邮箱 */
  short  ilGroupid=0;         /* 邮箱组 */
  short  ilOrg_q;             /* 文件传输源发邮箱 */
  short  ilOrg_group;         /* 文件传输邮箱组 */
  short  ilRc;                /* 返回值 */ 
  short  ilFldlen;            /* 域长 */
  short  ilEndflag;           /* 文件结束标志 */
  unsigned int  ilReadsize;          /* 读邮箱报文长 */
  short  ilReadn;             /* 读文件长  */
  short  ilBuflen;            /* 文件包长 */
  short  m,n;
  long   llFilelen;           /* 文件长 */
  char   alFilemode[2];       /* 文件传输方式 */
  char   alFileparm[171];     /* 文件传输参数 */
  char   alBuf[iMSGMAXLEN];
  char   alMsgpack[iMSGMAXLEN];
  char   paFldvalue[5][iFLDVALUELEN];
  FTPINFO ftpinfo;
  FILE   *fp;
  struct fileFtp slFileftp;   /* FTP文件传输参数结构 */
  struct fileQ   slFileQ;     /* Q 文件传输参数结构 */
  struct qreq slQreq;         /* Q请求包 */
  struct qres slQres;         /* Q响应包 */
  struct timeb  fb;
  union  filep ulFilep;        /* 文件传输参数 */

  /* 打印版本号 */
  if (argc > 1)
    _swVersion("swFile Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swFile")) exit(FAIL);
    
  /* 设置调试程序名称 */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strncpy(agDebugfile, "swFile.debug", sizeof(agDebugfile));

  /* 取得DEBUG标志 */
  if ( (cgDebug=_swDebugflag("swFile")) == FAIL )
  {
    fprintf(stderr,"取得DEBUG标志出错!");
    exit(FAIL);
  } 

  /* 忽略SIGCLD、SIGINT、SIGQUIT、SIGHUP 信号 */
  swVdebug(2,"S0010: 忽略SIGCLD、SIGINT、SIGQUIT、SIGHUP 信号");
   
  signal( SIGTERM , swQuit  );
  signal( SIGINT  , SIG_IGN );
  signal( SIGQUIT , SIG_IGN );
  signal( SIGTSTP , SIG_IGN );
  signal( SIGHUP  , SIG_IGN );
  signal( SIGCLD  , SIG_IGN );

  /* 定位本地邮箱 */
  if (qattach(iMBFILE)) 
  {
    swVdebug(0,"S0020: [错误/邮箱] 初始化邮箱出错!");
    exit(FAIL);
  }
  swVdebug(2,"S0030: 定位本地邮箱成功");

  ilRc = swShmcheck();
  if (ilRc)
  {
    swVdebug(0,"S0040: [错误/系统调用] swShmcheck()函数");
    exit(FAIL);
  }

  ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
  if (ilRc)
  {
    swVdebug(1,"S0050: [错误/系统调用] swShmselect_swt_sys_config()函数,返回码=%d",ilRc);
    exit(FAIL);
  }
  strcpy(paFldvalue[0],sgSwt_sys_config.ftpmode);
  sprintf(paFldvalue[1],"%d",sgSwt_sys_config.ftpovertime);  
  sprintf(paFldvalue[2],"%d",sgSwt_sys_config.qpacksize);  
  sprintf(paFldvalue[3],"%d",sgSwt_sys_config.qovertime);  
  sprintf(paFldvalue[4],"%d",sgSwt_sys_config.qresendnum);  
  
  for( ; ; )
  {
    /* 从文件管理邮箱读入报文 */ 
    ilReadsize = iMSGMAXLEN;
    ilPriority = 0;
    ilClass = 0;
    ilType = 0;
    if ( ( ilRc = qread3( (char *)&sgMsgpack, &ilReadsize, &ilOrgqid,\
     &ilGroupid,&ilPriority,&ilClass,&ilType)) != SUCCESS ) 
    {
      swVdebug(1,"S0060: [错误/函数调用] qread3()函数,读邮箱出错,返回码=%d",ilRc);
      swMberror(ilRc, NULL);
      swQuit(FAIL);
    }

    /* 报文源发邮箱为路由邮箱 */
    if (ilOrgqid == iMBROUTER)
    {
      /* 将报文中IMF各域分解至FML变量预解池（调预解IMF格式函数）中 */
      ilRc = swFmlunpack( sgMsgpack.aMsgbody,
        sgMsgpack.sMsghead.iBodylen, psgPreunpackbuf );
      if ( ilRc == FAIL )
      {
        swVdebug(1,"S0070: [错误/函数调用] swFmlunpack()函数,FML报文解包出错!");
        _swMsgsend( 399001, NULL ); 
        continue;
      }
      swVdebug(2,"S0080: FML报文解包成功");
 
      /* 从报文中取出_FILEMDODE域 */
      memset(alFilemode,0x00,sizeof(alFilemode));
      ilRc = swFmlget(psgPreunpackbuf,"_FILEMODE",&ilFldlen,alFilemode);
      if (ilRc)
      {
        swVdebug(1,"S0090: [错误/函数调用] swFmlget()函数,取_FILEMODE域出错,返回码=%d",ilRc);
        continue;
      }
      swVdebug(2,"S0100: 从报文中取出FILEMODE域"); 
      
      /* 从报文中取出_FILEPARM域 */
      memset(alFileparm,0x00,sizeof(alFileparm));
      ilRc = swFmlget(psgPreunpackbuf,"_FILEPARM",&ilFldlen,alFileparm);
      if (ilRc)
      {
        swVdebug(1,"S0110: [错误/函数调用] swFmlget()函数,取_FILEPARM域出错,返回码=%d",ilRc);
        continue;
      }
      swVdebug(2,"S0120: FILEPARM[%s]",alFileparm); 
 
      /* 解析文件传输参数 */
      if ((ilRc=swParse(alFileparm,alFilemode,&ulFilep))==FAIL)
      {
        swVdebug(1,"S0130: [错误/函数调用] swParse()函数,解析文件传输参数出错,返回码=%d",ilRc);
        continue;
      }
      swVdebug(2,"S0140: 解析文件传输参数成功");
 
      slFileftp=ulFilep.sFileftp;
      if (slFileftp.aTranmode[0] =='\0')
        strcpy(slFileftp.aTranmode,paFldvalue[0]);
      if (!slFileftp.iOvertime)
        slFileftp.iOvertime = atoi(paFldvalue[1]);
 
      slFileQ=ulFilep.sFileQ;
      if (!slFileQ.iPacksize)
        slFileQ.iPacksize = atoi(paFldvalue[2]);
      if (!slFileQ.iOvertime)
        slFileQ.iOvertime = atoi(paFldvalue[3]);
      if (!slFileQ.iSafnum)
        slFileQ.iSafnum = atoi(paFldvalue[4]);
         
      strcpy(agFilecode,"0");  

      ftime(&fb);
      swVdebug(3,"S0150: 文件传输开始:[%d毫秒]",fb.millitm);
 
      switch(alFilemode[0])
      {
        /* 以FTP方式传输文件 */
        case cFTPPUT:
        case cFTPGET:
       
        /* 创建一个子进程 */
        if (fork()==0)
        {
          /* 设置超时处理 */
          signal( SIGALRM, swAlarm );
          alarm(slFileftp.iOvertime);

          /* 连接到文件传输邮箱 */
          if (qattach(iMBFILETRAN))
          {
            swVdebug(1,"S0160: [错误/邮箱] 初始化文件传输邮箱出错!");
            exit(FAIL);
          }
          swVdebug(2,"S0170: 定位文件传输邮箱成功");
 
          /* 连接到远程主机 */
          ilRc=ftp_login(&ftpinfo,slFileftp.aHost,slFileftp.aUser,\
            slFileftp.aPasswd,NULL);
          if(ilRc < 0)
          {
            swVdebug(1,"S0180: [错误/函数调用] ftp_login()函数,登陆远程主机出错,返回码=%d",ilRc); 
            strcpy(agFilecode,"-1");
            /* 报文打包后发送给路由 */
            swPackandqwrite( ); 
            exit(-1);
          }
          swVdebug(2,"S0190: 登陆远程主机成功"); 
 
          /* 设置文件传输方式 */
          if ( !strncmp(slFileftp.aTranmode,"binary",6) || \
            !strncmp(slFileftp.aTranmode,"BINARY",6)) 
          {
            ftp_binary(&ftpinfo); 
            swVdebug(2,"S0200: 设置文件传输方式[binary]");
          }
          else if ( !strncmp(slFileftp.aTranmode,"ascii",5) || \
            !strncmp(slFileftp.aTranmode,"ASCII",5)) 
          {
            ftp_ascii(&ftpinfo);
            swVdebug(2,"S0210: 设置文件传输方式[ascci]");
          }
          else
          {
            swVdebug(1,"S0220: [错误/其它] FTP文件传输模式设置错误[%s]", 
              slFileftp.aTranmode);
            strcpy(agFilecode,"-4");
            /* 报文打包后发送给路由 */
            swPackandqwrite( ); 
            exit(-1);
          }
 
          if (alFilemode[0] == cFTPPUT)
          {
            /* 向远程主机发送文件 */
            ilRc=ftp_putfile(&ftpinfo,slFileftp.aRempath,slFileftp.aLocalpath); 
            swVdebug(1,"S0230: [错误/函数调用] ftp_putfile()函数,发送文件给远程主机,返回码=%d",ilRc); 
          }  
          else
          {
            /* 从远程主机取文件 */
            ilRc=ftp_getfile(&ftpinfo,slFileftp.aRempath,slFileftp.aLocalpath); 
            swVdebug(1,"S0240: [错误/函数调用] ftp_getfile()函数,从远程主机取文,返回码=%d",ilRc); 
          }
          /* 关闭FTP连接 */
          ftp_bye(&ftpinfo);
          swVdebug(2,"S0250: 关闭FTP连接");
 
          if (ilRc < 0)
          {
            swVdebug(1,"S0260: [错误/其它] 文件传输出错[%d]",ilRc); 
            strcpy(agFilecode,"-2");
            /* 报文打包后发送给路由 */
            swPackandqwrite( );
            exit(-1);
          }
          swVdebug(2,"S0270: 传输文件[%s]成功",slFileftp.aLocalpath);
           
          ftime(&fb);
          swVdebug(2,"S0280: 文件传输结束：[%d毫秒]",fb.millitm);
 
          /* 报文打包后发送给路由 */
          swPackandqwrite( );
          exit(0);
        }
        break;
      /* 以Q方式传输 */
      case cQPUT:
      case cQGET: 

        /* 创建一个子进程 */
        if (fork()==0)
        {
          /* 设置Type为进程号 */
         ilType=getpid();

          /* 连接到文件传输邮箱 */
          if (qattach(iMBFILETRAN))
          {
            swVdebug(1,"S0290: [错误/邮箱] 初始化文件传输邮箱出错!");
            exit(FAIL);
          }
          swVdebug(2,"S0300: 定位文件传输邮箱成功");
 
          /* 清邮箱中TYPE为进程号的报文 */
          if ((ilRc=swClrMB1(iMBFILETRAN,0,ilType)) !=SUCCESS)
          {
            swVdebug(1,"S0310: [错误/邮箱] 清邮箱失败");
            exit(-1);
          }
          swVdebug(2,"S0320: 清邮箱成功");
 
          /* NEW一个请求包头 */
          memset(&slQreq,0x00,sizeof(struct qreq)); 
          strcpy(slQreq.aRempath,slFileQ.aRempath);
          slQreq.iPacksize = slFileQ.iPacksize;

          /* QPUT */
          if (alFilemode[0] == cQPUT)
          {
            slQreq.cPackmode ='1';
            /* 打开本地文件 */
            if ( (fp = fopen(slFileQ.aLocalpath,"r")) == NULL)
            { 
              swVdebug(1,"S0330: [错误/系统调用] 打开文件[%s]出错,errno=%d[%s]",slFileQ.aLocalpath,errno,strerror(errno));
              strcpy(agFilecode,"-2");
              /* 报文打包后发送给路由 */
              swPackandqwrite( );
              exit(-1);
            }
            swVdebug(2,"S0340: 打开文件[%s]",slFileQ.aLocalpath);
 
            ilEndflag=0;
            n=0;
            while (!ilEndflag)
            {
              n++;
              m = 0;
              slQreq.iPackid=n;

              /* 从文件中读出第n个文件包 */
              memset(alBuf,0x00,sizeof(alBuf));
              ilReadn=fread(alBuf,sizeof(char),slFileQ.iPacksize,fp);
              if (ilReadn != slFileQ.iPacksize)
              {
                if ( !feof(fp))
                {
                  swVdebug(1,"S0350: [错误/函数调用] fread()函数,读文件[%s]出错",slFileQ.aLocalpath);
                  fclose(fp);
                  strcpy(agFilecode,"-2");
                  /* 报文打包后发送给路由 */
                  swPackandqwrite( );
                  exit(-1);
                }
                /* 置文件结束标志 */
                ilEndflag=1;  
                fclose(fp);
                swVdebug(2,"S0360: 文件结束");
              }
              swVdebug(2,"S0370: 从文件中读出第[%d]文件包",n);
                 
              memset(alMsgpack,0,sizeof(alMsgpack));
              memcpy(alMsgpack,(char *)&slQreq,sizeof(struct qreq));
              memcpy(alMsgpack + sizeof(struct qreq),alBuf,ilReadn);
Lable1:
              /* 向远程邮箱组发第n个QPUT请求报文 */
              ilRc = qwrite3(alMsgpack, sizeof(struct qreq)+ilReadn,
                iMBFILE,slFileQ.iGroup,ilPriority,ilClass,ilType);
              if(ilRc)
              {
                fclose(fp);
                swVdebug(1,"S0380: [错误/函数调用] qwrite3()函数,写远程邮箱出错,返回码=%d",ilRc);
                strcpy(agFilecode,"-1");
                /* 报文打包后发送给路由 */
                swPackandqwrite( );
                exit(1);
              }
              swVdebug(2,"S0390: 发第[%d]个文件包成功",n);
 Lable2:
              /* 非阻塞读第n个响应报文 */
              ilOrg_group = 0;
              ilOrg_q = 0;
              ilReadsize = iMSGMAXLEN;
              ilRc = qread3nw(alMsgpack,&ilReadsize, &ilOrg_q,
                &ilOrg_group,&ilPriority,&ilClass,&ilType, slFileQ.iOvertime);
              if ((ilRc == BMQ__TIMEOUT) || (ilRc == BMQ__NOMOREMSG))
              {
                /* 读确认报文超时 */
                swVdebug(1,"S0400: [错误/函数调用] qread3nw()函数,读第[%d]个响应包超时",n);
                m++;
                if (m <= slFileQ.iSafnum)
                {
                  /* 重发第n个请求报文 */
                  swVdebug(1,"S0410: [错误/其它] 第[%d]次重发第[%d]个QPUT请求包",m,n);
                  goto Lable1;
                }
                else
                {
                  swVdebug(1,"S0420: [错误/其它] 读第[%d]个响应包失败",n);
                  fclose(fp);
                  strcpy(agFilecode,"-3");
                  /* 报文打包后发送给路由 */
                  swPackandqwrite( );
                  exit(-1);
                }
              }
              /* 读邮箱出错 */
              if (ilRc)
              {
                fclose(fp);
                strcpy(agFilecode,"-4");
                /* 打包后发给路由邮箱 */
                swPackandqwrite( );
                exit(-1);
              }
            
              memcpy( (char *)&slQres,alMsgpack,ilReadsize);
              if (slQres.iPackid != n)
              {
                /* 收到非正常响应包，丢包 */
                swVdebug(2,"S0430: 收到一个非正常响应包"); 
                goto Lable2;
              }
              /* 收到对方出错信息包 */
              if (slQres.cRescode == '1')
              {
                fclose(fp);
                swVdebug(2,"S0440: 收到对方出错信息包"); 
                strcpy(agFilecode,"-4");
                /* 报文打包后发送给路由 */
                swPackandqwrite( );
                exit(-1);
              }
              swVdebug(2,"S0450: 读第[%d]个响应包成功",n);
            }
          }
          /* QGET */
          else 
          {
            slQreq.cPackmode='3';
            /* 创建本地文件 */
            if ( (fp=fopen(slFileQ.aLocalpath,"w")) == NULL)
            { 
              swVdebug(1,"S0460: [错误/系统调用] fopen()函数,创建文件[%s]出错,errno=%d[%s]",slFileQ.aLocalpath,errno,strerror(errno));
              strcpy(agFilecode,"-2");
              /* 报文打包后发送给路由 */
              swPackandqwrite( );
              exit(-1);
            }
            swVdebug(2,"S0470: 创建文件[%s]",slFileQ.aLocalpath);
 
            n=0;
            while(1)
            {
              n++;
              slQreq.iPackid=n;
              m=0;
Lable3:
              /* 向远程邮箱组发第n个QGET请求报文 */
              if ( (ilRc=qwrite3((char *) &slQreq,sizeof(struct qreq),iMBFILE,
                 slFileQ.iGroup,ilPriority,ilClass,ilType)) != SUCCESS )
              {
                swVdebug(1,"S0480: [错误/函数调用] qwrite3()写远程邮箱出错,返回码=%d",ilRc);
                fclose(fp);
                strcpy(agFilecode,"-1");
                /* 报文打包后发送给路由 */
                swPackandqwrite( );
                exit(-1);
              }
              swVdebug(2,"S0490: 发第[%d]个QGET请求包",n);
 Lable4:                           
              /* 非阻塞读第n个QGET文件响应包 */
              ilOrg_q=0;
              ilOrg_group=0;
              ilReadsize = iMSGMAXLEN;
              ilRc =qread3nw(alMsgpack,&ilReadsize, &ilOrg_q, &ilOrg_group,
                 &ilPriority,&ilClass,&ilType, slFileQ.iOvertime); 
              if ((ilRc == BMQ__TIMEOUT) || (ilRc == BMQ__NOMOREMSG))
              {
                m++;
                if (m<=slFileQ.iSafnum)
                {
                  /* 重发第n个请求包 */
                  swVdebug(1,"S0500: [错误/其它] 第[%d]次重发第[%d]个QPUT请求包",m,n);
                  goto Lable3;
                }
                else
                {
                  swVdebug(1,"S0510: [错误/其它] 读响应包超时");
                  fclose(fp);
                  strcpy(agFilecode,"-3");
                  /* 报文打包后发送给路由 */
                  swPackandqwrite( );
                  exit(-1);
                }
              }
              if (ilRc)
              {
        	/* 读邮箱出错 */
                fclose(fp);
                strcpy(agFilecode,"-4");
                /* 打包后发给路由邮箱 */
                swPackandqwrite( );
                exit(-1);
              }

              memcpy((char *)&slQres,alMsgpack,sizeof(struct qres));
              /* 收到非正常响应包,丢包 */
              if (slQres.iPackid != n)
              {
                swVdebug(2,"S0520: 收到一个非正常响应包"); 
                goto Lable4;
              }
              /* 收到对方出错信息 */
              if (slQres.cRescode == '1')
              {
                swVdebug(2,"S0530: 收到对方出错信息"); 
                fclose(fp);
                strcpy(agFilecode,"-2");
                /* 报文打包后发送给路由 */
                swPackandqwrite( );
                exit(-1);
              }
              swVdebug(2,"S0540: 读第[%d]个QGET响应包成功",n);
 
              memset(alBuf,0,sizeof(alBuf));
              memcpy(alBuf,alMsgpack+sizeof(struct qres),slQres.iPacksize);
                  
              /* 将文件包写入本地文件 */
              ilReadn=fwrite(alBuf,sizeof(char),slQres.iPacksize,fp);
              if (ilReadn < slQres.iPacksize)
              {
                swVdebug(1,"S0550: [错误/系统调用] fwrite()写文件[%s]出错,errno=%d[%s]",slFileQ.aLocalpath,errno,strerror(errno));
                fclose(fp);
                strcpy(agFilecode,"-4");
                /* 报文打包后发送给路由 */
                swPackandqwrite( );
                exit(-1);
              }
              if (slQres.cEndflag == '1')
              {
                swVdebug(2,"S0560: 文件传输结束");
                fclose(fp);
                break;
              }
            }
          }
          ftime(&fb);
          swVdebug(3,"S0570: 文件传输结束：[%d毫秒]",fb.millitm);
 
          /* 报文打包后发送给路由 */
          swPackandqwrite( );
          exit(0);
        }
        break;
      default:
        break;
      }
    }
    /* 报文源发邮箱为文件传输邮箱 */
    else if(ilOrgqid == iMBFILETRAN)
    {
      memset(&slQreq,0x00,sizeof(slQreq));
      memcpy((char *)&slQreq,(char *)&sgMsgpack,sizeof(struct qreq));
      memset(alBuf,0x00,sizeof(alBuf));

      /* NEW 一个响应包 */
      memset(&slQres,0x00,sizeof(struct qres));
      slQres.cRescode = '0';
      slQres.cEndflag = '0';

      /* QPUT请求包 */
      if (slQreq.cPackmode == '1')
      {
        slQres.cPackmode='2';
        slQres.iPackid=slQreq.iPackid;
        ilBuflen=ilReadsize - sizeof(struct qreq);
        memcpy(alBuf,(char *)&sgMsgpack+sizeof(struct qreq),ilBuflen);

        if(slQreq.iPackid == 1)
        {
          /* 以创建本地文件 */
          if ( (fp=fopen(slQreq.aRempath,"w")) == NULL)
          {
            swVdebug(1,"S0580: [错误/系统调用] fopen()打开文件[%s]出错,errno=%d[%s]",slQreq.aRempath,errno,strerror(errno));
            slQres.cRescode='1'; 
            goto Lable5;
          }
          swVdebug(2,"S0590: 打开文件[%s]",slQreq.aRempath);
        }
        else
        {
          /* 以追加方式打开文件 */
          if ( (fp=fopen(slQreq.aRempath,"a+")) == NULL)
          {
            swVdebug(1,"S0600: [错误/系统调用] fopen()打开文件[%s]出错,errno=%d[%s]",slQreq.aRempath,errno,strerror(errno));
            slQres.cRescode='1'; 
            goto Lable5;
          }
 
          /* 校验文件包合法性 */
          ilRc=fseek(fp, 0, SEEK_END);
          if ( ilRc ) 
          {
            swVdebug(1,"S0610: [错误/系统调用] fseek()操作文件出错,error=%d[%s]",errno,strerror(errno));
            slQres.cRescode='1'; 
            goto Lable5;
          }
          llFilelen = ftell(fp);
          if (llFilelen != (slQreq.iPackid - 1) * slQreq.iPacksize)
          {
            swVdebug(2,"S0620: [错误/其它] 文件包不合法");
            continue;
          }
        }

        /* 将文件包写入文件 */
        ilReadn=fwrite(alBuf,sizeof(char),ilBuflen,fp);
        /* 关闭文件 */
        fclose(fp);
        if (ilReadn != ilBuflen)
        {
          swVdebug(2,"S0630: 写文件[%s]出错",slQreq.aRempath);
          slQres.cRescode='1'; 
          goto Lable5;
        }
        swVdebug(2,"S0640: 写文件[%s]",slQreq.aRempath);
 
Lable5:
        /* 发送响应包给文件传输邮箱 */
        if ( ( ilRc = qwrite3( (char *)&slQres,sizeof(struct qres),
           ilOrgqid,ilGroupid,ilPriority,ilClass,ilType )) != SUCCESS)
        {
          swVdebug(2,"S0650: 发送响应包出错");
          swMberror(ilRc, "错误:qwrite至文件传输邮箱出错!");
          swQuit(FAIL);
        }
        swVdebug(2,"S0660: 发送第[%d]个响应包",slQreq.iPackid);
      }

      /* QGET请求包 */
      else if (slQreq.cPackmode == '3')
      {
        slQres.cPackmode='4';
        slQres.iPackid=slQreq.iPackid;
        ilReadn = 0;
        /* 以只读方式打开文件 */
        if ((fp=fopen(slQreq.aRempath,"r")) == NULL)
        {
          swVdebug(2,"S0670: 打开文件[%s]出错",slQreq.aRempath);
          slQres.cRescode='1'; 
          goto Lable6;
        }
        swVdebug(2,"S0680: 打开文件[%s]",slQreq.aRempath);
 
        /* 读第N个文件包 */
        ilRc = fseek(fp,(slQreq.iPackid-1)*slQreq.iPacksize,SEEK_SET); 
        if (ilRc)
        {
          swVdebug(2,"S0690: 操作文件出错:fseek():error");
          slQres.cRescode='1'; 
          goto Lable6;
        }

        ilReadn=fread(alBuf,sizeof(char),slQreq.iPacksize,fp);
        if( ilReadn != slQreq.iPacksize )
        {
          /* 文件未结束 */
          if (! feof (fp) )
          {
            /* 读文件出错 */
            swVdebug(2,"S0700: 读文件[%s]出错",slQreq.aRempath);
            /*置响应报文出错,关闭文件 */
            slQres.cRescode='1'; 
          }
          else
          {
            swVdebug(2,"S0710: 文件[%s]结束",slQreq.aRempath);
            /* 关闭文件, 置文件结束 */
            slQres.cEndflag='1';
          }
        }
        fclose(fp);
        swVdebug(2,"S0720: 读文件[%s]成功",slQreq.aRempath);
 
Lable6:
        slQres.iPacksize = ilReadn;
        memset(alMsgpack,0x00,sizeof(alMsgpack));
        memcpy(alMsgpack,(char *)&slQres,sizeof(struct qres));
        memcpy(alMsgpack+sizeof(struct qres),alBuf,ilReadn);
   
        /* 发送响应包给文件传输邮箱 */
        if ( ( ilRc = qwrite3( (char *)&alMsgpack,sizeof(struct qres) + ilReadn,
            ilOrgqid,ilGroupid,ilPriority,ilClass,ilType )) != SUCCESS)
        {
          swVdebug(2,"S0730: 发送响应包出错");
          swMberror(ilRc, "错误:qwrite至文件传输邮箱出错!");
          swQuit(FAIL);
        }
        swVdebug(2,"S0740: 发送第[%d]个响应包", slQreq.iPackid);
      }
    }
    else continue; 
  }
}

/**************************************************************
 ** 函数名:   swQuit()
 ** 功  能:   程序退出
 ** 作  者:   童赛丽
 ** 建立日期: 2001.05.23
 ** 最后修改日期:2001.05.23
 ** 调用其它函数:   
 ** 全局变量:  
 ** 参数含义:       
 ** 返回值:
***************************************************************/
void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swVdebug(0,"S0750: [swFile]已经停止!");
  qdetach();
  exit(sig);
}

/**************************************************************
 ** 函数名:   swParse()
 ** 功  能:   解析文件传输参数
 ** 作  者:   童赛丽
 ** 建立日期: 2001.05.23
 ** 最后修改日期:2001.05.23
 ** 调用其它函数: _swGetOneField()
 ** 全局变量:  
 ** 参数含义:       
 ** 返回值: 成功:SUCCESS  失败:FAIL
***************************************************************/

int swParse(char *alBuf,char *alMode ,union filep *ulFilep)
{
   char alStr[7][51];
   int i;
   
   swVdebug(4,"S0760: [函数调用] swParse()");
   for(i = 0;i<7;i++)
     _swGetOneField(alBuf,i+1,alStr[i],',');

   if ( alMode[0] == cFTPPUT || alMode[0] == cFTPGET)
   {
       strcpy(ulFilep->sFileftp.aHost,alStr[0]);
       strcpy(ulFilep->sFileftp.aUser,alStr[1]);
       strcpy(ulFilep->sFileftp.aPasswd,alStr[2]);
       strcpy(ulFilep->sFileftp.aLocalpath,alStr[3]);
       strcpy(ulFilep->sFileftp.aRempath,alStr[4]);
       strcpy(ulFilep->sFileftp.aTranmode,alStr[5]);
       ulFilep->sFileftp.iOvertime=atoi(alStr[6]);
       swVdebug(4,"S0770: [函数返回] swParse()返回码=0");
       return(SUCCESS);
   }
   else if(alMode[0] == cQPUT || alMode[0] == cQGET)
   {
      ulFilep->sFileQ.iGroup=atoi(alStr[0]);
      strcpy(ulFilep->sFileQ.aLocalpath,alStr[1]);
      strcpy(ulFilep->sFileQ.aRempath,alStr[2]);
      ulFilep->sFileQ.iPacksize=atoi(alStr[3]);
      ulFilep->sFileQ.iOvertime=atoi(alStr[4]);
      ulFilep->sFileQ.iSafnum=atoi(alStr[5]);
      return(SUCCESS);
   }
   else return(FAIL);

}

/**************************************************************
 ** 函数名:   swAlarm()
 ** 功  能:   超时处理
 ** 作  者:   童赛丽
 ** 建立日期: 2001.05.25
 ** 最后修改日期:2001.05.25
 ** 调用其它函数:   
 ** 全局变量:   psgPreunpackbuf,agFilecode 
 ** 参数含义:       
 ** 返回值:
***************************************************************/
void swAlarm( )
{
   signal(SIGALRM,SIG_IGN);
   swVdebug(0,"S0780: [错误/其它] 文件传输超时"); 
   strcpy(agFilecode,"-3");
   /* 报文打包后发送给路由 */
   swPackandqwrite( );
   exit(-1);
} 

/**************************************************************
 ** 函数名:   swPackandqwrite
 ** 功  能:   打包后发送报文到路由邮箱
 ** 作  者:   童赛丽
 ** 建立日期: 2001.05.25
 ** 最后修改日期:2001.05.25
 ** 调用其它函数:   
 ** 全局变量:   psgPreunpackbuf 
 ** 参数含义:       
 ** 返回值:
***************************************************************/
int swPackandqwrite( ) 
{
   short  ilRc;
   short  ilPriority=0;          /* 信箱参数:优先级 */
   short  ilClass=0;             /* 信箱参数:类别 */
   short  ilType=0;              /* 信箱参数:类型 */

   swVdebug(4,"S0790: [函数调用] swPackandqwriter()");
   /* 设置 _FILECODE域 */
   ilRc = swFmlset("_FILECODE",strlen(agFilecode),agFilecode,psgPreunpackbuf);
   if(ilRc != SUCCESS)
   {
     swVdebug(1,"S0800: [错误/函数调用] swFmlset()[_FILECODE]域出错,返回码=%d",ilRc);
     exit(FAIL);
   }
   swVdebug(2,"S0810: 设置 _FILECODE域[%s]",agFilecode);
 
   /* 报文重新打包 */
   ilRc = swFmlpack(psgPreunpackbuf,sgMsgpack.aMsgbody,
     &(sgMsgpack.sMsghead.iBodylen));
   if ( ilRc == FAIL )
   {
     swVdebug(1,"S0820: [错误/函数调用] swFmlpack()生成FML报文失败");
     _swMsgsend( 302016, NULL );
     exit(FAIL);
   }
   swVdebug(2,"S0830: 报文重新打包"); 
 
   /* 将报文发送到路由邮箱 */
   if ( ( ilRc = qwrite2( (char *)&sgMsgpack,
       sgMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
       iMBROUTER,ilPriority,ilClass,ilType )) != SUCCESS)
   {
     swVdebug(0,"S0840: [错误/函数调用] qwrite2()发送报文至路由邮箱出错,返回码=%d",ilRc);
     swMberror(ilRc, "错误:qwrite至路由箱出错!");
     exit(FAIL);
   }
   swVdebug(0,"S0850: 发送报文至路由邮箱成功");
   if (cgDebug>=2) 
     swDebugfml( (char *)&sgMsgpack);
   swVdebug(4,"S0860: [函数返回] swPackandqwriter()返回码=0");  
   return(SUCCESS); 
} 
/**************************************************************
 ** 函数名:   swClrMB1()
 ** 功  能:   选择清除邮箱中报文
 ** 作  者:   童赛丽
 ** 建立日期: 2001.05.23
 ** 最后修改日期:2001.05.23
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值: 成功：SUCCESS
***************************************************************/
int swClrMB1(short ilOrgqid,short ilClass,short ilType)
{
  short ilRn;
  unsigned int ilMsglen;
  short ilPriority;
  char slMsgbuf[iMSGMAXLEN];
  
  swVdebug(4,"S0870: [函数调用] swClrMB1(%d,%d,%d)",ilOrgqid,ilClass,ilType);
  while(1)
  {
    ilMsglen = iMSGMAXLEN;
    ilPriority = 0;

    ilRn = qreadnw( (char *)&slMsgbuf, &ilMsglen, &ilOrgqid,&ilPriority, 
      &ilClass, &ilType, 1 );
    if( (ilRn == BMQ__NOMOREMSG) || (ilRn == BMQ__TIMEOUT)) 
      break;
    else if (ilRn)
      return(ilRn);
  }
  swVdebug(4,"S0880: [函数返回] swClrMB1()返回码=0");
  return(SUCCESS);
}

