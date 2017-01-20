/****************************************************************/
/* ģ����	��BMQCPMMPUB                                        */
/* ģ������	���³�Q �ڲ����ú�����                              */
/* �� �� ��	��V2.2                                              */
/* ��    ��	�����                                              */
/* ��������	��2006/11/26                                        */
/* ����޸�����	��2006/12/31                                    */
/* ģ����;	���³�Q��������API����                              */
/* ��ģ���а������º���������˵��:                              */
/*  (1)  int _bmqGettime()             ����                     */
/****************************************************************/

/****************************************************************/
/* �޸ļ�¼                                                     */
/****************************************************************/
#include	"bmq.h"
#include	<setjmp.h>

#define byte unsigned char

int     igFlag;
int	    _bmqGetfield1();
void    _bmqTimeout1();
int     _bmqFileSrvCfg_load(short iGrpid,char *aHostIP,int *iPort);
int     compress(void *src, unsigned src_len, void *dst);
int     decompress(void *src, unsigned src_len,	void *dst);
int     _bmqcompress(void *src, unsigned src_len, void *dst ,char *press);
int     _bmqdecompress(void *src, unsigned src_len,	void *dst,char *press);

static  jmp_buf jmpbuffer;

int  _bmqGettime(date)
char    *date;
{
  long    Time;
  struct tm *T;
  char    tt[10];
  
  time(&Time);
  T=localtime(&Time);
  sprintf(date, "%04d-%02d-%02d",T->tm_year+1900,T->tm_mon+1,T->tm_mday);
  sprintf(tt, " %02d:%02d:%02d", T->tm_hour, T->tm_min, T->tm_sec);
  strcat(date, tt);
  return(0);
}

int  _bmqTimeBuf(time_t * lt,char *timebuf)
{
  struct tm *T;

  T = localtime(lt);
  
  sprintf(timebuf, "%04d-%02d-%02d %02d:%02d:%02d",
                    T->tm_year+1900,T->tm_mon+1,T->tm_mday,
                    T->tm_hour, T->tm_min, T->tm_sec);
  return(0);
}

void _bmqSignalinit()
{
  signal(SIGQUIT,SIG_IGN);
  signal(SIGHUP,SIG_IGN);
  signal(SIGINT,SIG_IGN);
  signal(SIGTSTP,SIG_IGN);
  signal(SIGSTOP,SIG_IGN);
  signal(SIGUSR1,SIG_IGN);
  signal(SIGUSR2,SIG_IGN);
  signal(SIGCHLD,SIG_IGN);
  signal(SIGTTOU,SIG_IGN);
  signal(SIGTTIN,SIG_IGN);
}

