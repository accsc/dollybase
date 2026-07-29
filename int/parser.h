/*
 * parser.h — dollybase recursive descent expression evaluator
 *
 * Operates on a token list produced by the tokenizer.
 * Takes a Token** cursor; advances it past consumed tokens.
 */

#ifndef _PARSER_H
#define _PARSER_H

#include "tokenizer.h"
#include "exprvalue.h"

/* ------------------------------------------------------------------ */
/* Parser error codes                                                  */
/* ------------------------------------------------------------------ */

typedef enum {
    PARSE_OK,           /* Expression parsed successfully             */
    PARSE_ERROR_SYNTAX, /* Unexpected token                           */
    PARSE_ERROR_UNARY,  /* Missing operand for unary operator         */
    PARSE_ERROR_FUNC,   /* Unknown function or wrong arg count        */
} ParseError;

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * parse_expression — evaluate a full dBase expression starting at *cur.
 *
 * Grammar (precedence low → high):
 *   or_expr       → and_expr ( "OR"  and_expr )*
 *   and_expr      → comp_expr ( "AND" comp_expr )*
 *   comp_expr     → add_expr ( ("=" | "<>" | ">" | "<" | ">=" | "<=") add_expr )*
 *   add_expr      → mul_expr ( ("+" | "-") mul_expr )*
 *   mul_expr      → pow_expr ( ("*" | "/" | "%") pow_expr )*
 *   pow_expr      → unary_expr ( "^" pow_expr )?        (right-assoc)
 *   unary_expr    → "NOT" unary_expr | "-" unary_expr | primary
 *   primary       → literal / variable / function_call / "(" expr ")"
 *
 * Advances *cur past the last consumed token.
 * Returns an ExprValue; on error returns val_null() and sets *error.
 */
ExprValue parse_expression(Token **cur, ParseError *error);

/**
 * peek_type — return the TokenType of the current token without advancing.
 * Returns TOK_EOF if cur is NULL or points to EOF/EOL.
 */
TokenType peek_type(const Token *cur);

/**
 * match_keyword — if *cur is a keyword with the given id, consume it and return 1.
 */
int match_keyword(Token **cur, KeywordId id);

/**
 * skip_comma — if *cur is TOK_COMMA, advance past it.
 */
void skip_comma(Token **cur);

/**
 * has_more_args — check whether we're at a closing paren or end of input.
 * Returns 0 if more arguments are available (not ) and not EOF/EOL).
 */
int has_more_args(const Token *cur);

#endif /* _PARSER_H */
