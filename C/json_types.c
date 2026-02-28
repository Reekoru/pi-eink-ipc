#include "json_types.h"

cJSON *ExtensionInput_toJSON(ExtensionInput_t *input)
{
    cJSON *json = cJSON_CreateObject();

    if(json == NULL) {
        perror("Failed to create JSON object");
        return NULL;
    }

    if(input == NULL) {
        cJSON_Delete(json);
        perror("Input is NULL");
        return NULL;
    }

    if(input->apkName != NULL) cJSON_AddStringToObject(json, "apkName", input->apkName);
    if(input->hasUpdate > -1) cJSON_AddBoolToObject(json, "hasUpdate", input->hasUpdate);
    if(input->iconUrl != NULL) cJSON_AddStringToObject(json, "iconUrl", input->iconUrl);
    if(input->isInstalled > -1) cJSON_AddBoolToObject(json, "isInstalled", input->isInstalled);
    if(input->isNsfw > -1) cJSON_AddBoolToObject(json, "isNsfw", input->isNsfw);
    if(input->isObsolete > -1) cJSON_AddBoolToObject(json, "isObsolete", input->isObsolete);
    if(input->lang != NULL)cJSON_AddStringToObject(json, "lang", input->lang);
    if(input->name != NULL)cJSON_AddStringToObject(json, "name", input->name);
    if(input->pkgName != NULL)cJSON_AddStringToObject(json, "pkgName", input->pkgName);
    if(input->repo != NULL)cJSON_AddStringToObject(json, "repo", input->repo);
    if(input->versionCode > -1)cJSON_AddNumberToObject(json, "versionCode", input->versionCode);
    if(input->versionName != NULL)cJSON_AddStringToObject(json, "versionName", input->versionName);

    return json;
}

cJSON *UpdateExtensionInput_toJSON(UpdateExtensionInput_t *input)
{
    cJSON *json = cJSON_CreateObject();

    if(json == NULL) {
        perror("Failed to create JSON object");
        return NULL;
    }

    if(input == NULL) {
        cJSON_Delete(json);
        perror("Input is NULL");
        return NULL;
    }

    if(input->id == NULL)
    {
        cJSON_Delete(json);
        perror("Input id is NULL");
        return NULL;
    }

    cJSON_AddStringToObject(json, "id", input->id);
    cJSON *patch_json = cJSON_CreateObject();
    if(input->patch.install > -1) cJSON_AddNumberToObject(patch_json, "install", input->patch.install);
    if(input->patch.uninstall > -1) cJSON_AddNumberToObject(patch_json, "uninstall", input->patch.uninstall);
    if(input->patch.update > -1) cJSON_AddNumberToObject(patch_json, "update", input->patch.update);
    cJSON_AddItemToObject(json, "patch", patch_json);

    return json;
}

cJSON *Pagination_toJSON(GraphQL_Pagination_t *input)
{
    cJSON *json = cJSON_CreateObject();

    if(json == NULL) {
        perror("Failed to create JSON object");
        return NULL;
    }

    if(input == NULL) {
        cJSON_Delete(json);
        perror("Input is NULL");
        return NULL;
    }

    cJSON_AddNumberToObject(json, "first", input->first);
    cJSON_AddNumberToObject(json, "offset", input->offset);

    return json;
}

cJSON *SourceInputCondition_toJSON(SourceInputCondition_t *input)
{
    cJSON *json = cJSON_CreateObject();

    if(json == NULL) {
        perror("Failed to create JSON object");
        return NULL;
    }

    if(input == NULL) {
        cJSON_Delete(json);
        perror("Input is NULL");
        return NULL;
    }

    if(input->id != NULL) cJSON_AddStringToObject(json, "id", input->id);
    if(input->isNsfw > -1) cJSON_AddNumberToObject(json, "isNsfw", input->isNsfw);
    if(input->lang != NULL)cJSON_AddStringToObject(json, "lang", input->lang);
    if(input->name != NULL)cJSON_AddStringToObject(json, "name", input->name);

    return json;
}

cJSON *MangaInputCondition_toJSON(MangaInputCondition_t *input)
{
    cJSON *json = cJSON_CreateObject();

    if(json == NULL) {
        perror("Failed to create JSON object");
        return NULL;
    }

    if(input == NULL) {
        cJSON_Delete(json);
        perror("Input is NULL");
        return NULL;
    }

    if(input->mangaId > -1) cJSON_AddNumberToObject(json, "mangaId", input->mangaId);

    return json;
}