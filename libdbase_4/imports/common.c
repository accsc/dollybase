/************************************************
 *
 *
 *	Common import module
 *
 *	(C) 2005 Alvaro Corés. accsc@arbornet.org
 *
 *
 *
 *
 *************************************************/

int d_modules[3];	    /* Import modules actives */
#define IMPORT_VER 0
#define IMPORT_SUBVER 2	    /* Version 0.1 */
#define IMPORT_DATE 200505  /* Febrary 2005*/


/* Definition of import specific functions */

#ifdef POSTGRES 
void init_postgres();
int create_pg_table(char *user, char *password, char *host, int port, char *db, char *table,char *dbf_name);
int fill_pg_data(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name);
#include "pg.c"
#endif

#ifdef MY
void init_mysql();
int create_my_table(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name);
int fill_my_table(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name);
#include "mysql.c"
#endif

#ifdef ORACLE
void init_oracle();
int create_ora_table(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name);
int fill_ora_table(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name);
#include "oracle.c"
#endif


/* Initialize import modules */
void init_import_modules()
{
	
#ifdef POSTGRES
	d_modules[0] = 1;
#else
	d_modules[0] = 0;
#endif
	
#ifdef MYSQL
	d_modules[1] = 1;
#else
	d_modules[1] = 0;
#endif
	
#ifdef ORACLE
	d_modules[2] = 1;
#else
	d_modules[2] = 0;
#endif

}

