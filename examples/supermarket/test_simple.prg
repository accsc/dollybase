* test_simple.prg - Minimal test

? "START"
SELECT 1
USE "data/products"
? "Products opened: " + LTRIM(STR(RECCOUNT()))
? "First: " + A->BARCODE
USE
? "END"
RETURN
