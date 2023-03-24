-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(37);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);
  
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
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'jt1');

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
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'jt1');

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
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'jt1');

  -- TEST 1:
  SELECT lives_ok('SELECT * FROM jft1');

  -- TEST 2:
  SELECT lives_ok('SELECT * FROM jft2');

  -- TEST 3:
  SELECT lives_ok('SELECT * FROM jft3');

  -- TEST 4: 테이블 2개 (jft1, jft2) JOIN 쿼리 플랜 검증
  SELECT results_eq(
    'EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.c_kor, JT2.c_kor
       FROM jft1 JT1 JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor) ORDER BY JT1.c_kor;',
    ARRAY[ 'Merge Join', 
      '  Output: jt1.c_kor, jt2.c_kor',
      '  Merge Cond: (jt1.c_kor = jt2.c_kor)',
      '  ->  Sort',
      '        Output: jt1.c_kor',
      '        Sort Key: jt1.c_kor',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.c_kor',
      '  ->  Sort',
      '        Output: jt2.c_kor',
      '        Sort Key: jt2.c_kor',
      '        ->  Foreign Scan on public.jft2 jt2',
      '              Output: jt2.c_kor'
    ]
  );

  -- TEST 5: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.c_kor, JT2.c_kor
    FROM jft1 JT1 JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor)
    ORDER BY JT1.c_kor;',
    $$VALUES (
      '가나다라마바사아자차카타파하'::CHAR(100),
      '가나다라마바사아자차카타파하'::CHAR(100))$$
  );

  -- TEST 6: 테이블 3개 (jft1, jft2, jft3) JOIN 쿼리 플랜 검증
  SELECT results_eq(
    'EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.nc_kor, JT2.nc_kor, JT3.nc_kor
       FROM jft1 JT1 JOIN jft2 JT2 ON (JT1.nc_kor = JT2.nc_kor)
         JOIN jft3 JT3 ON (JT3.nc_kor = JT1.nc_kor) ORDER BY JT1.nc_kor;',
    ARRAY[ 'Merge Join',
      '  Output: jt1.nc_kor, jt2.nc_kor, jt3.nc_kor',
      '  Merge Cond: (jt1.nc_kor = jt3.nc_kor)',
      '  ->  Merge Join',
      '        Output: jt1.nc_kor, jt2.nc_kor',
      '        Merge Cond: (jt1.nc_kor = jt2.nc_kor)',
      '        ->  Sort',
      '              Output: jt1.nc_kor',
      '              Sort Key: jt1.nc_kor',
      '              ->  Foreign Scan on public.jft1 jt1',
      '                    Output: jt1.nc_kor',
      '        ->  Sort',
      '              Output: jt2.nc_kor',
      '              Sort Key: jt2.nc_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.nc_kor',
      '  ->  Sort',
      '        Output: jt3.nc_kor',
      '        Sort Key: jt3.nc_kor',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.nc_kor'
    ]
  );

  -- TEST 7: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.nc_kor, JT2.nc_kor, JT3.nc_kor
      FROM jft1 JT1 JOIN jft2 JT2 ON (JT1.nc_kor = JT2.nc_kor)
      JOIN jft3 JT3 ON (JT3.nc_kor = JT1.nc_kor) ORDER BY JT1.nc_kor;',
    $$VALUES (
      '가나다라마바사아자차카타파하'::NCHAR(100),
      '가나다라마바사아자차카타파하'::NCHAR(100),
      '가나다라마바사아자차카타파하'::NCHAR(100))$$
  );

  -- TEST 8: LEFT OUTER JOIN (jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq(
    'EXPLAIN (VERBOSE, COSTS OFF) SELECT JT2.vc_kor, JT3.vc_kor
       FROM jft2 JT2 LEFT JOIN jft3 JT3 ON (JT2.vc_kor = JT3.vc_kor)
       ORDER BY JT2.vc_kor, JT3.vc_kor;',
    ARRAY[ 'Incremental Sort',
      '  Output: jt2.vc_kor, jt3.vc_kor',
      '  Sort Key: jt2.vc_kor, jt3.vc_kor',
      '  Presorted Key: jt2.vc_kor',
      '  ->  Merge Left Join',
      '        Output: jt2.vc_kor, jt3.vc_kor',
      '        Merge Cond: ((jt2.vc_kor)::text = (jt3.vc_kor)::text)',
      '        ->  Sort',
      '              Output: jt2.vc_kor',
      '              Sort Key: jt2.vc_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.vc_kor',
      '        ->  Sort',
      '              Output: jt3.vc_kor',
      '              Sort Key: jt3.vc_kor',
      '              ->  Foreign Scan on public.jft3 jt3',
      '                    Output: jt3.vc_kor'
    ]
  );

  -- TEST 9: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT2.vc_kor, JT3.vc_kor
      FROM jft2 JT2 LEFT JOIN jft3 JT3 ON (JT2.vc_kor = JT3.vc_kor)
      ORDER BY JT2.vc_kor, JT3.vc_kor;',
    $$VALUES (
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      '가나다라마바사아자차카타파하'::VARCHAR(100))$$
  );

  -- TEST 10: LEFT OUTER JOIN (jft1, jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq(
    'EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.vc2_kor, JT2.vc2_kor, JT3.vc2_kor 
       FROM jft1 JT1 
         LEFT JOIN jft2 JT2 ON (JT1.vc2_kor = JT2.vc2_kor) 
         LEFT JOIN jft3 JT3 ON (JT2.vc2_kor = JT3.vc2_kor);',
    ARRAY[ 'Merge Right Join',
      '  Output: jt1.vc2_kor, jt2.vc2_kor, jt3.vc2_kor',
      '  Merge Cond: ((jt2.vc2_kor)::text = (jt1.vc2_kor)::text)',
      '  ->  Merge Left Join',
      '        Output: jt2.vc2_kor, jt3.vc2_kor',
      '        Merge Cond: ((jt2.vc2_kor)::text = (jt3.vc2_kor)::text)',
      '        ->  Sort',
      '              Output: jt2.vc2_kor',
      '              Sort Key: jt2.vc2_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.vc2_kor',
      '        ->  Sort',
      '              Output: jt3.vc2_kor',
      '              Sort Key: jt3.vc2_kor',
      '              ->  Foreign Scan on public.jft3 jt3',
      '                    Output: jt3.vc2_kor',
      '  ->  Sort',
      '        Output: jt1.vc2_kor',
      '        Sort Key: jt1.vc2_kor',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.vc2_kor'
    ]
  );

  -- TEST 11: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.vc2_kor, JT2.vc2_kor, JT3.vc2_kor
       FROM jft1 JT1
         LEFT JOIN jft2 JT2 ON (JT1.vc2_kor = JT2.vc2_kor)
         LEFT JOIN jft3 JT3 ON (JT2.vc2_kor = JT3.vc2_kor);',
    $$VALUES (
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      '가나다라마바사아자차카타파하'::VARCHAR(100))$$
  );

  -- TEST 12: Subquery 포함된 LEFT OUTER JOIN (jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq(
    'EXPLAIN (VERBOSE, COSTS OFF) SELECT JT2.dt, JT3.dt 
       FROM jft2 JT2 
         LEFT JOIN (SELECT * FROM jft3 WHERE flt > 10) JT3 ON (JT2.dt = JT3.dt) 
       WHERE JT2.dt > ''20220101''',
    ARRAY['Hash Left Join',
      '  Output: jt2.dt, jft3.dt',
      '  Hash Cond: (jt2.dt = jft3.dt)',
      '  ->  Foreign Scan on public.jft2 jt2',
      '        Output: jt2.c_kor, jt2.c_eng, jt2.c_spc, jt2.nc_kor, jt2.nc_eng, jt2.nc_spc, jt2.vc_kor, jt2.vc_eng, jt2.vc_spc, jt2.vc2_kor, jt2.vc2_eng, jt2.vc2_spc, jt2.nvc_kor, jt2.nvc_eng, jt2.nvc_spc, jt2.rwid, jt2.nm, jt2.flt, jt2.dt, jt2.ts, jt2.tsz, jt2.iytm, jt2.idts',
      '        Filter: (jt2.dt > ''2022-01-01''::date)',
      '  ->  Hash',
      '        Output: jft3.dt',
      '        ->  Foreign Scan on public.jft3',
      '              Output: jft3.dt',
      '              Filter: (jft3.flt > ''10''::double precision)'
    ]
  );

  -- TEST 13: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT2.dt, JT3.dt
       FROM jft2 JT2
         LEFT JOIN (SELECT * FROM jft3 WHERE flt > 10) JT3 ON (JT2.dt = JT3.dt)
       WHERE JT2.dt > ''20220101''',
    $$VALUES (
      '2023-01-01'::DATE,
      '2023-01-01'::DATE)$$
  );

  -- TEST 14: RIGHT OUTER JOIN (jft1, jft2) 쿼리 플랜 검증
  SELECT results_eq(
    'EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.rwid, JT2.rwid 
       FROM jft1 JT1 
         RIGHT JOIN jft2 JT2 ON (JT1.rwid = JT2.rwid) 
       ORDER BY JT2.rwid, JT1.rwid',
    ARRAY['Incremental Sort',
      '  Output: jt1.rwid, jt2.rwid',
      '  Sort Key: jt2.rwid, jt1.rwid',
      '  Presorted Key: jt2.rwid',
      '  ->  Merge Left Join',
      '        Output: jt1.rwid, jt2.rwid',
      '        Merge Cond: (jt2.rwid = jt1.rwid)',
      '        ->  Sort',
      '              Output: jt2.rwid',
      '              Sort Key: jt2.rwid',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.rwid',
      '        ->  Sort',
      '              Output: jt1.rwid',
      '              Sort Key: jt1.rwid',
      '              ->  Foreign Scan on public.jft1 jt1',
      '                    Output: jt1.rwid'
    ]
  );

  -- TEST 15: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.rwid, JT2.rwid
      FROM jft1 JT1
        RIGHT JOIN jft2 JT2 ON (JT1.rwid = JT2.rwid)
      ORDER BY JT2.rwid, JT1.rwid',
    $$VALUES (
      'AAAArFAAAAAACTFAAA',
      'AAAArFAAAAAACTFAAA')$$
  );

  -- TEST 16: RIGHT OUTER JOIN (jft1, jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.nvc_spc, JT2.nvc_kor, JT3.nvc_eng 
      FROM jft2 JT2 
        RIGHT JOIN jft3 JT3 ON (JT2.nvc_spc = JT3.nvc_spc) 
        RIGHT JOIN jft1 JT1 ON (JT3.nvc_spc = JT1.nvc_spc)',
    ARRAY['Merge Right Join',
      '  Output: jt1.nvc_spc, jt2.nvc_kor, jt3.nvc_eng',
      '  Merge Cond: (jt3.nvc_spc = jt1.nvc_spc)',
      '  ->  Merge Left Join',
      '        Output: jt3.nvc_eng, jt3.nvc_spc, jt2.nvc_kor',
      '        Merge Cond: (jt3.nvc_spc = jt2.nvc_spc)',
      '        ->  Sort',
      '              Output: jt3.nvc_eng, jt3.nvc_spc',
      '              Sort Key: jt3.nvc_spc',
      '              ->  Foreign Scan on public.jft3 jt3',
      '                    Output: jt3.nvc_eng, jt3.nvc_spc',
      '        ->  Sort',
      '              Output: jt2.nvc_kor, jt2.nvc_spc',
      '              Sort Key: jt2.nvc_spc',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.nvc_kor, jt2.nvc_spc',
      '  ->  Sort',
      '        Output: jt1.nvc_spc',
      '        Sort Key: jt1.nvc_spc',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.nvc_spc'
    ]
  );

  -- TEST 17: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.nvc_spc, JT2.nvc_kor, JT3.nvc_eng
      FROM jft2 JT2
        RIGHT JOIN jft3 JT3 ON (JT2.nvc_spc = JT3.nvc_spc)
        RIGHT JOIN jft1 JT1 ON (JT3.nvc_spc = JT1.nvc_spc)',
    $$VALUES (
      '!@#$%^&*()<>/\\''',
      '가나다라마바사아자차카타파하',
      'abcdefghijklmnopqrstuvwxyz')$$
  );

  -- TEST 18: FULL OUTER JOIN (jft1, jft2) 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.c_spc, JT2.c_spc 
      FROM jft1 JT1 
        FULL JOIN jft2 JT2 ON (JT1.idts = JT2.idts) 
      ORDER BY JT1.c_spc, JT2.c_spc',
    ARRAY['Sort',
      '  Output: jt1.c_spc, jt2.c_spc',
      '  Sort Key: jt1.c_spc, jt2.c_spc',
      '  ->  Hash Full Join',
      '        Output: jt1.c_spc, jt2.c_spc',
      '        Hash Cond: (jt1.idts = jt2.idts)',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.c_kor, jt1.c_eng, jt1.c_spc, jt1.nc_kor, jt1.nc_eng, jt1.nc_spc, jt1.vc_kor, jt1.vc_eng, jt1.vc_spc, jt1.vc2_kor, jt1.vc2_eng, jt1.vc2_spc, jt1.nvc_kor, jt1.nvc_eng, jt1.nvc_spc, jt1.rwid, jt1.nm, jt1.flt, jt1.dt, jt1.ts, jt1.tsz, jt1.iytm, jt1.idts',
      '        ->  Hash',
      '              Output: jt2.c_spc, jt2.idts',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.c_spc, jt2.idts'
    ]
  );

  -- TEST 19: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.c_spc, JT2.c_spc
      FROM jft1 JT1
        FULL JOIN jft2 JT2 ON (JT1.idts = JT2.idts)
      ORDER BY JT1.c_spc, JT2.c_spc',
    $$VALUES (
      '!@#$%^&*()<>/\\'''::CHAR(100),
      '!@#$%^&*()<>/\\'''::CHAR(100)
    )$$
  );

  -- TEST 20: FULL OUTER JOIN (jft1, jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.flt, JT2.flt, JT3.flt 
      FROM jft1 JT1 
        FULL JOIN jft2 JT2 ON (JT1.flt = JT2.flt) 
        FULL JOIN jft3 JT3 ON (JT2.flt = JT3.flt)',
    ARRAY['Hash Full Join',
      '  Output: jt1.flt, jt2.flt, jt3.flt',
      '  Hash Cond: (jt2.flt = jt3.flt)',
      '  ->  Hash Full Join',
      '        Output: jt1.flt, jt2.flt',
      '        Hash Cond: (jt1.flt = jt2.flt)',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.c_kor, jt1.c_eng, jt1.c_spc, jt1.nc_kor, jt1.nc_eng, jt1.nc_spc, jt1.vc_kor, jt1.vc_eng, jt1.vc_spc, jt1.vc2_kor, jt1.vc2_eng, jt1.vc2_spc, jt1.nvc_kor, jt1.nvc_eng, jt1.nvc_spc, jt1.rwid, jt1.nm, jt1.flt, jt1.dt, jt1.ts, jt1.tsz, jt1.iytm, jt1.idts',
      '        ->  Hash',
      '              Output: jt2.flt',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.flt',
      '  ->  Hash',
      '        Output: jt3.flt',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.flt'
    ]
  );

  -- TEST 21: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.flt, JT2.flt, JT3.flt
      FROM jft1 JT1
        FULL JOIN jft2 JT2 ON (JT1.flt = JT2.flt)
        FULL JOIN jft3 JT3 ON (JT2.flt = JT3.flt)',
    $$VALUES (
      123456.123456789::FLOAT,
      123456.123456789::FLOAT,
      123456.123456789::FLOAT
    )$$
  );

  -- TEST 22: FULL OUTER JOIN 및 INNER JOIN (jft1, jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.ts, JT2.ts, JT3.ts 
      FROM jft1 JT1 
        INNER JOIN jft2 JT2 ON (JT1.ts = JT2.ts and JT1.flt > 1) 
        FULL JOIN jft3 JT3 ON (JT2.ts = JT3.ts)',
    ARRAY['Merge Full Join',
      '  Output: jt1.ts, jt2.ts, jt3.ts',
      '  Merge Cond: (jt2.ts = jt3.ts)',
      '  ->  Merge Join',
      '        Output: jt1.ts, jt2.ts',
      '        Merge Cond: (jt1.ts = jt2.ts)',
      '        ->  Sort',
      '              Output: jt1.ts',
      '              Sort Key: jt1.ts',
      '              ->  Foreign Scan on public.jft1 jt1',
      '                    Output: jt1.ts',
      '                    Filter: (jt1.flt > ''1''::double precision)',
      '        ->  Sort',
      '              Output: jt2.ts',
      '              Sort Key: jt2.ts',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.ts',
      '  ->  Sort',
      '        Output: jt3.ts',
      '        Sort Key: jt3.ts',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.ts'
    ]
  );

  -- TEST 23: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.ts, JT2.ts, JT3.ts
      FROM jft1 JT1
        INNER JOIN jft2 JT2 ON (JT1.ts = JT2.ts and JT1.flt > 1)
        FULL JOIN jft3 JT3 ON (JT2.ts = JT3.ts)',
    $$VALUES (
      TO_TIMESTAMP('2023-01-01 12:34:56.123456', 'YYYY-MM-DD HH24:MI:SS.FF6')::TIMESTAMP,
      TO_TIMESTAMP('2023-01-01 12:34:56.123456', 'YYYY-MM-DD HH24:MI:SS.FF6')::TIMESTAMP,
      TO_TIMESTAMP('2023-01-01 12:34:56.123456', 'YYYY-MM-DD HH24:MI:SS.FF6')::TIMESTAMP
    )$$
  );

  -- TEST 24: FULL OUTER JOIN + RIGHT OUTER JOIN (jft1, jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.vc_eng, JT2.vc_eng, JT3.vc_eng 
      FROM jft1 JT1 
        FULL JOIN jft2 JT2 ON (JT1.vc_eng = JT2.vc_eng) 
        RIGHT JOIN jft3 JT3 ON (JT2.vc_eng = JT3.vc_eng)',
    ARRAY['Merge Right Join',
      '  Output: jt1.vc_eng, jt2.vc_eng, jt3.vc_eng',
      '  Merge Cond: ((jt2.vc_eng)::text = (jt3.vc_eng)::text)',
      '  ->  Merge Left Join',
      '        Output: jt2.vc_eng, jt1.vc_eng',
      '        Merge Cond: ((jt2.vc_eng)::text = (jt1.vc_eng)::text)',
      '        ->  Sort',
      '              Output: jt2.vc_eng',
      '              Sort Key: jt2.vc_eng',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.vc_eng',
      '        ->  Sort',
      '              Output: jt1.vc_eng',
      '              Sort Key: jt1.vc_eng',
      '              ->  Foreign Scan on public.jft1 jt1',
      '                    Output: jt1.vc_eng',
      '  ->  Sort',
      '        Output: jt3.vc_eng',
      '        Sort Key: jt3.vc_eng',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.vc_eng'
    ]
  );

  -- TEST 25: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.vc_eng, JT2.vc_eng, JT3.vc_eng 
      FROM jft1 JT1 
        FULL JOIN jft2 JT2 ON (JT1.vc_eng = JT2.vc_eng) 
        RIGHT JOIN jft3 JT3 ON (JT2.vc_eng = JT3.vc_eng)',
    $$VALUES (
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100)
    )$$
  ); 

  -- TEST 26: RIGHT OUTER JOIN + FULL OUTER JOIN 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.vc2_kor, JT2.vc2_eng, JT3.vc2_spc 
      FROM jft1 JT1 
        RIGHT JOIN jft2 JT2 ON (JT1.vc2_kor = JT2.vc2_kor) 
        FULL JOIN jft3 JT3 ON (JT2.vc2_kor = JT3.vc2_kor)',
    ARRAY['Merge Full Join',
      '  Output: jt1.vc2_kor, jt2.vc2_eng, jt3.vc2_spc',
      '  Merge Cond: ((jt2.vc2_kor)::text = (jt3.vc2_kor)::text)',
      '  ->  Merge Left Join',
      '        Output: jt2.vc2_eng, jt2.vc2_kor, jt1.vc2_kor',
      '        Merge Cond: ((jt2.vc2_kor)::text = (jt1.vc2_kor)::text)',
      '        ->  Sort',
      '              Output: jt2.vc2_eng, jt2.vc2_kor',
      '              Sort Key: jt2.vc2_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.vc2_eng, jt2.vc2_kor',
      '        ->  Sort',
      '              Output: jt1.vc2_kor',
      '              Sort Key: jt1.vc2_kor',
      '              ->  Foreign Scan on public.jft1 jt1',
      '                    Output: jt1.vc2_kor',
      '  ->  Sort',
      '        Output: jt3.vc2_spc, jt3.vc2_kor',
      '        Sort Key: jt3.vc2_kor',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.vc2_spc, jt3.vc2_kor'
    ]
  );

  -- TEST 27: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.vc2_kor, JT2.vc2_eng, JT3.vc2_spc FROM jft1 JT1 
      RIGHT JOIN jft2 JT2 ON (JT1.vc2_kor = JT2.vc2_kor) 
      FULL JOIN jft3 JT3 ON (JT2.vc2_kor = JT3.vc2_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::VARCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::VARCHAR(100),
      '!@#$%^&*()<>/\\'''::VARCHAR(100)
    )$$
  );

  -- TEST 28: FULL OUTER JOIN + LEFT OUTER JOIN 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.nvc_kor, JT2.nvc_eng, JT3.nvc_spc 
      FROM jft1 JT1 
        FULL JOIN jft2 JT2 ON (JT1.nvc_kor = JT2.nvc_kor) 
        LEFT JOIN jft3 JT3 ON (JT2.nvc_kor = JT3.nvc_kor)',
    ARRAY['Hash Left Join',
      '  Output: jt1.nvc_kor, jt2.nvc_eng, jt3.nvc_spc',
      '  Hash Cond: (jt2.nvc_kor = jt3.nvc_kor)',
      '  ->  Hash Full Join',
      '        Output: jt1.nvc_kor, jt2.nvc_eng, jt2.nvc_kor',
      '        Hash Cond: (jt1.nvc_kor = jt2.nvc_kor)',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.c_kor, jt1.c_eng, jt1.c_spc, jt1.nc_kor, jt1.nc_eng, jt1.nc_spc, jt1.vc_kor, jt1.vc_eng, jt1.vc_spc, jt1.vc2_kor, jt1.vc2_eng, jt1.vc2_spc, jt1.nvc_kor, jt1.nvc_eng, jt1.nvc_spc, jt1.rwid, jt1.nm, jt1.flt, jt1.dt, jt1.ts, jt1.tsz, jt1.iytm, jt1.idts',
      '        ->  Hash',
      '              Output: jt2.nvc_eng, jt2.nvc_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.nvc_eng, jt2.nvc_kor',
      '  ->  Hash',
      '        Output: jt3.nvc_spc, jt3.nvc_kor',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.nvc_spc, jt3.nvc_kor'
    ]
  );

  -- TEST 29: JOIN 쿼리 실행 검증
  SELECT results_eq(
    'SELECT JT1.nvc_kor, JT2.nvc_eng, JT3.nvc_spc 
       FROM jft1 JT1 
         FULL JOIN jft2 JT2 ON (JT1.nvc_kor = JT2.nvc_kor) 
         LEFT JOIN jft3 JT3 ON (JT2.nvc_kor = JT3.nvc_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::TEXT,
      'abcdefghijklmnopqrstuvwxyz'::TEXT,
      '!@#$%^&*()<>/\\'''::TEXT
    )$$
  );

  -- TEST 30: LEFT OUTER JOIN + FULL OUTER JOIN 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.nc_kor, JT2.nc_eng, JT3.nc_spc 
      FROM jft1 JT1 
        LEFT JOIN jft2 JT2 ON (JT1.nc_kor = JT2.nc_kor) 
        FULL JOIN jft3 JT3 ON (JT2.nc_kor = JT3.nc_kor)',
    ARRAY['Hash Full Join',
      '  Output: jt1.nc_kor, jt2.nc_eng, jt3.nc_spc',
      '  Hash Cond: (jt2.nc_kor = jt3.nc_kor)',
      '  ->  Hash Left Join',
      '        Output: jt1.nc_kor, jt2.nc_eng, jt2.nc_kor',
      '        Hash Cond: (jt1.nc_kor = jt2.nc_kor)',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.c_kor, jt1.c_eng, jt1.c_spc, jt1.nc_kor, jt1.nc_eng, jt1.nc_spc, jt1.vc_kor, jt1.vc_eng, jt1.vc_spc, jt1.vc2_kor, jt1.vc2_eng, jt1.vc2_spc, jt1.nvc_kor, jt1.nvc_eng, jt1.nvc_spc, jt1.rwid, jt1.nm, jt1.flt, jt1.dt, jt1.ts, jt1.tsz, jt1.iytm, jt1.idts',
      '        ->  Hash',
      '              Output: jt2.nc_eng, jt2.nc_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.nc_eng, jt2.nc_kor',
      '  ->  Hash',
      '        Output: jt3.nc_spc, jt3.nc_kor',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.nc_spc, jt3.nc_kor'
    ]
  );

  -- TEST 31: JOIN 쿼리 실행 검증 
  SELECT results_eq('
    SELECT JT1.nc_kor, JT2.nc_eng, JT3.nc_spc 
      FROM jft1 JT1 
        LEFT JOIN jft2 JT2 ON (JT1.nc_kor = JT2.nc_kor) 
        FULL JOIN jft3 JT3 ON (JT2.nc_kor = JT3.nc_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::NCHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::NCHAR(100),
      '!@#$%^&*()<>/\\'''::NCHAR(100)
    )$$
  );

  -- TEST 32: RIGHT OUTER JOIN + LEFT OUTER JOIN (jft1, jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.c_kor, JT2.c_eng, JT3.c_spc 
      FROM jft1 JT1 
        RIGHT JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor) 
        LEFT JOIN jft3 JT3 ON (JT2.c_kor = JT3.c_kor)',
    ARRAY['Merge Left Join',
      '  Output: jt1.c_kor, jt2.c_eng, jt3.c_spc',
      '  Merge Cond: (jt2.c_kor = jt3.c_kor)',
      '  ->  Merge Left Join',
      '        Output: jt2.c_eng, jt2.c_kor, jt1.c_kor',
      '        Merge Cond: (jt2.c_kor = jt1.c_kor)',
      '        ->  Sort',
      '              Output: jt2.c_eng, jt2.c_kor',
      '              Sort Key: jt2.c_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.c_eng, jt2.c_kor',
      '        ->  Sort',
      '              Output: jt1.c_kor',
      '              Sort Key: jt1.c_kor',
      '              ->  Foreign Scan on public.jft1 jt1',
      '                    Output: jt1.c_kor',
      '  ->  Sort',
      '        Output: jt3.c_spc, jt3.c_kor',
      '        Sort Key: jt3.c_kor',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.c_spc, jt3.c_kor'
    ]
  );

  -- TEST 33: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.c_kor, JT2.c_eng, JT3.c_spc 
      FROM jft1 JT1 
        RIGHT JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor) 
        LEFT JOIN jft3 JT3 ON (JT2.c_kor = JT3.c_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::CHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::CHAR(100),
      '!@#$%^&*()<>/\\'''::CHAR(100)
    )$$
  );

  -- TEST 34: LEFT OUTER JOIN + RIGHT OUTER JOIN (jft1, jft2, jft3) 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.c_kor, JT2.c_eng, JT3.c_spc 
      FROM jft1 JT1 
        LEFT JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor) 
        RIGHT JOIN jft3 JT3 ON (JT2.c_kor = JT3.c_kor)',
    ARRAY['Merge Right Join',
      '  Output: jt1.c_kor, jt2.c_eng, jt3.c_spc',
      '  Merge Cond: (jt2.c_kor = jt3.c_kor)',
      '  ->  Merge Join',
      '        Output: jt1.c_kor, jt2.c_eng, jt2.c_kor',
      '        Merge Cond: (jt1.c_kor = jt2.c_kor)',
      '        ->  Sort',
      '              Output: jt1.c_kor',
      '              Sort Key: jt1.c_kor',
      '              ->  Foreign Scan on public.jft1 jt1',
      '                    Output: jt1.c_kor',
      '        ->  Sort',
      '              Output: jt2.c_eng, jt2.c_kor',
      '              Sort Key: jt2.c_kor',
      '              ->  Foreign Scan on public.jft2 jt2',
      '                    Output: jt2.c_eng, jt2.c_kor',
      '  ->  Sort',
      '        Output: jt3.c_spc, jt3.c_kor',
      '        Sort Key: jt3.c_kor',
      '        ->  Foreign Scan on public.jft3 jt3',
      '              Output: jt3.c_spc, jt3.c_kor'
    ]
  );

  -- TEST 35: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.c_kor, JT2.c_eng, JT3.c_spc 
      FROM jft1 JT1 
        LEFT JOIN jft2 JT2 ON (JT1.c_kor = JT2.c_kor) 
        RIGHT JOIN jft3 JT3 ON (JT2.c_kor = JT3.c_kor)',
    $$VALUES (
      '가나다라마바사아자차카타파하'::CHAR(100),
      'abcdefghijklmnopqrstuvwxyz'::CHAR(100),
      '!@#$%^&*()<>/\\'''::CHAR(100)
    )$$
  );

  -- TEST 36: xxxx JOIN xxxx 쿼리 플랜 검증
  SELECT results_eq('
    EXPLAIN (VERBOSE, COSTS OFF) SELECT JT1.flt, JT2.flt 
      FROM jft1 JT1 
      CROSS JOIN jft2 JT2 
    ORDER BY JT1.flt, JT2.flt',
    ARRAY['Sort',
      '  Output: jt1.flt, jt2.flt',
      '  Sort Key: jt1.flt, jt2.flt',
      '  ->  Nested Loop',
      '        Output: jt1.flt, jt2.flt',
      '        ->  Foreign Scan on public.jft1 jt1',
      '              Output: jt1.c_kor, jt1.c_eng, jt1.c_spc, jt1.nc_kor, jt1.nc_eng, jt1.nc_spc, jt1.vc_kor, jt1.vc_eng, jt1.vc_spc, jt1.vc2_kor, jt1.vc2_eng, jt1.vc2_spc, jt1.nvc_kor, jt1.nvc_eng, jt1.nvc_spc, jt1.rwid, jt1.nm, jt1.flt, jt1.dt, jt1.ts, jt1.tsz, jt1.iytm, jt1.idts',
      '        ->  Foreign Scan on public.jft2 jt2',
      '              Output: jt2.c_kor, jt2.c_eng, jt2.c_spc, jt2.nc_kor, jt2.nc_eng, jt2.nc_spc, jt2.vc_kor, jt2.vc_eng, jt2.vc_spc, jt2.vc2_kor, jt2.vc2_eng, jt2.vc2_spc, jt2.nvc_kor, jt2.nvc_eng, jt2.nvc_spc, jt2.rwid, jt2.nm, jt2.flt, jt2.dt, jt2.ts, jt2.tsz, jt2.iytm, jt2.idts'
    ]
  );

  -- TEST 37: JOIN 쿼리 실행 검증
  SELECT results_eq('
    SELECT JT1.flt, JT2.flt 
      FROM jft1 JT1 
      CROSS JOIN jft2 JT2 
    ORDER BY JT1.flt, JT2.flt',
    $$VALUES (
      123456.123456789::FLOAT,
      123456.123456789::FLOAT
    )$$
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();

ROLLBACK;
