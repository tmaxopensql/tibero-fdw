/* contrib/tibero_fdw/tibero_fdw--1.0.sql */

\echo Use "CREATE EXTENSION tibero_fdw" to load this file. \quit

CREATE FUNCTION tibero_fdw_handler()
RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FOREIGN DATA WRAPPER tibero_fdw
  HANDLER tibero_fdw_handler;
