-- ============================================================
--   Database name:  MODEL_2                                   
--   DBMS name:      ORACLE Version 7.x                        
--   Created on:     101-3-24  11:08                           
-- ============================================================

DROP TABLE swt_his_tran_log ;

DROP TABLE swt_other_rev ;

DROP TABLE swt_sav_saf ;

DROP TABLE swt_sav_proc_log ;

DROP TABLE swt_sav_tran_log ;

DROP TABLE swt_msghead ;

DROP TABLE swt_rev_saf ;

DROP TABLE swt_his_proc_log ;

drop table swt_tran_log;

-- ============================================================
--   Table: swt_his_proc_log                                   
-- ============================================================
CREATE TABLE swt_his_proc_log
(
    tran_id          int                not null,
    proc_step        smallint               not null,
    proc_begin       int                not null,
    q_target         smallint               null    ,
    proc_status      smallint               null    ,
    rev_mode         smallint               null    ,
    rev_fmtgrp       smallint               null    ,
    org_file         varchar(10)           null    ,
    org_offset       int                null    ,
    org_len          smallint               null    
);

-- ============================================================
--   Index: AK1_swt_his_proc_log                               
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_his_proc_log on swt_his_proc_log (tran_id , proc_step , proc_begin );

-- ============================================================
--   Table: swt_rev_saf                                        
-- ============================================================
CREATE TABLE swt_rev_saf
(
    saf_id           int                not null,
    tran_id          int                null    ,
    proc_step        smallint               null    ,
    saf_begin        int                null    ,
    saf_overtime     int                null    ,
    rev_overtime     smallint               null    ,
    saf_num          smallint               null    ,
    saf_status       CHAR(1)                null    ,
    saf_flag         CHAR(1)                null    ,
    saf_file         varchar(10)           null    ,
    saf_offset       int                null    ,
    saf_len          smallint               null    
);

-- ============================================================
--   Index: AK1_swt_rev_saf                                    
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_rev_saf on swt_rev_saf (saf_id );

-- ============================================================
--   Table: swt_msghead                                        
-- ============================================================
CREATE TABLE swt_msghead
(
    q_id             smallint               not null,
    head_key         varchar(50)           not null,
    tran_id          int                null    ,
    proc_step        smallint               null    ,
    saf_id           int                null    ,
    head_file        varchar(10)           null    ,
    head_offset      int                null    ,
    head_len         smallint               null    
);

-- ============================================================
--   Index: AK1_swt_msghead                                    
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_msghead on swt_msghead (q_id , head_key );

-- ============================================================
--   Table: swt_sav_tran_log                                   
-- ============================================================
CREATE TABLE swt_sav_tran_log
(
    tran_begin       int                not null,
    tran_id          int                not null,
    tran_status      smallint               null    ,
    tran_end         int                null    ,
    tran_overtime    int                null    ,
    q_tran_begin     smallint               null    ,
    tc_tran_begin    varchar(10)           null    ,
    q_target         smallint               null    ,
    resq_file        varchar(10)           null    ,
    resq_offset      int                null    ,
    resq_len         smallint               null    ,
    resp_file        varchar(10)           null    ,
    resp_offset      int                null    ,
    resp_len         smallint               null    ,
    rev_key          varchar(150)          null    
);

-- ============================================================
--   Index: AK1_swt_sav_tran_log                               
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_sav_tran_log on swt_sav_tran_log (tran_begin , tran_id );

-- ============================================================
--   Table: swt_sav_proc_log                                   
-- ============================================================
CREATE TABLE swt_sav_proc_log
(
    tran_id          int                not null,
    proc_step        smallint               not null,
    proc_begin       int                not null,
    q_target         smallint               null    ,
    proc_status      smallint               null    ,
    rev_mode         smallint               null    ,
    rev_fmtgrp       smallint               null    ,
    org_file         varchar(10)           null    ,
    org_offset       int                null    ,
    org_len          smallint               null    
);

-- ============================================================
--   Index: AK1_swt_sav_proc_log                               
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_sav_proc_log on swt_sav_proc_log (tran_id , proc_step , proc_begin );

-- ============================================================
--   Table: swt_sav_saf                                        
-- ============================================================
CREATE TABLE swt_sav_saf
(
    saf_id           int                not null,
    tran_id          int                null    ,
    proc_step        smallint               null    ,
    saf_begin        int                null    ,
    saf_overtime     int                null    ,
    rev_overtime     smallint               null    ,
    saf_num          smallint               null    ,
    saf_status       CHAR(1)                null    ,
    saf_flag         CHAR(1)                null    ,
    saf_file         varchar(10)           null    ,
    saf_offset       int                null    ,
    saf_len          smallint               null    
);

-- ============================================================
--   Index: AK1_swt_sav_saf                                    
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_sav_saf on swt_sav_saf (saf_id );

-- ============================================================
--   Table: swt_other_rev                                      
-- ============================================================
CREATE TABLE swt_other_rev
(
    other_q          smallint               not null,
    other_id         int                not null,
    cur_id           int                null    
);

-- ============================================================
--   Index: AK1_swt_other_rev                                  
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_other_rev on swt_other_rev (other_q , other_id );

-- ============================================================
--   Table: swt_his_tran_log                                   
-- ============================================================
CREATE TABLE swt_his_tran_log
(
    tran_id          int                not null,
    tran_begin       int                not null,
    tran_status      smallint               null    ,
    tran_end         int                null    ,
    tran_overtime    int                null    ,
    q_tran_begin     smallint               null    ,
    tc_tran_begin    varchar(10)           null    ,
    q_target         smallint               null    ,
    resq_file        varchar(10)           null    ,
    resq_offset      int                null    ,
    resq_len         smallint               null    ,
    resp_file        varchar(10)           null    ,
    resp_offset      int                null    ,
    resp_len         smallint               null    ,
    rev_key          varchar(150)          null
);

-- ============================================================
--   Index: AK1_swt_his_tran_log                               
-- ============================================================
CREATE UNIQUE INDEX AK1_swt_his_tran_log on swt_his_tran_log (tran_id , tran_begin );



create table swt_tran_log \
( \
    tran_id        INTEGER               not null , \
    tran_begin     INTEGER               not null , \
    tran_status    SMALLINT	                  , \
    tran_end       INTEGER                        , \
    tran_overtime  INTEGER                        , \
    q_tran_begin   SMALLINT                       , \
    tc_tran_begin  VARCHAR(10)                    , \
    q_target       SMALLINT                       , \
    resq_file      VARCHAR(10)                    , \
    resq_offset    INTEGER                        , \
    resq_len       SMALLINT                       , \
    resp_file      VARCHAR(10)                    , \
    resp_offset    INTEGER                        , \
    resp_len       SMALLINT                       , \
    rev_key        VARCHAR(150)                   , \
    msghdkey       VARCHAR(150)                   , \
    msghead        VARCHAR(150)                   , \
    xastatus       VARCHAR(20)                     \
);
create unique index AK1_swt_tran_l on swt_tran_log (tran_id asc, tran_begin asc);

commit;
quit;
