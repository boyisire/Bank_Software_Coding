
-- Create table
create table PBX_CASH_REG
(
  tran_date   VARCHAR2(8) not null,
  tran_time   VARCHAR2(6) not null,
  clear_date  VARCHAR2(8) not null,
  seqno       VARCHAR2(20) not null,
  serno       NUMBER(4) not null,
  tran_no     VARCHAR2(6) not null,
  pcode       VARCHAR2(6) not null,
  sys_no      VARCHAR2(11) not null,
  inst_no     VARCHAR2(11) not null,
  inst_type   VARCHAR2(1) not null,
  tlr_no      VARCHAR2(11) not null,
  box_no      VARCHAR2(10) not null,
  cur_type    VARCHAR2(3) not null,
  fee_type    VARCHAR2(4) not null,
  crdb        VARCHAR2(1) not null,
  cash_flag   VARCHAR2(1) not null,
  txamt       NUMBER(19,2) not null,
  opr_inst_no VARCHAR2(11) not null,
  opr_tlr_no  VARCHAR2(11) not null,
  auth_tlr    VARCHAR2(11) not null,
  stat        VARCHAR2(2) not null,
  natu_time   VARCHAR2(6) not null,
  sync_flag   VARCHAR2(1) not null,
  sync_code   VARCHAR2(32) not null,
  sync_time   VARCHAR2(14) not null,
  other1      VARCHAR2(32),
  other2      VARCHAR2(32),
  other3      VARCHAR2(64)
)
partition by list (TRAN_DATE)
(
  partition P20150406 values ('20150406')
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
    maxtrans 255
    storage
    (
      initial 1M
      next 1M
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
  partition P20150411 values ('20150411')
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
    )  
);
-- Add comments to the table 
comment on table PBX_CASH_REG
  is '�ֽ��ո��Ǽǲ�';
-- Add comments to the columns 
comment on column PBX_CASH_REG.tran_date
  is '��������';
comment on column PBX_CASH_REG.tran_time
  is '����ʱ��';
comment on column PBX_CASH_REG.clear_date
  is '�������';
comment on column PBX_CASH_REG.seqno
  is '��ˮ��';
comment on column PBX_CASH_REG.serno
  is '���';
comment on column PBX_CASH_REG.tran_no
  is 'ǰ̨������';
comment on column PBX_CASH_REG.pcode
  is '��̨������';
comment on column PBX_CASH_REG.sys_no
  is 'ҵ��ϵͳ';
comment on column PBX_CASH_REG.inst_no
  is '������';
comment on column PBX_CASH_REG.inst_type
  is '�������� : 0-���� 1-�������� 2-�ֹ�����';
comment on column PBX_CASH_REG.tlr_no
  is '��Ա��';
comment on column PBX_CASH_REG.box_no
  is 'β���';
comment on column PBX_CASH_REG.cur_type
  is '����';
comment on column PBX_CASH_REG.fee_type
  is '�������';
comment on column PBX_CASH_REG.crdb
  is '�ո����� : 1-�տ� 2-����';
comment on column PBX_CASH_REG.cash_flag
  is '�ֽ�ʽ : 1-�ֽ�ʽ ����-���ֽ�ʽ';
comment on column PBX_CASH_REG.txamt
  is '���׽��';
comment on column PBX_CASH_REG.opr_inst_no
  is '��������';
comment on column PBX_CASH_REG.opr_tlr_no
  is '������Ա';
comment on column PBX_CASH_REG.auth_tlr
  is '��Ȩ��Ա';
comment on column PBX_CASH_REG.stat
  is '����״̬ : 0-���� 1-ȡ�� 2-���� 3-�ط� 4-�ָ� 5-���� 6-��ʱ�ɹ� 7-ȡ�ʱʧ�� 8-ʧ�� A-��ȡ�� B-������ C-������ D-���ָ�';
comment on column PBX_CASH_REG.natu_time
  is 'ʱ���';
comment on column PBX_CASH_REG.sync_flag
  is 'ͬ����־ : 0-δͬ�� 1-��ͬ��';
comment on column PBX_CASH_REG.sync_code
  is 'ͬ��������';
comment on column PBX_CASH_REG.sync_time
  is 'ͬ��ʱ��';
comment on column PBX_CASH_REG.other1
  is '������ˮ��';
comment on column PBX_CASH_REG.other2
  is '�˺ſ���';
comment on column PBX_CASH_REG.other3
  is '�����ֶ�3';
-- Create/Recreate indexes 
create index IDX_PBX_CASH_REG_04 on PBX_CASH_REG (TRAN_DATE, INST_NO, BOX_NO)
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
-- Create/Recreate primary, unique and foreign key constraints 
alter table PBX_CASH_REG
  add primary key (TRAN_DATE, SEQNO, SERNO, SYS_NO)
  using index 
  tablespace TS_RPT_DATA
  pctfree 10
  initrans 2
  maxtrans 255
  storage
  (
    initial 9M
    next 1M
    minextents 1
    maxextents unlimited
    pctincrease 0
  );
-- Grant/Revoke object privileges 
grant select, insert, update, delete on PBX_CASH_REG to CHMAP4;
grant select, insert, update, delete on PBX_CASH_REG to CHMNG;
grant select, insert, update, delete on PBX_CASH_REG to CHMNG3;
grant select, insert, update, delete on PBX_CASH_REG to CHMNG4;
