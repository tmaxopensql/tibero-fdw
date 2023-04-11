-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(4);

  -- TEST 1
  SELECT lives_ok('
    CREATE EXTENSION tibero_fdw',
    'Test CREATE EXTENSION command'
  );

  -- TEST 2
  SELECT lives_ok('
    DROP EXTENSION tibero_fdw CASCADE',
    'Test DROP EXTENSION CASCADE command'
  );

  -- TEST 3
  SELECT lives_ok('
    CREATE EXTENSION IF NOT EXISTS tibero_fdw',
    'Test CREATE EXTENSION with IF NOT EXISTS option'
  );

  -- TEST 4
  SELECT has_extension('public',
    'tibero_fdw',
    'Check the extension is correctly registered in pg_extension table');

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
