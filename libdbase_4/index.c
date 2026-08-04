/****************************************************************
 *
 *
 *	(C) Alvaro Cortés. 2004. accsc@arbornet.org
 *
 *	index.c . Module for handle dbase/clipper index files (NTX/NDX)
 *
 *	Under GPL v2 or above. 
 *      NO WARRANTIY. USE UNDER YOUR OWN RISK.
 *     
 *
 ***************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "libdbase.h"

int search(char *str, char *cri, int mode)
{
        int ssize,csize,i;
        ssize = strlen(str);
        csize = strlen(cri);
        if( mode > 3 || mode <0)
        return -1;
        if( ssize < csize)
        return -1;
        if( csize <= 0 || ssize <= 0)
        return -1;

        for( i = 0; i<= (ssize-csize); ++i)
        {
                if( mode == 0)
                {
                        if( strncmp( str+i,cri,csize) == 0 )
                        return 0;
                }else if( mode == 1){
                        if( strncasecmp( str+i,cri,csize) == 0)
                                return 0;
                }else if( mode == 2){
                        if( strncmp( str+i,cri,csize) == 0  && ( *(str+i+csize) == ' ' || *(str+i+csize) == '\0' || *(str+i+csize) == '\t' || *(str+i+csize) == '\n') )
                        return 0;
                }else if( mode == 3){
                        if( strncasecmp( str+i,cri,csize) == 0 && ( *(str+i+csize) == ' ' || *(str+i+csize) == '\0' || *(str+i+csize) == '\t' || *(str+i+csize) == '\n') )
                        return 0;
                }

        }
return -1;
}


/********************************************************************
*  								    *
* The anchor node of the index file. Its better in memory... faster *
* Only the essential information is saved in memory 		    *
*								    *
********************************************************************/


NTX use_ntx(char *_fname)
{
FILE *indic;
char _head[1024];
int i;
NTX ind;
struct stat st1;
char *head = (char *) malloc(sizeof( _head));

ind.type = 0;
if( head == NULL) 
{
#ifdef DEBUG
	fprintf(stderr,"NTX: Error, not enought memory\n");
	fflush(stderr);
#endif 
	
	return ind;
}

if( ( indic = fopen(_fname,"rb")) == NULL)
{
#ifdef DEBUG
fprintf(stderr,"NTX: Cant open file %s\n",_fname);
fflush(stderr);
#endif
return ind;
}
fread(head,1,1023,indic);

/* Reformar esto */
/* strncpy(ind.fname,_fname,strlen(_fname)); */
/*strcpy(ind.fname,_fname);*/

stat(_fname,&st1);
ind.total_pages = (st1.st_size / 512);

ind.fname = _fname;
ind.compiler_type = head[0] + head[1];
#ifdef DEBUG
if( ind.compiler_version == 0x03)
fprintf(stderr,"NTX: File made by Clipper Autum 87\n");
else if( ind.compiler_version == 0x06)
fprintf(stderr,"NTX: File made by Clipper 5.x\n");
fflush(stderr);
#endif
ind.compiler_version = head[2] + head[3];
ind.root_page = head[4] + (head[5]*256) + (head[6]*65536) + (head[7]*16777216);
ind.next_page = head[8] + (head[9]*256) + (head[10]*65536) + (head[11]*16777216);

ind.type = 1; /* NTX */
#ifdef DEBUG
if ( (head[12] + (head[13]*256) -8) != (head[14] + (head[15]*256)))
fprintf(stderr,
"NTX: Warning Key lenght mistmach. Maybe corrupted file!. Continue...\n");

fflush(stderr);
#endif

ind.key_len = head[14] + (head[15]*256);
ind.key_dec = head[16] + (head[17]*256);
ind.max_keys_per_page = head[18] + (head[19]*256);
ind.min_keys_per_page = head[20] + (head[21]*256);
for( i = 0; i<=256; ++i)
{
	if( head[i+22] != 0)
	ind.field_name[i] = head[i+22];
	else
	ind.field_name[i] = 0;
}

ind.unique = head[278];
ind.pos = 1;
ind.node = 1;
free(head);
fclose(indic);
return ind;
}

