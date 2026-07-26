/*********************************************
 *
 *
 *
 *	mysql.c
 *
 *	(C) 2005 Alvaro Cortes. accsc@arbornet.org
 *
 *
 *	Handle import from MySQL databases
 *
 *
 *
 **********************************************/
#include <my_global.h>
#include <mysql.h>

#define MY_IMPORT_VER 0
#define MY_IMPORT_SUB 1
#define MY_IMPORT_DATA 200505

#warning "MySQL support activated"

void init_my()
{
	fprintf(stderr,"MySQL Import Module. Version %i.%i.%i\n",
			MY_IMPORT_VER, MY_IMPORT_SUB, MY_IMPORT_DATA);
	fflush(stderr);
}

int create_my_table(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name)
{
MYSQL a;
MYSQL_FIELD *c;
MYSQL_RES *b;
int d,i,o;
DATABASEDBF my_dbf;



mysql_init(&a);

if(mysql_real_connect(&a,host,user,password,dbf_name,0,NULL,0) == NULL)
{
#ifdef DEBUG
	fprintf(stderr,"Yeeepes, cant connect to database\n");
#endif
	return -1;
}

#ifdef DEBUG
fprintf(stderr,"Yeah. We got it.\n");

fprintf(stderr,"Conected to: %s\n",mysql_get_host_info(&a));
fprintf(stderr,"Server version: %s\n",mysql_get_server_info(&a));
fflush(stderr);
#endif
b = mysql_list_fields(&a,"dbf_table",NULL);
d = mysql_num_fields(b);
my_dbf.camposn = d;
for( i = 0; i<d ; ++i)
{
c = mysql_fetch_field_direct(b,i);
for( o = 0; o < strlen(c->name); ++o)
{
	my_dbf.fields.names[o][i+1] = (char) c->name[o];
}
for( o = o; o < 12; ++o)
	my_dbf.fields.names[o][i+1] = '\0';

	my_dbf.fields.longitudes[i+1] = (int) c->length;
	my_dbf.fields.decimales[i+1] = (int) c->decimals;
switch(c->type)
{
	case 0: my_dbf.fields.tipos[i+1] = 'N';  break;
	case 1: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 2: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 3: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 4: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 5: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 6: my_dbf.fields.tipos[i+1] = 'U'; break;
	case 7: my_dbf.fields.tipos[i+1] = 'T'; break;
	case 8: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 9: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 10: my_dbf.fields.tipos[i+1] = 'D'; break;
	case 11: my_dbf.fields.tipos[i+1] = 'T'; break;
	case 12: my_dbf.fields.tipos[i+1] = 'C'; break;
	case 13: my_dbf.fields.tipos[i+1] = 'N'; break;
	case 247: my_dbf.fields.tipos[i+1] = 'U'; break;
	case 248: my_dbf.fields.tipos[i+1] = 'U'; break;
	case 249: my_dbf.fields.tipos[i+1] = 'U'; break;
	case 250: my_dbf.fields.tipos[i+1] = 'U'; break;
	case 251: my_dbf.fields.tipos[i+1] = 'U'; break;
	case 252: my_dbf.fields.tipos[i+1] = 'U'; break;
	case 253: my_dbf.fields.tipos[i+1] = 'C'; break;
	case 254: my_dbf.fields.tipos[i+1] = 'C'; break;
	default: my_dbf.fields.tipos[i+1] = 'U'; break;
}
}
mysql_close(&a);
#ifdef DEBUG
	fprintf(stderr,"Database in memory.");
	fflush(stderr);
#endif

create_database(dbf_name,12,12,4,my_dbf,1);

#ifdef DEBUG
	fprintf(stderr,"Dump memory to file. Database Created.");
	fflush(stderr);
#endif

return 0;
}


int fill_my_table(char *user, char *password, char *host, int port, char *db, char *table, char *dbf_name)
{
}
