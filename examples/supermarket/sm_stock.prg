* sm_stock.prg - Stock management

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

PROCEDURE stock_adjust
  ACCEPT "Enter barcode: " TO sBarcode
  ACCEPT "Enter quantity to add: " TO sQty

  SELECT 1
  LOCATE FOR A->BARCODE = sBarcode

  IF FOUND()
    REPLACE A->STOCK WITH A->STOCK + VAL(sQty)
    ? "Stock updated. New stock: " + LTRIM(STR(A->STOCK))
  ELSE
    ? "Product not found."
  ENDIF

  WAIT

  RETURN

PROCEDURE stock_low
  SELECT 1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   LOW STOCK ALERT"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". [" + A->BARCODE + "] " + A->NAME
      ? "   Stock: " + LTRIM(STR(A->STOCK)) + " / Min: " + LTRIM(STR(A->MIN_STOCK))
      ? ""
    ENDIF
    SKIP
  ENDDO

  IF cnt = 0
    ? "All products are above minimum stock levels."
  ELSE
    ? "----------------------------------------"
    ? "Alert: " + LTRIM(STR(cnt)) + " product(s) below minimum"
  ENDIF

  ? "----------------------------------------"
  WAIT

  RETURN

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