/**************************************************************
 ** ������      : _bmqDebug
 ** ��  ��      : �Ǹ�����־:��ӡָ����Ϣ
 ** ��  ��      : ʷ����
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������: 
 ** ȫ�ֱ���    : 
 ** ��������    : frm,va_alist :�÷���ʽͬprintf
 ** ����ֵ      : FAIL:������־�ļ�����. SUCCESS:�ɹ�
***************************************************************/
#ifdef _LINUXES_
int _bmqDebug(char *frm, ...)
#else
int _bmqDebug(frm, va_alist)
char *frm;
va_dcl
#endif
{
  FILE *fp;
  char buf[iMBMAXPACKSIZE],fname1[100],buf1[iMBMAXPACKSIZE];
  va_list  ap;

  memset(buf,0x00,1024);
  memset(buf1,0x00,1024);
  memset(fname1,0x00,100);
  if (*frm) 
  {
#ifdef _LINUXES_
    va_start(ap,frm);
#else
    va_start(ap);
#endif
    vsprintf(buf, frm, ap);
    va_end(ap);
  }

  sprintf(fname1,"%s/bmqlog/debug/bmq%d.log",getenv("BMQ_PATH"),getpid());
  if (( fp = fopen(fname1,"a+")) == NULL)  return(-1023);
  _bmqGettime(buf1);
  strncat(buf1,": ",2);
  strncat(buf1,buf,strlen(buf));
  fprintf(fp,"%s\n",buf1+11);
  fclose(fp);
  return(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqVdebug
 ** ��  ��      : �Ǹ�����־:��ӡָ����Ϣ
 ** ��  ��      : 
 ** ��������    : 2000/10/25
 ** ����޸�����:
 ** ������������: 
 ** ȫ�ֱ���    : 
 ** ��������    : frm,va_alist :�÷���ʽͬprintf
 ** ����ֵ      : FAIL:������־�ļ�����. SUCCESS:�ɹ�
***************************************************************/
#ifdef _LINUXES_
int _bmqVdebug(short iDebug,char *frm, ...)
#else
int _bmqVdebug(short iDebug,frm, va_alist)
char *frm;
va_dcl
#endif
{
  FILE *fp;
  char buf[iMBMAXPACKSIZE],fname1[100],buf1[iMBMAXPACKSIZE];
  va_list  ap;

  if ( igDebug < iDebug ) return(0);
  	
  memset(buf,0x00,1024);
  memset(buf1,0x00,1024);
  memset(fname1,0x00,100);
  if (*frm) 
  {
#ifdef _LINUXES_
    va_start(ap,frm);
#else
    va_start(ap);
#endif
    vsprintf(buf, frm, ap);
    va_end(ap);
  }

  sprintf(fname1,"%s/bmqlog/debug/bmq%d.log",getenv("BMQ_PATH"),getpid());
  if (( fp = fopen(fname1,"a+")) == NULL)  return(-1023);
  _bmqGettime(buf1);
  strncat(buf1,": ",2);
  strncat(buf1,buf,strlen(buf));
  fprintf(fp,"%s\n",buf1+11);
  fclose(fp);
  return(SUCCESS);
}

/**************************************************************
 ** ������      : _bmqDebughex
 ** ��  ��      : ��ӡʮ�����Ƶ���־�ļ� 
 ** ��  ��      : 
 ** ��������    : 1999/11/19
 ** ����޸�����: 2000/3/10
 ** ������������: _bmqGettime
 ** ȫ�ֱ���    :
 ** ��������    : buff  :����ı���
 **               buflen:���ĳ���
 ** ����ֵ      : -1:�ļ��򿪳���  0:�ɹ�
***************************************************************/
/*int _bmqDebughex(char * buff,short buflen) delete by wh*/
int _bmqDebughex(char * buff,TYPE_SWITCH buflen)/*add by wh*/
{
  char fname1[80], buf[100];
  FILE *fp;
  TYPE_SWITCH i,g,k;
  unsigned char *abuf,*hbuf;

  _bmqGettime(buf);
  memset(fname1,0x00,80);
  sprintf(fname1,"%s/bmqlog/debug/bmq%d.log",getenv("BMQ_PATH"),getpid());
  if ( (fp=fopen(fname1,"a+")) == NULL) return(-1);

  hbuf=(unsigned char *)buff;
  abuf=(unsigned char *)buff;
  fprintf(fp,"%s *** ʮ�����Ʊ���(��ʼ) ***\n",buf + 5);
  for(i=0, g=buflen/16; i < g; i++)
  {
    fprintf(fp, "M(%6.6d)=< ",i*16);
    for(k=0; k<16; k++) fprintf(fp, "%02x ",*hbuf++);
    fprintf(fp,"> ");
    for(k=0; k<16; k++, abuf++)
      fprintf(fp, "%c",(*abuf>32) ? ((*abuf<128) ? *abuf : '*') : '.');
    fprintf(fp,"\n");
  }
  if((i=buflen%16) > 0)
  {
    fprintf(fp,"M(%6.6d)=< ",buflen-buflen%16);
    for(k=0; k < i; k++) fprintf(fp, "%02x ",*hbuf++);
    for(k=i; k < 16; k++) fprintf(fp, "   ");
    fprintf(fp, "> ");
    for(k=0; k < i; k++, abuf++)
      fprintf(fp, "%c",(*abuf>32) ? ((*abuf<128) ? *abuf : '*') : '.');
    fprintf(fp, "\n");
  }
  fprintf(fp,"                    *** ʮ�����Ʊ���(����) ***\n");
  fflush(fp);
  fclose(fp);
  return(0);
}

/**************************************************************
 ** ������: _bmqMac
 ** ����:   ����MAC����
 ** ����:   ���
 ** ��������: 2006/11/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ:
***************************************************************/
void _bmqMac( char *orgbuf, int buflen, char *macbuf )
{
  int	loop;
 
  /* ���MAC */ 
  for( loop = 0; loop < MAC_LEN; loop ++ )
    macbuf[loop] = orgbuf[(buflen/MAC_LEN)*loop];
}

/**************************************************************
 ** ������: _do_compress(),compress(),decompress()
 ** ����:   ѹ��,��ѹ������
 ** ����:   ���
 ** ��������: 2003/10/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:        
 ** ����ֵ:
***************************************************************/
static unsigned _do_compress (byte *in, unsigned in_len, byte *out, unsigned *out_len)
{
	static long wrkmem [16384L];
    register byte *ip;
    byte *op;
	byte *in_end = in + in_len;
    byte *ip_end = in + in_len - 13;
    byte *ii;
    byte **dict = (byte **)wrkmem;
    op = out;
	ip = in;
	ii = ip;
    ip += 4;
    for(;;)
	{
		register byte *m_pos;
		unsigned m_off;
		unsigned m_len;
		unsigned dindex;
		dindex = ((0x21*(((((((unsigned)(ip[3])<<6)^ip[2])<<5)^ip[1])<<5)^ip[0]))>>5) & 0x3fff;
		m_pos = dict [dindex];
		if(((unsigned)m_pos < (unsigned)in) ||
			(m_off = (unsigned)((unsigned)ip-(unsigned)m_pos) ) <= 0 ||
			m_off > 0xbfff)
			goto literal;
		if(m_off <= 0x0800 || m_pos[3] == ip[3])
			goto try_match;
		dindex = (dindex & 0x7ff ) ^ 0x201f;
		m_pos = dict[dindex];
		if((unsigned)(m_pos) < (unsigned)(in) ||
			(m_off = (unsigned)( (int)((unsigned)ip-(unsigned)m_pos))) <= 0 ||
			m_off > 0xbfff)
		    goto literal;
		if (m_off <= 0x0800 || m_pos[3] == ip[3])
			goto try_match;
		goto literal;
try_match:
		if(*(unsigned short*)m_pos == *(unsigned short*)ip && m_pos[2]==ip[2])
			goto match;
literal:
		dict[dindex] = ip;
		++ip;
		if (ip >= ip_end)
			break;
		continue;
match:
		dict[dindex] = ip;
		if(ip - ii > 0)
		{
			register unsigned t = ip - ii;
			
			if (t <= 3)
				op[-2] |= (byte)t;
			else if(t <= 18)
				*op++ = (byte)(t - 3);
			else
			{
				register unsigned tt = t - 18;
				*op++ = 0;
				while(tt > 255)
				{
					tt -= 255;
					*op++ = 0;
				}
				*op++ = (byte)tt;
			}
			do *op++ = *ii++; while (--t > 0);
		}
		ip += 3;
		if(m_pos[3] != *ip++ || m_pos[4] != *ip++ || m_pos[5] != *ip++ ||
			m_pos[6] != *ip++ || m_pos[7] != *ip++ || m_pos[8] != *ip++ )
		{
			--ip;
			m_len = ip - ii;
			
			if(m_off <= 0x0800 )
			{
				--m_off;
				*op++ = (byte)(((m_len - 1) << 5) | ((m_off & 7) << 2));
				*op++ = (byte)(m_off >> 3);
			}
			else
				if (m_off <= 0x4000 )
				{
					-- m_off;
					*op++ = (byte)(32 | (m_len - 2));
					goto m3_m4_offset;
				}
				else
				{
					m_off -= 0x4000;
					*op++ = (byte)(16 | ((m_off & 0x4000) >> 11) | (m_len - 2));
					goto m3_m4_offset;
				}
		}
		else
		{
			{
				byte *end = in_end;
				byte *m = m_pos + 9;
				while (ip < end && *m == *ip)
					m++, ip++;
				m_len = (ip - ii);
			}
			
			if(m_off <= 0x4000)
			{
				--m_off;
				if (m_len <= 33)
					*op++ = (byte)(32 | (m_len - 2));
				else
				{
					m_len -= 33;
					*op++=32;
					goto m3_m4_len;
				}
			}
			else
			{
				m_off -= 0x4000;
				if(m_len <= 9)
					*op++ = (byte)(16|((m_off & 0x4000) >> 11) | (m_len - 2));
				else
				{
					m_len -= 9;
					*op++ = (byte)(16 | ((m_off & 0x4000) >> 11));
m3_m4_len:
					while (m_len > 255)
					{
						m_len -= 255;
						*op++ = 0;
					}
					*op++ = (byte)m_len;
				}
			}
m3_m4_offset:
			*op++ = (byte)((m_off & 63) << 2);
			*op++ = (byte)(m_off >> 6);
		}
		ii = ip;
		if (ip >= ip_end)
			break;
    }
    *out_len = op - out;
    return (unsigned) (in_end - ii);
}
int compress(void *in, unsigned in_len,
			 void *out)
{
    byte *op = out;
    unsigned t,out_len;

    if (in_len <= 13)
		t = in_len;
    else 
	{
		t = _do_compress (in,in_len,op,&out_len);
		op += out_len;
    }
    if (t > 0)
	{
		byte *ii = (byte*)in + in_len - t;
		if (op == (byte*)out && t <= 238)
			*op++ = (byte) ( 17 + t );
		else
			if (t <= 3)
				op[-2] |= (byte)t ;
			else
				if (t <= 18)
					*op++ = (byte)(t-3);
				else
				{
					unsigned tt = t - 18;
					*op++ = 0;
					while (tt > 255) 
					{
						tt -= 255;
						*op++ = 0;
					}
					*op++ = (byte)tt;
				}
				do *op++ = *ii++; while (--t > 0);
    }
    *op++ = 17;
    *op++ = 0;
    *op++ = 0;
    return (op - (byte*)out);
}
int decompress (void *in, unsigned in_len,
				void *out)
{
    register byte *op;
    register byte *ip;
    register unsigned t;
    register byte *m_pos;
    byte *ip_end = (byte*)in + in_len;

    op = out;
    ip = in;
    if(*ip > 17)
	{
		t = *ip++ - 17;
		if (t < 4)
			goto match_next;
		do *op++ = *ip++; while (--t > 0);
		goto first_literal_run;
    }
    for(;;)
	{
		t = *ip++;
		if (t >= 16) goto match;
		if (t == 0)
		{
			while (*ip == 0)
			{
				t += 255;
				ip++;
			}
			t += 15 + *ip++;
		}
		* (unsigned *) op = * ( unsigned *) ip;
		op += 4; ip += 4;
		if (--t > 0)
		{
			if (t >= 4)
			{
				do
				{
					* (unsigned * ) op = * ( unsigned * ) ip;
					op += 4; ip += 4; t -= 4;
				} while (t >= 4);
				if (t > 0) do *op++ = *ip++; while (--t > 0);
			}
			else
				do *op++ = *ip++; while (--t > 0);
		}
first_literal_run:
		t = *ip++;
		if (t >= 16)
			goto match;
		m_pos = op - 0x0801;
		m_pos -= t >> 2;
		m_pos -= *ip++ << 2;
		*op++ = *m_pos++; *op++ = *m_pos++; *op++ = *m_pos;
		goto match_done;
		for(;;)
		{
match:
		if (t >= 64)
		{
			m_pos = op - 1;
			m_pos -= (t >> 2) & 7;
			m_pos -= *ip++ << 3;
			t = (t >> 5) - 1;
			goto copy_match;
		}
		else 
			if (t >= 32)
			{
				t &= 31;
				if (t == 0)	
				{
					while (*ip == 0) 
					{
						t += 255;
						ip++;
					}
					t += 31 + *ip++;
				}
				m_pos = op - 1;
				m_pos -= (* ( unsigned short * ) ip) >> 2;
				ip += 2;
			}
			else
				if (t >= 16) 
				{
					m_pos = op;
					m_pos -= (t & 8) << 11;
					t &= 7;
					if (t == 0)
					{
						while (*ip == 0)
						{
							t += 255;
							ip++;
						}
						t += 7 + *ip++;
					}
					m_pos -= (* ( unsigned short *) ip) >> 2;
					ip += 2;
					if (m_pos == op)
						goto eof_found;
					m_pos -= 0x4000;
				}
				else 
				{
					m_pos = op - 1;
					m_pos -= t >> 2;
					m_pos -= *ip++ << 2;
					*op++ = *m_pos++; *op++ = *m_pos;
					goto match_done;
				}
				if (t >= 6 && (op - m_pos) >= 4) 
				{
					* (unsigned *) op = * ( unsigned *) m_pos;
					op += 4; m_pos += 4; t -= 2;
					do
					{
						* (unsigned *) op = * ( unsigned *) m_pos;
						op += 4; m_pos += 4; t -= 4;
					}while (t >= 4);
					if (t > 0)
						do *op++ = *m_pos++; while (--t > 0);
				}
				else
				{
copy_match:
				*op++ = *m_pos++; *op++ = *m_pos++;
				do *op++ = *m_pos++; while (--t > 0);
				}
match_done:
				t = ip[-2] & 3;
				if (t == 0)	break;
match_next:
				do *op++ = *ip++; while (--t > 0);
				t = *ip++;
		}
   }
eof_found:
   if (ip != ip_end) return -1;
   return (op - (byte*)out);
}


void timeout_process()
{
  signal(SIGALRM,SIG_IGN);
  alarm(0);
  longjmp(jmpbuffer,1);
}

/**************************************************************
 ** ������: _comTcpSend
 ** ����:   ��������
 ** ����:   
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--������Ϣ�ĵ�ַ
             length--������Ϣ�ĳ���          
 ** ����ֵ: 0--�ɹ���-1031--ʧ��
***************************************************************/
/*int _comTcpSend(sockfd, buffer, length)
 int             sockfd;
 unsigned char   *buffer;
 short           *length;delete by wh*/
int _comTcpSend(sockfd, buffer, length)
 int             sockfd;
 unsigned char   *buffer;
 TYPE_SWITCH *length;/*add by wh*/
{
  /*short   len=-1;delete by wh*/
  TYPE_SWITCH len=-1;/*add by wh*/
  /*short   ilSendlen=0;delete by wh*/
  TYPE_SWITCH ilSendlen=0;/*add by wh*/
  struct  timeval  timev;
 
  timev.tv_sec = 60;
  timev.tv_usec = 0;
  setsockopt(sockfd,SOL_SOCKET,SO_SNDTIMEO,&timev,sizeof(timev));

  while( ilSendlen != *length )
  {
    if ( ( len = send(sockfd,buffer+ilSendlen,*length-ilSendlen,0)) > 0 )
    {
      ilSendlen += len;
    }
    else
    {
      _bmqDebug("S0010: _bmqGrpSend error! errno:%d,%s",errno,strerror(errno));
      return(-1031);
    }
  }
  *length = ilSendlen;
  return ( SUCCESS );
}

/**************************************************************
 ** ������: _comTcpReceive1
 ** ����:   ��������
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--�洢��Ϣ�ĵ�ַ
             length--�����������ߴ�
 ** ����ֵ: 0--�ɹ���-1041--ʧ��
***************************************************************/
/*int _comTcpReceive1( sockfd, buffer, length, timeout)
  int     sockfd;
  unsigned char *buffer;
  short   *length;
  int     timeout;  delete by wh*/
int _comTcpReceive1( sockfd, buffer, length, timeout)
  int     sockfd;
  unsigned char *buffer;
  TYPE_SWITCH *length;
  int     timeout; /*add by wh*/
{
  int   ilRn = -1040;
  /*short ilRcvlen=0;delete by wh*/
  TYPE_SWITCH ilRcvlen=0;/*add by wh*/
  int   ilLen;

  signal(SIGALRM,timeout_process);
  alarm(10);

  if(setjmp(jmpbuffer))
  {
    return(-1026);
  }

  memset (buffer, 0x00, *length);
  ilLen = *length;
  for(;;)
  {
     ilRcvlen=recv(sockfd,buffer,ilLen,MSG_EOR);
     if (ilRcvlen > 0)
     {
       if (ilRcvlen != ilLen)
       {
         ilLen = ilLen - ilRcvlen;
         buffer = buffer + ilRcvlen;
         continue;
       }
       else
       {
         ilRn = 0;
         break;
       }
     }
     else if(ilRcvlen == 0)
     {
       ilRn = 1040;
       if(igDebug >= 1)
         _bmqDebug("S0020: _comTcpReceive() error=[%d][%s]",errno,strerror(errno));
       break;
     }
     else
     {
       _bmqDebug("S0030: _comTcpReceive error! errno:%d,%s",errno,strerror(errno));
       ilRn = 1040;
       break;
     }
  }
  if(timeout > 0)
  {
    alarm(0);
    signal(SIGALRM,SIG_DFL);
  }
  return ( ilRn );
}

/**************************************************************
 ** ������: _comTcpReceive
 ** ����:   ��������
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--�洢��Ϣ�ĵ�ַ
             length--�����������ߴ�
 ** ����ֵ: 0--�ɹ���-1041--ʧ��
***************************************************************/
/*int _comTcpReceive( sockfd, buffer, length)
  int     sockfd;
  unsigned char *buffer;
  short   *length;  delete by wh*/
int _comTcpReceive( sockfd, buffer, length)
  int     sockfd;
  unsigned char *buffer;
  TYPE_SWITCH *length;/*add by wh*/
{
  int   ilRn = -1040;
  /*short ilRcvlen=0; delete by wh*/
  TYPE_SWITCH ilRcvlen=0;
  int   ilLen;

  memset (buffer, 0x00, *length);
  ilLen = *length;
  for(;;)
  {
     if ( ( ilRcvlen=recv(sockfd,buffer,ilLen,MSG_EOR)) > 0 )
     {
       if (ilRcvlen != ilLen)
       {
       	 ilLen = ilLen - ilRcvlen;
       	 buffer = buffer + ilRcvlen;
       	 continue; 
       }
       else
         return(0);
     }
     else if(ilRcvlen == 0)
     {
       ilRn = 1040;
       if(igDebug >= 1)
         _bmqDebug("S0040: _comTcpReceive() error=[%d][%s]",errno,strerror(errno));
     }
     else
       _bmqDebug("S0050: _comTcpReceive error! errno:%d,%s",errno,strerror(errno));
     break;
  }   
  return ( ilRn );
}

/**************************************************************
 ** ������: _bmqRecvconf
 ** ����:   ����ȷ������
 ** ����:   ���
 ** ��������: 2002/07/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--�洢��Ϣ�ĵ�ַ
             length--�����������ߴ�
 ** ����ֵ: 0--�ɹ���-1041--ʧ��
***************************************************************/
/*int _bmqRecvconf(sockfd,buffer,length)
  int  sockfd;
  char *buffer;
  short   *length;delete by wh*/
int _bmqRecvconf(sockfd,buffer,length)
  int  sockfd;
  char *buffer;
  TYPE_SWITCH *length; /*add by wh*/
{
  int ilRc = -1041;
  /*short ilRcvlen=0; delete by wh*/
  TYPE_SWITCH ilRcvlen=0;

  /*��������ʱ��*/
  signal(SIGALRM,_bmqTimeout1);
  alarm(60);
  igFlag = 0;

  ilRcvlen=recv(sockfd,buffer,*length,MSG_EOR);
  if (igFlag == 1) return(1016);

  alarm(0);
  signal(SIGALRM,SIG_DFL);
  
  if ( ilRcvlen > 0 )
  {
    ilRc  = 0;
  }
  else
  {
    _bmqDebug("S0060: _bmqRecvconf()����[%d]:%d,%s",
                 ilRcvlen,errno,strerror(errno));
    return ( ilRc );
  }
  return ( ilRc );
}

/**************************************************************
 ** ������: _comGrpSend
 ** ����:   ��������
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--������Ϣ�ĵ�ַ
             length--������Ϣ�ĳ���
 ** ����ֵ: 0--�ɹ���-1031--ʧ��
***************************************************************/
/*int _bmqGrpSend(sockfd, buffer, length)
 int             sockfd;
 char   *buffer;
 short           *length; delete by wh*/
int _bmqGrpSend(sockfd, buffer, length)
 int             sockfd;
 char   *buffer;
 TYPE_SWITCH *length; /*add by wh*/
{
  int     ret;
  char    buf[6];
  /*short   len=-1;delete by wh*/
  TYPE_SWITCH len=-1;/*add by wh*/
  /*short   ilSendlen=0; delete by wh*/
  TYPE_SWITCH ilSendlen=0;/*add by wh*/

  while( ilSendlen != *length )
  {
    if ( ( len = send(sockfd,buffer+ilSendlen,*length-ilSendlen,0)) > 0 )
    {
      ilSendlen += len;
    }
    else
    {
      _bmqDebug("S0260: _bmqGrpSend error! errno:%d,%s",errno,strerror(errno));       return(-1031);
    }
  }
  *length = ilSendlen;
  
  /* receive ack */
  len = 5;
  ret = _comTcpReceive(sockfd,buf,&len);
  if(ret) return(FAIL);
  if(memcmp(buf,"AAAAA",5))
  { 
    _bmqDebug("S0112 Grp_rcv MAC ERROR!");
    return(FAIL);
  }
  return ( SUCCESS );
}

/**************************************************************
 ** ������: _bmqGrpRecv
 ** ����:   ��������
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:sockfd--�׽��������� buffer--�洢��Ϣ�ĵ�ַ
             length--�����������ߴ�
 ** ����ֵ: 0--�ɹ���-1041--ʧ��
***************************************************************/
/*int _bmqGrpRecv( sockfd, buffer, length)
  int     sockfd;
  char *buffer;
  short   *length; delete by wh*/
int _bmqGrpRecv( sockfd, buffer, length)
  int     sockfd;
  char *buffer;
  TYPE_SWITCH *length; /* add by wh*/
{
  /*short len=0,ilRcvlen=0; delete by wh*/
  TYPE_SWITCH len=0,ilRcvlen=0; /*add by wh*/

  memset (buffer, 0x00, *length);
  while(ilRcvlen != *length)
  {
    if ( ( len=recv(sockfd,buffer+ilRcvlen,*length-ilRcvlen,0)) > 0 )
    {
      ilRcvlen += len;
    }
    else
    {
      _bmqDebug("S0160: _bmqGrpRecv error! errno:%d,%s",errno,strerror(errno));
      return(-1040);
    }
  }
  return ( SUCCESS );
}



void _bmqTimeout1()
{
  igFlag = 1;
  alarm(0);
  signal(SIGALRM,SIG_DFL);
}

/**************************************************************
 ** ������: _bmqGetfield,_bmqGetfield1
 ** ����:   ��ָ���ļ��ж���ָ����ֵ�Ķ�������
 ** ����:
 ** ��������: 2001/08/08
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:fname--�ļ���  key--��ֵ  key_n--����
             field--�洢������
 ** ����ֵ: 0--�ɹ���-1031--ʧ��
***************************************************************/
int _bmqGetfield(char *fname,char *key,int key_n,char *field)
{
  char	buf[256],str[64];
  FILE	*fp;

  fp=fopen(fname,"r");
  if (fp == NULL) {
    field[0] = '\0';
    return(-1021);
  }
  while(fgets(buf,240,fp) != NULL){
    _bmqGetfield1(buf, 1, str);
    if(!strcmp(key,str)) {
      _bmqGetfield1(buf, key_n, str);
      strcpy(field, str);
      fclose(fp);
      return(SUCCESS);
    }
  }
  fclose(fp);
  field[0] = '\0';
  return(-1091);
}

int _bmqGetfield1(char *Buffer,int n, char *Field)
{
  int i,len;
  char *p, *begin;

  i = 0 ;
  p = Buffer;

  while ( *p != '\0' &&  i != n-1) {
    if ( (*p == ' ') || (*p == '\t') )
    {
      while((*(p+1)==' ')||(*(p+1)=='\t')) p++;
      i++;
    }
    p++;
  }
  if ( *p == '\0') {
    Field[0] ='\0';
    return(SUCCESS);
  }
  begin = p;
  while ( *p != '\0' &&  *p != ' ' && *p != '\t') p++;
  len = p - begin;
  if ( len != 0 ){
    memcpy(Field, begin, len);
    Field[len] = '\0';
    return(len);
  }

  else {
    Field[0] ='\0';
    return(SUCCESS);
  }
}

/* add by xujun 2002.12.20 for file trans begin */
/**************************************************************
 ** ������: _bmqConnectfileser
 ** ����:    �����ļ����������
 ** ����:    ��� 	
 ** ��������: 2002/12/20
 ** ����޸�����:2006/11/15(���ӶԿ����֧��)
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: 0--�ɹ�
***************************************************************/
int _bmqConnectfileser(short iGrpid)
{
  int		ilFileport;
  int		ilRc,ilSockfd;
#ifdef OS_SCO
  int		ilAddrlen;
#else
  socklen_t	ilAddrlen;
#endif  
  struct	linger		ilLinger;
  struct	sockaddr_in	slServ_addr,slCli_addr;
  char		alVal[16],alResult[100],alMbhost[100],alInitfile[100];

  memset(alVal,0x00,sizeof(alVal));

  if( iGrpid == 0)/*����*/
  {
    /*�Ӳ����ļ���ȡ�������� IP��ַ��ͨѶ�˿ں�*/
    sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
    ilRc = _bmqGetfield(alInitfile,"MBHOST",2,alResult);
    if (ilRc)
    {
      _bmqDebug("S0070: ���ļ�%s��ȡ MBHOST �ֶ�ʧ��!",alInitfile);
      return(-1);
    }
    strcpy(alMbhost,alResult);
    
    ilRc = _bmqGetfield(alInitfile,"FILEPORT",2,alResult);
    if (ilRc)
    {
      _bmqDebug("S0080: ���ļ�%s��ȡ MBPORT �ֶ�ʧ��!",alInitfile);
      return(-1);
    }
    ilFileport = atoi(alResult);
  }
  else
  {
  	ilRc = _bmqFileSrvCfg_load(iGrpid,alMbhost,&ilFileport);
  	if (ilRc)
    {
      _bmqDebug("S0090: ���ļ�%s��ȡ MBHOST �ֶ�ʧ��!",alInitfile);
      return(-1);
    }
  }
 
  /* �����׽��� */
  if ((ilSockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    _bmqDebug("S0100: Creat Socket Error! :%d,%s",errno,strerror(errno));
    return (-1050);
  }

  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( alMbhost );
  slServ_addr.sin_port = htons( ilFileport );

  /*���ӷ�����ͨѶ�˿�*/
  if (connect(ilSockfd, (struct sockaddr *)&slServ_addr, sizeof(slServ_addr)) < 0)
  {
    _bmqDebug("S0110: Connect Server[%s][%d] Error! :%d,%s",
               alMbhost,ilFileport,errno,strerror(errno));
    return (-1060);
  }
 
  /* ��ȡ��ǰSOCKET�� */
  ilAddrlen = sizeof(struct sockaddr_in);
  memset ((char *)&slCli_addr, 0, sizeof(struct sockaddr_in));
  if ( getsockname( ilSockfd,(struct sockaddr*)&slCli_addr,&ilAddrlen) == -1 )
  {
    _bmqDebug("S0120:%d,%s",errno,strerror(errno));
    return (-1070);
  }

  /* ����SOCKETѡ�� */
  ilLinger.l_onoff = 1 ;
  ilLinger.l_linger = 1 ;
  ilRc = setsockopt( ilSockfd, SOL_SOCKET, SO_LINGER, &ilLinger,\
                     sizeof(struct linger));
  if ( ilRc < 0 )
  {
    _bmqDebug("S0130:%d,%s",errno,strerror(errno));
    return (-1080); 
  }
  return ilSockfd;
}

int _bmqFile_msg(char aParm[][iFLDVALUELEN])
{
  /*
  �ú���Ϊ�ļ�������,�û������ڴ˶�ָ���ļ���������ܡ�
  ���ܡ�ѹ������ѹ���ȴ�������ʹ�ñ�Σ��������Ϊ10����
  ��һ������(aParm[0])ȱʡΪ��������ļ�����
  */
  if(igDebug >= 2)
    _bmqDebug("S0140: bmqFile_msg(%s)",aParm[0]);
  return 0;
}

int _bmqFile_comm(char aParm[][iFLDVALUELEN])
{
  /*
  �ú���Ϊ�ļ����ͺ������û������ڴ˶�ָ���ļ����в��
  ����ָ���İ�������ָ���ķ�ʽ��������˷��͡����͵ĳ�
  ʱʱ����ط���������ʹ�ò�����ָ��������
  ��һ������(aParm[0])ȱʡΪ��������ļ�����
  �ڶ�������(aParm[1])ȱʡΪSOCKET ID��
  ����������(aParm[1])ȱʡΪfile offset��
  ���Ĵ��͸�ʽ��Լ����
  ���ͱ��ģ�
      ���ĳ���(8byte)+��������(1byte=1)+�����к�(8byte)+���ݰ�
  ���ձ��ģ�
      �����к�(1byte)+Server�յ���ʵ�ʰ���(8byte)
  Ϊ��ȷ�����ݰ����͵������ԣ�TCP��ʽ�°������鲻����2K
  */
  int		ilRc;
  char		alMac[MAC_LEN];
  char		alFile[101];
  char		alFilename[101];
  char		alTmpFileName[101];
  char		alOffset[20];
  FILE		*fp,*fq;
  /*short		ilMsglen,ilBuflen,ilBuflen1,ilSendlen; delete by wh*/
  TYPE_SWITCH ilMsglen,ilBuflen,ilBuflen1,ilSendlen; /*add by wh*/
  int		ilSocket;
  char		alBuf[iMBMAXPACKSIZE+1],alMsgbuf[iMBMAXPACKSIZE*2];
  char		alSendbuf[iMBMAXPACKSIZE*2];
  long		i = 0 , llOffset = 0;
  char alCryptFlag[10];
  char alCompressFlag[10];
  long		llLastLen = 0;
  long		llTime;
  struct timeb slSendTime;

  if(igDebug >= 2)
    _bmqDebug("S0150: bmqFile_comm(%s,%s)",aParm[0],aParm[1]);
  
  strcpy(alFilename,aParm[0]);
  ilSocket = atoi(aParm[1]);
  llOffset = atol(aParm[2]);
  strcpy(alCompressFlag,aParm[3]);
  strcpy(alCryptFlag,aParm[4]);

  if (( fp = fopen(alFilename,"r")) == NULL)
  {
    _bmqDebug("S0160: ���ܴ�ָ���ļ�[%s] errno:%d:%s",alFilename,errno,strerror(errno));	
    return(-2);
  }

  fseek(fp,llOffset,SEEK_SET);
  memcpy(&slSendTime,&sgMonMsg.sBeginTime,sizeof(struct timeb));

  i++;
  llLastLen = llOffset;
  /* ȡ���ļ��ĵ�һ���ַ�������ѹ����� */
  memset(alBuf,0x00,sizeof(alBuf));
  memset(alSendbuf,0x00,sizeof(alSendbuf));
  ilMsglen = fread(alBuf,sizeof(char),igSend_pack_size,fp);
  sgMonMsg.lSendSize  += ilMsglen;
  if(strlen(alCompressFlag)!=0)
    ilBuflen =_bmqcompress(alBuf,ilMsglen,alSendbuf,alCompressFlag);
  else
  {
    /* ������ѹ�� */
    ilBuflen = ilMsglen;
    memcpy(alSendbuf,alBuf,ilMsglen);
  }
  if(ilBuflen==0)
  {
    /* û�н���ѹ�� */
    ilBuflen = ilMsglen;
    memcpy(alSendbuf,alBuf,ilMsglen);
  }
  if(strlen(alCryptFlag)!=0)
  {
    _bmqCrypt_all(alSendbuf,ilBuflen,FILEKEY,alCryptFlag);
  }
  sprintf(alMsgbuf,"%08d",(ilBuflen+17+MAC_LEN));
  alMsgbuf[8]= 'F';
  sprintf(alMsgbuf+9,"%08ld",i);
  memcpy(alMsgbuf+17,alSendbuf,ilBuflen);
  _bmqMac(alSendbuf,ilBuflen,alMac);
  memcpy(alMsgbuf+17+ilBuflen,alMac,MAC_LEN);
  while(1)
  {
    /* ����ѹ��������� */
    while(1)
    {
      ilSendlen = ilBuflen + 17 + MAC_LEN;
      if(igDebug >= 2)
        _bmqDebug("S0170:[%d][%s]\n",ilSendlen,alMsgbuf);
      ilRc = _comTcpSend(ilSocket,alMsgbuf,&ilSendlen);
      if(!ilRc)
      {   
      	if( (sgMonMsg.lSendSize - llLastLen) > 1024*1024 )
      	{ 
      	  sgMonMsg.iSendFlag  = 2;          
          ftime(&sgMonMsg.sEndTime);
          llTime = (sgMonMsg.sEndTime.time-slSendTime.time)*1000 + (sgMonMsg.sEndTime.millitm -slSendTime.millitm);
          if(llTime)
          {          	
            sgMonMsg.fSendSpeed = (double)( sgMonMsg.lSendSize - llLastLen ) / llTime;
            memcpy(&slSendTime,&sgMonMsg.sEndTime,sizeof(struct timeb));
            llLastLen = sgMonMsg.lSendSize;                                    
          }                                   
          strcpy(sgMonMsg.aStatusDesc,"�ļ�������...");          
          _bmqMonMsgSend(MONMSGCODE,sgMonMsg);
        }
        break;
      }
      else
      {
        /* add by xujun 2006.11.15 ֧�ֶϵ����� begin */
        memset(alTmpFileName,0x00,sizeof(alTmpFileName));
        _bmqfilechr(alFilename,strlen(alFilename),alFile,'/');
        sprintf(alTmpFileName,"%s/temp/%s.tpf",getenv("BMQ_FILE_DIR"),alFile);
        /*��ʱ�ļ�������,�½�֮*/
        if (( fq = fopen(alTmpFileName,"w+")) == NULL)
        {
          _bmqDebug("S0180: ���ܴ�����ʱ�ļ�[%s] errno:%d:%s",alTmpFileName,errno,strerror(errno));
          return(-1);
        }
        llOffset = ftell(fp) - igSend_pack_size;
        sgMonMsg.lSendSize = llOffset;
        memset(alOffset,0x00,sizeof(alOffset));
        sprintf(alOffset,"%ld",llOffset);
        fwrite(alOffset,sizeof(char),sizeof(alOffset),fq);
        fclose(fq);
        /* add by xujun 2006.11.15 ֧�ֶϵ����� end */

        fclose(fp);
        return(-1);
      }
    }
    /* ȡ����һ���ļ��ַ� */
    if(feof(fp))
    {
      /* add by xujun 2006.11.15 ֧�ֶϵ����� begin */
      /* �������,ɾ����ʱ��¼�ļ� */
      memset(alTmpFileName,0x00,sizeof(alTmpFileName));
      _bmqfilechr(alFilename,strlen(alFilename),alFile,'/');
      sprintf(alTmpFileName,"%s/temp/%s.tpf",getenv("BMQ_FILE_DIR"),alFile);
      unlink(alTmpFileName);
      /* add by xujun 2006.11.15 ֧�ֶϵ����� end */
      break; 
    }
    else
    {
      ilMsglen = fread(alBuf,sizeof(char),igSend_pack_size,fp);
      sgMonMsg.lSendSize  += ilMsglen;
      if(strlen(alCompressFlag)!=0)
        ilBuflen1 =_bmqcompress(alBuf,ilMsglen,alSendbuf,alCompressFlag);
      else
      {
        /* ��ѹ�� */
        ilBuflen1 = ilMsglen;
        memcpy(alSendbuf,alBuf,ilMsglen);
      }
      if(ilBuflen1==0)
      {
        /* û�н���ѹ�� */
        ilBuflen1 = ilMsglen;
        memcpy(alSendbuf,alBuf,ilMsglen);
      }
      if(strlen(alCryptFlag)!=0)
      {
        _bmqCrypt_all(alSendbuf,ilBuflen,FILEKEY,alCryptFlag);
      }
      sprintf(alMsgbuf,"%08d",(ilBuflen1+17+MAC_LEN));
      alMsgbuf[8] = 'F';
      i++;
      sprintf(alMsgbuf+9,"%08ld",i);
      memcpy(alMsgbuf+17,alSendbuf,ilBuflen1);
      _bmqMac(alSendbuf,ilBuflen1,alMac);
      memcpy(alMsgbuf+17+ilBuflen1,alMac,MAC_LEN);
    }
    ilBuflen = ilBuflen1;
  }
  fclose(fp);
  return 0;
}
/**************************************************************
 ** ������: _bmqFileSrvCfg_load
 ** ����:   ��������ļ�������Ϣ���ò���
 ** ����:   ���
 ** ��������: 2001/08/08
 ** ����޸�����: 2006/11/16
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:iGrpid--���
 ** ����ֵ: SUCCESS--�ɹ���FAIL--ʧ��
***************************************************************/
int _bmqFileSrvCfg_load(short iGrpid,char *aHostIP,int *iPort)
{
  char *alPath,alFile[51],alType[9];
  char alBuf[80],alBuf1[80],alResult[50],alResult1[50],alResult2[50]; 
  FILE *fp;

  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S0190: ��ϵͳ��������[BMQ_PATH]ʧ��");
    return(FAIL);
  }  
  memset(alFile,0x00,sizeof(alFile));
  strcpy(alFile,alPath);
  strcat(alFile,"/etc/bmqfilesrv.ini");
  if ((fp = fopen(alFile, "r")) == NULL) return(FAIL);

  while (fgets(alBuf1, sizeof(alBuf), fp) != NULL)
  {
    _bmqTrim(alBuf1);
    if (strlen(alBuf1) == 0 || alBuf1[0] == '#') continue;
    if (memcmp(alBuf1,"BEGIN",5) != 0 ) continue;

    sscanf(alBuf1,"%s %s", alResult,alType);

    /*�����������IP��ͨѶ�˿�*/
    if(strcmp(alType,"GRPFILESRV") == 0)
    {
      while( fgets(alBuf, sizeof(alBuf), fp) != NULL)
      {
        _bmqTrim(alBuf);
        if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
        if ( memcmp( alBuf,"END",3) == 0 )
        {
          fclose(fp);
          return(SUCCESS);
        }
        sscanf(alBuf,"%s %s %s",alResult,alResult1,alResult2);
        _bmqTrim(alResult);
        _bmqTrim(alResult1);
        _bmqTrim(alResult2);
        
        if( atoi(alResult) != iGrpid) continue;
        memcpy(aHostIP, alResult1, sizeof(alResult1));
        *iPort = atol(alResult2);

      }
    }
  }
  fclose(fp);
  return(FAIL);
} 

int _bmqTrim(s)
char *s;
{
        short   i, l, r, len;

        for(len=0; s[len]; len++);
        for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
        if(l==len)
        {
                s[0]='\0';
                return(SUCCESS);
        }
        for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
        for(i=l; i<=r; i++) s[i-l]=s[i];
        s[r-l+1]='\0';
        return(SUCCESS);
}

void _bmqfilechr(char *aOrgbuf,short iBuflen,char *aDesbuf,char cFlag)
{
  short         i; 

  for(i=0;i<iBuflen;i++)
  {
    if( aOrgbuf[iBuflen-i-1] == cFlag )
    break;
  }
  memcpy(aDesbuf,aOrgbuf+(iBuflen-i),i);
  aDesbuf[i] = '\0';
  return;
}

/**************************************************************
 ** ������:  bmqRecvfileX
 ** ����:    �����ļ���Server
 ** ����:    ���
 ** ��������: 2002/12/23
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: 0--�ɹ�
***************************************************************/
int bmqRecvfileX(long lSerialno,char * aFileName)
{
  int		ilRc,ilSockfd;
  /*short 	ilBuflen,ilMsglen,ilLen,ilLen1,ilMsglen1; delete by wh*/
  TYPE_SWITCH ilBuflen,ilMsglen,ilLen,ilLen1,ilMsglen1; /*add by wh*/
  char		alMac[MAC_LEN];
  char		alResult[10],alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];
  char		alSendbuf[iMBMAXPACKSIZE+100];
  char		alFilename[101],alInitfile[101],alTmpFileName[101];
  FILE		*fp,*fq;
  long		llFileserial,llFileid;
  int		ilSrvBakFlag;
  int		ilErrFlag;
  long		llOffset;
  char alCompressFlag[10];
  char alCryptFlag[10];
  
  memset(alVal,0x00,sizeof(alVal));

  /* add by xujun 20061117 next 10 lines
     �ļ����ߺ��Ƿ��ڷ������������ݱ�־ */
  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"SRVBAKFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: ���ļ�%s��ȡ SRVBAKFLAG �ֶ�ʧ��!",alInitfile);
    alResult[0] = '0';
  }
  ilSrvBakFlag = atoi(alResult);
  ilRc = _bmqGetfield(alInitfile,"COMPRESSFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: ���ļ�%s��ȡ COMPRESSFLAG �ֶ�ʧ��!",alInitfile);
    alResult[0] = '0'; /*ȱʡΪ��ѹ��*/
  }
  strcpy(alCompressFlag,alResult);
  ilRc = _bmqGetfield(alInitfile,"CRYPTFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: ���ļ�%s��ȡ CRYPTFLAG �ֶ�ʧ��!",alInitfile);
    alResult[0] = '0'; /*ȱʡΪ������*/
  }
  strcpy(alCryptFlag,alResult);
  /* ���ӵ������� */
  ilSockfd = _bmqConnectfileser(0);
  if ( ilSockfd < 0 )
  {
    _bmqDebug("S0210: --%d--���ӵ�����������!",ilSockfd);    
    return (ilSockfd);
  }
  /* ��֯����� */
  ilMsglen = 32;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 11;			/* �����־ 11=Request Recvfile */
  sprintf(alVal,"%08ld",lSerialno);
  memcpy(alMsgbuf+9,alVal,8);
  /* add by xujun 20061117 next 2 lines
     �ļ����ߺ��Ƿ��ڷ������������ݱ�־ */
  sprintf(alVal,"%05d",ilSrvBakFlag);
  memcpy(alMsgbuf+17,alVal,5);
  sprintf(alVal,"%5s",alCompressFlag);
  memcpy(alMsgbuf+22,alVal,5);
  sprintf(alVal,"%5s",alCryptFlag);
  memcpy(alMsgbuf+27,alVal,5);

  /* ������������ļ�����ĵ������� */ 
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0220: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    return (ilRc);
  }
  
  /* ������Ӧ���� */
  ilMsglen = sizeof(alMsgbuf);
  memset(alMsgbuf,0x00,sizeof(alMsgbuf));

  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0230: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    return (ilRc);
  }

  if(!memcmp(alMsgbuf,"OPERR",5))
  {
    _bmqDebug("S0240: �޷���ָ���ļ�!");
    close(ilSockfd);
    return(1001);
  }

  if(memcmp(alMsgbuf,"BEGIN",5))
  {
    _bmqDebug("S0250: ������δ��׼���ſ�ʼ�����ļ���");
    close(ilSockfd);
    return(-1);
  }
   
  memset(alFilename,0x00,sizeof(alFilename)); 
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/recv/",alMsgbuf+5);
  sprintf(alTmpFileName,"%s%s%s.tmp",getenv("BMQ_FILE_DIR"),"/temp/",alMsgbuf+5);
  /*���������ʱ�ļ�,��ʾ�ļ��ϴν���δ���,
    �����ʾ�ļ�δ�����չ������ϴ��Ѿ���ɹ�����,
    ���������Ĭ��Ϊ���½���*/
  if((fp = fopen(alTmpFileName,"r")) == NULL)
  {
    if ( (fq = fopen(alFilename,"w+")) == NULL )
    {
      _bmqDebug("S0260: ���ܴ��ļ�[%s]",alFilename);
      close(ilSockfd);
      return(-1);
    }
    /* ����ǰ������ʱ�ļ� */
    if ( (fp = fopen(alTmpFileName,"w+")) == NULL )
    {
      _bmqDebug("S0270: ���ܴ��ļ�[%s]",alFilename);
      close(ilSockfd);
      return(-1);
    }
    fclose(fp);
  }
  else
  {
    if ( (fq = fopen(alFilename,"a+")) == NULL )
    {
      _bmqDebug("S0280: ���ܴ��ļ�[%s]",alFilename);
      close(ilSockfd); 
      unlink(alTmpFileName);
      return(-1);
    }
  }
  llOffset = ftell(fq);

  ilMsglen = 24;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 12;   /* ֪ͨ���������͵�offset */
  sprintf(alVal,"%015ld",llOffset);
  memcpy(alMsgbuf+9,alVal,15);

  /* ���ͽ���׼���ѱ��ļ�����ĵ������� */ 
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0290: Send to server ERROR!!--%d",ilRc);
    fclose(fq);
    unlink(alTmpFileName);
    close(ilSockfd);
    return (ilRc);
  }
  
  llFileserial = 0; 
  ilErrFlag    = 0;
  while(1)
  {
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;

    ilRc = _comTcpReceive1(ilSockfd, alMsgbuf, &ilMsglen, 10) ; 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0300: �ձ��ĳ���!-Retcode=%d",ilRc);
      close( ilSockfd );
      fclose(fq);
      return (ilRc);
    }  	
    
    memcpy(alVal,alMsgbuf,8);
    alVal[8] = '\0';
    ilLen = atoi(alVal);
    
    ilLen1 = ilMsglen1 =0;
    while( ilMsglen1 < ilLen )
    {
      ilMsglen = ilLen - 8 - ilLen1;
      ilRc = _comTcpReceive1(ilSockfd, alMsgbuf+8+ilLen1, &ilMsglen, 10) ;
      if ( ilRc != 0 || ilMsglen <= 0)
      {
        _bmqDebug("S0310: �ձ��ĳ���!-Retcode=%d",ilRc);
        close( ilSockfd );
        fclose(fq);
        return (ilRc);
      }
      ilLen1 += ilMsglen;
      ilMsglen1 = ilLen1 + 8;
    }
    /* ���Ľ��� */
    switch(alMsgbuf[8])
    {
      case 'F': /* file transform*/
        memcpy(alVal,alMsgbuf+9,8);
        alVal[8] = '\0';
        llFileid = atol(alVal);
        ilMsglen = ilLen - 17 - MAC_LEN;
        /* add by xujun 20030523 */
        if( !ilErrFlag && (llFileid == llFileserial+1) )
        {
          _bmqMac(alMsgbuf+17,ilMsglen,alMac);
          if( (memcmp(alMac,alMsgbuf+ilLen-MAC_LEN,MAC_LEN)) &&
              (memcmp(alMsgbuf+ilLen-MAC_LEN,"00000000",MAC_LEN)) )
          {
            /*���յ������ļ����ܲ���ȷ*/
            _bmqDebug("S0000 ���յ������ļ����ܲ���ȷ");
            ilErrFlag = 1;
            break;
          }
          _bmqDebug("HXZ ,alMsgbuf=[%s]",alMsgbuf);  
          if(strlen(alCryptFlag)!=0)
          {
            _bmqCrypt_all(alMsgbuf+17,ilLen-17-MAC_LEN,FILEKEY,alCryptFlag);
          }
          _bmqDebug("HXZ1111111111 ,alMsgbuf=[%s]",alMsgbuf);  
          /* ��ѹ���������� */
          if(strlen(alCompressFlag)!=0)
            ilBuflen = _bmqdecompress(alMsgbuf+17,ilLen-17-MAC_LEN,alSendbuf,alCompressFlag);
          else
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilBuflen);
          }
          if(ilBuflen==0)
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilBuflen);
          }
          if(ilBuflen > 0)
            ilMsglen = fwrite(alSendbuf,sizeof(char),ilBuflen,fq);
          llFileserial = llFileid;
        }
        break;
      case 'E': /* file tranform end flag */
        close(ilSockfd);
        fclose(fq);
        if( ilErrFlag == 0)
          unlink(alTmpFileName);
        strcpy(aFileName,alFilename);
        return(SUCCESS);
      case 'R':/*��open file error */
        _bmqDebug("S0320: ���������ļ�ʧ��");
        fclose(fq);
        return(SUCCESS);
      default:
        _bmqDebug("S0330: ����ʶ�����������[%d]",alMsgbuf[8]);
        break;
    }
  } 
}

