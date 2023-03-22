-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(13);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE FOREIGN TABLE just_conn_test_table (
    test    char(50)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'just_conn_test_table');

  CREATE FOREIGN TABLE charset_check (
      characterset_name         char(50),
      nchar_characterset_name   char(50)
  ) SERVER server_name OPTIONS (owner_name 'sys', table_name '_VT_NLS_CHARACTER_SET');

  -- TEST 1 ~ 2: test remote Tibero db charset
  SELECT alike(
    (SELECT characterset_name FROM charset_check LIMIT 1),
    'UTF%',
    '원격 Tibero 서버의 CHARACTERSET_NAME 확인'
  );

  SELECT alike(
    (SELECT nchar_characterset_name FROM charset_check LIMIT 1),
    'UTF%',
    '원격 Tibero 서버의 nchar CHARACTERSET_NAME 확인'
  );

  -- TEST 1: everything works as intended
  SELECT lives_ok('SELECT * FROM just_conn_test_table;');

  -- TEST 2: Foreign server with only host option - throws error
  ALTER SERVER server_name OPTIONS (DROP port);
  ALTER SERVER server_name OPTIONS (DROP dbname);
  SELECT throws_ok('SELECT * FROM just_conn_test_table;');

  -- TEST 3: Foreign server with only dbname option - throws error
  ALTER SERVER server_name OPTIONS (DROP host);
  ALTER SERVER server_name OPTIONS (ADD dbname :TIBERO_DB);
  SELECT throws_ok('SELECT * FROM just_conn_test_table;');

  -- TEST 4: Foreign server with only port option - throws error
  ALTER SERVER server_name OPTIONS (DROP dbname);
  ALTER SERVER server_name OPTIONS (ADD port :TIBERO_PORT);
  SELECT throws_ok('SELECT * FROM just_conn_test_table');

  -- TEST 5: Foreign server without host option - throws error
  ALTER SERVER server_name OPTIONS (ADD dbname :TIBERO_DB);
  SELECT throws_ok('SELECT * FROM just_conn_test_table');

  -- TEST 6: Foreign server without port option - throws error
  ALTER SERVER server_name OPTIONS (DROP port);
  ALTER SERVER server_name OPTIONS (ADD host :TIBERO_HOST);
  SELECT throws_ok('SELECT * FROM just_conn_test_table');

  -- TEST 7: Foreign server without dbname option - throws error
  ALTER SERVER server_name OPTIONS (DROP dbname);
  ALTER SERVER server_name OPTIONS (ADD port :TIBERO_PORT);
  SELECT throws_ok('SELECT * FROM just_conn_test_table');

  -- TEST 8: everything works as intended
  ALTER SERVER server_name OPTIONS (ADD dbname :TIBERO_DB);
  SELECT lives_ok('SELECT * FROM just_conn_test_table;');

  -- TEST 9: User mapping without username - throws error
  ALTER USER MAPPING FOR CURRENT_USER SERVER server_name
    OPTIONS (DROP username);
  SELECT throws_ok('SELECT * FROM just_conn_test_table;');

  -- TEST 10: User mapping without password - throws error
  ALTER USER MAPPING FOR CURRENT_USER SERVER server_name
    OPTIONS (ADD username :TIBERO_USER);
  ALTER USER MAPPING FOR CURRENT_USER SERVER server_name
    OPTIONS (DROP password);
  SELECT throws_ok('SELECT * FROM just_conn_test_table;');

  -- TEST 11: all good
  ALTER USER MAPPING FOR CURRENT_USER SERVER server_name
    OPTIONS (ADD password :TIBERO_PASS);
  SELECT lives_ok('SELECT * FROM just_conn_test_table;');

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
