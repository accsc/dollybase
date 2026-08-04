/*
 * executor.c — dollybase statement executor
 *
 * Walks a token list and dispatches statements: IF/ELSE/ENDIF, DO WHILE/ENDDO,
 * FOR/ENDFOR, assignment, ?, RETURN, SET, SKIP, USE, GO TOP/BOTTOM.
 * Supports PROCEDURE definitions, DO <name> calls, and RETURN with call stack.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>

#include "executor.h"
#include "parser.h"
#include "variables.h"
#include "exprvalue.h"
#include "workarea.h"
#include "ui.h"
#include "memfile.h"

/* Global SET flags */
int g_set_century = 1;  /* SET CENTURY ON by default (dBASE convention) */
int g_set_exact = 0;    /* SET EXACT OFF by default (dBASE convention)  */

/* SET ALTERNATE TO / SET CONSOLE state */
static FILE *g_alternate_file = NULL;   /* SET ALTERNATE TO "file" */
static int g_set_console = 1;           /* SET CONSOLE ON (default) */

static void alternate_write(const char *s)
{
    if (g_alternate_file && s) {
        fputs(s, g_alternate_file);
        fflush(g_alternate_file);
    }
}

void alternate_close(void)
{
    if (g_alternate_file) {
        fclose(g_alternate_file);
        g_alternate_file = NULL;
    }
}

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
/* Case-insensitive file resolution                                    */
/* ------------------------------------------------------------------ */
/*
 * resolve_file_path — find a file on disk ignoring case.
 *
 * Given a requested filename (e.g. "TAX.DBF" or "tours"), scans the
 * current directory for case-insensitive matches.
 *
 *   - Exactly 1 match  → copies the actual disk path into out[]
 *   - 0 matches        → copies a lowercase version into out[] (for error msg)
 *   - Multiple matches → prints an ambiguity error, copies lowercase into out[]
 *
 * Returns 1 if a unique file was found, 0 otherwise.
 * The 'out' buffer must be at least 'out_size' bytes.
 */

static int resolve_file_path(const char *requested, char *out, size_t out_size)
{
    DIR *dir = opendir(".");
    if (!dir) {
        /* Can't open dir — just lowercase and return */
        strncpy(out, requested, out_size - 1);
        out[out_size - 1] = '\0';
        for (char *p = out; *p; p++) *p = (char)tolower((unsigned char)*p);
        return 0;
    }

    /* Split requested into base name and extension */
    char req_name[512];
    char req_ext[512] = "";
    strncpy(req_name, requested, sizeof(req_name) - 1);
    req_name[sizeof(req_name) - 1] = '\0';

    char *dot = strrchr(req_name, '.');
    if (dot) {
        *dot = '\0';
        strcpy(req_ext, dot);  /* includes the dot */
    }

    /* Scan directory for matches */
    struct dirent *entry;
    char *matches[64];
    int match_count = 0;

    while ((entry = readdir(dir)) != NULL) {
        /* Skip . and .. */
        if (entry->d_name[0] == '.' && (entry->d_name[1] == '\0' ||
            (entry->d_name[1] == '.' && entry->d_name[2] == '\0')))
            continue;

        /* Split entry name into base + extension */
        char entry_name[512];
        char entry_ext[512] = "";
        strncpy(entry_name, entry->d_name, sizeof(entry_name) - 1);
        entry_name[sizeof(entry_name) - 1] = '\0';

        char *edot = strrchr(entry_name, '.');
        if (edot) {
            *edot = '\0';
            strcpy(entry_ext, edot);  /* includes the dot */
        }

        /* Compare name part (case-insensitive) */
        if (port_strcasecmp(entry_name, req_name) != 0)
            continue;

        /* If extension was specified, check it too (case-insensitive) */
        if (req_ext[0]) {
            if (entry_ext[0] == '\0' || port_strcasecmp(entry_ext, req_ext) != 0)
                continue;
        }

        if (match_count < 64) {
            matches[match_count++] = entry->d_name;
        }
    }
    closedir(dir);

    if (match_count == 1) {
        strncpy(out, matches[0], out_size - 1);
        out[out_size - 1] = '\0';
        return 1;
    }

    if (match_count > 1) {
        fprintf(stderr, "prg: ambiguous file '%s' (found %d matches):\n", requested, match_count);
        for (int i = 0; i < match_count && i < 64; i++) {
            fprintf(stderr, "  %s\n", matches[i]);
        }
    }

    /* No match or ambiguous — return lowercase version */
    strncpy(out, requested, out_size - 1);
    out[out_size - 1] = '\0';
    for (char *p = out; *p; p++) *p = (char)tolower((unsigned char)*p);
    return 0;
}

/*
 * force_lowercase — copy path to out, forcing all chars to lowercase.
 * Used for write paths (SAVE TO, SET ALTERNATE TO) so files are always
 * created with lowercase names on disk.
 */

static void force_lowercase(const char *path, char *out, size_t out_size)
{
    strncpy(out, path, out_size - 1);
    out[out_size - 1] = '\0';
    for (char *p = out; *p; p++) *p = (char)tolower((unsigned char)*p);
}

/* ------------------------------------------------------------------ */
/* Procedure registry                                                  */
/* ------------------------------------------------------------------ */

static Procedure procedures[MAX_PROCEDURES];
static int proc_count = 0;

/* Token lists loaded via SET PROCEDURE TO — kept alive for procedure bodies */
#define MAX_PROC_FILES 64
static Token *proc_file_tokens[MAX_PROC_FILES];
static int proc_file_count = 0;

void proc_registry_init(void)
{
    for (int i = 0; i < proc_file_count; i++) {
        free_tokens(proc_file_tokens[i]);
    }
    proc_file_count = 0;
    proc_count = 0;
    memset(procedures, 0, sizeof(procedures));
}

void proc_registry_add(const char *name, Token *start)
{
    if (proc_count >= MAX_PROCEDURES) return;
    strncpy(procedures[proc_count].name, name, sizeof(procedures[proc_count].name) - 1);
    procedures[proc_count].name[sizeof(procedures[proc_count].name) - 1] = '\0';
    procedures[proc_count].start = start;
    proc_count++;
}

const Procedure *proc_registry_lookup(const char *name)
{
    for (int i = 0; i < proc_count; i++) {
        if (port_strcasecmp(procedures[i].name, name) == 0)
            return &procedures[i];
    }
    return NULL;
}

/* Scan token list for PROCEDURE <name> definitions and register them. */
void proc_scan(Token *tokens)
{
    Token *cur = tokens;
    while (cur) {
        if (cur->type == TOK_KEYWORD && cur->keyword_id == KW_PROCEDURE) {
            /* Next token should be the procedure name (IDENT or keyword used as name) */
            Token *next = cur->next;
            if (next && next->type == TOK_IDENT) {
                /* Body starts after the name */
                Token *body = next->next;
                proc_registry_add(next->value, body);
            }
        }
        cur = cur->next;
    }
}

/* ------------------------------------------------------------------ */
/* Call stack for DO / RETURN                                          */
/* ------------------------------------------------------------------ */

#define MAX_CALL_DEPTH 32
#define MAX_PARAMS 32

typedef struct {
    Token *return_token;  /* Where to resume after RETURN               */
    Token *tokens;        /* The full token list (for cleanup on file DO) */
    int is_file;          /* 1 if loaded from external file (free tokens on return) */
    /* Parameters passed via DO ... WITH */
    ExprValue params[MAX_PARAMS];
    int param_count;
} CallFrame;

static CallFrame call_stack[MAX_CALL_DEPTH];
static int call_depth = 0;
static Token *last_return_target = NULL;  /* Set by exec_return when popping call stack */

/* Temporary argument buffer for DO ... WITH — transferred to call frame on push */
static ExprValue temp_call_args[MAX_PARAMS];
static int temp_call_arg_count = 0;

static void call_push(Token *return_token, Token *tokens, int is_file)
{
    if (call_depth >= MAX_CALL_DEPTH) {
        fprintf(stderr, "prg: call stack overflow\n");
        return;
    }
    CallFrame *frame = &call_stack[call_depth];
    frame->return_token = return_token;
    frame->tokens = tokens;
    frame->is_file = is_file;
    frame->param_count = 0;
    memset(frame->params, 0, sizeof(frame->params));
    call_depth++;
}

static int call_pop(Token **out_return)
{
    if (call_depth <= 0) {
        *out_return = NULL;
        return 0;
    }
    call_depth--;
    CallFrame *frame = &call_stack[call_depth];
    *out_return = frame->return_token;
    /* Free any parameters that weren't consumed by PARAMETERS */
    for (int i = 0; i < frame->param_count; i++) {
        free_value(&frame->params[i]);
    }
    if (frame->is_file && frame->tokens) {
        /* Don't free — procedure registry may hold pointers into these tokens.
           They'll leak but the program is short-lived. */
        // free_tokens(frame->tokens);
    }
    return 1;
}

/* Accessor for the current (top-of-stack) call frame */
static CallFrame *call_current(void)
{
    if (call_depth <= 0) return NULL;
    return &call_stack[call_depth - 1];
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
static ExecStatus exec_do_case(Token **cur);
static ExecStatus exec_do_call(Token **cur);
static ExecStatus exec_return(Token **cur);
static ExecStatus exec_parameters(Token **cur);
static ExecStatus exec_set(Token **cur);
static ExecStatus exec_skip(Token **cur);
static ExecStatus exec_store(Token **cur);
static ExecStatus exec_use(Token **cur);
static ExecStatus exec_close(Token **cur);
static ExecStatus exec_go(Token **cur);
static ExecStatus exec_print(Token **cur);
static ExecStatus exec_assign(Token **cur);
static ExecStatus exec_assign_macro(Token **cur);
static ExecStatus exec_accept(Token **cur);
static ExecStatus exec_delete(Token **cur);
static ExecStatus exec_recall(Token **cur);
static ExecStatus exec_pack(Token **cur);
static ExecStatus exec_wait(Token **cur);
static ExecStatus exec_zap(Token **cur);
static ExecStatus exec_replace(Token **cur);
static ExecStatus exec_average(Token **cur);
static ExecStatus exec_append(Token **cur);
static ExecStatus exec_display(Token **cur);
static ExecStatus exec_display_memory(Token **cur);
static ExecStatus exec_list(Token **cur);
static ExecStatus exec_browse(Token **cur);
static ExecStatus exec_seek(Token **cur);
static ExecStatus exec_select(Token **cur);
static ExecStatus exec_index(Token **cur);
static ExecStatus exec_locate(Token **cur);
static ExecStatus exec_continue(Token **cur);
static ExecStatus exec_count(Token **cur);
static ExecStatus exec_at(Token **cur);
static ExecStatus exec_read(Token **cur);
static ExecStatus exec_clear(Token **cur);
static ExecStatus exec_clear_all(Token **cur);
static ExecStatus exec_clear_memory(Token **cur);
static ExecStatus exec_text(Token **cur);
static ExecStatus exec_create(Token **cur);
static ExecStatus exec_save(Token **cur);
static ExecStatus exec_restore(Token **cur);

/* Block / loop helpers */
static ExecStatus exec_block_until(Token **cur, KeywordId kw1, KeywordId kw2);
static void skip_block_nested(Token **cur, KeywordId end_kw, KeywordId else_kw);
static ExecStatus exec_do_body(Token **cur);
static ExecStatus execute_tokens_from(Token **cur);

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static ExprValue parse_expr(Token **cur)
{
    ParseError err = PARSE_OK;
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

        /* Skip PROCEDURE definitions at the top level — they are registered
           by proc_scan() and only entered via DO <name>. */
        if (cur->type == TOK_KEYWORD && cur->keyword_id == KW_PROCEDURE) {
            /* Advance past this PROCEDURE keyword first, then skip to next PROCEDURE or EOF */
            cur = cur->next;
            while (cur && cur->type != TOK_EOF) {
                if (cur->type == TOK_KEYWORD && cur->keyword_id == KW_PROCEDURE)
                    break;
                cur = cur->next;
            }
            continue;
        }

        ExecStatus st = exec_statement(&cur);
        if (st == EXEC_RETURN || st == EXEC_CANCEL)
            return st;

        /* After a simple statement, advance past any trailing EOL so the
           next iteration starts at the next real token. Block constructs
           (IF, DO WHILE, FOR) already consume their own boundaries. */
        if (cur && cur->type == TOK_EOL) {
            cur = cur->next;
        }
    }
    return EXEC_OK;
}

/* Execute tokens starting from a cursor position (used after RETURN resumes). */
static ExecStatus execute_tokens_from(Token **cur)
{
    while (*cur) {
        if ((*cur)->type == TOK_EOL) {
            *cur = (*cur)->next;
            continue;
        }
        if ((*cur)->type == TOK_EOF)
            break;

        if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_PROCEDURE) {
            *cur = (*cur)->next;
            while (*cur && (*cur)->type != TOK_EOF) {
                if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_PROCEDURE)
                    break;
                *cur = (*cur)->next;
            }
            continue;
        }

        ExecStatus st = exec_statement(cur);
        if (st == EXEC_RETURN || st == EXEC_CANCEL)
            return st;

        if (*cur && (*cur)->type == TOK_EOL) {
            *cur = (*cur)->next;
        }
    }
    return EXEC_OK;
}

