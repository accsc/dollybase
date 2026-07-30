* sm_providers.prg - Provider management

PROCEDURE sm_providers
  CLEAR
  @  2, 2 SAY "--- PROVIDERS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Add provider"
  @  5, 2 SAY "   2. List providers"
  @  6, 2 SAY "   0. Back"
  @  7, 2 SAY "   Option: "
  @  7, 12 GET gSubChoice RANGE 0, 2
  READ

  DO CASE
    CASE gSubChoice = 1
      DO provider_add
    CASE gSubChoice = 2
      DO provider_list
  ENDCASE

  RETURN

PROCEDURE provider_add
  SELECT 2
  APPEND BLANK

  CLEAR
  @  3, 2 SAY "Provider ID: "
  @  4, 2 SAY "Name:        "
  @  5, 2 SAY "Phone:       "
  @  6, 2 SAY "Address:     "

  @  3, 15 GET B->ID PICTURE "!!!!"
  @  4, 15 GET B->NAME PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  5, 15 GET B->PHONE PICTURE "!!!!!!!!!!!!!!!"
  @  6, 15 GET B->ADDRESS

  READ

  ? "Provider added."
  WAIT

  RETURN

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