/* add by xujun 20061120 for recv file by filename begin */
/**************************************************************
 ** ������:  bmqRecvfileByName
 ** ����:    �����ļ����ӽ����ļ���Server
 ** ����:    ���
 ** ��������: 2006/11/20
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: 0--�ɹ�
***************************************************************/
int bmqRecvfileByName(short iGrpid,char * aFileName,char cCSFlag)
{
  int		ilRc,ilSockfd;
  /*short 	ilBuflen,ilMsglen,ilLen,ilLen1,ilMsglen1;  delete by wh*/
  TYPE_SWITCH ilBuflen,ilMsglen,ilLen,ilLen1,ilMsglen1; /*add by wh*/
  char		alMac[MAC_LEN];
  char		alVal[16],alMsgbuf[iMBMAXPACKSIZE+100];
  char          alSendbuf[iMBMAXPACKSIZE+100];
  char		alFilename[101],alTmpFileName[101];
  FILE		*fp,*fq;
  long		llFileserial,llFileid;
  long		llOffset;
  int		ilErrFlag;
  char alCompressFlag[10];
  char alCryptFlag[10];
  char		alResult[10],alInitfile[101];
  
  memset(alVal,0x00,sizeof(alVal));
  
  /*�����Ƿ����*/
  if( iGrpid == 0 ) iGrpid = psgMbshm->iMbgrpid;
  if( (psgMbshm->iMbgrpid == iGrpid) && cCSFlag == 'S' )
  {
  	/* ����ֱ�Ӵӷ�����Ŀ¼��ȡ�ļ�,���ش�·�����ļ��� */
  	memset(alFilename,0x00,sizeof(alFilename)); 
  	sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/send/",aFileName);
    strcpy(aFileName,alFilename);
    if((fp = fopen(aFileName,"r")) == NULL)
    {
      _bmqDebug("S0340: �޷���ȡָ���ļ�(%s)!",aFileName);
      return(1);
    }
    fclose(fp);
    return(SUCCESS);
  }	

  /* ���ӵ������� */
  ilSockfd = _bmqConnectfileser(iGrpid);
  if ( ilSockfd < 0 )
  {
    _bmqDebug("S0350: --%d--���ӵ�����������!",ilSockfd);    
    return (ilSockfd);
  }

  memset(alResult,0x00,sizeof(alResult));
  memset(alInitfile,0x00,sizeof(alInitfile));
  sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));
  ilRc = _bmqGetfield(alInitfile,"COMPRESSFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: ���ļ�%s��ȡ COMPRESSFLAG �ֶ�ʧ��!",alInitfile);
    alResult[0] = '0'; /*ȱʡΪ��ѹ��*/
  }
  strcpy(alCompressFlag,alResult);
  ilRc = _bmqGetfield(alInitfile,"CRYPTFLAG",2,alResult);
  if (ilRc)
  {
    _bmqDebug("S0200: ���ļ�%s��ȡ CRYPTFLAG �ֶ�ʧ��!",alInitfile);
    alResult[0] = '0'; /*ȱʡΪ������*/
  }
  strcpy(alCryptFlag,alResult);
  /* ��֯����� */
  ilMsglen = 19 + strlen(aFileName);
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 13;			/* �����־ 13=Request Recvfile by filename */
  sprintf(alVal,"%5s",alCompressFlag);
  memcpy(alMsgbuf+9,alVal,5);
  sprintf(alVal,"%5s",alCryptFlag);
  memcpy(alMsgbuf+14,alVal,5);
  memcpy(alMsgbuf+19,aFileName,strlen(aFileName));

  /* ������������ļ�����ĵ������� */ 
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0360: Send to server ERROR!!--%d",ilRc);
    close(ilSockfd);
    return (ilRc);
  }
  
  /* ������Ӧ���� */
  ilMsglen = sizeof(alMsgbuf);
  memset(alMsgbuf,0x00,sizeof(alMsgbuf));

  ilRc = _bmqRecvconf( ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 ) 
  {
    _bmqDebug("S0370: Receive from server error!!--%d",ilRc);
    close(ilSockfd);
    return (ilRc);
  }

  if(!memcmp(alMsgbuf,"OPERR",5))
  {
    _bmqDebug("S0380: �޷���ָ���ļ�!");
    close(ilSockfd);
    return(1001);
  }

  if(memcmp(alMsgbuf,"BEGIN",5))
  {
    _bmqDebug("S0390: ������δ��׼���ſ�ʼ�����ļ���");
    close(ilSockfd);
    return(-1);
  }
   
  memset(alFilename,0x00,sizeof(alFilename)); 
  memset(alTmpFileName,0x00,sizeof(alTmpFileName)); 
  sprintf(alFilename,"%s%s%s",getenv("BMQ_FILE_DIR"),"/recv/",alMsgbuf+5);
  sprintf(alTmpFileName,"%s%s%s.tmp",getenv("BMQ_FILE_DIR"),"/temp/",alMsgbuf+5);
  /*���������ʱ�ļ�,��ʾ�ļ��ϴν���δ���,
    �����ʾ�ļ�δ�����չ������ϴ��Ѿ���ɹ�����,
    ���������Ĭ��Ϊ���½���*/
  if((fp = fopen(alTmpFileName,"r")) == NULL)
  {
    if ( (fq = fopen(alFilename,"w+")) == NULL )
    {
      _bmqDebug("S0400: ���ܴ��ļ�[%s]",alFilename);
      close(ilSockfd);
      return(-1);
    }
    /* ����ǰ������ʱ�ļ� */
    if ( (fp = fopen(alTmpFileName,"w+")) == NULL )
    {
      _bmqDebug("S0410: ���ܴ��ļ�[%s]",alFilename);
      close(ilSockfd);
      return(-1);
    }
    fclose(fp);
  }
  else
  {
    if ( (fq = fopen(alFilename,"a+")) == NULL )
    {
      _bmqDebug("S0420: ���ܴ��ļ�[%s]",alFilename);
      close(ilSockfd); 
      unlink(alTmpFileName);
      return(-1);
    }
  }
  llOffset = ftell(fq);

  ilMsglen = 24;
  sprintf(alVal,"%08d",ilMsglen);
  memcpy(alMsgbuf,alVal,8);
  alMsgbuf[8] = 12;   /* ֪ͨ���������͵�offset */
  sprintf(alVal,"%015ld",llOffset);
  memcpy(alMsgbuf+9,alVal,15);

  /* ���ͽ���׼���ѱ��ļ�����ĵ������� */ 
  ilRc = _comTcpSend(ilSockfd,alMsgbuf,&ilMsglen); 
  if ( ilRc != 0 )
  {
    _bmqDebug("S0430: Send to server ERROR!!--%d",ilRc);
    fclose(fq);
    unlink(alTmpFileName);
    close(ilSockfd);
    return (ilRc);
  }
  
  llFileserial = 0; 
  ilErrFlag    = 0;
  while(1)
  {
    memset ( alMsgbuf, 0x00, sizeof(alMsgbuf) );
    ilMsglen = 8;

    ilRc = _comTcpReceive1(ilSockfd, alMsgbuf, &ilMsglen, 10) ; 
    if ( ilRc != 0 || ilMsglen <= 0)
    {
      _bmqDebug("S0440: �ձ��ĳ���!-Retcode=%d",ilRc);
      close( ilSockfd );
      fclose(fq);
      return (ilRc);
    }  	
    
    memcpy(alVal,alMsgbuf,8);
    alVal[8] = '\0';
    ilLen = atoi(alVal);
    
    ilLen1 = ilMsglen1 =0;
    while( ilMsglen1 < ilLen )
    {
      ilMsglen = ilLen - 8 - ilLen1;
      ilRc = _comTcpReceive1(ilSockfd, alMsgbuf+8+ilLen1, &ilMsglen, 10) ;
      if ( ilRc != 0 || ilMsglen <= 0)
      {
        _bmqDebug("S0450: �ձ��ĳ���!-Retcode=%d",ilRc);
        close( ilSockfd );
        fclose(fq);
        return (ilRc);
      }
      ilLen1 += ilMsglen;
      ilMsglen1 = ilLen1 + 8;
    }
    /* ���Ľ��� */
    switch(alMsgbuf[8])
    {
      case 'F': /* file transform*/
        memcpy(alVal,alMsgbuf+9,8);
        alVal[8] = '\0';
        llFileid = atol(alVal);
        ilMsglen = ilLen - 17 - MAC_LEN;
        /* add by xujun 20030523 */
        if( !ilErrFlag && (llFileid == llFileserial+1) )
        {
          _bmqMac(alMsgbuf+17,ilMsglen,alMac);
          if( (memcmp(alMac,alMsgbuf+ilLen-MAC_LEN,MAC_LEN)) &&
              (memcmp(alMsgbuf+ilLen-MAC_LEN,"00000000",MAC_LEN)) )
          {
            /*���յ������ļ����ܲ���ȷ*/
            ilErrFlag = 1;
            break;
          }

          if(strlen(alCryptFlag)!=0)
          {
            _bmqCrypt_all(alMsgbuf+17,ilLen-17-MAC_LEN,FILEKEY,alCryptFlag);
          }
          /* ��ѹ���������� */
          if(strlen(alCompressFlag)!=0)
            ilBuflen = _bmqdecompress(alMsgbuf+17,ilLen-17-MAC_LEN,alSendbuf,alCompressFlag);
          else
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilLen-17-MAC_LEN);
          }
          if(ilBuflen==0)
          {
            ilBuflen = ilLen-17-MAC_LEN;
            memcpy(alSendbuf,alMsgbuf+17,ilLen-17-MAC_LEN);
          }
          if(ilBuflen > 0)
            ilMsglen = fwrite(alSendbuf,sizeof(char),ilBuflen,fq);
          llFileserial = llFileid;
        }
        break;
      case 'E': /* file tranform end flag */
        close(ilSockfd);
        fclose(fq);
        if(ilErrFlag == 0)
          unlink(alTmpFileName);
        strcpy(aFileName,alFilename);
        return(SUCCESS);
      case 'R':/*��open file error */
        _bmqDebug("S0460: ���������ļ�ʧ��");
        fclose(fq);
        return(SUCCESS);
      default:
        _bmqDebug("S0470: ����ʶ�����������[%d]",alMsgbuf[8]);
        break;
    }
  } 
}

