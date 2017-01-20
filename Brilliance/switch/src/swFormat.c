/****************************************************************/
/* ģ����    ��FORMAT                                         */
/* ģ������    ����ʽ����                                       */
/* �� �� ��    ��V4.3.0                                         */
/* ��    ��    ��ʷ����                                         */
/* ��������    ��1999/11/18                                     */
/* ����޸����ڣ�2001/4/8                                       */
/* ģ����;    ����ʽ����                                       */
/* ��ģ���а������º���������˵����                             */
/*			 ��1��void main();                      */
/*			 ��2��void swQuit();                    */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/*   2000/3/13 ���Ӷ�MAC�Ĵ���                                  */
/*   2000/8/24 �Ż�,�˳�ʱFREE�ڴ�                              */
/*   2001/2/22 Oracle��ֲ                                       */
/*   2001/2/27 ����DEBUG����                                    */
/*   2001/3/24 ���V3.01,����1114��                           */
/*   2001/4/6  INFORMIX��ֲ(1116)                               */
/*   2001/4/8  �����˶Խ�����Ϊ�յĴ�����(1125)               */
/****************************************************************/

/* switch���� */
#include "switch.h"

#include "swNdbstruct.h"
#include "swShm.h"

/* ����ԭ�Ͷ��� */
void   swQuit(int);
short  swFormat(struct msgpack *psMsgpack);

/**************************************************************
 ** ������      ��main
 ** ��  ��      ����ʽ����ģ��������
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.8.24
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ����
***************************************************************/
int main(int argc, char *argv[])
{
  short ilRtncode;                   /* ������ */
  unsigned int ilReadSize;                  /* �����ĳ��� */
  struct msgpack  slMsgpack;    
  short ilOrigid=0;
  short ilPrior=0;    
  short ilClass=0;
  short ilType =0;

  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swFormat Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swFormat")) exit(FAIL);

  /* ���õ��Գ������� */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strncpy(agDebugfile, "swFormat.debug", sizeof(agDebugfile));

  /* ȡ��DEBUG��־ */
  if ((cgDebug = _swDebugflag("swFormat")) == FAIL)
  {
    fprintf(stderr, "main():_swDebugflag():�޷�ȡ��DEBUG��־!\n");
    exit(FAIL);
  } 

  /* ����SIGCLD��SIGINT��SIGQUIT��SIGHUP�ź� */
  signal( SIGTERM , swQuit  );
  signal( SIGINT  , SIG_IGN );
  signal( SIGQUIT , SIG_IGN );
  signal( SIGTSTP , SIG_IGN );
  signal( SIGHUP  , SIG_IGN );

  /* ��λ�������� */
  if (qattach(iMBFORMAT)) 
  {
    swVdebug(0,"S0010: [����/����] qattach()����");
     exit(FAIL);
  }

  ilRtncode = swShmcheck(); /* ��ʼ�������ڴ�ָ�� */

  if (ilRtncode != SUCCESS)
  {
    swVdebug(0,"S0020: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
     swQuit(FAIL);
  }

  /* ��������(��ʽ��������)����̨��� */
  _swMsgsend(102001, NULL);
  swVdebug(2,"S0030: ��ʽ����ģ���Ѿ�����......");

  for( ; ; )
  {
    ilPrior = 0;
    ilClass = 0;
    ilType  = 0;

    /* ��ȡ�����е���Ϣ */ 
    /* memset((char *)&slMsgpack, 0x00, iMSGMAXLEN ); */
    ilReadSize = iMSGMAXLEN;
    if ((ilRtncode = qread2((char *)&slMsgpack, &ilReadSize, 
          &ilOrigid, &ilPrior, &ilClass, &ilType)) != SUCCESS) 
    {
      swVdebug(0,"S0040: [����/��������] qread2()����,������=%d",ilRtncode);
       swMberror(ilRtncode, NULL);
      swQuit(FAIL);
    }

    ilRtncode = swShmcheck(); /* ��⹲���ڴ�������б䶯����ˢ��ָ�� */

    if (ilRtncode != SUCCESS)
    {
      swVdebug(0,"S0050: [����/��������] �����ڴ�ָ��ˢ��ʧ��");
       swQuit(FAIL);
    }
    swVdebug(2,"S0060: qread2():�յ�����[%d]�����ı���,���ȼ�=[%d],class=[%d],type=[%d]", ilOrigid, ilPrior, ilClass, ilType);
     swVdebug(3,"S0070: ���Ŀ�ʼ");
     if (cgDebug >= 2)
    {
      swDebugmsghead((char *)&slMsgpack);
      swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
    } 
    swVdebug(3,"S0080: ���Ľ���"); 
 
    ilRtncode = swFormat(&slMsgpack); /* ��ʽת������ */

    switch (ilRtncode)
    {
      case 0: 
        /* ��ʽת���ɹ� */
        /* ���ͱ����������ߵ����� */
        swVdebug(2,"S0090: ���ͱ���(����=%d)������[%d]!",
             slMsgpack.sMsghead.iBodylen,ilOrigid);
        swVdebug(3,"S0100: ���ͱ��Ŀ�ʼ");     
         if (cgDebug >= 2)
        {
          swDebugmsghead((char *)&slMsgpack);
          swDebughex(slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
         
        }
        swVdebug(3,"S0110: ���ͱ��Ľ���");
 
        ilRtncode = qwrite2((char *)&slMsgpack,
          slMsgpack.sMsghead.iBodylen+sizeof(struct msghead), ilOrigid,
          ilPrior, ilClass, ilType);
        if (ilRtncode != SUCCESS) 
        {
          swVdebug(0,"S0120: [����/��������] qwrite2()����,������=%d",ilRtncode);
           swMberror(ilRtncode, NULL);
          swQuit(FAIL);
        }
        else
        swVdebug(2,"S0130: ���ʹ�����ĵ�[%d]����ɹ�!", ilOrigid);
         break;
      case -1:
        /* �������ʹ��� */

        /* ����ͷ���� */
        /* Ԥ���ʧ�� */
        /* �Զ��庯���������(�����������) */
        /* ������Ϊ�� */
        /* �޷���λ��ʽת���� */
        /* ����ת���������ʽ���Ͳ�ƥ�� */
        /* ������Ӧ��ʽת��������FMLԤ�ð����� */
        /* MACУ����� */
        
        swVdebug(0,"S0140: [����/����] �������ʹ���");
         break;
      case -3:
        /* ����ʶ��ı������� */
        swVdebug(0,"S0150: [����/����] ����ʶ��ı�������");
         break;
      default:
        break;
    }
  }
}

 /**************************************************************
 ** ������      ��swQuit
 ** ��  ��      ���ͷ������Դ 
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999/11/22
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� SUCCESS, FAIL
***************************************************************/
void swQuit(int ilSig)
{
  short ilRtncode;
  signal(ilSig, SIG_IGN);

  ilRtncode = qdetach();
  if (ilRtncode < 0)
  {
    swVdebug(0,"S0160: [����/����] qdetach()����,������=%d",ilRtncode);
     exit(FAIL);
  }

  /* ��ϵͳ��ط�����Ϣ���� */
  ilRtncode = _swMsgsend(102002, NULL);

  exit(ilSig);
}
