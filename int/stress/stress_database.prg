* stress_database.prg — Test database operations

USE books
? "DBF name:", ALIAS()
? "Records:", RECN()
? "First record:", RECNO()

DISPLAY STRUCTURE


? "--- First 3 records ---"
DO WHILE RECNO() <= 3 AND NOT EOF()
  ? "RECNO:", RECNO()
  SKIP
ENDDO

? "--- GO n ---"
GO 5
? "After GO 5, RECNO:", RECNO()

? "--- SKIP ---"
GO TOP
SKIP 3
? "After SKIP 3, RECNO:", RECNO()

? "--- SKIP negative ---"
GO BOTTOM
SKIP -5
? "After SKIP -5 from bottom, RECNO:", RECNO()

? "--- GO TOP ---"
GO TOP
? "After GO TOP, RECNO:", RECNO()

WAIT "Press any key to continue..."
CLEAR

? "--- GO BOTTOM ---"
GO BOTTOM
? "After GO BOTTOM, RECNO:", RECNO()

? "--- EOF at last record (should be .T.) ---"
? "EOF:", EOF()

? "--- SKIP past last record ---"
SKIP
? "After SKIP past bottom, RECNO:", RECNO()

? "--- EOF past last record (should be .T.) ---"
? "EOF:", EOF()

? "--- GO TOP ---"
GO TOP
? "After GO TOP, RECNO:", RECNO()
? "BOF:", BOF()

* --- LOCATE tests ---
? "--- LOCATE tests ---"

LOCATE FOR A->AN_EDICION >= 1990
? "LOCATE FOR year>=1990: FOUND=", FOUND(), " RECNO=", RECNO()

LOCATE FOR A->AN_EDICION >= 9999
? "LOCATE no match: FOUND=", FOUND()

LOCATE FOR A->AN_EDICION >= 2000 WHILE RECNO() <= 5
? "LOCATE FOR..WHILE: FOUND=", FOUND()

* --- DELETE scope tests ---
? "--- DELETE scope tests ---"

* DELETE single record
GO TOP
DELETE
? "DELETE single - DELETED():", DELETED()
RECALL
? "RECALL single - DELETED():", DELETED()

* DELETE FOR condition
DELETE FOR A->AN_EDICION >= 2000
? "DELETE FOR year>=2000 - done"
GO TOP
del_count = 0
DO WHILE NOT EOF()
  IF DELETED()
    del_count = del_count + 1
  ENDIF
  SKIP
ENDDO
? "Records deleted:", del_count

* RECALL FOR condition
RECALL FOR DELETED()
? "RECALL FOR DELETED() - restored"

* DELETE ALL
DELETE ALL
GO TOP
? "DELETE ALL - rec 1 DELETED():", DELETED()
GO BOTTOM
? "DELETE ALL - rec 38 DELETED():", DELETED()

* RECALL ALL
RECALL ALL
GO TOP
? "RECALL ALL - rec 1 DELETED():", DELETED()
GO BOTTOM
? "RECALL ALL - rec 38 DELETED():", DELETED()

* DELETE FOR ... WHILE combined
DELETE FOR A->AN_EDICION >= 1950 WHILE RECNO() <= 5
? "DELETE FOR..WHILE (rec<=5) - done"
GO TOP
del_count = 0
DO WHILE NOT EOF()
  IF DELETED()
    del_count = del_count + 1
  ENDIF
  SKIP
ENDDO
? "Records deleted by FOR..WHILE:", del_count
RECALL ALL

* --- REPLACE scope tests ---
? "--- REPLACE scope tests ---"

* REPLACE single
GO TOP
REPLACE TITULO WITH "MODIFIED_SINGLE"
? "REPLACE single - TITULO:", A->TITULO
REPLACE TITULO WITH "EN CASA DEL PROFETA"

* REPLACE ALL
REPLACE ALL TITULO WITH "ALL_REPLACED"
GO TOP
? "REPLACE ALL - rec 1:", A->TITULO
GO BOTTOM
? "REPLACE ALL - rec 38:", A->TITULO

* REPLACE FOR
REPLACE TITULO WITH "FOR_REPLACED" FOR A->AN_EDICION >= 1990
GO TOP
? "REPLACE FOR - rec 1:", A->TITULO

* REPLACE FOR ... WHILE
REPLACE TITULO WITH "SCOPE_TEST" FOR A->AN_EDICION >= 1900 WHILE RECNO() <= 10
? "REPLACE FOR..WHILE - done"

* Restore
REPLACE ALL TITULO WITH "EN CASA DEL PROFETA"

CLOSE DATABASES
? "Done"
