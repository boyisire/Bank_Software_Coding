
/* Result Sets Interface */
#ifndef SQL_CRSR
#  define SQL_CRSR
  struct sql_cursor
  {
    unsigned int curocn;
    void *ptr1;
    void *ptr2;
    unsigned int magic;
  };
  typedef struct sql_cursor sql_cursor;
  typedef struct sql_cursor SQL_CURSOR;
#endif /* SQL_CRSR */

/* Thread Safety */
typedef void * sql_context;
typedef void * SQL_CONTEXT;

/* Object support */
struct sqltvn
{
  unsigned char *tvnvsn; 
  unsigned short tvnvsnl; 
  unsigned char *tvnnm;
  unsigned short tvnnml; 
  unsigned char *tvnsnm;
  unsigned short tvnsnml;
};
typedef struct sqltvn sqltvn;

struct sqladts
{
  unsigned int adtvsn; 
  unsigned short adtmode; 
  unsigned short adtnum;  
  sqltvn adttvn[1];       
};
typedef struct sqladts sqladts;

static struct sqladts sqladt = {
  1,1,0,
};

/* Binding to PL/SQL Records */
struct sqltdss
{
  unsigned int tdsvsn; 
  unsigned short tdsnum; 
  unsigned char *tdsval[1]; 
};
typedef struct sqltdss sqltdss;
static struct sqltdss sqltds =
{
  1,
  0,
};

/* File name & Package Name */
struct sqlcxp
{
  unsigned short fillen;
           char  filnam[12];
};
static struct sqlcxp sqlfpn =
{
    11,
    "swSysmon.pc"
};


static unsigned int sqlctx = 172251;


static struct sqlexd {
   unsigned long  sqlvsn;
   unsigned int   arrsiz;
   unsigned int   iters;
   unsigned int   offset;
   unsigned short selerr;
   unsigned short sqlety;
   unsigned int   occurs;
            short *cud;
   unsigned char  *sqlest;
            char  *stmt;
   sqladts *sqladtp;
   sqltdss *sqltdsp;
   unsigned char  **sqphsv;
   unsigned long  *sqphsl;
            int   *sqphss;
            short **sqpind;
            int   *sqpins;
   unsigned long  *sqparm;
   unsigned long  **sqparc;
   unsigned short  *sqpadto;
   unsigned short  *sqptdso;
   unsigned int   sqlcmax;
   unsigned int   sqlcmin;
   unsigned int   sqlcincr;
   unsigned int   sqlctimeout;
   unsigned int   sqlcnowait;
            int   sqfoff;
   unsigned int   sqcmod;
   unsigned int   sqfmod;
   unsigned char  *sqhstv[1];
   unsigned long  sqhstl[1];
            int   sqhsts[1];
            short *sqindv[1];
            int   sqinds[1];
   unsigned long  sqharm[1];
   unsigned long  *sqharc[1];
   unsigned short  sqadto[1];
   unsigned short  sqtdso[1];
} sqlstm = {12,1};

/* SQLLIB Prototypes */
extern sqlcxt ( void **, unsigned int *,
                   struct sqlexd *, struct sqlcxp * );
extern sqlcx2t( void **, unsigned int *,
                   struct sqlexd *, struct sqlcxp * );
extern sqlbuft( void **, char * );
extern sqlgs2t( void **, char * );
extern sqlorat( void **, unsigned int *, void * );

/* Forms Interface */
static int IAPSUCC = 0;
static int IAPFAIL = 1403;
static int IAPFTL  = 535;
extern void sqliem( unsigned char *, signed int * );

typedef struct { unsigned short len; unsigned char arr[1]; } VARCHAR;
typedef struct { unsigned short len; unsigned char arr[1]; } varchar;

/* CUD (Compilation Unit Data) Array */
static short sqlcud0[] =
{12,4130,852,0,0,
};


#line 1 "swSysmon.pc"
/****************************************************************/
/* ģ����    ��SYSMON                                         */ 
/* ģ������    ��ϵͳ���                                       */
/* �� �� ��    ��V4.3.0                                         */
/* ��    ��    ��                                               */
/* ��������    ��1999/11/22                                     */
/* ����޸����ڣ�2001/8/30                                      */
/* ģ����;    ���¼�������                                     */
/* ��ģ���а������º���������˵����                             */
/*                         ��1�� void main();                   */
/*                         ��2�� void swQuit();                 */
/*                         ��3�� int swLoadmsgpack();        	*/
/*                         ��4�� int swLoadttytype();           */
/*                         ��5�� int swMakebuffer();            */
/*                         ��8�� int swClschild();              */
/*                         ��9�� int swSendwin();               */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*   1999/12/21 �����˼�¼�¼���־                              */
/*   1999/12/22 �޸�����Ϣ�����ʽ�Ĵ���                        */
/*   2000/1/29  �������Ŷ��ڵ���ƽ����޸�                      */
/*   2000/8/24  �Ż�                                            */
/*   2001/1/22  Oracle��ֲ                                      */
/*   2001/3/24  ���V3.01,����968��                           */
/*   2001/4/3   DB2��ֲ(972)                                    */
/*   2001/4/6   INFORMIX��ֲ(974)                               */
/*   2001/06/29 �����ݿ����Ϊ���ñ�����                        */
/*   2001/7/30  ���ݽ��ս���ʵ�ֶ����                        */
/*   2001/8/2   ��Ϊ��������(1371)                              */
/****************************************************************/

/* ͷ�ļ����� */
#include  "switch.h"
#include  <curses.h> 
#include  "swNdbstruct.h"
#include  "swShm.h"
/* EXEC SQL INCLUDE sqlca;
 */ 
#line 1 "/home/oracle/app/oracle/product/11.2.0/dbhome_1/precomp/public/sqlca.h"
/*
 * $Header: sqlca.h 24-apr-2003.12:50:58 mkandarp Exp $ sqlca.h 
 */

/* Copyright (c) 1985, 2003, Oracle Corporation.  All rights reserved.  */
 
