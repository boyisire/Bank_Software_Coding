/****************************************************************/
/* ģ����    ��swFmtapi                                       */
/* ģ������    ����ʽ����api                                    */
/* �� �� ��    ��V4.3.0                                         */
/* ��    ��    ��ʷ����                                         */
/* ��������    ��1999/11/18                                     */
/* ����޸����ڣ�2001/4/8                                       */
/* ģ����;    ����ʽ����                                       */
/* ��ģ���а������º���������˵����                             */
/*           (1) int swFormat(struct msgpack *psMsgpack);     */
/*           (2) int  swPreunpack();                            */
/*           (3) int  swCheckmsghead();                         */
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

int swCmpFmtTrantype(short ilMsgformat,short ilTran_type,short flag );
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetXMFfromTDF(int iFmt_group,long lFmt_offset); */
int swGetXMFfromTDF(long iFmt_group,long lFmt_offset);
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetFMLfromTDF(short iFmt_group,long lFmt_offset,union preunpackbuf *prebuf); */
int swGetFMLfromTDF(long iFmt_group,long lFmt_offset,union preunpackbuf *prebuf);
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetSEPfromTDF(short iFmt_group,long lFmt_offset,union preunpackbuf *prebuf); */
int swGetSEPfromTDF(long iFmt_group,long lFmt_offset,union preunpackbuf *prebuf);
/* del by gengling at 2015.03.25 one line PSBC_V1.0 */
/* int swGetISOfromTDF(short iFmt_group,long lFmt_offset,union preunpackbuf *prebuf); */
int swGetISOfromTDF(long iFmt_group,long lFmt_offset,union preunpackbuf *prebuf);
int swSeppack(union preunpackbuf *prebuf,char *aMsgbody,unsigned int *iMsglen);
int swISOcheck (union preunpackbuf *prebuf, short tab_id);
int swIsopack(union preunpackbuf *prebuf,short qid,unsigned char *msgbody,unsigned int *msglen);
int sw8583pack(union preunpackbuf *prebuf,short qid,unsigned char *msgbody,unsigned int *msglen);
int swXMFpack(union preunpackbuf *prebuf,char *msgbody,unsigned int *msglen);
int swSepunpack(char *aMsgbody, unsigned int iMsglen, char *aRecs, char *aGrps, \
                char *aFlds, union preunpackbuf *prebuf);
int swIsounpack(unsigned char *msgbody,short msglen,short qid,union preunpackbuf *prebuf);
int sw8583unpack(unsigned char *msgbody,short msglen,short qid,union preunpackbuf *prebuf);

union  preunpackbuf psgUnpackbuf[iBUFFLDNUM]; /* TDFת����Ļ���� */
union  preunpackbuf psgMacneedbuf[iBUFFLDNUM];/* MAC���㻺��� */
char   agMac_check[iEXPRESSLEN];              /* MACУ����ʽ */
char   agMac_create[iEXPRESSLEN];             /* MAC���ɱ��ʽ */
short  ilRtncode;                             /* ������ */


/* ����ԭ�Ͷ��� */
int swFormat(struct msgpack *psMsgpack);
short swPreunpack(struct msgpack *psMsgpack);
int swCheckmsghead(struct msgpack * psMsgpack);

