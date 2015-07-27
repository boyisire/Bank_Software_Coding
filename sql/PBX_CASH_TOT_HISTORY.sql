
create table PBX_CASH_TOT_HISTORY
(
  TRAN_DATE       VARCHAR2(8) not null,
  INST_NO         VARCHAR2(11) not null,
  INST_TYPE       VARCHAR2(1) not null,
  BOX_NO          VARCHAR2(10) not null,
  CUR_TYPE        VARCHAR2(3) not null,
  LAST_BAL        NUMBER(19,2) not null,
  LAST_DRAW_BAL   NUMBER(19,2) not null,
  IN_SUM_AMT      NUMBER(19,2) not null,
  ALLOT_IN_AMT    NUMBER(19,2) not null,
  CR_SUM_AMT      NUMBER(19,2) not null,
  DB_SUM_AMT      NUMBER(19,2) not null,
  ALLOT_OUT_AMT   NUMBER(19,2) not null,
  OUT_SUM_AMT     NUMBER(19,2) not null,
  BAL             NUMBER(19,2) not null,
  USABLE_BAL      NUMBER(19,2) not null,
  BROKEN_LAST_BAL NUMBER(19,2) not null,
  BROKEN_IN_AMT   NUMBER(19,2) not null,
  BROKEN_CR_AMT   NUMBER(19,2) not null,
  BROKEN_DB_AMT   NUMBER(19,2) not null,
  BROKEN_OUT_AMT  NUMBER(19,2) not null,
  BROKEN_BAL      NUMBER(19,2) not null,
  SYNC_FLAG       VARCHAR2(1) not null,
  SYNC_CODE       VARCHAR2(32) not null,
  SYNC_TIME       VARCHAR2(14) not null,
  OTHER1          VARCHAR2(32),
  OTHER2          VARCHAR2(32),
  OTHER3          VARCHAR2(64)
)
partition by list (TRAN_DATE)
(
  partition P99999999  VALUES ('99999999')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 16K
      next 8K
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
   partition P20150407  VALUES ('20150407')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 16K
      next 8K
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
  partition P20150408 VALUES ('20150408')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 16K
      next 8K
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
  partition P20150409 VALUES ('20150409')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 16K
      next 8K
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
  partition P20150410 VALUES ('20150410')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 16K
      next 8K
      minextents 1
      maxextents unlimited
      pctincrease 0
    ),
  partition P20150411 VALUES ('20150411')
    tablespace TS_RPT_DATA
    pctfree 10
    initrans 1
    maxtrans 255
    storage
    (
      initial 16K
      next 8K
      minextents 1
      maxextents unlimited
      pctincrease 0
    )  
)
;



-- Add comments to the table 
comment on table PBX_CASH_TOT_HISTORY
  is '�ֽ������ʷ��';
-- Add comments to the columns 
comment on column PBX_CASH_TOT_HISTORY.TRAN_DATE
  is '��������';
comment on column PBX_CASH_TOT_HISTORY.INST_NO
  is '������';
comment on column PBX_CASH_TOT_HISTORY.INST_TYPE
  is '�������� : 0-���� 1-�������� 2-�ֹ�����';
comment on column PBX_CASH_TOT_HISTORY.BOX_NO
  is 'β���';
comment on column PBX_CASH_TOT_HISTORY.CUR_TYPE
  is '����';
comment on column PBX_CASH_TOT_HISTORY.LAST_BAL
  is '�ڳ�����';
comment on column PBX_CASH_TOT_HISTORY.LAST_DRAW_BAL
  is '����ǰ���';
comment on column PBX_CASH_TOT_HISTORY.IN_SUM_AMT
  is '�յ����';
comment on column PBX_CASH_TOT_HISTORY.ALLOT_IN_AMT
  is '������';
comment on column PBX_CASH_TOT_HISTORY.CR_SUM_AMT
  is '�տ���';
comment on column PBX_CASH_TOT_HISTORY.DB_SUM_AMT
  is '������';
comment on column PBX_CASH_TOT_HISTORY.ALLOT_OUT_AMT
  is '�������';
comment on column PBX_CASH_TOT_HISTORY.OUT_SUM_AMT
  is '�Ͻɽ��';
comment on column PBX_CASH_TOT_HISTORY.BAL
  is '�������';
comment on column PBX_CASH_TOT_HISTORY.USABLE_BAL
  is '������� : ��ȡ��ѭ��һ���ʹ��';
comment on column PBX_CASH_TOT_HISTORY.BROKEN_LAST_BAL
  is '������ڳ�����';
comment on column PBX_CASH_TOT_HISTORY.BROKEN_IN_AMT
  is '������յ����';
comment on column PBX_CASH_TOT_HISTORY.BROKEN_CR_AMT
  is '����ҹ���������';
comment on column PBX_CASH_TOT_HISTORY.BROKEN_DB_AMT
  is '����ҹ���֧�����';
comment on column PBX_CASH_TOT_HISTORY.BROKEN_OUT_AMT
  is '������Ͻɽ��';
comment on column PBX_CASH_TOT_HISTORY.BROKEN_BAL
  is '��������';
comment on column PBX_CASH_TOT_HISTORY.SYNC_FLAG
  is 'ͬ����־ : 0-δͬ�� 1-��ͬ��';
comment on column PBX_CASH_TOT_HISTORY.SYNC_CODE
  is 'ͬ��������';
comment on column PBX_CASH_TOT_HISTORY.SYNC_TIME
  is 'ͬ��ʱ��';
comment on column PBX_CASH_TOT_HISTORY.OTHER1
  is '�����ֶ�1';
comment on column PBX_CASH_TOT_HISTORY.OTHER2
  is '�����ֶ�2';
comment on column PBX_CASH_TOT_HISTORY.OTHER3
  is '�����ֶ�3';
-- Create/Recreate primary, unique and foreign key constraints 
alter table PBX_CASH_TOT_HISTORY
  add primary key (TRAN_DATE, INST_NO, BOX_NO, CUR_TYPE)
  using index 
  tablespace TS_RPT_DATA
  pctfree 10
  initrans 2
  maxtrans 255
  storage
  (
    initial 64K
    next 1M
    minextents 1
    maxextents unlimited
    pctincrease 0
  );
