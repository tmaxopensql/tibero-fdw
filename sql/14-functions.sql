-- Start transaction and plan the tests.
BEGIN;
  CREATE EXTENSION IF NOT EXISTS pgtap;

  SELECT plan(41);

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

  CREATE FOREIGN TABLE math_func_test_table (
      nb_default NUMERIC,
      nb_380 NUMERIC(38,0),
      nb_38191 NUMERIC(38,19),
      nb_38192 NUMERIC(38,19),
      nb_ltm NUMERIC,
      nb_gtm NUMERIC(130,130),
      flt FLOAT
  ) SERVER server_name OPTIONS (owner_name :TIBERO_USER, table_name 'math_func_test_table');

  -- TEST 1:
  SELECT results_eq('
    SELECT nb_default < nb_380 FROM math_func_test_table;',
    ARRAY[
      True, False, True, True, True, False, True, False, True
    ]
  );

  -- TEST 2:
  SELECT results_eq('
    SELECT nb_default > nb_380 FROM math_func_test_table;',
    ARRAY[
      False, False, False, False, False, True, False, False, False
    ]
  );

  -- TEST 3:
  SELECT results_eq('
    SELECT nb_default <= nb_380 FROM math_func_test_table;',
    ARRAY[
      True, True, True, True, True, False, True, True, True
    ]
  );

  -- TEST 4:
  SELECT results_eq('
    SELECT nb_default >= nb_380 FROM math_func_test_table;',
    ARRAY[
      False, True, False, False, False, True, False, True, False
    ]
  );

  -- TEST 5:
  SELECT results_eq('
    SELECT nb_default = nb_380 FROM math_func_test_table;',
    ARRAY[
      False, True, False, False, False, False, False, True, False
    ]
  );

  -- TEST 6:
  SELECT results_eq('
    SELECT nb_default <> nb_380 FROM math_func_test_table;',
    ARRAY[
      True, False, True, True, True, True, True, False, True
    ]
  );

  -- TEST 7:
  SELECT results_eq('
    SELECT nb_default != nb_380 FROM math_func_test_table;',
    ARRAY[
      True, False, True, True, True, True, True, False, True
    ]
  );

  -- TEST 8:
  SELECT results_eq('
    SELECT nb_38191 + nb_38192 FROM math_func_test_table;',
    ARRAY[
      -5888888.8888896000000000000::NUMERIC,
      -246912.2469120000000000000,
      246912.2469120000000000000,
      0.0000000000000000000,
      -7197531.1358016000000000000,
      0.0000000000000000000,
      0.0000000000000000000,
      -130.8642000000000000000,
      -77197531.1358016000000000000
    ]
  );

  -- TEST 9:
  SELECT results_eq('
    SELECT + nb_38191 FROM math_func_test_table;',
    ARRAY[
      654321.1234560000000000000::NUMERIC,
      -123456.1234560000000000000,
      123456.1234560000000000000,
      65.4321000000000000000,
      -654321.1234560000000000000,
      123456.1234560000000000000,
      -123456.1234560000000000000,
      -65.4321000000000000000,
      -654321.1234560000000000000
    ]
  );

  -- TEST 10:
  SELECT results_eq('
    SELECT nb_38191 - nb_38192 FROM math_func_test_table;',
    ARRAY[
      7197531.1358016000000000000::NUMERIC,
      0.0000000000000000000,
      0.0000000000000000000,
      130.8642000000000000000,
      5888888.8888896000000000000,
      246912.2469120000000000000,
      -246912.2469120000000000000,
      0.0000000000000000000,
      75888888.8888896000000000000
    ]
  );

  -- TEST 11:
  SELECT results_eq('
    SELECT - nb_38191 FROM math_func_test_table;',
    ARRAY[
      -654321.1234560000000000000::NUMERIC,
      123456.1234560000000000000,
      -123456.1234560000000000000,
      -65.4321000000000000000,
      654321.1234560000000000000,
      -123456.1234560000000000000,
      123456.1234560000000000000,
      65.4321000000000000000,
      654321.1234560000000000000
    ]
  );

  -- TEST 12:
  SELECT results_eq('
    SELECT nb_38191 * nb_38192 FROM math_func_test_table',
    ARRAY[
      -4281360526286.52062173839360000000000000000000000000::NUMERIC,
      15241414418.78311338393600000000000000000000000000,
      15241414418.78311338393600000000000000000000000000,
      -4281.35971041000000000000000000000000000000,
      4281360526286.52062173839360000000000000000000000000,
      -15241414418.78311338393600000000000000000000000000,
      -15241414418.78311338393600000000000000000000000000,
      4281.35971041000000000000000000000000000000,
      50083839168206.52062173839360000000000000000000000000
    ]
  );

  -- TEST 13:
  SELECT results_eq('
    SELECT nb_38191 / nb_38192 FROM math_func_test_table;',
    ARRAY[
      -0.10000001867912534817::NUMERIC,
      1.00000000000000000000,
      1.00000000000000000000,
      -1.00000000000000000000,
      0.10000001867912534817,
      -1.00000000000000000000,
      -1.00000000000000000000,
      1.00000000000000000000,
      0.00854838885579093182
    ]
  );

  -- TEST 14:
  SELECT results_eq('
    SELECT nb_38191 % nb_38192 FROM math_func_test_table;',
    ARRAY[
      654321.1234560000000000000::NUMERIC,
      0.0000000000000000000,
      0.0000000000000000000,
      0.0000000000000000000,
      -654321.1234560000000000000,
      0.0000000000000000000,
      0.0000000000000000000,
      0.0000000000000000000,
      -654321.1234560000000000000
    ]
  );

  -- TEST 15:
  SELECT results_eq('
    SELECT nb_38191 ^2 FROM math_func_test_table;',
    ARRAY[
      428136132600.7219933839360000000::NUMERIC,
      15241414418.7831133839360000000,
      15241414418.7831133839360000000,
      4281.3597104100000000000,
      428136132600.7219933839360000000,
      15241414418.7831133839360000000,
      15241414418.7831133839360000000,
      4281.3597104100000000000,
      428136132600.7219933839360000000
    ]
  );

  -- TEST 16:
  SELECT results_eq('
    SELECT nb_38191 ^3 FROM math_func_test_table;',
    ARRAY[
      280138515275411401.7877048026632028160::NUMERIC,
      -1881645940129346.5312152487432028160,
      1881645940129346.5312152487432028160,
      280138.3567075181610000000,
      -280138515275411401.7877048026632028160,
      1881645940129346.5312152487432028160,
      -1881645940129346.5312152487432028160,
      -280138.3567075181610000000,
      -280138515275411401.7877048026632028160
    ]
  );

  -- TEST 17:
  SELECT results_eq('
    SELECT |/ nb_38191 FROM math_func_test_table WHERE nb_38191 > 0;',
    ARRAY[
      808.9011827510205::DOUBLE PRECISION,
      351.36323577745014,
      8.08901106440089,
      351.36323577745014
    ]
  );

  -- TEST 18:
  SELECT results_eq('
    SELECT ||/ nb_38191 FROM math_func_test_table WHERE nb_38191 > 0;',
    ARRAY[
      86.81544191845924::DOUBLE PRECISION,
      49.793296444494906,
      4.02961560450549,
      49.793296444494906
    ]
  );

  -- TEST 19:
  SELECT results_eq('
    SELECT @ nb_38191 FROM math_func_test_table;',
    ARRAY[
      654321.1234560000000000000::NUMERIC,
      123456.1234560000000000000,
      123456.1234560000000000000,
      65.4321000000000000000,
      654321.1234560000000000000,
      123456.1234560000000000000,
      123456.1234560000000000000,
      65.4321000000000000000,
      654321.1234560000000000000
    ]
  );

  -- TEST 20: abs() 함수 
  SELECT results_eq('
    SELECT abs(nb_default) FROM math_func_test_table',
    ARRAY[
      0.654321::DECIMAL,
      6543212,
      123456.123456,
      65.4321,
      654321,
      6543212,
      6543210.0123456,
      65,
      1.654321
    ]
  );

  -- TEST 21: cbrt() 함수
  SELECT results_eq('
    SELECT cbrt(nb_default) FROM math_func_test_table WHERE nb_default >0',
    ARRAY[
      0.8681543645840428::DOUBLE PRECISION,
      187.03820699296367,
      187.03820699296367,
      1.1826963619874207
    ]
  );

  -- TEST 22: ceil() 함수
  SELECT results_eq('
    SELECT ceil(nb_default) FROM math_func_test_table',
    ARRAY[
      1::DECIMAL,
      6543212,
      -123456,
      -65,
      -654321,
      6543212,
      -6543210,
      -65,
      2
    ]
  );

  -- TEST 23: ceiling() 함수
  SELECT results_eq('
    SELECT ceiling(nb_default) FROM math_func_test_table',
    ARRAY[
      1::DECIMAL,
      6543212,
      -123456,
      -65,
      -654321,
      6543212,
      -6543210,
      -65,
      2
    ]
  );

  -- TEST 24: degrees() 함수
  SELECT results_eq('
    SELECT degrees(nb_default) FROM math_func_test_table',
    ARRAY[
      37.489831746779544::DOUBLE PRECISION,
      374898432.0593544,
      -7073514.829074847,
      -3748.983174677954,
      -37489831.74677954,
      374898432.0593544,
      -374898318.17514616,
      -3724.225668350351,
      94.78561125986185
    ]
  );

  -- TEST 25: div() 함수
  SELECT results_eq('
    SELECT div(nb_default, nb_default) FROM math_func_test_table',
    ARRAY[
      1::DECIMAL,
      1,
      1,
      1,
      1,
      1,
      1,
      1,
      1
    ]
  );

  -- TEST 26: exp() 함수
  SELECT results_eq('
    SELECT exp(nb_default) FROM math_func_test_table WHERE nb_default < 1000',
    ARRAY[
      1.9238357892914535::DECIMAL,
      0E-1000,
      3.830011082637204E-29,
      0E-1000,
      0E-1000,
      5.900090541597061E-29,
      5.2295278669701227
    ]
  );

  -- TEST 27: floor() 함수
  SELECT results_eq('
    SELECT floor(nb_default) FROM math_func_test_table',
    ARRAY[
      0::NUMERIC,
      6543212,
      -123457,
      -66,
      -654321,
      6543212,
      -6543211,
      -65,
      1
    ]
  );

  -- TEST 28: gcd() 함수
  SELECT results_eq('
    SELECT gcd(nb_default,nb_default) FROM math_func_test_table',
    ARRAY[
      0.654321::DECIMAL,
      6543212,
      123456.123456,
      65.4321,
      654321,
      6543212,
      6543210.0123456,
      65,
      1.654321
    ]
  );

  -- TEST 29: lcm() 함수
  SELECT results_eq('
    SELECT lcm(nb_default,nb_default)FROM math_func_test_table',
    ARRAY[
      0.654321::DECIMAL,
      6543212,
      123456.123456,
      65.4321,
      654321,
      6543212,
      6543210.0123456,
      65,
      1.654321
    ]
  );

  -- TEST 30: ln() 함수
  SELECT results_eq('
    SELECT ln(nb_default) FROM math_func_test_table WHERE nb_default > 0',
    ARRAY[
      -0.4241572222523926::DECIMAL,
      15.693938734366252,
      15.693938734366252,
      0.5033906527411591
    ]
  );

  -- TEST 31: log() 함수
  SELECT results_eq('
    SELECT log(nb_default) FROM math_func_test_table WHERE nb_default > 0',
    ARRAY[
      -0.1842091410836253::DECIMAL,
      6.815790991662967,
      6.815790991662967,
      0.2186197827271615
    ]
  );

  -- TEST 32: log10() 함수
  SELECT results_eq('
    SELECT log10(nb_default) FROM math_func_test_table WHERE nb_default > 0',
    ARRAY[
      -0.1842091410836253::DECIMAL,
      6.815790991662967,
      6.815790991662967,
      0.2186197827271615
    ]
  );

  -- TEST 33: min_scale() 함수
  SELECT results_eq('
    SELECT min_scale(nb_default) FROM math_func_test_table',
    ARRAY[
      6::INTEGER,
      0,
      6,
      4,
      0,
      0,
      7,
      0,
      6
    ]
  );

  -- TEST 34: mod() 함수
  SELECT results_eq('
    SELECT mod(nb_default,nb_default) FROM math_func_test_table',
    ARRAY[
      0.000000::DECIMAL,
      0,
      0.000000,
      0.0000,
      0,
      0,
      0E-7,
      0,
      0.000000
    ]
  );

  -- TEST 35: radians() 함수
  SELECT results_eq('
    SELECT radians(nb_default) FROM math_func_test_table',
    ARRAY[
      0.011420055814941817::DOUBLE PRECISION,
      114200.59305600321,
      -2154.715836055801,
      -1.1420055814941819,
      -11420.055814941818,
      114200.59305600321,
      -114200.55836488954,
      -1.1344640137963142,
      0.028873348334885113
    ]
  );

  -- TEST 36: round() 함수
  SELECT results_eq('
    SELECT round(nb_default) FROM math_func_test_table',
    ARRAY[
      1::DECIMAL,
      6543212,
      -123456,
      -65,
      -654321,
      6543212,
      -6543210,
      -65,
      2
    ]
  );

  -- TEST 37: scale() 함수
  SELECT results_eq('
    SELECT scale(nb_default) FROM math_func_test_table',
    ARRAY[
      6::INTEGER,
      0,
      6,
      4,
      0,
      0,
      7,
      0,
      6
    ]
  );

  -- TEST 38: sign() 함수
  SELECT results_eq('
    SELECT sign(nb_default) FROM math_func_test_table',
    ARRAY[
      1::NUMERIC,
      1,
      -1,
      -1,
      -1,
      1,
      -1,
      -1,
      1
    ]
  );

  -- TEST 39: sqrt() 함수
  SELECT results_eq('
    SELECT sqrt(nb_default) FROM math_func_test_table WHERE nb_default > 0',
    ARRAY[
      0.80890110644008888::DECIMAL,
      2557.9702891159623,
      2557.9702891159623,
      1.286204105109294
    ]
  );

  -- TEST 40: trim_scale() 함수
  SELECT results_eq('
    SELECT trim_scale(nb_default) FROM math_func_test_table',
    ARRAY[
      0.654321::DECIMAL,
      6543212,
      -123456.123456,
      -65.4321,
      -654321,
      6543212,
      -6543210.0123456,
      -65,
      1.654321
    ]
  );

  -- TEST 41: trunc() 함수
  SELECT results_eq('
    SELECT trunc(nb_default) FROM math_func_test_table',
    ARRAY[
      0::DECIMAL,
      6543212,
      -123456,
      -65,
      -654321,
      6543212,
      -6543210,
      -65,
      1
    ]
  );

  -- Finish the tests and clean up.
  SELECT * FROM finish();

ROLLBACK;