/**************************************************************
 ** ������      ��swFormat
 ** ��  ��      ����ʽת��
 ** ��  ��      ��ʷ���� 
 ** ��������    ��1999.8.24
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��psMsgpack ** ���ʽת���ı��ġ�ת����ı��� **
 ** ����ֵ      ��0  �ɹ�
 **              -1  ʧ��
 **              -2  MACУ�������ϵͳ��ط���ϢiMSSYSMON
 **              -3  ����ʶ��ı�������
***************************************************************/
int swFormat(struct msgpack *psMsgpack)
{
  short i;                           /* ��ʱ�������� */
  char  alMsgtranflag[2];            /* ��Ϣ���ݱ�ʶ */
  char  alMsgbody[iMSGMAXLEN];       /* ������ */
  unsigned int ilMsglen;
  char  alResult[iFLDVALUELEN + 1];  /* ���ʽ������ */
  short ilLength;                    /* ���ʽ���������� */
/*  static short flag=0;  */

  short  ilQ_id;                   /* ����� */
  /*modify by zcd 20141226
  short  ilFmt_group;*/
  long  ilFmt_group;              /* ��ʽת������ */
  long   llFmt_offset;
  struct swt_sys_queue slSwt_sys_queue;
  struct swt_sys_fmt_grp slSwt_sys_fmt_grp;

/*
  swVdebug(2,"S3095: before swLicense===============================");
  if (!flag)
  {
    if (_swLicense("swCtrl")) 
    {
      swVdebug(0,"S3000: swFormat����Ч��LICENSE");
      return(-9);
    }
    flag = 1;
  }
*/
  swVdebug(4,"iMsgtype_tbl_1=%d",psMsgpack->sMsghead.iMsgtype);  
  swVdebug(2,"S3168: psMsgpack->iMsgformat=%d", psMsgpack->sMsghead.iMsgformat);
  if (swCheckmsghead(psMsgpack) != SUCCESS)
  {
    swVdebug(1,"S3002: [����/��������] swCheckmsghead()����,����ͷ����!");
    return (-1);
  }
  swVdebug(2,"S3004: ��鱨��ͷ�ɹ�");
  swVdebug(4,"iMsgtype_tbl_2=%d",psMsgpack->sMsghead.iMsgtype);  
  swVdebug(2,"S3168: psMsgpack->iMsgformat=%d", psMsgpack->sMsghead.iMsgformat);
  switch ( psMsgpack->sMsghead.iMsgtype )
  {
    case iMSGUNPACK:
    case iMSGPACK:
    case iMSGREVPACK:
    case iMSGREVUNPACK:
      
      psgMsghead = &(psMsgpack->sMsghead);  /* add by szhengye 2001.10.16 */
      swVdebug(4,"iMsgtype_tbl_3=%d",psMsgpack->sMsghead.iMsgtype);  
      /* memcpy((char *)&sgMsghead, &(psMsgpack->sMsghead), 
        sizeof(struct msghead)); delete by szhengye 2001.10.16 */
   
      lgMsgoffset = 0;

      switch ( psMsgpack->sMsghead.iMsgtype )
      {
        /******************** ������������ ********************/
        case iMSGUNPACK:
        case iMSGREVUNPACK:

          psgUnpackbuf[0].sImfbuf.aFldname[0] = '\0';
          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';
          
          /* Ԥ��� */
          ilRtncode = swPreunpack(psMsgpack);
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3006: [����/��������] swPreunpack()����,Ԥ���ʧ��,������=%d",ilRtncode);
             _swMsgsend(302027, (char *)psMsgpack);
            return (-1);
          }
          swVdebug(2,"S3008: Ԥ����ɹ�");
       
          /* �����ָ��ת����,�����TDFת�� */
          memset(agMac_check, 0x00, sizeof(agMac_check));
          memset(agMac_create, 0x00, sizeof(agMac_create));

          if (psMsgpack->sMsghead.iMsgtype == iMSGUNPACK)
          {
            if (psMsgpack->sMsghead.iFmtgroup)
            {
              ilFmt_group = psMsgpack->sMsghead.iFmtgroup;
              /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
              /* swVdebug(2,"S3010: ָ����ʽ��[%d]", ilFmt_group); */
              swVdebug(2,"S3010: ָ����ʽ��[%ld]", ilFmt_group);
               psMsgpack->sMsghead.iFmtgroup = 0;

              memset(&slSwt_sys_fmt_grp, 0x00, 
                sizeof(struct swt_sys_fmt_grp));
              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group, 
                &slSwt_sys_fmt_grp, &llFmt_offset);
              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(0,"S3012: [����/�����ڴ�] ָ����ʽת����Ϊ[%d]������!",ilFmt_group); */
                swVdebug(1,"S3012: [����/�����ڴ�] ָ����ʽת����Ϊ[%ld]������!",ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check, 
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create, 
                sizeof(agMac_create));

              goto GetFMLFromTDF;
            }
          }
          else if (psMsgpack->sMsghead.iMsgtype == iMSGREVUNPACK)
          {
            if (psMsgpack->sMsghead.iRevfmtgroup)
            {
              ilFmt_group = psMsgpack->sMsghead.iRevfmtgroup;
              /* del by genling at 2015.03.25 one line */
              /* swVdebug(2,"S3014: ָ��������ʽ��[%d]", ilFmt_group); */
              swVdebug(2,"S3014: ָ��������ʽ��[%ld]", ilFmt_group);
               psMsgpack->sMsghead.iFmtgroup = 0;
              psMsgpack->sMsghead.iRevfmtgroup = 0;

              memset(&slSwt_sys_fmt_grp, 0x00, 
                sizeof(struct swt_sys_fmt_grp));
              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group,
                &slSwt_sys_fmt_grp, &llFmt_offset);

              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(0,"S3012: [����/�����ڴ�] ָ����ʽת����Ϊ[%d]������!",ilFmt_group); */
                swVdebug(1,"S3012: [����/�����ڴ�] ָ����ʽת����Ϊ[%ld]������!",ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check,
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create,
                sizeof(agMac_create));

              goto GetFMLFromTDF;
            }
          }
          /* ����������� */
          ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iOrg_q, 
            &slSwt_sys_queue);
          if (ilRtncode != SUCCESS)
          {
            swVdebug(1,"S3018: [����/�����ڴ�] ������[%d]û����!", psMsgpack->sMsghead.iOrg_q);
            swDebugmsghead((char *)psMsgpack);
            swDebughex((char *)psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
             return (-1);
          }
          swVdebug(2,"S3020: ����[%d]�����������ʽ=[%s]",
               psMsgpack->sMsghead.iOrg_q, slSwt_sys_queue.tc_unpack);

          memset(alResult, 0x00, sizeof(alResult));

          ilRtncode = _swExpress(slSwt_sys_queue.tc_unpack, 
            alResult, &ilLength);

          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3022: [����/��������] _swExpress()����,�Զ��庯��[%s]�������!",
               slSwt_sys_queue.tc_unpack);
            _swMsgsend(302007, NULL); 
            return (-1);
          }

          if (alResult[0] == 0x00)
          {
            swVdebug(1,"S3024: [����/����] ������Ϊ��!");
             return (-1);
          }

          /* ������ͷ��ֵ */
          memset(psMsgpack->sMsghead.aTrancode, 0x00, 
	    sizeof(psMsgpack->sMsghead.aTrancode));
          memcpy(psMsgpack->sMsghead.aTrancode, alResult, ilLength);

	  swVdebug(2,"S3026: ���������Ϊ[%10.10s]", 
               psMsgpack->sMsghead.aTrancode);

          /* ���ݱ���ͷ�еı������;�����Ϣ���ݱ�ʶ */
          memset(alMsgtranflag, 0x00, sizeof(alMsgtranflag));
          if (psMsgpack->sMsghead.iMsgtype == iMSGUNPACK)
            alMsgtranflag[0] = cMSGOUT; 
          else
            alMsgtranflag[0] = cMSGREVOUT;

          /* ���ݱ���ͷԴ���䡢�����롢���ײ��衢��Ϣ���ݱ�ʶ��λ��ʽת���� */
          ilRtncode = swShmselect_swt_sys_fmt_m(psMsgpack->sMsghead.iOrg_q, 
            psMsgpack->sMsghead.aTrancode, psMsgpack->sMsghead.iTranstep, 
            alMsgtranflag, &slSwt_sys_fmt_grp, &llFmt_offset);

          if (ilRtncode != SUCCESS)
          {
            swVdebug(1,"S3028: [����/�����ڴ�] ��ʽת�����޷���λ!:[%d],[%s],[%d],[%s]",
               psMsgpack->sMsghead.iOrg_q, psMsgpack->sMsghead.aTrancode,
              psMsgpack->sMsghead.iTranstep, alMsgtranflag);
            _swMsgsend(302003, NULL); 
            return (-1);
          }

          /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
          /* swVdebug(2,"S3030: ��λ��ʽת����Ϊ[%d]",slSwt_sys_fmt_grp.fmt_group); */
          swVdebug(2,"S3030: ��λ��ʽת����Ϊ[%ld]",slSwt_sys_fmt_grp.fmt_group);
           
          ilFmt_group = slSwt_sys_fmt_grp.fmt_group;

          /* 0 - XMF->IMF(���) 1 - IMF->XMF(���) */
          ilRtncode = swCmpFmtTrantype(psMsgpack->sMsghead.iMsgformat, 
            slSwt_sys_fmt_grp.tran_type, 0);

          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3032: [����/��������] swCmpFmtTrantype()����,����ת���������ʽ���Ͳ�ƥ��,������=%d",ilRtncode);
            /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
            /* swVdebug(0,"S3034: [����/��������] ��������[%d],��ʽת����[%d]����[%d]!", */
            swVdebug(1,"S3034: [����/��������] ��������[%d],��ʽת����[%ld]����[%d]!",
               psMsgpack->sMsghead.iMsgformat, 
              slSwt_sys_fmt_grp.fmt_group, 
              slSwt_sys_fmt_grp.tran_type);
            _swMsgsend(302006, NULL); 
            return (-1);
          }

