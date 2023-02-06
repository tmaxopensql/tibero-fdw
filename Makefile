MODULE_big = tibero_fdw
EXTENSION = tibero_fdw

DATA = tibero_fdw--1.0.sql
OBJS = tibero_fdw.o 


PG_CPPFLAGS = -I"./include"
LDFLAGS = -ltbcli

ifdef USE_PGXS
PG_CONFIG = pg_config
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
else
subdir = contrib/tibero_fdw
top_builddir= ../..
include $(top_builddir)/src/Makefile.global
include $(top_srcdir)/contrib/contrib-global.mk
endif 
