-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(20);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  -- TEST 1:
  CREATE FOREIGN TABLE d_ft1 (
      dt DATE,
      dt_bc9999 DATE,
      dt_ad9999 DATE,
      dt_detail DATE,
      ts TIMESTAMP,
      ts1 TIMESTAMP,
      ts6 TIMESTAMP,
      ts9 TIMESTAMP,
      ts_bc TIMESTAMP,
      ts_ad TIMESTAMP,
      tsz TIMESTAMP WITH TIME ZONE,
      tsz1 TIMESTAMP WITH TIME ZONE,
      tsz6 TIMESTAMP WITH TIME ZONE,
      tsz9 TIMESTAMP WITH TIME ZONE,
      tsz_bc TIMESTAMP WITH TIME ZONE,
      tsz_ad TIMESTAMP WITH TIME ZONE,
      tslz TIMESTAMP WITH TIME ZONE,
      tslz1 TIMESTAMP WITH TIME ZONE,
      tslz6 TIMESTAMP WITH TIME ZONE,
      tslz9 TIMESTAMP WITH TIME ZONE,
      tslz_bc TIMESTAMP WITH TIME ZONE,
      tslz_ad TIMESTAMP WITH TIME ZONE
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't3');

  SELECT is(
    (SELECT TO_CHAR(dt, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000'
  );


  SELECT pass();
  SELECT pass();

  /*
  SELECT is(
    (SELECT TO_CHAR(dt_bc9999, 'A.D. YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    'bc9999-01-01 00:00:00.000000'
  );

  SELECT is(
    (SELECT TO_CHAR(dt_ad9999, 'A.D. YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    'ad9999-01-01 00:00:00.000000'
  );
  */

  SELECT is(
    (SELECT TO_CHAR(dt_detail, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000'
  );

  SELECT is(
    (SELECT TO_CHAR(ts, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 12:34:56.123456'
  );

  SELECT is(
    (SELECT TO_CHAR(ts1, 'YYYY-MM-DD HH24:MI:SS.FF1') FROM d_ft1),
    '2023-01-01 12:34:56.1'
  );

  SELECT is(
    (SELECT TO_CHAR(ts6, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 12:34:56.123456'
  );

  SELECT pass();
  SELECT pass();
  /*
  -- TEST 8:
  SELECT is(
    (SELECT TO_CHAR(ts_bc, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    'bc9999-01-01 12:34:56.123456'
  );

  -- TEST 9:
  SELECT is(
    (SELECT TO_CHAR(ts_ad, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    'ad9999-01-01 12:34:56.123456'
  );
  */

  -- TEST 10:
  SELECT is(
    (SELECT TO_CHAR(tsz, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 12:34:56.123456'
  );

  -- TEST 11:
  SELECT is(
    (SELECT TO_CHAR(tsz1, 'YYYY-MM-DD HH24:MI:SS.FF1') FROM d_ft1),
    '2023-01-01 12:34:56.1'
  );

  -- TEST 12:
  SELECT is(
    (SELECT TO_CHAR(tsz6, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 12:34:56.123456'
  );

  SELECT pass();
  SELECT pass();
  /*
  -- TEST 13:
  SELECT is(
    (SELECT TO_CHAR(tsz_bc, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000'
  );

  -- TEST 14:
  SELECT is(
    (SELECT TO_CHAR(tsz_ad, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000'
  );
  */

  -- TEST 15: WITH LOCAL TIME ZONE 옵션으로 생성된 Column 변환 확인
  -- KST를 항상 전제하는 건지? (Tibero에서 삽입할 때 12:34:56, 변환하면 21:34:56)
  SELECT is(
    (SELECT TO_CHAR(tslz, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 21:34:56.123456'
  );

  -- TEST 16
  SELECT is(
    (SELECT TO_CHAR(tslz1, 'YYYY-MM-DD HH24:MI:SS.FF1') FROM d_ft1),
    '2023-01-01 21:34:56.1'
  );

  -- TEST 17
  SELECT is(
    (SELECT TO_CHAR(tslz6, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 21:34:56.123456'
  );

  SELECT pass();
  SELECT pass();

  /*
  -- TEST 18:
  SELECT is(
    (SELECT TO_CHAR(tslz_bc, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000'
  );

  -- TEST 19:
  SELECT is(
    (SELECT TO_CHAR(tslz_ad, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000'
  );
  */

  -- INTERVAL 관련 테스트 케이스

  CREATE FOREIGN TABLE d_ft2 (
      iytm INTERVAL YEAR TO MONTH,
      idts INTERVAL DAY TO SECOND
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't4');

  -- TEST 20
  SELECT lives_ok(
    'SELECT * FROM d_ft2'
  );


  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
