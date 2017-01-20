#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include <errno.h>
#include "PlSec.h"
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*******************************************************************
 * 函数名        : HsmMACGen_New
 * 函数功能      : 用于交易的MAC生成
 * 入口参数      : syscode 系统代码 4位 01 储蓄 后两位为省代字
 *                 macblock：参加计算mac的数据域
 *                 blocklen：参加计算mac的数据域的长度
 * 出口参数      : returnmac：返回的mac
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl    
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
int HsmMACGen_New(char *syscode, char *macblock, int blocklen,char *returnmac)
{
        int ilsocket;
        int flag;
	int iret;
        struct msgMacPack msgMacbuf;
	swVdebug(3,"agDebugfile=%s",agDebugfile);
        swVdebug(3,"swStructInit 初始化开始");
        /*结构体初始化*/
        swStructInit(flag,&msgMacbuf,macblock,blocklen,syscode);
        swVdebug(3,"swStructInit 初始化结束");
        /*连接加密sever端*/
        swVdebug(3,"swConnectKey 开始");
	swVdebug(3,"msgMacbuf.encKey=%s",msgMacbuf.encKey);
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey 结束");
        /*开始申请*/
        iret=swbegindKey(ilsocket,&msgMacbuf);
	if(iret){
		swVdebug(3,"swbegindKey error iret=%d",iret);
		return -2;
		}
        /*接收结果*/
        swVdebug(3,"msg Mac[%s]len[%d]",msgMacbuf.macData,strlen(msgMacbuf.macData));
        swRevKey(msgMacbuf,returnmac);
        if(strlen(msgMacbuf.macData)==0) {
		swVdebug(1,"获取MAC失败");
                return -5;
	}
	swVdebug(3,"strlen(returnmac)=%d",strlen(returnmac));
		
    return 0;
}


/*******************************************************************
 * 函数名        : swStructInit
 * 函数功能      : 自动补齐默认参数
 * 入口参数      : 
 *                
 *                 
 * 出口参数      : flag    
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl    
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
int swStructInit(int flag,struct msgMacPack *msgMacbuf,char *macblock,int blocklen,char *syscode)
{
        msgMacbuf->tran_type=1;
        msgMacbuf->algoType=2; 
        strcpy(msgMacbuf->subjectID,syscode);
        msgMacbuf->subjectIDLen=strlen(msgMacbuf->subjectID);
        strcpy(msgMacbuf->userData,macblock);
        msgMacbuf->userDataLen=blocklen;
    return 0;
}
/*******************************************************************
 * 函数名        :swConnectKey 
 * 函数功能      :连接密管平台sever端
 * 入口参数      :ip  port 
 * 出口参数      : 
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl
 * 初作时间      : 2015-04-08
 * 修改纪录      :
 ********************************************************************/
int swConnectKey()
{
        int ilSocket;
        int ilRn;
        int recg=0;
	time_t nowtime;
	time_t settime;
	nowtime=time(NULL);
	settime=nowtime-extconfig.oldtime;
	swVdebug(3,"settime=%ld",settime);
	swVdebug(3,"extconfig.freshtime=%ld",extconfig.freshtime);

	if((flag == -1) || (settime > extconfig.freshtime))
	{
		flag=0;
		recg=GetProfileValue();
		if(recg<0)
			swVdebug(3,"GetProfileValue error");
	}
	swVdebug(3,"extconfig.aip=%s,extconfig.iport=%d",extconfig.aip,extconfig.iport);
        ilSocket = PtConnectser_sec(extconfig.aip,extconfig.iport); 
        if (ilSocket < 0)
        {       
                swVdebug(3,"ERROR:连接到服务端[%s]端口=[%d]出错");
                close(ilSocket);      
                return -2;
        }       

        return ilSocket;
}

/*******************************************************************
 * 函数名        :swbegindKey
 * 函数功能      :申请MAC
 * 入口参数      :ip  port
 *
 *
 * 出口参数      :
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl
 * 初作时间      : 2015-04-08
 * 修改纪录      :
 ********************************************************************/
