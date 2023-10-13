#include "filesystem.h"
#include <GoodsackEngineConfig.h> // We need build info

#include <util/logger.h>
#include <util/sysdefs.h>

#if defined(SYS_ENV_WIN)
#include <windows.h>
#endif // SYS_ENV_WIN

static void
_to_forward_slash(char *buffer)
{
    int index = 0;
    while (buffer[index] && index < GSK_FS_MAX_PATH) {
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
#if defined(SYS_ENV_WIN)

#if 0 // path to executable
    GetModuleFileName(NULL, buffer, GSK_FS_MAX_PATH);
#elif 0 // path to working directory
    GetCurrentDirectory(GSK_FS_MAX_PATH, buffer);
#else // path to resource directory (from configure_file)
    char b[GSK_FS_MAX_PATH] = (_GOODSACK_FS_DIR_DATA "/");
    // strcat(b, "/res");
    strcpy(buffer, b);
    _to_forward_slash(buffer);
    return;
#endif

#elif defined(SYS_ENV_UNIX)
    LOG_ERROR("Not implemented");

#endif // SYS_ENV_WIN

    _to_forward_slash(buffer);
    _strip_filename(buffer);
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
    const int nparams = 3; // number of parameters that are to be assigned
    gsk_URI ret;

    int result = sscanf(
      uri_path, "%99[^:]%99[^/]//%99[^\n]", ret.scheme, ret.macro, ret.path);
    if (result != nparams) {
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

    char absolute_path[GSK_FS_MAX_PATH] = "";

    // Project-specific data directory
    if (!strcmp(ret.uri.scheme, "data")) {
        strcat(absolute_path, "C:/Projects");
    }
    // Engine-specific data directory
    else if (!strcmp(ret.uri.scheme, "gsk")) {

        char buff[256] = "";
        _get_absolute_path(buff);

        strcat(absolute_path, buff);
        strcat(absolute_path, ret.uri.path);
    }

    strcpy(ret.path, absolute_path);
    return ret;
}