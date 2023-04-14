Foregin Data Wrapper for Tibero
===============================

This PostgreSQL extension implements a Foreign Data Wrapper (FDW) for Tibero.

Installation
------------
To compile the Tibero FDW, You need to ensure the `pg_config` executable is in your path when you run `make`. This executable is typically in your PostgreSQL installation's bin directory.

Also you need PostgreSQL header files (if your PostgreSQL was installed with packages, install the development package).

```shell
## Go to the project home and set environment varibles for Tibero FDW
$ . ./setenv.sh `pwd`

## Compile the code using make
$ make

## Install Tibero foreign data wrapper
$ make install
```

Testing
---------
Tibero Foreign Data Wrapper depends on [pgTAP](https://pgtap.org/) as a testing framework. To test Tibero Foreign Data Wrapper, you need pgTAP installed on local PostgreSQL server, and a running remote Tibero Database server and access privilege to its system catalog.

1. Install pgTAP and Its Dependencies
```shell
## Install Perl parser library for pgTAP
$ cpan TAP::Parser::SourceHandler::pgTAP

## Clone source repository
$ git clone https://github.com/theory/pgtap.git

$ make
$ make install
$ make installcheck
```

2. Install pyodbc dependencies

```shell
$ sudo apt-get install unixodbc
$ python -m pip install -r $TBFDW_HOME/tests/requirements.txt
```

3. Run Tests

In order to run tests, you need to set the connection strings for remote Tibero and local PostgreSQL as environment variables.
```shell
export TBFDW_TEST_TBCONN=\[connection string\]
export TBFDW_TEST_PGCONN=\[connection string\]
```
Here connection string is of the format [host:port:dbname:user:password].
Now, On a project root directory, run below command to execute all test cases.

```shell
$ make test
```

To run specific test cases only, run below command to see the options
```shell
$ tbfdw_test --help
```
