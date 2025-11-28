/* orders.h */
#ifndef ORDERS_H
#define ORDERS_H
#include "export.h"

EXPORT int create_order_group(int user_id);
EXPORT int place_order_group(int group_id, int product_id, int quantity);
EXPORT char* get_orders_for_user(int user_id);
EXPORT char* list_all_orders();

EXPORT char* get_invoice_for_group(int group_id);

#endif
