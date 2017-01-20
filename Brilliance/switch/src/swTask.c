/****************************************************************/
/* Ä£¿é±àºÅ    £ºTASK                                           */
/* Ä£¿éÃû³Æ    £ºÈÎÎñ¹ÜÀí                                       */
/* °æ ±¾ ºÅ    £ºV4.3.0                                         */
/* ×÷    Õß    £º                                               */
/* ½¨Á¢ÈÕÆÚ    £º2001/7/18                                      */
/* ×îºóĞŞ¸ÄÈÕÆÚ£º2001/8/8                                       */
/* Ä£¿éÓÃÍ¾    £ºÆô¶¯Æ½Ì¨½ø³Ì,²¢¼à¿ØÆô¶¯½ø³ÌµÄ×´Ì¬,¶ÔÒì³£×÷´¦Àí */
/* ±¾Ä£¿éÖĞ°üº¬ÈçÏÂº¯Êı¼°¹¦ÄÜËµÃ÷£º                             */
/*               (1)void main();                                */
/*               (3)int  swExecSubProcess(); Æô¶¯×Ó½ø³Ì         */
/*               (4)int  swChkMailbox;       ¼ì²éÓÊÏä           */
/*               (5)int  swTaskmanage;       ¿ØÖÆÈÎÎñ¶àÆğ       */
/*               (6)int  swTasktimer;        ¶¨Ê±Æô¶¯¡¢¹Ø±ÕÈÎÎñ */
/*               (7)int  swChkSigkill();     ¼ì²é¹Ø±ÕĞÅºÅ       */ 
/*               (8)void swQuit();           ¹Ø±ÕÆ½Ì¨½ø³Ì       */
/****************************************************************/
#define NOSQL

/* switch¶¨Òå */
#include "switch.h"
#include <setjmp.h>
#include "swNdbstruct.h"
#include "swShm.h"

int swChkSigkill();
int swTasktimer();
int swTaskmanage();
int swChkMailbox();
int swExecSubProcess(int alFlag,struct swt_sys_task sSwt_sys_task);
int Parse(char *buf,char args[][101]);

/* ³£Á¿¶¨Òå */
#define  aINSTANCENAME  "swTask"
#define SHMNOTFOUND	100     /* Ã»ÕÒµ½¹²ÏíÄÚ´æ¼ÇÂ¼ */

/* ±äÁ¿¶¨Òå */
/*char  *agargs[50];*/ /* deleted by fzj at 2002.03.04 */
char agargs[15][101];  /* added by fzj at 2002.03.04 */
char *execvpargs[10];  /* added by fzj at 2002.03.04 */

/* º¯ÊıÔ­ĞÍ¶¨Òå */
void swSigcld(int iSig);
void swQuit(); 
 
