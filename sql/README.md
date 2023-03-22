Testing
------------
Tibero Foreign Data Wrapper depends on [pgTAP](https://pgtap.org/) as a testing framework. To test Tibero Foreign Data Wrapper, you need pgTAP installed on local PostgreSQL server, and a running remote Tibero Database server and access privilege to its system catalog.

1. Install pgTAP and its dependencies

```shell
## Install Perl parser library for pgTAP
$ cpan TAP::Parser::SourceHandler::pgTAP

## Clone source repository
$ git clone https://github.com/theory/pgtap.git

$ make
$ make install
$ make installcheck
```

2. Configure environment

Open `sql/run_test.sh` file with any text editor and modify parameters according to your environment and settings.
  - `TIBERO_HOST` : Host address of remote Tibero server
  - `TIBERO_PORT` : Port of remote Tibero server
  - `TIBERO_USER` : Username for remote Tibero server
  - `TIBERO_PASS` : Password for the user
  - `TIBERO_DB` : Name of database to use for testing

  - `POSTGRES_USER` : Username for local Postgres server
  - `POSTGRES_DB` : Name of database on local Postgres to use for testing
  - `PGPASSWORD` : Password for the local Postgres user

3. Setup Tibero Database 

  - 작성중

4. Run Tests

```shell
$ make test
```

