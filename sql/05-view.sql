-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(1);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE FOREIGN TABLE fvt1(
      col_char CHAR(10),
      col_varchar VARCHAR(20),
      col_varchar2 VARCHAR(20),
      col_nchar CHAR(10),
      col_nvarchar VARCHAR(20),
      col_nvarchar2 VARCHAR(20),
      col_rowid TEXT,
      col_number NUMERIC,
      col_number_n_m NUMERIC(5, 2),
      col_float REAL,
      col_date DATE,
      col_timestamp TIMESTAMP,
      col_timestamp_tz TIMESTAMP WITH TIME ZONE,
      col_interval_ym INTERVAL,
      col_interval_ds INTERVAL
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'v_test');

  -- TEST 1
  SELECT lives_ok('
    SELECT * FROM fvt1',
    '원격 서버에 생성한 VIEW를 Foreign Table로 조회 검증'
  );

  SELECT * FROM finish();
ROLLBACK;
