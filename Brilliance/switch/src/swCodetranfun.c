
#include "switch.h"

/* 数据库定义 */
/*EXEC SQL INCLUDE sqlca;*/

#include "swCodetranfun.h"

/*==================================================
 *      ascii char -> ebcdic char
 *=================================================*/
uchar _swASCII2EBCDIC(uchar achar)
{
    unsigned int    c;

    c = (achar&0xff) >> 1 ;
    return( (uchar) ( (achar&0x01) ? (TABLE_ASCII2EBCDIC[c])&0xff :
                                     (TABLE_ASCII2EBCDIC[c])>>8 ) ) ;
}

/*==================================================
 *      ebcdic char -> ascii char
 *=================================================*/
uchar _swEBCDIC2ASCII(uchar echar)
{
    unsigned int    c;

    c = (echar&0xff) >> 1 ;
    return( (uchar) ( (echar&0x01) ? (TABLE_EBCDIC2ASCII[c])&0xff :
                                     (TABLE_EBCDIC2ASCII[c])>>8 ) ) ;
}


/**************************************************************
 ** 函数名: _swAtoE(uchar *d,uchar *s,short len)
 ** 功  能: ascii 转换到 ebcdic码
 ** 作  者: 
 ** 建立日期:
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: d:ebcdic     s:ascii   len:ebcdic[d]的长度。
 **          
 ** 返回值:
***************************************************************/
int _swAtoE(uchar *d,uchar *s,unsigned int len)
{
	int i;
	for(i=0;i<len;i++)
		d[i]=_swASCII2EBCDIC(s[i]);
	d[len]='\0';
	return(0);
}

/**************************************************************
 ** 函数名: EtoA(uchar *d,uchar *s,short len)
 ** 功  能:
 ** 作  者: 
 ** 建立日期:
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: d:ascii码      s:ebcdic码   len:ascii[d]的长度
 **          
 ** 返回值:
***************************************************************/
int _swEtoA(uchar *d,uchar *s,unsigned int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		d[i]=_swEBCDIC2ASCII(s[i]);
	}
	d[len]='\0';
	return(0);
}

/**************************************************************
 ** 函数名: _AtoE(uchar *d,uchar *s,short len)
 ** 功  能: ascii 转换到 ebcdic码
 ** 作  者: 
 ** 建立日期:
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: d:ebcdic     s:ascii   len:ebcdic[d]的长度。
 **          
 ** 返回值:
***************************************************************/
int _AtoE(uchar *d,uchar *s,unsigned int len)
{
	int i;
	for(i=0;i<len;i++)
		d[i]=_swASCII2EBCDIC(s[i]);
	d[len]='\0';
}

/**************************************************************
 ** 函数名: EtoA(uchar *d,uchar *s,short len)
 ** 功  能:
 ** 作  者: 
 ** 建立日期:
 ** 最后修改日期:
 ** 调用其它函数:
 ** 全局变量:
 ** 参数含义: d:ascii码      s:ebcdic码   len:ascii[d]的长度
 **          
 ** 返回值:
***************************************************************/
int _EtoA(uchar *d,uchar *s,unsigned int len)
{
	int i;
	for(i=0;i<len;i++)
	{
		d[i]=_swEBCDIC2ASCII(s[i]);
	}
	d[len]='\0';
}

int	cflag=0;

