* stress_set_order.prg - Test SET ORDER TO
pass = 0
fail = 0

CLOSE DATABASES
USE books

* Create index (overwrite any stale index)
INDEX ON TITULO TO idx_title
? "Index 1 (TITULO) loaded"

* With single index active, SEEK should work
SEEK "TEORIA Z"
if FOUND()
    pass = pass + 1
    ? "PASS: SEEK works with active index"
else
    fail = fail + 1
    ? "FAIL: SEEK should find TEORIA Z"
endif

* Suspend index with SET ORDER TO 0
SET ORDER TO 0
SEEK "TEORIA Z"
if .NOT. FOUND()
    pass = pass + 1
    ? "PASS: SEEK fails with ORDER 0 (suspended)"
else
    fail = fail + 1
    ? "FAIL: SEEK should fail with ORDER 0"
endif

* Re-enable index
SET ORDER TO 1
SEEK "TEORIA Z"
if FOUND()
    pass = pass + 1
    ? "PASS: SEEK works again after ORDER 1"
else
    fail = fail + 1
    ? "FAIL: SEEK should work with ORDER 1"
endif

* Test that ORDER beyond tag count keeps current order (dBase behavior)
SET ORDER TO 99
SEEK "TEORIA Z"
if FOUND()
    pass = pass + 1
    ? "PASS: SEEK works (ORDER 99 ignored, kept current)"
else
    fail = fail + 1
    ? "FAIL: SEEK should work (ORDER 99 ignored)"
endif

CLOSE DATABASES

? "=============================="
? "Set Order tests: " + STR(pass, 5, 0) + " passed, " + STR(fail, 5, 0) + " failed"
? "=============================="
RETURN
