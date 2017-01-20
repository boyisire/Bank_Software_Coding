#include "switch.h"
#include "swDbstruct.h"
#include "swShm.h"


#define BUFLEN 8192

void  _shmGetApps();
void _shmGetImf();
void _shmGetMatchPort();
void _shmGetIso8583();
void _shmGetTask();
void _shmGetCode();
void _shmGetConfig();
void _shmGetHosts();
void _shmGetFormat();
void _shmGetRouter();
void _shmGetControl();
extern struct shmidx_s *psmShmidx_s; 
extern struct shmbuf_s smShmbuf_s;




/*********************************************
 ** 函数功能    ：查询静态共享内存中配置文件的数据
 ** 参数        ：
 ** 返回值      ：
 ** 作者        ：王浩
 ** 创建时间    ：20150313
 ** 最后修改时间：
 *********************************************/
int main(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("\t使用方法：swShmcheckTool [NAME]\n");
        printf("\tNAME    : imf ...........查询内存中SWIMF.CFG的内容\n"); 
        printf("\t          apps...........查询内存中SWAPPS.CFG的内容\n");
        printf("\t          match_port.....查询内存中SWMATCH_PORT.CFG的内容\n");
        printf("\t          task...........查询内存中SWTASK.CFG的内容\n");
        printf("\t          iso8583........查询内存中SWISO8583.CFG的内容\n");
        printf("\t          code...........查询内存中SWCODE.CFG的内容\n");
        printf("\t          config.........查询内存中SWCONFIG.CFG的内容\n");
        printf("\t          hosts..........查询内存中SWOTHER.CFG的内容\n");
        printf("\t          format.........查询内存中SWFORMAT.CFG的内容\n");
        printf("\t          router.........查询内存中SWROUTER.CFG的内容\n");
        printf("\t          control........查询内存中SWTRAN_CTL.CFG的内容\n");

        exit(-1);
    }

    if( strcmp(argv[1],"imf") == 0 )
    {
        _shmGetImf();
        return 0;
    }
    else if( strcmp(argv[1], "apps") == 0 )
    {
        _shmGetApps();
        return 0;
    } 
    else if( strcmp(argv[1], "match_port") == 0 )
    {
        _shmGetMatchPort();
        return 0;
    }
    else if( strcmp(argv[1], "task") == 0 )
    {
        _shmGetTask();
        return 0;
    }
    else if( strcmp(argv[1], "iso8583") == 0)
    {
        _shmGetIso8583();
        return 0;
    }
    else if( strcmp(argv[1], "code") == 0 )
    {
        _shmGetCode();
        return 0;
    }
    else if( strcmp(argv[1], "config") == 0 )
    {
        _shmGetConfig();
        return 0;
    }
    else if( strcmp(argv[1], "hosts") == 0 )
    {
        _shmGetHosts();
        return 0;
    }
    else if( strcmp(argv[1], "format") == 0 )
    {
        _shmGetFormat();
        return 0;
    }
    else if( strcmp(argv[1], "router") == 0 )
    {
        _shmGetRouter();
        return 0;
    }
    else if( strcmp(argv[1],"control") == 0 )
    {
        _shmGetControl();
        return 0;
    }
    else
    {
        printf("在静态共享内存中没有[%s]配置文件相关信息！\n",argv[1]);
        exit(-1);
    }

}


