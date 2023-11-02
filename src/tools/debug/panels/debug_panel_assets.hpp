#ifndef __DEBUG_PANEL_ASSETS_HPP__
#define __DEBUG_PANEL_ASSETS_HPP__

#include <tools/debug/debug_panel.hpp>

namespace gsk {
namespace tools {
namespace panels {

class Assets : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(Assets);
    virtual void draw(void);
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_ASSETS_HPP__