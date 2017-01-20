#include <stdio.h>
#include<stdlib.h>
#include<time.h>
#include <errno.h>
#include "PlSec.h"
#include<string.h>
#include <sys/socket.h>
#include <netinet/in.h>

/*******************************************************************
 * ������        : HsmMACGen_New
 * ��������      : ���ڽ��׵�MAC����
 * ��ڲ���      : syscode ϵͳ���� 4λ 01 ���� ����λΪʡ����
 *                 macblock���μӼ���mac��������
 *                 blocklen���μӼ���mac��������ĳ���
 * ���ڲ���      : returnmac�����ص�mac
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl    
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
 ********************************************************************/
int HsmMACGen_New(char *syscode, char *macblock, int blocklen,char *returnmac)
{
        int ilsocket;
        int flag;
	int iret;
        struct msgMacPack msgMacbuf;
	swVdebug(3,"agDebugfile=%s",agDebugfile);
        swVdebug(3,"swStructInit ��ʼ����ʼ");
        /*�ṹ���ʼ��*/
        swStructInit(flag,&msgMacbuf,macblock,blocklen,syscode);
        swVdebug(3,"swStructInit ��ʼ������");
        /*���Ӽ���sever��*/
        swVdebug(3,"swConnectKey ��ʼ");
	swVdebug(3,"msgMacbuf.encKey=%s",msgMacbuf.encKey);
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey ����");
        /*��ʼ����*/
        iret=swbegindKey(ilsocket,&msgMacbuf);
	if(iret){
		swVdebug(3,"swbegindKey error iret=%d",iret);
		return -2;
		}
        /*���ս��*/
        swVdebug(3,"msg Mac[%s]len[%d]",msgMacbuf.macData,strlen(msgMacbuf.macData));
        swRevKey(msgMacbuf,returnmac);
        if(strlen(msgMacbuf.macData)==0) {
		swVdebug(1,"��ȡMACʧ��");
                return -5;
	}
	swVdebug(3,"strlen(returnmac)=%d",strlen(returnmac));
		
    return 0;
}


/*******************************************************************
 * ������        : swStructInit
 * ��������      : �Զ�����Ĭ�ϲ���
 * ��ڲ���      : 
 *                
 *                 
 * ���ڲ���      : flag    
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl    
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
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
 * ������        :swConnectKey 
 * ��������      :�����ܹ�ƽ̨sever��
 * ��ڲ���      :ip  port 
 * ���ڲ���      : 
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
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
                swVdebug(3,"ERROR:���ӵ������[%s]�˿�=[%d]����");
                close(ilSocket);      
                return -2;
        }       

        return ilSocket;
}

/*******************************************************************
 * ������        :swbegindKey
 * ��������      :����MAC
 * ��ڲ���      :ip  port
 *
 *
 * ���ڲ���      :
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
 ********************************************************************/
int swbegindKey(int ilSocket,struct msgMacPack *sMsgPack)
{
        int ilRn;
        int igHeadlen=4;
        int ilMsglen;

        /*���ͽ�������*/
        swVdebug(3,"ilSocket=%d",ilSocket);
        swVdebug(3,"sMsgPack->algoType=%d",sMsgPack->algoType);
        swVdebug(3,"sizeof(struct msgMacPack)=%d",sizeof(struct msgMacPack));

        ilRn = PtTcpSndHead_sec(igHeadlen, ilSocket,sMsgPack, sizeof(struct msgMacPack));

        if ( ilRn==0 )
        {/*������ȷ����*/
                swVdebug(1,"send to Sock[%d] success",ilSocket);
        }       
        else    
        {       
                close(ilSocket);
                swVdebug(3,"ERROR:PbTcpSndHeadʧ�� error,ilRc= [%d]", ilRn);
                return -3;
        }       
        /*���ս���Ӧ��*/
        ilMsglen = PtTcpRcvHeadTime_sec(igHeadlen,ilSocket,sMsgPack,sizeof(struct msgMacPack), 3);
        if ( ilMsglen<0 ){
                close(ilSocket);
                swVdebug(3,"ERROR:PbTcpRcvHeadʧ�� error[%d],ilRc= [%d],strerror=[%s]",errno,ilMsglen,strerror(errno));
                return -4;
        }       
        swVdebug(3, "PbTcpRcvHead�յ����ģ�����Ϊ[%d]", ilMsglen);
        close(ilSocket);
        return 0;

}
/*******************************************************************
 * ������        :swRevKey
 * ��������      :ȡ��MACֵ
 * ��ڲ���      :recmac
 *
 *
 * ���ڲ���      :
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
 ********************************************************************/
