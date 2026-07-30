/*
 * make_sample_data.c — Create supermarket DBFs and seed sample data
 *
 * Creates 5 DBF files in data/ directory with realistic supermarket data.
 *
 * Compile:
 *   gcc -w -o make_sample_data make_sample_data.c -L../../libdbase_4 -ldbase -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../libdbase_4/libdbase.h"

/* Helper: set a field name (1-indexed, 12 chars max, zero-filled)
   Layout: names[char_pos][field_idx] — char_pos 0-11, field_idx 1+ */
static void set_fname(char names[12][256], int idx, const char *name)
{
    for (int c = 0; c < 12; c++)
        names[c][idx] = (c < (int)strlen(name)) ? name[c] : '\0';
}

/* Helper: pad string to exact length */
static void pad(char *buf, int len, const char *val)
{
    memset(buf, ' ', len);
    if (val) strncpy(buf, val, len - 1);
    buf[len - 1] = '\0';
}

/* Helper: format number as right-justified string with decimals */
static void fmt_num(char *buf, int len, double val, int dec)
{
    char tmp[64];
    if (dec > 0)
        snprintf(tmp, sizeof(tmp), "%*.*f", len, dec, val);
    else
        snprintf(tmp, sizeof(tmp), "%*d", len, (int)val);
    pad(buf, len, tmp);
}

/* Helper: format logical field */
static void fmt_logic(char *buf, int val)
{
    buf[0] = val ? 'T' : 'F';
    buf[1] = '\0';
}

/* Helper: format date as YYYYMMDD */
static void fmt_date(char *buf, int y, int m, int d)
{
    snprintf(buf, 9, "%04d%02d%02d", y, m, d);
}

/* ============================================================ */
/* PRODUCTS.DBF                                                 */
/* BARCODE(C,13) NAME(C,40) CATEGORY(C,20) PRICE(N,8,2)        */
/* COST(N,8,2) STOCK(N,5) MIN_STOCK(N,3) PROVIDER_ID(C,4)      */
/* ACTIVE(L,1) UNIT(C,10) TAX_RATE(N,3,2) LAST_SOLD(D,8)       */
/* REORDER_DATE(D,8) DESCRIPTION(M,10)                          */
/* ============================================================ */

