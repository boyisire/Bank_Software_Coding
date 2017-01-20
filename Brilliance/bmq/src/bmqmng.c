/****************************************************************/
/* ģ����    ��BMQMNG                                         */
/* ģ������    ���������ģ��                                   */
/* �� �� ��    ��V2.2                                           */
/* ��    ��    �����                                           */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�2006/12/29                                     */
/* ģ����;    ����ʾ����״̬���������䡢�������               */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2001.08.17 ���V2.0,����354��                              */
/* 2002.07.26 ���V2.1,����744��,����ͳ�ƹ���                 */
/* 2006.12.29 ���V2.2,����1010��,������ʾ�ļ�����״̬        */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"

/*��������*/
static int _bmqMng_quit();
static int _bmqCmdlist();
static int _bmqCmdreset();
static int _bmqCmdclear(short iMbid);
static int _bmqPrintmball();
static int _bmqPrintipcs();
static int _bmqLoadmbset();
static int _bmqPrintpid(short iMbid);
static int _bmqPrinttotalstat(short iOrgmb,short iDesmb);
static int _bmqPrintdetailstat(short iOrgmb,short iDesmb);
static int _bmqStatreset();
static int _bmqPrintFiletrans_cur();
static int _bmqPrintFiletrans_his(char *aDate);
static int _bmqPrintFiletrans_all();
static int _bmqClearFiletransRec(char *aRecno);

extern int _bmqPrintmbinfo();
extern int _bmqPrintmbqueue();
extern int _bmqPrintmbpack();
extern int _bmqPrintmbconnect();
extern int _bmqQueuefile_set(FILE *fp,short iTag,long lLlink,long lRlink,long lOffsetid);

int main(int argc,char **argv)
{
  int  ilRc;
 short ilDesmb,ilOrgmb;
 
  _bmqShowversion(argc,argv);

  if (argc < 2)
  {
    _bmqMng_quit();
  }
  
  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    printf("%s :���ӹ����ڴ�������:%d\n",__FILE__,ilRc);
    exit(FAIL);
  }
 
  if(argc == 1) _bmqMng_quit();
  
  if (strcmp(argv[1],"list") == 0) _bmqCmdlist();
  else if (strcmp(argv[1],"reset") == 0) _bmqCmdreset();
  else if (strcmp(argv[1],"ipcs") == 0) _bmqPrintipcs();
  else if (strcmp(argv[1],"load") == 0) _bmqLoadmbset();
  else if (strcmp(argv[1],"unlock") == 0) 
  {
     _bmqUnlock(LK_ALL); 
     printf("�������!\n");
     exit(SUCCESS);
  }
  else if (strcmp(argv[1],"clear") == 0)
  {
    if (argc == 2) _bmqCmdclear(0);
    else _bmqCmdclear(atoi(argv[2]));
  }
  else if (strcmp(argv[1],"pid") == 0)
  {
    if (argc == 2) _bmqPrintpid(0);
    else _bmqPrintpid(atoi(argv[2]));
  }
  else if (strcmp(argv[1],"debug") == 0)
  {
    if( argc == 2 ) _bmqMng_quit();
    if (strcmp(argv[2],"all") == 0) _bmqPrintmball();
    else if (strcmp(argv[2],"info") == 0) _bmqPrintmbinfo();
    else if (strcmp(argv[2],"queue") == 0) _bmqPrintmbqueue();
    else if (strcmp(argv[2],"pack") == 0) _bmqPrintmbpack();
    else if (strcmp(argv[2],"connect") == 0) _bmqPrintmbconnect();
    else _bmqMng_quit();
    exit(SUCCESS);
  }
  else if (strcmp(argv[1],"stat") == 0)
  {
    if( argc == 2 ) _bmqMng_quit();
    if (strcmp(argv[2],"total") == 0){
      if( argc == 4 ) {
      	ilDesmb = 0;	
      	ilOrgmb = atoi(argv[3]);
      }
      else if( argc == 3 ) ilOrgmb = ilDesmb = 0;
      else 
      {
      	ilDesmb = atoi(argv[4]);
      	ilOrgmb = atoi(argv[3]);
      }
      _bmqPrinttotalstat(ilOrgmb,ilDesmb); 
    }
    if (strcmp(argv[2],"list") == 0){
      if( argc == 4 ) {
      	ilDesmb = 0;	
      	ilOrgmb = atoi(argv[3]);
      }
      else if( argc == 3 ) ilOrgmb = ilDesmb = 0;
      else 
      {
      	ilDesmb = atoi(argv[4]);
      	ilOrgmb = atoi(argv[3]);
      }
      _bmqPrintdetailstat(ilOrgmb,ilDesmb); 
    }
    else if (strcmp(argv[2],"start") == 0) psgMbshm->iStatflag = 1;
    else if (strcmp(argv[2],"stop") == 0) psgMbshm->iStatflag = 0;
    else if (strcmp(argv[2],"reset") == 0) _bmqStatreset();
    else _bmqMng_quit();
    exit(SUCCESS);
  }
  else if (strcmp(argv[1],"file") == 0)
  {
    if( argc == 2 ) _bmqPrintFiletrans_cur();
    else if (strcmp(argv[2],"his") == 0)
    {
       if( argc == 3 ) _bmqPrintFiletrans_his(NULL);
       else _bmqPrintFiletrans_his(argv[3]);
    }
    else if (strcmp(argv[2],"all") == 0)
      _bmqPrintFiletrans_all();
    else if (strcmp(argv[2],"clear") == 0)
    {
      if( argc == 3 ) _bmqClearFiletransRec("0");
      else _bmqClearFiletransRec(argv[3]);
    }  
    else _bmqMng_quit();
  }
  else
  {
    _bmqMng_quit();
  }
  exit(0);
}

