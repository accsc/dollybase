* sm_orders.prg - Order management menu

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
