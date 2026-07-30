# Supermarket Application Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a complete supermarket management system in dBASE III+ `.prg` language to test the limits of the Dollybase interpreter clone.

**Architecture:** Two entry points (`admin.prg`, `checkout.prg`) sharing library modules under `lib/`. Five DBF databases across workareas A–E. In-memory cart via numbered variables for the POS terminal. All UI uses ncurses `@...SAY`/`@...GET`/`READ` and `?` print.

**Tech Stack:** Dollybase `.prg` interpreter, libdbase library, ncurses terminal, dBASE III+ language subset.

## Global Constraints

- All files go under `examples/supermarket/`
- Data files go under `examples/supermarket/data/`
- Library modules go under `examples/supermarket/lib/`
- No arrays — use numbered variables for cart (cART1_..., cART2_..., etc.)
- No `FOR/ENDFOR` — use `DO WHILE/ENDDO`
- No `SLEEP` — use `INKEY()` loops for delays
- Workareas: A=Products, B=Providers, C=Sales, D=SalesItems, E=Orders
- Procedures use `PROCEDURE name` / `RETURN` pattern
- Parameter passing via `DO proc WITH args` and `PARAMETERS p1, p2`
- String comparisons use `UPPER()` for case-insensitivity
- Sample data seeded once on first run (check `RECCOUNT() = 0`)
- Programs are run from `examples/supermarket/` directory so relative paths work

---

### Task 1: Database initialization and sample data seeding

**Files:**
- Create: `examples/supermarket/lib/init.prg`
- Create: `examples/supermarket/data/` (directory)

**Interfaces:**
- Produces: `init_system` procedure — creates all 5 DBFs if missing, seeds sample data
- Consumes: nothing (first task)

This task creates the database initialization module. Since `CREATE` is UI-only and requires interactive mode, we use a C helper program (similar to `make_books_memo.c`) to create the DBFs with the correct schema, OR we create them via the `CREATE` command interactively once and commit the `.dbf` files.

The simplest approach: create a C program `examples/supermarket/make_sample_data.c` that creates all 5 DBFs with proper schema and seeds sample data, compile it, and run it once. The `.prg` init module then just opens the databases.

- [ ] **Step 1: Create the data directory**

```bash
mkdir -p examples/supermarket/data
```

- [ ] **Step 2: Write make_sample_data.c**

Create `examples/supermarket/make_sample_data.c` that:
- Creates `products.dbf` with fields: BARCODE(C,13), NAME(C,40), CATEGORY(C,20), PRICE(N,8,2), COST(N,8,2), STOCK(N,5), MIN_STOCK(N,3), PROVIDER_ID(C,4), ACTIVE(L,1), UNIT(C,10), TAX_RATE(N,3,2), LAST_SOLD(D,8), REORDER_DATE(D,8), DESCRIPTION(M,10)
- Creates `providers.dbf` with fields: ID(C,4), NAME(C,40), PHONE(C,15), ADDRESS(M,10)
- Creates `sales.dbf` with fields: SALE_ID(C,10), SALE_DATE(D,8), TOTAL(N,10,2), ITEMS_COUNT(N,4), PAYMENT(C,10)
- Creates `salesitems.dbf` with fields: SALE_ID(C,10), BARCODE(C,13), QTY(N,4), UNIT_PRICE(N,8,2), LINE_TOTAL(N,10,2)
- Creates `orders.dbf` with fields: ORDER_ID(C,10), PROVIDER_ID(C,4), ORDER_DATE(D,8), STATUS(C,1), NOTES(M,10)
- Seeds 12 products across categories (dairy, bakery, beverages, snacks, produce) with realistic barcodes, prices, stock levels (some below MIN_STOCK for reorder testing)
- Seeds 4 providers with IDs P001–P004
- Seeds 3 sample sales with items for report testing

Pattern: follow `int/stress/make_books_memo.c` exactly — build `DATABASEDBF` structs, call `create_database()`, then use `use()` + field writes + `replace()` to seed data.

- [ ] **Step 3: Compile and run make_sample_data.c**

```bash
cd examples/supermarket
gcc -o make_sample_data make_sample_data.c -L../../libdbase_4 -ldbase -lm
./make_sample_data
```

Verify: `ls -la data/` shows all 5 .dbf files (+ .dbt for memo fields).

- [ ] **Step 4: Write lib/init.prg**

```
* lib/init.prg - Initialize supermarket system
* Opens all databases in their assigned workareas

PROCEDURE init_system
  * --- Open Products in workarea A (area 1) ---
  SELECT 1
  USE data/products

  * --- Open Providers in workarea B (area 2) ---
  SELECT 2
  USE data/providers

  * --- Open Sales in workarea C (area 3) ---
  SELECT 3
  USE data/sales

  * --- Open SalesItems in workarea D (area 4) ---
  SELECT 4
  USE data/salesitems

  * --- Open Orders in workarea E (area 5) ---
  SELECT 5
  USE data/orders

  * Return to default workarea
  SELECT 1

  RETURN
```

- [ ] **Step 5: Verify databases open correctly**

Create a quick test PRG:
```
DO lib/init
? "Products: " + LTRIM(STR(A->RECCOUNT()))
SELECT 2
? "Providers: " + LTRIM(STR(RECCOUNT()))
SELECT 1
```

Run: `../../int/prg test_open.prg` — should show seeded record counts.

- [ ] **Step 6: Commit**

```bash
git add examples/supermarket/
git commit -m "supermarket: database schema, sample data, and init module"
```

---

### Task 2: Shared menu system and admin entry point

**Files:**
- Create: `examples/supermarket/lib/menus.prg`
- Create: `examples/supermarket/admin.prg`

