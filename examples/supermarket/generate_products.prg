* generate_products.prg - Generate 100000 fake products for stress testing
* Uses a LCG pseudo-random number generator (no RANDOM() builtin)
*
* Usage: DO generate_products
*
* IMPORTANT: Run make_sample_data first to create empty DBFs
* This program will DELETE ALL existing records and regenerate

SET TALK OFF
SET PROCEDURE TO gen_helpers

? "=== Generating 100000 products ==="
? "Start time: " + TIME()

* --- Pseudo-random number generator (LCG) ---
* SEED is a global variable mutated by DO rand_next
SEED = 12345

* --- Data tables stored as pipe-delimited strings ---
* 20 categories, 10 entries each
CATEGORY_LIST = "Dairy|Bakery|Beverages|Snacks|Produce|Meat|Seafood|Frozen|Canned|Household|Personal Care|Baby|Pet Supplies|Electronics|Clothing|Sports|Books|Toys|Garden|Automotive"

* Provider IDs
PROVIDER_COUNT = 20

* Units
UNIT_LIST = "unit|unit|unit|kg|kg|kg|L|L|pack|pack|box|box|can|bottle|bottle|pair|roll|tube|bag|bag"

* Tax rates (Spain VAT: 4% reduced, 10% super-reduced, 21% standard)
TAX_LIST = "4.00|4.00|10.00|10.00|21.00|21.00|21.00|4.00|10.00|21.00"

* --- Product name nouns per category (pipe-delimited, 5 nouns each) ---
* 20 categories x 5 nouns = 100 entries
NOUN_LIST = "Leche|Yogurt|Crema|Queso|Requeson|Pan|Masa|Galleta|Bollo|Croissant|Agua|Zumo|Te|Cafe|Infusion|Patatas|Nuggets|Palitos|Galletas|Barrita|Manzanas|Platanos|Naranjas|Tomates|Zanahorias|Pollo|Ternera|Cerdo|Salchicha|Hamburguesa|Salmon|Atun|Merluza|Gambas|Calamares|Pizza|Helado|Verduras|Frutas|Empanada|Tomate|Atun|Lentejas|Garbanzos|Maiz|Limpieza|Detergente|Jabon|Papel|Esponja|Shampoo|Crema|Protector|Pasta|Desodorante|Pañales|Biberon|Leche|Toallas|Champú|Croquetas|Arena|Snack|Ración|Juguete|Cables|Auriculares|Baterias|Cargador|Memoria|Camiseta|Pantalon|Calcetines|Guantes|Bufanda|Zapatillas|Balón|Mancuernas|Esterilla|Casco|Novela|Cocina|Infantil|Historia|Guia|Puzzle|Muneco|Juego|Construccion|Coche|Planta|Fertilizante|Herramienta|Manguera|Maceta|Aceite|Filtro|Limpia|Antena|Neumático"

* --- Initialize ---
SELECT 1
USE products

DELETE ALL
PACK

* --- Generate products in batches of 1000 for progress reporting ---
TOTAL = 100000
BATCH = 1000
product_num = 0

