* order_generate.prg - Generate reorder list from low stock items

PROCEDURE order_generate
  CLEAR
  ? "Generating reorder list..."
  ? ""

  SELECT 1
  GO TOP

  ord_counter = 0
  today = DTOC(DATE())
  current_provider = ""
  order_items = ""

  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      IF current_provider = ""
        current_provider = A->PROVIDER_ID
      ENDIF

      IF A->PROVIDER_ID != current_provider
        SELECT 5
        APPEND BLANK
        ord_counter = ord_counter + 1
        E->ORDER_ID = today + LTRIM(STR(ord_counter, 3, 0))
        E->PROVIDER_ID = current_provider
        E->ORDER_DATE = CTOD(today)
        E->STATUS = "P"
        E->NOTES = order_items
        current_provider = A->PROVIDER_ID
        order_items = ""
      ENDIF

      SELECT 1
      order_items = order_items + A->NAME + " (" + LTRIM(STR(A->MIN_STOCK * 2 - A->STOCK)) + ") "
    ENDIF
    SKIP
  ENDDO

  IF order_items != ""
    SELECT 5
    APPEND BLANK
    ord_counter = ord_counter + 1
    E->ORDER_ID = today + LTRIM(STR(ord_counter, 3, 0))
    E->PROVIDER_ID = current_provider
    E->ORDER_DATE = CTOD(today)
    E->STATUS = "P"
    E->NOTES = order_items
  ENDIF

  ? "Generated " + LTRIM(STR(ord_counter)) + " reorder order(s)."
  WAIT

  RETURN
