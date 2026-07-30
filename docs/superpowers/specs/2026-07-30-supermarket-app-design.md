# Supermarket Application — Dollybase dBASE III+ Clone

**Date:** 2026-07-30
**Author:** Alvaro Cortes <alvarocortesc@gmail.com>
**Purpose:** Full-featured supermarket management application to test the limits of the Dollybase dBASE III+ interpreter clone.

---

## Overview

A complete supermarket management system with two entry points:
- **admin.prg** — Back-office management (products, providers, stock, orders, reports, sales history)
- **checkout.prg** — Point-of-sale terminal (barcode scanning, cart, receipt)

Both share a common library of `.prg` modules under `lib/`.

---

## Database Schema

All databases reside in `examples/supermarket/data/`.

### Products.dbf (Workarea A)

| Field | Type | Width | Dec | Description |
|-------|------|-------|-----|-------------|
| BARCODE | C | 13 | - | Unique product barcode |
| NAME | C | 40 | - | Product name |
| CATEGORY | C | 20 | - | Product category |
| PRICE | N | 8 | 2 | Selling price |
| COST | N | 8 | 2 | Acquisition cost |
| STOCK | N | 5 | 0 | Current stock quantity |
| MIN_STOCK | N | 3 | 0 | Reorder threshold |
| PROVIDER_ID | C | 4 | - | Foreign key to Providers.ID |
| ACTIVE | L | 1 | - | .T. = active product |
| UNIT | C | 10 | - | Unit of measure (unit, kg, L, etc.) |
| TAX_RATE | N | 3 | 2 | Tax percentage |
| LAST_SOLD | D | 8 | - | Last sale date |
| REORDER_DATE | D | 8 | - | Last reorder date |
| DESCRIPTION | M | 10 | - | Long description (memo) |

### Providers.dbf (Workarea B)

| Field | Type | Width | Dec | Description |
|-------|------|-------|-----|-------------|
| ID | C | 4 | - | Unique provider ID |
| NAME | C | 40 | - | Provider company name |
| PHONE | C | 15 | - | Contact phone |
| ADDRESS | M | 10 | - | Full address (memo) |

### Sales.dbf (Workarea C)

| Field | Type | Width | Dec | Description |
|-------|------|-------|-----|-------------|
| SALE_ID | C | 10 | - | Unique sale identifier (YYYYMMDDNNN) |
| SALE_DATE | D | 8 | - | Date of sale |
| TOTAL | N | 10 | 2 | Sale total amount |
| ITEMS_COUNT | N | 4 | 0 | Number of line items |
| PAYMENT | C | 10 | - | Payment method (CASH, CARD) |

### SalesItems.dbf (Workarea D)

| Field | Type | Width | Dec | Description |
|-------|------|-------|-----|-------------|
| SALE_ID | C | 10 | - | Links to Sales.SALE_ID |
| BARCODE | C | 13 | - | Product barcode |
| QTY | N | 4 | 0 | Quantity sold |
| UNIT_PRICE | N | 8 | 2 | Price at time of sale |
| LINE_TOTAL | N | 10 | 2 | QTY * UNIT_PRICE |

### Orders.dbf (Workarea E)

| Field | Type | Width | Dec | Description |
|-------|------|-------|-----|-------------|
| ORDER_ID | C | 10 | - | Unique order identifier |
| PROVIDER_ID | C | 4 | - | Links to Providers.ID |
| ORDER_DATE | D | 8 | - | Date order was created |
| STATUS | C | 1 | - | P=pending, S=shipped, R=received |
| NOTES | M | 10 | - | Order details (memo) |

---

## Application Structure

```
examples/supermarket/
├── admin.prg          ← Entry: back-office management
├── checkout.prg       ← Entry: POS checkout terminal
├── lib/
│   ├── init.prg       ← Create DBFs if missing, seed sample data
│   ├── menus.prg      ← Shared menu display routines
│   ├── products.prg   ← Product CRUD (add/edit/list/search/prices)
│   ├── providers.prg  ← Provider CRUD (add/list)
│   ├── stock.prg      ← Stock management, adjustments
│   ├── orders.prg     ← Generate reorder list, create orders
│   ├── reports.prg    ← All 5 report types
│   └── sales.prg      ← Sale recording, sales history lookup
├── data/
│   ├── products.dbf   (+ .dbt for memo)
│   ├── providers.dbf  (+ .dbt for memo)
│   ├── sales.dbf
│   ├── salesitems.dbf
│   └── orders.dbf     (+ .dbt for memo)
```

---

## Admin Menu Flow

```
========================================
   SUPERMARKET MANAGEMENT SYSTEM
========================================

   1. Products
      1.1 Add product
      1.2 Edit product
      1.3 List products
      1.4 Search product
      1.5 Update prices (by barcode or category)

   2. Providers
      2.1 Add provider
      2.2 List providers

   3. Stock Management
      3.1 Adjust stock (incoming)
      3.2 View low stock items
      3.3 Full stock report

   4. Orders
      4.1 Generate reorder list
      4.2 View pending orders
      4.3 Mark order as received

   5. Reports
      5.1 Low stock alert
      5.2 Weekly reorder by provider
      5.3 Daily sales summary
      5.4 Product sales ranking
      5.5 Revenue by category

   6. Sales History
   0. Exit
```

---

## Checkout Flow

1. Screen shows: `Enter barcode [F2=Checkout F3=Remove Esc=Cancel]: `
2. User enters barcode → product looked up in Products.dbf
3. If found: item added to in-memory cart, name and price shown
4. If not found: "Product not found" message
5. Cart shows running total at bottom of screen
6. **F2** — Finalize sale: write to Sales.dbf + SalesItems.dbf, reduce stock, show receipt
7. **F3** — Remove last item from cart
8. **Esc** — Cancel sale, discard cart

