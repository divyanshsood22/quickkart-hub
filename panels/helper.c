#include "quickkarthub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 200

void addProductToTextFileInteractive() {
    FILE *file = fopen("products_extra.txt", "a");
    if (file == NULL) {
        printf("Error opening file!\n");
        return;
    }

    char name[50], category[50], details[MAX_LEN];

    printf("\nEnter product name: ");
    scanf(" %[^\n]", name);  

    printf("Enter category: ");
    scanf(" %[^\n]", category);

    printf("Enter product details (e.g., material, color, type): ");
    scanf(" %[^\n]", details);

    fprintf(file, "%s,%s,%s\n", name, category, details);

    fclose(file);
    printf("\nProduct '%s' added to products_extra.txt successfully!\n", name);
}
