#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../libdbase_4/libdbase.h"

int main() {
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    use("books_memo.dbf", &db);
    printf("current=%d recnos=%d tipo=%d\n", db->current, db->recnos, db->tipo);
    
    skip(&db);
    printf("after skip: current=%d\n", db->current);
    
    skip(&db);
    printf("after skip: current=%d\n", db->current);
    
    free(db);
    return 0;
}
