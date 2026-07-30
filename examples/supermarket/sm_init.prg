* sm_init.prg - Initialize supermarket system
* Opens all databases in their assigned workareas
*
* Workarea mapping:
*   A (1) = Products
*   B (2) = Providers
*   C (3) = Sales
*   D (4) = SalesItems
*   E (5) = Orders

PROCEDURE init_system
  * --- Open Products in workarea A (area 1) ---
  SELECT 1
  USE "data/products"

  * --- Open Providers in workarea B (area 2) ---
  SELECT 2
  USE "data/providers"

  * --- Open Sales in workarea C (area 3) ---
  SELECT 3
  USE "data/sales"

  * --- Open SalesItems in workarea D (area 4) ---
  SELECT 4
  USE "data/salesitems"

  * --- Open Orders in workarea E (area 5) ---
  SELECT 5
  USE "data/orders"

  * Return to default workarea
  SELECT 1

  RETURN
