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

## Install Tibero FDW
$ make install
```

Testing
---------
Tibero FDW depends on [pgTAP](https://pgtap.org/) as a testing framework. To test Tibero FDW, you need pgTAP installed on local PostgreSQL server, and a running remote Tibero Database server with access privilege to its system catalog.

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

In order to run tests, copy tests/tbfdw_test.conf.eg to tests/tbfdw_test.conf and fill in the parameter values of tests/tbfdw_test.conf.
Now, On a project root directory, run below command to execute all test cases.

```shell
$ make test
```

To run specific test cases only, run below command to see the options
```shell
$ tbfdw_test --help
```