/* Execute a token range (used for procedure bodies). */
static ExecStatus execute_token_range(Token **cur, Token *end_sentinel)
{
    while (cur && *cur) {
        if (*cur == end_sentinel)
            break;
        if ((*cur)->type == TOK_EOF)
            break;
        if ((*cur)->type == TOK_EOL) {
            *cur = (*cur)->next;
            continue;
        }

        ExecStatus st = exec_statement(cur);
        if (st == EXEC_RETURN || st == EXEC_CANCEL)
            return st;

        if (*cur && (*cur)->type == TOK_EOL) {
            *cur = (*cur)->next;
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

    /* @...SAY / @...GET dispatch */
    if (t->type == TOK_AT)
        return exec_at(cur);

    /* Keyword-driven dispatch */
    if (t->type == TOK_KEYWORD) {
        switch (t->keyword_id) {
            case KW_IF:       return exec_if(cur);
            case KW_DO:       return exec_do_while(cur);
            case KW_FOR:      skip_to_eol(cur); return EXEC_OK;
            case KW_RETURN:     return exec_return(cur);
            case KW_PARAMETERS: return exec_parameters(cur);
            case KW_CANCEL:     (*cur) = (*cur)->next; return EXEC_CANCEL;
            case KW_QUIT:       (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_CANCEL;
            case KW_EXIT:     (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_EXIT;
            case KW_LOOP:     (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_LOOP;
            case KW_SET:      return exec_set(cur);
            case KW_SKIP:     return exec_skip(cur);
            case KW_STORE:    return exec_store(cur);
            case KW_USE:      return exec_use(cur);
            case KW_CLOSE:    return exec_close(cur);
            case KW_CREATE:   return exec_create(cur);
            case KW_GOTOP:    wa_goto_top(); (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_OK;
            case KW_GOBOTTOM: wa_goto_bottom(); (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_OK;
            case KW_GO:       return exec_go(cur);
            case KW_DELETE:   return exec_delete(cur);
            case KW_RECALL:   return exec_recall(cur);
            case KW_PACK:     return exec_pack(cur);
            case KW_WAIT:     return exec_wait(cur);
            case KW_ZAP:      return exec_zap(cur);
            case KW_REPLACE:  return exec_replace(cur);
            case KW_AVERAGE:  return exec_average(cur);
            case KW_ACCEPT:   return exec_accept(cur);
            case KW_APPEND:   return exec_append(cur);
            case KW_DISPLAY:  return exec_display(cur);
            case KW_LIST:     return exec_list(cur);
            case KW_SEEK:     return exec_seek(cur);
            case KW_SELECT:   return exec_select(cur);
            case KW_INDEX:    return exec_index(cur);
            case KW_LOCATE:   return exec_locate(cur);
            case KW_CONTINUE: return exec_continue(cur);
            case KW_COUNT:    return exec_count(cur);
            case KW_READ:     return exec_read(cur);
            case KW_CLEAR:      return exec_clear(cur);
            case KW_CLEAR_ALL:      return exec_clear_all(cur);
            case KW_CLEAR_MEMORY:   return exec_clear_memory(cur);
            case KW_TEXT:           return exec_text(cur);
            case KW_BROWSE:   return exec_browse(cur);
            case KW_SAVE:     return exec_save(cur);
            case KW_RESTORE:  return exec_restore(cur);
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

    /* Macro-expanded assignment: &<varname> = <expr>
       Must come after regular assignment so "x = &y" is handled as normal assign.
       Only matches when the line STARTS with &<name> = ... */
    if (t->type == TOK_OP_LOGIC && strcmp(t->value, "&") == 0) {
        Token *peek = t->next;
        if (peek && peek->type == TOK_IDENT) {
            Token *after = peek->next;
            if (after && (
                (after->type == TOK_OP_COMPARISON && strcmp(after->value, "=") == 0) ||
                after->type == TOK_ASSIGN))
            {
                return exec_assign_macro(cur);
            }
        }
    }

    /* Expression statement: evaluate and discard result (e.g., INKEY(1), EOF()) */
    if (t->type == TOK_KEYWORD) {
        /* Check if this keyword is a function-like builtin (has parens next) */
        Token *next = t->next;
        if (next && next->type == TOK_LPAREN) {
            ExprValue val = parse_expr(cur);
            free_value(&val);
            skip_to_eol(cur);
            return EXEC_OK;
        }
    }

    /* Fallback: skip to end of line */
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* RETURN                                                              */
/* ------------------------------------------------------------------ */

static ExecStatus exec_return(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "RETURN" */

    /* If there's a call stack, pop and resume */
    Token *return_token = NULL;
    if (call_pop(&return_token)) {
        /* Set a global return point so exec_do_call can resume */
        last_return_target = return_token;
        return EXEC_RETURN;  /* Bubble up to exec_do_call */
    }

    /* No call stack — return from the main program */
    return EXEC_RETURN;
}

/* ------------------------------------------------------------------ */
/* PARAMETERS p1, p2, ...                                              */
/* ------------------------------------------------------------------ */

static ExecStatus exec_parameters(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "PARAMETERS" */

    CallFrame *frame = call_current();
    int param_idx = 0;

    while (*cur && !is_eol_or_eof(*cur)) {
        /* Expect an identifier for the parameter name */
        if ((*cur)->type == TOK_IDENT) {
            char pname[256];
            strncpy(pname, (*cur)->value, sizeof(pname) - 1);
            pname[sizeof(pname) - 1] = '\0';
            *cur = (*cur)->next;

            /* Assign the corresponding argument from the call frame */
            if (frame && param_idx < frame->param_count) {
                /* Mark this param as consumed so call_pop doesn't free it */
                ExprValue val = frame->params[param_idx];
                frame->params[param_idx] = val_null();
                vars_set(pname, &val);
            }
            /* If no frame or out of range, just create a NULL variable */
            else {
                ExprValue null_val = val_null();
                vars_set(pname, &null_val);
            }
            param_idx++;
        } else {
            /* Unexpected token — skip */
            *cur = (*cur)->next;
        }

        /* Skip comma separator */
        if (*cur && (*cur)->type == TOK_COMMA) {
            *cur = (*cur)->next;
        }
    }

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* DO <name> [WITH <args>]                                             */
/* ------------------------------------------------------------------ */

static ExecStatus exec_do_call(Token **cur)
{
    /* cur points at the target name (IDENT or string literal) */
    if (!*cur) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    char target[256];
    if ((*cur)->type == TOK_IDENT) {
        strncpy(target, (*cur)->value, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';
        *cur = (*cur)->next;
    } else if ((*cur)->type == TOK_STRING) {
        strncpy(target, (*cur)->value, sizeof(target) - 1);
        target[sizeof(target) - 1] = '\0';
        *cur = (*cur)->next;
    } else {
        /* Unexpected token — skip line */
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Evaluate optional WITH <args> and store for the callee */
    temp_call_arg_count = 0;
    if (*cur && match_keyword(cur, KW_WITH)) {
        while (*cur && temp_call_arg_count < MAX_PARAMS) {
            if (is_eol_or_eof(*cur)) break;
            temp_call_args[temp_call_arg_count++] = parse_expr(cur);
            if (*cur && (*cur)->type == TOK_COMMA) {
                *cur = (*cur)->next;
            } else {
                break;
            }
        }
    }

    /* Skip EOL after DO statement */
    if (*cur && (*cur)->type == TOK_EOL) {
        *cur = (*cur)->next;
    }

    /* Save return point (the token after the DO statement) */
    Token *return_point = *cur;

    /* 1. Look up in internal procedure registry */
    const Procedure *proc = proc_registry_lookup(target);
    if (proc && proc->start) {
        /* Push call frame (no file cleanup needed) */
        call_push(return_point, NULL, 0);
        /* Transfer WITH arguments to the call frame */
        {
            CallFrame *frame = call_current();
            int n = temp_call_arg_count < MAX_PARAMS ? temp_call_arg_count : MAX_PARAMS;
            frame->param_count = n;
            for (int i = 0; i < n; i++) {
                frame->params[i] = temp_call_args[i];
            }
        }

        /* Find the end of this procedure (next PROCEDURE or EOF) */
        Token *end_sentinel = NULL;
        Token *scan = proc->start;
        while (scan) {
            if (scan->type == TOK_KEYWORD && scan->keyword_id == KW_PROCEDURE) {
                end_sentinel = scan;
                break;
            }
            if (scan->type == TOK_EOF)
                break;
            scan = scan->next;
        }

        Token *body_cur = proc->start;
        ExecStatus st = execute_token_range(&body_cur, end_sentinel);
        if (st == EXEC_RETURN) {
            /* RETURN popped the call stack. Resume at last_return_target. */
            *cur = last_return_target;
            return EXEC_OK;
        }
        return st;
    }

    /* 2. Try to load as external .prg file */
    char req_path[512];
    snprintf(req_path, sizeof(req_path), "%s.prg", target);

    char path[512];
    resolve_file_path(req_path, path, sizeof(path));

    char *source = NULL;
    FILE *fp = fopen(path, "r");
    if (fp) {
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (sz > 0) {
            source = malloc((size_t)sz + 1);
            if (source) {
                fread(source, 1, (size_t)sz, fp);
                source[sz] = '\0';
            }
        }
        fclose(fp);
    }

    if (!source || !*source) {
        free(source);
        fprintf(stderr, "prg: cannot open '%s'\n", path);
        /* Resume at return point */
        *cur = return_point;
        return EXEC_OK;
    }

    Token *file_tokens = tokenize(source);
    free(source);

    if (!file_tokens) {
        fprintf(stderr, "prg: failed to tokenize '%s'\n", path);
        *cur = return_point;
        return EXEC_OK;
    }

    /* Scan for procedures in the loaded file too */
    proc_scan(file_tokens);

    /* Look up the target procedure in the loaded file */
    const Procedure *fproc = proc_registry_lookup(target);

    /* Push call frame with file cleanup */
    call_push(return_point, file_tokens, 1);
    /* Transfer WITH arguments to the call frame */
    {
        CallFrame *frame = call_current();
        int n = temp_call_arg_count < MAX_PARAMS ? temp_call_arg_count : MAX_PARAMS;
        frame->param_count = n;
        for (int i = 0; i < n; i++) {
            frame->params[i] = temp_call_args[i];
        }
    }

    if (fproc && fproc->start) {
        /* File has the target procedure — run its body directly */
        Token *end_sentinel = NULL;
        Token *scan = fproc->start;
        while (scan) {
            if (scan->type == TOK_KEYWORD && scan->keyword_id == KW_PROCEDURE) {
                end_sentinel = scan;
                break;
            }
            if (scan->type == TOK_EOF)
                break;
            scan = scan->next;
        }
        Token *body_cur = fproc->start;
        ExecStatus st = execute_token_range(&body_cur, end_sentinel);
        if (st == EXEC_RETURN) {
            *cur = last_return_target;
            return EXEC_OK;
        }
        return st;
    }

    /* No procedure found — run top-level code in the file */
    Token *file_cur = file_tokens;
    ExecStatus st = execute_tokens(file_cur);
    if (st == EXEC_RETURN) {
        *cur = last_return_target;
        return EXEC_OK;
    }
    return st;
}

/* ------------------------------------------------------------------ */
/* execute_file — public API for loading and running a .prg file       */
/* ------------------------------------------------------------------ */

ExecStatus execute_file(const char *path)
{
    char req_path[512];
    if (strchr(path, '.') == NULL) {
        snprintf(req_path, sizeof(req_path), "%s.prg", path);
    } else {
        strncpy(req_path, path, sizeof(req_path) - 1);
        req_path[sizeof(req_path) - 1] = '\0';
    }

    char full_path[512];
    resolve_file_path(req_path, full_path, sizeof(full_path));

    FILE *fp = fopen(full_path, "r");
    if (!fp) {
        fprintf(stderr, "prg: cannot open '%s': %m\n", full_path);
        return EXEC_CANCEL;
    }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (sz < 0) {
        fclose(fp);
        return EXEC_CANCEL;
    }

    char *source = malloc((size_t)sz + 1);
    if (!source) {
        fclose(fp);
        return EXEC_CANCEL;
    }

    size_t n = fread(source, 1, (size_t)sz, fp);
    source[n] = '\0';
    fclose(fp);

    Token *tokens = tokenize(source);
    free(source);

    if (!tokens)
        return EXEC_CANCEL;

    proc_registry_init();
    proc_scan(tokens);

    ExecStatus st = execute_tokens(tokens);
    free_tokens(tokens);
    return st;
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

    /* Skip EOL after condition so the body starts at the next real token */
    if (*cur && (*cur)->type == TOK_EOL)
        *cur = (*cur)->next;

    if (truthy) {
        ExecStatus st = exec_block_until(cur, KW_ENDIF, KW_ELSE);
        if (st != EXEC_OK)
            return st;
        /* If we stopped at ELSE, skip the else-body to ENDIF */
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ELSE) {
            *cur = (*cur)->next; /* skip ELSE keyword itself */
            skip_block_nested(cur, KW_ENDIF, 0);
            if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDIF) {
                *cur = (*cur)->next; /* consume ENDIF */
            }
        }
        return EXEC_OK;
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
   If kw2 is 0, only kw1 is a sentinel.
   Nested IF/DO/FOR are fully consumed by exec_statement, so we only
   need to check for our own sentinels at the top level. */
static ExecStatus exec_block_until(Token **cur, KeywordId kw1, KeywordId kw2)
{
    while (*cur && (*cur)->type != TOK_EOF) {
        /* Skip EOL between lines of the block */
        if ((*cur)->type == TOK_EOL) {
            *cur = (*cur)->next;
            continue;
        }

        /* Check for our sentinel keywords at this level.
           Nested IF/DO/FOR blocks are consumed entirely by exec_statement,
           so any ENDIF/ELSE/ENDDO/ENDFOR we see here belongs to us. */
        if ((*cur)->type == TOK_KEYWORD) {
            if ((*cur)->keyword_id == kw1 || (kw2 && (*cur)->keyword_id == kw2))
                break;
        }

        ExecStatus st = exec_statement(cur);
        if (st == EXEC_RETURN || st == EXEC_CANCEL || st == EXEC_EXIT || st == EXEC_LOOP)
            return st;
    }

    /* Consume the sentinel keyword itself */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == kw1) {
        *cur = (*cur)->next;
    }

    return EXEC_OK;
}

/* Check if a DO keyword starts a block (DO WHILE / DO CASE) vs a bare call (DO <name>). */
static int do_is_block(const Token *tok)
{
    /* tok points at the DO keyword. Look at the next token. */
    Token *next = tok->next;
    if (!next)
        return 0;
    /* DO WHILE ... ENDDO or DO CASE ... ENDCASE are blocks. */
    if (next->type == TOK_KEYWORD &&
        (next->keyword_id == KW_WHILE || next->keyword_id == KW_CASE))
        return 1;
    /* DO <name> is a bare procedure call — not a block. */
    return 0;
}

/* Skip tokens until we hit a sentinel, respecting nesting. */
static void skip_block_nested(Token **cur, KeywordId end_kw, KeywordId else_kw)
{
    int depth = 1;
    while (*cur && (*cur)->type != TOK_EOF) {
        if ((*cur)->type == TOK_KEYWORD) {
            /* KW_FOR here must be a FOR/ENDFOR loop, NOT the "FOR" in
               "LOCATE FOR <expr>" or "INDEX ON <expr> TAG <name> FOR <expr>".
               Heuristic: FOR is a block only when followed by an expression
               that is eventually terminated by ENDFOR.  We can't easily tell
               here, so skip treating bare FOR as a nesting keyword and rely
               on the fact that ENDFOR is not listed among our sentinels. */
            if ((*cur)->keyword_id == KW_IF ||
                ((*cur)->keyword_id == KW_DO && do_is_block(*cur))) {
                depth++;
            } else if ((*cur)->keyword_id == end_kw ||
                       (*cur)->keyword_id == KW_ENDIF ||
                       (*cur)->keyword_id == KW_ENDDO ||
                       (*cur)->keyword_id == KW_ENDCASE ||
                       (else_kw && (*cur)->keyword_id == else_kw)) {
                depth--;
                if (depth <= 0) break;
            }
        }
        *cur = (*cur)->next;
    }
    /* Do NOT consume the sentinel — caller decides what to do with it */
}

/* ------------------------------------------------------------------ */
/* DO WHILE / ENDDO                                                    */
/* ------------------------------------------------------------------ */

static ExecStatus exec_do_while(Token **cur)
{
    /* DO WHILE <condition>  or  DO CASE  or  DO <name> [WITH ...] */
    *cur = (*cur)->next; /* skip "DO" */
    if (*cur && match_keyword(cur, KW_WHILE)) {
        /* DO WHILE — fall through to existing loop logic below */
    } else if (*cur && match_keyword(cur, KW_CASE)) {
        /* DO CASE — delegate to exec_do_case which back-patches cur */
        return exec_do_case(cur);
    } else {
        /* bare "DO <name>" — delegate to exec_do_call which back-patches cur */
        return exec_do_call(cur);
    }

    /* Save condition token range for re-evaluation */
    Token *cond_start = *cur;

    ExprValue cond = parse_expr(cur);
    int truthy = val_to_logical(&cond);
    free_value(&cond);

    /* Skip EOL after condition so the body starts at the next real token */
    if (*cur && (*cur)->type == TOK_EOL)
        *cur = (*cur)->next;

    /* Save body start for re-execution on each iteration */
    Token *body_start = *cur;

    while (truthy) {
        /* Reset cursor to body start for this iteration */
        *cur = body_start;

        /* Execute body until ENDDO, EXIT, or LOOP */
        ExecStatus st = exec_do_body(cur);

        if (st == EXEC_EXIT) {
            /* Skip rest of body to matching ENDDO, tracking nesting */
            int depth = 1;
            while (*cur && (*cur)->type != TOK_EOF) {
                if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_DO &&
                    do_is_block(*cur)) {
                    depth++;
                } else if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDDO) {
                    depth--;
                    if (depth == 0) {
                        *cur = (*cur)->next; /* consume matching ENDDO */
                        break;
                    }
                }
                *cur = (*cur)->next;
            }
            return EXEC_OK;
        }
        if (st == EXEC_RETURN || st == EXEC_CANCEL) {
            return st;
        }

        /* Re-evaluate condition from saved position */
        Token *cond_cur = cond_start;
        cond = parse_expr(&cond_cur);
        truthy = val_to_logical(&cond);
        free_value(&cond);
    }

    /* If the loop body was never entered (condition initially false),
       we must skip past the body to the matching ENDDO and consume it.
       Track nesting depth to handle nested DO WHILE inside the body. */
    if (!truthy || (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDDO)) {
        int depth = 1;
        while (*cur && (*cur)->type != TOK_EOF) {
            if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_DO) {
                /* Check if this is a DO WHILE (not DO CASE or DO <name>) */
                Token *peek = (*cur)->next;
                if (peek && peek->type == TOK_KEYWORD && peek->keyword_id == KW_WHILE) {
                    depth++;
                }
            }
            if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDDO) {
                depth--;
                if (depth == 0) {
                    *cur = (*cur)->next; /* consume matching ENDDO */
                    break;
                }
            }
            *cur = (*cur)->next;
        }
    }

    return EXEC_OK;
}

/* Execute the body of a DO WHILE loop. Stops at ENDDO, EXIT, or LOOP.
   Does NOT consume ENDDO — caller is responsible. */
static ExecStatus exec_do_body(Token **cur)
{
    while (*cur && (*cur)->type != TOK_EOF) {
        /* Skip EOL between lines of the body */
        if ((*cur)->type == TOK_EOL) {
            *cur = (*cur)->next;
            continue;
        }

        if ((*cur)->type == TOK_KEYWORD) {
            KeywordId kw = (*cur)->keyword_id;

            if (kw == KW_ENDDO) {
                /* Don't consume ENDDO — let caller handle it */
                return EXEC_OK;
            }
            if (kw == KW_EXIT) {
                *cur = (*cur)->next; /* consume EXIT */
                skip_to_eol(cur);
                /* Skip to matching ENDDO, tracking nesting */
                {
                    int depth = 1;
                    while (*cur && (*cur)->type != TOK_EOF) {
                        if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_DO &&
                            do_is_block(*cur)) {
                            depth++;
                        } else if ((*cur)->type == TOK_KEYWORD &&
                                   (*cur)->keyword_id == KW_ENDDO) {
                            depth--;
                            if (depth == 0) {
                                /* Don't consume ENDDO — let caller handle it */
                                break;
                            }
                        }
                        *cur = (*cur)->next;
                    }
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
/* DO CASE / CASE / OTHERWISE / ENDCASE                                */
/* ------------------------------------------------------------------ */

static ExecStatus exec_do_case(Token **cur)
{
    /* cur points just past "DO CASE" (CASE already consumed by caller).
       Iterate through CASE <expr> branches and OTHERWISE.
       Execute the first matching branch, skip the rest to ENDCASE. */

    int matched = 0;

    while (*cur && (*cur)->type != TOK_EOF) {
        if ((*cur)->type == TOK_EOL) {
            *cur = (*cur)->next;
            continue;
        }

        if ((*cur)->type != TOK_KEYWORD) {
            /* Non-keyword inside DO CASE — skip to EOL */
            skip_to_eol(cur);
            continue;
        }

        KeywordId kw = (*cur)->keyword_id;

        if (kw == KW_ENDCASE) {
            *cur = (*cur)->next; /* consume ENDCASE */
            return EXEC_OK;
        }

        if (kw == KW_CASE) {
            /* CASE <expression> */
            *cur = (*cur)->next; /* skip "CASE" */

            if (matched) {
                /* Already matched a branch — skip this one's body */
                /* Skip expression */
                ExprValue cond = parse_expr(cur);
                free_value(&cond);
                /* Skip EOL after expression */
                if (*cur && (*cur)->type == TOK_EOL)
                    *cur = (*cur)->next;
                /* Skip body to next CASE / OTHERWISE / ENDCASE */
                while (*cur && (*cur)->type != TOK_EOF) {
                    if ((*cur)->type == TOK_EOL) { *cur = (*cur)->next; continue; }
                    if ((*cur)->type == TOK_KEYWORD &&
                        ((*cur)->keyword_id == KW_CASE ||
                         (*cur)->keyword_id == KW_OTHERWISE ||
                         (*cur)->keyword_id == KW_ENDCASE))
                        break;
                    *cur = (*cur)->next;
                }
                continue;
            }

            /* Evaluate condition */
            ExprValue cond = parse_expr(cur);
            int truthy = val_to_logical(&cond);
            free_value(&cond);

            /* Skip EOL after expression */
            if (*cur && (*cur)->type == TOK_EOL)
                *cur = (*cur)->next;

            if (truthy) {
                matched = 1;
                /* Execute body until next CASE / OTHERWISE / ENDCASE */
                while (*cur && (*cur)->type != TOK_EOF) {
                    if ((*cur)->type == TOK_EOL) { *cur = (*cur)->next; continue; }
                    if ((*cur)->type == TOK_KEYWORD &&
                        ((*cur)->keyword_id == KW_CASE ||
                         (*cur)->keyword_id == KW_OTHERWISE ||
                         (*cur)->keyword_id == KW_ENDCASE))
                        break;
                    ExecStatus st = exec_statement(cur);
                    if (st == EXEC_RETURN || st == EXEC_CANCEL)
                        return st;
                }
            } else {
                /* Skip body to next CASE / OTHERWISE / ENDCASE */
                while (*cur && (*cur)->type != TOK_EOF) {
                    if ((*cur)->type == TOK_EOL) { *cur = (*cur)->next; continue; }
                    if ((*cur)->type == TOK_KEYWORD &&
                        ((*cur)->keyword_id == KW_CASE ||
                         (*cur)->keyword_id == KW_OTHERWISE ||
                         (*cur)->keyword_id == KW_ENDCASE))
                        break;
                    *cur = (*cur)->next;
                }
            }
            continue;
        }

        if (kw == KW_OTHERWISE) {
            if (matched) {
                /* Already matched — skip OTHERWISE body to ENDCASE */
                *cur = (*cur)->next; /* skip "OTHERWISE" */
                if (*cur && (*cur)->type == TOK_EOL)
                    *cur = (*cur)->next;
                while (*cur && (*cur)->type != TOK_EOF) {
                    if ((*cur)->type == TOK_EOL) { *cur = (*cur)->next; continue; }
                    if ((*cur)->type == TOK_KEYWORD &&
                        (*cur)->keyword_id == KW_ENDCASE)
                        break;
                    *cur = (*cur)->next;
                }
                continue;
            }
            /* Catch-all — execute body */
            matched = 1;
            *cur = (*cur)->next; /* skip "OTHERWISE" */
            if (*cur && (*cur)->type == TOK_EOL)
                *cur = (*cur)->next;
            while (*cur && (*cur)->type != TOK_EOF) {
                if ((*cur)->type == TOK_EOL) { *cur = (*cur)->next; continue; }
                if ((*cur)->type == TOK_KEYWORD &&
                    (*cur)->keyword_id == KW_ENDCASE)
                    break;
                ExecStatus st = exec_statement(cur);
                if (st == EXEC_RETURN || st == EXEC_CANCEL)
                    return st;
            }
            continue;
        }

        /* Unknown keyword inside DO CASE — skip line */
        skip_to_eol(cur);
    }

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* Assignment                                                          */
/* ------------------------------------------------------------------ */

static ExecStatus exec_assign(Token **cur)
{
    char varname[256];
    strncpy(varname, (*cur)->value, sizeof(varname) - 1);
    varname[sizeof(varname) - 1] = '\0';
    *cur = (*cur)->next; /* skip first identifier */

    /* Check for alias->field assignment: A->FIELD = expr */
    if (*cur && (*cur)->type == TOK_ARROW) {
        *cur = (*cur)->next; /* skip -> */
        /* Field name can be TOK_IDENT or TOK_KEYWORD (e.g. STATUS, DATE) */
        if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_KEYWORD)) {
            char fieldname[256];
            strncpy(fieldname, (*cur)->value, sizeof(fieldname) - 1);
            fieldname[sizeof(fieldname) - 1] = '\0';
            *cur = (*cur)->next; /* skip field name */

            /* Resolve alias to work area (0-based index) */
            int area = wa_alias_to_area(varname);
            if (area < 0)
                area = wa_get_selected(); /* fallback */

            /* Save current selection (0-based) and switch to target area (wa_select wants 1-based) */
            int saved_area = wa_get_selected();
            wa_select(area + 1);

            /* Skip '=' or ':=' */
            if (*cur && (*cur)->type == TOK_OP_COMPARISON && strcmp((*cur)->value, "=") == 0) {
                *cur = (*cur)->next;
            } else if (*cur && (*cur)->type == TOK_ASSIGN) {
                *cur = (*cur)->next;
            }

            ExprValue val = parse_expr(cur);
            char *s = val_to_string(&val);
            wa_replace(fieldname, s);
            free(s);
            free_value(&val);

            /* Restore original selection (saved_area is 0-based, wa_select wants 1-based) */
            wa_select(saved_area + 1);
            return EXEC_OK;
        }
    }

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

/* Handle &varname = expr (macro-expanded variable assignment)
   The &<name> token resolves <name> to get the actual target variable name,
   then assigns the RHS expression to that resolved name. */
static ExecStatus exec_assign_macro(Token **cur)
{
    /* cur points at '&' (TOK_OP_LOGIC) */
    *cur = (*cur)->next; /* skip '&' */

    /* Read the identifier that holds the target variable name */
    char ident[256] = "";
    if (*cur && (*cur)->type == TOK_IDENT) {
        strncpy(ident, (*cur)->value, sizeof(ident) - 1);
        ident[sizeof(ident) - 1] = '\0';
        *cur = (*cur)->next;
    } else {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Skip '=' or ':=' */
    if (*cur && (*cur)->type == TOK_OP_COMPARISON && strcmp((*cur)->value, "=") == 0) {
        *cur = (*cur)->next;
    } else if (*cur && (*cur)->type == TOK_ASSIGN) {
        *cur = (*cur)->next;
    }

    /* Resolve: the identifier holds the name of the target variable */
    ExprValue name_val = vars_get(ident);
    char *name_str = val_to_string(&name_val);
    free_value(&name_val);

    /* Evaluate RHS expression */
    ExprValue val = parse_expr(cur);

    if (name_str && *name_str) {
        vars_set(name_str, &val);
    }
    free(name_str);
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
    char alt_buf[4096] = {0};  /* accumulate alternate output */
    while (*cur && !is_eol_or_eof(*cur)) {
        ExprValue val = parse_expr(cur);
        char *s = val_to_string(&val);
        if (g_set_console) {
            if (ui_is_active()) {
                if (!first) ui_print(" ");
                ui_print(s);
            } else {
                if (!first) printf(" ");
                printf("%s", s);
            }
        }
        /* Mirror to alternate file */
        if (!first) strcat(alt_buf, " ");
        strcat(alt_buf, s);
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

    if (add_newline) {
        if (g_set_console) {
            if (ui_is_active()) {
                ui_print_newline();
            } else {
                printf("\n");
                fflush(stdout);
            }
        }
        strcat(alt_buf, "\n");
    }
    alternate_write(alt_buf);
    ui_refresh();

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

    /* Handle SET PROCEDURE TO <file1>, <file2>, ... */
    if (setting == KW_PROCEDURE) {
        /* Clear previously loaded procedure files */
        proc_registry_init();

        /* Expect: SET PROCEDURE TO <file1> [, <file2> ...] */
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
            *cur = (*cur)->next; /* skip "TO" */
        }
        /* Read comma-separated file names until EOL */
        while (*cur && !is_eol_or_eof(*cur)) {
            char fname[512];
            if ((*cur)->type == TOK_IDENT) {
                strncpy(fname, (*cur)->value, sizeof(fname) - 1);
                fname[sizeof(fname) - 1] = '\0';
                *cur = (*cur)->next;
            } else if ((*cur)->type == TOK_STRING) {
                strncpy(fname, (*cur)->value, sizeof(fname) - 1);
                fname[sizeof(fname) - 1] = '\0';
                *cur = (*cur)->next;
            } else {
                break;
            }
            /* Load file and scan for procedures (no execution) */
            {
                char req[512];
                if (strchr(fname, '.') == NULL) {
                    snprintf(req, sizeof(req), "%s.prg", fname);
                } else {
                    strncpy(req, fname, sizeof(req) - 1);
                    req[sizeof(req) - 1] = '\0';
                }
                char full[512];
                resolve_file_path(req, full, sizeof(full));
                FILE *fp = fopen(full, "r");
                if (fp) {
                    fseek(fp, 0, SEEK_END);
                    long sz = ftell(fp);
                    fseek(fp, 0, SEEK_SET);
                    if (sz > 0) {
                        char *src = malloc((size_t)sz + 1);
                        if (src) {
                            fread(src, 1, (size_t)sz, fp);
                            src[sz] = '\0';
                            Token *toks = tokenize(src);
                            free(src);
                            if (toks) {
                                proc_scan(toks);
                                /* Keep tokens alive — procedure bodies point into them */
                                if (proc_file_count < MAX_PROC_FILES) {
                                    proc_file_tokens[proc_file_count++] = toks;
                                } else {
                                    free_tokens(toks);
                                }
                            }
                        }
                    }
                    fclose(fp);
                }
            }
            /* Skip comma if present */
            if (*cur && (*cur)->type == TOK_COMMA) {
                *cur = (*cur)->next;
            }
        }
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Handle SET INDEX TO <file> */
    if (setting == KW_INDEX || setting == KW_SET_INDEX) {
        /* Skip optional "TO" */
        if (*cur && (*cur)->type == TOK_IDENT &&
            port_strcasecmp((*cur)->value, "to") == 0) {
            *cur = (*cur)->next;
        }
        if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING)) {
            char idxfile[1024];
            strncpy(idxfile, (*cur)->value, sizeof(idxfile) - 1);
            idxfile[sizeof(idxfile) - 1] = '\0';
            *cur = (*cur)->next;
            /* Resolve case-insensitively */
            {
                char req[1024];
                if (strchr(idxfile, '.') == NULL) {
                    snprintf(req, sizeof(req), "%s.ndx", idxfile);
                } else {
                    strncpy(req, idxfile, sizeof(req) - 1);
                    req[sizeof(req) - 1] = '\0';
                }
                char resolved[1024];
                resolve_file_path(req, resolved, sizeof(resolved));
                strncpy(idxfile, resolved, sizeof(idxfile) - 1);
                idxfile[sizeof(idxfile) - 1] = '\0';
            }
            wa_set_index(idxfile);
        } else {
            /* SET INDEX TO with no file = clear */
            wa_set_index_clear();
        }
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Handle SET ALTERNATE TO "file" or SET ALTERNATE TO (close) */
    if (setting == KW_ALTERNATE) {
        /* Skip optional "TO" */
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
            *cur = (*cur)->next;
        }
        /* Close any existing alternate file */
        alternate_close();
        /* Check for a file name */
        if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING)) {
            char altfile[1024];
            strncpy(altfile, (*cur)->value, sizeof(altfile) - 1);
            altfile[sizeof(altfile) - 1] = '\0';
            *cur = (*cur)->next;
            /* The tokenizer skips '.' between identifier parts, so if the
               next token is also an identifier (or keyword used as filename),
               it's the extension part (e.g. "alt_test.log" → "alt_test" + "log"). */
            if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_KEYWORD)) {
                strncat(altfile, ".", sizeof(altfile) - strlen(altfile) - 1);
                strncat(altfile, (*cur)->value, sizeof(altfile) - strlen(altfile) - 1);
                *cur = (*cur)->next;
            }
            /* Force lowercase for writes so files are always lowercase on disk */
            force_lowercase(altfile, altfile, sizeof(altfile));
            g_alternate_file = fopen(altfile, "a");
        }
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Handle SET ORDER TO <n> */
    if (setting == KW_SET_ORDER) {
        /* Skip optional "TO" */
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
            *cur = (*cur)->next;
        }
        int order = 0;
        if (*cur && (*cur)->type == TOK_INTEGER) {
            order = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
        wa_set_order(order);
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Read ON/OFF or TO <value> */
    if (*cur && (*cur)->type == TOK_IDENT) {
        const char *flag = (*cur)->value;
        int on = (port_strcasecmp(flag, "ON") == 0);
        if (setting == KW_CENTURY) {
            g_set_century = on;
        } else if (setting == KW_CONSOLE) {
            g_set_console = on;
        } else if (setting == KW_EXACT) {
            g_set_exact = on;
        }
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
    (*cur) = (*cur)->next; /* skip "SKIP" */
    int n = 1;

    if (*cur && !is_eol_or_eof(*cur)) {
        if (*cur && (*cur)->type == TOK_OP_ARITH && strcmp((*cur)->value, "-") == 0) {
            *cur = (*cur)->next;
            if (*cur && ((*cur)->type == TOK_INTEGER || (*cur)->type == TOK_REAL)) {
                n = -atoi((*cur)->value);
                *cur = (*cur)->next;
            }
        } else if (*cur && ((*cur)->type == TOK_INTEGER || (*cur)->type == TOK_REAL)) {
            n = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
    }

    wa_skip(n);
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* USE                                                                 */
/* ------------------------------------------------------------------ */

static ExecStatus exec_use(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "USE" */
    if (!*cur) { skip_to_eol(cur); return EXEC_OK; }

    char filename[1024];
    if ((*cur)->type == TOK_IDENT) {
        strncpy(filename, (*cur)->value, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        *cur = (*cur)->next;
    } else if ((*cur)->type == TOK_STRING) {
        strncpy(filename, (*cur)->value, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        *cur = (*cur)->next;
    } else {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Parse optional INDEX tag1,tag2,... */
    char index_tags[1024] = {0};
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_INDEX) {
        *cur = (*cur)->next; /* skip "INDEX" */
        /* Collect comma-separated tag names until ALIAS or EOL */
        while (*cur && (*cur)->type != TOK_EOL && (*cur)->type != TOK_EOF
               && !((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALIAS)) {
            if (*cur && (*cur)->type == TOK_IDENT) {
                if (index_tags[0]) strcat(index_tags, ",");
                strcat(index_tags, (*cur)->value);
                *cur = (*cur)->next;
                /* Skip comma if present */
                if (*cur && (*cur)->type == TOK_COMMA)
                    *cur = (*cur)->next;
            } else {
                *cur = (*cur)->next;
            }
        }
    }

    /* Parse optional ALIAS name */
    char alias_name[1024] = {0};
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALIAS) {
        *cur = (*cur)->next; /* skip "ALIAS" */
        if (*cur && (*cur)->type == TOK_IDENT) {
            strncpy(alias_name, (*cur)->value, sizeof(alias_name) - 1);
            alias_name[sizeof(alias_name) - 1] = '\0';
            *cur = (*cur)->next;
        }
    }

    /* Resolve filename case-insensitively (add .dbf if no extension) */
    {
        char req[1024];
        if (strchr(filename, '.') == NULL) {
            snprintf(req, sizeof(req), "%s.dbf", filename);
        } else {
            strncpy(req, filename, sizeof(req) - 1);
            req[sizeof(req) - 1] = '\0';
        }
        char resolved[1024];
        resolve_file_path(req, resolved, sizeof(resolved));
        strncpy(filename, resolved, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
    }

    if (wa_use(filename, -1, alias_name[0] ? alias_name : NULL) != 0) {
        /* wa_use already printed to stderr; also show on screen */
        char msg[1100];
        snprintf(msg, sizeof(msg), "Error: cannot open database '%s'", filename);
        printw("%s", msg);
        refresh();
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Set indexes if specified */
    if (index_tags[0]) {
        /* Parse comma-separated tags and set them via SET INDEX TO */
        char *saveptr = NULL;
        char *tag = strtok_r(index_tags, ",", &saveptr);
        char combined[1024] = "";
        while (tag) {
            /* Trim leading/trailing spaces */
            while (*tag == ' ') tag++;
            char *end = tag + strlen(tag) - 1;
            while (end > tag && *end == ' ') *end = '\0';

            if (combined[0]) strcat(combined, " ");
            strcat(combined, tag);
            tag = strtok_r(NULL, ",", &saveptr);
        }
        if (combined[0]) {
            wa_set_index(combined);
        }
    }

    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* CLOSE                                                               */
/* ------------------------------------------------------------------ */

static ExecStatus exec_close(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "CLOSE" */

    /* Handle CLOSE ALTERNATE */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALTERNATE) {
        *cur = (*cur)->next;
        alternate_close();
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* CLOSE ALL / CLOSE DATABASES / CLOSE DATA — close workareas */
    wa_close_all();
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* GO                                                                  */
/* ------------------------------------------------------------------ */

static ExecStatus exec_go(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "GO" */
    if (!*cur || is_eol_or_eof(*cur)) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Handle "GO TOP" and "GO BOTTOM" (tokenized as GO + IDENT "top"/"bottom") */
    if ((*cur)->type == TOK_IDENT &&
        port_strcasecmp((*cur)->value, "top") == 0) {
        *cur = (*cur)->next;
        wa_goto_top();
        skip_to_eol(cur);
        return EXEC_OK;
    }
    if ((*cur)->type == TOK_IDENT &&
        port_strcasecmp((*cur)->value, "bottom") == 0) {
        *cur = (*cur)->next;
        wa_goto_bottom();
        skip_to_eol(cur);
        return EXEC_OK;
    }

    ExprValue val = parse_expr(cur);
    int rec = val_to_int(&val);
    free_value(&val);
    wa_goto(rec);

    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* Scope modifier helper                                               */
/*                                                                      */
/* Parses optional ALL / FOR <expr> / WHILE <expr> from *cur.           */
/* Sets *scope_all, *for_start, *while_start.                           */
/* Advances *cur past the scope tokens (but not past EOL).              */
/*                                                                      */
/* Usage pattern in callers:                                            */
/*   if (scope_all)  -> operate on every record                         */
/*   if (for_start)  -> re-evaluate expr per record, skip if false      */
/*   if (while_start)-> re-evaluate expr per record, stop if false      */
/* ------------------------------------------------------------------ */

typedef struct {
    int    scope_all;
    int    scope_next;           /* 0 = not used, >0 = NEXT n */
    int    scope_next_start;     /* starting record for NEXT n */
    int    scope_record;         /* 0 = not used, >0 = RECORD n */
    int    scope_rest;           /* 1 = REST scope */
    Token *for_start;
    Token *while_start;
} ScopeInfo;

static void parse_scope(Token **cur, ScopeInfo *si)
{
    si->scope_all = 0;
    si->scope_next = 0;
    si->scope_next_start = 0;
    si->scope_record = 0;
    si->scope_rest = 0;
    si->for_start = NULL;
    si->while_start = NULL;

    if (!*cur || is_eol_or_eof(*cur))
        return;

    /* Check for ALL */
    if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALL) {
        si->scope_all = 1;
        *cur = (*cur)->next;
        if (!*cur || is_eol_or_eof(*cur))
            return;
    }

    /* Check for NEXT n */
    if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT) {
        *cur = (*cur)->next; /* skip "NEXT" */
        si->scope_next = 1;
        si->scope_next_start = wa_recno();
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si->scope_next_start++; /* NEXT starts from current+1 */
            si->scope_next = atoi((*cur)->value);
            *cur = (*cur)->next;
        } else {
            si->scope_next_start++; /* default: NEXT 1 = current+1 */
        }
        if (!*cur || is_eol_or_eof(*cur))
            return;
    }

    /* Check for RECORD n */
    if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_RECORD) {
        *cur = (*cur)->next; /* skip "RECORD" */
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si->scope_record = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
        if (!*cur || is_eol_or_eof(*cur))
            return;
    }

    /* Check for REST */
    if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_REST) {
        si->scope_rest = 1;
        *cur = (*cur)->next;
        if (!*cur || is_eol_or_eof(*cur))
            return;
    }

    /* Check for FOR <expr> */
    if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FOR) {
        *cur = (*cur)->next; /* skip "FOR" */
        si->for_start = *cur;

        /* Advance past the FOR expression to find WHILE or EOL */
        int paren_depth = 0;
        while (*cur && !is_eol_or_eof(*cur)) {
            if ((*cur)->type == TOK_LPAREN) paren_depth++;
            else if ((*cur)->type == TOK_RPAREN) { if (paren_depth > 0) paren_depth--; }
            else if (paren_depth == 0 &&
                     (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE)
                break;
            *cur = (*cur)->next;
        }
    }

    /* Check for WHILE <expr> */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE) {
        *cur = (*cur)->next; /* skip "WHILE" */
        si->while_start = *cur;
        /* Don't advance past WHILE expr — caller does skip_to_eol */
    }
}

/* Evaluate scope conditions for the current record. Returns 1 if the
   record should be processed, 0 if it should be skipped.
   For WHILE: returns 0 also means "stop scanning entirely".
   Use *stop_scan (output param) to distinguish skip vs stop. */
static int eval_scope(const ScopeInfo *si, int *stop_scan)
{
    *stop_scan = 0;

    /* Evaluate WHILE guard first */
    if (si->while_start) {
        Token *wcur = si->while_start;
        ExprValue wval = parse_expr(&wcur);
        int wresult = val_to_logical(&wval);
        free_value(&wval);
        if (!wresult) {
            *stop_scan = 1;
            return 0;
        }
    }

    /* Evaluate FOR condition */
    if (si->for_start) {
        Token *fcur = si->for_start;
        ExprValue fval = parse_expr(&fcur);
        int fresult = val_to_logical(&fval);
        free_value(&fval);
        if (!fresult)
            return 0;
    }

    return 1;
}

/* ------------------------------------------------------------------ */
/* DELETE [ALL|NEXT n|RECORD n|REST] [FOR expr] [WHILE expr]           */
/* ------------------------------------------------------------------ */

static ExecStatus exec_delete(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "DELETE" */

    ScopeInfo si;
    parse_scope(cur, &si);
    skip_to_eol(cur);

    DATABASEDBF *db = wa_db();
    if (!db)
        return EXEC_OK;

    int total = reccount(db);
    int saved_rec = wa_recno();

    if (si.scope_record) {
        /* DELETE RECORD n — delete specific record */
        gotos(wa_db_ptr(), si.scope_record);
        wa_delete();

    } else if (si.scope_all) {
        /* DELETE ALL — mark every record */
        for (int rec = 1; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            wa_delete();
        }

    } else if (si.scope_next) {
        /* DELETE NEXT n — delete n records starting from current+1 */
        int start = si.scope_next_start;
        int count = si.scope_next;
        int last_valid_rec = 0;
        for (int i = 0, rec = start; i < count && rec <= total; i++, rec++) {
            gotos(wa_db_ptr(), rec);
            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) break;
                    continue;
                }
            }
            last_valid_rec = rec;
            wa_delete();
        }
        if (last_valid_rec > 0)
            gotos(wa_db_ptr(), last_valid_rec);

    } else if (si.scope_rest) {
        /* DELETE REST — from current to EOF */
        int start = wa_recno();
        int last_valid_rec = 0;
        for (int rec = start; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) {
                        if (last_valid_rec > 0)
                            gotos(wa_db_ptr(), last_valid_rec);
                        break;
                    }
                    continue;
                }
            }
            last_valid_rec = rec;
            wa_delete();
        }

    } else if (si.for_start || si.while_start) {
        /* DELETE FOR/WHILE — scan entire file */
        int last_valid_rec = 0;
        for (int rec = 1; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            int stop = 0;
            if (!eval_scope(&si, &stop)) {
                if (stop) {
                    if (last_valid_rec > 0)
                        gotos(wa_db_ptr(), last_valid_rec);
                    break;
                }
                continue;
            }
            last_valid_rec = rec;
            wa_delete();
        }

    } else {
        /* No scope — delete current record only */
        wa_delete();
    }

    /* Update DBF header last-update date */
    dbf_update_date(db);

    /* Restore cursor position */
    if (!si.scope_next && !si.scope_rest)
        gotos(wa_db_ptr(), saved_rec);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* RECALL [ALL|NEXT n|RECORD n|REST] [FOR expr] [WHILE expr]           */
/* ------------------------------------------------------------------ */

static ExecStatus exec_recall(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "RECALL" */

    ScopeInfo si;
    parse_scope(cur, &si);
    skip_to_eol(cur);

    DATABASEDBF *db = wa_db();
    if (!db)
        return EXEC_OK;

    int total = reccount(db);
    int saved_rec = wa_recno();

    if (si.scope_record) {
        /* RECALL RECORD n */
        gotos(wa_db_ptr(), si.scope_record);
        wa_recall();

    } else if (si.scope_all) {
        for (int rec = 1; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            wa_recall();
        }

    } else if (si.scope_next) {
        /* RECALL NEXT n */
        int start = si.scope_next_start;
        int count = si.scope_next;
        int last_valid_rec = 0;
        for (int i = 0, rec = start; i < count && rec <= total; i++, rec++) {
            gotos(wa_db_ptr(), rec);
            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) break;
                    continue;
                }
            }
            last_valid_rec = rec;
            wa_recall();
        }
        if (last_valid_rec > 0)
            gotos(wa_db_ptr(), last_valid_rec);

    } else if (si.scope_rest) {
        /* RECALL REST */
        int start = wa_recno();
        int last_valid_rec = 0;
        for (int rec = start; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) {
                        if (last_valid_rec > 0)
                            gotos(wa_db_ptr(), last_valid_rec);
                        break;
                    }
                    continue;
                }
            }
            last_valid_rec = rec;
            wa_recall();
        }

    } else if (si.for_start || si.while_start) {
        int last_valid_rec = 0;
        for (int rec = 1; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            int stop = 0;
            if (!eval_scope(&si, &stop)) {
                if (stop) {
                    if (last_valid_rec > 0)
                        gotos(wa_db_ptr(), last_valid_rec);
                    break;
                }
                continue;
            }
            last_valid_rec = rec;
            wa_recall();
        }

    } else {
        wa_recall();
    }

    /* Update DBF header last-update date */
    dbf_update_date(db);

    if (!si.scope_next && !si.scope_rest)
        gotos(wa_db_ptr(), saved_rec);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* PACK                                                                */
/* ------------------------------------------------------------------ */

static ExecStatus exec_pack(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "PACK" */
    wa_pack();
    /* Update DBF header last-update date */
    { DATABASEDBF *db = wa_db(); if (db) dbf_update_date(db); }
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* ACCEPT "prompt" TO varname                                          */
/* ------------------------------------------------------------------ */

static ExecStatus exec_accept(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "ACCEPT" */

    /* Parse optional prompt expression */
    char msg[256] = "";
    if (*cur && !is_eol_or_eof(*cur)) {
        ExprValue val = parse_expr(cur);
        char *s = val_to_string(&val);
        strncpy(msg, s, sizeof(msg) - 1);
        msg[sizeof(msg) - 1] = '\0';
        free(s);
        free_value(&val);
    }

    /* Expect TO */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
        (*cur) = (*cur)->next; /* skip TO */
    }

    /* Expect variable name */
    if (*cur && (*cur)->type == TOK_IDENT) {
        char *varname = (*cur)->value;
        (*cur) = (*cur)->next;

        /* Read input from user */
        char buf[256] = "";
        if (ui_is_active()) {
            nodelay(stdscr, FALSE);
            nocbreak();
            echo();
            curs_set(1);
            addstr(msg);
            refresh();
            getnstr(buf, sizeof(buf) - 1);
            curs_set(0);
            noecho();
            cbreak();
            addch('\n');
            refresh();
        } else {
            printf("%s", msg);
            fflush(stdout);
            if (fgets(buf, sizeof(buf), stdin)) {
                /* Strip trailing newline */
                size_t len = strlen(buf);
                if (len > 0 && buf[len - 1] == '\n')
                    buf[len - 1] = '\0';
            }
        }
        ExprValue v = val_string(buf);
        vars_set(varname, &v);
        free_value(&v);
    }

    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* WAIT [message]                                                      */
/* ------------------------------------------------------------------ */

static ExecStatus exec_wait(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "WAIT" */

    char msg[256] = "";
    if (*cur && !is_eol_or_eof(*cur)) {
        ExprValue val = parse_expr(cur);
        char *s = val_to_string(&val);
        strncpy(msg, s, sizeof(msg) - 1);
        msg[sizeof(msg) - 1] = '\0';
        free(s);
        free_value(&val);
    }
    skip_to_eol(cur);

    if (ui_is_active()) {
        nodelay(stdscr, FALSE);
        addstr(msg);
        refresh();
        getch();
    } else {
        printf("%s", msg);
        fflush(stdout);
    }
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* TEXT ... ENDTEXT                                                    */
/* ------------------------------------------------------------------ */
/* TEXT ... ENDTEXT                                                    */
/* ------------------------------------------------------------------ */

/* Expand &varname macros in a raw text line. */
static void expand_macros(const char *line, char *out, size_t out_size)
{
    size_t o = 0;
    const char *p = line;
    while (*p && o < out_size - 1) {
        if (*p == '&') {
            /* Scan variable name: alphanumeric + underscore */
            const char *name_start = p + 1;
            if (*name_start && (isalnum((unsigned char)*name_start) || *name_start == '_')) {
                const char *name_end = name_start;
                while (*name_end && (isalnum((unsigned char)*name_end) || *name_end == '_'))
                    name_end++;
                size_t nlen = name_end - name_start;
                char name[256];
                if (nlen >= sizeof(name)) nlen = sizeof(name) - 1;
                memcpy(name, name_start, nlen);
                name[nlen] = '\0';

                ExprValue val = vars_get(name);
                char *s = val_to_string(&val);
                size_t slen = strlen(s);
                if (o + slen < out_size - 1) {
                    memcpy(out + o, s, slen);
                    o += slen;
                }
                free(s);
                free_value(&val);
                p = name_end;
            } else {
                out[o++] = *p++;
            }
        } else {
            out[o++] = *p++;
        }
    }
    out[o] = '\0';
}

static ExecStatus exec_text(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "TEXT" */

    /* Each TOK_STRING token is one raw line from the TEXT block.
       Expand &var macros and print each line. */
    char expanded[2048];

    while (*cur && (*cur)->type != TOK_EOF) {
        /* Stop at ENDTEXT */
        if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDTEXT) {
            (*cur) = (*cur)->next;
            break;
        }

        if ((*cur)->type == TOK_STRING) {
            expand_macros((*cur)->value, expanded, sizeof(expanded));
            if (ui_is_active()) {
                /* Store for redraw and display at current text_row */
                TextEntry *te = calloc(1, sizeof(TextEntry));
                if (te) {
                    te->row = text_row;
                    strncpy(te->text, expanded, sizeof(te->text) - 1);
                    te->next = text_list;
                    text_list = te;
                }
                mvaddstr(text_row, 0, expanded);
                refresh();
                text_row++;
            } else {
                printf("%s\n", expanded);
                fflush(stdout);
            }
        }

        (*cur) = (*cur)->next;
    }

    /* Skip trailing EOL after ENDTEXT */
    if (*cur && (*cur)->type == TOK_EOL)
        (*cur) = (*cur)->next;

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* ZAP                                                                 */
/* ------------------------------------------------------------------ */

static ExecStatus exec_zap(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "ZAP" */
    wa_zap();
    /* Update DBF header last-update date */
    { DATABASEDBF *db = wa_db(); if (db) dbf_update_date(db); }
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* REPLACE [scope] <field> WITH <expr> [FOR cond] [WHILE cond]         */
/*   scope = ALL | NEXT n | RECORD n | REST                            */
/* ------------------------------------------------------------------ */

/* Replacement pair: field name + token range of its expression */
typedef struct {
    char fieldname[256];
    Token *expr_start;
    Token *expr_end; /* exclusive — one past last token of expression */
    int target_area; /* 0-based area index from alias (e.g. A->FIELD → 0), -1 = current */
} ReplacePair;

/* Advance scan past one expression, stopping at comma/keyword boundaries.
   Returns the token just after the expression (comma, FOR, WHILE, EOL, EOF). */
static Token *skip_expression(Token *scan)
{
    int paren_depth = 0;
    while (scan && !is_eol_or_eof(scan)) {
        if (scan->type == TOK_LPAREN) paren_depth++;
        else if (scan->type == TOK_RPAREN) { if (paren_depth > 0) paren_depth--; }
        else if (paren_depth == 0) {
            /* Stop at comma (next field pair), FOR, WHILE */
            if (scan->type == TOK_COMMA) break;
            if (scan->type == TOK_KEYWORD &&
                (scan->keyword_id == KW_FOR || scan->keyword_id == KW_WHILE))
                break;
        }
        scan = scan->next;
    }
    return scan;
}

/* Apply all replacement pairs to the current record */
static void apply_replacements(ReplacePair *pairs, int npairs)
{
    for (int i = 0; i < npairs; i++) {
        /* Switch to target area if specified (wa_select wants 1-based) */
        int saved_area = wa_get_selected();
        if (pairs[i].target_area >= 0) {
            wa_select(pairs[i].target_area + 1);
        }

        Token *rcur = pairs[i].expr_start;
        ExprValue val = parse_expr(&rcur);
        char *s = val_to_string(&val);
        wa_replace(pairs[i].fieldname, s);
        free(s);
        free_value(&val);

        /* Restore original area */
        wa_select(saved_area + 1);
    }
}

static ExecStatus exec_replace(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "REPLACE" */

    /* Parse scope modifier before field name */
    ScopeInfo si;
    si.scope_all = 0;
    si.scope_next = 0;
    si.scope_next_start = 0;
    si.scope_record = 0;
    si.scope_rest = 0;
    si.for_start = NULL;
    si.while_start = NULL;

    /* Check for scope keywords before field name */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALL) {
        si.scope_all = 1;
        *cur = (*cur)->next;
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT) {
        *cur = (*cur)->next;
        si.scope_next = 1;
        si.scope_next_start = wa_recno(); /* NEXT n starts at current record */
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si.scope_next = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_RECORD) {
        *cur = (*cur)->next;
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si.scope_record = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_REST) {
        si.scope_rest = 1;
        *cur = (*cur)->next;
    }

    if (!*cur || (*cur)->type != TOK_IDENT) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Collect all field WITH expression pairs (comma-separated) */
    ReplacePair pairs[64];
    int npairs = 0;

    while (*cur && (*cur)->type == TOK_IDENT && npairs < 64) {
        ReplacePair *p = &pairs[npairs];
        p->target_area = -1; /* default: current area */

        /* Check for alias->field: A->FIELD WITH expr */
        Token *saved = (*cur)->next;
        if (saved && saved->type == TOK_ARROW) {
            /* Resolve alias */
            p->target_area = wa_alias_to_area((*cur)->value);
            if (p->target_area < 0)
                p->target_area = wa_get_selected();
            *cur = saved->next; /* skip -> */
            /* Field name can be TOK_IDENT or TOK_KEYWORD */
            if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_KEYWORD)) {
                strncpy(p->fieldname, (*cur)->value, sizeof(p->fieldname) - 1);
                p->fieldname[sizeof(p->fieldname) - 1] = '\0';
                *cur = (*cur)->next;
            } else {
                /* Not a valid field after ->, skip and break */
                npairs--;
                break;
            }
        } else {
            strncpy(p->fieldname, (*cur)->value, sizeof(p->fieldname) - 1);
            p->fieldname[sizeof(p->fieldname) - 1] = '\0';
            *cur = (*cur)->next;
        }

        /* Skip "WITH" keyword */
        if (*cur && (*cur)->type == TOK_KEYWORD &&
            (*cur)->keyword_id == KW_WITH) {
            *cur = (*cur)->next;
        }

        p->expr_start = *cur;

        /* Advance past the expression to find its end */
        Token *scan = skip_expression(*cur);
        p->expr_end = scan;
        *cur = scan;

        npairs++;

        /* If next token is a comma, skip it and continue to next pair.
           EOL after comma is already removed by the tokenizer (line continuation). */
        if (*cur && (*cur)->type == TOK_COMMA) {
            *cur = (*cur)->next;
        }
    }

    if (npairs == 0) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Parse FOR/WHILE scope after all field pairs */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FOR) {
        si.for_start = (*cur)->next;
        /* Advance *cur past the FOR expression to find WHILE or EOL */
        int paren_depth = 0;
        while (*cur && !is_eol_or_eof(*cur)) {
            if ((*cur)->type == TOK_LPAREN) paren_depth++;
            else if ((*cur)->type == TOK_RPAREN) { if (paren_depth > 0) paren_depth--; }
            else if (paren_depth == 0 &&
                     (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE)
                break;
            *cur = (*cur)->next;
        }
    }

    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE) {
        si.while_start = (*cur)->next;
    }

    skip_to_eol(cur);

    DATABASEDBF *db = wa_db();
    if (!db)
        return EXEC_OK;

    int total = reccount(db);
    int saved_rec = wa_recno();

    if (si.scope_record) {
        /* REPLACE RECORD n */
        gotos(wa_db_ptr(), si.scope_record);
        apply_replacements(pairs, npairs);

    } else if (si.scope_all) {
        for (int rec = 1; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            apply_replacements(pairs, npairs);
        }

    } else if (si.scope_next) {
        /* REPLACE NEXT n */
        int start = si.scope_next_start;
        int count = si.scope_next;
        int last_valid_rec = 0;
        for (int i = 0, rec = start; i < count && rec <= total; i++, rec++) {
            gotos(wa_db_ptr(), rec);
            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) break;
                    continue;
                }
            }
            last_valid_rec = rec;
            apply_replacements(pairs, npairs);
        }
        if (last_valid_rec > 0)
            gotos(wa_db_ptr(), last_valid_rec);

    } else if (si.scope_rest) {
        /* REPLACE REST */
        int start = wa_recno();
        int last_valid_rec = 0;
        for (int rec = start; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) {
                        if (last_valid_rec > 0)
                            gotos(wa_db_ptr(), last_valid_rec);
                        break;
                    }
                    continue;
                }
            }
            last_valid_rec = rec;
            apply_replacements(pairs, npairs);
        }

    } else if (si.for_start || si.while_start) {
        int last_valid_rec = 0;
        for (int rec = 1; rec <= total; rec++) {
            gotos(wa_db_ptr(), rec);
            int stop = 0;
            if (!eval_scope(&si, &stop)) {
                if (stop) {
                    if (last_valid_rec > 0)
                        gotos(wa_db_ptr(), last_valid_rec);
                    break;
                }
                continue;
            }
            last_valid_rec = rec;
            apply_replacements(pairs, npairs);
        }

    } else {
        /* No scope — replace current record only */
        apply_replacements(pairs, npairs);
    }

    /* Update DBF header last-update date */
    dbf_update_date(db);

    if (!si.scope_next && !si.scope_rest)
        gotos(wa_db_ptr(), saved_rec);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* AVERAGE field1, field2 ... [scope] [FOR expr] [WHILE expr] [TO var] */
