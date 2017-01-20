#ifndef SWITCH_H
#define SWITCH_H
/* UNIX系统定义 */
#include <stdio.h>
#include <string.h>
#include <ctype.h>
/*"ifdef" modified by qh 20070416*/
#ifdef OS_LINUX
#include <stdarg.h>
#endif
#ifdef OS_AIX
#include <varargs.h>
#endif
#ifdef OS_HPUX
#include <stdarg.h>
#endif
#ifdef OS_SCO
#include <stdarg.h>
#endif
#ifdef OS_SOLARIS
#include <stdarg.h>
#endif
#include <malloc.h>
#include <memory.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/signal.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/timeb.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>

/* switch定义 */
#include "swConstant.h"
#include "swStruct.h"
#include "swGlobal.h"
#include "swPubfun.h"
#include "swlog.h"

#define WITHOUT_SHM_FRESH 1

typedef unsigned int  sw_u_size_t;

#endif
