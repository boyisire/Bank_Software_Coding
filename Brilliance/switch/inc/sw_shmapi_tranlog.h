/*create by dyw, 2015-01-14*/
#ifndef SW_SHMAPI_TRANLOG_H
#define SW_SHMAPI_TRANLOG_H
#include "swShm.h"
#define NO_FREE_TRAN_LOG -1
#define HASH_TRANID(tranid)  (tranid%(SHM_TRANLOG_BUCKET_NUM))
#define GET_TRANLOG_SEM_NUM(tranid)  ( sw_tranid_sem_array[ tranid%(SHM_TRANLOG_BUCKET_NUM) ])
#define GET_TRANLOG_BUCKET_SEM_NUM(bucket)   ( sw_tranid_sem_array[bucket])
#endif
