* cart_remove_last.prg - Remove the last item from the cart

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
