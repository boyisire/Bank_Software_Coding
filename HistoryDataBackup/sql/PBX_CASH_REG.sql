
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
  is '现金收付登记簿';
-- Add comments to the columns 
comment on column PBX_CASH_REG.tran_date
  is '交易日期';
comment on column PBX_CASH_REG.tran_time
  is '交易时间';
comment on column PBX_CASH_REG.clear_date
  is '会计日期';
comment on column PBX_CASH_REG.seqno
  is '流水号';
comment on column PBX_CASH_REG.serno
  is '序号';
comment on column PBX_CASH_REG.tran_no
  is '前台交易码';
comment on column PBX_CASH_REG.pcode
  is '后台交易码';
comment on column PBX_CASH_REG.sys_no
  is '业务系统';
comment on column PBX_CASH_REG.inst_no
  is '机构号';
comment on column PBX_CASH_REG.inst_type
  is '机构类型 : 0-网点 1-自助银行 2-手工网点';
comment on column PBX_CASH_REG.tlr_no
  is '柜员号';
comment on column PBX_CASH_REG.box_no
  is '尾箱号';
comment on column PBX_CASH_REG.cur_type
  is '币种';
comment on column PBX_CASH_REG.fee_type
  is '金额类型';
comment on column PBX_CASH_REG.crdb
  is '收付类型 : 1-收款 2-付款';
comment on column PBX_CASH_REG.cash_flag
  is '现金方式 : 1-现金方式 其他-非现金方式';
comment on column PBX_CASH_REG.txamt
  is '交易金额';
comment on column PBX_CASH_REG.opr_inst_no
  is '操作机构';
comment on column PBX_CASH_REG.opr_tlr_no
  is '操作柜员';
comment on column PBX_CASH_REG.auth_tlr
  is '授权柜员';
comment on column PBX_CASH_REG.stat
  is '交易状态 : 0-正常 1-取消 2-冲正 3-重发 4-恢复 5-调整 6-存款超时成功 7-取款超时失败 8-失败 A-被取消 B-被冲正 C-被调整 D-被恢复';
comment on column PBX_CASH_REG.natu_time
  is '时间戳';
comment on column PBX_CASH_REG.sync_flag
  is '同步标志 : 0-未同步 1-已同步';
comment on column PBX_CASH_REG.sync_code
  is '同步交易码';
comment on column PBX_CASH_REG.sync_time
  is '同步时间';
comment on column PBX_CASH_REG.other1
  is '关联流水号';
comment on column PBX_CASH_REG.other2
  is '账号卡号';
comment on column PBX_CASH_REG.other3
  is '备用字段3';
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
