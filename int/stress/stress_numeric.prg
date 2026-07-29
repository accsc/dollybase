* ============================================================
* stress_numeric.prg — Numerical functions stress test
* Tests: ABS, EXP, INT, LOG, MAX, MIN, MOD, ROUND, SIGN, SQRT, STR, VAL
* ============================================================

pass = 0
fail = 0

* -----------------------------------------------------------
* ABS()
* -----------------------------------------------------------
? "=== ABS() ==="
r = ABS(-5)
if r = 5
    pass = pass + 1
    ? "PASS: ABS(-5) = 5"
else
    fail = fail + 1
    ? "FAIL: ABS(-5) expected 5, got " + STR(r, 10, 0)
endif

r = ABS(3.14)
if r = 3.14
    pass = pass + 1
    ? "PASS: ABS(3.14) = 3.14"
else
    fail = fail + 1
    ? "FAIL: ABS(3.14) expected 3.14, got " + STR(r, 10, 0)
endif

r = ABS(0)
if r = 0
    pass = pass + 1
    ? "PASS: ABS(0) = 0"
else
    fail = fail + 1
    ? "FAIL: ABS(0) expected 0, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* EXP() — e^x
* -----------------------------------------------------------
? "=== EXP() ==="
r = EXP(0)
if r = 1
    pass = pass + 1
    ? "PASS: EXP(0) = 1"
else
    fail = fail + 1
    ? "FAIL: EXP(0) expected 1, got " + STR(r, 10, 0)
endif

r = EXP(1)
if r > 2.718 AND r < 2.719
    pass = pass + 1
    ? "PASS: EXP(1) ~ 2.718"
else
    fail = fail + 1
    ? "FAIL: EXP(1) expected ~2.718, got " + STR(r, 10, 0)
endif

* EXP and LOG are inverses
r = LOG(EXP(5))
if r > 4.999 AND r < 5.001
    pass = pass + 1
    ? "PASS: LOG(EXP(5)) ~ 5"
else
    fail = fail + 1
    ? "FAIL: LOG(EXP(5)) expected ~5, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* INT() — integer part (truncate toward zero)
* -----------------------------------------------------------
? "=== INT() ==="
r = INT(3.7)
if r = 3
    pass = pass + 1
    ? "PASS: INT(3.7) = 3"
else
    fail = fail + 1
    ? "FAIL: INT(3.7) expected 3, got " + STR(r, 10, 0)
endif

r = INT(-2.9)
if r = -2
    pass = pass + 1
    ? "PASS: INT(-2.9) = -2"
else
    fail = fail + 1
    ? "FAIL: INT(-2.9) expected -2, got " + STR(r, 10, 0)
endif

r = INT(10)
if r = 10
    pass = pass + 1
    ? "PASS: INT(10) = 10"
else
    fail = fail + 1
    ? "FAIL: INT(10) expected 10, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* LOG() — natural logarithm
* -----------------------------------------------------------
? "=== LOG() ==="
r = LOG(1)
if r = 0
    pass = pass + 1
    ? "PASS: LOG(1) = 0"
else
    fail = fail + 1
    ? "FAIL: LOG(1) expected 0, got " + STR(r, 10, 0)
endif

r = LOG(2.718281828)
if r > 0.999 AND r < 1.001
    pass = pass + 1
    ? "PASS: LOG(e) ~ 1"
else
    fail = fail + 1
    ? "FAIL: LOG(e) expected ~1, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* MAX(a, b)
* -----------------------------------------------------------
? "=== MAX() ==="
r = MAX(3, 7)
if r = 7
    pass = pass + 1
    ? "PASS: MAX(3,7) = 7"
else
    fail = fail + 1
    ? "FAIL: MAX(3,7) expected 7, got " + STR(r, 10, 0)
endif

r = MAX(-1, -5)
if r = -1
    pass = pass + 1
    ? "PASS: MAX(-1,-5) = -1"
else
    fail = fail + 1
    ? "FAIL: MAX(-1,-5) expected -1, got " + STR(r, 10, 0)
endif

r = MAX(4.5, 4.5)
if r = 4.5
    pass = pass + 1
    ? "PASS: MAX(4.5,4.5) = 4.5"
else
    fail = fail + 1
    ? "FAIL: MAX(4.5,4.5) expected 4.5, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* MIN(a, b)
* -----------------------------------------------------------
? "=== MIN() ==="
r = MIN(3, 7)
if r = 3
    pass = pass + 1
    ? "PASS: MIN(3,7) = 3"
else
    fail = fail + 1
    ? "FAIL: MIN(3,7) expected 3, got " + STR(r, 10, 0)
endif

r = MIN(-1, -5)
if r = -5
    pass = pass + 1
    ? "PASS: MIN(-1,-5) = -5"
else
    fail = fail + 1
    ? "FAIL: MIN(-1,-5) expected -5, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* MOD(m, n)
* -----------------------------------------------------------
? "=== MOD() ==="
r = MOD(10, 3)
if r = 1
    pass = pass + 1
    ? "PASS: MOD(10,3) = 1"
else
    fail = fail + 1
    ? "FAIL: MOD(10,3) expected 1, got " + STR(r, 10, 0)
endif

r = MOD(7, 7)
if r = 0
    pass = pass + 1
    ? "PASS: MOD(7,7) = 0"
