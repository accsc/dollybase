/*
 * test_memo_replace.c — Verify REPLACE works on MEMO fields via the
 * high-level replace() API (the path used by the PRG interpreter).
 *
 * Specifically tests the y==0 branch (new memo block allocation)
 * that previously had a double-free bug.
 *
 * Compile:
 *   gcc -w -o test_memo_replace test_memo_replace.c ../.libs/libdbase_0.4_s.a
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../libdbase.h"

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)  do { \
    tests_run++; \
    printf("  %-60s", #name); \
} while (0)

#define PASS()  do { tests_passed++; printf("[PASS]\n"); } while (0)

#define FAIL(msg) do { \
    tests_failed++; \
    printf("[FAIL] %s\n", msg); \
} while (0)

#define ASSERT_EQ_INT(e, a)  do { \
    if ((e) != (a)) { \
        char _b[128]; \
        snprintf(_b, sizeof(_b), "expected %d, got %d", (e), (a)); \
        FAIL(_b); return; \
    } \
} while (0)

#define ASSERT_STR_EQ(e, a)  do { \
    if (strcmp((e), (a)) != 0) { \
        char _b[256]; \
        snprintf(_b, sizeof(_b), "expected \"%s\", got \"%s\"", (e), (a)); \
        FAIL(_b); return; \
    } \
} while (0)

#define ASSERT_TRUE(c)  do { if (!(c)) { FAIL("condition false"); return; } } while (0)

static void cleanup(const char *base)
{
    char p[1024];
    snprintf(p, sizeof(p), "%s.dbf", base); unlink(p);
    snprintf(p, sizeof(p), "%s.dbt", base); unlink(p);
}

/* Helper: set field name (names[char_pos][field_idx]) */
static void set_name(char names[11][128], int idx, const char *n)
{
    for (int c = 0; c < 11; c++)
        names[c][idx] = (c < (int)strlen(n)) ? n[c] : '\0';
}

/* ================================================================ */
/* Test: replace() on a new MEMO field (y==0 branch)               */
/* This is the path that previously double-freed 'me'              */
/* ================================================================ */
static void test_replace_new_memo(void)
{
    TEST(replace_new_memo_field);
    cleanup("memo_new");

    DATABASEDBF *db = malloc(sizeof(DATABASEDBF));
    memset(db, 0, sizeof(DATABASEDBF));
    db->camposn = 2;
    set_name(db->fields.names, 1, "ID");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 10;
    set_name(db->fields.names, 2, "DESC");
    db->fields.tipos[2] = 'M'; db->fields.longitudes[2] = 10;

    ASSERT_EQ_INT(0, create_database("memo_new.dbf", 30, 7, 26, db, 0));
    free(db);

    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));
    memset(asp, 0, sizeof(DATABASEDBF));
    use("memo_new.dbf", &asp);
    ASSERT_EQ_INT(3, asp->tipo);

    /* Append a blank record — memo block pointer is 0 */
    ASSERT_EQ_INT(0, append_blank(&asp));

    /* Replace ID (non-memo) — should work */
    ASSERT_EQ_INT(0, replace(asp, "ID", "REC1"));

    /* Replace MEMO field on new record — this hits the y==0 branch */
    int rc = replace(asp, "DESC", "First memo content");
    ASSERT_EQ_INT(0, rc);

    /* Read it back */
    char *memo = malloc(2048);
    char *p = memo;
    get_field(asp, 2, &p);
    ASSERT_STR_EQ("First memo content", memo);
    free(memo);

    cleanup("memo_new");
    free(asp);
    PASS();
}

