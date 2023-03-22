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

  -- TEST 1: Query pg_foreign_server table to check fetch_size option is properly applied
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_server
      WHERE srvname = 'fetch_server_100' AND
            srvoptions @> array['fetch_size=100'])::integer,
    1);

  -- TEST 2: Alter fetch_size option works properly
  ALTER SERVER fetch_server_100 OPTIONS (SET fetch_size '101');
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_server
      WHERE srvname = 'fetch_server_100' AND
            srvoptions @> array['fetch_size=100'])::integer,
    0);

  -- TEST 3: Alter fetch_size option works properly
  ALTER SERVER fetch_server_100 OPTIONS (SET fetch_size '101');
  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_server
      WHERE srvname = 'fetch_server_100' AND
            srvoptions @> array['fetch_size=101'])::integer,
    1);

  -- TEST 4: CREATE FOREIGN TABLE with fetch_size option
  CREATE FOREIGN TABLE fetch_table_1000 ( 
      x int 
  ) SERVER fetch_server_100 OPTIONS ( fetch_size '1000' );

  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_table
      WHERE ftrelid = 'fetch_table_1000'::regclass
        AND ftoptions @> array['fetch_size=1000'])::integer,
    1);

  -- TEST 5 ~ 6: ALTER FOREIGN TABLE fetch_size option
  ALTER FOREIGN TABLE fetch_table_1000 OPTIONS (SET fetch_size '1001');

  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_table
      WHERE ftrelid = 'fetch_table_1000'::regclass
        AND ftoptions @> array['fetch_size=1000'])::integer,
    0);

  SELECT is(
    (SELECT COUNT(*) FROM pg_catalog.pg_foreign_table
      WHERE ftrelid = 'fetch_table_1000'::regclass
        AND ftoptions @> array['fetch_size=1001'])::integer,
    1);

  SELECT * FROM finish();
ROLLBACK;
