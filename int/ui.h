/*
 * ui.h — ncurses UI layer for dBASE III Plus interpreter
 *
 * Provides screen I/O: @...SAY, @...GET, READ, CLEAR, BROWSE, EDIT, MENU.
 * Only active when ncurses is initialized (interactive mode).
 */

#ifndef _UI_H
#define _UI_H

#include <ncurses.h>
#include <form.h>
#include <menu.h>

/* ------------------------------------------------------------------ */
/* Lifecycle — call once at start/end of interactive session           */
/* ------------------------------------------------------------------ */

/* Initialize ncurses. Returns 0 on success, -1 on failure.            */
/* Must be called before any ui_* function.                            */
int  ui_init(void);

/* Shutdown ncurses and restore terminal.                              */
void ui_shutdown(void);

/* Check if ncurses is currently active.                               */
int  ui_is_active(void);

/* ------------------------------------------------------------------ */
/* @...SAY — positioned output                                        */
/* ------------------------------------------------------------------ */

/* Print string at (row, col). 0-based coordinates.                    */
void ui_say(int row, int col, const char *text);

/* Redraw all pending SAY text onto the active form window.            */
void ui_say_redraw(void);

/* Clear the SAY redraw list.                                          */
void ui_say_clear(void);

/* ------------------------------------------------------------------ */
/* TEXT ... ENDTEXT — block text output                               */
/* ------------------------------------------------------------------ */

/* Redraw all stored TEXT block lines.                                 */
void ui_text_redraw(void);

/* Clear the TEXT block list.                                          */
void ui_text_clear(void);

/* TEXT block tracking (used by executor for TEXT/ENDTEXT) */
typedef struct TextEntry TextEntry;
struct TextEntry {
    int row;
    char text[256];
    struct TextEntry *next;
};
extern TextEntry *text_list;
extern int text_row;

/* ------------------------------------------------------------------ */
/* @...GET — positioned input                                         */
/* ------------------------------------------------------------------ */

typedef struct UiGetField UiGetField;

struct UiGetField {
    FIELD    *field;       /* ncurses FIELD pointer                    */
    int      row;          /* 0-based row                              */
    int      col;          /* 0-based col                              */
    int      field_len;    /* field display width                      */
    char     varname[64];  /* dBASE variable name                      */
    char     picture[64];  /* PICTURE mask (empty = none)              */
    char     range_lo[64]; /* RANGE low bound (empty = none)           */
    char     range_hi[64]; /* RANGE high bound (empty = none)          */
    char     valid_expr[256]; /* VALID expression (empty = none)       */
    char     default_expr[256]; /* DEFAULT expression (empty = none)   */
    int      focus;        /* 1 if FOCUS specified                     */
    int      verified;     /* 1 if field passed validation             */
    UiGetField *next;      /* linked list                              */
};

/* Add a GET field to the pending READ queue.                          */
/* Returns pointer to the field, or NULL on allocation failure.        */
UiGetField *ui_get_add(int row, int col, const char *varname, int field_len);

/* Set PICTURE mask on the most recently added GET field.              */
void ui_get_set_picture(const char *picture);

/* Set RANGE on the most recently added GET field.                     */
void ui_get_set_range(const char *lo, const char *hi);

/* Set VALID expression on the most recently added GET field.          */
void ui_get_set_valid(const char *expr);

/* Set DEFAULT expression on the most recently added GET field.        */
void ui_get_set_default(const char *expr);

/* Mark the most recently added GET field as FOCUS target.             */
void ui_get_set_focus(void);

/* ------------------------------------------------------------------ */
/* READ — process all pending GET fields                              */
/* ------------------------------------------------------------------ */

/* Display all pending GET fields and process user input.              */
/* Populates dBASE variables with the results.                          */
/* Call ui_get_clear() after to free the queue.                        */
void ui_read(void);

/* Free all pending GET fields.                                        */
void ui_get_clear(void);

/* ------------------------------------------------------------------ */
/* CLEAR — screen clearing                                            */
/* ------------------------------------------------------------------ */

/* Clear the screen and move cursor to (0,0).                          */
void ui_clear(void);

/* ------------------------------------------------------------------ */
/* Helpers                                                            */
/* ------------------------------------------------------------------ */

/* Refresh the screen (call after any ui_* output).                     */
void ui_refresh(void);

/* Print a string at the current ncurses cursor position (like ?).      */
void ui_print(const char *text);

/* Advance to next line (for ? newline).                                */
void ui_print_newline(void);

/* ------------------------------------------------------------------ */
/* BROWSE — full-screen grid viewer/editor                            */
/* ------------------------------------------------------------------ */

/* Launch the BROWSE grid for the current work area.                    */
/* fields: NULL = all fields, or comma-separated field names.           */
/* Returns 0 on normal exit (F3/Escape).                               */
int  ui_browse(const char *fields);

#endif /* _UI_H */
