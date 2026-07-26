/************************************************************
 *
 *
 *
 *
 *
 * (C) Alvaro Cortés. 2004. accsc@arbornet.org
 *
 * Under GPL v2 or above licence. NO WARRANTY. Use UNDER YOUR OWN RISK.
 *
 *
 *
 * Module for libdollybase. Handle lock stuff.
 *
 *
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "libdbase.h"

/************************************************************
 *
 * First. Test with cb_lock if you can use locks functions.
 *
*************************************************************/

/* Can be locked? */
/* 0=NO, other nº of rec for lock */
int cb_lock(DATABASEDBF *asp)
{
	return field_to_number(asp,"_DBFLOCK");
}

/****************************
 * LOCK mechanism:
 *
 * First rec. _DBFLOCK contains 1 for lock the first rec, 2 for lock all file.
 *  other thing for unlock.
 * Other rec. _DBFLOCK conatins 1 for lock other rec. Other thing nothing.
 * 
 *****************************/

int dbf_lock(DATABASEDBF *asp)
{
	gotos(&asp,1);
	if( replace(asp,"_DBFLOCK","2") == 0)
		return 0;
	else
		return -1;
}

int dbf_unlock(DATABASEDBF *asp)
{
	gotos(&asp,1);
	if( replace(asp,"_DBFLOCK","0") == 0)
		return 0;
	else
		return -1;
}

int rec_lock(DATABASEDBF *asp, int nlock)
{
	gotos(&asp,nlock);
	if( replace(asp,"_DBFLOCK","1") == 0)
		return 0;
	else
		return -1;
}
int rec_unlock(DATABASEDBF *asp, int nlock)
{
	gotos(&asp,nlock);
	if( replace(asp,"_DBFLOCK","0") == 0)
		return 0;
	else 
		return -1;
}

int if_dbf_lock(DATABASEDBF *asp)
{
	char *f1;
	int i;
	if( (f1 = malloc(257)) == NULL)
	{
		fprintf(stderr,"Error. Sin memoria.\n");
		fflush(stderr);
		return -1;
	}
	gotos(&asp,1);
	get_field(asp, field_to_number(asp,"_DBFLOCK"),&f1);
	i = atoi(f1);
	free(f1);
	return i;
}
int if_rec_lock(DATABASEDBF *asp, int nlock)
{
	char *f1;
	int i;
	if( (f1 = malloc(257)) == NULL)
	{
		fprintf(stderr,"Error. Sin memoria.\n");
		fflush(stderr);
		return -1;
	}
	gotos(&asp,nlock);
	get_field(asp,field_to_number(asp,"_DBFLOCK"),&f1);
	i = atoi(f1);
	free(f1);
	return i;
}