/*
NAME
  SQLCA : SQL Communications Area.
FUNCTION
  Contains no code. Oracle fills in the SQLCA with status info
  during the execution of a SQL stmt.
NOTES
  **************************************************************
  ***                                                        ***
  *** This file is SOSD.  Porters must change the data types ***
  *** appropriately on their platform.  See notes/pcport.doc ***
  *** for more information.                                  ***
  ***                                                        ***
  **************************************************************

  If the symbol SQLCA_STORAGE_CLASS is defined, then the SQLCA
  will be defined to have this storage class. For example:
 
    #define SQLCA_STORAGE_CLASS extern
 
  will define the SQLCA as an extern.
 
  If the symbol SQLCA_INIT is defined, then the SQLCA will be
  statically initialized. Although this is not necessary in order
  to use the SQLCA, it is a good pgming practice not to have
  unitialized variables. However, some C compilers/OS's don't
  allow automatic variables to be init'd in this manner. Therefore,
  if you are INCLUDE'ing the SQLCA in a place where it would be
  an automatic AND your C compiler/OS doesn't allow this style
  of initialization, then SQLCA_INIT should be left undefined --
  all others can define SQLCA_INIT if they wish.

  If the symbol SQLCA_NONE is defined, then the SQLCA variable will
  not be defined at all.  The symbol SQLCA_NONE should not be defined
  in source modules that have embedded SQL.  However, source modules
  that have no embedded SQL, but need to manipulate a sqlca struct
  passed in as a parameter, can set the SQLCA_NONE symbol to avoid
  creation of an extraneous sqlca variable.
 
MODIFIED
    lvbcheng   07/31/98 -  long to int
    jbasu      12/12/94 -  Bug 217878: note this is an SOSD file
    losborne   08/11/92 -  No sqlca var if SQLCA_NONE macro set 
  Clare      12/06/84 - Ch SQLCA to not be an extern.
  Clare      10/21/85 - Add initialization.
  Bradbury   01/05/86 - Only initialize when SQLCA_INIT set
  Clare      06/12/86 - Add SQLCA_STORAGE_CLASS option.
*/
 
#ifndef SQLCA
#define SQLCA 1
 
struct   sqlca
         {
         /* ub1 */ char    sqlcaid[8];
         /* b4  */ int     sqlabc;
         /* b4  */ int     sqlcode;
         struct
           {
           /* ub2 */ unsigned short sqlerrml;
           /* ub1 */ char           sqlerrmc[70];
           } sqlerrm;
         /* ub1 */ char    sqlerrp[8];
         /* b4  */ int     sqlerrd[6];
         /* ub1 */ char    sqlwarn[8];
         /* ub1 */ char    sqlext[8];
         };

#ifndef SQLCA_NONE 
#ifdef   SQLCA_STORAGE_CLASS
SQLCA_STORAGE_CLASS struct sqlca sqlca
#else
         struct sqlca sqlca
#endif
 
#ifdef  SQLCA_INIT
         = {
         {'S', 'Q', 'L', 'C', 'A', ' ', ' ', ' '},
         sizeof(struct sqlca),
         0,
         { 0, {0}},
         {'N', 'O', 'T', ' ', 'S', 'E', 'T', ' '},
         {0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 0, 0}
         }
#endif
         ;
#endif
 
#endif
 
/* end SQLCA */
/* �������� */
#define FILLER       ";"              /* �ָ��� */
#define FILLERLEN    1                /* �ָ������� */
#define SENDMAX      100              /* ����ǰ̨������ */

/* �������� */
static short igCount_msgtype;               /* �������ͼ����� */
static short igCount_msgfile;               /* �������ͼ����� */
static int   igSock;

/* �ṹ���� */
struct swt_sys_msgpack *wsgSwt_sys_msgpack,*psgSwt_sys_msgpack; 
struct swt_sys_ttytype *psgSwt_sys_ttytype;

/* ����WINDOWSǰ̨�ṹ */
struct swt_sys_win
{
  int   usrflag;
  char  address[16];
  int   port;
  short Msg_id;
  long  iNum_id;         /*���ķ��ʹ���������*/
  long  iEcho_id;        /*��Ӧ���Ĵ���������*/
}psgSendwin[SENDMAX];

/* �˿����ýṹ */
struct swt_sys_port
{
  char  address[16];
  long  port;
}sgSwt_sys_port;

FILE **fgPack;                /* �����Ϣ��¼�ļ� */
FILE *fgWin;                  /* ǰ̨ǩ����Ϣ��¼�ļ� */

/* ����ԭ��˵�� */
void swQuit( int sig );
void swClschild( );
int  swLoadsysttytype(short *iTerm_num );
int  swLoadmsgpack( );
int  swMakebuffer(short iMsg_type,char * aBuffer);
int  swLoadmsgpack1( short cMsgtype,int *iCount);
int  swSendwin(int iSock,char *aAddr,short iPort,char *aMsgpack,short iMsglen);
int  swGetField(char *aBuffer,int n,char *aField,char Seperator);
extern int _swExpN2T(char *aNstr,char *aTstr);
int swFwrite(char *aBuf,short iType);
int swFwinr();
int swFwinw();

