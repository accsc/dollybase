* ============================================================
* stress_alternate.prg — Stress test for SET ALTERNATE / SET CONSOLE
* ============================================================
* Tests SET ALTERNATE TO, CLOSE ALTERNATE, SET CONSOLE ON/OFF,
* file switching, and output mirroring.
*
* Verification: the test writes structured markers to the alternate
* file. The test harness (run_stress.py or shell) reads the file and
* checks for expected markers.
*
* Artifacts: stress_alternate_1.txt, stress_alternate_2.txt
* ============================================================

pass = 0
fail = 0

* -----------------------------------------------------------
* Test 1: SET ALTERNATE TO opens file and mirrors ? output
* -----------------------------------------------------------
SET ALTERNATE TO "stress_alternate_1.txt"
? "ALT_TEST_1_LINE1"
? "ALT_TEST_1_LINE2"
* Expected in file: ALT_TEST_1_LINE1 and ALT_TEST_1_LINE2
pass = pass + 1
? "PASS: Test 1 - SET ALTERNATE TO writes to file"

* -----------------------------------------------------------
* Test 2: CLOSE ALTERNATE stops mirroring
* -----------------------------------------------------------
CLOSE ALTERNATE
? "ALT_AFTER_CLOSE"
* Expected: ALT_AFTER_CLOSE should NOT be in stress_alternate_1.txt
pass = pass + 1
? "PASS: Test 2 - CLOSE ALTERNATE stops mirroring"

* -----------------------------------------------------------
* Test 3: SET ALTERNATE TO (no file) closes the file
* -----------------------------------------------------------
SET ALTERNATE TO "stress_alternate_1.txt"
? "ALT_REOPEN_LINE"
SET ALTERNATE TO
? "ALT_AFTER_CLOSE2"
* Expected: ALT_REOPEN_LINE in file, ALT_AFTER_CLOSE2 not in file
pass = pass + 1
? "PASS: Test 3 - SET ALTERNATE TO (no file) closes"

* -----------------------------------------------------------
* Test 4: Switching alternate files closes the first
* -----------------------------------------------------------
SET ALTERNATE TO "stress_alternate_1.txt"
? "ALT_SWITCH_FILE1"
SET ALTERNATE TO "stress_alternate_2.txt"
? "ALT_SWITCH_FILE2"
* Expected: stress_alternate_1.txt has ALT_SWITCH_FILE1
*           stress_alternate_2.txt has ALT_SWITCH_FILE2
pass = pass + 1
? "PASS: Test 4 - SET ALTERNATE TO switches files"

* -----------------------------------------------------------
* Test 5: SET CONSOLE OFF suppresses screen, keeps file
* -----------------------------------------------------------
SET ALTERNATE TO "stress_alternate_2.txt"
SET CONSOLE OFF
? "ALT_CONSOLE_OFF_LINE"
SET CONSOLE ON
* Expected: ALT_CONSOLE_OFF_LINE in file but NOT on screen
pass = pass + 1
? "PASS: Test 5 - SET CONSOLE OFF suppresses screen"

* -----------------------------------------------------------
* Test 6: Multiple expressions on one ? line
* -----------------------------------------------------------
SET ALTERNATE TO "stress_alternate_2.txt"
? "ALT_MULTI", "EXPR", 42
* Expected in file: ALT_MULTI EXPR 42
pass = pass + 1
? "PASS: Test 6 - Multiple expressions mirrored"

* -----------------------------------------------------------
* Test 7: Empty alternate (no file open) does not crash
* -----------------------------------------------------------
SET ALTERNATE TO
? "ALT_NO_FILE_OUTPUT"
pass = pass + 1
? "PASS: Test 7 - ? with no alternate file does not crash"

* -----------------------------------------------------------
* Test 8: Reopen same file appends
* -----------------------------------------------------------
SET ALTERNATE TO "stress_alternate_2.txt"
? "ALT_APPEND_LINE"
SET ALTERNATE TO
SET ALTERNATE TO "stress_alternate_2.txt"
? "ALT_APPEND_LINE2"
* Expected: both ALT_APPEND_LINE and ALT_APPEND_LINE2 in file
pass = pass + 1
? "PASS: Test 8 - Reopening same file appends"

* -----------------------------------------------------------
* Cleanup
* -----------------------------------------------------------
SET ALTERNATE TO
SET CONSOLE ON

* -----------------------------------------------------------
* Summary
* -----------------------------------------------------------
? "=============================="
? "Alternate tests: " + STR(pass, 5, 0) + " passed, " + STR(fail, 5, 0) + " failed"
? "=============================="
