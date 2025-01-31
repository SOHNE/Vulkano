#ifndef VULKANO_DEVICECONTEXT_H
#define VULKANO_DEVICECONTEXT_H

#include <vector>
#include "vo_api.hpp"
#include "vo_common.hpp"
#include "vo_swapChain.hpp"

// ======================================================================================================================
// ============================================ Structs =================================================================
// ======================================================================================================================

/**
 * @struct function_set_t
 * @brief Holder for Vulkan function pointers
 */
struct VO_API function_set_t
{
    /**
     * @brief Links the function pointers to the Vulkan functions
     *
     * @param instance The Vulkan instance
     */
    static void Link( VkInstance instance );

    static PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT;
    static PFN_vkDestroyDebugReportCallbackEXT vkDestroyDebugReportCallbackEXT;
};

/**
 * @struct physical_device_properties_t
 * @brief Holds informations about the Physical Device
 */
struct VO_API physical_device_properties_t
{
    VkPhysicalDevice physicalDevice { VK_NULL_HANDLE };
    VkPhysicalDeviceProperties deviceProperties {};
    VkPhysicalDeviceMemoryProperties memoryProperties {};
    VkPhysicalDeviceFeatures features {};
    VkSurfaceCapabilitiesKHR surfaceCapabilities {};

    std::vector< VkSurfaceFormatKHR > surfaceFormats {};
    std::vector< VkPresentModeKHR > presentModes {};
    std::vector< VkQueueFamilyProperties > queueFamilyProperties {};
    std::vector< VkExtensionProperties > extensionProperties {};

    /**
     * @brief Acquires the properties of the physical device
     *
     * @param device The Vulkan physical device
     * @param vkSurface The Vulkan surface
     * @return true if the properties were acquired successfully
     * @return false if the properties could not be acquired
     */
    bool SetProperties( VkPhysicalDevice device, VkSurfaceKHR vkSurface );

    /**
     * @brief Checks if the physical device has the specified extensions
     *
     * @param extensions The list of extension names
     * @param num The number of extensions in the list
     * @return true if the physical device has all of the specified extensions
     * @return false if the physical device does not have all of the specified extensions
     */
    bool HasExtensionsSupport( const char ** extensions, int num ) const;
};

/**
 * @struct device_t
 * @brief Contains the physical and logical devices
 */
struct VO_API device_t
{
    VkPhysicalDevice physical { VK_NULL_HANDLE };
    VkDevice logical { VK_NULL_HANDLE };
    int index { -1 };
};

/**
 * @struct queueFamilyIndices_t
 * @brief Contains the indices of the queue families
 * @details The indices (locations) of Queue Families (if they exist at all)
 */
struct VO_API queue_families_t
{
    // Locations
    int32_t graphicsFamily { -1 };
    int32_t presentationFamily { -1 };

    /** @brief Check if the queue families are valid */
    FORCE_INLINE bool
    IsValid() const
    {
        return graphicsFamily > -1 && presentationFamily > -1;
    }

    FORCE_INLINE bool
    IsGraphicsAndPresentationEqual() const
    {
        return graphicsFamily == presentationFamily;
    }
};

// ======================================================================================================================
// ============================================ Device Context ==========================================================
// ======================================================================================================================

/**
 * @class voDeviceContext
 * @brief A class that encapsulates a Vulkan Device Context.
 *
 * @details The voDeviceContext class is responsible for managing a Vulkan Device Context.
 * It provides functionalities for creating a Vulkan instance, device, physical device, logical device, 
 * command buffers, and swap chain, as well as cleaning up and releasing all Vulkan resources attached.
 * It also provides functionalities for finding a memory type index that matches the specified filter and properties,
 * getting the physical device properties, and beginning and ending a frame.
 *
 * @code
 * voDeviceContext deviceContext;
 *
 * // Create a Vulkan instance
 * bool enableLayers = true;
 * std::vector<const char*> extensions = { ... };
 * deviceContext.CreateInstance(enableLayers, extensions);
 *
 * // Create a Vulkan device
 * deviceContext.CreateDevice();
 *
 * // Create command buffers
 * deviceContext.CreateCommandBuffers();
 *
 * // Create a SwapChain
 * int width = 800, height = 600;
 * deviceContext.CreateSwapChain(width, height);
 *
 * // Begin a frame
 * uint32_t frameIndex = deviceContext.BeginFrame();
 *
 * // End a frame
 * deviceContext.EndFrame();
 *
 * // Cleanup
 * deviceContext.Cleanup();
 * @endcode
 *
 * @see `voSwapChain`, `voDescriptor`, `voDescriptors`
 */
class VO_API voDeviceContext
{
  public:
    /**
     * @brief Create a Vulkan instance
     *
     * @param enableLayers True to enable Vulkan layers, false otherwise
     * @param extensions A `std::vector` of Vulkan instance extensions
     */
    bool CreateInstance( bool enableLayers, const std::vector< const char * > & extensions );

    /** @brief Clean up and release all Vulkan resources attached */
    void Cleanup();

    /** @brief Create a Vulkan device */
    bool CreateDevice();

    /** @brief Search and create a valid `VkPhysicalDevice` */
    void CreatePhysicalDevice();

    /** @brief Create a valid `VkDevice` */
    bool CreateLogicalDevice();

    /**
     * @brief Find a memory type index that matches the specified filter and properties
     *
     * @param typeFilter The memory type filter
     * @param properties The memory properties
     * @return The memory type index
     */
    uint32_t FindMemoryTypeIndex( uint32_t typeFilter, VkMemoryPropertyFlags properties );

