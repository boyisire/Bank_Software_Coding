-- ============================================================
--   Database name:  MODEL_2                                   
--   DBMS name:      ORACLE Version 9.0.2.4                        
--   Created on:     101-3-24  11:08                           
-- ============================================================

 drop table swt_his_tran_log cascade constraints;

drop table swt_other_rev cascade constraints;

drop table swt_sav_saf cascade constraints;

drop table swt_sav_proc_log cascade constraints;

drop table swt_sav_tran_log cascade constraints;

drop table swt_msghead cascade constraints;

drop table swt_rev_saf cascade constraints;

drop table swt_his_proc_log cascade constraints;

DROP TABLE SWT_TRAN_LOG cascade constraints;

DROP TABLE SWT_PROC_LOG cascade constraints;

-- ============================================================
--   Table: swt_his_proc_log                                   
-- ============================================================
create table swt_his_proc_log
(
    tran_id          INTEGER                not null,
    proc_step        SMALLINT               not null,
    proc_begin       INTEGER                not null,
    q_target         SMALLINT               null    ,
    proc_status      SMALLINT               null    ,
    rev_mode         SMALLINT               null    ,
    rev_fmtgrp       SMALLINT               null    ,
    org_file         VARCHAR2(10)           null    ,
    org_offset       INTEGER                null    ,
    org_len          SMALLINT               null    
);

-- ============================================================
--   Index: AK1_swt_his_proc_log                               
-- ============================================================
create unique index AK1_swt_his_proc_log on swt_his_proc_log (tran_id asc, proc_step asc, proc_begin asc);

-- ============================================================
--   Table: swt_rev_saf                                        
-- ============================================================
create table swt_rev_saf
(
    saf_id           INTEGER                not null,
    tran_id          INTEGER                null    ,
    proc_step        SMALLINT               null    ,
    saf_begin        INTEGER                null    ,
    saf_overtime     INTEGER                null    ,
    rev_overtime     SMALLINT               null    ,
    saf_num          SMALLINT               null    ,
    saf_status       CHAR(1)                null    ,
    saf_flag         CHAR(1)                null    ,
    saf_file         VARCHAR2(10)           null    ,
    saf_offset       INTEGER                null    ,
    saf_len          SMALLINT               null    
);

-- ============================================================
--   Index: AK1_swt_rev_saf                                    
-- ============================================================
create unique index AK1_swt_rev_saf on swt_rev_saf (saf_id asc);


-- ============================================================
--   Table: swt_msghead                                        
-- ============================================================
create table swt_msghead
(
    q_id             SMALLINT               not null,
    head_key         VARCHAR2(50)           not null,
    tran_id          INTEGER                null    ,
    proc_step        SMALLINT               null    ,
    saf_id           INTEGER                null    ,
    head_file        VARCHAR2(10)           null    ,
    head_offset      INTEGER                null    ,
    head_len         SMALLINT               null    
);

-- ============================================================
--   Index: AK1_swt_msghead                                    
-- ============================================================
create unique index AK1_swt_msghead on swt_msghead (q_id asc, head_key asc);

-- ============================================================
--   Table: swt_sav_tran_log                                   
-- ============================================================
create table swt_sav_tran_log
(
    tran_begin       INTEGER                not null,
    tran_id          INTEGER                not null,
    tran_status      SMALLINT               null    ,
    tran_end         INTEGER                null    ,
    tran_overtime    INTEGER                null    ,
    q_tran_begin     SMALLINT               null    ,
    tc_tran_begin    VARCHAR2(10)           null    ,
    q_target         SMALLINT               null    ,
    resq_file        VARCHAR2(10)           null    ,
    resq_offset      INTEGER                null    ,
    resq_len         SMALLINT               null    ,
    resp_file        VARCHAR2(10)           null    ,
    resp_offset      INTEGER                null    ,
    resp_len         SMALLINT               null    ,
    rev_key          VARCHAR2(150)          null    
)
;

-- ============================================================
--   Index: AK1_swt_sav_tran_log                               
-- ============================================================
create unique index AK1_swt_sav_tran_log on swt_sav_tran_log (tran_begin asc, tran_id asc)
;

-- ============================================================
--   Table: swt_sav_proc_log                                   
-- ============================================================
create table swt_sav_proc_log
(
    tran_id          INTEGER                not null,
    proc_step        SMALLINT               not null,
    proc_begin       INTEGER                not null,
    q_target         SMALLINT               null    ,
    proc_status      SMALLINT               null    ,
    rev_mode         SMALLINT               null    ,
    rev_fmtgrp       SMALLINT               null    ,
    org_file         VARCHAR2(10)           null    ,
    org_offset       INTEGER                null    ,
    org_len          SMALLINT               null    
)
;

