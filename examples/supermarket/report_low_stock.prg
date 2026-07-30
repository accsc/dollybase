* report_low_stock.prg - Low stock alert report

PROCEDURE report_low_stock
  SELECT 1
  GO TOP

  CLEAR
  ? "========================================"
  ? "   LOW STOCK ALERT REPORT"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""
  ? "Barcode      Product                    Stock  Min   Provider"
  ? REPLICATE("-", 70)

  cnt = 0
  total_value = 0
  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      cnt = cnt + 1
      reorder_qty = A->MIN_STOCK * 2 - A->STOCK
      total_value = total_value + reorder_qty * A->COST
      ? LEFT(A->BARCODE, 13) + "  " + LEFT(A->NAME, 25) + "  " + LTRIM(STR(A->STOCK, 5)) + "  " + LTRIM(STR(A->MIN_STOCK, 5)) + "  " + A->PROVIDER_ID
    ENDIF
    SKIP
  ENDDO

  ? REPLICATE("-", 70)
  ? "Products below minimum: " + LTRIM(STR(cnt))
  ? "Estimated reorder cost: " + LTRIM(STR(total_value, 10, 2))
  ? "========================================"
  WAIT

  RETURN
