* benchmark_products.prg - Benchmark SEEK (NDX index) vs LOCATE (linear scan)
* Usage: DO benchmark_products
* Requires: products.dbf with data, products_barcode.ndx, products_category.ndx

SET TALK OFF

? "=== SEEK (NDX) vs LOCATE Benchmark ==="
? "Date: " + DTOC(DATE()) + "  Time: " + TIME()
? ""

SELECT 1
USE products
total = RECCOUNT()
? "Records: " + STR(total)
? ""

* --- Collect actual barcodes from the data for testing ---
* Pick 10 barcodes spread across the file (hardcoded vars, no arrays)
num_test_keys = 10
m_step = INT(total / num_test_keys)
IF m_step < 1
    m_step = 1
ENDIF
k = 0
GO TOP
rec = 0
DO WHILE k < num_test_keys .AND. .NOT. EOF()
    IF rec % m_step = 0
        k = k + 1
        DO CASE
            CASE k = 1
                test_bc1 = BARCODE
            CASE k = 2
                test_bc2 = BARCODE
            CASE k = 3
                test_bc3 = BARCODE
            CASE k = 4
                test_bc4 = BARCODE
            CASE k = 5
                test_bc5 = BARCODE
            CASE k = 6
                test_bc6 = BARCODE
            CASE k = 7
                test_bc7 = BARCODE
            CASE k = 8
                test_bc8 = BARCODE
            CASE k = 9
                test_bc9 = BARCODE
            CASE k = 10
                test_bc10 = BARCODE
        ENDCASE
    ENDIF
    rec = rec + 1
    SKIP
ENDDO
? "Test keys collected: " + STR(k)
? ""

* --- Warm-up ---
SET INDEX TO products_barcode
SEEK test_bc1
SET INDEX TO
GO TOP
LOCATE FOR BARCODE = test_bc1
SET INDEX TO products_barcode
GO TOP

* ========================================================================
* Test 1: Single SEEK vs single LOCATE on barcode (unique key, found)
* ========================================================================
? "--- Test 1: Single lookup by barcode (found) ---"
iterations = 5
test_bc = test_bc1

SET INDEX TO products_barcode
t_seek = 0
iter = 0
DO WHILE iter < iterations
    iter = iter + 1
    GO TOP
    t0 = SECONDS()
    SEEK test_bc
    t1 = SECONDS()
    t_seek = t_seek + (t1 - t0)
ENDDO
avg_seek = t_seek / iterations
? "  SEEK (with NDX):     " + STR(t_seek, 8, 4) + "s  (" + STR(avg_seek * 1000, 6, 2) + " ms/op)"

SET INDEX TO
t_locate = 0
iter = 0
DO WHILE iter < iterations
    iter = iter + 1
    GO TOP
    t0 = SECONDS()
    LOCATE FOR BARCODE = test_bc
    t1 = SECONDS()
    t_locate = t_locate + (t1 - t0)
ENDDO
avg_locate = t_locate / iterations
? "  LOCATE (no index):   " + STR(t_locate, 8, 4) + "s  (" + STR(avg_locate * 1000, 6, 2) + " ms/op)"
IF t_seek > 0
    ? "  Speedup:           " + STR(t_locate / t_seek, 6, 1) + "x"
ENDIF
? ""

* ========================================================================
* Test 2: SEEK vs LOCATE on barcode (unique key, NOT found)
* ========================================================================
? "--- Test 2: Single lookup by barcode (not found) ---"
miss_bc = "9999999999999"

SET INDEX TO products_barcode
t_seek = 0
iter = 0
DO WHILE iter < iterations
    iter = iter + 1
    GO TOP
    t0 = SECONDS()
    SEEK miss_bc
    t1 = SECONDS()
    t_seek = t_seek + (t1 - t0)
ENDDO
avg_seek = t_seek / iterations
? "  SEEK (with NDX):     " + STR(t_seek, 8, 4) + "s  (" + STR(avg_seek * 1000, 6, 2) + " ms/op)"

SET INDEX TO
t_locate = 0
iter = 0
DO WHILE iter < iterations
    iter = iter + 1
    GO TOP
    t0 = SECONDS()
    LOCATE FOR BARCODE = miss_bc
    t1 = SECONDS()
    t_locate = t_locate + (t1 - t0)
ENDDO
avg_locate = t_locate / iterations
? "  LOCATE (no index):   " + STR(t_locate, 8, 4) + "s  (" + STR(avg_locate * 1000, 6, 2) + " ms/op)"
IF t_seek > 0
    ? "  Speedup:           " + STR(t_locate / t_seek, 6, 1) + "x"
ENDIF
? ""

* ========================================================================
* Test 3: Batch SEEK vs LOCATE on barcodes (actual data keys)
* ========================================================================
? "--- Test 3: " + STR(num_test_keys) + " barcode lookups (actual data keys) ---"

SET INDEX TO products_barcode
t_seek = 0
seek_hits = 0
GO TOP
t0 = SECONDS()
SEEK test_bc1
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc2
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc3
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc4
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc5
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc6
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc7
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc8
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc9
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
SEEK test_bc10
t1 = SECONDS()
t_seek = t_seek + (t1 - t0)
IF .NOT. EOF()
    seek_hits = seek_hits + 1
