#ifndef USERS_H
#define USERS_H

int add_user(const char* name, const char* email);
char* list_users();
int user_exists(const char* email);

#endif
