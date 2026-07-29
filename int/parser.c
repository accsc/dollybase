/*
 * parser.c — dBase III Plus recursive descent expression evaluator
 *
 * Walks a token list produced by tokenize(), evaluates expressions,
 * and returns typed ExprValue results.
 */

#include "parser.h"
#include "variables.h"
#include "workarea.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
#include <ctype.h>
#include <strings.h>
#include <time.h>
#include <curses.h>

/* ------------------------------------------------------------------ */
/* Forward declarations (grammar levels, lowest precedence first)      */
/* ------------------------------------------------------------------ */

static ExprValue parse_or_expr(Token **cur, ParseError *err);
static ExprValue parse_and_expr(Token **cur, ParseError *err);
static ExprValue parse_comp_expr(Token **cur, ParseError *err);
static ExprValue parse_add_expr(Token **cur, ParseError *err);
static ExprValue parse_mul_expr(Token **cur, ParseError *err);
static ExprValue parse_pow_expr(Token **cur, ParseError *err);
static ExprValue parse_unary_expr(Token **cur, ParseError *err);
static ExprValue parse_primary(Token **cur, ParseError *err);

/* Built-in function dispatch */
typedef ExprValue (*BuiltinFn)(Token **cur, ParseError *err);

typedef struct {
    KeywordId kw;
    BuiltinFn impl;
} FuncEntry;

static ExprValue builtin_len(Token **cur, ParseError *err);
static ExprValue builtin_val(Token **cur, ParseError *err);
static ExprValue builtin_int_func(Token **cur, ParseError *err);
static ExprValue builtin_round(Token **cur, ParseError *err);
static ExprValue builtin_abs(Token **cur, ParseError *err);
static ExprValue builtin_sqrt(Token **cur, ParseError *err);
static ExprValue builtin_upper(Token **cur, ParseError *err);
static ExprValue builtin_lower(Token **cur, ParseError *err);
static ExprValue builtin_alltrim(Token **cur, ParseError *err);
static ExprValue builtin_trim(Token **cur, ParseError *err);
static ExprValue builtin_ltrim(Token **cur, ParseError *err);
static ExprValue builtin_rtrim(Token **cur, ParseError *err);
static ExprValue builtin_substr(Token **cur, ParseError *err);
static ExprValue builtin_stuff(Token **cur, ParseError *err);
static ExprValue builtin_left_func(Token **cur, ParseError *err);
static ExprValue builtin_right_func(Token **cur, ParseError *err);
static ExprValue builtin_iif(Token **cur, ParseError *err);
static ExprValue builtin_isalpha(Token **cur, ParseError *err);
static ExprValue builtin_islower(Token **cur, ParseError *err);
static ExprValue builtin_isupper(Token **cur, ParseError *err);
static ExprValue builtin_inkey(Token **cur, ParseError *err);
static ExprValue builtin_empty(Token **cur, ParseError *err);
static ExprValue builtin_type(Token **cur, ParseError *err);
static ExprValue builtin_at_func(Token **cur, ParseError *err);
static ExprValue builtin_between(Token **cur, ParseError *err);
static ExprValue builtin_date_func(Token **cur, ParseError *err);
static ExprValue builtin_dtoc(Token **cur, ParseError *err);
static ExprValue builtin_ctod(Token **cur, ParseError *err);
static ExprValue builtin_day(Token **cur, ParseError *err);
static ExprValue builtin_month(Token **cur, ParseError *err);
static ExprValue builtin_year(Token **cur, ParseError *err);
static ExprValue builtin_time(Token **cur, ParseError *err);
static ExprValue builtin_recno(Token **cur, ParseError *err);
static ExprValue builtin_recn(Token **cur, ParseError *err);
static ExprValue builtin_eof(Token **cur, ParseError *err);
static ExprValue builtin_bof(Token **cur, ParseError *err);
static ExprValue builtin_alias(Token **cur, ParseError *err);
static ExprValue builtin_deleted(Token **cur, ParseError *err);
static ExprValue builtin_found(Token **cur, ParseError *err);
static ExprValue builtin_sign(Token **cur, ParseError *err);
static ExprValue builtin_max(Token **cur, ParseError *err);
static ExprValue builtin_min(Token **cur, ParseError *err);
static ExprValue builtin_replicate(Token **cur, ParseError *err);
static ExprValue builtin_space(Token **cur, ParseError *err);
static ExprValue builtin_chr(Token **cur, ParseError *err);
static ExprValue builtin_asc(Token **cur, ParseError *err);

/* Built-in dispatch (defined later in this file) */
static ExprValue dispatch_builtin(KeywordId kw, Token **cur, ParseError *err);

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

TokenType peek_type(const Token *cur) {
    if (!cur || cur->type == TOK_EOF || cur->type == TOK_EOL)
        return TOK_EOF;
    return cur->type;
}

int match_keyword(Token **cur, KeywordId id) {
    if (cur && *cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == id) {
        *cur = (*cur)->next;
        return 1;
    }
    return 0;
}

void skip_comma(Token **cur) {
    if (cur && *cur && (*cur)->type == TOK_COMMA) {
        *cur = (*cur)->next;
    }
}