int swbegindKey(int ilSocket,struct msgMacPack *sMsgPack)
{
        int ilRn;
        int igHeadlen=4;
        int ilMsglen;

        /*发送交易请求*/
        swVdebug(3,"ilSocket=%d",ilSocket);
        swVdebug(3,"sMsgPack->algoType=%d",sMsgPack->algoType);
        swVdebug(3,"sizeof(struct msgMacPack)=%d",sizeof(struct msgMacPack));

        ilRn = PtTcpSndHead_sec(igHeadlen, ilSocket,sMsgPack, sizeof(struct msgMacPack));

        if ( ilRn==0 )
        {/*发送正确处理*/
                swVdebug(1,"send to Sock[%d] success",ilSocket);
        }       
        else    
        {       
                close(ilSocket);
                swVdebug(3,"ERROR:PbTcpSndHead失败 error,ilRc= [%d]", ilRn);
                return -3;
        }       
        /*接收交易应答*/
        ilMsglen = PtTcpRcvHeadTime_sec(igHeadlen,ilSocket,sMsgPack,sizeof(struct msgMacPack), 3);
        if ( ilMsglen<0 ){
                close(ilSocket);
                swVdebug(3,"ERROR:PbTcpRcvHead失败 error[%d],ilRc= [%d],strerror=[%s]",errno,ilMsglen,strerror(errno));
                return -4;
        }       
        swVdebug(3, "PbTcpRcvHead收到报文，长度为[%d]", ilMsglen);
        close(ilSocket);
        return 0;

}
/*******************************************************************
 * 函数名        :swRevKey
 * 函数功能      :取得MAC值
 * 入口参数      :recmac
 *
 *
 * 出口参数      :
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl
 * 初作时间      : 2015-04-08
 * 修改纪录      :
 ********************************************************************/
int swRevKey(struct msgMacPack msgMacPack,char *returnmac)
{
        memcpy(returnmac,msgMacPack.macData,strlen(msgMacPack.macData));
        return 0;
}

/******************************************************************* 
 * 函数名        : HsmPINConvert_New
 * 函数功能      : 实现转加密
 * 入口参数      : syscode 后台系统代码 4位 01 储蓄 后两位为省代字
 *                 pan：帐号
 *                 pin_in：传入的密码密文（不带帐号）
 *                         
 * 出口参数      : pin_out：后台业务系统密文
 * 返回码        : 成功 ：0 
 *               : 失败 ：<0
 * 调用自定义函数: 
 * 处理概要      :       
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : pc    
 * 初作时间      : 2008-3-11 20:08
 * 修改纪录      :       
********************************************************************/
int HsmPINConvert_New(char *syscode, char *pan,char *pin_in,char *pin_out,char *syscodeb,char *aflag)
{
        int ilsocket;
        struct msgMacPack msgMacbuf;
        swVdebug(3,"pan=%s,pin_in=%s",pan,pin_in);
        swVdebug(3,"agDebugfile=%s",agDebugfile);
        swVdebug(3,"swStructInitPIN 初始化开始");
        /*结构体初始化*/
        swStructInitPIN(aflag,&msgMacbuf,pan,pin_in,syscode,syscodeb);
        swVdebug(3,"swStructInitPIN 初始化结束");
        /*连接加密sever端*/
        swVdebug(3,"swConnectKey 开始");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey 结束");
        /*开始申请*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*接收结果*/
        swRevKeyPIN(msgMacbuf,pin_out);
    return 0;

}
/*******************************************************************
 * 函数名        : swStructInitPIN
 * 函数功能      : 自动补齐默认参数 (实现转加密)
 * 入口参数      : 
 *                
 *                 
 * 出口参数      : flag    
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl    
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
int swStructInitPIN(char* aflag,struct msgMacPack *msgMacbuf,char *pan,char *pin_in,char *syscode,char *syscodeb)
{
	swVdebug(3,"syscode_test=%s",syscode);
	swVdebug(3,"tbl_test aflag=%s",aflag);
        msgMacbuf->tran_type=2;
        strcpy(msgMacbuf->srcSubjectID,syscode);
        strcpy(msgMacbuf->destSubjectID,syscodeb);
        strcpy(msgMacbuf->userAccount,pan);
        strcpy(msgMacbuf->srcPIN,pin_in);
	strcpy(msgMacbuf->account_flag,aflag);    
    return 0;
}
/*******************************************************************
 * 函数名        :swRevKeyPIN
 * 函数功能      :取得MAC值
 * 入口参数      :recmac
 *
 *
 * 出口参数      :
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl
 * 初作时间      : 2015-04-08
 * 修改纪录      :
 ********************************************************************/
