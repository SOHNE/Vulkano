#include "vulkano/vo_window.hpp"

#include <stdexcept>

voWindow::voWindow( const WindowConfig & config )
    : m_config( config )
{
    try
        {
            spdlog::info( "-------------------------------------------------" );
            spdlog::info( "\\\\//, Vulkano version: {} ({}.{:03}.{:03})", VULKANO_VERSION, VULKANO_VERSION_MAJOR, VULKANO_VERSION_MINOR, VULKANO_VERSION_PATCH );
            spdlog::info( "-------------------------------------------------" );

            spdlog::info( "Initializing GLFW..." );
            if( !glfwInit() )
                {
                    throw std::runtime_error( "Failed to initialize GLFW" );
                }

            spdlog::info( "Checking Vulkan support..." );
            if( !glfwVulkanSupported() )
                {
                    glfwTerminate();
                    throw std::runtime_error( "Vulkan is not supported" );
                }

            configureWindowHints();

            GLFWmonitor * monitor = hasFlag( Flag::Fullscreen ) ? glfwGetPrimaryMonitor() : nullptr;
            spdlog::info( "Creating window..." );
            m_window = glfwCreateWindow( config.width, config.height, config.title.c_str(), monitor, nullptr );

            if( !m_window )
                {
                    glfwTerminate();
                    throw std::runtime_error( "Failed to create GLFW window" );
                }

            spdlog::info( "Window created successfully" );

            glfwSetWindowUserPointer( m_window, this );
            setupCallbacks();
            applyFlags( m_flags );

glfwSwapInterval( 0 );

            spdlog::info( "" );
        }
    catch( const std::exception & e )
        {
            spdlog::error( "Error: {}", e.what() );
            throw;
        }
}

voWindow::voWindow( voWindow && other ) noexcept
    : m_window( other.m_window )
    , m_config( std::move( other.m_config ) )
    , m_flags( other.m_flags )
{
    other.m_window = nullptr;
    if( m_window )
        {
            glfwSetWindowUserPointer( m_window, this );
        }
}

voWindow &
voWindow::operator=( voWindow && other ) noexcept
{
    if( this != &other )
        {
            cleanup();
            m_window       = other.m_window;
            m_config       = std::move( other.m_config );
            m_flags        = other.m_flags;
            other.m_window = nullptr;
            if( m_window )
                {
                    glfwSetWindowUserPointer( m_window, this );
                }
        }
    return *this;
}

voWindow::~voWindow() { cleanup(); }

void
voWindow::toggleFullscreen()
{
    setFlag( Flag::Fullscreen, !hasFlag( Flag::Fullscreen ) );
    GLFWmonitor * monitor = hasFlag( Flag::Fullscreen ) ? glfwGetPrimaryMonitor() : nullptr;

    if( monitor )
        {
            const GLFWvidmode * mode = glfwGetVideoMode( monitor );
            glfwSetWindowMonitor( m_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate );
        }
    else
        {
            glfwSetWindowMonitor( m_window, nullptr, 100, 100, m_config.width, m_config.height, 0 );
        }
}

void
voWindow::setMouseMode( Flag mouseFlag )
{
    switch( mouseFlag )
        {
            case Flag::MouseRaw:
                glfwSetInputMode( m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE );
                break;
            case Flag::MouseVisible:
                glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL );
                break;
            case Flag::MouseCaptured:
                glfwSetInputMode( m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
                break;
            default:
                // Optionally handle an unknown or unexpected flag value
                break;
        }
    setFlag( mouseFlag );
}

bool
voWindow::CreateSurface( VkInstance instance, VkSurfaceKHR & surface ) const
{
    VK_CHECK( glfwCreateWindowSurface( instance, m_window, nullptr, &surface ),
              "Failed to create window surface for GLFW window '{}'",
              glfwGetWindowTitle( m_window ) );
    return true;
}

