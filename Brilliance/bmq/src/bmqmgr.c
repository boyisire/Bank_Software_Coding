/* ͷ�ļ����� */
#include <curses.h>
#include "bmq.h"
#include "bmqCurses.h"


int _bmqCmdlist_cur(char [][256]);
int _bmqCmdreset_cur(char [][256]);
int _bmqUnlock_cur(char [][256]);
int _bmqCmdclear_cur(char [][256]);
int _bmqPrintmbinfo_cur(char [][256]);
int _bmqPrintmbqueue_cur(char [][256]);
int _bmqPrintmbpack_cur(char [][256]);
int _bmqPrintmbconnect_cur(char [][256]);
int _bmqPrintmball_cur(char [][256]);
int _bmqReloadfilesvr_cur(char [][256]);
int _bmqReloadgrprcv_cur(char [][256]);
int _bmqReloadcls_cur(char [][256]);
int _bmqReloadclean_cur(char [][256]);
int _bmqReloadcfg_cur(char [][256]);
int _bmqShmdisconnect_cur();
extern int _bmqShmDt();
extern int _bmqGetfile();
extern int _bmqQueuefile_set();



/* ���庯������Ӧ�� */
func_t funclist[] = {
  {"_bmqCmdlist_cur()",_bmqCmdlist_cur},
  {"_bmqCmdreset_cur()",_bmqCmdreset_cur},
  {"_bmqUnlock_cur()",_bmqUnlock_cur},
  {"_bmqCmdclear_cur()",_bmqCmdclear_cur},
  {"_bmqPrintmbinfo_cur()",_bmqPrintmbinfo_cur},
  {"_bmqPrintmbqueue_cur()",_bmqPrintmbqueue_cur},
  {"_bmqPrintmbpack_cur()",_bmqPrintmbpack_cur},
  {"_bmqPrintmbconnect_cur()",_bmqPrintmbconnect_cur},
  {"_bmqPrintmball_cur()",_bmqPrintmball_cur},
  {"_bmqReloadfilesvr_cur()",_bmqReloadfilesvr_cur},
  {"_bmqReloadgrprcv_cur()",_bmqReloadgrprcv_cur},
  {"_bmqReloadcls_cur()",_bmqReloadcls_cur},
  {"_bmqReloadclean_cur()",_bmqReloadclean_cur},
  {"_bmqReloadcfg_cur()",_bmqReloadcfg_cur},
  {"",NULL},
};
/**************************************************************
 ** ������: main()
 ** ����:   ������
 ** ����:   
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: 
***************************************************************/

