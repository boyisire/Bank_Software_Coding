/****************************************************************/
/* 模块编号    ：BMQSTARTUP                                     */
/* 模块名称    :                                                */
/* 版 本 号    ：V2.0                                           */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2001/08/08                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：新晨Q启动                                      */
/****************************************************************/

/****************************************************************/
/* 修改记录：                                                   */
/* 2001.08.17 封版V2.0,程序共91行                               */
/* 2006.12.20 增加启动时导入保存的报文信息的功能,共229行        */
/****************************************************************/

/*库函数定义*/
#include "bmq.h"
#define  PROC_NUM      10

int    _bmqFileshm();
extern int _bmqLicense(char *);

/*定义全局变量*/
char *agPath,agFile[100];
char agBmqfile[2][100]={"/bmqlog/logfile/sendfile.rec","/bmqlog/logfile/packsave.rec"};

int main(int argc,char **argv)
{
  int   i,ilRc,ilStatus;
  int   ilGroupid;
  char  licname[15]="BrillianceQ";
  char  alProname[PROC_NUM][100] = { "S端侦听守护进程--bmqcls",  
                                    "邮箱队列、连接清理进程--bmqclean",
                                    "S跨组级联 发送守护进程--bmqGrp_snd",
                                    "S跨组级联 长链接接收守护进程--bmqGrp_rcv",
                                    "s跨组级联 短链接接收守护进程--bmqGrp_rcvs",
                                    "s跨组级联 短链接报文存储守护进程--bmqGrp_rcvw",
                                    "文件收发传输守护进程--bmqfilecls",
                                    "文件收发管理守护进程--bmqfilemng",
                                    "文件收发SAF守护进程--bmqfilesaf",
                                    "文件收发LOG守护进程--bmqfilelog",
                                  };

  ilRc = _bmqLicense(licname);
  if(ilRc)
  {
    printf("License错误，新晨Q V2.5无法启动!\n");
    exit(0);
  } 
  if( fork() == 0 )
  { 
    _bmqShowversion(argc,argv);
    ilRc = _bmqStartup();
    if (ilRc == 0) 
    {
      printf("新晨-Q 正在启动服务器");
      exit(0);
    }
    else if(ilRc == 1000) 
    {
      printf("新晨-Q 服务器已经启动,不能重复启动!\n");
      exit(1);
    }
    else 
    {
      printf("新晨Q 服务器启动失败\n");
      exit(-1);
    }
  }

  while( wait(&ilStatus) != -1)
  { 
    if( ilStatus ) exit(ilStatus);
    for(i=0;i<5;i++)
    {
      fflush(stdout);
      printf(".");
      sleep(1);
    }
    /*取得组号环境变量*/
    ilGroupid = atoi(getenv("BMQ_GROUP_ID"));
    if(ilGroupid == 0)
    {
      _bmqDebug("\n读系统环境变量[BMQ_GROUP_ID]失败\n");
      return(FAIL);	
    }
    
    /*读入参数配置文件SHMKEY和GROUP*/
    ilRc = _bmqConfig_load("GROUP",ilGroupid);
    if (ilRc < 0)
    {
      printf("\n从文件$BMQ_PATH/etc/bmq.ini中取GROUP值错误,启动失败!\n");
    }
    ilRc = _bmqConfig_load("SHMK",0);
    if (ilRc < 0)
    {
      printf("\n从文件$BMQ_PATH/etc/bmq.ini中取SHMKEY值错误,启动失败!\n");
      exit(-1);
    }
    /*连接共享内存区*/
    ilRc = _bmqShmConnect();
    if(ilRc)
    {
      printf("\n 连接共享内存区出错:%d,启动失败!\n",ilRc);
      exit(-1);
    }
    
    if ( !psgMbshm->lBmqcls_pid )     printf("\n%s未能正确启动!\n",alProname[0]);	
    if ( !psgMbshm->lBmqclean_pid )   printf("\n%s未能正确启动!\n",alProname[1]);
    if ( !psgMbshm->lBmqgrpsnd_pid )  printf("\n%s未能正确启动!\n",alProname[2]);	
    if ( !psgMbshm->lBmqfilecls_pid ) printf("\n%s未能正确启动!\n",alProname[6]);
    if ( !psgMbshm->lBmqfilemng_pid ) printf("\n%s未能正确启动!\n",alProname[7]);	
    if ( !psgMbshm->lBmqfilesaf_pid ) printf("\n%s未能正确启动!\n",alProname[8]);
    if ( !psgMbshm->lBmqfilelog_pid ) printf("\n%s未能正确启动!\n",alProname[9]);
    if ( agCommode[0] == 'L' && !psgMbshm->lBmqgrprcv_pid )     printf("\n%s未能正确启动!\n",alProname[3]);	
    if ( agCommode[0] == 'S' && !psgMbshm->lBmqgrprcvs_pid )    printf("\n%s未能正确启动!\n",alProname[4]);	
    if ( agCommode[0] == 'S' && !psgMbshm->lBmqgrprcvw_pid )    printf("\n%s未能正确启动!\n",alProname[5]);	
  }  
  ilRc= _bmqFileshm();/*调用函数导入文件信息*/
  printf("\n新晨-Q 服务器启动成功!\n");
  return(SUCCESS);
}

