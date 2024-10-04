/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#define GSK_FS_MAX_PATH    256
#define GSK_FS_MAX_SCHEME  16
#define GSK_FS_MAX_SEG_LEN 256

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// absolute path from the provided uri
#define GSK_PATH(uri) (gsk_filesystem_uri_to_path(uri).path)

typedef void (*FileUriHandler)(const char *uri);

typedef struct gsk_URI
{
    // char uri_full[GSK_FS_MAX_PATH];
    char scheme[GSK_FS_MAX_SEG_LEN]; // scheme
    char macro[GSK_FS_MAX_SEG_LEN];  // macro/alias - always relative
    char path[GSK_FS_MAX_SEG_LEN];

    // TODO: Filename and extension (for :macro expansion)
} gsk_URI;

typedef struct gsk_Path
{
    char path[GSK_FS_MAX_PATH]; // absolute path
    gsk_URI uri;
} gsk_Path;

// memstream utilities //

#if 0
static void filesystem_memstream(BUFFER, LEN);
static void filesystem_flush(BUFFER);
#endif
// filesystem_path(FS_DIR_DEBUG, "logs/logs.txt");

void
gsk_filesystem_initialize(const char *project_root, const char *project_scheme);

char *
gsk_filesystem_get_extension(const char *path);

gsk_URI
gsk_filesystem_uri(const char *uri_path);

gsk_Path
gsk_filesystem_uri_to_path(const char *uri_path);

char *
gsk_filesystem_path_to_uri(const char *file_path, char *output_uri);

void
gsk_filesystem_traverse(const char *dirpath, FileUriHandler process_fn);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __FILESYSTEM_H__