* detail.prg - Show full detail of the current book record (READ)
*
* Displays all fields including the memo COMMENTS field

PROCEDURE detail1
  IF EOF() .OR. BOF()
    ? "No current record to display."
    WAIT
    RETURN
  ENDIF

  CLEAR
  ? "----------------------------------------"
  ? "   BOOK DETAIL (Record " + LTRIM(STR(RECNO())) + ")"
  ? "----------------------------------------"
  ? ""
  ? "Title:      " + A->TITULO
  ? "Last Name 1: " + A->AP1_AU
  ? "Last Name 2: " + A->AP2_AU
  ? "Name:       " + A->NMB_AU
  ? "Publisher:  " + A->EDITORIAL
  ? "Year:       " + LTRIM(STR(A->AN_EDICION))
  ? ""
  ? "Comments:"
  ? A->COMMENTS
  ? ""
  ? "----------------------------------------"
  WAIT

  RETURN
