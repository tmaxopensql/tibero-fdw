CREATE FUNCTION tibero_fdw_handler() RETURNS fdw_handler
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FUNCTION tibero_fdw_validator(text[], oid) RETURNS void
AS 'MODULE_PATHNAME'
LANGUAGE C STRICT;

CREATE FOREIGN DATA WRAPPER tibero_fdw
  HANDLER tibero_fdw_handler
  VALIDATOR tibero_fdw_validator;

COMMENT ON FOREIGN DATA WRAPPER tibero_fdw
IS 'tibero foreign data wrapper';
