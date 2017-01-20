/*************************************************************************/
/* 模块编号    ：swRsync                                              */
/* 模块名称    ：同步远程配置文件到本地并加载到内存                     */
/* 版 本 号    ：                                                        */
/* 作    者    ：                                                        */
/* 建立日期    ：2006-12-30                                              */
/* 最后修改日期：                                                        */
/* 模块用途    ：交换平台配置管理文件数据库导入                          */
/* 本模块中包含如下函数及功能说明：                                      */
/*（1）int main();                                                       */
/* 函数说明    ：数据库相关数据导出                              */
/*（2）int swFile_format(long al_count,short rcd_count,char *tabname);  */
/* 函数说明    ：数据库数据格式调整,并写入相关文件                       */
/*************************************************************************/

/*************************************************************************/
/* 修改记录:                                                             */
/*最后修改日期2007-01-10                                                 */
/*************************************************************************/

/* 头文件定义 */
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <errno.h>
#include <stdio.h>   
#include <dirent.h>
/*#include "sqlenv.h"*/
#include "switch.h"

/* switch定义 */

#ifdef OS_AIX
#define CHANGCODE  "dos2aix"
#endif

#ifdef OS_LINUX
#define CHANGCODE  "dos2unix"
#endif

char aRemote_url_cfg[128];
char aRemote_url_all[128];
int iport=0;
char aPsw_file[128];
char aLocal_dir[128];
char aHostNameCfg[128];
char Hostname[41];

int swLoop_for_update_cfg();
int strtoupper(char *aBuf);
int run_sys_cmd(char *aCmd);
int GetProfileString(char *FileName,char *Section,char *Index,char *GetValue);
int GetFormatTime( char *FormatTime, int iLen, const char *Format );
int setCfgFlag(char *psCfgFile, int iswCfgFlag,int idbCfgFlag);
int iGetConfig();
int getCfgFlag(char *psCfgFile, int *iswCfgFlag,int *idbCfgFlag);
/**************************************************************/
/* 函数名: main()                                             */
/* 功  能: 数据库数据导出至相关配置文件                       */
/* 作  者: hxz                                                */
/* 建立日期: 2006-12-30                                       */
/* 最后修改日期:                                              */
/* 调用其它函数: swFile_format();                             */
/* 全局变量:                                                  */
/* 参数含义:                                                  */
/* 返回值:                                                    */
/**************************************************************/
int main(int argc,char **argv)
{   
    /* 设置调试程序名称 */
    memset(Hostname,0,sizeof(Hostname));
    /* 设置调试程序名称 */ 

    memset(agDebugfile,0x00,sizeof(agDebugfile));
    sprintf(agDebugfile,"%s.debug",argv[0]);
    if (iGetConfig()!=0)
    {
        exit(1);
    }

    swVdebug(2,"S0000: 版本号[1.0.0]");
    swVdebug(2,"S0010: DEBUG文件[%s]",agDebugfile);
    swVdebug(2,"S0020: DEBUG标志[%d]",cgDebug);

    gethostname(Hostname, sizeof(Hostname));
    _swTrim(Hostname);
    strtoupper(Hostname);

    memset(aHostNameCfg,0,sizeof(aHostNameCfg));
    sprintf(aHostNameCfg,"%s.CFG",Hostname);

    swLoop_for_update_cfg();

    return 0;
}


