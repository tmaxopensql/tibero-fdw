-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(3);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE FOREIGN TABLE range_parent_pt (
      id INT,
      dates DATE,
      amounts NUMERIC(10,2)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't_test1');

  -- TEST 1: RANGE(DATE) 조건으로 파티션 된 Foreign Table 조회
  SELECT lives_ok('SELECT * FROM range_parent_pt');

  CREATE FOREIGN TABLE list_parent_pt (
      id INT,
      name VARCHAR(50),
      region VARCHAR(20)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't_test2');

  -- TEST 2: LIST(VARCHAR) 조건으로 파티션 된 Foreign Table 조회
  SELECT lives_ok('SELECT * FROM list_parent_pt');

  CREATE FOREIGN TABLE hash_parent_pt (
      id INT,
      name VARCHAR(50),
      department VARCHAR(20)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't_test3');

  -- TEST 3: HASH(INT) 조건으로 파티션 된 Foreign Table 조회
  SELECT lives_ok('SELECT * FROM hash_parent_pt');

  SELECT * FROM finish();
ROLLBACK;
