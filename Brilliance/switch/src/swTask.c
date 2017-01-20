/****************************************************************/
/* ģ����    ��TASK                                           */
/* ģ������    ���������                                       */
/* �� �� ��    ��V4.3.0                                         */
/* ��    ��    ��                                               */
/* ��������    ��2001/7/18                                      */
/* ����޸����ڣ�2001/8/8                                       */
/* ģ����;    ������ƽ̨����,������������̵�״̬,���쳣������ */
/* ��ģ���а������º���������˵����                             */
/*               (1)void main();                                */
/*               (3)int  swExecSubProcess(); �����ӽ���         */
/*               (4)int  swChkMailbox;       �������           */
/*               (5)int  swTaskmanage;       �����������       */
/*               (6)int  swTasktimer;        ��ʱ�������ر����� */
/*               (7)int  swChkSigkill();     ���ر��ź�       */ 
/*               (8)void swQuit();           �ر�ƽ̨����       */
/****************************************************************/
#define NOSQL

/* switch���� */
#include "switch.h"
#include <setjmp.h>
#include "swNdbstruct.h"
#include "swShm.h"

int swChkSigkill();
int swTasktimer();
int swTaskmanage();
int swChkMailbox();
int swExecSubProcess(int alFlag,struct swt_sys_task sSwt_sys_task);
int Parse(char *buf,char args[][101]);

/* �������� */
#define  aINSTANCENAME  "swTask"
#define SHMNOTFOUND	100     /* û�ҵ������ڴ��¼ */

/* �������� */
/*char  *agargs[50];*/ /* deleted by fzj at 2002.03.04 */
char agargs[15][101];  /* added by fzj at 2002.03.04 */
char *execvpargs[10];  /* added by fzj at 2002.03.04 */

/* ����ԭ�Ͷ��� */
void swSigcld(int iSig);
void swQuit(); 
 