/* ================================================================ */
/* Test: multiple REPLACE on MEMO fields across multiple records   */
/* (the exact pattern seed_data.prg uses)                          */
/* ================================================================ */
static void test_replace_multiple_memo_records(void)
{
    TEST(replace_multiple_memo_records);
    cleanup("memo_multi");

    DATABASEDBF *db = malloc(sizeof(DATABASEDBF));
    memset(db, 0, sizeof(DATABASEDBF));
    db->camposn = 2;
    set_name(db->fields.names, 1, "NAME");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 20;
    set_name(db->fields.names, 2, "NOTES");
    db->fields.tipos[2] = 'M'; db->fields.longitudes[2] = 10;

    ASSERT_EQ_INT(0, create_database("memo_multi.dbf", 30, 7, 26, db, 0));
    free(db);

    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));
    memset(asp, 0, sizeof(DATABASEDBF));
    use("memo_multi.dbf", &asp);

    const char *expected[] = {
        "Note for item Alpha",
        "Note for item Beta",
        "Note for item Gamma",
        "Note for item Delta",
        "Note for item Epsilon"
    };
    const char *names[] = {"Alpha", "Beta", "Gamma", "Delta", "Epsilon"};
    int count = 5;

    for (int i = 0; i < count; i++) {
        ASSERT_EQ_INT(0, append_blank(&asp));
        ASSERT_EQ_INT(0, replace(asp, "NAME", names[i]));
        int rc = replace(asp, "NOTES", expected[i]);
        ASSERT_EQ_INT(0, rc);
    }

    /* Verify all records */
    for (int i = 1; i <= count; i++) {
        gotos(&asp, i);
        char *memo = malloc(2048);
        char *p = memo;
        get_field(asp, 2, &p);
        ASSERT_STR_EQ(expected[i - 1], memo);
        free(memo);
    }

    cleanup("memo_multi");
    free(asp);
    PASS();
}

/* ================================================================ */
/* Test: REPLACE existing MEMO block (y!=0 branch)                 */
/* ================================================================ */
static void test_replace_existing_memo(void)
{
    TEST(replace_existing_memo);
    cleanup("memo_upd");

    DATABASEDBF *db = malloc(sizeof(DATABASEDBF));
    memset(db, 0, sizeof(DATABASEDBF));
    db->camposn = 1;
    set_name(db->fields.names, 1, "TXT");
    db->fields.tipos[1] = 'M'; db->fields.longitudes[1] = 10;

    ASSERT_EQ_INT(0, create_database("memo_upd.dbf", 30, 7, 26, db, 0));
    free(db);

    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));
    memset(asp, 0, sizeof(DATABASEDBF));
    use("memo_upd.dbf", &asp);

    ASSERT_EQ_INT(0, append_blank(&asp));
    ASSERT_EQ_INT(0, replace(asp, "TXT", "Original text"));

    /* Update the same memo field — hits y!=0 branch */
    ASSERT_EQ_INT(0, replace(asp, "TXT", "Updated text"));

    char *memo = malloc(2048);
    char *p = memo;
    get_field(asp, 1, &p);
    ASSERT_STR_EQ("Updated text", memo);
    free(memo);

    cleanup("memo_upd");
    free(asp);
    PASS();
}

/* ================================================================ */
/* Test: REPLACE MEMO with empty string                            */
/* ================================================================ */
static void test_replace_memo_empty(void)
{
    TEST(replace_memo_empty);
    cleanup("memo_empty");

    DATABASEDBF *db = malloc(sizeof(DATABASEDBF));
    memset(db, 0, sizeof(DATABASEDBF));
    db->camposn = 1;
    set_name(db->fields.names, 1, "TXT");
    db->fields.tipos[1] = 'M'; db->fields.longitudes[1] = 10;

    ASSERT_EQ_INT(0, create_database("memo_empty.dbf", 30, 7, 26, db, 0));
    free(db);

    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));
    memset(asp, 0, sizeof(DATABASEDBF));
    use("memo_empty.dbf", &asp);

    ASSERT_EQ_INT(0, append_blank(&asp));
    ASSERT_EQ_INT(0, replace(asp, "TXT", ""));

    char *memo = malloc(2048);
    char *p = memo;
    get_field(asp, 1, &p);
    ASSERT_STR_EQ("", memo);
    free(memo);

    cleanup("memo_empty");
    free(asp);
    PASS();
}

/* ================================================================ */
int main(void)
{
    printf("=== MEMO REPLACE Regression Tests ===\n\n");

    test_replace_new_memo();
    test_replace_multiple_memo_records();
    test_replace_existing_memo();
    test_replace_memo_empty();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           tests_passed, tests_run, tests_failed);
    return tests_failed > 0 ? 1 : 0;
}