int etogb_41 (s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0x41)
	return (-2);
  else if (s[1]<0x59)
	{
	  t[0] = 0xa6;
	  t[1] = s[1] + 0x80;
	  return (0);
	  }
  else if (s[1]<0x61)
	return (-2);
  else if (s[1]<0x79)
	{
	  t[0] = 0xa6;
	  t[1] = s[1] + 0x40;
	  return (0);
	}
  else if (s[1]<0x80)
	return (-2);
  else if (s[1]<0xa1)
	{
	  t[0] = 0xa7;
	  t[1] = s[1] + 0x51;
	  return (0);
	}
  else if (s[1]<0xb1)
	return (-2);
  else if (s[1]<0xbb)
	{
	  t[0] = 0xa2;
	  t[1] = s[1] + 0x40;
	  return (0);
	}
  else if (s[1]<0xc0)
	return (-2);
  else if (s[1]<0xe1)
	{
	  t[0] = 0xa7;
	  t[1] = s[1] - 0x1f;
	  return (0);
	}
  else if (s[1]<0xf1)
	return (-2);
  else if (s[1]<0xfd)
	{
	  t[0] = 0xa2;
	  t[1] = s[1];
	  return (0);
	}
  else
	return (-2);
}

int etogb_42 (s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0x4a)
	return (-2);
  else if (s[1]==0x4a)
	{
	  t[0] = 0xa1;
	  t[1] = 0xea;
	  return (0);
	}
  else if ((s[1]==0x5f) || (s[1]==0x7d))
	  return (-40);
  else if (s[1]==0x79)
	{
	  t[0] = 0xa3;
	  t[1] = 0xe0;
	  return (0);
	}
  else if (s[1]<0x51)
	{
	  t[0] = 0xa3;
	  t[1] = TableEtoGB42[s[1]-0x40] + 0x60;
 	  if (t[1]<0xe1)
		t[1] ++;
	  if (t[1]==0x60)
		return (-2);
	  else
		return (0);
	}
  else if (s[1]<0x5a)
	return (-2);
  else if (s[1]<0x62)
	{
	  t[0] = 0xa3;
	  t[1] = TableEtoGB42[s[1]-0x40] + 0x60;
 	  if (t[1]<0xe1)
		t[1] ++;
	  if (t[1]==0x60)
		return (-2);
	  else
		return (0);
	}
  else if (s[1]<0x6a)
	return (-2);
  else if (s[1]==0x6a)
	return (-3);
  else if (s[1]<0x70)
	{
	  t[0] = 0xa3;
	  t[1] = TableEtoGB42[s[1]-0x40] + 0x60;
 	  if (t[1]<0xe1)
		t[1] ++;
	  if (t[1]==0x60)
		return (-2);
	  else
		return (0);
	}
  else if (s[1]<0x79)
	return (-2);
  else if (s[1]<0x80)
	{
	  t[0] = 0xa3;
	  t[1] = TableEtoGB42[s[1]-0x40] + 0x60;
 	  if (t[1]<0xe1)
		t[1] ++;
	  if (t[1]==0x60)
		return (-2);
	  else
		return (0);
	}
  else if (s[1]==0x80)
	return (-2);
  else if (s[1]<0x8a)
	{
	  t[0] = 0xa3;
	  t[1] = s[1] + 0x60;
	  return (0);
	}
  else if (s[1]<0x91)
	return (-2);
  else if (s[1]<0x9a)
	{
	  t[0] = 0xa3;
	  t[1] = s[1] + 0x59;
	  return (0);
	}
  else if (s[1]<0xa1)
	return (-2);
  else if (s[1]==0xa1)
	{
	  t[0] = 0xa3;
	  t[1] = 0xfe;
	  return (0);
	}
  else if (s[1]<0xaa)
	{
	  t[0] = 0xa3;
	  t[1] = s[1] + 0x51;
	  return (0);
	}
  else if (s[1]<0xc0)
	return (-2);
  else if (s[1]==0xc0)
	{
	  t[0] = 0xa3;
	  t[1] = 0xfb;
	  return (0);
	}
  else if (s[1]<0xca)
	{
	  t[0] = 0xa3;
	  t[1] = s[1];
	  return (0);
	}
  else if (s[1]<0xd0)
   	return (-2);
  else if (s[1]==0xd0)
	{
	  t[0] = 0xa3;
	  t[1] = 0xfd;
	  return (0);
	}
  else if (s[1]<0xda)
	{
	  t[0] = 0xa3;
	  t[1] = s[1] - 0x07;
	  return (0);
	}
  else if (s[1]==0xe0)
	{
	  t[0] = 0xa1;
	  t[1] = 0xe7;
	  return (0);
	}
  else if (s[1]<0xe2)
	return (-2);
  else if (s[1]<0xea)
	{
	  t[0] = 0xa3;
	  t[1] = s[1] - 0x0f;
	  return (0);
	}
  else if (s[1]<0xf0)
	return (-2);
  else if (s[1]<0xfa)
	{
	  t[0] = 0xa3;
	  t[1] = s[1] - 0x40;
	  return (0);
	}
  else 
	return (-2);
}

