/****************************************************************/
/* 模块编号    ：swTidTimCache                                      */
/* 模块名称    ：流水号定时缓存模块                             */
/* 版 本 号    ：V1.0.0                                         */
/* 作	 者    ：陈继辉                                         */
/* 建立日期    ：2014/12/20                                     */
/* 最后修改日期：2014/12/20                                     */
/* 模块用途    ：用于定时缓存流水号到文件                       */
/* 本模块中包含如下函数及功能说明：                             */
/*			 （1）int main();                                   */
/****************************************************************/

/****************************************************************/
/* 修改记录:                                                    */
/*  2014.12.20 创建                                             */
/****************************************************************/
#include "switch.h"
#include "swShmGroup.h"
#include "swNdbstruct.h"
#include "swShm.h"

extern struct shmidx_d *psmShmidx_d;   /* 动态表共享内存目录段指针 */
int lock_set(int fd,int type);

int main(int argc, char **argv)
{
	int ilShmid;
	long ltranid,lsafid;
	char *alShmtmp_d;
	FILE *fp;
    int time; /*间隔时间*/
	char aFilename[100];
    int fd;
    char str[50];
	
	/* 设置调试程序名称 */
	swLogInit("swTidTimCache", NULL); /*add by dyw, 2015.03.27, PSBC_V1.0*/
    strcpy(agDebugfile, "swTidTimCache.log");
	
	/* 取得DEBUG标志 */
	if ((cgDebug = _swDebugflag("swTidTimCache")) == FAIL)
	{
		printf("无法取得DEBUG标志!\n");
		exit(FAIL);
	}
	swVdebug(2,"S0000: 取Debug标志ok!");
	
	memset(aFilename,0x00,sizeof(aFilename));
	sprintf( aFilename,"%s/config/SWBUFSPE.CFG",getenv("SWITCH_CFGDIR") );
    if((fp = fopen(aFilename,"r"))==NULL)
    {
        printf("打开配置文件失败\n");
        exit(FAIL);
    }
    fscanf(fp,"%d",&time);
	swVdebug(2,"S0010:间隔时间time[%d]",time);
    fclose(fp); 

	  /* 初始化共享内存指针 */
  if(swShmcheck() != SUCCESS)
  {
    swVdebug(0,"S0080: [错误/共享内存] 初始化共享内存指针失败");
    exit(FAIL);
  }
  swVdebug(2,"S0090: 初始化共享内存指针成功");
	
	while(1)
	{		

	  /* 刷新共享内存指针 */
        if(swShmcheck() != SUCCESS)
        {
            swVdebug(0,"S0080: [错误/共享内存] 初始化共享内存指针失败");
            exit(FAIL);
        }
		ltranid = psmShmidx_d->lTranid;
        lsafid = psmShmidx_d->lSafid;
		swVdebug(2,"S0070:共享内存流水号tranid[%ld]冲正号safid[%ld]",ltranid,lsafid);		
		/*修改为文件加锁 modiy by cjh 20150203
        fp=fopen("/tmp/tranid.ini","w");    // 打开文件
		fprintf(fp,"%ld\n%ld",ltranid,lsafid);
        fclose(fp);*/
        sprintf(str,"%ld|%ld",ltranid,lsafid);
        fd=open("/tmp/tranid.ini",O_WRONLY|O_CREAT,0666);
        if(fd<0)
        {
            swVdebug(1,"S0071:文件打开失败");
            exit(FAIL);
        }
        lock_set(fd,F_WRLCK);
        ftruncate(fd,0); /*add by cjh20150427 PSBC_V1.0*/
        write(fd,str,strlen(str));
        lock_set(fd,F_UNLCK);
        close(fd);
		sleep(time);
        swVdebug(2,"S0080:————————————————————————");
	}
	
	exit(SUCCESS);
}

int lock_set(int fd,int type)
{
  struct flock lock;
  lock.l_type=type;
  lock.l_start=0;
  lock.l_whence=SEEK_SET;
  lock.l_len=0;
  lock.l_pid=-1;

  fcntl(fd,F_GETLK,&lock);
  if(lock.l_type!=F_UNLCK)
  {
   
    if(lock.l_type==F_RDLCK)          /*该文件已有读取锁*/
    {
      swVdebug(2,"Read lock already set by %d\n",lock.l_pid);
    }
    else if(lock.l_type==F_WRLCK)    /*该文件已有写入锁*/
    {
      swVdebug(2,"Write lock already set by %d\n",lock.l_pid);
    }
  }
 
  lock.l_type=type;
 
  if((fcntl(fd,F_SETLKW,&lock))<0)
  {
    swVdebug(1,"Lock failed:type=%d\n",lock.l_type);
    return -1;
  }
  switch(lock.l_type)
  {
    case F_RDLCK:
    {
      swVdebug(4,"Read lock set by %d\n",getpid());
    }
    break;
    case F_WRLCK:
    {
      swVdebug(4,"Write lock set by %d\n",getpid());
    }
    break;
    case F_UNLCK:
    {
      swVdebug(4,"Release lock by %d\n",getpid());
    }
    break;
   default:
   break;
  }
  return 0;
}

