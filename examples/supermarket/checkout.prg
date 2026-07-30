* checkout.prg - Point-of-sale terminal

SET TALK OFF

SET PROCEDURE TO sm_init, sm_main_menu, sm_products, sm_providers, sm_stock, sm_orders, sm_reports, sm_sales, product_add, product_edit, product_list, product_search, product_update_prices, provider_add, provider_list, stock_adjust, stock_low, stock_full, order_generate, order_pending, order_receive, report_low_stock, report_reorder, report_daily_sales, report_sales_ranking, report_category_revenue, sale_counter_reset, sale_record, cart_clear, cart_add, cart_remove_last

DO init_system

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

  IF inputBarcode = ""
    LOOP
  ENDIF

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
