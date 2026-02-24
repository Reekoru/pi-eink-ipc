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
Extensions_t extensions;
uint8_t status = 0;

static void print_messages()
{
    puts("Enter a message to send (or 'exit' to quit): ");
    puts("Available messages:");
    puts("1. ping");
    puts("2. get_settings");
    puts("3. get_extensions");
    puts("4. install_extension");
    puts("5. uninstall_extension");
    puts("6. open_extension_repo");
}

static void process_message(int fd)
{
    if (ipc_recv_message(fd, &status, output, BUF_SIZE) != 0) {
        printf("Timed out waiting for response\n");
    }
}

static void clear_terminal()
{
    printf("\033[H\033[J");
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

                FOREACH_EXTENSION(extensions, ext)
                {
                    printf("Extension %d: %s (isInstalled: %d)\n", i + 1, ext->pkgName, ext->isInstalled);
                }

                break;
            }
            case IPC_MSG_INSTALL_EXTENSION:
            case IPC_MSG_UNINSTALL_EXTENSION:
                
                puts("Please select an extension:");
                puts("");
                FOREACH_EXTENSION(extensions, ext)
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
                const ExtensionInput_t *selected_ext = &extensions.extensions[ext_choice - 1];
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

            case IPC_MSG_OPEN_EXTENSION_REPO:
            {
                puts("Please select an extension:");
                puts("");

                ExtensionInput_t params = {
                    .apkName = NULL,
                    .hasUpdate = -1,
                    .iconUrl = NULL,
                    .isInstalled = true,
                    .isNsfw = -1,
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
                FOREACH_EXTENSION(extensions, ext)
                {
                    printf("Extension %d: %s (isInstalled: %d)\n", i + 1, ext->pkgName, ext->isInstalled);
                }

                if(fgets(input, sizeof(input), stdin) == NULL) {
                    break;
                }
                int repo_choice = atoi(input);
                if(repo_choice < 1 || repo_choice > extensions.itemCount)
                {
                    printf("Invalid extension choice: %s\n", input);
                    break;
                }

                const ExtensionInput_t *selected_repo_ext = &extensions.extensions[repo_choice - 1];
                SourceInputCondition_t input_condition = 
                {
                    .id = -1,
                    .isNsfw = -1,
                    .lang = NULL,
                    .name = selected_repo_ext->name
                };

                create_message(IPC_MSG_OPEN_EXTENSION_REPO, NULL, &input_condition, input, BUF_SIZE);
                ipc_send_message(fd, input);
                process_message(fd);
                print_message();

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
    while (1) {
        process_input(fd);
    }

    free_extensions(&extensions);
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
    timeout.tv_sec = 5;
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