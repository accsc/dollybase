* benchmark_products.prg - Benchmark SEEK, LOCATE, SKIP on 100k products
* Usage: DO benchmark_products
* Requires: products.dbf with data, products_barcode.ndx, products_category.ndx

SET TALK OFF

? "=== Product Database Benchmark ==="
? "Start: " + TIME()
? ""

SELECT 1
USE products
? "Records: " + STR(RECCOUNT())
? ""

* --- Test 1: Sequential scan with SKIP ---
? "--- Test 1: Sequential SKIP through all records ---"
t1_start = TIME()
GO TOP
skip_count = 0
DO WHILE .NOT. EOF()
    skip_count = skip_count + 1
    SKIP
ENDDO
t1_end = TIME()
? "  Skipped " + STR(skip_count) + " records"
? "  Time: " + t1_start + " -> " + t1_end
? ""

* --- Test 2: LOCATE by barcode (linear scan, no index) ---
? "--- Test 2: LOCATE barcode (no index) ---"
USE products
SET INDEX TO
GO TOP
t2_start = TIME()
LOCATE FOR BARCODE = "84123456700001"
t2_end = TIME()
found2 = .NOT. EOF()
? "  Found: " + IIF(found2, "YES", "NO")
IF found2
    ? "  Barcode: " + BARCODE
    ? "  Name: " + NAME
ENDIF
? "  Time: " + t2_start + " -> " + t2_end
? ""

* --- Test 3: LOCATE by category (linear scan) ---
? "--- Test 3: LOCATE category=Dairy (no index) ---"
GO TOP
t3_start = TIME()
LOCATE FOR CATEGORY = "Dairy"
t3_end = TIME()
found3 = .NOT. EOF()
? "  Found: " + IIF(found3, "YES", "NO")
IF found3
    ? "  Category: " + CATEGORY
ENDIF
? "  Time: " + t3_start + " -> " + t3_end
? ""

* --- Test 4: COUNT by category ---
? "--- Test 4: COUNT records per category (no index) ---"
GO TOP
t4_start = TIME()
dairy_count = 0
bakery_count = 0
beverage_count = 0
produce_count = 0
other_count = 0
DO WHILE .NOT. EOF()
    DO CASE
        CASE CATEGORY = "Dairy"
            dairy_count = dairy_count + 1
        CASE CATEGORY = "Bakery"
            bakery_count = bakery_count + 1
        CASE CATEGORY = "Beverages"
            beverage_count = beverage_count + 1
        CASE CATEGORY = "Produce"
            produce_count = produce_count + 1
        OTHERWISE
            other_count = other_count + 1
    ENDCASE
    SKIP
ENDDO
t4_end = TIME()
? "  Dairy: " + STR(dairy_count)
? "  Bakery: " + STR(bakery_count)
? "  Beverages: " + STR(beverage_count)
? "  Produce: " + STR(produce_count)
? "  Other: " + STR(other_count)
? "  Time: " + t4_start + " -> " + t4_end
? ""

* --- Test 5: SEEK with barcode index ---
? "--- Test 5: SEEK barcode (with index) ---"
SET INDEX TO products_barcode
t5_start = TIME()
SEEK "84123456700001"
t5_end = TIME()
found5 = .NOT. EOF()
? "  Found: " + IIF(found5, "YES", "NO")
IF found5
    ? "  Barcode: " + BARCODE
    ? "  Name: " + NAME
ENDIF
? "  Time: " + t5_start + " -> " + t5_end
? ""

* --- Test 6: Multiple SEEKs with barcode index ---
? "--- Test 6: 100 sequential SEEKs (barcode index) ---"
t6_start = TIME()
seek_found = 0
seek_num = 0
DO WHILE seek_num < 100
    seek_num = seek_num + 1
    sbc = ALLTRIM(STR(seek_num))
    sbc_pad = 5 - LEN(sbc)
    IF sbc_pad < 0
        sbc_pad = 0
    ENDIF
    seek_bc = "841234567" + REPLICATE("0", sbc_pad) + sbc
    SEEK seek_bc
    IF .NOT. EOF()
        seek_found = seek_found + 1
    ENDIF
ENDDO
t6_end = TIME()
? "  Found " + STR(seek_found) + " / 100 seeks"
? "  Time: " + t6_start + " -> " + t6_end
? ""

* --- Test 7: SEEK with category index ---
? "--- Test 7: SEEK category (with index) ---"
SET INDEX TO products_category
t7_start = TIME()
SEEK "Dairy"
t7_end = TIME()
found7 = .NOT. EOF()
? "  Found: " + IIF(found7, "YES", "NO")
IF found7
    ? "  Category: " + CATEGORY
    ? "  Name: " + NAME
ENDIF
? "  Time: " + t7_start + " -> " + t7_end
? ""

* --- Test 8: COUNT with category index (SKIP within index range) ---
? "--- Test 8: COUNT Dairy with index SKIP ---"
SET INDEX TO products_category
SEEK "Dairy"
t8_start = TIME()
idx_count = 0
DO WHILE .NOT. EOF() .AND. CATEGORY = "Dairy"
    idx_count = idx_count + 1
    SKIP
ENDDO
t8_end = TIME()
? "  Dairy count via index: " + STR(idx_count)
? "  (vs linear scan: " + STR(dairy_count) + ")"
? "  Time: " + t8_start + " -> " + t8_end
? ""

* --- Test 9: GO TOP + SKIP random positions ---
? "--- Test 9: 50 random GO TOP + SKIP ---"
SET INDEX TO
t9_start = TIME()
goseek_num = 0
SEED = 42
DO WHILE goseek_num < 50
    goseek_num = goseek_num + 1
    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    target = MOD(INT(SEED / 65536), RECCOUNT()) + 1
    GO target
ENDDO
t9_end = TIME()
? "  Completed 50 random GO operations"
? "  Time: " + t9_start + " -> " + t9_end
? ""

* --- Summary ---
? "=== Benchmark Complete ==="
? "End: " + TIME()

CLOSE DATABASES
RETURN
