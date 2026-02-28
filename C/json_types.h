#ifndef JSON_TYPES_H
#define JSON_TYPES_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include "utils/cJSON.h"

typedef enum
{
    JSON_OK,
    JSON_ERR
} JSON_Status;

typedef struct
{
    int first;
    int offset;
} GraphQL_Pagination_t;

typedef struct 
{
    char *apkName;
    int8_t hasUpdate;
    char *iconUrl;
    int8_t isInstalled;
    int8_t isNsfw;
    int8_t isObsolete;
    char *lang;
    char *name;
    char *pkgName;
    char *repo;
    int versionCode;
    char *versionName;
} ExtensionInput_t;

typedef struct
{
    int8_t install;
    int8_t uninstall;
    int8_t update;
} Patch_t;


typedef struct 
{
    char *id;
    Patch_t patch;
}UpdateExtensionInput_t;

typedef struct
{
    char *id;
    int8_t isNsfw;
    char *lang;
    char *name;
} SourceInputCondition_t;

typedef struct
{
    char *displayName;
    char *iconUrl;
    char *id;
    bool isNsfw;
    char *lang;
    char *name;
} Source_t;


typedef struct
{
    int mangaId;
} MangaInputCondition_t;


typedef struct
{
    int totalCount;
    int itemCount;
    ExtensionInput_t *extensions;
} Extensions_t;

typedef struct
{
    char *title;
    char *author;
    char *thumbnailPath;
    int id;
} Manga_t;

typedef struct 
{
    int totalCount;
    int itemCount;
    Manga_t *mangas;
} Mangas_t;

typedef struct
{
    int chapterNumber;
    int id;
    bool isBookmarked;
    bool isDownloaded;
    bool isRead;
    char *name;
    int pageCount;
    int lastPageRead;
    char *lastReadAt;
} Chapter_t;

typedef struct
{
    int totalCount;
    int itemCount;
    Chapter_t *items;
} Chapters_t;

typedef struct GraphQL_List_t
{
    int itemCount;
    int totalCount;
    void *items;
} GraphQL_List_t;

typedef struct GraphQL_List_t GraphQL_ExtensionList_t;
typedef struct GraphQL_List_t GraphQL_MangaList_t;
typedef struct GraphQL_List_t GraphQL_ChapterList_t;
typedef struct GraphQL_List_t GraphQL_SourceList_t;


cJSON *ExtensionInput_toJSON(ExtensionInput_t *input);
cJSON *Pagination_toJSON(GraphQL_Pagination_t *input);
cJSON *UpdateExtensionInput_toJSON(UpdateExtensionInput_t *input);
cJSON *SourceInputCondition_toJSON(SourceInputCondition_t *input);
cJSON *MangaInputCondition_toJSON(MangaInputCondition_t *input);

#endif // JSON_TYPES_H