-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(29);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE FOREIGN TABLE d_ft1 (
      dt DATE,
      dt_bc9999 DATE,
      dt_ad9999 DATE,
      dt_detail DATE,
      ts TIMESTAMP,
      ts1 TIMESTAMP,
      ts6 TIMESTAMP,
      ts_bc TIMESTAMP,
      ts_ad TIMESTAMP,
      tsz TIMESTAMP WITH TIME ZONE,
      tsz1 TIMESTAMP WITH TIME ZONE,
      tsz6 TIMESTAMP WITH TIME ZONE,
      tsz_bc TIMESTAMP WITH TIME ZONE,
      tsz_ad TIMESTAMP WITH TIME ZONE,
      tslz TIMESTAMP WITH TIME ZONE,
      tslz1 TIMESTAMP WITH TIME ZONE,
      tslz6 TIMESTAMP WITH TIME ZONE,
      tslz_bc TIMESTAMP WITH TIME ZONE,
      tslz_ad TIMESTAMP WITH TIME ZONE
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't3');

  -- TEST 1
  SELECT is(
    (SELECT TO_CHAR(dt, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000',
    'Verify compatibility of Tibero DATE and Postgres DATE'
  );

  -- TEST skipped. Refer to known_issue/ for detail.
  SELECT pass();
  SELECT pass();
  /*
  -- TEST 2:
  SELECT is(
    (SELECT TO_CHAR(dt_bc9999, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    'bc9999-01-01 00:00:00.000000'
  );

  -- TEST 3:
  SELECT is(
    (SELECT TO_CHAR(dt_ad9999, 'bcYYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    'ad9999-01-01 00:00:00.000000'
  );
  */

  -- TEST 4
  SELECT is(
    (SELECT TO_CHAR(dt_detail, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 00:00:00.000000',
    'Verify compatibility of Tibero DATE and Postgres DATE with .FF6 format'
  );

  -- TEST 5
  SELECT is(
    (SELECT TO_CHAR(ts, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 12:34:56.123456',
    'Verify compatibility of Tibero TIMESTAMP and Postgres TIMESTAMP'
  );

  -- TEST 6
  SELECT is(
    (SELECT TO_CHAR(ts1, 'YYYY-MM-DD HH24:MI:SS.FF1') FROM d_ft1),
    '2023-01-01 12:34:56.1',
    'Verify compatibility of Tibero DATE and Postgres DATE with .FF1 format'
  );

  -- TEST 7
  SELECT is(
    (SELECT TO_CHAR(ts6, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 12:34:56.123456',
    'Verify compatibility of Tibero TIMESTAMP and Postgres TIMESTAMP with .FF6 format'
  );

  -- TEST skipped. Refer to known_issue/ for detail.
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
    (SELECT TO_CHAR(tsz, 'YYYY-MM-DD HH24:MI:SS.FF6 TZ') FROM d_ft1),
    '2023-01-01 21:34:56.123456 KST',
    'Verify compatibility of Tibero TIMESTAMP WITH TIME ZONE and Postgres TIMESTAMP WITH TIME ZONE'
  );

  -- TEST 11:
  SELECT is(
    (SELECT TO_CHAR(tsz1, 'YYYY-MM-DD HH24:MI:SS.FF1 TZ') FROM d_ft1),
    '2023-01-01 21:34:56.1 KST',
    'Verify compatibility of Tibero TIMESTAMP WITH TIME ZONE and Postgres TIMESTAMP WITH TIME ZONE with .FF1 format'
  );

  -- TEST 12:
  SELECT is(
    (SELECT TO_CHAR(tsz6, 'YYYY-MM-DD HH24:MI:SS.FF6 TZ') FROM d_ft1),
    '2023-01-01 21:34:56.123456 KST',
    'Verify compatibility of Tibero TIMESTAMP WITH TIME ZONE and Postgres TIMESTAMP WITH TIME ZONE with .FF6 format'
  );

  -- TEST skipped. Refer to known_issue/ for detail.
  SELECT pass();
  SELECT pass();

  /*
  -- TEST 13:
  SELECT is(
    (SELECT TO_CHAR(tsz_bc, 'bcYYYY-MM-DD HH24:MI:SS.FF6 TZ') FROM d_ft1),
    'bc9999-01-01 21:34:56.123456 KST'
  );

  -- TEST 14:
  SELECT is(
    (SELECT TO_CHAR(tsz_ad, 'bcYYYY-MM-DD HH24:MI:SS.FF6 TZ') FROM d_ft1),
    'ad9999-01-01 21:34:56.123456 KST'
  );
  */

  -- TEST 15
  SELECT is(
    (SELECT TO_CHAR(tslz, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 21:34:56.123456',
    'Verify compatibility of Tibero TIMESTAMP WITH LOCAL TIME ZONE and Postgres TIMESTAMP WITH LOCAL TIME ZONE'
  );

  -- TEST 16
  SELECT is(
    (SELECT TO_CHAR(tslz1, 'YYYY-MM-DD HH24:MI:SS.FF1') FROM d_ft1),
    '2023-01-01 21:34:56.1',
    'Verify compatibility of Tibero TIMESTAMP WITH LOCAL TIME ZONE and Postgres TIMESTAMP WITH LOCAL TIME ZONE with .FF1 format'
  );

  -- TEST 17
  SELECT is(
    (SELECT TO_CHAR(tslz6, 'YYYY-MM-DD HH24:MI:SS.FF6') FROM d_ft1),
    '2023-01-01 21:34:56.123456',
    'Verify compatibility of Tibero TIMESTAMP WITH LOCAL TIME ZONE and Postgres TIMESTAMP WITH LOCAL TIME ZONE with .FF6 format'
  );

  -- TEST skipped. Refer to known_issue/ for detail.
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

  CREATE FOREIGN TABLE d_ft2 (
      iytm INTERVAL YEAR TO MONTH,
      idts INTERVAL DAY TO SECOND
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't4');

  -- TEST 20
  SELECT lives_ok(
    'SELECT * FROM d_ft2',
    'Verify compatibility between Tibero INTERVAL(9) TO MONTH and Postgres INTERVAL YEAR TO MONTH'
  );

  -- TEST 21
  SELECT is(
    (SELECT COUNT(*) FROM d_ft2 WHERE iytm = INTERVAL '178000000-11' YEAR TO MONTH ),
    10::BIGINT,
    'Verify INTERVAL YEAR TO MONTH compatibility of values with partial months defined'
  );

  CREATE FOREIGN TABLE d_ft3 (
      dt TIMESTAMP,
      dt_bc9999 TIMESTAMP,
      dt_ad9999 TIMESTAMP,
      dt_detail TIMESTAMP,
      ts TIMESTAMP,
      ts1 TIMESTAMP,
      ts6 TIMESTAMP,
      ts_bc TIMESTAMP,
      ts_ad TIMESTAMP,
      tsz TIMESTAMP,
      tsz1 TIMESTAMP,
      tsz6 TIMESTAMP,
      tsz_bc TIMESTAMP,
      tsz_ad TIMESTAMP,
      tslz TIMESTAMP,
      tslz1 TIMESTAMP,
      tslz6 TIMESTAMP,
      tslz_bc TIMESTAMP,
      tslz_ad TIMESTAMP
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't3');

  -- TEST 22:
  SELECT lives_ok(
    'SELECT * FROM d_ft3',
    'Verify compatibility between Tibero DATE, TIMESTAMP types and Postgres TIMESTAMP'
  );

  CREATE FOREIGN TABLE d_ft4 (
      dt CHAR(20),
      dt_bc9999 CHAR(20),
      dt_ad9999 CHAR(20),
      dt_detail CHAR(20),
      ts CHAR(30),
      ts1 CHAR(30),
      ts6 CHAR(30),
      ts_bc CHAR(30),
      ts_ad CHAR(30),
      tsz CHAR(40),
      tsz1 CHAR(40),
      tsz6 CHAR(40),
      tsz_bc CHAR(40),
      tsz_ad CHAR(40),
      tslz CHAR(30),
      tslz1 CHAR(30),
      tslz6 CHAR(30),
      tslz_bc CHAR(30),
      tslz_ad CHAR(30)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't3');  

  -- TEST 23:
  SELECT lives_ok(
    'SELECT * FROM d_ft4',
    'Verify compatibility between Tibero DATE, TIMESTAMP types and Postgres CHAR type'
  );

  CREATE FOREIGN TABLE d_ft5 (
      dt VARCHAR(20),
      dt_bc9999 VARCHAR(20),
      dt_ad9999 VARCHAR(20),
      dt_detail VARCHAR(20),
      ts VARCHAR(30),
      ts1 VARCHAR(30),
      ts6 VARCHAR(30),
      ts_bc VARCHAR(30),
      ts_ad VARCHAR(30),
      tsz VARCHAR(40),
      tsz1 VARCHAR(40),
      tsz6 VARCHAR(40),
      tsz_bc VARCHAR(40),
      tsz_ad VARCHAR(40),
      tslz VARCHAR(30),
      tslz1 VARCHAR(30),
      tslz6 VARCHAR(30),
      tslz_bc VARCHAR(30),
      tslz_ad VARCHAR(30)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't3');

  -- TEST 24:
  SELECT lives_ok(
    'SELECT * FROM d_ft5',
    'Verify compatibility between Tibero DATE, TIMESTAMP types and Postgres VARCHAR type'
  );

  CREATE FOREIGN TABLE d_ft6 (
      dt TEXT,
      dt_bc9999 TEXT,
      dt_ad9999 TEXT,
      dt_detail TEXT,
      ts TEXT,
      ts1 TEXT,
      ts6 TEXT,
      ts_bc TEXT,
      ts_ad TEXT,
      tsz TEXT,
      tsz1 TEXT,
      tsz6 TEXT,
      tsz_bc TEXT,
      tsz_ad TEXT,
      tslz TEXT,
      tslz1 TEXT,
      tslz6 TEXT,
      tslz_bc TEXT,
      tslz_ad TEXT
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't3');

  -- TEST 25:
  SELECT lives_ok(
    'SELECT * FROM d_ft6',
    'Verify compatibility between Tibero DATE, TIMESTAMP types and Postgres TEXT type'
  );

  CREATE FOREIGN TABLE d_ft7 (
      dt DATE,
      dt_bc9999 DATE,
      dt_ad9999 DATE,
      dt_detail DATE,
      ts DATE,
      ts1 DATE,
      ts6 DATE,
      ts_bc DATE,
      ts_ad DATE,
      tsz DATE,
      tsz1 DATE,
      tsz6 DATE,
      tsz_bc DATE,
      tsz_ad DATE,
      tslz DATE,
      tslz1 DATE,
      tslz6 DATE,
      tslz_bc DATE,
      tslz_ad DATE
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't3');

  -- TEST 26:
  SELECT lives_ok(
    'SELECT * FROM d_ft7',
    'Verify compatibility between Tibero DATE, TIMESTAMP types and Postgres DATE type'
  );

  ------------------------------------------------------
  -- SELECT Data type valid matching range INTERVAL
  ------------------------------------------------------

  CREATE FOREIGN TABLE d_ft8 (
    iytm    CHAR(50),
    idts    CHAR(50)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't4');

  -- TEST 27:
  SELECT lives_ok(
    'SELECT * FROM d_ft8',
    'Verify compatibility between Tibero INTERVAL type and Postgres CHAR type'
  );

  CREATE FOREIGN TABLE d_ft9 (
    iytm    VARCHAR(50),
    idts    VARCHAR(50)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't4');

  -- TEST 28:
  SELECT lives_ok(
    'SELECT * FROM d_ft9',
    'Verify compatibility between Tibero INTERVAL type and Postgres VARCHAR type'
  );

  CREATE FOREIGN TABLE d_ft10 (
    iytm    TEXT,
    idts    TEXT
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't4');

  -- TEST 29:
  SELECT lives_ok(
    'SELECT * FROM d_ft10',
    'Verify compatibility between Tibero INTERVAL type and Postgres TEXT type'
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
