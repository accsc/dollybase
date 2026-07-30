* sm_sales.prg - Sale recording and history

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

PROCEDURE sale_counter_reset
  IF gSaleDate != DTOC(DATE())
    gSaleDate = DTOC(DATE())
    gSaleCounter = 0
  ENDIF
  RETURN

PROCEDURE sale_record
  DO sale_counter_reset

  gSaleCounter = gSaleCounter + 1
  sale_id = gSaleDate + LTRIM(STR(gSaleCounter, 3, 0))

  SELECT 3
  APPEND BLANK
  C->SALE_ID = sale_id
  C->SALE_DATE = DATE()
  C->TOTAL = cART_GRAND_TOTAL
  C->ITEMS_COUNT = cART_COUNT
  C->PAYMENT = gPaymentMethod

  i = 1
  DO WHILE i <= cART_COUNT
    cart_var = "cART" + LTRIM(STR(i)) + "_BARCODE"
    item_barcode = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_QTY"
    item_qty = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_PRICE"
    item_price = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_TOTAL"
    item_total = &cart_var

    SELECT 4
    APPEND BLANK
    D->SALE_ID = sale_id
    D->BARCODE = item_barcode
    D->QTY = item_qty
    D->UNIT_PRICE = item_price
    D->LINE_TOTAL = item_total

    SELECT 1
    LOCATE FOR A->BARCODE = item_barcode
    IF FOUND()
      REPLACE A->STOCK WITH A->STOCK - item_qty
      REPLACE A->LAST_SOLD WITH DATE()
    ENDIF

    i = i + 1
  ENDDO

  DO cart_clear

  RETURN
