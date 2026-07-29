* search.prg - Search books by title or author (READ)
*
* Prompts for a search string and uses LOCATE FOR with "$" operator
* to find matching records

PROCEDURE search1
  CLEAR
  ? "----------------------------------------"
  ? "   SEARCH BOOKS"
  ? "----------------------------------------"
  ? ""
  ACCEPT "Enter search term: " TO searchTerm
  GO TOP

  found_flag = 0
  DO WHILE .NOT. EOF()
    c1 = UPPER(searchTerm) $ UPPER(A->TITULO)
    c2 = UPPER(searchTerm) $ UPPER(A->NMB_AU)
    c3 = UPPER(searchTerm) $ UPPER(A->AP1_AU)
    c4 = UPPER(searchTerm) $ UPPER(A->AP2_AU)
    c5 =  UPPER(searchTerm) $ UPPER(A->EDITORIAL)
    c6 = UPPER(searchTerm) $ UPPER(A->COMMENTS)
    IF c1 .OR. c2 .OR. c3 .OR. c4 .OR. c5 .OR. c6
      found_flag = found_flag + 1
      ? LTRIM(STR(found_flag)) + ". " + A->TITULO
      ? "   " + A->NMB_AU + " " + A->AP1_AU + " " + A->AP2_AU
      ? "   " + A->EDITORIAL + " (" + LTRIM(STR(A->AN_EDICION)) + ")"
      ? ""
      ? "COMENTARIO: " + COMMENTS
    ENDIF
    SKIP
  ENDDO

  IF found_flag = 0
    ? "No books found_flag matching '" + searchTerm + "'"
  ELSE
    ? "----------------------------------------"
    ? "Found: " + LTRIM(STR(found_flag)) + " record(s)"
    ? "----------------------------------------"
  ENDIF

  WAIT

  RETURN
