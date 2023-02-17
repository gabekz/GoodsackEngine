#ifndef HPP_DEBUGUI
#define HPP_DEBUGUI

extern "C" {
#include <core/renderer/v1/renderer.h>
}
#include <ecs/ecs.h>

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
    bool m_showProfiler;
    bool m_showHDR;

    Entity m_selectedEntity;

    bool m_debugEnabled;

   private:
    int m_sceneQueued;
};

#endif // HPP_DEBUGUI