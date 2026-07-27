/*
 * executor.h — dBase III Plus statement executor
 *
 * Walks a token list (produced by tokenize()) and dispatches statements.
 * Supports PROCEDURE definitions, DO <name> calls, and RETURN.
 */

#ifndef _EXECUTOR_H
#define _EXECUTOR_H

#include "tokenizer.h"
#include "exprvalue.h"

/* ------------------------------------------------------------------ */
/* Execution status codes                                              */
/* ------------------------------------------------------------------ */

typedef enum {
    EXEC_OK,          /* Continue normally                              */
    EXEC_RETURN,      /* Return from procedure/program                  */
    EXEC_EXIT,        /* Exit loop (DO WHILE / FOR)                     */
    EXEC_LOOP,        /* Restart loop iteration                         */
    EXEC_CANCEL,      /* Abort program entirely                         */
} ExecStatus;

/* ------------------------------------------------------------------ */
/* Procedure registry                                                  */
/* ------------------------------------------------------------------ */

#define MAX_PROCEDURES 64

typedef struct {
    char name[256];
    Token *start;     /* First token of the procedure body              */
} Procedure;

/**
 * proc_registry_init — clear the procedure registry.
 */
void proc_registry_init(void);

/**
 * proc_registry_add — register a procedure by name and token start.
 */
void proc_registry_add(const char *name, Token *start);

/**
 * proc_registry_lookup — find a procedure by name (case-insensitive).
 * Returns the Procedure pointer, or NULL if not found.
 */
const Procedure *proc_registry_lookup(const char *name);

/**
 * proc_scan — scan a token list for PROCEDURE definitions and register them.
 * Must be called before execute_tokens.
 */
void proc_scan(Token *tokens);

/* ------------------------------------------------------------------ */
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * execute_tokens — run the full token list as a dBase program.
 * Returns an ExecStatus code.
 */
ExecStatus execute_tokens(Token *tokens);

/**
 * execute_file — load, tokenize, scan, and execute a .prg file.
 * Used internally by DO <external_file>.
 */
ExecStatus execute_file(const char *path);

#endif /* _EXECUTOR_H */
