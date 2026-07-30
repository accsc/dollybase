* sm_orders.prg - Order management (reorder to providers)

PROCEDURE sm_orders
  CLEAR
  @  2, 2 SAY "--- ORDERS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Generate reorder list"
  @  5, 2 SAY "   2. View pending orders"
  @  6, 2 SAY "   3. Mark order as received"
  @  7, 2 SAY "   0. Back"
  @  8, 2 SAY "   Option: "
  @  8, 12 GET gSubChoice RANGE 0, 3
  READ

  DO CASE
    CASE gSubChoice = 1
      DO order_generate
    CASE gSubChoice = 2
      DO order_pending
    CASE gSubChoice = 3
      DO order_receive
  ENDCASE

  RETURN

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

PROCEDURE order_pending
  SELECT 5
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   PENDING ORDERS"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF E->STATUS = "P" .AND. .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". Order: " + E->ORDER_ID
      ? "   Provider: " + E->PROVIDER_ID
      ? "   Date: " + DTOC(E->ORDER_DATE)
      ? "   Items: " + E->NOTES
      ? ""
    ENDIF
    SKIP
  ENDDO

  IF cnt = 0
    ? "No pending orders."
  ELSE
    ? "Total: " + LTRIM(STR(cnt)) + " pending order(s)"
  ENDIF

  ? "----------------------------------------"
  WAIT

  RETURN

PROCEDURE order_receive
  ACCEPT "Enter order ID to mark as received: " TO rOrderId

  SELECT 5
  LOCATE FOR E->ORDER_ID = rOrderId

  IF FOUND()
    REPLACE E->STATUS WITH "R"
    ? "Order " + rOrderId + " marked as received."
  ELSE
    ? "Order not found."
  ENDIF

  WAIT

  RETURN
