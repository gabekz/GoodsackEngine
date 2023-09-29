#ifndef __DEBUG_PANEL_COMPONENT_VIEWER_HPP__
#define __DEBUG_PANEL_COMPONENT_VIEWER_HPP__

#include <entity/v1/ecs.h>
#include <tools/debug/debug_panel.hpp>

namespace gsk {
namespace tools {
namespace panels {

class ComponentViewer : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(ComponentViewer);
    virtual void draw(void);

   private:
    Entity selected_entity;
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_ENTITY_VIEWER_HPP__