/**************************************************************
 ** ������      �� main
 ** ��  ��      �� ������
 ** ��  ��      �� nh
 ** ��������    �� 1999/11/22
 ** ����޸����ڣ� 2001/9/24
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/

int main(int argc,char *argv[])
{
  struct msgpack slMsgpack;                /* ���� */
  struct msghead *pslMsghead;              /* ����ͷ */
  short ilMsglen;                          /* ���ĳ��� */
  short ilCount_tty_type;                  /* �ն˸��������� */
  short i,j;                             /* ѭ������ */
  short ilRc;                              /* ������ */
  short ilSendFlag;                        /* ����ǰ̨�ı�־ */
  char  alSendBuffer[1024];                /* ����ǰ̨���� */
  char  alResult[iFLDVALUELEN];            /* ����Ľ�� */
  long  llCode;                            /* ��Ϣ��������� */
  short ilResultlen;
  char  *alMsgbody;                        /* ָ������ָ�� */
  char  alMsg_id[4],alIpadd[16],alPort[5],alPort1[5]; /* �ӱ�������ȡ�������� */
  short ilWinflag;
  int   ilSize;
  char  alAddr[16];                     
  int   ilPort;
  long  ilMsg_num;               /* ���ͱ��ĵ������� */
  long  ilEcho_num;              /* ���յ�����Ӧ��������� */
  short ilFileflag=0;              /* ���ļ����ļ��ı�־ 1:���ļ�*/
  char  alFiletmp[1024];          /* �Ʊ��ĵ��ļ��� */
  
  /* �����ź� */
  signal(SIGCLD,SIG_IGN);
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGTTOU,SIG_IGN);

  /* ȡ�ð汾�� */
  if(argc >= 3)
    _swVersion("swSysmon Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swSysmon")) exit(FAIL);

  /* ���õ��Գ������� */
  strcpy(agDebugfile,"swSysmon.debug");

  /* ȡ��DEBUG��־ */
  if((cgDebug = _swDebugflag("swSysmon")) == FAIL)
  {
    fprintf(stderr,"ȡDEBUG��־ʧ��!\n");
    exit(FAIL);
  } 
  swVdebug(2,"S0010: ȡ��debug���.cgDebug = %d",cgDebug);

  if(argc > 1)
  {
    if(!strcmp(argv[1],"1"))
      ilFileflag = 1;
  }

  /*add by gxz 2003.1.7*/  
  /* �����ݿ� */
  if(swDbopen() != SUCCESS)
  {
    swVdebug(0,"S0040: [����/���ݿ�] swDbopen()�����ݿ����");
    exit(FAIL);
  }
  swVdebug(2,"S0050: �����ݿ�ɹ�");
  
  ilRc = swShmcheck(); /* ��ʼ�������ڴ�ָ�� */
  if (ilRc != SUCCESS)
  {
    printf("û�����������ڴ�,��ͨ��swInit����...\n");
    swVdebug(0,"S0020: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    exit(FAIL);
  }
  swVdebug(2,"S0030: ��ʼ�������ڴ�ɹ�");
  
  /* Ԥ���ն����ñ� */
  swVdebug(2,"S0040: ��ʼ���ն����ñ�");

  ilRc = swLoadsysttytype(&ilCount_tty_type);
  if (ilRc)
  {
    swVdebug(0,"S0050: [����/����] Ԥ���ն����ñ�����");
    exit(-1);
  }
  
  igCount_msgfile = ilCount_tty_type;
  /* Ԥ��������֯ */
  ilRc = swLoadmsgpack();
  if (ilRc)
  {
    swVdebug(1,"S0060: [����/����] Ԥ��������֯����,������Ϊ:%d", ilRc);
    exit(FAIL);
  }
  if(ilFileflag == 1)
  {
    fgPack=(FILE **)malloc(igCount_msgfile * sizeof(FILE *));
    swVdebug(3,"S0070: д���ļ�����file count=%d",igCount_msgfile);
    for(i = 0;i < igCount_msgfile;i ++)
    {
      memset(alFiletmp,0x00,sizeof(alFiletmp));
      sprintf(alFiletmp,"%s/monlog/%s_%d.log",getenv("SWITCH_MON_PATH"),
        psgSwt_sys_ttytype[i].discrib,psgSwt_sys_ttytype[i].Term_id);
      if((fgPack[i]=fopen(alFiletmp,"a+")) == NULL)
      {
        swVdebug(1,"S0080: [����/ϵͳ����] fopen()����,errno=%d[%s],�ļ�[%s]�򿪳���",errno,strerror(errno),alFiletmp);
        fclose(fgPack[i]);
        exit(-1);
      }
      swVdebug(2,"S0090: ���ļ�[%s]�ɹ�",alFiletmp);
    }
  }
  
  /*�ӹ����ڴ��ж�ȡ����˿�����*/
  ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
  if (ilRc)
  {
    swVdebug(1,"S0070 [����/�����ڴ�] ��ȡ�����ڴ��ж˿ڳ���");
    exit(FAIL);
  }
  
  swVdebug(2,"S0100: ��ȡϵͳ������Ϣ�ɹ�"); 
  /* ��������ʼ�� */
  strcpy(sgSwt_sys_port.address,sgSwt_sys_config.sysmonaddr);
  sgSwt_sys_port.port = sgSwt_sys_config.sysmonport;

  ilMsg_num = sgSwt_sys_config.msg_num;
  ilEcho_num = sgSwt_sys_config.echotest_num;

  /* ��ʼ��ǰ̨��־�����ķ��ʹ��� */
  for(i=0;i<SENDMAX;i++)
  {
    psgSendwin[i].usrflag = 0;
    psgSendwin[i].iNum_id = 0;
    psgSendwin[i].iEcho_id = 0;
  }
  ilRc=swFwinr();
  if(ilRc)
  {
    swVdebug(1,"S0110: swFwinr() error");
    exit(-1);
  }

  /* ����UDP */
swVdebug(1,"port=%d",sgSwt_sys_port.port);
  igSock = _swUDPCreate(sgSwt_sys_port.port);
  if(igSock < 0)
  {
    swVdebug(1,"S0120: ��������_swUDPCreate(),������=%d,����socket����[%d][%s]",igSock,errno,strerror(errno));
    swVdebug(2,"S0130: �˿ں�[%d],����socket����[%d][%s]",sgSwt_sys_port.port,errno,strerror(errno));
    exit(-1);
  }
  swVdebug(2,"S0140: ����socket�ɹ�,sock = %d",igSock);
  
  signal(SIGTERM,swClschild); 

  swVdebug(2,"S0150: ���ݳ�ʼ��");
  /* ���ݳ�ʼ�� */
  ilSendFlag = 1;
  pslMsghead = (struct msghead *)&slMsgpack;
  
  /*swVdebug(1,"S0160: ѭ�����ղ�������ر���...");*/
  
  while(TRUE) 
  {       
    ilMsglen = iMSGMAXLEN;  
    ilSize=sizeof(struct sockaddr_in);
    memset(alAddr,0x00,sizeof(alAddr));                     
    ilPort=0;

    swVdebug(2,"S0170: ��ʼ��ȡ��Ϣ");
  
    /* ͨ��UDP��ȡ���� */
    ilRc=_swUDPGet(igSock,(char *)alAddr,(int *)&ilPort,(char *)&slMsgpack,\
         &ilMsglen);
    swVdebug(2,"S0180: alAddr=%s ilPort=%d",alAddr,ilPort);
    if (ilRc!=0)
    {
      swVdebug(1,"S0190: [����/��������] _swUDPGet()����,������=%d,��ȡ��Ϣʧ��",ilRc);
      swMberror(ilRc,(char *)&slMsgpack);
      swQuit(igSock);
    }

    if (cgDebug >= 2)
      swDebughex((char *)&slMsgpack,
        sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen);
    swVdebug(2,"S0200: ��ȡ��Ϣ�ɹ�");
   
    psgMsghead = &(slMsgpack.sMsghead); 
    llCode = pslMsghead->lCode;
    swVdebug(2,"S0210: switch(pslMsghead->iMsgtype=%d)",pslMsghead->iMsgtype);
    
    switch(pslMsghead->iMsgtype)
    {
      case iMSGMESSAGE:
        swVdebug(2,"S0220: case ��Ϣ����");
        if(ilSendFlag == 1)
        {
          swVdebug(2,"S0230: if(����ǰ̨�ı�־==1)");
          
          if(slMsgpack.sMsghead.iBodylen >= 0)
          {
            ilRc = swFmlunpack(slMsgpack.aMsgbody,
              slMsgpack.sMsghead.iBodylen,psgPreunpackbuf);
            if(ilRc != SUCCESS)
            {
              swVdebug(1,"S0240: [����/��������] swFmlunpack()����,������=%d",ilRc);
              continue;
            }
            swVdebug(2,"S0250: swFmlunpack success");
          }
          else continue;
          
          memset(alSendBuffer,0x00,sizeof(alSendBuffer));
          memset(alResult,0x0,sizeof(alResult)); 
          
          for ( i=0;i< ilCount_tty_type;i++)
          {
            /* �÷���ǰ̨�ı�־ */
            ilWinflag = 0;
            for( j=0;j<SENDMAX; j++)
            { 
              if((psgSendwin[j].usrflag == 1)&&(psgSendwin[j].Msg_id == 
                 psgSwt_sys_ttytype[i].Term_id))
              {
                ilWinflag = 1;
                break;
              }
            }
            /* ��֯���Ĳ��ҷ��͸�ǰ̨ */
            if(ilWinflag==1)
            { 
              ilRc = _swExpress(psgSwt_sys_ttytype[i].lexpress,\
                alResult,&ilResultlen);  
              if(ilRc != SUCCESS)
              {
                swVdebug(2,"S0260: �����ն���ƽ̨����ʽ����");
                continue;
              }
              swVdebug(2,"S0270: ����ƽ̨����ʽ�ɹ�,result=[%s]",alResult);
              
              if (alResult[0] == '1') 
              {
                swVdebug(2,"S0280: ƽ̨����ʽ������Ϊ��");
              
                ilRc=swMakebuffer(psgSwt_sys_ttytype[i].Term_id,alSendBuffer);
                if(ilRc)
                {
                  swVdebug(1,"S0290: [����/��������] swMakebuffer()����,������=%d,��֯���ĳ���",ilRc);
                  swQuit(igSock);
                }
                if (strlen(alSendBuffer)==0)
                {
                  swVdebug(1,"S0300: [����/����] Ҫ��ʾ��������Ϊ��");
                  continue;
                } 
                if(ilFileflag == 1)
                {
                 swVdebug(2,"S0310: д���ļ���type=[%d,%d],len=[%d],buf=[%s]",i,
                    psgSwt_sys_ttytype[i].Term_id,strlen(alSendBuffer),
                    alSendBuffer);
                  ilRc = swFwrite(alSendBuffer,i); 
                  if(ilRc)
                    swVdebug(1,"S0320: swFwrite()����ˮ��Ϣд���ļ�ʧ��!");
                  swVdebug(3,"S0330: swFwrite()����ˮ��Ϣд���ļ��ɹ�!");
                }
                /* ������Ϣ���ĵ�ǰ̨���ģ�� */                
                for( j=0;j<SENDMAX; j++)
                {
                  if((psgSendwin[j].usrflag == 1)&&(psgSendwin[j].Msg_id ==
                     psgSwt_sys_ttytype[i].Term_id))
                  {
                    ilRc = swSendwin(igSock,psgSendwin[j].address,\
                      psgSendwin[j].port,alSendBuffer,strlen(alSendBuffer));
	            if(ilRc)
                      swVdebug(1,"S0340: [����/����] ����ǰ̨ʧ��");
                    else
                    {
                      swVdebug(2,"S0350: ����ǰ̨�ɹ�IP=[%s] PORT=[%d]",psgSendwin[j].address,psgSendwin[j].port);
                      swVdebug(3,"S0360: ����ǰ̨����ΪSendbuf=%s",alSendBuffer);
                    }
	            
	            if(psgSendwin[j].iNum_id < ilMsg_num)
	            {
	              psgSendwin[j].iNum_id ++;
	            }
	            else 
                    {
                      if (psgSendwin[j].iEcho_id < ilEcho_num)
	              {  
	                _swUDPPut(igSock,psgSendwin[j].address,psgSendwin[j].port,"alive",5);
                        swVdebug(2,"S0370: ����alive��IP=[%s] PORT=[%d]�ɹ�",psgSendwin[j].address,psgSendwin[j].port);
	                psgSendwin[j].iEcho_id++;
                      }
	              else
                      {
                        swVdebug(1,"S0380: [����!!!] IP=[%s] PORT=[%d] \
                          ��ǰ̨�쳣�ж�,����!",psgSendwin[j].address, \
                          psgSendwin[j].port);
	                psgSendwin[j].usrflag = 0;
                        psgSendwin[j].iNum_id = 0;
                        psgSendwin[j].iEcho_id = 0;
                        ilRc=swFwinw();
                        if(ilRc)
                          swVdebug(1,"S0390: swFwinw()����ǩ����Ϣʧ��!");
                        swVdebug(3,"S0400: swFwinw()����ǩ����Ϣ�ɹ�");
                      }
                    }
                  }
                } /* end for */
              }
            } /* end if */
          } /* end for */
        } /*end if(ilSendFlag==1)*/  
        
        swVdebug(2,"S0410: END case ��Ϣ����");
        break;
/*    ����      
      case iMSGORDER:

         * ���͵�SYSCOM *
        swVdebug(2,"S0420: case �����");
        
        ilRc = sendto(igSock,(char *)&slMsgpack,sizeof(struct msgpack),0,
          (struct sockaddr *)&sSock,ilSize);
        if(ilRc)
        { 
          swVdebug(1,"S0430: [����/ϵͳ����] sendto()����,errno=%d[%s]",errno,strerror(errno));
          swMberror(ilRc,(char *)&slMsgpack);
          continue;
        }
        
        swVdebug(2,"S0440: switch(��Ϣ��/������)");
        
        switch(llCode)
        {
          case 9901:  * ��ɱ *
            swVdebug(2,"S0450: case 9901:��ɱ");
            exit(15);
            break;

          case 601:  * ��ʱ���� *
            swVdebug(2,"S0460: case 601:��ʱ����");
            _swOrdsend(601,(char *)&slMsgpack,iMBMONREV);
            break;

          case 602:  * ֹͣ�������� *
            swVdebug(2,"S0470: case 602:ֹͣ��������");
            _swOrdsend(602,(char *)&slMsgpack,iMBMONREV);
            break;

          case 902:  * ǰ̨��¼ *
            ilSendFlag = 1;
            swVdebug(2,"S0480: Login�ɹ�");
            break;

          case 903:  * ǰ̨�˳� *
            ilSendFlag = 0;
            swVdebug(2,"S0490: Logout�ɹ�");
            break;

          default:
            break;
        }
        break;
*/
      case iMSGWINDOW : 
        
        swVdebug(2,"S0500: case ǰ̨����");

        if(pslMsghead->iBodylen < 29)
        {
          swVdebug(1,"S0510: [����/����] ��Ч����");
          break;
        }
        swVdebug(2,"S0520: iBodylen=%d",pslMsghead->iBodylen); 
        /* alMsgbody = (char *)&slMsgpack + sizeof(struct msghead); */
        alMsgbody = (char *)slMsgpack.aMsgbody;

        /* �ӱ�������ȡ����������ID */ 
        memcpy(alMsg_id,alMsgbody+1,3);
        alMsg_id[3]='\0'; 
        _swTrim(alMsg_id);
        
        /* �ӱ�������ȡ��IP��ַ */ 
        memcpy(alIpadd,alMsgbody+4,15);
        alIpadd[15]='\0'; 
        _swTrim(alIpadd);
        
        /* �ӱ�������ȡ�����Ͷ˿ں� */ 
        memcpy(alPort,alMsgbody+19,5);
        alPort[5]='\0'; 
        _swTrim(alPort);
        
        /* �ӱ�������ȡ���˿ں� */ 
        memcpy(alPort1,alMsgbody+24,5);
        alPort1[5]='\0'; 
        _swTrim(alPort1);

        swVdebug(2,"S0530: Msg_id=%s,IP Address=%s,UDP Port=%s,SEND Port=%s",
          alMsg_id,alIpadd,alPort,alPort1);

        if(alMsgbody[0] == 'A') /*ǩ������*/
        {
          swVdebug(2,"S0540: ǩ������,IP=[%s] PORT=[%s]",alIpadd,alPort1);
        
          for(i=0;i<SENDMAX;i++)
          {
            
            if(( psgSendwin[i].usrflag == 1)&&
              ( strcmp(psgSendwin[i].address,alIpadd) ==0)&&
              ( psgSendwin[i].port == atoi(alPort1)))
            {
              /* �л��������ñ������� */
              psgSendwin[i].Msg_id = atoi(alMsg_id);
              break;
            } 
        
            else if( psgSendwin[i].usrflag == 0)
            {
              strcpy(psgSendwin[i].address ,alIpadd);
              psgSendwin[i].port = atoi(alPort);
              psgSendwin[i].Msg_id = atoi(alMsg_id);
              psgSendwin[i].usrflag = 1; 
              psgSendwin[i].iNum_id = 0;
              psgSendwin[i].iEcho_id = 0; 

              ilRc=swFwinw();
              if(ilRc)
                swVdebug(1,"S0550: swFwinw()����ǩ����Ϣʧ��!");
              swVdebug(3,"S0560: swFwinw()����ǩ����Ϣ�ɹ�");
              break;
            }
          }
          
          if(i == SENDMAX)
          {
            _swUDPPut(igSock,alIpadd,atoi(alPort),"fail",4);
            swVdebug(1,"S0570: [����/����] ǩ�����ﵽ���,IP=[%s] PORT=[%s]",alIpadd,alPort);
          }
          
          else
          {
            ilRc=_swUDPPut(igSock,alIpadd,atoi(alPort),"success",7);
            swVdebug(2,"S0580: ǩ���ɹ�,ip=[%s] port=[%s]",alIpadd,alPort);
          }
          
          continue;
        }
        
        else if(alMsgbody[0] == 'B') /*ǩ�˱���*/
        {
          swVdebug(2,"S0590: ǩ�˱���,IP=[%s] PORT=[%s]",alIpadd,alPort1);
        
          for(i=0;i<SENDMAX;i++)
          {
            if(( psgSendwin[i].usrflag == 1)&&
              ( strcmp(psgSendwin[i].address,alIpadd) ==0)&&
              ( psgSendwin[i].port == atoi(alPort1))&&
              ( psgSendwin[i].Msg_id == atoi(alMsg_id)))
            {
              psgSendwin[i].usrflag = 0; 
              psgSendwin[i].iNum_id = 0;
              psgSendwin[i].iEcho_id = 0;
              for (j=SENDMAX;j>=0;j--)
              {
                if (psgSendwin[j].usrflag == 1)
                {
                  memcpy((char *)&psgSendwin[i],(char *)&psgSendwin[j],
                    sizeof(struct swt_sys_win));
                  psgSendwin[j].usrflag = 0;
                  break;
                }
              }
              break;
            }
          }
        
          ilRc=swFwinw();
          if(ilRc)
            swVdebug(1,"S0600: swFwinw()����ǩ����Ϣʧ��!");
          swVdebug(3,"S0610: swFwinw()����ǩ����Ϣ�ɹ�");

          _swUDPPut(igSock,alIpadd,atoi(alPort),"success",7);
          swVdebug(2,"S0620: ǩ�˳ɹ�,ip=[%s] port=[%s]",alIpadd,alPort);
        
          continue;
        }
        else
          break;
      
      case iMSGRETURN :
      
        if(pslMsghead->iBodylen < 19)
        {
          swVdebug(1,"S0630: [����/����] ��Ч����");
          break;
        }
        swVdebug(2,"S0640: iBodylen=%d",pslMsghead->iBodylen);
        alMsgbody = (char *)slMsgpack.aMsgbody;

        /* �ӱ�������ȡ��IP��ַ */
        memcpy(alIpadd,alMsgbody,15);
        alIpadd[15]='\0';
        _swTrim(alIpadd);

        /* �ӱ�������ȡ�����Ͷ˿ں� */
        memcpy(alPort,alMsgbody+15,5);
        alPort[5]='\0';
        _swTrim(alPort);

        swVdebug(2,"S0650: case ��Ӧ����,IP=[%s] PORT=[%s]",alIpadd,alPort);
        for(i=0;i<SENDMAX;i++)
        {
          if(( psgSendwin[i].usrflag == 1)&&
              ( strcmp(psgSendwin[i].address,alIpadd) ==0)&&
              ( psgSendwin[i].port == atoi(alPort)))
          {
            psgSendwin[i].iNum_id = 0;
            psgSendwin[i].iEcho_id = 0;
            swVdebug(2,"�յ���Ӧ���ģ���ʼ�����ļ������ɹ�ip=%s port=%s",alIpadd,alPort);
          }
        }
      default:
        swVdebug(2,"S0660: END switch");
        break;
    }           
    
    swVdebug(2,"S0670: END switch");
 
  } /* end of while */
}               

