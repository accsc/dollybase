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

QUIT
