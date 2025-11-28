#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tickets.h"
#include "export.h"

#define TICKETS_FILE "D:/quickkart-hub/web/quickkart/tickets.txt"

// ---- safe strdup ----
static char* sdup(const char* s) {
    if (!s) return NULL;
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

// ---- create ticket ----
// FORMAT: id|user_id|message|priority|status
// Example: 1|5|Hello I need help|1|open
EXPORT int create_ticket(int user_id, const char* message, int priority)
{
    FILE* fp = fopen(TICKETS_FILE, "a+");
    if (!fp) return -1;

    // find last id
    int id = 0;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        int tid;
        if (sscanf(line, "%d|", &tid) == 1) {
            if (tid > id) id = tid;
        }
    }

    id += 1;

    // append new ticket
    fprintf(fp, "%d|%d|%s|%d|open\n", id, user_id, message, priority);
    fclose(fp);

    return id;
}

// ---- list tickets for a user ----
// returns JSON: [{id:1, message:"...", priority:0, status:"open"}, ...]
EXPORT char* list_tickets(int user_id)
{
    FILE* fp = fopen(TICKETS_FILE, "r");
    if (!fp) return sdup("[]");

    size_t cap = 4096;
    char* json = malloc(cap);
    json[0] = 0;

    strcat(json, "[");

    int first = 1;
    char line[1024];

    while (fgets(line, sizeof(line), fp)) {
        int tid, uid, priority;
        char message[600], status[50];

        if (sscanf(line, "%d|%d|%599[^|]|%d|%49s",
                   &tid, &uid, message, &priority, status) != 5)
            continue;

        if (uid != user_id) continue; // show ONLY user's own tickets

        if (!first) strcat(json, ",");
        first = 0;

        char entry[1200];
        snprintf(entry, sizeof(entry),
            "{\"id\":%d,\"user_id\":%d,\"message\":\"%s\","
            "\"priority\":%d,\"status\":\"%s\"}",
            tid, uid, message, priority, status);

        // extend if needed
        if (strlen(json) + strlen(entry) + 100 > cap) {
            cap *= 2;
            json = realloc(json, cap);
        }

        strcat(json, entry);
    }

    fclose(fp);

    strcat(json, "]");
    return json;
}


// ---- close ticket ----
EXPORT int close_ticket(int ticket_id)
{
    FILE* fp = fopen(TICKETS_FILE, "r");
    if (!fp) return -1;

    FILE* tmp = fopen("tickets_tmp.txt", "w");
    if (!tmp) { fclose(fp); return -1; }

    char line[1024];
    int found = 0;

    while (fgets(line, sizeof(line), fp)) {
        int tid, uid, priority;
        char message[600], status[50];

        if (sscanf(line, "%d|%d|%599[^|]|%d|%49s",
                   &tid, &uid, message, &priority, status) != 5)
        {
            fputs(line, tmp);
            continue;
        }

        if (tid == ticket_id) {
            found = 1;
            fprintf(tmp, "%d|%d|%s|%d|closed\n",
                    tid, uid, message, priority);
        } else {
            fputs(line, tmp);
        }
    }

    fclose(fp);
    fclose(tmp);

    remove(TICKETS_FILE);
    rename("tickets_tmp.txt", TICKETS_FILE);

    return found ? 1 : 0;
}