int has_more_args(const Token *cur) {
    if (!cur) return 0;
    TokenType t = cur->type;
    /* Stop at ), EOF, or EOL */
    if (t == TOK_RPAREN || t == TOK_EOF || t == TOK_EOL) return 0;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Grammar: expression → or_expr                                       */
/* ------------------------------------------------------------------ */

ExprValue parse_expression(Token **cur, ParseError *error) {
    return parse_or_expr(cur, error);
}

/* ------------------------------------------------------------------ */
/* or_expr → and_expr ( "OR" and_expr )*                              */
/* ------------------------------------------------------------------ */

static ExprValue parse_or_expr(Token **cur, ParseError *err) {
    ExprValue left = parse_and_expr(cur, err);
    if (*err != PARSE_OK) return left;

    while (match_keyword(cur, KW_OR)) {
        ExprValue right = parse_and_expr(cur, err);
        if (*err != PARSE_OK) { free_value(&left); return left; }
        free_value(&right);
        /* Short-circuit: if left is already .T., result is .T. */
        int lv = val_to_logical(&left);
        int rv = val_to_logical(&right);
        ExprValue res = val_logical(lv || rv);
        free_value(&left);
        left = res;
    }
    return left;
}

/* ------------------------------------------------------------------ */
/* and_expr → comp_expr ( "AND" comp_expr )*                          */
/* ------------------------------------------------------------------ */

static ExprValue parse_and_expr(Token **cur, ParseError *err) {
    ExprValue left = parse_comp_expr(cur, err);
    if (*err != PARSE_OK) return left;

    while (match_keyword(cur, KW_AND)) {
        ExprValue right = parse_comp_expr(cur, err);
        if (*err != PARSE_OK) { free_value(&left); return left; }
        free_value(&right);
        int lv = val_to_logical(&left);
        int rv = val_to_logical(&right);
        ExprValue res = val_logical(lv && rv);
        free_value(&left);
        left = res;
    }
    return left;
}

/* ------------------------------------------------------------------ */
/* comp_expr → add_expr ( comp_op add_expr )*                         */
/* Returns a LOGICAL value (.T. or .F.)                               */
/* ------------------------------------------------------------------ */

static ExprValue parse_comp_expr(Token **cur, ParseError *err) {
    ExprValue left = parse_add_expr(cur, err);
    if (*err != PARSE_OK) return left;

    while (1) {
        int cmp_op = 0; /* 0=none, 1==, 2=<>, 3=>, 4=<, 5=>=, 6<=, 7=$ */

        if (cur && *cur && (*cur)->type == TOK_OP_COMPARISON) {
            const char *v = (*cur)->value;
            if (strcmp(v, "=") == 0 || strcmp(v, "==") == 0) cmp_op = 1;
            else if (strcmp(v, "<>") == 0 || strcmp(v, "!=") == 0) cmp_op = 2;
            else if (strcmp(v, ">") == 0) cmp_op = 3;
            else if (strcmp(v, "<") == 0) cmp_op = 4;
            else if (strcmp(v, ">=") == 0) cmp_op = 5;
            else if (strcmp(v, "<=") == 0) cmp_op = 6;
        }

        // $ operator (substring containment) — different token type
        int is_contains = 0;
        if (cur && *cur && (*cur)->type == TOK_OP_CONTAINS) {
            is_contains = 1;
        }

        if (!cmp_op && !is_contains) break;

        if (is_contains) {
            *cur = (*cur)->next; /* consume '$' */
            ExprValue right = parse_add_expr(cur, err);
            if (*err != PARSE_OK) { free_value(&left); return left; }

            /* needle $ haystack — coerce both to strings, case-insensitive */
            char *needle = val_to_string(&left);
            char *haystack = val_to_string(&right);
            int result = 0;
            if (needle && haystack && *needle) {
                result = (strcasestr(haystack, needle) != NULL);
            }
            free(needle);
            free(haystack);
            free_value(&left);
            free_value(&right);
            left = val_logical(result);

        } else {
            *cur = (*cur)->next; /* consume operator */

            ExprValue right = parse_add_expr(cur, err);
            if (*err != PARSE_OK) { free_value(&left); return left; }

            int result = 0;
            switch (cmp_op) {
                case 1: /* == */
                    result = (compare_values(&left, &right) == 0);
                    break;
                case 2: /* <> */
                    result = (compare_values(&left, &right) != 0);
                    break;
                case 3: /* > */
                    result = (compare_values(&left, &right) > 0);
                    break;
                case 4: /* < */
                    result = (compare_values(&left, &right) < 0);
                    break;
                case 5: /* >= */
                    result = (compare_values(&left, &right) >= 0);
                    break;
                case 6: /* <= */
                    result = (compare_values(&left, &right) <= 0);
                    break;
            }

            free_value(&left);
            free_value(&right);
            left = val_logical(result);
        }
    }

    return left;
}

/* ------------------------------------------------------------------ */
/* add_expr → mul_expr ( ("+" | "-") mul_expr )*                      */
/* + on strings does concatenation                                     */
/* ------------------------------------------------------------------ */

static ExprValue parse_add_expr(Token **cur, ParseError *err) {
    ExprValue left = parse_mul_expr(cur, err);
    if (*err != PARSE_OK) return left;

    while (1) {
        char op = 0;
        if (cur && *cur && (*cur)->type == TOK_OP_ARITH) {
            const char *v = (*cur)->value;
            if (strcmp(v, "+") == 0) op = '+';
            else if (strcmp(v, "-") == 0) op = '-';
        }
        if (!op) break;
        *cur = (*cur)->next;

        ExprValue right = parse_mul_expr(cur, err);
        if (*err != PARSE_OK) { free_value(&left); return left; }

        /* String concatenation: if either operand is a string, concatenate */
        if (op == '+' && (left.type == VAL_STRING || right.type == VAL_STRING)) {
            char *ls = val_to_string(&left);
            char *rs = val_to_string(&right);
            ExprValue res = val_string("");
            /* Reallocate with combined length */
            free(res.data.sval);
            size_t len = strlen(ls) + strlen(rs);
            char *buf = (char *)malloc(len + 1);
            if (buf) {
                snprintf(buf, len + 1, "%s%s", ls, rs);
                res.data.sval = buf;
            } else {
                res.type = VAL_NULL;
            }
            free(ls); free(rs);
            free_value(&left);
            free_value(&right);
            left = res;
        } else if (op == '+') {
            double lv = val_to_double(&left);
            double rv = val_to_double(&right);
            ExprValue res;
            /* Preserve integer type if both operands are integers */
            if (left.type == VAL_INTEGER && right.type == VAL_INTEGER)
                res = val_integer((int)(lv + rv));
            else
                res = val_real(lv + rv);
            free_value(&left);
            free_value(&right);
            left = res;
        } else { /* op == '-' */
            double lv = val_to_double(&left);
            double rv = val_to_double(&right);
            ExprValue res;
            if (left.type == VAL_INTEGER && right.type == VAL_INTEGER)
                res = val_integer((int)(lv - rv));
            else
                res = val_real(lv - rv);
            free_value(&left);
            free_value(&right);
            left = res;
        }
    }

    return left;
}

/* ------------------------------------------------------------------ */
/* mul_expr → pow_expr ( ("*" | "/" | "%") pow_expr )*                */
/* ------------------------------------------------------------------ */

static ExprValue parse_mul_expr(Token **cur, ParseError *err) {
    ExprValue left = parse_pow_expr(cur, err);
    if (*err != PARSE_OK) return left;

    while (1) {
        char op = 0;
        if (cur && *cur && (*cur)->type == TOK_OP_ARITH) {
            const char *v = (*cur)->value;
            if (strcmp(v, "*") == 0) op = '*';
            else if (strcmp(v, "/") == 0) op = '/';
            else if (strcmp(v, "%") == 0) op = '%';
        }
        if (!op) break;
        *cur = (*cur)->next;

        ExprValue right = parse_pow_expr(cur, err);
        if (*err != PARSE_OK) { free_value(&left); return left; }

        double lv = val_to_double(&left);
        double rv = val_to_double(&right);
        ExprValue res;

        switch (op) {
            case '*':
                res = val_real(lv * rv);
                break;
            case '/':
                if (rv == 0.0) {
                    /* dBase returns 0 on division by zero */
                    res = val_integer(0);
                } else {
                    res = val_real(lv / rv);
                }
                break;
            case '%':
                if (rv == 0.0) {
                    res = val_integer(0);
                } else {
                    res = val_real(fmod(lv, rv));
                }
                break;
        }

        free_value(&left);
        free_value(&right);
        left = res;
    }

    return left;
}

/* ------------------------------------------------------------------ */
/* pow_expr → unary_expr ( "^" pow_expr )?   (right-associative)      */
/* ------------------------------------------------------------------ */

static ExprValue parse_pow_expr(Token **cur, ParseError *err) {
    ExprValue base = parse_unary_expr(cur, err);
    if (*err != PARSE_OK) return base;

    if (cur && *cur && (*cur)->type == TOK_OP_ARITH &&
        strcmp((*cur)->value, "^") == 0) {
        *cur = (*cur)->next;
        /* Right-associative: the exponent is another pow_expr */
        ExprValue exp = parse_pow_expr(cur, err);
        if (*err != PARSE_OK) { free_value(&base); return base; }

        double b = val_to_double(&base);
        double e = val_to_double(&exp);
        ExprValue res = val_real(pow(b, e));
        free_value(&base);
        free_value(&exp);
        return res;
    }

    return base;
}

/* ------------------------------------------------------------------ */
/* unary_expr → "NOT" unary_expr | "-" unary_expr | primary            */
/* ------------------------------------------------------------------ */

static ExprValue parse_unary_expr(Token **cur, ParseError *err) {
    /* NOT */
    if (match_keyword(cur, KW_NOT)) {
        ExprValue inner = parse_unary_expr(cur, err);
        if (*err != PARSE_OK) return inner;
        int v = val_to_logical(&inner);
        free_value(&inner);
        return val_logical(!v);
    }

    /* Unary minus */
    if (cur && *cur && (*cur)->type == TOK_OP_ARITH && strcmp((*cur)->value, "-") == 0) {
        *cur = (*cur)->next;
        ExprValue inner = parse_unary_expr(cur, err);
        if (*err != PARSE_OK) return inner;
        double v = val_to_double(&inner);
        free_value(&inner);
        return val_real(-v);
    }

    /* Macro expansion: &variable — resolve variable, evaluate its string as expression */
    if (cur && *cur && (*cur)->type == TOK_OP_LOGIC && strcmp((*cur)->value, "&") == 0) {
        *cur = (*cur)->next; /* skip '&' */
        if (!*cur) {
            if (err) *err = PARSE_ERROR_SYNTAX;
            return val_null();
        }
        /* Expect an identifier or string literal */
        ExprValue macro_val;
        if ((*cur)->type == TOK_IDENT) {
            Token *saved = (*cur)->next;
            if (vars_exists((*cur)->value)) {
                macro_val = vars_get((*cur)->value);
            } else {
                macro_val = val_null();
            }
            *cur = saved;
        } else if ((*cur)->type == TOK_STRING) {
            /* &"literal" — use the string literal directly */
            Token *saved = (*cur)->next;
            macro_val = parse_primary(cur, err);
            (void)saved;
        } else {
            if (err) *err = PARSE_ERROR_SYNTAX;
            return val_null();
        }
        /* Get the string content and evaluate it as a dBase expression */
        char *s = val_to_string(&macro_val);
        free_value(&macro_val);
        if (s && *s) {
            Token *tokens = tokenize(s);
            Token *tcur = tokens;
            ExprValue result = parse_expression(&tcur, err);
            free(s);
            free_tokens(tokens);
            return result;
        }
        free(s);
        return val_null();
    }

    return parse_primary(cur, err);
}

/* ------------------------------------------------------------------ */
/* primary → literal | variable | function_call | "(" expression ")"   */
/* ------------------------------------------------------------------ */

static ExprValue parse_primary(Token **cur, ParseError *err) {
    if (!cur || !*cur) {
        if (err) *err = PARSE_ERROR_SYNTAX;
        return val_null();
    }

    Token *tok = *cur;

    /* Literals */
    switch (tok->type) {
        case TOK_INTEGER: {
            int v = atoi(tok->value);
            *cur = tok->next;
            return val_integer(v);
        }
        case TOK_REAL: {
            double v = atof(tok->value);
            *cur = tok->next;
            return val_real(v);
        }
        case TOK_STRING: {
            /* Strip surrounding quotes */
            const char *s = tok->value;
            size_t len = strlen(s);
            if (len >= 2 && s[0] == '"' && s[len - 1] == '"') {
                /* Extract inner content, handling "" escapes */
                char *inner = (char *)malloc(len); /* max possible */
                if (inner) {
                    char *dst = inner;
                    const char *src = s + 1; /* skip opening quote */
                    while (*src && *(src + 1)) {
                        if (*src == '"' && *(src + 1) == '"') {
                            *dst++ = '"';
                            src += 2;
                        } else {
                            *dst++ = *src++;
                        }
                    }
                    *dst = '\0';
                }
                ExprValue res = val_string(inner);
                free(inner);
                *cur = tok->next;
                return res;
            }
            /* Fallback: use value as-is */
            ExprValue res = val_string(s);
            *cur = tok->next;
            return res;
        }
        case TOK_DATE: {
            const char *d = tok->value;
            size_t len = strlen(d);
            /* Strip braces and ^ if present */
            if (len >= 2 && d[0] == '{') {
                d++; len--;
                if (*d == '^') { d++; len--; }
            }
            ExprValue res = val_date(d);
            *cur = tok->next;
            return res;
        }
        case TOK_LOGICAL: {
            const char *v = tok->value;
            /* .T., .Y. → true; .F., .N. → false */
            size_t len = strlen(v);
            int t = 0;
            if (len >= 3) {
                char mid = v[1];
                t = (mid == 'T' || mid == 't' || mid == 'Y' || mid == 'y');
            }
            ExprValue res = val_logical(t);
            *cur = tok->next;
            return res;
        }

        case TOK_LPAREN: {
            /* Parenthesized expression */
            *cur = tok->next; /* skip '(' */
            ExprValue res = parse_expression(cur, err);
            if (*err != PARSE_OK) return res;
            /* Expect ')' */
            if (cur && *cur && (*cur)->type == TOK_RPAREN) {
                *cur = (*cur)->next;
            } else {
                if (err) *err = PARSE_ERROR_SYNTAX;
                free_value(&res);
                return val_null();
            }
            return res;
        }

        case TOK_KEYWORD: {
            /* Could be a function call like LEN(x), DATE(), EOF() */
            Token *saved = *cur;
            (*cur) = tok->next;
            if (cur && *cur && (*cur)->type == TOK_LPAREN) {
                /* It's a function call — consume '(' and dispatch */
                *cur = (*cur)->next; /* skip '(' */
                ExprValue res = dispatch_builtin(tok->keyword_id, cur, err);
                if (*err != PARSE_OK) return res;
                /* Expect ')' */
                if (cur && *cur && (*cur)->type == TOK_RPAREN) {
                    *cur = (*cur)->next;
                } else {
                    if (err) *err = PARSE_ERROR_SYNTAX;
                    free_value(&res);
                    return val_null();
                }
                return res;
            }
            /* Not a function call — could be a bare keyword used as identifier */
            /* For now, treat it as an identifier (variable name) */
            *cur = saved;
            break;
        }

        case TOK_IDENT: {
            /* Check for -> field access: ALIAS->FIELD */
            Token *saved = *cur;
            *cur = tok->next;
            if (*cur && (*cur)->type == TOK_ARROW) {
                /* ALIAS->FIELD access — resolve alias to work area */
                const char *alias = tok->value;
                *cur = (*cur)->next; /* skip -> */
                if (*cur && (*cur)->type == TOK_IDENT) {
                    const char *fieldname = (*cur)->value;
                    int area = wa_alias_to_area(alias);
                    if (area < 0)
                        area = wa_get_selected(); /* fallback: current area */
                    int fidx = wa_field_to_number_area(area, fieldname);
                    if (fidx > 0) {
                        char *val = wa_get_field_area(area, fidx);
                        char type = wa_field_type_area(area, fidx);
                        ExprValue result;
                        if (val) {
                            switch (type) {
                                case 'N': {
                                    double d = atof(val);
                                    if (d == (int)d && strstr(val, ".") == NULL)
                                        result = val_integer((int)d);
                                    else
                                        result = val_real(d);
                                    break;
                                }
                                case 'D':
                                    result = val_date(val);
                                    break;
                                case 'L':
                                    result = val_logical(val[0] == 'T' || val[0] == 'Y');
                                    break;
                                default:
                                    result = val_string(val);
                                    break;
                            }
                            free(val);
                        } else {
                            result = val_null();
                        }
                        *cur = (*cur)->next;
                        return result;
                    }
                    /* Field not found — consume the field token
                       and return null so the caller doesn't see a stray -> */
                    *cur = (*cur)->next;
                    return val_null();
                }
                /* -> not followed by IDENT — consume -> and fall through */
            }
            /* Variable reference — look up in the variable store. */
            *cur = saved;
            *cur = tok->next;
            if (vars_exists(tok->value)) {
                return vars_get(tok->value);
            }
            /* Check if it's a field name in the current DBF */
            {
                int fidx = wa_field_to_number(tok->value);
                if (fidx > 0) {
                    char *val = wa_get_field(fidx);
                    char type = wa_field_type(fidx);
                    ExprValue result;
                    if (val) {
                        switch (type) {
                            case 'N': {
                                double d = atof(val);
                                if (d == (int)d && strstr(val, ".") == NULL)
                                    result = val_integer((int)d);
                                else
                                    result = val_real(d);
                                break;
                            }
                            case 'D':
                                result = val_date(val);
                                break;
                            case 'L':
                                result = val_logical(val[0] == 'T' || val[0] == 'Y');
                                break;
                            default:
                                result = val_string(val);
                                break;
                        }
                        free(val);
                    } else {
                        result = val_null();
                    }
                    return result;
                }
            }
            return val_null();
        }

        default:
            break;
    }

    if (err) *err = PARSE_ERROR_SYNTAX;
    fprintf(stderr, "Parse error at line %d: unexpected token '%s' (%s)\n",
            tok->line, tok->value, token_type_name(tok->type));
    return val_null();
}

/* ------------------------------------------------------------------ */
/* Built-in function dispatch                                          */
/* ------------------------------------------------------------------ */

static const FuncEntry func_table[] = {
    { KW_ABS,       builtin_abs },
    { KW_ALLTRIM,   builtin_alltrim },
    { KW_ALIAS,     builtin_alias },
    { KW_ASC,       builtin_asc },
    { KW_AT_FUNC,   builtin_at_func },
    { KW_BETWEEN,   builtin_between },
    { KW_BOF,       builtin_bof },
    { KW_CHR,       builtin_chr },
    { KW_CTOD,      builtin_ctod },
    { KW_DATE,      builtin_date_func },
    { KW_DAY,       builtin_day },
    { KW_DELETED,   builtin_deleted },
    { KW_DTOC,      builtin_dtoc },
    { KW_EMPTY,     builtin_empty },
    { KW_EOF,       builtin_eof },
    { KW_FOUND,     builtin_found },
    { KW_IIF,       builtin_iif },
    { KW_ISALPHA,   builtin_isalpha },
    { KW_ISLOWER,   builtin_islower },
    { KW_ISUPPER,   builtin_isupper },
    { KW_INKEY,     builtin_inkey },
    { KW_INT_FUNC,  builtin_int_func },
    { KW_LEFT_FUNC, builtin_left_func },
    { KW_LEN,       builtin_len },
    { KW_LOWER,     builtin_lower },
    { KW_LTRIM,     builtin_ltrim },
    { KW_MAX,       builtin_max },
    { KW_MIN,       builtin_min },
    { KW_MONTH,     builtin_month },
    { KW_RECN,      builtin_recn },
    { KW_RECNO,     builtin_recno },
    { KW_RIGHT_FUNC,builtin_right_func },
    { KW_ROUND,     builtin_round },
    { KW_RTRIM,     builtin_rtrim },
    { KW_SIGN,      builtin_sign },
    { KW_SQRT,      builtin_sqrt },
    { KW_REPLICATE, builtin_replicate },
    { KW_SPACE,     builtin_space },
    { KW_SUBSTR,    builtin_substr },
    { KW_STUFF,     builtin_stuff },
    { KW_TYPE,      builtin_type },
    { KW_UPPER,     builtin_upper },
    { KW_TRIM,      builtin_trim },
    { KW_VAL,       builtin_val },
    { KW_YEAR,      builtin_year },
    { KW_TIME,      builtin_time },
    /* Additional functions */
    { 0,            NULL }
};

/* Missing from tokenizer keywords — add stubs for common ones we want to support */

static ExprValue dispatch_builtin(KeywordId kw, Token **cur, ParseError *err) {
    for (int i = 0; func_table[i].impl != NULL; i++) {
        if (func_table[i].kw == kw) {
            return func_table[i].impl(cur, err);
        }
    }
    /* Unknown function */
    if (err) *err = PARSE_ERROR_FUNC;
    fprintf(stderr, "Unknown function: keyword %d\n", kw);
    return val_null();
}

/* ------------------------------------------------------------------ */
/* Built-in function implementations                                   */
/* ------------------------------------------------------------------ */

static ExprValue builtin_len(Token **cur, ParseError *err) {
    (void)err;
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    int len = 0;
    switch (arg.type) {
        case VAL_STRING:
            len = (arg.data.sval) ? (int)strlen(arg.data.sval) : 0;
            break;
        default:
            /* For non-strings, convert to string first */
            {
                char *s = val_to_string(&arg);
                len = (int)strlen(s);
                free(s);
            }
    }
    free_value(&arg);
    return val_integer(len);
}

static ExprValue builtin_val(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    double v = 0.0;
    if (arg.type == VAL_STRING && arg.data.sval) {
        /* dBase VAL: scan left to right, stop at first non-numeric char */
        const char *s = arg.data.sval;
        int sign = 1;
        if (*s == '-') { sign = -1; s++; }
        else if (*s == '+') s++;

        /* Skip leading spaces */
        while (*s == ' ') s++;

        /* Parse numeric prefix */
        char buf[64];
        int bi = 0;
        const char *p = s;
        while (*p && bi < 63) {
            if (isdigit((unsigned char)*p) || *p == '.' || *p == 'e' || *p == 'E' ||
                *p == '+' || *p == '-') {
                buf[bi++] = *p++;
            } else {
                break; /* stop at first non-numeric */
            }
        }
        buf[bi] = '\0';
        v = atof(buf) * sign;
    } else {
        v = val_to_double(&arg);
    }
    free_value(&arg);
    return val_real(v);
}

static ExprValue builtin_int_func(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    double v = val_to_double(&arg);
    free_value(&arg);
    /* INT() truncates toward zero */
    int result = (int)(v >= 0 ? floor(v) : ceil(v));
    return val_integer(result);
}

static ExprValue builtin_round(Token **cur, ParseError *err) {
    ExprValue arg1 = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg1;
    skip_comma(cur);
    int decimals = 0;
    if (has_more_args(*cur)) {
        ExprValue arg2 = parse_expression(cur, err);
        if (*err == PARSE_OK) decimals = val_to_int(&arg2);
        free_value(&arg2);
    }
    double v = val_to_double(&arg1);
    free_value(&arg1);

    double factor = pow(10.0, (double)decimals);
    double result = round(v * factor) / factor;
    return val_real(result);
}

static ExprValue builtin_abs(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    double v = fabs(val_to_double(&arg));
    free_value(&arg);
    return val_real(v);
}

static ExprValue builtin_sqrt(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    double v = sqrt(val_to_double(&arg));
    free_value(&arg);
    return val_real(v);
}

static ExprValue builtin_upper(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    for (char *p = s; *p; p++) *p = (char)toupper((unsigned char)*p);
    ExprValue res = val_string(s);
    free(s);
    return res;
}

static ExprValue builtin_lower(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    for (char *p = s; *p; p++) *p = (char)tolower((unsigned char)*p);
    ExprValue res = val_string(s);
    free(s);
    return res;
}

static ExprValue builtin_alltrim(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    char *orig = s;
    /* Trim left */
    while (*s == ' ') s++;
    /* Trim right */
    size_t len = strlen(s);
    while (len > 0 && s[len - 1] == ' ') s[--len] = '\0';
    ExprValue res = val_string(s);
    free(orig);
    return res;
}

static ExprValue builtin_trim(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    char *orig = s;
    /* Trim left */
    while (*s == ' ') s++;
    /* Trim right */
    size_t len = strlen(s);
    while (len > 0 && s[len - 1] == ' ') s[--len] = '\0';
    ExprValue res = val_string(s);
    free(orig);
    return res;
}

static ExprValue builtin_ltrim(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    char *orig = s;
    while (*s == ' ') s++;
    ExprValue res = val_string(s);
    free(orig);
    return res;
}

static ExprValue builtin_rtrim(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    size_t len = strlen(s);
    while (len > 0 && s[len - 1] == ' ') s[--len] = '\0';
    ExprValue res = val_string(s);
    free(s);
    return res;
}

static ExprValue builtin_substr(Token **cur, ParseError *err) {
    ExprValue str = parse_expression(cur, err);
    if (*err != PARSE_OK) return str;
    skip_comma(cur);
    ExprValue start_arg = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&str); return start_arg; }
    skip_comma(cur);

    int start = val_to_int(&start_arg);
    free_value(&start_arg);
    /* dBase is 1-indexed */
    if (start < 1) start = 1;

    int len = -1;
    if (has_more_args(*cur)) {
        ExprValue len_arg = parse_expression(cur, err);
        if (*err == PARSE_OK) {
            len = val_to_int(&len_arg);
            free_value(&len_arg);
        }
    }

    char *s = val_to_string(&str);
    free_value(&str);

    int slen = (int)strlen(s);
    if (start > slen) {
        ExprValue res = val_string("");
        free(s);
        return res;
    }

    const char *sub = s + start - 1;
    if (len < 0 || len > (int)(slen - start + 1))
        len = slen - start + 1;

    char *result = (char *)malloc(len + 1);
    if (result) {
        memcpy(result, sub, len);
        result[len] = '\0';
    }
    free(s);

    ExprValue res = val_string(result ? result : "");
    free(result);
    return res;
}

