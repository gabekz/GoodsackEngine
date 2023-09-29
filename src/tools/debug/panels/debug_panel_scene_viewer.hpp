#ifndef __DEBUG_PANEL_SCENE_VIEWER_HPP__
#define __DEBUG_PANEL_SCENE_VIEWER_HPP__

#include <tools/debug/debug_panel.hpp>

namespace gsk {
namespace tools {
namespace panels {

class SceneViewer : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(SceneViewer);

   public:
    virtual void draw(void);

   private:
    int scene_queued = 0;
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_SCENE_VIEWER_HPP__