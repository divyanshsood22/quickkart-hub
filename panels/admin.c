#include "quickkarthub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

char helpBox[MAX_HELP_MESSAGES][100];
int helpBoxCount = 0;
Product *head = NULL;
Customer customers[MAX_CUSTOMERS];
int customerCount = 0;
Order orders[MAX_ORDERS];
int orderCount = 0;
Admin admins[MAX_ADMINS] = {
    {"admin", "admin123", "super"},
    {"manager", "manager123", "manager"},
    {"support", "support123", "support"}
};
int loggedInAdmin = -1;


void saveProductsToFile();
void loadProductsFromFile();
void saveCustomersToFile();
void loadCustomersFromFile();
void generateAndSaveBill(int orderId, int custId, Product *p, int qty);


static void portable_localtime(const time_t *ts, struct tm *out) {
#if defined(_MSC_VER)
    localtime_s(out, ts);
#elif defined(_POSIX_VERSION) || defined(__unix__) || defined(__APPLE__)
    localtime_r(ts, out);
#else
    struct tm *tmp = localtime(ts);
    if (tmp) *out = *tmp;
#endif
}

int findCustomerById(int id) {
    for (int i = 0; i < customerCount; i++) {
        if (customers[i].id == id)
            return i;
    }
    return -1;
}

Product* findProductById(int id) {
    Product *temp = head;
    while (temp) {
        if (temp->id == id)
            return temp;
        temp = temp->next;
    }
    return NULL;
}

void addProduct() {
    Product *newProduct = (Product *)malloc(sizeof(Product));
    if (!newProduct) {
        printf("Memory allocation failed.\n");
        return;
    }
    printf("Enter Product ID: ");
    scanf("%d", &newProduct->id);
    printf("Enter Product Name: ");
    scanf(" %[^\n]", newProduct->name);
    printf("Enter Product Price: ");
    scanf("%f", &newProduct->price);
    printf("Enter Product Quantity: ");
    scanf("%d", &newProduct->quantity);
    printf("Enter Discount (percentage 0-100): ");
    scanf("%d", &newProduct->discount);
    if (newProduct->discount < 0 || newProduct->discount > 100) {
        printf("Invalid discount. Setting to 0.\n");
        newProduct->discount = 0;
    }
    newProduct->monthlySales = 0;
    newProduct->ratingSum = 0;
    newProduct->ratingCount = 0;
    newProduct->feedbackCount = 0;
    newProduct->next = head;
    head = newProduct;
    printf("Product added successfully!\n");

    saveProductsToFile();
}

void checkLowStock(Product *product) {
    if (product->quantity <= LOW_STOCK_THRESHOLD) {
        printf("ALERT: Low stock for product '%s' (ID: %d)!\n", product->name, product->id);
        if (helpBoxCount < MAX_HELP_MESSAGES) {
            snprintf(helpBox[helpBoxCount++], 100, "Low stock alert for '%s' (ID: %d): Only %d left.", product->name, product->id, product->quantity);
        }
    }
}

void viewProducts() {
    Product *temp = head;
    if (!temp) {
        printf("No products available.\n");
        return;
    }
    printf("\nProduct List:\n");
    while (temp) {
        float discountedPrice = temp->price * (1.0f - temp->discount / 100.0f);
        printf("ID: %d, Name: %s, Price: %.2f, Discounted: %.2f, Quantity: %d, Monthly Sales: %d, Avg Rating: %.2f\n",
            temp->id, temp->name, temp->price, discountedPrice, temp->quantity, temp->monthlySales,
            temp->ratingCount ? (float)temp->ratingSum / temp->ratingCount : 0.0);
        checkLowStock(temp);
        temp = temp->next;
    }
}

void updateProduct() {
    int id;
    printf("Enter Product ID to update: ");
    scanf("%d", &id);
    Product *temp = findProductById(id);
    if (!temp) {
        printf("Product not found.\n");
        return;
    }
    printf("Enter new name (current: %s): ", temp->name);
    scanf(" %[^\n]", temp->name);
    printf("Enter new price (current: %.2f): ", temp->price);
    scanf("%f", &temp->price);
    printf("Enter new quantity (current: %d): ", temp->quantity);
    scanf("%d", &temp->quantity);
    printf("Enter new discount (current: %d): ", temp->discount);
    scanf("%d", &temp->discount);
    if (temp->discount < 0 || temp->discount > 100) {
        printf("Invalid discount. Setting to 0.\n");
        temp->discount = 0;
    }
    printf("Product updated successfully!\n");
    checkLowStock(temp);

    saveProductsToFile();
}

