* stock_low.prg - View low stock items

PROCEDURE stock_low
  SELECT 1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   LOW STOCK ALERT"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". [" + A->BARCODE + "] " + A->NAME
      ? "   Stock: " + LTRIM(STR(A->STOCK)) + " / Min: " + LTRIM(STR(A->MIN_STOCK))
      ? ""
    ENDIF
    SKIP
  ENDDO

  IF cnt = 0
    ? "All products are above minimum stock levels."
  ELSE
    ? "----------------------------------------"
    ? "Alert: " + LTRIM(STR(cnt)) + " product(s) below minimum"
  ENDIF

  ? "----------------------------------------"
  WAIT

  RETURN
