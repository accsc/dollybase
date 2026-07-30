* sm_menus.prg - Shared menu display routines
* Uses @...SAY for layout, stores selection in gChoice

PROCEDURE sm_main_menu
  CLEAR

  @  1, 2 SAY "========================================"
  @  2, 2 SAY "   SUPERMARKET MANAGEMENT SYSTEM"
  @  3, 2 SAY "========================================"
  @  4, 2 SAY ""
  @  5, 2 SAY "   1. Products"
  @  6, 2 SAY "   2. Providers"
  @  7, 2 SAY "   3. Stock Management"
  @  8, 2 SAY "   4. Orders"
  @  9, 2 SAY "   5. Reports"
  @ 10, 2 SAY "   6. Sales History"
  @ 11, 2 SAY "   0. Exit"
  @ 12, 2 SAY ""
  @ 13, 2 SAY "========================================"
  @ 14, 2 SAY "   Select an option: "
  @ 14, 22 GET gChoice RANGE 0, 6
  READ

  RETURN
