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
  SELECT 1
  USE products

  SELECT 2
  USE providers

  SELECT 3
  USE sales

  SELECT 4
  USE salesitems

  SELECT 5
  USE orders

  SELECT 1

  RETURN