int swLoop_for_update_cfg()
{
//    char  alTime_Now[15];
    char sConfigFile[128];
    char aCmd[256];
    char sTmp[256];
    int iRet = 0;
    int itime = 60;
    int idbCfgFlag = 0;
    int iswCfgFlag = 0;

    while(1)
    {
        //同步远程配置文件到本地
        //rsync -vzrtopg --progress --password-file=/app/switch/switch/etc/rsyncd.passwd --port=873 test@20.13.0.216::test/c
        memset(aCmd, 0x00, sizeof(aCmd));
        sprintf(aCmd,"rsync -vzrtopg --progress --password-file=%s/%s --port=%d %s/%s %s/%s",
                getenv("HOME"),aPsw_file,iport,aRemote_url_cfg,aHostNameCfg,getenv("HOME"),aLocal_dir);
        iRet = run_sys_cmd(aCmd);
        if (iRet != 0 )
        {
            swVdebug(2,"run cmd[%s] error,errno=%d!",aCmd,errno);
            sleep(itime);
            continue;
        }

        memset(sConfigFile, 0x00, sizeof(sConfigFile));
        sprintf(sConfigFile,"%s/%s/%s", getenv("HOME"),aLocal_dir,aHostNameCfg);

//        GetFormatTime(alTime_Now,sizeof(alTime_Now),"%y%m%d%h%m%s");
//  获取同步标志 0,未同步，1,已同步
        if ((iRet = getCfgFlag(sConfigFile, &iswCfgFlag,&idbCfgFlag))!=0)
        {
            swVdebug(2,"getCfgFlag[%s] ret=%d,errno=%d!",sConfigFile,iRet,errno);
            sleep(itime);
            continue;
        }

        swVdebug(2,"iswCfgFlag [%d] idbCfgFlag=[%d]!",iswCfgFlag,idbCfgFlag);
        //如果有一项没有同步，则把所有的标志改为已同步传到远程服务器
        if(iswCfgFlag == 0 || idbCfgFlag == 0)
        {
                setCfgFlag(sConfigFile, 1,1);
                //同步本地配置文件到远程目录
                //rsync -vzrtopg --progress --password-file=/app/switch/switch/etc/rsyncd.passwd --port=873 test@20.13.0.216::test/c .
                memset(aCmd, 0x00, sizeof(aCmd));
                sprintf(aCmd,"rsync -vzrtopg --progress --password-file=%s/%s --port=%d %s/%s/%s %s",
                                getenv("HOME"),aPsw_file,iport,getenv("HOME"),aLocal_dir,aHostNameCfg,aRemote_url_cfg);
                iRet = run_sys_cmd(aCmd);
                if (iRet != 0 )
                {
                        swVdebug(2,"run cmd[%s] error,errno=%d!",aCmd,errno);
                        sleep(itime);
                        continue;
                }

        }
        if ( iswCfgFlag == 0 )
        {
            //同步所有的配置文件到本地
            //rsync -vzrtopg --progress --password-file=/app/switch/switch/etc/rsyncd.passwd --port=873 test@20.13.0.216::test/c
            memset(aCmd,0,sizeof(aCmd));
            sprintf(aCmd,"rsync -vzrtopg --progress --password-file=%s/%s --port=%d %s %s/%s",
                    getenv("HOME"),aPsw_file,iport,aRemote_url_all,getenv("HOME"),aLocal_dir);
            iRet = run_sys_cmd(aCmd);
            if (iRet != 0 )
            {
                swVdebug(2,"run cmd[%s] error,errno=%d!",aCmd,errno);
                sleep(itime);
                continue;
            }

            //刷新共享内存
            memset(aCmd,0,sizeof(aCmd));
            strcat(aCmd,"swInit 2>&1");
            iRet = run_sys_cmd(aCmd);
            if (iRet != 0 )
            {
                swVdebug(2,"run cmd[%s] error,errno=%d!",aCmd,errno);
                sleep(itime);
                continue;
            }

        }
        if ( idbCfgFlag == 0 )
        {
            //同步所有的配置文件到本地
            //rsync -vzrtopg --progress --password-file=/app/switch/switch/etc/rsyncd.passwd --port=873 test@20.13.0.216::test/c
            memset(aCmd,0,sizeof(aCmd));
            sprintf(aCmd,"rsync -vzrtopg --progress --password-file=%s/%s --port=%d %s %s/%s",
                    getenv("HOME"),aPsw_file,iport,aRemote_url_all,getenv("HOME"),aLocal_dir);
            iRet = run_sys_cmd(aCmd);
            if (iRet != 0 )
            {
                swVdebug(2,"run cmd[%s] error,errno=%d!",aCmd,errno);
                sleep(itime);
                continue;
            }

            //刷新共享内存
            memset(aCmd,0,sizeof(aCmd));
            strcat(aCmd,"sh init.sh 2>&1");
            iRet = run_sys_cmd(aCmd);
            if (iRet != 0 )
            {
                swVdebug(2,"run cmd[%s] error,errno=%d!",aCmd,errno);
                sleep(itime);
                continue;
            }

        }
        sleep(60);
    }
}

int run_sys_cmd(char *aCmd)
{
    char aTmp[256];
    FILE *pp = NULL;

    memset(aTmp,0,sizeof(aTmp));

    swVdebug(2,"run command:[%s]",aCmd);
    if((pp = popen(aCmd, "r")) == NULL)
    {
        swVdebug(2,"popen() return error,errno=%d!",errno);
        return -1;
    }

    while(fgets(aTmp,sizeof(aTmp)-1,pp)!=NULL)
    {
        swVdebug(2,"%s",aTmp);
        memset(aTmp,0,sizeof(aTmp));
    }
    pclose(pp);
    return 0;
}