GetFMLFromTDF:

          lgMsgoffset = 0;
          psgUnpackbuf[0].sImfbuf.aFldname[0] = '\0';
          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';

          /* ������Ӧ��ʽת��������FMLԤ�ð� */
	  swVdebug(2,"S3036: ������Ӧ��ʽת��������FMLԤ�ð�");
          if (psMsgpack->sMsghead.iMsgformat == iFMTXMF)
          {
            /* ��XMF���ĸ���TDF����XMF���� */
            ilRtncode = swGetXMFfromTDF(ilFmt_group,llFmt_offset);
          }
          else
          {
            ilRtncode = swGetFMLfromTDF(ilFmt_group,llFmt_offset,psgUnpackbuf);
          }
          /* if ( ilRtncode == FAIL ) modify by szhengye 2001.10.12 */
          if ( ilRtncode )
          {
            swVdebug(1,"S3038: [����/��������] �õ���׼��ֵʧ��,������=%d",ilRtncode);
             _swMsgsend(302015, NULL); 
            return (ilRtncode);
          }

	 swVdebug(2,"S3040: MAC������ʽΪ[%s]", agMac_check);
 	 
	  /* ����MACУ�� */
          #ifdef NEEDMACCHECK
	  _swTrim(agMac_check);
	  if (strlen(agMac_check)) /* need check mac */
	  {
	    swVdebug(2,"S3042: ��Ҫ����MAC����");
 	    memcpy( psgMacneedbuf,psgPreunpackbuf,sizeof(psgMacneedbuf));
	    memset(alResult,0x00,sizeof(alResult));
	    if (_swExpress( agMac_check, alResult,&ilLength)==FAIL)
	    {
	      swVdebug(1,"S3044: [����/��������] _swExpress()����,MACУ�����!");
 	      _swMsgsend(302004, (char *)psMsgpack);
	      return (-1);
            }
            swVdebug(2,"S3046: swExpress �ɹ�");
 	    if (alResult[0]!='1')  /* mac check error */
	    { 
	      swVdebug(2,"S3048: MACУ��ʧ��!");
 
	      memset(alMsgbody,0x00,sizeof(alMsgbody));
              ilRtncode = swFmlpack( psgUnpackbuf , alMsgbody, &ilMsglen);
              if (ilRtncode == FAIL)
              {
                swVdebug(1,"S3050: [����/��������] swFmlpack()����,�����FML����ʧ��!");
                 _swMsgsend(302016, NULL); 
                return (-1);
              }

              /* �����µı���ͷ,�����ӱ����� */
              psMsgpack->sMsghead.iMsgformat = iFMTIMF;
              psMsgpack->sMsghead.iBodylen = ilMsglen;
              memcpy(psMsgpack->aMsgbody, alMsgbody, ilMsglen );

              psMsgpack->sMsghead.iMsgtypeorg = psMsgpack->sMsghead.iMsgtype;
              psMsgpack->sMsghead.iMsgtype = iMSGMESSAGE;
              psMsgpack->sMsghead.lCode = 302099;
              time(&psMsgpack->sMsghead.lBegintime);

              swVdebug(2,"S3052: ������Ϣ����(����:%d)��ϵͳ���", 
                   psMsgpack->sMsghead.iBodylen);
              swVdebug(3,"S3054: ���Ŀ�ʼ");
               if (cgDebug >= 2)
              {
                swDebugmsghead((char *)&psMsgpack);
                swDebughex(psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
              }
              swVdebug(3,"S3056: ���Ľ���");
               /* return (-2);  mac У������������䷢��Ϣ */
              _swMsgsend(302099, (char *)psMsgpack);
              return (-1);
	    }
	    else
	    {
	      swVdebug(2,"S3058: MACУ��ɹ�");
             }
	  }
          else 
	    swVdebug(2,"S3060: ����Ҫ����MAC����");
          #endif
 
          /* ��FMLԤ�������IMF�б��е���У�� */
          #ifdef NEEDFMLCHECK
          ilRtncode = swFMLcheck(psgUnpackbuf);
          if (ilRtncode == FAIL)
          {
	    swVdebug(2,"S3062: FML��У��ʧ��!");
             _swMsgsend(302024, NULL);
            return (-1);
          }
          #endif

          /* ��Ԥ������FML��ʽ���� */
          
          swVdebug(3,"S3064: FMLԤ�ð�����Ϊ����");
          for (i=0;i<5;i++)
          {
            swVdebug(3,"S3066: ����=[%s],�򳤶�=[%d],��ֵ=[%s]",
              psgUnpackbuf[i].sImfbuf.aFldname,
              psgUnpackbuf[i].sImfbuf.iFldlen,
              psgUnpackbuf[i].sImfbuf.aFldvalue);
          }

          ilRtncode = swFmlpack( psgUnpackbuf , alMsgbody, &ilMsglen);
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3068: [����/��������] swGetImfValueFormTdf()����,�����FML����ʧ��!");
              _swMsgsend(302016, NULL); 
            return (-1);
          }

          /* �����µı���ͷ,�����ӱ����� */
          psMsgpack->sMsghead.iMsgformat = iFMTIMF;
          psMsgpack->sMsghead.iBodylen = ilMsglen;
          memcpy(psMsgpack->aMsgbody, alMsgbody, ilMsglen );

          swVdebug(2,"S3070: �������������ɹ�");
          swVdebug(3,"S3072: ����������������BEGIN");
           if (cgDebug >= 2)
          {
            swDebugmsghead((char *)psMsgpack);
            swDebughex(psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
          }
          swVdebug(3,"S3074: ����������������END");
           return 0;
          break;
        /******************** ������������ ********************/
        case iMSGPACK:
        case iMSGREVPACK:

          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';

          swVdebug(2,"S3076: �յ�FML����");
           if (cgDebug>=2) swDebugfml((char *)psMsgpack);
          agMsgbody = psMsgpack->aMsgbody;
          igMsgbodylen = psMsgpack->sMsghead.iBodylen;

          /* ��FML��������ȫ��Ԥ�ð��� */
          ilRtncode = swFmlunpack(psMsgpack->aMsgbody, 
            psMsgpack->sMsghead.iBodylen,psgPreunpackbuf);
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3078: [����/��������] swFmlunpack()����,Ԥ�������!");
             _swMsgsend(302010, NULL); 
            return (-1);
          }
          swVdebug(2,"S3080: Ԥ����ɹ�");
          
           /* ����Դ����Ų�ѯMACλͼ */
          ilQ_id = psMsgpack->sMsghead.iDes_q; 

          /* �ñ���ͷ���ĸ�ʽ���� */
          ilRtncode = swShmselect_swt_sys_queue(ilQ_id, &slSwt_sys_queue);

          if (ilRtncode != SUCCESS)
          {
            swVdebug(1,"S3082: [����/�����ڴ�] ������[%d]û����!", psMsgpack->sMsghead.iDes_q);
             return (-1);
          }
          psMsgpack->sMsghead.iMsgformat = slSwt_sys_queue.msg_format;

	  /* �����ָ��ת����,�����TDFת�� */
          if ( psMsgpack->sMsghead.iMsgtype == iMSGPACK)
          {
	    if (psMsgpack->sMsghead.iFmtgroup)
	    {
              ilFmt_group = psMsgpack->sMsghead.iFmtgroup;
              /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
	          /* swVdebug(2,"S3084: ָ����ʽ��[%d]", ilFmt_group); */
	          swVdebug(2,"S3084: ָ����ʽ��[%ld]", ilFmt_group);
              psMsgpack->sMsghead.iFmtgroup = 0;

              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group,
                &slSwt_sys_fmt_grp, &llFmt_offset);

              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(1,"S3086: [����/�����ڴ�] ָ����ʽ��[%d]������!", ilFmt_group); */
                swVdebug(1,"S3086: [����/�����ڴ�] ָ����ʽ��[%ld]������!", ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check, 
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create, 
                sizeof(agMac_create));

              goto GetFMLFromTDF2;
	    }
          }
          else if (psMsgpack->sMsghead.iMsgtype == iMSGREVPACK)
          {
	    if (psMsgpack->sMsghead.iRevfmtgroup)
	    {
              ilFmt_group = psMsgpack->sMsghead.iRevfmtgroup;
              /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
	          /* swVdebug(2,"S3084: ָ����ʽ��[%d]", ilFmt_group); */
	          swVdebug(2,"S3084: ָ����ʽ��[%ld]", ilFmt_group);
              psMsgpack->sMsghead.iFmtgroup = 0;
              psMsgpack->sMsghead.iRevfmtgroup = 0;

              ilRtncode = swShmselect_swt_sys_fmt_grp(ilFmt_group,
                &slSwt_sys_fmt_grp, &llFmt_offset);

              if (ilRtncode != SUCCESS)
              {
                /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
                /* swVdebug(0,"S3086: [����/�����ڴ�] ָ����ʽ��[%d]������!", ilFmt_group); */
                swVdebug(1,"S3086: [����/�����ڴ�] ָ����ʽ��[%ld]������!", ilFmt_group);
                return (-1);
              }
              memcpy(agMac_check, slSwt_sys_fmt_grp.mac_check,
                sizeof(agMac_check));
              memcpy(agMac_create, slSwt_sys_fmt_grp.mac_create,
                sizeof(agMac_create));

              goto GetFMLFromTDF2;
	    }
          }

          /* ���ݱ���ͷ�еı������;�����Ϣ���ݱ�ʶ */
          memset(alMsgtranflag, 0x00, sizeof( alMsgtranflag));
          if (psMsgpack->sMsghead.iMsgtype == iMSGPACK)
            alMsgtranflag[0] = cMSGIN; 
          else
            alMsgtranflag[0] = cMSGREVIN;
          swVdebug(2,"S3092: ����[%d]�����������ʽ=[%s]",
               psMsgpack->sMsghead.iDes_q, slSwt_sys_queue.tc_pack);
            
          /* ����������� */
          memset(alResult, 0x00, sizeof(alResult));

          ilRtncode = _swExpress(slSwt_sys_queue.tc_pack,
            alResult, &ilLength);

          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3094: [����/��������] _swExpress()����,�������������Զ��庯��[%s]�������!",
               slSwt_sys_queue.tc_pack); 
            _swMsgsend(302007, NULL); 
            return (-1);
          }
          memset(psMsgpack->sMsghead.aTrancode,0x00,
            sizeof(psMsgpack->sMsghead.aTrancode));
          memcpy(psMsgpack->sMsghead.aTrancode,alResult,ilLength);
          swVdebug(2,"S3096: result=[%s],length=[%d]",alResult,ilLength);
           swVdebug(2,"S3098: ����Ľ�����Ϊ[%10.10s]",
               psMsgpack->sMsghead.aTrancode);

          /* ���ݱ���ͷ���䡢�����롢���ײ��衢��Ϣ���ݱ�ʶ��λ��ʽת���� */
          ilRtncode = swShmselect_swt_sys_fmt_m(psMsgpack->sMsghead.iDes_q, 
            psMsgpack->sMsghead.aTrancode, psMsgpack->sMsghead.iTranstep, 
            alMsgtranflag, &slSwt_sys_fmt_grp, &llFmt_offset ); 
            
          if (ilRtncode != SUCCESS)
          {
             swVdebug(1,"S3100: [����/�����ڴ�] ��ʽת�����޷���λ!:[%d],[%s],[%d],[%s]",
                psMsgpack->sMsghead.iDes_q, psMsgpack->sMsghead.aTrancode,
               psMsgpack->sMsghead.iTranstep, alMsgtranflag); /* 2000.3.16 */
             _swMsgsend(302003, NULL); 
             return (-1);
          }

          ilFmt_group = slSwt_sys_fmt_grp.fmt_group;

          /* del by gengling at 2015.03.25 one line PSBC_V1.0 */
          /* swVdebug(2,"S3102: ��ʽת����Ϊ[%d]",ilFmt_group); */
          swVdebug(2,"S3102: ��ʽת����Ϊ[%ld]",ilFmt_group);
 
          /* ���ݱ���ת�������뱨��ͷ���ĸ�ʽ�����Ƿ�ƥ�� */
          /* 0 - XMF->IMF(���) 1 - IMF->XMF(���) */
          ilRtncode = swCmpFmtTrantype(psMsgpack->sMsghead.iMsgformat,
            slSwt_sys_fmt_grp.tran_type, 1);

          if ( ilRtncode == FAIL )
          {
            swVdebug(1,"S3104: [����/��������] swCmpFmtTrantype()����,�������ת���������ʽ���Ͳ�ƥ��!");
            swVdebug(1,"S3106: [����/��������]��������[%d],��ʽת��������[%d]!",
               psMsgpack->sMsghead.iMsgformat,
              slSwt_sys_fmt_grp.tran_type);
            _swMsgsend(302006, NULL); 
            return (-1);
          }

