struct tcpcfg
{
  char	aPartner_addr[21];  /*��������ַ            */
  char  a1;
  short	iPartner_port;      /*�������˿�            */
} sgTcpcfg;

int swLoadcomcfg( char *aPortName );
int swTcpsnd(int iSockfd,char *aBuffer,unsigned intiLen);
int swTcprcv(int iSockfd,char *aBuffer,unsigned int *piLength);
int swTcpconnect( int iPort );
int swConnsend(short iflag,int *iSockfd, char *aMsgbuf,unsigned int iMsglen);
int swConnectser(char *aIp,int iPort);
int swCheck_comm();
