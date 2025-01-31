#ifndef VULKANO_SWAPCHAIN_H
#define VULKANO_SWAPCHAIN_H

#include "vo_api.hpp"
#include "vo_common.hpp"

class voDeviceContext;

/**
 * @class voSwapChain
 * @brief A class that encapsulates the Vulkan Swapchain.
 *
 * @details The voSwapChain class is responsible for managing the Vulkan Swapchain.
 * The Swapchain is a queue of images that are presented to the screen in a cyclic manner.
 * It's necessary for rendering images to the screen.
 * The class provides functionalities for creating, resizing, and cleaning up the Swapchain, as well as beginning and ending frames and render passes.
 *
 * @code
 * voDeviceContext deviceContext;
 * voSwapChain swapChain;
 *
 * // Create the swap chain
 * if ( !swapChain.Create( &deviceContext, windowWidth, windowHeight ) ) {
 *     std::cerr << "Failed to create swap chain." << std::endl;
 *     return;
 * }
 *
 * // Begin frame
 * uint32_t imageIndex = swapChain.BeginFrame( &deviceContext );
 *
 *    // Begin render pass
 *    swapChain.BeginRenderPass( &deviceContext );
 *
 *        // ... Render commands ...
 *
 *    // End render pass
 *    swapChain.EndRenderPass( &deviceContext );
 *
 * // End frame
 * swapChain.EndFrame( &deviceContext );
 *
 * // Cleanup
 * swapChain.Cleanup( &deviceContext );
 * @endcode
 * 
 * @see `voDeviceContext`
 */
class VO_API voSwapChain
{
  public:
    struct VO_API swapchain_buffers_t
    {
        VkImage image { VK_NULL_HANDLE };
        VkImageView view { VK_NULL_HANDLE };
    };

    /* -------------------------------------- Swapchain Lifecycle ------------------------------------------------------- */
    /**
     * @brief Constructs a new swap chain object.
     *
     * @param device A pointer to the device context.
     * @param width The width of the window.
     * @param height The height of the window.
     *
     * @return True if the swap chain was created successfully, false otherwise.
     *
     * @see Cleanup()
     */
    bool Create( voDeviceContext * device, int width, int height );

    /**
     * @brief Destroys the swap chain object and releases all associated resources.
     *
     * @param device A pointer to the device context.
     *
     * @see Create()
     */
    void Cleanup( voDeviceContext * device );

    /**
     * @brief Resizes the swap chain to the given width and height.
     *
     * @param device A pointer to the device context.
     * @param width The new width of the window.
     * @param height The new height of the window.
     */
    void Resize( voDeviceContext * device, int width, int height );

    /* -------------------------------------- Frame Lifecycle ------------------------------------------------------------ */
    /**
     * @brief Begins a new frame by acquiring the next available image from the swap chain.
     *
     * @param device A pointer to the device context.
     *
     * @return The index of the acquired image.
     */
    uint32_t BeginFrame( voDeviceContext * device );

    /**
     * @brief Ends the current frame by presenting the current image to the screen.
     *
     * @param device A pointer to the device context.
     *
     * @see BeginFrame()
     */
    void EndFrame( voDeviceContext * device );

    /* -------------------------------------- Render Pass Lifecycle ------------------------------------------------------ */
    /**
     * @brief Begins a new render pass.
     *
     * @param device A pointer to the device context.
     *
     * @see EndRenderPass()
     */
    void BeginRenderPass( voDeviceContext * device );

    /**
     * @brief Ends the current render pass.
     *
     * @param device A pointer to the device context.
     *
     * @see BeginRenderPass()
     */
    void EndRenderPass( voDeviceContext * device ) const;

    /* -------------------------------------- Getters -------------------------------------------------------------------- */

  public:
    [[nodiscard]] FORCE_INLINE VkRenderPass GetRenderPass() const;

    [[nodiscard]] FORCE_INLINE uint32_t GetWidth() const;

    [[nodiscard]] FORCE_INLINE uint32_t GetHeight() const;

    [[nodiscard]] FORCE_INLINE VkExtent2D GetExtent() const;

    [[nodiscard]] FORCE_INLINE uint32_t GetColorImagesSize() const;

  private:
    /* -------------------------------------- Swapchain Properties ------------------------------------------------------- */
    uint32_t m_width { 0 };
    uint32_t m_height { 0 };
    uint8_t m_resized : 1 { false }; ///< Flag indicating whether a resize has been requested

    VkSwapchainKHR m_vkSwapChain { VK_NULL_HANDLE };
    VkExtent2D m_vkExtent {};
    uint32_t m_currentImageIndex { 0 };

    /* -------------------------------------- Color Image Properties ----------------------------------------------------- */
    VkFormat m_vkColorImageFormat {};
    std::vector< swapchain_buffers_t > m_buffers {};

    /* -------------------------------------- Depth Buffer Properties ---------------------------------------------------- */
    VkFormat m_vkDepthFormat {};
    VkImage m_vkDepthImage { VK_NULL_HANDLE };
    VkImageView m_vkDepthImageView { VK_NULL_HANDLE };
    VkDeviceMemory m_vkDepthImageMemory { VK_NULL_HANDLE };

