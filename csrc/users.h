/* users.h */
#ifndef USERS_H
#define USERS_H
#include "export.h"

EXPORT int add_user(const char* name, const char* email);
EXPORT int user_exists(const char* email);
EXPORT char* list_users();

#endif
