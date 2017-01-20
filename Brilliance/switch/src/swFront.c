/****************************************************************/
/* ģ����    ��swFront                                        */ 
/* ģ������    ��ͨ��ǰ�ý���                                   */
/* ��	 ��    ��                                               */
/* ��������    ��2000/10/05                                     */
/* ����޸����ڣ�2001/7/12                                      */
/* ģ����;    ��ǰ�ý���                                       */
/* ��ģ���а������º���������˵����                             */
/*	        (1) void main();                                */
/*              (2) void swQuit(int);                           */
/****************************************************************/
/* ͷ�ļ����� */
#include <signal.h>
#include "swapi.h"
#include "swConstant.h"

char cgDebug=0;
/* ����ԭ�� */
void swQuit(int);

extern int swRecvpacklw(short *, char *, unsigned int *, short *, long *, long *);
extern int swSendpackl(short, char *, unsigned int, short, long, long);
extern int swDbopen();
extern int swDbclose();
 
/**************************************************************
 ** ������      �� main
 ** ��  ��      �� ������
 ** ��  ��      �� nh 
 ** ��������    ��
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/
int main(argc,argv)
 int argc;
 char **argv;
{
  struct msgpack slMsgpack;             /* ���� */
  short  ilQid_fore,ilQid_comm;         /*ǰ�������ͨѶ����*/
  short  ilRc;                          /* ������ */
  unsigned int ilMsglen;                      /* ��Ϣ���� */
  short  ilQid;                         /* ����� */
  short  ilPriority;
  long   llClass,llType;
  short  ilStaflag = 0;
  short  ilTypeflag = 0;
  int    c;

  /* �����ź� */
  signal(SIGTERM,swQuit);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGCLD, SIG_IGN); 

  c=0; 
  while((c=getopt(argc,argv,"f:c:d:ts")) != -1)
  {
    switch( c )
    {
      case 'f':  /* ����ǰ������ */
        ilQid_fore=atoi(optarg);
        break;
      case 'c':  /* ����ͨѶ���� */
        ilQid_comm=atoi(optarg);
        break;
      case 'd':  /* ����TRACE���� */
        cgDebug=atoi(optarg);
        break;
      case 't':  /* ��������type,class��ʶ */
        ilTypeflag = 1;
	break;
      case 's':  /* ���彻��Դ���˿ڱ�ʶ */
        ilStaflag = 1;
        break;
      default:
        break;
    }
  }
  
  if ((ilQid_fore&&ilQid_comm)==0) 
  {
    printf("ʹ�÷���: swFront -f ǰ������ -c ͨѶ���� [-d TRACE����] [-t] [-s]\n");
    printf("           -t: ���ñ��Ĺ�������(type,class)\n");
    printf("           -s: ����Դ���˿�\n");
    exit( 0 );
  }

  /* ���õ�����Ϣ�ļ� */
  memset(agDebugfile,0x00,sizeof(agDebugfile));
  sprintf(agDebugfile,"swFront_%d.debug",ilQid_fore);

  swVdebug(2,"S0000: ǰ�������=%d",ilQid_fore);
  swVdebug(2,"S0010: ͨѶ�����=%d",ilQid_comm);
  swVdebug(2,"S0020: TRACE����=%d",cgDebug); 
  swVdebug(2,"S0030: ����type,class��ʶ=%d",ilTypeflag);
  swVdebug(2,"S0030: ����Դ���˿ڱ�ʶ=%d",ilStaflag);

  swVdebug(2,"S0030: FLDNAMELEN=%d",iFLDNAMELEN);
  /* ����ƽ̨�����ڴ� */
  ilRc = swShmcheck();
  if (ilRc)
  {
    swVdebug(0,"S0040: ��⹲���ڴ����!");
    exit(-1);
  }
  swVdebug(2,"S0045: ���ӹ����ڴ�ɹ�");

  /* ��ʼ��ǰ�ý��� */
  ilRc = swInit(ilQid_fore);
  if (ilRc) 
  {
    swVdebug(0,"S0050: swInit():��ʼ��ǰ�ý���ʧ��!");
    exit(-1);
  }
  swVdebug(2,"S0060: ��ʼ��ǰ�ý���(%d)�ɹ�",ilQid_fore);

  /* ��ʼ��ǰ�ý��̵����� */
  ilRc = swMbopen(ilQid_fore);
  if (ilRc) 
  {
    swVdebug(0,"S0070: swMbopen():�����ʼ��ʧ��!");
    exit(-1);
  }
  swVdebug(2,"S0080: ��ʼ��ǰ�ý�������(%d)�ɹ�",ilQid_fore);

  /* �������ݿ� */
  ilRc = swDbopen();
  if (ilRc) 
  {
    swVdebug(0,"S0090: swDbopen():ʧ��!");
    exit(-1);
  }

  /* ѭ����ȡ����ı��ģ���������Ӧ�Ĵ��� */
  for(;;)
  {
    ilQid=0;
    llClass = 0;
    llType = 0;
    ilPriority = 0;
    ilMsglen = iMSGMAXLEN;

    /* ���ձ��� */
    ilRc = swRecvpacklw(&ilQid,(char *)&slMsgpack,&ilMsglen,&ilPriority, \
      &llClass,&llType);
    if (ilRc) 
    {
      swVdebug(1,"S0100: swRecvpackw():���ձ��ĳ���,ilRc=[%d]",ilRc);
      swQuit(-1);
    }
    swVdebug(2,"S0110: ���յ�������[%d]�����ı���,prior=[%d],class=[%d],type=[%d]",ilQid,ilPriority,llClass,llType);

/* ���֧�����ö�̬ˢ��,��Ҫȥ��ע�� */
    ilRc = swShmcheck();
    if (ilRc)
    {
      swVdebug(2,"S0120: ��⹲���ڴ����!");
      swQuit(-1);
    }
/**/

/*
    if (ilQid==iMBCTRL)  
      swVdebug(2,"S0130: ����CTRL���䷢���ı���");

    if ((ilQid==ilQid_comm)&&(ilStaflag)) 
      swVdebug(2,"S0140: �½��׿�ʼ:�����ⲿͨѶ���䷢���ı���");
*/

    swVdebug(3,"S0150: ����BEGIN.....");
    if(cgDebug >= 2)
      swDebughex((char *)&slMsgpack,ilMsglen);
    swVdebug(3,"S0160: ����END.....");

    /* ��Ҫ����ƽ̨�ڲ��ı���ͷ(�½���), ������llCass,llType*/
    if ((ilQid==ilQid_comm)&&(ilStaflag)) 
    {
      swVdebug(2,"S0170: �����½���");
/* ���ͨѶ�������Ĳ�������ͷ,��Ҫȡ��ע��
      memmove((char *)&slMsgpack+sizeof(struct msghead),(char *)&slMsgpack,ilMsglen);
      ilMsglen = ilMsglen + sizeof(struct msghead);
*/
      swNewtran(&(slMsgpack.sMsghead)); 
      slMsgpack.sMsghead.iBodylen = ilMsglen - sizeof(struct msghead);
    }
    /* ����llClass,llType*/
    if ((ilQid==ilQid_comm)&&(ilTypeflag))
    {
      swVdebug(2,"S0180: ����type,class������ͷ�е�aMemo��");
      memcpy(slMsgpack.sMsghead.aMemo,(char *)&llClass,sizeof(long));
      memcpy(slMsgpack.sMsghead.aMemo + sizeof(long),(char *)&llType,sizeof(long));
    }

    if(cgDebug >= 2)
      swDebugmsghead((char *)&(slMsgpack.sMsghead));
    if(cgDebug >= 2)
      swDebughex(slMsgpack.aMsgbody,ilMsglen-sizeof(struct msghead));

    switch(slMsgpack.sMsghead.iMsgtype)
    {
      case iMSGAPP:  /* Ӧ�ñ��� */
        if (ilQid == ilQid_comm)
        {
          /* ͨѶ�˿ڷ�����Ӧ�ñ���*/
          swVdebug(2,"S0210: �յ�ͨѶ�˿�[%d]��Ӧ�ñ���",ilQid_comm);

          /* �ñ�������Ϊ�������,��׼�����н������ */
          slMsgpack.sMsghead.iMsgtype = iMSGUNPACK;
          /* ��Դ�����ó�Ŀ������ */
          slMsgpack.sMsghead.iOrg_q = slMsgpack.sMsghead.iDes_q;
          /* Ŀ�������ó�0 */
          slMsgpack.sMsghead.iDes_q = 0;
          slMsgpack.sMsghead.iBodylen = ilMsglen - sizeof(struct msghead);
          /* ����ͷ�еı��ĸ�ʽ�óɶ˿�ָ���ı��ĸ�ʽ(8583,SEP,NOSEP,FML,XMF) */
          slMsgpack.sMsghead.iMsgformat = sgPortattrib.iMsgformat;
	  swVdebug(2,"S0220: ����swFormat�������н������");
           
          ilRc=swFormat(&slMsgpack);
          if (ilRc)
          {
            swVdebug(2,"S0230: ���ø�ʽת������swFormat()����,������=%d",ilRc);
            continue;
          }
       
/*
          ilRc = swFmlpackget(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,"a18",&ilFldlen, alTmp);
          if ( ilRc )
          {
            swVdebug(0,"S0340: Ӧ�ñ���������[a19]");
            continue;
          }
          swVdebug(2,"�������[%s]",alTmp);
          ilRc = swFmlpackget(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen,"a19",&ilFldlen, alTmp);
          if ( ilRc )
          {
            swVdebug(2,"S0340: Ӧ�ñ���������[a19]");
            continue;
          }
          swVdebug(2,"������ʾ:[%s]",alTmp);
*/

          /* �ñ�������ΪӦ�ñ��� */
          slMsgpack.sMsghead.iMsgtype = iMSGAPP;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);

	  swVdebug(2,"S0225: ����Ӧ�ñ��ĵ���������");
          ilRc = swSendpackl(iMBCTRL,(char *)&slMsgpack,ilMsglen,ilPriority, 
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0240: ���͵���������[%d]����[%d]",iMBCTRL,ilRc);
            continue;
          }
          continue;
        }
        else if(ilQid==iMBCTRL) 
        {
          /* ���ط�����Ӧ�ñ��� */
          swVdebug(2,"S0260: �յ����ط�����Ӧ�ñ���");  

          /* �ñ�������Ϊ�������,��׼�����д������ */
          slMsgpack.sMsghead.iMsgtype = iMSGPACK; 

	  swVdebug(2,"S0265: ����swFormat�������д������");
          ilRc=swFormat(&slMsgpack);
          if (ilRc)
          {
            swVdebug(2,"S0270: ���ø�ʽת������swFormat()����,������=%d",ilRc);
            continue;
          }

          /* �ñ�������ΪӦ�ñ��� */
          slMsgpack.sMsghead.iMsgtype = iMSGAPP;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);
          if (ilTypeflag == 1)
          {
            /* �ڱ���ͷaMemo����ȡ��type,class�� */
	    /*
            llClass = *((long *)slMsgpack.sMsghead.aMemo);
            llType = *((long *)(slMsgpack.sMsghead.aMemo + sizeof(long)));
            */
   memcpy(&llClass,slMsgpack.sMsghead.aMemo,sizeof(long));
   memcpy(&llType,slMsgpack.sMsghead.aMemo + sizeof(long),sizeof(long));
          }
          else
          {
            llClass = 0;
            llType = 0;
          }
          swVdebug(2,"S0000: ����Ӧ�ñ��ĵ�ͨѶ����[%d]: class=[%ld],type=[%ld]",ilQid_comm,llClass,llType);
          ilRc = swSendpackl(ilQid_comm,(char *)&slMsgpack,ilMsglen,ilPriority,
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0280: ���ͱ��ĸ�ͨѶ����[%d]����[%d]",ilQid_comm,ilRc);
            continue;
          }
          continue;
        }
        else 
        {
          swVdebug(2,"S0300: �յ�δ֪����[%d]������Ӧ�ñ���",ilQid);
          continue;
        }
        break;

      case iMSGREV:   /* �������� */
        if ((ilQid==iMBMONREV) || (ilQid==iMBCTRL))
        {
          swVdebug(2,"S0310: �յ�����[%d]�����ĳ�������",ilQid);
          /* �ñ�������Ϊ�������,�Խ��д������ */
          slMsgpack.sMsghead.iMsgtype = iMSGREVPACK;     

	  swVdebug(2,"S0265: ����swFormat�������г����������");
          ilRc=swFormat(&slMsgpack);
          if (ilRc)
          {
            swVdebug(2,"S0320: ���ø�ʽת������swFormat()����,������=%d",ilRc);
            continue;
          }

          /* �ñ�������Ϊ�������� */
          slMsgpack.sMsghead.iMsgtype = iMSGREV;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);
          if (ilTypeflag == 1)
          {
	  /*
            llClass = *((long *)slMsgpack.sMsghead.aMemo);
            llType = *((long *)(slMsgpack.sMsghead.aMemo + sizeof(long)));
          */
   memcpy(&llClass,slMsgpack.sMsghead.aMemo,sizeof(long));
   memcpy(&llType,slMsgpack.sMsghead.aMemo + sizeof(long),sizeof(long));
          }
          else
          {
            llClass = 0;
            llType = 0;
          }
          swVdebug(2,"S0000: ����ͨѶ����[%d]: class=[%ld],type=[%ld]",ilQid_comm,llClass,llType);

          ilRc = swSendpackl(ilQid_comm,(char *)&slMsgpack,ilMsglen,ilPriority,
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0330: ���ͳ������ĵ�ͨѶ����[%d]ʧ��![%d]",
              ilQid_comm,ilRc );
            continue;
          }
          continue;
        }
        else if (ilQid == ilQid_comm) 
        {
          swVdebug(2,"S0350: �յ�ͨѶ�˿�[%d]�ĳ�������",ilQid_comm);
          /* �ñ�������Ϊ�������,�Խ��н������ */
          slMsgpack.sMsghead.iMsgtype = iMSGREVUNPACK;
          slMsgpack.sMsghead.iOrg_q=slMsgpack.sMsghead.iDes_q;
          slMsgpack.sMsghead.iDes_q=0;
          slMsgpack.sMsghead.iBodylen=ilMsglen-sizeof(struct msghead);
          slMsgpack.sMsghead.iMsgformat=sgPortattrib.iMsgformat;

	  swVdebug(2,"S0265: ����swFormat�������г����������");
          ilRc=swFormat(&slMsgpack);
          if (ilRc != SUCCESS)
 	  {
            swVdebug(2,"S0360: ���ø�ʽת������swFormat()����,code=%d",ilRc);
            continue;
          }

          slMsgpack.sMsghead.iMsgtype = iMSGREV;
          ilMsglen = slMsgpack.sMsghead.iBodylen+sizeof(struct msghead);

          swVdebug(2,"S0000: ���ͳ������ĵ���������");
          ilRc = swSendpackl(iMBCTRL,(char *)&slMsgpack,ilMsglen,ilPriority,
            llClass,llType);
          if (ilRc) 
          {
            swVdebug(2,"S0370: ���ͳ������ĸ������������,������=%d",ilRc);
            continue;
          }
          continue;
        }
        else
        {
          swVdebug(2,"S0390: �յ�δ֪����[%d]�����ĳ�������",ilQid);
          continue;
        }
        break;

      default :
        swVdebug(2,"S0400: �յ�δ֪��������[%d]�ı���",slMsgpack.sMsghead.iMsgtype);
        break;
    }
  }
}

/**************************************************************
 ** ������      ��swQuit
 ** ��  ��      ���ͷ������Դ,�ر����ݿ� 
 ** ��  ��      :
 ** ��������    ��2000/5/8
 ** ����޸����ڣ�2000/5/8
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
***************************************************************/
void swQuit(int sig)
{
  short ilRtncode;

  signal(SIGTERM,SIG_IGN);

  ilRtncode = swDbclose();
  if (ilRtncode)
  {
     swVdebug(0,"S0410: �ر����ݿ�ʧ��!");
  }

  /* �ͷ���Դ */
  ilRtncode=swMbclose();
  if (ilRtncode) 
  {
    swVdebug(0,"S0420: �ر��������!");
  }
  exit(0);
} 
