#ifndef _swdbstruct_h
#define _swdbstruct_h

/* D001�ڲ���׼���б� */
struct swt_sys_imf 
{
  char	imf_name[iFLDNAMELEN];
  char	imf_type[2];
  char  a1[6];  
  short	imf_len;
  short	imf_dec;
  char	imf_check[iEXPRESSLEN];
  short	imf_id;
  char  imf_china[41];
  char  a2;
}sgSwt_sys_imf;

struct portattrib
{
  char  cPort_status;
  char  cPort_downcase;
  char  a1[6];
  long  lTrning;
  long  lTrnend;
  long  lTrnovertime;
  long  lTrnreving;
  long  lTrnrevend;
  long  lTrnrevfail;
  long  lTrnrevwait;
} ;

/* D002��������� */
struct swt_sys_queue 
{
  short	q_id;
  short	q_port_id;
  char	q_name[21];
  char  a1;
  char	q_statu[2];
  char	down_cause[2];
  short	msg_format;
  short	rev_overtime;
  short	rev_num;
  char	tc_unpack[iEXPRESSLEN];
  char	tc_pack[iEXPRESSLEN];
  char	tran_type[129];
  char  a2[7];
  char	def_rs[7];
  char	def_gs[7];
  char	def_fs[7];
  char  a3;
  char	rev_nogood[2];
  char	rev_nomrev[2];
  short	isotab_id;
  char	bitmap_type[2];
  char	msg_prio[2];
  char	key_express[iEXPRESSLEN];
  char	comm_type[2];
  char	comm_attr[iEXPRESSLEN];
  char	resu_express[iEXPRESSLEN];
  char  code_type[2];   
  char  resu_revexp[iEXPRESSLEN];
  short max_tran;      
  short reje_grp;     
  char  a4[2];
  struct portattrib sPortattrib;  
  long  trigger_freq;          /* ����Ƶ�� */
  char  trigger_term[iEXPRESSLEN];     /* ����ʱ�� */
  long  trigger_time;          /* ����ʱ�� */
  char  setdown_overtime[iEXPRESSLEN]; /* ���ó�ʱ�Ƿ���down */
  unsigned char  port_status;   /* �˿�״̬ �����λ����Ϊ */
                       /* ͨѶ�㡢Ӧ�ò㡢Q�㡢Echotest�㡢�˹���Ԥ */
  char  a5[7];
  long  tranbegin_num; /* �˿ڷ������� */
  long  tranbegin_max; /* �˿�������������� */
  long  traning_num;   /* �˿ڴ������� */
  long  traning_max;   /* �˿�������������� */
  long  transhm_max;   /* �˿�ת�ƴ����������� add by nh 20020910 */
  char rev_express[iEXPRESSLEN];
  short e8583tab_id;			/* ISO8583E group id */
  short bitmap_len;			/* ����bitmap���� */
  char  a6[4];
  char  bitmap_unpk[iEXPRESSLEN];	/* bitmap������� */
  char  bitmap_pack[iEXPRESSLEN];	/* bitmap������� */
  char  mac_express[iEXPRESSLEN];	/* mac express */
/* added by fzj at 2003.06.05 */
/* ����Դ������������CHECK,�����Ƿ���� */
  char chk_express[iEXPRESSLEN];
  /* add by gengling at 2015.04.13 begin PSBC_V1.0 */
  short max_fail_count;            /* ����ߴ��� */
  short fail_connect_try_interval; /* �������Լ��ʱ��(ȱʡ30��) */
  short fail_count;                /* ���ߴ��� */
  long fail_begin_time;            /* ���߿�ʼʱ�� */
  /* add by gengling at 2015.04.13 end PSBC_V1.0 */
}sgSwt_sys_queue;

/* === begin of added by fzj at 2002.02.28 === */
/* ���׶˿ڹ����� */
struct swt_sys_matchport
{
  short qid;			/* Դ���˿� */
  char  trancode[11];	/* ���״��� */
  char  a1;
  short port;			/* �����˿� */
}sgSwt_sys_matchport;
/* === end of added by fzj at 2002.02.28 === */

/* D003��ʽת������ */
struct swt_sys_fmt_m 
{
  short	q_id;
  char	tran_code[11];
  char  a1;
  short	tran_step;
  char	tran_type[2];
  char	mac_create[iEXPRESSLEN];
  char	mac_check[iEXPRESSLEN];
  /*delete by zcd 20141220
  short	fmt_group;
  ***end of delete by zcd 20141220*/
  /*add by zcd 20141220*/
  long	fmt_group;
  /*end of add by zcd 20141220*/
  short fmt_prior;
  char  a2[2];
}sgSwt_sys_fmt_m;

