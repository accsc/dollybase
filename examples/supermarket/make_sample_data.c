/*
 * make_sample_data.c — Create empty supermarket DBFs (schema only)
 *
 * Creates 5 DBF files in the current directory with correct schema.
 * Data seeding is done by seed_data.prg using the interpreter.
 *
 * Compile:
 *   gcc -w -o make_sample_data make_sample_data.c ../../libdbase_4/.libs/libdbase_0.4_s.a -lm
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../libdbase_4/libdbase.h"

/* Helper: set a field name (1-indexed, 11 chars max, zero-filled)
   Layout: names[char_pos][field_idx] — char_pos 0-10, field_idx 1+ */
static void set_fname(char names[11][128], int idx, const char *name)
{  
  
    printf("%s - %i\n",name,strlen(name));
    for (int c = 0; c < 11; c++)
    {
	if (c <= strlen(name))
	{
	names[c][idx] = name[c];
	}else{
		names[c][idx] = '\0';
	}

    }


}

static void create_and_seed(const char *path, int camposn,
    void (*setup)(DATABASEDBF *))
{
    DATABASEDBF *db = calloc(1, sizeof(DATABASEDBF));
    db->camposn = camposn;
    setup(db);

    unlink(path);
    char dbt_path[1024];
    snprintf(dbt_path, sizeof(dbt_path), "%s.dbt", path);
    unlink(dbt_path);

    if (create_database(path, 30, 7, 26, db, 0) != 0) {
        fprintf(stderr, "create_database %s failed\n", path);
        free(db); return;
    }
    printf("Created %s\n", path);
    free(db);
}

/* PRODUCTS.DBF */
static void setup_products(DATABASEDBF *db)
{
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
}

/* PROVIDERS.DBF */
static void setup_providers(DATABASEDBF *db)
{
    set_fname(db->fields.names, 1, "ID");
    db->fields.tipos[1] = 'C'; db->fields.longitudes[1] = 4;

    set_fname(db->fields.names, 2, "NAME");
    db->fields.tipos[2] = 'C'; db->fields.longitudes[2] = 40;

    set_fname(db->fields.names, 3, "PHONE");
    db->fields.tipos[3] = 'C'; db->fields.longitudes[3] = 15;

    set_fname(db->fields.names, 4, "ADDRESS");
    db->fields.tipos[4] = 'M'; db->fields.longitudes[4] = 10;
}

/* SALES.DBF */
static void setup_sales(DATABASEDBF *db)
{
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
}

/* SALESITEMS.DBF */
static void setup_salesitems(DATABASEDBF *db)
{
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
}

/* ORDERS.DBF */
static void setup_orders(DATABASEDBF *db)
{
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
}

int main(void)
{
    printf("=== Supermarket DBF Schema Creator ===\n\n");

    create_and_seed("products.dbf", 14, setup_products);
    create_and_seed("providers.dbf", 4, setup_providers);
    create_and_seed("sales.dbf", 5, setup_sales);
    create_and_seed("salesitems.dbf", 5, setup_salesitems);
    create_and_seed("orders.dbf", 5, setup_orders);

    printf("\n=== All schemas created ===\n");
    printf("Run seed_data.prg to populate sample data.\n");
    return 0;
}
