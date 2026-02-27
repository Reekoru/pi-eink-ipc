#include "message.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRUCT_BUFF_LEN 16
static ExtensionInput_t json_to_extension(cJSON *json);

static Manga_t json_to_manga(cJSON *json)
{
    Manga_t manga;

    cJSON *title = cJSON_GetObjectItem(json, "title");
    manga.title = (title && title->valuestring) ? strdup(title->valuestring) : NULL;

    cJSON *author = cJSON_GetObjectItem(json, "author");
    manga.author = (author && author->valuestring) ? strdup(author->valuestring) : NULL;

    cJSON *thumbnailPath = cJSON_GetObjectItem(json, "thumbnailPath");
    manga.thumbnailPath = (thumbnailPath && thumbnailPath->valuestring) ? strdup(thumbnailPath->valuestring) : NULL;

    cJSON *id = cJSON_GetObjectItem(json, "id");
    manga.id = id ? id->valueint : -1;

    return manga;
}

static Chapter_t json_to_chapter(cJSON *json)
{
    Chapter_t chapter;

    cJSON *chapterNumber = cJSON_GetObjectItem(json, "chapterNumber");
    chapter.chapterNumber = chapterNumber ? chapterNumber->valueint : -1;

    cJSON *id = cJSON_GetObjectItem(json, "id");
    chapter.id = id ? id->valueint : -1;

    cJSON *isBookmarked = cJSON_GetObjectItem(json, "isBookmarked");
    chapter.isBookmarked = isBookmarked ? isBookmarked->valueint : false;

    cJSON *isDownloaded = cJSON_GetObjectItem(json, "isDownloaded");
    chapter.isDownloaded = isDownloaded ? isDownloaded->valueint : false;

    cJSON *isRead = cJSON_GetObjectItem(json, "isRead");
    chapter.isRead = isRead ? isRead->valueint : false;

    cJSON *name = cJSON_GetObjectItem(json, "name");
    chapter.name = (name && name->valuestring) ? strdup(name->valuestring) : NULL;

    cJSON *pageCount = cJSON_GetObjectItem(json, "pageCount");
    chapter.pageCount = pageCount ? pageCount->valueint : -1;

    cJSON *lastPageRead = cJSON_GetObjectItem(json, "lastPageRead");
    chapter.lastPageRead = lastPageRead ? lastPageRead->valueint : -1;

    cJSON *lastReadAt = cJSON_GetObjectItem(json, "lastReadAt");
    chapter.lastReadAt = (lastReadAt && lastReadAt->valuestring) ? strdup(lastReadAt->valuestring) : NULL;

    return chapter;
}

void create_message(IPCMessage_t  message, GraphQL_Pagination_t *pagination, void *params, char *output, size_t output_size) {
    if (output == NULL) {
        perror("output buffer is NULL");
        return;
    }

    cJSON *m = cJSON_CreateObject();
    cJSON_AddNumberToObject(m, "id", message);

    if(params != NULL)
    {
        cJSON *params_json;
        switch(message)
        {
            case IPC_MSG_GET_EXTENSIONS:
                ExtensionInput_t *ext_params = (ExtensionInput_t *)params;
                params_json = ExtensionInput_toJSON(ext_params);
                cJSON_AddItemToObject(m, "params", params_json);

                break;

            case IPC_MSG_INSTALL_EXTENSION:
            case IPC_MSG_UNINSTALL_EXTENSION:
            {
                UpdateExtensionInput_t *update_params = (UpdateExtensionInput_t *)params;
                params_json = UpdateExtensionInput_toJSON(update_params);
                cJSON_AddItemToObject(m, "params", params_json);
                break;
            }
            case IPC_MSG_OPEN_EXTENSION_REPO:
            {
                SourceInputCondition_t *source_params = (SourceInputCondition_t *)params;
                params_json = SourceInputCondition_toJSON(source_params);
                cJSON_AddItemToObject(m, "params", params_json);
                break;
            }
            case IPC_MSG_OPEN_MANGA:
            {
                MangaInputCondition_t *manga_params = (MangaInputCondition_t *)params;
                params_json = MangaInputCondition_toJSON(manga_params);
                cJSON_AddItemToObject(m, "params", params_json);
                break;
            }
            default:
                cJSON_AddItemToObject(m, "params", cJSON_CreateObject());
                break;
        }

        if(pagination != NULL && params_json != NULL)
        {
            cJSON *pagination_json = Pagination_toJSON(pagination);
            cJSON_AddItemToObject(params_json, "pagination", pagination_json);
        }
    }
    else
    {
        cJSON_AddItemToObject(m, "params", cJSON_CreateObject());
    }

    bool ret = cJSON_PrintPreallocated(m, output, output_size, false);
    if (!ret) {
        perror("Failed to create message");
        output[0] = '\0';
    }

    size_t len = strlen(output);
    if (len < output_size - 1) {
        output[len] = '\n';
        output[len + 1] = '\0';
    }
    else
    {
        fprintf(stderr, "Output buffer too small for message\n");
    }
    cJSON_Delete(m);
}

