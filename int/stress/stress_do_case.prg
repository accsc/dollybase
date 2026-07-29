* --- DO CASE / CASE / OTHERWISE / ENDCASE stress tests ---

* Test 1: Basic DO CASE with numeric match
x = 2
result1 = ""
DO CASE
    CASE x = 1
        result1 = "ONE"
    CASE x = 2
        result1 = "TWO"
    CASE x = 3
        result1 = "THREE"
    OTHERWISE
        result1 = "OTHER"
ENDCASE
? "Test 1 (basic match): result1=" + result1

* Test 2: First case matches
x = 1
result2 = ""
DO CASE
    CASE x = 1
        result2 = "FIRST"
    CASE x = 2
        result2 = "SECOND"
    OTHERWISE
        result2 = "OTHER"
ENDCASE
? "Test 2 (first match): result2=" + result2

* Test 3: OTHERWISE catches unmatched
x = 99
result3 = ""
DO CASE
    CASE x = 1
        result3 = "ONE"
    CASE x = 2
        result3 = "TWO"
    OTHERWISE
        result3 = "CAUGHT"
ENDCASE
? "Test 3 (otherwise): result3=" + result3

* Test 4: DO CASE without OTHERWISE (no match)
x = 50
result4 = ""
DO CASE
    CASE x = 1
        result4 = "ONE"
    CASE x = 2
        result4 = "TWO"
ENDCASE
? "Test 4 (no match, no otherwise): result4=" + result4

* Test 5: String comparison in CASE
name = "ALICE"
greeting = ""
DO CASE
    CASE name = "BOB"
        greeting = "HI BOB"
    CASE name = "ALICE"
        greeting = "HI ALICE"
    OTHERWISE
        greeting = "HI STRANGER"
ENDCASE
? "Test 5 (string match): greeting=" + greeting

* Test 6: Complex expression in CASE
a = 10
b = 20
result6 = ""
DO CASE
    CASE a + b > 30
        result6 = "BIG"
    CASE a + b > 15
        result6 = "MEDIUM"
    OTHERWISE
        result6 = "SMALL"
ENDCASE
? "Test 6 (complex expr): result6=" + result6

* Test 7: Nested DO CASE inside DO WHILE
i = 1
total = 0
DO WHILE i <= 3
    DO CASE
        CASE i = 1
            total = total + 10
        CASE i = 2
            total = total + 20
        OTHERWISE
            total = total + 30
    ENDCASE
    i = i + 1
ENDDO
? "Test 7 (nested in loop): total=" + LTRIM(STR(total))

* Test 8: Multiple statements in a CASE branch
x = 2
val1 = 0
flag = ""
DO CASE
    CASE x = 1
        val1 = 100
        flag = "SET1"
    CASE x = 2
        val1 = 200
        flag = "SET2"
    OTHERWISE
        val1 = 999
        flag = "SET_OTHER"
ENDCASE
? "Test 8 (multi stmt): val1=" + LTRIM(STR(val1)) + " flag=" + flag

* Test 9: Logical expression with .AND.
a = 5
b = 10
result9 = ""
DO CASE
    CASE a > 3 .AND. b > 5
        result9 = "BOTH"
    CASE a > 3 .OR. b > 5
        result9 = "EITHER"
    OTHERWISE
        result9 = "NEITHER"
ENDCASE
? "Test 9 (.AND.): result9=" + result9

* Test 10: Empty DO CASE (no branches)
result10 = "BEFORE"
DO CASE
ENDCASE
result10 = result10 + "_AFTER"
? "Test 10 (empty case): result10=" + result10

* Test 11: DO CASE with only OTHERWISE
result11 = ""
DO CASE
    OTHERWISE
        result11 = "ALWAYS"
ENDCASE
? "Test 11 (only otherwise): result11=" + result11

* Test 12: CASE with .NOT.
x = .F.
result12 = ""
DO CASE
    CASE .NOT. x
        result12 = "FALSE"
    CASE x
        result12 = "TRUE"
ENDCASE
? "Test 12 (.NOT.): result12=" + result12

? "All DO CASE tests complete."