int etogb_43 (s, t)
unsigned char *s;
unsigned char *t;
{
  if ((s[1]<0x41) || (s[1]>0xe0))
	return (-2);
  if ((s[1]>0x5b) && (s[1]<0x81))
	return (-2);
  else if ((s[1]<=0x45) || (s[1]==0xa1))
	{
	  t[0] = 0xa1;
	  t[1] = TableEtoGB43[s[1]-0x40] + 0x61;
	  if (t[1]==0x61)
		return (-2);
	  return (0);
	} 
  else if (s[1]==0x4a)
	{
	  t[0] = 0xa1;
	  t[1] = TableEtoGB43[s[1]-0x40] + 0x60;
	  if (t[1]==0x60)
		return (-2);
	  return (0);
	}
  else if (s[1]==0xe0)
      {
	t[0] = 0xa3;
 	t[1] = 0xdc;
	return (0);
	}
  else if ((s[1]==0x58) || (s[1]==0x5b) 
		|| (s[1]==0xbe) || (s[1]==0xbf)
		|| (s[1]==0xdc) || (s[1]==0xdd))
	return (-40);	    /* !!! cannot found these char in gb TableEtoGB !!! */
  else
      {
	t[0] = 0xa5;
	t[1] = TableEtoGB43[s[1]-0x40] + 0x60;
	if (t[1]<0xe0)
  		t[1] ++;
  	if (t[1]==0x61)
		return (-4);
      }

  return (0);
}

int etogb_44 (s, t)
unsigned char *s;
unsigned char *t;
{
  if ((s[1]<0x42) || (s[1]>0xf3))
	return (-2);
  else if ((s[1]==0x5a) || (s[1]==0x5c) || (s[1]==0x5e)
		|| (s[1]==0x5f) || (s[1]==0x6c) || (s[1]==0x6d)
		|| (s[1]==0x6f) || (s[1]==0x7e) || (s[1]==0xdc)
		|| (s[1]==0xdd) || (s[1]==0xeb) || (s[1]==0xec))
	return (-40);
  else if (s[1]==0x70)
      {
	t[0] = 0xa3;
	t[1] = 0xde;
	return (0);
      }
  else if ((s[1]<0x44) || (s[1]>0xdf))
      {
	t[0] = 0xa1;
	t[1] = TableEtoGB44[s[1]-0x40] + 0x60;
 	if (t[1]<0xe0)
		t[1] ++;
  	if (t[1]==0x61)
	 	return (-2);
 	else
		return (0);
      }
  else if (s[1]<0x46)
      {
	t[0] = 0xa3;
	t[1] = TableEtoGB44[s[1]-0x40] + 0x60;
 	if (t[1]<0xe0)
		t[1] ++;
	if (t[1]==0x61)
		return (-2);
	else
		return (0);
      }
  else if (s[1]==0xdf)
	return (-2);
  else if ((s[1]<0x4a) || (s[1]>0x80))
      {
	t[0] = 0xa4;
	t[1] = TableEtoGB44[s[1]-0x40] + 0x02;
  	if (t[1]==0x02)
	 	return (-2);
 	else
		return (0);
      }
  else if (s[1]==0x80)
	return (-2);
  else if ((s[1]<0x4f) || (s[1]>0x5a))
      {
	t[0] = 0xa1;
	t[1] = TableEtoGB44[s[1]-0x40] + 0x60;
	if (t[1]<0xe0)
		t[1] ++;
	if (t[1]==0x61)
		return (-2);
	return (0);
      }
  else if (s[1]==0x4f)
	return (-2);
  else if (s[1]==0x50)
      {
	t[0] = 0xa3;
	t[1] = 0xa7;
	return (0);
      }
  else if (s[1]<0x58)
      {
	t[0] = 0xa4;
	t[1] = TableEtoGB44[s[1]-0x40] + 0x02;
	if (t[1]==0x02)
		return (-2);
	return (0);
      }
  else
	return (-2);
}

