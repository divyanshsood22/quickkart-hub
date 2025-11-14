#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define REPLY_FILE "ticket_replies.txt"

// REPLY_ID|TICKET_ID|MESSAGE|ROLE|TIMESTAMP

char* tr_strdup(const char* s) {
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

char* current_timestamp() {
    time_t now = time(NULL);
    struct tm* t = localtime(&now);

    static char buffer[50];
    sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
            t->tm_year + 1900, t->tm_mon + 1, t->tm_mday,
            t->tm_hour, t->tm_min, t->tm_sec);

    return buffer;
}

int reply_ticket(int ticket_id, const char* message, const char* role) {
    FILE* fp = fopen(REPLY_FILE, "a");
    if (!fp) return -1;

    int reply_id = 1;

    FILE* read = fopen(REPLY_FILE, "r");
    if (read) {
        char line[600];
        while (fgets(line, sizeof(line), read)) reply_id++;
        fclose(read);
    }

    fprintf(fp, "%d|%d|%s|%s|%s\n",
            reply_id, ticket_id, message, role, current_timestamp());

    fclose(fp);
    return reply_id;
}

char* list_replies(int ticket_id) {
    FILE* fp = fopen(REPLY_FILE, "r");
    if (!fp) return tr_strdup("[]");

    char buffer[20000] = "[";
    char line[600];
    int first = 1;

    while (fgets(line, sizeof(line), fp)) {
        int rid, tid;
        char message[400], role[50], time[60];

        sscanf(line, "%d|%d|%[^|]|%[^|]|%s", &rid, &tid, message, role, time);

        if (tid == ticket_id) {
            if (!first) strcat(buffer, ",");
            first = 0;

            char item[500];
            sprintf(item,
                "{\"reply_id\":%d,\"ticket_id\":%d,\"message\":\"%s\","
                "\"role\":\"%s\",\"time\":\"%s\"}",
                rid, tid, message, role, time);

            strcat(buffer, item);
        }
    }

    fclose(fp);
    strcat(buffer, "]");

    return tr_strdup("[]");
}
