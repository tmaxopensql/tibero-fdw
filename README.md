# tibero-fdw

Foregin Data Wrapper for Tibero
=============================== 

This PostgreSQL extension implements a Foreign Data Wrapper(FDW) for Tibero.

Installation 
------------
To compile the Tibero foreign data wrapper, You need to ensure the `pg_config` executable is in your path when you run `make`. This executable is typically in your PostgreSQL installation's bin directory. 

Also you need PostgreSQL headers (if your PostgreSQL was installed with packages, install the development package).

1. Set LD_LIBRARY_PATH for Tibero client libraries.  
```
. ./setenv
```

2. Compile the code using make
```
make
```

3. Install Tibero foreign data wrapper.
```
make install 
```