/**************************************************************
 ** ������      �� main
 ** ��  ��      ��������
 ** ��  ��      ��  
 ** ��������    �� 
 ** ����޸����ڣ�2001/09/10 
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/
int main(int argc,char *argv[])
{
  short ilRc;                                        /* ����ֵ */   
  short ilCount;                                     /* ��¼�� */
  FILE *plPopenfile;
  int  i,j;                                          /* ������ */
  struct swt_sys_task *pslSwt_sys_task;/* add by nh 20020807 */
  FILE *pp;                            /* add by nh 20020925 */
  char alName[20],alCmd[100];          /* add by nh 20020925 */


  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swTask Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swTask")) exit(FAIL);

  /* ���õ��Գ������� */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strncpy(agDebugfile, "swTask.debug", sizeof(agDebugfile));

  /* ȡ��DEBUG��־ */
  if ((cgDebug = _swDebugflag("swTask")) == FAIL)
  {
    fprintf(stderr,"�޷�ȡ��DEBUG��־!\n");
    exit(FAIL);
  } 

  /* add by nh 20020925 */
  pp=popen("id -un","r");
  fscanf(pp,"%s",alName);
  pclose(pp);
  sprintf(alCmd,"%s%s%s","ps -u ",alName,"|grep swTask|wc -l");
  swVdebug(2,"S0010: command=[%s]",alCmd);
  /* end add */
  plPopenfile=popen(alCmd,"r");
  fscanf(plPopenfile,"%d",&i);  
  swVdebug(2,"S0020: ��ǰ�û���swTask���̸���Ϊ[%d]",i);
  if (i>1)
  {
    swVdebug(1,"S0030: [����/ϵͳ����] popen()����,errno=%d,swTask�����Ѵ���!",errno);
      pclose(plPopenfile);
    exit(0);
  }
  else
    pclose(plPopenfile);

  ilRc = qattach(iMBTASK);
  if (ilRc)
  {
    swVdebug(1,"S0040: [����/����] qattack()����,������=%d,��ʼ���������",ilRc);
    exit(FAIL);
  }
  /* �ź����� */
  /* 
  signal(SIGTERM, swQuit);              
  signal(SIGCLD, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  */
  for(j=1;j<=64;j++)
    signal(j, SIG_IGN);
  
  signal(SIGSEGV,SIG_DFL); 
  signal(SIGTERM, swQuit);

  /* ��ʼ�������ڴ�ָ�� */
  ilRc = swShmcheck();  
  if ( ilRc != SUCCESS )
  {
    swVdebug(0,"S0050: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    exit(FAIL);
  }

  swVdebug(2,"S0060: ��ʼ�������ڴ�ָ��ɹ�");
 
  /* �ӹ����ڴ���ȡ�����������¼ */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_all_p(pslSwt_sys_task, &ilCount);*/
  ilRc = swShmselect_swt_sys_task_all_p(&pslSwt_sys_task, &ilCount);
  if ( ilRc != SUCCESS )
  {
    swVdebug(1,"S0070: [����/�����ڴ�] ���������ڴ�ȡ������¼����");
    exit(FAIL);
  } 
  
  /* ��������������� */
  for ( i=0; i < ilCount; i++ )
  {
    memset(&sgSwt_sys_task, 0x00, sizeof(struct swt_sys_task));
    memcpy(&sgSwt_sys_task, &(pslSwt_sys_task[i]), sizeof(struct swt_sys_task));
    /* �ж������Ƿ���� */
    if (sgSwt_sys_task.task_use[0] !=cTRUE ||\
        strlen(sgSwt_sys_task.task_timer) || \
        sgSwt_sys_task.task_flag !=1 )
    {
      sgSwt_sys_task.pid = -1;
      sgSwt_sys_task.restart_num = 0;
      sgSwt_sys_task.start_time = 0; 
      sgSwt_sys_task.task_status[0] = cTASKDOWN; 
    }
    else 
    {
      if ( ( sgSwt_sys_task.pid > 0 ) && ( kill(sgSwt_sys_task.pid,0) == 0 ) )
      {
	kill(sgSwt_sys_task.pid, 9); 
        swVdebug(2,"S0080: kill���� [%s]",sgSwt_sys_task.task_name );
       }
      sgSwt_sys_task.task_status[0] = cTASKRUNING; 
      sgSwt_sys_task.restart_num = 0;
      sgSwt_sys_task.start_time = 0;
      sgSwt_sys_task.pid = -1;      
      if ( sgSwt_sys_task.task_flag != 1 )
      {
        sgSwt_sys_task.task_flag = 0;
      }  
    }

    /* ���¹����ڴ�������״̬ */
    ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name, sgSwt_sys_task);
    if ( ilRc != SUCCESS ) 
    {
      swVdebug(1,"S0090:  [����/�����ڴ�] �����ڴ�������[%s]δ�ҵ�",sgSwt_sys_task.task_name); 
      continue; 
    }
  }

  /* ѡ����������״̬Ϊ cTASKRUNING �ļ�¼,�� start_id ���� */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_mrec_status_t("1", pslSwt_sys_task,&ilCount);*/
  ilRc = swShmselect_swt_sys_task_mrec_status_t("1", &pslSwt_sys_task,&ilCount);
  if ( ilRc != SUCCESS )
  {
    swVdebug(1,"S0100: [����/�����ڴ�] ���������ڴ����");
    swQuit( );
  } 
  
  /* ��������״̬Ϊ cTASKRUNING ��������� */
  for ( i = 0; i < ilCount; i++ )
  {
    memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
    memcpy(&sgSwt_sys_task, &pslSwt_sys_task[i], sizeof(struct swt_sys_task));
    
    _swTrim(sgSwt_sys_task.task_name);
    _swTrim(sgSwt_sys_task.task_file);
    Parse (sgSwt_sys_task.task_file, agargs);

    /* �����ӽ��� */
    swExecSubProcess( 1,sgSwt_sys_task );  
  }
  swVdebug(2,"S0110: ��顢����ƽ̨���̳ɹ�!");
 
  /* �� SIGCLD �ź� */
  signal(SIGCLD, swSigcld ); 

  for(;;) 
  {
    /* ���³�ʼ�������ڴ�ָ�� */
    ilRc = swShmcheck(); 
    if (ilRc != SUCCESS)
    {
      swVdebug(0,"S0120: [����/�����ڴ�] ��鹲���ڴ����");
      swQuit();
    }

    /* �Է�������ʽ������䣬���� Begin and End ���� */
    swChkMailbox(); 
  
    /* �����������  */
    /*
    swTaskmanage();
    */
 
    /* ����ʱ�������ر����� */
    /*
    swTasktimer();
    */
  }
}

