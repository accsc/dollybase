/*
 * ui.c — ncurses UI layer for dollybase interpreter
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
#include <unistd.h>

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

/* TEXT block lines — stored so we can redraw after form is posted */
TextEntry *text_list = NULL;
int text_row = 0;  /* Current row for TEXT output */

/* ------------------------------------------------------------------ */
/* Lifecycle                                                           */
/* ------------------------------------------------------------------ */

int ui_init(void)
{
    initscr();
    cbreak();            /* no wait for Enter                          */
    noecho();            /* don't echo input automatically             */
    keypad(stdscr, TRUE);/* enable special keys                        */
    scrollok(stdscr, TRUE); /* allow auto-scroll when cursor goes past bottom */
    scrl(1);                 /* actually enable scrolling on stdscr      */
    curs_set(1);            /* visible cursor                           */
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
    refresh();

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

void ui_text_redraw(void)
{
    if (!ui_active) return;
    TextEntry *te = text_list;
    while (te) {
        mvwaddstr(stdscr, te->row, 0, te->text);
        te = te->next;
    }
}

void ui_text_clear(void)
{
    TextEntry *te = text_list;
    while (te) {
        TextEntry *next = te->next;
        free(te);
        te = next;
    }
    text_list = NULL;
    text_row = 0;
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
    ui_text_clear();
    ui_say_clear();
    text_row = 0;
}

/* ------------------------------------------------------------------ */
/* RECTANGLE — @...TO                                                  */
/* ------------------------------------------------------------------ */

void ui_rect(int r1, int c1, int r2, int c2, int double_line)
{
    if (!ui_active) return;
    int r, c;
    char h_char, v_char, tl, tr, bl, br;

    if (double_line) {
        h_char = '=';
        v_char = '|';
        tl = '+'; tr = '+'; bl = '+'; br = '+';
    } else {
        h_char = '-';
        v_char = '|';
        tl = '+'; tr = '+'; bl = '+'; br = '+';
    }

    /* Ensure r1<=r2 and c1<=c2 */
    if (r1 > r2) { int tmp = r1; r1 = r2; r2 = tmp; }
    if (c1 > c2) { int tmp = c1; c1 = c2; c2 = tmp; }

    /* Top border */
    mvaddch(r1, c1, tl);
    for (c = c1 + 1; c < c2; c++)
        mvaddch(r1, c, h_char);
    mvaddch(r1, c2, tr);

    /* Bottom border */
    mvaddch(r2, c1, bl);
    for (c = c1 + 1; c < c2; c++)
        mvaddch(r2, c, h_char);
    mvaddch(r2, c2, br);

    /* Side borders */
    for (r = r1 + 1; r < r2; r++) {
        mvaddch(r, c1, v_char);
        mvaddch(r, c2, v_char);
    }

    refresh();
}

/* ------------------------------------------------------------------ */
/* CLEAR RECT — @...CLEAR [TO @...]                                    */
/* ------------------------------------------------------------------ */

void ui_clear_rect(int r1, int c1, int r2, int c2)
{
    if (!ui_active) return;
    int r, c;
    int maxy = getmaxy(stdscr);
    int maxx = getmaxx(stdscr);

    /* Ensure r1<=r2 and c1<=c2 */
    if (r1 > r2) { int tmp = r1; r1 = r2; r2 = tmp; }
    if (c1 > c2) { int tmp = c1; c1 = c2; c2 = tmp; }

    /* Clamp to screen bounds */
    if (r2 >= maxy) r2 = maxy - 1;
    if (c2 >= maxx) c2 = maxx - 1;
    if (r1 < 0) r1 = 0;
    if (c1 < 0) c1 = 0;

    for (r = r1; r <= r2; r++) {
        for (c = c1; c <= c2; c++) {
            mvaddch(r, c, ' ');
        }
    }

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
    addstr(text);
    refresh();
}

void ui_print_newline(void)
{
    if (!ui_active) return;
    addch('\n');
    refresh();
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

    /* Redraw SAY text and TEXT block that may have been erased by the form */
    ui_say_redraw();
    ui_text_redraw();
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
            /* Trim trailing spaces from field buffer (ncurses pads with spaces) */
            char buf_trimmed[256];
            strncpy(buf_trimmed, buf, sizeof(buf_trimmed) - 1);
            buf_trimmed[sizeof(buf_trimmed) - 1] = '\0';
            {
                char *p = buf_trimmed + strlen(buf_trimmed) - 1;
                while (p >= buf_trimmed && *p == ' ') *p-- = '\0';
            }

            /* Preserve existing variable type if possible */
            ExprValue existing = vars_get(gf->varname);
            if (existing.type == VAL_REAL || existing.type == VAL_INTEGER) {
                /* Variable was numeric — try to parse as number */
                char *endptr = NULL;
                double dval = strtod(buf_trimmed, &endptr);
                if (endptr && *endptr == '\0' && buf_trimmed[0] != '\0') {
                    /* Successfully parsed as number */
                    ExprValue v = val_real(dval);
                    vars_set(gf->varname, &v);
                    free_value(&v);
                } else {
                    /* Fallback to string */
                    vars_set_str(gf->varname, buf_trimmed);
                }
                free_value(&existing);
            } else {
                /* Variable is string or doesn't exist — store as string */
                vars_set_str(gf->varname, buf_trimmed);
                free_value(&existing);
            }
        }
        gf = gf->next;
    }

    unpost_form(form);
    free_form(form);
    free(fields);

    /* Restore stdscr after form removal so subsequent SAY/? render correctly */
    touchwin(stdscr);
    ui_say_redraw();
    ui_text_redraw();
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