int etogb_45 (s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0x45) 
	return (-2);
  if (s[1]<0x47)
      {
	t[0] = 0xa1;
     	t[1] = s[1] + 0x60;
	return (0);
      }
  else if (s[1]<0x5b)
	return (-2);
  else if (s[1]<0x5d)
      {
	t[0] = 0xa1;
	t[1] = s[1] + 0x61;
	return (0);
      }
  else if ((s[1]<0x62) || (s[1]==0x78))
    	return (-2);
  else if (s[1]<0x7b)
      {
	t[0] = 0xa1;
	t[1] = s[1] + 0x61;
	return (0);
      }
  else if ((s[1]==0x88) || (s[1]==0x8b))
      {
	t[0] = 0xa1;
	t[1] = s[1] + 0x60;
	return (0);
      }
  else if (s[1]<0xb1)
	return (-2);
  else if (s[1]<0xd9)
      {
	t[0] = 0xa2;
	t[1] = s[1];
	return (0);
      }
  else if (s[1]<0xe1)
	return (-2);
  else if (s[1]<0xeb)
      {
	t[0] = 0xa2;
	t[1] = s[1] - 0x08;
	return (0);
      }
  else if (s[1]<0xf1)
	return (-2);
  else if (s[1]<0xfb)
      {
	t[0] = 0xa2;
	t[1] = s[1] - 0x0c;
	return (0);
      }
  else 
	return (-2);
}

int etogb_46(s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0x41)
	return (-2);
  else if (s[1]<0x61)
      {
	t[0] = 0xa8;
	t[1] = s[1] + 0x60;
	return (0);
      }
  else if (s[1]<0x65)
	return (-2);
  else if (s[1]<0x8a)
      {
	t[0] = 0xa8;
	t[1] = s[1] + 0x60;
	return (0);
      }
  else if (s[1]<0xa4)
	return (-2);
  else if (s[1]<0xf0)
      {
	t[0] = 0xa9;
	t[1] = s[1];
	return (0);
      }
  else 
	return (-2);
}

