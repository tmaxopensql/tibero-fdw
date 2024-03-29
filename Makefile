# contrib/tibero_fdw/Makefile
MODULE_big = tibero_fdw
OBJS = utils.o deparse.o connection.o option.o conditions.o tibero_fdw.o
PGFILEDESC = "tibero_fdw - foreign data wrapper for Tibero"

PG_CPPFLAGS = -I./include
PG_LDFLAGS = -L./lib
SHLIB_LINK = -ltbcli

EXTENSION = tibero_fdw
DATA = tibero_fdw--1.0.sql

PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)

test:
	python ${TBFDW_HOME}/tests/tbfdw_test.py --quiet \