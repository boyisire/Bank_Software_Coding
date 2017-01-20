drop table swt_tran_log;

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