* stress_workarea.prg — Test multi-work-area field access via ALIAS->FIELD

* --- Test 1: Open two databases in different areas ---
SELECT 1
USE books
SELECT 2
USE books_memo

* --- Test 2: Read from area A (books) ---
GO TOP
? "Test 2: A->TITULO ="
? A->TITULO

* --- Test 3: Read from area B (books_memo) ---
GO TOP
? "Test 3: B->TITULO ="
? B->TITULO

* --- Test 4: Read memo field from B ---
? "Test 4: B->COMMENTS (first 30 chars) ="
? SUBSTR(B->COMMENTS, 1, 30)

* --- Test 5: Compare fields across areas ---
? "Test 5: A->TITULO = B->TITULO (both GO TOP) ="
result = A->TITULO = B->TITULO
? result

* --- Test 6: $ operator across areas ---
? "Test 6: 'PROFETA' $ B->TITULO ="
result = 'PROFETA' $ B->TITULO
? result

* --- Test 7: SELECT and verify current area ---
SELECT 1
? "Test 7: After SELECT 1, A->TITULO ="
? A->TITULO

SELECT 2
? "Test 8: After SELECT 2, B->TITULO ="
? B->TITULO

* --- Test 9: DBF name as alias ---
? "Test 9: books_memo->TITULO (using DBF name) ="
? books_memo->TITULO

* --- Test 10: Numeric field from other area ---
GO TOP
? "Test 10: A->AN_EDICION ="
? A->AN_EDICION

CLOSE ALL

*** END WORKAREA TESTS ***
