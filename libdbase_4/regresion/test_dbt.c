/*
 * test_dbt.c — DBT (memo field) regression test for libdollybase
 *
 * Tests:
 *   1. create_database with memo field sets 0x83 flag + auto-creates .dbt
 *   2. get_dbt() path derivation (lowercase, uppercase, with path, no ext)
 *   3. add_to_dbt() single-block write
 *   4. add_to_dbt() multi-block write (>510 bytes)
 *   5. get_memo_field() read-back
 *   6. replace_dbt_block() update
 *   7. pack_db_with_dbt_file() preserves memo data
 *
 * Compile:
 *   gcc -w -o test_dbt test_dbt.c ../.libs/libdbase_0.4_s.a
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include "../libdbase.h"

static int tests_run    = 0;
static int tests_passed = 0;
static int tests_failed = 0;

#define TEST(name)  do { \
    tests_run++; \
    printf("  %-60s", #name); \
} while (0)

#define PASS()  do { \
    tests_passed++; \
    printf("[PASS]\n"); \
} while (0)

#define FAIL(msg) do { \
    tests_failed++; \
    printf("[FAIL] %s\n", msg); \
} while (0)

#define ASSERT_EQ_INT(expected, actual)  do { \
    if ((expected) != (actual)) { \
        char _buf[128]; \
        snprintf(_buf, sizeof(_buf), \
                 "expected %d, got %d", (expected), (actual)); \
        FAIL(_buf); \
        return; \
    } \
} while (0)

#define ASSERT_STR_EQ(expected, actual)  do { \
    if (strcmp((expected), (actual)) != 0) { \
        char _buf[256]; \
        snprintf(_buf, sizeof(_buf), \
                 "expected \"%s\", got \"%s\"", (expected), (actual)); \
        FAIL(_buf); \
        return; \
    } \
} while (0)

#define ASSERT_TRUE(cond)  do { \
    if (!(cond)) { \
        FAIL("condition was false"); \
        return; \
    } \
} while (0)

/* Cleanup helper */
static void cleanup(const char *base)
{
    char path[1024];
    /* Remove .dbf */
    snprintf(path, sizeof(path), "%s.dbf", base);
    unlink(path);
    snprintf(path, sizeof(path), "%s.DBf", base);
    unlink(path);
    /* Remove .dbt */
    snprintf(path, sizeof(path), "%s.dbt", base);
    unlink(path);
    snprintf(path, sizeof(path), "%s.DBT", base);
    unlink(path);
}

/* ================================================================ */
/* Test: get_dbt() path derivation                                  */
/* ================================================================ */

static void test_get_dbt_lowercase(void)
{
    TEST(get_dbt_lowercase);
    char out[1024];
    get_dbt("test.dbf", out);
    ASSERT_STR_EQ("test.dbt", out);
    PASS();
}

static void test_get_dbt_uppercase(void)
{
    TEST(get_dbt_uppercase);
    char out[1024];
    get_dbt("TEST.DBF", out);
    ASSERT_STR_EQ("TEST.DBT", out);
    PASS();
}

static void test_get_dbt_with_path(void)
{
    TEST(get_dbt_with_path);
    char out[1024];
    get_dbt("/home/user/data/myfile.dbf", out);
    ASSERT_STR_EQ("/home/user/data/myfile.dbt", out);
    PASS();
}

static void test_get_dbt_no_extension(void)
{
    TEST(get_dbt_no_extension);
    char out[1024];
    get_dbt("myfile", out);
    ASSERT_STR_EQ("myfile.dbt", out);
    PASS();
}

/* ================================================================ */
/* Test: create_database with memo field                            */
/* ================================================================ */

static void test_create_with_memo(void)
{
    TEST(create_database_with_memo);
    DATABASEDBF *db = malloc(sizeof(DATABASEDBF));
    memset(db, 0, sizeof(DATABASEDBF));

    db->camposn = 2;
    /* Field 1: NAME C(20) — names[char_idx][field_num] */
    db->fields.names[0][1] = 'N'; db->fields.names[1][1] = 'A';
    db->fields.names[2][1] = 'M'; db->fields.names[3][1] = 'E';
    db->fields.tipos[1] = 'C';
    db->fields.longitudes[1] = 20;
    db->fields.decimales[1] = 0;

    /* Field 2: NOTES M */
    db->fields.names[0][2] = 'N'; db->fields.names[1][2] = 'O';
    db->fields.names[2][2] = 'T'; db->fields.names[3][2] = 'E';
    db->fields.names[4][2] = 'S';
    db->fields.tipos[2] = 'M';
    db->fields.longitudes[2] = 10; /* Will be forced to 10 by create_database */
    db->fields.decimales[2] = 0;

    cleanup("memo_test");

    int rc = create_database("memo_test.dbf", 27, 7, 26, db, 0);
    ASSERT_EQ_INT(0, rc);

    /* Verify .dbt was auto-created */
    ASSERT_TRUE(access("memo_test.dbt", F_OK) == 0);

    /* Verify the DBF header byte 0 is 0x83 (has memo) */
    FILE *f = fopen("memo_test.dbf", "rb");
    ASSERT_TRUE(f != NULL);
    unsigned char header_byte = getc(f);
    fclose(f);
    ASSERT_EQ_INT(0x83, header_byte);

    /* Verify tipo is set correctly when we use() the database */
    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));
    use("memo_test.dbf", &asp);
    ASSERT_EQ_INT(3, asp->tipo); /* tipo 3 = DBT memo */
    free(asp);

    cleanup("memo_test");
    free(db);
    PASS();
}

