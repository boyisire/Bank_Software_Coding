#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "swftp.h"

static int timeout=60;
static int isTimeOut=0;
static jmp_buf env_alrm;

static void DoTimeOut(int arg)
{
  isTimeOut = 1;
  longjmp(env_alrm,1);
  return;
}

static int InitCtrlConn(char *remhost);
static int InitDataConn();
static int DoUser(int sockfd, char *user);
static int DoPasswd(int sockfd, char *passwd);
static int DoPort(int sockfd, int listsd);
static int DoType(int sockfd, char type);
static int DoRetr(int sockfd, char *rempath, char *flag);
static int DoStor(int sockfd, char *locpath, char *flag);
static int EatAllMessages(int sockfd, char *buf);
static int ftp_accept(int lsndfd);
static int ftp_read(int fd,char *buf,int len);

/*
 * ftp_login
 */
int ftp_login(FTPINFO *ftpinfo, char *remhost, char *user, char *passwd, char *account)
{
  int sockfd,ilRc;
  char buf[BUFLEN+1];

  ftpinfo->loginok = 0;
  ftpinfo->sockfd = -1;
  ftpinfo->listsd = -1;
  ftpinfo->datasd = -1;
  ftpinfo->type = 'I';
  ftpinfo->debug = 0;

  sockfd = InitCtrlConn(remhost);
  if(sockfd<0) goto ErrReturn;
  
  ilRc = ftp_read(sockfd,buf,BUFLEN);
  if((ilRc<0)||isTimeOut) goto ErrReturn;
  buf[ilRc] = 0;
  if(strstr(buf,"220"))
  {
    ilRc = EatAllMessages(sockfd,buf);
    if(ilRc<0) goto ErrReturn;
  }
  else
    goto ErrReturn;
  
  ilRc = DoUser(sockfd,user);
  if(ilRc<0) goto ErrReturn;
  ilRc = DoPasswd(sockfd,passwd);
  if(ilRc<0) goto ErrReturn;
  
  ftpinfo->sockfd = sockfd; 
  ftpinfo->loginok = 1;
  return(0);
  
ErrReturn:
  if(sockfd>0) close(sockfd);
  return(-1);  
}

/*
 * ftp_putfile
 */
int ftp_putfile(FTPINFO *ftpinfo, char *rempath, char *locpath)
{
  int ilRc;
  char buf[BUFLEN+1],flag;
  int sockfd,datasd,listsd;
  FILE *fp;

  if(!ftpinfo->loginok)	return(-1);
  
  listsd = InitDataConn();
  if(listsd<0) return(-1);
  ftpinfo->listsd = listsd;

  sockfd = ftpinfo->sockfd;
  ilRc = DoPort(sockfd,listsd);
  if(ilRc<0) return(-1);

  ilRc = DoType(sockfd,ftpinfo->type);
  if(ilRc<0) return(-1);

  ilRc = DoStor(sockfd,rempath,&flag);
  if(ilRc<0) return(-1);
 
  datasd = ftp_accept(listsd);
  if(datasd<0||isTimeOut) return(-1);
  close(listsd);
  ftpinfo->listsd = -1;
  
  fp = fopen(locpath,"r");
  if(fp==NULL) return(-1);

  for(;;)
  { 
    if(feof(fp)) break;
    ilRc = fread(buf,1,sizeof(buf),fp);
    if(ilRc<=0)	break;
    ilRc = write(datasd,buf,ilRc);
    if(ilRc<=0) 
    {
      fclose(fp);
      return(-1);
    }
  }
  fclose(fp);
  close(datasd);
  ftpinfo->datasd = -1;

  if(!flag)
  {
    ilRc = ftp_read(sockfd,buf,BUFLEN);
    if(ilRc<0||isTimeOut) return(-1);
    buf[ilRc] = 0;
    if(strstr(buf,"226")||strstr(buf,"250"))
    {
      ilRc = EatAllMessages(sockfd,buf);
      if(ilRc<0) return(-1);
    }
    else
      return(-1);
  }

  return(0);
}

/*
 * ftp_getfile
 */
int ftp_getfile(FTPINFO *ftpinfo, char *rempath, char *locpath)
{
  int ilRc;
  char buf[BUFLEN+1],flag;
  int sockfd,datasd,listsd;
  FILE *fp;

  if(!ftpinfo->loginok)	return(-1);
  
  listsd = InitDataConn();
  if(listsd<0)	return(-1);
  ftpinfo->listsd = listsd;
  
  sockfd = ftpinfo->sockfd;
  ilRc = DoPort(sockfd,listsd);
  if(ilRc<0) return(-1);
  
  ilRc = DoType(sockfd,ftpinfo->type);
  if(ilRc<0) return(-1);
  
  ilRc = DoRetr(sockfd,rempath,&flag);
  if(ilRc<0) return(-1);
  
  datasd = ftp_accept(listsd);
  if(datasd<0||isTimeOut) return(-1);
  close(listsd);
  ftpinfo->listsd = -1;
  
  fp = fopen(locpath,"w");
  if(fp==NULL) return(-1);
 
  for(;;)
  {
    ilRc = ftp_read(datasd,buf,BUFLEN);
    if(ilRc<=0||isTimeOut) break;
    fwrite(buf,1,ilRc,fp);
  }  
  fclose(fp);
  close(datasd);
  ftpinfo->datasd = -1;

  if(!flag)
  {
    ilRc = ftp_read(sockfd,buf,BUFLEN);
    if(ilRc<0||isTimeOut) return(-1);
    buf[ilRc] = 0;
    if(strstr(buf,"226")||strstr(buf,"250"))
    {
      ilRc = EatAllMessages(sockfd,buf);
      if(ilRc<0) return(-1);
    }  
    else
      return(-1);
  }    

  return(0);
}

