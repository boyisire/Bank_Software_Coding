#include <assert.h>
#include "switch.h"
#include "swDbstruct.h"
#include "swShm.h"
#include "sw_shmapi_tranlog.h"
/****************************************************************/
/* 模块编号    ：sw_shmapi_tranlog                                       */
/* 模块名称    ：tran_log 共享内存api                                    */
/* 版 本 号    ：V4.3.0                                         */
/* 作    者    ：                                               */
/* 建立日期    ：2015/01/14                                     */
/* 最后修改日期：                                               */
/* 模块用途    ：tran_log 共享内存操作函数                               */
/* 本模块中包含如下函数及功能说明：                             */
/****************************************************************/

/*		swShminsert_swt_tran_log()			*/
/*		swShmdelete_swt_tran_log()			*/
/*		swShmupdate_swt_tran_log()			*/
/*		swShmselect_swt_tran_log()			*/
/*      shm_swt_sys_tran() HASH             */

#define SFTRANLOGID	0
#define SFPROCLOGID	1
#define SFSAFLOGID	2
#define OFFSETNULL 	-1

#define WITH_TIME_MSEC
//计算时间,并输出耗时信息
#ifdef WITH_TIME_MSEC

#include <sys/time.h>
#define _WITH_ITME_START struct timeval tv_start, tv_end; do{\
                gettimeofday(&tv_start, NULL);\
            }while(0);
#define _WITH_ITME_END do{\
                gettimeofday(&tv_end, NULL);\
                float microsec = (tv_end.tv_sec - tv_start.tv_sec) * 1000000 + \
                        (tv_end.tv_usec - tv_start.tv_usec);\
                swVdebug(0, "[%s, func_name:%s, line:%d] took %0.6fs time\r\n", \
                        __FILE__, __func__, __LINE__, microsec/ 1000000); \
}while(0);
#else

#include <time.h>
#define _WITH_ITME_START time_t ct_start, ct_end; do{\
                time (&ct_start);\
            }while(0);
#define _WITH_ITME_END do{\
                time (&ct_end);\
                swVdebug(0, "[%s, func_name:%s, line:%d] took %us time\r\n",\
                        __FILE__, __func__, __LINE__, ct_end-ct_start);\
}while(0);
#endif

const char sw_tranid_sem_array[SHM_TRANLOG_BUCKET_NUM] = { 1, 2, 3, 4 , 5, 6, 7, 8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32};

/*static*/ extern struct swt_sys_config smSwt_sys_config;
/*static*/extern  struct shmidx_s *psmShmidx_s;   /* 静态表共享内存目录段指针 */
/*static*/extern struct shmidx_d *psmShmidx_d;   /* 动态表共享内存目录段指针 */
/*static*/extern struct shmbuf_s smShmbuf_s;     /* 静态表共享内存内容段指针 */
/*static*/extern  struct shmbuf_d smShmbuf_d;     /* 动态表共享内存内容段指针 */
/*=================== 动态表共享内存操作函数 ===================*/

short sw_malloc_tran_log_link(int bucket,unsigned int *free_node);
short sw_free_tran_log_link(int bucket, unsigned int free_node);

#ifdef HASH
/*======================= swt_tran_log =========================*/

/*根据索引定位交易流量控制表在内存中的记录*/
int shm_swt_sys_tran(struct swt_sys_tran *p_in, struct swt_sys_tran **p_out)
{
    char idx_str[64];
    long record_offset = 0;
    long idx_offset = 0;
    long dataAreaOffset = 0;

    idx_offset =  sizeof(long) + sizeof(int);

    /*构建主键*/
    memset(idx_str, 0x00, sizeof(idx_str));
    snprintf(idx_str, sizeof(idx_str), "%d|%s", p_in->sys_id, p_in->tran_code);

    /*通过主键索引定位记录*/
    memcpy(&dataAreaOffset, smShmbuf_s.psSwt_sys_tran_hash, sizeof(long));
    if ((record_offset = idxSelect(smShmbuf_s.psSwt_sys_tran_hash + idx_offset, idx_str)) < 0)
    {
        swVdebug(1, "shm_swt_sys_tran idxSelect record_offset error: idx_str[%s], record_offset[%ld]",
                idx_str, record_offset);
        *p_out = NULL;
        return SHMNOTFOUND;
    }

    *p_out = (struct swt_sys_tran *)(smShmbuf_s.psSwt_sys_tran_hash +  record_offset + dataAreaOffset);

    return 0;
}
#endif