static void test_create_without_memo(void)
{
    TEST(create_database_without_memo);
    DATABASEDBF *db = malloc(sizeof(DATABASEDBF));
    memset(db, 0, sizeof(DATABASEDBF));

    db->camposn = 1;
    db->fields.names[0][1] = 'N'; db->fields.names[1][1] = 'A';
    db->fields.names[2][1] = 'M'; db->fields.names[3][1] = 'E';
    db->fields.tipos[1] = 'C';
    db->fields.longitudes[1] = 20;

    cleanup("nomemo_test");

    int rc = create_database("nomemo_test.dbf", 27, 7, 26, db, 0);
    ASSERT_EQ_INT(0, rc);

    /* Verify .dbt was NOT created */
    ASSERT_TRUE(access("nomemo_test.dbt", F_OK) != 0);

    /* Verify header byte is 0x03 (no memo) */
    FILE *f = fopen("nomemo_test.dbf", "rb");
    ASSERT_TRUE(f != NULL);
    unsigned char header_byte = getc(f);
    fclose(f);
    ASSERT_EQ_INT(0x03, header_byte);

    cleanup("nomemo_test");
    free(db);
    PASS();
}

/* ================================================================ */
/* Test: add_to_dbt + get_memo_field round-trip                     */
/* ================================================================ */

static void test_memo_roundtrip_single_block(void)
{
    TEST(memo_roundtrip_single_block);
    cleanup("memo_rt");

    /* Create DBT file */
    ASSERT_EQ_INT(0, create_dbt_file("memo_rt.dbt"));

    /* Write a memo field (< 510 bytes) */
    const char *content = "Hello, this is a memo field test!";
    int rc = add_to_dbt("memo_rt.dbt", (char *)content, strlen(content));
    ASSERT_EQ_INT(0, rc);

    /* Read it back */
    char *result = malloc(2048);
    rc = get_memo_field("memo_rt.dbt", 1, &result, 2047);
    ASSERT_EQ_INT(0, rc);
    ASSERT_STR_EQ(content, result);

    free(result);
    cleanup("memo_rt");
    PASS();
}

static void test_memo_roundtrip_multi_block(void)
{
    TEST(memo_roundtrip_multi_block);
    cleanup("memo_multi");

    /* Create DBT file */
    ASSERT_EQ_INT(0, create_dbt_file("memo_multi.dbt"));

    /* Write content > 510 bytes (spans 2 blocks) */
    char content[1200];
    for (int i = 0; i < 1199; ++i)
        content[i] = 'A' + (i % 26);
    content[1199] = '\0';

    int rc = add_to_dbt("memo_multi.dbt", content, strlen(content));
    ASSERT_EQ_INT(0, rc);

    /* Verify the DBT file has the right size:
     * 1199 bytes / 510 per block = 3 data blocks + 1 header = 4 blocks = 2048 bytes */
    struct stat st;
    stat("memo_multi.dbt", &st);
    ASSERT_EQ_INT(2048, st.st_size);

    /* Verify the next-block pointer in header is 4 (blocks 1-3 used) */
    FILE *f = fopen("memo_multi.dbt", "rb");
    ASSERT_TRUE(f != NULL);
    unsigned char hdr[4];
    fread(hdr, 1, 4, f);
    fclose(f);
    int next_block = hdr[0] + (hdr[1] << 8) + (hdr[2] << 16) + (hdr[3] << 24);
    ASSERT_EQ_INT(4, next_block);

    /* Verify content is split correctly across blocks */
    f = fopen("memo_multi.dbt", "rb");
    char buf[512];

    /* Block 1: bytes 0-509 */
    fseek(f, 512, SEEK_SET);
    fread(buf, 1, 512, f);
    for (int i = 0; i < 510; ++i) {
        if (buf[i] != content[i]) {
            FAIL("block 1 content mismatch");
            fclose(f);
            cleanup("memo_multi");
            return;
        }
    }

    /* Block 2: bytes 510-1019 */
    fseek(f, 1024, SEEK_SET);
    fread(buf, 1, 512, f);
    for (int i = 0; i < 510; ++i) {
        if (buf[i] != content[510 + i]) {
            FAIL("block 2 content mismatch");
            fclose(f);
            cleanup("memo_multi");
            return;
        }
    }

    /* Block 3: bytes 1020-1198 (179 bytes) */
    fseek(f, 1536, SEEK_SET);
    fread(buf, 1, 512, f);
    fclose(f);
    for (int i = 0; i < 179; ++i) {
        if (buf[i] != content[1020 + i]) {
            FAIL("block 3 content mismatch");
            cleanup("memo_multi");
            return;
        }
    }

    cleanup("memo_multi");
    PASS();
}