/*
 * ftp_ascii
 */
int ftp_ascii(FTPINFO *ftpinfo)
{
  if(!ftpinfo->loginok) return(-1);
  ftpinfo->type = 'A';
  return(0);
}   

/*
 * ftp_binary
 */
int ftp_binary(FTPINFO *ftpinfo)
{
  if(!ftpinfo->loginok) return(-1);
  ftpinfo->type = 'I';
  return(0);
}   

/*
 * ftp_timeout
 */
int ftp_timeout(FTPINFO *ftpinfo)
{
  if(!ftpinfo->loginok) return(-1);
  timeout = ftpinfo->timeout;
  return(0);
}

/*
 * ftp_bye
 */
int ftp_bye(FTPINFO *ftpinfo)
{
  if(!ftpinfo->loginok) return(-1);
  if(ftpinfo->sockfd>0) close(ftpinfo->sockfd);
  if(ftpinfo->listsd>0) close(ftpinfo->listsd);
  if(ftpinfo->datasd>0) close(ftpinfo->datasd);
  return(0);
}

/*
 * EatAllMessages
 */
int EatAllMessages(int sockfd, char *buf)
{
  int ilRc;
  char code[5];

  if(buf[3]!='-') return(0);
  memcpy(code,buf,3);
  code[3] = 0;
  strcat(code," ");
  while(!strstr(buf,code))
  {
    ilRc = ftp_read(sockfd,buf+strlen(buf),BUFLEN);
    if(ilRc<0||isTimeOut) return(-1);
    if(ilRc==0) break;
  }
  return(0);
}

/*
 * InitCtrlConn
 */
int InitCtrlConn(char *remhost)
{
  int sockfd;
  struct hostent *hostent;
  struct servent *servent;
  struct sockaddr_in sockaddr;

  bzero(&sockaddr,sizeof(struct sockaddr_in));
  sockaddr.sin_family = AF_INET;

  if((hostent = gethostbyname(remhost))!=NULL)
    memcpy(&sockaddr.sin_addr.s_addr,hostent->h_addr_list[0],hostent->h_length);
  else
    return(-1);
 
  if((servent = getservbyname("ftp","tcp"))!=NULL)
    sockaddr.sin_port = servent->s_port;
  else
    return(-1);

  if((sockfd = socket(AF_INET,SOCK_STREAM,0))<= 0)
    return(-1);
  if(connect(sockfd,(struct sockaddr *)&sockaddr,sizeof(sockaddr))<0)
  {
    close(sockfd);
    return(-1);
  }
  return(sockfd);
}

/*
 * InitDataConn
 */
int InitDataConn()
{
  int sockfd;
  char hostname[100];
  struct hostent *hostent;
  struct sockaddr_in sockaddr;

  bzero(&sockaddr,sizeof(struct sockaddr_in));
  sockaddr.sin_family = AF_INET;

  if(gethostname(hostname,sizeof(hostname))) return(-1);
  if((hostent = gethostbyname(hostname))==NULL) return(-1);
  memcpy(&sockaddr.sin_addr.s_addr,hostent->h_addr_list[0],hostent->h_length);
  sockaddr.sin_port = 0;

  sockfd = socket(AF_INET,SOCK_STREAM,0);
  if(sockfd<0) return(-1);
 
  if(bind(sockfd,(struct sockaddr *)&sockaddr,sizeof(sockaddr))<0)
  {
    close(sockfd);
    return(-1);
  }

  if(listen(sockfd,1)<0)
  {
    close(sockfd);
    return(-1);
  }

  return(sockfd);
}

/*
 * DoUser
 */
int DoUser(int sockfd, char *user)
{
  int ilRc;
  char buf[BUFLEN+1];

  sprintf(buf,"USER %s\r\n",user);
  ilRc = write(sockfd,buf,strlen(buf));
  if(ilRc<0) return(-1);

  ilRc = ftp_read(sockfd,buf,BUFLEN);
  if(ilRc<0||isTimeOut) return(-1);
  buf[ilRc] = 0;
  if(strstr(buf,"230"))
  {
    ilRc = EatAllMessages(sockfd,buf);
    if(ilRc< 0) return(-1);
  }
  else if(!strstr(buf,"331"))
    return(-1);

  return(0);
}

/* 
 * DoPasswd
 */
