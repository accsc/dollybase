#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "unistd.h"
#include "sys/stat.h"
#include "time.h"
#include "libdbase.h"

DATABASEDBF skip_index(DATABASEDBF asp);


int field_name(DATABASEDBF *asp, int campon, char **name)
{
int ui;
char *aquello;
aquello = (char *) malloc(257);
if( campon <=128)
{
	for(ui = 0; ui<= 10; ++ui)
	{
		aquello[ui] = asp->fields.names[ui][campon];
	}
	aquello[11] = '\0'; /* Null-terminate the 11-char field name */
	strcpy(*name,aquello);
}else{
free(aquello);
return -1;
}
free(aquello);
return 0;
}


void skip(DATABASEDBF **bsf)
{
	if( (**bsf).current < (**bsf).recnos )
		(**bsf).current++;
	return;
}

void lupdate(DATABASEDBF *asp,char **date)
{
*date=malloc(12);
if( *date == NULL)
return;

strcpy(*date,asp->date);
return;
}

/* Update the DBF header last-update date to today.
   Writes bytes 1-3 of the header (year, month, day) and updates asp->date. */
void dbf_update_date(DATABASEDBF *asp)
{
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    int year = t->tm_year % 100;  /* last 2 digits */
    int mes  = t->tm_mon + 1;
    int day  = t->tm_mday;

    /* Update in-memory date string */
    snprintf(asp->date, 10, "%d/%d/%d", day, mes, 2000 + year);

    /* Write to DBF header (bytes 1-3: year, month, day) */
    char dbf_name[1024];
    snprintf(dbf_name, sizeof(dbf_name), "%s.dbf", asp->name);
    FILE *f = fopen(dbf_name, "r+b");
    if (f) {
        fseek(f, 1, SEEK_SET);
        fputc(year % 10, f);   /* last digit of year (dBase convention) */
        fputc(mes, f);
        fputc(day, f);
        fclose(f);
    }
}

int reccount(DATABASEDBF *asp)
{
return asp->recnos;
}

int recno(DATABASEDBF *asp)
{
return asp->current;
}

int recsize(DATABASEDBF *asp)
{
return asp->rec_len;
}

int gotos(DATABASEDBF **bsf, int rec)
{

if( (**bsf).recnos >= rec)
(**bsf).current = rec;
return 0;
} 

int field_to_number(DATABASEDBF *asp, char *name)
{
int i,o,u;
char *dos;

if( (dos = (char *) malloc (14)) == NULL)
return -1;

for( i =1; i<= asp->camposn; ++i)
{
u = 0;
field_name(asp,i,&dos);
if( dos == NULL)
return -1;

	for(o = 0; o<11; ++o)
	{
		if( dos[o] == name[o]  && dos[o] != 0)
		{
			++u;	
		}
	}
	if(u == strlen(name) && strlen(dos) <= strlen(name) )
	{
/*	printf("Es: %i y voy a devolver %i\n",u,i);*/
	free(dos);
	return i; 
	}
	}
free(dos);
return -2;
}

void get_field(DATABASEDBF *asp, int number, char **presion)
{
char *dbt_name = (char *) malloc(1025);
FILE *a;
int i,pos;
char *presion2;
char *presion2_memo = NULL;
	int buf_size = (asp && number >= 1 && number <= 128 && asp->fields.tipos[number] == 'M') ? 1024 : 256;

if( dbt_name == NULL)
{
return;
}

if( (presion2 = (char *) malloc(buf_size + 1)) ==NULL)
{
	free(dbt_name);
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
	*presion = NULL;
	return;
}

if( (a = fopen(asp->name,"rb")) == NULL)
{
	*presion = NULL;
	free(presion2);
	free(dbt_name);
	return;
}

if( asp->current == 0)
asp->current = 1;
pos = ((asp->current-1)*asp->rec_len) + asp->header_len;
for(i = 1; i< number; ++i)
pos = pos + asp->fields.longitudes[i];
++pos;
fseek(a,pos,SEEK_SET);
for( i = 0; i< asp->fields.longitudes[number]; ++i)
{
presion2[i] = getc(a);
}
presion2[asp->fields.longitudes[number]] = 0;

if( asp->fields.tipos[number] == 'M')
{
	get_dbt(asp->name,dbt_name);
	if( asp->tipo == 3)
	{
	    get_memo_field(dbt_name,atoi(presion2),&presion2_memo,1023);
	    free(presion2);
	    presion2 = presion2_memo;
	}else if(asp->tipo == 4)
	    get_db4_memo_block(dbt_name,atoi(presion2),presion2,1023);
}

fclose(a);
/* Trim trailing spaces — but only for non-memo fields.
   Memo fields have content from the DBT that exceeds the DBF field width. */
if( asp->fields.tipos[number] != 'M')
{
	for(i = asp->fields.longitudes[number]-1; i>=1; --i)
	{
		if( presion2[i-1] == 32)
			presion2[i-1] = '\0';
		else
			break;
	}
}
strcpy(*presion, presion2);
free(presion2);
free(dbt_name);
return;
}

