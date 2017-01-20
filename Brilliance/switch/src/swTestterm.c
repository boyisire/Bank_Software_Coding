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
    printf("使用方法: swTestterm 通讯邮箱 前置邮箱 报文文件\n");
    printf("(swTestterm 根据前置邮箱信息置报文头,但不分配流水号)\n"); 
    exit(-1);
  }

  ilQ_org = atoi(argv[1]);
  ilQ_des = atoi(argv[2]);
  strcpy(alFmlfile,argv[3]);

  ilRc = swMbopen(ilQ_org);
  if (ilRc)
  {
    swDebug("邮箱打开出错(%d)!",ilRc);
    exit(0);
  }

  swDebug("邮箱打开成功!");

  ilRc = swInit(ilQ_des);
  if (ilRc)
  {
    swDebug("端口初始化出错(%d)!",ilRc);
    appQuit(-1);
  }
  swDebug("端口初始化成功!");

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
    swDebug("FML文件[%s]打开出错!",alFmlfile);
    appQuit(-1);
  }
  swDebug("FML文件[%s]打开成功!",alFmlfile);

  fseek(fp,0,SEEK_END);
  slMsgpack.sMsghead.iBodylen = ftell(fp);
  fseek(fp,0,SEEK_SET);
  slMsgpack.sMsghead.iBodylen = fread(slMsgpack.aMsgbody,sizeof(char),
    slMsgpack.sMsghead.iBodylen,fp);
  fclose(fp);

  swDebug("发送报文内容");
  swDebughex((char *)&slMsgpack,
    sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen);
  printf("发送报文给交换平台!\n");
  ilRc = swSendpack(ilQ_des,(char *)&slMsgpack,
    sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen,0,0,0);
  if (ilRc)
  {
    swDebug("swSendpack错误: %d",ilRc);
    appQuit(-1);
  }

  ilType = 0;
  ilClass = 0;
  ilPrior = 0;
  ilMsglen = iMSGMAXLEN;
  printf("等待交换平台返回报文...\n");
  ilRc = swRecvpackw(&ilQ_des,(char *)&slMsgpack,&ilMsglen,&ilPrior,
    &ilClass,&ilType);
  if (ilRc)
  {
    swDebug("swRecvpack错误: %d",ilRc);
    appQuit(-1);
  }
  printf("收到交换平台返回报文!\n");
  swDebug("返回报文内容");
  swDebughex((char *)&slMsgpack,ilMsglen);
  swMbclose();
  exit(0);
}

void appQuit(int iFlag)
{
  swMbclose();
  exit(iFlag);
}
