/*

	libDbase.h

	By Alvaro Cortés. (accsc@arbornet.org) 
	libdollybase. A library to use dbase files, like dbf and mem files
	and a completly memory and library functions similar to
	dBASE-compatible xBase syntax.
	
	To develop with libdollybase YOU MUST include this file in your app.


*/

#ifndef _LIBDBASE_H
#define _LIBDBASE_H

/* Para codigos de errores */
#define VERITAS 1
#define FALSO 2

#define MYSQL 1  /* Dont use " in field names */
#define OTHERS 2

#define MAX_DBT_CONTENT 2048 /* 2Kb for len of DBT fields         */
			     /* The block size is still 512 bytes  */
			  
extern int d_modules[3];

typedef struct {
int sig;
char Remarks[61];
int nLines;
int nCols;
int lMargin;
int spaceh;
int spacev;
int spacea;
char text[961];
} LBL;


struct ntx_handler{
char *fname;
int compiler_type;   	/* For NTX */
int compiler_version; 	/* For NTX */
int root_page;		/* For NTX/NDX */
int next_page;		/* For NTX */
int total_pages;	/* For NDX */
int key_len;		/* For NTX/NDX */
int key_dec;		/* For NTX only */
int key_type;		/* For NDX only */
int keys_per_page; 	/* For NDX */
int max_keys_per_page;	/* For NTX */
int min_keys_per_page;	/* For NTX */
char field_name[257];	/* For NTX/NDX */
int unique;		/* Only for NTX */

int current_keys_node;
int pos;
int node;
int type;
};

struct _found{
int pos;
int recno;
int page;
int interior;
};

typedef struct ntx_handler NTX;
typedef struct ntx_handler NDX;
typedef struct _found FOUND;

struct db_link{
char name[1024];
FILE *link_f;
};

typedef struct db_link DBLINK;


/* Estructura de los campos a utilizar con la de Base de Datos */
struct campos{
char names[1024][128]; /* Field names */
char tipos[128];       /* Types */
int longitudes[128];   /* Size */
int decimales[128];    /* Decimals */
};

/*************************************************************************
Estructura de base de datos, cada base de datos abierta es una estructura
de este tipo. Se usa con use().
*************************************************************************/

typedef struct {
int tipo;
char name[1024];
char date[10];
int recnos;     /* Total rec    */
int camposn;    /* Total fields */
int header_len; /* Head len     */
int current;    /* Current rec  */
int rec_len;    /* Rec len      */
int locate_is;
int located_campo;
char *located;
NTX index_node;	 /* Structure for anchor node of the index */
int index_type;  /* 0 = NDX,, 1 =NTX, 2=IDX  -1=NONE*/
struct campos fields;
} DATABASEDBF;



/* functions low.c */

void use( char *file, DATABASEDBF **asp);
int display_structure(DATABASEDBF *asp);
int eof_dbf(DATABASEDBF *asp);
int bof(DATABASEDBF *asp);
void DBF(DATABASEDBF *asp, char **name);

/* functions appends.c */

int append_blank(DATABASEDBF **asp);

/* functions memo.c */

/* functions deletes.c */
/*DATABASEDBF asp;*/

int delete(DATABASEDBF *asp);
int delete_next(DATABASEDBF *asp, int n);
int delete_all(DATABASEDBF *asp);
int recall(DATABASEDBF *asp);
int recall_next(DATABASEDBF **asp, int n);
int recall_all(DATABASEDBF *asp);
int zap(DATABASEDBF *asp);
int is_deleted(DATABASEDBF *asp);
int pack(DATABASEDBF *asp);
int pack_db_with_dbt_file(DATABASEDBF *asp, char *_na);

/* Acciones con registros o campos */
long long average_campo(DATABASEDBF *asp, char *campo, int nexts);
long long sum_campo(DATABASEDBF *asp, char *campo, int nexts);
int field_name(DATABASEDBF *asp, int campon, char **name);
void skip(DATABASEDBF **asp);
DATABASEDBF skip_index(DATABASEDBF asp);
void lupdate(DATABASEDBF *asp, char **date);
void dbf_update_date(DATABASEDBF *asp);
int reccount(DATABASEDBF *asp);
int recno(DATABASEDBF *asp);
int recsize(DATABASEDBF *asp);
int gotos(DATABASEDBF **asp, int rec);
int field_to_number(DATABASEDBF *asp, char *name);
void get_field2(DATABASEDBF *asp, int number, char **p);
void get_field(DATABASEDBF *asp, int number, char **p);
int dfield_type(DATABASEDBF *asp, int campon);
int replace(DATABASEDBF *asp, char *campo,char *rerum);
int replace2(DATABASEDBF *asp, char *campo, char *rerum);
int fields_num(DATABASEDBF *asp);



/* Filemakers  */
int create_database(char *name,int day, int month, int year,
		DATABASEDBF *db_struc,int multi);
int create_dbt_file(char *_name);



/* Index operations */
NTX use_ntx(char *_fname);
NDX * use_ndx(char *_fname);
void display_ntx_info(NTX *ind);
void display_ndx_info(NDX *ind);
FOUND search_ntx_next(NTX *ind,char *criteria,int last_page,int last_pos);
FOUND search_ndx_next(NTX *ind, char *criteria, int last_page,int last_pos);
FOUND seek_index(NTX *ind, char *criteria, int type);
FOUND seek_ndx_btree(NDX *ind, char *criteria);
FOUND seek_ntx_btree(NTX *ind, char *criteria);
int search(char *str, char *cri, int mode);
int create_index_ndx_generic(char **keys, int *recnos, int count,
                              int key_len, const char *field_name,
                              const char *_fname);

/* For export */
int export_as_csv(DATABASEDBF *asp, char sep, char *_fname);
int export_as_sql(DATABASEDBF *asp, char *_fname,int mode);
/*int export_as_html(DATABASEDBF asp, char *_fname);*/

/* DB Memo rec operations */
int get_db4_memo_block(char *_fname, int nblock, char *result, int max);
int get_memo_field( char *na,int block, char **result, int max);
int get_fpt_memo_field( char *_fname, int nBlock, char *res, int max);
int add_to_dbt(char *na, char *content,int max);
int replace_dbt_block(char *_na, int block, char *content);
int get_next_free_block(char *_na);

/* For Multi-User environment */
int cb_lock(DATABASEDBF *asp);
int dbf_lock(DATABASEDBF *asp);
int rec_lock(DATABASEDBF *asp, int nlock);
int dbf_unlock(DATABASEDBF *asp);
int rec_unlock(DATABASEDBF *asp, int nlock);
int if_dbf_lock(DATABASEDBF *asp);
int if_rec_lock(DATABASEDBF *asp, int nlock);

/* Label functions */
LBL * use_label(char *_fname);
int create_label(LBL * label1, char *_fname);

int get_dbt( char *na, char *ou);


void locate(DATABASEDBF **asp, int campo, char *re);

void create_relation(char *name, DATABASEDBF *asp, char *field, char *segundo, char *segundo_field, DBLINK **ba);
void get_relation_field(DBLINK *a, char *field, int rec, char **ba);
int get_relation_record(DBLINK *a, int rec);
int null_test( char *a);


#define COMPILED "UNIX"
#define VERSION "LibDollyBase 4 Series"


/***************** End of libDbase.h *****************/

#endif /* _LIBDBASE_H */


