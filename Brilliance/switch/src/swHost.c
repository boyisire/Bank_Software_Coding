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
    fprintf(stderr, "ʹ�÷���: swHost Դ����\n");
    exit(-1);
  }

  ilQ_org = atoi(argv[1]);
  
  ilRc = swMbopen(ilQ_org);
  if (ilRc)
  {
    fprintf(stderr,"������[%d]����!\n",ilQ_org);
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
      fprintf(stderr,"���ձ���ʱ����: %d",ilRc);
      exit(-1);
    }
    /*
    printf("���յ�������:[%d]\n[%s]\n",ilMsgreturn-sizeof(struct msghead),slMsgpack.aMsgbody);
    swVdebug(0,"���յ�������:[%d]",ilMsgreturn-sizeof(struct msghead));
    swDebughex(slMsgpack.aMsgbody,ilMsgreturn-sizeof(struct msghead));
    */
    ilPriority = 0;
    llClass = 0;
    llType = 0;
    ilRc = swSendpackl(ilQid,(char *)&slMsgpack,ilMsgreturn,
      ilPriority,llClass,llType);
    if (ilRc)
    {
      fprintf(stderr,"���ķ���ʱ����: %d",ilRc);
      exit(-1);
    }
    /*printf("������Ӧ���ͳɹ�\n");*/
  }
  
  /* �ر�ǰ�ý��������˿� */
  swMbclose();
  exit(0);
}
