#ifndef VULKANO_APPLICATION_H
#define VULKANO_APPLICATION_H

#include <cstdio>
#include <cstdlib>
#include <memory>

#include "vulkano/vulkano.hpp"

class Application
{
  public:
    Application() = default;
    ~Application();

    void Initialize();
    void MainLoop();

  private:
    void InitializeWindow();
    bool InitializeVulkan();
    void InitializeImGui();
    void Cleanup();

    void UpdateUniforms();
    void DrawFrame();

    void ResizeWindow( int windowWidth, int windowHeight );
    void Keyboard( int key, int scancode, int action, int modifiers );

  private:
    std::unique_ptr< voWindow > m_window { nullptr };

    voDeviceContext m_deviceContext;
    voShader m_triangleShader;
    voPipeline m_trianglePipeline;

    //	Model
    voModel m_modelTriangle;

    static const int WINDOW_WIDTH  = 800;
    static const int WINDOW_HEIGHT = 600;

    static const bool m_enableLayers = true;
};

#endif //VULKANO_APPLICATION_H
