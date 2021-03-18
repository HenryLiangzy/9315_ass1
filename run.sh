source /srvr/z5240778/env
cd /srvr/z5240778/postgresql-12.5/src/tutorial/

make

pgs start
dropdb test
createdb test

psql test -f intset.sql
psql test -f test.sql

pgs stop