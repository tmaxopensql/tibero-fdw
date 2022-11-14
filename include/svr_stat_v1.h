/**
 * @file    svr_stat_v1.h
 * @brief   instance stat API & library : stat API
 *
 * @author  cbpark, chhwang
 * @version $Id$
 */

#ifndef _TB_STAT_V1_H
#define _TB_STAT_V1_H

/* if used building libtbstat */
#ifdef _BUILD_LIBTBSTAT
#include "thr_util.h"
#include "sess.h"
#include "shm.h"
#include "j_cnt_type.h"
#include "wait_event.h"
#include "j_cnt.h"
#include "tbctl_msg.h"
#include "list.h"
#include "wlock.h"
#include "wlist.h"
/* if used in client module */
#else
#   ifndef _TB_STAT_CLIENT
#   define _TB_STAT_CLIENT
#   endif
#include "stat_struct_v1.h"
#include "svrstat_type_v1.h"

#if HAVE_TID_T
#include <sys/types.h>
typedef tid_t sessid_t;
#else
typedef int sessid_t;
#endif

#endif

#define MAX_DESC_LEN 60

#ifndef _PPID_T
#define _PPID_T
typedef int64_t ppid_t;
#endif

#ifdef _WIN32
#define localtime_r(time, tm) localtime_s((tm), (time))
#ifdef _BUILD_LIBTBSTAT
extern __declspec(dllexport) size_t shm_offset;  /* tbsvr shm 주소 + shm_offset = 나의 shm 주소 */
#else
extern __declspec(dllimport) size_t shm_offset;  /* tbsvr shm 주소 + shm_offset = 나의 shm 주소 */
#endif
#else
size_t shm_offset;
#endif

/* ============================================================================*/
/* -------------------------- tbstat err codes ------------------------------- */
/* ============================================================================*/

/* wlock list max iteration count */
#ifndef MAX_WLOCK_CHECK_COUNT
#define MAX_WLOCK_CHECK_COUNT               (10000)
#endif

/* error code base and last error code */
#define TBSTAT_ERR_BASE                     (-1000)
#define TBSTAT_ERR_LAST                     (TBSTAT_ERR_BASE - 1000)

/* success error code */
#define TBSTAT_ERR_SUCCESS                  (0)

/* common error codes */
#define TBSTAT_ERR_TBSTAT_INIT_REQUIRED     (TBSTAT_ERR_BASE -    1)
#define TBSTAT_ERR_ENVVAR_NOT_SET           (TBSTAT_ERR_BASE -    2)
#define TBSTAT_ERR_CANT_LOAD_PROC_INFO      (TBSTAT_ERR_BASE -    3)
#define TBSTAT_ERR_INVALID_SESSID           (TBSTAT_ERR_BASE -    4)
#define TBSTAT_ERR_INVALID_ELEM_ID          (TBSTAT_ERR_BASE -    5)
#define TBSTAT_ERR_SHM_ATTACH_FAILED        (TBSTAT_ERR_BASE -    6)
#define TBSTAT_ERR_INVALID_SHM_ADDR         (TBSTAT_ERR_BASE -    7)
#define TBSTAT_ERR_NO_HISTORY               (TBSTAT_ERR_BASE -    8)

/* macro */
#define ADD_OFFSET(x)   ((char *)(x) + shm_offset)

/* This has been deprecated. */
/* wlock list traverse */
#define WLOCK_LIST_START(pos, head, offset)                                                 \
    do {                                                                                    \
        int __tbstat_i = 0;                                                                 \
        list_t *iter = (list_t *)head;                                                      \
        if (!iter) break;                                                                   \
        for ( pos = extract_wlock_info(ADD_OFFSET(iter->next) - offset);                    \
              ADD_OFFSET(iter->next) != (char *)head && __tbstat_i < MAX_WLOCK_CHECK_COUNT; \
              iter = (list_t *)ADD_OFFSET(iter->next),                                      \
              pos = extract_wlock_info(ADD_OFFSET(iter->next) - offset),                    \
              __tbstat_i++ ) {                                                              \
              if (!tbstat_check_wlock_type(pos.type)) break;                                \

/* WLOCK_LIST_START - WLOCK_LIST_END pair로 써야 함 */
#define WLOCK_LIST_END()                                                                    \
        }} while (0)