/**************************************************************
 ** ������      :  swChkMailbox
 ** ��  ��      :  ��������������,����begin��end����
 ** ��  ��      :
 ** ��������    :
 ** ����޸�����: 2001/08/15
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :  SUCCESS , FAIL
***************************************************************/
int swChkMailbox()
{
  unsigned int ilMsglen, ilOrgqid, ilPriority;
  short  ilClass, ilType;
  short  ilSigkill;
  short  ilRc;
  char   alTaskname[51];
  long   llPid;
  struct  msgpack  slMsgpack;  /* ���Ķ��� */

  ilMsglen = iMSGMAXLEN;
  ilPriority = 0;
  ilClass = 0;
  ilType = 0;

  swVdebug(4,"S0130: [��������] swChkMailbox()");

  ilRc =  qreadnw((char *)&slMsgpack, &ilMsglen, &ilOrgqid, \
                &ilPriority, &ilClass, &ilType, iTASKTIMEOUT);
  if( ilRc == SUCCESS)  /* �б��� */
  {
    
    memset(alTaskname, 0x00, sizeof(alTaskname));
    memcpy(alTaskname, slMsgpack.aMsgbody, slMsgpack.sMsghead.iBodylen);
    slMsgpack.aMsgbody[slMsgpack.sMsghead.iBodylen]='\0';
    switch(slMsgpack.sMsghead.lCode)
    {
      case 701:            /* �������� */
        memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
        ilRc = swShmselect_swt_sys_task(slMsgpack.aMsgbody, &sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0140: [����/�����ڴ�] ���� [%s] ������!", sgSwt_sys_task.task_name);
          return(FAIL); 
        }

        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);

        if (sgSwt_sys_task.task_use[0] != '1')
        {
          swVdebug(1,"S0150: [����/����] [%s]�����趨Ϊ��ʼ������",sgSwt_sys_task.task_name);
          return(FAIL);
        }

        if (sgSwt_sys_task.pid <= 0)
        {
          _swTrim(sgSwt_sys_task.task_name);
          _swTrim(sgSwt_sys_task.task_file);
          Parse(sgSwt_sys_task.task_file, agargs);
          sgSwt_sys_task.restart_num = 0;

          /* �����ӽ��� */
          swExecSubProcess( 2,sgSwt_sys_task );
        }
        break;

      case 702:      /* �رս��� */
        memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
        ilRc = swShmselect_swt_sys_task(alTaskname, &sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0160: [����/�����ڴ�] ���� [%s] ������!", sgSwt_sys_task.task_name);
          return(FAIL);
        }
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        sgSwt_sys_task.restart_num = 0;
        sgSwt_sys_task.start_time = 0;

        if (sgSwt_sys_task.pid > 0)
        {
          /* ֹͣ���� */
          ilSigkill = swChkSigkill();
          /* sleep(sgSwt_sys_task.stop_wait);*/
          swVdebug(2,"S0170: END ����[%s] ", sgSwt_sys_task.task_name);
 
          llPid = sgSwt_sys_task.pid;
          sgSwt_sys_task.pid = -1;
          strcpy(sgSwt_sys_task.task_status, "4");

/*add by gxz 2001.09.11 Begin */
          if ( sgSwt_sys_task.task_flag == 9 )
            sgSwt_sys_task.task_flag=0;
/*add by gxz 2001.09.11 End */

          /* ���¹����ڴ�����״̬ */
          ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
                 sgSwt_sys_task);
          if (ilRc != SUCCESS)
          {
            swVdebug(1,"S0180: [����/�����ڴ�] ���¹����ڴ�����[%s]״̬ʧ��",\
              sgSwt_sys_task.task_name); 
            break;
          }
          kill(llPid, ilSigkill);
        }
        break;
      default: 
        swVdebug(1,"S0190: [����/����] �յ�δ֪�����[%ld]", slMsgpack.sMsghead.lCode); 
          break;
    } 
  }
  return(0);
}