**Interfaces:**
- Consumes: `init_system` from Task 1
- Produces: `show_main_menu` (stores result in `gChoice`), `show_submenu` (generic, takes title + options count)

- [ ] **Step 1: Write lib/menus.prg**

```
* lib/menus.prg - Shared menu display routines
* Uses @...SAY for layout, stores selection in gChoice

PROCEDURE show_main_menu
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
```

- [ ] **Step 2: Write admin.prg**

```
* admin.prg - Supermarket back-office management entry point

SET TALK OFF

DO lib/init

gChoice = 0

DO WHILE .T.
  DO lib/menus

  DO CASE
    CASE gChoice = 1
      DO lib/products
    CASE gChoice = 2
      DO lib/providers
    CASE gChoice = 3
      DO lib/stock
    CASE gChoice = 4
      DO lib/orders
    CASE gChoice = 5
      DO lib/reports
    CASE gChoice = 6
      DO lib/sales
    CASE gChoice = 0
      ? "Goodbye!"
      EXIT
    OTHERWISE
      ? "Invalid option"
      WAIT
  ENDCASE
ENDDO

CLOSE DATABASES
RETURN
```

- [ ] **Step 3: Test admin menu displays and exits cleanly**

Run: `cd examples/supermarket && ../../int/prg admin.prg`
Select option 0 — should show splash, menu, "Goodbye!", and END RUN.

- [ ] **Step 4: Commit**

```bash
git add examples/supermarket/lib/menus.prg examples/supermarket/admin.prg
git commit -m "supermarket: admin entry point and main menu"
```

---

### Task 3: Product CRUD module

**Files:**
- Create: `examples/supermarket/lib/products.prg`

**Interfaces:**
- Consumes: databases open in workarea A (Products)
- Produces: `product_menu` (submenu), `product_add`, `product_edit`, `product_list`, `product_search`, `product_update_prices`

- [ ] **Step 1: Write product submenu and add procedure**

```
* lib/products.prg - Product management (CRUD)

PROCEDURE product_menu
  CLEAR
  @  2, 2 SAY "--- PRODUCTS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Add product"
  @  5, 2 SAY "   2. Edit product"
  @  6, 2 SAY "   3. List products"
  @  7, 2 SAY "   4. Search product"
  @  8, 2 SAY "   5. Update prices"
  @  9, 2 SAY "   0. Back"
  @ 10, 2 SAY "   Option: "
  @ 10, 12 GET gSubChoice RANGE 0, 5
  READ

  DO CASE
    CASE gSubChoice = 1
      DO product_add
    CASE gSubChoice = 2
      DO product_edit
    CASE gSubChoice = 3
      DO product_list
    CASE gSubChoice = 4
      DO product_search
    CASE gSubChoice = 5
      DO product_update_prices
  ENDCASE

  RETURN

PROCEDURE product_add
  SELECT 1
  APPEND BLANK

  CLEAR
  @  3, 2 SAY "Barcode:     "
  @  4, 2 SAY "Name:        "
  @  5, 2 SAY "Category:    "
  @  6, 2 SAY "Price:       "
  @  7, 2 SAY "Cost:        "
  @  8, 2 SAY "Stock:       "
  @  9, 2 SAY "Min Stock:   "
  @ 10, 2 SAY "Provider ID: "
  @ 11, 2 SAY "Unit:        "
  @ 12, 2 SAY "Tax Rate:    "
  @ 13, 2 SAY "Description: "

  @  3, 14 GET A->BARCODE PICTURE "!!!!!!!!!!!"
  @  4, 14 GET A->NAME PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  5, 14 GET A->CATEGORY PICTURE "!!!!!!!!!!!!!!!!!!!!"
  @  6, 14 GET A->PRICE RANGE 0, 999999
  @  7, 14 GET A->COST RANGE 0, 999999
  @  8, 14 GET A->STOCK RANGE 0, 99999
  @  9, 14 GET A->MIN_STOCK RANGE 0, 999
  @ 10, 14 GET A->PROVIDER_ID PICTURE "!!!!"
  @ 11, 14 GET A->UNIT PICTURE "!!!!!!!!!!"
  @ 12, 14 GET A->TAX_RATE RANGE 0, 100
  @ 13, 14 GET A->DESCRIPTION

  READ

  A->ACTIVE = .T.

  ? "Product added."
  WAIT

  RETURN
```

- [ ] **Step 2: Write product_edit, product_list, product_search**