int dfield_type(DATABASEDBF *asp, int campon)
{
int ui;
if(campon <= 128)
{
return asp->fields.tipos[campon]; 
}
else
return -1;
}

int replace(DATABASEDBF *asp, char *campo,char *rerum)
{
FILE *a;
int x,pos,y,dos,otro,mas;
char *me, *memo_name;

if( (a = fopen(asp->name,"r+")) ==NULL)
{
#ifdef DEBUG
	fprintf(stderr,"replace. Cant open dbf file for replace\n");
	fflush(stderr);
#endif
	return -1;
}

x = field_to_number(asp,campo);
#ifdef DEBUG
	fprintf(stderr,"Field number: %s -> %i\n",campo,x);
	fprintf(stderr,"Header len: %i\n",asp->header_len);
	fflush(stderr);
#endif
if (x <= 0)
{
  fprintf(stderr,"Invalid field name\n");
  fclose(a);
  return -1;
}


if( asp->fields.tipos[x] == 'M')
{
	if( (me = (char *) malloc (1025) ) == NULL)
	{
#ifdef DEBUG
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
#endif
	fclose(a);
	return -1;
	}

	if( ( memo_name = (char *) malloc (1025)) == NULL)
	{
        fclose(a);
	free(me);
#ifdef DEBUG
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
#endif
	 return -1;
	}

	get_field2(asp,x, &me);  /* Without conversion of memo */
	y = atoi(me);
	get_dbt(asp->name,memo_name);
	if( y == 0)
	{
		/* New record: memo block not yet allocated — add it */
		FILE *dbt_f = fopen(memo_name, "rb");
		int new_block = 0;
		if( dbt_f)
		{
			char hdr[4];
			fread(hdr, 1, 4, dbt_f);
			fclose(dbt_f);
			new_block = hdr[0] + (hdr[1]*256) + (hdr[2]*65536) + (hdr[3]*16777216);
		}
		if( new_block <= 0)
		{
			free(memo_name);
			free(me);
			fclose(a);
			return -1;
		}
		/* Write the memo content to the .dbt file */
		if( add_to_dbt(memo_name, rerum, strlen(rerum)) != 0)
		{
			free(memo_name);
			free(me);
			fclose(a);
			return -1;
		}
		free(memo_name);
		/* Write the new block pointer back into the DBF record */
		{
			char ptr[11];
			snprintf(ptr, sizeof(ptr), "%010d", new_block);
			int pos2 = ((asp->current-1)*asp->rec_len) + asp->header_len;
			for(mas = 1; mas < x; ++mas)
				pos2 += asp->fields.longitudes[mas];
			++pos2;
			if( field_to_number(asp,"_DBFLOCK") > 0)
				pos2++;
			fseek(a, pos2, SEEK_SET);
			for(dos = 0; dos < (int)strlen(ptr); ++dos)
				fputc(ptr[dos], a);
			for(dos = 0; dos < (asp->fields.longitudes[x] - (int)strlen(ptr)); ++dos)
				fputc(' ', a);
			fflush(a);
		}
	}
	else
	{
		replace_dbt_block(memo_name,y,rerum);
		free(memo_name);
	}
	free(me);
}else{

pos = ((asp->current-1)*asp->rec_len) + asp->header_len;

#ifdef DEBUG
	fprintf(stderr,"replace.Pre-Position: %i\n",pos);
	fflush(stderr);
#endif

for( mas = 1; mas < x; ++mas)
{
#ifdef DEBUG 
	fprintf(stderr,"replace. Field %i size %i sumatory...\n",mas,asp->fields.longitudes[mas]);
#endif

pos = pos + asp->fields.longitudes[mas];
}
++pos; 
#ifdef DEBUG
	fprintf(stderr,"replace.Final position without LOCK field calcules: %i\n",pos);
#endif
if( field_to_number(asp,"_DBFLOCK") > 0)
pos = pos +1;  
fseek(a,pos,SEEK_SET);

#ifdef DEBUG
fprintf(stderr,"replace. Start replacing at pos: %i\n",pos);
fprintf(stderr,"replace. Content - Length: $%s$ - $%i$\n",rerum,strlen(rerum));
fflush(stderr);
#endif

y = asp->fields.longitudes[x] - strlen(rerum);

/* Truncate content and suppress padding if it exceeds field width */
if (y < 0)
y = 0;

#ifdef DEBUG
fprintf(stderr,"replace. Padding with %i blanks because %i-%i=%i\n",y,asp->fields.longitudes[x],strlen(rerum),y);
fflush(stderr);
#endif
for(dos = 0; dos < asp->fields.longitudes[x] && dos < strlen(rerum); ++dos)
fputc(rerum[dos],a);
for(dos = 0; dos < y; ++dos)
fputc(' ',a);
fflush(a);
}
fclose(a);
return 0;
}


