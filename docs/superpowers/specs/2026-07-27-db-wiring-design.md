# dBase III PRG Interpreter — Database Wiring Design

## Overview

Wire the PRG interpreter (`int/`) to `libdollybase` (`libdbase_4/`) so that
database commands and functions in `.prg` programs actually operate on real
DBF files instead of being no-op stubs.

## Scope

**In scope (core only):**
- `USE <file>` / `CLOSE DATABASES` / `CLOSE ALL`
- `SELECT <n>` — work area selection
- `SKIP [n]` (positive and negative)
- `GO TOP` / `GO BOTTOM` / `GO <n>`
- `DELETE` / `DELETE ALL` / `RECALL` / `RECALL ALL` / `PACK` / `ZAP`
- `REPLACE <field> WITH <expr>`
- `APPEND BLANK`
- `RECNO()` / `RECN()` / `RECCOUNT()` / `EOF()` / `BOF()` / `DELETED()` / `ALIAS()`
- Field access via `->` operator: `TABLE->FIELD`

**Out of scope (future):**
- `SET INDEX TO` / `SEEK` / index-aware navigation
- `LOCATE FOR` / `CONTINUE`
- `SET RELATION` / `SET ORDER`
- Multi-user locking (`dbf_lock`, `rec_lock`)
- Memo fields (DBT/FPT)
- Label files
- Export commands (CSV, SQL)

## Architecture

```
executor.c / parser.c
        ↓ calls
workarea.h / workarea.c   (NEW — bridge layer)
        ↓ wraps
libdbase.h / libdbase_0.4_s.a   (existing)
```

A new `workarea.c` / `workarea.h` module provides a clean API between the
interpreter and libdollybase. It manages:

1. **Work area registry** — up to 10 work areas (indices 0–9), each holding
   a `DATABASEDBF*` pointer. Maps to dBase `SELECT 1` through `SELECT 10`.
2. **Selected work area** — a global index (default 0). All database
   operations route through the selected work area.
3. **Convenience functions** — the executor and parser call these instead of
   libdollybase directly.

## Work Area API

### Lifecycle

```c
void wa_init(void);       // Reset registry. Call once at startup.
void wa_shutdown(void);   // Close all DBFs, free handles.
```

### Work area management

```c
int  wa_select(int area);       // SELECT n (1-based). Returns 0 on success.
int  wa_get_selected(void);     // Current work area index (0-based).
DATABASEDBF *wa_db(void);       // Pointer to current DBF (NULL if none open).
DATABASEDBF **wa_db_ptr(void);  // Double pointer for skip()/gotos().
```

### USE / CLOSE

```c
int  wa_use(const char *filename, int area);  // USE <file> in area (or next free if area < 0).
void wa_close(int area);                      // CLOSE area n.
void wa_close_all(void);                      // CLOSE DATABASES / CLOSE ALL.
```

### Navigation

```c
void wa_skip(int n);         // SKIP [n]. Negative n uses gotos(current - n).
int  wa_goto(int rec);       // GO <n>. Returns 0 on success.
void wa_goto_top(void);      // GO TOP.
void wa_goto_bottom(void);   // GO BOTTOM.
```

### Status

```c
int  wa_recno(void);         // RECNO()
int  wa_reccount(void);      // RECN() / RECCOUNT()
int  wa_eof(void);           // EOF() — returns 1 if past last record.
int  wa_bof(void);           // BOF() — returns 1 if before first record.
int  wa_is_deleted(void);    // DELETED() — returns 1 if current record is deleted.
```

### CRUD

```c
int  wa_delete(void);         // DELETE — mark current record deleted.
int  wa_delete_all(void);     // DELETE ALL — mark all records deleted.
int  wa_recall(void);         // RECALL — undelete current record.
int  wa_recall_all(void);     // RECALL ALL — undelete all deleted records.
int  wa_pack(void);           // PACK — permanently remove deleted records.
int  wa_zap(void);            // ZAP — same as PACK.
int  wa_append_blank(void);   // APPEND BLANK — append empty record.
```

### Field access

