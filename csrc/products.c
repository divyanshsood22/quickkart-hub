#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

#include "products.h"
#include "export.h"

#define PRODUCT_FILE "D:/quickkart-hub/web/quickkart/products.txt"
#define PRODUCT_FILE_TMP "D:/quickkart-hub/web/quickkart/products_tmp.txt"

static char *safe_strdup(const char *s) {
    if (!s) return NULL;
    size_t n = strlen(s) + 1;
    char *d = (char*)malloc(n);
    if (!d) return NULL;
    memcpy(d, s, n);
    return d;
}

/* generate random 4-digit id in [1000, 9999] and ensure uniqueness */
static int generate_unique_id(void) {
    /* seed once using time + address noise */
    static int seeded = 0;
    if (!seeded) {
        unsigned int seed = (unsigned int)time(NULL);
        seed ^= (unsigned int)(uintptr_t)&seed;
        seed ^= (unsigned int)clock();
        srand(seed);
        seeded = 1;
    }

    int tries = 0;
    while (tries < 2000) {
        int cand = 1000 + (rand() % 9000); /* 1000..9999 */
        /* verify uniqueness */
        FILE *fp = fopen(PRODUCT_FILE, "r");
        int found = 0;
        if (fp) {
            char line[512];
            while (fgets(line, sizeof(line), fp)) {
                int id;
                if (sscanf(line, "%d|", &id) == 1) {
                    if (id == cand) { found = 1; break; }
                }
            }
            fclose(fp);
        }
        if (!found) return cand;
        tries++;
    }

    /* fallback: pick max existing id + 1 */
    FILE *fp = fopen(PRODUCT_FILE, "r");
    int maxid = 999;
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            int id;
            if (sscanf(line, "%d|", &id) == 1) {
                if (id > maxid) maxid = id;
            }
        }
        fclose(fp);
    }
    return maxid + 1;
}

/* ------------------ Exposed functions ------------------ */

EXPORT int add_product(const char* name, float price, int stock, const char* category) {
    if (!name || !category) return -1;

    int id = generate_unique_id();
    FILE *fp = fopen(PRODUCT_FILE, "a");
    if (!fp) return -1;

    /* Ensure newline-terminated line */
    fprintf(fp, "%d|%s|%.2f|%d|%s\n", id, name, price, stock, category);
    fclose(fp);
    return id;
}

EXPORT int remove_product(int product_id) {
    FILE *fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return 0;
    FILE *tmp = fopen(PRODUCT_FILE_TMP, "w");
    if (!tmp) { fclose(fp); return 0; }

    char line[512];
    int removed = 0;
    while (fgets(line, sizeof(line), fp)) {
        int id;
        if (sscanf(line, "%d|", &id) == 1 && id == product_id) {
            removed = 1;
            continue; /* skip writing this line */
        }
        fputs(line, tmp);
    }

    fclose(fp);
    fclose(tmp);

    /* replace file atomically */
    remove(PRODUCT_FILE);
    rename(PRODUCT_FILE_TMP, PRODUCT_FILE);
    return removed;
}

EXPORT char* list_products() {
    FILE *fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return safe_strdup("[]");

    size_t cap = 32768;
    char *buf = (char*)malloc(cap);
    if (!buf) { fclose(fp); return safe_strdup("[]"); }
    buf[0] = '['; buf[1] = '\0';

    int first = 1;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[128] = {0}, category[128] = {0};
        int scanned = sscanf(line, "%d|%127[^|]|%f|%d|%127[^\n]", &id, name, &price, &stock, category);
        if (scanned < 4) continue;
        char item[1024];
        snprintf(item, sizeof(item),
                 "%s{\"id\":%d,\"name\":\"%s\",\"price\":%.2f,\"stock\":%d,\"category\":\"%s\"}",
                 first ? "" : ",", id, name, price, stock, category);
        first = 0;
        size_t need = strlen(buf) + strlen(item) + 16;
        if (need > cap) {
            while (need > cap) cap *= 2;
            char *tmp = (char*)realloc(buf, cap);
            if (!tmp) { free(buf); fclose(fp); return safe_strdup("[]"); }
            buf = tmp;
        }
        strcat(buf, item);
    }

    fclose(fp);
    strcat(buf, "]");
    return buf; /* caller must free */
}