int main(int argc,char *argv[])
{
    int 	ilRc;
    char	tmp[256];
    menu_t	menu;

  /* ��ӡ�汾�� */
  
  if ( argc > 1 )
    _bmqShowversion(argc,argv);
  
  signal(SIGINT,SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
/*  signal(SIGHUP,SIG_IGN);  */
  signal(SIGTSTP,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
 
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("%s:���ӹ����ڴ�������:%d\n",__FILE__,ilRc);
    /* return(-1);   */
  }
  /* Initialize */
  memset( &menu, 0x00, sizeof(menu_t));
  memset( tmp, 0x00, sizeof(tmp));
  memset(agMenupath,0x00,sizeof(agMenupath));
  sprintf(agMenupath,"%s/etc/curses",getenv("BMQ_PATH"));  
  sprintf(tmp,"%s/bmq.mu",agMenupath);
  ilRc = spw_loadmenu( &menu, tmp);
  if( ilRc < 0 )
  {
    ilRc = _bmqShmdisconnect_cur();
    if (ilRc)
    {
      spw_print(&g_frame,"�Ͽ������ڴ����!\n");
       return(-1);
    }  
    printf("���ز˵������ļ�����(%d)\n",ilRc);
    return(-1);
  }
  strcpy(g_frame.status_bar, "  �³�Q V2.0|               [ESC]=�˳�                  |�³������Ϸ���������  ");

  strcpy(tmp,"");
  spw_createframe(&g_frame,tmp);

  menu.cursor = -1;
  ilRc = spw_exec(&g_frame, &menu);
  spw_print(&g_frame,"ilRc = %d\n",ilRc);
  spw_closeframe(&g_frame);
  return(0);
}


int _bmqShmdisconnect_cur()
{
   /*�Ͽ������ڴ�*/
  return(_bmqShmDt());
}

/**************************************************************
 ** ������: _bmqPackfile_pick_cur
 ** ����:   �ӱ��Ļ����ļ���ȡһ��ָ����¼
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fp--�ļ�ָ��  slMbfilepack--�ṹָ��  
             lPack_head--���ļ�¼����
 ** ����ֵ: SUCCESS,FAIL
***************************************************************/
int _bmqPackfile_pick_cur(FILE *fp,struct mbfilepack *slMbfilepack,long lPack_head)
{
  int    ilRc,ilSize;
  char   alBuf[iFLDVALUELEN];

  if(igDebug >= 2)
    _bmqDebug("S2400 **���ú��� _bmqPackfile_pick_cur(fp,Mbfilepack,%ld)**",lPack_head);

   ilSize = sizeof(struct mbfilepack) + 1;

  fseek(fp,0,SEEK_SET);
  ilRc = _bmqGetfile(fp, (lPack_head-psgMbshm->lMbpacksize-1)*ilSize, ilSize,alBuf);
  if (ilRc == FAIL)
  {
    _bmqDebug("S2410 ���ļ����� _bmqPackfile_pick_cur");
    return (FAIL);
  }

  if(ilRc) return(ilRc);
  if(alBuf[0] == '0') return(WITHOUTUSE); 
  memset(slMbfilepack,0x00,sizeof(struct mbfilepack));
  memcpy(slMbfilepack,alBuf + 1,ilSize - 1);
   if(igDebug >= 2)
     _bmqDebug("S2420 **�������ú��� _bmqPackfile_pick_cur(..)**");
  return(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqCmdlist_cur
 ** ��  ��      : ��ʾBMQͳ��״̬
 ** ��  ��      :
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    :
 ** ����ֵ      :
***************************************************************/
int _bmqCmdlist_cur(char aValue[][256])
{
  int    i,ilRc,ch;
  int    ilPagecount = 0,ilPos,ilFileflag,ilEndflag,ilEndfor;
  long   llPendnum,llLink;
  FILE   *fp,*fp_bmq;
  char   alHour[3],alMin[3],alSec[3],alSndpid[9],alRcvpid[9];
  struct mbqueue slMbqueue;
  struct tm *psT1,*psT2;
  char alTmp[1024];

  ilFileflag=0;
  if(aValue[0][0]!='\0')
  {
    if (aValue[1][0] != '\0')
    {
      if ((fp = fopen(aValue[1],"a+")) == NULL)
      {
        spw_print(&g_frame,"���ļ�[%s]����!",aValue[0]);
        return(-1);	
      }
      ilFileflag = 1;
    }
    else
    {
      spw_print(&g_frame,"����ļ�������Ϊ��!\n");
      return(-1);
    }
  }
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    if (ilFileflag == 1) fclose(fp);
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q\n",ilRc);
    return(-1);
  }
  
  ilPos=1,ilEndflag=0;
  do
  {
    if(ilEndflag==1) 
    { 
      spw_redrawmsg(&g_frame);
      break;
    }
    if (ilFileflag == 0)
    {
      wmove(g_frame.main,1,0);	
      spw_printtowin(g_frame.main,"����� ������ �������� ����ʱ�� ���ͽ��� �������� ����ʱ�� ���ս��� ��������\n");
      ilPagecount = 1;
    }
    else
    {
      fprintf(fp,"%s","����� ������ �������� ����ʱ�� ���ͽ��� �������� ����ʱ�� ���ս��� ��������\n");
    }
        
    /*********************************/
    for(i=ilPos;i<=psgMbshm->iMbinfo;i++)
    {
      if(ilPagecount == LINES_MAIN - 2) break;
      
      memset(alSndpid,0x00,sizeof(alSndpid));
      memset(alRcvpid,0x00,sizeof(alRcvpid));
      llLink = psgMbinfo[i-1].lQueue_head;
      llPendnum = 0;
      /***********************************************/
      for(;;)
      {
        if (llLink == 0) break;
        if (llLink > psgMbshm-> lMbqueuecount)
        {
          /* �򿪶��л����ļ�,ȡ����¼ */
          _bmqLock(LK_ALL);
          if ((ilRc = _bmqOpenfile(0, &fp_bmq)) != SUCCESS)
          {
            _bmqDebug("S0060 %s:�򿪶��л����ļ�����",__FILE__);
            _bmqUnlock(LK_ALL);
            continue;
          }
          _bmqQueuefile_pick(fp_bmq,&slMbqueue,llLink);
          llLink = slMbqueue.lRlink;
          _bmqUnlock(LK_ALL);
          _bmqClosefile(fp_bmq);
        }
        else 
          llLink = psgMbqueue[llLink-1].lRlink;
        llPendnum ++;
       }
     /*************************************************/
     if ((psgMbinfo[i-1].lSendnum)||(psgMbinfo[i-1].lRecvnum)
      ||(psgMbinfo[i-1].lPendnum)||(psgMbinfo[i-1].iConnect))
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
       memset(alTmp,0x00,sizeof(alTmp));
       sprintf(alTmp,"%-6d %6d %8ld %2s:%2s:%2s ",
              i,
              psgMbinfo[i-1].iConnect,
              psgMbinfo[i-1].lSendnum,
              alHour,alMin,alSec);
       if (ilFileflag == 0)
         spw_printtowin(g_frame.main,alTmp);
       else
         fprintf(fp,"%s",alTmp);
        
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
       
       memset(alTmp,0x00,sizeof(alTmp));  
       sprintf(alTmp,"%8s %8ld %2s:%2s:%2s %8s %ld(%ld)\n",
          alSndpid,
          psgMbinfo[i-1].lRecvnum,
          alHour,alMin,alSec,
          alRcvpid,
          llPendnum,psgMbinfo[i-1].lPendnum);
       if (ilFileflag == 0)
         spw_printtowin(g_frame.main,alTmp);
       else
         fprintf(fp,"%s",alTmp);  
       ilPagecount++;  
      }
    }
    
    mvwaddstr(g_frame.msg,0,0,"                     ���� ��ҳ W/S�� ���� U�� ˢ�� Q�� �˳�\n");
    wrefresh(g_frame.msg);
    ilEndfor = 0;
    wrefresh(g_frame.main);
    for(;;)
    {
      ch = getch();
      switch(ch)
      {
      	 case 'Q':
         case 'q':
           ilEndfor = 1;
           ilEndflag = 1;
           break;
         case 's' :
           ilEndfor = 1;
           if(psgMbshm->iMbinfo > i)
             ilPos = ilPos + 1 ;
           break;
         case 'w' :
           ilEndfor = 1;
           if(ilPos > 1) ilPos = ilPos - 1;
           else ilPos = 1;
           break;
         case CTRKEY_UP:
           ilEndfor = 1;
           if(ilPos < (LINES_MAIN - 2)) ilPos = 1;
           else ilPos = ilPos - LINES_MAIN + 3 ; 
           break;
         case CTRKEY_DOWN:
           ilEndfor = 1; 
           if(i < psgMbshm->iMbinfo)
             ilPos = ilPos + LINES_MAIN - 3;
           break;  
         case 'u':
           ilEndfor = 1 ;
           ilPos = ilPos ;
           break;
      }
      if(ilEndfor == 1)  break; 
    }
    spw_redrawmain(&g_frame);
  }while(1);
  
  wrefresh(g_frame.main);
  if (ilFileflag == 1) fclose(fp);
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  }
  spw_redrawmsg(&g_frame);
  return(SUCCESS);
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
int _bmqCmdreset_cur(char aValue[][256])
{
  int   i;
  int   ilRc;

  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q\n",ilRc);
    return(ilRc);
  } 

  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {
      psgMbinfo[i-1].lSendnum=0;
      psgMbinfo[i-1].lSendtime=0;
      psgMbinfo[i-1].lSendpid=0;
      psgMbinfo[i-1].lRecvnum=0;
      psgMbinfo[i-1].lRecvtime=0;
      psgMbinfo[i-1].lRecvpid=0;
  }
  spw_print(&g_frame,"����״̬���óɹ�!\n");

  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  }
  return(SUCCESS);
}
/**************************************************************
 ** ������: _bmqUnlock
 ** ����:   �ͷ��źŵ���Դ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqUnlock_cur(char aValue[][256])
{
  int ilRc,iNum;
  struct sembuf slSembuf;

  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q\n",ilRc);
    return(ilRc);
  } 
  
  iNum = LK_ALL; 
  slSembuf.sem_num = iNum - 1;
  slSembuf.sem_op = 1;
  slSembuf.sem_flg = SEM_UNDO;
  ilRc = semop(psgMbshm->lSemid,&slSembuf,1);
  if (ilRc == -1)
  {
    _bmqDebug("S1800 semop P() errno:%d:%s",errno,strerror(errno));
    ilRc = _bmqShmdisconnect_cur();
    if (ilRc)
    {
      spw_print(&g_frame,"�Ͽ������ڴ����!\n");
      return(ilRc);
    } 
    return(-1);
  }
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  }
  spw_print(&g_frame, "�����ѽ��!\n");
  
  return(SUCCESS);
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
int _bmqCmdclear_cur(char aValue[][256])
{
  short iMbid;
  int    i,ilRc;
  long   llLink,llLink1;
  /*short  ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen;/*add by wh*/
  char   alBuf[iMBMAXPACKSIZE];
  FILE   *fp;
  struct mbqueue slMbqueue;

  iMbid = atoi(aValue[1]);
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q!\n",ilRc);
    return(-1);
  }

  if ((iMbid > psgMbshm->iMbinfo) || (iMbid < 0))
  {
    spw_print(&g_frame,"����ŷ�ΧӦΪ1��%d\n",psgMbshm->iMbinfo);
    ilRc = _bmqShmdisconnect_cur();
    if (ilRc)
    {
      spw_print(&g_frame,"�Ͽ������ڴ����!\n");
      return(-1);
    }
    return(-1);
  } 
  
  if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
  {
     _bmqDebug("S0550 ���ļ�����");
     ilRc = _bmqShmdisconnect_cur();
     if (ilRc)
     {
        spw_print(&g_frame,"�Ͽ������ڴ����!\n");
        return(ilRc);
     }
     return(-1);
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
          _bmqClosefile(fp);	
          ilRc = _bmqShmdisconnect_cur();
          if (ilRc)
          {
            spw_print(&g_frame,"�Ͽ������ڴ����!\n");
            return(ilRc);
          }
          return(-1);          	
        }
        if(slMbqueue.lPack_head > 0)
        {
          ilRc = _bmqPack_get(alBuf,&ilMsglen,slMbqueue.lPack_head);
          if (ilRc == FAIL) 
          {
            _bmqClosefile(fp);
            ilRc = _bmqShmdisconnect_cur();
            if (ilRc)
            {
              spw_print(&g_frame,"�Ͽ������ڴ����!\n");
              return(ilRc);
            }
            return(-1);
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
       	 _bmqClosefile(fp);
         ilRc = _bmqShmdisconnect_cur();
         if (ilRc)
         {
           spw_print(&g_frame,"�Ͽ������ڴ����!\n");
           return(ilRc);
         }
         return(-1);           	
       }
       ilRc = _bmqQueuefile_set(fp, 0, 0, 0, llLink);
       if (ilRc == FAIL)
       {   
         _bmqClosefile(fp);
         ilRc = _bmqShmdisconnect_cur();
         if (ilRc)
         {
           spw_print(&g_frame,"�Ͽ������ڴ����!\n");
           return(ilRc);
         }
         return(-1);       	 
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
    psgMbinfo[i-1].lPendnum = 0;

  }

  if (iMbid == 0) 
    spw_print(&g_frame,"������������ɹ�!\n");
  else
    spw_print(&g_frame,"��������[%d]�ɹ�!\n",iMbid);
  _bmqClosefile(fp);

  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  }

  return(SUCCESS);
}
/**************************************************************
 ** ������: _bmqPrintmbinfo
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqPrintmbinfo_cur(char aValue[][256])
{
  int i,ch,y,x;
  int ilRc,ilFileflag = 0;
  FILE *fp;
  char alTmp[1024];
  short ilPagecount = 0;               /* ������ʾ��Ŀ */
  
 if(aValue[0][0]!='\0')
  {
    if (aValue[1][0] != '\0')
    {
      if ((fp = fopen(aValue[1],"a+")) == NULL)
      {
        spw_print(&g_frame,"���ļ�[%s]����!",aValue[0]);
        return(-1);	
      }
      ilFileflag = 1;
    }
    else
    {
      spw_print(&g_frame,"����ļ�������Ϊ��!\n");
      return(-1);
    }
  }
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q!\n",ilRc);
    if (ilFileflag == 1) fclose(fp);
    return(-1);
  }
  
  getyx(g_frame.main,y,x);
  ilPagecount = y;

  if (ilFileflag == 0)
  { 
    if(ilPagecount >= (LINES_MAIN-3))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);
      ch = getch();
      if(ch == 'q' || ch == 'Q')  
      {
      	if (ilFileflag == 0) fclose(fp); 
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main);
        ilRc = _bmqShmdisconnect_cur();       
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n"); 
          return(-1);
        }
        spw_redrawmsg(&g_frame);
        return(-1);	
      }
      spw_redrawmain(&g_frame);	
      ilPagecount = 2;
    }
    spw_printtowin(g_frame.main,"������Ϣ======================================================\n");
    spw_printtowin(g_frame.main,"ID     iConnect lQueue_head lQueue_tail iConn_head iConn_tail\n");
    ilPagecount = ilPagecount + 2;
  }
  else
  {
    fprintf(fp,"%s","������Ϣ======================================================\n");
    fprintf(fp,"%s","ID     iConnect lQueue_head lQueue_tail iConn_head iConn_tail\n");
  }
  
  for(i=1;i<=psgMbshm->iMbinfo;i++)
  {
    if(ilPagecount ==(LINES_MAIN-2))
    {		
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);      
      ch = getch();
      if(ch == 'q' || ch == 'Q')  
      {
      	spw_redrawmain(&g_frame);
      	wrefresh(g_frame.main);
        if (ilFileflag == 1) fclose(fp);
        ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        { 
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }
        spw_redrawmsg(&g_frame);
        return(-1);      	
      }
      spw_redrawmain(&g_frame);
      spw_printtowin(g_frame.main,"������Ϣ======================================================\n");
      spw_printtowin(g_frame.main,"ID     iConnect lQueue_head lQueue_tail iConn_head iConn_tail\n");
      ilPagecount = 2;
    }    
    if ((psgMbinfo[i-1].lSendnum)
      ||(psgMbinfo[i-1].lRecvnum)
      ||(psgMbinfo[i-1].lPendnum)
      ||(psgMbinfo[i-1].iConnect))
    {
      memset(alTmp,0x00,sizeof(alTmp));
      sprintf(alTmp,"%-6d %-8d %-11ld %-11ld %-10d %-10d\n",i,
        psgMbinfo[i-1].iConnect,
        psgMbinfo[i-1].lQueue_head,
        psgMbinfo[i-1].lQueue_tail,
        psgMbinfo[i-1].iConn_head,
        psgMbinfo[i-1].iConn_tail);
      if (ilFileflag == 0)
        spw_printtowin(g_frame.main,alTmp);
      else
        fprintf(fp,"%s",alTmp);
      ilPagecount++;
    }
  }
  if (ilFileflag == 1) fclose(fp);
  wrefresh(g_frame.main);
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(-1);
  }
  spw_redrawmsg(&g_frame);
  return(0);
}

