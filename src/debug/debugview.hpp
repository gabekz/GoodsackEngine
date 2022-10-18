#ifndef H_DEBUGVIEW
#define H_DEBUGVIEW

#include <core/renderer/renderer.h>
#include <ecs/ecs.h>

class DebugGui {
public:
    DebugGui(Renderer* renderer);
    ~DebugGui();
    void Render();
    void SetStyle();

protected:
    Renderer *m_renderer;
    bool m_showExample;
    bool m_showSceneViewer;
    bool m_showEntityViewer;
    bool m_showComponentViewer;

    Entity m_selectedEntity;
};

#endif // H_DEBUGVIEW
