#ifndef SWLOG_H
#define SWLOG_H

/*封装swDebugfml, swDebughex, swDebugmsghead，添加日志级别 add by cjh 20150327,PSBC_V1.0*/

char agDebugfile[64];
char cgDebug;
char procName[20];

void swLogInit(char *,char *);

#define swVDebugfml(iDebug,msgpack) \
	do{ if ( cgDebug >= iDebug ) swDebugfml(msgpack); \
	}while(0);

#define swVDebughex(iDebug,buf, buflen) \
        do{ if ( cgDebug >= iDebug )swDebughex(buf, buflen); \
        }while(0);

#define swVDebugmsghead(iDebug,bufhead) \
        do{ if ( cgDebug >= iDebug ) swDebugmsghead(bufhead); \
        }while(0);

/* add by gengling at 2015.03.31 one line PSBC_V1.0 */
#ifdef SW_THREAD_LOG_SUPPORT
#define swVdebug(level, ...) _swVdebug_r(level, __FILE__, __LINE__, __VA_ARGS__)

#define SWLOGINIT(proc_name, sys_log) do{\
    swLogInit(proc_name, sys_log);\
    sw_thread_log_init();\
    sw_thread_log_alloc(); \
    sw_thread_log_set_attr(agDebugfile, cgDebug);\
    }while(0);
#else
#define swVdebug(level,...) _swVdebug(level, __FILE__, __LINE__, __VA_ARGS__)
#endif
#define gettid() syscall(__NR_gettid)

#endif