/**************************************************************
 ** ������       :  swTaskmanage
 ** ��  ��       :  �����������
 ** ��  ��       :  ������
 ** ��������     :  2001/09/10
 ** ����޸����� : 
 ** ������������ :
 ** ȫ�ֱ���     :
 ** ��������     :
 ** ����ֵ       :  SUCCESS , FAIL
**************************************************************/
int swTaskmanage()
{
  short ilQid,ilRc;
  short ilSigkill;                                   /* �رս����ź� */
  long  llPid;
  struct mbinfo slMbinfo;

  swVdebug(4,"S0200: [��������] swTaskmanage()");

  ilQid=1;
  while((ilRc = bmqGetmbinfo(ilQid,&slMbinfo)) != 100)
  {
    if ( ilRc == -1 )
    {
      swVdebug(1,"S0210: [����/����] bmqGetmbinfo()����,������=%d",ilRc);
      return(FAIL);
    }
    
    if ( slMbinfo.iStatus == 1 )
    {
      memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
      ilRc=swShmselect_swt_sys_task_qid( ilQid, 9, &sgSwt_sys_task );
      if ( ilRc == 0 )
      {
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        sgSwt_sys_task.restart_num = 0;
        sgSwt_sys_task.start_time = 0;

        if (sgSwt_sys_task.pid > 0)
        {
          /* ֹͣ���� */
          ilSigkill = swChkSigkill();
          llPid = sgSwt_sys_task.pid;
          swVdebug(2,"S0220: END ����[%s] ", sgSwt_sys_task.task_name);
 
          sgSwt_sys_task.pid = -1;
          strcpy(sgSwt_sys_task.task_status, "4");
          sgSwt_sys_task.task_flag =0;

          /* ���¹����ڴ�����״̬ */
          ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
               sgSwt_sys_task);
          if (ilRc != SUCCESS)
          {
            swVdebug(1,"S0230: [����/�����ڴ�] ���¹����ڴ�����״̬ʧ��");
          }
          kill(llPid, ilSigkill);
        }
      }
    }
    else if ( slMbinfo.iStatus == 3 )
    {
      memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
      ilRc=swShmselect_swt_sys_task_qid( ilQid, 0, &sgSwt_sys_task );
      if ( ilRc == 0 )
      {
   /*     if (sgSwt_sys_task.task_use[0] != '1')
        {
          swVdebug(2,"S0240: [%s] �����趨Ϊ��ʼ������!",
            sgSwt_sys_task.task_name);
          break;
        }
   */
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        Parse(sgSwt_sys_task.task_file, agargs);
        swExecSubProcess( 2,sgSwt_sys_task );
      }
    }
    ilQid++;
  }

  swVdebug(4,"S0250: [��������] swTaskmanage()������=0");
  return(SUCCESS);
}  

