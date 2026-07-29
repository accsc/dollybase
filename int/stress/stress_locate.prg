* stress_locate.prg — LOCATE FOR / LOCATE FOR...WHILE stress tests
* Uses books_memo.dbf (38 records)

USE books_memo

* Test 1: LOCATE FOR with a matching numeric condition
* AN_EDICION is a numeric field — find first record where year >= 1990
LOCATE FOR A->AN_EDICION >= 1990
? "Test 1 - LOCATE FOR numeric (>= 1990): FOUND=", FOUND(), " RECNO=", RECNO()

* Test 2: LOCATE FOR with a string condition
* Find first record where TITULO starts with "EN"
LOCATE FOR LEFT(A->TITULO, 2) = "EN"
? "Test 2 - LOCATE FOR string (starts EN): FOUND=", FOUND(), " RECNO=", RECNO()

* Test 3: LOCATE FOR that matches nothing
LOCATE FOR A->AN_EDICION >= 9999
? "Test 3 - LOCATE FOR no match: FOUND=", FOUND()

* Test 4: LOCATE FOR with WHILE guard that stops early
* Find a record with AN_EDICION >= 2000, but only scan while RECNO <= 5
LOCATE FOR A->AN_EDICION >= 2000 WHILE RECNO() <= 5
? "Test 4 - LOCATE FOR..WHILE (early stop): FOUND=", FOUND(), " RECNO=", RECNO()

* Test 5: LOCATE FOR with WHILE that lets all records through
LOCATE FOR A->AN_EDICION >= 1990 WHILE RECNO() <= 38
? "Test 5 - LOCATE FOR..WHILE (full scan): FOUND=", FOUND(), " RECNO=", RECNO()

* Test 6: LOCATE FOR with AND condition
LOCATE FOR A->AN_EDICION >= 1990 AND LEN(A->TITULO) > 5
? "Test 6 - LOCATE FOR AND: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 7: LOCATE FOR with OR condition
LOCATE FOR A->AN_EDICION >= 9999 OR A->AN_EDICION >= 1900
? "Test 7 - LOCATE FOR OR: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 8: LOCATE FOR with NOT
LOCATE FOR .NOT. (A->AN_EDICION < 1900)
? "Test 8 - LOCATE FOR NOT: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 9: LOCATE FOR with memo field
LOCATE FOR LEN(A->COMMENTS) > 50
? "Test 9 - LOCATE FOR memo len > 50: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 10: LOCATE FOR with WHILE that is false from the start
LOCATE FOR A->AN_EDICION >= 1990 WHILE RECNO() > 100
? "Test 10 - LOCATE FOR..WHILE (false from start): FOUND=", FOUND()

* Test 11: Multiple LOCATE calls in sequence — verify FOUND() resets
LOCATE FOR A->AN_EDICION >= 1990
? "Test 11a - First LOCATE: FOUND=", FOUND()
LOCATE FOR A->AN_EDICION >= 9999
? "Test 11b - Second LOCATE (no match): FOUND=", FOUND()

* Test 12: LOCATE after GO BOTTOM — should restart from top
GO BOTTOM
LOCATE FOR A->AN_EDICION >= 1990
? "Test 12 - LOCATE after GO BOTTOM: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 13: LOCATE FOR with comparison on field = literal
LOCATE FOR A->AN_EDICION = 1603
? "Test 13 - LOCATE FOR exact match: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 14: LOCATE FOR with EMPTY check
LOCATE FOR EMPTY(A->TITULO)
? "Test 14 - LOCATE FOR EMPTY: FOUND=", FOUND()

* Test 15: LOCATE FOR with IIF in expression
LOCATE FOR IIF(A->AN_EDICION >= 1990, .T., .F.)
? "Test 15 - LOCATE FOR IIF: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 16: LOCATE FOR with complex AND expression
LOCATE FOR A->AN_EDICION >= 1950 .AND. LEN(A->TITULO) > 10
? "Test 16 - LOCATE FOR AND complex: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 17: LOCATE FOR with string comparison
LOCATE FOR LEFT(A->TITULO, 4) = "EN CA"
? "Test 17 - LOCATE FOR string: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 18: LOCATE FOR with BETWEEN
LOCATE FOR BETWEEN(A->AN_EDICION, 1980, 2000)
? "Test 18 - LOCATE FOR BETWEEN: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 19: LOCATE FOR with NOT condition
LOCATE FOR .NOT. (A->AN_EDICION >= 1990)
? "Test 19 - LOCATE FOR NOT: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 20: LOCATE FOR with OR condition
LOCATE FOR A->AN_EDICION >= 9999 .OR. A->AN_EDICION >= 1600
? "Test 20 - LOCATE FOR OR: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 21: LOCATE FOR with WHILE that stops before match
LOCATE FOR A->AN_EDICION >= 2000 WHILE RECNO() <= 2
? "Test 21 - LOCATE FOR..WHILE (no match in range): FOUND=", FOUND()

