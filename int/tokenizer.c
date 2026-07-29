/*
 * tokenizer.c — dBase III Plus lexer implementation
 *
 * Single-pass character scanner that produces a linked list of tokens.
 */

#include "tokenizer.h"

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>   /* strcasecmp */

/* ------------------------------------------------------------------ */
/* Keyword table — sorted alphabetically for binary search             */
/* ------------------------------------------------------------------ */

typedef struct {
    char name[16];
    KeywordId id;
} KeywordEntry;

static const KeywordEntry KEYWORDS[] = {
    {"ABS",          KW_ABS},
    {"ACTIVATE",     KW_ACTIVATE},
    {"ALIAS",        KW_ALIAS},
    {"ALL",          KW_ALL},
    {"ALLTRIM",      KW_ALLTRIM},
    {"AND",          KW_AND},
    {"APPEND",       KW_APPEND},
    {"ARRAY",        KW_ARRAY},
    {"ASC",          KW_ASC},
    {"AT",           KW_AT_FUNC},
    {"ATC",          KW_ATC},
    {"BETWEEN",      KW_BETWEEN},
    {"BOF",          KW_BOF},
    {"BROWSE",       KW_BROWSE},
    {"CANCEL",       KW_CANCEL},
    {"CASE",         KW_CASE},
    {"CENTURY",      KW_CENTURY},
    {"CHR",          KW_CHR},
    {"CLEAR",        KW_CLEAR},
    {"CLEAR ALL",    KW_CLEAR_ALL},
    {"CLEAR MEMORY", KW_CLEAR_MEMORY},
    {"CLOSE",        KW_CLOSE},
    {"COLOR",        KW_COLOR},
    {"CONSOLE",      KW_CONSOLE},
    {"CONTINUE",     KW_CONTINUE},
    {"CTOD",         KW_CTOD},
    {"DATE",         KW_DATE},
    {"DAY",          KW_DAY},
    {"DEFAULT",      KW_DEFAULT},
    {"DELETE",       KW_DELETE},
    {"DELETED",      KW_DELETED},
    {"DIMENSION",    KW_DIMENSION},
    {"DISPLAY",      KW_DISPLAY},
    {"DMY",          KW_DMY},
    {"DO",           KW_DO},
    {"DTOC",         KW_DTOC},
    {"ELSE",         KW_ELSE},
    {"ELSEIF",       KW_ELSEIF},
    {"EMPTY",        KW_EMPTY},
    {"ENDCASE",      KW_ENDCASE},
    {"ENDDO",        KW_ENDDO},
    {"ENDFOR",       KW_ENDFOR},
    {"ENDIF",        KW_ENDIF},
    {"ENDTEXT",      KW_ENDTEXT},
    {"EOF",          KW_EOF},
    {"EXACT",        KW_EXACT},
    {"EXIT",         KW_EXIT},
    {"FIELD",        KW_FIELD},
    {"FIELDS",       KW_FIELDS},
    {"FOCUS",        KW_FOCUS},
    {"FOR",          KW_FOR},
    {"FOUND",        KW_FOUND},
    {"FUNCTION",     KW_FUNCTION},
    {"GET",          KW_GET},
    {"GO",           KW_GO},
    {"GOBOTTOM",     KW_GOBOTTOM},
    {"GOTO",         KW_GOTO},
    {"GOTOP",        KW_GOTOP},
    {"IF",           KW_IF},
    {"IIF",          KW_IIF},
    {"INDEX",        KW_INDEX},
    {"INKEY",        KW_INKEY},
    {"INSERT",       KW_INSERT},
    {"INT",          KW_INT_FUNC},
    {"JOIN",         KW_JOIN},
    {"LABEL",        KW_LABEL},
    {"LEFT",         KW_LEFT_FUNC},
    {"LEN",          KW_LEN},
    {"LIST",         KW_LIST},
    {"LOCAL",        KW_LOCAL},
    {"LOCATE",       KW_LOCATE},
    {"LOOP",         KW_LOOP},
    {"LOWER",        KW_LOWER},
    {"LTRIM",        KW_LTRIM},
    {"MAX",          KW_MAX},
    {"MIN",          KW_MIN},
    {"MONTH",        KW_MONTH},
    {"MULTILOCKS",   KW_MULTILOCKS},
    {"NEXT",         KW_NEXT},
    {"NOT",          KW_NOT},
    {"OTHERWISE",    KW_OTHERWISE},
    {"OR",           KW_OR},
    {"PACK",         KW_PACK},
    {"PARAMETERS",   KW_PARAMETERS},
    {"PICTURE",      KW_PICTURE},
    {"PRINTER",      KW_PRINTER},
    {"PRIVATE",      KW_PRIVATE},
    {"PROCEDURE",    KW_PROCEDURE},
    {"PUBLIC",       KW_PUBLIC},
    {"QUIT",         KW_QUIT},
    {"RANGE",        KW_RANGE},
    {"READ",         KW_READ},
    {"RECALL",       KW_RECALL},
    {"RECN",         KW_RECN},
    {"RECNO",        KW_RECNO},
    {"RECORD",       KW_RECORD},
    {"REPLACE",      KW_REPLACE},
    {"REST",         KW_REST},
    {"RETURN",       KW_RETURN},
    {"RIGHT",        KW_RIGHT_FUNC},
    {"ROUND",        KW_ROUND},
    {"RTRIM",        KW_RTRIM},
    {"SAFETY",       KW_SAFETY},
    {"SAY",          KW_SAY},
    {"SCOREBOARD",   KW_SCOREBOARD},
    {"SEEK",         KW_SEEK},
    {"SELECT",       KW_SELECT},
    {"SET",          KW_SET},
    {"SIGN",         KW_SIGN},
    {"SKIP",         KW_SKIP},
    {"SPACE",        KW_SPACE},
    {"SQRT",         KW_SQRT},
    {"STATUS",       KW_STATUS},
    {"STEP",         KW_STEP},
    {"SUBSTR",       KW_SUBSTR},
    {"SUM",          KW_SUM},
    {"TALK",         KW_TALK},
    {"TEXT",         KW_TEXT},
    {"TIME",         KW_TIME},
    {"TRIM",         KW_TRIM},
    {"TYPE",         KW_TYPE},
    {"UNIQUE",       KW_UNIQUE},
    {"UPDATE",       KW_UPDATE},
    {"UPPER",        KW_UPPER},
    {"USE",          KW_USE},
    {"VAL",          KW_VAL},
    {"VALID",        KW_VALID},
    {"WAIT",         KW_WAIT},
    {"WHILE",        KW_WHILE},
    {"WITH",         KW_WITH},
    {"YEAR",         KW_YEAR},
    {"ZAP",          KW_ZAP},
};