int swRevKey(struct msgMacPack msgMacPack,char *returnmac)
{
        memcpy(returnmac,msgMacPack.macData,strlen(msgMacPack.macData));
        return 0;
}

/******************************************************************* 
 * ������        : HsmPINConvert_New
 * ��������      : ʵ��ת����
 * ��ڲ���      : syscode ��̨ϵͳ���� 4λ 01 ���� ����λΪʡ����
 *                 pan���ʺ�
 *                 pin_in��������������ģ������ʺţ�
 *                         
 * ���ڲ���      : pin_out����̨ҵ��ϵͳ����
 * ������        : �ɹ� ��0 
 *               : ʧ�� ��<0
 * �����Զ��庯��: 
 * �����Ҫ      :       
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : pc    
 * ����ʱ��      : 2008-3-11 20:08
 * �޸ļ�¼      :       
********************************************************************/
int HsmPINConvert_New(char *syscode, char *pan,char *pin_in,char *pin_out,char *syscodeb,char *aflag)
{
        int ilsocket;
        struct msgMacPack msgMacbuf;
        swVdebug(3,"pan=%s,pin_in=%s",pan,pin_in);
        swVdebug(3,"agDebugfile=%s",agDebugfile);
        swVdebug(3,"swStructInitPIN ��ʼ����ʼ");
        /*�ṹ���ʼ��*/
        swStructInitPIN(aflag,&msgMacbuf,pan,pin_in,syscode,syscodeb);
        swVdebug(3,"swStructInitPIN ��ʼ������");
        /*���Ӽ���sever��*/
        swVdebug(3,"swConnectKey ��ʼ");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey ����");
        /*��ʼ����*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*���ս��*/
        swRevKeyPIN(msgMacbuf,pin_out);
    return 0;

}
/*******************************************************************
 * ������        : swStructInitPIN
 * ��������      : �Զ�����Ĭ�ϲ��� (ʵ��ת����)
 * ��ڲ���      : 
 *                
 *                 
 * ���ڲ���      : flag    
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl    
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
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
 * ������        :swRevKeyPIN
 * ��������      :ȡ��MACֵ
 * ��ڲ���      :recmac
 *
 *
 * ���ڲ���      :
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
 ********************************************************************/
int swRevKeyPIN(struct msgMacPack msgMacPack,char *pinout)
{
        memcpy(pinout,msgMacPack.destPIN,strlen(msgMacPack.destPIN));
        return 0;
}


/**************************************************************
 ** �� �� ����HsmPINtoMD5_New
 ** ��    �ܣ�DES����ת����ΪMD5ɢ�к���
 ** ��    �ߣ�YZ  
 ** �������ڣ�2011/6/13
 ** �޸����ڣ�
 ** ���ú�����
 ** ȫ�ֱ�����
 ** �������壺 aParm[0] ��̨ϵͳ���� 
 **            aParm[1] �ۺ�/����-�����������
 **            aParm[2] ����PINBLOCK  
 **            aParm[3] MD5��SALT
 ** �� �� ֵ�� 0 �ɹ� aResult ���ܺ��PIN����,�˺Ųμ�����
***************************************************************/
int HsmPINtoMD5_New(char *syscode, char *pan,char *pin_in,char *salt,char *pin_md5)
{
        int ilsocket;
        int flag;
        struct msgMacPack msgMacbuf;
        

        swVdebug(3,"swStructInitMD5 ��ʼ����ʼ");
        /*�ṹ���ʼ��*/
      //  swStructInitMD5(flag,&msgMacbuf,pan,pin_in,salt,syscode);
       /*add by zd */
        swStructInitMD5(&msgMacbuf,pan,pin_in,salt,syscode);
        swVdebug(3,"swStructInitMD5 ��ʼ������");
        /*���Ӽ���sever��*/
        swVdebug(3,"swConnectKey ��ʼ");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey ����");
        /*��ʼ����*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*���ս��*/
        swRevKeyMD5(msgMacbuf,pin_md5);
    return 0;
}
/*******************************************************************
 * ������        : swStructInitMD5
 * ��������      : �Զ�����Ĭ�ϲ���
 * ��ڲ���      : 
 *                
 *                 
 * ���ڲ���      : flag    
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl    
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
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
 * ������        :swRevKeyMD5
 * ��������      :ȡ��MACֵ
 * ��ڲ���      :recmac
 *
 *
 * ���ڲ���      :
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
 ********************************************************************/