int _EtoGB(uchar *s, uchar *t)
{
  if ((s[0] == 0x40) && (s[1] == 0x40))		/* 第一、二码=40 */
    {
      if (cflag == 1)				/* 汉字状态 */
	{
	  t[0] = 0xa1;
	  t[1] = 0xa1;
	  return (0);
	}
      else					/* 非汉字状态 */
	{
	  t[0] = 0x20;
	  t[1] = 0x20;
	  return (0);
	}
    }

  if (s[0] == 0x41)					/* 第一个码=41 */
	  return (etogb_41 (s, t));

  if (s[0] == 0x42)					/* 第一个码=42 */
	  return (etogb_42 (s, t));

  if (s[0] == 0x43)					/* 第一个码=43 */
	  return (etogb_43 (s, t));

  if (s[0] == 0x44)					/* 第一个码=44 */
	  return (etogb_44 (s, t));

  if (s[0] == 0x45)					/* 第一个码=45 */
	  return (etogb_45 (s, t));

  if (s[0] == 0x46)					/* 第一个码=46 */
	  return (etogb_46 (s, t));

  if ((s[0]>0x48) && (s[0]<=0x6c))			/* 第一个码：49--6C */
  {
	if ((s[1]>=0x40) && (s[1]<0xa0))		/* 第二个码：40--9F */		
	{
	  t[0] = 0xb0 + (s[0] - 0x48) * 2 - 1;
	  if (s[1]<0x80)				/* 第二个码：40--7F */
		t[1] = s[1] + 0x60;
	  else if (s[1]>0x80)				/* 第二个码：81--9F */
		t[1] = s[1] + 0x5f;
	  else					/* 第二个码：80, 不为汉字 */
		return (-2);
	}
	else if ((s[1]>=0xa0) && (s[1]<0xfe))	/* 第二个码：A0--FD */
	{
	  t[0] = 0xb0 + (s[0] - 0x48) * 2;
	  t[1] = s[1] + 0x01;
	}
	else					/*  不为汉字 */
	  return (-2);
	return (0);
  }
  else if ((s[0]==0x48) && (s[1]>=0xa0))	/* 第一个码=48，第二个码>=A0 */
  {
	t[0] = 0xb0 + (s[0] - 0x48) * 2;
	t[1] = s[1] + 0x01;
	return (0); 
  }
  return (-1);			/* 第一个码<48的其他情况或>6C, 不为汉字 */
}


int gbtoe_a1 (ss, t)
unsigned char *ss;
unsigned char *t;
{
  unsigned char s[3];

  if (ss[1]<0xa1)			/*不为汉字，则返回-2 */
	return (-2);
  else if (ss[1]==0xa1)		/* 第二个码=A1 */
  {
	t[0] = t[1] = 0x40;
	return (0);
  }
  else if (ss[1]<0xe0)		/* 第二个码=A2--DF */
	s[1] = ss[1] - 0x61;	/* 41--7E */
  else						/* 第二个码>=E0, 即不为汉字 */
	s[1] = ss[1] - 0x60;	/* >=80 */

  /* 41, 42, 43, 4A, 57, 58, 89 */
  if ((s[1]==0x41) || (s[1]==0x42) || (s[1]==0x43) || (s[1]==0x4a)
  ||  (s[1]==0x57) || (s[1]==0x58) || (s[1]==0x89))
	t[0] = 0x43;
  else if ((s[1]==0x87) || (s[1]==0x8a))	/* 87, 8A */
	t[0] = 0x42;
  else if (s[1]==0x78)						/* 78 */
	t[0] = 0x44;
  else if ((s[1]==0x44) || (s[1]==0x45) || (s[1]==0x5b)
		|| (s[1]==0x5c) || (s[1]==0x88) || (s[1]==0x8b)
		|| ((s[1]>0x61) && (s[1]<0x7b))	
		|| ((s[1]>0xae) && (s[1]<0xed)))
	t[0] = 0x45;	/* 44,45,5B,5C,88,8B,62--7A, AF--EC */
  else if (((s[1]>0x45) && (s[1]<0x62)) || ((s[1]>0x7a) && (s[1]<0x9f)))
	t[0] = 0x44;	/* 46--61, 7B--9E */
  else if ((s[1]>0xee) && (s[1]<0xfb))
	t[0] = 0x41;	/* EF--FA */
  else
	return (-2);

  t[1] = TableGBtoE81[s[1]-0x40];
  if (t[1] == 0x00)
	return (-2);

  return (0);
}

int gbtoe_a2 (s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0xb1)
	return (-2);
  else if (s[1]<0xd9)
      {
	t[0] = 0x45;
	t[1] = s[1];
	return (0);
      }
  else if (s[1]<0xe2)
      {
	t[0] = 0x45;
	t[1] = s[1] + 0x08;
	return (0);
      }
  else if (s[1]<0xe5)
	return (-2);
  else if (s[1]<0xef)
      {
	t[0] = 0x45;
	t[1] = s[1] + 0x0c;
  	return (0);
      }
  else if (s[1]<0xf1)
	return (-2);
  else if (s[1]<0xfd)
      {
	t[0] = 0x41;
	t[1] = s[1];
	return (0);
      }
  else
	return (-2);
}

