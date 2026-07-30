* product_update_prices.prg - Update product prices

PROCEDURE product_update_prices
  CLEAR
  ? "Price update mode:"
  ? "1 = Update by barcode"
  ? "2 = Update by category (percentage)"
  ACCEPT "Choose mode (1/2): " TO priceMode

  IF priceMode = "1"
    ACCEPT "Enter barcode: " TO pBarcode
    ACCEPT "Enter new price: " TO pPrice
    SELECT 1
    REPLACE A->PRICE WITH VAL(pPrice) FOR A->BARCODE = pBarcode
    ? "Price updated for barcode " + pBarcode
  ELSE
    ACCEPT "Enter category: " TO pCategory
    ACCEPT "Enter percentage change (e.g. 10 for +10%%): " TO pPct
    SELECT 1
    REPLACE A->PRICE WITH A->PRICE * (1 + VAL(pPct) / 100) FOR UPPER(A->CATEGORY) = UPPER(pCategory)
    ? "Prices updated for category: " + pCategory
  ENDIF

  WAIT

  RETURN