/**************************************************************
 ** º¯ÊıÃû      £º main
 ** ¹¦  ÄÜ      £ºÖ÷º¯Êı
 ** ×÷  Õß      £º  
 ** ½¨Á¢ÈÕÆÚ    £º 
 ** ×îºóĞŞ¸ÄÈÕÆÚ£º2001/09/10 
 ** µ÷ÓÃÆäËüº¯Êı£º
 ** È«¾Ö±äÁ¿    £º
 ** ²ÎÊıº¬Òå    £º
 ** ·µ»ØÖµ      £º ÎŞ
***************************************************************/
int main(int argc,char *argv[])
{
  short ilRc;                                        /* ·µ»ØÖµ */   
  short ilCount;                                     /* ¼ÇÂ¼Êı */
  FILE *plPopenfile;
  int  i,j;                                          /* ¼ÆÊıÆ÷ */
  struct swt_sys_task *pslSwt_sys_task;/* add by nh 20020807 */
  FILE *pp;                            /* add by nh 20020925 */
  char alName[20],alCmd[100];          /* add by nh 20020925 */


  /* ´òÓ¡°æ±¾ºÅ */
  if (argc > 1)
    _swVersion("swTask Version 4.3.0",argv[1]);

  /* LICENSE */
  if (_swLicense("swTask")) exit(FAIL);

  /* ÉèÖÃµ÷ÊÔ³ÌĞòÃû³Æ */
  memset(agDebugfile, 0x00, sizeof(agDebugfile));
  strncpy(agDebugfile, "swTask.debug", sizeof(agDebugfile));

  /* È¡µÃDEBUG±êÖ¾ */
  if ((cgDebug = _swDebugflag("swTask")) == FAIL)
  {
    fprintf(stderr,"ÎŞ·¨È¡µÃDEBUG±êÖ¾!\n");
    exit(FAIL);
  } 

  /* add by nh 20020925 */
  pp=popen("id -un","r");
  fscanf(pp,"%s",alName);
  pclose(pp);
  sprintf(alCmd,"%s%s%s","ps -u ",alName,"|grep swTask|wc -l");
  swVdebug(2,"S0010: command=[%s]",alCmd);
  /* end add */
  plPopenfile=popen(alCmd,"r");
  fscanf(plPopenfile,"%d",&i);  
  swVdebug(2,"S0020: µ±Ç°ÓÃ»§µÄswTask½ø³Ì¸öÊıÎª[%d]",i);
  if (i>1)
  {
    swVdebug(1,"S0030: [´íÎó/ÏµÍ³µ÷ÓÃ] popen()º¯Êı,errno=%d,swTask½ø³ÌÒÑ´æÔÚ!",errno);
      pclose(plPopenfile);
    exit(0);
  }
  else
    pclose(plPopenfile);

  ilRc = qattach(iMBTASK);
  if (ilRc)
  {
    swVdebug(1,"S0040: [´íÎó/ÓÊÏä] qattack()º¯Êı,´íÎóÂë=%d,³õÊ¼»¯ÓÊÏä³ö´í",ilRc);
    exit(FAIL);
  }
  /* ĞÅºÅÉèÖÃ */
  /* 
  signal(SIGTERM, swQuit);              
  signal(SIGCLD, SIG_IGN);
  signal(SIGINT, SIG_IGN);
  signal(SIGQUIT, SIG_IGN);
  signal(SIGTSTP, SIG_IGN);
  signal(SIGHUP, SIG_IGN);
  */
  for(j=1;j<=64;j++)
    signal(j, SIG_IGN);
  
  signal(SIGSEGV,SIG_DFL); 
  signal(SIGTERM, swQuit);

  /* ³õÊ¼»¯¹²ÏíÄÚ´æÖ¸Õë */
  ilRc = swShmcheck();  
  if ( ilRc != SUCCESS )
  {
    swVdebug(0,"S0050: [´íÎó/¹²ÏíÄÚ´æ] ³õÊ¼»¯¹²ÏíÄÚ´æÖ¸ÕëÊ§°Ü");
    exit(FAIL);
  }

  swVdebug(2,"S0060: ³õÊ¼»¯¹²ÏíÄÚ´æÖ¸Õë³É¹¦");
 
  /* ´Ó¹²ÏíÄÚ´æÖĞÈ¡³öËùÓĞÈÎÎñ¼ÇÂ¼ */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_all_p(pslSwt_sys_task, &ilCount);*/
  ilRc = swShmselect_swt_sys_task_all_p(&pslSwt_sys_task, &ilCount);
  if ( ilRc != SUCCESS )
  {
    swVdebug(1,"S0070: [´íÎó/¹²ÏíÄÚ´æ] ²Ù×÷¹²ÏíÄÚ´æÈ¡ÈÎÎñ±í¼ÇÂ¼³ö´í");
    exit(FAIL);
  } 
  
  /* ¼ì²éËùÓĞËùÓĞÈÎÎñ */
  for ( i=0; i < ilCount; i++ )
  {
    memset(&sgSwt_sys_task, 0x00, sizeof(struct swt_sys_task));
    memcpy(&sgSwt_sys_task, &(pslSwt_sys_task[i]), sizeof(struct swt_sys_task));
    /* ÅĞ¶ÏÈÎÎñÊÇ·ñ¸ÃÆô */
    if (sgSwt_sys_task.task_use[0] !=cTRUE ||\
        strlen(sgSwt_sys_task.task_timer) || \
        sgSwt_sys_task.task_flag !=1 )
    {
      sgSwt_sys_task.pid = -1;
      sgSwt_sys_task.restart_num = 0;
      sgSwt_sys_task.start_time = 0; 
      sgSwt_sys_task.task_status[0] = cTASKDOWN; 
    }
    else 
    {
      if ( ( sgSwt_sys_task.pid > 0 ) && ( kill(sgSwt_sys_task.pid,0) == 0 ) )
      {
	kill(sgSwt_sys_task.pid, 9); 
        swVdebug(2,"S0080: kill½ø³Ì [%s]",sgSwt_sys_task.task_name );
       }
      sgSwt_sys_task.task_status[0] = cTASKRUNING; 
      sgSwt_sys_task.restart_num = 0;
      sgSwt_sys_task.start_time = 0;
      sgSwt_sys_task.pid = -1;      
      if ( sgSwt_sys_task.task_flag != 1 )
      {
        sgSwt_sys_task.task_flag = 0;
      }  
    }

    /* ¸üĞÂ¹²ÏíÄÚ´æÖĞÈÎÎñ×´Ì¬ */
    ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name, sgSwt_sys_task);
    if ( ilRc != SUCCESS ) 
    {
      swVdebug(1,"S0090:  [´íÎó/¹²ÏíÄÚ´æ] ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]Î´ÕÒµ½",sgSwt_sys_task.task_name); 
      continue; 
    }
  }

  /* Ñ¡ÔñËùÓĞÈÎÎñ×´Ì¬Îª cTASKRUNING µÄ¼ÇÂ¼,°´ start_id ÅÅĞò */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_mrec_status_t("1", pslSwt_sys_task,&ilCount);*/
  ilRc = swShmselect_swt_sys_task_mrec_status_t("1", &pslSwt_sys_task,&ilCount);
  if ( ilRc != SUCCESS )
  {
    swVdebug(1,"S0100: [´íÎó/¹²ÏíÄÚ´æ] ²Ù×÷¹²ÏíÄÚ´æ³ö´í");
    swQuit( );
  } 
  
  /* Æô¶¯ËùÓĞ×´Ì¬Îª cTASKRUNING µÄÈÎÎñ½ø³Ì */
  for ( i = 0; i < ilCount; i++ )
  {
    memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
    memcpy(&sgSwt_sys_task, &pslSwt_sys_task[i], sizeof(struct swt_sys_task));
    
    _swTrim(sgSwt_sys_task.task_name);
    _swTrim(sgSwt_sys_task.task_file);
    Parse (sgSwt_sys_task.task_file, agargs);

    /* Æô¶¯×Ó½ø³Ì */
    swExecSubProcess( 1,sgSwt_sys_task );  
  }
  swVdebug(2,"S0110: ¼ì²é¡¢Æô¶¯Æ½Ì¨½ø³Ì³É¹¦!");
 
  /* ´ò¿ª SIGCLD ĞÅºÅ */
  signal(SIGCLD, swSigcld ); 

  for(;;) 
  {
    /* ÖØĞÂ³õÊ¼»¯¹²ÏíÄÚ´æÖ¸Õë */
    ilRc = swShmcheck(); 
    if (ilRc != SUCCESS)
    {
      swVdebug(0,"S0120: [´íÎó/¹²ÏíÄÚ´æ] ¼ì²é¹²ÏíÄÚ´æ³ö´í");
      swQuit();
    }

    /* ÒÔ·Ç×èÈû·½Ê½¼ì²éÓÊÏä£¬´¦Àí Begin and End ÃüÁî */
    swChkMailbox(); 
  
    /* ´¦ÀíÈÎÎñ¶àÆğ  */
    /*
    swTaskmanage();
    */
 
    /* ´¦Àí¶¨Ê±Æô¶¯¡¢¹Ø±ÕÈÎÎñ */
    /*
    swTasktimer();
    */
  }
}

