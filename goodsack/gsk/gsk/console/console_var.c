/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "console_var.h"

#include "util/logger.h"
#include "util/sysdefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define GSK_CONSOLE_MAX_VARS 128

typedef union GskConsoleVarValue {
    s32 i;
    f32 f;
    u8 b;
    char *s;
} GskConsoleVarValue;

typedef struct gsk_ConsoleVarDef
{
    const char *name;
    char *description;
    GskConsoleVarType type;

    GskConsoleVarValue value;
    GskConsoleVarValue default_value;

    u8 has_min;
    u8 has_max;
    GskConsoleVarValue min_value;
    GskConsoleVarValue max_value;

} gsk_ConsoleVarDef;

static gsk_ConsoleVarDef s_vars[GSK_CONSOLE_MAX_VARS];
static u32 s_vars_count = 0;

/*--------------------------------------------------------------------*/
static s32
_find_var_index(const char *var_name);
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static char *
_strdup_internal(const char *s)
{
    if (s == NULL) { return NULL; }

    size_t len = strlen(s);
    char *ret  = malloc(len + 1);
    if (ret == NULL)
    {
        LOG_ERROR("malloc failed");
        return NULL;
    }

    memcpy(ret, s, len + 1);
    return ret;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_console_var_register(const char *var_name)
{
#if 0
    if (var_name == NULL)
    {
        LOG_ERROR("failed to register convar: invalid name");
        return;
    }

    if (s_vars_count >= GSK_CONSOLE_MAX_VARS)
    {
        LOG_ERROR("convar registry full");
        return;
    }

    if (_find_var_index(cmd_name) >= 0) { return; }
#endif
}
/*--------------------------------------------------------------------*/

static void
_print_var_value(const gsk_ConsoleVarDef *var)
{
    if (var == NULL) { return; }

    switch (var->type)
    {
    case GskConsoleVarType_Integer:
        LOG_INFO("%s = %d (default %d)",
                 var->name,
                 var->value.i,
                 var->default_value.i);
        break;

    case GskConsoleVarType_Float:
        LOG_INFO("%s = %f (default %f)",
                 var->name,
                 var->value.f,
                 var->default_value.f);
        break;

    case GskConsoleVarType_String:
        LOG_INFO("%s = \"%s\" (default \"%s\")",
                 var->name,
                 var->value.s ? var->value.s : "",
                 var->default_value.s ? var->default_value.s : "");
        break;

    case GskConsoleVarType_Bool:
        LOG_INFO("%s = %s (default %s)",
                 var->name,
                 var->value.b ? "true" : "false",
                 var->default_value.b ? "true" : "false");
        break;

    default: LOG_ERROR("unknown convar type for %s", var->name); break;
    }
}

static u8
_parse_bool(const char *s, u8 *out_value)
{
    if (s == NULL || out_value == NULL) { return FALSE; }

    if (strcmp(s, "1") == 0 || strcmp(s, "true") == 0 || strcmp(s, "TRUE") == 0)
    {
        *out_value = TRUE;
        return TRUE;
    }

    if (strcmp(s, "0") == 0 || strcmp(s, "false") == 0 ||
        strcmp(s, "FALSE") == 0)
    {
        *out_value = FALSE;
        return TRUE;
    }

    return FALSE;
}

/*--------------------------------------------------------------------*/
static u8
_console_var_check_name(const char *name)
{
    if (name == NULL)
    {
        LOG_ERROR("failed to register convar: invalid name");
        return FALSE;
    }

    if (s_vars_count >= GSK_CONSOLE_MAX_VARS)
    {
        LOG_ERROR("convar registry full");
        return FALSE;
    }

    if (_find_var_index(name) >= 0)
    {
        LOG_ERROR("duplicate convar registration: %s", name);
        return FALSE;
    }

    return TRUE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_console_var_register_int(const char *name,
                             const char *description,
                             s32 default_value,
                             s32 min_value,
                             s32 max_value)
{

    if (!_console_var_check_name(name)) { return; }

    gsk_ConsoleVarDef *var = &s_vars[s_vars_count++];
    memset(var, 0, sizeof(*var));

    var->name        = _strdup_internal(name);
    var->description = _strdup_internal(description ? description : "");
    var->type        = GskConsoleVarType_Integer;

    var->default_value.i = default_value;
    var->value.i         = default_value;

    var->has_min     = TRUE;
    var->has_max     = TRUE;
    var->min_value.i = min_value;
    var->max_value.i = max_value;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_console_var_register_float(const char *name,
                               const char *description,
                               f32 default_value,
                               f32 min_value,
                               f32 max_value)
{

    if (!_console_var_check_name(name)) { return; }

    gsk_ConsoleVarDef *var = &s_vars[s_vars_count++];
    memset(var, 0, sizeof(*var));

    var->name        = _strdup_internal(name);
    var->description = _strdup_internal(description ? description : "");
    var->type        = GskConsoleVarType_Float;

    var->default_value.f = default_value;
    var->value.f         = default_value;

    var->has_min     = TRUE;
    var->has_max     = TRUE;
    var->min_value.f = min_value;
    var->max_value.f = max_value;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
void
gsk_console_var_register_string(const char *name,
                                const char *description,
                                const char *default_value)
{
    if (!_console_var_check_name(name)) { return; }

    gsk_ConsoleVarDef *var = &s_vars[s_vars_count++];
    memset(var, 0, sizeof(*var));

    var->name        = _strdup_internal(name);
    var->description = _strdup_internal(description ? description : "");
    var->type        = GskConsoleVarType_String;

    var->default_value.s = _strdup_internal(default_value ? default_value : "");
    var->value.s         = _strdup_internal(default_value ? default_value : "");
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
u8
gsk_console_var_set_from_string(const char *name, const char *value_str)
{
    s32 idx = _find_var_index(name);
    if (idx < 0)
    {
        LOG_ERROR("unknown convar: %s", name ? name : "(null)");
        return FALSE;
    }

    if (value_str == NULL)
    {
        LOG_ERROR("null value for convar: %s", name);
        return FALSE;
    }

    gsk_ConsoleVarDef *var = &s_vars[idx];

    switch (var->type)
    {
    case GskConsoleVarType_Integer: {
        char *end = NULL;
        long v    = strtol(value_str, &end, 10);
        if (*value_str == '\0' || *end != '\0')
        {
            LOG_ERROR("%s: invalid integer '%s'", name, value_str);
            return FALSE;
        }

        if (var->has_min && v < var->min_value.i) { v = var->min_value.i; }
        if (var->has_max && v > var->max_value.i) { v = var->max_value.i; }

        var->value.i = (s32)v;
        return TRUE;
    }

    case GskConsoleVarType_Float: {
        char *end = NULL;
        float v   = strtof(value_str, &end);
        if (*value_str == '\0' || *end != '\0')
        {
            LOG_ERROR("%s: invalid float '%s'", name, value_str);
            return FALSE;
        }

        if (var->has_min && v < var->min_value.f) { v = var->min_value.f; }
        if (var->has_max && v > var->max_value.f) { v = var->max_value.f; }

        var->value.f = v;
        return TRUE;
    }

    case GskConsoleVarType_Bool: {
        u8 v = FALSE;
        if (!_parse_bool(value_str, &v))
        {
            LOG_ERROR("%s: invalid bool '%s'", name, value_str);
            return FALSE;
        }

        var->value.b = v;
        return TRUE;
    }

    case GskConsoleVarType_String: {
        char *copy = _strdup_internal(value_str);
        if (copy == NULL) { return FALSE; }

        free(var->value.s);
        var->value.s = copy;
        return TRUE;
    }

    default: break;
    }

    return FALSE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
s32
gsk_console_var_get_int(const char *name, s32 fallback)
{
    s32 idx = _find_var_index(name);
    if (idx < 0) { return fallback; }
    if (s_vars[idx].type != GskConsoleVarType_Integer) { return fallback; }
    return s_vars[idx].value.i;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
f32
gsk_console_var_get_float(const char *name, f32 fallback)
{
    s32 idx = _find_var_index(name);
    if (idx < 0) { return fallback; }
    if (s_vars[idx].type != GskConsoleVarType_Float) { return fallback; }
    return s_vars[idx].value.f;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
const char *
gsk_console_var_get_string(const char *name, const char *fallback)
{
    s32 idx = _find_var_index(name);
    if (idx < 0) { return fallback; }
    if (s_vars[idx].type != GskConsoleVarType_String) { return fallback; }
    return s_vars[idx].value.s ? s_vars[idx].value.s : fallback;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
u8
gsk_console_var_print(const char *name)
{
    s32 idx = _find_var_index(name);
    if (idx < 0)
    {
        LOG_ERROR("unknown convar: %s", name ? name : "(null)");
        return FALSE;
    }

    _print_var_value(&s_vars[idx]);
    return TRUE;
}
/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/

/*--------------------------------------------------------------------*/
static s32
_find_var_index(const char *var_name)
{
    if (var_name == NULL) { return -1; }

    for (u32 i = 0; i < s_vars_count; i++)
    {
        if (strcmp(s_vars[i].name, var_name) == 0) { return (s32)i; }
    }

    return -1;
}
/*--------------------------------------------------------------------*/