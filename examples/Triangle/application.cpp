#include "application.hpp"

#include <cstdint>
#include <cstdio>

#include <cassert>
#include <ctime>

#include "vulkano/vo_common.hpp"
#include "vulkano/vo_window.hpp"

void
Application::Initialize()
{
    InitializeWindow();
    InitializeVulkan();
}

Application::~Application() { Cleanup(); }

void
Application::InitializeWindow()
{
    WindowConfig config;
    config.width  = WINDOW_WIDTH;
    config.height = WINDOW_HEIGHT;
    config.title  = "Vulkano Triangle";

    m_window = std::make_unique< voWindow >( config );

    // Set up callbacks
    m_window->setFramebufferResizeCallback( [&]( int width, int height ) -> void { ResizeWindow( width, height ); } );
}

bool
Application::InitializeVulkan()
{
    //	Vulkan Instance
    {
        std::vector< const char * > extensions = voWindow::GetRequiredInstanceExtensions( m_enableLayers );
        if( !m_deviceContext.CreateInstance( m_enableLayers, extensions ) )
            {
                printf( "ERROR: Failed to create vulkan instance\n" );
                assert( 0 );
                return false;
            }
    }

    //	Vulkan Surface for GLFW Window
    if( !m_window->CreateSurface( m_deviceContext.instance, m_deviceContext.VkSurface ) )
        {
            printf( "ERROR: Failed to create window sruface\n" );
            voAssert( 0 );
            return false;
        }

    //	Vulkan Device
    if( !m_deviceContext.CreateDevice() )
        {
            printf( "ERROR: Failed to create device\n" );
            assert( 0 );
            return false;
        }

    //	Create SwapChain
    {
        auto [width, height] = m_window->getFramebufferSize();
        if( !m_deviceContext.CreateSwapChain( width, height ) )
            {
                printf( "ERROR: Failed to create swapchain\n" );
                assert( 0 );
                return false;
            }
    }

    //	Command Buffers
    if( !m_deviceContext.CreateCommandBuffers() )
        {
            printf( "ERROR: Failed to create command buffers\n" );
            assert( 0 );
            return false;
        }

    //	Full screen texture rendering
    {
        FillTriangle( m_modelTriangle );
        m_modelTriangle.MakeVBO( &m_deviceContext );

        if( !m_triangleShader.Load( &m_deviceContext, "triangle" ) )
            {
                printf( "ERROR: Failed to load copy shader\n" );
                assert( 0 );
                return false;
            }

        voPipeline::CreateParms_t pipelineParms = {
            .renderPass = m_deviceContext.swapChain.GetRenderPass(),
            .shader     = &m_triangleShader,
            .width      = m_deviceContext.swapChain.GetWidth(),
            .height     = m_deviceContext.swapChain.GetHeight(),
            .cullMode   = voPipeline::CULL_MODE_BACK,
            .depthTest  = false,
            .depthWrite = false,
        };
        if( !m_trianglePipeline.Create( &m_deviceContext, pipelineParms ) )
            {
                printf( "ERROR: Failed to create copy pipeline\n" );
                assert( 0 );
                return false;
            }
    }

    return true;
}

void
Application::Cleanup()
{
    vkDeviceWaitIdle( m_deviceContext.deviceInfo.logical );

    m_triangleShader.Cleanup( &m_deviceContext );
    m_trianglePipeline.Cleanup( &m_deviceContext );
    m_modelTriangle.Cleanup( m_deviceContext );

    // Delete Device Context
    m_deviceContext.Cleanup();

    // Delete GLFW
    m_window.reset();
}

void
Application::ResizeWindow( int windowWidth, int windowHeight )
{
    m_deviceContext.ResizeWindow( windowWidth, windowHeight );

    //	Resize full screen texture rendering
    {
        m_trianglePipeline.Cleanup( &m_deviceContext );

        voPipeline::CreateParms_t pipelineParms = {
            .renderPass = m_deviceContext.swapChain.GetRenderPass(),
            .shader     = &m_triangleShader,
            .width      = m_deviceContext.swapChain.GetWidth(),
            .height     = m_deviceContext.swapChain.GetHeight(),
            .cullMode   = voPipeline::CULL_MODE_BACK,
            .depthTest  = false,
            .depthWrite = false,
        };
        if( !m_trianglePipeline.Create( &m_deviceContext, pipelineParms ) )
            {
                printf( "Unable to build pipeline!\n" );
                assert( 0 );
                return;
            }
    }
}

void
Application::MainLoop()
{
    while( !m_window->ShouldClose() )
        {
            // Draw the Scene
            DrawFrame();
        }
}

void
Application::DrawFrame()
{
    //	Begin the render frame
    const uint32_t imageIndex = m_deviceContext.BeginFrame();
    {
        //	Draw the offscreen framebuffer to the swap chain frame buffer
        m_deviceContext.BeginRenderPass();
        {
            VkCommandBuffer cmdBuffer = m_deviceContext.m_vkCommandBuffers[imageIndex];

            {
                // Binding the pipeline - or "use shader"
                m_trianglePipeline.BindPipeline( cmdBuffer );

                m_modelTriangle.DrawIndexed( cmdBuffer );
            }
        }
        m_deviceContext.EndRenderPass();
    }
    //	End the render frame
    m_deviceContext.EndFrame();
}