```
PROCEDURE product_edit
  ACCEPT "Enter barcode to edit: " TO editBarcode
  SELECT 1
  LOCATE FOR A->BARCODE = editBarcode

  IF .NOT. FOUND()
    ? "Product not found."
    WAIT
    RETURN
  ENDIF

  * Reuse add form to edit current record
  CLEAR
  @  3, 2 SAY "Barcode:     "
  @  4, 2 SAY "Name:        "
  @  5, 2 SAY "Category:    "
  @  6, 2 SAY "Price:       "
  @  7, 2 SAY "Cost:        "
  @  8, 2 SAY "Stock:       "
  @  9, 2 SAY "Min Stock:   "
  @ 10, 2 SAY "Provider ID: "
  @ 11, 2 SAY "Unit:        "
  @ 12, 2 SAY "Tax Rate:    "
  @ 13, 2 SAY "Description: "

  @  3, 14 GET A->BARCODE PICTURE "!!!!!!!!!!!"
  @  4, 14 GET A->NAME PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  5, 14 GET A->CATEGORY PICTURE "!!!!!!!!!!!!!!!!!!!!"
  @  6, 14 GET A->PRICE RANGE 0, 999999
  @  7, 14 GET A->COST RANGE 0, 999999
  @  8, 14 GET A->STOCK RANGE 0, 99999
  @  9, 14 GET A->MIN_STOCK RANGE 0, 999
  @ 10, 14 GET A->PROVIDER_ID PICTURE "!!!!"
  @ 11, 14 GET A->UNIT PICTURE "!!!!!!!!!!"
  @ 12, 14 GET A->TAX_RATE RANGE 0, 100
  @ 13, 14 GET A->DESCRIPTION

  READ

  ? "Product updated."
  WAIT

  RETURN

PROCEDURE product_list
  SELECT 1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   PRODUCT LIST"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". [" + A->BARCODE + "] " + A->NAME
      ? "   Cat: " + A->CATEGORY + "  Price: " + LTRIM(STR(A->PRICE, 8, 2)) + "  Stock: " + LTRIM(STR(A->STOCK))
      ? ""
    ENDIF
    SKIP
  ENDDO

  ? "----------------------------------------"
  ? "Total: " + LTRIM(STR(cnt)) + " products"
  ? "----------------------------------------"
  WAIT

  RETURN

PROCEDURE product_search
  CLEAR
  ACCEPT "Enter search term: " TO searchTerm
  SELECT 1
  GO TOP

  found_cnt = 0
  DO WHILE .NOT. EOF()
    IF UPPER(searchTerm) $ UPPER(A->NAME) .OR. UPPER(searchTerm) $ UPPER(A->BARCODE)
      found_cnt = found_cnt + 1
      ? LTRIM(STR(found_cnt)) + ". [" + A->BARCODE + "] " + A->NAME
      ? "   Price: " + LTRIM(STR(A->PRICE, 8, 2)) + "  Stock: " + LTRIM(STR(A->STOCK))
    ENDIF
    SKIP
  ENDDO

  IF found_cnt = 0
    ? "No products found matching '" + searchTerm + "'"
  ELSE
    ? "Found: " + LTRIM(STR(found_cnt)) + " product(s)"
  ENDIF

  WAIT

  RETURN
```

- [ ] **Step 3: Write product_update_prices**

```
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
    ACCEPT "Enter percentage change (e.g. 10 for +10%): " TO pPct
    SELECT 1
    REPLACE A->PRICE WITH A->PRICE * (1 + VAL(pPct) / 100) FOR UPPER(A->CATEGORY) = UPPER(pCategory)
    ? "Prices updated for category: " + pCategory
  ENDIF

  WAIT

  RETURN
```

- [ ] **Step 4: Test product CRUD**

Run admin.prg, select option 1, test each sub-option:
- Add a new product → verify it appears in list
- Search for it → verify found
- Edit its price → verify change
- List all → verify count increased

- [ ] **Step 5: Commit**

```bash
git add examples/supermarket/lib/products.prg
git commit -m "supermarket: product CRUD module (add/edit/list/search/prices)"
```

---

### Task 4: Provider management module

**Files:**
- Create: `examples/supermarket/lib/providers.prg`

**Interfaces:**
- Consumes: workarea B (Providers)
- Produces: `provider_menu`, `provider_add`, `provider_list`

- [ ] **Step 1: Write lib/providers.prg**

```
* lib/providers.prg - Provider management

PROCEDURE provider_menu
  CLEAR
  @  2, 2 SAY "--- PROVIDERS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Add provider"
  @  5, 2 SAY "   2. List providers"
  @  6, 2 SAY "   0. Back"
  @  7, 2 SAY "   Option: "
  @  7, 12 GET gSubChoice RANGE 0, 2
  READ

  DO CASE
    CASE gSubChoice = 1
      DO provider_add
    CASE gSubChoice = 2
      DO provider_list
  ENDCASE

  RETURN

PROCEDURE provider_add
  SELECT 2
  APPEND BLANK

  CLEAR
  @  3, 2 SAY "Provider ID: "
  @  4, 2 SAY "Name:        "
  @  5, 2 SAY "Phone:       "
  @  6, 2 SAY "Address:     "

  @  3, 15 GET B->ID PICTURE "!!!!"
  @  4, 15 GET B->NAME PICTURE "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"
  @  5, 15 GET B->PHONE PICTURE "!!!!!!!!!!!!!!!"
  @  6, 15 GET B->ADDRESS

  READ

  ? "Provider added."
  WAIT

  RETURN

PROCEDURE provider_list
  SELECT 2
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   PROVIDER LIST"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". [" + B->ID + "] " + B->NAME
      ? "   Phone: " + B->PHONE
      ? ""
    ENDIF
    SKIP
  ENDDO

  ? "Total: " + LTRIM(STR(cnt)) + " providers"
  WAIT

  RETURN
```

- [ ] **Step 2: Test provider module**

Run admin.prg → option 2 → add a provider → list providers → verify

- [ ] **Step 3: Commit**

```bash
git add examples/supermarket/lib/providers.prg
git commit -m "supermarket: provider management module"
```

---

### Task 5: Stock management module

**Files:**
- Create: `examples/supermarket/lib/stock.prg`

**Interfaces:**
- Consumes: workarea A (Products)
- Produces: `stock_menu`, `stock_adjust`, `stock_low`, `stock_full`

- [ ] **Step 1: Write lib/stock.prg**

