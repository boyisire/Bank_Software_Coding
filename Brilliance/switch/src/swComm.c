/****************************************************************/
/* 模块编号    ：swComm                                         */ 
/* 模块名称    ：通讯模块                                       */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/16                                      */
/* 最后修改日期：2001/9/16                                      */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/*			  (1) void main()                       */
/*                        (3) void swStrupper()                 */
/*                        (4) int swLoadcomcfg()                */
/****************************************************************/
/****************************************************************/
/* 修改记录：                                                   */
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
  
  /* 打印版本号 */
  if(argc > 1)
    _swVersion("swComm Version 4.3.0",argv[1]);

  /*检查输入是否正确*/
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
    swVdebug(0,"S0010: [错误/函数调用] swLoadcfg()函数,[%s]装入通讯配置参数出错,返回码=%d",alPortname,ilRc);
    exit( 1 );
  }

  swVdebug(2,"S0020: [%s]装入通讯配置参数成功!",argv[1]);
   
  /*执行对应的模块*/
  execlp(sgComcfg.aMode,sgComcfg.aMode,alPortname,(char *)0);
  return(0);
}

/****************************************************************/
/* 函数编号    ：swLoadcfg                                      */ 
/* 函数名称    ：                                               */
/* 作	 者    ：                                               */
/* 建立日期    ：2000/5/                                        */
/* 最后修改日期：2000/5/                                        */
/* 函数用途    ：                                               */
/* 函数返回值  : 成功   0                                       */
/*               失败   1                                       */
/****************************************************************/

int swLoadcfg( char *aPortName )
{
  FILE  *fp;
  int  ilRc;
  char palFldvalue[iFLDNUMBER][iFLDVALUELEN];
  
  swVdebug(4,"S0030: [函数调用] swLoadcfg(%s)",aPortName);
  memset( (char *)&sgComcfg, 0x00, sizeof(sgComcfg));

  if((fp=_swFopen("config/SWCOMM.CFG","r"))==NULL)
  {
    swVdebug(0,"S0040: 打开通讯配置文件出错!");
    return(-1);
  }

  /* 预读通讯端口配置文件到数组中 */
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
      swVdebug(4,"S0070: [函数返回] swLoadcfg()返回码=0");
      return(0);
    } /* end if */   
  } /* end while */
  swVdebug(0,"S0080: 读取通讯配置文件出错!");
  fclose(fp);
  return(-1);
}

void swQuit()
{
}
