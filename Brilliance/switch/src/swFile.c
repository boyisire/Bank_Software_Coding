/****************************************************************/
/* ģ����    ��FILE                                           */ 
/* ģ������    ���ļ�����                                       */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ��ͯ����                                         */
/* ��������    ��2001/05/21                                     */
/* ����޸����ڣ�2001/05/21                                     */
/* ģ����;    ������ƽ̨�ڲ��ļ�����                           */
/* ��ģ���а������º���������˵����                             */
/*			 ��1��int  main();                      */
/*			 ��2��int  swQuit();                    */
/*			 ��3��int  swParase();                  */
/*			 ��4��void swAlarm();                   */
/*			 ��5��int swPackandqwrite( )            */
/****************************************************************/
/****************************************************************/
/* �޸ļ�¼��                                                   */
/****************************************************************/

/* switch���� */
#include "switch.h"
#include "swNdbstruct.h"
#include "swShm.h"

#ifdef OS_SCO
#include <net/libftp.h>
#endif

#ifdef OS_AIX
#include "swftp.h"
#endif

#ifdef OS_LINUX
#include "swftp.h"
#endif

/* �������� */
char agFilecode[3];   
struct msgpack sgMsgpack;   /* ���Ķ��� */
 
/* FTP�ļ���������ṹ */
struct fileFtp
{
  char  aHost[21];
  char  aUser[21];
  char  aPasswd[21];
  char  aRempath[51];
  char  aLocalpath[51];
  char  aTranmode[7];
  short iOvertime;
};

/* Q�ļ���������ṹ */
struct fileQ
{
  short iGroup;
  char  aRempath[51];
  char  aLocalpath[51];
  unsigned int iPacksize;
  short iOvertime;
  short iSafnum;
};

union filep
{
  struct fileFtp sFileftp;
  struct fileQ   sFileQ;
};  

/* Q�����ͷ�ṹ */
struct qreq
{
  char  cPackmode;  /* 1-QPUT��ʾ�� 2-QPUT��Ӧ�� 3-QGET����� 4-QGET��Ӧ�� */
  char  aRempath[51];
  short iPackid;
  unsigned int iPacksize;
}; 
     
/* Q��Ӧ��ͷ�ṹ */
struct qres
{
  char  cPackmode;  /* 1-QPUT��ʾ�� 2-QPUT��Ӧ�� 3-QGET����� 4-QGET��Ӧ�� */
  char  cEndflag;
  char  cRescode;
  short iPackid;
  unsigned int iPacksize;
};