/**********************************************
**函数功能    ：读取静态共享内存中SWAPPS.CFG的数据
**参数        ：无
**返回值      ：无
**作者        ：王浩
**创建时间    ：20150316
**最后修改时间：20150319
***********************************************/
void _shmGetApps()
{
    int i;
    char buf[BUFLEN];
    short q_id;
    short q_port_id;
    char  q_name[21];
    short msg_format;
    short rev_overtime;
    short rev_num;
    char  tc_unpack[iEXPRESSLEN];
    char  tc_pack[iEXPRESSLEN];
    char  tran_type[129];
    char  def_rs[7];
    char  def_gs[7];
    char  def_fs[7];
    char  rev_nogood[2];
    char  rev_nomrev[2];
    short isotab_id;
    char  bitmap_type[2];
    char  msg_prio[2];
    char  key_express[iEXPRESSLEN];
    char  comm_type[2];
    char  comm_attr[iEXPRESSLEN];
    char  resu_express[iEXPRESSLEN];
    char  code_type[2];   
    char  resu_revexp[iEXPRESSLEN];
    short max_tran;      
    short reje_grp;       
    long  trigger_freq;          
    char  trigger_term[iEXPRESSLEN];              
    char  setdown_overtime[iEXPRESSLEN];  
    long  tranbegin_max;    
    long  traning_max;   
    char rev_express[iEXPRESSLEN];
    short e8583tab_id;            
    short bitmap_len;         
    char  bitmap_unpk[iEXPRESSLEN];      
    char  bitmap_pack[iEXPRESSLEN];      
    char  mac_express[iEXPRESSLEN];   
    char port_status;

    char *envPath;
    char fileName[256];
    FILE *fp;
    struct swt_sys_queue  *pslQueueid;
    int ilNum;
    char dir[200];
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));
    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
        strcpy(dir,"/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
    }

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWAPPS.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWAPPS.CFG失败！\n");
        exit(-1);
    }

    pslQueueid = (smShmbuf_s.psSwt_sys_queue);
    ilNum = psmShmidx_s->sIdx_sys_queue.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWAPPS.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        for(i = 0; i < ilNum; i++)
        {
            memset(buf, 0x00, sizeof(buf));

            memset(q_name,0x00,21);
            memset(msg_prio,0x00,2);
            memset(comm_type,0x00,2);
            memset(comm_attr,0x0,iEXPRESSLEN);
            memset(rev_nogood,0x00,2);
            memset(rev_nomrev,0x00,2);
            memset(resu_express,0x00,iEXPRESSLEN);
            memset(resu_revexp,0x00,iEXPRESSLEN);
            memset(tc_unpack,0x00,iEXPRESSLEN);
            memset(tc_pack,0x00,iEXPRESSLEN);
            memset(key_express,0x00,iEXPRESSLEN);
            memset(def_rs,0x00,7);
            memset(def_gs,0x00,7);
            memset(def_fs,0x00,7);
            memset(bitmap_type,0x00,2);
            memset(code_type,0x00,2);
            memset(tran_type,0x00,129);
            memset(trigger_term,0x00,iEXPRESSLEN);
            memset(setdown_overtime,0x00,iEXPRESSLEN);
            memset(bitmap_unpk,0x00,iEXPRESSLEN);
            memset(bitmap_pack,0x00,iEXPRESSLEN);
            memset(mac_express,0x00,iEXPRESSLEN);
            memset(rev_express,0x00,iEXPRESSLEN);
        
            q_id  = (pslQueueid+i)->q_id;
            q_port_id  = pslQueueid[i].q_port_id;
            memcpy(q_name,pslQueueid[i].q_name,21);
            msg_format =pslQueueid[i].msg_format;
            memcpy(msg_prio,pslQueueid[i].msg_prio,2);
            memcpy(comm_type,pslQueueid[i].comm_type,2);
            memcpy(comm_attr,pslQueueid[i].comm_attr,iEXPRESSLEN);
            rev_overtime = pslQueueid[i].rev_overtime;
            rev_num = pslQueueid[i].rev_num;
            memcpy(rev_nogood,pslQueueid[i].rev_nogood,2);
            memcpy(rev_nomrev,pslQueueid[i].rev_nomrev,2);
            memcpy(resu_express,pslQueueid[i].resu_express,iEXPRESSLEN);
            memcpy(resu_revexp,pslQueueid[i].resu_revexp,iEXPRESSLEN);
            memcpy(tc_unpack,pslQueueid[i].tc_unpack,iEXPRESSLEN);
            memcpy(tc_pack,pslQueueid[i].tc_pack,iEXPRESSLEN);
            memcpy(key_express,pslQueueid[i].key_express,iEXPRESSLEN);
            memcpy(def_rs,pslQueueid[i].def_rs,7);
            memcpy(def_gs,pslQueueid[i].def_gs,7);
            memcpy(def_fs,pslQueueid[i].def_fs,7);
            max_tran = pslQueueid[i].max_tran;
            reje_grp = pslQueueid[i].reje_grp;
            isotab_id = pslQueueid[i].isotab_id;
            memcpy(bitmap_type,pslQueueid[i].bitmap_type,2);
            memcpy(code_type, pslQueueid[i].code_type,2);
            memcpy(tran_type, pslQueueid[i].tran_type,129);
            trigger_freq = pslQueueid[i].trigger_freq;
            memcpy(trigger_term,pslQueueid[i].trigger_term,iEXPRESSLEN);
            memcpy(setdown_overtime,pslQueueid[i].setdown_overtime,iEXPRESSLEN);
            e8583tab_id = pslQueueid[i].e8583tab_id;
            bitmap_len = pslQueueid[i].bitmap_len;
            memcpy(bitmap_unpk,pslQueueid[i].bitmap_unpk,iEXPRESSLEN);
            memcpy(mac_express,pslQueueid[i].mac_express,iEXPRESSLEN);
            tranbegin_max = pslQueueid[i].tranbegin_max;
            traning_max = pslQueueid[i].traning_max;
            memcpy(rev_express,pslQueueid[i].rev_express,iEXPRESSLEN);
            
            port_status = pslQueueid[i].port_status ;

            sprintf(buf,"QID=%d\nPORTID=%d\nAPPNAME=%s\nMSGFORMAT=%d\nMSGPRIOR=%s\nCOMMTYPE=%s\nCOMATTR=%s\nOVERTIME=%d\nREVNUM=%d\nREV_NOGOOD=%s\n \
REV_NOM=%s\nAPPRESUEXP=%s\nREVRESUEXP=%s\nUNPACKEXP=%s\nPACKEXP=%s\nMSGHEADEXP=%s\n \
DEFRCD=%s\nDEFGRP=%s\nDEFFLD=%s\nMAXTRAN=%d\nREJEGRP=%d\nISOTABID=%d\nBITMAP_TYPE=%s\n \
CODETYPE=%s\nTRANTYPE=%d\nTRIGGER_FREQ=%ld\nTRIGGER_TERM=%s\nSETDOWN_OVERTIME=%s\nE8583TAB_ID=%d\nBITMAP_LEN=%d\n \
BITMAP_UNPK=%s\nBITMAP_PACK=%s\nMAC_EXPRESS=%s\nTRANBEGIN_MAX=%d\nTRANING_MAX=%d\ntranbegin_num=%ld\ntraning_num=%ld\n \
REV_EXPRESS=%s\nPORT_STATUS=%x\n \
MAX_FAIL_COUNT=%d\nFAIL_CON_TRY_INTER=%d\nfail_count=%d\nfail_begin_time=%ld\n",\
                      q_id,q_port_id,q_name,msg_format,msg_prio,comm_type,comm_attr,rev_overtime, rev_num,rev_nogood,rev_nomrev, resu_express,resu_revexp, \
                      tc_unpack, tc_pack,key_express ,def_rs, def_gs, def_fs,max_tran,reje_grp, isotab_id,bitmap_type, \
                      code_type,tran_type,trigger_freq,trigger_term,setdown_overtime,e8583tab_id,bitmap_len, \
                      bitmap_unpk, bitmap_pack,mac_express,tranbegin_max,traning_max,pslQueueid[i].tranbegin_num,pslQueueid[i].traning_num, \
                      rev_express , port_status, \
                      pslQueueid[i].max_fail_count, pslQueueid[i].fail_connect_try_interval, pslQueueid[i].fail_count, pslQueueid[i].fail_begin_time);
            //printf("q_id=%d, portid= %d &result= %d\n", q_id, q_port_id,( port_status&0xF8 )!= 0xF8 ) ;
            buf[strlen(buf)] = '\0';


            fwrite(buf, strlen(buf),1, fp);
        }

        fclose(fp);
         printf("检查SWAPPS.CFG成功，请查看[%s]！\n",fileName);
    }
}