/* ================================================================ */
/* Test: replace_dbt_block                                          */
/* ================================================================ */

static void test_replace_memo_block(void)
{
    TEST(replace_memo_block);
    cleanup("memo_rep");

    /* Create DBT file and write initial content */
    ASSERT_EQ_INT(0, create_dbt_file("memo_rep.dbt"));
    add_to_dbt("memo_rep.dbt", "Original content", 16);

    /* Replace the block */
    int rc = replace_dbt_block("memo_rep.dbt", 1, "Updated content!");
    ASSERT_EQ_INT(0, rc);

    /* Read back */
    char *result = malloc(2048);
    rc = get_memo_field("memo_rep.dbt", 1, &result, 2047);
    ASSERT_EQ_INT(0, rc);
    ASSERT_STR_EQ("Updated content!", result);

    free(result);
    cleanup("memo_rep");
    PASS();
}

/* ================================================================ */
/* Test: Full DBF+DBT lifecycle through library API                 */
/* ================================================================ */

static void test_full_lifecycle(void)
{
    TEST(full_dbf_dbt_lifecycle);
    cleanup("lifecycle");

    /* Create database with a memo field */
    DATABASEDBF *db = malloc(sizeof(DATABASEDBF));
    memset(db, 0, sizeof(DATABASEDBF));

    db->camposn = 2;
    db->fields.names[0][1] = 'I'; db->fields.names[1][1] = 'D';
    db->fields.tipos[1] = 'C';
    db->fields.longitudes[1] = 10;

    db->fields.names[0][2] = 'D'; db->fields.names[1][2] = 'E';
    db->fields.names[2][2] = 'S'; db->fields.names[3][2] = 'C';
    db->fields.tipos[2] = 'M';
    db->fields.longitudes[2] = 10;

    ASSERT_EQ_INT(0, create_database("lifecycle.dbf", 27, 7, 26, db, 0));

    /* Open the database */
    DATABASEDBF *asp = malloc(sizeof(DATABASEDBF));
    use("lifecycle.dbf", &asp);
    ASSERT_EQ_INT(3, asp->tipo);

    /* Append a record */
    ASSERT_EQ_INT(0, append_blank(&asp));
    ASSERT_EQ_INT(1, asp->recnos);

    /* Replace the ID field */
    replace(asp, "ID", "REC1");

    /* Write memo content to DBT (block 1) */
    add_to_dbt("lifecycle.dbt", "Memo data for record 1", 24);

    /* Set the memo block pointer in the DBF manually via replace2
     * (replace() for M fields tries to use the existing block pointer,
     *  which is empty — so we use replace2 which writes directly) */
    replace2(asp, "DESC", "1");

    /* Read back the memo field */
    char *memo = malloc(2048);
    char *p = memo;
    get_field(asp, 2, &p);
    ASSERT_STR_EQ("Memo data for record 1", memo);

    free(memo);
    free(asp);
    free(db);
    cleanup("lifecycle");
    PASS();
}

/* ================================================================ */
/* Main                                                             */
/* ================================================================ */

int main(void)
{
    printf("=== DBT (Memo Field) Regression Tests ===\n\n");

    printf("--- Path derivation ---\n");
    test_get_dbt_lowercase();
    test_get_dbt_uppercase();
    test_get_dbt_with_path();
    test_get_dbt_no_extension();

    printf("\n--- Database creation ---\n");
    test_create_with_memo();
    test_create_without_memo();

    printf("\n--- Memo I/O ---\n");
    test_memo_roundtrip_single_block();
    test_memo_roundtrip_multi_block();
    test_replace_memo_block();

    printf("\n--- Full lifecycle ---\n");
    test_full_lifecycle();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           tests_passed, tests_run, tests_failed);

    return tests_failed > 0 ? 1 : 0;
}