/**************************************************************
 ** ������      �� swQuit
 ** ��  ��      �� �źŴ�������
 ** ��  ��      ��  
 ** ��������    �� 1999/11/19
 ** ����޸����ڣ� 1999/12/22
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/

void swQuit(int sig)
{
  int i,ilRc;
  signal(SIGTERM,SIG_IGN);
  /*printf("===================ϵͳ��ؽ����Ѿ��ر�===================\n");*/

  swVdebug(2,"S0680: | system monitor stopped,signal is %d",sig);

  for( i=0;i<SENDMAX; i++)
  {
    if(psgSendwin[i].usrflag == 1)
    {
      ilRc=swSendwin(igSock,psgSendwin[i].address,psgSendwin[i].port,"QUIT",4);
      if(ilRc)
        swVdebug(1,"S0690: [����/����] ����ǰ̨ʧ��");
      else
      {
        swVdebug(2,"S0700: ����ǰ̨�ɹ�IP=[%s] PORT=[%d]",psgSendwin[i].address,psgSendwin[i].port);
      }
    }
  }
  for(i = 0; i < igCount_msgfile; i++)
    fclose(fgPack[i]);

  fclose(fgWin);
  
  /* �ر����ݿ����� */
  /*add by gxz 2003.1.7*/
  swDbclose();

  swVdebug(1,"S0710: | [swSysmon]�Ѿ��ر�!");
  close(sig);
  exit(sig);
}  

