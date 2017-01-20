#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include "switch.h"
#include "swShmGroup.h"




/*�õ�ͨѶ��·������Ϣ��ȫ�ֽṹ*/



	

/*���ݾ�̬������Ϣ,�γɿ�ֱ�ӵ��빲���ڴ�Ľṹ*/

/**************************************************************
 ** ������: main()
 ** ��  ��: ����뽻��ƽ̨�ӿ�ƽ̨��ʼ��
 ** ��  ��: ��־��
 ** ��������: 2001.11.27
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:
***************************************************************/
int main(int argc,char **argv)
{
	
	int ilRc;
	int	 i,j,iGrpID=1;
	PB_HOST_MBINFO psHostInfo;
	memset(&psHostInfo,0x00,sizeof(PB_HOST_MBINFO));
	
	cgDebug=5;
	sprintf(agDebugfile,"%s%s",argv[0],".debug");
	
	ilRc=swGrpKeycheck();
	if (ilRc<0)
	{
		swVdebug(0,"ȡ�����ֵ���� GrpKeycheck ����=[%d]",ilRc);
		exit (-1);
	}
	ilRc=swGrpShmCreat();
	if (ilRc<0)
	{
		swVdebug(0,"���鹲���ڴ潨��ʧ�� GrpShmCreat ����=[%d]",ilRc);
		exit (-1);
	}
	if (ilRc == 0)
	{
		ilRc=swGrpMbInit();
		if (ilRc<0)
		{
			swVdebug(0,"���鹲���ڴ��ʼ�� swGrpMbInit ����=[%d]",ilRc);
			exit (-1);
		}
	}	
		
	
	ilRc=swGrpInfoInit();
	if (ilRc<0)
	{
		swVdebug(0,"���鹲���ڴ��ʼ�� GrpInfoInit ����=[%d]",ilRc);
		exit (-1);
	}
	
	ilRc=swGrpSetHostMbList();
	if (ilRc<0)
	{
		swVdebug(0,"���鹲���ڴ��ʼ�� swGrpSetHostMbList ����=[%d]",ilRc);
		exit (-1);
	}
	
	
	for(i=0;i<psgGrpShm_pid->iGrpCount;i++)
	{
		swVdebug(0,"i=[%d] Grp=[%d] IP=[%s] PORT=[%d]",i,psgGrpShm_pid->sGrpInfo[i].iGrpID,psgGrpShm_pid->sGrpInfo[i].aGrpIP,psgGrpShm_pid->sGrpInfo[i].iGrpPort);
		swVdebug(0,"i=[%d] Grp=[%d] iBegSeqNo=[%ld] iEndSeqNo=[%ld]",i,psgGrpShm_pid->sGrpInfo[i].iGrpID,psgGrpShm_pid->sGrpInfo[i].iBegSeqNo,psgGrpShm_pid->sGrpInfo[i].iEndSeqNo);
		swVdebug(0,"i=[%d] Grp=[%d] iUseFlag=[%d]",i,psgGrpShm_pid->sGrpInfo[i].iGrpID,psgGrpShm_pid->sGrpInfo[i].iUseFlag);
		for(j=0;j<MAXMBCOUNT;j++)
		{
			swVdebug(4,"j=[%d] Grp=[%d] MB=[%d] STATUS=[%d]",j,psgGrpShm_pid->sGrpMbInfo[i].iGrp,psgGrpShm_pid->sGrpMbInfo[i].sMbInfo[j].iMbID,psgGrpShm_pid->sGrpMbInfo[i].sMbInfo[j].iMbStaus);
		}
	}
	ilRc=swGrpGetHostMbInfo(iGrpID,&psHostInfo);
	exit(0);
}










