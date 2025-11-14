#ifndef ORDERS_H
#define ORDERS_H

int place_order(int user_id, int product_id, int quantity);
char* get_orders_for_user(int user_id);
char* list_all_orders();

#endif
