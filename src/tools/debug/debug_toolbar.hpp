#ifndef __DEBUG_TOOLBAR_HPP__
#define __DEBUG_TOOLBAR_HPP__

#include <vector>

extern "C" {
#include <core/graphics/renderer/v1/renderer.h>
}
#include <entity/v1/ecs.h>

#define DEBUG_MENUS_COUNT 3

namespace gsk {
namespace tools {

// forward-declare
class DebugPanel;

class DebugToolbar {
   public:
    DebugToolbar(Renderer *renderer);
    ~DebugToolbar();

    void set_visibility(bool value);
    void toggle_visibility(void);

    void add_panel(DebugPanel *panel, int menu_index);

    void update(void);
    void render(void);

#if 0
    void set_style(void);
#endif

   protected:
    Renderer *m_renderer;

#if 1
    bool m_showExample;
    bool m_showSceneViewer;
    bool m_showSceneLighting;
    bool m_showEntityViewer;
    bool m_showComponentViewer;
    bool m_showAssets;
    bool m_showProfiler;
#endif

    Entity m_selectedEntity;

    bool m_debugEnabled;

    enum class Menus { File, Scene, Pipeline };
    const char *p_menu_names[DEBUG_MENUS_COUNT] = {"File", "Scene", "Pipeline"};

   private:
    int m_sceneQueued;
    std::vector<DebugPanel *> debug_panels;
    bool show_example_panel = false;
};

} // namespace tools
} // namespace gsk

#endif // __DEBUG_TOOLBAR_HPP__
