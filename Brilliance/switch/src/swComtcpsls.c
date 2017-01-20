/****************************************************************/
/* Ä£¿é±àºÅ    £ºswComtcpsls                                    */
/* Ä£¿éÃû³Æ    : Í¨ÐÅÀý³Ì-TCPÍ¬²½³¤Á¬½Óserver                   */
/* ×÷    Õß    £º     Û                                         */
/* ½¨Á¢ÈÕÆÚ    £º2000/5/18                                      */
/* ×îºóÐÞ¸ÄÈÕÆÚ£º2001/9/18                                      */
/* Ä£¿éÓÃÍ¾    £º                                               */
/* ±¾Ä£¿éÖÐ°üº¬ÈçÏÂº¯Êý¼°¹¦ÄÜËµÃ÷£º                             */
/*                        (1)int swComtcpsls()                  */
/*                        (2)int swDoitsls()                    */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

static long    igSockfd;
static long    igSocket;

void swQuit();
int swDoitsls();

int main( int argc , char **argv )
{
  int    ilRc;

  #ifdef OS_SCO
    int  ilAddrlen;
  #else
    socklen_t  ilAddrlen;
  #endif

  struct hostent *pslHp;
  struct linger slLinger;
  char   alCliname[80];
  char   alCliaddr[16];
  unsigned int ilQid,ilMsglen,ilPri,ilClass,ilType;
  char   alMsgbuf[iMSGMAXLEN];
  struct sockaddr_in slCli_addr;
  
  /* È¡µÃ°æ±¾ºÅ */
  if(argc > 1)
    _swVersion("swComtcpsls Version 4.3.0",argv[1]);

  /* ÉèÖÃµ÷ÊÔ³ÌÐòÃû³Æ */
  sprintf( agDebugfile,"swComtcp%s.debug",argv[1]);

  if ( argc != 2 )
  {
    swVdebug(0,"S0010: [´íÎó/ÆäËü] ´«Èë²ÎÊýÊýÁ¿²»ºÏ·¨!");
    exit(1);
  }
  
  /* ´¦ÀíÐÅºÅ */
  swSiginit( );
  
  ilRc = swLoadcomcfg( argv[1] );
  if ( ilRc != 0 )
  {
    swVdebug(0,"S0020: [´íÎó/º¯Êýµ÷ÓÃ] swLoadcomcfg()º¯Êý[%s]×°ÈëÍ¨Ñ¶ÅäÖÃ²ÎÊý³ö´í,Çë¼ì²éÊäÈëµÄ¶Ë¿ÚÃû,·µ»ØÂë=%d",argv[1],ilRc);
    exit( 1 );
  }
  
  swVdebug(2,"S0030: [%s]×°ÈëÍ¨Ñ¶ÅäÖÃ²ÎÊý³É¹¦",argv[1]);
    
  memset(alCliaddr,0x0,sizeof(alCliaddr));
  memset((char *)&slCli_addr, 0x00, sizeof(struct sockaddr_in) );

  igSocket=swTcpconnect(sgTcpcfg.iPartner_port);
  if (igSocket == -1)
  {
    swVdebug(0,"S0040: [´íÎó/º¯Êýµ÷ÓÃ] swTcpconnect()ERROR!");
    exit( -1 );
  }
  swVdebug(2,"S0050: ½¨Á¢Á¬½Ó³É¹¦");
   
  /*´ò¿ªÍ¨Ñ¶ÓÊÏä*/
  ilRc = swMbopen( sgComcfg.iMb_comm_id ); 
  if ( ilRc )
  {
    swVdebug(0,"S0060: [´íÎó/ÓÊÏä] ´ò¿ªÓÊÏä[%d]³ö´í!",sgComcfg.iMb_comm_id );
    close( igSocket );
    exit( -1 );
  }
  swVdebug(2,"S0070: ´ò¿ªÓÊÏä³É¹¦");
  
  /*½ÓÊÕÁ¬½Ó*/
  swVdebug(2,"S0080: ¿ªÊ¼½ÓÊÕ");
  ilAddrlen = sizeof(struct sockaddr_in);
  
  igSockfd  = accept(igSocket,(struct sockaddr*)&slCli_addr,&ilAddrlen);
  if ( igSockfd == -1)
  {
    swVdebug(0,"S0090: [´íÎó/ÏµÍ³µ÷ÓÃ] Á¬½Ó´íÎó,errno=%d[%s]",errno,strerror(errno));
    close(igSocket);
    swMbclose();
    exit( -1 );
  }
  swVdebug(2,"S0100: accept() SUCCESS!");
 
  strcpy(alCliaddr,inet_ntoa(slCli_addr.sin_addr)); 
  
  swVdebug(2,"S0110: alCliaddr=%s",alCliaddr);
  
  /*»ñµÃ¿Í»§¶ËÖ÷»úÃû*/
  pslHp = gethostbyaddr( (char *) &slCli_addr.sin_addr, sizeof(struct in_addr),
    slCli_addr.sin_family);
  if ( pslHp == NULL)
    strcpy( alCliname, inet_ntoa( slCli_addr.sin_addr ) );
  else
    strcpy( alCliname, pslHp->h_name );

  swVdebug(2,"S0120: gethostbyaddr SUCCESS");
    
  slLinger.l_onoff  =1;
  slLinger.l_linger =0;
  
  ilRc = setsockopt(igSockfd,SOL_SOCKET,SO_LINGER,&slLinger,
    sizeof(struct linger));
  if ( ilRc == -1 )
  {
    swVdebug(0,"S0130: [´íÎó/ÏµÍ³µ÷ÓÃ] setsockopt(),Connect error!errno=[%d,%s]",errno,strerror(errno));
    close( igSocket );
    close( igSockfd );
    swMbclose();
    exit( -1 );
  }
  swVdebug(2,"S0140: setsockopt SUCCESS!");
 
  for(;;)
  {
    /*´Ó¿Í»§¶Ë½ÓÊÕ±¨ÎÄ*/
    ilMsglen = sizeof( alMsgbuf );
  
    ilRc = swTcprcv(igSockfd, alMsgbuf, &ilMsglen );
    if ( ilRc )
    {
      swVdebug(1,"S0150: [´íÎó/º¯Êýµ÷ÓÃ] swTcprcv()º¯Êý,´Ó¿Í»§¶Ë{%s}½ÓÊÕ±¨ÎÄ³ö´í,·µ»ØÂë=%d",alCliname,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }
    swVdebug(2,"S0160: ´Ó¿Í»§¶Ë½ÓÊÕ±¨ÎÄ³É¹¦");
    if (cgDebug >= 2) 
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /* next add by nh*/
    if (sgComcfg.iMsghead == 0)
    {
      /* ÔÚ±¨ÎÄÇ°Ôö¼Ó¿Õ±¨ÎÄÍ· */
      swComaddblankmsghead(alMsgbuf,&ilMsglen);
    }

    swVdebug(2,"S0170: ´Ó¿Í»§¶Ë{%s}½ÓÊÕ±¨ÎÄÍê±Ï!len=%d",alCliname,ilMsglen);
 
    ilPri = 0 ;
    ilClass = 0 ;
    ilType  = 0 ;
 
    ilRc = swSendpack(sgComcfg.iMb_fore_id, alMsgbuf, ilMsglen, ilPri, 
      ilClass, ilType);
    if (ilRc)
    {
      swVdebug(1,"S0180: [´íÎó/º¯Êýµ÷ÓÃ] swSendpack()º¯Êý,Ð´±¨ÎÄµ½ÓÊÏä[%d]³ö´í,·µ»ØÂë=%d",ilQid,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }

      swVdebug(2,"S0190: Ð´±¨ÎÄµ½ÓÊÏä³É¹¦");
      swVdebug(2,"S0200: Writed to Qid=[%d],ilPri=[%d],Class=[%d],Type=[%d]",\
        ilQid, ilPri, ilClass, ilType );
        
    /*¶ÁÍ¨Ñ¶ÓÊÏä*/
    swVdebug(2,"S0210: µÈ´ý¶ÁÍ¨Ñ¶ÓÊÏä");
  
    ilMsglen = sizeof( alMsgbuf );
    ilPri = 0 ;
    ilClass = 0 ;
    ilType = 0 ;
 
    ilRc = swRecvpack( &ilQid, alMsgbuf, &ilMsglen, &ilPri, &ilClass,\
      &ilType , sgComcfg.iTime_out );
    if ( ilRc)
    {
      if ( ilRc == BMQ__TIMEOUT)
      {
        swVdebug(1,"S0220: [´íÎó/º¯Êýµ÷ÓÃ] swRecvpack()º¯Êý,¶ÁÓÊÏä[%d]³¬Ê±,·µ»ØÂë=%d",ilQid,ilRc);
        continue;
      }
      swVdebug(1,"S0230: [´íÎó/º¯Êýµ÷ÓÃ] swRecvpack()º¯Êý,¶ÁÓÊÏä[%d]³ö´í,·µ»ØÂë=%d",ilQid,ilRc);
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit(-1);
    }
    swVdebug(2,"S0240: swRecvpack() SUCCESS");
    if (cgDebug >= 2)
    {
      swDebughex(alMsgbuf,ilMsglen);
    }

    /*ËÍ»Ø¿Í»§¶Ë */
    /*ÅÐ¶ÏÓÐÎÞ±¨ÎÄÍ·*/
    if(sgComcfg.iMsghead == 0)
    {
      /* ²»´ø±¨ÎÄÍ·ÍùÍâ·¢ */
      ilRc = swTcpsnd(igSockfd,alMsgbuf + sizeof(struct msghead),
        ilMsglen - sizeof(struct msghead));
    }
    else
    {
      ilRc = swTcpsnd(igSockfd,alMsgbuf,ilMsglen);
    }
    if ( ilRc )
    {
      swVdebug(1,"S0250: [´íÎó/º¯Êýµ÷ÓÃ] swTcpsnd()º¯Êý ËÍ»Ø¿Í»§¶Ë³ö´í,·µ»ØÂë=%d",ilRc );
      close( igSocket );
      close( igSockfd );
      swMbclose();
      exit( -1 );
    }
    swVdebug(2,"S0260: ÒÑ³É¹¦ËÍ»Ø¿Í»§¶Ë!ilMsglen = %d",ilMsglen);
    if (cgDebug >= 2) 
    {
      swDebughex(alMsgbuf,ilMsglen);
    }
  }
}  


void swQuit(int sig)
{
  signal(SIGTERM,SIG_IGN);
  swMbclose();
  close(igSocket);
  close(igSockfd);
  exit(0);
}