int iGetConfig()
{
    char    sTmp[256];
    char    sConfigFile[256];
    char    sErrMsg[256];

    memset(sTmp, 0x00, sizeof(sTmp));
    memset(sConfigFile, 0x00, sizeof(sConfigFile));
    memset(sErrMsg, 0x00, sizeof(sErrMsg));

    cgDebug = 5;
    sprintf(sConfigFile, "%s/switch/etc/config/%s", getenv("HOME"), "SWRSYNC.CFG");
    swVdebug(3, "配置文件:[%s]", sConfigFile);

    /* Comm日志级别 */
    memset(sTmp,0,sizeof(sTmp));
    if (GetProfileString(sConfigFile, "CONFIG", "DEBUG", sTmp) < 0)
    {
        strcpy(sErrMsg, "Comm日志级别");
        goto ErrHandle;
    }    
    cgDebug = atoi(sTmp);
    swVdebug(3, "cgDebug:[%d]", cgDebug);

    /* REMOTE_URL_CFG */
    memset(sTmp,0,sizeof(sTmp));
    memset(aRemote_url_cfg,0,sizeof(aRemote_url_cfg));
    if (GetProfileString(sConfigFile, "CONFIG", "REMOTE_URL_CFG", sTmp) < 0)
    {
        strcpy(sErrMsg, "REMOTE_URL_CFG");
        goto ErrHandle;
    }   
    strcpy(aRemote_url_cfg,sTmp);
    swVdebug(3, "aRemote_url_cfg:[%s]", aRemote_url_cfg);

    /* REMOTE_URL_ALL */
    memset(sTmp,0,sizeof(sTmp));
    memset(aRemote_url_all,0,sizeof(aRemote_url_all));
    if (GetProfileString(sConfigFile, "CONFIG", "REMOTE_URL_ALL", sTmp) < 0)
    {
        strcpy(sErrMsg, "REMOTE_URL_ALL");
        goto ErrHandle;
    }   
    strcpy(aRemote_url_all,sTmp);
    swVdebug(3, "aRemote_url_all:[%s]", aRemote_url_all);

    /* port*/
    memset(sTmp,0,sizeof(sTmp));
    if (GetProfileString(sConfigFile, "CONFIG", "PORT", sTmp) < 0)
    {
        strcpy(sErrMsg, "PORT");
        goto ErrHandle;
    }    
    iport = atoi(sTmp);
    swVdebug(3, "iport:[%d]", iport);

    /* PSW_FILE */
    memset(sTmp,0,sizeof(sTmp));
    memset(aPsw_file,0,sizeof(aPsw_file));
    if (GetProfileString(sConfigFile, "CONFIG", "PSW_FILE", sTmp) < 0)
    {
        strcpy(sErrMsg, "PSW_FILE");
        goto ErrHandle;
    }   
    strcpy(aPsw_file,sTmp);
    swVdebug(3, "aPsw_file:[%s]", aPsw_file);

    /* LOCAL_DIR */
    memset(sTmp,0,sizeof(sTmp));
    memset(aLocal_dir,0,sizeof(aLocal_dir));
    if (GetProfileString(sConfigFile, "CONFIG", "LOCAL_DIR", sTmp) < 0)
    {
        strcpy(sErrMsg, "LOCAL_DIR");
        goto ErrHandle;
    }   
    strcpy(aLocal_dir,sTmp);
    swVdebug(3, "aLocal_dir:[%s]", aLocal_dir);

    return 0;

ErrHandle:
    swVdebug(2, "ERROR:获取[%s]配置信息失败!配置文件[%s]错误码[%d]",
            sErrMsg, sConfigFile, errno);
    printf("ERROR:获取[%s]配置信息失败!配置文件[%s]错误码[%d]",
            sErrMsg, sConfigFile, errno);

    return -1;
}

