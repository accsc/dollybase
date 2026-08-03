* test_save_restore.prg — Test SAVE TO / RESTORE FROM
* Tests: basic save/restore, specific vars, ADDITIVE, LIKE, EXCEPT

* --- Test 1: Basic save and restore ---
x = 42
y = "hello"
z = .T.
d = {^2024-06-15}

SAVE TO test1

RESTORE FROM test1

? "Test 1 - Basic save/restore:"
? "  x=", x, " (expect 42)"
? "  y=", y, " (expect hello)"
? "  z=", z, " (expect .T.)"

* --- Test 2: Save specific variables ---
a = 100
b = 200
c = 300

SAVE TO test2 a, b

RESTORE FROM test2

? "Test 2 - Save specific vars:"
? "  a=", a, " (expect 100)"
? "  b=", b, " (expect 200)"
? "  c exists=", TYPE('c'), " (expect U - not restored)"

* --- Test 3: ADDITIVE restore ---
m1 = "original"
m2 = "to_keep"

SAVE TO test3

m1 = "changed"
m3 = "new_var"

RESTORE FROM test3 ADDITIVE

? "Test 3 - ADDITIVE restore:"
? "  m1=", m1, " (expect original - restored)"
? "  m2=", m2, " (expect to_keep - restored)"
? "  m3=", m3, " (expect new_var - kept)"

* --- Test 4: SAVE TO ALL LIKE ---
alpha1 = 1
alpha2 = 2
beta1 = 10
beta2 = 20

SAVE TO test4 ALL LIKE "alpha*"

RESTORE FROM test4

? "Test 4 - SAVE ALL LIKE:"
? "  alpha1=", alpha1, " (expect 1)"
? "  alpha2=", alpha2, " (expect 2)"
? "  beta1 exists=", TYPE('beta1'), " (expect U)"

* --- Test 5: SAVE TO ALL EXCEPT ---
var_a = 1
var_b = 2
other = 99

SAVE TO test5 ALL EXCEPT "other"

RESTORE FROM test5

? "Test 5 - SAVE ALL EXCEPT:"
? "  var_a=", var_a, " (expect 1)"
? "  var_b=", var_b, " (expect 2)"
? "  other exists=", TYPE('other'), " (expect U)"

? "All tests complete."