/**************************************************************
 ** ������: _bmqPrintmbqueue
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqPrintmbqueue_cur(char aValue[][256])
{
  int    ilRc,ilPagecount = 0,ilFileflag = 0;
  int    y,x,ch;
  long   i;
  FILE   *fp,*fp_bmq;
  struct mbqueue slMbqueue;
  char alTmp[1024];
  
  if(aValue[0][0]!='\0')
  {
    if (aValue[1][0] != '\0')
    {
      if ((fp = fopen(aValue[1],"a+")) == NULL)
      {
        spw_print(&g_frame,"���ļ�[%s]����!\n",aValue[0]);
        return(-1);	
      }
      ilFileflag = 1;
    }
    else
    {
      spw_print(&g_frame,"����ļ�������Ϊ��!\n");
      return(-1);
    }
  }
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    if (ilFileflag == 1) fclose(fp);
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q!\n",ilRc);
    return(-1);
  }
  
  getyx(g_frame.main,y,x);
  ilPagecount = y;

  if (ilFileflag == 0)
  {
    if(ilPagecount >= (LINES_MAIN-3))
    {
      wrefresh(g_frame.main);		
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);
      ch = getch();
      if(ch == 'q' || ch == 'Q') 
      { 
        if (ilFileflag == 1) fclose(fp);      	
        spw_redrawmain(&g_frame);
        wrefresh(g_frame.main); 
        ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }    
        spw_redrawmsg(&g_frame); 
        return(-1);
      }
      spw_redrawmain(&g_frame);	
      ilPagecount = 2;
    }
    spw_printtowin(g_frame.main,"�ʼ�����======================================================\n");
    spw_printtowin(g_frame.main,"ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
    ilPagecount = ilPagecount + 2;
  }
  else
  {
    fprintf(fp,"%s","�ʼ�����======================================================\n");
    fprintf(fp,"%s","ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
  }
  
  for(i=1;i<=psgMbshm->lMbqueuecount;i++)
  {
    if(ilPagecount ==(LINES_MAIN-2))
    {
      wrefresh(g_frame.main);		
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);      
      ch = getch();
      if(ch == 'q' || ch == 'Q')
      {
      	spw_redrawmain(&g_frame);
      	wrefresh(g_frame.main);
        if (ilFileflag == 1) fclose(fp);
        ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        { 
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }
        spw_redrawmsg(&g_frame);
        return(-1);      	      	
      }
      spw_redrawmain(&g_frame);
      spw_printtowin(g_frame.main,"�ʼ�����=============== =======================================\n");
      spw_printtowin(g_frame.main,"ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
      ilPagecount = 2;
    } 
    
    if(psgMbqueue[i-1].cTag)
    {
      memset(alTmp,0x00,sizeof(alTmp)); 
      sprintf(alTmp,"%-6ld %-2d %-6ld %-6ld %-7ld %-5d %-5d %-4d %-6ld %-20s\n",i,
        psgMbqueue[i-1].cTag,
        psgMbqueue[i-1].lLlink,
        psgMbqueue[i-1].lRlink,
        psgMbqueue[i-1].lSerialnum,
        psgMbqueue[i-1].iPrior,
        psgMbqueue[i-1].iOrg_grp,
        psgMbqueue[i-1].iOrg_mailbox,
        psgMbqueue[i-1].lPack_head,
        psgMbqueue[i-1].aFilter);
      if (ilFileflag == 0)
        spw_printtowin(g_frame.main,alTmp);
      else
        fprintf(fp,"%s",alTmp);
        
      ilPagecount++;
    } 
  }
 
  if (ilFileflag == 0)
  {
    if(ilPagecount >= (LINES_MAIN-3))
    {
      wrefresh(g_frame.main);		
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);
      ch = getch();
      if(ch == 'q' || ch == 'Q')
      {
      	if (ilFileflag == 1) fclose(fp);
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main); 
        ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
        spw_redrawmsg(&g_frame);   
        return(0);      	
      }
      spw_redrawmain(&g_frame);	
      ilPagecount = 2;
    }
    spw_printtowin(g_frame.main,"�ʼ����л����ļ���¼ =========================================\n");
    spw_printtowin(g_frame.main,"ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
    ilPagecount = ilPagecount + 2;
  }
  else
  {
    fprintf(fp,"%s","�ʼ����л����ļ���¼ =========================================\n");
    fprintf(fp,"%s","ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
  }
  
  if ((ilRc = _bmqOpenfile(0, &fp_bmq)) != SUCCESS)
  {
    spw_print(&g_frame,"���ļ�����\n");
    wrefresh(g_frame.main); 
    if (ilFileflag == 1) fclose(fp);
    ilRc = _bmqShmdisconnect_cur();
    if (ilRc)
    {
      spw_print(&g_frame,"�Ͽ������ڴ����!\n");
      return(-1);
    }
    return(-1);
  }
  i = psgMbshm->lMbqueuecount;
  for(;;)
  {
    ilRc = _bmqQueuefile_pick(fp_bmq,&slMbqueue,++i);
    if(ilRc == FAIL || ilRc == SHMFILEEND) break;
    if(ilRc == WITHOUTUSE) continue;
    if (ilFileflag == 0) 
    {   
      if(ilPagecount ==(LINES_MAIN-2))
      {
        wrefresh(g_frame.main);		
        mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
        wrefresh(g_frame.msg);        
        ch = getch();
        if(ch == 'q' || ch == 'Q')
        {
      	  spw_redrawmain(&g_frame);
      	  wrefresh(g_frame.main);
          if (ilFileflag == 1) fclose(fp);
          ilRc = _bmqShmdisconnect_cur();
          if (ilRc)
          { 
            spw_print(&g_frame,"�Ͽ������ڴ����!\n");
            return(-1);
          } 
          spw_redrawmsg(&g_frame);
          return(-1);      	      	
        }
        spw_redrawmain(&g_frame);
        spw_printtowin(g_frame.main,"�ʼ����л����ļ���¼ =========================================\n");
        spw_printtowin(g_frame.main,"ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter\n");
        ilPagecount = 2;
      }
    }
    else
    {
      fprintf(fp,"%s\n","�ʼ����л����ļ���¼ =========================================");
      fprintf(fp,"%s\n","ID    Tag lLlink lRlink lSerlNo Prior O_grp O_mb P_head Filter");    	
    } 
    
    memset(alTmp,0x00,sizeof(alTmp));
    sprintf(alTmp,"%-6ld %-2d %-6ld %-6ld %-7ld %-5d %-5d %-4d %-6ld %-20s\n",i,
        slMbqueue.cTag,
        slMbqueue.lLlink,
        slMbqueue.lRlink,
        slMbqueue.lSerialnum,
        slMbqueue.iPrior,
        slMbqueue.iOrg_grp,
        slMbqueue.iOrg_mailbox,
        slMbqueue.lPack_head,
        slMbqueue.aFilter);
    if (ilFileflag == 0)
      spw_printtowin(g_frame.main,alTmp);
    else
      fprintf(fp,"%s",alTmp);
        
    ilPagecount++;
  }
  _bmqClosefile(fp_bmq);
  wrefresh(g_frame.main);
  if (ilFileflag == 1) fclose(fp);
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"S0070 �Ͽ������ڴ����!\n");
    return(-1);
  }
  spw_redrawmsg(&g_frame);
  return(0);
}

/**************************************************************
 ** ������: _bmqPrintmbpack
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqPrintmbpack_cur(char aValue[][256])
{
  int    ilRc,ilPagecount,ilFileflag = 0;
  int    y,x,ch;
  long   i;
  FILE   *fp,*fp_bmq;
  struct mbfilepack slMbfilepack;
  char alTmp[1024];

  if(aValue[0][0]!='\0')
  {
    if (aValue[1][0] != '\0')
    {
      if ((fp = fopen(aValue[1],"a+")) == NULL)
      {
        spw_print(&g_frame,"���ļ�[%s]����!\n",aValue[0]);
        return(-1);	
      }
      ilFileflag = 1;
    }
    else
    {
      spw_print(&g_frame,"����ļ�������Ϊ��!\n");
      return(-1);
    }
  }
  
  getyx(g_frame.main,y,x);
  ilPagecount = y;
  wrefresh(g_frame.main);
  if (ilFileflag == 0)
  {
    if(ilPagecount >= (LINES_MAIN-3))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);      
      ch = getch();
      if(ch == 'q' || ch == 'Q')
      {     
      	if (ilFileflag == 1) fclose(fp);
      	ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
      	spw_redrawmain(&g_frame);
      	wrefresh(g_frame.main);
        spw_redrawmsg(&g_frame);
        return(-1);       		
      }
      spw_redrawmain(&g_frame);	
      ilPagecount = 2;
    }
    spw_printtowin(g_frame.main,"�����ڴ����ݰ�================================================\n");
    spw_printtowin(g_frame.main,"ID     iMsglen lLink\n");
    ilPagecount = ilPagecount + 2;
  }
  else
  {
    fprintf(fp,"%s","�����ڴ����ݰ�================================================\n");
    fprintf(fp,"%s","ID     iMsglen lLink\n");
  }
  
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    if (ilFileflag == 1) fclose(fp);
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q! \n",ilRc);
    return(-1);
  }
  
  for(i=1;i<=psgMbshm->lMbpacksize;i++)
  {
    if(ilPagecount ==(LINES_MAIN-2))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);
      ch = getch();
      if(ch == 'q' || ch == 'Q') 
      {
      	spw_redrawmsg(&g_frame);
      	spw_redrawmain(&g_frame);
        if (ilFileflag == 1) fclose(fp);
      	ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
        spw_redrawmsg(&g_frame);   
        wrefresh(g_frame.main); 
        return(-1);
      }
      spw_redrawmain(&g_frame);
      spw_printtowin(g_frame.main,"�����ڴ����ݰ�================================================\n");
      spw_printtowin(g_frame.main,"ID     iMsglen lLink\n");
      ilPagecount = 2;
    } 
    if (psgMbpack[i-1].iMsglen)
    {
      memset(alTmp,0x00,sizeof(alTmp));	
      sprintf(alTmp,"%-6ld %7d %5ld\n",i,
        psgMbpack[i-1].iMsglen,
        psgMbpack[i-1].lLink);
      if (ilFileflag == 0)
        spw_printtowin(g_frame.main,alTmp);
      else
        fprintf(fp,"%s",alTmp);
         
      ilPagecount++;
    }
  }

  if (ilFileflag == 0)
  { 
    if(ilPagecount >= (LINES_MAIN-3))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);
      ch = getch();
      if(ch == 'q' || ch == 'Q')  
      {
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main);
        if (ilFileflag == 1) fclose(fp);
      	ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }
        spw_redrawmsg(&g_frame);  
        return(-1);       	
      }
      spw_redrawmain(&g_frame);	
      ilPagecount = 2;
    }	
    spw_printtowin(g_frame.main,"�ļ��ڴ����ݰ�================================================\n");
    spw_printtowin(g_frame.main,"ID     iMsglen     aLogfile lLogoffset\n");
    ilPagecount = ilPagecount + 2;
  }
  else
  {
    fprintf(fp,"%s","�ļ��ڴ����ݰ�================================================\n");
    fprintf(fp,"%s","ID     iMsglen     aLogfile lLogoffset\n");
  }
    
  if ((ilRc = _bmqOpenfile(1, &fp_bmq)) != SUCCESS)
  {
    spw_print(&g_frame,"���ļ�����\n");
    wrefresh(g_frame.main);
    if (ilFileflag == 1) fclose(fp);
    ilRc = _bmqShmdisconnect_cur();
    if (ilRc)
    {
      spw_print(&g_frame,"�Ͽ������ڴ����!\n");
      return(-1);
    }
    return(-1);
  }
  
  i = psgMbshm->lMbpacksize;
  for(;;)
  {
    if(ilPagecount ==(LINES_MAIN-2))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);
      ch = getch();
      if(ch == 'q' || ch == 'Q')  
      {
      	if (ilFileflag == 1) fclose(fp);
      	ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
        spw_redrawmsg(&g_frame);
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main); 
        return(-1);
      }
      spw_redrawmain(&g_frame);
      spw_printtowin(g_frame.main,"�ļ��ڴ����ݰ�================================================\n");
      spw_printtowin(g_frame.main,"ID     iMsglen lLink\n");
      ilPagecount = 2;
    } 
    ilRc = _bmqPackfile_pick_cur(fp_bmq,&slMbfilepack,++i);
    if(ilRc == FAIL || ilRc == SHMFILEEND) break;
    if(ilRc == WITHOUTUSE) continue;
    memset(alTmp,0x00,sizeof(alTmp));
    sprintf(alTmp,"%-6ld %7ld %12s %10ld\n",i,
        slMbfilepack.lLoglen,slMbfilepack.aLogfile,slMbfilepack.lLogoffset);
    if (ilFileflag == 0)
      spw_printtowin(g_frame.main,alTmp);
    else
      fprintf(fp,"%s",alTmp);
         
    ilPagecount++;
  }
  _bmqClosefile(fp_bmq); 

  ilRc = _bmqShmdisconnect_cur();
  wrefresh(g_frame.main);
  if (ilFileflag == 1) fclose(fp);
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(-1);
  }
  spw_redrawmsg(&g_frame);
  return(0);
}

/**************************************************************
 ** ������: _bmqPrintmbconnect
 ** ����:   ��ӡ��������Ϣ
 ** ����:   shizhengye
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
extern int _bmqPrintmbconnect_cur(char aValue[][256])
{
  int    i,ch,y,x;
  int 	 ilRc,ilPagecount,ilFileflag=0;
  FILE   *fp,*fp_bmq;
  struct mbconnect slMbconnect;
  char alTmp[1024];

  if(aValue[0][0]!='\0')
  {
    if (aValue[1][0] != '\0')
    {
      if ((fp = fopen(aValue[1],"a+")) == NULL)
      {
        spw_print(&g_frame,"���ļ�[%s]����! \n",aValue[0]);
        return(-1);	
      }
      ilFileflag = 1;
    }
    else
    {
      spw_print(&g_frame,"����ļ�������Ϊ��!\n");
      return(-1);
    }
  }
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    if (ilFileflag == 1) fclose(fp);
    spw_print(&g_frame,"���ӹ����ڴ�������:%d,���������³�Q! \n",ilRc);
    return(-1);
  }
  
  
  getyx(g_frame.main,y,x);
  ilPagecount = y;

  if (ilFileflag == 0)
  {
    if(ilPagecount >= (LINES_MAIN-3))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);
      ch = getch();
      if(ch == 'q' || ch == 'Q')
      {
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main); 
      	if (ilFileflag == 1) fclose(fp);
        ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }
        spw_redrawmsg(&g_frame);
        return(-1);       	
      }
      spw_redrawmain(&g_frame);	
      ilPagecount = 2;
    }
    spw_printtowin(g_frame.main,"�����������ջ ===============================================\n");
    spw_printtowin(g_frame.main,"ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
    ilPagecount = ilPagecount + 2;
  }
  else
  {
    fprintf(fp,"%s","�����������ջ ===============================================\n");
    fprintf(fp,"%s","ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
  }
  
  for(i=1;i<=psgMbshm->iMbcontcount;i++)
  {
    if(ilPagecount ==(LINES_MAIN-2))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);      
      ch = getch();
      if(ch == 'q' || ch == 'Q')  
      {
      	if (ilFileflag == 1) fclose(fp);
      	ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
      	spw_redrawmsg(&g_frame);
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main); 
        return(-1);
      }
      spw_redrawmain(&g_frame);
      spw_printtowin(g_frame.main,"�����������ջ ===============================================\n");
      spw_printtowin(g_frame.main,"ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
      ilPagecount = 2;
    } 
    if (psgMbconnect[i-1].cTag)
    {
      memset(alTmp,0x00,sizeof(alTmp));
      sprintf(alTmp,"%-6d %-4d %-6d %-6d %-6d %-20s %-6ld\n",i,
        psgMbconnect[i-1].cTag,
        psgMbconnect[i-1].iLlink,
        psgMbconnect[i-1].iRlink,
        psgMbconnect[i-1].iPrior,
        psgMbconnect[i-1].aFilter,
        psgMbconnect[i-1].lConnpid);
      if (ilFileflag == 0)
        spw_printtowin(g_frame.main,alTmp);
      else
        fprintf(fp,"%s",alTmp);
         
      ilPagecount++;
    }
  }

  if (ilFileflag == 0)
  {
    if(ilPagecount >= (LINES_MAIN-3))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);      
      ch = getch();
      if(ch == 'q' || ch == 'Q')
      {
      	if (ilFileflag == 1) fclose(fp);
      	ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main); 
        ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
        spw_redrawmsg(&g_frame);    
        return(-1);       	
      }
      spw_redrawmain(&g_frame);	
      ilPagecount = 2;
    }
    spw_printtowin(g_frame.main,"�������ӻ����ļ���¼ =========================================\n");
    spw_printtowin(g_frame.main,"ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
    ilPagecount = ilPagecount + 2;
  }
  else
  {
    fprintf(fp,"%s","�������ӻ����ļ���¼ =========================================\n");
    fprintf(fp,"%s","ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
  }
  
  if ((ilRc = _bmqOpenfile(2, &fp_bmq)) != SUCCESS)
  {
    spw_print(&g_frame,"���ļ�����\n");
    wrefresh(g_frame.main);
    if (ilFileflag == 1) fclose(fp);
    ilRc = _bmqShmdisconnect_cur();
    if (ilRc)
    {
      spw_print(&g_frame,"�Ͽ������ڴ����!\n");
      return(-1);
    }
    return(-1);
  }
  
  i = psgMbshm->iMbcontcount;
  for(;;)
  {
    ilRc = _bmqConnfile_pick(fp_bmq,&slMbconnect,++i);
      if(ilRc == FAIL || ilRc == SHMFILEEND) break;
      if(ilRc == WITHOUTUSE) continue;
    if(ilPagecount ==(LINES_MAIN-2))
    {
      wrefresh(g_frame.main);
      mvwaddstr(g_frame.msg,0,0,"                 Q���˳�,����������...        \n");
      wrefresh(g_frame.msg);      
      ch = getch();
      if(ch == 'q' || ch == 'Q')
      {  
      	if (ilFileflag == 1) fclose(fp);
      	ilRc = _bmqShmdisconnect_cur();
        if (ilRc)
        {
          spw_print(&g_frame,"�Ͽ������ڴ����!\n");
          return(-1);
        }  
      	spw_redrawmsg(&g_frame);
      	spw_redrawmain(&g_frame);
        wrefresh(g_frame.main); 
        return(-1);
      }
      spw_redrawmain(&g_frame);
      spw_printtowin(g_frame.main,"�������ӻ����ļ���¼ =========================================\n");
      spw_printtowin(g_frame.main,"ID     cTag iLlink iRlink iPrior aFilter              lPid\n");
      ilPagecount = 2;
    }
    memset(alTmp,0x00,sizeof(alTmp));
    sprintf(alTmp,"%-6d %-4d %-6d %-6d %-6d %-20s %-6ld\n",i,
        slMbconnect.cTag,
        slMbconnect.iLlink,
        slMbconnect.iRlink,
        slMbconnect.iPrior,
        slMbconnect.aFilter,
        slMbconnect.lConnpid);
    if (ilFileflag == 0)
        spw_printtowin(g_frame.main,alTmp);
      else
        fprintf(fp,"%s",alTmp);
         
      ilPagecount++;
  }
  _bmqClosefile(fp_bmq);
  if (ilFileflag == 1) fclose(fp);
  wrefresh(g_frame.main);
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(-1);
  }
  spw_redrawmsg(&g_frame);
  return(0);
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
int _bmqPrintmball_cur(char aValue[][256])
{
  int ilRc;
  ilRc = _bmqPrintmbinfo_cur(aValue);
  if(ilRc == -1) return (ilRc);
  ilRc = _bmqPrintmbqueue_cur(aValue);
  if(ilRc == -1) return (ilRc);
  ilRc = _bmqPrintmbpack_cur(aValue);
  if(ilRc == -1) return (ilRc);
  ilRc = _bmqPrintmbconnect_cur(aValue);
  if(ilRc == -1) return (ilRc);
  return(0);
}

/****************************************************************/
/* ģ����    ��BMQSETLOAD                                     */
/* ģ������    ����������ģ��                                   */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    ��xujun                                          */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ��				                */
/****************************************************************/
int _bmqReloadfilesvr_cur(char aValue[][256])
{
  int ilRc,ch;
  char  alFp_name[80];
  char  alPro_name[20];  
  
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d\n",ilRc);
    return(FAIL);
  }
  spw_print(&g_frame,"ȷ��Ҫ���� �ļ�������� ��? [y/n]   ");
  ch = getch();
  waddch(g_frame.msg,ch);
  waddch(g_frame.msg,'\n');
  wrefresh(g_frame.msg);
  if ((ch != 'Y') && (ch != 'y'))
  {
    spw_print(&g_frame,"ѡ�������ļ��������!\n");
    return(SUCCESS);
  }
  else
  {
    if( psgMbshm->lBmqfilecls_pid > 0 )
      kill(psgMbshm->lBmqfilecls_pid,SIGTERM);
    if( psgMbshm->lBmqfilemng_pid > 0 )
      kill(psgMbshm->lBmqfilemng_pid,SIGTERM);
    if( psgMbshm->lBmqfilesaf_pid > 0 )
      kill(psgMbshm->lBmqfilesaf_pid,SIGTERM);
    if( psgMbshm->lBmqfilelog_pid > 0 )
      kill(psgMbshm->lBmqfilelog_pid,SIGTERM);
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqfilecls",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqfilecls");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      exit(0);
    }
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqfilemng",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqfilemng");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      exit(0);
    }
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqfilesaf",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqfilesaf");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      exit(0);
    }
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqfilelog",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqfilelog");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      exit(0);
    }
  }
  spw_print(&g_frame,"�ļ��������������!\n"); 
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  } 
  return(0);	
}


