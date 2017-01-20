#include <stdio.h>
#include "swapi.h"
#undef DEBUG

int main(int argc,char **argv)
{
  FILE *fp;
  char alMsgpack[iMSGMAXLEN];
  char alMsgreturn[iMSGMAXLEN];
  char alMsgfile[100];
  short ilQ_org,ilQ_dest;
  unsigned int ilMsglen,ilMsgreturn;
  long  llClass,llType;
  short i,ilRc,ilPriority;
  short ilQid,ilSendnum;
  long lltime1,lltime2;
  struct msgpack slMsgpack;

  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swHost.debug");
  if(argc !=2) {
    fprintf(stderr, "使用方法: swHost 源邮箱\n");
    exit(-1);
  }

  ilQ_org = atoi(argv[1]);
  
  ilRc = swMbopen(ilQ_org);
  if (ilRc)
  {
    fprintf(stderr,"打开邮箱[%d]出错!\n",ilQ_org);
    exit(-1);
  }
  for(;;)
  {
    ilMsgreturn = iMSGMAXLEN;
    ilPriority = 0;
    llClass = 0;
    llType = 0;
    ilRc = swRecvpacklw(&ilQid,(char *)&slMsgpack,&ilMsgreturn,
      &ilPriority,&llClass,&llType);  
    if (ilRc)
    {
      fprintf(stderr,"接收报文时出错: %d",ilRc);
      exit(-1);
    }
    /*
    printf("接收到请求报文:[%d]\n[%s]\n",ilMsgreturn-sizeof(struct msghead),slMsgpack.aMsgbody);
    swVdebug(0,"接收到请求报文:[%d]",ilMsgreturn-sizeof(struct msghead));
    swDebughex(slMsgpack.aMsgbody,ilMsgreturn-sizeof(struct msghead));
    */
    ilPriority = 0;
    llClass = 0;
    llType = 0;
    ilRc = swSendpackl(ilQid,(char *)&slMsgpack,ilMsgreturn,
      ilPriority,llClass,llType);
    if (ilRc)
    {
      fprintf(stderr,"报文发送时出错: %d",ilRc);
      exit(-1);
    }
    /*printf("报文响应发送成功\n");*/
  }
  
  /* 关闭前置进程所属端口 */
  swMbclose();
  exit(0);
}