* Test 22: LOCATE FOR with WHILE that includes match
LOCATE FOR A->AN_EDICION >= 2000 WHILE RECNO() <= 10
? "Test 22 - LOCATE FOR..WHILE (match in range): FOUND=", FOUND(), " RECNO=", RECNO()

* Test 23: LOCATE FOR with EMPTY check
LOCATE FOR EMPTY(A->TITULO)
? "Test 23 - LOCATE FOR EMPTY: FOUND=", FOUND()

* Test 24: LOCATE FOR with IIF in expression
LOCATE FOR IIF(A->AN_EDICION >= 1990, .T., .F.)
? "Test 24 - LOCATE FOR IIF: FOUND=", FOUND(), " RECNO=", RECNO()

* Test 25: LOCATE FOR with SUBSTR on field
LOCATE FOR SUBSTR(A->TITULO, 1, 2) = "EN"
? "Test 25 - LOCATE FOR SUBSTR: FOUND=", FOUND(), " RECNO=", RECNO()

* ============================================================
* CONTINUE tests
* ============================================================

* Test 26: CONTINUE finds next match after LOCATE
LOCATE FOR A->AN_EDICION >= 1990
? "Test 26a - LOCATE: FOUND=", FOUND(), " RECNO=", RECNO()
first_rec = RECNO()
CONTINUE
? "Test 26b - CONTINUE: FOUND=", FOUND(), " RECNO=", RECNO()
? "Test 26c - second > first:", RECNO() > first_rec
second_rec = RECNO()

* Test 27: CONTINUE finds a third match
CONTINUE
? "Test 27 - CONTINUE again: FOUND=", FOUND(), " RECNO=", RECNO()
? "Test 27b - third > second:", RECNO() > second_rec

* Test 28: CONTINUE with WHILE guard
LOCATE FOR A->AN_EDICION >= 1990 WHILE RECNO() <= 20
? "Test 28a - LOCATE FOR..WHILE: FOUND=", FOUND(), " RECNO=", RECNO()
first_rec = RECNO()
CONTINUE
? "Test 28b - CONTINUE (should respect WHILE): FOUND=", FOUND(), " RECNO=", RECNO()

* Test 29: CONTINUE without prior LOCATE (should return FOUND=.F.)
locate_active = 0
LOCATE FOR A->AN_EDICION >= 9999
? "Test 29a - LOCATE no match: FOUND=", FOUND()
* Now CONTINUE after a failed LOCATE should still work (resume from current+1)
CONTINUE
? "Test 29b - CONTINUE after no match: FOUND=", FOUND()

* Test 30: CONTINUE exhausts all records
LOCATE FOR A->AN_EDICION >= 9999
? "Test 30a - LOCATE impossible: FOUND=", FOUND()
CONTINUE
? "Test 30b - CONTINUE impossible: FOUND=", FOUND()

* Test 31: CONTINUE invalidated after CLOSE + USE (DB pointer changed)
LOCATE FOR A->AN_EDICION >= 1990
? "Test 31a - LOCATE on books_memo: FOUND=", FOUND(), " RECNO=", RECNO()
CLOSE
USE books_memo
CONTINUE
? "Test 31b - CONTINUE after CLOSE+USE (DB changed): FOUND=", FOUND()

* Test 32: Per-area LOCATE — LOCATE on area 1, switch to area 2,
*         LOCATE on area 2, CONTINUE on area 2, switch back, CONTINUE on area 1
LOCATE FOR A->AN_EDICION >= 1990
? "Test 32a - LOCATE area 1: FOUND=", FOUND(), " RECNO=", RECNO()
area1_rec = RECNO()
SELECT 2
USE books_memo
LOCATE FOR A->AN_EDICION >= 2000
? "Test 32b - LOCATE area 2: FOUND=", FOUND(), " RECNO=", RECNO()
CONTINUE
? "Test 32c - CONTINUE area 2: FOUND=", FOUND(), " RECNO=", RECNO()
SELECT 1
CONTINUE
? "Test 32d - CONTINUE area 1 (after area 2): FOUND=", FOUND(), " RECNO=", RECNO()
? "Test 32e - area1 advanced:", RECNO() > area1_rec

SELECT 2
CLOSE
SELECT 1
CLOSE
