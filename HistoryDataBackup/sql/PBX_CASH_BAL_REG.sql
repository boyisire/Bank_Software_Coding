--预演环境
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
  is '尾箱现金余额明细登记簿';
-- Add comments to the columns 
comment on column PBX_CASH_BAL_REG.kernel_seqno
  is '中心流水号';
comment on column PBX_CASH_BAL_REG.tran_date
  is '交易日期';
comment on column PBX_CASH_BAL_REG.seqno
  is '交易流水号';
comment on column PBX_CASH_BAL_REG.tran_type
  is '交易类型 : 0-正常 1-重发 2-恢复';
comment on column PBX_CASH_BAL_REG.tran_time
  is '交易时间';
comment on column PBX_CASH_BAL_REG.tran_no
  is '前台交易码';
comment on column PBX_CASH_BAL_REG.pcode
  is '后台交易码';
comment on column PBX_CASH_BAL_REG.sys_no
  is '目标平台号';
comment on column PBX_CASH_BAL_REG.inst_no
  is '机构号';
comment on column PBX_CASH_BAL_REG.tlr_no
  is '柜员号';
comment on column PBX_CASH_BAL_REG.box_no
  is '尾箱号';
comment on column PBX_CASH_BAL_REG.cur_type
  is '币种';
comment on column PBX_CASH_BAL_REG.damage_type
  is '完残币类型 : 1-完整币 2-残损币';
comment on column PBX_CASH_BAL_REG.crdb
  is '收付类型 : 1-收 2-付';
comment on column PBX_CASH_BAL_REG.txamt
  is '交易金额';
comment on column PBX_CASH_BAL_REG.pre_bal
  is '交易前余额';
comment on column PBX_CASH_BAL_REG.pre_usable_bal
  is '交易前可用余额';
comment on column PBX_CASH_BAL_REG.pre_broken_bal
  is '交易前残损币余额';
comment on column PBX_CASH_BAL_REG.bal
  is '交易后余额';
comment on column PBX_CASH_BAL_REG.usable_bal
  is '交易后可用余额';
comment on column PBX_CASH_BAL_REG.broken_bal
  is '交易后残损币余额';
comment on column PBX_CASH_BAL_REG.sync_flag
  is '同步标志 : 0-未同步 1-已同步';
comment on column PBX_CASH_BAL_REG.sync_code
  is '同步交易码';
comment on column PBX_CASH_BAL_REG.sync_time
  is '同步时间';
comment on column PBX_CASH_BAL_REG.other1
  is '备用字段1';
comment on column PBX_CASH_BAL_REG.other2
  is '备用字段2';
comment on column PBX_CASH_BAL_REG.other3
  is '备用字段3';
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