/**************************************************************
 ** º¯ÊıÃû      :  swChkMailbox
 ** ¹¦  ÄÜ      :  ¼ì²âÈÎÎñ¹ÜÀíÓÊÏä,´¦Àíbegin¡¢endÃüÁî
 ** ×÷  Õß      :
 ** ½¨Á¢ÈÕÆÚ    :
 ** ×îºóĞŞ¸ÄÈÕÆÚ: 2001/08/15
 ** µ÷ÓÃÆäËüº¯Êı:
 ** È«¾Ö±äÁ¿    :
 ** ²ÎÊıº¬Òå    :
 ** ·µ»ØÖµ      :  SUCCESS , FAIL
***************************************************************/
int swChkMailbox()
{
  unsigned int ilMsglen, ilOrgqid, ilPriority;
  short  ilClass, ilType;
  short  ilSigkill;
  short  ilRc;
  char   alTaskname[51];
  long   llPid;
  struct  msgpack  slMsgpack;  /* ±¨ÎÄ¶¨Òå */

  ilMsglen = iMSGMAXLEN;
  ilPriority = 0;
  ilClass = 0;
  ilType = 0;

  swVdebug(4,"S0130: [º¯Êıµ÷ÓÃ] swChkMailbox()");

  ilRc =  qreadnw((char *)&slMsgpack, &ilMsglen, &ilOrgqid, \
                &ilPriority, &ilClass, &ilType, iTASKTIMEOUT);
  if( ilRc == SUCCESS)  /* ÓĞ±¨ÎÄ */
  {
    
    memset(alTaskname, 0x00, sizeof(alTaskname));
    memcpy(alTaskname, slMsgpack.aMsgbody, slMsgpack.sMsghead.iBodylen);
    slMsgpack.aMsgbody[slMsgpack.sMsghead.iBodylen]='\0';
    switch(slMsgpack.sMsghead.lCode)
    {
      case 701:            /* Æô¶¯½ø³Ì */
        memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
        ilRc = swShmselect_swt_sys_task(slMsgpack.aMsgbody, &sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0140: [´íÎó/¹²ÏíÄÚ´æ] ÈÎÎñ [%s] ²»´æÔÚ!", sgSwt_sys_task.task_name);
          return(FAIL); 
        }

        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);

        if (sgSwt_sys_task.task_use[0] != '1')
        {
          swVdebug(1,"S0150: [´íÎó/ÆäËü] [%s]½ø³ÌÉè¶¨Îª³õÊ¼²»Æô¶¯",sgSwt_sys_task.task_name);
          return(FAIL);
        }

        if (sgSwt_sys_task.pid <= 0)
        {
          _swTrim(sgSwt_sys_task.task_name);
          _swTrim(sgSwt_sys_task.task_file);
          Parse(sgSwt_sys_task.task_file, agargs);
          sgSwt_sys_task.restart_num = 0;

          /* Æô¶¯×Ó½ø³Ì */
          swExecSubProcess( 2,sgSwt_sys_task );
        }
        break;

      case 702:      /* ¹Ø±Õ½ø³Ì */
        memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
        ilRc = swShmselect_swt_sys_task(alTaskname, &sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0160: [´íÎó/¹²ÏíÄÚ´æ] ÈÎÎñ [%s] ²»´æÔÚ!", sgSwt_sys_task.task_name);
          return(FAIL);
        }
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        sgSwt_sys_task.restart_num = 0;
        sgSwt_sys_task.start_time = 0;

        if (sgSwt_sys_task.pid > 0)
        {
          /* Í£Ö¹½ø³Ì */
          ilSigkill = swChkSigkill();
          /* sleep(sgSwt_sys_task.stop_wait);*/
          swVdebug(2,"S0170: END ½ø³Ì[%s] ", sgSwt_sys_task.task_name);
 
          llPid = sgSwt_sys_task.pid;
          sgSwt_sys_task.pid = -1;
          strcpy(sgSwt_sys_task.task_status, "4");

/*add by gxz 2001.09.11 Begin */
          if ( sgSwt_sys_task.task_flag == 9 )
            sgSwt_sys_task.task_flag=0;
/*add by gxz 2001.09.11 End */

          /* ¸üĞÂ¹²ÏíÄÚ´æÈÎÎñ×´Ì¬ */
          ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
                 sgSwt_sys_task);
          if (ilRc != SUCCESS)
          {
            swVdebug(1,"S0180: [´íÎó/¹²ÏíÄÚ´æ] ¸üĞÂ¹²ÏíÄÚ´æÈÎÎñ[%s]×´Ì¬Ê§°Ü",\
              sgSwt_sys_task.task_name); 
            break;
          }
          kill(llPid, ilSigkill);
        }
        break;
      default: 
        swVdebug(1,"S0190: [´íÎó/ÆäËü] ÊÕµ½Î´ÖªÃüÁî±¨ÎÄ[%ld]", slMsgpack.sMsghead.lCode); 
          break;
    } 
  }
  return(0);
}

