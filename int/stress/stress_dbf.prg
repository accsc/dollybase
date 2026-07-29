* stress_dbf.prg — test DBF() and ALIAS() functions
CLEAR

* Test 1: DBF() with no database open should return empty
? "Test 1: DBF() with no database open"
dname = DBF()
IF dname = ""
    ? "  PASS - DBF() returned empty string"
ELSE
    ? "  FAIL - DBF() returned: " + dname
ENDIF

* Test 2: ALIAS() with no database open should return empty
? "Test 2: ALIAS() with no database open"
aname = ALIAS()
IF aname = ""
    ? "  PASS - ALIAS() returned empty string"
ELSE
    ? "  FAIL - ALIAS() returned: " + aname
ENDIF

* Test 3: DBF() returns name with extension
? "Test 3: DBF() returns name with extension"
USE books
dname = DBF()
? "  DBF() = " + dname
IF RIGHT(LOWER(dname), 4) = ".dbf"
    ? "  PASS - DBF() includes .dbf extension"
ELSE
    ? "  FAIL - DBF() missing .dbf extension"
ENDIF

* Test 4: ALIAS() returns name without extension
? "Test 4: ALIAS() returns name without extension"
aname = ALIAS()
? "  ALIAS() = " + aname
IF AT(".dbf", aname) = 0
    ? "  PASS - ALIAS() has no extension"
ELSE
    ? "  FAIL - ALIAS() still has extension"
ENDIF

* Test 5: DBF() and ALIAS() should differ only by extension
? "Test 5: DBF() vs ALIAS() consistency"
IF LEFT(dname, LEN(aname)) = aname
    ? "  PASS - DBF() starts with ALIAS() name"
ELSE
    ? "  FAIL - DBF() and ALIAS() mismatch"
ENDIF

* Test 6: After USE another database, DBF() changes
USE books
dname2 = DBF()
aname2 = ALIAS()
? "Test 6: DBF() after re-USE books"
? "  DBF() = " + dname2
? "  ALIAS() = " + aname2
IF dname2 = dname
    ? "  PASS - DBF() consistent after re-USE"
ELSE
    ? "  FAIL - DBF() changed unexpectedly"
ENDIF

* Test 7: After CLOSE, DBF() returns empty again
USE
CLOSE
dname3 = DBF()
? "Test 7: DBF() after CLOSE"
IF dname3 = ""
    ? "  PASS - DBF() is empty after CLOSE"
ELSE
    ? "  FAIL - DBF() still returns: " + dname3
ENDIF

? "Done."