    /** @brief Get the physical device properties */
    physical_device_properties_t * GetPhysicalProperties();

    /** @brief Get the physical device properties (const) */
    [[nodiscard]] const physical_device_properties_t * GetPhysicalProperties() const;

    /* --------------------------------------- Instance ---------------------------------------- */

    VkInstance instance { VK_NULL_HANDLE };
    VkDebugReportCallbackEXT m_vkDebugCallback { VK_NULL_HANDLE };

    uint8_t enableLayers : 1 { true };

    /* -------------------------------------- Surface --------------------------------------- */

    VkSurfaceKHR VkSurface { VK_NULL_HANDLE };

    /* ------------------------------ Physical Device and Device ------------------------------ */

    device_t deviceInfo {};

    queue_families_t queueIds {};

    VkQueue m_vkGraphicsQueue { VK_NULL_HANDLE };
    VkQueue presentQueue { VK_NULL_HANDLE };

    std::vector< physical_device_properties_t > m_physicalDevices {};

    std::vector< const char * > m_validationLayers {};
    static const std::vector< const char * > m_deviceExtensions;

    /* ------------------------------------- Command Buffers -------------------------------------- */

    /**
     * @brief Create command buffers
     *
     * @return True if the command buffers were created successfully, false otherwise
     */
    bool CreateCommandBuffers();

    VkCommandPool m_vkCommandPool { VK_NULL_HANDLE };

    std::vector< VkCommandBuffer > m_vkCommandBuffers {};

    /**
     * @brief Create a Vulkan command buffer
     *
     * @param level The command buffer level
     *
     * @return The created VkCommandBuffer object
     */
    VkCommandBuffer CreateCommandBuffer( VkCommandBufferLevel level = VK_COMMAND_BUFFER_LEVEL_PRIMARY );

    /**
     * @brief Flush a Vulkan command buffer
     *
     * @param commandBuffer The command buffer to flush
     * @param queue The Vulkan queue to submit the command buffer to
     */
    void FlushCommandBuffer( VkCommandBuffer commandBuffer, VkQueue queue );

    /* ------------------------------------- Swap Chain ------------------------------------- */

    /**
     * @brief A SwapChain object
     * @see voSwapChain
     */
    voSwapChain swapChain;

    /**
     * @brief Create a SwapChain
     *
     * @param width The width of the SwapChain
     * @param height The height of the SwapChain
     *
     * @return True if the SwapChain was created successfully, false otherwise
     */
    bool CreateSwapChain( int width, int height );

    /**
     * @brief Resize the window
     *
     * @param width The new width of the window
     * @param height The new height of the window
     */
    void ResizeWindow( int width, int height );

    /**
     * @brief Begin a frame
     *
     * @return The index of the current frame
     */
    uint32_t BeginFrame();

    /** @brief End a frame */
    void EndFrame();

    /** @brief Begin a render pass */
    void BeginRenderPass();

    /** @brief End a render pass */
    void EndRenderPass();

    /**
     * @brief Get the aligned uniform byte offset for the given offset
     *
     * @details It calculates the aligned uniform byte offset based on the minimum uniform buffer offset alignment of the physical device.
     * It is used to ensure that the uniform buffer offset is properly aligned according to the physical device's limits.
     *
     * @param offset The offset to be aligned
     * @return The aligned uniform byte offset
     */
    [[nodiscard]] int GetAligendUniformByteOffset( int offset ) const;
};

// ======================================================================================================================
// ============================================ Inline ==================================================================
// ======================================================================================================================

FORCE_INLINE const std::vector< const char * >
    voDeviceContext::m_deviceExtensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME,
            VK_KHR_GET_MEMORY_REQUIREMENTS_2_EXTENSION_NAME,
};

FORCE_INLINE physical_device_properties_t *
voDeviceContext::GetPhysicalProperties()
{
    return &m_physicalDevices[deviceInfo.index];
}

FORCE_INLINE const physical_device_properties_t *
voDeviceContext::GetPhysicalProperties() const
{
    return &m_physicalDevices[deviceInfo.index];
}

FORCE_INLINE bool
voDeviceContext::CreateSwapChain( int width, int height )
{
    return swapChain.Create( this, width, height );
}

FORCE_INLINE void
voDeviceContext::ResizeWindow( int width, int height )
{
    spdlog::info( std::string( 50, '-' ) );

    // Log the window resize request
    spdlog::info( "Resizing window to ({}, {})", width, height );

    // Update surface capabilities
    {
        VkSurfaceCapabilitiesKHR * props = &GetPhysicalProperties()->surfaceCapabilities;

        VK_CHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( deviceInfo.physical, VkSurface, props ),
                  "Failed to vkGetPhysicalDeviceSurfaceCapabilitiesKHR!" );

        // Log surface capabilities update
        spdlog::info( "Surface capabilities updated successfully" );
    }

    // Request Swapchain resize
    swapChain.Resize( this, width, height );

    // Log swapchain resize request
    spdlog::info( "Swapchain resized to ({}, {})", width, height );

    spdlog::info( "" );
}

FORCE_INLINE uint32_t
voDeviceContext::BeginFrame()
{
    return swapChain.BeginFrame( this );
}

FORCE_INLINE void
voDeviceContext::EndFrame()
{
    swapChain.EndFrame( this );
}

FORCE_INLINE void
voDeviceContext::BeginRenderPass()
{
    swapChain.BeginRenderPass( this );
}

FORCE_INLINE void
voDeviceContext::EndRenderPass()
{
    swapChain.EndRenderPass( this );
}

#endif //VULKANO_DEVICECONTEXT_H
