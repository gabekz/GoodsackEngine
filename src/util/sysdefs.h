/*-----------------------------------------
 * System-specific definitions
 * --------------------------------------*/

#ifndef H_SYSDEFS
#define H_SYSDEFS

// -- Platform definitions //

// Windows definitions
#if defined(_WIN32) || defined(_WIN64)
#define SYS_ENV_WIN
#if defined(_WIN64)
#define SYS_ENV_64
#else
#define SYS_ENV_32
#endif // _WIN64
#endif // WIN

// Unix definitions
#if defined(__GNUC__) || defined(__linux__) || defined(__unix__)
#define SYS_ENV_UNIX
#define _GNU_SOURCE_

#if defined(__x86_64__) || defined(__ppc64__)
#define SYS_ENV_64
#else
#define SYS_ENV_32
#endif
#endif // GCC

// Windows badly defines a lot of stuff. Undefine it.
#if defined(SYS_ENV_WIN)

#define WIN32_LEAN_AND_MEAN
#undef min    // override standard definition
#undef max    // override standard definition
#undef ERROR  // override (really stupid) wingdi.h standard definition
#undef DELETE // override (another really stupid) winnt.h standard definition
#undef MessageBox // override winuser.h standard definition
#undef Error
#undef OK
#undef CONNECT_DEFERRED // override from Windows SDK, clashes with Object enum

#endif // SYS_ENV_WIN

// -- C Specific boolean definitions //
#ifndef __cplusplus

#ifdef TRUE
#undef TRUE
#endif
#define TRUE 1

#ifdef FALSE
#undef FALSE
#endif
#define FALSE 0

#endif // __cplusplus

// -- Result //

#define SYS_SUCCESS 1
#define SYS_FAILURE 0

#define SYS_ENABLED  1
#define SYS_DISABLED 0

#define SYS_LOG_LEVEL 0
#define SYS_DEBUG     SYS_ENABLED

#ifndef SYS_DEBUG
#define SYS_DEBUG SYS_DISABLED
#endif

// -- Types //

#if defined(SYS_ENV_64)
typedef signed long long si64;
typedef unsigned long long ui64;
#if defined(SYS_ENV_WIN32)
typedef unsigned long long ulong;
#endif // defined(SYS_ENV_WIN32)
#else
typedef signed long si64;
typedef unsigned long ui64;
#endif // defined(SYS_ENV_64)

typedef double f64;

typedef signed int si32;
typedef unsigned int ui32;

typedef float f32;

typedef signed short si16;
typedef unsigned short ui16;

typedef signed char si8;
typedef unsigned char ui8;

// #ifdef byte_t
// #undef byte_t
// #endif
typedef ui8 byte_t;

// #ifdef size_t
// #undef size_t
// #endif

#if 0
#ifndef __cplusplus 
typedef unsigned char bool;
#endif // __cplusplus
#endif

#if 0
typedef const char*     cstring;
#ifdef __cplusplus
typedef std::string     string;
#endif
#endif

#endif // H_SYSDEFS
