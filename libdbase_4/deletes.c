
/*
Funciones: delete,recall,recall_all,delete_all,zap y pack.
*/

#include <stdio.h>
#include <stdlib.h>
#include "libdbase.h"

/* Añade la marca * de borrado al registro actual del espacio actual */

int delete(DATABASEDBF *asp)
{
FILE *o;
int pos;
if( (o = fopen(asp->name,"r+")) == NULL)
{
#ifdef DEBUG
	fprintf(stderr,"deletes. Cant open file for deleting.\n");
	fflush(stderr);
#endif
return -1;
}
pos = ((asp->current-1)*asp->rec_len) + asp->header_len + asp->current ;
fseek(o,pos,SEEK_SET);
fputc('*',o);
fclose(o);
return 0;
}
/* Añade la marca de borrado al reg actual y a 'n' siguientes */
int delete_next(DATABASEDBF *asp, int n)
{
int u;
for(u = 0; u< n; ++u)
{
if( delete(asp) != 0)
return -1;
else
skip(&asp);
}
return 0;
}
int delete_all(DATABASEDBF *asp)
{
gotos(&asp,1); 
delete_next(asp, asp->recnos); 
return 0;
}
/* Quita la marca de borrado al registro actual del espacio actual */
int recall(DATABASEDBF *asp)
{
FILE *o;
int pos;

if( (o = fopen(asp->name,"r+")) == NULL)
{
#ifdef DEBUG
	fprintf(stderr,"deletes. Cant open file for recall\n");
	fflush(stderr);
#endif
return -1;
}
pos = ((asp->current-1)*asp->rec_len) + asp->header_len + asp->current;
fseek(o,pos,SEEK_SET);
fputc(' ',o);
fclose(o);
return 0;
}
/* Quita la marca de borrada al reg actual y a 'n' siguientes */
int recall_next(DATABASEDBF **asp, int n)
{
DATABASEDBF *current = *asp;
int u;
for(u = 0; u< n; ++u)
{
recall(current);
skip(&current);
}
*asp = current;
return 0;
}
int recall_all(DATABASEDBF *asp)
{
gotos(&asp,1); recall_next(&asp, asp->recnos); 
return 0;
}
/* Borra todos los registros de la base de datos */
int zap(DATABASEDBF *asp)
{
if( delete_all(asp) != 0)
{
#ifdef DEBUG
	fprintf(stderr,"deletes. Zap failed, cant delete_all!\n");
	fflush(stderr);
#endif
	return -1;
}
if ( pack(asp) != 0)
{
#ifdef DEBUG
	fprintf(stderr,"deletes. Zap failed, cant pack!!!!, women & children first!\n");
	fflush(stderr);
#endif
	return -2;
}
return 0;
}
int is_deleted(DATABASEDBF *asp)
{
FILE *a;
int pos = 0;
char b;
if( (a = fopen(asp->name,"rb")) == NULL)
{
#ifdef DEBUG
	fprintf(stderr,"deletes. Cant open file for verify delete state\n");
	fflush(stderr);
#endif
	return -2;
}
pos = ((asp->current-1)*asp->rec_len) + asp->header_len + asp->current;
fseek(a,pos,SEEK_SET);
b=getc(a);
fclose(a);
if(b == '*')
return VERITAS;
else
return FALSO;
}

int pack(DATABASEDBF *asp)
{
FILE *h;
FILE *j; 
int pos,estos,i;
char *nomb_tmp;
int mas = 0;
estos = 0;
pos = reccount(asp);
gotos(&asp,1);
#ifdef DEBUG
fprintf(stderr,"Warning: pack() called, this is a alpha function\n");
fprintf(stderr,"If something goes wrong i've copied the db in other temp file\n");
fflush(stderr);
#endif
nomb_tmp = tempnam(NULL,"dolly");
if( (h = fopen(asp->name,"rb")) == NULL)
	return -1;
if ( (j = fopen(nomb_tmp,"wb")) == NULL)
	return -1;
for(mas = 1; mas <= (asp->header_len+1); ++mas)
{
	putc(getc(h),j);
}
for(i=1; i<= pos; ++i)
{
if( is_deleted(asp) == VERITAS){
fseek(h,asp->rec_len+1,SEEK_CUR);
estos++;
}else{
for( mas=0; mas < (asp->rec_len+1); ++mas)
fputc(getc(h),j);
}
skip(&asp);
}
#ifdef DEBUG
fprintf(stderr,"Registros eliminados %i/%i\n",estos,pos);
fflush(stderr);
#endif
fclose(h);
{
int rec_count = pos - estos;
int r1, r2, r3, r4;
r4 = rec_count / 16777216;
r3 = (rec_count - r4 * 16777216) / 65536;
r2 = (rec_count - r4 * 16777216 - r3 * 65536) / 256;
r1 = rec_count - r4 * 16777216 - r3 * 65536 - r2 * 256;
fseek(j,4,SEEK_SET);
fputc(r1,j);
fputc(r2,j);
fputc(r3,j);
fputc(r4,j);
if( rec_count == 0)
{
fseek(j,asp->header_len+1,SEEK_SET);
for(mas=0; mas <= asp->rec_len; ++mas)
fputc(' ',j);
}
}
fclose(j);
rename(nomb_tmp,asp->name);

return 0;
}