/**************************************************************
** ������:      _bmqCrypt
** ��  ��:      �򵥼��ܺ���
** ��  ��:      ���
** ��������:	2006/12/12
** ����޸����ڣ�
** ��������������
** ȫ�ֱ�����
** ��������:	�
** ����ֵ��  
***************************************************************/
int _bmqCrypt(char *inbuf,int inlen,char *key)
{
  register	char  ch;
  int		i,j=0,k=0;

  while(key[++k]);
  for(i=0; i<inlen; i++)
  {
    ch = inbuf[i];
    inbuf[i] = ch^key[j>=k?j=0:j++];
  }
  return(0);
}

/**************************************************************
** ������:	_bmqFmlpackget
** ��  ��:	�õ�FML������ָ�������ĳ��Ⱥ���ֵ
** ��  ��:	���
** ��������:2006/12/12
** ����޸����ڣ�
** ��������������
** ȫ�ֱ�����
** �������壺aFuname:�����Ӧ�ĺ�����.
             aInbuf :���뱨��
             aOutbuf:�������
** ����ֵ��  SUCCESS/FAIL
***************************************************************/
/*int _bmqFmlpackget(char *aMsgbody, short iMsglen, char *aFldname, short *piFldlen, char *aFldval) delete by wh*/
int _bmqFmlpackget(char *aMsgbody, TYPE_SWITCH iMsglen, char *aFldname, TYPE_SWITCH *piFldlen, char *aFldval) /*add by wh*/
{
  /*short  ilPos=0,ilLen; delete by wh*/
  TYPE_SWITCH ilPos=0,ilLen; /*add by wh*/
  char * alFldname;
	
  while (ilPos < iMsglen)
  {
    alFldname = aMsgbody + ilPos;
    if (strlen(alFldname) > iFLDNAMELEN)
    {
       _bmqDebug("S0070: [����/����] FML������������[%s]",alFldname);
       return(FAIL);
    }
    if (strcmp(alFldname, aFldname) == 0)
    {
      ilPos = ilPos + strlen(alFldname) + 1;
      /**piFldlen = *(short *)(aMsgbody + ilPos);
      ilPos = ilPos + sizeof(short); delete by wh*/
      *piFldlen = *(TYPE_SWITCH *)(aMsgbody + ilPos);
      ilPos = ilPos + sizeof(TYPE_SWITCH ); /*add by wh*/
      memcpy(aFldval, aMsgbody + ilPos, *piFldlen);
      return(SUCCESS);
    }
    else
    {
      ilPos = ilPos + strlen(alFldname) + 1;
      /*ilLen = *(short *)(aMsgbody + ilPos);
      ilPos = ilPos + sizeof(short) + ilLen;	delete by wh*/
      ilLen = *(TYPE_SWITCH*)(aMsgbody + ilPos); /*add by wh*/
      ilPos = ilPos + sizeof(TYPE_SWITCH) + ilLen;/*add by wh*/	
    }
  }
  return FAIL;
} 

