/* inventory.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "inventory.h"
#include "export.h"

#define PRODUCT_FILE "D:/quickkart-hub/web/quickkart/products.txt"

/*
    FILE FORMAT:
    ID|NAME|PRICE|STOCK|CATEGORY
*/

EXPORT int get_stock(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return -1;

    char line[512];

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[120], category[64];

        if (sscanf(line, "%d|%119[^|]|%f|%d|%63[^\n]",
                   &id, name, &price, &stock, category) < 4)
            continue;

        if (id == product_id) {
            fclose(fp);
            return stock;
        }
    }

    fclose(fp);
    return -1;
}

/*
    UPDATE STOCK FUNCTION
*/
EXPORT int update_stock_value(int product_id, int new_stock) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return 0;

    FILE* temp = fopen("products.tmp", "w");
    if (!temp) {
        fclose(fp);
        return 0;
    }

    char line[512];
    int updated = 0;

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[120], category[64];

        if (sscanf(line, "%d|%119[^|]|%f|%d|%63[^\n]",
                   &id, name, &price, &stock, category) < 4)
            continue;

        if (id == product_id) {
            stock = new_stock;
            updated = 1;
        }

        fprintf(temp, "%d|%s|%.2f|%d|%s\n",
                id, name, price, stock, category);
    }

    fclose(fp);
    fclose(temp);

    remove(PRODUCT_FILE);
    rename("products.tmp", PRODUCT_FILE);

    return updated;
}