```
* lib/stock.prg - Stock management

PROCEDURE stock_menu
  CLEAR
  @  2, 2 SAY "--- STOCK MANAGEMENT ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Adjust stock (incoming)"
  @  5, 2 SAY "   2. View low stock items"
  @  6, 2 SAY "   3. Full stock report"
  @  7, 2 SAY "   0. Back"
  @  8, 2 SAY "   Option: "
  @  8, 12 GET gSubChoice RANGE 0, 3
  READ

  DO CASE
    CASE gSubChoice = 1
      DO stock_adjust
    CASE gSubChoice = 2
      DO stock_low
    CASE gSubChoice = 3
      DO stock_full
  ENDCASE

  RETURN

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

PROCEDURE stock_low
  SELECT 1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   LOW STOCK ALERT"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". [" + A->BARCODE + "] " + A->NAME
      ? "   Stock: " + LTRIM(STR(A->STOCK)) + " / Min: " + LTRIM(STR(A->MIN_STOCK))
      ? ""
    ENDIF
    SKIP
  ENDDO

  IF cnt = 0
    ? "All products are above minimum stock levels."
  ELSE
    ? "----------------------------------------"
    ? "Alert: " + LTRIM(STR(cnt)) + " product(s) below minimum"
  ENDIF

  ? "----------------------------------------"
  WAIT

  RETURN

PROCEDURE stock_full
  SELECT 1
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   FULL STOCK REPORT"
  ? "----------------------------------------"
  ? REPLICATE("-", 60)
  ? "Barcode      Name                      Price    Stock  Min"
  ? REPLICATE("-", 60)

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LEFT(A->BARCODE, 13) + "  " + LEFT(A->NAME, 25) + "  " + LTRIM(STR(A->PRICE, 8, 2)) + "  " + LTRIM(STR(A->STOCK, 5)) + "  " + LTRIM(STR(A->MIN_STOCK, 3))
    ENDIF
    SKIP
  ENDDO

  ? REPLICATE("-", 60)
  ? "Total: " + LTRIM(STR(cnt)) + " products"
  ? "----------------------------------------"
  WAIT

  RETURN
```

- [ ] **Step 2: Test stock module**

Run admin.prg → option 3:
- View low stock → should show products seeded with low stock
- Adjust stock → add quantity to a product → verify stock increased
- Full report → verify all products listed

- [ ] **Step 3: Commit**

```bash
git add examples/supermarket/lib/stock.prg
git commit -m "supermarket: stock management module"
```

---

### Task 6: Order generation and management module

**Files:**
- Create: `examples/supermarket/lib/orders.prg`

**Interfaces:**
- Consumes: workarea A (Products), B (Providers), E (Orders)
- Produces: `order_menu`, `order_generate`, `order_pending`, `order_receive`

- [ ] **Step 1: Write lib/orders.prg**

```
* lib/orders.prg - Order management (reorder to providers)

PROCEDURE order_menu
  CLEAR
  @  2, 2 SAY "--- ORDERS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Generate reorder list"
  @  5, 2 SAY "   2. View pending orders"
  @  6, 2 SAY "   3. Mark order as received"
  @  7, 2 SAY "   0. Back"
  @  8, 2 SAY "   Option: "
  @  8, 12 GET gSubChoice RANGE 0, 3
  READ

  DO CASE
    CASE gSubChoice = 1
      DO order_generate
    CASE gSubChoice = 2
      DO order_pending
    CASE gSubChoice = 3
      DO order_receive
  ENDCASE

  RETURN

PROCEDURE order_generate
  * Find all products below minimum stock, group by provider
  * Create an order per provider with pending items

  CLEAR
  ? "Generating reorder list..."
  ? ""

  SELECT 1
  GO TOP

  ord_counter = 0
  today = DTOC(DATE())
  current_provider = ""
  order_items = ""

  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      IF current_provider = ""
        current_provider = A->PROVIDER_ID
      ENDIF

      IF A->PROVIDER_ID != current_provider
        * Save order for previous provider
        SELECT 5
        APPEND BLANK
        ord_counter = ord_counter + 1
        E->ORDER_ID = today + LTRIM(STR(ord_counter, 3, 0))
        E->PROVIDER_ID = current_provider
        E->ORDER_DATE = CTOD(today)
        E->STATUS = "P"
        E->NOTES = order_items
        current_provider = A->PROVIDER_ID
        order_items = ""
      ENDIF

      SELECT 1
      order_items = order_items + A->NAME + " (" + LTRIM(STR(A->MIN_STOCK * 2 - A->STOCK)) + ") "
    ENDIF
    SKIP
  ENDDO

  * Save last provider order if any
  IF order_items != ""
    SELECT 5
    APPEND BLANK
    ord_counter = ord_counter + 1
    E->ORDER_ID = today + LTRIM(STR(ord_counter, 3, 0))
    E->PROVIDER_ID = current_provider
    E->ORDER_DATE = CTOD(today)
    E->STATUS = "P"
    E->NOTES = order_items
  ENDIF

  ? "Generated " + LTRIM(STR(ord_counter)) + " reorder order(s)."
  WAIT

  RETURN

PROCEDURE order_pending
  SELECT 5
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   PENDING ORDERS"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF E->STATUS = "P" .AND. .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". Order: " + E->ORDER_ID
      ? "   Provider: " + E->PROVIDER_ID
      ? "   Date: " + DTOC(E->ORDER_DATE)
      ? "   Items: " + E->NOTES
      ? ""
    ENDIF
    SKIP
  ENDDO

  IF cnt = 0
    ? "No pending orders."
  ELSE
    ? "Total: " + LTRIM(STR(cnt)) + " pending order(s)"
  ENDIF

  ? "----------------------------------------"
  WAIT

  RETURN

PROCEDURE order_receive
  ACCEPT "Enter order ID to mark as received: " TO rOrderId

  SELECT 5
  LOCATE FOR E->ORDER_ID = rOrderId

  IF FOUND()
    REPLACE E->STATUS WITH "R"
    ? "Order " + rOrderId + " marked as received."
  ELSE
    ? "Order not found."
  ENDIF

  WAIT

  RETURN
```

