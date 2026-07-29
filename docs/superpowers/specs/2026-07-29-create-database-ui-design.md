# CREATE DATABASE UI Design

**Date:** 2026-07-29
**Status:** Approved
**Approach:** New isolated file (`int/create_ui.c/h`), following BROWSE pattern

---

## Overview

Implement a `CREATE file.dbf` command in the dBASE III PRG interpreter that launches a full-screen ncurses UI for interactively defining database fields. The UI presents a grid with columns for Field Name, Type, Width, and Decimals. On save, the UI populates a `DATABASEDBF` struct and calls the existing library function `create_database()`.

## Architecture

```
CREATE file.dbf
    │
    ▼
exec_create() parses filename → calls ui_create(filename)
    │
    ▼
ui_create() launches ncurses full-screen grid:
    │
    ├── Header: "CREATE DATABASE: <filename>"
    ├── Columns: [Field name] [Type] [Width] [Dec]
    ├── Data rows: one per field definition (starts empty)
    ├── Status bar: help keys + field count
    │
    ├── Navigation: arrows, PgUp/PgDn, Home, End
    ├── Editing: Enter to edit cell, Escape to cancel edit
    ├── Type column: Enter/Space → inline dropdown (C/N/D/L/M)
    ├── ^N: append new field row at bottom
    ├── ^U: remove current row (with Y/N confirmation)
    ├── ^End: validate + save → call create_database()
    └── Esc: cancel (discard, exit UI)
    │
    ▼
On ^End: populate DATABASEDBF struct → create_database() → .dbf created
```

## Grid Layout

```
CREATE DATABASE: mydata.dbf
+------------+------+------+---+
| Field name | Type | Width|Dec|
+------------+------+------+---+
| NAME       | C    |    20|  0|
| AGE        | N    |     3|  0|
| BIRTHDATE  | D    |     8|  0|
| ACTIVE     | L    |     1|  0|
| NOTES      | M    |    10|  0|
|            |      |      |   |
|            |      |      |   |
+--------------------------------+
Fields: 5  Esc:Cancel  ^End:Save  ^N:Add  ^U:Del  Arrows:Nav  Enter:Edit
```

- Column separators use `|`, `+`, `-` (ASCII, terminal-compatible)
- Current cell highlighted with `A_REVERSE`
- Width/Dec columns are right-aligned numbers
- Dec column forced to 0 for non-N types
- Empty rows at bottom for visual space

## Data Model

```c
typedef struct {
    char        filename[256];     /* target .dbf filename             */
    int         field_count;       /* number of defined fields           */
    int         max_fields;        /* allocated capacity (start 10)      */
    char        *field_names;      /* [max_fields][11] dBASE field names */
    char        *field_types;      /* [max_fields] C/N/D/L/M             */
    int         *field_widths;     /* [max_fields]                       */
    int         *field_decimals;   /* [max_fields]                       */
    int         cur_row;           /* current data row (0-based)         */
    int         cur_col;           /* 0=Name, 1=Type, 2=Width, 3=Dec    */
    int         data_rows;         /* visible data rows on screen        */
    int         start_row;         /* first visible row for scrolling    */
} CreateState;
```

Fixed 4 columns (no horizontal scrolling):
- Column 0: `Field name` (width ~16, editable text, max 10 chars dBASE limit)
- Column 1: `Type` (width ~6, dropdown C/N/D/L/M via inline popup)
- Column 2: `Width` (width ~7, editable integer 1-254)
- Column 3: `Dec` (width ~4, editable integer 0-15, only for N type)

## Key Bindings

| Key | Action |
|---|---|
| Arrows | Navigate between cells |
| PgUp/PgDn | Scroll page up/down |
| Home | Go to first field |
| End | Go to last field |
| Enter (on Name/Width/Dec) | Edit cell inline (line editor, Escape=cancel) |
| Enter/Space (on Type) | Show inline dropdown: C/N/D/L/M, arrows+Enter to pick |
| ^N (Ctrl+N) | Append new field row at bottom, auto-name (F1, F2...), defaults C/10/0 |
| ^U (Ctrl+U) | Remove current row → status bar: "Delete field? (Y/N)" → Y confirms |
| ^End (Ctrl+End) | Validate → if OK, call `create_database()` → exit UI |
| Escape | Cancel → exit UI without saving |

## Validation on Save (^End)

- No empty field names
- No duplicate field names (case-insensitive)
- Width >= 1 for all types; Width forced to 1 for L type
- Dec = 0 for non-N types (forced automatically)
- At least 1 field required
- Field names <= 10 chars (dBASE limit)

On validation failure: show specific error on status bar, keep user in UI to fix.

## Type Dropdown

When Enter/Space is pressed in the Type column:
- Draw a small inline popup at the current cell position showing:
  ```
  > C - Character
    N - Numeric
    D - Date
    L - Logical
    M - Memo
  ```
- Arrow keys navigate the options, Enter confirms selection
- Current selection shown with `>` prefix and `A_REVERSE`
- Popup disappears on selection, grid redraws with new type

## Integration Points

### Tokenizer (`int/tokenizer.h` + `int/tokenizer.c`)
- Add `KW_CREATE` to `KeywordId` enum (database commands section, after `KW_CLOSE`)
- Add `{"CREATE", KW_CREATE}` to `KEYWORDS[]` array (alphabetically after `CLEAR MEMORY`)

### Executor (`int/executor.c`)
- Add `static ExecStatus exec_create(Token **cur);` declaration
- Add `case KW_CREATE: return exec_create(cur);` in dispatch switch
- `exec_create()` parses `CREATE filename.dbf`, strips `.dbf` if present, calls `ui_create(filename)`

### UI Header (`int/ui.h`)
- Add `int ui_create(const char *filename);` declaration

### New Files
- `int/create_ui.c` — full implementation (~400 lines): `CreateState`, `create_draw()`, `create_edit_field()`, type dropdown, `ui_create()` main loop

### Makefile
- Add `create_ui.o` to the interpreter object list

### Library
- **No changes** to `libdbase_4/` — `create_database()` API is already correct

## Error Handling

- If `create_database()` fails (disk full, permission denied): show error on status bar, stay in UI
- If filename already exists: show "File exists. Overwrite? (Y/N)" on status bar
- If ncurses not active: show error message via `ui_print()`, return `EXEC_OK`

## Testing Plan

1. Create empty database (1 field, save, verify .dbf exists with correct structure)
2. Create database with multiple fields of each type (C/N/D/L/M)
3. Test ^N adds fields, ^U removes with confirmation
4. Test validation: empty name, duplicate name, zero width
5. Test navigation: arrows wrap, PgUp/PgDn scroll
6. Test Type dropdown: all 5 types selectable
7. Test Escape cancels without creating file
8. Test overwrite confirmation for existing file
