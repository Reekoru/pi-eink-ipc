// Simple UNIX domain socket client for two-way IPC tests.
// Build: gcc -Wall -Wextra -O2 -o socket_test ipc/socket_test.c

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include "json_types.h"
#include "utils/message.h"

#define SOCKET_PATH "/tmp/suwayomi.sock"
#define BUF_SIZE 10240
#define ITEM_BUF_SIZE 10

int ipc_send_message(int fd, const char *message);
int ipc_recv_message(int fd, uint8_t *status, char* output, size_t output_size);
int ipc_connect(int fd, const char *socket_path, int max_attempts);

char input[BUF_SIZE];
char output[BUF_SIZE];
GraphQL_ExtensionList_t extensions;
GraphQL_MangaList_t mangas;
GraphQL_ChapterList_t chapters;
GraphQL_SourceList_t sources;
uint8_t status = 0;

static void free_extension_list(GraphQL_ExtensionList_t *list)
{
    if (list == NULL || list->items == NULL) {
        return;
    }

    ExtensionInput_t *ext = NULL;
    FOREACH_ITEM(ext, (*list), ExtensionInput_t)
    {
        free(ext->apkName);
        free(ext->iconUrl);
        free(ext->lang);
        free(ext->name);
        free(ext->pkgName);
        free(ext->repo);
        free(ext->versionName);
    }

    free(list->items);
    list->items = NULL;
    list->itemCount = 0;
    list->totalCount = 0;
}

static void free_manga_list(GraphQL_MangaList_t *list)
{
    if (list == NULL || list->items == NULL) {
        return;
    }

    Manga_t *manga = NULL;
    FOREACH_ITEM(manga, (*list), Manga_t)
    {
        free(manga->title);
        free(manga->author);
        free(manga->thumbnailPath);
    }

    free(list->items);
    list->items = NULL;
    list->itemCount = 0;
    list->totalCount = 0;
}

static void clear_terminal()
{
    printf("\033[H\033[J");
}

static void print_messages()
{
    puts("Enter a message to send (or 'exit' to quit): ");
    puts("Available messages:");
    for(uint8_t i = 0; i < IPC_MSG_END; i++)
    {
        printf("%d: %s\n", i + 1, IPCMessageStrings[i]);
    }
}

static void process_message(int fd)
{
    if (ipc_recv_message(fd, &status, output, BUF_SIZE) != 0) {
        printf("Timed out waiting for response\n");
    }
}

static void print_message()
{
    clear_terminal();
    printf("Received message with status %d: %s\n", status, output);
    puts("");
}

