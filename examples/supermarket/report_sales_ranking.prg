* report_sales_ranking.prg - Product sales ranking

PROCEDURE report_sales_ranking
  SELECT 4
  GO TOP

  CLEAR
  ? "========================================"
  ? "   PRODUCT SALES RANKING"
  ? "========================================"
  ? ""
  ? "Rank  Barcode      Product                    Qty Sold  Revenue"
  ? REPLICATE("-", 70)

  rank = 0
  current_bc = ""
  current_qty = 0
  current_rev = 0

  DO WHILE .NOT. EOF()
    IF D->BARCODE != current_bc
      IF current_bc != ""
        rank = rank + 1
        SELECT 1
        LOCATE FOR A->BARCODE = current_bc
        prod_name = IIF(FOUND(), A->NAME, current_bc)
        SELECT 4
        ? LTRIM(STR(rank, 4)) + "  " + LEFT(current_bc, 13) + "  " + LEFT(prod_name, 25) + "  " + LTRIM(STR(current_qty, 6)) + "  " + LTRIM(STR(current_rev, 10, 2))
      ENDIF
      current_bc = D->BARCODE
      current_qty = D->QTY
      current_rev = D->LINE_TOTAL
    ELSE
      current_qty = current_qty + D->QTY
      current_rev = current_rev + D->LINE_TOTAL
    ENDIF
    SKIP
  ENDDO

  IF current_bc != ""
    rank = rank + 1
    SELECT 1
    LOCATE FOR A->BARCODE = current_bc
    prod_name = IIF(FOUND(), A->NAME, current_bc)
    SELECT 4
    ? LTRIM(STR(rank, 4)) + "  " + LEFT(current_bc, 13) + "  " + LEFT(prod_name, 25) + "  " + LTRIM(STR(current_qty, 6)) + "  " + LTRIM(STR(current_rev, 10, 2))
  ENDIF

  ? REPLICATE("-", 70)
  ? "Total products sold: " + LTRIM(STR(rank))
  ? "========================================"
  WAIT

  RETURN
