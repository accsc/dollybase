/************************************************************
 *
 *
 *
 *   Regresion test 0.0.1 for dollyBASE alpha series.
 *
 *   (C) Alvaro Cortés 2004. accsc@arbornet.org
 *
 *
 *   Under GPL license.
 *  
 ***********************************************************/

#include "stdio.h"
#include "unistd.h"
#include "../libdbase.h"


int  main(int argc, char *argv[])
{
	DATABASEDBF *dbf2;

	printf("EXPORTER. Export DBF as a SQL file.\n");
	printf("Copywright (C) 2006 Alvaro Cortes.\n");
	printf("Version 1.0.\n\n");

	if( argc < 2)
	{
		printf("Use: exporter <dbf file> <output.sql>\n");
		exit(0);
	}

	dbf2 = (DATABASEDBF *) malloc( sizeof(DATABASEDBF));
	if( dbf2 == NULL)
	{
		printf("Error. Not enought memory.\n");
		exit(-2);
	}
	use(argv[1],&dbf2);

	if( dbf2->tipo == 0)
	{
		printf("Error. Cannot recognize database file.\n");
		exit(-1);
	}
	printf("Export database to: %s ...\n",argv[2]);
	if( export_as_sql(dbf2,argv[2],1) == 0)
	{
		printf("Export finished correctly.\n");
		exit(0);
	}else{
		printf("Export failed.\n");
	}
}
