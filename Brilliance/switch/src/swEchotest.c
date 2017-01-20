#include "swapi.h" 
#include "swConstant.h"

#include "swNdbstruct.h"

#include "swPubfun.h"
#include "swShm.h"

void swQuit(int sig);
char cgDebug;

/**************************************************************
 ** ������      �� main
 ** ��  ��      �� ������
 ** ��  ��      �� 
 ** ��������    �� 2001/08/13
 ** ����޸����ڣ� 
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� ��
***************************************************************/
int main(int argc, char *argv[])
{
  short ilWaitTime;                             /* ϵͳ�ȴ�ʱ�� */
  long  llCurrentTime;                          /* ϵͳ��ǰʱ�� */
  short ilRc, i, ilCount, ilResultlen;
  unsigned int ilMsglen;
  char  alTrancode[11];
  char  alResult[iFLDVALUELEN+1];

  short ilPriority = 0, ilClass = 0, ilType = 0;        /* ������� */
  struct msgpack slMsgpack;                             /* ���� */
  struct swt_sys_queue pslSwt_sys_queue[iMAXQUEUEREC];	/* �˿� */

  FILE *plPopenfile;                            /* add by dgm 2001.9.5 */
  short ilPopen;                                /* add by dgm 2001.9.5 */

  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swEchotest Version 4.3.0",argv[1]);

  /* LICENSE */
  /*delete by guofeng 20030109 
  if (_swLicense("swEchotest")) exit(FAIL);
  */
  
  /* ���õ��Գ������� */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strcpy(agDebugfile, "swEchotest.debug");

  /* ȡ��DEBUG��־ */
  if ((cgDebug = _swDebugflag("swEchotest")) == FAIL)
  {
    printf("S0010 : ȡDEBUG��־ʧ��");
    exit(FAIL);
  }
  swVdebug(2,"S0010: ȡ�ð汾��[4.3.0]");
  swVdebug(2,"S0020: ���õ��Գ�������[%s]", agDebugfile);
  swVdebug(2,"S0030: ȡ��DEBUG��־[%d]", cgDebug);

  /* add by dgm 2001.9.5 */
  plPopenfile = popen("ps -e|grep swEchotest|wc", "r");
  fscanf(plPopenfile, "%hd", &ilPopen);  
  if (ilPopen > 1)
  {
    swVdebug(0,"S0040: [����/����] swEchotest�����Ѵ��ڣ�");
    pclose(plPopenfile);
    exit(FAIL);
  }
  else
    pclose(plPopenfile);
  /* end of add by dgm 2001.9.5 */
  

  /* ��λ�������� */
  if (swMbopen(iMBECHOTEST))
  {
    swVdebug(0,"S0050: [����/����] ��λ��������ʧ��");
    exit(FAIL);
  }
  swVdebug(2,"S0060: ��λ��������ɹ�");
  
  /* ��ʼ�������ڴ�ָ�� */
  if(swShmcheck() != SUCCESS)
  {
    swVdebug(0,"S0070: [����/�����ڴ�] ��ʼ�������ڴ�ָ�����");
    exit(FAIL);
  }
  swVdebug(2,"S0080:��ʼ�������ڴ�ָ��ɹ�");


  /* �����ź� */
  signal(SIGTERM, swQuit);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  swVdebug(2,"S0090: signal()�����ź�");

  /* ��ϵͳ��ط�����Ϣ���� */
  _swMsgsend(104001,NULL);
  swVdebug(2,"S0100: ����104001��Ϣ,[swEchotest]�Ѿ�����...");

  /* ��ʼ�� */
  ilRc = swInit(iMBECHOTEST);
  if (ilRc == FAIL) 
  {
    swVdebug(0,"S0110: [����/��������] swInit()����,��ʼ��Echotest����ʧ��");
    swQuit(FAIL);
  }  
  /* ȡ�õ�ǰϵͳʱ�� */
  time(&llCurrentTime);
  ilRc = swShmselect_swt_sys_queue_all(pslSwt_sys_queue, &ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0120: [����/�����ڴ�] ȡ�˿����ñ�ʧ��");
    exit(FAIL);
  }
  for (i=0; i<ilCount; i++)
  {
    if (pslSwt_sys_queue[i].trigger_freq > 0)
    {
      pslSwt_sys_queue[i].trigger_time = llCurrentTime + 
        pslSwt_sys_queue[i].trigger_freq;
      ilRc = swShmupdate_swt_sys_queue(pslSwt_sys_queue[i].q_id,
        pslSwt_sys_queue[i]);
      if (ilRc != SUCCESS)
      {
        swVdebug(1,"S0130: [����/�����ڴ�] ���¶˿�[%d]����", pslSwt_sys_queue[i].q_id);
        swQuit(FAIL);
      }
    }
  }

  /* ��ѯ�˿ڱ� */
  while (TRUE)
  {
    /* ��⹲���ڴ�״̬ */
    if (swShmcheck() == FAIL)
    {
      swVdebug(0,"S0140: [����/�����ڴ�] ��⹲���ڴ�״̬ʧ��");
      swQuit(FAIL);
    }

    /* ��ȡϵͳ��ѭʱ�� */
    ilRc = swShmselect_swt_sys_config(&sgSwt_sys_config);
    if (ilRc == FAIL)
    {
      swVdebug(1,"S0150: [����/�����ڴ�] ��ȡϵͳ��ѭʱ��ʧ��");
      swQuit(FAIL);
    }
    ilWaitTime = sgSwt_sys_config.echotest_inter;

    sleep(ilWaitTime);

    /* �屾������ */
    if ((ilRc = _swClrMb(iMBECHOTEST)) != SUCCESS)
    {
      swVdebug(1,"S0160: [����/����] �屾������[%d]����,������=%d", iMBECHOTEST,ilRc);
      exit(FAIL);
    }
    swVdebug(2,"S0170: �屾������[%d]�ɹ�", iMBECHOTEST);

    ilRc = swShmselect_swt_sys_queue_all(pslSwt_sys_queue, &ilCount);
    if (ilRc != SUCCESS)
    {
      swVdebug(1,"S0180: [����/�����ڴ�] ȡ�˿����ñ�ʧ��");
      swQuit(FAIL);
    }

    /* ȡ�õ�ǰϵͳʱ�� */
    time(&llCurrentTime);

    for (i=0; i<ilCount; i++)
    {
      if ((pslSwt_sys_queue[i].trigger_freq > 0) && 
          (llCurrentTime >= pslSwt_sys_queue[i].trigger_time))
      {
        /* ���㴥��ʱ�����ʽ */
        ilRc = _swExpress(pslSwt_sys_queue[i].trigger_term, alResult, 
          &ilResultlen);
        if (ilRc == FAIL)
        {
          swVdebug(1,"S0190: [����/��������] _swExpress()����,����Ӧ�ö�[%d]����ʱ�����ʽ����[%s]", 
            pslSwt_sys_queue[i].q_id,
            pslSwt_sys_queue[i].trigger_term);
          _swMsgsend(303006, NULL);
          continue;
        }
        
        if (alResult[0] == '1')
        {
          /* ��֯Echotest���ģ��������� */
          swNewtran(&(slMsgpack.sMsghead));
       
          /* ������Ϊ�˿ں� */
          sprintf(alTrancode, "%d", pslSwt_sys_queue[i].q_id);
          
          memcpy(slMsgpack.sMsghead.aTrancode, alTrancode, 
            strlen(alTrancode));
          memcpy(slMsgpack.sMsghead.aBegin_trancode, alTrancode, 
            strlen(alTrancode));

	  swVdebug(2,"S0200:[%s]",slMsgpack.sMsghead.aTrancode);
          slMsgpack.sMsghead.iMsgtype = 101;  /* ������� */
          slMsgpack.sMsghead.iBodylen = 0;    /* �հ� */
          slMsgpack.sMsghead.iOrg_q = iMBECHOTEST;    /* Դ������ */
          
          if (slMsgpack.sMsghead.cFormatter == '0')
          /* ���Զ����и�ʽת�� */
          {        
            ilRc = swFormat(&slMsgpack);
            if (ilRc < 0)
            {
              swVdebug(1,"S0210: [����/��������] swFormat()����,���ʧ��,������=%d",ilRc);
              continue;
            }
          }  
          slMsgpack.sMsghead.iMsgtype = 1; /* Ӧ�ñ���iMSGAPP */
        
          /* ����Ӧ�ñ��ĵ��������� */
          ilMsglen = sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen;
          ilPriority = 0;
          ilClass = 0;
          ilType = 0;
          ilRc = swSendpack(iMBCTRL, (char *)&slMsgpack, ilMsglen, 
            ilPriority, ilClass, ilType);
          if (ilRc) 
          {
            swVdebug(1,"S0220: [����/��������] swSendpack()����,����Ӧ�ñ��ĵ���������ʧ��,������=%d",ilRc);
            swQuit(FAIL);
          }
        }
        /* ���´���ʱ�� */
        pslSwt_sys_queue[i].trigger_time = llCurrentTime +
          pslSwt_sys_queue[i].trigger_freq;
        ilRc = swShmupdate_swt_sys_queue(pslSwt_sys_queue[i].q_id,
          pslSwt_sys_queue[i]);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0230: [����/�����ڴ�] ���¶˿�[%d]����", pslSwt_sys_queue[i].q_id);
          swQuit(FAIL);
        }
      }
    }
  }
}
/**************************************************************
 ** ������      ��swQuit
 ** ��  ��      ���ͷ������Դ,�ر����ݿ� 
 ** ��  ��      :
 ** ��������    ��2001/07/09
 ** ����޸����ڣ�2001/07/09
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
***************************************************************/
void swQuit(int sig)
{
  short ilRc;

  signal(SIGTERM, SIG_IGN);

  /* �ͷ���Դ */
/*  EXEC SQL close database; */
  ilRc = swMbclose();
  if (ilRc) 
  {
    swVdebug(0,"S0240: �ͷ���Դ����!");
    exit(-1);
  }
  exit(0);
} 
