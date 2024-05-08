/*
 * Copyright (c) 2024, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#include "texture_set.h"

#include "util/array_list.h"
#include "util/logger.h"
#include "util/sysdefs.h"

#include <string.h>

#define ALLOC_ITER 20

gsk_TextureSet
gsk_texture_set_init()
{
    gsk_TextureSet ret = {0};

    ret.list_descriptors =
      array_list_init(sizeof(gsk_TextureDescriptor), ALLOC_ITER);

    return ret;
}

void *
gsk_texture_set_add(gsk_TextureSet *p_self,
                    void *p_texture,
                    const char *lookup_name)
{
    gsk_TextureDescriptor descriptor = {0};
    descriptor.p_texture             = p_texture;
    strcpy(descriptor.name_reference, lookup_name);

    if (p_self->list_descriptors.is_list_empty)
    {
        array_list_push(&p_self->list_descriptors, &descriptor);
        return;
    }

    for (int i = 0; i < p_self->list_descriptors.list_next; i++)
    {
        gsk_TextureDescriptor *cmp_desc =
          array_list_get_at_index(&p_self->list_descriptors, i);

        if (!strcmp(descriptor.name_reference, cmp_desc->name_reference))
        {
            LOG_WARN("Can't add, reference already exists");
            return;
        }
    }

    array_list_push(&p_self->list_descriptors, &descriptor);
}

void *
gsk_texture_set_get_by_name(gsk_TextureSet *p_self, const char *lookup_name)
{
    for (int i = 0; i < p_self->list_descriptors.list_next; i++)
    {
        gsk_TextureDescriptor *desc =
          array_list_get_at_index(&p_self->list_descriptors, i);
        if (!strcmp(desc->name_reference, lookup_name))
        {
            return desc->p_texture;
        }
    }
    return NULL;
}