/**************************************************************
 ** ������       :  swTasktimer
 ** ��  ��       :  ��������Ķ�ʱ�������ر��
 ** ��  ��       :
 ** ��������     :
 ** ����޸����� :  2001/08/15
 ** ������������ :
 ** ȫ�ֱ���     :
 ** ��������     :
 ** ����ֵ       :  SUCCESS , FAIL
**************************************************************/
int swTasktimer()
{
  short  i,ilSigkill;
  short  ilRc;
  char   alResult[iFLDVALUELEN + 1]  ;
  unsigned int ilLength;
  short  ilCount;
  /* del by nh 20020807
  struct swt_sys_task pslSwt_sys_task[iMAXTASKREC];*/
  struct swt_sys_task *pslSwt_sys_task;/* add by nh 20020807*/
  
  swVdebug(4,"S0260: [��������] swTasktimer()");

  /* ���� task_use = "1" �ļ�¼ */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_mrec_use_t("1",pslSwt_sys_task,&ilCount);*/
  ilRc = swShmselect_swt_sys_task_mrec_use_t("1",&pslSwt_sys_task,&ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0270: [����/�����ڴ�] �ڹ����ڴ��в��ҿ����������!");
    swQuit();
  }
  
  /* ��ѯ����� */
  for (i=0; i < ilCount; i++)
  {
    memcpy(&sgSwt_sys_task,&pslSwt_sys_task[i],sizeof(struct swt_sys_task));
    
    _swTrim(sgSwt_sys_task.task_timer);

    /* �������״̬  */
    if (sgSwt_sys_task.pid > 0)
    {
      /* kill(pid,0)������״̬,ˢ������� */
      ilRc = kill(sgSwt_sys_task.pid, 0);
      if ( ilRc ) 
      {
        strcpy(sgSwt_sys_task.task_status, "4");
        sgSwt_sys_task.pid = -1;

        /* add by gxz 2001.09.11 Begin */
        if ( sgSwt_sys_task.task_flag != 1 )
          sgSwt_sys_task.task_flag = 0;
        /* add by gxz 2001.09.11 End */

        /* ���¹����ڴ� */
        ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
                 sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0280: [����/�����ڴ�] �����ڴ�������[%s]δ�ҵ�",\
            sgSwt_sys_task.task_name); 
          continue; 
        }
      }
    }

    if (sgSwt_sys_task.task_timer[0]) /* ��ʱ�������ر� */
    {
      ilRc = _swExpress(sgSwt_sys_task.task_timer, alResult,&ilLength);
      if (ilRc == FAIL)
      {
        swVdebug(1,"S0290: [����/��������] _swExpress()����,������=%d,��ʱ�������ʽ[%s]�������",ilRc,sgSwt_sys_task.task_timer);
        continue ;
      }

      /* ������������ */
      if ((alResult[0] == '1') && (sgSwt_sys_task.pid <= 0))
      {
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        Parse(sgSwt_sys_task.task_file, agargs);
/*        signal(SIGCLD,SIG_IGN);*/

        /* �������񣬸�������� */
        swExecSubProcess( 2,sgSwt_sys_task );
/*        signal(SIGCLD,swSigcld);*/
        continue;
      }
      if (alResult[0] == '0' && sgSwt_sys_task.pid > 0) /* ����ر����� */
      {
        swVdebug(2,"S0300: ��ʱ�رս���[%s]", sgSwt_sys_task.task_name);
 
        /* �ر����� */
        ilSigkill = swChkSigkill();
/*        signal(SIGCLD, SIG_IGN);*/
        kill(sgSwt_sys_task.pid, ilSigkill);
        sleep(sgSwt_sys_task.stop_wait);
  /*      signal(SIGCLD, swSigcld);*/

        sgSwt_sys_task.pid = -1;
        strcpy(sgSwt_sys_task.task_status, "4");
        sgSwt_sys_task.restart_num = 0; /* very important */
        sgSwt_sys_task.start_time = 0; /* very important */

        /* ���¹����ڴ� */
        ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
            sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0310: [����/�����ڴ�] �����ڴ�������[%s]δ�ҵ�",sgSwt_sys_task.task_name); 
          continue; 
        }
      }
    } /* end ��ʱ�������ر� */
  } /* end for */
  swVdebug(4,"S0320: [��������] swTasktimer()������=0");
  return( SUCCESS);
} 
     
