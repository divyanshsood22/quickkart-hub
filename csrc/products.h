#ifndef PRODUCTS_H
#define PRODUCTS_H

int add_product(const char* name, float price, int stock);
int remove_product(int product_id);
char* list_products();
char* get_product(int product_id);

#endif
