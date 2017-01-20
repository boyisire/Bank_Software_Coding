#include "switch.h"
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
EXEC SQL INCLUDE sqlca;
#ifdef DB_POSTGRESQL
EXEC SQL INCLUDE swDbstruct.h;
#endif
#ifdef DB_DB2
EXEC SQL INCLUDE '../inc/swDbstruct.h';
#endif
#ifdef DB_INFORMIX
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_ORACLE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_SYBASE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#else
#include "swDbstruct.h"
#endif
#include "swShm.h"

int Message();

/****add by wanghao 20150416 PSBC_V1.0***/
extern int swShmdelete_swt_rev_saf(long lSaf_id);
extern int swShmselect_swt_rev_saf_all(struct swt_rev_saf *psSwt_rev_saf, short *iCount);

/* add by gengling at 2013.04.17 two lines PSBC_V1.0 */
struct shmidx_s *psmShmidx_s;   /* ��̬�����ڴ�Ŀ¼��ָ�� */
struct shmbuf_s smShmbuf_s;     /* ��̬�����ڴ����ݶ�ָ�� */

#define  TASKNAME       "swTask"

#define  iMSGLEN       80
#define  iMAXTASKCOUNT   100
#define  iMAXRECORD      3000

/**************************************************************
 ** ������      : swListtask
 ** ��  ��      : ��ʾ������Ϣ
 ** ��  ��      : ������
 ** ��������    : 2001/08/23
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swListtask( )
{
  short ilRc;                      /* ������ */
  short ilTotalcount;              /* ���� */
  short ilPagecount;               /* ������ʾ��Ŀ */
  short i;

  /* modify by nh 20020807
  struct swt_sys_task pslSwt_sys_task[iMAXTASKCOUNT];*/
  struct swt_sys_task *pslSwt_sys_task;
  char alState[10];
  char alDisptime[10];             /* ��ʾ��������ʱ�� */
  char alTmp[101];

  swVdebug(4,"S0010: [��������] swListtask()");

  /* ���ݳ�ʼ�� */
  ilTotalcount=0;

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_all_q(pslSwt_sys_task,&ilTotalcount);*/
  ilRc = swShmselect_swt_sys_task_all_q(&pslSwt_sys_task,&ilTotalcount);
  if (ilRc)
  {
    Message("ִ��swShmselect_swt_sys_task_all_q����\n");
    return(FAIL);
  }

  if(ilTotalcount == iMAXTASKCOUNT)
  {
    Message("���������������������[%d],�����\n",iMAXTASKCOUNT);
    swVdebug(1,"S0020: [����/����] �������������������,�����!");
    return(FAIL);
  }

  Message("\n   ������                       �����  ���̺�  ����ʱ��  ״̬   ��������\n");
  Message(  "   ======                       ======  ======  ========  ====   ========\n");
  ilPagecount = 2 ;
  /* ��ȡ��¼ */
  for (i = 0;i<ilTotalcount;i++)
  {
    _swTrim(pslSwt_sys_task[i].task_name); 
    _swTrim(pslSwt_sys_task[i].task_file);

    if(pslSwt_sys_task[i].task_status[0]=='1')
      strcpy(alState, "BEG....");
    else if(pslSwt_sys_task[i].task_status[0]=='2')
      strcpy(alState, "UP");
    else if(pslSwt_sys_task[i].task_status[0]=='3')
      strcpy(alState, "END....");
    else if(pslSwt_sys_task[i].task_status[0]=='4')
      strcpy(alState, "DOWN");
    else if(pslSwt_sys_task[i].task_status[0]=='5')
      strcpy(alState, "ERROR");
    else 
      strcpy(alState, "UNKNOWN");

    swVdebug(3,"S0030: alState=[%s]",alState);
    if(pslSwt_sys_task[i].task_desc[0]!='\0')
    {
      sprintf(alTmp,"%s(%s)",pslSwt_sys_task[i].task_name,
                           pslSwt_sys_task[i].task_desc);
      if(strlen(alTmp)>30)
      {
        memcpy(alTmp+26,"...)",5);
      }
    }
    else
      strcpy(alTmp,pslSwt_sys_task[i].task_name);

    _swTimeLongToChs( pslSwt_sys_task[i].start_time,alDisptime,"HH:MM:SS"); 
    Message("  %-30.30s  %-4d  %-8d%-10.10s%-6.6s %4d|%-4d\n",
      alTmp,
      pslSwt_sys_task[i].q_id,
      pslSwt_sys_task[i].pid,
      alDisptime,
      alState,
      pslSwt_sys_task[i].restart_num,
      pslSwt_sys_task[i].restart_max);
    ilPagecount++;
    if(ilPagecount == 21)
    {
      printf("�밴[RETURN]������...\n");
      if(getchar() == 'q') break;
      Message("\n   ������                       �����  ���̺�  ����ʱ��  ״̬   ��������\n");
      Message("   ======                       ======  ======  ========  ====   ========\n");
      ilPagecount = 2 ;
    }
  }

  Message("   �ܹ���[%d]������\n",ilTotalcount);

  swVdebug(4,"S0040: [��������] swListtask()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swStarttask
 ** ��  ��      : ��������
 ** ��  ��      : ������
 ** ��������    : 
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern  int swStarttask( char *taskname)
{
  char  alDbtname[21];              /* �������� */
  long  llPid;                      /* ���̺� */
  short ilPidstatus;                /* ����״̬ */
  char  alRunfile[100];             /* ���г��� */
  char  alProname[21];              /* ���г������� */
  FILE  *plPopenfile;               /* �ܵ����� */
  char  alFile1[51],alFile2[51],alFile3[51],alFind[51]; 
  short ilFound;                    /* ���ֱ�־ */
  char  alFindTask[100];
  short ilRc;                       /* ������ */
  char  clKey;
 
  struct msgpack slMsgpack;

  swVdebug(4,"S0050: [��������] swStarttask(%s)",taskname);

  llPid = -1;
  ilPidstatus = 0;
  
  _swTrim(taskname);

  if ((strcmp(taskname,"\0") == 0)||(strcmp(taskname,TASKNAME)==0)) 
  {
    printf("ȷ��Ҫ��������ƽ̨��? (Y/N)");
    clKey = getchar();
    if (clKey == 'y' || clKey == 'Y')
      taskname = TASKNAME;
    else 
      return(FAIL); 
  }
  
  memset(alProname,0x00,sizeof(alProname));
  sprintf(alProname, "%s", taskname);

  /* �л���SWITCH_DIRĿ¼ */
  chdir(getenv("SWITCH_DIR"));

  swVdebug(2,"S0060: �л���SWITCH_DIRĿ¼");
  swVdebug(2,"S0070: �鿴swTask�����Ƿ��Ѿ�����");
  /* �鿴swTask�����Ƿ��Ѿ����� */
  sprintf(alFindTask,"ps -e|grep %s",TASKNAME);
  ilFound = 0;
  plPopenfile = popen( alFindTask ,"r");
  while ( feof(plPopenfile) == 0) 
  {
    memset(alFile1,0x00,sizeof(alFile1));
    memset(alFile2,0x00,sizeof(alFile2));
    memset(alFile3,0x00,sizeof(alFile3));
    memset(alFind,0x00,sizeof(alFind));

    fscanf(plPopenfile,"%s %s %s %s",alFile1,alFile2,alFile3,alFind);
    if ( strcmp(alFind,TASKNAME) == 0 )
    {
      ilFound = 1;
      break;
    }
  }
  pclose(plPopenfile);

  if(!strcmp(taskname, TASKNAME))
  {
    if ( ilFound == 1 )         /*����������swTask�Ѵ���*/
    {
      Message("����������[%s]����ϵͳ������!  Pid=[%s]\n", alFind, alFile1);
      return FAIL;
    }

    /*����������swTask������������swTask*/
    memset(alRunfile,0x00,sizeof(alRunfile));
    sprintf(alRunfile,"%s/bin/%s",getenv("SWITCH_DIR"), taskname);

    /* fork����,ִ��swTask���� */
    if( (llPid=fork()) == 0)
    {
      ilPidstatus = execl(alRunfile,alProname,(char * )0 );
      exit(SUCCESS);
    }
    if( ilPidstatus == 0)
    {
      ilRc = 0;
      Message("�����ɹ�!\n");
    }
    else
    {
      Message("����ʧ��!\n");
      ilRc = -1;
    }
    return(ilRc);
  }
  /*������swTask����*/
  if ( ilFound==0 )          /*����ƽ̨����swTaskδ��*/
  {
    Message("����ƽ̨δ�����������𽻻�ƽ̨\n");
    return FAIL;
  }

  /*����������������������Ƿ����*/
  strncpy(alDbtname,taskname,sizeof(alDbtname));

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_sys_task(alDbtname,&sgSwt_sys_task);
  if (ilRc)
  {
    Message("��Ҫ����������[%s]������������в�����\n",alDbtname);
    return(FAIL);
  }
  swVdebug(2,"S0080: ��Ҫ����������������������д���");

  /* ��λ����  */
  if ( qattach( iMBSYSCON ) != 0 )
  {
    swVdebug(1,"S0090: [����/����] qattach()����,������=-1,��λ�����������(iMBSYSCON)");
    return(FAIL);
  }
  
  /* ����ָ������ */
  swVdebug(2,"S0100: ��ʼ��������[%s]...",taskname);

  /* ��������ĸ���������� */
  memset(&slMsgpack, 0x00, sizeof(struct msgpack));
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.lCode = 701;
  slMsgpack.sMsghead.iBodylen = strlen(taskname);
  strcpy(slMsgpack.aMsgbody, taskname);
  ilRc = _qwrite2((char *)&slMsgpack,slMsgpack.sMsghead.iBodylen + 
    sizeof(struct msghead),iMBTASK,0,0,0);

  if (ilRc != SUCCESS) 
  {
    swVdebug(1,"S0110: [����/����] _qwrite2()����,������=-1,���ͱ��ĸ����������ʧ��");
    return (FAIL);
  }

  swVdebug(2,"S0120: ���ͱ��ĸ�����������ɹ�");
  swVdebug(2,"S0130: ����[%s]�����ɹ�!", alProname);
  Message("����[%s]�����ɹ�!\n", alProname);

  /* �ر�����*/
  qdetach();

  swVdebug(4,"S0140: [��������] swStarttask()������=0");
  return(SUCCESS);
}


/**************************************************************
 ** ������      : swStoptask
 ** ��  ��      : ֹͣ����
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern  int  swStoptask( char *taskname )
{
  FILE *plPopenfile;                /* �ܵ����� */
  char  alFile1[51],alFile2[51],alFile3[51],alFind[51]; 

  short ilFound;                    /* ���ֱ�־ */
  char  alFindTask[100];

  short ilRc;                       /* ������ */
  char  clKey;

  struct msgpack slMsgpack;

  swVdebug(4,"S0150: [��������] swStoptask(%s)",taskname);

  _swTrim(taskname);

  if ((strcmp(taskname,"\0") == 0)||(strcmp(taskname,TASKNAME)==0))
  {
    printf("ȷ��Ҫ�˳�����ƽ̨��? (Y/N)");
    clKey = getchar();
    if (clKey == 'y' || clKey == 'Y')
      taskname = TASKNAME;
    else
      return(FAIL);
  }
  /* ��������Ƿ���� */
  ilFound = 0;
  sprintf(alFindTask,"ps -e|grep %s",TASKNAME);
  plPopenfile = popen( alFindTask ,"r");

  while ( feof(plPopenfile) == 0 )
  {
    memset(alFile1,0x00,sizeof(alFile1));
    memset(alFile2,0x00,sizeof(alFile2));
    memset(alFile3,0x00,sizeof(alFile3));
    memset(alFind,0x00,sizeof(alFind));

    fscanf(plPopenfile,"%s %s %s %s",alFile1,alFile2,alFile3,alFind);
    if ( strcmp(alFind,TASKNAME) == 0 )
    {
      ilFound=1;
      break;
    }
  }
  pclose(plPopenfile);

  if ( ilFound ==0 )
  {
    Message("����ƽ̨[%s]��δ����! \n", TASKNAME);
    swVdebug(0,"S0160: [����] ����ƽ̨[%s]��δ����! ", TASKNAME);
    return FAIL;
  }
  if ( strcmp( taskname,TASKNAME ) == 0 )
  {
    kill(atoi(alFile1),SIGTERM);
    Message("����[%s]�˳�ϵͳ!  Pid=[%s]\n", alFind, alFile1);
    swVdebug(0,"����[%s]�˳�ϵͳ!  Pid=[%s]\n", alFind, alFile1);
    return SUCCESS;
  }
  
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_sys_task(taskname,&sgSwt_sys_task);

  if (ilRc)
  {
    Message("��Ҫֹͣ������[%s]������������в�����\n",taskname);
    return(FAIL);
  }
  
  if ( sgSwt_sys_task.pid <=0 ) 
  {
    Message("����[%s]��δ����! \n", taskname);
    swVdebug(0,"S0170: [����] ����[%s]��δ����! ", taskname);
    return FAIL;
  }

  swVdebug(2,"S0180: ��ʼֹͣ����[%s]...",taskname);

  /* ��λ����  */
  if (qattach( iMBSYSCON ) != 0)
  {
    swVdebug(0,"S0190: [����/����] qattach()����,������=-1,��λ�����������(iMBSYSCON)");
    return(FAIL);
  }

  /* ��������ĸ���������� */
  memset(&slMsgpack, 0x00, sizeof(struct msgpack));
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.lCode = 702;
  slMsgpack.sMsghead.iBodylen = strlen(taskname);
  strcpy(slMsgpack.aMsgbody, taskname);

  ilRc = _qwrite2((char *)&slMsgpack,slMsgpack.sMsghead.iBodylen + 
      sizeof(struct msghead),iMBTASK,0,0,0);
  if (ilRc != SUCCESS) 
  {
    swVdebug(1,"S0200: [����/����] _qwrite2()����,������=%d,���ͱ��ĸ����������ʧ��",ilRc);
    swMberror(ilRc,NULL);
    return (FAIL);
  }
  swVdebug(2,"S0210: ���ͱ��ĸ�����������ɹ�");
  Message("����[%s]�رճɹ�!\n",taskname);
  swVdebug(2,"S0220: ����[%s]�رճɹ�!", taskname);

  /* �ر�����*/
  qdetach();

  swVdebug(4,"S0230: [��������] swStoptask()������=0");
  return(SUCCESS);
}


