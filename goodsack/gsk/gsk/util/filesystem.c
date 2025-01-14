/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "filesystem.h"

#include <string.h>

#include <GoodsackEngineConfig.h> // We need build info

#include "util/logger.h"
#include "util/sysdefs.h"

#if defined(SYS_ENV_WIN)
#include <windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#endif // SYS_ENV_WIN

static struct
{
    char gsk_root[GSK_FS_MAX_PATH];
    char gsk_scheme[GSK_FS_MAX_SCHEME];
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

char *
gsk_filesystem_get_extension(const char *path)
{
    char *ext = strrchr(path, '.');
    return ext;
}

void
gsk_filesystem_initialize(const char *project_root, const char *project_scheme)
{
    if (project_root == NULL)
    {
        LOG_CRITICAL("Failed to initialize filesystem - missing project_root.");
    }

    strcpy(s_path_roots.gsk_root, (_GOODSACK_FS_DIR_DATA "/"));
    strcpy(s_path_roots.gsk_scheme, GSK_FS_GSK_SCHEME);

    strcpy(s_path_roots.proj_root, project_root);
    strcat(s_path_roots.proj_root, "/");

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
        LOG_WARN("Failed to parse URI: %s", uri_path);
        return (gsk_URI) {.scheme = '\0', .macro = '\0', .path = '\0'};
    }

    return ret;
}

gsk_Path
gsk_filesystem_uri_to_path(const char *uri_path)
{
    gsk_Path ret = (gsk_Path) {.uri = gsk_filesystem_uri(uri_path)};
    if (ret.uri.scheme[0] == '\0')
    {
        LOG_ERROR("Failed to parse URI: %s", uri_path);
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
    else if (!strcmp(ret.uri.scheme, s_path_roots.gsk_scheme))
    {
        strcpy(absolute_path, s_path_roots.gsk_root);
        _to_forward_slash(absolute_path);
    }

    strcat(absolute_path, ret.uri.path);
    strcpy(ret.path, absolute_path);
    return ret;
}

char *
gsk_filesystem_path_to_uri(const char *file_path, char *output_uri)
{
    void *p_root   = NULL;
    void *p_scheme = NULL;

    // engine-specific path
    if (strncmp(
          file_path, s_path_roots.gsk_root, strlen(s_path_roots.gsk_root)) == 0)
    {
        p_root   = s_path_roots.gsk_root;
        p_scheme = s_path_roots.gsk_scheme;

    }
    // project-specific path
    else if (strncmp(file_path,
                     s_path_roots.proj_root,
                     strlen(s_path_roots.proj_root)) == 0)
    {
        p_root   = s_path_roots.proj_root;
        p_scheme = s_path_roots.proj_scheme;
    }
    // unknown path
    else
    {
        LOG_ERROR("Failed to get uri for path: %s", file_path);
        return NULL;
    }

    // It's a project-specific path
    strcpy(output_uri, (char *)p_scheme);
    strcat(output_uri, "://");
    strcat(output_uri, file_path + strlen((char *)p_root));

    _to_forward_slash(output_uri);
    return output_uri;
}

#if defined(SYS_ENV_WIN)

// Windows-specific directory traversal
void
gsk_filesystem_traverse(const char *dirpath, FileUriHandler process_fn)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    char search_path[1024];

    snprintf(search_path, sizeof(search_path), "%s\\*", dirpath);
    hFind = FindFirstFile(search_path, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        fprintf(stderr, "Error opening directory: %s\n", dirpath);
        return;
    }

    do
    {
        if (strcmp(findFileData.cFileName, ".") == 0 ||
            strcmp(findFileData.cFileName, "..") == 0)
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s\\%s", dirpath, findFileData.cFileName);

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            gsk_filesystem_traverse(
              path, process_fn); // Recursively handle directories
        } else
        {
            // fix slashes and convert to URI
            _to_forward_slash(path);
            char output[GSK_FS_MAX_PATH];
            gsk_filesystem_path_to_uri(path, output);

            process_fn(output); // process file callback
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

#else

// POSIX-specific directory traversal
void
gsk_filesystem_traverse(const char *dirpath, FileUriHandler process_fn)
{
    struct dirent *entry;
    DIR *dir = opendir(dirpath);

    if (dir == NULL)
    {
        perror("opendir");
        return;
    }

    while ((entry = readdir(dir)) != NULL)
    {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
            continue;

        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", dirpath, entry->d_name);

        struct stat statbuf;
        if (stat(path, &statbuf) == -1)
        {
            perror("stat");
            continue;
        }

        if (S_ISDIR(statbuf.st_mode))
        {
            gsk_filesystem_traverse(
              path, process_fn); // Recursively handle directories
        } else if (S_ISREG(statbuf.st_mode))
        {
            process_fn(path); // process file callback
        }
    }

    closedir(dir);
}

#endif