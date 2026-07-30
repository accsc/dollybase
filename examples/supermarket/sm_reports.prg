* sm_reports.prg - Business reports

PROCEDURE sm_reports
  CLEAR
  @  2, 2 SAY "--- REPORTS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Low stock alert"
  @  5, 2 SAY "   2. Weekly reorder by provider"
  @  6, 2 SAY "   3. Daily sales summary"
  @  7, 2 SAY "   4. Product sales ranking"
  @  8, 2 SAY "   5. Revenue by category"
  @  9, 2 SAY "   0. Back"
  @ 10, 2 SAY "   Option: "
  @ 10, 12 GET gSubChoice RANGE 0, 5
  READ

  DO CASE
    CASE gSubChoice = 1
      DO report_low_stock
    CASE gSubChoice = 2
      DO report_reorder
    CASE gSubChoice = 3
      DO report_daily_sales
    CASE gSubChoice = 4
      DO report_sales_ranking
    CASE gSubChoice = 5
      DO report_category_revenue
  ENDCASE

  RETURN

PROCEDURE report_low_stock
  SELECT 1
  GO TOP

  CLEAR
  ? "========================================"
  ? "   LOW STOCK ALERT REPORT"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""
  ? "Barcode      Product                    Stock  Min   Provider"
  ? REPLICATE("-", 70)

  cnt = 0
  total_value = 0
  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      cnt = cnt + 1
      reorder_qty = A->MIN_STOCK * 2 - A->STOCK
      total_value = total_value + reorder_qty * A->COST
      ? LEFT(A->BARCODE, 13) + "  " + LEFT(A->NAME, 25) + "  " + LTRIM(STR(A->STOCK, 5)) + "  " + LTRIM(STR(A->MIN_STOCK, 5)) + "  " + A->PROVIDER_ID
    ENDIF
    SKIP
  ENDDO

  ? REPLICATE("-", 70)
  ? "Products below minimum: " + LTRIM(STR(cnt))
  ? "Estimated reorder cost: " + LTRIM(STR(total_value, 10, 2))
  ? "========================================"
  WAIT

  RETURN

PROCEDURE report_reorder
  SELECT 1
  GO TOP

  CLEAR
  ? "========================================"
  ? "   WEEKLY REORDER LIST BY PROVIDER"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""

  current_prov = ""
  total_orders = 0

  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      IF A->PROVIDER_ID != current_prov
        IF current_prov != ""
          ? ""
        ENDIF
        SELECT 2
        LOCATE FOR B->ID = A->PROVIDER_ID
        prov_name = IIF(FOUND(), B->NAME, A->PROVIDER_ID)

        ? "PROVIDER: [" + A->PROVIDER_ID + "] " + prov_name
        ? REPLICATE("-", 50)

        current_prov = A->PROVIDER_ID
      ENDIF

      SELECT 1
      reorder_qty = A->MIN_STOCK * 2 - A->STOCK
      total_orders = total_orders + 1
      ? "  " + LEFT(A->NAME, 30) + "  Qty: " + LTRIM(STR(reorder_qty)) + "  Est: " + LTRIM(STR(reorder_qty * A->COST, 8, 2))
    ENDIF
    SKIP
  ENDDO

  ? ""
  ? "========================================"
  ? "Total items to reorder: " + LTRIM(STR(total_orders))
  ? "========================================"
  WAIT

  RETURN

PROCEDURE report_daily_sales
  SELECT 3
  GO TOP

  CLEAR
  ? "========================================"
  ? "   DAILY SALES SUMMARY"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""

  today = DATE()
  total_sales = 0
  total_revenue = 0
  total_items = 0

  DO WHILE .NOT. EOF()
    IF C->SALE_DATE = today .AND. .NOT. DELETED()
      total_sales = total_sales + 1
      total_revenue = total_revenue + C->TOTAL
      total_items = total_items + C->ITEMS_COUNT
      ? "Sale " + C->SALE_ID + "  Items: " + LTRIM(STR(C->ITEMS_COUNT)) + "  Total: " + LTRIM(STR(C->TOTAL, 10, 2)) + "  (" + C->PAYMENT + ")"
    ENDIF
    SKIP
  ENDDO

  ? ""
  ? REPLICATE("-", 50)
  ? "Total sales today: " + LTRIM(STR(total_sales))
  ? "Total items sold:  " + LTRIM(STR(total_items))
  ? "Total revenue:     " + LTRIM(STR(total_revenue, 10, 2))
  ? "========================================"
  WAIT

  RETURN

PROCEDURE report_sales_ranking
  SELECT 4
  GO TOP

  CLEAR
  ? "========================================"
  ? "   PRODUCT SALES RANKING"
  ? "========================================"
  ? ""
  ? "Rank  Barcode      Product                    Qty Sold  Revenue"
  ? REPLICATE("-", 70)

  rank = 0
  current_bc = ""
  current_qty = 0
  current_rev = 0

  DO WHILE .NOT. EOF()
    IF D->BARCODE != current_bc
      IF current_bc != ""
        rank = rank + 1
        SELECT 1
        LOCATE FOR A->BARCODE = current_bc
        prod_name = IIF(FOUND(), A->NAME, current_bc)
        SELECT 4
        ? LTRIM(STR(rank, 4)) + "  " + LEFT(current_bc, 13) + "  " + LEFT(prod_name, 25) + "  " + LTRIM(STR(current_qty, 6)) + "  " + LTRIM(STR(current_rev, 10, 2))
      ENDIF
      current_bc = D->BARCODE
      current_qty = D->QTY
      current_rev = D->LINE_TOTAL
    ELSE
      current_qty = current_qty + D->QTY
      current_rev = current_rev + D->LINE_TOTAL
    ENDIF
    SKIP
  ENDDO

  IF current_bc != ""
    rank = rank + 1
    SELECT 1
    LOCATE FOR A->BARCODE = current_bc
    prod_name = IIF(FOUND(), A->NAME, current_bc)
    SELECT 4
    ? LTRIM(STR(rank, 4)) + "  " + LEFT(current_bc, 13) + "  " + LEFT(prod_name, 25) + "  " + LTRIM(STR(current_qty, 6)) + "  " + LTRIM(STR(current_rev, 10, 2))
  ENDIF

  ? REPLICATE("-", 70)
  ? "Total products sold: " + LTRIM(STR(rank))
  ? "========================================"
  WAIT

  RETURN

PROCEDURE report_category_revenue
  SELECT 4
  GO TOP

  CLEAR
  ? "========================================"
  ? "   REVENUE BY CATEGORY"
  ? "========================================"
  ? ""
  ? "Category                 Revenue       Products"
  ? REPLICATE("-", 55)

  total_rev = 0
  DO WHILE .NOT. EOF()
    SELECT 1
    LOCATE FOR A->BARCODE = D->BARCODE
    IF FOUND()
      current_cat = A->CATEGORY
    ELSE
      current_cat = "UNKNOWN"
    ENDIF
    SELECT 4
    total_rev = total_rev + D->LINE_TOTAL
    ? "  " + LEFT(current_cat, 23) + "  " + LTRIM(STR(D->LINE_TOTAL, 10, 2)) + "  (item: " + LEFT(D->BARCODE, 13) + ")"
    SKIP
  ENDDO

  ? REPLICATE("-", 55)
  ? "Total revenue: " + LTRIM(STR(total_rev, 12, 2))
  ? "========================================"
  WAIT

  RETURN
