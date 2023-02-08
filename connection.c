#include "postgres.h"

#include "utils/hsearch.h"
#include "utils/inval.h"
#include "utils/memutils.h"
#include "utils/syscache.h"
#include "foreign/foreign.h"
#include "fmgr.h"
#include "stdio.h"
#include "stdlib.h"
#include "sqlcli.h"

/*
 * Connection cache hash table entry
 *
 * The lookup key in this hash table is the foreign server OID plus the user
 * mapping OID.  (We use just one connection per user per foreign server,
 * so that we can ensure all scans use the same snapshot during a query.)
 */
typedef Oid ConnCacheKey;

typedef struct ConnCacheEntry
{
	ConnCacheKey key;			/* hash key (must be first) */
	SQLHANDLE	   *conn;			/* connection to foreign server, or NULL */
	bool		invalidated;	/* true if reconnect is pending */
	uint32		server_hashvalue;	/* hash value of foreign server OID */
	uint32		mapping_hashvalue;	/* hash value of user mapping OID */
} ConnCacheEntry;

/*
 * Connection cache (initialized on first use)
 */
static HTAB *ConnectionHash = NULL;

static void make_new_tibero_connection(ConnCacheEntry *entry, UserMapping *user);
static SQLHDBC connect_tibero(ForeignServer *server, UserMapping *user);
static void tibero_inval_callback(Datum arg, int cacheid, uint32 hashvalue);

void
GetConnection(UserMapping *user, bool will_prep_stmt)
{
	bool		found;
	ConnCacheEntry *entry;
	ConnCacheKey key;
	MemoryContext ccxt = CurrentMemoryContext;

	/* First time through, initialize connection cache hashtable */
	if (ConnectionHash == NULL)
	{
		HASHCTL		ctl;

		ctl.keysize = sizeof(ConnCacheKey);
		ctl.entrysize = sizeof(ConnCacheEntry);
		ConnectionHash = hash_create("tibero_fdw connections", 8,
									 &ctl,
									 HASH_ELEM | HASH_BLOBS);

		/*
		 * Register some callback functions that manage connection cleanup.
		 * This should be done just once in each backend.
		 */

		CacheRegisterSyscacheCallback(FOREIGNSERVEROID,
									  tibero_inval_callback, (Datum) 0);
		CacheRegisterSyscacheCallback(USERMAPPINGOID,
									  tibero_inval_callback, (Datum) 0);
	}

	// key = user->userid;
	
	/* For test */
	key = 15;

	/*
	 * Find or create cached entry for requested connection.
	 */
	entry = hash_search(ConnectionHash, &key, HASH_ENTER, &found);
	
	if (!found)
	{
		/* Initialize new hashtable entry (key is already filled in) */
		entry->conn = NULL;
	}

	/* If an existing entry has invalid connection then release it */
	if (entry->conn != NULL && entry->invalidated)
	{
		elog(DEBUG3, "disconnecting tibero_fdw connection %p for option changes to take effect",
			 entry->conn);
		
		/*TODO */
		//disconnect_tibero(entry);
		
		entry->conn = NULL;
	}

			
		/*For test*/
		entry->conn = connect_tibero(NULL,user);

	if (entry->conn == NULL)
	{
		// make_new_tibero_connection(entry,user);

	}
	
	//return entry->conn;	
}

static void
make_new_tibero_connection(ConnCacheEntry *entry, UserMapping *user)
{
	ForeignServer *server = GetForeignServer(user->serverid);

	Assert(entry->conn == NULL);

	entry->invalidated = false;
	entry->server_hashvalue =
		GetSysCacheHashValue1(FOREIGNSERVEROID,
							  ObjectIdGetDatum(server->serverid));
	entry->mapping_hashvalue =
		GetSysCacheHashValue1(USERMAPPINGOID,
							  ObjectIdGetDatum(user->umid));

	/* Now try to make the connection */
	entry->conn = connect_tibero(server, user);

	elog(DEBUG3, "new tibero fdw connection %p for server \"%s\" (user mapping oid %u, userid %u)",
		 entry->conn, server->servername, user->umid, user->userid);
}

SQLHDBC
connect_tibero(ForeignServer *server, UserMapping *user)
{
	SQLRETURN rc = SQL_ERROR;
	SQLHANDLE henv, hdbc, hstmt;
	SQLCHAR *sql = (SQLCHAR *)"SELECT * FROM dual";

	/* Env Handle*/                                                                
    SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv);                        
    SQLSetEnvAttr(henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0);       
    SQLAllocHandle(SQL_HANDLE_DBC, henv, &hdbc); 

	/* Tibero Connect */                                                                                                                                          
    rc = SQLDriverConnect(hdbc,            /*connection hadle*/                    
        (SQLHWND)NULL,                     /*window handle */                      
        (SQLCHAR *) "DRIVER={Tibero 7 ODBC Driver};\                               
        SERVER=192.1.2.54;PORT=8629;DB=tibero;UID=tibero;PWD=tmax;",/*connectionstring*/
        SQL_NTS,   /*length of the connection character string*/                   
        NULL,      /*outconnectionstring */                                        
        0,         /*buffer length*/                                               
        NULL,      /*length of the data that is saved in the buffer */             
        SQL_DRIVER_NOPROMPT);                                                      
                                                                                   
    if (rc != SQL_SUCCESS) {                                                       
      fprintf(stderr, "Connection failed!!!");                                                                                                          
    } else{fprintf(stderr, "Connection Success7"); }
	    
   /* Memory allocation for Statements */                                         
    // SQLAllocHandle(SQL_HANDLE_STMT, hdbc, &hstmt);                                 
    // printf("Query: %s\n", sql);                                                    
                                                                                   
    /* Execute query */                                                            
    //rc = SQLExecDirect(hstmt, sql, SQL_NTS);                                       
                                                                                   
    // if (rc != SQL_SUCCESS) {                                                       
    // fprintf(stderr, "SQLExecDirect failed!!!\n");                                                                                                         
    // }

	return hdbc;
}





/*
 * Connection invalidation callback function for mysql.
 *
 * After a change to a pg_foreign_server or pg_user_mapping catalog entry,
 * mark connections depending on that entry as needing to be remade. 
 */

static void
tibero_inval_callback(Datum arg, int cacheid, uint32 hashvalue)
{
	HASH_SEQ_STATUS scan;
	ConnCacheEntry *entry;

	Assert(cacheid == FOREIGNSERVEROID || cacheid == USERMAPPINGOID);

	/* ConnectionHash must exist already, if we're registered */
	hash_seq_init(&scan, ConnectionHash);
	while ((entry = (ConnCacheEntry *) hash_seq_search(&scan)))
	{
		/* Ignore invalid entries */
		if (entry->conn == NULL)
			continue;

		/* hashvalue == 0 means a cache reset, must clear all state */
		if (hashvalue == 0 ||
			(cacheid == FOREIGNSERVEROID &&
			 entry->server_hashvalue == hashvalue) ||
			(cacheid == USERMAPPINGOID &&
			 entry->mapping_hashvalue == hashvalue))
			entry->invalidated = true;
	}

}
