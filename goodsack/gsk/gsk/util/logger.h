/*
 * Copyright (c) 2022-2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __LOGGER_H__
#define __LOGGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

#ifndef __FILE_NAME__
#define __FILE_NAME__ __FILE__
#endif

#define LOG_PRINT(fmt, ...) \
    logger_log(LogLevel_NONE, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_TRACE(fmt, ...) \
    logger_log(LogLevel_TRACE, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(fmt, ...) \
    logger_log(LogLevel_DEBUG, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) \
    logger_log(LogLevel_INFO, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) \
    logger_log(LogLevel_WARN, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) \
    logger_log(LogLevel_ERROR, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_CRITICAL(fmt, ...) \
    logger_log(LogLevel_CRITICAL, __FILE_NAME__, __LINE__, fmt, ##__VA_ARGS__)

typedef enum {
    LogLevel_NONE,
    LogLevel_TRACE,
    LogLevel_DEBUG,
    LogLevel_INFO,
    LogLevel_WARN,
    LogLevel_ERROR,
    LogLevel_CRITICAL,
} LogLevel;

typedef enum {
    LogDetail_SIMPLE,
    LogDetail_EXTENDED,
    LogDetail_MSG,
} LogDetail;

/**
 * Initialize the logger as a console logger.
 * If the file pointer is NULL, stdout will be used.
 *
 * @param[in] output A file pointer. Make sure to set stdout or stderr.
 * @return Non-zero value upon success or 0 on error
 */
int
logger_initConsoleLogger(FILE *output);

/**
 * Initialize the logger as a file logger.
 * If the filename is NULL, return without doing anything.
 *
 * @param[in] filename The name of the output file
 * @param[in] maxFileSize The maximum number of bytes to write to any one file
 * @param[in] maxBackupFiles The maximum number of files for backup
 * @return Non-zero value upon success or 0 on error
 */
int
logger_initFileLogger(const char *filename,
                      long maxFileSize,
                      unsigned char maxBackupFiles);

/**
 * Set the log level.
 * Message levels lower than this value will be discarded.
 * The default log level is INFO.
 *
 * @param[in] level A log level
 */
void
logger_setLevel(LogLevel level);
LogLevel
logger_getLevel();

void
logger_setDetail(LogDetail detail);
LogDetail
logger_SetDetail();

void
logger_flush();
int
logger_isEnabled(LogLevel level);

void
logger_log(LogLevel level, const char *file, int line, const char *fmt, ...);

#ifdef __cplusplus
} /* extern "C" { */
#endif /* __cplusplus */

#endif // __LOGGER_H__
