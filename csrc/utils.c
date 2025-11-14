#include <stdlib.h>
#include <string.h>
#include "utils.h"

char* str_duplicate(const char* src) {
    char* dst = malloc(strlen(src) + 1);
    strcpy(dst, src);
    return dst;
}
