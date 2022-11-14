/**
 * @file    tblog_reader.h
 * @brief   TODO brief documentation here.
 *
 * @author
 * @version $Id$
 */

#ifndef _TLI_H
#define _TLI_H

#ifdef __linux
#include <stdint.h>
#elif WIN32
typedef unsigned __int32 uint32_t;
typedef unsigned __int8 uint8_t;
#else
#include <sys/types.h>
#endif

#ifndef true
#   define true  1
#   define false 0
#endif

typedef unsigned char* TLI_HANDLE;
typedef unsigned char* TLI_RES_HANDLE;
typedef uint64_t tli_tsnval_t;

enum tli_rc_e {
    TLI_RC_ERR_ENC_INFO = -15, /* encryption정보 세팅이 잘못된 경우 */
    TLI_RC_ERR_SEEK = -14,
    TLI_RC_ERR_UNCONV_TXTBL = -13,   /* tx정보와 호환되지 않는 seq/blkno를
                                        주고 seek한 경우 */
    TLI_RC_ERR_BUFF_TOO_SHORT = -12, /* 너무 짧은 버퍼를 준 경우 */
    TLI_RC_ERR_COL_TYPE  = -11, /* 잘못된 column type을 string으로 변환      */
    TLI_RC_ERR_OP        = -10, /* 잘못된 op(tli_op_t)                       */
    TLI_RC_ERR_TRACE     = -9,  /* log reader lib의 trace 파일 설정 실패     */
    TLI_RC_ERR_STATUS    = -8,  /* 잘못된 상태(tli_open없이 tli_read 호출등) */
    TLI_RC_ERR_OPEN_REDO = -7,  /* redo log file open 실패                   */
    TLI_RC_ERR_READ_REDO = -6,  /* redo log file/block read 실패(corrupted?) */
    TLI_RC_ERR_REDO      = -5,  /* redo log data 해석 실패                   */
    TLI_RC_ERR_DDL       = -4,  /* ddl 관련 redo log 읽기 실패               */
    TLI_RC_ERR_LOAD_TX   = -3,  /* 이전 redo log 남은 tx 정보 읽기 실패      */
    TLI_RC_ERR_SAVE_TX   = -2,  /* 현재 redo log 진행 중 tx 저장 실패        */
    TLI_RC_ERR_REUSED    = -1,  /* redo log file 재사용되어 덮어써짐         */
    TLI_RC_OK            = 0,
    TLI_RC_WAIT_LOG,
    TLI_RC_END_LOG,
    TLI_RC_UNKNOWN_TX,
    TLI_RC_COL_IS_NULL
};
typedef enum tli_rc_e tli_rc_t;

enum tli_op_e {
    TLI_OP_INVALID = -2,
    TLI_OP_ERR     = -1,
    TLI_OP_INS     = 0,
    TLI_OP_DEL,
    TLI_OP_UPD,
    TLI_OP_MI,
    TLI_OP_MD,
    TLI_OP_DPI,
    TLI_OP_BEGIN_TX,
    TLI_OP_COMMIT_TX,
    TLI_OP_ROLLBACK,    /* 한 문장이 rollback 되었음을 나타냄 */
    TLI_OP_DDL,
    TLI_OP_DDL_SYNC,    /* TAC상에서 DDL시 node간 동기화 목적 */
    TLI_OP_CR_END,      /* CACHE RECOVERY 종료를 나타냄 */
    TLI_OP_MU
};
typedef enum tli_op_e tli_op_t;

typedef char * tli_col_t;

enum tli_data_type_e {
    TLI_TYPE_NUMBER    = 1,
    TLI_TYPE_CHAR      = 2,
    TLI_TYPE_VARCHAR   = 3,
    TLI_TYPE_RAW       = 4,
    TLI_TYPE_DATE      = 5,
    TLI_TYPE_TIME      = 6,
    TLI_TYPE_TIMESTAMP = 7,
    TLI_TYPE_LONG      = 10,
    TLI_TYPE_LONG_RAW  = 11,
    TLI_TYPE_BLOB      = 12,
    TLI_TYPE_CLOB      = 13,
    TLI_TYPE_NCHAR     = 18,
    TLI_TYPE_NVARCHAR  = 19,
    TLI_TYPE_NCLOB     = 20
};
typedef enum tli_data_type_e tli_data_type_t;


#ifndef _TLI_RBA_T
#define _TLI_RBA_T
typedef struct tli_rba_s tli_rba_t;
#endif
struct tli_rba_s {
    uint32_t seq;
    uint32_t blkno;
    uint16_t offset;
};


#define tli_rowid_get_blkno(rowid)      ((uint32_t)((rowid) >> 32))
#define tli_rowid_get_reldfid(rowid)    ((uint16_t)(((rowid) >> 16)) & 0xffff)
#define tli_rowid_get_rowno(rowid)      ((uint16_t)((rowid) & 0xffff))

#define tli_rowid_new(_blkno, _reldfid, _rowno)                                \
        (((uint64_t)(_blkno) << 32) + ((uint64_t)(_reldfid) << 16) + (_rowno))