/***************************************************************
 ** 函数名      : swShminsert_swt_tran_log
 ** 功  能      : 插入交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/

int swShminsert_swt_tran_log(struct swt_tran_log sSwt_tran_log,char *cLoc_flag)
{

    int i,j,ilElink;
    long tranid;
    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    struct shmidx_d * p_shm_idx;

    tranid = sSwt_tran_log.tran_id;
    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ HASH_TRANID(tranid) ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ HASH_TRANID(tranid) ]);
    p_shm_idx =  psmShmidx_d;
    /*== End ==add by dyw,  2015-0114 ====*/

    if( *cLoc_flag == '0' )
    {
        if (swShmP( GET_TRANLOG_SEM_NUM(tranid) ) == FAIL) return(FAIL);  /*modifed by dyw*/

#if defined _test_lock_
        sleep(2);
#endif

        /* i = p_idx_tran_log->iFlink;*/  /*delete by dyw*/

        /*add by dyw, get free link from free link list*/
        if ( SUCCESS != sw_malloc_tran_log_link(HASH_TRANID( tranid ), &i )  )
        {
            if (swShmV( GET_TRANLOG_SEM_NUM( tranid )) == FAIL) return(FAIL);
            swVdebug(0,"S5060: Shm is full");
            return NO_FREE_TRAN_LOG;
        }

#if 0 
        /*delete by dyw, repalce by sw_malloc_tran_log_link*/
        for (;;)
        {
            if (p_shm_tran_log[i-1].sLinkinfo.cTag != 1) break;
            i++;
            if (i > ( smSwt_sys_config.iMaxtranlog/SHM_TRANLOG_BUCKET_NUM )) i = 1;
            if (i == p_idx_tran_log->iFlink)
            {
                if (swShmV( GET_TRANLOG_SEM_NUM( tranid )) == FAIL) return(FAIL);
                swVdebug(4,"S5060: Shm is full,insert into db");

#ifdef DB_SUPPORT  /*support database 2014-12*/
                ilRc = swDbinsert_swt_tran_log(sSwt_tran_log);
                if (ilRc == FAIL)
                    return(FAIL);
                *cLoc_flag = '1';
                return(SUCCESS);
                /*add by zcd 20141225*/
#else
#ifdef ONLY_SHM_SUPPORTED
                return (SHMFULL);
#else
                return (FAIL);
#endif
#endif
                /*end of add*/
            }       
        }
#endif

        memcpy((char *)(&(p_shm_tran_log[i-1].sSwt_tran_log)),
                (char *)&sSwt_tran_log, sizeof(struct swt_tran_log));

        /* 统计在共享内存当中的交易流水数 */
        p_shm_idx->iRecinshm[SFTRANLOGID] ++;  

        /* === begin of added by fzj at 2002.02.28 === */
        /* 统计端口源发交易数 */
        for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
        {
            if (smShmbuf_s.psSwt_sys_queue[j].q_id == sSwt_tran_log.q_tran_begin)
            {
                smShmbuf_s.psSwt_sys_queue[j].tranbegin_num += 1;
                if (sSwt_tran_log.q_target<=0) break;
            }
            if (smShmbuf_s.psSwt_sys_queue[j].q_id == sSwt_tran_log.q_target)
            {
                smShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
                if (sSwt_tran_log.q_tran_begin<=0) break;
            }
        }
        /* === end of added by fzj at 2002.02.28 === */

#ifndef HASH
        /* add by gengling at 2015.03.18 begin PSBC_V1.0 */
        /* 统计交易流量数 */
        _WITH_ITME_START;
        swVdebug(4, "sSwt_tran_log.q_tran_begin [%d] tc_tran_begin [%s]", sSwt_tran_log.q_tran_begin, sSwt_tran_log.tc_tran_begin);
        for (j=0; j<psmShmidx_s->sIdx_sys_tran.iCount; j++)
        {
            if (smShmbuf_s.psSwt_sys_tran[j].sys_id == sSwt_tran_log.q_tran_begin &&
                !strncmp(smShmbuf_s.psSwt_sys_tran[j].tran_code, sSwt_tran_log.tc_tran_begin, sizeof(smShmbuf_s.psSwt_sys_tran[j].tran_code)))
            {
                smShmbuf_s.psSwt_sys_tran[j].tranning_num += 1;
                swVdebug(4, "+1 smShmbuf_s.psSwt_sys_tran[%d].tranning_num [%ld]", j, smShmbuf_s.psSwt_sys_tran[j].tranning_num);
            }
        }
        _WITH_ITME_END;
        /* add by gengling at 2015.03.18 end PSBC_V1.0 */
#else
        int iRc = 0;
        struct swt_sys_tran p_in;
        struct swt_sys_tran *p_out = NULL;
        _WITH_ITME_START;

        p_in.sys_id = sSwt_tran_log.q_tran_begin;
        strcpy(p_in.tran_code,sSwt_tran_log.tc_tran_begin);

        iRc = shm_swt_sys_tran(&p_in, &p_out);
        if(iRc)
        {
            swVdebug(1, "shm_swt_sys_tran error iRc[%d]", iRc);
	    if (swShmV(GET_TRANLOG_SEM_NUM(tranid)) == FAIL) return(FAIL);  /*modifed by dyw*/
            return(FAIL);
        }

        p_out->tranning_num += 1;
        swVdebug(5, "sys_id[%d],tran_code[%s],tran_name[%s],priority[%d],status[%s],tranning_max[%ld],debug_level[%c],tranning_num[%ld]",
                p_out->sys_id, p_out->tran_code, p_out->tran_name, p_out->priority, p_out->status,
                p_out->tranning_max, p_out->debug_level, p_out->tranning_num); 
        _WITH_ITME_END;
#endif

        //    p_shm_tran_log[i-1].sLinkinfo.cTag = 1;
        p_shm_tran_log[i-1].sLinkinfo.iRlink = 0;
        ilElink =  p_idx_tran_log->iElink;
        p_shm_tran_log[i-1].sLinkinfo.iLlink = ilElink;
        if (ilElink > 0)
            p_shm_tran_log[ilElink-1].sLinkinfo.iRlink = i;

        if (p_idx_tran_log->iSlink == 0)
            p_idx_tran_log->iSlink = i;

        p_idx_tran_log->iElink = i;

        /* delete by dyw, free link  management has been realized in sw_malloc_tran_log_link
           if (i < (smSwt_sys_config.iMaxtranlog /SHM_TRANLOG_BUCKET_NUM) )
           p_idx_tran_log->iFlink = i+1;
           else
           p_idx_tran_log->iFlink = 1;  
         */
        if (swShmV(GET_TRANLOG_SEM_NUM(tranid)) == FAIL) return(FAIL);  /*modifed by dyw*/

    }
    else if( *cLoc_flag == '1' )
    {
#ifdef DB_SUPPORT  /*support database 2014-12*/
        ilRc = swDbinsert_swt_tran_log(sSwt_tran_log);
        if (ilRc == FAIL) 
            return(FAIL);
#else
        return (FAIL);
#endif
    }
    
    swVdebug(2,"S5064:insert tranlog id : %ld", tranid);
    return(SUCCESS);
}