static void create_products(void)
{
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    db->camposn = 14;

    set_fname(db->fields.names, 1, "BARCODE");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 13;

    set_fname(db->fields.names, 2, "NAME");
    db->fields.tipos[2] = 'C'; db->fields.longitudes[2] = 40;

    set_fname(db->fields.names, 3, "CATEGORY");
    db->fields.tipos[3] = 'C'; db->fields.longitudes[3] = 20;

    set_fname(db->fields.names, 4, "PRICE");
    db->fields.tipos[4] = 'N'; db->fields.longitudes[4] = 8; db->fields.decimales[4] = 2;

    set_fname(db->fields.names, 5, "COST");
    db->fields.tipos[5] = 'N'; db->fields.longitudes[5] = 8; db->fields.decimales[5] = 2;

    set_fname(db->fields.names, 6, "STOCK");
    db->fields.tipos[6] = 'N'; db->fields.longitudes[6] = 5;

    set_fname(db->fields.names, 7, "MIN_STOCK");
    db->fields.tipos[7] = 'N'; db->fields.longitudes[7] = 3;

    set_fname(db->fields.names, 8, "PROVIDER_ID");
    db->fields.tipos[8] = 'C'; db->fields.longitudes[8] = 4;

    set_fname(db->fields.names, 9, "ACTIVE");
    db->fields.tipos[9] = 'L'; db->fields.longitudes[9] = 1;

    set_fname(db->fields.names, 10, "UNIT");
    db->fields.tipos[10] = 'C'; db->fields.longitudes[10] = 10;

    set_fname(db->fields.names, 11, "TAX_RATE");
    db->fields.tipos[11] = 'N'; db->fields.longitudes[11] = 3; db->fields.decimales[11] = 2;

    set_fname(db->fields.names, 12, "LAST_SOLD");
    db->fields.tipos[12] = 'D'; db->fields.longitudes[12] = 8;

    set_fname(db->fields.names, 13, "REORDER_DATE");
    db->fields.tipos[13] = 'D'; db->fields.longitudes[13] = 8;

    set_fname(db->fields.names, 14, "DESCRIPTION");
    db->fields.tipos[14] = 'M'; db->fields.longitudes[14] = 10;

    unlink("data/products.dbf");
    unlink("data/products.dbt");

    if (create_database("data/products.dbf", 30, 7, 26, db, 0) != 0) {
        fprintf(stderr, "create_database products.dbf failed\n");
        free(db); return;
    }
    printf("Created data/products.dbf\n");
    free(db);

    /* Open and seed data */
    DATABASEDBF *tgt = calloc(1, sizeof(DATABASEDBF));
    use("data/products.dbf", &tgt);
    if (!tgt || tgt->tipo == 0) {
        fprintf(stderr, "Failed to open products.dbf\n"); free(tgt); return;
    }

    /* Product seed data: barcode, name, category, price, cost, stock, min_stock, provider_id, unit, tax_rate, desc */
    struct {
        char barcode[14];
        char name[41];
        char category[21];
        double price;
        double cost;
        int stock;
        int min_stock;
        char provider_id[5];
        char unit[11];
        double tax_rate;
        char desc[256];
    } products[] = {
        {"8412345670001", "Leche Entera 1L",       "Dairy",      1.20, 0.70,  50, 20, "P001", "unit",     4.00, "Fresh whole milk 1 liter bottle"},
        {"8412345670002", "Yogurt Natural 125g",    "Dairy",      0.85, 0.45,  30, 15, "P001", "unit",     4.00, "Natural yogurt single portion"},
        {"8412345670003", "Queso Manchego 200g",    "Dairy",      3.50, 2.10,   5, 10, "P001", "unit",     10.00, "Manchego cheese wedge 200g"},
        {"8412345670004", "Pan de Molde Integral",  "Bakery",     1.80, 1.00,  25, 10, "P002", "unit",     4.00, "Whole wheat sandwich bread loaf"},
        {"8412345670005", "Croissant Mantequilla",  "Bakery",     1.20, 0.60,   3, 10, "P002", "unit",     4.00, "Butter croissant fresh baked"},
        {"8412345670006", "Agua Mineral 1.5L",      "Beverages",  0.60, 0.20, 100, 30, "P003", "unit",     4.00, "Still mineral water 1.5L bottle"},
        {"8412345670007", "Refresco Cola 33cl",     "Beverages",  1.50, 0.70,  60, 20, "P003", "unit",     4.00, "Cola soft drink 330ml can"},
        {"8412345670008", "Zumo Naranja 1L",        "Beverages",  2.20, 1.30,   2, 10, "P003", "unit",     4.00, "Fresh orange juice 1L carton"},
        {"8412345670009", "Patatas Fritas 150g",    "Snacks",     1.90, 0.90,  40, 15, "P004", "unit",     4.00, "Crispy potato chips 150g bag"},
        {"8412345670010", "Galletas Chocolate",     "Snacks",     1.60, 0.80,   8, 10, "P004", "unit",     4.00, "Chocolate chip cookies pack"},
        {"8412345670011", "Manzanas Rojas kg",      "Produce",    2.50, 1.50,  20, 10, "P002", "kg",       4.00, "Red apples per kilogram"},
        {"8412345670012", "Platanos kg",            "Produce",    1.80, 1.00,   4, 10, "P002", "kg",       4.00, "Bananas per kilogram"},
    };

    int nproducts = sizeof(products) / sizeof(products[0]);

    for (int i = 0; i < nproducts; i++) {
        append_blank(&tgt);

        char buf[64];
        pad(buf, 13, products[i].barcode);
        replace2(tgt, "BARCODE", buf);

        pad(buf, 40, products[i].name);
        replace2(tgt, "NAME", buf);

        pad(buf, 20, products[i].category);
        replace2(tgt, "CATEGORY", buf);

        fmt_num(buf, 8, products[i].price, 2);
        replace2(tgt, "PRICE", buf);

        fmt_num(buf, 8, products[i].cost, 2);
        replace2(tgt, "COST", buf);

        fmt_num(buf, 5, products[i].stock, 0);
        replace2(tgt, "STOCK", buf);

        fmt_num(buf, 3, products[i].min_stock, 0);
        replace2(tgt, "MIN_STOCK", buf);

        pad(buf, 4, products[i].provider_id);
        replace2(tgt, "PROVIDER_ID", buf);

        fmt_logic(buf, 1);
        replace2(tgt, "ACTIVE", buf);

        pad(buf, 10, products[i].unit);
        replace2(tgt, "UNIT", buf);

        fmt_num(buf, 3, products[i].tax_rate, 2);
        replace2(tgt, "TAX_RATE", buf);

        /* LAST_SOLD: empty for now */
        pad(buf, 8, "");
        replace2(tgt, "LAST_SOLD", buf);

        /* REORDER_DATE: empty for now */
        pad(buf, 8, "");
        replace2(tgt, "REORDER_DATE", buf);

        /* DESCRIPTION: memo field */
        add_to_dbt("data/products.dbt", products[i].desc, strlen(products[i].desc));
        char block_str[16];
        snprintf(block_str, sizeof(block_str), "%d", i + 1);
        replace2(tgt, "DESCRIPTION", block_str);
    }

    printf("Seeded %d products\n", nproducts);
    free(tgt);
}