/**********************************************
**函数功能    ：读取静态共享内存中SWIMF.CFG的数据
**参数        ：无
**返回值      ：无
**作者        ：王浩
**创建时间    ：20150316
**最后修改时间：20150319
***********************************************/
void _shmGetImf()
{
    char buf[BUFLEN];
    int i;
    struct swt_sys_imf  *pslImfid;
    int ilNum;
    FILE *fp;
    char *envPath;
    char fileName[256];
    char dir[200];

    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));

    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
        //printf("未设置check的环境变量！\n");
        strcpy(dir,"/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }

    }
    else
    {
        strcpy(dir, envPath);
    }

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWIMF.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWIMF.CFG失败！\n");
        exit(-1);
    }

    pslImfid = (smShmbuf_s.psSwt_sys_imf);
    ilNum = psmShmidx_s->sIdx_sys_imf.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWAIMF.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        for(i = 0; i < ilNum; i++)
        {
            memset(buf, 0x00, sizeof(buf));

            sprintf(buf, "IMF_NAME=%s\nIMF_TYPE=%s\nIMF_LEN=%d\nIMF_DEC=%d\nIMF_CHECK=%s\nIMF_ID=%d\nIMF_CHINA=%s\n",\
                     pslImfid[i].imf_name,pslImfid[i].imf_type,pslImfid[i].imf_len,pslImfid[i].imf_dec,pslImfid[i].imf_check,pslImfid[i].imf_id,pslImfid[i].imf_china);
        }

        fwrite(buf, strlen(buf),1, fp);
    }
    fclose(fp);
    printf("检查SWIMF.CFG成功，请查看[%s]！\n",fileName);
}


/**********************************************
**函数功能    ：读取静态共享内存中SWMATCH_PORT.CFG的数据
**参数        ：无
**返回值      ：无
**作者        ：王浩
**创建时间    ：20150317
**最后修改时间：20150319
***********************************************/
void _shmGetMatchPort()
{

    char buf[BUFLEN];
    int i;
    struct swt_sys_matchport  *pslMatchPortid;
    char * envPath;
    int ilNum;
    FILE *fp;
    char fileName[256];
    char dir[200];
    
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));

    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
        strcpy(dir, "/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir, envPath);
    }

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWMATCH_PORT.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWMATCH_PORT.CFG失败！\n");
        exit(-1);
    }

    pslMatchPortid = (smShmbuf_s.psSwt_sys_matchport);
    ilNum = psmShmidx_s->sIdx_sys_matchport.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWMATCH_PORT.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        for(i = 0; i < ilNum; i++)
        {
            memset(buf, 0x00, sizeof(buf));

            sprintf(buf, "%d     %s     %d\n",pslMatchPortid[i].qid, pslMatchPortid[i].trancode, pslMatchPortid[i].port);

            fwrite(buf, strlen(buf), 1, fp);
        }
    }

    fclose(fp);
    printf("检查SWMATCH_PORT.CFG成功，请查看[%s]！\n",fileName);
}