/* wlock의 owner, converter, waiter traverse */
#define WLOCK_ELEM_LIST_START(pos, head, offset)                                            \
    do {                                                                                    \
        int __tbstat_j = 0;                                                                 \
        list_t *iter = (list_t *)head;                                                      \
        if (!iter) break;                                                                   \
        for ( pos = extract_wlock_elem_info(ADD_OFFSET(iter->next) - offset);               \
              ADD_OFFSET(iter->next) != (char *)head && __tbstat_j < MAX_WLOCK_CHECK_COUNT; \
              iter = (list_t *)ADD_OFFSET(iter->next),                                      \
              pos = extract_wlock_elem_info(ADD_OFFSET(iter->next) - offset),               \
              __tbstat_j++ ) {                                                              \
              if (!tbstat_check_wlock_elem_mode(pos.mode)) break;                           \

/* WLOCK_ELEM_LIST_START - WLOCK_ELEM_LIST_END pair로 써야 함 */
#define WLOCK_ELEM_LIST_END()                                                               \
         }} while (0)

/* type 구조체의 field offset */
#define OFFSETOF(type, field)    ((size_t)(&((type *)0)->field))



/* ============================================================================*/
/* -------------------------- tbstat Data Structures ------------------------- */
/* ============================================================================*/

/* ---------------------------------------------------------------------
 * jc_info_t:
 * j count description table 의 한 row를 구성하는 구조체 *
 * --------------------------------------------------------------------*/

struct jc_info_s {
    int jcid;                       /* stat 고유 ID */

    int jcclass;                    /* class */
    char desc[MAX_DESC_LEN + 1];        /* number description */
    char size_desc[MAX_DESC_LEN + 1];   /* size description */
    char time_desc[MAX_DESC_LEN + 1];   /* time description */

    /* bool type */
    unsigned char is_sysstat;       /* v$systat에 보이는지 여부 */
    unsigned char is_we;            /* wait event로도 보이는지 여부 */
};
typedef struct jc_info_s jc_info_t ;

/* ---------------------------------------------------------------------
 * we_info_t:
 * wait event description table 의 한 row를 구성하는 구조체 *
 * --------------------------------------------------------------------*/
struct we_info_s {
    int weid;                   /* wait event 고유 ID */

    int weclass;                /* class */
    char desc[MAX_DESC_LEN + 1];/* description */
};
typedef struct we_info_s we_info_t;

/* ---------------------------------------------------------------------
 * wl_info_t:
 * wlock description table 의 한 row를 구성하는 구조체 *
 * --------------------------------------------------------------------*/
struct wl_info_s {
    int wlid;                   /* wlock 고유 ID */
    char desc[MAX_DESC_LEN + 1];/* description */
};
typedef struct wl_info_s wl_info_t;

/*========================================================================
 * session info 에 관련된 구조체들: session_info_t
 * ======================================================================*/
struct session_info_s {
    unsigned int *serial_no;
    unsigned int *audsid;
    unsigned int *user_id;
    char *user_name;
    char *ipaddr;
    char *txinfo;
    pp_sqltext_command_type_t *command_type;
    volatile sess_status_t *status;
    unsigned int *schema_id;
    char *schema_name;
    thr_type_t *type;
    ppid_t *ppid;
    ppid_t *prev_ppid;
    const char **curr_sql;
    const char **prev_sql;
    tick_t *sql_start_tick;
    tb_date_t *logon_time;
    volatile thr_stat_t *thr_status;
    char *wlock_elem;
    unsigned int *wait_event;
    volatile tick_t *wait_start_time;
    mt_size_t *pga_alloc_size;
    unsigned char *sqltrc_psm;
    unsigned char *sqltrc_iparam;
    unsigned char *pdml_enabled;
    sessid_t *qc_sessid;
    char *prog_name;
    int *client_pid;
    pid_t *pid;
    int *wthr_id;
    char *osuser;
    char *machine;
    char *terminal;
    char *module;
    char *action;
    char *client_info;
    char *client_identifier;
};
typedef struct session_info_s session_info_t;

/*========================================================================
 * sesstat info 에 관련된 구조체들: sesstat_info_t
 * ======================================================================*/
