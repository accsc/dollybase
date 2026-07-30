* stock_full.prg - Full stock report

PROCEDURE stock_full
  SELECT 1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   FULL STOCK REPORT"
  ? "----------------------------------------"
  ? REPLICATE("-", 60)
  ? "Barcode      Name                      Price    Stock  Min"
  ? REPLICATE("-", 60)

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LEFT(A->BARCODE, 13) + "  " + LEFT(A->NAME, 25) + "  " + LTRIM(STR(A->PRICE, 8, 2)) + "  " + LTRIM(STR(A->STOCK, 5)) + "  " + LTRIM(STR(A->MIN_STOCK, 3))
    ENDIF
    SKIP
  ENDDO

  ? REPLICATE("-", 60)
  ? "Total: " + LTRIM(STR(cnt)) + " products"
  ? "----------------------------------------"
  WAIT

  RETURN