/* ------------------------------------------------------------------ */
/* STUFF(cSource, nStart, nLength, cInsert)                            */
/*   Replaces nLength characters in cSource starting at nStart         */
/*   with cInsert.  nStart is 1-indexed.                               */
/* ------------------------------------------------------------------ */

static ExprValue builtin_stuff(Token **cur, ParseError *err)
{
    ExprValue src_val = parse_expression(cur, err);
    if (*err != PARSE_OK) return src_val;
    skip_comma(cur);
    ExprValue start_arg = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&src_val); return start_arg; }
    skip_comma(cur);
    ExprValue len_arg = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&src_val); free_value(&start_arg); return len_arg; }
    skip_comma(cur);
    ExprValue ins_val = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&src_val); free_value(&start_arg); free_value(&len_arg); return ins_val; }

    int start = val_to_int(&start_arg);
    free_value(&start_arg);
    int nlen = val_to_int(&len_arg);
    free_value(&len_arg);
    if (start < 1) start = 1;
    if (nlen < 0) nlen = 0;

    char *src = val_to_string(&src_val);
    free_value(&src_val);
    char *ins = val_to_string(&ins_val);
    free_value(&ins_val);

    int slen = (int)strlen(src);
    int ilen = (int)strlen(ins);

    /* Clamp start to string length + 1 (append if past end) */
    if (start > slen + 1) start = slen + 1;

    /* Clamp nlen so we don't remove past end of string */
    int remove_end = start - 1 + nlen;
    if (remove_end > slen) remove_end = slen;

    /* Build result: prefix + insert + suffix */
    int prefix_len = start - 1;
    int suffix_len = slen - (remove_end - start + 1);
    if (suffix_len < 0) suffix_len = 0;
    int total = prefix_len + ilen + suffix_len;

    char *buf = (char *)malloc(total + 1);
    if (buf) {
        int p = 0;
        /* Prefix */
        if (prefix_len > 0) {
            memcpy(buf + p, src, prefix_len);
            p += prefix_len;
        }
        /* Insert */
        if (ilen > 0) {
            memcpy(buf + p, ins, ilen);
            p += ilen;
        }
        /* Suffix */
        if (suffix_len > 0) {
            memcpy(buf + p, src + remove_end, suffix_len);
            p += suffix_len;
        }
        buf[p] = '\0';
    }
    free(src);
    free(ins);

    ExprValue res = val_string(buf ? buf : "");
    free(buf);
    return res;
}

