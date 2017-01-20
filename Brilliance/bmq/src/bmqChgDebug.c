
#include <stdio.h>
#include <string.h>

main(int argc,char **argv)
{
  char alOrgfile[80],alDesfile[80],alLogfile[80],alBuf[1024],alBuf1[1024];
  char alBuf2[1024],alBuf3[1024];
  char *alStr1,*alStr2;
  FILE *fporg,*fpdes,*fplog;
  int i;

  i = 10;

  if (argc < 4) 
  {
    printf("使用方法 : bmqChgDebug 源文件名 目标文件名 日志文件名 [起始跟踪号]\n");
    exit(0);
  }

  strcpy(alOrgfile,argv[1]);
  strcpy(alDesfile,argv[2]);
  strcpy(alLogfile,argv[3]);

  if (argc >= 5) i = atoi(argv[4]);

  printf("  源文件 : %s\n目标文件 : %s\n日志文件 : %s\n起始号码 : %d\n",
    alOrgfile,alDesfile,alLogfile,i);
  fporg = fopen(alOrgfile,"r");
  fpdes = fopen(alDesfile,"w+");
  fplog = fopen(alLogfile,"w+");

  while(fgets(alBuf,256,fporg) != NULL)
  {
    /* printf("%s",alBuf); */
    alStr1 = strstr(alBuf,"_bmqDebug(\"S");
    if (alStr1)
    {
      memset(alBuf1,0x00,sizeof(alBuf1));
      memset(alBuf2,0x00,sizeof(alBuf2));
      memset(alBuf3,0x00,sizeof(alBuf3));
      strcpy(alBuf2,alBuf);
      alStr2 = strchr(alStr1,' ');
      if (alStr2)
      {
        sprintf(alBuf1,"_bmqDebug(\"S%04d: %s",i,alStr2+1);
        strcpy(alStr1,alBuf1);
        sprintf(alBuf3,">>%s",alBuf);
        fputs(alBuf2,fplog);
        fputs(">>>",fplog);
        fputs(alBuf,fplog);
        i = i + 10;
      }
    }
    fputs(alBuf,fpdes);
  }
  fclose(fpdes);
  fclose(fporg);
  fclose(fplog);
  printf("结束号码 : %d\n",i - 10);
}

