#include "quickkarthub.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LOCAL_MAX_LEN 200

static void toLowerCaseLocal(char *str) {
    for (int i = 0; str[i]; i++)
        str[i] = tolower((unsigned char)str[i]);
}

static int countCommonWordsLocal(const char *a, const char *b) {
    char tempA[LOCAL_MAX_LEN];
    char tempB[LOCAL_MAX_LEN];
    strncpy(tempA, a ? a : "", sizeof(tempA)); tempA[sizeof(tempA)-1] = '\0';
    strncpy(tempB, b ? b : "", sizeof(tempB)); tempB[sizeof(tempB)-1] = '\0';

    toLowerCaseLocal(tempA);
    toLowerCaseLocal(tempB);

    int count = 0;
    char *token = strtok(tempA, " ,.-");
    while (token != NULL) {
        if (strstr(tempB, token)) count++;
        token = strtok(NULL, " ,.-");
    }
    return count;
}

void recommendByName(Product *head, const char *selectedName) {
    if (!head || !selectedName) {
        printf("No data available for recommendations.\n");
        return;
    }

    Product *sel = head;
    while (sel) {
        if (strcasecmp(sel->name, selectedName) == 0) break;
        sel = sel->next;
    }

    if (!sel) {
        printf("Product '%s' not found in catalog.\n", selectedName);
        return;
    }

    printf("\n=== Recommendations for '%s' ===\n", sel->name);
    int found = 0;
    Product *p = head;
    while (p) {
        if (p != sel) {
            char selText[LOCAL_MAX_LEN];
            char pText[LOCAL_MAX_LEN];
            selText[0] = '\0'; pText[0] = '\0';
            strncat(selText, sel->name, sizeof(selText)-1);
            for (int i = 0; i < sel->feedbackCount; i++) {
                strncat(selText, " ", sizeof(selText)-1);
                strncat(selText, sel->feedbacks[i], sizeof(selText)-1);
            }
            strncat(pText, p->name, sizeof(pText)-1);
            for (int i = 0; i < p->feedbackCount; i++) {
                strncat(pText, " ", sizeof(pText)-1);
                strncat(pText, p->feedbacks[i], sizeof(pText)-1);
            }

            int score = countCommonWordsLocal(selText, pText);
            if (score > 0) {
                printf("%s (Qty:%d) [score: %d]\n", p->name, p->quantity, score);
                found = 1;
            }
        }
        p = p->next;
    }
    if (!found) printf("No similar products found.\n");
}
