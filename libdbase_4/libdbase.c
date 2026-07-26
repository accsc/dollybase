#include <stdio.h>
#include <stdlib.h>

#define COMPILED "UNIX"
#define VERSION "LibDollyBase 4 Series"

#include "str_funcs.c"
#include "index.c"
#include "low.c"
#include "recs.c"
/*#include "internal.c"*/
#include "deletes.c"
#include "seeks.c"
#include "appends.c"
/*#include "memo.c"*/
#include "mate.c"
#include "creates.c"
#include "printer.c"
/*#include "dbase2.c"*/
#include "export.c"
#include "memofields.c"
#include "locker.c"
#include "labels.c"
#include "imports/common.c"
#include "relations.c"

int init_libdollybase()
{
	return 0;
}