/* TLI APIs */
TLI_HANDLE tli_new(char *workdir_path);
tli_rc_t tli_free(TLI_HANDLE handle);

void tli_set_dbblk_size(TLI_HANDLE handle, uint16_t dbblk_size);
void tli_set_logblk_size(TLI_HANDLE handle, uint16_t logblk_size);
void tli_set_logblk_cnt(TLI_HANDLE handle, uint16_t logblk_cnt);
void tli_set_retry_cnt(TLI_HANDLE handle, uint16_t retry_cnt);
tli_rc_t tli_set_trace_file(char *path);
void tli_set_trace_lvl(int lvl);
tli_rc_t tli_set_encryption_info(TLI_HANDLE handle, /*void *wallet,*/
                                 int enc_alg, int key_len, unsigned char *global_key);

/* tli_open flag */
#define TLI_OPENFLAG_INIT_TX        0x01
#define TLI_OPENFLAG_ARCHIVE        0x02    /* w/o O_DIRECT */
#define TLI_OPENFLAG_USE_O_DIRECT   0x04    /* O_DIRECT 강제 사용 */

tli_rc_t tli_open(TLI_HANDLE handle, char *lf_name, int flag);
tli_rc_t tli_close(TLI_HANDLE handle);
tli_rc_t tli_seek(TLI_HANDLE handle, tli_rba_t *pos_rba);

/* 에러가 발생하면 더이상 tx_tbl을 믿을 수 없다. tx_tbl을 clear하고
 * 마지막 tx_tbl file을 load해서 재시작해야 한다.*/
tli_rc_t tli_read(TLI_HANDLE handle, TLI_RES_HANDLE res, tli_rba_t *next_rba,
         tli_rba_t *err_rba);

void tli_get_txinfo_next_rba(TLI_HANDLE handle, tli_rba_t *next_rba);

tli_rc_t tli_save_txinfo(TLI_HANDLE handle, char *fname_buf, int len,
                         tli_rba_t *out_rba);
tli_rc_t tli_load_txinfo(TLI_HANDLE handle, char *fname, tli_rba_t *tx_rba);

int tli_rba_equals(tli_rba_t *a, tli_rba_t *b);

/* TLI Result APIs */
TLI_RES_HANDLE tli_res_new(TLI_HANDLE handle);
void tli_res_reset(TLI_RES_HANDLE res);
void tli_res_free(TLI_RES_HANDLE res);

tli_op_t tli_res_get_op(TLI_RES_HANDLE res);
uint32_t tli_res_get_xid(TLI_RES_HANDLE res);
uint32_t tli_res_get_sgmtid(TLI_RES_HANDLE res);
uint64_t tli_res_get_rowid(TLI_RES_HANDLE res);
tli_tsnval_t tli_res_get_tsnval(TLI_RES_HANDLE res);

int tli_res_get_insert_colcnt(TLI_RES_HANDLE res);
int *tli_res_get_insert_colnos(TLI_RES_HANDLE res);
tli_col_t tli_res_get_insert_col(TLI_RES_HANDLE res, int colno);

int tli_res_get_delete_where_colcnt(TLI_RES_HANDLE res);
int *tli_res_get_delete_where_colnos(TLI_RES_HANDLE res);
tli_col_t tli_res_get_delete_where_col(TLI_RES_HANDLE res, int colno);

int tli_res_get_update_where_colcnt(TLI_RES_HANDLE res);
int *tli_res_get_update_where_colnos(TLI_RES_HANDLE res);
tli_col_t tli_res_get_update_where_col(TLI_RES_HANDLE res, int colno);

int tli_res_get_update_set_colcnt(TLI_RES_HANDLE res);
int *tli_res_get_update_set_colnos(TLI_RES_HANDLE res);
tli_col_t tli_res_get_update_set_col(TLI_RES_HANDLE res, int colno);

int tli_res_get_collen(tli_col_t col);
char *tli_res_get_coldata(tli_col_t col);

tli_rc_t tli_res_get_col_to_str(tli_col_t col, tli_data_type_t type,
                                char *buf, int buf_len, char *fmt);

int tli_res_get_next_row(TLI_RES_HANDLE res);

int tli_res_get_rollback_cnt(TLI_RES_HANDLE res);

uint32_t tli_res_get_login_id(TLI_RES_HANDLE res);
uint32_t tli_res_get_ddl_login_id(TLI_RES_HANDLE res);
uint32_t tli_res_get_ddl_schema_id(TLI_RES_HANDLE res);
uint16_t tli_res_get_ddl_sql_id(TLI_RES_HANDLE res);
uint16_t tli_res_get_ddl_sql_no(TLI_RES_HANDLE res);

uint32_t tli_res_get_obj_schema_id(TLI_RES_HANDLE res);
uint32_t tli_res_get_obj_id(TLI_RES_HANDLE res);

char *tli_res_get_ddl_stmt(TLI_RES_HANDLE res);
char *tli_res_get_obj_name(TLI_RES_HANDLE res);
#endif /* no _TLI_H */
