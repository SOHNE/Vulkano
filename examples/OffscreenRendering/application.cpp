#include <cstdint>
#include <cstdio>

#include <cassert>
#include <ctime>
#include "application.hpp"

#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"
#include "imgui.h"
#include "vulkano/vo_common.hpp"
#include "vulkano/vo_model.hpp"
#include "vulkano/vo_window.hpp"

static bool          gIsInitialized( false );
static unsigned long gTicksPerSecond;
static unsigned long gStartTicks;

int32_t
GetTimeMicroseconds()
{
    return 0;
}

void
Application::Initialize()
{
    InitializeGLFW();
    InitializeVulkan();
    InitializeImGui();

    m_bodies.emplace_back();

    m_models.reserve( m_bodies.size() );
    for( int i = 0; i < m_bodies.size(); i++ )
        {
            auto * model = new voModel();
            model->LoadFromFile( "data/objs/Froggs2.fbx", &m_deviceContext );

            m_models.push_back( model );
        }

    glm_vec2_zero( m_mousePosition );
    m_cameraPositionTheta = acosf( -1.0f ) / 2.0f;
    m_cameraPositionPhi   = 0.0f;
    m_cameraRadius        = 15.0f;
    glm_vec3_zero( m_cameraFocusPoint );

    m_isPaused  = true;
    m_stepFrame = false;
}

void
Application::InitializeImGui()
{
    {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO & io = ImGui::GetIO();
        VO_UNUSED( io );
        ImGui::StyleColorsDark();
    }

    // Create a separate descriptor pool
    {
        voDescriptors::CreateParms_t descriptorParms = {
            .numImageSamplers = 1,
        };

        m_imDescriptors.Create( &m_deviceContext, descriptorParms );
    }

    // Setup Platform/Renderer backends
    {
        ImGui_ImplGlfw_InitForVulkan( m_window->getHandle(), true );
        ImGui_ImplVulkan_InitInfo init_info = {
            .Instance       = m_deviceContext.instance,
            .PhysicalDevice = m_deviceContext.deviceInfo.physical,
            .Device         = m_deviceContext.deviceInfo.logical,
            .QueueFamily    = static_cast< uint32_t >( m_deviceContext.queueIds.graphicsFamily ),
            .Queue          = m_deviceContext.m_vkGraphicsQueue,
            .DescriptorPool = m_imDescriptors.vkDescriptorPool,
            .RenderPass     = m_deviceContext.swapChain.GetRenderPass(),
            .MinImageCount  = m_deviceContext.swapChain.GetColorImagesSize(),
            .ImageCount     = m_deviceContext.swapChain.GetColorImagesSize(),
            .MSAASamples    = VK_SAMPLE_COUNT_1_BIT,
            .PipelineCache  = VK_NULL_HANDLE,
            .Subpass        = 0,
            .Allocator      = VK_NULL_HANDLE,
        };
        ImGui_ImplVulkan_Init( &init_info );
    }
}

Application::~Application() { Cleanup(); }

void
Application::InitializeGLFW()
{
    WindowConfig config;
    config.width  = WINDOW_WIDTH;
    config.height = WINDOW_HEIGHT;
    config.title  = "Vulkano Offscreen";

    m_window = new voWindow( config );

    // Set up callbacks
    m_window->setFramebufferResizeCallback( [&]( int width, int height ) { ResizeWindow( width, height ); } );
    m_window->setCursorPosCallback( [&]( double xpos, double ypos ) { MouseMoved( xpos, ypos ); } );
    m_window->setScrollCallback( [&]( double xoffset, double yoffset ) { MouseScrolled( (float)yoffset ); } );
    m_window->setKeyCallback( [&]( int key, int scancode, int action, int mods ) { Keyboard( key, scancode, action, mods ); } );
    m_window->setCursorPosCallback( [&]( double xpos, double ypos ) { MouseMoved( xpos, ypos ); } );

    // Set input modes
    m_window->setFlag( voWindow::Flag::Resizable, true );
    m_window->setFlag( voWindow::Flag::MouseCaptured, true );
    m_window->setFlag( voWindow::Flag::StickyKeys, true );
    m_window->setFlag( voWindow::Flag::Decorated, true );
}

