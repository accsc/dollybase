* stress_ui.prg — @...SAY / @...GET / READ / CLEAR stress tests
* NOTE: Run in interactive mode (stdin) with ncurses.
*   ../prg < stress_ui.prg    (press Escape to exit each READ)
* The SAY tests work in file mode too, but GET/READ need ncurses.

* Test 1: Basic @...SAY
@ 1,1 SAY "Hello from dollybase"
@ 2,1 SAY "Test 1 passed"

* Test 2: @...SAY with variable concat
msg = "World"
@ 3,1 SAY "Hello, " + msg

* Test 3: CLEAR screen
CLEAR
@ 4,1 SAY "Screen cleared"

* Test 4: @...GET with READ (simple)
@ 5,1 SAY "Enter name: "
@ 5,15 GET mName
READ
@ 6,1 SAY "Name = " + mName

* Test 5: @...GET with DEFAULT
@ 7,1 SAY "Enter age (default 25): "
@ 7,28 GET mAge DEFAULT 25
READ
@ 8,1 SAY "Age = " + mAge

* Test 6: @...GET with RANGE
@ 9,1 SAY "Enter score (0-100): "
@ 9,25 GET mScore RANGE 0,100
READ
@ 10,1 SAY "Score = " + mScore

* Test 7: @...GET with FOCUS (cursor starts on second field)
@ 11,1 SAY "First: "
@ 11,10 GET mFirst
@ 12,1 SAY "Second: "
@ 12,10 GET mSecond FOCUS
READ
@ 13,1 SAY "First=" + mFirst + " Second=" + mSecond

* Test 8: Multiple GETs in one READ
@ 14,1 SAY "A: "
@ 14,5 GET mA
@ 15,1 SAY "B: "
@ 15,5 GET mB
@ 16,1 SAY "C: "
@ 16,5 GET mC
READ
@ 17,1 SAY "A=" + mA + " B=" + mB + " C=" + mC

