#ifndef TICKETS_H
#define TICKETS_H

int create_ticket(int user_id, const char* message, int priority);
char* list_tickets();
int close_ticket(int ticket_id);
int reply_ticket(int ticket_id, const char* message, const char* role);
char* list_replies(int ticket_id);


#endif
