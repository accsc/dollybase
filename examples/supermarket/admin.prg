* admin.prg - Supermarket back-office management entry point

SET TALK OFF

* Load all module files so their procedures are registered
* Only load leaf procedures — menu procedures are loaded on first call
DO sm_init
DO product_add
DO product_edit
DO product_list
DO product_search
DO product_update_prices
DO provider_add
DO provider_list
DO stock_adjust
DO stock_low
DO stock_full
DO order_generate
DO order_pending
DO order_receive
DO report_low_stock
DO report_reorder
DO report_daily_sales
DO report_sales_ranking
DO report_category_revenue
DO sm_sales
DO sale_counter_reset
DO sale_record
DO cart_clear
DO cart_add
DO cart_remove_last

DO init_system

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
