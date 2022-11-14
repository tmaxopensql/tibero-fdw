#ifndef _TBSTAT_TYPE_H
#define _TBSTAT_TYPE_H

typedef enum sess_status_e {
    SESS_IDLE,
    SESS_ASSIGNED_FOR_RECOVERY,
    SESS_ASSIGNED,
    SESS_READY,
    SESS_RUNNING,
    SESS_ROLLING_BACK,
    SESS_CLEANUP,
    SESS_CLOSING,
} sess_status_t;

typedef enum thr_type_e {
#define THR(type, limit, desc) type,
#include "thr_type.list"
#undef THR
    THR_TYPE_MAX
} thr_type_t;

typedef enum thr_stat_e {
    THR_STAT_INVALID,
    THR_STAT_NEW,
    THR_STAT_IDLE,
    THR_STAT_RUNNING,
    THR_STAT_WAITING,
    THR_STAT_RECV_WAITING,
    THR_STAT_IO_WAITING,
    THR_STAT_STOP_BY_MTHR,
    THR_STAT_DEAD,
} thr_stat_t;

typedef struct list_link_s list_link_t;
typedef struct list_link_s list_t;
struct list_link_s {
    list_link_t *next, *prev;
};


#endif