int fields_num(DATABASEDBF *asp)
{
return asp->camposn;
}

/* Only for DB III DBT */
int add_to_dbt(char *_na,char *content, int max)
{
	FILE *dbt_file;
	int _next[4];
	char *dbt_block = (char *) malloc( 512 );

	struct stat dbt_info;
	int next_block, i, i2; /* The next block */
	int offset = 0; /* Byte offset into content */
	int blocks_used = 0;

	if( access(_na,F_OK | R_OK | W_OK) != 0)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant manipulate DBT file\n");
		fflush(stderr);
#endif
		free(dbt_block);
		return -1;
	}

	if( (dbt_file = fopen(_na,"r+b")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant open DBT file: $%s$.\n",_na);
		fflush(stderr);
#endif
		free(dbt_block);
		return -1;
	}
	stat(_na,&dbt_info);
	fread(dbt_block,1,512,dbt_file);
#ifdef DEBUG
	if( dbt_block[16] != 0x03)
	{
		fprintf(stderr,"Byte 16 should be 0x03\n");
		fflush(stderr);

	}
#endif
	next_block = dbt_block[0] + (dbt_block[1]*256) + (dbt_block[2]*65536) + (dbt_block[3]*16777216);
#ifdef DEBUG
	fprintf(stderr,"Next Position blocks: %i - %i - %i - %i\n",dbt_block[0],dbt_block[1],dbt_block[2],dbt_block[3]);
	fflush(stderr);
#endif

	/* Write content across one or more 512-byte blocks */
	while (offset < max) {
		int bytes_in_block = max - offset;
		if (bytes_in_block > 510)
			bytes_in_block = 510;

		fseek(dbt_file, next_block * 512, SEEK_SET);
		fwrite(content + offset, 1, bytes_in_block, dbt_file);

		/* Pad remainder with 0x00, then two 0x1A terminators */
		for (i = bytes_in_block; i < 510; ++i)
			fputc(0x00, dbt_file);
		fputc(0x1A, dbt_file);
		fputc(0x1A, dbt_file);

		offset += bytes_in_block;
		next_block++;
		blocks_used++;
	}

	fflush(dbt_file);

	/* Update next-free-block pointer in header */
	fseek(dbt_file, 0L, SEEK_SET);
	_next[3] = next_block / 16777216;
	_next[2] = (next_block - (_next[3] * 16777216)) / 65536;
	_next[1] = (next_block - (_next[3] * 16777216) - (_next[2] * 65536)) / 256;
	_next[0] = next_block - (_next[3] * 16777216) - (_next[2] * 65536) - (_next[1] * 256);
	fwrite(_next, 1, 4, dbt_file);
	fflush(dbt_file);

	fclose(dbt_file);
	free(dbt_block);
	return 0;
}


/* use malloc(max+1) for result pointer. If not, ... Segment fault */

