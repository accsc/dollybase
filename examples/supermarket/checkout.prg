* checkout.prg - Point-of-sale terminal

SET TALK OFF

DO sm_init

DO cart_clear
cART_GRAND_TOTAL = 0
gSaleDate = DTOC(DATE())
gSaleCounter = 0

* Main checkout loop
DO WHILE .T.
  CLEAR
  @  1, 1 SAY REPLICATE("=", 70)
  @  2, 1 SAY "   CHECKOUT TERMINAL - Supermarket POS"
  @  3, 1 SAY REPLICATE("=", 70)
  @  4, 1 SAY ""
  @  5, 1 SAY "  Barcode        Name                      Qty   Price    Total"
  @  6, 1 SAY REPLICATE("-", 70)

  row = 7
  i = 1
  max_show = MIN(cART_COUNT, 15)
  DO WHILE i <= max_show .AND. row <= 20
    cart_var = "cART" + LTRIM(STR(i)) + "_BARCODE"
    d_barcode = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_NAME"
    d_name = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_QTY"
    d_qty = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_PRICE"
    d_price = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_TOTAL"
    d_total = &cart_var

    @ row, 1 SAY "  " + LEFT(d_barcode, 14) + "  " + LEFT(d_name, 24) + "  " + LTRIM(STR(d_qty, 3)) + "  " + LTRIM(STR(d_price, 8, 2)) + "  " + LTRIM(STR(d_total, 8, 2))
    row = row + 1
    i = i + 1
  ENDDO

  @ 22, 1 SAY REPLICATE("-", 70)
  @ 22, 1 SAY "  Items: " + LTRIM(STR(cART_COUNT)) + "   TOTAL: " + LTRIM(STR(cART_GRAND_TOTAL, 10, 2))
  @ 23, 1 SAY REPLICATE("-", 70)
  @ 24, 1 SAY "  Barcode: "
  @ 24, 11 GET inputBarcode PICTURE "!!!!!!!!!!!!!"
  @ 25, 1 SAY "  [Enter=Add] [F2=Checkout] [F3=Remove] [Esc=Cancel]"
  READ

  * Esc pressed (empty field) — check for special commands
  IF inputBarcode = ""
    * User pressed Esc on empty field — could be cancel
    * Check next key to distinguish
    LOOP
  ENDIF

  * Special commands typed in the barcode field
  IF inputBarcode = "F2" .OR. inputBarcode = "f2"
    IF cART_COUNT > 0
      gPaymentMethod = "CASH"
      DO sale_record
      CLEAR
      ? "========================================"
      ? "   RECEIPT - Sale " + sale_id
      ? "========================================"
      ? "Total: " + LTRIM(STR(cART_GRAND_TOTAL, 10, 2))
      ? "Items: " + LTRIM(STR(cART_COUNT))
      ? "Thank you!"
      ? "========================================"
      WAIT
    ENDIF
    LOOP
  ENDIF

  IF inputBarcode = "F3" .OR. inputBarcode = "f3"
    IF cART_COUNT > 0
      DO cart_remove_last
    ENDIF
    LOOP
  ENDIF

  IF inputBarcode = "Q" .OR. inputBarcode = "q"
    DO cart_clear
    EXIT
  ENDIF

  * Normal barcode entry
  SELECT 1
  LOCATE FOR A->BARCODE = inputBarcode

  IF FOUND()
    DO cart_add WITH inputBarcode, A->NAME, 1, A->PRICE
    LOOP
  ELSE
    ? "Product not found: " + inputBarcode
    WAIT
  ENDIF
ENDDO

CLOSE DATABASES
RETURN

PROCEDURE cart_clear
  STORE 0 TO cART_COUNT, cART_GRAND_TOTAL
  i = 1
  DO WHILE i <= 50
    cart_var = "cART" + LTRIM(STR(i)) + "_BARCODE"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(i)) + "_NAME"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(i)) + "_QTY"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(i)) + "_PRICE"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(i)) + "_TOTAL"
    &cart_var = 0
    i = i + 1
  ENDDO
  RETURN

PROCEDURE cart_add
  PARAMETERS pBarcode, pName, pQty, pPrice

  cART_COUNT = cART_COUNT + 1
  idx = cART_COUNT

  cart_var = "cART" + LTRIM(STR(idx)) + "_BARCODE"
  &cart_var = pBarcode

  cart_var = "cART" + LTRIM(STR(idx)) + "_NAME"
  &cart_var = pName

  cart_var = "cART" + LTRIM(STR(idx)) + "_QTY"
  &cart_var = pQty

  cart_var = "cART" + LTRIM(STR(idx)) + "_PRICE"
  &cart_var = pPrice

  cart_var = "cART" + LTRIM(STR(idx)) + "_TOTAL"
  &cart_var = pQty * pPrice

  cART_GRAND_TOTAL = cART_GRAND_TOTAL + (pQty * pPrice)

  RETURN

PROCEDURE cart_remove_last
  IF cART_COUNT > 0
    idx = cART_COUNT
    cart_var = "cART" + LTRIM(STR(idx)) + "_TOTAL"
    item_total = &cart_var
    cART_GRAND_TOTAL = cART_GRAND_TOTAL - item_total
    cART_COUNT = cART_COUNT - 1

    cart_var = "cART" + LTRIM(STR(idx)) + "_BARCODE"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(idx)) + "_NAME"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(idx)) + "_QTY"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(idx)) + "_PRICE"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(idx)) + "_TOTAL"
    &cart_var = 0
  ENDIF
  RETURN
