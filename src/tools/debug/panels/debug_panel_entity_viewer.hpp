#ifndef __DEBUG_PANEL_ENTITY_VIEWER_HPP__
#define __DEBUG_PANEL_ENTITY_VIEWER_HPP__

#include <entity/v1/ecs.h>
#include <tools/debug/debug_panel.hpp>

namespace gsk {
namespace tools {
namespace panels {

class EntityViewer : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(EntityViewer);
    virtual void draw(void);

   private:
    Entity selected_entity;
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_ENTITY_VIEWER_HPP__