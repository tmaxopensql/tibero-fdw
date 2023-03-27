-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(6);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER fetch_server_100 FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB, fetch_size '100');

  CREATE USER MAPPING FOR current_user
    SERVER fetch_server_100
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  -- TEST 1
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_server
      WHERE srvname = 'fetch_server_100' AND
            srvoptions @> array['fetch_size=100'])::integer,
    1,
    'CREATE SERVER 명령어의 fetch_size 옵션 pg_foreign_server 테이블 카탈로그의 srvoptions 열에 반영되는 것 확인'
  );

  -- TEST 2
  ALTER SERVER fetch_server_100 OPTIONS (SET fetch_size '101');
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_server
      WHERE srvname = 'fetch_server_100' AND
            srvoptions @> array['fetch_size=100'])::integer,
    0,
    'ALTER SERVER 명령어로 fetch_size 옵션 변경한 내용 반영되는 것 확인'
  );

  -- TEST 3
  ALTER SERVER fetch_server_100 OPTIONS (SET fetch_size '101');
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_server
      WHERE srvname = 'fetch_server_100' AND
            srvoptions @> array['fetch_size=101'])::integer,
    1,
    'ALTER SERVER 명령어로 fetch_size 옵션 변경한 내용 반영되는 것 확인'
  );

  -- TEST 4
  CREATE FOREIGN TABLE fetch_table_1000 ( 
      x int 
  ) SERVER fetch_server_100 OPTIONS ( fetch_size '1000' );

  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_table
      WHERE ftrelid = 'fetch_table_1000'::regclass
        AND ftoptions @> array['fetch_size=1000'])::integer,
    1,
    'CREATE FOREIGN TABLE 명령어의 fetch_size 옵션 pg_foreign_table 테이블 카탈로그의 ftoptions 열에 반영되는 것 확인');

  ALTER FOREIGN TABLE fetch_table_1000 OPTIONS (SET fetch_size '1001');

  -- TEST 5
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_table
      WHERE ftrelid = 'fetch_table_1000'::regclass
        AND ftoptions @> array['fetch_size=1000'])::integer,
    0,
    'ALTER FOREIGN TABLE 명령어로 fetch_size 옵션 변경한 내용 반영되는 것 확인'
  );

  -- TEST 6
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_table
      WHERE ftrelid = 'fetch_table_1000'::regclass
        AND ftoptions @> array['fetch_size=1001'])::integer,
    1,
    'ALTER FOREIGN TABLE 명령어로 fetch_size 옵션 변경한 내용 반영되는 것 확인'
  );

  SELECT * FROM finish();
ROLLBACK;
