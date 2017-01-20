/****************************************************************/
/* 模块编号    ：License                                        */
/* 模块名称    ：平台License模块                                */
/* 版 本 号    ：V4.3.0                                         */
/* 作    者    ：徐军                                           */
/* 建立日期    ：2001/11/15                                     */
/* 最后修改日期：2001//                                         */
/* 模块用途    ：                                               */
/* 本模块中包含如下函数及功能说明：                             */
/****************************************************************/

/****************************************************************/
/* 修改记录:                                                    */
/* 2001。11。30 封版	共	369行				*/
/****************************************************************/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<ctype.h>

#define	LOKIBLK	8
#define	ROUNDS	32

#define	ROL12(b)	b = ((b << 2) | (b >> 6));
#define	ROL13(b)	b = ((b << 3) | (b >> 5));

typedef	unsigned	char	Long;

extern FILE *_swFopen(const char *file, const char *mode);
extern	Long	lokikey[2];
extern	char	*loki_lib_ver;

typedef	struct{
	Long	loki_subkeys[ROUNDS];
}loki_ctx;

static	Long	f();
static	short	s();

char	PP[32] = {
	31,23,15,7,30,22,14,6,
	29,21,13,5,28,20,12,4,
	27,19,11,3,26,18,10,2,
	25,17,9,1,24,16,8,0
};

typedef struct{
	short	gen;
	short	exp;
}sfn_desc;

sfn_desc	sfn1[] = {
	{375,31},{379,31},
	{391,31},{395,31},
	{397,31},{415,31},
	{419,31},{425,31},
	{433,31},{445,31},
	{451,31},{463,31},
	{471,31},{477,31},
	{487,31},{499,31},
	{00,00} };

void perm32(Long *,Long *,char *);
void setlokikey(c,key)
loki_ctx	*c;
char		*key;
{
	register	int	i;
	register Long	KL,KR;
	KL = ((char *)key)[0];
	KR = ((char *)key)[1];

	for (i=0;i<ROUNDS;i+=4) {
		c->loki_subkeys[i] = KL;
		ROL12(KL);
		c->loki_subkeys[i+1] = KL;
		ROL13(KL);
		c->loki_subkeys[i+2] = KR;
		ROL12(KR);
		c->loki_subkeys[i+3] = KR;
		ROL13(KR);
	}
}

void enloki(c,b)
loki_ctx *c;
char *b;
{
	register int	i;
	register Long 	L,R;
	L = ((Long *)b)[0];
	R = ((Long *)b)[1];

	for(i=0;i<ROUNDS;i+=2){
		L ^= f(R,c->loki_subkeys[i]);
		R ^= f(L,c->loki_subkeys[i+1]);
	}

	((Long *)b)[0] = R;
	((Long *)b)[1] = L;
}

void
deloki(c,b)
loki_ctx *c;
char *b;
{
	register int	i;
	register Long	L,R;
	L = ((Long *)b)[0];
        R = ((Long *)b)[1];

        for(i=ROUNDS;i>0;i-=2){
                L ^= f(R,c->loki_subkeys[i-1]);
                R ^= f(L,c->loki_subkeys[i-2]);
        }

        ((Long *)b)[0] = R;
        ((Long *)b)[1] = L;
}

#define MASK12	0x0f

static Long
f(r,k)
register Long	r;
Long		k;
{
	Long	a,b,c;

	a = r^k;

	b = ((Long)s((a		& MASK12))	) |
	    ((Long)s(((a >> 2)	& MASK12)) << 2	) |
	    ((Long)s(((a >> 4)	& MASK12)) << 4) |
	    ((Long)s((((a >> 3)| (a << 1 )) & MASK12)) << 3);
	perm32(&c,&b,PP);
	return(c);
}

static short s(i)
register Long	i;
{
	register short r,c,v,t;
	short	exp8();

	r = ((i >> 2) & 0xc) | (i & 0x3);
	c = (i >> 2) & 0xff;
	t = (c + ((r*17)^ 0xff)) & 0xff;
	v = exp8(t,sfn1[r].exp,sfn1[r].gen);

	return(v);
}

#define		MSB	0x80

void perm32(out,in,perm)
Long *out;
Long *in;
char perm[32];
{
	Long	mask = MSB;
	register int	i,o,b;
	register char	*p = perm;

	*out = 0 ;
	for(o=0;o<8;o++){
		i = (int)*p++;
		b = (*in >> i) & 01;
		if(b)
			*out |= mask;
		mask >>= 1;
	}
}

#define	SIZE	256

short mult8(a,b,gen)
short	a,b;
short	gen;
{
	short	product  = 0;
	
	while(b != 0){
		if(b&01)
			product ^= a;
		a <<= 1;
		if(a >= SIZE)
			a ^= gen;
		b >>= 1;
	}
	return(product);
}	

