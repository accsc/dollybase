#include "stdio.h"
#include "stdlib.h"
#include "string.h"

struct _printer{
char dev_name[1024]; /* Name of the device */
int tested;	     /* Has been tested? */
int printing;	     /* 0 - No | 1 - Printing */
char name[512];	     /* Internal name of the printer */
};

typedef struct _printer printer;

int send_to_printer(printer prn, char *sthg);

printer set_printer(printer prn,char *dev,char name[512])
{
	if ( strlen(dev) > 1023)
	strncpy(prn.dev_name,dev,1023);
	else
	strncpy(prn.dev_name,dev,strlen(dev));
	strncpy(prn.name,name,511);
	return prn;
}

int print_line(printer prn, char *line)
{
	return send_to_printer(prn,line);
}
int print_chr(printer prn, char chr)
{
	char unico[2];
	unico[0] = chr;
	unico[1] = '\0';
	return send_to_printer(prn,unico);
}

int send_to_printer(printer prn, char *sthg)
{
	FILE *prin;

	if( (prin = fopen(prn.name,"ab")) == NULL)
	{
		return -1;
	}
	if( fwrite(sthg,1,strlen(sthg),prin) != strlen(sthg) )
	{
		return -2;
	}
	return 0;
}