int _bmqReloadgrprcv_cur(char aValue[][256])
{
  int ilRc,ch;
  char  alFp_name[80];
  char  alPro_name[20];  
  
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d\n",ilRc);
    return(FAIL);
  }
  spw_print(&g_frame,"ȷ��Ҫ���� bmqGrp_rcv��bmqGrp_snd ��?[y/n]   ");
  ch = getch();
  waddch(g_frame.msg,ch);
  waddch(g_frame.msg,'\n');
  wrefresh(g_frame.msg);
  if ((ch != 'Y') && (ch != 'y'))
  {
    spw_print(&g_frame,"ѡ������ bmqGrp_rcv��bmqGrp_snd! \n");
    return(SUCCESS);
  }
  else
  {
    if( psgMbshm->lBmqgrprcv_pid > 0 )
    kill(psgMbshm->lBmqgrprcv_pid,SIGTERM);
    if( psgMbshm->lBmqgrpsnd_pid > 0 )
    kill(psgMbshm->lBmqgrpsnd_pid,SIGTERM);
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqGrp_snd",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqGrp_snd");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      return(0);
    }
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqGrp_rcv",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqGrp_rcv");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      return(0);
    }
  }
  spw_print(&g_frame,"bmqGrp_rcv��bmqGrp_snd������!\n");  
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  }
  return(0);
	
}

