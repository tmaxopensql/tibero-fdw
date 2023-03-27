-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(4);

  -- TEST 1
  SELECT lives_ok('
    CREATE EXTENSION tibero_fdw',
    'Extension 생성 테스트'
  );

  -- TEST 2
  SELECT lives_ok('
    DROP EXTENSION tibero_fdw CASCADE',
    'Extension Drop 테스트'
  );

  -- TEST 3
  SELECT lives_ok('
    CREATE EXTENSION IF NOT EXISTS tibero_fdw',
    'IF NOT EXISTS 옵션으로 Extension 생성 테스트'
  );

  -- TEST 4
  SELECT has_extension('public', 'tibero_fdw', 'Extension 생성 여부 확인');

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
