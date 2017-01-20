/****************************************************************/
/* ģ����    ��SWAPI                                          */
/* ģ������    ��ǰ�ó���API����                                */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ���Ż�                                           */
/* ��������    ��1999/11/22                                     */
/* ����޸����ڣ�2001/4/6                                       */
/* ģ����;    ��ǰ�ó������õĺ���                             */
/* ��ģ���а������º���������˵����                             */
/*			 ��1��int swNewtran();                  */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼��                                                   */
/****************************************************************/
/* UNIXϵͳ���� */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <varargs.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <libgen.h>
#include <time.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>

/*
#include "p_entry.h"
#include "p_return.h"
#include "p_symbol.h"
#include "p_typecl.h"
#include "p_msg.h"
*/

/* switch���� */
#include "swConstant.h"
#include "swapisource.h"

/* ���ݿⶨ�� */
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
EXEC SQL INCLUDE sqlca;
#ifdef DB_POSTGRESQL
EXEC SQL INCLUDE swDbstruct.h;
#endif
#ifdef DB_DB2
EXEC SQL INCLUDE '../inc/swDbstruct.h';
#endif
#ifdef DB_INFORMIX
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_ORACLE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#ifdef DB_SYBASE
EXEC SQL INCLUDE "swDbstruct.h";
#endif
#else
#include "swDbstruct.h"
#endif

#include "swShm.h"
#include "swPubfun.h"

extern short swPreunpack(struct msgpack *psMsgpack);

/* added by fzj at 2002.01.23, begin */
extern char agMacbuf[iMSGMAXLEN];	/* MAC buffer */
extern short igMacbuflen;		/* MAC length */
/* added by fzj at 2002.01.23, end */

/***********************************************************************
 ** ������:       int swNewtran(struct msghead *) �����½��װ�����
 ** ��  ��:       �����·����׵ı���ͷ�����ո����˿ڵ��������ɱ���ͷ 
 ** ��  ��:
 ** ��������:     2000.01.26
 ** ����޸�����: 2000.01.26
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������:     struct msghead *psMsghead: ָ����ͷ��ָ��
 ** ����ֵ:       �ɹ����� 0(SUCCESS), 
		  ʧ��(ȡ�ڲ�������ˮ��ʱ���Թ����ڴ�������ɹ�)����  -1(FAIL) 
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
int swNewtran(struct msghead *psMsghead)
{
  short ilRc;
  long ilTranid;

  if (swShmisfull()) return(-2);
  
  /* ���ɽ��ױ���ͷ */
  memset(psMsghead,0x00,sizeof(struct msghead));
  psMsghead->cRouter = '1';
  psMsghead->cFormatter = '0';
  psMsghead->iMsgtype = 1;
  psMsghead->iTranstep = 1;
  psMsghead->iMsgformat = sgPortattrib.iMsgformat;
  psMsghead->iOrg_q = sgPortattrib.iQid;
  psMsghead->iDes_q = sgPortattrib.iQid;
  psMsghead->iBegin_q_id = sgPortattrib.iQid;
  psMsghead->aSysattr[0]='0'; /* add by nh 20020910 */
  psMsghead->call_flag = '0';/*add by baiqj20150319 ,PSBC_V1.0*/
  psMsghead->callstart.time = 0;
  psMsghead->callstart.millitm = 0;
  memset(psMsghead->timeconsum,0x00,sizeof(psMsghead->timeconsum));/*add by baiqj20150319,PSBC_V1.0*/

  time(&(psMsghead->lBegintime));
  
  /* ���������ڴ棬�����µĽ�����ˮ�� */
  ilRc = swShmgettranid(&ilTranid);
  if(ilRc == FAIL) return(FAIL);
  psMsghead->lTranid = ilTranid;

  return(SUCCESS);
}

/***  �����������Ϊ����������� ***/

/***********************************************************************
 ** ������:       int swMbopen( short qid ) ����򿪺���
 ** ��  ��:       ��ָ������ 
 ** ��  ��:
 ** ��������:     2000.01.26
 ** ����޸�����: 2000.01.26
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������:     short int qid: ��Ҫ�򿪵Ķ˿������
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/

int swMbopen( short qid )
{
  short ilRc ;

  ilRc = qattach( qid );
  return( ilRc );
}

/***********************************************************************
 ** ������:       int swMbclose() ����رպ���
 ** ��  ��:       �ر����� 
 ** ��  ��:
 ** ��������:     2000.01.26
 ** ����޸�����: 2000.01.26
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������:     
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������ 
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/

int swMbclose( )
{
  short ilRc ;

  ilRc = qdetach( );
  return( ilRc );
}

/***********************************************************************
 ** ������:       int swSendpack(short, char *, short, unsigned int, short, short ) 
		  ���ķ��ͺ���
 ** ��  ��:       ������(������ƽ̨��׼����ͷ)������ָ��������� 
 ** ��  ��:
 ** ��������:     2000.01.26
 ** ����޸�����: 2000.01.26
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������:     short qid: ������������� 
		  char *msgpack: Ҫ���͵ı�������(������ƽ̨��׼����ͷ)
		  short priority: ���ȼ� 
		  short msglen: ���ĳ���  
		  short class: ���� Class 
		  short type: ���� Type 
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
/*modify by zcd 20141229
int swSendpack(short qid, char *msgpack, short msglen, short priority, 
	       short class, short type)*/
int swSendpack(short qid, char *msgpack, unsigned int msglen, short priority, 
	       short class, short type)
{
  short  ilRc;
  short ilOther_id1;
  short ilOther_grp1;
  short ilOther_q1;
 
  /* ���ͱ��� */
  swVdebug(4,"S0010: [��������] swSendpack()����");
  if (qid<1000)
  {
     /* �������� */
     ilRc= _qwrite2( msgpack, msglen, qid, priority, class, type );
  }
  else
  {
     ilOther_id1= qid;
     
    ilRc = swShmselect_swt_sys_other(ilOther_id1, &sgSwt_sys_other);
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(0,"S0020: [����/�����ڴ�] �ⲿ�ڵ�[%d]û������",ilOther_id1);
      return(FAIL);
    }
    ilOther_grp1 = sgSwt_sys_other.other_grp;
    ilOther_q1 = sgSwt_sys_other.other_q;
    
    ilRc= qwrite3(msgpack,msglen,ilOther_q1,ilOther_grp1,priority,class,type);
  }
  swVdebug(4,"S0030: [��������] swSendpack()����,������=%d",ilRc);
  return(ilRc);
}

