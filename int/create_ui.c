/*
 * create_ui.c — CREATE DATABASE full-screen UI
 *
 * Launched by exec_create() from the interpreter.
 * Follows the same pattern as ui_browse() in ui.c.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>
#include <ncurses.h>
#include "../libdbase_4/libdbase.h"
#include "ui.h"

#define CREATE_COL_NAME    0
#define CREATE_COL_TYPE    1
#define CREATE_COL_WIDTH   2
#define CREATE_COL_DEC     3
#define CREATE_NUM_COLS    4

#define CREATE_W_NAME      16
#define CREATE_W_TYPE       6
#define CREATE_W_WIDTH      7
#define CREATE_W_DEC        4

#define CREATE_FIRST_DATA_ROW 3

#define CREATE_MAX_FIELDS  1024
#define CREATE_INIT_CAP     10

typedef struct {
    char        filename[256];
    int         field_count;
    int         max_fields;
    char        *field_names;      /* [max_fields][11] */
    char        *field_types;      /* [max_fields] */
    int         *field_widths;     /* [max_fields] */
    int         *field_decimals;   /* [max_fields] */
    int         cur_row;
    int         cur_col;
    int         data_rows;
    int         start_row;
} CreateState;

/* ------------------------------------------------------------------ */
/* Init / Free / Grow                                                  */
/* ------------------------------------------------------------------ */

static void create_init(CreateState *cs, const char *filename)
{
    strncpy(cs->filename, filename, sizeof(cs->filename) - 1);
    cs->filename[sizeof(cs->filename) - 1] = '\0';
    cs->field_count = 0;
    cs->max_fields = CREATE_INIT_CAP;
    cs->field_names = calloc((size_t)cs->max_fields, 11);
    cs->field_types = calloc((size_t)cs->max_fields, 1);
    cs->field_widths = calloc((size_t)cs->max_fields, sizeof(int));
    cs->field_decimals = calloc((size_t)cs->max_fields, sizeof(int));
    cs->cur_row = 0;
    cs->cur_col = CREATE_COL_NAME;
    cs->start_row = 0;
    cs->data_rows = LINES - 4; /* title + header + sep + status */
    if (cs->data_rows < 1) cs->data_rows = 1;
}

static void create_free(CreateState *cs)
{
    free(cs->field_names);
    free(cs->field_types);
    free(cs->field_widths);
    free(cs->field_decimals);
}

static void create_grow(CreateState *cs)
{
    if (cs->field_count >= cs->max_fields) {
        cs->max_fields *= 2;
        if (cs->max_fields > CREATE_MAX_FIELDS) cs->max_fields = CREATE_MAX_FIELDS;
        cs->field_names = realloc(cs->field_names, (size_t)cs->max_fields * 11);
        cs->field_types = realloc(cs->field_types, (size_t)cs->max_fields);
        cs->field_widths = realloc(cs->field_widths, (size_t)cs->max_fields * sizeof(int));
        cs->field_decimals = realloc(cs->field_decimals, (size_t)cs->max_fields * sizeof(int));
    }
}

/* ------------------------------------------------------------------ */
/* Add / Remove fields                                                  */
/* ------------------------------------------------------------------ */

static void create_add_field(CreateState *cs)
{
    create_grow(cs);
    int idx = cs->field_count;
    snprintf(cs->field_names + idx * 11, 11, "F%d", idx + 1);
    cs->field_types[idx] = 'C';
    cs->field_widths[idx] = 10;
    cs->field_decimals[idx] = 0;
    cs->field_count++;

    /* Scroll to show new row */
    if (idx >= cs->start_row + cs->data_rows)
        cs->start_row = idx - cs->data_rows + 1;
    cs->cur_row = idx - cs->start_row;
    cs->cur_col = CREATE_COL_NAME;
}

static void create_remove_field(CreateState *cs, int row)
{
    if (cs->field_count <= 0) return;
    for (int i = row; i < cs->field_count - 1; i++) {
        memcpy(cs->field_names + i * 11, cs->field_names + (i + 1) * 11, 11);
        cs->field_types[i] = cs->field_types[i + 1];
        cs->field_widths[i] = cs->field_widths[i + 1];
        cs->field_decimals[i] = cs->field_decimals[i + 1];
    }
    cs->field_count--;
    memset(cs->field_names + (cs->field_count) * 11, 0, 11);

    if (cs->cur_row >= cs->field_count && cs->cur_row > 0)
        cs->cur_row--;
    if (cs->cur_row < 0) cs->cur_row = 0;
}