/**********************************************
**函数功能：读取静态共享内存中SWTASK.CFG的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150317
**最后修改时间：20150319
***********************************************/
void _shmGetTask()
{

    char buf[BUFLEN];
    int i;
    struct swt_sys_task  *pslTaskid;
    FILE *fp;
    char *envPath;
    int ilNum;
    char fileName[256];
    char dir[200];
    
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));

    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
        strcpy(dir, "/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir, envPath);
    }

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWTASK.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWTASK.CFG失败！\n");
        exit(-1);
    }

    pslTaskid = (smShmbuf_s.psSwt_sys_task);
    ilNum = psmShmidx_s->sIdx_sys_task.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWTASK.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        for(i = 0; i < ilNum; i++)
        {
            memset(buf, 0x00, sizeof(buf));

            sprintf(buf, "%s\nTASKFILE=%s\nQID=%d\nSTARTID=%d\nSTARTWAIT=%d\nSTOPID=%d\nSTOPWAIT=%d\nPID=%ld\nTASK_STATUS=%s\nRESTART_FLAG=%s\nKILLID=%d\nTASKATTR=%s\nSTARTTIME=%ld\nRESTART_NUM=%d\nRESTART_MAX=%d\nTASKPRIOR=%d\nTASKUSE=%s\nTASKTIMER=%s\nTASKFLAG=%d\nTASKDESC=%s\n ",
                    pslTaskid[i].task_name,pslTaskid[i].task_file, pslTaskid[i].q_id,pslTaskid[i].start_id,pslTaskid[i].start_wait,pslTaskid[i].stop_id,\
                    pslTaskid[i].stop_wait,pslTaskid[i].pid,pslTaskid[i].task_status,pslTaskid[i].restart_flag,pslTaskid[i].kill_id,pslTaskid[i].task_attr,\
                    pslTaskid[i].start_time,pslTaskid[i].restart_num,pslTaskid[i].restart_max,pslTaskid[i].task_priority,pslTaskid[i].task_use, \
                    pslTaskid[i].task_timer,pslTaskid[i].task_flag,pslTaskid[i].task_desc);

            fwrite(buf, strlen(buf), 1, fp);

        }
    }

    fclose(fp);
    printf("检查SWTASK.CFG成功，请查看[%s]！\n",fileName);
}


/**********************************************
**函数功能：读取静态共享内存中SWISO8583.CFG的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150317
**最后修改时间：20150319
***********************************************/
void _shmGetIso8583()
{

    char buf[BUFLEN];
    int i;
    struct swt_sys_8583  *psl8583id;
    int ilNum;
    char fileName[256];
    FILE *fp;
    char *envPath;
    char dir[200];
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));

    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
        strcpy(dir, "/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
    }

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWISO8583.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWISO8583.CFG失败！\n");
        exit(-1);
    }

    psl8583id = (smShmbuf_s.psSwt_sys_8583);
    ilNum = psmShmidx_s->sIdx_sys_8583.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWISO8583.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        for(i = 0; i < ilNum; i++)
        {
            memset(buf, 0x00, sizeof(buf));
            sprintf(buf, "%5d%5d     %s%5d%5d%5d     %s\n",psl8583id[i].tab_id,psl8583id[i].fld_id,psl8583id[i].fld_name, \
                     psl8583id[i].fld_type,psl8583id[i].fld_attr,psl8583id[i].fld_len,psl8583id[i].fld_rule);

            fwrite(buf, strlen(buf), 1, fp);
        }
    }

    fclose(fp);
    printf("检查SWISO8583.CFG成功，请查看[%s]！\n",fileName);

}



/**********************************************
**函数功能：读取静态共享内存中SWCODE.CFG的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150317
**最后修改时间：20150319
***********************************************/
void _shmGetCode()
{

    char buf[BUFLEN];
    int i;
    int ilNum;
    struct swt_sys_code  *pslCodeid;
    FILE *fp;
    char fileName[256];
    char *envPath;
    char dir[200];
    
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }
    memset(dir, 0x00, sizeof(dir));

    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
        strcpy(dir, "/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
    }

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWCODE.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWCODE.CFG失败！\n");
        exit(-1);
    }

    pslCodeid = (smShmbuf_s.psSwt_sys_code);
    ilNum = psmShmidx_s->sIdx_sys_code.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWCODE.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));
        
        for(i = 0; i < ilNum; i++)
        {
            sprintf(buf, "%5ld        %s%15d%15d\n",pslCodeid[i].msg_code,pslCodeid[i].msg_desc, pslCodeid[i].msg_action,pslCodeid[i].msg_q);

            fwrite(buf, strlen(buf),1,fp);
        }
    }

    fclose(fp);
    printf("检查SWCODE.CFG成功，请查看[%s]！\n",fileName);

}



