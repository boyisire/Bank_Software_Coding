/****************************************************************/
/* ģ����	��DT_Proc_Mon					*/
/* ģ������	����ش���ģ��					*/
/* �� �� ��	��V1.0						*/
/* ��    ��	��Hu						*/
/* ��������	��2015/3/6 11:32:47                             */
/* ����޸�����	��2015/3/6 16:13:55                             */
/* ģ����;	����������Ϣ������				*/
/* ��ģ���а������º���������˵����				*/
/*	<1>.Mon_Client_Proc	CLIENT�˲ɼ������߳�		*/
/*	<2>.Mon_ConnectSerL	����ͨѶSERVER��		*/
/*	<3>.Mon_TcpSnd		���ͱ��ĵ������       		*/

/****************************************************************/
/* �޸ļ�¼	��                				*/
/* 2015/3/6 16:34:42 XXXXXXX                               	*/
/* 2015/3/6 16:34:44 ����XXX             			*/
/****************************************************************/
#include "DT_Pub.h"


/*�����ر���*/
char glMonServerIp[14];
int  glMonServerPort;

void Mon_Test()
{
	printf("�����Լ��--TEST!\n");	
	_swVdebug(5, "�����Լ��--TEST!");
}


/****************************************************************
 ** ������      :   Mon_Client_Proc(char *aSendMsg int iSendNum)*
 ** ��  ��      :   CLIENT�˲ɼ������߳�			*
 ** ��  ��      :   Hu 						*
 ** ��������    :   						*
 ** ����޸�����:   2015/4/3 14:49:28				*
 ** ������������:   2015/4/3 14:49:30				*
 ** ȫ�ֱ���    :						*
 ** ��������    :   aSendMsg��Ҫ���͵����ݰ�  iSendNum����������*
 ** ����ֵ      :   ��						*
***************************************************************/
int Mon_Client_Proc(char *aSendMsg, int iSendNum)
{
	short	ilRn;				/* ����ֵ       */
	short	ilMsglen;			/* ���ĳ���     */
	char	alMsgbuf[1024];			/* ����         */
	int 	igsockfd;
	int 	count=0;			//������
	  
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
				_swVdebug("Mon_Client_Proc��[%s][%d] ����������ȷ��IP�Ͷ˿��Ƿ���ȷ.",glMonServerIp, glMonServerPort);
				return -1;			
			}
			count++;
			continue;	
		}
		break;
	}
	_swVdebug( 0,  "PtConnectser�ɹ�!���ӵ�sockfd[%d]aFDip[%s],iFDPort[%d],׼������ilMsglen[%d]iSendNum[%d]", igsockfd, glMonServerIp, glMonServerPort, ilMsglen, iSendNum);
		
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
/* �������    ��Mon_ConnectSerL				*/ 
/* ��������    ������ͨѶSERVER��				*/
/* ��	 ��      ��Hu						*/
/* ��������    ��2015/4/3 14:48:12				*/
/* ����޸����ڣ�2015/4/3 14:48:14				*/
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ����   1                                       */
/****************************************************************/
int Mon_ConnectSerL(char *alIp,int ilPort)
{
	int sockfd;
	struct sockaddr_in serv_addr;
	struct linger lin;
	int on=1;
	
	//��serv_addr�ĸ����ֶ�����
	memset(&serv_addr, 0x00, sizeof(struct sockaddr_in));

	//�����׽���
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		_swVdebug(0, "Creat Socket Error[%d]", errno);
		return (-1);
	}
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = inet_addr( alIp );
	serv_addr.sin_port = htons( ilPort );
	
	//ʹ��sockfd�׽������ӵ���serv_addrָ����Ŀ�ĵ�ַ��
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
/* ģ����    ��Mon_TcpSnd					*/
/* ģ������    �����ͱ��ĵ������                               */
/* �� �� ��    ��                                               */
/* ��    ��    ��Hu                                             */
/* ��������    ��2015/4/3 14:48:46				*/
/* ����޸����ڣ�                                               */
/* ģ����;    ��                                               */
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