/* ------------------------------------------------------------------ */

static ExecStatus exec_average(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "AVERAGE" */

    /* Parse scope modifier before field list */
    ScopeInfo si;
    si.scope_all = 0;
    si.scope_next = 0;
    si.scope_next_start = 0;
    si.scope_record = 0;
    si.scope_rest = 0;
    si.for_start = NULL;
    si.while_start = NULL;

    /* Check for scope keywords before field list */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALL) {
        si.scope_all = 1;
        *cur = (*cur)->next;
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT) {
        *cur = (*cur)->next;
        si.scope_next = 1;
        si.scope_next_start = wa_recno();
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si.scope_next = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_RECORD) {
        *cur = (*cur)->next;
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si.scope_record = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_REST) {
        si.scope_rest = 1;
        *cur = (*cur)->next;
    }

    /* Collect comma-separated field expressions */
    Token *exprs[64];
    int nexprs = 0;

    if (*cur && (*cur)->type == TOK_IDENT) {
        Token *scan = skip_expression(*cur);
        exprs[nexprs++] = *cur;
        *cur = scan;

        while (*cur && (*cur)->type == TOK_COMMA && nexprs < 64) {
            *cur = (*cur)->next; /* skip comma */
            exprs[nexprs++] = *cur;
            scan = skip_expression(*cur);
            *cur = scan;
        }
    }

    /* Parse FOR/WHILE after field list */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FOR) {
        si.for_start = (*cur)->next;
        int paren_depth = 0;
        while (*cur && !is_eol_or_eof(*cur)) {
            if ((*cur)->type == TOK_LPAREN) paren_depth++;
            else if ((*cur)->type == TOK_RPAREN) { if (paren_depth > 0) paren_depth--; }
            else if (paren_depth == 0 &&
                     (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE)
                break;
            *cur = (*cur)->next;
        }
    }

    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE) {
        si.while_start = (*cur)->next;
    }

    /* Parse optional TO var */
    char target_var[64] = {0};
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
        *cur = (*cur)->next; /* skip "TO" */
        if (*cur && (*cur)->type == TOK_IDENT) {
            strncpy(target_var, (*cur)->value, sizeof(target_var) - 1);
        }
    }

    skip_to_eol(cur);

    DATABASEDBF *db = wa_db();
    if (!db)
        return EXEC_OK;

    int total = reccount(db);
    int saved_rec = wa_recno();

    /* Accumulate sums and counts for each expression */
    double sums[64];
    int counts[64];
    memset(sums, 0, sizeof(sums));
    memset(counts, 0, sizeof(counts));

    int start_rec = 1;
    int end_rec = total;

    if (si.scope_record) {
        start_rec = si.scope_record;
        end_rec = si.scope_record;
    } else if (si.scope_next) {
        start_rec = si.scope_next_start;
        end_rec = start_rec + si.scope_next - 1;
        if (end_rec > total) end_rec = total;
    } else if (si.scope_rest) {
        start_rec = wa_recno();
        end_rec = total;
    } else if (nexprs == 0) {
        /* AVERAGE with no fields and no scope = ALL records */
        start_rec = 1;
        end_rec = total;
    } else {
        /* AVERAGE with fields but no scope = ALL records */
        start_rec = 1;
        end_rec = total;
    }

    if (nexprs > 0) {
        /* Specific field expressions */
        for (int rec = start_rec; rec <= end_rec; rec++) {
            gotos(wa_db_ptr(), rec);

            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) break;
                    continue;
                }
            }

            for (int i = 0; i < nexprs; i++) {
                Token *ecur = exprs[i];
                ExprValue ev = parse_expr(&ecur);
                if (ev.type == VAL_INTEGER || ev.type == VAL_REAL) {
                    sums[i] += ev.data.rval;
                    counts[i]++;
                }
            }
        }
    } else {
        /* No fields specified — average all numeric fields */
        double field_sums[128];
        int field_counts[128];
        memset(field_sums, 0, sizeof(field_sums));
        memset(field_counts, 0, sizeof(field_counts));

        for (int rec = start_rec; rec <= end_rec; rec++) {
            gotos(wa_db_ptr(), rec);

            if (si.for_start || si.while_start) {
                int stop = 0;
                if (!eval_scope(&si, &stop)) {
                    if (stop) break;
                    continue;
                }
            }

            for (int f = 0; f < db->camposn; f++) {
                char ftype = db->fields.tipos[f];
                if (ftype == 'N' || ftype == 'n' || ftype == 'F' || ftype == 'f') {
                    char *fval = NULL;
                    get_field(db, f + 1, &fval);
                    if (fval && *fval) {
                        field_sums[f] += atof(fval);
                        field_counts[f]++;
                    }
                }
            }
        }

        /* Print results for all numeric fields */
        for (int f = 0; f < db->camposn; f++) {
            char ftype = db->fields.tipos[f];
            if (ftype == 'N' || ftype == 'n' || ftype == 'F' || ftype == 'f') {
                double avg = (field_counts[f] > 0) ? (field_sums[f] / field_counts[f]) : 0.0;
                if (strlen(target_var) > 0 && f == 0) {
                    ExprValue v = val_real(avg);
                    vars_set(target_var, &v);
                } else {
                    { char buf[128]; snprintf(buf, sizeof(buf), "%-12s %g\n", db->fields.names[f], avg); addstr(buf); }
                }
            }
        }

        if (!si.scope_next && !si.scope_rest)
            gotos(wa_db_ptr(), saved_rec);
        return EXEC_OK;
    }

    if (strlen(target_var) > 0 && nexprs > 0) {
        /* Store result in variable */
        double avg = (counts[0] > 0) ? (sums[0] / counts[0]) : 0.0;
        ExprValue v = val_real(avg);
        vars_set(target_var, &v);
    } else {
        /* Print results */
        for (int i = 0; i < nexprs; i++) {
            double avg = (counts[i] > 0) ? (sums[i] / counts[i]) : 0.0;
            { char buf[128]; snprintf(buf, sizeof(buf), "Average: %g\n", avg); addstr(buf); }
        }
    }

    if (!si.scope_next && !si.scope_rest)
        gotos(wa_db_ptr(), saved_rec);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* APPEND BLANK                                                        */