/* ============================================================ */
/* PROVIDERS.DBF                                                */
/* ID(C,4) NAME(C,40) PHONE(C,15) ADDRESS(M,10)                 */
/* ============================================================ */

static void create_providers(void)
{
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    db->camposn = 4;

    set_fname(db->fields.names, 1, "ID");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 4;

    set_fname(db->fields.names, 2, "NAME");
    db->fields.tipos[2] = 'C'; db->fields.longitudes[2] = 40;

    set_fname(db->fields.names, 3, "PHONE");
    db->fields.tipos[3] = 'C'; db->fields.longitudes[3] = 15;

    set_fname(db->fields.names, 4, "ADDRESS");
    db->fields.tipos[4] = 'M'; db->fields.longitudes[4] = 10;

    unlink("data/providers.dbf");
    unlink("data/providers.dbt");

    if (create_database("data/providers.dbf", 30, 7, 26, db, 0) != 0) {
        fprintf(stderr, "create_database providers.dbf failed\n");
        free(db); return;
    }
    printf("Created data/providers.dbf\n");
    free(db);

    DATABASEDBF *tgt = calloc(1, sizeof(DATABASEDBF));
    use("data/providers.dbf", &tgt);
    if (!tgt || tgt->tipo == 0) {
        fprintf(stderr, "Failed to open providers.dbf\n"); free(tgt); return;
    }

    struct {
        char id[5];
        char name[41];
        char phone[16];
        char address[256];
    } providers[] = {
        {"P001", "Lacteos del Norte SA",       "+34 912 345 678", "Calle Lactaria 15, 28001 Madrid, Spain. Main dairy distribution center serving central Spain."},
        {"P002", "Panaderia Central SL",        "+34 913 456 789", "Avenida del Pan 42, 28012 Madrid, Spain. Fresh bakery products and produce supplier."},
        {"P003", "Bebidas Ibericas SA",         "+34 914 567 890", "Poligono Industrial Sur, Nave 7, 28821 Coslada, Madrid, Spain. Beverage distribution."},
        {"P004", "Snacks y Conservas Iberica",  "+34 915 678 901", "Calle Industrial 8, 28022 Madrid, Spain. Snacks, canned goods, and packaged foods."},
    };

    int nprov = sizeof(providers) / sizeof(providers[0]);
    char buf[64];

    for (int i = 0; i < nprov; i++) {
        append_blank(&tgt);

        pad(buf, 4, providers[i].id);
        replace2(tgt, "ID", buf);

        pad(buf, 40, providers[i].name);
        replace2(tgt, "NAME", buf);

        pad(buf, 15, providers[i].phone);
        replace2(tgt, "PHONE", buf);

        add_to_dbt("data/providers.dbt", providers[i].address, strlen(providers[i].address));
        char block_str[16];
        snprintf(block_str, sizeof(block_str), "%d", i + 1);
        replace2(tgt, "ADDRESS", block_str);
    }

    printf("Seeded %d providers\n", nprov);
    free(tgt);
}

/* ============================================================ */
/* SALES.DBF                                                    */
/* SALE_ID(C,10) SALE_DATE(D,8) TOTAL(N,10,2) ITEMS_COUNT(N,4) */
/* PAYMENT(C,10)                                                 */
/* ============================================================ */

