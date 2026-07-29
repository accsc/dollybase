* list1.prg - List all books in a formatted table (READ)
*
* Shows title, author name, last names, publisher, year

PROCEDURE list1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   BOOK LIST"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". " + A->TITULO
      ? "   " + A->NMB_AU + " " + A->AP1_AU + " " + A->AP2_AU
      ? "   " + A->EDITORIAL + " (" + LTRIM(STR(A->AN_EDICION)) + ")"
      ? ""
    ENDIF
    SKIP
  ENDDO

  ? "----------------------------------------"
  ? "Total: " + LTRIM(STR(cnt)) + " records"
  ? "----------------------------------------"
  WAIT "Press any key to continue..."

  RETURN