/**************************************************************
 ** º¯ÊıÃû       :  swTaskmanage
 ** ¹¦  ÄÜ       :  ´¦ÀíÈÎÎñ¶àÆğ
 ** ×÷  Õß       :  ¹ËÏşÖÒ
 ** ½¨Á¢ÈÕÆÚ     :  2001/09/10
 ** ×îºóĞŞ¸ÄÈÕÆÚ : 
 ** µ÷ÓÃÆäËüº¯Êı :
 ** È«¾Ö±äÁ¿     :
 ** ²ÎÊıº¬Òå     :
 ** ·µ»ØÖµ       :  SUCCESS , FAIL
**************************************************************/
int swTaskmanage()
{
  short ilQid,ilRc;
  short ilSigkill;                                   /* ¹Ø±Õ½ø³ÌĞÅºÅ */
  long  llPid;
  struct mbinfo slMbinfo;

  swVdebug(4,"S0200: [º¯Êıµ÷ÓÃ] swTaskmanage()");

  ilQid=1;
  while((ilRc = bmqGetmbinfo(ilQid,&slMbinfo)) != 100)
  {
    if ( ilRc == -1 )
    {
      swVdebug(1,"S0210: [´íÎó/ÓÊÏä] bmqGetmbinfo()º¯Êı,´íÎóÂë=%d",ilRc);
      return(FAIL);
    }
    
    if ( slMbinfo.iStatus == 1 )
    {
      memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
      ilRc=swShmselect_swt_sys_task_qid( ilQid, 9, &sgSwt_sys_task );
      if ( ilRc == 0 )
      {
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        sgSwt_sys_task.restart_num = 0;
        sgSwt_sys_task.start_time = 0;

        if (sgSwt_sys_task.pid > 0)
        {
          /* Í£Ö¹½ø³Ì */
          ilSigkill = swChkSigkill();
          llPid = sgSwt_sys_task.pid;
          swVdebug(2,"S0220: END ½ø³Ì[%s] ", sgSwt_sys_task.task_name);
 
          sgSwt_sys_task.pid = -1;
          strcpy(sgSwt_sys_task.task_status, "4");
          sgSwt_sys_task.task_flag =0;

          /* ¸üĞÂ¹²ÏíÄÚ´æÈÎÎñ×´Ì¬ */
          ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
               sgSwt_sys_task);
          if (ilRc != SUCCESS)
          {
            swVdebug(1,"S0230: [´íÎó/¹²ÏíÄÚ´æ] ¸üĞÂ¹²ÏíÄÚ´æÈÎÎñ×´Ì¬Ê§°Ü");
          }
          kill(llPid, ilSigkill);
        }
      }
    }
    else if ( slMbinfo.iStatus == 3 )
    {
      memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
      ilRc=swShmselect_swt_sys_task_qid( ilQid, 0, &sgSwt_sys_task );
      if ( ilRc == 0 )
      {
   /*     if (sgSwt_sys_task.task_use[0] != '1')
        {
          swVdebug(2,"S0240: [%s] ½ø³ÌÉè¶¨Îª³õÊ¼²»Æô¶¯!",
            sgSwt_sys_task.task_name);
          break;
        }
   */
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        Parse(sgSwt_sys_task.task_file, agargs);
        swExecSubProcess( 2,sgSwt_sys_task );
      }
    }
    ilQid++;
  }

  swVdebug(4,"S0250: [º¯Êı·µ»Ø] swTaskmanage()·µ»ØÂë=0");
  return(SUCCESS);
}  