static void process_input(int fd)
{
    char in[BUF_SIZE];
    ExtensionInput_t *ext;
    clear_terminal();
    while(1)
    {
        // clear_terminal();
        print_messages();
        if (fgets(in, sizeof(in), stdin) == NULL) {
            break;
        }

        // Remove newline character
        in[strcspn(in, "\n")] = '\0';

        if (strcmp(in, "exit") == 0) {
            break;
        }

        // parse int
        int choice = atoi(in);
        choice -= 1;
        switch(choice)
        {
            case IPC_MSG_PING:
                create_message(IPC_MSG_PING, NULL, NULL, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                print_message();
                break;
            case IPC_MSG_GET_SETTINGS:
                create_message(IPC_MSG_GET_SETTINGS, NULL, NULL, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                print_message();

                break;
            case IPC_MSG_GET_EXTENSIONS:
            {
                ExtensionInput_t params = {
                    .apkName = NULL,
                    .hasUpdate = -1,
                    .iconUrl = NULL,
                    .isInstalled = -1,
                    .isNsfw = false,
                    .isObsolete = -1,
                    .lang = NULL,
                    .name = NULL,
                    .pkgName = NULL,
                    .repo = NULL,
                    .versionCode = -1,
                    .versionName = NULL
                };

                puts("Enter page number: ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }

                int page = atoi(input);
                if(page < 1)
                {
                    printf("Invalid page number: %s\n", input);
                    break;
                }
                GraphQL_Pagination_t pagination = {
                    .first = 10,
                    .offset = (page - 1) * 10
                };

                create_message(IPC_MSG_GET_EXTENSIONS, &pagination, &params, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);

                json_to_struct(J2S_EXTENSION, output, &extensions);
                printf("Extension Count: %d\n", extensions.itemCount);
                FOREACH_ITEM(ext, extensions, ExtensionInput_t)
                {
                    printf("Extension %d: %s (isInstalled: %d)\n", i + 1, ext->pkgName, ext->isInstalled);
                }

                break;
            }
            case IPC_MSG_INSTALL_EXTENSION:
            case IPC_MSG_UNINSTALL_EXTENSION:
                ExtensionInput_t params = {
                    .apkName = NULL,
                    .hasUpdate = -1,
                    .iconUrl = NULL,
                    .isInstalled = (choice == IPC_MSG_INSTALL_EXTENSION) ? -1 : true,
                    .isNsfw = (choice == IPC_MSG_INSTALL_EXTENSION) ? false : -1,
                    .isObsolete = -1,
                    .lang = NULL,
                    .name = NULL,
                    .pkgName = NULL,
                    .repo = NULL,
                    .versionCode = -1,
                    .versionName = NULL
                };

                puts("Please select an extension:");
                puts("");

                puts("Enter page number: ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }

                int page = atoi(input);
                if(page < 1)
                {
                    printf("Invalid page number: %s\n", input);
                    break;
                }
                GraphQL_Pagination_t pagination = {
                    .first = 10,
                    .offset = (page - 1) * 10
                };

                create_message(IPC_MSG_GET_EXTENSIONS, &pagination, &params, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                json_to_struct(J2S_EXTENSION, output, &extensions);
                printf("Extension Count: %d\n", extensions.itemCount);
                FOREACH_ITEM(ext, extensions, ExtensionInput_t)
                {
                    printf("Extension %d: %s (isInstalled: %d)\n", i + 1, ext->pkgName, ext->isInstalled);
                }

                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
                int ext_choice = atoi(input);
                if(ext_choice < 1 || ext_choice > extensions.itemCount)
                {
                    printf("Invalid extension choice: %s\n", input);
                    break;
                }
                const ExtensionInput_t *selected_ext = GET_ITEM(extensions, ext_choice - 1, ExtensionInput_t);
                UpdateExtensionInput_t update_input = {
                    .id = selected_ext->pkgName,
                    .patch = {
                        .install = (choice == IPC_MSG_INSTALL_EXTENSION) ? 1 : -1,
                        .uninstall = (choice == IPC_MSG_UNINSTALL_EXTENSION) ? 1 : -1,
                        .update = -1
                    }
                };
                create_message(choice, NULL, &update_input, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                print_message();
                break;

            case IPC_MSG_GET_MANGAS:
            {
                puts("Please select source:");
                puts("");

                SourceInputCondition_t params = {
                    .id = -1,
                    .isNsfw = -1,
                    .lang = NULL,
                    .name = NULL
                };

                puts("Enter page number: ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }

                int page = atoi(input);
                if(page < 1)
                {
                    printf("Invalid page number: %s\n", input);
                    break;
                }
                GraphQL_Pagination_t pagination = {
                    .first = 10,
                    .offset = (page - 1) * 10
                };
                create_message(IPC_MSG_GET_SOURCES, &pagination, &params, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                
                json_to_struct(J2S_SOURCE, output, &sources);

                Source_t *source;
                FOREACH_ITEM(source, sources, Source_t)
                {
                    printf("Source %d: %s (id: %s)\n", i + 1, source->displayName, source->id);
                }

                if(fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
                int repo_choice = atoi(input);
                if(repo_choice < 1 || repo_choice > sources.itemCount)
                {
                    printf("Invalid source choice: %s\n", input);
                    break;
                }

                const Source_t *selected_repo_source = GET_ITEM(sources, repo_choice - 1, Source_t);
                SourceInputCondition_t input_condition = 
                {
                    .id = selected_repo_source->id,
                    .isNsfw = -1,
                    .lang = NULL,
                    .name = NULL
                };

                create_message(IPC_MSG_GET_MANGAS, &pagination, &input_condition, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                
                json_to_struct(J2S_MANGA, output, &mangas);
                printf("Manga Count: %d\n", mangas.itemCount);

                Manga_t *manga;
                FOREACH_ITEM(manga, mangas, Manga_t)
                {
                    printf("Manga %d: %s (id: %d)\n", i + 1, manga->title, manga->id);
                }
                break;
            }
            case IPC_MSG_OPEN_MANGA:
            {
                if(mangas.itemCount == 0)
                {
                    printf("No mangas available. Please fetch mangas first.\n");
                    break;
                }
                puts("Enter manga ID: ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
                int manga_id = atoi(input);
                if(manga_id < 1 || manga_id > mangas.itemCount)
                {
                    printf("Invalid manga ID: %s\n", input);
                    break;
                }

                const Manga_t *selected_manga = GET_ITEM(mangas, manga_id - 1, Manga_t);
                SourceInputCondition_t manga_input_condition = 
                {
                    .id = selected_manga->id,
                    .isNsfw = -1,
                    .lang = NULL,
                    .name = NULL
                };

                GraphQL_Pagination_t pagination = {
                    .first = 10,
                    .offset = 0
                };

                printf("Please enter the page number for manga chapters: ");
                if (fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
                int page = atoi(input);
                if(page < 1)                {
                    printf("Invalid page number: %s\n", input);
                    break;
                }
                pagination.first = 10;
                pagination.offset = (page - 1) * 10;
                create_message(IPC_MSG_OPEN_MANGA, &pagination, &manga_input_condition, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                json_to_struct(J2S_CHAPTER, output, &chapters);
                printf("Chapter Count: %d\n", chapters.itemCount);
                
                Chapter_t *chapter;
                FOREACH_ITEM(chapter, chapters, Chapter_t)
                {
                    printf("Chapter %d: %s (id: %d)\n", i + 1, chapter->name, chapter->id);
                }
                break;
            }
            default:
                printf("Invalid choice: %s\n", input);
                break;
        }
    }
}

int main(void) {
    int fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (fd < 0) {
        perror("socket");
        return 1;
    }

    if (ipc_connect(fd, SOCKET_PATH, 10) != 0) {
        perror("connect");
        close(fd);
        return 1;
    }
    process_input(fd);

    free_extension_list(&extensions);
    free_manga_list(&mangas);
    close(fd);
    return 0;
}

int ipc_send_message(int fd, const char *message)
{
    size_t len = strlen(message);
    ssize_t sent = send(fd, message, len, 0);
    if (sent < 0) {
        perror("send");
        return -1;
    }
    if ((size_t)sent != len) {
        fprintf(stderr, "Partial send: %zd of %zu bytes\n", sent, len);
        return -1;
    }

    return 0;
}

int ipc_recv_message(int fd, uint8_t *status, char* output, size_t output_size)
{
    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(fd, &read_fds);

    struct timeval timeout;
    timeout.tv_sec = 20; // 20 second timeout
    timeout.tv_usec = 0;

    int ready = select(fd + 1, &read_fds, NULL, NULL, &timeout);
    if (ready < 0) {
        perror("select");
        return -1;
    }

    if (ready == 0) {
        return -1;
    }

    char out[output_size];
    if (FD_ISSET(fd, &read_fds)) {
        ssize_t received = recv(fd, out, output_size, 0);
        *status = out[0];
        strncpy(output, out + 1, output_size - 1);
        output[output_size - 1] = '\0';
        if (received < 0) 
        {
            perror("recv");
            return -1;
        }
    }

    return 0;
}

int ipc_connect(int fd, const char *socket_path, int max_attempts)
{
    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    for (int attempt = 1; attempt <= max_attempts; attempt++) {
        if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) == 0) {
            return 0;
        }
        if (attempt < max_attempts) {
            printf("Attempt %d: Failed to connect, retrying...\n", attempt);
            sleep(1);
        }
    }

    return -1;
}