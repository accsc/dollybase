#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../libdbase_4/libdbase.h"

int main() {
    char *result = malloc(2048);
    
    // Try block 1
    get_memo_field("books_memo.dbt", 1, &result, 2047);
    printf("Block 1: [%s]\n", result);
    memset(result, 0, 2048);
    
    // Try block 38
    get_memo_field("books_memo.dbt", 38, &result, 2047);
    printf("Block 38: [%s]\n", result);
    memset(result, 0, 2048);
    
    // Try block 2
    get_memo_field("books_memo.dbt", 2, &result, 2047);
    printf("Block 2: [%s]\n", result);
    
    free(result);
    return 0;
}
