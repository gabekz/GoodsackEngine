/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __SKYBOX_H__
#define __SKYBOX_H__

#include "core/drivers/opengl/opengl.h"
#include "core/graphics/shader/shader.h"
#include "core/graphics/texture/texture.h"

// 0 -- Regular | 1 -- Blurred Render
#define SKYBOX_DRAW_BLUR 0

#ifdef __cplusplus
extern "C" {
#endif

typedef struct gsk_Skybox
{
    gsk_Texture *cubemap;
    gsk_Texture *hdrTexture;
    gsk_Texture *irradianceMap;
    gsk_Texture *prefilterMap;
    gsk_Texture *brdfLUTTexture;
    gsk_GlVertexArray *vao;
    gsk_ShaderProgram *shader;
} gsk_Skybox;

gsk_Skybox *
gsk_skybox_create(gsk_Texture *cubemap);

void
gsk_skybox_draw(gsk_Skybox *self);

// HDR

gsk_Skybox *
gsk_skybox_hdr_create();
gsk_Texture *
gsk_skybox_hdr_projection(gsk_Skybox *skybox);

#ifdef __cplusplus
}
#endif

#endif // __SKYBOX_H__
