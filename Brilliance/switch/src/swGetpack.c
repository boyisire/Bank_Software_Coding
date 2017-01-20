#include <stdio.h>
#include "swapi.h"

int main(int argc,char **argv)
{
  FILE *fp;
  char alMsgreturn[iMSGMAXLEN];
  char alMsgfile[100];
  unsigned int ilMsgreturn;
  short ilPriority,ilClass,ilType,ilQid;
  short ilQ_org,ilRc;

  if(argc !=3) {
    fprintf(stderr, "ʹ�÷���: swGetpack Դ���� �����ļ�\n");
    exit(-1);
  }
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  strcpy(agDebugfile,"swGetpack.debug");

  ilQ_org = atoi(argv[1]);
  strcpy(alMsgfile,argv[2]);
  swMbopen(ilQ_org);

  ilMsgreturn = iMSGMAXLEN;
  ilPriority = 0;
  ilClass = 0;
  ilType = 0;
  swDebug("before strlen is:%d\n",strlen(alMsgreturn));
  ilRc = swRecvpackw(&ilQid,alMsgreturn,&ilMsgreturn,
    &ilPriority,&ilClass,&ilType);
  if (ilRc)
  {
    fprintf(stderr,"���ձ���ʱ����: %d",ilRc);
    exit(-1);
  }
  swDebug("after strlen is:%d\n",strlen(alMsgreturn));

  int n = strlen(alMsgreturn);
  printf("read len is:%d\n",n);
#ifdef DEBUG
    swDebug("��[%d]�����յ��ı���:",ilQid);
    swDebughex(alMsgreturn,ilMsgreturn);
#endif

  unlink(alMsgfile);
  fp=fopen(alMsgfile,"a+");

  printf("alMsgfile = %s\n",alMsgfile);
  if(fp==NULL)
  {
    fprintf(stderr,"�����ļ�[%s]�򿪳���!\n",alMsgfile);
    exit(-1);
  }


  ilRc = fwrite(alMsgreturn,1,ilMsgreturn,fp);
  fclose(fp);
    
  /* �ر�ǰ�ý��������˿� */
  swMbclose();
  exit(0);
}