/**************************************************************
 ** ������:  short swExecSubProcess( )
 ** ��  ��       :  ����ƽ̨����
 ** ��  ��       :
 ** ��������     :
 ** ����޸����� :  2001/09/05
 ** �޸���       �� ������
 ** ������������ :
 ** ȫ�ֱ���     :
 ** ��������     :  1--������ƽ̨ʱ    2--������ʱ 
 ** ����ֵ       :  SUCCESS = 0 , FAIL = -1 
***************************************************************/
int swExecSubProcess( int alFlag,struct swt_sys_task sSwt_sys_task  )
{
  short i,ilRc;
  short ilResult;
  long  llBegintime;
  long  llPid;

  swVdebug(4,"S0330: [��������] swExecSubProcess(%d)",alFlag);

  /*  ִ���ļ������ڻ���ִ��Ȩ�� */
  if ( (access(agargs[0], F_OK)) == -1 ||  (access(agargs[0], X_OK)) == -1 )
  {
    swVdebug(1,"S0340: [����/����] Ӧ�ó���[%s]�޷�����!",agargs[0]);
    strcpy(sSwt_sys_task.task_status, "5");
    sSwt_sys_task.start_time = 0;
    sSwt_sys_task.pid = -1;

    /* ���¹����ڴ� */
    ilRc = swShmupdate_swt_sys_task(sSwt_sys_task.task_name, sSwt_sys_task);
    if (ilRc != SUCCESS)
    {
      swVdebug(1,"S0350: [����/�����ڴ�] �����ڴ�������[%s]δ�ҵ�",sSwt_sys_task.task_name); 
      return(FAIL);
    }
    return(FAIL);
  }
  /* �����ӽ��� */
  if ((llPid = fork()) < 0)  
  {
    swVdebug(1,"S0360: [����/ϵͳ����] fork()����,errno=%d,fork�ӽ���ʧ��! ",errno);
    /* swQuit(); delete by szhengye 2001.10.30 */
    return(-1);
  }
  if (llPid == 0)  
  { 
    swVdebug(2,"agargs=[%s][%s][%s][%s]",agargs[0],agargs[1],agargs[2],agargs[3]);
    /* execvp( *agargs, agargs ); */ /* delete by fzj at 2002.03.04 */
    /* begin of added by fzj at 2002.03.04 */
    for (i=0;i<15;i++) 
    {
      if (agargs[i][0] == '\0')
      {
      	execvpargs[i] = NULL;
      	break;
      }	
      execvpargs[i] = (char *)(&agargs[i][0]);
    }  
    execvp(*execvpargs, execvpargs);
    /* end of added by fzj at 2002.03.04 */
    exit(0);   
  }
  /* �������Ƿ������ɹ� */
  if ( alFlag == 1 )
    sleep(sSwt_sys_task.start_wait);
  else 
    sleep(1);

  ilRc = kill(llPid, 0);
  if( ilRc == -1 )
  {
    sSwt_sys_task.task_status[0] = cTASKDOWN;
    sSwt_sys_task.start_time = 0; 
    swVdebug(1,"S0370: [����/����] ���� [%s] ʧ��", sSwt_sys_task.task_name);
    ilResult = FAIL;
  }   
  else      /* ִ�н��̳ɹ����޸�����״̬ */
  {
    time(&llBegintime); 
    sSwt_sys_task.pid = llPid;
    sSwt_sys_task.task_status[0] = cTASKRUNED;

/* add by gxz 2001.09.11 Begin */
    if ( alFlag != 1 )
    {
      if ( sSwt_sys_task.task_flag != 1 )
      {
        sSwt_sys_task.task_flag =9;
      }
    }
/* add by gxz 2001.09.11 End */
       
    sSwt_sys_task.start_time = llBegintime; /* very important */
    swVdebug(2,"S0380: ����[%s]�ɹ�,PID=%ld",sSwt_sys_task.task_name,llPid);
    ilResult = SUCCESS;
  }

  /* ���¹����ڴ� */
  ilRc = swShmupdate_swt_sys_task(sSwt_sys_task.task_name, sSwt_sys_task);
  if ( ilRc != SUCCESS )
  {
    swVdebug(1,"S0390: [����/�����ڴ�] �����ڴ�������[%s]δ�ҵ�",\
      sSwt_sys_task.task_name); 
    return(FAIL);
  }
  swVdebug(4,"S0400: [��������] swExecSubProcess()������=%d",ilResult);
  return(ilResult); 
}


