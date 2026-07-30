* sm_sales.prg - Sales history display

PROCEDURE sm_sales
  CLEAR
  ? "----------------------------------------"
  ? "   SALES HISTORY"
  ? "----------------------------------------"
  ? ""

  SELECT 3
  GO TOP

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". " + C->SALE_ID + "  Date: " + DTOC(C->SALE_DATE) + "  Items: " + LTRIM(STR(C->ITEMS_COUNT)) + "  Total: " + LTRIM(STR(C->TOTAL, 10, 2)) + "  (" + C->PAYMENT + ")"
    ENDIF
    SKIP
  ENDDO

  ? "----------------------------------------"
  ? "Total: " + LTRIM(STR(cnt)) + " sales"
  ? "----------------------------------------"
  WAIT

  RETURN
