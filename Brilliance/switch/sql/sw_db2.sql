drop table swt_his_proc_log

create table swt_his_proc_log \
( \
    tran_id        INTEGER               not null , \
    proc_step      SMALLINT              not null , \
    proc_begin     INTEGER               not null , \
    q_target       SMALLINT                       , \
    proc_status    SMALLINT                       , \
    rev_mode       SMALLINT                       , \
    rev_fmtgrp     SMALLINT                       , \
    org_file       VARCHAR(10)                    , \
    org_offset     INTEGER                        , \
    org_len        SMALLINT                        \
)

create unique index AK1_swt_his_proc_l on swt_his_proc_log (tran_id asc, proc_step asc, proc_begin asc)

drop table swt_rev_saf

create table swt_rev_saf \
( \
    saf_id         INTEGER               not null , \
    tran_id        INTEGER                        , \
    proc_step      SMALLINT                       , \
    saf_begin      INTEGER                        , \
    saf_overtime   INTEGER                        , \
    rev_overtime   SMALLINT                       , \
    saf_num        SMALLINT                       , \
    saf_status     CHAR(1)                        , \
    saf_flag       CHAR(1)                        , \
    saf_file       VARCHAR(10)                    , \
    saf_offset     INTEGER                        , \
    saf_len        SMALLINT                        \
)

create unique index AK1_swt_rev_saf on swt_rev_saf (saf_id asc)

drop table swt_msghead

create table swt_msghead \
( \
    q_id           SMALLINT              not null , \
    head_key       VARCHAR(50)           not null , \
    tran_id        INTEGER                        , \
    proc_step      SMALLINT                       , \
    saf_id         INTEGER                        , \
    head_file      VARCHAR(10)                    , \
    head_offset    INTEGER                        , \
    head_len       SMALLINT                        \
)

create unique index AK1_swt_msghead on swt_msghead (q_id asc, head_key asc)

drop table swt_sav_proc_log

create table swt_sav_proc_log \
( \
    tran_id        INTEGER               not null , \
    proc_step      SMALLINT              not null , \
    proc_begin     INTEGER               not null , \
    q_target       SMALLINT                       , \
    proc_status    SMALLINT                       , \
    rev_mode       SMALLINT                       , \
    rev_fmtgrp     SMALLINT                       , \
    org_file       VARCHAR(10)                    , \
    org_offset     INTEGER                        , \
    org_len        SMALLINT                        \
)

create unique index AK1_swt_sav_proc_l on swt_sav_proc_log (tran_id asc, proc_step asc, proc_begin asc)

drop table swt_sav_saf

create table swt_sav_saf \
( \
    saf_id         INTEGER               not null , \
    tran_id        INTEGER                        , \
    proc_step      SMALLINT                       , \
    saf_begin      INTEGER                        , \
    saf_overtime   INTEGER                        , \
    rev_overtime   SMALLINT                       , \
    saf_num        SMALLINT                       , \
    saf_status     CHAR(1)                        , \
    saf_flag       CHAR(1)                        , \
    saf_file       VARCHAR(10)                    , \
    saf_offset     INTEGER                        , \
    saf_len        SMALLINT                        \
)

create unique index AK1_swt_sav_saf on swt_sav_saf (saf_id asc)

drop table swt_other_rev

create table swt_other_rev \
( \
    other_q        SMALLINT              not null , \
    other_id       INTEGER               not null , \
    cur_id         INTEGER                         \
)

create unique index AK1_swt_other_rev on swt_other_rev (other_q asc, other_id asc) 

drop table swt_his_tran_log

create table swt_his_tran_log \
( \
    tran_id        INTEGER               not null , \
    tran_begin     INTEGER               not null , \
    tran_status    SMALLINT                       , \
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
    rev_key        VARCHAR(150)                    \
)
create unique index AK1_swt_his_tran_l on swt_his_tran_log (tran_id asc,tran_begin asc)

drop table swt_sav_tran_log

create table swt_sav_tran_log \
( \
    tran_id        INTEGER               not null , \
    tran_begin     INTEGER               not null , \
    tran_status    SMALLINT                       , \
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
    rev_key        VARCHAR(150)                    \
)

create index ix143_1 on swt_sav_tran_log (tran_id asc,tran_begin asc)

drop table swt_tran_log

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
)
create unique index AK1_swt_tran_l on swt_tran_log (tran_id asc, tran_begin asc)

drop table swt_proc_log

create table swt_proc_log \
( \
    tran_id        INTEGER               not null , \
    proc_step      SMALLINT              not null , \
    proc_begin     INTEGER               not null , \
    q_target       SMALLINT                       , \
    proc_status    SMALLINT                       , \
    rev_mode       SMALLINT                       , \
    rev_fmtgrp     SMALLINT                       , \
    org_file       VARCHAR(10)                    , \
    org_offset     INTEGER                        , \
    org_len        SMALLINT                        \
)

create unique index AK1_swt_proc_l on swt_proc_log (tran_id asc, proc_step asc, proc_begin asc)