/***************************************************************
 ** 函数名      : swShmdelete_swt_tran_log
 ** 功  能      : 删除交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmdelete_swt_tran_log(long lTran_id,char cLoc_flag)
{

    int i,j,ilLlink,ilRlink;
    short ilQorg,ilQdes;

    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    struct shmidx_d * p_shm_idx;

    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ HASH_TRANID(lTran_id) ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ HASH_TRANID(lTran_id) ]);
    p_shm_idx =	psmShmidx_d;
    /*== End ==add by dyw,  2015-0114 ====*/

    /* add by gengling at 2015.03.18 begin PSBC_V1.0 */
    short sys_id;
    char  tran_code[11];
    /* add by gengling at 2015.03.18 end PSBC_V1.0 */

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP( GET_TRANLOG_SEM_NUM( lTran_id ) ) == FAIL) return(FAIL);

        i = p_idx_tran_log->iSlink;
        while (i)
        {
            if (p_shm_tran_log[i-1].sSwt_tran_log.tran_id == lTran_id)
            {
                /* === begin of added by fzj at 2002.02.28 === */
                /* 统计端口源发交易数 */
                ilQorg = p_shm_tran_log[i-1].sSwt_tran_log.q_tran_begin;
                ilQdes = p_shm_tran_log[i-1].sSwt_tran_log.q_target;
                for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
                {
                    if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQorg)
                    {
                        if (smShmbuf_s.psSwt_sys_queue[j].tranbegin_num > 0) 
                        {
                            (smShmbuf_s.psSwt_sys_queue[j].tranbegin_num)--;
                        }
                        if (ilQdes<=0) break;
                    }
                    if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
                    {
                        if (smShmbuf_s.psSwt_sys_queue[j].traning_num > 0)
                        {
                            (smShmbuf_s.psSwt_sys_queue[j].traning_num)--;
                        }
                        if (ilQorg<=0) break;
                    }
                }
                /* === end of added by fzj at 2002.02.28 === */      

#ifndef HASH
                /* add by gengling at 2015.03.18 begin PSBC_V1.0 */
                /* 统计交易流量数 */
                _WITH_ITME_START;
                sys_id = p_shm_tran_log[i-1].sSwt_tran_log.q_tran_begin;
                memset(tran_code, 0x00, sizeof(tran_code));
                memcpy(tran_code, p_shm_tran_log[i-1].sSwt_tran_log.tc_tran_begin, sizeof(tran_code));
                swVdebug(4, "sSwt_tran_log.q_tran_begin [%d] tc_tran_begin [%s]", sys_id, tran_code);
                for (j=0; j<psmShmidx_s->sIdx_sys_tran.iCount; j++)
                {
                    if (smShmbuf_s.psSwt_sys_tran[j].sys_id == sys_id &&
                        !strncmp(smShmbuf_s.psSwt_sys_tran[j].tran_code, tran_code, sizeof(smShmbuf_s.psSwt_sys_tran[j].tran_code)))
                    {
                        if (smShmbuf_s.psSwt_sys_tran[j].tranning_num > 0)
                        {
                          smShmbuf_s.psSwt_sys_tran[j].tranning_num -= 1;
                          swVdebug(4, "-1 smShmbuf_s.psSwt_sys_tran[%d].tranning_num [%ld]", j, smShmbuf_s.psSwt_sys_tran[j].tranning_num);
                        }
                    }
                }
                _WITH_ITME_END;
                /* add by gengling at 2015.03.18 end PSBC_V1.0 */
#else
                int iRc = 0;
                struct swt_sys_tran p_in;
                struct swt_sys_tran *p_out;

        _WITH_ITME_START;
                p_in.sys_id = p_shm_tran_log[i-1].sSwt_tran_log.q_tran_begin;
                strcpy(p_in.tran_code, p_shm_tran_log[i-1].sSwt_tran_log.tc_tran_begin);

                iRc = shm_swt_sys_tran(&p_in, &p_out);
                if(iRc)
                {
                    swVdebug(1, "shm_swt_sys_tran error iRc[%d]", iRc);
		    if(swShmV( GET_TRANLOG_SEM_NUM( lTran_id ) ) == FAIL) return(FAIL);
                    return(FAIL);
                }

                if (p_out->tranning_num > 0)
                    p_out->tranning_num -= 1;

                swVdebug(5, "sys_id[%d],tran_code[%s],tran_name[%s],priority[%d],status[%s],tranning_max[%ld],debug_level[%c],tranning_num[%ld]",
                        p_out->sys_id, p_out->tran_code, p_out->tran_name, p_out->priority, p_out->status,
                        p_out->tranning_max, p_out->debug_level, p_out->tranning_num); 
        _WITH_ITME_END;
#endif
                /* 统计在共享内存当中的交易流水数 */
                p_shm_idx->iRecinshm[SFTRANLOGID] --;

                ilLlink = p_shm_tran_log[i-1].sLinkinfo.iLlink;
                ilRlink = p_shm_tran_log[i-1].sLinkinfo.iRlink;
                if ((p_idx_tran_log->iSlink == i) &&
                        (p_idx_tran_log->iElink == i)) 
                {
                    p_idx_tran_log->iSlink = 0;
                    p_idx_tran_log->iElink = 0;
                }
                else
                {
                    if (p_idx_tran_log->iSlink == i)
                    {
                        if(ilRlink > 0)
                            p_shm_tran_log[ilRlink-1].sLinkinfo.iLlink = 0;
                        p_idx_tran_log->iSlink = ilRlink;
                    }
                    else if (p_idx_tran_log->iElink == i)
                    {
                        if(ilLlink > 0)
                            p_shm_tran_log[ilLlink-1].sLinkinfo.iRlink = 0;
                        p_idx_tran_log->iElink = ilLlink;
                    }
                    else
                    {
                        p_shm_tran_log[ilLlink-1].sLinkinfo.iRlink = ilRlink;
                        p_shm_tran_log[ilRlink-1].sLinkinfo.iLlink = ilLlink;
                    }
                }
                /* delelte by dyw, repalced by 
                   p_idx_tran_log->iFlink = i;
                   p_shm_tran_log[i-1].sLinkinfo.cTag = 0;    
                 */

                /*add by dyw, free the link into free link pool*/
                sw_free_tran_log_link( HASH_TRANID( lTran_id ), i);

                if(swShmV( GET_TRANLOG_SEM_NUM( lTran_id ) ) == FAIL)
                    return(FAIL);
                else
                {
                    swVdebug(2,"S5064:delete tranlog id : %ld", lTran_id);
                    return(SUCCESS);          
                }
            }
            i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV( GET_TRANLOG_SEM_NUM(lTran_id) ) == FAIL) return(FAIL);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/
    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        ilRc = swDbdelete_swt_tran_log(lTran_id);
        return(ilRc);
    }
#endif
    return (SHMNOTFOUND);
} 

/***************************************************************
 ** 函数名      : swShmupdate_swt_tran_log
 ** 功  能      : 更新交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmupdate_swt_tran_log(long lTran_id, struct swt_tran_log sSwt_tran_log,char cLoc_flag)
{
    int i,j;
    short ilQdes,ilQdes_old;

    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    struct shmidx_d * p_shm_idx;

    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ HASH_TRANID(lTran_id) ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ HASH_TRANID(lTran_id) ]);
    p_shm_idx =	psmShmidx_d;
    /*== End ==add by dyw,  2015-0114 ====*/

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP( GET_TRANLOG_SEM_NUM( lTran_id ) ) == FAIL) return(FAIL);

        i = p_idx_tran_log->iSlink;
        while (i)
        {
            if (p_shm_tran_log[i-1].sSwt_tran_log.tran_id == lTran_id)
            {
                ilQdes_old = p_shm_tran_log[i-1].sSwt_tran_log.q_target;
                ilQdes = sSwt_tran_log.q_target;
                memcpy((char *)(&(p_shm_tran_log[i-1].sSwt_tran_log)),
                        (char *)&sSwt_tran_log, sizeof(struct swt_tran_log));
                /* === begin of added by fzj at 2002.02.28 === */
                if ((ilQdes > 0) && (ilQdes != ilQdes_old))
                {
                    for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
                    {
                        if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
                        {
                            smShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
                            if (ilQdes_old <= 0) break;
                        }
                        if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes_old)
                        {
                            smShmbuf_s.psSwt_sys_queue[j].traning_num -= 1;
                            if (ilQdes<=0) break;
                        }
                    }
                }
                /* === end of added by fzj at 2002.02.28 === */
                if(swShmV( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL)
                    return(FAIL);
                else
                    return(SUCCESS);  
            }
            i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL) return(FAIL);
    }

