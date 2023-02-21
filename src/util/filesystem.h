#ifndef H_FILESYSTEM
#define H_FILESYSTEM

#include <windows.h>

#define FS_EXE_PATH

#define FS_DIR_WORKING
#define FS_DIR_RESOURCES
#define FS_DIR_DEBUG

#ifndef MAX_PATH
#define MAX_PATH 260
#endif

static inline void
_GetDir()
{
    TCHAR NPath[260];

    GetCurrentDirectory(MAX_PATH, NPath);
}

filesystem_path(FS_DIR_DEBUG, "logs/logs.txt");

static void filesystem_memstream(BUFFER, LEN);
static void filesystem_flush(BUFFER);

#endif // H_FILESYSTEM