void display_ntx_info(NTX *ind)
{
	if( ind->type == 0)
	{
	#ifdef DEBUG
		fprintf(stderr,"Error. NTX not valid\n");
		fflush(stderr);
	#endif
	return;
	}
printf("NTX file summary\n");
printf("File name: %s\n",ind->fname);
printf("Key Name: %s\n",ind->field_name);
printf("Key size: %i\n",ind->key_len);
printf("Key decimals: %i\n",ind->key_dec);
printf("Unique: %i\n",ind->unique);
printf("Max/min per page: %i/%i\n",ind->max_keys_per_page,ind->min_keys_per_page);
printf("Estimated number of nodes: %i\n",ind->total_pages);
printf("Root page/Next Page: %i/%i\n",ind->root_page,ind->next_page);
if( ind->compiler_type == 0x03)
printf("Compiler Clipper Autum 87 - Version %i\n",ind->compiler_version);
else if( ind->compiler_type == 0x06)
printf("Compiler Clipper 5.x - Version %i\n",ind->compiler_version);
else if( ind->compiler_type == 0x00 && ind->compiler_version == 0x03)
printf("Compiler Default, No version\n");
fflush(stdout);
return;
}

/**********************************************************************
*
*  search in ntx to find something.
*  criteria -> text to find.
*  last_page -> next node.
*  last_pos -> last pos of record that match. 0 for the first time.
*  return FOUND.pos -> position in index file.
*  return FOUND.page -> page of index file.
*  return FOUND.recno -> Rec number
**********************************************************************/

