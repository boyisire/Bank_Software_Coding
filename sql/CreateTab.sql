/*==============================================================*/
/* Table: CH_DAYEND_CFG        日终配置信息表                   */
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
  is '日终信息配置表';
-- Add comments to the columns 
comment on column CH_DAYEND_CFG.id
  is '序号：建议对其分段处理。如001-500为正常日终使用，501-999可做临时或者测试用.';
comment on column CH_DAYEND_CFG.tabname
  is '表名';
comment on column CH_DAYEND_CFG.tabnamedesc
  is '中文名';
comment on column CH_DAYEND_CFG.fromuser
  is '源库用户名';
comment on column CH_DAYEND_CFG.frompass
  is '源库密码:如果用环境变量，请用"${}"标识.';
comment on column CH_DAYEND_CFG.fromlink
  is '源库连接串';
comment on column CH_DAYEND_CFG.touser
  is '目标库用户名';
comment on column CH_DAYEND_CFG.topass
  is '目标库密码:如果用环境变量，请用"${}"标识.';
comment on column CH_DAYEND_CFG.tolink
  is '目标库连接串';
comment on column CH_DAYEND_CFG.operflag
  is '操作标识：默认-000000000,指全部关闭.
  九位分别代表：数据表导出方式|数据表导入方式|数据表清理方式|数据表创建方式|数据表分区操作方式|外部执行程序开关|占位| 占位|占位
  1.数据表导出:
  	1.EXP方式-全表导出,
  	2.EXP方式-按WHERE条件导出,
  	3.EXP方式-按分区导出,
  	4.EXPDP方式--全表,
  	5.EXPDP方式--按WHERE条件+日期,
  	6.EXPDP方式--按分区导,
  	7.占位
  	8.点位
  	9.SQLPLUS方式
  2.数据表导入:
  	1-IMP,
  	2-IMPDP,
  	3-SQLLDR  	
  3.数据表清理:
  	1-Delete+Where(源数据库),
  	2-Drop+Partition(源数据库),
  	3-Truncate(源数据库),
  	4-Delete+Where(目标数据库),
  	5-Drop+Partition(目标数据库),
  	6-Truncate(目标数据库),
  4.数据表修改:
  	1-用Rename方式将目标表改为：表名+日期
  	2-用Rename方式将目标表改为：表名+来源库+日期
  5.数据表新增:
  	1-椐据配置表中的[新增日期字段]获取要创建N天之后的日期,
  	2-用ALTER+MODIFY PARTITION方式扩展下一天之后的表空间,
  	3-同时执行1、2操作
  6.占位备用
  7.占位备用
  8.占位备用
  9.执行外部程序开关:1-打开 <原样执行：ExecShell字段中的程序>    
  ';
comment on column CH_DAYEND_CFG.keepdate
  is '清理-保留日期天数';
comment on column CH_DAYEND_CFG.createdate
  is '创建-新增日期天数';
comment on column CH_DAYEND_CFG.datecloumn
  is '日期字段';
comment on column CH_DAYEND_CFG.dateformat
  is '日期格式';
comment on column CH_DAYEND_CFG.extendsize
  is '扩展分区大小，默认单位：M';
comment on column CH_DAYEND_CFG.execshell
  is '执行程序(注：请使用绝对路径)';
comment on column CH_DAYEND_CFG.monflag
  is '监控开关:0-关，1-开';
comment on column CH_DAYEND_CFG.monerro
  is '监控错误码:共9位=1位数据中心标识+1位用户标识+3位记录ID值+3位错误码(错误码:由9位的二进制数转换而来.).';
comment on column CH_DAYEND_CFG.moncode
is '监控代码:即步骤号。共4位：1位数据中心标识+3位的记录ID值。如果不用则将其置为空或者0000';
comment on column CH_DAYEND_CFG.mondesc
  is '监控信息描述';
comment on column CH_DAYEND_CFG.status
  is '开关 :0-关，1-开';
comment on column CH_DAYEND_CFG.operdate
  is '操作日期.(注：该字段不能为空，也不能与当天日期相同，否则会无法执行)';
comment on column CH_DAYEND_CFG.notes
  is '备注:该字段在程序无实际作用,只用来在修改数据库记录时做备注用.';

