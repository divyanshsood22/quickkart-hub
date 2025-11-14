#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "users.h"

#define USER_FILE "users.txt"

// ID|NAME|EMAIL

char* ustrdup(const char* s) {
    char* d = malloc(strlen(s) + 1);
    strcpy(d, s);
    return d;
}

int add_user(const char* name, const char* email) {
    FILE* check = fopen(USER_FILE, "r");
    if (check) {
        char line[256];
        while (fgets(line, sizeof(line), check)) {
            char mail[200], nm[200];
            int id;
            sscanf(line, "%d|%[^|]|%s", &id, nm, mail);

            if (strcmp(mail, email) == 0) {
                fclose(check);
                return -1;
            }
        }
        fclose(check);
    }

    FILE* fp = fopen(USER_FILE, "a");
    if (!fp) return -1;

    int id = 1;
    FILE* read = fopen(USER_FILE, "r");
    if (read) {
        char line[256];
        while (fgets(line, sizeof(line), read)) id++;
        fclose(read);
    }

    fprintf(fp, "%d|%s|%s\n", id, name, email);
    fclose(fp);

    return id;
}

int user_exists(const char* email) {
    FILE* fp = fopen(USER_FILE, "r");
    if (!fp) return 0;

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        int id;
        char name[200], mail[200];

        sscanf(line, "%d|%[^|]|%s", &id, name, mail);

        if (strcmp(mail, email) == 0) {
            fclose(fp);
            return id;
        }
    }

    fclose(fp);
    return 0;
}

char* list_users() {
    FILE* fp = fopen(USER_FILE, "r");
    if (!fp) return ustrdup("[]");

    char buffer[5000] = "[";
    char line[256];
    int first = 1;

    while (fgets(line, sizeof(line), fp)) {
        int id;
        char name[200], email[200];

        sscanf(line, "%d|%[^|]|%s", &id, name, email);

        if (!first) strcat(buffer, ",");
        first = 0;

        char item[300];
        sprintf(item,
                "{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\"}",
                id, name, email);

        strcat(buffer, item);
    }

    fclose(fp);
    strcat(buffer, "]");

    return ustrdup(buffer);
}
