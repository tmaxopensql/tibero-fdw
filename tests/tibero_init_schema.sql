DECLARE
	ddl_query VARCHAR2(2000);
BEGIN
	ddl_query := 'CREATE TABLE just_conn_test_table(
								test char(50)
								)';
	EXECUTE IMMEDIATE ddl_query;

	ddl_query := 'CREATE TABLE t1(
								c_kor CHAR(100),
								c_eng CHAR(100),
								c_spc CHAR(100),
								c_kor_full CHAR(2000),
								c_eng_full CHAR(2000),
								c_spc_full CHAR(2000),
								nc_kor NCHAR(100),
								nc_eng NCHAR(100),
								nc_spc NCHAR(100),
								nc_kor_full NCHAR(2000),
								nc_eng_full NCHAR(2000),
								nc_spc_full NCHAR(2000),
								vc_kor VARCHAR(100),
								vc_eng VARCHAR(100),
								vc_spc VARCHAR(100),
								vc_kor_full VARCHAR(65532 BYTE),
								vc_eng_full VARCHAR(65532 BYTE),
								vc_spc_full VARCHAR(65532 BYTE),
								vc2_kor VARCHAR2(100),
								vc2_eng VARCHAR2(100),
								vc2_spc VARCHAR2(100),
								vc2_kor_full VARCHAR2(65532 BYTE),
								vc2_eng_full VARCHAR2(65532 BYTE),
								vc2_spc_full VARCHAR2(65532 BYTE),
								nvc_kor NVARCHAR(100),
								nvc_eng NVARCHAR(100),
								nvc_spc NVARCHAR(100),
								nvc_kor_full NVARCHAR(32766),
								nvc_eng_full NVARCHAR(32766),
								nvc_spc_full NVARCHAR(32766),
								nvc2_kor NVARCHAR2(100),
								nvc2_eng NVARCHAR2(100),
								nvc2_spc NVARCHAR2(100),
								nvc2_kor_full NVARCHAR2(32766),
								nvc2_eng_full NVARCHAR2(32766),
								nvc2_spc_full NVARCHAR2(32766),
								rwid ROWID
								)';
	EXECUTE IMMEDIATE ddl_query;

	ddl_query := 'CREATE TABLE t2(
								nb_default NUMBER,
								nb_380 NUMBER(38,0),
								nb_38191 NUMBER(38,19),
								nb_38192 NUMBER(38,19),
								nb_ltm NUMBER(38,-38),
								nb_gtm NUMBER(38,38),
								flt FLOAT
								)';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE smallint_test(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE smallint_test_err(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE integer_test(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE integer_test_err(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE bigint_test(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE bigint_test_err(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE r_and_dp_test(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE t3(
                dt DATE,
                dt_bc9999 DATE,
                dt_ad9999 DATE,
                dt_detail DATE,
                ts TIMESTAMP,
                ts1 TIMESTAMP(1),
                ts6 TIMESTAMP(6),
                ts9 TIMESTAMP(9),
                ts_bc TIMESTAMP,
                ts_ad TIMESTAMP,
                tsz TIMESTAMP WITH TIME ZONE,
                tsz1 TIMESTAMP WITH TIME ZONE,
                tsz6 TIMESTAMP WITH TIME ZONE,
                tsz9 TIMESTAMP WITH TIME ZONE,
                tsz_bc TIMESTAMP WITH TIME ZONE,
                tsz_ad TIMESTAMP WITH TIME ZONE,
                tslz TIMESTAMP WITH LOCAL TIME ZONE,
                tslz1 TIMESTAMP WITH LOCAL TIME ZONE,
                tslz6 TIMESTAMP WITH LOCAL TIME ZONE,
                tslz9 TIMESTAMP WITH LOCAL TIME ZONE,
                tslz_bc TIMESTAMP WITH LOCAL TIME ZONE,
                tslz_ad TIMESTAMP WITH LOCAL TIME ZONE
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE t4 (
                iytm INTERVAL YEAR(9) TO MONTH,
                idts INTERVAL DAY(9) TO SECOND(9)
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE ft_test (
                c1 INTEGER NOT NULL,
                c2 INTEGER NOT NULL,
                cx INTEGER,
                c3 VARCHAR2(100),
                c4 TIMESTAMP WITH TIME ZONE,
                c5 TIMESTAMP,
                c6 VARCHAR(10),
                c7 CHAR(10) default ''ft2''
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE vt1(
                col_char CHAR(10),
                col_varchar VARCHAR(20),
                col_varchar2 VARCHAR2(20),
                col_nchar NCHAR(10),
                col_nvarchar NVARCHAR(20),
                col_nvarchar2 NVARCHAR(20),
                col_rowid ROWID,
                col_number NUMBER,
                col_number_n_m NUMBER(5, 2),
                col_float FLOAT,
                col_date DATE,
                col_timestamp TIMESTAMP,
                col_timestamp_tz TIMESTAMP WITH TIME ZONE,
                col_interval_ym INTERVAL YEAR TO MONTH,
                col_interval_ds INTERVAL DAY TO SECOND
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE VIEW v_test AS SELECT * FROM vt1;';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE jt1(
                c_kor CHAR(100),
                c_eng CHAR(100),
                c_spc CHAR(100),
                nc_kor NCHAR(100),
                nc_eng NCHAR(100),
                nc_spc NCHAR(100),
                vc_kor VARCHAR(100),
                vc_eng VARCHAR(100),
                vc_spc VARCHAR(100),
                vc2_kor VARCHAR2(100),
                vc2_eng VARCHAR2(100),
                vc2_spc VARCHAR2(100),
                nvc_kor NVARCHAR(100),
                nvc_eng NVARCHAR(100),
                nvc_spc NVARCHAR(100),
                rwid ROWID,
                nm NUMBER,
                flt FLOAT,
                dt DATE,
                ts TIMESTAMP,
                tsz TIMESTAMP WITH TIME ZONE,
                iytm INTERVAL YEAR TO MONTH,
                idts INTERVAL DAY TO SECOND
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE st1 (
                c1 INTEGER primary key,
                c2 VARCHAR(10),
                c3 CHAR(9),
                c4 NUMBER,
                c5 DATE,
                c6 NUMBER(10,5),
                c7 INTEGER,
                c8 INTEGER,
                c9 NCHAR(9),
                c10 VARCHAR2(100)
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE st2 (
                c1 INTEGER primary key,
                c2 VARCHAR(100),
                c3 VARCHAR(100)
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE t_test1 (
                id INT,
                dates DATE,
                amounts NUMBER(10,2)
                ) PARTITION BY RANGE(dates) (
                PARTITION Q1_2022 VALUES LESS THAN (''20220401''),
                PARTITION Q2_2022 VALUES LESS THAN (''20220701''),
                PARTITION Q3_2022 VALUES LESS THAN (''20221001''),
                PARTITION Q4_2022 VALUES LESS THAN (''20230101'')
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE t_test2 (
                id INT,
                name VARCHAR(50),
                region VARCHAR(20)
                ) PARTITION BY LIST(region) (
                PARTITION d1 VALUES (''Northeast''),
                PARTITION d2 VALUES (''Southeast''),
                PARTITION d3 VALUES (''Northwest''),
                PARTITION d4 VALUES (''Southwest'')
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE t_test3 (
                id INT,
                name VARCHAR(50),
                department VARCHAR(20)
                )
                PARTITION BY HASH (id) (
                PARTITION ht1,
                PARTITION ht2,
                PARTITION ht3
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE syn_test (
                col_char CHAR(10),
                col_varchar VARCHAR(20),
                col_varchar2 VARCHAR2(20),
                col_nchar NCHAR(10),
                col_nvarchar NVARCHAR(20),
                col_nvarchar2 NVARCHAR2(20),
                col_rowid ROWID,
                col_number NUMBER,
                col_number_n_m NUMBER(5, 2),
                col_float FLOAT,
                col_date DATE,
                col_timestamp TIMESTAMP,
                col_timestamp_tz TIMESTAMP WITH TIME ZONE,
                col_timestamp_local_zone TIMESTAMP WITH LOCAL TIME ZONE,
                col_interval_ym INTERVAL YEAR TO MONTH,
                col_interval_ds INTERVAL DAY TO SECOND
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE SYNONYM ex_syn FOR syn_test;';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE null_test_table(
                null_c CHAR(100),
                null_nc NCHAR(100),
                null_vc VARCHAR(100),
                null_vc2 VARCHAR2(100),
                null_nvc NVARCHAR(100),
                null_rwid ROWID,
                null_nm NUMBER,
                null_flt FLOAT,
                null_dt DATE,
                null_ts TIMESTAMP,
                null_tsz TIMESTAMP WITH TIME ZONE,
                null_iytm INTERVAL YEAR TO MONTH,
                null_idts INTERVAL DAY TO SECOND
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE math_func_test_table(
                nb_default NUMBER,
                nb_380 NUMBER(38,0),
                nb_38191 NUMBER(38,19),
                nb_38192 NUMBER(38,19),
                nb_ltm NUMBER(38,-38),
                nb_gtm NUMBER(38,38),
                flt FLOAT
                )';
  EXECUTE IMMEDIATE ddl_query;

  ddl_query := 'CREATE TABLE ins_test (
                c1 INTEGER,
                c2 VARCHAR(10), 
                c3 CHAR(9), 
                c4 NUMBER, 
                c5 DATE, 
                c6 NUMBER(10,5), 
                c7 INTEGER, 
                c8 INTEGER,
                c9 NCHAR(9),
                c10 VARCHAR2(100)
                )';
  EXECUTE IMMEDIATE ddl_query;

END;