#define NUM_KEYWORDS (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */

static Token *make_token(TokenType type, const char *value, int line)
{
    Token *tok = calloc(1, sizeof(Token));
    if (!tok)
        return NULL;
    tok->type = type;
    tok->keyword_id = KW_NONE;
    strncpy(tok->value, value, 254);
    tok->value[254] = '\0';
    tok->line = line;
    tok->next = NULL;
    return tok;
}

static void append_token(Token *tok, Token **head, Token **tail)
{
    if (!tok)
        return;
    if (*tail) {
        (*tail)->next = tok;
    } else {
        *head = tok;
    }
    *tail = tok;
}

/* ------------------------------------------------------------------ */
/* TEXT ... ENDTEXT — collect raw lines verbatim                       */
/* ------------------------------------------------------------------ */

static void scan_text_block(const char **pos, int *line, Token **head, Token **tail)
{
    /* pos points just past "TEXT".
       Collect lines verbatim until ENDTEXT is found.
       Each line becomes a TOK_STRING token.
       The final ENDTEXT is emitted as a keyword token. */
    char linebuf[1024];

    while (**pos) {
        /* Collect one line (up to \n), verbatim */
        size_t i = 0;
        while (**pos && **pos != '\n') {
            if (i < sizeof(linebuf) - 1)
                linebuf[i++] = **pos;
            (*pos)++;
        }
        linebuf[i] = '\0';

        if (**pos == '\n') {
            (*pos)++;
            (*line)++;
        }

        /* Strip leading/trailing whitespace for the ENDTEXT check */
        char *p = linebuf;
        while (*p == ' ' || *p == '\t') p++;
        /* Check if this line is ENDTEXT (case-insensitive) */
        if (strcasecmp(p, "ENDTEXT") == 0) {
            /* Emit ENDTEXT keyword token */
            Token *tok = make_token(TOK_KEYWORD, "ENDTEXT", *line - 1);
            tok->keyword_id = KW_ENDTEXT;
            append_token(tok, head, tail);
            return;
        }

        /* Emit the raw line as a TOK_STRING */
        Token *tok = make_token(TOK_STRING, linebuf, *line - 1);
        append_token(tok, head, tail);
    }
}