std::vector< const char * >
voWindow::GetRequiredInstanceExtensions( bool enableValidationLayers )
{
    uint32_t glfwExtensionCount  = 0;
    const char ** glfwExtensions = glfwGetRequiredInstanceExtensions( &glfwExtensionCount );
    std::vector< const char * > extensions( glfwExtensions, glfwExtensions + glfwExtensionCount );

    if( enableValidationLayers )
        {
            extensions.push_back( VK_EXT_DEBUG_REPORT_EXTENSION_NAME );
            extensions.push_back( VK_EXT_DEBUG_UTILS_EXTENSION_NAME );
        }

    return extensions;
}

void
voWindow::configureWindowHints() const
{
#define WINDOW_HINT( hint, value ) glfwWindowHint( hint, value )
    WINDOW_HINT( GLFW_CLIENT_API, GLFW_NO_API );
    WINDOW_HINT( GLFW_RESIZABLE, hasFlag( Flag::Resizable ) ? GLFW_TRUE : GLFW_FALSE );
    WINDOW_HINT( GLFW_DECORATED, hasFlag( Flag::Decorated ) ? GLFW_TRUE : GLFW_FALSE );
#undef WINDOW_HINT
}

void
voWindow::applyFlags( Flag flags )
{
    m_flags = flags;

#define SET_INPUT_MODE( window, mode, value ) glfwSetInputMode( window, mode, value )
    // Set raw mouse motion
    if( hasFlag( Flag::MouseRaw ) )
        {
            SET_INPUT_MODE( m_window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE );
        }

    // Set mouse visibility
    if( !hasFlag( Flag::MouseVisible ) )
        {
            SET_INPUT_MODE( m_window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN );
        }

    // Set mouse capture
    if( hasFlag( Flag::MouseCaptured ) )
        {
            SET_INPUT_MODE( m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED );
        }

    // Set VSync
    glfwSwapInterval( hasFlag( Flag::VSync ) ? 1 : 0 );

    // Set sticky keys
    SET_INPUT_MODE( m_window, GLFW_STICKY_KEYS, hasFlag( Flag::StickyKeys ) ? GLFW_TRUE : GLFW_FALSE );

    SET_INPUT_MODE( m_window, GLFW_DECORATED, hasFlag( Flag::Decorated ) ? GLFW_TRUE : GLFW_FALSE );
#undef SET_INPUT_MODE
}

void
voWindow::setupCallbacks()
{
    glfwSetKeyCallback( m_window, []( GLFWwindow * window, int key, int scancode, int action, int mods ) {
        auto * self = static_cast< voWindow * >( glfwGetWindowUserPointer( window ) );
        if( self && self->m_keyCallback )
            {
                self->m_keyCallback( key, scancode, action, mods );
            }
    } );

    glfwSetMouseButtonCallback( m_window, []( GLFWwindow * window, int button, int action, int mods ) {
        auto * self = static_cast< voWindow * >( glfwGetWindowUserPointer( window ) );
        if( self && self->m_mouseButtonCallback )
            {
                self->m_mouseButtonCallback( button, action, mods );
            }
    } );

    glfwSetCursorPosCallback( m_window, []( GLFWwindow * window, double xpos, double ypos ) {
        auto * self = static_cast< voWindow * >( glfwGetWindowUserPointer( window ) );
        if( self && self->m_cursorPosCallback )
            {
                self->m_cursorPosCallback( xpos, ypos );
            }
    } );

    glfwSetScrollCallback( m_window, []( GLFWwindow * window, double xoffset, double yoffset ) {
        auto * self = static_cast< voWindow * >( glfwGetWindowUserPointer( window ) );
        if( self && self->m_scrollCallback )
            {
                self->m_scrollCallback( xoffset, yoffset );
            }
    } );

    glfwSetFramebufferSizeCallback( m_window, []( GLFWwindow * window, int width, int height ) {
        auto * self = static_cast< voWindow * >( glfwGetWindowUserPointer( window ) );
        if( self && self->m_framebufferResizeCallback )
            {
                self->m_framebufferResizeCallback( width, height );
            }
    } );
}

void
voWindow::cleanup() noexcept
{
    if( m_window )
        {
            glfwDestroyWindow( m_window );
            m_window = nullptr;
        }
    glfwTerminate();
}
