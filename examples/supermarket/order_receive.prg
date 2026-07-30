* order_receive.prg - Mark an order as received

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
