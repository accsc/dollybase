* stress_list.prg — LIST command stress tests
* Requires books.dbf in the current directory

USE books

* Test 1: LIST current record
? "Test 1: LIST current record"
LIST

* Test 2: LIST NEXT 3
? "Test 2: LIST NEXT 3"
LIST NEXT 3

* Test 3: LIST REST
? "Test 3: LIST REST (first 5 via FOR)"
GO TOP
LIST REST FOR .F.

* Test 4: LIST ALL
? "Test 4: LIST ALL (first few via WHILE)"
LIST ALL WHILE RECNO() <= 5

* Test 5: LIST FIELD
? "Test 5: LIST FIELD TITULO, NMB_AU"
GO TOP
LIST NEXT 3 FIELD TITULO, NMB_AU

* Test 6: LIST FOR
? "Test 6: LIST FOR AN_EDICION > 0"
GO TOP
LIST ALL FOR AN_EDICION > 0 WHILE RECNO() <= 5

* Test 7: LIST RECORD n
? "Test 7: LIST RECORD 1"
LIST RECORD 1

* Test 8: LIST bare field names (no FIELD keyword)
? "Test 8: LIST bare field names"
GO TOP
LIST NEXT 2 TITULO, NMB_AU

* Test 9: LIST with $ operator in FOR
? "Test 9: LIST FOR with $ operator"
GO TOP
LIST TITULO FOR "ANONIMO"$AP1_AU

* Test 10: LIST FOR with $ on numeric coercion
? "Test 10: LIST FOR with $ on numeric"
GO TOP
LIST TITULO FOR "1983"$LTRIM(STR(AN_EDICION))

* Test 11: LIST FOR with .AND. and $
? "Test 11: LIST FOR with .AND. and $"
GO TOP
LIST TITULO, AP1_AU FOR "PROFETA"$TITULO .AND. LEN(AP1_AU) > 0 WHILE RECNO() <= 10

* --- Non-visual verification tests (logic only) ---

* Test 12: Verify $ filter matches expected count
GO TOP
count = 0
DO WHILE NOT EOF()
    IF "ANONIMO" $ AP1_AU
        count = count + 1
    ENDIF
    SKIP
ENDDO
? "Test 12: Records with ANONIMO in AP1_AU =", count

* Test 13: Verify bare field name in FOR works via LOCATE
GO TOP
LOCATE FOR "PROFETA" $ TITULO
? "Test 13: LOCATE PROFETA in TITULO - FOUND=", FOUND(), "RECNO=", RECNO()

* Test 14: Verify $ with numeric coercion
GO TOP
LOCATE FOR "1983" $ LTRIM(STR(AN_EDICION))
? "Test 14: LOCATE 1983 in AN_EDICION - FOUND=", FOUND(), "RECNO=", RECNO()

* Test 15: Verify .AND. with $ in LOCATE
GO TOP
LOCATE FOR "PROFETA" $ TITULO .AND. LEN(AP1_AU) > 0
? "Test 15: LOCATE PROFETA+LEN - FOUND=", FOUND(), "RECNO=", RECNO()

QUIT
