DO WHILE .T.
CLEAR
ANSWER = " "
TEXT

 Menu options

 1. For option1
 2. For option2
 3. For option3
 4. Exit
ENDTEXT

@ 7,28 SAY "ANSWER:"  GET ANSWER PICT "@!"
READ
*ANSWER = ALLTRIM(ANSWER)

DO CASE

 CASE .NOT. (ANSWER $ "1234")
 ? ANSWER
 @ 24,15 SAY "ERROR - Invalid option"
 WAIT "Press any key..."
 LOOP
 CASE ANSWER = "1"
  ? "option1 selected"
 CASE ANSWER = "4"
  RETURN

ENDCASE

ENDDO
