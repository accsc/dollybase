* ============================================================================
* stress_memo.prg — Stress test for DBT (memo field) operations
* ============================================================================
* Tests: USE with DBT, DISPLAY STRUCTURE, GO TOP/BOTTOM, SKIP,
*        TABLE->COMMENTS read, DELETE + PACK with DBT, EOF/BOF checks
* Run from int/ directory: ./prg < stress/stress_memo.prg
* ============================================================================

* --- Test 1: Open database with memo field ---
USE books_memo
? "Test 1: USE books_memo.dbf - OK"

* --- Test 2: DISPLAY STRUCTURE ---
DISPLAY STRUCTURE
? "Test 2: DISPLAY STRUCTURE - OK"

* --- Test 3: GO TOP and read first record memo ---
GO TOP
memo_val = TABLE->COMMENTS
? "Test 3: First record memo =", SUBSTR(memo_val, 1, 50)

* --- Test 4: GO BOTTOM and read last record memo ---
GO BOTTOM
memo_val = TABLE->COMMENTS
? "Test 4: Last record memo =", SUBSTR(memo_val, 1, 50)

* --- Test 5: SKIP forward and read memo ---
GO TOP
SKIP
SKIP
memo_val = TABLE->COMMENTS
? "Test 5: Record 3 memo =", SUBSTR(memo_val, 1, 50)

* --- Test 6: Navigate and verify EOF ---
GO BOTTOM
SKIP
if EOF()
    ? "Test 6: EOF after GO BOTTOM + SKIP - OK"
else
    ? "Test 6: FAIL - EOF should be true after SKIP past bottom"
endif

* --- Test 7: Not EOF at top ---
GO TOP
if .NOT. EOF()
    ? "Test 7: Not EOF at GO TOP - OK"
else
    ? "Test 7: FAIL - should not be EOF at top"
endif

* --- Test 8: Read all memo fields in a loop ---
GO TOP
rec_count = 0
DO WHILE .NOT. EOF()
    memo_val = TABLE->COMMENTS
    rec_count = rec_count + 1
    SKIP
ENDDO
? "Test 8: Read", rec_count, "memo records - OK"

* --- Test 9: DELETE records with memo fields ---
GO TOP
DELETE
DELETE
DELETE
? "Test 9: Deleted 3 records"

* --- Test 10: PACK database with DBT file ---
PACK
? "Test 10: PACK completed - OK"

* --- Test 11: Verify records after PACK ---
GO TOP
rec_count = 0
DO WHILE .NOT. EOF()
    rec_count = rec_count + 1
    SKIP
ENDDO
? "Test 11: Records after PACK =", rec_count

* --- Test 12: Read memo after PACK ---
GO TOP
memo_val = TABLE->COMMENTS
? "Test 12: Memo after PACK =", SUBSTR(memo_val, 1, 50)

* --- Test 13: Multiple SKIP operations ---
GO TOP
SKIP 5
SKIP 5
memo_val = TABLE->COMMENTS
? "Test 13: Memo at record ~11 =", SUBSTR(memo_val, 1, 50)

* --- Test 14: IIF with memo field ---
GO TOP
memo_len = LEN(TABLE->COMMENTS)
result = IIF(memo_len > 100, "LONG", "SHORT")
? "Test 14: Memo length check =", result

* --- Test 15: SUBSTR on memo field ---
GO TOP
short_memo = SUBSTR(TABLE->COMMENTS, 1, 30)
? "Test 15: SUBSTR of memo =", short_memo

* --- Test 16: LEN on memo field ---
GO TOP
memo_len = LEN(TABLE->COMMENTS)
? "Test 16: LEN of memo =", memo_len

* --- Test 17: AT function on memo field ---
GO TOP
pos = AT("obra", TABLE->COMMENTS)
? "Test 17: AT obra in memo =", pos

* --- Test 18: UPPER on memo field ---
GO TOP
upper_memo = UPPER(TABLE->COMMENTS)
? "Test 18: UPPER of memo =", SUBSTR(upper_memo, 1, 30)

* --- Test 19: DO WHILE loop with memo access ---
GO TOP
count_long = 0
i = 1
DO WHILE i <= 10
    IF .NOT. EOF()
        memo_len = LEN(TABLE->COMMENTS)
        IF memo_len > 200
            count_long = count_long + 1
        ENDIF
        SKIP
    ENDIF
    i = i + 1
ENDDO
? "Test 19: Long memos in first 10 =", count_long

* --- Test 20: DO WHILE with memo ---
GO TOP
total_len = 0
cnt = 0
DO WHILE .NOT. EOF() AND cnt < 5
    memo_len = LEN(TABLE->COMMENTS)
    total_len = total_len + memo_len
    cnt = cnt + 1
    SKIP
ENDDO
? "Test 20: Total memo length first 5 =", total_len

? ""
? "=== All memo stress tests completed ==="
