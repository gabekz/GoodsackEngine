/*
 * Copyright (c) 2023, Gabriel Kutuzov
 * SPDX-License-Identifier: MIT
 */

#ifndef __DEBUG_PANEL_COMPONENT_VIEWER_HPP__
#define __DEBUG_PANEL_COMPONENT_VIEWER_HPP__

#include "entity/v1/ecs.h"
#include "tools/debug/debug_panel.hpp"

namespace gsk {
namespace tools {
namespace panels {

class ComponentViewer : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(ComponentViewer);
    virtual void draw(void);

    void show_for_entity(gsk_Entity entity);

   private:
    gsk_Entity selected_entity;
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_ENTITY_VIEWER_HPP__