/* ------------------------------------------------------------------ */
/* BROWSE — full-screen grid viewer/editor                            */
/* ------------------------------------------------------------------ */

#include "workarea.h"

/* Browse state */
typedef struct {
    int       field_count;       /* number of visible fields           */
    int       *field_indices;    /* 1-based field indices in DBF       */
    int       *col_widths;       /* computed column widths             */
    int       total_records;
    int       start_rec;         /* first record shown on screen       */
    int       cur_row;           /* current data row (0-based on screen) */
    int       cur_col;           /* current column index               */
    int       data_rows;         /* number of data rows on screen      */
} BrowseState;

/* Compute column widths from field names and sample data */
static void browse_compute_widths(BrowseState *bs)
{
    for (int i = 0; i < bs->field_count; i++) {
        int idx = bs->field_indices[i];
        char *name = wa_field_name(idx);
        int w = (int)strlen(name);
        /* Minimum width based on field size */
        int ftype = wa_field_type(idx);
        if (ftype == 'N') {
            int fw = wa_field_type(idx); (void)fw;
            /* Use field length for numeric */
            char *sample = wa_get_field(idx);
            if (sample) {
                int sl = (int)strlen(sample);
                if (sl > w) w = sl;
                free(sample);
            }
            if (w < 6) w = 6;
        } else if (ftype == 'D') {
            if (w < 10) w = 10;
        } else if (ftype == 'L') {
            if (w < 5) w = 5;
        } else if (ftype == 'M') {
            if (w < 4) w = 4;
        } else {
            /* Character field — cap at 20 for display */
            char *sample = wa_get_field(idx);
            if (sample) {
                int sl = (int)strlen(sample);
                if (sl > w) w = sl;
                free(sample);
            }
            if (w > 20) w = 20;
        }
        bs->col_widths[i] = w + 2; /* padding */
        free(name);
    }
}

/* Draw the BROWSE grid */
static void browse_draw(BrowseState *bs)
{
    DATABASEDBF *db = wa_db();
    int status_row = LINES - 1;
    int header_row = 0;
    int first_data = 2; /* row 0 = header, row 1 = separator */

    erase();

    /* --- Header row: field names --- */
    int x = 0;
    mvaddstr(header_row, x, "Rec");
    x += 4;
    mvaddch(header_row, x, '|'); /* separator */
    x++;
    for (int i = 0; i < bs->field_count; i++) {
        char *name = wa_field_name(bs->field_indices[i]);
        int w = bs->col_widths[i];
        char buf[256] = "";
        snprintf(buf, sizeof(buf), "%-*s", w - 2, name);
        mvaddstr(header_row, x, buf);
        x += w;
        if (x >= COLS - 1) break;
        mvaddch(header_row, x, '|');
        x++;
        free(name);
    }

    /* Separator line */
    for (int c = 0; c < COLS; c++)
        mvaddch(header_row + 1, c, '-');

    /* --- Data rows --- */
    int rec = bs->start_rec;
    for (int r = 0; r < bs->data_rows && rec <= bs->total_records; r++) {
        int row = first_data + r;
        if (row >= status_row) break;

        /* Navigate to this record */
        wa_goto(rec);

        x = 0;
        /* Record number — prefix with * if deleted */
        char recbuf[16];
        int del = is_deleted(db) == VERITAS;
        if (del) {
            mvaddch(row, x, '*');
            snprintf(recbuf, sizeof(recbuf), "%3d", rec);
            mvaddstr(row, x + 1, recbuf);
        } else {
            snprintf(recbuf, sizeof(recbuf), "%4d", rec);
            mvaddstr(row, x, recbuf);
        }
        x += 4;
        mvaddch(row, x, '|');
        x++;

        for (int i = 0; i < bs->field_count; i++) {
            int w = bs->col_widths[i];
            char *val = wa_get_field(bs->field_indices[i]);
            char buf[256] = "";
            if (val) {
                /* Truncate if needed */
                snprintf(buf, sizeof(buf), "%-*s", w - 2, val);
                free(val);
            }
            /* Highlight current cell */
            if (r == bs->cur_row && i == bs->cur_col)
                attron(A_REVERSE);
            mvaddstr(row, x, buf);
            if (r == bs->cur_row && i == bs->cur_col)
                attroff(A_REVERSE);
            x += w;
            if (x >= COLS - 1) break;
            mvaddch(row, x, '|');
            x++;
        }
        rec++;
    }

    /* --- Status bar --- */
    int cur_rec = bs->start_rec + bs->cur_row;
    char status[256];
    snprintf(status, sizeof(status),
             "Rec: %d/%d  F1:Help  F2:Del  F3:Quit  F4:Recall  Enter:Edit  Arrows:Nav",
             cur_rec, bs->total_records);
    mvaddstr(status_row, 0, status);

    touchwin(stdscr);
    refresh();
}

