#ifndef  _swGlobal_H_
#define  _swGlobal_H_

/*char	agDebugfile[64];		 �����ļ��� */
/*char	cgDebug;			 �ǵ�����־��־ */
char    procName[20];                 /*������add by cjh 20150320, PSBC_V1.0*/

long    g_current_pid;                   /*������add by dyw 20150327, PSBC_V1.0*/
union   preunpackbuf psgPreunpackbuf[iBUFFLDNUM]; /* ȫ��Ԥ����ṹ */
union   preunpackbuf psgVarbuf[iBUFFLDNUM];
union   preunpackbuf psgVaridbuf[iBUFFLDNUM];

char    agMac_check[iEXPRESSLEN];              /* MACУ����ʽ */
char    agMac_create[iEXPRESSLEN];

short   igRec;
char	*agMsgbody;
//short   igMsgbodylen;
long   igMsgbodylen;/*modified by baiqj20150422*/
short   lgMsgoffset;
char	agCurfld[iFLDVALUELEN + 1];
struct  msghead  *psgMsghead;  /* ����ͷ */
int     igBitmaplen;
unsigned char agBitmap[33];
extern   short  igSrvId;      /* ������id�� */

#endif
