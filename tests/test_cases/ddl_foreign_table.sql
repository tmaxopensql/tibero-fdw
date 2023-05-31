-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(6);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :'TIBERO_HOST', port :'TIBERO_PORT', dbname :'TIBERO_DB');

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :'TIBERO_USER', password :'TIBERO_PASS');

  CREATE FOREIGN TABLE ft1 (
      c0 int,
      c1 int NOT NULL,
      c2 int NOT NULL,
      c3 text,
      c4 timestamptz,
      c5 timestamp,
      c6 varchar(10),
      c7 char(10) default 'opensql'
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'dummy_table');

  -- TEST 1: Check error is thrown when accessing foreign table created with an invalid table_name option
  SELECT throws_ok('SELECT * FROM ft1');

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

  -- TEST 2: Check error is thrown when accessing foreign table created with an invalid owner_name option
  SELECT throws_ok('SELECT * FROM ft2');

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

  -- TEST 3: Check error is thrown when accessing foreign table created without a valid owner_name option
  SELECT throws_ok(
    'SELECT * FROM ft_test', 
    '42P01', 
    'relation "ft_test" does not exist',
    'Check an error is thrown when accessing foreign table created without a valid owner_name option'
  );

  CREATE FOREIGN TABLE ft4 (
      c0 int,
      c1 int NOT NULL,
      c2 int NOT NULL,
      c3 text,
      c4 timestamptz,
      c5 timestamp,
      c6 varchar(10),
      c7 char(10) default 'opensql'
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'ft_test');

  -- TEST 4
  SELECT lives_ok(
    'ALTER FOREIGN TABLE ft4 DROP COLUMN c0',
    'Check ALTER FOREIGN TABLE command works'
  );

  -- TEST 5: Check error is thrown when trying to DROP non-existent column from foreign table
  SELECT throws_ok(
    'ALTER FOREIGN TABLE ft4 DROP COLUMN cx',
    '42703',
    'column "cx" of relation "ft4" does not exist',
    'Check an error is thrown when trying to DROP a non-existent column on the foreign table'
  );

  -- TEST 6
  SELECT lives_ok(
    'SELECT * FROM ft4',
    'Check SELECT FROM foreign table'
  );

  SELECT * FROM finish();
ROLLBACK;