-- ============================================================
--   Index: AK1_swt_sav_proc_log                               
-- ============================================================
create unique index AK1_swt_sav_proc_log on swt_sav_proc_log (tran_id asc, proc_step asc, proc_begin asc)
;

-- ============================================================
--   Table: swt_sav_saf                                        
-- ============================================================
create table swt_sav_saf
(
    saf_id           INTEGER                not null,
    tran_id          INTEGER                null    ,
    proc_step        SMALLINT               null    ,
    saf_begin        INTEGER                null    ,
    saf_overtime     INTEGER                null    ,
    rev_overtime     SMALLINT               null    ,
    saf_num          SMALLINT               null    ,
    saf_status       CHAR(1)                null    ,
    saf_flag         CHAR(1)                null    ,
    saf_file         VARCHAR2(10)           null    ,
    saf_offset       INTEGER                null    ,
    saf_len          SMALLINT               null    
)
;

-- ============================================================
--   Index: AK1_swt_sav_saf                                    
-- ============================================================
create unique index AK1_swt_sav_saf on swt_sav_saf (saf_id asc)
;

-- ============================================================
--   Table: swt_other_rev                                      
-- ============================================================
create table swt_other_rev
(
    other_q          SMALLINT               not null,
    other_id         INTEGER                not null,
    cur_id           INTEGER                null    
)
;

-- ============================================================
--   Index: AK1_swt_other_rev                                  
-- ============================================================
create unique index AK1_swt_other_rev on swt_other_rev (other_q asc, other_id asc)
;

-- ============================================================
--   Table: swt_his_tran_log                                   
-- ============================================================
create table swt_his_tran_log
(
    tran_id          INTEGER                not null,
    tran_begin       INTEGER                not null,
    tran_status      SMALLINT               null    ,
    tran_end         INTEGER                null    ,
    tran_overtime    INTEGER                null    ,
    q_tran_begin     SMALLINT               null    ,
    tc_tran_begin    VARCHAR2(10)           null    ,
    q_target         SMALLINT               null    ,
    resq_file        VARCHAR2(10)           null    ,
    resq_offset      INTEGER                null    ,
    resq_len         SMALLINT               null    ,
    resp_file        VARCHAR2(10)           null    ,
    resp_offset      INTEGER                null    ,
    resp_len         SMALLINT               null    ,
    rev_key          VARCHAR2(150)          null
)
;

-- ============================================================
--   Index: AK1_swt_his_tran_log                               
-- ============================================================
create unique index AK1_swt_his_tran_log on swt_his_tran_log (tran_id asc, tran_begin asc)
;
-- ============================================================
--   Table: SWT_TRAN_LOG                                   
-- ============================================================

 CREATE TABLE SWT_TRAN_LOG (
		  TRAN_ID INTEGER NOT NULL , 
		  TRAN_BEGIN INTEGER NOT NULL , 
		  TRAN_STATUS SMALLINT , 
		  TRAN_END INTEGER , 
		  TRAN_OVERTIME INTEGER , 
		  Q_TRAN_BEGIN SMALLINT , 
		  TC_TRAN_BEGIN VARCHAR(10) , 
		  Q_TARGET SMALLINT , 
		  RESQ_FILE VARCHAR(10) , 
		  RESQ_OFFSET INTEGER , 
		  RESQ_LEN SMALLINT , 
		  RESP_FILE VARCHAR(10) , 
		  RESP_OFFSET INTEGER , 
		  RESP_LEN SMALLINT , 
		  REV_KEY VARCHAR(150) , 
		  MSGHDKEY VARCHAR(150) , 
		  MSGHEAD VARCHAR(150) , 
		  XASTATUS VARCHAR(20) ); 
		 
-- ============================================================
--   Index: AK1_SWT_TRAN_LOG
-- ============================================================
CREATE UNIQUE INDEX AK1_SWT_TRAN_L ON SWT_TRAN_LOG 
		(TRAN_ID ASC,
		 TRAN_BEGIN ASC);

-- ============================================================
--   Table: SWT_PROC_LOG                                   
-- ============================================================

 CREATE TABLE SWT_PROC_LOG  (
		  TRAN_ID INTEGER NOT NULL , 
		  PROC_STEP SMALLINT NOT NULL , 
		  PROC_BEGIN INTEGER NOT NULL , 
		  Q_TARGET SMALLINT , 
		  PROC_STATUS SMALLINT , 
		  REV_MODE SMALLINT , 
		  REV_FMTGRP SMALLINT , 
		  ORG_FILE VARCHAR(10) , 
		  ORG_OFFSET INTEGER , 
		  ORG_LEN SMALLINT ); 
		 
-- ============================================================
--   Index: AK1_SWT_PROC_LOG
-- ============================================================

CREATE UNIQUE INDEX AK1_SWT_PROC_L ON SWT_PROC_LOG 
		(TRAN_ID ASC,
		 PROC_STEP ASC,
		 PROC_BEGIN ASC);