int DoPasswd(int sockfd, char *passwd)
{
  int ilRc;
  char buf[BUFLEN+1]; 
  
  sprintf(buf,"PASS %s\r\n",passwd);
  ilRc = write(sockfd,buf,strlen(buf));
  if(ilRc<0) return(-1);
  ilRc = ftp_read(sockfd,buf,BUFLEN);
  if(ilRc<0||isTimeOut) return(-1);
  buf[ilRc] = 0;
  if(strstr(buf,"230"))
  {
    ilRc = EatAllMessages(sockfd,buf);
    if(ilRc<0) return(-1);
  }
  else if(!strstr(buf,"332"))
    return(-1);

  return(0);
}  

/*
 * DoPort
 */
int DoPort(int sockfd, int listsd)
{
  int ilRc;
  int s1,s2,s3,s4,port;
  char addr[20],buf[BUFLEN+1];
#ifdef OS_SCO
  int len;
#else
  socklen_t len;
#endif
  struct sockaddr_in sockaddr;

  len = sizeof(struct sockaddr_in);  
  ilRc = getsockname(listsd,(struct sockaddr*)&sockaddr,&len);
  if(ilRc<0) return(-1);

  strcpy(addr,(char *)inet_ntoa(sockaddr.sin_addr));
  port = sockaddr.sin_port;
  sscanf(addr,"%d.%d.%d.%d",&s1,&s2,&s3,&s4);
#ifdef OS_AIX
  sprintf(buf,"PORT %d,%d,%d,%d,%d,%d\r\n",
          s1,s2,s3,s4,(port&0xff00)>>8,(port&0xff));
#else
  sprintf(buf,"PORT %d,%d,%d,%d,%d,%d\r\n",
          s1,s2,s3,s4,(port&0xff),(port&0xff00)>>8);
#endif
  ilRc = write(sockfd,buf,strlen(buf));
  if(ilRc<0) return(-1);
  ilRc = ftp_read(sockfd,buf,BUFLEN);
  if(ilRc<0||isTimeOut) return(-1);
  buf[ilRc] = 0;
  if(strstr(buf,"200"))
  {
    ilRc = EatAllMessages(sockfd,buf);
    if(ilRc<0) return(-1);
  }
  else
    return(-1);
  return(0);
}  

/*
 * DoType
 */  
int DoType(int sockfd, char type)
{ 
  int ilRc;
  char buf[BUFLEN+1]; 
  
  sprintf(buf,"TYPE %c\r\n",type);
  ilRc = write(sockfd,buf,strlen(buf));
  if(ilRc<0) return(-1);
  ilRc = ftp_read(sockfd,buf,BUFLEN);
  if(ilRc<0||isTimeOut) return(-1);
  buf[ilRc] = 0;
  if(strstr(buf,"200"))
  {
    ilRc = EatAllMessages(sockfd,buf);
    if(ilRc<0) return(-1);
  }
  else if(!strstr(buf,"332"))
    return(-1);

  return(0);
}  

/* 
 * DoRetr
 */ 
int DoRetr(int sockfd, char *rempath, char *flag)
{ 
  int ilRc;
  char buf[BUFLEN+1]; 
  
  sprintf(buf,"RETR %s\r\n",rempath);
  ilRc = write(sockfd,buf,strlen(buf));
  if(ilRc<0) return(-1);
  ilRc = ftp_read(sockfd,buf,BUFLEN);
  if(ilRc<0||isTimeOut) return(-1);
  buf[ilRc] = 0;
  if(strstr(buf,"150"))
  {
    if(strstr(buf,"226")||strstr(buf,"250"))
      *flag = 1;
    else
      *flag = 0;
    return(0);
  }
  else
    return(-1);
}

/* 
 * DoStor
 */
int DoStor(int sockfd, char *rempath, char *flag)
{ 
  int ilRc;
  char buf[BUFLEN+1]; 
  
  sprintf(buf,"STOR %s\r\n",rempath);
  ilRc = write(sockfd,buf,strlen(buf));
  if(ilRc<0) return(-1);

  ilRc = read(sockfd,buf,BUFLEN);
  if(ilRc<0||isTimeOut) return(-1);
  buf[ilRc] = 0;
  printf("DoStor,buf=%s\n",buf);
  if(strstr(buf,"150"))
  {
    if(strstr(buf,"226")||strstr(buf,"250"))
      *flag = 1;
    else
      *flag = 0;
    return(0);
  }
  else
    return(-1);
}

/*
 * ftp_accept
 */
int ftp_accept(int lsndfd)
{
  int ilRc;

  signal(SIGALRM,DoTimeOut);
  if(setjmp(env_alrm)!=0)
  {
    fprintf(stdout,"accept timeout\n");
    return(-1);
  }
  alarm(timeout);
  ilRc = accept(lsndfd,NULL,NULL);
  alarm(0);
  return(ilRc);
}

/* 
 * ftp_read
 */
int ftp_read(int fd,char *buf,int len)
{
  int ilRc;

  signal(SIGALRM,DoTimeOut);
  if(setjmp(env_alrm)!=0)
  {
    fprintf(stdout,"read timeout\n");
    return(-1);
  }
  alarm(timeout);
  ilRc = read(fd,buf,len);
  alarm(0);
  return(ilRc);
}
