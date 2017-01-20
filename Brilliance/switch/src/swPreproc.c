#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int _swTrim(char *s);
/**************************************************************
 ** ������      : main
 ** ��  ��      : �ļ�ת��
 ** ��  ��      :
 ** ��������    : 2001/09/28
 ** ����޸�����:
 ** ������������:
 ** ȫ�ֱ���    :
 ** ��������    : void
 ** ����ֵ      : void
***************************************************************/
int main(int argc,char **argv)
{
  char alOrgfile[80],alDesfile[80],alLogfile[80],alBuf[1024],alBuf1[1024],alBufin[1024];
  char alBuf2[1024];
  char alMacro[100];
  FILE *fporg,*fpdes,*fplog;

  if (argc < 4) 
  {
    printf("ʹ�÷��� : ������ Դ�ļ��� Ŀ���ļ��� [�����ļ�]\n");
    exit(0);
  }

  strcpy(alMacro,argv[1]);
  strcpy(alOrgfile,argv[2]);
  strcpy(alDesfile,argv[3]);
  if (argc >= 5) strcpy(alLogfile,argv[4]);

  fporg = fopen(alOrgfile,"r");
  if (fporg == NULL)
  {
    printf("���ļ�����: %d,%s\n",errno,strerror(errno));
    exit(0);
  }
  fpdes = fopen(alDesfile,"w+");
  if (fpdes == NULL)
  {
    printf("�����ļ�����: %d,%s\n",errno,strerror(errno));
    fclose(fpdes);
    exit(0);
  }
  if ( argc >= 5 )
  {
    fplog = fopen(alLogfile,"w+");
    if (fplog == NULL)
    {
      printf("�����ļ�����: %d,%s\n",errno,strerror(errno));
      fclose(fplog);
      exit(0);
    }
  }
  while(fgets(alBufin,256,fporg) != NULL)
  {
    strcpy(alBuf,alBufin);
    _swTrim(alBuf);
    if (strncmp(alBuf,"#ifdef",6) == 0)
    {
      alBuf2[0] = '<';
      strcpy(alBuf2+1,alBuf + 6);
      _swTrim(alBuf2+1);
      strcat(alBuf2,">");
      if (strstr(alMacro,alBuf2))
      {
        /* �ҵ��궨�忪ʼ�� */
        for (;;) {
          sprintf(alBuf1,"ɾ����: %s",alBufin);
          if(argc >= 5)
            fputs(alBuf1,fplog);
          if (strncmp(alBuf,"#endif",6) == 0)
          {
            /* �ҵ��궨������� */
            break;
          }
          if (!fgets(alBufin,256,fporg))  break;
          strcpy(alBuf,alBufin);
          _swTrim(alBuf);
        }
        continue;
      }
    }
    fputs(alBufin,fpdes);
  }

  fclose(fpdes);
  fclose(fporg);
  if(argc >= 5)
    fclose(fplog);
  return(0); 
}

int _swTrim(char *s)
{
	short	i, l, r, len;

	for(len=0; s[len]; len++);
	for(l=0; (s[l]==' ' || s[l]=='\t' || s[l]=='\n'); l++);
	if(l==len)
	{
		s[0]='\0';
		return(0);
	}
	for(r=len-1; (s[r]==' ' || s[r]=='\t' || s[r]=='\n'); r--);
	for(i=l; i<=r; i++) s[i-l]=s[i];
	s[r-l+1]='\0';
	return(0);
}

