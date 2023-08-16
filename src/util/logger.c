#include "logger.h"

#include <assert.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include <util/ansi_colors.h>
#include <util/sysdefs.h>

#ifdef SYS_ENV_WIN
#include <stdlib.h>
#include <winsock2.h>
#else

#ifdef SYS_ENV_UNIX
#include <pthread.h>
#include <signal.h>
#include <sys/syscall.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#endif // SYS_ENV_WIN

enum {
    /* Logger type */
    kConsoleLogger = 1 << 0,
    kFileLogger    = 1 << 1,

    kMaxFileNameLen     = 255,      /* without null character */
    kDefaultMaxFileSize = 1048576L, /* 1 MB */
};

/* Console logger */
static struct
{
    FILE *output;
    unsigned long long flushedTime;
} s_clog;

/* File logger */
static struct
{
    FILE *output;
    char filename[kMaxFileNameLen + 1];
    long maxFileSize;
    unsigned char maxBackupFiles;
    long currentFileSize;
    unsigned long long flushedTime;
} s_flog;

static volatile int s_logger;
static volatile LogLevel s_logLevel = LogLevel_INFO;
;
static volatile LogDetail s_logDetail = LogDetail_SIMPLE;
;
static volatile long s_flushInterval = 0; /* msec, 0 is auto flush off */
static volatile int s_initialized    = 0; /* false */

#if defined(_WIN32) || defined(_WIN64)
static CRITICAL_SECTION s_mutex;
#else
static pthread_mutex_t s_mutex;
#endif /* defined(_WIN32) || defined(_WIN64) */

#ifdef SYS_ENV_WIN
static int
gettimeofday(struct timeval *tp, struct timezone *tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch
    // has 9 trailing zero's This magic number is the number of 100 nanosecond
    // intervals since January 1, 1601 (UTC) until 00:00:00 January 1, 1970
    static const unsigned long long EPOCH =
      ((unsigned long long)116444736000000000ULL);

    SYSTEMTIME system_time;
    FILETIME file_time;
    unsigned long long time;

    GetSystemTime(&system_time);
    SystemTimeToFileTime(&system_time, &file_time);
    time = ((unsigned long long)file_time.dwLowDateTime);
    time += ((unsigned long long)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long)((time - EPOCH) / 10000000L);
    tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
    return 0;
}
#endif

static void
init(void)
{
    if (s_initialized) { return; }
#if defined(_WIN32) || defined(_WIN64)
    InitializeCriticalSection(&s_mutex);
#else
    pthread_mutex_init(&s_mutex, NULL);
#endif /* defined(_WIN32) || defined(_WIN64) */
    s_initialized = 1; /* true */
}

static void
getTimestamp(const struct timeval *time, char *timestamp, ui64 size)
{
    time_t sec =
      time
        ->tv_sec; /* a necessary variable to avoid a runtime error on Windows */
    struct tm calendar;

    assert(size >= 25);

// localtime_s(&sec, &calendar);
#if defined(SYS_ENV_WIN)
    localtime_s(&calendar, &sec);
#elif defined(SYS_ENV_UNIX)
    localtime_r(&sec, &calendar);
#endif

#if LOGGER_LOG_DATE
    strftime(timestamp, size, "%y-%m-%d %H:%M:%S", &calendar);
#else
    strftime(timestamp, size, "%H:%M:%S", &calendar);
#endif
    sprintf(&timestamp[17], ".%06ld", (long)time->tv_usec);
}

static int
hasFlag(int flags, int flag)
{
    return (flags & flag) == flag;
}

static char
getLevelChar(LogLevel level)
{
    switch (level) {
    case LogLevel_TRACE: return 'T';
    case LogLevel_DEBUG: return 'D';
    case LogLevel_INFO: return 'I';
    case LogLevel_WARN: return 'W';
    case LogLevel_ERROR: return 'E';
    case LogLevel_CRITICAL: return 'C';
    default: return ' ';
    }
}
static void
setLevelStr(LogLevel level, int colored, char *dest)
{
    char *levelStr;
    char *levelCol;

    switch (level) {
    case LogLevel_TRACE:
        levelStr = "Trace";
        levelCol = BLU;
        break;
    case LogLevel_DEBUG:
        levelStr = "Debug";
        levelCol = YEL;
        break;
    case LogLevel_INFO:
        levelStr = "Info";
        levelCol = GRN;
        break;
    case LogLevel_WARN:
        levelStr = "Warning";
        levelCol = YEL;
        break;
    case LogLevel_ERROR:
        levelStr = "Error";
        levelCol = MAG;
        break;
    case LogLevel_CRITICAL:
        levelStr = "Critical";
        levelCol = RED;
        break;
    default: return;
    }

    if (colored) {
        dest[0] = '\0';
        strncpy(dest, levelCol, 32);
        strncat(dest, levelStr, 32);
        strncat(dest, COLOR_RESET, 32);
    }
}

