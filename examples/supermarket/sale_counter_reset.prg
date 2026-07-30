* sale_counter_reset.prg - Reset sale counter for a new day

PROCEDURE sale_counter_reset
  IF gSaleDate != DTOC(DATE())
    gSaleDate = DTOC(DATE())
    gSaleCounter = 0
  ENDIF
  RETURN