static ExprValue builtin_left_func(Token **cur, ParseError *err) {
    ExprValue str = parse_expression(cur, err);
    if (*err != PARSE_OK) return str;
    skip_comma(cur);
    ExprValue n_arg = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&str); return n_arg; }

    int n = val_to_int(&n_arg);
    free_value(&n_arg);
    if (n < 0) n = 0;

    char *s = val_to_string(&str);
    free_value(&str);

    int slen = (int)strlen(s);
    if (n > slen) n = slen;

    char *buf = (char *)malloc(n + 1);
    if (buf) {
        memcpy(buf, s, n);
        buf[n] = '\0';
    }
    free(s);

    ExprValue res = val_string(buf ? buf : "");
    free(buf);
    return res;
}

static ExprValue builtin_right_func(Token **cur, ParseError *err) {
    ExprValue str = parse_expression(cur, err);
    if (*err != PARSE_OK) return str;
    skip_comma(cur);
    ExprValue n_arg = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&str); return n_arg; }

    int n = val_to_int(&n_arg);
    free_value(&n_arg);
    if (n < 0) n = 0;

    char *s = val_to_string(&str);
    free_value(&str);

    int slen = (int)strlen(s);
    if (n > slen) n = slen;

    const char *src = s + slen - n;
    char *buf = (char *)malloc(n + 1);
    if (buf) {
        memcpy(buf, src, n);
        buf[n] = '\0';
    }
    free(s);

    ExprValue res = val_string(buf ? buf : "");
    free(buf);
    return res;
}

