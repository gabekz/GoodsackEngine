#ifndef HPP_RENDERER
#define HPP_RENDERER

#include <core/scene/scene.h>
#include <model/material.h>

#include <util/gfx.h>

typedef enum RenderStage { REGULAR = 0, SHADOW } RenderStage;

class Renderer {
   public:
    Renderer(int windowWidth, int windowHeight);
    virtual ~Renderer();

    void Prime();
    void Tick();

    void Update();
    void FixedUpdate();

    void SetActiveScene();

   private:
    GLFWwindow *m_window;
    int m_windowWidth, m_windowHeight;

    RenderStage m_renderStage;
    Material *m_overrideMaterial;

    VulkanDeviceContext *m_vulkanDevice;

    struct
    {
        Scene **sceneL;
        int sceneC, activeScene;
    } m_sceneInfo;
};

#endif // HPP_RENDERER
