#include <stdio.h>
#include "swapi.h"

char cgDebug=2;

extern int swRecvpacklw(short *, char *, unsigned int *, short *, long *, long *);
extern int swSendpackl(short, char *, unsigned int, short, long, long);


int main(int argc,char **argv)
{
  short ilQ_org,ilQ_dest;
  unsigned int ilMsglen,ilMsgreturn;
  long  llClass,llType;
  short i,ilRc,ilPriority;
  short ilQid,ilSendnum;
  long lltime1,lltime2;
  struct msgpack slMsgpack;
  union preunpackbuf pslPrebuf[iBUFFLDNUM];
 /* char alCode[10];*/

  strcpy(agDebugfile,"swTest.debug");
  /* 读取DEBUG标志 */
  if(argc != 4) {
    fprintf(stderr, "使用方法: swTest 发送次数 源邮箱 目标邮箱\n");
    exit(-1);
  }

  ilSendnum = atoi(argv[1]);
  ilQ_org = atoi(argv[2]);
  ilQ_dest = atoi(argv[3]);
/*  strcpy(alCode,argv[4]);*/

  /* 组成1个FML报文 */

  pslPrebuf[0].sImfbuf.aFldname[0] = 0;

/*  swFmlset("val01",4,alCode,pslPrebuf);
  swFmlset("val02",5,"22222",pslPrebuf);
  swFmlset("val03",6,"333333",pslPrebuf);
  swFmlset("val04",7,"4444444",pslPrebuf);
  swFmlset("val05",3,"555",pslPrebuf);
  swFmlset("val06",4,"6666",pslPrebuf);
  swFmlset("val07",4,"7777",pslPrebuf);
  swFmlset("val08",3,"888",pslPrebuf);

  swFmlset("serilano",15,"123456789",pslPrebuf);
  swFmlset("filename",64,"test.txt",pslPrebuf);
  swFmlset("filesize",6,"100",pslPrebuf);
  swFmlset("sendflag",2,"1",pslPrebuf);
  swFmlset("regtype",2,"1",pslPrebuf);
  swFmlset("orggrpid",2,"1",pslPrebuf);
  swFmlset("orgmbid",2,"1",pslPrebuf);
  swFmlset("desgrpid",2,"1",pslPrebuf);
  swFmlset("desmbid",2,"2",pslPrebuf);
  swFmlset("begintime",20,"2006-12-26 12:21:21",pslPrebuf);
  swFmlset("endtime",20,"2006-12-26 12:21:31",pslPrebuf);
  swFmlset("statusdesc",128,"this is just a test.",pslPrebuf);
*/  
/*
  swFmlset("serilano",15,"123456789",pslPrebuf);
  swFmlset("class",64,"10",pslPrebuf);
  swFmlset("type",6,"1001",pslPrebuf);
  swFmlset("regtype",6,"1",pslPrebuf);
  swFmlset("srcqid",5,"50",pslPrebuf);
  swFmlset("desqid",5,"51",pslPrebuf);
  swFmlset("flag",2,"1",pslPrebuf);
  swFmlset("tradetime",20,"2006-12-26 12:21:21",pslPrebuf);
  swFmlset("statusdesc",128,"this is just a test.",pslPrebuf);  
*/
  swFmlset("tradecode",10,"1000",pslPrebuf);
  swFmlpack(pslPrebuf,slMsgpack.aMsgbody,&ilMsglen);
  slMsgpack.sMsghead.iBodylen = ilMsglen;

  swVdebug(3,"组成 fml 格式的报文体, ilmsglen = [%d]", ilMsglen);
  swVdebug(3,"组成的报文体为: ");
  if (cgDebug>1)
    swDebughex(slMsgpack.aMsgbody,ilMsglen);

  fprintf(stderr,"This is a test pack!\n");

  ilRc = swMbopen(ilQ_org);
  if (ilRc)
  {
    fprintf(stderr,"打开邮箱[%d]出错!\n",ilQ_org);
    exit(-1);
  }
  time(&lltime1);
  fprintf(stderr,"开始发送报文...\n");
  for(i=1;i<=ilSendnum;i++)
  {
    ilPriority = 0;
    llClass = 0;
    llType = 0;
    ilRc = swSendpackl(ilQ_dest,(char *)&slMsgpack,
      ilMsglen + sizeof(struct msghead),
      ilPriority,llClass,llType);
    if (ilRc)
    {
      fprintf(stderr,"报文发送时出错: %d",ilRc);
      exit(-1);
    }
  }
  fprintf(stderr,"开始接收报文...\n");
  for(i=1;i<=ilSendnum;i++)
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
  }
    
  fprintf(stderr,"接收报文完毕!!!\n");
  fprintf(stderr,"swTest is ok!!!\n");

  swVdebug(3,"从[%d]邮箱收到的报文:",ilQid);
  if (cgDebug>1)
    swDebughex((char *)&slMsgpack,ilMsgreturn);

  time(&lltime2);
 
  fprintf(stderr,"使用时间 = [%ld]\n",(lltime2-lltime1));
  swVdebug(3,"使用时间 = [%ld]",(lltime2-lltime1));
  
  /* 关闭前置进程所属端口 */
  swMbclose();
  exit(0);
}

