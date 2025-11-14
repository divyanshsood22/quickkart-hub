#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PRODUCT_FILE "products.txt"
#define ORDER_FILE "orders.txt"

// ORDER_ID|USER_ID|PRODUCT_ID|QUANTITY|TOTAL_PRICE

char* ostrdup(const char* s) {
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

float get_product_price(int product_id) {
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
            return price;
        }
    }

    fclose(fp);
    return -1;
}

int get_product_stock(int product_id) {
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

void set_product_stock(int product_id, int new_stock) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    FILE* temp = fopen("products_tmp.txt", "w");

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[100];

        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);

        if (id == product_id) {
            stock = new_stock;
        }

        fprintf(temp, "%d|%s|%.2f|%d\n", id, name, price, stock);
    }

    fclose(fp);
    fclose(temp);

    remove(PRODUCT_FILE);
    rename("products_tmp.txt", PRODUCT_FILE);
}

int place_order(int user_id, int product_id, int quantity) {
    int stock = get_product_stock(product_id);
    if (stock == -1) return -1;
    if (stock < quantity) return -2;

    float price = get_product_price(product_id);
    if (price < 0) return -3;

    float total = price * quantity;

    set_product_stock(product_id, stock - quantity);

    FILE* fp = fopen(ORDER_FILE, "a");
    if (!fp) return -4;

    int order_id = 1;
    FILE* read = fopen(ORDER_FILE, "r");
    if (read) {
        char line[256];
        while (fgets(line, sizeof(line), read)) order_id++;
        fclose(read);
    }

    fprintf(fp, "%d|%d|%d|%d|%.2f\n",
            order_id, user_id, product_id, quantity, total);

    fclose(fp);

    return order_id;
}

char* get_orders_for_user(int user_id) {
    FILE* fp = fopen(ORDER_FILE, "r");
    if (!fp) return ostrdup("[]");

    char buffer[10000] = "[";
    char line[256];
    int first = 1;

    while (fgets(line, sizeof(line), fp)) {
        int oid, uid, pid, qty;
        float total;

        sscanf(line, "%d|%d|%d|%d|%f", &oid, &uid, &pid, &qty, &total);

        if (uid == user_id) {
            if (!first) strcat(buffer, ",");
            first = 0;

            char item[300];
            sprintf(item,
                "{\"order_id\":%d,\"user_id\":%d,\"product_id\":%d,"
                "\"quantity\":%d,\"total\":%.2f}",
                oid, uid, pid, qty, total);

            strcat(buffer, item);
        }
    }

    fclose(fp);
    strcat(buffer, "]");

    return ostrdup(buffer);
}

char* list_all_orders() {
    FILE* fp = fopen(ORDER_FILE, "r");
    if (!fp) return ostrdup("[]");

    char buffer[10000] = "[";
    char line[256];
    int first = 1;

    while (fgets(line, sizeof(line), fp)) {
        int oid, uid, pid, qty;
        float total;

        sscanf(line, "%d|%d|%d|%d|%f", &oid, &uid, &pid, &qty, &total);

        if (!first) strcat(buffer, ",");
        first = 0;

        char item[300];
        sprintf(item,
            "{\"order_id\":%d,\"user_id\":%d,\"product_id\":%d,"
            "\"quantity\":%d,\"total\":%.2f}",
            oid, uid, pid, qty, total);

        strcat(buffer, item);
    }

    fclose(fp);
    strcat(buffer, "]");

    return ostrdup(buffer);
}
