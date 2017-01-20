#include "switch.h"
//#include "swConstant.h"
#include "swDbstruct.h"
#include "swShm.h"

/*static*/ struct shmidx_s *psmShmidx_s;   /* 静态表共享内存目录段指针 */
/*static*/ struct shmbuf_s smShmbuf_s;     /* 静态表共享内存内容段指针 */

/***************************************************************
 ** 函数名      : _sw_add_port_failure
 ** 功  能      : 统计端口失败次数，如果线路失败超过阈值，设置此线路为坏线，并记录坏线开始时间
 ** 作  者      : 
 ** 建立日期    : 2015/04/08
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : int portid:源发邮箱
 ** 返回值      : 0:成功-是坏线 / -1:失败-不是坏线
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
      /* 将端口状态置为DOWN */
      ilRc = swPortset(portid, 1, 0);
      if (ilRc)
      {
        swVdebug(1,"置[%d]端口为DOWN失败", portid);
        return -1;
      }
      else
      {
        swVdebug(4,"置[%d]端口为DOWN成功", portid); 
        return 0;
      }
    }
  }

  return -1;	
}

/***************************************************************
 ** 函数名      : _sw_reset_port_status
 ** 功  能      : 坏线清零，将线路的状态重置为正常
 ** 作  者      : 
 ** 建立日期    : 2015/04/08
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : int portid:源发邮箱
 ** 返回值      : 0:成功-清零 / -1:失败-没有清零
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
      /* 将端口状态置为UP */
      ilRc = swPortset(portid, 1, 1);
      if (ilRc)
      {
        swVdebug(1,"置[%d]端口为UP失败", portid);
        return -1;
      }
      else
      {
        swVdebug(4,"置[%d]端口为UP成功", portid); 
        /* 将坏线次数和坏线开始时间清零 */
        smShmbuf_s.psSwt_sys_queue[j].fail_count = 0;
        smShmbuf_s.psSwt_sys_queue[j].fail_begin_time = 0;
        return 0;
      }
    }
  }

  return -1;	
}

/***************************************************************
 ** 函数名      : _sw_is_port_failure_timeout
 ** 功  能      : 坏线重试超时判断
 ** 作  者      : 
 ** 建立日期    : 2015/04/08
 ** 最后修改日期: 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : int portid:源发邮箱
 ** 返回值      : 0:成功-坏线状态超过设定值/ -1:失败-坏线状态未超过设定值
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
        swVdebug(4, "坏线的时间超过设定值");
        return  0;
      }
    }
  }

  return -1;	
}
