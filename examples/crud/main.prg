* main.prg - CRUD application entry point for books_memo database
*
* DOLLYBASE CRUD Example
* by Alvaro Cortes <alvarocortesc@gmail.com> - GPLv2

* --- Global setup ---
SET TALK OFF

* Open the database
USE books_memo

? "Loaded " + LTRIM(STR(RECCOUNT(), 5,0)) + " records in books_memo"
WAIT

* --- Main loop ---
gChoice = 0
DO WHILE .T.
  DO menu1

  DO CASE
    CASE gChoice = 1
      DO add1
    CASE gChoice = 2
      DO edit1
    CASE gChoice = 3
      DO delete1
    CASE gChoice = 4
      DO list1
    CASE gChoice = 5
      DO search1
    CASE gChoice = 6
      DO detail1
    CASE gChoice = 0
      ? "Goodbye!"
      EXIT
    OTHERWISE
      ? "Invalid option"
      WAIT
  ENDCASE
ENDDO

CLOSE DATABASES
RETURN
