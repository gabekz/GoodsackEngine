/*-----------------------------------------
 * System-specific definitions
 * --------------------------------------*/

#ifndef SYSDEFS_H
#define SYSDEFS_H

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

#define LOG_LEVEL   0
#define DEBUG       1


typedef int si32;
typedef unsigned int ui32;

typedef short si16;
typedef unsigned short ui16;

typedef signed char schr;
typedef unsigned char byte;
typedef unsigned long ulng;


#if 0 // 32-bit
typedef long si64;
typedef unsigned long ui64;
#endif
#if 1 // 64-bit
typedef long long si64;
typedef unsigned long long ui64;
#endif

#endif // SYSDEFS_H
