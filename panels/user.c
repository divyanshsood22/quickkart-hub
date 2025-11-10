#include "quickkarthub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void saveCustomersToFile();

int userLogin() {
    int custId;
    printf("Enter your Customer ID: ");
    scanf("%d", &custId);
    int idx = -1;
    for (int i = 0; i < customerCount; i++) {
        if (customers[i].id == custId) {
            idx = i;
            break;
        }
    }
    if (idx == -1) {
        if (customerCount < MAX_CUSTOMERS) {
            printf("New user! Enter your name: ");
            scanf("%s", customers[customerCount].name);
            customers[customerCount].id = custId;
            customers[customerCount].monthlyPurchase = 0;
            customers[customerCount].wishlistCount = 0;
            customers[customerCount].totalSpent = 0.0f;
            idx = customerCount;
            customerCount++;

            saveCustomersToFile();
        } else {
            printf("Customer limit reached.\n");
            return -1;
        }
    }
    return idx;
}

static void viewCustomerBills(int custId) {
    char custfile[64];
    snprintf(custfile, sizeof(custfile), "invoices/bills/bill_%d.txt", custId);
    FILE *f = fopen(custfile, "r");
    if (!f) {
        printf("No bills found for customer %d.\n", custId);
        return;
    }
    printf("\n--- Bills for Customer %d ---\n", custId);
    char line[512];
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);
    }
    fclose(f);
}

void userPanel() {
    int idx = userLogin();
    if (idx == -1) {
        printf("Unable to login/create user.\n");
        return;
    }
    int choice;
    while (1) {
        printf("\n--- User Panel (Customer: %s, ID: %d) ---\n", customers[idx].name, customers[idx].id);
        printf("1. View Products\n");
        printf("2. Add to Wishlist\n");
        printf("3. View Wishlist\n");
        printf("4. Purchase Product\n");
        printf("5. Give Product Feedback\n");
        printf("6. Get Recommendations\n");
        printf("7. View My Bills\n");
        printf("8. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        switch (choice) {
            case 1: viewProducts(); break;
            case 2: addToWishlist(idx); break;
            case 3: viewWishlist(idx); break;
            case 4: purchaseProduct(); break;
            case 5: giveFeedback(); break;
            case 6: {
                int ch; while ((ch = getchar()) != '\n' && ch != EOF);
                char prodName[100];
                printf("Enter product name for recommendations: ");
                if (fgets(prodName, sizeof(prodName), stdin)) {
                    prodName[strcspn(prodName, "\r\n")] = '\0';
                    recommendByName(head, prodName);
                }
                break;
            }
            case 7: viewCustomerBills(customers[idx].id); break;
            case 8: printf("Exiting user panel...\n"); return;
            default: printf("Invalid choice. Try again.\n");
        }
    }
}