void json_to_struct(JSONToStruct_t type, char* json_str, void* output_struct)
{
    switch(type)
    {
        case J2S_EXTENSION:
        {
            Extensions_t extensions = { 0, 0, NULL };
            cJSON *json = cJSON_Parse(json_str);
            if (json == NULL) {
                perror("Failed to parse JSON");
                return;
            }
            
            const cJSON *data = cJSON_GetObjectItem(json, "extensions");
            if (!cJSON_IsObject(data)) {
                perror("Failed to get extensions object from JSON");
                cJSON_Delete(json);
                return;
            }
            const cJSON *totalCount = cJSON_GetObjectItem(data, "totalCount");
            const cJSON *nodes = cJSON_GetObjectItem(data, "nodes");
            if(cJSON_IsNumber(totalCount) && totalCount->valueint != NULL)
            {
                printf("Total Count: %d\n", totalCount->valueint);
                extensions.totalCount = totalCount->valueint;
            }
            else
            {
                extensions.totalCount = 0;
                perror("Failed to get totalCount from JSON");
                cJSON_Delete(json);
                return;
            }
            if(cJSON_IsArray(nodes) && nodes->child != NULL)
            {
                ExtensionInput_t ext[STRUCT_BUFF_LEN];
                cJSON *item = NULL;
                int index = 0;
                cJSON_ArrayForEach(item, nodes) {
                    if (index >= STRUCT_BUFF_LEN) {
                        fprintf(stderr, "Exceeded struct buffer length\n");
                        break;
                    }
                    ext[index] = json_to_extension(item);
                    index++;
                }
                
                // Store the actual number of items we parsed
                extensions.itemCount = index;
                
                extensions.extensions = malloc(sizeof(ExtensionInput_t) * index);
                if (extensions.extensions != NULL) {
                    memcpy(extensions.extensions, ext, sizeof(ExtensionInput_t) * index);
                } else {
                    perror("Failed to allocate memory for extensions");
                }
            }

            memcpy(output_struct, &extensions, sizeof(Extensions_t));

            cJSON_Delete(json);
            break;
        }
        case J2S_SETTING:
            break;
        case J2S_SOURCE:
            break;
        case J2S_MANGA:
        {
            Mangas_t mangas = {0};
            cJSON *json = cJSON_Parse(json_str);

            cJSON *count = cJSON_GetObjectItem(json, "count");
            if(cJSON_IsNumber(count) && count->valueint != NULL)
            {
                mangas.itemCount = count->valueint;
            }
            else
            {
                mangas.itemCount = 0;
                mangas.mangas = NULL;
                perror("Failed to get manga count from JSON");
                cJSON_Delete(json);
                return;
            }

            cJSON *mangas_json = cJSON_GetObjectItem(json, "mangas");
            if(cJSON_IsArray(mangas_json) && mangas_json->child != NULL)
            {                
                Manga_t manga_array[mangas.itemCount];
                cJSON *item = NULL;
                int index = 0;
                cJSON_ArrayForEach(item, mangas_json) {
                    if (index >= STRUCT_BUFF_LEN) {
                        fprintf(stderr, "Exceeded struct buffer length\n");
                        break;
                    }
                    manga_array[index] = json_to_manga(item);
                    index++;
                }

                mangas.mangas = malloc(sizeof(Manga_t) * index);
                if (mangas.mangas != NULL) {
                    memcpy(mangas.mangas, manga_array, sizeof(Manga_t) * index);
                } else {
                    perror("Failed to allocate memory for mangas");
                }
            }

            memcpy(output_struct, &mangas, sizeof(Mangas_t));

            cJSON_Delete(json);
            break;
        }
        case J2S_CHAPTER:
        {
            GraphQL_ChapterList_t chapters = {0};
            cJSON *json = cJSON_Parse(json_str);

            cJSON *count = cJSON_GetObjectItem(json, "count");
            if(cJSON_IsNumber(count) && count->valueint != NULL)
            {
                chapters.itemCount = count->valueint;
            }
            else
            {
                chapters.itemCount = 0;
                chapters.items = NULL;
                perror("Failed to get chapter count from JSON");
                cJSON_Delete(json);
                return;
            }

            cJSON *chapters_json = cJSON_GetObjectItem(json, "chapters");
            if(cJSON_IsArray(chapters_json) && chapters_json->child != NULL)
            {                
                Chapter_t chapter_array[chapters.itemCount];
                cJSON *item = NULL;
                int index = 0;
                cJSON_ArrayForEach(item, chapters_json) {
                    if (index >= STRUCT_BUFF_LEN) {
                        fprintf(stderr, "Exceeded struct buffer length\n");
                        break;
                    }
                    chapter_array[index] = json_to_chapter(item);
                    index++;
                }

                chapters.items = malloc(sizeof(Chapter_t) * index);
                if (chapters.items != NULL) {
                    memcpy(chapters.items, chapter_array, sizeof(Chapter_t) * index);
                } else {
                    perror("Failed to allocate memory for chapters");
                }
            }

            memcpy(output_struct, &chapters, sizeof(GraphQL_ChapterList_t));

            cJSON_Delete(json);
            break;
        }
    }
        
}

