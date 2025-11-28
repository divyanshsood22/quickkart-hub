#ifndef TICKETS_H
#define TICKETS_H

#include "export.h"

// create_ticket(user_id, message, priority)
// priority: 0 = low, 1 = high
EXPORT int create_ticket(int user_id, const char* message, int priority);

// list_tickets(user_id)
// returns JSON string
EXPORT char* list_tickets(int user_id);

// close_ticket(ticket_id)
EXPORT int close_ticket(int ticket_id);

#endif
