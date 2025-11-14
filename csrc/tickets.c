#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tickets.h"

#define TICKET_FILE "tickets.txt"

// TICKET_ID|USER_ID|MESSAGE|PRIORITY|STATUS

char* tstrdup(const char* s) {
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

int create_ticket(int user_id, const char* message, int priority) {
    FILE* fp = fopen(TICKET_FILE, "a");
    if (!fp) return -1;

    int ticket_id = 1;

    FILE* read = fopen(TICKET_FILE, "r");
    if (read) {
        char line[600];
        while (fgets(line, sizeof(line), read)) ticket_id++;
        fclose(read);
    }

    fprintf(fp, "%d|%d|%s|%d|open\n", ticket_id, user_id, message, priority);
    fclose(fp);

    return ticket_id;
}

char* list_tickets() {
    FILE* fp = fopen(TICKET_FILE, "r");
    if (!fp) return tstrdup("[]");

    char buffer[10000] = "[";
    char line[600];
    int first = 1;

    while (fgets(line, sizeof(line), fp)) {
        int id, user_id, priority;
        char message[300], status[50];

        sscanf(line, "%d|%d|%[^|]|%d|%s", &id, &user_id, message, &priority, status);

        if (!first) strcat(buffer, ",");
        first = 0;

        char item[400];
        sprintf(item,
            "{\"ticket_id\":%d,\"user_id\":%d,\"message\":\"%s\","
            "\"priority\":%d,\"status\":\"%s\"}",
            id, user_id, message, priority, status);

        strcat(buffer, item);
    }

    fclose(fp);
    strcat(buffer, "]");

    return tstrdup(buffer);
}

int close_ticket(int ticket_id) {
    FILE* fp = fopen(TICKET_FILE, "r");
    if (!fp) return -1;

    FILE* temp = fopen("tickets_tmp.txt", "w");
    if (!temp) {
        fclose(fp);
        return -1;
    }

    char line[600];
    int closed = 0;

    while (fgets(line, sizeof(line), fp)) {
        int id, user_id, priority;
        char message[300], status[50];

        sscanf(line, "%d|%d|%[^|]|%d|%s", &id, &user_id, message, &priority, status);

        if (id == ticket_id) {
            fprintf(temp, "%d|%d|%s|%d|closed\n",
                    id, user_id, message, priority);
            closed = 1;
        } else {
            fputs(line, temp);
        }
    }

    fclose(fp);
    fclose(temp);

    remove(TICKET_FILE);
    rename("tickets_tmp.txt", TICKET_FILE);

    return closed ? 1 : 0;
}
