#ifndef VULKANO_APPLICATION_H
#define VULKANO_APPLICATION_H

#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

#define CGLM_ALL_UNALIGNED
#include <cglm/call.h>
#include <cglm/cglm.h>

#include "offscreenRendering.hpp"
#include "vulkano/vulkano.hpp"

class Body
{
  public:
    Body()
    {
        glm_vec3_zero( m_position );
        glm_quat_identity( m_orientation );
        m_shape = NULL;
    }

    vec3   m_position;
    versor m_orientation;

    class voShape * m_shape;
};

class Application
{
  public:
    Application()
        : m_isPaused( true ), m_stepFrame( false )
    {
        glm_vec2_zero( m_mousePosition );
        glm_vec3_zero( m_cameraFocusPoint );

        m_cameraPositionTheta = 0.0f;
        m_cameraPositionPhi   = 0.0f;
        m_cameraRadius        = 0.0f;

        glm_vec3_fill( camPos, 75.0f );
    }
    ~Application();

    void Initialize();
    void MainLoop();

  private:
    void InitializeGLFW();
    bool InitializeVulkan();
    void InitializeImGui();
    void Cleanup();

    void UpdateUniforms();
    void DrawFrame();

    void ResizeWindow( int windowWidth, int windowHeight );
    void MouseMoved( float x, float y );
    void MouseScrolled( float z );
    void Keyboard( int key, int scancode, int action, int modifiers );

  private:
    voWindow * m_window;

    voDeviceContext m_deviceContext;

    voBuffer m_uniformBuffer;

    voModel                  m_modelFullScreen;
    std::vector< voModel * > m_models;

    voShader      m_copyShader;
    voDescriptors m_copyDescriptors;
    voPipeline    m_copyPipeline;

    voDescriptors m_imDescriptors;

    std::vector< Body > m_bodies;

    // User input
    vec2  m_mousePosition;
    vec3  m_cameraFocusPoint;
    float m_cameraPositionTheta;
    float m_cameraPositionPhi;
    float m_cameraRadius;
    bool  m_isPaused;
    bool  m_stepFrame;

    float dt_us { 0 };

    std::array< float, 50 > frameTimes {};
    float                   frameTimeMin = 9999.0f, frameTimeMax = 0.0f;

    vec3 camPos;

    std::vector< voRenderModel > m_renderModels;

    static const int WINDOW_WIDTH  = 800;
    static const int WINDOW_HEIGHT = 600;

    static const bool m_enableLayers = true;
};

extern Application * g_application;

#endif //VULKANO_APPLICATION_H