int gbtoe_a3 (ss, t)
unsigned char *ss;
unsigned char *t;
{
  unsigned char s[3];

  if ((ss[1]>0xa0) && (ss[1]<0xe0))
	s[1] = ss[1] - 0x61;
  else if ((ss[1]>=0xe0) && (ss[1]<0xff))
	s[1] = ss[1] - 0x60;
  else 
	return (-2);

  if (s[1]==0x7b)
    {
	t[0] = 0x43;
	t[1] = 0xe0;
	return (0);
    }
  else if ((s[1]==0x46) || ((s[1]>0x79) && (s[1]<0x7e)))
	t[0] = 0x44;
  else if ((s[1]>=0x40) && (s[1]<0x9f))
	t[0] = 0x42;
  else
	return (-2);

  t[1] = TableGBtoE82[s[1]-0x40];
  if (t[1]==0x00)
	return (-2);

  return (0);
}

int gbtoe_a4 (ss, t)
unsigned char *ss;
unsigned char *t;
{
  unsigned char s[3];

  if ((ss[1]<0xa1) || (ss[1]>0xf3))
	return (-2);
  else
	s[1] = ss[1] - 0x02;

  t[0] = 0x44;
  t[1] = TableGBtoE82[s[1]-0x40];
  if (t[1]==0x00)
	return (-2);

  return (0);
}

int gbtoe_a5 (ss, t)
unsigned char *ss;
unsigned char *t;
{
  unsigned char s[3];

  if (ss[1]<0xa1)
	return (-2);
  if (ss[1]<0xe0)
	s[1] = ss[1] - 0x61;
  else if (ss[1]<0xf7)
     	s[1] = ss[1] - 0x60;
  else
	return (-2);

  t[0] = 0x43;
  t[1] = TableGBtoE83 [s[1]-0x40];

  if (t[1]==0x00)
    	return (-2);

  return (0);
}

int gbtoe_a6 (s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0xa1)
	return (-4);
  else if (s[1]<0xb9)
      {
	t[0] = 0x41;
	t[1] = s[1] - 0x40;
	return (0);
      }
  else if (s[1]<0xc1)
	return (-2);
  else if (s[1]<0xd9)
      {
	t[0] = 0x41;
	t[1] = s[1] - 0x80;
	return (0);
      }
  else
	return (-3);
}

int gbtoe_a7 (s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0xa1)
	return (-2);
  else if (s[1]<0xc2)
      {
	t[0] = 0x41;
	t[1] = s[1] + 0x1f;
	return (0);
      }
  else if (s[1]<0xd1)
	return (-2);
  else if (s[1]<0xf2)
      {
	t[0] = 0x41;
	t[1] = s[1] - 0x51;
	return (0);
      }
  else
	return (-2);
}

int gbtoe_a8 (s, t)
unsigned char *s;
unsigned char *t;
{
  if (s[1]<0xa1)
	return (-2);
  else if (s[1]<0xc1)
      {
	t[0] = 0x46;
	t[1] = s[1] - 0x60;
	return (0);
      }
  else if (s[1]<0xc5)
	return (-2);
  else if (s[1]<0xe9)
      {
	t[0] = 0x46;
	t[1] = s[1] - 0x60;
	return (0);
      }
  else
	return (-2);
}

int gbtoe_a9 (s, t)
unsigned char *s;
unsigned char *t;
{
  if ((s[1]>0xa3) && (s[1]<0xf0))
  {
	t[0] = 0x46;
	t[1] = s[1];
	return (0);
  }
  else
	return (-2);
}