/**************************************************************
 ** ������      : swStop
 ** ��  ��      : ֹͣ����
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern  int  swStop()
{
  FILE *plPopenfile;                /* �ܵ����� */
  char  alFile1[51],alFile2[51],alFile3[51],alFind[51]; 

  short ilFound;                    /* ���ֱ�־ */
  char  alFindTask[100];


  ilFound = 0;
  sprintf(alFindTask,"ps -e|grep %s",TASKNAME);
  plPopenfile = popen( alFindTask ,"r");

  while ( feof(plPopenfile) == 0 )
  {
    memset(alFile1,0x00,sizeof(alFile1));
    memset(alFile2,0x00,sizeof(alFile2));
    memset(alFile3,0x00,sizeof(alFile3));
    memset(alFind,0x00,sizeof(alFind));

    fscanf(plPopenfile,"%s %s %s %s",alFile1,alFile2,alFile3,alFind);
    if ( strcmp(alFind,TASKNAME) == 0 )
    {
      ilFound=1;
      break;
    }
  }
  pclose(plPopenfile);

  if ( ilFound ==0 )
  {
    swVdebug(0,"S0160: [����] ����ƽ̨[%s]��δ����! ", TASKNAME);
    return FAIL;
  }
  kill(atoi(alFile1),SIGTERM);
  swVdebug(1,"����[%s]�˳�ϵͳ!  Pid=[%s]\n", alFind, alFile1);
  return SUCCESS;
}