void deleteProduct() {
    int id;
    printf("Enter Product ID to delete: ");
    scanf("%d", &id);
    Product *temp = head, *prev = NULL;
    while (temp && temp->id != id) {
        prev = temp;
        temp = temp->next;
    }
    if (!temp) {
        printf("Product not found.\n");
        return;
    }
    if (prev)
        prev->next = temp->next;
    else
        head = temp->next;
    free(temp);
    printf("Product deleted successfully!\n");

    saveProductsToFile();
}

void generateMonthlySales() {
    Product *temp = head;
    printf("\n--- Monthly Sales Report ---\n");
    while (temp) {
        printf("ID: %d, Name: %s, Monthly Sales: %d\n", temp->id, temp->name, temp->monthlySales);
        temp = temp->next;
    }
}

void addToWishlist(int custIdx) {
    int prodId;
    printf("Enter Product ID to add to wishlist: ");
    scanf("%d", &prodId);
    if (customers[custIdx].wishlistCount < MAX_WISHLIST) {
        customers[custIdx].wishlist[customers[custIdx].wishlistCount++] = prodId;
        printf("Product added to wishlist!\n");
        saveCustomersToFile();
    } else {
        printf("Wishlist full!\n");
    }
}

void viewWishlist(int custIdx) {
    printf("\nWishlist for %s:\n", customers[custIdx].name);
    for (int i = 0; i < customers[custIdx].wishlistCount; i++) {
        Product *p = findProductById(customers[custIdx].wishlist[i]);
        if (p)
            printf("ID: %d, Name: %s\n", p->id, p->name);
    }
}

void giveFeedback() {
    int prodId, rating;
    char feedback[100];
    printf("Enter Product ID to rate: ");
    scanf("%d", &prodId);
    Product *p = findProductById(prodId);
    if (!p) {
        printf("Product not found.\n");
        return;
    }
    printf("Enter rating (1-5): ");
    scanf("%d", &rating);
    if (rating < 1 || rating > 5) {
        printf("Invalid rating.\n");
        return;
    }
    p->ratingSum += rating;
    p->ratingCount++;
    printf("Enter feedback: ");
    scanf(" %[^\n]", feedback);
    if (p->feedbackCount < MAX_FEEDBACK) {
        strcpy(p->feedbacks[p->feedbackCount++], feedback);
    }
    printf("Thank you for your feedback!\n");

    saveProductsToFile();
}

void viewFeedbacks(int prodId) {
    Product *p = findProductById(prodId);
    if (!p) {
        printf("Product not found.\n");
        return;
    }
    printf("\nFeedbacks for %s:\n", p->name);
    for (int i = 0; i < p->feedbackCount; i++) {
        printf("%d. %s\n", i + 1, p->feedbacks[i]);
    }
}

void purchaseProduct() {
    int id, qty, custId;
    printf("Enter Customer ID: ");
    scanf("%d", &custId);
    int idx = findCustomerById(custId);
    if (idx == -1) {
        if (customerCount < MAX_CUSTOMERS) {
            customers[customerCount].id = custId;
            printf("Enter Customer Name: ");
            scanf("%s", customers[customerCount].name);
            customers[customerCount].monthlyPurchase = 0;
            customers[customerCount].wishlistCount = 0;
            customers[customerCount].totalSpent = 0.0f;
            idx = customerCount;
            customerCount++;
            saveCustomersToFile();
        } else {
            printf("Customer limit reached.\n");
            return;
        }
    }
    printf("Enter Product ID to purchase: ");
    scanf("%d", &id);
    Product *temp = findProductById(id);
    if (!temp) {
        printf("Product not found.\n");
        return;
    }
    printf("Enter quantity to purchase: ");
    scanf("%d", &qty);
    if (qty > temp->quantity) {
        printf("Not enough stock.\n");
        return;
    }

    temp->quantity -= qty;
    temp->monthlySales += qty;
    customers[idx].monthlyPurchase += qty;

    if (orderCount < MAX_ORDERS) {
        orders[orderCount].orderId = orderCount + 1;
        orders[orderCount].customerId = custId;
        orders[orderCount].productId = id;
        orders[orderCount].quantity = qty;
        strcpy(orders[orderCount].status, "pending");
        orders[orderCount].timestamp = time(NULL);
        orderCount++;
    }

    float unitPrice = temp->price;
    float discountedUnit = unitPrice * (1.0f - (temp->discount / 100.0f));
    float amount = discountedUnit * qty;
    customers[idx].totalSpent += amount;

    printf("Purchase successful! Bill amount: %.2f\n", amount);
    checkLowStock(temp);


    saveProductsToFile();
    saveCustomersToFile();

    generateAndSaveBill(orderCount, custId, temp, qty);
}