EXPORT char* get_product(int product_id) {
    FILE *fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return NULL;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[128] = {0}, category[128] = {0};
        int scanned = sscanf(line, "%d|%127[^|]|%f|%d|%127[^\n]", &id, name, &price, &stock, category);
        if (scanned < 4) continue;
        if (id == product_id) {
            char json[1024];
            snprintf(json, sizeof(json),
                     "{\"id\":%d,\"name\":\"%s\",\"price\":%.2f,\"stock\":%d,\"category\":\"%s\"}",
                     id, name, price, stock, category);
            fclose(fp);
            return safe_strdup(json);
        }
    }

    fclose(fp);
    return NULL;
}

/* returns Product struct by value */
EXPORT Product get_product_struct(int product_id) {
    Product p;
    p.id = -1;
    p.name[0] = '\0';
    p.category[0] = '\0';
    p.price = 0.0f;
    p.stock = 0;

    FILE *fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return p;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id;
        if (sscanf(line, "%d|%99[^|]|%f|%d|%49[^\n]",
                   &id, p.name, &p.price, &p.stock, p.category) < 4) {
            continue;
        }
        if (id == product_id) {
            p.id = id;
            break;
        }
    }
    fclose(fp);
    return p;
}

/* fills out_list with malloc'd array of Product; returns count (caller must free) */
EXPORT int get_products_by_category(const char* category, Product** out_list) {
    if (!category || !out_list) return 0;
    FILE *fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return 0;

    Product *arr = (Product*)malloc(sizeof(Product) * 512);
    if (!arr) { fclose(fp); return 0; }

    int count = 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        Product p;
        p.id = -1;
        if (sscanf(line, "%d|%99[^|]|%f|%d|%49[^\n]",
                   &p.id, p.name, &p.price, &p.stock, p.category) < 4) {
            continue;
        }
        if (strcmp(p.category, category) == 0) {
            if (count < 512) arr[count++] = p;
        }
    }

    fclose(fp);
    *out_list = arr;
    return count;
}

EXPORT int update_product_stock(int product_id, int new_stock) {
    FILE *fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return 0;
    FILE *tmp = fopen(PRODUCT_FILE_TMP, "w");
    if (!tmp) { fclose(fp); return 0; }

    char line[512];
    int updated = 0;
    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[128] = {0}, category[128] = {0};
        int scanned = sscanf(line, "%d|%127[^|]|%f|%d|%127[^\n]",
                             &id, name, &price, &stock, category);
        if (scanned < 4) continue;
        if (id == product_id) {
            stock = new_stock;
            updated = 1;
        }
        fprintf(tmp, "%d|%s|%.2f|%d|%s\n", id, name, price, stock, category);
    }

    fclose(fp);
    fclose(tmp);

    remove(PRODUCT_FILE);
    rename(PRODUCT_FILE_TMP, PRODUCT_FILE);
    return updated;
}

EXPORT int update_price(int product_id, float new_price) {
    FILE *fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return 0;
    FILE *tmp = fopen(PRODUCT_FILE_TMP, "w");
    if (!tmp) { fclose(fp); return 0; }

    char line[512];
    int updated = 0;
    while (fgets(line, sizeof(line), fp)) {
        int id, stock;
        float price;
        char name[128] = {0}, category[128] = {0};
        int scanned = sscanf(line, "%d|%127[^|]|%f|%d|%127[^\n]",
                             &id, name, &price, &stock, category);
        if (scanned < 4) continue;
        if (id == product_id) {
            price = new_price;
            updated = 1;
        }
        fprintf(tmp, "%d|%s|%.2f|%d|%s\n", id, name, price, stock, category);
    }

    fclose(fp);
    fclose(tmp);

    remove(PRODUCT_FILE);
    rename(PRODUCT_FILE_TMP, PRODUCT_FILE);
    return updated;
}
