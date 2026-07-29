#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../libdbase_4/libdbase.h"

int main() {
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    use("books_memo.dbf", &db);
    printf("Records: %d\n", db->recnos);
    
    gotos(&db, 1);
    char *p = malloc(257);
    char *raw = p;
    get_field2(db, 7, &raw);
    printf("Record 1 memo block ptr (raw): [%s]\n", raw);
    free(p);
    
    gotos(&db, 38);
    p = malloc(257);
    raw = p;
    get_field2(db, 7, &raw);
    printf("Record 38 memo block ptr (raw): [%s]\n", raw);
    free(p);
    
    p = malloc(1024);
    raw = p;
    get_field(db, 7, &raw);
    printf("Record 38 memo (full): [%s]\n", raw);
    free(p);
    
    free(db);
    return 0;
}