void updateOrderStatus() {
    int orderId;
    char status[20];
    printf("Enter Order ID to update: ");
    scanf("%d", &orderId);
    for (int i = 0; i < orderCount; i++) {
        if (orders[i].orderId == orderId) {
            printf("Enter new status (pending/shipped/delivered): ");
            scanf("%s", status);
            strcpy(orders[i].status, status);
            printf("Order status updated!\n");
            return;
        }
    }
    printf("Order not found.\n");
}

void viewOrders() {
    printf("\n--- Orders ---\n");
    for (int i = 0; i < orderCount; i++) {
        char timestr[64];
        struct tm t;
        portable_localtime(&orders[i].timestamp, &t);
        strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", &t);
        printf("Order ID: %d, Customer ID: %d, Product ID: %d, Quantity: %d, Status: %s, Time: %s\n",
            orders[i].orderId, orders[i].customerId, orders[i].productId, orders[i].quantity, orders[i].status, timestr);
    }
}

void viewHelpBox() {
    printf("\n--- Admin Help Box ---\n");
    if (helpBoxCount == 0) {
        printf("No messages in help box.\n");
        return;
    }
    for (int i = 0; i < helpBoxCount; i++) {
        printf("%d. %s\n", i + 1, helpBox[i]);
    }
}

void topCustomerOfMonth() {
    if (customerCount == 0) {
        printf("No customers found.\n");
        return;
    }
    int maxIdx = 0;
    for (int i = 1; i < customerCount; i++) {
        if (customers[i].totalSpent > customers[maxIdx].totalSpent)
            maxIdx = i;
    }
    printf("\n--- Top Customer (by total spent) ---\n");
    printf("ID: %d, Name: %s, Total Spent: %.2f\n", customers[maxIdx].id, customers[maxIdx].name, customers[maxIdx].totalSpent);
}

void productRecommendations() {
    printf("\n--- Product Recommendations (Top Sellers) ---\n");
    Product *temp = head;
    int maxSales = 0;
    while (temp) {
        if (temp->monthlySales > maxSales)
            maxSales = temp->monthlySales;
        temp = temp->next;
    }
    temp = head;
    while (temp) {
        if (temp->monthlySales == maxSales && maxSales > 0)
            printf("ID: %d, Name: %s, Monthly Sales: %d\n", temp->id, temp->name, temp->monthlySales);
        temp = temp->next;
    }
}

void inventoryAnalytics() {
    int totalSales = 0, lowStockCount = 0;
    Product *temp = head;
    while (temp) {
        totalSales += temp->monthlySales;
        if (temp->quantity <= LOW_STOCK_THRESHOLD)
            lowStockCount++;
        temp = temp->next;
    }
    printf("\n--- Inventory Analytics ---\n");
    printf("Total Sales (qty): %d\n", totalSales);
    printf("Low Stock Products: %d\n", lowStockCount);
}