#ifdef DB_SUPPORT  /*support database 2014-12*/

    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        swVdebug(4,"S5065: swShmupdate_swt_tran_log select from db");
        ilRc = swDbupdate_swt_tran_log(lTran_id, sSwt_tran_log);
        return(ilRc);
    }
#endif
    return (SHMNOTFOUND);
} 

/***************************************************************
 ** 函数名      : swShmupdate_setstat_tranlog 
 ** 功  能      ：更新交易流水状态
 ** 作  者      :
 ** 建立日期    : 2001/05/31
 ** 最后修改日期: 2001/05/31
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmupdate_setstat_tranlog(long lTran_id,short iStatus,char cLoc_flag)
{
    int i;

    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    struct shmidx_d * p_shm_idx;

    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ HASH_TRANID(lTran_id) ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ HASH_TRANID(lTran_id) ]);
    p_shm_idx =	psmShmidx_d;
    /*== End ==add by dyw,  2015-0114 ====*/



    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        swVdebug(4,"S5070: swShmupdate_setstat_tranlog select from shm");
        if (swShmP( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL) return(FAIL);

        i = p_idx_tran_log->iSlink;
        while (i)
        {
            if (p_shm_tran_log[i-1].sSwt_tran_log.tran_id == lTran_id)
            {
                p_shm_tran_log[i-1].sSwt_tran_log.tran_status = iStatus;
                if(swShmV( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL)
                    return(FAIL);
                else
                    return(SUCCESS);  
            }
            i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL) return(FAIL);
    }
#ifdef DB_SUPPORT  /*support database 2014-12*/

    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        swVdebug(4,"S5075: swShmupdate_setstat_tranlog select from db");
        ilRc = swDbupdate_setstat_tranlog(lTran_id, iStatus);
        return(ilRc);
    }
#endif
    return(SHMNOTFOUND);
}

/***************************************************************
 ** 函数名      : swShmselect_swt_tran_log
 ** 功  能      : 查找交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_swt_tran_log(long lTran_id, struct swt_tran_log *psSwt_tran_log,char cLoc_flag)
{
    int i;

    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    struct shmidx_d * p_shm_idx;

    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ HASH_TRANID(lTran_id) ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ HASH_TRANID(lTran_id) ]);
    p_shm_idx =	psmShmidx_d;
    /*== End ==add by dyw,  2015-0114 ====*/


    if((cLoc_flag == '0')||(cLoc_flag == 'a'))
    {
        if(swShmP( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL)
            return(FAIL);
        i = p_idx_tran_log->iSlink;
        while (i)
        {
swVdebug(1,"gengling ----------------------- %d",i);
            if(p_shm_tran_log[i-1].sSwt_tran_log.tran_id == lTran_id)
            {
                memcpy((char *)psSwt_tran_log, (char *)(&(p_shm_tran_log
                                [i-1].sSwt_tran_log)), sizeof(struct swt_tran_log));
                if(swShmV( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL)
                    return(FAIL);
                else
                    return(SUCCESS);  
            }
            i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
        }
        if(swShmV( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL) return(FAIL);
    }
#ifdef DB_SUPPORT  /*support database 2014-12*/
    if((cLoc_flag == '1')||(cLoc_flag == 'a'))
    {
        ilRc = swDbselect_swt_tran_log(lTran_id, psSwt_tran_log);
        return(ilRc);
    }
#endif
    return(SHMNOTFOUND);
}



#if 1

/***************************************************************
 ** 函数名      : swShmselect_overtime_min_tranlog_update
 ** 功  能      : 查找超时的最小交易流水记录,并更新流水状态为正在冲正
 ** 作  者      : cy
 ** 建立日期    : 2004/03/24
 ** 最后修改日期: 2004/03/24
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_overtime_min_tranlog_update(long lTime,struct swt_tran_log *psSwt_tran_log,char *cLoc_flag)
{
    int i,j,ilRc,ilTmp = 0;
    long ilTran_id = lMAXSERNO; 
    int k,m;
    struct swt_tran_log slSwt_tran_log;

    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    struct shmidx_d * p_shm_idx;

    swVdebug(2,"swShmselect_overtime_min_tranlog_update---------------------");
   p_shm_idx =	psmShmidx_d;
   for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
   {
    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);
	if(swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
  	i = p_idx_tran_log->iSlink;  
	/*search fromn the start link, because list is ascend order by tranid*/
    while(i)
    {
        swVdebug(2,"tran_status[%d]",p_shm_tran_log[i-1].sSwt_tran_log.tran_status );
        swVdebug(2,"tran_overtime[%d]",p_shm_tran_log[i-1].sSwt_tran_log.tran_overtime);
        swVdebug(2,"tranid[%ld]",p_shm_tran_log[i-1].sSwt_tran_log.tran_id);
        swVdebug(2,"ilTran_id[%ld]",ilTran_id);
        swVdebug(2,"lTime[%d]",lTime);

   	/*==== add by lh 20150120 begin ====*/	  
    	if(p_shm_tran_log[i-1].sSwt_tran_log.tran_status == 0 &&
       		p_shm_tran_log[i-1].sSwt_tran_log.tran_overtime < lTime &&
       		p_shm_tran_log[i-1].sSwt_tran_log.tran_overtime != 0 &&
       		p_shm_tran_log[i-1].sSwt_tran_log.tran_id < ilTran_id) 
    	{
      		if (ilTmp == 0) ilTmp = 1;
      		j = i;
	  		m = k;
	  		ilTran_id = p_shm_tran_log[i-1].sSwt_tran_log.tran_id;
      		swVdebug(2,"find tranid[%d]",ilTran_id);
			break; /*add by dyw, 2015.03.28*/
    	}
    	i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
    }
	if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);  
  }
  /*==== add by lh 20150120 begin ====*/
    swVdebug(2,"Temp[%d]",ilTmp);
    /*mod by cjh at 2015.04.08 one line %d->%ld PSBC_V1.0 */
    swVdebug(2,"tranid[%ld]",ilTran_id);

    if(ilTmp == 1)
    {
	if(swShmP( GET_TRANLOG_BUCKET_SEM_NUM(m) ) == FAIL) return(FAIL);
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ m ];
    if( p_shm_tran_log[j-1].sSwt_tran_log.tran_status != 0 ||
	    p_shm_tran_log[j-1].sSwt_tran_log.tran_id != ilTran_id )
	{
	  if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(m) ) == FAIL) return(FAIL);
      return (SHMNOTFOUND);
	}
	memcpy((char *)psSwt_tran_log, (char *)(&(p_shm_tran_log[j-1].sSwt_tran_log)), 
				sizeof(struct swt_tran_log));
        *cLoc_flag = '0';
    }

    if(ilTmp == 1)
    {
        /* ECHOTEST发起的查询交易不更新流水状态 */
        /* 更新共享内存内的交易流水状态 */
        if ((*cLoc_flag == '0') &&
                (psSwt_tran_log->q_tran_begin != iMBECHOTEST) ) 
        {
            psSwt_tran_log->tran_status = iTRNREVING;
         	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ m ];	
			memcpy((char *)(&(p_shm_tran_log[j-1].sSwt_tran_log)),
				(char *)psSwt_tran_log, sizeof(struct swt_tran_log));
        }     
		if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(m) ) == FAIL) return(FAIL);
        swVdebug(4,"S5565: swShmselect_overtime_min_tranlog_update()找到超时交易，并置状态成功!");
