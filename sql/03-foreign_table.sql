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

  -- TEST 1: CREATE FOREIGN TABLE without table_name - throws error
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

  -- TEST 2: CREATE FOREIGN TABLE with non-existent owner_name - throws error
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

  -- TEST 3: CREATE FOREIGN TABLE without owner_name - throws error
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

  -- TEST 4: CREATE FOREIGN TABLE with non-existent table name - throws error
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

  -- TEST 5~7: CREATE FOREIGN TABLE with proper owner_name and table_name reference
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

  -- TEST 5: ALTER FOREIGN TABLE working as intended
  SELECT lives_ok('ALTER FOREIGN TABLE ft5 DROP COLUMN c0');

  -- TEST 6: Try to drop a non-existent column - throws error
  SELECT throws_ok('ALTER FOREIGN TABLE ft5 DROP COLUMN cx');

  -- TEST 7: everything back to normal
  SELECT lives_ok('SELECT * FROM ft5');

  SELECT * FROM finish();
ROLLBACK;
