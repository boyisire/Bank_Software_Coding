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
 ** ������      : qattach
 ** ��  ��      : �����ʼ��
***************************************************************/
int qattach(short qid)
{
  return(bmqOpen(qid));
}

/**************************************************************
 ** ������      : qwrite
 ** ��  ��      : д����
***************************************************************/
int qwrite(char *msg_area, unsigned int msg_size, short qdest)
{
  return(bmqPut(0,qdest,0,0,0,msg_area,msg_size));
}

/**************************************************************
 ** ������      : qwrite
 ** ��  ��      : ������
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
 ** ������      : qdetach
 ** ��  ��      : ������ֹ��
***************************************************************/
int qdetach( )
{
  return(bmqClose());
}

/**************************************************************
 ** ������      : _qwrite2
 ** ��  ��      : д����
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
 ** ������      : qwrite2
 ** ��  ��      : д����
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
 ** ������      : qread2
 ** ��  ��      : ����������
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
 ** ������      : qreadnw
 ** ��  ��      : ������������
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
 ** ������      : qwrite9
 ** ��  ��      : д����
***************************************************************/
int qwrite9(char *msg_area, unsigned int msg_size, short qdest)
{
  return(bmqPut(0,qdest,1,0,0,msg_area,msg_size));
}

/**************************************************************
 ** ������      : qread9
 ** ��  ��      : ������
***************************************************************/
int qread9(char *msg_area, unsigned int *msg_size, short *qid)
{
  short ilGrpid;
  short ilPriority=0;
  long llType=0,llClass=0;
  
  return(bmqGetw(&ilGrpid,qid,&ilPriority,&llType,&llClass,msg_area,msg_size,0));
}

/**************************************************************
 ** ������      : qwrite3
 ** ��  ��      : ����д����
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
 ** ������      : qread3
 ** ��  ��      : ��������������
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
 ** ������      : qread3nw
 ** ��  ��      : ���������������
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
