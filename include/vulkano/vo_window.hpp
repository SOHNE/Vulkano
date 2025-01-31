#ifndef VULKANO_WINDOW_HPP
#define VULKANO_WINDOW_HPP

#include "vo_common.hpp"

#include <GLFW/glfw3.h>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// ======================================================================================================================
// Structures
// ======================================================================================================================

/**
 * @struct WindowConfig
 * @brief Configuration structure for setting up a `voWindow`.
 *
 * @details `WindowConfig` provides parameters to initialize a `voWindow` with specific attributes such as width, height, title, and debug settings.
 * This structure can be customized before passing it to the `voWindow` constructor, allowing for flexibility in setting up the window dimensions, title, and validation layer requirements.
 */
struct WindowConfig
{
    int width;                   ///< The initial width of the window in pixels.
    int height;                  ///< The initial height of the window in pixels.
    std::string title;           ///< The title displayed on the window.
    bool enableValidationLayers; ///< Enables Vulkan validation layers if true, useful for debugging.
};

/**
 * @class voWindow
 * @brief A class that manages a window, its properties, and events.
 *
 * @details The `voWindow` class provides a framework to create and manage a window with customizable flags for resizing, decoration, fullscreen, and various mouse and keyboard features.
 * It handles Vulkan surface creation, input callbacks, and window flags management.
 * This class offers inline and static methods for ease of access to window state, while allowing for efficient handling of user inputs and window resize events.
 *
 * @code
 * WindowConfig config;
 * voWindow window(config);
 * 
 * window.setFlag(voWindow::Flag::Resizable, true);
 * window.setMouseMode(voWindow::Flag::MouseCaptured);
 *
 * // Setup a key callback
 * window.setKeyCallback([](int key, int scancode, int action, int mods) {
 *     // Handle key events
 * });
 *
 * while (!window.shouldClose()) {
 *     voWindow::pollEvents();
 *     // Render loop logic
 * }
 *
 * window.close();
 * @endcode
 *
 * @see `WindowConfig`, `VkInstance`, `VkSurfaceKHR`, `GLFWwindow`, `glfwPollEvents`, `glfwCreateWindow`
 */
class voWindow
{
  public:
    /* ====================================== Enums and Operators ====================================================== */

    /**
     * @enum Flag
     * @brief Flags for window properties and behavior.
     *
     * @details The `Flag` enum defines various properties and behaviors of the window. Flags can be combined using bitwise operators to configure the window state.
     */
    enum class Flag : uint32_t
    {
        Resizable     = 1 << 0, ///< Allows the window to be resizable.
        Decorated     = 1 << 1, ///< Adds window decoration.
        Fullscreen    = 1 << 2, ///< Sets the window to fullscreen mode.
        VSync         = 1 << 3, ///< Enables vertical synchronization.
        MouseRaw      = 1 << 4, ///< Enables raw mouse input.
        MouseVisible  = 1 << 5, ///< Shows the mouse cursor.
        MouseCaptured = 1 << 6, ///< Captures the mouse within the window.
        StickyKeys    = 1 << 7  ///< Enables sticky keys functionality.
    };

    /**
     * @brief Bitwise OR operator for combining `Flag` values.
     * @param a The first flag.
     * @param b The second flag.
     * @return Combined `Flag` value with `a | b`.
     */
    friend constexpr Flag
    operator|( Flag a, Flag b )
    {
        return static_cast< Flag >( static_cast< uint32_t >( a ) | static_cast< uint32_t >( b ) );
    }

    /**
     * @brief Bitwise AND operator for checking common `Flag` values.
     * @param a The first flag.
     * @param b The second flag.
     * @return Combined `Flag` value with `a & b`.
     */
    friend constexpr Flag
    operator&( Flag a, Flag b )
    {
        return static_cast< Flag >( static_cast< uint32_t >( a ) & static_cast< uint32_t >( b ) );
    }

    /**
     * @brief Bitwise NOT operator for inverting `Flag` values.
     * @param a The flag to invert.
     * @return Inverted `Flag` value.
     */
    friend constexpr Flag
    operator~( Flag a )
    {
        return static_cast< Flag >( ~static_cast< uint32_t >( a ) );
    }

    /* ====================================== Type Definitions ========================================================= */