/* D004��ʽת����ϸ�� */
struct swt_sys_fmt_d 
{
  /*delete by zcd 20141220
  short	fmt_group;
  ***end of delete by zcd 20141220*/
  /*add by zcd 20141220*/
  long	fmt_group;
  /*end of add by zcd 20141220*/
  short	id;
  char	imf_name[iFLDNAMELEN];
  char  imf_array[iFLDNAMELEN]; 
  long	fld_id;  /*modified by baiqj20150413 PSBC_V1.0*/
  char	fld_express[iEXPRESSLEN];
  char  fmt_flag[2];
  char  fmt_cond[iEXPRESSLEN];
}sgSwt_sys_fmt_d;

/* D006Ӧ���������� */
struct swt_sys_task 
{
  char	task_name[21];
  char  a1;
  char  task_desc[41];
  char  a2;
  char	task_file[iEXPRESSLEN];
  short	q_id;
  short	start_id;
  short	start_wait;
  short	stop_id;
  short	stop_wait;
  char  a3[6];
  long	pid;
  char	task_status[2];
  char  restart_flag[2];
  short kill_id;
  char  task_attr[2];
  long  start_time;
  short restart_num;
  short restart_max;
  short task_priority;
  short task_flag;
  char	task_use[2];
  char  task_timer[51];
  char  a4[3];
}sgSwt_sys_task;

/* D007 ·������ */
struct swt_sys_route_d 
{
  short	route_grp;
  short	route_id;
  char	route_cond[iEXPRESSLEN];
  char	q_target[iEXPRESSLEN];
  short	next_id;
  char	oper_flag[2];
  char	saf_flag[2];
  char	end_flag[2];
  /*delete by zcd 20141220
  short	fmt_group;
  ***end of delete by zcd 20141220*/
  /*add by zcd 20141220*/
  long	fmt_group;
  /*end of add by zcd 20141220*/
  short	rev_mode;
 /*delete by zcd 20141220
  short	rev_fmtgrp;
  ***end of delete by zcd 20141220*/
  /*add by zcd 20141220*/
  long	rev_fmtgrp;
  /*end of add by zcd 20141220*/
  char	fml_name[iFLDNAMELEN];
  char  route_memo[iEXPRESSLEN];
  char  a1[6];
}sgSwt_sys_route_d;

struct swt_sys_route_grp
{
  short route_grp;
  char  route_desc[41];
  char  tran_sort[41];
  char  a1[4];
}sgSwt_sys_route_grp;

struct swt_sys_route_m
{
  short q_id;
  char  trancode[11];
  char  a1;
  short route_grp;
  short flag;    	/*add by nh ���׿���֧��*/
  char  a2[6];
}sgSwt_sys_route_m;

/* D008 ISO8583���ĸ�ʽ��׼ */
struct swt_sys_8583 
{
  short tab_id;
  long fld_id; /* mod by gengling at 2015.04.20 short->long PSBC_V1.0 */
  short	fld_type;
  short fld_attr;
  short	fld_len;
  char	fld_name[21];
  char  a1;
  char	fld_rule[iEXPRESSLEN];
}sgSwt_sys_8583;

/* added by fzj at 2002.01.23, begin */
/* ISO8583E���ĸ�ʽ��׼ */
struct swt_sys_8583E
{
  short tab_id;				/* group id */
  short	fld_id;				/* field id */
  char	fld_name[21];			/* field name */
  char  a1[7];
  long	fld_type;			/* field type */
  short	fld_len;			/* field max length */
  short fld_len2;			/* �䳤�򳤶�λ�� */
  char  fld_lenunpk[iEXPRESSLEN];	/* �䳤�򳤶Ƚ������ */ 
  char  fld_lenpack[iEXPRESSLEN];	/* �䳤�򳤶ȴ������ */
  char  fld_valunpk[iEXPRESSLEN];	/* ��ֵ������� */ 
  char  fld_valpack[iEXPRESSLEN];	/* ��ֵ������� */
  char	fld_rule[iEXPRESSLEN];		/* ��У�麯�� */
  char  a2[4];
}sgSwt_sys_8583E;
/* added by fzj at 2002.01.23, end */

/* D209�¼���ˮ�� */
struct swt_evt_log 
{
  long	eventtime;
  short	id;
  char  a1[6];
  long	begintime;
  long	tranid;
  short	transtep;
  char  a2[6];
  long	msgcode;
  char	trancode[11];
  char  a3;
  short	begin_q_id;
  char	begin_trancode[11];
  char  a4[3];
  short	org_q;
  short	des_q;
}sgSwt_evt_log;

/* D010 �û���¼�����ļ� */
struct user_profile 
{
  char	user_code[11];
  char	user_desc[21];
  char	user_dept[21];
  char	user_pswd[11];
  char	user_right[iEXPRESSLEN];
}sgUser_profile;

/* D009��ʽת���� */
struct swt_sys_fmt_grp 
{
  /*delete by zcd 20141220
  short	fmt_group;
  ***end of delete by zcd 20141220*/
  /*add by zcd 20141220*/
  long	fmt_group;
  /*end of add by zcd 20141220*/
  char	fmt_desc[41];
  char  a1;
  short	tran_type;
  char	mac_create[iEXPRESSLEN];
  char	mac_check[iEXPRESSLEN];
  char	tran_sort[41];
  char  a2;
}sgSwt_sys_fmt_grp;