short exp8(base,exponent,gen)
short	base;
short	exponent;
short	gen;
{
	short	accum = base;
	short	result = 1;

	if(base == 0) return(0);

	while(exponent != 0){
		if((exponent & 0x0001) == 0x0001)
			result = mult8(result,accum,gen);
		exponent >>= 1;
		accum = mult8(accum,accum,gen);
	}
	return(result);
}

void loki_key(loki_ctx *c,unsigned char *key){
	setlokikey(c,key);
}

void loki_enc(loki_ctx *c,unsigned char *data,int blocks){
	unsigned char *cp;
	int	i;

	cp = data;
	for(i=0;i<blocks;i++){
		enloki(c,cp);
		cp += 2;
	}
}

void loki_dec(loki_ctx *c,unsigned char *data,int blocks){
	unsigned char *cp;
	int	i;

	cp = data;
	for(i=0;i<blocks;i++){
		deloki(c,cp);
		cp += 2;
	}
}

int loki_trim(s)
char *s;
{
	short   i, l, r, len;

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

int _swLicense(char *aOrdername)
{
	int	i,j; 
	FILE	*fp;
	long	llCurrentTime,llAvailTime;
	struct	tm	slAvailTime;
	char	alBuf[200],alSignature[200],alDate[20];
	char	*alTmp,alResult[200],alVarname[30],alText[512],alPasswd[512];
	char	alLicense[50],alOrderlist[200],alUsers[20],alSerial[50];
	char	year[5],mon[3],day[3];
	loki_ctx	lc;
	unsigned	char	*cp;
	unsigned	char	key[]={'b','s','p','V',4,'.','3','0'};
 
        memset(alPasswd,0x00,sizeof(alPasswd));
        memset(alSignature,0x00,sizeof(alSignature));

	if ((fp = _swFopen("license.txt","r")) == NULL) 
	{
          printf("没有找到文件license.txt!\n");
	  return(-1);
	}

	while (fgets(alBuf, sizeof(alBuf), fp) != NULL)
	{
		loki_trim(alBuf);
		if (strlen(alBuf) == 0 || alBuf[0] == '#')
		{
			continue;
		}
    
		if ((alTmp = (char *)strchr(alBuf,'=')) == NULL) continue;
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
		if (strcmp(alVarname,"SIGNATURE") == 0)
			strcpy(alSignature,alResult);
	}

	memset(alText,0x00,sizeof(alText));
	strcpy(alText,alLicense);
	strcat(alText,alSerial);
	strcat(alText,alOrderlist);
	strcat(alText,alUsers);
	strcat(alText,alDate);


	loki_key(&lc,key);
	cp = (unsigned char *)alText;
	loki_enc(&lc,cp,(strlen(alText)/2 + 1));
	j = 0;
	for(i=0;i<512;i++)
	{
		/*if(isgraph(alText[i]))*/
		if( (alText[i]>32) && (alText[i]<127) )
		{
			alPasswd[j] = alText[i];
			j ++ ;
		}
	}

	if(strcmp(alPasswd,alSignature)) 
	{
		printf("您的LICENSE不正确! 请与我公司联系.\n");
		return(-1);
	}
 
	time(&llCurrentTime);
        memset(year,0x00,sizeof(year));
        memset(mon,0x00,sizeof(mon));
        memset(day,0x00,sizeof(day));
	strncpy(year,alDate,4);
	strncpy(mon,alDate+4,2);
	strncpy(day,alDate+6,2);

	slAvailTime.tm_mday	=	atoi(day);
	slAvailTime.tm_mon	=	atoi(mon) - 1;
	slAvailTime.tm_year	=	atoi(year) - 1900;
	slAvailTime.tm_hour	=	23;
	slAvailTime.tm_min	=	59;
	slAvailTime.tm_sec	=	59;
	llAvailTime = mktime(&slAvailTime);

	if(llCurrentTime > llAvailTime)
	{
		printf("您的LICENSE已经过期! 请与我公司联系.\n");
		return(-1);
	}

	strcpy(alResult,aOrdername);
	loki_trim(alResult);
/*
	alTmp = (char *)strchr(alResult,' ');
	*alTmp = '\0';
*/
	if( (strstr(alOrderlist,alResult)) == NULL )
	{
		printf("您并未购买[%s]软件! 请与我公司联系.\n",aOrdername);
		return(-1);
	}

	if( (llAvailTime - llCurrentTime)/(24*60*60) <= 29 )
		printf("注意: 您的软件License有效使用期还有%d天!!!\n",
			(int)(llAvailTime - llCurrentTime)/(24*60*60)+1);

	return(0);
}
