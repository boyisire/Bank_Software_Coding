/****************************************************************/
/* ģ����    ��swTidTimCache                                      */
/* ģ������    ����ˮ�Ŷ�ʱ����ģ��                             */
/* �� �� ��    ��V1.0.0                                         */
/* ��	 ��    ���¼̻�                                         */
/* ��������    ��2014/12/20                                     */
/* ����޸����ڣ�2014/12/20                                     */
/* ģ����;    �����ڶ�ʱ������ˮ�ŵ��ļ�                       */
/* ��ģ���а������º���������˵����                             */
/*			 ��1��int main();                                   */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼:                                                    */
/*  2014.12.20 ����                                             */
/****************************************************************/
#include "switch.h"
#include "swShmGroup.h"
#include "swNdbstruct.h"
#include "swShm.h"

extern struct shmidx_d *psmShmidx_d;   /* ��̬�����ڴ�Ŀ¼��ָ�� */
int lock_set(int fd,int type);

int main(int argc, char **argv)
{
	int ilShmid;
	long ltranid,lsafid;
	char *alShmtmp_d;
	FILE *fp;
    int time; /*���ʱ��*/
	char aFilename[100];
    int fd;
    char str[50];
	
	/* ���õ��Գ������� */
	swLogInit("swTidTimCache", NULL); /*add by dyw, 2015.03.27, PSBC_V1.0*/
    strcpy(agDebugfile, "swTidTimCache.log");
	
	/* ȡ��DEBUG��־ */
	if ((cgDebug = _swDebugflag("swTidTimCache")) == FAIL)
	{
		printf("�޷�ȡ��DEBUG��־!\n");
		exit(FAIL);
	}
	swVdebug(2,"S0000: ȡDebug��־ok!");
	
	memset(aFilename,0x00,sizeof(aFilename));
	sprintf( aFilename,"%s/config/SWBUFSPE.CFG",getenv("SWITCH_CFGDIR") );
    if((fp = fopen(aFilename,"r"))==NULL)
    {
        printf("�������ļ�ʧ��\n");
        exit(FAIL);
    }
    fscanf(fp,"%d",&time);
	swVdebug(2,"S0010:���ʱ��time[%d]",time);
    fclose(fp); 

	  /* ��ʼ�������ڴ�ָ�� */
  if(swShmcheck() != SUCCESS)
  {
    swVdebug(0,"S0080: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    exit(FAIL);
  }
  swVdebug(2,"S0090: ��ʼ�������ڴ�ָ��ɹ�");
	
	while(1)
	{		

	  /* ˢ�¹����ڴ�ָ�� */
        if(swShmcheck() != SUCCESS)
        {
            swVdebug(0,"S0080: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
            exit(FAIL);
        }
		ltranid = psmShmidx_d->lTranid;
        lsafid = psmShmidx_d->lSafid;
		swVdebug(2,"S0070:�����ڴ���ˮ��tranid[%ld]������safid[%ld]",ltranid,lsafid);		
		/*�޸�Ϊ�ļ����� modiy by cjh 20150203
        fp=fopen("/tmp/tranid.ini","w");    // ���ļ�
		fprintf(fp,"%ld\n%ld",ltranid,lsafid);
        fclose(fp);*/
        sprintf(str,"%ld|%ld",ltranid,lsafid);
        fd=open("/tmp/tranid.ini",O_WRONLY|O_CREAT,0666);
        if(fd<0)
        {
            swVdebug(1,"S0071:�ļ���ʧ��");
            exit(FAIL);
        }
        lock_set(fd,F_WRLCK);
        ftruncate(fd,0); /*add by cjh20150427 PSBC_V1.0*/
        write(fd,str,strlen(str));
        lock_set(fd,F_UNLCK);
        close(fd);
		sleep(time);
        swVdebug(2,"S0080:������������������������������������������������");
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
   
    if(lock.l_type==F_RDLCK)          /*���ļ����ж�ȡ��*/
    {
      swVdebug(2,"Read lock already set by %d\n",lock.l_pid);
    }
    else if(lock.l_type==F_WRLCK)    /*���ļ�����д����*/
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

