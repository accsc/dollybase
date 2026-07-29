* stress_menu.prg — Interactive menu demo with box drawing and INKEY selection
* Options 1-9 show a message, 0 exits, anything else shows an error

CLEAR

* --- Draw the menu frame ---
@  1,10 SAY "+" + REPLICATE("-", 38) + "+"
@  2,10 SAY "|        DOLLYBASE MAIN MENU           |"
@  3,10 SAY "+" + REPLICATE("-", 38) + "+"
@  4,10 SAY "|" + REPLICATE(" ", 38) + "|"
@  5,10 SAY "|  1. Open Database                    |"
@  6,10 SAY "|  2. List Records                     |"
@  7,10 SAY "|  3. Add New Record                   |"
@  8,10 SAY "|  4. Delete Record                    |"
@  9,10 SAY "|  5. Search Records                   |"
@ 10,10 SAY "|  6. Replace Fields                   |"
@ 11,10 SAY "|  7. Generate Report                  |"
@ 12,10 SAY "|  8. Export Data                      |"
@ 13,10 SAY "|  9. Settings                         |"
@ 14,10 SAY "|" + REPLICATE(" ", 38) + "|"
@ 15,10 SAY "|  0. Exit                             |"
@ 16,10 SAY "|" + REPLICATE(" ", 38) + "|"
@ 17,10 SAY "+" + REPLICATE("-", 38) + "+"
@ 18,10 SAY "|  Press the number key to select      |"
@ 19,10 SAY "+" + REPLICATE("-", 38) + "+"

* --- Menu loop ---
DO WHILE .T.
    @ 21,10 SAY " "
    @ 21,10 SAY "Select an option [0-9]: "
    key = INKEY()

    * INKEY() returns ASCII code: '0'=48, '1'=49, ... '9'=57
    * Convert to numeric option (0-9)
    IF key >= 48 .AND. key <= 57
        n = key - 48
    ELSE
        n = -1
    ENDIF

    DO CASE
        CASE n = 0
            @ 23,10 SAY "Goodbye!"
            WAIT
            RETURN
        CASE n = 1
            @ 23,10 SAY "You selected: Open Database"
        CASE n = 2
            @ 23,10 SAY "You selected: List Records"
        CASE n = 3
            @ 23,10 SAY "You selected: Add New Record"
        CASE n = 4
            @ 23,10 SAY "You selected: Delete Record"
        CASE n = 5
            @ 23,10 SAY "You selected: Search Records"
        CASE n = 6
            @ 23,10 SAY "You selected: Replace Fields"
        CASE n = 7
            @ 23,10 SAY "You selected: Generate Report"
        CASE n = 8
            @ 23,10 SAY "You selected: Export Data"
        CASE n = 9
            @ 23,10 SAY "You selected: Settings"
        OTHERWISE
            @ 23,10 SAY "ERROR - Invalid option. Press a number key [0-9]."
    ENDCASE

    WAIT
    CLEAR

    * Redraw menu
    @  1,10 SAY "+" + REPLICATE("-", 38) + "+"
    @  2,10 SAY "|        DOLLYBASE MAIN MENU          |"
    @  3,10 SAY "+" + REPLICATE("-", 38) + "+"
    @  4,10 SAY "|" + REPLICATE(" ", 38) + "|"
    @  5,10 SAY "|  1. Open Database                  |"
    @  6,10 SAY "|  2. List Records                   |"
    @  7,10 SAY "|  3. Add New Record                 |"
    @  8,10 SAY "|  4. Delete Record                  |"
    @  9,10 SAY "|  5. Search Records                 |"
    @ 10,10 SAY "|  6. Replace Fields                 |"
    @ 11,10 SAY "|  7. Generate Report                |"
    @ 12,10 SAY "|  8. Export Data                    |"
    @ 13,10 SAY "|  9. Settings                       |"
    @ 14,10 SAY "|" + REPLICATE(" ", 38) + "|"
    @ 15,10 SAY "|  0. Exit                           |"
    @ 16,10 SAY "|" + REPLICATE(" ", 38) + "|"
    @ 17,10 SAY "+" + REPLICATE("-", 38) + "+"
    @ 18,10 SAY "|  Press the number key to select    |"
    @ 19,10 SAY "+" + REPLICATE("-", 38) + "+"
ENDDO

