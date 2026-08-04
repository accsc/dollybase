* generate_products.prg - Generate 100000 fake products for stress testing
* Uses a LCG pseudo-random number generator (no RANDOM() builtin)
*
* Usage: DO generate_products
*
* IMPORTANT: Run make_sample_data first to create empty DBFs
* This program will DELETE ALL existing records and regenerate

SET TALK OFF

* --- Progress bar setup ---
@ 1, 0 SAY "=== Generating " + STR(TOTAL, 6) + " products ==="
@ 2, 0 SAY "Start: " + TIME()
@ 3, 0 SAY ""
@ 4, 0 SAY ""

* Progress bar row
prog_row = 6

* --- Pseudo-random number generator (LCG) ---
SEED = 12345

* Provider count
PROVIDER_COUNT = 20

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
    DO CASE
        CASE cat_idx = 1  category = "Dairy"
        CASE cat_idx = 2  category = "Bakery"
        CASE cat_idx = 3  category = "Beverages"
        CASE cat_idx = 4  category = "Snacks"
        CASE cat_idx = 5  category = "Produce"
        CASE cat_idx = 6  category = "Meat"
        CASE cat_idx = 7  category = "Seafood"
        CASE cat_idx = 8  category = "Frozen"
        CASE cat_idx = 9  category = "Canned"
        CASE cat_idx = 10 category = "Household"
        CASE cat_idx = 11 category = "Personal Care"
        CASE cat_idx = 12 category = "Baby"
        CASE cat_idx = 13 category = "Pet Supplies"
        CASE cat_idx = 14 category = "Electronics"
        CASE cat_idx = 15 category = "Clothing"
        CASE cat_idx = 16 category = "Sports"
        CASE cat_idx = 17 category = "Books"
        CASE cat_idx = 18 category = "Toys"
        CASE cat_idx = 19 category = "Garden"
        CASE cat_idx = 20 category = "Automotive"
    ENDCASE

    * Noun: pick from category block (5 nouns per group)
    * Using compound conditions to avoid nested DO CASE (parser limitation)
    noun_idx = (rnd3 % 5) + 1
    noun_key = cat_idx * 10 + noun_idx
    DO CASE
        CASE noun_key = 11  noun = "Leche"
        CASE noun_key = 12  noun = "Yogurt"
        CASE noun_key = 13  noun = "Crema"
        CASE noun_key = 14  noun = "Queso"
        CASE noun_key = 15  noun = "Requeson"
        CASE noun_key = 21  noun = "Pan"
        CASE noun_key = 22  noun = "Masa"
        CASE noun_key = 23  noun = "Galleta"
        CASE noun_key = 24  noun = "Bollo"
        CASE noun_key = 25  noun = "Croissant"
        CASE noun_key = 31  noun = "Agua"
        CASE noun_key = 32  noun = "Zumo"
        CASE noun_key = 33  noun = "Te"
        CASE noun_key = 34  noun = "Cafe"
        CASE noun_key = 35  noun = "Infusion"
        CASE noun_key = 41  noun = "Patatas"
        CASE noun_key = 42  noun = "Nuggets"
        CASE noun_key = 43  noun = "Palitos"
        CASE noun_key = 44  noun = "Galletas"
        CASE noun_key = 45  noun = "Barrita"
        CASE noun_key = 51  noun = "Manzanas"
        CASE noun_key = 52  noun = "Platanos"
        CASE noun_key = 53  noun = "Naranjas"
        CASE noun_key = 54  noun = "Tomates"
        CASE noun_key = 55  noun = "Zanahorias"
        CASE noun_key = 61  noun = "Pollo"
        CASE noun_key = 62  noun = "Ternera"
        CASE noun_key = 63  noun = "Cerdo"
        CASE noun_key = 64  noun = "Salchicha"
        CASE noun_key = 65  noun = "Hamburguesa"
        CASE noun_key = 71  noun = "Salmon"
        CASE noun_key = 72  noun = "Atun"
        CASE noun_key = 73  noun = "Merluza"
        CASE noun_key = 74  noun = "Gambas"
        CASE noun_key = 75  noun = "Calamares"
        CASE noun_key = 81  noun = "Pizza"
        CASE noun_key = 82  noun = "Helado"
        CASE noun_key = 83  noun = "Verduras"
        CASE noun_key = 84  noun = "Frutas"
        CASE noun_key = 85  noun = "Empanada"
        CASE noun_key = 91  noun = "Tomate"
        CASE noun_key = 92  noun = "Atun"
        CASE noun_key = 93  noun = "Lentejas"
        CASE noun_key = 94  noun = "Garbanzos"
        CASE noun_key = 95  noun = "Maiz"
        CASE noun_key = 101 noun = "Limpieza"
        CASE noun_key = 102 noun = "Detergente"
        CASE noun_key = 103 noun = "Jabon"
        CASE noun_key = 104 noun = "Papel"
        CASE noun_key = 105 noun = "Esponja"
        CASE noun_key = 111 noun = "Shampoo"
        CASE noun_key = 112 noun = "Crema"
        CASE noun_key = 113 noun = "Protector"
        CASE noun_key = 114 noun = "Pasta"
        CASE noun_key = 115 noun = "Desodorante"
        CASE noun_key = 121 noun = "Pañales"
        CASE noun_key = 122 noun = "Biberon"
        CASE noun_key = 123 noun = "Leche"
        CASE noun_key = 124 noun = "Toallas"
        CASE noun_key = 125 noun = "Champú"
        CASE noun_key = 131 noun = "Croquetas"
        CASE noun_key = 132 noun = "Arena"
        CASE noun_key = 133 noun = "Snack"
        CASE noun_key = 134 noun = "Ración"
        CASE noun_key = 135 noun = "Juguete"
        CASE noun_key = 141 noun = "Cables"
        CASE noun_key = 142 noun = "Auriculares"
        CASE noun_key = 143 noun = "Baterias"
        CASE noun_key = 144 noun = "Cargador"
        CASE noun_key = 145 noun = "Memoria"
        CASE noun_key = 151 noun = "Camiseta"
        CASE noun_key = 152 noun = "Pantalon"
        CASE noun_key = 153 noun = "Calcetines"
        CASE noun_key = 154 noun = "Guantes"
        CASE noun_key = 155 noun = "Bufanda"
        CASE noun_key = 161 noun = "Zapatillas"
        CASE noun_key = 162 noun = "Balón"
        CASE noun_key = 163 noun = "Mancuernas"
        CASE noun_key = 164 noun = "Esterilla"
        CASE noun_key = 165 noun = "Casco"
        CASE noun_key = 171 noun = "Novela"
        CASE noun_key = 172 noun = "Cocina"
        CASE noun_key = 173 noun = "Infantil"
        CASE noun_key = 174 noun = "Historia"
        CASE noun_key = 175 noun = "Guia"
        CASE noun_key = 181 noun = "Puzzle"
        CASE noun_key = 182 noun = "Muneco"
        CASE noun_key = 183 noun = "Juego"
        CASE noun_key = 184 noun = "Construccion"
        CASE noun_key = 185 noun = "Coche"
        CASE noun_key = 191 noun = "Planta"
        CASE noun_key = 192 noun = "Fertilizante"
        CASE noun_key = 193 noun = "Herramienta"
        CASE noun_key = 194 noun = "Manguera"
        CASE noun_key = 195 noun = "Maceta"
        CASE noun_key = 201 noun = "Aceite"
        CASE noun_key = 202 noun = "Filtro"
        CASE noun_key = 203 noun = "Limpia"
        CASE noun_key = 204 noun = "Antena"
        CASE noun_key = 205 noun = "Neumático"
        OTHERWISE           noun = "Producto"
    ENDCASE

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
    DO CASE
        CASE unit_idx <= 3  unit = "unit"
        CASE unit_idx <= 6  unit = "kg"
        CASE unit_idx <= 8  unit = "L"
        CASE unit_idx <= 10 unit = "pack"
        CASE unit_idx <= 12 unit = "box"
        CASE unit_idx <= 13 unit = "can"
        CASE unit_idx <= 15 unit = "bottle"
        CASE unit_idx <= 16 unit = "pair"
        CASE unit_idx <= 17 unit = "roll"
        CASE unit_idx <= 18 unit = "tube"
        OTHERWISE            unit = "bag"
    ENDCASE

    * Tax rate
    tax_idx = (rnd2 % 10) + 1
    DO CASE
        CASE tax_idx <= 2  tax_rate = 4.00
        CASE tax_idx <= 4  tax_rate = 10.00
        OTHERWISE           tax_rate = 21.00
    ENDCASE

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

    * Progress bar update every 500 records
    IF MOD(product_num, 500) = 0
        pct = INT(product_num * 100 / TOTAL)
        filled = INT(product_num * 50 / TOTAL)
        bar_sp = 50 - filled
        bar = "[" + REPLICATE("#", filled) + REPLICATE(" ", bar_sp) + "]"
        @ prog_row, 0 SAY "  " + STR(product_num, 6) + " / " + STR(TOTAL, 6) + "  " + STR(pct, 3) + "%  " + bar
    ENDIF

ENDDO

* Final progress bar (100%)
@ prog_row, 0 SAY "  " + STR(TOTAL, 6) + " / " + STR(TOTAL, 6) + "  100%  [" + REPLICATE("#", 50) + "]"

@ prog_row + 2, 0 SAY "=== Generation complete ==="
@ prog_row + 3, 0 SAY "Total records: " + STR(RECCOUNT())
@ prog_row + 4, 0 SAY "End time: " + TIME()

* Create index on barcode for testing
@ prog_row + 6, 0 SAY "Creating barcode index..."
INDEX ON BARCODE TO products_barcode
@ prog_row + 6, 0 SAY "Creating barcode index... done                    "

* Create index on category for testing
@ prog_row + 7, 0 SAY "Creating category index..."
INDEX ON CATEGORY TO products_category
@ prog_row + 7, 0 SAY "Creating category index... done                   "

CLOSE DATABASES
RETURN