/**************************************************************
 ** º¯ÊıÃû       :  swTasktimer
 ** ¹¦  ÄÜ       :  ´¦ÀíÈÎÎñµÄ¶¨Ê±Æô¶¯¡¢¹Ø±Õî
 ** ×÷  Õß       :
 ** ½¨Á¢ÈÕÆÚ     :
 ** ×îºóĞŞ¸ÄÈÕÆÚ :  2001/08/15
 ** µ÷ÓÃÆäËüº¯Êı :
 ** È«¾Ö±äÁ¿     :
 ** ²ÎÊıº¬Òå     :
 ** ·µ»ØÖµ       :  SUCCESS , FAIL
**************************************************************/
int swTasktimer()
{
  short  i,ilSigkill;
  short  ilRc;
  char   alResult[iFLDVALUELEN + 1]  ;
  unsigned int ilLength;
  short  ilCount;
  /* del by nh 20020807
  struct swt_sys_task pslSwt_sys_task[iMAXTASKREC];*/
  struct swt_sys_task *pslSwt_sys_task;/* add by nh 20020807*/
  
  swVdebug(4,"S0260: [º¯Êıµ÷ÓÃ] swTasktimer()");

  /* ²éÕÒ task_use = "1" µÄ¼ÇÂ¼ */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_mrec_use_t("1",pslSwt_sys_task,&ilCount);*/
  ilRc = swShmselect_swt_sys_task_mrec_use_t("1",&pslSwt_sys_task,&ilCount);
  if (ilRc != SUCCESS)
  {
    swVdebug(1,"S0270: [´íÎó/¹²ÏíÄÚ´æ] ÔÚ¹²ÏíÄÚ´æÖĞ²éÕÒ¿ÉÓÃÈÎÎñ³ö´í!");
    swQuit();
  }
  
  /* ÂÖÑ¯ÈÎÎñ±í */
  for (i=0; i < ilCount; i++)
  {
    memcpy(&sgSwt_sys_task,&pslSwt_sys_task[i],sizeof(struct swt_sys_task));
    
    _swTrim(sgSwt_sys_task.task_timer);

    /* ¼ì²éÈÎÎñ×´Ì¬  */
    if (sgSwt_sys_task.pid > 0)
    {
      /* kill(pid,0)¼ì²â½ø³Ì×´Ì¬,Ë¢ĞÂÈÎÎñ±í */
      ilRc = kill(sgSwt_sys_task.pid, 0);
      if ( ilRc ) 
      {
        strcpy(sgSwt_sys_task.task_status, "4");
        sgSwt_sys_task.pid = -1;

        /* add by gxz 2001.09.11 Begin */
        if ( sgSwt_sys_task.task_flag != 1 )
          sgSwt_sys_task.task_flag = 0;
        /* add by gxz 2001.09.11 End */

        /* ¸üĞÂ¹²ÏíÄÚ´æ */
        ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
                 sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0280: [´íÎó/¹²ÏíÄÚ´æ] ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]Î´ÕÒµ½",\
            sgSwt_sys_task.task_name); 
          continue; 
        }
      }
    }

    if (sgSwt_sys_task.task_timer[0]) /* ¶¨Ê±Æô¶¯¡¢¹Ø±Õ */
    {
      ilRc = _swExpress(sgSwt_sys_task.task_timer, alResult,&ilLength);
      if (ilRc == FAIL)
      {
        swVdebug(1,"S0290: [´íÎó/º¯Êıµ÷ÓÃ] _swExpress()º¯Êı,·µ»ØÂë=%d,¶¨Ê±Æô¶¯±í´ïÊ½[%s]¼ÆËã³ö´í",ilRc,sgSwt_sys_task.task_timer);
        continue ;
      }

      /* Âú×ãÆô¶¯Ìõ¼ş */
      if ((alResult[0] == '1') && (sgSwt_sys_task.pid <= 0))
      {
        _swTrim(sgSwt_sys_task.task_name);
        _swTrim(sgSwt_sys_task.task_file);
        Parse(sgSwt_sys_task.task_file, agargs);
/*        signal(SIGCLD,SIG_IGN);*/

        /* Æô¶¯ÈÎÎñ£¬¸ü¸ÄÈÎÎñ±í */
        swExecSubProcess( 2,sgSwt_sys_task );
/*        signal(SIGCLD,swSigcld);*/
        continue;
      }
      if (alResult[0] == '0' && sgSwt_sys_task.pid > 0) /* Âú×ã¹Ø±ÕÌõ¼ş */
      {
        swVdebug(2,"S0300: ¶¨Ê±¹Ø±Õ½ø³Ì[%s]", sgSwt_sys_task.task_name);
 
        /* ¹Ø±ÕÈÎÎñ */
        ilSigkill = swChkSigkill();
/*        signal(SIGCLD, SIG_IGN);*/
        kill(sgSwt_sys_task.pid, ilSigkill);
        sleep(sgSwt_sys_task.stop_wait);
  /*      signal(SIGCLD, swSigcld);*/

        sgSwt_sys_task.pid = -1;
        strcpy(sgSwt_sys_task.task_status, "4");
        sgSwt_sys_task.restart_num = 0; /* very important */
        sgSwt_sys_task.start_time = 0; /* very important */

        /* ¸üĞÂ¹²ÏíÄÚ´æ */
        ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name,
            sgSwt_sys_task);
        if (ilRc != SUCCESS)
        {
          swVdebug(1,"S0310: [´íÎó/¹²ÏíÄÚ´æ] ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]Î´ÕÒµ½",sgSwt_sys_task.task_name); 
          continue; 
        }
      }
    } /* end ¶¨Ê±Æô¶¯¡¢¹Ø±Õ */
  } /* end for */
  swVdebug(4,"S0320: [º¯Êı·µ»Ø] swTasktimer()·µ»ØÂë=0");
  return( SUCCESS);
} 
     