/* ------------------------------------------------------------------ */
/* Draw the grid                                                        */
/* ------------------------------------------------------------------ */

static void create_draw(CreateState *cs)
{
    int title_row = 0;
    int header_row = 1;
    int sep_row = 2;
    int first_data = CREATE_FIRST_DATA_ROW;
    int status_row = LINES - 1;

    erase();

    /* Title */
    mvaddstr(title_row, 0, "CREATE DATABASE:");
    mvaddstr(title_row, 17, cs->filename);

    /* Header row */
    int x = 0;
    mvaddch(header_row, x, '+'); x++;
    char buf[64];
    snprintf(buf, sizeof(buf), "%-*s", CREATE_W_NAME, "Field name");
    mvaddstr(header_row, x, buf); x += CREATE_W_NAME;
    mvaddch(header_row, x, '+'); x++;
    snprintf(buf, sizeof(buf), "%-*s", CREATE_W_TYPE, "Type");
    mvaddstr(header_row, x, buf); x += CREATE_W_TYPE;
    mvaddch(header_row, x, '+'); x++;
    snprintf(buf, sizeof(buf), "%-*s", CREATE_W_WIDTH, "Width");
    mvaddstr(header_row, x, buf); x += CREATE_W_WIDTH;
    mvaddch(header_row, x, '+'); x++;
    snprintf(buf, sizeof(buf), "%-*s", CREATE_W_DEC, "Dec");
    mvaddstr(header_row, x, buf); x += CREATE_W_DEC;
    mvaddch(header_row, x, '+');

    /* Separator line */
    x = 0;
    mvaddch(sep_row, x, '+'); x++;
    for (int i = 0; i < CREATE_W_NAME; i++) mvaddch(sep_row, x++, '-');
    mvaddch(sep_row, x, '+'); x++;
    for (int i = 0; i < CREATE_W_TYPE; i++) mvaddch(sep_row, x++, '-');
    mvaddch(sep_row, x, '+'); x++;
    for (int i = 0; i < CREATE_W_WIDTH; i++) mvaddch(sep_row, x++, '-');
    mvaddch(sep_row, x, '+'); x++;
    for (int i = 0; i < CREATE_W_DEC; i++) mvaddch(sep_row, x++, '-');
    mvaddch(sep_row, x, '+');

    /* Data rows */
    int rec = cs->start_row;
    for (int r = 0; r < cs->data_rows; r++) {
        int row = first_data + r;
        if (row >= status_row) break;

        x = 0;
        mvaddch(row, x, '|'); x++;

        if (rec < cs->field_count) {
            /* Field name */
            snprintf(buf, sizeof(buf), "%-*s", CREATE_W_NAME,
                     cs->field_names + rec * 11);
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_NAME)
                attron(A_REVERSE);
            mvaddstr(row, x, buf); x += CREATE_W_NAME;
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_NAME)
                attroff(A_REVERSE);

            mvaddch(row, x, '|'); x++;

            /* Type */
            snprintf(buf, sizeof(buf), "%c", cs->field_types[rec]);
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_TYPE)
                attron(A_REVERSE);
            mvaddstr(row, x, buf);
            for (int p = 1; p < CREATE_W_TYPE; p++)
                mvaddch(row, x + p, ' ');
            x += CREATE_W_TYPE;
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_TYPE)
                attroff(A_REVERSE);

            mvaddch(row, x, '|'); x++;

            /* Width (right-aligned) */
            snprintf(buf, sizeof(buf), "%*d", CREATE_W_WIDTH,
                     cs->field_widths[rec]);
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_WIDTH)
                attron(A_REVERSE);
            mvaddstr(row, x, buf); x += CREATE_W_WIDTH;
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_WIDTH)
                attroff(A_REVERSE);

            mvaddch(row, x, '|'); x++;

            /* Dec (right-aligned) */
            snprintf(buf, sizeof(buf), "%*d", CREATE_W_DEC,
                     cs->field_decimals[rec]);
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_DEC)
                attron(A_REVERSE);
            mvaddstr(row, x, buf); x += CREATE_W_DEC;
            if (r == cs->cur_row && cs->cur_col == CREATE_COL_DEC)
                attroff(A_REVERSE);

            mvaddch(row, x, '|');
        } else {
            /* Empty row padding */
            int total_w = CREATE_W_NAME + CREATE_W_TYPE +
                          CREATE_W_WIDTH + CREATE_W_DEC + 4;
            for (int c = 0; c < total_w; c++) {
                if (c == CREATE_W_NAME + 1 ||
                    c == CREATE_W_NAME + CREATE_W_TYPE + 2 ||
                    c == CREATE_W_NAME + CREATE_W_TYPE + CREATE_W_WIDTH + 3) {
                    mvaddch(row, x, '|'); x++;
                } else {
                    mvaddch(row, x, ' '); x++;
                }
            }
        }
        rec++;
    }

    /* Status bar */
    char status[256];
    snprintf(status, sizeof(status),
             "Fields: %d  Esc:Cancel  ^End:Save  ^N:Add  ^U:Del  Arrows:Nav  Enter:Edit",
             cs->field_count);
    mvaddstr(status_row, 0, status);

    touchwin(stdscr);
    refresh();
}

