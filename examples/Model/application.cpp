#include "application.hpp"

#include <cstdint>
#include <cstdio>

#include <cassert>
#include <ctime>
#include "cglm/affine-pre.h"
#include "cglm/mat4.h"
#include "cglm/types.h"
#include "cglm/vec2.h"

void
Application::Initialize()
{
    InitializeWindow();
    InitializeVulkan();

    glm_vec2_zero( m_mousePosition );
    m_cameraPositionTheta = acosf( -1.0f ) / 2.0f;
    m_cameraPositionPhi   = 0.0f;
    m_cameraRadius        = 15.0f;
    glm_vec3_zero( m_cameraFocusPoint );

    m_modelTriangle.LoadFromFile( "data/objs/scene.gltf", &m_deviceContext );
}

Application::~Application() { Cleanup(); }

void
Application::InitializeWindow()
{
    WindowConfig config;
    config.width  = WINDOW_WIDTH;
    config.height = WINDOW_HEIGHT;
    config.title  = "Vulkano Model";

    m_window = std::make_unique< voWindow >( config );

    // Set up callbacks
    m_window->setFramebufferResizeCallback( [&]( int width, int height ) -> void { ResizeWindow( width, height ); } );
    m_window->setKeyCallback( [&]( int key, int scancode, int action, int mods ) { Keyboard( key, scancode, action, mods ); } );
    m_window->setScrollCallback( [&]( double xoffset, double yoffset ) { MouseScrolled( (float)yoffset ); } );
    m_window->setCursorPosCallback( [&]( double xpos, double ypos ) { MouseMoved( xpos, ypos ); } );
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

    m_uniformBuffer.Allocate( &m_deviceContext, nullptr, sizeof( float ) * 16 * 4 * 128,
                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

    //	Full screen texture rendering
    {
        if( !m_triangleShader.Load( &m_deviceContext, "model" ) )
            {
                printf( "ERROR: Failed to load copy shader\n" );
                assert( 0 );
                return false;
            }

        voDescriptors::CreateParms_t descriptorParms {};
        memset( &descriptorParms, 0, sizeof( descriptorParms ) );
        descriptorParms.numUniformsVertex = 2;
        modelDescriptors.Create( &m_deviceContext, descriptorParms );

        voPipeline::CreateParms_t pipelineParms = {
            .renderPass  = m_deviceContext.swapChain.GetRenderPass(),
            .descriptors = &modelDescriptors,
            .shader      = &m_triangleShader,
            .width       = m_deviceContext.swapChain.GetWidth(),
            .height      = m_deviceContext.swapChain.GetHeight(),
            .cullMode    = voPipeline::CULL_MODE_BACK,
            .depthTest   = true,
            .depthWrite  = true,
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

    modelDescriptors.Cleanup( &m_deviceContext );
    m_uniformBuffer.Cleanup( &m_deviceContext );

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
            .renderPass  = m_deviceContext.swapChain.GetRenderPass(),
            .descriptors = &modelDescriptors,
            .shader      = &m_triangleShader,
            .width       = m_deviceContext.swapChain.GetWidth(),
            .height      = m_deviceContext.swapChain.GetHeight(),
            .cullMode    = voPipeline::CULL_MODE_BACK,
            .depthTest   = true,
            .depthWrite  = true,
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
Application::MouseScrolled( float z )
{
    m_cameraRadius -= z;
    if( m_cameraRadius < 0.5f )
        {
            m_cameraRadius = 0.5f;
        }
}

void
Application::MouseMoved( float x, float y )
{
    vec2 newPosition = { x, y };
    vec2 ds;
    glm_vec2_sub( newPosition, m_mousePosition, ds );
    glm_vec2_copy( newPosition, m_mousePosition );

    float sensitivity      = 0.01f;
    m_cameraPositionTheta += ds[1] * sensitivity;
    m_cameraPositionPhi   += ds[0] * sensitivity;

    if( m_cameraPositionTheta < 0.14f )
        {
            m_cameraPositionTheta = 0.14f;
        }
    if( m_cameraPositionTheta > 3.0f )
        {
            m_cameraPositionTheta = 3.0f;
        }
}

void
Application::Keyboard( int key, int scancode, int action, int modifiers )
{
    if( GLFW_KEY_ESCAPE == key && ( GLFW_PRESS == action ) )
        {
            m_window->close();
        }
}

void
Application::MainLoop()
{
    while( !m_window->ShouldClose() )
        {
            UpdateUniforms();

            // Draw the Scene
            DrawFrame();
        }
}

void
Application::UpdateUniforms()
{
    uint32_t uboByteOffset    = 0;
    uint32_t cameraByteOFfset = 0;
    uint32_t shadowByteOffset = 0;

    struct camera_t
    {
        mat4 matView;
        mat4 matProj;
        mat4 pad0;
        mat4 pad1;
    };
    camera_t camera {};

    {
        auto * mappedData = (unsigned char *)m_uniformBuffer.MapBuffer( &m_deviceContext );

        {
            vec3 camPos    = { 10, 0, 0 };
            vec3 camLookAt = { 0, 0, 0 };
            vec3 camUp     = { 0, 1, 0 };

            // Spherical coordinate calculation
            camPos[0] = cosf( m_cameraPositionPhi ) * sinf( m_cameraPositionTheta );
            camPos[1] = sinf( m_cameraPositionPhi ) * sinf( m_cameraPositionTheta );
            camPos[2] = cosf( m_cameraPositionTheta );

            // Scale by camera radius
            // glm_vec3_scale( camPos, m_cameraRadius, camPos );

            glm_vec3_add( camPos, m_cameraFocusPoint, camPos );
            glm_vec3_copy( m_cameraFocusPoint, camLookAt );

            int windowWidth  = m_window->getFramebufferSize().first;
            int windowHeight = m_window->getFramebufferSize().second;

            const float zNear  = 0.1f;
            const float zFar   = 1000.0f;
            const float fovy   = 45.0f;
            const float aspect = (float)windowWidth / (float)windowHeight; // Fixed aspect ratio

            glm_perspective( glm_rad( fovy ), aspect, zNear, zFar, camera.matProj );

            glm_lookat( camPos, camLookAt, camUp, camera.matView );

            memcpy( mappedData + uboByteOffset, &camera, sizeof( camera ) );

            cameraByteOFfset  = uboByteOffset;
            uboByteOffset    += m_deviceContext.GetAligendUniformByteOffset( sizeof( camera ) );
        }

        {
            Body & body = modelBody;

            // Create the transformation matrix
            mat4 matOrient;
            glm_mat4_identity( matOrient );

            // Convert body's quaternion orientation to rotation matrix
            mat4 rotationMatrix;
            glm_quat_mat4( body.orientation, rotationMatrix );

            // Combine identity matrix with body's rotation
            glm_mat4_mul( matOrient, rotationMatrix, matOrient );

            // Write to the mapped buffer
            memcpy( mappedData + uboByteOffset, matOrient, sizeof( matOrient ) );

            // Create render model
            voRenderModel renderModel {};
            renderModel.model         = &m_modelTriangle;
            renderModel.uboByteOffset = uboByteOffset;
            renderModel.uboByteSize   = sizeof( matOrient );

            // Copy body's position
            glm_vec3_copy( body.position, renderModel.pos );

            m_renderModels = renderModel;

            // Update offset for next iteration
            uboByteOffset += m_deviceContext.GetAligendUniformByteOffset( sizeof( matOrient ) );
        }

        m_uniformBuffer.UnmapBuffer( &m_deviceContext );
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
                const int camOffset = 0;
                const int camSize   = sizeof( float ) * 16 * 4;

                // Binding the pipeline - or "use shader"
                m_trianglePipeline.BindPipeline( cmdBuffer );

                voDescriptor descriptor = m_trianglePipeline.GetFreeDescriptor();
                descriptor.BindBuffer( &m_uniformBuffer, camOffset, camSize, 0 );                                       // bind the camera matrices
                descriptor.BindBuffer( &m_uniformBuffer, m_renderModels.uboByteOffset, m_renderModels.uboByteSize, 1 ); // bind the model matrices
                descriptor.BindDescriptor( &m_deviceContext, cmdBuffer, &m_trianglePipeline );
                m_renderModels.model->DrawIndexed( cmdBuffer );
            }
        }
        m_deviceContext.EndRenderPass();
    }
    //	End the render frame
    m_deviceContext.EndFrame();
}
