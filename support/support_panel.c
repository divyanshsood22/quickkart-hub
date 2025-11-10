#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <time.h>
#include <errno.h>
#ifdef _WIN32
  #include <direct.h>
  #define MKDIR(p) _mkdir(p)
#else
  #define MKDIR(p) mkdir(p,0755)
#endif
#include "../panels/quickkarthub.h"
#include "support_panel.h"

static int ticket_counter = 1;

static void ensure_dir(const char *d) {
    struct stat st;
    if (stat(d,&st) == -1) MKDIR(d);
}

static int parse_id_from_filename(const char *fn) {
    int start = (fn[0] == '!') ? 1 : 0;
    int id = 0;
    sscanf(fn + start, "%d", &id);
    return id;
}

void init_support_module(void) {
    DIR *dp;
    struct dirent *ent;
    int maxid = 0;
    ensure_dir("support/pending_req");
    ensure_dir("support/resolved_req");
    dp = opendir("support/pending_req");
    if (dp) {
        while ((ent = readdir(dp)) != NULL) {
            if (ent->d_type == DT_REG) {
                int id = parse_id_from_filename(ent->d_name);
                if (id > maxid) maxid = id;
            }
        }
        closedir(dp);
    }
    dp = opendir("support/resolved_req");
    if (dp) {
        while ((ent = readdir(dp)) != NULL) {
            if (ent->d_type == DT_REG) {
                int id = parse_id_from_filename(ent->d_name);
                if (id > maxid) maxid = id;
            }
        }
        closedir(dp);
    }
    if (maxid >= ticket_counter) ticket_counter = maxid + 1;
}

static int write_ticket_file(const char *dir, int id, int urgent, const char *user, const char *msg) {
    char name[256];
    if (urgent) snprintf(name, sizeof(name), "%s/!%d.txt", dir, id);
    else snprintf(name, sizeof(name), "%s/%d.txt", dir, id);
    FILE *f = fopen(name, "w");
    if (!f) return -1;
    fprintf(f, "TicketID:%d\n", id);
    fprintf(f, "Urgent:%d\n", urgent ? 1:0);
    fprintf(f, "User:%s\n", user ? user : "guest");
    fprintf(f, "Message:\n%s\n", msg ? msg : "");
    fclose(f);
    return 0;
}

void raise_ticket(int urgent) {
    char user[128];
    char msg[1024];
    printf("Enter your name: ");
    if (!fgets(user, sizeof(user), stdin)) return;
    user[strcspn(user, "\r\n")] = '\0';
    if (strlen(user) == 0) strncpy(user, "guest", sizeof(user));
    printf("Enter your message (end with a blank line):\n");
    msg[0] = '\0';
    while (1) {
        char line[512];
        if (!fgets(line, sizeof(line), stdin)) break;
        if (line[0] == '\n' || line[0] == '\r') break;
        strncat(msg, line, sizeof(msg) - strlen(msg) - 1);
    }
    int id = ticket_counter++;
    if (write_ticket_file("support/pending_req", id, urgent, user, msg) == 0) {
        printf("Ticket created with ID %d\n", id);
    } else {
        printf("Failed to create ticket. errno=%d\n", errno);
    }
}

void show_pending_requests(void) {
    DIR *dp = opendir("support/pending_req");
    if (!dp) { printf("No pending_req directory or cannot open it.\n"); return; }
    struct dirent *ent;
    int found = 0;
    while ((ent = readdir(dp)) != NULL) {
        if (ent->d_type != DT_REG) continue;
        found = 1;
        char path[256];
        snprintf(path, sizeof(path), "support/pending_req/%s", ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) { printf("Cannot open %s\n", path); continue; }
        printf("---- %s ----\n", ent->d_name);
        char buf[512];
        while (fgets(buf, sizeof(buf), f)) printf("%s", buf);
        printf("\n");
        fclose(f);
    }
    closedir(dp);
    if (!found) printf("No pending requests.\n");
}