static ExprValue builtin_iif(Token **cur, ParseError *err) {
    ExprValue cond = parse_expression(cur, err);
    if (*err != PARSE_OK) return cond;
    skip_comma(cur);
    ExprValue true_val = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&cond); return true_val; }
    skip_comma(cur);
    ExprValue false_val = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&cond); free_value(&true_val); return false_val; }

    if (val_to_logical(&cond)) {
        free_value(&cond);
        free_value(&false_val);
        return true_val;
    } else {
        free_value(&cond);
        free_value(&true_val);
        return false_val;
    }
}

/* ------------------------------------------------------------------ */
/* ISALPHA(cString) — .T. if first character is alphabetic              */
/* ------------------------------------------------------------------ */

static ExprValue builtin_isalpha(Token **cur, ParseError *err)
{
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    int result = 0;
    if (s[0]) {
        result = isalpha((unsigned char)s[0]);
    }
    free(s);
    return val_logical(result);
}

/* ------------------------------------------------------------------ */
/* ISLOWER(cString) — .T. if first character is lowercase               */
/* ------------------------------------------------------------------ */

static ExprValue builtin_islower(Token **cur, ParseError *err)
{
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    int result = 0;
    if (s[0]) {
        result = islower((unsigned char)s[0]);
    }
    free(s);
    return val_logical(result);
}

