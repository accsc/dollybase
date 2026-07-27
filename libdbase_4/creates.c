#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include "libdbase.h"

/**************************************************************
 *
 *
 *  create_database 
 *
 *  name -> name of the new database like "dfdfd.dbf"
 *  day,month,year -> Date
 *  db_struc -> a structure DATABASEDBF with:
 *  multi -> 0 = NO _DBFLOCK field, 1 = _DBFLOCK field will be added.
 *
 *   .camposn   -> Number of fields
 *  ----- .fields must be start at [1] not at [0] ----
 *   .fields.name -> name of field.
 *   .fields.tipos -> tipo of filed 'C' 'N' 'L' .
 *   .fileds.decimales -> number of decimals.
 *   .fields.longitudes -> len of the field.
 *
 *   
 * 
 *************************************************************/

int create_database(char *_name,int day, int month, int year,
		DATABASEDBF *db_struc, int multi)
{
FILE *new_db;
int contador, rec_len, cont, cont2;
char chr;
char *header;
int has_memo = 0;

/* Scan fields for memo type before writing header */
for( contador = 1; contador <= db_struc->camposn; ++contador)
{
	if( db_struc->fields.tipos[contador] == 'M')
	{
		has_memo = 1;
		break;
	}
}

if ( (header = (char *) malloc (34)) == NULL )
{
	fprintf(stderr,"Creates error. Sin memoria.\n");
	fflush(stderr);
	return -1;
}

	if( access(_name,F_OK) == 0)
	{
#ifdef DEBUG
		fprintf(stderr,"File already exist, wont be overwritten\n");
		fflush(stderr);
#endif
		free(header);
		return -3;
	}if( (new_db = fopen(_name,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"fopen: Cant open file for make a new db\n");
		fflush(stderr);
#endif
		free(header);
		return -2;
	}
/***************************** Head stuff *********************************/

	header[0] = has_memo ? 0x83 : 0x03; /* 0x83 = dBase III with DBT memo */
	header[1] = year;  /* Ex: 0 for 2000 or 7 for 2007 */
	header[2] = month; /* 1-12 */
	header[3] = day; /* 1-31 */

	/* Number of records. New DB has 0 recs */
	header[4] = 0x00;
	header[5] = 0x00;
	header[6] = 0x00;
	header[7] = 0x00;

	/* header[30] for codepages, header[16] for encryption(0x01 or 0x00) */
	for( contador = 8; contador <=32; contador++)
		header[contador] = 0x00;
	
	/* Added by dBase III ???? */
	header[8] = 0xE1;
	header[10] = 0x87; 
	
	/* Write header to db */
	fwrite(header,1,31,new_db);
	free(header);
	fclose(new_db);	
/*************************** End of head stuff *****************************/

	if( (new_db = fopen(_name,"a")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Failed to reopen file after header\n");
		fflush(stderr);
#endif
		return -4;
	}
	
/****************************** Fields stuff *******************************/
	fputc(' ',new_db);
	/*********** Add LOCK field - Optional ******************/
	if( multi == 1)
	{
#ifdef DEBUG
		fprintf(stderr,"CREATE. _DBFLOCK field added\n");
		fflush(stderr);
#endif
		fputc('_',new_db);
		fputc('D',new_db);
		fputc('B',new_db);
		fputc('F',new_db);
		fputc('L',new_db);
		fputc('O',new_db);
		fputc('C',new_db);
		fputc('K',new_db);
		fputc(0x00,new_db);
		fputc(0x00,new_db);
		fputc(0x00,new_db);
		fputc('N',new_db);
		fputc(0x00,new_db);
		fputc(0x00,new_db);
		fputc('%',new_db);
		fputc('F',new_db);
		fputc(2,new_db);
		for( cont2 = 1; cont2 <=15; ++cont2)
		fputc(0x00,new_db);
	}

 	/******************* End of LOCK field *****************/
		
		
	for( contador = 1; contador <= db_struc->camposn; ++contador)
	{
		for( cont = 1; cont <= 11; cont++)
		{
			chr = db_struc->fields.names[cont-1][contador];
			fputc(chr,new_db);
		}

		fputc(db_struc->fields.tipos[contador],new_db);
		if( db_struc->fields.tipos[contador] == 'M')
		{
			db_struc->fields.longitudes[contador] = 10;
		}
		fputc(0x00,new_db);
		fputc(0x00,new_db);
		fputc('%',new_db);
		fputc('F',new_db);
		fputc(db_struc->fields.longitudes[contador],new_db);
		
		fputc(db_struc->fields.decimales[contador],new_db);
		for( cont2 = 1; cont2 <= 14; ++cont2)
		fputc(0x00,new_db);
		
	}
	
	fputc(0x0D,new_db); /****** Very important ********/
	fputc(0xA1,new_db);
	fflush(new_db);
/*************************** End of Fields stuff **************************/
	fclose(new_db);

	/* Auto-create companion .dbt file if database has memo fields */
	if (has_memo) {
		char dbt_name[1024];
		char *dot = strrchr(_name, '.');
		if (dot != NULL) {
			/* Replace extension: copy base name, append ".dbt" */
			int base_len = (int)(dot - _name);
			memcpy(dbt_name, _name, base_len);
			memcpy(dbt_name + base_len, ".dbt", 4);
			dbt_name[base_len + 4] = '\0';
		} else {
			snprintf(dbt_name, sizeof(dbt_name), "%s.dbt", _name);
		}
		create_dbt_file(dbt_name);
	}

	return 0;
}
/******************** End of create database function *********************/


int create_dbt_file(char *_name)
{
char *dbt_ahead = (char *) malloc(512 + 1); /* The block size in dBase is 512 */
int i;
FILE *new_dbt;

if( dbt_ahead == NULL)
{
#ifdef DEBUG
	fprintf(stderr,"Error, not enought memory\n");
	fflush(stderr);
#endif
	return -1;
}


/************************* First Block stuff **************************/
dbt_ahead[0] = 0x1;
dbt_ahead[1] = 0x00;
dbt_ahead[2] = 0x00;
dbt_ahead[3] = 0x00;

for( i = 4; i<=511; ++i)
	dbt_ahead[i] = 0x00;
		
dbt_ahead[16] = 0x03; /* Version of dBase III+ */

/********************* End of First block stuff **********************/

if( access(_name,F_OK | R_OK) == 0)
{
#ifdef DEBUG
	fprintf(stderr,"Filename exist and wont be overwritten\n");
	fflush(stderr);
#endif
	free(dbt_ahead);
	return -1;
}
if( (new_dbt = fopen(_name,"wb")) == NULL)
{
#ifdef DEBUG
	fprintf(stderr,"Cant create dbt file\n");
	fflush(stderr);
#endif
	free(dbt_ahead);
	return -1;
}

fwrite(dbt_ahead,1,512,new_dbt);
free(dbt_ahead);
fclose(new_dbt);
return 0;
}