struct sesstat_info_s {
    jc_stat_t *stat_sess_start;     /* session 시작할 때 스냅샷 */
    jc_stat_t *stat_current;        /* 현재값 */
};
typedef struct sesstat_info_s sesstat_info_t;

/*========================================================================
 * session_event info 에 관련된 구조체들: session_event_info_t
 * ======================================================================*/
struct session_event_info_s {
    we_stat_t *event_sess_start;    /* session 시작할 때 스냅샷 */
    we_stat_t *event_current;       /* 현재값 */
};
typedef struct session_event_info_s session_event_info_t;

/*========================================================================
 * wlock info 에 관련된 구조체들: wlock_info_t
 * ======================================================================*/
struct wlock_info_s {
    unsigned int type;
    int id1, id2;
    int bucket_idx;
    list_t link;
    list_t *owners;
    list_t *converters;
    list_t *waiters;
};
typedef struct wlock_info_s wlock_info_t;

struct wlock_elem_info_s {
    unsigned int id;
    unsigned int mode;
};
typedef struct wlock_elem_info_s wlock_elem_info_t;

 /*========================================================================
  * struct tbstat_info_t:
  * tbstat_init()시에 받아가는 system wide 한
  * tbstat 기능 전반에 걸쳐 필요한 정보들
  * ex. (wthr갯수, jc description, we descriptions)
  * ======================================================================*/
struct tbstat_info_s {
    int thr_cnt;
    int sess_cnt;
    /* wsess 는 wsessid_start부터 wsessid_end까지 순차적으로 존재
       wsess 갯수 = wsessid_end - wsessid_start + 1 */
    int wsessid_start;          /* working session 의 ID 첫 번호 */
    int wsessid_end;            /* working session 의 ID 끝 번호 */

    int num_jc;                 /* jc table 목록의 갯수 */
    int num_we;                 /* Wait event table 목록의 갯수 */
    int queue_max_size;         /* IPARAM((SQL_STAT_HISTORY_QSIZE) */
    size_t shm_offset;          /* tbsvr shm 주소 + shm_offset = 나의 shm 주소 */
    tick_t *system_tick;        /* system_tick 주소 */

    stat_history_queue_t **q_ptr_list;
    slog_history_queue_t **slog_q_ptr_list;

    jc_info_t *jc_info_list;    /* jc description table */
    we_info_t *we_info_list;    /* wait event description table */
    wl_info_t *wl_info_list;    /* wlock description table */

    size_t session_info_size;
    size_t thr_info_size;
    session_info_t session_info;        /* v$session 관련 정보의 shm address */
    sesstat_info_t sesstat_info;        /* v$sesstat 관련 정보의 shm address */
    session_event_info_t session_event_info;  /* v$session_event 관련 정보의 shm address */

    size_t txinfo_ublkcnt_offset;       /* OFFSETOF(txinfo_t, ublkcnt) */
    size_t txinfo_ureccnt_offset;       /* OFFSETOF(txinfo_t, ureccnt) */

    int num_wlock_bucket;               /* WLOCK_BUCKET_CNT */
    list_t *wlock_buckets;              /* wlock hash bucket의 헤더 */
    size_t wlock_link_offset;           /* OFFSETOF(wlock_t, link) */
    size_t wlock_owners_offset;         /* OFFSETOF(wlock_t, owners) */
    size_t wlock_elem_link_offset;      /* OFFSETOF(wlock_elem_t, b.link) */
    size_t wlock_elem_lock_offset;      /* OFFSETOF(wlock_elem_t, lock) */
    size_t wlist_elems_offset;          /* OFFSETOF(wlist_t, elems) */
};
typedef struct tbstat_info_s tbstat_info_t ;

 /*========================================================================
  * struct cur_wait_t:
  * sess_get_current() 에서 현재 wait하고 있는 세션인 경우
  * wait event의 id, name, wait_time(0.1s) 정보를 넣어 주기 위한 구조체
  * ======================================================================*/
struct cur_wait_s {
    int id;                         /* wait event id */
    char name[MAX_DESC_LEN + 1];    /* wait event name */
    tick_t time;                    /* wait time (0.1s) */
};
typedef struct cur_wait_s cur_wait_t;


 /*========================================================================
 * -------------------------- tbstat APIs ---------------------------------
 *========================================================================*/