FOUND search_ntx_next(NTX *ind, char *criteria, int last_page, int last_pos)
{
	char _page[1025];
	FILE *indic = NULL;
	int recno;
	FOUND results;
	int i,op,o,r,pos; /* counters */
	char *page = (char *) malloc (sizeof(_page));
	char *head_node;
	char *rec_conx = (char *)malloc(ind->key_len);
	
	results.page = last_page;
	results.recno = 0;
	results.pos = -1;

	if( page == NULL || rec_conx == NULL) 
	{
#ifdef DEBUG
		fprintf(stderr,"NTX: Error, not enought memory\n");
		fflush(stderr);
#endif
		free(page);
		free(rec_conx);
		return results;
	}
	if ( (indic = fopen(ind->fname,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"NTX: Cant open index file\n");
		fflush(stderr);
#endif
                free(page);
		free(rec_conx);
		return results;
	}
	fseek(indic,(last_page)*1024,SEEK_SET);
	fread(page,1,1024,indic);
	fclose(indic);
	o = page[0] + (page[1]*256);
	
	if( o <= 0)
	{
#ifdef DEBUG
		fprintf(stderr,"NTX: Critical error i dont know where i am!\n");
		fprintf(stderr,"Frees and children first!\n");
		fflush(stderr);
#endif
	free(page);
	free(rec_conx);
	fflush(stdout);
	return results;
	}
		
		
	ind->current_keys_node = o;
#ifdef DEBUG
	fprintf(stderr,"Number of keys in this node: %i\n",o);
	fflush(stderr);
	
	if( o > ind->max_keys_per_page)
	{
		fprintf(stderr,"NTX: Keys of the node are more than the specified keys per node in head\n");
		fprintf(stderr,"NTX: Corrupt file?!!!\n");
		fflush(stderr);
	}
	if( o < ind->min_keys_per_page)
	{
		fprintf(stderr,"NTX: Probably last node with recs.\n");
		fflush(stderr);
	}

#endif
	head_node = (char *) malloc((ind->max_keys_per_page*2)+1);
	if( head_node == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"NTX: Error not enought memory\n");
		fflush(stderr);
#endif
                free(page);
                free(rec_conx);
		return results;
	}
	for( i = 0; i< (ind->max_keys_per_page*2); ++i)
	{	
	head_node[i] = page[2+i];
	}
	head_node[i+1] = 0;
#ifdef DEBUG
	fprintf(stderr,"Key offsets array saved in memory.\n");
	fflush(stderr);
#endif
	if( last_pos < 0 || last_pos > o)
	{
#ifdef DEBUG
		fprintf(stderr,"NTX: Rec position out of range.\n");
		fflush(stderr);
#endif
		free(page);
		free(head_node);
		free(rec_conx);
		return results;
	}
	ind->node = last_page;
	for( r= last_pos; r<o; ++r)
	{ 
	ind->pos = r;
	pos = i + (r*(ind->key_len+8));
	
#ifdef DEBUG
fprintf(stderr,"Next leaf node: %i\n",page[1+pos],(256*page[2+pos]),(65536*page[3+pos]),(16777216*page[4+pos]));
fflush(stderr);
#endif

recno=(16777216*page[5+pos])+(65536*page[pos+6])+(256*page[pos+7])+page[pos+8];

#ifdef DEBUG
fprintf(stderr,"Rec no: %i\n",recno);
fflush(stderr);
#endif
	pos = pos+3;
	for( op= 0; op< (ind->key_len); ++op)
	{
	rec_conx[op] = page[pos+9+op];
	/*fprintf(stderr,"%c",page[pos+9+op]);*/
	}
	rec_conx[op+1] = 0;
#ifdef DEBUG
	fprintf(stderr,"Record: %s\n",rec_conx);
	fflush(stderr);
#endif

	/* This function is in dollybase main program */
	/* TODO: move the function here */
	if( criteria == NULL) /* Request for next rec in a indexed db */
	{
		printf("Me piden el siguiente\n");
		fflush(stdout);
		free(page);
		free(head_node);
		free(rec_conx);
		results.page = last_page;
		results.recno = recno;
		results.pos = r;
		return results;
	}
	
	if( search(rec_conx,criteria,1) == 0)
	{
	free(page);
	free(head_node);
	free(rec_conx);
	results.page = last_page;
	results.recno=recno;
	results.pos = r;
	return results;
	}

	}

	free(page);
	free(head_node);

	free(rec_conx);
return results;
}


/* Next 4 multiple */
int ajust_key_per4(int key_size)
{
return ((key_size + 3) / 4) * 4;
}


NDX * use_ndx( char *_fname)
{
	NTX *ind = NULL;
	char _head[512];
	char *head = (char*) malloc(sizeof(_head));
	FILE *indic;
	int i;

	if( head == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"NDX: Error, not enought memory\n");
		fflush(stderr);
#endif
		return ind;
	}

        ind = (NTX *) calloc(sizeof(NTX), 1);

        if( ind == NULL)
        {
#ifdef DEBUG
                fprintf(stderr,"NDX: Error, not enought memory\n");
                fflush(stderr);
#endif
                free(head);
                return ind;
        }

	
	if( (indic = fopen(_fname,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"NDX: Error, Cant open the index file\n");
		fflush(stderr);
#endif
                free(head);
                free(ind);
                ind = NULL;
		return ind;
	}
	fread(head,1,511,indic);
	
	ind->fname=_fname;
	ind->root_page=head[0]+(head[1]*256)+(head[2]*65536)+(head[3]*16777216);
	ind->total_pages=head[4]+(head[5]*256)+(head[6]*65536)+(head[7]*16777216);
	ind->key_len = head[12]+(head[13]*256);
	ind->max_keys_per_page=head[14]+(head[15]*256);
	ind->key_type = head[16]+head[17];
	ind->unique = head[23];

	for( i = 0; i<=256; ++i)
	{
		if( head[24+i] == 0)
		break;
		ind->field_name[i] = head[24+i];
	}
	ind->field_name[i] = '\0';
	free(head);
	fclose(indic);
	return ind;
}

