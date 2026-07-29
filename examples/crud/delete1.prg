* delete.prg - Delete the current book record (DELETE)
*
* Flags the current record for deletion and confirms

PROCEDURE delete1
  IF EOF() .OR. BOF()
    ? "No current record to delete."
    WAIT
    RETURN
  ENDIF

  CLEAR
  ? "----------------------------------------"
  ? "   DELETE BOOK"
  ? "----------------------------------------"
  ? ""
  ? "Record " + LTRIM(STR(RECNO())) + ":"
  ? "  Title: " + A->TITULO
  ? "  Author: " + A->NMB_AU + " " + A->AP1_AU + " " + A->AP2_AU
  ? ""
  ? "Are you sure? (Y/N): "
  ACCEPT "" TO ans
  ans = UPPER(LEFT(ans, 1))
  IF ans = "Y"
    DELETE
    ? "Record flagged for deletion."
  ELSE
    ? "Deletion cancelled."
  ENDIF
  WAIT

  RETURN
