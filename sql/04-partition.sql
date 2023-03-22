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

  -- TEST 1: CREATE FOREIGN TABLE for partitioned remote table (by range)
  CREATE FOREIGN TABLE range_parent_pt (
      id INT,
      dates DATE,
      amounts NUMERIC(10,2)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't_test1');

  SELECT lives_ok('SELECT * FROM range_parent_pt');

  -- TEST 1: CREATE FOREIGN TABLE for partitioned remote table (by range)
  CREATE FOREIGN TABLE list_parent_pt (
      id INT,
      name VARCHAR(50),
      region VARCHAR(20)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't_test2');

  SELECT lives_ok('SELECT * FROM list_parent_pt');

  -- TEST 1: CREATE FOREIGN TABLE for partitioned remote table (by range)
  CREATE FOREIGN TABLE hash_parent_pt (
      id INT,
      name VARCHAR(50),
      department VARCHAR(20)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 't_test3');

  SELECT lives_ok('SELECT * FROM hash_parent_pt');

  SELECT * FROM finish();
ROLLBACK;