GetFMLFromTDF2:

          lgMsgoffset = 0;
          psgVarbuf[0].sImfbuf.aFldname[0] = '\0';

          /* ���ݱ��ĸ�ʽ���͵���Ӧ��Ԥ��� */
          switch (psMsgpack->sMsghead.iMsgformat)
          {
            case iFMTIMF:        /* �ڲ�����Ԥ��� */

              psgUnpackbuf[0].sImfbuf.aFldname[0] = '\0';

              /* ������Ӧ��ʽת��������FMLԤ�ð� */
	      swVdebug(2,"S3108: ��ʼ swGetFMLfromTDF...");
               ilRtncode = swGetFMLfromTDF(ilFmt_group,
                 llFmt_offset,psgUnpackbuf);
	      swVdebug(2,"S3110: swGetFMLfromTDF..�ɹ�");
                 break;
            case iFMTNOSEP:      /* �޷ָ�������Ԥ��� */
            case iFMTSEP:        /* �зָ�������Ԥ��� */

              psgUnpackbuf[0].sSepbuf.iRecid = -1;

              /* ������Ӧ��ʽת�����������޷ָ�������Ԥ�ð� */
	      swVdebug(2,"S3112: begin swGetSEPfromTDF...");
               ilRtncode = swGetSEPfromTDF(ilFmt_group,
                llFmt_offset,psgUnpackbuf);
	      swVdebug(2,"S3114: swGetSEPfromTDF...ok");
                 break;
            case iFMT8583:       /* ISO8583����Ԥ��� */
            case iFMT8583E:

              psgUnpackbuf[0].sIsobuf.iFldid = -1;
              psgUnpackbuf[0].sIsobuf.iFldlen = -1;
              psgUnpackbuf[0].sIsobuf.iOrgfldlen = -1;

              /* ������Ӧ��ʽת��������8583Ԥ�ð� */
	      swVdebug(2,"S3116: begin swGetISOfromTDF...");
               ilRtncode = swGetISOfromTDF(ilFmt_group,
                llFmt_offset,psgUnpackbuf);
	      swVdebug(2,"S3118: swGetISOfromTDF...ok");
                break;
            case iFMTXMF:       /* XMF����Ԥ��� */

              psgUnpackbuf[0].sIsobuf.iFldid = -1;

              /* ������Ӧ��ʽת��������XMFԤ�ð� */
	      swVdebug(2,"S3120: begin swGetXMFfromTDF...");
              ilRtncode = swGetXMFfromTDF(ilFmt_group,llFmt_offset);
	      swVdebug(2,"S3122: swGetXMFfromTDF...ok");
	      
                break;
          } 
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3124: [����/��������] swGetImfValueFormTdf()����,�õ���׼��ֵʧ��!");
             _swMsgsend(302015, NULL);
            return (-1);
          }
	  #ifdef NEEDFMLCHECK
          if (psMsgpack->sMsghead.iMsgformat == iFMTIMF)
          {
            /* ��FMLԤ�������IMF�б��е���У�� */
            ilRtncode = swFMLcheck(psgUnpackbuf);
            if (ilRtncode == FAIL)
            {
	      swVdebug(1,"S3126: [����/��������] swFMLcheck()����,IMF��У�����!");
               _swMsgsend(302024, NULL);
              return (-1);
            }
          }
	  #endif

	  swVdebug(2,"S3128: ��ʼ���...");
          /* ���ݱ��ĸ�ʽ���;������ */
          switch (psMsgpack->sMsghead.iMsgformat)
          { 
            case iFMTIMF:        /* �ڲ�������� */
              ilRtncode = swFmlpack( psgUnpackbuf, alMsgbody, &ilMsglen );
	      swVdebug(2,"S3130: swFmlpack():������,�����峤��[%d]",ilMsglen);
               break;
            case iFMTNOSEP:      /* �޷ָ���������� */
            case iFMTSEP:        /* �зָ���������� */
              ilRtncode = swSeppack(psgUnpackbuf,alMsgbody,&ilMsglen );
	      swVdebug(2,"S3132: swSeppack():������,�����峤��[%d]",ilMsglen);
               break;
            case iFMT8583:       /* ISO8583������� */
              /* ISO8583�������У�� */
              ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iDes_q,
                &slSwt_sys_queue);
              if (ilRtncode != SUCCESS)
              {
                return (-1);
              }

              #ifdef NEEDISOCHECK
              ilRtncode = swISOcheck(psgUnpackbuf, slSwt_sys_queue.isotab_id); 

              if (ilRtncode == FAIL)
              {
                swVdebug(1,"S3134: [����/��������] swISOcheck()����,ISO���ļ��ʧ��!");
                 _swMsgsend(302026, (char *)psMsgpack);
                return (-1);
              }
              #endif

              ilRtncode = swIsopack(psgUnpackbuf,
                psMsgpack->sMsghead.iDes_q,(unsigned char *)alMsgbody,
                &ilMsglen);

	      swVdebug(2,"S3136: swIsopack():������,�����峤��[%d]", ilMsglen);
 	      
              swVdebug(3,"S3138: �����忪ʼ"); 
 	      if (cgDebug>=2)
              {
		swDebughex(alMsgbody,ilMsglen);
	      }
	      swVdebug(3,"S3140: ���������");
 
	      _swTrim(agMac_create);
              if (strlen(agMac_create))
	      {
                memcpy(psgMacneedbuf ,psgUnpackbuf,sizeof(psgMacneedbuf));
	        memset(alResult,0x00,sizeof(alResult));
	        if (_swExpress(agMac_create, alResult,&ilLength)==FAIL)
	        {
		  swVdebug(1,"S3142: [����/��������] _swExpress()����,����MAC����(���ʽ)!");
 		  _swMsgsend(302005, (char *)psMsgpack);
		  return (-1);
                }
	        if (ilLength==0)  /* mac create error */
	        {
		  swVdebug(1,"S3144: [����/����] MAC���ɳ���!");
 		  _swMsgsend(302005, (char *)psMsgpack);
		  return (-1);
	        }
		/* ׷�ӵ�����֮�� */
		psMsgpack->sMsghead.iBodylen=ilMsglen+16;
	        memcpy((char *)psMsgpack+ilMsglen,(unsigned char*)alResult,16);
	      }

              break;
            case iFMT8583E:
              ilRtncode = sw8583pack(psgUnpackbuf,
                psMsgpack->sMsghead.iDes_q,(unsigned char *)alMsgbody,
                &ilMsglen);

              swVdebug(2,"S3146: sw8583pack(): ������,�����峤��[%d]",ilMsglen);

              swVdebug(3,"S3148: �����忪ʼ");
              if (cgDebug>=2)
              {
                swDebughex(alMsgbody,ilMsglen);
              }
              swVdebug(3,"S3150: ���������");
              break;
            case iFMTXMF:        /* XMF������� */
              ilRtncode = swXMFpack(psgUnpackbuf,alMsgbody,&ilMsglen );
	      swVdebug(2,"S3152: swXMFpack():������,�����峤��[%d]",ilMsglen);
               break;
            default:
              swVdebug(1,"S3154: [����/����] ����ʶ��ı�������!");
               _swMsgsend(302002, NULL); 
              return (-1);
          }
          if (ilRtncode == FAIL)
          {
            swVdebug(1,"S3156: [����/��������] swFmlpack()����,�ڲ��������ʧ��!");
             _swMsgsend(302020, NULL); 
            return (-1);
          }

          /* �����µı���ͷ,�����ӱ����� */
          psMsgpack->sMsghead.iBodylen = ilMsglen;
          memcpy( psMsgpack->aMsgbody, alMsgbody, ilMsglen );
						 
          swVdebug(2,"S3158: �������������ɹ�");
           swVdebug(3,"S3160: ����������������BEGIN");
           if (cgDebug >= 2)
          {
            swDebugmsghead((char *)psMsgpack);
            swDebughex(psMsgpack->aMsgbody,psMsgpack->sMsghead.iBodylen);
          }
          swVdebug(3,"S3162: ����������������END");
           return 0;
          break;

        default:
          swVdebug(1,"S3164: [����/����] ����ʶ��ı�������!");
           _swMsgsend(302002, NULL); 
          return (-3); /* ����ʶ��ı������� */
      }
      break;
    default:
      swVdebug(1,"S3166: [����/����] ����ʶ��ı�������!");
       _swMsgsend(302002, NULL); 
      return (-3); /* ����ʶ��ı������� */      
  }
}

