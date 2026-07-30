* admin.prg - Supermarket back-office management entry point

SET TALK OFF

* Load all module files so their procedures are registered
DO sm_init
DO sm_menus
DO sm_products
DO sm_providers
DO sm_stock
DO sm_orders
DO sm_reports
DO sm_sales

DO sm_init

gChoice = 0

DO WHILE .T.
  DO sm_main_menu

  DO CASE
    CASE gChoice = 1
      DO sm_products
    CASE gChoice = 2
      DO sm_providers
    CASE gChoice = 3
      DO sm_stock
    CASE gChoice = 4
      DO sm_orders
    CASE gChoice = 5
      DO sm_reports
    CASE gChoice = 6
      DO sm_sales
    CASE gChoice = 0
      ? "Goodbye!"
      RETURN
    OTHERWISE
      ? "Invalid option"
      WAIT
  ENDCASE
ENDDO

CLOSE DATABASES
RETURN
