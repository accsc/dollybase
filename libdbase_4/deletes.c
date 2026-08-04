
/*
Funciones: delete,recall,recall_all,delete_all,zap y pack.
*/

#include <stdio.h>
#include <string.h>
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
pos = ((asp->current-1)*asp->rec_len) + asp->header_len;
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
pos = ((asp->current-1)*asp->rec_len) + asp->header_len;
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
pos = ((asp->current-1)*asp->rec_len) + asp->header_len;
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
char nomb_tmp[2048];
int mas = 0;
estos = 0;
pos = reccount(asp);
gotos(&asp,1);
#ifdef DEBUG
fprintf(stderr,"Warning: pack() called, this is a alpha function\n");
fprintf(stderr,"If something goes wrong i've copied the db in other temp file\n");
fflush(stderr);
#endif
/* Create temp file in same directory as source to avoid cross-filesystem rename */
{
const char *slash = strrchr(asp->name, '/');
int dir_len = slash ? (int)(slash + 1 - asp->name) : 0;
snprintf(nomb_tmp, sizeof(nomb_tmp), "%.*s.dolly_tmp_", dir_len, asp->name);
int prefix_len = (int)strlen(nomb_tmp);
for (int k = 0; k < 6 && prefix_len + k < (int)sizeof(nomb_tmp) - 1; k++)
nomb_tmp[prefix_len + k] = rand() % 10 + '0';
nomb_tmp[prefix_len + 6] = '\0';
}
if( (h = fopen(asp->name,"rb")) == NULL)
	return -1;
if ( (j = fopen(nomb_tmp,"wb")) == NULL)
{
	fclose(h);
	return -1;
}
for(mas = 1; mas <= asp->header_len; ++mas)
{
	putc(getc(h),j);
}
for(i=1; i<= pos; ++i)
{
if( is_deleted(asp) == VERITAS){
fseek(h,asp->rec_len,SEEK_CUR);
estos++;
}else{
for( mas=0; mas < asp->rec_len; ++mas)
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
fseek(j,asp->header_len,SEEK_SET);
for(mas=0; mas < asp->rec_len; ++mas)
fputc(' ',j);
}
}
fclose(j);
rename(nomb_tmp,asp->name);

return 0;
}

/* Helper for pack_db_with_dbt_file: find new block number for a given old block number */
static int _find_new_block(int old_bn, int count, int *map_old, int *map_new)
{
	for (int i = 0; i < count; ++i)
	{
		if (map_old[i] == old_bn)
			return map_new[i];
	}
	return 0;
}

