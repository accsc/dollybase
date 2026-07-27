/*
 * ui.c — ncurses UI layer for dBASE III Plus interpreter
 *
 * Implements @...SAY, @...GET, READ, CLEAR using ncurses + libform.
 */

#include "ui.h"
#include "variables.h"
#include "exprvalue.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ------------------------------------------------------------------ */
/* Globals                                                             */
/* ------------------------------------------------------------------ */

static int ui_active = 0;
static UiGetField *get_queue = NULL;
static UiGetField *get_last  = NULL;

/* SAY text buffer — stored so we can redraw on form window */
typedef struct SayEntry {
    int row;
    int col;
    char text[256];
    struct SayEntry *next;
} SayEntry;

static SayEntry *say_list = NULL;

/* ------------------------------------------------------------------ */
/* Lifecycle                                                           */
/* ------------------------------------------------------------------ */

int ui_init(void)
{
    initscr();
    cbreak();            /* no wait for Enter                          */
    noecho();            /* don't echo input automatically             */
    keypad(stdscr, TRUE);/* enable special keys                        */
    curs_set(1);         /* visible cursor                             */
    ui_active = 1;
    return 0;
}

void ui_shutdown(void)
{
    if (!ui_active) return;
    curs_set(1);
    endwin();
    ui_active = 0;
}

int ui_is_active(void)
{
    return ui_active;
}

/* ------------------------------------------------------------------ */
/* @...SAY                                                             */
/* ------------------------------------------------------------------ */

void ui_say(int row, int col, const char *text)
{
    if (!ui_active || !text) return;
    mvaddstr(row, col, text);

    /* Store for redraw on form window */
    SayEntry *se = calloc(1, sizeof(SayEntry));
    if (se) {
        se->row = row;
        se->col = col;
        strncpy(se->text, text, sizeof(se->text) - 1);
        se->next = say_list;
        say_list = se;
    }
}

void ui_say_redraw(void)
{
    if (!ui_active) return;
    SayEntry *se = say_list;
    while (se) {
        mvwaddstr(stdscr, se->row, se->col, se->text);
        se = se->next;
    }
}

void ui_say_clear(void)
{
    SayEntry *se = say_list;
    while (se) {
        SayEntry *next = se->next;
        free(se);
        se = next;
    }
    say_list = NULL;
}

void ui_refresh(void)
{
    if (ui_active) refresh();
}

/* ------------------------------------------------------------------ */
/* CLEAR                                                               */
/* ------------------------------------------------------------------ */

void ui_clear(void)
{
    if (!ui_active) return;
    erase();
    refresh();
}

/* ------------------------------------------------------------------ */
/* ? — free-form print at cursor                                       */
/* ------------------------------------------------------------------ */

static int print_row = 0;
static int print_col = 0;

void ui_print(const char *text)
{
    if (!ui_active || !text) return;
    int len = strlen(text);
    if (print_col + len >= COLS) {
        /* Wrap: print what fits, rest on next line */
        int fit = COLS - print_col;
        if (fit > 0) mvaddstr(print_row, print_col, text);
        print_row++;
        if (print_row >= LINES) { print_row = LINES - 1; }
        if (fit < len) mvaddstr(print_row, 0, text + fit);
        print_col = (print_col + len) % COLS;
    } else {
        mvaddstr(print_row, print_col, text);
        print_col += len;
    }
}

void ui_print_newline(void)
{
    if (!ui_active) return;
    print_row++;
    print_col = 0;
    if (print_row >= LINES) {
        /* Scroll: move everything up one line */
        scrl(1);
        print_row = LINES - 1;
    }
}

/* ------------------------------------------------------------------ */
/* @...GET — queue management                                          */
/* ------------------------------------------------------------------ */

UiGetField *ui_get_add(int row, int col, const char *varname, int field_len)
{
    UiGetField *gf = calloc(1, sizeof(UiGetField));
    if (!gf) return NULL;

    gf->row = row;
    gf->col = col;
    gf->field_len = field_len > 0 ? field_len : 10;
    if (varname)
        strncpy(gf->varname, varname, sizeof(gf->varname) - 1);

    /* Create the ncurses FIELD */
    gf->field = new_field(1, gf->field_len, row, col, 0, 10);
    if (!gf->field) {
        free(gf);
        return NULL;
    }
    set_field_back(gf->field, A_REVERSE);
    /* Ensure field is visible and editable */
    field_opts_on(gf->field, O_VISIBLE | O_EDIT | O_WRAP);

    /* Append to queue */
    if (!get_queue) {
        get_queue = gf;
        get_last  = gf;
    } else {
        get_last->next = gf;
        get_last = gf;
    }
    return gf;
}

void ui_get_set_picture(const char *picture)
{
    if (get_last && picture)
        strncpy(get_last->picture, picture, sizeof(get_last->picture) - 1);
}

void ui_get_set_range(const char *lo, const char *hi)
{
    if (get_last) {
        if (lo) strncpy(get_last->range_lo, lo, sizeof(get_last->range_lo) - 1);
        if (hi) strncpy(get_last->range_hi, hi, sizeof(get_last->range_hi) - 1);
    }
}

void ui_get_set_valid(const char *expr)
{
    if (get_last && expr)
        strncpy(get_last->valid_expr, expr, sizeof(get_last->valid_expr) - 1);
}

