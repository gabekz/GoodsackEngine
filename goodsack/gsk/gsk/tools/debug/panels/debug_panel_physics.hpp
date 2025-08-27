/*
 * Copyright (c) 2022-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_PANEL_PHYSICS_HPP__
#define __DEBUG_PANEL_PHYSICS_HPP__

#include "tools/debug/debug_panel.hpp"

namespace gsk {
namespace tools {
namespace panels {

class Physics : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(Physics);
    virtual void draw(void);
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_PHYSICS_HPP__