#ifdef DB_SUPPORT  /*support database 2014-12*/  /*need review again*/
        if ((*cLoc_flag == '1') &&
                (psSwt_tran_log->q_tran_begin != iMBECHOTEST) ) 
        {
            psSwt_tran_log->tran_status = iTRNREVING;
            ilRc = swDbupdate_swt_tran_log(ilTran_id, *psSwt_tran_log);
        }
#endif
        return(SUCCESS);
    }
    else
        return(SHMNOTFOUND);
}

#endif


/***************************************************************
 ** 函数名      : swShmupdate_swt_tran_log_end
 ** 功  能      : 更新交易流水记录
 ** 作  者      : cy
 ** 建立日期    : 2004/03/24
 ** 最后修改日期: 2004/03/24
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmupdate_swt_tran_log_end(long lTran_id, struct swt_tran_log sSwt_tran_log,char cLoc_flag)
{
    int i,j;
    short ilQdes,ilQdes_old;

    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    struct shmidx_d * p_shm_idx;

    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ HASH_TRANID(lTran_id) ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ HASH_TRANID(lTran_id) ]);
    p_shm_idx =	psmShmidx_d;
    /*== End ==add by dyw,  2015-0114 ====*/

    if( (cLoc_flag == '0') || (cLoc_flag == 'a') )
    {
        if (swShmP( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL) return(FAIL);

        i = p_idx_tran_log->iSlink;
        while (i)
        {
            if ((p_shm_tran_log[i-1].sSwt_tran_log.tran_id == lTran_id) &&
                    (p_shm_tran_log[i-1].sSwt_tran_log.tran_status != iTRNREVING ))
            {
                ilQdes_old = p_shm_tran_log[i-1].sSwt_tran_log.q_target;
                ilQdes = sSwt_tran_log.q_target;
                memcpy((char *)(&(p_shm_tran_log[i-1].sSwt_tran_log)),
                        (char *)&sSwt_tran_log, sizeof(struct swt_tran_log));
                /* === begin of added by fzj at 2002.02.28 === */
                if ((ilQdes > 0) && (ilQdes != ilQdes_old))
                {
                    for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
                    {
                        if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
                        {
                            smShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
                            if (ilQdes_old <= 0) break;
                        }
                        if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes_old)
                        {
                            smShmbuf_s.psSwt_sys_queue[j].traning_num -= 1;
                            if (ilQdes<=0) break;
                        }
                    }
                }
                /* === end of added by fzj at 2002.02.28 === */
                if(swShmV( GET_TRANLOG_SEM_NUM( lTran_id ) ) == FAIL)
                    return(FAIL);
                else
                    return(SUCCESS);  
            }
            i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV( GET_TRANLOG_SEM_NUM( lTran_id )) == FAIL) return(FAIL);
    }
#ifdef DB_SUPPORT
    if((cLoc_flag == '1') || (cLoc_flag == 'a'))
    {
        swVdebug(4,"S5065: swShmupdate_swt_tran_log select from db");


        ilRc = swDbupdate_swt_tran_log_end(lTran_id, sSwt_tran_log);
        return(ilRc);

    }
#endif
    return (SHMNOTFOUND);
} 



/***************************************************************
 ** 函数名      : swShmselect_swt_tran_log_all
 ** 功  能      : 查找交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2015/01/14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_swt_tran_log_all(struct swt_tran_log *psSwt_tran_log, 
        short *iCount)
{
    int i, k;
    short ilCount = 0;
    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    /*== End ==add by dyw,  2015-0114 ====*/

    for( k =0; k< SHM_TRANLOG_BUCKET_NUM; k++)
    {
        p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
        p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);
        if(swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);

        i = p_idx_tran_log->iSlink;
        while(i)
        {
            if(ilCount >= iMAXTRANLOGREC)
            {
                swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) );
                return(FAIL);
            }
            memcpy((char *)&(psSwt_tran_log[ilCount]), 
                    (char *)(&(p_shm_tran_log[i-1].sSwt_tran_log)), 
                    sizeof(struct swt_tran_log));
            ilCount ++;
            i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
        }
        if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL); 
    }
    *iCount = ilCount;
    swVdebug(2,"ilCount=[%d]",ilCount);

    if((psmShmidx_d->iRecinfile[SFTRANLOGID]) > 0)
    {
#ifdef DB_SUPPORT  /*support database 2014-12*/
        ilRc = swDbselect_swt_tran_log_all(psSwt_tran_log, iCount);
        swVdebug(2,"ilCount2=[%d]",*iCount);
        if(ilRc == FAIL) return(FAIL);
#else

#endif

    }     

    if(*iCount > 0)
        return(SUCCESS);
    else
        return(SHMNOTFOUND);
}