/* Edit a single field in-place on stdscr (no sub-windows) */
static void browse_edit_field(BrowseState *bs, int rec, int col_idx)
{
    (void)rec; /* rec is already positioned via wa_goto before call */
    int fidx = bs->field_indices[col_idx];
    char *old_val = wa_get_field(fidx);
    char buf[256] = "";
    if (old_val) {
        strncpy(buf, old_val, sizeof(buf) - 1);
        free(old_val);
    }

    /* Screen position of the cell content (after the '|' separator) */
    int row = 2 + bs->cur_row; /* header(0) + sep(1) + data starts at 2 */
    int x = 5; /* "Rec|" = 4 + 1 */
    for (int i = 0; i < col_idx; i++) {
        x += bs->col_widths[i] + 1;
    }

    int edit_w = bs->col_widths[col_idx] - 2;
    if (edit_w < 1) edit_w = 1;

    /* Line editor on stdscr — track cursor position in `cp` */
    int cp = (int)strlen(buf); /* cursor at end of existing content */
    int changed = 0;

    /* Draw initial cell state */
    mvaddstr(row, x, buf);
    {
        int cur_len = (int)strlen(buf);
        for (int p = cur_len; p < edit_w; p++)
            mvaddch(row, x + p, ' ');
    }
    attron(A_REVERSE);
    move(row, x + cp);
    attroff(A_REVERSE);
    curs_set(2);
    refresh();

    int c;
    while ((c = getch()) != 10 && c != 13) {
        if (c == 27) { /* Escape = cancel */
            changed = -1;
            break;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (cp > 0) {
                memmove(buf + cp - 1, buf + cp, strlen(buf) - cp + 1);
                cp--;
                changed = 1;
            }
        } else if (c == KEY_LEFT) {
            if (cp > 0) cp--;
        } else if (c == KEY_RIGHT) {
            if (cp < (int)strlen(buf)) cp++;
        } else if (c == KEY_HOME) {
            cp = 0;
        } else if (c == KEY_END) {
            cp = (int)strlen(buf);
        } else if (c >= 32 && c < 127) {
            if ((int)strlen(buf) < (int)sizeof(buf) - 1) {
                memmove(buf + cp + 1, buf + cp, strlen(buf) - cp + 1);
                buf[cp] = (char)c;
                if (cp < edit_w) cp++;
                changed = 1;
            }
        } else if (c == KEY_DC) {
            if (cp < (int)strlen(buf)) {
                memmove(buf + cp, buf + cp + 1, strlen(buf) - cp);
                changed = 1;
            }
        } else {
            continue;
        }

        /* Redraw the cell */
        mvaddstr(row, x, buf);
        {
            int cur_len = (int)strlen(buf);
            for (int p = cur_len; p < edit_w; p++)
                mvaddch(row, x + p, ' ');
        }
        attron(A_REVERSE);
        move(row, x + cp);
        attroff(A_REVERSE);
        refresh();
    }
    curs_set(1);

    if (changed == 1) {
        char *fname = wa_field_name(fidx);
        wa_replace(fname, buf);
        free(fname);
    }
    /* If changed == -1 (Escape), don't update — old value stays */
}

/* ------------------------------------------------------------------ */
/* Public: ui_browse                                                  */
/* ------------------------------------------------------------------ */

