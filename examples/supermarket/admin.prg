* admin.prg - Supermarket back-office management entry point

SET TALK OFF

SET PROCEDURE TO sm_init, sm_main_menu, sm_products, sm_providers, sm_stock, sm_orders, sm_reports, sm_sales, product_add, product_edit, product_list, product_search, product_update_prices, provider_add, provider_list, stock_adjust, stock_low, stock_full, order_generate, order_pending, order_receive, report_low_stock, report_reorder, report_daily_sales, report_sales_ranking, report_category_revenue, sale_counter_reset, sale_record, cart_clear, cart_add, cart_remove_last

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