/* Binary search on KEYWORDS[]. Case-insensitive. Returns KeywordId or KW_NONE. */
static KeywordId lookup_keyword(const char *word)
{
    int lo = 0, hi = (int)NUM_KEYWORDS - 1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        int cmp = strcasecmp(word, KEYWORDS[mid].name);
        if (cmp == 0)
            return KEYWORDS[mid].id;
        else if (cmp < 0)
            hi = mid - 1;
        else
            lo = mid + 1;
    }
    return KW_NONE;
}

static void skip_whitespace(const char **pos, int *line)
{
    while (**pos && **pos != '\n') {
        if (isspace((unsigned char)**pos)) {
            (*pos)++;
        } else {
            break;
        }
    }
}

static void consume_to_eol(const char **pos, int *line)
{
    while (**pos && **pos != '\n') {
        if (**pos == '\r') {
            /* handle \r\n gracefully */
        }
        (*pos)++;
    }
    if (**pos == '\n') {
        (*pos)++;
        (*line)++;
    }
}

/* ------------------------------------------------------------------ */
/* Scanners                                                            */
/* ------------------------------------------------------------------ */

static void scan_string_literal(const char **pos, Token **out, int line)
{
    /* pos points at opening '"' */
    (*pos)++;  /* skip opening quote */

    char buf[256];
    size_t j = 0;

    while (**pos) {
        if (**pos == '\n') {
            /* Unterminated string across lines — stop at newline */
            break;
        }
        if (**pos == '"' && (*pos)[1] == '"') {
            /* "" escape -> single " */
            buf[j++] = '"';
            (*pos) += 2;
        } else if (**pos == '"') {
            /* Closing quote — stop */
            break;
        } else {
            buf[j++] = **pos;
            (*pos)++;
        }
        if (j >= 254)
            break;
    }
    buf[j] = '\0';

    if (**pos == '"')
        (*pos)++;  /* skip closing quote */

    *out = make_token(TOK_STRING, buf, line);
}

static void scan_date_literal(const char **pos, Token **out, int line)
{
    /* pos points at '{' */
    (*pos)++;  /* skip opening brace */

    if (**pos == '^')
        (*pos)++;  /* skip caret */

    const char *start = *pos;
    while (**pos && **pos != '}')
        (*pos)++;

    size_t len = (size_t)(*pos - start);
    if (len >= 254)
        len = 254;

    char buf[256];
    strncpy(buf, start, len);
    buf[len] = '\0';

    if (**pos == '}')
        (*pos)++;  /* skip closing brace */

    *out = make_token(TOK_DATE, buf, line);
}

static void scan_logical_literal(const char **pos, Token **out, int line)
{
    /* pos points at '.'; check for .T. .F. .Y. .N. */
    if (**pos != '.')
        return;
    (*pos)++;

    char c = toupper((unsigned char)**pos);
    if (c != 'T' && c != 'F' && c != 'Y' && c != 'N') {
        /* Not a logical literal — back up so caller can handle '.' normally */
        (*pos)--;  /* back up to '.' */
        *out = NULL;
        return;
    }
    (*pos)++;

    if (**pos != '.') {
        /* Not a logical literal (e.g. .NOT. has 'O' after 'N').
           Back up to BEFORE the '.' so the main loop skips '.' and
           then scans the identifier/keyword normally. */
        (*pos) -= 2;  /* back up past letter and past '.' */
        *out = NULL;
        return;
    }
    (*pos)++;  /* skip trailing '.' */

    char val[4];
    if (c == 'T' || c == 'Y') {
        strcpy(val, ".T.");
    } else {
        strcpy(val, ".F.");
    }

    *out = make_token(TOK_LOGICAL, val, line);
}

static void scan_number(const char **pos, Token **out, int line)
{
    const char *start = *pos;
    int has_dot = 0;

    while (isdigit((unsigned char)**pos))
        (*pos)++;

    if (**pos == '.') {
        /* Check if next char is a digit -> real number */
        if (isdigit((unsigned char)(*pos)[1])) {
            has_dot = 1;
            (*pos)++;  /* skip '.' */
            while (isdigit((unsigned char)**pos))
                (*pos)++;
        }
    }

    /* Scientific notation: E or e followed by optional +/- and digits */
    if (**pos == 'E' || **pos == 'e') {
        has_dot = 1;
        (*pos)++;
        if (**pos == '+' || **pos == '-')
            (*pos)++;
        while (isdigit((unsigned char)**pos))
            (*pos)++;
    }

    size_t len = (size_t)(*pos - start);
    if (len >= 254)
        len = 254;

    char buf[256];
    strncpy(buf, start, len);
    buf[len] = '\0';

    TokenType type = has_dot ? TOK_REAL : TOK_INTEGER;
    *out = make_token(type, buf, line);
}

