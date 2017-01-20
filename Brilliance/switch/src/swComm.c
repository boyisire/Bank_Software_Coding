/****************************************************************/
/* ģ����    ��swComm                                         */ 
/* ģ������    ��ͨѶģ��                                       */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/16                                      */
/* ����޸����ڣ�2001/9/16                                      */
/* ģ����;    ��                                               */
/* ��ģ���а������º���������˵����                             */
/*			  (1) void main()                       */
/*                        (3) void swStrupper()                 */
/*                        (4) int swLoadcomcfg()                */
/****************************************************************/
/****************************************************************/
/* �޸ļ�¼��                                                   */
/****************************************************************/

#include "swapi.h"
#include "swCompub.h"
#include "swComtcp.h"
#include "swPubfun.h"

int swLoadcfg( char *aPortName );

int main( int argc , char **argv )
{
  int ilRc;
  char alPortname[50];
  
  /* ��ӡ�汾�� */
  if(argc > 1)
    _swVersion("swComm Version 4.3.0",argv[1]);

  /*��������Ƿ���ȷ*/
  if ( argc != 2 )
  {
    printf("Usage: swComm portname \n");
    exit(0);
  }
  
  memset(alPortname,0x00,sizeof(alPortname));
  strcpy(alPortname,argv[1]);
  _swTrim(alPortname);
  sprintf(agDebugfile,"swCom%s.debug",alPortname);

  ilRc = swLoadcfg( alPortname );
  if ( ilRc )
  {
    swVdebug(0,"S0010: [����/��������] swLoadcfg()����,[%s]װ��ͨѶ���ò�������,������=%d",alPortname,ilRc);
    exit( 1 );
  }

  swVdebug(2,"S0020: [%s]װ��ͨѶ���ò����ɹ�!",argv[1]);
   
  /*ִ�ж�Ӧ��ģ��*/
  execlp(sgComcfg.aMode,sgComcfg.aMode,alPortname,(char *)0);
  return(0);
}

/****************************************************************/
/* �������    ��swLoadcfg                                      */ 
/* ��������    ��                                               */
/* ��	 ��    ��                                               */
/* ��������    ��2000/5/                                        */
/* ����޸����ڣ�2000/5/                                        */
/* ������;    ��                                               */
/* ��������ֵ  : �ɹ�   0                                       */
/*               ʧ��   1                                       */
/****************************************************************/

int swLoadcfg( char *aPortName )
{
  FILE  *fp;
  int  ilRc;
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  
  swVdebug(4,"S0030: [��������] swLoadcfg(%s)",aPortName);
  memset( (char *)&sgComcfg, 0x00, sizeof(sgComcfg));

  if((fp=_swFopen("config/SWCOMM.CFG","r"))==NULL)
  {
    swVdebug(0,"S0040: ��ͨѶ�����ļ�����!");
    return(-1);
  }

  /* Ԥ��ͨѶ�˿������ļ��������� */
  while (1)
  {
    memset(palFldvalue,0x0,sizeof(palFldvalue));
    printf("iFLDNUMBER=%d\n",iFLDNUMBER);
    ilRc = _swGetitemmode2(fp,palFldvalue,iFLDNUMBER);
    if (ilRc < 0) break;

    _swTrim( palFldvalue[0] );
    
    if ( strcmp( palFldvalue[0], aPortName ) == 0 )
    {
      strcpy( sgComcfg.aPort_name, palFldvalue[0] );
      strcpy( sgComcfg.aMode, palFldvalue[1] );
      
      sgComcfg.iDebug = atoi(palFldvalue[7]);
      cgDebug = sgComcfg.iDebug;

      swVdebug(3,"S0050: sgComcfg.aPort_name = [%s]",sgComcfg.aPort_name);
      swVdebug(3,"S0060: sgComcfg.aMode      = [%s]",sgComcfg.aMode);
 
      fclose(fp);
      swVdebug(4,"S0070: [��������] swLoadcfg()������=0");
      return(0);
    } /* end if */   
  } /* end while */
  swVdebug(0,"S0080: ��ȡͨѶ�����ļ�����!");
  fclose(fp);
  return(-1);
}

void swQuit()
{
}
