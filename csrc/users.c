/* users.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "users.h"
#include "export.h"

#define USER_FILE "D:/quickkart-hub/web/quickkart/users.txt"

static char* ustrdup(const char* s) {
    char* d = malloc(strlen(s)+1);
    strcpy(d, s);
    return d;
}

EXPORT int add_user(const char* name, const char* email) {
    // if exists return -1
    FILE* fp = fopen(USER_FILE, "r");
    if (fp) {
        char line[512];
        while (fgets(line, sizeof(line), fp)) {
            int id; char nm[200], mail[200];
            if (sscanf(line, "%d|%199[^|]|%199s", &id, nm, mail) >= 2) {
                if (strcmp(mail, email) == 0) { fclose(fp); return -1; }
            }
        }
        fclose(fp);
    }
    FILE* a = fopen(USER_FILE, "a");
    if (!a) return -1;
    int id = 1;
    FILE* r = fopen(USER_FILE, "r");
    if (r) { char line[512]; while (fgets(line, sizeof(line), r)) id++; fclose(r); }
    fprintf(a, "%d|%s|%s\n", id, name, email);
    fclose(a);
    return id;
}

EXPORT int user_exists(const char* email) {
    FILE* fp = fopen(USER_FILE, "r");
    if (!fp) return 0;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id; char nm[200], mail[200];
        if (sscanf(line, "%d|%199[^|]|%199s", &id, nm, mail) >= 2) {
            if (strcmp(mail, email) == 0) { fclose(fp); return id; }
        }
    }
    fclose(fp);
    return 0;
}

EXPORT char* list_users() {
    FILE* fp = fopen(USER_FILE, "r");
    if (!fp) return ustrdup("[]");
    size_t cap = 8192; char *buf = malloc(cap);
    buf[0]='['; buf[1]=0;
    int first = 1;
    char line[512];
    while (fgets(line, sizeof(line), fp)) {
        int id; char name[200], email[200];
        if (sscanf(line, "%d|%199[^|]|%199s", &id, name, email) < 2) continue;
        char item[512]; snprintf(item, sizeof(item), "%s{\"id\":%d,\"name\":\"%s\",\"email\":\"%s\"}", first ? "" : ",", id, name, email);
        first = 0;
        if (strlen(buf)+strlen(item)+16 > cap) { cap*=2; buf = realloc(buf, cap); }
        strcat(buf, item);
    }
    fclose(fp);
    strcat(buf, "]");
    return buf;
}