int pack_db_with_dbt_file(DATABASEDBF *asp, char *_na)
{
	FILE *input;
	FILE *dbt_file;
	FILE *output;
	FILE *output_dbt;

	int max_recs, contador, c2, deletes;
	char name_tmp[2048], dbt_tmp[2048];
	char dbt_header[512];

	/* Collect memo block references from surviving records */
	#define MAX_MEMO_BLOCKS 4096
	int memo_blocks[MAX_MEMO_BLOCKS];
	int memo_block_count = 0;

	/* Block mapping: old_block -> new_block (parallel arrays) */
	int map_old[MAX_MEMO_BLOCKS];
	int map_new[MAX_MEMO_BLOCKS];

	max_recs = reccount(asp);
	gotos(&asp, 1);

	/* Create temp files in same directory to avoid cross-filesystem rename */
	{
		const char *slash = strrchr(asp->name, '/');
		int dir_len = slash ? (int)(slash + 1 - asp->name) : 0;
		snprintf(name_tmp, sizeof(name_tmp), "%.*s.dolly_tmp_", dir_len, asp->name);
		for (int k = dir_len + 12; k < (int)sizeof(name_tmp); k++)
			name_tmp[k] = rand() % 10 + '0';
		snprintf(dbt_tmp, sizeof(dbt_tmp), "%.*s.dbtd_tmp_", dir_len, _na);
		for (int k = dir_len + 11; k < (int)sizeof(dbt_tmp); k++)
			dbt_tmp[k] = rand() % 10 + '0';
	}
#ifdef DEBUG
	fprintf(stderr, "Warning: Pack_db_with_dbt_file is an alpha function\n");
	fflush(stderr);
#endif
	if ((dbt_file = fopen(_na, "rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "Cant access to dbt file\n");
		fflush(stderr);
#endif
		return -1;
	}
	if ((output_dbt = fopen(dbt_tmp, "wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "Cant open temp file\n");
		fflush(stderr);
#endif
		fclose(dbt_file);
		return -1;
	}
	if ((input = fopen(asp->name, "rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "Cant access to db file\n");
		fflush(stderr);
#endif
		fclose(dbt_file);
		fclose(output_dbt);
		return -1;
	}
	if ((output = fopen(name_tmp, "wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr, "Cant open tempory file\n");
		fflush(stderr);
#endif
		fclose(dbt_file);
		fclose(output_dbt);
		fclose(input);
		return -1;
	}

	/* Copy db header to temp file */
	for (contador = 1; contador <= asp->header_len; ++contador)
		fputc(fgetc(input), output);

	/* Read and copy dbt header to temp2 file */
	fread(dbt_header, 1, 512, dbt_file);
	fwrite(dbt_header, 1, 512, output_dbt);

	/* First pass: collect memo block numbers from surviving records */
	gotos(&asp, 1);
	deletes = 0;
	for (contador = 1; contador <= max_recs; ++contador)
	{
		if (is_deleted(asp) == VERITAS)
		{
			++deletes;
		}
		else
		{
			/* For each memo field in this record, collect the block number */
			char *memo_val = malloc(16);
			for (c2 = 1; c2 <= asp->camposn; ++c2)
			{
				if (asp->fields.tipos[c2] == 'M')
				{
					get_field2(asp, c2, &memo_val);
					int block_num = atoi(memo_val);
					if (block_num > 0 && memo_block_count < MAX_MEMO_BLOCKS)
					{
						/* Check if already collected */
						int already = 0;
						for (int k = 0; k < memo_block_count; ++k)
						{
							if (memo_blocks[k] == block_num)
							{
								already = 1;
								break;
							}
						}
						if (!already)
							memo_blocks[memo_block_count++] = block_num;
					}
				}
			}
			free(memo_val);
		}
		skip(&asp);
	}

	/* Build block mapping and copy surviving memo blocks to new DBT */
	int new_block = 1; /* First data block in new DBT (block 0 is header) */
	for (int i = 0; i < memo_block_count; ++i)
	{
		map_old[i] = memo_blocks[i];
		map_new[i] = new_block;

		/* Copy the block from old DBT to new DBT */
		fseek(dbt_file, memo_blocks[i] * 512, SEEK_SET);
		char block_buf[512];
		fread(block_buf, 1, 512, dbt_file);
		fseek(output_dbt, new_block * 512, SEEK_SET);
		fwrite(block_buf, 1, 512, output_dbt);

		new_block++;
	}

	fflush(output_dbt);

	/* Second pass: copy surviving records, remapping memo block numbers */
	gotos(&asp, 1);
	int surviving = 0;
	for (contador = 1; contador <= max_recs; ++contador)
	{
		if (is_deleted(asp) == VERITAS)
		{
			fseek(input, asp->rec_len, SEEK_CUR);
		}
		else
		{
			surviving++;
			/* Copy the record byte-by-byte from input to output */
			for (c2 = 0; c2 < asp->rec_len; c2++)
				fputc(fgetc(input), output);

			/* Remap memo block numbers in the output file */
			for (c2 = 1; c2 <= asp->camposn; ++c2)
			{
				if (asp->fields.tipos[c2] == 'M')
				{
					/* Calculate position of this memo field in the output file */
					int pos = asp->header_len; /* After header */
					pos += (surviving - 1) * asp->rec_len; /* Record start */

					/* Offset to this field within the record */
					int field_offset = 1; /* Skip delete flag byte */
					for (int f = 1; f < c2; ++f)
						field_offset += asp->fields.longitudes[f];

					int memo_pos = pos + field_offset;

					/* Read old block number from output file */
					fseek(output, memo_pos, SEEK_SET);
					char old_block_str[11];
					fread(old_block_str, 1, 10, output);
					old_block_str[10] = '\0';
					int old_bn = atoi(old_block_str);

					/* Write new block number */
					char new_block_str[11];
					snprintf(new_block_str, sizeof(new_block_str), "%d", _find_new_block(old_bn, memo_block_count, map_old, map_new));
					int new_len = strlen(new_block_str);

					fseek(output, memo_pos, SEEK_SET);
					fwrite(new_block_str, 1, new_len, output);
					/* Pad remainder with spaces */
					for (int p = new_len; p < 10; ++p)
						fputc(' ', output);
				}
			}
		}
		skip(&asp);
	}

	fclose(input);
	fclose(dbt_file);
#ifdef DEBUG
	fprintf(stderr, "Pack done. Pack %i/%i\n", deletes, max_recs);
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
		fseek(output, 4, SEEK_SET);
		fputc(r1, output);
		fputc(r2, output);
		fputc(r3, output);
		fputc(r4, output);
		if (rec_count == 0)
		{
			fseek(output, asp->header_len, SEEK_SET);
			for (c2 = 0; c2 < asp->rec_len; ++c2)
				fputc(' ', output);
		}
	}

	fclose(output);
	rename(name_tmp, asp->name);

	/* Update next block in dbt */
	fseek(output_dbt, 0, SEEK_SET);
	fputc(new_block & 0xFF, output_dbt);
	fputc((new_block >> 8) & 0xFF, output_dbt);
	fputc((new_block >> 16) & 0xFF, output_dbt);
	fputc((new_block >> 24) & 0xFF, output_dbt);
	fclose(output_dbt);
	rename(dbt_tmp, _na);
	return 0;
}
