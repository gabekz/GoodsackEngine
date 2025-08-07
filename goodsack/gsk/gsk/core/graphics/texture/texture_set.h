/*
 * Copyright (c) 2024-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __TEXTURE_SET_H__
#define __TEXTURE_SET_H__

#include "util/array_list.h"

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

typedef struct gsk_TextureDescriptor
{
    void *p_texture;          /* pointer to the texture in memory */
    char name_reference[256]; /* reference to the texture in the table */
} gsk_TextureDescriptor;

typedef struct gsk_TextureSet
{
    ArrayList list_descriptors;
} gsk_TextureSet;

gsk_TextureSet
gsk_texture_set_init();

void *
gsk_texture_set_add(gsk_TextureSet *p_self,
                    void *p_texture,
                    const char *lookup_name);

void *
gsk_texture_set_get_by_name(gsk_TextureSet *p_self, const char *lookup_name);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif /* __TEXTURE_SET_H__ */