/**************************************************************
 ** ������      : swConup
 ** ��  ��      : �ö˿�Ӧ�ò�״̬ΪUP
 ** ��  ��      : ������
 ** ��������    : 2001/08/27
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swConup(char *alPortid)
{
  short ilPid;
  short ilRc;       /* ������ */

  swVdebug(4,"S0240: [��������] swConup(%s)",alPortid);

  _swTrim(alPortid);
  if (strcmp(alPortid,"\0") == 0)
  {
    Message("Usage: U   portid\n");
    return(FAIL);
  }
  ilPid = atoi(alPortid);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_sys_queue(ilPid,&sgSwt_sys_queue);

  if(ilRc == SHMNOTFOUND)
  {
    Message("û�ж˿�[%d]�ļ�¼\n",ilPid);
    return(FAIL);
  }
  if(ilRc)
  {
    Message("ѡ��˿ڱ����\n");
    swVdebug(1,"S0250: [����/�����ڴ�] ѡ��˿ڱ����");
    return(FAIL);
  }
  /*�ö˿�Ӧ�ò�״̬ΪUP*/
  ilRc =  swPortset( ilPid, 2, 1 );
  if ( ilRc )
  {
    Message("ǿ�ö˿�[%d]״̬����\n",ilPid);
    swVdebug(1,"S0260: [����/����] ǿ�ö˿�[%d]״̬����",ilPid);
    return(FAIL);
  }
  
  swVdebug(2,"S0270: ����[%d]�˿�ΪUP",ilPid);

  swVdebug(4,"S0280: [��������] swConup()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swCondown
 ** ��  ��      : �ö˿�Ӧ�ò�״̬ΪDOWN
 ** ��  ��      : ������
 ** ��������    : 2001/08/27
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swCondown(char *alPid)
{
  short ilRc;       /* ������ */
  short ilPid;

  swVdebug(4,"S0290: [��������] swCondown(%s)",alPid);

  _swTrim(alPid);
  if (strcmp(alPid,"\0") == 0)
  {
    Message("Usage: D    portid\n");
    return(FAIL);
  }

  ilPid = atoi(alPid);
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }
  ilRc = swShmselect_swt_sys_queue(ilPid,&sgSwt_sys_queue);

  if(ilRc == SHMNOTFOUND)
  {
    Message("û�ж˿�[%d]�ļ�¼\n",ilPid);
    return(FAIL);
  }
  if(ilRc)
  {
    Message("ѡ��˿ڱ����\n");
    swVdebug(1,"S0300: [����/�����ڴ�] ѡ��˿ڱ����");
    return(FAIL);
  }

  ilRc =  swPortset( ilPid, 2, 0 );
  if ( ilRc )
  {
    Message("ǿ�ö˿�[%d]״̬����\n",ilPid);
    swVdebug(1,"S0310: [����/����] ǿ�ö˿�[%d]״̬����",ilPid);
    return(FAIL);
  }

  swVdebug(2,"S0320: ����[%d]�˿�ΪDOWN",ilPid);

  swVdebug(4,"S0330: [��������] swCondown()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swConset
 ** ��  ��      : �ö˿�״̬ΪUP/DOWN
 ** ��  ��      : ������
 ** ��������    : 2001/08/27
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swConset(char *alPid,char *alValue)
{
  short ilRc;       /* ������ */
  short ilPid;
  int   ilNum;
  int   i;
  
  swVdebug(4,"S0340: [��������] swConset(%s,%s)",alPid,alValue);

  _swTrim(alPid);
  if ((strcmp(alPid,"\0") == 0)&&(strcmp(alValue,"\0") == 0))
  {
    Message("Usage: T  portid  value\n");
    return(FAIL);
  }

  /* �ж������״ֵ̬�Ƿ�Ϊ��ȷ��ֵ */ 
  ilNum = strlen(alValue);
  for(i=0;i<ilNum;i++) 
  { 
    if ( (alValue[i]!='0')&&(alValue[i]!='1') )
    {
      Message("�����״ֵ̬�Ƿ�\n");
      return(FAIL);
    }
  } 
     
  ilPid = atoi(alPid);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }
  ilRc = swShmselect_swt_sys_queue(ilPid,&sgSwt_sys_queue);

  if(ilRc == SHMNOTFOUND)
  {
    Message("û�ж˿�[%d]�ļ�¼\n",ilPid);
    return(FAIL);
  }
  if(ilRc)
  {
    Message("ѡ��˿ڱ����\n");
    swVdebug(1,"S0350: [����/�����ڴ�] ѡ��˿ڱ����");
    return(FAIL);
  }
  
  for (i=0;i<ilNum;i++)
  {
    if ( alValue[i] == '0' )
      ilRc =  swPortset( ilPid, i+1, 0 );
    else
      ilRc =  swPortset( ilPid, i+1, 1 );
    if ( ilRc )
    {
      Message("ǿ�ö˿�[%d]״̬����\n",ilPid);
      swVdebug(1,"S0360: [����/����] ǿ�ö˿�[%d]״̬����",ilPid);
      return(FAIL);
    }
  }

  swVdebug(2,"S0370: ����[%d]�˿�ΪDOWN",ilPid);

  swVdebug(4,"S0380: [��������] swConset()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swListtran1
 ** ��  ��      : ��ʾ���ڴ���Ľ�����ˮ
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swListtran1()
{
  short ilRc;       /* ������ */
  char lsTmp1[10],lsTmp2[10],lsTmp3[10];
  char clKey;

  short ilTotalcount;              /* ���� */
  short ilPagecount;               /* ������ʾ��Ŀ */
  short i;
  struct swt_tran_log pslSwt_tran_log[iMAXRECORD];

  swVdebug(4,"S0390: [��������] swListtran1()");

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  ilRc = swShmselect_swt_tran_log_all(pslSwt_tran_log,&ilTotalcount);
  if(ilRc == FAIL)
  {
    Message("ִ��swShmselect_swt_tran_log_allʧ��\n");
    swVdebug(1,"S0400: [����/�����ڴ�] ִ��swShmselect_swt_tran_log_allʧ��");
    return(FAIL);
  }

  /* ��ʾ */
  Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
  Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  for(i=0;i<ilTotalcount;i++)
  {
    _swTimeLongToChs(pslSwt_tran_log[i].tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(pslSwt_tran_log[i].tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(pslSwt_tran_log[i].tran_overtime,lsTmp3,"HH:MM:SS");

    Message( "   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      pslSwt_tran_log[i].tran_id,pslSwt_tran_log[i].tran_status,\
      lsTmp1,lsTmp2,lsTmp3,pslSwt_tran_log[i].q_tran_begin,\
      pslSwt_tran_log[i].tc_tran_begin );
  
    ilPagecount++;
    if(ilPagecount == 20) 
    {
      printf("�밴[RETURN]������...\n");
      clKey = getchar();
      if (clKey == 'q' || clKey == 'Q')  break;
      Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
      Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
      Message("   ============================================================\n");
      ilPagecount = 2;
    }
  }

  Message("   ����[%d]����¼\n",ilTotalcount);

  swVdebug(4,"S0410: [��������] swListtran1()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swListtran2
 ** ��  ��      : ��ѯ��ǰ�����մ���ɹ��Ľ��� 
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swListtran2()
{ 
  short ilRn;       /* ������ */


  swVdebug(4,"S0420: [��������] swListtran2()");

  /* �����ݿ� */
  ilRn=swDbopen();
  if (ilRn) 
  {
    Message("�����ݿ�ʧ��,sqlca.sqlcode=[%d]\n",ilRn);
    swVdebug(0,"S0430: [����/���ݿ�] �����ݿ�ʧ��,sqlca.sqlcode=[%d]",ilRn);
    return(FAIL); 
  }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL DECLARE tran_log_2 CURSOR FOR SELECT  *
    FROM swt_his_tran_log WHERE tran_status = 1 ORDER BY tran_id;
  if(sqlca.sqlcode)
  {
    Message("�����α�ʧ��,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0440: [����/���ݿ�] �����α�ʧ��,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  EXEC SQL OPEN tran_log_2;
  if(sqlca.sqlcode)
  {
    Message("���α�ʧ��,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0450: [����/���ݿ�] ���α�ʧ��,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  ilTotalcount = 0;

  Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
  Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  while(TRUE)
  {
    memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
    /* EXEC SQL FETCH tran_log_2 INTO :sgSwt_tran_log; delete by szhengye 2002.3.12 */
    EXEC SQL FETCH tran_log_2 INTO 
           :sgSwt_tran_log.tran_id,
           :sgSwt_tran_log.tran_begin,
           :sgSwt_tran_log.tran_status,
           :sgSwt_tran_log.tran_end,
           :sgSwt_tran_log.tran_overtime,
           :sgSwt_tran_log.q_tran_begin,
           :sgSwt_tran_log.tc_tran_begin,
           :sgSwt_tran_log.q_target,
           :sgSwt_tran_log.resq_file,
           :sgSwt_tran_log.resq_offset,
           :sgSwt_tran_log.resq_len, 
           :sgSwt_tran_log.resp_file,
           :sgSwt_tran_log.resp_offset,
           :sgSwt_tran_log.resp_len,
           :sgSwt_tran_log.rev_key;	/* add by szhengye 2002.3.12 */    

    if (sqlca.sqlcode) break;

    _swTimeLongToChs(sgSwt_tran_log.tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_overtime,lsTmp3,"HH:MM:SS");
   
    Message("   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status,lsTmp1,lsTmp2,\
      lsTmp3,sgSwt_tran_log.q_tran_begin, sgSwt_tran_log.tc_tran_begin);

    ilTotalcount++;
    ilPagecount++;
    if(ilPagecount == 20)
    {
      printf("�밴[RETURN]������...\n");
      clKey = getchar();
      if(clKey == 'q' || clKey == 'Q')
        break;
      Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
      Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
      Message("   ============================================================\n");
      ilPagecount=2;
    }
  }
  EXEC SQL CLOSE tran_log_2;
  swDbclose();

  Message("   ����[%d]����¼\n",ilTotalcount);
#endif

  swVdebug(4,"S0460: [��������] swListtran2()������=0");
  return(SUCCESS); 
}

/**************************************************************
 ** ������      : swListtran3
 ** ��  ��      : ��ѯ�����ɹ��Ľ��� 
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swListtran3()
{ 
  short ilRn;       /* ������ */


  swVdebug(4,"S0470: [��������] swListtran3()");

  /* �����ݿ� */
  ilRn=swDbopen();
  if (ilRn) 
  {
    Message("�����ݿ�ʧ��,sqlca.sqlcode=[%d]\n",ilRn);
    swVdebug(1,"S0480: [����/���ݿ�] �����ݿ�ʧ��,sqlca.sqlcode=[%d]",ilRn);
    return(FAIL); 
  }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL DECLARE tran_log_3 CURSOR FOR SELECT  *
    FROM swt_his_tran_log WHERE tran_status = 4 ORDER BY tran_id;
  if(sqlca.sqlcode)
  {
    Message("�����α�ʧ��,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0490: [����/���ݿ�] �����α�ʧ��,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  EXEC SQL OPEN tran_log_3;
  if(sqlca.sqlcode)
  {
    Message("���α�ʧ��,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0500: [����/���ݿ�] ���α�ʧ��,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  ilTotalcount = 0;

  Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
  Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  while(TRUE)
  {
    memset(&sgSwt_tran_log,0x00,sizeof(struct swt_tran_log));
    /* EXEC SQL FETCH tran_log_3 INTO :sgSwt_tran_log; delete by szhengye 2002.3.12 */
    EXEC SQL FETCH tran_log_3 INTO 
           :sgSwt_tran_log.tran_id,
           :sgSwt_tran_log.tran_begin,
           :sgSwt_tran_log.tran_status,
           :sgSwt_tran_log.tran_end,
           :sgSwt_tran_log.tran_overtime,
           :sgSwt_tran_log.q_tran_begin,
           :sgSwt_tran_log.tc_tran_begin,
           :sgSwt_tran_log.q_target,
           :sgSwt_tran_log.resq_file,
           :sgSwt_tran_log.resq_offset,
           :sgSwt_tran_log.resq_len, 
           :sgSwt_tran_log.resp_file,
           :sgSwt_tran_log.resp_offset,
           :sgSwt_tran_log.resp_len,
           :sgSwt_tran_log.rev_key;	/* add by szehgnye 2002.3.12 */    

    if (sqlca.sqlcode) break;

    _swTimeLongToChs(sgSwt_tran_log.tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_overtime,lsTmp3,"HH:MM:SS");
    Message("   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status,\
      lsTmp1,lsTmp2,lsTmp3,sgSwt_tran_log.q_tran_begin,\
      sgSwt_tran_log.tc_tran_begin);

    ilTotalcount++;
    ilPagecount++;
    if(ilPagecount == 20)
    {
      printf("�밴[RETURN]������...\n");
      clKey = getchar();
      if (clKey == 'q' || clKey == 'Q')
        break;
      Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
      Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
      Message("   ============================================================\n");
      ilPagecount = 2;
    }
  }
  EXEC SQL CLOSE tran_log_3;
  swDbclose();

  Message("   ����[%d]����¼\n",ilTotalcount);
#endif

  swVdebug(4,"S0510: [��������] swListtran3()������=0");
  return(SUCCESS); 
}

/**************************************************************
 ** ������      : swListtran4
 ** ��  ��      : ��ѯ����ʧ�ܵĽ��� 
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swListtran4()
{ 
  short ilRn;       /* ������ */


  swVdebug(4,"S0520: [��������] swListtran4()");

  /* �����ݿ� */
  ilRn=swDbopen();
  if (ilRn) 
  {
    Message("�����ݿ�ʧ��,sqlca.sqlcode=[%d]\n",ilRn);
    swVdebug(1,"S0530: [����/���ݿ�] �����ݿ�ʧ��,sqlca.sqlcode=[%d]",ilRn);
    return(FAIL); 
  }

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL DECLARE tran_log_4 CURSOR FOR SELECT  *
    FROM swt_his_tran_log WHERE tran_status = 5 ORDER BY tran_id;
  if(sqlca.sqlcode)
  {
    Message("�����α�ʧ��,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0540: [����/���ݿ�] �����α�ʧ��,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  EXEC SQL OPEN tran_log_4;
  if(sqlca.sqlcode)
  {
    Message("���α�ʧ��,sqlca.sqlcode=[%d]\n",sqlca.sqlcode);
    swVdebug(1,"S0550: [����/���ݿ�] ���α�ʧ��,sqlca.sqlcode=[%d]",sqlca.sqlcode);
    swDbclose();
    return(FAIL);
  }

  ilTotalcount=0;

  Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
  Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
  Message("   ============================================================\n");
  ilPagecount=2;

  while(TRUE)
  {
    memset(&sgSwt_tran_log, 0x00, sizeof(struct swt_tran_log));

    /* EXEC SQL FETCH tran_log_4 INTO :sgSwt_tran_log; delete by szhengye 2002.3.12 */
    EXEC SQL FETCH tran_log_3 INTO 
           :sgSwt_tran_log.tran_id,
           :sgSwt_tran_log.tran_begin,
           :sgSwt_tran_log.tran_status,
           :sgSwt_tran_log.tran_end,
           :sgSwt_tran_log.tran_overtime,
           :sgSwt_tran_log.q_tran_begin,
           :sgSwt_tran_log.tc_tran_begin,
           :sgSwt_tran_log.q_target,
           :sgSwt_tran_log.resq_file,
           :sgSwt_tran_log.resq_offset,
           :sgSwt_tran_log.resq_len, 
           :sgSwt_tran_log.resp_file,
           :sgSwt_tran_log.resp_offset,
           :sgSwt_tran_log.resp_len,
           :sgSwt_tran_log.rev_key;	/* add by szehgnye 2002.3.12 */    

    if (sqlca.sqlcode) break;

    _swTimeLongToChs(sgSwt_tran_log.tran_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_end,lsTmp2,"HH:MM:SS");
    _swTimeLongToChs(sgSwt_tran_log.tran_overtime,lsTmp3,"HH:MM:SS");
    Message("   %8d %2d %9.9s %9.9s %9.9s  %4d %10.10s\n",\
      sgSwt_tran_log.tran_id,sgSwt_tran_log.tran_status,\
      lsTmp1,lsTmp2,lsTmp3,sgSwt_tran_log.q_tran_begin,\
      sgSwt_tran_log.tc_tran_begin);

    ilTotalcount++;
    ilPagecount++;
    if(ilPagecount == 20)
    {
      printf("�밴[RETURN]������...\n");
      clKey=getchar();
      if(clKey=='q'||clKey=='Q')
        break;
      Message("\n                  ���׿�   ���׽�   ���׳�    ����     ����\n");
      Message("     ��ˮ�� ״̬  ʼʱ��   ��ʱ��   ʱʱ��    ����       ����\n");
      Message("   ============================================================\n");
      ilPagecount=2;
    }
  }
  EXEC SQL CLOSE tran_log_4;
  swDbclose();

  Message("   ����[%d]����¼\n",ilTotalcount);
#endif

  swVdebug(4,"S0560: [��������] swListtran4()������=0");
  return(SUCCESS); 
}

/**************************************************************
 ** ������      : swResaf
 ** ��  ��      : RESAF����
 ** ��  ��      : �Ż�
 ** ��������    : 1999/11/25
 ** ����޸�����: 2001/3/29
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swResaf(char *alSafid)
{
  short ilRc;     
  struct msghead slMsghead;

  swVdebug(4,"S0570: [��������] swResaf(%s)",alSafid);

  _swTrim(alSafid);
  if (strcmp(alSafid,"\0") == 0)
  {
    Message("Usage: R  Safid\n");
    return(FAIL);
  }

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  /* ��λ����  */
  if (qattach( iMBSYSCON ) != 0)
  {
    swVdebug(1,"S0580: [����/����] qattach()����,������=-1,��λ�����������(iMBSYSCON)");
    return(FAIL);
  }

  /* ��������ĸ���������� */
  memset(&slMsghead, 0x00, sizeof(struct msghead));
  slMsghead.iMsgtype = iMSGORDER;
  slMsghead.lCode = 604;
  slMsghead.iBodylen = 0;
  slMsghead.lSafid = atol(alSafid);

  ilRc = _qwrite2((char *)&slMsghead,sizeof(struct msghead),iMBMONREV,0,0,0);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0590: [����/����] _qwrite2()����,������=%d,���ͱ��ĸ�MONREVʧ��",ilRc);
    swMberror(ilRc,NULL);
    return (FAIL);
  }

  qdetach();

  Message("���� SAF �ɹ�!\n");

  swVdebug(4,"S0600: [��������] swResaf()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swListsaf
 ** ��  ��      : ��ʾSAF���� 
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swListsaf()
{
  char lsTmp1[10],lsTmp2[10];
  short ilRc;       /* ������ */
  char clKey;

  short ilPagecount;               /* ������ʾ��Ŀ */
  short i,ilCount;
  struct swt_rev_saf pslSwt_rev_saf[iMAXRECORD];

  swVdebug(4,"S0610: [��������] swListsaf()");

  /* �����ݿ� */
  ilRc=swDbopen();
  if (ilRc) 
  {
    Message("�����ݿ�ʧ��,sqlca.sqlcode=[%d]\n",ilRc);
    swVdebug(0,"S0620: [����/���ݿ�] �����ݿ�ʧ��,sqlca.sqlcode=[%d]",ilRc);
    return(FAIL); 
  }
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  /* modify by nh 20020923
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);*/
/* del by gengling at 2015.04.03 two lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/
  /* ilRc = swDbselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount); */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);
  if (ilRc == SHMNOTFOUND)
  {
    Message("SAF ����δ�ҵ���¼!\n");
    swVdebug(1,"S0630: [����/�����ڴ�] SAF ����δ�ҵ���¼!");
    return(FAIL);
  }
  if (ilRc != SUCCESS)
  {
    Message("���� SAF ��¼ʧ��!\n");
    swVdebug(1,"S0640: [����/�����ڴ�] ���� SAF ��¼ʧ��!");
    return(FAIL);
  }
  swDbclose();
/* #endif */ /* del by gengling at 2015.04.03 PSBC_V1.0 */
 
  Message("\n                SAF��     SAF��   ʣ���\n");
  Message("  ��ˮ�� ״̬  ʼʱ��    ʱʱ��   ������\n");
  Message("===========================================\n");
  ilPagecount = 2;

  for(i=0;i<ilCount;i++)
  {
    _swTimeLongToChs(pslSwt_rev_saf[i].saf_begin,lsTmp1,"HH:MM:SS");
    _swTimeLongToChs(pslSwt_rev_saf[i].saf_overtime,lsTmp2,"HH:MM:SS");
    Message("%8d   %1.1s %9.9s %9.9s %4d\n", pslSwt_rev_saf[i].tran_id,\
      pslSwt_rev_saf[i].saf_status,lsTmp1,lsTmp2,pslSwt_rev_saf[i].saf_num);
    ilPagecount++;
    if(ilPagecount == 20) 
    {
      printf("�밴[RETURN]������...\n");
      clKey = getchar();
      if (clKey == 'q' || clKey == 'Q')  break;
      Message("                SAF��     SAF��   ʣ���\n");
      Message("  ��ˮ�� ״̬  ʼʱ��    ʱʱ��   ������\n");
      Message("===========================================\n");
      ilPagecount = 2;
    }
  }

  Message("����[%d]����¼\n",ilCount);

  swVdebug(4,"S0650: [��������] swListsaf()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swClrsaf
 ** ��  ��      : ���SAF
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swClrsaf(char *alTranid )
{
  struct swt_rev_saf pslSwt_rev_saf[iMAXRECORD];
  short ilRc,i,ilCount,j = 0,ilAnswer;      
  long  llSaf_id,llTranid;
  
  swVdebug(4,"S0660: [��������] swClrsaf(%s)",alTranid);

  _swTrim(alTranid);
  if (strcmp(alTranid,"\0") == 0) 
  {
    Message("Usage: C    Tranid\n");
    Message("if Tranid = -1,Clear All\n");
    return(FAIL);
  }
  
  llTranid = atol(alTranid); 
  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  /* modify by nh 20020923
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);*/
/* del by gengling at 2015.04.03 tow lines PSBC_V1.0 */
/* #ifdef DB_SUPPORT */ /*support database,Database Deprivation Project 2014-12*/
  /* ilRc = swDbselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount); */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  ilRc = swShmselect_swt_rev_saf_all(pslSwt_rev_saf,&ilCount);
  if (ilRc == SHMNOTFOUND)
  {
    Message("SAF ����δ�ҵ���¼!\n");
    swVdebug(1,"S0670: [����/�����ڴ�] SAF ����δ�ҵ���¼!");
    return(FAIL);
  }
  if (ilRc != SUCCESS)
  {
    Message("���� SAF ��¼ʧ��!\n");
    swVdebug(1,"S0680: [����/�����ڴ�] ���� SAF ��¼ʧ��!");
    return(FAIL);
  }

  if(llTranid == -1) 
  {
    printf("ȷ��Ҫ��ձ��е����м�¼��? (Y/N)");
    ilAnswer = getchar();
    if (ilAnswer != 'y' && ilAnswer != 'Y') return(FAIL); 

    for(i=0;i<ilCount;i++)
    {
      /* modify by nh 20020923 
      ilRc = swShmdelete_swt_rev_saf(pslSwt_rev_saf[i].saf_id); */
	  /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
      /* ilRc = swDbdelete_swt_rev_saf(pslSwt_rev_saf[i].saf_id); */
	  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmdelete_swt_rev_saf(pslSwt_rev_saf[i].saf_id);
      if (ilRc != SUCCESS)
      {
        swVdebug(1,"S0690: [����/�����ڴ�] ִ��swShmdelete_swt_rev_saf����");
        Message("ɾ�� SAF ��¼ʧ��!\n");
        Message("ɾ����[%d]��SAF��¼]\n",ilCount);
        return(FAIL);
      }
    }
    Message("ɾ����[%d]��SAF��¼\n",ilCount);

    swVdebug(4,"S0700: [��������] swClrsaf()������=0");
    return(SUCCESS);
  }
  j=0;
  for (i = 1;i < ilCount;i++)
  {
    if (pslSwt_rev_saf[i].tran_id == llTranid)
    {
      llSaf_id = pslSwt_rev_saf[i].saf_id;
      /* modify by nh 20020923 
      ilRc = swShmdelete_swt_rev_saf(llSaf_id); */
	  /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
      /* ilRc = swDbdelete_swt_rev_saf(llSaf_id); */
	  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
	  ilRc = swShmdelete_swt_rev_saf(llSaf_id);
      if (ilRc != SUCCESS)
      {
        swVdebug(1,"S0710: [����/�����ڴ�] ִ��swShmdelete_swt_rev_saf����");
        Message("ɾ�� SAF ��¼����!\n");
        Message("��ɾ��[%d]��SAF��¼!\n",j);
        return(FAIL);
      }
      j ++;
    }
  }
  Message("ɾ��[%d]��SAF��¼!\n",j);
/* #endif */ /* del by gengling at 2015.04.03 PSBC_V1.0 */

  swVdebug(4,"S0720: [��������] swClrsaf()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swListport
 ** ��  ��      : �鿴�˿�״̬
 ** ��  ��      : ������
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swListport(char *alQ_id)
{
  short i,j,ilQ_id_where;
  short ilCount,ilTotalcount,ilRc,ilTmp = 0;
  struct swt_sys_queue  slSwt_sys_queue[400];
  char  alPortname[21];
  int   ilPagecount;
  int   ilTemp[8];

  swVdebug(4,"S0730: [��������] swListport(%s)",alQ_id);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  Message("\n     �˿ں�    ����       ͨѶ��  Ӧ�ò�    Q��   Echotest��  �˹���Ԥ��\n");
  Message("  ======================================================================\n");
  ilPagecount=2;
  ilTotalcount=0;

  if (strcmp(alQ_id,"\0") == 0)
    ilQ_id_where = 0;
  else
    ilQ_id_where = atoi(alQ_id);

  ilRc = swShmselect_swt_sys_queue_all(slSwt_sys_queue,&ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0740: [����/�����ڴ�] ִ�� swShmselect_swt_sys_queue_all ʧ��!"); 
    Message("ִ�� swShmselect_swt_sys_queue_all ʧ��!\n");
    return(FAIL);
  }
  
  for (i= 0;i < ilCount;i++)
  {
    if (ilQ_id_where > 0)
      if (slSwt_sys_queue[i].q_id != ilQ_id_where) 
          continue;
    
    for(j=0;j<8;j++)
    {
      if ( slSwt_sys_queue[i].port_status & (128>>j) )
        ilTemp[j]=1;
      else
        ilTemp[j]=0;
    }

    if (strlen(slSwt_sys_queue[i].q_name) > 20)
    {
      memcpy(alPortname,slSwt_sys_queue[i].q_name,17);
      alPortname[17] = '\0';
      strcat(alPortname,"...");
    }
    else
      strncpy(alPortname,slSwt_sys_queue[i].q_name,20);
 
    Message("  %-6d%-20s  %-8d%-8d%-8d%-8d%-8d\n", slSwt_sys_queue[i].q_id,
      alPortname,ilTemp[0],ilTemp[1],ilTemp[2],ilTemp[3],ilTemp[4]);
    ilPagecount++;
    ilTotalcount++;
    if ((ilTmp == 0) && (ilQ_id_where > 0))
    {
      ilTmp = 1;
      break;
    }

    if(ilPagecount == 21)
    {
      printf("�밴[RETURN]������...\n");
      if(getchar() == 'q') break;

      Message("\n     �˿ں�    ����       ͨѶ��  Ӧ�ò�    Q��   Echotest��  �˹���Ԥ��\n");
      Message("  ======================================================================\n");
      ilPagecount=2;
    }
  }

  if ((ilTmp == 0) && (ilQ_id_where >0))
  {
    Message("�˿�δ����!\n");
    return(FAIL);
  }

  Message("   �ܹ���[%d]��������Ϣ\n",ilTotalcount);

  swVdebug(4,"S0750: [��������] swListport()������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : swClrmailbox
 ** ��  ��      : �������
 ** ��  ��      : ������
 ** ��������    : 2001/08/23
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swClrmailbox(char *alQid)
{
  int ilRc,ilQid;
  short ilAnswer;

  swVdebug(4,"S0760: [��������] swClrmailbox(%s)",alQid);

  _swTrim(alQid);
  if (strcmp(alQid,"\0") == 0)
  {
    Message("Usage: C   q_id\n");
    Message("if q_id = 0,clear all Mailbox\n");
    return(FAIL);
  } 

  ilQid = atoi(alQid);
  if (ilQid == 0)
  {
    printf("ȷ��Ҫ�������������? (Y/N)");
    ilAnswer = getchar();
    if (ilAnswer != 'y' && ilAnswer != 'Y')  return(FAIL);
  }
 
  ilRc = bmqOpen(iMBSYSCON);
  if (ilRc)
  {
    swVdebug(0,"S0770: [����/����] bmqOpen()����,������=%d",ilRc);
    return(FAIL);
  }

  ilRc = bmqClearmb(ilQid);
  if (ilRc)
  {
    Message("�������ʧ��\n");
    ilRc = bmqClose();
    if (ilRc)
    {
      swVdebug(0,"S0780: [����/����] bmqClose()����,������=%d",ilRc);
      return(FAIL);
    }
    return(FAIL);
  }
  Message("�������ɹ�\n");
  ilRc = bmqClose();
  if (ilRc)
  {
    swVdebug(0,"S0790: [����/����] bmqClose()����,������=%d",ilRc);
    return(FAIL);
  }

  swVdebug(4,"S0800: [��������] swClrmailbox()������=0");
  return(SUCCESS);
}
   
/**************************************************************
 ** ������      : swListmailbox
 ** ��  ��      : ��ʾ������Ϣ
 ** ��  ��      : ������
 ** ��������    : 2001/08/23
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
extern int swListmailbox()
{

  short ilRc,i;
  char  clKey;
  struct mbinfo slMbinfo;

  short ilTotalcount=0;              /* ���� */
  short ilPagecount =0;              /* ������ʾ��Ŀ */

  swVdebug(4,"S0810: [��������] swListmailbox()");

  Message("\n      �����    ��������    ��������    ��������    ������  \n");
  Message("   ==========================================================\n");
  ilPagecount = 2;
  
  i = 1;
  ilRc = bmqOpen(iMBSYSCON);
  if (ilRc)
  {
    swVdebug(0,"S0820: [����/����] bmqOpen()����,������=%d",ilRc);
    return(FAIL);
  }
 
  memset(&slMbinfo,0x0,sizeof(struct mbinfo));

  while((ilRc = bmqGetmbinfo(i,&slMbinfo)) != 100)
  {
    if (ilRc == -1)
    {
      swVdebug(1,"S0830: [����/����] bmqGetmbinfo()����,������=%d",ilRc);
      return(FAIL);
    }
    if ( ilRc || slMbinfo.lSendnum || slMbinfo.lRecvnum || slMbinfo.lPendnum || slMbinfo.iConnect )
    {    
      Message("       %-7d    %-8d    %-7d     %-7d     %-6d\n", \
        i, slMbinfo.lRecvnum, slMbinfo.lSendnum, slMbinfo.lPendnum,
        slMbinfo.iConnect);

      ilPagecount++;
      ilTotalcount++;
      if (ilPagecount == 21)
      {
        printf("�밴[RETURN]������...\n");
        clKey = getchar();
        if(clKey == 'q' || clKey == 'Q')  break;
        Message("\n      �����    ��������    ��������    ��������    ������  \n");
        Message("   ==========================================================\n");
        ilPagecount = 2;
      }
    }
    i++;
    memset(&slMbinfo,0x0,sizeof(struct mbinfo));
  }

  Message("   �ܹ���[%d]��������Ϣ\n",ilTotalcount);
  ilRc = bmqClose();
  if (ilRc)
  {
    swVdebug(0,"S0840: [����/����] bmqClose()����,������=%d",ilRc);
    return(FAIL);
  }

  swVdebug(4,"S0850: [��������] swListmailbox()������=0");
  return(SUCCESS);
}

/* === begin of added by fzj at 2002.03.02 === */
/**************************************************************
 ** ������      : swListporttran
 ** ��  ��      : �鿴�˿ڽ�����
 ** ��  ��      : fanzhijie
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swListporttran(char *alQ_id)
{
  short i,ilQ_id_where;
  short ilCount,ilTotalcount,ilRc,ilTmp = 0;
  struct swt_sys_queue  slSwt_sys_queue[400];
  int   ilPagecount;

  swVdebug(4,"S0860: [��������] swListport(%s)",alQ_id);

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  Message("\n  �˿ں�  Դ��������  ���Դ��������  ��������  ���������\n");
  Message("  ==============================================================\n");
  ilPagecount=2;
  ilTotalcount=0;

  if (strcmp(alQ_id,"\0") == 0)
    ilQ_id_where = 0;
  else
    ilQ_id_where = atoi(alQ_id);

  ilRc = swShmselect_swt_sys_queue_all(slSwt_sys_queue,&ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0870: [����/�����ڴ�] ִ�� swShmselect_swt_sys_queue_all ʧ��!"); 
    Message("ִ�� swShmselect_swt_sys_queue_all ʧ��!\n");
    return(FAIL);
  }
  
  for (i= 0;i < ilCount;i++)
  {
    if (ilQ_id_where > 0)
      if (slSwt_sys_queue[i].q_id != ilQ_id_where) continue;

    Message("  %6d  %10d  %12d  %10d  %12d\n", slSwt_sys_queue[i].q_id, slSwt_sys_queue[i].tranbegin_num, slSwt_sys_queue[i].tranbegin_max, slSwt_sys_queue[i].traning_num, slSwt_sys_queue[i].traning_max);

    ilPagecount++;
    ilTotalcount++;
    if ((ilTmp == 0) && (ilQ_id_where > 0))
    {
      ilTmp = 1;
      break;
    }

    if(ilPagecount == 21)
    {
      printf("�밴[RETURN]������...\n");
      if(getchar() == 'q') break;
      Message("\n  �˿ں�  Դ��������  ���Դ��������  ��������  ���������\n");
      Message("  ==============================================================\n");
      ilPagecount=2;
    }
  }

  if ((ilTmp == 0) && (ilQ_id_where >0))
  {
    Message("�˿�δ����!\n");
    return(FAIL);
  }

  Message("   �ܹ���[%d]��������Ϣ\n",ilTotalcount);

  swVdebug(4,"S0880: [��������] swListporttran()������=0");
  return(SUCCESS);
}
/* === end of added by fzj at 2002.03.02 === */

/* === begin of added by fzj at 2002.03.02 === */
/**************************************************************
 ** ������      : swResetporttran
 ** ��  ��      : ���ö˿ڽ�����
 ** ��  ��      : fanzhijie
 ** ��������    : 2001/08/20
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : SUCCESS/FAIL
***************************************************************/
int swResetporttran()
{
  int ilRc;

  swVdebug(4,"S0890: [��������] swResetporttran()");

  ilRc = swShmcheck();
  if (ilRc != SUCCESS)
  {
    Message("�����ڴ������!\n");
    return(FAIL);
  }

  ilRc = swShmresetporttran();
  if (ilRc)
  {
    Message("���ö˿ڽ���������!\n");
    return(FAIL);
  }
  Message("���ö˿ڽ������ɹ� !\n");
  return(SUCCESS);
}
/* === end of added by fzj at 2002.03.02 === */

/* add by gengling at 2013.04.17 begin PSBC_V1.0 */
/************************************************************
 **��  ��: ����ĳһ�ʽ����������ƵĽ�����
 **��  ��: iQid      - ������� Դ������
           aTrancode - ������
 **����ֵ: 0  - �ɹ�
 ************************************************************/
int swShmtranflowreset(char *iQid, char *aTrancode)
{
    int i;
    short ilRc;
    
    swVdebug(4, "[��������] swShmtranflowreset(%s,%s)", iQid, aTrancode);

    ilRc = swShmcheck();
    if (ilRc != SUCCESS)
    {
        Message("�����ڴ������!\n");
        return(FAIL);
    }

#ifndef HASH
    for (i=0; i<psmShmidx_s->sIdx_sys_tran.iCount; i++)
    {
        if (smShmbuf_s.psSwt_sys_tran[i].sys_id == atoi(iQid) &&
                !strncmp(smShmbuf_s.psSwt_sys_tran[i].tran_code, aTrancode, sizeof(smShmbuf_s.psSwt_sys_tran[i].tran_code)))
        {
            smShmbuf_s.psSwt_sys_tran[i].tranning_num = 0;
            swVdebug(2, "����(%s,%s)������Ϊ0", iQid, aTrancode);
            break;
        }
    }
#else
    long dataoffset=0;
    struct swt_sys_tran *pslTranid;
    memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
    pslTranid = (struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset);

    for (i=0; i<psmShmidx_s->sIdx_sys_tran_hash.iCount; i++)
    {
        if (pslTranid[i].sys_id == atoi(iQid) &&
                !strncmp(pslTranid[i].tran_code, aTrancode, sizeof(pslTranid[i].tran_code)))
        {
            pslTranid[i].tranning_num = 0;
            swVdebug(2, "����(%s,%s)������Ϊ0", iQid, aTrancode);
            break;
        }
    }
#endif
    swVdebug(4, "[��������] swShmtranflowreset()������=0");
    return(SUCCESS);
}

/************************************************************
 **��  ��: �������н����������ƵĽ�����
 **��  ��: ��
 **����ֵ: 0  - �ɹ�
 ************************************************************/
int swShmtranflowresetall()
{
    int i;
    short ilRc;
    
    swVdebug(4, "[��������] swShmtranflowresetall()");

    ilRc = swShmcheck();
    if (ilRc != SUCCESS)
    {
        Message("�����ڴ������!\n");
        return(FAIL);
    }

#ifndef HASH
    for (i=0; i<psmShmidx_s->sIdx_sys_tran.iCount; i++)
    {
        smShmbuf_s.psSwt_sys_tran[i].tranning_num = 0;
        swVdebug(2, "����(%d,%s)������Ϊ0", smShmbuf_s.psSwt_sys_tran[i].sys_id, smShmbuf_s.psSwt_sys_tran[i].tran_code);
    }
#else
    long dataoffset=0;
    struct swt_sys_tran *pslTranid;
    memcpy(&dataoffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
    pslTranid = (struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash+dataoffset);

    for (i=0; i<psmShmidx_s->sIdx_sys_tran_hash.iCount; i++)
    {
            pslTranid[i].tranning_num = 0;
            swVdebug(2, "����(%d,%s)������Ϊ0", pslTranid[i].sys_id, pslTranid[i].tran_code);
    }
#endif
    swVdebug(4, "[��������] swShmtranflowresetall()������=0");
    return(SUCCESS);
}
/* add by gengling at 2013.04.17 end PSBC_V1.0 */