int _bmqReloadcls_cur(char aValue[][256])
{
  int ilRc,ch;
  char  alFp_name[80];
  char  alPro_name[20];  
  
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d\n",ilRc);
    return(FAIL);
  }
  spw_print(&g_frame,"ȷ��Ҫ���� bmqCls ��? [y/n]   ");
  ch = getch();
  waddch(g_frame.msg,ch);
  waddch(g_frame.msg,'\n');
  wrefresh(g_frame.msg);
  if ((ch != 'Y') && (ch != 'y'))
  {
    spw_print(&g_frame,"ѡ������bmqCls!\n");
    return(SUCCESS);
  }
  else
  {
    if( psgMbshm->lBmqcls_pid > 0 )
      kill(psgMbshm->lBmqcls_pid,SIGTERM);
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqcls",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqcls");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      return(0);
    }
  }
  spw_print(&g_frame,"bmqCls������!\n"); 
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  } 
  return(0);			
}

int _bmqReloadclean_cur(char aValue[][256])
{
  int ilRc,ch;
  char  alFp_name[80];
  char  alPro_name[20];
  
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d\n",ilRc);
    return(FAIL);
  }
  spw_print(&g_frame,"ȷ��Ҫ���� bmqClean ��? [y/n]   ");
  ch = getch();
  waddch(g_frame.msg,ch);
  waddch(g_frame.msg,'\n');
  wrefresh(g_frame.msg);
  if ((ch != 'Y') && (ch != 'y'))
  {
    spw_print(&g_frame,"ѡ������bmqClean!\n");
    return(SUCCESS);
  }
  else
  {
    if( psgMbshm->lBmqclean_pid > 0 )
      kill(psgMbshm->lBmqclean_pid,SIGTERM);
    if( fork() == 0)
    {
      sprintf(alFp_name,"%s/bin/bmqclean",getenv("BMQ_PATH"));
      sprintf(alPro_name,"%s","bmqclean");
      ilRc = execl(alFp_name,alPro_name,(char *)0);
      return(0);
    }
  }
  spw_print(&g_frame,"bmqClean������!\n"); 
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  } 
  return(0);		
}

