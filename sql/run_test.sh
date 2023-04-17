#!/bin/bash

if [ -e $(dirname "$0")/test.conf ]; then
  . $(dirname "$0")/test.conf
else
	echo "[ERROR] Follow the steps below"
	echo "1. Copy $(dirname "$0")/test.conf.eg to $(dirname "$0")/test.conf"
	echo "2. Configure $(dirname "$0")/test.conf"
	exit 0;
fi

if [[ -z ${TIBERO_HOST} ]] || [[ -z ${TIBERO_PORT} ]] || [[ -z ${TIBERO_USER} ]] || [[ -z ${TIBERO_PASS} ]] || [[ -z ${TIBERO_DB} ]];
then
  echo "Tibero7 connection parameters are incomplete. Exiting..."
  exit 1;
fi

if [ -z ${POSTGRES_USER} ];
then
  PG_USROPT="-U ${PGUSER}"
else
  PG_USROPT="-U ${POSTGRES_USER}"
fi

if [ -z ${POSTGRES_DB} ];
then
  PG_DBOPT=""
else
  PG_DBOPT="-d ${POSTGRES_DB}"
fi

if [ $# -eq 0 ]
then
  TARGET="*.sql"
else
  TARGET="$@"
fi

pg_prove --verbose --norc --set TIBERO_HOST=$TIBERO_HOST --set TIBERO_PORT=$TIBERO_PORT \
  --set TIBERO_USER=$TIBERO_USER --set TIBERO_PASS=$TIBERO_PASS --set TIBERO_DB=$TIBERO_DB \
  --set PSQLRC=default $PG_USROPT $PG_DBOPT --pset tuples_only=1 $TARGET