static void create_sales(void)
{
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    db->camposn = 5;

    set_fname(db->fields.names, 1, "SALE_ID");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 10;

    set_fname(db->fields.names, 2, "SALE_DATE");
    db->fields.tipos[2] = 'D'; db->fields.longitudes[2] = 8;

    set_fname(db->fields.names, 3, "TOTAL");
    db->fields.tipos[3] = 'N'; db->fields.longitudes[3] = 10; db->fields.decimales[3] = 2;

    set_fname(db->fields.names, 4, "ITEMS_COUNT");
    db->fields.tipos[4] = 'N'; db->fields.longitudes[4] = 4;

    set_fname(db->fields.names, 5, "PAYMENT");
    db->fields.tipos[5] = 'C'; db->fields.longitudes[5] = 10;

    unlink("data/sales.dbf");

    if (create_database("data/sales.dbf", 30, 7, 26, db, 0) != 0) {
        fprintf(stderr, "create_database sales.dbf failed\n");
        free(db); return;
    }
    printf("Created data/sales.dbf\n");
    free(db);

    DATABASEDBF *tgt = calloc(1, sizeof(DATABASEDBF));
    use("data/sales.dbf", &tgt);
    if (!tgt || tgt->tipo == 0) {
        fprintf(stderr, "Failed to open sales.dbf\n"); free(tgt); return;
    }

    struct {
        char sale_id[11];
        char sale_date[9];
        double total;
        int items_count;
        char payment[11];
    } sales[] = {
        {"20260728001", "20260728", 12.50, 5, "CASH"},
        {"20260728002", "20260728", 28.90, 8, "CARD"},
        {"20260729001", "20260729", 5.40,  3, "CASH"},
    };

    int nsales = sizeof(sales) / sizeof(sales[0]);
    char buf[64];

    for (int i = 0; i < nsales; i++) {
        append_blank(&tgt);

        pad(buf, 10, sales[i].sale_id);
        replace2(tgt, "SALE_ID", buf);

        pad(buf, 8, sales[i].sale_date);
        replace2(tgt, "SALE_DATE", buf);

        fmt_num(buf, 10, sales[i].total, 2);
        replace2(tgt, "TOTAL", buf);

        fmt_num(buf, 4, sales[i].items_count, 0);
        replace2(tgt, "ITEMS_COUNT", buf);

        pad(buf, 10, sales[i].payment);
        replace2(tgt, "PAYMENT", buf);
    }

    printf("Seeded %d sales\n", nsales);
    free(tgt);
}

/* ============================================================ */
/* SALESITEMS.DBF                                               */
/* SALE_ID(C,10) BARCODE(C,13) QTY(N,4) UNIT_PRICE(N,8,2)      */
/* LINE_TOTAL(N,10,2)                                            */
/* ============================================================ */

static void create_salesitems(void)
{
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    db->camposn = 5;

    set_fname(db->fields.names, 1, "SALE_ID");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 10;

    set_fname(db->fields.names, 2, "BARCODE");
    db->fields.tipos[2] = 'C'; db->fields.longitudes[2] = 13;

    set_fname(db->fields.names, 3, "QTY");
    db->fields.tipos[3] = 'N'; db->fields.longitudes[3] = 4;

    set_fname(db->fields.names, 4, "UNIT_PRICE");
    db->fields.tipos[4] = 'N'; db->fields.longitudes[4] = 8; db->fields.decimales[4] = 2;

    set_fname(db->fields.names, 5, "LINE_TOTAL");
    db->fields.tipos[5] = 'N'; db->fields.longitudes[5] = 10; db->fields.decimales[5] = 2;

    unlink("data/salesitems.dbf");

    if (create_database("data/salesitems.dbf", 30, 7, 26, db, 0) != 0) {
        fprintf(stderr, "create_database salesitems.dbf failed\n");
        free(db); return;
    }
    printf("Created data/salesitems.dbf\n");
    free(db);

    DATABASEDBF *tgt = calloc(1, sizeof(DATABASEDBF));
    use("data/salesitems.dbf", &tgt);
    if (!tgt || tgt->tipo == 0) {
        fprintf(stderr, "Failed to open salesitems.dbf\n"); free(tgt); return;
    }

    struct {
        char sale_id[11];
        char barcode[14];
        int qty;
        double unit_price;
        double line_total;
    } items[] = {
        /* Sale 1: 5 items, total 12.50 */
        {"20260728001", "8412345670001", 2, 1.20, 2.40},
        {"20260728001", "8412345670002", 1, 0.85, 0.85},
        {"20260728001", "8412345670006", 2, 0.60, 1.20},
        {"20260728001", "8412345670009", 1, 1.90, 1.90},
        {"20260728001", "8412345670011", 1, 2.50, 2.50},
        /* Sale 2: 8 items, total 28.90 */
        {"20260728002", "8412345670003", 1, 3.50, 3.50},
        {"20260728002", "8412345670004", 2, 1.80, 3.60},
        {"20260728002", "8412345670005", 3, 1.20, 3.60},
        {"20260728002", "8412345670007", 2, 1.50, 3.00},
        {"20260728002", "8412345670008", 1, 2.20, 2.20},
        {"20260728002", "8412345670010", 2, 1.60, 3.20},
        {"20260728002", "8412345670011", 1, 2.50, 2.50},
        {"20260728002", "8412345670012", 1, 1.80, 1.80},
        /* Sale 3: 3 items, total 5.40 */
        {"20260729001", "8412345670001", 1, 1.20, 1.20},
        {"20260729001", "8412345670006", 2, 0.60, 1.20},
        {"20260729001", "8412345670010", 2, 1.60, 3.20},
    };

    int nitems = sizeof(items) / sizeof(items[0]);
    char buf[64];

    for (int i = 0; i < nitems; i++) {
        append_blank(&tgt);

        pad(buf, 10, items[i].sale_id);
        replace2(tgt, "SALE_ID", buf);

        pad(buf, 13, items[i].barcode);
        replace2(tgt, "BARCODE", buf);

        fmt_num(buf, 4, items[i].qty, 0);
        replace2(tgt, "QTY", buf);

        fmt_num(buf, 8, items[i].unit_price, 2);
        replace2(tgt, "UNIT_PRICE", buf);

        fmt_num(buf, 10, items[i].line_total, 2);
        replace2(tgt, "LINE_TOTAL", buf);
    }

    printf("Seeded %d sales items\n", nitems);
    free(tgt);
}