/**************************************************************
** ������:	_bmqFmlpackset
** ��  ��:	����FML������ָ�������ĳ��Ⱥ���ֵ
** ��  ��:	���
** ��������:2006/12/12
** ����޸����ڣ�
** ��������������
** ȫ�ֱ�����
** �������壺aFuname:�����Ӧ�ĺ�����.
             aInbuf :���뱨��
             aOutbuf:�������
** ����ֵ��  SUCCESS/FAIL
***************************************************************/
/*int _bmqFmlpackset(char *aMsgbody, short *piMsglen, char *aFldname, short iFldlen, char *aFldval) delete by wh*/
int _bmqFmlpackset(char *aMsgbody, TYPE_SWITCH *piMsglen, char *aFldname, TYPE_SWITCH iFldlen, char *aFldval) /*add by wh*/
{
  /*short ilPos=0,ilLen,ilSize; delete by wh*/
  TYPE_SWITCH ilPos=0,ilLen,ilSize; /*add by wh*/
  char *alFldname;
  char alTmp[iMBMAXPACKSIZE];
  
  if(*piMsglen < 0)
  {
    _bmqDebug("S0080: [����/����] ��Чƫ����[%d]",*piMsglen);
    return(FAIL);
  }
  
  if(strlen(aFldname) > iFLDNAMELEN)
  {
    _bmqDebug("S0090: [����/����] FML������������[%s]",aFldname);
    return(FAIL);
  }

  while (ilPos < *piMsglen)
  {
    alFldname = aMsgbody + ilPos;
    if(strlen(alFldname) > iFLDNAMELEN)
    {
      _bmqDebug("S0100: [����/����] FML������������[%s]",alFldname);
       return(FAIL);
    }   
    if(strcmp(alFldname, aFldname) == 0)
    {
      ilPos = ilPos + strlen(alFldname) + 1;
      /*ilLen = *(short *)(aMsgbody + ilPos);
      ilSize = *piMsglen - ilPos - sizeof(short) - ilLen;
      memcpy(alTmp, (aMsgbody + ilPos + sizeof(short) + ilLen), ilSize);
      memcpy(aMsgbody + ilPos, (char *)&iFldlen, sizeof(short));
      ilPos = ilPos + sizeof(short); delete by wh*/


      ilLen = *(TYPE_SWITCH *)(aMsgbody + ilPos); /*add by wh*/
      ilSize = *piMsglen - ilPos - sizeof(TYPE_SWITCH) - ilLen; /*add by wh*/
      memcpy(alTmp, (aMsgbody + ilPos + sizeof(TYPE_SWITCH) + ilLen), ilSize); /*add by wh*/
      memcpy(aMsgbody + ilPos, (char *)&iFldlen, sizeof(TYPE_SWITCH)); /*add by wh*/
      ilPos = ilPos + sizeof(TYPE_SWITCH); /*add by wh*/

      memcpy(aMsgbody + ilPos, aFldval, iFldlen);
      ilPos = ilPos + iFldlen;
      memcpy(aMsgbody + ilPos,alTmp,ilSize);
      *piMsglen = *piMsglen + iFldlen - ilLen;
      return(SUCCESS);
    }
    else
    {
      ilPos = ilPos + strlen(alFldname) + 1;
      /*ilLen = *(short *)(aMsgbody + ilPos);
      ilPos = ilPos + sizeof(short) + ilLen; delete by wh*/	


      ilLen = *(TYPE_SWITCH*)(aMsgbody + ilPos); /*add by wh*/
      ilPos = ilPos + sizeof(TYPE_SWITCH) + ilLen;	/*add by wh*/
    }
  }
  memcpy((aMsgbody + *piMsglen), aFldname, strlen(aFldname));
  aMsgbody[*piMsglen + strlen(aFldname)] = '\0';
  /*memcpy(aMsgbody + *piMsglen + strlen(aFldname) + 1, 
    (char *)&iFldlen, sizeof(short)); 
  memcpy(aMsgbody + *piMsglen + strlen(aFldname) + sizeof(short) + 1,
    aFldval, iFldlen); 
  *piMsglen = *piMsglen +  strlen(aFldname) + 1 + sizeof(short) + iFldlen; 
    delete by wh*/

  memcpy(aMsgbody + *piMsglen + strlen(aFldname) + 1, 
    (char *)&iFldlen, sizeof(TYPE_SWITCH));         /*add by wh*/
  memcpy(aMsgbody + *piMsglen + strlen(aFldname) + sizeof(TYPE_SWITCH) + 1,
    aFldval, iFldlen); /*add by wh*/ 
  *piMsglen = *piMsglen +  strlen(aFldname) + 1 + sizeof(TYPE_SWITCH) + iFldlen;  /*add by wh*/
  
  return(SUCCESS) ;
}

