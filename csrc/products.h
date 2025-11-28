/* products.h */
#ifndef PRODUCTS_H
#define PRODUCTS_H
#include "export.h"

typedef struct {
    int id;
    char name[100];
    float price;
    int stock;
    char category[50];
} Product;

EXPORT int add_product(const char* name, float price, int stock, const char* category);
EXPORT int remove_product(int product_id);
EXPORT char* list_products();
EXPORT char* get_product(int product_id);
EXPORT Product get_product_struct(int product_id);
EXPORT int get_products_by_category(const char* category, Product** out_list);
EXPORT int update_product_stock(int product_id, int new_stock);
EXPORT int update_price(int product_id, float new_price);

#endif
