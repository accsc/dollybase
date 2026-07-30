* sm_reports.prg - Reports menu

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