/**************************************************************
 ** ������:  void swSigcld()
 ** ��  ��:  ���� SIGCLD �ź� 
 ** ��  ��:
 ** ��������:
 ** ����޸�����: 2001/08/15
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  
***************************************************************/
void swSigcld(int iSig)
{
  pid_t llPid;
  int ilStat;
  int ilRc;
  long llPidcld;
  struct swt_sys_task slSwt_sys_task;

  swVdebug(4,"S0410: [��������] swSigcld(%d)",iSig);

/*  signal(SIGCLD, SIG_IGN);*/
  swDebug("szhengye:swSigcld:%d",iSig);

  while ((llPid = waitpid(0, &ilStat, WNOHANG)) > 0)
  {
    swVdebug(2,"S0420: �ӽ����˳� Waitpid(),pid=[%ld]",llPid);
    llPidcld = llPid;
    /* �������жϵ����� */

    ilRc = swShmselect_swt_sys_task_pid(llPidcld, &slSwt_sys_task);
    if (ilRc != SUCCESS) 
    {
      swVdebug(2,"S0430: ���� PID = [%ld] ������ʧ�� !", llPidcld);
      continue;
    }
    
    /* �ж��Ƿ�֧������ */
    if (slSwt_sys_task.restart_flag[0] == '1') 
    { 
      /*�Ƚ��Ƿ�ﵽ����������*/ 
      if (slSwt_sys_task.restart_max > slSwt_sys_task.restart_num) 
      {
        slSwt_sys_task.restart_num++; 

        /* �������̳���·�� */
        _swTrim(slSwt_sys_task.task_name); 
        _swTrim(slSwt_sys_task.task_file);
        Parse(slSwt_sys_task.task_file, agargs);

        /* �����ӽ��� */
        swExecSubProcess( 2,slSwt_sys_task );  
      }
      /* fzj: 2002.9.17: ������������ﵽ������ʱӦ��״̬��DOWN */
      else
      {
        slSwt_sys_task.pid = -1;
        slSwt_sys_task.start_time = 0; 
        slSwt_sys_task.task_status[0] = cTASKDOWN; 
        ilRc=swShmupdate_swt_sys_task(slSwt_sys_task.task_name,slSwt_sys_task);
        if (ilRc) 
        {
          swVdebug(1,"S0440: [����/�����ڴ�] �����ڴ�������[%s]δ�ҵ�",\
            slSwt_sys_task.task_name); 
          continue;
        }
      }
      /**/
      /* add by ������ 2001.09.05 Begin */
      /* ����ʱ�������ر����� */
      /*
      swTasktimer();
      */
      /* add by ������ 2001.09.05 End */
    }
    else /* ��֧���Զ����� */
    {
      slSwt_sys_task.pid = -1;
      slSwt_sys_task.start_time = 0;
      strcpy(slSwt_sys_task.task_status, "5");
      swVdebug(2,"S0450: [%s] �趨Ϊ�������� ���Զ�����",\
        slSwt_sys_task.task_name);

      /* add by gxz 2001.09.11 Begin */
      if ( slSwt_sys_task.task_flag != 1 )
        slSwt_sys_task.task_flag = 0;
      /* add by gxz 2001.09.11 End */

      /* ���¹����ڴ� */
      ilRc = swShmupdate_swt_sys_task(slSwt_sys_task.task_name, slSwt_sys_task);
      if (ilRc) 
      {
        swVdebug(1,"S0460: [����/�����ڴ�] �����ڴ�������[%s]δ�ҵ�",\
          slSwt_sys_task.task_name); 
        continue;
      }
    }
  }  /* end of while */
  signal( SIGCLD, swSigcld );
}

/**************************************************************
 ** ������:  int Parse()
 ** ��  ��:
 ** ��  ��:
 ** ��������:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  SUCCESS , FAIL
***************************************************************/
/* begin of deleted by fzj at 2002.03.04 */
/*
int Parse(buf,args)
char *buf;
char * * args;
{
  char *buf1;

  swVdebug(4,"S0470: [��������] Parse()");

  buf1 = (char *) malloc(1024); 
  memcpy(buf1, buf, 1023);
  
  while (*buf1 != '\0')
  {
    while ((*buf1 == ' ') || (*buf1 == '\t') || (*buf1 == '\n'))
      *buf1++ = '\0';

    * args++ = buf1;

    while ((*buf1 != '\0') && (*buf1 != ' ') && (*buf1 != '\t') \
           && (*buf1 != '\n'))
      buf1++;
  }
  *args = '\0';
  return(0);
}
*/
/* end of deleted by fzj at 2002.03.04 */