/* ------------------------------------------------------------------ */

static ExecStatus exec_append(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "APPEND" */
    wa_append_blank();
    /* Update DBF header last-update date */
    { DATABASEDBF *db = wa_db(); if (db) dbf_update_date(db); }
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* DISPLAY MEMORY [TO <file>]                                          */
/* ------------------------------------------------------------------ */

static ExecStatus exec_display_memory(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "MEMORY" */

    /* Optional: TO <file> */
    FILE *out_file = NULL;
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
        *cur = (*cur)->next; /* skip "TO" */
        if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING)) {
            char fname[512];
            strncpy(fname, (*cur)->value, sizeof(fname) - 1);
            fname[sizeof(fname) - 1] = '\0';
            *cur = (*cur)->next;
            /* Handle dot-skipping: if next token is also ident/keyword, append */
            if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_KEYWORD)) {
                strncat(fname, ".", sizeof(fname) - strlen(fname) - 1);
                strncat(fname, (*cur)->value, sizeof(fname) - strlen(fname) - 1);
                *cur = (*cur)->next;
            }
            /* Force lowercase for writes */
            force_lowercase(fname, fname, sizeof(fname));
            out_file = fopen(fname, "w");
        }
    }

    int total = vars_count();

    for (int i = 0; i < total; i++) {
        const char *name;
        ExprValue val;
        if (!vars_get_by_index(i, &name, &val))
            continue;

        char *s = val_to_string(&val);
        const char *type_name = val_type_name(val.type);

        char line[512];
        snprintf(line, sizeof(line), "%-16s %-6s %s", name, type_name, s ? s : "");

        if (out_file) {
            fputs(line, out_file);
            fputc('\n', out_file);
        } else {
            if (ui_is_active()) {
                ui_print(line);
                ui_print_newline();
            } else {
                printf("%s\n", line);
            }
        }

        free(s);
        free_value(&val);
    }

    if (out_file) {
        fflush(out_file);
        fclose(out_file);
    } else if (ui_is_active()) {
        ui_refresh();
    }

    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* DISPLAY STRUCTURE                                                   */
