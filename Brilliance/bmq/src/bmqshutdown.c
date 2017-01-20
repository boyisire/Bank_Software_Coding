/****************************************************************/
/* ģ����    ��BMQSHUTDOWN                                    */
/* ģ������    ��                                               */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    �����                                           */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ���³�Q�ر�                                      */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2001.08.17 ���V2.0,����28��                               */
/* 2006.12.20 ���ӹر�ʱ�������䱨�ĵĹ���,��192��              */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"

/*��������*/
int _bmqShmsave();

/*����ȫ�ֱ���*/
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
      if (ilRc == 0) printf("�³�-Q �ɹ��ر�!\n");
      else printf("--%d--�³�-Q �رճ���!\n",ilRc);      
      exit(0);
    }
  }  
  
  printf("�ر��³�-Qʱ�Ƿ�Ҫ�����ڴ��е�����?[Y/N]\n");  
  ch = getchar();
  switch(ch)
  {
    case 'Y':
    case 'y':
      ilRc=_bmqShmsave();/*���ú��������ڴ��е����ݵ��ļ���*/
      if(ilRc)/*��������г���*/
      {
         if ((agPath = getenv("BMQ_PATH")) == NULL)
         {
           _bmqDebug("��ϵͳ��������[BMQ_PATH]ʧ��\n");
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
      if (ilRc == 0) printf("�³�-Q �ɹ��ر�!\n");
      else printf("--%d--�³�-Q �رճ���!\n",ilRc);
      break;
      
    default:
      ilRc = _bmqShutdown();
      if (ilRc == 0) printf("�³�-Q �ɹ��ر�!\n");
      else printf("--%d--�³�-Q �رճ���!\n",ilRc);      
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
    _bmqDebug("��ϵͳ��������[BMQ_PATH]ʧ��\n");
    return(FAIL);
  }
  
  /*���ӹ����ڴ���*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    printf("%s :���ӹ����ڴ�������:%d\n",__FILE__,ilRc);
    exit(FAIL);
  }
  
  /*�����ļ�����*/
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[0]);  
  fp=fopen(agFile,"w+");
  if(fp == NULL)
  { 
    printf("���ļ�sendfile.rec���ɹ�!\n");
    return(FAIL);
  }  
  for ( i =0 ; i < psgMbshm->iFilecount; i++ )
  { 
    if (psgMbfilemng[i].iFlag == 0)/*�ҵ�δʹ�ü�¼��*/ continue;
    psgMbfilemng[i].iFlag = 1;
    ilRc = fwrite((char *)&psgMbfilemng[i],sizeof(char),sizeof(struct monmsg),fp);
    if(ilRc != sizeof(struct monmsg))
    { 
      printf("д�ļ�sendfile.rec����!\n");
      fclose(fp);
      return(FAIL);
    }    
  }
  fclose(fp);
  printf("�����ڴ��ļ��������ݳɹ�!\n");
  

  /*���汨����Ϣ*/
  memset(agFile,0x00,sizeof(agFile));
  strcpy(agFile,agPath);
  strcat(agFile,agBmqfile[1]);
  fp=fopen(agFile,"w+");
  if(fp == NULL)
  {
    printf("���ļ�packsave.rec���ɹ�!\n");
    return(FAIL);  	
  }  

  for ( i = 0; i < psgMbshm->iMbinfo; i++)
  { 
    if (psgMbinfo[i].iFlag == 0)/*����δ����*/ continue;
    if (psgMbinfo[i].lPendnum == 0) continue;/*���䵱ǰ��Ϣ��Ϊ0*/
      
    ilRc = bmqOpen(i+1);
    if (ilRc)
    {
      printf("���������\n");
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
      
      /*��ֵ���¶���Ľṹ;*/
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
        printf("д�ļ�packsave.rec����\n");
        bmqClose();
        fclose(fp);
        return(FAIL);
      }    
    }
    bmqClose();
    /*���ӹ����ڴ���*/
    ilRc = _bmqShmConnect();
    if(ilRc)
    {
      printf("���ӹ����ڴ�������:%d\n",ilRc);
      return(FAIL);
    }    
  }	
  fclose(fp);
  printf("�����ڴ汨����Ϣ�ɹ�!\n");

  return(SUCCESS);
}