/**************************************************************
 ** º¯ÊıÃû:  short swExecSubProcess( )
 ** ¹¦  ÄÜ       :  Æô¶¯Æ½Ì¨½ø³Ì
 ** ×÷  Õß       :
 ** ½¨Á¢ÈÕÆÚ     :
 ** ×îºóĞŞ¸ÄÈÕÆÚ :  2001/09/05
 ** ĞŞ¸ÄÈË       £º ¹ËÏşÖÒ
 ** µ÷ÓÃÆäËüº¯Êı :
 ** È«¾Ö±äÁ¿     :
 ** ²ÎÊıº¬Òå     :  1--Æô½»»»Æ½Ì¨Ê±    2--ÆôÈÎÎñÊ± 
 ** ·µ»ØÖµ       :  SUCCESS = 0 , FAIL = -1 
***************************************************************/
int swExecSubProcess( int alFlag,struct swt_sys_task sSwt_sys_task  )
{
  short i,ilRc;
  short ilResult;
  long  llBegintime;
  long  llPid;

  swVdebug(4,"S0330: [º¯Êıµ÷ÓÃ] swExecSubProcess(%d)",alFlag);

  /*  Ö´ĞĞÎÄ¼ş²»´æÔÚ»òÎŞÖ´ĞĞÈ¨ÏŞ */
  if ( (access(agargs[0], F_OK)) == -1 ||  (access(agargs[0], X_OK)) == -1 )
  {
    swVdebug(1,"S0340: [´íÎó/ÆäËü] Ó¦ÓÃ³ÌĞò[%s]ÎŞ·¨Æô¶¯!",agargs[0]);
    strcpy(sSwt_sys_task.task_status, "5");
    sSwt_sys_task.start_time = 0;
    sSwt_sys_task.pid = -1;

    /* ¸üĞÂ¹²ÏíÄÚ´æ */
    ilRc = swShmupdate_swt_sys_task(sSwt_sys_task.task_name, sSwt_sys_task);
    if (ilRc != SUCCESS)
    {
      swVdebug(1,"S0350: [´íÎó/¹²ÏíÄÚ´æ] ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]Î´ÕÒµ½",sSwt_sys_task.task_name); 
      return(FAIL);
    }
    return(FAIL);
  }
  /* Æô¶¯×Ó½ø³Ì */
  if ((llPid = fork()) < 0)  
  {
    swVdebug(1,"S0360: [´íÎó/ÏµÍ³µ÷ÓÃ] fork()º¯Êı,errno=%d,fork×Ó½ø³ÌÊ§°Ü! ",errno);
    /* swQuit(); delete by szhengye 2001.10.30 */
    return(-1);
  }
  if (llPid == 0)  
  { 
    swVdebug(2,"agargs=[%s][%s][%s][%s]",agargs[0],agargs[1],agargs[2],agargs[3]);
    /* execvp( *agargs, agargs ); */ /* delete by fzj at 2002.03.04 */
    /* begin of added by fzj at 2002.03.04 */
    for (i=0;i<15;i++) 
    {
      if (agargs[i][0] == '\0')
      {
      	execvpargs[i] = NULL;
      	break;
      }	
      execvpargs[i] = (char *)(&agargs[i][0]);
    }  
    execvp(*execvpargs, execvpargs);
    /* end of added by fzj at 2002.03.04 */
    exit(0);   
  }
  /* ¼ì²é½ø³ÌÊÇ·ñÆô¶¯³É¹¦ */
  if ( alFlag == 1 )
    sleep(sSwt_sys_task.start_wait);
  else 
    sleep(1);

  ilRc = kill(llPid, 0);
  if( ilRc == -1 )
  {
    sSwt_sys_task.task_status[0] = cTASKDOWN;
    sSwt_sys_task.start_time = 0; 
    swVdebug(1,"S0370: [´íÎó/ÆäËü] Æô¶¯ [%s] Ê§°Ü", sSwt_sys_task.task_name);
    ilResult = FAIL;
  }   
  else      /* Ö´ĞĞ½ø³Ì³É¹¦£¬ĞŞ¸ÄÈÎÎñ×´Ì¬ */
  {
    time(&llBegintime); 
    sSwt_sys_task.pid = llPid;
    sSwt_sys_task.task_status[0] = cTASKRUNED;

/* add by gxz 2001.09.11 Begin */
    if ( alFlag != 1 )
    {
      if ( sSwt_sys_task.task_flag != 1 )
      {
        sSwt_sys_task.task_flag =9;
      }
    }
/* add by gxz 2001.09.11 End */
       
    sSwt_sys_task.start_time = llBegintime; /* very important */
    swVdebug(2,"S0380: Æô¶¯[%s]³É¹¦,PID=%ld",sSwt_sys_task.task_name,llPid);
    ilResult = SUCCESS;
  }

  /* ¸üĞÂ¹²ÏíÄÚ´æ */
  ilRc = swShmupdate_swt_sys_task(sSwt_sys_task.task_name, sSwt_sys_task);
  if ( ilRc != SUCCESS )
  {
    swVdebug(1,"S0390: [´íÎó/¹²ÏíÄÚ´æ] ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]Î´ÕÒµ½",\
      sSwt_sys_task.task_name); 
    return(FAIL);
  }
  swVdebug(4,"S0400: [º¯Êı·µ»Ø] swExecSubProcess()·µ»ØÂë=%d",ilResult);
  return(ilResult); 
}


