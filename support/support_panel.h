#ifndef SUPPORT_PANEL_H
#define SUPPORT_PANEL_H

void init_support_module(void);
void open_support_menu_from_main(void);
void raise_ticket(int urgent);
void show_pending_requests(void);
void show_resolved_requests(void);
void show_user_requests(void);
void search_ticket(void);
void resolve_ticket(int id);

#endif