static ExtensionInput_t json_to_extension(cJSON *json)
{
    ExtensionInput_t ext;

    cJSON *apkName = cJSON_GetObjectItem(json, "apkName");
    ext.apkName = (apkName && apkName->valuestring) ? strdup(apkName->valuestring) : NULL;

    cJSON *hasUpdate = cJSON_GetObjectItem(json, "hasUpdate");
    ext.hasUpdate = hasUpdate ? hasUpdate->valueint : -1;

    cJSON *iconUrl = cJSON_GetObjectItem(json, "iconUrl");
    ext.iconUrl = (iconUrl && iconUrl->valuestring) ? strdup(iconUrl->valuestring) : NULL;

    cJSON *isInstalled = cJSON_GetObjectItem(json, "isInstalled");
    ext.isInstalled = isInstalled ? isInstalled->valueint : -1;

    cJSON *isNsfw = cJSON_GetObjectItem(json, "isNsfw");
    ext.isNsfw = isNsfw ? isNsfw->valueint : -1;

    cJSON *isObsolete = cJSON_GetObjectItem(json, "isObsolete");
    ext.isObsolete = isObsolete ? isObsolete->valueint : -1;

    cJSON *lang = cJSON_GetObjectItem(json, "lang");
    ext.lang = (lang && lang->valuestring) ? strdup(lang->valuestring) : NULL;

    cJSON *name = cJSON_GetObjectItem(json, "name");
    ext.name = (name && name->valuestring) ? strdup(name->valuestring) : NULL;

    cJSON *pkgName = cJSON_GetObjectItem(json, "pkgName");
    ext.pkgName = (pkgName && pkgName->valuestring) ? strdup(pkgName->valuestring) : NULL;

    cJSON *repo = cJSON_GetObjectItem(json, "repo");
    ext.repo = (repo && repo->valuestring) ? strdup(repo->valuestring) : NULL;

    cJSON *versionCode = cJSON_GetObjectItem(json, "versionCode");
    ext.versionCode = versionCode ? versionCode->valueint : -1;

    cJSON *versionName = cJSON_GetObjectItem(json, "versionName");
    ext.versionName = (versionName && versionName->valuestring) ? strdup(versionName->valuestring) : NULL;

    return ext;
}

void free_extensions(Extensions_t *extensions)
{
    if (extensions == NULL || extensions->extensions == NULL) {
        return;
    }

    for (int i = 0; i < extensions->itemCount; i++) {
        ExtensionInput_t *ext = &extensions->extensions[i];
        free(ext->apkName);
        free(ext->iconUrl);
        free(ext->lang);
        free(ext->name);
        free(ext->pkgName);
        free(ext->repo);
        free(ext->versionName);
    }

    free(extensions->extensions);
    extensions->extensions = NULL;
    extensions->totalCount = 0;
    extensions->itemCount = 0;
}

void free_mangas(Mangas_t *mangas)
{
    if (mangas == NULL || mangas->mangas == NULL) {
        return;
    }

    for (int i = 0; i < mangas->itemCount; i++) {
        Manga_t *manga = &mangas->mangas[i];
        free(manga->title);
        free(manga->author);
        free(manga->thumbnailPath);
    }

    free(mangas->mangas);
    mangas->mangas = NULL;
    mangas->itemCount = 0;
}