int swRevKeyMD5(struct msgMacPack msgMacPack,char *pin_md5)
{
        memcpy(pin_md5,msgMacPack.hashPIN,strlen(msgMacPack.hashPIN));
    return 0;
}


/*******************************************************************
 * ������        :GetProfileString_m
 * ��������      :ȡ�������ļ�������
 * ��ڲ���      :
 *
 *
 * ���ڲ���      :
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl   
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
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
 ** ������      : _swTrim 
 ** ��  ��      : �ú������ַ��� s ��ǰ��ո�β�س�ȥ��
 ** ��  ��      : llx   
 ** ��������    : 1999/11/4
 ** ����޸�����: 2000/3/10
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : s   :�ַ��� 
 ** ����ֵ      : SUCCESS 
          ת������ַ�����s����
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
 ** ������      : _swTrim 
 ** ��  ��      : �ú������ַ��� s ��ǰ��ո�β�س�ȥ��
 ** ��  ��      : llx   
 ** ��������    : 1999/11/4
 ** ����޸�����: 2000/3/10
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : s   :�ַ��� 
 ** ����ֵ      : SUCCESS 
          ת������ַ�����s����
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
            swVdebug(3,"ERROR:ȡ��PORT ʧ��[%s][%d]",aConfigFile,errno);
            return -1;
         }
	extconfig.iport=atoi(iport);
        if (GetProfileString_m(aConfigFile,"SOFT","HOSTIP",agip) < 0)
        {
            swVdebug(3,"ERROR:ȡ��HOSTIP ʧ��[%s][%d]",aConfigFile,errno);
            return -1;
         }
	strcpy(extconfig.aip,agip);	
        if (GetProfileString_m(aConfigFile,"SOFT","FRESHTIME",freshtime) < 0)
        {
            swVdebug(3,"ERROR:ȡ��FRESHTIME ʧ��[%s][%d]",aConfigFile,errno);
            return -1;
         }
	extconfig.freshtime=atoi(freshtime);
        return 0;
}
/*******************************************************************
 * ������        : HsmMAC_APPLY_New
 * ��������      : ����������Կ�Ľ���
 * ��ڲ���      : syscode ϵͳ���� 
 *                 worktype��������Կ����
 *                 blocklen���μӼ���mac��������ĳ���
 * ���ڲ���      : returnmac�����ص�mac
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl    
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
 ********************************************************************/
int HsmMAC_APPLY_New(char *syscode, char *worktype, int blocklen,char *returnmac)
{
        int ilsocket;
        int flag;
        struct msgMacPack msgMacbuf;
        swVdebug(3,"swStructInitAPPLY ��ʼ����ʼ");
        /*�ṹ���ʼ��*/
        swStructInitAPPLY(flag,&msgMacbuf,worktype,blocklen,syscode);
        swVdebug(3,"swStructInitAPPLY ��ʼ������");
        /*���Ӽ���sever��*/
        swVdebug(3,"swConnectKeyAPPLY ��ʼ");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKeyAPPLY error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKeyAPPLY ����");
        /*��ʼ����*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*���ս��*/
        swRevKeyAPPLY(msgMacbuf,returnmac);
    return 0;

}
/*******************************************************************
 * ������        : swStructInitAPPLY
 * ��������      : �Զ�����Ĭ�ϲ���(��Կ����)
 * ��ڲ���      : 
 *                
 *                 
 * ���ڲ���      : flag    
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl    
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
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
 * ������        :swRevKeyAPPLY
 * ��������      :ȡ����������Կֵ
 * ��ڲ���      :recmac
 *
 *
 * ���ڲ���      :
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
 ********************************************************************/
