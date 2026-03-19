#ifndef __GSK_LLIB_VECTOR_H__
#define __GSK_LLIB_VECTOR_H__

#include "util/sysdefs.h"

#define VECTOR_LIB "goodsack.vector"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_L_Vector
{
    f32 float3[3];
} gsk_L_Vector;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__GSK_LLIB_VECTOR_H__