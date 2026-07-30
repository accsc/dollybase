* sale_record.prg - Record a completed sale to the database

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
