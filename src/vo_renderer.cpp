#include "vulkano/vo_renderer.hpp"
#include <cassert>
#include <cstdio>

Renderer::Renderer( const Config & config, bool enableLayers )
    : m_config( config ), m_enableLayers( enableLayers ) {}

Renderer::~Renderer()
{
    Cleanup();
}

bool
Renderer::Initialize()
{
    // Initialize window
    m_window = std::make_unique< voWindow >( WindowConfig { m_config.width, m_config.height, m_config.title } );

    // Initialize Vulkan instance
    std::vector< const char * > extensions = voWindow::GetRequiredInstanceExtensions( m_enableLayers );
    if( !m_deviceContext.CreateInstance( m_enableLayers, extensions ) )
        {
            printf( "ERROR: Failed to create Vulkan instance\n" );
            return false;
        }

    // Create surface
    if( !m_window->CreateSurface( m_deviceContext.instance, m_deviceContext.VkSurface ) )
        {
            printf( "ERROR: Failed to create window surface\n" );
            return false;
        }

    // Create device
    if( !m_deviceContext.CreateDevice() )
        {
            printf( "ERROR: Failed to create device\n" );
            return false;
        }

    // Create swapchain
    auto [width, height] = m_window->getFramebufferSize();
    if( !m_deviceContext.CreateSwapChain( width, height ) )
        {
            printf( "ERROR: Failed to create swapchain\n" );
            return false;
        }

    // Load shader and create pipeline
    if( !m_shader.Load( &m_deviceContext, "triangle" ) )
        {
            printf( "ERROR: Failed to load shader\n" );
            return false;
        }

    if( !CreatePipeline() )
        {
            return false;
        }

    return true;
}

bool
Renderer::CreatePipeline()
{
    voPipeline::CreateParms_t pipelineParms = {
        .renderPass = m_deviceContext.swapChain.GetRenderPass(),
        .shader     = &m_shader,
        .width      = m_deviceContext.swapChain.GetWidth(),
        .height     = m_deviceContext.swapChain.GetHeight(),
        .cullMode   = voPipeline::CULL_MODE_BACK,
        .depthTest  = false,
        .depthWrite = false,
    };

    if( !m_pipeline.Create( &m_deviceContext, pipelineParms ) )
        {
            printf( "ERROR: Failed to create pipeline\n" );
            return false;
        }

    return true;
}

void
Renderer::Cleanup()
{
    vkDeviceWaitIdle( m_deviceContext.deviceInfo.logical );

    m_shader.Cleanup( &m_deviceContext );
    m_pipeline.Cleanup( &m_deviceContext );

    m_deviceContext.Cleanup();
}

void
Renderer::BeginFrame()
{
    m_deviceContext.BeginFrame();
    m_deviceContext.BeginRenderPass();
}

void
Renderer::EndFrame()
{
    m_deviceContext.EndRenderPass();
    m_deviceContext.EndFrame();
}

void
Renderer::DrawModel( voModel & model )
{
    const uint32_t imageIndex = m_deviceContext.BeginFrame();
    VkCommandBuffer cmdBuffer = m_deviceContext.m_vkCommandBuffers[imageIndex];
    m_pipeline.BindPipeline( cmdBuffer );
    model.DrawIndexed( cmdBuffer );
}

void
Renderer::Resize( int width, int height )
{
    m_deviceContext.ResizeWindow( width, height );
    m_pipeline.Cleanup( &m_deviceContext );
    CreatePipeline();
}

bool
Renderer::ShouldClose() const
{
    return m_window->ShouldClose();
}

void
Renderer::SetFramebufferResizeCallback( const std::function< void( int, int ) > & callback )
{
    m_window->setFramebufferResizeCallback( callback );
}

void
Renderer::SetKeyCallback( const std::function< void( int, int, int, int ) > & callback )
{
    m_window->setKeyCallback( callback );
}
