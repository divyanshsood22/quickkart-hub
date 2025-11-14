#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "products.h"

#define PRODUCT_FILE "products.txt"

// ID|NAME|PRICE|STOCK

char* str_dup(const char* src) {
    char* dst = malloc(strlen(src) + 1);
    strcpy(dst, src);
    return dst;
}

int add_product(const char* name, float price, int stock) {
    FILE* fp = fopen(PRODUCT_FILE, "a");
    if (!fp) return -1;

    int id = 1;
    FILE* check = fopen(PRODUCT_FILE, "r");
    if (check) {
        char line[256];
        while (fgets(line, sizeof(line), check)) id++;
        fclose(check);
    }

    fprintf(fp, "%d|%s|%.2f|%d\n", id, name, price, stock);
    fclose(fp);
    return id;
}

int remove_product(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return -1;

    FILE* temp = fopen("products_tmp.txt", "w");
    char line[256];
    int removed = 0;

    while (fgets(line, sizeof(line), fp)) {
        int id;
        sscanf(line, "%d|", &id);
        if (id != product_id) {
            fputs(line, temp);
        } else {
            removed = 1;
        }
    }

    fclose(fp);
    fclose(temp);
    remove(PRODUCT_FILE);
    rename("products_tmp.txt", PRODUCT_FILE);

    return removed ? 1 : 0;
}

char* list_products() {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return str_dup("[]");

    char buffer[10000] = "[";
    char line[256];
    int first = 1;

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[100];

        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);

        if (!first) strcat(buffer, ",");
        first = 0;

        char item[300];
        sprintf(item, "{\"id\":%d,\"name\":\"%s\",\"price\":%.2f,\"stock\":%d}",
                id, name, price, stock);

        strcat(buffer, item);
    }

    fclose(fp);
    strcat(buffer, "]");

    return str_dup(buffer);
}

char* get_product(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return NULL;

    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[100];

        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);

        if (id == product_id) {
            char json[300];
            sprintf(json, "{\"id\":%d,\"name\":\"%s\",\"price\":%.2f,\"stock\":%d}",
                    id, name, price, stock);
            fclose(fp);
            return str_dup(json);
        }
    }

    fclose(fp);
    return NULL;
}
