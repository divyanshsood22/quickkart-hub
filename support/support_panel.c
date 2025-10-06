#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <dirent.h>
#include "support_panel.h"

static int ticket_counter = 1;

void to_lowercase(char *s) {
    for (int i = 0; s[i]; i++) s[i] = tolower(s[i]);
}

void raise_ticket(int urgent) {
    char name[100];
    char issue[2000];
    char line[200];
    char filename[50];

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    printf("Enter your issue (multi-line, type END on a new line to finish):\n");
    issue[0] = 0;
    while (1) {
        fgets(line, sizeof(line), stdin);
        if (strncmp(line, "END", 3) == 0) break;
        strcat(issue, line);
    }

    if (urgent) sprintf(filename, "pending_req/!%d.txt", ticket_counter);
    else sprintf(filename, "pending_req/%d.txt", ticket_counter);

    FILE *ticket_file = fopen(filename, "w");
    if (!ticket_file) {
        printf("Error: could not create ticket file.\n");
        return;
    }

    fprintf(ticket_file, "Name = %s\n", name);
    fprintf(ticket_file, "Issue = %s\n", issue);
    fclose(ticket_file);

    printf("Ticket created successfully with ID %d\n", ticket_counter);
    ticket_counter++;
}

void show_user_requests() {
    char name[100];
    char path[150];
    char line[500];

    printf("Enter your name: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;

    DIR *d = opendir("pending_req");
    struct dirent *dir;
    if (d) {
        printf("Pending tickets for %s:\n", name);
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                sprintf(path, "pending_req/%s", dir->d_name);
                FILE *f = fopen(path, "r");
                if (f) {
                    int match = 0;
                    while (fgets(line, sizeof(line), f)) {
                        if (strncmp(line, "Name = ", 7) == 0) {
                            if (strstr(line, name)) match = 1;
                        }
                    }
                    fclose(f);
                    if (match) {
                        printf("%s\n", dir->d_name);
                        f = fopen(path, "r");
                        while (fgets(line, sizeof(line), f)) printf("%s", line);
                        fclose(f);
                        printf("\n");
                    }
                }
            }
        }
        closedir(d);
    }

    d = opendir("resolved_req");
    if (d) {
        printf("Resolved tickets for %s:\n", name);
        while ((dir = readdir(d)) != NULL) {
            if (dir->d_type == DT_REG) {
                sprintf(path, "resolved_req/%s", dir->d_name);
                FILE *f = fopen(path, "r");
                if (f) {
                    int match = 0;
                    while (fgets(line, sizeof(line), f)) {
                        if (strncmp(line, "Name = ", 7) == 0) {
                            if (strstr(line, name)) match = 1;
                        }
                    }
                    fclose(f);
                    if (match) {
                        printf("%s\n", dir->d_name);
                        f = fopen(path, "r");
                        while (fgets(line, sizeof(line), f)) printf("%s", line);
                        fclose(f);
                        printf("\n");
                    }
                }
            }
        }
        closedir(d);
    }
}

void resolve_ticket(const char *ticket_file_name) {
    char source_file[100];
    char dest_file[100];
    char line[500];
    char content[5000];
    char response[2000];

    sprintf(source_file, "pending_req/%s", ticket_file_name);
    FILE *f = fopen(source_file, "r");
    if (!f) {
        printf("Ticket not found.\n");
        return;
    }

    content[0] = 0;
    printf("\t\t\t--- Ticket Content ---\n");
    while (fgets(line, sizeof(line), f)) {
        printf("%s", line);  // Show user issue to support team
        strcat(content, line);
    }
    fclose(f);
    printf("\t\t\t----------------------\n");

    printf("Enter response (Multi-line supported, type END in a new line when done):\n");
    response[0] = 0;
    while (1) {
        fgets(line, sizeof(line), stdin);
        if (strncmp(line, "END", 3) == 0) break;
        strcat(response, line);
    }

    sprintf(dest_file, "resolved_req/%s", ticket_file_name);
    FILE *out = fopen(dest_file, "w");
    fprintf(out, "%s", content);
    fprintf(out, "Response = %s\n", response);
    fclose(out);

    remove(source_file);
    printf("Ticket %s resolved and moved to resolved_req.\n", ticket_file_name);
}

void show_pending_requests() {
    DIR *d;
    struct dirent *dir;
    char pending_files[100][50];
    int count = 0;

    d = opendir("pending_req");
    if (!d) {
        printf("Pending folder not found.\n");
        return;
    }

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) {
            strcpy(pending_files[count], dir->d_name);
            count++;
        }
    }
    closedir(d);

    if (count == 0) {
        printf("No pending tickets.\n");
        return;
    }

    printf("Pending tickets:\n");

    for (int i = 0; i < count; i++) {
        if (pending_files[i][0] == '!') printf("%d. %s\n", i+1, pending_files[i]);
    }
    for (int i = 0; i < count; i++) {
        if (pending_files[i][0] != '!') printf("%d. %s\n", i+1, pending_files[i]);
    }

    int choice;
    printf("Enter ticket number to resolve (0 to cancel): ");
    scanf("%d", &choice);
    getchar();

    if (choice <= 0 || choice > count) {
        printf("Cancelled.\n");
        return;
    }

    resolve_ticket(pending_files[choice-1]);
}

void show_resolved_requests() {
    DIR *d;
    struct dirent *dir;

    d = opendir("resolved_req");
    if (!d) {
        printf("Resolved folder not found.\n");
        return;
    }

    printf("Resolved tickets:\n");
    while ((dir = readdir(d)) != NULL) {
        if (dir->d_type == DT_REG) printf("%s\n", dir->d_name);
    }
    closedir(d);
}

void search_ticket() {
    char ticket_id[50];
    printf("Enter ticket ID to search: ");
    fgets(ticket_id, sizeof(ticket_id), stdin);
    ticket_id[strcspn(ticket_id, "\n")] = 0;

    char files_to_check[4][100];
    sprintf(files_to_check[0], "pending_req/%s.txt", ticket_id);
    sprintf(files_to_check[1], "pending_req/!%s.txt", ticket_id);
    sprintf(files_to_check[2], "resolved_req/%s.txt", ticket_id);
    sprintf(files_to_check[3], "resolved_req/!%s.txt", ticket_id);

    FILE *f;
    char line[200];
    int found = 0;
    for (int i=0;i<4;i++) {
        f = fopen(files_to_check[i], "r");
        if (f) {
            while (fgets(line, sizeof(line), f)) printf("%s", line);
            fclose(f);
            found = 1;
            break;
        }
    }
    if (!found) printf("Ticket not found.\n");
}
