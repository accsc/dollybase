* Stress test for CLEAR MEMORY command
* Clears variables only — databases stay open

x = 42
msg = "hello"

USE books
? "Before CLEAR MEMORY:"
? "  x = " + LTRIM(STR(x))
? "  DBF() = " + DBF()
? "  RECCOUNT() = " + LTRIM(STR(RECCOUNT()))

CLEAR MEMORY

? "After CLEAR MEMORY:"
? "  x = " + LTRIM(STR(x))
? "  DBF() = " + DBF()
? "  RECCOUNT() = " + LTRIM(STR(RECCOUNT()))

? "=== CLEAR MEMORY test complete ==="
QUIT
