/* ************************************************************** */
/* �������    �� swPortSet                     */
/* ��������    �� ͨѶ�˿�״̬�޸�ģ��        */
/* ��    ��    :  YZ                              */
/* ��������    �� 2010/1/6                      */
/* ����޸����ڣ�                               */
/* ������;    ��                               */
/* �����б�    :                                  */
/* ************************************************************** */
#include "switch.h"

int main(int argc, char **argv)
{
	int rc = 0;
	int ilStat;
	short ilSt1;
	int ilMb;
	char aTmp[30+1];
	char aConfigFile[128];
	/* ���õ��Գ������� */
	memset(agDebugfile, 0x00, sizeof(agDebugfile));
	sprintf(agDebugfile, "%s.debug", argv[0]);
	
  /* ȡ��DEBUG��־ */
  if((cgDebug = _swDebugflag("swPortSet"))==FAIL)
  {
    fprintf(stderr,"ȡ��DEBUG��־����!");
    exit(FAIL);
  }

	swVdebug(2, "DEBUG�ļ�[%s]", agDebugfile);
	swVdebug(2, "DEBUG��־[%d]", cgDebug);
	
		
	if (argc<3)
	{
    fprintf(stderr,"ERROR:Usage: swPortSet MB STAT\n");		
		swVdebug(0, "ERROR:Usage: swPortSet MB STAT");
		exit(-1);				
	}
	
	ilMb = atoi(argv[1]);
	if (ilMb <= 0)
	{
		swVdebug(0, "ERROR:ilMb=[%d]", ilMb);
		exit(-1);					
	}
	swVdebug(5, "ilMb[%d]", ilMb);	
	
	ilStat = atoi(argv[2]);
		
	swVdebug(5, "ilStat[%d]", ilStat);		
	/* ���ӹ����ڴ� */
	swVdebug(5, "���ӹ����ڴ�......");		
	if(swShmcheck() < 0)
	{
		swVdebug(0, "ERROR:swShmcheck����![%d]", errno);
		exit(-1);		
	}
	swVdebug(5, "���ӹ����ڴ�ɹ�");	
	
	rc=swPortget( ilMb, 1,&ilSt1);
	if(rc< 0)
	{
		swVdebug(0, "ERROR:swPortget����![%d]", errno);
		exit(-1);		
	}
	
	swVdebug(4,"[%d]�˿ڵ�ǰ״̬[%d]",ilMb,ilSt1);

			
	/* �ö˿�״̬ */
	rc = swPortset(ilMb, 1, ilStat);
	if(rc)
	{
		swVdebug(0, "ERROR:�� [%d] �˿�Ϊ[%d]ʧ��", ilMb,ilStat);
	}

	/* ��������Ϊ��������,���ڽ������˿�״̬������Ⱥ�������� 
	rc = PbSndGrpinfo(KZCOMMTRAN);
	swVdebug(0, "PbSndGrpinfo rc [%d]", rc);*/
	swVdebug(2, "�� [%d] �˿�Ϊ[%d]���", ilMb,ilStat);
	return 0;
}	