- [ ] **Step 2: Test order module**

Run admin.prg → option 4:
- Generate reorder → should create orders for providers of low-stock products
- View pending → should show the generated orders
- Mark received → verify status changes to "R"

- [ ] **Step 3: Commit**

```bash
git add examples/supermarket/lib/orders.prg
git commit -m "supermarket: order generation and management module"
```

---

### Task 7: Reports module

**Files:**
- Create: `examples/supermarket/lib/reports.prg`

**Interfaces:**
- Consumes: workareas A (Products), B (Providers), C (Sales), D (SalesItems)
- Produces: `report_menu`, `report_low_stock`, `report_reorder`, `report_daily_sales`, `report_sales_ranking`, `report_category_revenue`

- [ ] **Step 1: Write report menu and low stock report**

```
* lib/reports.prg - Business reports

PROCEDURE report_menu
  CLEAR
  @  2, 2 SAY "--- REPORTS ---"
  @  3, 2 SAY ""
  @  4, 2 SAY "   1. Low stock alert"
  @  5, 2 SAY "   2. Weekly reorder by provider"
  @  6, 2 SAY "   3. Daily sales summary"
  @  7, 2 SAY "   4. Product sales ranking"
  @  8, 2 SAY "   5. Revenue by category"
  @  9, 2 SAY "   0. Back"
  @ 10, 2 SAY "   Option: "
  @ 10, 12 GET gSubChoice RANGE 0, 5
  READ

  DO CASE
    CASE gSubChoice = 1
      DO report_low_stock
    CASE gSubChoice = 2
      DO report_reorder
    CASE gSubChoice = 3
      DO report_daily_sales
    CASE gSubChoice = 4
      DO report_sales_ranking
    CASE gSubChoice = 5
      DO report_category_revenue
  ENDCASE

  RETURN

PROCEDURE report_low_stock
  SELECT 1
  GO TOP

  CLEAR
  ? "========================================"
  ? "   LOW STOCK ALERT REPORT"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""
  ? "Barcode      Product                    Stock  Min   Provider"
  ? REPLICATE("-", 70)

  cnt = 0
  total_value = 0
  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      cnt = cnt + 1
      reorder_qty = A->MIN_STOCK * 2 - A->STOCK
      total_value = total_value + reorder_qty * A->COST
      ? LEFT(A->BARCODE, 13) + "  " + LEFT(A->NAME, 25) + "  " + LTRIM(STR(A->STOCK, 5)) + "  " + LTRIM(STR(A->MIN_STOCK, 5)) + "  " + A->PROVIDER_ID
    ENDIF
    SKIP
  ENDDO

  ? REPLICATE("-", 70)
  ? "Products below minimum: " + LTRIM(STR(cnt))
  ? "Estimated reorder cost: " + LTRIM(STR(total_value, 10, 2))
  ? "========================================"
  WAIT

  RETURN
```

- [ ] **Step 2: Write reorder by provider report**

```
PROCEDURE report_reorder
  SELECT 1
  GO TOP

  CLEAR
  ? "========================================"
  ? "   WEEKLY REORDER LIST BY PROVIDER"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""

  current_prov = ""
  total_orders = 0

  DO WHILE .NOT. EOF()
    IF A->STOCK <= A->MIN_STOCK .AND. .NOT. DELETED()
      IF A->PROVIDER_ID != current_prov
        IF current_prov != ""
          ? ""
        ENDIF
        * Look up provider name
        SELECT 2
        LOCATE FOR B->ID = A->PROVIDER_ID
        prov_name = IIF(FOUND(), B->NAME, A->PROVIDER_ID)

        ? "PROVIDER: [" + A->PROVIDER_ID + "] " + prov_name
        ? REPLICATE("-", 50)

        current_prov = A->PROVIDER_ID
      ENDIF

      SELECT 1
      reorder_qty = A->MIN_STOCK * 2 - A->STOCK
      total_orders = total_orders + 1
      ? "  " + LEFT(A->NAME, 30) + "  Qty: " + LTRIM(STR(reorder_qty)) + "  Est: " + LTRIM(STR(reorder_qty * A->COST, 8, 2))
    ENDIF
    SKIP
  ENDDO

  ? ""
  ? "========================================"
  ? "Total items to reorder: " + LTRIM(STR(total_orders))
  ? "========================================"
  WAIT

  RETURN
```

- [ ] **Step 3: Write daily sales, ranking, and category revenue reports**