/***********************************************************************
 ** ������:       int swRecvpackw �����������ܺ���
 ** ��  ��:       ��������ʽ�ӵ�ǰ������ܱ��� 
 ** ��  ��:
 ** ��������:     2000.01.26
 ** ����޸�����: 2000.01.26
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������: short *qid : ���ر��ķ��͵�Դ���� 
	      char *msgpack: Ҫ���͵ı�������(������ƽ̨��׼����ͷ)
	      short *msglen: �յ��ı��ĳ���
	      short *priority: �յ��ı������ȼ� 
	      short *class: ִ��ǰ��Ϊ���� Class����������ִ�к󷵻ر��� class
              short *type: ִ��ǰ��Ϊ���� class ����������ִ�к󷵻ر��� class 
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
/*modiby by zcd 20141229
int swRecvpackw( short *qid, char *msgpack, short *msglen, short *priority,
		 short *class, short *type)*/
int swRecvpackw( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 short *class, short *type)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id2;
  short ilOther_grp2;
  short ilOther_q2;
  
  swVdebug(4,"S0040: [��������] swRecvpackw()����");
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* ��������ʽ��ȡ���� */ 
  //swVdebug(0,"########msglen is:%d\n",*msglen);
  ilRc = qread3((char *)msgpack,msglen,qid,&ilOther_grp2,priority,class,type);
  //swVdebug(0,"############after msglen is:%d\n",*msglen);
  //swVdebug(0,"############after strlen is:%s\n",strlen(msgpack));
  if (ilRc) return( ilRc );
  if (ilOther_grp2 == ilLocal_grp)
  {
    /* �������䷢����¼ */
    swVdebug(4,"S0050: [��������] swRecvpackw()����,������=0");
    return(SUCCESS);
  }
  ilOther_q2 = *qid;
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp2, ilOther_q2, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0060: [����/�����ڴ�] �ⲿ�ڵ�[%d,%d]û������",ilOther_grp2,ilOther_q2);
    return(FAIL);
  }  
  ilOther_id2 = sgSwt_sys_other.other_id;
  
  /* ����Ų����ⲿ�ڵ�� */
  *qid = ilOther_id2;
  swVdebug(4,"S0070: [��������] swRecvpackw()����,������=0");
  return(SUCCESS);
}

/***********************************************************************
 ** ������:       int swRecvpack ���ķ��������ܺ���
 ** ��  ��:       �Է�������ʽ�ӵ�ǰ������ܱ��ļ��� 
 ** ��  ��:
 ** ��������:     2000.01.26
 ** ����޸�����: 2000.01.26
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������: short *qid : ���ر��ķ��͵�Դ���� 
	      char *msgpack: Ҫ���͵ı�������(������ƽ̨��׼����ͷ)
	      short *msglen: �յ��ı��ĳ���
	      short *priority: �յ��ı������ȼ� 
	      short *class: ִ��ǰ��Ϊ���� Class����������ִ�к󷵻ر��� class
              short *type: ִ��ǰ��Ϊ���� type ����������ִ�к󷵻ر��� type 
	      short timee: ���ܳ�ʱʱ�� 
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
/*modify by zcd 20141229
int swRecvpack( short *qid, char *msgpack, short *msglen, short *priority,
		 short *class, short *type, short time)*/
int swRecvpack( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 short *class, short *type, short time)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id;
  short ilOther_grp;
  short ilOther_q;
   
  swVdebug(4,"S0080: [��������] swRecvpackw()����"); 
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* �Է�������ʽ��ȡ���� */
  ilRc = qread3nw( msgpack, msglen, qid, &ilOther_grp,priority, class,  
		      type, time );
  if ( ilRc != SUCCESS )
  {
    swVdebug(4,"S0090: [��������] swRecvpack()����,������=0");
    return( ilRc );
  }
  if (ilOther_grp == ilLocal_grp)
  {
    /* �������䷢����¼ */
    swVdebug(4,"S0100: [��������] swRecvpack()����,������=0");
    return(SUCCESS);
  }
  ilOther_q = *qid;
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp, ilOther_q, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0110: [����/�����ڴ�] �ⲿ�ڵ�û������");
    return(FAIL);
  }  
  ilOther_id = sgSwt_sys_other.other_id;
    
  /* ����Ų����ⲿ�ڵ�� */
  *qid = ilOther_id;
  swVdebug(4,"S0120: [��������] swRecvpack()����,������=0");
  return(SUCCESS);
}

/***********************************************************************
 ** ������:       int swSendpackl(short, char *, short, short, long, long ) 
		  ���ķ��ͺ���
 ** ��  ��:       ������(������ƽ̨��׼����ͷ)������ָ��������� 
 ** ��  ��:
 ** ��������:     2001.11.1
 ** ����޸�����: 
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������:     short qid: ������������� 
		  char *msgpack: Ҫ���͵ı�������(������ƽ̨��׼����ͷ)
		  short priority: ���ȼ� 
		  short msglen: ���ĳ���  
		  class: ���� Class 
		  long type: ���� Type 
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
/*modify by zcd 20141229*/
int swSendpackl(short qid, char *msgpack, unsigned int msglen, short priority, 
	       long class, long type)
{
  short  ilRc;
  short ilOther_id1;
  short ilOther_grp1;
  short ilOther_q1;
   	
  /* ���ͱ��� */
  if (qid<1000)
  {
     /* �������� */
     ilRc = bmqPut(0,qid,priority,type,class,msgpack, msglen);
  }
  else
  {
     ilOther_id1= qid;
     
    ilRc = swShmselect_swt_sys_other(ilOther_id1, &sgSwt_sys_other);
    if(ilRc == SHMNOTFOUND)
    {
      swVdebug(0,"S0130: [����/�����ڴ�] �ⲿ�ڵ�[%d]û������",ilOther_id1);
      return(FAIL);
    }
    ilOther_grp1 = sgSwt_sys_other.other_grp;
    ilOther_q1 = sgSwt_sys_other.other_q;
    ilRc = bmqPut(ilOther_grp1,ilOther_q1,priority,type,class,msgpack,msglen);  
  }
  return(ilRc);
}


/***********************************************************************
 ** ������:       int swRecvpacklw �����������ܺ���
 ** ��  ��:       ��������ʽ�ӵ�ǰ������ܱ��� 
 ** ��  ��:
 ** ��������:     2000.11.1
 ** ����޸�����: 
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������: short *qid : ���ر��ķ��͵�Դ���� 
	      char *msgpack: Ҫ���͵ı�������(������ƽ̨��׼����ͷ)
	      short *msglen: �յ��ı��ĳ���
	      short *priority: �յ��ı������ȼ� 
	      long *class: ִ��ǰ��Ϊ���� Class����������ִ�к󷵻ر��� class
              long *type: ִ��ǰ��Ϊ���� class ����������ִ�к󷵻ر��� class 
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
/*modify by zcd 20141229

int swRecvpacklw( short *qid, char *msgpack, short *msglen, short *priority,
		 long *class, long *type)*/