/* ------------------------------------------------------------------ */

static ExecStatus exec_display(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "DISPLAY" */

    /* DISPLAY MEMORY [TO <file>] */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_MEMORY) {
        return exec_display_memory(cur);
    }

    /* Expect "STRUCTURE" or "STATUS" — skip it */
    if (*cur && !is_eol_or_eof(*cur)) {
        *cur = (*cur)->next;
    }
    DATABASEDBF *db = wa_db();
    if (!db) {
        if (ui_is_active()) {
            ui_print("No database in use");
            ui_print_newline();
            ui_refresh();
        } else {
            printf("No database in use\n");
        }
    } else {
        /* ncurses-aware display_structure */
        int row = 0;
        char line[256];
        int total_size = 0;

        if (ui_is_active()) {
            /* DB name */
            snprintf(line, sizeof(line), "DB name: %s", db->name);
            ui_print(line); ui_print_newline();

            /* Record count */
            snprintf(line, sizeof(line), "Recs: %10d", (int)db->recnos);
            ui_print(line); ui_print_newline();

            /* Date */
            snprintf(line, sizeof(line), "Last Update Date: %s", db->date);
            ui_print(line); ui_print_newline();

            /* Header */
            ui_print("Field  Name      Type   Size    Decimals");
            ui_print_newline();

            /* Fields */
            for (int i = 1; i <= db->camposn; i++) {
                char fname[12] = "";
                for (int j = 0; j <= 10; j++) {
                    if (db->fields.names[j][i])
                        fname[j] = db->fields.names[j][i];
                    else
                        fname[j] = ' ';
                }
                fname[11] = '\0';
                total_size += db->fields.longitudes[i];

                snprintf(line, sizeof(line), "%5d  %-12s%c      %2d%s",
                         i, fname, db->fields.tipos[i],
                         db->fields.longitudes[i],
                         (db->fields.tipos[i] == 'N')
                             ? ("        " + 8 - (int)snprintf(NULL, 0, "%d", db->fields.decimales[i]))
                             : "");

                /* Simpler approach for decimals */
                if (db->fields.tipos[i] == 'N') {
                    char dec[16];
                    snprintf(dec, sizeof(dec), "        %d", db->fields.decimales[i]);
                    strcat(line, dec);
                }
                ui_print(line); ui_print_newline();
            }

            /* Total */
            snprintf(line, sizeof(line), "** Total **%17d", total_size);
            ui_print(line); ui_print_newline();
            ui_refresh();
        } else {
            /* Fallback to library function */
            display_structure(db);
        }
    }
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* LIST [FIELD fields] [scope] [FOR expr] [WHILE expr]                 */
/* Paginated ncurses output with "-- more --" prompt                    */
/* ------------------------------------------------------------------ */

