/****************************************************************/
/* 模块编号    ：BMQSHUTDOWN                                    */
/* 模块名称    ：                                               */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2001/08/08                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：新晨Q关闭                                      */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共28行                               */
/* 2006.12.20 增加关闭时保存邮箱报文的功能,共192行              */
/****************************************************************/

/*库函数定义*/
#include "bmq.h"

/*声明函数*/
int _bmqShmsave();

/*定义全局变量*/
char *agPath,agFile[100];
char agBmqfile[2][100]={"/bmqlog/logfile/sendfile.rec","/bmqlog/logfile/packsave.rec"};

int main(int argc,char **argv)
{ 
  int   ilRc;
  char  ch;  
  int   i;
  
  if(argc == 2)
  {
    if ((strcmp(argv[1],"-y")==0)||(strcmp(argv[1],"-Y")==0))
    {
      ilRc = _bmqShutdown();
      if (ilRc == 0) printf("新晨-Q 成功关闭!\n");
      else printf("--%d--新晨-Q 关闭出错!\n",ilRc);      
      exit(0);
    }
  }  
  
  printf("关闭新晨-Q时是否要保存内存中的数据?[Y/N]\n");  
  ch = getchar();
  switch(ch)
  {
    case 'Y':
    case 'y':
      ilRc=_bmqShmsave();/*调用函数保存内存中的数据到文件中*/
      if(ilRc)/*保存过程中出错*/
      {
         if ((agPath = getenv("BMQ_PATH")) == NULL)
         {
           _bmqDebug("读系统环境变量[BMQ_PATH]失败\n");
           return(FAIL);
         }
         for(i=0;i<2;i++)
         {
           memset(agFile,0x00,sizeof(agFile));
           strcpy(agFile,agPath);
           strcat(agFile,agBmqfile[i]);
           unlink(agFile);
         } 
      }
      ilRc = _bmqShutdown();
      if (ilRc == 0) printf("新晨-Q 成功关闭!\n");
      else printf("--%d--新晨-Q 关闭出错!\n",ilRc);
      break;
      
    default:
      ilRc = _bmqShutdown();
      if (ilRc == 0) printf("新晨-Q 成功关闭!\n");
      else printf("--%d--新晨-Q 关闭出错!\n",ilRc);      
      break;
  }
       
  return(SUCCESS);
}

int _bmqShmsave()
{ 
  FILE   *fp;
  int    ilRc;
  int    i;
  short  ilOrgGrpid;
  short  ilOrgMbid;
  short  ilPrior;
  char   alFilter[iMASKLEN];
  char   alMask[iMASKLEN];
  char   alMsgbuf[iMBMAXPACKSIZE];
  /*short  ilMsglen; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  struct packsave slPacksave;
  
  agPath = getenv("BMQ_PATH");
  if (agPath == NULL)
  {
    _bmqDebug("读系统环境变量[BMQ_PATH]失败\n");
    return(FAIL);
  }
  
  /*连接共享内存区*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    printf("%s :连接共享内存区出错:%d\n",__FILE__,ilRc);
    exit(FAIL);
  }
  
  /*保存文件传输*/
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[0]);  
  fp=fopen(agFile,"w+");
  if(fp == NULL)
  { 
    printf("打开文件sendfile.rec不成功!\n");
    return(FAIL);
  }  
  for ( i =0 ; i < psgMbshm->iFilecount; i++ )
  { 
    if (psgMbfilemng[i].iFlag == 0)/*找到未使用记录段*/ continue;
    psgMbfilemng[i].iFlag = 1;
    ilRc = fwrite((char *)&psgMbfilemng[i],sizeof(char),sizeof(struct monmsg),fp);
    if(ilRc != sizeof(struct monmsg))
    { 
      printf("写文件sendfile.rec出错!\n");
      fclose(fp);
      return(FAIL);
    }    
  }
  fclose(fp);
  printf("保存内存文件传输数据成功!\n");
  

  /*保存报文信息*/
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[1]);
  fp=fopen(agFile,"w+");
  if(fp == NULL)
  {
    printf("打开文件packsave.rec不成功!\n");
    return(FAIL);  	
  }  

  for ( i = 0; i < psgMbshm->iMbinfo; i++)
  { 
    if (psgMbinfo[i].iFlag == 0)/*邮箱未启用*/ continue;
    if (psgMbinfo[i].lPendnum == 0) continue;/*邮箱当前消息数为0*/
      
    ilRc = bmqOpen(i+1);
    if (ilRc)
    {
      printf("打开邮箱出错\n");
      return(FAIL);
    }
    
    for( ; ; )
    { 
      ilOrgGrpid = ilOrgMbid = ilPrior = 0;
      memset(alMask,0x00,sizeof(alMask));
      memset(alFilter,0x00,sizeof(alFilter));
      ilMsglen = iMBMAXPACKSIZE;
      memset(alMsgbuf,0x00,ilMsglen);
      
      ilRc = bmqGetfilter(&ilOrgGrpid,&ilOrgMbid,&ilPrior,alFilter,alMask,alMsgbuf,&ilMsglen);
      if(ilRc)       
         break;
      
      /*赋值到新定义的结构;*/
      memset(&slPacksave,0x00,sizeof(struct packsave));
      slPacksave.iMbid = i+1; 
      slPacksave.iOrgGrpid = ilOrgGrpid;
      slPacksave.iOrgMbid =  ilOrgMbid;
      slPacksave.iPrior = ilPrior;    
      memcpy(slPacksave.aFilter,alFilter,iMASKLEN);
      memcpy(slPacksave.aMsgbuf,alMsgbuf,ilMsglen);
      slPacksave.iMsglen = ilMsglen;      
      ilRc = fwrite((char *)&slPacksave,sizeof(char),sizeof(struct packsave),fp );
      if(ilRc != sizeof(struct packsave))
      {
        printf("写文件packsave.rec出错\n");
        bmqClose();
        fclose(fp);
        return(FAIL);
      }    
    }
    bmqClose();
    /*连接共享内存区*/
    ilRc = _bmqShmConnect();
    if(ilRc)
    {
      printf("连接共享内存区出错:%d\n",ilRc);
      return(FAIL);
    }    
  }	
  fclose(fp);
  printf("保存内存报文信息成功!\n");

  return(SUCCESS);
}