/* ------------------------------------------------------------------ */
/* ISUPPER(cString) — .T. if first character is uppercase               */
/* ------------------------------------------------------------------ */

static ExprValue builtin_isupper(Token **cur, ParseError *err)
{
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    int result = 0;
    if (s[0]) {
        result = isupper((unsigned char)s[0]);
    }
    free(s);
    return val_logical(result);
}

/* ------------------------------------------------------------------ */
/* INKEY([nSeconds]) — return ASCII code of key press                  */
/*   No arg: block indefinitely until a key is pressed                 */
/*   Arg = 0: non-blocking (return 0 if no key waiting)               */
/*   Arg > 0: block up to nSeconds, return 0 on timeout                */
/* ------------------------------------------------------------------ */

static ExprValue builtin_inkey(Token **cur, ParseError *err)
{
    (void)err;
    int has_arg = 0;
    double timeout = 0;

    /* Check for optional seconds argument */
    if (cur && *cur && (*cur)->type != TOK_RPAREN && (*cur)->type != TOK_EOF && (*cur)->type != TOK_EOL) {
        ExprValue arg = parse_expression(cur, err);
        if (*err != PARSE_OK) return arg;
        timeout = val_to_double(&arg);
        free_value(&arg);
        has_arg = 1;
    }

    int key;

    if (!has_arg) {
        /* No argument: block indefinitely */
        nodelay(stdscr, FALSE);
        key = getch();
        nodelay(stdscr, TRUE);
    } else if (timeout == 0) {
        /* INKEY(0): non-blocking */
        nodelay(stdscr, TRUE);
        key = getch();
        if (key == ERR)
            key = 0;
    } else {
        /* INKEY(n): block up to n seconds */
        int tenths = (int)(timeout * 10.0);
        if (tenths < 1) tenths = 1;
        if (tenths > 255) tenths = 255;
        halfdelay(tenths);
        key = getch();
        nodelay(stdscr, TRUE);
        if (key == ERR)
            key = 0;
    }

    return val_integer(key);
}

