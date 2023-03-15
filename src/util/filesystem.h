#ifndef H_FILESYSTEM
#define H_FILESYSTEM

#include <util/sysdefs.h>

#define FILESYSTEM_PATH_EXE

#define FILESYSTEM_WORKING_DIRECTORY
#define FILESYSTEM_RESOURCES_DIRECTORY
#define FILESYSTEM_DEBUG_DIRECTORY

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

#if defined(_WIN32)
#include <windows.h>
#endif

typedef struct goodsack_file
{
    char *fileExtension;
} goodsack_file;

static inline void
_GetDir()
{
    TCHAR NPath[260];

    GetCurrentDirectory(MAX_PATH, NPath);
}

// memstream utilities //

static void filesystem_memstream(BUFFER, LEN);
static void filesystem_flush(BUFFER);
// filesystem_path(FS_DIR_DEBUG, "logs/logs.txt");

// file properties //

static inline char *
filesystem_get_extension(const char *path)
{
    char *ext = strrchr(path, '.');
    return ext;
}

#endif // H_FILESYSTEM