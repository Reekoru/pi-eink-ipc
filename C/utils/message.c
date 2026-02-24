#include "message.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STRUCT_BUFF_LEN 16
static ExtensionInput_t json_to_extension(cJSON *json);

void create_message(IPCMessage_t  message, GraphQL_Pagination_t *pagination, void *params, char *output, size_t output_size) {
    if (output == NULL) {
        perror("output buffer is NULL");
        return;
    }

    cJSON *m = cJSON_CreateObject();
    cJSON_AddNumberToObject(m, "id", message);

    if(params != NULL)
    {
        switch(message)
        {
            case IPC_MSG_GET_EXTENSIONS:
                ExtensionInput_t *ext_params = (ExtensionInput_t *)params;
                cJSON *params_json = ExtensionInput_toJSON(ext_params);
                cJSON_AddItemToObject(m, "params", params_json);
            
                if(pagination != NULL)
                {
                    cJSON *pagination_json = Pagination_toJSON(pagination);
                    cJSON_AddItemToObject(params_json, "pagination", pagination_json);
                }

                break;

            case IPC_MSG_INSTALL_EXTENSION:
            case IPC_MSG_UNINSTALL_EXTENSION:
            {
                UpdateExtensionInput_t *update_params = (UpdateExtensionInput_t *)params;
                cJSON *update_json = UpdateExtensionInput_toJSON(update_params);
                cJSON_AddItemToObject(m, "params", update_json);
                break;
            }
            case IPC_MSG_OPEN_EXTENSION_REPO:
            {
                SourceInputCondition_t *source_params = (SourceInputCondition_t *)params;
                cJSON *source_json = SourceInputCondition_toJSON(source_params);
                cJSON_AddItemToObject(m, "params", source_json);
                break;
            }
        }
    }
    else
    {
        cJSON_AddObjectToObject(m, "params");
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
        case J2S_SETTING:
            break;
        case J2S_SOURCE:
            break;
        case J2S_MANGA:
            break;
        case J2S_CHAPTER:
            break;
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