/**********************************************
**函数功能：读取静态共享内存中SWCONFIG.CFG的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150317
**最后修改时间：20150319
***********************************************/
void _shmGetConfig()
{

    char buf[BUFLEN];
    int i;
    int ilNum;
    struct swt_sys_config  *pslConfigid;
    FILE *fp;
    char fileName[256];
    char *envPath;
    char dir[200];
    
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));
    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
        strcpy(dir, "/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
    }

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWCONFIG.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWCONFIG.CFG失败！\n");
        exit(-1);
    }

    pslConfigid = (smShmbuf_s.psSwt_sys_config);
    ilNum = psmShmidx_s->sIdx_sys_config.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWCONFIG.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));

        for(i = 0; i < ilNum; i++)
        {
            sprintf(buf, "LOG_INTER=%d\nSAF_INTER=%d\nSHMKEY=%d\nSHMWAITTIME=%d\nMAXTRANLOG=%d\nMAXPROCLOG=%d\nMAXSAFLOG=%d\nFTPMODE=%s\nFTPOVERTIME=%d\nQPACKSIZE=%d\nQOVERTIME=%d\nQRESENDNUM=%d\nSYSMON_IP=%s\nMSG_NUM=%ld\nSYSMON_PORT=%d\nECHOTEST_NUM=%ld\nECHOTEST_INTER=%d\nTRACE_TERM=%s\nMAXTRANNUM=%ld\nLOGCLEAR_COUNT=%d\nREREV_INTER=%d\n",\ 
                    pslConfigid[i].log_inter,pslConfigid[i].saf_inter,pslConfigid[i].iShmkey,pslConfigid[i].iShmwaittime,\
                    pslConfigid[i].iMaxtranlog,pslConfigid[i].iMaxproclog,pslConfigid[i].iMaxsaflog,pslConfigid[i].ftpmode,\
                    pslConfigid[i].ftpovertime,pslConfigid[i].qpacksize,pslConfigid[i].qovertime,pslConfigid[i].qresendnum, \
                    pslConfigid[i].sysmonaddr,pslConfigid[i].msg_num,pslConfigid[i].sysmonport,\
                    pslConfigid[i].echotest_num,pslConfigid[i].echotest_inter,pslConfigid[i].trace_term,pslConfigid[i].iMaxtrannum,\
                    pslConfigid[i].log_clear_count,pslConfigid[i].rerev_inter);

            fwrite(buf, strlen(buf), 1, fp);
        }
    }

    fclose(fp);
    printf("检查SWCONFIG.CFG成功，请查看[%s]\n",fileName);
}



/**********************************************
**函数功能：读取静态共享内存中SWHOSTS.CFG的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150317
**最后修改时间：20150319
***********************************************/
void _shmGetHosts()
{

    char buf[BUFLEN];
    int i;
    int ilNum;
    struct swt_sys_other  *pslOtherid;
    FILE *fp;
    char fileName[256];
    char *envPath;
    char dir[200];
    
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    envPath = getenv("SWITCH_CHECK");
    
    if(envPath == NULL)
    {
        strcpy(dir, "/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
    }
    

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWHOSTS.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWHOSTS.CFG失败！\n");
        exit(-1);
    }

    pslOtherid = (smShmbuf_s.psSwt_sys_other);
    ilNum = psmShmidx_s->sIdx_sys_other.iCount;


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWHOSTS.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));
        for(i = 0; i < ilNum; i++)
        {
            sprintf(buf, "%5d     %s%5d%5d%5d\n",pslOtherid[i].other_id,pslOtherid[i].other_name,pslOtherid[i].local_q,pslOtherid[i].other_grp,pslOtherid[i].other_q);
            fwrite(buf, strlen(buf), 1, fp);
        }
    }

    fclose(fp);
    printf("检查SWHOSTS.CFG成功，请查看[%s]\n",fileName);
}


/**********************************************
**函数功能：读取静态共享内存中*.FMT的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150318
**最后修改时间：20150319
***********************************************/
void _shmGetFormat()
{
    char buf[BUFLEN];
    int i;
    int ilNum;
    struct shm_sys_fmt_g *psfmtgid;
    struct swt_sys_fmt_d *psfmtdid;
    struct shm_sys_fmt_m *psfmtmid;
    FILE *fp;
    char fileName[256];
    char *envPath;
    int ilRet;
    long ilFmt;
    char tmp[20];
    char dir[200];

    ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check失败！\n");
        exit(-1);
    }

#ifndef HASH
    ilNum = psmShmidx_s->sIdx_sys_fmt_g.iCount;
#else
    ilNum = psmShmidx_s->sIdx_sys_fmt_g_hash.iCount;
