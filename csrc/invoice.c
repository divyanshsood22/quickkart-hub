#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "invoice.h"

#define ORDER_FILE "orders.txt"
#define PRODUCT_FILE "products.txt"
#define USER_FILE "users.txt"

char* istrdup(const char* s) {
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

char* get_product_name(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return istrdup("Unknown");

    char line[256], name[100];
    int id, stock;
    float price;

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d|%[^|]|%f|%d", &id, name, &price, &stock);
        if (id == product_id) {
            fclose(fp);
            return istrdup(name);
        }
    }

    fclose(fp);
    return istrdup("Unknown");
}

char* get_user_name(int user_id) {
    FILE* fp = fopen(USER_FILE, "r");
    if (!fp) return istrdup("Unknown User");

    char line[256], name[100], email[100];
    int id;

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d|%[^|]|%s", &id, name, email);
        if (id == user_id) {
            fclose(fp);
            return istrdup(name);
        }
    }

    fclose(fp);
    return istrdup("Unknown User");
}

char* generate_invoice(int order_id) {
    FILE* fp = fopen(ORDER_FILE, "r");
    if (!fp) return istrdup("Invoice not found.");

    char line[256];
    int oid, uid, pid, qty;
    float total;

    while (fgets(line, sizeof(line), fp)) {
        sscanf(line, "%d|%d|%d|%d|%f", &oid, &uid, &pid, &qty, &total);

        if (oid == order_id) {
            fclose(fp);

            char* pname = get_product_name(pid);
            char* uname = get_user_name(uid);

            char buffer[1000];
            sprintf(buffer,
                "====== QUICKKARTHUB INVOICE ======\n"
                "Order ID      : %d\n"
                "User          : %s (ID %d)\n"
                "Product       : %s (ID %d)\n"
                "Quantity      : %d\n"
                "Total Price   : %.2f\n"
                "===================================\n",
                oid, uname, uid, pname, pid, qty, total
            );

            free(pname);
            free(uname);

            return istrdup(buffer);
        }
    }

    fclose(fp);
    return istrdup("Invoice not found.");
}