/* ============================================================ */
/* ORDERS.DBF                                                   */
/* ORDER_ID(C,10) PROVIDER_ID(C,4) ORDER_DATE(D,8) STATUS(C,1) */
/* NOTES(M,10)                                                   */
/* ============================================================ */

static void create_orders(void)
{
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    db->camposn = 5;

    set_fname(db->fields.names, 1, "ORDER_ID");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 10;

    set_fname(db->fields.names, 2, "PROVIDER_ID");
    db->fields.tipos[2] = 'C'; db->fields.longitudes[2] = 4;

    set_fname(db->fields.names, 3, "ORDER_DATE");
    db->fields.tipos[3] = 'D'; db->fields.longitudes[3] = 8;

    set_fname(db->fields.names, 4, "STATUS");
    db->fields.tipos[4] = 'C'; db->fields.longitudes[4] = 1;

    set_fname(db->fields.names, 5, "NOTES");
    db->fields.tipos[5] = 'M'; db->fields.longitudes[5] = 10;

    unlink("data/orders.dbf");
    unlink("data/orders.dbt");

    if (create_database("data/orders.dbf", 30, 7, 26, db, 0) != 0) {
        fprintf(stderr, "create_database orders.dbf failed\n");
        free(db); return;
    }
    printf("Created data/orders.dbf\n");
    free(db);

    DATABASEDBF *tgt = calloc(1, sizeof(DATABASEDBF));
    use("data/orders.dbf", &tgt);
    if (!tgt || tgt->tipo == 0) {
        fprintf(stderr, "Failed to open orders.dbf\n"); free(tgt); return;
    }

    struct {
        char order_id[11];
        char provider_id[5];
        char order_date[9];
        char status[2];
        char notes[256];
    } orders[] = {
        {"20260725001", "P001", "20260725", "R", "Queso Manchego 200g (15) - Restocked"},
        {"20260726001", "P002", "20260726", "R", "Croissant Mantequilla (20), Manzanas Rojas kg (15) - Restocked"},
    };

    int norders = sizeof(orders) / sizeof(orders[0]);
    char buf[64];

    for (int i = 0; i < norders; i++) {
        append_blank(&tgt);

        pad(buf, 10, orders[i].order_id);
        replace2(tgt, "ORDER_ID", buf);

        pad(buf, 4, orders[i].provider_id);
        replace2(tgt, "PROVIDER_ID", buf);

        pad(buf, 8, orders[i].order_date);
        replace2(tgt, "ORDER_DATE", buf);

        pad(buf, 1, orders[i].status);
        replace2(tgt, "STATUS", buf);

        add_to_dbt("data/orders.dbt", orders[i].notes, strlen(orders[i].notes));
        char block_str[16];
        snprintf(block_str, sizeof(block_str), "%d", i + 1);
        replace2(tgt, "NOTES", block_str);
    }

    printf("Seeded %d orders\n", norders);
    free(tgt);
}

/* ============================================================ */

int main(void)
{
    printf("=== Supermarket Sample Data Generator ===\n\n");

    create_products();
    create_providers();
    create_sales();
    create_salesitems();
    create_orders();

    printf("\n=== All databases created in data/ ===\n");
    return 0;
}
