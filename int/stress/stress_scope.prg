* stress_scope.prg — DELETE/RECALL/REPLACE scope modifier tests
* Uses books_memo.dbf (38 records)

USE books_memo

* ============================================================
* REPLACE scope tests
* ============================================================

* Test 1: REPLACE single record (no scope)
GO TOP
REPLACE TITULO WITH "MODIFIED"
? "Test 1 - REPLACE single: RECNO=", RECNO(), " TITULO=", A->TITULO

* Restore
REPLACE TITULO WITH "EN CASA DEL PROFETA"

* Test 2: REPLACE ALL
REPLACE ALL TITULO WITH "ALL_REPLACED"
GO TOP
? "Test 2a - REPLACE ALL (rec 1): TITULO=", A->TITULO
GO BOTTOM
? "Test 2b - REPLACE ALL (rec 38): TITULO=", A->TITULO

* Restore all titles
* (we skip restore since subsequent tests use AN_EDICION)

* Test 3: REPLACE ... FOR condition
REPLACE TITULO WITH "HIGH_YEAR" FOR A->AN_EDICION >= 1990
? "Test 3 - REPLACE FOR: done"
GO TOP
? "Test 3a - rec 1 (should be HIGH_YEAR if >= 1990): TITULO=", A->TITULO

* Test 4: REPLACE ... FOR ... WHILE combined
REPLACE TITULO WITH "SCOPE_TEST" FOR A->AN_EDICION >= 1900 WHILE RECNO() <= 10
? "Test 4 - REPLACE FOR..WHILE: done"

* Restore titles
REPLACE ALL TITULO WITH "EN CASA DEL PROFETA"
? "Test 4b - restored: TITULO=", A->TITULO

* ============================================================
* DELETE scope tests
* ============================================================

* Test 5: DELETE single record
GO TOP
DELETE
? "Test 5 - DELETE single: DELETED()=", DELETED()
RECALL
? "Test 5b - after RECALL: DELETED()=", DELETED()

* Test 6: DELETE FOR condition
DELETE FOR A->AN_EDICION >= 2000
? "Test 6 - DELETE FOR: done"
GO TOP
DO WHILE NOT EOF()
  IF DELETED()
    ? "Test 6a - deleted rec:", RECNO()
  ENDIF
  SKIP
ENDDO

* Recall all deleted
RECALL FOR DELETED()
? "Test 6b - RECALL FOR DELETED(): restored"

* Test 7: DELETE ALL
DELETE ALL
GO TOP
? "Test 7a - DELETE ALL rec 1: DELETED()=", DELETED()
GO BOTTOM
? "Test 7b - DELETE ALL rec 38: DELETED()=", DELETED()

* Test 8: RECALL ALL
RECALL ALL
GO TOP
? "Test 8 - RECALL ALL rec 1: DELETED()=", DELETED()
GO BOTTOM
? "Test 8b - RECALL ALL rec 38: DELETED()=", DELETED()

* Test 9: DELETE FOR ... WHILE combined
DELETE FOR A->AN_EDICION >= 1950 WHILE RECNO() <= 5
? "Test 9 - DELETE FOR..WHILE (rec<=5): done"
GO TOP
DO WHILE NOT EOF()
  IF DELETED()
    ? "Test 9a - deleted rec:", RECNO()
  ENDIF
  SKIP
ENDDO

* Recall all
RECALL ALL

* Test 10: RECALL FOR condition
DELETE FOR A->AN_EDICION >= 1990
RECALL FOR A->AN_EDICION >= 2000
? "Test 10 - RECALL FOR: done"
GO TOP
DO WHILE NOT EOF()
  IF DELETED()
    ? "Test 10a - still deleted rec:", RECNO()
  ENDIF
  SKIP
ENDDO

* Final cleanup
RECALL ALL

* ============================================================
* NEXT n scope tests
* ============================================================

* Test 11: DELETE NEXT 3
GO TOP
SKIP 2
? "Test 11 - DELETE NEXT 3: at RECNO=", RECNO()
DELETE NEXT 3
? "Test 11a - after DELETE NEXT 3: RECNO=", RECNO()
GO TOP
DO WHILE NOT EOF()
  IF DELETED()
    ? "Test 11b - deleted rec:", RECNO()
  ENDIF
  SKIP
ENDDO
RECALL ALL