static void scan_identifier_or_keyword(const char **pos, Token **out, int line)
{
    const char *start = *pos;

    while (isalnum((unsigned char)**pos) || **pos == '_')
        (*pos)++;

    size_t len = (size_t)(*pos - start);
    if (len >= 254)
        len = 254;

    char buf[256];
    strncpy(buf, start, len);
    buf[len] = '\0';

    KeywordId kid = lookup_keyword(buf);
    if (kid != KW_NONE) {
        *out = make_token(TOK_KEYWORD, buf, line);
        (*out)->keyword_id = kid;
    } else {
        *out = make_token(TOK_IDENT, buf, line);
    }
}

static void scan_operator(const char **pos, Token **out, int line)
{
    char c1 = **pos;
    char c2 = (*pos)[1];

    /* Two-character operators */
    if (c1 == ':' && c2 == '=') {
        *out = make_token(TOK_ASSIGN, ":=", line);
        (*pos) += 2;
        return;
    }
    if (c1 == '-' && c2 == '>') {
        *out = make_token(TOK_ARROW, "->", line);
        (*pos) += 2;
        return;
    }
    if (c1 == '<' && c2 == '>') {
        *out = make_token(TOK_OP_COMPARISON, "<>", line);
        (*pos) += 2;
        return;
    }
    if (c1 == '<' && c2 == '=') {
        *out = make_token(TOK_OP_COMPARISON, "<=", line);
        (*pos) += 2;
        return;
    }
    if (c1 == '>' && c2 == '=') {
        *out = make_token(TOK_OP_COMPARISON, ">=", line);
        (*pos) += 2;
        return;
    }
    if (c1 == '!' && c2 == '=') {
        *out = make_token(TOK_OP_COMPARISON, "!=", line);
        (*pos) += 2;
        return;
    }
    if (c1 == '=' && c2 == '=') {
        *out = make_token(TOK_OP_COMPARISON, "==", line);
        (*pos) += 2;
        return;
    }

    /* Single-character operators */
    char val[2] = { c1, '\0' };
    TokenType type;

    switch (c1) {
    case '+':
    case '*':
    case '/':
    case '%':
    case '^':
    case '?':
        type = TOK_OP_ARITH;
        break;
    case '-':
        /* Distinguish minus from arrow (already handled above) */
        type = TOK_OP_ARITH;
        break;
    case '=':
    case '<':
    case '>':
    case '!':
        type = TOK_OP_COMPARISON;
        break;
    case '$':
        type = TOK_OP_CONTAINS;
        break;
    default:
        /* Unknown operator char — skip silently */
        (*pos)++;
        *out = NULL;
        return;
    }

    (*pos)++;
    *out = make_token(type, val, line);
}

/* ------------------------------------------------------------------ */
/* Main tokenize function                                              */
/* ------------------------------------------------------------------ */