#endif
    if(0 == ilNum)
    {
        printf("静态共享内存中没有swt_sys_fmt_grp类型的格式转换文件\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));
    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
       // printf("SWITCH_CHECK环境变量未设置！\n");
        strcpy(dir, "/tmp/check/format/");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
        strcat(dir,"/format/");
        ilRet = mkdir(dir,0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }

#ifndef HASH 
    psfmtgid = smShmbuf_s.psShm_sys_fmt_g;
#else
    long dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_g_hash, sizeof(long));
    psfmtgid = (struct shm_sys_fmt_g *)(smShmbuf_s.psShm_sys_fmt_g_hash+dataoffset);
#endif

    for(i = 0; i < ilNum; i++)
    {
        memset(tmp, 0x00, sizeof(tmp));
        memset(fileName, 0x00, sizeof(fileName));
        memset(buf, 0x00, sizeof(buf));

        ilFmt = psfmtgid[i].sSwt_sys_fmt_g.fmt_group;
        strcpy(fileName,dir);
       // strcat(fileName,"format/");
        sprintf(tmp,"%ld",ilFmt);
        strcat(fileName,tmp);
        strcat(fileName,".FMT");
        
        fp = fopen(fileName,"w");
        if(fp == NULL)
        {
            printf("创建文件[%s]失败！\n",fileName);
            exit(-1);
        }

         fwrite("\n[fmt_g]\n",strlen("\n[fmt_g]\n"),1,fp);

        sprintf(buf, "FMTDESC=%s\nTRANTYPE=%d\nMAC_CREATE=%s\nMAC_CHECK=%s\n",\
                psfmtgid[i].sSwt_sys_fmt_g.fmt_desc,\
                psfmtgid[i].sSwt_sys_fmt_g.tran_type,\
                psfmtgid[i].sSwt_sys_fmt_g.mac_create,\
                psfmtgid[i].sSwt_sys_fmt_g.mac_check);

        fwrite(buf, strlen(buf), 1, fp);
        
        fclose(fp);
    }

    ilNum = psmShmidx_s->sIdx_sys_fmt_d.iCount;
    if(0 == ilNum)
    {
        printf("静态共享内存中没有swt_sys_fmt_d类型的格式转换文件\n");
        exit(-1);
    }
    
    psfmtdid = smShmbuf_s.psSwt_sys_fmt_d;
    for(i = 0; i < ilNum; i++)
    {

        memset(tmp, 0x00, sizeof(tmp));
        memset(fileName, 0x00, sizeof(fileName));
        memset(buf, 0x00, sizeof(buf));

        ilFmt = psfmtdid[i].fmt_group;
        strcpy(fileName,dir);
        //strcat(fileName,"format/");
        sprintf(tmp,"%ld",ilFmt);
        strcat(fileName,tmp);
        strcat(fileName,".FMT");
        
        fp = fopen(fileName,"a");
        if(fp == NULL)
        {
            printf("打开文件[%s]失败！\n",fileName);
            exit(-1);
        }
       // if(i == 0)
        //{
            fwrite("\n[fmt_d]\n",strlen("\n[fmt_m]\n"),1,fp);
        //}

        sprintf(buf, "\nID=%d\nIMF_NAME=%s\nIMF_ARRAY=%s\nFLD_ID=%d\nFLD_EXPRESS=%s\nFMT_FLAG=%s\nFMT_COND=%s\n", \
                psfmtdid[i].id, \
                psfmtdid[i].imf_name, \
                psfmtdid[i].imf_array, \
                psfmtdid[i].fld_id, \
                psfmtdid[i].fld_express, \
                psfmtdid[i].fmt_flag, \
                psfmtdid[i].fmt_cond);
        fwrite(buf, strlen(buf), 1, fp);

        fclose(fp);
    }

#ifndef HASH
    ilNum = psmShmidx_s->sIdx_sys_fmt_m.iCount;
#else
    ilNum = psmShmidx_s->sIdx_sys_fmt_m_hash.iCount;
#endif
    if(0 == ilNum)
    {
        printf("静态共享内存中没有shm_sys_fmt_m类型的格式转换文件\n");
        exit(-1);
    }
#ifndef HASH
    psfmtmid = smShmbuf_s.psShm_sys_fmt_m;
#else
    dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_fmt_m_hash, sizeof(long));
    psfmtmid = (struct shm_sys_fmt_m *)(smShmbuf_s.psShm_sys_fmt_m_hash+dataoffset);
#endif
    for(i = 0; i < ilNum; i++)
    {

        memset(tmp, 0x00, sizeof(tmp));
        memset(fileName, 0x00, sizeof(fileName));
        memset(buf, 0x00, sizeof(buf));

        ilFmt = psfmtmid[i].sSwt_sys_fmt_m.fmt_group;
        strcpy(fileName,dir);
       // strcat(fileName,"format/");
        sprintf(tmp,"%ld",ilFmt);
        strcat(fileName,tmp);
        strcat(fileName,".FMT");
        
        fp = fopen(fileName,"a");
        if(fp == NULL)
        {
            printf("打开文件[%s]失败！\n",fileName);
            exit(-1);
        }
       // if(i == 0)
        //{
            fwrite("\n[fmt_m]\n",strlen("\n[fmt_m]\n"),1,fp);
        //}

        sprintf(buf, "\nQ_ID=%d\nTRAN_CODE=%s\nTRAN_STEP=%d\nTRAN_TYPE=%s\nMAC_CREATE=%s\nMAC_CHECK=%s\nFMT_PRIOR=%d\n",\
                psfmtmid[i].sSwt_sys_fmt_m.q_id,\
                psfmtmid[i].sSwt_sys_fmt_m.tran_code,\
                psfmtmid[i].sSwt_sys_fmt_m.tran_step,\
                psfmtmid[i].sSwt_sys_fmt_m.tran_type,\
                psfmtmid[i].sSwt_sys_fmt_m.mac_create,\
                psfmtmid[i].sSwt_sys_fmt_m.mac_check,\
                psfmtmid[i].sSwt_sys_fmt_m.fmt_prior);

        fwrite(buf, strlen(buf), 1, fp);
        fclose(fp);
    }
    printf("检查格式转换文件成功，请查看[%s]目录\n",dir);

}

