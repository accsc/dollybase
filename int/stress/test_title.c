#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../libdbase_4/libdbase.h"

int main() {
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    use("books_memo.dbf", &db);
    printf("current=%d recnos=%d\n", db->current, db->recnos);
    
    char *p = malloc(1024);
    char *raw = p;
    get_field(db, 1, &raw);
    printf("Record %d TITULO: [%s]\n", db->current, raw);
    free(p);
    
    skip(&db);
    printf("current=%d\n", db->current);
    
    p = malloc(1024);
    raw = p;
    get_field(db, 1, &raw);
    printf("Record %d TITULO: [%s]\n", db->current, raw);
    free(p);
    
    skip(&db);
    printf("current=%d\n", db->current);
    
    p = malloc(1024);
    raw = p;
    get_field(db, 1, &raw);
    printf("Record %d TITULO: [%s]\n", db->current, raw);
    free(p);
    
    free(db);
    return 0;
}
