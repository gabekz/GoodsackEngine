#ifndef __FILESYSTEM_H__
#define __FILESYSTEM_H__

#define GSK_FS_MAX_PATH    260
#define GSK_FS_MAX_SEG_LEN 99

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

// absolute path from the provided uri
#define GSK_PATH(uri) (gsk_filesystem_path_from_uri(uri).path)

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

// file properties //

char *
gsk_filesystem_get_extension(const char *path);

gsk_URI
gsk_filesystem_uri(const char *uri_path);

gsk_Path
gsk_filesystem_path_from_uri(const char *uri_path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __FILESYSTEM_H__