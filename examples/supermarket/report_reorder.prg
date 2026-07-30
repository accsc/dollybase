* report_reorder.prg - Weekly reorder list by provider

PROCEDURE report_reorder
  SELECT 1
  GO TOP

  CLEAR
  ? "========================================"
  ? "   WEEKLY REORDER LIST BY PROVIDER"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""

  current_prov = ""
  total_orders = 0

  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      IF A->PROVIDER_ID != current_prov
        IF current_prov != ""
          ? ""
        ENDIF
        SELECT 2
        LOCATE FOR B->ID = A->PROVIDER_ID
        prov_name = IIF(FOUND(), B->NAME, A->PROVIDER_ID)

        ? "PROVIDER: [" + A->PROVIDER_ID + "] " + prov_name
        ? REPLICATE("-", 50)

        current_prov = A->PROVIDER_ID
      ENDIF

      SELECT 1
      reorder_qty = A->MIN_STOCK * 2 - A->STOCK
      total_orders = total_orders + 1
      ? "  " + LEFT(A->NAME, 30) + "  Qty: " + LTRIM(STR(reorder_qty)) + "  Est: " + LTRIM(STR(reorder_qty * A->COST, 8, 2))
    ENDIF
    SKIP
  ENDDO

  ? ""
  ? "========================================"
  ? "Total items to reorder: " + LTRIM(STR(total_orders))
  ? "========================================"
  WAIT

  RETURN