else
    fail = fail + 1
    ? "FAIL: MOD(7,7) expected 0, got " + STR(r, 10, 0)
endif

r = MOD(17, 5)
if r = 2
    pass = pass + 1
    ? "PASS: MOD(17,5) = 2"
else
    fail = fail + 1
    ? "FAIL: MOD(17,5) expected 2, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* ROUND(n, m)
* -----------------------------------------------------------
? "=== ROUND() ==="
r = ROUND(3.14159, 2)
if r = 3.14
    pass = pass + 1
    ? "PASS: ROUND(3.14159, 2) = 3.14"
else
    fail = fail + 1
    ? "FAIL: ROUND(3.14159, 2) expected 3.14, got " + STR(r, 10, 0)
endif

r = ROUND(2.5, 0)
if r = 3.0
    pass = pass + 1
    ? "PASS: ROUND(2.5, 0) = 3"
else
    fail = fail + 1
    ? "FAIL: ROUND(2.5, 0) expected 3, got " + STR(r, 10, 0)
endif

r = ROUND(99.9, -1)
if r = 100.0
    pass = pass + 1
    ? "PASS: ROUND(99.9, -1) = 100"
else
    fail = fail + 1
    ? "FAIL: ROUND(99.9, -1) expected 100, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* SIGN()
* -----------------------------------------------------------
? "=== SIGN() ==="
r = SIGN(5)
if r = 1
    pass = pass + 1
    ? "PASS: SIGN(5) = 1"
else
    fail = fail + 1
    ? "FAIL: SIGN(5) expected 1, got " + STR(r, 10, 0)
endif

r = SIGN(-3)
if r = -1
    pass = pass + 1
    ? "PASS: SIGN(-3) = -1"
else
    fail = fail + 1
    ? "FAIL: SIGN(-3) expected -1, got " + STR(r, 10, 0)
endif

r = SIGN(0)
if r = 0
    pass = pass + 1
    ? "PASS: SIGN(0) = 0"
else
    fail = fail + 1
    ? "FAIL: SIGN(0) expected 0, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* SQRT()
* -----------------------------------------------------------
? "=== SQRT() ==="
r = SQRT(25)
if r = 5
    pass = pass + 1
    ? "PASS: SQRT(25) = 5"
else
    fail = fail + 1
    ? "FAIL: SQRT(25) expected 5, got " + STR(r, 10, 0)
endif

r = SQRT(2)
if r > 1.414 AND r < 1.415
    pass = pass + 1
    ? "PASS: SQRT(2) ~ 1.414"
else
    fail = fail + 1
    ? "FAIL: SQRT(2) expected ~1.414, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* STR(n, length, decimals)
* -----------------------------------------------------------
? "=== STR() ==="
s = STR(5, 10, 0)
if TRIM(s) = "5"
    pass = pass + 1
    ? "PASS: STR(5, 10, 0) = '         5'"
else
    fail = fail + 1
    ? "FAIL: STR(5,10,0) expected '5' trimmed, got '" + s + "'"
endif

s = STR(42, 5, 2)
if TRIM(s) = "42.00"
    pass = pass + 1
    ? "PASS: STR(42, 5, 2) = ' 42.00'"
else
    fail = fail + 1
    ? "FAIL: STR(42,5,2) expected '42.00' trimmed, got '" + s + "'"
endif

s = STR(3.14, 8, 3)
if TRIM(s) = "3.140"
    pass = pass + 1
    ? "PASS: STR(3.14, 8, 3) = '  3.140'"
else
    fail = fail + 1
    ? "FAIL: STR(3.14,8,3) expected '3.140' trimmed, got '" + s + "'"
endif

* -----------------------------------------------------------
* VAL()
* -----------------------------------------------------------
? "=== VAL() ==="
r = VAL("123")
if r = 123
    pass = pass + 1
    ? "PASS: VAL('123') = 123"
else
    fail = fail + 1
    ? "FAIL: VAL('123') expected 123, got " + STR(r, 10, 0)
endif

r = VAL("  45.67")
if r = 45.67
    pass = pass + 1
    ? "PASS: VAL('  45.67') = 45.67"
else
    fail = fail + 1
    ? "FAIL: VAL('  45.67') expected 45.67, got " + STR(r, 10, 0)
endif

r = VAL("-8.5")
if r = -8.5
    pass = pass + 1
    ? "PASS: VAL('-8.5') = -8.5"
else
    fail = fail + 1
    ? "FAIL: VAL('-8.5') expected -8.5, got " + STR(r, 10, 0)
endif

* VAL stops at first non-numeric
r = VAL("123abc")
if r = 123
    pass = pass + 1
    ? "PASS: VAL('123abc') = 123 (stops at non-numeric)"
else
    fail = fail + 1
    ? "FAIL: VAL('123abc') expected 123, got " + STR(r, 10, 0)
endif

r = VAL("abc")
if r = 0
    pass = pass + 1
    ? "PASS: VAL('abc') = 0"
else
    fail = fail + 1
    ? "FAIL: VAL('abc') expected 0, got " + STR(r, 10, 0)
endif

* -----------------------------------------------------------
* Summary
* -----------------------------------------------------------
? "=============================="
? "Numeric tests: " + STR(pass, 5, 0) + " passed, " + STR(fail, 5, 0) + " failed"
? "=============================="
