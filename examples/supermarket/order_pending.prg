* order_pending.prg - View pending orders

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