/* ------------------------------------------------------------------ */
/* Inline text editor (for Name, Width, Dec columns)                    */
/* ------------------------------------------------------------------ */

static void create_edit_text(int row, int col, char *buf, int max_w, int max_len)
{
    int cp = (int)strlen(buf);
    curs_set(2);

    mvaddstr(row, col, buf);
    for (int p = (int)strlen(buf); p < max_w; p++)
        mvaddch(row, col + p, ' ');
    attron(A_REVERSE);
    move(row, col + cp);
    attroff(A_REVERSE);
    refresh();

    int c;
    while ((c = getch()) != 10 && c != 13) {
        if (c == 27) {
            curs_set(1);
            return;
        } else if (c == KEY_BACKSPACE || c == 127) {
            if (cp > 0) {
                memmove(buf + cp - 1, buf + cp, strlen(buf) - cp + 1);
                cp--;
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
            if ((int)strlen(buf) < max_len) {
                memmove(buf + cp + 1, buf + cp, strlen(buf) - cp + 1);
                buf[cp] = (char)c;
                if (cp < max_w - 1) cp++;
            }
        } else if (c == KEY_DC) {
            if (cp < (int)strlen(buf)) {
                memmove(buf + cp, buf + cp + 1, strlen(buf) - cp);
            }
        } else {
            continue;
        }
        mvaddstr(row, col, buf);
        for (int p = (int)strlen(buf); p < max_w; p++)
            mvaddch(row, col + p, ' ');
        attron(A_REVERSE);
        move(row, col + cp);
        attroff(A_REVERSE);
        refresh();
    }
    curs_set(1);
}

static void create_edit_number(int row, int col, int *value,
                                int max_w, int min_val, int max_val)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", *value);
    char orig_buf[32];
    strncpy(orig_buf, buf, sizeof(orig_buf));

    create_edit_text(row, col, buf, max_w, (int)sizeof(buf) - 1);

    char *endp;
    long val = strtol(buf, &endp, 10);
    if (*endp == '\0' && val >= min_val && val <= max_val) {
        *value = (int)val;
    }
    /* If invalid, just keep original — no need to restore buf since
       the caller reads *value directly */
    (void)orig_buf;
}

/* ------------------------------------------------------------------ */
/* Type dropdown (inline popup: C/N/D/L/M)                              */
/* ------------------------------------------------------------------ */