```c
int  wa_field_count(void);                    // Number of fields.
int  wa_field_to_number(const char *name);    // Field index by name (1-based, 0 if not found).
char *wa_field_name(int idx);                 // Field name by index (1-based). Caller frees.
char wa_field_type(int idx);                  // Field type char ('C','N','D','L','M').
char *wa_get_field(int idx);                  // Get field value as string (1-based). Caller frees.
int  wa_replace(const char *fieldname, const char *value); // REPLACE field WITH value.
```

### Utility

```c
char *wa_dbf_name(void);  // ALIAS() / DBF() — returns the DBF filename. Caller frees.
```

## Integration Points

### executor.c — Statement handlers

| Command | Handler | Implementation |
|---------|---------|----------------|
| `USE <file>` | `exec_use` | Parse filename (IDENT or STRING), strip extension if present, call `wa_use(filename, -1)` |
| `SKIP [n]` | `exec_skip` | Parse optional numeric expression, call `wa_skip(n)` (default 1) |
| `GO TOP` | `exec_go` | Call `wa_goto_top()` |
| `GO BOTTOM` | `exec_go` | Call `wa_goto_bottom()` |
| `GO <n>` | `exec_go` | Parse expression, call `wa_goto(n)` |
| `CLOSE DATABASES` | `exec_close` | Call `wa_close_all()` |
| `CLOSE ALL` | `exec_close` | Call `wa_close_all()` |
| `DELETE` | NEW `exec_delete` | Call `wa_delete()` |
| `RECALL` | NEW `exec_recall` | Call `wa_recall()` |
| `PACK` | NEW `exec_pack` | Call `wa_pack()` |
| `ZAP` | NEW `exec_zap` | Call `wa_zap()` |
| `REPLACE f WITH e` | NEW `exec_replace` | Parse field name, evaluate expression, call `wa_replace()` |
| `APPEND BLANK` | NEW `exec_append` | Call `wa_append_blank()` |
| `SELECT <n>` | NEW `exec_select` | Parse area number, call `wa_select()` |

New keyword dispatch entries in `exec_statement`:
- `KW_DELETE` → `exec_delete`
- `KW_RECALL` → `exec_recall`
- `KW_PACK` → `exec_pack`
- `KW_ZAP` → `exec_zap`
- `KW_REPLACE` → `exec_replace`
- `KW_APPEND` → `exec_append`

### parser.c — Built-in functions

| Function | Implementation |
|----------|----------------|
| `RECNO()` | `val_integer(wa_recno())` |
| `RECN()` / `RECCOUNT()` | `val_integer(wa_reccount())` |
| `EOF()` | `val_logical(wa_eof())` |
| `BOF()` | `val_logical(wa_bof())` |
| `DELETED()` | NEW — `val_logical(wa_is_deleted())` |
| `ALIAS()` | NEW — `val_string(wa_dbf_name())` |

### parser.c — Field access via `->`

In `parse_primary`, after resolving an identifier, check if the next token
is `TOK_ARROW`. If so, consume the arrow and the field name token, then:
1. Look up the identifier as a DBF alias (check work area registry)
2. Look up the field name in that DBF
3. Return the field value as an `ExprValue`

Type coercion:
- Field type `N` → parse as number (`val_real` or `val_integer`)
- Field type `D` → `val_date`
- Field type `L` → `val_logical`
- Field type `C` → `val_string`
- Field type `M` → `val_string` (memo, out of scope for reading)

### prg.c — Initialization

```c
vars_init();
wa_init();       // NEW
// ... tokenize, scan, execute ...
wa_shutdown();   // NEW
vars_shutdown();
```

### Build

Link against `libdbase_4/.libs/libdbase_0.4_s.a`:

```
gcc -w -o prg prg.c tokenizer.c parser.c executor.c exprvalue.c variables.c workarea.c \
    ../libdbase_4/.libs/libdbase_0.4_s.a -lm
```

## Error Handling

If no DBF is open in the selected work area and a database command fires:
- Print a warning to stderr: `prg: no database open in selected work area`
- Return `EXEC_OK` (dBase III behavior — silent no-op)

If `USE` fails to open a file:
- Print error to stderr with `%m` (errno)
- Return `EXEC_OK` (the program continues)

## Testing

- Unit tests for `workarea.c` API (open/close, navigation, field access)
- Integration tests via `run()` with small DBF test fixtures
- Stress test `.prg` file that exercises USE, SKIP, GO, field access, REPLACE
