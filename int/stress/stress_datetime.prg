* ============================================================
* stress_datetime.prg — Date/time functions stress test
* Tests: CDOW, CMONTH, CTOD, DATE, DAY, DOW, DTOC, MONTH, YEAR
* ============================================================

pass = 0
fail = 0

* -----------------------------------------------------------
* DATE() — returns current system date
* -----------------------------------------------------------
? "=== DATE() ==="
today = DATE()
? "Today: " + DTOC(today)
y = YEAR(today)
m = MONTH(today)
d = DAY(today)
if y >= 2026 AND y <= 2030
    pass = pass + 1
    ? "PASS: YEAR(DATE()) = " + STR(y, 5, 0)
else
    fail = fail + 1
    ? "FAIL: YEAR(DATE()) expected ~2026+, got " + STR(y, 5, 0)
endif

if m >= 1 AND m <= 12
    pass = pass + 1
    ? "PASS: MONTH(DATE()) = " + STR(m, 3, 0)
else
    fail = fail + 1
    ? "FAIL: MONTH(DATE()) expected 1-12, got " + STR(m, 3, 0)
endif

if d >= 1 AND d <= 31
    pass = pass + 1
    ? "PASS: DAY(DATE()) = " + STR(d, 3, 0)
else
    fail = fail + 1
    ? "FAIL: DAY(DATE()) expected 1-31, got " + STR(d, 3, 0)
endif

* -----------------------------------------------------------
* DTOC(date) — date to string
* -----------------------------------------------------------
? "=== DTOC() ==="
s = DTOC(today)
? "DTOC(DATE()) = " + s
if LEN(s) = 10
    pass = pass + 1
    ? "PASS: DTOC returns 10-char string"
else
    fail = fail + 1
    ? "FAIL: DTOC length expected 10, got " + STR(LEN(s), 3, 0)
endif

* -----------------------------------------------------------
* CTOD(string) — string to date
* -----------------------------------------------------------
? "=== CTOD() ==="
dt = CTOD("2026-07-29")
if YEAR(dt) = 2026 AND MONTH(dt) = 7 AND DAY(dt) = 29
    pass = pass + 1
    ? "PASS: CTOD('2026-07-29') parsed correctly"
else
    fail = fail + 1
    ? "FAIL: CTOD('2026-07-29') got " + DTOC(dt)
endif

dt = CTOD("01/15/1990")
if YEAR(dt) = 1990 AND MONTH(dt) = 1 AND DAY(dt) = 15
    pass = pass + 1
    ? "PASS: CTOD('01/15/1990') parsed correctly"
else
    fail = fail + 1
    ? "FAIL: CTOD('01/15/1990') got " + DTOC(dt)
endif

* -----------------------------------------------------------
* DAY(date)
* -----------------------------------------------------------
? "=== DAY() ==="
dt = CTOD("2026-03-15")
r = DAY(dt)
if r = 15
    pass = pass + 1
    ? "PASS: DAY(CTOD('2026-03-15')) = 15"
else
    fail = fail + 1
    ? "FAIL: DAY expected 15, got " + STR(r, 3, 0)
endif

* -----------------------------------------------------------
* MONTH(date)
* -----------------------------------------------------------
? "=== MONTH() ==="
dt = CTOD("2026-11-03")
r = MONTH(dt)
if r = 11
    pass = pass + 1
    ? "PASS: MONTH(CTOD('2026-11-03')) = 11"
else
    fail = fail + 1
    ? "FAIL: MONTH expected 11, got " + STR(r, 3, 0)
endif

* -----------------------------------------------------------
* YEAR(date)
* -----------------------------------------------------------
? "=== YEAR() ==="
dt = CTOD("1985-06-20")
r = YEAR(dt)
if r = 1985
    pass = pass + 1
    ? "PASS: YEAR(CTOD('1985-06-20')) = 1985"
else
    fail = fail + 1
    ? "FAIL: YEAR expected 1985, got " + STR(r, 5, 0)
endif

* YEAR with SET CENTURY OFF
SET CENTURY OFF
dt = CTOD("1985-06-20")
r = YEAR(dt)
if r = 85
    pass = pass + 1
    ? "PASS: YEAR with CENTURY OFF = 85"
