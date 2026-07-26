/*
 * executor.h — dBase III Plus statement executor
 *
 * Walks a token list (produced by tokenize()) and dispatches statements.
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
/* Public API                                                          */
/* ------------------------------------------------------------------ */

/**
 * execute_tokens — run the full token list as a dBase program.
 * Returns an ExecStatus code.
 */
ExecStatus execute_tokens(Token *tokens);

#endif /* _EXECUTOR_H */