/* D011 ��Ϣ���� */
struct swt_sys_code
{
  long	msg_code;
  char	msg_desc[41];
  char  a1[3];
  short	msg_action;
  short	msg_q;
}sgSwt_sys_code;

/* D013 ��ر�����֯ */

struct swt_sys_msgpack
{
  short msg_type;
  short	fld_id;
  char  fld_desc[21];
  char	fld_express[51];
  short	fld_len;
  short fld_display_len;
  char  fld_alignment[2];
  char  a1[6];
}sgSwt_sys_msgpack;

struct swt_sys_batch
{
  short q_begin;
  short tran_status;
  char  tran_code[11];
  char  a1[7];
  short smtran_id;
  char  sql_express[iEXPRESSLEN];
}sgSwt_sys_batch;


struct swt_sys_ttytype
{
  short Term_id;
  char  discrib[21];
  char  message[81];
  char  lexpress[iEXPRESSLEN];
}sgSwt_sys_ttytype;

struct swt_sys_other
{
  short other_id;
  char  other_name[21];
  char  a1[3];
  short local_q;
  short other_grp;
  short other_q;
}sgSwt_sys_other;

struct swt_sys_config
{
  short log_inter;	/* ������ˮ����ѯ���ʱ�� */
  short saf_inter;	/* SAF����ѯ���ʱ�� */
  char  a1[4];
  long rerev_inter;     /* RESAF���ʱ�� */
  int  iShmkey;		/* �����ڴ�KEYֵ */
  int  iMaxtranlog;	/* ��ǰ������ˮ����¼�� */
  int  iMaxproclog;	/* ��ǰ������ˮ(����������)����¼�� */
  int  iMaxsaflog;	/* ��ǰSAF����¼�� */
  int  iShmwaittime;	/* �����ڴ����ñ�ˢ�µȴ�ʱ�� */
  char  a2[4];
  long iMaxtrannum;
  char sysmonaddr[16];
  int  sysmonport;
  char a3[4];
  long msg_num;
  long echotest_num;
  short echotest_inter;	/* Echotest������ѯ���ʱ�� */
  short log_clear_count;  /*������̻��Ѽ������*/
  char trace_term[21];
  char ftpmode[7];
  int  ftpovertime;
  int  qpacksize;
  int  qovertime;
  int  qresendnum;
}sgSwt_sys_config;

/* D201������ˮ���� */
struct swt_tran_log 
{
  long	tran_id;
  long	tran_begin;
  long	tran_end;
  long	tran_overtime;
  long  resq_offset;
  long  resp_offset;
  short	tran_status;
  short	q_tran_begin;
  short	q_target;
  short resq_len;
  short resp_len;
  char	tc_tran_begin[11];
  char  resq_file[11];
  char  resp_file[11];
  char  rev_key[151];
  char  msghdkey[151];
  /*del by baiqj20150323 ,PSBC_V1.0
  char  msghead[151];*/
  char  msghead[256];
  char  xastatus[21];		/* ��¼XA״̬ */
  char  a1[3];
}sgSwt_tran_log;

/* D202������ˮ���� */
struct swt_proc_log 
{
  long	tran_id;
  long  proc_begin;
  long  org_offset;
  short	q_target;
  short	proc_step;
  short	proc_status;
  short	rev_mode;
  /*delete by zcd 20141220
  short	rev_fmtgrp;
  ***end of delete by zcd 20141220*/
  /*add by zcd 20141220*/
  long	rev_fmtgrp;
  /*end of add by zcd 20141220*/
  short org_len;
  char  org_file[11];
  char  a1;
}sgSwt_proc_log;

/* D206����SAF�� */
struct swt_rev_saf 
{
  long 	saf_id;
  long	tran_id;
  long  saf_offset;
  long	saf_begin;
  long	saf_overtime;
  short	rev_overtime;
  short	proc_step;
  short	saf_num;
  short saf_len;
  char	saf_status[2];
  char	saf_flag[2];
  char  saf_file[11];
  char  a1;
}sgSwt_rev_saf;

/* D213����ͷ�洢�� */
struct swt_msghead
{
  long	tran_id;
  long	saf_id;
  long	head_offset;
  short	q_id;
  short	proc_step;
  short	head_len;
  char	head_key[51];
  char	head_file[11];
  char  a1[4];
}sgSwt_msghead;

/* add by gengling sgSwt_sys_tran struct */
/* �����������Ʊ� */
struct swt_sys_tran
{
  short sys_id;      /* Դ�������     */
  char tran_code[11]; /* ������         */      
  char tran_name[42];
  short priority;
  char status[2];
  long tranning_max; /* ����������� */
  char debug_level;
  long tranning_num; /* ������         */      
}sgSwt_sys_tran;

#endif