    using KeyCallback               = std::function< void( int key, int scancode, int action, int mods ) >;
    using MouseButtonCallback       = std::function< void( int button, int action, int mods ) >;
    using CursorPosCallback         = std::function< void( double xpos, double ypos ) >;
    using ScrollCallback            = std::function< void( double xoffset, double yoffset ) >;
    using FramebufferResizeCallback = std::function< void( int width, int height ) >;

    /* ====================================== Constructors & Destructor ================================================ */

    /**
     * @brief Constructs a `voWindow` with the specified configuration.
     * @param config The window configuration.
     */
    explicit voWindow( const WindowConfig & config = WindowConfig {} );

    /**
     * @brief Destructor for `voWindow`.
     */
    ~voWindow();

    /**
     * @brief Move constructor.
     */
    voWindow( voWindow && other ) noexcept;

    /**
     * @brief Move assignment operator.
     */
    voWindow & operator=( voWindow && other ) noexcept;

    /**
     * @brief Deleted copy constructor.
     */
    voWindow( const voWindow & ) = delete;

    /**
     * @brief Deleted copy assignment operator.
     */
    voWindow & operator=( const voWindow & ) = delete;

    /* ====================================== Public Methods =========================================================== */

    /**
     * @brief Toggles the fullscreen state of the window.
     */
    void toggleFullscreen();

    /**
     * @brief Sets the mouse mode based on the specified `Flag`.
     * @param mouseFlag The `Flag` to configure the mouse mode.
     */
    void setMouseMode( Flag mouseFlag );

    /**
     * @brief Creates a Vulkan surface for the window.
     * @param instance The Vulkan instance.
     * @return A Vulkan surface handle, or `std::nullopt` if creation failed.
     */
    bool CreateSurface( VkInstance instance, VkSurfaceKHR & surface ) const;

    /**
     * @brief Retrieves the required instance extensions.
     * @param enableValidationLayers Enables validation layers if true.
     * @return A vector of required instance extensions.
     */
    [[nodiscard]] static std::vector< const char * > GetRequiredInstanceExtensions( bool enableValidationLayers );

    /**
     * @brief Checks if a physical device supports presentation to the window.
     * @param instance The Vulkan instance.
     * @param device The physical device.
     * @param queueFamilyIndex The queue family index.
     * @return True if the device supports presentation, false otherwise.
     */
    [[nodiscard]] static bool checkPhysicalDevicePresentationSupport( VkInstance instance, VkPhysicalDevice device, uint32_t queueFamilyIndex );

    /* ====================================== Inline Methods =========================================================== */

    /**
     * @brief Sets or clears a window flag.
     * @param flag The `Flag` to set or clear.
     * @param value True to set the flag, false to clear.
     */
    FORCE_INLINE void setFlag( Flag flag, bool value = true );

    /**
     * @brief Checks if the window has a specific flag.
     * @param flag The `Flag` to check.
     * @return True if the flag is set, false otherwise.
     */
    [[nodiscard]] FORCE_INLINE bool hasFlag( Flag flag ) const;

    /**
     * @brief Retrieves the internal GLFW window handle.
     * @return A pointer to the `GLFWwindow`.
     */
    [[nodiscard]] FORCE_INLINE GLFWwindow * getHandle() const noexcept;

    /**
     * @brief Checks if the window should close.
     * @return True if the window should close, false otherwise.
     */
    [[nodiscard]] FORCE_INLINE bool ShouldClose() const noexcept;

    /**
     * @brief Sets the window's "should close" status.
     * @param value True to close the window, false otherwise.
     */
    FORCE_INLINE void setShouldClose( bool value ) noexcept;

    /**
     * @brief Gets the framebuffer size.
     * @return A pair containing the width and height of the framebuffer.
     */
    [[nodiscard]] FORCE_INLINE std::pair< int, int > getFramebufferSize() const;

    /**
     * @brief Closes the window.
     */
    FORCE_INLINE void close() noexcept;

    /**
     * @brief Processes all pending events.
     */
    FORCE_INLINE static void pollEvents() noexcept;

    /**
     * @brief Waits for events.
     */
    FORCE_INLINE static void waitEvents() noexcept;

    /* ====================================== Callback Methods ========================================================= */

    /**
     * @brief Sets the key event callback.
     * @param callback The callback function.
     */
    FORCE_INLINE void setKeyCallback( KeyCallback callback );

    /**
     * @brief Sets the mouse button event callback.
     * @param callback The callback function.
     */
    FORCE_INLINE void setMouseButtonCallback( MouseButtonCallback callback );

