* provider_add.prg - Add a new provider

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