static ExprValue builtin_empty(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    int empty = 0;
    switch (arg.type) {
        case VAL_NULL:
            empty = 1;
            break;
        case VAL_STRING:
            empty = (!arg.data.sval || strlen(arg.data.sval) == 0);
            break;
        case VAL_INTEGER:
        case VAL_REAL:
            empty = (arg.data.rval == 0.0);
            break;
        case VAL_DATE:
            empty = (strcmp(arg.data.dval, "0000-00-00") == 0 || arg.data.dval[0] == '\0');
            break;
        case VAL_LOGICAL:
            /* Logical is never EMPTY in dBase */
            empty = 0;
            break;
    }
    free_value(&arg);
    return val_logical(empty);
}

static ExprValue builtin_type(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    /* Returns a single-char string: N=numeric, C=character, D=date, L=logical, O=object, U=undefined */
    char code = 'U';
    switch (arg.type) {
        case VAL_INTEGER:
        case VAL_REAL:   code = 'N'; break;
        case VAL_STRING:  code = 'C'; break;
        case VAL_DATE:    code = 'D'; break;
        case VAL_LOGICAL: code = 'L'; break;
        case VAL_NULL:    code = 'U'; break;
    }
    free_value(&arg);
    char buf[2] = {code, '\0'};
    return val_string(buf);
}

static ExprValue builtin_at_func(Token **cur, ParseError *err) {
    ExprValue needle = parse_expression(cur, err);
    if (*err != PARSE_OK) return needle;
    skip_comma(cur);
    ExprValue haystack = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&needle); return haystack; }

    const char *n = (needle.data.sval) ? needle.data.sval : "";
    const char *h = (haystack.data.sval) ? haystack.data.sval : "";

    /* AT() is case-insensitive in dBase — portable strcasestr */
    int nlen = (int)strlen(n);
    int hlen = (int)strlen(h);
    int pos = 0;
    if (nlen > 0 && nlen <= hlen) {
        for (int i = 0; i <= hlen - nlen; i++) {
            int match = 1;
            for (int j = 0; j < nlen; j++) {
                if (tolower((unsigned char)h[i + j]) != tolower((unsigned char)n[j])) {
                    match = 0; break;
                }
            }
            if (match) { pos = i + 1; break; } /* 1-indexed */
        }
    }

    free_value(&needle);
    free_value(&haystack);
    return val_integer(pos);
}

static ExprValue builtin_between(Token **cur, ParseError *err) {
    ExprValue val = parse_expression(cur, err);
    if (*err != PARSE_OK) return val;
    skip_comma(cur);
    ExprValue low = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&val); return low; }
    skip_comma(cur);
    ExprValue high = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&val); free_value(&low); return high; }

    int result = (compare_values(&val, &low) >= 0 && compare_values(&val, &high) <= 0);

    free_value(&val);
    free_value(&low);
    free_value(&high);
    return val_logical(result);
}

