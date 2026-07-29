# PRG Interpreter Bug Fixes

All fixes applied to the dBase III .PRG interpreter in `int/`. Test suite: **99/99 passing**.

---

## Executor Fixes (`int/executor.c`)

### 1. Multi-line loop/block bodies stopped at EOL

**Functions affected:** `exec_do_body`, `exec_for_body`, `exec_block_until`, `skip_block_nested`

**Problem:** All four functions used `!is_eol_or_eof(*cur)` as their loop condition. After executing the first line of a multi-line body (e.g., `x = x + 1`), the cursor hit EOL and the loop exited. Subsequent lines (`IF x >= 5 / EXIT / ENDIF`) were never reached, causing infinite loops.

**Fix:** Changed loop condition to `(*cur)->type != TOK_EOF` and added explicit EOL skipping at the top of each loop iteration:
```c
if ((*cur)->type == TOK_EOL) { *cur = (*cur)->next; continue; }
```

### 2. EXIT/LOOP not handled in exec_statement

**Problem:** `KW_EXIT` and `KW_LOOP` were not cases in the `exec_statement` switch. They fell through to `default`, which broke out and called `skip_to_eol`, silently ignoring the keyword. This meant EXIT inside an IF body (nested inside a loop) never propagated.

**Fix:** Added cases to the switch:
```c
case KW_EXIT:   (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_EXIT;
case KW_LOOP:   (*cur) = (*cur)->next; skip_to_eol(cur); return EXEC_LOOP;
```

### 3. EXEC_EXIT/EXEC_LOOP not propagated from exec_block_until

**Problem:** `exec_block_until` only checked for `EXEC_RETURN` and `EXEC_CANCEL` from `exec_statement`. `EXEC_EXIT` and `EXEC_LOOP` were silently swallowed.

**Fix:** Extended the check:
```c
if (st == EXEC_RETURN || st == EXEC_CANCEL || st == EXEC_EXIT || st == EXEC_LOOP)
    return st;
```

### 4. EXEC_EXIT cursor left mid-body

**Problem:** When EXIT fired inside a nested IF within a FOR/DO WHILE loop, `exec_for`/`exec_do_while` returned immediately. The cursor was left pointing at unexecuted statements after the IF block. `execute_tokens` then continued from that position and executed those leftover statements.

**Example:** `FOR ... / IF i > 3 / EXIT / ENDIF / s = s + i / NEXT` — after EXIT at i=4, cursor was at `s = s + i`, which `execute_tokens` then executed.

**Fix:** In both `exec_for` and `exec_do_while`, when `EXEC_EXIT` is received from the body executor, skip the rest of the body to the loop terminator (NEXT/ENDDO) before returning `EXEC_OK`:
```c
if (st == EXEC_EXIT) {
    while (*cur && (*cur)->type != TOK_EOF &&
           !((*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT)) {
        if ((*cur)->type == TOK_EOL) { *cur = (*cur)->next; continue; }
        *cur = (*cur)->next;
    }
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_NEXT)
        *cur = (*cur)->next;
    return EXEC_OK;
}
```

### 5. IF/ELSE executed both branches

**Problem:** When the IF condition was TRUE, `exec_block_until(cur, KW_ENDIF, KW_ELSE)` executed the THEN body and stopped at ELSE (without consuming it). `exec_if` returned `EXEC_OK`, leaving the cursor at ELSE. The caller then saw ELSE and — in the false-branch path of a subsequent IF or in `execute_tokens` — executed the ELSE body too.

**Fix:** After the truthy `exec_block_until` returns, check if cursor is at ELSE. If so, skip the ELSE body to ENDIF:
```c
if (truthy) {
    ExecStatus st = exec_block_until(cur, KW_ENDIF, KW_ELSE);
    if (st != EXEC_OK) return st;
    if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ELSE) {
        skip_block_nested(cur, KW_ENDIF, 0);
        if (*cur && (*cur)->type == TOK_KEYWORD && (*cur)->keyword_id == KW_ENDIF)
            *cur = (*cur)->next;
    }
    return EXEC_OK;
}
```

---

## Tokenizer Fix (`int/tokenizer.c`)

### 6. RETURN missing from keyword table

**Problem:** `KW_RETURN` was defined in the `KeywordId` enum (`tokenizer.h:65`) but had no entry in the `KEYWORDS[]` lookup table. The word `RETURN` was tokenized as `TOK_IDENT` instead of `TOK_KEYWORD`, so `exec_statement` never dispatched it.

**Fix:** Added entry to `KEYWORDS[]` (alphabetically between REPLACE and RIGHT):
```c
{"RETURN", KW_RETURN},
```

---

## Test Suite Fix (`int/test_prg.c`)

### 7. Variable store not isolated between executor tests

**Problem:** The global variable store persisted across all tests. Tests like `exec_if_false` (expecting `x` to be NULL) and `exec_complex_program` (expecting clean `s` and `result`) failed because prior tests had set those variables.

**Fix:** Added `vars_shutdown(); vars_init();` before the executor test section and inside `test_exec_if_false` and `test_exec_complex_program`.

---

## Summary of Changes

| File | Lines Changed | Description |
|------|--------------|-------------|
| `int/executor.c` | ~60 | EOL skipping in 4 functions, EXIT/LOOP in exec_statement, propagation in exec_block_until, cursor cleanup on EXIT, IF/ELSE branch fix |
| `int/tokenizer.c` | 1 | Added RETURN to keyword table |
| `int/test_prg.c` | 3 | Variable store isolation in 3 places |