int swRevKeyAPPLY(struct msgMacPack msgMacPack,char *returnmac)
{
        memcpy(returnmac,msgMacPack.workKey,strlen(msgMacPack.workKey));
        return 0;
}
/*******************************************************************
 * ������        : swStructInitMacByKey
 * ��������      : �ⲿ������Կ����MAC�ṹ���ʼ��
 * ��ڲ���      :          
 * ���ڲ���      :  
 * ������          : �ɹ� ��0
 *                       : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : 
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
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
 * ������        :swRevMacByKey
 * ��������      :ȡ����Կֵ
 * ��ڲ���      :recmac
 *  
 *
 * ���ڲ���      :
 * ������          : �ɹ� ��0
 *                       : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :
 ********************************************************************/
int swRevMacByKey(struct msgMacPack msgMacPack,char *mac)
{
        memcpy(mac,msgMacPack.macData,strlen(msgMacPack.macData));
    return 0;
}
/*******************************************************************
 * ������        : HsmGenMacByKey
 * ��������      : �����ⲿ������Կ����MAC
 * ��ڲ���      : syscode ϵͳ���� 
 *                          encKey  �ͣ�����Կ������
 *                           userData  macbuf
 *                           
 * ���ڲ���      : return macData�����ص�macData
 * ������        : �ɹ� ��0
 *               : ʧ�� ��<0
 * �����Զ��庯��:
 * �����Ҫ      :
 * ����ȫ�ֱ���  :
 * �޸ĵ�ȫ�ֱ���:
 * �����        : tbl    
 * ����ʱ��      : 2015-04-08
 * �޸ļ�¼      :       
 ********************************************************************/
int HsmGenMacByKey(char *syscode,char *encKey ,char *userData,char *macData)
{
	int ilsocket;
     struct msgMacPack msgMacbuf;
	
	swVdebug(3,"swStructInitMacByKey��ʼ����ʼ");
        /*�ṹ���ʼ��*/
        swStructInitMacByKey(&msgMacbuf,syscode,encKey,userData);
        swVdebug(3,"swStructInitMacByKey��ʼ������");
        /*���Ӽ���sever��*/
        swVdebug(3,"swConnectKey ��ʼ");
        ilsocket=swConnectKey();
        if(ilsocket<0){
                swVdebug(3,"swConnectKey error =%d",ilsocket);
                return -1;
        }
        swVdebug(3,"swConnectKey ����");
        /*��ʼ����*/
        swbegindKey(ilsocket,&msgMacbuf);
        /*���ս��*/
        swRevMacByKey(msgMacbuf,macData);
    return 0;

}


/****************************************************************/
/* �������    ��PtTcpSndHead_sec*/ 
/* ��������    �����ͱ��ĵ�ͨѶ�˿�(����ָ��λ����ͷ)              */
/* ��	 ��    ��PC                                       */
/* ��������    ��2001/10/26                                     */
/* ����޸����ڣ�2001/10/26                                     */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�:  0                                       */
/*               ʧ��: -1                                       */
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
		swVdebug(0,"�����sockfd �Ƿ�!");
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
/* �������    ��PtTcpRcvHeadTime_sec*/ 
/* ��������    �����ܱ��ģ��ӳ�ʱ���ƺͱ���ͷ��                 */
/* ��	 ��    ��mopewolf                                       */
/* ��������    ��2001/10/26                                     */
/* ����޸����ڣ�2001/10/26                                     */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�:  0                                       */
/*               ʧ��: -1                                       */
/* �޸ļ�¼��   �޸ĳ�ʱ�ķ���ֵΪ-3 2004-02-21 BY MOPEWOLF     */
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
/* �������    ��PtConnectser_sec*/ 
/* ��������    ������ͨѶSERVER��                               */
/* ��	 ��    �������                                         */
/* ��������    ��2000/5/15                                      */
/* ����޸����ڣ�2000/5/15                                      */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
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