int swRevKeyPIN(struct msgMacPack msgMacPack,char *pinout)
{
        memcpy(pinout,msgMacPack.destPIN,strlen(msgMacPack.destPIN));
        return 0;
}


/**************************************************************
 ** 函 数 名：HsmPINtoMD5_New
 ** 功    能：DES密码转加密为MD5散列函数
 ** 作    者：YZ  
 ** 建立日期：2011/6/13
 ** 修改日期：
 ** 调用函数：
 ** 全局变量：
 ** 参数含义： aParm[0] 后台系统代字 
 **            aParm[1] 折号/卡号-参与解密运算
 **            aParm[2] 密文PINBLOCK  
 **            aParm[3] MD5的SALT
 ** 返 回 值： 0 成功 aResult 解密后的PIN明文,账号参加运算
***************************************************************/
int HsmPINtoMD5_New(char *syscode, char *pan,char *pin_in,char *salt,char *pin_md5)
{
        int ilsocket;
        int flag;
        struct msgMacPack msgMacbuf;
        

        swVdebug(3,"swStructInitMD5 初始化开始");
        /*结构体初始化*/
      //  swStructInitMD5(flag,&msgMacbuf,pan,pin_in,salt,syscode);
       /*add by zd */
        swStructInitMD5(&msgMacbuf,pan,pin_in,salt,syscode);
        swVdebug(3,"swStructInitMD5 初始化结束");
        /*连接加密sever端*/
        swVdebug(3,"swConnectKey 开始");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey 结束");
        /*开始申请*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*接收结果*/
        swRevKeyMD5(msgMacbuf,pin_md5);
    return 0;
}
/*******************************************************************
 * 函数名        : swStructInitMD5
 * 函数功能      : 自动补齐默认参数
 * 入口参数      : 
 *                
 *                 
 * 出口参数      : flag    
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl    
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
//int swStructInitMD5(int flag,struct msgMacPack *msgMacbuf,char *pan,char *pin_in,char *salt,char *syscode)
int swStructInitMD5(struct msgMacPack *msgMacbuf,char *pan,char *pin_in,char *salt,char *syscode)
{
        msgMacbuf->tran_type=3;
        strcpy(msgMacbuf->subjectID,syscode);
        strcpy(msgMacbuf->userAccount,pan);
        strcpy(msgMacbuf->EncSubjectPIN,pin_in);        
        strcpy(msgMacbuf->salt,salt);
        swVdebug(3,"syscode:%s",syscode);
        swVdebug(3,"salt:%s",salt);
	
    return 0;

}
/*******************************************************************
 * 函数名        :swRevKeyMD5
 * 函数功能      :取得MAC值
 * 入口参数      :recmac
 *
 *
 * 出口参数      :
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl
 * 初作时间      : 2015-04-08
 * 修改纪录      :
 ********************************************************************/
int swRevKeyMD5(struct msgMacPack msgMacPack,char *pin_md5)
{
        memcpy(pin_md5,msgMacPack.hashPIN,strlen(msgMacPack.hashPIN));
    return 0;
}


/*******************************************************************
 * 函数名        :GetProfileString_m
 * 函数功能      :取得配置文件中内容
 * 入口参数      :
 *
 *
 * 出口参数      :
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl   
 * 初作时间      : 2015-04-08
 * 修改纪录      :
*******************************************************************/

int GetProfileString_m(char *FileName,char *Section,char *Index,char *GetValue)
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
                /*if (buff[i] == ' ' || buff[i] == '\n'){*/
                
                if (buff[i] == ';'|| buff[i] == '#'|| buff[i]=='\n'){
                    buff[i] = 0x00;
                    break;
                }       
            }
            strcpy( GetValue,buff+iIndexLen+1);
            iFind = strlen(GetValue) ;
            GetValue[iFind]=0;
            trim(GetValue);
            break;
        }
    }
    fclose(fp);
    return (iFind);
}


