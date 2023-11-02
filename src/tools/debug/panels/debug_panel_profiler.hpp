#ifndef __DEBUG_PANEL_PROFILER_HPP__
#define __DEBUG_PANEL_PROFILER_HPP__

#include <tools/debug/debug_panel.hpp>

namespace gsk {
namespace tools {
namespace panels {

class Profiler : public DebugPanel {
   public:
    _DECL_DEBUG_PANEL(Profiler);
    virtual void draw(void);
};

} // namespace panels
} // namespace tools
} // namespace gsk

#endif // __DEBUG_PANEL_ASSETS_HPP__