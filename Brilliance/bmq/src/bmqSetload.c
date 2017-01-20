/****************************************************************/
/* 模块编号    ：BMQSETLOAD                                     */
/* 模块名称    ：参数载入模块                                   */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：xujun                                          */
/* 建立日期    ：2001/08/08                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：				                */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共194行                              */
/****************************************************************/

/*库函数定义*/
#include "bmq.h"

int main(int argc,char **argv)
{
  int   ilRc,i;
  char  alBuf[10];
  char  alFp_name[80];
  char  alPro_name[20];
  int   ilGroupid;

  printf("-------------------------------------------------------------------------------\n");
  printf("***修改参数请注意以下事项:***\n");
  printf("-------------------------------------------------------------------------------\n");
  printf("* BMQ----共享内存参数的修改需要重新启动新晨Q!\n");
  printf("* SHMK---共享内存ID参数在新晨Q运行时不能修改!\n");
  printf("* GROUP--级联时新晨Q组配置修改后需要重新运行");
  printf("  bmqGrp_rcv和bmqGrp_snd方能生效!\n");
  printf("* WTIME--系统轮询时间的修改需要重新运行bmqclean方能生效!\n");
  printf("* MAIL---邮箱配置的修改将在运行本进程后生效!\n");
  printf("* FILE---文件传输配置的修改将在重新运行FILE后生效!\n");
  printf("-------------------------------------------------------------------------------\n");
  printf("\n[1] 重起 文件传输4个进程");
  printf("\n[2] 重起 跨组守护进程");
  printf("\n[3] 重起 bmqclean");
  printf("\n[4] 需要刷新邮箱配置");
  printf("\n请输入选项[1~4]:");

  scanf("%s",alBuf);
  i  = atoi(alBuf);
  /*连接共享内存*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    printf("连接共享内存区出错:%d\n",ilRc);
    exit(FAIL);
  }
  switch(i)
  {
    case 1:
           printf("\n确认重起文件传输4个守护进程否[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("没有重起文件传输4个守护进程!\n");
             exit(SUCCESS);
           }
           else
           {
             if( psgMbshm->lBmqfilecls_pid > 0 )
               kill(psgMbshm->lBmqfilecls_pid,SIGTERM);
             if( psgMbshm->lBmqfilemng_pid > 0 )
               kill(psgMbshm->lBmqfilemng_pid,SIGTERM);
             if( psgMbshm->lBmqfilesaf_pid > 0 )
               kill(psgMbshm->lBmqfilesaf_pid,SIGTERM);
             if( psgMbshm->lBmqfilelog_pid > 0 )
               kill(psgMbshm->lBmqfilelog_pid,SIGTERM);
             if( fork() == 0)
             {
               sprintf(alFp_name,"%s/bin/bmqfilecls",getenv("BMQ_PATH"));
               sprintf(alPro_name,"%s","bmqfilecls");
               ilRc = execl(alFp_name,alPro_name,(char *)0);
               exit(0);
             }
             if( fork() == 0)
             {
               sprintf(alFp_name,"%s/bin/bmqfilemng",getenv("BMQ_PATH"));
               sprintf(alPro_name,"%s","bmqfilemng");
               ilRc = execl(alFp_name,alPro_name,(char *)0);
               exit(0);
             }
             if( fork() == 0)
             {
               sprintf(alFp_name,"%s/bin/bmqfilesaf",getenv("BMQ_PATH"));
               sprintf(alPro_name,"%s","bmqfilesaf");
               ilRc = execl(alFp_name,alPro_name,(char *)0);
               exit(0);
             }
             if( fork() == 0)
             {
               sprintf(alFp_name,"%s/bin/bmqfilelog",getenv("BMQ_PATH"));
               sprintf(alPro_name,"%s","bmqfilelog");
               ilRc = execl(alFp_name,alPro_name,(char *)0);
               exit(0);
             }
           }
           printf("\n文件传输4个守护进程已重起!\n");
           break;
    case 2:
           /*判断组的链接类型*/
           /*取得组号环境变量*/
           ilGroupid = atoi(getenv("BMQ_GROUP_ID"));
           if(ilGroupid == 0)
           {
             _bmqDebug("\n读系统环境变量[BMQ_GROUP_ID]失败\n");
             return(FAIL);	
           }
    
           /*读入参数配置文件GROUP*/
           ilRc = _bmqConfig_load("GROUP",ilGroupid);
           if (ilRc < 0)
           {
             printf("\n从文件$BMQ_PATH/etc/bmq.ini中取GROUP值错误,启动失败!\n");
           }
           printf("\n确认重起跨组守护进程否[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("没有重起跨组守护进程!\n");
             exit(SUCCESS);
           }
           else
           { 
             if( psgMbshm->lBmqgrpsnd_pid > 0 )
                 kill(psgMbshm->lBmqgrpsnd_pid,SIGTERM);
             if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_snd",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_snd");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);
               }
               
             if(agCommode[0] == 'L')
             {
               if( psgMbshm->lBmqgrprcv_pid > 0 )
                 kill(psgMbshm->lBmqgrprcv_pid,SIGTERM);               
               if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_rcv",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_rcv");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);
               }
               printf("\nbmqGrp_rcv、bmqGrp_snd已重起!\n");
             }
             if(agCommode[0] == 'S')
             {
               if( psgMbshm->lBmqgrprcvs_pid > 0 )
                  kill(psgMbshm->lBmqgrprcvs_pid,SIGTERM);
               if( psgMbshm->lBmqgrprcvw_pid > 0 )
                  kill(psgMbshm->lBmqgrprcvw_pid,SIGTERM);  
               if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_rcvs",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_rcvs");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);
               }  	
               if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_rcvw",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_rcvw");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);                 
               }
               printf("\nbmqGrp_rcvs、bmqGrp_rcvw、bmqGrp_snd已重起!\n");	            	
             }             
           }           
           break;
    case 3:
           printf("\n确认重起bmqclean否[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("没有重起bmqclean!\n");
             exit(SUCCESS);
           }
           else
           {
             if( psgMbshm->lBmqclean_pid > 0 )
               kill(psgMbshm->lBmqclean_pid,SIGTERM);
             if( fork() == 0)
             {
               sprintf(alFp_name,"%s/bin/bmqclean",getenv("BMQ_PATH"));
               sprintf(alPro_name,"%s","bmqclean");
               ilRc = execl(alFp_name,alPro_name,(char *)0);
               exit(0);
             }
           }
           printf("\nbmqclean已重起!\n");
           break;
    case 4:
           printf("\n确认刷新[mail]设置否[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("没有刷新[mail]设置!\n");
             exit(SUCCESS);
           }
           else
           {
             ilRc = _bmqLock(LK_ALL);
             if( ilRc )
             {
               printf("信号灯操作失败");
               printf("[MAIL]邮箱配置未刷新!\n");
               exit(FAIL);
             }
             /*载入邮箱驻留时间和阀值参数*/
             ilRc = _bmqConfig_load("MAIL",0);
             if (ilRc < 0)
             {
               printf("载入MAIL参数失败，请查看参数文件%s/etc/bmq.ini\n",
                 getenv("BMQ_PATH"));
               printf("[MAIL]邮箱配置未刷新!\n");
               exit(FAIL);
             }
             _bmqLoadRouterinfo();
             _bmqUnlock(LK_ALL);

             printf("\n[MAIL]邮箱配置已刷新!\n");
           }
           break;
    default:printf("\n选择不合法!\n");
            break;
  }
  exit(SUCCESS);
}