/**************************************************************
 ** 函数名      : _swTrim 
 ** 功  能      : 该函数将字符串 s 的前后空格及尾回车去掉
 ** 作  者      : llx   
 ** 建立日期    : 1999/11/4
 ** 最后修改日期: 2000/3/10
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : s   :字符串 
 ** 返回值      : SUCCESS 
          转换后的字符串从s返回
***************************************************************/
int trim(s) 
char *s;
{
  short i, l, r, len; 

  for(len=0; s[len]; len++); 
  for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n' || s[l]=='\r'); l++);
  if(l==len)
  {
    s[0]='\0';
    return(1);
  }
  for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n' || s[r]=='\r'); r--);
  for(i=l; i<=r; i++) s[i-l]=s[i];
  s[r-l+1]='\0';
  return(1);
}
/**************************************************************
 ** 函数名      : _swTrim 
 ** 功  能      : 该函数将字符串 s 的前后空格及尾回车去掉
 ** 作  者      : llx   
 ** 建立日期    : 1999/11/4
 ** 最后修改日期: 2000/3/10
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : s   :字符串 
 ** 返回值      : SUCCESS 
          转换后的字符串从s返回
***************************************************************/
int GetProfileValue()
{
        int rec;
        char aConfigFile[128];

        char iport[12];
        char agip[32];
        char portnum[4];
	time_t oldtime;
	char freshtime[12];
	extconfig.oldtime=time(NULL);

        sprintf(aConfigFile,"%s/%s",getenv("HOME"),"switch/etc/seckey.ini");

        swVdebug(3,"aConfigFile_test=%s",aConfigFile); 
        if (GetProfileString_m(aConfigFile,"SOFT","PORT",iport) < 0)
        {
            swVdebug(3,"ERROR:取得PORT 失败[%s][%d]",aConfigFile,errno);
            return -1;
         }
	extconfig.iport=atoi(iport);
        if (GetProfileString_m(aConfigFile,"SOFT","HOSTIP",agip) < 0)
        {
            swVdebug(3,"ERROR:取得HOSTIP 失败[%s][%d]",aConfigFile,errno);
            return -1;
         }
	strcpy(extconfig.aip,agip);	
        if (GetProfileString_m(aConfigFile,"SOFT","FRESHTIME",freshtime) < 0)
        {
            swVdebug(3,"ERROR:取得FRESHTIME 失败[%s][%d]",aConfigFile,errno);
            return -1;
         }
	extconfig.freshtime=atoi(freshtime);
        return 0;
}
/*******************************************************************
 * 函数名        : HsmMAC_APPLY_New
 * 函数功能      : 用于申请秘钥的交易
 * 入口参数      : syscode 系统代码 
 *                 worktype：工作密钥类型
 *                 blocklen：参加计算mac的数据域的长度
 * 出口参数      : returnmac：返回的mac
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl    
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
int HsmMAC_APPLY_New(char *syscode, char *worktype, int blocklen,char *returnmac)
{
        int ilsocket;
        int flag;
        struct msgMacPack msgMacbuf;
        swVdebug(3,"swStructInitAPPLY 初始化开始");
        /*结构体初始化*/
        swStructInitAPPLY(flag,&msgMacbuf,worktype,blocklen,syscode);
        swVdebug(3,"swStructInitAPPLY 初始化结束");
        /*连接加密sever端*/
        swVdebug(3,"swConnectKeyAPPLY 开始");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKeyAPPLY error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKeyAPPLY 结束");
        /*开始申请*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*接收结果*/
        swRevKeyAPPLY(msgMacbuf,returnmac);
    return 0;

}
/*******************************************************************
 * 函数名        : swStructInitAPPLY
 * 函数功能      : 自动补齐默认参数(秘钥申请)
 * 入口参数      : 
 *                
 *                 
 * 出口参数      : flag    
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl    
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
int swStructInitAPPLY(int flag,struct msgMacPack *msgMacbuf,char *worktype,int blocklen,char *syscode)
{
        msgMacbuf->tran_type=4;
        msgMacbuf->algoType=1;
        msgMacbuf->workKeyType=atoi(worktype); 
        strcpy(msgMacbuf->preOutId,syscode);
        strcpy(msgMacbuf->orgId,syscode);
        return 0;
}
/*******************************************************************
 * 函数名        :swRevKeyAPPLY
 * 函数功能      :取得申请后的秘钥值
 * 入口参数      :recmac
 *
 *
 * 出口参数      :
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl
 * 初作时间      : 2015-04-08
 * 修改纪录      :
 ********************************************************************/
