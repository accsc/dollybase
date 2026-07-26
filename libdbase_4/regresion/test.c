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

void test_data();
void test_packs();
void test_memo();
void test_locks();
void test_indexing();
void test_label();
void test_db2();
void test_export();
void test_creates();
void pauset();

int  main()
{
	printf("--------------------------------------------------------------------\n");
	printf("Regresion test v0.0.1 - (C) Alvaro Cortés. 2004 (accsc@arbornet.org)\n");
	printf("--------------------------------------------------------------------\n\n");
	printf("Test properly working of libdollybase\n\n");
	fflush(stdout);
	printf("This program will execute some test to test if dollybase is ok.\n");
	printf("Press Ctrl-C to exit or other key to start...\n");
	getchar();
	test_creates();
	pauset();
	test_data();
	pauset(); 
/*	test_locks();
	pauset();*/
	test_label();
	pauset();
	test_export();
	pauset();
	return 0;
}


/***************** Test CREATE DBF & DBT files *******************/

void test_creates()
{
	DATABASEDBF *db_test;

	db_test = (DATABASEDBF *) malloc (sizeof(DATABASEDBF));

	printf("\n===========================================\n");
	printf("======== Starting CREATES.C checks ========\n");
	printf("===========================================\n");
	printf("Adding data to db_test structure about FIELDS...\n");
	db_test->camposn = 4;
	db_test->fields.names[0][1] = 'F';
	db_test->fields.names[1][1] = '1';
	db_test->fields.names[2][1] = '\0';
	db_test->fields.tipos[1] = 'C';
	db_test->fields.longitudes[1] = 50;

	db_test->fields.names[0][2] = 'F';
	db_test->fields.names[1][2] = '2';
	db_test->fields.names[2][2] = '\0';
	db_test->fields.tipos[2] = 'N';
	db_test->fields.decimales[2] = 0;
	db_test->fields.longitudes[2] = 5;
	
	db_test->fields.names[0][3] = 'F';
	db_test->fields.names[1][3] = '3';
	db_test->fields.names[2][3] = '\0';
	db_test->fields.tipos[3] = 'M';
	db_test->fields.longitudes[3] = 10;

	db_test->fields.names[0][4] = 'F';
	db_test->fields.names[1][4] = '\0';
	db_test->fields.tipos[4] = 'L';
	db_test->fields.longitudes[4] = 3; 
	printf("create_database returned: %i & should be 0\n",create_database("test.dbf",14,6,4,db_test,1)); 
	use("test.dbf",(DATABASEDBF *) &db_test);
	if( db_test->tipo > 0)
	{
		display_structure(db_test);
	}
	printf("create_dbt_file returned: %i & should be 0\n",create_dbt_file("test.dbt"));
	if( access("test.dbt", R_OK & F_OK) == 0)
	{
		printf("------- -=CREATE DBT test result OK=-  ----------\n");
		fflush(stdout);
	}else{
		printf("------- -=CREATE DBT test result FAIL=- ------------\n");
		fflush(stdout);
		exit(-1);
	}
	printf("================= CREATES.C END =====================\n");
	free(db_test);
}



