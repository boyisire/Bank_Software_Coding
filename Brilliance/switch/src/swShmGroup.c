#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/errno.h>
#include "switch.h"
#include "swShmGroup.h"




/*得到通讯线路配置信息到全局结构*/



	

/*根据静态配置信息,形成可直接导入共享内存的结构*/

/**************************************************************
 ** 函数名: main()
 ** 功  能: 大机与交换平台接口平台初始化
 ** 作  者: 乔志国
 ** 建立日期: 2001.11.27
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义:
 ** 返回值:
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
		swVdebug(0,"取跨组键值出错 GrpKeycheck 返回=[%d]",ilRc);
		exit (-1);
	}
	ilRc=swGrpShmCreat();
	if (ilRc<0)
	{
		swVdebug(0,"跨组共享内存建立失败 GrpShmCreat 返回=[%d]",ilRc);
		exit (-1);
	}
	if (ilRc == 0)
	{
		ilRc=swGrpMbInit();
		if (ilRc<0)
		{
			swVdebug(0,"跨组共享内存初始化 swGrpMbInit 返回=[%d]",ilRc);
			exit (-1);
		}
	}	
		
	
	ilRc=swGrpInfoInit();
	if (ilRc<0)
	{
		swVdebug(0,"跨组共享内存初始化 GrpInfoInit 返回=[%d]",ilRc);
		exit (-1);
	}
	
	ilRc=swGrpSetHostMbList();
	if (ilRc<0)
	{
		swVdebug(0,"跨组共享内存初始化 swGrpSetHostMbList 返回=[%d]",ilRc);
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










