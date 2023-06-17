#ifndef HPP_DEBUGUI
#define HPP_DEBUGUI

extern "C" {
#include <core/graphics/renderer/v1/renderer.h>
}
#include <entity/v1/ecs.h>

class DebugGui {
   public:
    DebugGui(Renderer *renderer);
    ~DebugGui();
    void Update();
    void Render();
    void SetStyle();

    void SetVisibility(bool value);
    void ToggleVisibility();

   protected:
    Renderer *m_renderer;
    bool m_showExample;
    bool m_showSceneViewer;
    bool m_showSceneLighting;
    bool m_showEntityViewer;
    bool m_showComponentViewer;
    bool m_showAssets;
    bool m_showProfiler;

    Entity m_selectedEntity;

    bool m_debugEnabled;

   private:
    int m_sceneQueued;
};

#endif // HPP_DEBUGUI