int swRecvpacklw( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 long *class, long *type)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id2;
  short ilOther_grp2=0;
  short ilOther_q2;
  
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* ��������ʽ��ȡ���� */ 
  ilRc = bmqGetw(&ilOther_grp2,qid,priority,type,class,(char *)msgpack,msglen,0);
  if (ilRc) return( ilRc );
  if (ilOther_grp2 == ilLocal_grp)
  {
    /* �������䷢����¼ */
    return(SUCCESS);
  }
  ilOther_q2 = *qid;              
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp2, ilOther_q2, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0140: [����/�����ڴ�] �ⲿ�ڵ�[%d,%d]û������",ilOther_grp2,ilOther_q2);
    return(FAIL);
  }  
  ilOther_id2 = sgSwt_sys_other.other_id;
  
  /* ����Ų����ⲿ�ڵ�� */
   *qid = ilOther_id2; 
  return(SUCCESS);
}

/***********************************************************************
 ** ������:       int swRecvpackl ���ķ��������ܺ���
 ** ��  ��:       �Է�������ʽ�ӵ�ǰ������ܱ��ļ��� 
 ** ��  ��:
 ** ��������:     2001.11.1
 ** ����޸�����: 
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������: short *qid : ���ر��ķ��͵�Դ���� 
	      char *msgpack: Ҫ���͵ı�������(������ƽ̨��׼����ͷ)
	      short *msglen: �յ��ı��ĳ���
	      short *priority: �յ��ı������ȼ� 
	      long *class: ִ��ǰ��Ϊ���� Class����������ִ�к󷵻ر��� class
              long *type: ִ��ǰ��Ϊ���� type ����������ִ�к󷵻ر��� type 
	      short timee: ���ܳ�ʱʱ�� 
 ** ����ֵ:       �ɹ����� 0(SUCCESS), ʧ�ܷ��� MessageQ �ڲ������
*************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
/*modify by zcd 20141229
int swRecvpackl( short *qid, char *msgpack, short *msglen, short *priority,
		 long *class, long *type, short time)*/

int swRecvpackl( short *qid, char *msgpack, unsigned int *msglen, short *priority,
		 long *class, long *type, short time)
{
  short ilRc;
  short ilLocal_grp;
  short ilOther_id;
  short ilOther_grp;
  short ilOther_q;
    
  ilLocal_grp = atoi(getenv("BMQ_GROUP_ID"));
  /* �Է�������ʽ��ȡ���� */
  if (time == 0)
    ilRc = bmqGet(&ilOther_grp,qid,priority,type,class,(char *)msgpack,msglen);
  else
    ilRc = bmqGetw(&ilOther_grp,qid,priority,type,class,(char *)msgpack,msglen,time);

  if ( ilRc != SUCCESS ) return( ilRc );

  if (ilOther_grp == ilLocal_grp)
  {
    /* �������䷢����¼ */
    return(SUCCESS);
  }
  ilOther_q = *qid;
  
  ilRc = swShmselect_swt_sys_other_grp_q(ilOther_grp, ilOther_q, 
    &sgSwt_sys_other);
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0150: [����/�����ڴ�] �ⲿ�ڵ�û������");
    return(FAIL);
  }  
  ilOther_id = sgSwt_sys_other.other_id;
    
  /* ����Ų����ⲿ�ڵ�� */
  *qid = ilOther_id;
  return(SUCCESS);
}


/************************************************************************
 ** ������:       int swTranrev ( long	tran_id )
 ** ��  ��:       ֹͣ�����������裬���ͳ�������������������������
                  �Դ�����(iMBMONREV)�� �����������Ϊ���������ģ�����
                  ������Ϊֹͣ�������ģ������ͼ�ʱ�������ġ�
 ** ��  ��:       ·��ʤ
 ** ��������:     2000.01.27
 ** ����޸�����: 2000.01.27
 ** ������������: swMsgsend (), qwrite (), swMberror (), swDberror ()
 ** ȫ�ֱ���:     ��
 ** ��������:     tran_id -- ����ƽ̨�ڲ���ˮ��
 ** ����ֵ:       �ɹ����� 0 (SUCCESS)��ʧ�ܷ��� -1 (FAIL)
 ************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
int swTranrev(long tran_id)
{
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  llTranid;
    sqlint64  llTime;
    short ilTran_status;
    short ilTmp1;
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llTranid;
    long  llTime;
    short ilTran_status;
    short ilTmp1;
  EXEC SQL END DECLARE SECTION;
#endif
#else
  long  llTranid;
  long  llTime;
  short ilTran_status;
  short ilTmp1;
#endif

  int ilRc, ilMsglen = 0;
  struct msgpack slMsgpack;
  
  swVdebug(4,"S0160: [��������] swTranrev(%ld)����",tran_id);

  memset ((char*)&slMsgpack, 0, sizeof (struct msgpack));

  llTranid = tran_id;

  /* ����ˮ״̬,��SHM��DB�� */
  #ifdef DB_SUPPORT
  ilRc = swShmselect_swt_tran_log(llTranid, &sgSwt_tran_log,'a');
  #else
  ilRc = swShmselect_swt_tran_log(llTranid, &sgSwt_tran_log,'0'); /*2014-12-16*/
  #endif
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0170: [����/�����ڴ�] ���Ҵ˽�����ˮ[%ld]ʱ����",llTranid);
    return(FAIL);
  }
  
  if ( ilRc == SHMNOTFOUND ) 
  {
    #ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL SELECT tran_status INTO :ilTran_status:ilTmp1 FROM swt_his_tran_log
      WHERE tran_id = :llTranid;
    if (sqlca.sqlcode == SQLNOTFOUND)
    {
      swVdebug(0,"S0180: [����/���ݿ�] sqlcode=%ld",sqlca.sqlcode);
#ifdef TRANREVLOG
      time(&llTime);      
      EXEC SQL INSERT INTO swt_tranrev_log VALUES(:llTranid,:llTime);
      if(sqlca.sqlcode)
      {
        if (!(sqlca.sqlcode==SQLDUPKEY||sqlca.sqlcode==SQLDUPKEY2||sqlca.sqlcode==SQLDUPKEY3))
        { 
          swVdebug(0,"S0185: insert into swt_tran_rev_log failed, sqlcode=%d",sqlca.sqlcode);
        }  
      }  
#endif      
      return FAIL;
    }
    else if(sqlca.sqlcode)
    {
      swDberror ((char *)&slMsgpack);
      return FAIL;
    }
    else
    {
      if (ilTran_status == iTRNREVEND) 
      {
        swVdebug(4,"S0190: [��������] swTranrev()����,������=0");
        return(SUCCESS);
      }
    }
	#endif
  }
  else 
  {
    ilTran_status = sgSwt_tran_log.tran_status;
    
    if (ilTran_status == iTRNREVEND) 
    {
      swVdebug(4,"S0200: [��������] swTranrev()����,������=0");
      return(SUCCESS);
    }
  }

  swVdebug(2,"S0210: ilTran_status=%d",ilTran_status);

  slMsgpack.sMsghead.lTranid = tran_id;
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.aSysattr[0] = '0';
  ilMsglen = sizeof (struct msghead);

  switch ( ilTran_status ) {
    case iTRNING:
    case iTRNSAF:
    case iTRNOVERTIME:
    case iTRNREVFAIL:
    case iTRNEND:
      slMsgpack.sMsghead.lCode = 601;
      break;
    case iTRNREVING:
/* delete by szhengye 2000.3.20
      slMsgpack.sMsghead.lCode = 602;
delete by szhengye 2000.3.20 */
      swVdebug(4,"S0220: [��������] swTranrev()����,������=0");
      return SUCCESS;
      break;
    default:
      swVdebug(4,"S0230: [��������] swTranrev()����,������=0");
      return SUCCESS;
      break;
  }

  /* send command */
  ilRc = qwrite ((char *)&slMsgpack, ilMsglen, iMBMONREV);
  if(ilRc) {
    swMberror (ilRc, (char *)&slMsgpack);
    return FAIL;
  }
  swVdebug(4,"S0240: [��������] swTranrev()����,������=0");
  return SUCCESS;
}

