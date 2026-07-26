/****************************************************
 *
 *
 *
 *
 * (C) Alvaro Cortes. 2004. accsc@arbornet.org
 *
 *
 * Under GPL licence. NO WARRANTY. Use UNDER YOUR OWN RISK.
 *
 *
 *
 *
 *
 *
 *
 ******************************************************/

#include <stdio.h>
#include <stdlib.h>
#include "libdbase.h"


/* Export DBF file as CSV file with spec separator */
/* This is a common function in other xBase programs */

int export_as_csv(DATABASEDBF *asp,char sep, char *_fname)
{
	FILE *out,*in;
	char *max;
	char *f2;
	int i,o = 0;

	if( (max = (char *) malloc(12)) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Error not enought memory\n");
		fflush(stderr);
#endif
		return -1;
		
	}
	if( ( f2= (char *) malloc(1025)) == NULL)
	{
		fprintf(stderr,"Error. Sin memoria.\n");
		fflush(stderr);
		free(max);
		return -1;
	}
	if( (out = fopen(_fname,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open output file\n");
		fflush(stderr);
#endif
		free(max);
		free(f2);
		return -1;
	}
	if( (in = fopen(asp->name,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open DB file\n");
		fflush(stderr);
#endif
		free(max);
		free(f2);
		return -1;
	}
	for( i = 1; i<= asp->camposn; ++i)
	{
		field_name(asp,i,&max);
		fprintf(out,"\"%s,%c,%i\"",max,asp->fields.tipos[i],asp->fields.longitudes[i]);
		if( i != asp->camposn)
			fprintf(out,"%c",sep);
		else
			fprintf(out,"\n");
		fflush(out);
	}
	for( o = asp->current; o <= asp->recnos; ++o)
	{
		for( i = 1; i<=asp->camposn; ++i)
		{
			get_field(asp,i,&f2);
			fprintf(out,"\"%s\"",f2);
			if( i != asp->camposn)
				fprintf(out,"%c",sep);
			else
				fprintf(out,"\n");
			fflush(stdout);
		}

		skip(&asp);
	}

	free(max);
	free(f2);
	fclose(out);
	fclose(in);
	return 0;
}


/* Export DBF file as SQL ASCII file */


int export_as_sql(DATABASEDBF *asp, char *_fname, int mode)
{
	FILE *out,*in;
	char *max;
	char *f2;
	int i,o = 0;
	
	if( (max = (char *) malloc(12)) == NULL)
	{
		fprintf(stderr,"Error. Sin memoria.\n");
		fflush(stderr);
		return -1;
	}
	if( (f2 = (char *) malloc(257)) == NULL)
	{	
		fprintf(stderr,"Error. Sin memoria.\n");
		fflush(stderr);
		free(max);
		return -1;
	}
	if( (out = fopen(_fname,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open output file\n");
		fflush(stderr);
#endif
		free(max);
		free(f2);
		return -1;
	}
	if( (in = fopen(asp->name,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open DB file\n");
		fflush(stderr);
#endif
		free(f2);
		free(max);
		return -1;
	}
	
	fprintf(out,"Table: %s\n---------------------*/\n",asp->name);
	if( mode != 1)
	fprintf(out,"CREATE TABLE \"dbf_table\" (\n");
	else
	fprintf(out,"CREATE TABLE dbf_table (\n");
	
	for( i = 1; i<= asp->camposn; ++i)
	{
		field_name(asp,i,&max);

	if ( mode != 1)
		fprintf(out,"\"%s\"",max);
	else
		fprintf(out,"%s",max);

		if( asp->fields.tipos[i] == 'C')
		fprintf(out," varchar(%i)",asp->fields.longitudes[i]);
		else if( asp->fields.tipos[i] == 'D')
		fprintf(out," date");
		else if( asp->fields.tipos[i] == 'L')
		fprintf(out," boolean");
		else if( asp->fields.tipos[i] == 'N')
		fprintf(out," integer");
		else if( asp->fields.tipos[i] == 'M')
		fprintf(out," text");
		
		if( i != asp->camposn)
			fprintf(out,",\n");
		else
			fprintf(out,"\n);\n\n");
		fflush(out);
	}
	for( o = asp->current; o <= asp->recnos; ++o)
	{
	if( mode == 1)
	fprintf(out,"INSERT INTO dbf_table (");
	else
	fprintf(out,"INSERT INTO \"dbf_table\" (");
	
	for( i = 1; i<= asp->camposn; ++i)
	{
		field_name(asp,i,&max);

	if( mode != 1)
		fprintf(out,"\"%s\"",max);
	else
		fprintf(out,"%s",max);
		
		if( i != asp->camposn)
			fprintf(out,",");
		else
			fprintf(out,") ");
		fflush(out);
	}
	fprintf(out,"VALUES (");
	
		for( i = 1; i<=asp->camposn; ++i)
		{

		get_field(asp,i, &f2);

			if( asp->fields.tipos[i] == 'C')
			fprintf(out,"'%s'",f2);
			else if ( asp->fields.tipos[i] == 'N')
			fprintf(out,"%s",f2);
			else if ( asp->fields.tipos[i] == 'D')
			fprintf(out,"'%s'",f2);
			else if( asp->fields.tipos[i] == 'L')
			fprintf(out,"'%s'",f2);
			else if( asp->fields.tipos[i] == 'M')
			fprintf(out,"'%s'",f2);
			else
			fprintf(out,"%s",f2);
						
			if( i != asp->camposn)
				fprintf(out,",");
			else
				fprintf(out,");\n");
			fflush(stdout);
		}

		skip(&asp);
	}

	free(f2);
	free(max);
	fclose(out);
	fclose(in);
	return 0;
}


int export_as_html(DATABASEDBF *asp, char *_fname)
{
	FILE *out,*in;
	char *max = (char *) malloc(12);
	char *f2 = (char *) malloc(257);
	char *otro = (char *) malloc( 200);
	char *dbt_name = (char *) malloc(1025);
	int i,o = 0;

	
	if( max == NULL || f2 == NULL || dbt_name == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Error not enought memory\n");
		fflush(stderr);
#endif
		return -1;
		
	}
	
	if( (out = fopen(_fname,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open output file\n");
		fflush(stderr);
#endif
		free(max);
		free(f2);
		free(dbt_name);
		return -1;
	}
	if( (in = fopen(asp->name,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open DB file\n");
		fflush(stderr);
#endif
		free(f2);
		free(dbt_name);
		free(max);
		return -1;
	}
	
	fprintf(out,"<HTML>\n<TITLE>Database %s HTMP dump</TITLE>",asp->name);
	fprintf(out,"Table: %s<BR>\n",asp->name);
/*	fprintf(out,"<TABLE BORDER=1 COLS=%i ROWS=%i>",asp->camposn+1,asp->recnos);*/
	fprintf(out,"<TABLE BORDER=1>");
	fprintf(out,"<TR>");
	fprintf(out,"<TD>Record</TD>");
	for( i = 1; i<= asp->camposn; ++i)
	{
		fprintf(out,"<TD>");
		field_name(asp,i,&f2);
		fprintf(out,"%s",f2);
		
		fprintf(out,"</TD>");
		fflush(out);
	}
	fprintf(out,"</TR>\n");
	
	for( o = asp->current; o <= asp->recnos; ++o)
	{
	fprintf(out,"<TR>\n");
	
		fprintf(out,"<TD>%i</TD>\n",o);
		for( i = 1; i<=asp->camposn; ++i)
		{
			get_field(asp,i,&dbt_name);
			fprintf(out,"<TD>%s</TD>\n",dbt_name);
			fflush(stdout);
		}
		fprintf(out,"</TR><BR>\n");
		fflush(stdout);
		skip(&asp);
	}
	fprintf(out,"</TABLE></HTML>");
	free(f2);
	free(dbt_name);
	free(max);
	fclose(out);
	fclose(in);
	return 0;
}

/* int export_as_xml(DATABASEDBF asp, char *_fname)
{
	FILE *out,*in;
	char *max = (char *) malloc(12);
	char *f2 = (char *) malloc(257);
	char *otro = (char *) malloc( 200);
	char *dbt_name = (char *) malloc(1025);
	int i,o = 0;

	
	if( max == NULL || f2 == NULL || dbt_name == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Error not enought memory\n");
		fflush(stderr);
#endif
		return -1;
		
	}
	
	if( (out = fopen(_fname,"wb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open output file\n");
		fflush(stderr);
#endif
		free(max);
		free(f2);
		free(dbt_name);
		return -1;
	}
	if( (in = fopen(asp.name,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"EXPORT. Cant open DB file\n");
		fflush(stderr);
#endif
		free(f2);
		free(dbt_name);
		free(max);
		return -1;
	}
	
	fprintf(out,"<XML>\nDatabase %s HTMP dump</TITLE>",asp.name);
	fprintf(out,"Table: %s<BR>\n",asp.name);
	fprintf(out,"<TABLE BORDER=1 COLS=%i ROWS=%i>",asp.camposn+1,asp.recnos);
	fprintf(out,"<TR>");
	fprintf(out,"<TD>Record</TD>");
	for( i = 1; i<= asp.camposn; ++i)
	{
		fprintf(out,"<TD>");
		max = field_name(asp,i);
		fprintf(out,"%s",max);
		
		fprintf(out,"</TD>");
		fflush(out);
	}
	fprintf(out,"</TR>\n");
	
	for( o = asp.current; o <= asp.recnos; ++o)
	{
	fprintf(out,"<TR>\n");
	
		fprintf(out,"<TD>%i</TD>\n",o);
		for( i = 1; i<=asp.camposn; ++i)
		{
			fprintf(out,"<TD>%s</TD>\n",get_field(asp,i));
			fflush(stdout);
		}
		fprintf(out,"</TR><BR>\n");
		fflush(stdout);
		asp = skip(asp);
	}
	fprintf(out,"</TABLE></HTML>");
	free(f2);
	free(dbt_name);
	free(max);
	fclose(out);
	fclose(in);
	return 0;
}
*/