#if 1

	
/***************************************************************
 ** 函数名      : swShmselect_overtime_min_tranlog
 ** 功  能      : 查找超时的最小交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 修改记录：  2015/01/20 liuhuan
 ** 修改记录:  2015/03/28  Deng Yuwei
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_overtime_min_tranlog(long lTime, 
        struct swt_tran_log *psSwt_tran_log,char *pcLoc_flag)
{
    int i,j,ilRc,ilTmp = 0;
    long ilTran_id = lMAXSERNO;
    struct swt_tran_log slSwt_tran_log;
  int k,m;
  struct shm_tran_log* p_shm_tran_log;
  struct shmidx_item_d* p_idx_tran_log;
  struct shmidx_d * p_shm_idx;
  p_shm_idx =	psmShmidx_d;
  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
	p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);  
   	if(swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
    i = p_idx_tran_log->iSlink;
    while(i)
    {
    	if(p_shm_tran_log[i-1].sSwt_tran_log.tran_status == 0 &&
       		p_shm_tran_log[i-1].sSwt_tran_log.tran_overtime < lTime &&
       		p_shm_tran_log[i-1].sSwt_tran_log.tran_overtime != 0 &&
       		p_shm_tran_log[i-1].sSwt_tran_log.tran_id < ilTran_id)
    	{
            if (ilTmp == 0) ilTmp = 1;
            j = i;
 			m = k;
    		ilTran_id = p_shm_tran_log[i-1].sSwt_tran_log.tran_id;
			break;
    	}
    	i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
  	}
  	if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
  }
  if(ilTmp == 1)
  {
		p_shm_tran_log = smShmbuf_d.psShm_tran_log[ m ];
		memcpy((char *)psSwt_tran_log, (char *)(&(p_shm_tran_log[j-1].sSwt_tran_log)), 
	        sizeof(struct swt_tran_log));
    	*pcLoc_flag = '0';
		return SUCCESS;  
  }else
  {
 	return SHMNOTFOUND;
  }
}


/***************************************************************
 ** 函数名      : swShmselect_max_tran_begin
 ** 功  能      : 获取交易流水表中交易时间最大
 ** 作  者      :
 ** 建立日期    : 2001/05/31
 ** 最后修改日期: 2001/05/31
 ** modified record: 2015/01/21 Liu Huan
 ** modified record: 2015/03/29 Deng Yuwei
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
short swShmselect_max_tran_begin(long *lTran_begin)
{
  
  int  i,ilRc,ilTmp = 0;
  long llTmp_begin = 0;
  long llTran_begin = 0;
  int k,m;
  struct shm_tran_log* p_shm_tran_log;
  struct shmidx_item_d* p_idx_tran_log;
  struct shmidx_d * p_shm_idx;
  p_shm_idx =	psmShmidx_d;
  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
	p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);
	if(swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
	i = p_idx_tran_log->iElink;	
    while ( i )
   {
    	if (p_shm_tran_log[i - 1].sSwt_tran_log.tran_begin > llTmp_begin)
    	{
      		ilTmp = 1;
      		llTmp_begin = p_shm_tran_log[i-1].sSwt_tran_log.tran_begin;
    	}
    	i = p_shm_tran_log[i - 1].sLinkinfo.iLlink;
  	}
  
  	if (swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
  }
    
  if (ilTmp == 1)
  {
    *lTran_begin = llTmp_begin;
	return SUCCESS;
  }else
  	return SHMNOTFOUND;
}

/***************************************************************
 ** 函数名      : swShmselect_max_tran_id
 ** 功  能      : 获取交易流水表中交易时间最大的交易流水号
 ** 作  者      :
 ** 建立日期    : 2001/05/31
 ** 最后修改日期: 2001/05/31
 ** 修改记录 : 2015/01/20
 ** 修改记录:  2015/03/29
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
short swShmselect_max_tran_id(long lTran_begin,long *lTran_id)
{
  int i,ilRc,ilTmp = 0;
  long llTran_id_tmp = 0;
  long llTran_id = 0;
  int k,m;
  struct shm_tran_log* p_shm_tran_log;
  struct shmidx_item_d* p_idx_tran_log;
  struct shmidx_d * p_shm_idx;
  p_shm_idx =	psmShmidx_d;
  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
	p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);  
	if(swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
	i = p_idx_tran_log->iElink;
	while(i)
    {
	  if ((p_shm_tran_log[i - 1].sSwt_tran_log.tran_begin == 
         lTran_begin) &&
        (p_shm_tran_log[i - 1].sSwt_tran_log.tran_id > 
         llTran_id_tmp))
        {
          llTran_id_tmp = p_shm_tran_log[i - 1].sSwt_tran_log.tran_id;
          ilTmp = 1;
        }
      i = p_shm_tran_log[i - 1].sLinkinfo.iLlink;
    }
	if (swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
  }
  if (ilTmp == 1)
  {
    *lTran_id = llTran_id_tmp;
    return(SUCCESS);
  }
  else
    return (SHMNOTFOUND);
}
/***************************************************************
 ** 函数名      : swShmselect_swt_tran_log_clear
 ** 功  能      : 查找指定时间以前的交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/09/13
 ** 最后修改日期: 
 ** modified record: 2015/01/20 liu huan
 ** modified reoord: 2015/03/29 Deng Yuwei
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
short swShmselect_swt_tran_log_clear(long lTran_begin, 
        struct swt_tran_log *psSwt_tran_log,char *pcLoc_flag)
{
 int i,ilRc,ilTmp = 0;
  int k,m;
  struct shm_tran_log* p_shm_tran_log;
  struct shmidx_item_d* p_idx_tran_log;
  struct shmidx_d * p_shm_idx;
  p_shm_idx =	psmShmidx_d;
  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
	p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);
	if( swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
	i = p_idx_tran_log->iSlink;
    while (i)
	{
	  if (p_shm_tran_log[i-1].sSwt_tran_log.tran_begin <= lTran_begin)
      {
      	memcpy((char *)psSwt_tran_log, (char *)(&(p_shm_tran_log[i-1].sSwt_tran_log))
				, sizeof(struct swt_tran_log));
      	ilTmp = 1;      
      	break;
      }
      i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
    }
	if ( swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
	if ( ilTmp == 1 ) break;  
  }
  if (ilTmp == 1)
  {
    *pcLoc_flag = '0';
    return(SUCCESS);
  }
  else
    return (SHMNOTFOUND);
}




/***************************************************************
 ** 函数名      : swShmfilefind_tran_log
 ** 功  能      : 判断是否存在指定resq_file或resp_file的记录 
 ** 作  者      :
 ** 建立日期    : 2001/05/31
 ** 最后修改日期: 2001/05/31
 ** 修改记录: 2015/01/20  Liu huan
 ** 修改记录: 2015/03/29 Deng Yuwei 
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    :
 ** 返回值      : 1:存在 0:不存在
 ***************************************************************/
int swShmfilefind_tran_log(char * aFilename)
{
  int i,ilTmp = FALSE;
  short ilRc;
  int k,m;
  struct shm_tran_log* p_shm_tran_log;
  struct shmidx_item_d* p_idx_tran_log;
  struct shmidx_d * p_shm_idx;
  p_shm_idx =	psmShmidx_d;
  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
	p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);
	if(swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
	i = p_idx_tran_log->iSlink;
    while(i)
    {
      if((strcmp(p_shm_tran_log[i-1].sSwt_tran_log.resq_file,
        aFilename) == 0) || 
        (strcmp(p_shm_tran_log[i-1].sSwt_tran_log.resp_file,
        aFilename) == 0))
      {
        ilTmp = TRUE;
        break;
      }
      i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
    }
    if ( swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL ) return(FAIL);
    if( ilTmp == TRUE ) break;
  }	  
  return(ilTmp);

} 


