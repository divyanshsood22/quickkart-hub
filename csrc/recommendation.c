/* recommendation.c */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "recommendation.h"
#include "products.h"
#include "export.h"

#define PRODUCT_FILE "products.txt"

static char* dupstr(const char* s){ char*d=malloc(strlen(s)+1); strcpy(d,s); return d; }

EXPORT char* get_recommendations(int product_id) {
    // find product, get category
    FILE* fp = fopen(PRODUCT_FILE, "r");
    if (!fp) return dupstr("[]");
    char line[512]; char category[64] = "";
    while (fgets(line, sizeof(line), fp)) {
        int id, stock; float price; char name[120], cat[64];
        if (sscanf(line, "%d|%119[^|]|%f|%d|%63[^\n]", &id, name, &price, &stock, cat) >= 4) {
            if (id == product_id) { strncpy(category, cat, sizeof(category)-1); break; }
        }
    }
    if (category[0] == 0) { fclose(fp); return dupstr("[]"); }
    rewind(fp);
    size_t cap=8192; char *buf=malloc(cap); buf[0]='['; buf[1]=0; int first=1;
    while (fgets(line, sizeof(line), fp)) {
        int id, stock; float price; char name[120], cat[64];
        if (sscanf(line, "%d|%119[^|]|%f|%d|%63[^\n]", &id, name, &price, &stock, cat) < 4) continue;
        if (strcmp(cat, category)==0 && id != product_id) {
            char item[256]; snprintf(item, sizeof(item), "%s{\"id\":%d,\"name\":\"%s\",\"price\":%.2f}", first ? "" : ",", id, name, price);
            first=0;
            if (strlen(buf)+strlen(item)+8>cap) { cap*=2; buf=realloc(buf,cap); }
            strcat(buf,item);
        }
    }
    fclose(fp); strcat(buf, "]"); return buf;
}
