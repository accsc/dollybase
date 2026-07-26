#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "libdbase.h"

/* Portable fallback for strcasestr (GNU-specific) */
#ifndef HAVE_STRCASESTR
static char *my_strcasestr(const char *haystack, const char *needle)
{
    size_t hlen = strlen(haystack);
    size_t nlen = strlen(needle);
    if (nlen > hlen)
        return NULL;
    for (const char *p = haystack; p <= haystack + hlen - nlen; p++)
    {
        size_t i;
        for (i = 0; i < nlen; i++)
        {
            if (tolower((unsigned char)p[i]) != tolower((unsigned char)needle[i]))
                break;
        }
        if (i == nlen)
            return (char *)p;
    }
    return NULL;
}
#define strcasestr my_strcasestr
#endif /* HAVE_STRCASESTR */

void locate(DATABASEDBF **asp, int y,char *condicion)
{
int o = 0;
char *dos;
DATABASEDBF *bsp;
if (( dos = (char *)malloc(1025)) == NULL)
{
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
	return;
}

bsp = *asp;
bsp->locate_is=0;
if( bsp->current == 0)
gotos(&bsp, 1);

for( o=0; o< bsp->recnos; ++o)
{
get_field(bsp,y,&dos);
/*printf("Intento encontrar %s en %s.\n",condicion,dos);*/

if( strcasestr(dos,condicion))
{
bsp->locate_is=1;
bsp->located_campo = y;
bsp->located=condicion;
memcpy(*asp,bsp, sizeof(DATABASEDBF));
free(dos);
return;
}

skip(&bsp);
}
free(dos);
return;
}

int seek()
{
return VERITAS;
}

int found(DATABASEDBF *asp)
{
	if( asp->locate_is == 1)
	{
	return VERITAS;
	}
	return FALSO;
}


/* type = 0 for NDX ----- type = 1 for NTX */

FOUND seek_index(NTX *ind, char *criteria, int type)
{
	FOUND fin;
	int i;
		for(i = 0; i<ind->total_pages; ++i)
		{
			if ( type == 0)
				fin = search_ndx_next(ind,criteria,i,0);
			else if( type == 1)
				fin = search_ntx_next(ind,criteria,i,0);

			if( fin.recno != 0)
				return fin;
				
		}
return fin;
}
/*DATABASEDBF skip_index(DATABASEDBF asp)
{
	FOUND a;
	int i;
		for( i = 0; i<asp.index_node.total_pages; ++i)
		{
			a = search_ndx_next(asp.index_node,NULL,i,0);
		}
	if( asp.index_node.type != 0)
	{
		asp.current = asp.current +1;
		return asp;
	}
	a = seek_index(asp.index_node,NULL,1);
	if( a.recno <= asp.recnos)
		asp.current = a.recno;
	printf("%i - %i\n",a.pos,a.recno);
	fflush(stdout);
	getchar();
	return asp;
}*/



int continues(DATABASEDBF **asp)
{
int o = 0;
char *dos;
DATABASEDBF *bsp;
if (( dos = (char *)malloc(1025)) == NULL)
{
	fprintf(stderr,"Error. Sin memoria.\n");
	fflush(stderr);
	return -1;
}
bsp = *asp;
bsp->locate_is=0;
if( bsp->current == 0)
gotos(&bsp, 1);

for( o=0; o< bsp->recnos; ++o)
{
get_field(bsp,bsp->located_campo,&dos);
/*printf("Intento encontrar %s en %s.\n",condicion,dos);*/

if( strcasestr(dos,bsp->located))
{
bsp->locate_is=1;
memcpy(*asp,bsp, sizeof(DATABASEDBF));
free(dos);
return 0;
}

skip(&bsp);
}
free(dos);
return 1;
}