/* �������� */
void swQuit(int); 
void swAlarm( );
int  swParse(char *,char *,union filep *);
int  swPackandqwrite( );
int  swClrMB1(short,short,short);
/**************************************************************
 ** ������      ��main
 ** ��  ��      �������� 
 ** ��  ��      ��ͯ����
 ** ��������    ��2001/05/21
 ** ����޸����ڣ�2001/05/21
 ** ��������������
 ** ȫ�ֱ���    ��psgPreunpack
 ** ��������    ��
 ** ����ֵ      ��
***************************************************************/
int main(int argc,char *argv[])
{
  short  ilPriority;          /* �������:���ȼ� */
  short  ilClass;             /* �������:��� */
  short  ilType;              /* �������:���� */
  short  ilOrgqid=0;          /* Դ������ */
  short  ilGroupid=0;         /* ������ */
  short  ilOrg_q;             /* �ļ�����Դ������ */
  short  ilOrg_group;         /* �ļ����������� */
  short  ilRc;                /* ����ֵ */ 
  short  ilFldlen;            /* �� */
  short  ilEndflag;           /* �ļ�������־ */
  unsigned int  ilReadsize;          /* �����䱨�ĳ� */
  short  ilReadn;             /* ���ļ���  */
  short  ilBuflen;            /* �ļ����� */
  short  m,n;
  long   llFilelen;           /* �ļ��� */
  char   alFilemode[2];       /* �ļ����䷽ʽ */
  char   alFileparm[171];     /* �ļ�������� */
  char   alBuf[iMSGMAXLEN];
  char   alMsgpack[iMSGMAXLEN];
  char   paFldvalue[5][iFLDVALUELEN];
  FTPINFO ftpinfo;
  FILE   *fp;
  struct fileFtp slFileftp;   /* FTP�ļ���������ṹ */
  struct fileQ   slFileQ;     /* Q �ļ���������ṹ */
  struct qreq slQreq;         /* Q����� */
  struct qres slQres;         /* Q��Ӧ�� */
  struct timeb  fb;
  union  filep ulFilep;        /* �ļ�������� */

  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swFile Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swFile")) exit(FAIL);
    
  /* ���õ��Գ������� */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strncpy(agDebugfile, "swFile.debug", sizeof(agDebugfile));

  /* ȡ��DEBUG��־ */
  if ( (cgDebug=_swDebugflag("swFile")) == FAIL )
  {
    fprintf(stderr,"ȡ��DEBUG��־����!");
    exit(FAIL);
  } 

  /* ����SIGCLD��SIGINT��SIGQUIT��SIGHUP �ź� */
  swVdebug(2,"S0010: ����SIGCLD��SIGINT��SIGQUIT��SIGHUP �ź�");
   
  signal( SIGTERM , swQuit  );
  signal( SIGINT  , SIG_IGN );
  signal( SIGQUIT , SIG_IGN );
  signal( SIGTSTP , SIG_IGN );
  signal( SIGHUP  , SIG_IGN );
  signal( SIGCLD  , SIG_IGN );

  /* ��λ�������� */
  if (qattach(iMBFILE)) 
  {
    swVdebug(0,"S0020: [����/����] ��ʼ���������!");
    exit(FAIL);
  }
  swVdebug(2,"S0030: ��λ��������ɹ�");

  ilRc = swShmcheck();
  if (ilRc)
  {
    swVdebug(0,"S0040: [����/ϵͳ����] swShmcheck()����");
    exit(FAIL);
  }

  ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
  if (ilRc)
  {
    swVdebug(1,"S0050: [����/ϵͳ����] swShmselect_swt_sys_config()����,������=%d",ilRc);
    exit(FAIL);
  }
  strcpy(paFldvalue[0],sgSwt_sys_config.ftpmode);
  sprintf(paFldvalue[1],"%d",sgSwt_sys_config.ftpovertime);  
  sprintf(paFldvalue[2],"%d",sgSwt_sys_config.qpacksize);  
  sprintf(paFldvalue[3],"%d",sgSwt_sys_config.qovertime);  
  sprintf(paFldvalue[4],"%d",sgSwt_sys_config.qresendnum);  
  
  for( ; ; )
  {
    /* ���ļ�����������뱨�� */ 
    ilReadsize = iMSGMAXLEN;
    ilPriority = 0;
    ilClass = 0;
    ilType = 0;
    if ( ( ilRc = qread3( (char *)&sgMsgpack, &ilReadsize, &ilOrgqid,\
     &ilGroupid,&ilPriority,&ilClass,&ilType)) != SUCCESS ) 
    {
      swVdebug(1,"S0060: [����/��������] qread3()����,���������,������=%d",ilRc);
      swMberror(ilRc, NULL);
      swQuit(FAIL);
    }

    /* ����Դ������Ϊ·������ */
    if (ilOrgqid == iMBROUTER)
    {
      /* ��������IMF����ֽ���FML����Ԥ��أ���Ԥ��IMF��ʽ�������� */
      ilRc = swFmlunpack( sgMsgpack.aMsgbody,
        sgMsgpack.sMsghead.iBodylen, psgPreunpackbuf );
      if ( ilRc == FAIL )
      {
        swVdebug(1,"S0070: [����/��������] swFmlunpack()����,FML���Ľ������!");
        _swMsgsend( 399001, NULL ); 
        continue;
      }
      swVdebug(2,"S0080: FML���Ľ���ɹ�");
 
      /* �ӱ�����ȡ��_FILEMDODE�� */
      memset(alFilemode,0x00,sizeof(alFilemode));
      ilRc = swFmlget(psgPreunpackbuf,"_FILEMODE",&ilFldlen,alFilemode);
      if (ilRc)
      {
        swVdebug(1,"S0090: [����/��������] swFmlget()����,ȡ_FILEMODE�����,������=%d",ilRc);
        continue;
      }
      swVdebug(2,"S0100: �ӱ�����ȡ��FILEMODE��"); 
      
      /* �ӱ�����ȡ��_FILEPARM�� */
      memset(alFileparm,0x00,sizeof(alFileparm));
      ilRc = swFmlget(psgPreunpackbuf,"_FILEPARM",&ilFldlen,alFileparm);
      if (ilRc)
      {
        swVdebug(1,"S0110: [����/��������] swFmlget()����,ȡ_FILEPARM�����,������=%d",ilRc);
        continue;
      }
      swVdebug(2,"S0120: FILEPARM[%s]",alFileparm); 
 
      /* �����ļ�������� */
      if ((ilRc=swParse(alFileparm,alFilemode,&ulFilep))==FAIL)
      {
        swVdebug(1,"S0130: [����/��������] swParse()����,�����ļ������������,������=%d",ilRc);
        continue;
      }
      swVdebug(2,"S0140: �����ļ���������ɹ�");
 
      slFileftp=ulFilep.sFileftp;
      if (slFileftp.aTranmode[0] =='\0')
        strcpy(slFileftp.aTranmode,paFldvalue[0]);
      if (!slFileftp.iOvertime)
        slFileftp.iOvertime = atoi(paFldvalue[1]);
 
      slFileQ=ulFilep.sFileQ;
      if (!slFileQ.iPacksize)
        slFileQ.iPacksize = atoi(paFldvalue[2]);
      if (!slFileQ.iOvertime)
        slFileQ.iOvertime = atoi(paFldvalue[3]);
      if (!slFileQ.iSafnum)
        slFileQ.iSafnum = atoi(paFldvalue[4]);
         
      strcpy(agFilecode,"0");  

      ftime(&fb);
      swVdebug(3,"S0150: �ļ����俪ʼ:[%d����]",fb.millitm);
 
      switch(alFilemode[0])
      {
        /* ��FTP��ʽ�����ļ� */
        case cFTPPUT:
        case cFTPGET:
       
        /* ����һ���ӽ��� */
        if (fork()==0)
        {
          /* ���ó�ʱ���� */
          signal( SIGALRM, swAlarm );
          alarm(slFileftp.iOvertime);

          /* ���ӵ��ļ��������� */
          if (qattach(iMBFILETRAN))
          {
            swVdebug(1,"S0160: [����/����] ��ʼ���ļ������������!");
            exit(FAIL);
          }
          swVdebug(2,"S0170: ��λ�ļ���������ɹ�");
 
          /* ���ӵ�Զ������ */
          ilRc=ftp_login(&ftpinfo,slFileftp.aHost,slFileftp.aUser,\
            slFileftp.aPasswd,NULL);
          if(ilRc < 0)
          {
            swVdebug(1,"S0180: [����/��������] ftp_login()����,��½Զ����������,������=%d",ilRc); 
            strcpy(agFilecode,"-1");
            /* ���Ĵ�����͸�·�� */
            swPackandqwrite( ); 
            exit(-1);
          }
          swVdebug(2,"S0190: ��½Զ�������ɹ�"); 
 
          /* �����ļ����䷽ʽ */
          if ( !strncmp(slFileftp.aTranmode,"binary",6) || \
            !strncmp(slFileftp.aTranmode,"BINARY",6)) 
          {
            ftp_binary(&ftpinfo); 
            swVdebug(2,"S0200: �����ļ����䷽ʽ[binary]");
          }
          else if ( !strncmp(slFileftp.aTranmode,"ascii",5) || \
            !strncmp(slFileftp.aTranmode,"ASCII",5)) 
          {
            ftp_ascii(&ftpinfo);
            swVdebug(2,"S0210: �����ļ����䷽ʽ[ascci]");
          }
          else
          {
            swVdebug(1,"S0220: [����/����] FTP�ļ�����ģʽ���ô���[%s]", 
              slFileftp.aTranmode);
            strcpy(agFilecode,"-4");
            /* ���Ĵ�����͸�·�� */
            swPackandqwrite( ); 
            exit(-1);
          }
 
          if (alFilemode[0] == cFTPPUT)
          {
            /* ��Զ�����������ļ� */
            ilRc=ftp_putfile(&ftpinfo,slFileftp.aRempath,slFileftp.aLocalpath); 
            swVdebug(1,"S0230: [����/��������] ftp_putfile()����,�����ļ���Զ������,������=%d",ilRc); 
          }  
          else
          {
            /* ��Զ������ȡ�ļ� */
            ilRc=ftp_getfile(&ftpinfo,slFileftp.aRempath,slFileftp.aLocalpath); 
            swVdebug(1,"S0240: [����/��������] ftp_getfile()����,��Զ������ȡ��,������=%d",ilRc); 
          }
          /* �ر�FTP���� */
          ftp_bye(&ftpinfo);
          swVdebug(2,"S0250: �ر�FTP����");
 
          if (ilRc < 0)
          {
            swVdebug(1,"S0260: [����/����] �ļ��������[%d]",ilRc); 
            strcpy(agFilecode,"-2");
            /* ���Ĵ�����͸�·�� */
            swPackandqwrite( );
            exit(-1);
          }
          swVdebug(2,"S0270: �����ļ�[%s]�ɹ�",slFileftp.aLocalpath);
           
          ftime(&fb);
          swVdebug(2,"S0280: �ļ����������[%d����]",fb.millitm);
 
          /* ���Ĵ�����͸�·�� */
          swPackandqwrite( );
          exit(0);
        }
        break;
      /* ��Q��ʽ���� */
      case cQPUT:
      case cQGET: 

        /* ����һ���ӽ��� */
        if (fork()==0)
        {
          /* ����TypeΪ���̺� */
         ilType=getpid();

          /* ���ӵ��ļ��������� */
          if (qattach(iMBFILETRAN))
          {
            swVdebug(1,"S0290: [����/����] ��ʼ���ļ������������!");
            exit(FAIL);
          }
          swVdebug(2,"S0300: ��λ�ļ���������ɹ�");
 
          /* ��������TYPEΪ���̺ŵı��� */
          if ((ilRc=swClrMB1(iMBFILETRAN,0,ilType)) !=SUCCESS)
          {
            swVdebug(1,"S0310: [����/����] ������ʧ��");
            exit(-1);
          }
          swVdebug(2,"S0320: ������ɹ�");
 
          /* NEWһ�������ͷ */
          memset(&slQreq,0x00,sizeof(struct qreq)); 
          strcpy(slQreq.aRempath,slFileQ.aRempath);
          slQreq.iPacksize = slFileQ.iPacksize;

          /* QPUT */
          if (alFilemode[0] == cQPUT)
          {
            slQreq.cPackmode ='1';
            /* �򿪱����ļ� */
            if ( (fp = fopen(slFileQ.aLocalpath,"r")) == NULL)
            { 
              swVdebug(1,"S0330: [����/ϵͳ����] ���ļ�[%s]����,errno=%d[%s]",slFileQ.aLocalpath,errno,strerror(errno));
              strcpy(agFilecode,"-2");
              /* ���Ĵ�����͸�·�� */
              swPackandqwrite( );
              exit(-1);
            }
            swVdebug(2,"S0340: ���ļ�[%s]",slFileQ.aLocalpath);
 
            ilEndflag=0;
            n=0;
            while (!ilEndflag)
            {
              n++;
              m = 0;
              slQreq.iPackid=n;

              /* ���ļ��ж�����n���ļ��� */
              memset(alBuf,0x00,sizeof(alBuf));
              ilReadn=fread(alBuf,sizeof(char),slFileQ.iPacksize,fp);
              if (ilReadn != slFileQ.iPacksize)
              {
                if ( !feof(fp))
                {
                  swVdebug(1,"S0350: [����/��������] fread()����,���ļ�[%s]����",slFileQ.aLocalpath);
                  fclose(fp);
                  strcpy(agFilecode,"-2");
                  /* ���Ĵ�����͸�·�� */
                  swPackandqwrite( );
                  exit(-1);
                }
                /* ���ļ�������־ */
                ilEndflag=1;  
                fclose(fp);
                swVdebug(2,"S0360: �ļ�����");
              }
              swVdebug(2,"S0370: ���ļ��ж�����[%d]�ļ���",n);
                 
              memset(alMsgpack,0,sizeof(alMsgpack));
              memcpy(alMsgpack,(char *)&slQreq,sizeof(struct qreq));
              memcpy(alMsgpack + sizeof(struct qreq),alBuf,ilReadn);
Lable1:
              /* ��Զ�������鷢��n��QPUT������ */
              ilRc = qwrite3(alMsgpack, sizeof(struct qreq)+ilReadn,
                iMBFILE,slFileQ.iGroup,ilPriority,ilClass,ilType);
              if(ilRc)
              {
                fclose(fp);
                swVdebug(1,"S0380: [����/��������] qwrite3()����,дԶ���������,������=%d",ilRc);
                strcpy(agFilecode,"-1");
                /* ���Ĵ�����͸�·�� */
                swPackandqwrite( );
                exit(1);
              }
              swVdebug(2,"S0390: ����[%d]���ļ����ɹ�",n);
 Lable2:
              /* ����������n����Ӧ���� */
              ilOrg_group = 0;
              ilOrg_q = 0;
              ilReadsize = iMSGMAXLEN;
              ilRc = qread3nw(alMsgpack,&ilReadsize, &ilOrg_q,
                &ilOrg_group,&ilPriority,&ilClass,&ilType, slFileQ.iOvertime);
              if ((ilRc == BMQ__TIMEOUT) || (ilRc == BMQ__NOMOREMSG))
              {
                /* ��ȷ�ϱ��ĳ�ʱ */
                swVdebug(1,"S0400: [����/��������] qread3nw()����,����[%d]����Ӧ����ʱ",n);
                m++;
                if (m <= slFileQ.iSafnum)
                {
                  /* �ط���n�������� */
                  swVdebug(1,"S0410: [����/����] ��[%d]���ط���[%d]��QPUT�����",m,n);
                  goto Lable1;
                }
                else
                {
                  swVdebug(1,"S0420: [����/����] ����[%d]����Ӧ��ʧ��",n);
                  fclose(fp);
                  strcpy(agFilecode,"-3");
                  /* ���Ĵ�����͸�·�� */
                  swPackandqwrite( );
                  exit(-1);
                }
              }
              /* ��������� */
              if (ilRc)
              {
                fclose(fp);
                strcpy(agFilecode,"-4");
                /* ����󷢸�·������ */
                swPackandqwrite( );
                exit(-1);
              }
            
              memcpy( (char *)&slQres,alMsgpack,ilReadsize);
              if (slQres.iPackid != n)
              {
                /* �յ���������Ӧ�������� */
                swVdebug(2,"S0430: �յ�һ����������Ӧ��"); 
                goto Lable2;
              }
              /* �յ��Է�������Ϣ�� */
              if (slQres.cRescode == '1')
              {
                fclose(fp);
                swVdebug(2,"S0440: �յ��Է�������Ϣ��"); 
                strcpy(agFilecode,"-4");
                /* ���Ĵ�����͸�·�� */
                swPackandqwrite( );
                exit(-1);
              }
              swVdebug(2,"S0450: ����[%d]����Ӧ���ɹ�",n);
            }
          }
          /* QGET */
          else 
          {
            slQreq.cPackmode='3';
            /* ���������ļ� */
            if ( (fp=fopen(slFileQ.aLocalpath,"w")) == NULL)
            { 
              swVdebug(1,"S0460: [����/ϵͳ����] fopen()����,�����ļ�[%s]����,errno=%d[%s]",slFileQ.aLocalpath,errno,strerror(errno));
              strcpy(agFilecode,"-2");
              /* ���Ĵ�����͸�·�� */
              swPackandqwrite( );
              exit(-1);
            }
            swVdebug(2,"S0470: �����ļ�[%s]",slFileQ.aLocalpath);
 
            n=0;
            while(1)
            {
              n++;
              slQreq.iPackid=n;
              m=0;
Lable3:
              /* ��Զ�������鷢��n��QGET������ */
              if ( (ilRc=qwrite3((char *) &slQreq,sizeof(struct qreq),iMBFILE,
                 slFileQ.iGroup,ilPriority,ilClass,ilType)) != SUCCESS )
              {
                swVdebug(1,"S0480: [����/��������] qwrite3()дԶ���������,������=%d",ilRc);
                fclose(fp);
                strcpy(agFilecode,"-1");
                /* ���Ĵ�����͸�·�� */
                swPackandqwrite( );
                exit(-1);
              }
              swVdebug(2,"S0490: ����[%d]��QGET�����",n);
 Lable4:                           
              /* ����������n��QGET�ļ���Ӧ�� */
              ilOrg_q=0;
              ilOrg_group=0;
              ilReadsize = iMSGMAXLEN;
              ilRc =qread3nw(alMsgpack,&ilReadsize, &ilOrg_q, &ilOrg_group,
                 &ilPriority,&ilClass,&ilType, slFileQ.iOvertime); 
              if ((ilRc == BMQ__TIMEOUT) || (ilRc == BMQ__NOMOREMSG))
              {
                m++;
                if (m<=slFileQ.iSafnum)
                {
                  /* �ط���n������� */
                  swVdebug(1,"S0500: [����/����] ��[%d]���ط���[%d]��QPUT�����",m,n);
                  goto Lable3;
                }
                else
                {
                  swVdebug(1,"S0510: [����/����] ����Ӧ����ʱ");
                  fclose(fp);
                  strcpy(agFilecode,"-3");
                  /* ���Ĵ�����͸�·�� */
                  swPackandqwrite( );
                  exit(-1);
                }
              }
              if (ilRc)
              {
        	/* ��������� */
                fclose(fp);
                strcpy(agFilecode,"-4");
                /* ����󷢸�·������ */
                swPackandqwrite( );
                exit(-1);
              }

              memcpy((char *)&slQres,alMsgpack,sizeof(struct qres));
              /* �յ���������Ӧ��,���� */
              if (slQres.iPackid != n)
              {
                swVdebug(2,"S0520: �յ�һ����������Ӧ��"); 
                goto Lable4;
              }
              /* �յ��Է�������Ϣ */
              if (slQres.cRescode == '1')
              {
                swVdebug(2,"S0530: �յ��Է�������Ϣ"); 
                fclose(fp);
                strcpy(agFilecode,"-2");
                /* ���Ĵ�����͸�·�� */
                swPackandqwrite( );
                exit(-1);
              }
              swVdebug(2,"S0540: ����[%d]��QGET��Ӧ���ɹ�",n);
 
              memset(alBuf,0,sizeof(alBuf));
              memcpy(alBuf,alMsgpack+sizeof(struct qres),slQres.iPacksize);
                  
              /* ���ļ���д�뱾���ļ� */
              ilReadn=fwrite(alBuf,sizeof(char),slQres.iPacksize,fp);
              if (ilReadn < slQres.iPacksize)
              {
                swVdebug(1,"S0550: [����/ϵͳ����] fwrite()д�ļ�[%s]����,errno=%d[%s]",slFileQ.aLocalpath,errno,strerror(errno));
                fclose(fp);
                strcpy(agFilecode,"-4");
                /* ���Ĵ�����͸�·�� */
                swPackandqwrite( );
                exit(-1);
              }
              if (slQres.cEndflag == '1')
              {
                swVdebug(2,"S0560: �ļ��������");
                fclose(fp);
                break;
              }
            }
          }
          ftime(&fb);
          swVdebug(3,"S0570: �ļ����������[%d����]",fb.millitm);
 
          /* ���Ĵ�����͸�·�� */
          swPackandqwrite( );
          exit(0);
        }
        break;
      default:
        break;
      }
    }
    /* ����Դ������Ϊ�ļ��������� */
    else if(ilOrgqid == iMBFILETRAN)
    {
      memset(&slQreq,0x00,sizeof(slQreq));
      memcpy((char *)&slQreq,(char *)&sgMsgpack,sizeof(struct qreq));
      memset(alBuf,0x00,sizeof(alBuf));

      /* NEW һ����Ӧ�� */
      memset(&slQres,0x00,sizeof(struct qres));
      slQres.cRescode = '0';
      slQres.cEndflag = '0';

      /* QPUT����� */
      if (slQreq.cPackmode == '1')
      {
        slQres.cPackmode='2';
        slQres.iPackid=slQreq.iPackid;
        ilBuflen=ilReadsize - sizeof(struct qreq);
        memcpy(alBuf,(char *)&sgMsgpack+sizeof(struct qreq),ilBuflen);

        if(slQreq.iPackid == 1)
        {
          /* �Դ��������ļ� */
          if ( (fp=fopen(slQreq.aRempath,"w")) == NULL)
          {
            swVdebug(1,"S0580: [����/ϵͳ����] fopen()���ļ�[%s]����,errno=%d[%s]",slQreq.aRempath,errno,strerror(errno));
            slQres.cRescode='1'; 
            goto Lable5;
          }
          swVdebug(2,"S0590: ���ļ�[%s]",slQreq.aRempath);
        }
        else
        {
          /* ��׷�ӷ�ʽ���ļ� */
          if ( (fp=fopen(slQreq.aRempath,"a+")) == NULL)
          {
            swVdebug(1,"S0600: [����/ϵͳ����] fopen()���ļ�[%s]����,errno=%d[%s]",slQreq.aRempath,errno,strerror(errno));
            slQres.cRescode='1'; 
            goto Lable5;
          }
 
          /* У���ļ����Ϸ��� */
          ilRc=fseek(fp, 0, SEEK_END);
          if ( ilRc ) 
          {
            swVdebug(1,"S0610: [����/ϵͳ����] fseek()�����ļ�����,error=%d[%s]",errno,strerror(errno));
            slQres.cRescode='1'; 
            goto Lable5;
          }
          llFilelen = ftell(fp);
          if (llFilelen != (slQreq.iPackid - 1) * slQreq.iPacksize)
          {
            swVdebug(2,"S0620: [����/����] �ļ������Ϸ�");
            continue;
          }
        }

        /* ���ļ���д���ļ� */
        ilReadn=fwrite(alBuf,sizeof(char),ilBuflen,fp);
        /* �ر��ļ� */
        fclose(fp);
        if (ilReadn != ilBuflen)
        {
          swVdebug(2,"S0630: д�ļ�[%s]����",slQreq.aRempath);
          slQres.cRescode='1'; 
          goto Lable5;
        }
        swVdebug(2,"S0640: д�ļ�[%s]",slQreq.aRempath);
 
Lable5:
        /* ������Ӧ�����ļ��������� */
        if ( ( ilRc = qwrite3( (char *)&slQres,sizeof(struct qres),
           ilOrgqid,ilGroupid,ilPriority,ilClass,ilType )) != SUCCESS)
        {
          swVdebug(2,"S0650: ������Ӧ������");
          swMberror(ilRc, "����:qwrite���ļ������������!");
          swQuit(FAIL);
        }
        swVdebug(2,"S0660: ���͵�[%d]����Ӧ��",slQreq.iPackid);
      }

      /* QGET����� */
      else if (slQreq.cPackmode == '3')
      {
        slQres.cPackmode='4';
        slQres.iPackid=slQreq.iPackid;
        ilReadn = 0;
        /* ��ֻ����ʽ���ļ� */
        if ((fp=fopen(slQreq.aRempath,"r")) == NULL)
        {
          swVdebug(2,"S0670: ���ļ�[%s]����",slQreq.aRempath);
          slQres.cRescode='1'; 
          goto Lable6;
        }
        swVdebug(2,"S0680: ���ļ�[%s]",slQreq.aRempath);
 
        /* ����N���ļ��� */
        ilRc = fseek(fp,(slQreq.iPackid-1)*slQreq.iPacksize,SEEK_SET); 
        if (ilRc)
        {
          swVdebug(2,"S0690: �����ļ�����:fseek():error");
          slQres.cRescode='1'; 
          goto Lable6;
        }

        ilReadn=fread(alBuf,sizeof(char),slQreq.iPacksize,fp);
        if( ilReadn != slQreq.iPacksize )
        {
          /* �ļ�δ���� */
          if (! feof (fp) )
          {
            /* ���ļ����� */
            swVdebug(2,"S0700: ���ļ�[%s]����",slQreq.aRempath);
            /*����Ӧ���ĳ���,�ر��ļ� */
            slQres.cRescode='1'; 
          }
          else
          {
            swVdebug(2,"S0710: �ļ�[%s]����",slQreq.aRempath);
            /* �ر��ļ�, ���ļ����� */
            slQres.cEndflag='1';
          }
        }
        fclose(fp);
        swVdebug(2,"S0720: ���ļ�[%s]�ɹ�",slQreq.aRempath);
 
Lable6:
        slQres.iPacksize = ilReadn;
        memset(alMsgpack,0x00,sizeof(alMsgpack));
        memcpy(alMsgpack,(char *)&slQres,sizeof(struct qres));
        memcpy(alMsgpack+sizeof(struct qres),alBuf,ilReadn);
   
        /* ������Ӧ�����ļ��������� */
        if ( ( ilRc = qwrite3( (char *)&alMsgpack,sizeof(struct qres) + ilReadn,
            ilOrgqid,ilGroupid,ilPriority,ilClass,ilType )) != SUCCESS)
        {
          swVdebug(2,"S0730: ������Ӧ������");
          swMberror(ilRc, "����:qwrite���ļ������������!");
          swQuit(FAIL);
        }
        swVdebug(2,"S0740: ���͵�[%d]����Ӧ��", slQreq.iPackid);
      }
    }
    else continue; 
  }
}

