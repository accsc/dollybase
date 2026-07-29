* stress_replace_multi.prg — Test REPLACE with multiple comma-separated fields
FAILS = 0

USE books

* --- Test 1: Single field REPLACE (baseline) ---
GO TOP
REPLACE TITULO WITH "SINGLE_FIELD_TEST"
GO TOP
IF TITULO = "SINGLE_FIELD_TEST"
    ? "PASS: Test 1 - Single field REPLACE"
ELSE
    ? "FAIL: Test 1 - Single field REPLACE"
    FAILS = FAILS + 1
ENDIF

* --- Test 2: Two fields in one REPLACE ---
REPLACE TITULO WITH "TWO_T", AP1_AU WITH "TWO_A"
GO TOP
IF TITULO = "TWO_T" .AND. AP1_AU = "TWO_A"
    ? "PASS: Test 2 - Two field REPLACE"
ELSE
    ? "FAIL: Test 2 - Got TITULO='", TITULO, "' AP1_AU='", AP1_AU, "'"
    FAILS = FAILS + 1
ENDIF

* --- Test 3: Three fields in one REPLACE ---
REPLACE TITULO WITH "THREE_T", AP1_AU WITH "THREE_A", AP2_AU WITH "THREE_B"
GO TOP
IF TITULO = "THREE_T" .AND. AP1_AU = "THREE_A" .AND. AP2_AU = "THREE_B"
    ? "PASS: Test 3 - Three field REPLACE"
ELSE
    ? "FAIL: Test 3 - Got TITULO='", TITULO, "' AP1_AU='", AP1_AU, "' AP2_AU='", AP2_AU, "'"
    FAILS = FAILS + 1
ENDIF

* --- Test 4: REPLACE ALL with multiple fields ---
REPLACE ALL TITULO WITH "ALL_T", AP1_AU WITH "ALL_A"
GO TOP
t1 = TITULO
a1 = AP1_AU
GO BOTTOM
t2 = TITULO
a2 = AP1_AU
IF t1 = "ALL_T" .AND. a1 = "ALL_A" .AND. t2 = "ALL_T" .AND. a2 = "ALL_A"
    ? "PASS: Test 4 - REPLACE ALL multi-field"
ELSE
    ? "FAIL: Test 4 - REPLACE ALL multi-field"
    FAILS = FAILS + 1
ENDIF

* --- Test 5: REPLACE with expression values ---
GO TOP
REPLACE TITULO WITH "EXP_" + LEFT(TITULO, 3), AP1_AU WITH LOWER(AP1_AU)
GO TOP
IF LEFT(TITULO, 4) = "EXP_"
    ? "PASS: Test 5 - REPLACE with expressions"
ELSE
    ? "FAIL: Test 5 - Got TITULO='", TITULO, "'"
    FAILS = FAILS + 1
ENDIF

* --- Test 6: REPLACE FOR with multiple fields ---
REPLACE TITULO WITH "FOR_T", AP1_AU WITH "FOR_A" FOR RECNO() <= 3
GO TOP
cnt = 0
DO WHILE .NOT. EOF() .AND. RECNO() <= 3
    IF TITULO = "FOR_T" .AND. AP1_AU = "FOR_A"
        cnt = cnt + 1
    ENDIF
    SKIP
ENDDO
IF cnt = 3
    ? "PASS: Test 6 - REPLACE FOR multi-field (", cnt, "/3)"
ELSE
    ? "FAIL: Test 6 - REPLACE FOR multi-field (", cnt, "/3)"
    FAILS = FAILS + 1
ENDIF

* --- Test 7: REPLACE NEXT with multiple fields ---
GO TOP
REPLACE NEXT 2 TITULO WITH "NEXT_T", AP1_AU WITH "NEXT_A"
* NEXT 2 replaces current (rec 1) and next (rec 2)
GO TOP
t1 = TITULO
a1 = AP1_AU
SKIP
t2 = TITULO
a2 = AP1_AU
IF t1 = "NEXT_T" .AND. a1 = "NEXT_A" .AND. t2 = "NEXT_T" .AND. a2 = "NEXT_A"
    ? "PASS: Test 7 - REPLACE NEXT multi-field"
ELSE
    ? "FAIL: Test 7 - REPLACE NEXT multi-field"
    FAILS = FAILS + 1
ENDIF

* --- Test 8: REPLACE FOR..WHILE with multiple fields ---
REPLACE TITULO WITH "FW_T", AP1_AU WITH "FW_A" FOR RECNO() >= 1 WHILE RECNO() <= 4
GO TOP
cnt = 0
DO WHILE .NOT. EOF() .AND. RECNO() <= 10
    IF TITULO = "FW_T" .AND. AP1_AU = "FW_A"
        cnt = cnt + 1
    ENDIF
    SKIP
ENDDO
IF cnt = 4
    ? "PASS: Test 8 - REPLACE FOR..WHILE multi-field (", cnt, "/4)"
ELSE
    ? "FAIL: Test 8 - REPLACE FOR..WHILE multi-field (", cnt, "/4)"
    FAILS = FAILS + 1
ENDIF

* --- Test 9: Multi-line REPLACE (trailing comma line continuation) ---
REPLACE TITULO WITH "ML_T",
    AP1_AU WITH "ML_A",
    AP2_AU WITH "ML_B"
GO TOP
IF TITULO = "ML_T" .AND. AP1_AU = "ML_A" .AND. AP2_AU = "ML_B"
    ? "PASS: Test 9 - Multi-line REPLACE"
ELSE
    ? "FAIL: Test 9 - Multi-line REPLACE"
    FAILS = FAILS + 1
ENDIF

* --- Test 10: Cross-area REPLACE ---
SELECT 2
USE books_memo
SELECT 1
GO TOP
REPLACE TITULO WITH B->AP1_AU
GO TOP
IF LEN(TITULO) > 0
    ? "PASS: Test 10 - Cross-area REPLACE (B->AP1_AU)"
ELSE
    ? "FAIL: Test 10 - Cross-area REPLACE"
    FAILS = FAILS + 1
ENDIF

* --- Summary ---
?
IF FAILS = 0
    ? "*** ALL REPLACE MULTI-FIELD TESTS PASSED ***"
ELSE
    ? "*** ", FAILS, " REPLACE MULTI-FIELD TESTS FAILED ***"
ENDIF

CLOSE ALL
QUIT