```
PROCEDURE report_daily_sales
  SELECT 3
  GO TOP

  CLEAR
  ? "========================================"
  ? "   DAILY SALES SUMMARY"
  ? "   Date: " + DTOC(DATE())
  ? "========================================"
  ? ""

  today = DATE()
  total_sales = 0
  total_revenue = 0
  total_items = 0

  DO WHILE .NOT. EOF()
    IF C->SALE_DATE = today .AND. .NOT. DELETED()
      total_sales = total_sales + 1
      total_revenue = total_revenue + C->TOTAL
      total_items = total_items + C->ITEMS_COUNT
      ? "Sale " + C->SALE_ID + "  Items: " + LTRIM(STR(C->ITEMS_COUNT)) + "  Total: " + LTRIM(STR(C->TOTAL, 10, 2)) + "  (" + C->PAYMENT + ")"
    ENDIF
    SKIP
  ENDDO

  ? ""
  ? REPLICATE("-", 50)
  ? "Total sales today: " + LTRIM(STR(total_sales))
  ? "Total items sold:  " + LTRIM(STR(total_items))
  ? "Total revenue:     " + LTRIM(STR(total_revenue, 10, 2))
  ? "========================================"
  WAIT

  RETURN

PROCEDURE report_sales_ranking
  * Aggregate total quantity sold per product from SalesItems
  SELECT 4
  GO TOP

  CLEAR
  ? "========================================"
  ? "   PRODUCT SALES RANKING"
  ? "========================================"
  ? ""
  ? "Rank  Barcode      Product                    Qty Sold  Revenue"
  ? REPLICATE("-", 70)

  * Simple approach: iterate sales items, accumulate per product
  * Since we can't use arrays, we do a single pass and show raw data
  * Group by barcode manually

  rank = 0
  current_bc = ""
  current_qty = 0
  current_rev = 0

  DO WHILE .NOT. EOF()
    IF D->BARCODE != current_bc
      IF current_bc != ""
        rank = rank + 1
        * Look up product name
        SELECT 1
        LOCATE FOR A->BARCODE = current_bc
        prod_name = IIF(FOUND(), A->NAME, current_bc)
        SELECT 4
        ? LTRIM(STR(rank, 4)) + "  " + LEFT(current_bc, 13) + "  " + LEFT(prod_name, 25) + "  " + LTRIM(STR(current_qty, 6)) + "  " + LTRIM(STR(current_rev, 10, 2))
      ENDIF
      current_bc = D->BARCODE
      current_qty = D->QTY
      current_rev = D->LINE_TOTAL
    ELSE
      current_qty = current_qty + D->QTY
      current_rev = current_rev + D->LINE_TOTAL
    ENDIF
    SKIP
  ENDDO

  * Print last group
  IF current_bc != ""
    rank = rank + 1
    SELECT 1
    LOCATE FOR A->BARCODE = current_bc
    prod_name = IIF(FOUND(), A->NAME, current_bc)
    SELECT 4
    ? LTRIM(STR(rank, 4)) + "  " + LEFT(current_bc, 13) + "  " + LEFT(prod_name, 25) + "  " + LTRIM(STR(current_qty, 6)) + "  " + LTRIM(STR(current_rev, 10, 2))
  ENDIF

  ? REPLICATE("-", 70)
  ? "Total products sold: " + LTRIM(STR(rank))
  ? "========================================"
  WAIT

  RETURN

PROCEDURE report_category_revenue
  SELECT 4
  GO TOP

  CLEAR
  ? "========================================"
  ? "   REVENUE BY CATEGORY"
  ? "========================================"
  ? ""
  ? "Category                 Revenue       Products"
  ? REPLICATE("-", 55)

  * For each sales item, look up product category and accumulate
  total_rev = 0
  DO WHILE .NOT. EOF()
    * This is a simplified approach - in a real app we'd pre-aggregate
    SELECT 1
    LOCATE FOR A->BARCODE = D->BARCODE
    IF FOUND()
      current_cat = A->CATEGORY
    ELSE
      current_cat = "UNKNOWN"
    ENDIF
    SELECT 4
    total_rev = total_rev + D->LINE_TOTAL
    ? "  " + LEFT(current_cat, 23) + "  " + LTRIM(STR(D->LINE_TOTAL, 10, 2)) + "  (item: " + LEFT(D->BARCODE, 13) + ")"
    SKIP
  ENDDO

  ? REPLICATE("-", 55)
  ? "Total revenue: " + LTRIM(STR(total_rev, 12, 2))
  ? "========================================"
  WAIT

  RETURN
```

- [ ] **Step 4: Test reports**

Run admin.prg → option 5:
- Low stock alert → should show seeded low-stock products
- Reorder by provider → should group by provider
- Daily sales → should show sample sales (if any from today)
- Sales ranking → should show products from sample sales
- Category revenue → should show revenue breakdown

- [ ] **Step 5: Commit**

```bash
git add examples/supermarket/lib/reports.prg
git commit -m "supermarket: reports module (5 report types)"
```

---

### Task 8: Sales history module

**Files:**
- Create: `examples/supermarket/lib/sales.prg`

**Interfaces:**
- Consumes: workarea C (Sales), D (SalesItems)
- Produces: `sale_history` (display), `sale_record` (write sale + items, reduce stock), `sale_counter_reset`

- [ ] **Step 1: Write lib/sales.prg**

