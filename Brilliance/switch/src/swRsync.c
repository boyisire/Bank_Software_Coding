/*************************************************************************/
/* ģ����    ��swRsync                                              */
/* ģ������    ��ͬ��Զ�������ļ������ز����ص��ڴ�                     */
/* �� �� ��    ��                                                        */
/* ��    ��    ��                                                        */
/* ��������    ��2006-12-30                                              */
/* ����޸����ڣ�                                                        */
/* ģ����;    ������ƽ̨���ù����ļ����ݿ⵼��                          */
/* ��ģ���а������º���������˵����                                      */
/*��1��int main();                                                       */
/* ����˵��    �����ݿ�������ݵ���                              */
/*��2��int swFile_format(long al_count,short rcd_count,char *tabname);  */
/* ����˵��    �����ݿ����ݸ�ʽ����,��д������ļ�                       */
/*************************************************************************/

/*************************************************************************/
/* �޸ļ�¼:                                                             */
/*����޸�����2007-01-10                                                 */
/*************************************************************************/

/* ͷ�ļ����� */
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

/* switch���� */

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
/* ������: main()                                             */
/* ��  ��: ���ݿ����ݵ�������������ļ�                       */
/* ��  ��: hxz                                                */
/* ��������: 2006-12-30                                       */
/* ����޸�����:                                              */
/* ������������: swFile_format();                             */
/* ȫ�ֱ���:                                                  */
/* ��������:                                                  */
/* ����ֵ:                                                    */
/**************************************************************/
int main(int argc,char **argv)
{   
    /* ���õ��Գ������� */
    memset(Hostname,0,sizeof(Hostname));
    /* ���õ��Գ������� */ 

    memset(agDebugfile,0x00,sizeof(agDebugfile));
    sprintf(agDebugfile,"%s.debug",argv[0]);
    if (iGetConfig()!=0)
    {
        exit(1);
    }

    swVdebug(2,"S0000: �汾��[1.0.0]");
    swVdebug(2,"S0010: DEBUG�ļ�[%s]",agDebugfile);
    swVdebug(2,"S0020: DEBUG��־[%d]",cgDebug);

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
        //ͬ��Զ�������ļ�������
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
//  ��ȡͬ����־ 0,δͬ����1,��ͬ��
        if ((iRet = getCfgFlag(sConfigFile, &iswCfgFlag,&idbCfgFlag))!=0)
        {
            swVdebug(2,"getCfgFlag[%s] ret=%d,errno=%d!",sConfigFile,iRet,errno);
            sleep(itime);
            continue;
        }

        swVdebug(2,"iswCfgFlag [%d] idbCfgFlag=[%d]!",iswCfgFlag,idbCfgFlag);
        //�����һ��û��ͬ����������еı�־��Ϊ��ͬ������Զ�̷�����
        if(iswCfgFlag == 0 || idbCfgFlag == 0)
        {
                setCfgFlag(sConfigFile, 1,1);
                //ͬ�����������ļ���Զ��Ŀ¼
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
            //ͬ�����е������ļ�������
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

            //ˢ�¹����ڴ�
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
            //ͬ�����е������ļ�������
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

            //ˢ�¹����ڴ�
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
    swVdebug(3, "�����ļ�:[%s]", sConfigFile);

    /* Comm��־���� */
    memset(sTmp,0,sizeof(sTmp));
    if (GetProfileString(sConfigFile, "CONFIG", "DEBUG", sTmp) < 0)
    {
        strcpy(sErrMsg, "Comm��־����");
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
    swVdebug(2, "ERROR:��ȡ[%s]������Ϣʧ��!�����ļ�[%s]������[%d]",
            sErrMsg, sConfigFile, errno);
    printf("ERROR:��ȡ[%s]������Ϣʧ��!�����ļ�[%s]������[%d]",
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

    /* �������ļ� */


    if ( ( fp = fopen (FileName,"r") ) == NULL )
        return (-1);
    /* Ѱ��ƥ������ */

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

    /* Ѱ��ƥ������ */

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
** �������� _swChgvalue
** ����˵�� ����Լ���Ĺ���ȡ�ַ�����ֵ
** ��������
** ����˵��
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
 ** ������      : strtoupper
 ** ��  ��      : �ַ���ת��Ϊ��д
 ** ��  ��      :       
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������: 
 ** ȫ�ֱ���    : 
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ�� 
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
 ** ��������        GetFormatTime
 ** ��  �ܣ�        ����<Format>���ض�Ӧ���ַ���
 ** ��  �ߣ�        xzhu    
 ** �������ڣ�      2001/08/21
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ�����
 ** �������壺      <FormatTime>    :�����ַ���ָ��
            <iLen>      :����   
            <Format>    :ָ����ʽ
                �磺%y%Y%m%d%H%M%S...
 ** ����ֵ��
***************************************************************/

int GetFormatTime( char *FormatTime, int iLen, const char *Format )
{
   time_t       clock;  
   int      iRc;    

   clock = time ( (time_t *) 0 ); 
   iRc= strftime ( FormatTime, iLen , Format , localtime ( &clock ) );
   return ( iRc );
}
