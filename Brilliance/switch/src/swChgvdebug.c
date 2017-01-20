#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**************************************************************
 ** 函数名      : main
 ** 功  能      : 主函数
 ** 作  者      : 史正烨
 ** 建立日期    : 1999/11/25
 ** 最后修改日期: 2000/3/24
 ** 调用其它函数: 
 ** 全局变量    :
 ** 参数含义    : void
 ** 返回值      : void
***************************************************************/
int main(int argc,char **argv)
{
  char alOrgfile[80],alDesfile[80],alLogfile[80],alBuf[1024],alBuf1[1024];
  char alBuf2[1024],alBuf3[1024];
  char *alStr1,*alStr2,*alStr3;
  FILE *fporg,*fpdes,*fplog;
  int i,j;

  i = 10;
  j = 10;

  if (argc < 4) 
  {
    printf("使用方法 : %s 源文件名 目标文件名 日志文件名 [起始跟踪号] [间隔数]\n",argv[0]);
    exit(0);
  }

  strcpy(alOrgfile,argv[1]);
  strcpy(alDesfile,argv[2]);
  strcpy(alLogfile,argv[3]);

  if (argc >= 5) i = atoi(argv[4]);
  if (argc >= 6) j = atoi(argv[5]);

  printf("  源文件 : %s\n目标文件 : %s\n日志文件 : %s\n起始号码 : %d\n  间隔数 ：%d\n",alOrgfile,alDesfile,alLogfile,i,j);
  fporg = fopen(alOrgfile,"r");
  fpdes = fopen(alDesfile,"w+");
  fplog = fopen(alLogfile,"w+");

  while(fgets(alBuf,256,fporg) != NULL)
  {
    /* printf("%s",alBuf); */
    alStr1 = strstr(alBuf,"swVdebug(");
    if (alStr1)
    {
      memset(alBuf1,0x00,sizeof(alBuf1));
      memset(alBuf2,0x00,sizeof(alBuf2));
      memset(alBuf3,0x00,sizeof(alBuf3));
      strcpy(alBuf2,alBuf);
      alStr2 = strchr(alStr1,',');
      if (!alStr2)
      {
        fputs(alBuf,fpdes);
        continue;
      }
      alStr3 = strchr(alStr2,':');
      if (!alStr3)
      {
        fputs(alBuf,fpdes);
        continue;
      }
      memcpy(alBuf3,alStr1+9,alStr2-alStr1-9);
      fputs(alBuf3,fplog);
      sprintf(alBuf1,"swVdebug(%d,\"S%04d%s",atoi(alBuf3),i,alStr3);
      memset(alBuf3,0x00,sizeof(alBuf3));
      strcpy(alStr1,alBuf1);
      sprintf(alBuf3,">>%s",alBuf);
      fputs(alBuf2,fplog);
      fputs(">>>",fplog);
      fputs(alBuf,fplog);
      i = i + j;
    }
    fputs(alBuf,fpdes);
  }
  fclose(fpdes);
  fclose(fporg);
  fclose(fplog);
  printf("结束号码 : %d\n",i - j);
  exit(0);
}

