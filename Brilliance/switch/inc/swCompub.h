/*头文件定义*/

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
/*#include <sys/conf.h>*/
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <setjmp.h>
#include <errno.h>


/*函数原型*/
int swSwcheckcontpack(char *aBuf);
int swComcheckcontpack(char *aBuf,unsigned int iMsglen);
int swComaddblankmsghead(char *aBuf,unsigned int*piMsglen);
int swReadcomcfg(char paFldvalue[][iFLDVALUELEN]);
int swSiginit();

#define iTRYFREQ  10

struct comcfg
{
  char  aPort_name[9];      /*端口名称              */
  char  aMode[31];          /*相应执行模块名        */
  short iMb_comm_id;        /*通讯邮箱              */
  short iMb_fore_id;        /*前置邮箱              */
  short iTrytimes;          /*重发次数              */
  short iTryfreq;           /*重发频率              */
  short	iTime_out;          /*超时时间              */
  short iDebug;             /*DEBUG标志             */
  short iMsghead;           /*有报文头标志          */
  char  aEnd_string[5];     /*结束符                */
  char  a1[5];
  short iPre_msglen;        /*预读长度              */
  char  aExp_len[101];      /*长度表达式            */
  char  aExp_cont[101];     /*通讯后续包表达式      */   
  char  a2[4];
  short iPrep_cont;         /* 前置后续包标志 1-判断aMemo 0-不判断 */ 
  char  a3[6];
} sgComcfg;
  
/*
struct
{
	char	aPort_name[9];
        char    aMode[30];
	short	iMb_comm_id;
	short	iMb_fore_id;
	char	aPartner_addr[21];
	short	iPartner_port;
        char    aEnd_flag[5];
        short   iPre_msglen;
        char    aExpress[30];

        short   iTrytimes;
        short   iTryfreq;
	short	iTime_out;
        short   iDebug;
        short   iMsghead;
        char    aHexpress[101];
        short   iHflag;       

} sgPortcfg;
*/

/* PORTCFG  sgPortcfg;*/

char cgDebug;

char *agMsgbody;
//short igMsgbodylen;
long igMsgbodylen; /*modified by baiqj20150422 PSBC_V1.0*/

#define BMQ__TIMEOUT    1006
