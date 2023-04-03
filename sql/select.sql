-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(44);

  CREATE EXTENSION IF NOT EXISTS tibero_fdw;

  CREATE SERVER server_name FOREIGN DATA WRAPPER tibero_fdw
    OPTIONS (host :TIBERO_HOST, port :TIBERO_PORT, dbname :TIBERO_DB);

  CREATE USER MAPPING FOR current_user
    SERVER server_name
    OPTIONS (username :TIBERO_USER, password :TIBERO_PASS);

  CREATE FOREIGN TABLE fst1 (
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
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'st1');

  CREATE FOREIGN TABLE fst2 (
      c1 INTEGER,
      c2 VARCHAR(14),
      c3 VARCHAR(13)
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'st2');

  -- TEST 1:
  SELECT lives_ok(
    'SELECT * FROM fst1;'
  );

  -- TEST 2:
  SELECT lives_ok(
    'SELECT * FROM fst2;'
  );

  -- TEST 3:
  SELECT results_eq('
    SELECT c1, c2, c3, c4, c5, c6, c7, c8, c9, c10
      FROM fst1
      ORDER BY c1 DESC, c8',
    $$VALUES 
      (1400::INT, 'HS14'::VARCHAR, 'KOREA'::CHAR(9), 700::BIGINT, 
      '1982-01-23'::DATE, 1300.00000::DECIMAL, null::INT, 10::SMALLINT, 
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (1300::INT, 'HS13'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1981-12-03'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'mmmmm'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT),
      (1200::INT, 'HS12'::VARCHAR, 'KOREA'::CHAR(9), 600::BIGINT,
      '1981-12-03'::DATE, 950.00000::DECIMAL, null::INT, 30::SMALLINT,
      'hhhhh'::NCHAR(9), '타타타타타타타타타타타타타타'::TEXT),
      (1100::INT, 'HS11'::VARCHAR, 'KOREA'::CHAR(9), 800::BIGINT,
      '1987-05-23'::DATE, 1100.00000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '바바바바바바바바바바바바바바'::TEXT),
      (1000::INT, 'HS10'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1980-09-08'::DATE, 1500.00000::DECIMAL, 0::INT, 30::SMALLINT,
      'jjjjj'::NCHAR(9), '차차차차차차차차차차차차차차'::TEXT),
      (900::INT, 'HS9'::VARCHAR, 'JEJU'::CHAR(9), null::BIGINT,
      '1981-11-17'::DATE, 5000.00000::DECIMAL, null::INT, 10::SMALLINT,
      'iiiii'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (800::INT, 'HS8'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1987-04-19'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'hhhhh'::NCHAR(9), '아아아아아아아아아아아아아아'::TEXT),
      (700::INT, 'HS7'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-06-09'::DATE, 2450.45000::DECIMAL, null::INT, 10::SMALLINT,
      'ggggg'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT),
      (600::INT, 'HS6'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-05-01'::DATE, 2850.00000::DECIMAL, null::INT, 30::SMALLINT,
      'fffff'::NCHAR(9), '바바바바바바바바바바바바바바'::TEXT),
      (500::INT, 'HS5'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1981-09-28'::DATE, 1250.00000::DECIMAL, 1400::INT, 30::SMALLINT,
      'hhhhh'::NCHAR(9), '마마마마마마마마마마마마마마'::TEXT),
      (400::INT, 'HS4'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-04-02'::DATE, 2975.12000::DECIMAL, null::INT, 20::SMALLINT,
      'ddddd'::NCHAR(9), '라라라라라라라라라라라라라라'::TEXT),
      (300::INT, 'HS3'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1981-02-22'::DATE, 1250.00000::DECIMAL, 500::INT, 30::SMALLINT,
      'hhhhh'::NCHAR(9), '다다다다다다다다다다다다다다'::TEXT),
      (200::INT, 'HS2'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1981-02-20'::DATE, 1600.00000::DECIMAL, 300::INT, 30::SMALLINT,
      'bbbbb'::NCHAR(9), '나나나나나나나나나나나나나나'::TEXT),
      (100::INT, 'HS1'::VARCHAR, 'KOREA'::CHAR(9), 1300::BIGINT,
      '1980-12-17'::DATE, 800.23000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT)
    $$
  );

  -- TEST 4:
  SELECT results_eq('
    SELECT DISTINCT c8 FROM fst1 ORDER BY 1',
    ARRAY [ 10::SMALLINT, 20::SMALLINT, 30::SMALLINT ]
  );

  -- TEST 5:
  SELECT results_eq('
    SELECT c2 AS COUNTRY_Name FROM fst1 ORDER BY 1',
    ARRAY [ 'HS1'::VARCHAR, 'HS10'::VARCHAR, 'HS11'::VARCHAR,
      'HS12'::VARCHAR, 'HS13'::VARCHAR, 'HS14'::VARCHAR,
      'HS2'::VARCHAR, 'HS3'::VARCHAR, 'HS4'::VARCHAR,
      'HS5'::VARCHAR, 'HS6'::VARCHAR, 'HS7'::VARCHAR,
      'HS8'::VARCHAR, 'HS9'::VARCHAR]
  );

  -- TEST 6:
  SELECT results_eq('
    SELECT c8, c6, c7 FROM fst1 ORDER BY 1, 2, 3',
    $$VALUES
      (10::SMALLINT, 1300.00000::DECIMAL, null::INT),
      (10::SMALLINT, 2450.45000::DECIMAL, null::INT),
      (10::SMALLINT, 5000.00000::DECIMAL, null::INT),
      (20::SMALLINT, 800.23000::DECIMAL, null::INT),
      (20::SMALLINT, 1100.00000::DECIMAL, null::INT),
      (20::SMALLINT, 2975.12000::DECIMAL, null::INT),
      (20::SMALLINT, 3000.00000::DECIMAL, null::INT),
      (20::SMALLINT, 3000.00000::DECIMAL, null::INT),
      (30::SMALLINT, 950.00000::DECIMAL, null::INT),
      (30::SMALLINT, 1250.00000::DECIMAL, 500::INT),
      (30::SMALLINT, 1250.00000::DECIMAL, 1400::INT),
      (30::SMALLINT, 1500.00000::DECIMAL, 0::INT),
      (30::SMALLINT, 1600.00000::DECIMAL, 300::INT),
      (30::SMALLINT, 2850.00000::DECIMAL, null::INT)
    $$
  );

  -- TEST 7:
  SELECT results_eq('
    SELECT c1, c2, c3, c4, c5, c6, c7, c8, c9, c10 FROM fst1 WHERE c1 = 100 ORDER BY 1;
    ',
    $$VALUES 
      (100::INT, 'HS1'::VARCHAR, 'KOREA'::CHAR(9), 1300::BIGINT,
      '1980-12-17'::DATE, 800.23000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT)
    $$
  );

  -- TEST 8:
  SELECT results_eq('
    SELECT c1, c2, c3, c4, c5, c6, c7, c8, c9, c10
      FROM fst1 WHERE c1 = 100 OR c1 = 700 ORDER BY 1',
    $$VALUES
      (100::INT, 'HS1'::VARCHAR, 'KOREA'::CHAR(9), 1300::BIGINT,
      '1980-12-17'::DATE, 800.23000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (700::INT, 'HS7'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-06-09'::DATE, 2450.45000::DECIMAL, null::INT, 10::SMALLINT,
      'ggggg'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT)
    $$
  );

  -- TEST 9:
  SELECT results_eq('
    SELECT * FROM fst1 WHERE c3 like ''JAPAN%'' ORDER BY 1;',
    $$VALUES
      (200::INT, 'HS2'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1981-02-20'::DATE, 1600.00000::DECIMAL, 300::INT, 30::SMALLINT,
      'bbbbb'::NCHAR(9), '나나나나나나나나나나나나나나'::TEXT),
      (300::INT, 'HS3'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1981-02-22'::DATE, 1250.00000::DECIMAL, 500::INT, 30::SMALLINT,
      'hhhhh'::NCHAR(9), '다다다다다다다다다다다다다다'::TEXT),
      (500::INT, 'HS5'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1981-09-28'::DATE, 1250.00000::DECIMAL, 1400::INT, 30::SMALLINT,
      'hhhhh'::NCHAR(9), '마마마마마마마마마마마마마마'::TEXT),
      (1000::INT, 'HS10'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1980-09-08'::DATE, 1500.00000::DECIMAL, 0::INT, 30::SMALLINT,
      'jjjjj'::NCHAR(9), '차차차차차차차차차차차차차차'::TEXT)
    $$
  );

  -- TEST 10:
  SELECT results_eq('
    SELECT * FROM fst1 WHERE c1 IN (100, 700) ORDER BY 1;',
    $$VALUES
      (100::INT, 'HS1'::VARCHAR, 'KOREA'::CHAR(9), 1300::BIGINT,
      '1980-12-17'::DATE, 800.23000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (700::INT, 'HS7'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-06-09'::DATE, 2450.45000::DECIMAL, null::INT, 10::SMALLINT,
      'ggggg'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT)
    $$
  );

  -- TEST 11:
  SELECT results_eq('
    SELECT * 
      FROM fst1 WHERE c9 NOT IN (''aaaaa'', ''hhhhh'') 
      ORDER BY 1 LIMIT 5',
    $$VALUES
      (200::INT, 'HS2'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1981-02-20'::DATE, 1600.00000::DECIMAL, 300::INT, 30::SMALLINT,
      'bbbbb'::NCHAR(9), '나나나나나나나나나나나나나나'::TEXT),
      (400::INT, 'HS4'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-04-02'::DATE, 2975.12000::DECIMAL, null::INT, 20::SMALLINT,
      'ddddd'::NCHAR(9), '라라라라라라라라라라라라라라'::TEXT),
      (600::INT, 'HS6'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-05-01'::DATE, 2850.00000::DECIMAL, null::INT, 30::SMALLINT,
      'fffff'::NCHAR(9), '바바바바바바바바바바바바바바'::TEXT),
      (700::INT, 'HS7'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-06-09'::DATE, 2450.45000::DECIMAL, null::INT, 10::SMALLINT,
      'ggggg'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT),
      (900::INT, 'HS9'::VARCHAR, 'JEJU'::CHAR(9), null::BIGINT,
      '1981-11-17'::DATE, 5000.00000::DECIMAL, null::INT, 10::SMALLINT,
      'iiiii'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT)
    $$
  );

  -- TEST 12:
  SELECT results_eq('
    SELECT * FROM fst1 WHERE c8 BETWEEN 10 AND 20 ORDER BY 1;',
    $$VALUES
      (100::INT, 'HS1'::VARCHAR, 'KOREA'::CHAR(9), 1300::BIGINT,
      '1980-12-17'::DATE, 800.23000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (400::INT, 'HS4'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-04-02'::DATE, 2975.12000::DECIMAL, null::INT, 20::SMALLINT,
      'ddddd'::NCHAR(9), '라라라라라라라라라라라라라라'::TEXT),
      (700::INT, 'HS7'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-06-09'::DATE, 2450.45000::DECIMAL, null::INT, 10::SMALLINT,
      'ggggg'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT),
      (800::INT, 'HS8'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1987-04-19'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'hhhhh'::NCHAR(9), '아아아아아아아아아아아아아아'::TEXT),
      (900::INT, 'HS9'::VARCHAR, 'JEJU'::CHAR(9), null::BIGINT,
      '1981-11-17'::DATE, 5000.00000::DECIMAL, null::INT, 10::SMALLINT,
      'iiiii'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (1100::INT, 'HS11'::VARCHAR, 'KOREA'::CHAR(9), 800::BIGINT,
      '1987-05-23'::DATE, 1100.00000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '바바바바바바바바바바바바바바'::TEXT),
      (1300::INT, 'HS13'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1981-12-03'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'mmmmm'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT),
      (1400::INT, 'HS14'::VARCHAR, 'KOREA'::CHAR(9), 700::BIGINT,
      '1982-01-23'::DATE, 1300.00000::DECIMAL, null::INT, 10::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT)
    $$
  );

  -- TEST 13:
  SELECT results_eq('
    SELECT * FROM fst1 ORDER BY 1 OFFSET 5;',
    $$VALUES
      (600::INT, 'HS6'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-05-01'::DATE, 2850.00000::DECIMAL, null::INT, 30::SMALLINT,
      'fffff'::NCHAR(9), '바바바바바바바바바바바바바바'::TEXT),
      (700::INT, 'HS7'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-06-09'::DATE, 2450.45000::DECIMAL, null::INT, 10::SMALLINT,
      'ggggg'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT),
      (800::INT, 'HS8'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1987-04-19'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'hhhhh'::NCHAR(9), '아아아아아아아아아아아아아아'::TEXT),
      (900::INT, 'HS9'::VARCHAR, 'JEJU'::CHAR(9), null::BIGINT,
      '1981-11-17'::DATE, 5000.00000::DECIMAL, null::INT, 10::SMALLINT,
      'iiiii'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (1000::INT, 'HS10'::VARCHAR, 'JAPAN'::CHAR(9), 600::BIGINT,
      '1980-09-08'::DATE, 1500.00000::DECIMAL, 0::INT, 30::SMALLINT,
      'jjjjj'::NCHAR(9), '차차차차차차차차차차차차차차'::TEXT),
      (1100::INT, 'HS11'::VARCHAR, 'KOREA'::CHAR(9), 800::BIGINT,
      '1987-05-23'::DATE, 1100.00000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '바바바바바바바바바바바바바바'::TEXT),
      (1200::INT, 'HS12'::VARCHAR, 'KOREA'::CHAR(9), 600::BIGINT,
      '1981-12-03'::DATE, 950.00000::DECIMAL, null::INT, 30::SMALLINT,
      'hhhhh'::NCHAR(9), '타타타타타타타타타타타타타타'::TEXT),
      (1300::INT, 'HS13'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1981-12-03'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'mmmmm'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT),
      (1400::INT, 'HS14'::VARCHAR, 'KOREA'::CHAR(9), 700::BIGINT,
      '1982-01-23'::DATE, 1300.00000::DECIMAL, null::INT, 10::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT)
    $$
  );

  -- TEST 14:
  SELECT results_eq('
    SELECT c8 SMALL_CNT, COUNT(c1) Total_CNT FROM fst1 GROUP BY c8 ORDER BY c8;',
    $$VALUES
      (10::SMALLINT, 3::BIGINT),
      (20::SMALLINT, 5::BIGINT),
      (30::SMALLINT, 6::BIGINT)
    $$
  );

  -- TEST 15:
  SELECT results_eq('
    SELECT c8, SUM(c6) FROM fst1 GROUP BY c8 HAVING c8 IN (10, 30) ORDER BY c8;',
    $$VALUES
      (10::SMALLINT, 8750.45000::DECIMAL),
      (30::SMALLINT, 9400.00000::DECIMAL)
    $$
  );

  -- TEST 16:
  SELECT results_eq('
    SELECT c8, SUM(c6) FROM fst1 GROUP BY c8 HAVING SUM(c6) > 9400 ORDER BY c8;',
    $$VALUES
      (20::SMALLINT, 10875.35000::DECIMAL)
    $$
  );

  -- TEST 17:
  SELECT results_eq('
    SELECT UPPER(c2), LOWER(c2) FROM fst2 ORDER BY 1, 2;',
    $$VALUES
      ('EAST'::TEXT, 'east'::TEXT),
      ('NORTH', 'north'),
      ('SOUTH', 'south'),
      ('WEST', 'west')
    $$
  );

  -- TEST 18:
  SELECT results_eq('
    SELECT * FROM fst1 
      WHERE c8 <> ALL (SELECT c1 FROM fst2 WHERE c1 IN (10, 30, 40)) 
      ORDER BY c1;',
    $$VALUES
      (100::INT, 'HS1'::VARCHAR, 'KOREA'::CHAR(9), 1300::BIGINT,
      '1980-12-17'::DATE, 800.23000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '가가가가가가가가가가가가가가'::TEXT),
      (400::INT, 'HS4'::VARCHAR, 'CHINA'::CHAR(9), 900::BIGINT,
      '1981-04-02'::DATE, 2975.12000::DECIMAL, null::INT, 20::SMALLINT,
      'ddddd'::NCHAR(9), '라라라라라라라라라라라라라라'::TEXT),
      (800::INT, 'HS8'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1987-04-19'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'hhhhh'::NCHAR(9), '아아아아아아아아아아아아아아'::TEXT),
      (1100::INT, 'HS11'::VARCHAR, 'KOREA'::CHAR(9), 800::BIGINT,
      '1987-05-23'::DATE, 1100.00000::DECIMAL, null::INT, 20::SMALLINT,
      'aaaaa'::NCHAR(9), '바바바바바바바바바바바바바바'::TEXT),
      (1300::INT, 'HS13'::VARCHAR, 'AMERICA'::CHAR(9), 400::BIGINT,
      '1981-12-03'::DATE, 3000.00000::DECIMAL, null::INT, 20::SMALLINT,
      'mmmmm'::NCHAR(9), '사사사사사사사사사사사사사사'::TEXT)
    $$
  );

  -- TEST 19:
  SELECT results_eq('
    SELECT c1, c2, c3 FROM fst2
      WHERE EXISTS (SELECT 1 FROM fst1 WHERE fst2.c1 = fst1.c8)
      ORDER BY 1, 2;',
    $$VALUES
      (10::INTEGER, 'EAST'::VARCHAR, 'PUNE'::VARCHAR),
      (20::INTEGER, 'WEST'::VARCHAR, 'BANGLORE'::VARCHAR),
      (30::INTEGER, 'NORTH'::VARCHAR, 'MUMBAI'::VARCHAR)
    $$
  );

  -- TEST 20:
  SELECT results_eq('
    SELECT c1, c2, c3, c4, c5, c6, c7, c8, c9, c10 FROM fst1 
      WHERE c8 NOT IN (SELECT c1 FROM fst2) 
      ORDER BY c1;',
    ARRAY[]::VARCHAR[]
  );

  -- TEST 21: UNION
  SELECT results_eq('
    SELECT c1, c2 FROM fst2 UNION
      SELECT c1, c2 FROM fst1 ORDER BY c1',
    $$VALUES
      (10::INT, 'EAST'::VARCHAR), (20::INT, 'WEST'::VARCHAR), (30::INT, 'NORTH'::VARCHAR),
      (40::INT, 'SOUTH'::VARCHAR), (100::INT, 'HS1'::VARCHAR), (200::INT, 'HS2'::VARCHAR),
      (300::INT, 'HS3'::VARCHAR), (400::INT, 'HS4'::VARCHAR), (500::INT, 'HS5'::VARCHAR),
      (600::INT, 'HS6'::VARCHAR), (700::INT, 'HS7'::VARCHAR), (800::INT, 'HS8'::VARCHAR),
      (900::INT, 'HS9'::VARCHAR), (1000::INT, 'HS10'::VARCHAR), (1100::INT, 'HS11'::VARCHAR),
      (1200::INT, 'HS12'::VARCHAR), (1300::INT, 'HS13'::VARCHAR), (1400::INT, 'HS14'::VARCHAR)
    $$
  );

  -- TEST 22: UNION ALL
  SELECT results_eq('
    SELECT c2 FROM fst2 UNION ALL
      SELECT c2 FROM fst1 ORDER BY c2;',
    ARRAY[ 'EAST'::VARCHAR, 'HS1', 'HS10', 'HS11', 'HS12', 'HS13', 'HS14', 'HS2',
      'HS3', 'HS4', 'HS5', 'HS6', 'HS7', 'HS8', 'HS9', 'NORTH', 'SOUTH', 'WEST' ]
  );

  -- TEST 23:
  SELECT results_eq('
    SELECT c2 FROM fst1 WHERE c1 >= 800 INTERSECT
      SELECT c2 FROM fst1 WHERE c1 >= 400 ORDER BY c2',
    ARRAY[ 'HS10'::VARCHAR, 'HS11', 'HS12', 'HS13', 'HS14', 'HS8', 'HS9' ]
  );

  -- TEST 24:
  SELECT results_eq('
    SELECT c2 FROM fst1 WHERE c1 >= 800 INTERSECT ALL
      SELECT c2 FROM fst1 WHERE c1 >= 400 ORDER BY c2',
    ARRAY[ 'HS10'::VARCHAR, 'HS11', 'HS12', 'HS13', 'HS14', 'HS8', 'HS9' ]
  );

  -- TEST 25:
  SELECT results_eq('
    SELECT c2 FROM fst1 EXCEPT
      SELECT c2 FROM fst1 WHERE c1 > 900 ORDER BY c2',
    ARRAY [ 'HS1'::VARCHAR, 'HS2', 'HS3', 'HS4', 'HS5', 'HS6', 'HS7', 'HS8', 'HS9' ]
  );

  -- TEST 26:
  SELECT results_eq('
    SELECT c2 FROM fst1 EXCEPT ALL
      SELECT c2 FROM fst1 WHERE c1 > 900 ORDER BY c2',
    ARRAY [ 'HS1'::VARCHAR, 'HS2', 'HS3', 'HS4', 'HS5', 'HS6', 'HS7', 'HS8', 'HS9' ]
  );

  -- TEST 27:
  SELECT results_eq('
    WITH with_qry AS (SELECT c1, c2, c3 FROM fst2)
    SELECT e.c2, e.c6, w.c1, w.c2 
      FROM fst1 e, with_qry w
      WHERE e.c8 = w.c1 ORDER BY e.c8, e.c2;',
    $$VALUES
      ('HS14'::VARCHAR, 1300.00000::DECIMAL, 10::INT, 'EAST'::VARCHAR),
      ('HS7'::VARCHAR, 2450.45000::DECIMAL, 10::INT, 'EAST'::VARCHAR),
      ('HS9'::VARCHAR, 5000.00000::DECIMAL, 10::INT, 'EAST'::VARCHAR),
      ('HS1'::VARCHAR, 800.23000::DECIMAL, 20::INT, 'WEST'::VARCHAR),
      ('HS11'::VARCHAR, 1100.00000::DECIMAL, 20::INT, 'WEST'::VARCHAR),
      ('HS13'::VARCHAR, 3000.00000::DECIMAL, 20::INT, 'WEST'::VARCHAR),
      ('HS4'::VARCHAR, 2975.12000::DECIMAL, 20::INT, 'WEST'::VARCHAR),
      ('HS8'::VARCHAR, 3000.00000::DECIMAL, 20::INT, 'WEST'::VARCHAR),
      ('HS10'::VARCHAR, 1500.00000::DECIMAL, 30::INT, 'NORTH'::VARCHAR),
      ('HS12'::VARCHAR, 950.00000::DECIMAL, 30::INT, 'NORTH'::VARCHAR),
      ('HS2'::VARCHAR, 1600.00000::DECIMAL, 30::INT, 'NORTH'::VARCHAR),
      ('HS3'::VARCHAR, 1250.00000::DECIMAL, 30::INT, 'NORTH'::VARCHAR),
      ('HS5'::VARCHAR, 1250.00000::DECIMAL, 30::INT, 'NORTH'::VARCHAR),
      ('HS6'::VARCHAR, 2850.00000::DECIMAL, 30::INT, 'NORTH'::VARCHAR)
    $$
  );

  -- TEST 28:
  SELECT results_eq('
    WITH 
      test_tbl2_costs AS (SELECT d.c2, SUM(c6) test_tbl2_total FROM fst1 e, fst2 d
                          WHERE e.c8 = d.c1 GROUP BY 1),
      avg_cost AS (SELECT SUM(test_tbl2_total)/COUNT(*) avg 
                   FROM test_tbl2_costs)
    SELECT * FROM test_tbl2_costs
      WHERE test_tbl2_total > (SELECT avg FROM avg_cost) ORDER BY c2;',
    $$VALUES
      ('WEST'::VARCHAR, 10875.35000::DECIMAL)
    $$
  );

  -- TEST 29:
  SELECT results_eq('
    SELECT c8, c1, c6, AVG(c6) OVER (PARTITION BY c8) 
      FROM fst1 ORDER BY c8, c1',
    $$VALUES
      (10::SMALLINT, 700::INT, 2450.45000::DECIMAL, 2916.8166666666666667::NUMERIC),
      (10::SMALLINT, 900::INT, 5000.00000::DECIMAL, 2916.8166666666666667::NUMERIC),
      (10::SMALLINT, 1400::INT, 1300.00000::DECIMAL, 2916.8166666666666667::NUMERIC),
      (20::SMALLINT, 100::INT, 800.23000::DECIMAL, 2175.0700000000000000::NUMERIC),
      (20::SMALLINT, 400::INT, 2975.12000::DECIMAL, 2175.0700000000000000::NUMERIC),
      (20::SMALLINT, 800::INT, 3000.00000::DECIMAL, 2175.0700000000000000::NUMERIC),
      (20::SMALLINT, 1100::INT, 1100.00000::DECIMAL, 2175.0700000000000000::NUMERIC),
      (20::SMALLINT, 1300::INT, 3000.00000::DECIMAL, 2175.0700000000000000::NUMERIC),
      (30::SMALLINT, 200::INT, 1600.00000::DECIMAL, 1566.6666666666666667::NUMERIC),
      (30::SMALLINT, 300::INT, 1250.00000::DECIMAL, 1566.6666666666666667::NUMERIC),
      (30::SMALLINT, 500::INT, 1250.00000::DECIMAL, 1566.6666666666666667::NUMERIC),
      (30::SMALLINT, 600::INT, 2850.00000::DECIMAL, 1566.6666666666666667::NUMERIC),
      (30::SMALLINT, 1000::INT, 1500.00000::DECIMAL, 1566.6666666666666667::NUMERIC),
      (30::SMALLINT, 1200::INT, 950.00000::DECIMAL, 1566.6666666666666667::NUMERIC)
    $$
  );

  -- TEST 30:
  SELECT results_eq('
    SELECT c8, c1, c6, COUNT(c6) OVER (PARTITION BY c8) 
      FROM fst1 WHERE c8 IN (10, 30, 40, 50, 60, 70) 
      ORDER BY c8, c1',
    $$VALUES
      (10::SMALLINT, 700::INT, 2450.45000::DECIMAL, 3::BIGINT),
      (10::SMALLINT, 900::INT, 5000.00000::DECIMAL, 3::BIGINT),
      (10::SMALLINT, 1400::INT, 1300.00000::DECIMAL, 3::BIGINT),
      (30::SMALLINT, 200::INT, 1600.00000::DECIMAL, 6::BIGINT),
      (30::SMALLINT, 300::INT, 1250.00000::DECIMAL, 6::BIGINT),
      (30::SMALLINT, 500::INT, 1250.00000::DECIMAL, 6::BIGINT),
      (30::SMALLINT, 600::INT, 2850.00000::DECIMAL, 6::BIGINT),
      (30::SMALLINT, 1000::INT, 1500.00000::DECIMAL, 6::BIGINT),
      (30::SMALLINT, 1200::INT, 950.00000::DECIMAL, 6::BIGINT)
    $$
  );

  -- TEST 31:
  SELECT results_eq('
    SELECT c8, c1, c6, SUM(c6) OVER (PARTITION BY c8) 
      FROM fst1 
      ORDER BY c8, c1;',
    $$VALUES
      (10::SMALLINT, 700::INT, 2450.45000::DECIMAL, 8750.45000::NUMERIC),
      (10::SMALLINT, 900::INT, 5000.00000::DECIMAL, 8750.45000::NUMERIC),
      (10::SMALLINT, 1400::INT, 1300.00000::DECIMAL, 8750.45000::NUMERIC),
      (20::SMALLINT, 100::INT, 800.23000::DECIMAL, 10875.35000::NUMERIC),
      (20::SMALLINT, 400::INT, 2975.12000::DECIMAL, 10875.35000::NUMERIC),
      (20::SMALLINT, 800::INT, 3000.00000::DECIMAL, 10875.35000::NUMERIC),
      (20::SMALLINT, 1100::INT, 1100.00000::DECIMAL, 10875.35000::NUMERIC),
      (20::SMALLINT, 1300::INT, 3000.00000::DECIMAL, 10875.35000::NUMERIC),
      (30::SMALLINT, 200::INT, 1600.00000::DECIMAL, 9400.00000::NUMERIC),
      (30::SMALLINT, 300::INT, 1250.00000::DECIMAL, 9400.00000::NUMERIC),
      (30::SMALLINT, 500::INT, 1250.00000::DECIMAL, 9400.00000::NUMERIC),
      (30::SMALLINT, 600::INT, 2850.00000::DECIMAL, 9400.00000::NUMERIC),
      (30::SMALLINT, 1000::INT, 1500.00000::DECIMAL, 9400.00000::NUMERIC),
      (30::SMALLINT, 1200::INT, 950.00000::DECIMAL, 9400.00000::NUMERIC)
    $$
  );

  CREATE FOREIGN TABLE null_test_table (
      null_c CHAR(100),
      null_nc NCHAR(100),
      null_vc VARCHAR(100),
      null_vc2 TEXT,
      null_nvc TEXT,
      null_rwid TEXT,
      null_nm NUMERIC,
      null_flt FLOAT,
      null_dt DATE,
      null_ts TIMESTAMP,
      null_tsz TIMESTAMP WITH TIME ZONE,
      null_iytm INTERVAL YEAR TO MONTH,
      null_idts INTERVAL DAY TO SECOND
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'null_test_table');

  -- TEST 32:
  SELECT results_eq('
    SELECT coalesce(null_c, ''THIS DATA IS NULL'') AS null_c FROM null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::CHAR(20),
      'THIS DATA IS NULL'::CHAR(20),
      'THIS DATA IS NULL'::CHAR(20)
    ]
  );

  -- TEST 33:
  SELECT results_eq('
    SELECT coalesce(null_nc, ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::NCHAR(20),
      'THIS DATA IS NULL'::NCHAR(20),
      'THIS DATA IS NULL'::NCHAR(20)
    ]
  );

  -- TEST 34:
  SELECT results_eq('
    SELECT coalesce(null_vc, ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::VARCHAR,
      'THIS DATA IS NULL'::VARCHAR,
      'THIS DATA IS NULL'::VARCHAR
    ]
  );

  -- TEST 35:
  SELECT results_eq('
    SELECT coalesce(null_vc2, ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- TEST 36:
  SELECT results_eq('
    SELECT coalesce(null_nvc, ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- TEST 37:
  SELECT results_eq('
    SELECT coalesce(null_rwid, ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- TEST 38:
  SELECT results_eq('
    SELECT num_nulls(null_nm) FROM null_test_table',
    ARRAY[
      1, 1, 1
    ]
  );

  -- TEST 39:
  SELECT results_eq('
    SELECT num_nulls(null_flt) FROM null_test_table',
    ARRAY [
      1, 1, 1
    ]
  );

  -- TEST 40:
  SELECT results_eq('
    SELECT coalesce(TEXT(null_dt), ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- TEST 41:
  SELECT results_eq('
    SELECT coalesce(TEXT(null_ts), ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- TEST 42:
  SELECT results_eq('
    SELECT coalesce(TEXT(null_tsz), ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- TEST 43:
  SELECT results_eq('
    SELECT coalesce(TEXT(null_iytm), ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- TEST 44:
  SELECT results_eq('
    SELECT coalesce(TEXT(null_idts), ''THIS DATA IS NULL'') as null_c from null_test_table',
    ARRAY[
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT,
      'THIS DATA IS NULL'::TEXT
    ]
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();

ROLLBACK;
