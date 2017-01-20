#include "swapi.h"

void appQuit(int iFlag);

int main(int argc,char **argv)
{
  FILE *fp;
  struct msgpack slMsgpack;
  short ilQ_org,ilQ_des,ilRc;
  unsigned int ilPrior,ilType,ilClass,ilMsglen;
  char  alFmlfile[100];

  strcpy(agDebugfile,"swTestterm.debug");
  if(argc != 4) {
    printf("ʹ�÷���: swTestterm ͨѶ���� ǰ������ �����ļ�\n");
    printf("(swTestterm ����ǰ��������Ϣ�ñ���ͷ,����������ˮ��)\n"); 
    exit(-1);
  }

  ilQ_org = atoi(argv[1]);
  ilQ_des = atoi(argv[2]);
  strcpy(alFmlfile,argv[3]);

  ilRc = swMbopen(ilQ_org);
  if (ilRc)
  {
    swDebug("����򿪳���(%d)!",ilRc);
    exit(0);
  }

  swDebug("����򿪳ɹ�!");

  ilRc = swInit(ilQ_des);
  if (ilRc)
  {
    swDebug("�˿ڳ�ʼ������(%d)!",ilRc);
    appQuit(-1);
  }
  swDebug("�˿ڳ�ʼ���ɹ�!");

  memset((char *)&slMsgpack.sMsghead,0x00,sizeof(struct msghead));
  slMsgpack.sMsghead.cRouter = '1';
  slMsgpack.sMsghead.cFormatter = '0';
  slMsgpack.sMsghead.iMsgtype = 1;
  slMsgpack.sMsghead.iTranstep = 1;
  slMsgpack.sMsghead.iMsgformat = sgPortattrib.iMsgformat;
  slMsgpack.sMsghead.iOrg_q = sgPortattrib.iQid;
  slMsgpack.sMsghead.iDes_q = sgPortattrib.iQid;
  slMsgpack.sMsghead.iBegin_q_id = sgPortattrib.iQid;
  time(&(slMsgpack.sMsghead.lBegintime));
  
  if ((fp = fopen(alFmlfile,"r")) == NULL)
  {
    swDebug("FML�ļ�[%s]�򿪳���!",alFmlfile);
    appQuit(-1);
  }
  swDebug("FML�ļ�[%s]�򿪳ɹ�!",alFmlfile);

  fseek(fp,0,SEEK_END);
  slMsgpack.sMsghead.iBodylen = ftell(fp);
  fseek(fp,0,SEEK_SET);
  slMsgpack.sMsghead.iBodylen = fread(slMsgpack.aMsgbody,sizeof(char),
    slMsgpack.sMsghead.iBodylen,fp);
  fclose(fp);

  swDebug("���ͱ�������");
  swDebughex((char *)&slMsgpack,
    sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen);
  printf("���ͱ��ĸ�����ƽ̨!\n");
  ilRc = swSendpack(ilQ_des,(char *)&slMsgpack,
    sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen,0,0,0);
  if (ilRc)
  {
    swDebug("swSendpack����: %d",ilRc);
    appQuit(-1);
  }

  ilType = 0;
  ilClass = 0;
  ilPrior = 0;
  ilMsglen = iMSGMAXLEN;
  printf("�ȴ�����ƽ̨���ر���...\n");
  ilRc = swRecvpackw(&ilQ_des,(char *)&slMsgpack,&ilMsglen,&ilPrior,
    &ilClass,&ilType);
  if (ilRc)
  {
    swDebug("swRecvpack����: %d",ilRc);
    appQuit(-1);
  }
  printf("�յ�����ƽ̨���ر���!\n");
  swDebug("���ر�������");
  swDebughex((char *)&slMsgpack,ilMsglen);
  swMbclose();
  exit(0);
}

void appQuit(int iFlag)
{
  swMbclose();
  exit(iFlag);
}
