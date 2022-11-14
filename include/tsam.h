/* ------------------------------ tsam.h ------------------------------------ */
/*                                                                            */
/*                  Copyright (c) 2005 TmaxSoft Co., Ltd                      */
/*                        All Rights Reserved                                 */
/*                                                                            */
/* - Integrated Tmax VSAM Header File                                         */
/*                                                                            */
/* -------------------------------------------------------------------------- */

#ifndef __TSAM_H_
#define __TSAM_H_

#ifndef _WIN32
#include <inttypes.h>
#else
/* Windows */
typedef __int8  int8_t;
typedef __int16 int16_t;
typedef __int32 int32_t;
typedef __int64 int64_t;

typedef unsigned __int8  uint8_t;
typedef unsigned __int16 uint16_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#endif


#if defined (__cplusplus)
extern "C" {
#endif

/* -------------------------- tsam error codes ------------------------------ */

/* error code base and last error code */
#define TSAM_ERR_BASE                   (-1000)
#define TSAM_ERR_LAST                   (TSAM_ERR_BASE - 1000)

/* success error code */
#define TSAM_ERR_SUCCESS                (0)

/* common error codes */
#define TSAM_ERR_DUPLICATE_RECORD       (TSAM_ERR_BASE -    1)
#define TSAM_ERR_DUPLICATE_KEY          (TSAM_ERR_BASE -    2)
#define TSAM_ERR_NOT_FOUND              (TSAM_ERR_BASE -    3)
#define TSAM_ERR_END_OF_FILE            (TSAM_ERR_BASE -    4)
#define TSAM_ERR_RECORD_LOCKED          (TSAM_ERR_BASE -    5)
#define TSAM_ERR_RECORD_BUSY            (TSAM_ERR_BASE -    6)
#define TSAM_ERR_LENGTH_ERROR           (TSAM_ERR_BASE -    7)
#define TSAM_ERR_INVALID_REQUEST        (TSAM_ERR_BASE -    8)
#define TSAM_ERR_NOT_ALLOWED            (TSAM_ERR_BASE -    9)
#define TSAM_ERR_NO_SPACE               (TSAM_ERR_BASE -   10)
#define TSAM_ERR_MEMORY_ALLOC           (TSAM_ERR_BASE -   11)
#define TSAM_ERR_IO_FAILURE             (TSAM_ERR_BASE -   12)
#define TSAM_ERR_INVALID_OPERATION      (TSAM_ERR_BASE -   13)
#define TSAM_ERR_MAX_TASKS              (TSAM_ERR_BASE -   14)
#define TSAM_ERR_INTERNAL               (TSAM_ERR_BASE -   15)
#define TSAM_ERR_NOT_IMPLEMENTED        (TSAM_ERR_BASE -   16)

/* connection error codes */
#define TSAM_ERR_INVALID_CONN_ID        (TSAM_ERR_BASE -  101)
#define TSAM_ERR_MAX_CONNECTIONS        (TSAM_ERR_BASE -  102)
#define TSAM_ERR_DUPLICATE_CONN_ID      (TSAM_ERR_BASE -  103)
#define TSAM_ERR_TX_STILL_ACTIVE        (TSAM_ERR_BASE -  104)
#define TSAM_ERR_CONNECT_FAILURE        (TSAM_ERR_BASE -  105)
#define TSAM_ERR_AUTHENTICATION_FAILURE (TSAM_ERR_BASE -  106)
#define TSAM_ERR_INVALID_DATABASE       (TSAM_ERR_BASE -  107) /* new */
#define TSAM_ERR_DISCONNECTED           (TSAM_ERR_BASE -  108)

/* transaction error codes */
#define TSAM_ERR_INVALID_TX_ID          (TSAM_ERR_BASE -  201)
#define TSAM_ERR_MAX_TRANSACTIONS       (TSAM_ERR_BASE -  202)
#define TSAM_ERR_DUPLICATE_TX_ID        (TSAM_ERR_BASE -  203)
#define TSAM_ERR_TX_STARTED_ALREADY     (TSAM_ERR_BASE -  204)
#define TSAM_ERR_TX_START_FAILURE       (TSAM_ERR_BASE -  205)
#define TSAM_ERR_TX_COMMIT_FAILURE      (TSAM_ERR_BASE -  206)
#define TSAM_ERR_TX_ROLLBACK_FAILURE    (TSAM_ERR_BASE -  207)

/* file session error codes */
#define TSAM_ERR_INVALID_FD             (TSAM_ERR_BASE -  301)
#define TSAM_ERR_MAX_OPEN_FILES         (TSAM_ERR_BASE -  302)
#define TSAM_ERR_DUPLICATE_FD           (TSAM_ERR_BASE -  303)
#define TSAM_ERR_FILE_NOT_FOUND         (TSAM_ERR_BASE -  304)
#define TSAM_ERR_INVALID_ORGANIZATION   (TSAM_ERR_BASE -  305)
#define TSAM_ERR_VOLUME_NOT_FOUND       (TSAM_ERR_BASE -  306)

#define TSAM_ERR_INVALID_KEYDEF         (TSAM_ERR_BASE -  311) /* temporary */
#define TSAM_ERR_INVALID_ENTRY_TYPE     (TSAM_ERR_BASE -  312) /* temporary */
#define TSAM_ERR_MAX_KEYINFOS           (TSAM_ERR_BASE -  313) /* temporary */
#define TSAM_ERR_KEYINFO_NOT_FOUND      (TSAM_ERR_BASE -  314) /* temporary */
#define TSAM_ERR_PATHENTRY_NOT_FOUND    (TSAM_ERR_BASE -  315) /* temporary */
#define TSAM_ERR_DELETE_FILE_FAILURE    (TSAM_ERR_BASE -  316) /* temporary */
#define TSAM_ERR_ENTRY_DEFINED_ALREADY  (TSAM_ERR_BASE -  317) /* temporary */
#define TSAM_ERR_FILE_HAS_NO_KEYINFO    (TSAM_ERR_BASE -  318) /* temporary */
#define TSAM_ERR_INVALID_KEYFORMAT      (TSAM_ERR_BASE -  319) /* temporary */
#define TSAM_ERR_FILE_IN_USE            (TSAM_ERR_BASE -  320) /* temporary */
#define TSAM_ERR_INVALID_SHARE_OPTION   (TSAM_ERR_BASE -  321) /* SHARE */
#define TSAM_ERR_ALREADY_OPEN_OUTPUT    (TSAM_ERR_BASE -  322) /* SHARE */
#define TSAM_ERR_NON_RLS_OUTPUT_EXIST   (TSAM_ERR_BASE -  323) /* RLS */
#define TSAM_ERR_NON_RLS_INPUT_EXIST    (TSAM_ERR_BASE -  324) /* RLS */
#define TSAM_ERR_DEADLOCK               (TSAM_ERR_BASE -  325)
#ifdef  TSAM_NO_ESDS_SEQUENCE
#define TSAM_ERR_RBA_SEQ_NOT_FOUND      (TSAM_ERR_BASE -  326)
#endif /* TSAM_NO_ESDS_SEQUENCE */

/* record access error codes */
#define TSAM_ERR_INVALID_TOKEN          (TSAM_ERR_BASE -  401)
#define TSAM_ERR_MAX_LOCK_TOKENS        (TSAM_ERR_BASE -  402)
#define TSAM_ERR_TX_NOT_STARTED         (TSAM_ERR_BASE -  403)
#define TSAM_ERR_KEY_NOT_MATCHED        (TSAM_ERR_BASE -  404)
#define TSAM_ERR_KEYLENGTH              (TSAM_ERR_BASE -  405)
#define TSAM_ERR_GENERIC_KEYLENGTH      (TSAM_ERR_BASE -  406)

/* scan access error codes */
#define TSAM_ERR_INVALID_REQUEST_ID     (TSAM_ERR_BASE -  501)
#define TSAM_ERR_MAX_REQUEST_IDS        (TSAM_ERR_BASE -  502)
#define TSAM_ERR_DUPLICATE_REQUEST_ID   (TSAM_ERR_BASE -  503)
#define TSAM_ERR_INVALID_SCAN_ID        (TSAM_ERR_BASE -  504) /* TSAM/Tibero */

/* database related error codes */
#define TSAM_ERR_ALLOC_RDB_OBJECT       (TSAM_ERR_BASE -  901)
#define TSAM_ERR_FREE_RDB_OBJECT        (TSAM_ERR_BASE -  902)
#define TSAM_ERR_DEFINE_VARIABLE        (TSAM_ERR_BASE -  903)
#define TSAM_ERR_BIND_VARIABLE          (TSAM_ERR_BASE -  904)
#define TSAM_ERR_PREPARE_STATEMENT      (TSAM_ERR_BASE -  905)
#define TSAM_ERR_EXECUTE_STATEMENT      (TSAM_ERR_BASE -  906)
#define TSAM_ERR_FETCH_STATEMENT        (TSAM_ERR_BASE -  907)
#define TSAM_ERR_RETRIEVE_ROWID         (TSAM_ERR_BASE -  908)
#define TSAM_ERR_RETRIEVE_ROW_COUNT     (TSAM_ERR_BASE -  909)
#define TSAM_ERR_TABLE_NAME_LENGTH      (TSAM_ERR_BASE -  910) /* TSAM/Tibero */

/* -------------------------- transaction flags ----------------------------- */

#define TSAM_TX_READWRITE               0x00 /* default tx start flag */
#define TSAM_TX_READONLY                0x01
#define TSAM_TX_SERIALIZABLE            0x02
#define TSAM_TX_AUTOCOMMIT              0x04
#define TSAM_TX_DEADLOCK_PRTY           0x10

/* -------------------------- file organization ----------------------------- */

#define TSAM_ORGANIZATION_ESDS          0x01 /* Entity Sequenced Data Set */
#define TSAM_ORGANIZATION_KSDS          0x02 /* Key Sequenced Data Set */
#define TSAM_ORGANIZATION_RRDS          0x04 /* Relative Record Data Set */
#define TSAM_ORGANIZATION_VRDS          0x05 /* Variable Length RRDS */

/* -------------------------- file attributes ------------------------------- */

#define TSAM_ATTR_WRITECHECK            0x01 /* write check */
#define TSAM_ATTR_SPANNED               0x02 /* spanned records alloed */
#define TSAM_ATTR_UNIQUEKEY             0x04 /* non-unique or unique keys allowed */
#define TSAM_ATTR_LOGUNDO               0x10 /* LOG(UNDO), 2006.01.17 */
#define TSAM_ATTR_LOGREDO               0x20 /* virtual, 2006.01.17 */
#define TSAM_ATTR_LOGALL                0x30 /* LOG(ALL), 2006.01.17 */

/* -------------------------- AIX attributes -------------------------------- */

#define TSAM_RGATTR_EAIX                0x01 /* AIX for ESDS */
#define TSAM_RGATTR_KAIX                0x02 /* AIX for KSDS */
#define TSAM_RGATTR_UPGRADE             0x04 /* upgrade for AIX */
#define TSAM_RGATTR_UPDATE              0x04 /* update for path, 2006.01.17 */
#define TSAM_RGATTR_PATH                0x08 /* XXX temporary for native TSAM */

/* -------------------------- component attributes -------------------------- */

#define TSAM_CATTR_SPEED                0x01 /* close is not issued until the data set has been loaded */
#define TSAM_CATTR_UNIQUE               0x02 /* AIX alternate key value is unique */
#define TSAM_CATTR_REUSABLE             0x04 /* the data set can be reused */
#define TSAM_CATTR_ERASE                0x08 /* records are to be erased when deleted */
#define TSAM_CATTR_IHBUPDATE            0x10 /* the data component cannot be updated */
#define TSAM_CATTR_TEMPEXP              0x20 /* the data component was temporarily exported */

/* -------------------------- component share options ----------------------- */

#define TSAM_SHARE_REGION_MASK          0xF0
#define TSAM_SHARE_REGION_01            0x00
#define TSAM_SHARE_REGION_02            0x10
#define TSAM_SHARE_REGION_03            0x20
#define TSAM_SHARE_REGION_04            0x30

#define TSAM_SHARE_SYSTEM_MASK          0x0F
#define TSAM_SHARE_SYSTEM_01            0x00
#define TSAM_SHARE_SYSTEM_02            0x01
#define TSAM_SHARE_SYSTEM_03            0x02
#define TSAM_SHARE_SYSTEM_04            0x03

/* -------------------------- file open flags ------------------------------- */

/* in-out flags */
#define TSAM_MASK_IOMODE                0xFF000000
#define TSAM_FLAG_INPUT                 0x01000000
#define TSAM_FLAG_OUTPUT                0x02000000
#define TSAM_FLAG_EXTEND                0x04000000

/* access flags */
#define TSAM_MASK_ACCESSMODE            0x00FF0000
#define TSAM_FLAG_SEQUENTIAL            0x00010000
#define TSAM_FLAG_RANDOM                0x00020000
#define TSAM_FLAG_PREFETCH_NEVER        0x00200000

/* lock flags */
#define TSAM_MASK_LOCKMODE              0x0000FF00
#define TSAM_FLAG_AUTOLOCK              0x00000100
#define TSAM_FLAG_MULTILOCK             0x00000200
#define TSAM_FLAG_EXCLUSIVE             0x00000400

#define TSAM_FLAG_IGNORELOCK            0x00001000
#define TSAM_FLAG_RETRYLOCK             0x00002000
#define TSAM_FLAG_SKIPLOCK              0x00004000

/* miscellaneous flags */
#define TSAM_MASK_MISCELLANEOUS         0x000000FF
#define TSAM_FLAG_OPTIONAL              0x00000001
#define TSAM_FLAG_NO_REPORT             0x00000002
#define TSAM_FLAG_SPHERE                0x00000008 /* for native TSAM - open as sphere's member */

/* open mode */
#define TSAM_OPEN_MODE                  (TSAM_MASK_IOMODE)
#define TSAM_OPEN_INPUT                 (TSAM_FLAG_INPUT)
#define TSAM_OPEN_OUTPUT                (TSAM_FLAG_OUTPUT)
#define TSAM_OPEN_INOUT                 (TSAM_FLAG_INPUT  | TSAM_FLAG_OUTPUT)
#define TSAM_OPEN_EXTEND                (TSAM_FLAG_OUTPUT | TSAM_FLAG_EXTEND)
#define TSAM_OPEN_INOUTX                (TSAM_FLAG_INPUT  | TSAM_FLAG_OUTPUT | TSAM_FLAG_EXTEND)

/* access mode */
#define TSAM_ACCESS_MODE                (TSAM_MASK_ACCESSMODE)
#define TSAM_ACCESS_SEQUENTIAL          (TSAM_FLAG_SEQUENTIAL)
#define TSAM_ACCESS_RANDOM              (TSAM_FLAG_RANDOM)
#define TSAM_ACCESS_DYNAMIC             (TSAM_FLAG_RANDOM | TSAM_FLAG_SEQUENTIAL)

/* lock mode */
#define TSAM_LOCK_MODE                  (TSAM_MASK_LOCKMODE)
#define TSAM_LOCK_AUTO                  (TSAM_FLAG_AUTOLOCK)
#define TSAM_LOCK_MULTI                 (TSAM_FLAG_MULTILOCK)
#define TSAM_LOCK_EXCLUSIVE             (TSAM_FLAG_EXCLUSIVE)

/* -------------------------- file permission flags ------------------------- */

#define TSAM_PERMISSION_ALL             0xFFFF
#define TSAM_PERMISSION_ADD             0x0001
#define TSAM_PERMISSION_BROWSE          0x0002
#define TSAM_PERMISSION_DELETE          0x0004
#define TSAM_PERMISSION_READ            0x0008
#define TSAM_PERMISSION_UPDATE          0x0010

/* -------------------------- record level sharing mode  -------------------- */

/* XXX RLS mode */
#define TSAM_RLS_DR                     0x0001 /* NRI - Dirty Read        */
#define TSAM_RLS_CR                     0x0002 /* CR  - Committed Read    */
#define TSAM_RLS_RR                     0x0004 /* CRE - Repeatable Read   */

/* -------------------------- record level flags ---------------------------- */

/* positioning flags */
#define TSAM_MASK_POSITION              0x000000FF /* mask */
#define TSAM_FLAG_GENERIC               0x00000001
#define TSAM_FLAG_EQUAL                 0x00000002
#define TSAM_FLAG_GTEQ                  0x00000004

#define TSAM_FLAG_CURRENT               0x00000008
#define TSAM_FLAG_FIRST                 0x00000010
#define TSAM_FLAG_LAST                  0x00000020
#define TSAM_FLAG_NEXT                  0x00000040
#define TSAM_FLAG_PREV                  0x00000080

#define TSAM_DIRECTION_MASK                                             \
    (TSAM_FLAG_FIRST | TSAM_FLAG_LAST | TSAM_FLAG_NEXT | TSAM_FLAG_PREV)
#define TSAM_FLAG_IS_DIRECTION(x)   ( (x) & TSAM_DIRECTION_MASK )

/* rid type flags */
#define TSAM_MASK_RIDTYPE               0x0000FF00 /* mask */
#define TSAM_FLAG_KEY                   0x00000000
#define TSAM_FLAG_RBA                   0x00000100
#define TSAM_FLAG_RRN                   0x00000200
#define TSAM_FLAG_TOK                   0x00000400

/* other options */
#define TSAM_FLAG_MASSINSERT            0x00010000
#define TSAM_FLAG_UPDATE                0x00020000
#define TSAM_FLAG_NOSUSPEND             0x00040000
#define TSAM_FLAG_AUTOCOMMIT            0x00080000 /* for internal use only */
#define TSAM_FLAG_NOSKIP                0x00400000 /* do not skip in browsing */
#define TSAM_FLAG_REPLACE               0x00800000 /* MASSINSERT DUPREC */

/* for read_prefetch  */
#define TSAM_FLAG_NO_READ_PREFETCH      0x01000000
#define TSAM_FLAG_REQID_INTERNAL        0x02000000 /* for internal use */
#define TSAM_FLAG_START_SKIP            0x04000000 /* for internal use */


/* -------------------------- unlock token ---------------------------------- */

#define TSAM_TOKEN_ALL_LOCKS            -1

/* -------------------------- tsam connect type ----------------------------- */

#define TSAM_USERNAME_LEN               31
#define TSAM_PASSWORD_LEN               31
#define TSAM_DATABASE_LEN               255

typedef struct {
    char        username[TSAM_USERNAME_LEN + 1];
    char        password[TSAM_PASSWORD_LEN + 1];
    char        database[TSAM_DATABASE_LEN + 1];
    int         rlstmout; /* XXX RLS LOCK TIMEOUT */
} tsam_connect_t;

/* -------------------------- tsam backup type ----------------------------- */

typedef struct {
    char        username[TSAM_USERNAME_LEN + 1];
    char        password[TSAM_PASSWORD_LEN + 1];
    char        database[TSAM_DATABASE_LEN + 1];
    int         rlstmout; /* XXX RLS LOCK TIMEOUT */
    int         retry_count; /* maximum number of retry count */
    int         retry_interval; /* retry interval in seconds */
} tsam_backup_t;

/* -------------------------- tsam file create type ------------------------- */

#define TSAM_NAME_LEN                   63
#define TSAM_PATH_LEN                   255
#define TSAM_VOLSER_LEN                 6

typedef struct {
    /* association information */
    char        basename[TSAM_NAME_LEN + 1]; /* temporarily for native TSAM */
    char        dsname[TSAM_NAME_LEN + 1];
    char        dataname[TSAM_NAME_LEN + 1];
    char        indexname[TSAM_NAME_LEN + 1];
    /* volumes information */
    char        volser[TSAM_VOLSER_LEN + 2]; /* volume serial number */
    char        filepath[TSAM_PATH_LEN + 1];
    /* allocation information */
    int32_t     datpspac;       /* data component primary space */
    int32_t     datsspac;       /* data component secondary space */
    int32_t     idxpspac;       /* index component primary space */
    int32_t     idxsspac;       /* index component secondary space */
    /* VSAM attributes */
    int32_t     cisize;         /* size of control interval in bytes */
    int16_t     avglrecl;       /* average record length */
    int16_t     maxlrecl;       /* maximum record length */
    int16_t     keyfmt;         /* KSDS key format */
    int16_t     keypos;         /* KSDS key position */
    int16_t     keylen;         /* KSDS key length */
    int16_t     baseklen;       /* base key length of this AIX */
    /* VSAM information */
    uint8_t     vsamtype;       /* VSAM data set type information */
    uint8_t     vsamattr;       /* VSAM data set attributes - bit flags */
    uint8_t     rgattr;         /* alternate index attributes - bit flags */
    /* component attributes */
    uint8_t     datattrs;       /* data component attributes */
    uint8_t     datshopt;       /* data component share options */
    uint8_t     idxattrs;       /* index component attributes */
    uint8_t     idxshopt;       /* index component share options */
} tsam_create_t;

/* -------------------------- tsam file alter type -------------------------- */

typedef struct {
    /* association information */
    char        filename[TSAM_NAME_LEN + 1];
    /* VSAM attributes */
    int32_t     avglrecl;
    int32_t     maxlrecl;
    int16_t     keypos;
    int16_t     keylen;
    char        entry_type; /* temporary, for native TSAM only */
} tsam_alter_t;

/* -------------------------- tsam file stat -------------------------------- */

typedef struct {
    /* association information */
    char        dsname[TSAM_NAME_LEN + 1];
    char        dataname[TSAM_NAME_LEN + 1];
    char        indexname[TSAM_NAME_LEN + 1];
    /* volumes information */
    char        volser[TSAM_VOLSER_LEN + 2]; /* volume serial number */
    char        filepath[TSAM_PATH_LEN + 1];
    /* VSAM information */
    uint8_t     vsamtype;       /* VSAM data set type information */
    uint8_t     vsamattr;       /* VSAM data set attributes - bit flags */
    uint8_t     rgattr;         /* alternate index attributes - bit flags */
    uint8_t     filler;
    /* allocation information */
    int64_t     dat_harbads;    /* data set high allocated RBA */
    int64_t     dat_hurbads;    /* data set high used RBA */
    int16_t     dat_noextnt;    /* number of extents in data or index component */
    int64_t     idx_harbads;    /* data set high allocated RBA */
    int64_t     idx_hurbads;    /* data set high used RBA */
    int16_t     idx_noextnt;    /* number of extents in data or index component */
    /* statistics information */
    int32_t     rec_total;      /* number of logical records */
    int32_t     freespace;      /* bytes of free space in component */
    int16_t     ca_freespace;   /* percentage of free CIs in CA */
    int16_t     ci_freespace;   /* percentage of bytes free in CI */
    int16_t     ca_splits;      /* number of CA splits */
    int16_t     ci_splits;      /* number of CI splits */
    int32_t     entries_sect;   /* number of entries in each section (index only) */
    int32_t     hi_level_rba;   /* the RBA of the highest level index record (index only) */
    int32_t     levels;         /* number of levels of records in the index (index only) */
    int32_t     seq_set_rba;    /* the RBA of the first sequence set record (index only) */
    char        entry_type;     /* temporary, for native TSAM only */
} tsam_stat_t;

/* -------------------------- tsam file info -------------------------------- */

typedef struct {
    /* comminication handle */
    uint32_t    handle;         /* file handle to communicate TSAM server */
    uint32_t    bhandle;        /* base handle to communicate TSAM server */
    /* association information */
    char        filename[TSAM_NAME_LEN + 1];
    char        dsname[TSAM_NAME_LEN + 1];
    char        dataname[TSAM_NAME_LEN + 1];
    char        indexname[TSAM_NAME_LEN + 1];
    /* volumes information */
    char        volser[TSAM_VOLSER_LEN + 2]; /* volume serial number */
    char        filepath[TSAM_PATH_LEN + 1];
    /* open flags */
    uint32_t    openflags;      /* file open flags */
    uint16_t    permissions;    /* access permission */
    uint16_t    rlsmode;        /* record level sharing mode */
    /* VSAM attributes */
    int32_t     cisize;         /* size of control interval in bytes - added, 2005.07.01 */
    int16_t     avglrecl;       /* average record length */
    int16_t     maxlrecl;       /* maximum record length */
    int16_t     keyfmt;         /* KSDS key format */
    int16_t     keypos;         /* KSDS key position */
    int16_t     keylen;         /* KSDS key length */
    int16_t     akeypos;        /* offset in the data record of this AIX key */
    int16_t     baseklen;       /* base key length of this AIX */
    int16_t     baserecl;       /* base record length of this AIX */
    /* VSAM information */
    uint8_t     vsamtype;       /* VSAM data set type information */
    uint8_t     vsamattr;       /* VSAM data set attributes - bit flags */
    uint8_t     rgattr;         /* alternate index attributes - bit flags */
    /* SHARE OPTIONS */
    uint8_t     datshopt;       /* data component share options */
    uint8_t     idxshopt;       /* index component share options */
} tsam_file_t;

/* -------------------------- tsam file report ------------------------------ */

typedef struct {
    /* statistics information */
    int32_t     rec_inserted;   /* number of inserted records */
    int32_t     rec_retrieved;  /* number of retrieved records */
    int32_t     rec_updated;    /* number of updated records */
    int32_t     rec_deleted;    /* number of deleted records */
} tsam_report_t;

/* -------------------------- compatible API switch ------------------------- */

/**
 * program using TSAM API can be coded like following;
 * loose coupling, change only TSAM library (tsr or tsam)
 *
 * tsam_switch_t *tsam_sw;
 *
 * tsam_sw = {
 *     tsam_connect,
 *     tsam_disconnct,
 *     tsam_tx_start,
 *     ...
 * };
 **/

#define TSAM_IMPL_NAME_LEN              31

typedef struct {
    /* identification */
    char  tsam_impl_name[TSAM_IMPL_NAME_LEN + 1];
    int   tsam_version;
    /* connection */
    int (*tsam_connect_entry)(int cd, tsam_connect_t *conn);
    int (*tsam_connect2_entry)(int cd, tsam_connect_t *conn, tsam_backup_t *back);
    int (*tsam_disconnect_entry)(int cd);
    /* transaction */
    int (*tsam_tx_start_entry)(int txid, int cd, int flags);
    int (*tsam_tx_end_entry)(int txid);
    int (*tsam_tx_commit_entry)(int txid);
    int (*tsam_tx_rollback_entry)(int txid);
    /* file management */
    int (*tsam_create_entry)(int cd, char *filename, tsam_create_t *create);
    int (*tsam_remove_entry)(int cd, char *filename, tsam_file_t *file);
    int (*tsam_truncate_entry)(int cd, char *filename, tsam_file_t *file);
    int (*tsam_alter_entry)(int cd, char *filename, tsam_alter_t *alter);
    int (*tsam_stat_entry)(int cd, char *filename, tsam_stat_t *stat);
    int (*tsam_filename_entry)(int cd, char *dsname, char *filename);
#ifdef  TSAM_NO_ESDS_SEQUENCE
    int (*tsam_rba_table_entry)(int cd, int command, char *volser);
#endif
    /* file open session */
    int (*tsam_open_entry)(int fd, int cd, tsam_file_t *file, tsam_report_t *report);
    int (*tsam_close_entry)(int fd);
    int (*tsam_bldindex_entry)(int fd, int basefd);
    /* record access */
    int (*tsam_start_entry)(int fd, void *ridfld, int keylen, int flags);
    int (*tsam_read_entry)(int fd, void *ridfld, int keylen, char *buf, int *buflen, int flags);
    int (*tsam_unlock_entry)(int fd, int token);
    int (*tsam_write_entry)(int fd, void *ridfld, int keylen, char *buf, int buflen, int flags);
    int (*tsam_rewrite_entry)(int fd, void *ridfld, int keylen, char *buf, int buflen, int flags);
    int (*tsam_delete_entry)(int fd, void *ridfld, int keylen, int flags);
    /* scan access */
    int (*tsam_start_br_entry)(int fd, int reqid, void *ridfld, int keylen, int flags);
    int (*tsam_reset_br_entry)(int fd, int reqid, void *ridfld, int keylen, int flags);
    int (*tsam_end_br_entry)(int fd, int reqid);
    int (*tsam_read_next_entry)(int fd, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
    int (*tsam_read_prev_entry)(int fd, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
    /* sphere open session */
    int (*tsam_sphere_open_entry)(int fcnt, int *fds, int cd, tsam_file_t **files, tsam_report_t **reports);
    int (*tsam_sphere_close_entry)(int fcnt, int *fds);
    /* sphere record access */
    int (*tsam_sphere_start_entry)(int fcnt, int *fds, int aix, void *ridfld, int keylen, int flags);
    int (*tsam_sphere_read_entry)(int fcnt, int *fds, int aix, void *ridfld, int keylen, char *buf, int *buflen, int flags);
    int (*tsam_sphere_unlock_entry)(int fcnt, int *fds, int aix, int token);
    int (*tsam_sphere_write_entry)(int fcnt, int *fds, int aix, void *ridfld, int keylen, char *buf, int buflen, int flags);
    int (*tsam_sphere_rewrite_entry)(int fcnt, int *fds, int aix, void *ridfld, int keylen, char *buf, int buflen, int flags);
    int (*tsam_sphere_delete_entry)(int fcnt, int *fds, int aix, void *ridfld, int keylen, int flags);
    /* sphere scan access */
    int (*tsam_sphere_start_br_entry)(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, int flags);
    int (*tsam_sphere_reset_br_entry)(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, int flags);
    int (*tsam_sphere_end_br_entry)(int fcnt, int *fds, int aix, int reqid);
    int (*tsam_sphere_read_next_entry)(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
    int (*tsam_sphere_read_prev_entry)(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
    /* XA connect for multiple RM */
    int (*tsam_xa_connect_entry)(int cd, char *xa_conn_id);
} tsam_switch_t;

/**
 * Every implementation of TSAM API should define tsam_switch_t tsam_sw
 **/

extern tsam_switch_t tsam_sw;

/* -------------------------- identification -------------------------------- */

/**
 * XXX Original TSAM API:
 *
 *   char  tsam_impl_name[TSAM_IMPL_NAME_LEN + 1];
 *   int   tsam_version;
 *
 * XXX tsam_impl_name:
 *
 *   "TSAM NATIVE" : native tsam implementation
 *   "TSAM ON RDB" : tsam on rdb implementation
 *
 * XXX tsam_version:
 *
 *   200 : tsam version 2.0
 **/

#define tsam_impl_name (tsam_sw.tsam_impl_name)
#define tsam_version (tsam_sw.tsam_version)

/* -------------------------- connection ------------------------------------ */

/**
 * XXX Original TSAM API:
 *
 *   int tsam_connect(int cd, tsam_connect_t *conn);
 *   int tsam_connect2(int cd, tsam_connect_t *conn, tsam_backup_t *back);
 *   int tsam_disconnect(int cd);
 **/

#define tsam_connect(cd, conn) (*(tsam_sw.tsam_connect_entry))(cd, conn)
#define tsam_connect2(cd, conn, back) (*(tsam_sw.tsam_connect2_entry))(cd, conn, back)
#define tsam_xa_connect(cd, conn_id) (*(tsam_sw.tsam_xa_connect_entry))(cd, conn_id)
#define tsam_disconnect(cd) (*(tsam_sw.tsam_disconnect_entry))(cd)

/* -------------------------- transaction ----------------------------------- */

/**
 * transaction API name change - infix '_tx_' explicitly
 * (tsam_'tx'_start/end/commit/rollback)
 *
 * transaction start function receives 'int flags' additionally
 * (meaning transaction level - read only, read write, serializable)
 *
 * XXX omit 'int cd' parameter - can be determined from 'int txid'
 * map (txid) ---> (cd)
 *
 * XXX Original TSAM API:
 *
 *   int tsam_tx_start(int txid, int cd, int flags);
 *   int tsam_tx_end(int txid);
 *
 *   int tsam_tx_commit(int txid);
 *   int tsam_tx_rollback(int txid);
 **/

#define tsam_tx_start(txid, cd, flags) (*(tsam_sw.tsam_tx_start_entry))(txid, cd, flags)
#define tsam_tx_end(txid) (*(tsam_sw.tsam_tx_end_entry))(txid)

#define tsam_tx_commit(txid) (*(tsam_sw.tsam_tx_commit_entry))(txid)
#define tsam_tx_rollback(txid) (*(tsam_sw.tsam_tx_rollback_entry))(txid)

/* -------------------------- file management ------------------------------- */

/**
 * function name change - tsam_build() --> tsam_create()
 * function name change - tsam_erase() --> tsam_remove()
 *
 * XXX Original TSAM API:
 *
 *   int tsam_create(int cd, char *filename, tsam_create_t *create);
 *   int tsam_remove(int cd, char *filename, tsam_file_t *file);
 *
 *   int tsam_truncate(int cd, char *filename, tsam_file_t *file);
 *   int tsam_alter(int cd, char *filename, tsam_alter_t *alter);
 *   int tsam_stat(int cd, char *filename, tsam_stat_t *stat);
 *
 *   int tsam_filename(int cd, char *dsname, char *filename);
 **/

#define tsam_create(cd, filename, create) (*(tsam_sw.tsam_create_entry))(cd, filename, create)
#define tsam_remove(cd, filename, file) (*(tsam_sw.tsam_remove_entry))(cd, filename, file)

#define tsam_truncate(cd, filename, file) (*(tsam_sw.tsam_truncate_entry))(cd, filename, file)
#define tsam_alter(cd, filename, alter) (*(tsam_sw.tsam_alter_entry))(cd, filename, alter)
#define tsam_stat(cd, filename, stat) (*(tsam_sw.tsam_stat_entry))(cd, filename, stat)

#define tsam_filename(cd, dsname, filename) (*(tsam_sw.tsam_filename_entry))(cd, dsname, filename)

#ifdef  TSAM_NO_ESDS_SEQUENCE
#define TSAM_RBA_TABLE_CREATE           0x01
#define TSAM_RBA_TABLE_REMOVE           0x02
#endif

#ifdef  TSAM_NO_ESDS_SEQUENCE
#define tsam_rba_table(cd, command, volser) (*(tsam_sw.tsam_rba_table_entry))(cd, command, volser)
#endif

/* -------------------------- file open session ----------------------------- */

/**
 * XXX omit 'int cd' - can be determined from 'int fd'
 * map (fd) ---> (cd, [txid])
 *
 * XXX file information - after? or before? tsam file open
 *
 * XXX tsam file create does or not open file (return fd?)
 * (create then open for MFCOBOL OPEN OUTPUT)
 *
 * XXX AMS provide tsam_file_t for tsam_open()
 *
 * XXX Original TSAM API:
 *
 *   int tsam_open(int fd, int cd, tsam_file_t *file, tsam_report_t *report);
 *   int tsam_close(int fd);
 *   int tsam_bldindex(int fd, int basefd);
 **/

#define tsam_open(fd, cd, file, report) (*(tsam_sw.tsam_open_entry))(fd, cd, file, report)
#define tsam_close(fd) (*(tsam_sw.tsam_close_entry))(fd)
#define tsam_bldindex(fd, basefd) (*(tsam_sw.tsam_bldindex_entry))(fd, basefd)

/* -------------------------- record access --------------------------------- */

/**
 * XXX omit 'int cd, int txid' - can be determined from 'int fd'
 * map (fd) ---> (cd, [txid])
 *
 * void *ridfld, int keylen --> tsam_rid_t *rid
 * (tsam server already using tsam_rid_t internally, change the name)
 *
 * char **buf, int *buflen --> char *buf, int *buflen
 * (read functions use *buflen as INOUT param)
 *
 * read buffer must be provided by caller (ex, cics_fc, ...)
 * (XXX CICS SET storage functionality must be re-implemented)
 *
 * READ FOR UPDATE 'token' is managed by TSAM
 *
 * XXX new flag TSAM_RID_TOK
 *
 * XXX Original TSAM API:
 *
 *   int tsam_start(int fd, void *ridfld, int keylen, int flags);
 *   int tsam_read(int fd, void *ridfld, int keylen, char *buf, int *buflen, int flags);
 *   int tsam_unlock(int fd, int token);
 *
 *   int tsam_write(int fd, void *ridfld, int keylen, char *buf, int buflen, int flags);
 *   int tsam_rewrite(int fd, void *ridfld, int keylen, char *buf, int buflen, int flags);
 *   int tsam_delete(int fd, void *ridfld, int keylen, int flags);
 **/

#define tsam_start(fd, ridfld, keylen, flags) (*(tsam_sw.tsam_start_entry))(fd, ridfld, keylen, flags)
#define tsam_read(fd, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_read_entry))(fd, ridfld, keylen, buf, buflen, flags)
#define tsam_unlock(fd, token) (*(tsam_sw.tsam_unlock_entry))(fd, token)

#define tsam_write(fd, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_write_entry))(fd, ridfld, keylen, buf, buflen, flags)
#define tsam_rewrite(fd, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_rewrite_entry))(fd, ridfld, keylen, buf, buflen, flags)
#define tsam_delete(fd, ridfld, keylen, flags) (*(tsam_sw.tsam_delete_entry))(fd, ridfld, keylen, flags)

/* -------------------------- scan access ----------------------------------- */

/**
 * XXX omit 'int cd, int txid, int fd' - can be determined from 'int scanid'
 * except, tsam_scan_reset() whose fd may be used to change scanning file.
 * map (scanid) ---> (cd, txid, fd)
 *
 * tsam_scan_reset() first argument is changed into scanid from fd
 *
 * tsam_scan_end() need no flags as argument
 *
 * XXX tsr scan next/prev functions missing 'flags' parameter, is it OK?
 *
 * XXX Original TSAM API:
 *
 *   int tsam_start_br(int fd, int reqid, void *ridfld, int keylen, int flags);
 *   int tsam_reset_br(int fd, int reqid, void *ridfld, int keylen, int flags);
 *   int tsam_end_br(int fd, int reqid);
 *
 *   int tsam_read_next(int fd, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
 *   int tsam_read_prev(int fd, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
 **/

#define tsam_start_br(fd, reqid, ridfld, keylen, flags) (*(tsam_sw.tsam_start_br_entry))(fd, reqid, ridfld, keylen, flags)
#define tsam_reset_br(fd, reqid, ridfld, keylen, flags) (*(tsam_sw.tsam_reset_br_entry))(fd, reqid, ridfld, keylen, flags)
#define tsam_end_br(fd, reqid) (*(tsam_sw.tsam_end_br_entry))(fd, reqid)

#define tsam_read_next(fd, reqid, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_read_next_entry))(fd, reqid, ridfld, keylen, buf, buflen, flags)
#define tsam_read_prev(fd, reqid, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_read_prev_entry))(fd, reqid, ridfld, keylen, buf, buflen, flags)

/* -------------------------- sphere open session --------------------------- */

/**
 * XXX Original TSAM API:
 *
 *   int tsam_sphere_open(int fcnt, int *fds, int cd, tsam_file_t **files, tsam_report_t **reports);
 *   int tsam_sphere_close(int fcnt, int *fds);
 **/

#define tsam_sphere_open(fcnt, fds, cd, files, reports) (*(tsam_sw.tsam_sphere_open_entry))(fcnt, fds, cd, files, reports)
#define tsam_sphere_close(fcnt, fds) (*(tsam_sw.tsam_sphere_close_entry))(fcnt, fds)

/* -------------------------- sphere record access -------------------------- */

/**
 * XXX Original TSAM API:
 *
 *   int tsam_sphere_start(int fcnt, int *fds, int aix, void *ridfld, int keylen, int flags);
 *   int tsam_sphere_read(int fcnt, int *fds, int aix, void *ridfld, int keylen, char *buf, int *buflen, int flags);
 *   int tsam_sphere_unlock(int fcnt, int *fds, int aix, int token);
 *
 *   int tsam_sphere_write(int fcnt, int *fds, int aix, void *ridfld, int keylen, char *buf, int buflen, int flags);
 *   int tsam_sphere_rewrite(int fcnt, int *fds, int aix, void *ridfld, int keylen, char *buf, int buflen, int flags);
 *   int tsam_sphere_delete(int fcnt, int *fds, int aix, void *ridfld, int keylen, int flags);
 **/

#define tsam_sphere_start(fcnt, fds, aix, ridfld, keylen, flags) (*(tsam_sw.tsam_sphere_start_entry))(fcnt, fds, aix, ridfld, keylen, flags)
#define tsam_sphere_read(fcnt, fds, aix, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_sphere_read_entry))(fcnt, fds, aix, ridfld, keylen, buf, buflen, flags)
#define tsam_sphere_unlock(fcnt, fds, aix, token) (*(tsam_sw.tsam_sphere_unlock_entry))(fcnt, fds, aix, token)

#define tsam_sphere_write(fcnt, fds, aix, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_sphere_write_entry))(fcnt, fds, aix, ridfld, keylen, buf, buflen, flags)
#define tsam_sphere_rewrite(fcnt, fds, aix, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_sphere_rewrite_entry))(fcnt, fds, aix, ridfld, keylen, buf, buflen, flags)
#define tsam_sphere_delete(fcnt, fds, aix, ridfld, keylen, flags) (*(tsam_sw.tsam_sphere_delete_entry))(fcnt, fds, aix, ridfld, keylen, flags)

/* -------------------------- sphere scan access ---------------------------- */

/**
 * XXX Original TSAM API:
 *
 *   int tsam_sphere_start_br(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, int flags);
 *   int tsam_sphere_reset_br(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, int flags);
 *   int tsam_sphere_end_br(int fcnt, int *fds, int aix, int reqid);
 *
 *   int tsam_sphere_read_next(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
 *   int tsam_sphere_read_prev(int fcnt, int *fds, int aix, int reqid, void *ridfld, int keylen, char *buf, int *buflen, int flags);
 **/

#define tsam_sphere_start_br(fcnt, fds, aix, reqid, ridfld, keylen, flags) (*(tsam_sw.tsam_sphere_start_br_entry))(fcnt, fds, aix, reqid, ridfld, keylen, flags)
#define tsam_sphere_reset_br(fcnt, fds, aix, reqid, ridfld, keylen, flags) (*(tsam_sw.tsam_sphere_reset_br_entry))(fcnt, fds, aix, reqid, ridfld, keylen, flags)
#define tsam_sphere_end_br(fcnt, fds, aix, reqid) (*(tsam_sw.tsam_sphere_end_br_entry))(fcnt, fds, aix, reqid)

#define tsam_sphere_read_next(fcnt, fds, aix, reqid, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_sphere_read_next_entry))(fcnt, fds, aix, reqid, ridfld, keylen, buf, buflen, flags)
#define tsam_sphere_read_prev(fcnt, fds, aix, reqid, ridfld, keylen, buf, buflen, flags) (*(tsam_sw.tsam_sphere_read_prev_entry))(fcnt, fds, aix, reqid, ridfld, keylen, buf, buflen, flags)

/* -------------------------- end of header --------------------------------- */

#if defined (__cplusplus)
}
#endif

/* vim:set et ts=4 sw=4 cin: */
#endif /* __TSAM_H_ */