static void
lock(void)
{
#if defined(_WIN32) || defined(_WIN64)
    EnterCriticalSection(&s_mutex);
#else
    pthread_mutex_lock(&s_mutex);
#endif /* defined(_WIN32) || defined(_WIN64) */
}

static void
unlock(void)
{
#if defined(_WIN32) || defined(_WIN64)
    LeaveCriticalSection(&s_mutex);
#else
    pthread_mutex_unlock(&s_mutex);
#endif /* defined(_WIN32) || defined(_WIN64) */
}

static long
getCurrentThreadID(void)
{
#if defined(_WIN32) || defined(_WIN64)
    return GetCurrentThreadId();
#elif __linux__
    return syscall(SYS_gettid);
#elif defined(__APPLE__) && defined(__MACH__)
    return syscall(SYS_thread_selfid);
#else
    return (long)pthread_self();
#endif /* defined(_WIN32) || defined(_WIN64) */
}

static void
getBackupFileName(const char *basename,
                  unsigned char index,
                  char *backupname,
                  size_t size)
{
    char indexname[5];

    assert(size >= strlen(basename) + sizeof(indexname));

    strncpy(backupname, basename, size);
    if (index > 0) {
        sprintf(indexname, ".%d", index);
        strncat(backupname, indexname, strlen(indexname));
    }
}

static int
isFileExist(const char *filename)
{
    FILE *fp;

    if ((fp = fopen(filename, "r")) == NULL) {
        return 0;
    } else {
        fclose(fp);
        return 1;
    }
}

static long
getFileSize(const char *filename)
{
    FILE *fp;
    long size;

    if ((fp = fopen(filename, "rb")) == NULL) { return 0; }
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fclose(fp);
    return size;
}

static long
vflog(FILE *fp,
      const char *levelStr,
      const char *timestamp,
      long threadID,
      const char *file,
      int line,
      const char *fmt,
      va_list arg,
      unsigned long long currentTime,
      unsigned long long *flushedTime)
{
    int size       = 0;
    long totalsize = 0;

    if (s_logDetail == LogDetail_SIMPLE) {
        size = fprintf(fp, "%s [%s] ", timestamp, levelStr);
    } else if (s_logDetail == LogDetail_EXTENDED) {
        size = fprintf(
          fp, "%s [%s] %ld %s:%d: ", timestamp, levelStr, threadID, file, line);
    } else if (s_logDetail == LogDetail_MSG) {
        size = 0;
    }

    if (size > 0) { totalsize += size; }

    if ((size = vfprintf(fp, fmt, arg)) > 0) { totalsize += size; }
    if ((size = fprintf(fp, "\n")) > 0) { totalsize += size; }
    if (s_flushInterval > 0) {
        if (currentTime - *flushedTime > s_flushInterval) {
            fflush(fp);
            *flushedTime = currentTime;
        }
    }
    return totalsize;
}

static int
rotateLogFiles(void)
{
    int i;
    /* backup filename: <filename>.xxx (xxx: 1-255) */
    char src[kMaxFileNameLen + 5],
      dst[kMaxFileNameLen + 5]; /* with null character */

    if (s_flog.currentFileSize < s_flog.maxFileSize) {
        return s_flog.output != NULL;
    }
    fclose(s_flog.output);
    for (i = (int)s_flog.maxBackupFiles; i > 0; i--) {
        getBackupFileName(s_flog.filename, i - 1, src, sizeof(src));
        getBackupFileName(s_flog.filename, i, dst, sizeof(dst));
        if (isFileExist(dst)) {
            if (remove(dst) != 0) {
                fprintf(
                  stderr, "ERROR: logger: Failed to remove file: `%s`\n", dst);
            }
        }
        if (isFileExist(src)) {
            if (rename(src, dst) != 0) {
                fprintf(stderr,
                        "ERROR: logger: Failed to rename file: `%s` -> `%s`\n",
                        src,
                        dst);
            }
        }
    }
    s_flog.output = fopen(s_flog.filename, "a");
    if (s_flog.output == NULL) {
        fprintf(stderr,
                "ERROR: logger: Failed to open file: `%s`\n",
                s_flog.filename);
        return 0;
    }
    s_flog.currentFileSize = getFileSize(s_flog.filename);
    return 1;
}
int
logger_isEnabled(LogLevel level)
{
    return s_logLevel <= level;
}

