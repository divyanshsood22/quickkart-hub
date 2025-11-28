/* inventory.h */
#ifndef INVENTORY_H
#define INVENTORY_H

#include "export.h"

#ifdef __cplusplus
extern "C" {
#endif

// Read current stock of a product
EXPORT int get_stock(int product_id);

// Update stock of a product (the function you were missing)
EXPORT int update_stock_value(int product_id, int new_stock);

#ifdef __cplusplus
}
#endif

#endif
