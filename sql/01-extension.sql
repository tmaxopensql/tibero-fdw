-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(4);

  -- TEST 1: CREATE EXTENSION
  SELECT lives_ok('CREATE EXTENSION tibero_fdw');

  -- TEST 2: DROP EXTENSION
  SELECT lives_ok('DROP EXTENSION tibero_fdw CASCADE');

  -- TEST 3: CREATE EXTENSION with IF NOT EXISTS option
  SELECT lives_ok('CREATE EXTENSION IF NOT EXISTS tibero_fdw');

  -- TEST 4: Check the extension is created as intended
  SELECT has_extension('tibero_fdw');

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