int swRevKeyAPPLY(struct msgMacPack msgMacPack,char *returnmac)
{
        memcpy(returnmac,msgMacPack.workKey,strlen(msgMacPack.workKey));
        return 0;
}
/*******************************************************************
 * 函数名        : swStructInitMacByKey
 * 函数功能      : 外部输入密钥生成MAC结构体初始化
 * 入口参数      :          
 * 出口参数      :  
 * 返回码          : 成功 ：0
 *                       : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : 
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
int swStructInitMacByKey(struct msgMacPack *msgMacbuf,char *syscode,char *encKey,char *userData)
{
        msgMacbuf->tran_type=5;
	swVdebug(3,"11111111");
	swVdebug(3,"syscode_test=%s",syscode);
	swVdebug(3,"syscode_length=%d",strlen(syscode));
        strcpy(msgMacbuf->preOutId,syscode);
	swVdebug(3,"22222222");
        strcpy(msgMacbuf->encKey,encKey);
	swVdebug(3,"33333333");
        strcpy(msgMacbuf->userData,userData);
	swVdebug(3,"44444444");
        return 0;
}
/*******************************************************************
 * 函数名        :swRevMacByKey
 * 函数功能      :取得秘钥值
 * 入口参数      :recmac
 *  
 *
 * 出口参数      :
 * 返回码          : 成功 ：0
 *                       : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl
 * 初作时间      : 2015-04-08
 * 修改纪录      :
 ********************************************************************/
int swRevMacByKey(struct msgMacPack msgMacPack,char *mac)
{
        memcpy(mac,msgMacPack.macData,strlen(msgMacPack.macData));
    return 0;
}
/*******************************************************************
 * 函数名        : HsmGenMacByKey
 * 函数功能      : 用于外部输入密钥生成MAC
 * 入口参数      : syscode 系统代码 
 *                          encKey  ＭＡＣ密钥的密文
 *                           userData  macbuf
 *                           
 * 出口参数      : return macData：返回的macData
 * 返回码        : 成功 ：0
 *               : 失败 ：<0
 * 调用自定义函数:
 * 处理概要      :
 * 引用全局变量  :
 * 修改的全局变量:
 * 编程者        : tbl    
 * 初作时间      : 2015-04-08
 * 修改纪录      :       
 ********************************************************************/
int HsmGenMacByKey(char *syscode,char *encKey ,char *userData,char *macData)
{
	int ilsocket;
     struct msgMacPack msgMacbuf;
	
	swVdebug(3,"swStructInitMacByKey初始化开始");
        /*结构体初始化*/
        swStructInitMacByKey(&msgMacbuf,syscode,encKey,userData);
        swVdebug(3,"swStructInitMacByKey初始化结束");
        /*连接加密sever端*/
        swVdebug(3,"swConnectKey 开始");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey 结束");
        /*开始申请*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*接收结果*/
        swRevMacByKey(msgMacbuf,macData);
    return 0;

}


/****************************************************************/
/* 函数编号    ：PtTcpSndHead_sec*/ 
/* 函数名称    ：发送报文到通讯端口(加送指定位报文头)              */
/* 作	 者    ：PC                                       */
/* 建立日期    ：2001/10/26                                     */
/* 最后修改日期：2001/10/26                                     */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功:  0                                       */
/*               失败: -1                                       */
/****************************************************************/

int PtTcpSndHead_sec(int headlen,int sockfd, char *buffer, int length)
{
	int  len,rc;
	int  totalcnt;
	int  iRealLen;
	char aBuf[MSGMAXLEN];
	char format[16];
	fd_set wset;
	struct timeval timeout;
	int iTmp;
	
	if ( length<=0 )
		return(-2);
	
	if ( sockfd<=0 ){
		swVdebug(0,"传入的sockfd 非法!");
		return -2;
	}
	memset(aBuf,0x00,sizeof(aBuf));
	if(headlen>0)
	{
		sprintf(format,"%%0%dd",headlen);   		
		sprintf(aBuf,format,length);
		memcpy(aBuf+headlen,buffer,length);
		iRealLen=length+headlen;   	 			
	}
	else{
   		iRealLen=length;
   		memcpy(aBuf,buffer,length);
	}   
	totalcnt = 0;  
	FD_ZERO(&wset);
	FD_SET(sockfd,&wset);
	timeout.tv_sec=30;
	timeout.tv_usec=0;
	rc = select( sockfd+1, NULL, &wset, NULL, &timeout );
	if ( rc==1 && FD_ISSET(sockfd,&wset) ){
      while(totalcnt < iRealLen){
         len = write(sockfd, &aBuf[totalcnt], iRealLen - totalcnt);
         if (len < 0)
         {
            if (errno==EINTR)
            {
               len = 0;
               break;
            }
            else
            {
               return (-1);
            }
         }
         else if (len == 0)
            break;
         totalcnt = totalcnt + len;
      }
      if(totalcnt == iRealLen)
         return(0);
   }
   else if ( rc==0 ){
      swVdebug(0,"select timeout!");
   }
   else
   		swVdebug(0,"select error![%s] sockfd[%d]",strerror(errno),sockfd);
   return(-1);
}

