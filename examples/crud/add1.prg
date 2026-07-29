* add.prg - Add a new book record (CREATE)
*
* Appends a blank record and prompts for all fields via @...GET

PROCEDURE add1
  APPEND BLANK

  CLEAR
  ? "----------------------------------------"
  ? "   ADD NEW BOOK"
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
  ? "Record added. Press any key..."
  WAIT

  RETURN