static ExecStatus exec_list(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "LIST" */

    /* Parse optional field list: FIELD <names> or bare <name>[,<name>...]
       Stop at scope keywords (ALL, NEXT, RECORD, REST, FOR, WHILE) */
    int *field_list = NULL;
    int field_count = 0;
    int show_all_fields = 1;

    /* Optional FIELD keyword */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FIELD) {
        *cur = (*cur)->next; /* skip "FIELD" */
        show_all_fields = 0;
    }

    /* Collect field names until scope keyword or EOL */
    while (*cur && !is_eol_or_eof(*cur)) {
        /* Stop at scope keywords */
        if ((*cur)->type == TOK_KEYWORD) {
            KeywordId kw = (*cur)->keyword_id;
            if (kw == KW_ALL || kw == KW_NEXT || kw == KW_RECORD ||
                kw == KW_REST || kw == KW_FOR || kw == KW_WHILE)
                break;
        }
        if ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_KEYWORD) {
            show_all_fields = 0;
            int *tmp = realloc(field_list, (size_t)(field_count + 1) * sizeof(int));
            if (tmp) {
                field_list = tmp;
                char *fname = strdup((*cur)->value);
                int fc = wa_field_count();
                for (int i = 1; i <= fc; i++) {
                    char *n = wa_field_name(i);
                    if (port_strcasecmp(n, fname) == 0) {
                        field_list[field_count++] = i;
                        break;
                    }
                    free(n);
                }
                free(fname);
            }
            *cur = (*cur)->next;
        } else if ((*cur)->type == TOK_COMMA) {
            show_all_fields = 0;
            *cur = (*cur)->next;
        } else {
            break;
        }
    }

    /* Parse scope */
    ScopeInfo si;
    parse_scope(cur, &si);
    /* LIST/DISPLAY with FOR/WHILE implicitly means ALL scope */
    if (si.for_start || si.while_start)
        si.scope_all = 1;
    skip_to_eol(cur);

    DATABASEDBF *db = wa_db();
    if (!db)
        return EXEC_OK;

    int total_fields = wa_field_count();
    if (show_all_fields) {
        field_list = malloc((size_t)total_fields * sizeof(int));
        for (int i = 0; i < total_fields; i++)
            field_list[i] = i + 1;
        field_count = total_fields;
    }

    if (!field_list || field_count == 0) {
        free(field_list);
        return EXEC_OK;
    }

    /* Determine iteration range */
    int saved_rec = wa_recno();
    int start_rec = 0, end_rec = 0;
    int iterate = 0; /* 1 if we need to loop through records */

    if (si.scope_record) {
        wa_goto(si.scope_record);
    } else if (si.scope_all) {
        wa_goto_top();
        start_rec = 1;
        end_rec = (int)db->recnos;
        iterate = 1;
    } else if (si.scope_next) {
        start_rec = si.scope_next_start;
        end_rec = start_rec + si.scope_next - 1;
        iterate = 1;
    } else if (si.scope_rest) {
        start_rec = wa_recno();
        end_rec = (int)db->recnos;
        iterate = 1;
    } else {
        /* No scope — LIST defaults to ALL in dBASE */
        wa_goto_top();
        start_rec = 1;
        end_rec = (int)db->recnos;
        iterate = 1;
    }

    /* Paginated display */
    int page_rows = LINES > 0 ? LINES - 3 : 20;
    if (page_rows < 5) page_rows = 5;
    int current_row = 0;

    /* Display loop */
    int rec;
    if (iterate) {
        for (rec = start_rec; rec <= end_rec; rec++) {
            if (rec < 1 || rec > (int)db->recnos)
                continue;
            gotos(&db, rec);

            /* Check deleted */
            if (is_deleted(db) == VERITAS)
                continue;

            /* Check scope conditions */
            {
                int stop_scan = 0;
                if (!eval_scope(&si, &stop_scan)) {
                    if (stop_scan) break;
                    continue;
                }
            }

            /* Build the line */
            {
                char line[4096] = "";
                int first = 1;
                for (int fi = 0; fi < field_count; fi++) {
                    int fidx = field_list[fi];
                    char *val = wa_get_field(fidx);
                    char *name = wa_field_name(fidx);
                    if (!first) strcat(line, "  ");
                    strcat(line, name);
                    strcat(line, "=");
                    if (val) strcat(line, val);
                    free(val);
                    free(name);
                    first = 0;
                }
                addstr(line);
                addch('\n');
                refresh();
                current_row++;
            }

            if (current_row >= page_rows) {
                addstr("          -- more --");
                refresh();
                int ch = getch();
                if (ch == 27 || ch == 'q' || ch == 'Q')
                    break;
                addch('\n');
                current_row = 0;
                refresh();
            }
        }
    } else {
        /* Single record (current or RECORD n) */
        rec = wa_recno();
        if (rec >= 1 && rec <= (int)db->recnos) {
            gotos(&db, rec);
            if (is_deleted(db) != VERITAS) {
                char line[4096] = "";
                int first = 1;
                for (int fi = 0; fi < field_count; fi++) {
                    int fidx = field_list[fi];
                    char *val = wa_get_field(fidx);
                    char *name = wa_field_name(fidx);
                    if (!first) strcat(line, "  ");
                    strcat(line, name);
                    strcat(line, "=");
                    if (val) strcat(line, val);
                    free(val);
                    free(name);
                    first = 0;
                }
                addstr(line);
                addch('\n');
                refresh();
            }
        }
    }

    refresh();
    free(field_list);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* BROWSE [FIELDS f1,f2,...] [FOR expr]                                */
/* ------------------------------------------------------------------ */

static ExecStatus exec_browse(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "BROWSE" */

    /* Parse optional FIELDS clause */
    char fields_buf[512] = "";
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FIELD) {
        *cur = (*cur)->next; /* skip "FIELD" */
        int first = 1;
        while (*cur && !is_eol_or_eof(*cur)) {
            if ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_KEYWORD) {
                if (!first) strcat(fields_buf, ",");
                strcat(fields_buf, (*cur)->value);
                first = 0;
                *cur = (*cur)->next;
            } else if ((*cur)->type == TOK_COMMA) {
                *cur = (*cur)->next;
            } else {
                break;
            }
        }
    }

    /* Skip optional FOR expr — BROWSE handles filtering internally */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FOR) {
        *cur = (*cur)->next; /* skip "FOR" */
        /* Evaluate and skip the expression */
        while (*cur && !is_eol_or_eof(*cur)) {
            ExprValue val = parse_expr(cur);
            free_value(&val);
        }
    }

    skip_to_eol(cur);

    if (ui_is_active()) {
        ui_browse(fields_buf[0] ? fields_buf : NULL);
    } else {
        /* Fallback: just do LIST */
        /* Not ideal, but works in non-ncurses mode */
    }

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* SEEK <expr>                                                         */
/* ------------------------------------------------------------------ */

