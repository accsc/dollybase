* sm_products.prg - Product management (CRUD)

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

PROCEDURE product_add
  SELECT 1
  APPEND BLANK

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

  A->ACTIVE = .T.

  ? "Product added."
  WAIT

  RETURN

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

PROCEDURE product_list
  SELECT 1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   PRODUCT LIST"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". [" + A->BARCODE + "] " + A->NAME
      ? "   Cat: " + A->CATEGORY + "  Price: " + LTRIM(STR(A->PRICE, 8, 2)) + "  Stock: " + LTRIM(STR(A->STOCK))
      ? ""
    ENDIF
    SKIP
  ENDDO

  ? "----------------------------------------"
  ? "Total: " + LTRIM(STR(cnt)) + " products"
  ? "----------------------------------------"
  WAIT

  RETURN

PROCEDURE product_search
  CLEAR
  ACCEPT "Enter search term: " TO searchTerm
  SELECT 1
  GO TOP

  found_cnt = 0
  DO WHILE .NOT. EOF()
    IF UPPER(searchTerm) $ UPPER(A->NAME) .OR. UPPER(searchTerm) $ UPPER(A->BARCODE)
      found_cnt = found_cnt + 1
      ? LTRIM(STR(found_cnt)) + ". [" + A->BARCODE + "] " + A->NAME
      ? "   Price: " + LTRIM(STR(A->PRICE, 8, 2)) + "  Stock: " + LTRIM(STR(A->STOCK))
    ENDIF
    SKIP
  ENDDO

  IF found_cnt = 0
    ? "No products found matching '" + searchTerm + "'"
  ELSE
    ? "Found: " + LTRIM(STR(found_cnt)) + " product(s)"
  ENDIF

  WAIT

  RETURN

PROCEDURE product_update_prices
  CLEAR
  ? "Price update mode:"
  ? "1 = Update by barcode"
  ? "2 = Update by category (percentage)"
  ACCEPT "Choose mode (1/2): " TO priceMode

  IF priceMode = "1"
    ACCEPT "Enter barcode: " TO pBarcode
    ACCEPT "Enter new price: " TO pPrice
    SELECT 1
    REPLACE A->PRICE WITH VAL(pPrice) FOR A->BARCODE = pBarcode
    ? "Price updated for barcode " + pBarcode
  ELSE
    ACCEPT "Enter category: " TO pCategory
    ACCEPT "Enter percentage change (e.g. 10 for +10%%): " TO pPct
    SELECT 1
    REPLACE A->PRICE WITH A->PRICE * (1 + VAL(pPct) / 100) FOR UPPER(A->CATEGORY) = UPPER(pCategory)
    ? "Prices updated for category: " + pCategory
  ENDIF

  WAIT

  RETURN
