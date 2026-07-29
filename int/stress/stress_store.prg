* stress_store.prg — STORE <expr> TO <var1>, <var2>, ... tests
CLEAR

* Test 1: STORE to single variable
STORE 42 TO x
? "Test 1: x=" + LTRIM(STR(x))

* Test 2: STORE to multiple variables
STORE 0 TO a, b, c
? "Test 2: a=" + LTRIM(STR(a)) + " b=" + LTRIM(STR(b)) + " c=" + LTRIM(STR(c))

* Test 3: STORE string to multiple variables
STORE "hello" TO s1, s2
? "Test 3: s1=" + s1 + " s2=" + s2

* Test 4: STORE expression
STORE 10 + 5 TO r1, r2
? "Test 4: r1=" + LTRIM(STR(r1)) + " r2=" + LTRIM(STR(r2))

* Test 5: STORE logical
STORE .T. TO flag1, flag2
? "Test 5: flag1=" + LTRIM(STR(flag1)) + " flag2=" + LTRIM(STR(flag2))

WAIT "Press any key to finish STORE tests"
