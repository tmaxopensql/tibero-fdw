-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT * FROM no_plan();
  --SELECT plan(28);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :'TIBERO_HOST', port :'TIBERO_PORT', dbname :'TIBERO_DB');

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :'TIBERO_USER', password :'TIBERO_PASS');

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
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 't3');

  -- Below cases deal with the 'sYYYY'-formatted signed year of Tibero DBMS, 
  -- which will be supported only after pushdown for PROJECTION is implemented.
  -- For now the cases are stored here and not executed as part of main TAP testing.
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

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
