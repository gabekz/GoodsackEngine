/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

/*-----------------------------------------
 * System-specific definitions
 * --------------------------------------*/

#ifndef __SYSDEFS_H__
#define __SYSDEFS_H__

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

#define CACHE_LINE 16

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

#define CACHE_LINE 16

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

#define SYS_DEBUG SYS_ENABLED

// -- Cache/Memory Alignment //

#if defined(SYS_ENV_WIN)
#define CACHE_ALIGN(...) __declspec(align(CACHE_LINE)) __VA_ARGS__
#elif defined(SYS_ENV_UNIX)
#define CACHE_ALIGN(...) __VA_ARGS__ __attribute__((aligned(CACHE_LINE)))
#else
#define CACHE_ALIGN void
#pragma warning Unknown dynamic link import / export semantics.
#endif

// -- Assert //

#ifdef __cplusplus
#ifndef _Static_assert
#define _Static_assert static_assert
#endif // _Static_assert
#endif // __cplusplus
#define STATIC_ASSERT(test_true) \
    _Static_assert((test_true), "(" #test_true ") failed")

#if defined(_MSC_VER)
#define _BRK __debugbreak
#elif defined(__GNUC__)
#define _BRK __builtin_trap
#endif

// -- Generally useful string-macros

#define GLUE_HELPER(x, y) x##y
#define GLUE(x, y)        GLUE_HELPER(x, y)

#define _EXPAND(x) x

// -- Types //

#if defined(SYS_ENV_64)
typedef signed long long s64;
typedef unsigned long long u64;
#if defined(SYS_ENV_WIN)
typedef unsigned long long ulong;
#endif // defined(SYS_ENV_WIN)
#else
typedef signed long s64;
typedef unsigned long u64;
#endif // defined(SYS_ENV_64)

typedef double f64;

typedef signed int s32;
typedef unsigned int u32;

typedef float f32;

typedef signed short s16;
typedef unsigned short u16;

typedef signed char s8;
typedef unsigned char u8;

// #ifdef byte_t
// #undef byte_t
// #endif
typedef u8 byte_t;

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

#endif // __SYSDEFS_H__