```
* lib/sales.prg - Sale recording and history

PROCEDURE sale_history
  SELECT 3
  GO TOP

  CLEAR
  ? "----------------------------------------"
  ? "   SALES HISTORY"
  ? "----------------------------------------"
  ? ""

  cnt = 0
  DO WHILE .NOT. EOF()
    IF .NOT. DELETED()
      cnt = cnt + 1
      ? LTRIM(STR(cnt)) + ". " + C->SALE_ID + "  Date: " + DTOC(C->SALE_DATE) + "  Items: " + LTRIM(STR(C->ITEMS_COUNT)) + "  Total: " + LTRIM(STR(C->TOTAL, 10, 2)) + "  (" + C->PAYMENT + ")"
    ENDIF
    SKIP
  ENDDO

  ? "----------------------------------------"
  ? "Total: " + LTRIM(STR(cnt)) + " sales"
  ? "----------------------------------------"
  WAIT

  RETURN

PROCEDURE sale_counter_reset
  * Reset daily counter if date changed
  IF gSaleDate != DTOC(DATE())
    gSaleDate = DTOC(DATE())
    gSaleCounter = 0
  ENDIF
  RETURN

PROCEDURE sale_record
  * Parameters: payment method
  * Uses global cart variables: cART_COUNT, cART1_BARCODE..cART50_..., cART_GRAND_TOTAL

  DO sale_counter_reset

  gSaleCounter = gSaleCounter + 1
  sale_id = gSaleDate + LTRIM(STR(gSaleCounter, 3, 0))

  * Write sale header to Sales.dbf
  SELECT 3
  APPEND BLANK
  C->SALE_ID = sale_id
  C->SALE_DATE = DATE()
  C->TOTAL = cART_GRAND_TOTAL
  C->ITEMS_COUNT = cART_COUNT
  C->PAYMENT = gPaymentMethod

  * Write each cart item to SalesItems.dbf and reduce stock
  i = 1
  DO WHILE i <= cART_COUNT
    * Get cart item via macro expansion
    cart_var = "cART" + LTRIM(STR(i)) + "_BARCODE"
    item_barcode = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_QTY"
    item_qty = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_PRICE"
    item_price = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_TOTAL"
    item_total = &cart_var

    * Write to SalesItems
    SELECT 4
    APPEND BLANK
    D->SALE_ID = sale_id
    D->BARCODE = item_barcode
    D->QTY = item_qty
    D->UNIT_PRICE = item_price
    D->LINE_TOTAL = item_total

    * Reduce stock in Products
    SELECT 1
    LOCATE FOR A->BARCODE = item_barcode
    IF FOUND()
      REPLACE A->STOCK WITH A->STOCK - item_qty
      REPLACE A->LAST_SOLD WITH DATE()
    ENDIF

    i = i + 1
  ENDDO

  * Clear cart
  DO cart_clear

  RETURN
```

- [ ] **Step 2: Test sales history**

Run admin.prg → option 6 → should show sample sales seeded in Task 1

- [ ] **Step 3: Commit**

```bash
git add examples/supermarket/lib/sales.prg
git commit -m "supermarket: sales recording and history module"
```

---

### Task 9: Checkout (POS) terminal

**Files:**
- Create: `examples/supermarket/checkout.prg`

**Interfaces:**
- Consumes: `init_system` (Task 1), `sale_record` (Task 8), workarea A (Products)
- Produces: Full POS terminal with barcode entry, cart display, receipt

- [ ] **Step 1: Write checkout.prg with cart helpers and main loop**

```
* checkout.prg - Point-of-sale terminal

SET TALK OFF

DO lib/init

* Initialize cart
DO cart_clear
cART_GRAND_TOTAL = 0
gSaleDate = DTOC(DATE())
gSaleCounter = 0

CLEAR
@  1, 1 SAY REPLICATE("=", 70)
@  2, 1 SAY "   CHECKOUT TERMINAL - Supermarket POS"
@  3, 1 SAY REPLICATE("=", 70)

* Main checkout loop
DO WHILE .T.
  * Display cart summary (items 1-10 visible)
  @  5, 1 SAY REPLICATE("-", 70)
  @  5, 1 SAY "  Barcode        Name                      Qty   Price    Total"
  @  5, 1 SAY REPLICATE("-", 70)

  row = 6
  display_total = 0
  i = 1
  max_show = MIN(cART_COUNT, 15)
  DO WHILE i <= max_show .AND. row <= 20
    cart_var = "cART" + LTRIM(STR(i)) + "_BARCODE"
    d_barcode = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_NAME"
    d_name = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_QTY"
    d_qty = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_PRICE"
    d_price = &cart_var

    cart_var = "cART" + LTRIM(STR(i)) + "_TOTAL"
    d_total = &cart_var

    @ row, 1 SAY "  " + LEFT(d_barcode, 14) + "  " + LEFT(d_name, 24) + "  " + LTRIM(STR(d_qty, 3)) + "  " + LTRIM(STR(d_price, 8, 2)) + "  " + LTRIM(STR(d_total, 8, 2))
    display_total = display_total + d_total
    row = row + 1
    i = i + 1
  ENDDO

  @ 22, 1 SAY REPLICATE("-", 70)
  @ 22, 1 SAY "  Items: " + LTRIM(STR(cART_COUNT)) + "   TOTAL: " + LTRIM(STR(cART_GRAND_TOTAL, 10, 2))
  @ 23, 1 SAY REPLICATE("-", 70)
  @ 24, 1 SAY "  [Barcode] [F2=Checkout] [F3=Remove last] [Esc=Cancel]"

  * Wait for input
  key = INKEY()

  IF key = 3
    * Escape - cancel
    DO cart_clear
    EXIT
  ENDIF

  IF key = 154
    * F2 - checkout
    IF cART_COUNT > 0
      gPaymentMethod = "CASH"
      DO sale_record
      ? ""
      ? "========================================"
      ? "   RECEIPT - Sale " + sale_id
      ? "========================================"
      ? "Total: " + LTRIM(STR(cART_GRAND_TOTAL, 10, 2))
      ? "Items: " + LTRIM(STR(cART_COUNT))
      ? "Thank you!"
      ? "========================================"
      WAIT
      DO cart_clear
      cART_GRAND_TOTAL = 0
    ENDIF
    LOOP
  ENDIF

  IF key = 155
    * F3 - remove last item
    IF cART_COUNT > 0
      DO cart_remove_last
    ENDIF
    LOOP
  ENDIF

  * If regular key press, treat as barcode entry
  IF key > 31 .AND. key < 127
    * Get barcode from user
    ACCEPT "Barcode: " TO inputBarcode
    SELECT 1
    LOCATE FOR A->BARCODE = inputBarcode

    IF FOUND()
      * Add to cart
      DO cart_add WITH inputBarcode, A->NAME, 1, A->PRICE
      LOOP
    ELSE
      ? "Product not found: " + inputBarcode
      WAIT
    ENDIF
  ENDIF
ENDDO

CLEAR DATABASES
RETURN

PROCEDURE cart_clear
  STORE 0 TO cART_COUNT, cART_GRAND_TOTAL
  * Clear individual cart slots
  i = 1
  DO WHILE i <= 50
    cart_var = "cART" + LTRIM(STR(i)) + "_BARCODE"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(i)) + "_NAME"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(i)) + "_QTY"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(i)) + "_PRICE"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(i)) + "_TOTAL"
    &cart_var = 0
    i = i + 1
  ENDDO
  RETURN

PROCEDURE cart_add
  PARAMETERS pBarcode, pName, pQty, pPrice

  cART_COUNT = cART_COUNT + 1
  idx = cART_COUNT

  cart_var = "cART" + LTRIM(STR(idx)) + "_BARCODE"
  &cart_var = pBarcode

  cart_var = "cART" + LTRIM(STR(idx)) + "_NAME"
  &cart_var = pName

  cart_var = "cART" + LTRIM(STR(idx)) + "_QTY"
  &cart_var = pQty

  cart_var = "cART" + LTRIM(STR(idx)) + "_PRICE"
  &cart_var = pPrice

  cart_var = "cART" + LTRIM(STR(idx)) + "_TOTAL"
  &cart_var = pQty * pPrice

  cART_GRAND_TOTAL = cART_GRAND_TOTAL + (pQty * pPrice)

  RETURN

PROCEDURE cart_remove_last
  IF cART_COUNT > 0
    idx = cART_COUNT
    cart_var = "cART" + LTRIM(STR(idx)) + "_TOTAL"
    item_total = &cart_var
    cART_GRAND_TOTAL = cART_GRAND_TOTAL - item_total
    cART_COUNT = cART_COUNT - 1

    * Clear the slot
    cart_var = "cART" + LTRIM(STR(idx)) + "_BARCODE"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(idx)) + "_NAME"
    &cart_var = ""
    cart_var = "cART" + LTRIM(STR(idx)) + "_QTY"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(idx)) + "_PRICE"
    &cart_var = 0
    cart_var = "cART" + LTRIM(STR(idx)) + "_TOTAL"
    &cart_var = 0
  ENDIF
  RETURN
```

