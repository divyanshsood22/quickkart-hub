#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "recommendation.h"

#define PRODUCT_FILE "products.txt"

char* rstrdup(const char* s) {
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

char* get_recommendations(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return rstrdup("[]");

    char target_name[100] = "";
    int target_found = 0;
    char line[256];

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[100];

        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);

        if (id == product_id) {
            strcpy(target_name, name);
            target_found = 1;
            break;
        }
    }

    if (!target_found) {
        fclose(fp);
        return rstrdup("[]");
    }

    rewind(fp);

    char buffer[5000] = "[";
    int first = 1;
    char t0 = target_name[0];

    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[100];

        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);

        if (id == product_id) continue;

        if (name[0] == t0) {
            if (!first) strcat(buffer, ",");
            first = 0;

            char item[300];
            sprintf(item, "{\"id\":%d,\"name\":\"%s\"}", id, name);
            strcat(buffer, item);
        }
    }

    fclose(fp);
    strcat(buffer, "]");

    return rstrdup(buffer);
}
