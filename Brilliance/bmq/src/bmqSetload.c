/****************************************************************/
/* ģ����    ��BMQSETLOAD                                     */
/* ģ������    ����������ģ��                                   */
/* �� �� ��    ��V2.0                                           */
/* ��    ��    ��xujun                                          */
/* ��������    ��2001/08/08                                     */
/* ����޸����ڣ�                                               */
/* ģ����;    ��				                */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/* 2001.08.17 ���V2.0,����194��                              */
/****************************************************************/

/*�⺯������*/
#include "bmq.h"

int main(int argc,char **argv)
{
  int   ilRc,i;
  char  alBuf[10];
  char  alFp_name[80];
  char  alPro_name[20];
  int   ilGroupid;

  printf("-------------------------------------------------------------------------------\n");
  printf("***�޸Ĳ�����ע����������:***\n");
  printf("-------------------------------------------------------------------------------\n");
  printf("* BMQ----�����ڴ�������޸���Ҫ���������³�Q!\n");
  printf("* SHMK---�����ڴ�ID�������³�Q����ʱ�����޸�!\n");
  printf("* GROUP--����ʱ�³�Q�������޸ĺ���Ҫ��������");
  printf("  bmqGrp_rcv��bmqGrp_snd������Ч!\n");
  printf("* WTIME--ϵͳ��ѯʱ����޸���Ҫ��������bmqclean������Ч!\n");
  printf("* MAIL---�������õ��޸Ľ������б����̺���Ч!\n");
  printf("* FILE---�ļ��������õ��޸Ľ�����������FILE����Ч!\n");
  printf("-------------------------------------------------------------------------------\n");
  printf("\n[1] ���� �ļ�����4������");
  printf("\n[2] ���� �����ػ�����");
  printf("\n[3] ���� bmqclean");
  printf("\n[4] ��Ҫˢ����������");
  printf("\n������ѡ��[1~4]:");

  scanf("%s",alBuf);
  i  = atoi(alBuf);
  /*���ӹ����ڴ�*/
  ilRc = _bmqShmConnect();
  if(ilRc)
  {
    printf("���ӹ����ڴ�������:%d\n",ilRc);
    exit(FAIL);
  }
  switch(i)
  {
    case 1:
           printf("\nȷ�������ļ�����4���ػ����̷�[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("û�������ļ�����4���ػ�����!\n");
             exit(SUCCESS);
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
           printf("\n�ļ�����4���ػ�����������!\n");
           break;
    case 2:
           /*�ж������������*/
           /*ȡ����Ż�������*/
           ilGroupid = atoi(getenv("BMQ_GROUP_ID"));
           if(ilGroupid == 0)
           {
             _bmqDebug("\n��ϵͳ��������[BMQ_GROUP_ID]ʧ��\n");
             return(FAIL);	
           }
    
           /*������������ļ�GROUP*/
           ilRc = _bmqConfig_load("GROUP",ilGroupid);
           if (ilRc < 0)
           {
             printf("\n���ļ�$BMQ_PATH/etc/bmq.ini��ȡGROUPֵ����,����ʧ��!\n");
           }
           printf("\nȷ����������ػ����̷�[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("û����������ػ�����!\n");
             exit(SUCCESS);
           }
           else
           { 
             if( psgMbshm->lBmqgrpsnd_pid > 0 )
                 kill(psgMbshm->lBmqgrpsnd_pid,SIGTERM);
             if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_snd",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_snd");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);
               }
               
             if(agCommode[0] == 'L')
             {
               if( psgMbshm->lBmqgrprcv_pid > 0 )
                 kill(psgMbshm->lBmqgrprcv_pid,SIGTERM);               
               if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_rcv",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_rcv");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);
               }
               printf("\nbmqGrp_rcv��bmqGrp_snd������!\n");
             }
             if(agCommode[0] == 'S')
             {
               if( psgMbshm->lBmqgrprcvs_pid > 0 )
                  kill(psgMbshm->lBmqgrprcvs_pid,SIGTERM);
               if( psgMbshm->lBmqgrprcvw_pid > 0 )
                  kill(psgMbshm->lBmqgrprcvw_pid,SIGTERM);  
               if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_rcvs",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_rcvs");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);
               }  	
               if( fork() == 0)
               {
                 sprintf(alFp_name,"%s/bin/bmqGrp_rcvw",getenv("BMQ_PATH"));
                 sprintf(alPro_name,"%s","bmqGrp_rcvw");
                 ilRc = execl(alFp_name,alPro_name,(char *)0);
                 exit(0);                 
               }
               printf("\nbmqGrp_rcvs��bmqGrp_rcvw��bmqGrp_snd������!\n");	            	
             }             
           }           
           break;
    case 3:
           printf("\nȷ������bmqclean��[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("û������bmqclean!\n");
             exit(SUCCESS);
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
               exit(0);
             }
           }
           printf("\nbmqclean������!\n");
           break;
    case 4:
           printf("\nȷ��ˢ��[mail]���÷�[y/n]");
           scanf("%s",alBuf);
           _bmqTrim(alBuf);
           if( alBuf[0] != 'Y' && alBuf[0] != 'y' )
           {
             printf("û��ˢ��[mail]����!\n");
             exit(SUCCESS);
           }
           else
           {
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

             printf("\n[MAIL]����������ˢ��!\n");
           }
           break;
    default:printf("\nѡ�񲻺Ϸ�!\n");
            break;
  }
  exit(SUCCESS);
}
