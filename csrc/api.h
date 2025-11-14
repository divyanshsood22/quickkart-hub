#ifndef API_H
#define API_H

// PRODUCT FUNCTIONS
int add_product(const char* name, float price, int stock);
int remove_product(int product_id);
char* list_products();
char* get_product(int product_id); 

// INVENTORY FUNCTIONS
int update_stock(int product_id, int new_qty);
int get_stock(int product_id);

// USER FUNCTIONS
int add_user(const char* name, const char* email);
char* list_users();
int user_exists(const char* email);

// ORDER FUNCTIONS
int place_order(int user_id, int product_id, int quantity);
char* get_orders_for_user(int user_id);
char* list_all_orders();                

// TICKET FUNCTIONS
int create_ticket(int user_id, const char* message, int priority);
char* list_tickets();              
int close_ticket(int ticket_id);

// INVOICE FUNCTIONS
char* generate_invoice(int order_id);

// RECOMMENDATION FUNCTIONS
char* get_recommendations(int product_id);

#endif