/**************************************************************
 ** ������      ��swMakebuffer
 ** ��  ��      ����֯������ˮ����
 ** ��  ��      ��nh
 ** ��������    ��2000/2/22
 ** ����޸����ڣ�2000/2/22
 ** ��������������
 ** ȫ�ֱ���    ��wsgSwt_sys_msgpack
 ** ��������    ��msgpack ����
 **               buffer  ��֯�õı��� 
 ** ����ֵ      ��SUCCESS,FAIL
***************************************************************/

int swMakebuffer(short iMsg_type,char * aBuffer)
{
  int   i,j=0;
  char  alTmpBuffer1[100];
  char  alTmpBuffer[200];
  short ilLen;
  char  *alPointer;
  char  *alTmp;
  
  swVdebug(4,"S0720: [��������] swMakebuffer(%d)",iMsg_type);

  alPointer = aBuffer;
  for (i=0;i<igCount_msgtype;i++)
  {
    alTmp = alTmpBuffer;
    if(wsgSwt_sys_msgpack[i].msg_type == iMsg_type)
    {
      if(_swExpress( wsgSwt_sys_msgpack[i].fld_express,\
        alTmpBuffer1, &ilLen) ==SUCCESS) 
      {
        for(j=0;alTmpBuffer1[j] != '\0';j++)
        {
          if(( alTmpBuffer1[j] == ';' )||( alTmpBuffer1[j] == '^'))
          { 
            *alTmp = '^';
            alTmp++;
            ilLen++;
          }

          *alTmp = alTmpBuffer1[j];
          alTmp++;
        }

        *alTmp = '\0';
        swVdebug(3,"S0730:"\
          "swMakeBuffer():swExpress,return = %s ilLen = %d",alTmpBuffer,ilLen);
      }
      else
      {
      	swVdebug(1,"S0740: [����/��������] _swExpress()����,������=-1");
        /* next add by nh 2002.3.20 */
        strcpy(alTmpBuffer," ");
        ilLen = 3;
        swVdebug(3,"S0750:"\
          "swMakeBuffer():swExpress,return = %s ilLen = %d",alTmpBuffer,ilLen);
        /* end add by nh */
/*        return(FAIL);*/
      }

      memcpy(alPointer, alTmpBuffer, ilLen);
      alPointer += ilLen;
      memcpy(alPointer, FILLER, FILLERLEN);
      alPointer += FILLERLEN;
    }
  }

  *alPointer = '\0';
  swVdebug(2,"S0760: ��֯���ĳɹ�");

  swVdebug(4,"S0770: [��������] swMakebuffer()������=0");
  return(SUCCESS);
}

