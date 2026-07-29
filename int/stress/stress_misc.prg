* ============================================================
* stress_misc.prg — OS(), VERSION(), TYPE() stress test
* ============================================================

pass = 0
fail = 0

* -----------------------------------------------------------
* OS()
* -----------------------------------------------------------
? "=== OS() ==="
os = OS()
if LEN(os) > 0
    pass = pass + 1
    ? "PASS: OS() = '" + os + "'"
else
    fail = fail + 1
    ? "FAIL: OS() returned empty string"
endif

* -----------------------------------------------------------
* VERSION()
* -----------------------------------------------------------
? "=== VERSION() ==="
v = VERSION()
if v = "0.5"
    pass = pass + 1
    ? "PASS: VERSION() = '0.5'"
else
    fail = fail + 1
    ? "FAIL: VERSION() expected '0.5', got '" + v + "'"
endif

* -----------------------------------------------------------
* TYPE() — with variable name lookup
* -----------------------------------------------------------
? "=== TYPE() ==="
numVar = 42
strVar = "hello"
logVar = .T.
datVar = CTOD("2026-07-29")

t = TYPE("numVar")
if t = "N"
    pass = pass + 1
    ? "PASS: TYPE('numVar') = 'N'"
else
    fail = fail + 1
    ? "FAIL: TYPE('numVar') expected 'N', got '" + t + "'"
endif

t = TYPE("strVar")
if t = "C"
    pass = pass + 1
    ? "PASS: TYPE('strVar') = 'C'"
else
    fail = fail + 1
    ? "FAIL: TYPE('strVar') expected 'C', got '" + t + "'"
endif

t = TYPE("logVar")
if t = "L"
    pass = pass + 1
    ? "PASS: TYPE('logVar') = 'L'"
else
    fail = fail + 1
    ? "FAIL: TYPE('logVar') expected 'L', got '" + t + "'"
endif

t = TYPE("datVar")
if t = "D"
    pass = pass + 1
    ? "PASS: TYPE('datVar') = 'D'"
else
    fail = fail + 1
    ? "FAIL: TYPE('datVar') expected 'D', got '" + t + "'"
endif

t = TYPE("doesNotExist")
if t = "U"
    pass = pass + 1
    ? "PASS: TYPE('doesNotExist') = 'U'"
else
    fail = fail + 1
    ? "FAIL: TYPE('doesNotExist') expected 'U', got '" + t + "'"
endif

* TYPE() with direct value (fallback)
t = TYPE("hello")
if t = "C"
    pass = pass + 1
    ? "PASS: TYPE('hello' literal) = 'C'"
else
    fail = fail + 1
    ? "FAIL: TYPE('hello') expected 'C', got '" + t + "'"
endif

* -----------------------------------------------------------
* Summary
* -----------------------------------------------------------
? "=============================="
? "Misc tests: " + STR(pass, 5, 0) + " passed, " + STR(fail, 5, 0) + " failed"
? "=============================="
