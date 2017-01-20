/****************************************************************/
/* ģ����    ��BMQCLEAN                                       */
/* ģ������    ����������ģ��                                   */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���³�Q�����������ܵ�ʵ�֡��������ӵ����        */
/*               �˿ڹ��������ؾ���                           */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2001.08.17 ���V2.0,����355��                              */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"

int main(int argc,char *argv[])
{
  long   llCurrentTime;				/* ϵͳ��ǰʱ�� */
  long   llQhead,llPack_head,llMsgid;
  short  ilConn_head,ilConn_head_tmp;
  int    ilRc,i,ilSize,ilSize1;
  FILE   *fp;
  char   alBuf[iMBMAXPACKSIZE+100];
  /*short  ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  long   llRlink;
  short  ilTmp;
  struct mbqueue   slMbqueue;
  struct mbconnect slMbconnect;
  char   alFilter[iMASKLEN];

  /*�����ź�*/
  _bmqSignalinit();

  /* �����ػ����� */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010 %s :�����ػ�����ʧ�ܣ��˳�!",__FILE__);
    exit(FAIL);
  }

  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0020 %s :���ӹ����ڴ�������:%d",__FILE__,ilRc);
    exit(FAIL);
  }
  
  if(psgMbshm->lBmqclean_pid)
  {
    if( kill(psgMbshm->lBmqclean_pid,0) == 0 ) 
    {
      printf("\n�������������� bmqclean �Ѿ������������ظ�����!\n");
      exit(-1);
    }
  }
  ilRc = bmqOpen(psgMbshm->iMbinfo);
  if(ilRc)
  {
    _bmqDebug("S0030 %s :�򿪹�������[%d]����:%d",
      __FILE__,psgMbshm->iMbinfo,ilRc);
    exit(FAIL);
  }

  /*ɾ����bmqOpen()��������Ϣ����*/
  _bmqLock(LK_ALL);
  llMsgid = _bmqGetmsgid(igMb_sys_current_connect);
  ilRc = msgctl(llMsgid,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1850 ɾ����Ϣ���� msgctl(RM-recv) ���� errno:%d,%s",errno,strerror(errno));
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }
  _bmqUnlock(LK_ALL);

   /* ��ȡϵͳ��ѭʱ�� */
  ilRc = _bmqConfig_load("WTIME",0);
  if (ilRc == -1)
  {
    _bmqDebug("S0040 %s :����WTIME����ʧ�ܣ���鿴�����ļ�%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    bmqClose();
    exit(FAIL);
  }
  ilSize  = sizeof(struct mbqueue) + 1;
  ilSize1 = sizeof(struct mbconnect) + 1;

  /*�������pid*/
  psgMbshm->lBmqclean_pid = getpid();

  _bmqDebug("S0050 ***������С������������***");
  while(1)
  {
    /*�ȴ�ָ����ѯʱ��*/
    sleep(lgWAITTIME);
    /* ɾ������פ��ʱ��Ķ��кͱ���*/
    for(i=0;i<psgMbshm->iMbinfo;i++)
    {
      /* �����޶��л�δ�ƶ�����ʱ��,continue*/
      if( psgMbinfo[i].lQueue_head == 0 ) continue;
      if( psgMbinfo[i].lExisttime == 0)   continue;
    
      llQhead = psgMbinfo[i].lQueue_head;
      while(llQhead)
      {
        /*ȡϵͳ��ǰʱ��*/
        time(&llCurrentTime);
        /*��¼���ļ���*/
        if(llQhead > psgMbshm->lMbqueuecount)
        {
          /* �򿪶��л����ļ�,ȡ����¼ */
          if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
          {
            _bmqDebug("S0060 %s:�򿪶��л����ļ�����",__FILE__);
            continue;
          }
          _bmqQueuefile_pick(fp,&slMbqueue,llQhead);
 
          /*����ʱ���*/
          llCurrentTime = llCurrentTime - slMbqueue.lBegintime;   

          /*������������*/
          if( llCurrentTime >= psgMbinfo[i].lExisttime)
          {
            ilRc = _bmqLock(LK_ALL);
            if( ilRc ) break;

            llRlink = slMbqueue.lRlink;
            /*���м�¼ɾ��*/
            ilRc = _bmqQueue_del(i + 1,&slMbqueue,&ilTmp,&ilTmp,&ilTmp,alFilter,
              &llPack_head); 
            if(ilRc)
            {
              _bmqDebug("S0070 %s:ɾ��һ�����м�¼ʧ��",__FILE__);
              _bmqUnlock(LK_ALL);
              _bmqClosefile(fp);
              break;
            }

            /*���û����ļ���¼*/
            alBuf[0] = '0';
            fseek(fp,0,SEEK_SET);
            ilRc = _bmqPutfile(fp, (llQhead-psgMbshm->lMbqueuecount-1)*ilSize,
              ilSize,alBuf);
            if (ilRc)
            {
              _bmqDebug("S0080 %s:���û����ļ���¼ʧ��",__FILE__);
              _bmqClosefile(fp);
              _bmqUnlock(LK_ALL);
              break;
            }

            _bmqClosefile(fp);
            /*ɾ������*/
            ilRc = _bmqPack_get(alBuf,&ilMsglen,llPack_head);
            if( ilRc )
            {
              _bmqDebug("S0090 %s:��ȡ����ʧ��",__FILE__);
              _bmqUnlock(LK_ALL);
              break;
            }
            psgMbinfo[i].lPendnum--;
            llQhead = llRlink;
            _bmqUnlock(LK_ALL);
          }
          else
          {
            llQhead = slMbqueue.lRlink;
            _bmqClosefile(fp);
          }
        } 
        /*��¼�ڹ����ڴ���*/
        else
        { 
          llCurrentTime = llCurrentTime - psgMbqueue[llQhead-1].lBegintime;
          /*������������*/
          if( llCurrentTime >= psgMbinfo[i].lExisttime)
          {
            ilRc = _bmqLock(LK_ALL);
            if( ilRc ) break;
            
            llRlink = psgMbqueue[llQhead - 1].lRlink;           
            /*���м�¼ɾ��*/
            ilRc = _bmqQueue_del(i + 1,&psgMbqueue[llQhead-1],&ilTmp,&ilTmp,
              &ilTmp,alFilter,&llPack_head);
            if(ilRc)
            {
              _bmqDebug("S0100 %s:ɾ��һ�����м�¼ʧ��",__FILE__);
              _bmqUnlock(LK_ALL);
              break;
            }

            /*��ȡ����*/
            ilRc = _bmqPack_get(alBuf,&ilMsglen,llPack_head);
            if( ilRc )
            {
              _bmqDebug("S0110 %s:��ȡ����ʧ��",__FILE__);
              _bmqUnlock(LK_ALL);
              break;
            }
            psgMbinfo[i].lPendnum--;
            llQhead = llRlink;
            _bmqUnlock(LK_ALL);
          }
          else
            llQhead = psgMbqueue[llQhead-1].lRlink;
        }
      }   /*while(llQhead)*/
    }     /*for(Mbinfo)*/


    /*ɾ�����ӽ��̲����ڵ����Ӽ�¼*/
    for( i=0; i<psgMbshm->iMbinfo; i++)
    {
      if(psgMbinfo[i].iConnect == 0) continue;
      ilConn_head = psgMbinfo[i].iConn_head; 
 
      while(ilConn_head)
      { 
        /*��¼���ļ���*/
        if(ilConn_head > psgMbshm->iMbcontcount)
        {
          /*�����ӻ����ļ�*/
          if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
          {
            _bmqDebug("S0120 %s:�����ӻ����ļ�����",__FILE__);
            break;
          }
          /*ȡ��ָ����¼*/
          ilRc = _bmqConnfile_pick(fp,&slMbconnect,ilConn_head);
          ilConn_head_tmp = slMbconnect.iRlink;
          /*�жϽ����Ƿ����*/
          if( kill(slMbconnect.lConnpid,0) != 0 )    
          {
            if(errno != 3) break;
            ilRc = _bmqLock(LK_ALL);
            if( ilRc ) break;

            /*ɾ���ļ���¼*/
            ilRc = _bmqConnect_del(i,&slMbconnect);
            if(ilRc == FAIL)
            {
              _bmqDebug("S0130 ���ļ���ɾ�������ڵ����");
              _bmqUnlock(LK_ALL);
              break;
            }
            alBuf[0] = '0';
            memcpy(alBuf + 1, (char *)&slMbconnect, ilSize1 - 1);

            fseek(fp,0,SEEK_SET);
            ilRc = _bmqPutfile(fp,(ilConn_head-psgMbshm->iMbcontcount-1)*ilSize1
                      ,ilSize1,alBuf);
            if (ilRc)
            {
              _bmqDebug("S0140 %s:��д���ӻ����ļ�����",__FILE__);
              _bmqUnlock(LK_ALL);
              _bmqClosefile(fp);
              break;
            }
            _bmqUnlock(LK_ALL);
          }
          ilConn_head = ilConn_head_tmp;
          _bmqClosefile(fp); 
        }
        else
        {
          ilConn_head_tmp = psgMbconnect[ilConn_head-1].iRlink;
          if( kill(psgMbconnect[ilConn_head-1].lConnpid,0) != 0 )
          { 
            if(errno != 3) break;
            ilRc = _bmqLock(LK_ALL);
            if( ilRc ) break;

            ilRc = _bmqConnect_del(i,&psgMbconnect[ilConn_head-1]);
            if(ilRc == FAIL)
            {
              _bmqDebug("S0150 ���ڴ���ɾ�������ڵ����");
              _bmqUnlock(LK_ALL);
              break;
            }
            _bmqUnlock(LK_ALL);
          }
          ilConn_head = ilConn_head_tmp;
        }
      }    /*while(ilConn_head)*/
    }     /*for(Mbinfo)*/
    /*�¼����������ؾ���*/
    for( i=0; i<psgMbshm->iMbinfo; i++)
    { 
      if( (psgMbinfo[i].iHload == 0) || (psgMbinfo[i].iLload == 0) ) continue;
      /*�������ٽ��*/
      if( psgMbinfo[i].lPendnum <= psgMbinfo[i].iLload )
      {
        if(psgMbinfo[i].iStatus == 4)
          psgMbinfo[i].iLastnum++;
        else
        {
          psgMbinfo[i].iStatus  = 4;
          psgMbinfo[i].iLastnum = 1;
        }
        if( psgMbinfo[i].iLastnum > psgMbinfo[i].iTimes )
        {
          /*
          slMbmanger.iMsgType = 999;
          slMbmanger.iMbid    = i+1;
          slMbmanger.iStatus  = psgMbinfo[i].iStatus;
          ilRc =  bmqPut(0,iMBTASK,0,0,0,(char *)&slMbmanger,
            sizeof(struct mbmanger));
          if(ilRc)
          {
            _bmqDebug("S0160 [%d]�������������[%d]���ͱ���[%d]ʧ��:%d",
              i+1,psgMbshm->iMbinfo,psgMbinfo[i].iStatus,ilRc);
            continue;
          }
          */
          psgMbinfo[i].iLastnum = 0;
          psgMbinfo[i].iStatus  = 1;
        }  
        continue;
      } 
      /*�������ٽ��*/
      if( psgMbinfo[i].lPendnum >= psgMbinfo[i].iHload )
      {
        if(psgMbinfo[i].iStatus == 5)
          psgMbinfo[i].iLastnum++;
        else
        {
          psgMbinfo[i].iStatus  = 5;
          psgMbinfo[i].iLastnum = 1;
        }
        if( psgMbinfo[i].iLastnum > psgMbinfo[i].iTimes )
        {
          /*
          memset(&slMbmanger,0x00,sizeof(struct mbmanger));
          slMbmanger.iMsgType = 999;
          slMbmanger.iMbid    = i+1;
          slMbmanger.iStatus  = psgMbinfo[i].iStatus;
          ilRc = bmqPut(0,iMBTASK,0,0,0,(char *)&slMbmanger,
            sizeof(struct mbmanger));
          if(ilRc)
          {
            _bmqDebug("S0170 [%d]�������������[%d]���ͱ���[%d]ʧ��:%d",
              i+1,psgMbshm->iMbinfo,psgMbinfo[i].iStatus,ilRc);
            continue;
          }
          */
          psgMbinfo[i].iStatus  = 3;
          psgMbinfo[i].iLastnum = 0;
        }
        continue;
      }
      psgMbinfo[i].iStatus = psgMbinfo[i].iLastnum = 0;
    }    /* for(Mb)  */
  }     /* while(1) */
}      /* main end */