static ExecStatus exec_seek(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "SEEK" */
    if (!*cur || is_eol_or_eof(*cur)) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    ExprValue val = parse_expr(cur);
    char *s = val_to_string(&val);
    wa_seek(s);
    free(s);
    free_value(&val);

    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* SELECT <n> — switch work area (1-based)                             */
/* ------------------------------------------------------------------ */

static ExecStatus exec_select(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "SELECT" */
    int area = 1;
    if (*cur && (*cur)->type == TOK_INTEGER) {
        area = atoi((*cur)->value);
        *cur = (*cur)->next;
    }
    wa_select(area);
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* STORE <expr> TO <var1>, <var2>, ...                                 */
/* ------------------------------------------------------------------ */

static ExecStatus exec_store(Token **cur)
{
    Token *t = *cur;
    t = t->next; /* skip "STORE" */

    /* Parse the expression to store */
    ExprValue val = parse_expr(&t);

    /* Expect TO keyword */
    if (!t || t->type != TOK_KEYWORD || t->keyword_id != KW_TO) {
        free_value(&val);
        skip_to_eol(cur);
        return EXEC_OK;
    }
    t = t->next; /* skip "TO" */

    /* Assign to each variable in the comma-separated list */
    while (t && !is_eol_or_eof(t)) {
        if (t->type == TOK_IDENT) {
            vars_set(t->value, &val);
            t = t->next;
            /* Skip comma if present */
            if (t && t->type == TOK_COMMA) {
                t = t->next;
            }
        } else {
            break;
        }
    }

    free_value(&val);
    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* @ row,col SAY <expr>                                                */
/* @ row,col GET <var> [PICTURE "mask"] [RANGE lo,hi] [VALID expr]     */
/*             [DEFAULT expr] [FOCUS]                                  */
/* @ row1,col1 TO @ row2,col2 [DOUBLE]                                 */
/* @ row1,col1 CLEAR [TO @ row2,col2]                                  */
/* ------------------------------------------------------------------ */

static ExecStatus exec_at(Token **cur)
{
    Token *t = *cur;
    t = t->next; /* skip "@" */

    /* Parse row (integer expression) */
    Token *rcur = t;
    ExprValue row_val = parse_expr(&rcur);
    int row = (int)val_to_double(&row_val);
    free_value(&row_val);

    /* Expect comma */
    if (!rcur || rcur->type != TOK_COMMA) {
        skip_to_eol(cur);
        return EXEC_OK;
    }
    rcur = rcur->next; /* skip "," */

    /* Parse col (integer expression) */
    ExprValue col_val = parse_expr(&rcur);
    int col = (int)val_to_double(&col_val);
    free_value(&col_val);

    /* Expect SAY, GET, TO, or CLEAR */
    if (!rcur || rcur->type != TOK_KEYWORD) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* @...TO @... — draw rectangle */
    if (rcur->keyword_id == KW_TO) {
        rcur = rcur->next; /* skip "TO" */
        /* Expect "@" */
        if (rcur && rcur->type == TOK_AT) {
            rcur = rcur->next; /* skip "@" */
        }
        /* Parse row2 */
        ExprValue row2_val = parse_expr(&rcur);
        int row2 = (int)val_to_double(&row2_val);
        free_value(&row2_val);
        /* Expect comma */
        if (rcur && rcur->type == TOK_COMMA) {
            rcur = rcur->next;
        }
        /* Parse col2 */
        ExprValue col2_val = parse_expr(&rcur);
        int col2 = (int)val_to_double(&col2_val);
        free_value(&col2_val);

        /* Check for optional DOUBLE */
        int double_line = 0;
        if (rcur && rcur->type == TOK_KEYWORD && rcur->keyword_id == KW_DOUBLE) {
            double_line = 1;
        }

        ui_rect(row, col, row2, col2, double_line);
        ui_refresh();
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* @...CLEAR [TO @...] — clear region */
    if (rcur->keyword_id == KW_CLEAR) {
        rcur = rcur->next; /* skip "CLEAR" */
        /* Check for optional "TO @" */
        if (rcur && rcur->type == TOK_KEYWORD && rcur->keyword_id == KW_TO) {
            rcur = rcur->next; /* skip "TO" */
            /* Expect "@" */
            if (rcur && rcur->type == TOK_AT) {
                rcur = rcur->next; /* skip "@" */
            }
            /* Parse row2 */
            ExprValue row2_val = parse_expr(&rcur);
            int row2 = (int)val_to_double(&row2_val);
            free_value(&row2_val);
            /* Expect comma */
            if (rcur && rcur->type == TOK_COMMA) {
                rcur = rcur->next;
            }
            /* Parse col2 */
            ExprValue col2_val = parse_expr(&rcur);
            int col2 = (int)val_to_double(&col2_val);
            free_value(&col2_val);
            ui_clear_rect(row, col, row2, col2);
        } else {
            /* Clear from (row,col) to bottom-right of screen */
            int maxy = LINES - 1;
            int maxx = COLS - 1;
            ui_clear_rect(row, col, maxy, maxx);
        }
        ui_refresh();
        skip_to_eol(cur);
        return EXEC_OK;
    }

    if (rcur->keyword_id == KW_SAY) {
        /* @...SAY <expr> [GET <var>] */
        rcur = rcur->next; /* skip "SAY" */
        ExprValue val = parse_expr(&rcur);
        char *s = val_to_string(&val);
        ui_say(row, col, s);
        ui_refresh();
        free(s);
        free_value(&val);

        /* Check for optional GET after SAY */
        if (rcur && rcur->type == TOK_KEYWORD && rcur->keyword_id == KW_GET) {
            rcur = rcur->next; /* skip "GET" */
            if (rcur && rcur->type == TOK_IDENT) {
                char varname[64];
                strncpy(varname, rcur->value, sizeof(varname) - 1);
                varname[sizeof(varname) - 1] = '\0';
                rcur = rcur->next;
                ui_get_add(row, col, varname, 10);
            }
        }
        skip_to_eol(cur);
        return EXEC_OK;

    } else if (rcur->keyword_id == KW_GET) {
        /* @...GET <var> [options] */
        rcur = rcur->next; /* skip "GET" */

        /* Parse variable name */
        if (!rcur || rcur->type != TOK_IDENT) {
            skip_to_eol(cur);
            return EXEC_OK;
        }
        char varname[64];
        strncpy(varname, rcur->value, sizeof(varname) - 1);
        varname[sizeof(varname) - 1] = '\0';
        rcur = rcur->next;

        /* Scan ahead for PICTURE to determine field length */
        int fld_len = 10;
        {
            Token *scan = rcur;
            while (scan && !is_eol_or_eof(scan)) {
                if (scan->type == TOK_KEYWORD && scan->keyword_id == KW_PICTURE) {
                    Token *pval = scan->next;
                    if (pval && pval->type == TOK_STRING) {
                        int pic_len = (int)strlen(pval->value);
                        if (pic_len > fld_len)
                            fld_len = pic_len;
                    }
                    break;
                }
                scan = scan->next;
            }
        }

        UiGetField *gf = ui_get_add(row, col, varname, fld_len);
        (void)gf; /* suppress unused warning */

        /* Parse optional clauses: PICTURE, RANGE, VALID, DEFAULT, FOCUS */
        while (rcur && !is_eol_or_eof(rcur)) {
            if (rcur->type == TOK_KEYWORD) {
                switch (rcur->keyword_id) {
                    case KW_PICTURE:
                        rcur = rcur->next; /* skip "PICTURE" */
                        if (rcur && rcur->type == TOK_STRING) {
                            ui_get_set_picture(rcur->value);
                            rcur = rcur->next;
                        }
                        break;
                    case KW_RANGE:
                        rcur = rcur->next; /* skip "RANGE" */
                        {
                            char lo_str[64] = "";
                            char hi_str[64] = "";

                            /* Optional low bound — skip if next token is comma */
                            if (rcur && !is_eol_or_eof(rcur) && rcur->type != TOK_COMMA) {
                                ExprValue lo_val = parse_expr(&rcur);
                                snprintf(lo_str, sizeof(lo_str), "%g", val_to_double(&lo_val));
                                free_value(&lo_val);
                            }
                            /* Skip comma if present */
                            if (rcur && rcur->type == TOK_COMMA)
                                rcur = rcur->next;
                            /* Optional high bound — skip if at EOL/EOF or next keyword */
                            if (rcur && !is_eol_or_eof(rcur) && rcur->type != TOK_KEYWORD) {
                                ExprValue hi_val = parse_expr(&rcur);
                                snprintf(hi_str, sizeof(hi_str), "%g", val_to_double(&hi_val));
                                free_value(&hi_val);
                            }
                            ui_get_set_range(lo_str, hi_str);
                        }
                        break;
                    case KW_VALID:
                        rcur = rcur->next; /* skip "VALID" */
                        {
                            Token *vstart = rcur;
                            Token *vscan = rcur;
                            int pd = 0;
                            while (vscan && !is_eol_or_eof(vscan)) {
                                if (vscan->type == TOK_LPAREN) pd++;
                                else if (vscan->type == TOK_RPAREN) { if (pd > 0) pd--; }
                                else if (pd == 0 && vscan->type == TOK_KEYWORD)
                                    break;
                                vscan = vscan->next;
                            }
                            char expr[256] = "";
                            Token *e = vstart;
                            while (e && e != vscan) {
                                strncat(expr, e->value, sizeof(expr) - strlen(expr) - 1);
                                strncat(expr, " ", sizeof(expr) - strlen(expr) - 1);
                                e = e->next;
                            }
                            ui_get_set_valid(expr);
                            rcur = vscan;
                        }
                        break;
                    case KW_DEFAULT:
                        rcur = rcur->next; /* skip "DEFAULT" */
                        {
                            ExprValue dval = parse_expr(&rcur);
                            char *ds = val_to_string(&dval);
                            ui_get_set_default(ds);
                            free(ds);
                            free_value(&dval);
                        }
                        break;
                    case KW_FOCUS:
                        rcur = rcur->next; /* skip "FOCUS" */
                        ui_get_set_focus();
                        break;
                    default:
                        goto done_options;
                }
            } else {
                goto done_options;
            }
        }
        done_options:;
    }

    skip_to_eol(cur);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* READ — process all pending @...GET fields                           */
/* ------------------------------------------------------------------ */

static ExecStatus exec_read(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "READ" */
    skip_to_eol(cur);
    ui_read();
    ui_get_clear();
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* CLEAR — clear screen                                                */
/* ------------------------------------------------------------------ */

static ExecStatus exec_clear(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "CLEAR" */
    skip_to_eol(cur);
    ui_clear();
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* CLEAR ALL                                                           */
/* Closes all databases, clears all memory variables, resets workareas */
/* ------------------------------------------------------------------ */

static ExecStatus exec_clear_all(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "CLEAR ALL" */
    skip_to_eol(cur);

    /* Close all workareas (frees DBF handles, resets aliases) */
    wa_close_all();

    /* Clear all memory variables */
    vars_init();

    /* Reset the screen */
    ui_clear();

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* CLEAR MEMORY                                                        */
/* Clears all memory variables only — databases stay open              */
/* ------------------------------------------------------------------ */

static ExecStatus exec_clear_memory(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "CLEAR MEMORY" */
    skip_to_eol(cur);

    /* Clear all memory variables — databases remain open */
    vars_init();

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* COUNT [NEXT n] [FOR expr] [WHILE expr] [TO var]                    */
/* ------------------------------------------------------------------ */

static ExecStatus exec_count(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "COUNT" */

    /* Parse scope modifier */
    ScopeInfo si;
    si.scope_all = 0;
    si.scope_next = 0;
    si.scope_next_start = 0;
    si.scope_record = 0;
    si.scope_rest = 0;
    si.for_start = NULL;
    si.while_start = NULL;

    /* Check for NEXT n */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT) {
        *cur = (*cur)->next;
        si.scope_next = 1;
        si.scope_next_start = wa_recno();
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si.scope_next = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALL) {
        si.scope_all = 1;
        *cur = (*cur)->next;
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_REST) {
        si.scope_rest = 1;
        *cur = (*cur)->next;
    } else if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_RECORD) {
        *cur = (*cur)->next;
        if (*cur && (*cur)->type == TOK_INTEGER) {
            si.scope_record = atoi((*cur)->value);
            *cur = (*cur)->next;
        }
    }

    /* Parse FOR <expr> */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FOR) {
        si.for_start = (*cur)->next;
        int paren_depth = 0;
        while (*cur && !is_eol_or_eof(*cur)) {
            if ((*cur)->type == TOK_LPAREN) paren_depth++;
            else if ((*cur)->type == TOK_RPAREN) { if (paren_depth > 0) paren_depth--; }
            else if (paren_depth == 0 &&
                     (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE)
                break;
            *cur = (*cur)->next;
        }
    }

    /* Parse WHILE <expr> */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_WHILE) {
        si.while_start = (*cur)->next;
    }

    /* Parse optional TO var */
    char target_var[64] = {0};
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
        *cur = (*cur)->next;
        if (*cur && (*cur)->type == TOK_IDENT) {
            strncpy(target_var, (*cur)->value, sizeof(target_var) - 1);
        }
    }

    skip_to_eol(cur);

    DATABASEDBF *db = wa_db();
    if (!db)
        return EXEC_OK;

    int total = reccount(db);
    int saved_rec = wa_recno();

    int start_rec = 1;
    int end_rec = total;

    if (si.scope_record) {
        start_rec = si.scope_record;
        end_rec = si.scope_record;
    } else if (si.scope_next) {
        start_rec = si.scope_next_start;
        end_rec = start_rec + si.scope_next - 1;
        if (end_rec > total) end_rec = total;
    } else if (si.scope_rest) {
        start_rec = wa_recno();
        end_rec = total;
    }

    int count = 0;

    for (int rec = start_rec; rec <= end_rec; rec++) {
        gotos(wa_db_ptr(), rec);

        if (si.for_start || si.while_start) {
            int stop = 0;
            if (!eval_scope(&si, &stop)) {
                if (stop) break;
                continue;
            }
        }

        count++;
    }

    /* Store or print result */
    if (strlen(target_var) > 0) {
        ExprValue v = val_real((double)count);
        vars_set(target_var, &v);
    } else {
        char buf[128];
        snprintf(buf, sizeof(buf), "Records counted: %d\n", count);
        addstr(buf);
    }

    if (!si.scope_next && !si.scope_rest)
        gotos(wa_db_ptr(), saved_rec);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* INDEX ON <expression> TO <filename>                                 */
/* ------------------------------------------------------------------ */

/* Helper: compare IndexEntry for qsort */
typedef struct {
    char *key;
    int   recno;
} _IdxEntry;

static int _cmp_idx(const void *a, const void *b)
{
    const _IdxEntry *ea = (const _IdxEntry *)a;
    const _IdxEntry *eb = (const _IdxEntry *)b;
    int c = port_strcasecmp(ea->key, eb->key);
    if (c != 0) return c;
    return ea->recno - eb->recno;
}

static ExecStatus exec_index(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "INDEX" */

    /* Expect "ON" */
    if (!*cur || !((*cur)->type == TOK_IDENT &&
        port_strcasecmp((*cur)->value, "on") == 0)) {
        fprintf(stderr, "prg: expected INDEX ON\n");
        skip_to_eol(cur);
        return EXEC_OK;
    }
    *cur = (*cur)->next; /* skip "ON" */

    /* Parse the key expression — save its token range for re-evaluation */
    Token *expr_start = *cur;
    /* We need to figure out where the expression ends (before "TO").
     * Scan ahead to find "TO" (case-insensitive IDENT). */
    {
        Token *scan = *cur;
        while (scan && scan->type != TOK_EOF && scan->type != TOK_EOL) {
            if (scan->type == TOK_IDENT &&
                port_strcasecmp(scan->value, "to") == 0) {
                /* Expression ends before this token */
                break;
            }
            scan = scan->next;
        }
    }

    /* Evaluate the expression once to determine key type/length */
    ExprValue val = parse_expr(cur);
    char *sample_key = val_to_string(&val);
    int key_len = (int)strlen(sample_key);
    if (key_len < 1) key_len = 10;
    free(sample_key);
    free_value(&val);

    /* Expect "TO" — can be TOK_KEYWORD (KW_TO) or TOK_IDENT */
    int found_to = 0;
    if (*cur) {
        if ((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO)
            found_to = 1;
        else if ((*cur)->type == TOK_IDENT && port_strcasecmp((*cur)->value, "to") == 0)
            found_to = 1;
    }
    if (!found_to) {
        fprintf(stderr, "prg: expected TO in INDEX ON\n");
        skip_to_eol(cur);
        return EXEC_OK;
    }
    *cur = (*cur)->next; /* skip "TO" */

    /* Get filename */
    char filename[1024];
    if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING)) {
        strncpy(filename, (*cur)->value, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        *cur = (*cur)->next;
    } else {
        fprintf(stderr, "prg: expected filename in INDEX ON\n");
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Ensure .ndx extension */
    char path[1024];
    if (strchr(filename, '.') == NULL) {
        snprintf(path, sizeof(path), "%s.ndx", filename);
    } else {
        strncpy(path, filename, sizeof(path) - 1);
        path[sizeof(path) - 1] = '\0';
    }

    skip_to_eol(cur);

    /* Collect keys for all records */
    DATABASEDBF *db = wa_db();
    if (!db) {
        fprintf(stderr, "prg: no database open\n");
        return EXEC_OK;
    }

    int total = reccount(db);
    _IdxEntry *entries = calloc(total, sizeof(_IdxEntry));
    if (!entries) {
        fprintf(stderr, "prg: out of memory for index\n");
        return EXEC_OK;
    }

    int entry_count = 0;

    /* Save current position */
    int saved_current = wa_recno();

    for (int rec = 1; rec <= total; rec++) {
        gotos(wa_db_ptr(), rec);

        /* Re-evaluate the expression for this record */
        Token *expr_cur = expr_start;
        ExprValue v = parse_expr(&expr_cur);
        char *key = val_to_string(&v);

        if (key && *key) {
            /* Update key_len if we see a longer key */
            int klen = (int)strlen(key);
            if (klen > key_len) key_len = klen;

            entries[entry_count].key = key;
            entries[entry_count].recno = rec;
            entry_count++;
        } else {
            free(key);
        }
        free_value(&v);
    }

    /* Restore position */
    gotos(wa_db_ptr(), saved_current);

    if (entry_count == 0) {
        free(entries);
        fprintf(stderr, "prg: no keys generated for index\n");
        return EXEC_OK;
    }

    /* Sort entries */
    qsort(entries, entry_count, sizeof(_IdxEntry), _cmp_idx);

    /* Build arrays for the generic library function */
    char **keys = malloc(entry_count * sizeof(char *));
    int *recnos = malloc(entry_count * sizeof(int));
    if (!keys || !recnos) {
        for (int i = 0; i < entry_count; i++) free(entries[i].key);
        free(entries);
        free(keys); free(recnos);
        return EXEC_OK;
    }

    for (int i = 0; i < entry_count; i++) {
        keys[i] = entries[i].key;
        recnos[i] = entries[i].recno;
    }
    free(entries);

    /* Create the index file */
    int rc = create_index_ndx_generic(keys, recnos, entry_count,
                                       key_len, "INDEX", path);

    free(keys);
    free(recnos);

    if (rc == 0) {
        /* Auto-open the newly created index */
        wa_set_index(path);
    } else {
        fprintf(stderr, "prg: failed to create index '%s'\n", path);
    }

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* LOCATE FOR <expr> [WHILE <expr>]                                    */
/*                                                                      */
/* Scans from record 1 to the end looking for the first record where    */
/* <for_expr> evaluates to .T.  If a WHILE clause is present, scanning  */
/* stops as soon as <while_expr> evaluates to .F.                       */
/* Sets FOUND() to .T. on success, .F. otherwise.                      */
/* ------------------------------------------------------------------ */

static ExecStatus exec_locate(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "LOCATE" */

    /* Expect "FOR" */
    if (!*cur || !((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_FOR)) {
        skip_to_eol(cur);
        wa_locate_clear();
        return EXEC_OK;
    }
    (*cur) = (*cur)->next; /* skip "FOR" */

    /* Save the FOR expression token range (start to just before WHILE or EOL/EOF) */
    Token *for_start = *cur;
    Token *for_end = NULL;

    /* Scan ahead to find WHERE the FOR expression ends */
    {
        Token *scan = *cur;
        int paren_depth = 0;
        while (scan) {
            if (scan->type == TOK_LPAREN) paren_depth++;
            else if (scan->type == TOK_RPAREN) { if (paren_depth > 0) paren_depth--; }
            else if (scan->type == TOK_EOF || scan->type == TOK_EOL) {
                for_end = scan;
                break;
            }
            else if (paren_depth == 0 &&
                     scan->type == TOK_KEYWORD && scan->keyword_id == KW_WHILE) {
                for_end = scan;
                break;
            }
            scan = scan->next;
        }
        if (!for_end) for_end = scan; /* EOF or nothing found */
    }

    /* Check for optional WHILE clause */
    Token *while_start = NULL;
    if (for_end && for_end->type == TOK_KEYWORD && for_end->keyword_id == KW_WHILE) {
        while_start = for_end->next;
    }

    /* Save locate state for CONTINUE (per work area) */
    wa_locate_save(for_start, for_end, while_start, wa_db());

    /* Advance cur past the entire LOCATE line */
    skip_to_eol(cur);

    /* --- Execute the locate scan --- */
    DATABASEDBF *db = wa_db();
    if (!db) {
        wa_set_found(0);
        return EXEC_OK;
    }

    int total = reccount(db);
    int found = 0;

    wa_goto_top();

    int last_valid_rec = 0;

    for (int rec = 1; rec <= total; rec++) {
        gotos(wa_db_ptr(), rec);

        /* Evaluate WHILE guard first (if present) — stop if false */
        if (while_start) {
            Token *wcur = while_start;
            ExprValue wval = parse_expr(&wcur);
            int wresult = val_to_logical(&wval);
            free_value(&wval);
            if (!wresult) {
                /* WHILE condition failed — restore to last valid record */
                if (last_valid_rec > 0)
                    gotos(wa_db_ptr(), last_valid_rec);
                break;
            }
            last_valid_rec = rec;
        }

        /* Evaluate FOR condition */
        Token *fcur = for_start;
        ExprValue fval = parse_expr(&fcur);
        int fresult = val_to_logical(&fval);
        free_value(&fval);

        if (fresult) {
            found = 1;
            break; /* Stay on this record */
        }
    }

    wa_set_found(found);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* CONTINUE — resume LOCATE scan from current record + 1               */
/*   Uses per-work-area locate state stored in workarea module         */
/* ------------------------------------------------------------------ */

static ExecStatus exec_continue(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "CONTINUE" */
    skip_to_eol(cur);

    if (!wa_locate_active()) {
        wa_set_found(0);
        return EXEC_OK;
    }

    DATABASEDBF *db = wa_db();
    if (!db) {
        wa_set_found(0);
        return EXEC_OK;
    }

    Token *for_start   = wa_locate_for_start();
    Token *while_start = wa_locate_while_start();

    int total = reccount(db);
    int found = 0;
    int start_rec = wa_recno() + 1;

    if (start_rec > total) {
        wa_set_found(0);
        return EXEC_OK;
    }

    int last_valid_rec = 0;

    for (int rec = start_rec; rec <= total; rec++) {
        gotos(wa_db_ptr(), rec);

        if (while_start) {
            Token *wcur = while_start;
            ExprValue wval = parse_expr(&wcur);
            int wresult = val_to_logical(&wval);
            free_value(&wval);
            if (!wresult) {
                if (last_valid_rec > 0)
                    gotos(wa_db_ptr(), last_valid_rec);
                break;
            }
            last_valid_rec = rec;
        }

        Token *fcur = for_start;
        ExprValue fval = parse_expr(&fcur);
        int fresult = val_to_logical(&fval);
        free_value(&fval);

        if (fresult) {
            found = 1;
            break;
        }
    }

    wa_set_found(found);
    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* CREATE <filename>                                                   */
/* ------------------------------------------------------------------ */

static ExecStatus exec_create(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "CREATE" */

    /* Expect a filename (identifier or string) */
    if (!*cur || is_eol_or_eof(*cur)) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    char filename[256] = "";
    if ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING) {
        strncpy(filename, (*cur)->value, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        *cur = (*cur)->next;
    }

    if (filename[0] == '\0') {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Strip .dbf extension if present */
    size_t len = strlen(filename);
    if (len >= 4 && strcasecmp(filename + len - 4, ".dbf") == 0) {
        filename[len - 4] = '\0';
    }

    skip_to_eol(cur);

    if (ui_is_active()) {
        ui_create(filename);
    } else {
        ui_print("CREATE requires interactive mode");
    }

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* Wildcard pattern matching (LIKE / EXCEPT)                           */
/* ------------------------------------------------------------------ */

/* Simple wildcard match: '*' matches any sequence, '?' matches any single char.
   Case-insensitive. Returns 1 if matches, 0 otherwise. */
static int wildcard_match(const char *pattern, const char *str)
{
    const char *p = pattern;
    const char *s = str;

    while (*s) {
        if (*p == '*') {
            /* Skip consecutive '*' */
            while (*p == '*') p++;
            if (*p == '\0') return 1; /* '*' at end matches everything */
            /* Try matching the rest of pattern at every position */
            for (const char *sp = s; *sp; sp++) {
                if (wildcard_match(p, sp)) return 1;
            }
            /* Also try matching from end of string */
            return wildcard_match(p, "");
        }
        if (*p == '?') {
            p++; s++;
            continue;
        }
        if (tolower((unsigned char)*p) != tolower((unsigned char)*s)) {
            return 0;
        }
        p++; s++;
    }

    /* Skip trailing '*' in pattern */
    while (*p == '*') p++;
    return (*p == '\0');
}

/* Check if a variable name matches the SAVE TO filter criteria.
   like_pattern: NULL means no LIKE filter (match all)
   except_pattern: NULL means no EXCEPT filter
   Returns 1 if the variable should be included. */
static int var_matches_filter(const char *name, const char *like_pattern, const char *except_pattern)
{
    /* LIKE filter: only include if name matches */
    if (like_pattern && !wildcard_match(like_pattern, name))
        return 0;

    /* EXCEPT filter: exclude if name matches */
    if (except_pattern && wildcard_match(except_pattern, name))
        return 0;

    return 1;
}

/* ------------------------------------------------------------------ */
/* SAVE TO                                                             */
/* ------------------------------------------------------------------ */
/*
 * Syntax:
 *   SAVE TO <file> [ALL [LIKE <pattern>] [EXCEPT <pattern>]] [<var1>, <var2>, ...]
 */

static ExecStatus exec_save(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "SAVE" */

    /* Expect "TO" */
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_TO) {
        *cur = (*cur)->next;
    }

    /* Get filename */
    if (!*cur || is_eol_or_eof(*cur)) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    char filename[512];
    if ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING) {
        strncpy(filename, (*cur)->value, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        *cur = (*cur)->next;
    } else {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Add .mem extension if not present */
    {
        size_t len = strlen(filename);
        if (len < 4 || strcasecmp(filename + len - 4, ".mem") != 0) {
            strncat(filename, ".mem", sizeof(filename) - strlen(filename) - 1);
        }
    }

    /* Force lowercase for writes */
    force_lowercase(filename, filename, sizeof(filename));

    /* Parse optional: ALL [LIKE <pattern>] [EXCEPT <pattern>] or variable list */
    int is_all = 0;
    char like_pattern[256] = "";
    char except_pattern[256] = "";
    char *var_names[256];
    int var_count = 0;

    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ALL) {
        is_all = 1;
        *cur = (*cur)->next;

        /* Optional LIKE <pattern> */
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_LIKE) {
            *cur = (*cur)->next;
            if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING)) {
                strncpy(like_pattern, (*cur)->value, sizeof(like_pattern) - 1);
                like_pattern[sizeof(like_pattern) - 1] = '\0';
                *cur = (*cur)->next;
            }
        }

        /* Optional EXCEPT <pattern> */
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_EXCEPT) {
            *cur = (*cur)->next;
            if (*cur && ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING)) {
                strncpy(except_pattern, (*cur)->value, sizeof(except_pattern) - 1);
                except_pattern[sizeof(except_pattern) - 1] = '\0';
                *cur = (*cur)->next;
            }
        }
    } else {
        /* Variable name list */
        while (*cur && !is_eol_or_eof(*cur)) {
            if ((*cur)->type == TOK_IDENT) {
                var_names[var_count++] = strdup((*cur)->value);
                *cur = (*cur)->next;
            } else {
                break;
            }
            /* Skip comma */
            if (*cur && (*cur)->type == TOK_COMMA) {
                *cur = (*cur)->next;
            }
        }
    }

    skip_to_eol(cur);

    /* Perform the save */
    if (is_all) {
        /* Filter variables by LIKE/EXCEPT patterns */
        int total = vars_count();
        char *filtered[256];
        int filtered_count = 0;

        for (int i = 0; i < total && filtered_count < 256; i++) {
            const char *name;
            ExprValue val;
            if (!vars_get_by_index(i, &name, &val))
                continue;
            if (var_matches_filter(name, like_pattern[0] ? like_pattern : NULL,
                                   except_pattern[0] ? except_pattern : NULL)) {
                filtered[filtered_count++] = strdup(name);
            }
            free_value(&val);
        }

        memfile_save(filename,
                     filtered_count > 0 ? (const char **)filtered : NULL,
                     filtered_count);

        for (int i = 0; i < filtered_count; i++)
            free(filtered[i]);
    } else if (var_count > 0) {
        memfile_save(filename, (const char **)var_names, var_count);
        for (int i = 0; i < var_count; i++)
            free(var_names[i]);
    } else {
        /* SAVE TO file with no vars and no ALL = save all variables */
        memfile_save(filename, NULL, 0);
    }

    return EXEC_OK;
}

/* ------------------------------------------------------------------ */
/* RESTORE FROM                                                        */
/* ------------------------------------------------------------------ */
/*
 * Syntax:
 *   RESTORE FROM <file> [ADDITIVE]
 */

static ExecStatus exec_restore(Token **cur)
{
    (*cur) = (*cur)->next; /* skip "RESTORE" */

    /* Expect "FROM" — but FROM might not be a keyword, so accept any ident "from" */
    if (*cur && (*cur)->type == TOK_IDENT &&
        port_strcasecmp((*cur)->value, "from") == 0) {
        *cur = (*cur)->next;
    }

    /* Get filename */
    if (!*cur || is_eol_or_eof(*cur)) {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    char filename[512];
    if ((*cur)->type == TOK_IDENT || (*cur)->type == TOK_STRING) {
        strncpy(filename, (*cur)->value, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
        *cur = (*cur)->next;
    } else {
        skip_to_eol(cur);
        return EXEC_OK;
    }

    /* Add .mem extension if not present */
    {
        size_t len = strlen(filename);
        if (len < 4 || strcasecmp(filename + len - 4, ".mem") != 0) {
            strncat(filename, ".mem", sizeof(filename) - strlen(filename) - 1);
        }
    }

    /* Resolve case-insensitively */
    {
        char resolved[512];
        resolve_file_path(filename, resolved, sizeof(resolved));
        strncpy(filename, resolved, sizeof(filename) - 1);
        filename[sizeof(filename) - 1] = '\0';
    }

    /* Check for ADDITIVE */
    int additive = 0;
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ADDITIVE) {
        additive = 1;
        *cur = (*cur)->next;
    }

    skip_to_eol(cur);

    /* Perform the restore */
    memfile_restore(filename, additive);

    return EXEC_OK;
}