int ui_browse(const char *fields)
{
    DATABASEDBF *db = wa_db();
    if (!db || db->camposn == 0) {
        if (ui_is_active()) {
            mvaddstr(LINES - 1, 0, "No database open for BROWSE");
            refresh();
        }
        return -1;
    }

    BrowseState bs;
    memset(&bs, 0, sizeof(bs));

    /* Determine which fields to show */
    if (fields && fields[0]) {
        /* Parse comma-separated field names */
        char *fcpy = strdup(fields);
        char *saveptr = NULL;
        char *tok = strtok_r(fcpy, ",", &saveptr);
        int count = 0;
        while (tok) {
            /* Trim whitespace */
            while (*tok == ' ') tok++;
            count++;
            tok = strtok_r(NULL, ",", &saveptr);
        }
        bs.field_count = count;
        bs.field_indices = calloc((size_t)count, sizeof(int));
        bs.col_widths = calloc((size_t)count, sizeof(int));

        tok = strtok_r(fcpy, ",", &saveptr);
        for (int i = 0; i < count && tok; i++) {
            while (*tok == ' ') tok++;
            int fidx = wa_field_to_number(tok);
            if (fidx > 0)
                bs.field_indices[i] = fidx;
            else
                bs.field_indices[i] = i + 1; /* fallback */
            tok = strtok_r(NULL, ",", &saveptr);
        }
        free(fcpy);
    } else {
        /* All fields */
        bs.field_count = db->camposn;
        bs.field_indices = calloc((size_t)bs.field_count, sizeof(int));
        bs.col_widths = calloc((size_t)bs.field_count, sizeof(int));
        for (int i = 0; i < bs.field_count; i++)
            bs.field_indices[i] = i + 1;
    }

    bs.total_records = db->recnos;
    bs.data_rows = LINES - 3; /* header + separator + status bar */
    if (bs.data_rows < 1) bs.data_rows = 1;
    bs.cur_row = 0;
    bs.cur_col = 0;
    bs.start_rec = 1;

    /* Compute column widths */
    browse_compute_widths(&bs);

    /* Save original record position */
    int saved_rec = wa_recno();

    /* Main loop */
    int done = 0;
    while (!done) {
        browse_draw(&bs);

        /* Use halfdelay so we can distinguish bare Escape from F-key sequences.
           In cbreak mode, F keys arrive as ESC + sequence. A bare Escape is
           sent alone. We wait briefly to see if more bytes follow. */
        int c = getch();
        if (c == ERR) continue;

        /* If we got Escape, distinguish bare Escape from F-key escape
           sequences by checking if more input bytes are pending. */
        if (c == 27) {
            nodelay(stdscr, TRUE);
            /* Collect all pending bytes of the escape sequence */
            int seq_bytes[16];
            seq_bytes[0] = 27;
            int slen = 1;
            /* Wait for the full sequence to arrive (up to 200ms total) */
            for (int i = 0; i < 20 && slen < 16; i++) {
                usleep(10000); /* 10ms per iteration */
                int c2 = getch();
                if (c2 == ERR) continue;
                seq_bytes[slen++] = c2;
            }
            nodelay(stdscr, FALSE);
            if (slen > 1) {
                /* Escape sequence received — parse it */
                c = 27; /* fallback */
                if (slen >= 3 && seq_bytes[1] == '[') {
                    /* CSI sequence */
                    if (slen == 3) {
                        switch (seq_bytes[2]) {
                            case 'A': c = KEY_UP; break;
                            case 'B': c = KEY_DOWN; break;
                            case 'C': c = KEY_RIGHT; break;
                            case 'D': c = KEY_LEFT; break;
                        }
                    }
                    /* F keys: [11~=F1, [12~=F2, [13~=F3, [14~=F4,
                               [15~=F5, [17~=F6, [18~=F7, [19~=F8 */
                    if (slen >= 4 && seq_bytes[slen - 1] == '~') {
                        if (seq_bytes[2] == '1') {
                            switch (seq_bytes[3]) {
                                case '1': c = KEY_F(1); break;
                                case '2': c = KEY_F(2); break;
                                case '3': c = KEY_F(3); break;
                                case '4': c = KEY_F(4); break;
                                case '5': c = KEY_F(5); break;
                            }
                        } else if (seq_bytes[2] == '1' && seq_bytes[3] == '7') {
                            c = KEY_F(6);
                        } else if (seq_bytes[2] == '1' && seq_bytes[3] == '8') {
                            c = KEY_F(7);
                        } else if (seq_bytes[2] == '1' && seq_bytes[3] == '9') {
                            c = KEY_F(8);
                        }
                    }
                } else if (slen >= 3 && seq_bytes[1] == 'O') {
                    /* SS3 sequence — F1-F4 on many terminals */
                    switch (seq_bytes[2]) {
                        case 'P': c = KEY_F(1); break;
                        case 'Q': c = KEY_F(2); break;
                        case 'R': c = KEY_F(3); break;
                        case 'S': c = KEY_F(4); break;
                    }
                }
                /* If still 27, unrecognized sequence — treat as quit */
            }
            /* If slen == 1, it was a bare Escape — c stays 27 = quit */
        }

        int cur_rec = bs.start_rec + bs.cur_row;

        switch (c) {
            case 27: /* Escape = quit */
            case KEY_F(3):
                done = 1;
                break;

            case KEY_UP:
                if (bs.cur_row > 0) {
                    bs.cur_row--;
                } else if (bs.start_rec > 1) {
                    bs.start_rec--;
                }
                break;

            case KEY_DOWN:
                if (bs.cur_row < bs.data_rows - 1 &&
                    bs.start_rec + bs.cur_row + 1 <= bs.total_records) {
                    bs.cur_row++;
                } else if (bs.start_rec + bs.data_rows <= bs.total_records) {
                    bs.start_rec++;
                }
                break;

            case KEY_LEFT:
                if (bs.cur_col > 0) {
                    bs.cur_col--;
                } else if (bs.cur_row > 0) {
                    bs.cur_row--;
                    bs.cur_col = bs.field_count - 1;
                } else if (bs.start_rec > 1) {
                    bs.start_rec--;
                }
                break;

            case KEY_RIGHT:
                if (bs.cur_col < bs.field_count - 1) {
                    bs.cur_col++;
                } else if (bs.cur_row < bs.data_rows - 1 &&
                           bs.start_rec + bs.cur_row + 1 <= bs.total_records) {
                    bs.cur_row++;
                    bs.cur_col = 0;
                } else if (bs.start_rec + bs.data_rows <= bs.total_records) {
                    bs.start_rec++;
                    bs.cur_col = 0;
                }
                break;

            case KEY_PPAGE: /* Page Up */
                if (bs.start_rec > bs.data_rows)
                    bs.start_rec -= bs.data_rows;
                else
                    bs.start_rec = 1;
                bs.cur_row = 0;
                break;

            case KEY_NPAGE: /* Page Down */
                if (bs.start_rec + bs.data_rows * 2 <= bs.total_records)
                    bs.start_rec += bs.data_rows;
                else
                    bs.start_rec = bs.total_records - bs.data_rows + 1;
                if (bs.start_rec < 1) bs.start_rec = 1;
                bs.cur_row = bs.data_rows - 1;
                break;

            case KEY_HOME:
                bs.start_rec = 1;
                bs.cur_row = 0;
                bs.cur_col = 0;
                break;

            case KEY_END:
                bs.start_rec = bs.total_records - bs.data_rows + 1;
                if (bs.start_rec < 1) bs.start_rec = 1;
                bs.cur_row = bs.data_rows - 1;
                bs.cur_col = bs.field_count - 1;
                break;

            case 10: /* LF */
            case 13: /* CR — Enter to edit */
                if (cur_rec >= 1 && cur_rec <= bs.total_records) {
                    wa_goto(cur_rec);
                    browse_edit_field(&bs, cur_rec, bs.cur_col);
                }
                break;

            case KEY_F(2): /* Delete record */
                if (cur_rec >= 1 && cur_rec <= bs.total_records) {
                    wa_goto(cur_rec);
                    wa_delete();
                    /* Flash message on status bar */
                    int sr = LINES - 1;
                    mvaddstr(sr, 0, "Record deleted (flagged)                                                                  ");
                    mvaddstr(sr, 0, "Record deleted (flagged)");
                    refresh();
                    usleep(500000); /* 0.5s flash */
                }
                break;

            case KEY_F(4): /* Recall record */
                if (cur_rec >= 1 && cur_rec <= bs.total_records) {
                    wa_goto(cur_rec);
                    wa_recall();
                    /* Flash message on status bar */
                    int sr = LINES - 1;
                    mvaddstr(sr, 0, "Record recalled                                                                          ");
                    mvaddstr(sr, 0, "Record recalled");
                    refresh();
                    usleep(500000);
                }
                break;

            case KEY_F(1): /* Help — just show on status, already there */
                break;
        }
    }

    /* Restore original record position */
    wa_goto(saved_rec);

    /* Cleanup */
    free(bs.field_indices);
    free(bs.col_widths);

    /* Redraw screen after browse */
    touchwin(stdscr);
    refresh();

    return 0;
}
