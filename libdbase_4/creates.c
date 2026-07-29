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
int contador, cont, cont2;
char chr;
char header[32];
int has_memo = 0;
int num_fields;
int rec_len;

/* Scan fields for memo type before writing header */
for( contador = 1; contador <= db_struc->camposn; ++contador)
{
	if( db_struc->fields.tipos[contador] == 'M')
	{
		has_memo = 1;
		break;
	}
}

	if( access(_name,F_OK) == 0)
	{
#ifdef DEBUG
		fprintf(stderr,"File already exist, wont be overwritten\n");
		fflush(stderr);
#endif
		return -3;
	}
	if( (new_db = fopen(_name,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"fopen: Cant open file for make a new db\n");
		fflush(stderr);
#endif
		return -2;
	}

/***************************** Fixed header (32 bytes) *********************/

	/* Calculate header_len and rec_len */
	num_fields = db_struc->camposn + (multi ? 1 : 0);
	rec_len = 1; /* delete flag byte */
	for( contador = 1; contador <= db_struc->camposn; ++contador)
	{
		if (db_struc->fields.tipos[contador] == 'M')
			rec_len += 10;
		else
			rec_len += db_struc->fields.longitudes[contador];
	}
	if (multi)
		rec_len += 2; /* _DBFLOCK field length */

	/* header_len = 32 (fixed) + 32 * num_fields + 1 (0x0D terminator) */
	int header_len = 32 + 32 * num_fields + 1;

	/* Build the 32-byte fixed header */
	memset(header, 0, 32);
	header[0] = has_memo ? 0x83 : 0x03;
	header[1] = year;
	header[2] = month;
	header[3] = day;
	/* Record count = 0 (bytes 4-7 already zeroed) */
	/* header_len LE at bytes 8-9 */
	header[8] = header_len & 0xFF;
	header[9] = (header_len >> 8) & 0xFF;
	/* rec_len LE at bytes 10-11 */
	header[10] = rec_len & 0xFF;
	header[11] = (rec_len >> 8) & 0xFF;
	/* Bytes 12-31 are zero (reserved) */

	fwrite(header, 1, 32, new_db);

/****************************** Field descriptors **************************/
	/*********** Add LOCK field - Optional ******************/
	if( multi == 1)
	{
#ifdef DEBUG
		fprintf(stderr,"CREATE. _DBFLOCK field added\n");
		fflush(stderr);
#endif
		/* Name: _DBFLOCK (11 bytes, zero-padded) */
		{
		const char lockname[11] = {'_','D','B','F','L','O','C','K',0,0,0};
		fwrite(lockname, 1, 11, new_db);
		}
		fputc('N', new_db);          /* type */
		fputc(0x00, new_db);         /* reserved */
		fputc(0x00, new_db);         /* reserved */
		fputc(0x00, new_db);         /* reserved */
		fputc(0x00, new_db);         /* reserved */
		fputc(2, new_db);            /* field length */
		fputc(0, new_db);            /* decimal count */
		for( cont2 = 1; cont2 <= 14; ++cont2)
			fputc(0x00, new_db);     /* rest (14 bytes) */
	}

 	/******************* End of LOCK field *****************/
		
		
	for( contador = 1; contador <= db_struc->camposn; ++contador)
	{
		/* Field name (11 bytes) */
		for( cont = 0; cont <= 10; cont++)
		{
			chr = db_struc->fields.names[cont][contador];
			fputc(chr, new_db);
		}

		/* Field type */
		fputc(db_struc->fields.tipos[contador], new_db);

		/* Reserved (4 bytes) */
		fputc(0x00, new_db);
		fputc(0x00, new_db);
		fputc(0x00, new_db);
		fputc(0x00, new_db);

		/* Field length */
		if( db_struc->fields.tipos[contador] == 'M')
			fputc(10, new_db);
		else
			fputc(db_struc->fields.longitudes[contador], new_db);

		/* Decimal count */
		fputc(db_struc->fields.decimales[contador], new_db);

		/* Rest (14 bytes) */
		for( cont2 = 1; cont2 <= 14; ++cont2)
			fputc(0x00, new_db);
	}

	/* Header terminator */
	fputc(0x0D, new_db);
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
		
dbt_ahead[16] = 0x03; /* DBT version byte */

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


