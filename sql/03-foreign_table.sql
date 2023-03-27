-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(7);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  -- TEST 1: table_name 옵션 없이 CREATE FOREIGN TABLE 실행 - 에러 발생 확인
  CREATE FOREIGN TABLE ft1 (
      c0 int,
      c1 int NOT NULL,
      c2 int NOT NULL,
      c3 text,
      c4 timestamptz,
      c5 timestamp,
      c6 varchar(10),
      c7 char(10) default 'opensql'
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER);

  SELECT throws_ok('SELECT * FROM ft1');

  -- TEST 2: 존재하지 않는 owner_name 옵션으로 CREATE FOREIGN TABLE 실행 - 에러 발생 확인
  CREATE FOREIGN TABLE ft2 (
      c0 int,
      c1 int NOT NULL,
      c2 int NOT NULL,
      c3 text,
      c4 timestamptz,
      c5 timestamp,
      c6 varchar(10),
      c7 char(10) default 'opensql'
  ) SERVER server_name OPTIONS (owner_name 'dummy_user', table_name 'ft_test');

  SELECT throws_ok('SELECT * FROM ft2');

  -- TEST 3: owner_name 옵션 없이 CREATE FOREIGN TABLE 실행 - 에러 발생 확인
  CREATE FOREIGN TABLE ft3 (
      c0 int,
      c1 int NOT NULL,
      c2 int NOT NULL,
      c3 text,
      c4 timestamptz,
      c5 timestamp,
      c6 varchar(10),
      c7 char(10) default 'opensql'
  ) SERVER server_name OPTIONS (table_name 'ft_test');

  SELECT throws_ok('SELECT * FROM ft_test');

  -- TEST 4: 존재하지 않는 table_name 옵션으로 CREATE FOREIGN TABLE 실행 - 에러 발생 확인
  CREATE FOREIGN TABLE ft4 (
      c0 int,
      c1 int NOT NULL,
      c2 int NOT NULL,
      c3 text,
      c4 timestamptz,
      c5 timestamp,
      c6 varchar(10),
      c7 char(10) default 'opensql'
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'dummy_table');

  SELECT throws_ok('SELECT * FROM ft4');

  CREATE FOREIGN TABLE ft5 (
      c0 int,
      c1 int NOT NULL,
      c2 int NOT NULL,
      c3 text,
      c4 timestamptz,
      c5 timestamp,
      c6 varchar(10),
      c7 char(10) default 'opensql'
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'ft_test');

  -- TEST 5: ALTER FOREIGN TABLE 정상 동작 확인
  SELECT lives_ok('ALTER FOREIGN TABLE ft5 DROP COLUMN c0');

  -- TEST 6: 존재하지 않는 Column을 DROP 하려는 경우 에러 발생 확인
  SELECT throws_ok('ALTER FOREIGN TABLE ft5 DROP COLUMN cx');

  -- TEST 7: 정상 동작 확인
  SELECT lives_ok('SELECT * FROM ft5');

  SELECT * FROM finish();
ROLLBACK;
