* sm_products.prg - Product management menu

PROCEDURE sm_products
  CLEAR
  @  2, 2 SAY "--- PRODUCTS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Add product"
  @  5, 2 SAY "   2. Edit product"
  @  6, 2 SAY "   3. List products"
  @  7, 2 SAY "   4. Search product"
  @  8, 2 SAY "   5. Update prices"
  @  9, 2 SAY "   0. Back"
  @ 10, 2 SAY "   Option: "
  @ 10, 12 GET gSubChoice RANGE 0, 5
  READ

  DO CASE
    CASE gSubChoice = 1
      DO product_add
    CASE gSubChoice = 2
      DO product_edit
    CASE gSubChoice = 3
      DO product_list
    CASE gSubChoice = 4
      DO product_search
    CASE gSubChoice = 5
      DO product_update_prices
  ENDCASE

  RETURN