static int _bmqUDPClose(int sock)
{
  close(sock);
  return(0);
}

static int _bmqUDPCreate(int iPort)
{
  int sockfd;
  struct sockaddr_in slAddr;

  if((sockfd = socket(AF_INET,SOCK_DGRAM,0)) == -1)
  {
    return -1;
  }

  slAddr.sin_family = AF_INET;
  slAddr.sin_addr.s_addr = htonl(INADDR_ANY);
  slAddr.sin_port = htons(iPort); 

  if(bind(sockfd,(struct sockaddr *)&slAddr,sizeof(struct sockaddr_in)) == -1)
  {
    _bmqUDPClose(sockfd);
    return -1;
  }

  return sockfd;
}

/*static int _bmqUDPPut(int sock,char *aAddr,int iPort,char *aMsgpack,short iMsglen) delete by wh*/
static int _bmqUDPPut(int sock,char *aAddr,int iPort,char *aMsgpack,TYPE_SWITCH iMsglen) /*add by wh*/
{
  struct sockaddr_in slSockaddr;
  struct hostent *host;
  int ilTolen = sizeof(struct sockaddr_in);
  int ilRc;

  if(!(host = gethostbyname(aAddr)))
  {
    if(!(host = gethostbyaddr(aAddr,strlen(aAddr),AF_INET)))
      return -1;
  }

  slSockaddr.sin_family = AF_INET;
  memcpy((char*)&slSockaddr.sin_addr,host->h_addr,host->h_length);
  slSockaddr.sin_port = htons(iPort);
  ilRc = sendto(sock,aMsgpack,iMsglen,0,
        (struct sockaddr *)&slSockaddr,ilTolen);
  if(ilRc == -1)
    return -1;

  return(0);
}