void saveProductsToFile() {
    FILE *fp = fopen("panels/products.txt", "w");
    if (!fp) {
        perror("Failed to open products.txt for writing");
        return;
    }
    Product *p = head;
    while (p) {
        fprintf(fp, "%d,%s,%.2f,%d,%d,%d,%d,%d,%d", p->id, p->name, p->price, p->quantity, p->discount, p->monthlySales, p->ratingSum, p->ratingCount, p->feedbackCount);
        for (int i = 0; i < p->feedbackCount; i++) {
            fprintf(fp, ",%s", p->feedbacks[i]);
        }
        fprintf(fp, "\n");
        p = p->next;
    }
    fclose(fp);
}

void loadProductsFromFile() {
    FILE *fp = fopen("panels/products.txt", "r");
    if (!fp) {
        return;
    }
    char line[1024];
    Product *cur = head;
    while (cur) {
        Product *n = cur->next;
        free(cur);
        cur = n;
    }
    head = NULL;

    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        char *tokens[128];
        int tcount = 0;
        char *ptr = strtok(line, ",");
        while (ptr && tcount < 128) {
            tokens[tcount++] = ptr;
            ptr = strtok(NULL, ",");
        }
        if (tcount < 9) continue;

        Product *np = (Product *)malloc(sizeof(Product));
        if (!np) break;
        np->id = atoi(tokens[0]);
        strncpy(np->name, tokens[1], sizeof(np->name)); np->name[sizeof(np->name)-1] = '\0';
        np->price = atof(tokens[2]);
        np->quantity = atoi(tokens[3]);
        np->discount = atoi(tokens[4]);
        np->monthlySales = atoi(tokens[5]);
        np->ratingSum = atoi(tokens[6]);
        np->ratingCount = atoi(tokens[7]);
        np->feedbackCount = atoi(tokens[8]);
        if (np->feedbackCount > MAX_FEEDBACK) np->feedbackCount = MAX_FEEDBACK;
        for (int i = 0; i < np->feedbackCount && 9 + i < tcount; i++) {
            strncpy(np->feedbacks[i], tokens[9 + i], sizeof(np->feedbacks[i]));
            np->feedbacks[i][sizeof(np->feedbacks[i]) - 1] = '\0';
        }
        np->next = head;
        head = np;
    }
    fclose(fp);
}

void saveCustomersToFile() {
    FILE *fp = fopen("panels/customers.txt", "w");
    if (!fp) {
        perror("Failed to open customers.txt for writing");
        return;
    }
    for (int i = 0; i < customerCount; i++) {
        Customer *c = &customers[i];
        fprintf(fp, "%d,%s,%d,%d", c->id, c->name, c->monthlyPurchase, c->wishlistCount);
        for (int j = 0; j < c->wishlistCount; j++) {
            fprintf(fp, ",%d", c->wishlist[j]);
        }
        fprintf(fp, ",%.2f\n", c->totalSpent);
    }
    fclose(fp);
}

void loadCustomersFromFile() {
    FILE *fp = fopen("panels/customers.txt", "r");
    if (!fp) {
        customerCount = 0;
        return;
    }
    char line[512];
    customerCount = 0;
    while (fgets(line, sizeof(line), fp)) {
        line[strcspn(line, "\r\n")] = '\0';
        char *tokens[128];
        int tcount = 0;
        char *ptr = strtok(line, ",");
        while (ptr && tcount < 128) {
            tokens[tcount++] = ptr;
            ptr = strtok(NULL, ",");
        }
        if (tcount < 5) continue;
        int id = atoi(tokens[0]);
        strncpy(customers[customerCount].name, tokens[1], sizeof(customers[customerCount].name));
        customers[customerCount].name[sizeof(customers[customerCount].name)-1] = '\0';
        customers[customerCount].id = id;
        customers[customerCount].monthlyPurchase = atoi(tokens[2]);
        customers[customerCount].wishlistCount = atoi(tokens[3]);
        if (customers[customerCount].wishlistCount > MAX_WISHLIST) customers[customerCount].wishlistCount = MAX_WISHLIST;
        for (int j = 0; j < customers[customerCount].wishlistCount && 4 + j < tcount; j++) {
            customers[customerCount].wishlist[j] = atoi(tokens[4 + j]);
        }

        int totalIndex = 4 + customers[customerCount].wishlistCount;
        if (totalIndex < tcount) {
            customers[customerCount].totalSpent = (float)atof(tokens[totalIndex]);
        } else {
            customers[customerCount].totalSpent = 0.0f;
        }
        customerCount++;
        if (customerCount >= MAX_CUSTOMERS) break;
    }
    fclose(fp);
}

