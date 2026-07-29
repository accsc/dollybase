# dBASE III Plus — Command Implementation Audit

**Legend:** ✅ Done | ⚠️ Partial | ❌ Missing | ⏸️ Parked

---

## Scope Modifiers

| Command | Status | Notes |
|---------|--------|-------|
| `ALL` | ✅ | Tokenized, parsed by `parse_scope()` |
| `NEXT n` | ✅ | Tokenized, parsed by `parse_scope()` |
| `RECORD n` | ✅ | Tokenized, parsed by `parse_scope()` |
| `REST` | ✅ | Tokenized, parsed by `parse_scope()` |
| `FOR expr` | ⚠️ | Works in DELETE, RECALL, LIST, LOCATE, REPLACE. Not in DISPLAY. |
| `WHILE expr` | ⚠️ | Works in DELETE, RECALL, LOCATE. Not in REPLACE, LIST, DISPLAY. |
| `FIELDS` | ⚠️ | Tokenized. Works in LIST, BROWSE. Not in DISPLAY, REPLACE. |

---

## File Management

| Command | Status | Notes |
|---------|--------|-------|
| `USE` | ✅ | `exec_use()` |
| `CLOSE` | ✅ | `exec_close()` — closes all or specific area |
| `CREATE` | ❌ | Not in tokenizer |
| `MODIFY STRUCTURE` | ❌ | Not in tokenizer |
| `COPY TO` | ❌ | Not in tokenizer |
| `COPY STRUCTURE TO` | ❌ | Not in tokenizer |
| `RENAME` | ❌ | Not in tokenizer |
| `DELETE FILE` | ❌ | Not in tokenizer |
| `DIR` | ❌ | Not in tokenizer |

---

## Record Operations

| Command | Status | Notes |
|---------|--------|-------|
| `APPEND BLANK` | ✅ | `exec_append()` |
| `APPEND FROM` | ❌ | Not in tokenizer |
| `INSERT` | ❌ | Tokenized but not dispatched |
| `EDIT` | ⏸️ | Parked |
| `BROWSE` | ✅ | Full grid with edit, F2/F3/F4, FIELDS support |
| `REPLACE` | ⚠️ | Has ALL/FOR/WHILE/NEXT/RECORD/REST scope. Single field only. |
| `DELETE` | ✅ | Has ALL/FOR/WHILE/NEXT/RECORD/REST scope |
| `RECALL` | ✅ | Has ALL/FOR/WHILE/NEXT/RECORD/REST scope |
| `PACK` | ✅ | `exec_pack()` + `wa_pack()` close/reopen |
| `ZAP` | ⚠️ | Calls `wa_pack()` instead of `delete_all()` + `pack()` |

---

## Navigation

| Command | Status | Notes |
|---------|--------|-------|
| `GO n` | ✅ | `exec_go()` |
| `GOTO n` | ❌ | Tokenized but not dispatched (GO n works) |
| `GO TOP` | ✅ | `KW_GOTOP` |
| `GO BOTTOM` | ✅ | `KW_GOBOTTOM` |
| `SKIP` | ✅ | `exec_skip()` with n, negative |
| `LOCATE FOR` | ✅ | `exec_locate()` with FOR, WHILE, per-area state |
| `CONTINUE` | ✅ | `exec_continue()` per-area |
| `SEEK` | ✅ | `exec_seek()` |
| `FIND` | ❌ | Not in tokenizer |

---

## Display Commands

| Command | Status | Notes |
|---------|--------|-------|
| `LIST` | ✅ | Has ALL/NEXT/RECORD/REST/FOR/WHILE/FIELDS + pagination |
| `DISPLAY STRUCTURE` | ✅ | ncurses-aware |
| `DISPLAY STATUS` | ⚠️ | Token skipped, falls through to STRUCTURE |
| `DISPLAY` (records) | ❌ | Only STRUCTURE works, not record data display |
| `DISPLAY MEMORY` | ❌ | Not implemented |
| `BROWSE` | ✅ | Full grid (see above) |

---

## Programming

| Command | Status | Notes |
|---------|--------|-------|
| `DO program` | ✅ | `exec_do_call()` |
| `DO WHILE` | ✅ | `exec_do_while()` |
| `IF/ELSE/ENDIF` | ✅ | `exec_if()` |
| `FOR/ENDFOR` | ✅ | `exec_for()` with STEP |
| `LOOP` | ✅ | |
| `EXIT` | ✅ | |
| `RETURN` | ✅ | `exec_return()` |
| `PROCEDURE/PARAMETERS` | ✅ | Tokenized, dispatched |
| `DO CASE/ENDCASE` | ❌ | CASE tokenized but not dispatched |
| `QUIT` | ✅ | |
| `CANCEL` | ✅ | |

---

## Screen I/O

