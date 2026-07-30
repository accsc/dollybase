* stock_adjust.prg - Adjust stock for a product

PROCEDURE stock_adjust
  ACCEPT "Enter barcode: " TO sBarcode
  ACCEPT "Enter quantity to add: " TO sQty

  SELECT 1
  LOCATE FOR A->BARCODE = sBarcode

  IF FOUND()
    REPLACE A->STOCK WITH A->STOCK + VAL(sQty)
    ? "Stock updated. New stock: " + LTRIM(STR(A->STOCK))
  ELSE
    ? "Product not found."
  ENDIF

  WAIT

  RETURN
