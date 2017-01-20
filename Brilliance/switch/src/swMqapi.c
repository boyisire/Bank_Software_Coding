#include "switch.h"

/***  Prototype declarations for DMQAPI

int32  qattach(short);
int32  qwrite(char *, short, short, short);
int32  qread(char *, short *, short *);
int32  qdetach(void);
int32  _qwrite2(char *, short, short, short, short, short);
int32  qread2(char *, short *, short *, short, short, short);
int32  qreadnw(char *, short *, short *,short *, short *, short *, short *, short *);

End of prototype declarations      ***/

/**************************************************************
 ** 函数名      : qattach
 ** 功  能      : 邮箱初始化
***************************************************************/
int qattach(short qid)
{
  return(bmqOpen(qid));
}

/**************************************************************
 ** 函数名      : qwrite
 ** 功  能      : 写邮箱
***************************************************************/
int qwrite(char *msg_area, unsigned int msg_size, short qdest)
{
  return(bmqPut(0,qdest,0,0,0,msg_area,msg_size));
}

/**************************************************************
 ** 函数名      : qwrite
 ** 功  能      : 读邮箱
***************************************************************/
int qread(char *msg_area, unsigned int *msg_size, short *qid)
{
  short ilGrpid;
  short ilPrior;
  long llType;
  long llClass;

  return(bmqGetw(&ilGrpid,qid,&ilPrior,&llType,&llClass,msg_area,msg_size,0));
}

/**************************************************************
 ** 函数名      : qdetach
 ** 功  能      : 邮箱终止化
***************************************************************/
int qdetach( )
{
  return(bmqClose());
}

/**************************************************************
 ** 函数名      : _qwrite2
 ** 功  能      : 写邮箱
***************************************************************/
int _qwrite2(char *msg_area, unsigned int msg_size, short qid, 
short mpriority, short mclass, short mtype)
{
  long llType,llClass;
  
  llType = mtype;
  llClass = mclass;
  
  return(bmqPut(0,qid,mpriority,llType,llClass,msg_area,msg_size));
}

/**************************************************************
 ** 函数名      : qwrite2
 ** 功  能      : 写邮箱
***************************************************************/
int qwrite2(char *msg_area, unsigned int msg_size, short qid, 
short mpriority, short mclass, short mtype)
{
  long llType,llClass;
  
  llType = mtype;
  llClass = mclass;
  return(bmqPut(0,qid,mpriority,llType,llClass,msg_area,msg_size));
}

/**************************************************************
 ** 函数名      : qread2
 ** 功  能      : 阻塞读邮箱
***************************************************************/
int qread2(char *msg_area, unsigned int *msg_size, short *qid,
short *mpriority, short *mclass, short *mtype)
{
  short ilGrpid;
  long llType;
  long llClass;
  int  ilRc;
  
  llType  = (long)*mtype;
  llClass = (long)*mclass;
  ilRc = bmqGetw(&ilGrpid,qid,mpriority,&llType,&llClass,msg_area,msg_size,0);
  *mtype  = (short)llType;
  *mclass = (short)llClass;
  return(ilRc);
}

/**************************************************************
 ** 函数名      : qreadnw
 ** 功  能      : 非阻塞读邮箱
***************************************************************/
int qreadnw(char *msg_area, unsigned int *msg_size, short *qid,
short *mpriority, short *mclass, short *mtype, short wtime)
{
  short ilGrpid;
  long llType;
  long llClass;
  int  ilRc;
  
  llType  = (long)*mtype;
  llClass = (long)*mclass;

  if (wtime == 0)
    ilRc = bmqGet(&ilGrpid,qid,mpriority,&llType,&llClass,msg_area,msg_size);
  else
    ilRc = bmqGetw(&ilGrpid,qid,mpriority,&llType,&llClass,msg_area,msg_size,wtime);
  *mtype  = (short)llType;
  *mclass = (short)llClass;
  return(ilRc);
}

/**************************************************************
 ** 函数名      : qwrite9
 ** 功  能      : 写邮箱
***************************************************************/
int qwrite9(char *msg_area, unsigned int msg_size, short qdest)
{
  return(bmqPut(0,qdest,1,0,0,msg_area,msg_size));
}

/**************************************************************
 ** 函数名      : qread9
 ** 功  能      : 读邮箱
***************************************************************/
int qread9(char *msg_area, unsigned int *msg_size, short *qid)
{
  short ilGrpid;
  short ilPriority=0;
  long llType=0,llClass=0;
  
  return(bmqGetw(&ilGrpid,qid,&ilPriority,&llType,&llClass,msg_area,msg_size,0));
}

/**************************************************************
 ** 函数名      : qwrite3
 ** 功  能      : 跨阻写邮箱
***************************************************************/
int qwrite3(char *msg_area, unsigned int msg_size, short qid, short grpid,
short mpriority, short mclass, short mtype)
{
  long llType,llClass;
 
  llType = mtype;
  llClass = mclass;
  return(bmqPut(grpid,qid,mpriority,llType,llClass,msg_area,msg_size));
}

/**************************************************************
 ** 函数名      : qread3
 ** 功  能      : 跨组阻塞读邮箱
***************************************************************/
int qread3(char *msg_area, unsigned int *msg_size, short *qid, short *grpid,
short *mpriority, short *mclass, short *mtype)
{
  long llType,llClass;
  int ilRc;
  swDebug("in qread3 msg_size is:%d\n",*msg_size);
  swDebug(" in qread3 before strlen is:%d\n",strlen(msg_area));

  llType  = (long)*mtype;
  llClass = (long)*mclass;  
  ilRc = bmqGetw(grpid,qid,mpriority,&llType,&llClass,msg_area,msg_size,0);
  swDebug(" in qread3 after strlen is:%d\n",strlen(msg_area));
  *mtype  = (short)llType;
  *mclass = (short)llClass;
  return(ilRc);
}

/**************************************************************
 ** 函数名      : qread3nw
 ** 功  能      : 跨组非阻塞读邮箱
***************************************************************/
int qread3nw(char *msg_area, unsigned int *msg_size, short *qid,short *grpid,
short *mpriority, short *mclass, short *mtype, short wtime)
{
  long llType,llClass;
  int  ilRc;

  llType  = (long)*mtype;
  llClass = (long)*mclass;  

  if (wtime == 0)
    ilRc = bmqGet(grpid,qid,mpriority,&llType,&llClass,msg_area,msg_size);
  else
    ilRc = bmqGetw(grpid,qid,mpriority,&llType,&llClass,msg_area,msg_size,wtime);
  *mtype  = (short)llType;
  *mclass = (short)llClass;
  return(ilRc);
}
