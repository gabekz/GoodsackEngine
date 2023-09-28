#ifndef __DEBUG_PANEL_HPP__
#define __DEBUG_PANEL_HPP__

namespace gsk {
namespace tools {

class DebugPanel {
   public:
    // DebugPanel();
    // virtual ~DebugPanel();

    virtual void draw() = 0;
    bool visible        = false;
};

namespace panels {
class SceneViewer : public DebugPanel {
   public:
    virtual void draw();
};
} // namespace panels

} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_HPP__