### In-Memory Cart Implementation

No arrays available in the interpreter. Cart uses numbered global variables (max 50 items):

- `cART_COUNT` — number of items in cart
- `cART1_BARCODE`, `cART1_QTY`, `cART1_PRICE`, `cART1_NAME`, `cART1_TOTAL`
- `cART2_BARCODE`, ... up to `cART50_...`
- `cART_GRAND_TOTAL` — running total

A helper procedure `cart_add(barcode)` looks up the product and adds it to the next slot.
A helper procedure `cart_clear()` resets all cart variables.

### Sale ID Generation

Format: `YYYYMMDDNNN` where NNN is an incrementing counter.
Example: `20260730001`, `20260730002`.

Counter is tracked in variable `gSaleCounter`, reset each day by comparing `gSaleDate` with `DATE()`.

---

## Module Details

### lib/init.prg

- Checks if each DBF exists (via `DBF()` or file test)
- If missing, creates the database with `CREATE_DATABASE` library function
- Seeds sample data:
  - 10-15 products across categories (dairy, bakery, beverages, snacks, produce)
  - 3-4 providers
  - A few sample sales for report testing
- Called by both `admin.prg` and `checkout.prg` at startup

### lib/products.prg

Procedures:
- `product_add` — @...GET form for all product fields, APPEND BLANK
- `product_edit` — LOCATE FOR BARCODE, then @...GET to edit current record
- `product_list` — GO TOP, DO WHILE .NOT. EOF(), display formatted
- `product_search` — ACCEPT search term, LOCATE with `$` operator
- `product_update_prices` — ACCEPT new price or percentage, REPLACE FOR category/barcode

### lib/providers.prg

Procedures:
- `provider_add` — @...GET form for provider fields
- `provider_list` — DO WHILE loop, display all providers

### lib/stock.prg

Procedures:
- `stock_adjust` — ACCEPT barcode, ACCEPT quantity, REPLACE STOCK WITH STOCK + qty
- `stock_low` — LIST FOR STOCK <= MIN_STOCK, show product name, current stock, min stock
- `stock_full` — Full stock report with all products

### lib/orders.prg

Procedures:
- `order_generate` — LOCATE FOR STOCK <= MIN_STOCK, group by PROVIDER_ID, create order records
- `order_pending` — LIST FOR STATUS = "P"
- `order_receive` — ACCEPT order_id, REPLACE STATUS WITH "R" FOR ORDER_ID = x

### lib/reports.prg

Procedures:
- `report_low_stock` — Products where STOCK <= MIN_STOCK, with provider name
- `report_reorder` — Grouped by provider, with contact info and items to order
- `report_daily_sales` — Sales for today, total revenue, item count
- `report_sales_ranking` — Top selling products by total quantity sold
- `report_category_revenue` — Revenue grouped by product category

### lib/sales.prg

Procedures:
- `sale_record` — Write sale header to Sales.dbf, write items to SalesItems.dbf, reduce stock
- `sale_history` — LIST of sales with date and total
- `sale_counter_reset` — Reset daily counter if date changed

### lib/menus.prg

Procedures:
- `show_main_menu` — @...SAY display, stores selection in `gChoice`
- `show_submenu` — Generic submenu display with title and options

---

## Dollybase Features Exercised

This application exercises the following interpreter capabilities:

- **Database:** USE, SELECT, APPEND BLANK, LOCATE FOR, REPLACE, DELETE, RECALL, PACK, SKIP, GO TOP, GO BOTTOM, EOF(), BOF(), DELETED(), RECCOUNT(), RECNO(), DBF(), LUPDATE()
- **Control flow:** DO WHILE/ENDDO, DO CASE/CASE/OTHERWISE/ENDCASE, IF/ELSE/ENDIF, LOOP, EXIT, RETURN
- **I/O:** ? (print), ACCEPT TO, @...SAY, @...GET, READ, WAIT, INKEY(), CLEAR
- **Functions:** String (LEFT, RIGHT, SUBSTR, STUFF, LEN, LOWER, UPPER, LTRIM, RTRIM, TRIM, REPLICATE, SPACE, AT, ASC, CHR, ISALPHA, ISLOWER, ISUPPER, `$`), Numeric (ABS, EXP, INT, LOG, MAX, MIN, MOD, ROUND, SQRT, STR, VAL, SIGN), Date/Time (DATE, TIME, DAY, MONTH, YEAR, DOW, CDOW, CMONTH, DTOC, CTOD), Misc (EMPTY, TYPE, VERSION, OS, IIF, FIELD)
- **Commands:** STORE, COUNT, AVERAGE, CLEAR ALL, CLEAR MEMORY, SET TALK, TEXT/ENDTEXT, MACRO (&)
- **Structure:** PROCEDURE, PARAMETERS, DO ... WITH, multiple workareas (A->, B->, etc.)
- **Memo fields:** DESCRIPTION, ADDRESS, NOTES with .dbt files

---

## Constraints & Assumptions

- No arrays in the interpreter — cart uses numbered variables
- No `FOR/ENDFOR` — use `DO WHILE/ENDDO`
- No `SLEEP` — use `INKEY()` with timing loop for delays
- ncurses is always available (no `--disable-ncurses` support)
- All string comparisons should use `UPPER()` for case-insensitive matching
- Barcode is the primary lookup key (no INDEX used for simplicity, LOCATE FOR BARCODE = x)
- Sample data is seeded on first run only (check if RECCOUNT() = 0)
