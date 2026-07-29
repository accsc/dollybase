#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libdbase.h"


#define MAX_HEAD 1024 /* Max 1Kb of header */
/*char *located;
char *located_campo;
char locate_is=0; */
/*
	Use. Use a database with asp structure 
*/
void use(char *dbf_name, DATABASEDBF **bsp)
{
FILE *data;
int con = 0, zise = 0, conter = 0, i =0;
int runn = 0,prun = 0;
int day,mes,year;
DATABASEDBF *asp;
int u[4]; /* Better for FoxBASE reccounts */
asp = (DATABASEDBF *) malloc( sizeof(DATABASEDBF));

	if( asp == NULL)
	{
		fprintf(stderr,"Error. Sin memoria.\n");
		fflush(stderr);
		return;
	}
asp->located = NULL;
asp->located_campo = 0;
asp->tipo= 0;
asp->rec_len =0;
	if( (data = fopen(dbf_name,"rb")) == NULL)
	{
		fprintf(stderr,"Error. No existe la base de datos.\n");
		fflush(stderr);
		free(asp);
		return;
	}
asp->header_len = 0;
switch( getc(data) ){
case 0x02: fprintf(stderr,"dBASE II/FoxBASE Database, DB II supported not tested yet\n"); break;
case 0x03: asp->tipo= 1; break;
case 0x04: asp->tipo = 1; break;
case 0x05: asp->tipo= 1; break;
case 0x83: asp->tipo= 3; /*fprintf(stderr,"Database uses a DBT file, untested!!\n");*/ break;
case 0x7B: asp->tipo = 4; fprintf(stderr,"Database uses DB IV DBT file format, limited support untested\n"); break;
case 0x8B: asp->tipo = 4; fprintf(stderr,"Database uses DB IV DBT file format, limited support untested\n"); break;
case 0x30:  asp->tipo =5; fprintf(stderr,"Visual FoxPro Database. Not supported fully\n"); break;
case 0x31: asp->tipo = 5; fprintf(stderr,"Visual Foxpro Database with autoincrement enable. Not fully supported.\n");	    
case 0x8E: fprintf(stderr,"This database use SQL Table, not supported\n"); break;
case 0xF5: asp->tipo = 5; fprintf(stderr,"FoxPro 2.x Database with .fmp memo, FMP not supported.\n"); 
break;
case 0x43: fprintf(stderr,"dBase IV SQL table file. Not supported at all.\n"); break;
case 0x63: fprintf(stderr,"dBase IV SQL system files. Not supported at all.\n"); break;
case 0xCB: fprintf(stderr,"dBase IV SQL table files with memo. Not supported at all.\n"); break;
case 0xFB: asp->tipo = 3 ; break;
default: fprintf(stderr,"Error. Base de datos no soportada.\n"); fflush(stderr); return;  break;
}

year = getc(data);
mes = getc(data);
day = getc(data);
if( year == 0)
snprintf(asp->date,10,"%i/%i/2000",day,mes); 
else if( year < 10 && year > 0)
snprintf(asp->date,10,"%i/%i/200%i",day,mes,year);
else if( year >= 10 &&  year <= 80)
snprintf(asp->date,10,"%i/%i/20%i",day,mes,year);
else if( year >= 81 && year <= 99)
snprintf(asp->date,10,"%i/%i/19%i",day,mes,year);
else if( year > 99)
snprintf(asp->date,10,"%i/%i/2%i",day,mes,year);
else
snprintf(asp->date,10,"%i/%i/200%i",day,mes,year);
/* header_len and rec_len will be read from header offsets 8-11 */
/*asp->recnos = getc(data)+(getc(data)*256)+(getc(data)*65536)+(getc(data)*16777216); */
u[0] = getc(data);
u[1] = getc(data);
u[2] = getc(data);
u[3] = getc(data);
asp->recnos = (16777216*u[3])+(65536*u[2])+(u[1]*256)+u[0]; 

for( i = 0; i<1022; ++i)
{
	if(dbf_name[i] == '\0')
		break;
	asp->name[i] = dbf_name[i];
}
asp->name[i] = '\0';

if( asp->recnos < 0)
	asp->recnos = u[3]+(u[2]*256)+(u[1]*65536)+u[0]*16777216;
if( asp->recnos < 0)
	asp->recnos = 0;

/* Read header_len and rec_len from header (offsets 8-11) */
{
int hl0 = getc(data);
int hl1 = getc(data);
asp->header_len = hl0 + (hl1 * 256);
int rl0 = getc(data);
int rl1 = getc(data);
asp->rec_len = rl0 + (rl1 * 256);
/* Skip remaining 4 bytes of the 8-byte block (offsets 12-15) */
for( con = 1; con <= 4; con++)
getc(data);
}
if(getc(data) == 0x01)
{
#ifdef DEBUG
fprintf(stderr,"Database Encrypted, not supported\n"); 
fflush(stderr);
#endif
} 
for( con = 1; con<= 12+1; con ++)
{
getc(data);
}
if( getc(data) != 0x00)
{
#ifdef DEBUG
fprintf(stderr,"The database is foxpro and use codepages, the codepages will be ignored\n");
fflush(stderr);
#endif
}
getc(data);
prun = 0;
do{
++conter;
if( conter == MAX_HEAD)
{
#ifdef DEBUG
	fprintf(stderr,"DB corruption, libdollybase cant read db file. Incorrect format?\n");
	fprintf(stderr,"Quiting ...\n");
	fflush(stderr);
#endif
	printf("LibDollyBASE FATAL ERROR, QUITING MAIN PROGRAM\n");
	fflush(stdout);
	exit(-1); /* No chance, killing app or Segmentation Fault */
}
asp->camposn=prun;
runn = getc(data);
if ( runn == 0x0D)
{
	asp->current = 1;
	fclose(data);
	memcpy(*bsp,asp, sizeof( DATABASEDBF));
	free(asp);
	return;
}
prun++;
asp->fields.names[0][prun] = runn; 
for( con = 1; con <= 10; ++con)
{ 
asp->fields.names[con][prun] = getc(data);
} 
asp->fields.names[11][prun] = '\0';
asp->fields.tipos[prun] = getc(data); 
for(con = 1; con <= 4; con++)
{
getc(data);
}
zise = getc(data);
asp->fields.longitudes[prun] = zise; 
asp->fields.decimales[prun] = getc(data);
for(con = 1; con <= 14; con++)
{
runn = getc(data);
}
}while( runn != 0x0D || conter == MAX_HEAD);

asp->current = 1;
fclose(data);
memcpy(*bsp,asp, sizeof( DATABASEDBF));
free(asp);
return;
}