/**************************************************************
 ** º¯ÊıÃû:  void swSigcld()
 ** ¹¦  ÄÜ:  ´¦Àí SIGCLD ĞÅºÅ 
 ** ×÷  Õß:
 ** ½¨Á¢ÈÕÆÚ:
 ** ×îºóĞŞ¸ÄÈÕÆÚ: 2001/08/15
 ** µ÷ÓÃÆäËüº¯Êı:
 ** È«¾Ö±äÁ¿:
 ** ²ÎÊıº¬Òå:
 ** ·µ»ØÖµ:  
***************************************************************/
void swSigcld(int iSig)
{
  pid_t llPid;
  int ilStat;
  int ilRc;
  long llPidcld;
  struct swt_sys_task slSwt_sys_task;

  swVdebug(4,"S0410: [º¯Êıµ÷ÓÃ] swSigcld(%d)",iSig);

/*  signal(SIGCLD, SIG_IGN);*/
  swDebug("szhengye:swSigcld:%d",iSig);

  while ((llPid = waitpid(0, &ilStat, WNOHANG)) > 0)
  {
    swVdebug(2,"S0420: ×Ó½ø³ÌÍË³ö Waitpid(),pid=[%ld]",llPid);
    llPidcld = llPid;
    /* ²éÕÒÒÑÖĞ¶ÏµÄÈÎÎñ */

    ilRc = swShmselect_swt_sys_task_pid(llPidcld, &slSwt_sys_task);
    if (ilRc != SUCCESS) 
    {
      swVdebug(2,"S0430: ²éÕÒ PID = [%ld] µÄÈÎÎñÊ§°Ü !", llPidcld);
      continue;
    }
    
    /* ÅĞ¶ÏÊÇ·ñÖ§³ÖÖØÆô */
    if (slSwt_sys_task.restart_flag[0] == '1') 
    { 
      /*±È½ÏÊÇ·ñ´ïµ½×î´óÖØÆğ´ÎÊı*/ 
      if (slSwt_sys_task.restart_max > slSwt_sys_task.restart_num) 
      {
        slSwt_sys_task.restart_num++; 

        /* ½âÎö½ø³Ì³ÌĞòÂ·¾¶ */
        _swTrim(slSwt_sys_task.task_name); 
        _swTrim(slSwt_sys_task.task_file);
        Parse(slSwt_sys_task.task_file, agargs);

        /* Æô¶¯×Ó½ø³Ì */
        swExecSubProcess( 2,slSwt_sys_task );  
      }
      /* fzj: 2002.9.17: ½ø³ÌÖØÆğ´ÎÊı´ïµ½×î´ó´ÎÊıÊ±Ó¦½«×´Ì¬ÖÃDOWN */
      else
      {
        slSwt_sys_task.pid = -1;
        slSwt_sys_task.start_time = 0; 
        slSwt_sys_task.task_status[0] = cTASKDOWN; 
        ilRc=swShmupdate_swt_sys_task(slSwt_sys_task.task_name,slSwt_sys_task);
        if (ilRc) 
        {
          swVdebug(1,"S0440: [´íÎó/¹²ÏíÄÚ´æ] ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]Î´ÕÒµ½",\
            slSwt_sys_task.task_name); 
          continue;
        }
      }
      /**/
      /* add by ¹ËÏşÖÒ 2001.09.05 Begin */
      /* ´¦Àí¶¨Ê±Æô¶¯¡¢¹Ø±ÕÈÎÎñ */
      /*
      swTasktimer();
      */
      /* add by ¹ËÏşÖÒ 2001.09.05 End */
    }
    else /* ²»Ö§³Ö×Ô¶¯ÖØÆô */
    {
      slSwt_sys_task.pid = -1;
      slSwt_sys_task.start_time = 0;
      strcpy(slSwt_sys_task.task_status, "5");
      swVdebug(2,"S0450: [%s] Éè¶¨Îª²»¿ÉÖØÆğ£¬ ²»×Ô¶¯ÖØÆğ",\
        slSwt_sys_task.task_name);

      /* add by gxz 2001.09.11 Begin */
      if ( slSwt_sys_task.task_flag != 1 )
        slSwt_sys_task.task_flag = 0;
      /* add by gxz 2001.09.11 End */

      /* ¸üĞÂ¹²ÏíÄÚ´æ */
      ilRc = swShmupdate_swt_sys_task(slSwt_sys_task.task_name, slSwt_sys_task);
      if (ilRc) 
      {
        swVdebug(1,"S0460: [´íÎó/¹²ÏíÄÚ´æ] ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]Î´ÕÒµ½",\
          slSwt_sys_task.task_name); 
        continue;
      }
    }
  }  /* end of while */
  signal( SIGCLD, swSigcld );
}

/**************************************************************
 ** º¯ÊıÃû:  int Parse()
 ** ¹¦  ÄÜ:
 ** ×÷  Õß:
 ** ½¨Á¢ÈÕÆÚ:
 ** ×îºóĞŞ¸ÄÈÕÆÚ:
 ** µ÷ÓÃÆäËüº¯Êı:
 ** È«¾Ö±äÁ¿:
 ** ²ÎÊıº¬Òå:
 ** ·µ»ØÖµ:  SUCCESS , FAIL
***************************************************************/
/* begin of deleted by fzj at 2002.03.04 */
/*
int Parse(buf,args)
char *buf;
char * * args;
{
  char *buf1;

  swVdebug(4,"S0470: [º¯Êıµ÷ÓÃ] Parse()");

  buf1 = (char *) malloc(1024); 
  memcpy(buf1, buf, 1023);
  
  while (*buf1 != '\0')
  {
    while ((*buf1 == ' ') || (*buf1 == '\t') || (*buf1 == '\n'))
      *buf1++ = '\0';

    * args++ = buf1;

    while ((*buf1 != '\0') && (*buf1 != ' ') && (*buf1 != '\t') \
           && (*buf1 != '\n'))
      buf1++;
  }
  *args = '\0';
  return(0);
}
*/
/* end of deleted by fzj at 2002.03.04 */

/* begin of added by fzj at 2002.03.04 */
int Parse(char *buf, char args[][101])
{
  int i,ilLen;
  char *plBuf,*plArg;

  for(i=0;i<10;i++) args[i][0] = '\0';

  i = 0;
  plBuf = buf;
  while(*plBuf != '\0')
  {
    while(*plBuf==' ' || *plBuf=='\t' || *plBuf=='\n')
    {
      plBuf++;
      continue;
    }
    plArg = plBuf;
    ilLen = 0;
    while(*plBuf!=' ' && *plBuf!='\t' && *plBuf!='\n')
    {
      if(*plBuf=='\0') break;
      ilLen++;
      plBuf++;
      continue;
    }
    memcpy(args[i],plArg,ilLen);
    args[i][ilLen] = '\0';
    i++;
  }
  return(0);
}
/* end of added by fzj at 2002.03.04 */

