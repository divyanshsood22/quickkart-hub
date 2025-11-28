/* invoice.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "invoice.h"
#include "products.h"
#include "export.h"

#define ORDER_FILE "D:/quickkart-hub/web/quickkart/orders.txt"

/*
ORDER FILE FORMAT (from your orders.c)
---------------------------------------
group_id | user_id | product_name | price | qty | total
*/

static char* str_dup(const char* s) {
    size_t n = strlen(s) + 1;
    char* d = malloc(n);
    memcpy(d, s, n);
    return d;
}