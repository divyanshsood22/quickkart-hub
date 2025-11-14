#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRODUCT_FILE "products.txt"

int update_stock(int product_id, int new_qty) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return -1;

    FILE* temp = fopen("products_tmp.txt", "w");
    if (!temp) {
        fclose(fp);
        return -1;
    }

    char line[256];
    int updated = 0;

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[100];

        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);

        if (id == product_id) {
            stock = new_qty;
            updated = 1;
        }

        fprintf(temp, "%d|%s|%.2f|%d\n", id, name, price, stock);
    }

    fclose(fp);
    fclose(temp);

    remove(PRODUCT_FILE);
    rename("products_tmp.txt", PRODUCT_FILE);

    return updated ? 1 : 0;
}

int get_stock(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return -1;

    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[100];

        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);

        if (id == product_id) {
            fclose(fp);
            return stock;
        }
    }

    fclose(fp);
    return -1;
}
