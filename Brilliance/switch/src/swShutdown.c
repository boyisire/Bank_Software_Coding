/****************************************************************/
/* 模块编号    ：SHUTDOWN                                       */
/* 模块名称    ：共享内存终止化模块                             */
/* 版 本 号    ：V4.3.0                                         */
/* 作	 者    ：范志杰                                         */
/* 建立日期    ：2001/6/7                                       */
/* 最后修改日期：2001/6/7                                       */
/* 模块用途    ：删除共享内存和信号灯                           */
/* 本模块中包含如下函数及功能说明：                             */
/*			 （1）int main();                       */
/****************************************************************/

/****************************************************************/
/* 修改记录:                                                    */
/*  2001.6.7 创建                                               */
/****************************************************************/

#include "switch.h"
#include "swNdbstruct.h"
#include "swShm.h"
#include "sw_shmapi_tranlog.h"

int ilIsTimeout = 0;
void swTimeout(int sig) {ilIsTimeout = 1;}
int swGetchoose();
extern int _swShmcheck();

/***add by wanghao 20150409 PSBC_V1.0****/
extern int clearShm();

int main()
{
    int i,ilRc,ilShmid,ilCnt, k;
    short ilSize;
    char *alShmtmp_d;
    char alShmfile[100],alBuf[256],alFile[100];
    FILE *fp,*fp1;

     char *envPath;
    char fileName[100];

    signal(SIGTERM,SIG_IGN);

    /* 设置调试程序名称 */
    strcpy(agDebugfile,"swShutdown.debug");

    /* 读取DEBUG标志 */
    if((cgDebug = _swDebugflag("swShutdown"))==FAIL)
    {
        fprintf(stderr,"读取DEBUG标志出错!\n");
        exit(FAIL);
    }
    swLogInit("swShutdown" , NULL);
    swVdebug(2,"S0010: 版本号[4.3.0]");
    swVdebug(2,"S0020: DEBUG文件[%s]",agDebugfile);
    swVdebug(2,"S0030: DEBUG标志[%d]",cgDebug);

    /* 读取参数表 */
    if (swShmcfgload(&sgSwt_sys_config))
    {
        swVdebug(0,"S0040: [错误/其它] 读取参数表[sgSwt_sys_config]错误!");
        exit(FAIL);
    }


    /****add by wanghao 20150409 for clear static shm PSBC_V1.0******/

    ilRc = clearShm();
    if(ilRc)
    {
        swVdebug(0,"清理静态共享内存失败！\n");
    }

    /*****************end add*******************************/

    /* 联结动态表共享内存 */
    ilShmid = shmget((key_t)sgSwt_sys_config.iShmkey,0,IPC_EXCL|0666);
    if (ilShmid == -1)
    {
        swVdebug(0,"S0050: [错误/系统调用] shmget()函数,errno=%d",errno);
        printf("新晨交换平台还未初始化!\n");
        exit(FAIL);
    }
    alShmtmp_d = shmat(ilShmid,(char *)0,SHM_RND);  
    if ((alShmtmp_d == (char *)-1))
    {
        swVdebug(0,"S0060: [错误/系统调用] shmat()函数,errno=%d",errno);
        printf("新晨交换平台卸载失败!\n");
        exit(FAIL);
    }
    pg_lock_array = (sw_slock_array_t*)(alShmtmp_d);
    alShmtmp_d += sizeof(sw_slock_array_t);
    psgShmidx_d = (struct shmidx_d *)alShmtmp_d;
    psgShmidx_s = NULL;
    for(i=0; i< SHM_TRANLOG_BUCKET_NUM ; i++)
    {
        sgShmbuf_d.psShm_tran_log[i] = (struct shm_tran_log *)
            (alShmtmp_d + psgShmidx_d->sIdx_tran_log[i].lOffset);
    }
    sgShmbuf_d.psShm_proc_log = (struct shm_proc_log *)
        (alShmtmp_d + psgShmidx_d->sIdx_proc_log.lOffset);
    sgShmbuf_d.psShm_rev_saf = (struct shm_rev_saf *)
        (alShmtmp_d + psgShmidx_d->sIdx_rev_saf.lOffset);

    printf("正在锁定共享内存...\n");
    signal(SIGALRM,swTimeout);
    alarm(5);
    _swShmcheck();
    for(k=0; k <SHM_TRANLOG_BUCKET_NUM; k++ )
    {
        if (swShmP(GET_TRANLOG_BUCKET_SEM_NUM(k)))
            swVdebug(0,"S0070: [警告/共享内存] swShmP()函数,返回码=-1");
    }
    if (swShmP(iSEMNUM_PROCLOG))
        swVdebug(0,"S0080: [警告/共享内存] swShmP()函数,返回码=-1");
    if (swShmP(iSEMNUM_REVSAF))
        swVdebug(0,"S0090: [警告/共享内存] swShmP()函数,返回码=-1");
    alarm(0);

    /* 保存交易流水 */
    sprintf(alFile,"%s/log/shm/tranlog.shm",getenv("SWITCH_DIR"));
    printf("保存交易流水到临时文件[%s]...\n",alFile);
    if((fp=fopen(alFile,"w"))==NULL)
    {
        printf("打开文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
        if(swGetchoose())
        {
            fclose(fp);
            exit(FAIL);
        }  
    }
    ilCnt = 0;
    ilSize = sizeof(struct swt_tran_log);
    for(k=0; k< SHM_TRANLOG_BUCKET_NUM; k++)
    {
        i = psgShmidx_d->sIdx_tran_log[k].iSlink;
        while (i)
        {
            if (!sgShmbuf_d.psShm_tran_log[k][i-1].sLinkinfo.cTag)
            {
                i = sgShmbuf_d.psShm_tran_log[k][i-1].sLinkinfo.iRlink;
                continue;
            }
            ilRc = fwrite((char *)&sgShmbuf_d.psShm_tran_log[k][i-1].sSwt_tran_log,1,ilSize,fp);
            if(ilRc != ilSize)
            {
                printf("写文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
                if(swGetchoose())
                {
                    fclose(fp);
                    exit(FAIL);
                }
                else
                {
                    fclose(fp);
                    goto LSAVEPROCLOG;
                }
            }
            ilCnt ++;   
            i = sgShmbuf_d.psShm_tran_log[k][i-1].sLinkinfo.iRlink;
        }
    }

    sprintf(alShmfile,"%s/log/shm/tranlog",getenv("SWITCH_DIR"));
    if ((fp1 = fopen(alShmfile,"r")) != NULL)
    {
        while (!feof(fp1))
        {
            ilRc = fread(alBuf,sizeof(char),ilSize+1,fp1);
            if (!alBuf[0]) continue;
            ilRc = fwrite((char *)(alBuf+1),1,ilSize,fp);
            if(ilRc != ilSize)
            {
                printf("写文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
                if(swGetchoose())
                {
                    fclose(fp);
                    fclose(fp1);
                    exit(FAIL);
                }
                else
                {
                    fclose(fp);
                    fclose(fp1);
                    goto LSAVEPROCLOG;
                }
            }
            ilCnt ++;    
        }
        fclose(fp1);
        unlink(alShmfile);
    }
    fclose(fp);
    if(ilCnt == 0) unlink(alFile);
    printf("共有[%d]条交易流水被保存\n",ilCnt);

LSAVEPROCLOG:

    /* 保存处理流水 */
    sprintf(alFile,"%s/log/shm/proclog.shm",getenv("SWITCH_DIR"));
    printf("保存处理流水到临时文件[%s]...\n",alFile);
    if((fp=fopen(alFile,"w"))==NULL)
    {
        printf("打开文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
        if(swGetchoose())
        {
            fclose(fp);
            exit(FAIL);
        }  
    }
    ilCnt = 0;
    ilSize = sizeof(struct swt_proc_log);
    i = psgShmidx_d->sIdx_proc_log.iSlink;
    while (i)
    {
        if (!sgShmbuf_d.psShm_proc_log[i-1].sLinkinfo.cTag)
        {
            i = sgShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
            continue;
        }
        ilRc = fwrite((char *)&sgShmbuf_d.psShm_proc_log[i-1].sSwt_proc_log,1,ilSize,fp);
        if(ilRc != ilSize)
        {
            printf("写文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
            if(swGetchoose())
            {
                fclose(fp);
                exit(FAIL);
            }
            else
            {
                fclose(fp);
                goto LSAVESAFLOG;
            }
        }
        ilCnt ++;    
        i = sgShmbuf_d.psShm_proc_log[i-1].sLinkinfo.iRlink;
    }

    sprintf(alShmfile,"%s/log/shm/proclog",getenv("SWITCH_DIR"));
    if ((fp1 = fopen(alShmfile,"r")) != NULL)
    {
        while (!feof(fp1))
        {
            ilRc = fread(alBuf,sizeof(char),ilSize+1,fp1);
            if (!alBuf[0]) continue;
            ilRc = fwrite((char *)(alBuf+1),1,ilSize,fp);
            if(ilRc != ilSize)
            {
                printf("写文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
                if(swGetchoose())
                {
                    fclose(fp);
                    fclose(fp1);
                    exit(FAIL);
                }
                else
                {
                    fclose(fp);
                    fclose(fp1);
                    goto LSAVESAFLOG;
                }
            }
            ilCnt ++;    
        }
        fclose(fp1);
        unlink(alShmfile);
    }
    fclose(fp);
    if(ilCnt == 0) unlink(alFile);  
    printf("共有[%d]条处理流水被保存\n",ilCnt);

LSAVESAFLOG:

    /* 保存冲正流水 */
    sprintf(alFile,"%s/log/shm/saflog.shm",getenv("SWITCH_DIR"));
    printf("保存冲正流水到临时文件[%s]...\n",alFile);
    if((fp=fopen(alFile,"w"))==NULL)
    {
        printf("打开文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
        if(swGetchoose())
        {
            fclose(fp);
            exit(FAIL);
        }  
    }
    ilCnt = 0;
    ilSize = sizeof(struct swt_rev_saf);
    i = psgShmidx_d->sIdx_rev_saf.iSlink;
    while (i)
    {
        if (!sgShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.cTag)
        {
            i = sgShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
            continue;
        }
        ilRc = fwrite((char *)&sgShmbuf_d.psShm_rev_saf[i-1].sSwt_rev_saf,1,ilSize,fp);
        if(ilRc != ilSize)
        {
            printf("写文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
            if(swGetchoose())
            {
                fclose(fp);
                exit(FAIL);
            }
            else
            {
                fclose(fp);
                goto LSAVESAFLOG;
            }
        }
        ilCnt ++;   
        i = sgShmbuf_d.psShm_rev_saf[i-1].sLinkinfo.iRlink;
    }

    sprintf(alShmfile,"%s/log/shm/revsaf",getenv("SWITCH_DIR"));
    if ((fp1 = fopen(alShmfile,"r")) != NULL)
    {
        while (!feof(fp1))
        {
            ilRc = fread(alBuf,sizeof(char),ilSize,fp1);
            if (!alBuf[0]) continue;
            ilRc = fwrite((char *)(alBuf+1),1,ilSize,fp);
            if(ilRc != ilSize)
            {
                printf("写文件[%s]出错! errno=%d, %s\n",alFile,errno,strerror(errno));
                if(swGetchoose())
                {
                    fclose(fp);
                    fclose(fp1);
                    exit(FAIL);
                }
                else
                {
                    fclose(fp);
                    fclose(fp1);
                    goto LSAVEPROCLOG;
                }
            }
            ilCnt ++;    
        }
        fclose(fp1);
        unlink(alShmfile);
    }
    fclose(fp);
    if(ilCnt == 0) unlink(alFile);
    printf("共有[%d]条冲正流水被保存\n",ilCnt);

    for(k=0; k <SHM_TRANLOG_BUCKET_NUM; k++ ) swShmV(GET_TRANLOG_BUCKET_SEM_NUM(k));

    swShmV(iSEMNUM_PROCLOG);
    swShmV(iSEMNUM_REVSAF);

    /* 删除信号灯 */
    printf("删除信号灯[id=%d]...\n",psgShmidx_d->iSemid);
    if (semctl(psgShmidx_d->iSemid,0,IPC_RMID,0) == -1)
        swVdebug(0,"S0100: [警告/系统调用] semctl()函数,errno=%d",errno);
    else
        swVdebug(2,"S0110: 删除信号灯成功");

    /* 删除静态表共享内存 */
    printf("删除静态表共享内存[id=%d]...\n",psgShmidx_d->iShmid);
    if (shmctl(psgShmidx_d->iShmid,IPC_RMID,0) == -1)
        swVdebug(0,"S0120: [警告/系统调用] shmctl()函数,errno=%d",errno);
    else
        swVdebug(2,"S0130: 删除静态表共享内存成功");

    /* 删除动态表共享内存 */
    printf("删除动态表共享内存[id=%d]...\n",ilShmid);
    if (shmctl(ilShmid,IPC_RMID,0) == -1)
        swVdebug(0,"S0140: [警告/系统调用] shmctl()函数,errno=%d",errno);
    else
        swVdebug(2,"S0150: 删除动态表共享内存成功");


    /**************add by wanghao 20150409 PSBC_V1.0******/

    envPath = getenv("SWITCH_DIR");
    if(envPath == NULL) 
    {
        printf("getenv错误！\n");
        return -1;
    }

    memset(fileName, 0x00, sizeof(fileName));
    sprintf(fileName,"%s/tmp/shmid.txt",envPath);

    if(remove(fileName))
    {
        printf("删除文件%s失败！\n",fileName);
    }

    /**********************end add ************************/

    printf("新晨交换平台卸载成功!\n");
    exit(0);
}

int swGetchoose()
{
    char ch;

    printf("是否继续?[y/n], 注意: 如果继续可能造成交易数据丢失!!! ");
    while(1)
    {
        ch = getchar();
        switch(ch)
        {
            case 'Y':
            case 'y':
                return 0;
            case 'N':
            case 'n':
                return 1;
            default:
                break;
        }
    }
}        

