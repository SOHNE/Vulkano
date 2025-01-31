#pragma once

#include "vulkano/vo_common.hpp"
#include "vulkano/vo_deviceContext.hpp"
#include "vulkano/vo_model.hpp"
#include "vulkano/vo_pipeline.hpp"
#include "vulkano/vo_shader.hpp"
#include "vulkano/vo_window.hpp"

#include <memory>

class Renderer
{
  public:
    struct Config
    {
        int width;
        int height;
        const char * title;
    };

    explicit Renderer( const Config & config, bool enableLayers = false );
    ~Renderer();

    bool Initialize();
    void Cleanup();

    void BeginFrame();
    void EndFrame();

    void DrawModel( voModel & model );
    void Resize( int width, int height );

    bool ShouldClose() const;
    void SetFramebufferResizeCallback( const std::function< void( int, int ) > & callback );
    void SetKeyCallback( const std::function< void( int, int, int, int ) > & callback );

  private:
    Config m_config;
    bool m_enableLayers;
    std::unique_ptr< voWindow > m_window;
    voDeviceContext m_deviceContext;
    voShader m_shader;
    voPipeline m_pipeline;

    bool CreatePipeline();
};
