#include <stdio.h>
#include "libdbase.h"

/*
        Add a blank record at the end of database
*/

int append_blank(DATABASEDBF **asp)
{
FILE *a;
int u,restos;
int uno,dos,tres,cuatro = 0;
(**asp).recnos++;
cuatro = (**asp).recnos/16777216;
tres = ((**asp).recnos - (cuatro*16777216)) / 65536;
dos = ((**asp).recnos - (cuatro*16777216) - (tres*65536)) / 256;
uno = ((**asp).recnos - (cuatro*16777216) - (tres*65536) - (dos*256));

	if( (a = fopen((**asp).name,"r+")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"appends. Cant open file for adding blanks at end\n");
		fflush(stderr);
#endif
		return -1;
	}

rewind(a);
fseek(a,4,SEEK_SET);
fputc(uno,a);  /* This change the number of records in the database */
fputc(dos,a);
fputc(tres,a);
fputc(cuatro,a);

fseek(a,0,SEEK_END);

#ifdef DEBUG
	fprintf(stderr,"appends. Reccount updated.\n"); 
	fprintf(stderr,"appends. Adding %i blanks at end\n",(**asp).rec_len);
	fflush(stderr);
#endif

for(u = 0; u < (**asp).rec_len; ++u)
fputc(' ',a);
fflush(a);
fclose(a);
(**asp).current = (**asp).recnos;
return 0;
}

