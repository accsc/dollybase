* sm_providers.prg - Provider management menu

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