/* =================== file_swt_tran_log ====================== */

#if 0
/***************************************************************
 ** 函数名      : swShmf2shm_swt_tran_log
 ** 功  能      : 从文件移一条交易流水到共享内存(删除时用)
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : iPos    在共享内存中的位置
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
short swShmf2shm_swt_tran_log(short iPos)
{
    short ilRc;
    short ilSize;
    short ilTmp = FALSE;
    short ilQorg,ilQdes;
    long  llOffset = 0;
    long  llOffset_tmp = -1;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
        swVdebug(0,"S5120: [错误/函数调用] swShmopenfile()函数,返回码=%d, 打开文件出错",ilRc);
        return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
        ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
        if (ilRc < 0)
        {
            swVdebug(0,"S5125: [错误/函数调用] swShmgetfile()函数,返回码=%d,读文件出错",ilRc);
            swShmclosefile(SFTRANLOGID, fp);
            return (FAIL);
        }
        llOffset = llOffset + ilSize;
        if (ilRc == SUCCESS)
        {
            if (alBuf[0] == '1')
            {
                memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);

                ilQorg = slSwt_tran_log.q_tran_begin;
                ilQdes = slSwt_tran_log.q_target;

                llOffset_tmp = llOffset - ilSize;

                ilTmp = TRUE;
                alBuf[0] = '0';

                ilRc = swShmputfile(fp, llOffset_tmp, ilSize, alBuf);
                if (ilRc)
                {
                    swVdebug(0,"S5130: [错误/函数调用] swShmputfile()函数,返回码=%d,写文件出错",ilRc);
                    swShmclosefile(SFTRANLOGID, fp);
                    return (FAIL);
                }    
                break;
            }  
        }  
        if (ilRc == SHMFILEEND)  break;
    }

    if (ilTmp)
    {
        psmShmidx_d->iRecinfile[SFTRANLOGID] --;

        memcpy((char *)(&(smShmbuf_d.psShm_tran_log[iPos].sSwt_tran_log)),
                (char *)&slSwt_tran_log, ilSize - 1);

        swShmclosefile(SFTRANLOGID, fp);
        return (SUCCESS);
    }
    else
    {
        swShmclosefile(SFTRANLOGID, fp);
        return (SHMNOTFOUND);
    }    
}

#endif

/***************************************************************
 ** 函数名      : swShmresetporttran
 ** 功  能      : 
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 修改记录 : 2015/01/20 Liu huan
 ** 修改记录 : 2015/01/20 Deng Yuwei
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : iPos    在共享内存中的位置
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/


int swShmresetporttran()
{
    int i,j,ilRc;
    short ilSize,ilQorg,ilQdes;
    long  llOffset = 0;
    char  alBuf[iFLDVALUELEN];
    FILE  *fp;
    struct swt_tran_log slSwt_tran_log;

  int k,m;
  struct shm_tran_log* p_shm_tran_log;
  struct shmidx_item_d* p_idx_tran_log;
  struct shmidx_d * p_shm_idx;
  
  for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
  {
    smShmbuf_s.psSwt_sys_queue[j].tranbegin_num = 0;
    smShmbuf_s.psSwt_sys_queue[j].traning_num = 0;
  }
  
  p_shm_idx =	psmShmidx_d;
  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
	p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);
	if( swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
	i = p_idx_tran_log->iSlink;
	while (i)
    {
      ilQorg = p_shm_tran_log[i-1].sSwt_tran_log.q_tran_begin;
      ilQdes = p_shm_tran_log[i-1].sSwt_tran_log.q_target;
    
      for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
      {
        if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQorg)
        {
          smShmbuf_s.psSwt_sys_queue[j].tranbegin_num += 1;
          if (ilQdes <= 0) break;
        }
        if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
        {
          smShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
          if (ilQorg <= 0) break;
        }
      }
      i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
    }
  }
  /*==== add by lh 20150121 end ====*/  
  /* 从文件中查找 */
  if (psmShmidx_d->iRecinfile[SFTRANLOGID] > 0)
  {
    if ((ilRc = swShmopenfile(SFTRANLOGID, &fp)) != SUCCESS)
    {
      swVdebug(0,"S5565: [错误/函数调用] swShmopenfile()函数,返回码=%d, 打开文件出错",ilRc);
	  /*==== delete by lh 20150121 begin ====
      swShmV(iSEMNUM_TRANLOG);
      ==== delete by lh 20150121 end ====*/
      /*==== add by lh 20150121 begin ====*/	  
      for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
      {
        swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) );
      }
      /*==== add by lh 20150121 end ====*/     
	  return (FAIL);
    }
    ilSize = sizeof(struct swt_tran_log) + 1;
    for(;;)
    {
      ilRc = swShmgetfile(fp, OFFSETNULL, ilSize, alBuf);
      if (ilRc < 0)
      {
        swVdebug(0,"S5570: [错误/函数调用] swShmgetfile()函数,返回码=%d,读文件出错",ilRc);
        swShmclosefile(SFTRANLOGID, fp);
	  /*==== delete by lh 20150121 begin ====		
        swShmV(iSEMNUM_TRANLOG);
      ==== delete by lh 20150121 end ====*/
      /*==== add by lh 20150121 begin ====*/	  
      for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
      {
        swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) );
      }
      /*==== add by lh 20150121 end ====*/ 	  
		return (FAIL);
      }
      llOffset = llOffset + ilSize;
      if (ilRc == SUCCESS)
      {
        if (alBuf[0] == '1')
        {    
          memcpy((char *)&slSwt_tran_log, alBuf + 1, ilSize - 1);
          ilQorg = slSwt_tran_log.q_tran_begin;
          ilQdes = slSwt_tran_log.q_target;
          for (j=0; j<psmShmidx_s->sIdx_sys_queue.iCount; j++)
          {
            if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQorg)
            {
              smShmbuf_s.psSwt_sys_queue[j].tranbegin_num += 1;
              if (ilQdes <= 0) break;
            }
            if (smShmbuf_s.psSwt_sys_queue[j].q_id == ilQdes)
            {
              smShmbuf_s.psSwt_sys_queue[j].traning_num += 1;
              if (ilQorg <= 0) break;
            }
          } 
        }  
      }  
      if (ilRc == SHMFILEEND)  break;
    }
    swShmclosefile(SFTRANLOGID, fp);
  }

  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
    if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
  }
  return(SUCCESS);

}

/* === end of added by fzj at 2002.02.28 === */

