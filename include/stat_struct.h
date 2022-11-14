/**
 * @file    stat_struct.h
 * @brief   instance stat API & library : stat struct type
 *
 * @author  cbpark, chhwang
 * @version $Id$
 */

#ifndef _STAT_STRUCT_H
#define _STAT_STRCUT_H

#ifdef _WIN32
#   include <Winsock2.h>
#else
#   include <sys/time.h>
#endif

#if defined(_WIN32) && defined(_TB_STAT_CLIENT)
typedef struct timeval_for_win {
    time_t tv_sec;
    long tv_usec;
} timeval_for_win;
#endif

#ifdef _WIN32
/* Windows */
typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;

typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;

typedef DWORD pid_t;
#ifdef _TB_STAT_CLIENT
#   if __WORDSIZE == 64
#   define __PRI64_PREFIX  "l"
#   define __PRIPTR_PREFIX "l"
#   else
#   define __PRI64_PREFIX  "ll"
#   define __PRIPTR_PREFIX
#   endif
#   define PRIu32 "u"
#   define PRIu64 __PRI64_PREFIX "u"
#   define PRId64 __PRI64_PREFIX "d"
#endif

#else
/* UNIX */
#include <inttypes.h>
#endif

/* script로 생성한 client용 헤더 파일 */
/* -------------------------------------------------------- */
#include "stat_type.h"

#define SESS_IPADDR_MAXLEN 46
#define TB_MAXLEN_PROGNAME 30
/* -------------------------------------------------------- */

/* gen_j_cnt_list.pl과 맞춰줘야 한다 */
#ifndef _STAT_CLASS_E
#define _STAT_CLASS_E
typedef enum stat_class_e {
    /* j_cnt */
    STAT_CLASS_USER     ,
    STAT_CLASS_REDO     ,
    STAT_CLASS_CACHE    ,
    STAT_CLASS_DATA     ,
    STAT_CLASS_SQL      ,
    STAT_CLASS_TAC      ,
    STAT_CLASS_SSVR     ,
    STAT_CLASS_DEBUG    ,
    STAT_CLASS_TAS      ,
    STAT_CLASS_IOD      ,
    STAT_CLASS_FLASHBACK,

    /* oracle event classes */
    STAT_CLASS_APPLICATION, /* wlock, commit */
    STAT_CLASS_CLUSTER    ,
    STAT_CLASS_CONCURRENCY, /* spinlock */
    STAT_CLASS_IDLE       ,
    STAT_CLASS_NETWORK    ,
    STAT_CLASS_SYSTEM_IO  ,
    STAT_CLASS_USER_IO    ,
    STAT_CLASS_2PC        ,
    STAT_CLASS_OTHER      ,
    STAT_CLASS_MAX
} stat_class_t;
#endif

#ifndef _TICK_T
#define _TICK_T
typedef uint32_t tick_t;
#endif

#ifndef _TB_TIMEVAL_T

#define _TB_TIMEVAL_T
#if !defined(_USE_GETTIMEOFDAY) &&                                             \
    (defined(_LINUX_X86_32) || defined(_FREEBSD_32) ||                         \
     defined(_LINUX_X86_64) || defined(_FREEBSD_64) ||                         \
     defined(_LINUX_IA64_64)||                                                 \
     defined(_WIN32) ||                                                        \
     defined(_OS_SOLARIS) ||                                                   \
     defined(_HP_IA64_64) ||                                                   \
     defined(_LINUX_PPC_64) || defined(_LINUX_PPC_32) ||                       \
     defined(_LINUX_PPCLE_64) ||                                               \
     defined(_LINUX_S390X_64) || defined(_ARM64) || defined(_SW_64))
typedef uint64_t tb_timeval_t;
#elif !defined(_USE_GETTIMEOFDAY) && defined(_OS_AIX)
typedef timebasestruct_t tb_timeval_t;
#elif !defined(_USE_GETTIMEOFDAY) && defined(_HP_IA64_32)
typedef hrtime_t tb_timeval_t;
#else
typedef struct timeval tb_timeval_t;
#endif

#endif  /* _TB_TIMEVAL_T*/

#ifndef _PPID_T
#define _PPID_T
typedef int64_t ppid_t;
#endif  /* _PPID_T */

#ifndef _MT_SIZE_T
#define _MT_SIZE_T
typedef int64_t mt_size_t;
#endif  /* _MT_SIZE_T */

#define MAXLEN_SQLID 128

#ifndef _WE_STAT_T
#define _WE_STAT_T
struct we_stat_s {
    uint64_t total_num;     /* # of total requeset */
    uint64_t wait_num;      /* # of wait state */
    uint64_t timeout_num;
    uint64_t wait_time;     /* wait tb timeval(not usec) */
    uint64_t max_wait;      /* max wait tb timeval(not usec) */
};
typedef struct we_stat_s we_stat_t;
#endif

