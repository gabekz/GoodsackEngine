/*
 * Copyright (c) 2023-present, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_PANEL_ENTITY_VIEWER_HPP__
#define __DEBUG_PANEL_ENTITY_VIEWER_HPP__

#include "entity/ecs.h"
#include "tools/debug/debug_panel.hpp"

namespace gsk {
namespace tools {
namespace panels {

// forward declare ComponentViewer
class ComponentViewer;

class EntityViewer : public DebugPanel {

   public:
    _DECL_DEBUG_PANEL(EntityViewer);

    virtual void draw(void);

    void set_component_viewer(ComponentViewer *ref);

   private:
    ComponentViewer *p_component_viewer; // pointer to component viewer
                                         // for toggling its panel
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_ENTITY_VIEWER_HPP__