/***************************************************************
 ** 函数名      : swShmselect_qid_revkey_tranlog
 ** 功  能      : 查找交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_qid_revkey_tranlog(int iQid, char *aKey, struct swt_tran_log *psSwt_tran_log)
{
    int i,ilRc;
  int k,m;
  struct shm_tran_log* p_shm_tran_log;
  struct shmidx_item_d* p_idx_tran_log;
  struct shmidx_d * p_shm_idx;
  p_shm_idx =	psmShmidx_d;
  for( k=0 ; k < SHM_TRANLOG_BUCKET_NUM ; k++ )
  {
	p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
	p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]); 
	if( swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL ) return(FAIL);
	i = p_idx_tran_log->iSlink;
    while (i)
    {
      if (p_shm_tran_log[i-1].sSwt_tran_log.q_tran_begin == iQid
	  && strcmp(p_shm_tran_log[i-1].sSwt_tran_log.rev_key,aKey) == 0)
        {
          memcpy((char *)psSwt_tran_log, (char *)(&(p_shm_tran_log[i-1].sSwt_tran_log))
		    , sizeof(struct swt_tran_log));
       
          if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL)
            return(FAIL);
          else
            return(SUCCESS);      
        }
      i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
    }
    if (swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);
  }   
  return (SHMNOTFOUND);
}

#endif




/***************************************************************
 ** 函数名      : swShmselect_qid_msghdkey_tranlog
 ** 功  能      : 查找交易流水记录
 ** 作  者      : 
 ** 建立日期    : 2001/05/26
 ** 最后修改日期: 2001/05/26
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
int swShmselect_qid_msghdkey_tranlog(int iQid, char *aKey, struct swt_tran_log *psSwt_tran_log)
{
    int i,ilRc, k;
    /*== Begin ==add by dyw,  2015-0114 ====*/
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    /*== End ==add by dyw,  2015-0114 ====*/
    for( k =0; k< SHM_TRANLOG_BUCKET_NUM; k++)
    {
        p_shm_tran_log = smShmbuf_d.psShm_tran_log[ k ];
        p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ k ]);

        if ( swShmP( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL) return(FAIL);

        i = p_idx_tran_log->iSlink;
        while (i)
        {
            if ( p_shm_tran_log[i-1].sSwt_tran_log.q_target == iQid) 
            {
                if (strcmp( p_shm_tran_log[i-1].sSwt_tran_log.msghdkey,aKey) == 0)
                {
                    memcpy((char *)psSwt_tran_log, (char *)(&(p_shm_tran_log
                                    [i-1].sSwt_tran_log)), sizeof(struct swt_tran_log));
                    if(swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k) ) == FAIL)
                        return(FAIL);
                    else
                        return(SUCCESS);    
                }
            }
            i = p_shm_tran_log[i-1].sLinkinfo.iRlink;
        }
        if (swShmV( GET_TRANLOG_BUCKET_SEM_NUM(k)) == FAIL) return(FAIL);
    }
    return (SHMNOTFOUND);
}


/***************************************************************
 ** 函数名      : sw_new_tran_log_link
 ** 功  能      : 分配交易流水空闲节点
 ** 作  者      : 
 ** 建立日期    : 2015/01/14
 ** 最后修改日期: 2015/01/14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
short sw_malloc_tran_log_link(int bucket,unsigned int *free_node)
{

    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ bucket ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ bucket ]);

    if( p_idx_tran_log->iFlink <= 0)
    {
        p_idx_tran_log->used_log++;
        return NO_FREE_TRAN_LOG;
    }
    else
    {
        (*free_node) = p_idx_tran_log->iFlink;
        p_idx_tran_log->used_log++;
        if(p_idx_tran_log->max_used_log < p_idx_tran_log->used_log )
        {
            p_idx_tran_log->max_used_log =  p_idx_tran_log->used_log; 
        }
    }

    // assert(p_shm_tran_log[(*free_node)-1].sLinkinfo.cTag == 0 );
    p_shm_tran_log[(*free_node)-1].sLinkinfo.cTag = 1;
    p_idx_tran_log->iFlink = p_shm_tran_log[(*free_node)-1].sLinkinfo.iRlink;
    return SUCCESS;
}



/***************************************************************
 ** 函数名      : sw_free_tran_log_link
 ** 功  能      : 释放交易流水节点
 ** 作  者      : 
 ** 建立日期    : 2015/01/14
 ** 最后修改日期: 2015/01/14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/

short sw_free_tran_log_link(int bucket, unsigned int link)
{
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ bucket ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ bucket]);

    p_shm_tran_log[link-1].sLinkinfo.iRlink 
        = p_idx_tran_log->iFlink;
    p_shm_tran_log[link-1].sLinkinfo.cTag = 0;
    p_idx_tran_log->iFlink = link;
    p_idx_tran_log->used_log--;
    return SUCCESS;
}


/***************************************************************
 ** 函数名      : sw_show_tran_log_free_list
 ** 功  能      : 显示交易流水空闲节点
 ** 作  者      : 
 ** 建立日期    : 2015/01/14
 ** 最后修改日期: 2015/01/14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/

short  sw_show_tran_log_free_list(int bucket, int num)
{
    unsigned int i, p;
    struct shm_tran_log* p_shm_tran_log;
    struct shmidx_item_d* p_idx_tran_log;
    p_shm_tran_log = smShmbuf_d.psShm_tran_log[ bucket ];
    p_idx_tran_log =  &(psmShmidx_d->sIdx_tran_log[ bucket ]);

    p_shm_tran_log  = (struct shm_tran_log *)
        ( (char*)psmShmidx_d + p_idx_tran_log->lOffset);

    smShmbuf_d.psShm_proc_log = (struct shm_proc_log *)
        ( (char*)psmShmidx_d + psmShmidx_d->sIdx_proc_log.lOffset);

    for( i= 0, p = p_idx_tran_log->iFlink; p != 0 && i <num; i++ )
    {

        printf("%dthfree tran link = %d\n",i,  p ) ;
        p = p_shm_tran_log[p-1].sLinkinfo.iRlink;
    }
    return SUCCESS;
}


/***************************************************************
 ** 函数名      : sw_show_proc_log_free_list
 ** 功  能      : 显示处理流水空闲节点
 ** 作  者      : 
 ** 建立日期    : 2015/01/14
 ** 最后修改日期: 2015/01/14
 ** 调用其它函数:
 ** 全局变量    :
 ** 参数含义    : 
 ** 返回值      : 0:成功 / -1:失败
 ***************************************************************/
short  sw_show_proc_log_free_list(int num)
{	
    unsigned int i, p;
    for( i= 0, p = psmShmidx_d->sIdx_proc_log.iFlink ; p != 0&& i <num; i++ )
    {
        printf("%dth free proc link = %d\n",i, p );
        p = smShmbuf_d.psShm_proc_log[p-1].sLinkinfo.iRlink;
    }
    return SUCCESS;
}

