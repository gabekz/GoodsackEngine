/*-----------------------------------------
 * System-specific definitions
 * --------------------------------------*/

#ifndef H_SYSDEFS
#define H_SYSDEFS

/*
 * Platform
 */

// Windows definitions
#if defined(_WIN32) || defined(_WIN64)
#define SYS_ENV_WIN
#if defined(_WIN64)
#define SYS_ENV_64
#else
#define SYS_ENV_32
#endif
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
#ifdef SYS_ENV_WIN
#undef min    // override standard definition
#undef max    // override standard definition
#undef ERROR  // override (really stupid) wingdi.h standard definition
#undef DELETE // override (another really stupid) winnt.h standard definition
#undef MessageBox // override winuser.h standard definition
#undef Error
#undef OK
#undef CONNECT_DEFERRED // override from Windows SDK, clashes with Object enum
#endif

#ifndef __cplusplus // C-Specific

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#define TRUE  1
#define FALSE 0

#endif // C-Specific

#define SYS_SUCCESS 1
#define SYS_FAILURE 0

#define SYS_ENABLED  1
#define SYS_DISABLED 0

#define SYS_LOG_LEVEL 0
#define SYS_DEBUG     SYS_ENABLED

#ifndef SYS_DEBUG
#define SYS_DEBUG SYS_DISABLED
#endif

/*
 * Types
 */

typedef signed char sichar;
typedef unsigned char byte;

typedef unsigned long ulng;

typedef short si16;
typedef unsigned short ui16;

typedef int si32;
typedef unsigned int ui32;

typedef float f32;
typedef double f64;

#if defined(SYS_ENV_64)
typedef unsigned long ulng;
typedef long int si64;
typedef unsigned long int ui64;
#endif

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
