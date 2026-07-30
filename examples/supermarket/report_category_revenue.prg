* report_category_revenue.prg - Revenue by category

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