- [ ] **Step 2: Test checkout terminal**

Run: `cd examples/supermarket && ../../int/prg checkout.prg`
- Enter a seeded barcode → verify product appears in cart
- Enter another barcode → verify cart shows 2 items
- Press F3 → verify last item removed
- Press F2 → verify receipt shows, stock reduced, sale recorded
- Press Esc → verify cart cleared, exit

- [ ] **Step 3: Verify sales were recorded**

Run admin.prg → option 6 (Sales History) → verify new sale appears
Run admin.prg → option 3.2 (Low stock) → verify stock reduced

- [ ] **Step 4: Commit**

```bash
git add examples/supermarket/checkout.prg
git commit -m "supermarket: checkout POS terminal with cart and receipt"
```

---

### Task 10: Integration testing and polish

**Files:**
- Modify: any module with bugs found during integration testing
- Create: `examples/supermarket/README.prg` (optional usage guide as a PRG that prints help)

**Interfaces:**
- Consumes: all modules from Tasks 1-9

- [ ] **Step 1: Full admin workflow test**

Run through the complete admin flow:
1. Start admin.prg
2. Add a new product (option 1.1)
3. List products (option 1.3) — verify new product appears
4. Search for the new product (option 1.4) — verify found
5. Update its price (option 1.5) — verify price changed
6. Add a new provider (option 2.1)
7. List providers (option 2.2) — verify new provider
8. View low stock (option 3.2) — verify alert works
9. Generate reorder (option 4.1) — verify orders created
10. View pending orders (option 4.2) — verify orders listed
11. Run all 5 reports (option 5.x) — verify each produces output
12. View sales history (option 6) — verify sales listed
13. Exit (option 0) — verify clean exit

- [ ] **Step 2: Full checkout workflow test**

1. Start checkout.prg
2. Scan 3 different products (enter barcodes)
3. Verify cart shows all 3 items with correct totals
4. Remove one item (F3) — verify cart updated
5. Checkout (F2) — verify receipt, stock reduced
6. Verify in admin that sale was recorded and stock updated

- [ ] **Step 3: Fix any bugs found during testing**

Common issues to watch for:
- Workarea not selected before field access (always SELECT before LOCATE/REPLACE)
- Macro expansion (`&`) with empty variables causing errors
- Date comparison issues (CTOD/DTOC round-trip)
- STR() formatting with wrong parameters

- [ ] **Step 4: Final commit**

```bash
git add examples/supermarket/
git commit -m "supermarket: integration fixes and final polish"
```

---

## Self-Review Checklist

**Spec coverage:**
- [x] Database schema — Task 1 (all 5 DBFs with correct fields)
- [x] Admin entry point — Task 2
- [x] Product CRUD — Task 3
- [x] Provider CRUD — Task 4
- [x] Stock management — Task 5
- [x] Order generation — Task 6
- [x] All 5 reports — Task 7
- [x] Sales history — Task 8
- [x] Checkout POS — Task 9
- [x] Integration testing — Task 10

**Placeholder scan:** No TBDs, no "implement later", no vague steps. Each step has concrete code or commands.

**Type consistency:** Workarea aliases (A->, B->, C->, D->, E->) consistent across all tasks. Cart variable naming (cART_N_FIELD) consistent. Procedure naming (module_action) consistent.

**Scope check:** Focused on one application. 10 tasks, each producing a testable deliverable. Tasks 1-3 can be reviewed independently of Tasks 7-10.

---
