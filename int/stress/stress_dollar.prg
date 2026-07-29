* --- $ (substring containment) operator tests ---

* Test 1: literal in literal (found)
? "Test 1: 'LAW' $ 'LAWRENCE OF ARABIA' ="
result = 'LAW' $ 'LAWRENCE OF ARABIA'
? result

* Test 2: literal in literal (not found)
? "Test 2: 'XYZ' $ 'LAWRENCE OF ARABIA' ="
result = 'XYZ' $ 'LAWRENCE OF ARABIA'
? result

* Test 3: variable needle in literal haystack
y = 'LAW'
? "Test 3: y='LAW' ; y $ 'LAWRENCE OF ARABIA' ="
result = y $ 'LAWRENCE OF ARABIA'
? result

* Test 4: literal needle in variable haystack
y = 'LAWRENCE OF ARABIA'
? "Test 4: y='LAWRENCE OF ARABIA' ; 'LAW' $ y ="
result = 'LAW' $ y
? result

* Test 5: variable needle in variable haystack
needle = 'LAW'
haystack = 'LAWRENCE OF ARABIA'
? "Test 5: needle $ haystack ="
result = needle $ haystack
? result

* Test 6: case insensitive
? "Test 6: 'law' $ 'LAWRENCE OF ARABIA' (case insensitive) ="
result = 'law' $ 'LAWRENCE OF ARABIA'
? result

* Test 7: empty needle (should be .F.)
? "Test 7: '' $ 'something' (empty needle) ="
result = '' $ 'something'
? result

* Test 8: bracket-delimited string
? "Test 8: [LAW] $ [LAWRENCE OF ARABIA] ="
result = [LAW] $ [LAWRENCE OF ARABIA]
? result

* Test 9: combined with .AND.
? "Test 9: 'LAW' $ 'LAWRENCE' .AND. 'ARA' $ 'ARABIA' ="
result = 'LAW' $ 'LAWRENCE' .AND. 'ARA' $ 'ARABIA'
? result

* Test 10: combined with .OR.
? "Test 10: 'XYZ' $ 'ABC' .OR. 'BC' $ 'ABC' ="
result = 'XYZ' $ 'ABC' .OR. 'BC' $ 'ABC'
? result

* Test 11: with database field
USE books
? "Test 11: 'LAW' $ A->TITULO (first record) ="
result = 'LAW' $ A->TITULO
? result
CLOSE ALL

* Test 12: numeric coerced to string
? "Test 12: 123 $ 123456 (numeric coerced) ="
result = 123 $ 123456
? result

* Test 13: .NOT. with $
? "Test 13: .NOT. ('XYZ' $ 'ABC') ="
result = .NOT. ('XYZ' $ 'ABC')
? result

* Test 14: $ in IF condition
? "Test 14: IF with $"
IF 'LAW' $ 'LAWRENCE OF ARABIA'
    ? "FOUND"
ELSE
    ? "NOT FOUND"
ENDIF

* Test 15: $ with LOCATE
USE books
? "Test 15: LOCATE FOR 'LAW' $ TITULO"
LOCATE FOR 'LAW' $ TITULO
? FOUND()
? RECNO()
CLOSE ALL

*** END DOLLAR TESTS ***
