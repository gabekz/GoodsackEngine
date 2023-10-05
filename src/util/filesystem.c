#include "filesystem.h"

#include <util/logger.h>
#include <util/sysdefs.h>

#if defined(SYS_ENV_WIN)
#include <windows.h>
#endif // SYS_ENV_WIN

static void
_get_dir()
{
#if defined(SYS_ENV_WIN)
    TCHAR NPath[GSK_FS_MAX_PATH];
    GetCurrentDirectory(GSK_FS_MAX_PATH, NPath);
#endif // SYS_ENV_WIN
}

char *
gsk_filesystem_get_extension(const char *path)
{
    char *ext = strrchr(path, '.');
    return ext;
}

gsk_URI
gsk_filesystem_uri(const char *uri_path)
{
    int nParams = 3; // number of parameters that are to be assigned
    gsk_URI ret;

    int result = sscanf(
      uri_path, "%99[^:]%99[^/]//%99[^\n]", ret.scheme, ret.macro, ret.path);
    if (result != nParams) {
        // LOG_WARN("Failed to parse URI: %s", uri_path);
        return (gsk_URI) {.scheme = '\0', .macro = '\0', .path = '\0'};
    }

    return ret;
}
gsk_Path
gsk_filesystem_path_from_uri(const char *uri_path)
{
    gsk_Path ret = (gsk_Path) {.uri = gsk_filesystem_uri(uri_path)};
    if (ret.uri.scheme[0] == '\0') {
        // LOG_WARN("Failed to parse URI: %s", uri_path);
        return (gsk_Path) {.path = '\0'};
    }

    // build path from URI

    const char *p = "hello!";
    strcpy(ret.path, p);
    return ret;
}