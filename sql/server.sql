-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(14);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  -- TEST 1
  SELECT throws_ok('
    CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (port ''0000'', dbname ''invalid_dbname'');',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to create a foreign server without a valid host option'
  );

  -- TEST 2
  SELECT throws_ok('
    CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host ''localhost'', dbname ''invalid_dbname'');',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to create a foreign server without a valid port option'
  );

  -- TEST 3
  SELECT throws_ok('
    CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host ''localhost'', port ''0000'');',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to create a foreign server without a valid dbname option'
  );

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

  -- TEST 4
  SELECT lives_ok('
    SELECT * FROM just_conn_test_table;',
    'Test connection with valid foreign server options'
  );

  -- TEST 5
  SELECT alike(
    (SELECT characterset_name FROM charset_check LIMIT 1),
    'UTF%',
    'Test CHARACTERSET_NAME of remote Tibero server - must be of Unicode family'
  );

  -- TEST 6
  SELECT alike(
    (SELECT nchar_characterset_name FROM charset_check LIMIT 1),
    'UTF%',
    'Test NCHAR_CHARACTERSET_NAME of remote Tibero server - must be of Unicode family'
  );

  -- TEST 7
  SELECT throws_ok(
    'ALTER SERVER server_name OPTIONS (DROP host);',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to DROP host option from foreign server');

  -- TEST 8
  SELECT throws_ok(
    'ALTER SERVER server_name OPTIONS (DROP port);',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to DROP port option from foreign server');

  -- TEST 9
  SELECT throws_ok(
    'ALTER SERVER server_name OPTIONS (DROP dbname);',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to DROP dbname option from foreign server');

  -- TEST 10
  SELECT throws_ok(
    'ALTER SERVER server_name OPTIONS (ADD host ''invalid_host'');',
    '42710',
    'option "host" provided more than once',
    'Check an error is thrown when trying to add a host option to foreign server'
  );

  -- TEST 11
  SELECT throws_ok(
    'ALTER SERVER server_name OPTIONS (ADD port ''0000'');',
    '42710',
    'option "port" provided more than once',
    'Check an error is thrown when trying to add a port option to foreign server'
  );

  -- TEST 12
  SELECT throws_ok(
    'ALTER SERVER server_name OPTIONS (ADD dbname ''invalid_dbname'');',
    '42710',
    'option "dbname" provided more than once',
    'Check an error is thrown when trying to add a dbname option to foreign server'
  );

  -- TEST 13
  SELECT throws_ok(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER server_name OPTIONS (DROP username);',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to DROP username option from user mapping'
  );

  -- TEST 14
  SELECT throws_ok(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER server_name OPTIONS (DROP password);',
    'HV000',
    'insufficient options',
    'Check an error is thrown when trying to DROP password option from user mapping'
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