* Test 12: RECALL NEXT 2
DELETE ALL
GO TOP
RECALL NEXT 2
? "Test 12 - RECALL NEXT 2: at RECNO=", RECNO()
GO TOP
? "Test 12a - rec 1 DELETED()=", DELETED()
SKIP
? "Test 12b - rec 2 DELETED()=", DELETED()
SKIP
? "Test 12c - rec 3 DELETED()=", DELETED()
RECALL ALL

* Test 13: REPLACE NEXT 3 field WITH expr
GO TOP
REPLACE NEXT 3 TITULO WITH "NEXT_REPLACED"
? "Test 13 - REPLACE NEXT 3: done, RECNO=", RECNO()
GO TOP
? "Test 13a - rec 1:", A->TITULO
SKIP
? "Test 13b - rec 2:", A->TITULO
SKIP
? "Test 13c - rec 3:", A->TITULO
SKIP
? "Test 13d - rec 4 (should be original):", A->TITULO
REPLACE ALL TITULO WITH "EN CASA DEL PROFETA"

* Test 14: DELETE NEXT with FOR condition
GO TOP
SKIP 2
DELETE NEXT 5 FOR A->AN_EDICION >= 1990
? "Test 14 - DELETE NEXT 5 FOR: done"
GO TOP
DO WHILE NOT EOF()
  IF DELETED()
    ? "Test 14a - deleted rec:", RECNO()
  ENDIF
  SKIP
ENDDO
RECALL ALL

* Test 15: REPLACE NEXT with FOR condition
GO TOP
SKIP 1
REPLACE NEXT 3 TITULO WITH "COND_REPLACED" FOR A->AN_EDICION >= 1980
? "Test 15 - REPLACE NEXT 3 FOR: done"
GO TOP
? "Test 15a - rec 1:", A->TITULO
SKIP
? "Test 15b - rec 2:", A->TITULO
SKIP
? "Test 15c - rec 3:", A->TITULO
REPLACE ALL TITULO WITH "EN CASA DEL PROFETA"

* ============================================================
* RECORD n scope tests
* ============================================================

* Test 16: DELETE RECORD 5
DELETE RECORD 5
? "Test 16 - DELETE RECORD 5: done"
GO TOP
SKIP 4
? "Test 16a - rec 5 DELETED()=", DELETED()
SKIP
? "Test 16b - rec 6 DELETED()=", DELETED()
RECALL ALL

* Test 17: REPLACE RECORD 10 field WITH expr
REPLACE RECORD 10 TITULO WITH "RECORD_10_REPLACED"
? "Test 17 - REPLACE RECORD 10: done"
GO TOP
SKIP 9
? "Test 17a - rec 10:", A->TITULO
REPLACE ALL TITULO WITH "EN CASA DEL PROFETA"

* Test 18: RECALL RECORD 3
DELETE ALL
RECALL RECORD 3
? "Test 18 - RECALL RECORD 3: done"
GO TOP
? "Test 18a - rec 1 DELETED()=", DELETED()
SKIP
? "Test 18b - rec 2 DELETED()=", DELETED()
SKIP
? "Test 18c - rec 3 DELETED()=", DELETED()
RECALL ALL

* ============================================================
* REST scope tests
* ============================================================

* Test 19: DELETE REST
GO TOP
SKIP 36
DELETE REST
? "Test 19 - DELETE REST from rec 37: done"
GO TOP
SKIP 36
? "Test 19a - rec 37 DELETED()=", DELETED()
SKIP
? "Test 19b - rec 38 DELETED()=", DELETED()
RECALL ALL

* Test 20: REPLACE REST field WITH expr
GO TOP
SKIP 36
REPLACE REST TITULO WITH "REST_REPLACED"
? "Test 20 - REPLACE REST: done"
GO TOP
SKIP 37
? "Test 20a - rec 38:", A->TITULO
REPLACE ALL TITULO WITH "EN CASA DEL PROFETA"

* Test 21: RECALL REST
DELETE ALL
GO TOP
SKIP 36
RECALL REST
? "Test 21 - RECALL REST from rec 37: done"
GO TOP
SKIP 37
? "Test 21a - rec 38 DELETED()=", DELETED()
RECALL ALL

* ============================================================
* Final cleanup
* ============================================================
RECALL ALL

CLOSE
