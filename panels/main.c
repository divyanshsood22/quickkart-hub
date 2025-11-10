#include <stdio.h>
#include <string.h>
#include "quickkarthub.h"
#include "../support/support_panel.h"

int main() {
    int choice;
    loadAllData();
    init_support_module();
    printf("\n\t\t\tWelcome to QuickKartHub!\n");
    while (1) {
        printf("\n=== QuickKartHub Main Menu ===\n");
        printf("1. Admin Login\n");
        printf("2. User Login\n");
        printf("3. Save Data Now\n");
        printf("4. Support\n");
        printf("5. Exit\n");
        printf("Enter your choice: ");
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Invalid input. Try again.\n");
            continue;
        }
        if (choice == 1) {
            if (adminLogin()) adminPanel();
            else printf("Admin login failed.\n");
        } else if (choice == 2) {
            userPanel();
        } else if (choice == 3) {
            if (saveAllData()) printf("Data saved successfully.\n");
            else printf("Save failed.\n");
        } else if (choice == 4) {
            int ch;
            while ((ch = getchar()) != '\n' && ch != EOF) {}
            open_support_menu_from_main();
        } else if (choice == 5) {
            saveAllData();
            printf("Exiting. Data saved. Goodbye!\n");
            break;
        } else {
            printf("Invalid choice.\n");
        }
    }
    return 0;
}