/**************************************************************
 ** ������:   swQuit()
 ** ��  ��:   �����˳�
 ** ��  ��:   ͯ����
 ** ��������: 2001.05.23
 ** ����޸�����:2001.05.23
 ** ������������:   
 ** ȫ�ֱ���:  
 ** ��������:       
 ** ����ֵ:
***************************************************************/
void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swVdebug(0,"S0750: [swFile]�Ѿ�ֹͣ!");
  qdetach();
  exit(sig);
}

/**************************************************************
 ** ������:   swParse()
 ** ��  ��:   �����ļ��������
 ** ��  ��:   ͯ����
 ** ��������: 2001.05.23
 ** ����޸�����:2001.05.23
 ** ������������: _swGetOneField()
 ** ȫ�ֱ���:  
 ** ��������:       
 ** ����ֵ: �ɹ�:SUCCESS  ʧ��:FAIL
***************************************************************/

int swParse(char *alBuf,char *alMode ,union filep *ulFilep)
{
   char alStr[7][51];
   int i;
   
   swVdebug(4,"S0760: [��������] swParse()");
   for(i = 0;i<7;i++)
     _swGetOneField(alBuf,i+1,alStr[i],',');

   if ( alMode[0] == cFTPPUT || alMode[0] == cFTPGET)
   {
       strcpy(ulFilep->sFileftp.aHost,alStr[0]);
       strcpy(ulFilep->sFileftp.aUser,alStr[1]);
       strcpy(ulFilep->sFileftp.aPasswd,alStr[2]);
       strcpy(ulFilep->sFileftp.aLocalpath,alStr[3]);
       strcpy(ulFilep->sFileftp.aRempath,alStr[4]);
       strcpy(ulFilep->sFileftp.aTranmode,alStr[5]);
       ulFilep->sFileftp.iOvertime=atoi(alStr[6]);
       swVdebug(4,"S0770: [��������] swParse()������=0");
       return(SUCCESS);
   }
   else if(alMode[0] == cQPUT || alMode[0] == cQGET)
   {
      ulFilep->sFileQ.iGroup=atoi(alStr[0]);
      strcpy(ulFilep->sFileQ.aLocalpath,alStr[1]);
      strcpy(ulFilep->sFileQ.aRempath,alStr[2]);
      ulFilep->sFileQ.iPacksize=atoi(alStr[3]);
      ulFilep->sFileQ.iOvertime=atoi(alStr[4]);
      ulFilep->sFileQ.iSafnum=atoi(alStr[5]);
      return(SUCCESS);
   }
   else return(FAIL);

}