ENDIF
? "  SEEK (with NDX):     " + STR(t_seek, 8, 4) + "s  (" + STR(t_seek / num_test_keys * 1000, 6, 2) + " ms/op)  hits=" + STR(seek_hits)

SET INDEX TO
t_locate = 0
locate_hits = 0
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc1
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc2
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc3
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc4
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc5
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc6
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc7
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc8
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc9
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
GO TOP
t0 = SECONDS()
LOCATE FOR BARCODE = test_bc10
t1 = SECONDS()
t_locate = t_locate + (t1 - t0)
IF .NOT. EOF()
    locate_hits = locate_hits + 1
ENDIF
? "  LOCATE (no index):   " + STR(t_locate, 8, 4) + "s  (" + STR(t_locate / num_test_keys * 1000, 6, 2) + " ms/op)  hits=" + STR(locate_hits)
IF t_seek > 0
    ? "  Speedup:           " + STR(t_locate / t_seek, 6, 1) + "x"
ENDIF
? ""

* ========================================================================
* Test 4: SEEK vs LOCATE on category (non-unique key, found)
* ========================================================================
? "--- Test 4: Single lookup by category=Dairy (found) ---"
iterations = 5

SET INDEX TO products_category
t_seek = 0
iter = 0
DO WHILE iter < iterations
    iter = iter + 1
    GO TOP
    t0 = SECONDS()
    SEEK "Dairy"
    t1 = SECONDS()
    t_seek = t_seek + (t1 - t0)
ENDDO
? "  SEEK (with NDX):     " + STR(t_seek, 8, 4) + "s  (" + STR(t_seek / iterations * 1000, 6, 2) + " ms/op)"

SET INDEX TO
t_locate = 0
iter = 0
DO WHILE iter < iterations
    iter = iter + 1
    GO TOP
    t0 = SECONDS()
    LOCATE FOR CATEGORY = "Dairy"
    t1 = SECONDS()
    t_locate = t_locate + (t1 - t0)
ENDDO
? "  LOCATE (no index):   " + STR(t_locate, 8, 4) + "s  (" + STR(t_locate / iterations * 1000, 6, 2) + " ms/op)"
IF t_seek > 0
    ? "  Speedup:           " + STR(t_locate / t_seek, 6, 1) + "x"
ENDIF
? ""

* ========================================================================
* Test 5: Full sequential scan
* ========================================================================
? "--- Test 5: Full sequential scan (all " + STR(total) + " records) ---"

SET INDEX TO
t0 = SECONDS()
GO TOP
scan_count = 0
DO WHILE .NOT. EOF()
    scan_count = scan_count + 1
    SKIP
ENDDO
t1 = SECONDS()
? "  SKIP scan:           " + STR(t1 - t0, 8, 4) + "s  (" + STR((t1 - t0) / scan_count * 1000, 6, 3) + " ms/rec)  records=" + STR(scan_count)
? ""

* ========================================================================
* Test 6: Category count via full scan
* ========================================================================
? "--- Test 6: Count records per category (full scan) ---"

SET INDEX TO
t0 = SECONDS()
GO TOP
cnt_bakery = 0
cnt_beverages = 0
cnt_dairy = 0
cnt_frozen = 0
cnt_meat = 0
cnt_produce = 0
cnt_snacks = 0
cnt_other = 0
DO WHILE .NOT. EOF()
    DO CASE
        CASE CATEGORY = "Bakery"
            cnt_bakery = cnt_bakery + 1
        CASE CATEGORY = "Beverages"
            cnt_beverages = cnt_beverages + 1
        CASE CATEGORY = "Dairy"
            cnt_dairy = cnt_dairy + 1
        CASE CATEGORY = "Frozen"
            cnt_frozen = cnt_frozen + 1
        CASE CATEGORY = "Meat"
            cnt_meat = cnt_meat + 1
        CASE CATEGORY = "Produce"
            cnt_produce = cnt_produce + 1
        CASE CATEGORY = "Snacks"
            cnt_snacks = cnt_snacks + 1
        OTHERWISE
            cnt_other = cnt_other + 1
    ENDCASE
    SKIP
ENDDO
t1 = SECONDS()
? "  Bakery:      " + STR(cnt_bakery, 6)
? "  Beverages:   " + STR(cnt_beverages, 6)
? "  Dairy:       " + STR(cnt_dairy, 6)
? "  Frozen:      " + STR(cnt_frozen, 6)
? "  Meat:        " + STR(cnt_meat, 6)
? "  Produce:     " + STR(cnt_produce, 6)
? "  Snacks:      " + STR(cnt_snacks, 6)
? "  Other:       " + STR(cnt_other, 6)
? "  Time:        " + STR(t1 - t0, 8, 4) + "s"
? ""

* ========================================================================
* Summary
* ========================================================================
? "=== Benchmark Complete ==="
? "End: " + TIME()

CLOSE DATABASES
RETURN
