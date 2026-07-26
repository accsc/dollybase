/*
 * executor.c — dBase III Plus statement executor
 *
 * Walks a token list and dispatches statements: IF/ELSE/ENDIF, DO WHILE/ENDDO,
 * FOR/NEXT, assignment, ?, RETURN, SET, SKIP, USE, GO TOP/BOTTOM.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "executor.h"
#include "parser.h"
#include "variables.h"

/* ------------------------------------------------------------------ */
/* Portable case-insensitive string compare                            */
/* ------------------------------------------------------------------ */

static int port_strcasecmp(const char *a, const char *b)
{
    while (*a && *b) {
        unsigned char ca = (unsigned char)*a;
        unsigned char cb = (unsigned char)*b;
        if ('A' <= ca && ca <= 'Z') ca += 'a' - 'A';
        if ('A' <= cb && cb <= 'Z') cb += 'a' - 'A';
        if (ca != cb) return (int)(ca - cb);
        a++; b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

/* ------------------------------------------------------------------ */
/* Forward declarations                                                */
/* ------------------------------------------------------------------ */

static ExecStatus exec_statement(Token **cur);
static int is_eol_or_eof(const Token *t);
static void skip_to_eol(Token **cur);

/* Statement handlers called from the dispatcher before their definitions */
static ExecStatus exec_if(Token **cur);
static ExecStatus exec_do_while(Token **cur);
static ExecStatus exec_for(Token **cur);
static ExecStatus exec_set(Token **cur);
static ExecStatus exec_skip(Token **cur);
static ExecStatus exec_use(Token **cur);
static ExecStatus exec_close(Token **cur);
static ExecStatus exec_go(Token **cur);
static ExecStatus exec_print(Token **cur);
static ExecStatus exec_assign(Token **cur);

/* Block / loop helpers */
static ExecStatus exec_block_until(Token **cur, KeywordId kw1, KeywordId kw2);
static void skip_block_nested(Token **cur, KeywordId end_kw, KeywordId else_kw);
static ExecStatus exec_do_body(Token **cur);
static ExecStatus exec_for_body(Token **cur);

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static ExprValue parse_expr(Token **cur)
{
    ParseError err;
    return parse_expression(cur, &err);
}

static int is_eol_or_eof(const Token *t)
{
    if (!t) return 1;
    return t->type == TOK_EOF || t->type == TOK_EOL;
}

static void skip_to_eol(Token **cur)
{
    while (*cur && !is_eol_or_eof(*cur)) {
        *cur = (*cur)->next;
    }
    if (*cur) *cur = (*cur)->next; /* consume the EOL/EOF */
}

/* ------------------------------------------------------------------ */
/* Main dispatch loop                                                  */
/* ------------------------------------------------------------------ */

ExecStatus execute_tokens(Token *tokens)
{
    Token *cur = tokens;
    while (cur) {
        /* Skip blank lines / EOL between statements */
        if (cur->type == TOK_EOL) {
            cur = cur->next;
            continue;
        }
        if (cur->type == TOK_EOF)
            break;

        fprintf(stderr, "DEBUG exec: dispatch at type=%d \"%s\"\n", cur->type, cur->value);
        ExecStatus st = exec_statement(&cur);
        fprintf(stderr, "DEBUG exec: after statement, cur type=%d \"%s\"\n",
                cur ? cur->type : -1, cur ? cur->value : "");
        if (st == EXEC_RETURN || st == EXEC_CANCEL)
            return st;

        /* After a simple statement, advance past any trailing EOL so the
           next iteration starts at the next real token. Block constructs
           (IF, DO WHILE, FOR) already consume their own boundaries. */
        if (cur && cur->type == TOK_EOL) {
            fprintf(stderr, "DEBUG exec: skipping EOL\n");
            cur = cur->next;
        }
    }
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* Statement dispatcher                                                */
/* ------------------------------------------------------------------ */

static ExecStatus exec_statement(Token **cur)
{
    Token *t = *cur;
    if (!t || t->type == TOK_EOF || t->type == TOK_EOL)
        return EXEC_OK;

    /* Keyword-driven dispatch */
    if (t->type == TOK_KEYWORD) {
        switch (t->keyword_id) {
            case KW_IF:       return exec_if(cur);
            case KW_DO:       return exec_do_while(cur);
            case KW_FOR:      return exec_for(cur);
            case KW_RETURN:   (*cur) = (*cur)->next; return EXEC_RETURN;
            case KW_CANCEL:   (*cur) = (*cur)->next; return EXEC_CANCEL;
            case KW_SET:      return exec_set(cur);
            case KW_SKIP:     return exec_skip(cur);
            case KW_USE:      return exec_use(cur);
            case KW_CLOSE:    return exec_close(cur);
            case KW_GOTOP:    (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_OK;
            case KW_GOBOTTOM: (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_OK;
            case KW_GO:       return exec_go(cur);
            default:
                /* Unknown keyword — treat as expression or skip */
                break;
        }
    }

    /* ? and ?? (print commands) — tokenized as TOK_OP_ARITH with "?" value */
    if (t->type == TOK_OP_ARITH && strcmp(t->value, "?") == 0) {
        return exec_print(cur);
    }

    /* Assignment: <ident> = <expr> or <ident> := <expr> */
    if (t->type == TOK_IDENT) {
        return exec_assign(cur);
    }

    /* Fallback: skip to end of line */
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* IF / ELSE / ENDIF                                                   */
/* ------------------------------------------------------------------ */

static ExecStatus exec_if(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "IF" */

    ExprValue cond = parse_expr(cur);
    int truthy = val_to_logical(&cond);
    free_value(&cond);

    if (truthy) {
        return exec_block_until(cur, KW_ENDIF, KW_ELSE);
    } else {
        /* Skip the true branch to ELSE or ENDIF */
        skip_block_nested(cur, KW_ENDIF, KW_ELSE);

        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ELSE) {
            *cur = (*cur)->next; /* skip "ELSE" */
            return exec_block_until(cur, KW_ENDIF, 0);
        }
        /* Already past ENDIF or at EOF — nothing more to do */
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDIF) {
            *cur = (*cur)->next; /* skip "ENDIF" */
        }
    }

    return EXEC_OK;
}

/* Execute statements until we hit one of the sentinel keywords.
   If kw2 is 0, only kw1 is a sentinel. */
static ExecStatus exec_block_until(Token **cur, KeywordId kw1, KeywordId kw2)
{
    while (*cur && !is_eol_or_eof(*cur)) {
        /* Check for sentinel */
        if ((*cur)->type == TOK_KEYWORD) {
            if ((*cur)->keyword_id == kw1 || (kw2 && (*cur)->keyword_id == kw2))
                break;
            /* Nested IF/DO WHILE/FOR — we still execute them normally,
               they consume their own blocks */
        }

        ExecStatus st = exec_statement(cur);
        if (st == EXEC_RETURN || st == EXEC_CANCEL)
            return st;
    }

    /* Consume the sentinel keyword itself */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == kw1) {
        *cur = (*cur)->next;
    }

    return EXEC_OK;
}

/* Skip tokens until we hit a sentinel, respecting nesting. */
static void skip_block_nested(Token **cur, KeywordId end_kw, KeywordId else_kw)
{
    int depth = 1;
    while (*cur && !is_eol_or_eof(*cur)) {
        if ((*cur)->type == TOK_KEYWORD) {
            if ((*cur)->keyword_id == KW_IF || (*cur)->keyword_id == KW_DO ||
                (*cur)->keyword_id == KW_FOR) {
                depth++;
            } else if ((*cur)->keyword_id == end_kw ||
                       (else_kw && (*cur)->keyword_id == else_kw)) {
                depth--;
                if (depth <= 0) break;
            }
        }
        *cur = (*cur)->next;
    }
    /* Consume the sentinel */
    if (*cur && !is_eol_or_eof(*cur)) {
        *cur = (*cur)->next;
    }
}

/* ------------------------------------------------------------------ */
/* DO WHILE / ENDDO                                                    */
/* ------------------------------------------------------------------ */

static ExecStatus exec_do_while(Token **cur)
{
    /* DO WHILE <condition> */
    *cur = (*cur)->next; /* skip "DO" */
    if (*cur && match_keyword(cur, KW_WHILE)) {
        /* ok */
    } else {
        /* bare "DO <program>" — not implemented yet, skip line */
        skip_to_eol(cur);
        return EXEC_OK;
    }

    ExprValue cond = parse_expr(cur);
    int truthy = val_to_logical(&cond);
    free_value(&cond);

    while (truthy) {
        /* Execute body until ENDDO, EXIT, or LOOP */
        ExecStatus st = exec_do_body(cur);

        if (st == EXEC_EXIT || st == EXEC_RETURN || st == EXEC_CANCEL) {
            return st;
        }

        /* Re-evaluate condition */
        cond = parse_expr(cur);
        truthy = val_to_logical(&cond);
        free_value(&cond);
    }

    /* Consume ENDDO if present (exec_do_body may have left us at it) */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDDO) {
        *cur = (*cur)->next;
    }

    return EXEC_OK;
}

/* Execute the body of a DO WHILE loop. Stops at ENDDO, EXIT, or LOOP. */
static ExecStatus exec_do_body(Token **cur)
{
    while (*cur && !is_eol_or_eof(*cur)) {
        if ((*cur)->type == TOK_KEYWORD) {
            KeywordId kw = (*cur)->keyword_id;

            if (kw == KW_ENDDO) {
                *cur = (*cur)->next; /* consume ENDDO */
                return EXEC_OK;
            }
            if (kw == KW_EXIT) {
                *cur = (*cur)->next; /* consume EXIT */
                skip_to_eol(cur);
                /* Leave cur at or past ENDDO — caller handles it */
                while (*cur && !is_eol_or_eof(*cur) &&
                       !((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDDO)) {
                    *cur = (*cur)->next;
                }
                if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDDO) {
                    *cur = (*cur)->next;
                }
                return EXEC_EXIT;
            }
            if (kw == KW_LOOP) {
                *cur = (*cur)->next; /* consume LOOP */
                skip_to_eol(cur);
                return EXEC_OK; /* fall through to condition re-check */
            }
        }

        ExecStatus st = exec_statement(cur);
        if (st != EXEC_OK) return st;
    }
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* FOR / NEXT                                                          */
/* ------------------------------------------------------------------ */

static ExecStatus exec_for(Token **cur)
{
    /* FOR <var> = <start> TO <end> [STEP <step>] */
    *cur = (*cur)->next; /* skip "FOR" */

    if (!*cur || (*cur)->type != TOK_IDENT) {
        skip_to_eol(cur);
        return EXEC_OK;
    }
    char varname[256];
    strncpy(varname, (*cur)->value, sizeof(varname) - 1);
    varname[sizeof(varname) - 1] = '\0';
    *cur = (*cur)->next; /* skip variable name */

    /* Skip '=' or ':=' */
    if (*cur && (*cur)->type == TOK_OP_COMPARISON && strcmp((*cur)->value, "=") == 0) {
        *cur = (*cur)->next;
    } else if (*cur && (*cur)->type == TOK_ASSIGN) {
        *cur = (*cur)->next;
    }

    ExprValue start_val = parse_expr(cur);
    double start_d = val_to_double(&start_val);
    free_value(&start_val);

    /* Skip "TO" — it may be an ident or keyword depending on tokenizer */
    if (*cur && (*cur)->type == TOK_IDENT && port_strcasecmp((*cur)->value, "to") == 0) {
        *cur = (*cur)->next;
    } else if (*cur && match_keyword(cur, KW_STEP)) {
        /* STEP used as TO? fallback */
    }

    ExprValue end_val = parse_expr(cur);
    double end_d = val_to_double(&end_val);
    free_value(&end_val);

    double step = 1.0;
    if (*cur && match_keyword(cur, KW_STEP)) {
        ExprValue sv = parse_expr(cur);
        step = val_to_double(&sv);
        free_value(&sv);
    }

    /* Initialize loop variable */
    { ExprValue v = val_real(start_d); vars_set(varname, &v); }

    double cur_val = start_d;
    while (1) {
        int done = 0;
        if (step > 0 && cur_val > end_d + 0.0001) done = 1;
        if (step < 0 && cur_val < end_d - 0.0001) done = 1;
        /* Handle exact equality for zero-step edge */
        if (step == 0) break;

        if (done) break;

        { ExprValue v = val_real(cur_val); vars_set(varname, &v); }

        ExecStatus st = exec_for_body(cur);
        if (st == EXEC_EXIT || st == EXEC_RETURN || st == EXEC_CANCEL) {
            return st;
        }

        cur_val += step;
    }

    /* Consume NEXT */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT) {
        *cur = (*cur)->next;
    }

    return EXEC_OK;
}

static ExecStatus exec_for_body(Token **cur)
{
    while (*cur && !is_eol_or_eof(*cur)) {
        if ((*cur)->type == TOK_KEYWORD) {
            KeywordId kw = (*cur)->keyword_id;
            if (kw == KW_NEXT) {
                *cur = (*cur)->next;
                return EXEC_OK;
            }
            if (kw == KW_EXIT) {
                *cur = (*cur)->next;
                skip_to_eol(cur);
                while (*cur && !is_eol_or_eof(*cur) &&
                       !((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT)) {
                    *cur = (*cur)->next;
                }
                if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT) {
                    *cur = (*cur)->next;
                }
                return EXEC_EXIT;
            }
        }

        ExecStatus st = exec_statement(cur);
        if (st != EXEC_OK) return st;
    }
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* Assignment                                                          */
/* ------------------------------------------------------------------ */

static ExecStatus exec_assign(Token **cur)
{
    char varname[256];
    strncpy(varname, (*cur)->value, sizeof(varname) - 1);
    varname[sizeof(varname) - 1] = '\0';
    *cur = (*cur)->next; /* skip variable name */

    /* Skip '=' or ':=' */
    if (*cur && (*cur)->type == TOK_OP_COMPARISON && strcmp((*cur)->value, "=") == 0) {
        *cur = (*cur)->next;
    } else if (*cur && (*cur)->type == TOK_ASSIGN) {
        *cur = (*cur)->next;
    }

    ExprValue val = parse_expr(cur);
    vars_set(varname, &val);
    free_value(&val);

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* ? (print)                                                           */
/* ------------------------------------------------------------------ */

static ExecStatus exec_print(Token **cur)
{
    int add_newline = 1; /* default: ? adds newline, ?? does not */
    char *val_str = (*cur)->value;
    if (val_str && strlen(val_str) == 2 && val_str[0] == '?' && val_str[1] == '?') {
        add_newline = 0;
    }

    *cur = (*cur)->next; /* skip ? or ?? */

    int first = 1;
    while (*cur && !is_eol_or_eof(*cur)) {
        ExprValue val = parse_expr(cur);
        char *s = val_to_string(&val);
        if (!first) printf(" ");
        printf("%s", s);
        free(s);
        free_value(&val);
        first = 0;

        /* Skip comma separator */
        if (*cur && (*cur)->type == TOK_COMMA) {
            *cur = (*cur)->next;
        } else {
            break;
        }
    }

    if (add_newline) printf("\n");
    fflush(stdout);

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* SET command                                                         */
/* ------------------------------------------------------------------ */

static ExecStatus exec_set(Token **cur)
{
    /* SET <setting> ON|OFF or SET <setting> TO <value> */
    (*cur) = (*cur)->next; /* skip "SET" */

    if (!*cur || (*cur)->type != TOK_KEYWORD) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    KeywordId setting = (*cur)->keyword_id;
    (*cur) = (*cur)->next;

    /* Read ON/OFF or TO <value> */
    if (*cur && (*cur)->type == TOK_IDENT) {
        const char *flag = (*cur)->value;
        int on = (port_strcasecmp(flag, "ON") == 0);
        (void)on; /* settings not yet wired to config */
        (*cur) = (*cur)->next;
    } else if (*cur && match_keyword(cur, KW_STEP)) {
        /* SET STEP ON/OFF — used for debugger */
        skip_to_eol(cur);
        return EXEC_OK;
    }

    skip_to_eol(cur);
    (void)setting; /* suppress unused warning until settings are wired */
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* SKIP                                                                */
/* ------------------------------------------------------------------ */

static ExecStatus exec_skip(Token **cur)
{
    /* SKIP [n] — advance record pointer; not implemented without DBF */
    (*cur) = (*cur)->next;
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* USE                                                                 */
/* ------------------------------------------------------------------ */

static ExecStatus exec_use(Token **cur)
{
    /* USE <filename> [EXCLUSIVE] ... — not implemented without libdbase link */
    (*cur) = (*cur)->next;
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* CLOSE                                                               */
/* ------------------------------------------------------------------ */

static ExecStatus exec_close(Token **cur)
{
    /* CLOSE DATABASES / ALL — not implemented without work areas */
    (*cur) = (*cur)->next;
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* GO                                                                  */
/* ------------------------------------------------------------------ */

static ExecStatus exec_go(Token **cur)
{
    /* GO <n> / GO TOP / GO BOTTOM — not implemented without DBF */
    (*cur) = (*cur)->next;
    skip_to_eol(cur);
    return EXEC_OK;
}
