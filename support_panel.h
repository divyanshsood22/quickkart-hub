#ifndef SUPPORT_PANEL_H
#define SUPPORT_PANEL_H

void raise_ticket(int urgent);
void show_user_requests();
void resolve_ticket(const char *ticket_file_name);
void show_pending_requests();
void show_resolved_requests();
void search_ticket();

#endif
