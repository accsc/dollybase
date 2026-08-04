* generate_products.prg - Generate fake products for stress testing
* Uses a LCG pseudo-random number generator (no RANDOM() builtin)
*
* Usage: DO generate_products
*
* IMPORTANT: Run make_sample_data first to create empty DBFs
* This program will DELETE ALL existing records and regenerate
*
* NOTE: Memory variables use m_ prefix to avoid shadowing DBF field names.
*   In xBase, field names take precedence over variables when a DBF is open.

SET TALK OFF

* --- Pseudo-random number generator (LCG) ---
SEED = 12345

* Provider count
m_provider_count = 20

* --- Total records to generate ---
m_total = 1000

* --- Initialize ---
SELECT 1
USE products

DELETE ALL
PACK

* --- Progress bar setup (after m_total is set) ---
@ 1, 0 SAY "=== Generating " + STR(m_total, 6) + " products ==="
@ 2, 0 SAY "Start: " + TIME()
@ 3, 0 SAY ""
@ 4, 0 SAY ""

* Progress bar row
prog_row = 6
m_product_num = 0

DO WHILE m_product_num < m_total
    m_product_num = m_product_num + 1

    * --- Advance LCG ---
    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd1 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd2 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd3 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd4 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd5 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd6 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd7 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd8 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd9 = MOD(INT(SEED / 65536), 100000)

    SEED = MOD(SEED * 1103515245 + 12345, 2147483648)
    m_rnd10 = MOD(INT(SEED / 65536), 100000)

    * Generate barcode: 84123456 + prefix + 4-digit zero-padded number
    m_bc_num = m_rnd1 % 10000
    m_bc_str = ALLTRIM(STR(m_bc_num))
    m_bc_pad = 4 - LEN(m_bc_str)
    IF m_bc_pad < 0
        m_bc_pad = 0
    ENDIF
    m_bc4 = REPLICATE("0", m_bc_pad) + m_bc_str
    m_bc_prefix_num = INT(m_product_num / 10000) + 1
    m_bc_prefix = ALLTRIM(STR(m_bc_prefix_num))
    m_barcode = "84123456" + m_bc_prefix + m_bc4

    * Category index 1-20
    m_cat_idx = (m_rnd2 % 20) + 1
    m_category = ""
    DO CASE
        CASE m_cat_idx = 1  m_category = "Dairy"
        CASE m_cat_idx = 2  m_category = "Bakery"
        CASE m_cat_idx = 3  m_category = "Beverages"
        CASE m_cat_idx = 4  m_category = "Snacks"
        CASE m_cat_idx = 5  m_category = "Produce"
        CASE m_cat_idx = 6  m_category = "Meat"
        CASE m_cat_idx = 7  m_category = "Seafood"
        CASE m_cat_idx = 8  m_category = "Frozen"
        CASE m_cat_idx = 9  m_category = "Canned"
        CASE m_cat_idx = 10 m_category = "Household"
        CASE m_cat_idx = 11 m_category = "Personal Care"
        CASE m_cat_idx = 12 m_category = "Baby"
        CASE m_cat_idx = 13 m_category = "Pet Supplies"
        CASE m_cat_idx = 14 m_category = "Electronics"
        CASE m_cat_idx = 15 m_category = "Clothing"
        CASE m_cat_idx = 16 m_category = "Sports"
        CASE m_cat_idx = 17 m_category = "Books"
        CASE m_cat_idx = 18 m_category = "Toys"
        CASE m_cat_idx = 19 m_category = "Garden"
        CASE m_cat_idx = 20 m_category = "Automotive"
    ENDCASE

    * Noun: pick from category block (5 nouns per group)
    m_noun_idx = (m_rnd3 % 5) + 1
    m_noun_key = m_cat_idx * 10 + m_noun_idx
    m_noun = ""
    DO CASE
        CASE m_noun_key = 11  m_noun = "Leche"
        CASE m_noun_key = 12  m_noun = "Yogurt"
        CASE m_noun_key = 13  m_noun = "Crema"
        CASE m_noun_key = 14  m_noun = "Queso"
        CASE m_noun_key = 15  m_noun = "Requeson"
        CASE m_noun_key = 21  m_noun = "Pan"
        CASE m_noun_key = 22  m_noun = "Masa"
        CASE m_noun_key = 23  m_noun = "Galleta"
        CASE m_noun_key = 24  m_noun = "Bollo"
        CASE m_noun_key = 25  m_noun = "Croissant"
        CASE m_noun_key = 31  m_noun = "Agua"
        CASE m_noun_key = 32  m_noun = "Zumo"
        CASE m_noun_key = 33  m_noun = "Te"
        CASE m_noun_key = 34  m_noun = "Cafe"
        CASE m_noun_key = 35  m_noun = "Infusion"
        CASE m_noun_key = 41  m_noun = "Patatas"
        CASE m_noun_key = 42  m_noun = "Nuggets"
        CASE m_noun_key = 43  m_noun = "Palitos"
        CASE m_noun_key = 44  m_noun = "Galletas"
        CASE m_noun_key = 45  m_noun = "Barrita"
        CASE m_noun_key = 51  m_noun = "Manzanas"
        CASE m_noun_key = 52  m_noun = "Platanos"
        CASE m_noun_key = 53  m_noun = "Naranjas"
        CASE m_noun_key = 54  m_noun = "Tomates"
        CASE m_noun_key = 55  m_noun = "Zanahorias"
        CASE m_noun_key = 61  m_noun = "Pollo"
        CASE m_noun_key = 62  m_noun = "Ternera"
        CASE m_noun_key = 63  m_noun = "Cerdo"
        CASE m_noun_key = 64  m_noun = "Salchicha"
        CASE m_noun_key = 65  m_noun = "Hamburguesa"
        CASE m_noun_key = 71  m_noun = "Salmon"
        CASE m_noun_key = 72  m_noun = "Atun"
        CASE m_noun_key = 73  m_noun = "Merluza"
        CASE m_noun_key = 74  m_noun = "Gambas"
        CASE m_noun_key = 75  m_noun = "Calamares"
        CASE m_noun_key = 81  m_noun = "Pizza"
        CASE m_noun_key = 82  m_noun = "Helado"
        CASE m_noun_key = 83  m_noun = "Verduras"
        CASE m_noun_key = 84  m_noun = "Frutas"
        CASE m_noun_key = 85  m_noun = "Empanada"
        CASE m_noun_key = 91  m_noun = "Tomate"
        CASE m_noun_key = 92  m_noun = "Atun"
        CASE m_noun_key = 93  m_noun = "Lentejas"
        CASE m_noun_key = 94  m_noun = "Garbanzos"
        CASE m_noun_key = 95  m_noun = "Maiz"
        CASE m_noun_key = 101 m_noun = "Limpieza"
        CASE m_noun_key = 102 m_noun = "Detergente"
        CASE m_noun_key = 103 m_noun = "Jabon"
        CASE m_noun_key = 104 m_noun = "Papel"
        CASE m_noun_key = 105 m_noun = "Esponja"
        CASE m_noun_key = 111 m_noun = "Shampoo"
        CASE m_noun_key = 112 m_noun = "Crema"
        CASE m_noun_key = 113 m_noun = "Protector"
        CASE m_noun_key = 114 m_noun = "Pasta"
        CASE m_noun_key = 115 m_noun = "Desodorante"
        CASE m_noun_key = 121 m_noun = "Pañales"
        CASE m_noun_key = 122 m_noun = "Biberon"
        CASE m_noun_key = 123 m_noun = "Leche"
        CASE m_noun_key = 124 m_noun = "Toallas"
        CASE m_noun_key = 125 m_noun = "Champú"
        CASE m_noun_key = 131 m_noun = "Croquetas"
        CASE m_noun_key = 132 m_noun = "Arena"
        CASE m_noun_key = 133 m_noun = "Snack"
        CASE m_noun_key = 134 m_noun = "Ración"
        CASE m_noun_key = 135 m_noun = "Juguete"
        CASE m_noun_key = 141 m_noun = "Cables"
        CASE m_noun_key = 142 m_noun = "Auriculares"
        CASE m_noun_key = 143 m_noun = "Baterias"
        CASE m_noun_key = 144 m_noun = "Cargador"
        CASE m_noun_key = 145 m_noun = "Memoria"
        CASE m_noun_key = 151 m_noun = "Camiseta"
        CASE m_noun_key = 152 m_noun = "Pantalon"
        CASE m_noun_key = 153 m_noun = "Calcetines"
        CASE m_noun_key = 154 m_noun = "Guantes"
        CASE m_noun_key = 155 m_noun = "Bufanda"
        CASE m_noun_key = 161 m_noun = "Zapatillas"
        CASE m_noun_key = 162 m_noun = "Balón"
        CASE m_noun_key = 163 m_noun = "Mancuernas"
        CASE m_noun_key = 164 m_noun = "Esterilla"
        CASE m_noun_key = 165 m_noun = "Casco"
        CASE m_noun_key = 171 m_noun = "Novela"
        CASE m_noun_key = 172 m_noun = "Cocina"
        CASE m_noun_key = 173 m_noun = "Infantil"
        CASE m_noun_key = 174 m_noun = "Historia"
        CASE m_noun_key = 175 m_noun = "Guia"
        CASE m_noun_key = 181 m_noun = "Puzzle"
        CASE m_noun_key = 182 m_noun = "Muneco"
        CASE m_noun_key = 183 m_noun = "Juego"
        CASE m_noun_key = 184 m_noun = "Construccion"
        CASE m_noun_key = 185 m_noun = "Coche"
        CASE m_noun_key = 191 m_noun = "Planta"
        CASE m_noun_key = 192 m_noun = "Fertilizante"
        CASE m_noun_key = 193 m_noun = "Herramienta"
        CASE m_noun_key = 194 m_noun = "Manguera"
        CASE m_noun_key = 195 m_noun = "Maceta"
        CASE m_noun_key = 201 m_noun = "Aceite"
        CASE m_noun_key = 202 m_noun = "Filtro"
        CASE m_noun_key = 203 m_noun = "Limpia"
        CASE m_noun_key = 204 m_noun = "Antena"
        CASE m_noun_key = 205 m_noun = "Neumático"
        OTHERWISE           m_noun = "Producto"
    ENDCASE

    * Size/weight suffix
    m_size_idx = (m_rnd4 % 8) + 1
    m_name = m_noun
    DO CASE
        CASE m_size_idx = 1
            m_name = m_name + " 1kg"
        CASE m_size_idx = 2
            m_name = m_name + " 500g"
        CASE m_size_idx = 3
            m_name = m_name + " 250g"
        CASE m_size_idx = 4
            m_name = m_name + " 1L"
        CASE m_size_idx = 5
            m_name = m_name + " 2L"
        CASE m_size_idx = 6
            m_name = m_name + " x4"
        CASE m_size_idx = 7
            m_name = m_name + " x6"
        CASE m_size_idx = 8
            m_name = m_name + " Premium"
    ENDCASE

    * Price: 0.10 to 99.99
    m_price = ((m_rnd5 % 999) + 10) / 100.0

    * Cost: 40-70% of price
    m_cost = m_price * ((m_rnd6 % 30) + 40) / 100.0

    * Stock: 0-500
    m_stock = m_rnd7 % 501

    * Min stock: 5-50
    m_min_stock = (m_rnd8 % 46) + 5

    * Provider ID: P001-P020
    m_prov_idx = (m_rnd9 % m_provider_count) + 1
    m_prov_str = ALLTRIM(STR(m_prov_idx))
    m_prov_pad = 3 - LEN(m_prov_str)
    IF m_prov_pad < 0
        m_prov_pad = 0
    ENDIF
    m_prov_id = "P" + REPLICATE("0", m_prov_pad) + m_prov_str

    * Active: mostly true (90%)
    m_active = ((m_rnd10 % 10) < 9)

    * Unit
    m_unit_idx = (m_rnd1 % 20) + 1
    m_unit = ""
    DO CASE
        CASE m_unit_idx <= 3  m_unit = "unit"
        CASE m_unit_idx <= 6  m_unit = "kg"
        CASE m_unit_idx <= 8  m_unit = "L"
        CASE m_unit_idx <= 10 m_unit = "pack"
        CASE m_unit_idx <= 12 m_unit = "box"
        CASE m_unit_idx <= 13 m_unit = "can"
        CASE m_unit_idx <= 15 m_unit = "bottle"
        CASE m_unit_idx <= 16 m_unit = "pair"
        CASE m_unit_idx <= 17 m_unit = "roll"
        CASE m_unit_idx <= 18 m_unit = "tube"
        OTHERWISE            m_unit = "bag"
    ENDCASE

    * Tax rate
    m_tax_idx = (m_rnd2 % 10) + 1
    m_tax_rate = 21.00
    DO CASE
        CASE m_tax_idx <= 2  m_tax_rate = 4.00
        CASE m_tax_idx <= 4  m_tax_rate = 10.00
    ENDCASE

    * Last sold: random date in 2025-2026
    m_sold_day = (m_rnd3 % 28) + 1
    m_sold_mn = (m_rnd4 % 12) + 1
    m_sold_yr = 2025 + (m_rnd5 % 2)
    m_last_sold = CTOD(STR(m_sold_mn) + "/" + STR(m_sold_day) + "/" + STR(m_sold_yr))

    * Reorder date
    m_reorder_day = (m_rnd6 % 28) + 1
    m_reorder_mn = (m_rnd7 % 12) + 1
    m_reorder_yr = 2026 + (m_rnd8 % 2)
    m_reorder_date = CTOD(STR(m_reorder_mn) + "/" + STR(m_reorder_day) + "/" + STR(m_reorder_yr))

    * Description (memo field) - keep short to avoid 64K DBT block issues
    m_desc = m_noun + " - Producto " + m_category + " proveedor " + m_prov_id

    APPEND BLANK
    REPLACE BARCODE WITH m_barcode, NAME WITH m_name, CATEGORY WITH m_category, PRICE WITH m_price, COST WITH m_cost, STOCK WITH m_stock, MIN_STOCK WITH m_min_stock, PROVIDER_ID WITH m_prov_id, ACTIVE WITH m_active, UNIT WITH m_unit, TAX_RATE WITH m_tax_rate, LAST_SOLD WITH m_last_sold, REORDER_DAT WITH m_reorder_date
    REPLACE DESCRIPTION WITH m_desc

    * Progress bar update every 500 records
    IF MOD(m_product_num, 500) = 0
        m_pct = INT(m_product_num * 100 / m_total)
        m_filled = INT(m_product_num * 50 / m_total)
        m_bar_sp = 50 - m_filled
        m_bar = "[" + REPLICATE("#", m_filled) + REPLICATE(" ", m_bar_sp) + "]"
        @ prog_row, 0 SAY "  " + STR(m_product_num, 6) + " / " + STR(m_total, 6) + "  " + STR(m_pct, 3) + "%  " + m_bar
    ENDIF

ENDDO

* Final progress bar (100%)
@ prog_row, 0 SAY "  " + STR(m_total, 6) + " / " + STR(m_total, 6) + "  100%  [" + REPLICATE("#", 50) + "]"

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
