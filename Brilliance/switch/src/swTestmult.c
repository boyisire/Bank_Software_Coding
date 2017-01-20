#include <stdio.h>
#include "swapi.h"

int main(int argc,char **argv)
{
  FILE *fp;
  char alMsgpack[iMSGMAXLEN];
  char alMsgreturn[iMSGMAXLEN];
  char alMsgfile[100];
  short ilQ_org,ilQ_dest;
  unsigned int ilMsglen,ilMsgreturn;
  short ilClass,ilType;
  short i,ilRc,ilPriority;
  short ilQid,ilSendnum;
  long lltime1,lltime2;

  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swTestmult.debug");
  if(argc !=5) {
    fprintf(stderr, "使用方法: swTestsign 报文文件 发送次数 源邮箱 目标邮箱\n");
    exit(-1);
  }

  strcpy(alMsgfile,argv[1]);
  ilSendnum = atoi(argv[2]);
  ilQ_org = atoi(argv[3]);
  ilQ_dest = atoi(argv[4]);

  fp=fopen(alMsgfile,"r");

  printf("alMsgfile = %s\n",alMsgfile);
  if(fp==NULL)
  {
    fprintf(stderr,"报文文件[%s]打开出错!\n",alMsgfile);
    exit(-1);
  }
  fseek(fp,0,SEEK_END);
  ilMsglen = ftell(fp);
  printf("ilMsglen = %d\n",ilMsglen);
  fseek(fp,0,SEEK_SET);
  ilMsglen = fread(alMsgpack,sizeof(char),ilMsglen,fp);
  printf("ilMsglen = %d\n",ilMsglen);
  fclose(fp);
  
  ilRc = swMbopen(ilQ_org);
  if (ilRc)
  {
    fprintf(stderr,"打开邮箱[%d]出错!\n",ilQ_org);
    exit(-1);
  }
  swDebug("准备进行发送的报文:");
  swDebughex(alMsgpack,ilMsglen);
  time(&lltime1);
  for(i=1;i<=ilSendnum;i++)
  {
    ilPriority = 1;
    ilClass = 0;
    ilType = 0;
    ilRc = swSendpack(ilQ_dest,alMsgpack,ilMsglen,
      ilPriority,ilClass,ilType);
    if (ilRc)
    {
      fprintf(stderr,"报文发送时出错: %d",ilRc);
      exit(-1);
    }
  }
  for(i=1;i<=ilSendnum;i++)
  {
    ilMsgreturn = iMSGMAXLEN;
    ilPriority = 0;
    ilClass = 0;
    ilType = 0;
    ilRc = swRecvpackw(&ilQid,alMsgreturn,&ilMsgreturn,
      &ilPriority,&ilClass,&ilType);
    if (ilRc)
    {
      fprintf(stderr,"接收报文时出错: %d",ilRc);
      exit(-1);
    }
#ifdef DEBUG
    swDebug("从[%d]邮箱收到的报文:",ilQid);
    swDebughex(alMsgreturn,ilMsgreturn);
#endif
  }
    
  time(&lltime2);
 
  printf("使用时间 = endtime - begintime = [%ld]\n",(lltime2-lltime1));
  swDebug("使用时间 = endtime - begintime = [%ld]",(lltime2-lltime1));
  
  /* 关闭前置进程所属端口 */
  swMbclose();
  exit(0);
}

