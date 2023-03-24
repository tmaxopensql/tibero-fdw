-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT * FROM no_plan();

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE OR REPLACE FUNCTION print_result( TEXT )
  RETURNS VOID AS $$
    DECLARE
      item record;
      sqlquery  text;
    BEGIN
      sqlquery := 'SELECT * FROM ' || $1 || ' ';
      FOR item IN EXECUTE sqlquery LOOP
        RAISE NOTICE 'Value: %', item;
      END LOOP;
    END;
  $$ LANGUAGE plpgsql;

  CREATE OR REPLACE FUNCTION print_exec( TEXT )
  RETURNS VOID AS $$
    DECLARE
      item record;
    BEGIN
      FOR item IN EXECUTE $1 LOOP
        RAISE NOTICE 'Value: %', item;
      END LOOP;
    END;
  $$ LANGUAGE plpgsql;

  SELECT pass();

  -- Finish the tests and clean up.
  SELECT * FROM finish();

ROLLBACK;
