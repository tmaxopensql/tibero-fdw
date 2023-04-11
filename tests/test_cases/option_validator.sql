-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;
  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  SELECT plan(29);

  /* Tests server options */
  -- TEST 1
  SELECT throws_matching(
    'CREATE SERVER tibero_server FOREIGN DATA WRAPPER tibero_fdw
     OPTIONS (wrong_option ''wrong option value'')',
     'invalid option "wrong_option"',
     'Create server with an unknown option'
  );

  -- TEST 2
  SELECT lives_ok(
    'CREATE SERVER tibero_server FOREIGN DATA WRAPPER tibero_fdw
     OPTIONS (host ''localhost'', port ''1234'', dbname ''testdb'', fetch_size ''100'')',
     'Create server with valid options for server'
  );

  -- TEST 3
  SELECT lives_ok(
    'ALTER SERVER tibero_server OPTIONS (SET host ''tiberohost'')',
    'Set a valid option in server'
  );

  -- TEST 4
  SELECT throws_matching(
    'ALTER SERVER tibero_server OPTIONS (SET wrong_option ''wrong option value'')',
    'option "wrong_option" not found',
    'Set an unknown option in server'
  );

  -- TEST 5
  SELECT lives_ok(
    'ALTER SERVER tibero_server OPTIONS (DROP fetch_size)',
    'Drop a valid option from server'
  );

  -- TEST 6
  SELECT throws_matching(
    'ALTER SERVER tibero_server OPTIONS (DROP wrong_option)',
    'option "wrong_option" not found',
    'Drop an unknown option from server'
  );

  -- TEST 7
  SELECT lives_ok(
    'ALTER SERVER tibero_server OPTIONS (ADD fetch_size ''100'')',
    'Add a valid option to server'
  );

  -- TEST 8
  SELECT throws_matching(
    'ALTER SERVER tibero_server OPTIONS (ADD wrong_option ''wrong value'')',
    'invalid option "wrong_option"',
    'Add an unknown option to server'
  );

  /* Tests user mapping options */
  -- TEST 9
  SELECT throws_matching(
    'CREATE USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (wrong_option ''wrong option value'')',
    'invalid option "wrong_option"',
    'Create user mapping with an unknown option'
  );

  -- TEST 10
  SELECT lives_ok(
    'CREATE USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (username ''tibero'', password ''tmax'')',
     'Create user mapping with valid options'
  );

  -- TEST 11
  SELECT lives_ok(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (SET username ''another_user'')',
    'Set a valid option in user mapping'
  );

  -- TEST 12
  SELECT throws_matching(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (SET wrong_option ''wrong option value'')',
    'option "wrong_option" not found',
    'Set an unknown option in user mapping'
  );

  -- TEST 13
  SELECT throws_matching(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (DROP username)',
    'insufficient options',
    'Drop a valid option from user mapping'
  );

  -- TEST 14
  SELECT throws_matching(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (DROP wrong_option)',
    'option "wrong_option" not found',
    'Drop an unknown option from user mapping'
  );

  -- TEST 15
  SELECT lives_ok(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (ADD password_required ''true'')',
    'Add a valid option to user mapping'
  );

  -- TEST 16
  SELECT throws_matching(
    'ALTER USER MAPPING FOR CURRENT_USER SERVER tibero_server
     OPTIONS (ADD wrong_option ''wrong option value'')',
     'invalid option "wrong_option"',
    'Add an unknown option to user mapping'
  );

  /* Tests foreign table options */
  -- TEST 17
  SELECT throws_matching(
    'CREATE FOREIGN TABLE foreign_tbl (c1 int) SERVER tibero_server
     OPTIONS (wrong_option ''wrong option value'')',
    'invalid option "wrong_option"',
    'Create foreign table with an unknown option'
  );

  -- TEST 18
  SELECT lives_ok(
    'CREATE FOREIGN TABLE foreign_tbl (c1 int) SERVER tibero_server
     OPTIONS (owner_name ''tmax'', table_name ''test'')',
    'Create foreign table with valid options'
  );

  -- TEST 19
  SELECT lives_ok(
    'ALTER FOREIGN TABLE foreign_tbl OPTIONS (SET owner_name ''another_owner'')',
    'Set a valid option in foreign table'
  );

  -- TEST 20
  SELECT throws_matching(
    'ALTER FOREIGN TABLE foreign_tbl OPTIONS (SET wrong_option ''wrong option value'')',
    'option "wrong_option" not found',
    'Set an unknown option in foreign table'
  );

  -- TEST 21
  SELECT lives_ok(
    'ALTER FOREIGN TABLE foreign_tbl OPTIONS (DROP owner_name)',
    'Drop a valid option from foreign table'
  );

  -- TEST 22
  SELECT throws_matching(
    'ALTER FOREIGN TABLE foreign_tbl OPTIONS (DROP wrong_option)',
    'option "wrong_option" not found',
    'Drop an unknown option from foreign table'
  );

  -- TEST 23
  SELECT lives_ok(
    'ALTER FOREIGN TABLE foreign_tbl OPTIONS (ADD owner_name ''tibero'')',
    'Add a valid option to foreign table'
  );

  -- TEST 24
  SELECT throws_matching(
    'ALTER FOREIGN TABLE foreign_tbl OPTIONS (ADD wrong_option ''wrong option value'')',
     'invalid option "wrong_option"',
    'Add an unknown option to foreign table'
  );

  /* Options with empty string value */
  CREATE FOREIGN TABLE test25_tbl (c1 int) SERVER tibero_server
  OPTIONS (owner_name 'tmax', table_name 'test');

  -- TEST 25
  SELECT throws_matching(
    'ALTER FOREIGN TABLE test25_tbl OPTIONS (SET owner_name '''')',
    '"owner_name" requires non-empty value',
    'Set an option to empty value'
  );

  -- TEST 26
  SELECT throws_matching(
    'ALTER FOREIGN TABLE test25_tbl OPTIONS (ADD fetch_size '''')',
    '"fetch_size" requires non-empty value',
    'Add an option with empty value'
  );

  -- TEST 27
  SELECT throws_matching(
    'ALTER FOREIGN TABLE test25_tbl OPTIONS (ADD use_fb_query '''')',
    '"use_fb_query" requires non-empty value',
    'Add a boolean option with empty value'
  );

  /* Required options validation */
  -- TEST 28
  SELECT throws_matching(
    'CREATE SERVER tibero_server26 FOREIGN DATA WRAPPER tibero_fdw
     OPTIONS (host ''localhost'', port ''5432'')',
    'insufficient options',
    'Forget to put a required option "dbname"'
  );

  -- TEST 29
  CREATE SERVER tibero_server27 FOREIGN DATA WRAPPER tibero_fdw
  OPTIONS (host 'localhost', port '5432', dbname 'testdb');

  SELECT throws_matching(
    'ALTER SERVER tibero_server27 OPTIONS (DROP host)',
    'insufficient options',
    'Drop a required option "host"'
  );


  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
