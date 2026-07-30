* product_list.prg - List all products

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