static void create_type_dropdown(CreateState *cs, int row, int col)
{
    const char type_chars[] = {'C', 'N', 'D', 'L', 'M'};
    const char *labels[] = {"Character", "Numeric", "Date", "Logical", "Memo"};
    int num_types = 5;
    int sel = 0;

    int abs_row = cs->start_row + cs->cur_row;
    if (abs_row < cs->field_count) {
        for (int i = 0; i < num_types; i++) {
            if (type_chars[i] == cs->field_types[abs_row]) {
                sel = i;
                break;
            }
        }
    }

    int drop_row = row + 1;
    int drop_col = col;

    while (1) {
        for (int i = 0; i < num_types; i++) {
            char line[64];
            int line_row = drop_row + i;
            if (line_row >= LINES - 1) break;
            if (i == sel) {
                attron(A_REVERSE);
                snprintf(line, sizeof(line), "> %c - %-9s",
                         type_chars[i], labels[i]);
            } else {
                snprintf(line, sizeof(line), "  %c - %-9s",
                         type_chars[i], labels[i]);
            }
            mvaddstr(line_row, drop_col, line);
            if (i == sel) attroff(A_REVERSE);
            int len = (int)strlen(line);
            for (int p = len; p < 24; p++)
                mvaddch(line_row, drop_col + p, ' ');
        }
        refresh();

        int c = getch();
        if (c == KEY_UP) {
            if (sel > 0) sel--;
        } else if (c == KEY_DOWN) {
            if (sel < num_types - 1) sel++;
        } else if (c == 10 || c == 13) {
            cs->field_types[abs_row] = type_chars[sel];
            if (type_chars[sel] != 'N')
                cs->field_decimals[abs_row] = 0;
            if (type_chars[sel] == 'L')
                cs->field_widths[abs_row] = 1;
            /* Clear dropdown area */
            for (int i = 0; i < num_types; i++) {
                int line_row = drop_row + i;
                if (line_row >= LINES - 1) break;
                mvaddstr(line_row, drop_col, "                        ");
            }
            break;
        } else if (c == 27) {
            for (int i = 0; i < num_types; i++) {
                int line_row = drop_row + i;
                if (line_row >= LINES - 1) break;
                mvaddstr(line_row, drop_col, "                        ");
            }
            break;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Escape sequence parser (same pattern as ui.c browse)                 */
/* ------------------------------------------------------------------ */

static int parse_escape_sequence(void)
{
    nodelay(stdscr, TRUE);
    int seq_bytes[16];
    seq_bytes[0] = 27;
    int slen = 1;
    for (int i = 0; i < 20 && slen < 16; i++) {
        usleep(10000);
        int c2 = getch();
        if (c2 == ERR) continue;
        seq_bytes[slen++] = c2;
    }
    nodelay(stdscr, FALSE);

    if (slen <= 1) return 27; /* bare Escape */

    if (slen >= 3 && seq_bytes[1] == '[') {
        if (slen == 3) {
            switch (seq_bytes[2]) {
                case 'A': return KEY_UP;
                case 'B': return KEY_DOWN;
                case 'C': return KEY_RIGHT;
                case 'D': return KEY_LEFT;
                default: break;
            }
        }
        if (slen >= 4 && seq_bytes[slen - 1] == '~') {
            if (seq_bytes[2] == '1') {
                switch (seq_bytes[3]) {
                    case '1': return KEY_F(1);
                    case '2': return KEY_F(2);
                    case '3': return KEY_F(3);
                    case '4': return KEY_F(4);
                    default: break;
                }
            }
        }
    } else if (slen >= 3 && seq_bytes[1] == 'O') {
        switch (seq_bytes[2]) {
            case 'P': return KEY_F(1);
            case 'Q': return KEY_F(2);
            case 'R': return KEY_F(3);
            case 'S': return KEY_F(4);
            default: break;
        }
    }
    return 27; /* unrecognized — treat as quit */
}

/* ------------------------------------------------------------------ */
/* Public: ui_create                                                    */
/* ------------------------------------------------------------------ */

int ui_create(const char *filename)
{
    if (!ui_is_active()) {
        fprintf(stderr, "CREATE requires interactive (ncurses) mode\n");
        return -1;
    }

    CreateState cs;
    create_init(&cs, filename);

    /* Start with one empty field */
    create_add_field(&cs);

    int done = 0;
    while (!done) {
        create_draw(&cs);

        int c = getch();
        if (c == ERR) continue;

        if (c == 27) {
            c = parse_escape_sequence();
        }

        int abs_row = cs.start_row + cs.cur_row;
        int status_row = LINES - 1;

        switch (c) {
            case 27: /* Escape = cancel */
                done = 1;
                break;

            case KEY_UP:
                if (cs.cur_row > 0) {
                    cs.cur_row--;
                } else if (cs.start_row > 0) {
                    cs.start_row--;
                }
                break;

            case KEY_DOWN:
                if (cs.cur_row < cs.data_rows - 1 &&
                    abs_row + 1 < cs.field_count) {
                    cs.cur_row++;
                } else if (abs_row + 1 < cs.field_count) {
                    cs.start_row++;
                    cs.cur_row = 0;
                }
                break;

            case KEY_LEFT:
                if (cs.cur_col > 0) {
                    cs.cur_col--;
                } else if (cs.cur_row > 0) {
                    cs.cur_row--;
                    cs.cur_col = CREATE_NUM_COLS - 1;
                } else if (cs.start_row > 0) {
                    cs.start_row--;
                }
                break;

            case KEY_RIGHT:
                if (cs.cur_col < CREATE_NUM_COLS - 1) {
                    cs.cur_col++;
                } else if (cs.cur_row < cs.data_rows - 1 &&
                           abs_row + 1 < cs.field_count) {
                    cs.cur_row++;
                    cs.cur_col = 0;
                } else if (abs_row + 1 < cs.field_count) {
                    cs.start_row++;
                    cs.cur_row = 0;
                    cs.cur_col = 0;
                }
                break;

            case KEY_PPAGE:
                if (cs.start_row > cs.data_rows)
                    cs.start_row -= cs.data_rows;
                else
                    cs.start_row = 0;
                cs.cur_row = 0;
                break;

            case KEY_NPAGE:
                if (cs.start_row + cs.data_rows * 2 < cs.field_count)
                    cs.start_row += cs.data_rows;
                else
                    cs.start_row = cs.field_count - cs.data_rows;
                if (cs.start_row < 0) cs.start_row = 0;
                cs.cur_row = cs.data_rows - 1;
                break;

            case KEY_HOME:
                cs.start_row = 0;
                cs.cur_row = 0;
                cs.cur_col = 0;
                break;

            case 10: /* LF */
            case 13: { /* CR — Enter to edit or type dropdown */
                if (abs_row >= cs.field_count) break;
                int data_row = CREATE_FIRST_DATA_ROW + cs.cur_row;
                if (data_row >= status_row) break;

                int x = 1; /* after first '|' */

                if (cs.cur_col == CREATE_COL_TYPE) {
                    create_type_dropdown(&cs, data_row, x);
                } else {
                    char edit_buf[64] = "";

                    if (cs.cur_col == CREATE_COL_NAME) {
                        strncpy(edit_buf, cs.field_names + abs_row * 11, 10);
                        edit_buf[10] = '\0';
                        create_edit_text(data_row, x, edit_buf,
                                         CREATE_W_NAME, 10);
                        strncpy(cs.field_names + abs_row * 11,
                                edit_buf, 10);
                        cs.field_names[abs_row * 11 + 10] = '\0';
                    } else if (cs.cur_col == CREATE_COL_WIDTH) {
                        x = 1 + CREATE_W_NAME + 1 + CREATE_W_TYPE + 1;
                        create_edit_number(data_row, x,
                                           &cs.field_widths[abs_row],
                                           CREATE_W_WIDTH, 1, 254);
                    } else if (cs.cur_col == CREATE_COL_DEC) {
                        x = 1 + CREATE_W_NAME + 1 + CREATE_W_TYPE + 1 +
                            CREATE_W_WIDTH + 1;
                        if (cs.field_types[abs_row] == 'N') {
                            create_edit_number(data_row, x,
                                               &cs.field_decimals[abs_row],
                                               CREATE_W_DEC, 0, 15);
                        }
                    }
                }
                break;
            }

            case 'n' ^ 0x40: /* ^N — Ctrl+N: add new field */
                create_add_field(&cs);
                break;

            case 'u' ^ 0x40: { /* ^U — Ctrl+U: remove with confirmation */
                if (abs_row >= cs.field_count) break;
                mvaddstr(status_row, 0,
                    "                                                                                         ");
                mvaddstr(status_row, 0, "Delete field? (Y/N)");
                refresh();
                int ans = getch();
                if (ans == 'y' || ans == 'Y') {
                    create_remove_field(&cs, abs_row);
                }
                break;
            }

            case 236: /* Ctrl+End = 0xEC = 236 on many terminals */
            case 0x14: /* Ctrl+D is sometimes used for Ctrl+End */
            { /* Save and create database */
                int error = 0;
                char err_msg[256] = "";

                if (cs.field_count == 0) {
                    strncpy(err_msg, "Error: At least one field required",
                            sizeof(err_msg) - 1);
                    error = 1;
                }

                for (int i = 0; i < cs.field_count && !error; i++) {
                    if (strlen(cs.field_names + i * 11) == 0) {
                        snprintf(err_msg, sizeof(err_msg),
                                 "Error: Field %d has no name", i + 1);
                        error = 1;
                    }
                    for (int j = i + 1; j < cs.field_count; j++) {
                        if (strcasecmp(cs.field_names + i * 11,
                                       cs.field_names + j * 11) == 0) {
                            snprintf(err_msg, sizeof(err_msg),
                                     "Error: Duplicate field name '%.10s'",
                                     cs.field_names + i * 11);
                            error = 1;
                            break;
                        }
                    }
                }

                if (error) {
                    mvaddstr(status_row, 0,
                        "                                                                                         ");
                    mvaddstr(status_row, 0, err_msg);
                    refresh();
                    usleep(2000000);
                    break;
                }

                /* Force constraints */
                for (int i = 0; i < cs.field_count; i++) {
                    if (cs.field_types[i] == 'L') cs.field_widths[i] = 1;
                    if (cs.field_types[i] != 'N') cs.field_decimals[i] = 0;
                    if (cs.field_widths[i] < 1) cs.field_widths[i] = 1;
                    if (cs.field_widths[i] > 254) cs.field_widths[i] = 254;
                }

                /* Check if file exists */
                char full_path[512];
                snprintf(full_path, sizeof(full_path), "%s.dbf", cs.filename);
                if (access(full_path, F_OK) == 0) {
                    mvaddstr(status_row, 0,
                        "                                                                                         ");
                    mvaddstr(status_row, 0, "File exists. Overwrite? (Y/N)");
                    refresh();
                    int ans = getch();
                    if (ans != 'y' && ans != 'Y') break;
                    unlink(full_path);
                    char dbt_path[512];
                    snprintf(dbt_path, sizeof(dbt_path), "%s.dbt", cs.filename);
                    unlink(dbt_path);
                }

                /* Build DATABASEDBF struct — fields start at index 1 */
                DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
                db->camposn = cs.field_count;

                for (int i = 0; i < cs.field_count; i++) {
                    /* names[char_pos][field_idx] — field_idx is 1-based */
                    for (int cp = 0; cp < 11; cp++) {
                        db->fields.names[cp][i + 1] =
                            cs.field_names[i * 11 + cp];
                    }
                    db->fields.tipos[i + 1] = cs.field_types[i];
                    db->fields.longitudes[i + 1] = cs.field_widths[i];
                    db->fields.decimales[i + 1] = cs.field_decimals[i];
                }

                /* Current date */
                time_t now = time(NULL);
                struct tm *tm_info = localtime(&now);
                int day = tm_info->tm_mday;
                int month = tm_info->tm_mon + 1;
                int year = tm_info->tm_year % 100;

                int rc = create_database(cs.filename, day, month, year, db, 0);
                free(db);

                if (rc != 0) {
                    mvaddstr(status_row, 0,
                        "                                                                                         ");
                    mvaddstr(status_row, 0, "Error: Failed to create database");
                    refresh();
                    usleep(2000000);
                    break;
                }

                mvaddstr(status_row, 0,
                    "                                                                                         ");
                mvaddstr(status_row, 0, "Database created successfully!");
                refresh();
                usleep(1000000);
                done = 1;
                break;
            }

            default:
                break;
        }
    }

    create_free(&cs);

    touchwin(stdscr);
    refresh();
    return 0;
}
