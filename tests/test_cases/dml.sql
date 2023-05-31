-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(2);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :'TIBERO_HOST', port :'TIBERO_PORT', dbname :'TIBERO_DB');

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :'TIBERO_USER', password :'TIBERO_PASS');

  CREATE FOREIGN TABLE fins_test (
    c1 INT,
    c2 VARCHAR(10),
    c3 CHAR(9),
    c4 BIGINT,
    c5 DATE,
    c6 DECIMAL(10,5),
    c7 INT,
    c8 SMALLINT,
    c9 NCHAR(9),
    c10 TEXT
  ) server server_name options (owner_name :'TIBERO_USER', table_name 'ins_test', updatable 'on');

  -- TEST 1:
  SELECT lives_ok('
    INSERT INTO fins_test 
    VALUES (100, ''HS1'', ''KOREA'', 1300, ''1980-12-17'', 800.23, NULL, 20, ''aaaaa'', ''가가가가가가가가가가가가가가'');');

  -- TEST 2:
  SELECT results_eq('
    SELECT c1, c2, c3, c4, c5, c6, c7, c8, c9, c10
    FROM fins_test
    LIMIT 1;',
    $$VALUES 
    (100::INT, 'HS1'::VARCHAR, 'KOREA'::CHAR(9), 1300::BIGINT,
     '1980-12-17'::DATE, 800.23000::DECIMAL, null::INT, 20::SMALLINT,
     'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT)
    $$,
    'Verify query results for insert'
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();

ROLLBACK;
