/*
 * tokenizer.h — dollybase lexer
 *
 * Character-by-character scanner that produces a typed linked list of tokens.
 * No global state: tokenize(source) returns the head of the token list.
 */

#ifndef _TOKENIZER_H
#define _TOKENIZER_H

#include <stddef.h>

/* ------------------------------------------------------------------ */
/* Token types                                                         */
/* ------------------------------------------------------------------ */

typedef enum {
    TOK_INTEGER,          /* Literal integer: 42                    */
    TOK_REAL,             /* Literal float: 3.14                    */
    TOK_STRING,           /* Quoted string: "hello"                 */
    TOK_DATE,             /* Date literal: {^2006-07-26}            */
    TOK_LOGICAL,          /* .T., .F., .Y., .N.                     */
    TOK_IDENT,            /* Variable name: xMyVar                  */
    TOK_KEYWORD,          /* Reserved word: IF, ELSE, DO, USE...    */
    TOK_OP_ARITH,         /* + - * / % ^                            */
    TOK_OP_COMPARISON,    /* = == != <> >= <= > <                   */
    TOK_OP_LOGIC,         /* AND OR NOT (as operators)              */
    TOK_OP_CONTAINS,      /* $  substring containment               */
    TOK_LPAREN,           /* (                                       */
    TOK_RPAREN,           /* )                                       */
    TOK_COMMA,            /* ,                                       */
    TOK_SEMICOLON,        /* ;  line continuation                   */
    TOK_COLON,            /* :                                       */
    TOK_ARROW,            /* ->  relation field access              */
    TOK_ASSIGN,           /* := or = (assignment)                   */
    TOK_AT,               /* @ for @...SAY / @...GET                */
    TOK_EOF,              /* End of source                          */
    TOK_EOL,              /* End of logical line                    */
} TokenType;

/* ------------------------------------------------------------------ */
/* Keyword IDs — one per reserved word, organized by category          */
/* ------------------------------------------------------------------ */

typedef enum {
    KW_NONE = 0,

    /* Control flow */
    KW_AND,
    KW_CANCEL,
    KW_CASE,
    KW_DO,
    KW_ELSE,
    KW_ELSEIF,
    KW_ENDCASE,
    KW_ENDDO,
    KW_ENDIF,
    KW_EXIT,
    KW_FOR,
    KW_GOTO,
    KW_IF,
    KW_LABEL,
    KW_LOOP,
    KW_ENDTEXT,
    KW_OR,
    KW_OTHERWISE,
    KW_QUIT,
    KW_RETURN,
    KW_WHILE,
    KW_WITH,

    /* Database commands */
    KW_ACCEPT,
    KW_ALL,
    KW_ALIAS,
    KW_DBF,
    KW_NEXT,
    KW_RECORD,
    KW_REST,
    KW_APPEND,
    KW_BROWSE,
    KW_CLOSE,
    KW_CREATE,
    KW_CONTINUE,
    KW_DELETE,
    KW_DELETED,
    KW_DISPLAY,
    KW_GO,
    KW_GOTOP,
    KW_GOBOTTOM,
    KW_INDEX,
    KW_INSERT,
    KW_JOIN,
    KW_LIST,
    KW_LOCATE,
    KW_PACK,
    KW_WAIT,
    KW_RECALL,
    KW_REPLACE,
    KW_REPLICATE,
    KW_SEEK,
    KW_SELECT,
    KW_SET_ORDER,
    KW_SET_RELATION,
    KW_SET_INDEX,
    KW_SET_FILTER,
    KW_SET_DELETED,
    KW_SKIP,
    KW_UPDATE,
    KW_USE,
    KW_ZAP,

    /* Program structure */
    KW_ACTIVATE,
    KW_ARRAY,
    KW_CLEAR,
    KW_CLEAR_ALL,
    KW_CLEAR_MEMORY,
    KW_DEFAULT,
    KW_DIMENSION,
    KW_FOCUS,
    KW_FUNCTION,
    KW_GET,
    KW_PARAMETERS,
    KW_PICTURE,
    KW_PROCEDURE,
    KW_LOCAL,
    KW_PRIVATE,
    KW_PUBLIC,
    KW_READ,
    KW_RANGE,
    KW_SAY,
    KW_VALID,

    /* I/O / system */
    KW_AT_SAY_GET,        /* @...SAY / @...GET (tokenized as one)   */
    KW_COLOR,
    KW_CONSOLE,
    KW_DATE_CMD,          /* SET DATE                               */
    KW_CENTURY,
    KW_EXACT,
    KW_MULTILOCKS,
    KW_PRINTER,
    KW_SAFETY,
    KW_SCOREBOARD,
    KW_STATUS,
    KW_STEP,
    KW_TALK,
    KW_UNIQUE,

    /* Expression functions */
    KW_ABS,
    KW_ALLTRIM,
    KW_ASC,
    KW_AT_FUNC,           /* AT()                                   */
    KW_ATC,
    KW_BETWEEN,
    KW_BOF,
    KW_CHR,
    KW_CDOW,
    KW_CMONTH,
    KW_CTOD,
    KW_DATE,              /* DATE() function                        */
    KW_DAY,
    KW_DOW,
    KW_DOUBLE,
    KW_DMY,
    KW_DTOC,
    KW_EMPTY,
    KW_EOF,
    KW_EXP,
    KW_FIELD,
    KW_FIELDS,
    KW_FOUND,
    KW_IIF,
    KW_ISALPHA,
    KW_ISLOWER,
    KW_ISUPPER,
    KW_INKEY,
    KW_INT_FUNC,          /* INT()                                  */
    KW_LEFT_FUNC,         /* LEFT()                                 */
    KW_LEN,
    KW_LOG,
    KW_LTRIM,
    KW_LOWER,
    KW_MAX,
    KW_MIN,
    KW_MOD,
    KW_MONTH,
    KW_OS,
    KW_RECN,
    KW_RECNO,
    KW_RIGHT_FUNC,        /* RIGHT()                                */
    KW_ROUND,
    KW_RTRIM,
    KW_SIGN,
    KW_STR,
    KW_SQRT,
    KW_SPACE,
    KW_SUBSTR,
    KW_STUFF,
    KW_STORE,
    KW_SUM,
    KW_TYPE,
    KW_UPPER,
    KW_VERSION,
    KW_TEXT,
    KW_TIME,
    KW_TRIM,
    KW_TO,
    KW_VAL,
    KW_YEAR,

    /* SET command keyword */
    KW_SET,
} KeywordId;

/* ------------------------------------------------------------------ */
/* Token struct                                                        */
/* ------------------------------------------------------------------ */

typedef struct Token {
    TokenType type;
    KeywordId keyword_id;   /* Valid only when type == TOK_KEYWORD  */
    char value[256];        /* Token text (always null-terminated)  */
    int line;               /* Source line number (1-indexed)       */
    struct Token *next;     /* Linked list forward pointer          */
} Token;

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * tokenize — scan source and return head of token linked list.
 * Returns NULL on empty input or allocation failure.
 */
Token *tokenize(const char *source);

/**
 * free_tokens — release all tokens in the list.
 */
void free_tokens(Token *head);

/**
 * print_token_list — debug dump to stderr.
 */
void print_token_list(Token *head);

/**
 * token_type_name — human-readable name for a TokenType (for errors).
 */
const char *token_type_name(TokenType type);

#endif /* _TOKENIZER_H */
