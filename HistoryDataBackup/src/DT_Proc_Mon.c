/****************************************************************/
/* 模块编号	：DT_Proc_Mon					*/
/* 模块名称	：监控处理模块					*/
/* 版 本 号	：V1.0						*/
/* 作    者	：Hu						*/
/* 建立日期	：2015/3/6 11:32:47                             */
/* 最后修改日期	：2015/3/6 16:13:55                             */
/* 模块用途	：处理监控消息并发送				*/
/* 本模块中包含如下函数及功能说明：				*/
/*	<1>.Mon_Client_Proc	CLIENT端采集发送线程		*/
/*	<2>.Mon_ConnectSerL	连接通讯SERVER端		*/
/*	<3>.Mon_TcpSnd		发送报文到服务端       		*/

/****************************************************************/
/* 修改记录	：                				*/
/* 2015/3/6 16:34:42 XXXXXXX                               	*/
/* 2015/3/6 16:34:44 增加XXX             			*/
/****************************************************************/
#include "DT_Pub.h"


/*监控相关变量*/
char glMonServerIp[14];
int  glMonServerPort;

void Mon_Test()
{
	printf("我来自监控--TEST!\n");	
	_swVdebug(5, "我来自监控--TEST!");
}


/****************************************************************
 ** 函数名      :   Mon_Client_Proc(char *aSendMsg int iSendNum)*
 ** 功  能      :   CLIENT端采集发送线程			*
 ** 作  者      :   Hu 						*
 ** 建立日期    :   						*
 ** 最后修改日期:   2015/4/3 14:49:28				*
 ** 调用其它函数:   2015/4/3 14:49:30				*
 ** 全局变量    :						*
 ** 参数含义    :   aSendMsg：要发送的数据包  iSendNum：数据条数*
 ** 返回值      :   无						*
***************************************************************/
int Mon_Client_Proc(char *aSendMsg, int iSendNum)
{
	short	ilRn;				/* 返回值       */
	short	ilMsglen;			/* 报文长度     */
	char	alMsgbuf[1024];			/* 报文         */
	int 	igsockfd;
	int 	count=0;			//计数器
	  
	memset(alMsgbuf, 0x00, sizeof(alMsgbuf));
  	sprintf(alMsgbuf, "%s", aSendMsg);
	ilMsglen = strlen(alMsgbuf);
	//swVdebug( 4,  "ilMsglen [%d] iSendNum [%d]", ilMsglen, iSendNum);
	//swVdebug( 3,  "SEN alMsgbuf [%s]", alMsgbuf);
	
	while(1){
		igsockfd = -1;
		igsockfd = Mon_ConnectSerL(glMonServerIp, glMonServerPort);
		if (igsockfd<0) {
			_swVdebug( 0, "PtConnectser[%s][%d]SOCKfd[%d],ERRNO[%d %s]",glMonServerIp, glMonServerPort, igsockfd, errno, strerror(errno));
			sleep(2);
			if(count>2){
				_swVdebug("Mon_Client_Proc：[%s][%d] 连接有误！请确认IP和端口是否正确.",glMonServerIp, glMonServerPort);
				return -1;			
			}
			count++;
			continue;	
		}
		break;
	}
	_swVdebug( 0,  "PtConnectser成功!连接到sockfd[%d]aFDip[%s],iFDPort[%d],准备发送ilMsglen[%d]iSendNum[%d]", igsockfd, glMonServerIp, glMonServerPort, ilMsglen, iSendNum);
		
	ilRn = Mon_TcpSnd(igsockfd, alMsgbuf, ilMsglen);
	if (ilRn == 0) {
		close(igsockfd);
		_swVdebug( 0, "Mon_TcpSnd to sockfd[%d] len[%d] success close socket[%d]!", igsockfd, ilMsglen, igsockfd);
		return 0;
	}
	else{
		close(igsockfd);
		return -1;		
	}	  
}


/****************************************************************/
/* 函数编号    ：Mon_ConnectSerL				*/ 
/* 函数名称    ：连接通讯SERVER端				*/
/* 作	 者      ：Hu						*/
/* 建立日期    ：2015/4/3 14:48:12				*/
/* 最后修改日期：2015/4/3 14:48:14				*/
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               出错   1                                       */
/****************************************************************/
int Mon_ConnectSerL(char *alIp,int ilPort)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct linger lin;
	int on=1;
	
	//将serv_addr的各个字段清零
	memset(&serv_addr, 0x00, sizeof(struct sockaddr_in));

	//创建套接字
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		_swVdebug(0, "Creat Socket Error[%d]", errno);
		return (-1);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr( alIp );
	serv_addr.sin_port = htons( ilPort );
	
	//使用sockfd套接字连接到由serv_addr指定的目的地址上
	if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0)
	{
		_swVdebug(0, "Connect Server [%s:%d] Error[%d]", alIp, ilPort, errno);
		close(sockfd);
		return (-1);
	}

	lin.l_onoff = 1;
	lin.l_linger = 2;
	if (setsockopt(sockfd, SOL_SOCKET, SO_LINGER,(char *)&lin, sizeof(lin)) != 0)
	{
		_swVdebug(0,"setsockopt linger error [%d %s]", errno, strerror(errno) );
		close(sockfd);
		return -1;
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_OOBINLINE, (char *)&on, sizeof(on)))
	{
		_swVdebug(0,"setsockopt SO_OOBINLINE error [%d %s]", errno, strerror(errno) );
		close(sockfd);
		return -1;
	}

	on = 1;
	if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *)&on, sizeof(on)))
	{
		_swVdebug(0,"setsockopt TCP_NODELAY error [%d %s]", errno, strerror(errno) );
		close(sockfd);
		return -1;
	}   
	return sockfd;
}

/****************************************************************/
/* 模块编号    ：Mon_TcpSnd					*/
/* 模块名称    ：发送报文到服务端                               */
/* 版 本 号    ：                                               */
/* 作    者    ：Hu                                             */
/* 建立日期    ：2015/4/3 14:48:46				*/
/* 最后修改日期：                                               */
/* 模块用途    ：                                               */
/****************************************************************/
int Mon_TcpSnd(int sockfd, char *buffer, int length)
{
	int	len;
	int totalcnt = 0;
  
	_swVdebug(4, "Now Send Socket[%d]......", sockfd);
	while(totalcnt < length) 
	{
		len = write(sockfd, buffer + totalcnt, length - totalcnt);
		_swVdebug(4, "Send Socket len[%d]",len);
		if (len < 0) 
		{
			if (errno == EINTR) 
			{
				len = 0;
				break;
			}
			else
				return(-1);
		}
		else if (len == 0)
			break;
      
		totalcnt = totalcnt + len;
	}
	return 0;
}