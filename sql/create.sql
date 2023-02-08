CREATE EXTENSION IF NOT EXISTS tibero_fdw;

CREATE SERVER tibero_server FOREIGN DATA WRAPPER tibero_fdw 
OPTIONS (host '127.0.0.1', port '8629', dbname 'tb7');

CREATE USER MAPPING FOR postgres SERVER tibero_server 
OPTIONS (username 'tibero', password 'tmax');

CREATE FOREIGN TABLE rt1 (c1 numeric, c2 real, c3 numeric, c4 varchar(100)) 
SERVER tibero_server 
OPTIONS (owner_name 'tibero', table_name 't1', fetch_size '1000');

