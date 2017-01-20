#ifndef  _swGlobal_H_
#define  _swGlobal_H_

/*char	agDebugfile[64];		 调试文件名 */
/*char	cgDebug;			 记调试日志标志 */
char    procName[20];                 /*程序名add by cjh 20150320, PSBC_V1.0*/

long    g_current_pid;                   /*程序名add by dyw 20150327, PSBC_V1.0*/
union   preunpackbuf psgPreunpackbuf[iBUFFLDNUM]; /* 全局预解包结构 */
union   preunpackbuf psgVarbuf[iBUFFLDNUM];
union   preunpackbuf psgVaridbuf[iBUFFLDNUM];

char    agMac_check[iEXPRESSLEN];              /* MAC校验表达式 */
char    agMac_create[iEXPRESSLEN];

short   igRec;
char	*agMsgbody;
//short   igMsgbodylen;
long   igMsgbodylen;/*modified by baiqj20150422*/
short   lgMsgoffset;
char	agCurfld[iFLDVALUELEN + 1];
struct  msghead  *psgMsghead;  /* 报文头 */
int     igBitmaplen;
unsigned char agBitmap[33];
extern   short  igSrvId;      /* 服务类id号 */

#endif
