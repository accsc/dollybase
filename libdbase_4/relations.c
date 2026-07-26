/**************************************************************



	(C) 2005, Alvaro Cortés.  accsc@arbornet.org


	Module for virtual links for two databases.

	This module Handle by one temp file links for records
	of two databases with one field in common.

	Very primitive only one link per record and strange file format.
	It Works...

************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "libdbase.h"

void  create_relation(char *name, DATABASEDBF *asp, char *field, char *second, char *second_field, DBLINK **linko)
{
DATABASEDBF *se;
int se_n, asp_n;
int i;
DBLINK *a;
char *f1;

if( ( a = (DBLINK *) malloc ( sizeof(DBLINK))) == NULL)
{
#ifdef DEBUG
fprintf(stderr,"Error. Not enought memory.\n");
fflush(stderr);
#endif
return;
}

#ifdef DEBUG
fprintf(stderr,"New link: %s->%s <-> %s->%s\n",asp->name,field,second,second_field);
#endif

if( access(second,R_OK) != 0)
{
	fprintf(stderr,"DBLinks Error: Can not open second database.\n");	
	fflush(stderr);
	a->link_f = NULL;
	memcpy(*linko,a, sizeof(DBLINK));
	free(a);
	return;
}
if( (se = (DATABASEDBF *) malloc( sizeof(DATABASEDBF)) ) == NULL)
{
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
	a->link_f = NULL;
	memcpy(*linko, a, sizeof(DBLINK));
	free(a);
	return;
}

use(second,&se);

if( se->tipo == 0)
{
	fprintf(stderr,"DBLinks Error: Second database is not a dBase file.\n");
	fflush(stderr);
	a->link_f = NULL;	
	memcpy(*linko,a,sizeof(DBLINK));
	free(a);
	free(se);
	return;
}
se_n = field_to_number(se,second_field);
asp_n = field_to_number(asp,field);
if( se_n <= 0)
{
	fprintf(stderr,"DBLinks Error: Field '%s' not exists in database %s\n",second_field,second);
	fflush(stderr);
	a->link_f = NULL;
	memcpy(*linko, a, sizeof(DBLINK));
	free(a);
	free(se);
	return;
}
if( asp_n <= 0)
{
	fprintf(stderr,"DBLinks Error: Field '%s' not exists in database '%s'\n",field,asp->name);
	fflush(stderr);
	a->link_f = NULL;
	memcpy(*linko, a, sizeof(DBLINK));
	free(a);
	free(se);
	return;
}

/*if ( (a = tmpfile()) == NULL)*/
if( (a->link_f = fopen(name,"wb")) == NULL)
{
	fprintf(stderr,"DBLinks Error: Can not open DBLinks temp file\n");
	fflush(stderr);
	a->link_f = NULL;
	memcpy(*linko,a, sizeof(DBLINK));
	free(a);
	free(se);
	return;
}
fputc(0x7,a->link_f);
fputc(0x3,a->link_f);
fprintf(a->link_f,"%s",asp->name);
fputc(0x3,a->link_f);
fprintf(a->link_f,field);
fputc(0x3,a->link_f);
fprintf(a->link_f,"%s",se->name);
fputc(0x3,a->link_f);
fprintf(a->link_f,"%s",second_field);
fputc(0x3,a->link_f);
fputc(0x7,a->link_f);
asp->current = 1;

	if( (f1 = (char *) malloc (2048)) == NULL)
	{
		fprintf(stderr,"Error. Sin memoria.\n");
		fflush(stderr);
		a->link_f=NULL;
		memcpy( *linko, a, sizeof(DBLINK));
		free(a);
		free(se);
		return;
	}
printf("Prehaciendo Links.\n");
fflush(stdout);
/* Creating Links ...*/
for( i = 1; i <= asp->recnos; ++i)
{
	se->current = 1;
	get_field(asp,asp_n,&f1);
#ifdef DEBUG
	fprintf(stderr,"Contenido de campo en base1: %s.\n",f1);
	fflush(stderr);
#endif
#ifdef DEBUG
	fprintf(stderr,"Buscando %s en base2 campo: %s.\n", f1, second_field);
	fflush(stderr);
#endif
	locate(&se, se_n, f1);
	fputc(asp->current,a->link_f);
	fputc(0x3,a->link_f);
	asp->current++;
	if( se->locate_is == 1)
	{
	fputc(se->current,a->link_f);
	}else{
	fputc(0x00,a->link_f);
	}
}
#ifdef DEBUG
fprintf(stderr,"DBLINK finalizado.\n");
fflush(stderr);
#endif
fputc(0x3,a->link_f);
fputc(0x7,a->link_f);

if( strlen(name) > 1024)
strncpy(a->name,name,1023);
else
strcpy(a->name,name);

