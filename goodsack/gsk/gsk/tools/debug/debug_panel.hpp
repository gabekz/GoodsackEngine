/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_PANEL_HPP__
#define __DEBUG_PANEL_HPP__

#include "core/graphics/renderer/v1/renderer.h"
#include <string>

#define _DECL_DEBUG_PANEL(x)     x(std::string str) : DebugPanel(str) {};
#define _DECL_DEBUG_PANEL2(x, y) x(y) : DebugPanel(y) {};

namespace gsk {
namespace tools {

class DebugPanel {
   public:
    DebugPanel(std::string str) : title(str) {};
    // virtual ~DebugPanel();

    virtual void draw(void) = 0;
    void set_menu_index(int menu_index);

    bool visible   = false;
    int menu_index = -1;

    std::string title;

    gsk_Renderer *p_renderer;
};

} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_HPP__