int _bmqReloadcfg_cur(char aValue[][256])
{
  int ilRc,ch;
 
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    spw_print(&g_frame,"���ӹ����ڴ�������:%d\n",ilRc);
    return(FAIL);
  }
  spw_print(&g_frame,"ȷ��Ҫˢ��[mail]������? [y/n]   ");
  ch = getch();
  waddch(g_frame.msg,ch);
  waddch(g_frame.msg,'\n');
  wrefresh(g_frame.msg);
  if ((ch != 'Y') && (ch != 'y'))
  {
    spw_print(&g_frame,"ѡ��ˢ��[mail]����!\n");
    return(SUCCESS);
  }
  else
  {
    ilRc = _bmqLock(LK_ALL);
    if( ilRc )
    {
      spw_print(&g_frame,"�źŵƲ���ʧ��");
      spw_print(&g_frame,"[MAIL]��������δˢ��!\n");
      return(FAIL);
    }
    /*��������פ��ʱ��ͷ�ֵ����*/
    ilRc = _bmqConfig_load("MAIL",0);
    if (ilRc < 0)
    {
      spw_print(&g_frame,"����MAIL����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini\n",
        getenv("BMQ_PATH"));
      spw_print(&g_frame,"[MAIL]��������δˢ��!\n");
      return(FAIL);
    }
    _bmqUnlock(LK_ALL);
    spw_print(&g_frame,"[MAIL]����������ˢ��!\n");
  }
  ilRc = _bmqShmdisconnect_cur();
  if (ilRc)
  {
    spw_print(&g_frame,"�Ͽ������ڴ����!\n");
    return(ilRc);
  } 
  return(0);		
}