bool
Application::InitializeVulkan()
{
    //
    //	Vulkan Instance
    //
    {
        std::vector< const char * > extensions = voWindow::GetRequiredInstanceExtensions( m_enableLayers );
        if( !m_deviceContext.CreateInstance( m_enableLayers, extensions ) )
            {
                printf( "ERROR: Failed to create vulkan instance\n" );
                assert( 0 );
                return false;
            }
    }

    //
    //	Vulkan Surface for GLFW Window
    //
    if( !m_window->CreateSurface( m_deviceContext.instance, m_deviceContext.VkSurface ) )
        {
            printf( "ERROR: Failed to create window sruface\n" );
            voAssert( 0 );
            return false;
        }

    //
    //	Vulkan Device
    //
    if( !m_deviceContext.CreateDevice() )
        {
            printf( "ERROR: Failed to create device\n" );
            assert( 0 );
            return false;
        }

    //
    //	Create SwapChain
    //
    {
        auto [width, height] = m_window->getFramebufferSize();
        if( !m_deviceContext.CreateSwapChain( width, height ) )
            {
                printf( "ERROR: Failed to create swapchain\n" );
                assert( 0 );
                return false;
            }
    }

    //
    //	Initialize texture samplers
    //
    voSamplers::InitializeSamplers( &m_deviceContext );

    //
    //	Command Buffers
    //
    if( !m_deviceContext.CreateCommandBuffers() )
        {
            printf( "ERROR: Failed to create command buffers\n" );
            assert( 0 );
            return false;
        }

    //
    //	Uniform Buffer
    //
    m_uniformBuffer.Allocate( &m_deviceContext, nullptr, sizeof( float ) * 16 * 4 * 128,
                              VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT );

    //
    //	Offscreen rendering
    //
    InitOffscreen( &m_deviceContext, (int)m_deviceContext.swapChain.GetWidth(), (int)m_deviceContext.swapChain.GetHeight() );

    //
    //	Full screen texture rendering
    //
    {
        FillFullScreenQuad( m_modelFullScreen );
        for( auto & m_vertice : m_modelFullScreen.m_vertices )
            {
                m_vertice.pos[1] *= -1.0f;
            }
        m_modelFullScreen.MakeVBO( &m_deviceContext );

        if( !m_copyShader.Load( &m_deviceContext, "Image2D" ) )
            {
                printf( "ERROR: Failed to load copy shader\n" );
                assert( 0 );
                return false;
            }

        voDescriptors::CreateParms_t descriptorParms {
            .numUniformsFragment = 1,
            .numImageSamplers    = 1,
        };
        m_copyDescriptors.Create( &m_deviceContext, descriptorParms );

        voPipeline::CreateParms_t pipelineParms = {
            .renderPass  = m_deviceContext.swapChain.GetRenderPass(),
            .descriptors = &m_copyDescriptors,
            .shader      = &m_copyShader,
            .width       = m_deviceContext.swapChain.GetWidth(),
            .height      = m_deviceContext.swapChain.GetHeight(),
            .cullMode    = voPipeline::CULL_MODE_BACK,
            .depthTest   = false,
            .depthWrite  = false,
        };
        if( !m_copyPipeline.Create( &m_deviceContext, pipelineParms ) )
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

    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    CleanupOffscreen( &m_deviceContext );

    m_copyShader.Cleanup( &m_deviceContext );
    m_copyDescriptors.Cleanup( &m_deviceContext );
    m_copyPipeline.Cleanup( &m_deviceContext );
    m_modelFullScreen.Cleanup( m_deviceContext );
    m_imDescriptors.Cleanup( &m_deviceContext );

    // Delete models
    for( const auto & model : m_models )
        {
            model->Cleanup( m_deviceContext );
            delete model;
        }
    m_models.clear();
    m_bodies.clear();

    // Delete Uniform Buffer Memory
    m_uniformBuffer.Cleanup( &m_deviceContext );

    // Delete Samplers
    voSamplers::Cleanup( &m_deviceContext );

    // Delete Device Context
    m_deviceContext.Cleanup();

    // Delete GLFW
    delete m_window;
}

void
Application::ResizeWindow( int windowWidth, int windowHeight )
{
    m_deviceContext.ResizeWindow( windowWidth, windowHeight );

    Resize( &m_deviceContext, m_deviceContext.swapChain.GetWidth(), m_deviceContext.swapChain.GetHeight() );

    //	Resize full screen texture rendering
    {
        m_copyPipeline.Cleanup( &m_deviceContext );

        voPipeline::CreateParms_t pipelineParms = {
            .renderPass  = m_deviceContext.swapChain.GetRenderPass(),
            .descriptors = &m_copyDescriptors,
            .shader      = &m_copyShader,
            .width       = m_deviceContext.swapChain.GetWidth(),
            .height      = m_deviceContext.swapChain.GetHeight(),
            .cullMode    = voPipeline::CULL_MODE_BACK,
            .depthTest   = false,
            .depthWrite  = false,
        };
        if( !m_copyPipeline.Create( &m_deviceContext, pipelineParms ) )
            {
                printf( "Unable to build pipeline!\n" );
                assert( 0 );
                return;
            }
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

    m_cameraPositionTheta = glm_clamp( m_cameraPositionTheta, 0.14f, 3.0f );
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
Application::Keyboard( int key, int scancode, int action, int modifiers )
{
    if( GLFW_KEY_R == key && GLFW_RELEASE == action )
        {
            //m_scene->Reset();
        }
    if( GLFW_KEY_T == key && GLFW_RELEASE == action )
        {
            m_isPaused = !m_isPaused;
        }
    if( GLFW_KEY_Y == key && ( GLFW_PRESS == action || GLFW_REPEAT == action ) )
        {
            m_stepFrame = m_isPaused && !m_stepFrame;
        }
    if( GLFW_KEY_UP == key && ( GLFW_PRESS == action || GLFW_REPEAT == action ) )
        {
            vec3 delta = { 0, 0, 1 };
            glm_vec3_add( m_bodies[0].m_position, delta, m_bodies[0].m_position );
        }
    if( GLFW_KEY_DOWN == key && ( GLFW_PRESS == action || GLFW_REPEAT == action ) )
        {
            vec3 delta = { 0, 0, -1 };
            glm_vec3_add( m_bodies[0].m_position, delta, m_bodies[0].m_position );
        }
    if( GLFW_KEY_LEFT == key && ( GLFW_PRESS == action || GLFW_REPEAT == action ) )
        {
            vec3 delta = { 1, 0, 0 };
            glm_vec3_add( m_bodies[0].m_position, delta, m_bodies[0].m_position );
        }
    if( GLFW_KEY_RIGHT == key && ( GLFW_PRESS == action || GLFW_REPEAT == action ) )
        {
            vec3 delta = { -1, 0, 0 };
            glm_vec3_add( m_bodies[0].m_position, delta, m_bodies[0].m_position );
        }
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
            // Update Shader uniforms
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
            vec3 camPos    = { 10, 0, 5 };
            vec3 camLookAt = { 0, 0, 0 };
            vec3 camUp     = { 0, 0, 1 };

            // Spherical coordinate calculation
            camPos[0] = cosf( m_cameraPositionPhi ) * sinf( m_cameraPositionTheta );
            camPos[1] = sinf( m_cameraPositionPhi ) * sinf( m_cameraPositionTheta );
            camPos[2] = cosf( m_cameraPositionTheta );

            // Scale by camera radius
            glm_vec3_scale( camPos, m_cameraRadius, camPos );

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
            vec3 camLookAt = { 0, 0, 0 };
            vec3 camUp     = { 0, 0, 1 };
            vec3 tmp;
            glm_vec3_cross( camPos, camUp, tmp );
            glm_vec3_cross( tmp, camPos, camUp );
            glm_vec3_normalize( camUp );

            extern voFrameBuffer g_shadowFrameBuffer;
            const int            windowWidth  = g_shadowFrameBuffer.parms.width;
            const int            windowHeight = g_shadowFrameBuffer.parms.height;

            const float halfWidth = 60.0f;
            const float xmin      = -halfWidth;
            const float xmax      = halfWidth;
            const float ymin      = -halfWidth;
            const float ymax      = halfWidth;
            const float zNear     = 25.0f;
            const float zFar      = 175.0f;

            glm_ortho( xmin, xmax, ymin, ymax, zNear, zFar, camera.matProj );
            glm_mat4_transpose( camera.matProj );

            glm_lookat( camPos, camLookAt, camUp, camera.matView );
            glm_mat4_transpose( camera.matView );

            memcpy( mappedData + uboByteOffset, &camera, sizeof( camera ) );

            shadowByteOffset  = uboByteOffset;
            uboByteOffset    += m_deviceContext.GetAligendUniformByteOffset( sizeof( camera ) );
        }

        m_renderModels.clear();
        for( int i = 0; i < m_bodies.size(); i++ )
            {
                Body & body = m_bodies[i];

                // Create the transformation matrix properly
                mat4 matOrient;
                glm_mat4_identity( matOrient );

                // Write to the mapped buffer
                memcpy( mappedData + uboByteOffset, matOrient, sizeof( matOrient ) );

                // Create render model
                voRenderModel renderModel {};
                renderModel.model         = m_models[i];
                renderModel.uboByteOffset = uboByteOffset;
                renderModel.uboByteSize   = sizeof( matOrient );
                glm_vec3_copy( body.m_position, renderModel.pos );
                m_renderModels.push_back( renderModel );

                // Update offset for next iteration
                uboByteOffset += m_deviceContext.GetAligendUniformByteOffset( sizeof( matOrient ) );
            }

        m_uniformBuffer.UnmapBuffer( &m_deviceContext );
    }
}

void
Application::DrawFrame()
{
    //
    //	Begin the render frame
    //
    const uint32_t imageIndex = m_deviceContext.BeginFrame();

    // Draw everything in an offscreen buffer
    DrawOffscreen( &m_deviceContext, imageIndex, &m_uniformBuffer, m_renderModels.data(), (int)m_renderModels.size() );

    //
    //	Draw the offscreen framebuffer to the swap chain frame buffer
    //
    m_deviceContext.BeginRenderPass();
    {
        VkCommandBuffer cmdBuffer = m_deviceContext.m_vkCommandBuffers[imageIndex];

        {
            extern voFrameBuffer g_offscreenFrameBuffer;

            // Binding the pipeline is effectively the "use shader" we had back in our opengl apps
            m_copyPipeline.BindPipeline( cmdBuffer );

            // Descriptor is how we bind our buffers and images
            voDescriptor descriptor = m_copyPipeline.GetFreeDescriptor();
            descriptor.BindImage( VK_IMAGE_LAYOUT_GENERAL, g_offscreenFrameBuffer.imageColor.vkImageView,
                                  voSamplers::m_samplerStandard, 0 );
            descriptor.BindDescriptor( &m_deviceContext, cmdBuffer, &m_copyPipeline );
            m_modelFullScreen.DrawIndexed( cmdBuffer );
        }

        //
        // ImGui
        //
        {
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin( "Directional light pos" );

            ImGui::End();

            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData( ImGui::GetDrawData(), cmdBuffer );
        }
    }
    m_deviceContext.EndRenderPass();

    //
    //	End the render frame
    //
    m_deviceContext.EndFrame();
}
