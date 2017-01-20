/* ************************************************************** */
/* 函数编号    ： swPortSet                     */
/* 函数名称    ： 通讯端口状态修改模块        */
/* 作    者    :  YZ                              */
/* 建立日期    ： 2010/1/6                      */
/* 最后修改日期：                               */
/* 函数用途    ：                               */
/* 函数列表    :                                  */
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
	/* 设置调试程序名称 */
	memset(agDebugfile, 0x00, sizeof(agDebugfile));
	sprintf(agDebugfile, "%s.debug", argv[0]);
	
  /* 取得DEBUG标志 */
  if((cgDebug = _swDebugflag("swPortSet"))==FAIL)
  {
    fprintf(stderr,"取得DEBUG标志出错!");
    exit(FAIL);
  }

	swVdebug(2, "DEBUG文件[%s]", agDebugfile);
	swVdebug(2, "DEBUG标志[%d]", cgDebug);
	
		
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
	/* 连接共享内存 */
	swVdebug(5, "连接共享内存......");		
	if(swShmcheck() < 0)
	{
		swVdebug(0, "ERROR:swShmcheck出错![%d]", errno);
		exit(-1);		
	}
	swVdebug(5, "连接共享内存成功");	
	
	rc=swPortget( ilMb, 1,&ilSt1);
	if(rc< 0)
	{
		swVdebug(0, "ERROR:swPortget出错![%d]", errno);
		exit(-1);		
	}
	
	swVdebug(4,"[%d]端口当前状态[%d]",ilMb,ilSt1);

			
	/* 置端口状态 */
	rc = swPortset(ilMb, 1, ilStat);
	if(rc)
	{
		swVdebug(0, "ERROR:置 [%d] 端口为[%d]失败", ilMb,ilStat);
	}

	/* 以下两句为跨组新增,用于将本机端口状态发给机群其他机器 
	rc = PbSndGrpinfo(KZCOMMTRAN);
	swVdebug(0, "PbSndGrpinfo rc [%d]", rc);*/
	swVdebug(2, "置 [%d] 端口为[%d]完毕", ilMb,ilStat);
	return 0;
}	
