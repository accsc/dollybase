* product_edit.prg - Edit an existing product

PROCEDURE product_edit
  ACCEPT "Enter barcode to edit: " TO editBarcode
  SELECT 1
  LOCATE FOR A->BARCODE = editBarcode

  IF .NOT. FOUND()
    ? "Product not found."
    WAIT
    RETURN
  ENDIF

  CLEAR
  @  3, 2 SAY "Barcode:     "
  @  4, 2 SAY "Name:        "
  @  5, 2 SAY "Category:    "
  @  6, 2 SAY "Price:       "
  @  7, 2 SAY "Cost:        "
  @  8, 2 SAY "Stock:       "
  @  9, 2 SAY "Min Stock:   "
  @ 10, 2 SAY "Provider ID: "
  @ 11, 2 SAY "Unit:        "
  @ 12, 2 SAY "Tax Rate:    "
  @ 13, 2 SAY "Description: "

  @  3, 14 GET A->BARCODE PICTURE "!!!!!!!!!!!!!"
  @  4, 14 GET A->NAME PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  5, 14 GET A->CATEGORY PICTURE "!!!!!!!!!!!!!!!!!!!!"
  @  6, 14 GET A->PRICE RANGE 0, 999999
  @  7, 14 GET A->COST RANGE 0, 999999
  @  8, 14 GET A->STOCK RANGE 0, 99999
  @  9, 14 GET A->MIN_STOCK RANGE 0, 999
  @ 10, 14 GET A->PROVIDER_ID PICTURE "!!!!"
  @ 11, 14 GET A->UNIT PICTURE "!!!!!!!!!!"
  @ 12, 14 GET A->TAX_RATE RANGE 0, 100
  @ 13, 14 GET A->DESCRIPTION

  READ

  ? "Product updated."
  WAIT

  RETURN
