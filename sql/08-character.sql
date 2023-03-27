-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(14);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE FOREIGN TABLE ft8 (
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
      nc_spc_full NCHAR(2000)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');
  
  -- TEST 1
  SELECT is(
    (SELECT c_kor FROM ft8),
    '가',
    '한국어 문자열 SELECT 확인'
  );

  -- TEST 2
  SELECT is(
    (SELECT nc_kor_full FROM ft8),
    '가나다라마바사아자차카타파하',
    '한국어 다중길이 문자열 SELECT 확인'
  );

  -- TEST 3
  SELECT is(
    (SELECT nc_spc_full FROM ft8),
    '!@#$%^&*()<>/\\''',
    '특수문자 SELECT 확인'
  );
  
  -- TEST 4
  SELECT row_eq(
    $$SELECT * FROM ft8 LIMIT 1$$,
    ROW('가'::char, 'a'::char, '!'::char, 
      '가나다라마바사아자차카타파하'::char(2000),
      'abcdefghijklmnopqrstuvwxyz'::char(2000), 
      '!@#$%^&*()<>/\\'''::char(2000), 
      '가'::char, 'a'::char, '!'::char,
      '가나다라마바사아자차카타파하'::char(2000),
      'abcdefghijklmnopqrstuvwxyz'::char(2000),
      '!@#$%^&*()<>/\\'''::char(2000)),
    '모든 열 CHAR 타입으로 결과 SELECT 확인'
  );

  CREATE FOREIGN TABLE err_ft8 (
      c_kor CHAR,
      c_eng CHAR,
      c_spc CHAR,
      c_kor_full CHAR,
      c_eng_full CHAR,
      c_spc_full CHAR,
      nc_kor NCHAR,
      nc_eng NCHAR,
      nc_spc NCHAR,
      nc_kor_full NCHAR,
      nc_eng_full NCHAR,
      nc_spc_full NCHAR
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');

  -- TEST 5
  SELECT throws_ok(
    'SELECT c_kor_full FROM err_ft8',
    '22001',
    'value too long for type character(1)',
    'CHAR(1) 타입 컬럼으로 긴 문자열을 받아오려는 경우 에러 발생 여부 확인'
  );

  -- TEST 6
  SELECT throws_ok(
    'SELECT nc_spc_full FROM err_ft8',
    '22001',
    'value too long for type character(1)',
    'NCHAR(1) 타입 컬럼으로 긴 문자열을 받아오려는 경우 에러 발생 여부 확인'
  );

  CREATE FOREIGN TABLE ft9 (
      vc_kor VARCHAR(100),
      vc_eng VARCHAR(100),
      vc_spc VARCHAR(100),
      vc_kor_full VARCHAR(65532),
      vc_eng_full VARCHAR(65532),
      vc_spc_full VARCHAR(65532),
      vc2_kor VARCHAR(100),
      vc2_eng VARCHAR(100),
      vc2_spc VARCHAR(100),
      vc2_kor_full VARCHAR(65532),
      vc2_eng_full VARCHAR(65532),
      vc2_spc_full VARCHAR(65532)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');

  -- TEST 7
  SELECT row_eq(
    $$SELECT * FROM ft9 LIMIT 1$$,
    ROW('가'::VARCHAR, 'a'::VARCHAR, '!'::VARCHAR,
      '가나다라마바사아자차카타파하'::VARCHAR,
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR,
      '!@#$%^&*()<>/\\'''::VARCHAR,
      '가'::VARCHAR, 'a'::VARCHAR, '!'::VARCHAR,
      '가나다라마바사아자차카타파하'::VARCHAR,
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR,
      '!@#$%^&*()<>/\\'''::VARCHAR),
    '모든 열 VARCHAR 타입으로 결과 SELECT 확인'
  );

  CREATE FOREIGN TABLE err_ft9 (
      vc_kor VARCHAR(1),
      vc_eng VARCHAR(1),
      vc_spc VARCHAR(1),
      vc_kor_full VARCHAR(1),
      vc_eng_full VARCHAR(1),
      vc_spc_full VARCHAR(1),
      vc2_kor VARCHAR(1),
      vc2_eng VARCHAR(1),
      vc2_spc VARCHAR(1),
      vc2_kor_full VARCHAR(1),
      vc2_eng_full VARCHAR(1),
      vc2_spc_full VARCHAR(1)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');

  -- TEST 8
  SELECT throws_ok(
    'SELECT vc_kor_full FROM err_ft9',
    '22001',
    'value too long for type character varying(1)',
    'VARCHAR(1) 타입 컬럼으로 긴 문자열을 받아오려는 경우 에러 발생 여부 확인'
  );

  -- TEST 9
  SELECT throws_ok(
    'SELECT vc_eng_full FROM err_ft9',
    '22001',
    'value too long for type character varying(1)',
    'VARCHAR(1) 타입 컬럼으로 긴 문자열을 받아오려는 경우 에러 발생 여부 확인'
  );

  -- TEST 10
  SELECT throws_ok(
    'SELECT vc_spc_full FROM err_ft9',
    '22001',
    'value too long for type character varying(1)',
    'VARCHAR(1) 타입 컬럼으로 긴 문자열을 받아오려는 경우 에러 발생 여부 확인'
  );

  CREATE FOREIGN TABLE ft10 (
      nvc_kor TEXT,
      nvc_eng TEXT,
      nvc_spc TEXT,
      nvc_kor_full TEXT,
      nvc_eng_full TEXT,
      nvc_spc_full TEXT,
      nvc2_kor TEXT,
      nvc2_eng TEXT,
      nvc2_spc TEXT,
      nvc2_kor_full TEXT,
      nvc2_eng_full TEXT,
      nvc2_spc_full TEXT
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');

  -- TEST 11
  SELECT row_eq(
    $$SELECT * FROM ft10 LIMIT 1$$,
    ROW('가'::TEXT, 'a'::TEXT, '!'::TEXT,
      '가나다라마바사아자차카타파하'::TEXT,
      'abcdefghijklmnopqrstuvwxyz'::TEXT,
      '!@#$%^&*()<>/\\'''::TEXT,
      '가'::TEXT, 'a'::TEXT, '!'::TEXT,
      '가나다라마바사아자차카타파하'::TEXT,
      'abcdefghijklmnopqrstuvwxyz'::TEXT,
      '!@#$%^&*()<>/\\'''::TEXT),
    'TEXT 타입 컬럼 결과값 가져오는 것 검증'
  );

  CREATE FOREIGN TABLE char_err_ft10 (
      nvc_kor CHAR(1),
      nvc_eng CHAR(1),
      nvc_spc CHAR(1),
      nvc_kor_full CHAR(1),
      nvc_eng_full CHAR(1),
      nvc_spc_full CHAR(1),
      nvc2_kor CHAR(1),
      nvc2_eng CHAR(1),
      nvc2_spc CHAR(1),
      nvc2_kor_full CHAR(1),
      nvc2_eng_full CHAR(1),
      nvc2_spc_full CHAR(1)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');

  -- TEST 12
  SELECT throws_ok(
    'SELECT * FROM char_err_ft10',
    '22001',
    'value too long for type character(1)',
    'CHAR(1) 타입 컬럼으로 긴 문자열을 받아오려는 경우 에러 발생 여부 확인'
  );

  CREATE FOREIGN TABLE varchar_err_ft10 (
      nvc_kor VARCHAR(1),
      nvc_eng VARCHAR(1),
      nvc_spc VARCHAR(1),
      nvc_kor_full VARCHAR(1),
      nvc_eng_full VARCHAR(1),
      nvc_spc_full VARCHAR(1),
      nvc2_kor VARCHAR(1),
      nvc2_eng VARCHAR(1),
      nvc2_spc VARCHAR(1),
      nvc2_kor_full VARCHAR(1),
      nvc2_eng_full VARCHAR(1),
      nvc2_spc_full VARCHAR(1)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');

  -- TEST 13
  SELECT throws_ok(
    'SELECT * FROM varchar_err_ft10',
    '22001',
    'value too long for type character varying(1)',
    'VARCHAR(1) 타입 컬럼으로 긴 문자열을 받아오려는 경우 에러 발생 여부 확인'
  );

  CREATE FOREIGN TABLE ft11 (
      rwid TEXT
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't1');

  -- TEST 14
  SELECT is(
    (SELECT * FROM ft11),
    'AAAArFAAAAAACTFAAA',
    'ROWID 데이터 조회 확인'
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