| Command | Status | Notes |
|---------|--------|-------|
| `?` / `??` | ✅ | ncurses-aware with scrolling |
| `@...SAY` | ✅ | ncurses positioned output |
| `@...GET` | ✅ | ncurses libform input |
| `READ` | ✅ | Processes GET queue |
| `CLEAR` | ✅ | ncurses screen clear |
| `PAUSE` | ✅ | Blocks at cursor position |

---

## Memory Variables

| Command | Status | Notes |
|---------|--------|-------|
| `=` (assignment) | ✅ | |
| `STORE` | ❌ | Not in tokenizer |
| `PRIVATE` | ❌ | Tokenized but not dispatched |
| `PUBLIC` | ❌ | Tokenized but not dispatched |
| `RELEASE` | ❌ | Not in tokenizer |
| `ACCEPT` | ❌ | Not in tokenizer |
| `INPUT` | ❌ | Not in tokenizer |
| `WAIT` | ❌ | Not in tokenizer (PAUSE exists as alternative) |
| `CLEAR MEMORY` | ❌ | Not implemented |
| `LIST MEMORY` | ❌ | Not implemented |
| `DISPLAY MEMORY` | ❌ | Not implemented |
| `ARRAY`/`DIMENSION` | ❌ | Tokenized but not dispatched |

---

## SET Commands

| Command | Status | Notes |
|---------|--------|-------|
| `SET TALK` | ⚠️ | Parsed but stub |
| `SET EXACT` | ⚠️ | Parsed but stub |
| `SET DATE` | ⚠️ | Parsed but stub |
| `SET CENTURY` | ⚠️ | Parsed but stub |
| `SET DELETED` | ⚠️ | Parsed but stub |
| `SET INDEX` | ✅ | `wa_set_index()` |
| `SET ORDER` | ❌ | Not implemented |
| `SET DEFAULT` | ⚠️ | Parsed but stub |
| `SET PATH` | ⚠️ | Parsed but stub |
| `SET CONSOLE` | ⚠️ | Parsed but stub |
| `SET PRINTER` | ⚠️ | Parsed but stub |
| `SET SAFETY` | ⚠️ | Parsed but stub |
| `SET CONFIRM` | ⚠️ | Parsed but stub |
| `SET FILTER` | ❌ | Not implemented |
| `SET ECHO` | ❌ | Not implemented |

---

## Indexing

| Command | Status | Notes |
|---------|--------|-------|
| `INDEX ON` | ⚠️ | `exec_index()` exists, creates NDX |
| `SET INDEX TO` | ✅ | |
| `SET ORDER TO` | ❌ | Not implemented |
| `REINDEX` | ❌ | Not in tokenizer |

---

## Aggregate / Relational

| Command | Status | Notes |
|---------|--------|-------|
| `COUNT` | ❌ | Not in tokenizer |
| `SUM` | ❌ | Tokenized but not dispatched (SUM function exists) |
| `AVERAGE` | ❌ | Not in tokenizer |
| `TOTAL` | ❌ | Not in tokenizer |
| `JOIN` | ❌ | Tokenized but not dispatched |
| `SELECT` | ✅ | `exec_select()` work area selection |
| `SET RELATION` | ❌ | Not implemented |
| `LABEL` | ❌ | Tokenized but not dispatched |
| `REPORT` | ❌ | Not in tokenizer |
| `SORT` | ❌ | Not in tokenizer |
| `UPDATE` | ❌ | Tokenized but not dispatched |
| `FORMAT` | ❌ | Not in tokenizer |

---

## Functions (in parser.c dispatch)

All implemented as expression built-ins: `ABS`, `ALIAS`, `ALLTRIM`, `ASC`, `AT`, `ATC`, `BETWEEN`, `BOF`, `CHR`, `CTOD`, `DATE`, `DAY`, `DTOC`, `EMPTY`, `EOF`, `FOUND`, `IIF`, `INT`, `LEFT`, `LEN`, `LOWER`, `LTRIM`, `MAX`, `MIN`, `MONTH`, `RECNO`, `RECN`, `RIGHT`, `ROUND`, `RTRIM`, `SIGN`, `SQRT`, `SUBSTR`, `TRIM`, `TYPE`, `UPPER`, `VAL`, `YEAR`, `DELETED`, `SPACE`

---

## Summary

| Category | ✅ Done | ⚠️ Partial | ❌ Missing | ⏸️ Parked |
|----------|---------|------------|------------|-----------|
| Scope Modifiers | 4 | 3 | 0 | 0 |
| File Management | 2 | 0 | 7 | 0 |
| Record Operations | 5 | 2 | 2 | 1 |
| Navigation | 7 | 0 | 2 | 0 |
| Display | 3 | 1 | 2 | 0 |
| Programming | 9 | 0 | 1 | 0 |
| Screen I/O | 6 | 0 | 0 | 0 |
| Memory Variables | 1 | 0 | 8 | 0 |
| SET Commands | 1 | 11 | 2 | 0 |
| Indexing | 1 | 1 | 2 | 0 |
| Aggregate/Relational | 1 | 0 | 10 | 0 |
| **Total** | **40** | **18** | **34** | **1** |