void ui_get_set_default(const char *expr)
{
    if (get_last && expr)
        strncpy(get_last->default_expr, expr, sizeof(get_last->default_expr) - 1);
}

void ui_get_set_focus(void)
{
    if (get_last)
        get_last->focus = 1;
}

/* ------------------------------------------------------------------ */
/* Field verification callback for RANGE / VALID                       */
/* ------------------------------------------------------------------ */

/* char_check filter: called per-character during input.
   Returns TRUE to accept the character, FALSE to reject. */
static int check_get_field(int c, const void *ptr)
{
    /* For now, accept all characters. RANGE/VALID checked after READ. */
    (void)c; (void)ptr;
    return TRUE;
}

/* ------------------------------------------------------------------ */
/* READ — process all pending GET fields                               */
/* ------------------------------------------------------------------ */

void ui_read(void)
{
    if (!ui_active || !get_queue) return;

    /* Build FIELD array for the FORM */
    int count = 0;
    UiGetField *gf = get_queue;
    while (gf) { count++; gf = gf->next; }

    FIELD **fields = calloc((size_t)(count + 1), sizeof(FIELD *));
    if (!fields) return;

    int idx = 0;
    int focus_idx = 0;
    gf = get_queue;
    while (gf) {
        /* Set default value if specified */
        if (gf->default_expr[0]) {
            /* For now, store the expr as default text — evaluate later */
            set_field_buffer(gf->field, 0, gf->default_expr);
        }

        /* Attach user pointer for verification */
        set_field_userptr(gf->field, gf);

        fields[idx] = gf->field;
        if (gf->focus) focus_idx = idx;
        idx++;
        gf = gf->next;
    }
    fields[count] = NULL;

    /* Create and post the FORM */
    FORM *form = new_form(fields);
    if (!form) { free(fields); return; }

    set_form_userptr(form, get_queue);
    post_form(form);

    /* Make the form window transparent so SAY text shows through */
    WINDOW *fw = form_win(form);
    wbkgd(fw, ' ' | COLOR_PAIR(0));
    keypad(fw, TRUE);

    /* Set focus field if specified */
    if (focus_idx > 0) {
        set_current_field(form, fields[focus_idx]);
    }

    pos_form_cursor(form);

    /* Redraw SAY text that may have been erased by the form */
    ui_say_redraw();
    touchwin(stdscr);
    refresh();
    wrefresh(fw);

    /* Drive the form — user navigates with Tab/Enter, exits with Esc */
    int c;
    int err_count = 0;
    while ((c = wgetch(fw)) != 27) { /* 27 = Escape */
        if (c == ERR) {
            if (++err_count > 10)
                break;  /* stdin exhausted (pipe mode), exit READ */
            continue;
        }
        err_count = 0;

        /* Map special keys to form_driver requests */
        int req = c;
        if (c == 9) /* TAB */
            req = REQ_NEXT_FIELD;
        else if (c == KEY_BTAB)
            req = REQ_PREV_FIELD;
        else if (c == 10 || c == 13) { /* LF / CR — Enter */
            /* Check if on last field — if so, exit READ */
            FIELD *cur_field = current_field(form);
            FIELD **flist = form_fields(form);
            int fcount = 0;
            while (flist[fcount]) fcount++;
            if (cur_field == flist[fcount - 1]) {
                break;  /* Enter on last field — done */
            }
            req = REQ_NEXT_FIELD;
        }
        else if (c == KEY_UP)
            req = REQ_PREV_FIELD;
        else if (c == KEY_DOWN)
            req = REQ_NEXT_FIELD;
        else if (c == KEY_BACKSPACE || c == 127) /* BS / DEL — delete prev char */
            req = REQ_DEL_PREV;
        else if (c == KEY_DC) /* Delete key — delete current char */
            req = REQ_DEL_CHAR;
        else if (c == KEY_LEFT)
            req = REQ_PREV_CHAR;
        else if (c == KEY_RIGHT)
            req = REQ_NEXT_CHAR;
        else if (c == KEY_HOME)
            req = REQ_BEG_FIELD;
        else if (c == KEY_END)
            req = REQ_END_FIELD;

        switch (form_driver(form, req)) {
            case E_OK:
                pos_form_cursor(form);
                refresh();
                wrefresh(fw);
                break;
            case E_REQUEST_DENIED:
            case E_INVALID_FIELD:
                /* ignore */
                break;
        }
    }

    /* Validate fields so input is committed to buffers before reading */
    form_driver(form, REQ_VALIDATION);

    /* Copy field buffers back to dBASE variables */
    gf = get_queue;
    while (gf) {
        const char *buf = field_buffer(gf->field, 0);
        if (buf && gf->varname[0]) {
            vars_set_str(gf->varname, buf);
        }
        gf = gf->next;
    }

    unpost_form(form);
    free_form(form);
    free(fields);

    /* Restore stdscr after form removal so subsequent SAY/? render correctly */
    touchwin(stdscr);
    ui_say_redraw();
    refresh();
}

/* ------------------------------------------------------------------ */
/* Cleanup                                                             */
/* ------------------------------------------------------------------ */

void ui_get_clear(void)
{
    UiGetField *gf = get_queue;
    while (gf) {
        UiGetField *next = gf->next;
        if (gf->field) free_field(gf->field);
        free(gf);
        gf = next;
    }
    get_queue = NULL;
    get_last  = NULL;
}
