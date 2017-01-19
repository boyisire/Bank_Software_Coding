/*==============================================================*/
/* Table: CH_DAYEND_CFG        ����������Ϣ��                   */
/*==============================================================*/
-- Create table
create table CH_DAYEND_CFG
(
  id          INTEGER not null,
  tabname     VARCHAR2(30),
  tabnamedesc VARCHAR2(30),
  fromuser    VARCHAR2(30),
  frompass    VARCHAR2(30),
  fromlink    VARCHAR2(30),
  touser      VARCHAR2(30),
  topass      VARCHAR2(30),
  tolink      VARCHAR2(30),
  operflag    CHAR(9) default '000000000',
  keepdate    VARCHAR2(3),
  createdate  VARCHAR2(3),
  datecloumn  VARCHAR2(30),
  dateformat  VARCHAR2(10) default 'YYYYMMDD',
  extendsize  INTEGER,
  execshell   VARCHAR2(500),
  monflag     CHAR(1),
  monerro     VARCHAR2(10),
  moncode     CHAR(4),
  mondesc     VARCHAR2(200),
  status      CHAR(1),
  operdate    DATE not null,
  notes       VARCHAR2(500)
)
tablespace TS_CHMAP_DATA
  pctfree 10
  initrans 1
  maxtrans 255
  storage
  (
    initial 16
    next 8
    minextents 1
    maxextents unlimited
  );
-- Add comments to the table 
comment on table CH_DAYEND_CFG
  is '������Ϣ���ñ�';
-- Add comments to the columns 
comment on column CH_DAYEND_CFG.id
  is '��ţ��������ֶδ�����001-500Ϊ��������ʹ�ã�501-999������ʱ���߲�����.';
comment on column CH_DAYEND_CFG.tabname
  is '����';
comment on column CH_DAYEND_CFG.tabnamedesc
  is '������';
comment on column CH_DAYEND_CFG.fromuser
  is 'Դ���û���';
comment on column CH_DAYEND_CFG.frompass
  is 'Դ������:����û�������������"${}"��ʶ.';
comment on column CH_DAYEND_CFG.fromlink
  is 'Դ�����Ӵ�';
comment on column CH_DAYEND_CFG.touser
  is 'Ŀ����û���';
comment on column CH_DAYEND_CFG.topass
  is 'Ŀ�������:����û�������������"${}"��ʶ.';
comment on column CH_DAYEND_CFG.tolink
  is 'Ŀ������Ӵ�';
comment on column CH_DAYEND_CFG.operflag
  is '������ʶ��Ĭ��-000000000,ָȫ���ر�.
  ��λ�ֱ�������ݱ�����ʽ|���ݱ��뷽ʽ|���ݱ�����ʽ|���ݱ�����ʽ|���ݱ����������ʽ|�ⲿִ�г��򿪹�|ռλ| ռλ|ռλ
  1.���ݱ���:
  	1.EXP��ʽ-ȫ����,
  	2.EXP��ʽ-��WHERE��������,
  	3.EXP��ʽ-����������,
  	4.EXPDP��ʽ--ȫ��,
  	5.EXPDP��ʽ--��WHERE����+����,
  	6.EXPDP��ʽ--��������,
  	7.ռλ
  	8.��λ
  	9.SQLPLUS��ʽ
  2.���ݱ���:
  	1-IMP,
  	2-IMPDP,
  	3-SQLLDR  	
  3.���ݱ�����:
  	1-Delete+Where(Դ���ݿ�),
  	2-Drop+Partition(Դ���ݿ�),
  	3-Truncate(Դ���ݿ�),
  	4-Delete+Where(Ŀ�����ݿ�),
  	5-Drop+Partition(Ŀ�����ݿ�),
  	6-Truncate(Ŀ�����ݿ�),
  4.���ݱ��޸�:
  	1-��Rename��ʽ��Ŀ����Ϊ������+����
  	2-��Rename��ʽ��Ŀ����Ϊ������+��Դ��+����
  5.���ݱ�����:
  	1-駾����ñ��е�[���������ֶ�]��ȡҪ����N��֮�������,
  	2-��ALTER+MODIFY PARTITION��ʽ��չ��һ��֮��ı�ռ�,
  	3-ͬʱִ��1��2����
  6.ռλ����
  7.ռλ����
  8.ռλ����
  9.ִ���ⲿ���򿪹�:1-�� <ԭ��ִ�У�ExecShell�ֶ��еĳ���>    
  ';
comment on column CH_DAYEND_CFG.keepdate
  is '����-������������';
comment on column CH_DAYEND_CFG.createdate
  is '����-������������';
comment on column CH_DAYEND_CFG.datecloumn
  is '�����ֶ�';
comment on column CH_DAYEND_CFG.dateformat
  is '���ڸ�ʽ';
comment on column CH_DAYEND_CFG.extendsize
  is '��չ������С��Ĭ�ϵ�λ��M';
comment on column CH_DAYEND_CFG.execshell
  is 'ִ�г���(ע����ʹ�þ���·��)';
comment on column CH_DAYEND_CFG.monflag
  is '��ؿ���:0-�أ�1-��';
comment on column CH_DAYEND_CFG.monerro
  is '��ش�����:��9λ=1λ�������ı�ʶ+1λ�û���ʶ+3λ��¼IDֵ+3λ������(������:��9λ�Ķ�������ת������.).';
comment on column CH_DAYEND_CFG.moncode
is '��ش���:������š���4λ��1λ�������ı�ʶ+3λ�ļ�¼IDֵ���������������Ϊ�ջ���0000';
comment on column CH_DAYEND_CFG.mondesc
  is '�����Ϣ����';
comment on column CH_DAYEND_CFG.status
  is '���� :0-�أ�1-��';
comment on column CH_DAYEND_CFG.operdate
  is '��������.(ע�����ֶβ���Ϊ�գ�Ҳ�����뵱��������ͬ��������޷�ִ��)';
comment on column CH_DAYEND_CFG.notes
  is '��ע:���ֶ��ڳ�����ʵ������,ֻ�������޸����ݿ��¼ʱ����ע��.';

