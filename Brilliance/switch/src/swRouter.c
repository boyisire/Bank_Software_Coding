/****************************************************************/
/* ģ����    ��ROUTER                                         */ 
/* ģ������    ��·�ɹ���                                       */
/* �� �� ��    ��V4.3.0                                         */
/* ��	 ��    ��ʷ����                                         */
/* ��������    ��1999/11/18                                     */
/* ����޸����ڣ�2001/4/3                                       */
/* ģ����;    �����ܻ�·��ģ��                                 */
/* ��ģ���а������º���������˵����                             */
/*			 ��1��int  main();                      */
/*			 ��2��void swQuit();                    */
/*			 ��3��int  swSaf_local();                     */
/****************************************************************/

/* switch���� */
#include "switch.h"

#include "swNdbstruct.h"

#include "swShm.h"

/* �������� */

/* ����ԭ�Ͷ��� */
void swQuit(int);
int swSaf_local(char * msgpack);

/**************************************************************
 ** ������      ��main
 ** ��  ��      �������� 
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999/11/18
 ** ����޸����ڣ�2000/4/7
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      ��
***************************************************************/
int main(int argc,char *argv[])
{
  short   j, k;             /* ��ʱ�������� */
  short   ilLine_maxid;        /* ·�ɽű����� */
  short   ilScriptexit;        /* ·�ɽű��˳���ʶ 0-���ɹ� 1-�ɹ� */
  short   ilScriptexec;        /* ·�ɽű�ִ�б�ʶ */
  short   ilResult;            /* ·�������жϽ�� 0-���ɹ� 1-�ɹ� */
  unsigned int    ilReadSize;          /* �����ĳ��� */   /*modified by dyw, 20150417, PSBC_V1.0*/
  short   ilOrgqid=0;          /* Դ������� */
  short   ilRtnCode;           /* ������ 0-�ɹ� ��0-���ɹ� */
  short   ilPriority;          /* �������:���ȼ� */
  short   ilClass;             /* �������:��� */
  short   ilType;              /* �������:���� */
  short   ilBegin_q_id;        /* ����ʼ������ */
  char    alTrancode[11];      /* ������ */
  short   ilResultlen;                             /* ���ʽ���������� */
  char    alResult[iFLDVALUELEN + 1];              /* ���ʽ������ */
  struct  msgpack  slMsgpack;                      /* ���Ķ��� */
  struct  swt_sys_route_m slSwt_sys_route_m;       /* ·���������ڴ�ָ�� */
  struct  swt_sys_route_d pslSwt_sys_route_d[101];  /* ·����ϸ�����ڴ�ָ�� */

  /* ��ӡ�汾�� */
  if (argc > 1)
    _swVersion("swRouter Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swRouter")) exit(FAIL);
 
  /* ���õ��Գ������� */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  /* strncpy(agDebugfile, "swRouter.debug", sizeof(agDebugfile)); delete by dyw, 2015.03.25, PSBC_V1.0*/


  /*==begin== add by cjh, 2015.03.25, PSBC_V1.0*/