/*导入保存信息*/    
int _bmqFileshm()
{
  int   ilRc;	
  int   i=0;
  long  llLen;
  FILE  *fp;		
  struct packsave slPacksv;	
  struct monmsg slMonmsg;
  
  agPath = getenv("BMQ_PATH");
  if (agPath == NULL)
  {
    _bmqDebug("\n读系统环境变量[BMQ_PATH]失败\n");
    return(FAIL);	
  }
  /*导入报文保存信息*/	
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[1]);

  while( (fp=fopen(agFile,"r"))!=NULL )/*如果该文件存在*/
  { 
    /*判断文件是否有内容*/
    fseek(fp,0L,SEEK_END);
    llLen = ftell(fp);
    if (llLen == 0)
    { 
    	fclose(fp);
    	unlink(agFile);
    	break;
    }    
    ilRc = bmqOpen(1);
    if (ilRc)
    {
      printf("\n打开邮箱失败!\n");
      break;	
    }
    /*重置文件位置指针*/
    fseek(fp,0L,SEEK_SET);
    
    while(!feof(fp)) 
    { 
      ilRc = fread((char *)&slPacksv,sizeof(char),sizeof(struct packsave),fp);
      if (ilRc &&(ilRc != sizeof(struct packsave)))
      {
        printf("\n读文件packsave.rec出错!\n");
        break;
      }
      if(ilRc == 0) continue;   
      ilRc = bmqPutSavePack(slPacksv.iOrgGrpid,slPacksv.iOrgMbid,slPacksv.iMbid,slPacksv.iPrior,slPacksv.aFilter,slPacksv.aMsgbuf,slPacksv.iMsglen);
      if(ilRc) break;    
    }
    printf("\n文件packsave.rec导入成功!\n");
    bmqClose(); 
    fclose(fp);
    unlink(agFile);
    
  }
  
  /*导入文件传输保存信息*/
  
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[0]);
  
  while((fp = fopen(agFile,"r")) != NULL)/*如果存在该文件*/
  {  
    fseek(fp,0L,SEEK_END);
    llLen = ftell(fp);
    if(llLen == 0) 
    { 
    	fclose(fp);
    	unlink(agFile);
    	break;      	
    }
    /*连接共享内存区*/
    ilRc = _bmqShmConnect();
    if(ilRc)
    {
      printf("%s :连接共享内存区出错:%d\n",__FILE__,ilRc);
      return(FAIL);
    }
    /*重置文件位置指针*/
    fseek(fp,0L,SEEK_SET); 
      
    while( !feof(fp) )
    {        
      ilRc = fread((char *)&slMonmsg,sizeof(char),sizeof(struct monmsg),fp);
      if (ilRc && (ilRc != sizeof(struct monmsg)))
      {
    	  printf("\n读文件sendfile.rec出错!\n");
    	  break;
      }
      if (ilRc == 0) continue;
      memcpy(&psgMbfilemng[i],&slMonmsg,sizeof(struct monmsg));
      ++i;  
      if(i >= psgMbshm->iFilecount)
      { 
    	  printf("\n超出内存最大同时发送文件数!\n");
    	  break;
      }    	
    }
          
    printf("\n文件sendfile.rec导入成功!\n");
    fclose(fp);
    unlink(agFile);       
  }
        
  return(SUCCESS); 
}
