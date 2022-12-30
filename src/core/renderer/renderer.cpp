#include "renderer.hpp"

#include <core/context/context.h>
#include <core/scene/scene.h>

//#include <ecs/ecs.h>

Renderer::Renderer(int windowWidth, int windowHeight)
{
    m_window       = createWindow(windowWidth, windowHeight, &m_vulkanDevice);
    m_windowWidth  = windowWidth;
    m_windowHeight = windowHeight;

    Scene *scene = (Scene *)malloc(sizeof(Scene));
    scene->id    = 0;
    // scene->ecs = ecs_init(NULL);
    Scene **sceneList = (Scene **)malloc(sizeof(Scene *));
    *(sceneList)      = scene;

    m_sceneInfo.sceneL      = sceneList;
    m_sceneInfo.sceneC      = 1;
    m_sceneInfo.activeScene = 0;
}

void
Renderer::Prime()
{
}

void
Renderer::Tick()
{
}
