* test_stderr.prg - Write to stderr to verify execution

? "START"
SELECT 1
USE "data/products"
? "Products: " + LTRIM(STR(RECCOUNT()))
GO TOP
? "First: " + A->BARCODE
USE
? "END"
QUIT