int _GBtoE(uchar *s, uchar *t)
{
  if ((s[1] == 0xa0) || (s[1]==0xff))
	return (-1);	/* A0或FF */

  if ((s[0]==0x81) && (s[1]==0x40))
     {
	t[0] = 0x40;
 	t[1] = 0x40;
	return (0);
     }
  
  if (s[0]==0xa1)					/* 如第一个码为A1 */
	return (gbtoe_a1 (s, t));
  
  if (s[0]==0xa2)					/* 如第一个码为A2 */
	return (gbtoe_a2 (s, t));
  
  if (s[0]==0xa3)					/* 如第一个码为A3 */
	return (gbtoe_a3 (s, t));
  
  if (s[0]==0xa4)					/* 如第一个码为A4 */
	return (gbtoe_a4 (s, t));
  
  if (s[0]==0xa5)					/* 如第一个码为A5 */
	return (gbtoe_a5 (s, t));

  if (s[0]==0xa6)					/* 如第一个码为A6 */
	return (gbtoe_a6 (s, t));

  if (s[0]==0xa7)					/* 如第一个码为A7 */
	return (gbtoe_a7 (s, t));

  if (s[0]==0xa8)					/* 如第一个码为A8 */
	return (gbtoe_a8 (s, t));

  if (s[0]==0xa9)					/* 如第一个码为A9 */
	return (gbtoe_a9 (s, t));

  if ((s[0]>=0xb0) && (s[0]<=0xf7))			/* 如第一个码为B0--F7 */
  {								/* 16-87区 */
	if (s[1]<0xa0)				/* 第二个码<A0，不为汉字 */
		return (-2);	
	t[0] = (s[0] - 0xb0)/0x02 + (s[0] - 0xb0)%0x02+ 0x48;
	if (s[0]%0x02)						/* 奇数区 */
	{
	  if ((s[1]>0xa0) && (s[1]<=0xdf))	/* A1--DF=01-95 */
		t[1] = s[1] -0x60;				/* ->41--7F */
	  else
		t[1] = s[1] -0x5f;
	}
	else							/* 偶数区 */
	  t[1] = s[1] -0x01;
	return (0);
  }
  
  return (-1);							/* 不为汉字 */
}

/************************************************/
/*   程序名：SetDefault()			*/
/*   功  能：将字段设置成缺省值         	*/
/*   参  数：1. fld_buf: 被设置的字段指针       */
/*	     2. fld_len: 字段长度		*/
/*   返  回：无                                 */
/************************************************/
int SetDefault(fld_buf, fld_len, fld_type)
uchar *fld_buf;
unsigned int fld_len;
char fld_type;
{
  int i,j;
  char tmpbuf[iFLDVALUELEN];
  
  memset(tmpbuf,'\0',sizeof(tmpbuf));

  switch(fld_type)
  {
    case 'A':
    case 'O':
      for(i=0;i<fld_len;i++) fld_buf[i]=' ';
      break;
    case 'P':
	if ( strlen( (char *)fld_buf ) == 0)
	   {
	   memset( fld_buf, '0', fld_len);
	   break;
	   }
	for ( i=0;i< fld_len;i++)
	    {
		if ( fld_buf[i] == ' ')
		    fld_buf[i] = '0';
	    }
	break;
    case 'S': 
        if ( strlen( (char *)fld_buf ) == 0 )
	   {
      	    for(i=0;i<fld_len;i++) fld_buf[i]='0';
	    break;
	   }
        for ( i=0; i< fld_len; i++)
	    {
	     if ( fld_buf[i] == ' ' || fld_buf[i] == 0 )
		  fld_buf[i] = '0';
            }
	break;
    case 'M':
	for ( i=0;i< fld_len-strlen((char *)fld_buf);i++)
	    tmpbuf[i] = '0';
	memcpy( tmpbuf+i, fld_buf, strlen((char *)fld_buf));
	memcpy( fld_buf, tmpbuf, fld_len);
	break;
  }

  return(0);
}


