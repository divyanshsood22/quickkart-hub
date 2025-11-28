/* orders.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "orders.h"
#include "products.h"
#include "export.h"

#define ORDER_FILE "D:/quickkart-hub/web/quickkart/orders.txt"
#define PRODUCT_FILE "D:/quickkart-hub/web/quickkart/products.txt"

/* Safe strdup */
static char* sdup(const char* s) {
    if (!s) return NULL;
    char* d = malloc(strlen(s)+1);
    strcpy(d, s);
    return d;
}

/* INTERNAL HELPERS */
static float get_price(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return -1;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id, stock; float price;
        char name[120], category[64];

        if (sscanf(line, "%d|%119[^|]|%f|%d|%63[^\n]",
                   &id, name, &price, &stock, category) < 4)
            continue;

        if (id == product_id) {
            fclose(fp);
            return price;
        }
    }
    fclose(fp);
    return -1;
}

static int get_stock(int product_id) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return -1;

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id, stock; float price;
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

static void set_stock(int product_id, int new_stock) {
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return;

    FILE* tmp = fopen("products_tmp.txt", "w");
    if (!tmp) { fclose(fp); return; }

    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id, stock; float price;
        char name[120], category[64];

        sscanf(line, "%d|%119[^|]|%f|%d|%63[^\n]",
               &id, name, &price, &stock, category);

        if (id == product_id)
            stock = new_stock;

        fprintf(tmp, "%d|%s|%.2f|%d|%s\n",
                id, name, price, stock, category);
    }

    fclose(fp);
    fclose(tmp);
    remove(PRODUCT_FILE);
    rename("products_tmp.txt", PRODUCT_FILE);
}

/* ========== PUBLIC EXPORTS ========== */

EXPORT int create_order_group(int user_id) {
    int max_gid = 0;
    FILE* fp = fopen(ORDER_FILE, "r");

    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            int gid;
            if (sscanf(line, "%d|", &gid) == 1) {
                if (gid > max_gid) max_gid = gid;
            }
        }
        fclose(fp);
    }

    return max_gid + 1;
}

EXPORT int place_order_group(int group_id, int product_id, int qty) {
    if (qty <= 0) return -1;

    int stock = get_stock(product_id);
    if (stock < qty) return -2;

    float price = get_price(product_id);
    if (price < 0) return -3;

    set_stock(product_id, stock - qty);
    float total = price * qty;

    FILE* fp = fopen(ORDER_FILE, "a");
    if (!fp) return -4;

    /* format: group_id|user_id|product_id|quantity|total */
    fprintf(fp, "%d|%d|%d|%d|%.2f\n",
            group_id, 1, product_id, qty, total);

    fclose(fp);
    return 1;
}

static int append_str(char **buf, size_t *cap, size_t *len, const char *s) {
    size_t need = strlen(s);
    if (*len + need + 1 >= *cap) {
        size_t newcap = (*cap) * 2;
        while (*len + need + 1 >= newcap) newcap *= 2;
        char *tmp = realloc(*buf, newcap);
        if (!tmp) return 0;
        *buf = tmp;
        *cap = newcap;
    }
    memcpy(*buf + *len, s, need);
    *len += need;
    (*buf)[*len] = '\0';
    return 1;
}

/* safer number appenders */
static int append_fmt(char **buf, size_t *cap, size_t *len, const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    /* try to write into a small stack buffer first */
    char small[512];
    int wrote = vsnprintf(small, sizeof(small), fmt, ap);
    va_end(ap);
    if (wrote < 0) return 0;
    if ((size_t)wrote < sizeof(small)) {
        return append_str(buf, cap, len, small);
    }
    /* too big for stack buffer: allocate temp */
    char *tmpbuf = malloc(wrote + 1);
    if (!tmpbuf) return 0;
    va_start(ap, fmt);
    vsnprintf(tmpbuf, wrote + 1, fmt, ap);
    va_end(ap);
    int ok = append_str(buf, cap, len, tmpbuf);
    free(tmpbuf);
    return ok;
}

EXPORT char* get_invoice_for_group(int group_id) {
    FILE* fp = fopen(ORDER_FILE, "r");
    if (!fp) return sdup("{}");

    size_t cap = 4096;
    char *json = malloc(cap);
    if (!json) { fclose(fp); return sdup("{}"); }
    json[0] = '\0';
    size_t len = 0;

    /* start JSON */
    if (!append_fmt(&json, &cap, &len, "{\"order_id\":%d,\"items\":[", group_id)) {
        fclose(fp); free(json); return sdup("{}");
    }

    int first = 1;
    float grand_total = 0.0f;
    char line[512];

    while (fgets(line, sizeof(line), fp)) {
        int gid = 0, uid = 0, pid = 0, qty = 0;
        float total = 0.0f;

        if (sscanf(line, "%d|%d|%d|%d|%f", &gid, &uid, &pid, &qty, &total) != 5) {
            continue;
        }

        if (gid != group_id) continue;

        Product p = get_product_struct(pid);
        if (p.id == -1) continue;

        /* Add comma between items */
        if (!first) {
            if (!append_str(&json, &cap, &len, ",")) { fclose(fp); free(json); return sdup("{}"); }
        }
        first = 0;

        /* Build single item JSON. Escape quotes in name (basic) */
        /* We'll create a safe name copy that replaces " with ' to avoid invalid JSON */
        char safe_name[256];
        size_t i, j;
        for (i = j = 0; i < sizeof(p.name) && p.name[i] != '\0' && j + 1 < sizeof(safe_name); ++i) {
            if (p.name[i] == '"') safe_name[j++] = '\'';
            else safe_name[j++] = p.name[i];
        }
        safe_name[j] = '\0';

        if (!append_fmt(&json, &cap, &len,
                        "{\"product_id\":%d,\"name\":\"%s\",\"price\":%.2f,\"quantity\":%d,\"total\":%.2f}",
                        pid, safe_name, p.price, qty, total)) {
            fclose(fp); free(json); return sdup("{}");
        }

        grand_total += total;
    }

    fclose(fp);

    /* close items array and add grand_total */
    if (!append_fmt(&json, &cap, &len, "],\"grand_total\":%.2f}", grand_total)) {
        free(json);
        return sdup("{}");
    }

    /* shrink to fit optionally */
    char *shrink = realloc(json, len + 1);
    if (shrink) json = shrink;

    return json;
}
