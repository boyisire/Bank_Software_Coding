--Ԥ�ݻ���
create table PBX_CASH_BAL_REG
(
  kernel_seqno   VARCHAR2(21) not null,
  tran_date      VARCHAR2(8) not null,
  seqno          VARCHAR2(21) not null,
  tran_type      VARCHAR2(1) not null,
  tran_time      VARCHAR2(6) not null,
  tran_no        VARCHAR2(6) not null,
  pcode          VARCHAR2(6) not null,
  sys_no         VARCHAR2(11) not null,
  inst_no        VARCHAR2(8) not null,
  tlr_no         VARCHAR2(11) not null,
  box_no         VARCHAR2(8) not null,
  cur_type       VARCHAR2(3) not null,
  damage_type    VARCHAR2(1) not null,
  crdb           VARCHAR2(1) not null,
  txamt          NUMBER(19,2) not null,
  pre_bal        NUMBER(19,2) not null,
  pre_usable_bal NUMBER(19,2) not null,
  pre_broken_bal NUMBER(19,2) not null,
  bal            NUMBER(19,2) not null,
  usable_bal     NUMBER(19,2) not null,
  broken_bal     NUMBER(19,2) not null,
  sync_flag      VARCHAR2(1) not null,
  sync_code      VARCHAR2(32) not null,
  sync_time      VARCHAR2(14) not null,
  other1         VARCHAR2(32),
  other2         VARCHAR2(32),
  other3         VARCHAR2(64)
)
partition by list (TRAN_DATE)
(
  partition P20150407 values ('20150407')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 1M
      next 1M
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
    partition P20150408 values ('20150408')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 1M
      next 1M
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
      partition P20150409 values ('20150409')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 1M
      next 1M
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
  partition P20150410 values ('20150410')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255,
  partition P20150411 values ('20150411')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
);
-- Add comments to the table 
comment on table PBX_CASH_BAL_REG
  is 'β���ֽ������ϸ�Ǽǲ�';
-- Add comments to the columns 
comment on column PBX_CASH_BAL_REG.kernel_seqno
  is '������ˮ��';
comment on column PBX_CASH_BAL_REG.tran_date
  is '��������';
comment on column PBX_CASH_BAL_REG.seqno
  is '������ˮ��';
comment on column PBX_CASH_BAL_REG.tran_type
  is '�������� : 0-���� 1-�ط� 2-�ָ�';
comment on column PBX_CASH_BAL_REG.tran_time
  is '����ʱ��';
comment on column PBX_CASH_BAL_REG.tran_no
  is 'ǰ̨������';
comment on column PBX_CASH_BAL_REG.pcode
  is '��̨������';
comment on column PBX_CASH_BAL_REG.sys_no
  is 'Ŀ��ƽ̨��';
comment on column PBX_CASH_BAL_REG.inst_no
  is '������';
comment on column PBX_CASH_BAL_REG.tlr_no
  is '��Ա��';
comment on column PBX_CASH_BAL_REG.box_no
  is 'β���';
comment on column PBX_CASH_BAL_REG.cur_type
  is '����';
comment on column PBX_CASH_BAL_REG.damage_type
  is '��б����� : 1-������ 2-�����';
comment on column PBX_CASH_BAL_REG.crdb
  is '�ո����� : 1-�� 2-��';
comment on column PBX_CASH_BAL_REG.txamt
  is '���׽��';
comment on column PBX_CASH_BAL_REG.pre_bal
  is '����ǰ���';
comment on column PBX_CASH_BAL_REG.pre_usable_bal
  is '����ǰ�������';
comment on column PBX_CASH_BAL_REG.pre_broken_bal
  is '����ǰ��������';
comment on column PBX_CASH_BAL_REG.bal
  is '���׺����';
comment on column PBX_CASH_BAL_REG.usable_bal
  is '���׺�������';
comment on column PBX_CASH_BAL_REG.broken_bal
  is '���׺��������';
comment on column PBX_CASH_BAL_REG.sync_flag
  is 'ͬ����־ : 0-δͬ�� 1-��ͬ��';
comment on column PBX_CASH_BAL_REG.sync_code
  is 'ͬ��������';
comment on column PBX_CASH_BAL_REG.sync_time
  is 'ͬ��ʱ��';
comment on column PBX_CASH_BAL_REG.other1
  is '�����ֶ�1';
comment on column PBX_CASH_BAL_REG.other2
  is '�����ֶ�2';
comment on column PBX_CASH_BAL_REG.other3
  is '�����ֶ�3';
-- Create/Recreate primary, unique and foreign key constraints 
alter table PBX_CASH_BAL_REG
  add primary key (KERNEL_SEQNO)
  using index 
  tablespace TS_RPT_DATA
  pctfree 10
  initrans 2
  maxtrans 255
  storage
  (
    initial 1M
    next 1M
    minextents 1
    maxextents unlimited
    pctincrease 0
  );
