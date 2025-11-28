#ifndef TICKET_REPLIES_H
#define TICKET_REPLIES_H

#include "export.h"

/*
   reply_ticket(ticket_id, admin_name, message)

   Returns:
     1  -> success
    -1 -> file error
*/
EXPORT int reply_ticket(int ticket_id, const char* admin_name, const char* message);

/*
   list_replies(ticket_id)

   Returns a JSON string (must be freed by Python):
   [
     {"admin":"Support","message":"Hello"},
     {"admin":"Team","message":"We resolved it"}
   ]
*/
EXPORT char* list_replies(int ticket_id);

#endif
