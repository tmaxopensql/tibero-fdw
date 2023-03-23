-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(4);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE FOREIGN TABLE tx_ft1 (
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

  CREATE FOREIGN TABLE tx_ft2 (
      nb_default NUMERIC,
      nb_380 NUMERIC(38,0),
      nb_38191 NUMERIC(38,19),
      nb_38192 NUMERIC(38,19),
      nb_ltm NUMERIC(38,19),
      nb_gtm NUMERIC(130,130),
      flt FLOAT
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't2');

  -- TEST 1: Foreign Table 2개 한번에 조회
  SELECT lives_ok(
    'SELECT * FROM tx_ft1, tx_ft2;'
  );

  -- TEST 2:
  SELECT lives_ok(
    'SELECT * FROM tx_ft1;'
  );

  -- TEST 3:
  SELECT lives_ok(
    'SELECT * FROM tx_ft2;'
  );

  CREATE FOREIGN TABLE idx_ft1 (
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

  -- TEST 4: Foreign Table 대상으로 Index 생성하려고 할 때 에러 발생 검증
  SELECT throws_ok(
    'CREATE INDEX ON idx_ft1(nvc_kor)',
    '42809',
    'cannot create index on foreign table "idx_ft1"'
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
