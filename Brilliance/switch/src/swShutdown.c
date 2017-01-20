/****************************************************************/
/* ģ����    ��SHUTDOWN                                       */
/* ģ������    �������ڴ���ֹ��ģ��                             */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ����־��                                         */
/* ��������    ��2001/6/7                                       */
/* ����޸����ڣ�2001/6/7                                       */
/* ģ����;    ��ɾ�������ڴ���źŵ�                           */
/* ��ģ���а������º���������˵����                             */
/*			 ��1��int main();                       */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼:                                                    */
/*  2001.6.7 ����                                               */
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

    /* ���õ��Գ������� */
    strcpy(agDebugfile,"swShutdown.debug");

    /* ��ȡDEBUG��־ */
    if((cgDebug = _swDebugflag("swShutdown"))==FAIL)
    {
        fprintf(stderr,"��ȡDEBUG��־����!\n");
        exit(FAIL);
    }
    swLogInit("swShutdown" , NULL);
    swVdebug(2,"S0010: �汾��[4.3.0]");
    swVdebug(2,"S0020: DEBUG�ļ�[%s]",agDebugfile);
    swVdebug(2,"S0030: DEBUG��־[%d]",cgDebug);

    /* ��ȡ������ */
    if (swShmcfgload(&sgSwt_sys_config))
    {
        swVdebug(0,"S0040: [����/����] ��ȡ������[sgSwt_sys_config]����!");
        exit(FAIL);
    }


    /****add by wanghao 20150409 for clear static shm PSBC_V1.0******/

    ilRc = clearShm();
    if(ilRc)
    {
        swVdebug(0,"����̬�����ڴ�ʧ�ܣ�\n");
    }

    /*****************end add*******************************/

    /* ���ᶯ̬�����ڴ� */
    ilShmid = shmget((key_t)sgSwt_sys_config.iShmkey,0,IPC_EXCL|0666);
    if (ilShmid == -1)
    {
        swVdebug(0,"S0050: [����/ϵͳ����] shmget()����,errno=%d",errno);
        printf("�³�����ƽ̨��δ��ʼ��!\n");
        exit(FAIL);
    }
    alShmtmp_d = shmat(ilShmid,(char *)0,SHM_RND);  
    if ((alShmtmp_d == (char *)-1))
    {
        swVdebug(0,"S0060: [����/ϵͳ����] shmat()����,errno=%d",errno);
        printf("�³�����ƽ̨ж��ʧ��!\n");
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

    printf("�������������ڴ�...\n");
    signal(SIGALRM,swTimeout);
    alarm(5);
    _swShmcheck();
    for(k=0; k <SHM_TRANLOG_BUCKET_NUM; k++ )
    {
        if (swShmP(GET_TRANLOG_BUCKET_SEM_NUM(k)))
            swVdebug(0,"S0070: [����/�����ڴ�] swShmP()����,������=-1");
    }
    if (swShmP(iSEMNUM_PROCLOG))
        swVdebug(0,"S0080: [����/�����ڴ�] swShmP()����,������=-1");
    if (swShmP(iSEMNUM_REVSAF))
        swVdebug(0,"S0090: [����/�����ڴ�] swShmP()����,������=-1");
    alarm(0);

    /* ���潻����ˮ */
    sprintf(alFile,"%s/log/shm/tranlog.shm",getenv("SWITCH_DIR"));
    printf("���潻����ˮ����ʱ�ļ�[%s]...\n",alFile);
    if((fp=fopen(alFile,"w"))==NULL)
    {
        printf("���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
                printf("д�ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
                printf("д�ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
    printf("����[%d]��������ˮ������\n",ilCnt);

LSAVEPROCLOG:

    /* ���洦����ˮ */
    sprintf(alFile,"%s/log/shm/proclog.shm",getenv("SWITCH_DIR"));
    printf("���洦����ˮ����ʱ�ļ�[%s]...\n",alFile);
    if((fp=fopen(alFile,"w"))==NULL)
    {
        printf("���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
            printf("д�ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
                printf("д�ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
    printf("����[%d]��������ˮ������\n",ilCnt);

LSAVESAFLOG:

    /* ���������ˮ */
    sprintf(alFile,"%s/log/shm/saflog.shm",getenv("SWITCH_DIR"));
    printf("���������ˮ����ʱ�ļ�[%s]...\n",alFile);
    if((fp=fopen(alFile,"w"))==NULL)
    {
        printf("���ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
            printf("д�ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
                printf("д�ļ�[%s]����! errno=%d, %s\n",alFile,errno,strerror(errno));
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
    printf("����[%d]��������ˮ������\n",ilCnt);

    for(k=0; k <SHM_TRANLOG_BUCKET_NUM; k++ ) swShmV(GET_TRANLOG_BUCKET_SEM_NUM(k));

    swShmV(iSEMNUM_PROCLOG);
    swShmV(iSEMNUM_REVSAF);

    /* ɾ���źŵ� */
    printf("ɾ���źŵ�[id=%d]...\n",psgShmidx_d->iSemid);
    if (semctl(psgShmidx_d->iSemid,0,IPC_RMID,0) == -1)
        swVdebug(0,"S0100: [����/ϵͳ����] semctl()����,errno=%d",errno);
    else
        swVdebug(2,"S0110: ɾ���źŵƳɹ�");

    /* ɾ����̬�����ڴ� */
    printf("ɾ����̬�����ڴ�[id=%d]...\n",psgShmidx_d->iShmid);
    if (shmctl(psgShmidx_d->iShmid,IPC_RMID,0) == -1)
        swVdebug(0,"S0120: [����/ϵͳ����] shmctl()����,errno=%d",errno);
    else
        swVdebug(2,"S0130: ɾ����̬�����ڴ�ɹ�");

    /* ɾ����̬�����ڴ� */
    printf("ɾ����̬�����ڴ�[id=%d]...\n",ilShmid);
    if (shmctl(ilShmid,IPC_RMID,0) == -1)
        swVdebug(0,"S0140: [����/ϵͳ����] shmctl()����,errno=%d",errno);
    else
        swVdebug(2,"S0150: ɾ����̬�����ڴ�ɹ�");


    /**************add by wanghao 20150409 PSBC_V1.0******/

    envPath = getenv("SWITCH_DIR");
    if(envPath == NULL) 
    {
        printf("getenv����\n");
        return -1;
    }

    memset(fileName, 0x00, sizeof(fileName));
    sprintf(fileName,"%s/tmp/shmid.txt",envPath);

    if(remove(fileName))
    {
        printf("ɾ���ļ�%sʧ�ܣ�\n",fileName);
    }

    /**********************end add ************************/

    printf("�³�����ƽ̨ж�سɹ�!\n");
    exit(0);
}

int swGetchoose()
{
    char ch;

    printf("�Ƿ����?[y/n], ע��: �������������ɽ������ݶ�ʧ!!! ");
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

