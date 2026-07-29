* Verify $ operator logic via IF/ELSE branches that set known variables

* Test 1: literal in literal (found)
IF 'LAW'$'LAWRENCE OF ARABIA'
    t1 = "PASS"
ELSE
    t1 = "FAIL"
ENDIF

* Test 2: literal in literal (not found)
IF 'XYZ'$'LAWRENCE OF ARABIA'
    t2 = "FAIL"
ELSE
    t2 = "PASS"
ENDIF

* Test 3: variable needle in literal haystack
y = 'LAW'
IF y$'LAWRENCE OF ARABIA'
    t3 = "PASS"
ELSE
    t3 = "FAIL"
ENDIF

* Test 4: literal needle in variable haystack
y = 'LAWRENCE OF ARABIA'
IF 'LAW'$y
    t4 = "PASS"
ELSE
    t4 = "FAIL"
ENDIF

* Test 5: variable needle in variable haystack
needle = 'LAW'
haystack = 'LAWRENCE OF ARABIA'
IF needle$haystack
    t5 = "PASS"
ELSE
    t5 = "FAIL"
ENDIF

* Test 6: case insensitive
IF 'law'$'LAWRENCE OF ARABIA'
    t6 = "PASS"
ELSE
    t6 = "FAIL"
ENDIF

* Test 7: empty needle (should be .F.)
IF ''$'something'
    t7 = "FAIL"
ELSE
    t7 = "PASS"
ENDIF

* Test 8: bracket-delimited string
IF [LAW]$[LAWRENCE OF ARABIA]
    t8 = "PASS"
ELSE
    t8 = "FAIL"
ENDIF

* Test 9: combined with .AND.
IF 'LAW'$'LAWRENCE' .AND. 'ARA'$'ARABIA'
    t9 = "PASS"
ELSE
    t9 = "FAIL"
ENDIF

* Test 10: combined with .OR.
IF 'XYZ' $ 'ABC' .OR. 'BC' $ 'ABC'
    t10 = "PASS"
ELSE
    t10 = "FAIL"
ENDIF

* Test 11: numeric coerced to string
IF 123 $ 123456
    t11 = "PASS"
ELSE
    t11 = "FAIL"
ENDIF

* Test 12: .NOT. with $
IF .NOT. ('XYZ' $ 'ABC')
    t12 = "PASS"
ELSE
    t12 = "FAIL"
ENDIF

* Test 13: $ with LOCATE
USE books
LOCATE FOR 'LAW' $ TITULO
IF FOUND()
    t13 = "PASS"
ELSE
    t13 = "FAIL"
ENDIF
CLOSE ALL

? "RESULTS"
* Print results
? "T1:", t1
? "T2:", t2
? "T3:", t3
? "T4:", t4
? "T5:", t5
? "T6:", t6
? "T7:", t7
? "T8:", t8
? "T9:", t9
? "T10:", t10
? "T11:", t11
? "T12:", t12
? "T13:", t13

WAIT
