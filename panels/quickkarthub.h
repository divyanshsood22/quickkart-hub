#ifndef QUICKKARTHUB_H
#define QUICKKARTHUB_H

#include <time.h>

#define LOW_STOCK_THRESHOLD 5
#define MAX_HELP_MESSAGES 100
#define MAX_CUSTOMERS 100
#define MAX_PRODUCTS 100
#define MAX_ORDERS 100
#define MAX_WISHLIST 10
#define MAX_FEEDBACK 10
#define MAX_ADMINS 3
#define BILLS_FILE "invoices/ledger.txt"

typedef struct Product {
    int id;
    char name[50];
    float price;
    int quantity;
    int monthlySales;
    int discount;
    int ratingSum;
    int ratingCount;
    char feedbacks[MAX_FEEDBACK][100];
    int feedbackCount;
    struct Product *next;
} Product;

typedef struct Customer {
    int id;
    char name[50];
    int monthlyPurchase; 
    int wishlist[MAX_WISHLIST];
    int wishlistCount;
    float totalSpent; 
} Customer;

typedef struct Order {
    int orderId;
    int customerId;
    int productId;
    int quantity;
    char status[20];
    time_t timestamp;
} Order;

typedef struct Admin {
    char username[20];
    char password[20];
    char role[20];
} Admin;


extern char helpBox[MAX_HELP_MESSAGES][100];
extern int helpBoxCount;
extern Product *head;
extern Customer customers[MAX_CUSTOMERS];
extern int customerCount;
extern Order orders[MAX_ORDERS];
extern int orderCount;
extern Admin admins[MAX_ADMINS];
extern int loggedInAdmin;


int adminLogin();
void adminPanel();
int userLogin();
void userPanel();

Product* findProductById(int id);
int findCustomerById(int id);
void viewProducts();
void addToWishlist(int custIdx);
void viewWishlist(int custIdx);
void purchaseProduct();
void giveFeedback();
void viewFeedbacks(int prodId);
void recommendByName(Product *head, const char *name);
void support_handle_query(int customerId);

int loadAllData();
int saveAllData();
void loadProductsFromFile();
void saveProductsToFile();

void loadCustomersFromFile();
void saveCustomersToFile();

void generateAndSaveBill(int orderId, int custId, Product *p, int qty);
void appendBillToGlobalFile(int orderId, int custId, const char *custName, Product *p, int qty, float amount, time_t ts);

void addProductToTextFileInteractive();

#endif  
