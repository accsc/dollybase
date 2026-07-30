* test_open.prg - Verify databases open correctly

SET TALK OFF

DO sm_init

* (databases opened via sm_init with USE "data/...")

? "Products: " + LTRIM(STR(A->RECCOUNT()))
SELECT 2
? "Providers: " + LTRIM(STR(RECCOUNT()))
SELECT 3
? "Sales: " + LTRIM(STR(RECCOUNT()))
SELECT 4
? "SalesItems: " + LTRIM(STR(RECCOUNT()))
SELECT 5
? "Orders: " + LTRIM(STR(RECCOUNT()))
SELECT 1

* Show first product
GO TOP
? ""
? "First product: [" + A->BARCODE + "] " + A->NAME
? "Category: " + A->CATEGORY + "  Price: " + LTRIM(STR(A->PRICE, 8, 2)) + "  Stock: " + LTRIM(STR(A->STOCK))

CLOSE DATABASES
RETURN