/************************************************************************
 ** ������:       int swRevbykey(int iQid, char *aKey)
 ** ��  ��:       ֹͣ�����������裬���ͳ�������������������������
                  �Դ�����(iMBMONREV)�� �����������Ϊ���������ģ�����
                  ������Ϊֹͣ�������ģ������ͼ�ʱ�������ġ�
 ** ��  ��:       fzj
 ** ��������:     2000.01.27
 ** ����޸�����: 2000.01.27
 ** ������������: swMsgsend (), qwrite (), swMberror (), swDberror ()
 ** ȫ�ֱ���:     ��
 ** ��������:     tran_id -- ����ƽ̨�ڲ���ˮ��
 ** ����ֵ:       �ɹ����� 0 (SUCCESS)��ʧ�ܷ��� -1 (FAIL)
 ************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
int swRevbykey(int iQid, char *aKey)
{
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64 llTran_id
    short ilQid;
    char  alKey[151];
    short ilTran_stat;
    short ilTmp2;
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llTran_id;
    short ilQid;
    char  alKey[151];
    short ilTran_stat;
    short ilTmp2;
  EXEC SQL END DECLARE SECTION;
#endif
#else
  long  llTran_id;
  short ilQid;
  char  alKey[151];
  short ilTran_stat;
  short ilTmp2;
#endif
  int ilRc, ilMsglen = 0;
  struct msgpack slMsgpack;
  
  swVdebug(4,"S0250: [��������] swRevbykey(%d,%s)����",iQid,aKey);

  ilQid = iQid;
  memcpy(alKey,aKey,sizeof(alKey));
  memset ((char*)&slMsgpack, 0, sizeof (struct msgpack));

#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL SELECT tran_id,tran_status INTO :llTran_id:ilTmp2,:ilTran_stat:ilTmp2
    FROM swt_his_tran_log WHERE q_tran_begin = :ilQid AND rev_key = :alKey;
  if (sqlca.sqlcode == SQLNOTFOUND)
  {
    ilRc = swShmselect_qid_revkey_tranlog(ilQid,alKey,&sgSwt_tran_log);
    /*
    ilRc = swShmselect_swt_tran_log(llTran_id, &sgSwt_tran_log);
    */
    if (ilRc == FAIL)
      swVdebug(0,"S0260: [����/�����ڴ�] ���ҽ�����ˮ����");
    else if (ilRc == SQLNOTFOUND)
      swVdebug(2,"S0270: [����/�����ڴ�] �Ҳ���������ˮ");
    return FAIL;
  }
  else if(sqlca.sqlcode) 
  {
    swDberror ((char *)&slMsgpack);
    return FAIL;
  }
  else
  {
    if (ilTran_stat == iTRNREVEND) 
    {
      swVdebug(4,"S0280: [��������] swRevbykey()����,������=0");
      return(SUCCESS);
    }
  }
#endif

  swVdebug(2,"S0290: ilTran_id = %ld,ilTran_stat=%d",llTran_id,ilTran_stat);

  slMsgpack.sMsghead.lTranid = llTran_id;
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  ilMsglen = sizeof (struct msghead);

  switch ( ilTran_stat ) {
    case iTRNING:
    case iTRNSAF:
    case iTRNOVERTIME:
    case iTRNREVFAIL:
    case iTRNEND:
      slMsgpack.sMsghead.lCode = 601;
      break;
    case iTRNREVING:
      swVdebug(4,"S0300: [��������] swRevbykey()����,������=0");
      return SUCCESS;
      break;
    default:
      swVdebug(4,"S0310: [��������] swRevbykey()����,������=0");
      return SUCCESS;
      break;
  }

  /* send command */
  ilRc = qwrite ((char *)&slMsgpack, ilMsglen, iMBMONREV);
  if(ilRc) {
    swMberror (ilRc, (char *)&slMsgpack);
    return FAIL;
  }
  swVdebug(4,"S0320: [��������] swRevbykey()����,������=0");
  return SUCCESS;
}

