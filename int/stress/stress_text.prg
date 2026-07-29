* stress_text.prg — Test TEXT...ENDTEXT block

* --- Basic TEXT block ---
TEXT
Hello World
This is a multi-line text block.
ENDTEXT

* --- TEXT with macro expansion ---
myName = "dollybase"
myNum = 42
TEXT
Welcome to &myName
The answer is &myNum
ENDTEXT

* --- TEXT with empty lines ---
TEXT

Line after blank

Another blank above.

ENDTEXT

* --- TEXT on same line as keyword ---
TEXT Single line text block
ENDTEXT

* --- Verify execution continues after TEXT ---
? "After all TEXT blocks"

