* stress_use_alias.prg — Test USE with INDEX and ALIAS clauses
FAILS = 0

* --- Test 1: USE with ALIAS ---
USE books ALIAS mislibros
a = ALIAS()
IF a != "mislibros"
    ? "FAIL: Test 1 - ALIAS() returned '", a, "' expected 'mislibros'"
    FAILS = FAILS + 1
ELSE
    ? "PASS: Test 1 - ALIAS() = ", a
ENDIF

* --- Test 2: USE without ALIAS returns DBF name ---
SELECT 2
USE books
a2 = ALIAS()
IF a2 = "books"
    ? "PASS: Test 2 - ALIAS() = ", a2, " (no custom alias)"
ELSE
    ? "FAIL: Test 2 - ALIAS() returned '", a2, "' expected 'books'"
    FAILS = FAILS + 1
ENDIF

* --- Test 3: Alias resolves in -> access ---
SELECT 1
x = mislibros->TITULO
IF LEN(x) > 0
    ? "PASS: Test 3 - mislibros->TITULO resolved"
ELSE
    ? "FAIL: Test 3 - mislibros->TITULO did not resolve"
    FAILS = FAILS + 1
ENDIF

* --- Test 4: Multiple areas with different aliases ---
SELECT 3
USE books_memo ALIAS area3_db
SELECT 3
a3 = ALIAS()
IF a3 = "area3_db"
    ? "PASS: Test 4 - area3 alias = ", a3
ELSE
    ? "FAIL: Test 4 - Expected area3_db, got '", a3, "'"
    FAILS = FAILS + 1
ENDIF

* --- Test 5: Cross-area access with custom alias ---
SELECT 1
t1 = mislibros->TITULO
SELECT 3
t3 = area3_db->TITULO
IF LEN(t1) > 0 .AND. LEN(t3) > 0
    ? "PASS: Test 5 - Cross-area alias access works"
ELSE
    ? "FAIL: Test 5 - Cross-area alias access failed"
    FAILS = FAILS + 1
ENDIF

* --- Test 6: Alias resolves via alias_to_area (used in field expressions) ---
SELECT 1
y = mislibros->AP1_AU
IF LEN(y) > 0
    ? "PASS: Test 6 - Alias resolves for field AP1_AU"
ELSE
    ? "FAIL: Test 6 - Alias failed for AP1_AU"
    FAILS = FAILS + 1
ENDIF

* --- Summary ---
?
IF FAILS = 0
    ? "*** ALL USE/ALIAS TESTS PASSED ***"
ELSE
    ? "*** ", FAILS, " USE/ALIAS TESTS FAILED ***"
ENDIF

CLOSE ALL
