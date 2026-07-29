* edit.prg - Edit the current book record (UPDATE)
*
* Prompts for all fields via @...GET on the current record

PROCEDURE edit1
  IF EOF() .OR. BOF()
    ? "No current record to edit."
    WAIT
    RETURN
  ENDIF

  CLEAR
  ? "----------------------------------------"
  ? "   EDIT BOOK (Record " + LTRIM(STR(RECNO())) + ")"
  ? "----------------------------------------"
  ? ""

  @  4, 2 SAY "Title:        "
  @  5, 2 SAY "Last Name 1:  "
  @  6, 2 SAY "Last Name 2:  "
  @  7, 2 SAY "Name:         "
  @  8, 2 SAY "Publisher:    "
  @  9, 2 SAY "Year:         "
  @ 10, 2 SAY "Comments:     "

  @  4, 15 GET A->TITULO PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  5, 15 GET A->AP1_AU PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  6, 15 GET A->AP2_AU PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  7, 15 GET A->NMB_AU PICTURE "!!!!!!!!!!!!!!!!!!!!"
  @  8, 15 GET A->EDITORIAL PICTURE "!!!!!!!!!!!!!!!!!!!!"
  @  9, 15 GET A->AN_EDICION RANGE 1800, 2100
  @ 10, 15 GET A->COMMENTS

  READ

  ? ""
  ? "Record updated. Press any key..."
  WAIT

  RETURN