/* #ifdef DB_SUPPORT */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */
/************************************************************************
 ** ������:       int swSaf ( char * msgpack )
 ** ��  ��:       �����ķ���SAF����SAF���ƽ��з��͡�Ŀ������Ϊ����ͷ.iDes_q
 ** ��  ��:       ʷ����
 ** ��������:     2000.02.27
 ** ����޸�����: 2000.02.27
 ** ������������: 
 ** ȫ�ֱ���:     ��
 ** ��������:     msgpack -- ��������
 ** ����ֵ:       �ɹ����� 0 (SUCCESS)��ʧ�ܷ��� -1 (FAIL)
 ************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
int swSaf(char * msgpack)
{
  short ilRc;
  long llSafid;
  struct msgpack * pslMsgpack;
  /* EXEC SQL BEGIN DECLARE SECTION; */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */
    short ilDes_q;                      /* ����� */
  /* EXEC SQL END DECLARE SECTION; */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */

  swVdebug(4,"S0330: [��������] swSaf()����");
  memset(&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
  pslMsgpack = (struct msgpack *)msgpack;
  ilDes_q = pslMsgpack->sMsghead.iDes_q;

  /* ������˿����� */
  memset((char *)&sgSwt_sys_queue,0x00,sizeof(struct swt_sys_queue));
  ilRc = swShmselect_swt_sys_queue(ilDes_q,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0340: [����/�����ڴ�] ������[%d]���ó���",ilDes_q);
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0350: [����/�����ڴ�] ����[%d]û������",ilDes_q);
    return(FAIL);
  }  
  
  if(swShmgetsafid(&llSafid) == FAIL)
  {
    swVdebug(0,"S0360: [����/�����ڴ�] ��ȡSAF��ˮ�ų���");
    return(FAIL);
  }  
  
  pslMsgpack->sMsghead.lSafid = llSafid;
  
  pslMsgpack->sMsghead.cSafflag = '1';

  sgSwt_rev_saf.saf_id = pslMsgpack->sMsghead.lSafid;
  sgSwt_rev_saf.tran_id = 0;
  sgSwt_rev_saf.proc_step = 0;
  time(&(sgSwt_rev_saf.saf_begin));
  sgSwt_rev_saf.saf_overtime = sgSwt_rev_saf.saf_begin + sgSwt_sys_queue.rev_overtime;
  sgSwt_rev_saf.rev_overtime = sgSwt_sys_queue.rev_overtime;
  if (sgSwt_sys_queue.rev_num > 0)
    sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num - 1;
  else
    sgSwt_rev_saf.saf_num = sgSwt_sys_queue.rev_num;
  
  sgSwt_rev_saf.saf_len = 
    sizeof(struct msghead) + pslMsgpack->sMsghead.iBodylen;
  if (swPutlog((char *)pslMsgpack,
    sgSwt_rev_saf.saf_len,&(sgSwt_rev_saf.saf_offset),
    sgSwt_rev_saf.saf_file) == -1)
  {
    swVdebug(0,"S0370: [����/��������] swPutlog()����,�汨�ĳ���");
    return(FAIL);
  }
  sgSwt_rev_saf.saf_status[0] = '0';
  sgSwt_rev_saf.saf_flag[0] = '1';

  /* ����һ��SAF��¼�����ݿ��� */
  /* del by gengling at 2015.04.02 one line PSBC_V1.0 */
  /* if ( (ilRc=swDbinsert_swt_rev_saf(sgSwt_rev_saf)) != SUCCESS) */ /*unsolved*/
  /* add by gengling at 2015.04.02 one line PSBC_V1.0 */
  if ( (ilRc=swShminsert_swt_rev_saf(sgSwt_rev_saf)) != SUCCESS)
  /* end of add */
  {
    swVdebug(0,"S0380: [����/�����ڴ�] ���빲���ڴ�SAF��¼����");
    return(FAIL);
  }

  /* �����ķ��͸�ǰ�ó��� */
  pslMsgpack->sMsghead.iOrg_q = ilDes_q;
  pslMsgpack->sMsghead.iDes_q = 0;
  /* lijunwen at 2000-4-18:  qwrite2 ��Ϊ _qwrite2 */
  ilRc=_qwrite2(msgpack,sizeof(struct msghead)
    + pslMsgpack->sMsghead.iBodylen, ilDes_q,1,0,0);
  if(ilRc)
  {
    swVdebug(0,"S0390: [����/��������] _qwrite2()����,����SAF����ʧ��,������=%d",ilRc);
    swMberror(ilRc,msgpack);
    return(FAIL);
  }
  swVdebug(4,"S0400: [��������] swSaf()����,������=0");
  return(SUCCESS);
}

/* #endif */ /* del by gengling at 2015.04.02 one line PSBC_V1.0 */

/*************************************************************************
 ** ������:       int swInit ( short qid )
 ** ��  ��:       ��ǰ�ý��̵Ļ������г�ʼ��, ����ǰ�����Ե����ã������ڴ�
                  �Ľ����ȵȡ�
 ** ��  ��:       ·��ʤ
 ** ��������:     2000.01.27
 ** ����޸�����: 2000.01.27
 ** ������������: swDbopen (), swDbclose ()
 ** ȫ�ֱ���:     ��
 ** ��������:     qid -- ǰ�ý��������
 ** ����ֵ:       �ɹ����� 0 (SUCCESS)��ʧ�ܷ��� -1 (FAIL)
 *************************************************************************
 ** �޸ļ�¼ ** 
**************************************************************************/
int swInit(short iQid)
{
  short ilRc;

  swVdebug(4,"S0410: [��������] swInit(%d)����",iQid);
  /* ��ʼ�������ڴ�ָ�� */
  if(swShmcheck() == FAIL)
  {
    swVdebug(0,"S0420: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    return(FAIL);
  }
  
  /* ������˿����� */
  memset((char *)&sgSwt_sys_queue,0x00,sizeof(struct swt_sys_queue));
  ilRc = swShmselect_swt_sys_queue(iQid,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0430: [����/�����ڴ�] ������[%d]���ó���",iQid);
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"S0440: [����/�����ڴ�] ����[%d]û������",iQid);
    return(FAIL);
  }  

  /* ��ʼ��sgPortattrib */
  sgPortattrib.iQid = iQid;
  sgPortattrib.iMsgformat = sgSwt_sys_queue.msg_format;
  swVdebug(4,"S0450: [��������] swInit()����,������=0");
  return SUCCESS;
}
/**************************************************************
 ** ������: swGetheadkey
 ** ��  ��: ȡ����ͷ�ؼ���
 ** ��  ��: ʷ����
 ** ��������: 2000.2.27
 ** ����޸�����: 2000.2.27
 ** ������������: swPreunpack()
 ** ȫ�ֱ���:
 ** ��������: iQid	���б��汨��ͷ������ǰ�������
 **           psMsgpack	����
 **           aHead_key	����ͷ�ؼ���
 ** ����ֵ��SUCCESS-�ɹ�/FAIL-ʧ��
***************************************************************/
int swGetheadkey(short iQid, struct msgpack *psMsgpack, char *aExpress, char *aHeadkey)
{
  /* delete by gxz 2001.10.17 
  int ilRc, ilLength; delete by gxz 2001.10.17 */

  int ilRc;
/*modify by zcd 20141229
 short ilLength;*/
  unsigned int ilLength;
  struct swt_sys_queue slSwt_sys_queue;
  /* add by gxz 2001.10.17 */
  short ilMsgformat;
  /* add by gxz 2001.10.17 */

  /* ��Ӧ�ö˿����� */
  swVdebug(4,"S0460: [��������] swGetheadkey()����");
  ilRc = swShmselect_swt_sys_queue(iQid, &slSwt_sys_queue);
  if (ilRc != SUCCESS)
  {
    swVdebug(0,"S0470: [����/�����ڴ�] ��Ӧ�ö˿�[%d]����ʧ��",iQid);
    return (FAIL);
  }
  /* Ԥ��� */
  /* add by gxz 2001.10.17 */
  ilMsgformat = psMsgpack->sMsghead.iMsgformat; 
  /* add by gxz 2001.10.17 */
  psMsgpack->sMsghead.iMsgformat = slSwt_sys_queue.msg_format;
  ilRc = swPreunpack(psMsgpack);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0480: [����/��������] swPreunpack(),Ԥ���ʧ��,������=%d",ilRc);
    _swMsgsend(302027, (char *)psMsgpack);
    return(FAIL);
  }
  /* ���㱨��ͷ�ؼ��� */
  ilRc = _swExpress(aExpress, aHeadkey, &ilLength);
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0490: [����/��������] _swExpress(),���㱨��ͷ�ؼ���%sʧ��,������=%d",aExpress,ilRc);
    _swMsgsend(302028, (char *)psMsgpack);
    return (FAIL);
  }
  aHeadkey[ilLength] = 0;
  psMsgpack->sMsghead.iMsgformat = ilMsgformat; 
  swVdebug(4,"S0500: [��������] swGetheadkey()����,������=0");
  return (SUCCESS);
}