void show_resolved_requests(void) {
    DIR *dp = opendir("support/resolved_req");
    if (!dp) { printf("No resolved_req directory or cannot open it.\n"); return; }
    struct dirent *ent;
    int found = 0;
    while ((ent = readdir(dp)) != NULL) {
        if (ent->d_type != DT_REG) continue;
        found = 1;
        char path[256];
        snprintf(path, sizeof(path), "support/resolved_req/%s", ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) { printf("Cannot open %s\n", path); continue; }
        printf("---- %s ----\n", ent->d_name);
        char buf[512];
        while (fgets(buf, sizeof(buf), f)) printf("%s", buf);
        printf("\n");
        fclose(f);
    }
    closedir(dp);
    if (!found) printf("No resolved requests.\n");
}

void show_user_requests(void) {
    char user[128];
    printf("Enter username to filter: ");
    if (!fgets(user, sizeof(user), stdin)) return;
    user[strcspn(user, "\r\n")] = '\0';
    if (strlen(user) == 0) strncpy(user, "guest", sizeof(user));
    DIR *dp = opendir("support/pending_req");
    if (!dp) { printf("No pending_req directory or cannot open it.\n"); return; }
    struct dirent *ent;
    int found = 0;
    while ((ent = readdir(dp)) != NULL) {
        if (ent->d_type != DT_REG) continue;
        char path[256];
        snprintf(path, sizeof(path), "support/pending_req/%s", ent->d_name);
        FILE *f = fopen(path, "r");
        if (!f) continue;
        char line[512];
        int printed = 0;
        while (fgets(line, sizeof(line), f)) {
            if (strncmp(line, "User:", 5) == 0) {
                char *uname = line + 5;
                while (*uname == ' ') uname++;
                uname[strcspn(uname, "\r\n")] = '\0';
                if (strcmp(uname, user) == 0) {
                    if (!printed) printf("---- %s ----\n", ent->d_name);
                    printed = 1;
                    found = 1;
                } else break;
            }
            if (printed) printf("%s", line);
        }
        if (printed) printf("\n");
        fclose(f);
    }
    closedir(dp);
    if (!found) printf("No tickets found for user %s\n", user);
}

void search_ticket(void) {
    char buf[64];
    printf("Enter ticket id to search: ");
    if (!fgets(buf, sizeof(buf), stdin)) return;
    int id = atoi(buf);
    if (id <= 0) { printf("Invalid id\n"); return; }
    char name1[128], name2[128];
    snprintf(name1, sizeof(name1), "support/pending_req/%d.txt", id);
    snprintf(name2, sizeof(name2), "support/pending_req/!%d.txt", id);
    FILE *f = fopen(name1, "r");
    if (!f) f = fopen(name2, "r");
    if (!f) {
        snprintf(name1, sizeof(name1), "support/resolved_req/%d.txt", id);
        snprintf(name2, sizeof(name2), "support/resolved_req/!%d.txt", id);
        f = fopen(name1, "r");
        if (!f) f = fopen(name2, "r");
    }
    if (!f) { printf("Ticket %d not found\n", id); return; }
    char line[512];
    while (fgets(line, sizeof(line), f)) printf("%s", line);
    printf("\n");
    fclose(f);
}

