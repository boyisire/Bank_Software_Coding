/*ͷ�ļ�����*/

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


/*����ԭ��*/
int swSwcheckcontpack(char *aBuf);
int swComcheckcontpack(char *aBuf,unsigned int iMsglen);
int swComaddblankmsghead(char *aBuf,unsigned int*piMsglen);
int swReadcomcfg(char paFldvalue[][iFLDVALUELEN]);
int swSiginit();

#define iTRYFREQ  10

struct comcfg
{
  char  aPort_name[9];      /*�˿�����              */
  char  aMode[31];          /*��Ӧִ��ģ����        */
  short iMb_comm_id;        /*ͨѶ����              */
  short iMb_fore_id;        /*ǰ������              */
  short iTrytimes;          /*�ط�����              */
  short iTryfreq;           /*�ط�Ƶ��              */
  short	iTime_out;          /*��ʱʱ��              */
  short iDebug;             /*DEBUG��־             */
  short iMsghead;           /*�б���ͷ��־          */
  char  aEnd_string[5];     /*������                */
  char  a1[5];
  short iPre_msglen;        /*Ԥ������              */
  char  aExp_len[101];      /*���ȱ��ʽ            */
  char  aExp_cont[101];     /*ͨѶ���������ʽ      */   
  char  a2[4];
  short iPrep_cont;         /* ǰ�ú�������־ 1-�ж�aMemo 0-���ж� */ 
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