int GetProfileString(char *FileName,char *Section,char *Index,char *GetValue)
{   

    FILE *fp;
    int iSectionLen,iIndexLen,iFind;
    char buff[512];
    int i;  

    iSectionLen=strlen(Section);
    iIndexLen=strlen(Index);

    swVdebug(3,"FileName=%s",FileName);

    /* 打开配置文件 */


    if ( ( fp = fopen (FileName,"r") ) == NULL )
        return (-1);
    /* 寻找匹配条件 */

    iFind =-2;
    while( fgets(buff,sizeof(buff) - 1,fp)) {
        if ( *buff == '#' || *buff == ';' )
            continue;

        if ( (*buff=='[') && (*(buff+iSectionLen+1)==']') && \
                (!memcmp(buff+1,Section,iSectionLen)) ) {
            iFind =0;
            break;
        }
    }       
    if (iFind) {
        fclose(fp);
        return ( -2 );
    }

    /* 寻找匹配数据 */

    iFind =-3;  

    while( fgets(buff,sizeof(buff),fp)) {

        if (*buff=='[' )
            break;

        if (*buff=='#'||*buff==';')
            continue;

        if ( (*buff==*Index) && ( *(buff+iIndexLen)=='=') && \
                (!memcmp(buff,Index,iIndexLen)) ) {

            for (i = iIndexLen;i<= strlen(buff);i++){

                if (buff[i] == ';'|| buff[i] == '#'|| buff[i]=='\n'){
                    buff[i] = 0x00;
                    break;
                }       
            }
            strcpy( GetValue,buff+iIndexLen+1);
            iFind = strlen(GetValue) ;
            GetValue[iFind]=0;
            _swTrim(GetValue);
            break;
        }
    }
    fclose(fp);
    return (iFind);
}

/*
** SHANGHAI BRILLIANCE TECHNOLOGY CENTER
** 函数名称 _swChgvalue
** 函数说明 根据约定的规则取字符串的值
** 建立日期
** 参数说明
*/
int _swChgvalue1(char *aValue)
{
  char alBuf[257];

  _swTrim(aValue);
  if ((aValue[0] == '"') && (aValue[strlen(aValue) - 1] == '"')) {
    strcpy(alBuf, aValue);
    strcpy(aValue, alBuf + 1);
    aValue[strlen(aValue) - 1] = '\0';
  }
  return 0;
}


int getCfgFlag(char *psCfgFile, int *iswCfgFlag,int *idbCfgFlag)
{
    char alBuf[257],alResult[101],*alTmp;
    FILE *fp = NULL;
    
    memset(alBuf,0,sizeof(alBuf));
//    if ((fp = _swFopen("config/SWCONFIG.CFG", "r")) == NULL) return -1; 
    if ((fp = fopen(psCfgFile, "r")) == NULL) return -1; 
    while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
    {
        _swTrim(alBuf);
        if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
        if ((alTmp = strchr(alBuf,'#')) != NULL) *alTmp = '\0';
        _swTrim(alBuf);
        if ((alBuf[0] == '[') && (alBuf[strlen(alBuf) -1] == ']')) continue;
        if ((alTmp = strchr(alBuf, '=')) == NULL) continue;
        strcpy(alResult,alTmp + 1);
        _swChgvalue(alResult);
        *alTmp = '\0';
        _swTrim(alBuf);
        if (strcmp(alBuf,"SWSHMFLAG") == 0)
        {
            *iswCfgFlag = atoi(alResult);
        }
        if (strcmp(alBuf,"DBSHMFLAG") == 0)
        {
            *idbCfgFlag = atoi(alResult);
        }
        memset(alBuf,0,sizeof(alBuf));
    }
    fclose(fp);
    return 0;
}

int setCfgFlag(char *psCfgFile, int iswCfgFlag,int idbCfgFlag)
{
    FILE *fp = NULL;
//    if ((fp = _swFopen("config/SWCONFIG.CFG", "w")) == NULL) return -1; 
    if ((fp = fopen(psCfgFile, "w")) == NULL) return -1; 
    fprintf(fp,"SWSHMFLAG=%d\n",iswCfgFlag);
    fprintf(fp,"DBSHMFLAG=%d\n",idbCfgFlag);
    fclose(fp);
    return 0;
}


/***************************************************************
 ** 函数名      : strtoupper
 ** 功  能      : 字符串转换为大写
 ** 作  者      :       
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数: 
 ** 全局变量    : 
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败 
***************************************************************/
int strtoupper(char *aBuf)
{
  while (*aBuf != '\0')
  {
    *aBuf = toupper(*aBuf);
    aBuf++; 
  }
  return(0);
}

/*************************************************************
 ** 函数名：        GetFormatTime
 ** 功  能：        根据<Format>返回对应的字符串
 ** 作  者：        xzhu    
 ** 建立日期：      2001/08/21
 ** 最后修改日期：
 ** 调用其它函数：
 ** 全局变量：
 ** 参数含义：      <FormatTime>    :返回字符串指针
            <iLen>      :长度   
            <Format>    :指定格式
                如：%y%Y%m%d%H%M%S...
 ** 返回值：
***************************************************************/

int GetFormatTime( char *FormatTime, int iLen, const char *Format )
{
   time_t       clock;  
   int      iRc;    

   clock = time ( (time_t *) 0 ); 
   iRc= strftime ( FormatTime, iLen , Format , localtime ( &clock ) );
   return ( iRc );
}