DO WHILE product_num < TOTAL
    product_num = product_num + 1

    * --- Advance LCG ---
    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd1 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd2 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd3 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd4 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd5 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd6 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd7 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd8 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd9 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    rnd10 = MOD(INT(SEED / 65536), 100000)

    * Generate barcode: 841234567 + 5 zero-padded digits
    bc_num = rnd1 % 10000
    bc_str = ALLTRIM(STR(bc_num))
    bc_pad = 4 - LEN(bc_str)
    IF bc_pad < 0
        bc_pad = 0
    ENDIF
    bc4 = REPLICATE("0", bc_pad) + bc_str
    bc_prefix_num = INT(product_num / 10000) + 1
    bc_prefix = ALLTRIM(STR(bc_prefix_num))
    barcode = "84123456" + bc_prefix + bc4

    * Category index 1-20
    cat_idx = (rnd2 % 20) + 1
    g_pp_str = CATEGORY_LIST
    DO pick_pipe WITH cat_idx
    category = g_pp_result

    * Noun: pick from category block in NOUN_LIST (5 nouns per group)
    noun_idx = (rnd3 % 5) + 1
    noun_base_idx = (cat_idx - 1) * 5 + 1
    g_pp_str = NOUN_LIST
    DO pick_pipe WITH noun_base_idx + noun_idx - 1
    noun = g_pp_result

    * Size/weight suffix
    size_idx = (rnd4 % 8) + 1
    name = noun
    DO CASE
        CASE size_idx = 1
            name = name + " 1kg"
        CASE size_idx = 2
            name = name + " 500g"
        CASE size_idx = 3
            name = name + " 250g"
        CASE size_idx = 4
            name = name + " 1L"
        CASE size_idx = 5
            name = name + " 2L"
        CASE size_idx = 6
            name = name + " x4"
        CASE size_idx = 7
            name = name + " x6"
        CASE size_idx = 8
            name = name + " Premium"
    ENDCASE

    * Price: 0.10 to 99.99
    price = ((rnd5 % 999) + 10) / 100.0

    * Cost: 40-70% of price
    cost = price * ((rnd6 % 30) + 40) / 100.0

    * Stock: 0-500
    stock = rnd7 % 501

    * Min stock: 5-50
    min_stock = (rnd8 % 46) + 5

    * Provider ID: P001-P020
    prov_idx = (rnd9 % PROVIDER_COUNT) + 1
    prov_str = ALLTRIM(STR(prov_idx))
    prov_pad = 3 - LEN(prov_str)
    IF prov_pad < 0
        prov_pad = 0
    ENDIF
    prov_id = "P" + REPLICATE("0", prov_pad) + prov_str

    * Active: mostly true (90%)
    active = ((rnd10 % 10) < 9)

    * Unit
    unit_idx = (rnd1 % 20) + 1
    g_pp_str = UNIT_LIST
    DO pick_pipe WITH unit_idx
    unit = g_pp_result

    * Tax rate
    tax_idx = (rnd2 % 10) + 1
    g_pp_str = TAX_LIST
    DO pick_pipe WITH tax_idx
    tax_rate = VAL(g_pp_result)

    * Last sold: random date in 2025-2026
    sold_day = (rnd3 % 28) + 1
    sold_mn = (rnd4 % 12) + 1
    sold_yr = 2025 + (rnd5 % 2)
    last_sold = CTOD(STR(sold_mn) + "/" + STR(sold_day) + "/" + STR(sold_yr))

    * Reorder date
    reorder_day = (rnd6 % 28) + 1
    reorder_mn = (rnd7 % 12) + 1
    reorder_yr = 2026 + (rnd8 % 2)
    reorder_date = CTOD(STR(reorder_mn) + "/" + STR(reorder_day) + "/" + STR(reorder_yr))

    * Description (memo field) - keep short to avoid 64K DBT block issues
    desc = noun + " - Producto " + category + " proveedor " + prov_id

    APPEND BLANK
    REPLACE BARCODE WITH barcode, NAME WITH name, CATEGORY WITH category, PRICE WITH price, COST WITH cost, STOCK WITH stock, MIN_STOCK WITH min_stock, PROVIDER_ID WITH prov_id, ACTIVE WITH active, UNIT WITH unit, TAX_RATE WITH tax_rate, LAST_SOLD WITH last_sold, REORDER_DATE WITH reorder_date
    REPLACE DESCRIPTION WITH desc

    * Progress every 1000 records
    IF MOD(product_num, BATCH) = 0
        ? "  Generated " + STR(product_num, 6) + " / " + STR(TOTAL, 6) + " products..."
    ENDIF

ENDDO

? ""
? "=== Generation complete ==="
? "Total records: " + STR(RECCOUNT())
? "End time: " + TIME()

* Create index on barcode for testing
INDEX ON BARCODE TO products_barcode

? "Index created: products_barcode.ndx"

* Create index on category for testing
INDEX ON CATEGORY TO products_category

? "Index created: products_category.ndx"

CLOSE DATABASES
RETURN