/**************************************************************
 ** ������      : _bmqMsgsend
 ** ��  ��      : ��UDP��ʽ�򽻻�ƽ̨���ģ�鷢��Ϣ����               
 ** ��  ��      : ���
 ** ��������    : 2006/12/22
 ** ����޸�����: 
 ** ������������: _bmqUDPCreate(),_bmqUDPPut(),_bmqUDPClose()
 ** ȫ�ֱ���    :
 ** ��������    : code : ��Ϣ����
		   msg  : FML����(������ͷ)
 ** ����ֵ      : SUCCESS,FAIL
***************************************************************/
int  _bmqMsgsend(long code,char *msg)
{
  struct msghead *pslMsghead;
  struct msgpack slMsgpack;
  /*short  ilMsglen,ilRc; delete by wh*/
  TYPE_SWITCH ilMsglen; /*add by wh*/
  short ilRc;           /*add by wh*/
  int    ilSockfd;
  char   alResult[20];
  char   alInitfile[101];
  static char alAddr[16];
  static int ilPort = -1;

  pslMsghead = (struct msghead *)msg;

  if ( msg )
  {
    ilMsglen = pslMsghead->iBodylen + sizeof(struct msghead);
    if (ilMsglen > sizeof(struct msgpack))
      memcpy(&slMsgpack,msg,sizeof(struct msgpack));
    else
      memcpy(&slMsgpack,msg,ilMsglen);
  }
  else 
  {
    slMsgpack.sMsghead.iBodylen = 0;
  }
  slMsgpack.sMsghead.iMsgtypeorg = slMsgpack.sMsghead.iMsgtype;
  slMsgpack.sMsghead.iMsgtype    = 901;
  slMsgpack.sMsghead.lCode       = code;
  if (slMsgpack.sMsghead.lBegintime == 0) time(&slMsgpack.sMsghead.lBegintime);
  ilMsglen = slMsgpack.sMsghead.iBodylen + sizeof(struct msghead);
  
  if(ilPort == -1)
  {
    /*��ȡ���IP�Ͷ˿�*/
    memset(alAddr,0x00,sizeof(alAddr));
    memset(alInitfile,0x00,sizeof(alInitfile));
    sprintf(alInitfile,"%s/etc/bmqcls.ini",getenv("BMQ_PATH"));  
    ilRc = _bmqGetfield(alInitfile,"MONHOST",2,alAddr);
    if (ilRc)
    {
      _bmqDebug("S8200: ���ļ�%s��ȡ MONHOST �ֶ�ʧ��!",alInitfile);
      return(FAIL);
    }
    _bmqTrim(alAddr);
    ilRc = _bmqGetfield(alInitfile,"MONPORT",2,alResult);
    if (ilRc)
    {
      _bmqDebug("S8210: ���ļ�%s��ȡ MONPORT �ֶ�ʧ��!",alInitfile);
      return(FAIL);
    }
    ilPort = atoi(alResult);
  }
   
  if ((ilSockfd = _bmqUDPCreate(0)) == -1) return(FAIL);
  _bmqVdebug(1,"S8212: ���ͼ�ر��ĵ�[%s][%d]!",alAddr,ilPort);
  ilRc = _bmqUDPPut(ilSockfd,alAddr,ilPort,(char *)&slMsgpack,ilMsglen);
  _bmqUDPClose(ilSockfd);
   
  if (ilRc)
    return(FAIL);
  else
    return(SUCCESS); 
}

/**************************************************************
 ** ������      : _bmqMonMsgSend
 ** ��  ��      : ���ļ������������(1������)���ʹ���״̬����               
 ** ��  ��      : ���
 ** ��������    : 2006/12/22
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : lMsgCode : ��Ϣ����
 ** ����ֵ      : SUCCESS,FAIL
***************************************************************/
int _bmqMonMsgSend(long lMsgCode,struct monmsg sMonMsg)
{
  int		ilRc;
  
  sMonMsg.lMsgCode = lMsgCode;
  ilRc = bmqPut(0,iMBFILEMNG,0,0,0,(char *)&sMonMsg,sizeof(struct monmsg));
  if(ilRc)
    return(FAIL);
  
  return(SUCCESS);
}
 
/**************************************************************
 ** ������      : _bmqMonMsgPut
 ** ��  ��      : �򽻻�ƽ̨��غ����ģ�鷢��FML״̬����              
 ** ��  ��      : ���
 ** ��������    : 2006/12/22
 ** ����޸�����: 
 ** ������������: 
 ** ȫ�ֱ���    :
 ** ��������    : lMsgCode : ��Ϣ����
 ** ����ֵ      : SUCCESS,FAIL
***************************************************************/
void _bmqMonMsgPut(struct monmsg sMonMsg)
{
  /*short		ilMsgLen = 0; delete by wh*/
  TYPE_SWITCH ilMsgLen = 0; /* add  by wh*/
  char		alTimeBuf[30];
  char		alFld[iFLDVALUELEN];
  struct	msgpack	slMsgpack;

  memset(&slMsgpack,0x00,sizeof(struct msgpack));
  
  slMsgpack.sMsghead.iMsgtype	= iMSGFILETRANS;
  sprintf(alFld,"%d",sMonMsg.iSendFlag);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"sendflag",strlen(alFld),alFld);
  if(sMonMsg.iSendFlag == 5)
    sprintf(alFld,"%d",1);
  else
    sprintf(alFld,"%d",3);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"regtype",strlen(alFld),alFld);
  sprintf(alFld,"%s",sMonMsg.aStatusDesc);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"statusdesc",strlen(alFld),alFld);
  sprintf(alFld,"%ld",sMonMsg.lSerialno);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"serilano",strlen(alFld),alFld);
  memset(alFld,0x00,sizeof(alFld));
  _bmqfilechr(sMonMsg.aFileName,strlen(sMonMsg.aFileName),alFld,'/');
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"filename",strlen(alFld),alFld);
  sprintf(alFld,"%d",sMonMsg.iOrgGrpid);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"orggrpid",strlen(alFld),alFld);
  sprintf(alFld,"%d",sMonMsg.iOrgMbid);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"orgmbid",strlen(alFld),alFld);
  sprintf(alFld,"%d",sMonMsg.iDesGrpid);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"desgrpid",strlen(alFld),alFld);
  sprintf(alFld,"%d",sMonMsg.iDesMbid);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"desmbid",strlen(alFld),alFld);
  _bmqTimeBuf(&sMonMsg.sBeginTime.time,alTimeBuf);
  sprintf(alFld,"%s",alTimeBuf);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"begintime",strlen(alFld),alFld);
  _bmqTimeBuf(&sMonMsg.sEndTime.time,alTimeBuf);
  sprintf(alFld,"%s",alTimeBuf);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"endtime",strlen(alFld),alFld);
  sprintf(alFld,"%ld",sMonMsg.lFileSize);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"filesize",strlen(alFld),alFld);
  sprintf(alFld,"%ld(%.2lf%%)",sMonMsg.lSendSize,(double)(sMonMsg.lSendSize/(sMonMsg.lFileSize*0.01)));
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"sendsize",strlen(alFld),alFld);
  sprintf(alFld,"%.2lf",sMonMsg.fSendSpeed);
  _bmqFmlpackset(slMsgpack.aMsgbody,&ilMsgLen,"sendspeed",strlen(alFld),alFld);
  
  slMsgpack.sMsghead.iBodylen = ilMsgLen;
  _bmqMsgsend(sMonMsg.lMsgCode,(char *)&slMsgpack);
  bmqPut(0,iMBAUDIT,0,0,0,(char *)&slMsgpack,ilMsgLen+sizeof(struct msghead)); 
}
/* add by xujun 20061120 for recv file by filename end */

/* add by xujun 20070110 for virus scan begin */
/********************************************************************
 ** ������:   _bmqLoadVirusDbDir
 ** ����:     װ��ָ��·���µĲ�����(��ָ��ʱʹ��ȱʡֵ)
 ** ����:     ���
 ** ��������: 2007/01/10
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: aPath-������·�� sRoot-���ڵ� iVirusNum-�ɼ�ⲡ����
 ** ����ֵ: SUCCESS/FAIL
*********************************************************************/
int _bmqLoadVirusDbDir(char *aPath,struct cl_node **sRoot,unsigned int *iVirusNum)
{
#ifdef VIRUS_SCAN
  int    ilRc;
  char   alVirusDbDir[120];

  if(aPath == NULL)
    sprintf(alVirusDbDir,"%s/lib/clamav",getenv("BMQ_PATH"));
  else
    sprintf(alVirusDbDir,"%s",aPath);

  /* load all available databases from directory */   
  if((ilRc = cl_loaddbdir(alVirusDbDir, sRoot, iVirusNum))) 
  {
    _bmqDebug("S3530: bmqLoadVirusDbDir fail: %s",cl_perror(ilRc));
    return(FAIL);
  }
 
  if(igDebug >= 1)
    _bmqDebug("S3540: Loaded %d signatures",*iVirusNum);

  /* build engine */
  if((ilRc = cl_build(*sRoot))) 
  {
    _bmqDebug("S3550: Database initialization error: %s", cl_strerror(ilRc));;
    cl_free(*sRoot);
    return(FAIL);
  }

  if(igDebug >= 1)
    _bmqDebug("S3560: Virus Database initialization success");

#endif

  return(SUCCESS);
}

