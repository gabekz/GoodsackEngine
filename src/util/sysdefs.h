/*-----------------------------------------
 * System-specific definitions
 * --------------------------------------*/

#ifndef H_SYSDEFS
#define H_SYSDEFS

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
#if defined(__x86_64__) || defined(__ppc64__)
#define SYS_ENV_64
#else
#define SYS_ENV_32
#endif
#endif // GCC

#define DEFAULT_WINDOW_WIDTH 1280
#define DEFAULT_WINDOW_HEIGHT 720

#define DRAWING_MODE GL_TRIANGLES

#define OPEN_GL     0xF0F01
#define VULKAN      0xF0F02
#define DX11        0xF0F03
#define DX12        0xF0F04
#define RENDERER    OPEN_GL

// Drawing modes
#define DRAW_MODE_ARRAYS     0xD000
#define DRAW_MODE_ELEMENTS   0xD001

#define SHADOW_WIDTH    2048
#define SHADOW_HEIGHT   2048

#define LOG_LEVEL   0
#define DEBUG       1

/* 
 * Types 
*/

typedef signed char     sichar;
typedef unsigned char   byte;

typedef unsigned long   ulng;

typedef short           si16;
typedef unsigned short  ui16;

typedef int             si32;
typedef unsigned int    ui32;

typedef float           f32;
typedef double          f64;

#if defined(SYS_ENV_64)
typedef unsigned long       ulng;
typedef long int            si64;
typedef unsigned long int   ui64;
#endif

#if 0
typedef const char*     cstring;
#ifdef __cplusplus
typedef std::string     string;
#endif
#endif

#endif // H_SYSDEFS