void resolve_ticket(int id) {
    char src1[256];
    char src2[256];
    char dst1[256];
    char dst2[256];
    snprintf(src1, sizeof(src1), "support/pending_req/%d.txt", id);
    snprintf(src2, sizeof(src2), "support/pending_req/!%d.txt", id);
    snprintf(dst1, sizeof(dst1), "support/resolved_req/%d.txt", id);
    snprintf(dst2, sizeof(dst2), "support/resolved_req/!%d.txt", id);
    FILE *f = fopen(src1, "r");
    int used_src = 0;
    if (!f) {
        f = fopen(src2, "r");
        if (!f) {
            printf("Ticket %d not found in pending_req.\n", id);
            return;
        } else used_src = 1;
    } else used_src = 0;
    char content[8192];
    content[0] = '\0';
    char line[1024];
    while (fgets(line, sizeof(line), f)) {
        strncat(content, line, sizeof(content) - strlen(content) - 1);
    }
    fclose(f);
    char responder[128];
    char response[4096];
    printf("Enter your name (resolver): ");
    if (!fgets(responder, sizeof(responder), stdin)) return;
    responder[strcspn(responder, "\r\n")] = '\0';
    if (strlen(responder) == 0) strncpy(responder, "support", sizeof(responder));
    printf("Enter response (end with a blank line):\n");
    response[0] = '\0';
    while (1) {
        char rline[1024];
        if (!fgets(rline, sizeof(rline), stdin)) break;
        if (rline[0] == '\n' || rline[0] == '\r') break;
        strncat(response, rline, sizeof(response) - strlen(response) - 1);
    }
    time_t t = time(NULL);
    char timestr[128];
    struct tm *tm = localtime(&t);
    if (tm) strftime(timestr, sizeof(timestr), "%Y-%m-%d %H:%M:%S", tm);
    else snprintf(timestr, sizeof(timestr), "unknown");
    FILE *out = NULL;
    if (used_src == 0) out = fopen(dst1, "w");
    else out = fopen(dst2, "w");
    if (!out) {
        printf("Failed to open resolved file for writing. errno=%d\n", errno);
        return;
    }
    fprintf(out, "%s", content);
    fprintf(out, "ResponseBy:%s\n", responder);
    fprintf(out, "ResponseAt:%s\n", timestr);
    fprintf(out, "Response:\n%s\n", response);
    fclose(out);
    if (used_src == 0) {
        if (remove(src1) == 0) printf("Ticket %d resolved and moved to resolved_req.\n", id);
        else printf("Ticket %d resolved but failed to remove pending file %s\n", id, src1);
    } else {
        if (remove(src2) == 0) printf("Ticket %d resolved and moved to resolved_req.\n", id);
        else printf("Ticket %d resolved but failed to remove pending file %s\n", id, src2);
    }
}

void open_support_menu_from_main(void) {
    char buf[128];
    while (1) {
        printf("\n=== Support ===\n");
        printf("1. Create ticket\n");
        printf("2. Create urgent ticket\n");
        printf("3. Show pending requests\n");
        printf("4. Show resolved requests\n");
        printf("5. Show my requests\n");
        printf("6. Search ticket\n");
        printf("7. Admin panel\n");
        printf("8. Return to main menu\n");
        printf("Enter choice: ");
        if (!fgets(buf, sizeof(buf), stdin)) return;
        int c = atoi(buf);
        if (c == 1) raise_ticket(0);
        else if (c == 2) raise_ticket(1);
        else if (c == 3) show_pending_requests();
        else if (c == 4) show_resolved_requests();
        else if (c == 5) show_user_requests();
        else if (c == 6) search_ticket();
        else if (c == 7) {
            char codebuf[32];
            printf("Enter admin code: ");
            if (!fgets(codebuf, sizeof(codebuf), stdin)) continue;
            codebuf[strcspn(codebuf, "\r\n")] = '\0';
            if (strcmp(codebuf, "123456") == 0) {
                while (1) {
                    printf("\n--- Support Admin ---\n");
                    printf("1. Show pending requests\n");
                    printf("2. Show resolved requests\n");
                    printf("3. Search a ticket\n");
                    printf("4. Resolve ticket\n");
                    printf("5. Exit admin\n");
                    printf("Enter choice: ");
                    if (!fgets(buf, sizeof(buf), stdin)) break;
                    int ac = atoi(buf);
                    if (ac == 1) show_pending_requests();
                    else if (ac == 2) show_resolved_requests();
                    else if (ac == 3) search_ticket();
                    else if (ac == 4) {
                        char idbuf[32];
                        printf("Enter ticket id to resolve: ");
                        if (!fgets(idbuf, sizeof(idbuf), stdin)) continue;
                        int rid = atoi(idbuf);
                        if (rid > 0) resolve_ticket(rid);
                        else printf("Invalid id.\n");
                    }
                    else if (ac == 5) break;
                    else printf("Invalid choice.\n");
                }
            } else {
                printf("Wrong admin code.\n");
            }
        }
        else if (c == 8) return;
        else printf("Invalid choice\n");
    }
}