/*
  strcpy(agDebugfile,"swSyslog.debug");
  memset(procName,0x00,sizeof(procName));
  strncpy(procName,"swRouter",sizeof(procName)-1);
*/
   swLogInit("swRouter", NULL); /*add by dyw, 2015.03.27, PSBC_V1.0*/
  /*==end== add by cjh, 2015.03.25, PSBC_V1.0*/

  /* ȡ��DEBUG��־ */
  if((cgDebug = _swDebugflag("swRouter"))==FAIL)
  {
    fprintf(stderr,"ȡ��DEBUG��־����!");
    exit(FAIL);
  } 

  /* ����SIGCLD��SIGINT��SIGQUIT��SIGHUP �ź� */
  signal( SIGTERM , swQuit );
  signal( SIGINT  , SIG_IGN );
  signal( SIGQUIT , SIG_IGN );
  signal( SIGTSTP , SIG_IGN );
  signal( SIGHUP  , SIG_IGN );
  swVdebug(2,"S0010: ����SIGCLD��SIGINT��SIGQUIT��SIGHUP �ź�");
  
   /* ��λ�������� */
  if(qattach(iMBROUTER)) 
  {
    swVdebug(0,"S0020: [����/����] qattach()����,������=-1,��ʼ���������!");
    exit(FAIL);
  }
  swVdebug(2,"S0030: ��λ��������ɹ�");
 
  /* ��ʼ�������ڴ� */
  ilRtnCode = swShmcheck();
  if (ilRtnCode != SUCCESS)
  {
    swVdebug(0,"S0040: [����/�����ڴ�] ��ʼ�������ڴ�ָ��ʧ��");
    swQuit(FAIL);
  }

  /* ��ϵͳ��ط�����Ϣ���� */
  _swMsgsend(103001,NULL);

  for( ; ; )
  {
    /* ��·��������뱨�� */ 
    ilReadSize=iMSGMAXLEN;
    ilPriority = 0;
    ilClass = 0;
    ilType = 0;
    if( ( ilRtnCode = qread2( (char *)&slMsgpack, &ilReadSize, &ilOrgqid,
      &ilPriority,&ilClass,&ilType)) != SUCCESS ) 
    {
      swVdebug(0,"S0050: [����/����] qrea2()����,������=%d,���������",ilRtnCode);
      swMberror(ilRtnCode, NULL);
      swQuit(FAIL);
    }
    /*=====begin====������־������־����, add by cjh, 2015.03.23, PSBC_V1.0=======*/
    cgDebug=slMsgpack.sMsghead.log_level;
    memset(agDebugfile,0x00,sizeof(agDebugfile));
    memcpy(agDebugfile,slMsgpack.sMsghead.file_log_name,sizeof(slMsgpack.sMsghead.file_log_name));
    /*======end=====������־������־����, add by cjh , 2015.03.23, PSBC_V1.0======*/

    psgMsghead = &(slMsgpack.sMsghead);  /* add by szhengye 2001.10.16 */
    /* memcpy((char *)&sgMsghead,&slMsgpack.sMsghead,sizeof(struct msghead));
    delete by szhengye 2001.10.16 */
    
    if ((ilOrgqid != iMBCTRL) && (ilOrgqid != iMBDBSQL) && 
        (ilOrgqid!=iMBFILE) &&(ilOrgqid!=iMBFILETRAN))
    {
      swVdebug(0,"S0060: [����/����] swRouterֻ�ܴ���swCtrl/swDb/swFile������·��[201]����!");
      continue;
    }

    swVdebug(2,"S0070: �յ�����,Դ������=[%d],Priority=%d,Class=%d, Type=%d", ilOrgqid, ilPriority,ilClass,ilType);
    if (cgDebug >= 2)
    {
      swDebugmsghead((char *)&slMsgpack);
      if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
      {
         swDebugfml((char *)&slMsgpack);
         swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
      }
      else 
         swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
    }

    /* ���¹����ڴ� */
    if((ilRtnCode=swShmcheck())!=SUCCESS)
    {
      swVdebug(0,"S0080: [����/�����ڴ�] ˢ���ڴ�ָ�����");
      swQuit(FAIL);
    }
    if ( slMsgpack.sMsghead.iMsgtype != iMSGROUTE) 
    {
      /* ��������(��·�ɱ���)��Ϣ��������̨��أ�303001) */
      swVdebug(0,"S0090: [����/����] �������Ͳ���ȷ!");
      _swMsgsend( 303001, NULL);
      continue;
    }
    swVdebug(2,"S0100: ��������Ϊ·�ɱ���");

    if ( slMsgpack.sMsghead.cRouter == cROUTENO )
    /* ����ͷ.·������ʽ Ϊ <ǿ�Ʋ�����·������> */
    {
      swVdebug(2,"S0110: ·������ʽΪǿ�Ʋ�����·������");
      /* �����ķ���������Դ������ */ 
      if( ( ilRtnCode = qwrite2((char *)&slMsgpack, slMsgpack.sMsghead.iBodylen
           + iMSGHEADLEN, iMBCTRL,ilPriority,ilClass,ilType )) != SUCCESS) 
      {
	 swVdebug(0,"S0120: [����/����] qwrite2()����,������=%d,����·�ɱ��ĵ�����[%d]����!",ilRtnCode,ilOrgqid);
         swMberror(ilRtnCode, NULL);
         swQuit(FAIL);
      }
      else
      {
        if (cgDebug >= 2)
        {
          swDebugmsghead((char *)&slMsgpack);
          if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
          {
            swDebugfml((char *)&slMsgpack);
            swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
          }
          else
            swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
        }
	swVdebug(2,"S0130: ����·�ɱ��ĵ�����[%d]�ɹ�!",ilOrgqid);
        continue;
      }
    }
    swVdebug(2,"S0140: ·������ʽΪ����·������");

    /* �жϱ��ĸ�ʽ�����Ƿ�Ϊ�ڲ���ʽ���� */ 
    if ( slMsgpack.sMsghead.iMsgformat != iFMTIMF )
    {
      /* ����ͷ.���ĸ�ʽ���� ��Ϊ <FML�ڲ����ĸ�ʽ> */
      swVdebug(0,"S0150: [����/����] ���ĸ�ʽ���Ͳ�ΪFML���ĸ�ʽ");
      _swMsgsend( 303004, NULL );
      continue;
    }
    /* ��������IMF����ֽ���FML����Ԥ��أ���Ԥ��IMF��ʽ�������� */
    ilRtnCode = swFmlunpack( slMsgpack.aMsgbody,
      slMsgpack.sMsghead.iBodylen, psgPreunpackbuf );
    if ( ilRtnCode == FAIL )
    {
      swVdebug(0,"S0160: [����/��������] swFmlunpack()����,������=%d, FML���Ľ������!",ilRtnCode);
      _swMsgsend( 399001, NULL ); 
      continue;
    }
    swVdebug(2,"S0170: FML���Ľ���ɹ�");
 
    /* �ӱ���ͷ��ȡ����ʼ���䡢������ */
    ilBegin_q_id = slMsgpack.sMsghead.iBegin_q_id;
    memset (alTrancode,0x00,sizeof(alTrancode));
    memcpy(alTrancode ,slMsgpack.sMsghead.aBegin_trancode,\
      sizeof(slMsgpack.sMsghead.aBegin_trancode)); 
    swVdebug(2,"S0180: �ӱ���ͷ��ȡ����ʼ����=[%d],������=[%s]",\
      ilBegin_q_id,alTrancode);
 
    /* ������ʼ���䡢��������ڴ�·�������н���ƥ��õ�·���� */      
    if ( (ilRtnCode = swShmselect_swt_sys_route_m(ilBegin_q_id,alTrancode,
      &slSwt_sys_route_m))!=SUCCESS)
    {
      swVdebug(0,"S0190: [����/�����ڴ�] �ӹ����ڴ�·������������·�ɽű������[%d]",ilRtnCode);
      continue;
    }
                  
    swVdebug(2,"S0200: ƥ��õ�·�ɽű���[%d]",slSwt_sys_route_m.route_grp);
    /* ����·������ڴ����ҵ�·�ɽű��� */
    if ( (ilRtnCode = swShmselect_swt_sys_route_d( slSwt_sys_route_m.route_grp,
       pslSwt_sys_route_d,&ilLine_maxid)) !=SUCCESS)
    {
      swVdebug(0,"S0210: [����/�����ڴ�] �ӹ����ڴ���ȡ·�ɽű�����[%d]",ilRtnCode);
      continue;
    }
    swVdebug(2,"S0220: �ӹ����ڴ���ȡ��·�ɽű�");

    /* ·�ɽű�ִ���˳���ʶ��Ϊ�� */
    ilScriptexit = FALSE;
    /* ·��ִ�б�ʶ��Ϊ�� */
    ilScriptexec = FALSE;

   /************ add by baiqj, 2015.03.19 ,PSBC_V1.0 *****/
   /*judge if msg is return for CALL, if it is CALL return, calculate the time consume
    *�жϱ����Ƿ���CALL�ķ��ر��ģ�����ǣ������call��ʱ
    */
    if (slMsgpack.sMsghead.call_flag != '0')
    {
	sw_calc_call_cost(&slMsgpack.sMsghead.callstart,
			slSwt_sys_route_m.route_grp,
			slMsgpack.sMsghead.iRouteline ,
			slMsgpack.sMsghead.timeconsum,
			sizeof(slMsgpack.sMsghead.timeconsum)
			);
        slMsgpack.sMsghead.call_flag = '0';
        slMsgpack.sMsghead.callstart.time = 0;
        slMsgpack.sMsghead.callstart.millitm = 0;
    }

    /* ���к�Ϊ�����ָ��·�ɽű����еĽű� */
    for ( j=0; j<ilLine_maxid; j++ )
    {
      /* �����кŴ��ڱ���ͷ.�ϴνű�ִ���кŵļ�¼ */
      if (pslSwt_sys_route_d[j].route_id <= slMsgpack.sMsghead.iRouteline) 
         continue;
      swVdebug(2,"S0230: ·�ɽű�ִ���к�[%d]",
	    pslSwt_sys_route_d[j].route_id);

      ilResult = FALSE;
      /* ·���ж�����Ϊ�� */
      if ( strlen( pslSwt_sys_route_d[j].route_cond ) == 0 )
        ilResult = TRUE;
      else
      {
        swVdebug(2,"S0240: ·���ж�����-%s",
           pslSwt_sys_route_d[j].route_cond);

	/* ��<·���жϺ���(·���ж�����)>�õ�·��������� */
        ilRtnCode=_swExpress(pslSwt_sys_route_d[j].route_cond,
          alResult,&ilResultlen );
        if (ilRtnCode == FAIL ) 
        {
          swVdebug(2,"S0250: ���ʽ[%s]�������",pslSwt_sys_route_d[j].route_cond);
          _swMsgsend( 303006, NULL ); 
	  ilScriptexec = TRUE;
          break; 
        }
	swVdebug(2,"S0260: ������[%s] = [%s]",
           pslSwt_sys_route_d[j].route_cond,alResult);
        if ( alResult[0] != '0' )
            ilResult = TRUE;
      }

      /*��·��������������������һ�ж�*/
      if (ilResult == FALSE )
      {
        swVdebug(2,"S0270: ��ǰ·������������,ִ���������");
        continue;
      }
      swVdebug(2,"S0280: ·����������,ִ�е�ǰ���");

      memset(alResult,0x00,iFLDVALUELEN + 1);
      slMsgpack.sMsghead.iRevmode = 0;

      /* SWITCH��·������ʶ��*/
      switch ( pslSwt_sys_route_d[j].oper_flag[0] )
      {
        case cROUTECALL:
          swVdebug(2,"S0290: ִ��CALL���");
          /* ·��ִ�б�ʶ = �� */
	  ilScriptexec = TRUE;
	  /* ����Ŀ��������ʽ����Ŀ������� */
          ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,
              alResult,&ilResultlen);
          if( ilRtnCode == FAIL )
          {
            swVdebug(0,"S0300: [����/��������] _swExpress()����,������=%d,Ŀ��������ʽ�����[%s]!",ilRtnCode,pslSwt_sys_route_d[j].q_target);
            _swMsgsend( 303006, NULL ); 
            ilScriptexit = TRUE;
               break; 
          }
          _swTrim(alResult);
	  swVdebug(2,"S0310: ִ��·�ɶ���CALL %s",alResult);

          slMsgpack.sMsghead.iDes_q = atoi(alResult);
          /* ���Ŀ�������Ƿ�Ϸ� */
          if( swCheckportid(slMsgpack.sMsghead.iDes_q) == FAIL )
          {
            swVdebug(0,"S0320: [����/��������] swCheckportid()����,������=-1, Ŀ�����䲻�Ϸ�,���������%s������",alResult);
            ilScriptexit = TRUE;
            break;
          }

          slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
          slMsgpack.sMsghead.iFmtgroup = pslSwt_sys_route_d[j].fmt_group;
          slMsgpack.sMsghead.iRevmode = pslSwt_sys_route_d[j].rev_mode;
          slMsgpack.sMsghead.iRevfmtgroup = pslSwt_sys_route_d[j].rev_fmtgrp;
	  
	  /*add by baiqj, 20150319,PSBC_V1.0, store the call start time to msghead*/
	  ftime( &(slMsgpack.sMsghead.callstart) );

	  slMsgpack.sMsghead.call_flag = '1';
      
          /* ���ͱ��ĵ�����Ŀ������ */
          if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
             slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
             iMBCTRL,ilPriority,ilClass,ilType )) != SUCCESS)
          {
            swVdebug(2,"S0330: ���ͱ�����Ŀ������[%d]����",iMBCTRL);
            swMberror(ilRtnCode, "����:qwrite��Ŀ���������!");
            swQuit(FAIL);
          }
          swVdebug(2,"S0340: ���ͱ�����Ŀ������[%d]�ɹ�",iMBCTRL);
          if (cgDebug >= 2)
          {
            swDebugmsghead((char *)&slMsgpack);
            if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
	    {
              swDebugfml((char *)&slMsgpack);
              swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
            else
              swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
          }

          /* ·�ɽű�ִ���˳���ʶ��Ϊ�� */
          ilScriptexit = TRUE;
          break;

          case cROUTESEND:
            swVdebug(2,"S0350: ִ��SEND���");
            /* ����Ŀ��������ʽ����Ŀ������� */
            ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,
                alResult,&ilResultlen);
            if( ilRtnCode == FAIL )
            {
              swVdebug(2,"S0360: Ŀ��������ʽ�������[%s]!",
                pslSwt_sys_route_d[j].q_target);
              _swMsgsend( 303006, NULL ); 
              ilScriptexit = TRUE;
              break; 
            }
            _swTrim(alResult);
	    swVdebug(2,"S0370: ִ��·�ɶ��� SEND %s",alResult);

            slMsgpack.sMsghead.iDes_q = atoi(alResult);
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
            slMsgpack.sMsghead.iFmtgroup = pslSwt_sys_route_d[j].fmt_group;
            slMsgpack.sMsghead.iRevmode = pslSwt_sys_route_d[j].rev_mode;
            slMsgpack.sMsghead.iRevfmtgroup = pslSwt_sys_route_d[j].rev_fmtgrp;

            if (pslSwt_sys_route_d[j].end_flag[0] == '1')
            {
              /* ��ǰ��¼END�����ʶΪ�� */
              slMsgpack.sMsghead.cEndflag = cTRUE;
	      /* ·��ִ�б�ʶ = �� */
	      ilScriptexec = TRUE;
	      /* ·��ִ�нű��˳���ʶ��Ϊ�� */
	      ilScriptexit = TRUE;
            }

            /* ����ͷ.SAF�����ʶ=��ǰ��¼SAF�����ʶ */
            slMsgpack.sMsghead.cSafflag = pslSwt_sys_route_d[j].saf_flag[0];
            if (slMsgpack.sMsghead.cSafflag == cTRUE)
	    {
              swVdebug(2,"S0380: ����SAF����");
	      ilRtnCode=swSaf_local((char *)&slMsgpack);
              if(ilRtnCode)
              {
                swVdebug(0,"S0390: [����/��������] swSaf_local()����,������=%d,����SAFʧ��",ilRtnCode);
                ilScriptexit = TRUE;
                break;
              }
            }
	    else
	    {
              if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
                slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
                iMBCTRL,ilPriority,ilClass,ilType )) != SUCCESS)
              {
                swVdebug(2,"S0400: ���ͱ�����Ŀ������[%d]����",iMBCTRL);
                swMberror(ilRtnCode, "����:qwrite��Ŀ���������!");
                swQuit(FAIL);
              }
              swVdebug(2,"S0410: ���ͱ�����Ŀ������[%d]�ɹ�",iMBCTRL);
              if (cgDebug >= 2)
              {
                swDebugmsghead((char *)&slMsgpack);
                if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
		{
                  swDebugfml((char *)&slMsgpack);
                  swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
                }
                else
                  swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }

            }
            slMsgpack.sMsghead.cSafflag = cFALSE;
            /* ����ͷ.���ײ���=����ͷ.���ײ���+1 */
            slMsgpack.sMsghead.iTranstep = slMsgpack.sMsghead.iTranstep + 1;
            break;

	  case cROLLBACK:
            swVdebug(2,"S0420: ִ��ROLLBACK���");
            /* ·��ִ�б�ʶ = �� */
            ilScriptexec = TRUE; 
            /* ·��ִ�нű��˳���ʶ��Ϊ�� */
	    ilScriptexit = TRUE;
	    /* ����ʱ������Ϣ����(��Ϣ����103003)����̨��� */
	    _swMsgsend(103003,(char *)&slMsgpack);
	    /* �����������(��Ϣ����601)���������ģ�� */
	    slMsgpack.sMsghead.iMsgtype = iMSGORDER;
	    slMsgpack.sMsghead.lCode = 601;

	    ilRtnCode = qwrite ((char *)&slMsgpack,
              sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen,iMBMONREV);
            if (ilRtnCode != SUCCESS)
	    {
              swVdebug(0,"S0430: [����/����] qwrite()����,������=%d,���ͳ����������������������",ilRtnCode);
              swMberror(ilRtnCode, (char *)&slMsgpack);
              swQuit(FAIL);
            }
            swVdebug(2,"S0440:���ͳ�������������������ɹ�");
            if (cgDebug >= 2)
            {
              swDebugmsghead((char *)&slMsgpack);
              if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
              {
                swDebugfml((char *)&slMsgpack);
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }
              else
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
	    break;
          case cGOTO:
	    /* ������ǰ·�ɽű��ε���������Բ���GOTO��Ŀ������� */
	    swVdebug(2,"S0450: ·�ɶ���: GOTO %d",
        	pslSwt_sys_route_d[j].next_id);
            for( k=0; k<ilLine_maxid ; k++ )
            if(pslSwt_sys_route_d[k].route_id == pslSwt_sys_route_d[j].next_id)
                break;
            if(pslSwt_sys_route_d[k].route_id != pslSwt_sys_route_d[j].next_id)
            {
	      swVdebug(2,"S0460: GOTO������");
              /* ����GOTO��������Ϣ���ģ�303005�� */
              _swMsgsend(303005,(char *)&slMsgpack);
	      /* ·��ִ�нű��˳���ʶ��Ϊ�� */
              ilScriptexit = TRUE;
              break;
            }
            j = k-1;
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[k].route_id - 1;
            break;

          case cFMLSET:
	    swVdebug(2,"S0470: ·�ɶ��� SET %s = %s",
               pslSwt_sys_route_d[j].fml_name, pslSwt_sys_route_d[j].q_target);
	    /* ���ݱ��ʽ����FMLSET��ֵ */
            ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,
               alResult,&ilResultlen);
            if( ilRtnCode == FAIL )
            {
              swVdebug(2,"S0480: ������ʽ[%s]����",pslSwt_sys_route_d[j].q_target);
              _swMsgsend( 303006, NULL ); 
              ilScriptexit = TRUE;
              break; 
            }
	    swVdebug(2,"S0490: ������ %s = %s",
               pslSwt_sys_route_d[j].fml_name,alResult);
            ilRtnCode = swFmlset(pslSwt_sys_route_d[j].fml_name,
               ilResultlen,alResult,psgPreunpackbuf);
            if (ilRtnCode != SUCCESS)
            {
              swVdebug(0,"S0500: [����/��������] swFmlset()����,������=%d,����[%s]swFmlset����!",ilRtnCode,pslSwt_sys_route_d[j].fml_name);
              ilScriptexit = TRUE;
              break;
            }

            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;

            ilRtnCode = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,
               &(slMsgpack.sMsghead.iBodylen));
            if ( ilRtnCode == FAIL )
            {
              swVdebug(0,"S0510: [����/��������] swFmlpack()����,������=%d,����FML����ʧ��",ilRtnCode);
              _swMsgsend( 302016, NULL );
              ilScriptexit = TRUE;
              break;
            }
            break;
          case cEXECSQL:
	    ilScriptexec = TRUE;
            swVdebug(2,"S0520: ���ݿ���� %s",
               pslSwt_sys_route_d[j].q_target);
            ilRtnCode = swFmlset("_SQLTEXT",
               strlen(pslSwt_sys_route_d[j].q_target),
               pslSwt_sys_route_d[j].q_target,psgPreunpackbuf);
            if (ilRtnCode)
            {
              swVdebug(0,"S0530: [����/��������] swFmlset()����,������=%d,��[_SQLTEXT]��ʧ��",ilRtnCode);
              ilScriptexit = TRUE;
              break;
            }

            ilRtnCode = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,
               &(slMsgpack.sMsghead.iBodylen));
            if ( ilRtnCode == FAIL )
            {
              swVdebug(0,"S0540: [����/��������] swFmlpack()����,������=%d,����FML����ʧ��",ilRtnCode);
              _swMsgsend( 302016, NULL );
              ilScriptexit = TRUE;

              break;
            }

	    /* ����ͷ.��ǰ·���ж�˳���=�ü�¼·���ж�˳��� */
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
            ilType = pslSwt_sys_route_d[j].next_id;
            if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
               slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
               iMBDBSQL,ilPriority,ilClass,ilType )) != SUCCESS)
            {
              swVdebug(2,"S0550: ���ͱ��������ݿ�����������");
              swMberror(ilRtnCode, "����:qwrite�����ݿ�����������!");
              swQuit(FAIL);
            }
            swVdebug(2,"S0560: ���ͱ��������ݿ��������ɹ�");
            if (cgDebug >= 2)
            {
              swDebugmsghead((char *)&slMsgpack);
              if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
              {
                swDebugfml((char *)&slMsgpack);
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }
              else
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }
            ilScriptexit = TRUE;
            break;
          case cFTPPUT:  
          case cFTPGET:
          case cQPUT:
          case cQGET:  
            swVdebug(2,"S0570: �ļ�������� ");
	    ilScriptexec = TRUE;
            ilRtnCode = swFmlset("_FILEMODE",
               strlen(pslSwt_sys_route_d[j].oper_flag),
               pslSwt_sys_route_d[j].oper_flag,psgPreunpackbuf);
            if (ilRtnCode)
            {
              swVdebug(0,"S0580: [����/��������] swFmlset()����,������=%d,��[_FILEMODE]��ʧ��",ilRtnCode);
              ilScriptexit = TRUE;
              break;
            }

            ilRtnCode = swFmlset("_FILEPARM",
                strlen(pslSwt_sys_route_d[j].q_target),
                pslSwt_sys_route_d[j].q_target,psgPreunpackbuf);
            if (ilRtnCode)
            {
              swVdebug(0,"S0590: [����/��������] swFmlset()����,������=%d,��[_FILEPARM]��ʧ��",ilRtnCode);
              ilScriptexit = TRUE;
              break;
            }

            ilRtnCode = swFmlpack(psgPreunpackbuf,slMsgpack.aMsgbody,
               &(slMsgpack.sMsghead.iBodylen));
            if ( ilRtnCode == FAIL )
            {
              swVdebug(0,"S0600: [����/��������] swFmlpack()����,������=%d,����FML����ʧ��",ilRtnCode);
              _swMsgsend( 302016, NULL );
              ilScriptexit = TRUE;
              break;
            }

	    /* ����ͷ.��ǰ·���ж�˳���=�ü�¼·���ж�˳��� */
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;

            if( ( ilRtnCode = qwrite2( (char *)&slMsgpack,  
               slMsgpack.sMsghead.iBodylen+iMSGHEADLEN,
               iMBFILE,ilPriority,ilClass,ilType )) != SUCCESS)
            {
              swVdebug(2,"S0610: ���ͱ������ļ������������");
              swMberror(ilRtnCode, "����:qwrite���ļ������������!");
              swQuit(FAIL);
            }
            swVdebug(2,"S0620: ���ͱ������ļ���������ɹ�");
            if (cgDebug >= 2)
            {
              swDebugmsghead((char *)&slMsgpack);
              if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
	      {
                swDebugfml((char *)&slMsgpack);
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
              }
              else
                swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
            }

            ilScriptexit = TRUE;
            break;

          /*  add by zjj 2001.11.06  */
          case cDO: 
            swVdebug(2,"S0630: ִ�� DO ����!");
            ilScriptexec = TRUE;
            /* ִ�б��ʽ,�������˳�,����ִ����һ�� */
            ilRtnCode = _swExpress(pslSwt_sys_route_d[j].q_target,\
              alResult,&ilResultlen);
            if (ilRtnCode == FAIL)
            {
              swVdebug(0,"S0640: [����/��������] ִ��DO����[%s]ʧ��!",\
                pslSwt_sys_route_d[j].q_target);
              _swMsgsend( 303006, NULL ); 
	      ilScriptexit = TRUE;
              break; 
            }
            swVdebug(2,"S0650: ִ�� DO �������OK!");
            /* ����ͷ.��ǰ·���ж�˳���=�ü�¼·���ж�˳��� */
            slMsgpack.sMsghead.iRouteline = pslSwt_sys_route_d[j].route_id;
            break;
          /* end add by zjj 2001.11.06 */

          default:
            swVdebug(0,"S0660: [����/����] ·�ɽű���ʶ����[%d]", \
              pslSwt_sys_route_d[j].oper_flag[0]);
            ilScriptexit = TRUE;
            break;
       }
       /* ·��ִ�нű��˳���ʶ��Ϊ�� */
       if (ilScriptexit)  break; 
       swVdebug(2,"S0670: ����ִ��·�ɽű�");
     }

     if ( ilScriptexec == FALSE )
     /* ·��ִ�б�ʶΪ�� */
     {
       /* 303003:û��·���ж��������� */
       swVdebug(0,"S0680: [����/����] û��·���ж���������");
       _swMsgsend( 303003, NULL);
     }
     swVdebug(2,"S0690: ִ��·�ɽű��ɹ�");
  }
}