/**********************************************
**函数功能：读取静态共享内存中*.SCR的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150318
**最后修改时间：20150319
***********************************************/
void _shmGetRouter()
{

    char buf[BUFLEN];
    int i;
    int ilNum;
    struct shm_sys_route_g *psRoutergid;
    struct swt_sys_route_d *psRouterdid;
    struct shm_sys_route_m *psRoutermid;
    FILE *fp;
    char fileName[256];
    char *envPath;
    int ilRet;
    long ilFmt;
    char tmp[20];
    char dir[200];

    ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check失败！\n");
        exit(-1);
    }


    ilNum = psmShmidx_s->sIdx_sys_route_g.iCount;
    if(0 == ilNum)
    {
        printf("静态共享内存中没有swt_sys_route_grp类型的路由文件\n");
        exit(-1);
    }

    memset(dir, 0x00, sizeof(dir));

    envPath = getenv("SWITCH_CHECK");
    if(envPath == NULL)
    {
       // printf("SWITCH_CHECK环境变量未设置！\n");
        strcpy(dir, "/tmp/check/router/");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录%s失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
        strcat(dir,"/router/");
        ilRet = mkdir(dir,0755);
        if(ilRet)
        {
        //printf("errno is:%d\n",errno);
       // printf("msg:%s\n",strerror(errno));
            if(errno != 17 )
            {
                printf("创建目录失败！\n");
                exit(-1);
            }
        }
    }

    
    psRoutergid = smShmbuf_s.psShm_sys_route_g;

    for(i = 0; i < ilNum; i++)
    {
        memset(tmp, 0x00, sizeof(tmp));
        memset(fileName, 0x00, sizeof(fileName));
        memset(buf, 0x00, sizeof(buf));

        ilFmt = psRoutergid[i].sSwt_sys_route_g.route_grp;
        strcpy(fileName,dir);
        //strcat(fileName,"router/");
        sprintf(tmp,"%ld",ilFmt);
        strcat(fileName,tmp);
        strcat(fileName,".SCR");
        
        fp = fopen(fileName,"w");
        if(fp == NULL)
        {
            printf("创建文件[%s]失败！\n",fileName);
            exit(-1);
        }

        fwrite("\n[route_g]\n",strlen("\n[route_g]\n"),1,fp);

        sprintf(buf, "ROUTE_DESC=%s\nTRAN_SORT=%s\n",\
                 psRoutergid[i].sSwt_sys_route_g.route_desc, psRoutergid[i].sSwt_sys_route_g.tran_sort);

        fwrite(buf, strlen(buf), 1, fp);
        
        fclose(fp);
    }


    ilNum = psmShmidx_s->sIdx_sys_route_d.iCount;
    if(0 == ilNum)
    {
        printf("静态共享内存中没有swt_sys_route_d类型的格式转换文件\n");
        exit(-1);
    }
    
    psRouterdid = smShmbuf_s.psSwt_sys_route_d;
    for(i = 0; i < ilNum; i++)
    {

        memset(tmp, 0x00, sizeof(tmp));
        memset(fileName, 0x00, sizeof(fileName));
        memset(buf, 0x00, sizeof(buf));

        ilFmt = psRouterdid[i].route_grp;
        strcpy(fileName,dir);
       // strcat(fileName,"router/");
        sprintf(tmp,"%ld",ilFmt);
        strcat(fileName,tmp);
        strcat(fileName,".SCR");
        
        fp = fopen(fileName,"a");
        if(fp == NULL)
        {
            printf("打开文件[%s]失败！\n",fileName);
            exit(-1);
        }
        
        fwrite("\n[router_d]\n",strlen("\n[router_d]\n"),1,fp);


        sprintf(buf,"\nROUTE_ID=%d\nROUTE_COND=%s\nQ_TARGET=%s\nNEXT_ID=%d\nOPER_FLAG=%s\nSAF_FLAG=%s\nEND_FLAG=%s\nFMT_GROUP=%ld\nREV_MODE=%d\nREV_FMTGRP=%ld\nFML_NAME=%s\nROUTE_MEMO=%s\n",\
                psRouterdid[i].route_id,psRouterdid[i].route_cond,psRouterdid[i].q_target,\
                psRouterdid[i].next_id,psRouterdid[i].oper_flag,psRouterdid[i].saf_flag,\
                psRouterdid[i].end_flag,psRouterdid[i].fmt_group,psRouterdid[i].rev_mode,\
                psRouterdid[i].rev_fmtgrp,psRouterdid[i].fml_name,psRouterdid[i].route_memo);
        fwrite(buf, strlen(buf), 1, fp);

        fclose(fp);
    }