int get_memo_field(char *_na, int block, char **result, int max)
{

	FILE *dbt_file;
	char *dbt_block  = (char *) malloc( 512 );
	struct stat dbt_info;
	int next_block,i,it;
	char *result2;

	if( block == 0)
		block = 1; /* First block is header */

	if( dbt_block == NULL)
	{
#ifdef DEBUG
	fprintf(stderr,"Memo. Not enought memory.\n");
	fflush(stderr);
#endif
	return -1;
	}

	if( (result2 = (char *) malloc ( max+1)) == NULL)
	{
		free(dbt_block);
		return -1;
	}
	if( access(_na,F_OK | R_OK) != 0)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant manipulate DBT file: $%s$.\n",_na);
		fflush(stderr);
#endif
	free(dbt_block);
	free(result2);
		return -1;
	}
	if( (dbt_file = fopen(_na,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant open DBT file: $%s$.\n",_na);
		fflush(stderr);
#endif
	free(dbt_block);
	free(result2);
		return -1;
	}
	stat(_na,&dbt_info);

	fread(dbt_block,1,512,dbt_file);
	
#ifdef DEBUG	
	if( dbt_block[16] != 0x03)
	{
		fprintf(stderr,"Byte 16 should be 0x03\n");
		fflush(stderr);
	}
#endif
	next_block = dbt_block[0] + (256*dbt_block[1]) + (65536*dbt_block[2]) + (16777216*dbt_block[3]);
	if( block > next_block || block > (dbt_info.st_size/512) )
	{
#ifdef DEBUG
		fprintf(stderr,"Block out of file. No such block\n");
		fflush(stderr);
#endif
		free(dbt_block);
		free(result2);
		fclose(dbt_file);
		return -1;
	}
	fflush(stderr);
	fseek(dbt_file,block*512,SEEK_SET);
	if( (next_block-1) == block)
	max = dbt_info.st_size - (block*512);
	it = 0;
	do{
	i = fgetc(dbt_file);
	if (i == EOF) break;
	result2[it] = i;
	++it;
	if( i == 0x00 || i == 0xA1 || it >= max )
		break;
	}while( it < max );
	result2[it] = '\0';
	strcpy(*result,result2);
	free(result2);
	fclose(dbt_file);
	free(dbt_block);
	return 0;
}


int get_next_free_block(char *_na)
{
	FILE *dbt_file;
	char *dbt_block  = (char *) malloc(512);
	struct stat dbt_info;
	int next_block,i,it;

	if( dbt_block == NULL)
	{
#ifdef DEBUG
	fprintf(stderr,"Memo. Not enought memory.\n");
	fflush(stderr);
#endif
	return -1;
	}

	if( (dbt_file = fopen(_na,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Cant open DBT file\n");
		fflush(stderr);
#endif
	free(dbt_block);
		return -1;
	}
	stat(_na,&dbt_info);

	fread(dbt_block,1,512,dbt_file);
	
#ifdef DEBUG	
	if( dbt_block[16] != 0x03)
	{
		fprintf(stderr,"Byte 16 should be 0x03\n");
		fflush(stderr);
	}
#endif
	next_block = dbt_block[0] + (256*dbt_block[1]) + (65536*dbt_block[2]) + (16777216*dbt_block[3]);

	fclose(dbt_file);
	free(dbt_block);
	return next_block;

}

int replace_dbt_block(char *_na, int block, char *content)
{

	FILE *dbt_file;
	int _next[4];
	char *dbt_block = (char *) malloc(512);

	struct stat dbt_info;  
	int next_block, i,i2; /* The next block */
	
	if( dbt_block == NULL)
	{
#ifdef DEBUG
	fprintf(stderr,"Memo. Error. Not enought memory.\n");
	fflush(stderr);
#endif
		return -1;
	}


	if( (dbt_file = fopen(_na,"r+b")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"Memo. Error. Cant open DBT file\n");
		fflush(stderr);
#endif
		free(dbt_block);
		return -1;
	}
	stat(_na,&dbt_info);
	fread(dbt_block,1,512,dbt_file);
#ifdef DEBUG
	if( dbt_block[16] != 0x03)
	{
		fprintf(stderr,"Byte 16 should be 0x03\n");
		fflush(stderr);
	
	}
#endif
	next_block = dbt_block[0] + (dbt_block[1]*256) + (dbt_block[2]*65536) + (dbt_block[3]*16777216);
#ifdef DEBUG
	fprintf(stderr,"Next Position blocks: %i - %i - %i - %i\n",dbt_block[0],dbt_block[1],dbt_block[2],dbt_block[3]);
	fflush(stderr);
#endif
	if( block >= next_block || block < 1)
	{
#ifdef DEBUG
	fprintf(stderr,"Memo. Error. Block out of Range. %i Not in Range [1-%i].\n",block,next_block);
	fflush(stderr);
#endif	
	free(dbt_block);
	fclose(dbt_file);
	return -1;
	}

	fseek(dbt_file,(block*512),SEEK_SET);

/* Perfect, only one block */
	if( strlen(content) <= 510 )
	{
		fwrite(content,1,strlen(content),dbt_file);
		
		for( i = strlen(content); i <= 509; ++i)
			fputc(0x00,dbt_file);
		
		fputc(0x1A,dbt_file); /* Two EOF markers for DBT */
		fputc(0x1A,dbt_file);
	}else{ 
#ifdef DEBUG
	fprintf(stderr,"Memo. Error. Cant overwrite more than one block.\n");
	fflush(stderr);
#endif
	free(dbt_block);
	fclose(dbt_file);
	return -1;
	}
	fflush(dbt_file);

	
	fclose(dbt_file);
	free(dbt_block);
	return 0;
}



void get_field2(DATABASEDBF *asp, int number, char **presion)
{
char *dbt_name = (char *) malloc(1025);
FILE *a;
int i,pos;
char *presion2;
	
if( dbt_name == NULL)
{
return;
}

if( (presion2 = (char *) malloc(257)) ==NULL)
{
	free(dbt_name);
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
	*presion = NULL;
	return;
}

if( (a = fopen(asp->name,"rb")) == NULL)
{
	*presion = NULL;
	free(presion2);
	free(dbt_name);
	return;
}

if( asp->current == 0)
asp->current = 1;
pos = ((asp->current-1)*asp->rec_len) + asp->header_len;
for(i = 1; i< number; ++i)
pos = pos + asp->fields.longitudes[i];
++pos;
fseek(a,pos,SEEK_SET);
for( i = 0; i< asp->fields.longitudes[number]; ++i)
{
presion2[i] = getc(a);
}
presion2[asp->fields.longitudes[number]] = 0;

fclose(a);
for(i = asp->fields.longitudes[number]-1; i>=1; --i)
{
	if( presion2[i-1] == 32)
		presion2[i-1] = '\0';
	else
		break;
}
strcpy(*presion, presion2);
free(presion2);
free(dbt_name);
return;
}




int replace2(DATABASEDBF *asp, char *campo,char *rerum)
{
FILE *a;
int x,pos,y,dos,otro,mas;
char *me, *memo_name;

if( (a = fopen(asp->name,"r+")) ==NULL)
{
#ifdef DEBUG
	fprintf(stderr,"replace. Cant open dbf file for replace\n");
	fflush(stderr);
#endif
	return -1;
}

x = field_to_number(asp,campo);
#ifdef DEBUG
	fprintf(stderr,"Field number: %s -> %i\n",campo,x);
	fprintf(stderr,"Header len: %i\n",asp->header_len);
	fflush(stderr);
#endif
if (x <= 0)
{
  fprintf(stderr,"Invalid field name\n");
  fclose(a);
  return -1;
}

pos = ((asp->current-1)*asp->rec_len) + asp->header_len;

#ifdef DEBUG
	fprintf(stderr,"replace.Pre-Position: %i\n",pos);
	fflush(stderr);
#endif

for( mas = 1; mas < x; ++mas)
{
#ifdef DEBUG 
	fprintf(stderr,"replace. Field %i size %i sumatory...\n",mas,asp->fields.longitudes[mas]);
#endif

pos = pos + asp->fields.longitudes[mas];
}
++pos; 
#ifdef DEBUG
	fprintf(stderr,"replace.Final position without LOCK field calcules: %i\n",pos);
#endif
if( field_to_number(asp,"_DBFLOCK") > 0)
pos = pos +1;  
fseek(a,pos,SEEK_SET);

#ifdef DEBUG
fprintf(stderr,"replace. Start replacing at pos: %i\n",pos);
fprintf(stderr,"replace. Content - Length: $%s$ - $%i$\n",rerum,strlen(rerum));
fflush(stderr);
#endif

y = asp->fields.longitudes[x] - strlen(rerum);

/* Truncate content and suppress padding if it exceeds field width */
if (y < 0)
y = 0;

#ifdef DEBUG
fprintf(stderr,"replace. Padding with %i blanks because %i-%i=%i\n",y,asp->fields.longitudes[x],strlen(rerum),y);
fflush(stderr);
#endif
for(dos = 0; dos < asp->fields.longitudes[x] && dos < strlen(rerum); ++dos)
fputc(rerum[dos],a);
for(dos = 0; dos < y; ++dos)
fputc(' ',a);
fflush(a);
fclose(a);
return 0;
}