/**************************************************************
 ** ������      : _bmqMng_quit
 ** ��  ��      : ��ʾ�˳�
 ** ��  ��      :
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      :
***************************************************************/
static int _bmqMng_quit()
{
  printf("ʹ�÷���: bmqmng list|reset|unlock|clear|ipcs|pid|load|debug|stat\n");
  printf("  list                   ��ʾBMQͳ��״̬\n");
  printf("  reset                  ����BMQͳ��״̬\n");
  printf("  unlock                 �������\n");
  printf("  clear [n]              ��������\n");
  printf("  ipcs                   ��ʾBMQʹ�õ�IPCS��Դ��Ϣ\n");
  printf("  pid   [n]              ��ӡ������Ľ���PID\n");
  printf("  load                   ˢ��������Զ�·������\n");
  printf("  file [his|all|clear]   �ļ�������Ϣ,�޲�����ʾ��ǰ������Ϣ\n");
  printf("    his [yyyymmdd]       ��ʾָ������(��ȫ��)������Ϣ\n");
  printf("    all                  ��ʾȫ��(��ǰ����ʷ)������Ϣ\n");
  printf("    clear [n]            ���ָ����¼��Ŵ����¼\n");
  printf("  debug info|queue|pack|connect   BMQ�ڲ�������Ϣ\n");
  printf("    all                  BMQ�ڲ�������Ϣ(ȫ��)\n");
  printf("    info                 BMQ�ڲ�������Ϣ(������Ϣ)\n");
  printf("    queue                BMQ�ڲ�������Ϣ(�ʼ�����)\n");
  printf("    pack                 BMQ�ڲ�������Ϣ(�����ڴ����ݰ�)\n");
  printf("    connect              BMQ�ڲ�������Ϣ(��������)\n");
  printf("  stat total|list|start|stop|reset  ���ķ���ͳ��\n");
  printf("    total org_mb des_mb  ���ķ���ͳ����Ϣ(����)\n");
  printf("    list  org_mb des_mb  ���ķ���ͳ����Ϣ(��ϸ)\n");
  printf("    start                ���ñ��ķ���ͳ����Ϣ\n");
  printf("    stop                 ֹͣ���ķ���ͳ����Ϣ\n");
  printf("    reset                ���ñ��ķ���ͳ����Ϣ\n");
  exit(FAIL);
}


