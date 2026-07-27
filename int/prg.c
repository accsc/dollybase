/*
 * prg.c — dBase III Plus .PRG interpreter
 *
 * Usage:
 *   prg [file.prg]     Run a .prg file
 *   prg                Read from stdin (REPL-like)
 *
 * Compile:
 *   gcc -w -o prg prg.c tokenizer.c parser.c executor.c \
 *       exprvalue.c variables.c -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tokenizer.h"
#include "parser.h"
#include "executor.h"
#include "variables.h"
#include "exprvalue.h"
#include "workarea.h"
#include "ui.h"

/* ------------------------------------------------------------------ */
/* Read an entire file into a malloc'd, null-terminated buffer.        */
/* Returns NULL on failure. Caller must free().                        */
/* ------------------------------------------------------------------ */

static char *read_file(const char *path)
{
    FILE *fp = fopen(path, "r");
    if (!fp) {
        fprintf(stderr, "prg: cannot open '%s': %m\n", path);
        return NULL;
    }

    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (sz < 0) {
        fclose(fp);
        return NULL;
    }

    char *buf = malloc((size_t)sz + 1);
    if (!buf) {
        fclose(fp);
        return NULL;
    }

    size_t n = fread(buf, 1, (size_t)sz, fp);
    buf[n] = '\0';
    fclose(fp);
    return buf;
}

/* ------------------------------------------------------------------ */
/* Read all of stdin into a malloc'd, null-terminated buffer.          */
/* Returns NULL on failure. Caller must free().                        */
/* ------------------------------------------------------------------ */

static char *read_stdin(void)
{
    size_t cap = 4096;
    char *buf = malloc(cap);
    if (!buf)
        return NULL;

    size_t len = 0;
    int ch;
    while ((ch = getchar()) != EOF) {
        if (len + 1 >= cap) {
            cap *= 2;
            char *tmp = realloc(buf, cap);
            if (!tmp) {
                free(buf);
                return NULL;
            }
            buf = tmp;
        }
        buf[len++] = (char)ch;
    }
    buf[len] = '\0';
    return buf;
}

/* ------------------------------------------------------------------ */

int main(int argc, char *argv[])
{
    if (argc > 2) {
        fprintf(stderr, "Usage: %s [file.prg]\n", argv[0]);
        return 1;
    }

    char *source = NULL;

    if (argc == 2) {
        source = read_file(argv[1]);
    } else {
        source = read_stdin();
    }

    if (!source || !*source) {
        free(source);
        if (argc == 2)
            return 1;  /* file was given but unreadable/empty */
        return 0;       /* empty stdin is fine */
    }

    vars_init();
    wa_init();

    /* Initialize ncurses — always needed for @...SAY/GET/READ/CLEAR */
    ui_init();

    Token *tokens = tokenize(source);
    free(source);

    if (!tokens) {
        ui_shutdown();
        vars_shutdown();
        return 1;
    }

    proc_scan(tokens);

    ExecStatus st = execute_tokens(tokens);
    free_tokens(tokens);

    /* Show end message and pause so the user can see the screen */
    touchwin(stdscr);
    mvaddstr(LINES - 1, 0, " *** END DOLLYBASE RUN ***");
    refresh();
    { int gc = getch(); (void)gc; }
    ui_shutdown();

    wa_shutdown();
    vars_shutdown();

    /* Map ExecStatus to exit codes */
    switch (st) {
        case EXEC_OK:      return 0;
        case EXEC_RETURN:  return 0;
        case EXEC_CANCEL:  return 2;
        default:           return 1;
    }
}