/********************************************************************
 ** ������:   _bmqVirusScanByRoot
 ** ����:     ɨ��ָ���ļ��Ƿ��Ⱦ����(��װ�ز�����)
 ** ����:     ���
 ** ��������: 2007/01/10
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: aFileName-��ɨ���ļ� sRoot-��������ڵ� aVirName-��������
 ** ����ֵ: SUCCESS/FAIL
*********************************************************************/
int _bmqVirusScanByRoot(char *aFileName,struct cl_node *sRoot,char *aVirName)
{
#ifdef VIRUS_SCAN
  int      ilRc;
  int      fd;
  unsigned long int blocks = 0;
  const    char *virname;
  struct   cl_limits limits;
 
  if((fd = open(aFileName, O_RDONLY)) == -1) 
  {
    _bmqDebug("S3570: Can't open file %s", aFileName);
    return(FAIL);
  }

  /* set up archive limits */
  memset(&limits, 0, sizeof(struct cl_limits));
  limits.maxfiles      = 1000;         /* max files */
  limits.maxfilesize   = 10 * 1048576; /* maximal archived file size == 10 Mb */
  limits.maxreclevel   = 5;            /* maximal recursion level */
  limits.maxratio      = 200;          /* maximal compression ratio */
  limits.archivememlim = 0;            /* disable memory limit for bzip2 scanner */
 
  /* scan descriptor */
  if((ilRc = cl_scandesc(fd, &virname, &blocks, sRoot, &limits, CL_SCAN_STDOPT)) == CL_VIRUS) 
  {
    _bmqDebug("S3580: File[%s]'s virus detected: %s",aFileName,virname);
    strcpy(aVirName,virname);
  }
  else 
  {
    if(igDebug >=1)
      _bmqDebug("S3590: File[%s] No virus detected.",aFileName);
    strcpy(aVirName,"");
    if(ilRc != CL_CLEAN)
    {
      _bmqDebug("S3600: File[%s] virus scan error:%s",aFileName,cl_perror(ilRc));
      return(FAIL);
    }
  }
  close(fd);

#endif
  return(SUCCESS);
}

/********************************************************************
 ** ������:   _bmqFreeVirusRoot
 ** ����:     �ͷ�װ�صĲ�����ڵ�
 ** ����:     ���
 ** ��������: 2007/01/10
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: sRoot-��������ڵ�
 ** ����ֵ: 
*********************************************************************/
void _bmqFreeVirusRoot(struct cl_node *sRoot)
{
#ifdef VIRUS_SCAN
  cl_free(sRoot);
#endif
}

/********************************************************************
 ** ������:   _bmqScanVirus
 ** ����:     ɨ��ָ���ļ� 
 ** ����:     ���
 ** ��������: 2007/01/10
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: aVirusDbDir-������·�� aFileName-��ɨ���ļ��� aVirName-��������
 ** ����ֵ:   SUCCESS/FAIL
*********************************************************************/
int _bmqScanVirus(char *aVirusDbDir,char *aFileName,char *aVirName)
{
#ifdef VIRUS_SCAN
  int    ilRc;
  struct cl_node *root = NULL;
  unsigned int sigs    = 0; 

  ilRc = _bmqLoadVirusDbDir(aVirusDbDir,&root,&sigs);
  if(ilRc) return(ilRc);

  ilRc = _bmqVirusScanByRoot(aFileName,root,aVirName);

  _bmqFreeVirusRoot(root);
  return(ilRc);
#endif
  return(SUCCESS);
}
/* add by xujun 20070110 for virus scan end */

/* add by xujun 2007.01.26 for msg route begin */
/********************************************************************
 ** ������:   _bmqLoadRouterinfo
 ** ����:     ���붨���·��������Ϣ 
 ** ����:     ���
 ** ��������: 2007/01/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������: 
 ** ����ֵ:   SUCCESS/FAIL
*********************************************************************/
int _bmqLoadRouterinfo()
{
  short  ilRecnum;                  /*��¼����*/
  char   *alPath;                   /*·��*/
  char   alFileName[101];           /*�ļ�����*/
  char   alBuf[2048]; 
  FILE   *fp;
  
  if ((alPath = getenv("BMQ_PATH")) == NULL)
  {
    _bmqDebug("S3700 ��ϵͳ��������[BMQ_PATH]ʧ��");
    return(FAIL);
  }
  
  memset(alFileName,0x00,sizeof(alFileName));
  strcpy(alFileName,alPath);
  strcat(alFileName,"/etc/bmqfilter.ini");
  if ((fp = fopen(alFileName, "r")) == NULL){
    _bmqDebug("S3710 �޷���·�������ļ�[%s]",alFileName);
    return(FAIL);
  }
  
  ilRecnum = 0;
  while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
  {
    _bmqTrim(alBuf);
    if (strlen(alBuf) == 0 || alBuf[0] == '#') continue;
  
    sscanf(alBuf,"%hd %s %hd %hd %hd %s",
                  &psgMbrouter[ilRecnum].iOrgMbid,                   
                   psgMbrouter[ilRecnum].aValue,
                  &psgMbrouter[ilRecnum].iDesGrpid,
                  &psgMbrouter[ilRecnum].iDesMbid,
                  &psgMbrouter[ilRecnum].iOpenFlag,
                   psgMbrouter[ilRecnum].aDesc);
    ilRecnum++;                   
  }
  
  psgMbshm->iRouternum = ilRecnum;
  _bmqDebug("S3720 Load��[%d]��·�ɼ�¼",psgMbshm->iRouternum);
  fclose(fp);
  return(SUCCESS);
}

/**************************************************************
 ** ������:_bmqGetRouter
 ** ����:  �����������ȡ·��
 ** ����:  ���
 ** ��������: 2007/01/25
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:
 ** ����ֵ: SUCCESS
***************************************************************/
int _bmqGetRouter(short iMbid,char *aExpress,short *piGrpid,short *piMbid)
{
  int    i;

  for(i = 0; i < psgMbshm->iRouternum; i++)
  {
  	if( (psgMbrouter[i].iOrgMbid == iMbid) && (!memcmp(psgMbrouter[i].aValue,aExpress,strlen(aExpress))) )
    {
      if(psgMbrouter[i].iOpenFlag != 1)
      {
        _bmqVdebug(0,"S8110 ��·��[%d][%s]û������!",iMbid,aExpress);
        return(FAIL);
      }
      *piGrpid = psgMbrouter[i].iDesGrpid;
      *piMbid  = psgMbrouter[i].iDesMbid;
      return(SUCCESS);
    }
  }

  _bmqVdebug(0,"S8120 ����[%d]������[%s]û�ж����Զ�·��",iMbid,aExpress);
  return(FAIL);
}
/* add by xujun 2007.01.26 for msg route end */

int _bmqTimeOut(int iFd, long lSeconds)
{
   int ilRc;
   int error=0,len;

   fd_set rset, wset;
   struct timeval timeout;

   FD_ZERO(&rset);
   FD_ZERO(&wset);
   FD_SET(iFd, &rset);
   FD_SET(iFd, &wset);
   timeout.tv_sec  = lSeconds;
   timeout.tv_usec = 0 ;

   ilRc = select( iFd+1, &rset, &wset, NULL, &timeout );
   if ( ilRc == 0 )
   {
      return 1 ;
   }
   if(FD_ISSET(iFd,&wset)||FD_ISSET(iFd,&rset))
   {
     len = sizeof(error);
     if ( getsockopt(iFd,SOL_SOCKET,SO_ERROR,&error,&len)<0 )
     {
        return 2;
     }
   }
   else
     return 3;
   if(error)
     return 4;

   return(SUCCESS);
}

/**************************************************************
 ** ������: _bmqGrpcontserS
 ** ����:   ���ӿ��������(������)
 ** ����:
 ** ��������: 2007/01/18
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���:
 ** ��������:aIp--������IP��ַ  iPort--ͨѶ�˿� lTime ��ʱʱ��
 ** ����ֵ: 0--�ɹ�
***************************************************************/
int _bmqGrpcontserS(char *aIp,short iPort,long lTime)
{
  int    ilRc,flag;
  int    sockfd = -1;
  struct sockaddr_in slServ_addr;
  struct sockaddr_in sCli_addr;  
  
  /* �����׽��� */
  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    _bmqDebug("S0170 : Creat Socket Error! :errno: %d,%s",errno,strerror(errno));
    return (-1050);
  }
  memset((char *)&sCli_addr,0x00,sizeof(struct sockaddr_in));
  memset((char *)&slServ_addr, 0, sizeof(struct sockaddr_in));
  slServ_addr.sin_family = AF_INET;
  slServ_addr.sin_addr.s_addr = inet_addr( aIp );
  slServ_addr.sin_port = htons( iPort );

  flag =fcntl( sockfd, F_GETFL, 0 );
  if ( flag < 0 ){
    _bmqDebug("S0171: fcntl F_GETFL error");
    close(sockfd);
    return (-1060);
  }
  if ( fcntl( sockfd, F_SETFL, flag|O_NONBLOCK )<0 ){
    _bmqDebug("S0172: fcntl F_SETFL error");
    close(sockfd);
    return (-1060);
  }

  /*����ͨѶ�˿�*/
  ilRc = connect(sockfd,(struct sockaddr *)&slServ_addr,sizeof(slServ_addr));  
  if(ilRc < 0)
  {
    if(errno!=EINPROGRESS && errno !=0)
    {
      _bmqDebug("S0180 : Connect Server[%s][%d] Error! errno: %d,%s",aIp,iPort,errno,strerror(errno));
      close(sockfd);
      return (-1060);
    }
  }
  if(ilRc != 0)
  {
    ilRc = _bmqTimeOut ( sockfd , lTime );
    if(  ilRc != 0 )
    {
      _bmqDebug("S0181 : Connect Server[%s][%d] Error! errno: %d,%s",aIp,iPort,errno,strerror(errno));
      _bmqDebug("S0182 : ilRc=[%d][%ld]",ilRc,lTime);
      close(sockfd);
      return(-1060);
    }
  }
  if ( fcntl( sockfd, F_SETFL, flag )<0 ){
     _bmqDebug("S0173: fcntl F_SETFL error");
     close(sockfd);
     return (-1060);
  }
 
  return sockfd;
}

int _bmqcompress(void *src, unsigned src_len, void *dst ,char *press)
{
   short ilRc=0;   
   if(strcmp(press,"haha")==0)
   {
     ilRc=compress(src,src_len,dst); 
     if(ilRc<=0)
     { 
       _bmqDebug("S0174: ����compress��������");
       return(ilRc);
     }
   }
   else
   {
    
   }
   return(0);
   
}

int _bmqdecompress(void *src, unsigned src_len,	void *dst,char *press)
{
   short ilRc=0;   
   if(strcmp(press,"haha")==0)
   {
     ilRc=decompress(src,src_len,dst); 
     if(ilRc<=0)
     { 
       _bmqDebug("S0175: ����decompress��������");
       return(ilRc);
     }
   }
   else
   {
    /*memcpy(dst,src_len,src);*/
   }  
   return(0);
}

int _bmqCrypt_all(char *inbuf,int inlen,char *key,char *press)
{
   short ilRc=0;   
   if(strcmp(press,"haha")==0)
   {
     ilRc=_bmqCrypt(inbuf,inlen,key);
     if(ilRc!=0)
     { 
       _bmqDebug("S0176: ����_bmqCrypt��������");
       exit(ilRc);
     }
     
   }
   else
   {}  
   return(inlen);
}


