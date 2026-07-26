/************************************************
 *
 *
 *
 *	PostreSQL import module
 *
 *
 * 	(C) 2005. Alvaro Cortes. accsc@arbornet.org
 *
 *	This is a part of LibdollyBASE. it's under de same license
 *
 * 
 *
 ***********************************************/
#include "libpq-fe.h"

#define PG_IMPORT_VER 0
#define PG_IMPORT_SUB 1
#define PG_IMPORT_DATE 200502

#warning "PostgreSQL Import activated"

void init_postgres()
{
	fprintf(stderr,"PostgreSQL import init. Version %i.%i/%i",PG_IMPORT_VER,PG_IMPORT_SUB,PG_IMPORT_DATE);
	fflush(stderr);
	
}

int create_pg_table(char *use, char *password, char *host, int port, char *db, char *table, char *dbf_name)
{
	PGconn *conn;
	PGresult *results;
	ExecStatusType status;

	char connect_string[1024], query[1024];
	int len, connect_status,n,i,o,o2;
	int *tipos;
	char *fname = (char *) malloc(12);
	char *type; 
	DATABASEDBF *pg_dbf;
	if( fname == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Not enought memory\n");
		fflush(stderr);
#endif
		return -6;
	}
	if( (pg_dbf = (DATABASEDBF *) malloc ( sizeof(DATABASEDBF))) == NULL)
	{
		free(fname);
		return -10;
	}
	if( port >= 65555 || port <= 0)
	{
#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Port out of range\n");
		fflush(stderr); 
#endif
		return -1;  /* Port out of range */
	}
	len = strlen(host)+strlen(use)+strlen(password)+strlen(db)+5;

	if( len+36 >= 1022)
	{
#ifdef DEBUG
		fprintf(stderr,"Import.PG moudle. Connection string too big\n");
		fflush(stderr);
#endif
		return -2;  /* Connection string out of range */
	}
	if( strlen(table) +10 > 1022)
	{
#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Query too big\n");
		fflush(stderr);
#endif
		return -2;
	}

	sprintf(connect_string,"host=%s port=%i dbname=%s user=%s password=%s",
			host,port,db,use,password);
	sprintf(query,"SELECT * FROM %s",table);

	conn = PQconnectdb(connect_string);
	connect_status = PQstatus(conn);

	if( connect_status == CONNECTION_BAD)
	{
#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Can connect to database\n");
		fflush(stderr);
#endif
		return -3;
	}
	
	/* Now Extract structure from database table */
	results = PQexec(conn,query);
	status = PQresultStatus(results);
	switch(status)
	{
		case PGRES_TUPLES_OK:
			n = PQnfields(results);
			pg_dbf->camposn = n;
			tipos = (int *) malloc( sizeof( int *) *n);
		for( i = 0; i<n; ++i)	
		{
			fname = PQfname(results,i);
			for( o = 0; o < strlen(fname); ++o)
			{
				if( o == 12)
					break;
	 			pg_dbf->fields.names[o][i+1] = fname[o]; 
			}
			if( o < 12)
			{
				for( o; o<12; ++o)
				pg_dbf->fields.names[o][i+1] = '\0';
			}
			
			pg_dbf->fields.names[12][i+1] = '\0';
			pg_dbf->fields.longitudes[i+1] = PQfsize(results,i);
			tipos[i] = PQftype(results,i);
		}
		break;
		default:
#ifdef DEBUG
			fprintf(stderr,"Import.PG module.Fail when getting table structure\n");
			fflush(stderr);
#endif
			PQfinish(conn); 
			free(fname); 
			free(pg_dbf);
			return -5; 
			break;
		
	}
	fflush(stdout);
	type = (char *) malloc(256);
	if( type == NULL)
	{	
#ifdef DEBUG
	fprintf(stderr,"Import.PG module. Not enought memory\n");
	fflush(stderr);
#endif
		free(tipos);
		free(fname);
		free(pg_dbf);
		return -6;
	}
	for( i = 0; i <n; ++i)
	{
	PQclear(results);
	sprintf(query,"select typname from pg_type where oid =%lu",tipos[i]);
	results = PQexec(conn,query);
 	type = PQgetvalue(results,0,0);	
	if( strstr(type,"int") > 0)
		pg_dbf->fields.tipos[i+1] = 'N';
	else if( strstr(type,"varchar") > 0)
		pg_dbf->fields.tipos[i+1] = 'C';
	else 
		pg_dbf->fields.tipos[i+1] = 'U';
	}
	PQfinish(conn);
	free(tipos);
	return create_database("h1.dbf",12,12,4,pg_dbf,1);
}

int fill_pg_data(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name)
{
	char connect_string[1024], query[1024];
	int len, connect_status,n,i,o;
	char *fname = (char *) malloc(1024);
	DATABASEDBF *pg_dbf;
	PGconn *conn;
	PGresult *results;
	ExecStatusType status;


	if( fname == NULL)
	{
	#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Not enought memory\n");
		fflush(stderr);
	#endif
		return -6;
	}
	if( (pg_dbf = (DATABASEDBF *) malloc( sizeof(DATABASEDBF))) == NULL)
	{
		free(fname);
		return -10;
	}
	use(dbf_name, &pg_dbf);
	if( pg_dbf->tipo <= 0)
	{
	#ifdef DEBUG
	fprintf(stderr,"Import. PG module. Can not read database file\n");
	fflush(stderr);
	#endif
	return -1;
	}
	
	
	if( port >= 65555 || port <= 0)
	{
#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Port out of range\n");
		fflush(stderr); 
#endif
		return -1;  /* Port out of range */
	}
	len = strlen(host)+strlen(use)+strlen(password)+strlen(db)+5;
	if( len > 1020)
	{
	#ifdef DEBUG
		fprintf(stderr,"Import.PG moudle. Connection string too big\n");
		fflush(stderr);
	#endif
		return -2;  /* Connection string out of range */
	}
	if( strlen(table) +10 > 1022)
	{
	#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Query too big\n");
		fflush(stderr);
	#endif
		return -2;
	}

	sprintf(connect_string,"host=%s port=%i dbname=%s user=%s password=%s",
			host,port,db,use,password);
	sprintf(query,"SELECT * FROM %s",table);

	conn = PQconnectdb(connect_string);
	connect_status = PQstatus(conn);

	if( connect_status == CONNECTION_BAD)
	{
	#ifdef DEBUG
		fprintf(stderr,"Import.PG module. Can connect to database\n");
		fflush(stderr);
	#endif
		return -3;
	}
	
	/* Now Extract structure from database table */
	results = PQexec(conn,query);
	status = PQresultStatus(results);
	switch(status)
	{
		case PGRES_TUPLES_OK:
			n = PQnfields(results);
			pg_dbf->camposn = n;
	for( o = 0; o< PQntuples(results); ++o)
	{
		for( i = 0; i<n; ++i)	
		{
			if( PQfsize(results,i) > 256)
			{
	#ifdef DEBUG
	fprintf(stderr,"Import.PG module. Error, field oversized! (>256)\n");
	fflush(stderr);
	#endif
				break;
			}
			append_blank(&pg_dbf);
			fname = PQgetvalue(results,o,i);
			replace(pg_dbf,PQfname(results,i),fname);
		}
	}
	}
		

	PQfinish(conn);
	free(fname);
	free(pg_dbf);
	return 0;
}