/**************************************************************
 ** ������      ��swQuit
 ** ��  ��      ���˳����ͷ������Դ 
 ** ��  ��      ���ⴺ�� 
 ** ��������    ��1999/11/22
 ** ����޸����ڣ�1999.11.22
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��
 ** ����ֵ      �� SUCCESS, FAIL
***************************************************************/
void swQuit(int sig)
{
  short  ilRtnCode;                 /* ������ 0-�ɹ� ��0-���ɹ� */
  signal(SIGTERM,SIG_IGN);
  swVdebug(2,"S0700: [swRouter]�Ѿ�ֹͣ!");
  qdetach();
  qattach(iMBROUTER);

  /* ��ϵͳ��ط�����Ϣ���� */
  ilRtnCode=_swMsgsend(103002,NULL);
  qdetach();
  if(ilRtnCode!=0)
    exit(ilRtnCode);
  else
    exit(sig);
}

/************************************************************************
 ** ������:       swSaf_local
 ** ��  ��:       �����ķ���SAF����SAF���ƽ��з���
 ** ��  ��:       ʷ����
 ** ��������:     2000.02.27
 ** ����޸�����: 2000.02.27
 ** ������������: 
 ** ȫ�ֱ���:     ��
 ** ��������:     msgpack -- ��������
 ** ����ֵ:       �ɹ����� SUCCESS��ʧ�ܷ��� FAIL
 ************************************************************************
 ** �޸ļ�¼ ** 
*************************************************************************/
int swSaf_local(char * msgpack)
{
  short ilRtnCode;                    /* ������ */
  struct msgpack *pslMsgpack;    
  struct msgpack slMsgpack;          /* ���Ķ��� add by szhengye 2001.10.10 */
  short ilDes_q;                      /* Ŀ������� */
  short ilRev_overtime,ilRev_num;     /* ������ʱʱ�䡢�������� */
  struct swt_sys_queue slSwt_sys_queue;  
  long  llSafid;

  memset(&sgSwt_rev_saf,0x00,sizeof(struct swt_rev_saf));
  pslMsgpack = (struct msgpack *)msgpack; 
  memcpy((char *)&slMsgpack,msgpack,pslMsgpack->sMsghead.iBodylen + sizeof(struct msghead));
  ilDes_q = pslMsgpack->sMsghead.iDes_q;

  /* �ӹ����ڴ����ҳ�Ŀ���������� */
  if ( (ilRtnCode=swShmselect_swt_sys_queue(ilDes_q,&slSwt_sys_queue))!=SUCCESS)
  {
    swVdebug(2,"S0710: �����ڴ���δ�ҵ�Ŀ����������[%d][%d]",
      ilRtnCode,ilDes_q);
    return(FAIL);
  }

  /* ȡĿ������ĳ���ʱ��ͳ������� */ 
  ilRev_overtime=slSwt_sys_queue.rev_overtime;
  ilRev_num=slSwt_sys_queue.rev_num;     

  /* �ӹ����ڴ���ȡ��SAF��ˮ�� */
  if ( (ilRtnCode=swShmgetsafid(&llSafid))!=SUCCESS)
  {
    swVdebug(2,"S0720: ���������ڴ�SAF��ˮ�ų���");
    return(FAIL);
  }
  swVdebug(2,"S0730: ���������ڴ�����µ�SAF��ˮ��");

  slMsgpack.sMsghead.lSafid = llSafid;
  slMsgpack.sMsghead.cSafflag = '1';
  slMsgpack.sMsghead.iMsgtype = iMSGAPP;

  /* ��֯һ��SAF��¼ */
  sgSwt_rev_saf.saf_id = slMsgpack.sMsghead.lSafid;
  sgSwt_rev_saf.tran_id = 0;
  sgSwt_rev_saf.proc_step = 0;
  time(&(sgSwt_rev_saf.saf_begin));
  sgSwt_rev_saf.saf_overtime = sgSwt_rev_saf.saf_begin + ilRev_overtime;
  sgSwt_rev_saf.rev_overtime = ilRev_overtime;
  if (ilRev_num > 0)
    sgSwt_rev_saf.saf_num = ilRev_num - 1;
  else
    sgSwt_rev_saf.saf_num = ilRev_num;
  sgSwt_rev_saf.saf_len = 
    sizeof(struct msghead) + slMsgpack.sMsghead.iBodylen;

  /* ������д��LOG�ļ� */
  if (swPutlog((char *)&slMsgpack,
    sgSwt_rev_saf.saf_len,&(sgSwt_rev_saf.saf_offset),
    sgSwt_rev_saf.saf_file) == -1)
  {
    swVdebug(0,"S0740: [����/��������] swPutlog()����,������=-1,�汨�ĳ���");
    return(FAIL);
  }
  sgSwt_rev_saf.saf_status[0] = '0';
  sgSwt_rev_saf.saf_flag[0] = '1';

/* #ifdef DB_SUPPORT */ /* del by gengling at 2015.04.03 PSBC_V1.0 */
  /* ADDFLAG */
  /* ����һ��SAF��¼�����ݿ��� modify shm->db by xujun 2002.09.13 */
  /* del by gengling at 2015.04.03 one line PSBC_V1.0 */
  /* if ( (ilRtnCode=swDbinsert_swt_rev_saf(sgSwt_rev_saf)) !=SUCCESS) */
  /* add by gengling at 2015.04.03 one line PSBC_V1.0 */
  if ( (ilRtnCode=swShminsert_swt_rev_saf(sgSwt_rev_saf)) != SUCCESS)
  {
    swVdebug(2,"S0750: ����SAF��¼����,[saf_id=%ld,tran_id=%ld]",
             sgSwt_rev_saf.saf_id,sgSwt_rev_saf.tran_id);
    return(FAIL);
  }
  swVdebug(2,"S0760: �����ڴ����һ��SAF��¼ ");
/* #endif */ /* del by gengling at 2015.04.03 PSBC_V1.0 */
  slMsgpack.sMsghead.iOrg_q = ilDes_q;
  slMsgpack.sMsghead.iDes_q = 0;

  /* �����ķ��͸�ǰ�ó��� */
  ilRtnCode=qwrite2((char *)&slMsgpack,sizeof(struct msghead)
    + slMsgpack.sMsghead.iBodylen, ilDes_q,1,0,0);
  if(ilRtnCode)
  {
    swVdebug(0,"S0770: [����/����] qwrite2()����,������=%d,����SAF����ʧ��!",ilRtnCode);
    swMberror(ilRtnCode,msgpack);
    swQuit(FAIL);
  }
  if (cgDebug >= 2)
  {
    swDebugmsghead((char *)&slMsgpack);
    if (slMsgpack.sMsghead.iMsgformat == iFMTIMF)
    {
      swDebugfml((char *)&slMsgpack);
      swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
    } 
    else
      swDebughex((char *)slMsgpack.aMsgbody,slMsgpack.sMsghead.iBodylen);
  }

  return(SUCCESS);
}

