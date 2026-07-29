* menu.prg - Display main menu, stores selection in gChoice
*
* Uses global variable gChoice (no pass-by-ref in DO...WITH yet)

PROCEDURE menu1
  CLEAR

  @  1, 2 SAY "========================================"
  @  2, 2 SAY "   BOOKS MANAGEMENT SYSTEM"
  @  3, 2 SAY "========================================"
  @  4, 2 SAY ""
  @  5, 2 SAY "   1. Add new book"
  @  6, 2 SAY "   2. Edit current book"
  @  7, 2 SAY "   3. Delete current book"
  @  8, 2 SAY "   4. List all books"
  @  9, 2 SAY "   5. Search books"
  @ 10, 2 SAY "   6. Show book detail"
  @ 11, 2 SAY "   0. Exit"
  @ 12, 2 SAY ""
  @ 13, 2 SAY "========================================"
  @ 14, 2 SAY "   Select an option: "
  @ 14, 30 GET gChoice RANGE 0, 6
  READ

  RETURN