/**************************************************************
 ** ������      : _bmqCmdlist
 ** ��  ��      : ��ʾBMQͳ��״̬
 ** ��  ��      :
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
***************************************************************/
static int _bmqCmdlist()
{
  int    i,ilRc;
  long   llLink;
  long   llPendnum;
  FILE   *fp;
  char   alHour[3],alMin[3],alSec[3],alSndpid[9],alRcvpid[9];
  struct mbqueue slMbqueue;
  struct tm *psT1,*psT2;

  printf("����� ������ �������� ����ʱ�� ���ͽ��� �������� ����ʱ�� ���ս��� ��������\n");
  printf("----------------------------------------------------------------------------\n");
  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {
    memset(alSndpid,0x00,sizeof(alSndpid));
    memset(alRcvpid,0x00,sizeof(alRcvpid));
    llLink = psgMbinfo[i-1].lQueue_head;
    llPendnum = 0;
    for(;;)
    {
      if (llLink == 0) break;
      if (llLink > psgMbshm-> lMbqueuecount)
      {
        /* �򿪶��л����ļ�,ȡ����¼ */
        _bmqLock(LK_ALL);
        if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
        {
          _bmqDebug("S0060 %s:�򿪶��л����ļ�����",__FILE__);
          _bmqUnlock(LK_ALL);
          continue;
        }
        _bmqQueuefile_pick(fp,&slMbqueue,llLink);
        llLink = slMbqueue.lRlink;
        _bmqUnlock(LK_ALL);
        _bmqClosefile(fp);
      }
      else 
        llLink = psgMbqueue[llLink-1].lRlink;
      llPendnum ++;
    }

    if ((psgMbinfo[i-1].lSendnum)
      ||(psgMbinfo[i-1].lRecvnum)
      ||(psgMbinfo[i-1].lPendnum)
      ||(psgMbinfo[i-1].iConnect))
    {
      psT1 = localtime(&(psgMbinfo[i-1].lSendtime));
      if( psgMbinfo[i-1].lSendtime )
      {
        sprintf(alHour,"%02d",psT1->tm_hour);
        sprintf(alMin,"%02d",psT1->tm_min);
        sprintf(alSec,"%02d",psT1->tm_sec);
      }
      else
      {
        memcpy(alHour,"--",2);
        memcpy(alMin,"--",2);
        memcpy(alSec,"--",2);
        alHour[2] = alMin[2] = alSec[2] = '\0';
      }

      printf("%-6d %6d %8ld %2s:%2s:%2s ",
        i,
        psgMbinfo[i-1].iConnect,
        psgMbinfo[i-1].lSendnum,
        alHour,alMin,alSec);

      psT2 = localtime(&(psgMbinfo[i-1].lRecvtime));
      if( psgMbinfo[i-1].lRecvtime )
      {
        sprintf(alHour,"%02d",psT2->tm_hour);
        sprintf(alMin,"%02d",psT2->tm_min);
        sprintf(alSec,"%02d",psT2->tm_sec);
      }
      else
      {
        memcpy(alHour,"--",2);
        memcpy(alMin,"--",2);
        memcpy(alSec,"--",2);
        alHour[2] = alMin[2] = alSec[2] = '\0';
      }
      if( psgMbinfo[i-1].lSendpid )
        sprintf(alSndpid,"%8ld",psgMbinfo[i-1].lSendpid); 
      else
        memcpy(alSndpid,"--",2);
      if( psgMbinfo[i-1].lRecvpid )
        sprintf(alRcvpid,"%8ld",psgMbinfo[i-1].lRecvpid); 
      else
        memcpy(alRcvpid,"--",2);
      printf("%8s %8ld %2s:%2s:%2s %8s %ld(%ld)\n",
        alSndpid,
        psgMbinfo[i-1].lRecvnum,
        alHour,alMin,alSec,
        alRcvpid,
        llPendnum,psgMbinfo[i-1].lPendnum);
    }
  }
  printf("----------------------------------------------------------------------------\n");
  exit(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqCmdreset
 ** ��  ��      : ����BMQͳ��״̬
 ** ��  ��      :
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
***************************************************************/
static int _bmqCmdreset()
{
  int   i;
 /* printf("psgMbshm->iMbinfo is:%d\n",psgMbshm->iMbinfo);*/ 

  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {
      psgMbinfo[i-1].lSendnum=0;
      psgMbinfo[i-1].lSendtime=0;
      psgMbinfo[i-1].lSendpid=0;
      psgMbinfo[i-1].lRecvnum=0;
      psgMbinfo[i-1].lRecvtime=0;
      psgMbinfo[i-1].lRecvpid=0;
  }
  printf("����״̬���óɹ�!\n");
  exit(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqCmdclear
 ** ��  ��      : ��������
 ** ��  ��      :
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : iMbid--ָ������� '0'��ʾ����������
 ** ����ֵ      :
***************************************************************/
static int _bmqCmdclear(short iMbid)
{
  int    i,ilRc;
  long   llLink,llLink1;
  /*short  ilMsglen;delete by wh*/
  TYPE_SWITCH ilMsglen;/*add by wh*/
  char   alBuf[iMBMAXPACKSIZE];
  FILE   *fp;
  struct mbqueue slMbqueue;

  if ((iMbid > psgMbshm->iMbinfo) || (iMbid < 0))
  {
    printf("����ŷ�ΧӦΪ1��%d\n",psgMbshm->iMbinfo);
    exit(FAIL);
  } 
  
  ilRc = _bmqLock(LK_ALL);
  if(ilRc < 0)
  {
    printf("�źŵƳ���!\n");
    exit(FAIL);
  }
  if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
  {
     printf("�򿪶��л����ļ�����\n");
     _bmqUnlock(LK_ALL);
     exit(FAIL);
  }
  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {
    if ((iMbid > 0) && (iMbid != i)) continue;
    llLink = psgMbinfo[i-1].lQueue_head;
    for(;;)
    {
      if (llLink == 0) break;
      if (llLink > psgMbshm->lMbqueuecount)
      {
        ilRc = _bmqQueuefile_pick(fp, &slMbqueue, llLink);
          if (ilRc == FAIL) 
          {
            _bmqUnlock(LK_ALL);
            _bmqClosefile(fp);
            printf("Queuefile_pick error\n");
            exit(FAIL);
          }
        if(slMbqueue.lPack_head > 0)
        {
          ilRc = _bmqPack_get(alBuf,&ilMsglen,slMbqueue.lPack_head);
            if (ilRc == FAIL) 
            {
              _bmqUnlock(LK_ALL);
              _bmqClosefile(fp);
              printf("Packget error\n");
              exit(FAIL);
            }
        } 
        llLink = slMbqueue.lRlink; 
      }
      else
      {
        if (psgMbqueue[llLink-1].lPack_head > 0)
        {
          ilRc = _bmqPack_get(alBuf,&ilMsglen,
            psgMbqueue[llLink-1].lPack_head);
        }
        llLink = psgMbqueue[llLink-1].lRlink;
      }
    }

    llLink = psgMbinfo[i-1].lQueue_head;
    for(;;)
    {
      if (llLink == 0) break;
      if(llLink > psgMbshm->lMbqueuecount)
      {
       ilRc = _bmqQueuefile_pick(fp, &slMbqueue, llLink);
          if (ilRc == FAIL) 
          { 
            _bmqUnlock(LK_ALL);
            _bmqClosefile(fp);
            printf("Queuefile_pick error\n");          
            exit(FAIL);
          }
       ilRc = _bmqQueuefile_set(fp, 0, 0, 0, llLink);
          if (ilRc == FAIL) 
          { 
            _bmqUnlock(LK_ALL);
            _bmqClosefile(fp);
            printf("Queuefile_set error!\n");
            exit(FAIL);
          }
       llLink = slMbqueue.lRlink;
      }
      else 
      { 
        psgMbqueue[llLink-1].cTag = 0;
        llLink1 = llLink;
        llLink  = psgMbqueue[llLink-1].lRlink;
      }
    }
    psgMbinfo[i-1].lQueue_head = 0;
    psgMbinfo[i-1].lQueue_tail = 0;
    psgMbinfo[i-1].lPendnum = 0;

  }
  if (iMbid == 0) printf("������������ɹ�!\n");
  else printf("��������[%d]�ɹ�!\n",iMbid);
  _bmqUnlock(LK_ALL);
  _bmqClosefile(fp);

  exit(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqCmdPrintmball
 ** ��  ��      : ��ӡ����������Ϣ
 ** ��  ��      :
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
***************************************************************/
static int _bmqPrintmball()
{
  _bmqPrintmbinfo();
  _bmqPrintmbqueue();
  _bmqPrintmbpack();
  _bmqPrintmbconnect();
  return(0);
}

/**************************************************************
 ** ������      : _bmqCmdPrintipcs
 ** ��  ��      : ��ӡ�³�Qʹ�õ�IPCS��Ϣ
 ** ��  ��      :
 ** ��������    : 2002/04/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
***************************************************************/
static int _bmqPrintipcs()
{
  int	len;
  char	alShmid[20],alTmp[15];
  char	alCmd[80];
  int	i;
  
  memset(alShmid,0x00,sizeof(alShmid));
  memset(alTmp,0x00,sizeof(alTmp));
  memcpy(alShmid,"0x00000000",10);
  sprintf(alTmp,"%lx",lgSHMKEY_MB);
  len = strlen(alTmp);
  memcpy(&alShmid[10-len],alTmp,len);

  printf("\n �³�Qʹ�õĹ����ڴ���Դ...\n");
#ifdef AIXOS
  sprintf(alCmd,"ipcs -bmp |grep %s",alShmid);
  printf("T        ID     KEY        MODE       OWNER    GROUP     SEGSZ  CPID  LPID\n");
#else
  sprintf(alCmd,"ipcs -m |grep %s",alShmid);
  printf("key       shmid     owner     perms     bytes     nattch    status\n");
#endif
  system(alCmd);

  alTmp[0] = '"';
#ifdef AIXOS
  sprintf(&alTmp[1],"%10ld",psgMbshm->lSemid);
#else
  sprintf(&alTmp[1],"%-10ld",psgMbshm->lSemid);
#endif
  alTmp[11] = '"';
  printf("\n �³�Qʹ�õ��ź�����Դ...\n");
#ifdef AIXOS
  sprintf(alCmd,"ipcs -bsp |grep %s",alTmp);
  printf("T        ID     KEY        MODE       OWNER    GROUP NSEMS\n");
#else
  sprintf(alCmd,"ipcs -s |grep %s",alTmp);
  printf("key       semid     owner     perms     nsems     status\n");
#endif
  system(alCmd);

  
  printf("\n �³�Qʹ�õ���Ϣ������Դ...\n");
#ifdef AIXOS
  printf("T        ID     KEY        MODE       OWNER    GROUP QBYTES LSPID LRPID\n");
#else
  printf("key       msqid     owner     perms     used-bytes  messages\n");
#endif

  if(psgMbshm->lMsgid_grp)
  {
    alTmp[0]='"';
#ifdef AIXOS
      sprintf(&alTmp[1],"%10ld",psgMbshm->lMsgid_grp);
#else
      sprintf(&alTmp[1],"%-10ld",psgMbshm->lMsgid_grp);
#endif
      alTmp[11]='"';
#ifdef AIXOS
      sprintf(alCmd,"ipcs -bpq |grep %s",alTmp);
#else
      sprintf(alCmd,"ipcs -q |grep %s",alTmp);
#endif
      system(alCmd);
  }
  
  if(psgMbshm->lMsgid_grp_rcv)
  {  
    alTmp[0]='"';
#ifdef AIXOS
      sprintf(&alTmp[1],"%10ld",psgMbshm->lMsgid_grp_rcv);
#else
      sprintf(&alTmp[1],"%-10ld",psgMbshm->lMsgid_grp_rcv);
#endif
      alTmp[11]='"';
#ifdef AIXOS
      sprintf(alCmd,"ipcs -bpq |grep %s",alTmp);
#else
      sprintf(alCmd,"ipcs -q |grep %s",alTmp);
#endif
      system(alCmd);
  }
  
  for(i=0;i<psgMbshm->iMbcontcount;i++)
  {
    if(!psgMbconnect[i].cTag) continue;
    alTmp[0]='"';
#ifdef AIXOS
    sprintf(&alTmp[1],"%10ld",psgMbconnect[i].lMsgid_recv);
#else
    sprintf(&alTmp[1],"%-10ld",psgMbconnect[i].lMsgid_recv);
#endif
    alTmp[11]='"';
#ifdef AIXOS
    sprintf(alCmd,"ipcs -bpq |grep %s",alTmp);
#else
    sprintf(alCmd,"ipcs -q |grep %s",alTmp);
#endif
    system(alCmd);
  }
  
  return(0);
}

/**************************************************************
 ** ������      : _bmqCmdPrintpid
 ** ��  ��      : ��ӡ���³�Q����Ľ���PID
 ** ��  ��      :
 ** ��������    : 2002/04/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
static int _bmqPrintpid(short iMbid)
{
  short	i,j,k;
  char	alPid[10];
  char	alCmd[51];

  if(iMbid == 0)
  {
    for(i=0;i<psgMbshm->iMbinfo;i++)
    {
      if(!psgMbinfo[i].iConnect) continue;
      printf("***������[%d]***\n",i+1);
      printf(" PID  ������\n");
      j = psgMbinfo[i].iConn_head;
      sprintf(alPid,"%5ld ",psgMbconnect[j-1].lConnpid);
      sprintf(alCmd,"export UNIX95=\"ps -o pid,comm\";ps -e -o pid -o comm|grep \"%s\"",alPid);
      system(alCmd);
      k = psgMbconnect[j-1].iRlink;
      while(k)
      {
        sprintf(alPid,"%5ld ",psgMbconnect[k-1].lConnpid);
        sprintf(alCmd,"export UNIX95=\"ps -o pid,comm\";ps -e -o pid -o comm|grep \"%s\"",alPid);
        system(alCmd);
        k = psgMbconnect[k-1].iRlink;
      }
    }
  }
  else
  {
    if(!psgMbinfo[iMbid-1].iConnect) 
    {
      printf("������û�н��̰�\n");
      return(0);
    }
    printf("***������[%d]***\n",iMbid);
    printf(" PID  ������\n");
    j = psgMbinfo[iMbid-1].iConn_head;
    sprintf(alPid,"%5ld ",psgMbconnect[j-1].lConnpid);
    sprintf(alCmd,"export UNIX95=\"ps -o pid,comm\";ps -e -o pid -o comm|grep \"%s\"",alPid);
    system(alCmd);
    k = psgMbconnect[j-1].iRlink;
    while(k)
    {
      sprintf(alPid,"%5ld ",psgMbconnect[k-1].lConnpid);
      sprintf(alCmd,"export UNIX95=\"ps -o pid,comm\";ps -e -o pid -o comm|grep \"%s\"",alPid);
      system(alCmd);
      k = psgMbconnect[k-1].iRlink;
    }
  }
  return(0);
}

/**************************************************************
 ** ������      : _bmqStatreset
 ** ��  ��      : ���ñ��ķ���ͳ����Ϣ
 ** ��  ��      :
 ** ��������    : 2002/07/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
static int _bmqStatreset()
{
  char   *alPath,alFile[80];
  char   alBmqfile[101]="/bmqlog/debug/bmqstatfile.log";
 
  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S1930 ��ϵͳ��������[BMQ_PATH]ʧ��");
    return -1;
  }

  
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,alBmqfile);

  unlink(alFile);
  return 0;
}

/**************************************************************
 ** ������      : _bmqPrinttotalstat
 ** ��  ��      : ���ܱ��ķ���ͳ����Ϣ
 ** ��  ��      :
 ** ��������    : 2002/07/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
static int _bmqPrinttotalstat(short iOrgmb,short iDesmb)
{
  char		alFname[80];
  short		i,j;
  /*long		llNum_mb = 0,llNum_total = 0;*/
  struct	mbstat	slMbstat;  
  struct	tm	*T;
  /*
  long		NUM[psgMbshm->iMbinfo][psgMbshm->iMbinfo];
  long		TIME[psgMbshm->iMbinfo][psgMbshm->iMbinfo];
  long		PID[psgMbshm->iMbinfo][psgMbshm->iMbinfo];
  */
  long		NUM[200][200];
  long		TIME[200][200];
  long		PID[200][200];
  FILE		*fp;
  
  sprintf(alFname,"%s/bmqlog/debug/bmqstatfile.log",getenv("BMQ_PATH"));
  if (( fp = fopen(alFname,"r")) == NULL)
  {
    printf("���ͱ���ͳ��Դ�ļ�[%s]������!\n",alFname);
    exit(0);	
  }
  
  for(i=0;i<psgMbshm->iMbinfo;i++)
    for(j=0;j<psgMbshm->iMbinfo;j++)
      NUM[i][j] = 0;
      
  while( fread((char *)(&slMbstat),sizeof(char),sizeof(struct mbstat),fp) > 0 )
  {
    if( (!iOrgmb || (iOrgmb == slMbstat.iOrgmb)) && (!iDesmb || (iDesmb == slMbstat.iDesmb)) )
    {
      NUM[slMbstat.iOrgmb-1][slMbstat.iDesmb-1]	++;
      PID[slMbstat.iOrgmb-1][slMbstat.iDesmb-1]		= slMbstat.lSendpid;
      TIME[slMbstat.iOrgmb-1][slMbstat.iDesmb-1]	= slMbstat.lSendtime;
    }
  }
  
  printf("Դ������� Ŀ������� ͳ�Ʒ������� ��������� �����ʱ�� ���ͽ���\n");
  printf("---------------------------------------------------------------------\n");
  for(i=0;i<psgMbshm->iMbinfo;i++)
  {
    for(j=0;j<psgMbshm->iMbinfo;j++)
    {
      if(NUM[i][j])
      {
      	T	= localtime(&TIME[i][j]);
        printf("%10d %10d %12ld %6d-%02d-%02d %6d:%02d:%02d %8ld\n",
          i+1,j+1,NUM[i][j],T->tm_year+1900,T->tm_mon+1,T->tm_mday, 
          T->tm_hour, T->tm_min, T->tm_sec,PID[i][j]);
        /*llNum_mb += NUM[i][j];*/
      }
    }
    /*
    if(llNum_mb)
    {
      printf("--------------------------------------------\n");
      printf("С��: %4d %23ld\n",i+1,llNum_mb);      
      llNum_total	+= llNum_mb;
      llNum_mb		 = 0;
      printf("---------------------------------------------\n");      
    }
    */
  }
  /*
  printf("�ܼ�: %28ld\n",llNum_total);      
  */
  printf("----------------------------------------------------------------------\n");
  fclose(fp);
  exit(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqPrintdetailstat
 ** ��  ��      : ���ķ���ͳ����Ϣ��ϸ��ӡ
 ** ��  ��      :
 ** ��������    : 2002/07/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
static int _bmqPrintdetailstat(short iOrgmb,short iDesmb)
{
  char   alFname[80];
  FILE   *fp;
  struct	mbstat	slMbstat;
  struct	tm	*T;
  
  sprintf(alFname,"%s/bmqlog/debug/bmqstatfile.log",getenv("BMQ_PATH"));
  if (( fp = fopen(alFname,"r")) == NULL)
  {
    printf("���ͱ���ͳ��Դ�ļ�[%s]������!\n",alFname);
    exit(0);	
  }
  
  printf("Դ������� Ŀ������� ���ķ������� ���ķ���ʱ�� ���ͽ���\n");
  printf("--------------------------------------------------------\n");
  
  while( fread((char *)(&slMbstat),sizeof(char),sizeof(struct mbstat),fp) > 0 )
  {
    if( (!iOrgmb || (iOrgmb == slMbstat.iOrgmb)) && (!iDesmb || (iDesmb == slMbstat.iDesmb)) )
    {
      T = localtime(&slMbstat.lSendtime);	
      printf("%10hd %10hd %6d-%02d-%02d %6d:%02d:%02d %8ld\n",
              slMbstat.iOrgmb,slMbstat.iDesmb,T->tm_year+1900,T->tm_mon+1,T->tm_mday,
              T->tm_hour, T->tm_min, T->tm_sec,slMbstat.lSendpid);
    }
  }
  
  printf("--------------------------------------------------------\n");
 
  fclose(fp);
  exit(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqPrintFiletrans_cur
 ** ��  ��      : ��ǰ���ڴ�����ļ���Ϣ��ӡ
 ** ��  ��      :
 ** ��������    : 2006/12/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
int _bmqPrintFiletrans_cur()
{ 
  int  i,ilFlag;
  char alTimeBufb[30];
  char alFile[80];
  
  printf("\n��ǰ���ڷ��͵��ļ���¼:\n");
  printf("��� �ļ���                 Դ     Ŀ��        ��ʼʱ��        �ļ���С      �Ѵ���С      ��������\n");
  printf("---------------------------------------------------------------------------------------------------\n");
  ilFlag = 0;
  for(i = 0; i <= psgMbshm->iFilecount; i++ )
  {    
     if (psgMbfilemng[i].iFlag == 0)/*�ҵ�δʹ�ü�¼��*/ continue;
     
     _bmqTimeBuf(&psgMbfilemng[i].sBeginTime.time,alTimeBufb); 
     _bmqfilechr(psgMbfilemng[i].aFileName,strlen(psgMbfilemng[i].aFileName),alFile,'/');
     _bmqTrim(alFile);
     if(!ilFlag) ilFlag = 1;
     printf("%-5d%-20.20s%3d:%-4d%3d:%-4d%-22s%-14ld%-14ld%-5.2lf\n" ,i+1,
             alFile,psgMbfilemng[i].iOrgGrpid,psgMbfilemng[i].iOrgMbid,psgMbfilemng[i].iDesGrpid,
             psgMbfilemng[i].iDesMbid,alTimeBufb,psgMbfilemng[i].lFileSize,psgMbfilemng[i].lSendSize,
             psgMbfilemng[i].fSendSpeed);
  }
  if(!ilFlag)
    printf("û�����ڷ��͵��ļ�...\n");
  printf("---------------------------------------------------------------------------------------------------\n");
  return(SUCCESS);
  
}

/**************************************************************
 ** ������      : _bmqPrintFiletrans_his
 ** ��  ��      : �ļ�������ʷ��Ϣ��ӡ
 ** ��  ��      :
 ** ��������    : 2006/12/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
int _bmqPrintFiletrans_his(char *aDate)
{ 	
  int ilRc;
  FILE *fp;
  FILE *pp;
  char alPath[80];
  char alCmd[100];
  char alFiletmp[80];
  char alFilen[80];
  struct monmsg slMonview;
  char alFilename[80];
  char alTimeBufb[30];
  char alFile[80];
  char alDatec[10];
  
  if(aDate)
  {            
    sprintf(alFilename,"%s/mng/filemng.%s",getenv("BMQ_FILE_DIR"),aDate);
    /* ��ָ�������ļ�,ȡ����¼ */ 
    fp=fopen(alFilename,"r");
    if(fp == NULL)
    { 
      printf("��ָ������[%s]�����¼\n",aDate);
      return(FAIL);
    }
    printf("\n��¼����:%s\n",aDate);
    printf("�ļ���                ״̬����           Դ     Ŀ��  ��ʼʱ��    �ļ���С      ��������\n");
    printf("----------------------------------------------------------------------------------------\n");
    while( !feof(fp) )
    { 
      memset(&slMonview,0x00,sizeof(struct monmsg));     
      ilRc=fread((char *)&slMonview,sizeof(char),sizeof(struct monmsg),fp);
      if(ilRc && (ilRc != sizeof(struct monmsg)))
      { 
        printf("��ȡ�ļ������¼�ļ�ʧ��\n");
        fclose(fp); 
        return(FAIL);
      }
      if(ilRc == 0) continue;
      
      _bmqTimeBuf(&slMonview.sBeginTime.time,alTimeBufb);
      _bmqfilechr(slMonview.aFileName,strlen(slMonview.aFileName),alFile,'/');
      _bmqTrim(alFile);
            
      printf("%-20.20s%-18s%3d:%-4d%3d:%-4d%-12s%-14ld%-5.2lf\n",
              alFile,slMonview.aStatusDesc,
              slMonview.iOrgGrpid,slMonview.iOrgMbid,slMonview.iDesGrpid,slMonview.iDesMbid,
              alTimeBufb+11,slMonview.lFileSize,slMonview.fSendSpeed);      
    }
     fclose(fp);
     printf("--------------------------------------------------------------------------------------\n"); 
     return(SUCCESS);  
  }

  
  else 
  {       
    /*��ʾ���л����ļ��ļ�¼*/
    memset(alPath,0x00,sizeof(alPath));
    sprintf(alPath,"%s/mng/",getenv("BMQ_FILE_DIR"));
    memset(alCmd,0x00,sizeof(alCmd));
    strcpy(alCmd,"ls ");
    strcat(alCmd,alPath);
    pp = popen(alCmd, "r");
    if (pp == NULL)
    {
     printf("�򿪹ܵ��ļ�����\n");
     return(FAIL);
    }

    while (!feof(pp))
    {       
      memset(alFiletmp,0x00,sizeof(alFiletmp));
      fscanf(pp,"%s",alFiletmp);           
      if (alFiletmp[0] == '\0') continue;
      if (strstr(alFiletmp,"filemng.") == NULL) continue;
      
      _bmqfilechr(alFiletmp,strlen(alFiletmp),alDatec,'.');    
      sprintf(alFilen,"%s/mng/%s",getenv("BMQ_FILE_DIR"),alFiletmp);
      fp=fopen(alFilen,"r");      
      if (fp == NULL)
      { 
        printf("���ļ������¼�ļ����ɹ�\n");
        return(FAIL);
      }
      
      printf("\n��¼����:%s\n",alDatec); 
      printf("�ļ���                ״̬����           Դ     Ŀ��  ��ʼʱ��    �ļ���С      ��������\n");
      printf("----------------------------------------------------------------------------------------\n");
      while(!feof(fp))
      {        
        ilRc=fread((char *)&slMonview,sizeof(char),sizeof(struct monmsg),fp);
        if(ilRc && ilRc != sizeof(struct monmsg))
        {
          printf("���ļ������¼�ļ�ʧ��\n");
          fclose(fp); 
          pclose(pp);
          return(FAIL);
        } 
        if(ilRc == 0) continue;
        
        _bmqTimeBuf(&slMonview.sBeginTime.time,alTimeBufb); 
        _bmqfilechr(slMonview.aFileName,strlen(slMonview.aFileName),alFile,'/'); 
        _bmqTrim(alFile);        
                       
        printf("%-20.20s%-18s%3d:%-4d%3d:%-4d%-12s%-14ld%-5.2lf\n",
              alFile,slMonview.aStatusDesc,
              slMonview.iOrgGrpid,slMonview.iOrgMbid,slMonview.iDesGrpid,slMonview.iDesMbid,
              alTimeBufb+11,slMonview.lFileSize,slMonview.fSendSpeed);
      }      
      fclose(fp); 
      printf("----------------------------------------------------------------------------------------\n");    
    }     
    pclose(pp);
    return(SUCCESS);  
  }
}

/**************************************************************
 ** ������      : _bmqClearFiletransRec
 ** ��  ��      : ���ָ����¼��Ŵ����¼
 ** ��  ��      :
 ** ��������    : 2006/12/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
int _bmqClearFiletransRec(char *aRecno)
{
  int	i;
  
  for(i = 0; i < strlen(aRecno) ; i++)
  {
    if(!isdigit(aRecno[i]))
    {
      printf("��¼��ű�����������!\n");
      return(FAIL);
    }
  }
  
  if(atoi(aRecno) > psgMbshm->iFilecount)
  {
    printf("��¼���Խ��,max=[%d]\n",psgMbshm->iFilecount);
    return(FAIL);
  }
  
  if (atoi(aRecno))
  {
    psgMbfilemng[atoi(aRecno)-1].iFlag = 0;
    printf("���ָ����¼��Ŵ����¼�ɹ�!\n");
  }    
  else 
  { 
    for(i = 0; i <= psgMbshm->iFilecount; i++ )
    {
      psgMbfilemng[i].iFlag = 0;	
    }
    printf("���ȫ����¼��Ŵ����¼�ɹ�!\n");
  }
  
  return(SUCCESS);	
}

/**************************************************************
 ** ������      : _bmqPrintFiletrans_all
 ** ��  ��      : �ļ�����ȫ����Ϣ��ӡ
 ** ��  ��      :
 ** ��������    : 2006/12/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
int _bmqPrintFiletrans_all()
{ 
  _bmqPrintFiletrans_his(NULL);
  _bmqPrintFiletrans_cur();
  
  return(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqLoadmbset
 ** ��  ��      : ����������Զ�·������
 ** ��  ��      :
 ** ��������    : 2007/01/27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
**************************************************************/
int _bmqLoadmbset()
{
  int   ilRc;

  ilRc = _bmqLock(LK_ALL);
  if( ilRc )
  {
    printf("�źŵƲ���ʧ��");
    printf("[MAIL]��������δˢ��!\n");
    exit(FAIL);
  }
  /*��������פ��ʱ��ͷ�ֵ����*/
  ilRc = _bmqConfig_load("MAIL",0);
  if (ilRc < 0)
  { 
    printf("����MAIL����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini\n",
      getenv("BMQ_PATH"));
    printf("[MAIL]��������δˢ��!\n");
    exit(FAIL);
  }
  _bmqLoadRouterinfo();
  _bmqUnlock(LK_ALL);

  printf("������Զ�·��������ˢ��!\n");
  
  return(SUCCESS);
}