/**************************************************************
 ** ������:   swAlarm()
 ** ��  ��:   ��ʱ����
 ** ��  ��:   ͯ����
 ** ��������: 2001.05.25
 ** ����޸�����:2001.05.25
 ** ������������:   
 ** ȫ�ֱ���:   psgPreunpackbuf,agFilecode 
 ** ��������:       
 ** ����ֵ:
***************************************************************/
void swAlarm( )
{
   signal(SIGALRM,SIG_IGN);
   swVdebug(0,"S0780: [����/����] �ļ����䳬ʱ"); 
   strcpy(agFilecode,"-3");
   /* ���Ĵ�����͸�·�� */
   swPackandqwrite( );
   exit(-1);
} 

/**************************************************************
 ** ������:   swPackandqwrite
 ** ��  ��:   ������ͱ��ĵ�·������
 ** ��  ��:   ͯ����
 ** ��������: 2001.05.25
 ** ����޸�����:2001.05.25
 ** ������������:   
 ** ȫ�ֱ���:   psgPreunpackbuf 
 ** ��������:       
 ** ����ֵ:
***************************************************************/
int swPackandqwrite( ) 
{
   short  ilRc;
   short  ilPriority=0;          /* �������:���ȼ� */
   short  ilClass=0;             /* �������:��� */
   short  ilType=0;              /* �������:���� */

   swVdebug(4,"S0790: [��������] swPackandqwriter()");
   /* ���� _FILECODE�� */
   ilRc = swFmlset("_FILECODE",strlen(agFilecode),agFilecode,psgPreunpackbuf);
   if(ilRc != SUCCESS)
   {
     swVdebug(1,"S0800: [����/��������] swFmlset()[_FILECODE]�����,������=%d",ilRc);
     exit(FAIL);
   }
   swVdebug(2,"S0810: ���� _FILECODE��[%s]",agFilecode);
 
   /* �������´�� */
   ilRc = swFmlpack(psgPreunpackbuf,sgMsgpack.aMsgbody,
     &(sgMsgpack.sMsghead.iBodylen));
   if ( ilRc == FAIL )
   {
     swVdebug(1,"S0820: [����/��������] swFmlpack()����FML����ʧ��");
     _swMsgsend( 302016, NULL );
     exit(FAIL);
   }
   swVdebug(2,"S0830: �������´��"); 
 
   /* �����ķ��͵�·������ */
   if ( ( ilRc = qwrite2( (char *)&sgMsgpack,
       sgMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
       iMBROUTER,ilPriority,ilClass,ilType )) != SUCCESS)
   {
     swVdebug(0,"S0840: [����/��������] qwrite2()���ͱ�����·���������,������=%d",ilRc);
     swMberror(ilRc, "����:qwrite��·�������!");
     exit(FAIL);
   }
   swVdebug(0,"S0850: ���ͱ�����·������ɹ�");
   if (cgDebug>=2) 
     swDebugfml( (char *)&sgMsgpack);
   swVdebug(4,"S0860: [��������] swPackandqwriter()������=0");  
   return(SUCCESS); 
} 
/**************************************************************
 ** ������:   swClrMB1()
 ** ��  ��:   ѡ����������б���
 ** ��  ��:   ͯ����
 ** ��������: 2001.05.23
 ** ����޸�����:2001.05.23
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: �ɹ���SUCCESS
***************************************************************/
int swClrMB1(short ilOrgqid,short ilClass,short ilType)
{
  short ilRn;
  unsigned int ilMsglen;
  short ilPriority;
  char slMsgbuf[iMSGMAXLEN];
  
  swVdebug(4,"S0870: [��������] swClrMB1(%d,%d,%d)",ilOrgqid,ilClass,ilType);
  while(1)
  {
    ilMsglen = iMSGMAXLEN;
    ilPriority = 0;

    ilRn = qreadnw( (char *)&slMsgbuf, &ilMsglen, &ilOrgqid,&ilPriority, 
      &ilClass, &ilType, 1 );
    if( (ilRn == BMQ__NOMOREMSG) || (ilRn == BMQ__TIMEOUT)) 
      break;
    else if (ilRn)
      return(ilRn);
  }
  swVdebug(4,"S0880: [��������] swClrMB1()������=0");
  return(SUCCESS);
}