/****************************************************************************************
 * 		calculate the time consume by CALL in route
 * ����call��ʼʱ���뵱ǰʱ���ʱ����������ʱ�������������consume��
 * consumeͬʱ�����˱���call֮ǰ�����׵�����call�ĺ�ʱ
 * ����ĸ�ʽΪ:
 * ·�ɺ�:��һ��CALL���ڵ��к�,��ʱ|�ڶ���CALL���к�,��ʱ|....|��N��CALL���к�,��ʱ
 *****************************************************************************************/
int  sw_calc_call_cost(struct timeb*  start_time, int route_id , int route_line, char *consume, int size)
{
	struct timeb  ret_time;
	time_t sec;
	long  interval;
	int i = 0;
	ftime(&ret_time);

    interval  = (ret_time.time-start_time->time)*1000L + (ret_time.millitm-start_time->millitm);
	
    for(;  (i<size) && (consume[i]!='\0'); i++) ;
	
	if( (size - i) <= 2 ) 
	{
		printf("size=%d\n", size);
		return -2;
	}
	
	if( i != 0 )
	{
		consume[i++] = '|';
	}
	else
	{
		snprintf(consume,(size) ,"%d:",route_id);
		i += strlen(consume);
	}
	
	if(interval > 1000*30*3) /*����3���ӣ���call��ʱ����*/
	{
		snprintf(consume + i, (size-i), "%d,MAX", route_line, (interval/1000));
		return 0;
        }
	
	if(interval > 1000) 
	{
		snprintf(consume + i, (size-i), "%d,%.3fs", route_line, (interval/1000));
	}
	else
	{
		snprintf(consume + i, (size -i) , "%d,%d",route_line, interval);
	}

	return 0;
}


