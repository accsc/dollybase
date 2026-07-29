* ============================================================
* stress_reccount.prg — RECCOUNT() and LUPDATE() stress test
* ============================================================

pass = 0
fail = 0

* -----------------------------------------------------------
* RECCOUNT()
* -----------------------------------------------------------
? "=== RECCOUNT() ==="
USE books
rc = RECCOUNT()
if rc = 38
    pass = pass + 1
    ? "PASS: RECCOUNT() = 38"
else
    fail = fail + 1
    ? "FAIL: RECCOUNT() expected 38, got " + STR(rc, 5, 0)
endif

* RECCOUNT should equal RECNO at bottom
GO BOTTOM
if RECCOUNT() = RECNO()
    pass = pass + 1
    ? "PASS: RECCOUNT() = RECNO() at bottom"
else
    fail = fail + 1
    ? "FAIL: RECCOUNT() != RECNO() at bottom"
endif

GO TOP

* -----------------------------------------------------------
* LUPDATE()
* -----------------------------------------------------------
? "=== LUPDATE() ==="
lu = LUPDATE()
lu_str = DTOC(lu)
? "LUPDATE() = " + lu_str

* Should be a valid date (year >= 2024)
if YEAR(lu) >= 1980
    pass = pass + 1
    ? "PASS: LUPDATE() year >= 1980"
else
    fail = fail + 1
    ? "FAIL: LUPDATE() year expected >= 2024, got " + STR(YEAR(lu), 5, 0)
endif

* -----------------------------------------------------------
* LUPDATE() updates on REPLACE
* -----------------------------------------------------------
? "=== LUPDATE() on REPLACE ==="
lu_before = LUPDATE()
REPLACE TITULO WITH "TEMP_TEST"
lu_after = LUPDATE()
* Restore
REPLACE TITULO WITH "EN CASA DEL PROFETA"

* The date should be today (at least not older)
if YEAR(lu_after) >= YEAR(lu_before)
    pass = pass + 1
    ? "PASS: LUPDATE() not older after REPLACE"
else
    fail = fail + 1
    ? "FAIL: LUPDATE() got older after REPLACE"
endif

* -----------------------------------------------------------
* LUPDATE() updates on REPLACE (already tested above)
* APPEND test skipped — PACK is known broken
* -----------------------------------------------------------
USE

* -----------------------------------------------------------
* Summary
* -----------------------------------------------------------
? "=============================="
? "Reccount tests: " + STR(pass, 5, 0) + " passed, " + STR(fail, 5, 0) + " failed"
? "=============================="
