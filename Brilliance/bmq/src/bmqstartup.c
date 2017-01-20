/****************************************************************/
/* ģ����    ��BMQSTARTUP                                     */
/* ģ������    :                                                */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���³�Q����                                      */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2001.08.17 ���V2.0,����91��                               */
/* 2006.12.20 ��������ʱ���뱣��ı�����Ϣ�Ĺ���,��229��        */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"
#define  PROC_NUM      10

int    _bmqFileshm();
extern int _bmqLicense(char *);

/*����ȫ�ֱ���*/
char *agPath,agFile[100];
char agBmqfile[2][100]={"/bmqlog/logfile/sendfile.rec","/bmqlog/logfile/packsave.rec"};

int main(int argc,char **argv)
{
  int   i,ilRc,ilStatus;
  int   ilGroupid;
  char  licname[15]="BrillianceQ";
  char  alProname[PROC_NUM][100] = { "S�������ػ�����--bmqcls",  
                                    "������С������������--bmqclean",
                                    "S���鼶�� �����ػ�����--bmqGrp_snd",
                                    "S���鼶�� �����ӽ����ػ�����--bmqGrp_rcv",
                                    "s���鼶�� �����ӽ����ػ�����--bmqGrp_rcvs",
                                    "s���鼶�� �����ӱ��Ĵ洢�ػ�����--bmqGrp_rcvw",
                                    "�ļ��շ������ػ�����--bmqfilecls",
                                    "�ļ��շ������ػ�����--bmqfilemng",
                                    "�ļ��շ�SAF�ػ�����--bmqfilesaf",
                                    "�ļ��շ�LOG�ػ�����--bmqfilelog",
                                  };

  ilRc = _bmqLicense(licname);
  if(ilRc)
  {
    printf("License�����³�Q V2.5�޷�����!\n");
    exit(0);
  } 
  if( fork() == 0 )
  { 
    _bmqShowversion(argc,argv);
    ilRc = _bmqStartup();
    if (ilRc == 0) 
    {
      printf("�³�-Q ��������������");
      exit(0);
    }
    else if(ilRc == 1000) 
    {
      printf("�³�-Q �������Ѿ�����,�����ظ�����!\n");
      exit(1);
    }
    else 
    {
      printf("�³�Q ����������ʧ��\n");
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
    /*ȡ����Ż�������*/
    ilGroupid = atoi(getenv("BMQ_GROUP_ID"));
    if(ilGroupid == 0)
    {
      _bmqDebug("\n��ϵͳ��������[BMQ_GROUP_ID]ʧ��\n");
      return(FAIL);	
    }
    
    /*������������ļ�SHMKEY��GROUP*/
    ilRc = _bmqConfig_load("GROUP",ilGroupid);
    if (ilRc < 0)
    {
      printf("\n���ļ�$BMQ_PATH/etc/bmq.ini��ȡGROUPֵ����,����ʧ��!\n");
    }
    ilRc = _bmqConfig_load("SHMK",0);
    if (ilRc < 0)
    {
      printf("\n���ļ�$BMQ_PATH/etc/bmq.ini��ȡSHMKEYֵ����,����ʧ��!\n");
      exit(-1);
    }
    /*���ӹ����ڴ���*/
    ilRc = _bmqShmConnect();
    if(ilRc)
    {
      printf("\n ���ӹ����ڴ�������:%d,����ʧ��!\n",ilRc);
      exit(-1);
    }
    
    if ( !psgMbshm->lBmqcls_pid )     printf("\n%sδ����ȷ����!\n",alProname[0]);	
    if ( !psgMbshm->lBmqclean_pid )   printf("\n%sδ����ȷ����!\n",alProname[1]);
    if ( !psgMbshm->lBmqgrpsnd_pid )  printf("\n%sδ����ȷ����!\n",alProname[2]);	
    if ( !psgMbshm->lBmqfilecls_pid ) printf("\n%sδ����ȷ����!\n",alProname[6]);
    if ( !psgMbshm->lBmqfilemng_pid ) printf("\n%sδ����ȷ����!\n",alProname[7]);	
    if ( !psgMbshm->lBmqfilesaf_pid ) printf("\n%sδ����ȷ����!\n",alProname[8]);
    if ( !psgMbshm->lBmqfilelog_pid ) printf("\n%sδ����ȷ����!\n",alProname[9]);
    if ( agCommode[0] == 'L' && !psgMbshm->lBmqgrprcv_pid )     printf("\n%sδ����ȷ����!\n",alProname[3]);	
    if ( agCommode[0] == 'S' && !psgMbshm->lBmqgrprcvs_pid )    printf("\n%sδ����ȷ����!\n",alProname[4]);	
    if ( agCommode[0] == 'S' && !psgMbshm->lBmqgrprcvw_pid )    printf("\n%sδ����ȷ����!\n",alProname[5]);	
  }  
  ilRc= _bmqFileshm();/*���ú��������ļ���Ϣ*/
  printf("\n�³�-Q �����������ɹ�!\n");
  return(SUCCESS);
}

/*���뱣����Ϣ*/    
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
    _bmqDebug("\n��ϵͳ��������[BMQ_PATH]ʧ��\n");
    return(FAIL);	
  }
  /*���뱨�ı�����Ϣ*/	
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[1]);

  while( (fp=fopen(agFile,"r"))!=NULL )/*������ļ�����*/
  { 
    /*�ж��ļ��Ƿ�������*/
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
      printf("\n������ʧ��!\n");
      break;	
    }
    /*�����ļ�λ��ָ��*/
    fseek(fp,0L,SEEK_SET);
    
    while(!feof(fp)) 
    { 
      ilRc = fread((char *)&slPacksv,sizeof(char),sizeof(struct packsave),fp);
      if (ilRc &&(ilRc != sizeof(struct packsave)))
      {
        printf("\n���ļ�packsave.rec����!\n");
        break;
      }
      if(ilRc == 0) continue;   
      ilRc = bmqPutSavePack(slPacksv.iOrgGrpid,slPacksv.iOrgMbid,slPacksv.iMbid,slPacksv.iPrior,slPacksv.aFilter,slPacksv.aMsgbuf,slPacksv.iMsglen);
      if(ilRc) break;    
    }
    printf("\n�ļ�packsave.rec����ɹ�!\n");
    bmqClose(); 
    fclose(fp);
    unlink(agFile);
    
  }
  
  /*�����ļ����䱣����Ϣ*/
  
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[0]);
  
  while((fp = fopen(agFile,"r")) != NULL)/*������ڸ��ļ�*/
  {  
    fseek(fp,0L,SEEK_END);
    llLen = ftell(fp);
    if(llLen == 0) 
    { 
    	fclose(fp);
    	unlink(agFile);
    	break;      	
    }
    /*���ӹ����ڴ���*/
    ilRc = _bmqShmConnect();
    if(ilRc)
    {
      printf("%s :���ӹ����ڴ�������:%d\n",__FILE__,ilRc);
      return(FAIL);
    }
    /*�����ļ�λ��ָ��*/
    fseek(fp,0L,SEEK_SET); 
      
    while( !feof(fp) )
    {        
      ilRc = fread((char *)&slMonmsg,sizeof(char),sizeof(struct monmsg),fp);
      if (ilRc && (ilRc != sizeof(struct monmsg)))
      {
    	  printf("\n���ļ�sendfile.rec����!\n");
    	  break;
      }
      if (ilRc == 0) continue;
      memcpy(&psgMbfilemng[i],&slMonmsg,sizeof(struct monmsg));
      ++i;  
      if(i >= psgMbshm->iFilecount)
      { 
    	  printf("\n�����ڴ����ͬʱ�����ļ���!\n");
    	  break;
      }    	
    }
          
    printf("\n�ļ�sendfile.rec����ɹ�!\n");
    fclose(fp);
    unlink(agFile);       
  }
        
  return(SUCCESS); 
}
