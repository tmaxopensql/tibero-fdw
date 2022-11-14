


#ifndef _ALLOCATOR_T
#define _ALLOCATOR_T
typedef struct allocator_s allocator_t;
#endif

#ifndef _TB_SRC_POS_T
#define _TB_SRC_POS_T
typedef struct tb_src_pos_s tb_src_pos_t;
#endif  /* _TB_SRC_POS_T */

#ifndef _TBPARSER_ERR_INFO_T
#define _TBPARSER_ERR_INFO_T
typedef struct tbparser_err_info_s tbparser_err_info_t;
#endif /* _TBPARSER_ERR_INFO_T */

void psm_parser(char *src, list_t *err_list);

tb_bool_t dml_parser(allocator_t *alloc, char *src,  *err_pos);

void ddl_parser(const char *stmt, list_t *err_info);