#ifndef HASH
    ilNum = psmShmidx_s->sIdx_sys_route_m.iCount;
#else
    ilNum = psmShmidx_s->sIdx_sys_route_m_hash.iCount;
#endif
    if(0 == ilNum)
    {
        printf("静态共享内存中没有shm_sys_route_m类型的格式转换文件\n");
        exit(-1);
    }
#ifndef HASH
    psRoutermid = smShmbuf_s.psShm_sys_route_m;
#else
    long dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psShm_sys_route_m_hash, sizeof(long));
    psRoutermid = (struct shm_sys_route_m *)(smShmbuf_s.psShm_sys_route_m_hash+dataoffset);
#endif
    for(i = 0; i < ilNum; i++)
    {

        memset(tmp, 0x00, sizeof(tmp));
        memset(fileName, 0x00, sizeof(fileName));
        memset(buf, 0x00, sizeof(buf));

        ilFmt = psRoutermid[i].sSwt_sys_route_m.route_grp;
        strcpy(fileName,dir);
        //strcat(fileName,"router/");
        sprintf(tmp,"%ld",ilFmt);
        strcat(fileName,tmp);
        strcat(fileName,".SCR");
        
        fp = fopen(fileName,"a");
        if(fp == NULL)
        {
            printf("打开文件[%s]失败！\n",fileName);
            exit(-1);
        }
        
        fwrite("\n[Router_m]\n",strlen("\n[Router_m]\n"),1,fp);


        sprintf(buf, "\nQ_ID=%d\nTRANCODE=%s\nFLAG=%d\n",
                psRoutermid[i].sSwt_sys_route_m.q_id,\
                psRoutermid[i].sSwt_sys_route_m.trancode,\
                psRoutermid[i].sSwt_sys_route_m.flag);
        fwrite(buf, strlen(buf), 1, fp);
        fclose(fp);

    }

    printf("检查路由脚步文件成功，请查看[%s]目录！\n",dir);
}



/**********************************************
**函数功能：读取静态共享内存中SWTRAN_CLT.CFG的数据
**参数    ：无
**返回值  ：无
**作者    ：王浩
**创建时间    ：20150330
**最后修改时间：20150521
  add by gengling at 2015.05.21 debug_level
***********************************************/
void _shmGetControl()
{

    char buf[BUFLEN];
    int i;
    int ilNum;
    struct swt_sys_tran  *pslTranid;
    FILE *fp;
    char fileName[256];
    char *envPath;
    char dir[200];
    
    int ilRet = swShmcheck();
    if(ilRet)
    {
        printf("check 失败！\n");
        exit(-1);
    }

    envPath = getenv("SWITCH_CHECK");
    
    if(envPath == NULL)
    {
        strcpy(dir, "/tmp/check");
        ilRet = mkdir(dir, 0755);
        if(ilRet)
        {
            if(errno != 17)
            {
                printf("创建目录[%s]失败！\n",dir);
                exit(-1);
            }
        }
    }
    else
    {
        strcpy(dir,envPath);
    }
    

    memset(fileName, 0x00, sizeof(fileName));
    strcpy(fileName,dir);
    strcat(fileName,"/SWTRAN_CTL.CFG");

    fp = fopen(fileName, "w");
    if(fp == NULL)
    {
        printf("创建文件SWTRAN_CTL.CFG失败！\n");
        exit(-1);
    }

#ifndef HASH
    pslTranid = (smShmbuf_s.psSwt_sys_tran);
    ilNum = psmShmidx_s->sIdx_sys_tran.iCount;
#else
    long dataoffset=0;
    memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
    pslTranid = (struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset);
    ilNum = psmShmidx_s->sIdx_sys_tran_hash.iCount;
#endif


    if( 0 == ilNum )
    {
        printf("在静态共享内存中没有SWTRAN_CTL.CFG的信息！\n");
        fclose(fp);
        exit(0);
    }
    else
    {
        memset(buf, 0x00, sizeof(buf));
        for(i = 0; i < ilNum; i++)
        {
            sprintf(buf, "%6d      %s      %s%6d      %s%6d %6d     %ld \n",pslTranid[i].sys_id, pslTranid[i].tran_code,\
                    pslTranid[i].tran_name, pslTranid[i].priority, pslTranid[i].status,\
                    pslTranid[i].tranning_max, pslTranid[i].debug_level, pslTranid[i].tranning_num);
            fwrite(buf, strlen(buf), 1, fp);
        }
    }

    fclose(fp);
    printf("检查SWTRAN_CTL.CFG成功，请查看[%s]\n",fileName);
}