/**
 * @brief       init_tbstat
 *              1. 티베로 프로세스 정보 읽기
 *              2. shared memory attach
 *              3. tbstat_info setting and return
 *              ** 각종 list 정보들에 대한 메모리할당을 라이브러리에서 함
 *              ** free_tbstat과 짝으로 불러야 함
 *
 * @param[out]  tbstat_info 구조체:
 *                  - total working thread IDs (start 부터 end까지 순차적)
 *                  - stat과 wait event 의 목록의 갯수
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 *                  - TBSTAT_ERR_CANT_LOAD_PROC_INFO: 티베로가 떠있지 않거나 환경변수
 *                                         (TB_HOME, TB_SID설정이 안되어있을 때)
 */
#ifdef _WIN32
__declspec(dllexport)
int
init_tbstat(tbstat_info_t *tbstat_info);
#else
int
init_tbstat(tbstat_info_t *tbstat_info);
#endif

/**
 * @brief       free_tbstat
 *              tbstat_info_t 구조체 free
 *
 * @param[out]  tbstat_info 구조체:
 *                  - total working thread IDs (start 부터 end까지 순차적)
 *                  - stat과 wait event 의 목록의 갯수
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 */
#ifdef _WIN32
__declspec(dllexport)
int
free_tbstat(tbstat_info_t *tbstat_info);
#else
int
free_tbstat(tbstat_info_t *tbstat_info);
#endif

/**
 * @brief       sess_get_current
 *
 * @param[in]   sessid
 * @param[out]  curr: 한 thread의 current tbstat
 *                 - stat_history_elem_t 는 한 sql실행마다 남긴 정보가 포함
 *              cw: 세션이 wait하고 있는 이벤트가 있을 경우 그 정보
 *
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 *                  - TBSTAT_ERR_INVALID_SESSID
*                      : 인자로 들어온 sessid가 working session의 sessid가
*                        아니거나 주어진 session이 active 하지 않을 때
 */
#ifdef _WIN32
__declspec(dllexport)
int
sess_get_current(int sessid, stat_history_elem_t* curr, cur_wait_t *cw);
#else
int
sess_get_current(int sessid, stat_history_elem_t* curr, cur_wait_t *cw);
#endif

/**
 * @brief       thr_get_current
 *              !!! DEPRECATED !!!
 *              하위 버전과 호환성을 위한 것이지만,  Shared server가 아닌
 *              경우에만 제대로 동작 가능하다.
 *
 * @param[in]   sessid
 * @param[out]  curr: 한 thread의 current tbstat
 *                 - stat_history_elem_t 는 한 sql실행마다 남긴 정보가 포함
 *              cw: 세션이 wait하고 있는 이벤트가 있을 경우 그 정보
 *
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 *                  - TBSTAT_ERR_INVALID_SESSID
*                      : 인자로 들어온 sessid가 working session의 sessid가
*                        아니거나 주어진 session이 active 하지 않을 때
 */
#ifdef _WIN32
__declspec(dllexport)
int
thr_get_current(int sessid, stat_history_elem_t* curr, cur_wait_t *cw);
#else
int
thr_get_current(int sessid, stat_history_elem_t* curr, cur_wait_t *cw);
#endif


/**
 * @brief       get_stat_history_elem
 *
 * @param[in]   wsess_id: WSESS id
 *              elem_id: stat history의 element ID (0~SQL_STAT_HISTORY_QSIZE-1)
 * @param[out]  e: (sessid, elem_id)에 해당하는 stat history element 주소
 *
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 *                  - TBSTAT_ERR_INVALID_SESSID
*                      : 인자로 들어온 sessid가 working session의 sessid가
*                        아니거나 주어진 session이 active 하지 않을 때
 *                  - TBSTAT_ERR_INVALID_ELEM_ID: 인자로 들어온 elem_id가 존재하지 않을 때
 */
