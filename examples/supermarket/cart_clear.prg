* cart_clear.prg - Clear all cart entries

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
