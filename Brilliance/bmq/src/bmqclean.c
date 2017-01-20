/****************************************************************/
/* 模块编号    ：BMQCLEAN                                       */
/* 模块名称    ：邮箱清理模块                                   */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2001/08/08                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：新晨Q邮箱蒸发功能的实现、垃圾连接的清除        */
/*               端口管理－－负载均衡                           */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共355行                              */
/****************************************************************/

/*库函数定义*/
#include "bmq.h"

int main(int argc,char *argv[])
{
  long   llCurrentTime;				/* 系统当前时间 */
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

  /*设置信号*/
  _bmqSignalinit();

  /* 创建守护进程 */
  if (_bmqDaemon_creat() == FAIL)
  {
    _bmqDebug("S0010 %s :创建守护进程失败，退出!",__FILE__);
    exit(FAIL);
  }

  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    _bmqDebug("S0020 %s :连接共享内存区出错:%d",__FILE__,ilRc);
    exit(FAIL);
  }
  
  if(psgMbshm->lBmqclean_pid)
  {
    if( kill(psgMbshm->lBmqclean_pid,0) == 0 ) 
    {
      printf("\n邮箱事务管理进程 bmqclean 已经启动，不能重复启动!\n");
      exit(-1);
    }
  }
  ilRc = bmqOpen(psgMbshm->iMbinfo);
  if(ilRc)
  {
    _bmqDebug("S0030 %s :打开管理邮箱[%d]出错:%d",
      __FILE__,psgMbshm->iMbinfo,ilRc);
    exit(FAIL);
  }

  /*删除由bmqOpen()创建的消息队列*/
  _bmqLock(LK_ALL);
  llMsgid = _bmqGetmsgid(igMb_sys_current_connect);
  ilRc = msgctl(llMsgid,IPC_RMID,0);
  if(ilRc < 0)
  {
    _bmqDebug("S1850 删除消息队列 msgctl(RM-recv) 出错 errno:%d,%s",errno,strerror(errno));
    _bmqUnlock(LK_ALL);
    return(FAIL);
  }
  _bmqUnlock(LK_ALL);

   /* 读取系统轮循时间 */
  ilRc = _bmqConfig_load("WTIME",0);
  if (ilRc == -1)
  {
    _bmqDebug("S0040 %s :载入WTIME参数失败，请查看参数文件%s/etc/bmq.ini",
      __FILE__,getenv("BMQ_PATH"));
    bmqClose();
    exit(FAIL);
  }
  ilSize  = sizeof(struct mbqueue) + 1;
  ilSize1 = sizeof(struct mbconnect) + 1;

  /*保存进程pid*/
  psgMbshm->lBmqclean_pid = getpid();

  _bmqDebug("S0050 ***邮箱队列、连接清理进程***");
  while(1)
  {
    /*等待指定轮询时间*/
    sleep(lgWAITTIME);
    /* 删除超过驻留时间的队列和报文*/
    for(i=0;i<psgMbshm->iMbinfo;i++)
    {
      /* 邮箱无队列或未制定阻留时间,continue*/
      if( psgMbinfo[i].lQueue_head == 0 ) continue;
      if( psgMbinfo[i].lExisttime == 0)   continue;
    
      llQhead = psgMbinfo[i].lQueue_head;
      while(llQhead)
      {
        /*取系统当前时间*/
        time(&llCurrentTime);
        /*记录在文件中*/
        if(llQhead > psgMbshm->lMbqueuecount)
        {
          /* 打开队列缓存文件,取出记录 */
          if ((ilRc = _bmqOpenfile(0, &fp)) != SUCCESS)
          {
            _bmqDebug("S0060 %s:打开队列缓存文件出错",__FILE__);
            continue;
          }
          _bmqQueuefile_pick(fp,&slMbqueue,llQhead);
 
          /*计算时间差*/
          llCurrentTime = llCurrentTime - slMbqueue.lBegintime;   

          /*符合蒸发条件*/
          if( llCurrentTime >= psgMbinfo[i].lExisttime)
          {
            ilRc = _bmqLock(LK_ALL);
            if( ilRc ) break;

            llRlink = slMbqueue.lRlink;
            /*队列记录删除*/
            ilRc = _bmqQueue_del(i + 1,&slMbqueue,&ilTmp,&ilTmp,&ilTmp,alFilter,
              &llPack_head); 
            if(ilRc)
            {
              _bmqDebug("S0070 %s:删除一条队列记录失败",__FILE__);
              _bmqUnlock(LK_ALL);
              _bmqClosefile(fp);
              break;
            }

            /*重置缓存文件记录*/
            alBuf[0] = '0';
            fseek(fp,0,SEEK_SET);
            ilRc = _bmqPutfile(fp, (llQhead-psgMbshm->lMbqueuecount-1)*ilSize,
              ilSize,alBuf);
            if (ilRc)
            {
              _bmqDebug("S0080 %s:重置缓存文件记录失败",__FILE__);
              _bmqClosefile(fp);
              _bmqUnlock(LK_ALL);
              break;
            }

            _bmqClosefile(fp);
            /*删除报文*/
            ilRc = _bmqPack_get(alBuf,&ilMsglen,llPack_head);
            if( ilRc )
            {
              _bmqDebug("S0090 %s:提取报文失败",__FILE__);
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
        /*记录在共享内存中*/
        else
        { 
          llCurrentTime = llCurrentTime - psgMbqueue[llQhead-1].lBegintime;
          /*符合蒸发条件*/
          if( llCurrentTime >= psgMbinfo[i].lExisttime)
          {
            ilRc = _bmqLock(LK_ALL);
            if( ilRc ) break;
            
            llRlink = psgMbqueue[llQhead - 1].lRlink;           
            /*队列记录删除*/
            ilRc = _bmqQueue_del(i + 1,&psgMbqueue[llQhead-1],&ilTmp,&ilTmp,
              &ilTmp,alFilter,&llPack_head);
            if(ilRc)
            {
              _bmqDebug("S0100 %s:删除一条队列记录失败",__FILE__);
              _bmqUnlock(LK_ALL);
              break;
            }

            /*提取报文*/
            ilRc = _bmqPack_get(alBuf,&ilMsglen,llPack_head);
            if( ilRc )
            {
              _bmqDebug("S0110 %s:提取报文失败",__FILE__);
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


    /*删除连接进程不存在的连接记录*/
    for( i=0; i<psgMbshm->iMbinfo; i++)
    {
      if(psgMbinfo[i].iConnect == 0) continue;
      ilConn_head = psgMbinfo[i].iConn_head; 
 
      while(ilConn_head)
      { 
        /*记录在文件中*/
        if(ilConn_head > psgMbshm->iMbcontcount)
        {
          /*打开连接缓存文件*/
          if ((ilRc = _bmqOpenfile(2, &fp)) != SUCCESS)
          {
            _bmqDebug("S0120 %s:打开连接缓存文件出错",__FILE__);
            break;
          }
          /*取出指定记录*/
          ilRc = _bmqConnfile_pick(fp,&slMbconnect,ilConn_head);
          ilConn_head_tmp = slMbconnect.iRlink;
          /*判断进程是否存在*/
          if( kill(slMbconnect.lConnpid,0) != 0 )    
          {
            if(errno != 3) break;
            ilRc = _bmqLock(LK_ALL);
            if( ilRc ) break;

            /*删除文件记录*/
            ilRc = _bmqConnect_del(i,&slMbconnect);
            if(ilRc == FAIL)
            {
              _bmqDebug("S0130 从文件中删除垃圾节点错误");
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
              _bmqDebug("S0140 %s:重写连接缓存文件出错",__FILE__);
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
              _bmqDebug("S0150 从内存中删除垃圾节点错误");
              _bmqUnlock(LK_ALL);
              break;
            }
            _bmqUnlock(LK_ALL);
          }
          ilConn_head = ilConn_head_tmp;
        }
      }    /*while(ilConn_head)*/
    }     /*for(Mbinfo)*/
    /*事件管理－－负载均衡*/
    for( i=0; i<psgMbshm->iMbinfo; i++)
    { 
      if( (psgMbinfo[i].iHload == 0) || (psgMbinfo[i].iLload == 0) ) continue;
      /*低于下临界点*/
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
            _bmqDebug("S0160 [%d]邮箱向管理邮箱[%d]发送报文[%d]失败:%d",
              i+1,psgMbshm->iMbinfo,psgMbinfo[i].iStatus,ilRc);
            continue;
          }
          */
          psgMbinfo[i].iLastnum = 0;
          psgMbinfo[i].iStatus  = 1;
        }  
        continue;
      } 
      /*高于上临界点*/
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
            _bmqDebug("S0170 [%d]邮箱向管理邮箱[%d]发送报文[%d]失败:%d",
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
