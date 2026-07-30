* sm_stock.prg - Stock management menu

PROCEDURE sm_stock
  CLEAR
  @  2, 2 SAY "--- STOCK MANAGEMENT ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Adjust stock (incoming)"
  @  5, 2 SAY "   2. View low stock items"
  @  6, 2 SAY "   3. Full stock report"
  @  7, 2 SAY "   0. Back"
  @  8, 2 SAY "   Option: "
  @  8, 12 GET gSubChoice RANGE 0, 3
  READ

  DO CASE
    CASE gSubChoice = 1
      DO stock_adjust
    CASE gSubChoice = 2
      DO stock_low
    CASE gSubChoice = 3
      DO stock_full
  ENDCASE

  RETURN
