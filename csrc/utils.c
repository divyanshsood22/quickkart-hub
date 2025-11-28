/* utils.c */
#include <stdlib.h>
#include <string.h>
#include "utils.h"
#include "export.h"

EXPORT char* str_duplicate(const char* src) {
    if (!src) return NULL;
    char *d = malloc(strlen(src)+1); if (!d) return NULL; strcpy(d, src); return d;
}
