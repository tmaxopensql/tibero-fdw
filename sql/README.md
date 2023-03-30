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

Open `sql/test.conf` file with any text editor and modify parameters according to your environment and settings.

  - `TIBERO_HOST` : Host address of remote Tibero server
  - `TIBERO_PORT` : Port of remote Tibero server
  - `TIBERO_USER` : Username for remote Tibero server
  - `TIBERO_PASS` : Password for the user
  - `TIBERO_DB` : Name of database to use for testing

The above parameters are mandatory as they are essential to create connection to remote Tibero database.

  - `POSTGRES_USER` : Username for local Postgres server
  - `POSTGRES_DB` : Name of database on local Postgres to use for testing
  - `PGPASSWORD` : Password for the local Postgres user

If the above parameters are not given, the tests will be run under same user as session.

3. Setup Tibero Database

On the Tibero server, execute the below SQL scripts to create test tables and required dataset.
Username, Password and Database on which the below scripts run, must match the parameters given in `test.conf` file.

```shell
$ tbsql ${TIBERO_USER}/${TIBERO_PASS}@${TIBERO_DB} @tibero_init.sql
```

4. Run Tests

On a project root directory, run below command to execute all test cases. 

```shell
$ make test
```

To run specific test script file only, run below script directly under `sql/` directory.

```shell
$ ./run_test.sh 01-extension.sql
```

 