    /* -------------------------------------- Framebuffer and Render Pass Properties ------------------------------------- */
    std::vector< VkFramebuffer > m_framebuffers {};
    VkRenderPass m_vkRenderPass { VK_NULL_HANDLE };

    /* -------------------------------------- Synchronization Properties -------------------------------------------------- */
    VkSemaphore m_vkRenderFinishedSemaphore { VK_NULL_HANDLE };
    VkSemaphore m_vkImageAvailableSemaphore { VK_NULL_HANDLE };

  private:
    /* -------------------------------------- Create Functions --------------------------------------------------------- */
    /**
     * @brief Initializes semaphores for synchronizing operations within or across command queues.
     * 
     * @param device Pointer to the device context.
     */
    void CreateSemaphores( voDeviceContext * device );

    /**
     * @brief Sets up the swapchain for presenting images.
     *
     * @param device Pointer to the device context.
     */
    void CreateSwapchain( voDeviceContext * device );

    /**
     * @brief Configures the depth and stencil buffers for the swapchain images.
     *
     * @param device Pointer to the device context.
     */
    void CreateDepthStencil( voDeviceContext * device );

    /**
     * @brief Creates a render pass object specifying the render target formats and the rendering steps.
     *
     * @param device Pointer to the device context.
     */
    void CreateRenderPass( voDeviceContext * device );

    /**
     * @brief Creates framebuffers for each swapchain image, which will be used as render targets.
     *
     * @param device Pointer to the device context.
     */
    void CreateFramebuffers( voDeviceContext * device );

    /* -------------------------------------- Choose Functions --------------------------------------------------------- */
    /**
     * @brief Chooses the best surface format from the given vector of surface formats.
     *
     * @param InFormats A vector of surface formats.
     *
     * @return The best surface format.
     */
    VkSurfaceFormatKHR ChooseBestSurfaceFormat( const std::vector< VkSurfaceFormatKHR > & InFormats );

    /**
     * @brief Chooses the best presentation mode from the given vector of presentation modes.
     *
     * @param InPresentationModes A vector of presentation modes.
     *
     * @return The best presentation mode.
     */
    VkPresentModeKHR ChooseBestPresentationMode( const std::vector< VkPresentModeKHR > & InPresentationModes );

    /**
     * @brief Chooses the swap extent based on the given surface capabilities, width, and height.
     *
     * @param InSurfaceCapabilities The surface capabilities.
     * @param width The width of the window.
     * @param height The height of the window.
     *
     * @return The swap extent.
     */
    VkExtent2D ChooseSwapExtent( const VkSurfaceCapabilitiesKHR & InSurfaceCapabilities, int width, int height );

    /**
     * @brief Selects the optimal depth format for a given physical device.
     *
     * @param InPhysicalDevice Reference to the physical device for which the depth format is being chosen.
     * @param OutDepthFormat Pointer to store the chosen depth format.
     * @return VkBool32 Returns VK_TRUE if a suitable depth format is found, VK_FALSE otherwise.
     */
    VkBool32 ChooseBestDepthFormat( VkPhysicalDevice & InPhysicalDevice, VkFormat * OutDepthFormat );

    /**
     * @brief Determines the composite alpha mode based on the surface's capabilities.
     *
     * @param InSurfaceCapabilities The capabilities of the surface on which the images will be presented.
     * @return VkCompositeAlphaFlagBitsKHR The composite alpha mode that best suits the surface's capabilities.
     */
    VkCompositeAlphaFlagBitsKHR ChooseCompositeAlpha( const VkSurfaceCapabilitiesKHR & InSurfaceCapabilities );

    /* -------------------------------------- Utilities Functions --------------------------------------------------------- */
    /**
     * @brief Sets the extent (width and height) for the swapchain images.
     *
     * @param device Pointer to the device context.
     * @param width The width of the swapchain images.
     * @param height The height of the swapchain images.
     */
    void SetExtent( VkSurfaceCapabilitiesKHR & InSurfaceCapabilities, int width, int height );
};

// ======================================================================================================================
// ============================================ Inline ==================================================================
// ======================================================================================================================

FORCE_INLINE VkRenderPass
voSwapChain::GetRenderPass() const
{
    return m_vkRenderPass;
}

FORCE_INLINE uint32_t
voSwapChain::GetWidth() const
{
    return m_width;
}

FORCE_INLINE uint32_t
voSwapChain::GetHeight() const
{
    return m_height;
}

FORCE_INLINE VkExtent2D
voSwapChain::GetExtent() const
{
    return m_vkExtent;
}

FORCE_INLINE uint32_t
voSwapChain::GetColorImagesSize() const
{
    return static_cast< uint32_t >( m_buffers.size() );
}

FORCE_INLINE void
voSwapChain::SetExtent( VkSurfaceCapabilitiesKHR & InSurfaceCapabilities, int width, int height )
{
    m_width  = width;
    m_height = height;

    m_vkExtent = ChooseSwapExtent( InSurfaceCapabilities, width, height );
}

#endif //VULKANO_SWAPCHAIN_H
