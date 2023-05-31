-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(15);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :'TIBERO_HOST', port :'TIBERO_PORT', dbname :'TIBERO_DB');

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :'TIBERO_USER', password :'TIBERO_PASS');

  CREATE FOREIGN TABLE n_ft (
      nb_default NUMERIC,
      nb_380 NUMERIC(38,0),
      nb_38191 NUMERIC(38,19),
      nb_38192 NUMERIC(38,19),
      nb_ltm NUMERIC(38,19),
      nb_gtm NUMERIC(130,130),
      flt FLOAT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 't2');

  -- TEST 1
  SELECT lives_ok(
    'SELECT * FROM n_ft',
    'Verify compatibility between Tibero NUMBER, FLOAT types and Postgres NUMERIC type'
  );

  CREATE FOREIGN TABLE n_prec (
      nb_38191  NUMERIC(38, 2),
      nb_gtm    NUMERIC(12, 4)
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 't2');

  -- TEST 2
  SELECT is(
    (SELECT SUM(CASE WHEN nb_38191 = '123456.12'::numeric THEN 1 ELSE 0 END) FROM n_prec)::INTEGER,
    2,
    'Verify NUMERIC precision of foreign NUMBER type data'
  );

  -- TEST 3
  SELECT is(
    (SELECT SUM(CASE WHEN nb_gtm = '0.6543'::numeric THEN 1 ELSE 0 END) FROM n_prec)::INTEGER,
    4,
    'Verify NUMERIC precision of foreign NUMBER type data'
  );

  CREATE FOREIGN TABLE n_ft1 (
      nb_default CHAR(39),
      nb_380 CHAR(39),
      nb_38191 CHAR(39),
      nb_38192 CHAR(39),
      nb_ltm CHAR(39),
      nb_gtm CHAR(39),
      flt CHAR(39)
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 't2');

  -- TEST 4
  SELECT lives_ok('SELECT * FROM n_ft1',
    'Verify compatibility between Tibero NUMBER, FLOAT types and Postgres CHAR type'
  );

  CREATE FOREIGN TABLE n_ft2 (
      nb_default VARCHAR(39),
      nb_380 VARCHAR(39),
      nb_38191 VARCHAR(39),
      nb_38192 VARCHAR(39),
      nb_ltm VARCHAR(39),
      nb_gtm VARCHAR(39),
      flt VARCHAR(39)
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 't2');

  -- TEST 5
  SELECT lives_ok('SELECT * FROM n_ft2',
    'Verify compatibility between Tibero NUMBER, FLOAT types and Postgres VARCHAR type'
  );

  CREATE FOREIGN TABLE n_ft3 (
      nb_default TEXT,
      nb_380 TEXT,
      nb_38191 TEXT,
      nb_38192 TEXT,
      nb_ltm TEXT,
      nb_gtm TEXT,
      flt TEXT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 't2');

  -- TEST 6
  SELECT lives_ok('SELECT * FROM n_ft3',
    'Verify compatibility between Tibero NUMBER, FLOAT types and Postgres TEXT type'
  );

  CREATE FOREIGN TABLE n_ft4 (
      nb_default SMALLINT,
      nb_380 SMALLINT,
      nb_38191 SMALLINT,
      nb_38192 SMALLINT,
      nb_ltm SMALLINT,
      nb_gtm SMALLINT,
      flt SMALLINT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'smallint_test');

  -- TEST 7
  SELECT lives_ok('SELECT * FROM n_ft4',
    'Verify compatibility between Tibero NUMBER, FLOAT types and Postgres SMALLINT type'
  );

  CREATE FOREIGN TABLE n_ft5 (
      nb_default SMALLINT,
      nb_380 SMALLINT,
      nb_38191 SMALLINT,
      nb_38192 SMALLINT,
      nb_ltm SMALLINT,
      nb_gtm SMALLINT,
      flt SMALLINT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'smallint_test_err');

  -- TEST 8
  SELECT throws_ok(
    'SELECT * FROM n_ft5',
    '22003',
    'value "-32769" is out of range for type smallint',
    'Check an error is thrown when trying to fetch out-of-range Tibero NUMBER, FLOAT values into Postgres SMALLINT'
  );

  CREATE FOREIGN TABLE n_ft6 (
    nb_default INTEGER,
    nb_380 INTEGER,
    nb_38191 INTEGER,
    nb_38192 INTEGER,
    nb_ltm INTEGER,
    nb_gtm INTEGER,
    flt INTEGER
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'integer_test');

  -- TEST 9
  SELECT lives_ok(
    'SELECT * FROM n_ft6',
    'Verify compatibility between Tibero NUMBER type and Postgres INTEGER type'
  );

  CREATE FOREIGN TABLE n_ft7 (
      nb_default INTEGER,
      nb_380 INTEGER,
      nb_38191 INTEGER,
      nb_38192 INTEGER,
      nb_ltm INTEGER,
      nb_gtm INTEGER,
      flt INTEGER
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'integer_test_err');

  -- TEST 10
  SELECT throws_ok(
    'SELECT * FROM n_ft7',
    '22003',
    'value "-2147483649" is out of range for type integer',
    'Check an error is thrown when trying to fetch out-of-range Tibero NUMBER value into Postgres INTEGER'
  );

  CREATE FOREIGN TABLE n_ft8 (
      nb_default BIGINT,
      nb_380 BIGINT,
      nb_38191 BIGINT,
      nb_38192 BIGINT,
      nb_ltm BIGINT,
      nb_gtm BIGINT,
      flt BIGINT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'bigint_test');

  -- TEST 11
  SELECT lives_ok(
    'SELECT * FROM n_ft8',
    'Verify compatibility between Tibero NUMBER type and Postgres BIGINT type'
  );

  CREATE FOREIGN TABLE n_ft9 (
      nb_default BIGINT,
      nb_380 BIGINT,
      nb_38191 BIGINT,
      nb_38192 BIGINT,
      nb_ltm BIGINT,
      nb_gtm BIGINT,
      flt BIGINT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'bigint_test_err');

  -- TEST 12
  SELECT throws_ok(
    'SELECT * FROM n_ft9',
    '22003',
    'value "-9223372036854775809" is out of range for type bigint',
    'Check an error is thrown when trying to fetch out-of-range Tibero NUMBER value into Postgres BIGINT'
  );

  CREATE FOREIGN TABLE n_ft10 (
      nb_default REAL,
      nb_380 REAL,
      nb_38191 REAL,
      nb_38192 REAL,
      nb_ltm REAL,
      nb_gtm REAL,
      flt REAL
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'r_and_dp_test');

  -- TEST 13
  SELECT lives_ok(
    'SELECT * FROM n_ft10',
    'Verify compatibility between Tibero NUMBER type and Postgres REAL type'
  );

  CREATE FOREIGN TABLE n_ft11 (
      nb_default DOUBLE PRECISION,
      nb_380 DOUBLE PRECISION,
      nb_38191 DOUBLE PRECISION,
      nb_38192 DOUBLE PRECISION,
      nb_ltm DOUBLE PRECISION,
      nb_gtm DOUBLE PRECISION,
      flt DOUBLE PRECISION
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'r_and_dp_test');

  -- TEST 14
  SELECT lives_ok(
    'SELECT * FROM n_ft11',
    'Verify compatibility between Tibero NUMBER type and Postgres PRECISION type'
  );

  CREATE FOREIGN TABLE n_ft12 (
      nb_default NUMERIC(2,1),
      nb_380 NUMERIC(2,1),
      nb_38191 NUMERIC(2,1),
      nb_38192 NUMERIC(2,1),
      nb_ltm NUMERIC(2,1),
      nb_gtm NUMERIC(2,1),
      flt FLOAT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 't2');

  -- TEST 15
  SELECT throws_ok(
    'SELECT * FROM n_ft12',
    '22003',
    'numeric field overflow',
    'Check an Numeric field overflow error occurs when trying to fetch Tibero NUMBER values into Postgres NUMERIC columns of insufficient scales'
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();
ROLLBACK;
