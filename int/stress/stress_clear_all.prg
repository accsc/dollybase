* Stress test for CLEAR ALL command
* Sets up state, clears it, and verifies everything is reset

* Set up variables
x = 42
msg = "hello"

* Open a database
USE books
? "Before CLEAR ALL:"
? "  DBF() = " + DBF()
? "  RECCOUNT() = " + LTRIM(STR(RECCOUNT()))
? "  x = " + LTRIM(STR(x))
? "  msg = " + msg

* Clear everything
CLEAR ALL

* After CLEAR ALL: variables should be gone, DB closed
? "After CLEAR ALL:"

* Trying to use x should give empty/null
? "  x = " + LTRIM(STR(x))

* Trying to use DBF should give empty (no database open)
? "  DBF() = [" + DBF() + "]"

* Should be able to open a new database fine
USE books
? "  Re-opened books, RECCOUNT() = " + LTRIM(STR(RECCOUNT()))

? "=== CLEAR ALL test complete ==="
QUIT