    /**
     * @brief Sets the cursor position event callback.
     * @param callback The callback function.
     */
    FORCE_INLINE void setCursorPosCallback( CursorPosCallback callback );

    /**
     * @brief Sets the scroll event callback.
     * @param callback The callback function.
     */
    FORCE_INLINE void setScrollCallback( ScrollCallback callback );

    /**
     * @brief Sets the framebuffer resize event callback.
     * @param callback The callback function.
     */
    FORCE_INLINE void setFramebufferResizeCallback( FramebufferResizeCallback callback );

  private:
    /* ====================================== Private Members ========================================================== */

    GLFWwindow * m_window = nullptr;                                         ///< Internal GLFW window handle.
    WindowConfig m_config;                                                   ///< Window configuration details.
    Flag m_flags { Flag::Decorated | Flag::MouseVisible | Flag::Resizable }; ///< Window flags for initial setup.

    KeyCallback m_keyCallback;                             ///< Key event callback.
    MouseButtonCallback m_mouseButtonCallback;             ///< Mouse button event callback.
    CursorPosCallback m_cursorPosCallback;                 ///< Cursor position event callback.
    ScrollCallback m_scrollCallback;                       ///< Scroll event callback.
    FramebufferResizeCallback m_framebufferResizeCallback; ///< Framebuffer resize callback.

    /* ====================================== Private Methods ========================================================== */

    /**
     * @brief Configures window hints for creation.
     */
    void configureWindowHints() const;

    /**
     * @brief Applies the specified flags to the window.
     * @param flags The flags to apply.
     */
    void applyFlags( Flag flags );

    /**
     * @brief Sets up all input and window callbacks.
     */
    void setupCallbacks();

    /**
     * @brief Cleans up resources used by the window.
     */
    void cleanup() noexcept;
};

// ======================================================================================================================
// ============================================ Inline ==================================================================
// ======================================================================================================================

FORCE_INLINE bool
voWindow::checkPhysicalDevicePresentationSupport( VkInstance instance, VkPhysicalDevice device,
                                                  uint32_t queueFamilyIndex )
{
    return glfwGetPhysicalDevicePresentationSupport( instance, device, queueFamilyIndex );
}

FORCE_INLINE void
voWindow::setFlag( Flag flag, bool value )
{
    m_flags = value ? ( m_flags | flag ) : ( m_flags & ~flag );
    applyFlags( m_flags );
}

FORCE_INLINE bool
voWindow::hasFlag( Flag flag ) const
{
    return static_cast< bool >( m_flags & flag );
}

FORCE_INLINE GLFWwindow *
voWindow::getHandle() const noexcept
{
    return m_window;
}

FORCE_INLINE bool
voWindow::ShouldClose() const noexcept
{
    voWindow::pollEvents();
    return glfwWindowShouldClose( m_window );
}

FORCE_INLINE void
voWindow::setShouldClose( bool value ) noexcept
{
    glfwSetWindowShouldClose( m_window, value ? GLFW_TRUE : GLFW_FALSE );
}

FORCE_INLINE std::pair< int, int >
voWindow::getFramebufferSize() const
{
    int width, height;
    glfwGetFramebufferSize( m_window, &width, &height );
    return { width, height };
}

FORCE_INLINE void
voWindow::close() noexcept
{
    glfwSetWindowShouldClose( m_window, GLFW_TRUE );
}

FORCE_INLINE void
voWindow::pollEvents() noexcept
{
    glfwPollEvents();
}

FORCE_INLINE void
voWindow::waitEvents() noexcept
{
    glfwWaitEvents();
}

FORCE_INLINE void
voWindow::setKeyCallback( KeyCallback callback )
{
    m_keyCallback = std::move( callback );
}

FORCE_INLINE void
voWindow::setMouseButtonCallback( MouseButtonCallback callback )
{
    m_mouseButtonCallback = std::move( callback );
}

FORCE_INLINE void
voWindow::setCursorPosCallback( CursorPosCallback callback )
{
    m_cursorPosCallback = std::move( callback );
}

FORCE_INLINE void
voWindow::setScrollCallback( ScrollCallback callback )
{
    m_scrollCallback = std::move( callback );
}

FORCE_INLINE void
voWindow::setFramebufferResizeCallback( FramebufferResizeCallback callback )
{
    m_framebufferResizeCallback = std::move( callback );
}

#endif // VULKANO_WINDOW_HPP