/************************************************/
/*   程序名：_NAtoEP()			        */
/*   功  能：将P型字段进行压缩                  */
/*   参  数：1. src_buf: P型ASCII字段           */
/*   返  回: 2. des_buf: P型AS400字段           */
/*   编  写：ADD negative process,2000/05/19    */
/************************************************/
int _NAtoEP(uchar *src_buf,unsigned int *destlen,uchar *des_buf)
{
  int	i,j,k;
  uchar	tmp_buf[iFLDVALUELEN];
  unsigned int fld_len;

  memset(des_buf, 0x00, iFLDVALUELEN);
  fld_len = *destlen;

  if ( src_buf[0] == '-' || src_buf[0] == '+' )
     {
      fld_len = fld_len - 1;
      if (src_buf[0] == '-')
         tmp_buf[fld_len+1] = 0x0d;
      else 
	 tmp_buf[fld_len+1] = 0x0f;
       for(i=0;i<fld_len;i++)
         tmp_buf[i+1]=src_buf[i+1]-0x30;
     }
  else
     {
      tmp_buf[fld_len+1]=0x0f;
       for(i=0;i<fld_len;i++)
         tmp_buf[i+1]=src_buf[i]-0x30;
     } 

  k=0;
  if (((fld_len/2)*2)==fld_len)
    tmp_buf[0]=0;
  else
    k=1;
  
  for(i=k,j=0;i<=fld_len;i+=2,j++)
  {
    tmp_buf[i]<<=4;
    tmp_buf[i]&=0xf0;
    des_buf[j]=tmp_buf[i] | tmp_buf[i+1];
  }
  *destlen = fld_len/2+1;

  return(SUCCESS);
}

/************************************************/
/*   程序名：_NEtoAP()		         	*/
/*   功  能：将P型字段解压			*/
/*   参  数：1. src_buf: 来自AS400的P型字段     */
/*   返  回：2. des_buf: 转换后的ASCII字段      */
/*   编  写：                                   */
/************************************************/
void _NEtoAP(uchar *src_buf,unsigned int *destlen,uchar *des_buf)
{
  int   i,j;
  uchar ch;
  uchar tmp_buf[iFLDVALUELEN];
  unsigned int fld_len;

  swDebug("_NEtoAP src_buflen=%d,buf is",*destlen);
  swDebughex((char *)src_buf,*destlen);

  /*memset(des_buf, '\0', iFLDVALUELEN);*/
  fld_len = *destlen;

  if((fld_len/2*2) == fld_len)
  {
    for(i=0,j=1;i<fld_len;i++)
    {
      ch=src_buf[i]&0xf0;
      ch>>=4;
      des_buf[j++]=ch+0x30;
      ch=src_buf[i]&0x0f;
      des_buf[j++]=ch+0x30;
    }
    if ( des_buf[j-1] == 0x3d )
      {
        des_buf[0] = '-';
	des_buf[j-1] = '\0';
	j = j-1;
      }
    else
      {
       des_buf[j-1] = '\0';
       memcpy(tmp_buf,des_buf+1,j);
       memcpy(des_buf,tmp_buf,j);
       j = j-2;
      }
  }
  else
  {
    for(i=0,j=0;i< fld_len;i++)
    {
      ch=src_buf[i]&0xf0;
      ch>>=4;
      des_buf[j++]=ch+0x30;
      ch=src_buf[i]&0x0f;
      des_buf[j++]=ch+0x30;
    }
    if ( des_buf[j-1] == 0x3d )
    {
      j = j - 1;
      des_buf[0] = '-';
      des_buf[j] = '\0';
    }
    else
    {
      j = j - 2;
      memcpy(tmp_buf,des_buf+1,j);
      memcpy(des_buf,tmp_buf,j);
      des_buf[j]='\0';
    }
  }

  *destlen=j;

  swDebug("_NEtoAP des_buf is : ");
  swDebughex((char *)des_buf,*destlen);

  return;
}
