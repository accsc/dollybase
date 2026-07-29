* stress_rect.prg — @...TO rectangle and @...CLEAR tests
CLEAR
@ 1,1 SAY "=== Rectangle Tests ==="

* Test 1: Basic rectangle (single line)
@ 3,5 TO @ 8,30
@ 2,5 SAY "Test 1: Basic rectangle OK"

* Test 2: Double-line rectangle
@ 10,5 TO @ 14,35 DOUBLE
@ 9,5 SAY "Test 2: Double rectangle OK"

* Test 3: @...CLEAR TO (clear a region)
@ 16,1 SAY "This text will be cleared in the middle"
@ 16,1 CLEAR TO @ 16,20
@ 17,1 SAY "Test 3: Regional clear OK"

* Test 4: @...CLEAR without TO (clear from coord to bottom-right)
@ 19,1 SAY "This line will be cleared"
@ 20,1 SAY "This too"
@ 19,1 CLEAR
@ 18,1 SAY "Test 4: Full clear from coord OK"

* Test 5: Rectangle with SAY inside
@ 22,10 TO @ 25,40
@ 23,15 SAY "Inside!"
@ 21,10 SAY "Test 5: Rectangle with SAY OK"

WAIT "Press any key to finish rectangle tests"
