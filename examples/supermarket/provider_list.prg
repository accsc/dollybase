* provider_list.prg - List all providers

PROCEDURE provider_list
  SELECT 2
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   PROVIDER LIST"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". [" + B->ID + "] " + B->NAME
      ? "   Phone: " + B->PHONE
      ? ""
    ENDIF
    SKIP
  ENDDO

  ? "Total: " + LTRIM(STR(cnt)) + " providers"
  WAIT

  RETURN
