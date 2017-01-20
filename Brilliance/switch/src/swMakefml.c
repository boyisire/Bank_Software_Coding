#include "swapi.h"

void appQuit(int iFlag);

int main(int argc,char **argv)
{
  FILE *fp;
  unsigned int ilMsglen;
  char alMsgbody[iMSGMAXLEN];
  char alOrgfile[100],alDesfile[100],alFmlname[iFLDNAMELEN+1],alFmllen[10],alFmlvalue[iFLDVALUELEN];
  short ilRc;

  strcpy(agDebugfile,"swMakefml.debug");
  if(argc !=3) {
    fprintf(stderr, "ʹ�÷���: swMakefml FML�����ļ� FML�����ļ�\n");
    exit(-1);
  }

  strcpy(alOrgfile,argv[1]);
  strcpy(alDesfile,argv[2]);

  if ((fp = fopen(alOrgfile,"r")) == NULL)
  {
    swDebug("FML�����ļ�[%s]�򿪳���!",alOrgfile);
    exit(-1);
  }

  ilMsglen = 0;
  while ((ilRc = fscanf(fp,"%s %s %s",alFmlname,alFmllen,alFmlvalue)) > 0)
  {
    ilRc = swFmlpackset(alMsgbody,&ilMsglen,
      alFmlname,atoi(alFmllen),alFmlvalue);
    if (ilRc)
    {
      swDebug("swFmlpack����: %d",ilRc);
      swDebug("%s,%s,%s",alFmlname,alFmllen,alFmlvalue);
      fclose(fp);
      exit(-1);
    }
  }

  fclose(fp);

  unlink(alDesfile);

  if ((fp = fopen(alDesfile,"a+")) == NULL)
  {
    swDebug("FML�����ļ�[%s]�򿪳���!",alDesfile);
    exit(-1);
  }

  ilRc = fwrite(alMsgbody,1,ilMsglen,fp);
  fclose(fp);

  exit(0);
}

