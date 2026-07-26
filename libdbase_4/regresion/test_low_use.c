/*
 * Unit tests for use() in low.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../libdbase.h"

static int tests_run = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define ASSERT(cond, msg) do { \
    tests_run++; \
    if (!(cond)) { \
        tests_failed++; \
        printf("  FAIL: %s\n", msg); \
    } else { \
        tests_passed++; \
        printf("  PASS: %s\n", msg); \
    } \
} while (0)

/* Test: use() with a valid DBF file (books.dbf) */
static void test_use_valid_dbf(void)
{
    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));

    printf("\n--- test_use_valid_dbf ---\n");
    use("books.dbf", &asp);

    ASSERT(asp != NULL, "use() returned a non-NULL pointer");
    ASSERT(asp->tipo != 0, "database type was detected (tipo != 0)");
    ASSERT(asp->recnos >= 0, "record count is non-negative");
    ASSERT(asp->camposn > 0, "field count is greater than zero");
    ASSERT(asp->header_len > 0, "header length is greater than zero");
    ASSERT(asp->current == 1, "current record starts at 1");
    ASSERT(asp->rec_len > 0, "record length is greater than zero");
    ASSERT(strlen(asp->name) > 0, "database name was populated");
    ASSERT(strlen(asp->date) > 0, "database date was populated");
    ASSERT(asp->located == NULL, "located pointer is NULL by default");
    ASSERT(asp->located_campo == 0, "located_campo is 0 by default");
    free(asp);
}

/* Test: use() with a non-existent file */
static void test_use_nonexistent_file(void)
{
    DATABASEDBF *asp = NULL;

    printf("\n--- test_use_nonexistent_file ---\n");
    use("this_file_does_not_exist.dbf", &asp);

    ASSERT(asp == NULL, "use() returns NULL for non-existent file");
}

/* Test: use() populates field names correctly */
static void test_use_field_names(void)
{
    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));
    int i;

    printf("\n--- test_use_field_names ---\n");
    use("books.dbf", &asp);

    ASSERT(asp != NULL, "use() succeeded");

    for (i = 1; i <= asp->camposn; i++) {
        ASSERT(asp->fields.names[0][i] != '\0',
               "each field has at least one character in its name");
        ASSERT(asp->fields.tipos[i] != '\0',
               "each field has a type character");
        ASSERT(asp->fields.longitudes[i] > 0,
               "each field has a positive length");
    }
    free(asp);
}

/* Test: use() with relative path to books.dbf */
static void test_use_with_path(void)
{
    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));

    printf("\n--- test_use_with_path ---\n");
    use("../books.dbf", &asp);

    ASSERT(asp != NULL, "use() with relative path succeeded");
    ASSERT(asp->tipo != 0, "database type was detected");
    ASSERT(asp->camposn > 0, "field count is greater than zero");
    free(asp);
}

int main(void)
{
    printf("========================================\n");
    printf(" Unit tests for use() in low.c\n");
    printf("========================================\n");

    test_use_valid_dbf();
    test_use_nonexistent_file();
    test_use_field_names();
    test_use_with_path();

    printf("\n========================================\n");
    printf(" Results: %d/%d passed, %d failed\n",
           tests_passed, tests_run, tests_failed);
    printf("========================================\n");

    return tests_failed > 0 ? 1 : 0;
}
