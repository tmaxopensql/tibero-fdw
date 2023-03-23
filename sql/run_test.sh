#!/bin/bash
TIBERO_HOST="'127.0.0.1'"
TIBERO_PORT="'8629'"
TIBERO_USER="'tibero'"
TIBERO_PASS="'tmax'"
TIBERO_DB="'tibero'"

POSTGRES_USER="yhjeon"
POSTGRES_DB="dvdrental"

if [ $# -eq 0 ]
then
  TARGET="*.sql"
else
  TARGET="$@"
fi

export PGPASSWORD="postgres"

pg_prove --set TIBERO_HOST=$TIBERO_HOST --set TIBERO_PORT=$TIBERO_PORT \
  --set TIBERO_USER=$TIBERO_USER --set TIBERO_PASS=$TIBERO_PASS --set TIBERO_DB=$TIBERO_DB \
  -U $POSTGRES_USER -d $POSTGRES_DB --pset tuples_only=1 $TARGET