#ifdef DEBUG
fprintf(stderr,"Cerrando fichero...\n");
fflush(stderr);
#endif
fclose(a->link_f);
#ifdef DEBUG
fprintf(stderr,"Liberando memoria...\n");
fflush(stderr);
#endif
free(se);
free(f1);
#ifdef DEBUG
fprintf(stderr,"Transfiriendo copia de DBLINK a posicion de memoria parametro.\n");
fflush(stderr);
#endif
memcpy(*linko,a,sizeof(DBLINK));
#ifdef DEBUG
fprintf(stderr,"Liberando parametro copia.\n");
fflush(stderr);
#endif
free(a);
return;
}


/* Get linked rec for one rec in origin database */
int get_relation_record(DBLINK *a, int rec)
{
int c,i, orec, drec, nfield;
char campo[15];
char campo2[15];
char name1[1024];
char name2[1024];

if( a == NULL)
return 0;

if ( (a->link_f = fopen(a->name,"rb")) == NULL)
{
	fprintf(stderr,"DBLinks Error: Can not open Links file.\n");
	fflush(stderr);
	return 0;
}
fseek(a->link_f,2,SEEK_SET);
for(i = 0; i<= 1023; i++)
{
	if( (c = fgetc(a->link_f)) == 0x3)
	{
	name1[i] = '\0';
	break;
	}else{
	name1[i] = c;
	}
}
for(i = 0; i<= 10; i++)
{
	if( (c = fgetc(a->link_f)) == 0x3)
	{
		campo[i] = '\0';
		break;
	}else{
	campo[i] = c;
	}
}
for(i = 0; i<= 1023; i++)
{
	if( (c = fgetc(a->link_f)) == 0x3)
	{
	name2[i] = '\0';
	break;
	}else{
	name2[i] = c;
	}
}
for( i = 0; i<=10; ++i)
{
	if( ( c=fgetc(a->link_f)) == 0x3)
	{
		campo2[i] = '\0';
		break;
	}else{ campo2[i] = c; }
}

fseek(a->link_f,1,SEEK_CUR);
fseek(a->link_f, (rec-1)*3, SEEK_CUR);
fseek(a->link_f,1,SEEK_CUR);
drec = (int) fgetc(a->link_f);
fclose(a->link_f);
return drec;
}


/* Get a field of the corresponding record in linked database */
void get_relation_field(DBLINK *a,char *field, int rec, char **vale)
{
DATABASEDBF *se;
int c,i, orec, drec, nfield;
char campo[15];
char campo2[15];
char name1[1024];
char name2[1024];
char *f1;
if( (se = (DATABASEDBF *) malloc ( sizeof(DATABASEDBF))) == NULL)
{
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
	*vale = NULL;
	return;
}


if ( (a->link_f = fopen(a->name,"rb")) == NULL)
{
	fprintf(stderr,"DBLinks Error: Can not open Links file.\n");
	fflush(stderr);
	free(se);
	*vale = NULL;
	return;
}
fseek(a->link_f,2,SEEK_SET);
for(i = 0; i<= 1023; i++)
{
	if( (c = fgetc(a->link_f)) == 0x3)
	{
	name1[i] = '\0';
	break;
	}else{
	name1[i] = c;
	}
}
for(i = 0; i<= 10; i++)
{
	if( (c = fgetc(a->link_f)) == 0x3)
	{
		campo[i] = '\0';
		break;
	}else{
	campo[i] = c;
	}
}
for(i = 0; i<= 1023; i++)
{
	if( (c = fgetc(a->link_f)) == 0x3)
	{
	name2[i] = '\0';
	break;
	}else{
	name2[i] = c;
	}
}
for( i = 0; i<=10; i++)
{
	if( (c=fgetc(a->link_f)) == 0x3)
	{
		campo2[i] = '\0';
		break;
	}else{ campo2[i] = c; }
}
use(name2,&se);
fseek(a->link_f,1,SEEK_CUR);
fseek(a->link_f, (rec-1)*3, SEEK_CUR);
if (se->tipo == 0)
{
	fprintf(stderr,"DBLink Error: Can not open linked database.\n");
	fflush(stderr);
	free(se);
	*vale = NULL;
	return;
}
orec = (int) fgetc(a->link_f);
fseek(a->link_f,1,SEEK_CUR);
drec = (int) fgetc(a->link_f);
se->current = drec;

if ( (nfield = field_to_number(se,field)) == 0)
{
	fprintf(stderr,"DBLink Error: Can not find field in linked database.\n");
	fflush(stderr);
	free(se);
	*vale = NULL;
	return;
}

fclose(a->link_f);
if( drec== 0)
{
	free(se);
	*vale = NULL;
	return;
}
else{
if( (f1 = (char *) malloc(2048)) == NULL)
{
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
	free(se);
	*vale = NULL;
	return;
}
get_field(se,nfield,&f1);
free(se);
memcpy(*vale,f1, strlen(f1));
free(f1);
return;
}

}
