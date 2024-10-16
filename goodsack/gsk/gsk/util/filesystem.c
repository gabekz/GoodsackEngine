/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "filesystem.h"

#include <string.h>

#include <GoodsackEngineConfig.h> // We need build info

#include "util/logger.h"
#include "util/sysdefs.h"

#if defined(SYS_ENV_WIN)
#include <windows.h>
#endif // SYS_ENV_WIN

static struct
{
    char gsk_root[GSK_FS_MAX_PATH];
    char proj_root[GSK_FS_MAX_PATH];
    char proj_scheme[GSK_FS_MAX_SCHEME];
} s_path_roots;

static void
_to_forward_slash(char *buffer)
{
    int index = 0;
    while (buffer[index] && index < GSK_FS_MAX_PATH)
    {
        if (buffer[index] == '\\') buffer[index] = '/';
        index++;
    }
}

static void
_strip_filename(char *buffer)
{
    char *pos = strrchr(buffer, '/');
    if (pos != NULL) { *pos = '\0'; }
}

static void
_get_absolute_path(char *buffer)
{

    /*------------------------------------------------------
      Some directory check testing stuff                  */

#if 0
#if defined(SYS_ENV_WIN)

#if 0
    GetModuleFileName(NULL, buffer, GSK_FS_MAX_PATH);
#elif 0
    GetCurrentDirectory(GSK_FS_MAX_PATH, buffer);
#endif
    char b[GSK_FS_MAX_PATH] = (_GOODSACK_FS_DIR_DATA "/");
    // strcat(b, "/res");
    strcpy(buffer, s_path_roots.gsk_root);
    _to_forward_slash(buffer);
    return;
#endif // SYS_ENV_WIN
#endif // 0
    /*-----------------------------------------------------*/

    char b[GSK_FS_MAX_PATH] = (_GOODSACK_FS_DIR_DATA "/");
    // strcat(b, "/res");
    strcpy(buffer, s_path_roots.gsk_root);
    _to_forward_slash(buffer);
    return;

#if 0 // TODO: no need for this right now
    _strip_filename(buffer);
#endif
}

char *
gsk_filesystem_get_extension(const char *path)
{
    char *ext = strrchr(path, '.');
    return ext;
}

void
gsk_filesystem_initialize(const char *project_root, const char *project_scheme)
{
    if (project_root == NULL) { LOG_ERROR("Failed to initialize filesystem"); }

    strcpy(s_path_roots.gsk_root, (_GOODSACK_FS_DIR_DATA "/"));
    strcpy(s_path_roots.proj_root, project_root);
    strcpy(s_path_roots.proj_scheme, project_scheme);
}

gsk_URI
gsk_filesystem_uri(const char *uri_path)
{
    const int nparams = 3; // number of parameters that are to be assigned
    gsk_URI ret;

    int result = sscanf(
      uri_path, "%99[^:]%99[^/]//%99[^\n]", ret.scheme, ret.macro, ret.path);
    if (result != nparams)
    {
        // LOG_WARN("Failed to parse URI: %s", uri_path);
        return (gsk_URI) {.scheme = '\0', .macro = '\0', .path = '\0'};
    }

    return ret;
}

gsk_Path
gsk_filesystem_path_from_uri(const char *uri_path)
{
    gsk_Path ret = (gsk_Path) {.uri = gsk_filesystem_uri(uri_path)};
    if (ret.uri.scheme[0] == '\0')
    {
        // LOG_WARN("Failed to parse URI: %s", uri_path);
        return (gsk_Path) {.path = '\0'};
    }

    // build path from URI

    char absolute_path[GSK_FS_MAX_PATH] = "";

    // Project-specific data directory
    if (!strcmp(ret.uri.scheme, s_path_roots.proj_scheme))
    {
        strcat(absolute_path, s_path_roots.proj_root);
    }
    // Engine-specific data directory
    else if (!strcmp(ret.uri.scheme, "gsk"))
    {

        char buff[256] = "";
        _get_absolute_path(buff);
        strcat(absolute_path, buff);
    }

    strcat(absolute_path, ret.uri.path);
    strcpy(ret.path, absolute_path);
    return ret;
}