/****************************************************************/
/* 函数编号    ：PtTcpRcvHeadTime_sec*/ 
/* 函数名称    ：接受报文（加超时控制和报文头）                 */
/* 作	 者    ：mopewolf                                       */
/* 建立日期    ：2001/10/26                                     */
/* 最后修改日期：2001/10/26                                     */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功:  0                                       */
/*               失败: -1                                       */
/* 修改记录：   修改超时的返回值为-3 2004-02-21 BY MOPEWOLF     */
/****************************************************************/
int PtTcpRcvHeadTime_sec( int headlen,int sock, char *buf, int length, int timeout )
{
   fd_set rset;
   struct timeval wishtime;
   int n=0;
   int rc=0;
   int aLen=0;
   int len=0;
   char aBuf[5];

   wishtime.tv_sec=timeout;
   wishtime.tv_usec=0;

   FD_ZERO(&rset);
   FD_SET(sock,&rset);
   rc=select(sock+1,&rset,NULL,NULL,&wishtime);
   if ( ( rc==1 ) && (FD_ISSET( sock,&rset )) )
   {
      memset(aBuf,0x00,sizeof(aBuf));
      if ( read(sock,aBuf,headlen)!=headlen ){
         swVdebug(0,"recv head error![%s]",aBuf);
         return(-1);
      }
      len=atoi(aBuf);
swVdebug(4,"len=[%d][%s]",len,aBuf);
			if (len==0)
					return -4;      
					
      if ( len>length )
         return(-2);
         
      aLen=0;
      while(aLen<len)
      {
         rc = read(sock, buf + aLen, len - aLen);
         if (rc < 0){
            if (errno == EINTR){
               rc = 0;
               continue;
            }
            else{
               return(-3);
            }
         }
         else if (rc == 0)
            break;

         aLen += rc;
      }
      if ( aLen != len )
         return(-5);
      return(aLen);
   }
   else if ( rc < 0 ){
      swVdebug(0,"PtTcpRcvHeadTime select error [%s] sockfd[%d]",strerror(errno),sock);
      return(-6);
   }
   else{
      swVdebug(0,"timeout [%s]",strerror(errno));
      return(-7);
   }
   return(-8);
}

/****************************************************************/
/* 函数编号    ：PtConnectser_sec*/ 
/* 函数名称    ：连接通讯SERVER端                               */
/* 作	 者    ：杨金舟                                         */
/* 建立日期    ：2000/5/15                                      */
/* 最后修改日期：2000/5/15                                      */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/

int PtConnectser_sec(char *alIp,int ilPort)
{
   int sockfd;
   struct sockaddr_in serv_addr;
   struct linger Linger;

   if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
   {
      swVdebug(0, "Creat Socket Error[%d]", errno);
      return (-1);
   }


   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = inet_addr( alIp );
   serv_addr.sin_port = htons( ilPort );
   if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
   {
      swVdebug(0, "Connect Server [%s:%d] Error[%d]",alIp,ilPort,errno);
      close(sockfd);
      return (-1);
   }
   /* set linger */
	/***
   Linger.l_onoff = 1;
   Linger.l_linger = 0;
   if (setsockopt(sockfd,SOL_SOCKET,SO_LINGER, \
		(char *)&Linger,sizeof(Linger)) != 0)
    swVdebug(0, "setsockopt() set linger Error[%d]", errno);
   ***/
   return sockfd;
}
