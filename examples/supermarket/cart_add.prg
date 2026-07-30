* cart_add.prg - Add an item to the cart

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