/**************************************************************
 ** º¯ÊıÃû:  int swChkSigkill()
 ** ¹¦  ÄÜ:  È¡µÃÉè¶¨Æ½Ì¨½ø³ÌÖĞµÄ kill_id
 ** ×÷  Õß:
 ** ½¨Á¢ÈÕÆÚ:
 ** ×îºóĞŞ¸ÄÈÕÆÚ:
 ** µ÷ÓÃÆäËüº¯Êı:
 ** È«¾Ö±äÁ¿:
 ** ²ÎÊıº¬Òå:
 ** ·µ»ØÖµ:  ·µ»Øµ±Ç°µÄÆ½Ì¨½ø³ÌÖ® kill_id Éè¶¨Öµ£¬Èç¿ÕÔòÎª 9
***************************************************************/
int swChkSigkill()
{
  short ilRc;

  swVdebug(4,"S0480: [º¯Êıµ÷ÓÃ] swChkSigkill()");

  if ((sgSwt_sys_task.kill_id == 0))
  {
    ilRc = 9;
  }
  else
  {
    ilRc = sgSwt_sys_task.kill_id;
  }

  swVdebug(4,"S0490: [º¯Êı·µ»Ø] swChkSigkill()·µ»ØÂë=%d",ilRc);
  return(ilRc);
}

/**************************************************************
 ** º¯ÊıÃû:  void swQuit()
 ** ¹¦  ÄÜ:  ¹Ø±ÕÇ°ÖÃ½ø³Ì
 ** ×÷  Õß:
 ** ½¨Á¢ÈÕÆÚ:
 ** ×îºóĞŞ¸ÄÈÕÆÚ: 2001/08/15
 ** µ÷ÓÃÆäËüº¯Êı:
 ** È«¾Ö±äÁ¿: sgSwt_sys_task
 ** ²ÎÊıº¬Òå:
 ** ·µ»ØÖµ:
***************************************************************/
void   swQuit()                          
{
  short  ilSigkill;
  short  ilRc;
  short  i;
  /* del by nh 20020807
  struct swt_sys_task pslSwt_sys_task[iMAXTASKREC];*/
  struct swt_sys_task *pslSwt_sys_task;/* add by nh 20020807*/
  short ilCount;  

  signal(SIGCLD, SIG_IGN);
  swVdebug(1,"S1111: ÈÎÎñ¹ÜÀí½ø³ÌÍË³ö");
  
  qdetach();
 
  /* Ñ¡ÔñËùÓĞ¼ÇÂ¼£¬°´ stop_id ÅÅĞò */
  /* modify by nh 20020807
  ilRc = swShmselect_swt_sys_task_all_p(pslSwt_sys_task, &ilCount);*/
  ilRc = swShmselect_swt_sys_task_all_p(&pslSwt_sys_task, &ilCount);
  if (ilRc)
  {
    swVdebug(2,"S0500: ²Ù×÷¹²ÏíÄÚ´æÈÎÎñ±í³ö´í");
    exit (-1);
  }
  for (i = 0; i < ilCount; i++)
  {
    memset(&sgSwt_sys_task, 0x00, sizeof(sgSwt_sys_task));
    memcpy(&sgSwt_sys_task, &pslSwt_sys_task[i], sizeof(struct swt_sys_task));
    /* ¹Ø±ÕÒÑÆô¶¯µÄÆ½Ì¨½ø³Ì */
    if (sgSwt_sys_task.pid > 0)
    {
      _swTrim(sgSwt_sys_task.task_name);
      _swTrim(sgSwt_sys_task.task_file);
      Parse(sgSwt_sys_task.task_file, agargs);

      ilRc = kill(sgSwt_sys_task.pid, 0);
      if ( ilRc == 0)
      {
	ilSigkill = swChkSigkill();
	kill(sgSwt_sys_task.pid, ilSigkill); 
	swVdebug(2,"S0510: kill [%s] ½ø³Ì", sgSwt_sys_task.task_name);
        sleep ( sgSwt_sys_task.stop_wait );
      }
    }

    strcpy(sgSwt_sys_task.task_status, "4");
    sgSwt_sys_task.pid = -1;
    sgSwt_sys_task.start_time = 0;
    sgSwt_sys_task.restart_num = 0;

/* add by gxz 2001.09.11 Begin */
    if ( sgSwt_sys_task.task_flag != 1 )
      sgSwt_sys_task.task_flag = 0;
/* add by gxz 2001.09.11 End */

    /* ¸üĞÂ¹²ÏíÄÚ´æÖĞÈÎÎñ×´Ì¬ */
    ilRc = swShmupdate_swt_sys_task(sgSwt_sys_task.task_name, sgSwt_sys_task);
    if (ilRc) 
    {
      swVdebug(1,"S0520: [´íÎó/¹²ÏíÄÚ´æ] ¸üĞÂ¹²ÏíÄÚ´æÖĞÈÎÎñ[%s]×´Ì¬³ö´í",\
        sgSwt_sys_task.task_name); 
      continue;
    }
  }
  swVdebug(2,"S0530: ¹Ø±Õ½»»»Æ½Ì¨½ø³ÌÍê±Ï");
  exit (0);
}         