void
logger_flush()
{
    if (s_logger == 0 || !s_initialized) {
        assert(0 && "logger is not initialized");
        return;
    }

    if (hasFlag(s_logger, kConsoleLogger)) { fflush(s_clog.output); }
    if (hasFlag(s_logger, kFileLogger)) { fflush(s_flog.output); }
}

void
logger_log(LogLevel level, const char *file, int line, const char *fmt, ...)
{
    struct timeval now;
    unsigned long long currentTime; /* milliseconds */
    char levelStr[32];
    char levelc;
    char timestamp[32];
    long threadID;
    va_list carg, farg;

    if (s_logger == 0 || !s_initialized) {
        assert(0 && "logger is not initialized");
        return;
    }

    if (!logger_isEnabled(level)) { return; }

    switch (level) {
    case LogLevel_INFO: logger_setDetail(LogDetail_SIMPLE); break;
    case LogLevel_NONE: logger_setDetail(LogDetail_MSG); break;
    default: logger_setDetail(LogDetail_EXTENDED); break;
    }

    gettimeofday(&now, NULL);
    currentTime = now.tv_sec * 1000 + now.tv_usec / 1000;
    levelc      = getLevelChar(level);
    threadID    = getCurrentThreadID();

    if (level != LogLevel_NONE) { setLevelStr(level, TRUE, levelStr); }

    getTimestamp(&now, timestamp, sizeof(timestamp));

    lock();

    if (hasFlag(s_logger, kConsoleLogger)) {
        va_start(carg, fmt);
        vflog(s_clog.output,
              levelStr,
              timestamp,
              threadID,
              file,
              line,
              fmt,
              carg,
              currentTime,
              &s_clog.flushedTime);
        va_end(carg);
    }
    if (hasFlag(s_logger, kFileLogger)) {
        if (rotateLogFiles()) {
            va_start(farg, fmt);
            s_flog.currentFileSize += vflog(s_flog.output,
                                            levelStr,
                                            timestamp,
                                            threadID,
                                            file,
                                            line,
                                            fmt,
                                            farg,
                                            currentTime,
                                            &s_flog.flushedTime);
            va_end(farg);
        }
    }
    unlock();
}

int
logger_initConsoleLogger(FILE *output)
{
    output = (output != NULL) ? output : stdout;
    if (output != stdout && output != stderr) {
        assert(0 && "output must be stdout or stderr");
        return 0;
    }

    init();
    lock();
    s_clog.output = output;
    s_logger |= kConsoleLogger;
    unlock();
    return 1;
}

void
logger_setLevel(LogLevel level)
{
    s_logLevel = level;
}

LogLevel
logger_getLevel()
{
    return s_logLevel;
}

void
logger_setDetail(LogDetail detail)
{
    s_logDetail = detail;
}

LogDetail
logger_getDetail()
{
    return s_logDetail;
}

int
logger_initFileLogger(const char *filename,
                      long maxFileSize,
                      unsigned char maxBackupFiles)
{
    int ok = 0; /* false */

    if (filename == NULL) {
        assert(0 && "filename must not be NULL");
        return 0;
    }
    if (strlen(filename) > kMaxFileNameLen) {
        assert(0 && "filename exceeds the maximum number of characters");
        return 0;
    }

    init();
    lock();
    if (s_flog.output != NULL) { /* reinit */
        fclose(s_flog.output);
    }
    s_flog.output = fopen(filename, "a");
    if (s_flog.output == NULL) {
        fprintf(stderr, "ERROR: logger: Failed to open file: `%s`\n", filename);
        goto cleanup;
    }
    s_flog.currentFileSize = getFileSize(filename);
    strncpy(s_flog.filename, filename, sizeof(s_flog.filename));
    s_flog.maxFileSize = (maxFileSize > 0) ? maxFileSize : kDefaultMaxFileSize;
    s_flog.maxBackupFiles = maxBackupFiles;
    s_logger |= kFileLogger;
    ok = 1; /* true */
cleanup:
    unlock();
    return ok;
}