#ifndef _JC_STAT_T
#define _JC_STAT_T
struct jc_stat_s {
    uint64_t total_num;     /* jc_marker / jc_start 가 불린 총 회수 */
    uint64_t size;          /* jc_marker와 함께 기록되는 값 */
    uint64_t used_time;     /* jc_start~jc_end 사이의 총 시간 (tb timeval) */
    uint64_t max_used;      /* jc_start~jc_end 사이의 max time (tb timeval) */
    tb_timeval_t tv;        /* 시간 측정을 위한 변수 */
#ifdef _JC_STK
    int16_t idx;
#endif
};
typedef struct jc_stat_s jc_stat_t;
#endif

#ifndef _THR_SPIN_STAT_T
#define _THR_SPIN_STAT_T
typedef struct thr_spin_stat_s {
    uint64_t fast_cnt;
    uint64_t spin_cnt;
    uint64_t sleep_cnt;
    uint64_t sleeps_cnt;
    uint64_t try_get_cnt;
    uint64_t try_fail_cnt;
    uint64_t wait_time;
    uint64_t max_wait;
} thr_spin_stat_t;
#endif

#ifndef _STAT_HISTORY_ELEM_T
#define _STAT_HISTORY_ELEM_T
struct stat_history_elem_s {
    int stat_history_id;
#ifdef _WIN32
    struct timeval_for_win start_time;
#else
    struct timeval start_time;
#endif
    uint64_t elapsed;

    int rows_processed;
    uint64_t hashval;
    ppid_t ppid;
    const char *org_stmt;       /* shared memory 주소 */
    int org_stmt_len;           /* org_stmt 길이 */
    const char *prev_stmt;      /* shared memory 주소 */
    int prev_stmt_len;          /* prev_stmt 길이 */
    uint64_t prev_hashval;
    int wsess_id;
    unsigned int sess_serial_no;
    char sess_username[MAXLEN_SQLID + 1];
    int pe_coordinator;

    char ipaddr[SESS_IPADDR_MAXLEN + 1];
    pp_sqltext_command_type_t command_type;
    tb_date_t logon_time;
    mt_size_t ti_sess_pga_alloc_size;
    char progname[TB_MAXLEN_PROGNAME + 1];
    pid_t ti_pid;
    char module_name[64 + 1];
    char action_name[64 + 1];
    char client_info_name[64 + 1];
    int client_pid;

    jc_stat_t *si_jc_stat_start;            /* 시작 jcnt에 대한 shm 주소 */
    jc_stat_t *si_jc_stat_end;              /* 현재 jcnt에 대한 shm 주소 */
    we_stat_t *si_wait_event_stat_start;    /* 시작 waitevent에 대한 shm 주소 */
    we_stat_t *si_wait_event_stat_end;      /* 현재 waitevent에 대한 shm 주소 */
    thr_spin_stat_t *ti_spin_stat_start;    /* 시작 spinstat에 대한 shm 주소 */
    thr_spin_stat_t *ti_spin_stat_end;      /* 현재 spinstat에 대한 shm 주소 */

    unsigned char is_written;
};
typedef struct stat_history_elem_s stat_history_elem_t;
#endif  /* _STAT_HISTORY_ELEM_T */

#ifndef _STAT_HISTORY_QUEUE_T
#define _STAT_HISTORY_QUEUE_T
struct stat_history_queue_s {
    int cur_id;  /* circular queue 의 현재 index; 마지막으로 써진 index */
    int num_elem; /* # of elems in circular queue  */
    int queue_max_size; /* IPARAM((SQL_STAT_HISTORY_QSIZE) */

    stat_history_elem_t *stat_histories; /* stat history element 들의 배열 */
};
typedef struct stat_history_queue_s stat_history_queue_t;
#endif  /* _STAT_HISTORY_QUEUE_T */

#ifndef _SLOG_HISTORY_ELEM_T
#define _SLOG_HISTORY_ELEM_T
struct slog_history_elem_s {
    int slog_history_id;
    const char *str;
    unsigned char is_written;
};
typedef struct slog_history_elem_s slog_history_elem_t;
#endif  /* _SLOG_HISTORY_ELEM_T */

#ifndef _SLOG_HISTORY_QUEUE_T
#define _SLOG_HISTORY_QUEUE_T
struct slog_history_queue_s {
    int cur_id;  /* circular queue 의 현재 index; 마지막으로 써진 index */
    int num_elem; /* # of elems in circular queue  */
    int queue_max_size; /* IPARAM((SLOG_HISTORY_QSIZE) */

    slog_history_elem_t *slog_histories; /* system log history element 들의 배열 */
};
typedef struct slog_history_queue_s slog_history_queue_t;
#endif  /* _SLOG_HISTORY_QUEUE_T */


#endif /* no _STAT_STRUCT_H */