Token *tokenize(const char *source)
{
    if (!source || !*source)
        return NULL;

    Token *head = NULL;
    Token *tail = NULL;
    const char *pos = source;
    int line = 1;
    int at_line_start = 1;  /* Track whether we are at the start of a line */

    while (*pos) {
        skip_whitespace(&pos, &line);
        if (!*pos)
            break;

        Token *tok = NULL;
        char c = *pos;

        if (at_line_start && c == '*') {
            /* Line comment: consume to end of line */
            consume_to_eol(&pos, &line);
            at_line_start = 0;
            continue;
        }

        switch (c) {
        case '"':
            scan_string_literal(&pos, &tok, line);
            break;

        case '{':
            scan_date_literal(&pos, &tok, line);
            break;

        case '.':
            /* Could be a logical literal (.T. .F. etc.) */
            scan_logical_literal(&pos, &tok, line);
            if (!tok) {
                /* Not a logical — treat '.' as part of identifier or skip */
                /* In dBase, standalone '.' is not meaningful; skip it */
                pos++;
            }
            break;

        case '0': case '1': case '2': case '3': case '4':
        case '5': case '6': case '7': case '8': case '9':
            scan_number(&pos, &tok, line);
            break;

        default:
            if (isalpha((unsigned char)c) || c == '_') {
                scan_identifier_or_keyword(&pos, &tok, line);
                /* If we just scanned TEXT keyword, collect the block verbatim */
                if (tok && tok->type == TOK_KEYWORD && tok->keyword_id == KW_TEXT) {
                    append_token(tok, &head, &tail);
                    tok = NULL;  /* don't append again below */
                    scan_text_block(&pos, &line, &head, &tail);
                    at_line_start = 1;
                    continue;
                }
            } else if (c == '(') {
                tok = make_token(TOK_LPAREN, "(", line);
                pos++;
            } else if (c == ')') {
                tok = make_token(TOK_RPAREN, ")", line);
                pos++;
            } else if (c == ',') {
                tok = make_token(TOK_COMMA, ",", line);
                pos++;
            } else if (c == ';') {
                /* Line continuation: consume to EOL, suppress that EOL */
                tok = make_token(TOK_SEMICOLON, ";", line);
                pos++;
                consume_to_eol(&pos, &line);
            } else if (c == ':') {
                /* Could be := or standalone ':' */
                if (*(pos + 1) == '=') {
                    tok = make_token(TOK_ASSIGN, ":=", line);
                    pos += 2;
                } else {
                    tok = make_token(TOK_COLON, ":", line);
                    pos++;
                }
            } else if (c == '?' || c == '!' ||
                       c == '+' || c == '-' || c == '*' ||
                       c == '/' || c == '%' || c == '^' ||
                       c == '=' || c == '<' || c == '>') {
                scan_operator(&pos, &tok, line);
            } else if (c == '\n') {
                tok = make_token(TOK_EOL, "\n", line);
                pos++;
                line++;
                at_line_start = 1;
            } else if (c == '@') {
                tok = make_token(TOK_AT, "@", line);
                pos++;
            } else if (c == '&') {
                /* Check for && inline comment */
                if (*(pos + 1) == '&') {
                    consume_to_eol(&pos, &line);
                    continue;
                } else {
                    /* & — macro expansion / concatenation operator */
                    tok = make_token(TOK_OP_LOGIC, "&", line);
                    pos++;
                }
            } else {
                /* Unknown character — skip silently */
                pos++;
                at_line_start = 0;
                continue;
            }
            break;
        }

        if (tok) {
            append_token(tok, &head, &tail);
            at_line_start = 0;
        }
    }

    /* Append EOF token */
    Token *eof_tok = make_token(TOK_EOF, "", line);
    if (eof_tok)
        append_token(eof_tok, &head, &tail);

    return head;
}

/* ------------------------------------------------------------------ */
/* Utility functions                                                   */
/* ------------------------------------------------------------------ */

void free_tokens(Token *head)
{
    while (head) {
        Token *next = head->next;
        free(head);
        head = next;
    }
}

void print_token_list(Token *head)
{
    for (Token *t = head; t; t = t->next) {
        fprintf(stderr, "  [%3d] %-16s \"%s\"",
                t->line, token_type_name(t->type), t->value);
        if (t->type == TOK_KEYWORD && t->keyword_id != KW_NONE) {
            fprintf(stderr, " (KW_%d)", t->keyword_id);
        }
        fprintf(stderr, "\n");
    }
}

const char *token_type_name(TokenType type)
{
    switch (type) {
    case TOK_INTEGER:       return "TOK_INTEGER";
    case TOK_REAL:          return "TOK_REAL";
    case TOK_STRING:        return "TOK_STRING";
    case TOK_DATE:          return "TOK_DATE";
    case TOK_LOGICAL:       return "TOK_LOGICAL";
    case TOK_IDENT:         return "TOK_IDENT";
    case TOK_KEYWORD:       return "TOK_KEYWORD";
    case TOK_OP_ARITH:      return "TOK_OP_ARITH";
    case TOK_OP_COMPARISON: return "TOK_OP_COMPARISON";
    case TOK_OP_LOGIC:      return "TOK_OP_LOGIC";
    case TOK_OP_CONTAINS:   return "TOK_OP_CONTAINS";
    case TOK_LPAREN:        return "TOK_LPAREN";
    case TOK_RPAREN:        return "TOK_RPAREN";
    case TOK_COMMA:         return "TOK_COMMA";
    case TOK_SEMICOLON:     return "TOK_SEMICOLON";
    case TOK_COLON:         return "TOK_COLON";
    case TOK_ARROW:         return "TOK_ARROW";
    case TOK_ASSIGN:        return "TOK_ASSIGN";
    case TOK_AT:            return "TOK_AT";
    case TOK_EOF:           return "TOK_EOF";
    case TOK_EOL:           return "TOK_EOL";
    default:                return "TOK_UNKNOWN";
    }
}
