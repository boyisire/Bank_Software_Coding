#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>

#define ROUNDS  32

typedef struct{
        unsigned        char    loki_subkeys[ROUNDS];
}loki_ctx;

extern int loki_trim();
extern int loki_key();
extern int loki_enc();

int main(int argc,char *argv[])
{
	int		i,j;
	FILE		*fp;
	char		alBuf[200];
	char		*alTmp,alResult[200],alVarname[30],alText[512];
	char		alPasswd[512],alDate[20],alUsers[10];
	char		alLicense[50],alOrderlist[200],alSerial[50];
	loki_ctx	lc;
	unsigned	char	*cp;
	unsigned	char	key[]={'b','s','p','V','4','.','3','0'};
 
	if(argc != 2)
	{ 
		printf("Usage :getlicense sourcefile\n");
		exit(0);
	}

	if ((fp = fopen(argv[1], "r")) == NULL)
	{
		printf("不能读取文件 %s!\n",argv[1]);
		exit(-1);
	}

	while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
	{
		loki_trim(alBuf);
		if (strlen(alBuf) == 0 || alBuf[0] == '#')
		{
			continue;
		}
    
		if ((alTmp = strchr(alBuf, '=')) == NULL) continue;
		strcpy(alResult,alTmp + 1);
		*alTmp = '\0';
		strcpy(alVarname,alBuf);

		loki_trim(alResult);
		loki_trim(alVarname);

		if (strcmp(alVarname,"LICENSEE") == 0)
			strcpy(alLicense,alResult);
		if (strcmp(alVarname,"SERIAL") == 0)
			strcpy(alSerial,alResult);
		if (strcmp(alVarname,"ORDERLIST") == 0)
			strcpy(alOrderlist,alResult);
		if (strcmp(alVarname,"USERS") == 0)
			strcpy(alUsers,alResult);
		if (strcmp(alVarname,"DATE") == 0)
			strcpy(alDate,alResult);
	}
	memset(alText,0x00,sizeof(alText));
	strcpy(alText,alLicense);
	strcat(alText,alSerial);
	strcat(alText,alOrderlist);
	strcat(alText,alUsers);
	strcat(alText,alDate);
	
	loki_key(&lc,key);
	cp = (unsigned char *)alText;
	loki_enc(&lc,cp,(strlen(alText)/2) + 1);
	j = 0;
	memset(alPasswd,0x00,sizeof(alPasswd));
	for(i=0;i<512;i++)
	{
		/*if(isgraph(alText1[i]))*/
		if( (alText[i]>32) && (alText[i]<127) )
		{
			alPasswd[j] = alText[i];
			j ++ ;
		}
	}

	printf("[Brilliance Q 2.0]\n");
	printf("LICENSEE=%s\n",alLicense);
	printf("SERIAL=%s\n",alSerial);
	printf("ORDERLIST=%s\n",alOrderlist);
	printf("USERS=%s\n",alUsers);
	printf("DATE=%s\n",alDate);
	printf("SIGNATURE=%s\n",alPasswd);

	return 0;
}