void display_ndx_info(NDX *ind)
{
	printf("---- NDX index file summary ----\n");
	printf("Root Page: %i\n",ind->root_page);
	printf("Next Page: %i\n",ind->next_page);
	printf("Total Pages: %i\n",ind->total_pages);
	printf("Key size: %i\n",ind->key_len);
	printf("Key Type: %i\n",ind->key_type);
	printf("Keys per page: %i\n",ind->max_keys_per_page);
	printf("Field: %s\n",ind->field_name);
	printf("Unique: %i\n",ind->unique);
	printf("---- End of NDX summary for debug propouses ----\n");
	fflush(stdout);

}

FOUND search_ndx_next(NTX *ind, char *criteria, int last_page, int last_pos)
{
	FILE *indic;
	char _head[513];
	char *head = (char *) malloc( sizeof( _head));
	char *rec_conx = (char *) malloc( ind->key_len+1);

	FOUND fin;
	unsigned long nentry;
	long i, dbfrec, leafnode,o;

	fin.pos = 0;
	fin.recno = 0;
	fin.page = 0;

	if( head == NULL || rec_conx == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"NDX: Not enoguth memory\n");
		fflush(stderr);
#endif
                free(head);
                free(rec_conx);
		return fin;
	}
	if( (indic = fopen(ind->fname,"rb")) == NULL)
	{
#ifdef DEBUG
		fprintf(stderr,"NDX: Cant open index file\n");
		fflush(stderr);
#endif
		free(head);
		free(rec_conx);
		return fin;
	}

	fseek(indic,last_page*512,SEEK_SET);
 	fread(head,1,512,indic);
#ifdef DEBUG
	fprintf(stderr,"First node now in memory!\n");
	fflush(stderr);
#endif
	nentry = head[0] + (256* head[1]) + (65536*head[2]) + (16777216*head[3]);
#ifdef DEBUG
	fprintf(stderr,"Next entry: %i\n",nentry);
	fflush(stderr);
	getchar();
#endif
	for( i = 0; i< nentry; ++i)
	{
	leafnode = head[i*(ind->key_len+8)+4] + (head[i*(ind->key_len+8)+5]*256) + (head[i*(ind->key_len+8)+6]*65536) + (16777216*head[i*(ind->key_len+8)+7]);
	dbfrec = head[i*(ind->key_len+8)+8] + (256*head[i*(ind->key_len+8)+9]) + (65536*head[i*(ind->key_len+8)+10]) + (16777216*head[i*(ind->key_len+8)+11]);
		
		/* Interior node, not direcly to database record */
		if( leafnode != 0 && dbfrec == 0)  
		fin.interior = 1; /* Yes */
		else 
		fin.interior = 0; /* No */
		
		for( o = 0; o< ind->key_len; ++o)
		rec_conx[o] = head[i*(ind->key_len+8)+12+o];
		rec_conx[o+1] = '\0';
#ifdef DEBUG
		fprintf(stderr,"Leaf/Interior Node info:\n");
		fprintf(stderr,"%s - %i\n",rec_conx,dbfrec); 
		fflush(stderr);
#endif 
		if( criteria == NULL)
		{
			fin.pos = i;
			fin.page = last_page;
			if( leafnode != 0 && dbfrec == 0)
				fin.recno = leafnode;
			else
				fin.recno = dbfrec;
			free(head);
			free(rec_conx);
			fclose(indic);
			return fin;
		}
		if( search(rec_conx,criteria,1) == 0)
		{
			fin.pos = i;
			fin.page = last_page;
			if( leafnode != 0 && dbfrec == 0)
			fin.recno = leafnode;
			else
			fin.recno = dbfrec;
			free(head);
			free(rec_conx);
			fclose(indic);
			return fin;
		}
	}
	
	

	free(head);
	free(rec_conx);
	fclose(indic);
	return fin;
}