void test_data()
{
 	DATABASEDBF *dbf2;
	char *f1;
	char *f2;
	char *f3;
	printf("\n===================================\n");
	printf("====== Staring RECS.C cheks =======\n");
	printf("===================================\n");

	dbf2 = (DATABASEDBF *) malloc( sizeof(DATABASEDBF));
	f1 = (char *) malloc ( 257);
	f2 = (char *) malloc (257);
	f3 = (char *) malloc(257);
	use("test.dbf",(DATABASEDBF *) &dbf2);

	printf("append_blank returned: %i & should be 0\n",append_blank(&dbf2));
	dbf2->current = 1;
	printf("replace for FIELD 1 returned: %i & should be 0\n",replace(dbf2,"F1","TEST 1"));
	printf("replace for FIELD 2 returned: %i & should be 0\n",replace(dbf2,"F2","2399"));
	printf("replace for FIELD 3 returned: %i & should be 0\n",replace(dbf2,"F3","1")); 
	printf("add_to_dbt for DBT Field returned: %i & should be 0\n",add_to_dbt("test.dbt","DBT field test",16));
	printf("----------- -= APPEND DATA TEST END =- ----------\n");
	fflush(stdout);
	

	get_field(dbf2,2,&f1);
	get_field(dbf2,3,&f2);
	get_field(dbf2,4,&f3);
	printf("Content of Database  ---  Test data\n");
	printf("'%s' --- should be ->  'TEST 1'\n'%s' --- should be ->  '2399'\n'%s' --- should be -> 'DBT field test'\n\n",f1,f2,f3);
	printf("----------- -= GET DATA TEST END =- -------------\n");
	fflush(stdout);

	printf("delete returned: %i & should be 0 \n",delete(dbf2));
	printf("is_deleted() returned: %i & should be 0\n",is_deleted(dbf2));
	printf("recall returned: %i & should be 0\n",recall(dbf2));
	printf("is_deleted() returned: %i & should be -1\n",is_deleted(dbf2));
	printf("------------ -= DELETE/RECALL DATA TEST END =- ----------\n");
	free(f1);
	free(f2);
	free(f3);
	free(dbf2);
}
void test_locks()
{
	DATABASEDBF *dbf3;
	dbf3 = (DATABASEDBF *) malloc ( sizeof(DATABASEDBF));
	use("test.dbf",(DATABASEDBF *) &dbf3);
	printf("\n=====================================\n");
	printf("====== Starting LOCKER.c checks =====\n");
	printf("=====================================\n");
	printf("cb_lock returned %i & should be 1\n",cb_lock(dbf3));
	printf("dbf_lock returned %i & shold be 0\n",dbf_lock(dbf3));
	printf("if_dbf_lock returned %i & should be 2\n",if_dbf_lock(dbf3));
	printf("rec_lock returned %i & should be 0\n",rec_lock(dbf3,1));
	printf("if_rec_lock returned %i & should be 1\n",if_rec_lock(dbf3,1));
	printf("rec_unlock returned %i & shoulde be 0\n",rec_unlock(dbf3));
	printf("------- -= LOCK TEST END =- --------\n");
}
void test_export()
{
	DATABASEDBF *dbf4;
	dbf4 = (DATABASEDBF *) malloc (sizeof(DATABASEDBF));
	use("test.dbf",(DATABASEDBF *) &dbf4);
	printf("\n========================================\n");
	printf("======= Starting EXPORT.C checks =======\n");
	printf("========================================\n");
	printf("export_as_sql returned: %i & should be 0\n",export_as_sql(dbf4,"test.sql",1));
	printf("export_as_csv returned: %i & should be 0\n",export_as_csv(dbf4,',',"test.csv"));
	printf("Now check test.sql, test.csv & test.html for all data please!\n");
	printf("------- -= EXPORT TEST END =- --------\n");
	fflush(stdout);
	free(dbf4);
}
void pauset()
{
	printf("Press any key to continue with next test ...\n");
	fflush(stdout);
	getchar();

}
void test_indexing()
{
	DATABASEDBF *dbf5;
	FOUND find;
	dbf5 = (DATABASEDBF *) malloc ( sizeof(DATABASEDBF));
	use("books.dbf",(DATABASEDBF *) &dbf5);
	dbf5->current = 1;
/*	create_index_ndx_fast(dbf5,"TITULO","a.ndx");*/
	pauset();
	dbf5->index_node = use_ndx("a.ndx");
	display_ndx_info(dbf5->index_node);
	find = seek_index(dbf5->index_node,"TEORIA Z",0);
	printf("Pos: %i\n",dbf5->index_node.pos);
}

void test_label()
{
	LBL l1;
	printf("===================================\n");
	printf("====== Starting LABEL test ========\n");
	printf("===================================\n");
	fflush(stdout);
	l1.lMargin = 0;
	l1.nLines = 0;
	l1.nCols = 0;
	l1.spaceh = 35;
	l1.spacev = 5;
	l1.spacea = 1; 
	sprintf(l1.text,"aaaaaaa");

	printf("create_label returned %i and should be 0\n",create_label(l1,"test.lbl"));
	l1 = use_label("test.lbl");
	printf(" ===> Label Information\n");
	print_label_info(l1);
	printf("----------- -= LABEL test end =- --------------\n");
	fflush(stdout);
}