void swClschild()
{
  swVdebug(1,"S0780: | [swSysmon]�Ѿ��ر�!");

  swQuit(igSock);
}


/**************************************************************
 ** ������      ��swLoadsysttytype
 ** ��  ��      ��Ԥ���ն����ñ�
 ** ��  ��      ��
 ** ��������    ��
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��psgSwt_sys_ttytype
 ** ��������    ����
 ** ����ֵ      ��SUCCESS,FAIL
***************************************************************/

int swLoadsysttytype(short *iTerm_num)
{
  FILE *fp;
  int  i=0;
  short ilRc;
  char palFldvalue[10][iFLDVALUELEN];
  char alTmpexp[2 * iFLDVALUELEN + 1];  /* add by zjj 2001.11.29  */
  
  swVdebug(4,"S0790: [��������] swLoadsysttytype()");

  /* Ԥ���ն������ļ���*/
  if ((fp = _swFopen("config/SWMONTERM.CFG","r")) == NULL)
  {
    swVdebug(1,"S0800: [����/ϵͳ����] fopen()����,errno=%d[%s], swt_sys_ttytype�ļ��򿪳���",errno,strerror(errno));
    fclose(fp);
    return(-1);
  }
  
  while ((ilRc = _swGetitemmode2(fp, palFldvalue,11)) == 0)
  {
    i++;
  
    sgSwt_sys_ttytype.Term_id = atoi(palFldvalue[0]);
    strcpy(sgSwt_sys_ttytype.discrib,palFldvalue[1]);
    strcpy(sgSwt_sys_ttytype.message,palFldvalue[2]);
    
    ilRc = _swExpN2T(palFldvalue[3],alTmpexp);
    if (ilRc != SUCCESS) return(ilRc);
    strcpy(sgSwt_sys_ttytype.lexpress,alTmpexp);

     
    _swTrim(sgSwt_sys_ttytype.discrib);
    _swTrim(sgSwt_sys_ttytype.message);
    _swTrim(sgSwt_sys_ttytype.lexpress);
    
    psgSwt_sys_ttytype = (struct swt_sys_ttytype *)realloc(psgSwt_sys_ttytype,
      i * sizeof(struct swt_sys_ttytype));

    memcpy((char *)&(psgSwt_sys_ttytype[i-1]),(char *)&sgSwt_sys_ttytype,
      sizeof(struct swt_sys_ttytype));
  
    swVdebug(3,"S0670 | %d,%s,%s,%s",
      	   psgSwt_sys_ttytype[i-1].Term_id,
           psgSwt_sys_ttytype[i-1].discrib,
           psgSwt_sys_ttytype[i-1].message,
           psgSwt_sys_ttytype[i-1].lexpress);
  }
  
  *iTerm_num = i;
  fclose(fp);
  swVdebug(2,"S0810: ���ն����ñ��ɹ�");
  
  swVdebug(4,"S0820: [��������] swLoadsysttytype()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      ��swLoadmsgpack()
 ** ��  ��      ��Ԥ���ն����ñ�
 ** ��  ��      ��
 ** ��������    ��
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��psgSwt_sys_msgpack
 ** ��������    ����
 ** ����ֵ      ��SUCCESS,FAIL
***************************************************************/

int swLoadmsgpack()
{
  FILE *fp;
  int  i;
  short ilRc;
  char palFldvalue[10][iFLDVALUELEN];
  char alTmpexp[2 * iFLDVALUELEN + 1]; /* add by zjj 2001.11.29 */
  
  swVdebug(4,"S0830: [��������] swLoadmsgpack()");

  /* Ԥ���ն������ļ���*/
  if ((fp = _swFopen("config/SWMSGPACK.CFG","r")) == NULL)
  {
    swVdebug(1,"S0840: [����/ϵͳ����] fopen()����,errno=%d[%s], swt_sys_ttytype�ļ��򿪳���",errno,strerror(errno));
    fclose(fp);
    return(-1);
  }
  
  i = 0;
  igCount_msgtype = 0;
  
  while ((ilRc = _swGetitemmode2(fp, palFldvalue,11)) == 0)
  {
    i++; 
    sgSwt_sys_msgpack.msg_type = atoi(palFldvalue[0]);
    if( sgSwt_sys_msgpack.msg_type < 1)
    {
      swVdebug(1,"S0850: [����/����] ���ͳ���");
    }
  
    wsgSwt_sys_msgpack = (struct swt_sys_msgpack *)realloc(wsgSwt_sys_msgpack,
      i * sizeof(struct swt_sys_msgpack));

    sgSwt_sys_msgpack.fld_id = atoi(palFldvalue[1]);
    strcpy(sgSwt_sys_msgpack.fld_desc,palFldvalue[2]);

    ilRc = _swExpN2T(palFldvalue[3],alTmpexp);
    if (ilRc != SUCCESS) return(ilRc);
    strcpy(sgSwt_sys_msgpack.fld_express,alTmpexp);

         
    sgSwt_sys_msgpack.fld_len = atoi(palFldvalue[4]);
    sgSwt_sys_msgpack.fld_display_len = atoi(palFldvalue[5]);
    strcpy(sgSwt_sys_msgpack.fld_alignment,palFldvalue[6]);
  
    memcpy((char *)&(wsgSwt_sys_msgpack[i-1]),
      (char *)&sgSwt_sys_msgpack,sizeof(struct swt_sys_msgpack));
  
    swVdebug(2,"S0860: wsgSwt_sys_msgpack[%d]:%d,%d,%s,%s,"\
      "%d,%d,%s",i-1,
    	  wsgSwt_sys_msgpack[i-1].msg_type,
     	  wsgSwt_sys_msgpack[i-1].fld_id,
      	  wsgSwt_sys_msgpack[i-1].fld_desc,
      	  wsgSwt_sys_msgpack[i-1].fld_express,
      	  wsgSwt_sys_msgpack[i-1].fld_len,
      	  wsgSwt_sys_msgpack[i-1].fld_display_len,
      	  wsgSwt_sys_msgpack[i-1].fld_alignment);
  
  }
  
  igCount_msgtype = i;
  fclose(fp);
  swVdebug(2,"S0870: ��������֯���ɹ�");
  
  swVdebug(4,"S0880: [��������] swLoadmsgpack()������=0");
  return(SUCCESS);
}
 
/**************************************************************
 ** ������      ��swLoadmsgpack1()
 ** ��  ��      ��Ԥ���ն����ñ�
 ** ��  ��      ��
 ** ��������    ��
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��psgSwt_sys_msgpack
 ** ��������    ��iMsgtype:��ʾ��������;iCount:��ʾ��������
 ** ����ֵ      ��SUCCESS,FAIL
***************************************************************/

int swLoadmsgpack1(short iMsgtype,int *iCount)
{
  int i;
  FILE *fp;
  char palFldvalue[10][iFLDVALUELEN];
  short ilRc;
  char alTmpexp[2*iFLDVALUELEN + 1];   /* add by zjj 2001.11.29 */
  
  swVdebug(4,"S0890: [��������] swLoadmsgpack1(%d)",iMsgtype);

  swVdebug(2,"S0900: ��ʼ��ȡ����Ϊ%d�ı��� ",iMsgtype);

  /* Ԥ�������ļ���*/
  if ((fp = _swFopen("config/SWMSGPACK.CFG","r")) == NULL)
  {
    swVdebug(1,"S0910: [����/ϵͳ����] fopen()����,errno=%d[%s],��swt_sys_msgpack.cfg�ļ�����",errno,strerror(errno));
    return(-1);
  }
  i = 0;

  while ((ilRc = _swGetitemmode2(fp, palFldvalue,11)) == 0)
  {
    if (atoi(palFldvalue[0]) == iMsgtype)
    {
      sgSwt_sys_msgpack.msg_type = atoi(palFldvalue[0]);
      sgSwt_sys_msgpack.fld_id = atoi(palFldvalue[1]);
      strcpy(sgSwt_sys_msgpack.fld_desc,palFldvalue[2]);
      
      /* add by zjj 2001.11.29  */
      ilRc = _swExpN2T(palFldvalue[3],alTmpexp);
      if (ilRc != SUCCESS) return(ilRc);
      strcpy(sgSwt_sys_msgpack.fld_express,alTmpexp);
      /* end add by zjj 2001.11.29  */

           
      sgSwt_sys_msgpack.fld_len = atoi(palFldvalue[4]);
      sgSwt_sys_msgpack.fld_display_len = atoi(palFldvalue[5]);
      strcpy(sgSwt_sys_msgpack.fld_alignment,palFldvalue[6]);

      psgSwt_sys_msgpack = (struct swt_sys_msgpack *)realloc(psgSwt_sys_msgpack,
        (i+1)*sizeof(struct swt_sys_msgpack));

      memcpy((char *)&psgSwt_sys_msgpack[i],(char *)&sgSwt_sys_msgpack,
        sizeof(struct swt_sys_msgpack));

      swVdebug(2,"S0920: psgSwt_sys_msgpack[%d]:%d,%d,%s,%s,"\
         "%d,%d,%s",i,
          psgSwt_sys_msgpack[i].msg_type,
          psgSwt_sys_msgpack[i].fld_id,
      	  psgSwt_sys_msgpack[i].fld_desc,
      	  psgSwt_sys_msgpack[i].fld_express,
      	  psgSwt_sys_msgpack[i].fld_len,
      	  psgSwt_sys_msgpack[i].fld_display_len,
      	  psgSwt_sys_msgpack[i].fld_alignment);
       i ++;
    }
  }/* end while */

  *iCount = i;
  swVdebug(2,"S0930: ��ȡ����Ϊ%d�ı��ĳɹ����ñ��ĵ������Ϊ%d ",
    iMsgtype,*iCount);

  swVdebug(4,"S0940: [��������] swLoadmsgpack1()������=0");
  return(0);
}


/**************************************************************
 ** ������      ��swSendwin()
 ** ��  ��      �� ���͵�windowsǰ��
 ** ��  ��      ��nh
 ** ��������    ��2001/8/2
 ** ����޸����ڣ�
 ** ����������������
 ** ȫ�ֱ���    ��
 ** ��������    ����
 ** ����ֵ      ��SUCCESS,FAIL
***************************************************************/

int swSendwin(int iSock,char *aAddr,short iPort,char *aMsgpack,short iMsglen)
{
  struct sockaddr_in slSockaddr;
  struct hostent *host;
  int ilTolen = sizeof(struct sockaddr_in);
  int ilRc;

  swVdebug(4,"S0950: [��������] swSendwin()");

  if(!(host = gethostbyname(aAddr)))
  {
    if(!(host = gethostbyaddr(aAddr,strlen(aAddr),AF_INET)))
      return -1;
  }

  slSockaddr.sin_family = AF_INET;
  memcpy((char*)&slSockaddr.sin_addr,host->h_addr,host->h_length);
  slSockaddr.sin_port = htons(iPort);
  
  ilRc = sendto(iSock,aMsgpack,iMsglen,0,
        (struct sockaddr *)&slSockaddr,ilTolen);
  if(ilRc == -1)
    return -1;

  swVdebug(4,"S0960: [��������] swSendwin()������=0");
  return(0);
}

/**************************************************************
 ** ������      : swGetField
 ** ��  ��      : �� aBuffer ��ȡ��n���ֶη���aField�С�
                  aBuffer�е�ÿ���ֶ���SeperatorΪ�ָ��ޡ�
                  n �� 1 ��ʼ��
 ** ��  ��      : nh 
 ** ��������    : 2001/08/10
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : aBuffer   : ԭ��
                  n         : ���
                  aField    : Ŀ����
                  Seperator : �ָ���
 ** ����ֵ      : SUCCESS
***************************************************************/

int swGetField(char *aBuffer,int n,char *aField,char Seperator)
{
  int  i,ilLen,j;
  char *alTmp, alBegin[200];

  swVdebug(4,"S0970: [��������] swGetField()");

  i=0 ;
  alTmp=aBuffer;

  if(*alTmp == Seperator)
  {
    alTmp++;
    i++;
  }
  
  while(*alTmp != '\0' &&  i != n-1) 
  {
    if(*alTmp == Seperator && *(alTmp-1) != '^') 
    {
      alTmp++;
      i++;
    }
    else
    alTmp++;
  }
  
  if (*alTmp == '\0') 
  {
    aField[0] ='\0';
    return(0);
  }
  
  alBegin[0] = '\0';
  ilLen=0;
  
  for(j=0; *alTmp != '\0' && j<200; j++)
  {
    if(*alTmp == Seperator && *(alTmp-1) != '^') 
      break;
    if(*alTmp == '^' && (*(alTmp+1) == '^' || *(alTmp+1) ==Seperator)) 
    {
      alTmp++;
      alBegin[j] = *alTmp;
    }
    else alBegin[j] = *alTmp;
    alTmp++;
    ilLen++;
  }
  
  if (ilLen != 0)
  {
    memcpy(aField, alBegin, ilLen);
    aField[ilLen] = '\0';
    return(ilLen);
  }
  
  else 
  {
    aField[0] = '\0';

    swVdebug(4,"S0980: [��������] swGetField()������=0");
    return(0);
  }
}
int swFwrite(char *aBuf,short iType)
{
  int  ilRc,ilLen=0;
  long llLogfilesize;
  char alFile[1024],alTmp[1024],alCmdbuf[1024];
  long llDatetime;
  struct stat slBuf;
  struct tm  *sttm;

  sprintf(alTmp,"%s\n",aBuf);
  llLogfilesize = 8388608L;

  memset(alFile,0x00,sizeof(alFile));
  sprintf(alFile,"%s/monlog/%s_%d.log",getenv("SWITCH_MON_PATH"),
    psgSwt_sys_ttytype[iType].discrib,psgSwt_sys_ttytype[iType].Term_id);

  ilRc=stat(alFile, &slBuf);
  if (slBuf.st_size > llLogfilesize)
  {
    fclose(fgPack[iType]);

    time(&llDatetime);
    sttm = localtime( &llDatetime );

    sprintf(alCmdbuf,"mv %s %s.%02d%02d%02d%02d%02d",
      alFile,alFile,
      sttm->tm_mon+1,
      sttm->tm_mday,
      sttm->tm_hour,
      sttm->tm_min,
      sttm->tm_sec );
    if ((access(alFile,F_OK)) != -1)
      system(alCmdbuf);
    if (( fgPack[iType] = fopen(alFile,"a+")) == NULL)  return(-1);
  }

  fprintf(fgPack[iType],"%s",alTmp);

  swVdebug(2,"S0990: swFwrite() д�ļ��ɹ�,len=[%d]",ilLen);
  fflush(fgPack[iType]);
  
  return(0);
}

int swFwinr()
{
  int  i,ilLen = 0;
  char alFile[100];
  struct swt_sys_win pslWin[SENDMAX];
  
  swVdebug(3,"S1000: swFwinr() begin ");
  sprintf(alFile,"%s/sysmon.ini",getenv("SWITCH_MON_PATH"));
  if((fgWin = fopen(alFile,"r")) == NULL)
  {
    if((fgWin = fopen(alFile, "w+")) == NULL)
    {
      swVdebug(1,"S1005: ���ļ�[%s]����!",alFile);
      return(FAIL);
    }
  }
  swVdebug(1,"S1006: ���ļ�[%s]�ɹ�!",alFile);
  memset((char *)pslWin,0x00,SENDMAX*sizeof(struct swt_sys_win));
  ilLen = fread((char *)pslWin,sizeof(struct swt_sys_win),SENDMAX,fgWin); 
  if(ilLen < 0)
    return(-1);
  if(ilLen > 0)
  {
    swVdebug(4,"S1010: swFwinr() fread len=[%d]",ilLen);

    for(i = 0; i < SENDMAX; i++)
    {
      if(pslWin[i].usrflag == 0) break;
      memcpy((char *)&psgSendwin[i],(char *)&pslWin[i],sizeof(struct swt_sys_win));
      psgSendwin[i].iNum_id = 0;
      psgSendwin[i].iEcho_id = 0;

    }
    for(i = 0; i < SENDMAX; i++)
      swVdebug(4,"S1020: swFwinr() [%d][%s][%d][%d][%ld][%ld]",
        psgSendwin[i].usrflag,
        psgSendwin[i].address,
        psgSendwin[i].port,
        psgSendwin[i].Msg_id,
        psgSendwin[i].iNum_id,
        psgSendwin[i].iEcho_id);
  }
  swVdebug(3,"S1030: swFwinr() return success ");
  fclose(fgWin);
  return(0);
}
int swFwinw()
{
  int  i,ilLen = 0;
  char alFilewin[100];

  swVdebug(3,"S1040: swFwinw() begin ");
  
  sprintf(alFilewin,"%s/sysmon.ini",getenv("SWITCH_MON_PATH"));
  if((fgWin = fopen(alFilewin,"w+")) == NULL)
  {
    swVdebug(1,"S1050: swFwinw() fopen(),errno=%d[%s], �ļ�[%s]�򿪳���",errno,strerror(errno),alFilewin);
    fclose(fgWin);
    return(-1);
  }
  
  for(i=0;i<SENDMAX;i++)
  {
    if(psgSendwin[i].usrflag == 1)
    { 
      ilLen = fwrite((char *)&psgSendwin[i],sizeof(struct swt_sys_win),1,fgWin);
      if(ilLen < 0)
        return(-1);
      swVdebug(4,"S1060: swFwinw() fwrite len=[%d]",ilLen);
    }
  }

  fclose(fgWin);
  return(0);
}