/**************************************************************
 ** ������      ��swPreunpack
 ** ��  ��      ������Ԥ���
 ** ��  ��      ���Ż�
 ** ��������    ��2000/2/18
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    �� psgPreunpackbuf Ԥ���������
 **             �� agMsgbody �޷ָ������Ĵ����
 ** ��������    �� psMsgpack->��ҪԤ����ı���
 ** ����ֵ      �� SUCCESS, FAIL
 **************************************************************/
short swPreunpack(struct msgpack *psMsgpack)
{
  struct swt_sys_queue slSwt_sys_queue;

  swVdebug(2,"S3168: psMsgpack->iMsgformat=%d",
       psMsgpack->sMsghead.iMsgformat);

  /* delete by szhengye
  memset(psgPreunpackbuf,0x00,sizeof(union preunpackbuf)*iBUFFLDNUM);
  */

  switch(psMsgpack->sMsghead.iMsgformat)
  {
    case iFMTIMF:
      agMsgbody = psMsgpack->aMsgbody;
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      psgPreunpackbuf[0].sImfbuf.aFldname[0] = '\0';
      ilRtncode = swFmlunpack(psMsgpack->aMsgbody, 
        psMsgpack->sMsghead.iBodylen,psgPreunpackbuf);
      if ( ilRtncode == FAIL )
      {
        swVdebug(1,"S3170: [����/��������] swFmlunpack()����,FML����Ԥ���ʧ��!");
         _swMsgsend(302010, NULL); 
	return(FAIL);
      }
      swVdebug(2,"S3172: FML����Ԥ����ɹ�");
       break;

    case iFMTNOSEP:  /* �޷ָ�����XMF���Ĳ�Ԥ��� */
    case iFMTXMF:
      agMsgbody = psMsgpack->aMsgbody; /* add by szhengye 2001.9.11 */
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      swVdebug(2,"S3174: �޷ָ�����XMF���Ĳ�Ԥ���");
       break;

    case iFMTSEP:
      /* delete by szhengye 2001.9.11
      memcpy(agMsgbody,psMsgpack->aMsgbody, psMsgpack->sMsghead.iBodylen); */
      agMsgbody = psMsgpack->aMsgbody; /* add by szhengye 2001.9.11 */
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;

      ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iOrg_q,
        &slSwt_sys_queue);
      if (ilRtncode != SUCCESS)
      {
        return (-1);
      }

      psgPreunpackbuf[0].sSepbuf.iRecid = -1;

      ilRtncode = swSepunpack( psMsgpack->aMsgbody,
        psMsgpack->sMsghead.iBodylen,
        slSwt_sys_queue.def_rs,
        slSwt_sys_queue.def_gs,
        slSwt_sys_queue.def_fs,
        psgPreunpackbuf);

      if ( ilRtncode == FAIL )
      {
        swVdebug(1,"S3176: [����/��������] swSepunpack()����,SEP����Ԥ���ʧ��!");
         _swMsgsend(302011, NULL); 
	return(FAIL);
      }
      swVdebug(2,"S3178: SEP����Ԥ����ɹ�");
       break;

    case iFMT8583:
      agMsgbody = psMsgpack->aMsgbody;
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      psgPreunpackbuf[0].sIsobuf.iFldid = -1;
      ilRtncode = swIsounpack((unsigned char *)psMsgpack->aMsgbody, 
        psMsgpack->sMsghead.iBodylen,
        psMsgpack->sMsghead.iOrg_q,
        psgPreunpackbuf);
      if ( ilRtncode == FAIL )
      {
        swVdebug(1,"S3180: [����/��������] swIsounpack()����,ISO����Ԥ���ʧ��!");
         _swMsgsend(302012, NULL); 
	return(FAIL);
      }
      ilRtncode = swShmselect_swt_sys_queue(psMsgpack->sMsghead.iOrg_q,
        &slSwt_sys_queue);
      if (ilRtncode != SUCCESS)
      {
        return (-1);
      }
      
      /* ISO8583����������У�� */
      #ifdef NEEDISOCHECK
      ilRtncode = swISOcheck(psgPreunpackbuf, slSwt_sys_queue.isotab_id);

      if (ilRtncode == FAIL)
      {
        swVdebug(1,"S3182: [����/��������] swISOcheck()����,ISO����Ԥ������ʧ��!");
         _swMsgsend(302026, (char *)psMsgpack);
	return(FAIL);
      }
      #endif

      swVdebug(2,"S3184: ISO����Ԥ����ɹ�");
 
      break;

    case iFMT8583E:
      agMsgbody = psMsgpack->aMsgbody;
      igMsgbodylen = psMsgpack->sMsghead.iBodylen;
      psgPreunpackbuf[0].sIsobuf.iFldid = -1;
      ilRtncode = sw8583unpack((unsigned char *)psMsgpack->aMsgbody,
        psMsgpack->sMsghead.iBodylen, 
        psMsgpack->sMsghead.iOrg_q,
        psgPreunpackbuf);
      if (ilRtncode == FAIL)
      {
        swVdebug(1,"S3186: [����/��������] sw8583unpack()����, ISO8583����Ԥ���ʧ��!");
        _swMsgsend(302012, NULL);
        return(FAIL);
      }
      swVdebug(2,"S3188: ISO8583����Ԥ����ɹ�");

      break;

    default:
     swVdebug(1,"S3190: [����/����] ����ʶ��ı��ĸ�ʽ!");
       _swMsgsend(302002, (char *)psMsgpack);
      return(FAIL);
      break;
  }
  return(SUCCESS);
}

/**************************************************************
 ** ������      ��swCheckmsghead
 ** ��  ��      ������ͷ���
 ** ��  ��      ���Ż�
 ** ��������    ��2000/3/14
 ** ����޸����ڣ�
 ** ��������������
 ** ȫ�ֱ���    ��
 ** ��������    ��psMsgpack->��ҪԤ����ı���
 ** ����ֵ      ��SUCCESS, FAIL
 **************************************************************/
int swCheckmsghead(struct msgpack * psMsgpack)
{
  switch(psMsgpack->sMsghead.iMsgtype)
  {
    case iMSGAPP:
    case iMSGREV:
    case iMSGUNPACK:
    case iMSGPACK:
    case iMSGREVUNPACK:
    case iMSGREVPACK:
    case iMSGSAVEHEAD:
    case iMSGLOADHEAD:
    case iMSGROUTE:
    case iMSGMESSAGE:
    case iMSGORDER:
      break;
    default:
      swVdebug(1,"S3192: [����/����] ����ʶ��ı�������!");
       return(FAIL);
      break;
  }
  return(SUCCESS);
}

