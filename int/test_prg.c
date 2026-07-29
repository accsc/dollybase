/*
 * test_prg.c — dBase III Plus .PRG interpreter test suite
 *
 * Tests: tokenizer, expression parser, variable store, executor
 *
 * Compile:
 *   gcc -w -o test_prg test_prg.c tokenizer.c parser.c executor.c \
 *       exprvalue.c variables.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "tokenizer.h"
#include "parser.h"
#include "executor.h"
#include "variables.h"
#include "exprvalue.h"
#include "workarea.h"

/* ------------------------------------------------------------------ */
/* Test harness                                                        */
/* ------------------------------------------------------------------ */

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

#define ASSERT_EQ_DBL(expected, actual, tol)  do { \
    if (fabs((expected) - (actual)) > (tol)) { \
        char _buf[128]; \
        snprintf(_buf, sizeof(_buf), \
                 "expected %.6f, got %.6f", (expected), (actual)); \
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

/* ------------------------------------------------------------------ */
/* Helper: tokenize a string and return the token list                  */
/* ------------------------------------------------------------------ */

static Token *tok(const char *src)
{
    return tokenize(src);
}

/* Helper: parse expression from source string */
static ExprValue parse(const char *src)
{
    Token *tokens = tok(src);
    Token *cur = tokens;
    ParseError err = PARSE_OK;
    ExprValue result = parse_expression(&cur, &err);
    free_tokens(tokens);
    return result;
}

/* Helper: run a full program and return status */
static ExecStatus run(const char *src)
{
    Token *tokens = tok(src);
    proc_scan(tokens);
    ExecStatus st = execute_tokens(tokens);
    free_tokens(tokens);
    return st;
}

/* ------------------------------------------------------------------ */
/* Tokenizer tests                                                      */
/* ------------------------------------------------------------------ */

static void test_tokenizer_integer(void)
{
    TEST(tokenizer_integer);
    Token *t = tok("42");
    ASSERT_EQ_INT(TOK_INTEGER, t->type);
    ASSERT_STR_EQ("42", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_real(void)
{
    TEST(tokenizer_real);
    Token *t = tok("3.14");
    ASSERT_EQ_INT(TOK_REAL, t->type);
    ASSERT_STR_EQ("3.14", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_string(void)
{
    TEST(tokenizer_string);
    Token *t = tok("\"hello\"");
    ASSERT_EQ_INT(TOK_STRING, t->type);
    ASSERT_STR_EQ("hello", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_logical_true(void)
{
    TEST(tokenizer_logical_true);
    Token *t = tok(".T.");
    ASSERT_EQ_INT(TOK_LOGICAL, t->type);
    ASSERT_STR_EQ(".T.", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_logical_false(void)
{
    TEST(tokenizer_logical_false);
    Token *t = tok(".F.");
    ASSERT_EQ_INT(TOK_LOGICAL, t->type);
    ASSERT_STR_EQ(".F.", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_logical_y_n(void)
{
    TEST(tokenizer_logical_y_n);
    Token *t = tok(".Y.");
    ASSERT_EQ_INT(TOK_LOGICAL, t->type);
    ASSERT_STR_EQ(".T.", t->value);  /* .Y. normalizes to .T. */
    free_tokens(t);

    t = tok(".N.");
    ASSERT_EQ_INT(TOK_LOGICAL, t->type);
    ASSERT_STR_EQ(".F.", t->value);  /* .N. normalizes to .F. */
    free_tokens(t);
    PASS();
}

static void test_tokenizer_date_literal(void)
{
    TEST(tokenizer_date_literal);
    Token *t = tok("{^2006-07-26}");
    ASSERT_EQ_INT(TOK_DATE, t->type);
    ASSERT_STR_EQ("2006-07-26", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_date_no_caret(void)
{
    TEST(tokenizer_date_no_caret);
    Token *t = tok("{2006-07-26}");
    ASSERT_EQ_INT(TOK_DATE, t->type);
    ASSERT_STR_EQ("2006-07-26", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_keyword_if(void)
{
    TEST(tokenizer_keyword_if);
    Token *t = tok("IF");
    ASSERT_EQ_INT(TOK_KEYWORD, t->type);
    ASSERT_EQ_INT(KW_IF, t->keyword_id);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_keyword_while(void)
{
    TEST(tokenizer_keyword_while);
    Token *t = tok("WHILE");
    ASSERT_EQ_INT(TOK_KEYWORD, t->type);
    ASSERT_EQ_INT(KW_WHILE, t->keyword_id);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_assignment_colon_eq(void)
{
    TEST(tokenizer_assignment_colon_eq);
    Token *t = tok(":=");
    ASSERT_EQ_INT(TOK_ASSIGN, t->type);
    ASSERT_STR_EQ(":=", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_comparison_ops(void)
{
    TEST(tokenizer_comparison_ops);
    Token *t = tok("<>");
    ASSERT_EQ_INT(TOK_OP_COMPARISON, t->type);
    ASSERT_STR_EQ("<>", t->value);
    free_tokens(t);

    t = tok("<=");
    ASSERT_EQ_INT(TOK_OP_COMPARISON, t->type);
    ASSERT_STR_EQ("<=", t->value);
    free_tokens(t);

    t = tok(">=");
    ASSERT_EQ_INT(TOK_OP_COMPARISON, t->type);
    ASSERT_STR_EQ(">=", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_eol(void)
{
    TEST(tokenizer_eol);
    Token *t = tok("x\ny");
    Token *head = t;
    /* tokens: IDENT "x", EOL, IDENT "y", EOF */
    ASSERT_EQ_INT(TOK_IDENT, t->type);
    ASSERT_STR_EQ("x", t->value);
    t = t->next;
    ASSERT_EQ_INT(TOK_EOL, t->type);
    t = t->next;
    ASSERT_EQ_INT(TOK_IDENT, t->type);
    ASSERT_STR_EQ("y", t->value);
    free_tokens(head);
    PASS();
}

static void test_tokenizer_line_comment(void)
{
    TEST(tokenizer_line_comment);
    Token *t = tok("* this is a comment\n42");
    /* Comment consumed, first real token is 42 */
    ASSERT_EQ_INT(TOK_INTEGER, t->type);
    ASSERT_STR_EQ("42", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_inline_comment(void)
{
    TEST(tokenizer_inline_comment);
    Token *t = tok("x && inline comment");
    Token *head = t;
    /* && consumed as inline comment, only "x" remains (plus EOF) */
    ASSERT_EQ_INT(TOK_IDENT, t->type);
    ASSERT_STR_EQ("x", t->value);
    t = t->next;
    ASSERT_EQ_INT(TOK_EOF, t->type);
    free_tokens(head);
    PASS();
}

static void test_tokenizer_string_escape(void)
{
    TEST(tokenizer_string_escape);
    Token *t = tok("\"he\"\"llo\"");
    ASSERT_EQ_INT(TOK_STRING, t->type);
    ASSERT_STR_EQ("he\"llo", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_scientific_notation(void)
{
    TEST(tokenizer_scientific_notation);
    Token *t = tok("1.5e10");
    ASSERT_EQ_INT(TOK_REAL, t->type);
    ASSERT_STR_EQ("1.5e10", t->value);
    free_tokens(t);

    t = tok("2E+5");
    ASSERT_EQ_INT(TOK_REAL, t->type);
    ASSERT_STR_EQ("2E+5", t->value);
    free_tokens(t);
    PASS();
}

static void test_tokenizer_question_mark(void)
{
    TEST(tokenizer_question_mark);
    Token *t = tok("?");
    /* ? is tokenized as TOK_OP_ARITH with value "?" */
    ASSERT_EQ_INT(TOK_OP_ARITH, t->type);
    ASSERT_STR_EQ("?", t->value);
    free_tokens(t);
    PASS();
}

/* ------------------------------------------------------------------ */
/* ExprValue tests                                                      */
/* ------------------------------------------------------------------ */

static void test_val_constructors(void)
{
    TEST(val_constructors);
    ExprValue v;

    v = val_integer(42);
    ASSERT_EQ_INT(VAL_INTEGER, v.type);
    ASSERT_EQ_DBL(42.0, v.data.rval, 0.001);

    v = val_real(3.14);
    ASSERT_EQ_INT(VAL_REAL, v.type);
    ASSERT_EQ_DBL(3.14, v.data.rval, 0.001);

    v = val_string("hello");
    ASSERT_EQ_INT(VAL_STRING, v.type);
    ASSERT_STR_EQ("hello", v.data.sval);
    free_value(&v);

    v = val_logical(1);
    ASSERT_EQ_INT(VAL_LOGICAL, v.type);
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);

    v = val_null();
    ASSERT_EQ_INT(VAL_NULL, v.type);
    PASS();
}

static void test_val_coercion_to_double(void)
{
    TEST(val_coercion_to_double);
    ExprValue v;

    v = val_integer(42);
    ASSERT_EQ_DBL(42.0, val_to_double(&v), 0.001);

    v = val_string("123.5");
    ASSERT_EQ_DBL(123.5, val_to_double(&v), 0.001);
    free_value(&v);

    v = val_logical(1);
    ASSERT_EQ_DBL(1.0, val_to_double(&v), 0.001);

    v = val_null();
    ASSERT_EQ_DBL(0.0, val_to_double(&v), 0.001);
    PASS();
}

static void test_val_coercion_to_string(void)
{
    TEST(val_coercion_to_string);
    ExprValue v;
    char *s;

    v = val_integer(42);
    s = val_to_string(&v);
    ASSERT_STR_EQ("42", s);
    free(s);

    v = val_real(3.14);
    s = val_to_string(&v);
    ASSERT_STR_EQ("3.14", s);
    free(s);

    v = val_logical(1);
    s = val_to_string(&v);
    ASSERT_STR_EQ(".T.", s);
    free(s);

    v = val_logical(0);
    s = val_to_string(&v);
    ASSERT_STR_EQ(".F.", s);
    free(s);
    PASS();
}

static void test_val_comparison(void)
{
    TEST(val_comparison);
    ExprValue a, b;

    a = val_integer(1);
    b = val_integer(2);
    ASSERT_EQ_INT(-1, compare_values(&a, &b));

    a = val_integer(2);
    b = val_integer(1);
    ASSERT_EQ_INT(1, compare_values(&a, &b));

    a = val_integer(1);
    b = val_integer(1);
    ASSERT_EQ_INT(0, compare_values(&a, &b));

    a = val_string("abc");
    b = val_string("ABC");
    ASSERT_EQ_INT(0, compare_values(&a, &b));  /* case-insensitive */
    free_value(&a);
    free_value(&b);
    PASS();
}

static void test_val_copy(void)
{
    TEST(val_copy);
    ExprValue orig = val_string("hello");
    ExprValue copy = copy_value(&orig);
    ASSERT_EQ_INT(VAL_STRING, copy.type);
    ASSERT_STR_EQ("hello", copy.data.sval);
    free_value(&orig);
    free_value(&copy);
    PASS();
}

/* ------------------------------------------------------------------ */
/* Variable store tests                                                 */
/* ------------------------------------------------------------------ */

static void test_vars_set_get(void)
{
    TEST(vars_set_get);
    // vars_init handled globally
    ExprValue v = val_integer(42);
    vars_set("x", &v);
    free_value(&v);

    ExprValue g = vars_get("x");
    ASSERT_EQ_INT(VAL_INTEGER, g.type);
    ASSERT_EQ_DBL(42.0, g.data.rval, 0.001);
    free_value(&g);
    // vars_shutdown handled globally
    PASS();
}

static void test_vars_case_insensitive(void)
{
    TEST(vars_case_insensitive);
    // vars_init handled globally
    ExprValue v = val_string("hello");
    vars_set("MyVar", &v);
    free_value(&v);

    ExprValue g = vars_get("myvar");
    ASSERT_EQ_INT(VAL_STRING, g.type);
    ASSERT_STR_EQ("hello", g.data.sval);
    free_value(&g);
    // vars_shutdown handled globally
    PASS();
}

static void test_vars_overwrite(void)
{
    TEST(vars_overwrite);
    // vars_init handled globally
    ExprValue v1 = val_integer(10);
    vars_set("x", &v1);
    free_value(&v1);

    ExprValue v2 = val_integer(20);
    vars_set("x", &v2);
    free_value(&v2);

    ExprValue g = vars_get("x");
    ASSERT_EQ_DBL(20.0, g.data.rval, 0.001);
    free_value(&g);
    // vars_shutdown handled globally
    PASS();
}

static void test_vars_missing(void)
{
    TEST(vars_missing);
    // vars_init handled globally
    ExprValue g = vars_get("nonexistent");
    ASSERT_EQ_INT(VAL_NULL, g.type);
    free_value(&g);
    // vars_shutdown handled globally
    PASS();
}

static void test_vars_exists(void)
{
    TEST(vars_exists);
    // vars_init handled globally
    ExprValue v = val_integer(1);
    vars_set("x", &v);
    free_value(&v);

    ASSERT_EQ_INT(1, vars_exists("x"));
    ASSERT_EQ_INT(1, vars_exists("X"));  /* case-insensitive */
    ASSERT_EQ_INT(0, vars_exists("y"));
    // vars_shutdown handled globally
    PASS();
}

/* ------------------------------------------------------------------ */
/* Expression parser tests                                              */
/* ------------------------------------------------------------------ */

static void test_parse_integer_literal(void)
{
    TEST(parse_integer_literal);
    ExprValue v = parse("42");
    ASSERT_EQ_INT(VAL_INTEGER, v.type);
    ASSERT_EQ_DBL(42.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_real_literal(void)
{
    TEST(parse_real_literal);
    ExprValue v = parse("3.14");
    ASSERT_EQ_INT(VAL_REAL, v.type);
    ASSERT_EQ_DBL(3.14, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_string_literal(void)
{
    TEST(parse_string_literal);
    ExprValue v = parse("\"hello\"");
    ASSERT_EQ_INT(VAL_STRING, v.type);
    ASSERT_STR_EQ("hello", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_parse_logical_literal(void)
{
    TEST(parse_logical_literal);
    ExprValue v = parse(".T.");
    ASSERT_EQ_INT(VAL_LOGICAL, v.type);
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse(".F.");
    ASSERT_EQ_INT(VAL_LOGICAL, v.type);
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_addition(void)
{
    TEST(parse_addition);
    ExprValue v = parse("2 + 3");
    ASSERT_EQ_DBL(5.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_subtraction(void)
{
    TEST(parse_subtraction);
    ExprValue v = parse("10 - 3");
    ASSERT_EQ_DBL(7.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_multiplication(void)
{
    TEST(parse_multiplication);
    ExprValue v = parse("4 * 5");
    ASSERT_EQ_DBL(20.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_division(void)
{
    TEST(parse_division);
    ExprValue v = parse("10 / 4");
    ASSERT_EQ_DBL(2.5, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_division_by_zero(void)
{
    TEST(parse_division_by_zero);
    ExprValue v = parse("10 / 0");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);  /* dBase returns 0 */
    free_value(&v);
    PASS();
}

static void test_parse_modulo(void)
{
    TEST(parse_modulo);
    ExprValue v = parse("10 % 3");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_power(void)
{
    TEST(parse_power);
    ExprValue v = parse("2 ^ 10");
    ASSERT_EQ_DBL(1024.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_precedence(void)
{
    TEST(parse_precedence);
    /* 2 + 3 * 4 should be 14, not 20 */
    ExprValue v = parse("2 + 3 * 4");
    ASSERT_EQ_DBL(14.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_parentheses(void)
{
    TEST(parse_parentheses);
    /* (2 + 3) * 4 should be 20 */
    ExprValue v = parse("(2 + 3) * 4");
    ASSERT_EQ_DBL(20.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_unary_minus(void)
{
    TEST(parse_unary_minus);
    ExprValue v = parse("-5");
    ASSERT_EQ_DBL(-5.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_not(void)
{
    TEST(parse_not);
    ExprValue v = parse("NOT .T.");
    ASSERT_EQ_INT(VAL_LOGICAL, v.type);
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("NOT .F.");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_and(void)
{
    TEST(parse_and);
    ExprValue v = parse(".T. AND .T.");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse(".T. AND .F.");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_or(void)
{
    TEST(parse_or);
    ExprValue v = parse(".F. OR .T.");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse(".F. OR .F.");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_comparison_eq(void)
{
    TEST(parse_comparison_eq);
    ExprValue v = parse("5 = 5");
    ASSERT_EQ_INT(VAL_LOGICAL, v.type);
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("5 = 3");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_comparison_lt(void)
{
    TEST(parse_comparison_lt);
    ExprValue v = parse("3 < 5");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("5 < 3");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_comparison_gte(void)
{
    TEST(parse_comparison_gte);
    ExprValue v = parse("5 >= 5");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("5 >= 3");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("3 >= 5");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_comparison_neq(void)
{
    TEST(parse_comparison_neq);
    ExprValue v = parse("5 <> 3");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("5 <> 5");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_parse_string_concat(void)
{
    TEST(parse_string_concat);
    ExprValue v = parse("\"hello\" + \" world\"");
    ASSERT_EQ_INT(VAL_STRING, v.type);
    ASSERT_STR_EQ("hello world", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_parse_variable_ref(void)
{
    TEST(parse_variable_ref);
    // vars_init handled globally
    ExprValue v = val_integer(99);
    vars_set("x", &v);
    free_value(&v);

    ExprValue r = parse("x");
    ASSERT_EQ_INT(VAL_INTEGER, r.type);
    ASSERT_EQ_DBL(99.0, r.data.rval, 0.001);
    free_value(&r);
    // vars_shutdown handled globally
    PASS();
}

static void test_parse_variable_expr(void)
{
    TEST(parse_variable_expr);
    // vars_init handled globally
    ExprValue v = val_integer(10);
    vars_set("x", &v);
    free_value(&v);

    ExprValue r = parse("x + 5");
    ASSERT_EQ_DBL(15.0, r.data.rval, 0.001);
    free_value(&r);
    // vars_shutdown handled globally
    PASS();
}

/* ------------------------------------------------------------------ */
/* Built-in function tests                                              */
/* ------------------------------------------------------------------ */

static void test_builtin_len(void)
{
    TEST(builtin_len);
    ExprValue v = parse("LEN(\"hello\")");
    ASSERT_EQ_DBL(5.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_val(void)
{
    TEST(builtin_val);
    ExprValue v = parse("VAL(\"123.45\")");
    ASSERT_EQ_DBL(123.45, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_int(void)
{
    TEST(builtin_int);
    ExprValue v = parse("INT(3.7)");
    ASSERT_EQ_DBL(3.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_abs(void)
{
    TEST(builtin_abs);
    ExprValue v = parse("ABS(-5.5)");
    ASSERT_EQ_DBL(5.5, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_sqrt(void)
{
    TEST(builtin_sqrt);
    ExprValue v = parse("SQRT(16)");
    ASSERT_EQ_DBL(4.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_upper(void)
{
    TEST(builtin_upper);
    ExprValue v = parse("UPPER(\"hello\")");
    ASSERT_STR_EQ("HELLO", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_builtin_lower(void)
{
    TEST(builtin_lower);
    ExprValue v = parse("LOWER(\"HELLO\")");
    ASSERT_STR_EQ("hello", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_builtin_alltrim(void)
{
    TEST(builtin_alltrim);
    ExprValue v = parse("ALLTRIM(\"  hello  \")");
    ASSERT_STR_EQ("hello", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_builtin_substr(void)
{
    TEST(builtin_substr);
    ExprValue v = parse("SUBSTR(\"hello\", 2, 3)");
    ASSERT_STR_EQ("ell", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_builtin_left(void)
{
    TEST(builtin_left);
    ExprValue v = parse("LEFT(\"hello\", 3)");
    ASSERT_STR_EQ("hel", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_builtin_right(void)
{
    TEST(builtin_right);
    ExprValue v = parse("RIGHT(\"hello\", 3)");
    ASSERT_STR_EQ("llo", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_builtin_iif(void)
{
    TEST(builtin_iif);
    ExprValue v = parse("IIF(.T., 1, 0)");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("IIF(.F., 1, 0)");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_empty(void)
{
    TEST(builtin_empty);
    ExprValue v = parse("EMPTY(\"\")");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);  /* empty string is EMPTY */
    free_value(&v);

    v = parse("EMPTY(\"x\")");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("EMPTY(0)");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);  /* 0 is EMPTY */
    free_value(&v);
    PASS();
}

static void test_builtin_type(void)
{
    TEST(builtin_type);
    /* TYPE() evaluates its argument as an expression.
       TYPE("hello") → the argument is a string literal → "C" */
    ExprValue v = parse("TYPE(\"hello\")");
    ASSERT_STR_EQ("C", v.data.sval);  /* character */
    free_value(&v);

    v = parse("TYPE(42)");
    ASSERT_STR_EQ("N", v.data.sval);  /* numeric */
    free_value(&v);

    v = parse("TYPE(.T.)");
    ASSERT_STR_EQ("L", v.data.sval);  /* logical */
    free_value(&v);
    PASS();
}

static void test_builtin_at(void)
{
    TEST(builtin_at);
    ExprValue v = parse("AT(\"lo\", \"hello\")");
    ASSERT_EQ_DBL(4.0, v.data.rval, 0.001);  /* 1-indexed */
    free_value(&v);

    v = parse("AT(\"x\", \"hello\")");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);  /* not found */
    free_value(&v);
    PASS();
}

static void test_builtin_between(void)
{
    TEST(builtin_between);
    ExprValue v = parse("BETWEEN(5, 1, 10)");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("BETWEEN(15, 1, 10)");
    ASSERT_EQ_DBL(0.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_round(void)
{
    TEST(builtin_round);
    ExprValue v = parse("ROUND(3.14159, 2)");
    ASSERT_EQ_DBL(3.14, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_max_min(void)
{
    TEST(builtin_max_min);
    ExprValue v = parse("MAX(3, 7)");
    ASSERT_EQ_DBL(7.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("MIN(3, 7)");
    ASSERT_EQ_DBL(3.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_builtin_date(void)
{
    TEST(builtin_date);
    ExprValue v = parse("DATE()");
    ASSERT_EQ_INT(VAL_DATE, v.type);
    /* Stub returns "2026-07-26" */
    ASSERT_STR_EQ("2026-07-26", v.data.dval);
    free_value(&v);
    PASS();
}

static void test_builtin_dtoc(void)
{
    TEST(builtin_dtoc);
    ExprValue v = parse("DTOC({^2006-07-26})");
    ASSERT_EQ_INT(VAL_STRING, v.type);
    ASSERT_STR_EQ("2006-07-26", v.data.sval);
    free_value(&v);
    PASS();
}

static void test_builtin_ctod(void)
{
    TEST(builtin_ctod);
    ExprValue v = parse("CTOD(\"2006-07-26\")");
    ASSERT_EQ_INT(VAL_DATE, v.type);
    ASSERT_STR_EQ("2006-07-26", v.data.dval);
    free_value(&v);
    PASS();
}

static void test_builtin_day_month_year(void)
{
    TEST(builtin_day_month_year);
    ExprValue v = parse("DAY({^2006-07-26})");
    ASSERT_EQ_DBL(26.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("MONTH({^2006-07-26})");
    ASSERT_EQ_DBL(7.0, v.data.rval, 0.001);
    free_value(&v);

    v = parse("YEAR({^2006-07-26})");
    ASSERT_EQ_DBL(2006.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

/* ------------------------------------------------------------------ */
/* Executor tests                                                       */
/* ------------------------------------------------------------------ */

static void test_exec_assignment(void)
{
    TEST(exec_assignment);
    // vars_init handled globally
    run("x = 42");
    ExprValue v = vars_get("x");
    ASSERT_EQ_INT(VAL_INTEGER, v.type);
    ASSERT_EQ_DBL(42.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_assignment_colon_eq(void)
{
    TEST(exec_assignment_colon_eq);
    // vars_init handled globally
    run("x := 99");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(99.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_if_true(void)
{
    TEST(exec_if_true);
    // vars_init handled globally
    run("IF .T.\n"
        "x = 1\n"
        "ENDIF");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_if_false(void)
{
    TEST(exec_if_false);
    vars_shutdown(); vars_init();
    run("IF .F.\n"
        "x = 1\n"
        "ENDIF");
    ExprValue v = vars_get("x");
    ASSERT_EQ_INT(VAL_NULL, v.type);  /* x was never set */
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_if_else(void)
{
    TEST(exec_if_else);
    // vars_init handled globally
    run("IF .F.\n"
        "x = 1\n"
        "ELSE\n"
        "x = 2\n"
        "ENDIF");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(2.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_nested_if(void)
{
    TEST(exec_nested_if);
    // vars_init handled globally
    run("IF .T.\n"
        "  IF .T.\n"
        "    x = 42\n"
        "  ENDIF\n"
        "ENDIF");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(42.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_do_while(void)
{
    TEST(exec_do_while);
    // vars_init handled globally
    run("x = 0\n"
        "DO WHILE x < 3\n"
        "  x = x + 1\n"
        "ENDDO");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(3.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_for_loop(void)
{
    TEST(exec_for_loop);
    // vars_init handled globally
    run("s = 0\n"
        "FOR i = 1 TO 5\n"
        "  s = s + i\n"
        "ENDFOR\n");
    ExprValue v = vars_get("s");
    ASSERT_EQ_DBL(15.0, v.data.rval, 0.001);  /* 1+2+3+4+5 */
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_for_step(void)
{
    TEST(exec_for_step);
    // vars_init handled globally
    run("s = 0\n"
        "FOR i = 0 TO 10 STEP 2\n"
        "  s = s + 1\n"
        "ENDFOR\n");
    ExprValue v = vars_get("s");
    ASSERT_EQ_DBL(6.0, v.data.rval, 0.001);  /* 0,2,4,6,8,10 = 6 iterations */
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_return(void)
{
    TEST(exec_return);
    // vars_init handled globally
    ExecStatus st = run("x = 1\n"
                        "RETURN\n"
                        "x = 2");
    ASSERT_EQ_INT(EXEC_RETURN, st);
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_multiple_assignments(void)
{
    TEST(exec_multiple_assignments);
    // vars_init handled globally
    run("a = 10\n"
        "b = 20\n"
        "c = a + b");
    ExprValue v = vars_get("c");
    ASSERT_EQ_DBL(30.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_string_assignment(void)
{
    TEST(exec_string_assignment);
    // vars_init handled globally
    run("name = \"Alice\"");
    ExprValue v = vars_get("name");
    ASSERT_EQ_INT(VAL_STRING, v.type);
    ASSERT_STR_EQ("Alice", v.data.sval);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_logical_assignment(void)
{
    TEST(exec_logical_assignment);
    // vars_init handled globally
    run("flag = .T.");
    ExprValue v = vars_get("flag");
    ASSERT_EQ_INT(VAL_LOGICAL, v.type);
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_date_assignment(void)
{
    TEST(exec_date_assignment);
    // vars_init handled globally
    run("d = {^2006-07-26}");
    ExprValue v = vars_get("d");
    ASSERT_EQ_INT(VAL_DATE, v.type);
    ASSERT_STR_EQ("2006-07-26", v.data.dval);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_if_with_expression(void)
{
    TEST(exec_if_with_expression);
    // vars_init handled globally
    run("x = 5\n"
        "IF x > 3\n"
        "  y = 1\n"
        "ENDIF");
    ExprValue v = vars_get("y");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_do_while_with_exit(void)
{
    TEST(exec_do_while_with_exit);
    // vars_init handled globally
    run("x = 0\n"
        "DO WHILE .T.\n"
        "  x = x + 1\n"
        "  IF x >= 5\n"
        "    EXIT\n"
        "  ENDIF\n"
        "ENDDO");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(5.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_for_with_exit(void)
{
    TEST(exec_for_with_exit);
    // vars_init handled globally
    run("s = 0\n"
        "FOR i = 1 TO 100\n"
        "  IF i > 3\n"
        "    EXIT\n"
        "  ENDIF\n"
        "  s = s + i\n"
        "ENDFOR\n");
    ExprValue v = vars_get("s");
    ASSERT_EQ_DBL(6.0, v.data.rval, 0.001);  /* 1+2+3 */
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_set_command(void)
{
    TEST(exec_set_command);
    // vars_init handled globally
    ExecStatus st = run("SET TALK OFF");
    ASSERT_EQ_INT(EXEC_OK, st);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_skip_command(void)
{
    TEST(exec_skip_command);
    // vars_init handled globally
    ExecStatus st = run("SKIP 5");
    ASSERT_EQ_INT(EXEC_OK, st);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_gotop(void)
{
    TEST(exec_gotop);
    // vars_init handled globally
    ExecStatus st = run("GO TOP");
    ASSERT_EQ_INT(EXEC_OK, st);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_gobottom(void)
{
    TEST(exec_gobottom);
    // vars_init handled globally
    ExecStatus st = run("GO BOTTOM");
    ASSERT_EQ_INT(EXEC_OK, st);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_cancel(void)
{
    TEST(exec_cancel);
    // vars_init handled globally
    ExecStatus st = run("x = 1\n"
                        "CANCEL\n"
                        "x = 2");
    ASSERT_EQ_INT(EXEC_CANCEL, st);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_complex_program(void)
{
    TEST(exec_complex_program);
    vars_shutdown(); vars_init();
    run("n = 10\n"
        "s = 0\n"
        "FOR i = 1 TO n\n"
        "  s = s + i\n"
        "ENDFOR\n"
        "IF s = 55\n"
        "  result = .T.\n"
        "ELSE\n"
        "  result = .F.\n"
        "ENDIF");
    ExprValue v = vars_get("result");
    ASSERT_EQ_INT(VAL_LOGICAL, v.type);
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);  /* sum 1..10 = 55 */
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_function_in_program(void)
{
    TEST(exec_function_in_program);
    // vars_init handled globally
    run("x = LEN(\"hello\")");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(5.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_builtin_in_if(void)
{
    TEST(exec_builtin_in_if);
    // vars_init handled globally
    run("IF EMPTY(\"\")\n"
        "  x = 1\n"
        "ENDIF");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    // vars_shutdown handled globally
    PASS();
}

static void test_exec_procedure_call(void)
{
    TEST(exec_procedure_call);
    vars_shutdown(); vars_init();
    run("DO SayHi\n"
        "? x\n"
        "\n"
        "PROCEDURE SayHi\n"
        "x = 42\n"
        "RETURN");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(42.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_procedure_nested(void)
{
    TEST(exec_procedure_nested);
    vars_shutdown(); vars_init();
    run("x = 0\n"
        "DO Outer\n"
        "\n"
        "PROCEDURE Outer\n"
        "x = x + 1\n"
        "DO Inner\n"
        "RETURN\n"
        "\n"
        "PROCEDURE Inner\n"
        "x = x + 10\n"
        "RETURN");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(11.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_procedure_multiple_calls(void)
{
    TEST(exec_procedure_multiple_calls);
    vars_shutdown(); vars_init();
    run("x = 0\n"
        "DO Inc\n"
        "DO Inc\n"
        "DO Inc\n"
        "\n"
        "PROCEDURE Inc\n"
        "x = x + 1\n"
        "RETURN");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(3.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_procedure_in_loop(void)
{
    TEST(exec_procedure_in_loop);
    vars_shutdown(); vars_init();
    run("x = 0\n"
        "DO WHILE x < 3\n"
        "  DO Inc\n"
        "ENDDO\n"
        "\n"
        "PROCEDURE Inc\n"
        "x = x + 1\n"
        "RETURN");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(3.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_return_from_main(void)
{
    TEST(exec_return_from_main);
    vars_shutdown(); vars_init();
    run("x = 1\n"
        "RETURN\n"
        "x = 2");
    ExprValue v = vars_get("x");
    ASSERT_EQ_DBL(1.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_parameters_basic(void)
{
    TEST(exec_parameters_basic);
    vars_shutdown(); vars_init();
    run("a = 10\n"
        "b = 20\n"
        "DO AddEm WITH a, b\n"
        "\n"
        "PROCEDURE AddEm\n"
        "PARAMETERS x, y\n"
        "r = x + y\n"
        "RETURN");
    ExprValue v = vars_get("r");
    ASSERT_EQ_DBL(30.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_parameters_expression(void)
{
    TEST(exec_parameters_expression);
    vars_shutdown(); vars_init();
    run("DO Calc WITH 3 + 4, 5 * 2\n"
        "\n"
        "PROCEDURE Calc\n"
        "PARAMETERS a, b\n"
        "res = a * b\n"
        "RETURN");
    ExprValue v = vars_get("res");
    ASSERT_EQ_DBL(70.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_parameters_nested(void)
{
    TEST(exec_parameters_nested);
    vars_shutdown(); vars_init();
    run("DO Outer WITH 5\n"
        "\n"
        "PROCEDURE Outer\n"
        "PARAMETERS a\n"
        "DO Inner WITH a + 10, a * 2\n"
        "RETURN\n"
        "\n"
        "PROCEDURE Inner\n"
        "PARAMETERS x, y\n"
        "res = x + y\n"
        "RETURN");
    ExprValue v = vars_get("res");
    ASSERT_EQ_DBL(25.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_parameters_fewer_than_args(void)
{
    TEST(exec_parameters_fewer_than_args);
    vars_shutdown(); vars_init();
    run("DO Test WITH 1, 2, 3\n"
        "\n"
        "PROCEDURE Test\n"
        "PARAMETERS a, b\n"
        "res = a + b\n"
        "RETURN");
    ExprValue v = vars_get("res");
    ASSERT_EQ_DBL(3.0, v.data.rval, 0.001);
    free_value(&v);
    PASS();
}

static void test_exec_parameters_string(void)
{
    TEST(exec_parameters_string);
    vars_shutdown(); vars_init();
    run("DO Test WITH \"hello\", \" world\"\n"
        "\n"
        "PROCEDURE Test\n"
        "PARAMETERS s1, s2\n"
        "res = s1 + s2\n"
        "RETURN");
    ExprValue v = vars_get("res");
    char *s = val_to_string(&v);
    ASSERT_STR_EQ("hello world", s);
    free(s);
    free_value(&v);
    PASS();
}

/* ------------------------------------------------------------------ */
/* Main                                                                 */
/* ------------------------------------------------------------------ */

static void test_wa_use_close(void)
{
    TEST(wa_use_close);
    wa_close_all();
    int rc = wa_use("stress/books.dbf", -1);
    ASSERT_EQ_INT(0, rc);
    DATABASEDBF *db = wa_db();
    if (db == NULL) { FAIL("db should not be NULL"); return; }
    wa_close_all();
    db = wa_db();
    if (db != NULL) { FAIL("db should be NULL after close_all"); return; }
    PASS();
}

static void test_wa_navigation(void)
{
    TEST(wa_navigation);
    wa_close_all();
    wa_use("stress/books.dbf", -1);
    int total = wa_reccount();
    if (total <= 0) { FAIL("expected records > 0"); wa_close_all(); return; }

    wa_goto_top();
    ASSERT_EQ_INT(1, wa_recno());

    wa_goto_bottom();
    ASSERT_EQ_INT(total, wa_recno());

    wa_skip(-1);
    ASSERT_EQ_INT(total - 1, wa_recno());

    wa_close_all();
    PASS();
}

static void test_wa_status(void)
{
    TEST(wa_status);
    wa_close_all();
    wa_use("stress/books.dbf", -1);

    char *name = wa_dbf_name();
    if (name == NULL) { FAIL("alias should not be NULL"); free(name); wa_close_all(); return; }
    free(name);

    wa_goto_top();
    ASSERT_EQ_INT(1, wa_bof());

    ASSERT_EQ_INT(0, wa_eof());

    wa_close_all();
    PASS();
}

int main(void)
{
    vars_init();
    wa_init();
    printf("=== dBase III Plus .PRG Interpreter Tests ===\n\n");

    printf("--- Tokenizer ---\n");
    test_tokenizer_integer();
    test_tokenizer_real();
    test_tokenizer_string();
    test_tokenizer_logical_true();
    test_tokenizer_logical_false();
    test_tokenizer_logical_y_n();
    test_tokenizer_date_literal();
    test_tokenizer_date_no_caret();
    test_tokenizer_keyword_if();
    test_tokenizer_keyword_while();
    test_tokenizer_assignment_colon_eq();
    test_tokenizer_comparison_ops();
    test_tokenizer_eol();
    test_tokenizer_line_comment();
    test_tokenizer_inline_comment();
    test_tokenizer_string_escape();
    test_tokenizer_scientific_notation();
    test_tokenizer_question_mark();

    printf("\n--- ExprValue ---\n");
    test_val_constructors();
    test_val_coercion_to_double();
    test_val_coercion_to_string();
    test_val_comparison();
    test_val_copy();

    printf("\n--- Variables ---\n");
    test_vars_set_get();
    test_vars_case_insensitive();
    test_vars_overwrite();
    test_vars_missing();
    test_vars_exists();

    printf("\n--- Expression Parser ---\n");
    test_parse_integer_literal();
    test_parse_real_literal();
    test_parse_string_literal();
    test_parse_logical_literal();
    test_parse_addition();
    test_parse_subtraction();
    test_parse_multiplication();
    test_parse_division();
    test_parse_division_by_zero();
    test_parse_modulo();
    test_parse_power();
    test_parse_precedence();
    test_parse_parentheses();
    test_parse_unary_minus();
    test_parse_not();
    test_parse_and();
    test_parse_or();
    test_parse_comparison_eq();
    test_parse_comparison_lt();
    test_parse_comparison_gte();
    test_parse_comparison_neq();
    test_parse_string_concat();
    test_parse_variable_ref();
    test_parse_variable_expr();

    printf("\n--- Built-in Functions ---\n");
    test_builtin_len();
    test_builtin_val();
    test_builtin_int();
    test_builtin_abs();
    test_builtin_sqrt();
    test_builtin_upper();
    test_builtin_lower();
    test_builtin_alltrim();
    test_builtin_substr();
    test_builtin_left();
    test_builtin_right();
    test_builtin_iif();
    test_builtin_empty();
    test_builtin_type();
    test_builtin_at();
    test_builtin_between();
    test_builtin_round();
    test_builtin_max_min();
    test_builtin_date();
    test_builtin_dtoc();
    test_builtin_ctod();
    test_builtin_day_month_year();

    printf("\n--- Executor ---\n");
    vars_shutdown(); vars_init();  /* Clean slate for executor tests */
    test_exec_assignment();
    test_exec_assignment_colon_eq();
    test_exec_if_true();
    test_exec_if_false();
    test_exec_if_else();
    test_exec_nested_if();
    test_exec_do_while();
    test_exec_for_loop();
    test_exec_for_step();
    test_exec_return();
    test_exec_multiple_assignments();
    test_exec_string_assignment();
    test_exec_logical_assignment();
    test_exec_date_assignment();
    test_exec_if_with_expression();
    test_exec_do_while_with_exit();
    test_exec_for_with_exit();
    test_exec_set_command();
    test_exec_skip_command();
    test_exec_gotop();
    test_exec_gobottom();
    test_exec_cancel();
    test_exec_complex_program();
    test_exec_function_in_program();
    test_exec_builtin_in_if();
    test_exec_procedure_call();
    test_exec_procedure_nested();
    test_exec_procedure_multiple_calls();
    test_exec_procedure_in_loop();
    test_exec_return_from_main();
    test_exec_parameters_basic();
    test_exec_parameters_expression();
    test_exec_parameters_nested();
    test_exec_parameters_fewer_than_args();
    test_exec_parameters_string();

    printf("\n--- Workarea ---\n");
    test_wa_use_close();
    test_wa_navigation();
    test_wa_status();

    printf("\n=== Results: %d/%d passed, %d failed ===\n",
           tests_passed, tests_run, tests_failed);

    wa_shutdown();
    vars_shutdown();
    return tests_failed ? 1 : 0;
}