/* begin of added by fzj at 2002.03.04 */
int Parse(char *buf, char args[][101])
{
  int i,ilLen;
  char *plBuf,*plArg;

  for(i=0;i<10;i++) args[i][0] = '\0';

  i = 0;
  plBuf = buf;
  while(*plBuf != '\0')
  {
    while(*plBuf==' ' || *plBuf=='\t' || *plBuf=='\n')
    {
      plBuf++;
      continue;
    }
    plArg = plBuf;
    ilLen = 0;
    while(*plBuf!=' ' && *plBuf!='\t' && *plBuf!='\n')
    {
      if(*plBuf=='\0') break;
      ilLen++;
      plBuf++;
      continue;
    }
    memcpy(args[i],plArg,ilLen);
    args[i][ilLen] = '\0';
    i++;
  }
  return(0);
}
/* end of added by fzj at 2002.03.04 */

/**************************************************************
 ** ������:  int swChkSigkill()
 ** ��  ��:  ȡ���趨ƽ̨�����е� kill_id
 ** ��  ��:
 ** ��������:
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:  ���ص�ǰ��ƽ̨����֮ kill_id �趨ֵ�������Ϊ 9
***************************************************************/
int swChkSigkill()
{
  short ilRc;

  swVdebug(4,"S0480: [��������] swChkSigkill()");

  if ((sgSwt_sys_task.kill_id == 0))
  {
    ilRc = 9;
  }
  else
  {
    ilRc = sgSwt_sys_task.kill_id;
  }

  swVdebug(4,"S0490: [��������] swChkSigkill()������=%d",ilRc);
  return(ilRc);
}

/**************************************************************
 ** ������:  void swQuit()
 ** ��  ��:  �ر�ǰ�ý���
 ** ��  ��:
 ** ��������:
 ** ����޸�����: 2001/08/15
 ** ������������:
 ** ȫ�ֱ���: sgSwt_sys_task
 ** ��������:
 ** ����ֵ:
***************************************************************/
void   swQuit()                          
{
  short  ilSigkill;
  short  ilRc;
  short  i;
  /* del by nh 20020807
  struct swt_sys_task pslSwt_sys_task[iMAXTASKREC];*/
  struct swt_sys_task *pslSwt_sys_task;/* add by nh 20020807*/
  short ilCount;  

  signal(SIGCLD, SIG_IGN);
  swVdebug(1,"S1111: �����������˳�");
  
  qdetach();
 
  /* ѡ�����м�¼���� stop_id ���� */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_all_p(pslSwt_sys_task, &ilCount);*/
  ilRc = swShmselect_swt_sys_task_all_p(&pslSwt_sys_task, &ilCount);
  if (ilRc)
  {
    swVdebug(2,"S0500: ���������ڴ���������");
    exit (-1);
  }
  for (i = 0; i < ilCount; i++)
  {
    memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
    memcpy(&sgSwt_sys_task, &pslSwt_sys_task[i], sizeof(struct swt_sys_task));
    /* �ر���������ƽ̨���� */
    if (sgSwt_sys_task.pid > 0)
    {
      _swTrim(sgSwt_sys_task.task_name);
      _swTrim(sgSwt_sys_task.task_file);
      Parse(sgSwt_sys_task.task_file, agargs);

      ilRc = kill(sgSwt_sys_task.pid, 0);
      if ( ilRc == 0)
      {
	ilSigkill = swChkSigkill();
	kill(sgSwt_sys_task.pid, ilSigkill); 
	swVdebug(2,"S0510: kill [%s] ����", sgSwt_sys_task.task_name);
        sleep ( sgSwt_sys_task.stop_wait );
      }
    }

    strcpy(sgSwt_sys_task.task_status, "4");
    sgSwt_sys_task.pid = -1;
    sgSwt_sys_task.start_time = 0;
    sgSwt_sys_task.restart_num = 0;

/* add by gxz 2001.09.11 Begin */
    if ( sgSwt_sys_task.task_flag != 1 )
      sgSwt_sys_task.task_flag = 0;
/* add by gxz 2001.09.11 End */

    /* ���¹����ڴ�������״̬ */
    ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name, sgSwt_sys_task);
    if (ilRc) 
    {
      swVdebug(1,"S0520: [����/�����ڴ�] ���¹����ڴ�������[%s]״̬����",\
        sgSwt_sys_task.task_name); 
      continue;
    }
  }
  swVdebug(2,"S0530: �رս���ƽ̨�������");
  exit (0);
}         

