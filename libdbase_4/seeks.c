#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
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
int o;
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

/* ------------------------------------------------------------------ */
/* B-tree descent for NDX (dBase III+ native index, 512-byte pages)    */
/* Returns FOUND with recno set to the matching DBF record, or 0.      */
/* ------------------------------------------------------------------ */

FOUND seek_ndx_btree(NDX *ind, char *criteria)
{
    FILE *fp = NULL;
    char *page = NULL;
    char *keybuf = NULL;
    FOUND fin;
    int page_size = 512;
    int page_num = ind->root_page;

    memset(&fin, 0, sizeof(fin));
    if (!ind || !criteria || ind->root_page == 0)
        return fin;

    fp = fopen(ind->fname, "rb");
    if (!fp) return fin;

    page = malloc(page_size);
    keybuf = malloc(ind->key_len + 1);
    if (!page || !keybuf) {
        free(page); free(keybuf); fclose(fp); return fin;
    }

    /* Walk down the B-tree from root to leaf */
    while (1) {
        fseek(fp, page_num * page_size, SEEK_SET);
        fread(page, 1, page_size, fp);

        /* First 4 bytes: number of entries in this node */
        int nentry = page[0] + (page[1] << 8) + (page[2] << 16) + (page[3] << 24);
        if (nentry <= 0) break;

        int found_entry = -1;

        for (int i = 0; i < nentry; i++) {
            int base = 4 + i * (ind->key_len + 8);

            /* Bytes 0-3: leaf node pointer (child page for interior) */
            int leafnode = page[base] + (page[base+1] << 8) +
                           (page[base+2] << 16) + (page[base+3] << 24);
            /* Bytes 4-7: DBF record number */
            int dbfrec = page[base+4] + (page[base+5] << 8) +
                         (page[base+6] << 16) + (page[base+7] << 24);
            /* Bytes 8+: key value */
            int klen = ind->key_len;
            if (base + 8 + klen > page_size) klen = page_size - base - 8;
            memcpy(keybuf, page + base + 8, klen);
            keybuf[klen] = '\0';

            /* Strip trailing spaces from key for comparison */
            while (klen > 0 && keybuf[klen - 1] == ' ')
                keybuf[--klen] = '\0';

            int cmp = strncasecmp(keybuf, criteria, (size_t)klen > strlen(criteria) ? strlen(criteria) : klen);

            if (cmp == 0) {
                /* Exact match */
                if (dbfrec != 0) {
                    /* Leaf entry — we found the record */
                    fin.recno = dbfrec;
                    fin.page = page_num;
                    fin.pos = i;
                    fin.interior = 0;
                    free(page); free(keybuf); fclose(fp);
                    return fin;
                } else if (leafnode != 0) {
                    /* Interior entry — descend to child */
                    page_num = leafnode;
                    found_entry = i;
                    break;
                }
            } else if (cmp < 0) {
                /* Key < criteria — criteria could be in this child's range */
                /* Remember this entry as a candidate and keep looking */
                if (leafnode != 0 && dbfrec == 0) {
                    page_num = leafnode;
                    found_entry = i;
                    /* Don't break — keep looking for a better match */
                } else if (dbfrec != 0) {
                    /* Leaf entry with key < criteria — keep scanning */
                    found_entry = -2; /* marker: still scanning leaf */
                }
            } else {
                /* Key > criteria — stop; last candidate is our target */
                break;
            }
        }

        /* If we scanned all entries and criteria > last key,
           use the last child (found_entry should already be set) */
        if (found_entry == -2) {
            /* We were scanning a leaf and criteria exceeded all keys */
            found_entry = -1;
        }

        if (found_entry == -1) {
            /* No match found in this branch */
            break;
        }
    }

    free(page); free(keybuf); fclose(fp);
    return fin;
}

/* ------------------------------------------------------------------ */
/* B-tree descent for NTX (Clipper index, 1024-byte pages)             */
/* ------------------------------------------------------------------ */

FOUND seek_ntx_btree(NTX *ind, char *criteria)
{
    FILE *fp = NULL;
    char *page = NULL;
    char *keybuf = NULL;
    FOUND fin;
    int page_size = 1024;
    int page_num = ind->root_page;

    memset(&fin, 0, sizeof(fin));
    if (!ind || !criteria || ind->root_page == 0)
        return fin;

    fp = fopen(ind->fname, "rb");
    if (!fp) return fin;

    page = malloc(page_size);
    keybuf = malloc(ind->key_len + 1);
    if (!page || !keybuf) {
        free(page); free(keybuf); fclose(fp); return fin;
    }

    while (1) {
        fseek(fp, page_num * page_size, SEEK_SET);
        fread(page, 1, page_size, fp);

        /* First 2 bytes: number of keys in this node */
        int nkeys = page[0] + (page[1] << 8);
        if (nkeys <= 0) break;

        int found_entry = -1;

        for (int i = 0; i < nkeys; i++) {
            /* NTX layout: 8-byte child pointer + 4-byte recno + key_len bytes */
            int base = 2 + (2 * nkeys) + i * (ind->key_len + 8);
            if (base + ind->key_len + 8 > page_size) break;

            /* Bytes 0-3: child page pointer */
            int child_pg = page[base] + (page[base+1] << 8) +
                           (page[base+2] << 16) + (page[base+3] << 24);
            /* Bytes 4-7: DBF record number */
            int dbfrec = page[base+4] + (page[base+5] << 8) +
                         (page[base+6] << 16) + (page[base+7] << 24);
            /* Bytes 8+: key value */
            int klen = ind->key_len;
            if (base + 8 + klen > page_size) klen = page_size - base - 8;
            memcpy(keybuf, page + base + 8, klen);
            keybuf[klen] = '\0';

            /* Strip trailing spaces from key for comparison */
            while (klen > 0 && keybuf[klen - 1] == ' ')
                keybuf[--klen] = '\0';

            int cmp = strncasecmp(keybuf, criteria, (size_t)klen > strlen(criteria) ? strlen(criteria) : klen);

            if (cmp == 0) {
                if (dbfrec != 0) {
                    fin.recno = dbfrec;
                    fin.page = page_num;
                    fin.pos = i;
                    fin.interior = 0;
                    free(page); free(keybuf); fclose(fp);
                    return fin;
                } else if (child_pg != 0) {
                    page_num = child_pg;
                    found_entry = i;
                    break;
                }
            } else if (cmp < 0) {
                if (child_pg != 0 && dbfrec == 0) {
                    page_num = child_pg;
                    found_entry = i;
                    break;
                }
            } else {
                break;
            }
        }

        if (found_entry == -1)
            break;
    }

    free(page); free(keybuf); fclose(fp);
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
int o;
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