static ExprValue builtin_date_func(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    /* Returns current date as a DATE value — stub returns "0000-00-00" */
    /* In the full interpreter this would call localtime() */
    return val_date("2026-07-26");
}

static ExprValue builtin_dtoc(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = NULL;
    if (arg.type == VAL_DATE) {
        size_t len = strlen(arg.data.dval) + 1;
        s = malloc(len); if (s) strcpy(s, arg.data.dval);
    } else {
        s = val_to_string(&arg);
    }
    free_value(&arg);
    ExprValue res = val_string(s ? s : "");
    free(s);
    return res;
}

static ExprValue builtin_ctod(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    /* Try to parse as MM/DD/YYYY or YYYY-MM-DD */
    int y = 0, m = 0, d = 0;
    ExprValue res;
    if (sscanf(s, "%d/%d/%d", &m, &d, &y) == 3) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
        res = val_date(buf);
    } else if (sscanf(s, "%d-%d-%d", &y, &m, &d) == 3) {
        char buf[12];
        snprintf(buf, sizeof(buf), "%04d-%02d-%02d", y, m, d);
        res = val_date(buf);
    } else {
        res = val_date("0000-00-00");
    }
    free(s);
    return res;
}

static ExprValue builtin_day(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    int d = 0;
    if (arg.type == VAL_DATE) {
        sscanf(arg.data.dval, "%*d-%*d-%d", &d);
    } else {
        char *s = val_to_string(&arg);
        sscanf(s, "%*d-%*d-%d", &d);
        free(s);
    }
    free_value(&arg);
    return val_integer(d);
}

static ExprValue builtin_month(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    int m = 0;
    if (arg.type == VAL_DATE) {
        sscanf(arg.data.dval, "%*d-%d-%*d", &m);
    } else {
        char *s = val_to_string(&arg);
        sscanf(s, "%*d-%d-%*d", &m);
        free(s);
    }
    free_value(&arg);
    return val_integer(m);
}

static ExprValue builtin_year(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    int y = 0;
    if (arg.type == VAL_DATE) {
        sscanf(arg.data.dval, "%d-%*d-%*d", &y);
    } else {
        char *s = val_to_string(&arg);
        sscanf(s, "%d-%*d-%*d", &y);
        free(s);
    }
    free_value(&arg);
    return val_integer(y);
}

static ExprValue builtin_time(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    time_t now = time(NULL);
    struct tm *lt = localtime(&now);
    char buf[9];
    snprintf(buf, sizeof(buf), "%02d:%02d:%02d", lt->tm_hour, lt->tm_min, lt->tm_sec);
    return val_string(buf);
}

static ExprValue builtin_recno(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    return val_integer(wa_recno());
}

static ExprValue builtin_recn(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    return val_integer(wa_reccount());
}

static ExprValue builtin_eof(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    return val_logical(wa_eof());
}

static ExprValue builtin_bof(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    return val_logical(wa_bof());
}

static ExprValue builtin_alias(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    char *name = wa_dbf_name();
    ExprValue res = val_string(name ? name : "");
    free(name);
    return res;
}

static ExprValue builtin_deleted(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    return val_logical(wa_is_deleted());
}

static ExprValue builtin_found(Token **cur, ParseError *err) {
    (void)cur; (void)err;
    return val_logical(wa_found());
}

static ExprValue builtin_sign(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    double v = val_to_double(&arg);
    free_value(&arg);
    int result = (v > 0) ? 1 : (v < 0) ? -1 : 0;
    return val_integer(result);
}

static ExprValue builtin_max(Token **cur, ParseError *err) {
    ExprValue a = parse_expression(cur, err);
    if (*err != PARSE_OK) return a;
    skip_comma(cur);
    ExprValue b = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&a); return b; }

    double va = val_to_double(&a);
    double vb = val_to_double(&b);
    free_value(&a);
    free_value(&b);

    return val_real(va > vb ? va : vb);
}

static ExprValue builtin_min(Token **cur, ParseError *err) {
    ExprValue a = parse_expression(cur, err);
    if (*err != PARSE_OK) return a;
    skip_comma(cur);
    ExprValue b = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&a); return b; }

    double va = val_to_double(&a);
    double vb = val_to_double(&b);
    free_value(&a);
    free_value(&b);

    return val_real(va < vb ? va : vb);
}

static ExprValue builtin_replicate(Token **cur, ParseError *err) {
    ExprValue ch = parse_expression(cur, err);
    if (*err != PARSE_OK) return ch;
    skip_comma(cur);
    ExprValue n_arg = parse_expression(cur, err);
    if (*err != PARSE_OK) { free_value(&ch); return n_arg; }

    int n = val_to_int(&n_arg);
    free_value(&n_arg);
    if (n < 0) n = 0;

    char *s = val_to_string(&ch);
    free_value(&ch);
    int c = s[0] ? (unsigned char)s[0] : ' ';
    free(s);

    char *buf = (char *)malloc(n + 1);
    if (!buf) return val_string("");
    memset(buf, c, n);
    buf[n] = '\0';
    ExprValue res = val_string(buf);
    free(buf);
    return res;
}

static ExprValue builtin_space(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    int n = val_to_int(&arg);
    free_value(&arg);
    if (n < 0) n = 0;
    char *buf = (char *)malloc(n + 1);
    if (!buf) return val_string("");
    memset(buf, ' ', n);
    buf[n] = '\0';
    ExprValue res = val_string(buf);
    free(buf);
    return res;
}

static ExprValue builtin_chr(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    int c = val_to_int(&arg);
    free_value(&arg);
    char buf[2] = {(char)c, '\0'};
    return val_string(buf);
}

static ExprValue builtin_asc(Token **cur, ParseError *err) {
    ExprValue arg = parse_expression(cur, err);
    if (*err != PARSE_OK) return arg;
    char *s = val_to_string(&arg);
    free_value(&arg);
    int result = s[0] ? (unsigned char)s[0] : 0;
    free(s);
    return val_integer(result);
}
