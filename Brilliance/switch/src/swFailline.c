#include "switch.h"
//#include "swConstant.h"
#include "swDbstruct.h"
#include "swShm.h"

/*static*/ struct shmidx_s *psmShmidx_s;   /* ��̬�����ڴ�Ŀ¼��ָ�� */
/*static*/ struct shmbuf_s smShmbuf_s;     /* ��̬�����ڴ����ݶ�ָ�� */

/***************************************************************
 ** ������      : _sw_add_port_failure
 ** ��  ��      : ͳ�ƶ˿�ʧ�ܴ����������·ʧ�ܳ�����ֵ�����ô���·Ϊ���ߣ�����¼���߿�ʼʱ��
 ** ��  ��      : 
 ** ��������    : 2015/04/08
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : int portid:Դ������
 ** ����ֵ      : 0:�ɹ�-�ǻ��� / -1:ʧ��-���ǻ���
***************************************************************/
int _sw_add_port_failure(int portid)
{
  int j, ilRc;
  

  swVdebug(4, "portid [%d]", portid);
  swVdebug(4,"ggggggggggggggggggg");
  swVdebug(4, "psmShmidx_s->sIdx_sys_queue.iCount[%d]", psmShmidx_s->sIdx_sys_queue.iCount);
  swVdebug(4,"aaaaaaaaaaaaaaaaa");
  for (j = 0; j < psmShmidx_s->sIdx_sys_queue.iCount; j++) 
  {
    swVdebug (3, "smShmbuf_s.psSwt_sys_queue[%d].q_id [%d]", j, smShmbuf_s.psSwt_sys_queue[j].q_id);
    if (smShmbuf_s.psSwt_sys_queue[j].q_id == portid)
      smShmbuf_s.psSwt_sys_queue[j].fail_count += 1; 
    swVdebug(4, "smShmbuf_s.psSwt_sys_queue[%d].fail_count +1 [%d]", j, smShmbuf_s.psSwt_sys_queue[j].fail_count);
    swVdebug(4, "smShmbuf_s.psSwt_sys_queue[%d].max_fail_count [%d]", j, smShmbuf_s.psSwt_sys_queue[j].max_fail_count);
     
    if (smShmbuf_s.psSwt_sys_queue[j].fail_count > smShmbuf_s.psSwt_sys_queue[j].max_fail_count)
    {
      time(&smShmbuf_s.psSwt_sys_queue[j].fail_begin_time);
      swVdebug(4, "smShmbuf_s.psSwt_sys_queue[%d].fail_begin_time [%ld]", j, smShmbuf_s.psSwt_sys_queue[j].fail_begin_time);
      /* ���˿�״̬��ΪDOWN */
      ilRc = swPortset(portid, 1, 0);
      if (ilRc)
      {
        swVdebug(1,"��[%d]�˿�ΪDOWNʧ��", portid);
        return -1;
      }
      else
      {
        swVdebug(4,"��[%d]�˿�ΪDOWN�ɹ�", portid); 
        return 0;
      }
    }
  }

  return -1;	
}

/***************************************************************
 ** ������      : _sw_reset_port_status
 ** ��  ��      : �������㣬����·��״̬����Ϊ����
 ** ��  ��      : 
 ** ��������    : 2015/04/08
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : int portid:Դ������
 ** ����ֵ      : 0:�ɹ�-���� / -1:ʧ��-û������
***************************************************************/
int _sw_reset_port_status(int portid)
{
  int j, ilRc;
  
  swVdebug(4, "portid [%d]", portid);
  for (j = 0; j < psmShmidx_s->sIdx_sys_queue.iCount; j++) 
  {
    swVdebug (3, "smShmbuf_s.psSwt_sys_queue[%d].q_id [%d]", j, smShmbuf_s.psSwt_sys_queue[j].q_id);
    if (smShmbuf_s.psSwt_sys_queue[j].q_id == portid)
    {
      /* ���˿�״̬��ΪUP */
      ilRc = swPortset(portid, 1, 1);
      if (ilRc)
      {
        swVdebug(1,"��[%d]�˿�ΪUPʧ��", portid);
        return -1;
      }
      else
      {
        swVdebug(4,"��[%d]�˿�ΪUP�ɹ�", portid); 
        /* �����ߴ����ͻ��߿�ʼʱ������ */
        smShmbuf_s.psSwt_sys_queue[j].fail_count = 0;
        smShmbuf_s.psSwt_sys_queue[j].fail_begin_time = 0;
        return 0;
      }
    }
  }

  return -1;	
}

/***************************************************************
 ** ������      : _sw_is_port_failure_timeout
 ** ��  ��      : �������Գ�ʱ�ж�
 ** ��  ��      : 
 ** ��������    : 2015/04/08
 ** ����޸�����: 
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : int portid:Դ������
 ** ����ֵ      : 0:�ɹ�-����״̬�����趨ֵ/ -1:ʧ��-����״̬δ�����趨ֵ
***************************************************************/
int _sw_is_port_failure_timeout(int portid)
{
  int j;
  long llCurrentTime;
  
  swVdebug(4, "portid [%d]", portid);
  for (j = 0; j < psmShmidx_s->sIdx_sys_queue.iCount; j++) 
  {
    swVdebug (3, "smShmbuf_s.psSwt_sys_queue[%d].q_id [%d]", j, smShmbuf_s.psSwt_sys_queue[j].q_id);
    if (smShmbuf_s.psSwt_sys_queue[j].q_id == portid)
    {
      time(&llCurrentTime);
      swVdebug(4, "llCurrentTime [%ld]", llCurrentTime);
      swVdebug (3, "smShmbuf_s.psSwt_sys_queue[%d].fail_connect_try_interval [%d]", j, smShmbuf_s.psSwt_sys_queue[j].fail_connect_try_interval);
      if (llCurrentTime - smShmbuf_s.psSwt_sys_queue[j].fail_begin_time > smShmbuf_s.psSwt_sys_queue[j].fail_connect_try_interval)
      {
        swVdebug(4, "���ߵ�ʱ�䳬���趨ֵ");
        return  0;
      }
    }
  }

  return -1;	
}
