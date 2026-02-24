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
    int id;
    int8_t isNsfw;
    char *lang;
    char *name;
} SourceInputCondition_t;



typedef struct
{
    int totalCount;
    int itemCount;
    ExtensionInput_t *extensions;
} Extensions_t;


cJSON *ExtensionInput_toJSON(ExtensionInput_t *input);
cJSON *Pagination_toJSON(GraphQL_Pagination_t *input);
cJSON *UpdateExtensionInput_toJSON(UpdateExtensionInput_t *input);
cJSON *SourceInputCondition_toJSON(SourceInputCondition_t *input);

#endif // JSON_TYPES_H