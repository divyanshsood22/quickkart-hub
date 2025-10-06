#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "support_panel.h"

#define ADMIN_CODE "123456"

int main() {
    int choice;
    char input[100];

    while (1) {
        printf("\n1. Raise a normal ticket\n");
        printf("2. Raise an urgent ticket\n");
        printf("3. Show existing requests\n");
        printf("4. Exit\n");
        printf("Enter your choice :");
        fgets(input, sizeof(input), stdin);
        input[strcspn(input, "\n")] = 0;

        if (strcmp(input, ADMIN_CODE) == 0) {
            while (1) {
                printf("\t\t\t--- Admin Panel ---\n");
                printf("1. Show pending requests\n");
                printf("2. Show resolved requests\n");
                printf("3. Search a ticket\n");
                printf("4. Exit\n");
                printf("Enter choice: ");
                fgets(input, sizeof(input), stdin);
                int adminChoice = atoi(input);

                if (adminChoice == 1) {
                    show_pending_requests();
                } else if (adminChoice == 2) {
                    show_resolved_requests();
                } else if (adminChoice == 3) {
                    search_ticket();
                } else if (adminChoice == 4) {
                    break;
                }
            }
        } else {
            choice = atoi(input);
            if (choice == 1) {
                raise_ticket(0);
            } else if (choice == 2) {
                raise_ticket(1);
            } else if (choice == 3) {
                show_user_requests();
            } else if (choice == 4) {
                printf("Exiting...\n");
                break;
            } else {
                printf("Invalid input.\n");
            }
        }
    }

    return 0;
}