/****************************************************************************************
 *���msg�����һ��call�ĺ�ʱ��ʱ�侫ȷ�����룬��msghead��timeconsum��ֵ��ֻ��Ҫ�����
 * ���룺msghd    ����ͷ
 * ���:
 * time����ʱ     routid ��·�����   lineid��call�����к�  
 *timeconsum �ĸ�ʽ��·�ɺ�:��һ��CALL���ڵ��к�,��ʱ|�ڶ���CALL���к�,��ʱ|....|��N��CALL���к�,��ʱ
 *add by baiqj20150323,PSBC_V1.0
 *****************************************************************************************/

//int sw_get_calltime_consum(char* timeconsum,int *time,int * routid,int *lineid)
int sw_get_calltime_consum(struct msghead *msghd,int *time,int * routid,int *lineid)
{
    char tembuf[10];
    char tembuf1[10];
    char tembuf2[10];/*the last call info*/
    char *buf1;
    char *buf2;
    char buf3[10];
    char *ptr;
    int i = 0,j = 0;
    buf1 = strtok(msghd->timeconsum,"|");
//    buf1 = strtok(timeconsum,"|");
    while(buf1 != NULL)
    {
        if(0 == i)
        {
            strncpy(tembuf,buf1,strlen(buf1));/*tembuf = 1250:10,24*/
            tembuf[strlen(buf1)] = '\0';
        }
        memset(tembuf2,0x00,sizeof(tembuf2));
        strncpy(tembuf2,buf1,strlen(buf1));  /*the last time consume data*/
        i ++;
        buf1 = strtok(NULL,"|");
    }

    buf2 = strtok(tembuf,":");   /*split tembuf = 1250:10,24 to get routid 1250*/
    while(buf2 != NULL)
    {
        if(0 == j)
            *routid = atoi(buf2);
        if(1 == j)
        {
            strncpy(tembuf1,buf2,strlen(buf2));
            tembuf1[strlen(buf2)] = '\0';
        }
        buf2 = strtok(NULL,":");

        j ++;
    }

    if(1 == i) /*only one call,split 10,24*/
    {
        j = 0;
        buf1 = strtok(tembuf1,",");
        while(buf1 != NULL)
        {
            memset(buf3,0x00,sizeof(buf3));
            strncpy(buf3,buf1,strlen(buf1));
            if(0 == j)
                *lineid = atoi(buf1);
            else if(1 == j)  /*calculate the result from second to milisecond*/
            {

                ptr = strchr(buf3,'s');
                if(ptr)
                {

                    *ptr = '\0';
                    *time = atoi(buf3)*1000;
                }
                else 
                    *time = atoi(buf1);
            }
                j ++;
            buf1 = strtok(NULL,",");
        }
    }
    else if(i > 1)/*i calls,split the last time consume data:tembuf2  20,15s*/
    {
        j = 0;
        buf1 = strtok(tembuf2,",");
        while(buf1 != NULL)
        {
            memset(buf3,0x00,sizeof(buf3));
            strncpy(buf3,buf1,strlen(buf1));
            if(0 == j)
                *lineid = atoi(buf1);
            else if(1 == j)
            {
                ptr = strchr(buf3,'s');
                if(ptr)
                {
                    *ptr = '\0';
                    *time = atoi(buf3)*1000;
                }
                else 
                    *time = atoi(buf1);
            }
            j ++;
            buf1 = strtok(NULL,",");
        }
    }
    return 0;
}