else
    fail = fail + 1
    ? "FAIL: YEAR with CENTURY OFF expected 85, got " + STR(r, 5, 0)
endif

SET CENTURY ON
dt = CTOD("1985-06-20")
r = YEAR(dt)
if r = 1985
    pass = pass + 1
    ? "PASS: YEAR with CENTURY ON = 1985"
else
    fail = fail + 1
    ? "FAIL: YEAR with CENTURY ON expected 1985, got " + STR(r, 5, 0)
endif

* -----------------------------------------------------------
* DOW(date) — day of week (1=Sunday .. 7=Saturday)
* -----------------------------------------------------------
? "=== DOW() ==="
* 2026-07-29 is a Wednesday => DOW = 4
dt = CTOD("2026-07-29")
r = DOW(dt)
if r = 4
    pass = pass + 1
    ? "PASS: DOW(CTOD('2026-07-29')) = 4 (Wednesday)"
else
    fail = fail + 1
    ? "FAIL: DOW expected 4, got " + STR(r, 3, 0)
endif

* 2026-07-26 is a Sunday => DOW = 1
dt = CTOD("2026-07-26")
r = DOW(dt)
if r = 1
    pass = pass + 1
    ? "PASS: DOW(CTOD('2026-07-26')) = 1 (Sunday)"
else
    fail = fail + 1
    ? "FAIL: DOW expected 1, got " + STR(r, 3, 0)
endif

* 2026-07-25 is a Saturday => DOW = 7
dt = CTOD("2026-07-25")
r = DOW(dt)
if r = 7
    pass = pass + 1
    ? "PASS: DOW(CTOD('2026-07-25')) = 7 (Saturday)"
else
    fail = fail + 1
    ? "FAIL: DOW expected 7, got " + STR(r, 3, 0)
endif

* -----------------------------------------------------------
* CDOW(date) — full day name
* -----------------------------------------------------------
? "=== CDOW() ==="
dt = CTOD("2026-07-29")
s = CDOW(dt)
if s = "Wednesday"
    pass = pass + 1
    ? "PASS: CDOW(CTOD('2026-07-29')) = 'Wednesday'"
else
    fail = fail + 1
    ? "FAIL: CDOW expected 'Wednesday', got '" + s + "'"
endif

dt = CTOD("2026-07-26")
s = CDOW(dt)
if s = "Sunday"
    pass = pass + 1
    ? "PASS: CDOW(CTOD('2026-07-26')) = 'Sunday'"
else
    fail = fail + 1
    ? "FAIL: CDOW expected 'Sunday', got '" + s + "'"
endif

dt = CTOD("2026-07-25")
s = CDOW(dt)
if s = "Saturday"
    pass = pass + 1
    ? "PASS: CDOW(CTOD('2026-07-25')) = 'Saturday'"
else
    fail = fail + 1
    ? "FAIL: CDOW expected 'Saturday', got '" + s + "'"
endif

* -----------------------------------------------------------
* CMONTH(date) — full month name
* -----------------------------------------------------------
? "=== CMONTH() ==="
dt = CTOD("2026-01-15")
s = CMONTH(dt)
if s = "January"
    pass = pass + 1
    ? "PASS: CMONTH(CTOD('2026-01-15')) = 'January'"
else
    fail = fail + 1
    ? "FAIL: CMONTH expected 'January', got '" + s + "'"
endif

dt = CTOD("2026-12-25")
s = CMONTH(dt)
if s = "December"
    pass = pass + 1
    ? "PASS: CMONTH(CTOD('2026-12-25')) = 'December'"
else
    fail = fail + 1
    ? "FAIL: CMONTH expected 'December', got '" + s + "'"
endif

dt = CTOD("2026-07-04")
s = CMONTH(dt)
if s = "July"
    pass = pass + 1
    ? "PASS: CMONTH(CTOD('2026-07-04')) = 'July'"
else
    fail = fail + 1
    ? "FAIL: CMONTH expected 'July', got '" + s + "'"
endif

* -----------------------------------------------------------
* Summary
* -----------------------------------------------------------
? "=============================="
? "Date/Time tests: " + STR(pass, 5, 0) + " passed, " + STR(fail, 5, 0) + " failed"
? "=============================="
