#ifndef VULKANO_APPLICATION_H
#define VULKANO_APPLICATION_H

#include <cstdio>
#include <cstdlib>
#include <memory>

#include "cglm/cglm.h"
#include "cglm/quat.h"
#include "vulkano/vulkano.hpp"

struct Body
{
    // Quaternion first (4x float = 16 bytes)
    versor orientation { 0.0f, 0.0f, 0.0f, 1.0f };

    // Position next (3x float = 12 bytes, with 4 bytes padding)
    vec3 position { 0.0f, 0.0f, 0.0f };
};

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

    void MouseScrolled( float z );
    void MouseMoved( float x, float y );

  private:
    std::unique_ptr< voWindow > m_window { nullptr };

    voDeviceContext m_deviceContext;
    voShader        m_triangleShader;
    voPipeline      m_trianglePipeline;

    voBuffer m_uniformBuffer;

    // User input
    vec2  m_mousePosition;
    vec3  m_cameraFocusPoint;
    float m_cameraPositionTheta;
    float m_cameraPositionPhi;
    float m_cameraRadius;

    voRenderModel m_renderModels;

    voDescriptors modelDescriptors;

    Body modelBody {
        .orientation = { 0.7071f, 0.0f, 0.0f, 0.7071f }, // 90-degree rotation around X-axis
        .position    = { 0.0f, 0.0f, 0.0f }
    };

    vec3 camPos;

    //	Model
    voModel m_modelTriangle;

    static const int WINDOW_WIDTH  = 800;
    static const int WINDOW_HEIGHT = 600;

    static const bool m_enableLayers = true;
};

#endif //VULKANO_APPLICATION_H
