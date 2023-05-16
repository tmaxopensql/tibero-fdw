-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(21);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :'TIBERO_HOST', port :'TIBERO_PORT', dbname :'TIBERO_DB');

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :'TIBERO_USER', password :'TIBERO_PASS');

  SET COMPUTE_QUERY_ID=false;

  CREATE FOREIGN TABLE jft1 (
      c_kor CHAR(100),
      c_eng CHAR(100),
      c_spc CHAR(100),
      nc_kor NCHAR(100),
      nc_eng NCHAR(100),
      nc_spc NCHAR(100),
      vc_kor VARCHAR(100),
      vc_eng VARCHAR(100),
      vc_spc VARCHAR(100),
      vc2_kor VARCHAR(100),
      vc2_eng VARCHAR(100),
      vc2_spc VARCHAR(100),
      nvc_kor TEXT,
      nvc_eng TEXT,
      nvc_spc TEXT,
      rwid TEXT,
      nm NUMERIC,
      flt FLOAT,
      dt DATE,
      ts TIMESTAMP,
      tsz TIMESTAMP WITH TIME ZONE,
      iytm INTERVAL YEAR TO MONTH,
      idts INTERVAL DAY TO SECOND
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'jt1');

  CREATE FOREIGN TABLE jft2 (
      c_kor CHAR(100),
      c_eng CHAR(100),
      c_spc CHAR(100),
      nc_kor NCHAR(100),
      nc_eng NCHAR(100),
      nc_spc NCHAR(100),
      vc_kor VARCHAR(100),
      vc_eng VARCHAR(100),
      vc_spc VARCHAR(100),
      vc2_kor VARCHAR(100),
      vc2_eng VARCHAR(100),
      vc2_spc VARCHAR(100),
      nvc_kor TEXT,
      nvc_eng TEXT,
      nvc_spc TEXT,
      rwid TEXT,
      nm NUMERIC,
      flt FLOAT,
      dt DATE,
      ts TIMESTAMP,
      tsz TIMESTAMP WITH TIME ZONE,
      iytm INTERVAL YEAR TO MONTH,
      idts INTERVAL DAY TO SECOND
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'jt1');

  CREATE FOREIGN TABLE jft3 (
      c_kor CHAR(100),
      c_eng CHAR(100),
      c_spc CHAR(100),
      nc_kor NCHAR(100),
      nc_eng NCHAR(100),
      nc_spc NCHAR(100),
      vc_kor VARCHAR(100),
      vc_eng VARCHAR(100),
      vc_spc VARCHAR(100),
      vc2_kor VARCHAR(100),
      vc2_eng VARCHAR(100),
      vc2_spc VARCHAR(100),
      nvc_kor TEXT,
      nvc_eng TEXT,
      nvc_spc TEXT,
      rwid TEXT,
      nm NUMERIC,
      flt FLOAT,
      dt DATE,
      ts TIMESTAMP,
      tsz TIMESTAMP WITH TIME ZONE,
      iytm INTERVAL YEAR TO MONTH,
      idts INTERVAL DAY TO SECOND
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'jt1');

  -- TEST 1:
  SELECT lives_ok('SELECT * FROM jft1');

  -- TEST 2:
  SELECT lives_ok('SELECT * FROM jft2');

  -- TEST 3:
  SELECT lives_ok('SELECT * FROM jft3');

	-- TEST 4
  SELECT results_eq('
    SELECT JT1.c_kor, JT2.c_kor
    FROM jft1 JT1 JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor)
    ORDER BY JT1.c_kor;',
    $$VALUES (
      '가나다라마바사아자차카타파하'::CHAR(100),
      '가나다라마바사아자차카타파하'::CHAR(100))$$,
    'Verify query results for JOIN multiple foreign table query'
  );

	-- TEST 5
  SELECT results_eq('
    SELECT JT1.nc_kor, JT2.nc_kor, JT3.nc_kor
      FROM jft1 JT1 JOIN jft2 JT2 ON (JT1.nc_kor = JT2.nc_kor)
      JOIN jft3 JT3 ON (JT3.nc_kor = JT1.nc_kor) ORDER BY JT1.nc_kor;',
    $$VALUES (
      '가나다라마바사아자차카타파하'::NCHAR(100),
      '가나다라마바사아자차카타파하'::NCHAR(100),
      '가나다라마바사아자차카타파하'::NCHAR(100))$$,
    'Verify query results for JOIN multiple foreign table query'
  );

  -- TEST 6
  SELECT results_eq('
    SELECT JT2.vc_kor, JT3.vc_kor
      FROM jft2 JT2 LEFT JOIN jft3 JT3 ON (JT2.vc_kor = JT3.vc_kor)
      ORDER BY JT2.vc_kor, JT3.vc_kor;',
    $$VALUES (
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      '가나다라마바사아자차카타파하'::VARCHAR(100))$$,
    'Verify query results for LEFT OUTER JOIN multiple foreign table query'
  );

	-- TEST 7
  SELECT results_eq('
    SELECT JT1.vc2_kor, JT2.vc2_kor, JT3.vc2_kor
       FROM jft1 JT1
         LEFT JOIN jft2 JT2 ON (JT1.vc2_kor = JT2.vc2_kor)
         LEFT JOIN jft3 JT3 ON (JT2.vc2_kor = JT3.vc2_kor);',
    $$VALUES (
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      '가나다라마바사아자차카타파하'::VARCHAR(100))$$,
    'Verify query results for LEFT OUTER JOIN multiple foreign table query'
  );

  -- TEST 8
  SELECT results_eq('
    SELECT JT2.dt, JT3.dt
       FROM jft2 JT2
         LEFT JOIN (SELECT * FROM jft3 WHERE flt > 10) JT3 ON (JT2.dt = JT3.dt)
       WHERE JT2.dt > ''20220101''',
    $$VALUES (
      '2023-01-01'::DATE,
      '2023-01-01'::DATE)$$,
    'Verify query results for LEFT OUTER JOIN multiple foreign table query with subquery'
  );
  
  -- TEST 9: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.rwid, JT2.rwid
      FROM jft1 JT1
        RIGHT JOIN jft2 JT2 ON (JT1.rwid = JT2.rwid)
      ORDER BY JT2.rwid, JT1.rwid',
    $$VALUES (
      'AAAArFAAAAAACTFAAA',
      'AAAArFAAAAAACTFAAA')$$,
    'Verify query results for RIGHT OUTER JOIN multiple foreign table query'
  );

  -- TEST 10
  SELECT results_eq('
    SELECT JT1.nvc_spc, JT2.nvc_kor, JT3.nvc_eng
      FROM jft2 JT2
        RIGHT JOIN jft3 JT3 ON (JT2.nvc_spc = JT3.nvc_spc)
        RIGHT JOIN jft1 JT1 ON (JT3.nvc_spc = JT1.nvc_spc)',
    $$VALUES (
      '!@#$%^&*()<>/\\''',
      '가나다라마바사아자차카타파하',
      'abcdefghijklmnopqrstuvwxyz')$$,
    'Verify query results for RIGHT OUTER JOIN multiple foreign table query'
  );

  -- TEST 11
  SELECT results_eq('
    SELECT JT1.c_spc, JT2.c_spc
      FROM jft1 JT1
        FULL JOIN jft2 JT2 ON (JT1.idts = JT2.idts)
      ORDER BY JT1.c_spc, JT2.c_spc',
    $$VALUES (
      '!@#$%^&*()<>/\\'''::CHAR(100),
      '!@#$%^&*()<>/\\'''::CHAR(100)
    )$$,
    'Verify query results for FULL OUTER JOIN multiple foreign table query'
  );

  -- TEST 12
  SELECT results_eq('
    SELECT JT1.flt, JT2.flt, JT3.flt
      FROM jft1 JT1
        FULL JOIN jft2 JT2 ON (JT1.flt = JT2.flt)
        FULL JOIN jft3 JT3 ON (JT2.flt = JT3.flt)',
    $$VALUES (
      123456.123456789::FLOAT,
      123456.123456789::FLOAT,
      123456.123456789::FLOAT
    )$$,
    'Verify query results for FULL OUTER JOIN multiple foreign table query'
  );

  -- TEST 13
  SELECT results_eq('
    SELECT JT1.ts, JT2.ts, JT3.ts
      FROM jft1 JT1
        INNER JOIN jft2 JT2 ON (JT1.ts = JT2.ts and JT1.flt > 1)
        FULL JOIN jft3 JT3 ON (JT2.ts = JT3.ts)',
    $$VALUES (
      TO_TIMESTAMP('2023-01-01 12:34:56.123456', 'YYYY-MM-DD HH24:MI:SS.FF6')::TIMESTAMP,
      TO_TIMESTAMP('2023-01-01 12:34:56.123456', 'YYYY-MM-DD HH24:MI:SS.FF6')::TIMESTAMP,
      TO_TIMESTAMP('2023-01-01 12:34:56.123456', 'YYYY-MM-DD HH24:MI:SS.FF6')::TIMESTAMP
    )$$,
    'Verify query results for FULL OUTER JOIN with INNER JOIN multiple foreign table query'
  );

  -- TEST 14
  SELECT results_eq('
    SELECT JT1.vc_eng, JT2.vc_eng, JT3.vc_eng 
      FROM jft1 JT1 
        FULL JOIN jft2 JT2 ON (JT1.vc_eng = JT2.vc_eng) 
        RIGHT JOIN jft3 JT3 ON (JT2.vc_eng = JT3.vc_eng)',
    $$VALUES (
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100)
    )$$,
    'Verify query results for FULL OUTER JOIN with RIGHT OUTER JOIN multiple foreign table query'
  ); 

  -- TEST 15
  SELECT results_eq('
    SELECT JT1.vc2_kor, JT2.vc2_eng, JT3.vc2_spc FROM jft1 JT1 
      RIGHT JOIN jft2 JT2 ON (JT1.vc2_kor = JT2.vc2_kor) 
      FULL JOIN jft3 JT3 ON (JT2.vc2_kor = JT3.vc2_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100),
      '!@#$%^&*()<>/\\'''::VARCHAR(100)
    )$$,
    'Verify query results for RIGHT OUTER JOIN with FULL OUTER JOIN multiple foreign table query'
  );

  -- TEST 16
  SELECT results_eq(
    'SELECT JT1.nvc_kor, JT2.nvc_eng, JT3.nvc_spc 
       FROM jft1 JT1 
         FULL JOIN jft2 JT2 ON (JT1.nvc_kor = JT2.nvc_kor) 
         LEFT JOIN jft3 JT3 ON (JT2.nvc_kor = JT3.nvc_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::TEXT,
      'abcdefghijklmnopqrstuvwxyz'::TEXT,
      '!@#$%^&*()<>/\\'''::TEXT
    )$$,
    'Verify query results for FULL OUTER JOIN with LEFT OUTER JOIN multiple foreign table query'
  );

  -- TEST 17
  SELECT results_eq('
    SELECT JT1.nc_kor, JT2.nc_eng, JT3.nc_spc 
      FROM jft1 JT1 
        LEFT JOIN jft2 JT2 ON (JT1.nc_kor = JT2.nc_kor) 
        FULL JOIN jft3 JT3 ON (JT2.nc_kor = JT3.nc_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::NCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::NCHAR(100),
      '!@#$%^&*()<>/\\'''::NCHAR(100)
    )$$,
    'Verify query results for LEFT OUTER JOIN with FULL OUTER JOIN multiple foreign table query'
  );

  -- TEST 18
  SELECT results_eq('
    SELECT JT1.c_kor, JT2.c_eng, JT3.c_spc 
      FROM jft1 JT1 
        RIGHT JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor) 
        LEFT JOIN jft3 JT3 ON (JT2.c_kor = JT3.c_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::CHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::CHAR(100),
      '!@#$%^&*()<>/\\'''::CHAR(100)
    )$$,
    'Verify query results for RIGHT OUTER JOIN with LEFT OUTER JOIN multiple foreign table query'
  );

  -- TEST 19
  SELECT results_eq('
    SELECT JT1.c_kor, JT2.c_eng, JT3.c_spc 
      FROM jft1 JT1 
        LEFT JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor) 
        RIGHT JOIN jft3 JT3 ON (JT2.c_kor = JT3.c_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::CHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::CHAR(100),
      '!@#$%^&*()<>/\\'''::CHAR(100)
    )$$,
    'Verify query results for LEFT OUTER JOIN with RIGHT OUTER JOIN multiple foreign table query'
  );

  -- TEST 20
  SELECT results_eq('
    SELECT JT1.flt, JT2.flt 
      FROM jft1 JT1 
      CROSS JOIN jft2 JT2 
    ORDER BY JT1.flt, JT2.flt',
    $$VALUES (
      123456.123456789::FLOAT,
      123456.123456789::FLOAT
    )$$,
    'Verify query results for CROSS JOIN multiple foreign table'
  );

  CREATE FOREIGN TABLE fst1 (
      c1 INT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'st1');

  CREATE FOREIGN TABLE fst2 (
      c1 INT
  ) SERVER server_name OPTIONS (owner_name :'TIBERO_USER', table_name 'st2');

  -- TEST 21
  SELECT results_eq('
    SELECT * FROM fst1, fst2 where fst1.c1 <= 200;',
    $$VALUES
      (100::INT, 10::INT),
      (100::INT, 20::INT),
      (100::INT, 30::INT),
      (100::INT, 40::INT),
      (200::INT, 10::INT),
      (200::INT, 20::INT),
      (200::INT, 30::INT),
      (200::INT, 40::INT)
    $$
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();

ROLLBACK;
