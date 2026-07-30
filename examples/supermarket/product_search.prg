* product_search.prg - Search products by name or barcode

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