int pack_db_with_dbt_file(DATABASEDBF *asp, char *_na)
{
	FILE *input;
	FILE *dbt_file;
	FILE *output;
	FILE *output_dbt;
	
	int max_recs,contador,c2,deletes, next_block;
	char *name_tmp, *dbt_tmp;
	
	max_recs = reccount(asp);
	gotos(&asp,1);

	name_tmp = tempnam(NULL,"dolly");
	dbt_tmp = tempnam(NULL,"dbtd");
#ifdef DEBUG	
	fprintf(stderr,"Warning: Pack_db_with_dbt_file is an alpha function\n");
	fflush(stderr);
#endif
	if( (dbt_file = fopen(_na,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant access to dbt file\n");
		fflush(stderr);
#endif
		return -1;
	}
	if( (output_dbt = fopen(dbt_tmp,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant open temp file\n");
		fflush(stderr);
#endif
		return -1;
	}
	if( (input = fopen(asp->name,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant access to db file\n");
		fflush(stderr);
#endif
		return -1;
	}
	if( (output = fopen(name_tmp,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant open tempory file\n");
		fflush(stderr);
#endif
		return -1;
	}

	/* Copy db header to temp file */
	for( contador = 1; contador <= (asp->header_len+1); ++contador)
		fputc( fgetc(input), output);
	
	/* Copy dbt header to temp2 file */
	for( contador = 0; contador <= 511; ++contador )
		fputc(fgetc(dbt_file),output_dbt); 
	
	/* Copy non-deleted recs */
	deletes = 0;
	for( contador = 1; contador <= max_recs; ++contador)
	{
		if( is_deleted(asp) == VERITAS)
		{
			fseek(input, (asp->rec_len+1), SEEK_CUR);
			++deletes;
		}else{
			for( c2 = 0; c2 < (asp->rec_len+1); c2++)
				fputc( fgetc(input), output);
			for( c2 = 0; c2 <= 511; ++c2)
				fputc( fgetc(dbt_file), output_dbt);
		}

		skip(&asp);
	}

	fclose(input);	
	fclose(dbt_file);
#ifdef DEBUG
	fprintf(stderr,"Pack done. Pack %i/%i\n",deletes,max_recs);
	fflush(stderr);
#endif

	/* Update number of recs in db */
	{
	int rec_count = max_recs - deletes;
	int r1, r2, r3, r4;
	r4 = rec_count / 16777216;
	r3 = (rec_count - r4 * 16777216) / 65536;
	r2 = (rec_count - r4 * 16777216 - r3 * 65536) / 256;
	r1 = rec_count - r4 * 16777216 - r3 * 65536 - r2 * 256;
	fseek(output,4,SEEK_SET);
	fputc(r1,output);
	fputc(r2,output);
	fputc(r3,output);
	fputc(r4,output);
	if( rec_count == 0)
	{
		fseek(output,asp->header_len+1,SEEK_SET);
		for( c2 = 0; c2<=asp->rec_len; ++c2)
			fputc(' ',output);
	}
	}
	
	fclose(output);
	rename(name_tmp,asp->name);

	
	/* Update next block in dbt */
	next_block = max_recs - deletes;
	fseek(output_dbt,0,SEEK_SET);

	fputc(0,output_dbt);
	fputc(0,output_dbt);
	fputc(0,output_dbt);
	fputc(next_block+1,output_dbt);
	fclose(output_dbt);
	rename(dbt_tmp,_na);
	return 0;
}
