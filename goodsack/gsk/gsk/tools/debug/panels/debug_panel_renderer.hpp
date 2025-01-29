/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_PANEL_RENDERER_HPP__
#define __DEBUG_PANEL_RENDERER_HPP__

#include "tools/debug/debug_panel.hpp"

namespace gsk {
namespace tools {
namespace panels {

class RenderInfo : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(RenderInfo);
    virtual void draw(void);
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_RENDERER_HPP__