/*
   Display Structure. Show the structure of the database 
*/
int display_structure(DATABASEDBF *asp)
{
int i,i2;
int falta,tr;
tr=0;
	if(asp->tipo == 0)
	{
	fprintf(stdout,"No database in use\n");
	return -1;
	}
	fprintf(stdout,"DB name: %s\n",asp->name);
        fprintf(stdout,"Recs: %10i\n",asp->recnos);
	fprintf(stdout,"Last Update Date: %s\n",asp->date);
	fprintf(stdout,"Field  Name      Type   Size    Decimals\n");

	for(i =	1; i<= asp->camposn; ++i)
	{	
	falta = 0;
	printf("%5i  ",i);
		for(i2 = 0; i2<= 10; ++i2)
		{
		if( asp->fields.names[i2][i] == 0)
		fprintf(stdout," ");
		else
		fprintf(stdout,"%c",asp->fields.names[i2][i]);
		}
	fprintf(stdout,"%c      ",asp->fields.tipos[i]);
	tr = tr + asp->fields.longitudes[i];
	fprintf(stdout,"%2i",asp->fields.longitudes[i]);
	if( asp->fields.tipos[i] == 'N')
	fprintf(stdout,"        %i",asp->fields.decimales[i]);
	fprintf(stdout,"\n");
	}
	fprintf(stdout,"** Total **%17i\n",tr);
	
return 0;

}

/*
	End of file?
	1 - True(VERITAS)
	2 - False(FALSO)	
*/
int eof_dbf(DATABASEDBF *asp)
{
	if(asp->current >= asp->recnos) 
	    return VERITAS;
	else
	    return FALSO;
}
char chr(int c)
{
return c;
}
int bof(DATABASEDBF *asp)
{
        if(asp->current == 1)
            return VERITAS;
        else
            return FALSO;
}

void DBF(DATABASEDBF *asp, char **name)
{
*name = malloc(2048);
strcpy(*name,asp->name);
}

int get_dbt(char *na, char *out)
{
    char *dot = strrchr(na, '.');
    if (dot != NULL) {
        /* Has an extension — replace it in-place (case-preserving) */
        strcpy(out, na);
        /* Find the last char of the extension */
        char *end = out + strlen(out) - 1;
        /* Preserve case: if original was uppercase, keep it */
        if (*end >= 'A' && *end <= 'Z')
            *end = 'T';
        else
            *end = 't';
    } else {
        /* No extension — append ".dbt" */
        snprintf(out, 1024, "%s.dbt", na);
    }
#ifdef DEBUG
	fprintf(stderr,"Using DBT file: %s",out);
	fflush(stderr);
#endif
    return 0;
}

int null_test( char *a)
{
	int i, siz;
	siz = strlen(a);
for( i= 0; i< siz; ++i)
{
	if( a[i] != ' ' && a[i] != '\t' && a[i] != '\0')
	{
		#ifdef DEBUG
			fprintf(stderr,"%s no es nulo\n");
			fflush(stderr);
		#endif
	return 1;
	}
} 
#ifdef DEBUG
	fprintf(stderr,"%s is null string\n",a);
	fflush(stderr);
#endif
return 0;
}
