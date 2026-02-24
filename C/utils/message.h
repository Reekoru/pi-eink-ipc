#ifndef MESSAGE_H
#define MESSAGE_H

#include "cJSON.h"
#include "../json_types.h"
#include <stddef.h>

#define FOREACH_EXTENSION(extensions, ext) \
    for (int i = 0; i < extensions.itemCount && (ext = &extensions.extensions[i]) != NULL; i++)

typedef enum
{
    IPC_MSG_PING,
    IPC_MSG_GET_SETTINGS,
    IPC_MSG_GET_EXTENSIONS,
    IPC_MSG_INSTALL_EXTENSION,
    IPC_MSG_UNINSTALL_EXTENSION,
    IPC_MSG_OPEN_EXTENSION_REPO,
    IPC_MSG_END
} IPCMessage_t;

typedef enum
{
    J2S_EXTENSION,
    J2S_SETTING,
    J2S_SOURCE,
    J2S_MANGA,
    J2S_CHAPTER
} JSONToStruct_t;

void create_message(IPCMessage_t  message, GraphQL_Pagination_t *pagination, void *params, char *output, size_t output_size);
void json_to_struct(JSONToStruct_t type, char* json_str, void* output_struct);
void free_extensions(Extensions_t *extensions);
#endif // MESSAGE_H