/* ==========================================================================
 * Generic NDX creation from pre-computed (key, recno) pairs.
 * The caller provides sorted keys (case-insensitive sort assumed).
 * Keys are NOT freed by this function — caller retains ownership.
 * key_len is the maximum key length (used for padding).
 * ========================================================================== */

int create_index_ndx_generic(char **keys, int *recnos, int count,
                              int key_len, const char *field_name,
                              const char *_fname)
{
    if (!keys || !recnos || count <= 0 || !_fname)
        return -1;

    if (key_len <= 0 || key_len > 256)
        key_len = 50;

    int entry_size = key_len + 8;
    int entries_per_page = (512 - 4) / entry_size;
    if (entries_per_page < 1) entries_per_page = 1;

    int num_leaves = (count + entries_per_page - 1) / entries_per_page;
    int root_page = num_leaves + 1;
    int total_pages = root_page + 1;

    FILE *f = fopen(_fname, "wb");
    if (!f) return -1;

    /* --- Page 0: header --- */
    {
        char header[512];
        memset(header, 0, 512);
        header[0] = root_page & 0xFF;
        header[1] = (root_page >> 8) & 0xFF;
        header[2] = 0;
        header[3] = 0;
        header[4] = total_pages & 0xFF;
        header[5] = (total_pages >> 8) & 0xFF;
        header[6] = 0;
        header[7] = 0;
        header[12] = key_len & 0xFF;
        header[13] = (key_len >> 8) & 0xFF;
        header[14] = entries_per_page & 0xFF;
        header[15] = (entries_per_page >> 8) & 0xFF;
        if (field_name) {
            int flen = (int)strlen(field_name);
            if (flen > 256) flen = 256;
            memcpy(header + 24, field_name, flen);
        }
        fwrite(header, 1, 512, f);
    }

    /* --- Pages 1..num_leaves: leaf data --- */
    for (int p = 0; p < num_leaves; p++) {
        int start = p * entries_per_page;
        int batch = count - start;
        if (batch > entries_per_page) batch = entries_per_page;

        char page[512];
        memset(page, 0, 512);
        page[0] = batch & 0xFF;
        page[1] = (batch >> 8) & 0xFF;

        for (int j = 0; j < batch; j++) {
            int idx = start + j;
            int base = 4 + j * entry_size;
            int rec = recnos[idx];
            page[base + 4] = rec & 0xFF;
            page[base + 5] = (rec >> 8) & 0xFF;
            page[base + 6] = (rec >> 16) & 0xFF;
            page[base + 7] = (rec >> 24) & 0xFF;
            int klen = (int)strlen(keys[idx]);
            if (klen > key_len) klen = key_len;
            memcpy(page + base + 8, keys[idx], klen);
            for (int k = klen; k < key_len; k++)
                page[base + 8 + k] = ' ';
        }
        fwrite(page, 1, 512, f);
    }

    /* --- Root page: interior node --- */
    {
        char page[512];
        memset(page, 0, 512);
        int root_entries = num_leaves;
        if (root_entries > entries_per_page)
            root_entries = entries_per_page;
        page[0] = root_entries & 0xFF;
        page[1] = (root_entries >> 8) & 0xFF;

        for (int e = 0; e < root_entries; e++) {
            int base = 4 + e * entry_size;
            int child = e + 1;
            page[base]     = child & 0xFF;
            page[base + 1] = (child >> 8) & 0xFF;
            page[base + 2] = 0;
            page[base + 3] = 0;
            int first_in_leaf = e * entries_per_page;
            char *sep_key = keys[first_in_leaf];
            int klen = (int)strlen(sep_key);
            if (klen > key_len) klen = key_len;
            memcpy(page + base + 8, sep_key, klen);
            for (int k = klen; k < key_len; k++)
                page[base + 8 + k] = ' ';
        }
        fwrite(page, 1, 512, f);
    }

    fclose(f);
    return 0;
}

