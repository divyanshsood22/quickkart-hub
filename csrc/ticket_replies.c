#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ticket_replies.h"
#include "export.h"

#define REPLIES_FILE "D:/quickkart-hub/web/quickkart/ticket_replies.txt"

// Safe strdup
static char* sdup(const char* s) {
    if (!s) return NULL;
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

/*
   reply_ticket(ticket_id, admin_name, message)

   reply file format:
   ticket_id|admin_name|message
*/
EXPORT int reply_ticket(int ticket_id, const char* admin_name, const char* message)
{
    FILE* fp = fopen(REPLIES_FILE, "a");
    if (!fp) return -1;

    fprintf(fp, "%d|%s|%s\n", ticket_id, admin_name, message);
    fclose(fp);

    return 1;
}

/*
   list_replies(ticket_id)

   Returns JSON:
   [
     {"admin":"Support","message":"We are checking"},
     {"admin":"Support2","message":"Issue resolved"}
   ]
*/
EXPORT char* list_replies(int ticket_id)
{
    FILE* fp = fopen(REPLIES_FILE, "r");
    if (!fp) return sdup("[]");

    size_t cap = 4096;
    char* json = malloc(cap);
    json[0] = 0;

    strcat(json, "[");

    char line[1024];
    int first = 1;

    while (fgets(line, sizeof(line), fp)) {
        int tid;
        char admin[200], msg[700];

        if (sscanf(line, "%d|%199[^|]|%699[^\n]",
                   &tid, admin, msg) != 3)
            continue;

        if (tid != ticket_id)
            continue;

        if (!first) strcat(json, ",");
        first = 0;

        char entry[1200];
        snprintf(entry, sizeof(entry),
                 "{\"admin\":\"%s\",\"message\":\"%s\"}",
                  admin, msg);

        if (strlen(json) + strlen(entry) + 50 > cap) {
            cap *= 2;
            json = realloc(json, cap);
        }

        strcat(json, entry);
    }

    fclose(fp);
    strcat(json, "]");

    return json;
}