/**************************************************************
 ** ������: swSavemsghead
 ** ��  ��: ���汨��ͷ
 ** ��  ��: ʷ����
 ** ��������: 2000.2.27
 ** ����޸�����: 2000.2.27
 ** ������������: swPutlog
 ** ȫ�ֱ���:
 ** ��������: iQid	���б��汨��ͷ������ǰ�������
 **           aHead_key	����ͷ�ؼ���
 **           psMsghead	����ͷ
 ** ����ֵ: SUCCESS-�ɹ�/FAIL-ʧ��
***************************************************************/
int swSavemsghead(short iQid,char * aHead_key,struct msghead *psMsghead)
{
  int ilRtncode;
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  tran_id;
    sqlint64  saf_id;
    sqlint64  head_offset;
    short q_id;
    char  head_key[51];
    short proc_step;
    char  head_file[11];
    short head_len;
  EXEC SQL END   DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  tran_id;
    long  saf_id;
    long  head_offset;
    short q_id;
    char  head_key[51];
    short proc_step;
    char  head_file[11];
    short head_len;
  EXEC SQL END   DECLARE SECTION;
#endif
#else
  long	tran_id;
  long	saf_id;
  long	head_offset;
  short q_id;
  char	head_key[51];
  short proc_step;
  char	head_file[11];
  short head_len;
#endif


  swVdebug(4,"S0510: [��������] swSavemsghead()����");
  memset(head_key,0x00,sizeof(head_key));
  memset(head_file,0x00,sizeof(head_file));
  ilRtncode = swPutlog((char *)psMsghead,sizeof(struct msghead),
    &head_offset,head_file);
  if (ilRtncode == FAIL) return(FAIL);

  q_id = iQid;
  strcpy(head_key, aHead_key);
  
  swVdebug(2,"S0520: ���汨��ͷ�ؼ��� =q_id[%d]key[%s]",q_id,head_key); 

  tran_id = psMsghead->lTranid;
  proc_step = psMsghead->iTranstep;
  saf_id = psMsghead->lSafid;
  head_len = sizeof(struct msghead);
  
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL COMMIT WORK;
  EXEC SQL INSERT INTO swt_msghead VALUES 
    (:q_id,
     :head_key,
     :tran_id,
     :proc_step,
     :saf_id,
     :head_file,
     :head_offset,
     :head_len);
#ifdef DB_INFORMIX
  if (sqlca.sqlcode == SQLDUPKEY || sqlca.sqlcode == SQLDUPKEY2 || sqlca.sqlcode == SQLDUPKEY3)
#else
  if (sqlca.sqlcode == SQLDUPKEY)
#endif
  {
    EXEC SQL DELETE FROM swt_msghead WHERE q_id = :q_id
      AND head_key = :head_key;
    if (sqlca.sqlcode)
    {
      swDberror(NULL);
      EXEC SQL ROLLBACK WORK;
      return(FAIL);
    }
    EXEC SQL INSERT INTO swt_msghead VALUES 
      (:q_id,
       :head_key,
       :tran_id,
       :proc_step,
       :saf_id,
       :head_file,
       :head_offset,
       :head_len);
  }
  if (sqlca.sqlcode) 
  {
    swDberror(NULL);
    EXEC SQL ROLLBACK WORK;
    return(FAIL);
  }
  EXEC SQL COMMIT WORK;
#endif
  swVdebug(4,"S0530: [��������] swSavemsghead()����,������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������: swLoadmsghead
 ** ��  ��: ȡ����ͷ
 ** ��  ��: ʷ����
 ** ��������: 2000.2.27
 ** ����޸�����: 2000.2.27
 ** ������������: swGetlog
 ** ȫ�ֱ���:
 ** ��������: iQid	���б��汨��ͷ������ǰ�������
 **           aHead_key	����ͷ�ؼ���
 **           psMsghead	����ͷ
 ** ����ֵ: SUCCESS-�ɹ�/FAIL-ʧ��/100-û���ҵ�
***************************************************************/
int swLoadmsghead(short iQid,char * aHead_key,struct msghead *psMsghead)
{
  int ilRtncode;
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  load_offset;
    short load_q;
    char  load_key[51];
    char  load_file[11];
    short load_len;
    short ilTmp3;
  EXEC SQL END   DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  load_offset;
    short load_q;
    char  load_key[51];
    char  load_file[11];
    short load_len;
    short ilTmp3;
  EXEC SQL END   DECLARE SECTION;
#endif
#else
  long  load_offset;
  short load_q;
  char  load_key[51];
  char  load_file[11];
  short load_len;
  short ilTmp3;
#endif
  
  swVdebug(4,"S0540: [��������] swLoadmsghead()����");
  load_q = iQid;
  memset(load_key,0x00,sizeof(load_key));
  memset(load_file,0x00,sizeof(load_file));
  strcpy(load_key, aHead_key);
  
  swVdebug(2,"S0550: ȡ������ͷ�ؼ��� = %s",load_key); /* 11 */
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
  EXEC SQL SELECT head_file,head_offset,head_len
    INTO :load_file:ilTmp3,
	 :load_offset:ilTmp3,
         :load_len:ilTmp3 
    FROM swt_msghead
    WHERE q_id = :load_q AND head_key =:load_key;
  if (sqlca.sqlcode == 100) 
    return(SQLNOTFOUND);
  else if (sqlca.sqlcode) 
  {
  	swVdebug(0,"����q_id[%d]head_key[%s]����ʧ��",load_q,load_key);
    swDberror(NULL);
    return(FAIL);
  }

  _swTrim(load_file);
  ilRtncode = swGetlog(load_file,load_len,load_offset,(char *)psMsghead);
  if (ilRtncode != SUCCESS)
  {
    swVdebug(0,"S0560: [����/��������] swGetlog()����,������=%d",ilRtncode);
    return(FAIL);
  }
#endif
  swVdebug(4,"S0570: [��������] swLoadmsghead()����,������=0");
  return(SUCCESS);
}

/**************************************************************
 ** ������: swBpcount
 ** ��  ��: ȡ�÷��𷽶˿����ڴ���Ľ��ױ���
 ** ��  ��: byz
 ** ��������: 2001.3.16
 ** ����޸�����: 2001.3.16
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: iQid      ǰ�������
 ** ����ֵ: SUCCESS-�ɹ� FAIL-ʧ��
***************************************************************/
/*
int swBpcount(short Qid)
{
  int i;
  
  for(i=0;i<iMAXPORTCOUNT;i++)
  {
    if (Qid==psgPortattrib[i].iQ_id ) 
    {
      return(psgPortattrib[i].lTrning);
    }
  }
  return(FAIL);
}
*/

/* ========= delete by qy 2001/08/22 ==========

int swBpcount(short iQid)
{
  int ilRc;

  [[* ������˿����� *]]
  memset((char *)&sgSwt_sys_queue,0x00,sizeof(struct swt_sys_queue));
  ilRc = swShmselect_swt_sys_queue(iQid,&sgSwt_sys_queue);
  if(ilRc == FAIL)
  {
    swVdebug(0,"������[%d]���ó���",iQid);
    return(FAIL);
  }
  if(ilRc == SHMNOTFOUND)
  {
    swVdebug(0,"����[%d]û������",iQid);
    return(FAIL);
  }  
  
  return(sgSwt_sys_queue.sPortattrib.lTrning);
}

============ end of delete by qy 2001/08/22 === */

/***************************************************************
 ** ������      : swSavetranhead
 ** ��  ��      : ���汨��ͷ
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
***************************************************************/
int swSavetranhead(short iQid,char *aMsghdkey,struct msghead *psMsghead)
{
  int ilRc;
  struct swt_tran_log slSwt_tran_log;
  
  ilRc = swShmselect_swt_tran_log(psMsghead->lTranid,&slSwt_tran_log,
           psMsghead->aSysattr[0]);
  if (ilRc) return(ilRc);
  if (slSwt_tran_log.q_target != iQid) return(-1);
  /*del by baiqj20150408 PSBC_V1.0  
  memcpy(slSwt_tran_log.msghdkey,aMsghdkey,iEXPRESSLEN);*/
  memcpy(slSwt_tran_log.msghdkey,aMsghdkey,sizeof(slSwt_tran_log.msghdkey)-1);
  memcpy(slSwt_tran_log.msghead,(char *)psMsghead,sizeof(struct msghead));
  ilRc = swShmupdate_swt_tran_log(psMsghead->lTranid,
           slSwt_tran_log,psMsghead->aSysattr[0]);
  return(ilRc);
}

/***************************************************************
 ** ������      : swLoadtranhead
 ** ��  ��      : ȡ����ͷ
 ** ��  ��      : 
 ** ��������    : 2001/05/26
 ** ����޸�����: 2001/05/26
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : 
 ** ����ֵ      : 0:�ɹ� / -1:ʧ��
***************************************************************/
int swLoadtranhead(short iQid,char *aMsghdkey,struct msghead *psMsghead)
{
  int ilRc;
  struct swt_tran_log slSwt_tran_log;
  
  ilRc = swShmselect_qid_msghdkey_tranlog(iQid,aMsghdkey,&slSwt_tran_log);
  if (ilRc == SUCCESS) 
    memcpy((char *)psMsghead,slSwt_tran_log.msghead,sizeof(struct msghead));
  return (ilRc);
}

/* added by fzj at 2002.01.23, begin */
/***************************************************************
 ** ������: swGetmacbuf
 ** ����: ȡMAC buffer
 ** ����:
 ** ��������: 2002/01/23
 ** ����޸�����: 2002/01/23
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: 0:�ɹ� / -1:ʧ��
***************************************************************/
int swGetmacbuf(char *aMacbuf, short *iMacbuflen)
{
  if (igMacbuflen <= 0) return(-1);
  memcpy(aMacbuf, agMacbuf, igMacbuflen);
  *iMacbuflen = igMacbuflen;
  return(0);
} 
/* added by fzj at 2002.01.23, end */

/* === begin of added by fzj at 2002.02.28 === */
/*****************************************************************************
 ** ������:       int swNewtran(struct msghead *) �����½��װ�����
 ** ��  ��:       �����·����׵ı���ͷ�����ո����˿ڵ��������ɱ���ͷ 
 ** ��  ��:
 ** ��������:     2000.01.26
 ** ����޸�����: 2000.01.26
 ** ������������:  
 ** ȫ�ֱ���:     
 ** ��������:     struct msghead *psMsghead: ָ����ͷ��ָ��
 ** ����ֵ:       �ɹ����� 0(SUCCESS), 
		  -1��ʧ��(ȡ�ڲ�������ˮ��ʱ���Թ����ڴ�������ɹ�)
		  -2��ƽ̨�����������
		  -3���˿ڷ�����������
		  -4�������˿ڴ�����������
		  -5�������˿�״̬down
                  -6: ������������ //add by gengling at 2015.03.18 one line PSBC_V1.0 
	          -7: û����SWTRAN_CTL.CFG ������
******************************************************************************
 ** �޸ļ�¼ ** 
*****************************************************************************/
int swNewtranhead(char *aTrancode, struct msghead *psMsghead)
{
  int ilRc;
  long ilTranid;

  memset(psMsghead,0x00,sizeof(struct msghead));

  if (swShmisfull()) return(-2);
  if (swShmtranisfull(sgPortattrib.iQid)) return(-3);
  /* add by gengling at 2015.03.18 one line PSBC_V1.0 */
  ilRc=swShmtranflowisfull(sgPortattrib.iQid, aTrancode);
  if (ilRc <0  ){
		if(ilRc == -2)
			return(-7);
		else
			return(-6);
	} 
	
  
#if 0 
  if (aTrancode != NULL)
  {
  	/*2009-9-9 10:58 del by pc ����������������ж�*/
    if ((ilRc = swShmportisdown(sgPortattrib.iQid,aTrancode))) 
    {
        swVdebug(0, "port is down [%d][%s]ret=[%d]", sgPortattrib.iQid, aTrancode, ilRc);
        return(ilRc);
    }
  }
#endif

  /* ���ɽ��ױ���ͷ */
  psMsghead->cRouter = '1';
  psMsghead->cFormatter = '0';
  psMsghead->iMsgtype = 1;
  psMsghead->iTranstep = 1;
  psMsghead->iMsgformat = sgPortattrib.iMsgformat;
  psMsghead->iOrg_q = sgPortattrib.iQid;
  psMsghead->iDes_q = sgPortattrib.iQid;
  psMsghead->iBegin_q_id = sgPortattrib.iQid;
  psMsghead->aSysattr[0]='0'; /* add by nh 20020910 */
  psMsghead->call_flag = '0';/*add by baiqj20150319,PSBC_V1.0*/
  psMsghead->callstart.time = 0;
  psMsghead->callstart.millitm = 0;
  memset(psMsghead->timeconsum,0x00,sizeof(psMsghead->timeconsum));/*add by baiqj20150319,PSBC_V1.0*/
  time(&(psMsghead->lBegintime));
  
  /* ���������ڴ棬�����µĽ�����ˮ�� */
  ilRc = swShmgettranid(&ilTranid);
  if(ilRc == FAIL) return(FAIL);
  psMsghead->lTranid = ilTranid;

  return(SUCCESS);
}
/* === end of added by fzj at 2002.02.28 === */
/************************************************************************
 ** ������:       int swTranrevStatus ( long	tran_id )
 ** ��  ��:       ֹͣ�����������裬���ͳ�������������������������
                  �Դ�����(iMBMONREV)�� �����������Ϊ���������ģ�����
                  ������Ϊֹͣ�������ģ������ͼ�ʱ�������ġ�
 ** ��  ��:       ·��ʤ
 ** ��������:     2000.01.27
 ** ����޸�����: 2000.01.27
 ** ������������: swMsgsend (), qwrite (), swMberror (), swDberror ()
 ** ȫ�ֱ���:     ��
 ** ��������:     tran_id -- ����ƽ̨�ڲ���ˮ��
 ** ����ֵ:       �ɹ����� 0 (SUCCESS)��ʧ�ܷ��� -1 (FAIL)
 ************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
int swTranrevStatus(long tran_id)
{
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
/* "ifdef" added by qh 20070420  ��Ҫ�����db2 64λ������.��long��Ϊsqlint64*/
#ifdef DB_DB2_64
  EXEC SQL BEGIN DECLARE SECTION;
    sqlint64  llTranid1;
    sqlint64  llTime1;
    short ilTran_status1;
    short ilTmp;
  EXEC SQL END DECLARE SECTION;
#endif
#ifdef DATABASE
  EXEC SQL BEGIN DECLARE SECTION;
    long  llTranid1;
    long  llTime1;
    short ilTran_status1;
    short ilTmp;
  EXEC SQL END DECLARE SECTION;
#endif
#else
  long  llTranid1;
  long  llTime1;
  short ilTran_status1;
  short ilTmp;
#endif
  int ilRc, ilMsglen = 0;
  struct msgpack slMsgpack;
  
  swVdebug(4,"S0160: [��������] swTranrev(%ld)����",tran_id);

  memset ((char*)&slMsgpack, 0, sizeof (struct msgpack));

  llTranid1 = tran_id;

  /* ����ˮ״̬,��SHM��DB�� */
  #ifdef DB_SUPPORT
  ilRc = swShmselect_swt_tran_log(llTranid1, &sgSwt_tran_log,'a');
  #else
  ilRc = swShmselect_swt_tran_log(llTranid1, &sgSwt_tran_log,'0'); 
  #endif
  if(ilRc == FAIL)
  {
    swVdebug(0,"S0170: [����/�����ڴ�] ���Ҵ˽�����ˮ[%ld]ʱ����",llTranid1);
    return(FAIL);
  }
  
  if ( ilRc == SHMNOTFOUND ) 
  {
#ifdef DB_SUPPORT  /*support database,Database Deprivation Project 2014-12*/
    EXEC SQL SELECT tran_status INTO :ilTran_status1:ilTmp FROM swt_his_tran_log
      WHERE tran_id = :llTranid1;
    if (sqlca.sqlcode == SQLNOTFOUND)
    {
      swVdebug(0,"S0180: [����/���ݿ�] sqlcode=%ld",sqlca.sqlcode);
#ifdef TRANREVLOG
      time(&llTime1);      
      EXEC SQL INSERT INTO swt_tranrev_log VALUES(:llTranid1,:llTime1);
      if(sqlca.sqlcode)
      {
        if (!(sqlca.sqlcode==SQLDUPKEY||sqlca.sqlcode==SQLDUPKEY2||sqlca.sqlcode==SQLDUPKEY3))
        { 
          swVdebug(0,"S0185: insert into swt_tran_rev_log failed, sqlcode=%d",sqlca.sqlcode);
        }  
      }  
#endif      
      return FAIL;
    }
    else if(sqlca.sqlcode)
    {
      swDberror ((char *)&slMsgpack);
      return FAIL;
    }
    else
    {
      if (ilTran_status1 == iTRNREVEND) 
      {
        swVdebug(4,"S0190: [��������] swTranrev()����,������=0");
        return(ilTran_status1);
      }
    }
#endif
  }
  else 
  {
    ilTran_status1 = sgSwt_tran_log.tran_status;
    
    if (ilTran_status1 == iTRNREVEND) 
    {
      swVdebug(4,"S0200: [��������] swTranrev()����,������=0");
      return(ilTran_status1);
    }
  }

  swVdebug(2,"S0210: ilTran_status1=%d",ilTran_status1);

  slMsgpack.sMsghead.lTranid = tran_id;
  slMsgpack.sMsghead.iMsgtype = iMSGORDER;
  slMsgpack.sMsghead.aSysattr[0] = '0';
  ilMsglen = sizeof (struct msghead);

  switch ( ilTran_status1 ) {
    case iTRNING:
    case iTRNSAF:
    case iTRNOVERTIME:
    case iTRNREVFAIL:
    case iTRNEND:
      slMsgpack.sMsghead.lCode = 601;
      break;
    case iTRNREVING:
/* delete by szhengye 2000.3.20
      slMsgpack.sMsghead.lCode = 602;
delete by szhengye 2000.3.20 */
      swVdebug(4,"S0220: [��������] swTranrev()����,������=0");
      return ilTran_status1;
      break;
    default:
      swVdebug(4,"S0230: [��������] swTranrev()����,������=0");
      return(FAIL);
      break;
  }

  /* send command */
  ilRc = qwrite ((char *)&slMsgpack, ilMsglen, iMBMONREV);
  if(ilRc) {
    swMberror (ilRc, (char *)&slMsgpack);
    return FAIL;
  }
  swVdebug(4,"S0240: [��������] swTranrev()����,������=0");
  return ilTran_status1;
}
