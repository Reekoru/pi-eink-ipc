#ifndef MESSAGE_H
#define MESSAGE_H

#include "cJSON.h"
#include "../json_types.h"
#include <stddef.h>

#define FOREACH_EXTENSION(extensions, ext) \
    for (int i = 0; i < extensions.itemCount && (ext = &extensions.extensions[i]) != NULL; i++)

#define FOREACH_MANGA(mangas, manga) \
    for (int i = 0; i < mangas.itemCount && (manga = &mangas.mangas[i]) != NULL; i++)

#define FOREACH_NODE(item, structure) \
    for (int i = 0; i < structure.itemCount && (item = &structure.items[i]) != NULL; i++)

#define FOREACH_ITEM(item, list, Type) \
    for (int i = 0; i < (list).itemCount && (((item) = &((Type *)(list).items)[i]), 1); i++)

#define GET_ITEM(list, index, Type) \
    (((index) >= 0 && (index) < (list).itemCount) ? &((Type *)(list).items)[(index)] : NULL)

typedef enum
{
    IPC_MSG_PING,
    IPC_MSG_GET_SETTINGS,
    IPC_MSG_GET_EXTENSIONS,
    IPC_MSG_INSTALL_EXTENSION,
    IPC_MSG_UNINSTALL_EXTENSION,
    IPC_MSG_GET_SOURCES,
    IPC_MSG_GET_MANGAS,
    IPC_MSG_OPEN_MANGA,
    IPC_MSG_END
} IPCMessage_t;
    
// Lookup table for IPCMessage_t to string
static const char* IPCMessageStrings[] = {
    [IPC_MSG_PING] = "PING",
    [IPC_MSG_GET_SETTINGS] = "GET_SETTINGS",
    [IPC_MSG_GET_EXTENSIONS] = "GET_EXTENSIONS",
    [IPC_MSG_INSTALL_EXTENSION] = "INSTALL_EXTENSION",
    [IPC_MSG_UNINSTALL_EXTENSION] = "UNINSTALL_EXTENSION",
    [IPC_MSG_GET_SOURCES] = "GET_SOURCES",
    [IPC_MSG_GET_MANGAS] = "GET_MANGAS",
    [IPC_MSG_OPEN_MANGA] = "OPEN_MANGA",
    [IPC_MSG_END] = "END"
};

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
void free_mangas(Mangas_t *mangas);
#endif // MESSAGE_H