#ifdef _WIN32
__declspec(dllexport)
int
get_stat_history_elem(int sessid, int elem_id, stat_history_elem_t **e);
#else
int
get_stat_history_elem(int sessid, int elem_id, stat_history_elem_t **e);
#endif
/**
 * @brief       get_slog_history_elem
 *
 * @param[in]   wsess_id: WSESS id
 *              elem_id: slog history의 element ID (0~SQL_SLOG_HISTORY_QSIZE-1)
 * @param[out]  e: (sessid, elem_id)에 해당하는 slog history element 주소
 *
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 *                  - TBSTAT_ERR_INVALID_SESSID
*                      : 인자로 들어온 sessid가 working session의 sessid가
*                        아니거나 주어진 session이 active 하지 않을 때
 *                  - TBSTAT_ERR_INVALID_ELEM_ID: 인자로 들어온 elem_id가 존재하지 않을 때
 */
#ifdef _WIN32
__declspec(dllexport)
int
get_slog_history_elem(int sessid, int elem_id, slog_history_elem_t **e);
#else
int
get_slog_history_elem(int sessid, int elem_id, slog_history_elem_t **e);
#endif

/**
 * @brief       set_session_shm_address
 *
 * @param[out]  s: session_info_t *
 *
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 */
#ifdef _WIN32
__declspec(dllexport)
int
set_session_shm_address(session_info_t *s);
#else
int
set_session_shm_address(session_info_t *s);
#endif

/**
 * @brief       set_sesstat_shm_address
 *
 * @param[out]  s: sesstat_info_t *
 *
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 */
#ifdef _WIN32
__declspec(dllexport)
int
set_sesstat_shm_address(sesstat_info_t *s);
#else
int
set_sesstat_shm_address(sesstat_info_t *s);
#endif

/**
 * @brief       set_session_event_shm_address
 *
 * @param[out]  s: session_event_info_t *
 *
 * @return      error code
 *                  - TBSTAT_ERR_SUCCESS
 */
#ifdef _WIN32
__declspec(dllexport)
int
set_session_event_shm_address(session_event_info_t *s);
#else
int
set_session_event_shm_address(session_event_info_t *s);
#endif

/**
 * @brief       extract_wlock_info
 *
 * @param[out]  s: wlock_t address
 *
 * @return      wlock info
 */
#ifdef _WIN32
__declspec(dllexport)
wlock_info_t
extract_wlock_info(char *s);
#else
wlock_info_t
extract_wlock_info(char *s);
#endif

/**
 * @brief       extract_wlock_elem_info
 *
 * @param[out]  s: wlock_elem_t address
 *
 * @return      wlock elem info
 */
#ifdef _WIN32
__declspec(dllexport)
wlock_elem_info_t
extract_wlock_elem_info(char *s);
#else
wlock_elem_info_t
extract_wlock_elem_info(char *s);
#endif

/**
 * @brief       is_valid_shm_addr
 *
 * @param[out]  s: shared memory address
 *
 * @return      address validation check
 *              - sucess: TBSTAT_ERR_SUCCESS
 *              - fail:   TBSTAT_ERR_INVALID_SHM_ADDR
 */
#ifdef _WIN32
__declspec(dllexport)
int
is_valid_shm_addr(const char *s);
#else
int
is_valid_shm_addr(const char *s);
#endif

/**
 * @brief       convert hasval to base32 string
 *
 * @param[in]   val: hashval
 * @param[out]  buf: buffer
 *
 * @return      conversion check
 *              - sucess: TBSTAT_ERR_SUCCESS
 */
#ifdef _WIN32
__declspec(dllexport)
int
tbstat_sql_id_2_str(uint64_t val, char *buf);
#else
int
tbstat_sql_id_2_str(uint64_t val, char *buf);
#endif

/**
 * @brief       check wlock type validity
 *
 * @param[in]   wlock type
 *
 * @return      wlock type check result
 *              - sucess: 1
 *              - fail  : 0
 */
#ifdef _WIN32
__declspec(dllexport)
int
tbstat_check_wlock_type(int type);
#else
int
tbstat_check_wlock_type(int type);
#endif

/**
 * @brief       check wlock mode validity
 *
 * @param[in]   wlock mode
 *
 * @return      wlock mode check result
 *              - sucess: 1
 *              - fail  : 0
 */
#ifdef _WIN32
__declspec(dllexport)
int
tbstat_check_wlock_elem_mode(int mode);
#else
int
tbstat_check_wlock_elem_mode(int mode);
#endif

#endif /* no _TB_STAT_V1_H */