int loadAllData() {
    loadProductsFromFile();
    loadCustomersFromFile();

    return 1;
}

int saveAllData() {
    saveProductsToFile();
    saveCustomersToFile();
    return 1;
}


void chatbotHelp() {
    int choice;
    printf("\n--- Chatbot Help Desk ---\n");
    printf("1. How to add a product?\n");
    printf("2. How to view orders?\n");
    printf("3. How to update product?\n");
    printf("4. How to contact support?\n");
    printf("5. Exit\n");
    printf("Enter your choice: ");
    scanf("%d", &choice);
    switch (choice) {
        case 1:
            printf("Go to Admin Panel > Add Product and enter details.\n");
            break;
        case 2:
            printf("Go to Admin Panel > View Orders to see all orders.\n");
            break;
        case 3:
            printf("Go to Admin Panel > Update Product and enter product ID.\n");
            break;
        case 4:
            printf("Contact support at divyanshsood22@gmail.com\n");
            break;
        case 5:
            printf("Exiting chatbot help.\n");
            break;
        default:
            printf("Invalid choice.\n");
    }
}

int adminLogin() {
    char username[20], password[20];
    printf("Enter admin username: ");
    scanf("%s", username);
    printf("Enter admin password: ");
    scanf("%s", password);
    for (int i = 0; i < MAX_ADMINS; i++) {
        if (strcmp(admins[i].username, username) == 0 && strcmp(admins[i].password, password) == 0) {
            loggedInAdmin = i;
            printf("Login successful! Role: %s\n", admins[i].role);
            return 1;
        }
    }
    printf("Login failed.\n");
    return 0;
}

void adminPanel() {
    int choice;
    while (1) {
        printf("\n--- Admin Panel ---\n");
        printf("1. Add Product\n");
        printf("2. View Products\n");
        printf("3. Update Product\n");
        printf("4. Delete Product\n");
        printf("5. Generate Monthly Sales Report\n");
        printf("6. Purchase Product (simulate customer buying)\n");
        printf("7. View Help Box\n");
        printf("8. View Top Customer of the Month\n");
        printf("9. Product Recommendations\n");
        printf("10. Inventory Analytics\n");
        printf("11. Persist Data (save now)\n");
        printf("12. Chatbot Help Desk\n");
        printf("13. View Orders\n");
        printf("14. Update Order Status\n");
        printf("15. Add to Wishlist\n");
        printf("16. View Wishlist\n");
        printf("17. Give Product Feedback\n");
        printf("18. View Product Feedbacks\n");
        printf("19. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: addProduct(); break;
            case 2: viewProducts(); break;
            case 3: updateProduct(); break;
            case 4: deleteProduct(); break;
            case 5: generateMonthlySales(); break;
            case 6: purchaseProduct(); break;
            case 7: viewHelpBox(); break;
            case 8: topCustomerOfMonth(); break;
            case 9: productRecommendations(); break;
            case 10: inventoryAnalytics(); break;
            case 11: saveAllData(); printf("Data saved.\n"); break;
            case 12: chatbotHelp(); break;
            case 13: viewOrders(); break;
            case 14: updateOrderStatus(); break;
            case 15: {
                int custId;
                printf("Enter Customer ID: ");
                scanf("%d", &custId);
                int idx = findCustomerById(custId);
                if (idx == -1) {
                    printf("Customer not found.\n");
                } else {
                    addToWishlist(idx);
                }
                break;
            }
            case 16: {
                int custId;
                printf("Enter Customer ID: ");
                scanf("%d", &custId);
                int idx = findCustomerById(custId);
                if (idx == -1) {
                    printf("Customer not found.\n");
                } else {
                    viewWishlist(idx);
                }
                break;
            }
            case 17: giveFeedback(); break;
            case 18: {
                int prodId;
                printf("Enter Product ID: ");
                scanf("%d", &prodId);
                viewFeedbacks(prodId);
                break;
            }
            case 19: printf("Exiting admin panel...\n"); return;
            default: printf("Invalid choice. Try again.\n");
        }
    }
}
