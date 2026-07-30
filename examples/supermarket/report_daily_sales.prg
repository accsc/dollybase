* report_daily_sales.prg - Daily sales summary

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
